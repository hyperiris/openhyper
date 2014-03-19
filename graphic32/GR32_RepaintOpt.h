//unit GR32_RepaintOpt;
#pragma once

#include "GR32.h"
#include "GR32_LowLevel.h"
#include "GR32_Containers.h"
#include "GR32_Layers.h"


//  { TCustomRepaintOptimizer }
class TCustomRepaintOptimizer
{
private:
	FEnabled: Boolean;
	FLayerCollections: TList;
	FInvalidRects: TRectList;
	FBuffer: TBitmap32;
protected:
	function GetEnabled: Boolean; virtual;
	void SetEnabled(const Value: Boolean); virtual;
	property LayerCollections: TList read FLayerCollections write FLayerCollections;
	property Buffer: TBitmap32 read FBuffer write FBuffer;
	property InvalidRects: TRectList read FInvalidRects write FInvalidRects;

	// LayerCollection handler
	void LayerCollectionNotifyHandler(Sender: TLayerCollection;
Action: TLayerListNotification; Layer: TCustomLayer; Index: Integer); virtual; abstract;
public:
	MyClass();
	~MyClass();
	constructor Create(Buffer: TBitmap32; InvalidRects: TRectList); virtual;
	destructor Destroy; override;

	void RegisterLayerCollection(Layers: TLayerCollection); virtual;
	void UnregisterLayerCollection(Layers: TLayerCollection); virtual;

	void BeginPaint; virtual;
	void EndPaint; virtual;
	void BeginPaintBuffer; virtual;
	void EndPaintBuffer; virtual;

	void Reset; virtual; abstract;
	function  UpdatesAvailable: Boolean; virtual; abstract;
	void PerformOptimization; virtual; abstract;

	// handlers
	void AreaUpdateHandler(Sender: TObject; const Area: TRect; const Info: Cardinal); virtual; abstract;
	void LayerUpdateHandler(Sender: TObject; Layer: TCustomLayer); virtual; abstract;
	void BufferResizedHandler(const NewWidth, NewHeight: Integer); virtual; abstract;

	property Enabled: Boolean read GetEnabled write SetEnabled;
};

//TCustomRepaintOptimizerClass = class of TCustomRepaintOptimizer;

// differs from InflateRect in the way that it does also handle negative rects
void InflateArea(TRect& Area, int Dx, int Dy);

