//unit GR32_Backends;
#pragma once

#include "GR32.h"
#include "GR32_Containers.h"
#include "GR32_Image.h"


type
  ITextSupport = interface(IUnknown)
  ['{225997CC-958A-423E-8B60-9EDE0D3B53B5}']
    procedure Textout(X, Y: Integer; const Text: String); overload;
    procedure Textout(X, Y: Integer; const ClipRect: TRect; const Text: String); overload;
    procedure Textout(var DstRect: TRect; const Flags: Cardinal; const Text: String); overload;
    function  TextExtent(const Text: String): TSize;

    procedure TextoutW(X, Y: Integer; const Text: Widestring); overload;
    procedure TextoutW(X, Y: Integer; const ClipRect: TRect; const Text: Widestring); overload;
    procedure TextoutW(var DstRect: TRect; const Flags: Cardinal; const Text: Widestring); overload;
    function  TextExtentW(const Text: Widestring): TSize;
  end;

  IFontSupport = interface(IUnknown)
  ['{67C73044-1EFF-4FDE-AEA2-56BFADA50A48}']
    function GetOnFontChange: TNotifyEvent;
    procedure SetOnFontChange(Handler: TNotifyEvent);
    function GetFont: TFont;
    procedure SetFont(const Font: TFont);

    procedure UpdateFont;
    property Font: TFont read GetFont write SetFont;
    property OnFontChange: TNotifyEvent read GetOnFontChange write SetOnFontChange;
  end;

  ICanvasSupport = interface(IUnknown)
  ['{5ACFEEC7-0123-4AD8-8AE6-145718438E01}']
    function GetCanvasChange: TNotifyEvent;
    procedure SetCanvasChange(Handler: TNotifyEvent);
    function GetCanvas: TCanvas;

    procedure DeleteCanvas;
    function CanvasAllocated: Boolean;

    property Canvas: TCanvas read GetCanvas;
    property OnCanvasChange: TNotifyEvent read GetCanvasChange write SetCanvasChange;
  end;

  IDeviceContextSupport = interface(IUnknown)
  ['{DD1109DA-4019-4A5C-A450-3631A73CF288}']
    function GetHandle: HDC;

    procedure Draw(const DstRect, SrcRect: TRect; hSrc: HDC);
    procedure DrawTo(hDst: HDC; DstX, DstY: Integer); overload;
    procedure DrawTo(hDst: HDC; const DstRect, SrcRect: TRect); overload;

    property Handle: HDC read GetHandle;
  end;

  IBitmapContextSupport = interface(IUnknown)
  ['{DF0F9475-BA13-4C6B-81C3-D138624C4D08}']
    function GetBitmapInfo: TBitmapInfo;
    function GetBitmapHandle: THandle;

    property BitmapInfo: TBitmapInfo read GetBitmapInfo;
    property BitmapHandle: THandle read GetBitmapHandle;
  end;

  IPaintSupport = interface(IUnknown)
  ['{CE64DBEE-C4A9-4E8E-ABCA-1B1FD6F45924}']
    procedure ImageNeeded;
    procedure CheckPixmap;
    procedure DoPaint(ABuffer: TBitmap32; AInvalidRects: TRectList; ACanvas: TCanvas; APaintBox: TCustomPaintBox32);
  end;

  TRequireOperatorMode = (romAnd, romOr);

// Helper functions to temporarily switch the back-end depending on the required interfaces

procedure RequireBackendSupport(TargetBitmap: TCustomBitmap32;
  RequiredInterfaces: array of TGUID;
  Mode: TRequireOperatorMode; UseOptimizedDestructiveSwitchMethod: Boolean;
  out ReleasedBackend: TCustomBackend);

procedure RestoreBackend(TargetBitmap: TCustomBitmap32; const SavedBackend: TCustomBackend);

resourcestring
  RCStrCannotAllocateDIBHandle = 'Can''t allocate the DIB handle';
  RCStrCannotCreateCompatibleDC = 'Can''t create compatible DC';
  RCStrCannotSelectAnObjectIntoDC = 'Can''t select an object into DC';

