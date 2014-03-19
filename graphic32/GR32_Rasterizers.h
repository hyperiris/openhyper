//unit GR32_Rasterizers;
#pragma once

#include "GR32.h"
#include "GR32_Blend.h"
#include "GR32_OrdinalMaps.h"
  
type
  TAssignColor = procedure(var Dst: TColor32; Src: TColor32) of object;

  PCombineInfo = ^TCombineInfo;
  TCombineInfo = record
    SrcAlpha: Integer;
    DrawMode: TDrawMode;
    CombineMode: TCombineMode;
    CombineCallBack: TPixelCombineEvent;
    TransparentColor: TColor32;
  end;

type
  { TRasterizer }
  { A base class for TCustomBitmap32-specific rasterizers. }
  TRasterizer = class(TThreadPersistent)
  private
    FSampler: TCustomSampler;
    FSrcAlpha: Integer;
    FBlendMemEx: TBlendMemEx;
    FCombineCallBack: TPixelCombineEvent;
    FAssignColor: TAssignColor;
    FTransparentColor: TColor32;
    procedure SetSampler(const Value: TCustomSampler);
    procedure SetCombineInfo(const CombineInfo: TCombineInfo);
    procedure AssignColorOpaque(var Dst: TColor32; Src: TColor32);
    procedure AssignColorBlend(var Dst: TColor32; Src: TColor32);
    procedure AssignColorCustom(var Dst: TColor32; Src: TColor32);
    procedure AssignColorTransparent(var Dst: TColor32; Src: TColor32);
  protected
    procedure AssignTo(Dst: TPersistent); override;
    procedure DoRasterize(Dst: TCustomBitmap32; DstRect: TRect); virtual; abstract;
    property AssignColor: TAssignColor read FAssignColor write FAssignColor;
  public
    constructor Create; override;
    procedure Assign(Source: TPersistent); override;
    procedure Rasterize(Dst: TCustomBitmap32); overload;
    procedure Rasterize(Dst: TCustomBitmap32; const DstRect: TRect); overload;
    procedure Rasterize(Dst: TCustomBitmap32; const DstRect: TRect; const CombineInfo: TCombineInfo); overload;
    procedure Rasterize(Dst: TCustomBitmap32; const DstRect: TRect; Src: TCustomBitmap32); overload;
  published
    property Sampler: TCustomSampler read FSampler write SetSampler;
  end;

  TRasterizerClass = class of TRasterizer;

  { TRegularSamplingRasterizer }
  { This rasterizer simply picks one sample for each pixel in the output bitmap. }
  TRegularRasterizer = class(TRasterizer)
  private
    FUpdateRowCount: Integer;
  protected
    procedure DoRasterize(Dst: TCustomBitmap32; DstRect: TRect); override;
  public
    constructor Create; override;
  published
    property UpdateRowCount: Integer read FUpdateRowCount write FUpdateRowCount;
  end;

  { TSwizzlingRasterizer }
  { An interesting rasterization method where sample locations are choosen
    according to a fractal pattern called 'swizzling'. With a slight
    modification to the algorithm this routine will actually yield the
    well-known sierpinski triangle fractal. An advantage with this pattern
    is that it may benefit from local coherency in the sampling method used. }
  TSwizzlingRasterizer = class(TRasterizer)
  private
    FBlockSize: Integer;
    procedure SetBlockSize(const Value: Integer);
  protected
    procedure DoRasterize(Dst: TCustomBitmap32; DstRect: TRect); override;
  public
    constructor Create; override;
  published
    property BlockSize: Integer read FBlockSize write SetBlockSize default 3;
  end;

  { TProgressiveRasterizer }
  { This class will perform rasterization in a progressive manner. It performs
    subsampling with a block size of 2^n and will successively decrease n in
    each iteration until n equals zero.  }
  TProgressiveRasterizer = class(TRasterizer)
  private
    FSteps: Integer;
    FUpdateRows: Boolean;
    procedure SetSteps(const Value: Integer);
    procedure SetUpdateRows(const Value: Boolean);
  protected
    procedure DoRasterize(Dst: TCustomBitmap32; DstRect: TRect); override;
  public
    constructor Create; override;
  published
    property Steps: Integer read FSteps write SetSteps default 4;
    property UpdateRows: Boolean read FUpdateRows write SetUpdateRows default True;
  end;

  { TTesseralRasterizer }
  { This is a recursive rasterization method. It uses a divide-and-conquer
    scheme to subdivide blocks vertically and horizontally into smaller blocks. }
  TTesseralRasterizer = class(TRasterizer)
  protected
    procedure DoRasterize(Dst: TCustomBitmap32; DstRect: TRect); override;
  end;

  { TContourRasterizer }
  TContourRasterizer = class(TRasterizer)
  protected
    procedure DoRasterize(Dst: TCustomBitmap32; DstRect: TRect); override;
  end;

  { TMultithreadedRegularRasterizer }
  TMultithreadedRegularRasterizer = class(TRasterizer)
  protected
    procedure DoRasterize(Dst: TCustomBitmap32; DstRect: TRect); override;
  end;

{ Auxiliary routines }
function CombineInfo(Bitmap: TCustomBitmap32): TCombineInfo;

const
  DEFAULT_COMBINE_INFO: TCombineInfo = (
    SrcAlpha: $FF;
    DrawMode: dmOpaque;
    CombineMode: cmBlend;
    CombineCallBack: nil;
    TransparentColor: clBlack32;
  );

var
  DefaultRasterizerClass: TRasterizerClass = TRegularRasterizer;
  NumberOfProcessors: Integer = 1;

implementation
