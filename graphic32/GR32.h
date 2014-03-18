//unit GR32;
#pragma once

#include "GR32_System.h"

const WCHAR* Graphics32Version = L"1.9.1";

typedef DWORD		TColor32;
typedef TColor32*	PColor32;

type

  PColor32Array = ^TColor32Array;
  TColor32Array = array [0..0] of TColor32;
  TArrayOfColor32 = array of TColor32;


  TColor32Component = (ccBlue, ccGreen, ccRed, ccAlpha);
  TColor32Components = set of TColor32Component;

  PColor32Entry = ^TColor32Entry;
  TColor32Entry = packed record
    case Integer of
      0: (B, G, R, A: Byte);
      1: (ARGB: TColor32);
      2: (Planes: array[0..3] of Byte);
      3: (Components: array[TColor32Component] of Byte);
  end;

  PColor32EntryArray = ^TColor32EntryArray;
  TColor32EntryArray = array [0..0] of TColor32Entry;
  TArrayOfColor32Entry = array of TColor32Entry;

  PPalette32 = ^TPalette32;
  TPalette32 = array [Byte] of TColor32;

const
  // Some predefined color constants
const TColor32 clBlack32		= 0xFF000000;
const TColor32 clDimGray32		= 0xFF3F3F3F;
const TColor32 clGray32			= 0xFF7F7F7F;
const TColor32 clLightGray32	= 0xFFBFBFBF;
const TColor32 clWhite32		= 0xFFFFFFFF;
const TColor32 clMaroon32		= 0xFF7F0000;
const TColor32 clGreen32		= 0xFF007F00;
const TColor32 clOlive32		= 0xFF7F7F00;
const TColor32 clNavy32			= 0xFF00007F;
const TColor32 clPurple32		= 0xFF7F007F;
const TColor32 clTeal32			= 0xFF007F7F;
const TColor32 clRed32			= 0xFFFF0000;
const TColor32 clLime32			= 0xFF00FF00;
const TColor32 clYellow32              = 0xFFFFFF00;
const TColor32 clBlue32                = 0xFF0000FF;
const TColor32 clFuchsia32             = 0xFFFF00FF;
const TColor32 clAqua32                = 0xFF00FFFF;

