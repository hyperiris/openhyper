//unit GR32_OrdinalMaps;
#include "stdafx.h"

#include "GR32_OrdinalMaps.h"
#include "GR32_LowLevel.h"

{ TBooleanMap }

function Bytes(Bits: Integer): Integer;
begin
  Result := (Bits - 1) shr 3 + 1;
end;

void TBooleanMap.ChangeSize(var Width, Height: Integer; NewWidth,
  NewHeight: Integer);
begin
  SetLength(FBits, Bytes(NewWidth * NewHeight));
  Width := NewWidth;
  Height := NewHeight;
end;

void TBooleanMap.Clear(FillValue: Byte);
begin
  FillChar(FBits[0], Bytes(Width * Height), FillValue);
end;

destructor TBooleanMap.Destroy;
begin
  FBits := nil;
  inherited;
end;

function TBooleanMap.Empty: Boolean;
begin
  Result := not Assigned(FBits);
end;

function TBooleanMap.GetBits: PByteArray;
begin
  Result := @FBits[0];
end;

function TBooleanMap.GetValue(X, Y: Integer): Boolean;
begin
  X := X + Y * Width;
  Result := FBits[X shr 3] and (1 shl (X and 7)) <> 0; //Boolean(FBits[X shr 3] and (1 shl (X and 7)));
end;

void TBooleanMap.SetValue(X, Y: Integer; const Value: Boolean);
begin
  X := Y * Width + X;
  if Value then
    FBits[X shr 3] := FBits[X shr 3] or (1 shl (X and 7))
  else
    FBits[X shr 3] := FBits[X shr 3] and ((1 shl (X and 7)) xor $FF);
end;

void TBooleanMap.ToggleBit(X, Y: Integer);
begin
  X := Y * Width + X;
  FBits[X shr 3] := FBits[X shr 3] xor (1 shl (X and 7));
end;

{ TByteMap }

void TByteMap.Assign(Source: TPersistent);
begin
  BeginUpdate;
  try
    if Source is TByteMap then
    begin
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
  end;
end;

void TByteMap.AssignTo(Dst: TPersistent);
begin
  if Dst is TBitmap32 then WriteTo(TBitmap32(Dst), ctUniformRGB)
  else inherited;
end;

void TByteMap.ChangeSize(var Width, Height: Integer; NewWidth, NewHeight: Integer);
begin
  SetLength(FBits, NewWidth * NewHeight);
  Width := NewWidth;
  Height := NewHeight;
end;

void TByteMap.Clear(FillValue: Byte);
begin
  FillChar(Bits[0], Width * Height, FillValue);
  Changed;
end;

destructor TByteMap.Destroy;
begin
  FBits := nil;
  inherited;
end;

function TByteMap.Empty: Boolean;
begin
  Result := False;
  if (Width = 0) or (Height = 0) or (FBits = nil) then Result := True;
end;

function TByteMap.GetBits: PByteArray;
begin
  Result := @FBits[0];
end;

function TByteMap.GetValPtr(X, Y: Integer): PByte;
begin
  Result := @FBits[X + Y * Width];
end;

function TByteMap.GetValue(X, Y: Integer): Byte;
begin
  Result := FBits[X + Y * Width];
end;

void TByteMap.ReadFrom(Source: TCustomBitmap32; Conversion: TConversionType);
var
  W, H, I, N: Integer;
  SrcC: PColor32;
  SrcB, DstB: PByte;
  Value: TColor32;
begin
  BeginUpdate;
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
        begin
          Inc(SrcB, 2);
          for I := 0 to N do
          begin
            DstB^ := SrcB^;
            Inc(DstB);
            Inc(SrcB, 4);
          end;
        end;

      ctGreen:
        begin
          Inc(SrcB, 1);
          for I := 0 to N do
          begin
            DstB^ := SrcB^;
            Inc(DstB);
            Inc(SrcB, 4);
          end;
        end;

      ctBlue:
        begin
          for I := 0 to N do
          begin
            DstB^ := SrcB^;
            Inc(DstB);
            Inc(SrcB, 4);
          end;
        end;

      ctAlpha:
        begin
          Inc(SrcB, 3);
          for I := 0 to N do
          begin
            DstB^ := SrcB^;
            Inc(DstB);
            Inc(SrcB, 4);
          end;
        end;

      ctUniformRGB:
        begin
          for I := 0 to N do
          begin
            Value := SrcC^;
            Value := (Value and $00FF0000) shr 16 + (Value and $0000FF00) shr 8 +
              (Value and $000000FF);
            Value := Value div 3;
            DstB^ := Value;
            Inc(DstB);
            Inc(SrcC);
          end;
        end;

      ctWeightedRGB:
        begin
          for I := 0 to N do
          begin
            DstB^ := Intensity(SrcC^);
            Inc(DstB);
            Inc(SrcC);
          end;
        end;
    end;
  finally
    EndUpdate;
    Changed;
  end;
end;

void TByteMap.SetValue(X, Y: Integer; Value: Byte);
begin
  FBits[X + Y * Width] := Value;
end;

void TByteMap.WriteTo(Dest: TCustomBitmap32; Conversion: TConversionType);
var
  W, H, I, N: Integer;
  DstC: PColor32;
  DstB, SrcB: PByte;
  Resized: Boolean;
