//unit GR32_Bindings;
#pragma once
#include "GR32_System.h"

type
  TFunctionName = type string;
  TFunctionID = type Integer;

  PFunctionInfo = ^TFunctionInfo;
  TFunctionInfo = record
    FunctionID: Integer;
    Proc: Pointer;
    CPUFeatures: TCPUFeatures;
    Flags: Integer;
  end;

  TFunctionPriority = function (Info: PFunctionInfo): Integer;

  PFunctionBinding = ^TFunctionBinding;
  TFunctionBinding = record
    FunctionID: Integer;
    BindVariable: PPointer;
  end;

  { TFunctionRegistry }
  { This class fascilitates a registry that allows multiple function to be
    registered together with information about their CPU requirements and
    an additional 'flags' parameter. Functions that share the same FunctionID
    can be assigned to a function variable through the rebind methods.
    A priority callback function is used to assess the most optimal function. }
  TFunctionRegistry = class(TPersistent)
  private
    FItems: TList;
    FBindings: TList;
    FName: string;
    procedure SetName(const Value: string);
    function GetItems(Index: Integer): PFunctionInfo;
    procedure SetItems(Index: Integer; const Value: PFunctionInfo);
  public
    constructor Create; virtual;
    destructor Destroy; override;
    procedure Clear;

    procedure Add(FunctionID: Integer; Proc: Pointer; CPUFeatures: TCPUFeatures = []; Flags: Integer = 0);

    // function rebinding support
    procedure RegisterBinding(FunctionID: Integer; BindVariable: PPointer);
    procedure RebindAll(PriorityCallback: TFunctionPriority = nil);
    procedure Rebind(FunctionID: Integer; PriorityCallback: TFunctionPriority = nil);

    function FindFunction(FunctionID: Integer; PriorityCallback: TFunctionPriority = nil): Pointer;
    property Items[Index: Integer]: PFunctionInfo read GetItems write SetItems;
  published
    property Name: string read FName write SetName;
  end;

function NewRegistry(const Name: string = ''): TFunctionRegistry;

function DefaultPriorityProc(Info: PFunctionInfo): Integer;

var
  DefaultPriority: TFunctionPriority = DefaultPriorityProc;

const
  INVALID_PRIORITY: Integer = MaxInt;

implementation