const TColor32 clAliceBlue32           = 0xFFF0F8FF;
const TColor32 clAntiqueWhite32        = 0xFFFAEBD7;
const TColor32 clAquamarine32          = 0xFF7FFFD4;
const TColor32 clAzure32               = 0xFFF0FFFF;
const TColor32 clBeige32               = 0xFFF5F5DC;
const TColor32 clBisque32              = 0xFFFFE4C4;
const TColor32 clBlancheDalmond32      = 0xFFFFEBCD;
const TColor32 clBlueViolet32          = 0xFF8A2BE2;
const TColor32 clBrown32               = 0xFFA52A2A;
const TColor32 clBurlyWood32           = 0xFFDEB887;
const TColor32 clCadetblue32           = 0xFF5F9EA0;
const TColor32 clChartReuse32          = 0xFF7FFF00;
const TColor32 clChocolate32           = 0xFFD2691E;
const TColor32 clCoral32               = 0xFFFF7F50;
const TColor32 clCornFlowerBlue32      = 0xFF6495ED;
const TColor32 clCornSilk32            = 0xFFFFF8DC;
const TColor32 clCrimson32             = 0xFFDC143C;
const TColor32 clDarkBlue32            = 0xFF00008B;
const TColor32 clDarkCyan32            = 0xFF008B8B;
const TColor32 clDarkGoldenRod32       = 0xFFB8860B;
const TColor32 clDarkGray32            = 0xFFA9A9A9;
const TColor32 clDarkGreen32           = 0xFF006400;
const TColor32 clDarkGrey32            = 0xFFA9A9A9;
const TColor32 clDarkKhaki32           = 0xFFBDB76B;
const TColor32 clDarkMagenta32         = 0xFF8B008B;
const TColor32 clDarkOliveGreen32      = 0xFF556B2F;
const TColor32 clDarkOrange32          = 0xFFFF8C00;
const TColor32 clDarkOrchid32          = 0xFF9932CC;
const TColor32 clDarkRed32             = 0xFF8B0000;
const TColor32 clDarkSalmon32          = 0xFFE9967A;
const TColor32 clDarkSeaGreen32        = 0xFF8FBC8F;
const TColor32 clDarkSlateBlue32       = 0xFF483D8B;
const TColor32 clDarkSlateGray32       = 0xFF2F4F4F;
const TColor32 clDarkSlateGrey32       = 0xFF2F4F4F;
const TColor32 clDarkTurquoise32       = 0xFF00CED1;
const TColor32 clDarkViolet32          = 0xFF9400D3;
const TColor32 clDeepPink32            = 0xFFFF1493;
const TColor32 clDeepSkyBlue32         = 0xFF00BFFF;
const TColor32 clDodgerBlue32          = 0xFF1E90FF;
const TColor32 clFireBrick32           = 0xFFB22222;
const TColor32 clFloralWhite32         = 0xFFFFFAF0;
const TColor32 clGainsBoro32           = 0xFFDCDCDC;
const TColor32 clGhostWhite32          = 0xFFF8F8FF;
const TColor32 clGold32                = 0xFFFFD700;
const TColor32 clGoldenRod32           = 0xFFDAA520;
const TColor32 clGreenYellow32         = 0xFFADFF2F;
const TColor32 clGrey32                = 0xFF808080;
const TColor32 clHoneyDew32            = 0xFFF0FFF0;
const TColor32 clHotPink32             = 0xFFFF69B4;
const TColor32 clIndianRed32           = 0xFFCD5C5C;
const TColor32 clIndigo32              = 0xFF4B0082;
const TColor32 clIvory32               = 0xFFFFFFF0;
const TColor32 clKhaki32               = 0xFFF0E68C;
const TColor32 clLavender32            = 0xFFE6E6FA;
const TColor32 clLavenderBlush32       = 0xFFFFF0F5;
const TColor32 clLawnGreen32           = 0xFF7CFC00;
const TColor32 clLemonChiffon32        = 0xFFFFFACD;
const TColor32 clLightBlue32           = 0xFFADD8E6;
const TColor32 clLightCoral32          = 0xFFF08080;
const TColor32 clLightCyan32           = 0xFFE0FFFF;
const TColor32 clLightGoldenRodYellow32= 0xFFFAFAD2;
const TColor32 clLightGreen32          = 0xFF90EE90;
const TColor32 clLightGrey32           = 0xFFD3D3D3;
const TColor32 clLightPink32           = 0xFFFFB6C1;
const TColor32 clLightSalmon32         = 0xFFFFA07A;
const TColor32 clLightSeagreen32       = 0xFF20B2AA;
const TColor32 clLightSkyblue32        = 0xFF87CEFA;
const TColor32 clLightSlategray32      = 0xFF778899;
const TColor32 clLightSlategrey32      = 0xFF778899;
const TColor32 clLightSteelblue32      = 0xFFB0C4DE;
const TColor32 clLightYellow32         = 0xFFFFFFE0;
const TColor32 clLtGray32              = 0xFFC0C0C0;
const TColor32 clMedGray32             = 0xFFA0A0A4;
const TColor32 clDkGray32              = 0xFF808080;
const TColor32 clMoneyGreen32          = 0xFFC0DCC0;
const TColor32 clLegacySkyBlue32       = 0xFFA6CAF0;
const TColor32 clCream32               = 0xFFFFFBF0;
const TColor32 clLimeGreen32           = 0xFF32CD32;
const TColor32 clLinen32               = 0xFFFAF0E6;
const TColor32 clMediumAquamarine32    = 0xFF66CDAA;
const TColor32 clMediumBlue32          = 0xFF0000CD;
const TColor32 clMediumOrchid32        = 0xFFBA55D3;
const TColor32 clMediumPurple32        = 0xFF9370DB;
const TColor32 clMediumSeaGreen32      = 0xFF3CB371;
const TColor32 clMediumSlateBlue32     = 0xFF7B68EE;
const TColor32 clMediumSpringGreen32   = 0xFF00FA9A;
const TColor32 clMediumTurquoise32     = 0xFF48D1CC;
const TColor32 clMediumVioletRed32     = 0xFFC71585;
const TColor32 clMidnightBlue32        = 0xFF191970;
const TColor32 clMintCream32           = 0xFFF5FFFA;
const TColor32 clMistyRose32           = 0xFFFFE4E1;
const TColor32 clMoccasin32            = 0xFFFFE4B5;
const TColor32 clNavajoWhite32         = 0xFFFFDEAD;
const TColor32 clOldLace32             = 0xFFFDF5E6;
const TColor32 clOliveDrab32           = 0xFF6B8E23;
const TColor32 clOrange32              = 0xFFFFA500;
const TColor32 clOrangeRed32           = 0xFFFF4500;
const TColor32 clOrchid32              = 0xFFDA70D6;
const TColor32 clPaleGoldenRod32       = 0xFFEEE8AA;
const TColor32 clPaleGreen32           = 0xFF98FB98;
const TColor32 clPaleTurquoise32       = 0xFFAFEEEE;
const TColor32 clPaleVioletred32       = 0xFFDB7093;
const TColor32 clPapayaWhip32          = 0xFFFFEFD5;
const TColor32 clPeachPuff32           = 0xFFFFDAB9;
const TColor32 clPeru32                = 0xFFCD853F;
const TColor32 clPlum32                = 0xFFDDA0DD;
const TColor32 clPowderBlue32          = 0xFFB0E0E6;
const TColor32 clRosyBrown32           = 0xFFBC8F8F;
const TColor32 clRoyalBlue32           = 0xFF4169E1;
const TColor32 clSaddleBrown32         = 0xFF8B4513;
const TColor32 clSalmon32              = 0xFFFA8072;
const TColor32 clSandyBrown32          = 0xFFF4A460;
const TColor32 clSeaGreen32            = 0xFF2E8B57;
const TColor32 clSeaShell32            = 0xFFFFF5EE;
const TColor32 clSienna32              = 0xFFA0522D;
const TColor32 clSilver32              = 0xFFC0C0C0;
const TColor32 clSkyblue32             = 0xFF87CEEB;
const TColor32 clSlateBlue32           = 0xFF6A5ACD;
const TColor32 clSlateGray32           = 0xFF708090;
const TColor32 clSlateGrey32           = 0xFF708090;
const TColor32 clSnow32                = 0xFFFFFAFA;
const TColor32 clSpringgreen32         = 0xFF00FF7F;
const TColor32 clSteelblue32           = 0xFF4682B4;
const TColor32 clTan32                 = 0xFFD2B48C;
const TColor32 clThistle32             = 0xFFD8BFD8;
const TColor32 clTomato32              = 0xFFFF6347;
const TColor32 clTurquoise32           = 0xFF40E0D0;
const TColor32 clViolet32              = 0xFFEE82EE;
const TColor32 clWheat32               = 0xFFF5DEB3;
const TColor32 clWhitesmoke32          = 0xFFF5F5F5;
const TColor32 clYellowgreen32         = 0xFF9ACD32;

  // Some semi-transparent color constants
const TColor32 clTrWhite32             = 0x7FFFFFFF;
const TColor32 clTrBlack32             = 0x7F000000;
const TColor32 clTrRed32               = 0x7FFF0000;
const TColor32 clTrGreen32             = 0x7F00FF00;
const TColor32 clTrBlue32              = 0x7F0000FF;

// Color construction and conversion functions
function Color32(WinColor: TColor): TColor32; overload;
function Color32(R, G, B: Byte; A: Byte = $FF): TColor32; overload;
function Color32(Index: Byte; var Palette: TPalette32): TColor32; overload;
function Gray32(Intensity: Byte; Alpha: Byte = $FF): TColor32; {$IFDEF USEINLINING} inline; {$ENDIF}
function WinColor(Color32: TColor32): TColor;
function ArrayOfColor32(Colors: array of TColor32): TArrayOfColor32;

// Color component access
procedure Color32ToRGB(Color32: TColor32; var R, G, B: Byte);
procedure Color32ToRGBA(Color32: TColor32; var R, G, B, A: Byte);
function Color32Components(R, G, B, A: Boolean): TColor32Components;
function RedComponent(Color32: TColor32): Integer; {$IFDEF USEINLINING} inline; {$ENDIF}
function GreenComponent(Color32: TColor32): Integer; {$IFDEF USEINLINING} inline; {$ENDIF}
function BlueComponent(Color32: TColor32): Integer; {$IFDEF USEINLINING} inline; {$ENDIF}
function AlphaComponent(Color32: TColor32): Integer; {$IFDEF USEINLINING} inline; {$ENDIF}
function Intensity(Color32: TColor32): Integer; {$IFDEF USEINLINING} inline; {$ENDIF}
function SetAlpha(Color32: TColor32; NewAlpha: Integer): TColor32; {$IFDEF USEINLINING} inline; {$ENDIF}

