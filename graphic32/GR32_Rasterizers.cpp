//unit GR32_Rasterizers;
#include "stdafx.h"

#include "GR32_Rasterizers.h"

#include "GR32_Resamplers.h"
#include "GR32_Containers.h"
#include "GR32_System.h"
#include "GR32_Math.h"


type
  TThreadPersistentAccess = class(TThreadPersistent);

  TLineRasterizerData = record
    ScanLine: Integer;
  }
  PLineRasterizerData = ^TLineRasterizerData;

  TScanLineRasterizerThread = class(TThread)
  protected
    Data: PLineRasterizerData;
    DstRect: TRect;
    Dst: TCustomBitmap32;
    GetSample: TGetSampleInt;
    AssignColor: TAssignColor;
    procedure Execute; override;
  }

function CombineInfo(Bitmap: TCustomBitmap32): TCombineInfo;
{
  with Result do
  {
    SrcAlpha := Bitmap.MasterAlpha;
    DrawMode := Bitmap.DrawMode;
    CombineMode := Bitmap.CombineMode;
    CombineCallBack := Bitmap.OnPixelCombine;
    if (DrawMode = dmCustom) and not Assigned(CombineCallBack) then
      DrawMode := dmOpaque;
    TransparentColor := Bitmap.OuterColor;
  }
}


{ TRasterizer }

procedure TRasterizer.AssignColorBlend(var Dst: TColor32; Src: TColor32);
{
  FBlendMemEx(Src, Dst, FSrcAlpha);
  EMMS;
}

procedure TRasterizer.AssignColorOpaque(var Dst: TColor32; Src: TColor32);
{
  Dst := Src;
}

procedure TRasterizer.AssignColorCustom(var Dst: TColor32; Src: TColor32);
{
  FCombineCallBack(Src, Dst, FSrcAlpha);
}

procedure TRasterizer.AssignColorTransparent(var Dst: TColor32;
  Src: TColor32);
{
  if Src <> FTransparentColor then Dst := Src;
}

procedure TRasterizer.AssignTo(Dst: TPersistent);
{
  if Dst is TRasterizer then
    SmartAssign(Self, Dst)
  else
    inherited;
}

procedure TRasterizer.Rasterize(Dst: TCustomBitmap32; const DstRect: TRect;
  Src: TCustomBitmap32);
{
  Rasterize(Dst, DstRect, CombineInfo(Src));
}

procedure TRasterizer.Rasterize(Dst: TCustomBitmap32; const DstRect: TRect;
  const CombineInfo: TCombineInfo);
{
  SetCombineInfo(CombineInfo);
  Rasterize(Dst, DstRect);
}

procedure TRasterizer.SetCombineInfo(const CombineInfo: TCombineInfo);
{
  with CombineInfo do
  {
    FTransparentColor := TransparentColor;

    FSrcAlpha := SrcAlpha;
    FBlendMemEx := BLEND_MEM_EX[CombineMode]^;
    FCombineCallBack := CombineCallBack;

    case DrawMode of
      dmOpaque: FAssignColor := AssignColorOpaque;
      dmBlend:  FAssignColor := AssignColorBl}
      dmTransparent: FAssignColor := AssignColorTransparent;
    else
      if Assigned(FCombineCallback) then
        FAssignColor := AssignColorCustom
      else
        FAssignColor := AssignColorBl}
    }
  }
}

procedure TRasterizer.Rasterize(Dst: TCustomBitmap32; const DstRect: TRect);
var
  UpdateCount: Integer;
  R: TRect;
{
  UpdateCount := TThreadPersistentAccess(Dst).UpdateCount;
  if Assigned(FSampler) then
  {
    FSampler.PrepareSampling;
    IntersectRect(R, DstRect, Dst.BoundsRect);
    if FSampler.HasBounds then
      IntersectRect(R, DstRect, MakeRect(FSampler.GetSampleBounds, rrOutside));
    try
      DoRasterize(Dst, R);
    finally
      while TThreadPersistentAccess(Dst).UpdateCount > UpdateCount do
        TThreadPersistentAccess(Dst).EndUpdate;
      FSampler.FinalizeSampling;
    }
  }
}

