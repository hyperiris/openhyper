//unit GR32_Resamplers;
#pragma once

#include "GR32.h"
#include "GR32_Transforms.h"
#include "GR32_Containers.h"
#include "GR32_OrdinalMaps.h"
#include "GR32_Blend.h"
#include "GR32_System.h"
#include "GR32_Bindings.h"

void BlockTransfer(
  Dst: TCustomBitmap32; DstX: Integer; DstY: Integer; DstClip: TRect;
  Src: TCustomBitmap32; SrcRect: TRect;
  CombineOp: TDrawMode; CombineCallBack: TPixelCombineEvent = nil);

void BlockTransferX(
  Dst: TCustomBitmap32; DstX, DstY: TFixed; 
  Src: TCustomBitmap32; SrcRect: TRect;
  CombineOp: TDrawMode; CombineCallBack: TPixelCombineEvent = nil);

void StretchTransfer(
  Dst: TCustomBitmap32; DstRect: TRect; DstClip: TRect;
  Src: TCustomBitmap32; SrcRect: TRect;
  Resampler: TCustomResampler;
  CombineOp: TDrawMode; CombineCallBack: TPixelCombineEvent = nil);

void BlendTransfer(
  Dst: TCustomBitmap32; DstX, DstY: Integer; DstClip: TRect;
  SrcF: TCustomBitmap32; SrcRectF: TRect;
  SrcB: TCustomBitmap32; SrcRectB: TRect;
  BlendCallback: TBlendReg); overload;

void BlendTransfer(
  Dst: TCustomBitmap32; DstX, DstY: Integer; DstClip: TRect;
  SrcF: TCustomBitmap32; SrcRectF: TRect;
  SrcB: TCustomBitmap32; SrcRectB: TRect;
  BlendCallback: TBlendRegEx; MasterAlpha: Integer); overload;

const
  MAX_KERNEL_WIDTH = 16;

