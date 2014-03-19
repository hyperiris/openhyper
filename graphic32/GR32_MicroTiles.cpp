//unit GR32_MicroTiles;
#include "stdafx.h"

#include "GR32_MicroTiles.h"
#include "GR32_LowLevel.h"
#include "GR32_Math.h"

implementation

var
  MicroTilesU: void(var DstTiles: TMicroTiles; const SrcTiles: TMicroTiles);

{ MicroTile auxiliary routines }

function MakeMicroTile(const Left, Top, Right, Bottom: Integer): TMicroTile;
{
  Result := Left shl 24 or Top shl 16 or Right shl 8 or Bottom;
}

function MicroTileHeight(const Tile: TMicroTile): Integer;
{
  Result := (Tile and $FF) - (Tile shr 16 and $FF);
}

function MicroTileWidth(const Tile: TMicroTile): Integer;
{
  Result := (Tile shr 8 and $FF) - (Tile shr 24);
}

void MicroTileUnion_Pas(var DstTile: TMicroTile; const SrcTile: TMicroTile);
var
  SrcLeft, SrcTop, SrcRight, SrcBottom: Integer;
{
  SrcLeft := SrcTile shr 24;
  SrcTop := (SrcTile and $FF0000) shr 16;
  SrcRight := (SrcTile and $FF00) shr 8;
  SrcBottom := SrcTile and $FF;

  if (DstTile <> MICROTILE_FULL) and (SrcTile <> MICROTILE_EMPTY) and
     (SrcRight - SrcLeft <> 0) and (SrcBottom - SrcTop <> 0) then
  {
    if (DstTile = MICROTILE_EMPTY) or (SrcTile = MICROTILE_FULL) then
      DstTile := SrcTile
    else
    {
      DstTile := Min(DstTile shr 24, SrcLeft) shl 24 or
                 Min(DstTile shr 16 and $FF, SrcTop) shl 16 or
                 Max(DstTile shr 8 and $FF, SrcRight) shl 8 or
                 Max(DstTile and $FF, SrcBottom);
    }
  }
}

{$IFDEF TARGET_x86}
void MicroTileUnion_EMMX(var DstTile: TMicroTile; const SrcTile: TMicroTile);
var
  SrcLeft, SrcTop, SrcRight, SrcBottom: Integer;
{
  SrcLeft := SrcTile shr 24;
  SrcTop := (SrcTile and $FF0000) shr 16;
  SrcRight := (SrcTile and $FF00) shr 8;
  SrcBottom := SrcTile and $FF;

  if (DstTile <> MICROTILE_FULL) and (SrcTile <> MICROTILE_EMPTY) and
     (SrcRight - SrcLeft <> 0) and (SrcBottom - SrcTop <> 0) then
  {
    if (DstTile = MICROTILE_EMPTY) or (SrcTile = MICROTILE_FULL) then
      DstTile := SrcTile
    else
    asm
      MOVD   MM1,[SrcTile]

      MOV    EAX,[DstTile]
      MOVD   MM2, [EAX]

      MOVQ   MM3, MM1

      MOV    ECX,$FFFF0000   // Mask
      MOVD   MM0, ECX
      PMINUB MM1, MM2
      PAND   MM1, MM0

      PSRLD  MM0, 16         // shift mask right by 16 bits
      PMAXUB MM2, MM3
      PAND   MM2, MM0

      POR    MM1, MM2

      MOVD   [EAX], MM1

      EMMS
    }
  }
}
{$ENDIF}

{ MicroTiles auxiliary routines }

function MakeEmptyMicroTiles: TMicroTiles;
{
  FillChar(Result, SizeOf(TMicroTiles), 0);
  ReallocMem(Result.Tiles, 0);
}

void MicroTilesCreate(var MicroTiles: TMicroTiles);
{
  FillChar(MicroTiles, SizeOf(TMicroTiles), 0);
  ReallocMem(MicroTiles.Tiles, 0);
}

void MicroTilesDestroy(var MicroTiles: TMicroTiles);
{
  ReallocMem(MicroTiles.Tiles, 0);
}

void MicroTilesSetSize(var MicroTiles: TMicroTiles; const DstRect: TRect);
{
  MicroTiles.BoundsRect := DstRect;
  MicroTiles.Columns := ((DstRect.Right - DstRect.Left) shr MICROTILE_SHIFT) + 1;
  MicroTiles.Rows := ((DstRect.Bottom - DstRect.Top) shr MICROTILE_SHIFT) + 1;

  MicroTiles.Count := (MicroTiles.Columns + 1) * (MicroTiles.Rows + 1);
  ReallocMem(MicroTiles.Tiles, MicroTiles.Count * SizeOf(TMicroTile));

  MicroTilesClear(MicroTiles)
}

void MicroTilesClear(var MicroTiles: TMicroTiles; const Value: TMicroTile);
{
  MicroTiles.BoundsUsedTiles := MakeRect(MicroTiles.Columns, MicroTiles.Rows, 0, 0);
  FillLongword(MicroTiles.Tiles^[0], MicroTiles.Count, Value);
}

void MicroTilesClearUsed(var MicroTiles: TMicroTiles; const Value: TMicroTile);
var
  I: Integer;
{
  for I := MicroTiles.BoundsUsedTiles.Top to MicroTiles.BoundsUsedTiles.Bottom do
    FillLongword(MicroTiles.Tiles^[I * MicroTiles.Columns + MicroTiles.BoundsUsedTiles.Left],
      MicroTiles.BoundsUsedTiles.Right - MicroTiles.BoundsUsedTiles.Left + 1, Value);

  MicroTiles.BoundsUsedTiles := MakeRect(MicroTiles.Columns, MicroTiles.Rows, 0, 0);
}

