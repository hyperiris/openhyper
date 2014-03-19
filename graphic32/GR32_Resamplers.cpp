//unit GR32_Resamplers;

#include "GR32_Resamplers.h"

#include "GR32_LowLevel.h"
#include "GR32_Rasterizers.h"
#include "GR32_Math.h"

resourcestring
  RCStrInvalidSrcRect = 'Invalid SrcRect';

const
  CAlbrecht2    : array [0..1] of Double = (5.383553946707251E-1, 4.616446053292749E-1);
  CAlbrecht3    : array [0..2] of Double = (3.46100822018625E-1,  4.97340635096738E-1,
                                            1.56558542884637E-1);
  CAlbrecht4    : array [0..3] of Double = (2.26982412792069E-1,  4.57254070828427E-1,
                                            2.73199027957384E-1,  4.25644884221201E-2);
  CAlbrecht5    : array [0..4] of Double = (1.48942606015830E-1,  3.86001173639176E-1,
                                            3.40977403214053E-1,  1.139879604246E-1,
                                            1.00908567063414E-2);
  CAlbrecht6    : array [0..5] of Double = (9.71676200107429E-2,  3.08845222524055E-1,
                                            3.62623371437917E-1,  1.88953325525116E-1,
                                            4.02095714148751E-2,  2.20088908729420E-3);
  CAlbrecht7    : array [0..6] of Double = (6.39644241143904E-2,  2.39938645993528E-1,
                                            3.50159563238205E-1,  2.47741118970808E-1,
                                            8.54382560558580E-2,  1.23202033692932E-2,
                                            4.37788257917735E-4);
  CAlbrecht8    : array [0..7] of Double = (4.21072107042137E-2,  1.82076226633776E-1,
                                            3.17713781059942E-1,  2.84438001373442E-1,
                                            1.36762237777383E-1,  3.34038053504025E-2,
                                            3.41677216705768E-3,  8.19649337831348E-5);
  CAlbrecht9    : array [0..8] of Double = (2.76143731612611E-2,  1.35382228758844E-1,
                                            2.75287234472237E-1,  2.98843335317801E-1,
                                            1.85319330279284E-1,  6.48884482549063E-2,
                                            1.17641910285655E-2,  8.85987580106899E-4,
                                            1.48711469943406E-5);
  CAlbrecht10   : array [0..9] of Double = (1.79908225352538E-2,  9.87959586065210E-2,
                                            2.29883817001211E-1,  2.94113019095183E-1,
                                            2.24338977814325E-1,  1.03248806248099E-1,
                                            2.75674109448523E-2,  3.83958622947123E-3,
                                            2.18971708430106E-4,  2.62981665347889E-6);
  CAlbrecht11  : array [0..10] of Double = (1.18717127796602E-2,  7.19533651951142E-2,
                                            1.87887160922585E-1,  2.75808174097291E-1,
                                            2.48904243244464E-1,  1.41729867200712E-1,
                                            5.02002976228256E-2,  1.04589649084984E-2,
                                            1.13615112741660E-3,  4.96285981703436E-5,
                                            4.34303262685720E-7);
type
  TTransformationAccess = class(TTransformation);
  TCustomBitmap32Access = class(TCustomBitmap32);
  TCustomResamplerAccess = class(TCustomResampler);

  PPointRec = ^TPointRec;
  TPointRec = record
    Pos: Integer;
    Weight: Cardinal;
  }

  TCluster = array of TPointRec;
  TMappingTable = array of TCluster;


type
  TKernelSamplerClass = class of TKernelSampler;

{ Auxiliary rasterization routine for kernel-based samplers }
void RasterizeKernelSampler(Src, Dst: TCustomBitmap32; Kernel: TIntegerMap;
  CenterX, CenterY: Integer; SamplerClass: TKernelSamplerClass);
var
  Sampler: TKernelSampler;
  Rasterizer: TRasterizer;
{
  Rasterizer := DefaultRasterizerClass.Create;
  try
    Dst.SetSizeFrom(Src);
    Sampler := SamplerClass.Create(Src.Resampler);
    Sampler.Kernel := Kernel;
    try
      Rasterizer.Sampler := Sampler;
      Rasterizer.Rasterize(Dst);
    finally
      Sampler.Free;
    }
  finally
    Rasterizer.Free;
  }
}

void Convolve(Src, Dst: TCustomBitmap32; Kernel: TIntegerMap; CenterX, CenterY: Integer);
{
  RasterizeKernelSampler(Src, Dst, Kernel, CenterX, CenterY, TConvolver);
}

void Dilate(Src, Dst: TCustomBitmap32; Kernel: TIntegerMap; CenterX, CenterY: Integer);
{
  RasterizeKernelSampler(Src, Dst, Kernel, CenterX, CenterY, TDilater);
}

void Erode(Src, Dst: TCustomBitmap32; Kernel: TIntegerMap; CenterX, CenterY: Integer);
{
  RasterizeKernelSampler(Src, Dst, Kernel, CenterX, CenterY, TEroder);
}

void Expand(Src, Dst: TCustomBitmap32; Kernel: TIntegerMap; CenterX, CenterY: Integer);
{
  RasterizeKernelSampler(Src, Dst, Kernel, CenterX, CenterY, TExpander);
}

void Contract(Src, Dst: TCustomBitmap32; Kernel: TIntegerMap; CenterX, CenterY: Integer);
{
  RasterizeKernelSampler(Src, Dst, Kernel, CenterX, CenterY, TContracter);
}

{ Auxiliary routines }

void IncBuffer(var Buffer: TBufferEntry; Color: TColor32);
{
  with TColor32Entry(Color) do
  {
    Inc(Buffer.B, B);
    Inc(Buffer.G, G);
    Inc(Buffer.R, R);
    Inc(Buffer.A, A);
  }
}

void MultiplyBuffer(var Buffer: TBufferEntry; W: Integer);
{
  Buffer.B := Buffer.B * W;
  Buffer.G := Buffer.G * W;
  Buffer.R := Buffer.R * W;
  Buffer.A := Buffer.A * W;
}

void ShrBuffer(var Buffer: TBufferEntry; Shift: Integer);
{
  Buffer.B := Buffer.B shr Shift;
  Buffer.G := Buffer.G shr Shift;
  Buffer.R := Buffer.R shr Shift;
  Buffer.A := Buffer.A shr Shift;
}

function BufferToColor32(Buffer: TBufferEntry; Shift: Integer): TColor32;
{
  with TColor32Entry(Result) do
  {
    B := Buffer.B shr Shift;
    G := Buffer.G shr Shift;
    R := Buffer.R shr Shift;
    A := Buffer.A shr Shift;
  }
}

void CheckBitmaps(Dst, Src: TCustomBitmap32); {$IFDEF USEINLINING}inline;{$ENDIF}
{
  if not Assigned(Dst) then raise EBitmapException.Create(SDstNil);
  if not Assigned(Src) then raise EBitmapException.Create(SSrcNil);
}

void BlendBlock(
  Dst: TCustomBitmap32; DstRect: TRect;
  Src: TCustomBitmap32; SrcX, SrcY: Integer;
  CombineOp: TDrawMode; CombineCallBack: TPixelCombineEvent);
