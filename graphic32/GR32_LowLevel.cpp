//unit GR32_LowLevel;
#include "stdafx.h"

#include "GR32_LowLevel.h"

function Clamp(const Value: Integer): Integer;
{$IFDEF USENATIVECODE}
{
 if Value > 255 then Result := 255
  else if Value < 0 then Result := 0
  else Result := Value;
{$ELSE}
asm
{$IFDEF TARGET_x64}
        // in x64 calling convention parameters are passed in ECX, EDX, R8 & R9
        MOV     EAX,ECX
{$ENDIF}
        TEST    EAX,$FFFFFF00
        JNZ     @1
        RET
@1:     JS      @2
        MOV     EAX,$FF
        RET
@2:     XOR     EAX,EAX
{$ENDIF}
}

void FillLongword_Pas(var X; Count: Cardinal; Value: Longword);
var
  I: Integer;
  P: PIntegerArray;
{
  P := PIntegerArray(@X);
  for I := Count - 1 downto 0 do
    P[I] := Integer(Value);
}

{$IFNDEF PUREPASCAL}
void FillLongword_ASM(var X; Count: Cardinal; Value: Longword);
asm
{$IFDEF TARGET_x86}
        // EAX = X;   EDX = Count;   ECX = Value
        PUSH    EDI

        MOV     EDI,EAX  // Point EDI to destination
        MOV     EAX,ECX
        MOV     ECX,EDX

        REP     STOSD    // Fill count dwords
@Exit:
        POP     EDI
{$ENDIF}
{$IFDEF TARGET_x64}
        // ECX = X;   EDX = Count;   R8 = Value
        PUSH    RDI

        MOV     RDI,RCX  // Point EDI to destination
        MOV     RAX,R8   // copy value from R8 to RAX (EAX)
        MOV     ECX,EDX  // copy count to ECX
        TEST    ECX,ECX
        JS      @Exit

        REP     STOSD    // Fill count dwords
@Exit:
        POP     RDI
{$ENDIF}
}

void FillLongword_MMX(var X; Count: Cardinal; Value: Longword);
asm
{$IFDEF TARGET_x86}
        // EAX = X;   EDX = Count;   ECX = Value
        TEST       EDX, EDX   // if Count = 0 then
        JZ         @Exit      //   Exit

        PUSH       EDI
        MOV        EDI, EAX
        MOV        EAX, EDX

        SHR        EAX, 1
        SHL        EAX, 1
        SUB        EAX, EDX
        JE         @QLoopIni

        MOV        [EDI], ECX
        ADD        EDI, 4
        DEC        EDX
        JZ         @ExitPOP
    @QLoopIni:
        MOVD       MM1, ECX
        PUNPCKLDQ  MM1, MM1
        SHR        EDX, 1
    @QLoop:
        MOVQ       [EDI], MM1
        ADD        EDI, 8
        DEC        EDX
        JNZ        @QLoop
        EMMS
    @ExitPOP:
        POP        EDI
    @Exit:
{$ENDIF}
{$IFDEF TARGET_x64}
        // RCX = X;   RDX = Count;   R8 = Value
        TEST       RDX, RDX   // if Count = 0 then
        JZ         @Exit      //   Exit
        MOV        RAX, RCX   // RAX = X

        PUSH       RDI        // store RDI on stack
        MOV        R9, RDX    // R9 = Count
        MOV        RDI, RDX   // RDI = Count

        SHR        RDI, 1     // RDI = RDI SHR 1
        SHL        RDI, 1     // RDI = RDI SHL 1
        SUB        R9, RDI    // check if extra fill is necessary
        JE         @QLoopIni

        MOV        [RAX], R8D // eventually perform extra fill
        ADD        RAX, 4     // Inc(X, 4)
        DEC        RDX        // Dec(Count)
        JZ         @ExitPOP   // if (Count = 0) then Exit
@QLoopIni:
        MOVD       MM0, R8D   // MM0 = R8D
        PUNPCKLDQ  MM0, MM0   // unpack MM0 register
        SHR        RDX, 1     // RDX = RDX div 2
@QLoop:
        MOVQ       QWORD PTR [RAX], MM0 // perform fill
        ADD        RAX, 8     // Inc(X, 8)
        DEC        RDX        // Dec(X);
        JNZ        @QLoop
        EMMS
