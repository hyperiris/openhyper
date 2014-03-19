//unit GR32_DrawingEx;
#include "stdafx.h"
#include "GR32_DrawingEx.h"

#include "GR32_LowLevel.h"
#include "GR32_Blend.h"
#include "GR32_Math.h"

BOOL ClipLine(var X1, Y1, X2, Y2: Integer; MinX, MinY, MaxX, MaxY: Integer)
{
	int C1;
	int C2;
	int V;
	//{ Get edge codes }
   C1: = Ord(X1 < MinX) + Ord(X1 > MaxX) shl 1 + Ord(Y1 < MinY) shl 2 + Ord(Y1 > MaxY) shl 3;
   C2: = Ord(X2 < MinX) + Ord(X2 > MaxX) shl 1 + Ord(Y2 < MinY) shl 2 + Ord(Y2 > MaxY) shl 3;

	   if ((C1 and C2) = 0) and((C1 or C2) <> 0) then
		   begin
	   if (C1 and 12) <> 0 then
		   begin
	   if C1 < 8 then V : = MinY else V : = MaxY;
	   Inc(X1, MulDiv(V - Y1, X2 - X1, Y2 - Y1));
   Y1: = V;
   C1: = Ord(X1 < MinX) + Ord(X1 > MaxX) shl 1;
	   end;

	   if (C2 and 12) <> 0 then
		   begin
	   if C2 < 8 then V : = MinY else V : = MaxY;
	   Inc(X2, MulDiv(V - Y2, X2 - X1, Y2 - Y1));
   Y2: = V;
   C2: = Ord(X2 < MinX) + Ord(X2 > MaxX) shl 1;
	   end;

	   if ((C1 and C2) = 0) and((C1 or C2) <> 0) then
		   begin
	   if C1 <> 0 then
		   begin
	   if C1 = 1 then V : = MinX else V : = MaxX;
	   Inc(Y1, MulDiv(V - X1, Y2 - Y1, X2 - X1));
   X1: = V;
   C1: = 0;
	   end;

	   if C2 <> 0 then
		   begin
	   if C2 = 1 then V : = MinX else V : = MaxX;
	   Inc(Y2, MulDiv(V - X2, Y2 - Y1, X2 - X1));
   X2: = V;
   C2: = 0;
	   end;
	   end;
	   end;

   Result: = (C1 or C2) = 0;
}
