//unit GR32_DrawingEx;
#pragma once

#include "GR32.h"

/* ClipLine */
/* Clips the (X1, Y1)-(X2,Y2) line to the rectangle using Sutherland-Cohen Line
  Clipping Algorithm */
BOOL ClipLine(var X1, Y1, X2, Y2: Integer; MinX, MinY, MaxX, MaxY: Integer);

type
  TBlendLineProc = procedure(Src, Dst: PColor32; Count: Integer);