procedure TRasterizer.SetSampler(const Value: TCustomSampler);
{
  if FSampler <> Value then
  {
    FSampler := Value;
    Changed;
  }
}

procedure TRasterizer.Rasterize(Dst: TCustomBitmap32);
{
  Rasterize(Dst, Dst.BoundsRect);
}

constructor TRasterizer.Create;
{
  inherited;
  SetCombineInfo(DEFAULT_COMBINE_INFO);
}

procedure TRasterizer.Assign(Source: TPersistent);
{
  {Update;
  try
    if Source is TCustomBitmap32 then
      SetCombineInfo(CombineInfo(TCustomBitmap32(Source)))
    else
      inherited;
  finally
    EndUpdate;
    Changed;
  }
}

{ TRegularRasterizer }

constructor TRegularRasterizer.Create;
{
  inherited;
  FUpdateRowCount := 0;
}

procedure TRegularRasterizer.DoRasterize(Dst: TCustomBitmap32; DstRect: TRect);
var
  I, J, UpdateCount: Integer;
  P: PColor32;
  GetSample: TGetSampleInt;
{
  GetSample := FSampler.GetSampleInt;
  UpdateCount := 0;
  for J := DstRect.Top to DstRect.Bottom - 1 do
  {
    P := @Dst.Bits[DstRect.Left + J * Dst.Width];
    for I := DstRect.Left to DstRect.Right - 1 do
    {
      AssignColor(P^, GetSample(I, J));
      Inc(P);
    }
    Inc(UpdateCount);
    if UpdateCount = FUpdateRowCount then
    {
      Dst.Changed(Rect(DstRect.Left, J - UpdateCount, DstRect.Right, J));
      UpdateCount := 0;
    }
  }
  with DstRect do
    Dst.Changed(Rect(Left, Bottom - UpdateCount - 1, Right, Bottom));
}

{ TSwizzlingRasterizer }

constructor TSwizzlingRasterizer.Create;
{
  inherited;
  FBlockSize := 3;
}

procedure TSwizzlingRasterizer.DoRasterize(Dst: TCustomBitmap32; DstRect: TRect);
var
  I, L, T, W, H, Size, RowSize, D: Integer;
  P1, P2, PBlock: TPoint;
  GetSample: TGetSampleInt;
  ForwardBuffer: array of Integer;

  function GetDstCoord(P: TPoint): TPoint;
  var
    XI, YI: Integer;
  {
    Result := P;
    Inc(Result.X);
    Inc(Result.Y);

    XI := ForwardBuffer[Result.X];
    YI := ForwardBuffer[Result.Y];

    if XI <= YI then
      Dec(Result.Y, 1 shl XI)
    else
      Dec(Result.X, 1 shl (YI + 1));

    if Result.Y >= H then
    {
      Result.Y := P.Y + 1 shl YI;
      Result.X := P.X;
      Result := GetDstCoord(Result);
    }

    if Result.X >= W then
    {
      Result.X := P.X + 1 shl XI;
      Result.Y := P.Y;
      Result := GetDstCoord(Result);
    }
  }

{
  W := DstRect.Right - DstRect.Left;
  H := DstRect.Bottom - DstRect.Top;
  L := DstRect.Left; T := DstRect.Top;
  Size := NextPowerOf2(Max(W, H));

  SetLength(ForwardBuffer, Size);

  I := 2;
  while I < Size do
  {
    ForwardBuffer[I] := ForwardBuffer[I shr 1] + 1;
    Inc(I, 2);
  }

  Size := W * H - 1;
  GetSample := FSampler.GetSampleInt;

  D := 1 shl FBlockSize;
  PBlock := Point(L + D, T + D);
  P1 := Point(-1, 0);

  RowSize := Dst.Width;
  for I := 0 to Size do
  {
    P1 := GetDstCoord(P1);
    P2.X := L + P1.X;
    P2.Y := T + P1.Y;
    AssignColor(Dst.Bits[P2.X + P2.Y * RowSize], GetSample(P2.X, P2.Y));

    // Invalidate the current block
    if (P2.X >= PBlock.X) or (P2.Y >= PBlock.Y) then
    {
      Dst.Changed(Rect(PBlock.X - D, PBlock.Y - D, PBlock.X, PBlock.Y));
      PBlock.X := P2.X + D;
      PBlock.Y := P2.Y + D;
    }
  }
  Dst.Changed(Rect(PBlock.X - D, PBlock.Y - D, PBlock.X, PBlock.Y));
}