void MicroTilesCopy(var DstTiles: TMicroTiles; SrcTiles: TMicroTiles);
var
  CurRow, Width: Integer;
  SrcTilePtr, DstTilePtr: PMicroTile;
{
  if Assigned(DstTiles.Tiles) and (DstTiles.Count > 0) then
    MicroTilesClearUsed(DstTiles);

  DstTiles.BoundsRect := SrcTiles.BoundsRect;
  DstTiles.Columns := SrcTiles.Columns;
  DstTiles.Rows := SrcTiles.Rows;
  DstTiles.BoundsUsedTiles := SrcTiles.BoundsUsedTiles;

  ReallocMem(DstTiles.Tiles, SrcTiles.Count * SizeOf(TMicroTile));

  if DstTiles.Count < SrcTiles.Count then
    FillLongword(DstTiles.Tiles^[DstTiles.Count], SrcTiles.Count - DstTiles.Count, MICROTILE_EMPTY);

  DstTiles.Count := SrcTiles.Count;

  SrcTilePtr := @SrcTiles.Tiles^[SrcTiles.BoundsUsedTiles.Top * SrcTiles.Columns + SrcTiles.BoundsUsedTiles.Left];
  DstTilePtr := @DstTiles.Tiles^[SrcTiles.BoundsUsedTiles.Top * DstTiles.Columns + SrcTiles.BoundsUsedTiles.Left];
  Width := SrcTiles.BoundsUsedTiles.Right - SrcTiles.BoundsUsedTiles.Left + 1;

  for CurRow := SrcTiles.BoundsUsedTiles.Top to SrcTiles.BoundsUsedTiles.Bottom do
  {
    MoveLongword(SrcTilePtr^, DstTilePtr^, Width);
    Inc(DstTilePtr, DstTiles.Columns);
    Inc(SrcTilePtr, SrcTiles.Columns);
  end
}

void MicroTilesAddLine(var MicroTiles: TMicroTiles; X1, Y1, X2, Y2: Integer; LineWidth: Integer; RoundToWholeTiles: Boolean = False);
var
  I: Integer;
  Dx, Dy: Integer;
  Sx, Sy: Integer;
  DeltaX, DeltaY: Integer;
  Rects: Integer;
  NewX, NewY: Integer;
  TempRect: TRect;
  Swapped: Boolean;
{
  Dx := X2 - X1;
  Dy := Y2 - Y1;

  LineWidth := LineWidth shl 1;

  if Dx > 0 then
    Sx := 1
  else if Dx < 0 then
  {
    Dx := -Dx;
    Sx := -1;
  end
  else // Dx = 0
  {
    TempRect := MakeRect(X1, Y1, X2, Y2);
    InflateArea(TempRect, LineWidth, LineWidth);
    MicroTilesAddRect(MicroTiles, TempRect, RoundToWholeTiles);
    Exit;
  }

  if Dy > 0 then
    Sy := 1
  else if Dy < 0 then
  {
    Dy := -Dy;
    Sy := -1;
  end
  else // Dy = 0
  {
    TempRect := MakeRect(X1, Y1, X2, Y2);
    InflateArea(TempRect, LineWidth, LineWidth);
    MicroTilesAddRect(MicroTiles, TempRect, RoundToWholeTiles);
    Exit;
  }

  X1 := X1 * FixedOne;
  Y1 := Y1 * FixedOne;

  Dx := Dx * FixedOne;
  Dy := Dy * FixedOne;

  if Dx < Dy then
  {
    Swapped := True;
    Swap(Dx, Dy);
  end
  else
    Swapped := False;

  Rects := Dx div MICROTILE_SIZE;

  DeltaX := MICROTILE_SIZE * FixedOne;
  DeltaY := FixedDiv(Dy, Rects);

  if Swapped then
    Swap(DeltaX, DeltaY);

  DeltaX := Sx * DeltaX;
  DeltaY := Sy * DeltaY;

  for I := 1 to FixedCeil(Rects) do
  {
    NewX := X1 + DeltaX;
    NewY := Y1 + DeltaY;

    TempRect := MakeRect(FixedRect(X1, Y1, NewX, NewY));
    InflateArea(TempRect, LineWidth, LineWidth);
    MicroTilesAddRect(MicroTiles, TempRect, RoundToWholeTiles);

    X1 := NewX;
    Y1 := NewY;
  }
}

void MicroTilesAddRect(var MicroTiles: TMicroTiles; Rect: TRect; RoundToWholeTiles: Boolean);
var
  ModLeft, ModRight, ModTop, ModBottom, Temp: Integer;
  LeftTile, TopTile, RightTile, BottomTile, ColSpread, RowSpread: Integer;
  CurRow, CurCol: Integer;
  TilePtr, TilePtr2: PMicroTile;
{
  if MicroTiles.Count = 0 then Exit;

  with Rect do
  {
    TestSwap(Left, Right);
    TestSwap(Top, Bottom);

    if Left < 0 then Left := 0;
    if Top < 0 then Top := 0;
    Temp := MicroTiles.Columns shl MICROTILE_SHIFT;
    if Right > Temp then Right := Temp;
    Temp := MicroTiles.Rows shl MICROTILE_SHIFT;
    if Bottom > Temp then Bottom := Temp;
    
    if (Left > Right) or (Top > Bottom) then Exit;
  }

  LeftTile := Rect.Left shr MICROTILE_SHIFT;
  TopTile := Rect.Top shr MICROTILE_SHIFT;
  RightTile := Rect.Right shr MICROTILE_SHIFT;
  BottomTile := Rect.Bottom shr MICROTILE_SHIFT;

  TilePtr := @MicroTiles.Tiles^[TopTile * MicroTiles.Columns + LeftTile];

  if RoundToWholeTiles then
  {
    for CurRow := TopTile to BottomTile do
    {
      FillLongword(TilePtr^, RightTile - LeftTile + 1, MICROTILE_FULL);
      Inc(TilePtr, MicroTiles.Columns);
    }
  end
  else
  {
    // calculate number of tiles needed in columns and rows
    ColSpread := ((Rect.Right + MICROTILE_SIZE) shr MICROTILE_SHIFT) -
             (Rect.Left shr MICROTILE_SHIFT);
    RowSpread := ((Rect.Bottom + MICROTILE_SIZE) shr MICROTILE_SHIFT) -
              (Rect.Top shr MICROTILE_SHIFT);

    ModLeft := Rect.Left mod MICROTILE_SIZE;
    ModTop := Rect.Top mod MICROTILE_SIZE;
    ModRight := Rect.Right mod MICROTILE_SIZE;
    ModBottom := Rect.Bottom mod MICROTILE_SIZE;

    if (ColSpread = 1) and (RowSpread = 1) then
      MicroTileUnion(TilePtr^, MakeMicroTile(ModLeft, ModTop, ModRight, ModBottom))
    else if ColSpread = 1 then
    {
      MicroTileUnion(TilePtr^, MakeMicroTile(ModLeft, ModTop, ModRight, MICROTILE_SIZE));
      Inc(TilePtr, MicroTiles.Columns);

      if RowSpread > 2 then
        for CurCol := TopTile + 1 to BottomTile - 1 do
        {
          MicroTileUnion(TilePtr^, MakeMicroTile(ModLeft, 0, ModRight, MICROTILE_SIZE));
          Inc(TilePtr, MicroTiles.Columns);
        }

      MicroTileUnion(TilePtr^, MakeMicroTile(ModLeft, 0, ModRight, ModBottom));
    end
    else if RowSpread = 1 then
    {
      MicroTileUnion(TilePtr^, MakeMicroTile(ModLeft, ModTop, MICROTILE_SIZE, ModBottom));
      Inc(TilePtr);

      if ColSpread > 2 then
        for CurRow := LeftTile + 1 to RightTile - 1 do
        {
          MicroTileUnion(TilePtr^, MakeMicroTile(0, ModTop, MICROTILE_SIZE, ModBottom));
          Inc(TilePtr);
        }

      MicroTileUnion(TilePtr^, MakeMicroTile(0, ModTop, ModRight, ModBottom));
    end
    else
    {
      TilePtr2 := TilePtr;

      // TOP:
      // render top-left corner
      MicroTileUnion(TilePtr2^, MakeMicroTile(ModLeft, ModTop, MICROTILE_SIZE, MICROTILE_SIZE));
      Inc(TilePtr2);

      // render top edge
      if ColSpread > 2 then
        for CurRow := LeftTile + 1 to RightTile - 1 do
        {
          MicroTileUnion(TilePtr2^, MakeMicroTile(0, ModTop, MICROTILE_SIZE, MICROTILE_SIZE));
          Inc(TilePtr2);
        }

      // render top-right corner
      MicroTileUnion(TilePtr2^, MakeMicroTile(0, ModTop, ModRight, MICROTILE_SIZE));

      Inc(TilePtr, MicroTiles.Columns);

      // INTERMEDIATE AREA:
      if RowSpread > 2 then
        for CurCol := TopTile + 1 to BottomTile - 1 do
        {
          TilePtr2 := TilePtr;

          // render left edge
          MicroTileUnion(TilePtr2^, MakeMicroTile(ModLeft, 0, MICROTILE_SIZE, MICROTILE_SIZE));
          Inc(TilePtr2);

          // render content
          if ColSpread > 2 then
          {
            FillLongword(TilePtr2^, RightTile - LeftTile - 1, MICROTILE_FULL);
            Inc(TilePtr2, RightTile - LeftTile - 1);
          }

          // render right edge
          MicroTileUnion(TilePtr2^, MakeMicroTile(0, 0, ModRight, MICROTILE_SIZE));

          Inc(TilePtr, MicroTiles.Columns);
        }

      TilePtr2 := TilePtr;

      // BOTTOM:
      // render bottom-left corner
      MicroTileUnion(TilePtr2^, MakeMicroTile(ModLeft, 0, MICROTILE_SIZE, ModBottom));
      Inc(TilePtr2);

      // render bottom edge
      if ColSpread > 2 then
        for CurRow := LeftTile + 1 to RightTile - 1 do
        {
          MicroTileUnion(TilePtr2^, MakeMicroTile(0, 0, MICROTILE_SIZE, ModBottom));
          Inc(TilePtr2);
        }

      // render bottom-right corner
      MicroTileUnion(TilePtr2^, MakeMicroTile(0, 0, ModRight, ModBottom));
    }
  }

  with MicroTiles.BoundsUsedTiles do
  {
    if LeftTile < Left then Left := LeftTile;
    if TopTile < Top then Top := TopTile;
    if RightTile > Right then Right := RightTile;
    if BottomTile > Bottom then Bottom := BottomTile;
  }
}


void MicroTilesUnion_Pas(var DstTiles: TMicroTiles; const SrcTiles: TMicroTiles);
var
  SrcTilePtr, DstTilePtr: PMicroTile;
  SrcTilePtr2, DstTilePtr2: PMicroTile;
  X, Y: Integer;
  SrcLeft, SrcTop, SrcRight, SrcBottom: Integer;
  SrcTile: TMicroTile;
{
  SrcTilePtr := @SrcTiles.Tiles^[SrcTiles.BoundsUsedTiles.Top * SrcTiles.Columns + SrcTiles.BoundsUsedTiles.Left];
  DstTilePtr := @DstTiles.Tiles^[SrcTiles.BoundsUsedTiles.Top * DstTiles.Columns + SrcTiles.BoundsUsedTiles.Left];

  for Y := SrcTiles.BoundsUsedTiles.Top to SrcTiles.BoundsUsedTiles.Bottom do
  {
    SrcTilePtr2 := SrcTilePtr;
    DstTilePtr2 := DstTilePtr;
    for X := SrcTiles.BoundsUsedTiles.Left to SrcTiles.BoundsUsedTiles.Right do
    {
      SrcTile := SrcTilePtr2^;
      SrcLeft := SrcTile shr 24;
      SrcTop := (SrcTile and $FF0000) shr 16;
      SrcRight := (SrcTile and $FF00) shr 8;
      SrcBottom := SrcTile and $FF;

      if (DstTilePtr2^ <> MICROTILE_FULL) and (SrcTilePtr2^ <> MICROTILE_EMPTY) and
         (SrcRight - SrcLeft <> 0) and (SrcBottom - SrcTop <> 0) then
      {
        if (DstTilePtr2^ = MICROTILE_EMPTY) or (SrcTilePtr2^ = MICROTILE_FULL) then
          DstTilePtr2^ := SrcTilePtr2^
        else
          DstTilePtr2^ := Min(DstTilePtr2^ shr 24, SrcLeft) shl 24 or
                          Min(DstTilePtr2^ shr 16 and $FF, SrcTop) shl 16 or
                          Max(DstTilePtr2^ shr 8 and $FF, SrcRight) shl 8 or
                          Max(DstTilePtr2^ and $FF, SrcBottom);
      }

      Inc(DstTilePtr2);
      Inc(SrcTilePtr2);
    }
    Inc(DstTilePtr, DstTiles.Columns);
    Inc(SrcTilePtr, SrcTiles.Columns);
  }
}

{$IFDEF TARGET_x86}
void MicroTilesUnion_EMMX(var DstTiles: TMicroTiles; const SrcTiles: TMicroTiles);
var
  SrcTilePtr, DstTilePtr: PMicroTile;
  SrcTilePtr2, DstTilePtr2: PMicroTile;
  X, Y: Integer;
  SrcLeft, SrcTop, SrcRight, SrcBottom: Integer;
{
  SrcTilePtr := @SrcTiles.Tiles^[SrcTiles.BoundsUsedTiles.Top * SrcTiles.Columns + SrcTiles.BoundsUsedTiles.Left];
  DstTilePtr := @DstTiles.Tiles^[SrcTiles.BoundsUsedTiles.Top * DstTiles.Columns + SrcTiles.BoundsUsedTiles.Left];

  asm
    MOV    ECX, $FFFF  // Mask
    MOVD   MM0, ECX
    MOVQ   MM4, MM0
    PSLLD  MM4, 16     // shift mask left by 16 bits
  }

  for Y := SrcTiles.BoundsUsedTiles.Top to SrcTiles.BoundsUsedTiles.Bottom do
  {
    SrcTilePtr2 := SrcTilePtr;
    DstTilePtr2 := DstTilePtr;
    for X := SrcTiles.BoundsUsedTiles.Left to SrcTiles.BoundsUsedTiles.Right do
    {
      SrcLeft := SrcTilePtr2^ shr 24;
      SrcTop := (SrcTilePtr2^ and $FF0000) shr 16;
      SrcRight := (SrcTilePtr2^ and $FF00) shr 8;
      SrcBottom := SrcTilePtr2^ and $FF;

      if (DstTilePtr2^ <> MICROTILE_FULL) and (SrcTilePtr2^ <> MICROTILE_EMPTY) and
         (SrcRight - SrcLeft <> 0) and (SrcBottom - SrcTop <> 0) then
      {
        if (DstTilePtr2^ = MICROTILE_EMPTY) or (SrcTilePtr2^ = MICROTILE_FULL) then
          DstTilePtr2^ := SrcTilePtr2^
        else
        asm
          MOV    EAX, [DstTilePtr2]
          MOVD   MM2, [EAX]

          MOV    ECX, [SrcTilePtr2]
          MOVD   MM1, [ECX]
          MOVQ   MM3, MM1

          PMINUB MM1, MM2
          PAND   MM1, MM4

          PMAXUB MM2, MM3
          PAND   MM2, MM0

          POR    MM1, MM2

          MOVD   [EAX], MM1
        }
      }

      Inc(DstTilePtr2);
      Inc(SrcTilePtr2);
    }
    Inc(DstTilePtr, DstTiles.Columns);
    Inc(SrcTilePtr, SrcTiles.Columns);
  }

  asm
    db $0F,$77               /// EMMS
  }
}
{$ENDIF}

void MicroTilesUnion(var DstTiles: TMicroTiles; const SrcTiles: TMicroTiles; RoundToWholeTiles: Boolean);
var
  SrcTilePtr, DstTilePtr: PMicroTile;
  SrcTilePtr2, DstTilePtr2: PMicroTile;
  X, Y: Integer;
  SrcLeft, SrcTop, SrcRight, SrcBottom: Integer;
{
  if SrcTiles.Count = 0 then Exit;

  if RoundToWholeTiles then
  {
    SrcTilePtr := @SrcTiles.Tiles^[SrcTiles.BoundsUsedTiles.Top * SrcTiles.Columns + SrcTiles.BoundsUsedTiles.Left];
    DstTilePtr := @DstTiles.Tiles^[SrcTiles.BoundsUsedTiles.Top * DstTiles.Columns + SrcTiles.BoundsUsedTiles.Left];

    for Y := SrcTiles.BoundsUsedTiles.Top to SrcTiles.BoundsUsedTiles.Bottom do
    {
      SrcTilePtr2 := SrcTilePtr;
      DstTilePtr2 := DstTilePtr;
      for X := SrcTiles.BoundsUsedTiles.Left to SrcTiles.BoundsUsedTiles.Right do
      {
        SrcLeft := SrcTilePtr2^ shr 24;
        SrcTop := (SrcTilePtr2^ and $FF0000) shr 16;
        SrcRight := (SrcTilePtr2^ and $FF00) shr 8;
        SrcBottom := SrcTilePtr2^ and $FF;

        if (DstTilePtr2^ <> MICROTILE_FULL) and (SrcTilePtr2^ <> MICROTILE_EMPTY) and
           (SrcRight - SrcLeft <> 0) and (SrcBottom - SrcTop <> 0) then
          DstTilePtr2^ := MICROTILE_FULL;

        Inc(DstTilePtr2);
        Inc(SrcTilePtr2);
      }
      Inc(DstTilePtr, DstTiles.Columns);
      Inc(SrcTilePtr, SrcTiles.Columns);
    end
  end
  else
    MicroTilesU(DstTiles, SrcTiles);

  with DstTiles.BoundsUsedTiles do
  {
    if SrcTiles.BoundsUsedTiles.Left < Left then Left := SrcTiles.BoundsUsedTiles.Left;
    if SrcTiles.BoundsUsedTiles.Top < Top then Top := SrcTiles.BoundsUsedTiles.Top;
    if SrcTiles.BoundsUsedTiles.Right > Right then Right := SrcTiles.BoundsUsedTiles.Right;
    if SrcTiles.BoundsUsedTiles.Bottom > Bottom then Bottom := SrcTiles.BoundsUsedTiles.Bottom;
  }
}

function MicroTilesCalcRects(const MicroTiles: TMicroTiles; DstRects: TRectList;
  CountOnly, RoundToWholeTiles: Boolean): Integer;
{
  Result := MicroTilesCalcRects(MicroTiles, DstRects, MicroTiles.BoundsRect, CountOnly);
}


function MicroTilesCalcRects(const MicroTiles: TMicroTiles; DstRects: TRectList;
  const Clip: TRect; CountOnly, RoundToWholeTiles: Boolean): Integer;
var
  Rects: Array Of TRect;
  Rect: PRect;
  CombLUT: Array Of Integer;
  StartIndex: Integer;
  CurTile, TempTile: TMicroTile;
  Temp: Integer;
  NewLeft, NewTop, NewRight, NewBottom: Integer;
  CurCol, CurRow, I, RectsCount: Integer;
{
  Result := 0;

  if (MicroTiles.Count = 0) or
     (MicroTiles.BoundsUsedTiles.Right - MicroTiles.BoundsUsedTiles.Left < 0) or
     (MicroTiles.BoundsUsedTiles.Bottom - MicroTiles.BoundsUsedTiles.Top < 0) then Exit;

  SetLength(Rects, MicroTiles.Columns * MicroTiles.Rows);
  SetLength(CombLUT, MicroTiles.Columns * MicroTiles.Rows);
  FillLongword(CombLUT[0], Length(CombLUT), Cardinal(-1));

  I := 0;
  RectsCount := 0;

  if not RoundToWholeTiles then
    for CurRow := 0 to MicroTiles.Rows - 1 do
    {
      CurCol := 0;
      while CurCol < MicroTiles.Columns do
      {
        CurTile := MicroTiles.Tiles[I];

        if CurTile <> MICROTILE_EMPTY then
        {
          Temp := CurRow shl MICROTILE_SHIFT;
          NewTop := Constrain(Temp + CurTile shr 16 and $FF, Clip.Top, Clip.Bottom);
          NewBottom := Constrain(Temp + CurTile and $FF, Clip.Top, Clip.Bottom);
          NewLeft := Constrain(CurCol shl MICROTILE_SHIFT + CurTile shr 24, Clip.Left, Clip.Right);

          StartIndex := I;

          if (CurTile shr 8 and $FF = MICROTILE_SIZE) and (CurCol <> MicroTiles.Columns - 1) then
          {
            while True do
            {
              Inc(CurCol);
              Inc(I);

              TempTile := MicroTiles.Tiles[I];
              if (CurCol = MicroTiles.Columns) or
                 (TempTile shr 16 and $FF <> CurTile shr 16 and $FF) or
                 (TempTile and $FF <> CurTile and $FF) or
                 (TempTile shr 24 <> 0) then
              {
                Dec(CurCol);
                Dec(I);
                Break;
              }
            }
          }

          NewRight := Constrain(CurCol shl MICROTILE_SHIFT + MicroTiles.Tiles[I] shr 8 and $FF, Clip.Left, Clip.Right);

          Temp := CombLUT[StartIndex];

          Rect := nil;
          if Temp <> -1 then Rect := @Rects[Temp];

          if Assigned(Rect) and
             (Rect.Left = NewLeft) and
             (Rect.Right = NewRight) and
             (Rect.Bottom = NewTop) then
          {
            Rect.Bottom := NewBottom;

            if CurRow <> MicroTiles.Rows - 1 then
              CombLUT[StartIndex + MicroTiles.Columns] := Temp;
          end
          else
            with Rects[RectsCount] do
            {
              Left := NewLeft;    Top := NewTop;
              Right := NewRight;  Bottom := NewBottom;

              if CurRow <> MicroTiles.Rows - 1 then
                CombLUT[StartIndex + MicroTiles.Columns] := RectsCount;

              Inc(RectsCount);
            }
        }

        Inc(I);
        Inc(CurCol);
      }
    end
  else
    for CurRow := 0 to MicroTiles.Rows - 1 do
    {
      CurCol := 0;
      while CurCol < MicroTiles.Columns do
      {
        CurTile := MicroTiles.Tiles[I];

        if CurTile <> MICROTILE_EMPTY then
        {
          Temp := CurRow shl MICROTILE_SHIFT;
          NewTop := Constrain(Temp, Clip.Top, Clip.Bottom);
          NewBottom := Constrain(Temp + MICROTILE_SIZE, Clip.Top, Clip.Bottom);
          NewLeft := Constrain(CurCol shl MICROTILE_SHIFT, Clip.Left, Clip.Right);

          StartIndex := I;

          if CurCol <> MicroTiles.Columns - 1 then
          {
            while True do
            {
              Inc(CurCol);
              Inc(I);

              TempTile := MicroTiles.Tiles[I];
              if (CurCol = MicroTiles.Columns) or (TempTile = MICROTILE_EMPTY) then
              {
                Dec(CurCol);
                Dec(I);
                Break;
              }
            }
          }

          NewRight := Constrain(CurCol shl MICROTILE_SHIFT + MICROTILE_SIZE, Clip.Left, Clip.Right);

          Temp := CombLUT[StartIndex];

          Rect := nil;
          if Temp <> -1 then Rect := @Rects[Temp];

          if Assigned(Rect) and
             (Rect.Left = NewLeft) and
             (Rect.Right = NewRight) and
             (Rect.Bottom = NewTop) then
          {
            Rect.Bottom := NewBottom;

            if CurRow <> MicroTiles.Rows - 1 then
              CombLUT[StartIndex + MicroTiles.Columns] := Temp;
          end
          else
            with Rects[RectsCount] do
            {
              Left := NewLeft;    Top := NewTop;
              Right := NewRight;  Bottom := NewBottom;

              if CurRow <> MicroTiles.Rows - 1 then
                CombLUT[StartIndex + MicroTiles.Columns] := RectsCount;

              Inc(RectsCount);
            }
        }

        Inc(I);
        Inc(CurCol);
      }
    }


  Result := RectsCount;

  if not CountOnly then
    for I := 0 to RectsCount - 1 do DstRects.Add(Rects[I]);
}

function MicroTilesCountEmptyTiles(const MicroTiles: TMicroTiles): Integer;
var
  CurRow, CurCol: Integer;
  TilePtr: PMicroTile;
{
  Result := 0;
  if MicroTiles.Count > 0 then
  {
    TilePtr := @MicroTiles.Tiles^[0];
    for CurRow := 0 to MicroTiles.Rows - 1 do
      for CurCol := 0 to MicroTiles.Columns - 1 do
      {
        if TilePtr^ = MICROTILE_EMPTY then Inc(Result);
        Inc(TilePtr);
      }
  }
}

{$IFDEF MICROTILES_DEBUGDRAW}
void MicroTilesDebugDraw(const MicroTiles: TMicroTiles; DstBitmap: TBitmap32; DrawOptimized, RoundToWholeTiles: Boolean);
var
  I: Integer;
  TempRect: TRect;
  Rects: TRectList;

  C1, C2: TColor32;
{
{$IFDEF MICROTILES_DEBUGDRAW_RANDOM_COLORS}
  C1 := Random(MaxInt) AND $00FFFFFF;
  C2 := C1 OR $90000000;
  C1 := C1 OR $30000000;
{$ELSE}
  C1 := clDebugDrawFill;
  C2 := clDebugDrawFrame;
{$ENDIF}

  if DrawOptimized then
  {
    Rects := TRectList.Create;
    MicroTilesCalcRects(MicroTiles, Rects, False, RoundToWholeTiles);
    try
      if Rects.Count > 0 then
      {
        for I := 0 to Rects.Count - 1 do
        {
          DstBitmap.FillRectTS(Rects[I]^, C1);
          DstBitmap.FrameRectTS(Rects[I]^, C2);
        }
      end
    finally
      Rects.Free;
    }
  end
  else
    for I := 0 to MicroTiles.Count - 1 do
    {
      if MicroTiles.Tiles^[i] <> MICROTILE_EMPTY then
      {
        TempRect.Left := ((I mod MicroTiles.Columns) shl MICROTILE_SHIFT) + (MicroTiles.Tiles[i] shr 24);
        TempRect.Top := ((I div MicroTiles.Columns) shl MICROTILE_SHIFT) + (MicroTiles.Tiles[i] shr 16 and $FF);
        TempRect.Right := ((I mod MicroTiles.Columns) shl MICROTILE_SHIFT) + (MicroTiles.Tiles[i] shr 8 and $FF);
        TempRect.Bottom := ((I div MicroTiles.Columns) shl MICROTILE_SHIFT) + (MicroTiles.Tiles[i] and $FF);

        DstBitmap.FillRectTS(TempRect, C1);
        DstBitmap.FrameRectTS(TempRect, C2);
      }
    }
}
{$ENDIF}

{ TMicroTilesMap }

function TMicroTilesMap.Add(Item: Pointer): PPMicroTiles;
var
  TilesPtr: PMicroTiles;
  IsNew: Boolean;
{
  Result := PPMicroTiles(inherited Add(Item, IsNew));
  if IsNew then
  {
    New(TilesPtr);
    MicroTilesCreate(TilesPtr^);
    Result^ := TilesPtr;
  }
}

function TMicroTilesMap.Delete(BucketIndex, ItemIndex: Integer): Pointer;
var
  TilesPtr: PMicroTiles;
{
  TilesPtr := inherited Delete(BucketIndex, ItemIndex);
  MicroTilesDestroy(TilesPtr^);
  Dispose(TilesPtr);
  Result := nil;
}

void TMicroTilesMap.SetData(Item: Pointer; const Data: PMicroTiles);
{
  inherited SetData(Item, Data);
}

function TMicroTilesMap.GetData(Item: Pointer): PMicroTiles;
{
  Result := inherited GetData(Item);
}



{ TMicroTilesRepaintManager }

type
  TLayerCollectionAccess = class(TLayerCollection);
  TCustomLayerAccess = class(TCustomLayer);

const
  PL_MICROTILES         = 0;
  PL_WHOLETILES         = 1;
  PL_FULLSCENE          = 2;

  TIMER_PENALTY         = 250;
  TIMER_LOWLIMIT        = 1000;
  TIMER_HIGHLIMIT       = 5000;

  INVALIDRECTS_DELTA    = 10;

constructor TMicroTilesRepaintOptimizer.Create(Buffer: TBitmap32; InvalidRects: TRectList);
{
  inherited;
  FOldInvalidTilesMap := TMicroTilesMap.Create;
  FInvalidLayers := TList.Create;
  FPerfTimer := TPerfTimer.Create;
{$IFNDEF MICROTILES_DEBUGDRAW}
  {$IFNDEF MICROTILES_NO_ADAPTION}
  FAdaptiveMode := True;
  {$ENDIF}
{$ENDIF}

  MicroTilesCreate(FInvalidTiles);
  MicroTilesCreate(FTempTiles);
  MicroTilesCreate(FForcedInvalidTiles);

{$IFDEF MICROTILES_DEBUGDRAW}
  MicroTilesCreate(FDebugMicroTiles);
  FDebugInvalidRects := TRectList.Create;
{$ENDIF}
}

destructor TMicroTilesRepaintOptimizer.Destroy;
{
  MicroTilesDestroy(FForcedInvalidTiles);
  MicroTilesDestroy(FTempTiles);
  MicroTilesDestroy(FInvalidTiles);

  FPerfTimer.Free;
  FInvalidLayers.Free;
  FOldInvalidTilesMap.Free;

{$IFDEF MICROTILES_DEBUGDRAW}
  FDebugInvalidRects.Free;
  MicroTilesDestroy(FDebugMicroTiles);
{$ENDIF}

  inherited;
}

void TMicroTilesRepaintOptimizer.AreaUpdateHandler(Sender: TObject; const Area: TRect;
  const Info: Cardinal);
{
  ValidateWorkingTiles;
  AddArea(FForcedInvalidTiles, Area, Info);
  FUseInvalidTiles := True;
}

void TMicroTilesRepaintOptimizer.AddArea(var Tiles: TMicroTiles; const Area: TRect;
  const Info: Cardinal);
var
  LineWidth: Integer;
  TempRect: TRect;
{
  if Info and AREAINFO_LINE <> 0 then
  {
    LineWidth := Info and $00FFFFFF;
    TempRect := Area;
    InflateArea(TempRect, LineWidth, LineWidth);
    with TempRect do
      MicroTilesAddLine(Tiles, Left, Top, Right, Bottom, LineWidth, FPerformanceLevel > PL_MICROTILES);
  end
  else
    MicroTilesAddRect(Tiles, Area, FPerformanceLevel > PL_MICROTILES);
}

void TMicroTilesRepaintOptimizer.LayerUpdateHandler(Sender: TObject; Layer: TCustomLayer);
{
  if FOldInvalidTilesValid and not TCustomLayerAccess(Layer).Invalid then
  {
    FInvalidLayers.Add(Layer);
    TCustomLayerAccess(Layer).Invalid := True;
    FUseInvalidTiles := True;
  }
}

void TMicroTilesRepaintOptimizer.LayerCollectionNotifyHandler(Sender: TLayerCollection;
  Action: TLayerListNotification; Layer: TCustomLayer; Index: Integer);
var
  TilesPtr: PMicroTiles;
{
  case Action of
    lnLayerAdded, lnLayerInserted:
      {
        TilesPtr := FOldInvalidTilesMap.Add(Layer)^;
        MicroTilesSetSize(TilesPtr^, Buffer.BoundsRect);
        FOldInvalidTilesValid := True;
      }

    lnLayerDeleted:
      {
        if FOldInvalidTilesValid then
        {
          // force repaint of tiles that the layer did previously allocate
          MicroTilesUnion(FInvalidTiles, FOldInvalidTilesMap[Layer]^);
          FUseInvalidTiles := True;
        }
        FInvalidLayers.Remove(Layer);
        FOldInvalidTilesMap.Remove(Layer);
      }

    lnCleared:
      {
        if FOldInvalidTilesValid then
        {
          with TPointerMapIterator.Create(FOldInvalidTilesMap) do
          try
            while Next do
              MicroTilesUnion(FInvalidTiles, PMicroTiles(Data)^);
          finally
            Free;
          }

          FUseInvalidTiles := True;
          ResetAdaptiveMode;
        }
        FOldInvalidTilesMap.Clear;
        FOldInvalidTilesValid := True;
      }
  }
}

void TMicroTilesRepaintOptimizer.ValidateWorkingTiles;
{
  if not FWorkingTilesValid then  // check if working microtiles need resize...
  {
    MicroTilesSetSize(FTempTiles, FBufferBounds);
    MicroTilesSetSize(FInvalidTiles, FBufferBounds);
    MicroTilesSetSize(FForcedInvalidTiles, FBufferBounds);
    FWorkingTilesValid := True;
  }
}

void TMicroTilesRepaintOptimizer.BufferResizedHandler(const NewWidth, NewHeight: Integer);
{
  FBufferBounds := MakeRect(0, 0, NewWidth, NewHeight);
  Reset;
}

void TMicroTilesRepaintOptimizer.Reset;
{
  FWorkingTilesValid := False;     // force resizing of working microtiles
  FOldInvalidTilesValid := False;  // force resizing and rerendering of invalid tiles
  UpdateOldInvalidTiles;

  // mark whole buffer area invalid... 
  MicroTilesClear(FForcedInvalidTiles, MICROTILE_FULL);
  FForcedInvalidTiles.BoundsUsedTiles := MakeRect(0, 0, FForcedInvalidTiles.Columns, FForcedInvalidTiles.Rows);
  FUseInvalidTiles := True;
}

function TMicroTilesRepaintOptimizer.UpdatesAvailable: Boolean;
{
  UpdateOldInvalidTiles;
  Result := FUseInvalidTiles;
}

void TMicroTilesRepaintOptimizer.UpdateOldInvalidTiles;
var
  I, J: Integer;
  TilesPtr: PMicroTiles;
  Layer: TCustomLayer;
{
  if not FOldInvalidTilesValid then  // check if old Invalid tiles need resize and rerendering...
  {
    ValidateWorkingTiles;

    for I := 0 to LayerCollections.Count - 1 do
    with TLayerCollection(LayerCollections[I]) do
      for J := 0 to Count - 1 do
      {
        Layer := Items[J];
        TilesPtr := FOldInvalidTilesMap.Add(Layer)^;

        MicroTilesSetSize(TilesPtr^, FBufferBounds);
        DrawLayerToMicroTiles(TilesPtr^, Layer);
        TCustomLayerAccess(Layer).Invalid := False;
      }

    FInvalidLayers.Clear;

    FOldInvalidTilesValid := True;
    FUseInvalidTiles := False;
  }
}

void TMicroTilesRepaintOptimizer.RegisterLayerCollection(Layers: TLayerCollection);
{
  inherited;

  if Enabled then
    with TLayerCollectionAccess(Layers) do
    {
      OnLayerUpdated := LayerUpdateHandler;
      OnAreaUpdated := AreaUpdateHandler;
      OnListNotify := LayerCollectionNotifyHandler;
    }
}

void TMicroTilesRepaintOptimizer.UnregisterLayerCollection(Layers: TLayerCollection);
{
  with TLayerCollectionAccess(Layers) do
  {
    OnLayerUpdated := nil;
    OnAreaUpdated := nil;
    OnListNotify := nil;
  }

  inherited;
}

void TMicroTilesRepaintOptimizer.SetEnabled(const Value: Boolean);
var
  I: Integer;
{
  if Value <> Enabled then
  {
    if Value then
    {
      // initialize:
      for I := 0 to LayerCollections.Count - 1 do
      with TLayerCollectionAccess(LayerCollections[I]) do
      {
        OnLayerUpdated := LayerUpdateHandler;
        OnAreaUpdated := AreaUpdateHandler;
        OnListNotify := LayerCollectionNotifyHandler;
      }

      BufferResizedHandler(Buffer.Width, Buffer.Height);
    end
    else
    {
      // clean up:
      for I := 0 to LayerCollections.Count - 1 do
      with TLayerCollectionAccess(LayerCollections[I]) do
      {
        OnLayerUpdated := nil;
        OnAreaUpdated := nil;
        OnListNotify := nil;
      }

      MicroTilesDestroy(FInvalidTiles);
      MicroTilesDestroy(FTempTiles);
      MicroTilesDestroy(FForcedInvalidTiles);

      FUseInvalidTiles := False;
      FOldInvalidTilesValid := False;
      FOldInvalidTilesMap.Clear;
      FInvalidLayers.Clear;
    }
    inherited;
  }
}

void TMicroTilesRepaintOptimizer.SetAdaptiveMode(const Value: Boolean);
{
  if FAdaptiveMode <> Value then
  {
    FAdaptiveMode := Value;
    ResetAdaptiveMode;
  }
}

void TMicroTilesRepaintOptimizer.ResetAdaptiveMode;
{
  FTimeDelta := TIMER_LOWLIMIT;
  FAdaptionFailed := False;
  FPerformanceLevel := PL_MICROTILES;
}

void TMicroTilesRepaintOptimizer.{PaintBuffer;
{
  if AdaptiveMode then FPerfTimer.Start;
}

void TMicroTilesRepaintOptimizer.EndPaintBuffer;
{
  FUseInvalidTiles := False;

{$IFDEF MICROTILES_DEBUGDRAW}
  {$IFDEF MICROTILES_DEBUGDRAW_UNOPTIMIZED}
    MicroTilesDebugDraw(FDebugMicroTiles, Buffer, False, FDebugWholeTiles);
  {$ELSE}
    MicroTilesDebugDraw(FDebugMicroTiles, Buffer, True, FDebugWholeTiles);
  {$ENDIF}
  MicroTilesClear(FDebugMicroTiles);
{$ENDIF}

{$IFNDEF MICROTILES_NO_ADAPTION}
  EndAdaption;
{$ENDIF}
}

void TMicroTilesRepaintOptimizer.DrawLayerToMicroTiles(var DstTiles: TMicroTiles; Layer: TCustomLayer);
{
  Buffer.{Measuring(DrawMeasuringHandler);
  FWorkMicroTiles := @DstTiles;
  TCustomLayerAccess(Layer).DoPaint(Buffer);
  Buffer.EndMeasuring;
}

void TMicroTilesRepaintOptimizer.DrawMeasuringHandler(Sender: TObject; const Area: TRect;
  const Info: Cardinal);
{
  AddArea(FWorkMicroTiles^, Area, Info);
}

void TMicroTilesRepaintOptimizer.PerformOptimization;
var
  I: Integer;
  Layer: TCustomLayer;
  UseWholeTiles: Boolean;
  LayerTilesPtr: PMicroTiles;
{
  if FUseInvalidTiles then
  {
    ValidateWorkingTiles;
    // Determine if the use of whole tiles is better for current performance level
{$IFNDEF MICROTILES_NO_ADAPTION}
    UseWholeTiles := FPerformanceLevel > PL_MICROTILES;
{$ELSE}
  {$IFDEF MICROTILES_NO_ADAPTION_FORCE_WHOLETILES}
    UseWholeTiles := True;
  {$ELSE}
    UseWholeTiles := False;
  {$ENDIF}
{$ENDIF}

    if FInvalidLayers.Count > 0 then
    {
      for I := 0 to FInvalidLayers.Count - 1 do
      {
        Layer := FInvalidLayers[I];

        // Clear temporary tiles
        MicroTilesClearUsed(FTempTiles);
        // Draw layer to temporary tiles
        DrawLayerToMicroTiles(FTempTiles, Layer);

        // Combine temporary tiles with the global invalid tiles
        MicroTilesUnion(FInvalidTiles, FTempTiles, UseWholeTiles);

        // Retrieve old invalid tiles for the current layer
        LayerTilesPtr := FOldInvalidTilesMap[Layer];

        // Combine old invalid tiles with the global invalid tiles
        MicroTilesUnion(FInvalidTiles, LayerTilesPtr^, UseWholeTiles);

        // Copy temporary (current) invalid tiles to the layer
        MicroTilesCopy(LayerTilesPtr^, FTempTiles);

        // Unmark layer as invalid
        TCustomLayerAccess(Layer).Invalid := False;
      }
      FInvalidLayers.Clear;
    }

{$IFDEF MICROTILES_DEBUGDRAW}
    MicroTilesCalcRects(FInvalidTiles, InvalidRects, False, UseWholeTiles);
    MicroTilesCalcRects(FForcedInvalidTiles, InvalidRects, False, UseWholeTiles);
    MicroTilesCopy(FDebugMicroTiles, FInvalidTiles);
    MicroTilesUnion(FDebugMicroTiles, FForcedInvalidTiles);
    FDebugWholeTiles := UseWholeTiles;
{$ELSE}
    // Calculate optimized rectangles from global invalid tiles
    MicroTilesCalcRects(FInvalidTiles, InvalidRects, False, UseWholeTiles);
    // Calculate optimized rectangles from forced invalid tiles
    MicroTilesCalcRects(FForcedInvalidTiles, InvalidRects, False, UseWholeTiles);
{$ENDIF}
  }

{$IFNDEF MICROTILES_NO_ADAPTION}
  {Adaption;
{$ENDIF}

{$IFDEF MICROTILES_DEBUGDRAW}
  if InvalidRects.Count > 0 then
  {
    FDebugInvalidRects.Count := InvalidRects.Count;
    Move(InvalidRects[0]^, FDebugInvalidRects[0]^, InvalidRects.Count * SizeOf(TRect));
    InvalidRects.Clear;
  }
{$ENDIF}

  // Rects have been created, so we don't need the tiles any longer, clear them.
  MicroTilesClearUsed(FInvalidTiles);
  MicroTilesClearUsed(FForcedInvalidTiles);
}

void TMicroTilesRepaintOptimizer.{Adaption;
{
  if AdaptiveMode and (FPerformanceLevel > PL_MICROTILES) then
  {
    if Integer(GetTickCount) > FNextCheck then
    {
      FPerformanceLevel := Constrain(FPerformanceLevel - 1, PL_MICROTILES, PL_FULLSCENE);
      {$IFDEF CODESITE}
      CodeSite.SendInteger('PrepareInvalidRects(Timed): FPerformanceLevel', FPerformanceLevel);
      {$ENDIF}
      FTimedCheck := True;
    end
    else if not FAdaptionFailed and (InvalidRects.Count < FOldInvalidRectsCount - INVALIDRECTS_DELTA) then
    {
      FPerformanceLevel := Constrain(FPerformanceLevel - 1, PL_MICROTILES, PL_FULLSCENE);
      {$IFDEF CODESITE}
      CodeSite.SendInteger('PrepareInvalidRects: FPerformanceLevel', FPerformanceLevel);
      {$ENDIF}
    end
    else if FPerformanceLevel = PL_FULLSCENE then
      // we need a full scene rendition, so clear the invalid rects
      InvalidRects.Clear;
  }
}

void TMicroTilesRepaintOptimizer.EndAdaption;
var
  TimeElapsed: Int64;
  Level: Integer;
{
  // our KISS(TM) repaint mode balancing starts here...
  TimeElapsed := FPerfTimer.ReadValue;

{$IFDEF MICROTILES_DEBUGDRAW}
  if FDebugInvalidRects.Count = 0 then
{$ELSE}
  if InvalidRects.Count = 0 then
{$ENDIF}
    FElapsedTimeForFullSceneRepaint := TimeElapsed
  else if AdaptiveMode then
  {
    if TimeElapsed > FElapsedTimeForFullSceneRepaint then
    {
      Level := Constrain(FPerformanceLevel + 1, PL_MICROTILES, PL_FULLSCENE);
      // did performance level change from previous level?
      if Level <> FPerformanceLevel then
      {
{$IFDEF MICROTILES_DEBUGDRAW}
        FOldInvalidRectsCount := FDebugInvalidRects.Count;
{$ELSE}
        // save count of old invalid rects so we can use it in PrepareInvalidRects
        // the next time...
        FOldInvalidRectsCount := InvalidRects.Count;
{$ENDIF}
        FPerformanceLevel := Level;
        {$IFDEF CODESITE}
        CodeSite.SendInteger('EndPaintBuffer: FPerformanceLevel', FPerformanceLevel);
        {$ENDIF}
        // was this a timed check?
        if FTimedCheck then
        {
          // time based approach failed, so add penalty
          FTimeDelta := Constrain(Integer(FTimeDelta + TIMER_PENALTY), TIMER_LOWLIMIT, TIMER_HIGHLIMIT);
          // schedule next check
          FNextCheck := Integer(GetTickCount) + FTimeDelta;
          FElapsedTimeOnLastPenalty := TimeElapsed;
          FTimedCheck := False;
          {$IFDEF CODESITE}
          CodeSite.SendInteger('timed check failed, new delta', FTimeDelta);
          {$ENDIF}
        }
        {$IFDEF CODESITE}
        CodeSite.AddSeparator;
        {$ENDIF}
        FAdaptionFailed := True;
      }
    end
    else if TimeElapsed < FElapsedTimeForFullSceneRepaint then
    {
      if FTimedCheck then
      {
        // time based approach had success!!
        // reset time delta back to lower limit, ie. remove penalties
        FTimeDelta := TIMER_LOWLIMIT;
        // schedule next check
        FNextCheck := Integer(GetTickCount) + FTimeDelta;
        FTimedCheck := False;
        {$IFDEF CODESITE}
        CodeSite.SendInteger('timed check succeeded, new delta', FTimeDelta);
        CodeSite.AddSeparator;
        {$ENDIF}
        FAdaptionFailed := False;
      end
      else
      {
        // invalid rect count approach had success!!
        // shorten time for next check to benefit nonetheless in case we have a fallback...
        if FTimeDelta > TIMER_LOWLIMIT then
        {
          // remove the penalty value 4 times from the current time delta
          FTimeDelta := Constrain(FTimeDelta - 4 * TIMER_PENALTY, TIMER_LOWLIMIT, TIMER_HIGHLIMIT);
          // schedule next check
          FNextCheck := Integer(GetTickCount) + FTimeDelta;
          {$IFDEF CODESITE}
          CodeSite.SendInteger('invalid rect count approach succeeded, new timer delta', FTimeDelta);
          CodeSite.AddSeparator;
          {$ENDIF}
        }
        FAdaptionFailed := False;
      }
    end
    else if (TimeElapsed < FElapsedTimeOnLastPenalty) and FTimedCheck then
    {
      // time approach had success optimizing the situation, so shorten time until next check
      FTimeDelta := Constrain(FTimeDelta - TIMER_PENALTY, TIMER_LOWLIMIT, TIMER_HIGHLIMIT);
      // schedule next check
      FNextCheck := Integer(GetTickCount) + FTimeDelta;
      FTimedCheck := False;
      {$IFDEF CODESITE}
      CodeSite.SendInteger('timed check succeeded, new delta', FTimeDelta);
      CodeSite.AddSeparator;
      {$ENDIF}
    }
  }

  FElapsedTimeForLastRepaint := TimeElapsed;
}

{$IFDEF CODESITE}

{ TDebugMicroTilesRepaintOptimizer }

void TDebugMicroTilesRepaintOptimizer.AreaUpdateHandler(Sender: TObject;
  const Area: TRect; const Info: Cardinal);
{
  DumpCallStack('TDebugMicroTilesRepaintOptimizer.AreaUpdateHandler');
  inherited;
}

void TDebugMicroTilesRepaintOptimizer.{PaintBuffer;
{
  DumpCallStack('TDebugMicroTilesRepaintOptimizer.{PaintBuffer');
  inherited;
}

void TDebugMicroTilesRepaintOptimizer.BufferResizedHandler(const NewWidth,
  NewHeight: Integer);
{
  DumpCallStack('TDebugMicroTilesRepaintOptimizer.BufferResizedHandler');
  inherited;
}

void TDebugMicroTilesRepaintOptimizer.EndPaintBuffer;
{
  DumpCallStack('TDebugMicroTilesRepaintOptimizer.EndPaintBuffer');
  inherited;
  CodeSite.AddSeparator;  
}

void TDebugMicroTilesRepaintOptimizer.LayerUpdateHandler(Sender: TObject;
  Layer: TCustomLayer);
{
  DumpCallStack('TDebugMicroTilesRepaintOptimizer.LayerUpdateHandler');
  inherited;
}

void TDebugMicroTilesRepaintOptimizer.PerformOptimization;
{
  DumpCallStack('TDebugMicroTilesRepaintOptimizer.PerformOptimization');
  inherited;
}

void TDebugMicroTilesRepaintOptimizer.Reset;
{
  DumpCallStack('TDebugMicroTilesRepaintOptimizer.Reset');
  inherited;
  CodeSite.AddSeparator;
}

function TDebugMicroTilesRepaintOptimizer.UpdatesAvailable: Boolean;
{
  DumpCallStack('TDebugMicroTilesRepaintOptimizer.UpdatesAvailable');
  Result := inherited UpdatesAvailable;
}

{$ENDIF}

const
  FID_MICROTILEUNION = 0;
  FID_MICROTILESUNION = 1;

var
  Registry: TFunctionRegistry;

void RegisterBindings;
{
  Registry := NewRegistry('GR32_MicroTiles bindings');
  Registry.RegisterBinding(FID_MICROTILEUNION, @@MicroTileUnion);
  Registry.RegisterBinding(FID_MICROTILESUNION, @@MicroTilesU);
  Registry.Add(FID_MICROTILEUNION, @MicroTileUnion_Pas);
  Registry.Add(FID_MICROTILESUNION, @MicroTilesUnion_Pas);

{$IFNDEF PUREPASCAL}
{$IFDEF TARGET_x86}
  Registry.Add(FID_MICROTILEUNION, @MicroTileUnion_EMMX, [ciEMMX]);
  Registry.Add(FID_MICROTILESUNION, @MicroTilesUnion_EMMX, [ciEMMX]);
{$ENDIF}
{$ENDIF}
  Registry.RebindAll;
}

initialization
  RegisterBindings;

end.
