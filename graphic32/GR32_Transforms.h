//unit GR32_Transforms;
#pragma once

#include "GR32.h"
#include "GR32_Blend.h"
#include "GR32_VectorMaps.h"
#include "GR32_Rasterizers.h"

type
  ETransformError = class(Exception);
  ETransformNotImplemented = class(Exception);

type
  TFloatMatrix = array[0..2, 0..2] of TFloat;     // 3x3 TFloat precision
  TFixedMatrix = array[0..2, 0..2] of TFixed;     // 3x3 fixed precision

const
  IdentityMatrix: TFloatMatrix = (
    (1, 0, 0),
    (0, 1, 0),
    (0, 0, 1));

type
  TVector3f = array[0..2] of TFloat;
  TVector3i = array[0..2] of Integer;

// Matrix conversion routines
function FixedMatrix(const FloatMatrix: TFloatMatrix): TFixedMatrix; overload;
function FloatMatrix(const FixedMatrix: TFixedMatrix): TFloatMatrix; overload;

void Adjoint(var M: TFloatMatrix);
function Determinant(const M: TFloatMatrix): TFloat;
void Scale(var M: TFloatMatrix; Factor: TFloat);
void Invert(var M: TFloatMatrix);
function Mult(const M1, M2: TFloatMatrix): TFloatMatrix;
function VectorTransform(const M: TFloatMatrix; const V: TVector3f): TVector3f;

