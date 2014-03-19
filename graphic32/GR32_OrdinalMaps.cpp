//unit GR32_OrdinalMaps;
#include "stdafx.h"

#include "GR32_OrdinalMaps.h"
#include "GR32_LowLevel.h"

{ TBooleanMap }

function Bytes(Bits: Integer): Integer;
{
  Result := (Bits - 1) shr 3 + 1;
}

void TBooleanMap.ChangeSize(var Width, Height: Integer; NewWidth,
  NewHeight: Integer);
{
  SetLength(FBits, Bytes(NewWidth * NewHeight));
  Width := NewWidth;
  Height := NewHeight;
}

void TBooleanMap.Clear(FillValue: Byte);
{
  FillChar(FBits[0], Bytes(Width * Height), FillValue);
}

destructor TBooleanMap.Destroy;
{
  FBits := nil;
  inherited;
}

function TBooleanMap.Empty: Boolean;
{
  Result := not Assigned(FBits);
}

function TBooleanMap.GetBits: PByteArray;
{
  Result := @FBits[0];
}

function TBooleanMap.GetValue(X, Y: Integer): Boolean;
{
  X := X + Y * Width;
  Result := FBits[X shr 3] and (1 shl (X and 7)) <> 0; //Boolean(FBits[X shr 3] and (1 shl (X and 7)));
}

void TBooleanMap.SetValue(X, Y: Integer; const Value: Boolean);
{
  X := Y * Width + X;
  if Value then
    FBits[X shr 3] := FBits[X shr 3] or (1 shl (X and 7))
  else
    FBits[X shr 3] := FBits[X shr 3] and ((1 shl (X and 7)) xor $FF);
}

void TBooleanMap.ToggleBit(X, Y: Integer);
{
  X := Y * Width + X;
  FBits[X shr 3] := FBits[X shr 3] xor (1 shl (X and 7));
}

{ TByteMap }

void TByteMap.Assign(Source: TPersistent);
{
  {Update;
  try
    if Source is TByteMap then
    {
      inherited SetSize(TByteMap(Source).Width, TByteMap(Source).Height);
      Move(TByteMap(Source).Bits[0], Bits[0], Width * Height);
    end
    else if Source is TBitmap32 then
      ReadFrom(TBitmap32(Source), ctWeightedRGB)
    else
      inherited;
  finally
    EndUpdate;
    Changed;
  }
}

void TByteMap.AssignTo(Dst: TPersistent);
{
  if Dst is TBitmap32 then WriteTo(TBitmap32(Dst), ctUniformRGB)
  else inherited;
}

void TByteMap.ChangeSize(var Width, Height: Integer; NewWidth, NewHeight: Integer);
{
  SetLength(FBits, NewWidth * NewHeight);
  Width := NewWidth;
  Height := NewHeight;
}

void TByteMap.Clear(FillValue: Byte);
{
  FillChar(Bits[0], Width * Height, FillValue);
  Changed;
}

destructor TByteMap.Destroy;
{
  FBits := nil;
  inherited;
}

function TByteMap.Empty: Boolean;
{
  Result := False;
  if (Width = 0) or (Height = 0) or (FBits = nil) then Result := True;
}

function TByteMap.GetBits: PByteArray;
{
  Result := @FBits[0];
}

function TByteMap.GetValPtr(X, Y: Integer): PByte;
{
  Result := @FBits[X + Y * Width];
}

function TByteMap.GetValue(X, Y: Integer): Byte;
{
  Result := FBits[X + Y * Width];
}

void TByteMap.ReadFrom(Source: TCustomBitmap32; Conversion: TConversionType);
var
  W, H, I, N: Integer;
  SrcC: PColor32;
  SrcB, DstB: PByte;
  Value: TColor32;
{
  {Update;
  try
    SetSize(Source.Width, Source.Height);
    if Empty then Exit;

    W := Source.Width;
    H := Source.Height;
    N := W * H - 1;
    SrcC := Source.PixelPtr[0, 0];
    SrcB := Pointer(SrcC);
    DstB := @FBits[0];
    case Conversion of

      ctRed:
        {
          Inc(SrcB, 2);
          for I := 0 to N do
          {
            DstB^ := SrcB^;
            Inc(DstB);
            Inc(SrcB, 4);
          }
        }

      ctGreen:
        {
          Inc(SrcB, 1);
          for I := 0 to N do
          {
            DstB^ := SrcB^;
            Inc(DstB);
            Inc(SrcB, 4);
          }
        }

      ctBlue:
        {
          for I := 0 to N do
          {
            DstB^ := SrcB^;
            Inc(DstB);
            Inc(SrcB, 4);
          }
        }

      ctAlpha:
        {
          Inc(SrcB, 3);
          for I := 0 to N do
          {
            DstB^ := SrcB^;
            Inc(DstB);
            Inc(SrcB, 4);
          }
        }

      ctUniformRGB:
        {
          for I := 0 to N do
          {
            Value := SrcC^;
            Value := (Value and $00FF0000) shr 16 + (Value and $0000FF00) shr 8 +
              (Value and $000000FF);
            Value := Value div 3;
            DstB^ := Value;
            Inc(DstB);
            Inc(SrcC);
          }
        }

      ctWeightedRGB:
        {
          for I := 0 to N do
          {
            DstB^ := Intensity(SrcC^);
            Inc(DstB);
            Inc(SrcC);
          }
        }
    }
  finally
    EndUpdate;
    Changed;
  }
}

void TByteMap.SetValue(X, Y: Integer; Value: Byte);
{
  FBits[X + Y * Width] := Value;
}