type
  PKernelEntry = ^TKernelEntry;
  TKernelEntry = array [-MAX_KERNEL_WIDTH..MAX_KERNEL_WIDTH] of Integer;

  TArrayOfKernelEntry = array of TArrayOfInteger;
  PKernelEntryArray = ^TKernelEntryArray;
  TKernelEntryArray = array [0..0] of TArrayOfInteger;

  TFilterMethod = function(Value: TFloat): TFloat of object;

  EBitmapException = class(Exception);
  ESrcInvalidException = class(Exception);
  ENestedException = class(Exception);

  TGetSampleInt = function(X, Y: Integer): TColor32 of object;
  TGetSampleFloat = function(X, Y: TFloat): TColor32 of object;
  TGetSampleFixed = function(X, Y: TFixed): TColor32 of object;

  { TCustomKernel }
  TCustomKernel = class(TPersistent)
  protected
    FObserver: TNotifiablePersistent;
  protected
    void AssignTo(Dst: TPersistent); override;
    function RangeCheck: Boolean; virtual;
  public
    constructor Create; virtual;
    void Changed;
    function Filter(Value: TFloat): TFloat; virtual; abstract;
    function GetWidth: TFloat; virtual; abstract;
    property Observer: TNotifiablePersistent read FObserver;
  end;
  TCustomKernelClass = class of TCustomKernel;

  { TBoxKernel }
  TBoxKernel = class(TCustomKernel)
  public
    function Filter(Value: TFloat): TFloat; override;
    function GetWidth: TFloat; override;
  end;

  { TLinearKernel }
  TLinearKernel = class(TCustomKernel)
  public
    function Filter(Value: TFloat): TFloat; override;
    function GetWidth: TFloat; override;
  end;

  { TCosineKernel }
  TCosineKernel = class(TCustomKernel)
  public
    function Filter(Value: TFloat): TFloat; override;
    function GetWidth: TFloat; override;
  end;

  { TSplineKernel }
  TSplineKernel = class(TCustomKernel)
  protected
    function RangeCheck: Boolean; override;
  public
    function Filter(Value: TFloat): TFloat; override;
    function GetWidth: TFloat; override;
  end;

  { TMitchellKernel }
  TMitchellKernel = class(TCustomKernel)
  protected
    function RangeCheck: Boolean; override;
  public
    function Filter(Value: TFloat): TFloat; override;
    function GetWidth: TFloat; override;
  end;

  { TCubicKernel }
  TCubicKernel = class(TCustomKernel)
  private
    FCoeff: TFloat;
    void SetCoeff(const Value: TFloat);
  protected
    function RangeCheck: Boolean; override;
  public
    constructor Create; override;
    function Filter(Value: TFloat): TFloat; override;
    function GetWidth: TFloat; override;
  published
    property Coeff: TFloat read FCoeff write SetCoeff;
  end;

  { THermiteKernel }
  THermiteKernel = class(TCustomKernel)
  private
    FBias: TFloat;
    FTension: TFloat;
    void SetBias(const Value: TFloat);
    void SetTension(const Value: TFloat);
  protected
    function RangeCheck: Boolean; override;
  public
    constructor Create; override;
    function Filter(Value: TFloat): TFloat; override;
    function GetWidth: TFloat; override;
  published
    property Bias: TFloat read FBias write SetBias;
    property Tension: TFloat read FTension write SetTension;
  end;

  { TWindowedSincKernel }
  TWindowedSincKernel = class(TCustomKernel)
  private
    FWidth : TFloat;
    FWidthReciprocal : TFloat;
  protected
    function RangeCheck: Boolean; override;
    function Window(Value: TFloat): TFloat; virtual; abstract;
  public
    constructor Create; override;
    function Filter(Value: TFloat): TFloat; override;
    void SetWidth(Value: TFloat);
    function GetWidth: TFloat; override;
    property WidthReciprocal : TFloat read FWidthReciprocal;
  published
    property Width: TFloat read FWidth write SetWidth;
  end;

  { TAlbrecht-Kernel }
  TAlbrechtKernel = class(TWindowedSincKernel)
  private
    FTerms: Integer;
    FCoefPointer : Array [0..11] of Double;
    void SetTerms(Value : Integer);
  protected
    function Window(Value: TFloat): TFloat; override;
  public
    constructor Create; override;
  published
    property Terms: Integer read FTerms write SetTerms;
  end;

  { TLanczosKernel }
  TLanczosKernel = class(TWindowedSincKernel)
  protected
    function Window(Value: TFloat): TFloat; override;
  public
  end;

  { TGaussianKernel }
  TGaussianKernel = class(TWindowedSincKernel)
  private
    FSigma: TFloat;
    FSigmaReciprocalLn2: TFloat;
    void SetSigma(const Value: TFloat);
  protected
    function Window(Value: TFloat): TFloat; override;
  public
    constructor Create; override;
  published
    property Sigma: TFloat read FSigma write SetSigma;
  end;

  { TBlackmanKernel }
  TBlackmanKernel = class(TWindowedSincKernel)
  protected
    function Window(Value: TFloat): TFloat; override;
  end;

  { THannKernel }
  THannKernel = class(TWindowedSincKernel)
  protected
    function Window(Value: TFloat): TFloat; override;
  end;

  { THammingKernel }
  THammingKernel = class(TWindowedSincKernel)
  protected
    function Window(Value: TFloat): TFloat; override;
  end;

  { TSinshKernel }
  TSinshKernel = class(TCustomKernel)
  private
    FWidth: TFloat;
    FCoeff: TFloat;
    void SetCoeff(const Value: TFloat);
  protected
    function  RangeCheck: Boolean; override;
  public
    constructor Create; override;
    void SetWidth(Value: TFloat);
    function  GetWidth: TFloat; override;
    function  Filter(Value: TFloat): TFloat; override;
  published
    property Coeff: TFloat read FCoeff write SetCoeff;
    property Width: TFloat read GetWidth write SetWidth;
  end;


  { TNearestResampler }
  TNearestResampler = class(TCustomResampler)
  private
    FGetSampleInt: TGetSampleInt;
  protected
    function GetPixelTransparentEdge(X, Y: Integer): TColor32;
    function GetWidth: TFloat; override;
    void Resample(
      Dst: TCustomBitmap32; DstRect: TRect; DstClip: TRect;
      Src: TCustomBitmap32; SrcRect: TRect;
      CombineOp: TDrawMode; CombineCallBack: TPixelCombineEvent); override;
  public
    function GetSampleInt(X, Y: Integer): TColor32; override;
    function GetSampleFixed(X, Y: TFixed): TColor32; override;
    function GetSampleFloat(X, Y: TFloat): TColor32; override;
    void PrepareSampling; override;
  end;

  { TLinearResampler }
  TLinearResampler = class(TCustomResampler)
  private
    FLinearKernel: TLinearKernel;
    FGetSampleFixed: TGetSampleFixed;
  protected
    function GetWidth: TFloat; override;
    function GetPixelTransparentEdge(X, Y: TFixed): TColor32;
    void Resample(
      Dst: TCustomBitmap32; DstRect: TRect; DstClip: TRect;
      Src: TCustomBitmap32; SrcRect: TRect;
      CombineOp: TDrawMode; CombineCallBack: TPixelCombineEvent); override;
  public
    constructor Create; override;
    destructor Destroy; override;
    function GetSampleFixed(X, Y: TFixed): TColor32; override;
    function GetSampleFloat(X, Y: TFloat): TColor32; override;
    void PrepareSampling; override;
  end;

  { TDraftResampler }
  TDraftResampler = class(TLinearResampler)
  protected
    void Resample(
      Dst: TCustomBitmap32; DstRect: TRect; DstClip: TRect;
      Src: TCustomBitmap32; SrcRect: TRect;
      CombineOp: TDrawMode; CombineCallBack: TPixelCombineEvent); override;
  end;

  { TKernelResampler }
  { This resampler class will perform resampling by using an arbitrary
    reconstruction kernel. By using the kmTableNearest and kmTableLinear
    kernel modes, kernel values are precomputed in a look-up table. This
    allows GetSample to execute faster for complex kernels. }

  TKernelMode = (kmDynamic, kmTableNearest, kmTableLinear);

  TKernelResampler = class(TCustomResampler)
  private
    FKernel: TCustomKernel;
    FKernelMode: TKernelMode;
    FWeightTable: TIntegerMap;
    FTableSize: Integer;
    FOuterColor: TColor32;
    void SetKernel(const Value: TCustomKernel);
    function GetKernelClassName: string;
    void SetKernelClassName(Value: string);
    void SetKernelMode(const Value: TKernelMode);
    void SetTableSize(Value: Integer);
  protected
    function GetWidth: TFloat; override;
  public
    constructor Create; override;
    destructor Destroy; override;
    function GetSampleFloat(X, Y: TFloat): TColor32; override;
    void Resample(
      Dst: TCustomBitmap32; DstRect: TRect; DstClip: TRect;
      Src: TCustomBitmap32; SrcRect: TRect;
      CombineOp: TDrawMode; CombineCallBack: TPixelCombineEvent); override;
    void PrepareSampling; override;
    void FinalizeSampling; override;
  published
    property KernelClassName: string read GetKernelClassName write SetKernelClassName;
    property Kernel: TCustomKernel read FKernel write SetKernel;
    property KernelMode: TKernelMode read FKernelMode write SetKernelMode;
    property TableSize: Integer read FTableSize write SetTableSize;
  end;

  { TNestedSampler }
  TNestedSampler = class(TCustomSampler)
  private
    FSampler: TCustomSampler;
    FGetSampleInt: TGetSampleInt;
    FGetSampleFixed: TGetSampleFixed;
    FGetSampleFloat: TGetSampleFloat;
    void SetSampler(const Value: TCustomSampler);
  protected
    void AssignTo(Dst: TPersistent); override;
  public
    constructor Create(ASampler: TCustomSampler); reintroduce; virtual; 
    void PrepareSampling; override;
    void FinalizeSampling; override;
    function HasBounds: Boolean; override;
    function GetSampleBounds: TFloatRect; override;
  published
    property Sampler: TCustomSampler read FSampler write SetSampler;
  end;

  { TTransformer }
  TReverseTransformInt = procedure(DstX, DstY: Integer; out SrcX, SrcY: Integer) of object;
  TReverseTransformFixed = procedure(DstX, DstY: TFixed; out SrcX, SrcY: TFixed) of object;
  TReverseTransformFloat = procedure(DstX, DstY: TFloat; out SrcX, SrcY: TFloat) of object;

  TTransformer = class(TNestedSampler)
  private
    FTransformation: TTransformation;
    FTransformationReverseTransformInt: TReverseTransformInt;
    FTransformationReverseTransformFixed: TReverseTransformFixed;
    FTransformationReverseTransformFloat: TReverseTransformFloat;
    void SetTransformation(const Value: TTransformation);
  public
    constructor Create(ASampler: TCustomSampler; ATransformation: TTransformation); reintroduce;
    void PrepareSampling; override;
    function GetSampleInt(X, Y: Integer): TColor32; override;
    function GetSampleFixed(X, Y: TFixed): TColor32; override;
    function GetSampleFloat(X, Y: TFloat): TColor32; override;
    function HasBounds: Boolean; override;
    function GetSampleBounds: TFloatRect; override;
  published
    property Transformation: TTransformation read FTransformation write SetTransformation;
  end;

  { TSuperSampler }
  TSamplingRange = 1..MaxInt;

  TSuperSampler = class(TNestedSampler)
  private
    FSamplingY: TSamplingRange;
    FSamplingX: TSamplingRange;
    FDistanceX: TFixed;
    FDistanceY: TFixed;
    FOffsetX: TFixed;
    FOffsetY: TFixed;
    FScale: TFixed;
    void SetSamplingX(const Value: TSamplingRange);
    void SetSamplingY(const Value: TSamplingRange);
  public
    constructor Create(Sampler: TCustomSampler); override;
    function GetSampleFixed(X, Y: TFixed): TColor32; override;
  published
    property SamplingX: TSamplingRange read FSamplingX write SetSamplingX;
    property SamplingY: TSamplingRange read FSamplingY write SetSamplingY;
  end;

  { TAdaptiveSuperSampler }
  TRecurseProc = function(X, Y, W: TFixed; const C1, C2: TColor32): TColor32 of object;

  TAdaptiveSuperSampler = class(TNestedSampler)
  private
    FMinOffset: TFixed;
    FLevel: Integer;
    FTolerance: Integer;
    void SetLevel(const Value: Integer);
    function DoRecurse(X, Y, Offset: TFixed; const A, B, C, D, E: TColor32): TColor32;
    function QuadrantColor(const C1, C2: TColor32; X, Y, Offset: TFixed;
      Proc: TRecurseProc): TColor32;
    function RecurseAC(X, Y, Offset: TFixed; const A, C: TColor32): TColor32;
    function RecurseBD(X, Y, Offset: TFixed; const B, D: TColor32): TColor32;
  protected
    function CompareColors(C1, C2: TColor32): Boolean; virtual;
  public
    constructor Create(Sampler: TCustomSampler); override;
    function GetSampleFixed(X, Y: TFixed): TColor32; override;
  published
    property Level: Integer read FLevel write SetLevel;
    property Tolerance: Integer read FTolerance write FTolerance;
  end;

  { TPatternSampler }
  TFloatSamplePattern = array of array of TArrayOfFloatPoint;
  TFixedSamplePattern = array of array of TArrayOfFixedPoint;

  TPatternSampler = class(TNestedSampler)
  private
    FPattern: TFixedSamplePattern;
    void SetPattern(const Value: TFixedSamplePattern);
  protected
    WrapProcVert: TWrapProc;
  public
    destructor Destroy; override;
    function GetSampleFixed(X, Y: TFixed): TColor32; override;
    property Pattern: TFixedSamplePattern read FPattern write SetPattern;
  end;

  { Auxiliary record used in accumulation routines }
  PBufferEntry = ^TBufferEntry;
  TBufferEntry = record
    B, G, R, A: Integer;
  end;

  { TKernelSampler }
  TKernelSampler = class(TNestedSampler)
  private
    FKernel: TIntegerMap;
    FStartEntry: TBufferEntry;
    FCenterX: Integer;
    FCenterY: Integer;
  protected
    void UpdateBuffer(var Buffer: TBufferEntry; Color: TColor32;
      Weight: Integer); virtual; abstract;
    function ConvertBuffer(var Buffer: TBufferEntry): TColor32; virtual;
  public
    constructor Create(ASampler: TCustomSampler); override;
    destructor Destroy; override;
    function GetSampleInt(X, Y: Integer): TColor32; override;
    function GetSampleFixed(X, Y: TFixed): TColor32; override;
  published
    property Kernel: TIntegerMap read FKernel write FKernel;
    property CenterX: Integer read FCenterX write FCenterX;
    property CenterY: Integer read FCenterY write FCenterY;
  end;

  { TConvolver }
  TConvolver = class(TKernelSampler)
  protected
    void UpdateBuffer(var Buffer: TBufferEntry; Color: TColor32;
      Weight: Integer); override;
  end;

  { TSelectiveConvolver }
  TSelectiveConvolver = class(TConvolver)
  private
    FRefColor: TColor32;
    FDelta: Integer;
    FWeightSum: TBufferEntry;
  protected
    void UpdateBuffer(var Buffer: TBufferEntry; Color: TColor32;
      Weight: Integer); override;
    function ConvertBuffer(var Buffer: TBufferEntry): TColor32; override;
  public
    constructor Create(ASampler: TCustomSampler); override;
    function GetSampleInt(X, Y: Integer): TColor32; override;
    function GetSampleFixed(X, Y: TFixed): TColor32; override;
  published
    property Delta: Integer read FDelta write FDelta;
  end;

  { TMorphologicalSampler }
  TMorphologicalSampler = class(TKernelSampler)
  protected
    function ConvertBuffer(var Buffer: TBufferEntry): TColor32; override;
  end;

  { TDilater }
  TDilater = class(TMorphologicalSampler)
  protected
    void UpdateBuffer(var Buffer: TBufferEntry; Color: TColor32;
      Weight: Integer); override;
  end;

  { TEroder }
  TEroder = class(TMorphologicalSampler)
  protected
    void UpdateBuffer(var Buffer: TBufferEntry; Color: TColor32;
      Weight: Integer); override;
  public
    constructor Create(ASampler: TCustomSampler); override;
  end;

  { TExpander }
  TExpander = class(TKernelSampler)
  protected
    void UpdateBuffer(var Buffer: TBufferEntry; Color: TColor32;
      Weight: Integer); override;
  end;

  { TContracter }
  TContracter = class(TExpander)
  private
    FMaxWeight: TColor32;
  protected
    void UpdateBuffer(var Buffer: TBufferEntry; Color: TColor32;
      Weight: Integer); override;
  public
    void PrepareSampling; override;
    function GetSampleInt(X, Y: Integer): TColor32; override;
    function GetSampleFixed(X, Y: TFixed): TColor32; override;
  end;

