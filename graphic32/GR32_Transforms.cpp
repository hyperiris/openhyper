//unit GR32_Transforms;

#include "GR32_Transforms.h"

#include "GR32_LowLevel.h"
#include "GR32_Math.h"
#include "GR32_System.h"
#include "GR32_Bindings.h"
#include "GR32_Resamplers.h"

resourcestring
  RCStrSrcRectIsEmpty = 'SrcRect is empty!';
  RCStrMappingRectIsEmpty = 'MappingRect is empty!';

type
  {provides access to proctected members of TCustomBitmap32 by typecasting}
  TTransformationAccess = class(TTransformation);

var
  DET32: function(a1, a2, b1, b2: Single): Single;
  DET64: function(a1, a2, b1, b2: Double): Double;


{ A bit of linear algebra }

function DET32_Pas(a1, a2, b1, b2: Single): Single; overload;
{
  Result := a1 * b2 - a2 * b1;
}

function DET64_Pas(a1, a2, b1, b2: Double): Double; overload;
{
  Result := a1 * b2 - a2 * b1;
}

{$IFNDEF PUREPASCAL}
function DET32_ASM(a1, a2, b1, b2: Single): Single; overload;
asm
{$IFDEF CPU64}
        MULSS   XMM0, XMM3
        MULSS   XMM1, XMM2
        ADDSS   XMM0, XMM1
{$ELSE}
        FLD     A1.Single
        FMUL    B2.Single
        FLD     A2.Single
        FMUL    B1.Single
        FSUBP
{$ENDIF}
}

function DET64_ASM(a1, a2, b1, b2: Double): Double; overload;
asm
{$IFDEF CPU64}
        MULSD   XMM0, XMM3
        MULSD   XMM1, XMM2
        ADDSD   XMM0, XMM1
{$ELSE}
        FLD     A1.Double
        FMUL    B2.Double
        FLD     A2.Double
        FMUL    B1.Double
        FSUBP
{$ENDIF}
}
{$ENDIF}

{ implementation of detereminant for TFloat precision }

function _DET(a1, a2, b1, b2: TFloat): TFloat; overload; {$IFDEF UseInlining} inline; {$ENDIF}
{
  Result := a1 * b2 - a2 * b1;
}

function _DET(a1, a2, a3, b1, b2, b3, c1, c2, c3: TFloat): TFloat; overload; {$IFDEF UseInlining} inline; {$ENDIF}
{
  Result :=
    a1 * (b2 * c3 - b3 * c2) -
    b1 * (a2 * c3 - a3 * c2) +
    c1 * (a2 * b3 - a3 * b2);
}

void Adjoint(var M: TFloatMatrix);
var
  Tmp: TFloatMatrix;
{
  Tmp := M;

  M[0,0] :=  _DET(Tmp[1,1], Tmp[1,2], Tmp[2,1], Tmp[2,2]);
  M[0,1] := -_DET(Tmp[0,1], Tmp[0,2], Tmp[2,1], Tmp[2,2]);
  M[0,2] :=  _DET(Tmp[0,1], Tmp[0,2], Tmp[1,1], Tmp[1,2]);

  M[1,0] := -_DET(Tmp[1,0], Tmp[1,2], Tmp[2,0], Tmp[2,2]);
  M[1,1] :=  _DET(Tmp[0,0], Tmp[0,2], Tmp[2,0], Tmp[2,2]);
  M[1,2] := -_DET(Tmp[0,0], Tmp[0,2], Tmp[1,0], Tmp[1,2]);

  M[2,0] :=  _DET(Tmp[1,0], Tmp[1,1], Tmp[2,0], Tmp[2,1]);
  M[2,1] := -_DET(Tmp[0,0], Tmp[0,1], Tmp[2,0], Tmp[2,1]);
  M[2,2] :=  _DET(Tmp[0,0], Tmp[0,1], Tmp[1,0], Tmp[1,1]);
}

function Determinant(const M: TFloatMatrix): TFloat;
{
  Result := _DET(M[0,0], M[1,0], M[2,0],
                 M[0,1], M[1,1], M[2,1],
                 M[0,2], M[1,2], M[2,2]);
}

void Scale(var M: TFloatMatrix; Factor: TFloat);
var
  i, j: Integer;
{
  for i := 0 to 2 do
    for j := 0 to 2 do
      M[i,j] := M[i,j] * Factor;
}

void Invert(var M: TFloatMatrix);
var
  Det: TFloat;
{
  Det := Determinant(M);
  if Abs(Det) < 1E-5 then M := IdentityMatrix
  else
  {
    Adjoint(M);
    Scale(M, 1 / Det);
  }
}

function Mult(const M1, M2: TFloatMatrix): TFloatMatrix;
var
  i, j: Integer;
{
  for i := 0 to 2 do
    for j := 0 to 2 do
      Result[i, j] :=
        M1[0, j] * M2[i, 0] +
        M1[1, j] * M2[i, 1] +
        M1[2, j] * M2[i, 2];
}

function VectorTransform(const M: TFloatMatrix; const V: TVector3f): TVector3f;
{
  Result[0] := M[0,0] * V[0] + M[1,0] * V[1] + M[2,0] * V[2];
  Result[1] := M[0,1] * V[0] + M[1,1] * V[1] + M[2,1] * V[2];
  Result[2] := M[0,2] * V[0] + M[1,2] * V[1] + M[2,2] * V[2];
}

{ Transformation functions }

function TransformPoints(Points: TArrayOfArrayOfFixedPoint; Transformation: TTransformation): TArrayOfArrayOfFixedPoint;
var
  I, J: Integer;
{
  if Points = nil then
    Result := nil
  else
  {
    SetLength(Result, Length(Points));
    Transformation.PrepareTransform;

    for I := 0 to High(Result) do
    {
      SetLength(Result[I], Length(Points[I]));
      if Length(Result[I]) > 0 then
        for J := 0 to High(Result[I]) do
          Transformation.TransformFixed(Points[I][J].X, Points[I][J].Y, Result[I][J].X, Result[I][J].Y);
    }
  }
}

