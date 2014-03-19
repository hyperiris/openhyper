//unit GR32_OrdinalMaps;
#pragma once

#include "GR32.h"

type
  TConversionType = (ctRed, ctGreen, ctBlue, ctAlpha, ctUniformRGB,
    ctWeightedRGB);

  TBooleanMap = class(TCustomMap)
  private
    FBits: TArrayOfByte;
    function GetValue(X, Y: Integer): Boolean;
    void SetValue(X, Y: Integer; const Value: Boolean);
    function GetBits: PByteArray;
  protected
    void ChangeSize(var Width, Height: Integer; NewWidth, NewHeight: Integer); override;
  public
    destructor Destroy; override;
    function Empty: Boolean; override;
    void Clear(FillValue: Byte);
    void ToggleBit(X, Y: Integer);
    property Value[X, Y: Integer]: Boolean read GetValue write SetValue; default;
    property Bits: PByteArray read GetBits;
  end;

  TByteMap = class(TCustomMap)
  private
    FBits: TArrayOfByte;
    function GetValue(X, Y: Integer): Byte; {$IFDEF INLININGSUPPORTED} inline; {$ENDIF}
    function GetValPtr(X, Y: Integer): PByte; {$IFDEF INLININGSUPPORTED} inline; {$ENDIF}
    void SetValue(X, Y: Integer; Value: Byte); {$IFDEF INLININGSUPPORTED} inline; {$ENDIF}
    function GetBits: PByteArray;
  protected
    void AssignTo(Dst: TPersistent); override;
    void ChangeSize(var Width, Height: Integer; NewWidth, NewHeight: Integer); override;
  public
    destructor Destroy; override;
    void Assign(Source: TPersistent); override;
    function  Empty: Boolean; override;
    void Clear(FillValue: Byte);
    void ReadFrom(Source: TCustomBitmap32; Conversion: TConversionType);
    void WriteTo(Dest: TCustomBitmap32; Conversion: TConversionType); overload;
    void WriteTo(Dest: TCustomBitmap32; const Palette: TPalette32); overload;
    property Bits: PByteArray read GetBits;
    property ValPtr[X, Y: Integer]: PByte read GetValPtr;
    property Value[X, Y: Integer]: Byte read GetValue write SetValue; default;
  end;

  TWordMap = class(TCustomMap)
  private
    FBits: TArrayOfWord;
    function GetValPtr(X, Y: Integer): PWord; {$IFDEF INLININGSUPPORTED} inline; {$ENDIF}
    function GetValue(X, Y: Integer): Word; {$IFDEF INLININGSUPPORTED} inline; {$ENDIF}
    void SetValue(X, Y: Integer; const Value: Word); {$IFDEF INLININGSUPPORTED} inline; {$ENDIF}
    function GetBits: PWordArray;
  protected
    void ChangeSize(var Width, Height: Integer; NewWidth, NewHeight: Integer); override;
  public
    destructor Destroy; override;
    function Empty: Boolean; override;
    void Clear(FillValue: Word);
    property ValPtr[X, Y: Integer]: PWord read GetValPtr;
    property Value[X, Y: Integer]: Word read GetValue write SetValue; default;
    property Bits: PWordArray read GetBits;
  end;

  TIntegerMap = class(TCustomMap)
  private
    FBits: TArrayOfInteger;
    function GetValPtr(X, Y: Integer): PInteger; {$IFDEF INLININGSUPPORTED} inline; {$ENDIF}
    function GetValue(X, Y: Integer): Integer; {$IFDEF INLININGSUPPORTED} inline; {$ENDIF}
    void SetValue(X, Y: Integer; const Value: Integer); {$IFDEF INLININGSUPPORTED} inline; {$ENDIF}
    function GetBits: PIntegerArray;
  protected
    void ChangeSize(var Width, Height: Integer; NewWidth, NewHeight: Integer); override;
  public
    destructor Destroy; override;
    function Empty: Boolean; override;
    void Clear(FillValue: Integer);
    property ValPtr[X, Y: Integer]: PInteger read GetValPtr;
    property Value[X, Y: Integer]: Integer read GetValue write SetValue; default;
    property Bits: PIntegerArray read GetBits;
  end;

  TFloatMap = class(TCustomMap)
  private
    FBits: TArrayOfFloat;
    function GetValPtr(X, Y: Integer): PFloat; {$IFDEF INLININGSUPPORTED} inline; {$ENDIF}
    function GetValue(X, Y: Integer): TFloat; {$IFDEF INLININGSUPPORTED} inline; {$ENDIF}
    void SetValue(X, Y: Integer; const Value: TFloat); {$IFDEF INLININGSUPPORTED} inline; {$ENDIF}
    function GetBits: PFloatArray;
  protected
    void ChangeSize(var Width, Height: Integer; NewWidth, NewHeight: Integer); override;
  public
    destructor Destroy; override;
    function Empty: Boolean; override;
    void Clear; overload;
    void Clear(FillValue: TFloat); overload;
    property ValPtr[X, Y: Integer]: PFloat read GetValPtr;
    property Value[X, Y: Integer]: TFloat read GetValue write SetValue; default;
    property Bits: PFloatArray read GetBits;
  end;