// Color space conversion
function HSLtoRGB(H, S, L: Single): TColor32; overload;
procedure RGBtoHSL(RGB: TColor32; out H, S, L : Single); overload;
function HSLtoRGB(H, S, L: Integer): TColor32; overload;
procedure RGBtoHSL(RGB: TColor32; out H, S, L: Byte); overload;

{$IFNDEF PLATFORM_INDEPENDENT}
// Palette conversion functions
function WinPalette(const P: TPalette32): HPALETTE;
{$ENDIF}

//{ A fixed-point type }

type
  // This type has data bits arrangement compatible with Windows.TFixed
typedef int TFixed;
typedef TFixed* PFixed;

  PFixedRec = ^TFixedRec;
  TFixedRec = packed record
    case Integer of
      0: (Fixed: TFixed);
      1: (Frac: Word; Int: SmallInt);
  end;

  PFixedArray = ^TFixedArray;
  TFixedArray = array [0..0] of TFixed;
  PArrayOfFixed = ^TArrayOfFixed;
  TArrayOfFixed = array of TFixed;
  PArrayOfArrayOfFixed = ^TArrayOfArrayOfFixed;
  TArrayOfArrayOfFixed = array of TArrayOfFixed;

  // TFloat determines the precision level for certain floating-point operations
  PFloat = ^TFloat;
  TFloat = Single;

{ Other dynamic arrays }
type
  PByteArray = ^TByteArray;
  TByteArray = array [0..0] of Byte;
  PArrayOfByte = ^TArrayOfByte;
  TArrayOfByte = array of Byte;

  PWordArray = ^TWordArray;
  TWordArray = array [0..0] of Word;
  PArrayOfWord = ^TArrayOfWord;
  TArrayOfWord = array of Word;

  PIntegerArray = ^TIntegerArray;
  TIntegerArray = array [0..0] of Integer;
  PArrayOfInteger = ^TArrayOfInteger;
  TArrayOfInteger = array of Integer;
  PArrayOfArrayOfInteger = ^TArrayOfArrayOfInteger;
  TArrayOfArrayOfInteger = array of TArrayOfInteger;

  PSingleArray = ^TSingleArray;
  TSingleArray = array [0..0] of Single;
  PArrayOfSingle = ^TArrayOfSingle;
  TArrayOfSingle = array of Single;

  PFloatArray = ^TFloatArray;
  TFloatArray = array [0..0] of TFloat;
  PArrayOfFloat = ^TArrayOfFloat;
  TArrayOfFloat = array of TFloat;

const
  // Fixed point math constants
const int FixedOne = 0x10000;
const int FixedHalf = 0x7FFF;
  FixedPI  = Round(PI * FixedOne);
  FixedToFloat = 1/FixedOne;

function Fixed(S: Single): TFixed; overload; {$IFDEF USEINLINING} inline; {$ENDIF}
function Fixed(I: Integer): TFixed; overload; {$IFDEF USEINLINING} inline; {$ENDIF}

//{ Points }

type
{$IFNDEF FPC}
{$IFNDEF BCB}
  PPoint = ^TPoint;
  TPoint = Windows.TPoint;
{$ENDIF}
{$ENDIF}

  PPointArray = ^TPointArray;
  TPointArray = array [0..0] of TPoint;
  PArrayOfPoint = ^TArrayOfPoint;
  TArrayOfPoint = array of TPoint;
  PArrayOfArrayOfPoint = ^TArrayOfArrayOfPoint;
  TArrayOfArrayOfPoint = array of TArrayOfPoint;

  PFloatPoint = ^TFloatPoint;
  TFloatPoint = record
    X, Y: TFloat;
  end;

  PFloatPointArray = ^TFloatPointArray;
  TFloatPointArray = array [0..0] of TFloatPoint;
  PArrayOfFloatPoint = ^TArrayOfFloatPoint;
  TArrayOfFloatPoint = array of TFloatPoint;
  PArrayOfArrayOfFloatPoint = ^TArrayOfArrayOfFloatPoint;
  TArrayOfArrayOfFloatPoint = array of TArrayOfFloatPoint;

  PFixedPoint = ^TFixedPoint;
  TFixedPoint = record
    X, Y: TFixed;
  end;

  PFixedPointArray = ^TFixedPointArray;
  TFixedPointArray = array [0..0] of TFixedPoint;
  PArrayOfFixedPoint = ^TArrayOfFixedPoint;
  TArrayOfFixedPoint = array of TFixedPoint;
  PArrayOfArrayOfFixedPoint = ^TArrayOfArrayOfFixedPoint;
  TArrayOfArrayOfFixedPoint = array of TArrayOfFixedPoint;

// construction and conversion of point types
function Point(X, Y: Integer): TPoint; overload; {$IFDEF USEINLINING} inline; {$ENDIF}
function Point(const FP: TFloatPoint): TPoint; overload; {$IFDEF USEINLINING} inline; {$ENDIF}
function Point(const FXP: TFixedPoint): TPoint; overload; {$IFDEF USEINLINING} inline; {$ENDIF}
function FloatPoint(X, Y: Single): TFloatPoint; overload; {$IFDEF USEINLINING} inline; {$ENDIF}
function FloatPoint(const P: TPoint): TFloatPoint; overload; {$IFDEF USEINLINING} inline; {$ENDIF}
function FloatPoint(const FXP: TFixedPoint): TFloatPoint; overload; {$IFDEF USEINLINING} inline; {$ENDIF}
function FixedPoint(X, Y: Integer): TFixedPoint; overload; {$IFDEF USEINLINING} inline; {$ENDIF}
function FixedPoint(X, Y: Single): TFixedPoint; overload; {$IFDEF USEINLINING} inline; {$ENDIF}
function FixedPoint(const P: TPoint): TFixedPoint; overload; {$IFDEF USEINLINING} inline; {$ENDIF}
function FixedPoint(const FP: TFloatPoint): TFixedPoint; overload; {$IFDEF USEINLINING} inline; {$ENDIF}

{ Rectangles }

type