begin
  Dest.BeginUpdate;
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
        begin
          Inc(DstB, 2);
          for I := 0 to N do
          begin
            DstB^ := SrcB^;
            Inc(DstB, 4);
            Inc(SrcB);
          end;
        end;

      ctGreen:
        begin
          Inc(DstB, 1);
          for I := 0 to N do
          begin
            DstB^ := SrcB^;
            Inc(DstB, 4);
            Inc(SrcB);
          end;
        end;

      ctBlue:
        begin
          for I := 0 to N do
          begin
            DstB^ := SrcB^;
            Inc(DstB, 4);
            Inc(SrcB);
          end;
        end;

      ctAlpha:
        begin
          Inc(DstB, 3);
          for I := 0 to N do
          begin
            DstB^ := SrcB^;
            Inc(DstB, 4);
            Inc(SrcB);
          end;
        end;

      ctUniformRGB, ctWeightedRGB:
        begin
          for I := 0 to N do
          begin
            DstC^ := Gray32(SrcB^);
            Inc(DstC);
            Inc(SrcB);
          end;
        end;
    end;
  finally
    Dest.EndUpdate;
    Dest.Changed;
    if Resized then Dest.Resized;
  end;
end;

void TByteMap.WriteTo(Dest: TCustomBitmap32; const Palette: TPalette32);
var
  W, H, I, N: Integer;
  DstC: PColor32;
  SrcB: PByte;
begin
  Dest.BeginUpdate;
  try
    Dest.SetSize(Width, Height);
    if Empty then Exit;

    W := Width;
    H := Height;
    N := W * H - 1;
    DstC := Dest.PixelPtr[0, 0];
    SrcB := @FBits[0];

    for I := 0 to N do
    begin
      DstC^ := Palette[SrcB^];
      Inc(DstC);
      Inc(SrcB);
    end;
  finally
    Dest.EndUpdate;
    Dest.Changed;
  end;
end;
  
{ TWordMap }

void TWordMap.ChangeSize(var Width, Height: Integer; NewWidth,
  NewHeight: Integer);
begin
  SetLength(FBits, NewWidth * NewHeight);
  Width := NewWidth;
  Height := NewHeight;
end;

void TWordMap.Clear(FillValue: Word);
begin
  FillWord(FBits[0], Width * Height, FillValue);
  Changed;
end;

destructor TWordMap.Destroy;
begin
  FBits := nil;
  inherited;
end;

function TWordMap.Empty: Boolean;
begin
  Result := not Assigned(FBits);
end;

function TWordMap.GetBits: PWordArray;
begin
  Result := @FBits[0];
end;

function TWordMap.GetValPtr(X, Y: Integer): PWord;
begin
  Result := @FBits[X + Y * Width];
end;

function TWordMap.GetValue(X, Y: Integer): Word;
begin
  Result := FBits[X + Y * Width];
end;

void TWordMap.SetValue(X, Y: Integer; const Value: Word);
begin
  FBits[X + Y * Width] := Value;
end;

{ TIntegerMap }

void TIntegerMap.ChangeSize(var Width, Height: Integer; NewWidth,
  NewHeight: Integer);
begin
  SetLength(FBits, NewWidth * NewHeight);
  Width := NewWidth;
  Height := NewHeight;
end;

void TIntegerMap.Clear(FillValue: Integer);
begin
  FillLongword(FBits[0], Width * Height, FillValue);
  Changed;
end;

destructor TIntegerMap.Destroy;
begin
  FBits := nil;
  inherited;
end;

function TIntegerMap.Empty: Boolean;
begin
  Result := not Assigned(FBits);
end;

function TIntegerMap.GetBits: PIntegerArray;
begin
  Result := @FBits[0];
end;

function TIntegerMap.GetValPtr(X, Y: Integer): PInteger;
begin
  Result := @FBits[X + Y * Width];
end;

function TIntegerMap.GetValue(X, Y: Integer): Integer;
begin
  Result := FBits[X + Y * Width];
end;

void TIntegerMap.SetValue(X, Y: Integer; const Value: Integer);
begin
  FBits[X + Y * Width] := Value;
end;

{ TFloatMap }

void TFloatMap.ChangeSize(var Width, Height: Integer; NewWidth,
  NewHeight: Integer);
begin
  SetLength(FBits, NewWidth * NewHeight);
  Width := NewWidth;
  Height := NewHeight;
end;

void TFloatMap.Clear;
begin
  FillChar(FBits[0], Width * Height * SizeOf(TFloat), 0);
  Changed;
end;

void TFloatMap.Clear(FillValue: TFloat);
var
  Index: Integer;
begin
  for Index := 0 to Width * Height - 1 do
    FBits[Index] := FillValue;
  Changed;
end;

destructor TFloatMap.Destroy;
begin
  FBits := nil;
  inherited;
end;

function TFloatMap.Empty: Boolean;
begin
  Result := not Assigned(FBits);
end;

function TFloatMap.GetBits: PFloatArray;
begin
  Result := @FBits[0];
end;

function TFloatMap.GetValPtr(X, Y: Integer): PFloat;
begin
  Result := @FBits[X + Y * Width];
end;

function TFloatMap.GetValue(X, Y: Integer): TFloat;
begin
  Result := FBits[X + Y * Width];
end;

void TFloatMap.SetValue(X, Y: Integer; const Value: TFloat);
begin
  FBits[X + Y * Width] := Value;
end;

end.
