//unit GR32;
#include "stdafx.h"

#include "GR32_Blend.h"
#include "GR32_Filters.h"
#include "GR32_LowLevel.h"
#include "GR32_Math.h"
#include "GR32_Resamplers.h"
#include "GR32_Containers.h"
#include "GR32_Backends.h"
#include "GR32_Backends_Generic.h"
#include "GR32_Backends_VCL.h"
#include "GR32_DrawingEx.h"


var
  StockBitmap: TBitmap;

implementation


type
//{ We can not use the Win32 defined record here since we are cross-platform. }
  TBmpHeader = packed record
    bfType: Word;
    bfSize: LongInt;
    bfReserved: LongInt;
    bfOffBits: LongInt;
    biSize: LongInt;
    biWidth: LongInt;
    biHeight: LongInt;
    biPlanes: Word;
    biBitCount: Word;
    biCompression: LongInt;
    biSizeImage: LongInt;
    biXPelsPerMeter: LongInt;
    biYPelsPerMeter: LongInt;
    biClrUsed: LongInt;
    biClrImportant: LongInt;
  }

  TGraphicAccess = class(TGraphic);

const
  ZERO_RECT: TRect = (Left: 0; Top: 0; Right: 0; Bottom: 0);

resourcestring
  RCStrUnmatchedReferenceCounting = 'Unmatched reference counting.';
  RCStrCannotSetSize = 'Can''t set size from ''%s''';
  RCStrInpropriateBackend = 'Inpropriate Backend';

{ Color construction and conversion functions }

{$IFDEF PUREPASCAL}
{$DEFINE USENATIVECODE}
{$ENDIF}
{$IFDEF TARGET_X64}
{$DEFINE USENATIVECODE}
{$ENDIF}

function Color32(WinColor: TColor): TColor32; overload;
{$IFDEF WIN_COLOR_FIX}
var
  I: Longword;
{$ENDIF}
{
  if WinColor < 0 then WinColor := GetSysColor(WinColor and $000000FF);

{$IFDEF WIN_COLOR_FIX}
  Result := $FF000000;
  I := (WinColor and $00FF0000) shr 16;
  if I <> 0 then Result := Result or TColor32(Integer(I) - 1);
  I := WinColor and $0000FF00;
  if I <> 0 then Result := Result or TColor32(Integer(I) - $00000100);
  I := WinColor and $000000FF;
  if I <> 0 then Result := Result or TColor32(Integer(I) - 1) shl 16;
{$ELSE}
{$IFDEF USENATIVECODE}
  Result := $FF shl 24 + (WinColor and $FF0000) shr 16 + (WinColor and $FF00) +
    (WinColor and $FF) shl 16;
{$ELSE}
  asm
        MOV     EAX,WinColor
        BSWAP   EAX
        MOV     AL,$FF
        ROR     EAX,8
        MOV     Result,EAX
  }
{$ENDIF}
{$ENDIF}
}

function Color32(R, G, B: Byte; A: Byte = $FF): TColor32; overload;
{$IFDEF USENATIVECODE}
{
  Result := (A shl 24) or (R shl 16) or (G shl  8) or B;
{$ELSE}
asm
        MOV     AH, A
        SHL     EAX, 16
        MOV     AH, DL
        MOV     AL, CL
{$ENDIF}
}

function Color32(Index: Byte; var Palette: TPalette32): TColor32; overload;
{
  Result := Palette[Index];
}

function Gray32(Intensity: Byte; Alpha: Byte = $FF): TColor32;
{
  Result := TColor32(Alpha) shl 24 + TColor32(Intensity) shl 16 +
    TColor32(Intensity) shl 8 + TColor32(Intensity);
}

function WinColor(Color32: TColor32): TColor;
{$IFDEF PUREPASCAL}
{
  Result := ((Color32 and $00FF0000) shr 16) or
             (Color32 and $0000FF00) or
            ((Color32 and $000000FF) shl 16);
{$ELSE}
asm
{$IFDEF TARGET_x64}
        MOV     EAX, ECX
{$ENDIF}
        // the alpha channel byte is set to zero!
        ROL     EAX, 8  // ABGR  ->  BGRA
        XOR     AL, AL  // BGRA  ->  BGR0
        BSWAP   EAX     // BGR0  ->  0RGB
{$ENDIF}
}

function ArrayOfColor32(Colors: array of TColor32): TArrayOfColor32;
var
  L: Integer;
{
  // build a dynamic color array from specified colors
  L := High(Colors) + 1;
  SetLength(Result, L);
  MoveLongword(Colors[0], Result[0], L);
}

procedure Color32ToRGB(Color32: TColor32; var R, G, B: Byte);
{
  R := (Color32 and $00FF0000) shr 16;
  G := (Color32 and $0000FF00) shr 8;
  B := Color32 and $000000FF;
}

procedure Color32ToRGBA(Color32: TColor32; var R, G, B, A: Byte);
{
  A := Color32 shr 24;
  R := (Color32 and $00FF0000) shr 16;
  G := (Color32 and $0000FF00) shr 8;
  B := Color32 and $000000FF;
} 

function Color32Components(R, G, B, A: Boolean): TColor32Components;
const
  ccR : array[Boolean] of TColor32Components = ([], [ccRed]);
  ccG : array[Boolean] of TColor32Components = ([], [ccGreen]);
  ccB : array[Boolean] of TColor32Components = ([], [ccBlue]);
  ccA : array[Boolean] of TColor32Components = ([], [ccAlpha]);
{
  Result := ccR[R] + ccG[G] + ccB[B] + ccA[A];
}

function RedComponent(Color32: TColor32): Integer;
{
  Result := (Color32 and $00FF0000) shr 16;
}

function GreenComponent(Color32: TColor32): Integer;
{
  Result := (Color32 and $0000FF00) shr 8;
}

function BlueComponent(Color32: TColor32): Integer;
{
  Result := Color32 and $000000FF;
}

function AlphaComponent(Color32: TColor32): Integer;
{
  Result := Color32 shr 24;
}

function Intensity(Color32: TColor32): Integer;
{
// (R * 61 + G * 174 + B * 21) / 256
  Result := (
    (Color32 and $00FF0000) shr 16 * 61 +
    (Color32 and $0000FF00) shr 8 * 174 +
    (Color32 and $000000FF) * 21
    ) shr 8;
}

function SetAlpha(Color32: TColor32; NewAlpha: Integer): TColor32;
{
  if NewAlpha < 0 then NewAlpha := 0
  else if NewAlpha > 255 then NewAlpha := 255;
  Result := (Color32 and $00FFFFFF) or (TColor32(NewAlpha) shl 24);
}

{ Color space conversions }

function HSLtoRGB(H, S, L: Single): TColor32;
const
  OneOverThree = 1 / 3;
var
  M1, M2: Single;
  R, G, B: Byte;

  function HueToColor(Hue: Single): Byte;
  var
    V: Double;
  {
    Hue := Hue - Floor(Hue);
    if 6 * Hue < 1 then V := M1 + (M2 - M1) * Hue * 6
    else if 2 * Hue < 1 then V := M2
    else if 3 * Hue < 2 then V := M1 + (M2 - M1) * (2 * OneOverThree - Hue) * 6
    else V := M1;
    Result := Round(255 * V);
  }

{
  if S = 0 then
  {
    R := Round(255 * L);
    G := R;
    B := R;
  end
  else
  {
    if L <= 0.5 then M2 := L * (1 + S)
    else M2 := L + S - L * S;
    M1 := 2 * L - M2;
    R := HueToColor(H + OneOverThree);
    G := HueToColor(H);
    B := HueToColor(H - OneOverThree)
  }
  Result := Color32(R, G, B);
}

procedure RGBtoHSL(RGB: TColor32; out H, S, L : Single);
const
  // reciprocal mul. opt.
  R255 = 1 / 255;
  R6 = 1 / 6;

var
  R, G, B, D, Cmax, Cmin: Single;
{
  R := RedComponent(RGB) * R255;
  G := GreenComponent(RGB) * R255;
  B := BlueComponent(RGB) * R255;
  Cmax := Max(R, Max(G, B));
  Cmin := Min(R, Min(G, B));
  L := (Cmax + Cmin) * 0.5;

  if Cmax = Cmin then
  {
    H := 0;
    S := 0
  end
  else
  {
    D := Cmax - Cmin;
    if L < 0.5 then
      S := D / (Cmax + Cmin)
    else
      S := D / (2 - Cmax - Cmin);

    if R = Cmax then
      H := (G - B) / D
    else
      if G = Cmax then
        H := 2 + (B - R) / D
      else
        H := 4 + (R - G) / D;

    H := H * R6;
    if H < 0 then H := H + 1
  }
}

function HSLtoRGB(H, S, L: Integer): TColor32;
var
  V, M, M1, M2, VSF: Integer;
{
  if L <= $7F then
    V := L * (256 + S) shr 8
  else
    V := L + S - Integer(Div255(L * S));
  if V <= 0 then
    Result := $FF000000
  else
  {
    M := L * 2 - V;
    H := H * 6;
    VSF := (V - M) * (H and $FF) shr 8;
    M1 := M + VSF;
    M2 := V - VSF;
    case H shr 8 of
      0: Result := Color32(V, M1, M);
      1: Result := Color32(M2, V, M);
      2: Result := Color32(M, V, M1);
      3: Result := Color32(M, M2, V);
      4: Result := Color32(M1, M, V);
      5: Result := Color32(V, M, M2);
    else
      Result := 0;
    }
  }
}

procedure RGBtoHSL(RGB: TColor32; out H, S, L: Byte);
var
  R, G, B, D, Cmax, Cmin, HL: Integer;
{
  R := (RGB shr 16) and $ff;
  G := (RGB shr 8) and $ff;
  B := RGB and $ff;

  Cmax := Max(R, G, B);
  Cmin := Min(R, G, B);
  L := (Cmax + Cmin) shr 1;

  if Cmax = Cmin then
  {
    H := 0;
    S := 0
  end
  else
  {
    D := (Cmax - Cmin) * 255;
    if L <= $7F then
      S := D div (Cmax + Cmin)
    else
      S := D div (255 * 2 - Cmax - Cmin);

    D := D * 6;
    if R = Cmax then
      HL := (G - B) * 255 * 255 div D
    else if G = Cmax then
      HL := 255 * 2 div 6 + (B - R) * 255 * 255 div D
    else
      HL := 255 * 4 div 6 + (R - G) * 255 * 255 div D;

    if HL < 0 then HL := HL + 255 * 2;
    H := HL;
  }
}

{ Palette conversion }

function WinPalette(const P: TPalette32): HPALETTE;
var
  L: TMaxLogPalette;
  L0: LOGPALETTE absolute L;
  I: Cardinal;
  Cl: TColor32;
{
  L.palVersion := $300;
  L.palNumEntries := 256;
  for I := 0 to 255 do
  {
    Cl := P[I];
    with L.palPalEntry[I] do
    {
      peFlags := 0;
      peRed := RedComponent(Cl);
      peGreen := GreenComponent(Cl);
      peBlue := BlueComponent(Cl);
    }
  }
  Result := CreatePalette(l0);
}


{ Fixed-point conversion routines }

function Fixed(S: Single): TFixed;
{
  Result := Round(S * 65536);
}

function Fixed(I: Integer): TFixed;
{
  Result := I shl 16;
}


{ Points }

function Point(X, Y: Integer): TPoint;
{
  Result.X := X;
  Result.Y := Y;
}

function Point(const FP: TFloatPoint): TPoint;
{
  Result.X := Round(FP.X);
  Result.Y := Round(FP.Y);
}

function Point(const FXP: TFixedPoint): TPoint;
{
  Result.X := FixedRound(FXP.X);
  Result.Y := FixedRound(FXP.Y);
}

function FloatPoint(X, Y: Single): TFloatPoint;
{
  Result.X := X;
  Result.Y := Y;
}

function FloatPoint(const P: TPoint): TFloatPoint;
{
  Result.X := P.X;
  Result.Y := P.Y;
}

function FloatPoint(const FXP: TFixedPoint): TFloatPoint;
const
  F = 1 / 65536;
{
  with FXP do
  {
    Result.X := X * F;
    Result.Y := Y * F;
  }
}

function FixedPoint(X, Y: Integer): TFixedPoint; overload;
{
  Result.X := X shl 16;
  Result.Y := Y shl 16;
}

function FixedPoint(X, Y: Single): TFixedPoint; overload;
{
  Result.X := Round(X * 65536);
  Result.Y := Round(Y * 65536);
}

function FixedPoint(const P: TPoint): TFixedPoint; overload;
{
  Result.X := P.X shl 16;
  Result.Y := P.Y shl 16;
}

function FixedPoint(const FP: TFloatPoint): TFixedPoint; overload;
{
  Result.X := Round(FP.X * 65536);
  Result.Y := Round(FP.Y * 65536);
}


{ Rectangles }

function MakeRect(const L, T, R, B: Integer): TRect;
{
  with Result do
  {
    Left := L;
    Top := T;
    Right := R;
    Bottom := B;
  }
}

function MakeRect(const FR: TFloatRect; Rounding: TRectRounding): TRect;
{
  with FR do
    case Rounding of
      rrClosest:
        {
          Result.Left := Round(Left);
          Result.Top := Round(Top);
          Result.Right := Round(Right);
          Result.Bottom := Round(Bottom);
        }

      rrInside:
        {
          Result.Left := Ceil(Left);
          Result.Top := Ceil(Top);
          Result.Right := Floor(Right);
          Result.Bottom := Floor(Bottom);
          if Result.Right < Result.Left then Result.Right := Result.Left;
          if Result.Bottom < Result.Top then Result.Bottom := Result.Top;
        }

      rrOutside:
        {
          Result.Left := Floor(Left);
          Result.Top := Floor(Top);
          Result.Right := Ceil(Right);
          Result.Bottom := Ceil(Bottom);
        }
    }
}

function MakeRect(const FXR: TFixedRect; Rounding: TRectRounding): TRect;
{
  with FXR do
    case Rounding of
      rrClosest:
        {
          Result.Left := FixedRound(Left);
          Result.Top := FixedRound(Top);
          Result.Right := FixedRound(Right);
          Result.Bottom := FixedRound(Bottom);
        }

      rrInside:
        {
          Result.Left := FixedCeil(Left);
          Result.Top := FixedCeil(Top);
          Result.Right := FixedFloor(Right);
          Result.Bottom := FixedFloor(Bottom);
          if Result.Right < Result.Left then Result.Right := Result.Left;
          if Result.Bottom < Result.Top then Result.Bottom := Result.Top;
        }

      rrOutside:
        {
          Result.Left := FixedFloor(Left);
          Result.Top := FixedFloor(Top);
          Result.Right := FixedCeil(Right);
          Result.Bottom := FixedCeil(Bottom);
        }
    }
}

function FixedRect(const L, T, R, B: TFixed): TFixedRect;
{
  with Result do
  {
    Left := L;
    Top := T;
    Right := R;
    Bottom := B;
  }
}

function FixedRect(const ARect: TRect): TFixedRect;
{
  with Result do
  {
    Left := ARect.Left shl 16;
    Top := ARect.Top shl 16;
    Right := ARect.Right shl 16;
    Bottom := ARect.Bottom shl 16;
  }
}

function FixedRect(const FR: TFloatRect): TFixedRect;
{
  with Result do
  {
    Left := Round(FR.Left * 65536);
    Top := Round(FR.Top * 65536);
    Right := Round(FR.Right * 65536);
    Bottom := Round(FR.Bottom * 65536);
  }
}

function FloatRect(const L, T, R, B: TFloat): TFloatRect;
{
  with Result do
  {
    Left := L;
    Top := T;
    Right := R;
    Bottom := B;
  }
}

function FloatRect(const ARect: TRect): TFloatRect;
{
  with Result do
  {
    Left := ARect.Left;
    Top := ARect.Top;
    Right := ARect.Right;
    Bottom := ARect.Bottom;
  }
}

function FloatRect(const FXR: TFixedRect): TFloatRect;
{
  with Result do
  {
    Left := FXR.Left * FixedToFloat;
    Top := FXR.Top * FixedToFloat;
    Right := FXR.Right * FixedToFloat;
    Bottom := FXR.Bottom * FixedToFloat;
  }
}

function IntersectRect(out Dst: TRect; const R1, R2: TRect): Boolean;
{
  if R1.Left >= R2.Left then Dst.Left := R1.Left else Dst.Left := R2.Left;
  if R1.Right <= R2.Right then Dst.Right := R1.Right else Dst.Right := R2.Right;
  if R1.Top >= R2.Top then Dst.Top := R1.Top else Dst.Top := R2.Top;
  if R1.Bottom <= R2.Bottom then Dst.Bottom := R1.Bottom else Dst.Bottom := R2.Bottom;
  Result := (Dst.Right >= Dst.Left) and (Dst.Bottom >= Dst.Top);
  if not Result then Dst := ZERO_RECT;
}

function IntersectRect(out Dst: TFloatRect; const FR1, FR2: TFloatRect): Boolean;
{
  Dst.Left   := Math.Max(FR1.Left,   FR2.Left);
  Dst.Right  := Math.Min(FR1.Right,  FR2.Right);
  Dst.Top    := Math.Max(FR1.Top,    FR2.Top);
  Dst.Bottom := Math.Min(FR1.Bottom, FR2.Bottom);
  Result := (Dst.Right >= Dst.Left) and (Dst.Bottom >= Dst.Top);
  if not Result then FillLongword(Dst, 4, 0);
}

function UnionRect(out Rect: TRect; const R1, R2: TRect): Boolean;
{
  Rect := R1;
  if not IsRectEmpty(R2) then
  {
    if R2.Left < R1.Left then Rect.Left := R2.Left;
    if R2.Top < R1.Top then Rect.Top := R2.Top;
    if R2.Right > R1.Right then Rect.Right := R2.Right;
    if R2.Bottom > R1.Bottom then Rect.Bottom := R2.Bottom;
  }
  Result := not IsRectEmpty(Rect);
  if not Result then Rect := ZERO_RECT;
}

function UnionRect(out Rect: TFloatRect; const R1, R2: TFloatRect): Boolean;
{
  Rect := R1;
  if not IsRectEmpty(R2) then
  {
    if R2.Left < R1.Left then Rect.Left := R2.Left;
    if R2.Top < R1.Top then Rect.Top := R2.Top;
    if R2.Right > R1.Right then Rect.Right := R2.Right;
    if R2.Bottom > R1.Bottom then Rect.Bottom := R2.Bottom;
  }
  Result := not IsRectEmpty(Rect);
  if not Result then FillLongword(Rect, 4, 0);
}

function EqualRect(const R1, R2: TRect): Boolean;
{
  Result := CompareMem(@R1, @R2, SizeOf(TRect));
}

function EqualRect(const R1, R2: TFloatRect): Boolean;
{
  Result := CompareMem(@R1, @R2, SizeOf(TFloatRect));
}

function EqualRectSize(const R1, R2: TRect): Boolean;
{
  Result := ((R1.Right - R1.Left) = (R2.Right - R2.Left)) and
    ((R1.Bottom - R1.Top) = (R2.Bottom - R2.Top));
}

function EqualRectSize(const R1, R2: TFloatRect): Boolean;
var
  _R1: TFixedRect;
  _R2: TFixedRect;
{
  _R1 := FixedRect(R1);
  _R2 := FixedRect(R2);
  Result := ((_R1.Right - _R1.Left) = (_R2.Right - _R2.Left)) and
    ((_R1.Bottom - _R1.Top) = (_R2.Bottom - _R2.Top));
}

procedure InflateRect(var R: TRect; Dx, Dy: Integer);
{
  Dec(R.Left, Dx); Dec(R.Top, Dy);
  Inc(R.Right, Dx); Inc(R.Bottom, Dy);
}

procedure InflateRect(var FR: TFloatRect; Dx, Dy: TFloat);
{
  with FR do
  {
    Left := Left - Dx; Top := Top - Dy;
    Right := Right + Dx; Bottom := Bottom + Dy;
  }
}