typedef RECT TRect;
typedef PRECT PRect;

  PFloatRect = ^TFloatRect;
  TFloatRect = packed record
    case Integer of
      0: (Left, Top, Right, Bottom: TFloat);
      1: (TopLeft, BottomRight: TFloatPoint);
  end;

  PFixedRect = ^TFixedRect;
  TFixedRect = packed record
    case Integer of
      0: (Left, Top, Right, Bottom: TFixed);
      1: (TopLeft, BottomRight: TFixedPoint);
  end;

  TRectRounding = (rrClosest, rrOutside, rrInside);

// Rectangle construction/conversion functions
function MakeRect(const L, T, R, B: Integer): TRect; overload; {$IFDEF USEINLINING} inline; {$ENDIF}
function MakeRect(const FR: TFloatRect; Rounding: TRectRounding = rrClosest): TRect; overload;
function MakeRect(const FXR: TFixedRect; Rounding: TRectRounding = rrClosest): TRect; overload;
function FixedRect(const L, T, R, B: TFixed): TFixedRect; overload; {$IFDEF USEINLINING} inline; {$ENDIF}
function FixedRect(const ARect: TRect): TFixedRect; overload; {$IFDEF USEINLINING} inline; {$ENDIF}
function FixedRect(const FR: TFloatRect): TFixedRect; overload; {$IFDEF USEINLINING} inline; {$ENDIF}
function FloatRect(const L, T, R, B: TFloat): TFloatRect; overload; {$IFDEF USEINLINING} inline; {$ENDIF}
function FloatRect(const ARect: TRect): TFloatRect; overload; {$IFDEF USEINLINING} inline; {$ENDIF}
function FloatRect(const FXR: TFixedRect): TFloatRect; overload; {$IFDEF USEINLINING} inline; {$ENDIF}

// Some basic operations over rectangles
function IntersectRect(out Dst: TRect; const R1, R2: TRect): Boolean; overload;
function IntersectRect(out Dst: TFloatRect; const FR1, FR2: TFloatRect): Boolean; overload;
function UnionRect(out Rect: TRect; const R1, R2: TRect): Boolean; overload;
function UnionRect(out Rect: TFloatRect; const R1, R2: TFloatRect): Boolean; overload;
function EqualRect(const R1, R2: TRect): Boolean; overload; {$IFDEF USEINLINING} inline; {$ENDIF}
function EqualRect(const R1, R2: TFloatRect): Boolean; overload; {$IFDEF USEINLINING} inline; {$ENDIF}
procedure InflateRect(var R: TRect; Dx, Dy: Integer); overload; {$IFDEF USEINLINING} inline; {$ENDIF}
procedure InflateRect(var FR: TFloatRect; Dx, Dy: TFloat); overload; {$IFDEF USEINLINING} inline; {$ENDIF}
procedure OffsetRect(var R: TRect; Dx, Dy: Integer); overload; {$IFDEF USEINLINING} inline; {$ENDIF}
procedure OffsetRect(var FR: TFloatRect; Dx, Dy: TFloat); overload; {$IFDEF USEINLINING} inline; {$ENDIF}
function IsRectEmpty(const R: TRect): Boolean; overload; {$IFDEF USEINLINING} inline; {$ENDIF}
function IsRectEmpty(const FR: TFloatRect): Boolean; overload; {$IFDEF USEINLINING} inline; {$ENDIF}
function PtInRect(const R: TRect; const P: TPoint): Boolean; overload; {$IFDEF USEINLINING} inline; {$ENDIF}
function PtInRect(const R: TFloatRect; const P: TPoint): Boolean; overload; {$IFDEF USEINLINING} inline; {$ENDIF}
function PtInRect(const R: TRect; const P: TFloatPoint): Boolean; overload; {$IFDEF USEINLINING} inline; {$ENDIF}
function PtInRect(const R: TFloatRect; const P: TFloatPoint): Boolean; overload; {$IFDEF USEINLINING} inline; {$ENDIF}
function EqualRectSize(const R1, R2: TRect): Boolean; overload; {$IFDEF USEINLINING} inline; {$ENDIF}
function EqualRectSize(const R1, R2: TFloatRect): Boolean; overload; {$IFDEF USEINLINING} inline; {$ENDIF}

type
//{ TBitmap32 draw mode }
enum TDrawMode
{
	dmOpaque,
	dmBlend,
	dmCustom,
	dmTransparent
};

enum TCombineMode
{
	cmBlend,
	cmMerge
};

enum TWrapMode
{
	wmClamp,
	wmRepeat,
	wmMirror
};

  TWrapProc = function(Value, Max: Integer): Integer;
  TWrapProcEx = function(Value, Min, Max: Integer): Integer;
/*
{$IFDEF DEPRECATEDMODE}
{ Stretch filters }
  TStretchFilter = (sfNearest, sfDraft, sfLinear, sfCosine, sfSpline,
    sfLanczos, sfMitchell);
{$ENDIF}
*/
//{ Gamma bias for line/pixel antialiasing }

var
  GAMMA_TABLE: array [Byte] of Byte;

void SetGamma(float Gamma = 0.7f);