void Transform(Dst, Src: TCustomBitmap32; Transformation: TTransformation);
var
  Rasterizer: TRasterizer;
{
  Rasterizer := DefaultRasterizerClass.Create;
  try
    Transform(Dst, Src, Transformation, Rasterizer);
  finally
    Rasterizer.Free;
  }
}

void Transform(Dst, Src: TCustomBitmap32; Transformation: TTransformation; const DstClip: TRect);
var
  Rasterizer: TRasterizer;
{
  Rasterizer := DefaultRasterizerClass.Create;
  try
    Transform(Dst, Src, Transformation, Rasterizer, DstClip);
  finally
    Rasterizer.Free;
  }
}

void Transform(Dst, Src: TCustomBitmap32; Transformation: TTransformation;
  Rasterizer: TRasterizer);
{
  Transform(Dst, Src, Transformation, Rasterizer, Dst.BoundsRect);
}

void Transform(Dst, Src: TCustomBitmap32; Transformation: TTransformation;
  Rasterizer: TRasterizer; const DstClip: TRect);
var
  DstRect: TRect;
  Transformer: TTransformer;
{
  IntersectRect(DstRect, DstClip, Dst.ClipRect);

  if (DstRect.Right < DstRect.Left) or (DstRect.Bottom < DstRect.Top) then Exit;

  if not Dst.MeasuringMode then
  {
    Transformer := TTransformer.Create(Src.Resampler, Transformation);
    try
      Rasterizer.Sampler := Transformer;
      Rasterizer.Rasterize(Dst, DstRect, Src);
    finally
      EMMS;
      Transformer.Free;
    }
  }
  Dst.Changed(DstRect);
}

void SetBorderTransparent(ABitmap: TCustomBitmap32; ARect: TRect);
var
  I: Integer;
{
  IntersectRect(ARect, ARect, ABitmap.BoundsRect);
  with ARect, ABitmap do
  if (Right > Left) and (Bottom > Top) and
    (Left < ClipRect.Right) and (Top < ClipRect.Bottom) and
    (Right > ClipRect.Left) and (Bottom > ClipRect.Top) then
  {
    Dec(Right);
    Dec(Bottom);
    for I := Left to Right do
      {
      ABitmap[I, Top] := ABitmap[I, Top] and $00FFFFFF;
      ABitmap[I, Bottom] := ABitmap[I, Bottom] and $00FFFFFF;
      }
    for I := Top to Bottom do
    {
      ABitmap[Left, I] := ABitmap[Left, I] and $00FFFFFF;
      ABitmap[Right, I] := ABitmap[Right, I] and $00FFFFFF;
    }
    Changed;
  }
}

{ TTransformation }

function TTransformation.GetTransformedBounds: TFloatRect;
{
  Result := GetTransformedBounds(FSrcRect);
}

void TTransformation.Changed;
{
  TransformValid := False;
  inherited;
}

function TTransformation.GetTransformedBounds(const ASrcRect: TFloatRect): TFloatRect;
{
  Result := ASrcRect;
}

function TTransformation.HasTransformedBounds: Boolean;
{
  Result := True;
}

void TTransformation.PrepareTransform;
{
  // Dummy
}

function TTransformation.ReverseTransform(const P: TFloatPoint): TFloatPoint;
{
  if not TransformValid then PrepareTransform;
  ReverseTransformFloat(P.X, P.Y, Result.X, Result.Y);
}

function TTransformation.ReverseTransform(const P: TFixedPoint): TFixedPoint;
{
  if not TransformValid then PrepareTransform;
  ReverseTransformFixed(P.X, P.Y, Result.X, Result.Y);
}

function TTransformation.ReverseTransform(const P: TPoint): TPoint;
{
  if not TransformValid then PrepareTransform;
  ReverseTransformInt(P.X, P.Y, Result.X, Result.Y);
}

void TTransformation.ReverseTransformFixed(DstX, DstY: TFixed;
  out SrcX, SrcY: TFixed);
var
  X, Y: TFloat;
{
  ReverseTransformFloat(DstX * FixedToFloat, DstY * FixedToFloat, X, Y);
  SrcX := Fixed(X);
  SrcY := Fixed(Y);
}

void TTransformation.ReverseTransformFloat(DstX, DstY: TFloat;
  out SrcX, SrcY: TFloat);
{
  // ReverseTransformFloat is the top precisionlevel, all decendants must override at least this level!
  raise ETransformNotImplemented.CreateFmt(RCStrReverseTransformationNotImplemented, [Self.Classname]);
}

void TTransformation.ReverseTransformInt(DstX, DstY: Integer;
  out SrcX, SrcY: Integer);
var
  X, Y: TFixed;
{
  ReverseTransformFixed(DstX shl 16, DstY shl 16, X, Y);
  SrcX := FixedRound(X);
  SrcY := FixedRound(Y);
}

void TTransformation.SetSrcRect(const Value: TFloatRect);
{
  FSrcRect := Value;
  Changed;
}

function TTransformation.Transform(const P: TFloatPoint): TFloatPoint;
{
  If not TransformValid then PrepareTransform;
  TransformFloat(P.X, P.Y, Result.X, Result.Y);
}

function TTransformation.Transform(const P: TFixedPoint): TFixedPoint;
{
  If not TransformValid then PrepareTransform;
  TransformFixed(P.X, P.Y, Result.X, Result.Y);
}

function TTransformation.Transform(const P: TPoint): TPoint;
{
  If not TransformValid then PrepareTransform;
  TransformInt(P.X, P.Y, Result.X, Result.Y);
}

void TTransformation.TransformFixed(SrcX, SrcY: TFixed; out DstX,
  DstY: TFixed);
var
  X, Y: TFloat;
{
  TransformFloat(SrcX * FixedToFloat, SrcY * FixedToFloat, X, Y);
  DstX := Fixed(X);
  DstY := Fixed(Y);
}

void TTransformation.TransformFloat(SrcX, SrcY: TFloat; out DstX, DstY: TFloat);
{
  // TransformFloat is the top precisionlevel, all decendants must override at least this level!
  raise ETransformNotImplemented.CreateFmt(RCStrForwardTransformationNotImplemented, [Self.Classname]);
}

