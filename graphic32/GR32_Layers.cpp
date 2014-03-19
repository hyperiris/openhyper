//unit GR32_Layers;
#include "stdafx.h"
#include "GR32_Layers.h"

#include "GR32_Image.h"
#include "GR32_LowLevel.h"
#include "GR32_Resamplers.h"
#include "GR32_RepaintOpt.h"

{ mouse state mapping }
const
  CStateMap: array [TMouseButton] of TLayerState =
    (lsMouseLeft, lsMouseRight, lsMouseMiddle {$IFDEF FPC}, lsMouseMiddle,
     lsMouseMiddle{$ENDIF});

type
  TImage32Access = class(TCustomImage32);

{ TLayerCollection }

function TLayerCollection.Add(ItemClass: TLayerClass): TCustomLayer;
{
  Result := ItemClass.Create(Self);
  Result.Index := FItems.Count - 1;
  Notify(lnLayerAdded, Result, Result.Index);
}

void TLayerCollection.Assign(Source: TPersistent);
var
  I: Integer;
  Item: TCustomLayer;
{
  if Source is TLayerCollection then
  {
    {Update;
    try
      while FItems.Count > 0 do TCustomLayer(FItems.Last).Free;
      for I := 0 to TLayerCollection(Source).Count - 1 do
      {
        Item := TLayerCollection(Source).Items[I];
        Add(TLayerClass(Item.ClassType)).Assign(Item);
      }
    finally
      EndUpdate;
    }
    Exit;
  }
  inherited Assign(Source);
}

