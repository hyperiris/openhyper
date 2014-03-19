//unit GR32_Containers;
#pragma once

#include "GR32.h"
#include "GR32_LowLevel.h"

const
  BUCKET_MASK = $FF;               
  BUCKET_COUNT = BUCKET_MASK + 1;  // 256 buckets by default

type
  PPItem = ^PItem;
  PItem = Pointer;

  PPData = ^PData;
  PData = Pointer;

  PPointerBucketItem = ^TPointerBucketItem;
  TPointerBucketItem = record
    Item: PItem;
    Data: PData;
  end;
  TPointerBucketItemArray = array of TPointerBucketItem;

  TPointerBucket = record
    Count: Integer;
    Items: TPointerBucketItemArray;
  end;
  TPointerBucketArray = array[0..BUCKET_MASK] of TPointerBucket;

  { TPointerMap } 
  { Associative pointer map
    Inspired by TBucketList, which is not available on D5/CB5, it is
    reimplemented from scratch, simple, optimized and light-weight.
    Not thread-safe. Does use exceptions only for Data property. }
  TPointerMap = class
  private
    FBuckets: TPointerBucketArray;
    FCount: Integer;
  protected
    function GetData(Item: PItem): PData;
    procedure SetData(Item: PItem; const Data: PData);
    function Exists(Item: Pointer; out BucketIndex, ItemIndex: Integer): Boolean;
    function Delete(BucketIndex, ItemIndex: Integer): PData; virtual;
  public
    destructor Destroy; override;
    function Add(NewItem: PItem): PPData; overload;
    function Add(NewItem: PItem; out IsNew: Boolean): PPData; overload;
    function Add(NewItem: PItem; NewData: PData): PPData; overload;
    function Add(NewItem: PItem; NewData: PData; out IsNew: Boolean): PPData; overload;
    function Remove(Item: PItem): PData;
    procedure Clear;
    function Contains(Item: PItem): Boolean;
    function Find(Item: PItem; out Data: PPData): Boolean;
    property Data[Item: PItem]: PData read GetData write SetData; default;
    property Count: Integer read FCount;
  end;

  { TPointerMapIterator }
  { Iterator object for the associative pointer map
    See below for usage example... }
  TPointerMapIterator = class
  private
    FSrcPointerMap: TPointerMap;
    FItem: PItem;
    FData: PData;
    FCurBucketIndex: Integer;
    FCurItemIndex: Integer;
  public
    constructor Create(SrcPointerMap: TPointerMap);
    function Next: Boolean;
    property Item: PItem read FItem;
    property Data: PData read FData;
  end;
  {
    USAGE EXAMPLE:
    --------------
    with TPointerMapIterator.Create(MyPointerMap) do
    try
      while Next do
      begin
        // do something with Item and Data here...
      end;
    finally
      Free;
    end;
  }

  PPolyRects = ^TPolyRects;
  TPolyRects = Array[0..Maxint div 32 - 1] of TRect;

  { TRectList }
  { List that holds Rects
    Do not reuse TList due to pointer structure.
    A direct structure is more memory efficient.
    stripped version of TList blatantly stolen from Classes.pas }
  TRectList = class
  private
    FList: PPolyRects;
    FCount: Integer;
    FCapacity: Integer;
  protected
    function Get(Index: Integer): PRect;
    procedure Grow; virtual;
    procedure SetCapacity(NewCapacity: Integer);
    procedure SetCount(NewCount: Integer);
  public
    destructor Destroy; override;
    function Add(const Rect: TRect): Integer;
    procedure Clear; virtual;
    procedure Delete(Index: Integer);
    procedure Exchange(Index1, Index2: Integer);
    function IndexOf(const Rect: TRect): Integer;
    procedure Insert(Index: Integer; const Rect: TRect);
    procedure Move(CurIndex, NewIndex: Integer);
    function Remove(const Rect: TRect): Integer;
    procedure Pack;
    property Capacity: Integer read FCapacity write SetCapacity;
    property Count: Integer read FCount write SetCount;
    property Items[Index: Integer]: PRect read Get; default;
    property List: PPolyRects read FList;
  end;

  { TClassList }
  { This is a class that maintains a list of classes. }
  TClassList = class(TList)
  protected
    function GetItems(Index: Integer): TClass;
    procedure SetItems(Index: Integer; AClass: TClass);
  public
    function Add(AClass: TClass): Integer;
    function Extract(Item: TClass): TClass;
    function Remove(AClass: TClass): Integer;
    function IndexOf(AClass: TClass): Integer;
    function First: TClass;
    function Last: TClass;
    function Find(AClassName: string): TClass;
    procedure GetClassNames(Strings: TStrings);
    procedure Insert(Index: Integer; AClass: TClass);
    property Items[Index: Integer]: TClass read GetItems write SetItems; default;
  end;


  PLinkedNode = ^TLinkedNode;
  TLinkedNode = record
    Prev: PLinkedNode;
    Next: PLinkedNode;
    Data: Pointer;
  end;

  TIteratorProc = procedure(Node: PLinkedNode; Index: Integer);

  TFreeDataEvent = procedure(Data: Pointer) of object;

  { TLinkedList }
  { A class for maintaining a linked list }
  TLinkedList = class
   private
     FCount: Integer;
     FHead: PLinkedNode;
     FTail: PLinkedNode;
     FOnFreeData: TFreeDataEvent;
   protected
     procedure DoFreeData(Data: Pointer); virtual;
   public
     destructor Destroy; override;
     function Add: PLinkedNode;
     procedure Remove(Node: PLinkedNode);
     function IndexOf(Node: PLinkedNode): Integer;
     function GetNode(Index: Integer): PLinkedNode;
     procedure Exchange(Node1, Node2: PLinkedNode);
     procedure InsertBefore(Node, NewNode: PLinkedNode);
     procedure InsertAfter(Node, NewNode: PLinkedNode);
     procedure Clear;
     procedure IterateList(CallBack: TIteratorProc);
     property Head: PLinkedNode read FHead write FHead;
     property Tail: PLinkedNode read FTail write FTail;
     property Count: Integer read FCount write FCount;
     property OnFreeData: TFreeDataEvent read FOnFreeData write FOnFreeData;
   end;


procedure SmartAssign(Src, Dst: TPersistent; TypeKinds: TTypeKinds = tkProperties);
procedure Advance(var Node: PLinkedNode; Steps: Integer = 1);

