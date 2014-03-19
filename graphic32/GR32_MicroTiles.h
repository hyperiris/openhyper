//unit GR32_MicroTiles;
#pragma once

interface

{$I GR32.inc}
{-$DEFINE CODESITE}
{-$DEFINE CODESITE_HIGH}
{-$DEFINE PROFILINGDRYRUN}
{-$DEFINE MICROTILES_DEBUGDRAW}
  {-$DEFINE MICROTILES_DEBUGDRAW_RANDOM_COLORS}
  {-$DEFINE MICROTILES_DEBUGDRAW_UNOPTIMIZED}
{-$DEFINE MICROTILES_NO_ADAPTION}
  {-$DEFINE MICROTILES_NO_ADAPTION_FORCE_WHOLETILES}

uses
{$IFDEF FPC}
  Types,
  {$IFDEF Windows}
    Windows,
  {$ENDIF}
{$ELSE}
  Windows,
{$ENDIF}
{$IFDEF CODESITE}
  CSIntf, CSAux,
{$ENDIF}
{$IFDEF COMPILER2005_UP}
  Types,
{$ENDIF}
  SysUtils, Classes,
  GR32, GR32_System, GR32_Containers, GR32_Layers, GR32_RepaintOpt, GR32_Bindings;

const
  MICROTILE_SHIFT = 5;
  MICROTILE_SIZE = 1 shl MICROTILE_SHIFT;

  MICROTILE_EMPTY = 0;
  // MICROTILE_EMPTY -> Left: 0, Top: 0, Right:  0, Bottom:  0

  MICROTILE_FULL = MICROTILE_SIZE shl 8 or MICROTILE_SIZE;
  // MICROTILE_FULL -> Left: 0, Top: 0, Right: MICROTILE_SIZE, Bottom: MICROTILE_SIZE

  MicroTileSize = MaxInt div 16;

{$IFDEF MICROTILES_DEBUGDRAW}
  clDebugDrawFill = TColor32($30FF0000);
  clDebugDrawFrame = TColor32($90FF0000);
{$ENDIF}

type
  PMicroTile = ^TMicroTile;
  TMicroTile = type Integer;

  PMicroTileArray = ^TMicroTileArray;
  TMicroTileArray = array[0..MicroTileSize - 1] of TMicroTile;

  PPMicroTiles = ^PMicroTiles;
  PMicroTiles = ^TMicroTiles;
  TMicroTiles = record
    BoundsRect: TRect;
    Columns, Rows: Integer;
    BoundsUsedTiles: TRect;
    Count: Integer;
    Tiles: PMicroTileArray;
  end;

// MicroTile auxiliary routines
function MakeMicroTile(const Left, Top, Right, Bottom: Integer): TMicroTile; {$IFDEF USEINLINING} inline; {$ENDIF}
function MicroTileHeight(const Tile: TMicroTile): Integer; {$IFDEF USEINLINING} inline; {$ENDIF}
function MicroTileWidth(const Tile: TMicroTile): Integer; {$IFDEF USEINLINING} inline; {$ENDIF}

var
  MicroTileUnion: procedure(var DstTile: TMicroTile; const SrcTile: TMicroTile);

// MicroTiles auxiliary routines
function MakeEmptyMicroTiles: TMicroTiles; {$IFDEF USEINLINING} inline; {$ENDIF}
procedure MicroTilesCreate(var MicroTiles: TMicroTiles); {$IFDEF USEINLINING} inline; {$ENDIF}
procedure MicroTilesDestroy(var MicroTiles: TMicroTiles); {$IFDEF USEINLINING} inline; {$ENDIF}
procedure MicroTilesSetSize(var MicroTiles: TMicroTiles; const DstRect: TRect);
procedure MicroTilesClear(var MicroTiles: TMicroTiles; const Value: TMicroTile = MICROTILE_EMPTY); {$IFDEF USEINLINING} inline; {$ENDIF}
procedure MicroTilesClearUsed(var MicroTiles: TMicroTiles; const Value: TMicroTile = MICROTILE_EMPTY);
procedure MicroTilesCopy(var DstTiles: TMicroTiles; SrcTiles: TMicroTiles);
procedure MicroTilesAddLine(var MicroTiles: TMicroTiles; X1, Y1, X2, Y2: Integer; LineWidth: Integer; RoundToWholeTiles: Boolean = False);
procedure MicroTilesAddRect(var MicroTiles: TMicroTiles; Rect: TRect; RoundToWholeTiles: Boolean = False);
procedure MicroTilesUnion(var DstTiles: TMicroTiles; const SrcTiles: TMicroTiles; RoundToWholeTiles: Boolean = False);
function MicroTilesCalcRects(const MicroTiles: TMicroTiles; DstRects: TRectList; CountOnly: Boolean = False; RoundToWholeTiles: Boolean = False): Integer; overload;
function MicroTilesCalcRects(const MicroTiles: TMicroTiles; DstRects: TRectList; const Clip: TRect; CountOnly: Boolean = False; RoundToWholeTiles: Boolean = False): Integer; overload;
function MicroTilesCountEmptyTiles(const MicroTiles: TMicroTiles): Integer;