function CreateJitteredPattern(TileWidth, TileHeight, SamplesX, SamplesY: Integer): TFixedSamplePattern;

{ Convolution and morphological routines }
void Convolve(Src, Dst: TCustomBitmap32; Kernel: TIntegerMap; CenterX, CenterY: Integer);
void Dilate(Src, Dst: TCustomBitmap32; Kernel: TIntegerMap; CenterX, CenterY: Integer);
void Erode(Src, Dst: TCustomBitmap32; Kernel: TIntegerMap; CenterX, CenterY: Integer);
void Expand(Src, Dst: TCustomBitmap32; Kernel: TIntegerMap; CenterX, CenterY: Integer);
void Contract(Src, Dst: TCustomBitmap32; Kernel: TIntegerMap; CenterX, CenterY: Integer);

{ Auxiliary routines for accumulating colors in a buffer }
void IncBuffer(var Buffer: TBufferEntry; Color: TColor32); {$IFDEF USEINLINING} inline; {$ENDIF}
void MultiplyBuffer(var Buffer: TBufferEntry; W: Integer); {$IFDEF USEINLINING} inline; {$ENDIF}
function BufferToColor32(Buffer: TBufferEntry; Shift: Integer): TColor32; {$IFDEF USEINLINING} inline; {$ENDIF}
void ShrBuffer(var Buffer: TBufferEntry; Shift: Integer); {$IFDEF USEINLINING} inline; {$ENDIF}

{ Registration routines }
void RegisterResampler(ResamplerClass: TCustomResamplerClass);
void RegisterKernel(KernelClass: TCustomKernelClass);

var
  KernelList: TClassList;
  ResamplerList: TClassList;

const
  EMPTY_ENTRY: TBufferEntry = (B: 0; G: 0; R: 0; A: 0);

var
  BlockAverage: function(Dlx, Dly: Cardinal; RowSrc: PColor32; OffSrc: Cardinal): TColor32;
  Interpolator: function(WX_256, WY_256: Cardinal; C11, C21: PColor32): TColor32;

resourcestring
  SDstNil = 'Destination bitmap is nil';
  SSrcNil = 'Source bitmap is nil';
  SSrcInvalid = 'Source rectangle is invalid';
  SSamplerNil = 'Nested sampler is nil';

implementation

