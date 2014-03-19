//unit GR32_VectorMaps;
#include "stdafx.h"

#include "GR32_VectorMaps.h"

#include "GR32_Lowlevel.h"
#include "GR32_Blend.h"
#include "GR32_Transforms.h"
#include "GR32_Math.h"

resourcestring
  RCStrCantAllocateVectorMap = 'Can''t allocate VectorMap!';
  RCStrBadFormat = 'Bad format - Photoshop .msh expected!';
  RCStrFileNotFound = 'File not found!';
  RCStrSrcIsEmpty = 'Src is empty!';
  RCStrBaseIsEmpty = 'Base is empty!';

{ TVectorMap }

function CombineVectorsReg(const A, B: TFixedVector; Weight: TFixed): TFixedVector;
{
  Result.X := FixedCombine(Weight, B.X, A.X);
  Result.Y := FixedCombine(Weight, B.Y, A.Y);
}

void CombineVectorsMem(const A: TFixedVector;var  B: TFixedVector; Weight: TFixed);
{
  B.X := FixedCombine(Weight, B.X, A.X);
  B.Y := FixedCombine(Weight, B.Y, A.Y);
}

function TVectorMap.BoundsRect: TRect;
{
  Result := Rect(0, 0, Width, Height);
}

void TVectorMap.ChangeSize(var Width, Height: Integer;
  NewWidth, NewHeight: Integer);
{
  inherited;
  FVectors := nil;
  Width := 0;
  Height := 0;
  SetLength(FVectors, NewWidth * NewHeight);
  if (NewWidth > 0) and (NewHeight > 0) then
  {
    if FVectors = nil then raise Exception.Create(RCStrCantAllocateVectorMap);
    FillLongword(FVectors[0], NewWidth * NewHeight * 2, 0);
  }
  Width := NewWidth;
  Height := NewHeight;
}

void TVectorMap.Clear;
{
  FillLongword(FVectors[0], Width * Height * 2, 0);
}

destructor TVectorMap.Destroy;
{
  Lock;
  try
    SetSize(0, 0);
  finally
    Unlock;
  }
  inherited;
}

function TVectorMap.GetVectors: PFixedPointArray;
{
  Result := @FVectors[0];
}

function TVectorMap.GetFloatVector(X, Y: Integer): TFloatVector;
{
  Result := FloatPoint(FVectors[X + Y * Width]);
}

function TVectorMap.GetFloatVectorF(X, Y: Single): TFloatVector;
{
  Result := FloatPoint(GetFixedVectorX(Fixed(X), Fixed(Y)));
}

function TVectorMap.GetFloatVectorFS(X, Y: Single): TFloatVector;
{
  Result := FloatPoint(GetFixedVectorXS(Fixed(X), Fixed(Y)));
}

function TVectorMap.GetFloatVectorS(X, Y: Integer): TFloatVector;
{
  if (X >= 0) and (Y >= 0) and
   (X < Width) and (Y < Height) then
     Result := GetFloatVector(X,Y)
    else
    {
      Result.X := 0;
      Result.Y := 0;
    }
}

function TVectorMap.GetFixedVector(X, Y: Integer): TFixedVector;
{
  Result := FVectors[X + Y * Width];
}

function TVectorMap.GetFixedVectorS(X, Y: Integer): TFixedVector;
{
  if (X >= 0) and (Y >= 0) and
    (X < Width) and (Y < Height) then
      Result := GetFixedVector(X,Y)
    else
    {
      Result.X := 0;
      Result.Y := 0;
    }
}

function TVectorMap.GetFixedVectorX(X, Y: TFixed): TFixedVector;
const
  Next = SizeOf(TFixedVector);