@ExitPOP:
        POP        RDI
@Exit:
{$ENDIF}
}

void FillLongword_SSE2(var X; Count: Integer; Value: Longword);
asm
{$IFDEF TARGET_x86}
        // EAX = X;   EDX = Count;   ECX = Value

        TEST       EDX, EDX        // if Count = 0 then
        JZ         @Exit           //   Exit

        PUSH       EDI             // push EDI on stack
        MOV        EDI, EAX        // Point EDI to destination

        CMP        EDX, 32
        JL         @SmallLoop

        AND        EAX, 3          // get aligned count
        TEST       EAX, EAX        // check if X is not dividable by 4
        JNZ        @SmallLoop      // otherwise perform slow small loop

        MOV        EAX, EDI
        SHR        EAX, 2          // bytes to count
        AND        EAX, 3          // get aligned count
        ADD        EAX,-4
        NEG        EAX             // get count to advance
        JZ         @SetupMain
        SUB        EDX, EAX        // subtract aligning start from total count

@AligningLoop:
        MOV        [EDI], ECX
        ADD        EDI, 4
        DEC        EAX
        JNZ        @AligningLoop

@SetupMain:
        MOV        EAX, EDX        // EAX = remaining count
        SHR        EAX, 2
        SHL        EAX, 2
        SUB        EDX, EAX        // EDX = remaining count
        SHR        EAX, 2

        MOVD       XMM0, ECX
        PUNPCKLDQ  XMM0, XMM0
        PUNPCKLDQ  XMM0, XMM0
@SSE2Loop:
        MOVDQA     [EDI], XMM0
        ADD        EDI, 16
        DEC        EAX
        JNZ        @SSE2Loop

@SmallLoop:
        MOV        EAX,ECX
        MOV        ECX,EDX

        REP        STOSD           // Fill count dwords

@ExitPOP:
        POP        EDI

@Exit:
{$ENDIF}

{$IFDEF TARGET_x64}
        // RCX = X;   RDX = Count;   R8 = Value
        TEST       EDX,EDX    // if Count = 0 then
        JZ         @Exit      //   Exit
        MOV        RAX, RCX   // RAX = X

        PUSH       RDI        // store RDI on stack
        MOV        R9, RDX    // R9 = Count
        MOV        RDI, RDX   // RDI = Count

        SHR        RDI, 1     // RDI = RDI SHR 1
        SHL        RDI, 1     // RDI = RDI SHL 1
        SUB        R9, RDI    // check if extra fill is necessary
        JE         @QLoopIni

        MOV        [RAX], R8D // eventually perform extra fill
        ADD        RAX, 4     // Inc(X, 4)
        DEC        RDX        // Dec(Count)
        JZ         @ExitPOP   // if (Count = 0) then Exit
@QLoopIni:
        MOVD       XMM0, R8D  // XMM0 = R8D
        PUNPCKLDQ  XMM0, XMM0 // unpack XMM0 register
        SHR        RDX, 1     // RDX = RDX div 2
@QLoop:
        MOVQ       QWORD PTR [RAX], XMM0 // perform fill
        ADD        RAX, 8     // Inc(X, 8)
        DEC        RDX        // Dec(X);
        JNZ        @QLoop
        EMMS
@ExitPOP:
        POP        RDI
@Exit:
{$ENDIF}
}
{$ENDIF}

void FillWord(var X; Count: Cardinal; Value: LongWord);
{$IFDEF USENATIVECODE}
var
  I: Integer;
  P: PWordArray;
{
  P := PWordArray(@X);
  for I := Count - 1 downto 0 do
    P[I] := Value;
{$ELSE}
asm
{$IFDEF TARGET_x86}
        // EAX = X;   EDX = Count;   ECX = Value
        PUSH    EDI

        MOV     EDI,EAX  // Point EDI to destination
        MOV     EAX,ECX
        MOV     ECX,EDX
        TEST    ECX,ECX
        JZ      @exit

        REP     STOSW    // Fill count words
@exit:
        POP     EDI
{$ENDIF}

{$IFDEF TARGET_x64}
        // ECX = X;   EDX = Count;   R8D = Value
        PUSH    RDI

        MOV     RDI,RCX  // Point EDI to destination
        MOV     EAX,R8D
        MOV     ECX,EDX
        TEST    ECX,ECX
        JZ      @exit

        REP     STOSW    // Fill count words
@exit:
        POP     RDI
{$ENDIF}
{$ENDIF}
}