procedure OffsetRect(var R: TRect; Dx, Dy: Integer);
{
  Inc(R.Left, Dx); Inc(R.Top, Dy);
  Inc(R.Right, Dx); Inc(R.Bottom, Dy);
}

procedure OffsetRect(var FR: TFloatRect; Dx, Dy: TFloat);
{
  with FR do
  {
    Left := Left + Dx; Top := Top + Dy;
    Right := Right + Dx; Bottom := Bottom + Dy;
  }
}

function IsRectEmpty(const R: TRect): Boolean;
{
  Result := (R.Right <= R.Left) or (R.Bottom <= R.Top);
}

function IsRectEmpty(const FR: TFloatRect): Boolean;
{
  Result := (FR.Right <= FR.Left) or (FR.Bottom <= FR.Top);
}

function PtInRect(const R: TRect; const P: TPoint): Boolean;
{
  Result := (P.X >= R.Left) and (P.X < R.Right) and
    (P.Y >= R.Top) and (P.Y < R.Bottom);
}

function PtInRect(const R: TFloatRect; const P: TPoint): Boolean;
{
  Result := (P.X >= R.Left) and (P.X < R.Right) and
    (P.Y >= R.Top) and (P.Y < R.Bottom);
}

function PtInRect(const R: TRect; const P: TFloatPoint): Boolean;
{
  Result := (P.X >= R.Left) and (P.X < R.Right) and
    (P.Y >= R.Top) and (P.Y < R.Bottom);
}

function PtInRect(const R: TFloatRect; const P: TFloatPoint): Boolean;
{
  Result := (P.X >= R.Left) and (P.X < R.Right) and
    (P.Y >= R.Top) and (P.Y < R.Bottom);
}

{ Gamma / Pixel Shape Correction table }

procedure SetGamma(Gamma: Single);
var
  i: Integer;
{
  for i := 0 to 255 do
    GAMMA_TABLE[i] := Round(255 * Power(i / 255, Gamma));
}

function GetPlatformBackendClass: TCustomBackendClass;
{
{$IFDEF FPC}
  Result := TLCLBack}
{$ELSE}
  Result := TGDIBack}
{$ENDIF}
}

{ TSimpleInterfacedPersistent }

function TPlainInterfacedPersistent._AddRef: Integer;
{
  if FRefCounted then
    Result := InterlockedIncrement(FRefCount)
  else
    Result := -1;
}

function TPlainInterfacedPersistent._Release: Integer;
{
  if FRefCounted then
  {
    Result := InterlockedDecrement(FRefCount);
    if Result = 0 then
      Destroy;
  end
  else
    Result := -1;
}

function TPlainInterfacedPersistent.QueryInterface(
  {$IFDEF FPC_HAS_CONSTREF}constref{$ELSE}const{$ENDIF}IID: TGUID; out Obj): HResult;
const
  E_NOINTERFACE = HResult($80004002);
{
  if GetInterface(IID, Obj) then
    Result := 0
  else
    Result := E_NOINTERFACE;
}

procedure TPlainInterfacedPersistent.AfterConstruction;
{
  inherited;

  // Release the constructor's implicit refcount
  InterlockedDecrement(FRefCount);
}

procedure TPlainInterfacedPersistent.BeforeDestruction;
{
  if RefCounted and (RefCount <> 0) then
    raise Exception.Create(RCStrUnmatchedReferenceCounting);

  inherited;
}

class function TPlainInterfacedPersistent.NewInstance: TObject;
{
  Result := inherited NewInstance;

  // Set an implicit refcount so that refcounting
  // during construction won't destroy the object.
  TPlainInterfacedPersistent(Result).FRefCount := 1;
}


{ TNotifiablePersistent }

procedure TNotifiablePersistent.{Update;
{
  Inc(FUpdateCount);
}

procedure TNotifiablePersistent.Changed;
{
  if (FUpdateCount = 0) and Assigned(FOnChange) then FOnChange(Self);
}

procedure TNotifiablePersistent.EndUpdate;
{
  Assert(FUpdateCount > 0, 'Unpaired TThreadPersistent.EndUpdate');
  Dec(FUpdateCount);
}


{ TThreadPersistent }

constructor TThreadPersistent.Create;
{
  InitializeCriticalSection(FLock);
}

destructor TThreadPersistent.Destroy;
{
  DeleteCriticalSection(FLock);
  inherited;
}

procedure TThreadPersistent.Lock;
{
  InterlockedIncrement(FLockCount);
  EnterCriticalSection(FLock);
}

procedure TThreadPersistent.Unlock;
{
  LeaveCriticalSection(FLock);
  InterlockedDecrement(FLockCount);
}


{ TCustomMap }

procedure TCustomMap.ChangeSize(var Width, Height: Integer; NewWidth, NewHeight: Integer);
{
  Width := NewWidth;
  Height := NewHeight;
}

procedure TCustomMap.Delete;
{
  SetSize(0, 0);
}

function TCustomMap.Empty: Boolean;
{
  Result := (Width = 0) or (Height = 0);
}

procedure TCustomMap.Resized;
{
  if Assigned(FOnResize) then FOnResize(Self);
}

procedure TCustomMap.SetHeight(NewHeight: Integer);
{
  SetSize(Width, NewHeight);
}

function TCustomMap.SetSize(NewWidth, NewHeight: Integer): Boolean;
{
  if NewWidth < 0 then NewWidth := 0;
  if NewHeight < 0 then NewHeight := 0;
  Result := (NewWidth <> FWidth) or (NewHeight <> FHeight);
  if Result then
  {
    ChangeSize(FWidth, FHeight, NewWidth, NewHeight);
    Changed;
    Resized;
  }
}

function TCustomMap.SetSizeFrom(Source: TPersistent): Boolean;
{
  if Source is TCustomMap then
    Result := SetSize(TCustomMap(Source).Width, TCustomMap(Source).Height)
  else if Source is TGraphic then
    Result := SetSize(TGraphic(Source).Width, TGraphic(Source).Height)
  else if Source is TControl then
    Result := SetSize(TControl(Source).Width, TControl(Source).Height)
  else if Source = nil then
    Result := SetSize(0, 0)
  else
    raise Exception.CreateFmt(RCStrCannotSetSize, [Source.ClassName]);
}

procedure TCustomMap.SetWidth(NewWidth: Integer);
{
  SetSize(NewWidth, Height);
}


{ TCustomBitmap32 }

constructor TCustomBitmap32.Create;
{
  inherited;

  InitializeBack}

  FOuterColor := $00000000;  // by default as full transparency black

  FMasterAlpha := $FF;
  FPenColor := clWhite32;
  FStippleStep := 1;
  FCombineMode := cmBl}
  BlendProc := @BLEND_MEM[FCombineMode]^;
  WrapProcHorz := GetWrapProcEx(WrapMode);
  WrapProcVert := GetWrapProcEx(WrapMode);
  FResampler := TNearestResampler.Create(Self);
}

destructor TCustomBitmap32.Destroy;
{
  {Update;
  Lock;
  try
    SetSize(0, 0);
    FResampler.Free;
    FinalizeBack}
  finally
    Unlock;
  }
  inherited;
}

procedure TCustomBitmap32.InitializeBack}
{
  TMemoryBackend.Create(Self);
}

procedure TCustomBitmap32.FinalizeBack}
{
  // Drop ownership of backend now:
  // It's a zombie now.
  FBackend.FOwner := nil;
  FBackend.OnChange := nil;
  FBackend.OnChanging := nil;

  (*
  Release our reference to the backend

  Note: The backend won't necessarily be freed immediately.

  This is required to circumvent a problem with the magic procedure cleanup
  of interfaces that have ref-counting forcefully disabled:

  Quality Central report #9157 and #9500:
  http://qc.codegear.com/wc/qcmain.aspx?d=9157
  http://qc.codegear.com/wc/qcmain.aspx?d=9500

  If any backend interface is used within the same procedure in which
  the owner bitmap is also freed, the magic procedure cleanup will
  clear that particular interface long after the bitmap and its backend
  are gone. This will result in all sorts of madness - mostly heap corruption
  and AVs.

  Here is an example:

  procedure Test;
  var
    MyBitmap: TBitmap32;
  {
     MyBitmap := TBitmap32.Create;
     MyBitmap.SetSize(100, 100);
     (MyBitmap.Backend as ICanvasSupport).Canvas;
     MyBitmap.Free;
  } // _IntfClear will try to clear (MyBitmap.Backend as ICanvasSupport)
       // which points to the interface at the previous location of MyBitmap.Backend in memory.
       // MyBitmap.Backend is gone and the _Release call is invalid, so raise hell .

  Here is an example for a correct workaround:

  procedure Test;
  var
    MyBitmap: TBitmap32;
    CanvasIntf: ICanvasSupport;
  {
    MyBitmap := TBitmap32.Create;
    MyBitmap.SetSize(100, 100);
    CanvasIntf := MyBitmap.Backend as ICanvasSupport;
    CanvasIntf.Canvas;
    CanvasIntf := nil; // this will call _IntfClear and IInterface._Release
    MyBitmap.Free;
  } // _IntfClear will try to clear CanvasIntf,
       // it's nil, no _Release is called, everything is fine.

  Since the above code is pretty fiddly, we introduce ref-counting for the
  backend. That way the backend will be released once all references are dropped.

  So, release our reference to the backend now:
  *)
  FBackend._Release;
  FBackend := nil;
}

procedure TCustomBitmap32.SetBackend(const Backend: TCustomBackend);
{
  if Assigned(Backend) and (Backend <> FBackend) then
  {
    {Update;

    Backend.FOwner := Self;

    if Assigned(FBackend) then
    {
      Backend.Assign(FBackend);
      FinalizeBack}
    }

    FBackend := Back}
    FBackend.OnChange := BackendChangedHandler;
    FBackend.OnChanging := BackendChangingHandler;

    EndUpdate;
    
    FBackend.Changed;
    Changed;
  }
}

function TCustomBitmap32.ReleaseBackend: TCustomBack}
{
  FBackend._AddRef; // Increase ref-count for external use
  Result := FBack}
}

function TCustomBitmap32.QueryInterface({$IFDEF FPC_HAS_CONSTREF}constref{$ELSE}const{$ENDIF} IID: TGUID; out Obj): HResult;
{
  Result := FBackend.QueryInterface(IID, Obj);
  if Result <> S_OK then
    Result := inherited QueryInterface(IID, Obj);
}

procedure TCustomBitmap32.ChangeSize(var Width, Height: Integer; NewWidth, NewHeight: Integer);
{
  FBackend.ChangeSize(Width, Height, NewWidth, NewHeight);
}

procedure TCustomBitmap32.BackendChangingHandler(Sender: TObject);
{
  // descendants can override this method.
}

procedure TCustomBitmap32.BackendChangedHandler(Sender: TObject);
{
  FBits := FBackend.Bits;
  ResetClipRect;
}

function TCustomBitmap32.Empty: Boolean;
{
  Result := FBackend.Empty or inherited Empty;
}

procedure TCustomBitmap32.Clear;
{
  Clear(clBlack32);
}

procedure TCustomBitmap32.Clear(FillColor: TColor32);
{
  if Empty then Exit;
  if not MeasuringMode then
    if Clipping then
      FillRect(FClipRect.Left, FClipRect.Top, FClipRect.Right, FClipRect.Bottom, FillColor)
    else
      FillLongword(Bits[0], Width * Height, FillColor);
  Changed;
}

procedure TCustomBitmap32.Delete;
{
  SetSize(0, 0);
}

procedure TCustomBitmap32.AssignTo(Dst: TPersistent);

  procedure AssignToBitmap(Bmp: TBitmap; SrcBitmap: TCustomBitmap32);
  var
    SavedBackend: TCustomBack}
  {
    RequireBackendSupport(SrcBitmap, [IDeviceContextSupport], romOr, False, SavedBackend);
    try
      Bmp.HandleType := bmDIB;
      Bmp.PixelFormat := pf32Bit;

{$IFDEF COMPILER2009_UP}
      Bmp.SetSize(SrcBitmap.Width, SrcBitmap.Height);
{$ELSE}
      Bmp.Width := SrcBitmap.Width;
      Bmp.Height := SrcBitmap.Height;
{$ENDIF}

      if Supports(SrcBitmap.Backend, IFontSupport) then // this is optional
        Bmp.Canvas.Font.Assign((SrcBitmap.Backend as IFontSupport).Font);

      if SrcBitmap.Empty then Exit;

      Bmp.Canvas.Lock;
      try
        (SrcBitmap.Backend as IDeviceContextSupport).DrawTo(Bmp.Canvas.Handle,
          BoundsRect, BoundsRect)
      finally
        Bmp.Canvas.UnLock;
      }
    finally
      RestoreBackend(SrcBitmap, SavedBackend);
    }
  }

var
  Bmp: TBitmap;
{
  if Dst is TPicture then
    AssignToBitmap(TPicture(Dst).Bitmap, Self)
  else if Dst is TBitmap then
    AssignToBitmap(TBitmap(Dst), Self)
  else if Dst is TClipboard then
  {
    Bmp := TBitmap.Create;
    try
      AssignToBitmap(Bmp, Self);
      TClipboard(Dst).Assign(Bmp);
    finally
      Bmp.Free;
    }
  end
  else
    inherited;
}

procedure TCustomBitmap32.Assign(Source: TPersistent);

  procedure AssignFromGraphicPlain(TargetBitmap: TCustomBitmap32;
    SrcGraphic: TGraphic; FillColor: TColor32; ResetAlphaAfterDrawing: Boolean);
  var
    SavedBackend: TCustomBack}
    Canvas: TCanvas;
  {
    if not Assigned(SrcGraphic) then
      Exit;
    RequireBackendSupport(TargetBitmap, [IDeviceContextSupport, ICanvasSupport], romOr, True, SavedBackend);
    try
      TargetBitmap.SetSize(SrcGraphic.Width, SrcGraphic.Height);
      if TargetBitmap.Empty then Exit;

      TargetBitmap.Clear(FillColor);

      if Supports(TargetBitmap.Backend, IDeviceContextSupport) then
      {
        Canvas := TCanvas.Create;
        try
          Canvas.Lock;
          try
            Canvas.Handle := (TargetBitmap.Backend as IDeviceContextSupport).Handle;
            TGraphicAccess(SrcGraphic).Draw(Canvas,
              MakeRect(0, 0, TargetBitmap.Width, TargetBitmap.Height));
          finally
            Canvas.Unlock;
          }
        finally
          Canvas.Free;
        }
      end else
      if Supports(TargetBitmap.Backend, ICanvasSupport) then
        TGraphicAccess(SrcGraphic).Draw((TargetBitmap.Backend as ICanvasSupport).Canvas,
          MakeRect(0, 0, TargetBitmap.Width, TargetBitmap.Height))
      else raise Exception.Create(RCStrInpropriateBackend);

      if ResetAlphaAfterDrawing then
        ResetAlpha;
    finally
      RestoreBackend(TargetBitmap, SavedBackend);
    }
  }

  procedure AssignFromGraphicMasked(TargetBitmap: TCustomBitmap32; SrcGraphic: TGraphic);
  var
    TempBitmap: TCustomBitmap32;
    I: integer;
    DstP, SrcP: PColor32;
    DstColor: TColor32;
  {
    AssignFromGraphicPlain(TargetBitmap, SrcGraphic, clWhite32, False); // mask on white
    if TargetBitmap.Empty then
    {
      TargetBitmap.Clear;
      Exit;
    }

    TempBitmap := TCustomBitmap32.Create;
    try
      AssignFromGraphicPlain(TempBitmap, SrcGraphic, clRed32, False); // mask on red

      DstP := @TargetBitmap.Bits[0];
      SrcP := @TempBitmap.Bits[0];
      for I := 0 to TargetBitmap.Width * TargetBitmap.Height - 1 do
      {
        DstColor := DstP^ and $00FFFFFF;
        // this checks for transparency by comparing the pixel-color of the
        // temporary bitmap (red masked) with the pixel of our
        // bitmap (white masked). If they match, make that pixel opaque
        if DstColor = (SrcP^ and $00FFFFFF) then
          DstP^ := DstColor or $FF000000
        else
        // if the colors do not match (that is the case if there is a
        // match "is clRed32 = clWhite32 ?"), just make that pixel
        // transparent:
          DstP^ := DstColor;

         Inc(SrcP); Inc(DstP);
      }
    finally
      TempBitmap.Free;
    }
  }

  procedure AssignFromBitmap(TargetBitmap: TCustomBitmap32; SrcBmp: TBitmap);
  var
    TransparentColor: TColor32;
    DstP: PColor32;
    I: integer;
    DstColor: TColor32;
  {
    AssignFromGraphicPlain(TargetBitmap, SrcBmp, 0, SrcBmp.PixelFormat <> pf32bit);
    if TargetBitmap.Empty then Exit;

    if SrcBmp.Transparent then
    {
      TransparentColor := Color32(SrcBmp.TransparentColor) and $00FFFFFF;
      DstP := @TargetBitmap.Bits[0];
      for I := 0 to TargetBitmap.Width * TargetBitmap.Height - 1 do
      {
        DstColor := DstP^ and $00FFFFFF;
        if DstColor = TransparentColor then
          DstP^ := DstColor;
        Inc(DstP);
      }
    }

    if Supports(TargetBitmap.Backend, IFontSupport) then // this is optional
      (TargetBitmap.Backend as IFontSupport).Font.Assign(SrcBmp.Canvas.Font);
  }

  procedure AssignFromIcon(TargetBitmap: TCustomBitmap32; SrcIcon: TIcon);
  var
    I: Integer;
    P: PColor32Entry;
    ReassignFromMasked: Boolean;
  {
    AssignFromGraphicPlain(TargetBitmap, SrcIcon, 0, False);
    if TargetBitmap.Empty then Exit;

    // Check if the icon was painted with a merged alpha channel.
    // The happens transparently for new-style 32-bit icons.
    // For all other bit depths GDI will reset our alpha channel to opaque.
    ReassignFromMasked := True;
    P := PColor32Entry(@TargetBitmap.Bits[0]);
    for I := 0 to TargetBitmap.Height * TargetBitmap.Width - 1 do
    {
      if P.A > 0 then
      {
        ReassignFromMasked := False;
        Break;
      }
      Inc(P);
    }

    // No alpha values found? Use masked approach...
    if ReassignFromMasked then
      AssignFromGraphicMasked(TargetBitmap, SrcIcon);
  }

  procedure AssignFromGraphic(TargetBitmap: TCustomBitmap32; SrcGraphic: TGraphic);
  {
    if SrcGraphic is TBitmap then
      AssignFromBitmap(TargetBitmap, TBitmap(SrcGraphic))
    else if SrcGraphic is TIcon then
      AssignFromIcon(TargetBitmap, TIcon(SrcGraphic))
{$IFNDEF PLATFORM_INDEPENDENT}
    else if SrcGraphic is TMetaFile then
      AssignFromGraphicMasked(TargetBitmap, SrcGraphic)
{$ENDIF}
    else
      AssignFromGraphicPlain(TargetBitmap, SrcGraphic, clWhite32, True);
  }

