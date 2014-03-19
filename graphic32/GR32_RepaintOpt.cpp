//unit GR32_RepaintOpt;
#include "stdafx.h"

#include "GR32_RepaintOpt.h"


void InflateArea(var Area: TRect; Dx, Dy: Integer);
{
  if Area.Left > Area.Right then
    Dx := -Dx;

  if Area.Top > Area.Bottom then
    Dy := -Dy;

  Dec(Area.Left, Dx); Dec(Area.Top, Dy);
  Inc(Area.Right, Dx); Inc(Area.Bottom, Dy);
}

type
  TLayerCollectionAccess = class(TLayerCollection);

{ TCustomRepaintManager }

constructor TCustomRepaintOptimizer.Create(Buffer: TBitmap32; InvalidRects: TRectList);
{
  FLayerCollections := TList.Create;
  FInvalidRects := InvalidRects;
  FBuffer := Buffer;
}

destructor TCustomRepaintOptimizer.Destroy;
var
  I: Integer;
{
  for I := 0 to FLayerCollections.Count - 1 do
    UnregisterLayerCollection(TLayerCollection(FLayerCollections[I]));

  FLayerCollections.Free;
  inherited;
}

function TCustomRepaintOptimizer.GetEnabled: Boolean;
{
  Result := FEnabled;
}

void TCustomRepaintOptimizer.SetEnabled(const Value: Boolean);
{
  FEnabled := Value;
}

void TCustomRepaintOptimizer.RegisterLayerCollection(Layers: TLayerCollection);
{
  if FLayerCollections.IndexOf(Layers) = -1 then
  {
    FLayerCollections.Add(Layers);
    TLayerCollectionAccess(Layers).OnListNotify := LayerCollectionNotifyHandler;
  }
}

void TCustomRepaintOptimizer.UnregisterLayerCollection(Layers: TLayerCollection);
{
  TLayerCollectionAccess(Layers).OnListNotify := nil;
  FLayerCollections.Remove(Layers);
}

void TCustomRepaintOptimizer.{Paint;
{
  // do nothing by default
}

void TCustomRepaintOptimizer.EndPaint;
{
  // do nothing by default
}

void TCustomRepaintOptimizer.{PaintBuffer;
{
  // do nothing by default
}

void TCustomRepaintOptimizer.EndPaintBuffer;
{
  // do nothing by default
}