type
  { TPlainInterfacedPersistent }
  { TPlainInterfacedPersistent provides simple interface support with
    optional reference-counting operation. }
  TPlainInterfacedPersistent = class(TPersistent, IInterface)
  private
    FRefCounted: Boolean;
    FRefCount: Integer;
  protected
    { IInterface }
    function _AddRef: Integer; {$IFNDEF WINDOWS}cdecl{$ELSE}stdcall{$ENDIF};
    function _Release: Integer; {$IFNDEF WINDOWS}cdecl{$ELSE}stdcall{$ENDIF};
    function QueryInterface({$IFDEF FPC_HAS_CONSTREF}constref{$ELSE}const{$ENDIF}IID: TGUID; out Obj): HResult; virtual; {$IFNDEF WINDOWS}cdecl{$ELSE}stdcall{$ENDIF};

    property RefCounted: Boolean read FRefCounted write FRefCounted;
  public
    procedure AfterConstruction; override;
    procedure BeforeDestruction; override;
    class function NewInstance: TObject; override;

    property RefCount: Integer read FRefCount;
  end;

  { TNotifiablePersistent }
  { TNotifiablePersistent provides a change notification mechanism }
  TNotifiablePersistent = class(TPlainInterfacedPersistent)
  private
    FUpdateCount: Integer;
    FOnChange: TNotifyEvent;
  protected
    property UpdateCount: Integer read FUpdateCount;
  public
    procedure Changed; virtual;
    procedure BeginUpdate; virtual;
    procedure EndUpdate; virtual;
    property OnChange: TNotifyEvent read FOnChange write FOnChange;
  end;

  { TThreadPersistent }
  { TThreadPersistent is an ancestor for TBitmap32 object. In addition to
    TPersistent methods, it provides thread-safe locking and change notification }
  TThreadPersistent = class(TNotifiablePersistent)
  private
    FLockCount: Integer;
  protected
    {$IFDEF FPC}
    FLock: TCriticalSection;
    {$ELSE}
    FLock: TRTLCriticalSection;
    {$ENDIF}
    property LockCount: Integer read FLockCount;
  public
    constructor Create; virtual;
    destructor Destroy; override;
    procedure Lock;
    procedure Unlock;
  end;

  { TCustomMap }
  { An ancestor for bitmaps and similar 2D distributions wich have width and
    height properties }
  TCustomMap = class(TThreadPersistent)
  protected
    FHeight: Integer;
    FWidth: Integer;
    FOnResize: TNotifyEvent;
    procedure SetHeight(NewHeight: Integer); virtual;
    procedure SetWidth(NewWidth: Integer); virtual;
    procedure ChangeSize(var Width, Height: Integer; NewWidth, NewHeight: Integer); virtual;
  public
    procedure Delete; virtual;
    function  Empty: Boolean; virtual;
    procedure Resized; virtual;
    function SetSizeFrom(Source: TPersistent): Boolean;
    function SetSize(NewWidth, NewHeight: Integer): Boolean; virtual;
    property Height: Integer read FHeight write SetHeight;
    property Width: Integer read FWidth write SetWidth;
    property OnResize: TNotifyEvent read FOnResize write FOnResize;
  end;

  { TBitmap32 }
  { This is the core of Graphics32 unit. The TBitmap32 class is responsible
    for storage of a bitmap, as well as for drawing in it.
    The OnCombine event is fired only when DrawMode is set to dmCustom and two
    bitmaps are blended together. Unlike most normal events, it does not contain
    "Sender" parameter and is not called through some virtual method. This
    (a little bit non-standard) approach allows for faster operation. }

const
  // common cases
  AREAINFO_RECT         = $80000000;
  AREAINFO_LINE         = $40000000; // 24 bits for line width in pixels...
  AREAINFO_ELLIPSE      = $20000000;
  AREAINFO_ABSOLUTE     = $10000000;

  AREAINFO_MASK         = $FF000000;

type
  TPixelCombineEvent = procedure(F: TColor32; var B: TColor32; M: TColor32) of object;
  TAreaChangedEvent = procedure(Sender: TObject; const Area: TRect;
    const Info: Cardinal) of object;

  TCustomResampler = class;

  TCustomBackend = class;
  TCustomBackendClass = class of TCustomBackend;

  TCustomBitmap32 = class(TCustomMap)
  private
    FBackend: TCustomBackend;
    FBits: PColor32Array;
    FClipRect: TRect;
    FFixedClipRect: TFixedRect;
    F256ClipRect: TRect;
    FClipping: Boolean;
    FDrawMode: TDrawMode;
    FCombineMode: TCombineMode;
    FWrapMode: TWrapMode;

    FMasterAlpha: Cardinal;
    FOuterColor: TColor32;
    FPenColor: TColor32;
    FStippleCounter: Single;
    FStipplePattern: TArrayOfColor32;
    FStippleStep: Single;
{$IFDEF DEPRECATEDMODE}
    FStretchFilter: TStretchFilter;
{$ENDIF}
    FOnPixelCombine: TPixelCombineEvent;
    FOnAreaChanged: TAreaChangedEvent;
    FOldOnAreaChanged: TAreaChangedEvent;
    FMeasuringMode: Boolean;
    FResampler: TCustomResampler;
    procedure BackendChangedHandler(Sender: TObject); virtual;
    procedure BackendChangingHandler(Sender: TObject); virtual;

