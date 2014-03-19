//unit GR32_Math;
#pragma once

#include "GR32.h"

{ Fixed point math routines }
function FixedFloor(A: TFixed): Integer;
function FixedCeil(A: TFixed): Integer;
function FixedMul(A, B: TFixed): TFixed;
function FixedDiv(A, B: TFixed): TFixed;
function OneOver(Value: TFixed): TFixed;
function FixedRound(A: TFixed): Integer;
function FixedSqr(Value: TFixed): TFixed;
function FixedSqrtLP(Value: TFixed): TFixed;      // 8-bit precision
function FixedSqrtHP(Value: TFixed): TFixed;      // 16-bit precision
// Fixed point interpolation
function FixedCombine(W, X, Y: TFixed): TFixed;


{ Trigonometric routines }

procedure SinCos(const Theta: TFloat; out Sin, Cos: TFloat); overload;
procedure SinCos(const Theta, Radius: Single; out Sin, Cos: Single); overload;
function Hypot(const X, Y: TFloat): TFloat; overload;
function Hypot(const X, Y: Integer): Integer; overload;
function FastSqrt(const Value: TFloat): TFloat;
function FastSqrtBab1(const Value: TFloat): TFloat;
function FastSqrtBab2(const Value: TFloat): TFloat;
function FastInvSqrt(const Value: Single): Single; {$IFDEF INLININGSUPPORTED} inline; {$ENDIF} overload;


{ Misc. Routines }

{ MulDiv a faster implementation of Windows.MulDiv funtion }
function MulDiv(Multiplicand, Multiplier, Divisor: Integer): Integer;

// tells if X is a power of 2, returns true when X = 1,2,4,8,16 etc.
function IsPowerOf2(Value: Integer): Boolean; {$IFDEF INLININGSUPPORTED} inline; {$ENDIF}
// returns X rounded down to the nearest power of two
function PrevPowerOf2(Value: Integer): Integer;
// returns X rounded down to the nearest power of two, i.e. 5 -> 8, 7 -> 8, 15 -> 16
function NextPowerOf2(Value: Integer): Integer;

// fast average without overflow, useful for e.g. fixed point math
function Average(A, B: Integer): Integer;
// fast sign function
function Sign(Value: Integer): Integer;

function FloatMod(x, y: Double): Double; {$IFDEF INLININGSUPPORTED} inline; {$ENDIF}