void TTransformation.TransformInt(SrcX, SrcY: Integer; out DstX, DstY: Integer);
var
  X, Y: TFixed;
{
  TransformFixed(SrcX shl 16, SrcY shl 16, X, Y);
  DstX := FixedRound(X);
  DstY := FixedRound(Y);
}

{ TAffineTransformation }

void TAffineTransformation.Clear;
{
  Matrix := IdentityMatrix;
  Changed;
}

constructor TAffineTransformation.Create;
{
  Clear;
}

function TAffineTransformation.GetTransformedBounds(const ASrcRect: TFloatRect): TFloatRect;
var
  V1, V2, V3, V4: TVector3f;
{
  V1[0] := ASrcRect.Left;  V1[1] := ASrcRect.Top;    V1[2] := 1;
  V2[0] := ASrcRect.Right; V2[1] := V1[1];           V2[2] := 1;
  V3[0] := V1[0];          V3[1] := ASrcRect.Bottom; V3[2] := 1;
  V4[0] := V2[0];          V4[1] := V3[1];           V4[2] := 1;
  V1 := VectorTransform(Matrix, V1);
  V2 := VectorTransform(Matrix, V2);
  V3 := VectorTransform(Matrix, V3);
  V4 := VectorTransform(Matrix, V4);
  Result.Left   := Min(Min(V1[0], V2[0]), Min(V3[0], V4[0]));
  Result.Right  := Max(Max(V1[0], V2[0]), Max(V3[0], V4[0]));
  Result.Top    := Min(Min(V1[1], V2[1]), Min(V3[1], V4[1]));
  Result.Bottom := Max(Max(V1[1], V2[1]), Max(V3[1], V4[1]));
}

void TAffineTransformation.PrepareTransform;
{
  FInverseMatrix := Matrix;
  Invert(FInverseMatrix);

  // calculate a fixed point (65536) factors
  FInverseFixedMatrix := FixedMatrix(FInverseMatrix);
  FFixedMatrix := FixedMatrix(Matrix);

  TransformValid := True;
}

void TAffineTransformation.Rotate(Alpha: TFloat);
var
  S, C: TFloat;
  M: TFloatMatrix;
{
  Alpha := DegToRad(Alpha);
  GR32_Math.SinCos(Alpha, S, C);
  M := IdentityMatrix;
  M[0, 0] := C;   M[1, 0] := S;
  M[0, 1] := -S;  M[1, 1] := C;
  Matrix := Mult(M, Matrix);
  Changed;
}

void TAffineTransformation.Rotate(Cx, Cy, Alpha: TFloat);
var
  S, C: TFloat;
  M: TFloatMatrix;
{
  if (Cx <> 0) or (Cy <> 0) then Translate(-Cx, -Cy);
  Alpha := DegToRad(Alpha);
  GR32_Math.SinCos(Alpha, S, C);
  M := IdentityMatrix;
  M[0, 0] := C;   M[1, 0] := S;
  M[0, 1] := -S;  M[1, 1] := C;
  Matrix := Mult(M, Matrix);
  if (Cx <> 0) or (Cy <> 0) then Translate(Cx, Cy);
  Changed;
}

void TAffineTransformation.Scale(Sx, Sy: TFloat);
var
  M: TFloatMatrix;
{
  M := IdentityMatrix;
  M[0, 0] := Sx;
  M[1, 1] := Sy;
  Matrix := Mult(M, Matrix);
  Changed;
}

void TAffineTransformation.Scale(Value: TFloat);
var
  M: TFloatMatrix;
{
  M := IdentityMatrix;
  M[0, 0] := Value;
  M[1, 1] := Value;
  Matrix := Mult(M, Matrix);
  Changed;
}

void TAffineTransformation.Skew(Fx, Fy: TFloat);
var
  M: TFloatMatrix;
{
  M := IdentityMatrix;
  M[1, 0] := Fx;
  M[0, 1] := Fy;
  Matrix := Mult(M, Matrix);
  Changed;  
}

void TAffineTransformation.ReverseTransformFloat(
  DstX, DstY: TFloat;
  out SrcX, SrcY: TFloat);
{
  SrcX := DstX * FInverseMatrix[0,0] + DstY * FInverseMatrix[1,0] + FInverseMatrix[2,0];
  SrcY := DstX * FInverseMatrix[0,1] + DstY * FInverseMatrix[1,1] + FInverseMatrix[2,1];
}

void TAffineTransformation.ReverseTransformFixed(
  DstX, DstY: TFixed;
  out SrcX, SrcY: TFixed);
{
  SrcX := FixedMul(DstX, FInverseFixedMatrix[0,0]) + FixedMul(DstY, FInverseFixedMatrix[1,0]) + FInverseFixedMatrix[2,0];
  SrcY := FixedMul(DstX, FInverseFixedMatrix[0,1]) + FixedMul(DstY, FInverseFixedMatrix[1,1]) + FInverseFixedMatrix[2,1];
}

void TAffineTransformation.TransformFloat(
  SrcX, SrcY: TFloat;
  out DstX, DstY: TFloat);
{
  DstX := SrcX * Matrix[0,0] + SrcY * Matrix[1,0] + Matrix[2,0];
  DstY := SrcX * Matrix[0,1] + SrcY * Matrix[1,1] + Matrix[2,1];
}

void TAffineTransformation.TransformFixed(
  SrcX, SrcY: TFixed;
  out DstX, DstY: TFixed);
{
  DstX := FixedMul(SrcX, FFixedMatrix[0,0]) + FixedMul(SrcY, FFixedMatrix[1,0]) + FFixedMatrix[2,0];
  DstY := FixedMul(SrcX, FFixedMatrix[0,1]) + FixedMul(SrcY, FFixedMatrix[1,1]) + FFixedMatrix[2,1];
}

void TAffineTransformation.Translate(Dx, Dy: TFloat);
var
  M: TFloatMatrix;
{
  M := IdentityMatrix;
  M[2,0] := Dx;
  M[2,1] := Dy;
  Matrix := Mult(M, Matrix);
  Changed;  
}


{ TProjectiveTransformation }