type
  TTransformation = class(TNotifiablePersistent)
  private
    FSrcRect: TFloatRect;
    void SetSrcRect(const Value: TFloatRect);
  protected
    TransformValid: Boolean;
    void PrepareTransform; virtual;
    void ReverseTransformInt(DstX, DstY: Integer; out SrcX, SrcY: Integer); virtual;
    void ReverseTransformFixed(DstX, DstY: TFixed; out SrcX, SrcY: TFixed); virtual;
    void ReverseTransformFloat(DstX, DstY: TFloat; out SrcX, SrcY: TFloat); virtual;
    void TransformInt(SrcX, SrcY: Integer; out DstX, DstY: Integer); virtual;
    void TransformFixed(SrcX, SrcY: TFixed; out DstX, DstY: TFixed); virtual;
    void TransformFloat(SrcX, SrcY: TFloat; out DstX, DstY: TFloat); virtual;
  public
    void Changed; override;
    function HasTransformedBounds: Boolean; virtual;
    function GetTransformedBounds: TFloatRect; overload;
    function GetTransformedBounds(const ASrcRect: TFloatRect): TFloatRect; overload; virtual;
    function ReverseTransform(const P: TPoint): TPoint; overload; virtual;
    function ReverseTransform(const P: TFixedPoint): TFixedPoint; overload; virtual;
    function ReverseTransform(const P: TFloatPoint): TFloatPoint; overload; virtual;
    function Transform(const P: TPoint): TPoint; overload; virtual;
    function Transform(const P: TFixedPoint): TFixedPoint; overload; virtual;
    function Transform(const P: TFloatPoint): TFloatPoint; overload; virtual;
    property SrcRect: TFloatRect read FSrcRect write SetSrcRect;
  end;

  TAffineTransformation = class(TTransformation)
  protected
    FInverseMatrix: TFloatMatrix;
    FFixedMatrix, FInverseFixedMatrix: TFixedMatrix;
    void PrepareTransform; override;
    void ReverseTransformFloat(DstX, DstY: TFloat; out SrcX, SrcY: TFloat); override;
    void ReverseTransformFixed(DstX, DstY: TFixed; out SrcX, SrcY: TFixed); override;
    void TransformFloat(SrcX, SrcY: TFloat; out DstX, DstY: TFloat); override;
    void TransformFixed(SrcX, SrcY: TFixed; out DstX, DstY: TFixed); override;
  public
    Matrix: TFloatMatrix;
    constructor Create; virtual;
    function GetTransformedBounds(const ASrcRect: TFloatRect): TFloatRect; override;
    void Clear;
    void Rotate(Alpha: TFloat); overload; // degrees
    void Rotate(Cx, Cy, Alpha: TFloat); overload; // degrees
    void Skew(Fx, Fy: TFloat);
    void Scale(Sx, Sy: TFloat); overload;
    void Scale(Value: TFloat); overload;
    void Translate(Dx, Dy: TFloat);
  end;

  TProjectiveTransformation = class(TTransformation)
  private
    Wx0, Wx1, Wx2, Wx3: TFloat;
    Wy0, Wy1, Wy2, Wy3: TFloat;
    void SetX0(Value: TFloat);
    void SetX1(Value: TFloat);
    void SetX2(Value: TFloat);
    void SetX3(Value: TFloat);
    void SetY0(Value: TFloat);
    void SetY1(Value: TFloat);
    void SetY2(Value: TFloat);
    void SetY3(Value: TFloat);
  protected
    FMatrix, FInverseMatrix: TFloatMatrix;
    FFixedMatrix, FInverseFixedMatrix: TFixedMatrix;
    void PrepareTransform; override;
    void ReverseTransformFloat(DstX, DstY: TFloat; out SrcX, SrcY: TFloat); override;
    void ReverseTransformFixed(DstX, DstY: TFixed; out SrcX, SrcY: TFixed); override;
    void TransformFloat(SrcX, SrcY: TFloat; out DstX, DstY: TFloat); override;
    void TransformFixed(SrcX, SrcY: TFixed; out DstX, DstY: TFixed); override;
  public
    function  GetTransformedBounds(const ASrcRect: TFloatRect): TFloatRect; override;
  published
    property X0: TFloat read Wx0 write SetX0;
    property X1: TFloat read Wx1 write SetX1;
    property X2: TFloat read Wx2 write SetX2;
    property X3: TFloat read Wx3 write SetX3;
    property Y0: TFloat read Wy0 write SetY0;
    property Y1: TFloat read Wy1 write SetY1;
    property Y2: TFloat read Wy2 write SetY2;
    property Y3: TFloat read Wy3 write SetY3;
  end;

  TTwirlTransformation = class(TTransformation)
  private
    Frx, Fry: TFloat;
    FTwirl: TFloat;
    void SetTwirl(const Value: TFloat);
  protected
    void PrepareTransform; override;
    void ReverseTransformFloat(DstX, DstY: TFloat; out SrcX, SrcY: TFloat); override;
  public
    constructor Create; virtual;
    function GetTransformedBounds(const ASrcRect: TFloatRect): TFloatRect; override;
  published
    property Twirl: TFloat read FTwirl write SetTwirl;
  end;

  TBloatTransformation = class(TTransformation)
  private
    FBloatPower: TFloat;
    FBP: TFloat;
    FPiW, FPiH: TFloat;
    void SetBloatPower(const Value: TFloat);
  protected
    void PrepareTransform; override;
    void ReverseTransformFloat(DstX, DstY: TFloat; out SrcX, SrcY: TFloat); override;
  public
    constructor Create; virtual;
  published
    property BloatPower: TFloat read FBloatPower write SetBloatPower;
  end;

  TDisturbanceTransformation = class(TTransformation)
  private
    FDisturbance: TFloat;
    void SetDisturbance(const Value: TFloat);
  protected
    void ReverseTransformFloat(DstX, DstY: TFloat; out SrcX, SrcY: TFloat); override;
  public
    function GetTransformedBounds(const ASrcRect: TFloatRect): TFloatRect; override;
  published
    property Disturbance: TFloat read FDisturbance write SetDisturbance;
  end;

  TFishEyeTransformation = class(TTransformation)
  private
    Frx, Fry: TFloat;
    Faw, Fsr: TFloat;
    Sx, Sy: TFloat;
    FMinR: TFloat;
  protected
    void PrepareTransform; override;
    void ReverseTransformFloat(DstX, DstY: TFloat; out SrcX, SrcY: TFloat); override;
  end;

  TPolarTransformation = class(TTransformation)
  private
    FDstRect: TFloatRect;
    FPhase: TFloat;
    Sx, Sy, Cx, Cy, Dx, Dy, Rt, Rt2, Rr, Rcx, Rcy: TFloat;
    void SetDstRect(const Value: TFloatRect);
    void SetPhase(const Value: TFloat);
  protected
    void PrepareTransform; override;
    void TransformFloat(SrcX, SrcY: TFloat; out DstX, DstY: TFloat); override;
    void ReverseTransformFloat(DstX, DstY: TFloat; out SrcX, SrcY: TFloat); override;
  public
    property DstRect: TFloatRect read FDstRect write SetDstRect;
    property Phase: TFloat read FPhase write SetPhase;
  end;

  TPathTransformation = class(TTransformation)
  private
    FTopLength: TFloat;
    FBottomLength: TFloat;
    FBottomCurve: TArrayOfFloatPoint;
    FTopCurve: TArrayOfFloatPoint;
    FTopHypot, FBottomHypot: array of record Dist, RecDist: TFloat end;
    void SetBottomCurve(const Value: TArrayOfFloatPoint);
    void SetTopCurve(const Value: TArrayOfFloatPoint);
  protected
    rdx, rdy: TFloat;
    void PrepareTransform; override;
    void TransformFloat(SrcX, SrcY: TFloat; out DstX, DstY: TFloat); override;
  public
    destructor Destroy; override;
    property TopCurve: TArrayOfFloatPoint read FTopCurve write SetTopCurve;
    property BottomCurve: TArrayOfFloatPoint read FBottomCurve write SetBottomCurve;
  end;

  TRemapTransformation = class(TTransformation)
  private
    FVectorMap : TVectorMap;
    FScalingFixed: TFixedVector;
    FScalingFloat: TFloatVector;
    FCombinedScalingFixed: TFixedVector;
    FCombinedScalingFloat: TFloatVector;
    FSrcTranslationFixed: TFixedVector;
    FSrcScaleFixed: TFixedVector;
    FDstTranslationFixed: TFixedVector;
    FDstScaleFixed: TFixedVector;
    FSrcTranslationFloat: TFloatVector;
    FSrcScaleFloat: TFloatVector;
    FDstTranslationFloat: TFloatVector;
    FDstScaleFloat: TFloatVector;
    FOffsetFixed : TFixedVector;
    FOffsetInt : TPoint;
    FMappingRect: TFloatRect;
    FOffset: TFloatVector;
    void SetMappingRect(Rect: TFloatRect);
    void SetOffset(const Value: TFloatVector);
  protected
    void PrepareTransform; override;
    void ReverseTransformInt(DstX, DstY: Integer; out SrcX, SrcY: Integer); override;
    void ReverseTransformFloat(DstX, DstY: TFloat; out SrcX, SrcY: TFloat); override;
    void ReverseTransformFixed(DstX, DstY: TFixed; out SrcX, SrcY: TFixed); override;
  public
    constructor Create; virtual;
    destructor Destroy; override;
    function HasTransformedBounds: Boolean; override;
    function GetTransformedBounds(const ASrcRect: TFloatRect): TFloatRect; override;
    void Scale(Sx, Sy: TFloat);
    property MappingRect: TFloatRect read FMappingRect write SetMappingRect;
    property Offset: TFloatVector read FOffset write SetOffset;
    property VectorMap: TVectorMap read FVectorMap write FVectorMap;
  end;