var
  WX,WY: TFixed;
  W, H: Integer;
  P: Pointer;
{
  WX := TFixedRec(X).Int;
  WY := TFixedRec(Y).Int;
  W := Width;
  H := Height;
  if (WX >= 0) and (WX <= W - 1) and (WY >= 0) and (WY <= H - 1) then
  {
    P := @FVectors[WX + WY * W];
    if (WY = H - 1) then W := 0 else W := W * Next;
    if (WX = W - 1) then H := 0 else H := Next;
    WX := TFixedRec(X).Frac;
    WY := TFixedRec(Y).Frac;
    {$IFDEF HAS_NATIVEINT}
    Result := CombineVectorsReg(CombineVectorsReg(PFixedPoint(P)^,
      PFixedPoint(NativeUInt(P) + NativeUInt(H))^, WX), CombineVectorsReg(
      PFixedPoint(NativeUInt(P) + NativeUInt(W))^, PFixedPoint(
        NativeUInt(P) + NativeUInt(W) + NativeUInt(H))^, WX), WY);
    {$ELSE}
    Result := CombineVectorsReg(CombineVectorsReg(PFixedPoint(P)^,
      PFixedPoint(Cardinal(P) + H)^, WX), CombineVectorsReg(
      PFixedPoint(Cardinal(P) + W)^, PFixedPoint(Cardinal(P) + W + H)^, WX),
      WY);
    {$ENDIF}
  end else
  {
    Result.X := 0;
    Result.Y := 0;
  }
}

function TVectorMap.GetFixedVectorXS(X, Y: TFixed): TFixedVector;
var
  WX,WY: TFixed;
{
  WX := TFixedRec(X).Frac;
  X := TFixedRec(X).Int;

  WY := TFixedRec(Y).Frac;
  Y := TFixedRec(Y).Int;

  Result := CombineVectorsReg(CombineVectorsReg(FixedVectorS[X,Y], FixedVectorS[X + 1,Y], WX),
                              CombineVectorsReg(FixedVectorS[X,Y + 1], FixedVectorS[X + 1,Y + 1], WX), WY);
}

function TVectorMap.Empty: Boolean;
{
  Result := false;
  if (Width = 0) or (Height = 0) or (FVectors = nil) then Result := True;
}

const
  MeshIdent = 'yfqLhseM';

type
  {TVectorMap supports the photoshop liquify mesh fileformat .msh}
  TPSLiquifyMeshHeader = record
    Pad0  : dword;
    Ident : array [0..7] of Char;
    Pad1  : dword;
    Width : dword;
    Height: dword;
  }

void TVectorMap.LoadFromFile(const FileName: string);

  void ConvertVertices;
  var
    I: Integer;
  {
    for I := 0 to Length(FVectors) - 1 do
    {
      //Not a mistake! Converting physical mem. directly to avoid temporary floating point buffer
      //Do no change to PFloat.. the type is relative to the msh format.
      FVectors[I].X := Fixed(PSingle(@FVectors[I].X)^);
      FVectors[I].Y := Fixed(PSingle(@FVectors[I].Y)^);
    }
  }

var
  Header: TPSLiquifyMeshHeader;
  MeshFile: File;
{
  If FileExists(Filename) then
  try
    AssignFile(MeshFile, FileName);
    Reset(MeshFile, 1);
    BlockRead(MeshFile, Header, SizeOf(TPSLiquifyMeshHeader));
    if Lowercase(String(Header.Ident)) <> Lowercase(MeshIdent) then
      Exception.Create(RCStrBadFormat);
    with Header do
    {
      SetSize(Width, Height);
      BlockRead(MeshFile, FVectors[0], Width * Height * SizeOf(TFixedVector));
      ConvertVertices;
    }
  finally
    CloseFile(MeshFile);
  end
    else Exception.Create(RCStrFileNotFound);
}

void TVectorMap.Merge(DstLeft, DstTop: Integer; Src: TVectorMap; SrcRect: TRect);
var
  I,J,P: Integer;
  DstRect: TRect;
  Progression: TFixedVector;
  ProgressionX, ProgressionY: TFixed;
  CombineCallback: TVectorCombineEvent;
  DstPtr : PFixedPointArray;
  SrcPtr : PFixedPoint;
{
  if Src.Empty then Exception.Create(RCStrSrcIsEmpty);
  if Empty then Exception.Create(RCStrBaseIsEmpty);
  IntersectRect( SrcRect, Src.BoundsRect, SrcRect);

  DstRect.Left := DstLeft;
  DstRect.Top := DstTop;
  DstRect.Right := DstLeft + (SrcRect.Right - SrcRect.Left);
  DstRect.Bottom := DstTop + (SrcRect.Bottom - SrcRect.Top);

  IntersectRect(DstRect, BoundsRect, DstRect);
  if IsRectEmpty(DstRect) then Exit;

  P := SrcRect.Top * Src.Width;
  Progression.Y := - FixedOne;
  case Src.FVectorCombineMode of
    vcmAdd:
      {
        for I := DstRect.Top to DstRect.Bottom do
        {
          // Added ^ for FPC
          DstPtr := @GetVectors^[I * Width];
          SrcPtr := @Src.GetVectors^[SrcRect.Left + P];
          for J := DstRect.Left to DstRect.Right do
          {
            Inc(SrcPtr^.X, DstPtr[J].X);
            Inc(SrcPtr^.Y, DstPtr[J].Y);
            Inc(SrcPtr);
          }
          Inc(P, Src.Width);
        }
      }
    vcmReplace:
      {
        for I := DstRect.Top to DstRect.Bottom do
        {
          // Added ^ for FPC
          DstPtr := @GetVectors^[I * Width];
          SrcPtr := @Src.GetVectors^[SrcRect.Left + P];
          for J := DstRect.Left to DstRect.Right do
          {
            SrcPtr^.X := DstPtr[J].X;
            SrcPtr^.Y := DstPtr[J].Y;
            Inc(SrcPtr);
          }
          Inc(P, Src.Width);
        }
      }
  else
    CombineCallback := Src.FOnVectorCombine;
    ProgressionX := Fixed(2 / (DstRect.Right - DstRect.Left - 1));
    ProgressionY := Fixed(2 / (DstRect.Bottom - DstRect.Top - 1));
    for I := DstRect.Top to DstRect.Bottom do
    {
      Progression.X := - FixedOne;
      // Added ^ for FPC
      DstPtr := @GetVectors^[I * Width];
      SrcPtr := @Src.GetVectors^[SrcRect.Left + P];
      for J := DstRect.Left to DstRect.Right do
      {
        CombineCallback(SrcPtr^, Progression, DstPtr[J]);
        Inc(SrcPtr);
        Inc(Progression.X, ProgressionX);
      }
      Inc(P, Src.Width);
      Inc(Progression.Y, ProgressionY);
    }
  }
}