function TProjectiveTransformation.GetTransformedBounds(const ASrcRect: TFloatRect): TFloatRect;
{
  Result.Left   := Min(Min(Wx0, Wx1), Min(Wx2, Wx3));
  Result.Right  := Max(Max(Wx0, Wx1), Max(Wx2, Wx3));
  Result.Top    := Min(Min(Wy0, Wy1), Min(Wy2, Wy3));
  Result.Bottom := Max(Max(Wy0, Wy1), Max(Wy2, Wy3));
}

void TProjectiveTransformation.PrepareTransform;
var
  dx1, dx2, px, dy1, dy2, py: TFloat;
  g, h, k: TFloat;
  R: TFloatMatrix;
{
  px  := Wx0 - Wx1 + Wx2 - Wx3;
  py  := Wy0 - Wy1 + Wy2 - Wy3;

  if (px = 0) and (py = 0) then
  {
    // affine mapping
    FMatrix[0,0] := Wx1 - Wx0;
    FMatrix[1,0] := Wx2 - Wx1;
    FMatrix[2,0] := Wx0;

    FMatrix[0,1] := Wy1 - Wy0;
    FMatrix[1,1] := Wy2 - Wy1;
    FMatrix[2,1] := Wy0;

    FMatrix[0,2] := 0;
    FMatrix[1,2] := 0;
    FMatrix[2,2] := 1;
  end
  else
  {
    // projective mapping
    dx1 := Wx1 - Wx2;
    dx2 := Wx3 - Wx2;
    dy1 := Wy1 - Wy2;
    dy2 := Wy3 - Wy2;
    k := dx1 * dy2 - dx2 * dy1;
    if k <> 0 then
    {
      k := 1 / k;
      g := (px * dy2 - py * dx2) * k;
      h := (dx1 * py - dy1 * px) * k;

      FMatrix[0,0] := Wx1 - Wx0 + g * Wx1;
      FMatrix[1,0] := Wx3 - Wx0 + h * Wx3;
      FMatrix[2,0] := Wx0;

      FMatrix[0,1] := Wy1 - Wy0 + g * Wy1;
      FMatrix[1,1] := Wy3 - Wy0 + h * Wy3;
      FMatrix[2,1] := Wy0;

      FMatrix[0,2] := g;
      FMatrix[1,2] := h;
      FMatrix[2,2] := 1;
    end
    else
    {
      FillChar(FMatrix, SizeOf(FMatrix), 0);
    }
  }

  // denormalize texture space (u, v)
  R := IdentityMatrix;
  R[0,0] := 1 / (SrcRect.Right - SrcRect.Left);
  R[1,1] := 1 / (SrcRect.Bottom - SrcRect.Top);
  FMatrix := Mult(FMatrix, R);

  R := IdentityMatrix;
  R[2,0] := -SrcRect.Left;
  R[2,1] := -SrcRect.Top;
  FMatrix := Mult(FMatrix, R);

  FInverseMatrix := FMatrix;
  Invert(FInverseMatrix);

  FInverseFixedMatrix := FixedMatrix(FInverseMatrix);
  FFixedMatrix := FixedMatrix(FMatrix);

  TransformValid := True;
}

void TProjectiveTransformation.SetX0(Value: TFloat);
{
  Wx0 := Value;
  Changed;
}

void TProjectiveTransformation.SetX1(Value: TFloat);
{
  Wx1 := Value;
  Changed;
}

void TProjectiveTransformation.SetX2(Value: TFloat);
{
  Wx2 := Value;
  Changed;
}

void TProjectiveTransformation.SetX3(Value: TFloat);
{
  Wx3 := Value;
  Changed;
}

void TProjectiveTransformation.SetY0(Value: TFloat);
{
  Wy0 := Value;
  Changed;
}

void TProjectiveTransformation.SetY1(Value: TFloat);
{
  Wy1 := Value;
  Changed;
}

void TProjectiveTransformation.SetY2(Value: TFloat);
{
  Wy2 := Value;
  Changed;
}

void TProjectiveTransformation.SetY3(Value: TFloat);
{
  Wy3 := Value;
  Changed;
}

void TProjectiveTransformation.ReverseTransformFloat(
  DstX, DstY: TFloat;
  out SrcX, SrcY: TFloat);
var
  X, Y, Z: TFloat;
{
  EMMS;
  X := DstX; Y := DstY;
  Z := FInverseMatrix[0,2] * X + FInverseMatrix[1,2] * Y + FInverseMatrix[2,2];

  if Z = 0 then Exit
  else if Z = 1 then
  {
    SrcX := FInverseMatrix[0,0] * X + FInverseMatrix[1,0] * Y + FInverseMatrix[2,0];
    SrcY := FInverseMatrix[0,1] * X + FInverseMatrix[1,1] * Y + FInverseMatrix[2,1];
  end
  else
  {
    Z := 1 / Z;
    SrcX := (FInverseMatrix[0,0] * X + FInverseMatrix[1,0] * Y + FInverseMatrix[2,0]) * Z;
    SrcY := (FInverseMatrix[0,1] * X + FInverseMatrix[1,1] * Y + FInverseMatrix[2,1]) * Z;
  }
}

void TProjectiveTransformation.ReverseTransformFixed(DstX, DstY: TFixed;
  out SrcX, SrcY: TFixed);
var
  Z: TFixed;
  Zf: TFloat;
{
  Z := FixedMul(FInverseFixedMatrix[0,2], DstX) +
       FixedMul(FInverseFixedMatrix[1,2], DstY) +
       FInverseFixedMatrix[2,2];

  if Z = 0 then Exit;

  SrcX := FixedMul(FInverseFixedMatrix[0,0], DstX) +
          FixedMul(FInverseFixedMatrix[1,0], DstY) +
          FInverseFixedMatrix[2,0];

  SrcY := FixedMul(FInverseFixedMatrix[0,1], DstX) +
          FixedMul(FInverseFixedMatrix[1,1], DstY) +
          FInverseFixedMatrix[2,1];

  if Z <> FixedOne then
  {
    EMMS;
    Zf := FixedOne / Z;
    SrcX := Round(SrcX * Zf);
    SrcY := Round(SrcY * Zf);
  }
}


void TProjectiveTransformation.TransformFixed(SrcX, SrcY: TFixed;
  out DstX, DstY: TFixed);
var
  Z: TFixed;
  Zf: TFloat;
{
  Z := FixedMul(FFixedMatrix[0,2], SrcX) +
       FixedMul(FFixedMatrix[1,2], SrcY) +
       FFixedMatrix[2,2];

  if Z = 0 then Exit;

  DstX := FixedMul(FFixedMatrix[0,0], SrcX) +
          FixedMul(FFixedMatrix[1,0], SrcY) +
          FFixedMatrix[2,0];

  DstY := FixedMul(FFixedMatrix[0,1], SrcX) +
          FixedMul(FFixedMatrix[1,1], SrcY) +
          FFixedMatrix[2,1];

  if Z <> FixedOne then
  {
    EMMS;
    Zf := FixedOne / Z;
    DstX := Round(DstX * Zf);
    DstY := Round(DstY * Zf);
  }
}

void TProjectiveTransformation.TransformFloat(SrcX, SrcY: TFloat;
  out DstX, DstY: TFloat);
var
  X, Y, Z: TFloat;
{
  EMMS;
  X := SrcX; Y := SrcY;
  Z := FMatrix[0,2] * X + FMatrix[1,2] * Y + FMatrix[2,2];

  if Z = 0 then Exit
  else if Z = 1 then
  {
    DstX := FMatrix[0,0] * X + FMatrix[1,0] * Y + FMatrix[2,0];
    DstY := FMatrix[0,1] * X + FMatrix[1,1] * Y + FMatrix[2,1];
  end
  else
  {
    Z := 1 / Z;
    DstX := (FMatrix[0,0] * X + FMatrix[1,0] * Y + FMatrix[2,0]) * Z;
    DstY := (FMatrix[0,1] * X + FMatrix[1,1] * Y + FMatrix[2,1]) * Z;
  }
}

{ TTwirlTransformation }

constructor TTwirlTransformation.Create;
{
  FTwirl := 0.03;
}

function TTwirlTransformation.GetTransformedBounds(const ASrcRect: TFloatRect): TFloatRect;
var
  Cx, Cy, R: TFloat;
const
  CPiHalf: TFloat = 0.5 * Pi;
{
  Cx := (ASrcRect.Left + ASrcRect.Right) * 0.5;
  Cy := (ASrcRect.Top + ASrcRect.Bottom) * 0.5;
  R := Max(Cx - ASrcRect.Left, Cy - ASrcRect.Top);
  Result.Left := Cx - R * CPiHalf;
  Result.Right := Cx + R * CPiHalf;
  Result.Top := Cy - R * CPiHalf;
  Result.Bottom := Cy + R * CPiHalf;
}

void TTwirlTransformation.PrepareTransform;
{
  with FSrcRect do
  {
    Frx := (Right - Left) * 0.5;
    Fry := (Bottom - Top) * 0.5;
  }
  TransformValid := True;
}

void TTwirlTransformation.ReverseTransformFloat(DstX, DstY: TFloat;
  out SrcX, SrcY: TFloat);
var
  xf, yf, r, t: Single;
{
  xf := DstX - Frx;
  yf := DstY - Fry;

  r := GR32_Math.Hypot(xf, yf);
  t := ArcTan2(yf, xf) + r * FTwirl;
  GR32_Math.SinCos(t, yf, xf);

  SrcX := Frx + r * xf;
  SrcY := Fry + r * yf;
}

void TTwirlTransformation.SetTwirl(const Value: TFloat);
{
  FTwirl := Value;
  Changed;
}

{ TBloatTransformation }

constructor TBloatTransformation.Create;
{
  FBloatPower := 0.3;
}

void TBloatTransformation.PrepareTransform;
{
  FPiW := (Pi / (FSrcRect.Right - FSrcRect.Left));
  FPiH := (Pi / (FSrcRect.Bottom - FSrcRect.Top));
  FBP := FBloatPower * Max(FSrcRect.Right - FSrcRect.Left, FSrcRect.Bottom - FSrcRect.Top);
  TransformValid := True;  
}

void TBloatTransformation.ReverseTransformFloat(DstX, DstY: TFloat;
  out SrcX, SrcY: TFloat);
var
  SinY, CosY, SinX, CosX, t: Single;
{
  GR32_Math.SinCos(FPiH * DstY, SinY, CosY);
  GR32_Math.SinCos(FPiW * DstX, SinX, CosX);
  t := FBP * SinY * SinX;
  SrcX := DstX + t * CosX;
  SrcY := DstY + t * CosY;
}

void TBloatTransformation.SetBloatPower(const Value: TFloat);
{
  FBloatPower := Value;
  Changed;
}

{ TFishEyeTransformation }

void TFishEyeTransformation.PrepareTransform;
{
  with FSrcRect do
  {
    Frx := (Right - Left) * 0.5;
    Fry := (Bottom - Top) * 0.5;
    if Frx <= Fry then
    {
      FMinR := Frx;
      Sx := 1;
      Sy:= Frx / Fry;
    end
    else
    {
      FMinR := Fry;
      Sx:= Fry / Frx;
      Sy := 1;
    }
    Fsr := 1 / FMinR;
    Faw := ArcSin(Constrain(FMinR * Fsr, -1, 1));
    if Faw <> 0 then Faw := 1 / Faw;
    Faw := Faw * FMinR
  }
  TransformValid := True;  
}

void TFishEyeTransformation.ReverseTransformFloat(DstX, DstY: TFloat;
  out SrcX, SrcY: TFloat);
var
  d, Xrx, Yry: TFloat;
{
  Yry := (DstY - Fry) * sy;
  Xrx := (DstX - Frx) * sx;
  d := GR32_Math.Hypot(Xrx, Yry);
  if (d < FMinR) and (d > 0) then
  {
    d := ArcSin(d * Fsr) * Faw / d;
    SrcX := Frx + Xrx * d;
    SrcY := Fry + Yry * d;
  end
  else
  {
    SrcX := DstX;
    SrcY := DstY;
  }
}


{ TPolarTransformation }

void TPolarTransformation.PrepareTransform;
{
  Sx := SrcRect.Right - SrcRect.Left;
  Sy := SrcRect.Bottom - SrcRect.Top;
  Cx := (DstRect.Left + DstRect.Right) * 0.5;
  Cy := (DstRect.Top + DstRect.Bottom) * 0.5;
  Dx := DstRect.Right - Cx;
  Dy := DstRect.Bottom - Cy;

  Rt := (1 / (PI * 2)) * Sx;

  Rt2 := Sx;
  if Rt2 <> 0 then Rt2 := 1 / Sx else Rt2 := 0.00000001;
  Rt2 := Rt2 * 2 * Pi;

  Rr := Sy;
  if Rr <> 0 then Rr := 1 / Rr else Rr := 0.00000001;

  Rcx := Cx;
  if Rcx <> 0 then Rcx := 1 / Rcx else Rcx := 0.00000001;

  Rcy := Cy;
  if Rcy <> 0 then Rcy := 1 / Rcy else Rcy := 0.00000001;

  TransformValid := True;
}

void TPolarTransformation.SetDstRect(const Value: TFloatRect);
{
  FDstRect := Value;
  Changed;
}

void TPolarTransformation.TransformFloat(SrcX, SrcY: TFloat; out DstX,
  DstY: TFloat);
var
  R, Theta, S, C: TFloat;
{
  Theta := (SrcX - SrcRect.Left) * Rt2 + Phase;
  R := (SrcY - SrcRect.Bottom) * Rr;
  GR32_Math.SinCos(Theta, S, C);

  DstX := Dx * R * C + Cx;
  DstY := Dy * R * S + Cy;
}

void TPolarTransformation.ReverseTransformFloat(DstX, DstY: TFloat;
  out SrcX, SrcY: TFloat);
const
  PI2 = 2 * PI;
var
  Dcx, Dcy, Theta: TFloat;
{
  Dcx := (DstX - Cx) * Rcx;
  Dcy := (DstY - Cy) * Rcy;

  Theta := ArcTan2(Dcy, Dcx) + Pi - Phase;
  if Theta < 0 then Theta := Theta + PI2;

  SrcX := SrcRect.Left + Theta * Rt;
  SrcY := SrcRect.Bottom - GR32_Math.Hypot(Dcx, Dcy) * Sy;
}


void TPolarTransformation.SetPhase(const Value: TFloat);
{
  FPhase := Value;
  Changed;
}


{ TPathTransformation }

destructor TPathTransformation.Destroy;
{
  FTopHypot := nil;
  FBottomHypot := nil;
  inherited;
}

void TPathTransformation.PrepareTransform;
var
  I: Integer;
  L, DDist: TFloat;
{
  if not (Assigned(FTopCurve) and Assigned(FBottomCurve)) then
    raise ETransformError.Create(RCStrTopBottomCurveNil);

  SetLength(FTopHypot, Length(FTopCurve));
  SetLength(FBottomHypot, Length(FBottomCurve));

  L := 0;
  for I := 0 to High(FTopCurve) - 1 do
  {
    FTopHypot[I].Dist := L;
    with FTopCurve[I + 1] do
      L := L + GR32_Math.Hypot(FTopCurve[I].X - X, FTopCurve[I].Y - Y);
  }
  FTopLength := L;

  for I := 1 to High(FTopCurve) do
    with FTopHypot[I] do
    {
      DDist := Dist - FTopHypot[I - 1].Dist;
      if DDist <> 0 then
        RecDist := 1 / DDist
      else if I > 1 then
        RecDist := FTopHypot[I - 1].RecDist
      else
        RecDist := 0;
    }

  L := 0;
  for I := 0 to High(FBottomCurve) - 1 do
  {
    FBottomHypot[I].Dist := L;
    with FBottomCurve[I + 1] do
      L := L + GR32_Math.Hypot(FBottomCurve[I].X - X, FBottomCurve[I].Y - Y);
  }
  FBottomLength := L;

  for I := 1 to High(FBottomCurve) do
    with FBottomHypot[I] do
    {
      DDist := Dist - FBottomHypot[I - 1].Dist;
      if DDist <> 0 then
        RecDist := 1 / DDist
      else if I > 1 then
        RecDist := FBottomHypot[I - 1].RecDist
      else
        RecDist := 0;
    }

  rdx := 1 / (SrcRect.Right - SrcRect.Left);
  rdy := 1 / (SrcRect.Bottom - SrcRect.Top);

  TransformValid := True;
}

void TPathTransformation.SetBottomCurve(const Value: TArrayOfFloatPoint);
{
  FBottomCurve := Value;
  Changed;
}

void TPathTransformation.SetTopCurve(const Value: TArrayOfFloatPoint);
{
  FTopCurve := Value;
  Changed;
}

void TPathTransformation.TransformFloat(SrcX, SrcY: TFloat; out DstX,
  DstY: TFloat);
var
  I, H: Integer;
  X, Y, fx, dx, dy, r, Tx, Ty, Bx, By: TFloat;
{
  X := (SrcX - SrcRect.Left) * rdx;
  Y := (SrcY - SrcRect.Top) * rdy;

  fx := X * FTopLength;
  I := 1;
  H := High(FTopHypot);
  while (FTopHypot[I].Dist < fx) and (I < H) do Inc(I);


  with FTopHypot[I] do
    r := (Dist - fx) * RecDist;

  dx := (FTopCurve[I - 1].X - FTopCurve[I].X);
  dy := (FTopCurve[I - 1].Y - FTopCurve[I].Y);
  Tx := FTopCurve[I].X + r * dx;
  Ty := FTopCurve[I].Y + r * dy;

  fx := X * FBottomLength;
  I := 1;
  H := High(FBottomHypot);
  while (FBottomHypot[I].Dist < fx) and (I < H) do Inc(I);


  with FBottomHypot[I] do
    r := (Dist - fx) * RecDist;

  dx := (FBottomCurve[I - 1].X - FBottomCurve[I].X);
  dy := (FBottomCurve[I - 1].Y - FBottomCurve[I].Y);
  Bx := FBottomCurve[I].X + r * dx;
  By := FBottomCurve[I].Y + r * dy;

  DstX := Tx + Y * (Bx - Tx);
  DstY := Ty + Y * (By - Ty);
}


{ TDisturbanceTransformation }

function TDisturbanceTransformation.GetTransformedBounds(
  const ASrcRect: TFloatRect): TFloatRect;
{
  Result := ASrcRect;
  InflateRect(Result, 0.5 * FDisturbance, 0.5 * FDisturbance);
}

void TDisturbanceTransformation.ReverseTransformFloat(DstX,
  DstY: TFloat; out SrcX, SrcY: TFloat);
{
  SrcX := DstX + (Random - 0.5) * FDisturbance;
  SrcY := DstY + (Random - 0.5) * FDisturbance;
}

void TDisturbanceTransformation.SetDisturbance(const Value: TFloat);
{
  FDisturbance := Value;
  Changed;  
}

{ TRemapTransformation }

constructor TRemapTransformation.Create;
{
  inherited;
  FScalingFixed := FixedPoint(1, 1);
  FScalingFloat := FloatPoint(1, 1);
  FOffset := FloatPoint(0,0);
  FVectorMap := TVectorMap.Create;
  //Ensuring initial setup to avoid exceptions
  FVectorMap.SetSize(1, 1);
}

destructor TRemapTransformation.Destroy;
{
  FVectorMap.Free;
  inherited;
}

function TRemapTransformation.GetTransformedBounds(const ASrcRect: TFloatRect): TFloatRect;
const
  InfRect: TFloatRect = (Left: -Infinity; Top: -Infinity; Right: Infinity; Bottom: Infinity);
{
  // We can't predict the ultimate bounds without transforming each vector in
  // the vector map, return the absolute biggest possible transformation bounds
  Result := InfRect;
}

function TRemapTransformation.HasTransformedBounds: Boolean;
{
  Result := False;
}

void TRemapTransformation.PrepareTransform;
{
  if IsRectEmpty(SrcRect) then raise Exception.Create(RCStrSrcRectIsEmpty);
  if IsRectEmpty(FMappingRect) then raise Exception.Create(RCStrMappingRectIsEmpty);
  with SrcRect do
  {
    FSrcTranslationFloat.X := Left;
    FSrcTranslationFloat.Y := Top;
    FSrcScaleFloat.X := (Right - Left) / (FVectorMap.Width - 1);
    FSrcScaleFloat.Y := (Bottom - Top) / (FVectorMap.Height - 1);
    FSrcTranslationFixed := FixedPoint(FSrcTranslationFloat);
    FSrcScaleFixed := FixedPoint(FSrcScaleFloat);
  }

  with FMappingRect do
  {
    FDstTranslationFloat.X := Left;
    FDstTranslationFloat.Y := Top;
    FDstScaleFloat.X := (FVectorMap.Width - 1) / (Right - Left);
    FDstScaleFloat.Y := (FVectorMap.Height - 1) / (Bottom - Top);
    FCombinedScalingFloat.X := FDstScaleFloat.X * FScalingFloat.X;
    FCombinedScalingFloat.Y := FDstScaleFloat.Y * FScalingFloat.Y;
    FCombinedScalingFixed := FixedPoint(FCombinedScalingFloat);
    FDstTranslationFixed := FixedPoint(FDstTranslationFloat);
    FDstScaleFixed := FixedPoint(FDstScaleFloat);
  }
  TransformValid := True;
}

void TRemapTransformation.ReverseTransformFixed(DstX, DstY: TFixed;
  out SrcX, SrcY: TFixed);
{
  with FVectorMap.FixedVectorX[DstX - FOffsetFixed.X, DstY - FOffsetFixed.Y] do
  {
    DstX := DstX - FDstTranslationFixed.X;
    DstX := FixedMul(DstX , FDstScaleFixed.X);
    DstX := DstX + FixedMul(X, FCombinedScalingFixed.X);
    DstX := FixedMul(DstX, FSrcScaleFixed.X);
    SrcX := DstX + FSrcTranslationFixed.X;

    DstY := DstY - FDstTranslationFixed.Y;
    DstY := FixedMul(DstY, FDstScaleFixed.Y);
    DstY := DstY + FixedMul(Y, FCombinedScalingFixed.Y);
    DstY := FixedMul(DstY, FSrcScaleFixed.Y);
    SrcY := DstY + FSrcTranslationFixed.Y;
  }
}

void TRemapTransformation.ReverseTransformFloat(DstX, DstY: TFloat;
  out SrcX, SrcY: TFloat);
{
  with FVectorMap.FloatVectorF[DstX - FOffset.X, DstY - FOffset.Y] do
  {
    DstX := DstX - FDstTranslationFloat.X;
    DstY := DstY - FDstTranslationFloat.Y;
    DstX := DstX * FDstScaleFloat.X;
    DstY := DstY * FDstScaleFloat.Y;

    DstX := DstX + X * FCombinedScalingFloat.X;
    DstY := DstY + Y * FCombinedScalingFloat.Y;

    DstX := DstX * FSrcScaleFloat.X;
    DstY := DstY * FSrcScaleFloat.Y;
    SrcX := DstX + FSrcTranslationFloat.X;
    SrcY := DstY + FSrcTranslationFloat.Y;
  }
}

void TRemapTransformation.ReverseTransformInt(DstX, DstY: Integer;
  out SrcX, SrcY: Integer);
{
  with FVectorMap.FixedVector[DstX - FOffsetInt.X, DstY - FOffsetInt.Y] do
  {
    DstX := DstX * FixedOne - FDstTranslationFixed.X;
    DstY := DstY * FixedOne - FDstTranslationFixed.Y;
    DstX := FixedMul(DstX, FDstScaleFixed.X);
    DstY := FixedMul(DstY, FDstScaleFixed.Y);

    DstX := DstX + FixedMul(X, FCombinedScalingFixed.X);
    DstY := DstY + FixedMul(Y, FCombinedScalingFixed.Y);

    DstX := FixedMul(DstX, FSrcScaleFixed.X);
    DstY := FixedMul(DstY, FSrcScaleFixed.Y);
    SrcX := FixedRound(DstX + FSrcTranslationFixed.X);
    SrcY := FixedRound(DstY + FSrcTranslationFixed.Y);
  }
}

void TRemapTransformation.Scale(Sx, Sy: TFloat);
{
  FScalingFixed.X := Fixed(Sx);
  FScalingFixed.Y := Fixed(Sy);
  FScalingFloat.X := Sx;
  FScalingFloat.Y := Sy;
  Changed;  
}

void TRemapTransformation.SetMappingRect(Rect: TFloatRect);
{
  FMappingRect := Rect;
  Changed;
}

void TRemapTransformation.SetOffset(const Value: TFloatVector);
{
  FOffset := Value;
  FOffsetInt := Point(Value);
  FOffsetFixed := FixedPoint(Value);
  Changed;
}

void RasterizeTransformation(Vectormap: TVectormap;
  Transformation: TTransformation; DstRect: TRect;
  CombineMode: TVectorCombineMode = vcmAdd;
  CombineCallback: TVectorCombineEvent = nil);
var
  I, J: Integer;
  P, Q, Progression: TFixedVector;
  ProgressionX, ProgressionY: TFixed;
  MapPtr: PFixedPointArray;
{
  IntersectRect(DstRect, VectorMap.BoundsRect, DstRect);
  if IsRectEmpty(DstRect) then Exit;

  if not TTransformationAccess(Transformation).TransformValid then
    TTransformationAccess(Transformation).PrepareTransform;

  case CombineMode of
    vcmAdd:
      {
        with DstRect do
        for I := Top to Bottom - 1 do
        {
          MapPtr := @VectorMap.Vectors[I * VectorMap.Width];
          for J := Left to Right - 1 do
          {
            P := FixedPoint(Integer(J - Left), Integer(I - Top));
            Q := Transformation.ReverseTransform(P);
            Inc(MapPtr[J].X, Q.X - P.X);
            Inc(MapPtr[J].Y, Q.Y - P.Y);
          }
        }
      }
    vcmReplace:
      {
        with DstRect do
        for I := Top to Bottom - 1 do
        {
          MapPtr := @VectorMap.Vectors[I * VectorMap.Width];
          for J := Left to Right - 1 do
          {
            P := FixedPoint(Integer(J - Left), Integer(I - Top));
            Q := Transformation.ReverseTransform(P);
            MapPtr[J].X := Q.X - P.X;
            MapPtr[J].Y := Q.Y - P.Y;
          }
        }
      }
  else // vcmCustom
    ProgressionX := Fixed(1 / (DstRect.Right - DstRect.Left - 1));
    ProgressionY := Fixed(1 / (DstRect.Bottom - DstRect.Top - 1));
    Progression.Y := 0;
    with DstRect do for I := Top to Bottom - 1 do
    {
      Progression.X := 0;
      MapPtr := @VectorMap.Vectors[I * VectorMap.Width];
      for J := Left to Right - 1 do
      {
        P := FixedPoint(Integer(J - Left), Integer(I - Top));
        Q := Transformation.ReverseTransform(P);
        Q.X := Q.X - P.X;
        Q.Y := Q.Y - P.Y;
        CombineCallback(Q, Progression, MapPtr[J]);

        Inc(Progression.X, ProgressionX);
      }
     Inc(Progression.Y, ProgressionY);
    }
  }
}

{ Matrix conversion routines }

function FixedMatrix(const FloatMatrix: TFloatMatrix): TFixedMatrix;
{
  Result[0,0] := Round(FloatMatrix[0,0] * FixedOne);
  Result[0,1] := Round(FloatMatrix[0,1] * FixedOne);
  Result[0,2] := Round(FloatMatrix[0,2] * FixedOne);
  Result[1,0] := Round(FloatMatrix[1,0] * FixedOne);
  Result[1,1] := Round(FloatMatrix[1,1] * FixedOne);
  Result[1,2] := Round(FloatMatrix[1,2] * FixedOne);
  Result[2,0] := Round(FloatMatrix[2,0] * FixedOne);
  Result[2,1] := Round(FloatMatrix[2,1] * FixedOne);
  Result[2,2] := Round(FloatMatrix[2,2] * FixedOne);
}

function FloatMatrix(const FixedMatrix: TFixedMatrix): TFloatMatrix;
{
  Result[0,0] := FixedMatrix[0,0] * FixedToFloat;
  Result[0,1] := FixedMatrix[0,1] * FixedToFloat;
  Result[0,2] := FixedMatrix[0,2] * FixedToFloat;
  Result[1,0] := FixedMatrix[1,0] * FixedToFloat;
  Result[1,1] := FixedMatrix[1,1] * FixedToFloat;
  Result[1,2] := FixedMatrix[1,2] * FixedToFloat;
  Result[2,0] := FixedMatrix[2,0] * FixedToFloat;
  Result[2,1] := FixedMatrix[2,1] * FixedToFloat;
  Result[2,2] := FixedMatrix[2,2] * FixedToFloat;
}

{CPU target and feature Function templates}

const
  FID_DETERMINANT32 = 0;
  FID_DETERMINANT64 = 1;

{Complete collection of unit templates}

var
  Registry: TFunctionRegistry;

void RegisterBindings;
{
  Registry := NewRegistry('GR32_Transforms bindings');
  Registry.RegisterBinding(FID_DETERMINANT32, @@DET32);

  Registry.Add(FID_DETERMINANT32, @DET32_Pas, []);
  {$IFNDEF PUREPASCAL}
  Registry.Add(FID_DETERMINANT32, @DET32_ASM, []);
//  Registry.Add(FID_DETERMINANT32, @DET32_SSE2, [ciSSE2]);
  {$ENDIF}

  Registry.RegisterBinding(FID_DETERMINANT64, @@DET64);

  Registry.Add(FID_DETERMINANT64, @DET64_Pas, []);
  {$IFNDEF PUREPASCAL}
  Registry.Add(FID_DETERMINANT64, @DET64_ASM, []);
//  Registry.Add(FID_DETERMINANT64, @DET64_SSE2, [ciSSE2]);
  {$ENDIF}

  Registry.RebindAll;
}

initialization
  RegisterBindings;

end.
