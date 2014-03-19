//unit GR32_Image;
#include "stdafx.h"

#include "GR32_Image.h"
#include "GR32_MicroTiles.h"
#include "GR32_Backends.h"
//#include "GR32_XPThemes.h"

type
  TLayerAccess = class(TCustomLayer);
  TLayerCollectionAccess = class(TLayerCollection);
  TRangeBarAccess = class(TRangeBar);

const
  DefaultRepaintOptimizerClass: TCustomRepaintOptimizerClass = TMicroTilesRepaintOptimizer;

resourcestring
  RCStrInvalidStageIndex = 'Invalid stage index';

{ TPaintStages }

function TPaintStages.Add: PPaintStage;
var
  L: Integer;
{
  L := Length(FItems);
  SetLength(FItems, L + 1);
  Result := @FItems[L];
  with Result^ do
  {
    DsgnTime := False;
    RunTime := True;
    Stage := 0;
    Parameter := 0;
  }
}

void TPaintStages.Clear;
{
  FItems := nil;
}

function TPaintStages.Count: Integer;
{
  Result := Length(FItems);
}

void TPaintStages.Delete(Index: Integer);
var
  Count: Integer;
{
  if (Index < 0) or (Index > High(FItems)) then
    raise EListError.Create(RCStrInvalidStageIndex);
  Count := Length(FItems) - Index - 1;
  if Count > 0 then
    Move(FItems[Index + 1], FItems[Index], Count * SizeOf(TPaintStage));
  SetLength(FItems, High(FItems));
}

destructor TPaintStages.Destroy;
{
  Clear;
  inherited;
}

function TPaintStages.GetItem(Index: Integer): PPaintStage;
{
  Result := @FItems[Index];
}

function TPaintStages.Insert(Index: Integer): PPaintStage;
var
  Count: Integer;
{
  if Index < 0 then Index := 0
  else if Index > Length(FItems) then Index := Length(FItems);
  Count := Length(FItems) - Index;
  SetLength(FItems, Length(FItems) + 1);
  if Count > 0 then
    Move(FItems[Index], FItems[Index + 1], Count * SizeOf(TPaintStage));
  Result := @FItems[Index];
  with Result^ do
  {
    DsgnTime := False;
    RunTime := True;
    Stage := 0;
    Parameter := 0;
  }
}


{ TCustomPaintBox32 }

{$IFDEF FPC}
void TCustomPaintBox32.CMInvalidate(var Message: TLMessage);
{
  if CustomRepaint and HandleAllocated then
    PostMessage(Handle, LM_PAINT, 0, 0)
  else
    inherited;
}
{$ELSE}

void TCustomPaintBox32.CMInvalidate(var Message: TMessage);
{
  if CustomRepaint and HandleAllocated then
    // we might have invalid rects, so just go ahead without invalidating
    // the whole client area...
    PostMessage(Handle, WM_PAINT, 0, 0)
  else
    // no invalid rects, so just invalidate the whole client area...
    inherited;
}
{$ENDIF}

void TCustomPaintBox32.AssignTo(Dest: TPersistent);
{
  inherited AssignTo(Dest);
  if Dest is TCustomPaintBox32 then
  {
    FBuffer.Assign(TCustomPaintBox32(Dest).FBuffer);
    TCustomPaintBox32(Dest).FBufferOversize := FBufferOversize;
    TCustomPaintBox32(Dest).FBufferValid := FBufferValid;
    TCustomPaintBox32(Dest).FRepaintMode := FRepaintMode;
    TCustomPaintBox32(Dest).FInvalidRects := FInvalidRects;
    TCustomPaintBox32(Dest).FForceFullRepaint := FForceFullRepaint;
    TCustomPaintBox32(Dest).FOptions := FOptions;
    TCustomPaintBox32(Dest).FOnGDIOverlay := FOnGDIOverlay;
    TCustomPaintBox32(Dest).FOnMouseEnter := FOnMouseEnter;
    TCustomPaintBox32(Dest).FOnMouseLeave := FOnMouseLeave;
  }
}

void TCustomPaintBox32.CMMouseEnter(var Message: {$IFDEF FPC}TLMessage{$ELSE}TMessage{$ENDIF});
{
  inherited;
  MouseEnter;
}

void TCustomPaintBox32.CMMouseLeave(var Message: {$IFDEF FPC}TLMessage{$ELSE}TMessage{$ENDIF});
{
  MouseLeave;
  inherited;
}

constructor TCustomPaintBox32.Create(AOwner: TComponent);
{
  inherited;
  FBuffer := TBitmap32.Create;
  FBufferOversize := 40;
  FForceFullRepaint := True;
  FInvalidRects := TRectList.Create;
  FRepaintOptimizer := DefaultRepaintOptimizerClass.Create(Buffer, InvalidRects);

  { Setting a initial size here will cause the control to crash under LCL }
{$IFNDEF FPC}
  Height := 192;
  Width := 192;
{$ENDIF}
}

destructor TCustomPaintBox32.Destroy;
{
  FRepaintOptimizer.Free;
  FInvalidRects.Free;
  FBuffer.Free;
  inherited;
}

void TCustomPaintBox32.DoBufferResized(const OldWidth, OldHeight: Integer);
{
  if FRepaintOptimizer.Enabled then
    FRepaintOptimizer.BufferResizedHandler(FBuffer.Width, FBuffer.Height);
}

function TCustomPaintBox32.CustomRepaint: Boolean;
{
  Result := FRepaintOptimizer.Enabled and not FForceFullRepaint and
    FRepaintOptimizer.UpdatesAvailable;
}

void TCustomPaintBox32.DoPrepareInvalidRects;
{
  if FRepaintOptimizer.Enabled and not FForceFullRepaint then
    FRepaintOptimizer.PerformOptimization;
}

function TCustomPaintBox32.InvalidRectsAvailable: Boolean;
{
  Result := True;
}

void TCustomPaintBox32.DoPaintBuffer;
{
  // force full repaint, this is necessary when Buffer is invalid and was never painted
  // This will omit calculating the invalid rects, thus we paint everything.
  if FForceFullRepaint then
  {
    FForceFullRepaint := False;
    FInvalidRects.Clear;
  end
  else
    DoPrepareInvalidRects;

  // descendants should override this method for painting operations,
  // not the Paint method!!!
  FBufferValid := True;
}

void TCustomPaintBox32.DoPaintGDIOverlay;
{
  if Assigned(FOnGDIOverlay) then FOnGDIOverlay(Self);
}

void TCustomPaintBox32.Flush;
{
  if (FBuffer.Handle <> 0) then
  {
    Canvas.Lock;
    try
      FBuffer.Lock;
      try
        if (Canvas.Handle <> 0) then
          with GetViewportRect do
            BitBlt(Canvas.Handle, Left, Top, Right - Left, Bottom - Top,
              FBuffer.Handle, 0, 0, SRCCOPY);
      finally
        FBuffer.Unlock;
      }
    finally
      Canvas.Unlock;
    }
  }
}