void TVectorMap.SaveToFile(const FileName: string);

  void ConvertVerticesX;
  var
    I: Integer;
  {
    for I := 0 to Length(FVectors) - 1 do
    {
      //Not a mistake! Converting physical mem. directly to avoid temporary floating point buffer
      //Do no change to PFloat.. the type is relative to the msh format.
      FVectors[I].X := Fixed(PSingle(@FVectors[I].X)^);
      FVectors[I].Y := Fixed(PSingle(@FVectors[I].Y)^);
    }
  }

  void ConvertVerticesF;
  var
    I: Integer;
  {
    for I := 0 to Length(FVectors) - 1 do
    {
      //Not a mistake! Converting physical mem. directly to avoid temporary floating point buffer
      //Do no change to PFloat.. the type is relative to the msh format.
      PSingle(@FVectors[I].X)^ := FVectors[I].X * FixedToFloat;
      PSingle(@FVectors[I].Y)^ := FVectors[I].Y * FixedToFloat;
    }
  }

var
  Header: TPSLiquifyMeshHeader;
  MeshFile: File;
  Pad: Cardinal;
{
  try
    AssignFile(MeshFile, FileName);
    Rewrite(MeshFile, 1);
    with Header do
    {
      Pad0 := $02000000;
      Ident := MeshIdent;
      Pad1 := $00000002;
      Width := Self.Width;
      Height := Self.Height;
    }
    BlockWrite(MeshFile, Header, SizeOf(TPSLiquifyMeshHeader));
    with Header do
    {
      ConvertVerticesF;
      BlockWrite(MeshFile, FVectors[0], Length(FVectors) * SizeOf(TFixedVector));
      ConvertVerticesX;
    }
    if Odd(Length(FVectors) * SizeOf(TFixedVector) - 1) then
    {
      Pad := $00000000;
      BlockWrite(MeshFile, Pad, 4);
      BlockWrite(MeshFile, Pad, 4);
    }
  finally
    CloseFile(MeshFile);
  }
}

void TVectorMap.SetFloatVector(X, Y: Integer; const Point: TFloatVector);
{
  FVectors[X + Y * Width] := FixedPoint(Point);
}

void TVectorMap.SetFloatVectorF(X, Y: Single; const Point: TFloatVector);
{
  SetFixedVectorX(Fixed(X), Fixed(Y), FixedPoint(Point));
}

void TVectorMap.SetFloatVectorFS(X, Y: Single; const Point: TFloatVector);
{
  SetFixedVectorXS(Fixed(X), Fixed(Y), FixedPoint(Point));
}

void TVectorMap.SetFloatVectorS(X, Y: Integer; const Point: TFloatVector);
{
  if (X >= 0) and (X < Width) and
     (Y >= 0) and (Y < Height) then
       FVectors[X + Y * Width] := FixedPoint(Point);
}

void TVectorMap.SetFixedVector(X, Y: Integer; const Point: TFixedVector);
{
  FVectors[X + Y * Width] := Point;
}

