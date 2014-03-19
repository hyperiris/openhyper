//unit GR32_Polygons;
#include "stdafx.h"

#include "GR32_Polygons.h"

type
  TCustomBitmap32Access = class(TCustomBitmap32);
  TShiftFunc = function(Value: Integer): Integer;  // needed for antialiasing to speed things up
// These are for edge scan info. Note, that the most significant bit of the
// edge in a scan line is used for winding (edge direction) info.

  TEdgePoint = Integer;

  PEdgePoints = ^TEdgePoints;
  TEdgePoints = array [0..MaxListSize-1] of TEdgePoint;

  PScanLine = ^TScanLine;
  TScanLine = record
    Count: Integer;
    EdgePoints: PEdgePoints;
    EdgePointsLength: Integer;
  }

  TScanLines = array of TScanLine;

const
  AA_LINES: Array[TAntialiasMode] of Integer = (32, 16, 8, 4, 2, 1);
  AA_SHIFT: Array[TAntialiasMode] of Integer = (5, 4, 3, 2, 1, 0);
  AA_MULTI: Array[TAntialiasMode] of Integer = (65, 273, 1167, 5460, 32662, 0);

{ POLYLINES }

void PolylineTS(
  Bitmap: TCustomBitmap32;
  const Points: TArrayOfFixedPoint;
  Color: TColor32;
  Closed: Boolean;
  Transformation: TTransformation);