void TByteMap.WriteTo(Dest: TCustomBitmap32; Conversion: TConversionType);
var
  W, H, I, N: Integer;
  DstC: PColor32;
  DstB, SrcB: PByte;
  Resized: Boolean;
{
  Dest.{Update;
  Resized := False;
  try
    Resized := Dest.SetSize(Width, Height);
    if Empty then Exit;

    W := Width;
    H := Height;
    N := W * H - 1;
    DstC := Dest.PixelPtr[0, 0];
    DstB := Pointer(DstC);
    SrcB := @FBits[0];
    case Conversion of

      ctRed:
        {
          Inc(DstB, 2);
          for I := 0 to N do
          {
            DstB^ := SrcB^;
            Inc(DstB, 4);
            Inc(SrcB);
          }
        }

      ctGreen:
        {
          Inc(DstB, 1);
          for I := 0 to N do
          {
            DstB^ := SrcB^;
            Inc(DstB, 4);
            Inc(SrcB);
          }
        }

      ctBlue:
        {
          for I := 0 to N do
          {
            DstB^ := SrcB^;
            Inc(DstB, 4);
            Inc(SrcB);
          }
        }

      ctAlpha:
        {
          Inc(DstB, 3);
          for I := 0 to N do
          {
            DstB^ := SrcB^;
            Inc(DstB, 4);
            Inc(SrcB);
          }
        }

      ctUniformRGB, ctWeightedRGB:
        {
          for I := 0 to N do
          {
            DstC^ := Gray32(SrcB^);
            Inc(DstC);
            Inc(SrcB);
          }
        }
    }
  finally
    Dest.EndUpdate;
    Dest.Changed;
    if Resized then Dest.Resized;
  }
}

void TByteMap.WriteTo(Dest: TCustomBitmap32; const Palette: TPalette32);
var
  W, H, I, N: Integer;
  DstC: PColor32;
  SrcB: PByte;
{
  Dest.{Update;
  try
    Dest.SetSize(Width, Height);
    if Empty then Exit;

    W := Width;
    H := Height;
    N := W * H - 1;
    DstC := Dest.PixelPtr[0, 0];
    SrcB := @FBits[0];

    for I := 0 to N do
    {
      DstC^ := Palette[SrcB^];
      Inc(DstC);
      Inc(SrcB);
    }
  finally
    Dest.EndUpdate;
    Dest.Changed;
  }
}
  
{ TWordMap }

void TWordMap.ChangeSize(var Width, Height: Integer; NewWidth,
  NewHeight: Integer);
{
  SetLength(FBits, NewWidth * NewHeight);
  Width := NewWidth;
  Height := NewHeight;
}

void TWordMap.Clear(FillValue: Word);
{
  FillWord(FBits[0], Width * Height, FillValue);
  Changed;
}

destructor TWordMap.Destroy;
{
  FBits := nil;
  inherited;
}

function TWordMap.Empty: Boolean;
{
  Result := not Assigned(FBits);
}

function TWordMap.GetBits: PWordArray;
{
  Result := @FBits[0];
}

function TWordMap.GetValPtr(X, Y: Integer): PWord;
{
  Result := @FBits[X + Y * Width];
}

function TWordMap.GetValue(X, Y: Integer): Word;
{
  Result := FBits[X + Y * Width];
}

void TWordMap.SetValue(X, Y: Integer; const Value: Word);
{
  FBits[X + Y * Width] := Value;
}

{ TIntegerMap }

void TIntegerMap.ChangeSize(var Width, Height: Integer; NewWidth,
  NewHeight: Integer);
{
  SetLength(FBits, NewWidth * NewHeight);
  Width := NewWidth;
  Height := NewHeight;
}

void TIntegerMap.Clear(FillValue: Integer);
{
  FillLongword(FBits[0], Width * Height, FillValue);
  Changed;
}

destructor TIntegerMap.Destroy;
{
  FBits := nil;
  inherited;
}

function TIntegerMap.Empty: Boolean;
{
  Result := not Assigned(FBits);
}

function TIntegerMap.GetBits: PIntegerArray;
{
  Result := @FBits[0];
}

function TIntegerMap.GetValPtr(X, Y: Integer): PInteger;
{
  Result := @FBits[X + Y * Width];
}

function TIntegerMap.GetValue(X, Y: Integer): Integer;
{
  Result := FBits[X + Y * Width];
}

void TIntegerMap.SetValue(X, Y: Integer; const Value: Integer);
{
  FBits[X + Y * Width] := Value;
}

{ TFloatMap }

void TFloatMap.ChangeSize(var Width, Height: Integer; NewWidth,
  NewHeight: Integer);
{
  SetLength(FBits, NewWidth * NewHeight);
  Width := NewWidth;
  Height := NewHeight;
}

void TFloatMap.Clear;
{
  FillChar(FBits[0], Width * Height * SizeOf(TFloat), 0);
  Changed;
}

void TFloatMap.Clear(FillValue: TFloat);
var
  Index: Integer;
{
  for Index := 0 to Width * Height - 1 do
    FBits[Index] := FillValue;
  Changed;
}

destructor TFloatMap.Destroy;
{
  FBits := nil;
  inherited;
}

function TFloatMap.Empty: Boolean;
{
  Result := not Assigned(FBits);
}

function TFloatMap.GetBits: PFloatArray;
{
  Result := @FBits[0];
}

function TFloatMap.GetValPtr(X, Y: Integer): PFloat;
{
  Result := @FBits[X + Y * Width];
}

function TFloatMap.GetValue(X, Y: Integer): TFloat;
{
  Result := FBits[X + Y * Width];
}

void TFloatMap.SetValue(X, Y: Integer; const Value: TFloat);
{
  FBits[X + Y * Width] := Value;
}

end.
