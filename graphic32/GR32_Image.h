//unit GR32_Image;
#pragma once
#include "GR32.h"
#include "GR32_Layers.h"
#include "GR32_RangeBars.h"
#include "GR32_LowLevel.h"
#include "GR32_System.h"
#include "GR32_Containers.h"
#include "GR32_RepaintOpt.h"

//  { Paint Stage Constants }
#define PST_CUSTOM            1   // Calls OnPaint with # of current stage in parameter
#define PST_CLEAR_BUFFER      2   // Clears the buffer
#define PST_CLEAR_BACKGND     3   // Clears a visible buffer area
#define PST_DRAW_BITMAP       4   // Draws a bitmap
#define PST_DRAW_LAYERS       5   // Draw layers (Parameter = Layer Mask)
#define PST_CONTROL_FRAME     6   // Draws a dotted frame around the control
#define PST_BITMAP_FRAME      7   // Draws a dotted frame around the scaled bitmap

type
  TPaintStageEvent = procedure(Sender: TObject; Buffer: TBitmap32; StageNum: Cardinal) of object;

  { TPaintStage }
  PPaintStage = ^TPaintStage;
  TPaintStage = record
    DsgnTime: Boolean;
    RunTime: Boolean;
    Stage: Cardinal;             // a PST_* constant
    Parameter: Cardinal;         // an optional parameter
  end;

  { TPaintStages }
  TPaintStages = class
  private
    FItems: array of TPaintStage;
    function GetItem(Index: Integer): PPaintStage;
  public
    destructor Destroy; override;
    function  Add: PPaintStage;
    procedure Clear;
    function  Count: Integer;
    procedure Delete(Index: Integer);
    function  Insert(Index: Integer): PPaintStage;
    property Items[Index: Integer]: PPaintStage read GetItem; default;
  end;

  { Alignment of the bitmap in TCustomImage32 }
  TBitmapAlign = (baTopLeft, baCenter, baTile, baCustom);
  TScaleMode = (smNormal, smStretch, smScale, smResize, smOptimal, smOptimalScaled);
  TPaintBoxOptions = set of (pboWantArrowKeys, pboAutoFocus);

  TRepaintMode = (rmFull, rmDirect, rmOptimizer);

  { TCustomPaintBox32 }
  TCustomPaintBox32 = class(TCustomControl)
  private
    FBuffer: TBitmap32;
    FBufferOversize: Integer;
    FBufferValid: Boolean;
    FRepaintMode: TRepaintMode;
    FInvalidRects: TRectList;
    FForceFullRepaint: Boolean;
    FRepaintOptimizer: TCustomRepaintOptimizer;
    FOptions: TPaintBoxOptions;
    FOnGDIOverlay: TNotifyEvent;
    FMouseInControl: Boolean;
    FOnMouseEnter: TNotifyEvent;
    FOnMouseLeave: TNotifyEvent;
    procedure SetBufferOversize(Value: Integer);
{$IFDEF FPC}
    procedure WMEraseBkgnd(var Message: TLMEraseBkgnd); message LM_ERASEBKGND;
    procedure WMGetDlgCode(var Msg: TLMessage); message LM_GETDLGCODE;
    procedure WMPaint(var Message: TLMPaint); message LM_PAINT;
    procedure CMMouseEnter(var Message: TLMessage); message LM_MOUSEENTER;
    procedure CMMouseLeave(var Message: TLMessage); message LM_MOUSELEAVE;
    procedure CMInvalidate(var Message: TLMessage); message CM_INVALIDATE;
{$ELSE}
    procedure WMEraseBkgnd(var Message: TWmEraseBkgnd); message WM_ERASEBKGND;
    procedure WMGetDlgCode(var Msg: TWmGetDlgCode); message WM_GETDLGCODE;
    procedure WMPaint(var Message: TMessage); message WM_PAINT;
    procedure CMMouseEnter(var Message: TMessage); message CM_MOUSEENTER;
    procedure CMMouseLeave(var Message: TMessage); message CM_MOUSELEAVE;
    procedure CMInvalidate(var Message: TMessage); message CM_INVALIDATE;
{$ENDIF}
    procedure DirectAreaUpdateHandler(Sender: TObject; const Area: TRect; const Info: Cardinal);
  protected
    procedure SetRepaintMode(const Value: TRepaintMode); virtual;
    function  CustomRepaint: Boolean; virtual;
    function  InvalidRectsAvailable: Boolean; virtual;
    procedure DoPrepareInvalidRects; virtual;
    procedure DoPaintBuffer; virtual;
    procedure DoPaintGDIOverlay; virtual;
    procedure DoBufferResized(const OldWidth, OldHeight: Integer); virtual;
    procedure MouseDown(Button: TMouseButton; Shift: TShiftState; X, Y: Integer); override;
    procedure MouseEnter; {$IFDEF FPC} override; {$ELSE} virtual; {$ENDIF}
    procedure MouseLeave; {$IFDEF FPC} override; {$ELSE} virtual; {$ENDIF}
    procedure Paint; override;
    procedure ResetInvalidRects;
    procedure ResizeBuffer;
    property  RepaintOptimizer: TCustomRepaintOptimizer read FRepaintOptimizer;
    property  BufferValid: Boolean read FBufferValid write FBufferValid;
    property  InvalidRects: TRectList read FInvalidRects;
  public
    constructor Create(AOwner: TComponent); override;
    destructor Destroy; override;
    function  GetViewportRect: TRect; virtual;
    procedure Flush; overload;
    procedure Flush(const SrcRect: TRect); overload;
    procedure Invalidate; override;
    procedure ForceFullInvalidate; virtual;
    procedure Loaded; override;
    procedure Resize; override;
    procedure SetBounds(ALeft, ATop, AWidth, AHeight: Integer); override;
    procedure AssignTo(Dest: TPersistent); override;
    property Buffer: TBitmap32 read FBuffer;
    property BufferOversize: Integer read FBufferOversize write SetBufferOversize;
    property Options: TPaintBoxOptions read FOptions write FOptions default [];
    property MouseInControl: Boolean read FMouseInControl;
    property RepaintMode: TRepaintMode read FRepaintMode write SetRepaintMode default rmFull;
    property OnMouseEnter: TNotifyEvent read FOnMouseEnter write FOnMouseEnter;
    property OnMouseLeave: TNotifyEvent read FOnMouseLeave write FOnMouseLeave;
    property OnGDIOverlay: TNotifyEvent read FOnGDIOverlay write FOnGDIOverlay;
  end;

  { TPaintBox32 }
  TPaintBox32 = class(TCustomPaintBox32)
  private
    FOnPaintBuffer: TNotifyEvent;
  protected
    procedure DoPaintBuffer; override;
  public
    property Canvas;
  published
    property Align;
    property Anchors;
    property AutoSize;
    property Constraints;
    property Cursor;
    property DragCursor;
    property DragMode;
    property Options;
    property ParentShowHint;
    property PopupMenu;
    property RepaintMode;
    property ShowHint;
    property TabOrder;
    property TabStop;
    property Visible;
{$IFNDEF PLATFORM_INDEPENDENT}
    property OnCanResize;
{$ENDIF}
    property OnClick;
    property OnDblClick;
    property OnDragDrop;
    property OnDragOver;
    property OnEndDrag;
    property OnGDIOverlay;
    property OnMouseDown;
    property OnMouseMove;
    property OnMouseUp;
    property OnMouseWheel;
    property OnMouseWheelDown;
    property OnMouseWheelUp;
    property OnMouseEnter;
    property OnMouseLeave;
    property OnPaintBuffer: TNotifyEvent read FOnPaintBuffer write FOnPaintBuffer;
    property OnResize;
    property OnStartDrag;
  end;

  { TCustomImage32 }
  TImgMouseEvent = procedure(Sender: TObject; Button: TMouseButton;
    Shift: TShiftState; X, Y: Integer; Layer: TCustomLayer) of object;
  TImgMouseMoveEvent = procedure(Sender: TObject; Shift: TShiftState;
    X, Y: Integer; Layer: TCustomLayer) of object;
  TPaintStageHandler = procedure(Dest: TBitmap32; StageNum: Integer) of object;

  TCustomImage32 = class(TCustomPaintBox32)
  private
    FBitmap: TBitmap32;
    FBitmapAlign: TBitmapAlign;
    FLayers: TLayerCollection;
    FOffsetHorz: TFloat;
    FOffsetVert: TFloat;
    FPaintStages: TPaintStages;
    FPaintStageHandlers: array of TPaintStageHandler;
    FPaintStageNum: array of Integer;
    FScaleX: TFloat;
    FScaleY: TFloat;
    FScaleMode: TScaleMode;
    FUpdateCount: Integer;
    FOnBitmapResize: TNotifyEvent;
    FOnChange: TNotifyEvent;
    FOnInitStages: TNotifyEvent;
    FOnMouseDown: TImgMouseEvent;
    FOnMouseMove: TImgMouseMoveEvent;
    FOnMouseUp: TImgMouseEvent;
    FOnPaintStage: TPaintStageEvent;
    FOnScaleChange: TNotifyEvent;
    procedure BitmapResizeHandler(Sender: TObject);
    procedure BitmapChangeHandler(Sender: TObject);
    procedure BitmapAreaChangeHandler(Sender: TObject; const Area: TRect; const Info: Cardinal);
    procedure BitmapDirectAreaChangeHandler(Sender: TObject; const Area: TRect; const Info: Cardinal);
    procedure LayerCollectionChangeHandler(Sender: TObject);
    procedure LayerCollectionGDIUpdateHandler(Sender: TObject);
    procedure LayerCollectionGetViewportScaleHandler(Sender: TObject; out ScaleX, ScaleY: TFloat);
    procedure LayerCollectionGetViewportShiftHandler(Sender: TObject; out ShiftX, ShiftY: TFloat);
    function  GetOnPixelCombine: TPixelCombineEvent;
    procedure SetBitmap(Value: TBitmap32);
    procedure SetBitmapAlign(Value: TBitmapAlign);
    procedure SetLayers(Value: TLayerCollection);
    procedure SetOffsetHorz(Value: TFloat);
    procedure SetOffsetVert(Value: TFloat);
    procedure SetScale(Value: TFloat);
    procedure SetScaleX(Value: TFloat);
    procedure SetScaleY(Value: TFloat);
    procedure SetOnPixelCombine(Value: TPixelCombineEvent);
  protected
    CachedBitmapRect: TRect;
    CachedShiftX, CachedShiftY,
    CachedScaleX, CachedScaleY,
    CachedRecScaleX, CachedRecScaleY: TFloat;
    CacheValid: Boolean;
    OldSzX, OldSzY: Integer;
    PaintToMode: Boolean;
    procedure BitmapResized; virtual;
    procedure BitmapChanged(const Area: TRect); reintroduce; virtual;
    function  CanAutoSize(var NewWidth, NewHeight: Integer): Boolean; override;
    procedure DoInitStages; virtual;
    procedure DoPaintBuffer; override;
    procedure DoPaintGDIOverlay; override;
    procedure DoScaleChange; virtual;
    procedure InitDefaultStages; virtual;
    procedure InvalidateCache;
    function  InvalidRectsAvailable: Boolean; override;
    procedure DblClick; override;
    procedure MouseDown(Button: TMouseButton; Shift: TShiftState; X, Y: Integer); overload; override;
    procedure MouseMove(Shift: TShiftState; X, Y: Integer); overload; override;
    procedure MouseUp(Button: TMouseButton; Shift: TShiftState; X, Y: Integer); overload; override;
    procedure MouseDown(Button: TMouseButton; Shift: TShiftState; X, Y: Integer; Layer: TCustomLayer); reintroduce; overload; virtual;
    procedure MouseMove(Shift: TShiftState; X, Y: Integer; Layer: TCustomLayer); reintroduce; overload; virtual;
    procedure MouseUp(Button: TMouseButton; Shift: TShiftState; X, Y: Integer; Layer: TCustomLayer); reintroduce; overload; virtual;
    procedure MouseLeave; override;
    procedure SetRepaintMode(const Value: TRepaintMode); override;
    procedure SetScaleMode(Value: TScaleMode); virtual;
    procedure SetXForm(ShiftX, ShiftY, ScaleX, ScaleY: TFloat);
    procedure UpdateCache; virtual;
    property  UpdateCount: Integer read FUpdateCount;
  public
    constructor Create(AOwner: TComponent); override;
    destructor Destroy; override;
    procedure BeginUpdate; virtual;
    function  BitmapToControl(const APoint: TPoint): TPoint; overload;
    function  BitmapToControl(const APoint: TFloatPoint): TFloatPoint; overload;
    procedure Changed; virtual;
    procedure Update(const Rect: TRect); reintroduce; overload; virtual;
    function  ControlToBitmap(const APoint: TPoint): TPoint;  overload;
    function  ControlToBitmap(const APoint: TFloatPoint): TFloatPoint; overload;
    procedure EndUpdate; virtual;
    procedure ExecBitmapFrame(Dest: TBitmap32; StageNum: Integer); virtual;   // PST_BITMAP_FRAME
    procedure ExecClearBuffer(Dest: TBitmap32; StageNum: Integer); virtual;   // PST_CLEAR_BUFFER
    procedure ExecClearBackgnd(Dest: TBitmap32; StageNum: Integer); virtual;  // PST_CLEAR_BACKGND
    procedure ExecControlFrame(Dest: TBitmap32; StageNum: Integer); virtual;  // PST_CONTROL_FRAME
    procedure ExecCustom(Dest: TBitmap32; StageNum: Integer); virtual;        // PST_CUSTOM
    procedure ExecDrawBitmap(Dest: TBitmap32; StageNum: Integer); virtual;    // PST_DRAW_BITMAP
    procedure ExecDrawLayers(Dest: TBitmap32; StageNum: Integer); virtual;    // PST_DRAW_LAYERS
    function  GetBitmapRect: TRect; virtual;
    function  GetBitmapSize: TSize; virtual;
    procedure Invalidate; override;
    procedure Loaded; override;
    procedure PaintTo(Dest: TBitmap32; DestRect: TRect); virtual;
    procedure Resize; override;
    procedure SetupBitmap(DoClear: Boolean = False; ClearColor: TColor32 = $FF000000); virtual;
    property Bitmap: TBitmap32 read FBitmap write SetBitmap;
    property BitmapAlign: TBitmapAlign read FBitmapAlign write SetBitmapAlign;
    property Canvas;
    property Layers: TLayerCollection read FLayers write SetLayers;
    property OffsetHorz: TFloat read FOffsetHorz write SetOffsetHorz;
    property OffsetVert: TFloat read FOffsetVert write SetOffsetVert;
    property PaintStages: TPaintStages read FPaintStages;
    property Scale: TFloat read FScaleX write SetScale;
    property ScaleX: TFloat read FScaleX write SetScaleX;
    property ScaleY: TFloat read FScaleY write SetScaleY;
    property ScaleMode: TScaleMode read FScaleMode write SetScaleMode;
    property OnBitmapResize: TNotifyEvent read FOnBitmapResize write FOnBitmapResize;
    property OnBitmapPixelCombine: TPixelCombineEvent read GetOnPixelCombine write SetOnPixelCombine;
    property OnChange: TNotifyEvent read FOnChange write FOnChange;
    property OnInitStages: TNotifyEvent read FOnInitStages write FOnInitStages;
    property OnMouseDown: TImgMouseEvent read FOnMouseDown write FOnMouseDown;
    property OnMouseMove: TImgMouseMoveEvent read FOnMouseMove write FOnMouseMove;
    property OnMouseUp: TImgMouseEvent read FOnMouseUp write FOnMouseUp;
    property OnPaintStage: TPaintStageEvent read FOnPaintStage write FOnPaintStage;
    property OnScaleChange: TNotifyEvent read FOnScaleChange write FOnScaleChange;
  end;

  TImage32 = class(TCustomImage32)
  published
    property Align;
    property Anchors;
    property AutoSize;
    property Bitmap;
    property BitmapAlign;
    property Color;
    property Constraints;
    property Cursor;
    property DragCursor;
    property DragMode;
    property ParentColor;
    property ParentShowHint;
    property PopupMenu;
    property RepaintMode;
    property Scale;
    property ScaleMode;
    property ShowHint;
    property TabOrder;
    property TabStop;
    property Visible;
    property OnBitmapResize;
{$IFNDEF PLATFORM_INDEPENDENT}
    property OnCanResize;
{$ENDIF}
    property OnClick;
    property OnChange;
    property OnDblClick;
    property OnGDIOverlay;
    property OnDragDrop;
    property OnDragOver;
    property OnEndDrag;
    property OnInitStages;
    property OnKeyDown;
    property OnKeyPress;
    property OnKeyUp;
    property OnMouseDown;
    property OnMouseMove;
    property OnMouseUp;
    property OnMouseWheel;
    property OnMouseWheelDown;
    property OnMouseWheelUp;
    property OnMouseEnter;
    property OnMouseLeave;
    property OnPaintStage;
    property OnResize;
    property OnStartDrag;
  end;

  TCustomImgView32 = class;

  TScrollBarVisibility = (svAlways, svHidden, svAuto);

  { TIVScrollProperties }
  TIVScrollProperties = class(TArrowBarAccess)
  private
    function GetIncrement: Integer;
    function GetSize: Integer;
    function GetVisibility: TScrollbarVisibility;
    procedure SetIncrement(Value: Integer);
    procedure SetSize(Value: Integer);
    procedure SetVisibility(const Value: TScrollbarVisibility);
  protected
    ImgView: TCustomImgView32;
  published
    property Increment: Integer read GetIncrement write SetIncrement default 8;
    property Size: Integer read GetSize write SetSize default 0;
    property Visibility: TScrollBarVisibility read GetVisibility write SetVisibility default svAlways;
  end;

  TSizeGripStyle = (sgAuto, sgNone, sgAlways);

  { TCustomImgView32 }
  TCustomImgView32 = class(TCustomImage32)
  private
    FCentered: Boolean;
    FScrollBarSize: Integer;
    FScrollBarVisibility: TScrollBarVisibility;    
    FScrollBars: TIVScrollProperties;
    FSizeGrip: TSizeGripStyle;
    FOnScroll: TNotifyEvent;
    FOverSize: Integer;
    procedure SetCentered(Value: Boolean);
    procedure SetScrollBars(Value: TIVScrollProperties);
    procedure SetSizeGrip(Value: TSizeGripStyle);
    procedure SetOverSize(const Value: Integer);
  protected
    DisableScrollUpdate: Boolean;
    HScroll: TCustomRangeBar;
    VScroll: TCustomRangeBar;
    procedure AlignAll;
    procedure BitmapResized; override;
    procedure DoDrawSizeGrip(R: TRect);
    procedure DoScaleChange; override;
    procedure DoScroll; virtual;
    function  GetScrollBarsVisible: Boolean;
    function  GetScrollBarSize: Integer;
    function  GetSizeGripRect: TRect;
    function  IsSizeGripVisible: Boolean;
    procedure MouseDown(Button: TMouseButton; Shift: TShiftState; X, Y: Integer); override;
    procedure MouseMove(Shift: TShiftState; X, Y: Integer); override;
    procedure Paint; override;
    procedure Recenter;
    procedure SetScaleMode(Value: TScaleMode); override;
    procedure ScrollHandler(Sender: TObject); virtual;
    procedure UpdateImage; virtual;
    procedure UpdateScrollBars; virtual;
  public
    constructor Create(AOwner: TComponent); override;
    destructor Destroy; override;
    function  GetViewportRect: TRect; override;
    procedure Loaded; override;
    procedure Resize; override;
    procedure ScrollToCenter(X, Y: Integer);
    procedure Scroll(Dx, Dy: Integer);
    property Centered: Boolean read FCentered write SetCentered default True;
    property ScrollBars: TIVScrollProperties read FScrollBars write SetScrollBars;
    property SizeGrip: TSizeGripStyle read FSizeGrip write SetSizeGrip default sgAuto;
    property OverSize: Integer read FOverSize write SetOverSize;    
    property OnScroll: TNotifyEvent read FOnScroll write FOnScroll;
  end;

  TImgView32 = class(TCustomImgView32)
    property Align;
    property Anchors;
    property AutoSize;
    property Bitmap;
    property BitmapAlign;    
    property Centered;
    property Color;
    property Constraints;
    property Cursor;
    property DragCursor;
    property DragMode;
    property ParentColor;
    property ParentShowHint;
    property PopupMenu;
    property RepaintMode;
    property Scale;
    property ScaleMode;
    property ScrollBars;
    property ShowHint;
    property SizeGrip;
    property OverSize;
    property TabOrder;
    property TabStop;
    property Visible;
    property OnBitmapResize;
{$IFNDEF PLATFORM_INDEPENDENT}
    property OnCanResize;
{$ENDIF}
    property OnClick;
    property OnChange;
    property OnDblClick;
    property OnDragDrop;
    property OnDragOver;
    property OnEndDrag;
    property OnGDIOverlay;
    property OnInitStages; 
    property OnKeyDown;
    property OnKeyPress;
    property OnKeyUp;
    property OnMouseDown;
    property OnMouseEnter;
    property OnMouseLeave;
    property OnMouseMove;
    property OnMouseUp;
    property OnMouseWheel;
    property OnMouseWheelDown;
    property OnMouseWheelUp;
    property OnPaintStage;
    property OnResize;
    property OnScroll;
    property OnStartDrag;
  end;

  { TBitmap32Item }
  { A bitmap container designed to be inserted into TBitmap32Collection }
  TBitmap32Item = class(TCollectionItem)
  private
    FBitmap: TBitmap32;
    procedure SetBitmap(ABitmap: TBitmap32);
  protected
    procedure AssignTo(Dest: TPersistent); override;
  public
    constructor Create(Collection: TCollection); override;
    destructor Destroy; override;
  published
    property Bitmap: TBitmap32 read FBitmap write SetBitmap;
  end;

  TBitmap32ItemClass = class of TBitmap32Item;

  { TBitmap32Collection }
  { A collection of TBitmap32Item objects }
  TBitmap32Collection = class(TCollection)
  private
    FOwner: TPersistent;
    function  GetItem(Index: Integer): TBitmap32Item;
    procedure SetItem(Index: Integer; Value: TBitmap32Item);
  protected
    function  GetOwner: TPersistent; override;
  public
    constructor Create(AOwner: TPersistent; ItemClass: TBitmap32ItemClass);
    function Add: TBitmap32Item;
    property Items[Index: Integer]: TBitmap32Item read GetItem write SetItem; default;
  end;

  { TBitmap32List }
  { A component that stores TBitmap32Collection }
  TBitmap32List = class(TComponent)
  private
    FBitmap32Collection: TBitmap32Collection;
    procedure SetBitmap(Index: Integer; Value: TBitmap32);
    function GetBitmap(Index: Integer): TBitmap32;
    procedure SetBitmap32Collection(Value: TBitmap32Collection);
  public
    constructor Create(AOwner: TComponent); override;
    destructor Destroy; override;
    property Bitmap[Index: Integer]: TBitmap32 read GetBitmap write SetBitmap; default;
  published
    property Bitmaps: TBitmap32Collection read FBitmap32Collection write SetBitmap32Collection;
  end;

implementation