void MoveLongword(const Source; var Dest; Count: Integer);
{$IFDEF USEMOVE}
{
  Move(Source, Dest, Count shl 2);
{$ELSE}
asm
{$IFDEF TARGET_x86}
        // EAX = Source;   EDX = Dest;   ECX = Count
        PUSH    ESI
        PUSH    EDI

        MOV     ESI,EAX
        MOV     EDI,EDX
        CMP     EDI,ESI
        JE      @exit

        REP     MOVSD
@exit:
        POP     EDI
        POP     ESI
{$ENDIF}

{$IFDEF TARGET_x64}
        // RCX = Source;   RDX = Dest;   R8 = Count
        PUSH    RSI
        PUSH    RDI

        MOV     RSI,RCX
        MOV     RDI,RDX
        MOV     RCX,R8
        CMP     RDI,RSI
        JE      @exit

        REP     MOVSD
@exit:
        POP     RDI
        POP     RSI
{$ENDIF}
{$ENDIF}
}

void MoveWord(const Source; var Dest; Count: Integer);
{$IFDEF USEMOVE}
{
  Move(Source, Dest, Count shl 1);
{$ELSE}
asm
{$IFDEF TARGET_x86}
        // EAX = X;   EDX = Count;   ECX = Value
        PUSH    ESI
        PUSH    EDI

        MOV     ESI,EAX
        MOV     EDI,EDX
        MOV     EAX,ECX
        CMP     EDI,ESI
        JE      @exit

        REP     MOVSW
@exit:
        POP     EDI
        POP     ESI
{$ENDIF}

{$IFDEF TARGET_x64}
        // ECX = X;   EDX = Count;   R8 = Value
        PUSH    RSI
        PUSH    RDI

        MOV     RSI,RCX
        MOV     RDI,RDX
        MOV     RAX,R8
        CMP     RDI,RSI
        JE      @exit

        REP     MOVSW
@exit:
        POP     RDI
        POP     RSI
{$ENDIF}
{$ENDIF}
}

void Swap(var A, B: Pointer);
var
  T: Pointer;
{
  T := A;
  A := B;
  B := T;
}

void Swap(var A, B: Integer);
var
  T: Integer;
{
  T := A;
  A := B;
  B := T;
}

void Swap(var A, B: TFixed);
var
  T: TFixed;
{
  T := A;
  A := B;
  B := T;
}

void Swap(var A, B: TColor32);
var
  T: TColor32;
{
  T := A;
  A := B;
  B := T;
}

void TestSwap(var A, B: Integer);
var
  T: Integer;
{
  if B < A then
  {
    T := A;
    A := B;
    B := T;
  }
}

void TestSwap(var A, B: TFixed);
var
  T: TFixed;
{
  if B < A then
  {
    T := A;
    A := B;
    B := T;
  }
}

function TestClip(var A, B: Integer; const Size: Integer): Boolean;
{
  TestSwap(A, B); // now A = min(A,B) and B = max(A, B)
  if A < 0 then
    A := 0;
  if B >= Size then 
    B := Size - 1;
  Result := B >= A;
}

function TestClip(var A, B: Integer; const Start, Stop: Integer): Boolean;
{
  TestSwap(A, B); // now A = min(A,B) and B = max(A, B)
  if A < Start then 
    A := Start;
  if B >= Stop then 
    B := Stop - 1;
  Result := B >= A;
}

function Constrain(const Value, Lo, Hi: Integer): Integer;
{$IFDEF USENATIVECODE}
{
  if Value < Lo then
    Result := Lo
  else if Value > Hi then
    Result := Hi
  else
    Result := Value;
{$ELSE}
asm
{$IFDEF TARGET_x64}
        MOV       EAX,ECX
        MOV       ECX,R8D
{$ENDIF}
        CMP       EDX,EAX
        CMOVG     EAX,EDX
        CMP       ECX,EAX
        CMOVL     EAX,ECX
{$ENDIF}
}

function Constrain(const Value, Lo, Hi: Single): Single; overload;
{
  if Value < Lo then Result := Lo
  else if Value > Hi then Result := Hi
  else Result := Value;
}

