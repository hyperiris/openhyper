//unit GR32_Blend;
#pragma once
#include "GR32.h"
#include "GR32_System.h"
#include "GR32_Bindings.h"

var
  MMX_ACTIVE: Boolean;

type
{ Function Prototypes }
  TBlendReg    = function(F, B: TColor32): TColor32;
  TBlendMem    = procedure(F: TColor32; var B: TColor32);
  TBlendRegEx  = function(F, B, M: TColor32): TColor32;
  TBlendMemEx  = procedure(F: TColor32; var B: TColor32; M: TColor32);
  TBlendLine   = procedure(Src, Dst: PColor32; Count: Integer);
  TBlendLineEx = procedure(Src, Dst: PColor32; Count: Integer; M: TColor32);
  TCombineReg  = function(X, Y, W: TColor32): TColor32;
  TCombineMem  = procedure(X: TColor32; var Y: TColor32; W: TColor32);
  TCombineLine = procedure(Src, Dst: PColor32; Count: Integer; W: TColor32);
  TLightenReg  = function(C: TColor32; Amount: Integer): TColor32;

var
{$IFNDEF OMIT_MMX}
  EMMS: procedure;
{$ENDIF}

{ Function Variables }
  BlendReg: TBlendReg;
  BlendMem: TBlendMem;

  BlendRegEx: TBlendRegEx;
  BlendMemEx: TBlendMemEx;

  BlendLine: TBlendLine;
  BlendLineEx: TBlendLineEx;

  CombineReg: TCombineReg;
  CombineMem: TCombineMem;
  CombineLine: TCombineLine;

  MergeReg: TBlendReg;
  MergeMem: TBlendMem;

  MergeRegEx: TBlendRegEx;
  MergeMemEx: TBlendMemEx;

  MergeLine: TBlendLine;
  MergeLineEx: TBlendLineEx;

{ Color algebra functions }
  ColorAdd: TBlendReg;
  ColorSub: TBlendReg;
  ColorDiv: TBlendReg;
  ColorModulate: TBlendReg;
  ColorMax: TBlendReg;
  ColorMin: TBlendReg;
  ColorDifference: TBlendReg;
  ColorAverage: TBlendReg;
  ColorExclusion: TBlendReg;
  ColorScale: TBlendReg;

{ Special LUT pointers }
  AlphaTable: Pointer;
  bias_ptr: Pointer;
  alpha_ptr: Pointer;


{ Misc stuff }
  LightenReg: TLightenReg;

function Lighten(C: TColor32; Amount: Integer): TColor32; {$IFDEF USEINLINING} inline; {$ENDIF}

{ Access to alpha composite functions corresponding to a combine mode }

const
  BLEND_REG: array[TCombineMode] of ^TBlendReg = ((@@BlendReg),(@@MergeReg));
  BLEND_MEM: array[TCombineMode] of ^TBlendMem = ((@@BlendMem),(@@MergeMem));
  BLEND_REG_EX: array[TCombineMode] of ^TBlendRegEx = ((@@BlendRegEx),(@@MergeRegEx));
  BLEND_MEM_EX: array[TCombineMode] of ^TBlendMemEx = ((@@BlendMemEx),(@@MergeMemEx));
  BLEND_LINE: array[TCombineMode] of ^TBlendLine = ((@@BlendLine),(@@MergeLine));
  BLEND_LINE_EX: array[TCombineMode] of ^TBlendLineEx = ((@@BlendLineEx),(@@MergeLineEx));

var
  BlendRegistry: TFunctionRegistry;

{$IFDEF OMIT_MMX}
procedure EMMS; {$IFDEF USEINLINING} inline; {$ENDIF}
{$ENDIF}

implementation
