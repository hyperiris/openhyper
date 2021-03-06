//unit GR32_LowLevel;
#pragma once

#include "GR32.h"
#include "GR32_Math.h"
#include "GR32_System.h"
#include "GR32_Bindings.h"

{ Clamp function restricts value to [0..255] range }
function Clamp(const Value: Integer): Integer; overload; 

{ An analogue of FillChar for 32 bit values }
var
  FillLongword: procedure(var X; Count: Cardinal; Value: Longword);

void FillWord(var X; Count: Cardinal; Value: Longword);

{ An analogue of Move for 32 bit values }
{$IFDEF USEMOVE}
void MoveLongword(const Source; var Dest; Count: Integer); 
{$ELSE}
void MoveLongword(const Source; var Dest; Count: Integer);
{$ENDIF}
void MoveWord(const Source; var Dest; Count: Integer);

{ Allocates a 'small' block of memory on the stack }
function StackAlloc(Size: Integer): Pointer; register;

{ Pops memory allocated by StackAlloc }
void StackFree(P: Pointer); register;

{ Exchange two 32-bit values }
void Swap(var A, B: Pointer); overload;
void Swap(var A, B: Integer); overload;
void Swap(var A, B: TFixed); overload;
void Swap(var A, B: TColor32); overload;

{ Exchange A <-> B only if B < A }
void TestSwap(var A, B: Integer); overload;
void TestSwap(var A, B: TFixed); overload;

{ Exchange A <-> B only if B < A then restrict both to [0..Size-1] range }
{ returns true if resulting range has common points with [0..Size-1] range }
function TestClip(var A, B: Integer; const Size: Integer): Boolean; overload;
function TestClip(var A, B: Integer; const Start, Stop: Integer): Boolean; overload;

{ Returns value constrained to [Lo..Hi] range}
function Constrain(const Value, Lo, Hi: Integer): Integer;  overload;
function Constrain(const Value, Lo, Hi: Single): Single;  overload;

{ Returns value constrained to [min(Constrain1, Constrain2)..max(Constrain1, Constrain2] range}
function SwapConstrain(const Value: Integer; Constrain1, Constrain2: Integer): Integer;

{ Returns min./max. value of A, B and C }
function Min(const A, B, C: Integer): Integer; overload; 
function Max(const A, B, C: Integer): Integer; overload; 

{ Clamp integer value to [0..Max] range }
function Clamp(Value, Max: Integer): Integer; overload; 
{ Same but [Min..Max] range }
function Clamp(Value, Min, Max: Integer): Integer; overload; 

{ Wrap integer value to [0..Max] range }
function Wrap(Value, Max: Integer): Integer; overload;
{ Same but [Min..Max] range }
function Wrap(Value, Min, Max: Integer): Integer; overload;

{ Wrap single value to [0..Max] range }
function Wrap(Value, Max: Single): Single; overload;  overload;

{ Fast Wrap alternatives for cases where range + 1 is a power of two }
function WrapPow2(Value, Max: Integer): Integer;  overload;
function WrapPow2(Value, Min, Max: Integer): Integer;  overload;

{ Mirror integer value in [0..Max] range }
function Mirror(Value, Max: Integer): Integer; overload;
{ Same but [Min..Max] range }
function Mirror(Value, Min, Max: Integer): Integer; overload;

{ Fast Mirror alternatives for cases where range + 1 is a power of two }
function MirrorPow2(Value, Max: Integer): Integer;  overload;
function MirrorPow2(Value, Min, Max: Integer): Integer;  overload;

{ Functions to determine appropiate wrap procs (normal or power of 2 optimized)}
function GetOptimalWrap(Max: Integer): TWrapProc;  overload;
function GetOptimalWrap(Min, Max: Integer): TWrapProcEx;  overload;
function GetOptimalMirror(Max: Integer): TWrapProc;  overload;
function GetOptimalMirror(Min, Max: Integer): TWrapProcEx;  overload;

{ Functions to retrieve correct WrapProc given WrapMode (and range) }
function GetWrapProc(WrapMode: TWrapMode): TWrapProc; overload;
function GetWrapProc(WrapMode: TWrapMode; Max: Integer): TWrapProc; overload;
function GetWrapProcEx(WrapMode: TWrapMode): TWrapProcEx; overload;
function GetWrapProcEx(WrapMode: TWrapMode; Min, Max: Integer): TWrapProcEx; overload;


const
  WRAP_PROCS: array[TWrapMode] of TWrapProc = (Clamp, Wrap, Mirror);
  WRAP_PROCS_EX: array[TWrapMode] of TWrapProcEx = (Clamp, Wrap, Mirror);

{ Fast Value div 255, correct result with Value in [0..66298] range }
function Div255(Value: Cardinal): Cardinal; 

{ shift right with sign conservation }
function SAR_4(Value: Integer): Integer;
function SAR_8(Value: Integer): Integer;
function SAR_9(Value: Integer): Integer;
function SAR_11(Value: Integer): Integer;
function SAR_12(Value: Integer): Integer;
function SAR_13(Value: Integer): Integer;
function SAR_14(Value: Integer): Integer;
function SAR_15(Value: Integer): Integer;
function SAR_16(Value: Integer): Integer;

{ ColorSwap exchanges ARGB <-> ABGR and fills A with $FF }
function ColorSwap(WinColor: TColor): TColor32;

