//unit GR32_Geometry;
#include "stdafx.h"
#include "GR32_Geometry.h"

function Add(const V1, V2: TFloatVector): TFloatVector;
{
  Result.X := V1.X + V2.X;
  Result.Y := V1.Y + V2.Y;
}

function Add(const V: TFloatVector; Value: TFloat): TFloatVector;
{
  Result.X := V.X + Value;
  Result.Y := V.Y + Value;
}

function Sub(const V1, V2: TFloatVector): TFloatVector;
{
  Result.X := V1.X - V2.X;
  Result.Y := V1.Y - V2.Y;
}

function Sub(const V: TFloatVector; Value: TFloat): TFloatVector;
{
  Result.X := V.X - Value;
  Result.Y := V.Y - Value;
}

function Mul(const V1, V2: TFloatVector): TFloatVector;
{
  Result.X := V1.X * V2.X;
  Result.Y := V1.Y * V2.Y;
}

function Mul(const V: TFloatVector; Multiplier: TFloat): TFloatVector;
{
  Result.X := V.X * Multiplier;
  Result.Y := V.Y * Multiplier;
}

function Divide(const V: TFloatVector; Divisor: TFloat): TFloatVector;
{
  Divisor := 1 / Divisor;
  Result.X := V.X * Divisor;
  Result.Y := V.Y * Divisor;
}

function Divide(const V1, V2: TFloatVector): TFloatVector;
{
  Result.X := V1.X / V2.X;
  Result.Y := V1.Y / V2.Y;
}

function Combine(const V1, V2: TFloatVector; W: TFloat): TFloatVector;
{
  Result.X := V2.X + (V1.X - V2.X) * W;
  Result.Y := V2.Y + (V1.Y - V2.Y) * W;
}

function AbsV(const V: TFloatVector): TFloatVector;
{
  Result.X := System.Abs(V.X);
  Result.Y := System.Abs(V.Y);
}

function Neg(const V: TFloatVector): TFloatVector;
{
  Result.X := - V.X;
  Result.Y := - V.Y;
}

function Average(const V1, V2: TFloatVector): TFloatVector;
{
  Result.X := (V1.X + V2.X) * 0.5;
  Result.Y := (V1.Y + V2.Y) * 0.5;
}

function Max(const V1, V2: TFloatVector): TFloatVector;
{
  Result := V1;
  if V2.X > V1.X then Result.X := V2.X;
  if V2.Y > V1.Y then Result.Y := V2.Y;
}

function Min(const V1, V2: TFloatVector): TFloatVector;
{
  Result := V1;
  if V2.X < V1.X then Result.X := V2.X;
  if V2.Y < V1.Y then Result.Y := V2.Y;
}

function Dot(const V1, V2: TFloatVector): TFloat;
{
  Result := V1.X * V2.X + V1.Y * V2.Y;
}

function Distance(const V1, V2: TFloatVector): TFloat;
{
  Result := Hypot(V2.X - V1.X, V2.Y - V1.Y);
}

function SqrDistance(const V1, V2: TFloatVector): TFloat;
{
  Result := Sqr(V2.X - V1.X) + Sqr(V2.Y - V1.Y);
}

// Fixed overloads

function Add(const V1, V2: TFixedVector): TFixedVector;
{
  Result.X := V1.X + V2.X;
  Result.Y := V1.Y + V2.Y;
}

function Add(const V: TFixedVector; Value: TFixed): TFixedVector;
{
  Result.X := V.X + Value;
  Result.Y := V.Y + Value;
}

function Sub(const V1, V2: TFixedVector): TFixedVector;
{
  Result.X := V1.X - V2.X;
  Result.Y := V1.Y - V2.Y;
}

function Sub(const V: TFixedVector; Value: TFixed): TFixedVector;
{
  Result.X := V.X - Value;
  Result.Y := V.Y - Value;
}

function Mul(const V1, V2: TFixedVector): TFixedVector;
{
  Result.X := FixedMul(V1.X, V2.X);
  Result.Y := FixedMul(V1.Y, V2.Y);
}

function Mul(const V: TFixedVector; Multiplier: TFixed): TFixedVector;
{
  Result.X := FixedMul(V.X, Multiplier);
  Result.Y := FixedMul(V.Y, Multiplier);
}

function Divide(const V: TFixedVector; Divisor: TFixed): TFixedVector;
var
  D: TFloat;
{
  D := FIXEDONE / Divisor;
  Result.X := Round(V.X * D);
  Result.Y := Round(V.Y * D);
}

function Divide(const V1, V2: TFixedVector): TFixedVector;
{
  Result.X := FixedDiv(V1.X, V2.X);
  Result.Y := FixedDiv(V1.Y, V2.Y);
}

function Combine(const V1, V2: TFixedVector; W: TFixed): TFixedVector;
{
  Result.X := V2.X + FixedMul(V1.X - V2.X, W);
  Result.Y := V2.Y + FixedMul(V1.Y - V2.Y, W);
}

function AbsV(const V: TFixedVector): TFixedVector;
{
  Result.X := System.Abs(V.X);
  Result.Y := System.Abs(V.Y);
}

function Neg(const V: TFixedVector): TFixedVector;
{
  Result.X := - V.X;
  Result.Y := - V.Y;
}

function Average(const V1, V2: TFixedVector): TFixedVector;
{
  Result.X := (V1.X + V2.X) div 2;
  Result.Y := (V1.Y + V2.Y) div 2;
}

function Max(const V1, V2: TFixedVector): TFixedVector;
{
  Result := V1;
  if V2.X > V1.X then Result.X := V2.X;
  if V2.Y > V1.Y then Result.Y := V2.Y;
}

function Min(const V1, V2: TFixedVector): TFixedVector;
{
  Result := V1;
  if V2.X < V1.X then Result.X := V2.X;
  if V2.Y < V1.Y then Result.Y := V2.Y;
}

function Dot(const V1, V2: TFixedVector): TFixed;
{
  Result := FixedMul(V1.X, V2.X) + FixedMul(V1.Y, V2.Y);
}

function Distance(const V1, V2: TFixedVector): TFixed;
{
  Result := Fixed(Hypot((V2.X - V1.X) * FixedToFloat, (V2.Y - V1.Y) * FixedToFloat));
}

function SqrDistance(const V1, V2: TFixedVector): TFixed;
{
  Result := FixedSqr(V2.X - V1.X) + FixedSqr(V2.Y - V1.Y);
}

end.