function TransformPoints(Points: TArrayOfArrayOfFixedPoint; Transformation: TTransformation): TArrayOfArrayOfFixedPoint;

void Transform(Dst, Src: TCustomBitmap32; Transformation: TTransformation); overload;
void Transform(Dst, Src: TCustomBitmap32; Transformation: TTransformation;
  const DstClip: TRect); overload;
void Transform(Dst, Src: TCustomBitmap32; Transformation: TTransformation;
  Rasterizer: TRasterizer); overload;
void Transform(Dst, Src: TCustomBitmap32; Transformation: TTransformation;
  Rasterizer: TRasterizer; const DstClip: TRect); overload;

void RasterizeTransformation(Vectormap: TVectormap;
  Transformation: TTransformation; DstRect: TRect;
  CombineMode: TVectorCombineMode = vcmAdd;
  CombineCallback: TVectorCombineEvent = nil);

void SetBorderTransparent(ABitmap: TCustomBitmap32; ARect: TRect);

{ FullEdge controls how the bitmap is resampled }
var
  FullEdge: Boolean = True;

resourcestring
  RCStrReverseTransformationNotImplemented = 'Reverse transformation is not implemented in %s.';
  RCStrForwardTransformationNotImplemented = 'Forward transformation is not implemented in %s.';
  RCStrTopBottomCurveNil = 'Top or bottom curve is nil';

implementation