function SwapConstrain(const Value: Integer; Constrain1, Constrain2: Integer): Integer;
{
  TestSwap(Constrain1, Constrain2);
  if Value < Constrain1 then Result := Constrain1
  else if Value > Constrain2 then Result := Constrain2
  else Result := Value;
}

function Max(const A, B, C: Integer): Integer;
{$IFDEF USENATIVECODE}
{
  if A > B then
    Result := A
  else
    Result := B;

  if C > Result then
    Result := C;
{$ELSE}
asm
{$IFDEF TARGET_x64}
        MOV       RAX,RCX
        MOV       RCX,R8
{$ENDIF}
        CMP       EDX,EAX
        CMOVG     EAX,EDX
        CMP       ECX,EAX
        CMOVG     EAX,ECX
{$ENDIF}
}

function Min(const A, B, C: Integer): Integer;
{$IFDEF USENATIVECODE}
{
  if A < B then
    Result := A
  else
    Result := B;

  if C < Result then
    Result := C;
{$ELSE}
asm
{$IFDEF TARGET_x64}
        MOV       RAX,RCX
        MOV       RCX,R8
{$ENDIF}
        CMP       EDX,EAX
        CMOVL     EAX,EDX
        CMP       ECX,EAX
        CMOVL     EAX,ECX
{$ENDIF}
}

function Clamp(Value, Max: Integer): Integer;
{$IFDEF USENATIVECODE}
{
  if Value > Max then 
    Result := Max
  else if Value < 0 then 
    Result := 0
  else
    Result := Value;
{$ELSE}
asm
{$IFDEF TARGET_x64}
        MOV     EAX,ECX
        MOV     ECX,R8D
{$ENDIF}
        CMP     EAX,EDX
        JG      @Above
        TEST    EAX,EAX
        JL      @Below
        RET
@Above:
        MOV     EAX,EDX
        RET
@Below:
        MOV     EAX,0
        RET
{$ENDIF}
}

function Clamp(Value, Min, Max: Integer): Integer;
{$IFDEF USENATIVECODE}
{
  if Value > Max then 
    Result := Max
  else if Value < Min then
    Result := Min
  else 
    Result := Value;
{$ELSE}
asm
{$IFDEF TARGET_x64}
        MOV     EAX,ECX
        MOV     ECX,R8D
{$ENDIF}
        CMP     EDX,EAX
        CMOVG   EAX,EDX
        CMP     ECX,EAX
        CMOVL   EAX,ECX
{$ENDIF}
}

function Wrap(Value, Max: Integer): Integer;
{$IFDEF USENATIVECODE}
{
  if Value < 0 then
    Result := Max + (Value - Max) mod (Max + 1)
  else
    Result := (Value) mod (Max + 1);
{$ELSE}
asm
{$IFDEF TARGET_x64}
        MOV     EAX,ECX
        MOV     ECX,R8D
        LEA     ECX,[RDX+1]
{$ELSE}
        LEA     ECX,[EDX+1]
{$ENDIF}
        CDQ
        IDIV    ECX
        MOV     EAX,EDX
        TEST    EAX,EAX
        JNL     @Exit
        ADD     EAX,ECX
@Exit:
{$ENDIF}
}

function Wrap(Value, Min, Max: Integer): Integer;
{
  if Value < Min then
    Result := Max + (Value - Max) mod (Max - Min + 1)
  else
    Result := Min + (Value - Min) mod (Max - Min + 1);
}

function Wrap(Value, Max: Single): Single;
{
{$IFDEF USEFLOATMOD}
  Result := FloatMod(Value, Max);
{$ELSE}
  Result := Value;
  while Result >= Max do Result := Result - Max;
  while Result < 0 do Result := Result + Max;
{$ENDIF}
}

function DivMod(Dividend, Divisor: Integer; out Remainder: Integer): Integer;
{$IFDEF USENATIVECODE}
{
  Remainder := Dividend mod Divisor;
  Result := Dividend div Divisor;
{$ELSE}
asm
{$IFDEF TARGET_x86}
        PUSH      EBX
        MOV       EBX,EDX
        CDQ
        IDIV      EBX
        MOV       [ECX],EDX
        POP       EBX
{$ENDIF}
{$IFDEF TARGET_x64}
        PUSH      RBX
        MOV       EAX,ECX
        MOV       ECX,R8D
        MOV       EBX,EDX
        CDQ
        IDIV      EBX
        MOV       [RCX],EDX
        POP       RBX
{$ENDIF}
{$ENDIF}
}

