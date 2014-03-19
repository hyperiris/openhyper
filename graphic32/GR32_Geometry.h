//unit GR32_Geometry;
#pragma once

#include "GR32.h"
#include "GR32_Math.h"

type
  PFixedVector = ^TFixedVector;
  TFixedVector = TFixedPoint;

  PFloatVector = ^TFloatVector;
  TFloatVector = TFloatPoint;

function Add(const V1, V2: TFloatVector): TFloatVector;  overload;
function Add(const V: TFloatVector; Value: TFloat): TFloatVector; overload;
function Sub(const V1, V2: TFloatVector): TFloatVector; overload;
function Sub(const V: TFloatVector; Value: TFloat): TFloatVector; overload;
function Mul(const V1, V2: TFloatVector): TFloatVector; overload;
function Mul(const V: TFloatVector; Multiplier: TFloat): TFloatVector; overload;
function Divide(const V: TFloatVector; Divisor: TFloat): TFloatVector; overload;
function Divide(const V1, V2: TFloatVector): TFloatVector; overload;

function Combine(const V1, V2: TFloatVector; W: TFloat): TFloatVector; overload;
function AbsV(const V: TFloatVector): TFloatVector; overload;
function Neg(const V: TFloatVector): TFloatVector; overload;
function Average(const V1, V2: TFloatVector): TFloatVector; overload;
function Max(const V1, V2: TFloatVector): TFloatVector; overload;
function Min(const V1, V2: TFloatVector): TFloatVector; overload;

function Dot(const V1, V2: TFloatVector): TFloat; overload;
function Distance(const V1, V2: TFloatVector): TFloat; overload;
function SqrDistance(const V1, V2: TFloatVector): TFloat; overload;

// Fixed Overloads
function Add(const V1, V2: TFixedVector): TFixedVector;  overload;
function Add(const V: TFixedVector; Value: TFixed): TFixedVector; overload;
function Sub(const V1, V2: TFixedVector): TFixedVector; overload;
function Sub(const V: TFixedVector; Value: TFixed): TFixedVector; overload;
function Mul(const V1, V2: TFixedVector): TFixedVector; overload;
function Mul(const V: TFixedVector; Multiplier: TFixed): TFixedVector; overload;
function Divide(const V: TFixedVector; Divisor: TFixed): TFixedVector; overload;
function Divide(const V1, V2: TFixedVector): TFixedVector; overload;

function Combine(const V1, V2: TFixedVector; W: TFixed): TFixedVector; overload;
function AbsV(const V: TFixedVector): TFixedVector; overload;
function Neg(const V: TFixedVector): TFixedVector; overload;
function Average(const V1, V2: TFixedVector): TFixedVector; overload;
function Max(const V1, V2: TFixedVector): TFixedVector; overload;
function Min(const V1, V2: TFixedVector): TFixedVector; overload;

function Dot(const V1, V2: TFixedVector): TFixed; overload;
function Distance(const V1, V2: TFixedVector): TFixed; overload;
function SqrDistance(const V1, V2: TFixedVector): TFixed; overload;

implementation
