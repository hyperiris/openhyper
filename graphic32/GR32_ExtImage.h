//unit GR32_ExtImage;
#pragma once
#include "GR32.h"
#include "GR32_Image.h"
#include "GR32_Rasterizers.h"

type
  TRenderThread = class;

  TRenderMode = (rnmFull, rnmConstrained);

  { TSyntheticImage32 }
  TSyntheticImage32 = class(TPaintBox32)
  private
    FRasterizer: TRasterizer;
    FAutoRasterize: Boolean;
    FDefaultProc: TWndMethod;
    FResized: Boolean;
    FRenderThread: TRenderThread;
    FOldAreaChanged: TAreaChangedEvent;
    FDstRect: TRect;
    FRenderMode: TRenderMode;
    FClearBuffer: Boolean;
    procedure SetRasterizer(const Value: TRasterizer);
    procedure StopRenderThread;
    procedure SetDstRect(const Value: TRect);
    procedure SetRenderMode(const Value: TRenderMode);
  protected
    procedure RasterizerChanged(Sender: TObject);
    procedure SetParent(AParent: TWinControl); override;
    {$IFDEF FPC}
    procedure FormWindowProc(var Message: TLMessage);
    {$ELSE}
    procedure FormWindowProc(var Message: TMessage);
    {$ENDIF}
    procedure DoRasterize;
    property RepaintMode;
  public
    constructor Create(AOwner: TComponent); override;
    destructor Destroy; override;
    procedure Resize; override;
    procedure Rasterize;
    property DstRect: TRect read FDstRect write SetDstRect;
  published
    property AutoRasterize: Boolean read FAutoRasterize write FAutoRasterize;
    property Rasterizer: TRasterizer read FRasterizer write SetRasterizer;
    property Buffer;
    property Color;
    property ClearBuffer: Boolean read FClearBuffer write FClearBuffer;
    property RenderMode: TRenderMode read FRenderMode write SetRenderMode;
  end;

  { TRenderThread }
  TRenderThread = class(TThread)
  private
    FDest: TBitmap32;
    FRasterizer: TRasterizer;
    FOldAreaChanged: TAreaChangedEvent;
    FArea: TRect;
    FDstRect: TRect;
    procedure SynchronizedAreaChanged;
    procedure AreaChanged(Sender: TObject; const Area: TRect; const Hint: Cardinal);
  protected
    procedure Execute; override;
    procedure Rasterize;
  public
    constructor Create(Rasterizer: TRasterizer; Dst: TBitmap32; DstRect: TRect;
      Suspended: Boolean);
  end;

procedure Rasterize(Rasterizer: TRasterizer; Dst: TBitmap32; DstRect: TRect);

implementation
