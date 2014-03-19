//unit GR32_Polygons;
#pragma once

#include "GR32.h"
#include "GR32_LowLevel.h"
#include "GR32_Blend.h"
#include "GR32_Transforms.h"
#include "GR32_Resamplers.h"
#include "GR32_Math.h"


{ Polylines }

procedure PolylineTS(Bitmap: TCustomBitmap32; const Points: TArrayOfFixedPoint;
  Color: TColor32; Closed: Boolean = False; Transformation: TTransformation = nil);
procedure PolylineAS(Bitmap: TCustomBitmap32; const Points: TArrayOfFixedPoint;
  Color: TColor32; Closed: Boolean = False; Transformation: TTransformation = nil);
procedure PolylineXS(Bitmap: TCustomBitmap32; const Points: TArrayOfFixedPoint;
  Color: TColor32; Closed: Boolean = False; Transformation: TTransformation = nil);
procedure PolylineXSP(Bitmap: TCustomBitmap32; const Points: TArrayOfFixedPoint;
  Closed: Boolean = False; Transformation: TTransformation = nil);

procedure PolyPolylineTS(Bitmap: TCustomBitmap32; const Points: TArrayOfArrayOfFixedPoint;
  Color: TColor32; Closed: Boolean = False; Transformation: TTransformation = nil);
procedure PolyPolylineAS(Bitmap: TCustomBitmap32; const Points: TArrayOfArrayOfFixedPoint;
  Color: TColor32; Closed: Boolean = False; Transformation: TTransformation = nil);
procedure PolyPolylineXS(Bitmap: TCustomBitmap32; const Points: TArrayOfArrayOfFixedPoint;
  Color: TColor32; Closed: Boolean = False; Transformation: TTransformation = nil);
procedure PolyPolylineXSP(Bitmap: TCustomBitmap32; const Points: TArrayOfArrayOfFixedPoint;
  Closed: Boolean = False; Transformation: TTransformation = nil);

{ Polygons }

type
  TPolyFillMode = (pfAlternate, pfWinding);
  TAntialiasMode = (am32times, am16times, am8times, am4times, am2times, amNone);

  TFillLineEvent = procedure(Dst: PColor32; DstX, DstY, Length: Integer; AlphaValues: PColor32) of object;

  TCustomPolygonFiller = class
  protected
    function GetFillLine: TFillLineEvent; virtual; abstract;
  public
    property FillLine: TFillLineEvent read GetFillLine;
  end;

const
  DefaultAAMode = am8times; // Use 54 levels of transparency for antialiasing.

procedure PolygonTS(Bitmap: TCustomBitmap32; const Points: TArrayOfFixedPoint;
  Color: TColor32; Mode: TPolyFillMode = pfAlternate; Transformation: TTransformation = nil); overload;
procedure PolygonTS(Bitmap: TCustomBitmap32; const Points: TArrayOfFixedPoint;
  FillLineCallback: TFillLineEvent; Mode: TPolyFillMode = pfAlternate; Transformation: TTransformation = nil); overload;
procedure PolygonTS(Bitmap: TCustomBitmap32; const Points: TArrayOfFixedPoint;
  Filler: TCustomPolygonFiller; Mode: TPolyFillMode = pfAlternate; Transformation: TTransformation = nil); overload;

procedure PolygonXS(Bitmap: TCustomBitmap32; const Points: TArrayOfFixedPoint;
  Color: TColor32; Mode: TPolyFillMode = pfAlternate;
  const AAMode: TAntialiasMode = DefaultAAMode; Transformation: TTransformation = nil); overload;
procedure PolygonXS(Bitmap: TCustomBitmap32; const Points: TArrayOfFixedPoint;
  FillLineCallback: TFillLineEvent; Mode: TPolyFillMode = pfAlternate;
  const AAMode: TAntialiasMode = DefaultAAMode; Transformation: TTransformation = nil); overload;
procedure PolygonXS(Bitmap: TCustomBitmap32; const Points: TArrayOfFixedPoint;
  Filler: TCustomPolygonFiller; Mode: TPolyFillMode = pfAlternate;
  const AAMode: TAntialiasMode = DefaultAAMode; Transformation: TTransformation = nil); overload;

procedure PolyPolygonTS(Bitmap: TCustomBitmap32; const Points: TArrayOfArrayOfFixedPoint;
  Color: TColor32; Mode: TPolyFillMode = pfAlternate; Transformation: TTransformation = nil); overload;
procedure PolyPolygonTS(Bitmap: TCustomBitmap32; const Points: TArrayOfArrayOfFixedPoint;
  FillLineCallback: TFillLineEvent; Mode: TPolyFillMode = pfAlternate; Transformation: TTransformation = nil); overload;
procedure PolyPolygonTS(Bitmap: TCustomBitmap32; const Points: TArrayOfArrayOfFixedPoint;
  Filler: TCustomPolygonFiller; Mode: TPolyFillMode = pfAlternate; Transformation: TTransformation = nil); overload;

procedure PolyPolygonXS(Bitmap: TCustomBitmap32; const Points: TArrayOfArrayOfFixedPoint;
  FillLineCallback: TFillLineEvent; Mode: TPolyFillMode = pfAlternate;
  const AAMode: TAntialiasMode = DefaultAAMode; Transformation: TTransformation = nil); overload;