{$IFDEF BITS_GETTER}
    function GetBits: PColor32Array;     {$IFDEF USEINLINING} inline; {$ENDIF}
{$ENDIF}

    function GetPixelPtr(X, Y: Integer): PColor32;
    function GetScanLine(Y: Integer): PColor32Array;

    procedure SetCombineMode(const Value: TCombineMode);
    procedure SetDrawMode(Value: TDrawMode);
    procedure SetWrapMode(Value: TWrapMode);
    procedure SetMasterAlpha(Value: Cardinal);
{$IFDEF DEPRECATEDMODE}
    procedure SetStretchFilter(Value: TStretchFilter);
{$ENDIF}
    procedure SetClipRect(const Value: TRect);
    procedure SetResampler(Resampler: TCustomResampler);
    function GetResamplerClassName: string;
    procedure SetResamplerClassName(Value: string);
  protected
    WrapProcHorz: TWrapProcEx;
    WrapProcVert: TWrapProcEx;
    BlendProc: Pointer;
    RasterX, RasterY: Integer;
    RasterXF, RasterYF: TFixed;
    procedure ChangeSize(var Width, Height: Integer; NewWidth, NewHeight: Integer); override;
    procedure CopyMapTo(Dst: TCustomBitmap32); virtual;
    procedure CopyPropertiesTo(Dst: TCustomBitmap32); virtual;
    procedure AssignTo(Dst: TPersistent); override;
    function  Equal(B: TCustomBitmap32): Boolean;
    procedure SET_T256(X, Y: Integer; C: TColor32);
    procedure SET_TS256(X, Y: Integer; C: TColor32);
    function  GET_T256(X, Y: Integer): TColor32;
    function  GET_TS256(X, Y: Integer): TColor32;
    procedure ReadData(Stream: TStream); virtual;
    procedure WriteData(Stream: TStream); virtual;
    procedure DefineProperties(Filer: TFiler); override;

    procedure InitializeBackend; virtual;
    procedure FinalizeBackend; virtual;
    procedure SetBackend(const Backend: TCustomBackend); virtual;

    function QueryInterface({$IFDEF FPC_HAS_CONSTREF}constref{$ELSE}const{$ENDIF} IID: TGUID; out Obj): HResult; override;

    function  GetPixel(X, Y: Integer): TColor32; {$IFDEF USEINLINING} inline; {$ENDIF}
    function  GetPixelS(X, Y: Integer): TColor32; {$IFDEF USEINLINING} inline; {$ENDIF}
    function  GetPixelW(X, Y: Integer): TColor32; {$IFDEF USEINLINING} inline; {$ENDIF}

    function  GetPixelF(X, Y: Single): TColor32; {$IFDEF USEINLINING} inline; {$ENDIF}
    function  GetPixelFS(X, Y: Single): TColor32; {$IFDEF USEINLINING} inline; {$ENDIF}
    function  GetPixelFW(X, Y: Single): TColor32; {$IFDEF USEINLINING} inline; {$ENDIF}

    function  GetPixelX(X, Y: TFixed): TColor32;
    function  GetPixelXS(X, Y: TFixed): TColor32;
    function  GetPixelXW(X, Y: TFixed): TColor32;

    function GetPixelFR(X, Y: Single): TColor32;
    function GetPixelXR(X, Y: TFixed): TColor32;

    function  GetPixelB(X, Y: Integer): TColor32; {$IFDEF USEINLINING} inline; {$ENDIF}

    procedure SetPixel(X, Y: Integer; Value: TColor32); {$IFDEF USEINLINING} inline; {$ENDIF}
    procedure SetPixelS(X, Y: Integer; Value: TColor32);
    procedure SetPixelW(X, Y: Integer; Value: TColor32); {$IFDEF USEINLINING} inline; {$ENDIF}

    procedure SetPixelF(X, Y: Single; Value: TColor32);  {$IFDEF USEINLINING} inline; {$ENDIF}
    procedure SetPixelFS(X, Y: Single; Value: TColor32);
    procedure SetPixelFW(X, Y: Single; Value: TColor32);

    procedure SetPixelX(X, Y: TFixed; Value: TColor32);
    procedure SetPixelXS(X, Y: TFixed; Value: TColor32);
    procedure SetPixelXW(X, Y: TFixed; Value: TColor32);
  public
    constructor Create; override;
    destructor Destroy; override;

    procedure Assign(Source: TPersistent); override;
    function  BoundsRect: TRect;
    function  Empty: Boolean; override;
    procedure Clear; overload;
    procedure Clear(FillColor: TColor32); overload;
    procedure Delete; override;

    procedure BeginMeasuring(const Callback: TAreaChangedEvent);
    procedure EndMeasuring;

    function ReleaseBackend: TCustomBackend;

    procedure PropertyChanged; virtual;
    procedure Changed; overload; override;
    procedure Changed(const Area: TRect; const Info: Cardinal = AREAINFO_RECT); reintroduce; overload; virtual;

    procedure LoadFromStream(Stream: TStream); virtual;
    procedure SaveToStream(Stream: TStream; SaveTopDown: Boolean = False); virtual;

    procedure LoadFromFile(const FileName: string); virtual;
    procedure SaveToFile(const FileName: string; SaveTopDown: Boolean = False); virtual;

    procedure LoadFromResourceID(Instance: THandle; ResID: Integer);
    procedure LoadFromResourceName(Instance: THandle; const ResName: string);

    procedure ResetAlpha; overload;
    procedure ResetAlpha(const AlphaValue: Byte); overload;

    procedure Draw(DstX, DstY: Integer; Src: TCustomBitmap32); overload;
    procedure Draw(DstX, DstY: Integer; const SrcRect: TRect; Src: TCustomBitmap32); overload;
    procedure Draw(const DstRect, SrcRect: TRect; Src: TCustomBitmap32); overload;

    procedure SetPixelT(X, Y: Integer; Value: TColor32); overload;
    procedure SetPixelT(var Ptr: PColor32; Value: TColor32); overload;
    procedure SetPixelTS(X, Y: Integer; Value: TColor32);

    procedure DrawTo(Dst: TCustomBitmap32); overload;
    procedure DrawTo(Dst: TCustomBitmap32; DstX, DstY: Integer; const SrcRect: TRect); overload;
    procedure DrawTo(Dst: TCustomBitmap32; DstX, DstY: Integer); overload;
    procedure DrawTo(Dst: TCustomBitmap32; const DstRect: TRect); overload;
    procedure DrawTo(Dst: TCustomBitmap32; const DstRect, SrcRect: TRect); overload;

    procedure SetStipple(NewStipple: TArrayOfColor32); overload;
    procedure SetStipple(NewStipple: array of TColor32); overload;
    procedure AdvanceStippleCounter(LengthPixels: Single);
    function  GetStippleColor: TColor32;

    procedure HorzLine(X1, Y, X2: Integer; Value: TColor32);
    procedure HorzLineS(X1, Y, X2: Integer; Value: TColor32);
    procedure HorzLineT(X1, Y, X2: Integer; Value: TColor32);
    procedure HorzLineTS(X1, Y, X2: Integer; Value: TColor32);
    procedure HorzLineTSP(X1, Y, X2: Integer);
    procedure HorzLineX(X1, Y, X2: TFixed; Value: TColor32);
    procedure HorzLineXS(X1, Y, X2: TFixed; Value: TColor32);

    procedure VertLine(X, Y1, Y2: Integer; Value: TColor32);
    procedure VertLineS(X, Y1, Y2: Integer; Value: TColor32);
    procedure VertLineT(X, Y1, Y2: Integer; Value: TColor32);
    procedure VertLineTS(X, Y1, Y2: Integer; Value: TColor32);
    procedure VertLineTSP(X, Y1, Y2: Integer);
    procedure VertLineX(X, Y1, Y2: TFixed; Value: TColor32);
    procedure VertLineXS(X, Y1, Y2: TFixed; Value: TColor32);

    procedure Line(X1, Y1, X2, Y2: Integer; Value: TColor32; L: Boolean = False);
    procedure LineS(X1, Y1, X2, Y2: Integer; Value: TColor32; L: Boolean = False);
    procedure LineT(X1, Y1, X2, Y2: Integer; Value: TColor32; L: Boolean = False);
    procedure LineTS(X1, Y1, X2, Y2: Integer; Value: TColor32; L: Boolean = False);
    procedure LineA(X1, Y1, X2, Y2: Integer; Value: TColor32; L: Boolean = False);
    procedure LineAS(X1, Y1, X2, Y2: Integer; Value: TColor32; L: Boolean = False);
    procedure LineX(X1, Y1, X2, Y2: TFixed; Value: TColor32; L: Boolean = False); overload;
    procedure LineF(X1, Y1, X2, Y2: Single; Value: TColor32; L: Boolean = False); overload;
    procedure LineXS(X1, Y1, X2, Y2: TFixed; Value: TColor32; L: Boolean = False); overload;
    procedure LineFS(X1, Y1, X2, Y2: Single; Value: TColor32; L: Boolean = False); overload;
    procedure LineXP(X1, Y1, X2, Y2: TFixed; L: Boolean = False); overload;
    procedure LineFP(X1, Y1, X2, Y2: Single; L: Boolean = False); overload;
    procedure LineXSP(X1, Y1, X2, Y2: TFixed; L: Boolean = False); overload;
    procedure LineFSP(X1, Y1, X2, Y2: Single; L: Boolean = False); overload;

    property  PenColor: TColor32 read FPenColor write FPenColor;
    procedure MoveTo(X, Y: Integer);
    procedure LineToS(X, Y: Integer);
    procedure LineToTS(X, Y: Integer);
    procedure LineToAS(X, Y: Integer);
    procedure MoveToX(X, Y: TFixed);
    procedure MoveToF(X, Y: Single);
    procedure LineToXS(X, Y: TFixed);
    procedure LineToFS(X, Y: Single);
    procedure LineToXSP(X, Y: TFixed);
    procedure LineToFSP(X, Y: Single);

    procedure FillRect(X1, Y1, X2, Y2: Integer; Value: TColor32);
    procedure FillRectS(X1, Y1, X2, Y2: Integer; Value: TColor32); overload;
    procedure FillRectT(X1, Y1, X2, Y2: Integer; Value: TColor32);
    procedure FillRectTS(X1, Y1, X2, Y2: Integer; Value: TColor32); overload;
    procedure FillRectS(const ARect: TRect; Value: TColor32); overload;
    procedure FillRectTS(const ARect: TRect; Value: TColor32); overload;

    procedure FrameRectS(X1, Y1, X2, Y2: Integer; Value: TColor32); overload;
    procedure FrameRectTS(X1, Y1, X2, Y2: Integer; Value: TColor32); overload;
    procedure FrameRectTSP(X1, Y1, X2, Y2: Integer);
    procedure FrameRectS(const ARect: TRect; Value: TColor32); overload;
    procedure FrameRectTS(const ARect: TRect; Value: TColor32); overload;

    procedure RaiseRectTS(X1, Y1, X2, Y2: Integer; Contrast: Integer); overload;
    procedure RaiseRectTS(const ARect: TRect; Contrast: Integer); overload;

    procedure Roll(Dx, Dy: Integer; FillBack: Boolean; FillColor: TColor32);
    procedure FlipHorz(Dst: TCustomBitmap32 = nil);
    procedure FlipVert(Dst: TCustomBitmap32 = nil);
    procedure Rotate90(Dst: TCustomBitmap32 = nil);
    procedure Rotate180(Dst: TCustomBitmap32 = nil);
    procedure Rotate270(Dst: TCustomBitmap32 = nil);

    procedure ResetClipRect;

    property  Pixel[X, Y: Integer]: TColor32 read GetPixel write SetPixel; default;
    property  PixelS[X, Y: Integer]: TColor32 read GetPixelS write SetPixelS;
    property  PixelW[X, Y: Integer]: TColor32 read GetPixelW write SetPixelW;
    property  PixelX[X, Y: TFixed]: TColor32 read GetPixelX write SetPixelX;
    property  PixelXS[X, Y: TFixed]: TColor32 read GetPixelXS write SetPixelXS;
    property  PixelXW[X, Y: TFixed]: TColor32 read GetPixelXW write SetPixelXW;
    property  PixelF[X, Y: Single]: TColor32 read GetPixelF write SetPixelF;
    property  PixelFS[X, Y: Single]: TColor32 read GetPixelFS write SetPixelFS;
    property  PixelFW[X, Y: Single]: TColor32 read GetPixelFW write SetPixelFW;
    property  PixelFR[X, Y: Single]: TColor32 read GetPixelFR;
    property  PixelXR[X, Y: TFixed]: TColor32 read GetPixelXR;

    property Backend: TCustomBackend read FBackend write SetBackend;