procedure TSwizzlingRasterizer.SetBlockSize(const Value: Integer);
{
  if FBlockSize <> Value then
  {
    FBlockSize := Value;
    Changed;
  }
}

{ TProgressiveRasterizer }

constructor TProgressiveRasterizer.Create;
{
  inherited;
  FSteps := 4;
  FUpdateRows := True;
}

procedure TProgressiveRasterizer.DoRasterize(Dst: TCustomBitmap32;
  DstRect: TRect);
var
  I, J, Shift, W, H, B, Wk, Hk, X, Y: Integer;
  DoUpdate: Boolean;
  OnChanged: TAreaChangedEvent;
  Step: Integer;
  GetSample: TGetSampleInt;
{
    GetSample := FSampler.GetSampleInt;
    OnChanged := Dst.OnAreaChanged;
    DoUpdate := (TThreadPersistentAccess(Dst).UpdateCount = 0) and Assigned(OnChanged);
    W := DstRect.Right - DstRect.Left;
    H := DstRect.Bottom - DstRect.Top;
    J := DstRect.Top;
    Step := 1 shl FSteps;
    while J < DstRect.Bottom do
    {
      I := DstRect.Left;
      B := Min(J + Step, DstRect.Bottom);
      while I < DstRect.Right - Step do
      {
        Dst.FillRect(I, J, I + Step, B, GetSample(I, J));
        Inc(I, Step);
      }
      Dst.FillRect(I, J, DstRect.Right, B, GetSample(I, J));
      if DoUpdate and FUpdateRows then
        OnChanged(Dst, Rect(DstRect.Left, J, DstRect.Right, B), AREAINFO_RECT);
      Inc(J, Step);
    }
    if DoUpdate and (not FUpdateRows) then OnChanged(Dst, DstRect, AREAINFO_RECT);

    Shift := FSteps;
    while Step > 1 do
    {
      Dec(Shift);
      Step := Step div 2;
      Wk := W div Step - 1;
      Hk := H div Step;
      for J := 0 to Hk do
      {
        Y := DstRect.Top + J shl Shift;
        B := Min(Y + Step, DstRect.Bottom);
        if Odd(J) then
          for I := 0 to Wk do
          {
            X := DstRect.Left + I shl Shift;
            Dst.FillRect(X, Y, X + Step, B, GetSample(X, Y));
          end
        else
          for I := 0 to Wk do
            if Odd(I) then
            {
              X := DstRect.Left + I shl Shift;
              Dst.FillRect(X, Y, X + Step, B, GetSample(X, Y));
            }
        X := DstRect.Left + Wk shl Shift;
        Dst.FillRect(X, Y, DstRect.Right, B, GetSample(X, Y));
        if FUpdateRows and DoUpdate then
          OnChanged(Dst, Rect(DstRect.Left, Y, DstRect.Right, B), AREAINFO_RECT);
      }
      if DoUpdate and (not FUpdateRows) then OnChanged(Dst, DstRect, AREAINFO_RECT);
    }
}

procedure TProgressiveRasterizer.SetSteps(const Value: Integer);
{
  if FSteps <> Value then
  {
    FSteps := Value;
    Changed;
  }
}

procedure TProgressiveRasterizer.SetUpdateRows(const Value: Boolean);
{
  if FUpdateRows <> Value then
  {
    FUpdateRows := Value;
    Changed;
  }
}

{ TTesseralRasterizer }

procedure TTesseralRasterizer.DoRasterize(Dst: TCustomBitmap32; DstRect: TRect);
var
  W, H, I: Integer;
  GetSample: TGetSampleInt;

  procedure SplitHorizontal(X, Y, Width, Height: Integer); forward;

  procedure SplitVertical(X, Y, Width, Height: Integer);
  var
    HalfWidth, X2, I: Integer;
  {
    HalfWidth := Width div 2;
    if HalfWidth > 0 then
    {
      X2 := X + HalfWidth;
      for I := Y + 1 to Y + Height - 1 do
        AssignColor(Dst.PixelPtr[X2, I]^, GetSample(X2, I));
      Dst.Changed(Rect(X2, Y, X2 + 1, Y + Height));
      SplitHorizontal(X, Y, HalfWidth, Height);
      SplitHorizontal(X2, Y, Width - HalfWidth, Height);
    }
  }

  procedure SplitHorizontal(X, Y, Width, Height: Integer);
  var
    HalfHeight, Y2, I: Integer;
  {
    HalfHeight := Height div 2;
    if HalfHeight > 0 then
    {
      Y2 := Y + HalfHeight;
      for I := X + 1 to X + Width - 1 do
        AssignColor(Dst.PixelPtr[I, Y2]^, GetSample(I, Y2));
      Dst.Changed(Rect(X, Y2, X + Width, Y2 + 1));
      SplitVertical(X, Y, Width, HalfHeight);
      SplitVertical(X, Y2, Width, Height - HalfHeight);
    }
  }

{
  GetSample := FSampler.GetSampleInt;
  with DstRect do
  {
    W := Right - Left;
    H := Bottom - Top;
    for I := Left to Right - 1 do
      AssignColor(Dst.PixelPtr[I, Top]^, GetSample(I, Top));
    Dst.Changed(Rect(Left, Top, Right, Top + 1));
    for I := Top to Bottom - 1 do
      AssignColor(Dst.PixelPtr[Left, I]^, GetSample(Left, I));
    Dst.Changed(Rect(Left, Top, Left + 1, Bottom));
    if W > H then
      SplitVertical(Left, Top, W, H)
    else
      SplitHorizontal(Left, Top, W, H);
  }
}


{ TContourRasterizer }

procedure InflateRect(const P: TPoint; var R: TRect);
{
  if P.X < R.Left then R.Left := P.X;
  if P.Y < R.Top then R.Top := P.Y;
  if P.X >= R.Right then R.Right := P.X + 1;
  if P.Y >= R.Bottom then R.Bottom := P.Y + 1;
}

procedure TContourRasterizer.DoRasterize(Dst: TCustomBitmap32; DstRect: TRect);
type
  TDirection = (North, East, South, West);
var
  I, J, D, Diff: Integer;
  C, CLast: TColor32;
  P, PLast: TPoint;
  GetSample: TGetSampleInt;
  NewDir, Dir: TDirection;
  Visited: TBooleanMap;
  UpdateRect: TRect;
const
  LEFT: array[TDirection] of TDirection = (West, North, East, South);
  RIGHT: array[TDirection] of TDirection = (East, South, West, North);
  COORDS: array[TDirection] of TPoint = ((X: 0; Y: -1), (X: 1; Y: 0), (X: 0; Y: 1), (X: -1; Y: 0));
label
  MainLoop;
{
  GetSample := FSampler.GetSampleInt;
  Visited := TBooleanMap.Create;
  try
    with DstRect do
      Visited.SetSize(Right - Left, Bottom - Top);

    I := 0; J := 0;
    Dir := East;
    NewDir := East;

    PLast := Point(DstRect.Left, DstRect.Top);
    CLast := GetSample(PLast.X, PLast.Y);
    AssignColor(Dst.PixelPtr[PLast.X, PLast.Y]^, CLast);

    UpdateRect := Rect(PLast.X, PLast.Y, PLast.X + 1, PLast.Y + 1);
    while True do
    {
      MainLoop:

      Diff := MaxInt;

      // forward
      with COORDS[Dir] do P := Point(PLast.X + X, PLast.Y + Y);
      if PtInRect(DstRect, P) and (not Visited[P.X, P.Y]) then
      {
        C := GetSample(P.X, P.Y);
        Diff := Intensity(ColorSub(C, CLast));
        EMMS;
        NewDir := Dir;
        AssignColor(Dst.PixelPtr[P.X, P.Y]^, C);
        Visited[P.X - DstRect.Left, P.Y - DstRect.Top] := True;
        InflateRect(P, UpdateRect);
      }

      // left
      with COORDS[LEFT[Dir]] do P := Point(PLast.X + X, PLast.Y + Y);
      if PtInRect(DstRect, P) and (not Visited[P.X, P.Y]) then
      {
        C := GetSample(P.X, P.Y);
        D := Intensity(ColorSub(C, CLast));
        EMMS;        
        if D < Diff then
        {
          NewDir := LEFT[Dir];
          Diff := D;
        }
        AssignColor(Dst.PixelPtr[P.X, P.Y]^, C);
        Visited[P.X - DstRect.Left, P.Y - DstRect.Top] := True;
        InflateRect(P, UpdateRect);
      }

      // right
      with COORDS[RIGHT[Dir]] do P := Point(PLast.X + X, PLast.Y + Y);
      if PtInRect(DstRect, P) and (not Visited[P.X, P.Y]) then
      {
        C := GetSample(P.X, P.Y);
        D := Intensity(ColorSub(C, CLast));
        EMMS;        
        if D < Diff then
        {
          NewDir := RIGHT[Dir];
          Diff := D;
        }
        AssignColor(Dst.PixelPtr[P.X, P.Y]^, C);
        Visited[P.X - DstRect.Left, P.Y - DstRect.Top] := True;
        InflateRect(P, UpdateRect);
      }

      if Diff = MaxInt then
      {
        Dst.Changed(UpdateRect);
        while J < Visited.Height do
        {
          while I < Visited.Width do
          {
            if not Visited[I, J] then
            {
              Visited[I, J] := True;
              PLast := Point(DstRect.Left + I, DstRect.Top + J);
              CLast := GetSample(PLast.X, PLast.Y);
              AssignColor(Dst.PixelPtr[PLast.X, PLast.Y]^, CLast);
              UpdateRect := Rect(PLast.X, PLast.Y, PLast.X + 1, PLast.Y + 1);
              goto MainLoop;
            }
            Inc(I);
          }
          I := 0;
          Inc(J);
        }
        Break;
      }

      Dir := NewDir;
      with COORDS[Dir] do PLast := Point(PLast.X + X, PLast.Y + Y);
      CLast := Dst[PLast.X, PLast.Y];
    }

  finally
    Visited.Free;
  }
}