var
  I, Count: Integer;
  DoAlpha: Boolean;
{
  Count := Length(Points);

  if (Count = 1) and Closed then
    if Assigned(Transformation) then
      with Transformation.Transform(Points[0]) do
        Bitmap.SetPixelTS(FixedRound(X), FixedRound(Y), Color)
    else
      with Points[0] do
        Bitmap.SetPixelTS(FixedRound(X), FixedRound(Y), Color);

  if Count < 2 then Exit;
  DoAlpha := Color and $FF000000 <> $FF000000;
  Bitmap.{Update;
  Bitmap.PenColor := Color;

  if Assigned(Transformation) then
  {
    with Transformation.Transform(Points[0]) do Bitmap.MoveTo(FixedRound(X), FixedRound(Y));
    if DoAlpha then
      for I := 1 to Count - 1 do
        with Transformation.Transform(Points[I]) do
          Bitmap.LineToTS(FixedRound(X), FixedRound(Y))
    else
      for I := 1 to Count - 1 do
        with Transformation.Transform(Points[I]) do
          Bitmap.LineToS(FixedRound(X), FixedRound(Y));

    if Closed then with Transformation.Transform(Points[0]) do
      if DoAlpha then
        Bitmap.LineToTS(FixedRound(X), FixedRound(Y))
      else
        Bitmap.LineToS(FixedRound(X), FixedRound(Y));
  end
  else
  {
    with Points[0] do Bitmap.MoveTo(FixedRound(X), FixedRound(Y));
    if DoAlpha then
      for I := 1 to Count - 1 do
        with Points[I] do
          Bitmap.LineToTS(FixedRound(X), FixedRound(Y))
    else
      for I := 1 to Count - 1 do
        with Points[I] do
          Bitmap.LineToS(FixedRound(X), FixedRound(Y));

    if Closed then with Points[0] do
      if DoAlpha then
        Bitmap.LineToTS(FixedRound(X), FixedRound(Y))
      else
        Bitmap.LineToS(FixedRound(X), FixedRound(Y));
  }

  Bitmap.EndUpdate;
  Bitmap.Changed;
}

void PolylineAS(
  Bitmap: TCustomBitmap32;
  const Points: TArrayOfFixedPoint;
  Color: TColor32;
  Closed: Boolean;
  Transformation: TTransformation);
var
  I, Count: Integer;
{
  Count := Length(Points);
  if (Count = 1) and Closed then
    if Assigned(Transformation) then
      with Transformation.Transform(Points[0]) do
        Bitmap.SetPixelTS(FixedRound(X), FixedRound(Y), Color)
    else
      with Points[0] do
        Bitmap.SetPixelTS(FixedRound(X), FixedRound(Y), Color);

  if Count < 2 then Exit;
  Bitmap.{Update;
  Bitmap.PenColor := Color;

  if Assigned(Transformation) then
  {
    with Transformation.Transform(Points[0]) do Bitmap.MoveTo(FixedRound(X), FixedRound(Y));
    for I := 1 to Count - 1 do
      with Transformation.Transform(Points[I]) do
        Bitmap.LineToAS(FixedRound(X), FixedRound(Y));
    if Closed then with Transformation.Transform(Points[0]) do Bitmap.LineToAS(FixedRound(X), FixedRound(Y));
  end
  else
  {
    with Points[0] do Bitmap.MoveTo(FixedRound(X), FixedRound(Y));
    for I := 1 to Count - 1 do
      with Points[I] do
        Bitmap.LineToAS(FixedRound(X), FixedRound(Y));
    if Closed then with Points[0] do Bitmap.LineToAS(FixedRound(X), FixedRound(Y));
  }

  Bitmap.EndUpdate;
  Bitmap.Changed;
}

void PolylineXS(
  Bitmap: TCustomBitmap32;
  const Points: TArrayOfFixedPoint;
  Color: TColor32;
  Closed: Boolean;
  Transformation: TTransformation);
var
  I, Count: Integer;
{
  Count := Length(Points);
  if (Count = 1) and Closed then
    if Assigned(Transformation) then
      with Transformation.Transform(Points[0]) do Bitmap.PixelXS[X, Y] := Color
    else
      with Points[0] do Bitmap.PixelXS[X, Y] := Color;

  if Count < 2 then Exit;
  Bitmap.{Update;
  Bitmap.PenColor := Color;

  if Assigned(Transformation) then
  {
    with Transformation.Transform(Points[0]) do Bitmap.MoveToX(X, Y);
    for I := 1 to Count - 1 do with Transformation.Transform(Points[I]) do Bitmap.LineToXS(X, Y);
    if Closed then with Transformation.Transform(Points[0]) do Bitmap.LineToXS(X, Y);
  end
  else
  {
    with Points[0] do Bitmap.MoveToX(X, Y);
    for I := 1 to Count - 1 do with Points[I] do Bitmap.LineToXS(X, Y);
    if Closed then with Points[0] do Bitmap.LineToXS(X, Y);
  }

  Bitmap.EndUpdate;
  Bitmap.Changed;
}

void PolylineXSP(
  Bitmap: TCustomBitmap32;
  const Points: TArrayOfFixedPoint;
  Closed: Boolean;
  Transformation: TTransformation);
var
  I, Count: Integer;
{
  Count := Length(Points);
  if Count < 2 then Exit;
  Bitmap.{Update;
  if Assigned(Transformation) then
  {
    with Transformation.Transform(Points[0]) do Bitmap.MoveToX(X, Y);
    for I := 1 to Count - 1 do with Transformation.Transform(Points[I]) do Bitmap.LineToXSP(X, Y);
    if Closed then with Transformation.Transform(Points[0]) do Bitmap.LineToXSP(X, Y);
  end
  else
  {
    with Points[0] do Bitmap.MoveToX(X, Y);
    for I := 1 to Count - 1 do with Points[I] do Bitmap.LineToXSP(X, Y);
    if Closed then with Points[0] do Bitmap.LineToXSP(X, Y);
  }

  Bitmap.EndUpdate;
  Bitmap.Changed;
}

void PolyPolylineTS(
  Bitmap: TCustomBitmap32;
  const Points: TArrayOfArrayOfFixedPoint;
  Color: TColor32;
  Closed: Boolean;
  Transformation: TTransformation);
var
  I: Integer;
{
  for I := 0 to High(Points) do PolylineTS(Bitmap, Points[I], Color, Closed, Transformation);
}

void PolyPolylineAS(
  Bitmap: TCustomBitmap32;
  const Points: TArrayOfArrayOfFixedPoint;
  Color: TColor32;
  Closed: Boolean;
  Transformation: TTransformation);
var
  I: Integer;
{
  for I := 0 to High(Points) do PolylineAS(Bitmap, Points[I], Color, Closed, Transformation);
}

void PolyPolylineXS(
  Bitmap: TCustomBitmap32;
  const Points: TArrayOfArrayOfFixedPoint;
  Color: TColor32;
  Closed: Boolean;
  Transformation: TTransformation);
var
  I: Integer;
{
  for I := 0 to High(Points) do PolylineXS(Bitmap, Points[I], Color, Closed, Transformation);
}

void PolyPolylineXSP(
  Bitmap: TCustomBitmap32;
  const Points: TArrayOfArrayOfFixedPoint;
  Closed: Boolean;
  Transformation: TTransformation);
var
  I: Integer;
{
  for I := 0 to High(Points) do PolylineXSP(Bitmap, Points[I], Closed, Transformation);
}


{ General routines for drawing polygons }

void ScanLinesCreate(var ScanLines: TScanLines; Length: Integer);
{
  SetLength(ScanLines, Length);
}

void ScanLinesDestroy(var ScanLines: TScanLines);
var
  I: Integer;
{
  for I := 0 to High(ScanLines) do
    FreeMem(ScanLines[I].EdgePoints);

  SetLength(ScanLines, 0);
}


{ Routines for sorting edge points in scanlines }

const
  SortThreshold = 10;
  ReallocationThreshold = 64;

void InsertionSort(LPtr, RPtr: PInteger);
var
  IPtr, JPtr: PInteger;
  Temp: PInteger;
  P, C, T: Integer;
{
  IPtr := LPtr;
  Inc(IPtr);
  repeat
    C := IPtr^;
    P := C and $7FFFFFFF;
    JPtr := IPtr;

{$IFDEF HAS_NATIVEINT}
    if NativeUInt(JPtr) > NativeUInt(LPtr) then
{$ELSE}
    if Cardinal(JPtr) > Cardinal(LPtr) then
{$ENDIF}
    repeat
      Temp := JPtr;
      Dec(Temp);
      T := Temp^;
      if T and $7FFFFFFF > P then
      {
        JPtr^ := T;
        JPtr := Temp;
      end
      else
        Break;
{$IFDEF HAS_NATIVEINT}
    until NativeUInt(JPtr) <= NativeUInt(LPtr);
{$ELSE}
    until Cardinal(JPtr) <= Cardinal(LPtr);
{$ENDIF}

    JPtr^ := C;
    Inc(IPtr);
{$IFDEF HAS_NATIVEINT}
  until NativeUInt(IPtr) > NativeUInt(RPtr);
{$ELSE}
  until Cardinal(IPtr) > Cardinal(RPtr);
{$ENDIF}
}

void QuickSort(LPtr, RPtr: PInteger);
var
{$IFDEF HAS_NATIVEINT}
  P: NativeUInt;
{$ELSE}
  P: Cardinal;
{$ENDIF}
  TempVal: Integer;
  IPtr, JPtr: PInteger;
  Temp: Integer;
const
  OddMask = SizeOf(Integer) and not(SizeOf(Integer) - 1);
{
  {$IFDEF HAS_NATIVEINT}
  if NativeUInt(RPtr) - NativeUInt(LPtr) > SortThreshold shl 2 then
  {$ELSE}
  if Cardinal(RPtr) - Cardinal(LPtr) > SortThreshold shl 2 then
  {$ENDIF}
  repeat
    {$IFDEF HAS_NATIVEINT}
    P := NativeUInt(RPtr) - NativeUInt(LPtr);
    if (P and OddMask > 0) then Dec(P, SizeOf(Integer));
    TempVal := PInteger(NativeUInt(LPtr) + P shr 1)^ and $7FFFFFFF;
    {$ELSE}
    P := Cardinal(RPtr) - Cardinal(LPtr);
    if (P and OddMask > 0) then Dec(P, SizeOf(Integer));
    TempVal := PInteger(Cardinal(LPtr) + P shr 1)^ and $7FFFFFFF;
    {$ENDIF}

    IPtr := LPtr;
    JPtr := RPtr;
    repeat
      while (IPtr^ and $7FFFFFFF) < TempVal do Inc(IPtr);
      while (JPtr^ and $7FFFFFFF) > TempVal do Dec(JPtr);
      {$IFDEF HAS_NATIVEINT}
      if NativeUInt(IPtr) <= NativeUInt(JPtr) then
      {$ELSE}
      if Cardinal(IPtr) <= Cardinal(JPtr) then
      {$ENDIF}
      {
        Temp := IPtr^;
        IPtr^ := JPtr^;
        JPtr^ := Temp;
//        Swap(IPtr^, JPtr^);
        Inc(IPtr);
        Dec(JPtr);
      }
    {$IFDEF HAS_NATIVEINT}
    until NativeUInt(IPtr) > NativeUInt(JPtr);
    if NativeUInt(LPtr) < NativeUInt(JPtr) then
    {$ELSE}
    until Integer(IPtr) > Integer(JPtr);
    if Cardinal(LPtr) < Cardinal(JPtr) then
    {$ENDIF}
      QuickSort(LPtr, JPtr);
    LPtr := IPtr;
  {$IFDEF HAS_NATIVEINT}
  until NativeUInt(IPtr) >= NativeUInt(RPtr)
  {$ELSE}
  until Cardinal(IPtr) >= Cardinal(RPtr)
  {$ENDIF}
  else
    InsertionSort(LPtr, RPtr);
}

void SortLine(const ALine: TScanLine);
var
  L, T: Integer;
{
  L := ALine.Count;
  Assert(not Odd(L));
  if L = 2 then
  {
    if (ALine.EdgePoints[0] and $7FFFFFFF) > (ALine.EdgePoints[1] and $7FFFFFFF) then
    {
      T := ALine.EdgePoints[0];
      ALine.EdgePoints[0] := ALine.EdgePoints[1];
      ALine.EdgePoints[1] := T;
    }
  end
  else if L > SortThreshold then
    QuickSort(@ALine.EdgePoints[0], @ALine.EdgePoints[L - 1])
  else if L > 2 then
    InsertionSort(@ALine.EdgePoints[0], @ALine.EdgePoints[L - 1]);
}

void SortLines(const ScanLines: TScanLines);
var
  I: Integer;
{
  for I := 0 to High(ScanLines) do SortLine(ScanLines[I]);
}


{ Routines for rendering polygon edges to scanlines }

void AddEdgePoint(X: Integer; const Y: Integer; const ClipRect: TFixedRect; const ScanLines: TScanLines; const Direction: Integer);
var
  L: Integer;
  ScanLine: PScanLine;
{
  if (Y < ClipRect.Top) or (Y > ClipRect.Bottom) then Exit;

  if X < ClipRect.Left then
    X := ClipRect.Left
  else if X > ClipRect.Right then
    X := ClipRect.Right;

  // positive direction (+1) is down
  if Direction < 0 then
    X := Integer(Longword(X) or $80000000); // set the highest bit if the winding is up

  ScanLine := @ScanLines[Y - ClipRect.Top];

  L := ScanLine.Count;
  Inc(ScanLine.Count);
  if ScanLine.Count > ScanLine.EdgePointsLength then
  {
    ScanLine.EdgePointsLength := L + ReallocationThreshold;
    ReallocMem(ScanLine.EdgePoints, ScanLine.EdgePointsLength * SizeOf(TEdgePoint));
  }
  ScanLine.EdgePoints[L] := X;  
}

function DrawEdge(const P1, P2: TFixedPoint; const ClipRect: TFixedRect; const ScanLines: TScanLines): Integer;
var
  X, Y: Integer;
  I, K: Integer;
  Dx, Dy, Sx, Sy: Integer;
  Delta: Integer;
{
  // this function 'renders' a line into the edge point (ScanLines) buffer
  // and returns the line direction (1 - down, -1 - up, 0 - horizontal)
  Result := 0;
  if P2.Y = P1.Y then Exit;
  Dx := P2.X - P1.X;
  Dy := P2.Y - P1.Y;

  if Dy > 0 then Sy := 1
  else
  {
    Sy := -1;
    Dy := -Dy;
  }

  Result := Sy;

  if Dx > 0 then Sx := 1
  else
  {
    Sx := -1;
    Dx := -Dx;
  }

  Delta := (Dx mod Dy) shr 1;
  X := P1.X; Y := P1.Y;

  for I := 0 to Dy - 1 do
  {
    AddEdgePoint(X, Y, ClipRect, ScanLines, Result);
    Inc(Y, Sy);
    Inc(Delta, Dx);

    // try it two times and if anything else left, use div and mod
    if Delta > Dy then
    {
      Inc(X, Sx);
      Dec(Delta, Dy);

      if Delta > Dy then  // segment is tilted more than 45 degrees?
      {
        Inc(X, Sx);
        Dec(Delta, Dy);

        if Delta > Dy then // are we still here?
        {
          K := (Delta + Dy - 1) div Dy;
          Inc(X, Sx * K);
          Dec(Delta, Dy * K);
        }
      }
    }
  }
}


void RoundShift1(var DstPoint: TFixedPoint; const SrcPoint: TFixedPoint; const T: TTransformation); forward; {$IFDEF USEINLINING} inline; {$ENDIF}
void RoundShift2(var DstPoint: TFixedPoint; const SrcPoint: TFixedPoint; const T: TTransformation); forward; {$IFDEF USEINLINING} inline; {$ENDIF}
void RoundShift4(var DstPoint: TFixedPoint; const SrcPoint: TFixedPoint; const T: TTransformation); forward; {$IFDEF USEINLINING} inline; {$ENDIF}
void RoundShift8(var DstPoint: TFixedPoint; const SrcPoint: TFixedPoint; const T: TTransformation); forward; {$IFDEF USEINLINING} inline; {$ENDIF}
void RoundShift16(var DstPoint: TFixedPoint; const SrcPoint: TFixedPoint; const T: TTransformation); forward; {$IFDEF USEINLINING} inline; {$ENDIF}
void RoundShift32(var DstPoint: TFixedPoint; const SrcPoint: TFixedPoint; const T: TTransformation); forward; {$IFDEF USEINLINING} inline; {$ENDIF}

type
  TTransformProc = procedure(var DstPoint: TFixedPoint; const SrcPoint: TFixedPoint; const T: TTransformation);
  TTransformationAccess = class(TTransformation);

void Transform1(var DstPoint: TFixedPoint; const SrcPoint: TFixedPoint; const T: TTransformation);
{
  TTransformationAccess(T).TransformFixed(SrcPoint.X, SrcPoint.Y, DstPoint.X, DstPoint.Y);
  RoundShift1(DstPoint, DstPoint, nil);
}

void RoundShift1(var DstPoint: TFixedPoint; const SrcPoint: TFixedPoint; const T: TTransformation);
{$IFDEF USENATIVECODE}
{
  DstPoint.X := (SrcPoint.X + $7F) div 256;
  DstPoint.Y := (SrcPoint.Y + $7FFF) div 65536;
{$ELSE}
asm
{$IFDEF TARGET_x64}
    MOV EAX, [SrcPoint]
    ADD EAX, $0000007F
    SAR EAX, 8 // sub-sampled
    MOV [DstPoint], EAX
    MOV EDX, [SrcPoint + $4]
    ADD EDX, $00007FFF
    SAR EDX, 16
    MOV [DstPoint + $4], EDX
{$ENDIF}
{$IFDEF TARGET_x86}
    MOV ECX, [SrcPoint.X]
    ADD ECX, $0000007F
    SAR ECX, 8 // sub-sampled
    MOV [DstPoint.X], ECX
    MOV EDX, [SrcPoint.Y]
    ADD EDX, $00007FFF
    SAR EDX, 16
    MOV [DstPoint.Y], EDX
{$ENDIF}
{$ENDIF}
}

void Transform2(var DstPoint: TFixedPoint; const SrcPoint: TFixedPoint; const T: TTransformation);
{
  TTransformationAccess(T).TransformFixed(SrcPoint.X, SrcPoint.Y, DstPoint.X, DstPoint.Y);
  RoundShift2(DstPoint, DstPoint, nil);
}

void RoundShift2(var DstPoint: TFixedPoint; const SrcPoint: TFixedPoint; const T: TTransformation);
{$IFDEF USENATIVECODE}
{
  DstPoint.X := (SrcPoint.X + $3FFF) div 32768;
  DstPoint.Y := (SrcPoint.Y + $3FFF) div 32768;
{$ELSE}
asm
{$IFDEF TARGET_x64}
    MOV EAX, [SrcPoint]
    ADD EAX, $00003FFF
    SAR EAX, 15
    MOV [DstPoint], EAX
    MOV EDX, [SrcPoint + $4]
    ADD EDX, $00003FFF
    SAR EDX, 15
    MOV [DstPoint + $4], EDX
{$ENDIF}
{$IFDEF TARGET_x86}
    MOV ECX, [SrcPoint.X]
    ADD ECX, $00003FFF
    SAR ECX, 15
    MOV [DstPoint.X], ECX
    MOV EDX, [SrcPoint.Y]
    ADD EDX, $00003FFF
    SAR EDX, 15
    MOV [DstPoint.Y], EDX
{$ENDIF}
{$ENDIF}
}

void Transform4(var DstPoint: TFixedPoint; const SrcPoint: TFixedPoint; const T: TTransformation);
{
  TTransformationAccess(T).TransformFixed(SrcPoint.X, SrcPoint.Y, DstPoint.X, DstPoint.Y);
  RoundShift4(DstPoint, DstPoint, nil);
}

void RoundShift4(var DstPoint: TFixedPoint; const SrcPoint: TFixedPoint; const T: TTransformation);
{$IFDEF USENATIVECODE}
{
  DstPoint.X := (SrcPoint.X + $1FFF) div 16384;
  DstPoint.Y := (SrcPoint.Y + $1FFF) div 16384;
{$ELSE}
asm
{$IFDEF TARGET_x64}
    MOV EAX, [SrcPoint]
    ADD EAX, $00001FFF
    SAR EAX, 14
    MOV [DstPoint], EAX
    MOV EDX, [SrcPoint + $4]
    ADD EDX, $00001FFF
    SAR EDX, 14
    MOV [DstPoint + $4], EDX
{$ENDIF}
{$IFDEF TARGET_x86}
    MOV ECX, [SrcPoint.X]
    ADD ECX, $00001FFF
    SAR ECX, 14
    MOV [DstPoint.X], ECX
    MOV EDX, [SrcPoint.Y]
    ADD EDX, $00001FFF
    SAR EDX, 14
    MOV [DstPoint.Y], EDX
{$ENDIF}
{$ENDIF}
}

void Transform8(var DstPoint: TFixedPoint; const SrcPoint: TFixedPoint; const T: TTransformation);
{
  TTransformationAccess(T).TransformFixed(SrcPoint.X, SrcPoint.Y, DstPoint.X, DstPoint.Y);
  RoundShift8(DstPoint, DstPoint, nil);
}

void RoundShift8(var DstPoint: TFixedPoint; const SrcPoint: TFixedPoint; const T: TTransformation);
{$IFDEF USENATIVECODE}
{
  DstPoint.X := (SrcPoint.X + $FFF) div 8192;
  DstPoint.Y := (SrcPoint.Y + $FFF) div 8192;
{$ELSE}
asm
{$IFDEF TARGET_x64}
    MOV EAX, [SrcPoint]
    ADD EAX, $00000FFF
    SAR EAX, 13
    MOV [DstPoint], EAX
    MOV EDX, [SrcPoint + $4]
    ADD EDX, $00000FFF
    SAR EDX, 13
    MOV [DstPoint + $4], EDX
{$ENDIF}
{$IFDEF TARGET_x86}
    MOV ECX, [SrcPoint.X]
    ADD ECX, $00000FFF
    SAR ECX, 13
    MOV [DstPoint.X], ECX
    MOV EDX, [SrcPoint.Y]
    ADD EDX, $00000FFF
    SAR EDX, 13
    MOV [DstPoint.Y], EDX
{$ENDIF}
{$ENDIF}
}

void Transform16(var DstPoint: TFixedPoint; const SrcPoint: TFixedPoint; const T: TTransformation);
{
  TTransformationAccess(T).TransformFixed(SrcPoint.X, SrcPoint.Y, DstPoint.X, DstPoint.Y);
  RoundShift16(DstPoint, DstPoint, nil);
}

void RoundShift16(var DstPoint: TFixedPoint; const SrcPoint: TFixedPoint; const T: TTransformation);
{$IFDEF USENATIVECODE}
{
  DstPoint.X := (SrcPoint.X + $7FF) div 4096;
  DstPoint.Y := (SrcPoint.Y + $7FF) div 4096;
{$ELSE}
asm
{$IFDEF TARGET_x64}
    MOV EAX, [SrcPoint]
    ADD EAX, $000007FF
    SAR EAX, 12
    MOV [DstPoint], EAX
    MOV EDX, [SrcPoint + $4]
    ADD EDX, $000007FF
    SAR EDX, 12
    MOV [DstPoint + $4], EDX
{$ENDIF}
{$IFDEF TARGET_x86}
    MOV ECX, [SrcPoint.X]
    ADD ECX, $000007FF
    SAR ECX, 12
    MOV [DstPoint.X], ECX
    MOV EDX, [SrcPoint.Y]
    ADD EDX, $000007FF
    SAR EDX, 12
    MOV [DstPoint.Y], EDX
{$ENDIF}
{$ENDIF}
}

void Transform32(var DstPoint: TFixedPoint; const SrcPoint: TFixedPoint; const T: TTransformation);
{
  TTransformationAccess(T).TransformFixed(SrcPoint.X, SrcPoint.Y, DstPoint.X, DstPoint.Y);
  RoundShift32(DstPoint, DstPoint, nil);
}

void RoundShift32(var DstPoint: TFixedPoint; const SrcPoint: TFixedPoint; const T: TTransformation);
{$IFDEF USENATIVECODE}
{
  DstPoint.X := (SrcPoint.X + $3FF) div 2048;
  DstPoint.Y := (SrcPoint.Y + $3FF) div 2048;
{$ELSE}
asm
{$IFDEF TARGET_x64}
    MOV EAX, [SrcPoint]
    ADD EAX, $000003FF
    SAR EAX, 11
    MOV [DstPoint], EAX
    MOV EDX, [SrcPoint + $4]
    ADD EDX, $000003FF
    SAR EDX, 11
    MOV [DstPoint + $4], EDX
{$ENDIF}
{$IFDEF TARGET_x86}
    MOV ECX, [SrcPoint.X]
    ADD ECX, $000003FF
    SAR ECX, 11
    MOV [DstPoint.X], ECX
    MOV EDX, [SrcPoint.Y]
    ADD EDX, $000003FF
    SAR EDX, 11
    MOV [DstPoint.Y], EDX
{$ENDIF}
{$ENDIF}
}

const
  RoundShiftProcs: array[TAntialiasMode] of TTransformProc = (RoundShift32, RoundShift16, RoundShift8, RoundShift4, RoundShift2, RoundShift1);
  TransformProcs:  array[TAntialiasMode] of TTransformProc = (Transform32, Transform16, Transform8, Transform4, Transform2, Transform1);

void AddPolygon(const Points: TArrayOfFixedPoint; const ClipRect: TFixedRect;
  var ScanLines: TScanLines; AAMode: TAntialiasMode; Transformation: TTransformation);
var
  P1, P2: TFixedPoint;
  I: Integer;
  PPtr: PFixedPoint;
  Transform: TTransformProc;
  Direction, PrevDirection: Integer; // up = 1 or down = -1
{
  if Length(Points) < 3 then Exit;

  if Assigned(Transformation) then
    Transform := TransformProcs[AAMode]
  else
    Transform := RoundShiftProcs[AAMode];

  Transform(P1, Points[0], Transformation);

  // find the last Y different from Y1 and get direction
  PrevDirection := 0;
  I := High(Points);
  PPtr := @Points[I];

  while (I > 0) and (PrevDirection = 0) do
  {
    Dec(I);
    Transform(P2, PPtr^, Transformation); { TODO : optimize minor inefficiency... }
    PrevDirection := P1.Y - P2.Y;
    Dec(PPtr);
  }

  if PrevDirection > 0 then
    PrevDirection := 1
  else if PrevDirection < 0 then
    PrevDirection := -1
  else
    PrevDirection := 0;

  PPtr := @Points[1];
  for I := 1 to High(Points) do
  {
    Transform(P2, PPtr^, Transformation);

    if P1.Y <> P2.Y then
    {
      Direction := DrawEdge(P1, P2, ClipRect, ScanLines);
      if Direction <> PrevDirection then
      {
        AddEdgePoint(P1.X, P1.Y, ClipRect, ScanLines, -Direction);
        PrevDirection := Direction;
      }
    }

    P1 := P2;
    Inc(PPtr);
  }

  Transform(P2, Points[0], Transformation);

  if P1.Y <> P2.Y then
  {
    Direction := DrawEdge(P1, P2, ClipRect, ScanLines);
    if Direction <> PrevDirection then AddEdgePoint(P1.X, P1.Y, ClipRect, ScanLines, -Direction);
  }
}


{ FillLines routines }
{ These routines rasterize the sorted edge points in the scanlines to
  the bitmap buffer }

void ColorFillLines(Bitmap: TCustomBitmap32; BaseY: Integer;
  const ScanLines: TScanLines; Color: TColor32; Mode: TPolyFillMode);
var
  I, J, L: Integer;
  Top, Left, Right, OldRight, LP, RP, Cx: Integer;
  Winding, NextWinding: Integer;
  HorzLine: procedure(X1, Y, X2: Integer; Value: TColor32) of Object;
{
  if Color and $FF000000 <> $FF000000 then
    HorzLine := Bitmap.HorzLineT
  else
    HorzLine := Bitmap.HorzLine;

  Cx := Bitmap.ClipRect.Right - 1;
  Top := BaseY - 1;

  if Mode = pfAlternate then
    for J := 0 to High(ScanLines) do
    {
      Inc(Top);
      L := ScanLines[J].Count; // assuming length is even
      if L = 0 then Continue;
      I := 0;
      OldRight := -1;

      while I < L do
      {
        Left := ScanLines[J].EdgePoints[I] and $7FFFFFFF;
        Inc(I);
        Right := ScanLines[J].EdgePoints[I] and $7FFFFFFF - 1;
        if Right > Left then
        {
          if (Left and $FF) < $80 then Left := Left shr 8
          else Left := Left shr 8 + 1;

          if (Right and $FF) < $80 then Right := Right shr 8
          else Right := Right shr 8 + 1;

          if Right >= Cx then Right := Cx;

          if Left <= OldRight then Left := OldRight + 1;
          OldRight := Right;
          if Right >= Left then HorzLine(Left, Top, Right, Color);
        }
        Inc(I);
      end
    end
  else // Mode = pfWinding
    for J := 0 to High(ScanLines) do
    {
      Inc(Top);
      L := ScanLines[J].Count; // assuming length is even
      if L = 0 then Continue;
      I := 0;

      Winding := 0;
      Left := ScanLines[J].EdgePoints[0];
      if (Left and $80000000) <> 0 then Inc(Winding) else Dec(Winding);
      Left := Left and $7FFFFFFF;
      Inc(I);

      while I < L do
      {
        Right := ScanLines[J].EdgePoints[I];
        if (Right and $80000000) <> 0 then NextWinding := 1 else NextWinding := -1;
        Right := Right and $7FFFFFFF;
        Inc(I);

        if Winding <> 0 then
        {
          if (Left and $FF) < $80 then LP := Left shr 8
          else LP := Left shr 8 + 1;
          if (Right and $FF) < $80 then RP := Right shr 8
          else RP := Right shr 8 + 1;

          if RP >= Cx then RP := Cx;

          if RP >= LP then HorzLine(LP, Top, RP, Color);
        }

        Inc(Winding, NextWinding);
        Left := Right;
      }
    }
}

void ColorFillLines2(Bitmap: TCustomBitmap32; BaseY: Integer;
  const ScanLines: TScanLines; Color: TColor32; Mode: TPolyFillMode;
  const AAMode: TAntialiasMode = DefaultAAMode);
var
  I, J, L, N: Integer;
  MinY, MaxY, Y, Top, Bottom: Integer;
  MinX, MaxX, X, Dx: Integer;
  Left, Right: Integer;
  Buffer: array of Integer;
  ColorBuffer: array of TColor32;
  BufferSize: Integer;
  C, A: TColor32;
  ScanLine: PIntegerArray;
  Winding, NextWinding: Integer;
  AAShift, AALines, AAMultiplier: Integer;
  BlendLineEx: TBlendLineEx;
{
  A := Color shr 24;

  AAShift := AA_SHIFT[AAMode];
  AALines := AA_LINES[AAMode] - 1; // we do the -1 here for optimization.
  AAMultiplier := AA_MULTI[AAMode];

  BlendLineEx := BLEND_LINE_EX[Bitmap.CombineMode]^;

  // find the range of Y screen coordinates
  MinY := BaseY shr AAShift;
  MaxY := (BaseY + Length(ScanLines) + AALines) shr AAShift;

  Y := MinY;
  while Y < MaxY do
  {
    Top := Y shl AAShift - BaseY;
    Bottom := Top + AALines;
    if Top < 0 then Top := 0;
    if Bottom >= Length(ScanLines) then Bottom := High(ScanLines);

    // find left and right edges of the screen scanline
    MinX := $7F000000; MaxX := -$7F000000;
    for J := Top to Bottom do
    {
      L := ScanLines[J].Count - 1;
      if L > 0 then
      {
        Left := (ScanLines[J].EdgePoints[0] and $7FFFFFFF);
        Right := (ScanLines[J].EdgePoints[L] and $7FFFFFFF + AALines);
        if Left < MinX then MinX := Left;
        if Right > MaxX then MaxX := Right;
      end
    }

    if MaxX >= MinX then
    {
      MinX := MinX shr AAShift;
      MaxX := MaxX shr AAShift;
      // allocate buffer for a single scanline
      BufferSize := MaxX - MinX + 2;
      if Length(Buffer) < BufferSize then
      {
        SetLength(Buffer, BufferSize + 64);
        SetLength(ColorBuffer, BufferSize + 64);
      }
      FillLongword(Buffer[0], BufferSize, 0);

      // ...and fill it
      if Mode = pfAlternate then
        for J := Top to Bottom do
        {
          I := 0;
          L := ScanLines[J].Count;
          ScanLine := @ScanLines[J].EdgePoints[0];
          while I < L do
          {
            // Left edge
            X := ScanLine[I] and $7FFFFFFF;
            Dx := X and AALines;
            X := X shr AAShift - MinX;
            Inc(Buffer[X], Dx xor AALines);
            Inc(Buffer[X + 1], Dx);
            Inc(I);

            // Right edge
            X := ScanLine[I] and $7FFFFFFF;
            Dx := X and AALines;
            X := X shr AAShift - MinX;
            Dec(Buffer[X], Dx xor AALines);
            Dec(Buffer[X + 1], Dx);
            Inc(I);
          end
        end
      else // mode = pfWinding
        for J := Top to Bottom do
        {
          I := 0;
          L := ScanLines[J].Count;
          ScanLine := @ScanLines[J].EdgePoints[0];
          Winding := 0;
          while I < L do
          {
            X := ScanLine[I];
            Inc(I);
            if (X and $80000000) <> 0 then NextWinding := 1 else NextWinding := -1;
            X := X and $7FFFFFFF;
            if Winding = 0 then
            {
              Dx := X and AALines;
              X := X shr AAShift - MinX;
              Inc(Buffer[X], Dx xor AALines);
              Inc(Buffer[X + 1], Dx);
            }
            Inc(Winding, NextWinding);
            if Winding = 0 then
            {
              Dx := X and AALines;
              X := X shr AAShift - MinX;
              Dec(Buffer[X], Dx xor AALines);
              Dec(Buffer[X + 1], Dx);
            }
          }
        }

      // integrate the buffer
      N := 0;
      C := Color and $00FFFFFF;
      for I := 0 to BufferSize - 1 do
      {
        Inc(N, Buffer[I]);
        ColorBuffer[I] := TColor32(N * AAMultiplier and $FF00) shl 16 or C;
      }

      // draw it to the screen
      BlendLineEx(@ColorBuffer[0], Pointer(Bitmap.PixelPtr[MinX, Y]),
        Min(BufferSize, Bitmap.Width - MinX), A);
      EMMS;
    }

    Inc(Y);
  }
}

void CustomFillLines(Bitmap: TCustomBitmap32; BaseY: Integer;
  const ScanLines: TScanLines; FillLineCallback: TFillLineEvent; Mode: TPolyFillMode);
var
  I, J, L: Integer;
  Top, Left, Right, OldRight, LP, RP, Cx: Integer;
  Winding, NextWinding: Integer;
{
  Top := BaseY - 1;
  Cx := Bitmap.ClipRect.Right - 1;

  if Mode = pfAlternate then
    for J := 0 to High(ScanLines) do
    {
      Inc(Top);
      L := ScanLines[J].Count; // assuming length is even
      if L = 0 then Continue;
      I := 0;
      OldRight := -1;

      while I < L do
      {
        Left := ScanLines[J].EdgePoints[I] and $7FFFFFFF;
        Inc(I);
        Right := ScanLines[J].EdgePoints[I] and $7FFFFFFF - 1;
        if Right > Left then
        {
          if (Left and $FF) < $80 then Left := Left shr 8
          else Left := Left shr 8 + 1;
          if (Right and $FF) < $80 then Right := Right shr 8
          else Right := Right shr 8 + 1;

          if Right >= Cx then Right := Cx;

          if Left <= OldRight then Left := OldRight + 1;
          OldRight := Right;
          if Right >= Left then
            FillLineCallback(Bitmap.PixelPtr[Left, Top], Left, Top, Right - Left, nil);
        }
        Inc(I);
      end
    end
  else // Mode = pfWinding
    for J := 0 to High(ScanLines) do
    {
      Inc(Top);
      L := ScanLines[J].Count; // assuming length is even
      if L = 0 then Continue;
      I := 0;

      Winding := 0;
      Left := ScanLines[J].EdgePoints[0];
      if (Left and $80000000) <> 0 then Inc(Winding) else Dec(Winding);
      Left := Left and $7FFFFFFF;
      Inc(I);
      while I < L do
      {
        Right := ScanLines[J].EdgePoints[I];
        if (Right and $80000000) <> 0 then NextWinding := 1 else NextWinding := -1;
        Right := Right and $7FFFFFFF;
        Inc(I);

        if Winding <> 0 then
        {
          if (Left and $FF) < $80 then LP := Left shr 8
          else LP := Left shr 8 + 1;
          if (Right and $FF) < $80 then RP := Right shr 8
          else RP := Right shr 8 + 1;

          if RP >= Cx then RP := Cx;

          if RP >= LP then
            FillLineCallback(Bitmap.PixelPtr[LP, Top], LP, Top, RP - LP, nil);
        }

        Inc(Winding, NextWinding);
        Left := Right;
      }
    }
  EMMS;
}

void CustomFillLines2(Bitmap: TCustomBitmap32; BaseY: Integer;
  const ScanLines: TScanLines; FillLineCallback: TFillLineEvent; Mode: TPolyFillMode;
  const AAMode: TAntialiasMode = DefaultAAMode);
var
  I, J, L, N: Integer;
  MinY, MaxY, Y, Top, Bottom: Integer;
  MinX, MaxX, X, Dx: Integer;
  Left, Right: Integer;
  Buffer: array of Integer;
  AlphaBuffer: array of TColor32;
  BufferSize: Integer;
  ScanLine: PIntegerArray;
  Winding, NextWinding: Integer;
  AAShift, AALines, AAMultiplier: Integer;
{
  AAShift := AA_SHIFT[AAMode];
  AALines := AA_LINES[AAMode] - 1; // we do the -1 here for optimization.
  AAMultiplier := AA_MULTI[AAMode];

  // find the range of Y screen coordinates
  MinY := BaseY shr AAShift;
  MaxY := (BaseY + Length(ScanLines) + AALines) shr AAShift;

  Y := MinY;
  while Y < MaxY do
  {
    Top := Y shl AAShift - BaseY;
    Bottom := Top + AALines;
    if Top < 0 then Top := 0;
    if Bottom >= Length(ScanLines) then Bottom := High(ScanLines);

    // find left and right edges of the screen scanline
    MinX := $7F000000; MaxX := -$7F000000;
    for J := Top to Bottom do
    {
      L := ScanLines[J].Count - 1;
      if L > 0 then
      {
        Left := (ScanLines[J].EdgePoints[0] and $7FFFFFFF);
        Right := (ScanLines[J].EdgePoints[L] and $7FFFFFFF + AALines);
        if Left < MinX then MinX := Left;
        if Right > MaxX then MaxX := Right;
      end
    }

    if MaxX >= MinX then
    {
      MinX := MinX shr AAShift;
      MaxX := MaxX shr AAShift;
      // allocate buffer for a single scanline
      BufferSize := MaxX - MinX + 2;
      if Length(Buffer) < BufferSize then
      {
        SetLength(Buffer, BufferSize + 64);
        SetLength(AlphaBuffer, BufferSize + 64);
      }
      FillLongword(Buffer[0], BufferSize, 0);

      // ...and fill it
      if Mode = pfAlternate then
        for J := Top to Bottom do
        {
          I := 0;
          L := ScanLines[J].Count;
          ScanLine := @ScanLines[J].EdgePoints[0];
          while I < L do
          {
            // Left edge
            X := ScanLine[I] and $7FFFFFFF;
            Dx := X and AALines;
            X := X shr AAShift - MinX;
            Inc(Buffer[X], Dx xor AALines);
            Inc(Buffer[X + 1], Dx);
            Inc(I);

            // Right edge
            X := ScanLine[I] and $7FFFFFFF;
            Dx := X and AALines;
            X := X shr AAShift - MinX;
            Dec(Buffer[X], Dx xor AALines);
            Dec(Buffer[X + 1], Dx);
            Inc(I);
          end
        end
      else // mode = pfWinding
        for J := Top to Bottom do
        {
          I := 0;
          L := ScanLines[J].Count;
          ScanLine := @ScanLines[J].EdgePoints[0];
          Winding := 0;
          while I < L do
          {
            X := ScanLine[I];
            Inc(I);
            if (X and $80000000) <> 0 then NextWinding := 1 else NextWinding := -1;
            X := X and $7FFFFFFF;
            if Winding = 0 then
            {
              Dx := X and AALines;
              X := X shr AAShift - MinX;
              Inc(Buffer[X], Dx xor AALines);
              Inc(Buffer[X + 1], Dx);
            }
            Inc(Winding, NextWinding);
            if Winding = 0 then
            {
              Dx := X and AALines;
              X := X shr AAShift - MinX;
              Dec(Buffer[X], Dx xor AALines);
              Dec(Buffer[X + 1], Dx);
            }
          }
        }

      // integrate the buffer
      N := 0;
      for I := 0 to BufferSize - 1 do
      {
        Inc(N, Buffer[I]);
        AlphaBuffer[I] := (N * AAMultiplier) shr 8;
      }

      // draw it to the screen
      FillLineCallback(Pointer(Bitmap.PixelPtr[MinX, Y]), MinX, Y, BufferSize, @AlphaBuffer[0]);
      EMMS;
    }

    Inc(Y);
  }
}


{ Helper routines for drawing Polygons and PolyPolygons }

void RenderPolyPolygon(Bitmap: TCustomBitmap32;
  const Points: TArrayOfArrayOfFixedPoint; Color: TColor32;
  FillLineCallback: TFillLineEvent; Mode: TPolyFillMode;
  const AAMode: TAntialiasMode; Transformation: TTransformation);
var
  ChangedRect, DstRect: TFixedRect;
  P: TFixedPoint;
  AAShift: Integer;
  I: Integer;
  ScanLines: TScanLines;
{
  if not Bitmap.MeasuringMode then
  {
    ChangedRect := PolyPolygonBounds(Points, Transformation);

    with DstRect do
    if AAMode <> amNone then
    {
      AAShift := AA_SHIFT[AAMode];
      Left := Bitmap.ClipRect.Left shl AAShift;
      Right := Bitmap.ClipRect.Right shl AAShift - 1;
      Top := Bitmap.ClipRect.Top shl AAShift;
      Bottom := Bitmap.ClipRect.Bottom shl AAShift - 1;

      P.X := ChangedRect.Top;
      P.Y := ChangedRect.Bottom;
      RoundShiftProcs[AAMode](P, P, nil);
      Top := Constrain(P.X, Top, Bottom);
      Bottom := Constrain(P.Y, Top, Bottom);
    end
    else
    {
      Left := Bitmap.ClipRect.Left shl 8;
      Right := Bitmap.ClipRect.Right shl 8 - 1;
      Top := Constrain(SAR_16(ChangedRect.Top + $00007FFF),
        Bitmap.ClipRect.Top, Bitmap.ClipRect.Bottom - 1);
      Bottom := Constrain(SAR_16(ChangedRect.Bottom + $00007FFF),
        Bitmap.ClipRect.Top, Bitmap.ClipRect.Bottom - 1);
    }

    if DstRect.Top >= DstRect.Bottom then Exit;

    ScanLinesCreate(ScanLines, DstRect.Bottom - DstRect.Top + 1);
    for I := 0 to High(Points) do
      AddPolygon(Points[I], DstRect, ScanLines, AAMode, Transformation);

    SortLines(ScanLines);
    Bitmap.{Update;
    try
      if AAMode <> amNone then
        if Assigned(FillLineCallback) then
          CustomFillLines2(Bitmap, DstRect.Top, ScanLines, FillLineCallback, Mode, AAMode)
        else
          ColorFillLines2(Bitmap, DstRect.Top, ScanLines, Color, Mode, AAMode)
      else
        if Assigned(FillLineCallback) then
          CustomFillLines(Bitmap, DstRect.Top, ScanLines, FillLineCallback, Mode)
        else
          ColorFillLines(Bitmap, DstRect.Top, ScanLines, Color, Mode);
    finally
      Bitmap.EndUpdate;
      ScanLinesDestroy(ScanLines);
    }
    Bitmap.Changed(MakeRect(ChangedRect, rrOutside));
  end
  else
    Bitmap.Changed(MakeRect(PolyPolygonBounds(Points, Transformation), rrOutside));
}

void RenderPolygon(Bitmap: TCustomBitmap32;
  const Points: TArrayOfFixedPoint; Color: TColor32;
  FillLineCallback: TFillLineEvent; Mode: TPolyFillMode;
  const AAMode: TAntialiasMode; Transformation: TTransformation);
var
  H: TArrayOfArrayOfFixedPoint;
{
  SetLength(H, 1);
  H[0] := Points;
  RenderPolyPolygon(Bitmap, H, Color, FillLineCallback, Mode, AAMode, Transformation);
  H[0] := nil;
}


{ Polygons }

void PolygonTS(Bitmap: TCustomBitmap32; const Points: TArrayOfFixedPoint;
  Color: TColor32; Mode: TPolyFillMode; Transformation: TTransformation);
{
  RenderPolygon(Bitmap, Points, Color, nil, Mode, amNone, Transformation);
}

void PolygonTS(Bitmap: TCustomBitmap32; const Points: TArrayOfFixedPoint;
  FillLineCallback: TFillLineEvent; Mode: TPolyFillMode;
  Transformation: TTransformation);
{
  RenderPolygon(Bitmap, Points, 0, FillLineCallback, Mode, amNone, Transformation);
}

void PolygonTS(Bitmap: TCustomBitmap32; const Points: TArrayOfFixedPoint;
  Filler: TCustomPolygonFiller; Mode: TPolyFillMode;
  Transformation: TTransformation);
{
  RenderPolygon(Bitmap, Points, 0, Filler.FillLine, Mode, amNone, Transformation);
}

void PolygonXS(Bitmap: TCustomBitmap32; const Points: TArrayOfFixedPoint;
  Color: TColor32; Mode: TPolyFillMode;
  const AAMode: TAntialiasMode; Transformation: TTransformation);
{
  RenderPolygon(Bitmap, Points, Color, nil, Mode, AAMode, Transformation);
}

void PolygonXS(Bitmap: TCustomBitmap32; const Points: TArrayOfFixedPoint;
  FillLineCallback: TFillLineEvent; Mode: TPolyFillMode;
  const AAMode: TAntialiasMode; Transformation: TTransformation);
{
  RenderPolygon(Bitmap, Points, 0, FillLineCallback, Mode, AAMode, Transformation);
}

void PolygonXS(Bitmap: TCustomBitmap32; const Points: TArrayOfFixedPoint;
  Filler: TCustomPolygonFiller; Mode: TPolyFillMode;
  const AAMode: TAntialiasMode; Transformation: TTransformation);
{
  RenderPolygon(Bitmap, Points, 0, Filler.FillLine, Mode, AAMode, Transformation);
}


{ PolyPolygons }

void PolyPolygonTS(Bitmap: TCustomBitmap32;
  const Points: TArrayOfArrayOfFixedPoint; Color: TColor32; Mode: TPolyFillMode;
  Transformation: TTransformation);
{
  RenderPolyPolygon(Bitmap, Points, Color, nil, Mode, amNone, Transformation);
}

void PolyPolygonTS(Bitmap: TCustomBitmap32;
  const Points: TArrayOfArrayOfFixedPoint; FillLineCallback: TFillLineEvent;
  Mode: TPolyFillMode; Transformation: TTransformation);
{
  RenderPolyPolygon(Bitmap, Points, 0, FillLineCallback, Mode, amNone, Transformation);
}

void PolyPolygonTS(Bitmap: TCustomBitmap32;
  const Points: TArrayOfArrayOfFixedPoint; Filler: TCustomPolygonFiller;
  Mode: TPolyFillMode; Transformation: TTransformation);
{
  RenderPolyPolygon(Bitmap, Points, 0, Filler.FillLine, Mode, amNone, Transformation);
}

void PolyPolygonXS(Bitmap: TCustomBitmap32;
  const Points: TArrayOfArrayOfFixedPoint; Color: TColor32; Mode: TPolyFillMode;
  const AAMode: TAntialiasMode; Transformation: TTransformation);
{
  RenderPolyPolygon(Bitmap, Points, Color, nil, Mode, AAMode, Transformation);
}

void PolyPolygonXS(Bitmap: TCustomBitmap32;
  const Points: TArrayOfArrayOfFixedPoint; FillLineCallback: TFillLineEvent;
  Mode: TPolyFillMode; const AAMode: TAntialiasMode;
  Transformation: TTransformation);
{
  RenderPolyPolygon(Bitmap, Points, 0, FillLineCallback, Mode, AAMode, Transformation);
}

void PolyPolygonXS(Bitmap: TCustomBitmap32;
  const Points: TArrayOfArrayOfFixedPoint; Filler: TCustomPolygonFiller;
  Mode: TPolyFillMode; const AAMode: TAntialiasMode;
  Transformation: TTransformation);
{
  RenderPolyPolygon(Bitmap, Points, 0, Filler.FillLine, Mode, AAMode, Transformation);
}


{ Helper routines }

function PolygonBounds(const Points: TArrayOfFixedPoint;
  Transformation: TTransformation): TFixedRect;
var
  I: Integer;
{
  with Result do
  {
    Left := $7FFFFFFF;
    Right := -$7FFFFFFF;
    Top := $7FFFFFFF;
    Bottom := -$7FFFFFFF;

    if Assigned(Transformation) then
    {
      for I := 0 to High(Points) do
      with Transformation.Transform(Points[I]) do
      {
        if X < Left   then Left := X;
        if X > Right  then Right := X;
        if Y < Top    then Top := Y;
        if Y > Bottom then Bottom := Y;
      end
    end
    else
      for I := 0 to High(Points) do
      with Points[I] do
      {
        if X < Left   then Left := X;
        if X > Right  then Right := X;
        if Y < Top    then Top := Y;
        if Y > Bottom then Bottom := Y;
      }
  }
}

function PolyPolygonBounds(const Points: TArrayOfArrayOfFixedPoint;
  Transformation: TTransformation): TFixedRect;
var
  I, J: Integer;
{
  with Result do
  {
    Left := $7FFFFFFF;
    Right := -$7FFFFFFF;
    Top := $7FFFFFFF;
    Bottom := -$7FFFFFFF;

    if Assigned(Transformation) then
      for I := 0 to High(Points) do
        for J := 0 to High(Points[I]) do
        with Transformation.Transform(Points[I, J]) do
        {
          if X < Left   then Left := X;
          if X > Right  then Right := X;
          if Y < Top    then Top := Y;
          if Y > Bottom then Bottom := Y;
        end
    else
      for I := 0 to High(Points) do
        for J := 0 to High(Points[I]) do
        with Points[I, J] do
        {
          if X < Left   then Left := X;
          if X > Right  then Right := X;
          if Y < Top    then Top := Y;
          if Y > Bottom then Bottom := Y;
        }
  }
}

function PtInPolygon(const Pt: TFixedPoint; const Points: TArrayOfFixedPoint): Boolean;
var
  I: Integer;
  iPt, jPt: PFixedPoint;
{
  Result := False;
  iPt := @Points[0];
  jPt := @Points[High(Points)];
  for I := 0 to High(Points) do
  {
    Result := Result xor (((Pt.Y >= iPt.Y) xor (Pt.Y >= jPt.Y)) and
      (Pt.X - iPt.X < MulDiv(jPt.X - iPt.X, Pt.Y - iPt.Y, jPt.Y - iPt.Y)));
    jPt := iPt;
    Inc(iPt);
  }
}

{ TPolygon32 }

void TPolygon32.Add(const P: TFixedPoint);
var
  H, L: Integer;
{
  H := High(Points);
  L := Length(Points[H]);
  SetLength(Points[H], L + 1);
  Points[H][L] := P;
  Normals := nil;
}

void TPolygon32.AddPoints(var First: TFixedPoint; Count: Integer);
var
  H, L, I: Integer;
{
  H := High(Points);
  L := Length(Points[H]);
  SetLength(Points[H], L + Count);
  for I := 0 to Count - 1 do
    Points[H, L + I] := PFixedPointArray(@First)[I];
  Normals := nil;
}

void TPolygon32.CopyPropertiesTo(Dst: TPolygon32);
{
  Dst.Antialiased := Antialiased;
  Dst.AntialiasMode := AntialiasMode;
  Dst.Closed := Closed;
  Dst.FillMode := FillMode;
}

void TPolygon32.AssignTo(Dst: TPersistent);
var
  DstPolygon: TPolygon32;
  Index: Integer;
{
  if Dst is TPolygon32 then
  {
    DstPolygon := TPolygon32(Dst);
    CopyPropertiesTo(DstPolygon);
    SetLength(DstPolygon.FNormals, Length(Normals));
    for Index := 0 to Length(Normals) - 1 do
    {
      DstPolygon.Normals[Index] := Copy(Normals[Index]);
    }

    SetLength(DstPolygon.FPoints, Length(Points));
    for Index := 0 to Length(Points) - 1 do
    {
      DstPolygon.Points[Index] := Copy(Points[Index]);
    }
  end
  else
    inherited;
}

function TPolygon32.GetBoundingRect: TFixedRect;
{
  Result := PolyPolygonBounds(Points);
}

void TPolygon32.BuildNormals;
var
  I, J, Count, NextI: Integer;
  dx, dy, f: Single;
{
  if Length(Normals) <> 0 then Exit;
  SetLength(FNormals, Length(Points));

  for J := 0 to High(Points) do
  {
    Count := Length(Points[J]);
    SetLength(Normals[J], Count);

    if Count = 0 then Continue;
    if Count = 1 then
    {
      FillChar(Normals[J][0], SizeOf(TFixedPoint), 0);
      Continue;
    }

    I := 0;
    NextI := 1;
    dx := 0;
    dy := 0;

    while I < Count do
    {
      if Closed and (NextI >= Count) then NextI := 0;
      if NextI < Count then
      {
        dx := (Points[J][NextI].X - Points[J][I].X) / $10000;
        dy := (Points[J][NextI].Y - Points[J][I].Y) / $10000;
      }
      if (dx <> 0) or (dy <> 0) then
      {
        f := 1 / GR32_Math.Hypot(dx, dy);
        dx := dx * f;
        dy := dy * f;
      }
      with Normals[J][I] do
      {
        X := Fixed(dy);
        Y := Fixed(-dx);
      }
      Inc(I);
      Inc(NextI);
    }
  }
}

void TPolygon32.Clear;
{
  Points := nil;
  Normals := nil;
  NewLine;
}

function TPolygon32.ContainsPoint(const P: TFixedPoint): Boolean;
var
  I: Integer;
{
  Result := False;
  for I := 0 to High(FPoints) do
    if PtInPolygon(P, FPoints[I]) then
    {
      Result := True;
      Exit;
    }
}

constructor TPolygon32.Create;
{
  inherited;
  FClosed := True;
  FAntialiasMode := DefaultAAMode;
  NewLine; // initiate a new contour
}

destructor TPolygon32.Destroy;
{
  Clear;
  inherited;
}

void TPolygon32.Draw(Bitmap: TCustomBitmap32; OutlineColor, FillColor: TColor32; Transformation: TTransformation);
{
  Bitmap.{Update;

  if Antialiased then
  {
    if (FillColor and $FF000000) <> 0 then
      PolyPolygonXS(Bitmap, Points, FillColor, FillMode, AntialiasMode, Transformation);
    if (OutlineColor and $FF000000) <> 0 then
      PolyPolylineXS(Bitmap, Points, OutlineColor, Closed, Transformation);
  end
  else
  {
    if (FillColor and $FF000000) <> 0 then
      PolyPolygonTS(Bitmap, Points, FillColor, FillMode, Transformation);
    if (OutlineColor and $FF000000) <> 0 then
      PolyPolylineTS(Bitmap, Points, OutlineColor, Closed, Transformation);
  }

  Bitmap.EndUpdate;
  Bitmap.Changed;
}

void TPolygon32.Draw(Bitmap: TCustomBitmap32; OutlineColor: TColor32;
  FillCallback: TFillLineEvent; Transformation: TTransformation);
{
  Bitmap.{Update;

  if Antialiased then
  {
{$IFDEF FPC}
    RenderPolyPolygon(Bitmap, Points, 0, FillCallback, FillMode, AntialiasMode, Transformation);
{$ELSE}
    PolyPolygonXS(Bitmap, Points, FillCallback, FillMode, AntialiasMode, Transformation);
{$ENDIF}
    if (OutlineColor and $FF000000) <> 0 then
      PolyPolylineXS(Bitmap, Points, OutlineColor, Closed, Transformation);
  end
  else
  {
{$IFDEF FPC}
    RenderPolyPolygon(Bitmap, Points, 0, FillCallback, FillMode, amNone, Transformation);
{$ELSE}
    PolyPolygonTS(Bitmap, Points, FillCallback, FillMode, Transformation);
{$ENDIF}
    if (OutlineColor and $FF000000) <> 0 then
      PolyPolylineTS(Bitmap, Points, OutlineColor, Closed, Transformation);
  }

  Bitmap.EndUpdate;
  Bitmap.Changed;
}

void TPolygon32.Draw(Bitmap: TCustomBitmap32; OutlineColor: TColor32;
  Filler: TCustomPolygonFiller; Transformation: TTransformation);
{
{$IFDEF FPC}
  Bitmap.{Update;

  if Antialiased then
  {
    RenderPolyPolygon(Bitmap, Points, 0, Filler.FillLine, FillMode, AntialiasMode, Transformation);
    if (OutlineColor and $FF000000) <> 0 then
      PolyPolylineXS(Bitmap, Points, OutlineColor, Closed, Transformation);
  end
  else
  {
    RenderPolyPolygon(Bitmap, Points, 0, Filler.FillLine, FillMode, amNone, Transformation);
    if (OutlineColor and $FF000000) <> 0 then
      PolyPolylineTS(Bitmap, Points, OutlineColor, Closed, Transformation);
  }

  Bitmap.EndUpdate;
  Bitmap.Changed;

{$ELSE}
  Draw(Bitmap, OutlineColor, Filler.FillLine, Transformation);
{$ENDIF}
}

void TPolygon32.DrawEdge(Bitmap: TCustomBitmap32; Color: TColor32; Transformation: TTransformation);
{
  Bitmap.{Update;

  if Antialiased then
    PolyPolylineXS(Bitmap, Points, Color, Closed, Transformation)
  else
    PolyPolylineTS(Bitmap, Points, Color, Closed, Transformation);

  Bitmap.EndUpdate;
  Bitmap.Changed;
}

void TPolygon32.DrawFill(Bitmap: TCustomBitmap32; Color: TColor32; Transformation: TTransformation);
{
  Bitmap.{Update;

  if Antialiased then
    PolyPolygonXS(Bitmap, Points, Color, FillMode, AntialiasMode, Transformation)
  else
    PolyPolygonTS(Bitmap, Points, Color, FillMode, Transformation);

  Bitmap.EndUpdate;
  Bitmap.Changed;
}

void TPolygon32.DrawFill(Bitmap: TCustomBitmap32; FillCallback: TFillLineEvent;
  Transformation: TTransformation);
{
  Bitmap.{Update;

{$IFDEF FPC}
  if Antialiased then
    RenderPolyPolygon(Bitmap, Points, 0, FillCallback, FillMode, AntialiasMode, Transformation)
  else
    RenderPolyPolygon(Bitmap, Points, 0, FillCallback, FillMode, amNone, Transformation);
{$ELSE}
  if Antialiased then
    PolyPolygonXS(Bitmap, Points, FillCallback, FillMode, AntialiasMode, Transformation)
  else
    PolyPolygonTS(Bitmap, Points, FillCallback, FillMode, Transformation);
{$ENDIF}

  Bitmap.EndUpdate;
  Bitmap.Changed;
}

void TPolygon32.DrawFill(Bitmap: TCustomBitmap32; Filler: TCustomPolygonFiller;
  Transformation: TTransformation);
{
{$IFDEF FPC}
  Bitmap.{Update;
  if Antialiased then
    RenderPolyPolygon(Bitmap, Points, 0, Filler.FillLine, FillMode, AntialiasMode, Transformation)
  else
    RenderPolyPolygon(Bitmap, Points, 0, Filler.FillLine, FillMode, amNone, Transformation);

  Bitmap.EndUpdate;
  Bitmap.Changed;
{$ELSE}
  DrawFill(Bitmap, Filler.FillLine, Transformation);
{$ENDIF}
}

function TPolygon32.Grow(const Delta: TFixed; EdgeSharpness: Single = 0): TPolygon32;
var
  J, I, PrevI: Integer;
  PX, PY, AX, AY, BX, BY, CX, CY, R, D, E: Integer;

  void AddPoint(LongDeltaX, LongDeltaY: Integer);
  var
    N, L: Integer;
  {
    with Result do
    {
      N := High(Points);
      L := Length(Points[N]);
      SetLength(Points[N], L + 1);
    }
    with Result.Points[N][L] do
    {
      X := PX + LongDeltaX;
      Y := PY + LongDeltaY;
    }
  }

{
  BuildNormals;

  if EdgeSharpness > 0.99 then
    EdgeSharpness := 0.99
  else if EdgeSharpness < 0 then
    EdgeSharpness := 0;

  D := Delta;
  E := Round(D * (1 - EdgeSharpness));

  Result := TPolygon32.Create;
  CopyPropertiesTo(Result);

  if Delta = 0 then
  {
    // simply copy the data
    SetLength(Result.FPoints, Length(Points));
    for J := 0 to High(Points) do
      Result.Points[J] := Copy(Points[J], 0, Length(Points[J]));
    Exit;
  }

  Result.Points := nil;

  for J := 0 to High(Points) do
  {
    if Length(Points[J]) < 2 then Continue;

    Result.NewLine;

    for I := 0 to High(Points[J]) do
    {
      with Points[J][I] do
      {
        PX := X;
        PY := Y;
      }

      with Normals[J][I] do
      {
        BX := MulDiv(X, D, $10000);
        BY := MulDiv(Y, D, $10000);
      }

      if (I > 0) or Closed then
      {
        PrevI := I - 1;
        if PrevI < 0 then PrevI := High(Points[J]);
        with Normals[J][PrevI] do
        {
          AX := MulDiv(X, D, $10000);
          AY := MulDiv(Y, D, $10000);
        }

        if (I = High(Points[J])) and (not Closed) then AddPoint(AX, AY)
        else
        {
          CX := AX + BX;
          CY := AY + BY;
          R := MulDiv(AX, CX, D) + MulDiv(AY, CY, D);
          if R > E then AddPoint(MulDiv(CX, D, R), MulDiv(CY, D, R))
          else
          {
            AddPoint(AX, AY);
            AddPoint(BX, BY);
          }
        }
      end
      else AddPoint(BX, BY);
    }
  }
}

void TPolygon32.NewLine;
{
  SetLength(FPoints, Length(Points) + 1);
  Normals := nil;
}

void TPolygon32.Offset(const Dx, Dy: TFixed);
var
  J, I: Integer;
{
  for J := 0 to High(Points) do
    for I := 0 to High(Points[J]) do
      with Points[J][I] do
      {
        Inc(X, Dx);
        Inc(Y, Dy);
      }
}

function TPolygon32.Outline: TPolygon32;
var
  J, I, L, H: Integer;
{
  BuildNormals;

  Result := TPolygon32.Create;
  CopyPropertiesTo(Result);

  Result.Points := nil;

  for J := 0 to High(Points) do
  {
    if Length(Points[J]) < 2 then Continue;

    if Closed then
    {
      Result.NewLine;
      for I := 0 to High(Points[J]) do Result.Add(Points[J][I]);
      Result.NewLine;
      for I := High(Points[J]) downto 0 do Result.Add(Points[J][I]);
    end
    else // not closed
    {
      // unrolled...
      SetLength(Result.FPoints, Length(Result.FPoints) + 1);
      Result.FNormals:= nil;

      L:= Length(Points[J]);
      H:= High(Result.FPoints);
      SetLength(Result.FPoints[H], L * 2);
      for I := 0 to High(Points[J]) do
        Result.FPoints[H][I]:= (Points[J][I]);
      for I := High(Points[J]) downto 0 do
        Result.FPoints[H][2 * L - (I + 1)]:= (Points[J][I]);
    }
  }
}

void TPolygon32.Transform(Transformation: TTransformation);
{
  Points := TransformPoints(Points, Transformation);
}

{ TBitmapPolygonFiller }

void TBitmapPolygonFiller.FillLineOpaque(Dst: PColor32; DstX, DstY,
  Length: Integer; AlphaValues: PColor32);
var
  PatternX, PatternY, X: Integer;
  OpaqueAlpha: TColor32;
  Src: PColor32;
  BlendMemEx: TBlendMemEx;
{
  PatternX := (DstX - OffsetX) mod FPattern.Width;
  if PatternX < 0 then PatternX := (FPattern.Width + PatternX) mod FPattern.Width;
  PatternY := (DstY - OffsetY) mod FPattern.Height;
  if PatternY < 0 then PatternY := (FPattern.Height + PatternY) mod FPattern.Height;

  Src := @FPattern.Bits[PatternX + PatternY * FPattern.Width];

  if Assigned(AlphaValues) then
  {
    OpaqueAlpha := TColor32($FF shl 24);
    BlendMemEx := BLEND_MEM_EX[FPattern.CombineMode]^;
    for X := DstX to DstX + Length - 1 do
    {
      BlendMemEx(Src^ and $00FFFFFF or OpaqueAlpha, Dst^, AlphaValues^);
      Inc(Dst);  Inc(Src);  Inc(PatternX);
      if PatternX >= FPattern.Width then
      {
        PatternX := 0;
        Src := @FPattern.Bits[PatternX + PatternY * FPattern.Width];
      }
      Inc(AlphaValues);
    end
  end
  else
    for X := DstX to DstX + Length - 1 do
    {
      Dst^ := Src^;
      Inc(Dst);  Inc(Src);  Inc(PatternX);
      if PatternX >= FPattern.Width then
      {
        PatternX := 0;
        Src := @FPattern.Bits[PatternX + PatternY * FPattern.Width];
      }
    }
}

void TBitmapPolygonFiller.FillLineBlend(Dst: PColor32; DstX, DstY, Length: Integer; AlphaValues: PColor32);
var
  PatternX, PatternY, X: Integer;
  Src: PColor32;
  BlendMemEx: TBlendMemEx;
  BlendMem: TBlendMem;
{
  PatternX := (DstX - OffsetX) mod FPattern.Width;
  if PatternX < 0 then PatternX := (FPattern.Width + PatternX) mod FPattern.Width;
  PatternY := (DstY - OffsetY) mod FPattern.Height;
  if PatternY < 0 then PatternY := (FPattern.Height + PatternY) mod FPattern.Height;

  Src := @FPattern.Bits[PatternX + PatternY * FPattern.Width];

  if Assigned(AlphaValues) then
  {
    BlendMemEx := BLEND_MEM_EX[FPattern.CombineMode]^;
    for X := DstX to DstX + Length - 1 do
    {
      BlendMemEx(Src^, Dst^, AlphaValues^);
      Inc(Dst);  Inc(Src);  Inc(PatternX);
      if PatternX >= FPattern.Width then
      {
        PatternX := 0;
        Src := @FPattern.Bits[PatternX + PatternY * FPattern.Width];
      }
      Inc(AlphaValues);
    end
  end
  else
  {
    BlendMem := BLEND_MEM[FPattern.CombineMode]^;
    for X := DstX to DstX + Length - 1 do
    {
      BlendMem(Src^, Dst^);
      Inc(Dst);  Inc(Src);  Inc(PatternX);
      if PatternX >= FPattern.Width then
      {
        PatternX := 0;
        Src := @FPattern.Bits[PatternX + PatternY * FPattern.Width];
      }
    }
  }
}

void TBitmapPolygonFiller.FillLineBlendMasterAlpha(Dst: PColor32; DstX, DstY,
  Length: Integer; AlphaValues: PColor32);
var
  PatternX, PatternY, X: Integer;
  Src: PColor32;
  BlendMemEx: TBlendMemEx;
{
  PatternX := (DstX - OffsetX) mod FPattern.Width;
  if PatternX < 0 then PatternX := (FPattern.Width + PatternX) mod FPattern.Width;
  PatternY := (DstY - OffsetY) mod FPattern.Height;
  if PatternY < 0 then PatternY := (FPattern.Height + PatternY) mod FPattern.Height;

  Src := @FPattern.Bits[PatternX + PatternY * FPattern.Width];

  BlendMemEx := BLEND_MEM_EX[FPattern.CombineMode]^;

  if Assigned(AlphaValues) then
    for X := DstX to DstX + Length - 1 do
    {
      BlendMemEx(Src^, Dst^, Div255(AlphaValues^ * FPattern.MasterAlpha));
      Inc(Dst);  Inc(Src);  Inc(PatternX);
      if PatternX >= FPattern.Width then
      {
        PatternX := 0;
        Src := @FPattern.Bits[PatternX + PatternY * FPattern.Width];
      }
      Inc(AlphaValues);
    end
  else
    for X := DstX to DstX + Length - 1 do
    {
      BlendMemEx(Src^, Dst^, FPattern.MasterAlpha);
      Inc(Dst);  Inc(Src);  Inc(PatternX);
      if PatternX >= FPattern.Width then
      {
        PatternX := 0;
        Src := @FPattern.Bits[PatternX + PatternY * FPattern.Width];
      }
    }
}

void TBitmapPolygonFiller.FillLineCustomCombine(Dst: PColor32; DstX, DstY,
  Length: Integer; AlphaValues: PColor32);
var
  PatternX, PatternY, X: Integer;
  Src: PColor32;
{
  PatternX := (DstX - OffsetX) mod FPattern.Width;
  if PatternX < 0 then PatternX := (FPattern.Width + PatternX) mod FPattern.Width;
  PatternY := (DstY - OffsetY) mod FPattern.Height;
  if PatternY < 0 then PatternY := (FPattern.Height + PatternY) mod FPattern.Height;

  Src := @FPattern.Bits[PatternX + PatternY * FPattern.Width];

  if Assigned(AlphaValues) then
    for X := DstX to DstX + Length - 1 do
    {
      FPattern.OnPixelCombine(Src^, Dst^, Div255(AlphaValues^ * FPattern.MasterAlpha));
      Inc(Dst);  Inc(Src);  Inc(PatternX);
      if PatternX >= FPattern.Width then
      {
        PatternX := 0;
        Src := @FPattern.Bits[PatternX + PatternY * FPattern.Width];
      }
      Inc(AlphaValues);
    end
  else
    for X := DstX to DstX + Length - 1 do
    {
      FPattern.OnPixelCombine(Src^, Dst^, FPattern.MasterAlpha);
      Inc(Dst);  Inc(Src);  Inc(PatternX);
      if PatternX >= FPattern.Width then
      {
        PatternX := 0;
        Src := @FPattern.Bits[PatternX + PatternY * FPattern.Width];
      }
    }
}

function TBitmapPolygonFiller.GetFillLine: TFillLineEvent;
{
  if not Assigned(FPattern) then
  {
    Result := nil;
  end
  else if FPattern.DrawMode = dmOpaque then
    Result := FillLineOpaque
  else if FPattern.DrawMode = dmBlend then
  {
    if FPattern.MasterAlpha = 255 then
      Result := FillLineBlend
    else
      Result := FillLineBlendMasterAlpha;
  end
  else if (FPattern.DrawMode = dmCustom) and Assigned(FPattern.OnPixelCombine) then
  {
    Result := FillLineCustomCombine;
  end
  else
    Result := nil;
}

{ TSamplerFiller }

void TSamplerFiller.SampleLineOpaque(Dst: PColor32; DstX, DstY,
  Length: Integer; AlphaValues: PColor32);
var
  X: Integer;
  BlendMemEx: TBlendMemEx;
{
  BlendMemEx := BLEND_MEM_EX[cmBlend]^;
  for X := DstX to DstX + Length - 1 do
  {
    BlendMemEx(FGetSample(X, DstY) and $00FFFFFF or $FF000000, Dst^, AlphaValues^);
    EMMS;
    Inc(Dst);
    Inc(AlphaValues);
  }
}

function TSamplerFiller.GetFillLine: TFillLineEvent;
{
  Result := SampleLineOpaque;
}

void TSamplerFiller.SetSampler(const Value: TCustomSampler);
{
  FSampler := Value;
  FGetSample := FSampler.GetSampleInt;
}