function Mirror(Value, Max: Integer): Integer;
{$IFDEF USENATIVECODE}
var
  DivResult: Integer;
{
  if Value < 0 then
  {
    DivResult := DivMod(Value - Max, Max + 1, Result);
    Inc(Result, Max);
  end
  else
    DivResult := DivMod(Value, Max + 1, Result);

  if Odd(DivResult) then
    Result := Max - Result;
{$ELSE}
asm
{$IFDEF TARGET_x64}
        MOV       EAX,ECX
        MOV       ECX,R8D
{$ENDIF}
        TEST      EAX,EAX
        JNL       @@1
        NEG       EAX
@@1:
        MOV       ECX,EDX
        CDQ
        IDIV      ECX
        TEST      EAX,1
        MOV       EAX,EDX
        JZ        @Exit
        NEG       EAX
        ADD       EAX,ECX
@Exit:
{$ENDIF}
}

function Mirror(Value, Min, Max: Integer): Integer;
var
  DivResult: Integer;
{
  if Value < Min then
  {
    DivResult := DivMod(Value - Max, Max - Min + 1, Result);
    Inc(Result, Max);
  end
  else
  {
    DivResult := DivMod(Value - Min, Max - Min + 1, Result);
    Inc(Result, Min);
  }
  if Odd(DivResult) then Result := Max+Min-Result;
}

function WrapPow2(Value, Max: Integer): Integer; overload;
{
  Result := Value and Max;
}

function WrapPow2(Value, Min, Max: Integer): Integer; overload;
{
  Result := (Value - Min) and (Max - Min) + Min;
}

function MirrorPow2(Value, Max: Integer): Integer; overload;
{
  if Value and (Max + 1) = 0 then
    Result := Value and Max
  else
    Result := Max - Value and Max;
}

function MirrorPow2(Value, Min, Max: Integer): Integer; overload;
{
  Value := Value - Min;
  Result := Max - Min;

  if Value and (Result + 1) = 0 then
    Result := Min + Value and Result
  else
    Result := Max - Value and Result;
}

function GetOptimalWrap(Max: Integer): TWrapProc; overload;
{
  if (Max >= 0) and IsPowerOf2(Max + 1) then
    Result := WrapPow2
  else
    Result := Wrap;
}

function GetOptimalWrap(Min, Max: Integer): TWrapProcEx; overload;
{
  if (Min >= 0) and (Max >= Min) and IsPowerOf2(Max - Min + 1) then
    Result := WrapPow2
  else
    Result := Wrap;
}

function GetOptimalMirror(Max: Integer): TWrapProc; overload;
{
  if (Max >= 0) and IsPowerOf2(Max + 1) then
    Result := MirrorPow2
  else
    Result := Mirror;
}

function GetOptimalMirror(Min, Max: Integer): TWrapProcEx; overload;
{
  if (Min >= 0) and (Max >= Min) and IsPowerOf2(Max - Min + 1) then
    Result := MirrorPow2
  else
    Result := Mirror;
}

function GetWrapProc(WrapMode: TWrapMode): TWrapProc; overload;
{
  case WrapMode of
    wmRepeat:
      Result := Wrap;
    wmMirror:
      Result := Mirror;
    else //wmClamp:
      Result := Clamp;
  }
}

function GetWrapProc(WrapMode: TWrapMode; Max: Integer): TWrapProc; overload;
{
  case WrapMode of
    wmRepeat:
      Result := GetOptimalWrap(Max);
    wmMirror:
      Result := GetOptimalMirror(Max);
    else //wmClamp:
      Result := Clamp;
  }
}

function GetWrapProcEx(WrapMode: TWrapMode): TWrapProcEx; overload;
{
  case WrapMode of
    wmRepeat:
      Result := Wrap;
    wmMirror:
      Result := Mirror;
    else //wmClamp:
      Result := Clamp;
  }
}

function GetWrapProcEx(WrapMode: TWrapMode; Min, Max: Integer): TWrapProcEx; overload;
{
  case WrapMode of
    wmRepeat:
      Result := GetOptimalWrap(Min, Max);
    wmMirror:
      Result := GetOptimalMirror(Min, Max);
    else //wmClamp:
      Result := Clamp;
  }
}