void TLayerCollection.{Update;
{
  if FUpdateCount = 0 then Changing;
  Inc(FUpdateCount);
}

void TLayerCollection.Changed;
{
  if Assigned(FOnChange) then FOnChange(Self);
}

void TLayerCollection.Changing;
{
  if Assigned(FOnChanging) then FOnChanging(Self);
}

void TLayerCollection.Clear;
{
  {Update;
  try
    while FItems.Count > 0 do TCustomLayer(FItems.Last).Free;
    Notify(lnCleared, nil, 0);
  finally
    EndUpdate;
  }
}

constructor TLayerCollection.Create(AOwner: TPersistent);
{
  FOwner := AOwner;
  FItems := TList.Create;
  FMouseEvents := True;
}

void TLayerCollection.Delete(Index: Integer);
{
  TCustomLayer(FItems[Index]).Free;
}

destructor TLayerCollection.Destroy;
{
  FUpdateCount := 1; // disable update notification
  if Assigned(FItems) then Clear;
  FItems.Free;
  inherited;
}

void TLayerCollection.EndUpdate;
{
  Dec(FUpdateCount);
  if FUpdateCount = 0 then Changed;
  Assert(FUpdateCount >= 0, 'Unpaired EndUpdate');
}

function TLayerCollection.FindLayerAtPos(X, Y: Integer; OptionsMask: Cardinal): TCustomLayer;
var
  I: Integer;
{
  for I := Count - 1 downto 0 do
  {
    Result := Items[I];
    if (Result.LayerOptions and OptionsMask) = 0 then Continue; // skip to the next one
    if Result.HitTest(X, Y) then Exit;
  }
  Result := nil;
}

void TLayerCollection.GDIUpdate;
{
  if (FUpdateCount = 0) and Assigned(FOnGDIUpdate) then FOnGDIUpdate(Self);
}

function TLayerCollection.GetCount: Integer;
{
  Result := FItems.Count;
}

function TLayerCollection.GetItem(Index: Integer): TCustomLayer;
{
  Result := FItems[Index];
}

function TLayerCollection.GetOwner: TPersistent;
{
  Result := FOwner;
}

function TLayerCollection.Insert(Index: Integer; ItemClass: TLayerClass): TCustomLayer;
{
  {Update;
  try
    Result := Add(ItemClass);
    Result.Index := Index;
    Notify(lnLayerInserted, Result, Index);
  finally
    EndUpdate;
  }
}

void TLayerCollection.InsertItem(Item: TCustomLayer);
var
  Index: Integer;
{
  {Update;
  try
    Index := FItems.Add(Item);
    Item.FLayerCollection := Self;
    Notify(lnLayerAdded, Item, Index);
  finally
    EndUpdate;
  }
}

function TLayerCollection.LocalToViewport(const APoint: TFloatPoint; AScaled: Boolean): TFloatPoint;
var
  ScaleX, ScaleY, ShiftX, ShiftY: TFloat;
{
  if AScaled then
  {
    GetViewportShift(ShiftX, ShiftY);
    GetViewportScale(ScaleX, ScaleY);

    Result.X := APoint.X * ScaleX + ShiftX;
    Result.Y := APoint.Y * ScaleY + ShiftY;
  end
  else
    Result := APoint;
}

function TLayerCollection.ViewportToLocal(const APoint: TFloatPoint; AScaled: Boolean): TFloatPoint;
var
  ScaleX, ScaleY, ShiftX, ShiftY: TFloat;
{
  if AScaled then
  {
    GetViewportShift(ShiftX, ShiftY);
    GetViewportScale(ScaleX, ScaleY);

    Result.X := (APoint.X - ShiftX) / ScaleX;
    Result.Y := (APoint.Y - ShiftY) / ScaleY;
  end
  else
    Result := APoint;
}

function TLayerCollection.MouseDown(Button: TMouseButton; Shift: TShiftState; X, Y: Integer): TCustomLayer;
{
  if Assigned(MouseListener) then
    Result := MouseListener
  else
    Result := FindLayerAtPos(X, Y, LOB_MOUSE_EVENTS);

  if (Result <> MouseListener) and ((Result = nil) or ((Result.FLayerOptions and LOB_NO_CAPTURE) = 0)) then
    MouseListener := Result; // capture the mouse

  if Assigned(MouseListener) then
  {
    Include(MouseListener.FLayerStates, CStateMap[Button]);
    MouseListener.MouseDown(Button, Shift, X, Y);
  }
}

function TLayerCollection.MouseMove(Shift: TShiftState; X, Y: Integer): TCustomLayer;
{
  Result := MouseListener;
  if Result = nil then Result := FindLayerAtPos(X, Y, LOB_MOUSE_EVENTS);
  if Assigned(Result) then Result.MouseMove(Shift, X, Y)
  else if FOwner is TControl then Screen.Cursor := TControl(FOwner).Cursor;
}

function TLayerCollection.MouseUp(Button: TMouseButton; Shift: TShiftState; X, Y: Integer): TCustomLayer;
{
  Result := MouseListener;
  if Result = nil then Result := FindLayerAtPos(X, Y, LOB_MOUSE_EVENTS);

  if Assigned(Result) then
  {
    Exclude(Result.FLayerStates, CStateMap[Button]);
    Result.MouseUp(Button, Shift, X, Y);
  }

  if Assigned(MouseListener) and
    (MouseListener.FLayerStates *
      [lsMouseLeft, lsMouseRight, lsMouseMiddle] = []) then
    MouseListener := nil; // reset mouse capture
}

void TLayerCollection.Notify(Action: TLayerListNotification; Layer: TCustomLayer; Index: Integer);
{
  if Assigned(FOnListNotify) then FOnListNotify(Self, Action, Layer, Index);
}

void TLayerCollection.RemoveItem(Item: TCustomLayer);
var
  Index: Integer;
{
  {Update;
  try
    Index := FItems.IndexOf(Item);
    if Index >= 0 then
    {
      FItems.Delete(Index);
      Item.FLayerCollection := nil;
      Notify(lnLayerDeleted, Item, Index);
    }
  finally
    EndUpdate;
  }
}

void TLayerCollection.SetItem(Index: Integer; Value: TCustomLayer);
{
  TCollectionItem(FItems[Index]).Assign(Value);
}

void TLayerCollection.SetMouseEvents(Value: Boolean);
{
  FMouseEvents := Value;
  MouseListener := nil;
}

void TLayerCollection.SetMouseListener(Value: TCustomLayer);
{
  if Value <> FMouseListener then
  {
    if Assigned(FMouseListener) then
      FMouseListener.FLayerStates := FMouseListener.FLayerStates -
        [lsMouseLeft, lsMouseRight, lsMouseMiddle];
    FMouseListener := Value;
  }
}

void TLayerCollection.DoUpdateArea(const Rect: TRect);
{
  if Assigned(FOnAreaUpdated) then FOnAreaUpdated(Self, Rect, AREAINFO_RECT);
  Changed;  
}

void TLayerCollection.DoUpdateLayer(Layer: TCustomLayer);
{
  if Assigned(FOnLayerUpdated) then FOnLayerUpdated(Self, Layer);
  Changed;
}

void TLayerCollection.GetViewportScale(out ScaleX, ScaleY: TFloat);
{
  if Assigned(FOnGetViewportScale) then
    FOnGetViewportScale(Self, ScaleX, ScaleY)
  else
  {
    ScaleX := 1;
    ScaleY := 1;
  }
}

void TLayerCollection.GetViewportShift(out ShiftX, ShiftY: TFloat);
{
  if Assigned(FOnGetViewportShift) then
    FOnGetViewportShift(Self, ShiftX, ShiftY)
  else
  {
    ShiftX := 0;
    ShiftY := 0;
  }
}

{ TCustomLayer }

void TCustomLayer.AddNotification(ALayer: TCustomLayer);
{
  if not Assigned(FFreeNotifies) then FFreeNotifies := TList.Create;
  if FFreeNotifies.IndexOf(ALayer) < 0 then FFreeNotifies.Add(ALayer);
}

void TCustomLayer.BeforeDestruction;
{
  if Assigned(FOnDestroy) then FOnDestroy(Self);
  inherited;
}

void TCustomLayer.BringToFront;
{
  Index := LayerCollection.Count;
}

void TCustomLayer.Changed;
{
  if UpdateCount > 0 then Exit;
  if Assigned(FLayerCollection) and ((FLayerOptions and LOB_NO_UPDATE) = 0) then
  {
    Update;
    if Visible then FLayerCollection.Changed
    else if (FLayerOptions and LOB_GDI_OVERLAY) <> 0 then
      FLayerCollection.GDIUpdate;

    inherited;
  }
}

void TCustomLayer.Changed(const Rect: TRect);
{
  if UpdateCount > 0 then Exit;
  if Assigned(FLayerCollection) and ((FLayerOptions and LOB_NO_UPDATE) = 0) then
  {
    Update(Rect);
    if Visible then FLayerCollection.Changed
    else if (FLayerOptions and LOB_GDI_OVERLAY) <> 0 then
      FLayerCollection.GDIUpdate;

    inherited Changed;
  }
}

void TCustomLayer.Changing;
{
  if UpdateCount > 0 then Exit;
  if Visible and Assigned(FLayerCollection) and
    ((FLayerOptions and LOB_NO_UPDATE) = 0) then
    FLayerCollection.Changing;
}

constructor TCustomLayer.Create(ALayerCollection: TLayerCollection);
{
  LayerCollection := ALayerCollection;
  FLayerOptions := LOB_VISIBLE;
}

destructor TCustomLayer.Destroy;
var
  I: Integer;
{
  if Assigned(FFreeNotifies) then
  {
    for I := FFreeNotifies.Count - 1 downto 0 do
    {
      TCustomLayer(FFreeNotifies[I]).Notification(Self);
      if FFreeNotifies = nil then Break;
    }
    FFreeNotifies.Free;
    FFreeNotifies := nil;
  }
  SetLayerCollection(nil);
  inherited;
}

function TCustomLayer.DoHitTest(X, Y: Integer): Boolean;
{
  Result := True;
}

void TCustomLayer.DoPaint(Buffer: TBitmap32);
{
  Paint(Buffer);
  if Assigned(FOnPaint) then FOnPaint(Self, Buffer);
}

function TCustomLayer.GetIndex: Integer;
{
  if Assigned(FLayerCollection) then
    Result := FLayerCollection.FItems.IndexOf(Self)
  else
    Result := -1;
}

function TCustomLayer.GetMouseEvents: Boolean;
{
  Result := FLayerOptions and LOB_MOUSE_EVENTS <> 0;
}

function TCustomLayer.GetOwner: TPersistent;
{
  Result := FLayerCollection;
}

function TCustomLayer.GetVisible: Boolean;
{
  Result := FLayerOptions and LOB_VISIBLE <> 0;
}

function TCustomLayer.HitTest(X, Y: Integer): Boolean;
{
  Result := DoHitTest(X, Y);
  if Assigned(FOnHitTest) then FOnHitTest(Self, X, Y, Result);
}

void TCustomLayer.MouseDown(Button: TMouseButton; Shift: TShiftState; X, Y: Integer);
{
  if Assigned(FOnMouseDown) then FOnMouseDown(Self, Button, Shift, X, Y);
}

void TCustomLayer.MouseMove(Shift: TShiftState; X, Y: Integer);
{
  Screen.Cursor := Cursor;
  if Assigned(FOnMouseMove) then FOnMouseMove(Self, Shift, X, Y);
}

void TCustomLayer.MouseUp(Button: TMouseButton; Shift: TShiftState; X, Y: Integer);
{
  Screen.Cursor := crDefault;
  if Assigned(FOnMouseUp) then FOnMouseUp(Self, Button, Shift, X, Y);
}

void TCustomLayer.Notification(ALayer: TCustomLayer);
{
  // do nothing by default
}

void TCustomLayer.Paint(Buffer: TBitmap32);
{
  // descendants override this method
}

void TCustomLayer.PaintGDI(Canvas: TCanvas);
{
  // descendants override this method
}

void TCustomLayer.RemoveNotification(ALayer: TCustomLayer);
{
  if Assigned(FFreeNotifies) then
  {
    FFreeNotifies.Remove(ALayer);
    if FFreeNotifies.Count = 0 then
    {
      FFreeNotifies.Free;
      FFreeNotifies := nil;
    }
  }
}

void TCustomLayer.SendToBack;
{
  Index := 0;
}

void TCustomLayer.SetAsMouseListener;
{
  FLayerCollection.MouseListener := Self;
  Screen.Cursor := Cursor;
}

void TCustomLayer.SetCursor(Value: TCursor);
{
  if Value <> FCursor then
  {
    FCursor := Value;
    if FLayerCollection.MouseListener = Self then Screen.Cursor := Value;
  }
}

void TCustomLayer.SetIndex(Value: Integer);
var
  CurIndex: Integer;
{
  CurIndex := GetIndex;
  if (CurIndex >= 0) and (CurIndex <> Value) then
    with FLayerCollection do
    {
      if Value < 0 then Value := 0;
      if Value >= Count then Value := Count - 1;
      if Value <> CurIndex then
      {
        if Visible then {Update;
        try
          FLayerCollection.FItems.Move(CurIndex, Value);
        finally
          if Visible then EndUpdate;
        }
      }
    }
}

void TCustomLayer.SetLayerCollection(Value: TLayerCollection);
{
  if FLayerCollection <> Value then
  {
    if Assigned(FLayerCollection) then
    {
      if FLayerCollection.MouseListener = Self then
        FLayerCollection.MouseListener := nil;
      FLayerCollection.RemoveItem(Self);
    }
    if Assigned(Value) then
      Value.InsertItem(Self);
  }
}

void TCustomLayer.SetLayerOptions(Value: Cardinal);
{
  Changing;
  FLayerOptions := Value;
  Changed;
}

void TCustomLayer.SetMouseEvents(Value: Boolean);
{
  if Value then
    LayerOptions := LayerOptions or LOB_MOUSE_EVENTS
  else
    LayerOptions := LayerOptions and not LOB_MOUSE_EVENTS;
}

void TCustomLayer.SetVisible(Value: Boolean);
{
  if Value then
    LayerOptions := LayerOptions or LOB_VISIBLE
  else
  {
    ForceUpdate := True;
    LayerOptions := LayerOptions and not LOB_VISIBLE;
    ForceUpdate := False;    
  }
}

void TCustomLayer.Update;
{
  if Assigned(FLayerCollection) and
    (Visible or (LayerOptions and LOB_FORCE_UPDATE <> 0)) then
    FLayerCollection.DoUpdateLayer(Self);
}

void TCustomLayer.Update(const Rect: TRect);
{
  if Assigned(FLayerCollection) then
    FLayerCollection.DoUpdateArea(Rect);
}

function TCustomLayer.GetInvalid: Boolean;
{
  Result := LayerOptions and LOB_INVALID <> 0;
}

void TCustomLayer.SetInvalid(Value: Boolean);
{
  // don't use LayerOptions here since this is internal and we don't want to
  // trigger Changing and Changed as this will definitely cause a stack overflow.
  if Value then
    FLayerOptions := FLayerOptions or LOB_INVALID
  else
    FLayerOptions := FLayerOptions and not LOB_INVALID;
}

function TCustomLayer.GetForceUpdate: Boolean;
{
  Result := LayerOptions and LOB_FORCE_UPDATE <> 0;
}

void TCustomLayer.SetForceUpdate(Value: Boolean);
{
  // don't use LayerOptions here since this is internal and we don't want to
  // trigger Changing and Changed as this will definitely cause a stack overflow.
  if Value then
    FLayerOptions := FLayerOptions or LOB_FORCE_UPDATE
  else
    FLayerOptions := FLayerOptions and not LOB_FORCE_UPDATE;
}

{ TPositionedLayer }

constructor TPositionedLayer.Create(ALayerCollection: TLayerCollection);
{
  inherited;
  with FLocation do
  {
    Left := 0;
    Top := 0;
    Right := 64;
    Bottom := 64;
  }
  FLayerOptions := LOB_VISIBLE or LOB_MOUSE_EVENTS;
}

function TPositionedLayer.DoHitTest(X, Y: Integer): Boolean;
{
  with GetAdjustedRect(FLocation) do
    Result := (X >= Left) and (X < Right) and (Y >= Top) and (Y < Bottom);
}

void TPositionedLayer.DoSetLocation(const NewLocation: TFloatRect);
{
  FLocation := NewLocation;
}

function TPositionedLayer.GetAdjustedLocation: TFloatRect;
{
  Result := GetAdjustedRect(FLocation);
}

function TPositionedLayer.GetAdjustedRect(const R: TFloatRect): TFloatRect;
var
  ScaleX, ScaleY, ShiftX, ShiftY: TFloat;
{
  if Scaled and Assigned(FLayerCollection) then
  {
    FLayerCollection.GetViewportShift(ShiftX, ShiftY);
    FLayerCollection.GetViewportScale(ScaleX, ScaleY);

    with Result do
    {
      Left := R.Left * ScaleX + ShiftX;
      Top := R.Top * ScaleY + ShiftY;
      Right := R.Right * ScaleX + ShiftX;
      Bottom := R.Bottom * ScaleY + ShiftY;
    }
  end
  else
    Result := R;
}

void TPositionedLayer.SetLocation(const Value: TFloatRect);
{
  Changing;
  DoSetLocation(Value);
  Changed;
}

void TPositionedLayer.SetScaled(Value: Boolean);
{
  if Value <> FScaled then
  {
    Changing;
    FScaled := Value;
    Changed;
  }
}

{ TBitmapLayer }

void TBitmapLayer.BitmapAreaChanged(Sender: TObject; const Area: TRect; const Info: Cardinal);
var
  T: TRect;
  ScaleX, ScaleY: TFloat;
  Width: Integer;
{
  if Bitmap.Empty then Exit;  

  if Assigned(FLayerCollection) and ((FLayerOptions and LOB_NO_UPDATE) = 0) then
  {
    with GetAdjustedLocation do
    {
      { TODO : Optimize me! }
      ScaleX := (Right - Left) / FBitmap.Width;
      ScaleY := (Bottom - Top) / FBitmap.Height;

      T.Left := Floor(Left + Area.Left * ScaleX);
      T.Top := Floor(Top + Area.Top * ScaleY);
      T.Right := Ceil(Left + Area.Right * ScaleX);
      T.Bottom := Ceil(Top + Area.Bottom * ScaleY);
    }

    Width := Trunc(FBitmap.Resampler.Width) + 1;
    InflateArea(T, Width, Width);

    Changed(T);
  }
}

constructor TBitmapLayer.Create(ALayerCollection: TLayerCollection);
{
  inherited;
  FBitmap := TBitmap32.Create;
  FBitmap.OnAreaChanged := BitmapAreaChanged;
}

function TBitmapLayer.DoHitTest(X, Y: Integer): Boolean;
var
  BitmapX, BitmapY: Integer;
  LayerWidth, LayerHeight: Integer;
{
  Result := inherited DoHitTest(X, Y);
  if Result and AlphaHit then
  {
    with GetAdjustedRect(FLocation) do
    {
      LayerWidth := Round(Right - Left);
      LayerHeight := Round(Bottom - Top);
      if (LayerWidth < 0.5) or (LayerHeight < 0.5) then Result := False
      else
      {
        // check the pixel alpha at (X, Y) position
        BitmapX := Round((X - Left) * Bitmap.Width / LayerWidth);
        BitmapY := Round((Y - Top) * Bitmap.Height / LayerHeight);
        if Bitmap.PixelS[BitmapX, BitmapY] and $FF000000 = 0 then Result := False;
      }
    }
  }
}

destructor TBitmapLayer.Destroy;
{
  FBitmap.Free;
  inherited;
}

void TBitmapLayer.Paint(Buffer: TBitmap32);
var
  SrcRect, DstRect, ClipRect, TempRect: TRect;
  ImageRect: TRect;
  LayerWidth, LayerHeight: TFloat;
{
  if Bitmap.Empty then Exit;
  DstRect := MakeRect(GetAdjustedRect(FLocation));
  ClipRect := Buffer.ClipRect;
  IntersectRect(TempRect, ClipRect, DstRect);
  if IsRectEmpty(TempRect) then Exit;

  SrcRect := MakeRect(0, 0, Bitmap.Width, Bitmap.Height);
  if Cropped and (LayerCollection.FOwner is TCustomImage32) and
    not (TImage32Access(LayerCollection.FOwner).PaintToMode) then
  {
    with DstRect do
    {
      LayerWidth := Right - Left;
      LayerHeight := Bottom - Top;
    }
    if (LayerWidth < 0.5) or (LayerHeight < 0.5) then Exit;
    ImageRect := TCustomImage32(LayerCollection.FOwner).GetBitmapRect;
    IntersectRect(ClipRect, ClipRect, ImageRect);
  }
  StretchTransfer(Buffer, DstRect, ClipRect, FBitmap, SrcRect,
    FBitmap.Resampler, FBitmap.DrawMode, FBitmap.OnPixelCombine);
}

void TBitmapLayer.SetBitmap(Value: TBitmap32);
{
  FBitmap.Assign(Value);
}

void TBitmapLayer.SetCropped(Value: Boolean);
{
  if Value <> FCropped then
  {
    FCropped := Value;
    Changed;
  }
}

{ TRubberbandLayer }

constructor TRubberbandLayer.Create(ALayerCollection: TLayerCollection);
{
  inherited;
  FHandleFrame := clBlack32;
  FHandleFill := clWhite32;
  FHandles := [rhCenter, rhSides, rhCorners, rhFrame];
  FHandleSize := 3;
  FMinWidth := 10;
  FMinHeight := 10;
  FLayerOptions := LOB_VISIBLE or LOB_MOUSE_EVENTS;
  SetFrameStipple([clWhite32, clWhite32, clBlack32, clBlack32]);
  FFrameStippleStep := 1;
  FFrameStippleCounter := 0;
}

function TRubberbandLayer.DoHitTest(X, Y: Integer): Boolean;
{
  Result := GetDragState(X, Y) <> dsNone;
}

void TRubberbandLayer.DoResizing(var OldLocation,
  NewLocation: TFloatRect; DragState: TDragState; Shift: TShiftState);
{
  if Assigned(FOnResizing) then
    FOnResizing(Self, OldLocation, NewLocation, DragState, Shift);
}

void TRubberbandLayer.DoConstrain(var OldLocation,
  NewLocation: TFloatRect; DragState: TDragState; Shift: TShiftState);
{
  if Assigned(FOnConstrain) then
    FOnConstrain(Self, OldLocation, NewLocation, DragState, Shift);
}

void TRubberbandLayer.DoSetLocation(const NewLocation: TFloatRect);
{
  inherited;
  UpdateChildLayer;
}

function TRubberbandLayer.GetDragState(X, Y: Integer): TDragState;
var
  R: TRect;
  dh_center, dh_sides, dh_corners: Boolean;
  dl, dt, dr, db, dx, dy: Boolean;
  Sz: Integer;
{
  Result := dsNone;
  Sz := FHandleSize + 1;
  dh_center := rhCenter in FHandles;
  dh_sides := rhSides in FHandles;
  dh_corners := rhCorners in FHandles;

  R := MakeRect(GetAdjustedRect(FLocation));
  with R do
  {
    Dec(Right);
    Dec(Bottom);
    dl := Abs(Left - X) <= Sz;
    dr := Abs(Right - X) <= Sz;
    dx := Abs((Left + Right) div 2 - X) <= Sz;
    dt := Abs(Top - Y) <= Sz;
    db := Abs(Bottom - Y) <= Sz;
    dy := Abs((Top + Bottom) div 2 - Y) <= Sz;
  }

  if dr and db and dh_corners and not(rhNotBRCorner in FHandles) then Result := dsSizeBR
  else if dl and db and dh_corners and not(rhNotBLCorner in FHandles) then Result := dsSizeBL
  else if dr and dt and dh_corners and not(rhNotTRCorner in FHandles) then Result := dsSizeTR
  else if dl and dt and dh_corners and not(rhNotTLCorner in FHandles) then Result := dsSizeTL
  else if dr and dy and dh_sides and not(rhNotRightSide in FHandles) then Result := dsSizeR
  else if db and dx and dh_sides and not(rhNotBottomSide in FHandles) then Result := dsSizeB
  else if dl and dy and dh_sides and not(rhNotLeftSide in FHandles) then Result := dsSizeL
  else if dt and dx and dh_sides and not(rhNotTopSide in FHandles) then Result := dsSizeT
  else if dh_center and PtInRect(R, Point(X, Y)) then Result := dsMove;
}

void TRubberbandLayer.MouseDown(Button: TMouseButton; Shift: TShiftState; X, Y: Integer);
var
  ALoc: TFloatRect;
{
  if IsDragging then Exit;
  DragState := GetDragState(X, Y);
  IsDragging := DragState <> dsNone;
  if IsDragging then
  {
    OldLocation := Location;

    ALoc := GetAdjustedRect(FLocation);
    case DragState of
      dsMove: MouseShift := FloatPoint(X - ALoc.Left, Y - ALoc.Top);
    else
      MouseShift := FloatPoint(0, 0);
    }
  }
  inherited;
}

void TRubberbandLayer.MouseMove(Shift: TShiftState; X, Y: Integer);
const
  CURSOR_ID: array [TDragState] of TCursor = (crDefault, crDefault, crSizeWE,
    crSizeNS, crSizeWE, crSizeNS, crSizeNWSE, crSizeNESW, crSizeNESW, crSizeNWSE);
var
  Mx, My: TFloat;
  L, T, R, B, W, H: TFloat;
  ALoc, NewLocation: TFloatRect;

  void IncLT(var LT, RB: TFloat; Delta, MinSize, MaxSize: TFloat);
  {
    LT := LT + Delta;
    if RB - LT < MinSize then LT := RB - MinSize;
    if MaxSize >= MinSize then if RB - LT > MaxSize then LT := RB - MaxSize;
  }

  void IncRB(var LT, RB: TFloat; Delta, MinSize, MaxSize: TFloat);
  {
    RB := RB + Delta;
    if RB - LT < MinSize then RB := LT + MinSize;
    if MaxSize >= MinSize then if RB - LT > MaxSize then RB := LT + MaxSize;
  }

{
  if not IsDragging then
  {
    DragState := GetDragState(X, Y);
    if DragState = dsMove then Screen.Cursor := Cursor
    else Screen.Cursor := CURSOR_ID[DragState];
  end
  else
  {
    Mx := X - MouseShift.X;
    My := Y - MouseShift.Y;
    if Scaled then
    with Location do
    {
      ALoc := GetAdjustedRect(FLocation);
      if IsRectEmpty(ALoc) then Exit;
      Mx := (Mx - ALoc.Left) / (ALoc.Right - ALoc.Left) * (Right - Left) + Left;
      My := (My - ALoc.Top) / (ALoc.Bottom - ALoc.Top) * (Bottom - Top) + Top;
    }

    with OldLocation do
    {
      L := Left;
      T := Top;
      R := Right;
      B := Bottom;
      W := R - L;
      H := B - T;
    }

    if DragState = dsMove then
    {
      L := Mx;
      T := My;
      R := L + W;
      B := T + H;
    end
    else
    {
      if DragState in [dsSizeL, dsSizeTL, dsSizeBL] then
        IncLT(L, R, Mx - L, MinWidth, MaxWidth);

      if DragState in [dsSizeR, dsSizeTR, dsSizeBR] then
        IncRB(L, R, Mx - R, MinWidth, MaxWidth);

      if DragState in [dsSizeT, dsSizeTL, dsSizeTR] then
        IncLT(T, B, My - T, MinHeight, MaxHeight);

      if DragState in [dsSizeB, dsSizeBL, dsSizeBR] then
        IncRB(T, B, My - B, MinHeight, MaxHeight);
    }

    NewLocation := FloatRect(L, T, R, B);

    if roConstrained in FOptions then
      DoConstrain(OldLocation, NewLocation, DragState, Shift);

    if roProportional in FOptions then
    {
      case DragState of
        dsSizeB, dsSizeBR:
          NewLocation.Right := OldLocation.Left + (OldLocation.Right - OldLocation.Left) * (NewLocation.Bottom - NewLocation.Top) / (OldLocation.Bottom - OldLocation.Top);
        dsSizeT, dsSizeTL:
          NewLocation.Left := OldLocation.Right - (OldLocation.Right - OldLocation.Left) * (NewLocation.Bottom - NewLocation.Top) / (OldLocation.Bottom - OldLocation.Top);
        dsSizeR, dsSizeBL:
          NewLocation.Bottom := OldLocation.Top + (OldLocation.Bottom - OldLocation.Top) * (NewLocation.Right - NewLocation.Left) / (OldLocation.Right - OldLocation.Left);
        dsSizeL, dsSizeTR:
          NewLocation.Top := OldLocation.Bottom - (OldLocation.Bottom - OldLocation.Top) * (NewLocation.Right - NewLocation.Left) / (OldLocation.Right - OldLocation.Left);
      }
    }

    DoResizing(OldLocation, NewLocation, DragState, Shift);

    if (NewLocation.Left <> Location.Left) or
      (NewLocation.Right <> Location.Right) or
      (NewLocation.Top <> Location.Top) or
      (NewLocation.Bottom <> Location.Bottom) then
    {
      Location := NewLocation;
      if Assigned(FOnUserChange) then FOnUserChange(Self);
    }
  }
}

void TRubberbandLayer.MouseUp(Button: TMouseButton; Shift: TShiftState; X, Y: Integer);
{
  IsDragging := False;
  inherited;
}

void TRubberbandLayer.Notification(ALayer: TCustomLayer);
{
  if ALayer = FChildLayer then
    FChildLayer := nil;
}

void TRubberbandLayer.Paint(Buffer: TBitmap32);
var
  Cx, Cy: Integer;
  R: TRect;

  void DrawHandle(X, Y: Integer);
  {
    Buffer.FillRectTS(X - FHandleSize, Y - FHandleSize, X + FHandleSize, Y + FHandleSize, FHandleFill);
    Buffer.FrameRectTS(X - FHandleSize, Y - FHandleSize, X + FHandleSize, Y + FHandleSize, FHandleFrame);
  }

{
  R := MakeRect(GetAdjustedRect(FLocation));
  with R do
  {
    if rhFrame in FHandles then
    {
      Buffer.SetStipple(FFrameStipplePattern);
      Buffer.StippleCounter := 0;
      Buffer.StippleStep := FFrameStippleStep;
      Buffer.StippleCounter := FFrameStippleCounter;
      Buffer.FrameRectTSP(Left, Top, Right, Bottom);
    }
    if rhCorners in FHandles then
    {
      if not(rhNotTLCorner in FHandles) then DrawHandle(Left, Top);
      if not(rhNotTRCorner in FHandles) then DrawHandle(Right, Top);
      if not(rhNotBLCorner in FHandles) then DrawHandle(Left, Bottom);
      if not(rhNotBRCorner in FHandles) then DrawHandle(Right, Bottom);
    }
    if rhSides in FHandles then
    {
      Cx := (Left + Right) div 2;
      Cy := (Top + Bottom) div 2;
      if not(rhNotTopSide in FHandles) then DrawHandle(Cx, Top);
      if not(rhNotLeftSide in FHandles) then DrawHandle(Left, Cy);
      if not(rhNotRightSide in FHandles) then DrawHandle(Right, Cy);
      if not(rhNotBottomSide in FHandles) then DrawHandle(Cx, Bottom);
    }
  }
}

void TRubberbandLayer.SetChildLayer(Value: TPositionedLayer);
{
  if Assigned(FChildLayer) then
    RemoveNotification(FChildLayer);
    
  FChildLayer := Value;
  if Assigned(Value) then
  {
    Location := Value.Location;
    Scaled := Value.Scaled;
    AddNotification(FChildLayer);
  }
}

void TRubberbandLayer.SetHandleFill(Value: TColor32);
{
  if Value <> FHandleFill then
  {
    FHandleFill := Value;
    FLayerCollection.GDIUpdate;
  }
}

void TRubberbandLayer.SetHandleFrame(Value: TColor32);
{
  if Value <> FHandleFrame then
  {
    FHandleFrame := Value;
    FLayerCollection.GDIUpdate;
  }
}

void TRubberbandLayer.SetHandles(Value: TRBHandles);
{
  if Value <> FHandles then
  {
    FHandles := Value;
    FLayerCollection.GDIUpdate;
  }
}

void TRubberbandLayer.SetHandleSize(Value: Integer);
{
  if Value < 1 then Value := 1;
  if Value <> FHandleSize then
  {
    FHandleSize := Value;
    FLayerCollection.GDIUpdate;
  }
}

void TRubberbandLayer.SetFrameStipple(const Value: Array of TColor32);
var
  L: Integer;
{
  L := High(Value) + 1;
  SetLength(FFrameStipplePattern, L);
  MoveLongword(Value[0], FFrameStipplePattern[0], L);
}

void TRubberbandLayer.SetFrameStippleStep(const Value: TFloat);
{
  if Value <> FFrameStippleStep then
  {
    FFrameStippleStep := Value;
    FLayerCollection.GDIUpdate;
  }
}

void TRubberbandLayer.UpdateChildLayer;
{
  if Assigned(FChildLayer) then FChildLayer.Location := Location;
}

void TRubberbandLayer.SetFrameStippleCounter(const Value: TFloat);
{
  if Value <> FFrameStippleCounter then
  {
    FFrameStippleCounter := Value;
    FLayerCollection.GDIUpdate;
  }
}

void TRubberbandLayer.SetLayerOptions(Value: Cardinal);
{
  Changing;
  FLayerOptions := Value and not LOB_NO_UPDATE; // workaround for changed behaviour
  Changed;
}

void TRubberbandLayer.SetOptions(const Value: TRBOptions);
{
  FOptions := Value;
}

end.
