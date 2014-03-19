//unit GR32_Backends_VCL;
#pragma once

#include "GR32.h"
#include "GR32_Backends.h"
#include "GR32_Containers.h"
#include "GR32_Image.h"
#include "GR32_Backends_Generic.h"

type
  { TGDIBackend }
  { This backend is the default backend on Windows.
    It uses the GDI to manage and provide the buffer and additional
    graphics sub system features. The backing buffer is kept in memory. }

  TGDIBackend = class(TCustomBackend, IPaintSupport,
    IBitmapContextSupport, IDeviceContextSupport,
    ITextSupport, IFontSupport, ICanvasSupport)
  private
    procedure FontChangedHandler(Sender: TObject);
    procedure CanvasChangedHandler(Sender: TObject);
    procedure CanvasChanged;
    procedure FontChanged;
  protected
    FBitmapInfo: TBitmapInfo;
    FBitmapHandle: HBITMAP;
    FHDC: HDC;
    FFont: TFont;
    FCanvas: TCanvas;
    FFontHandle: HFont;
    FMapHandle: THandle;

    FOnFontChange: TNotifyEvent;
    FOnCanvasChange: TNotifyEvent;

    procedure InitializeSurface(NewWidth, NewHeight: Integer; ClearBuffer: Boolean); override;
    procedure FinalizeSurface; override;

    procedure PrepareFileMapping(NewWidth, NewHeight: Integer); virtual;
  public
    constructor Create; override;
    destructor Destroy; override;

    procedure Changed; override;

    function Empty: Boolean; override;
  public
    { IPaintSupport }
    procedure ImageNeeded;
    procedure CheckPixmap;
    procedure DoPaint(ABuffer: TBitmap32; AInvalidRects: TRectList; ACanvas: TCanvas; APaintBox: TCustomPaintBox32);

    { IBitmapContextSupport }
    function GetBitmapInfo: TBitmapInfo;
    function GetBitmapHandle: THandle;

    property BitmapInfo: TBitmapInfo read GetBitmapInfo;
    property BitmapHandle: THandle read GetBitmapHandle;

    { IDeviceContextSupport }
    function GetHandle: HDC;

    procedure Draw(const DstRect, SrcRect: TRect; hSrc: HDC); overload;
    procedure DrawTo(hDst: {$IFDEF BCB}Cardinal{$ELSE}HDC{$ENDIF}; DstX, DstY: Integer); overload;
    procedure DrawTo(hDst: {$IFDEF BCB}Cardinal{$ELSE}HDC{$ENDIF}; const DstRect, SrcRect: TRect); overload;

    property Handle: HDC read GetHandle;

    { ITextSupport }
    procedure Textout(X, Y: Integer; const Text: String); overload;
    procedure Textout(X, Y: Integer; const ClipRect: TRect; const Text: String); overload;
    procedure Textout(var DstRect: TRect; const Flags: Cardinal; const Text: String); overload;
    function  TextExtent(const Text: String): TSize;

    procedure TextoutW(X, Y: Integer; const Text: Widestring); overload;
    procedure TextoutW(X, Y: Integer; const ClipRect: TRect; const Text: Widestring); overload;
    procedure TextoutW(var DstRect: TRect; const Flags: Cardinal; const Text: Widestring); overload;
    function  TextExtentW(const Text: Widestring): TSize;

    { IFontSupport }
    function GetOnFontChange: TNotifyEvent;
    procedure SetOnFontChange(Handler: TNotifyEvent);
    function GetFont: TFont;
    procedure SetFont(const Font: TFont);

    procedure UpdateFont;
    property Font: TFont read GetFont write SetFont;
    property OnFontChange: TNotifyEvent read FOnFontChange write FOnFontChange;

    { ICanvasSupport }
    function GetCanvasChange: TNotifyEvent;
    procedure SetCanvasChange(Handler: TNotifyEvent);
    function GetCanvas: TCanvas;

    procedure DeleteCanvas;
    function CanvasAllocated: Boolean;

    property Canvas: TCanvas read GetCanvas;
    property OnCanvasChange: TNotifyEvent read GetCanvasChange write SetCanvasChange;
  end;

  { TGDIMMFBackend }
  { Same as TGDIBackend but relies on memory mapped files or mapped swap space
    for the backing buffer. }

  TGDIMMFBackend = class(TGDIBackend)
  private
    FMapFileHandle: THandle;
    FMapIsTemporary: Boolean;
    FMapFileName: string;
  protected
    procedure PrepareFileMapping(NewWidth, NewHeight: Integer); override;
  public
    constructor Create(Owner: TBitmap32; IsTemporary: Boolean = True; const MapFileName: string = ''); virtual;
    destructor Destroy; override;
  end;

  { TGDIMemoryBackend }
  { A backend that keeps the backing buffer entirely in memory and offers
    IPaintSupport without allocating a GDI handle }

  TGDIMemoryBackend = class(TMemoryBackend, IPaintSupport, IDeviceContextSupport)
  private
    procedure DoPaintRect(ABuffer: TBitmap32; ARect: TRect; ACanvas: TCanvas);

    function GetHandle: {$IFDEF BCB}Cardinal{$ELSE}HDC{$ENDIF}; // Dummy
  protected
    FBitmapInfo: TBitmapInfo;

    procedure InitializeSurface(NewWidth: Integer; NewHeight: Integer;
      ClearBuffer: Boolean); override;
  public
    constructor Create; override;

    { IPaintSupport }
    procedure ImageNeeded;
    procedure CheckPixmap;
    procedure DoPaint(ABuffer: TBitmap32; AInvalidRects: TRectList; ACanvas: TCanvas; APaintBox: TCustomPaintBox32);

    { IDeviceContextSupport }
    procedure Draw(const DstRect, SrcRect: TRect; hSrc: {$IFDEF BCB}Cardinal{$ELSE}HDC{$ENDIF}); overload;
    procedure DrawTo(hDst: {$IFDEF BCB}Cardinal{$ELSE}HDC{$ENDIF}; DstX, DstY: Integer); overload;
    procedure DrawTo(hDst: {$IFDEF BCB}Cardinal{$ELSE}HDC{$ENDIF}; const DstRect, SrcRect: TRect); overload;
  end;

implementation