procedure PolyPolygonXS(Bitmap: TCustomBitmap32; const Points: TArrayOfArrayOfFixedPoint;
  Color: TColor32; Mode: TPolyFillMode = pfAlternate;
  const AAMode: TAntialiasMode = DefaultAAMode; Transformation: TTransformation = nil); overload;
procedure PolyPolygonXS(Bitmap: TCustomBitmap32; const Points: TArrayOfArrayOfFixedPoint;
  Filler: TCustomPolygonFiller; Mode: TPolyFillMode = pfAlternate;
  const AAMode: TAntialiasMode = DefaultAAMode; Transformation: TTransformation = nil); overload;

function PolygonBounds(const Points: TArrayOfFixedPoint; Transformation: TTransformation = nil): TFixedRect;
function PolyPolygonBounds(const Points: TArrayOfArrayOfFixedPoint; Transformation: TTransformation = nil): TFixedRect;

function PtInPolygon(const Pt: TFixedPoint; const Points: TArrayOfFixedPoint): Boolean;

{ TPolygon32 }
{ TODO : Bezier Curves, and QSpline curves for TrueType font rendering }
{ TODO : Check if QSpline is compatible with Type1 fonts }
type
  TPolygon32 = class(TThreadPersistent)
  private
    FAntialiased: Boolean;
    FClosed: Boolean;
    FFillMode: TPolyFillMode;
    FNormals: TArrayOfArrayOfFixedPoint;
    FPoints: TArrayOfArrayOfFixedPoint;
    FAntialiasMode: TAntialiasMode;
  protected
    procedure BuildNormals;
    procedure CopyPropertiesTo(Dst: TPolygon32); virtual;
    procedure AssignTo(Dst: TPersistent); override;
  public
    constructor Create; override;
    destructor Destroy; override;
    procedure Add(const P: TFixedPoint);
    procedure AddPoints(var First: TFixedPoint; Count: Integer);
    function  ContainsPoint(const P: TFixedPoint): Boolean;
    procedure Clear;
    function  Grow(const Delta: TFixed; EdgeSharpness: Single = 0): TPolygon32;

    procedure Draw(Bitmap: TCustomBitmap32; OutlineColor, FillColor: TColor32; Transformation: TTransformation = nil); overload;
    procedure Draw(Bitmap: TCustomBitmap32; OutlineColor: TColor32; FillCallback: TFillLineEvent; Transformation: TTransformation = nil); overload;
    procedure Draw(Bitmap: TCustomBitmap32; OutlineColor: TColor32; Filler: TCustomPolygonFiller; Transformation: TTransformation = nil); overload;

    procedure DrawEdge(Bitmap: TCustomBitmap32; Color: TColor32; Transformation: TTransformation = nil);

    procedure DrawFill(Bitmap: TCustomBitmap32; Color: TColor32; Transformation: TTransformation = nil); overload;
    procedure DrawFill(Bitmap: TCustomBitmap32; FillCallback: TFillLineEvent; Transformation: TTransformation = nil); overload;
    procedure DrawFill(Bitmap: TCustomBitmap32; Filler: TCustomPolygonFiller; Transformation: TTransformation = nil); overload;

    procedure NewLine;
    procedure Offset(const Dx, Dy: TFixed);
    function  Outline: TPolygon32;
    procedure Transform(Transformation: TTransformation);
    function GetBoundingRect: TFixedRect;

    property Antialiased: Boolean read FAntialiased write FAntialiased;
    property AntialiasMode: TAntialiasMode read FAntialiasMode write FAntialiasMode;
    property Closed: Boolean read FClosed write FClosed;
    property FillMode: TPolyFillMode read FFillMode write FFillMode;

    property Normals: TArrayOfArrayOfFixedPoint read FNormals write FNormals;
    property Points: TArrayOfArrayOfFixedPoint read FPoints write FPoints;
  end;

  { TBitmapPolygonFiller }
  TBitmapPolygonFiller = class(TCustomPolygonFiller)
  private
    FPattern: TCustomBitmap32;
    FOffsetY: Integer;
    FOffsetX: Integer;
  protected
    function GetFillLine: TFillLineEvent; override;
    procedure FillLineOpaque(Dst: PColor32; DstX, DstY, Length: Integer; AlphaValues: PColor32);
    procedure FillLineBlend(Dst: PColor32; DstX, DstY, Length: Integer; AlphaValues: PColor32);
    procedure FillLineBlendMasterAlpha(Dst: PColor32; DstX, DstY, Length: Integer; AlphaValues: PColor32);
    procedure FillLineCustomCombine(Dst: PColor32; DstX, DstY, Length: Integer; AlphaValues: PColor32);
  public
    property Pattern: TCustomBitmap32 read FPattern write FPattern;
    property OffsetX: Integer read FOffsetX write FOffsetX;
    property OffsetY: Integer read FOffsetY write FOffsetY;
  end;

  { TSamplerFiller }
  TSamplerFiller = class(TCustomPolygonFiller)
  private
    FSampler: TCustomSampler;
    FGetSample: TGetSampleInt;
    procedure SetSampler(const Value: TCustomSampler);
  protected
    function GetFillLine: TFillLineEvent; override;
    procedure SampleLineOpaque(Dst: PColor32; DstX, DstY, Length: Integer; AlphaValues: PColor32);

    property Sampler: TCustomSampler read FSampler write SetSampler;
  end;
    
implementation
