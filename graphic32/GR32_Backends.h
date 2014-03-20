//unit GR32_Backends;
#pragma once

#include "GR32.h"
#include "GR32_Containers.h"
#include "GR32_Image.h"

class ITextSupport
{
	virtual void Textout(X, Y: Integer; const Text : String) = 0;
	virtual void Textout(X, Y: Integer; const ClipRect : TRect; const Text : String) = 0;
	virtual void Textout(var DstRect : TRect; const Flags : Cardinal; const Text : String) = 0;
	virtual TSize TextExtent(const Text : String) = 0;

	virtual void TextoutW(X, Y: Integer; const Text : Widestring) = 0;
	virtual void TextoutW(X, Y: Integer; const ClipRect : TRect; const Text : Widestring) = 0;
	virtual void TextoutW(var DstRect : TRect; const Flags : Cardinal; const Text : Widestring) = 0;
	virtual TSize TextExtentW(const Text : Widestring) = 0;
};

class IFontSupport
{
	virtual function GetOnFontChange : TNotifyEvent = 0;
	virtual void SetOnFontChange(Handler: TNotifyEvent) = 0;
	virtual function GetFont : TFont = 0;
	virtual void SetFont(const Font : TFont) = 0;

	virtual void UpdateFont = 0;
	virtual property Font: TFont read GetFont write SetFont;
	virtual property OnFontChange: TNotifyEvent read GetOnFontChange write SetOnFontChange;
};

class ICanvasSupport
{
	virtual function GetCanvasChange : TNotifyEvent = 0;
	virtual void SetCanvasChange(Handler: TNotifyEvent) = 0;
	virtual function GetCanvas : TCanvas = 0;

	virtual void DeleteCanvas = 0;
	virtual function CanvasAllocated : Boolean = 0;

	virtual property Canvas: TCanvas read GetCanvas;
	virtual property OnCanvasChange: TNotifyEvent read GetCanvasChange write SetCanvasChange;
};

class IDeviceContextSupport
{
	virtual function GetHandle : HDC = 0;

	virtual void Draw(const DstRect, SrcRect: TRect; hSrc: HDC) = 0;
	virtual void DrawTo(hDst: HDC; DstX, DstY: Integer); overload = 0;
	virtual void DrawTo(hDst: HDC; const DstRect, SrcRect: TRect); overload = 0;

	virtual property Handle: HDC read GetHandle;
};

class IBitmapContextSupport
{
	virtual function GetBitmapInfo : TBitmapInfo = 0;
	virtual function GetBitmapHandle : THandle = 0;

	virtual property BitmapInfo: TBitmapInfo read GetBitmapInfo;
	virtual property BitmapHandle: THandle read GetBitmapHandle;
};

class IPaintSupport
{
	virtual void ImageNeeded = 0;
	virtual void CheckPixmap = 0;
	virtual void DoPaint(ABuffer: TBitmap32; AInvalidRects: TRectList; ACanvas: TCanvas; APaintBox: TCustomPaintBox32) = 0;
};

enum TRequireOperatorMode
{
	romAnd,
	romOr
};

// Helper functions to temporarily switch the back-end depending on the required interfaces

void RequireBackendSupport(TargetBitmap: TCustomBitmap32;
  RequiredInterfaces: array of TGUID;
  Mode: TRequireOperatorMode; UseOptimizedDestructiveSwitchMethod: Boolean;
  out ReleasedBackend: TCustomBackend);

void RestoreBackend(TargetBitmap: TCustomBitmap32; const SavedBackend: TCustomBackend);

/*
resourcestring
  RCStrCannotAllocateDIBHandle = 'Can''t allocate the DIB handle';
  RCStrCannotCreateCompatibleDC = 'Can''t create compatible DC';
  RCStrCannotSelectAnObjectIntoDC = 'Can''t select an object into DC';
*/