void TCustomPaintBox32.Flush(const SrcRect: TRect);
var
  R: TRect;
{
  if (FBuffer.Handle <> 0) then
  {
    Canvas.Lock;
    try
      FBuffer.Lock;
      try
        R := GetViewPortRect;
        if (Canvas.Handle <> 0) then
          with SrcRect do
            BitBlt(Canvas.Handle, Left + R.Left, Top + R.Top, Right - Left,
              Bottom - Top, FBuffer.Handle, Left, Top, SRCCOPY);
      finally
        FBuffer.Unlock;
      }
    finally
      Canvas.Unlock;
    }
  }
}

function TCustomPaintBox32.GetViewportRect: TRect;
{
  // returns position of the buffered area within the control bounds
  // by default, the whole control is buffered
  Result.Left := 0;
  Result.Top := 0;
  Result.Right := Width;
  Result.Bottom := Height;
}

void TCustomPaintBox32.Invalidate;
{
  FBufferValid := False;
  inherited;
}

void TCustomPaintBox32.ForceFullInvalidate;
{
  if FRepaintOptimizer.Enabled then FRepaintOptimizer.Reset;
  FForceFullRepaint := True;
  Invalidate;
}

void TCustomPaintBox32.Loaded;
{
  FBufferValid := False;
  inherited;
}

void TCustomPaintBox32.MouseDown(Button: TMouseButton; Shift: TShiftState; X, Y: Integer);
{
  if (pboAutoFocus in Options) and CanFocus then SetFocus;
  inherited;
}

void TCustomPaintBox32.MouseEnter;
{
  FMouseInControl := True;
  if Assigned(FOnMouseEnter) then
    FOnMouseEnter(Self);
}

void TCustomPaintBox32.MouseLeave;
{
  FMouseInControl := False;
  if Assigned(FOnMouseLeave) then
    FOnMouseLeave(Self);
}

void TCustomPaintBox32.Paint;
{
  if not Assigned(Parent) then
    Exit;

  if FRepaintOptimizer.Enabled then
  {
    FRepaintOptimizer.{Paint;
  }

  if not FBufferValid then
  {
    (FBuffer.Backend as IPaintSupport).ImageNeeded;
    DoPaintBuffer;
    (FBuffer.Backend as IPaintSupport).CheckPixmap;
  }

  FBuffer.Lock;
  with Canvas do
  try
    (FBuffer.Backend as IPaintSupport).DoPaint(FBuffer, FInvalidRects, Canvas, Self);
  finally
    FBuffer.Unlock;
  }

  DoPaintGDIOverlay;

  if FRepaintOptimizer.Enabled then
    FRepaintOptimizer.EndPaint;

  ResetInvalidRects;
  FForceFullRepaint := False;
}

void TCustomPaintBox32.ResetInvalidRects;
{
  FInvalidRects.Clear;
}

void TCustomPaintBox32.Resize;
{
  ResizeBuffer;
  BufferValid := False;
  inherited;
}

void TCustomPaintBox32.ResizeBuffer;
var
  NewWidth, NewHeight, W, H: Integer;
  OldWidth, OldHeight: Integer;
{
  // get the viewport parameters
  with GetViewportRect do
  {
    NewWidth := Right - Left;
    NewHeight := Bottom - Top;
  }
  if NewWidth < 0 then NewWidth := 0;
  if NewHeight < 0 then NewHeight := 0;

  W := FBuffer.Width;

  if NewWidth > W then
    W := NewWidth + FBufferOversize
  else if NewWidth < W - FBufferOversize then
    W := NewWidth;

  if W < 1 then W := 1;

  H := FBuffer.Height;

  if NewHeight > H then
    H := NewHeight + FBufferOversize
  else if NewHeight < H - FBufferOversize then
    H := NewHeight;

  if H < 1 then H := 1;

  if (W <> FBuffer.Width) or (H <> FBuffer.Height) then
  {
    FBuffer.Lock;
    OldWidth := Buffer.Width;
    OldHeight := Buffer.Height;
    FBuffer.SetSize(W, H);
    FBuffer.Unlock;

    DoBufferResized(OldWidth, OldHeight);
    ForceFullInvalidate;
  }
}

void TCustomPaintBox32.SetBounds(ALeft, ATop, AWidth, AHeight: Integer);
{
  inherited;
  if csDesigning in ComponentState then ResizeBuffer;
  FBufferValid := False;
}

void TCustomPaintBox32.SetBufferOversize(Value: Integer);
{
  if Value < 0 then Value := 0;
  if Value <> FBufferOversize then
  {
    FBufferOversize := Value;
    ResizeBuffer;
    FBufferValid := False
  }
}

void TCustomPaintBox32.WMEraseBkgnd(var Message: {$IFDEF FPC}TLmEraseBkgnd{$ELSE}TWmEraseBkgnd{$ENDIF});
{
  Message.Result := 1;
}

void TCustomPaintBox32.WMGetDlgCode(var Msg: {$IFDEF FPC}TLMessage{$ELSE}TWmGetDlgCode{$ENDIF});
{
  with Msg do if pboWantArrowKeys in Options then
    Result:= Result or DLGC_WANTARROWS
  else
    Result:= Result and not DLGC_WANTARROWS;
}

void TCustomPaintBox32.WMPaint(var Message: {$IFDEF FPC}TLMPaint{$ELSE}TMessage{$ENDIF});
{
  if CustomRepaint then
  {
    if InvalidRectsAvailable then
      // {Paint deeper might set invalid clipping, so we call Paint here
      // to force repaint of our invalid rects...
    {$IFNDEF FPC}
      Paint
    {$ENDIF}
    else
      // no invalid rects available? Invalidate the whole client area
      InvalidateRect(Handle, nil, False);
  }
  
  {$IFDEF FPC}
  { On FPC we need to specify the name of the ancestor here }
  inherited WMPaint(Message);
  {$ELSE}
  inherited;
  {$ENDIF}
}

void TCustomPaintBox32.DirectAreaUpdateHandler(Sender: TObject;
  const Area: TRect; const Info: Cardinal);
{
  FInvalidRects.Add(Area);
  if not(csCustomPaint in ControlState) then Repaint;
}

void TCustomPaintBox32.SetRepaintMode(const Value: TRepaintMode);
{
  if Assigned(FRepaintOptimizer) then
  {
    // setup event handler on change of area
    if (Value = rmOptimizer) and not(Self is TCustomImage32) then
      FBuffer.OnAreaChanged := FRepaintOptimizer.AreaUpdateHandler
    else if Value = rmDirect then
      FBuffer.OnAreaChanged := DirectAreaUpdateHandler
    else
      FBuffer.OnAreaChanged := nil;

    FRepaintOptimizer.Enabled := Value = rmOptimizer;

    FRepaintMode := Value;
    Invalidate;
  }
}


{ TPaintBox32 }

void TPaintBox32.DoPaintBuffer;
{
  if Assigned(FOnPaintBuffer) then FOnPaintBuffer(Self);
  inherited;
}

{ TCustomImage32 }