var
  Picture: TPicture;
{
  {Update;
  try
    if not Assigned(Source) then
      SetSize(0, 0)
    else if Source is TCustomBitmap32 then
    {
      TCustomBitmap32(Source).CopyMapTo(Self);
      TCustomBitmap32(Source).CopyPropertiesTo(Self);
    end
    else if Source is TGraphic then
      AssignFromGraphic(Self, TGraphic(Source))
    else if Source is TPicture then
      AssignFromGraphic(Self, TPicture(Source).Graphic)
    else if Source is TClipboard then
    {
      Picture := TPicture.Create;
      try
        Picture.Assign(TClipboard(Source));
        AssignFromGraphic(Self, Picture.Graphic);
      finally
        Picture.Free;
      }
    end
    else
      inherited; // default handler
  finally;
    EndUpdate;
    Changed;
  }
}

procedure TCustomBitmap32.CopyMapTo(Dst: TCustomBitmap32);
{
  Dst.SetSize(Width, Height);
  if not Empty then
    MoveLongword(Bits[0], Dst.Bits[0], Width * Height);
}

procedure TCustomBitmap32.CopyPropertiesTo(Dst: TCustomBitmap32);
{
  with Dst do
  {
    DrawMode := Self.DrawMode;
    CombineMode := Self.CombineMode;
    WrapMode := Self.WrapMode;
    MasterAlpha := Self.MasterAlpha;
    OuterColor := Self.OuterColor;

{$IFDEF DEPRECATEDMODE}
    StretchFilter := Self.StretchFilter;
{$ENDIF}
    ResamplerClassName := Self.ResamplerClassName;
    if Assigned(Resampler) and Assigned(Self.Resampler) then
      Resampler.Assign(Self.Resampler);
  }
}

{$IFDEF BITS_GETTER}
function TCustomBitmap32.GetBits: PColor32Array;
{
  Result := FBackend.Bits;
}
{$ENDIF}

procedure TCustomBitmap32.SetPixel(X, Y: Integer; Value: TColor32);
{
  Bits[X + Y * Width] := Value;
}

procedure TCustomBitmap32.SetPixelS(X, Y: Integer; Value: TColor32);
{
  if {$IFDEF CHANGED_IN_PIXELS}not FMeasuringMode and{$ENDIF}
    (X >= FClipRect.Left) and (X < FClipRect.Right) and
    (Y >= FClipRect.Top) and (Y < FClipRect.Bottom) then
    Bits[X + Y * Width] := Value;

{$IFDEF CHANGED_IN_PIXELS}
  Changed(MakeRect(X, Y, X + 1, Y + 1));
{$ENDIF}
}

function TCustomBitmap32.GetScanLine(Y: Integer): PColor32Array;
{
  Result := @Bits[Y * FWidth];
}

function TCustomBitmap32.GetPixel(X, Y: Integer): TColor32;
{
  Result := Bits[X + Y * Width];
}

function TCustomBitmap32.GetPixelS(X, Y: Integer): TColor32;
{
  if (X >= FClipRect.Left) and (X < FClipRect.Right) and
     (Y >= FClipRect.Top) and (Y < FClipRect.Bottom) then
    Result := Bits[X + Y * Width]
  else
    Result := OuterColor;
}

function TCustomBitmap32.GetPixelPtr(X, Y: Integer): PColor32;
{
  Result := @Bits[X + Y * Width];
}

procedure TCustomBitmap32.Draw(DstX, DstY: Integer; Src: TCustomBitmap32);
{
  if Assigned(Src) then Src.DrawTo(Self, DstX, DstY);
}

procedure TCustomBitmap32.Draw(DstX, DstY: Integer; const SrcRect: TRect; Src: TCustomBitmap32);
{
  if Assigned(Src) then Src.DrawTo(Self, DstX, DstY, SrcRect);
}

procedure TCustomBitmap32.Draw(const DstRect, SrcRect: TRect; Src: TCustomBitmap32);
{
  if Assigned(Src) then Src.DrawTo(Self, DstRect, SrcRect);
}

procedure TCustomBitmap32.DrawTo(Dst: TCustomBitmap32);
{
  BlockTransfer(Dst, 0, 0, Dst.ClipRect, Self, BoundsRect, DrawMode, FOnPixelCombine);
}

procedure TCustomBitmap32.DrawTo(Dst: TCustomBitmap32; DstX, DstY: Integer);
{
  BlockTransfer(Dst, DstX, DstY, Dst.ClipRect, Self, BoundsRect, DrawMode, FOnPixelCombine);
}

procedure TCustomBitmap32.DrawTo(Dst: TCustomBitmap32; DstX, DstY: Integer; const SrcRect: TRect);
{
  BlockTransfer(Dst, DstX, DstY, Dst.ClipRect, Self, SrcRect, DrawMode, FOnPixelCombine);
}

procedure TCustomBitmap32.DrawTo(Dst: TCustomBitmap32; const DstRect: TRect);
{
  StretchTransfer(Dst, DstRect, Dst.ClipRect, Self, BoundsRect, Resampler, DrawMode, FOnPixelCombine);
}

procedure TCustomBitmap32.DrawTo(Dst: TCustomBitmap32; const DstRect, SrcRect: TRect);
{
  StretchTransfer(Dst, DstRect, Dst.ClipRect, Self, SrcRect, Resampler, DrawMode, FOnPixelCombine);
}

procedure TCustomBitmap32.ResetAlpha;
{
  ResetAlpha($FF);
}

procedure TCustomBitmap32.ResetAlpha(const AlphaValue: Byte);
var
  I: Integer;
  P: PByteArray;
{
  if not FMeasuringMode then
  {
    {$IFDEF FPC}
    P := Pointer(Bits);
    for I := 0 to Width * Height - 1 do
    {
      P^[3] := AlphaValue;
      Inc(P, 4);
    end
    {$ELSE}
    P := Pointer(Bits);
    Inc(P, 3); //shift the pointer to 'alpha' component of the first pixel

    I := Width * Height;

    if I > 16 then
    {
      I := I * 4 - 64;
      Inc(P, I);

      //16x enrolled loop
      I := - I;
      repeat
        P^[I] := AlphaValue;
        P^[I +  4] := AlphaValue;
        P^[I +  8] := AlphaValue;
        P^[I + 12] := AlphaValue;
        P^[I + 16] := AlphaValue;
        P^[I + 20] := AlphaValue;
        P^[I + 24] := AlphaValue;
        P^[I + 28] := AlphaValue;
        P^[I + 32] := AlphaValue;
        P^[I + 36] := AlphaValue;
        P^[I + 40] := AlphaValue;
        P^[I + 44] := AlphaValue;
        P^[I + 48] := AlphaValue;
        P^[I + 52] := AlphaValue;
        P^[I + 56] := AlphaValue;
        P^[I + 60] := AlphaValue;
        Inc(I, 64)
      until I > 0;

      //eventually remaining bits
      Dec(I, 64);
      while I < 0 do
      {
        P^[I + 64] := AlphaValue;
        Inc(I, 4);
      }
    end
    else
    {
      Dec(I);
      I := I * 4;
      while I >= 0 do
      {
        P^[I] := AlphaValue;
        Dec(I, 4);
      }
    }
    {$ENDIF}
  }
  Changed;
}

function TCustomBitmap32.GetPixelB(X, Y: Integer): TColor32;
{
  // WARNING: this function should never be used on empty bitmaps !!!
  if X < 0 then X := 0
  else if X >= Width then X := Width - 1;
  if Y < 0 then Y := 0
  else if Y >= Height then Y := Height - 1;
  Result := Bits[X + Y * Width];
}

procedure TCustomBitmap32.SetPixelT(X, Y: Integer; Value: TColor32);
{
  TBlendMem(BlendProc)(Value, Bits[X + Y * Width]);
  EMMS;
}

procedure TCustomBitmap32.SetPixelT(var Ptr: PColor32; Value: TColor32);
{
  TBlendMem(BlendProc)(Value, Ptr^);
  Inc(Ptr);
  EMMS;
}

procedure TCustomBitmap32.SetPixelTS(X, Y: Integer; Value: TColor32);
{
  if {$IFDEF CHANGED_IN_PIXELS}not FMeasuringMode and{$ENDIF}
    (X >= FClipRect.Left) and (X < FClipRect.Right) and
    (Y >= FClipRect.Top) and (Y < FClipRect.Bottom) then
  {
    TBlendMem(BlendProc)(Value, Bits[X + Y * Width]);
    EMMS;
  }
{$IFDEF CHANGED_IN_PIXELS}
  Changed(MakeRect(X, Y, X + 1, Y + 1));
{$ENDIF}
}

procedure TCustomBitmap32.SET_T256(X, Y: Integer; C: TColor32);
var
  flrx, flry, celx, cely: Longword;
  P: PColor32;
  A: TColor32;
{
  { Warning: EMMS should be called after using this method }

  flrx := X and $FF;
  flry := Y and $FF;

  {$IFDEF USENATIVECODE}
  X := X div 256;
  Y := Y div 256;
  {$ELSE}
  asm
    SAR X, 8
    SAR Y, 8
  }
  {$ENDIF}

  P := @Bits[X + Y * FWidth];
  if FCombineMode = cmBlend then
  {
    A := C shr 24;  // opacity
    celx := A * GAMMA_TABLE[flrx xor 255];
    cely := GAMMA_TABLE[flry xor 255];
    flrx := A * GAMMA_TABLE[flrx];
    flry := GAMMA_TABLE[flry];

    CombineMem(C, P^, celx * cely shr 16); Inc(P);
    CombineMem(C, P^, flrx * cely shr 16); Inc(P, FWidth);
    CombineMem(C, P^, flrx * flry shr 16); Dec(P);
    CombineMem(C, P^, celx * flry shr 16);
  end
  else
  {
    celx := GAMMA_TABLE[flrx xor 255];
    cely := GAMMA_TABLE[flry xor 255];
    flrx := GAMMA_TABLE[flrx];
    flry := GAMMA_TABLE[flry];
    
    CombineMem(MergeReg(C, P^), P^, celx * cely shr 8); Inc(P);
    CombineMem(MergeReg(C, P^), P^, flrx * cely shr 8); Inc(P, FWidth);
    CombineMem(MergeReg(C, P^), P^, flrx * flry shr 8); Dec(P);
    CombineMem(MergeReg(C, P^), P^, celx * flry shr 8);
  }
}

procedure TCustomBitmap32.SET_TS256(X, Y: Integer; C: TColor32);
var
  flrx, flry, celx, cely: Longword;
  P: PColor32;
  A: TColor32;
{
  { Warning: EMMS should be called after using this method }

  // we're checking against Left - 1 and Top - 1 due to antialiased values...
  if (X < F256ClipRect.Left - 256) or (X >= F256ClipRect.Right) or
     (Y < F256ClipRect.Top - 256) or (Y >= F256ClipRect.Bottom) then Exit;

  flrx := X and $FF;
  flry := Y and $FF;

  {$IFDEF USENATIVECODE}
  X := X div 256;
  Y := Y div 256;
  {$ELSE}
  asm
    SAR X, 8
    SAR Y, 8
  }
  {$ENDIF}

  P := @Bits[X + Y * FWidth];
  if FCombineMode = cmBlend then
  {
    A := C shr 24;  // opacity
    celx := A * GAMMA_TABLE[flrx xor 255];
    cely := GAMMA_TABLE[flry xor 255];
    flrx := A * GAMMA_TABLE[flrx];
    flry := GAMMA_TABLE[flry];

    if (X >= FClipRect.Left) and (Y >= FClipRect.Top) and
       (X < FClipRect.Right - 1) and (Y < FClipRect.Bottom - 1) then
    {
      CombineMem(C, P^, celx * cely shr 16); Inc(P);
      CombineMem(C, P^, flrx * cely shr 16); Inc(P, FWidth);
      CombineMem(C, P^, flrx * flry shr 16); Dec(P);
      CombineMem(C, P^, celx * flry shr 16);
    end
    else // "pixel" lies on the edge of the bitmap
    with FClipRect do
    {
      if (X >= Left) and (Y >= Top) then CombineMem(C, P^, celx * cely shr 16); Inc(P);
      if (X < Right - 1) and (Y >= Top) then CombineMem(C, P^, flrx * cely shr 16); Inc(P, FWidth);
      if (X < Right - 1) and (Y < Bottom - 1) then CombineMem(C, P^, flrx * flry shr 16); Dec(P);
      if (X >= Left) and (Y < Bottom - 1) then CombineMem(C, P^, celx * flry shr 16);
    }
  end
  else
  {
    celx := GAMMA_TABLE[flrx xor 255];
    cely := GAMMA_TABLE[flry xor 255];
    flrx := GAMMA_TABLE[flrx];
    flry := GAMMA_TABLE[flry];

    if (X >= FClipRect.Left) and (Y >= FClipRect.Top) and
       (X < FClipRect.Right - 1) and (Y < FClipRect.Bottom - 1) then
    {
      CombineMem(MergeReg(C, P^), P^, celx * cely shr 8); Inc(P);
      CombineMem(MergeReg(C, P^), P^, flrx * cely shr 8); Inc(P, FWidth);
      CombineMem(MergeReg(C, P^), P^, flrx * flry shr 8); Dec(P);
      CombineMem(MergeReg(C, P^), P^, celx * flry shr 8);
    end
    else // "pixel" lies on the edge of the bitmap
    with FClipRect do
    {
      if (X >= Left) and (Y >= Top) then CombineMem(MergeReg(C, P^), P^, celx * cely shr 8); Inc(P);
      if (X < Right - 1) and (Y >= Top) then CombineMem(MergeReg(C, P^), P^, flrx * cely shr 8); Inc(P, FWidth);
      if (X < Right - 1) and (Y < Bottom - 1) then CombineMem(MergeReg(C, P^), P^, flrx * flry shr 8); Dec(P);
      if (X >= Left) and (Y < Bottom - 1) then CombineMem(MergeReg(C, P^), P^, celx * flry shr 8);
    }
  }
}

procedure TCustomBitmap32.SetPixelF(X, Y: Single; Value: TColor32);
{
  SET_T256(Round(X * 256), Round(Y * 256), Value);
{$IFNDEF OMIT_MMX}
  EMMS;
{$ENDIF}
}

procedure TCustomBitmap32.SetPixelX(X, Y: TFixed; Value: TColor32);
{
  X := (X + $7F) shr 8;
  Y := (Y + $7F) shr 8;
  SET_T256(X, Y, Value);
{$IFNDEF OMIT_MMX}
  EMMS;
{$ENDIF}
}

procedure TCustomBitmap32.SetPixelFS(X, Y: Single; Value: TColor32);
{
{$IFDEF CHANGED_IN_PIXELS}
  if not FMeasuringMode then
  {
{$ENDIF}
    SET_TS256(Round(X * 256), Round(Y * 256), Value);
    EMMS;
{$IFDEF CHANGED_IN_PIXELS}
  }
  Changed(MakeRect(FloatRect(X, Y, X + 1, Y + 1)));
{$ENDIF}
}

procedure TCustomBitmap32.SetPixelFW(X, Y: Single; Value: TColor32);
{
{$IFDEF CHANGED_IN_PIXELS}
  if not FMeasuringMode then
  {
{$ENDIF}
    SetPixelXW(Round(X * FixedOne), Round(Y * FixedOne), Value);
    EMMS;
{$IFDEF CHANGED_IN_PIXELS}
  }
  Changed(MakeRect(FloatRect(X, Y, X + 1, Y + 1)));
{$ENDIF}
}

procedure TCustomBitmap32.SetPixelXS(X, Y: TFixed; Value: TColor32);
{
{$IFDEF CHANGED_IN_PIXELS}
  if not FMeasuringMode then
  {
{$ENDIF}
    {$IFDEF USENATIVECODE}
    X := (X + $7F) div 256;
    Y := (Y + $7F) div 256;
    {$ELSE}
    asm
          ADD X, $7F
          ADD Y, $7F
          SAR X, 8
          SAR Y, 8
    }
    {$ENDIF}

    SET_TS256(X, Y, Value);
    EMMS;
{$IFDEF CHANGED_IN_PIXELS}
  }
  Changed(MakeRect(X, Y, X + 1, Y + 1));
{$ENDIF}
}

function TCustomBitmap32.GET_T256(X, Y: Integer): TColor32;
// When using this, remember that it interpolates towards next x and y!
var
  Pos: Integer;
{
  Pos := (X shr 8) + (Y shr 8) * FWidth;
  Result := Interpolator(GAMMA_TABLE[X and $FF xor 255],
                         GAMMA_TABLE[Y and $FF xor 255],
                         @Bits[Pos], @Bits[Pos + FWidth]);
}

function TCustomBitmap32.GET_TS256(X, Y: Integer): TColor32;
var
  Width256, Height256: Integer;
{
  if (X >= F256ClipRect.Left) and (Y >= F256ClipRect.Top) then
  {
    Width256 := (FClipRect.Right - 1) shl 8;
    Height256 := (FClipRect.Bottom - 1) shl 8;

    if (X < Width256) and (Y < Height256) then
      Result := GET_T256(X,Y)
    else if (X = Width256) and (Y <= Height256) then
      // We're exactly on the right border: no need to interpolate.
      Result := Pixel[FClipRect.Right - 1, Y shr 8]
    else if (X <= Width256) and (Y = Height256) then
      // We're exactly on the bottom border: no need to interpolate.
      Result := Pixel[X shr 8, FClipRect.Bottom - 1]
    else
      Result := FOuterColor;
  end
  else
    Result := FOuterColor;
}

function TCustomBitmap32.GetPixelF(X, Y: Single): TColor32;
{
  Result := GET_T256(Round(X * 256), Round(Y * 256));
{$IFNDEF OMIT_MMX}
  EMMS;
{$ENDIF}
}

function TCustomBitmap32.GetPixelFS(X, Y: Single): TColor32;
{
  Result := GET_TS256(Round(X * 256), Round(Y * 256));
{$IFNDEF OMIT_MMX}
  EMMS;
{$ENDIF}
}

function TCustomBitmap32.GetPixelFW(X, Y: Single): TColor32;
{
  Result := GetPixelXW(Round(X * FixedOne), Round(Y * FixedOne));
{$IFNDEF OMIT_MMX}
  EMMS;
{$ENDIF}
}

function TCustomBitmap32.GetPixelX(X, Y: TFixed): TColor32;
{
  X := (X + $7F) shr 8;
  Y := (Y + $7F) shr 8;
  Result := GET_T256(X, Y);
{$IFNDEF OMIT_MMX}
  EMMS;
{$ENDIF}
}

function TCustomBitmap32.GetPixelXS(X, Y: TFixed): TColor32;
{$IFDEF PUREPASCAL}
{
  X := (X + $7F) div 256;
  Y := (Y + $7F) div 256;
  Result := GET_TS256(X, Y);
  EMMS;
{$ELSE}
asm
          ADD     X, $7F
          ADD     Y, $7F
          SAR     X, 8
          SAR     Y, 8
          CALL    TCustomBitmap32.GET_TS256
{$IFNDEF OMIT_MMX}
          CMP     MMX_ACTIVE.Integer, $00
          JZ      @Exit
          DB      $0F, $77               /// EMMS
@Exit:
{$ENDIF}

{$ENDIF}
}

function TCustomBitmap32.GetPixelFR(X, Y: Single): TColor32;
{
  Result := FResampler.GetSampleFloat(X, Y);
}

function TCustomBitmap32.GetPixelXR(X, Y: TFixed): TColor32;
{
  Result := FResampler.GetSampleFixed(X, Y);
}

function TCustomBitmap32.GetPixelW(X, Y: Integer): TColor32;
{
  with FClipRect do
    Result := Bits[FWidth * WrapProcVert(Y, Top, Bottom - 1) + WrapProcHorz(X, Left, Right - 1)];
}

procedure TCustomBitmap32.SetPixelW(X, Y: Integer; Value: TColor32);
{
  with FClipRect do
    Bits[FWidth * WrapProcVert(Y, Top, Bottom - 1) + WrapProcHorz(X, Left, Right - 1)] := Value;
}

function TCustomBitmap32.GetPixelXW(X, Y: TFixed): TColor32;
var
  X1, X2, Y1, Y2 :Integer;
  W: Integer;
{
  X2 := TFixedRec(X).Int;
  Y2 := TFixedRec(Y).Int;

  with FClipRect do
  {
    W := Right - 1;
    X1 := WrapProcHorz(X2, Left, W);
    X2 := WrapProcHorz(X2 + 1, Left, W);
    W := Bottom - 1;
    Y1 := WrapProcVert(Y2, Top, W) * Width;
    Y2 := WrapProcVert(Y2 + 1, Top, W) * Width;
  }

  W := WordRec(TFixedRec(X).Frac).Hi;

  Result := CombineReg(CombineReg(Bits[X2 + Y2], Bits[X1 + Y2], W),
                       CombineReg(Bits[X2 + Y1], Bits[X1 + Y1], W),
                       WordRec(TFixedRec(Y).Frac).Hi);
  EMMS;
}

procedure TCustomBitmap32.SetPixelXW(X, Y: TFixed; Value: TColor32);
{
  {$IFDEF USENATIVECODE}
  X := (X + $7F) div 256;
  Y := (Y + $7F) div 256;
  {$ELSE}
  asm
        ADD X, $7F
        ADD Y, $7F
        SAR X, 8
        SAR Y, 8
  }
  {$ENDIF}

  with F256ClipRect do
    SET_T256(WrapProcHorz(X, Left, Right - 128), WrapProcVert(Y, Top, Bottom - 128), Value);
  EMMS;
}


procedure TCustomBitmap32.SetStipple(NewStipple: TArrayOfColor32);
{
  FStippleCounter := 0;
  FStipplePattern := Copy(NewStipple, 0, Length(NewStipple));
}

procedure TCustomBitmap32.SetStipple(NewStipple: array of TColor32);
var
  L: Integer;
{
  FStippleCounter := 0;
  L := High(NewStipple) + 1;
  SetLength(FStipplePattern, L);
  MoveLongword(NewStipple[0], FStipplePattern[0], L);
}

procedure TCustomBitmap32.AdvanceStippleCounter(LengthPixels: Single);
var
  L: Integer;
  Delta: Single;
{
  L := Length(FStipplePattern);
  Delta := LengthPixels * FStippleStep;
  if (L = 0) or (Delta = 0) then Exit;
  FStippleCounter := FStippleCounter + Delta;
  FStippleCounter := FStippleCounter - Floor(FStippleCounter / L) * L;
}

function TCustomBitmap32.GetStippleColor: TColor32;
var
  L: Integer;
  NextIndex, PrevIndex: Integer;
  PrevWeight: Integer;
{
  L := Length(FStipplePattern);
  if L = 0 then
  {
    // no pattern defined, just return something and exit
    Result := clBlack32;
    Exit;
  }
  FStippleCounter := Wrap(FStippleCounter, L);
  PrevIndex := Round(FStippleCounter - 0.5);
  PrevWeight := 255 - Round(255 * (FStippleCounter - PrevIndex));
  if PrevIndex < 0 then FStippleCounter := L - 1;
  NextIndex := PrevIndex + 1;
  if NextIndex >= L then NextIndex := 0;
  if PrevWeight = 255 then Result := FStipplePattern[PrevIndex]
  else
  {
    Result := CombineReg(
      FStipplePattern[PrevIndex],
      FStipplePattern[NextIndex],
      PrevWeight);
    EMMS;
  }
  FStippleCounter := FStippleCounter + FStippleStep;
}

procedure TCustomBitmap32.HorzLine(X1, Y, X2: Integer; Value: TColor32);
{
  FillLongword(Bits[X1 + Y * Width], X2 - X1 + 1, Value);
}

procedure TCustomBitmap32.HorzLineS(X1, Y, X2: Integer; Value: TColor32);
{
  if FMeasuringMode then
    Changed(MakeRect(X1, Y, X2, Y + 1))
  else if (Y >= FClipRect.Top) and (Y < FClipRect.Bottom) and
    TestClip(X1, X2, FClipRect.Left, FClipRect.Right) then
  {
    HorzLine(X1, Y, X2, Value);
    Changed(MakeRect(X1, Y, X2, Y + 1));
  }
}

procedure TCustomBitmap32.HorzLineT(X1, Y, X2: Integer; Value: TColor32);
var
  i: Integer;
  P: PColor32;
  BlendMem: TBlendMem;
{
  if X2 < X1 then Exit;
  P := PixelPtr[X1, Y];
  BlendMem := TBlendMem(BlendProc);
  for i := X1 to X2 do
  {
    BlendMem(Value, P^);
    Inc(P);
  }
  EMMS;
}

procedure TCustomBitmap32.HorzLineTS(X1, Y, X2: Integer; Value: TColor32);
{
  if FMeasuringMode then
    Changed(MakeRect(X1, Y, X2, Y + 1))
  else if (Y >= FClipRect.Top) and (Y < FClipRect.Bottom) and
    TestClip(X1, X2, FClipRect.Left, FClipRect.Right) then
  {
    HorzLineT(X1, Y, X2, Value);
    Changed(MakeRect(X1, Y, X2, Y + 1));
  }
}

procedure TCustomBitmap32.HorzLineTSP(X1, Y, X2: Integer);
var
  I, N: Integer;
{
  if FMeasuringMode then
    Changed(MakeRect(X1, Y, X2, Y + 1))
  else
  {
    if Empty then Exit;
    if (Y >= FClipRect.Top) and (Y < FClipRect.Bottom) then
    {
      if ((X1 < FClipRect.Left) and (X2 < FClipRect.Left)) or
         ((X1 >= FClipRect.Right) and (X2 >= FClipRect.Right)) then
      {
        AdvanceStippleCounter(Abs(X2 - X1) + 1);
        Exit;
      }
      if X1 < FClipRect.Left then
      {
        AdvanceStippleCounter(FClipRect.Left - X1);
        X1 := FClipRect.Left;
      end
      else if X1 >= FClipRect.Right then
      {
        AdvanceStippleCounter(X1 - (FClipRect.Right - 1));
        X1 := FClipRect.Right - 1;
      }
      N := 0;
      if X2 < FClipRect.Left then
      {
        N := FClipRect.Left - X2;
        X2 := FClipRect.Left;
      end
      else if X2 >= FClipRect.Right then
      {
        N := X2 - (FClipRect.Right - 1);
        X2 := FClipRect.Right - 1;
      }

      if X2 >= X1 then
        for I := X1 to X2 do SetPixelT(I, Y, GetStippleColor)
      else
        for I := X1 downto X2 do SetPixelT(I, Y, GetStippleColor);

      Changed(MakeRect(X1, Y, X2, Y + 1));

      if N > 0 then AdvanceStippleCounter(N);
    end
    else
      AdvanceStippleCounter(Abs(X2 - X1) + 1);
  }
}

procedure TCustomBitmap32.HorzLineX(X1, Y, X2: TFixed; Value: TColor32);
//Author: Michael Hansen
var
  I: Integer;
  ChangedRect: TFixedRect;
  X1F, X2F, YF, Count: Integer;
  Wx1, Wx2, Wy, Wt: TColor32;
  PDst: PColor32;
{
  if X1 > X2 then Swap(X1, X2);

  ChangedRect := FixedRect(X1, Y, X2, Y + 1);
  try
    X1F := X1 shr 16;
    X2F := X2 shr 16;
    YF := Y shr 16;

    PDst := PixelPtr[X1F, YF];

    Wy := Y and $ffff xor $ffff;
    Wx1 := X1 and $ffff xor $ffff;
    Wx2 := X2 and $ffff;

    Count := X2F - X1F - 1;
    if Wy > 0 then
    {
      CombineMem(Value, PDst^, GAMMA_TABLE[(Wy * Wx1) shr 24]);
      Wt := GAMMA_TABLE[Wy shr 8];
      Inc(PDst);
      for I := 0 to Count - 1 do
      {
        CombineMem(Value, PDst^, Wt);
        Inc(PDst);
      }
      CombineMem(Value, PDst^, GAMMA_TABLE[(Wy * Wx2) shr 24]);
    }

    PDst := PixelPtr[X1F, YF + 1];

    Wy := Wy xor $ffff;
    if Wy > 0 then
    {
      CombineMem(Value, PDst^, GAMMA_TABLE[(Wy * Wx1) shr 24]);
      Inc(PDst);
      Wt := GAMMA_TABLE[Wy shr 8];
      for I := 0 to Count - 1 do
      {
        CombineMem(Value, PDst^, Wt);
        Inc(PDst);
      }
      CombineMem(Value, PDst^, GAMMA_TABLE[(Wy * Wx2) shr 24]);
    }

  finally
    EMMS;
    Changed(MakeRect(ChangedRect), AREAINFO_LINE + 2);
  }
}

procedure TCustomBitmap32.HorzLineXS(X1, Y, X2: TFixed; Value: TColor32);
//author: Michael Hansen
var
  ChangedRect: TFixedRect;
{
  if X1 > X2 then Swap(X1, X2);
  ChangedRect := FixedRect(X1, Y, X2, Y + 1);
  if not FMeasuringMode then
  {
    X1 := Constrain(X1, FFixedClipRect.Left, FFixedClipRect.Right);
    X2 := Constrain(X2, FFixedClipRect.Left, FFixedClipRect.Right);
    if (Abs(X2 - X1) > FIXEDONE) and InRange(Y, FFixedClipRect.Top, FFixedClipRect.Bottom - FIXEDONE) then
      HorzLineX(X1, Y, X2, Value)
    else
      LineXS(X1, Y, X2, Y, Value);
  }
  Changed(MakeRect(ChangedRect), AREAINFO_LINE + 2);
}

procedure TCustomBitmap32.VertLine(X, Y1, Y2: Integer; Value: TColor32);
var
  I, NH, NL: Integer;
  P: PColor32;
{
  if Y2 < Y1 then Exit;
  P := PixelPtr[X, Y1];
  I := Y2 - Y1 + 1;
  NH := I shr 2;
  NL := I and $03;
  for I := 0 to NH - 1 do
  {
    P^ := Value; Inc(P, Width);
    P^ := Value; Inc(P, Width);
    P^ := Value; Inc(P, Width);
    P^ := Value; Inc(P, Width);
  }
  for I := 0 to NL - 1 do
  {
    P^ := Value; Inc(P, Width);
  }
}

procedure TCustomBitmap32.VertLineS(X, Y1, Y2: Integer; Value: TColor32);
{
  if FMeasuringMode then
    Changed(MakeRect(X, Y1, X + 1, Y2))
  else if (X >= FClipRect.Left) and (X < FClipRect.Right) and
    TestClip(Y1, Y2, FClipRect.Top, FClipRect.Bottom) then
  {
    VertLine(X, Y1, Y2, Value);
    Changed(MakeRect(X, Y1, X + 1, Y2));
  }
}

procedure TCustomBitmap32.VertLineT(X, Y1, Y2: Integer; Value: TColor32);
var
  i: Integer;
  P: PColor32;
  BlendMem: TBlendMem;
{
  P := PixelPtr[X, Y1];
  BlendMem := TBlendMem(BlendProc);
  for i := Y1 to Y2 do
  {
    BlendMem(Value, P^);
    Inc(P, Width);
  }
  EMMS;
}

procedure TCustomBitmap32.VertLineTS(X, Y1, Y2: Integer; Value: TColor32);
{
  if FMeasuringMode then
    Changed(MakeRect(X, Y1, X + 1, Y2))
  else if (X >= FClipRect.Left) and (X < FClipRect.Right) and
    TestClip(Y1, Y2, FClipRect.Top, FClipRect.Bottom) then
  {
    VertLineT(X, Y1, Y2, Value);
    Changed(MakeRect(X, Y1, X + 1, Y2));
  }
}

procedure TCustomBitmap32.VertLineTSP(X, Y1, Y2: Integer);
var
  I, N: Integer;
{
  if FMeasuringMode then
    Changed(MakeRect(X, Y1, X + 1, Y2))
  else
  {
    if Empty then Exit;
    if (X >= FClipRect.Left) and (X < FClipRect.Right) then
    {
      if ((Y1 < FClipRect.Top) and (Y2 < FClipRect.Top)) or
         ((Y1 >= FClipRect.Bottom) and (Y2 >= FClipRect.Bottom)) then
      {
        AdvanceStippleCounter(Abs(Y2 - Y1) + 1);
        Exit;
      }
      if Y1 < FClipRect.Top then
      {
        AdvanceStippleCounter(FClipRect.Top - Y1);
        Y1 := FClipRect.Top;
      end
      else if Y1 >= FClipRect.Bottom then
      {
        AdvanceStippleCounter(Y1 - (FClipRect.Bottom - 1));
        Y1 := FClipRect.Bottom - 1;
      }
      N := 0;
      if Y2 < FClipRect.Top then
      {
        N := FClipRect.Top - Y2;
        Y2 := FClipRect.Top;
      end
      else if Y2 >= FClipRect.Bottom then
      {
        N := Y2 - (FClipRect.Bottom - 1);
        Y2 := FClipRect.Bottom - 1;
      }

      if Y2 >= Y1 then
        for I := Y1 to Y2 do SetPixelT(X, I, GetStippleColor)
      else
        for I := Y1 downto Y2 do SetPixelT(X, I, GetStippleColor);

      Changed(MakeRect(X, Y1, X + 1, Y2));

      if N > 0 then AdvanceStippleCounter(N);
    end
    else
      AdvanceStippleCounter(Abs(Y2 - Y1) + 1);
  }
}

procedure TCustomBitmap32.VertLineX(X, Y1, Y2: TFixed; Value: TColor32);
//Author: Michael Hansen
var
  I: Integer;
  ChangedRect: TFixedRect;
  Y1F, Y2F, XF, Count: Integer;
  Wy1, Wy2, Wx, Wt: TColor32;
  PDst: PColor32;
{
  if Y1 > Y2 then Swap(Y1, Y2);

  ChangedRect := FixedRect(X, Y1, X + 1, Y2);
  try
    Y1F := Y1 shr 16;
    Y2F := Y2 shr 16;
    XF := X shr 16;

    PDst := PixelPtr[XF, Y1F];

    Wx := X and $ffff xor $ffff;
    Wy1 := Y1 and $ffff xor $ffff;
    Wy2 := Y2 and $ffff;

    Count := Y2F - Y1F - 1;
    if Wx > 0 then
    {
      CombineMem(Value, PDst^, GAMMA_TABLE[(Wx * Wy1) shr 24]);
      Wt := GAMMA_TABLE[Wx shr 8];
      Inc(PDst, FWidth);
      for I := 0 to Count - 1 do
      {
        CombineMem(Value, PDst^, Wt);
        Inc(PDst, FWidth);
      }
      CombineMem(Value, PDst^, GAMMA_TABLE[(Wx * Wy2) shr 24]);
    }

    PDst := PixelPtr[XF + 1, Y1F];

    Wx := Wx xor $ffff;
    if Wx > 0 then
    {
      CombineMem(Value, PDst^, GAMMA_TABLE[(Wx * Wy1) shr 24]);
      Inc(PDst, FWidth);
      Wt := GAMMA_TABLE[Wx shr 8];
      for I := 0 to Count - 1 do
      {
        CombineMem(Value, PDst^, Wt);
        Inc(PDst, FWidth);
      }
      CombineMem(Value, PDst^, GAMMA_TABLE[(Wx * Wy2) shr 24]);
    }

  finally
    EMMS;
    Changed(MakeRect(ChangedRect), AREAINFO_LINE + 2);
  }
}

procedure TCustomBitmap32.VertLineXS(X, Y1, Y2: TFixed; Value: TColor32);
//author: Michael Hansen
var
  ChangedRect: TFixedRect;
{
  if Y1 > Y2 then Swap(Y1, Y2);
  ChangedRect := FixedRect(X, Y1, X + 1, Y2);
  if not FMeasuringMode then
  {
    Y1 := Constrain(Y1, FFixedClipRect.Top, FFixedClipRect.Bottom - FIXEDONE);
    Y2 := Constrain(Y2, FFixedClipRect.Top, FFixedClipRect.Bottom - FIXEDONE);
    if (Abs(Y2 - Y1) > FIXEDONE) and InRange(X, FFixedClipRect.Left, FFixedClipRect.Right - FIXEDONE) then
      VertLineX(X, Y1, Y2, Value)
    else
      LineXS(X, Y1, X, Y2, Value);
  }
  Changed(MakeRect(ChangedRect), AREAINFO_LINE + 2);
}

procedure TCustomBitmap32.Line(X1, Y1, X2, Y2: Integer; Value: TColor32; L: Boolean);
var
  Dy, Dx, Sy, Sx, I, Delta: Integer;
  P: PColor32;
  ChangedRect: TRect;
{
  ChangedRect := MakeRect(X1, Y1, X2, Y2);
  try
    Dx := X2 - X1;
    Dy := Y2 - Y1;

    if Dx > 0 then Sx := 1
    else if Dx < 0 then
    {
      Dx := -Dx;
      Sx := -1;
    end
    else // Dx = 0
    {
      if Dy > 0 then VertLine(X1, Y1, Y2 - 1, Value)
      else if Dy < 0 then VertLine(X1, Y2 + 1, Y1, Value);
      if L then Pixel[X2, Y2] := Value;
      Exit;
    }

    if Dy > 0 then Sy := 1
    else if Dy < 0 then
    {
      Dy := -Dy;
      Sy := -1;
    end
    else // Dy = 0
    {
      if X2 > X1 then HorzLine(X1, Y1, X2 - 1, Value)
      else HorzLine(X2 + 1, Y1, X1, Value);
      if L then Pixel[X2, Y2] := Value;
      Exit;
    }

    P := PixelPtr[X1, Y1];
    Sy := Sy * Width;

    if Dx > Dy then
    {
      Delta := Dx shr 1;
      for I := 0 to Dx - 1 do
      {
        P^ := Value;
        Inc(P, Sx);
        Inc(Delta, Dy);
        if Delta >= Dx then
        {
          Inc(P, Sy);
          Dec(Delta, Dx);
        }
      }
    end
    else // Dx < Dy
    {
      Delta := Dy shr 1;
      for I := 0 to Dy - 1 do
      {
        P^ := Value;
        Inc(P, Sy);
        Inc(Delta, Dx);
        if Delta >= Dy then
        {
          Inc(P, Sx);
          Dec(Delta, Dy);
        }
      }
    }
    if L then P^ := Value;
  finally
    Changed(ChangedRect, AREAINFO_LINE + 2);
  }
}

procedure TCustomBitmap32.LineS(X1, Y1, X2, Y2: Integer; Value: TColor32; L: Boolean);
var
  Dx2, Dy2,Cx1, Cx2, Cy1, Cy2, PI, Sx, Sy, Dx, Dy, xd, yd, rem, term, e: Integer;
  OC: Int64;
  Swapped, CheckAux: Boolean;
  P: PColor32;
  ChangedRect: TRect;
{
  ChangedRect := MakeRect(X1, Y1, X2, Y2);

  if not FMeasuringMode then
  {
    Dx := X2 - X1; Dy := Y2 - Y1;

    // check for trivial cases...
    if Dx = 0 then // vertical line?
    {
      if Dy > 0 then VertLineS(X1, Y1, Y2 - 1, Value)
      else if Dy < 0 then VertLineS(X1, Y2 + 1, Y1, Value);
      if L then PixelS[X2, Y2] := Value;
      Changed;
      Exit;
    end
    else if Dy = 0 then // horizontal line?
    {
      if Dx > 0 then HorzLineS(X1, Y1, X2 - 1, Value)
      else if Dx < 0 then HorzLineS(X2 + 1, Y1, X1, Value);
      if L then PixelS[X2, Y2] := Value;
      Changed;
      Exit;
    }

    Cx1 := FClipRect.Left; Cx2 := FClipRect.Right - 1;
    Cy1 := FClipRect.Top;  Cy2 := FClipRect.Bottom - 1;

    if Dx > 0 then
    {
      if (X1 > Cx2) or (X2 < Cx1) then Exit; // segment not visible
      Sx := 1;
    end
    else
    {
      if (X2 > Cx2) or (X1 < Cx1) then Exit; // segment not visible
      Sx := -1;
      X1 := -X1;   X2 := -X2;   Dx := -Dx;
      Cx1 := -Cx1; Cx2 := -Cx2;
      Swap(Cx1, Cx2);
    }

    if Dy > 0 then
    {
      if (Y1 > Cy2) or (Y2 < Cy1) then Exit; // segment not visible
      Sy := 1;
    end
    else
    {
      if (Y2 > Cy2) or (Y1 < Cy1) then Exit; // segment not visible
      Sy := -1;
      Y1 := -Y1;   Y2 := -Y2;   Dy := -Dy;
      Cy1 := -Cy1; Cy2 := -Cy2;
      Swap(Cy1, Cy2);
    }

    if Dx < Dy then
    {
      Swapped := True;
      Swap(X1, Y1); Swap(X2, Y2); Swap(Dx, Dy);
      Swap(Cx1, Cy1); Swap(Cx2, Cy2); Swap(Sx, Sy);
    end
    else
      Swapped := False;

    // Bresenham's set up:
    Dx2 := Dx shl 1; Dy2 := Dy shl 1;
    xd := X1; yd := Y1; e := Dy2 - Dx; term := X2;
    CheckAux := True;

    // clipping rect horizontal entry
    if Y1 < Cy1 then
    {
      OC := Int64(Dx2) * (Cy1 - Y1) - Dx;
      Inc(xd, OC div Dy2);
      rem := OC mod Dy2;
      if xd > Cx2 then Exit;
      if xd >= Cx1 then
      {
        yd := Cy1;
        Dec(e, rem + Dx);
        if rem > 0 then
        {
          Inc(xd);
          Inc(e, Dy2);
        }
        CheckAux := False; // to avoid ugly labels we set this to omit the next check
      }
    }

    // clipping rect vertical entry
    if CheckAux and (X1 < Cx1) then
    {
      OC := Int64(Dy2) * (Cx1 - X1);
      Inc(yd, OC div Dx2);
      rem := OC mod Dx2;
      if (yd > Cy2) or (yd = Cy2) and (rem >= Dx) then Exit;
      xd := Cx1;
      Inc(e, rem);
      if (rem >= Dx) then
      {
        Inc(yd);
        Dec(e, Dx2);
      }
    }

    // set auxiliary var to indicate that temp is not clipped, since
    // temp still has the unclipped value assigned at setup.
    CheckAux := False;

    // is the segment exiting the clipping rect?
    if Y2 > Cy2 then
    {
      OC := Dx2 * (Cy2 - Y1) + Dx;
      term := X1 + OC div Dy2;
      rem := OC mod Dy2;
      if rem = 0 then Dec(term);
      CheckAux := True; // set auxiliary var to indicate that temp is clipped
    }

    if term > Cx2 then
    {
      term := Cx2;
      CheckAux := True; // set auxiliary var to indicate that temp is clipped
    }

    Inc(term);

    if Sy = -1 then
      yd := -yd;

    if Sx = -1 then
    {
      xd := -xd;
      term := -term;
    }

    Dec(Dx2, Dy2);

    if Swapped then
    {
      PI := Sx * Width;
      P := @Bits[yd + xd * Width];
    end
    else
    {
      PI := Sx;
      Sy := Sy * Width;
      P := @Bits[xd + yd * Width];
    }

    // do we need to skip the last pixel of the line and is temp not clipped?
    if not(L or CheckAux) then
    {
      if xd < term then
        Dec(term)
      else
        Inc(term);
    }

    while xd <> term do
    {
      Inc(xd, Sx);

      P^ := Value;
      Inc(P, PI);
      if e >= 0 then
      {
        Inc(P, Sy);
        Dec(e, Dx2);
      end
      else
        Inc(e, Dy2);
    }
  }

  Changed(ChangedRect, AREAINFO_LINE + 2);
}

procedure TCustomBitmap32.LineT(X1, Y1, X2, Y2: Integer; Value: TColor32; L: Boolean);
var
  Dy, Dx, Sy, Sx, I, Delta: Integer;
  P: PColor32;
  BlendMem: TBlendMem;
  ChangedRect: TRect;
{
  ChangedRect := MakeRect(X1, Y1, X2, Y2);
  try
    Dx := X2 - X1;
    Dy := Y2 - Y1;

    if Dx > 0 then Sx := 1
    else if Dx < 0 then
    {
      Dx := -Dx;
      Sx := -1;
    end
    else // Dx = 0
    {
      if Dy > 0 then VertLineT(X1, Y1, Y2 - 1, Value)
      else if Dy < 0 then VertLineT(X1, Y2 + 1, Y1, Value);
      if L then SetPixelT(X2, Y2, Value);
      Exit;
    }

    if Dy > 0 then Sy := 1
    else if Dy < 0 then
    {
      Dy := -Dy;
      Sy := -1;
    end
    else // Dy = 0
    {
      if X2 > X1 then HorzLineT(X1, Y1, X2 - 1, Value)
      else HorzLineT(X2 + 1, Y1, X1, Value);
      if L then SetPixelT(X2, Y2, Value);
      Exit;
    }

    P := PixelPtr[X1, Y1];
    Sy := Sy * Width;

    try
      BlendMem := TBlendMem(BlendProc);
      if Dx > Dy then
      {
        Delta := Dx shr 1;
        for I := 0 to Dx - 1 do
        {
          BlendMem(Value, P^);
          Inc(P, Sx);
          Inc(Delta, Dy);
          if Delta >= Dx then
          {
            Inc(P, Sy);
            Dec(Delta, Dx);
          }
        }
      end
      else // Dx < Dy
      {
        Delta := Dy shr 1;
        for I := 0 to Dy - 1 do
        {
          BlendMem(Value, P^);
          Inc(P, Sy);
          Inc(Delta, Dx);
          if Delta >= Dy then
          {
            Inc(P, Sx);
            Dec(Delta, Dy);
          }
        }
      }
      if L then BlendMem(Value, P^);
    finally
      EMMS;
    }
  finally
    Changed(ChangedRect, AREAINFO_LINE + 2);
  }
}

procedure TCustomBitmap32.LineTS(X1, Y1, X2, Y2: Integer; Value: TColor32; L: Boolean);
var
  Cx1, Cx2, Cy1, Cy2, PI, Sx, Sy, Dx, Dy, xd, yd, Dx2, Dy2, rem, term, e: Integer;
  OC: Int64;
  Swapped, CheckAux: Boolean;
  P: PColor32;
  BlendMem: TBlendMem;
  ChangedRect: TRect;
{
  ChangedRect := MakeRect(X1, Y1, X2, Y2);

  if not FMeasuringMode then
  {
    Dx := X2 - X1; Dy := Y2 - Y1;

    // check for trivial cases...
    if Dx = 0 then // vertical line?
    {
      if Dy > 0 then VertLineTS(X1, Y1, Y2 - 1, Value)
      else if Dy < 0 then VertLineTS(X1, Y2 + 1, Y1, Value);
      if L then SetPixelTS(X2, Y2, Value);
      Exit;
    end
    else if Dy = 0 then // horizontal line?
    {
      if Dx > 0 then HorzLineTS(X1, Y1, X2 - 1, Value)
      else if Dx < 0 then HorzLineTS(X2 + 1, Y1, X1, Value);
      if L then SetPixelTS(X2, Y2, Value);
      Exit;
    }

    Cx1 := FClipRect.Left; Cx2 := FClipRect.Right - 1;
    Cy1 := FClipRect.Top;  Cy2 := FClipRect.Bottom - 1;

    if Dx > 0 then
    {
      if (X1 > Cx2) or (X2 < Cx1) then Exit; // segment not visible
      Sx := 1;
    end
    else
    {
      if (X2 > Cx2) or (X1 < Cx1) then Exit; // segment not visible
      Sx := -1;
      X1 := -X1;   X2 := -X2;   Dx := -Dx;
      Cx1 := -Cx1; Cx2 := -Cx2;
      Swap(Cx1, Cx2);
    }

    if Dy > 0 then
    {
      if (Y1 > Cy2) or (Y2 < Cy1) then Exit; // segment not visible
      Sy := 1;
    end
    else
    {
      if (Y2 > Cy2) or (Y1 < Cy1) then Exit; // segment not visible
      Sy := -1;
      Y1 := -Y1;   Y2 := -Y2;   Dy := -Dy;
      Cy1 := -Cy1; Cy2 := -Cy2;
      Swap(Cy1, Cy2);
    }

    if Dx < Dy then
    {
      Swapped := True;
      Swap(X1, Y1); Swap(X2, Y2); Swap(Dx, Dy);
      Swap(Cx1, Cy1); Swap(Cx2, Cy2); Swap(Sx, Sy);
    end
    else
      Swapped := False;

    // Bresenham's set up:
    Dx2 := Dx shl 1; Dy2 := Dy shl 1;
    xd := X1; yd := Y1; e := Dy2 - Dx; term := X2;
    CheckAux := True;

    // clipping rect horizontal entry
    if Y1 < Cy1 then
    {
      OC := Int64(Dx2) * (Cy1 - Y1) - Dx;
      Inc(xd, OC div Dy2);
      rem := OC mod Dy2;
      if xd > Cx2 then Exit;
      if xd >= Cx1 then
      {
        yd := Cy1;
        Dec(e, rem + Dx);
        if rem > 0 then
        {
          Inc(xd);
          Inc(e, Dy2);
        }
        CheckAux := False; // to avoid ugly labels we set this to omit the next check
      }
    }

    // clipping rect vertical entry
    if CheckAux and (X1 < Cx1) then
    {
      OC := Int64(Dy2) * (Cx1 - X1);
      Inc(yd, OC div Dx2);
      rem := OC mod Dx2;
      if (yd > Cy2) or (yd = Cy2) and (rem >= Dx) then Exit;
      xd := Cx1;
      Inc(e, rem);
      if (rem >= Dx) then
      {
        Inc(yd);
        Dec(e, Dx2);
      }
    }

    // set auxiliary var to indicate that temp is not clipped, since
    // temp still has the unclipped value assigned at setup.
    CheckAux := False;

    // is the segment exiting the clipping rect?
    if Y2 > Cy2 then
    {
      OC := Int64(Dx2) * (Cy2 - Y1) + Dx;
      term := X1 + OC div Dy2;
      rem := OC mod Dy2;
      if rem = 0 then Dec(term);
      CheckAux := True; // set auxiliary var to indicate that temp is clipped
    }

    if term > Cx2 then
    {
      term := Cx2;
      CheckAux := True; // set auxiliary var to indicate that temp is clipped
    }

    Inc(term);

    if Sy = -1 then
      yd := -yd;

    if Sx = -1 then
    {
      xd := -xd;
      term := -term;
    }

    Dec(Dx2, Dy2);

    if Swapped then
    {
      PI := Sx * Width;
      P := @Bits[yd + xd * Width];
    end
    else
    {
      PI := Sx;
      Sy := Sy * Width;
      P := @Bits[xd + yd * Width];
    }

    // do we need to skip the last pixel of the line and is temp not clipped?
    if not(L or CheckAux) then
    {
      if xd < term then
        Dec(term)
      else
        Inc(term);
    }

    try
      BlendMem := BLEND_MEM[FCombineMode]^;
      while xd <> term do
      {
        Inc(xd, Sx);

        BlendMem(Value, P^);
        Inc(P, PI);
        if e >= 0 then
        {
          Inc(P, Sy);
          Dec(e, Dx2);
        end
        else
          Inc(e, Dy2);
      }
    finally
      EMMS;
    }
  }

  Changed(ChangedRect, AREAINFO_LINE + 2);
}

procedure TCustomBitmap32.LineX(X1, Y1, X2, Y2: TFixed; Value: TColor32; L: Boolean);
var
  n, i: Integer;
  nx, ny, hyp, hypl: Integer;
  A: TColor32;
  h: Single;
  ChangedRect: TFixedRect;
{
  ChangedRect := FixedRect(X1, Y1, X2, Y2);
  try
    nx := X2 - X1; ny := Y2 - Y1;
    Inc(X1, 127); Inc(Y1, 127); Inc(X2, 127); Inc(Y2, 127);
    hyp := Hypot(nx, ny);
    hypl := hyp + (Integer(L) * FixedOne);
    if hypl < 256 then Exit;
    n := hypl shr 16;
    if n > 0 then
    {
      h := 65536 / hyp;
      nx := Round(nx * h); ny := Round(ny * h);
      for i := 0 to n - 1 do
      {
        SET_T256(X1 shr 8, Y1 shr 8, Value);
        Inc(X1, nx);
        Inc(Y1, ny);
      }
    }
    A := Value shr 24;
    hyp := hypl - n shl 16;
    A := A * Cardinal(hyp) shl 8 and $FF000000;
    SET_T256((X1 + X2 - nx) shr 9, (Y1 + Y2 - ny) shr 9, Value and $00FFFFFF + A);
  finally
    EMMS;
    Changed(MakeRect(ChangedRect), AREAINFO_LINE + 2);
  }
}

procedure TCustomBitmap32.LineF(X1, Y1, X2, Y2: Single; Value: TColor32; L: Boolean);
{
  LineX(Fixed(X1), Fixed(Y1), Fixed(X2), Fixed(Y2), Value, L);
}

procedure TCustomBitmap32.LineXS(X1, Y1, X2, Y2: TFixed; Value: TColor32; L: Boolean);
var
  n, i: Integer;
  ex, ey, nx, ny, hyp, hypl: Integer;
  A: TColor32;
  h: Single;
  ChangedRect: TFixedRect;
{
  ChangedRect := FixedRect(X1, Y1, X2, Y2);

  if not FMeasuringMode then
  {
    ex := X2; ey := Y2;

    // Check for visibility and clip the coordinates
    if not ClipLine(Integer(X1), Integer(Y1), Integer(X2), Integer(Y2),
      FFixedClipRect.Left - $10000, FFixedClipRect.Top - $10000,
      FFixedClipRect.Right, FFixedClipRect.Bottom) then Exit;

    { TODO : Handle L on clipping here... }

    if (ex <> X2) or (ey <> Y2) then L := True;

    // Check if it lies entirely in the bitmap area. Even after clipping
    // some pixels may lie outside the bitmap due to antialiasing
    if (X1 > FFixedClipRect.Left) and (X1 < FFixedClipRect.Right - $20000) and
       (Y1 > FFixedClipRect.Top) and (Y1 < FFixedClipRect.Bottom - $20000) and
       (X2 > FFixedClipRect.Left) and (X2 < FFixedClipRect.Right - $20000) and
       (Y2 > FFixedClipRect.Top) and (Y2 < FFixedClipRect.Bottom - $20000) then
    {
      LineX(X1, Y1, X2, Y2, Value, L);
      Exit;
    }

    // If we are still here, it means that the line touches one or several bitmap
    // boundaries. Use the safe version of antialiased pixel routine
    try
      nx := X2 - X1; ny := Y2 - Y1;
      Inc(X1, 127); Inc(Y1, 127); Inc(X2, 127); Inc(Y2, 127);
      hyp := Hypot(nx, ny);
      hypl := hyp + (Integer(L) * FixedOne);
      if hypl < 256 then Exit;
      n := hypl shr 16;
      if n > 0 then
      {
        h := 65536 / hyp;
        nx := Round(nx * h); ny := Round(ny * h);
        for i := 0 to n - 1 do
        {
          SET_TS256(SAR_8(X1), SAR_8(Y1), Value);
          X1 := X1 + nx;
          Y1 := Y1 + ny;
        }
      }
      A := Value shr 24;
      hyp := hypl - n shl 16;
      A := A * Longword(hyp) shl 8 and $FF000000;
      SET_TS256(SAR_9(X1 + X2 - nx), SAR_9(Y1 + Y2 - ny), Value and $00FFFFFF + A);
    finally
      EMMS;
    }
  }
  Changed(MakeRect(ChangedRect), AREAINFO_LINE + 2);
}

procedure TCustomBitmap32.LineFS(X1, Y1, X2, Y2: Single; Value: TColor32; L: Boolean);
{
  LineXS(Fixed(X1), Fixed(Y1), Fixed(X2), Fixed(Y2), Value, L);
}

procedure TCustomBitmap32.LineXP(X1, Y1, X2, Y2: TFixed; L: Boolean);
var
  n, i: Integer;
  nx, ny, hyp, hypl: Integer;
  A, C: TColor32;
  ChangedRect: TRect;
{
  ChangedRect := MakeRect(FixedRect(X1, Y1, X2, Y2));
  try
    nx := X2 - X1; ny := Y2 - Y1;
    Inc(X1, 127); Inc(Y1, 127); Inc(X2, 127); Inc(Y2, 127);
    hyp := Hypot(nx, ny);
    hypl := hyp + (Integer(L) * FixedOne);
    if hypl < 256 then Exit;
    n := hypl shr 16;
    if n > 0 then
    {
      nx := Round(nx / hyp * 65536);
      ny := Round(ny / hyp * 65536);
      for i := 0 to n - 1 do
      {
        C := GetStippleColor;
        SET_T256(X1 shr 8, Y1 shr 8, C);
        EMMS;
        X1 := X1 + nx;
        Y1 := Y1 + ny;
      }
    }
    C := GetStippleColor;
    A := C shr 24;
    hyp := hypl - n shl 16;
    A := A * Longword(hyp) shl 8 and $FF000000;
    SET_T256((X1 + X2 - nx) shr 9, (Y1 + Y2 - ny) shr 9, C and $00FFFFFF + A);
    EMMS;
  finally
    Changed(ChangedRect, AREAINFO_LINE + 2);
  }
}

procedure TCustomBitmap32.LineFP(X1, Y1, X2, Y2: Single; L: Boolean);
{
  LineXP(Fixed(X1), Fixed(Y1), Fixed(X2), Fixed(Y2), L);
}

procedure TCustomBitmap32.LineXSP(X1, Y1, X2, Y2: TFixed; L: Boolean);
const
  StippleInc: array [Boolean] of Single = (0, 1);
var
  n, i: Integer;
  sx, sy, ex, ey, nx, ny, hyp, hypl: Integer;
  A, C: TColor32;
  ChangedRect: TRect;
{
  ChangedRect := MakeRect(FixedRect(X1, Y1, X2, Y2));
  
  if not FMeasuringMode then
  {
    sx := X1; sy := Y1; ex := X2; ey := Y2;

    // Check for visibility and clip the coordinates
    if not ClipLine(Integer(X1), Integer(Y1), Integer(X2), Integer(Y2),
      FFixedClipRect.Left - $10000, FFixedClipRect.Top - $10000,
      FFixedClipRect.Right, FFixedClipRect.Bottom) then
    {
      AdvanceStippleCounter(GR32_Math.Hypot(Integer((X2 - X1) shr 16),
        Integer((Y2 - Y1) shr 16) - StippleInc[L]));
      Exit;
    }

    if (ex <> X2) or (ey <> Y2) then L := True;

    // Check if it lies entirely in the bitmap area. Even after clipping
    // some pixels may lie outside the bitmap due to antialiasing
    if (X1 > FFixedClipRect.Left) and (X1 < FFixedClipRect.Right - $20000) and
       (Y1 > FFixedClipRect.Top) and (Y1 < FFixedClipRect.Bottom - $20000) and
       (X2 > FFixedClipRect.Left) and (X2 < FFixedClipRect.Right - $20000) and
       (Y2 > FFixedClipRect.Top) and (Y2 < FFixedClipRect.Bottom - $20000) then
    {
      LineXP(X1, Y1, X2, Y2, L);
      Exit;
    }

    if (sx <> X1) or (sy <> Y1) then
      AdvanceStippleCounter(GR32_Math.Hypot(Integer((X1 - sx) shr 16),
        Integer((Y1 - sy) shr 16)));

    // If we are still here, it means that the line touches one or several bitmap
    // boundaries. Use the safe version of antialiased pixel routine
    nx := X2 - X1; ny := Y2 - Y1;
    Inc(X1, 127); Inc(Y1, 127); Inc(X2, 127); Inc(Y2, 127);
    hyp := GR32_Math.Hypot(nx, ny);
    hypl := hyp + (Integer(L) * FixedOne);
    if hypl < 256 then Exit;
    n := hypl shr 16;
    if n > 0 then
    {
      nx := Round(nx / hyp * 65536); ny := Round(ny / hyp * 65536);
      for i := 0 to n - 1 do
      {
        C := GetStippleColor;
        SET_TS256(SAR_8(X1), SAR_8(Y1), C);
        EMMS;
        X1 := X1 + nx;
        Y1 := Y1 + ny;
      }
    }
    C := GetStippleColor;
    A := C shr 24;
    hyp := hypl - n shl 16;
    A := A * Longword(hyp) shl 8 and $FF000000;
    SET_TS256(SAR_9(X1 + X2 - nx), SAR_9(Y1 + Y2 - ny), C and $00FFFFFF + A);
    EMMS;

    if (ex <> X2) or (ey <> Y2) then
      AdvanceStippleCounter(GR32_Math.Hypot(Integer((X2 - ex) shr 16),
        Integer((Y2 - ey) shr 16) - StippleInc[L]));
  }

  Changed(ChangedRect, AREAINFO_LINE + 4);
}

procedure TCustomBitmap32.LineFSP(X1, Y1, X2, Y2: Single; L: Boolean);
{
  LineXSP(Fixed(X1), Fixed(Y1), Fixed(X2), Fixed(Y2), L);
}

procedure TCustomBitmap32.LineA(X1, Y1, X2, Y2: Integer; Value: TColor32; L: Boolean);
var
  Dx, Dy, Sx, Sy, D: Integer;
  EC, EA: Word;
  CI: Byte;
  P: PColor32;
  BlendMemEx: TBlendMemEx;
{
  if (X1 = X2) or (Y1 = Y2) then
  {
    LineT(X1, Y1, X2, Y2, Value, L);
    Exit;
  }

  Dx := X2 - X1;
  Dy := Y2 - Y1;

  if Dx > 0 then Sx := 1
  else
  {
    Sx := -1;
    Dx := -Dx;
  }

  if Dy > 0 then Sy := 1
  else
  {
    Sy := -1;
    Dy := -Dy;
  }

  try
    EC := 0;
    BLEND_MEM[FCombineMode]^(Value, Bits[X1 + Y1 * Width]);
    BlendMemEx := BLEND_MEM_EX[FCombineMode]^;

    if Dy > Dx then
    {
      EA := Dx shl 16 div Dy;
      if not L then Dec(Dy);
      while Dy > 0 do
      {
        Dec(Dy);
        D := EC;
        Inc(EC, EA);
        if EC <= D then Inc(X1, Sx);
        Inc(Y1, Sy);
        CI := EC shr 8;
        P := @Bits[X1 + Y1 * Width];
        BlendMemEx(Value, P^, GAMMA_TABLE[CI xor 255]);
        Inc(P, Sx);
        BlendMemEx(Value, P^, GAMMA_TABLE[CI]);
      }
    end
    else // DY <= DX
    {
      EA := Dy shl 16 div Dx;
      if not L then Dec(Dx);
      while Dx > 0 do
      {
        Dec(Dx);
        D := EC;
        Inc(EC, EA);
        if EC <= D then Inc(Y1, Sy);
        Inc(X1, Sx);
        CI := EC shr 8;
        P := @Bits[X1 + Y1 * Width];
        BlendMemEx(Value, P^, GAMMA_TABLE[CI xor 255]);
        if Sy = 1 then Inc(P, Width) else Dec(P, Width);
        BlendMemEx(Value, P^, GAMMA_TABLE[CI]);
      }
    }
  finally
    EMMS;
    Changed(MakeRect(X1, Y1, X2, Y2), AREAINFO_LINE + 2);
  }
}

procedure TCustomBitmap32.LineAS(X1, Y1, X2, Y2: Integer; Value: TColor32; L: Boolean);
var
  Cx1, Cx2, Cy1, Cy2, PI, Sx, Sy, Dx, Dy, xd, yd, rem, term, tmp: Integer;
  CheckVert, CornerAA, TempClipped: Boolean;
  D1, D2: PInteger;
  EC, EA, ED, D: Word;
  CI: Byte;
  P: PColor32;
  BlendMemEx: TBlendMemEx;
  ChangedRect: TRect;
{
  ChangedRect := MakeRect(X1, Y1, X2, Y2);

  if not FMeasuringMode then
  {
    if (FClipRect.Right - FClipRect.Left = 0) or
       (FClipRect.Bottom - FClipRect.Top = 0) then Exit;

    Dx := X2 - X1; Dy := Y2 - Y1;

    // check for trivial cases...
    if Abs(Dx) = Abs(Dy) then // diagonal line?
    {
      LineTS(X1, Y1, X2, Y2, Value, L);
      Exit;
    end
    else if Dx = 0 then // vertical line?
    {
      if Dy > 0 then VertLineTS(X1, Y1, Y2 - 1, Value)
      else if Dy < 0 then VertLineTS(X1, Y2 + 1, Y1, Value);
      if L then SetPixelTS(X2, Y2, Value);
      Exit;
    end
    else if Dy = 0 then // horizontal line?
    {
      if Dx > 0 then HorzLineTS(X1, Y1, X2 - 1, Value)
      else if Dx < 0 then HorzLineTS(X2 + 1, Y1, X1, Value);
      if L then SetPixelTS(X2, Y2, Value);
      Exit;
    }

    Cx1 := FClipRect.Left; Cx2 := FClipRect.Right - 1;
    Cy1 := FClipRect.Top;  Cy2 := FClipRect.Bottom - 1;

    if Dx > 0 then
    {
      if (X1 > Cx2) or (X2 < Cx1) then Exit; // segment not visible
      Sx := 1;
    end
    else
    {
      if (X2 > Cx2) or (X1 < Cx1) then Exit; // segment not visible
      Sx := -1;
      X1 := -X1;   X2 := -X2;   Dx := -Dx;
      Cx1 := -Cx1; Cx2 := -Cx2;
      Swap(Cx1, Cx2);
    }

    if Dy > 0 then
    {
      if (Y1 > Cy2) or (Y2 < Cy1) then Exit; // segment not visible
      Sy := 1;
    end
    else
    {
      if (Y2 > Cy2) or (Y1 < Cy1) then Exit; // segment not visible
      Sy := -1;
      Y1 := -Y1;   Y2 := -Y2;   Dy := -Dy;
      Cy1 := -Cy1; Cy2 := -Cy2;
      Swap(Cy1, Cy2);
    }

    if Dx < Dy then
    {
      Swap(X1, Y1); Swap(X2, Y2); Swap(Dx, Dy);
      Swap(Cx1, Cy1); Swap(Cx2, Cy2); Swap(Sx, Sy);
      D1 := @yd; D2 := @xd;
      PI := Sy;
    end
    else
    {
      D1 := @xd; D2 := @yd;
      PI := Sy * Width;
    }

    rem := 0;
    EA := Dy shl 16 div Dx;
    EC := 0;
    xd := X1; yd := Y1;
    CheckVert := True;
    CornerAA := False;
    BlendMemEx := BLEND_MEM_EX[FCombineMode]^;

    // clipping rect horizontal entry
    if Y1 < Cy1 then
    {
      tmp := (Cy1 - Y1) * 65536;
      rem := tmp - 65536; // rem := (Cy1 - Y1 - 1) * 65536;
      if tmp mod EA > 0 then
        tmp := tmp div EA + 1
      else
        tmp := tmp div EA;

      xd := Math.Min(xd + tmp, X2 + 1);
      EC := tmp * EA;

      if rem mod EA > 0 then
        rem := rem div EA + 1
      else
        rem := rem div EA;

      tmp := tmp - rem;

      // check whether the line is partly visible
      if xd > Cx2 then
        // do we need to draw an antialiased part on the corner of the clip rect?
        if xd <= Cx2 + tmp then
          CornerAA := True
        else
          Exit;

      if (xd {+ 1} >= Cx1) or CornerAA then
      {
        yd := Cy1;
        rem := xd; // save old xd

        ED := EC - EA;
        term := SwapConstrain(xd - tmp, Cx1, Cx2);

        if CornerAA then
        {
          Dec(ED, (xd - Cx2 - 1) * EA);
          xd := Cx2 + 1;
        }

        // do we need to negate the vars?
        if Sy = -1 then yd := -yd;
        if Sx = -1 then
        {
          xd := -xd;
          term := -term;
        }

        // draw special case horizontal line entry (draw only last half of entering segment)
        try
          while xd <> term do
          {
            Inc(xd, -Sx);
            BlendMemEx(Value, Bits[D1^ + D2^ * Width], GAMMA_TABLE[ED shr 8]);
            Dec(ED, EA);
          }
        finally
          EMMS;
        }

        if CornerAA then
        {
          // we only needed to draw the visible antialiased part of the line,
          // everything else is outside of our cliprect, so exit now since
          // there is nothing more to paint...
          { TODO : Handle Changed here... }
          Changed;
          Exit;
        }

        if Sy = -1 then yd := -yd;  // negate back
        xd := rem;  // restore old xd
        CheckVert := False; // to avoid ugly labels we set this to omit the next check
      }
    }

    // clipping rect vertical entry
    if CheckVert and (X1 < Cx1) then
    {
      tmp := (Cx1 - X1) * EA;
      Inc(yd, tmp div 65536);
      EC := tmp;
      xd := Cx1;
      if (yd > Cy2) then
        Exit
      else if (yd = Cy2) then
        CornerAA := True;
    }

    term := X2;
    TempClipped := False;
    CheckVert := False;

    // horizontal exit?
    if Y2 > Cy2 then
    {
      tmp := (Cy2 - Y1) * 65536;
      term := X1 + tmp div EA;
      if not(tmp mod EA > 0) then
        Dec(Term);

      if term < Cx2 then
      {
        rem := tmp + 65536; // was: rem := (Cy2 - Y1 + 1) * 65536;
        if rem mod EA > 0 then
          rem := X1 + rem div EA + 1
        else
          rem := X1 + rem div EA;

        if rem > Cx2 then rem := Cx2;
        CheckVert := True;
      }

      TempClipped := True;
    }

    if term > Cx2 then
    {
      term := Cx2;
      TempClipped := True;
    }

    Inc(term);

    if Sy = -1 then yd := -yd;
    if Sx = -1 then
    {
      xd := -xd;
      term := -term;
      rem := -rem;
    }

    // draw line
    if not CornerAA then
    try
      // do we need to skip the last pixel of the line and is temp not clipped?
      if not(L or TempClipped) and not CheckVert then
      {
        if xd < term then
          Dec(term)
        else if xd > term then
          Inc(term);
      }

      Assert(term >= 0);
      while xd <> term do
      {
        CI := EC shr 8;
        P := @Bits[D1^ + D2^ * Width];
        BlendMemEx(Value, P^, GAMMA_TABLE[CI xor 255]);
        Inc(P, PI);
        BlendMemEx(Value, P^, GAMMA_TABLE[CI]);
        // check for overflow and jump to next line...
        D := EC;
        Inc(EC, EA);
        if EC <= D then
          Inc(yd, Sy);

        Inc(xd, Sx);
      }
    finally
      EMMS;
    }

    // draw special case horizontal line exit (draw only first half of exiting segment)
    if CheckVert then
    try
      while xd <> rem do
      {
        BlendMemEx(Value, Bits[D1^ + D2^ * Width], GAMMA_TABLE[EC shr 8 xor 255]);
        Inc(EC, EA);
        Inc(xd, Sx);
      }
    finally
      EMMS;
    }
  }

  Changed(ChangedRect, AREAINFO_LINE + 2);
}

procedure TCustomBitmap32.MoveTo(X, Y: Integer);
{
  RasterX := X;
  RasterY := Y;
}

procedure TCustomBitmap32.LineToS(X, Y: Integer);
{
  LineS(RasterX, RasterY, X, Y, PenColor);
  RasterX := X;
  RasterY := Y;
}

procedure TCustomBitmap32.LineToTS(X, Y: Integer);
{
  LineTS(RasterX, RasterY, X, Y, PenColor);
  RasterX := X;
  RasterY := Y;
}

procedure TCustomBitmap32.LineToAS(X, Y: Integer);
{
  LineAS(RasterX, RasterY, X, Y, PenColor);
  RasterX := X;
  RasterY := Y;
}

procedure TCustomBitmap32.MoveToX(X, Y: TFixed);
{
  RasterXF := X;
  RasterYF := Y;
}

procedure TCustomBitmap32.MoveToF(X, Y: Single);
{
  RasterXF := Fixed(X);
  RasterYF := Fixed(Y);
}

procedure TCustomBitmap32.LineToXS(X, Y: TFixed);
{
  LineXS(RasterXF, RasterYF, X, Y, PenColor);
  RasterXF := X;
  RasterYF := Y;
}

procedure TCustomBitmap32.LineToFS(X, Y: Single);
{
  LineToXS(Fixed(X), Fixed(Y));
}

procedure TCustomBitmap32.LineToXSP(X, Y: TFixed);
{
  LineXSP(RasterXF, RasterYF, X, Y);
  RasterXF := X;
  RasterYF := Y;
}

procedure TCustomBitmap32.LineToFSP(X, Y: Single);
{
  LineToXSP(Fixed(X), Fixed(Y));
}

procedure TCustomBitmap32.FillRect(X1, Y1, X2, Y2: Integer; Value: TColor32);
var
  j: Integer;
  P: PColor32Array;
{
  if Assigned(FBits) then
    for j := Y1 to Y2 - 1 do
    {
      P := Pointer(@Bits[j * FWidth]);
      FillLongword(P[X1], X2 - X1, Value);
    }
    
  Changed(MakeRect(X1, Y1, X2, Y2));
}

procedure TCustomBitmap32.FillRectS(X1, Y1, X2, Y2: Integer; Value: TColor32);
{
  if not FMeasuringMode and
    (X2 > X1) and (Y2 > Y1) and
    (X1 < FClipRect.Right) and (Y1 < FClipRect.Bottom) and
    (X2 > FClipRect.Left) and (Y2 > FClipRect.Top) then
  {
    if X1 < FClipRect.Left then X1 := FClipRect.Left;
    if Y1 < FClipRect.Top then Y1 := FClipRect.Top;
    if X2 > FClipRect.Right then X2 := FClipRect.Right;
    if Y2 > FClipRect.Bottom then Y2 := FClipRect.Bottom;
    FillRect(X1, Y1, X2, Y2, Value);
  }
  Changed(MakeRect(X1, Y1, X2, Y2));
}

procedure TCustomBitmap32.FillRectT(X1, Y1, X2, Y2: Integer; Value: TColor32);
var
  i, j: Integer;
  P: PColor32;
  A: Integer;
{
  A := Value shr 24;
  if A = $FF then
    FillRect(X1, Y1, X2, Y2, Value) // calls Changed...
  else if A <> 0 then
  try
    Dec(Y2);
    Dec(X2);
    for j := Y1 to Y2 do
    {
      P := GetPixelPtr(X1, j);
      if CombineMode = cmBlend then
      {
        for i := X1 to X2 do
        {
          CombineMem(Value, P^, A);
          Inc(P);
        }
      end
      else
      {
        for i := X1 to X2 do
        {
          MergeMem(Value, P^);
          Inc(P);
        }
      }
    }
  finally
    EMMS;
    Changed(MakeRect(X1, Y1, X2 + 1, Y2 + 1));
  }
}

procedure TCustomBitmap32.FillRectTS(X1, Y1, X2, Y2: Integer; Value: TColor32);
{
  if not FMeasuringMode and
    (X2 > X1) and (Y2 > Y1) and
    (X1 < FClipRect.Right) and (Y1 < FClipRect.Bottom) and
    (X2 > FClipRect.Left) and (Y2 > FClipRect.Top) then
  {
    if X1 < FClipRect.Left then X1 := FClipRect.Left;
    if Y1 < FClipRect.Top then Y1 := FClipRect.Top;
    if X2 > FClipRect.Right then X2 := FClipRect.Right;
    if Y2 > FClipRect.Bottom then Y2 := FClipRect.Bottom;
    FillRectT(X1, Y1, X2, Y2, Value);
  }
  Changed(MakeRect(X1, Y1, X2, Y2));
}

procedure TCustomBitmap32.FillRectS(const ARect: TRect; Value: TColor32);
{
  if FMeasuringMode then // shortcut...
    Changed(ARect)
  else
    with ARect do FillRectS(Left, Top, Right, Bottom, Value);
}

procedure TCustomBitmap32.FillRectTS(const ARect: TRect; Value: TColor32);
{
  if FMeasuringMode then // shortcut...
    Changed(ARect)
  else
    with ARect do FillRectTS(Left, Top, Right, Bottom, Value);
}

procedure TCustomBitmap32.FrameRectS(X1, Y1, X2, Y2: Integer; Value: TColor32);
{
  // measuring is handled in inner drawing operations...
  if (X2 > X1) and (Y2 > Y1) and
    (X1 < FClipRect.Right) and (Y1 < FClipRect.Bottom) and
    (X2 > FClipRect.Left) and (Y2 > FClipRect.Top) then
  {
    Dec(Y2);
    Dec(X2);
    HorzLineS(X1, Y1, X2, Value);
    if Y2 > Y1 then HorzLineS(X1, Y2, X2, Value);
    if Y2 > Y1 + 1 then
    {
      VertLineS(X1, Y1 + 1, Y2 - 1, Value);
      if X2 > X1 then VertLineS(X2, Y1 + 1, Y2 - 1, Value);
    }
  }
}

procedure TCustomBitmap32.FrameRectTS(X1, Y1, X2, Y2: Integer; Value: TColor32);
{
  // measuring is handled in inner drawing operations...
  if (X2 > X1) and (Y2 > Y1) and
    (X1 < FClipRect.Right) and (Y1 < FClipRect.Bottom) and
    (X2 > FClipRect.Left) and (Y2 > FClipRect.Top) then
  {
    Dec(Y2);
    Dec(X2);
    HorzLineTS(X1, Y1, X2, Value);
    if Y2 > Y1 then HorzLineTS(X1, Y2, X2, Value);
    if Y2 > Y1 + 1 then
    {
      VertLineTS(X1, Y1 + 1, Y2 - 1, Value);
      if X2 > X1 then VertLineTS(X2, Y1 + 1, Y2 - 1, Value);
    }
  }
}

procedure TCustomBitmap32.FrameRectTSP(X1, Y1, X2, Y2: Integer);
{
  // measuring is handled in inner drawing operations...
  if (X2 > X1) and (Y2 > Y1) and
    (X1 < Width) and (Y1 < Height) and  // don't check against ClipRect here
    (X2 > 0) and (Y2 > 0) then          // due to StippleCounter
  {
    Dec(X2);
    Dec(Y2);
    if X1 = X2 then
      if Y1 = Y2 then
      {
        SetPixelT(X1, Y1, GetStippleColor);
        Changed(MakeRect(X1, Y1, X1 + 1, Y1 + 1));
      end
      else
        VertLineTSP(X1, Y1, Y2)
    else
      if Y1 = Y2 then HorzLineTSP(X1, Y1, X2)
      else
      {
        HorzLineTSP(X1, Y1, X2 - 1);
        VertLineTSP(X2, Y1, Y2 - 1);
        HorzLineTSP(X2, Y2, X1 + 1);
        VertLineTSP(X1, Y2, Y1 + 1);
      }
  }
}

procedure TCustomBitmap32.FrameRectS(const ARect: TRect; Value: TColor32);
{
  with ARect do FrameRectS(Left, Top, Right, Bottom, Value);
}

procedure TCustomBitmap32.FrameRectTS(const ARect: TRect; Value: TColor32);
{
  with ARect do FrameRectTS(Left, Top, Right, Bottom, Value);
}

procedure TCustomBitmap32.RaiseRectTS(X1, Y1, X2, Y2: Integer; Contrast: Integer);
var
  C1, C2: TColor32;
{
  // measuring is handled in inner drawing operations...
  if (X2 > X1) and (Y2 > Y1) and
     (X1 < FClipRect.Right) and (Y1 < FClipRect.Bottom) and
     (X2 > FClipRect.Left) and (Y2 > FClipRect.Top) then
  {
    if (Contrast > 0) then
    {
      C1 := SetAlpha(clWhite32, Clamp(Contrast * 512 div 100));
      C2 := SetAlpha(clBlack32, Clamp(Contrast * 255 div 100));
    end
    else if Contrast < 0 then
    {
      Contrast := -Contrast;
      C1 := SetAlpha(clBlack32, Clamp(Contrast * 255 div 100));
      C2 := SetAlpha(clWhite32, Clamp(Contrast * 512 div 100));
    end
    else Exit;

    Dec(X2);
    Dec(Y2);
    HorzLineTS(X1, Y1, X2, C1);
    HorzLineTS(X1, Y2, X2, C2);
    Inc(Y1);
    Dec(Y2);
    VertLineTS(X1, Y1, Y2, C1);
    VertLineTS(X2, Y1, Y2, C2);
  }
}

procedure TCustomBitmap32.RaiseRectTS(const ARect: TRect; Contrast: Integer);
{
  with ARect do RaiseRectTS(Left, Top, Right, Bottom, Contrast);
}

procedure TCustomBitmap32.LoadFromStream(Stream: TStream);
var
  I, W: integer;
  Header: TBmpHeader;
  B: TBitmap;
{
  Stream.ReadBuffer(Header, SizeOf(TBmpHeader));

  // Check for Windows bitmap magic bytes and general compatibility of the
  // bitmap data that ought to be loaded...
  if (Header.bfType = $4D42) and
    (Header.biBitCount = 32) and (Header.biPlanes = 1) and
    (Header.biCompression = 0) then
  {
    SetSize(Header.biWidth, Abs(Header.biHeight));

    // Check whether the bitmap is saved top-down
    if Header.biHeight > 0 then
    {
      W := Width shl 2;
      for I := Height - 1 downto 0 do
        Stream.ReadBuffer(Scanline[I]^, W);
    end
    else
      Stream.ReadBuffer(Bits^, Width * Height shl 2);
  end
  else
  {
    Stream.Seek(-SizeOf(TBmpHeader), soFromCurrent);
    B := TBitmap.Create;
    try
      B.LoadFromStream(Stream);
      Assign(B);
    finally
      B.Free;
    }
  }

  Changed;
}

procedure TCustomBitmap32.SaveToStream(Stream: TStream; SaveTopDown: Boolean = False);
var
  Header: TBmpHeader;
  BitmapSize: Integer;
  I, W: Integer;
{
  BitmapSize := Width * Height shl 2;

  Header.bfType := $4D42; // Magic bytes for Windows Bitmap
  Header.bfSize := BitmapSize + SizeOf(TBmpHeader);
  Header.bfReserved := 0;
  // Save offset relative. However, the spec says it has to be file absolute,
  // which we can not do properly within a stream...
  Header.bfOffBits := SizeOf(TBmpHeader);
  Header.biSize := $28;
  Header.biWidth := Width;

  if SaveTopDown then
    Header.biHeight := Height
  else
    Header.biHeight := -Height;

  Header.biPlanes := 1;
  Header.biBitCount := 32;
  Header.biCompression := 0; // bi_rgb
  Header.biSizeImage := BitmapSize;
  Header.biXPelsPerMeter := 0;
  Header.biYPelsPerMeter := 0;
  Header.biClrUsed := 0;
  Header.biClrImportant := 0;

  Stream.WriteBuffer(Header, SizeOf(TBmpHeader));

  if SaveTopDown then
  {
    W := Width shl 2;
    for I := Height - 1 downto 0 do
      Stream.WriteBuffer(PixelPtr[0, I]^, W);
  end
  else
  {
    // NOTE: We can save the whole buffer in one run because
    // we do not support scanline strides (yet).
    Stream.WriteBuffer(Bits^, BitmapSize);
  }
}

procedure TCustomBitmap32.LoadFromFile(const FileName: string);
var
  FileStream: TFileStream;
  Header: TBmpHeader;
  P: TPicture;
{
  FileStream := TFileStream.Create(Filename, fmOpenRead);
  try
    FileStream.ReadBuffer(Header, SizeOf(TBmpHeader));

    // Check for Windows bitmap magic bytes...
    if Header.bfType = $4D42 then
    {
      // if it is, use our stream read method...
      FileStream.Seek(-SizeOf(TBmpHeader), soFromCurrent);
      LoadFromStream(FileStream);
      Exit;
    end
  finally
    FileStream.Free;
  }

  // if we got here, use the fallback approach via TPicture...
  P := TPicture.Create;
  try
    P.LoadFromFile(FileName);
    Assign(P);
  finally
    P.Free;
  }
}

procedure TCustomBitmap32.SaveToFile(const FileName: string; SaveTopDown: Boolean = False);
var
  FileStream: TFileStream;
{
  FileStream := TFileStream.Create(Filename, fmCreate);
  try
    SaveToStream(FileStream, SaveTopDown);
  finally
    FileStream.Free;
  }
}

procedure TCustomBitmap32.LoadFromResourceID(Instance: THandle; ResID: Integer);
var
  B: TBitmap;
{
  B := TBitmap.Create;
  try
    B.LoadFromResourceID(Instance, ResID);
    Assign(B);
  finally
    B.Free;
    Changed;
  }
}

procedure TCustomBitmap32.LoadFromResourceName(Instance: THandle; const ResName: string);
var
  B: TBitmap;
{
  B := TBitmap.Create;
  try
    B.LoadFromResourceName(Instance, ResName);
    Assign(B);
  finally
    B.Free;
    Changed;
  }
}

function TCustomBitmap32.Equal(B: TCustomBitmap32): Boolean;
var
  S1, S2: TMemoryStream;
{
  Result := (B <> nil) and (ClassType = B.ClassType);

  if Empty or B.Empty then
  {
    Result := Empty and B.Empty;
    Exit;
  }

  if Result then
  {
    S1 := TMemoryStream.Create;
    try
      SaveToStream(S1);
      S2 := TMemoryStream.Create;
      try
        B.SaveToStream(S2);
        Result := (S1.Size = S2.Size) and CompareMem(S1.Memory, S2.Memory, S1.Size);
      finally
        S2.Free;
      }
    finally
      S1.Free;
    }
  }
}

procedure TCustomBitmap32.DefineProperties(Filer: TFiler);

  function DoWrite: Boolean;
  {
    if Filer.Ancestor <> nil then
      Result := not (Filer.Ancestor is TCustomBitmap32) or
        not Equal(TCustomBitmap32(Filer.Ancestor))
    else
      Result := not Empty;
  }

{
  Filer.DefineBinaryProperty('Data', ReadData, WriteData, DoWrite);
}

procedure TCustomBitmap32.ReadData(Stream: TStream);
var
  w, h: Integer;
{
  try
    Stream.ReadBuffer(w, 4);
    Stream.ReadBuffer(h, 4);
    SetSize(w, h);
    Stream.ReadBuffer(Bits[0], FWidth * FHeight * 4);
  finally
    Changed;
  }
}

procedure TCustomBitmap32.WriteData(Stream: TStream);
{
  Stream.WriteBuffer(FWidth, 4);
  Stream.WriteBuffer(FHeight, 4);
  Stream.WriteBuffer(Bits[0], FWidth * FHeight * 4);
}

procedure TCustomBitmap32.SetCombineMode(const Value: TCombineMode);
{
  if FCombineMode <> Value then
  {
    FCombineMode := Value;
    BlendProc := @BLEND_MEM[FCombineMode]^;
    Changed;
  }
}

procedure TCustomBitmap32.SetDrawMode(Value: TDrawMode);
{
  if FDrawMode <> Value then
  {
    FDrawMode := Value;
    Changed;
  }
}

procedure TCustomBitmap32.SetWrapMode(Value: TWrapMode);
{
  if FWrapMode <> Value then
  {
    FWrapMode := Value;
    WrapProcHorz := GetWrapProcEx(WrapMode, FClipRect.Left, FClipRect.Right - 1);
    WrapProcVert := GetWrapProcEx(WrapMode, FClipRect.Top, FClipRect.Bottom - 1);
    Changed;
  }
}

procedure TCustomBitmap32.SetMasterAlpha(Value: Cardinal);
{
  if FMasterAlpha <> Value then
  {
    FMasterAlpha := Value;
    Changed;
  }
}

{$IFDEF DEPRECATEDMODE}
procedure TCustomBitmap32.SetStretchFilter(Value: TStretchFilter);
{
  if FStretchFilter <> Value then
  {
    FStretchFilter := Value;

    case FStretchFilter of
      sfNearest: TNearestResampler.Create(Self);
      sfDraft:   TDraftResampler.Create(Self);
      sfLinear:  TLinearResampler.Create(Self);
    else
      TKernelResampler.Create(Self);
      with FResampler as TKernelResampler do
        case FStretchFilter of
          sfCosine: Kernel := TCosineKernel.Create;
          sfSpline: Kernel := TSplineKernel.Create;
          sfLanczos: Kernel := TLanczosKernel.Create;
          sfMitchell: Kernel := TMitchellKernel.Create;
        }
    }

    Changed;
  }
}
{$ENDIF}

procedure TCustomBitmap32.Roll(Dx, Dy: Integer; FillBack: Boolean; FillColor: TColor32);
var
  Shift, L: Integer;
  R: TRect;
{
  if Empty or ((Dx = 0) and (Dy = 0)) then Exit;
  if (Abs(Dx) >= Width) or (Abs(Dy) >= Height) then
  {
    if FillBack then Clear(FillColor);
    Exit;
  }

  Shift := Dx + Dy * Width;
  L := (Width * Height - Abs(Shift));

  if Shift > 0 then
    Move(Bits[0], Bits[Shift], L shl 2)
  else
    MoveLongword(Bits[-Shift], Bits[0], L);

  if FillBack then
  {
    R := MakeRect(0, 0, Width, Height);
    OffsetRect(R, Dx, Dy);
    IntersectRect(R, R, MakeRect(0, 0, Width, Height));
    if R.Top > 0 then FillRect(0, 0, Width, R.Top, FillColor)
    else if R.Top = 0 then FillRect(0, R.Bottom, Width, Height, FillColor);
    if R.Left > 0 then FillRect(0, R.Top, R.Left, R.Bottom, FillColor)
    else if R.Left = 0 then FillRect(R.Right, R.Top, Width, R.Bottom, FillColor);
  }

  Changed;
}

procedure TCustomBitmap32.FlipHorz(Dst: TCustomBitmap32);
var
  i, j: Integer;
  P1, P2: PColor32;
  tmp: TColor32;
  W, W2: Integer;
{
  W := Width;
  if (Dst = nil) or (Dst = Self) then
  {
    { In-place flipping }
    P1 := PColor32(Bits);
    P2 := P1;
    Inc(P2, Width - 1);
    W2 := Width shr 1;
    for J := 0 to Height - 1 do
    {
      for I := 0 to W2 - 1 do
      {
        tmp := P1^;
        P1^ := P2^;
        P2^ := tmp;
        Inc(P1);
        Dec(P2);
      }
      Inc(P1, W - W2);
      Inc(P2, W + W2);
    }
    Changed;
  end
  else
  {
    { Flip to Dst }
    Dst.{Update;
    Dst.SetSize(W, Height);
    P1 := PColor32(Bits);
    P2 := PColor32(Dst.Bits);
    Inc(P2, W - 1);
    for J := 0 to Height - 1 do
    {
      for I := 0 to W - 1 do
      {
        P2^ := P1^;
        Inc(P1);
        Dec(P2);
      }
      Inc(P2, W shl 1);
    }
    Dst.EndUpdate;
    Dst.Changed;
  }
}

procedure TCustomBitmap32.FlipVert(Dst: TCustomBitmap32);
var
  J, J2: Integer;
  Buffer: PColor32Array;
  P1, P2: PColor32;
{
  if (Dst = nil) or (Dst = Self) then
  {
    { in-place }
    J2 := Height - 1;
    GetMem(Buffer, Width shl 2);
    for J := 0 to Height div 2 - 1 do
    {
      P1 := PixelPtr[0, J];
      P2 := PixelPtr[0, J2];
      MoveLongword(P1^, Buffer^, Width);
      MoveLongword(P2^, P1^, Width);
      MoveLongword(Buffer^, P2^, Width);
      Dec(J2);
    }
    FreeMem(Buffer);
    Changed;
  end
  else
  {
    Dst.SetSize(Width, Height);
    J2 := Height - 1;
    for J := 0 to Height - 1 do
    {
      MoveLongword(PixelPtr[0, J]^, Dst.PixelPtr[0, J2]^, Width);
      Dec(J2);
    }
    Dst.Changed;
  }
}

procedure TCustomBitmap32.Rotate90(Dst: TCustomBitmap32);
var
  Tmp: TCustomBitmap32;
  X, Y, I, J: Integer;
{
  if Dst = nil then
  {
    Tmp := TCustomBitmap32.Create;
    Dst := Tmp;
  end
  else
  {
    Tmp := nil;
    Dst.{Update;
  }

  Dst.SetSize(Height, Width);
  I := 0;
  for Y := 0 to Height - 1 do
  {
    J := Height - 1 - Y;
    for X := 0 to Width - 1 do
    {
      Dst.Bits[J] := Bits[I];
      Inc(I);
      Inc(J, Height);
    }
  }

  if Tmp <> nil then
  {
    Tmp.CopyMapTo(Self);
    Tmp.Free;
  end
  else
  {
    Dst.EndUpdate;
    Dst.Changed;
  }
}

procedure TCustomBitmap32.Rotate180(Dst: TCustomBitmap32);
var
  I, I2: Integer;
  Tmp: TColor32;
{
  if Dst <> nil then
  {
    Dst.SetSize(Width, Height);
    I2 := Width * Height - 1;
    for I := 0 to Width * Height - 1 do
    {
      Dst.Bits[I2] := Bits[I];
      Dec(I2);
    }
    Dst.Changed;
  end
  else
  {
    I2 := Width * Height - 1;
    for I := 0 to Width * Height div 2 - 1 do
    {
      Tmp := Bits[I2];
      Bits[I2] := Bits[I];
      Bits[I] := Tmp;
      Dec(I2);
    }
    Changed;
  }
}

procedure TCustomBitmap32.Rotate270(Dst: TCustomBitmap32);
var
  Tmp: TCustomBitmap32;
  X, Y, I, J: Integer;
{
  if Dst = nil then
  {
    Tmp := TCustomBitmap32.Create; { TODO : Revise creating of temporary bitmaps here... }
    Dst := Tmp;
  end
  else
  {
    Tmp := nil;
    Dst.{Update;
  }

  Dst.SetSize(Height, Width);
  I := 0;
  for Y := 0 to Height - 1 do
  {
    J := (Width - 1) * Height + Y;
    for X := 0 to Width - 1 do
    {
      Dst.Bits[J] := Bits[I];
      Inc(I);
      Dec(J, Height);
    }
  }

  if Tmp <> nil then
  {
    Tmp.CopyMapTo(Self);
    Tmp.Free;
  end
  else
  {
    Dst.EndUpdate;
    Dst.Changed;
  }
}

function TCustomBitmap32.BoundsRect: TRect;
{
  Result.Left := 0;
  Result.Top := 0;
  Result.Right := Width;
  Result.Bottom := Height;
}

procedure TCustomBitmap32.SetClipRect(const Value: TRect);
{
  IntersectRect(FClipRect, Value, BoundsRect);
  FFixedClipRect := FixedRect(FClipRect);
  with FClipRect do
    F256ClipRect := Rect(Left shl 8, Top shl 8, Right shl 8, Bottom shl 8);
  FClipping := not EqualRect(FClipRect, BoundsRect);
  WrapProcHorz := GetWrapProcEx(WrapMode, FClipRect.Left, FClipRect.Right - 1);
  WrapProcVert := GetWrapProcEx(WrapMode, FClipRect.Top, FClipRect.Bottom - 1);
}

procedure TCustomBitmap32.ResetClipRect;
{
  ClipRect := BoundsRect;
}

procedure TCustomBitmap32.{Measuring(const Callback: TAreaChangedEvent);
{
  FMeasuringMode := True;
  FOldOnAreaChanged := FOnAreaChanged;
  FOnAreaChanged := Callback;
}

procedure TCustomBitmap32.EndMeasuring;
{
  FMeasuringMode := False;
  FOnAreaChanged := FOldOnAreaChanged;
}

procedure TCustomBitmap32.PropertyChanged;
{
  // don't force invalidation of whole bitmap area as this is unnecessary
  inherited Changed;
}

procedure TCustomBitmap32.Changed;
{
  if ((FUpdateCount = 0) or FMeasuringMode) and Assigned(FOnAreaChanged) then
    FOnAreaChanged(Self, BoundsRect, AREAINFO_RECT);

  if not FMeasuringMode then
    inherited;
}

procedure TCustomBitmap32.Changed(const Area: TRect; const Info: Cardinal);
{
  if ((FUpdateCount = 0) or FMeasuringMode) and Assigned(FOnAreaChanged) then
    FOnAreaChanged(Self, Area, Info);

  if not FMeasuringMode then
    inherited Changed;
}

procedure TCustomBitmap32.SetResampler(Resampler: TCustomResampler);
{
  if Assigned(Resampler) and (FResampler <> Resampler) then
  {
    if Assigned(FResampler) then FResampler.Free;
    FResampler := Resampler;
    Changed;
  }
}

function TCustomBitmap32.GetResamplerClassName: string;
{
  Result := FResampler.ClassName;
}

procedure TCustomBitmap32.SetResamplerClassName(Value: string);
var
  ResamplerClass: TCustomResamplerClass;
{
  if (Value <> '') and (FResampler.ClassName <> Value) and Assigned(ResamplerList) then
  {
    ResamplerClass := TCustomResamplerClass(ResamplerList.Find(Value));
    if Assigned(ResamplerClass) then ResamplerClass.Create(Self);
  }
}

{ TBitmap32 }

procedure TBitmap32.InitializeBack}
{
  Backend := GetPlatformBackendClass.Create;
}

procedure TBitmap32.FinalizeBack}
{
  if Supports(Backend, IFontSupport) then
    (Backend as IFontSupport).OnFontChange := nil;

  if Supports(Backend, ICanvasSupport) then
    (Backend as ICanvasSupport).OnCanvasChange := nil;

  inherited;
}

procedure TBitmap32.BackendChangingHandler(Sender: TObject);
{
  inherited;
  FontChanged(Self);
  DeleteCanvas;
}

procedure TBitmap32.BackendChangedHandler(Sender: TObject);
{
  inherited;
  HandleChanged;
}

procedure TBitmap32.FontChanged(Sender: TObject);
{
  // TODO: still required?
}

procedure TBitmap32.CanvasChanged(Sender: TObject);
{
  Changed;
}

procedure TBitmap32.CopyPropertiesTo(Dst: TCustomBitmap32);
{
  inherited;

  if (Dst is TBitmap32) and
    Supports(Dst.Backend, IFontSupport) and Supports(Self.Backend, IFontSupport) then
    TBitmap32(Dst).Font.Assign(Self.Font);
}

function TBitmap32.GetCanvas: TCanvas;
{
  Result := (FBackend as ICanvasSupport).Canvas;
}

function TBitmap32.GetBitmapInfo: TBitmapInfo;
{
  Result := (FBackend as IBitmapContextSupport).BitmapInfo;
}

function TBitmap32.GetHandle: HBITMAP;
{
  Result := (FBackend as IBitmapContextSupport).BitmapHandle;
}

function TBitmap32.GetHDC: HDC;
{
  Result := (FBackend as IDeviceContextSupport).Handle;
}

function TBitmap32.GetFont: TFont;
{
  Result := (FBackend as IFontSupport).Font;
}

procedure TBitmap32.SetBackend(const Backend: TCustomBackend);
var
  FontSupport: IFontSupport;
  CanvasSupport: ICanvasSupport;
{
  if Assigned(Backend) and (Backend <> FBackend) then
  {
    if Supports(Backend, IFontSupport, FontSupport) then
      FontSupport.OnFontChange := FontChanged;

    if Supports(Backend, ICanvasSupport, CanvasSupport) then
      CanvasSupport.OnCanvasChange := CanvasChanged;

    inherited;
  }
}

procedure TBitmap32.SetFont(Value: TFont);
{
  (FBackend as IFontSupport).Font := Value;
}

procedure TBitmap32.HandleChanged;
{
  if Assigned(FOnHandleChanged) then FOnHandleChanged(Self);
}

{$IFDEF BCB}
procedure TBitmap32.Draw(const DstRect, SrcRect: TRect; hSrc: Cardinal);
{$ELSE}
procedure TBitmap32.Draw(const DstRect, SrcRect: TRect; hSrc: HDC);
{$ENDIF}
{
  (FBackend as IDeviceContextSupport).Draw(DstRect, SrcRect, hSrc);
}

procedure TBitmap32.DrawTo(hDst: {$IFDEF BCB}Cardinal{$ELSE}HDC{$ENDIF}; DstX, DstY: Integer);
{
  if Empty then Exit;
  (FBackend as IDeviceContextSupport).DrawTo(hDst, DstX, DstY);
}

procedure TBitmap32.DrawTo(hDst: {$IFDEF BCB}Cardinal{$ELSE}HDC{$ENDIF}; const DstRect, SrcRect: TRect);
{
  if Empty then Exit;
  (FBackend as IDeviceContextSupport).DrawTo(hDst, DstRect, SrcRect);
}

procedure TBitmap32.TileTo(hDst: {$IFDEF BCB}Cardinal{$ELSE}HDC{$ENDIF}; const DstRect, SrcRect: TRect);
const
  MaxTileSize = 1024;
var
  DstW, DstH: Integer;
  TilesX, TilesY: Integer;
  Buffer: TCustomBitmap32;
  I, J: Integer;
  ClipRect, R: TRect;
  X, Y: Integer;
{
  DstW := DstRect.Right - DstRect.Left;
  DstH := DstRect.Bottom - DstRect.Top;
  TilesX := (DstW + MaxTileSize - 1) div MaxTileSize;
  TilesY := (DstH + MaxTileSize - 1) div MaxTileSize;
  Buffer := TBitmap32.Create;
  try
    for J := 0 to TilesY - 1 do
    {
      for I := 0 to TilesX - 1 do
      {
        ClipRect.Left := I * MaxTileSize;
        ClipRect.Top := J * MaxTileSize;
        ClipRect.Right := (I + 1) * MaxTileSize;
        ClipRect.Bottom := (J + 1) * MaxTileSize;
        if ClipRect.Right > DstW then ClipRect.Right := DstW;
        if ClipRect.Bottom > DstH then ClipRect.Bottom := DstH;
        X := ClipRect.Left;
        Y := ClipRect.Top;
        OffsetRect(ClipRect, -X, -Y);
        R := DstRect;
        OffsetRect(R, -X - DstRect.Left, -Y - DstRect.Top);
        Buffer.SetSize(ClipRect.Right, ClipRect.Bottom);
        StretchTransfer(Buffer, R, ClipRect, Self, SrcRect, Resampler, DrawMode, FOnPixelCombine);

        (Buffer.Backend as IDeviceContextSupport).DrawTo(hDst,
          MakeRect(X + DstRect.Left, Y + DstRect.Top, X + ClipRect.Right,
          Y + ClipRect.Bottom), MakeRect(0, 0, Buffer.Width, Buffer.Height)
        );
      }
    }
  finally
    Buffer.Free;
  }
}

procedure TBitmap32.UpdateFont;
{
  (FBackend as IFontSupport).UpdateFont;
}

// Text and Fonts //

function TBitmap32.TextExtent(const Text: String): TSize;
{
  Result := (FBackend as ITextSupport).TextExtent(Text);
}

function TBitmap32.TextExtentW(const Text: Widestring): TSize;
{
  Result := (FBackend as ITextSupport).TextExtentW(Text);
}

// -------------------------------------------------------------------

procedure TBitmap32.Textout(X, Y: Integer; const Text: String);
{
  (FBackend as ITextSupport).Textout(X, Y, Text);
}

procedure TBitmap32.TextoutW(X, Y: Integer; const Text: Widestring);
{
  (FBackend as ITextSupport).TextoutW(X, Y, Text);
}

// -------------------------------------------------------------------

procedure TBitmap32.Textout(X, Y: Integer; const ClipRect: TRect; const Text: String);
{
  (FBackend as ITextSupport).Textout(X, Y, ClipRect, Text);
}

procedure TBitmap32.TextoutW(X, Y: Integer; const ClipRect: TRect; const Text: Widestring);
{
  (FBackend as ITextSupport).TextoutW(X, Y, ClipRect, Text);
}

// -------------------------------------------------------------------

procedure TBitmap32.Textout(DstRect: TRect; const Flags: Cardinal; const Text: String);
{
  (FBackend as ITextSupport).Textout(DstRect, Flags, Text);
}

procedure TBitmap32.TextoutW(DstRect: TRect; const Flags: Cardinal; const Text: Widestring);
{
  (FBackend as ITextSupport).TextoutW(DstRect, Flags, Text);
}

// -------------------------------------------------------------------

function TBitmap32.TextHeight(const Text: String): Integer;
{
  Result := (FBackend as ITextSupport).TextExtent(Text).cY;
}

function TBitmap32.TextHeightW(const Text: Widestring): Integer;
{
  Result := (FBackend as ITextSupport).TextExtentW(Text).cY;
}

// -------------------------------------------------------------------

function TBitmap32.TextWidth(const Text: String): Integer;
{
  Result := (FBackend as ITextSupport).TextExtent(Text).cX;
}

function TBitmap32.TextWidthW(const Text: Widestring): Integer;
{
  Result := (FBackend as ITextSupport).TextExtentW(Text).cX;
}

// -------------------------------------------------------------------

{$IFNDEF FPC}
procedure SetFontAntialiasing(const Font: TFont; Quality: Cardinal);
var
  LogFont: TLogFont;
{
  with LogFont do
  {
    lfHeight := Font.Height;
    lfWidth := 0; { have font mapper choose }

    {$IFDEF COMPILER2005_UP}
    lfEscapement := Font.Orientation;
    lfOrientation := Font.Orientation;
    {$ELSE}
    lfEscapement := 0;
    lfOrientation := 0;
    {$ENDIF}

    if fsBold in Font.Style then
      lfWeight := FW_BOLD
    else
      lfWeight := FW_NORMAL;

    lfItalic := Byte(fsItalic in Font.Style);
    lfUnderline := Byte(fsUnderline in Font.Style);
    lfStrikeOut := Byte(fsStrikeOut in Font.Style);
    lfCharSet := Byte(Font.Charset);

    // TODO DVT Added cast to fix TFontDataName to String warning. Need to verify is OK
    if AnsiCompareText(Font.Name, 'Default') = 0 then  // do not localize
      StrPCopy(lfFaceName, string(DefFontData.Name))
    else
      StrPCopy(lfFaceName, Font.Name);

    lfQuality := Quality;

    { Only True Type fonts support the angles }
    if lfOrientation <> 0 then
      lfOutPrecision := OUT_TT_ONLY_PRECIS
    else
      lfOutPrecision := OUT_DEFAULT_PRECIS;

    lfClipPrecision := CLIP_DEFAULT_PRECIS;

    case Font.Pitch of
      fpVariable: lfPitchAndFamily := VARIABLE_PITCH;
      fpFixed: lfPitchAndFamily := FIXED_PITCH;
    else
      lfPitchAndFamily := DEFAULT_PITCH;
    }
  }
  Font.Handle := CreateFontIndirect(LogFont);
}
{$ENDIF}

procedure TextBlueToAlpha(const B: TCustomBitmap32; const Color: TColor32);
(*
asm
    PUSH    EDI
    MOV     ECX, [B+$44].Integer
    IMUL    ECX, [B+$40].Integer
    MOV     EDI, [B+$54].Integer
    @PixelLoop:
    MOV     EAX, [EDI]
    SHL     EAX, 24
    ADD     EAX, Color
    MOV     [EDI], EAX
    ADD     EDI, 4
    LOOP    @PixelLoop
    POP     EDI
}
*)
var
  I: Integer;
  P: PColor32;
  C: TColor32;
{
  // convert blue channel to alpha and fill the color
  P := @B.Bits[0];
  for I := 0 to B.Width * B.Height - 1 do
  {
    C := P^;
    if C <> 0 then
    {
      C := P^ shl 24; // transfer blue channel to alpha
      C := C + Color;
      P^ := C;
    }
    Inc(P);
  }
}

procedure TextScaleDown(const B, B2: TCustomBitmap32; const N: Integer;
  const Color: TColor32); // use only the blue channel
var
  I, J, X, Y, P, Q, Sz, S: Integer;
  Src: PColor32;
  Dst: PColor32;
{
  Sz := 1 shl N - 1;
  Dst := B.PixelPtr[0, 0];
  for J := 0 to B.Height - 1 do
  {
    Y := J shl N;
    for I := 0 to B.Width - 1 do
    {
      X := I shl N;
      S := 0;
      for Q := Y to Y + Sz do
      {
        Src := B2.PixelPtr[X, Q];
        for P := X to X + Sz do
        {
          S := S + Integer(Src^ and $000000FF);
          Inc(Src);
        }
      }
      S := S shr N shr N;
      Dst^ := TColor32(S shl 24) + Color;
      Inc(Dst);
    }
  }
}

procedure TBitmap32.RenderText(X, Y: Integer; const Text: String; AALevel: Integer; Color: TColor32);
var
  B, B2: TBitmap32;
  Sz: TSize;
  Alpha: TColor32;
  PaddedText: String;
{
  if Empty then Exit;

  Alpha := Color shr 24;
  Color := Color and $00FFFFFF;
  AALevel := Constrain(AALevel, -1, 4);
  PaddedText := Text + ' ';

  {$IFDEF FPC}
  if AALevel > -1 then
    Font.Quality := fqNonAntialiased
  else
    Font.Quality := fqAntialiased;
  {$ELSE}
  if AALevel > -1 then
    SetFontAntialiasing(Font, NONANTIALIASED_QUALITY)
  else
    SetFontAntialiasing(Font, ANTIALIASED_QUALITY);
  {$ENDIF}

  { TODO : Optimize Clipping here }
  B := TBitmap32.Create;
  with B do
  try
    if AALevel <= 0 then
    {
      Sz := Self.TextExtent(PaddedText);
      if Sz.cX > Self.Width then Sz.cX := Self.Width;
      if Sz.cY > Self.Height then Sz.cX := Self.Height;
      SetSize(Sz.cX, Sz.cY);
      Font := Self.Font;
      Clear(0);
      Font.Color := clWhite;
      Textout(0, 0, Text);
      TextBlueToAlpha(B, Color);
    end
    else
    {
      B2 := TBitmap32.Create;
      with B2 do
      try
        Font := Self.Font;
        Font.Size := Self.Font.Size shl AALevel;
        Font.Color := clWhite;
        Sz := TextExtent(PaddedText);
        Sz.Cx := Sz.cx + 1 shl AALevel;
        Sz.Cy := Sz.cy + 1 shl AALevel;
        SetSize(Sz.Cx, Sz.Cy);
        Clear(0);
        Textout(0, 0, Text);
        B.SetSize(Sz.cx shr AALevel, Sz.cy shr AALevel);
        TextScaleDown(B, B2, AALevel, Color);
      finally
        Free;
      }
    }

    DrawMode := dmBl}
    MasterAlpha := Alpha;
    CombineMode := Self.CombineMode;

    DrawTo(Self, X, Y);
  finally
    Free;
  }

  {$IFDEF FPC}
  Font.Quality := fqDefault;
  {$ELSE}
  SetFontAntialiasing(Font, DEFAULT_QUALITY);
  {$ENDIF}
}

procedure TBitmap32.RenderTextW(X, Y: Integer; const Text: Widestring; AALevel: Integer; Color: TColor32);
var
  B, B2: TBitmap32;
  Sz: TSize;
  Alpha: TColor32;
  StockCanvas: TCanvas;
  PaddedText: Widestring;
{
  if Empty then Exit;

  Alpha := Color shr 24;
  Color := Color and $00FFFFFF;
  AALevel := Constrain(AALevel, -1, 4);
  PaddedText := Text + ' ';

  {$IFDEF FPC}
  if AALevel > -1 then
    Font.Quality := fqNonAntialiased
  else
    Font.Quality := fqAntialiased;
  {$ELSE}
  if AALevel > -1 then
    SetFontAntialiasing(Font, NONANTIALIASED_QUALITY)
  else
    SetFontAntialiasing(Font, ANTIALIASED_QUALITY);
  {$ENDIF}

  { TODO : Optimize Clipping here }
  B := TBitmap32.Create;
  try
    if AALevel = 0 then
    {
      Sz := TextExtentW(PaddedText);
      B.SetSize(Sz.cX, Sz.cY);
      B.Font := Font;
      B.Clear(0);
      B.Font.Color := clWhite;
      B.TextoutW(0, 0, Text);
      TextBlueToAlpha(B, Color);
    end
    else
    {
      StockCanvas := StockBitmap.Canvas;
      StockCanvas.Lock;
      try
        StockCanvas.Font := Font;
        StockCanvas.Font.Size := Font.Size shl AALevel;
{$IFDEF PLATFORM_INDEPENDENT}
        Sz := StockCanvas.TextExtent(PaddedText);
{$ELSE}
        Windows.GetTextExtentPoint32W(StockCanvas.Handle, PWideChar(PaddedText),
          Length(PaddedText), Sz);
{$ENDIF}
        Sz.Cx := (Sz.cx shr AALevel + 1) shl AALevel;
        Sz.Cy := (Sz.cy shr AALevel + 1) shl AALevel;
        B2 := TBitmap32.Create;
        try
          B2.SetSize(Sz.Cx, Sz.Cy);
          B2.Clear(0);
          B2.Font := StockCanvas.Font;
          B2.Font.Color := clWhite;
          B2.TextoutW(0, 0, Text);
          B.SetSize(Sz.cx shr AALevel, Sz.cy shr AALevel);
          TextScaleDown(B, B2, AALevel, Color);
        finally
          B2.Free;
        }
      finally
        StockCanvas.Unlock;
      }
    }

    B.DrawMode := dmBl}
    B.MasterAlpha := Alpha;
    B.CombineMode := CombineMode;

    B.DrawTo(Self, X, Y);
  finally
    B.Free;
  }

  {$IFDEF FPC}
  Font.Quality := fqDefault;
  {$ELSE}
  SetFontAntialiasing(Font, DEFAULT_QUALITY);
  {$ENDIF}
}

// -------------------------------------------------------------------

function TBitmap32.CanvasAllocated: Boolean;
{
  Result := (FBackend as ICanvasSupport).CanvasAllocated;
}

procedure TBitmap32.DeleteCanvas;
{
  if Supports(Backend, ICanvasSupport) then
    (FBackend as ICanvasSupport).DeleteCanvas;
}


{ TCustomBackend }

constructor TCustomBackend.Create;
{
  RefCounted := True;
  _AddRef;
  inherited;
}

constructor TCustomBackend.Create(Owner: TCustomBitmap32);
{
  FOwner := Owner;
  Create;
   if Assigned(Owner) then
    Owner.Backend := Self;
}

destructor TCustomBackend.Destroy;
{
  Clear;
  inherited;
}

procedure TCustomBackend.Clear;
var
  Width, Height: Integer;
{
  if Assigned(FOwner) then
    ChangeSize(FOwner.FWidth, FOwner.FHeight, 0, 0, False)
  else
    ChangeSize(Width, Height, 0, 0, False);
}

procedure TCustomBackend.Changing;
{
  if Assigned(FOnChanging) then
    FOnChanging(Self);
}

{$IFDEF BITS_GETTER}
function TCustomBackend.GetBits: PColor32Array;
{
  Result := FBits;
}
{$ENDIF}

procedure TCustomBackend.ChangeSize(var Width, Height: Integer; NewWidth, NewHeight: Integer; ClearBuffer: Boolean);
{
  try
    Changing;

    FinalizeSurface;

    Width := 0;
    Height := 0;

    if (NewWidth > 0) and (NewHeight > 0) then
      InitializeSurface(NewWidth, NewHeight, ClearBuffer);

    Width := NewWidth;
    Height := NewHeight;
  finally
    Changed;
  }
}

procedure TCustomBackend.Assign(Source: TPersistent);
var
  SrcBackend: TCustomBack}
{
  if Source is TCustomBackend then
  {
    if Assigned(FOwner) then
    {
      SrcBackend := TCustomBackend(Source);

      ChangeSize(
        FOwner.FWidth, FOwner.FHeight,
        SrcBackend.FOwner.Width, SrcBackend.FOwner.Height,
        False
      );

      if not SrcBackend.Empty then
        MoveLongword(
          SrcBackend.Bits[0], Bits[0],
          SrcBackend.FOwner.Width * SrcBackend.FOwner.Height
        );
    }
  end
  else
    inherited;
}

function TCustomBackend.Empty: Boolean;
{
  Result := False;
}

procedure TCustomBackend.FinalizeSurface;
{
  // descendants override this method
}

procedure TCustomBackend.InitializeSurface(NewWidth, NewHeight: Integer; ClearBuffer: Boolean);
{
  // descendants override this method
}

{ TCustomSampler }

function TCustomSampler.GetSampleInt(X, Y: Integer): TColor32;
{
  Result := GetSampleFixed(X * FixedOne, Y * FixedOne);
}

function TCustomSampler.GetSampleFixed(X, Y: TFixed): TColor32;
{
  Result := GetSampleFloat(X * FixedToFloat, Y * FixedToFloat);
}

function TCustomSampler.GetSampleFloat(X, Y: TFloat): TColor32;
{
  Result := GetSampleFixed(Fixed(X), Fixed(Y));
}

procedure TCustomSampler.PrepareSampling;
{
  // descendants override this method
}

procedure TCustomSampler.FinalizeSampling;
{
  // descendants override this method
}

function TCustomSampler.HasBounds: Boolean;
{
  Result := False;
}

function TCustomSampler.GetSampleBounds: TFloatRect;
const
  InfRect: TFloatRect = (Left: -Infinity; Top: -Infinity; Right: Infinity; Bottom: Infinity);
{
  Result := InfRect;
}


{ TCustomResampler }

procedure TCustomResampler.AssignTo(Dst: TPersistent);
{
  if Dst is TCustomResampler then
    SmartAssign(Self, Dst)
  else
    inherited;
}

procedure TCustomResampler.Changed;
{
  if Assigned(FBitmap) then FBitmap.Changed;
}

constructor TCustomResampler.Create;
{
  inherited;
  FPixelAccessMode := pamSafe;
}

constructor TCustomResampler.Create(ABitmap: TCustomBitmap32);
{
  Create;
  FBitmap := ABitmap;
  if Assigned(ABitmap) then ABitmap.Resampler := Self;
}

function TCustomResampler.GetSampleBounds: TFloatRect;
{
  Result := FloatRect(FBitmap.ClipRect);
  if PixelAccessMode = pamTransparentEdge then
    InflateRect(Result, 1, 1);
}

function TCustomResampler.GetWidth: TFloat;
{
  Result := 0;
}

function TCustomResampler.HasBounds: Boolean;
{
  Result := FPixelAccessMode <> pamWrap;
}

procedure TCustomResampler.PrepareSampling;
{
  FClipRect := FBitmap.ClipRect;
}

procedure TCustomResampler.SetPixelAccessMode(
  const Value: TPixelAccessMode);
{
  if FPixelAccessMode <> Value then
  {
    FPixelAccessMode := Value;
    Changed;
  }
}

initialization
  SetGamma;
  StockBitmap := TBitmap.Create;
  StockBitmap.Width := 8;
  StockBitmap.Height := 8;

finalization
  StockBitmap.Free;

end.
