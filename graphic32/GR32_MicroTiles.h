//unit GR32_MicroTiles;
#pragma once

#include "GR32.h"
#include "GR32_System.h"
#include "GR32_Containers.h"
#include "GR32_Layers.h"
#include "GR32_RepaintOpt.h"
#include "GR32_Bindings.h"

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
  MicroTileUnion: void(var DstTile: TMicroTile; const SrcTile: TMicroTile);

// MicroTiles auxiliary routines
function MakeEmptyMicroTiles: TMicroTiles; {$IFDEF USEINLINING} inline; {$ENDIF}
void MicroTilesCreate(var MicroTiles: TMicroTiles); {$IFDEF USEINLINING} inline; {$ENDIF}
void MicroTilesDestroy(var MicroTiles: TMicroTiles); {$IFDEF USEINLINING} inline; {$ENDIF}
void MicroTilesSetSize(var MicroTiles: TMicroTiles; const DstRect: TRect);
void MicroTilesClear(var MicroTiles: TMicroTiles; const Value: TMicroTile = MICROTILE_EMPTY); {$IFDEF USEINLINING} inline; {$ENDIF}
void MicroTilesClearUsed(var MicroTiles: TMicroTiles; const Value: TMicroTile = MICROTILE_EMPTY);
void MicroTilesCopy(var DstTiles: TMicroTiles; SrcTiles: TMicroTiles);
void MicroTilesAddLine(var MicroTiles: TMicroTiles; X1, Y1, X2, Y2: Integer; LineWidth: Integer; RoundToWholeTiles: Boolean = False);
void MicroTilesAddRect(var MicroTiles: TMicroTiles; Rect: TRect; RoundToWholeTiles: Boolean = False);
void MicroTilesUnion(var DstTiles: TMicroTiles; const SrcTiles: TMicroTiles; RoundToWholeTiles: Boolean = False);
function MicroTilesCalcRects(const MicroTiles: TMicroTiles; DstRects: TRectList; CountOnly: Boolean = False; RoundToWholeTiles: Boolean = False): Integer; overload;
function MicroTilesCalcRects(const MicroTiles: TMicroTiles; DstRects: TRectList; const Clip: TRect; CountOnly: Boolean = False; RoundToWholeTiles: Boolean = False): Integer; overload;
function MicroTilesCountEmptyTiles(const MicroTiles: TMicroTiles): Integer;

type
  { TMicroTilesMap }
  { associative array that is used to map Layers to their MicroTiles }
  TMicroTilesMap = class(TPointerMap)
  private
    function GetData(Item: Pointer): PMicroTiles;
    void SetData(Item: Pointer; const Data: PMicroTiles);
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

    void DrawLayerToMicroTiles(var DstTiles: TMicroTiles; Layer: TCustomLayer);
    void DrawMeasuringHandler(Sender: TObject; const Area: TRect; const Info: Cardinal);

    void ValidateWorkingTiles;
    void UpdateOldInvalidTiles;
    void SetAdaptiveMode(const Value: Boolean);
    void ResetAdaptiveMode;
    void BeginAdaption;
    void EndAdaption;

    void AddArea(var Tiles: TMicroTiles; const Area: TRect; const Info: Cardinal);
  protected
    void SetEnabled(const Value: Boolean); override;

    // LayerCollection handler
    void LayerCollectionNotifyHandler(Sender: TLayerCollection;
      Action: TLayerListNotification; Layer: TCustomLayer; Index: Integer); override;
  public
    constructor Create(Buffer: TBitmap32; InvalidRects: TRectList); override;
    destructor Destroy; override;

    void RegisterLayerCollection(Layers: TLayerCollection); override;
    void UnregisterLayerCollection(Layers: TLayerCollection); override;

    void Reset; override;

    function  UpdatesAvailable: Boolean; override;
    void PerformOptimization; override;

    void BeginPaintBuffer; override;
    void EndPaintBuffer; override;

    // handlers
    void AreaUpdateHandler(Sender: TObject; const Area: TRect; const Info: Cardinal); override;
    void LayerUpdateHandler(Sender: TObject; Layer: TCustomLayer); override;
    void BufferResizedHandler(const NewWidth, NewHeight: Integer); override;

    // custom settings:
    property AdaptiveMode: Boolean read FAdaptiveMode write SetAdaptiveMode;
  end;

{$IFDEF CODESITE}
  TDebugMicroTilesRepaintOptimizer = class(TMicroTilesRepaintOptimizer)
  public
    void Reset; override;
    function  UpdatesAvailable: Boolean; override;
    void PerformOptimization; override;

    void BeginPaintBuffer; override;
    void EndPaintBuffer; override;

    void AreaUpdateHandler(Sender: TObject; const Area: TRect; const Info: Cardinal); override;
    void LayerUpdateHandler(Sender: TObject; Layer: TCustomLayer); override;
    void BufferResizedHandler(const NewWidth, NewHeight: Integer); override;
  end;
{$ENDIF}

implementation