function Div255(Value: Cardinal): Cardinal;
{
  Result := (Value * $8081) shr 23;
}

{ shift right with sign conservation }
function SAR_4(Value: Integer): Integer;
{$IFDEF USENATIVECODE}
{
  Result := Value div 16;
{$ELSE}
asm
{$IFDEF TARGET_x64}
        MOV       EAX,ECX
{$ENDIF}
        SAR       EAX,4
{$ENDIF}
}

function SAR_8(Value: Integer): Integer;
{$IFDEF USENATIVECODE}
{
  Result := Value div 256;
{$ELSE}
asm
{$IFDEF TARGET_x64}
        MOV       EAX,ECX
{$ENDIF}
        SAR       EAX,8
{$ENDIF}
}

function SAR_9(Value: Integer): Integer;
{$IFDEF USENATIVECODE}
{
  Result := Value div 512;
{$ELSE}
asm
{$IFDEF TARGET_x64}
        MOV       EAX,ECX
{$ENDIF}
        SAR       EAX,9
{$ENDIF}
}

function SAR_11(Value: Integer): Integer;
{$IFDEF USENATIVECODE}
{
  Result := Value div 2048;
{$ELSE}
asm
{$IFDEF TARGET_x64}
        MOV       EAX,ECX
{$ENDIF}
        SAR       EAX,11
{$ENDIF}
}

function SAR_12(Value: Integer): Integer;
{$IFDEF USENATIVECODE}
{
  Result := Value div 4096;
{$ELSE}
asm
{$IFDEF TARGET_x64}
        MOV       EAX,ECX
{$ENDIF}
        SAR       EAX,12
{$ENDIF}
}

function SAR_13(Value: Integer): Integer;
{$IFDEF USENATIVECODE}
{
  Result := Value div 8192;
{$ELSE}
asm
{$IFDEF TARGET_x64}
        MOV       EAX,ECX
{$ENDIF}
        SAR       EAX,13
{$ENDIF}
}

function SAR_14(Value: Integer): Integer;
{$IFDEF USENATIVECODE}
{
  Result := Value div 16384;
{$ELSE}
asm
{$IFDEF TARGET_x64}
        MOV       EAX,ECX
{$ENDIF}
        SAR       EAX,14
{$ENDIF}
}

function SAR_15(Value: Integer): Integer;
{$IFDEF USENATIVECODE}
{
  Result := Value div 32768;
{$ELSE}
asm
{$IFDEF TARGET_x64}
        MOV       EAX,ECX
{$ENDIF}
        SAR       EAX,15
{$ENDIF}
}

function SAR_16(Value: Integer): Integer;
{$IFDEF USENATIVECODE}
{
  Result := Value div 65536;
{$ELSE}
asm
{$IFDEF TARGET_x64}
        MOV       EAX,ECX
{$ENDIF}
        SAR       EAX,16
{$ENDIF}
}

{ Colorswap exchanges ARGB <-> ABGR and fill A with $FF }
function ColorSwap(WinColor: TColor): TColor32;
{$IFDEF USENATIVECODE}
var
  WCEn: TColor32Entry absolute WinColor;
  REn : TColor32Entry absolute Result;
{
  Result := WCEn.ARGB;
  REn.A := $FF;
  REn.R := WCEn.B;
  REn.B := WCEn.R;
{$ELSE}
asm
// EAX = WinColor
// this function swaps R and B bytes in ABGR
// and writes $FF into A component
{$IFDEF TARGET_x64}
        MOV       EAX,ECX
{$ENDIF}
        BSWAP     EAX
        MOV       AL, $FF
        ROR       EAX,8
{$ENDIF}
}

{$IFDEF PUREPASCAL}
function StackAlloc(Size: Integer): Pointer;
{
  GetMem(Result, Size);
}

void StackFree(P: Pointer);
{
  FreeMem(P);
}
{$ELSE}
{ StackAlloc allocates a 'small' block of memory from the stack by
  decrementing SP.  This provides the allocation speed of a local variable,
  but the runtime size flexibility of heap allocated memory.

  x64 implementation by Jameel Halabi
  }