{$IFDEF BITS_GETTER}
    property Bits: PColor32Array read GetBits;
{$ELSE}
    property Bits: PColor32Array read FBits;
{$ENDIF}

    property ClipRect: TRect read FClipRect write SetClipRect;
    property Clipping: Boolean read FClipping;

    property PixelPtr[X, Y: Integer]: PColor32 read GetPixelPtr;
    property ScanLine[Y: Integer]: PColor32Array read GetScanLine;
    property StippleCounter: Single read FStippleCounter write FStippleCounter;
    property StippleStep: Single read FStippleStep write FStippleStep;

    property MeasuringMode: Boolean read FMeasuringMode;
  published
    property DrawMode: TDrawMode read FDrawMode write SetDrawMode default dmOpaque;
    property CombineMode: TCombineMode read FCombineMode write SetCombineMode default cmBlend;
    property WrapMode: TWrapMode read FWrapMode write SetWrapMode default wmClamp;
    property MasterAlpha: Cardinal read FMasterAlpha write SetMasterAlpha default $FF;
    property OuterColor: TColor32 read FOuterColor write FOuterColor default 0;
{$IFDEF DEPRECATEDMODE}
    property StretchFilter: TStretchFilter read FStretchFilter write SetStretchFilter default sfNearest;
{$ENDIF}
    property ResamplerClassName: string read GetResamplerClassName write SetResamplerClassName;
    property Resampler: TCustomResampler read FResampler write SetResampler;
    property OnChange;
    property OnPixelCombine: TPixelCombineEvent read FOnPixelCombine write FOnPixelCombine;
    property OnAreaChanged: TAreaChangedEvent read FOnAreaChanged write FOnAreaChanged;
    property OnResize;
  end;

  TBitmap32 = class(TCustomBitmap32)
  private
    FOnHandleChanged: TNotifyEvent;
      
    procedure BackendChangedHandler(Sender: TObject); override;
    procedure BackendChangingHandler(Sender: TObject); override;
    
    procedure FontChanged(Sender: TObject);
    procedure CanvasChanged(Sender: TObject);
    function GetCanvas: TCanvas;         {$IFDEF USEINLINING} inline; {$ENDIF}

    function GetBitmapInfo: TBitmapInfo; {$IFDEF USEINLINING} inline; {$ENDIF}
    function GetHandle: HBITMAP;         {$IFDEF USEINLINING} inline; {$ENDIF}
    function GetHDC: HDC;                {$IFDEF USEINLINING} inline; {$ENDIF}

    function GetFont: TFont;
    procedure SetFont(Value: TFont);
  protected
    procedure InitializeBackend; override;
    procedure FinalizeBackend; override;
    procedure SetBackend(const Backend: TCustomBackend); override;
    
    procedure HandleChanged; virtual;
    procedure CopyPropertiesTo(Dst: TCustomBitmap32); override;
  public
  {$IFDEF BCB}
    procedure Draw(const DstRect, SrcRect: TRect; hSrc: Cardinal); overload;
  {$ELSE}
    procedure Draw(const DstRect, SrcRect: TRect; hSrc: HDC); overload;
  {$ENDIF}

