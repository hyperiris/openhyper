//unit GR32_Layers;
#pragma once

#include "GR32.h"

const
  { Layer Options Bits }
  LOB_VISIBLE           = $80000000; // 31-st bit
  LOB_GDI_OVERLAY       = $40000000; // 30-th bit
  LOB_MOUSE_EVENTS      = $20000000; // 29-th bit
  LOB_NO_UPDATE         = $10000000; // 28-th bit
  LOB_NO_CAPTURE        = $08000000; // 27-th bit
  LOB_INVALID           = $04000000; // 26-th bit
  LOB_FORCE_UPDATE      = $02000000; // 25-th bit
  LOB_RESERVED_24       = $01000000; // 24-th bit
  LOB_RESERVED_MASK     = $FF000000;

type
  TCustomLayer = class;
  TPositionedLayer = class;
  TLayerClass = class of TCustomLayer;

  TLayerCollection = class;

  TLayerUpdateEvent = procedure(Sender: TObject; Layer: TCustomLayer) of object;
  TAreaUpdateEvent = TAreaChangedEvent;
  TLayerListNotification = (lnLayerAdded, lnLayerInserted, lnLayerDeleted, lnCleared);
  TLayerListNotifyEvent = procedure(Sender: TLayerCollection; Action: TLayerListNotification;
    Layer: TCustomLayer; Index: Integer) of object;
  TGetScaleEvent = procedure(Sender: TObject; out ScaleX, ScaleY: TFloat) of object;
  TGetShiftEvent = procedure(Sender: TObject; out ShiftX, ShiftY: TFloat) of object;

  TLayerCollection = class(TPersistent)
  private
    FItems: TList;
    FMouseEvents: Boolean;
    FMouseListener: TCustomLayer;
    FUpdateCount: Integer;
    FOwner: TPersistent;
    FOnChanging: TNotifyEvent;
    FOnChange: TNotifyEvent;
    FOnGDIUpdate: TNotifyEvent;
    FOnListNotify: TLayerListNotifyEvent;
    FOnLayerUpdated: TLayerUpdateEvent;
    FOnAreaUpdated: TAreaUpdateEvent;
    FOnGetViewportScale: TGetScaleEvent;
    FOnGetViewportShift: TGetShiftEvent;
    function GetCount: Integer;
    procedure InsertItem(Item: TCustomLayer);
    procedure RemoveItem(Item: TCustomLayer);
    procedure SetMouseEvents(Value: Boolean);
    procedure SetMouseListener(Value: TCustomLayer);
  protected
    procedure BeginUpdate;
    procedure Changed;
    procedure Changing;
    procedure EndUpdate;
    function  FindLayerAtPos(X, Y: Integer; OptionsMask: Cardinal): TCustomLayer;
    function  GetItem(Index: Integer): TCustomLayer;
    function  GetOwner: TPersistent; override;
    procedure GDIUpdate;
    procedure DoUpdateLayer(Layer: TCustomLayer);
    procedure DoUpdateArea(const Rect: TRect);
    procedure Notify(Action: TLayerListNotification; Layer: TCustomLayer; Index: Integer);
    procedure SetItem(Index: Integer; Value: TCustomLayer);
    function MouseDown(Button: TMouseButton; Shift: TShiftState; X, Y: Integer): TCustomLayer;
    function MouseMove(Shift: TShiftState; X, Y: Integer): TCustomLayer;
    function MouseUp(Button: TMouseButton; Shift: TShiftState; X, Y: Integer): TCustomLayer;
    property OnChanging: TNotifyEvent read FOnChanging write FOnChanging;
    property OnChange: TNotifyEvent read FOnChange write FOnChange;
    property OnListNotify: TLayerListNotifyEvent read FOnListNotify write FOnListNotify;
    property OnGDIUpdate: TNotifyEvent read FOnGDIUpdate write FOnGDIUpdate;
    property OnLayerUpdated: TLayerUpdateEvent read FOnLayerUpdated write FOnLayerUpdated;
    property OnAreaUpdated: TAreaUpdateEvent read FOnAreaUpdated write FOnAreaUpdated;
    property OnGetViewportScale: TGetScaleEvent read FOnGetViewportScale write FOnGetViewportScale;
    property OnGetViewportShift: TGetShiftEvent read FOnGetViewportShift write FOnGetViewportShift;
  public
    constructor Create(AOwner: TPersistent);
    destructor Destroy; override;
    function  Add(ItemClass: TLayerClass): TCustomLayer;
    procedure Assign(Source: TPersistent); override;
    procedure Clear;
    procedure Delete(Index: Integer);
    function  Insert(Index: Integer; ItemClass: TLayerClass): TCustomLayer;
    function  LocalToViewport(const APoint: TFloatPoint; AScaled: Boolean): TFloatPoint;
    function  ViewportToLocal(const APoint: TFloatPoint; AScaled: Boolean): TFloatPoint;
    procedure GetViewportScale(out ScaleX, ScaleY: TFloat); virtual;
    procedure GetViewportShift(out ShiftX, ShiftY: TFloat); virtual;
    property Count: Integer read GetCount;
    property Owner: TPersistent read FOwner;
    property Items[Index: Integer]: TCustomLayer read GetItem write SetItem; default;
    property MouseListener: TCustomLayer read FMouseListener write SetMouseListener;
    property MouseEvents: Boolean read FMouseEvents write SetMouseEvents;
  end;

  TLayerState = (lsMouseLeft, lsMouseRight, lsMouseMiddle);
  TLayerStates = set of TLayerState;

  TPaintLayerEvent = procedure(Sender: TObject; Buffer: TBitmap32) of object;
  THitTestEvent = procedure(Sender: TObject; X, Y: Integer; var Passed: Boolean) of object;

  TCustomLayer = class(TNotifiablePersistent)
  private
    FCursor: TCursor;
    FFreeNotifies: TList;
    FLayerCollection: TLayerCollection;
    FLayerStates: TLayerStates;
    FLayerOptions: Cardinal;
    FOnHitTest: THitTestEvent;
    FOnMouseDown: TMouseEvent;
    FOnMouseMove: TMouseMoveEvent;
    FOnMouseUp: TMouseEvent;
    FOnPaint: TPaintLayerEvent;
    FTag: Integer;
    FOnDestroy: TNotifyEvent;
    function  GetIndex: Integer;
    function  GetMouseEvents: Boolean;
    function  GetVisible: Boolean;
    procedure SetMouseEvents(Value: Boolean);
    procedure SetVisible(Value: Boolean);
    function GetInvalid: Boolean;
    procedure SetInvalid(Value: Boolean);
    function GetForceUpdate: Boolean;
    procedure SetForceUpdate(Value: Boolean);
  protected
    procedure AddNotification(ALayer: TCustomLayer);
    procedure Changing;
    function  DoHitTest(X, Y: Integer): Boolean; virtual;
    procedure DoPaint(Buffer: TBitmap32);
    function  GetOwner: TPersistent; override;
    procedure MouseDown(Button: TMouseButton; Shift: TShiftState; X, Y: Integer); virtual;
    procedure MouseMove(Shift: TShiftState; X, Y: Integer); virtual;
    procedure MouseUp(Button: TMouseButton; Shift: TShiftState; X, Y: Integer); virtual;
    procedure Notification(ALayer: TCustomLayer); virtual;
    procedure Paint(Buffer: TBitmap32); virtual;
    procedure PaintGDI(Canvas: TCanvas); virtual;
    procedure RemoveNotification(ALayer: TCustomLayer);
    procedure SetIndex(Value: Integer); virtual;
    procedure SetCursor(Value: TCursor); virtual;
    procedure SetLayerCollection(Value: TLayerCollection); virtual;
    procedure SetLayerOptions(Value: Cardinal); virtual;
    property Invalid: Boolean read GetInvalid write SetInvalid;
    property ForceUpdate: Boolean read GetForceUpdate write SetForceUpdate;
  public
    constructor Create(ALayerCollection: TLayerCollection); virtual;
    destructor Destroy; override;
    procedure BeforeDestruction; override;
    procedure BringToFront;
    procedure Changed; overload; override;
    procedure Changed(const Rect: TRect); reintroduce; overload;
    procedure Update; overload;
    procedure Update(const Rect: TRect); overload;
    function  HitTest(X, Y: Integer): Boolean;
    procedure SendToBack;
    procedure SetAsMouseListener;
    property Cursor: TCursor read FCursor write SetCursor;
    property Index: Integer read GetIndex write SetIndex;
    property LayerCollection: TLayerCollection read FLayerCollection write SetLayerCollection;
    property LayerOptions: Cardinal read FLayerOptions write SetLayerOptions;
    property LayerStates: TLayerStates read FLayerStates;
    property MouseEvents: Boolean read GetMouseEvents write SetMouseEvents;
    property Tag: Integer read FTag write FTag;
    property Visible: Boolean read GetVisible write SetVisible;
    property OnDestroy: TNotifyEvent read FOnDestroy write FOnDestroy;
    property OnHitTest: THitTestEvent read FOnHitTest write FOnHitTest;
    property OnPaint: TPaintLayerEvent read FOnPaint write FOnPaint;
    property OnMouseDown: TMouseEvent read FOnMouseDown write FOnMouseDown;
    property OnMouseMove: TMouseMoveEvent read FOnMouseMove write FOnMouseMove;
    property OnMouseUp: TMouseEvent read FOnMouseUp write FOnMouseUp;
  end;

  TPositionedLayer = class(TCustomLayer)
  private
    FLocation: TFloatRect;
    FScaled: Boolean;
    procedure SetLocation(const Value: TFloatRect);
    procedure SetScaled(Value: Boolean);
  protected
    function DoHitTest(X, Y: Integer): Boolean; override;
    procedure DoSetLocation(const NewLocation: TFloatRect); virtual;
  public
    constructor Create(ALayerCollection: TLayerCollection); override;
    function GetAdjustedRect(const R: TFloatRect): TFloatRect; virtual;
    function GetAdjustedLocation: TFloatRect;
    property Location: TFloatRect read FLocation write SetLocation;
    property Scaled: Boolean read FScaled write SetScaled;
  end;

  TBitmapLayer = class(TPositionedLayer)
  private
    FBitmap: TBitmap32;
    FAlphaHit: Boolean;
    FCropped: Boolean;
    procedure BitmapAreaChanged(Sender: TObject; const Area: TRect; const Info: Cardinal);
    procedure SetBitmap(Value: TBitmap32);
    procedure SetCropped(Value: Boolean);
  protected
    function DoHitTest(X, Y: Integer): Boolean; override;
    procedure Paint(Buffer: TBitmap32); override;
  public
    constructor Create(ALayerCollection: TLayerCollection); override;
    destructor Destroy; override;
    property AlphaHit: Boolean read FAlphaHit write FAlphaHit;
    property Bitmap: TBitmap32 read FBitmap write SetBitmap;
    property Cropped: Boolean read FCropped write SetCropped;
  end;

  TDragState = (dsNone, dsMove, dsSizeL, dsSizeT, dsSizeR, dsSizeB,
    dsSizeTL, dsSizeTR, dsSizeBL, dsSizeBR);
  TRBHandles = set of (rhCenter, rhSides, rhCorners, rhFrame,
    rhNotLeftSide, rhNotRightSide, rhNotTopSide, rhNotBottomSide,
    rhNotTLCorner, rhNotTRCorner, rhNotBLCorner, rhNotBRCorner);
  TRBOptions = set of (roProportional, roConstrained);
  TRBResizingEvent = procedure(
    Sender: TObject;
    const OldLocation: TFloatRect;
    var NewLocation: TFloatRect;
    DragState: TDragState;
    Shift: TShiftState) of object;
  TRBConstrainEvent = TRBResizingEvent;

  TRubberbandLayer = class(TPositionedLayer)
  private
    FChildLayer: TPositionedLayer;
    FFrameStipplePattern: TArrayOfColor32;
    FFrameStippleStep: TFloat;
    FFrameStippleCounter: TFloat;
    FHandleFrame: TColor32;
    FHandleFill: TColor32;
    FHandles: TRBHandles;
    FHandleSize: Integer;
    FMinWidth: TFloat;
    FMaxHeight: TFloat;
    FMinHeight: TFloat;
    FMaxWidth: TFloat;
    FOnUserChange: TNotifyEvent;
    FOnResizing: TRBResizingEvent;
    FOnConstrain: TRBConstrainEvent;
    FOptions: TRBOptions;
    procedure SetFrameStippleStep(const Value: TFloat);
    procedure SetFrameStippleCounter(const Value: TFloat);
    procedure SetChildLayer(Value: TPositionedLayer);
    procedure SetHandleFill(Value: TColor32);
    procedure SetHandleFrame(Value: TColor32);
    procedure SetHandles(Value: TRBHandles);
    procedure SetHandleSize(Value: Integer);
    procedure SetOptions(const Value: TRBOptions);
  protected
    IsDragging: Boolean;
    DragState: TDragState;
    OldLocation: TFloatRect;
    MouseShift: TFloatPoint;
    function  DoHitTest(X, Y: Integer): Boolean; override;
    procedure DoResizing(var OldLocation, NewLocation: TFloatRect; DragState: TDragState; Shift: TShiftState); virtual;
    procedure DoConstrain(var OldLocation, NewLocation: TFloatRect; DragState: TDragState; Shift: TShiftState); virtual;
    procedure DoSetLocation(const NewLocation: TFloatRect); override;
    function  GetDragState(X, Y: Integer): TDragState; virtual;
    procedure MouseDown(Button: TMouseButton; Shift: TShiftState; X, Y: Integer); override;
    procedure MouseMove(Shift: TShiftState; X, Y: Integer); override;
    procedure MouseUp(Button: TMouseButton; Shift: TShiftState; X, Y: Integer); override;
    procedure Notification(ALayer: TCustomLayer); override;
    procedure Paint(Buffer: TBitmap32); override;
    procedure SetLayerOptions(Value: Cardinal); override;
    procedure UpdateChildLayer;
  public
    constructor Create(ALayerCollection: TLayerCollection); override;
    procedure SetFrameStipple(const Value: Array of TColor32);
    property ChildLayer: TPositionedLayer read FChildLayer write SetChildLayer;
    property Options: TRBOptions read FOptions write SetOptions;
    property Handles: TRBHandles read FHandles write SetHandles;
    property HandleSize: Integer read FHandleSize write SetHandleSize;
    property HandleFill: TColor32 read FHandleFill write SetHandleFill;
    property HandleFrame: TColor32 read FHandleFrame write SetHandleFrame;
    property FrameStippleStep: TFloat read FFrameStippleStep write SetFrameStippleStep;
    property FrameStippleCounter: TFloat read FFrameStippleCounter write SetFrameStippleCounter;
    property MaxHeight: TFloat read FMaxHeight write FMaxHeight;
    property MaxWidth: TFloat read FMaxWidth write FMaxWidth;
    property MinHeight: TFloat read FMinHeight write FMinHeight;
    property MinWidth: TFloat read FMinWidth write FMinWidth;
    property OnUserChange: TNotifyEvent read FOnUserChange write FOnUserChange;
    property OnConstrain: TRBConstrainEvent read FOnConstrain write FOnConstrain;
    property OnResizing: TRBResizingEvent read FOnResizing write FOnResizing;
  end;

implementation