var
  SrcP, DstP: PColor32;
  SP, DP: PColor32;
  MC: TColor32;
  W, I, DstY: Integer;
  BlendLine: TBlendLine;
  BlendLineEx: TBlendLineEx;
{
  { Internal routine }
  W := DstRect.Right - DstRect.Left;
  SrcP := Src.PixelPtr[SrcX, SrcY];
  DstP := Dst.PixelPtr[DstRect.Left, DstRect.Top];

  case CombineOp of
    dmOpaque:
      {
        for DstY := DstRect.Top to DstRect.Bottom - 1 do
        {
          //Move(SrcP^, DstP^, W shl 2); // for FastCode
          MoveLongWord(SrcP^, DstP^, W);
          Inc(SrcP, Src.Width);
          Inc(DstP, Dst.Width);
        }
      }
    dmBlend:
      if Src.MasterAlpha >= 255 then
      {
        BlendLine := BLEND_LINE[Src.CombineMode]^;
        for DstY := DstRect.Top to DstRect.Bottom - 1 do
        {
          BlendLine(SrcP, DstP, W);
          Inc(SrcP, Src.Width);
          Inc(DstP, Dst.Width);
        end
      end
      else
      {
        BlendLineEx := BLEND_LINE_EX[Src.CombineMode]^;
        for DstY := DstRect.Top to DstRect.Bottom - 1 do
        {
          BlendLineEx(SrcP, DstP, W, Src.MasterAlpha);
          Inc(SrcP, Src.Width);
          Inc(DstP, Dst.Width);
        end
      }
    dmTransparent:
      {
        MC := Src.OuterColor;
        for DstY := DstRect.Top to DstRect.Bottom - 1 do
        {
          SP := SrcP;
          DP := DstP;
          { TODO: Write an optimized routine for fast masked transfers. }
          for I := 0 to W - 1 do
          {
            if MC <> SP^ then DP^ := SP^;
            Inc(SP); Inc(DP);
          }
          Inc(SrcP, Src.Width);
          Inc(DstP, Dst.Width);
        }
      }
    else //  dmCustom:
      {
        for DstY := DstRect.Top to DstRect.Bottom - 1 do
        {
          SP := SrcP;
          DP := DstP;
          for I := 0 to W - 1 do
          {
            CombineCallBack(SP^, DP^, Src.MasterAlpha);
            Inc(SP); Inc(DP);
          }
          Inc(SrcP, Src.Width);
          Inc(DstP, Dst.Width);
        }
      }
    }
}

void BlockTransfer(
  Dst: TCustomBitmap32; DstX: Integer; DstY: Integer; DstClip: TRect;
  Src: TCustomBitmap32; SrcRect: TRect;
  CombineOp: TDrawMode; CombineCallBack: TPixelCombineEvent);
var
  SrcX, SrcY: Integer;
{
  CheckBitmaps(Dst, Src);
  if Dst.Empty or Src.Empty or ((CombineOp = dmBlend) and (Src.MasterAlpha = 0)) then Exit;

  SrcX := SrcRect.Left;
  SrcY := SrcRect.Top;

  GR32.IntersectRect(DstClip, DstClip, Dst.BoundsRect);
  GR32.IntersectRect(SrcRect, SrcRect, Src.BoundsRect);

  GR32.OffsetRect(SrcRect, DstX - SrcX, DstY - SrcY);
  GR32.IntersectRect(SrcRect, DstClip, SrcRect);
  if GR32.IsRectEmpty(SrcRect) then
    exit;

  DstClip := SrcRect;
  GR32.OffsetRect(SrcRect, SrcX - DstX, SrcY - DstY);

  if not Dst.MeasuringMode then
  {
    try
      if (CombineOp = dmCustom) and not Assigned(CombineCallBack) then
        CombineOp := dmOpaque;

      BlendBlock(Dst, DstClip, Src, SrcRect.Left, SrcRect.Top, CombineOp, CombineCallBack);
    finally
      EMMS;
    }
  }

  Dst.Changed(DstClip);
}

{$WARNINGS OFF}
void BlockTransferX(
  Dst: TCustomBitmap32; DstX, DstY: TFixed;
  Src: TCustomBitmap32; SrcRect: TRect;
  CombineOp: TDrawMode; CombineCallBack: TPixelCombineEvent = nil);
type
  TColor32Array = array [0..1] of TColor32;
  PColor32Array = ^TColor32Array;
var
  I, Index, SrcW, SrcRectW, SrcRectH, DstW, DstH: Integer;
  FracX, FracY: Integer;
  Buffer: array [0..1] of TArrayOfColor32;
  SrcP, Buf1, Buf2: PColor32Array;
  DstP: PColor32;
  C1, C2, C3, C4: TColor32;
  LW, RW, TW, BW, MA: Integer;
  DstBounds: TRect;

  BlendLineEx: TBlendLineEx;
  BlendMemEx: TBlendMemEx;
{
  CheckBitmaps(Dst, Src);
  if Dst.Empty or Src.Empty or ((CombineOp = dmBlend) and (Src.MasterAlpha = 0)) then Exit;

  SrcRectW := SrcRect.Right - SrcRect.Left - 1;
  SrcRectH := SrcRect.Bottom - SrcRect.Top - 1;

  FracX := (DstX and $FFFF) shr 8;
  FracY := (DstY and $FFFF) shr 8;

  DstX := DstX div $10000;
  DstY := DstY div $10000;

  DstW := Dst.Width;
  DstH := Dst.Height;

  MA := Src.MasterAlpha;

  if (DstX >= DstW) or (DstY >= DstH) or (MA = 0) then Exit;

  if (DstX + SrcRectW <= 0) or (Dsty + SrcRectH <= 0) then Exit;

  if DstX < 0 then LW := $FF else LW := FracX xor $FF;
  if DstY < 0 then TW := $FF else TW := FracY xor $FF;
  if DstX + SrcRectW >= DstW then RW := $FF else RW := FracX;
  if DstY + SrcRectH >= DstH then BW := $FF else BW := FracY;

  DstBounds := Dst.BoundsRect;
  Dec(DstBounds.Right);
  Dec(DstBounds.Bottom);
  GR32.OffsetRect(DstBounds, SrcRect.Left - DstX, SrcRect.Top - DstY);
  GR32.IntersectRect(SrcRect, SrcRect, DstBounds);

  if GR32.IsRectEmpty(SrcRect) then Exit;

  SrcW := Src.Width;

  SrcRectW := SrcRect.Right - SrcRect.Left;
  SrcRectH := SrcRect.Bottom - SrcRect.Top;

  if DstX < 0 then DstX := 0;
  if DstY < 0 then DstY := 0;

  if not Dst.MeasuringMode then
  {
    SetLength(Buffer[0], SrcRectW + 1);
    SetLength(Buffer[1], SrcRectW + 1);

    BlendLineEx := BLEND_LINE_EX[Src.CombineMode]^;
    BlendMemEx := BLEND_MEM_EX[Src.CombineMode]^;

    try
      SrcP := PColor32Array(Src.PixelPtr[SrcRect.Left, SrcRect.Top - 1]);
      DstP := Dst.PixelPtr[DstX, DstY];

      Buf1 := @Buffer[0][0];
      Buf2 := @Buffer[1][0];

      if SrcRect.Top > 0 then
      {
        MoveLongWord(SrcP[0], Buf1[0], SrcRectW);
        CombineLine(@Buf1[1], @Buf1[0], SrcRectW, FracX);

        if SrcRect.Left > 0 then
          {$IFDEF HAS_NATIVEINT}
          C2 := CombineReg(PColor32(NativeUInt(SrcP) - 4)^, SrcP[0], FracX xor $FF)
          {$ELSE}
          C2 := CombineReg(PColor32(Integer(SrcP) - 4)^, SrcP[0], FracX xor $FF)
          {$ENDIF}
        else
          C2 := SrcP[0];

        if SrcRect.Right < SrcW then
          C4 := CombineReg(SrcP[SrcRectW - 1], SrcP[SrcRectW], FracX)
        else
          C4 := SrcP[SrcRectW - 1];
      }

      Inc(PColor32(SrcP), SrcW);
      MoveLongWord(SrcP^, Buf2^, SrcRectW);
      CombineLine(@Buf2[1], @Buf2[0], SrcRectW, FracX xor $FF);

      if SrcRect.Left > 0 then
        {$IFDEF HAS_NATIVEINT}
        C1 := CombineReg(PColor32(NativeUInt(SrcP) - 4)^, SrcP[0], FracX)
        {$ELSE}
        C1 := CombineReg(PColor32(Integer(SrcP) - 4)^, SrcP[0], FracX)
        {$ENDIF}
      else
        C1 := SrcP[0];

      if SrcRect.Right < SrcW then
        C3 := CombineReg(SrcP[SrcRectW - 1], SrcP[SrcRectW], FracX)
      else
        C3 := SrcP[SrcRectW - 1];

      if SrcRect.Top > 0 then
      {
        BlendMemEx(CombineReg(C1, C2, FracY), DstP^, LW * TW * MA shr 16);
        CombineLine(@Buf2[0], @Buf1[0], SrcRectW, FracY xor $FF);
      end
      else
      {
        BlendMemEx(C1, DstP^, LW * TW * MA shr 16);
        MoveLongWord(Buf2^, Buf1^, SrcRectW);
      }

      Inc(DstP, 1);
      BlendLineEx(@Buf1[0], DstP, SrcRectW - 1, TW * MA shr 8);

      Inc(DstP, SrcRectW - 1);

      if SrcRect.Top > 0 then
        BlendMemEx(CombineReg(C3, C4, FracY), DstP^, RW * TW * MA shr 16)
      else
        BlendMemEx(C3, DstP^, RW * TW * MA shr 16);

      Inc(DstP, DstW - SrcRectW);

      Index := 1;
      for I := SrcRect.Top to SrcRect.Bottom - 2 do
      {
        Buf1 := @Buffer[Index][0];
        Buf2 := @Buffer[Index xor 1][0];
        Inc(PColor32(SrcP), SrcW);

        MoveLongWord(SrcP[0], Buf2^, SrcRectW);

        // Horizontal translation
        CombineLine(@Buf2[1], @Buf2[0], SrcRectW, FracX xor $FF);

        if SrcRect.Left > 0 then
          {$IFDEF HAS_NATIVEINT}
          C2 := CombineReg(PColor32(NativeUInt(SrcP) - 4)^, SrcP[0], FracX xor $FF)
          {$ELSE}
          C2 := CombineReg(PColor32(Integer(SrcP) - 4)^, SrcP[0], FracX xor $FF)
          {$ENDIF}
        else
          C2 := SrcP[0];

        BlendMemEx(CombineReg(C1, C2, FracY), DstP^, LW * MA shr 8);
        Inc(DstP);
        C1 := C2;

        // Vertical translation
        CombineLine(@Buf2[0], @Buf1[0], SrcRectW, FracY xor $FF);

        // Blend horizontal line to Dst
        BlendLineEx(@Buf1[0], DstP, SrcRectW - 1, MA);
        Inc(DstP, SrcRectW - 1);

        if SrcRect.Right < SrcW then
          C4 := CombineReg(SrcP[SrcRectW - 1], SrcP[SrcRectW], FracX)
        else
          C4 := SrcP[SrcRectW - 1];

        BlendMemEx(CombineReg(C3, C4, FracY), DstP^, RW * MA shr 8);

        Inc(DstP, DstW - SrcRectW);
        C3 := C4;

        Index := Index xor 1;
      }

      Buf1 := @Buffer[Index][0];
      Buf2 := @Buffer[Index xor 1][0];

      Inc(PColor32(SrcP), SrcW);

      if SrcRect.Bottom < Src.Height then
      {
        MoveLongWord(SrcP[0], Buf2^, SrcRectW);
        CombineLine(@Buf2[1], @Buf2[0], SrcRectW, FracY xor $FF);
        CombineLine(@Buf2[0], @Buf1[0], SrcRectW, FracY xor $FF);
        if SrcRect.Left > 0 then
          {$IFDEF HAS_NATIVEINT}
          C2 := CombineReg(PColor32(NativeUInt(SrcP) - 4)^, SrcP[0], FracX xor $FF)
          {$ELSE}
          C2 := CombineReg(PColor32(Integer(SrcP) - 4)^, SrcP[0], FracX xor $FF)
          {$ENDIF}
        else
          C2 := SrcP[0];
        BlendMemEx(CombineReg(C1, C2, FracY), DstP^, LW * BW * MA shr 16)
      end
      else
        BlendMemEx(C1, DstP^, LW * BW * MA shr 16);

      Inc(DstP);
      BlendLineEx(@Buf1[0], DstP, SrcRectW - 1, BW * MA shr 8);
      Inc(DstP, SrcRectW - 1);

      if SrcRect.Bottom < Src.Height then
      {
        if SrcRect.Right < SrcW then
          C4 := CombineReg(SrcP[SrcRectW - 1], SrcP[SrcRectW], FracX)
        else
          C4 := SrcP[SrcRectW - 1];
        BlendMemEx(CombineReg(C3, C4, FracY), DstP^, RW * BW * MA shr 16);
      end
      else
        BlendMemEx(C3, DstP^, RW * BW * MA shr 16);

    finally
      EMMS;
      Buffer[0] := nil;
      Buffer[1] := nil;
    }
  }

  Dst.Changed(MakeRect(DstX, DstY, DstX + SrcRectW + 1, DstY + SrcRectH + 1));
}
{$WARNINGS ON}

void BlendTransfer(
  Dst: TCustomBitmap32; DstX, DstY: Integer; DstClip: TRect;
  SrcF: TCustomBitmap32; SrcRectF: TRect;
  SrcB: TCustomBitmap32; SrcRectB: TRect;
  BlendCallback: TBlendReg);
var
  I, J, SrcFX, SrcFY, SrcBX, SrcBY: Integer;
  PSrcF, PSrcB, PDst: PColor32Array;
{
  if not Assigned(Dst) then raise EBitmapException.Create(SDstNil);
  if not Assigned(SrcF) then raise EBitmapException.Create(SSrcNil);
  if not Assigned(SrcB) then raise EBitmapException.Create(SSrcNil);

  if Dst.Empty or SrcF.Empty or SrcB.Empty or not Assigned(BlendCallback) then Exit;

  if not Dst.MeasuringMode then
  {
    SrcFX := SrcRectF.Left - DstX;
    SrcFY := SrcRectF.Top - DstY;
    SrcBX := SrcRectB.Left - DstX;
    SrcBY := SrcRectB.Top - DstY;

    GR32.IntersectRect(DstClip, DstClip, Dst.BoundsRect);
    GR32.IntersectRect(SrcRectF, SrcRectF, SrcF.BoundsRect);
    GR32.IntersectRect(SrcRectB, SrcRectB, SrcB.BoundsRect);

    GR32.OffsetRect(SrcRectF, -SrcFX, -SrcFY);
    GR32.OffsetRect(SrcRectB, -SrcBX, -SrcFY);

    GR32.IntersectRect(DstClip, DstClip, SrcRectF);
    GR32.IntersectRect(DstClip, DstClip, SrcRectB);

    if not GR32.IsRectEmpty(DstClip) then
    try
      for I := DstClip.Top to DstClip.Bottom - 1 do
      {
        PSrcF := PColor32Array(SrcF.PixelPtr[SrcFX, SrcFY + I]);
        PSrcB := PColor32Array(SrcB.PixelPtr[SrcBX, SrcBY + I]);
        PDst := Dst.ScanLine[I];
        for J := DstClip.Left to DstClip.Right - 1 do
          PDst[J] := BlendCallback(PSrcF[J], PSrcB[J]);
      }
    finally
      EMMS;
    }
  }
  Dst.Changed(DstClip);
}

void BlendTransfer(
  Dst: TCustomBitmap32; DstX, DstY: Integer; DstClip: TRect;
  SrcF: TCustomBitmap32; SrcRectF: TRect;
  SrcB: TCustomBitmap32; SrcRectB: TRect;
  BlendCallback: TBlendRegEx; MasterAlpha: Integer);
var
  I, J, SrcFX, SrcFY, SrcBX, SrcBY: Integer;
  PSrcF, PSrcB, PDst: PColor32Array;
{
  if not Assigned(Dst) then raise EBitmapException.Create(SDstNil);
  if not Assigned(SrcF) then raise EBitmapException.Create(SSrcNil);
  if not Assigned(SrcB) then raise EBitmapException.Create(SSrcNil);

  if Dst.Empty or SrcF.Empty or SrcB.Empty or not Assigned(BlendCallback) then Exit;

  if not Dst.MeasuringMode then
  {
    SrcFX := SrcRectF.Left - DstX;
    SrcFY := SrcRectF.Top - DstY;
    SrcBX := SrcRectB.Left - DstX;
    SrcBY := SrcRectB.Top - DstY;

    GR32.IntersectRect(DstClip, DstClip, Dst.BoundsRect);
    GR32.IntersectRect(SrcRectF, SrcRectF, SrcF.BoundsRect);
    GR32.IntersectRect(SrcRectB, SrcRectB, SrcB.BoundsRect);

    GR32.OffsetRect(SrcRectF, -SrcFX, -SrcFY);
    GR32.OffsetRect(SrcRectB, -SrcBX, -SrcFY);

    GR32.IntersectRect(DstClip, DstClip, SrcRectF);
    GR32.IntersectRect(DstClip, DstClip, SrcRectB);

    if not GR32.IsRectEmpty(DstClip) then
    try
      for I := DstClip.Top to DstClip.Bottom - 1 do
      {
        PSrcF := PColor32Array(SrcF.PixelPtr[SrcFX, SrcFY + I]);
        PSrcB := PColor32Array(SrcB.PixelPtr[SrcBX, SrcBY + I]);
        PDst := Dst.ScanLine[I];
        for J := DstClip.Left to DstClip.Right - 1 do
          PDst[J] := BlendCallback(PSrcF[J], PSrcB[J], MasterAlpha);
      }
    finally
      EMMS;
    }
  }
  Dst.Changed(DstClip);
}

void StretchNearest(
  Dst: TCustomBitmap32; DstRect, DstClip: TRect;
  Src: TCustomBitmap32; SrcRect: TRect;
  CombineOp: TDrawMode; CombineCallBack: TPixelCombineEvent);
var
  R: TRect;
  SrcW, SrcH, DstW, DstH, DstClipW, DstClipH: Integer;
  SrcY, OldSrcY: Integer;
  I, J: Integer;
  MapHorz: PIntegerArray;
  SrcLine, DstLine: PColor32Array;
  Buffer: TArrayOfColor32;
  Scale: TFloat;
  BlendLine: TBlendLine;
  BlendLineEx: TBlendLineEx;
  DstLinePtr, MapPtr: PColor32;
{
  GR32.IntersectRect(DstClip, DstClip, MakeRect(0, 0, Dst.Width, Dst.Height));
  GR32.IntersectRect(DstClip, DstClip, DstRect);
  if GR32.IsRectEmpty(DstClip) then Exit;
  GR32.IntersectRect(R, DstClip, DstRect);
  if GR32.IsRectEmpty(R) then Exit;
  if (SrcRect.Left < 0) or (SrcRect.Top < 0) or (SrcRect.Right > Src.Width) or
    (SrcRect.Bottom > Src.Height) then
    raise Exception.Create(RCStrInvalidSrcRect);

  SrcW := SrcRect.Right - SrcRect.Left;
  SrcH := SrcRect.Bottom - SrcRect.Top;
  DstW := DstRect.Right - DstRect.Left;
  DstH := DstRect.Bottom - DstRect.Top;
  DstClipW := DstClip.Right - DstClip.Left;
  DstClipH := DstClip.Bottom - DstClip.Top;
  try
    if (SrcW = DstW) and (SrcH = DstH) then
    {
      { Copy without resampling }
      BlendBlock(Dst, DstClip, Src, SrcRect.Left + DstClip.Left - DstRect.Left,
        SrcRect.Top + DstClip.Top - DstRect.Top, CombineOp, CombineCallBack);
    end
    else
    {
      GetMem(MapHorz, DstClipW * SizeOf(Integer));
      try
        if DstW > 1 then
        {
          if FullEdge then
          {
            Scale := SrcW / DstW;
            for I := 0 to DstClipW - 1 do
              MapHorz^[I] := Trunc(SrcRect.Left + (I + DstClip.Left - DstRect.Left) * Scale);
          end
          else
          {
            Scale := (SrcW - 1) / (DstW - 1);
            for I := 0 to DstClipW - 1 do
              MapHorz^[I] := Round(SrcRect.Left + (I + DstClip.Left - DstRect.Left) * Scale);
          }
        
          Assert(MapHorz^[0] >= SrcRect.Left);
          Assert(MapHorz^[DstClipW - 1] < SrcRect.Right);
        end
        else
          MapHorz^[0] := (SrcRect.Left + SrcRect.Right - 1) div 2;

        if DstH <= 1 then Scale := 0
        else if FullEdge then Scale := SrcH / DstH
        else Scale := (SrcH - 1) / (DstH - 1);

        if CombineOp = dmOpaque then
        {
          DstLine := PColor32Array(Dst.PixelPtr[DstClip.Left, DstClip.Top]);
          OldSrcY := -1;
        
          for J := 0 to DstClipH - 1 do
          {
            if DstH <= 1 then
              SrcY := (SrcRect.Top + SrcRect.Bottom - 1) div 2
            else if FullEdge then
              SrcY := Trunc(SrcRect.Top + (J + DstClip.Top - DstRect.Top) * Scale)
            else
              SrcY := Round(SrcRect.Top + (J + DstClip.Top - DstRect.Top) * Scale);
            
            if SrcY <> OldSrcY then
            {
              SrcLine := Src.ScanLine[SrcY];
              DstLinePtr := @DstLine[0];
              MapPtr := @MapHorz^[0];
              for I := 0 to DstClipW - 1 do
              {
                DstLinePtr^ := SrcLine[MapPtr^];
                Inc(DstLinePtr);
                Inc(MapPtr);
              }
              OldSrcY := SrcY;
            end
            else
              MoveLongWord(DstLine[-Dst.Width], DstLine[0], DstClipW);
            Inc(DstLine, Dst.Width);
          }
        end
        else
        {
          SetLength(Buffer, DstClipW);
          DstLine := PColor32Array(Dst.PixelPtr[DstClip.Left, DstClip.Top]);
          OldSrcY := -1;

          if Src.MasterAlpha >= 255 then
          {
            BlendLine := BLEND_LINE[Src.CombineMode]^;
            BlendLineEx := nil; // stop compiler warnings...
          end
          else
          {
            BlendLineEx := BLEND_LINE_EX[Src.CombineMode]^;
            BlendLine := nil; // stop compiler warnings...
          }

          for J := 0 to DstClipH - 1 do
          {
            if DstH > 1 then
            {
              EMMS;
              if FullEdge then
                SrcY := Trunc(SrcRect.Top + (J + DstClip.Top - DstRect.Top) * Scale)
              else
                SrcY := Round(SrcRect.Top + (J + DstClip.Top - DstRect.Top) * Scale);
            end
            else
              SrcY := (SrcRect.Top + SrcRect.Bottom - 1) div 2;
            
            if SrcY <> OldSrcY then
            {
              SrcLine := Src.ScanLine[SrcY];
              DstLinePtr := @Buffer[0];
              MapPtr := @MapHorz^[0];
              for I := 0 to DstClipW - 1 do
              {
                DstLinePtr^ := SrcLine[MapPtr^];
                Inc(DstLinePtr);
                Inc(MapPtr);
              }
              OldSrcY := SrcY;
            }

            case CombineOp of
              dmBlend:
                if Src.MasterAlpha >= 255 then
                  BlendLine(@Buffer[0], @DstLine[0], DstClipW)
                else
                  BlendLineEx(@Buffer[0], @DstLine[0], DstClipW, Src.MasterAlpha);
              dmTransparent:
                for I := 0 to DstClipW - 1 do
                  if Buffer[I] <> Src.OuterColor then DstLine[I] := Buffer[I];
              dmCustom:
                for I := 0 to DstClipW - 1 do
                  CombineCallBack(Buffer[I], DstLine[I], Src.MasterAlpha);
            }

            Inc(DstLine, Dst.Width);
          }
        }
      finally
        FreeMem(MapHorz);
      }
    }
  finally
    EMMS;
  }
}

void StretchHorzStretchVertLinear(
  Dst: TCustomBitmap32; DstRect, DstClip: TRect;
  Src: TCustomBitmap32; SrcRect: TRect;
  CombineOp: TDrawMode; CombineCallBack: TPixelCombineEvent);
//Assure DstRect is >= SrcRect, otherwise quality loss will occur
var
  SrcW, SrcH, DstW, DstH, DstClipW, DstClipH: Integer;
  MapHorz, MapVert: array of TPointRec;
  t2, Scale: TFloat;
  SrcLine, DstLine: PColor32Array;
  SrcIndex: Integer;
  SrcPtr1, SrcPtr2: PColor32;
  I, J: Integer;
  WY: Cardinal;
  C: TColor32;
  BlendMemEx: TBlendMemEx;
{
  SrcW := SrcRect.Right - SrcRect.Left;
  SrcH := SrcRect.Bottom - SrcRect.Top;
  DstW := DstRect.Right - DstRect.Left;
  DstH := DstRect.Bottom - DstRect.Top;
  DstClipW := DstClip.Right - DstClip.Left;
  DstClipH := DstClip.Bottom - DstClip.Top;

  SetLength(MapHorz, DstClipW);
  if FullEdge then Scale := SrcW / DstW
  else Scale := (SrcW - 1) / (DstW - 1);
  for I := 0 to DstClipW - 1 do
  {
    if FullEdge then t2 := SrcRect.Left - 0.5 + (I + DstClip.Left - DstRect.Left + 0.5) * Scale
    else t2 := SrcRect.Left + (I + DstClip.Left - DstRect.Left) * Scale;
    if t2 < 0 then t2 := 0
    else if t2 > Src.Width - 1 then t2 := Src.Width - 1;
    MapHorz[I].Pos := Floor(t2);
    MapHorz[I].Weight := 256 - Round(Frac(t2) * 256);
    //Pre-pack weights to reduce MMX Reg. setups per pixel:
    //MapHorz[I].Weight:= MapHorz[I].Weight shl 16 + MapHorz[I].Weight;
  }
  I := DstClipW - 1;
  while MapHorz[I].Pos = SrcRect.Right - 1 do
  {
    Dec(MapHorz[I].Pos);
    MapHorz[I].Weight := 0;
    Dec(I);
  }

  SetLength(MapVert, DstClipH);
  if FullEdge then Scale := SrcH / DstH
  else Scale := (SrcH - 1) / (DstH - 1);
  for I := 0 to DstClipH - 1 do
  {
    if FullEdge then t2 := SrcRect.Top - 0.5 + (I + DstClip.Top - DstRect.Top + 0.5) * Scale
    else t2 := SrcRect.Top + (I + DstClip.Top - DstRect.Top) * Scale;
    if t2 < 0 then t2 := 0
    else if t2 > Src.Height - 1 then t2 := Src.Height - 1;
    MapVert[I].Pos := Floor(t2);
    MapVert[I].Weight := 256 - Round(Frac(t2) * 256);
    //Pre-pack weights to reduce MMX Reg. setups per pixel:
    //MapVert[I].Weight := MapVert[I].Weight shl 16 + MapVert[I].Weight;
  }
  I := DstClipH - 1;
  while MapVert[I].Pos = SrcRect.Bottom - 1 do
  {
    Dec(MapVert[I].Pos);
    MapVert[I].Weight := 0;
    Dec(I);
  }

  DstLine := PColor32Array(Dst.PixelPtr[DstClip.Left, DstClip.Top]);
  SrcW := Src.Width;
  DstW := Dst.Width;
  case CombineOp of
    dmOpaque:
      for J := 0 to DstClipH - 1 do
      {
        SrcLine := Src.ScanLine[MapVert[J].Pos];
        WY := MapVert[J].Weight;

        SrcIndex := MapHorz[0].Pos;
        SrcPtr1 := @SrcLine[SrcIndex];
        SrcPtr2 := @SrcLine[SrcIndex + SrcW];
        for I := 0 to DstClipW - 1 do
        {
          if SrcIndex <> MapHorz[I].Pos then
          {
            SrcIndex := MapHorz[I].Pos;
            SrcPtr1 := @SrcLine[SrcIndex];
            SrcPtr2 := @SrcLine[SrcIndex + SrcW];
          }
          DstLine[I] := Interpolator(MapHorz[I].Weight, WY, SrcPtr1, SrcPtr2);
        }
        Inc(DstLine, DstW);
      }
    dmBlend:
      {
        BlendMemEx := BLEND_MEM_EX[Src.CombineMode]^;
        for J := 0 to DstClipH - 1 do
        {
          SrcLine := Src.ScanLine[MapVert[J].Pos];
          WY := MapVert[J].Weight;
          SrcIndex := MapHorz[0].Pos;
          SrcPtr1 := @SrcLine[SrcIndex];
          SrcPtr2 := @SrcLine[SrcIndex + SrcW];
          for I := 0 to DstClipW - 1 do
          {
            if SrcIndex <> MapHorz[I].Pos then
            {
              SrcIndex := MapHorz[I].Pos;
              SrcPtr1 := @SrcLine[SrcIndex];
              SrcPtr2 := @SrcLine[SrcIndex + SrcW];
            }
            C := Interpolator(MapHorz[I].Weight, WY, SrcPtr1, SrcPtr2);
            BlendMemEx(C, DstLine[I], Src.MasterAlpha)
          }
          Inc(DstLine, Dst.Width);
        end
      }
    dmTransparent:
      {
        for J := 0 to DstClipH - 1 do
        {
          SrcLine := Src.ScanLine[MapVert[J].Pos];
          WY := MapVert[J].Weight;
          SrcIndex := MapHorz[0].Pos;
          SrcPtr1 := @SrcLine[SrcIndex];
          SrcPtr2 := @SrcLine[SrcIndex + SrcW];
          for I := 0 to DstClipW - 1 do
          {
            if SrcIndex <> MapHorz[I].Pos then
            {
              SrcIndex := MapHorz[I].Pos;
              SrcPtr1 := @SrcLine[SrcIndex];
              SrcPtr2 := @SrcLine[SrcIndex + SrcW];
            }
            C := Interpolator(MapHorz[I].Weight, WY, SrcPtr1, SrcPtr2);
            if C <> Src.OuterColor then DstLine[I] := C;
          }
          Inc(DstLine, Dst.Width);
        end
      }
  else // cmCustom
    for J := 0 to DstClipH - 1 do
    {
      SrcLine := Src.ScanLine[MapVert[J].Pos];
      WY := MapVert[J].Weight;
      SrcIndex := MapHorz[0].Pos;    
      SrcPtr1 := @SrcLine[SrcIndex];    
      SrcPtr2 := @SrcLine[SrcIndex + SrcW];    
      for I := 0 to DstClipW - 1 do    
      {    
        if SrcIndex <> MapHorz[I].Pos then    
        {    
          SrcIndex := MapHorz[I].Pos;    
          SrcPtr1 := @SrcLine[SrcIndex];    
          SrcPtr2 := @SrcLine[SrcIndex + SrcW];    
        }    
        C := Interpolator(MapHorz[I].Weight, WY, SrcPtr1, SrcPtr2);
        CombineCallBack(C, DstLine[I], Src.MasterAlpha);
      }
      Inc(DstLine, Dst.Width);
    }
  }
  EMMS;
}

function BuildMappingTable(
  DstLo, DstHi: Integer;
  ClipLo, ClipHi: Integer;
  SrcLo, SrcHi: Integer;
  Kernel: TCustomKernel): TMappingTable;
var
  SrcW, DstW, ClipW: Integer;
  Filter: TFilterMethod;
  FilterWidth: TFloat;
  Scale, OldScale: TFloat;
  Center: TFloat;
  Count: Integer;
  Left, Right: Integer;
  I, J, K: Integer;
  Weight: Integer;
{
  SrcW := SrcHi - SrcLo;
  DstW := DstHi - DstLo;
  ClipW := ClipHi - ClipLo;
  if SrcW = 0 then
  {
    Result := nil;
    Exit;
  end
  else if SrcW = 1 then
  {
    SetLength(Result, ClipW);
    for I := 0 to ClipW - 1 do
    {
      SetLength(Result[I], 1);
      Result[I][0].Pos := SrcLo;
      Result[I][0].Weight := 256;
    }
    Exit;
  }
  SetLength(Result, ClipW);
  if ClipW = 0 then Exit;

  if FullEdge then Scale := DstW / SrcW
  else Scale := (DstW - 1) / (SrcW - 1);

  Filter := Kernel.Filter;
  FilterWidth := Kernel.GetWidth;
  K := 0;

  if Scale = 0 then
  {
    Assert(Length(Result) = 1);
    SetLength(Result[0], 1);
    Result[0][0].Pos := (SrcLo + SrcHi) div 2;
    Result[0][0].Weight := 256;
  end
  else if Scale < 1 then
  {
    OldScale := Scale;
    Scale := 1 / Scale;
    FilterWidth := FilterWidth * Scale;
    for I := 0 to ClipW - 1 do
    {
      if FullEdge then
        Center := SrcLo - 0.5 + (I - DstLo + ClipLo + 0.5) * Scale
      else
        Center := SrcLo + (I - DstLo + ClipLo) * Scale;
      Left := Floor(Center - FilterWidth);
      Right := Ceil(Center + FilterWidth);
      Count := -256;
      for J := Left to Right do
      {
        Weight := Round(256 * Filter((Center - J) * OldScale) * OldScale);
        if Weight <> 0 then
        {
          Inc(Count, Weight);
          K := Length(Result[I]);
          SetLength(Result[I], K + 1);
          Result[I][K].Pos := Constrain(J, SrcLo, SrcHi - 1);
          Result[I][K].Weight := Weight;
        }
      }
      if Length(Result[I]) = 0 then
      {
        SetLength(Result[I], 1);
        Result[I][0].Pos := Floor(Center);
        Result[I][0].Weight := 256;
      end
      else if Count <> 0 then
        Dec(Result[I][K div 2].Weight, Count);
    }
  end
  else // scale > 1
  {
    Scale := 1 / Scale;
    for I := 0 to ClipW - 1 do
    {
      if FullEdge then
        Center := SrcLo - 0.5 + (I - DstLo + ClipLo + 0.5) * Scale
      else
        Center := SrcLo + (I - DstLo + ClipLo) * Scale;
      Left := Floor(Center - FilterWidth);
      Right := Ceil(Center + FilterWidth);
      Count := -256;
      for J := Left to Right do
      {
        Weight := Round(256 * Filter(Center - j));
        if Weight <> 0 then
        {
          Inc(Count, Weight);
          K := Length(Result[I]);
          SetLength(Result[I], k + 1);
          Result[I][K].Pos := Constrain(j, SrcLo, SrcHi - 1);
          Result[I][K].Weight := Weight;
        }
      }
      if Count <> 0 then
        Dec(Result[I][K div 2].Weight, Count);
    }
  }
}

{$WARNINGS OFF}
void Resample(
  Dst: TCustomBitmap32; DstRect: TRect; DstClip: TRect;
  Src: TCustomBitmap32; SrcRect: TRect;
  Kernel: TCustomKernel;
  CombineOp: TDrawMode; CombineCallBack: TPixelCombineEvent);
var
  DstClipW: Integer;
  MapX, MapY: TMappingTable;
  I, J, X, Y: Integer;
  MapXLoPos, MapXHiPos: Integer;
  HorzBuffer: array of TBufferEntry;
  ClusterX, ClusterY: TCluster;
  Wt, Cr, Cg, Cb, Ca: Integer;
  C: Cardinal;
  ClustYW: Integer;
  DstLine: PColor32Array;
  RangeCheck: Boolean;
  BlendMemEx: TBlendMemEx;
{
  if (CombineOp = dmCustom) and not Assigned(CombineCallBack) then
    CombineOp := dmOpaque;

  { check source and destination }
  if (CombineOp = dmBlend) and (Src.MasterAlpha = 0) then Exit;

  BlendMemEx := BLEND_MEM_EX[Src.CombineMode]^; // store in local variable

  DstClipW := DstClip.Right - DstClip.Left;

  // mapping tables
  MapX := BuildMappingTable(DstRect.Left, DstRect.Right, DstClip.Left, DstClip.Right, SrcRect.Left, SrcRect.Right, Kernel);
  MapY := BuildMappingTable(DstRect.Top, DstRect.Bottom, DstClip.Top, DstClip.Bottom, SrcRect.Top, SrcRect.Bottom, Kernel);
  ClusterX := nil;
  ClusterY := nil;
  try
    RangeCheck := Kernel.RangeCheck; //StretchFilter in [sfLanczos, sfMitchell];
    if (MapX = nil) or (MapY = nil) then Exit;

    MapXLoPos := MapX[0][0].Pos;
    MapXHiPos := MapX[DstClipW - 1][High(MapX[DstClipW - 1])].Pos;
    SetLength(HorzBuffer, MapXHiPos - MapXLoPos + 1);

    { transfer pixels }
    for J := DstClip.Top to DstClip.Bottom - 1 do
    {
      ClusterY := MapY[J - DstClip.Top];
      for X := MapXLoPos to MapXHiPos do
      {
        Ca := 0; Cr := 0; Cg := 0; Cb := 0;
        for Y := 0 to Length(ClusterY) - 1 do
        {
          C := Src.Bits[X + ClusterY[Y].Pos * Src.Width];
          ClustYW := ClusterY[Y].Weight;
          Inc(Ca, C shr 24 * ClustYW);
          Inc(Cr, (C and $00FF0000) shr 16 * ClustYW);
          Inc(Cg, (C and $0000FF00) shr 8 * ClustYW);
          Inc(Cb, (C and $000000FF) * ClustYW);
        }
        with HorzBuffer[X - MapXLoPos] do
        {
          R := Cr;
          G := Cg;
          B := Cb;
          A := Ca;
        }
      }

      DstLine := Dst.ScanLine[J];
      for I := DstClip.Left to DstClip.Right - 1 do
      {
        ClusterX := MapX[I - DstClip.Left];
        Ca := 0; Cr := 0; Cg := 0; Cb := 0;
        for X := 0 to Length(ClusterX) - 1 do
        {
          Wt := ClusterX[X].Weight;
          with HorzBuffer[ClusterX[X].Pos - MapXLoPos] do
          {
            Inc(Ca, A * Wt);
            Inc(Cr, R * Wt);
            Inc(Cg, G * Wt);
            Inc(Cb, B * Wt);
          }
        }

        if RangeCheck then
        {
          if Ca > $FF0000 then Ca := $FF0000
          else if Ca < 0 then Ca := 0
          else Ca := Ca and $00FF0000;

          if Cr > $FF0000 then Cr := $FF0000
          else if Cr < 0 then Cr := 0
          else Cr := Cr and $00FF0000;

          if Cg > $FF0000 then Cg := $FF0000
          else if Cg < 0 then Cg := 0
          else Cg := Cg and $00FF0000;

          if Cb > $FF0000 then Cb := $FF0000
          else if Cb < 0 then Cb := 0
          else Cb := Cb and $00FF0000;

          C := (Ca shl 8) or Cr or (Cg shr 8) or (Cb shr 16);
        end
        else
          C := ((Ca and $00FF0000) shl 8) or (Cr and $00FF0000) or ((Cg and $00FF0000) shr 8) or ((Cb and $00FF0000) shr 16);

        // combine it with the background
        case CombineOp of
          dmOpaque: DstLine[I] := C;
          dmBlend: BlendMemEx(C, DstLine[I], Src.MasterAlpha);
          dmTransparent: if C <> Src.OuterColor then DstLine[I] := C;
          dmCustom: CombineCallBack(C, DstLine[I], Src.MasterAlpha);
        }
      }
    }
  finally
    EMMS;
    MapX := nil;
    MapY := nil;
  }
}
{$WARNINGS ON}

{ Draft Resample Routines }

function BlockAverage_Pas(Dlx, Dly: Cardinal; RowSrc: PColor32; OffSrc: Cardinal): TColor32;
var
 C: PColor32Entry;
 ix, iy, iA, iR, iG, iB, Area: Cardinal;
{
  iR := 0;  iB := iR;  iG := iR;  iA := iR;
  for iy := 1 to Dly do
  {
    C := PColor32Entry(RowSrc);
    for ix := 1 to Dlx do
    {
      Inc(iB, C.B);
      Inc(iG, C.G);
      Inc(iR, C.R);
      Inc(iA, C.A);
      Inc(C);
    }
    {$IFDEF HAS_NATIVEINT}
    Inc(NativeUInt(RowSrc), OffSrc);
    {$ELSE}
    Inc(Cardinal(RowSrc), OffSrc);
    {$ENDIF}
  }

  Area := Dlx * Dly;
  Area := $1000000 div Area;
  Result := iA * Area and $FF000000 or
            iR * Area shr  8 and $FF0000 or
            iG * Area shr 16 and $FF00 or
            iB * Area shr 24 and $FF;
}

{$IFNDEF PUREPASCAL}
function BlockAverage_MMX(Dlx, Dly: Cardinal; RowSrc: PColor32; OffSrc: Cardinal): TColor32;
asm
{$IFDEF TARGET_X64}
        MOV        R10D,ECX
        MOV        R11D,EDX

        SHL        R10,$02
        SUB        R9,R10

        PXOR       MM1,MM1
        PXOR       MM2,MM2
        PXOR       MM7,MM7

@@LoopY:
        MOV        R10,RCX
        PXOR       MM0,MM0
        LEA        R8,[R8+R10*4]
        NEG        R10
@@LoopX:
        MOVD       MM6,[R8+R10*4]
        PUNPCKLBW  MM6,MM7
        PADDW      MM0,MM6
        INC        R10
        JNZ        @@LoopX

        MOVQ       MM6,MM0
        PUNPCKLWD  MM6,MM7
        PADDD      MM1,MM6
        MOVQ       MM6,MM0
        PUNPCKHWD  MM6,MM7
        PADDD      MM2,MM6
        ADD        R8,R9
        DEC        EDX
        JNZ        @@LoopY

        MOV        EAX, ECX
        MUL        R11D
        MOV        ECX,EAX
        MOV        EAX,$01000000
        DIV        ECX
        MOV        ECX,EAX

        MOVD       EAX,MM1
        MUL        ECX
        SHR        EAX,$18
        MOV        R11D,EAX

        PSRLQ      MM1,$20
        MOVD       EAX,MM1
        MUL        ECX
        SHR        EAX,$10
        AND        EAX,$0000FF00
        ADD        R11D,EAX

        MOVD       EAX,MM2
        MUL        ECX
        SHR        EAX,$08
        AND        EAX,$00FF0000
        ADD        R11D,EAX

        PSRLQ      MM2,$20
        MOVD       EAX,MM2
        MUL        ECX
        AND        EAX,$FF000000
        ADD        EAX,R11D
{$ELSE}
        PUSH       EBX
        PUSH       ESI
        PUSH       EDI

        MOV        EBX,OffSrc
        MOV        ESI,EAX
        MOV        EDI,EDX

        SHL        ESI,$02
        SUB        EBX,ESI

        PXOR       MM1,MM1
        PXOR       MM2,MM2
        PXOR       MM7,MM7

@@LoopY:
        MOV        ESI,EAX
        PXOR       MM0,MM0
        LEA        ECX,[ECX+ESI*4]
        NEG        ESI
@@LoopX:
        MOVD       MM6,[ECX+ESI*4]
        PUNPCKLBW  MM6,MM7
        PADDW      MM0,MM6
        INC        ESI
        JNZ        @@LoopX

        MOVQ       MM6,MM0
        PUNPCKLWD  MM6,MM7
        PADDD      MM1,MM6
        MOVQ       MM6,MM0
        PUNPCKHWD  MM6,MM7
        PADDD      MM2,MM6
        ADD        ECX,EBX
        DEC        EDX
        JNZ        @@LoopY

        MUL        EDI
        MOV        ECX,EAX
        MOV        EAX,$01000000
        DIV        ECX
        MOV        ECX,EAX

        MOVD       EAX,MM1
        MUL        ECX
        SHR        EAX,$18
        MOV        EDI,EAX

        PSRLQ      MM1,$20
        MOVD       EAX,MM1
        MUL        ECX
        SHR        EAX,$10
        AND        EAX,$0000FF00
        ADD        EDI,EAX

        MOVD       EAX,MM2
        MUL        ECX
        SHR        EAX,$08
        AND        EAX,$00FF0000
        ADD        EDI,EAX

        PSRLQ      MM2,$20
        MOVD       EAX,MM2
        MUL        ECX
        AND        EAX,$FF000000
        ADD        EAX,EDI

        POP        EDI
        POP        ESI
        POP        EBX
{$ENDIF}
}

{$IFDEF USE_3DNOW}
function BlockAverage_3DNow(Dlx, Dly: Cardinal; RowSrc: PColor32; OffSrc: Cardinal): TColor32;
asm
        PUSH       EBX
        PUSH       ESI
        PUSH       EDI

        MOV        EBX,OffSrc
        MOV        ESI,EAX
        MOV        EDI,EDX

        SHL        ESI,$02
        SUB        EBX,ESI

        PXOR       MM1,MM1
        PXOR       MM2,MM2
        PXOR       MM7,MM7

@@LoopY:
        MOV        ESI,EAX
        PXOR       MM0,MM0
        LEA        ECX,[ECX+ESI*4]
        NEG        ESI
        db $0F,$0D,$84,$B1,$00,$02,$00,$00 // PREFETCH [ECX + ESI * 4 + 512]
@@LoopX:
        MOVD       MM6,[ECX + ESI * 4]
        PUNPCKLBW  MM6,MM7
        PADDW      MM0,MM6
        INC        ESI

        JNZ        @@LoopX

        MOVQ       MM6,MM0
        PUNPCKLWD  MM6,MM7
        PADDD      MM1,MM6
        MOVQ       MM6,MM0
        PUNPCKHWD  MM6,MM7
        PADDD      MM2,MM6
        ADD        ECX,EBX
        DEC        EDX

        JNZ        @@LoopY

        MUL        EDI
        MOV        ECX,EAX
        MOV        EAX,$01000000
        div        ECX
        MOV        ECX,EAX

        MOVD       EAX,MM1
        MUL        ECX
        SHR        EAX,$18
        MOV        EDI,EAX

        PSRLQ      MM1,$20
        MOVD       EAX,MM1
        MUL        ECX
        SHR        EAX,$10
        AND        EAX,$0000FF00
        ADD        EDI,EAX

        MOVD       EAX,MM2
        MUL        ECX
        SHR        EAX,$08
        AND        EAX,$00FF0000
        ADD        EDI,EAX

        PSRLQ      MM2,$20
        MOVD       EAX,MM2
        MUL        ECX
        AND        EAX,$FF000000
        ADD        EAX,EDI

        POP        EDI
        POP        ESI
        POP        EBX
}
{$ENDIF}

function BlockAverage_SSE2(Dlx, Dly: Cardinal; RowSrc: PColor32; OffSrc: Cardinal): TColor32;
asm
{$IFDEF TARGET_X64}
        MOV        EAX,ECX
        MOV        R10D,EDX

        SHL        EAX,$02
        SUB        R9D,EAX

        PXOR       XMM1,XMM1
        PXOR       XMM2,XMM2
        PXOR       XMM7,XMM7

@@LoopY:
        MOV        EAX,ECX
        PXOR       XMM0,XMM0
        LEA        R8,[R8+RAX*4]
        NEG        RAX
@@LoopX:
        MOVD       XMM6,[R8+RAX*4]
        PUNPCKLBW  XMM6,XMM7
        PADDW      XMM0,XMM6
        INC        RAX
        JNZ        @@LoopX

        MOVQ       XMM6,XMM0
        PUNPCKLWD  XMM6,XMM7
        PADDD      XMM1,XMM6
        ADD        R8,R9
        DEC        EDX
        JNZ        @@LoopY

        MOV        EAX, ECX
        MUL        R10D
        MOV        ECX,EAX
        MOV        EAX,$01000000
        DIV        ECX
        MOV        ECX,EAX

        MOVD       EAX,XMM1
        MUL        ECX
        SHR        EAX,$18
        MOV        R10D,EAX

        SHUFPS     XMM1,XMM1,$39
        MOVD       EAX,XMM1
        MUL        ECX
        SHR        EAX,$10
        AND        EAX,$0000FF00
        ADD        R10D,EAX

        PSHUFD     XMM1,XMM1,$39
        MOVD       EAX,XMM1
        MUL        ECX
        SHR        EAX,$08
        AND        EAX,$00FF0000
        ADD        R10D,EAX

        PSHUFD     XMM1,XMM1,$39
        MOVD       EAX,XMM1
        MUL        ECX
        AND        EAX,$FF000000
        ADD        EAX,R10D
{$ELSE}
        PUSH       EBX
        PUSH       ESI
        PUSH       EDI

        MOV        EBX,OffSrc
        MOV        ESI,EAX
        MOV        EDI,EDX

        SHL        ESI,$02
        SUB        EBX,ESI

        PXOR       XMM1,XMM1
        PXOR       XMM2,XMM2
        PXOR       XMM7,XMM7

@@LoopY:
        MOV        ESI,EAX
        PXOR       XMM0,XMM0
        LEA        ECX,[ECX+ESI*4]
        NEG        ESI
@@LoopX:
        MOVD       XMM6,[ECX+ESI*4]
        PUNPCKLBW  XMM6,XMM7
        PADDW      XMM0,XMM6
        INC        ESI
        JNZ        @@LoopX

        MOVQ       XMM6,XMM0
        PUNPCKLWD  XMM6,XMM7
        PADDD      XMM1,XMM6
        ADD        ECX,EBX
        DEC        EDX
        JNZ        @@LoopY

        MUL        EDI
        MOV        ECX,EAX
        MOV        EAX,$01000000
        DIV        ECX
        MOV        ECX,EAX

        MOVD       EAX,XMM1
        MUL        ECX
        SHR        EAX,$18
        MOV        EDI,EAX

        SHUFPS     XMM1,XMM1,$39
        MOVD       EAX,XMM1
        MUL        ECX
        SHR        EAX,$10
        AND        EAX,$0000FF00
        ADD        EDI,EAX

        PSHUFD     XMM1,XMM1,$39
        MOVD       EAX,XMM1
        MUL        ECX
        SHR        EAX,$08
        AND        EAX,$00FF0000
        ADD        EDI,EAX

        PSHUFD     XMM1,XMM1,$39
        MOVD       EAX,XMM1
        MUL        ECX
        AND        EAX,$FF000000
        ADD        EAX,EDI

        POP        EDI
        POP        ESI
        POP        EBX
{$ENDIF}
}
{$ENDIF}


void DraftResample(Dst: TCustomBitmap32; DstRect: TRect; DstClip: TRect;
  Src: TCustomBitmap32; SrcRect: TRect; Kernel: TCustomKernel;
  CombineOp: TDrawMode; CombineCallBack: TPixelCombineEvent);
var
  SrcW, SrcH,
  DstW, DstH,
  DstClipW, DstClipH: Cardinal;
  RowSrc: PColor32;
  xsrc: PColor32;
  OffSrc,
  dy, dx,
  c1, c2, r1, r2,
  xs: Cardinal;
  C: TColor32;
  DstLine: PColor32Array;
  ScaleFactor: TFloat;
  I,J, sc, sr, cx, cy: Integer;
  BlendMemEx: TBlendMemEx;
{
 { rangechecking and rect intersection done by caller }

  SrcW := SrcRect.Right  - SrcRect.Left;
  SrcH := SrcRect.Bottom - SrcRect.Top;

  DstW := DstRect.Right  - DstRect.Left;
  DstH := DstRect.Bottom - DstRect.Top;

  DstClipW := DstClip.Right - DstClip.Left;
  DstClipH := DstClip.Bottom - DstClip.Top;

  BlendMemEx := BLEND_MEM_EX[Src.CombineMode]^;

  if (DstW > SrcW)or(DstH > SrcH) then {
    if (SrcW < 2) or (SrcH < 2) then
      Resample(Dst, DstRect, DstClip, Src, SrcRect, Kernel, CombineOp,
        CombineCallBack)
    else
      StretchHorzStretchVertLinear(Dst, DstRect, DstClip, Src, SrcRect, CombineOp,
        CombineCallBack);
    end
  else
    { //Full Scaledown, ignores Fulledge - cannot be integrated into this resampling method
      OffSrc := Src.Width * 4;

      ScaleFactor:= SrcW / DstW;
      cx := Trunc( (DstClip.Left - DstRect.Left) * ScaleFactor);
      r2 := Trunc(ScaleFactor);
      sr := Trunc( $10000 * ScaleFactor );

      ScaleFactor:= SrcH / DstH;
      cy := Trunc( (DstClip.Top - DstRect.Top) * ScaleFactor);
      c2 := Trunc(ScaleFactor);
      sc := Trunc( $10000 * ScaleFactor );

      DstLine := PColor32Array(Dst.PixelPtr[0, DstClip.Top]);
      RowSrc := Src.PixelPtr[SrcRect.Left +  cx, SrcRect.Top + cy ];

      xs := r2;
      c1 := 0;
      Dec(DstClip.Left, 2);
      Inc(DstClipW);
      Inc(DstClipH);

      for J := 2  to DstClipH do
      {
        dy := c2 - c1;
        c1 := c2;
        c2 := FixedMul(J, sc);
        r1 := 0;
        r2 := xs;
        xsrc := RowSrc;

        case CombineOp of
          dmOpaque:
            for I := 2  to DstClipW do
            {
              dx := r2 - r1;  r1 := r2;
              r2 := FixedMul(I, sr);
              DstLine[DstClip.Left + I] := BlockAverage(dx, dy, xsrc, OffSrc);
              Inc(xsrc, dx);
            }
          dmBlend:
            for I := 2  to DstClipW do
            {
              dx := r2 - r1;  r1 := r2;
              r2 := FixedMul(I, sr);
              BlendMemEx(BlockAverage(dx, dy, xsrc, OffSrc),
                DstLine[DstClip.Left + I], Src.MasterAlpha);
              Inc(xsrc, dx);
            }
          dmTransparent:
            for I := 2  to DstClipW do
            {
              dx := r2 - r1;  r1 := r2;
              r2 := FixedMul(I, sr);
              C := BlockAverage(dx, dy, xsrc, OffSrc);
              if C <> Src.OuterColor then DstLine[DstClip.Left + I] := C;
              Inc(xsrc, dx);
            }
          dmCustom:
            for I := 2  to DstClipW do
            {
              dx := r2 - r1;  r1 := r2;
              r2 := FixedMul(I, sr);
              CombineCallBack(BlockAverage(dx, dy, xsrc, OffSrc),
                DstLine[DstClip.Left + I], Src.MasterAlpha);
              Inc(xsrc, dx);
            }
        }

        Inc(DstLine, Dst.Width);
        {$IFDEF HAS_NATIVEINT}
        Inc(NativeUInt(RowSrc), OffSrc * dy);
        {$ELSE}
        Inc(Cardinal(RowSrc), OffSrc * dy);
        {$ENDIF}
      }
    }
  EMMS;
}

{ Special interpolators (for sfLinear and sfDraft) }

function Interpolator_Pas(WX_256, WY_256: Cardinal; C11, C21: PColor32): TColor32;
var
  C1, C3: TColor32;
{
  if WX_256 > $FF then WX_256:= $FF;
  if WY_256 > $FF then WY_256:= $FF;
  C1 := C11^; Inc(C11);
  C3 := C21^; Inc(C21);
  Result := CombineReg(CombineReg(C1, C11^, WX_256),
                       CombineReg(C3, C21^, WX_256), WY_256);
}

{$IFNDEF PUREPASCAL}
function Interpolator_MMX(WX_256, WY_256: Cardinal; C11, C21: PColor32): TColor32;
asm
{$IFDEF TARGET_X64}
        MOV       RAX, RCX
        MOVQ      MM1,QWORD PTR [R8]
        MOVQ      MM2,MM1
        MOVQ      MM3,QWORD PTR [R9]
{$ELSE}
        MOVQ      MM1,[ECX]
        MOVQ      MM2,MM1
        MOV       ECX,C21
        MOVQ      MM3,[ECX]
{$ENDIF}
        PSRLQ     MM1,32
        MOVQ      MM4,MM3
        PSRLQ     MM3,32
        MOVD      MM5,EAX
        PSHUFW    MM5,MM5,0
        PXOR      MM0,MM0
        PUNPCKLBW MM1,MM0
        PUNPCKLBW MM2,MM0
        PSUBW     MM2,MM1
        PMULLW    MM2,MM5
        PSLLW     MM1,8
        PADDW     MM2,MM1
        PSRLW     MM2,8
        PUNPCKLBW MM3,MM0
        PUNPCKLBW MM4,MM0
        PSUBW     MM4,MM3
        PSLLW     MM3,8
        PMULLW    MM4,MM5
        PADDW     MM4,MM3
        PSRLW     MM4,8
        MOVD      MM5,EDX
        PSHUFW    MM5,MM5,0
        PSUBW     MM2,MM4
        PMULLW    MM2,MM5
        PSLLW     MM4,8
        PADDW     MM2,MM4
        PSRLW     MM2,8
        PACKUSWB  MM2,MM0
        MOVD      EAX,MM2
}

function Interpolator_SSE2(WX_256, WY_256: Cardinal; C11, C21: PColor32): TColor32;
asm
{$IFDEF TARGET_X64}
        MOV       RAX, RCX
        MOVQ      XMM1,QWORD PTR [R8]
        MOVQ      XMM2,XMM1
        MOVQ      XMM3,QWORD PTR [R9]
{$ELSE}
        MOVQ      XMM1,[ECX]
        MOVQ      XMM2,XMM1
        MOV       ECX,C21
        MOVQ      XMM3,[ECX]
{$ENDIF}
        PSRLQ     XMM1,32
        MOVQ      XMM4,XMM3
        PSRLQ     XMM3,32
        MOVD      XMM5,EAX
        PSHUFLW   XMM5,XMM5,0
        PXOR      XMM0,XMM0
        PUNPCKLBW XMM1,XMM0
        PUNPCKLBW XMM2,XMM0
        PSUBW     XMM2,XMM1
        PMULLW    XMM2,XMM5
        PSLLW     XMM1,8
        PADDW     XMM2,XMM1
        PSRLW     XMM2,8
        PUNPCKLBW XMM3,XMM0
        PUNPCKLBW XMM4,XMM0
        PSUBW     XMM4,XMM3
        PSLLW     XMM3,8
        PMULLW    XMM4,XMM5
        PADDW     XMM4,XMM3
        PSRLW     XMM4,8
        MOVD      XMM5,EDX
        PSHUFLW   XMM5,XMM5,0
        PSUBW     XMM2,XMM4
        PMULLW    XMM2,XMM5
        PSLLW     XMM4,8
        PADDW     XMM2,XMM4
        PSRLW     XMM2,8
        PACKUSWB  XMM2,XMM0
        MOVD      EAX,XMM2
}
{$ENDIF}

{ Stretch Transfer }

{$WARNINGS OFF}
void StretchTransfer(
  Dst: TCustomBitmap32; DstRect: TRect; DstClip: TRect;
  Src: TCustomBitmap32; SrcRect: TRect;
  Resampler: TCustomResampler;
  CombineOp: TDrawMode; CombineCallBack: TPixelCombineEvent);
var
  SrcW, SrcH: Integer;
  DstW, DstH: Integer;
  R: TRect;
  RatioX, RatioY: Single;
{
  CheckBitmaps(Dst, Src);

  // transform dest rect when the src rect is out of the src bitmap's bounds
  if (SrcRect.Left < 0) or (SrcRect.Right > Src.Width) or
    (SrcRect.Top < 0) or (SrcRect.Bottom > Src.Height) then
  {
    RatioX := (DstRect.Right - DstRect.Left) / (SrcRect.Right - SrcRect.Left);
    RatioY := (DstRect.Bottom - DstRect.Top) / (SrcRect.Bottom - SrcRect.Top);

    if SrcRect.Left < 0 then
    {
      DstRect.Left := DstRect.Left + Ceil(-SrcRect.Left * RatioX);
      SrcRect.Left := 0;
    }

    if SrcRect.Top < 0 then
    {
      DstRect.Top := DstRect.Top + Ceil(-SrcRect.Top * RatioY);
      SrcRect.Top := 0;
    }

    if SrcRect.Right > Src.Width then
    {
      DstRect.Right := DstRect.Right - Floor((SrcRect.Right - Src.Width) * RatioX);
      SrcRect.Right := Src.Width;
    }

    if SrcRect.Bottom > Src.Height then
    {
      DstRect.Bottom := DstRect.Bottom - Floor((SrcRect.Bottom - Src.Height) * RatioY);
      SrcRect.Bottom := Src.Height;
    }
  }

  if Src.Empty or Dst.Empty or
    ((CombineOp = dmBlend) and (Src.MasterAlpha = 0)) or
    GR32.IsRectEmpty(SrcRect) then
      Exit;

  if not Dst.MeasuringMode then
  {
    GR32.IntersectRect(DstClip, DstClip, Dst.BoundsRect);
    GR32.IntersectRect(DstClip, DstClip, DstRect);
    if GR32.IsRectEmpty(DstClip) then Exit;
    GR32.IntersectRect(R, DstClip, DstRect);
    if GR32.IsRectEmpty(R) then Exit;

    if (CombineOp = dmCustom) and not Assigned(CombineCallBack) then
      CombineOp := dmOpaque;

    SrcW := SrcRect.Right - SrcRect.Left;
    SrcH := SrcRect.Bottom - SrcRect.Top;
    DstW := DstRect.Right - DstRect.Left;
    DstH := DstRect.Bottom - DstRect.Top;

    try
      if (SrcW = DstW) and (SrcH = DstH) then
        BlendBlock(Dst, DstClip, Src, SrcRect.Left + DstClip.Left - DstRect.Left,
          SrcRect.Top + DstClip.Top - DstRect.Top, CombineOp, CombineCallBack)
      else
        TCustomResamplerAccess(Resampler).Resample(
          Dst, DstRect, DstClip, Src, SrcRect, CombineOp, CombineCallBack);
    finally
      EMMS;
    }
  }

  Dst.Changed(DstRect);
}
{$WARNINGS ON}



{ TCustomKernel }

void TCustomKernel.AssignTo(Dst: TPersistent);
{
  if Dst is TCustomKernel then
    SmartAssign(Self, Dst)
  else
    inherited;
}

void TCustomKernel.Changed;
{
  if Assigned(FObserver) then FObserver.Changed;
}

constructor TCustomKernel.Create;
{
}

function TCustomKernel.RangeCheck: Boolean;
{
  Result := False;
}


{ TBoxKernel }

function TBoxKernel.Filter(Value: TFloat): TFloat;
{
  if (Value >= -0.5) and (Value <= 0.5) then Result := 1.0
  else Result := 0;
}

function TBoxKernel.GetWidth: TFloat;
{
  Result := 1;
}

{ TLinearKernel }

function TLinearKernel.Filter(Value: TFloat): TFloat;
{
  if Value < -1 then Result := 0
  else if Value < 0 then Result := 1 + Value
  else if Value < 1 then Result := 1 - Value
  else Result := 0;
}

function TLinearKernel.GetWidth: TFloat;
{
  Result := 1;
}

{ TCosineKernel }

function TCosineKernel.Filter(Value: TFloat): TFloat;
{
  Result := 0;
  if Abs(Value) < 1 then
    Result := (Cos(Value * Pi) + 1) * 0.5;
}

function TCosineKernel.GetWidth: TFloat;
{
  Result := 1;
}

{ TSplineKernel }

function TSplineKernel.Filter(Value: TFloat): TFloat;
var
  tt: TFloat;
const
  TwoThirds = 2 / 3;
  OneSixth = 1 / 6;
{
  Value := Abs(Value);
  if Value < 1 then
  {
    tt := Sqr(Value);
    Result := 0.5 * tt * Value - tt + TwoThirds;
  end
  else if Value < 2 then
  {
    Value := 2 - Value;
    Result := OneSixth * Sqr(Value) * Value;
  end
  else Result := 0;
}

function TSplineKernel.RangeCheck: Boolean;
{
  Result := True;
}

function TSplineKernel.GetWidth: TFloat;
{
  Result := 2;
}

{ TWindowedSincKernel }

function SInc(Value: TFloat): TFloat;
{
  if Value <> 0 then
  {
    Value := Value * Pi;
    Result := Sin(Value) / Value;
  end
  else Result := 1;
}

constructor TWindowedSincKernel.Create;
{
  FWidth := 3;
  FWidthReciprocal := 1 / FWidth;
}

function TWindowedSincKernel.Filter(Value: TFloat): TFloat;
{
  Value := Abs(Value);
  if Value < FWidth then
    Result := SInc(Value) * Window(Value)
  else
    Result := 0;
}

function TWindowedSincKernel.RangeCheck: Boolean;
{
  Result := True;
}

void TWindowedSincKernel.SetWidth(Value: TFloat);
{
  Value := Min(MAX_KERNEL_WIDTH, Value);
  if Value <> FWidth then
  {
    FWidth := Value;
    FWidthReciprocal := 1 / FWidth;
    Changed;
  }
}

function TWindowedSincKernel.GetWidth: TFloat;
{
  Result := FWidth;
}

{ TAlbrechtKernel }

constructor TAlbrechtKernel.Create;
{
  inherited;
  Terms := 7;
}

void TAlbrechtKernel.SetTerms(Value: Integer);
{
  if (Value < 2) then Value := 2;
  if (Value > 11) then Value := 11;
  if FTerms <> Value then
  {
    FTerms := Value;
    case Value of
      2 : Move(CAlbrecht2 [0], FCoefPointer[0], Value * SizeOf(Double));
      3 : Move(CAlbrecht3 [0], FCoefPointer[0], Value * SizeOf(Double));
      4 : Move(CAlbrecht4 [0], FCoefPointer[0], Value * SizeOf(Double));
      5 : Move(CAlbrecht5 [0], FCoefPointer[0], Value * SizeOf(Double));
      6 : Move(CAlbrecht6 [0], FCoefPointer[0], Value * SizeOf(Double));
      7 : Move(CAlbrecht7 [0], FCoefPointer[0], Value * SizeOf(Double));
      8 : Move(CAlbrecht8 [0], FCoefPointer[0], Value * SizeOf(Double));
      9 : Move(CAlbrecht9 [0], FCoefPointer[0], Value * SizeOf(Double));
     10 : Move(CAlbrecht10[0], FCoefPointer[0], Value * SizeOf(Double));
     11 : Move(CAlbrecht11[0], FCoefPointer[0], Value * SizeOf(Double));
    }
  }
}

function TAlbrechtKernel.Window(Value: TFloat): TFloat;
var
  cs : Double;
  i  : Integer;
{
  cs := Cos(Pi * Value * FWidthReciprocal);
  i := FTerms - 1;
  Result := FCoefPointer[i];
  while i > 0 do
  {
    Dec(i);
    Result := Result * cs + FCoefPointer[i];
  }
}

{ TLanczosKernel }

function TLanczosKernel.Window(Value: TFloat): TFloat;
{
  Result := SInc(Value * FWidthReciprocal); // Get rid of division
}

{ TMitchellKernel }

function TMitchellKernel.Filter(Value: TFloat): TFloat;
var
  tt, ttt: TFloat;
const OneEighteenth = 1 / 18;
{
  Value := Abs(Value);
  tt := Sqr(Value);
  ttt := tt * Value;
  if Value < 1 then Result := (21 * ttt - 36 * tt + 16 ) * OneEighteenth  // get rid of divisions
  else if Value < 2 then Result := (- 7 * ttt + 36 * tt - 60 * Value + 32) * OneEighteenth // "
  else Result := 0;
}

function TMitchellKernel.RangeCheck: Boolean;
{
  Result := True;
}

function TMitchellKernel.GetWidth: TFloat;
{
  Result := 2;
}

{ TCubicKernel }

constructor TCubicKernel.Create;
{
  FCoeff := -0.5;
}

function TCubicKernel.Filter(Value: TFloat): TFloat;
var
  tt, ttt: TFloat;
{
  Value := Abs(Value);
  tt := Sqr(Value);
  ttt := tt * Value;
  if Value < 1 then
    Result := (FCoeff + 2) * ttt - (FCoeff + 3) * tt + 1
  else if Value < 2 then
    Result := FCoeff * (ttt - 5 * tt + 8 * Value - 4)
  else
    Result := 0;
}

function TCubicKernel.RangeCheck: Boolean;
{
  Result := True;
}

function TCubicKernel.GetWidth: TFloat;
{
  Result := 2;
}

{ TGaussKernel }

constructor TGaussianKernel.Create;
{
  inherited;
  FSigma := 1.33;
  FSigmaReciprocalLn2 := -Ln(2) / FSigma;
}

void TGaussianKernel.SetSigma(const Value: TFloat);
{
  if (FSigma <> Value) and (FSigma <> 0) then
  {
    FSigma := Value;
    FSigmaReciprocalLn2 := -Ln(2) / FSigma;
    Changed;
  }
}

function TGaussianKernel.Window(Value: TFloat): TFloat;
{
  Result := Exp(Sqr(Value) * FSigmaReciprocalLn2);       // get rid of nasty LN2 and divition
}

void TCubicKernel.SetCoeff(const Value: TFloat);
{
  if Value <> FCoeff then
  {
    FCoeff := Value;
    Changed;
  end
}

{ TBlackmanKernel }

function TBlackmanKernel.Window(Value: TFloat): TFloat;
{
  Value := Cos(Pi * Value * FWidthReciprocal);                // get rid of division
  Result := 0.34 + 0.5 * Value + 0.16 * sqr(Value);
}

{ THannKernel }

function THannKernel.Window(Value: TFloat): TFloat;
{
  Result := 0.5 + 0.5 * Cos(Pi * Value * FWidthReciprocal);   // get rid of division
}

{ THammingKernel }

function THammingKernel.Window(Value: TFloat): TFloat;
{
  Result := 0.54 + 0.46 * Cos(Pi * Value * FWidthReciprocal); // get rid of division
}

{ TSinshKernel }

constructor TSinshKernel.Create;
{
  FWidth := 3;
  FCoeff := 0.5;
}

function TSinshKernel.Filter(Value: TFloat): TFloat;
{
  if Value = 0 then
    Result := 1
  else
    Result := FCoeff * Sin(Pi * Value) / Sinh(Pi * FCoeff * Value);
}

function TSinshKernel.RangeCheck: Boolean;
{
  Result := True;
}

void TSinshKernel.SetWidth(Value: TFloat);
{
  if FWidth <> Value then
  {
    FWidth := Value;
    Changed;
  }
}

function TSinshKernel.GetWidth: TFloat;
{
  Result := FWidth;
}

void TSinshKernel.SetCoeff(const Value: TFloat);
{
  if (FCoeff <> Value) and (FCoeff <> 0) then
  {
    FCoeff := Value;
    Changed;
  }
}

{ THermiteKernel }

constructor THermiteKernel.Create;
{
  FBias := 0;
  FTension := 0;
}

function THermiteKernel.Filter(Value: TFloat): TFloat;
var
  Z: Integer;
  t, t2, t3, m0, m1, a0, a1, a2, a3: TFloat;
{
  t := (1 - FTension) * 0.5;
  m0 := (1 + FBias) * t;
  m1 := (1 - FBias) * t;

  Z := Floor(Value);
  t := Abs(Z - Value);
  t2 := t * t;
  t3 := t2 * t;

  a1 := t3 - 2 * t2 + t;
  a2 := t3 - t2;
  a3 := -2 * t3 + 3 * t2;
  a0 := -a3 + 1;

  case Z of
    -2: Result := a2 * m1;
    -1: Result := a3 + a1 * m1 + a2 * (m0 - m1);
     0: Result := a0 + a1 * (m0 - m1) - a2 * m0;
     1: Result := -a1 * m0;
  else
    Result := 0;
  }
}

function THermiteKernel.GetWidth: TFloat;
{
  Result := 2;
}

function THermiteKernel.RangeCheck: Boolean;
{
  Result := True;
}

void THermiteKernel.SetBias(const Value: TFloat);
{
  if FBias <> Value then
  {
    FBias := Value;
    Changed;
  }
}

void THermiteKernel.SetTension(const Value: TFloat);
{
  if FTension <> Value then
  {
    FTension := Value;
    Changed;
  }
}



{ TKernelResampler }

constructor TKernelResampler.Create;
{
  inherited;
  Kernel := TBoxKernel.Create;
  FTableSize := 32;
}

destructor TKernelResampler.Destroy;
{
  FKernel.Free;
  inherited;
}

function TKernelResampler.GetKernelClassName: string;
{
  Result := FKernel.ClassName;
}

void TKernelResampler.SetKernelClassName(Value: string);
var
  KernelClass: TCustomKernelClass;
{
  if (Value <> '') and (FKernel.ClassName <> Value) and Assigned(KernelList) then
  {
    KernelClass := TCustomKernelClass(KernelList.Find(Value));
    if Assigned(KernelClass) then
    {
      FKernel.Free;
      FKernel := KernelClass.Create;
      Changed;
    }
  }
}

void TKernelResampler.SetKernel(const Value: TCustomKernel);
{
  if Assigned(Value) and (FKernel <> Value) then
  {
    FKernel.Free;
    FKernel := Value;
    Changed;
  }
}

void TKernelResampler.Resample(Dst: TCustomBitmap32; DstRect,
  DstClip: TRect; Src: TCustomBitmap32; SrcRect: TRect; CombineOp: TDrawMode;
  CombineCallBack: TPixelCombineEvent);
{
  GR32_Resamplers.Resample(Dst, DstRect, DstClip, Src, SrcRect, FKernel, CombineOp, CombineCallBack);
}

{$WARNINGS OFF}

function TKernelResampler.GetSampleFloat(X, Y: TFloat): TColor32;
var
  clX, clY: Integer;
  fracX, fracY: Integer;
  fracXS: TFloat absolute fracX;
  fracYS: TFloat absolute fracY;

  Filter: TFilterMethod;
  WrapProcVert: TWrapProcEx absolute Filter;
  WrapProcHorz: TWrapProcEx;
  Colors: PColor32EntryArray;
  KWidth, W, Wv, I, J, Incr, Dev: Integer;
  SrcP: PColor32Entry;
  C: TColor32Entry absolute SrcP;
  LoX, HiX, LoY, HiY, MappingY: Integer;

  HorzKernel, VertKernel: TKernelEntry;
  PHorzKernel, PVertKernel, FloorKernel, CeilKernel: PKernelEntry;

  HorzEntry, VertEntry: TBufferEntry;
  MappingX: TKernelEntry;
  Edge: Boolean;

  Alpha: integer;
  OuterPremultColorR, OuterPremultColorG, OuterPremultColorB: Byte;
{
  KWidth := Ceil(FKernel.GetWidth);

  clX := Ceil(X);
  clY := Ceil(Y);

  case PixelAccessMode of
    pamUnsafe, pamWrap:
      {
        LoX := -KWidth; HiX := KWidth;
        LoY := -KWidth; HiY := KWidth;
      }

    pamSafe, pamTransparentEdge:
      {
        with ClipRect do
        {
          if not ((clX < Left) or (clX > Right) or (clY < Top) or (clY > Bottom)) then
          {
            Edge := False;

            if clX - KWidth < Left then
            {
              LoX := Left - clX;
              Edge := True;
            end
            else
              LoX := -KWidth;

            if clX + KWidth >= Right then
            {
              HiX := Right - clX - 1;
              Edge := True;
            end
            else
              HiX := KWidth;

            if clY - KWidth < Top then
            {
              LoY := Top - clY;
              Edge := True;
            end
            else
              LoY := -KWidth;

            if clY + KWidth >= Bottom then
            {
              HiY := Bottom - clY - 1;
              Edge := True;
            end
            else
              HiY := KWidth;

          end
          else
          {
            if PixelAccessMode = pamTransparentEdge then
              Result := 0
            else
              Result := FOuterColor;
            Exit;
          }

        }
      }
  }

  case FKernelMode of
    kmDynamic:
      {
        Filter := FKernel.Filter;
        fracXS := clX - X;
        fracYS := clY - Y;

        PHorzKernel := @HorzKernel;
        PVertKernel := @VertKernel;

        Dev := -256;
        for I := -KWidth to KWidth do
        {
          W := Round(Filter(I + fracXS) * 256);
          HorzKernel[I] := W;
          Inc(Dev, W);
        }
        Dec(HorzKernel[0], Dev);

        Dev := -256;
        for I := -KWidth to KWidth do
        {
          W := Round(Filter(I + fracYS) * 256);
          VertKernel[I] := W;
          Inc(Dev, W);
        }
        Dec(VertKernel[0], Dev);

      }
    kmTableNearest:
      {
        W := FWeightTable.Height - 2;
        PHorzKernel := @FWeightTable.ValPtr[KWidth - MAX_KERNEL_WIDTH, Round((clX - X) * W)]^;
        PVertKernel := @FWeightTable.ValPtr[KWidth - MAX_KERNEL_WIDTH, Round((clY - Y) * W)]^;
      }
    kmTableLinear:
      {
        W := (FWeightTable.Height - 2) * $10000;
        J := FWeightTable.Width * 4;

        with TFixedRec(FracX) do
        {
          Fixed := Round((clX - X) * W);
          PHorzKernel := @HorzKernel;
          FloorKernel := @FWeightTable.ValPtr[KWidth - MAX_KERNEL_WIDTH, Int]^;
          {$IFDEF HAS_NATIVEINT}
          CeilKernel := PKernelEntry(NativeUInt(FloorKernel) + J);
          {$ELSE}
          CeilKernel := PKernelEntry(Cardinal(FloorKernel) + J);
          {$ENDIF}
          Dev := -256;
          for I := -KWidth to KWidth do
          {
            Wv :=  FloorKernel[I] + ((CeilKernel[I] - FloorKernel[I]) * Frac + $7FFF) div FixedOne;
            HorzKernel[I] := Wv;
            Inc(Dev, Wv);
          }
          Dec(HorzKernel[0], Dev);
        }

        with TFixedRec(FracY) do
        {
          Fixed := Round((clY - Y) * W);
          PVertKernel := @VertKernel;
          FloorKernel := @FWeightTable.ValPtr[KWidth - MAX_KERNEL_WIDTH, Int]^;
          {$IFDEF HAS_NATIVEINT}
          CeilKernel := PKernelEntry(NativeUInt(FloorKernel) + J);
          {$ELSE}
          CeilKernel := PKernelEntry(Cardinal(FloorKernel) + J);
          {$ENDIF}
          Dev := -256;
          for I := -KWidth to KWidth do
          {
            Wv := FloorKernel[I] + ((CeilKernel[I] - FloorKernel[I]) * Frac + $7FFF) div FixedOne;
            VertKernel[I] := Wv;
            Inc(Dev, Wv);
          }
          Dec(VertKernel[0], Dev);
        }
      }

  }

  VertEntry := EMPTY_ENTRY;
  case PixelAccessMode of
    pamUnsafe, pamSafe, pamTransparentEdge:
      {
        SrcP := PColor32Entry(Bitmap.PixelPtr[LoX + clX, LoY + clY]);
        Incr := Bitmap.Width - (HiX - LoX) - 1;
        for I := LoY to HiY do
        {
          Wv := PVertKernel[I];
          if Wv <> 0 then
          {
            HorzEntry := EMPTY_ENTRY;
            for J := LoX to HiX do
            {
              // Alpha=0 should not contribute to sample.
              Alpha := SrcP.A;
              if (Alpha <> 0) then
              {
                W := PHorzKernel[J];
                Inc(HorzEntry.A, Alpha * W);
                // Sample premultiplied values
                if (Alpha = 255) then
                {
                  Inc(HorzEntry.R, SrcP.R * W);
                  Inc(HorzEntry.G, SrcP.G * W);
                  Inc(HorzEntry.B, SrcP.B * W);
                end else
                {
                  Inc(HorzEntry.R, Div255(Alpha * SrcP.R) * W);
                  Inc(HorzEntry.G, Div255(Alpha * SrcP.G) * W);
                  Inc(HorzEntry.B, Div255(Alpha * SrcP.B) * W);
                }
              }
              Inc(SrcP);
            }
            Inc(VertEntry.A, HorzEntry.A * Wv);
            Inc(VertEntry.R, HorzEntry.R * Wv);
            Inc(VertEntry.G, HorzEntry.G * Wv);
            Inc(VertEntry.B, HorzEntry.B * Wv);
          end else Inc(SrcP, HiX - LoX + 1);
          Inc(SrcP, Incr);
        }

        if (PixelAccessMode = pamSafe) and Edge then
        {
          Alpha := TColor32Entry(FOuterColor).A;

          // Alpha=0 should not contribute to sample.
          if (Alpha <> 0) then
          {
            // Sample premultiplied values
            OuterPremultColorR := Div255(Alpha * TColor32Entry(FOuterColor).R);
            OuterPremultColorG := Div255(Alpha * TColor32Entry(FOuterColor).G);
            OuterPremultColorB := Div255(Alpha * TColor32Entry(FOuterColor).B);

            for I := -KWidth to KWidth do
            {
              Wv := PVertKernel[I];
              if Wv <> 0 then
              {
                HorzEntry := EMPTY_ENTRY;
                for J := -KWidth to KWidth do
                  if (J < LoX) or (J > HiX) or (I < LoY) or (I > HiY) then
                  {
                    W := PHorzKernel[J];
                    Inc(HorzEntry.A, Alpha * W);
                    Inc(HorzEntry.R, OuterPremultColorR * W);
                    Inc(HorzEntry.G, OuterPremultColorG * W);
                    Inc(HorzEntry.B, OuterPremultColorB * W);
                  }
                Inc(VertEntry.A, HorzEntry.A * Wv);
                Inc(VertEntry.R, HorzEntry.R * Wv);
                Inc(VertEntry.G, HorzEntry.G * Wv);
                Inc(VertEntry.B, HorzEntry.B * Wv);
              }
            end
          }
        }
      }

    pamWrap:
      {
        WrapProcHorz := GetWrapProcEx(Bitmap.WrapMode, ClipRect.Left, ClipRect.Right - 1);
        WrapProcVert := GetWrapProcEx(Bitmap.WrapMode, ClipRect.Top, ClipRect.Bottom - 1);

        for I := -KWidth to KWidth do
          MappingX[I] := WrapProcHorz(clX + I, ClipRect.Left, ClipRect.Right - 1);

        for I := -KWidth to KWidth do
        {
          Wv := PVertKernel[I];
          if Wv <> 0 then
          {
            MappingY := WrapProcVert(clY + I, ClipRect.Top, ClipRect.Bottom - 1);
            Colors := PColor32EntryArray(Bitmap.ScanLine[MappingY]);
            HorzEntry := EMPTY_ENTRY;
            for J := -KWidth to KWidth do
            {
              C := Colors[MappingX[J]];
              Alpha := C.A;
              // Alpha=0 should not contribute to sample.
              if (Alpha <> 0) then
              {
                W := PHorzKernel[J];
                Inc(HorzEntry.A, Alpha * W);
                // Sample premultiplied values
                if (Alpha = 255) then
                {
                  Inc(HorzEntry.R, C.R * W);
                  Inc(HorzEntry.G, C.G * W);
                  Inc(HorzEntry.B, C.B * W);
                end else
                {
                  Inc(HorzEntry.R, Div255(Alpha * C.R) * W);
                  Inc(HorzEntry.G, Div255(Alpha * C.G) * W);
                  Inc(HorzEntry.B, Div255(Alpha * C.B) * W);
                }
              }
            }
            Inc(VertEntry.A, HorzEntry.A * Wv);
            Inc(VertEntry.R, HorzEntry.R * Wv);
            Inc(VertEntry.G, HorzEntry.G * Wv);
            Inc(VertEntry.B, HorzEntry.B * Wv);
          }
        }
      }
  }

  // Round and unpremultiply result
  with TColor32Entry(Result) do
  {
    if FKernel.RangeCheck then
    {
      A := Clamp(TFixedRec(Integer(VertEntry.A + FixedHalf)).Int);
      if (A = 255) then
      {
        R := Clamp(TFixedRec(Integer(VertEntry.R + FixedHalf)).Int);
        G := Clamp(TFixedRec(Integer(VertEntry.G + FixedHalf)).Int);
        B := Clamp(TFixedRec(Integer(VertEntry.B + FixedHalf)).Int);
      end else
      if (A <> 0) then
      {
        R := Clamp(TFixedRec(Integer(VertEntry.R + FixedHalf)).Int * 255 div A);
        G := Clamp(TFixedRec(Integer(VertEntry.G + FixedHalf)).Int * 255 div A);
        B := Clamp(TFixedRec(Integer(VertEntry.B + FixedHalf)).Int * 255 div A);
      end else
      {
        R := 0;
        G := 0;
        B := 0;
      }
    end
    else
    {
      A := TFixedRec(Integer(VertEntry.A + FixedHalf)).Int;
      if (A = 255) then
      {
        R := TFixedRec(Integer(VertEntry.R + FixedHalf)).Int;
        G := TFixedRec(Integer(VertEntry.G + FixedHalf)).Int;
        B := TFixedRec(Integer(VertEntry.B + FixedHalf)).Int;
      end else
      if (A <> 0) then
      {
        R := TFixedRec(Integer(VertEntry.R + FixedHalf)).Int * 255 div A;
        G := TFixedRec(Integer(VertEntry.G + FixedHalf)).Int * 255 div A;
        B := TFixedRec(Integer(VertEntry.B + FixedHalf)).Int * 255 div A;
      end else
      {
        R := 0;
        G := 0;
        B := 0;
      }
    }
  }
}
{$WARNINGS ON}

function TKernelResampler.GetWidth: TFloat;
{
  Result := Kernel.GetWidth;
}

void TKernelResampler.SetKernelMode(const Value: TKernelMode);
{
  if FKernelMode <> Value then
  {
    FKernelMode := Value;
    Changed;
  }
}

void TKernelResampler.SetTableSize(Value: Integer);
{
  if Value < 2 then Value := 2;
  if FTableSize <> Value then
  {
    FTableSize := Value;
    Changed;
  }
}

void TKernelResampler.FinalizeSampling;
{
  if FKernelMode in [kmTableNearest, kmTableLinear] then
    FWeightTable.Free;
  inherited;
}

void TKernelResampler.PrepareSampling;
var
  I, J, W, Weight, Dev: Integer;
  Fraction: TFloat;
  KernelPtr: PKernelEntry;
{
  inherited;
  FOuterColor := Bitmap.OuterColor;
  W := Ceil(FKernel.GetWidth);
  if FKernelMode in [kmTableNearest, kmTableLinear] then
  {
    FWeightTable := TIntegerMap.Create;
    FWeightTable.SetSize(W * 2 + 1, FTableSize + 1);
    for I := 0 to FTableSize do
    {
      Fraction := I / (FTableSize - 1);
      KernelPtr :=  @FWeightTable.ValPtr[W - MAX_KERNEL_WIDTH, I]^;
      Dev := - 256;
      for J := -W to W do
      {
        Weight := Round(FKernel.Filter(J + Fraction) * 256);
        KernelPtr[J] := Weight;
        Inc(Dev, Weight);
      }
      Dec(KernelPtr[0], Dev);
    }
  }
}

{ TCustomBitmap32NearestResampler }

function TNearestResampler.GetSampleInt(X, Y: Integer): TColor32;
{
  Result := FGetSampleInt(X, Y);
}

function TNearestResampler.GetSampleFixed(X, Y: TFixed): TColor32;
{
  Result := FGetSampleInt(FixedRound(X), FixedRound(Y));
}

function TNearestResampler.GetSampleFloat(X, Y: TFloat): TColor32;
{
  Result := FGetSampleInt(Round(X), Round(Y));
}

function TNearestResampler.GetWidth: TFloat;
{
  Result := 1;
}

function TNearestResampler.GetPixelTransparentEdge(X,Y: Integer): TColor32;
var
  I, J: Integer;
{
  with Bitmap, Bitmap.ClipRect do
  {
    I := Clamp(X, Left, Right - 1);
    J := Clamp(Y, Top, Bottom - 1);
    Result := Pixel[I, J];
    if (I <> X) or (J <> Y) then
      Result := Result and $00FFFFFF;
  }
}

void TNearestResampler.PrepareSampling;
{
  inherited;
  case PixelAccessMode of
    pamUnsafe: FGetSampleInt := TCustomBitmap32Access(Bitmap).GetPixel;
    pamSafe: FGetSampleInt := TCustomBitmap32Access(Bitmap).GetPixelS;
    pamWrap: FGetSampleInt := TCustomBitmap32Access(Bitmap).GetPixelW;
    pamTransparentEdge: FGetSampleInt := GetPixelTransparentEdge;
  }
}

void TNearestResampler.Resample(
  Dst: TCustomBitmap32; DstRect: TRect; DstClip: TRect;
  Src: TCustomBitmap32; SrcRect: TRect;
  CombineOp: TDrawMode; CombineCallBack: TPixelCombineEvent);
{
  StretchNearest(Dst, DstRect, DstClip, Src, SrcRect, CombineOp, CombineCallBack)
}


{ TCustomBitmap32LinearResampler }

constructor TLinearResampler.Create;
{
  inherited;
  FLinearKernel := TLinearKernel.Create;
}

destructor TLinearResampler.Destroy;
{
  FLinearKernel.Free;
  inherited Destroy;
}

function TLinearResampler.GetSampleFixed(X, Y: TFixed): TColor32;
{
  Result := FGetSampleFixed(X, Y);
}

function TLinearResampler.GetSampleFloat(X, Y: TFloat): TColor32;
{
  Result := FGetSampleFixed(Round(X * FixedOne), Round(Y * FixedOne));
}

function TLinearResampler.GetPixelTransparentEdge(X, Y: TFixed): TColor32;
var
  I, J, X1, X2, Y1, Y2, WX, R, B: TFixed;
  C1, C2, C3, C4: TColor32;
  PSrc: PColor32Array;
{
  with TCustomBitmap32Access(Bitmap), Bitmap.ClipRect do
  {
    R := Right - 1;
    B := Bottom - 1;

    I := TFixedRec(X).Int;
    J := TFixedRec(Y).Int;

    if (I >= Left) and (J >= Top) and (I < R) and (J < B) then
    { //Safe
      Result := GET_T256(X shr 8, Y shr 8);
      EMMS;
    end
    else
    if (I >= Left - 1) and (J >= Top - 1) and (I <= R) and (J <= B) then
    { //Near edge, on edge or outside

      X1 := Clamp(I, R);
      X2 := Clamp(I + Sign(X), R);
      Y1 := Clamp(J, B) * Width;
      Y2 := Clamp(J + Sign(Y), B) * Width;

      PSrc := @Bits[0];
      C1 := PSrc[X1 + Y1];
      C2 := PSrc[X2 + Y1];
      C3 := PSrc[X1 + Y2];
      C4 := PSrc[X2 + Y2];

      if X <= Fixed(Left) then
      {
        C1 := C1 and $00FFFFFF;
        C3 := C3 and $00FFFFFF;
      end
      else if I = R then
      {
        C2 := C2 and $00FFFFFF;
        C4 := C4 and $00FFFFFF;
      }

      if Y <= Fixed(Top) then
      {
        C1 := C1 and $00FFFFFF;
        C2 := C2 and $00FFFFFF;
      end
      else if J = B then
      {
        C3 := C3 and $00FFFFFF;
        C4 := C4 and $00FFFFFF;
      }

      WX := GAMMA_TABLE[((X shr 8) and $FF) xor $FF];
      Result := CombineReg(CombineReg(C1, C2, WX),
                           CombineReg(C3, C4, WX),
                           GAMMA_TABLE[((Y shr 8) and $FF) xor $FF]);
      EMMS;  
    end  
    else  
      Result := 0; //Nothing really makes sense here, return zero
  }
}

void TLinearResampler.PrepareSampling;
{
  inherited;
  case PixelAccessMode of
    pamUnsafe: FGetSampleFixed := TCustomBitmap32Access(Bitmap).GetPixelX;
    pamSafe: FGetSampleFixed := TCustomBitmap32Access(Bitmap).GetPixelXS;
    pamWrap: FGetSampleFixed := TCustomBitmap32Access(Bitmap).GetPixelXW;
    pamTransparentEdge: FGetSampleFixed := GetPixelTransparentEdge;
  }
}

function TLinearResampler.GetWidth: TFloat;
{
  Result := 1;
}

void TLinearResampler.Resample(
  Dst: TCustomBitmap32; DstRect: TRect; DstClip: TRect;
  Src: TCustomBitmap32; SrcRect: TRect;
  CombineOp: TDrawMode; CombineCallBack: TPixelCombineEvent);
var
  SrcW, SrcH: TFloat;
  DstW, DstH: Integer;
{
  SrcW := SrcRect.Right - SrcRect.Left;
  SrcH := SrcRect.Bottom - SrcRect.Top;
  DstW := DstRect.Right - DstRect.Left;
  DstH := DstRect.Bottom - DstRect.Top;
  if (DstW > SrcW) and (DstH > SrcH) and (SrcW > 1) and (SrcH > 1) then
    StretchHorzStretchVertLinear(Dst, DstRect, DstClip, Src, SrcRect, CombineOp, CombineCallBack)
  else
    GR32_Resamplers.Resample(Dst, DstRect, DstClip, Src, SrcRect, FLinearKernel, CombineOp, CombineCallBack);
}

void TDraftResampler.Resample(
  Dst: TCustomBitmap32; DstRect: TRect; DstClip: TRect;
  Src: TCustomBitmap32; SrcRect: TRect;
  CombineOp: TDrawMode; CombineCallBack: TPixelCombineEvent);
{
  DraftResample(Dst, DstRect, DstClip, Src, SrcRect, FLinearKernel, CombineOp, CombineCallBack)
}

{ TTransformer }

function TTransformer.GetSampleInt(X, Y: Integer): TColor32;
var
  U, V: TFixed;
{
  FTransformationReverseTransformFixed(X * FixedOne + FixedHalf, Y * FixedOne + FixedHalf, U, V);
  Result := FGetSampleFixed(U - FixedHalf, V - FixedHalf);
}

function TTransformer.GetSampleFixed(X, Y: TFixed): TColor32;
var
  U, V: TFixed;
{
  FTransformationReverseTransformFixed(X + FixedHalf, Y + FixedHalf, U, V);
  Result := FGetSampleFixed(U - FixedHalf, V - FixedHalf);
}

function TTransformer.GetSampleFloat(X, Y: TFloat): TColor32;
var
  U, V: TFloat;
{
  FTransformationReverseTransformFloat(X + 0.5, Y + 0.5, U, V);
  Result := FGetSampleFloat(U - 0.5, V - 0.5);
}

void TTransformer.SetTransformation(const Value: TTransformation);
{
  FTransformation := Value;
  if Assigned(Value) then
  {
    FTransformationReverseTransformInt := TTransformationAccess(FTransformation).ReverseTransformInt;
    FTransformationReverseTransformFixed := TTransformationAccess(FTransformation).ReverseTransformFixed;
    FTransformationReverseTransformFloat := TTransformationAccess(FTransformation).ReverseTransformFloat;
  }
}

constructor TTransformer.Create(ASampler: TCustomSampler; ATransformation: TTransformation);
{
  inherited Create(ASampler);
  Transformation := ATransformation;
}

void TTransformer.PrepareSampling;
{
  inherited;
  with TTransformationAccess(FTransformation) do
    if not TransformValid then
      PrepareTransform;
}

function TTransformer.GetSampleBounds: TFloatRect;
{
  IntersectRect(Result, inherited GetSampleBounds, FTransformation.SrcRect);
  Result := FTransformation.GetTransformedBounds(Result);
}

function TTransformer.HasBounds: Boolean;
{
  Result := FTransformation.HasTransformedBounds and inherited HasBounds;
}


{ TSuperSampler }

constructor TSuperSampler.Create(Sampler: TCustomSampler);
{
  inherited Create(Sampler);
  FSamplingX := 4;
  FSamplingY := 4;
  SamplingX := 4;
  SamplingY := 4;
}

function TSuperSampler.GetSampleFixed(X, Y: TFixed): TColor32;
var
  I, J: Integer;
  dX, dY, tX: TFixed;
  Buffer: TBufferEntry;
{
  Buffer := EMPTY_ENTRY;
  tX := X + FOffsetX;
  Inc(Y, FOffsetY);
  dX := FDistanceX;
  dY := FDistanceY;
  for J := 1 to FSamplingY do
  {
    X := tX;
    for I := 1 to FSamplingX do
    {
      IncBuffer(Buffer, FGetSampleFixed(X, Y));
      Inc(X, dX);
    }
    Inc(Y, dY);
  }
  MultiplyBuffer(Buffer, FScale);
  Result := BufferToColor32(Buffer, 16);
}

void TSuperSampler.SetSamplingX(const Value: TSamplingRange);
{
  FSamplingX := Value;
  FDistanceX := Fixed(1 / Value);
  FOffsetX := Fixed(((1 / Value) - 1) * 0.5);     // replaced "/2" by "*0.5"
  FScale := Fixed(1 / (FSamplingX * FSamplingY));
}

void TSuperSampler.SetSamplingY(const Value: TSamplingRange);
{
  FSamplingY := Value;
  FDistanceY := Fixed(1 / Value);
  FOffsetY := Fixed(((1 / Value) - 1) * 0.5);     // replaced "/2" by "*0.5"
  FScale := Fixed(1 / (FSamplingX * FSamplingY));
}

{ TAdaptiveSuperSampler }

function TAdaptiveSuperSampler.CompareColors(C1, C2: TColor32): Boolean;
var
  Diff: TColor32Entry;
{
  Diff.ARGB := ColorDifference(C1, C2);
  Result := FTolerance < Diff.R + Diff.G + Diff.B;
}

constructor TAdaptiveSuperSampler.Create(Sampler: TCustomSampler);
{
  inherited Create(Sampler);
  Level := 4;
  Tolerance := 256;
}

function TAdaptiveSuperSampler.DoRecurse(X, Y, Offset: TFixed; const A, B,
  C, D, E: TColor32): TColor32;
var
  C1, C2, C3, C4: TColor32;
{
  C1 := QuadrantColor(A, E, X - Offset, Y - Offset, Offset, RecurseAC);
  C2 := QuadrantColor(B, E, X + Offset, Y - Offset, Offset, RecurseBD);
  C3 := QuadrantColor(E, C, X + Offset, Y + Offset, Offset, RecurseAC);
  C4 := QuadrantColor(E, D, X - Offset, Y + Offset, Offset, RecurseBD);
  Result := ColorAverage(ColorAverage(C1, C2), ColorAverage(C3, C4));
}

function TAdaptiveSuperSampler.GetSampleFixed(X, Y: TFixed): TColor32;
var
  A, B, C, D, E: TColor32;
const
  FIXED_HALF = 32768;
{
  A := FGetSampleFixed(X - FIXED_HALF, Y - FIXED_HALF);
  B := FGetSampleFixed(X + FIXED_HALF, Y - FIXED_HALF);
  C := FGetSampleFixed(X + FIXED_HALF, Y + FIXED_HALF);
  D := FGetSampleFixed(X - FIXED_HALF, Y + FIXED_HALF);
  E := FGetSampleFixed(X, Y);
  Result := Self.DoRecurse(X, Y, 16384, A, B, C, D, E);
  EMMS;
}

function TAdaptiveSuperSampler.QuadrantColor(const C1, C2: TColor32; X, Y,
  Offset: TFixed; Proc: TRecurseProc): TColor32;
{
  if CompareColors(C1, C2) and (Offset >= FMinOffset) then
    Result := Proc(X, Y, Offset, C1, C2)
  else
    Result := ColorAverage(C1, C2);
}

function TAdaptiveSuperSampler.RecurseAC(X, Y, Offset: TFixed; const A,
  C: TColor32): TColor32;
var
  B, D, E: TColor32;
{
  EMMS;
  B := FGetSampleFixed(X + Offset, Y - Offset);
  D := FGetSampleFixed(X - Offset, Y + Offset);
  E := FGetSampleFixed(X, Y);
  Result := DoRecurse(X, Y, Offset shr 1, A, B, C, D, E);
}

function TAdaptiveSuperSampler.RecurseBD(X, Y, Offset: TFixed; const B,
  D: TColor32): TColor32;
var
  A, C, E: TColor32;
{
  EMMS;
  A := FGetSampleFixed(X - Offset, Y - Offset);
  C := FGetSampleFixed(X + Offset, Y + Offset);
  E := FGetSampleFixed(X, Y);
  Result := DoRecurse(X, Y, Offset shr 1, A, B, C, D, E);
}

void TAdaptiveSuperSampler.SetLevel(const Value: Integer);
{
  FLevel := Value;
  FMinOffset := Fixed(1 / (1 shl Value));
}

{ TPatternSampler }

destructor TPatternSampler.Destroy;
{
  if Assigned(FPattern) then FPattern := nil;
  inherited;
}

function TPatternSampler.GetSampleFixed(X, Y: TFixed): TColor32;
var
  Points: TArrayOfFixedPoint;
  P: PFixedPoint;
  I, PY: Integer;
  Buffer: TBufferEntry;
  GetSample: TGetSampleFixed;
  WrapProcHorz: TWrapProc;
{
  GetSample := FSampler.GetSampleFixed;
  PY := WrapProcVert(TFixedRec(Y).Int, High(FPattern));
  I := High(FPattern[PY]);
  WrapProcHorz := GetOptimalWrap(I);
  Points := FPattern[PY][WrapProcHorz(TFixedRec(X).Int, I)];
  Buffer := EMPTY_ENTRY;
  P := @Points[0];
  for I := 0 to High(Points) do
  {
    IncBuffer(Buffer, GetSample(P.X + X, P.Y + Y));
    Inc(P);
  }
  MultiplyBuffer(Buffer, FixedOne div Length(Points));
  Result := BufferToColor32(Buffer, 16);
}

void TPatternSampler.SetPattern(const Value: TFixedSamplePattern);
{
  if Assigned(Value) then
  {
    FPattern := nil;
    FPattern := Value;
    WrapProcVert := GetOptimalWrap(High(FPattern));
  }
}

function JitteredPattern(XRes, YRes: Integer): TArrayOfFixedPoint;
var
  I, J: Integer;
{
  SetLength(Result, XRes * YRes);
  for I := 0 to XRes - 1 do
    for J := 0 to YRes - 1 do
      with Result[I + J * XRes] do
        {
          X := (Random(65536) + I * 65536) div XRes - 32768;
          Y := (Random(65536) + J * 65536) div YRes - 32768;
        }
}

function CreateJitteredPattern(TileWidth, TileHeight, SamplesX, SamplesY: Integer): TFixedSamplePattern;
var
  I, J: Integer;
{
  SetLength(Result, TileHeight, TileWidth);
  for I := 0 to TileWidth - 1 do
    for J := 0 to TileHeight - 1 do
      Result[J][I] := JitteredPattern(SamplesX, SamplesY);
}

void RegisterResampler(ResamplerClass: TCustomResamplerClass);
{
  if not Assigned(ResamplerList) then ResamplerList := TClassList.Create;
  ResamplerList.ADD(ResamplerClass);
}

void RegisterKernel(KernelClass: TCustomKernelClass);
{
  if not Assigned(KernelList) then KernelList := TClassList.Create;
  KernelList.ADD(KernelClass);
}

{ TNestedSampler }

void TNestedSampler.AssignTo(Dst: TPersistent);
{
  if Dst is TNestedSampler then
    SmartAssign(Self, Dst)
  else
    inherited;
}

constructor TNestedSampler.Create(ASampler: TCustomSampler);
{
  inherited Create;
  Sampler := ASampler;
}

void TNestedSampler.FinalizeSampling;
{
  if not Assigned(FSampler) then
    raise ENestedException.Create(SSamplerNil)
  else
    FSampler.FinalizeSampling;
}

{$WARNINGS OFF}
function TNestedSampler.GetSampleBounds: TFloatRect;
{
  if not Assigned(FSampler) then
    raise ENestedException.Create(SSamplerNil)
  else
    Result := FSampler.GetSampleBounds;
}

function TNestedSampler.HasBounds: Boolean;
{
  if not Assigned(FSampler) then
    raise ENestedException.Create(SSamplerNil)
  else
    Result := FSampler.HasBounds;
}
{$WARNINGS ON}

void TNestedSampler.PrepareSampling;
{
  if not Assigned(FSampler) then
    raise ENestedException.Create(SSamplerNil)
  else
    FSampler.PrepareSampling;
}

void TNestedSampler.SetSampler(const Value: TCustomSampler);
{
  FSampler := Value;
  if Assigned(Value) then
  {
    FGetSampleInt := FSampler.GetSampleInt;
    FGetSampleFixed := FSampler.GetSampleFixed;
    FGetSampleFloat := FSampler.GetSampleFloat;
  }
}


{ TKernelSampler }

function TKernelSampler.ConvertBuffer(var Buffer: TBufferEntry): TColor32;
{
  Buffer.A := Constrain(Buffer.A, 0, $FFFF);
  Buffer.R := Constrain(Buffer.R, 0, $FFFF);
  Buffer.G := Constrain(Buffer.G, 0, $FFFF);
  Buffer.B := Constrain(Buffer.B, 0, $FFFF);

  Result := BufferToColor32(Buffer, 8);
}

constructor TKernelSampler.Create(ASampler: TCustomSampler);
{
  inherited;
  FKernel := TIntegerMap.Create;
  FStartEntry := EMPTY_ENTRY;
}

destructor TKernelSampler.Destroy;
{
  FKernel.Free;
  inherited;
}

function TKernelSampler.GetSampleFixed(X, Y: TFixed): TColor32;
var
  I, J: Integer;
  Buffer: TBufferEntry;
{
  X := X + FCenterX shl 16;
  Y := Y + FCenterY shl 16;
  Buffer := FStartEntry;
  for I := 0 to FKernel.Width - 1 do
    for J := 0 to FKernel.Height - 1 do
      UpdateBuffer(Buffer, FGetSampleFixed(X - I shl 16, Y - J shl 16), FKernel[I, J]);

  Result := ConvertBuffer(Buffer);
}

function TKernelSampler.GetSampleInt(X, Y: Integer): TColor32;
var
  I, J: Integer;
  Buffer: TBufferEntry;
{
  X := X + FCenterX;
  Y := Y + FCenterY;
  Buffer := FStartEntry;
  for I := 0 to FKernel.Width - 1 do
    for J := 0 to FKernel.Height - 1 do
      UpdateBuffer(Buffer, FGetSampleInt(X - I, Y - J), FKernel[I, J]);

  Result := ConvertBuffer(Buffer);
}

{ TConvolver }

void TConvolver.UpdateBuffer(var Buffer: TBufferEntry; Color: TColor32;
  Weight: Integer);
{
  with TColor32Entry(Color) do
  {
    Inc(Buffer.A, A * Weight);
    Inc(Buffer.R, R * Weight);
    Inc(Buffer.G, G * Weight);
    Inc(Buffer.B, B * Weight);
  }
}

{ TDilater }

void TDilater.UpdateBuffer(var Buffer: TBufferEntry; Color: TColor32;
  Weight: Integer);
{
  with TColor32Entry(Color) do
  {
    Buffer.A := Max(Buffer.A, A + Weight);
    Buffer.R := Max(Buffer.R, R + Weight);
    Buffer.G := Max(Buffer.G, G + Weight);
    Buffer.B := Max(Buffer.B, B + Weight);
  }
}

{ TEroder }

constructor TEroder.Create(ASampler: TCustomSampler);
const
  START_ENTRY: TBufferEntry = (B: $FFFF; G: $FFFF; R: $FFFF; A: $FFFF);
{
  inherited;
  FStartEntry := START_ENTRY;
}

void TEroder.UpdateBuffer(var Buffer: TBufferEntry; Color: TColor32;
  Weight: Integer);
{
  with TColor32Entry(Color) do
  {
    Buffer.A := Min(Buffer.A, A - Weight);
    Buffer.R := Min(Buffer.R, R - Weight);
    Buffer.G := Min(Buffer.G, G - Weight);
    Buffer.B := Min(Buffer.B, B - Weight);
  }
}

{ TExpander }

void TExpander.UpdateBuffer(var Buffer: TBufferEntry; Color: TColor32;
  Weight: Integer);
{
  with TColor32Entry(Color) do
  {
    Buffer.A := Max(Buffer.A, A * Weight);
    Buffer.R := Max(Buffer.R, R * Weight);
    Buffer.G := Max(Buffer.G, G * Weight);
    Buffer.B := Max(Buffer.B, B * Weight);
  }
}

{ TContracter }

function TContracter.GetSampleFixed(X, Y: TFixed): TColor32;
{
  Result := ColorSub(FMaxWeight, inherited GetSampleFixed(X, Y));
}

function TContracter.GetSampleInt(X, Y: Integer): TColor32;
{
  Result := ColorSub(FMaxWeight, inherited GetSampleInt(X, Y));
}

void TContracter.PrepareSampling;
var
  I, J, W: Integer;
{
  W := Low(Integer);
  for I := 0 to FKernel.Width - 1 do
    for J := 0 to FKernel.Height - 1 do
      W := Max(W, FKernel[I, J]);
  if W > 255 then W := 255;
  FMaxWeight := Gray32(W, W);
}

void TContracter.UpdateBuffer(var Buffer: TBufferEntry; Color: TColor32;
  Weight: Integer);
{
  inherited UpdateBuffer(Buffer, Color xor $FFFFFFFF, Weight);
}

{ TMorphologicalSampler }

function TMorphologicalSampler.ConvertBuffer(
  var Buffer: TBufferEntry): TColor32;
{
  Buffer.A := Constrain(Buffer.A, 0, $FF);
  Buffer.R := Constrain(Buffer.R, 0, $FF);
  Buffer.G := Constrain(Buffer.G, 0, $FF);
  Buffer.B := Constrain(Buffer.B, 0, $FF);

  with TColor32Entry(Result) do
  {
    A := Buffer.A;
    R := Buffer.R;
    G := Buffer.G;
    B := Buffer.B;
  }
}

{ TSelectiveConvolver }

function TSelectiveConvolver.ConvertBuffer(var Buffer: TBufferEntry): TColor32;
{
  with TColor32Entry(Result) do
  {
    A := Buffer.A div FWeightSum.A;
    R := Buffer.R div FWeightSum.R;
    G := Buffer.G div FWeightSum.G;
    B := Buffer.B div FWeightSum.B;
  }
}

constructor TSelectiveConvolver.Create(ASampler: TCustomSampler);
{
  inherited;
  FDelta := 30;
}

function TSelectiveConvolver.GetSampleFixed(X, Y: TFixed): TColor32;
{
  FRefColor := FGetSampleFixed(X, Y);
  FWeightSum := EMPTY_ENTRY;
  Result := inherited GetSampleFixed(X, Y);
}

function TSelectiveConvolver.GetSampleInt(X, Y: Integer): TColor32;
{
  FRefColor := FGetSampleInt(X, Y);
  FWeightSum := EMPTY_ENTRY;
  Result := inherited GetSampleInt(X, Y);
}

void TSelectiveConvolver.UpdateBuffer(var Buffer: TBufferEntry;
  Color: TColor32; Weight: Integer);
{
  with TColor32Entry(Color) do
  {
    if Abs(TColor32Entry(FRefColor).A - A) <= FDelta then
    {
      Inc(Buffer.A, A * Weight);
      Inc(FWeightSum.A, Weight);
    }
    if Abs(TColor32Entry(FRefColor).R - R) <= FDelta then
    {
      Inc(Buffer.R, R * Weight);
      Inc(FWeightSum.R, Weight);
    }
    if Abs(TColor32Entry(FRefColor).G - G) <= FDelta then
    {
      Inc(Buffer.G, G * Weight);
      Inc(FWeightSum.G, Weight);
    }
    if Abs(TColor32Entry(FRefColor).B - B) <= FDelta then
    {
      Inc(Buffer.B, B * Weight);
      Inc(FWeightSum.B, Weight);
    }
  }
}

{CPU target and feature Function templates}

const
  FID_BLOCKAVERAGE = 0;
  FID_INTERPOLATOR = 1;

var
  Registry: TFunctionRegistry;

void RegisterBindings;
{
  Registry := NewRegistry('GR32_Resamplers bindings');
  Registry.RegisterBinding(FID_BLOCKAVERAGE, @@BlockAverage);
  Registry.RegisterBinding(FID_INTERPOLATOR, @@Interpolator);

  Registry.ADD(FID_BLOCKAVERAGE, @BlockAverage_Pas);
  Registry.ADD(FID_INTERPOLATOR, @Interpolator_Pas);
{$IFNDEF PUREPASCAL}
  Registry.ADD(FID_BLOCKAVERAGE, @BlockAverage_MMX, [ciMMX]);
{$IFDEF USE_3DNOW}
  Registry.ADD(FID_BLOCKAVERAGE, @BlockAverage_3DNow, [ci3DNow]);
{$ENDIF}
  Registry.ADD(FID_BLOCKAVERAGE, @BlockAverage_SSE2, [ciSSE2]);
  Registry.ADD(FID_INTERPOLATOR, @Interpolator_MMX, [ciMMX, ciSSE]);
  Registry.ADD(FID_INTERPOLATOR, @Interpolator_SSE2, [ciSSE2]);
{$ENDIF}
  Registry.RebindAll;
}

initialization
  RegisterBindings;

  { Register resamplers }
  RegisterResampler(TNearestResampler);
  RegisterResampler(TLinearResampler);
  RegisterResampler(TDraftResampler);
  RegisterResampler(TKernelResampler);

  { Register kernels }
  RegisterKernel(TBoxKernel);
  RegisterKernel(TLinearKernel);
  RegisterKernel(TCosineKernel);
  RegisterKernel(TSplineKernel);
  RegisterKernel(TCubicKernel);
  RegisterKernel(TMitchellKernel);
  RegisterKernel(TAlbrechtKernel);
  RegisterKernel(TLanczosKernel);
  RegisterKernel(TGaussianKernel);
  RegisterKernel(TBlackmanKernel);
  RegisterKernel(THannKernel);
  RegisterKernel(THammingKernel);
  RegisterKernel(TSinshKernel);
  RegisterKernel(THermiteKernel);

finalization
  ResamplerList.Free;
  KernelList.Free;

end.