{$IFDEF BCB}
    procedure DrawTo(hDst: Cardinal; DstX, DstY: Integer); overload;
    procedure DrawTo(hDst: Cardinal; const DstRect, SrcRect: TRect); overload;
    procedure TileTo(hDst: Cardinal; const DstRect, SrcRect: TRect);
{$ELSE}
    procedure DrawTo(hDst: HDC; DstX, DstY: Integer); overload;
    procedure DrawTo(hDst: HDC; const DstRect, SrcRect: TRect); overload;
    procedure TileTo(hDst: HDC; const DstRect, SrcRect: TRect);
{$ENDIF}

    procedure UpdateFont;
    procedure Textout(X, Y: Integer; const Text: String); overload;
    procedure Textout(X, Y: Integer; const ClipRect: TRect; const Text: String); overload;
    procedure Textout(DstRect: TRect; const Flags: Cardinal; const Text: String); overload;
    function  TextExtent(const Text: String): TSize;
    function  TextHeight(const Text: String): Integer;
    function  TextWidth(const Text: String): Integer;
    procedure RenderText(X, Y: Integer; const Text: String; AALevel: Integer; Color: TColor32);
    procedure TextoutW(X, Y: Integer; const Text: Widestring); overload;
    procedure TextoutW(X, Y: Integer; const ClipRect: TRect; const Text: Widestring); overload;
    procedure TextoutW(DstRect: TRect; const Flags: Cardinal; const Text: Widestring); overload;
    function  TextExtentW(const Text: Widestring): TSize;
    function  TextHeightW(const Text: Widestring): Integer;
    function  TextWidthW(const Text: Widestring): Integer;
    procedure RenderTextW(X, Y: Integer; const Text: Widestring; AALevel: Integer; Color: TColor32);

    property  Canvas: TCanvas read GetCanvas;
    function  CanvasAllocated: Boolean;
    procedure DeleteCanvas;

    property Font: TFont read GetFont write SetFont;

    property BitmapHandle: HBITMAP read GetHandle;
    property BitmapInfo: TBitmapInfo read GetBitmapInfo;
    property Handle: HDC read GetHDC;
  published
    property OnHandleChanged: TNotifyEvent read FOnHandleChanged write FOnHandleChanged;
  end;

  { TCustomBackend }
  { This class functions as backend for the TBitmap32 class.
    It manages and provides the backing buffer as well as OS or
    graphics subsystem specific features.}

  TCustomBackend = class(TThreadPersistent)
  protected
    FBits: PColor32Array;
    FOwner: TCustomBitmap32;
    FOnChanging: TNotifyEvent;

    procedure Changing; virtual;

{$IFDEF BITS_GETTER}
    function GetBits: PColor32Array; virtual;
{$ENDIF}

    procedure InitializeSurface(NewWidth, NewHeight: Integer; ClearBuffer: Boolean); virtual;
    procedure FinalizeSurface; virtual;
  public
    constructor Create; overload; override;
    constructor Create(Owner: TCustomBitmap32); reintroduce; overload; virtual;
    destructor Destroy; override;

    procedure Assign(Source: TPersistent); override;

    procedure Clear; virtual;
    function Empty: Boolean; virtual;

    procedure ChangeSize(var Width, Height: Integer; NewWidth, NewHeight: Integer; ClearBuffer: Boolean = True); virtual;

{$IFDEF BITS_GETTER}
    property Bits: PColor32Array read GetBits;
{$ELSE}
    property Bits: PColor32Array read FBits;
{$ENDIF}

    property OnChanging: TNotifyEvent read FOnChanging write FOnChanging;
  end;

  { TCustomSampler }
  TCustomSampler = class(TNotifiablePersistent)
  public
    function GetSampleInt(X, Y: Integer): TColor32; virtual;
    function GetSampleFixed(X, Y: TFixed): TColor32; virtual;
    function GetSampleFloat(X, Y: TFloat): TColor32; virtual;
    procedure PrepareSampling; virtual;
    procedure FinalizeSampling; virtual;
    function HasBounds: Boolean; virtual;
    function GetSampleBounds: TFloatRect; virtual;
  end;


  TPixelAccessMode = (pamUnsafe, pamSafe, pamWrap, pamTransparentEdge);

  { TCustomResampler }
  { Base class for TCustomBitmap32 specific resamplers. }
  TCustomResampler = class(TCustomSampler)
  private
    FBitmap: TCustomBitmap32;
    FClipRect: TRect;
    FPixelAccessMode: TPixelAccessMode;
    procedure SetPixelAccessMode(const Value: TPixelAccessMode);
  protected
    function GetWidth: TFloat; virtual;
    procedure Resample(
      Dst: TCustomBitmap32; DstRect: TRect; DstClip: TRect;
      Src: TCustomBitmap32; SrcRect: TRect;
      CombineOp: TDrawMode; CombineCallBack: TPixelCombineEvent); virtual; abstract;
    procedure AssignTo(Dst: TPersistent); override;
    property ClipRect: TRect read FClipRect;
  public
    constructor Create; overload; virtual;
    constructor Create(ABitmap: TCustomBitmap32); overload; virtual;
    procedure Changed; override;
    procedure PrepareSampling; override;
    function HasBounds: Boolean; override;
    function GetSampleBounds: TFloatRect; override;
    property Bitmap: TCustomBitmap32 read FBitmap write FBitmap;
    property Width: TFloat read GetWidth;
  published
    property PixelAccessMode: TPixelAccessMode read FPixelAccessMode write SetPixelAccessMode default pamSafe;
  end;
  TCustomResamplerClass = class of TCustomResampler;

function GetPlatformBackendClass: TCustomBackendClass;

var
  StockBitmap: TBitmap;

