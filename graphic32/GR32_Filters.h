//unit GR32_Filters;
#pragma once

#include "GR32.h"
#include "GR32_Blend.h"
#include "GR32_System.h"
#include "GR32_Bindings.h"

{ Basic processing }
type
  TLUT8 = array [Byte] of Byte;
  TLogicalOperator = (loXOR, loAND, loOR);

procedure CopyComponents(Dst, Src: TCustomBitmap32; Components: TColor32Components);overload;
procedure CopyComponents(Dst: TCustomBitmap32; DstX, DstY: Integer; Src: TCustomBitmap32;
  SrcRect: TRect; Components: TColor32Components); overload;

procedure AlphaToGrayscale(Dst, Src: TCustomBitmap32);
procedure ColorToGrayscale(Dst, Src: TCustomBitmap32; PreserveAlpha: Boolean = False);
procedure IntensityToAlpha(Dst, Src: TCustomBitmap32);

procedure Invert(Dst, Src: TCustomBitmap32; Components : TColor32Components = [ccAlpha, ccRed, ccGreen, ccBlue]);
procedure InvertRGB(Dst, Src: TCustomBitmap32);

procedure ApplyLUT(Dst, Src: TCustomBitmap32; const LUT: TLUT8; PreserveAlpha: Boolean = False);
procedure ChromaKey(ABitmap: TCustomBitmap32; TrColor: TColor32);

function CreateBitmask(Components: TColor32Components): TColor32;

procedure ApplyBitmask(Dst: TCustomBitmap32; DstX, DstY: Integer; Src: TCustomBitmap32;
  SrcRect: TRect; Bitmask: TColor32; LogicalOperator: TLogicalOperator); overload;
procedure ApplyBitmask(ABitmap: TCustomBitmap32; ARect: TRect; Bitmask: TColor32;
  LogicalOperator: TLogicalOperator); overload;

procedure CheckParams(Dst, Src: TCustomBitmap32; ResizeDst: Boolean = True);

implementation