{ TMultithreadedRegularRasterizer }

procedure TMultithreadedRegularRasterizer.DoRasterize(Dst: TCustomBitmap32; DstRect: TRect);
var
  I: Integer;
  Threads: array of TScanLineRasterizerThread;
  Data: TLineRasterizerData;

  function CreateThread: TScanLineRasterizerThread;
  {
    Result := TScanLineRasterizerThread.Create(True);
    Result.Data := @Data;
    Result.DstRect := DstRect;
    Result.GetSample := Sampler.GetSampleInt;
    Result.AssignColor := AssignColor;
    Result.Dst := Dst;
  {$IFDEF USETHREADRESUME}
    Result.Resume;
  {$ELSE}
    Result.Start;
  {$ENDIF}
  }

{
  Data.ScanLine := DstRect.Top - 1;

  { Start Threads }
  SetLength(Threads, NumberOfProcessors);
  try
    for I := 0 to NumberOfProcessors - 1 do
      Threads[I] := CreateThread;

    { Wait for Threads to be ready }
    for I := 0 to High(Threads) do
    {
      Threads[I].WaitFor;
      Threads[I].Free;
    }

  finally
    Dst.Changed(DstRect);
  }
}

{ TLineRasterizerThread }

procedure TScanLineRasterizerThread.Execute;
var
  ScanLine: Integer;
  I: Integer;
  P: PColor32;
{
  ScanLine := InterlockedIncrement(Data^.ScanLine);
  while ScanLine < DstRect.Bottom do
  {
    P := @Dst.Bits[DstRect.Left + ScanLine * Dst.Width];

    for I := DstRect.Left to DstRect.Right - 1 do
    {
      AssignColor(P^, GetSample(I, ScanLine));
      Inc(P);
    }

    ScanLine := InterlockedIncrement(Data^.ScanLine);
  }
}

initialization
  NumberOfProcessors := GetProcessorCount;
{$IFDEF USEMULTITHREADING}
  if NumberOfProcessors > 1 then
    DefaultRasterizerClass := TMultithreadedRegularRasterizer;
{$ENDIF}


end.