type
  { TMicroTilesMap }
  { associative array that is used to map Layers to their MicroTiles }
  TMicroTilesMap = class(TPointerMap)
  private
    function GetData(Item: Pointer): PMicroTiles;
    procedure SetData(Item: Pointer; const Data: PMicroTiles);
  protected
    function Delete(BucketIndex: Integer; ItemIndex: Integer): Pointer; override;
  public
    function Add(Item: Pointer): PPMicroTiles;
    property Data[Item: Pointer]: PMicroTiles read GetData write SetData; default;
  end;


type
  { TMicroTilesRepaintOptimizer }
  { Repaint manager that optimizes the repaint process using MicroTiles }
  TMicroTilesRepaintOptimizer = class(TCustomRepaintOptimizer)
  private
    // working tiles
    FBufferBounds: TRect;
    FWorkMicroTiles: PMicroTiles; // used by DrawLayerToMicroTiles
    FTempTiles: TMicroTiles;
    FInvalidTiles: TMicroTiles;
    FForcedInvalidTiles: TMicroTiles;

    // list of invalid layers
    FInvalidLayers: TList;

    // association that maps layers to their old invalid tiles
    FOldInvalidTilesMap: TMicroTilesMap;

    FWorkingTilesValid: Boolean;
    FOldInvalidTilesValid: Boolean;
    FUseInvalidTiles: Boolean;

    // adaptive stuff...
    FAdaptiveMode: Boolean;

    FPerfTimer: TPerfTimer;
    FPerformanceLevel: Integer;
    FElapsedTimeForLastRepaint: Int64;
    FElapsedTimeForFullSceneRepaint: Int64;
    FAdaptionFailed: Boolean;

    // vars for time based approach
    FTimedCheck: Boolean;
    FTimeDelta: Integer;
    FNextCheck: Integer;
    FElapsedTimeOnLastPenalty: Int64;

    // vars for invalid rect difference approach
    FOldInvalidRectsCount: Integer;

{$IFDEF MICROTILES_DEBUGDRAW}
    FDebugWholeTiles: Boolean;
    FDebugMicroTiles: TMicroTiles;
    FDebugInvalidRects: TRectList;
{$ENDIF}

    procedure DrawLayerToMicroTiles(var DstTiles: TMicroTiles; Layer: TCustomLayer);
    procedure DrawMeasuringHandler(Sender: TObject; const Area: TRect; const Info: Cardinal);

    procedure ValidateWorkingTiles;
    procedure UpdateOldInvalidTiles;
    procedure SetAdaptiveMode(const Value: Boolean);
    procedure ResetAdaptiveMode;
    procedure BeginAdaption;
    procedure EndAdaption;

    procedure AddArea(var Tiles: TMicroTiles; const Area: TRect; const Info: Cardinal);
  protected
    procedure SetEnabled(const Value: Boolean); override;

    // LayerCollection handler
    procedure LayerCollectionNotifyHandler(Sender: TLayerCollection;
      Action: TLayerListNotification; Layer: TCustomLayer; Index: Integer); override;
  public
    constructor Create(Buffer: TBitmap32; InvalidRects: TRectList); override;
    destructor Destroy; override;

    procedure RegisterLayerCollection(Layers: TLayerCollection); override;
    procedure UnregisterLayerCollection(Layers: TLayerCollection); override;

    procedure Reset; override;

    function  UpdatesAvailable: Boolean; override;
    procedure PerformOptimization; override;

    procedure BeginPaintBuffer; override;
    procedure EndPaintBuffer; override;

    // handlers
    procedure AreaUpdateHandler(Sender: TObject; const Area: TRect; const Info: Cardinal); override;
    procedure LayerUpdateHandler(Sender: TObject; Layer: TCustomLayer); override;
    procedure BufferResizedHandler(const NewWidth, NewHeight: Integer); override;

    // custom settings:
    property AdaptiveMode: Boolean read FAdaptiveMode write SetAdaptiveMode;
  end;

{$IFDEF CODESITE}
  TDebugMicroTilesRepaintOptimizer = class(TMicroTilesRepaintOptimizer)
  public
    procedure Reset; override;
    function  UpdatesAvailable: Boolean; override;
    procedure PerformOptimization; override;

    procedure BeginPaintBuffer; override;
    procedure EndPaintBuffer; override;

    procedure AreaUpdateHandler(Sender: TObject; const Area: TRect; const Info: Cardinal); override;
    procedure LayerUpdateHandler(Sender: TObject; Layer: TCustomLayer); override;
    procedure BufferResizedHandler(const NewWidth, NewHeight: Integer); override;
  end;
{$ENDIF}

implementation