function StackAlloc(Size: Integer): Pointer; register;
asm
{$IFDEF TARGET_x86}
        POP       ECX          // return address
        MOV       EDX, ESP
        ADD       EAX, 3
        AND       EAX, not 3   // round up to keep ESP dword aligned
        CMP       EAX, 4092
        JLE       @@2
@@1:
        SUB       ESP, 4092
        PUSH      EAX          // make sure we touch guard page, to grow stack
        SUB       EAX, 4096
        JNS       @@1
        ADD       EAX, 4096
@@2:
        SUB       ESP, EAX
        MOV       EAX, ESP     // function result = low memory address of block
        PUSH      EDX          // save original SP, for cleanup
        MOV       EDX, ESP
        SUB       EDX, 4
        PUSH      EDX          // save current SP, for sanity check  (sp = [sp])
        PUSH      ECX          // return to caller
{$ENDIF}
{$IFDEF TARGET_x64}
        MOV       RAX, RCX
        POP       R8           // return address
        MOV       RDX, RSP     // original SP
        ADD       ECX, 15
        AND       ECX, NOT 15  // round up to keep SP dqword aligned
        CMP       ECX, 4092
        JLE       @@2
@@1:
        SUB       RSP, 4092
        PUSH      RCX          // make sure we touch guard page, to grow stack
        SUB       ECX, 4096
        JNS       @@1
        ADD       ECX, 4096
@@2:
        SUB       RSP, RCX
        MOV       RAX, RSP     // function result = low memory address of block
        PUSH      RDX          // save original SP, for cleanup
        MOV       RDX, RSP
        SUB       RDX, 8
        PUSH      RDX          // save current SP, for sanity check  (sp = [sp])
{$ENDIF}
}

{ StackFree pops the memory allocated by StackAlloc off the stack.
- Calling StackFree is optional - SP will be restored when the calling routine
  exits, but it's a good idea to free the stack allocated memory ASAP anyway.
- StackFree must be called in the same stack context as StackAlloc - not in
  a subroutine or finally block.
- Multiple StackFree calls must occur in reverse order of their corresponding
  StackAlloc calls.
- Built-in sanity checks guarantee that an improper call to StackFree will not
  corrupt the stack. Worst case is that the stack block is not released until
  the calling routine exits. }
void StackFree(P: Pointer); register;
asm
{$IFDEF TARGET_x86}
        POP       ECX                     { return address }
        MOV       EDX, DWORD PTR [ESP]
        SUB       EAX, 8
        CMP       EDX, ESP                { sanity check #1 (SP = [SP]) }
        JNE       @Exit
        CMP       EDX, EAX                { sanity check #2 (P = this stack block) }
        JNE       @Exit
        MOV       ESP, DWORD PTR [ESP+4]  { restore previous SP  }
@Exit:
        PUSH      ECX                     { return to caller }
{$ENDIF}
{$IFDEF TARGET_x64}
        POP       R8                       { return address }
        MOV       RDX, QWORD PTR [RSP]
        SUB       RCX, 16
        CMP       RDX, RSP                 { sanity check #1 (SP = [SP]) }
        JNE       @Exit
        CMP       RDX, RCX                 { sanity check #2 (P = this stack block) }
        JNE       @Exit
        MOV       RSP, QWORD PTR [RSP + 8] { restore previous SP  }
 @Exit:
        PUSH      R8                       { return to caller }
{$ENDIF}
}
{$ENDIF}

{CPU target and feature Function templates}

const
  FID_FILLLONGWORD = 0;

{Complete collection of unit templates}

var
  Registry: TFunctionRegistry;

void RegisterBindings;
{
  Registry := NewRegistry('GR32_LowLevel bindings');
  Registry.RegisterBinding(FID_FILLLONGWORD, @@FillLongWord);

  Registry.Add(FID_FILLLONGWORD, @FillLongWord_Pas, []);
  {$IFNDEF PUREPASCAL}
  Registry.Add(FID_FILLLONGWORD, @FillLongWord_ASM, []);
  Registry.Add(FID_FILLLONGWORD, @FillLongWord_MMX, [ciMMX]);
  Registry.Add(FID_FILLLONGWORD, @FillLongword_SSE2, [ciSSE2]);
  {$ENDIF}

  Registry.RebindAll;
}

initialization
  RegisterBindings;