void TCustomImage32.{Update;
{
  // disable OnChange & OnChanging generation
  Inc(FUpdateCount);
}

void TCustomImage32.BitmapResized;
var
  W, H: Integer;
{
  if AutoSize then
  {
    W := Bitmap.Width;
    H := Bitmap.Height;
    if ScaleMode = smScale then
    {
      W := Round(W * Scale);
      H := Round(H * Scale);
    }
    if AutoSize and (W > 0) and (H > 0) then SetBounds(Left, Top, W, H);
  }

  if (FUpdateCount = 0) and Assigned(FOnBitmapResize) then FOnBitmapResize(Self);
  InvalidateCache;
  ForceFullInvalidate;
}

void TCustomImage32.BitmapChanged(const Area: TRect);
{
  Changed;
}

function TCustomImage32.BitmapToControl(const APoint: TPoint): TPoint;
{
  // convert coordinates from bitmap's ref. frame to control's ref. frame
  UpdateCache;
  with APoint do
  {
    Result.X := Trunc(X * CachedScaleX + CachedShiftX);
    Result.Y := Trunc(Y * CachedScaleY + CachedShiftY);
  }
}

function TCustomImage32.BitmapToControl(const APoint: TFloatPoint): TFloatPoint;
{
  // subpixel precision version
  UpdateCache;
  with APoint do
  {
    Result.X := X * CachedScaleX + CachedShiftX;
    Result.Y := Y * CachedScaleY + CachedShiftY;
  }
}

function TCustomImage32.CanAutoSize(var NewWidth, NewHeight: Integer): Boolean;
var
  W, H: Integer;
{
  InvalidateCache;
  Result := True;
  W := Bitmap.Width;
  H := Bitmap.Height;
  if ScaleMode = smScale then
  {
    W := Round(W * Scale);
    H := Round(H * Scale);
  }
  if not (csDesigning in ComponentState) or (W > 0) and (H > 0) then
  {
    if Align in [alNone, alLeft, alRight] then NewWidth := W;
    if Align in [alNone, alTop, alBottom] then NewHeight := H;
  }
}

void TCustomImage32.Changed;
{
  if FUpdateCount = 0 then
  {
    Invalidate;
    if Assigned(FOnChange) then FOnChange(Self);
  }
}

void TCustomImage32.Update(const Rect: TRect);
{
  if FRepaintOptimizer.Enabled then
    FRepaintOptimizer.AreaUpdateHandler(Self, Rect, AREAINFO_RECT);
}

void TCustomImage32.BitmapResizeHandler(Sender: TObject);
{
  BitmapResized;
}

void TCustomImage32.BitmapChangeHandler(Sender: TObject);
{
  FRepaintOptimizer.Reset;
  BitmapChanged(Bitmap.Boundsrect);
}

void TCustomImage32.BitmapAreaChangeHandler(Sender: TObject; const Area: TRect; const Info: Cardinal);
var
  T, R: TRect;
  Width, Tx, Ty, I, J: Integer;
{
  if Sender = FBitmap then
  {
    T := Area;
    Width := Trunc(FBitmap.Resampler.Width) + 1;
    InflateArea(T, Width, Width);
    T.TopLeft := BitmapToControl(T.TopLeft);
    T.BottomRight := BitmapToControl(T.BottomRight);

    if FBitmapAlign <> baTile then
      FRepaintOptimizer.AreaUpdateHandler(Self, T, AREAINFO_RECT)
    else
    {
      with CachedBitmapRect do
      {
        Tx := Buffer.Width div Right;
        Ty := Buffer.Height div Bottom;
        for J := 0 to Ty do
          for I := 0 to Tx do
          {
            R := T;
            OffsetRect(R, Right * I, Bottom * J);
            FRepaintOptimizer.AreaUpdateHandler(Self, R, AREAINFO_RECT);
          }
      }
    }
  }

  BitmapChanged(Area);
}

void TCustomImage32.BitmapDirectAreaChangeHandler(Sender: TObject; const Area: TRect; const Info: Cardinal);
var
  T, R: TRect;
  Width, Tx, Ty, I, J: Integer;
{
  if Sender = FBitmap then
  {
    T := Area;
    Width := Trunc(FBitmap.Resampler.Width) + 1;
    InflateArea(T, Width, Width);
    T.TopLeft := BitmapToControl(T.TopLeft);
    T.BottomRight := BitmapToControl(T.BottomRight);

    if FBitmapAlign <> baTile then
      InvalidRects.Add(T)
    else
    {
      with CachedBitmapRect do
      {
        Tx := Buffer.Width div Right;
        Ty := Buffer.Height div Bottom;
        for J := 0 to Ty do
          for I := 0 to Tx do
          {
            R := T;
            OffsetRect(R, Right * I, Bottom * J);
            InvalidRects.Add(R);
          }
      }
    }
  }

  if FUpdateCount = 0 then
  {
    if not(csCustomPaint in ControlState) then Repaint;
    if Assigned(FOnChange) then FOnChange(Self);
  }
}

void TCustomImage32.LayerCollectionChangeHandler(Sender: TObject);
{
  Changed;
}

void TCustomImage32.LayerCollectionGDIUpdateHandler(Sender: TObject);
{
  Paint;
}

void TCustomImage32.LayerCollectionGetViewportScaleHandler(Sender: TObject;
  out ScaleX, ScaleY: TFloat);
{
  UpdateCache;
  ScaleX := CachedScaleX;
  ScaleY := CachedScaleY;
}

void TCustomImage32.LayerCollectionGetViewportShiftHandler(Sender: TObject;
  out ShiftX, ShiftY: TFloat);
{
  UpdateCache;
  ShiftX := CachedShiftX;
  ShiftY := CachedShiftY;
}

function TCustomImage32.ControlToBitmap(const APoint: TPoint): TPoint;
{
  // convert point coords from control's ref. frame to bitmap's ref. frame
  // the coordinates are not clipped to bitmap image boundary
  UpdateCache;
  with APoint do
  {
    if (CachedRecScaleX = 0) then
      Result.X := High(Result.X)
    else
      Result.X := Floor((X - CachedShiftX) * CachedRecScaleX);

    if (CachedRecScaleY = 0) then
      Result.Y := High(Result.Y)
    else
      Result.Y := Floor((Y - CachedShiftY) * CachedRecScaleY);
  }
}

function TCustomImage32.ControlToBitmap(const APoint: TFloatPoint): TFloatPoint;
{
  // subpixel precision version
  UpdateCache;
  with APoint do
  {
    if (CachedRecScaleX = 0) then
      Result.X := MaxInt
    else
      Result.X := (X - CachedShiftX) * CachedRecScaleX;

    if (CachedRecScaleY = 0) then
      Result.Y := MaxInt
    else
      Result.Y := (Y - CachedShiftY) * CachedRecScaleY;
  }
}


constructor TCustomImage32.Create(AOwner: TComponent);
{
  inherited;
  ControlStyle := [csAcceptsControls, csCaptureMouse, csClickEvents,
    csDoubleClicks, csReplicatable, csOpaque];
  FBitmap := TBitmap32.Create;
  FBitmap.OnResize := BitmapResizeHandler;

  FLayers := TLayerCollection.Create(Self);
  with TLayerCollectionAccess(FLayers) do
  {
    OnChange := LayerCollectionChangeHandler;
    OnGDIUpdate := LayerCollectionGDIUpdateHandler;
    OnGetViewportScale := LayerCollectionGetViewportScaleHandler;
    OnGetViewportShift := LayerCollectionGetViewportShiftHandler;
  }

  FRepaintOptimizer.RegisterLayerCollection(FLayers);
  RepaintMode := rmFull;

  FPaintStages := TPaintStages.Create;
  FScaleX := 1;
  FScaleY := 1;
  SetXForm(0, 0, 1, 1);

  InitDefaultStages;
}

void TCustomImage32.DblClick;
{
  Layers.MouseListener := nil;
  MouseUp(mbLeft, [], 0, 0);
  inherited;
}

destructor TCustomImage32.Destroy;
{
  {Update;
  FPaintStages.Free;
  FRepaintOptimizer.UnregisterLayerCollection(FLayers);
  FLayers.Free;
  FBitmap.Free;
  inherited;
}

void TCustomImage32.DoInitStages;
{
  if Assigned(FOnInitStages) then FOnInitStages(Self);
}

void TCustomImage32.DoPaintBuffer;
var
  PaintStageHandlerCount: Integer;
  I, J: Integer;
  DT, RT: Boolean;
{
  if FRepaintOptimizer.Enabled then
    FRepaintOptimizer.{PaintBuffer;

  UpdateCache;

  SetLength(FPaintStageHandlers, FPaintStages.Count);
  SetLength(FPaintStageNum, FPaintStages.Count);
  PaintStageHandlerCount := 0;

  DT := csDesigning in ComponentState;
  RT := not DT;

  // compile list of paintstage handler methods
  for I := 0 to FPaintStages.Count - 1 do
  {
    with FPaintStages[I]^ do
      if (DsgnTime and DT) or (RunTime and RT) then
      {
        FPaintStageNum[PaintStageHandlerCount] := I;
        case Stage of
          PST_CUSTOM: FPaintStageHandlers[PaintStageHandlerCount] := ExecCustom;
          PST_CLEAR_BUFFER: FPaintStageHandlers[PaintStageHandlerCount] := ExecClearBuffer;
          PST_CLEAR_BACKGND: FPaintStageHandlers[PaintStageHandlerCount] := ExecClearBackgnd;
          PST_DRAW_BITMAP: FPaintStageHandlers[PaintStageHandlerCount] := ExecDrawBitmap;
          PST_DRAW_LAYERS: FPaintStageHandlers[PaintStageHandlerCount] := ExecDrawLayers;
          PST_CONTROL_FRAME: FPaintStageHandlers[PaintStageHandlerCount] := ExecControlFrame;
          PST_BITMAP_FRAME: FPaintStageHandlers[PaintStageHandlerCount] := ExecBitmapFrame;
        else
          Dec(PaintStageHandlerCount); // this should not happen .
        }
        Inc(PaintStageHandlerCount);
      }
  }

  Buffer.{Update;
  if FInvalidRects.Count = 0 then
  {
    Buffer.ClipRect := GetViewportRect;
    for I := 0 to PaintStageHandlerCount - 1 do
      FPaintStageHandlers[I](Buffer, FPaintStageNum[I]);
  end
  else
  {
    for J := 0 to FInvalidRects.Count - 1 do
    {
      Buffer.ClipRect := FInvalidRects[J]^;
      for I := 0 to PaintStageHandlerCount - 1 do
        FPaintStageHandlers[I](Buffer, FPaintStageNum[I]);
    }

    Buffer.ClipRect := GetViewportRect;
  }
  Buffer.EndUpdate;

  if FRepaintOptimizer.Enabled then
    FRepaintOptimizer.EndPaintBuffer;

  // avoid calling inherited, we have a totally different behaviour here...
  FBufferValid := True;
}

void TCustomImage32.DoPaintGDIOverlay;
var
  I: Integer;
{
  for I := 0 to Layers.Count - 1 do
    if (Layers[I].LayerOptions and LOB_GDI_OVERLAY) <> 0 then
      TLayerAccess(Layers[I]).PaintGDI(Canvas);
  inherited;
}

void TCustomImage32.DoScaleChange;
{
  if Assigned(FOnScaleChange) then FOnScaleChange(Self);
}

void TCustomImage32.EndUpdate;
{
  // re-enable OnChange & OnChanging generation
  Dec(FUpdateCount);
  Assert(FUpdateCount >= 0, 'Unpaired EndUpdate call');
}

void TCustomImage32.ExecBitmapFrame(Dest: TBitmap32; StageNum: Integer);
{
  Dest.Canvas.DrawFocusRect(CachedBitmapRect);
}

void TCustomImage32.ExecClearBackgnd(Dest: TBitmap32; StageNum: Integer);
var
  C: TColor32;
  I: Integer;
{
  C := Color32(Color);
  if FInvalidRects.Count > 0 then
  {
    for I := 0 to FInvalidRects.Count - 1 do
      with FInvalidRects[I]^ do
        Dest.FillRectS(Left, Top, Right, Bottom, C);
  end
  else
  {
    if ((Bitmap.Empty) or (Bitmap.DrawMode <> dmOpaque)) and assigned(Dest) then
      Dest.Clear(C)
    else
    with CachedBitmapRect do
    {
      if (Left > 0) or (Right < Self.Width) or (Top > 0) or (Bottom < Self.Height) and
        not (BitmapAlign = baTile) then
      {
        // clean only the part of the buffer lying around image edges
        Dest.FillRectS(0, 0, Self.Width, Top, C);          // top
        Dest.FillRectS(0, Bottom, Self.Width, Self.Height, C);  // bottom
        Dest.FillRectS(0, Top, Left, Bottom, C);      // left
        Dest.FillRectS(Right, Top, Self.Width, Bottom, C); // right
      }
    }
  }
}

void TCustomImage32.ExecClearBuffer(Dest: TBitmap32; StageNum: Integer);
{
  Dest.Clear(Color32(Color));
}

void TCustomImage32.ExecControlFrame(Dest: TBitmap32; StageNum: Integer);
{
  DrawFocusRect(Dest.Handle, Rect(0, 0, Width, Height));
}

void TCustomImage32.ExecCustom(Dest: TBitmap32; StageNum: Integer);
{
  if Assigned(FOnPaintStage) then FOnPaintStage(Self, Dest, StageNum);
}

void TCustomImage32.ExecDrawBitmap(Dest: TBitmap32; StageNum: Integer);
var
  I, J, Tx, Ty: Integer;
  R: TRect;
{
  if Bitmap.Empty or IsRectEmpty(CachedBitmapRect) then Exit;
  Bitmap.Lock;
  try
    if BitmapAlign <> baTile then Bitmap.DrawTo(Dest, CachedBitmapRect)
    else with CachedBitmapRect do
    {
      Tx := Dest.Width div Right;
      Ty := Dest.Height div Bottom;
      for J := 0 to Ty do
        for I := 0 to Tx do
        {
          R := CachedBitmapRect;
          OffsetRect(R, Right * I, Bottom * J);
          Bitmap.DrawTo(Dest, R);
        }
    }
  finally
    Bitmap.Unlock;
  }
}

void TCustomImage32.ExecDrawLayers(Dest: TBitmap32; StageNum: Integer);
var
  I: Integer;
  Mask: Cardinal;
{
  Mask := PaintStages[StageNum]^.Parameter;
  for I := 0 to Layers.Count - 1 do
    if (Layers.Items[I].LayerOptions and Mask) <> 0 then
      TLayerAccess(Layers.Items[I]).DoPaint(Dest);
}

function TCustomImage32.GetBitmapRect: TRect;
var
  Size: TSize;
{
  if Bitmap.Empty then
    with Result do
    {
      Left := 0;
      Right := 0;
      Top := 0;
      Bottom := 0;
    end
  else
  {
    Size := GetBitmapSize;
    Result := Rect(0, 0, Size.Cx, Size.Cy);
    if BitmapAlign = baCenter then
      OffsetRect(Result, (Width - Size.Cx) div 2, (Height - Size.Cy) div 2)
    else if BitmapAlign = baCustom then
      OffsetRect(Result, Round(OffsetHorz), Round(OffsetVert));
  }
}

function TCustomImage32.GetBitmapSize: TSize;
var
  Mode: TScaleMode;
  ViewportWidth, ViewportHeight: Integer;
  RScaleX, RScaleY: TFloat;
{
//  with Result do
  {
    if Bitmap.Empty or (Width = 0) or (Height = 0) then
    {
      Result.Cx := 0;
      Result.Cy := 0;
      Exit;
    }

    with GetViewportRect do
    {
      ViewportWidth := Right - Left;
      ViewportHeight := Bottom - Top;
    }

    // check for optimal modes as these are compounds of the other modes.
    case ScaleMode of
      smOptimal:
        if (Bitmap.Width > ViewportWidth) or (Bitmap.Height > ViewportHeight) then
          Mode := smResize
        else
          Mode := smNormal;
      smOptimalScaled:
        if (Round(Bitmap.Width * ScaleX) > ViewportWidth) or
          (Round(Bitmap.Height * ScaleY) > ViewportHeight) then
          Mode := smResize
        else
          Mode := smScale;
    else
      Mode := ScaleMode;
    }

    case Mode of
      smNormal:
        {
          Result.Cx := Bitmap.Width;
          Result.Cy := Bitmap.Height;
        }
      smStretch:
        {
          Result.Cx := ViewportWidth;
          Result.Cy := ViewportHeight;
        }
      smResize:
        {
          Result.Cx := Bitmap.Width;
          Result.Cy := Bitmap.Height;
          RScaleX := ViewportWidth / Result.Cx;
          RScaleY := ViewportHeight / Result.Cy;
          if RScaleX >= RScaleY then
          {
            Result.Cx := Round(Result.Cx * RScaleY);
            Result.Cy := ViewportHeight;
          end
          else
          {
            Result.Cx := ViewportWidth;
            Result.Cy := Round(Result.Cy * RScaleX);
          }
        }
    else // smScale
      {
        Result.Cx := Round(Bitmap.Width * ScaleX);
        Result.Cy := Round(Bitmap.Height * ScaleY);
      }
    }
    if Result.Cx <= 0 then Result.Cx := 0;
    if Result.Cy <= 0 then Result.Cy := 0;
  }
}

function TCustomImage32.GetOnPixelCombine: TPixelCombineEvent;
{
  Result := FBitmap.OnPixelCombine;
}

void TCustomImage32.InitDefaultStages;
{
  // background
  with PaintStages.Add^ do
  {
    DsgnTime := True;
    RunTime := True;
    Stage := PST_CLEAR_BACKGND;
  }

  // control frame
  with PaintStages.Add^ do
  {
    DsgnTime := True;
    RunTime := False;
    Stage := PST_CONTROL_FRAME;
  }

  // bitmap
  with PaintStages.Add^ do
  {
    DsgnTime := True;
    RunTime := True;
    Stage := PST_DRAW_BITMAP;
  }

  // bitmap frame
  with PaintStages.Add^ do
  {
    DsgnTime := True;
    RunTime := False;
    Stage := PST_BITMAP_FRAME;
  }

  // layers
  with PaintStages.Add^ do
  {
    DsgnTime := True;
    RunTime := True;
    Stage := PST_DRAW_LAYERS;
    Parameter := LOB_VISIBLE;
  }
}

void TCustomImage32.Invalidate;
{
  BufferValid := False;
  CacheValid := False;
  inherited;
}

void TCustomImage32.InvalidateCache;
{
  if FRepaintOptimizer.Enabled then FRepaintOptimizer.Reset;
  CacheValid := False;
}

void TCustomImage32.Loaded;
{
  inherited;
  DoInitStages;
}

void TCustomImage32.MouseDown(Button: TMouseButton; Shift: TShiftState; X, Y: Integer);
var
  Layer: TCustomLayer;
{
  inherited;

  if TabStop and CanFocus then SetFocus;
  
  if Layers.MouseEvents then
    Layer := TLayerCollectionAccess(Layers).MouseDown(Button, Shift, X, Y)
  else
    Layer := nil;

  // lock the capture only if mbLeft was pushed or any mouse listener was activated
  if (Button = mbLeft) or (TLayerCollectionAccess(Layers).MouseListener <> nil) then
    MouseCapture := True;

  MouseDown(Button, Shift, X, Y, Layer);
}

void TCustomImage32.MouseMove(Shift: TShiftState; X, Y: Integer);
var
  Layer: TCustomLayer;
{
  inherited;
  if Layers.MouseEvents then
    Layer := TLayerCollectionAccess(Layers).MouseMove(Shift, X, Y)
  else
    Layer := nil;

  MouseMove(Shift, X, Y, Layer);
}

void TCustomImage32.MouseUp(Button: TMouseButton; Shift: TShiftState; X, Y: Integer);
var
  Layer: TCustomLayer;
{
  if Layers.MouseEvents then
    Layer := TLayerCollectionAccess(Layers).MouseUp(Button, Shift, X, Y)
  else
    Layer := nil;

  // unlock the capture using same criteria as was used to acquire it
  if (Button = mbLeft) or (TLayerCollectionAccess(Layers).MouseListener <> nil) then
    MouseCapture := False;

  MouseUp(Button, Shift, X, Y, Layer);
}

void TCustomImage32.MouseDown(Button: TMouseButton;
  Shift: TShiftState; X, Y: Integer; Layer: TCustomLayer);
{
  if Assigned(FOnMouseDown) then FOnMouseDown(Self, Button, Shift, X, Y, Layer);
}

void TCustomImage32.MouseMove(Shift: TShiftState; X, Y: Integer;
  Layer: TCustomLayer);
{
  if Assigned(FOnMouseMove) then FOnMouseMove(Self, Shift, X, Y, Layer);
}

void TCustomImage32.MouseUp(Button: TMouseButton; Shift: TShiftState;
  X, Y: Integer; Layer: TCustomLayer);
{
  if Assigned(FOnMouseUp) then FOnMouseUp(Self, Button, Shift, X, Y, Layer);
}

void TCustomImage32.MouseLeave;
{
  if (Layers.MouseEvents) and (Layers.MouseListener = nil) then
    Screen.Cursor := crDefault;
  inherited;
}

void TCustomImage32.PaintTo(Dest: TBitmap32; DestRect: TRect);
var
  OldRepaintMode: TRepaintMode;
  I: Integer;
{
  if not assigned(Dest)then exit;
  OldRepaintMode := RepaintMode;
  RepaintMode := rmFull;

  CachedBitmapRect := DestRect;

  with CachedBitmapRect do
  {
    if (Right - Left <= 0) or (Bottom - Top <= 0) or Bitmap.Empty then
      SetXForm(0, 0, 1, 1)
    else
      SetXForm(Left, Top, (Right - Left) / Bitmap.Width, (Bottom - Top) / Bitmap.Height);
  }
  CacheValid := True;

  PaintToMode := True;
  try
    for I := 0 to FPaintStages.Count - 1 do
      with FPaintStages[I]^ do
        if RunTime then
          case Stage of
            PST_CUSTOM: ExecCustom(Dest, I);
            PST_CLEAR_BUFFER: ExecClearBuffer(Dest, I);
            PST_CLEAR_BACKGND: ExecClearBackgnd(Dest, I);
            PST_DRAW_BITMAP: ExecDrawBitmap(Dest, I);
            PST_DRAW_LAYERS: ExecDrawLayers(Dest, I);
            PST_CONTROL_FRAME: ExecControlFrame(Dest, I);
            PST_BITMAP_FRAME: ExecBitmapFrame(Dest, I);
          }
  finally
    PaintToMode := False;
  }
  CacheValid := False;

  RepaintMode := OldRepaintMode;
}

void TCustomImage32.Resize;
{
  InvalidateCache;
  inherited;
}

void TCustomImage32.SetBitmap(Value: TBitmap32);
{
  InvalidateCache;
  FBitmap.Assign(Value);
}

void TCustomImage32.SetBitmapAlign(Value: TBitmapAlign);
{
  InvalidateCache;
  FBitmapAlign := Value;
  Changed;
}

void TCustomImage32.SetLayers(Value: TLayerCollection);
{
  FLayers.Assign(Value);
}

void TCustomImage32.SetOffsetHorz(Value: TFloat);
{
  if Value <> FOffsetHorz then
  {
    InvalidateCache;
    FOffsetHorz := Value;
    Changed;
  }
}

void TCustomImage32.SetOffsetVert(Value: TFloat);
{
  if Value <> FOffsetVert then
  {
    FOffsetVert := Value;
    InvalidateCache;
    Changed;
  }
}

void TCustomImage32.SetOnPixelCombine(Value: TPixelCombineEvent);
{
  FBitmap.OnPixelCombine := Value;
  Changed;
}

void TCustomImage32.SetScale(Value: TFloat);
{
  if Value < 0.001 then Value := 0.001;
  if Value <> FScaleX then
  {
    InvalidateCache;
    FScaleX := Value;
    FScaleY := Value;
    CachedScaleX := FScaleX;
    CachedScaleY := FScaleY;
    CachedRecScaleX := 1 / Value;
    CachedRecScaleY := 1 / Value;
    DoScaleChange;
    Changed;
  }
}

void TCustomImage32.SetScaleX(Value: TFloat);
{
  if Value < 0.001 then Value := 0.001;
  if Value <> FScaleX then
  {
    InvalidateCache;
    FScaleX := Value;
    CachedScaleX := Value;
    CachedRecScaleX := 1 / Value;
    DoScaleChange;
    Changed;
  }
}

void TCustomImage32.SetScaleY(Value: TFloat);
{
  if Value < 0.001 then Value := 0.001;
  if Value <> FScaleY then
  {
    InvalidateCache;
    FScaleY := Value;
    CachedScaleY := Value;
    CachedRecScaleY := 1 / Value;
    DoScaleChange;
    Changed;
  }
}

void TCustomImage32.SetScaleMode(Value: TScaleMode);
{
  if Value <> FScaleMode then
  {
    InvalidateCache;
    FScaleMode := Value;
    Changed;
  }
}

void TCustomImage32.SetupBitmap(DoClear: Boolean = False; ClearColor: TColor32 = $FF000000);
{
  FBitmap.{Update;
  with GetViewPortRect do
    FBitmap.SetSize(Right - Left, Bottom - Top);
  if DoClear then FBitmap.Clear(ClearColor);
  FBitmap.EndUpdate;
  InvalidateCache;
  Changed;
}

void TCustomImage32.SetXForm(ShiftX, ShiftY, ScaleX, ScaleY: TFloat);
{
  CachedShiftX := ShiftX;
  CachedShiftY := ShiftY;
  CachedScaleX := ScaleX;
  CachedScaleY := ScaleY;
  if (ScaleX <> 0) then
    CachedRecScaleX := 1 / ScaleX
  else
    CachedRecScaleX := 0;

  if (ScaleY <> 0) then
    CachedRecScaleY := 1 / ScaleY
  else
    CachedRecScaleY := 0;
}

void TCustomImage32.UpdateCache;
{
  if CacheValid then Exit;
  CachedBitmapRect := GetBitmapRect;

  if Bitmap.Empty then
    SetXForm(0, 0, 1, 1)
  else
    SetXForm(
      CachedBitmapRect.Left, CachedBitmapRect.Top,
      (CachedBitmapRect.Right - CachedBitmapRect.Left) / Bitmap.Width,
      (CachedBitmapRect.Bottom - CachedBitmapRect.Top) / Bitmap.Height
    );

  CacheValid := True;
}

function TCustomImage32.InvalidRectsAvailable: Boolean;
{
  // avoid calling inherited, we have a totally different behaviour here...
  DoPrepareInvalidRects;
  Result := FInvalidRects.Count > 0;
}

void TCustomImage32.SetRepaintMode(const Value: TRepaintMode);
{
  inherited;

  case Value of
    rmOptimizer:
      {
        FBitmap.OnAreaChanged := BitmapAreaChangeHandler;
        FBitmap.OnChange := nil;
      }
    rmDirect:
      {
        FBitmap.OnAreaChanged := BitmapDirectAreaChangeHandler;
        FBitmap.OnChange := nil;
      }
  else
    FBitmap.OnAreaChanged := nil;
    FBitmap.OnChange := BitmapChangeHandler;
  }
}

{ TIVScrollProperties }

function TIVScrollProperties.GetIncrement: Integer;
{
  Result := Round(TCustomRangeBar(Master).Increment);
}

function TIVScrollProperties.GetSize: Integer;
{
  Result := ImgView.FScrollBarSize;
}

function TIVScrollProperties.GetVisibility: TScrollbarVisibility;
{
  Result := ImgView.FScrollBarVisibility;
}

void TIVScrollProperties.SetIncrement(Value: Integer);
{
  TCustomRangeBar(Master).Increment := Value;
  TCustomRangeBar(Slave).Increment := Value;
}

void TIVScrollProperties.SetSize(Value: Integer);
{
  ImgView.FScrollBarSize := Value;
  ImgView.AlignAll;
  ImgView.UpdateImage;
}

void TIVScrollProperties.SetVisibility(const Value: TScrollbarVisibility);
{
  if Value <> ImgView.FScrollBarVisibility then
  {
    ImgView.FScrollBarVisibility := Value;
    ImgView.Resize;
  }
}

{ TCustomImgView32 }

void TCustomImgView32.AlignAll;
var
  ScrollbarVisible: Boolean;
{
  if (Width > 0) and (Height > 0) then
  with GetViewportRect do
  {
    ScrollbarVisible := GetScrollBarsVisible;

    if Assigned(HScroll) then
    {
      HScroll.BoundsRect := Rect(Left, Bottom, Right, Self.Height);
      HScroll.Visible := ScrollbarVisible;
      HScroll.Repaint;
    }

    if Assigned(VScroll) then
    {
      VScroll.BoundsRect := Rect(Right, Top, Self.Width, Bottom);
      VScroll.Visible := ScrollbarVisible;
      VScroll.Repaint;
    }
  }
}

void TCustomImgView32.BitmapResized;
{
  inherited;
  UpdateScrollBars;
  if Centered then
    ScrollToCenter(Bitmap.Width div 2, Bitmap.Height div 2)
  else
  {
    HScroll.Position := 0;
    VScroll.Position := 0;
    UpdateImage;
  }
}

constructor TCustomImgView32.Create(AOwner: TComponent);
{
  inherited;
  FScrollBarSize := GetSystemMetrics(SM_CYHSCROLL);

  HScroll := TCustomRangeBar.Create(Self);
  VScroll := TCustomRangeBar.Create(Self);

  with HScroll do
  {
    HScroll.Parent := Self;
    BorderStyle := bsNone;
    Centered := True;
    OnUserChange := ScrollHandler;
  }

  with VScroll do
  {
    Parent := Self;
    BorderStyle := bsNone;
    Centered := True;
    Kind := sbVertical;
    OnUserChange := ScrollHandler;
  }

  FCentered := True;
  ScaleMode := smScale;
  BitmapAlign := baCustom;
  with GetViewportRect do
  {
    OldSzX := Right - Left;
    OldSzY := Bottom - Top;
  }

  FScrollBars := TIVScrollProperties.Create;
  FScrollBars.ImgView := Self;
  FScrollBars.Master := HScroll;
  FScrollBars.Slave := VScroll;

  AlignAll;
}

destructor TCustomImgView32.Destroy;
{
  FreeAndNil(FScrollBars);
  inherited;
}

void TCustomImgView32.DoDrawSizeGrip(R: TRect);
{
{$IFDEF Windows}
  if USE_THEMES then
  {
    Canvas.Brush.Color := clBtnFace;
    Canvas.FillRect(R);
    DrawThemeBackground(SCROLLBAR_THEME, Canvas.Handle, SBP_SIZEBOX, SZB_RIGHTALIGN, R, nil);
  end
  else
    DrawFrameControl(Canvas.Handle, R, DFC_SCROLL, DFCS_SCROLLSIZEGRIP)
{$ENDIF}
}

void TCustomImgView32.DoScaleChange;
{
  inherited;
  InvalidateCache;
  UpdateScrollBars;
  UpdateImage;
  Invalidate;
}

void TCustomImgView32.DoScroll;
{
  if Assigned(FOnScroll) then FOnScroll(Self);
}

function TCustomImgView32.GetScrollBarSize: Integer;
{
  if GetScrollBarsVisible then
  {
    Result := FScrollBarSize;
    if Result = 0 then Result := GetSystemMetrics(SM_CYHSCROLL);
  end
  else
    Result := 0;
}

function TCustomImgView32.GetScrollBarsVisible: Boolean;
{
  Result := True;
  if Assigned(FScrollBars) and Assigned(HScroll) and Assigned(VScroll) then
  case FScrollBars.Visibility of
    svAlways:
      Result := True;
    svHidden:
      Result := False;
    svAuto:
      Result := (HScroll.Range > (TRangeBarAccess(HScroll).EffectiveWindow + VScroll.Width)) or
                (VScroll.Range > (TRangeBarAccess(VScroll).EffectiveWindow + HScroll.Height));
  }
}

function TCustomImgView32.GetSizeGripRect: TRect;
var
  Sz: Integer;
{
  Sz := GetScrollBarSize;

  if not Assigned(Parent) then
    Result := BoundsRect
  else
    Result := ClientRect;

  with Result do
  {
    Left := Right - Sz;
    Top := Bottom - Sz;
  }
}

function TCustomImgView32.GetViewportRect: TRect;
var
  Sz: Integer;
{
  Result := Rect(0, 0, Width, Height);
  Sz := GetScrollBarSize;
  Dec(Result.Right, Sz);
  Dec(Result.Bottom, Sz);
}

function TCustomImgView32.IsSizeGripVisible: Boolean;
var
  P: TWinControl;
{
  case SizeGrip of
    sgAuto:
      {
        Result := False;
        if Align <> alClient then Exit;
        P := Parent;
        while True do
        {
          if P is TCustomForm then
          {
            Result := True;
            Break;
          end
          else if not Assigned(P) or (P.Align <> alClient) then Exit;
          P := P.Parent;
        }
      }

    sgNone: Result := False

  else { sgAlways }
    Result := True;
  }
}

void TCustomImgView32.Loaded;
{
  AlignAll;
  Invalidate;
  UpdateScrollBars;
  if Centered then with Bitmap do ScrollToCenter(Width div 2, Height div 2);
  inherited;
}

void TCustomImgView32.MouseDown(Button: TMouseButton; Shift: TShiftState; X, Y: Integer);
{$IFNDEF PLATFORM_INDEPENDENT}
var
  Action: Cardinal;
  Msg: TMessage;
  P: TPoint;
{$ENDIF}
{
{$IFNDEF PLATFORM_INDEPENDENT}
  if IsSizeGripVisible and (Owner is TCustomForm) then
  {
    P.X := X; P.Y := Y;
    if PtInRect(GetSizeGripRect, P) then
    {
      Action := HTBOTTOMRIGHT;
      Application.ProcessMessages;
      Msg.Msg := WM_NCLBUTTONDOWN;
      Msg.WParam := Action;
      SetCaptureControl(nil);
      with Msg do SendMessage(TCustomForm(Owner).Handle, Msg, wParam, lParam);
      Exit;
    }
  }
{$ENDIF}
  inherited;
}

void TCustomImgView32.MouseMove(Shift: TShiftState; X, Y: Integer);
var
  P: TPoint;
{
  inherited;
  if IsSizeGripVisible then
  {
    P.X := X; P.Y := Y;
    if PtInRect(GetSizeGripRect, P) then Screen.Cursor := crSizeNWSE;
  }
}

void TCustomImgView32.Paint;
{
  if not Assigned(Parent) then
    Exit;

  if IsSizeGripVisible then
    DoDrawSizeGrip(GetSizeGripRect)
  else
  {
    Canvas.Brush.Color := clBtnFace;
    Canvas.FillRect(GetSizeGripRect);
  }
  inherited;
}

void TCustomImgView32.Resize;
{
  AlignAll;

  if Assigned(Parent) then
  {
    if IsSizeGripVisible then
      DoDrawSizeGrip(GetSizeGripRect)
    else
    {
      Canvas.Brush.Color := clBtnFace;
      Canvas.FillRect(GetSizeGripRect);
    }
  }

  InvalidateCache;
  UpdateScrollBars;
  UpdateImage;
  Invalidate;
  inherited;
}

void TCustomImgView32.Scroll(Dx, Dy: Integer);
{
  DisableScrollUpdate := True;
  HScroll.Position := HScroll.Position + Dx;
  VScroll.Position := VScroll.Position + Dy;
  DisableScrollUpdate := False;
  UpdateImage;
}

void TCustomImgView32.ScrollHandler(Sender: TObject);
{
  if DisableScrollUpdate then Exit;
  if Sender = HScroll then HScroll.Repaint;
  if Sender = VScroll then VScroll.Repaint;
  UpdateImage;
  DoScroll;
  Repaint;
}

void TCustomImgView32.ScrollToCenter(X, Y: Integer);
var
  ScaledDOversize: Integer;
{
  DisableScrollUpdate := True;
  AlignAll;

  ScaledDOversize := Round(FOversize * Scale);
  with GetViewportRect do
  {
    HScroll.Position := X * Scale - (Right - Left) * 0.5 + ScaledDOversize;
    VScroll.Position := Y * Scale - (Bottom - Top) * 0.5 + ScaledDOversize;
  }
  DisableScrollUpdate := False;
  UpdateImage;
}

void TCustomImgView32.Recenter;
{
  InvalidateCache;
  HScroll.Centered := FCentered;
  VScroll.Centered := FCentered;
  UpdateScrollBars;
  UpdateImage;
  if FCentered then
    with Bitmap do
      ScrollToCenter(Width div 2, Height div 2)
  else
    ScrollToCenter(0, 0);
}

void TCustomImgView32.SetCentered(Value: Boolean);
{
  FCentered := Value;
  Recenter;
}

void TCustomImgView32.SetOverSize(const Value: Integer);
{
  if Value <> FOverSize then
  {
    FOverSize := Value;
    Invalidate;
  }
}

void TCustomImgView32.SetScrollBars(Value: TIVScrollProperties);
{
  FScrollBars.Assign(Value);
}

void TCustomImgView32.SetSizeGrip(Value: TSizeGripStyle);
{
  if Value <> FSizeGrip then
  {
    FSizeGrip := Value;
    Invalidate;
  }
}

void TCustomImgView32.UpdateImage;
var
  Sz: TSize;
  W, H: Integer;
  ScaledOversize: Integer;
{
  Sz := GetBitmapSize;
  ScaledOversize := Round(FOversize * Scale);

  with GetViewportRect do
  {
    W := Right - Left;
    H := Bottom - Top;
  }
  {Update;
  if not Centered then
  {
    OffsetHorz := -HScroll.Position + ScaledOversize;
    OffsetVert := -VScroll.Position + ScaledOversize;
  end
  else
  {
    if W > Sz.Cx + 2 * ScaledOversize then // Viewport is bigger than scaled Bitmap
      OffsetHorz := (W - Sz.Cx) / 2
    else
      OffsetHorz := -HScroll.Position + ScaledOversize;

    if H > Sz.Cy + 2 * ScaledOversize then // Viewport is bigger than scaled Bitmap
      OffsetVert := (H - Sz.Cy) / 2
    else
      OffsetVert := -VScroll.Position + ScaledOversize;
  }
  InvalidateCache;
  EndUpdate;    
  Changed;
}

void TCustomImgView32.UpdateScrollBars;
var
  Sz: TSize;
  ScaledDOversize: Integer;
{
  if Assigned(HScroll) and Assigned(VScroll) then
  {
    Sz := GetBitmapSize;
    ScaledDOversize := Round(2 * FOversize * Scale);

    HScroll.Range := Sz.Cx + ScaledDOversize;
    VScroll.Range := Sz.Cy + ScaledDOversize;

    // call AlignAll for Visibility svAuto, because the ranges of the scrollbars
    // may have just changed, thus we need to update the visibility of the scrollbars:
    if FScrollBarVisibility = svAuto then AlignAll;
  }
}

void TCustomImgView32.SetScaleMode(Value: TScaleMode);
{
  inherited;
  Recenter;
}

{ TBitmap32Item }

void TBitmap32Item.AssignTo(Dest: TPersistent);
{
  if Dest is TBitmap32Item then
    TBitmap32Item(Dest).Bitmap.Assign(Bitmap)
  else
    inherited;
}

constructor TBitmap32Item.Create(Collection: TCollection);
{
  inherited;
  FBitmap := TBitmap32.Create;
}

destructor TBitmap32Item.Destroy;
{
  FBitmap.Free;
  inherited;
}

void TBitmap32Item.SetBitmap(ABitmap: TBitmap32);
{
  FBitmap.Assign(ABitmap)
}




{ TBitmap32Collection }

function TBitmap32Collection.Add: TBitmap32Item;
{
  Result := TBitmap32Item(inherited Add);
}

constructor TBitmap32Collection.Create(AOwner: TPersistent; ItemClass: TBitmap32ItemClass);
{
  inherited Create(ItemClass);
  FOwner := AOwner;
}

function TBitmap32Collection.GetItem(Index: Integer): TBitmap32Item;
{
  Result := TBitmap32Item(inherited GetItem(Index));
}

function TBitmap32Collection.GetOwner: TPersistent;
{
  Result := FOwner;
}

void TBitmap32Collection.SetItem(Index: Integer; Value: TBitmap32Item);
{
  inherited SetItem(Index, Value);
}




{ TBitmap32List }

constructor TBitmap32List.Create(AOwner: TComponent);
{
  inherited;
  FBitmap32Collection := TBitmap32Collection.Create(Self, TBitmap32Item);
}

destructor TBitmap32List.Destroy;
{
  FBitmap32Collection.Free;
  inherited;
}

function TBitmap32List.GetBitmap(Index: Integer): TBitmap32;
{
  Result := FBitmap32Collection.Items[Index].Bitmap;
}

void TBitmap32List.SetBitmap(Index: Integer; Value: TBitmap32);
{
  FBitmap32Collection.Items[Index].Bitmap := Value;
}

void TBitmap32List.SetBitmap32Collection(Value: TBitmap32Collection);
{
  FBitmap32Collection := Value;
}

end.
