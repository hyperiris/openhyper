//unit GR32_ExtImage;
#include "stdafx.h"
#include "GR32_ExtImage.h"

procedure Rasterize(Rasterizer: TRasterizer; Dst: TBitmap32; DstRect: TRect);
var
  R: TRenderThread;
begin
  R := TRenderThread.Create(Rasterizer, Dst, DstRect, True);
  R.FreeOnTerminate := True;
{$IFDEF USETHREADRESUME}
  R.Resume;
{$ELSE}
  R.Start;
{$ENDIF}
end;

{ TSyntheticImage32 }

constructor TSyntheticImage32.Create(AOwner: TComponent);
begin
  inherited;
  FRasterizer := TRegularRasterizer.Create;
  FRasterizer.Sampler := Buffer.Resampler;
  FAutoRasterize := True;
  FResized := False;
  RepaintMode := rmDirect;
  RenderMode := rnmFull;
  BufferOversize := 0;
end;

destructor TSyntheticImage32.Destroy;
var
  ParentForm: TCustomForm;
begin
  StopRenderThread;
  if Assigned(FRenderThread) then FRenderThread.Free;
  if Assigned(FDefaultProc) then
  begin
    ParentForm := GetParentForm(Self);
    if ParentForm <> nil then
      ParentForm.WindowProc := FDefaultProc;
  end;
  FRasterizer.Free;
  inherited;
end;

procedure TSyntheticImage32.DoRasterize;
begin
  if FAutoRasterize then Rasterize;
end;

{$IFDEF FPC}
procedure TSyntheticImage32.FormWindowProc(var Message: TLMessage);
var
  CmdType: Integer;
begin
  FDefaultProc(Message);
  case Message.Msg of
    534: FResized := False;
    562:
      begin
        if FResized then DoRasterize;
        FResized := True;
      end;
    274:
      begin
        CmdType := Message.WParam and $FFF0;
        if (CmdType = SC_MAXIMIZE) or (CmdType = SC_RESTORE) then
          DoRasterize;
      end;
  end;
end;
{$ELSE}
procedure TSyntheticImage32.FormWindowProc(var Message: TMessage);
var
  CmdType: Integer;
begin
  FDefaultProc(Message);
  case Message.Msg of
    WM_MOVING: FResized := False;
    WM_EXITSIZEMOVE:
      begin
        if FResized then DoRasterize;
        FResized := True;
      end;
    WM_SYSCOMMAND:
      begin
        CmdType := Message.WParam and $FFF0;
        if (CmdType = SC_MAXIMIZE) or (CmdType = SC_RESTORE) then
          DoRasterize;
      end;
  end;
end;
{$ENDIF}

procedure TSyntheticImage32.Rasterize;
var
  R: TRect;
begin
  { Clear buffer before rasterization }
  if FClearBuffer then
  begin
    Buffer.Clear(Color32(Color));
    Invalidate;
  end;

  { Create rendering thread }
  StopRenderThread;
  FOldAreaChanged := Buffer.OnAreaChanged;
  if FRenderMode = rnmFull then
    R := Rect(0, 0, Buffer.Width, Buffer.Height)
  else
    R := FDstRect;

  FRenderThread := TRenderThread.Create(FRasterizer, Buffer, R, False);
  FResized := True;
end;

procedure TSyntheticImage32.RasterizerChanged(Sender: TObject);
begin
  DoRasterize;
end;

procedure TSyntheticImage32.Resize;
begin
  if not FResized then StopRenderThread;
  inherited;
end;

procedure TSyntheticImage32.SetDstRect(const Value: TRect);
begin
  FDstRect := Value;
end;

procedure TSyntheticImage32.SetParent(AParent: TWinControl);
var
  ParentForm: TCustomForm;
begin
  ParentForm := GetParentForm(Self);
  if ParentForm = AParent then Exit;
  if ParentForm <> nil then
    if Assigned(FDefaultProc) then
      ParentForm.WindowProc := FDefaultProc;
  inherited;
  if AParent <> nil then
  begin
    ParentForm := GetParentForm(Self);
    if ParentForm <> nil then
    begin
      FDefaultProc := ParentForm.WindowProc;
      ParentForm.WindowProc := FormWindowProc;
    end;
  end;
end;

procedure TSyntheticImage32.SetRasterizer(const Value: TRasterizer);
begin
  if Value <> FRasterizer then
  begin
    StopRenderThread;
    if Assigned(FRasterizer) then FRasterizer.Free;
    FRasterizer := Value;
    FRasterizer.OnChange := RasterizerChanged;
    DoRasterize;
    Changed;
  end;
end;

procedure TSyntheticImage32.SetRenderMode(const Value: TRenderMode);
begin
  FRenderMode := Value;
end;

procedure TSyntheticImage32.StopRenderThread;
begin
  if Assigned(FRenderThread) and (not FRenderThread.Terminated) then
  begin
    FRenderThread.Synchronize(FRenderThread.Terminate);
    FRenderThread.WaitFor;
    FreeAndNil(FRenderThread);
  end;
end;

{ TRenderThread }

constructor TRenderThread.Create(Rasterizer: TRasterizer; Dst: TBitmap32;
  DstRect: TRect; Suspended: Boolean);
begin
{$IFDEF USETHREADRESUME}
  inherited Create(True);
{$ELSE}
  inherited Create(Suspended);
{$ENDIF}
  FRasterizer := Rasterizer;
  FDest := Dst;
  FDstRect := DstRect;
  Priority := tpNormal;
{$IFDEF USETHREADRESUME}
  if not Suspended then Resume;
{$ENDIF}
end;

procedure TRenderThread.Execute;
begin
  Rasterize;
end;

procedure TRenderThread.Rasterize;
begin
  FRasterizer.Lock;

  { Save current AreaChanged handler }
  FOldAreaChanged := FDest.OnAreaChanged;

  FDest.OnAreaChanged := AreaChanged;
  try
    FRasterizer.Rasterize(FDest, FDstRect);
  except
    on EAbort do;
  end;

  { Reset old AreaChanged handler }
  FDest.OnAreaChanged := FOldAreaChanged;

  Synchronize(FRasterizer.Unlock);
end;

procedure TRenderThread.AreaChanged(Sender: TObject; const Area: TRect;
  const Hint: Cardinal);
begin
  if Terminated then Abort else
  begin
    FArea := Area;
    Synchronize(SynchronizedAreaChanged);
  end;
end;

procedure TRenderThread.SynchronizedAreaChanged;
begin
  if Assigned(FOldAreaChanged) then
    FOldAreaChanged(FDest, FArea, AREAINFO_RECT);
end;

end.