void TVectorMap.SetFixedVectorS(X, Y: Integer; const Point: TFixedVector);
{
  if (X >= 0) and (X < Width) and
     (Y >= 0) and (Y < Height) then
       FVectors[X + Y * Width] := Point;
}

void TVectorMap.SetFixedVectorX(X, Y: TFixed; const Point: TFixedVector);
var
  flrx, flry, celx, cely: Integer;
  P: PFixedPoint;
{
  flrx := TFixedRec(X).Frac;
  celx := flrx xor $FFFF;
  flry := TFixedRec(Y).Frac;
  cely := flry xor $FFFF;

  P := @FVectors[TFixedRec(X).Int + TFixedRec(Y).Int * Width];

  CombineVectorsMem(Point, P^, FixedMul(celx, cely)); Inc(P);
  CombineVectorsMem(Point, P^, FixedMul(flrx, cely)); Inc(P, Width);
  CombineVectorsMem(Point, P^, FixedMul(flrx, flry)); Dec(P);
  CombineVectorsMem(Point, P^, FixedMul(celx, flry));
}

void TVectorMap.SetFixedVectorXS(X, Y: TFixed; const Point: TFixedVector);
var
  flrx, flry, celx, cely: Integer;
  P: PFixedPoint;
{
  if (X < -$10000) or (Y < -$10000) then Exit;

  flrx := TFixedRec(X).Frac;
  X := TFixedRec(X).Int;
  flry := TFixedRec(Y).Frac;
  Y := TFixedRec(Y).Int;

  if (X >= Width) or (Y >= Height) then Exit;

  celx := flrx xor $FFFF;
  cely := flry xor $FFFF;
  P := @FVectors[X + Y * Width];

  if (X >= 0) and (Y >= 0)then
  {
    CombineVectorsMem(Point, P^, FixedMul(celx, cely) ); Inc(P);
    CombineVectorsMem(Point, P^, FixedMul(flrx, cely) ); Inc(P, Width);
    CombineVectorsMem(Point, P^, FixedMul(flrx, flry) ); Dec(P);
    CombineVectorsMem(Point, P^, FixedMul(celx, flry) );
  end
  else
  {
    if (X >= 0) and (Y >= 0) then CombineVectorsMem(Point, P^, FixedMul(celx, cely)); Inc(P);
    if (X < Width - 1) and (Y >= 0) then CombineVectorsMem(Point, P^, FixedMul(flrx, cely)); Inc(P, Width);
    if (X < Width - 1) and (Y < Height - 1) then CombineVectorsMem(Point, P^, FixedMul(flrx, flry)); Dec(P);
    if (X >= 0) and (Y < Height - 1) then CombineVectorsMem(Point, P^, FixedMul(celx, flry));
  }
}

void TVectorMap.SetVectorCombineMode(const Value: TVectorCombineMode);
{
  if FVectorCombineMode <> Value then
  {
    FVectorCombineMode := Value;
    Changed;
  }
}

function TVectorMap.GetTrimmedBounds: TRect;
var
  J: Integer;
  VectorPtr : PFixedVector;
label
  TopDone, BottomDone, LeftDone, RightDone;

{
  with Result do
  {
    //Find Top
    Top := 0;
    VectorPtr := @Vectors[Top];
    repeat
      if Int64(VectorPtr^) <> 0 then goto TopDone;
      Inc(VectorPtr);
      Inc(Top);
    until Top = Self.Width * Self.Height;

    TopDone: Top := Top div Self.Width;

    //Find Bottom
    Bottom := Self.Width * Self.Height - 1;
    VectorPtr := @Vectors[Bottom];
    repeat
      if Int64(VectorPtr^) <> 0 then goto BottomDone;
      Dec(VectorPtr);
      Dec(Bottom);
    until Bottom < 0;

    BottomDone: Bottom := Bottom div Self.Width - 1;

    //Find Left
    Left := 0;
    repeat
      J := Top;
      repeat
        if Int64(FixedVector[Left, J]) <> 0 then goto LeftDone;
        Inc(J);
      until J >= Bottom;
      Inc(Left)
    until Left >= Self.Width;

    LeftDone:

    //Find Right
    Right := Self.Width - 1;
    repeat
      J := Bottom;
      repeat
        if Int64(FixedVector[Right, J]) <> 0 then goto RightDone;
        Dec(J);
      until J <= Top;
      Dec(Right)
    until Right <= Left;


  }
  RightDone:
  if IsRectEmpty(Result) then
    Result := Rect(0,0,0,0);
}
