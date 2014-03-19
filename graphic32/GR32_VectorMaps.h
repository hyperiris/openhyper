//unit GR32_VectorMaps;
#pragma once

#include "GR32.h"

type
  TFixedVector = TFixedPoint;
  PFixedVector = ^TFixedVector;
  TFloatVector = TFloatPoint;
  PFloatVector = ^TFloatVector;
  TArrayOfFixedVector = array of TFixedVector;
  PArrayOfFixedVector = ^TArrayOfFixedVector;
  TArrayOfFloatVector = array of TFloatVector;
  PArrayOfFloatVector = ^TArrayOfFixedVector;

type
  TVectorCombineMode = (vcmAdd, vcmReplace, vcmCustom);
  TVectorCombineEvent= procedure(F, P: TFixedVector; var B: TFixedVector) of object;

  TVectorMap = class(TCustomMap)
  private
    FVectors: TArrayOfFixedVector;
    FOnVectorCombine: TVectorCombineEvent;
    FVectorCombineMode: TVectorCombineMode;
    function GetVectors: PFixedPointArray;
    function GetFixedVector(X,Y: Integer): TFixedVector;
    function GetFixedVectorS(X,Y: Integer): TFixedVector;
    function GetFixedVectorX(X,Y: TFixed): TFixedVector;
    function GetFixedVectorXS(X,Y: TFixed): TFixedVector;
    function GetFloatVector(X,Y: Integer): TFloatVector;
    function GetFloatVectorS(X,Y: Integer): TFloatVector;
    function GetFloatVectorF(X,Y: Single): TFloatVector;
    function GetFloatVectorFS(X,Y: Single): TFloatVector;
    void SetFixedVector(X,Y: Integer; const Point: TFixedVector);
    void SetFixedVectorS(X,Y: Integer; const Point: TFixedVector);
    void SetFixedVectorX(X,Y: TFixed; const Point: TFixedVector);
    void SetFixedVectorXS(X,Y: TFixed; const Point: TFixedVector);
    void SetFloatVector(X,Y: Integer; const Point: TFloatVector);
    void SetFloatVectorS(X,Y: Integer; const Point: TFloatVector);
    void SetFloatVectorF(X,Y: Single; const Point: TFloatVector);
    void SetFloatVectorFS(X,Y: Single; const Point: TFloatVector);
    void SetVectorCombineMode(const Value: TVectorCombineMode);
  protected
    void ChangeSize(var Width, Height: Integer; NewWidth,
      NewHeight: Integer); override;
  public
    destructor Destroy; override;

    void Clear;
    void Merge(DstLeft, DstTop: Integer; Src: TVectorMap; SrcRect: TRect);

    property Vectors: PFixedPointArray read GetVectors;
    function BoundsRect: TRect;
    function GetTrimmedBounds: TRect;
    function Empty: Boolean; override;
    void LoadFromFile(const FileName: string);
    void SaveToFile(const FileName: string);

    property FixedVector[X, Y: Integer]: TFixedVector read GetFixedVector write SetFixedVector; default;
    property FixedVectorS[X, Y: Integer]: TFixedVector read GetFixedVectorS write SetFixedVectorS;
    property FixedVectorX[X, Y: TFixed]: TFixedVector read GetFixedVectorX write SetFixedVectorX;
    property FixedVectorXS[X, Y: TFixed]: TFixedVector read GetFixedVectorXS write SetFixedVectorXS;

    property FloatVector[X, Y: Integer]: TFloatVector read GetFloatVector write SetFloatVector;
    property FloatVectorS[X, Y: Integer]: TFloatVector read GetFloatVectorS write SetFloatVectorS;
    property FloatVectorF[X, Y: Single]: TFloatVector read GetFloatVectorF write SetFloatVectorF;
    property FloatVectorFS[X, Y: Single]: TFloatVector read GetFloatVectorFS write SetFloatVectorFS;
  published
    property VectorCombineMode: TVectorCombineMode read FVectorCombineMode write SetVectorCombineMode;
    property OnVectorCombine: TVectorCombineEvent read FOnVectorCombine write FOnVectorCombine;
  end;

implementation
