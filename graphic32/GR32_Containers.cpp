//unit GR32_Containers;
#include "stdafx.h"

#include "GR32_Containers.h"

void SmartAssign(Src, Dst: TPersistent; TypeKinds: TTypeKinds = tkProperties);
var
  Count, I: Integer;
  Props: PPropList;
  SubSrc, SubDst: TPersistent;
{
  Count := GetTypeData(Src.ClassInfo).PropCount;
  if Count = 0 then Exit;

  GetMem(Props, Count * SizeOf(PPropInfo));
  try
    // Get the property list in an unsorted fashion.
    // This is important so the order in which the properties are defined is obeyed,
    // ie. mimic how the Delphi form loader would set the properties.
    Count := GetPropList(Src.ClassInfo, TypeKinds, Props, False);

    for I := 0 to Count - 1 do
    with Props^[I]^ do
    {
      if PropType^.Kind = tkClass then
      {
        // TODO DVT Added cast to fix ShortString to String warnings. Need to verify is OK
        SubDst := TPersistent(GetObjectProp(Dst, string(Name)));
        if not Assigned(SubDst) then Continue;

        SubSrc := TPersistent(GetObjectProp(Src, string(Name)));
        if Assigned(SubSrc) then SubDst.Assign(SubSrc);
      end
      else
        SetPropValue(Dst, string(Name), GetPropValue(Src, string(Name), True));
    }
  finally
    FreeMem(Props, Count * SizeOf(PPropInfo));
  }
}

void Advance(var Node: PLinkedNode; Steps: Integer);
{
  if Steps > 0 then
  {
    while Assigned(Node) and (Steps > 0) do
    {
      Dec(Steps);
      Node := Node.Next;
    }
  end
  else
  {
    while Assigned(Node) and (Steps < 0) do
    {
      Inc(Steps);
      Node := Node.Prev;
    }
  }
}


{ TPointerMap }

function TPointerMap.Add(NewItem: PItem; NewData: PData): PPData;
var
  Dummy: Boolean;
{
  Result := Add(NewItem, NewData, Dummy);
}

function TPointerMap.Add(NewItem: PItem): PPData;
var
  Dummy: Boolean;
{
  Result := Add(NewItem, nil, Dummy);
}

function TPointerMap.Add(NewItem: PItem; out IsNew: Boolean): PPData;
{
  Result := Add(NewItem, nil, IsNew);
}

function TPointerMap.Add(NewItem: PItem; NewData: PData; out IsNew: Boolean): PPData;
var
  BucketIndex, ItemIndex, Capacity: Integer;
{
  if Exists(NewItem, BucketIndex, ItemIndex) then
  {
    IsNew := False;
    Result := @FBuckets[BucketIndex].Items[ItemIndex].Data
  end
  else
  {
    with FBuckets[BucketIndex] do
    {
      Capacity := Length(Items);

      // enlarge capacity if completely used
      if Count = Capacity then
      {
        if Capacity > 64 then
          Inc(Capacity, Capacity div 4)
        else if Capacity > 8 then
          Inc(Capacity, 16)
        else
          Inc(Capacity, 4);

        SetLength(Items, Capacity);
      }

      with Items[Count] do
      {
        Item := NewItem;
        Data := NewData;
        Result := @Data;
      }

      Inc(Count);
      IsNew := True;
    }
    Inc(FCount);
  }
}

void TPointerMap.Clear;
var
  BucketIndex, ItemIndex: Integer;
{
  FCount := 0;

  for BucketIndex := 0 to BUCKET_MASK do
  with FBuckets[BucketIndex] do
  {
    for ItemIndex := Count - 1 downto 0 do
      Delete(BucketIndex, ItemIndex);

    Count := 0;
    SetLength(Items, 0);
  }
}

destructor TPointerMap.Destroy;
{
  Clear;
  inherited Destroy;
}

function TPointerMap.Delete(BucketIndex, ItemIndex: Integer): PData;
{
  with FBuckets[BucketIndex] do
  {
    Result := Items[ItemIndex].Data;

    if FCount = 0 then Exit;

    Dec(Count);
    if Count = 0 then
      SetLength(Items, 0)
    else
    if (ItemIndex < Count) then
      Move(Items[ItemIndex + 1], Items[ItemIndex], (Count - ItemIndex) * SizeOf(TPointerBucketItem));
  }
  Dec(FCount);
}

function TPointerMap.Remove(Item: PItem): PData;
var
  BucketIndex, ItemIndex: Integer;
{
  if Exists(Item, BucketIndex, ItemIndex) then
    Result := Delete(BucketIndex, ItemIndex)
  else
    Result := nil;
}

function TPointerMap.Contains(Item: PItem): Boolean;
var
  Dummy: Integer;
{
  Result := Exists(Item, Dummy, Dummy);
}

function TPointerMap.Find(Item: PItem; out Data: PPData): Boolean;
var
  BucketIndex, ItemIndex: Integer;
{
  Result := Exists(Item, BucketIndex, ItemIndex);
  if Result then
    Data := @FBuckets[BucketIndex].Items[ItemIndex].Data;
}

function TPointerMap.Exists(Item: PItem; out BucketIndex, ItemIndex: Integer): Boolean;
var
  I: Integer;
{
  BucketIndex := Cardinal(Item) shr 8 and BUCKET_MASK; // KISS pointer hash(TM)
  // due to their randomness, pointers most commonly differ at byte 1, we use
  // this characteristic for our hash and just apply the mask to it.
  // Worst case scenario happens when most changes are at byte 0, which causes
  // one bucket to be saturated whereas the other buckets are almost empty...

  Result := False;
  with FBuckets[BucketIndex] do
  for I := 0 to Count - 1 do
    if Items[I].Item = Item then
    {
      ItemIndex := I;
      Result := True;
      Exit;
    }
}

function TPointerMap.GetData(Item: PItem): PData;
var
  BucketIndex, ItemIndex: Integer;
{
  if not Exists(Item, BucketIndex, ItemIndex) then
    raise EListError.CreateFmt(SItemNotFound, [Integer(Item)])
  else
    Result := FBuckets[BucketIndex].Items[ItemIndex].Data;
}

void TPointerMap.SetData(Item: PItem; const Data: PData);
var
  BucketIndex, ItemIndex: Integer;
{
  if not Exists(Item, BucketIndex, ItemIndex) then
    raise EListError.CreateFmt(SItemNotFound, [Integer(Item)])
  else
    FBuckets[BucketIndex].Items[ItemIndex].Data := Data;
}

{ TPointerMapIterator }

constructor TPointerMapIterator.Create(SrcPointerMap: TPointerMap);
{
  inherited Create;
  FSrcPointerMap := SrcPointerMap;

  FCurBucketIndex := -1;
  FCurItemIndex := -1;
}

function TPointerMapIterator.Next: Boolean;
{
  if FCurItemIndex > 0 then
    Dec(FCurItemIndex)
  else
  {
    FCurItemIndex := -1;
    while (FCurBucketIndex < BUCKET_MASK) and (FCurItemIndex = -1) do
    {
      Inc(FCurBucketIndex);
      FCurItemIndex := FSrcPointerMap.FBuckets[FCurBucketIndex].Count - 1;
    }

    if FCurBucketIndex = BUCKET_MASK then
    {
      Result := False;
      Exit;
    end
  }

  Result := True;
  with FSrcPointerMap.FBuckets[FCurBucketIndex].Items[FCurItemIndex] do
  {
    FItem := Item;
    FData := Data;
  }
}


{ TRectList }

destructor TRectList.Destroy;
{
  SetCount(0);
  SetCapacity(0);
}

function TRectList.Add(const Rect: TRect): Integer;
{
  Result := FCount;
  if Result = FCapacity then
    Grow;
  FList^[Result] := Rect;
  Inc(FCount);
}

void TRectList.Clear;
{
  SetCount(0);
  SetCapacity(10);
}

void TRectList.Delete(Index: Integer);
{
  Dec(FCount);
  if Index < FCount then
    System.Move(FList^[Index + 1], FList^[Index],
      (FCount - Index) * SizeOf(TRect));
}

void TRectList.Exchange(Index1, Index2: Integer);
var
  Item: TRect;
{
  Item := FList^[Index1];
  FList^[Index1] := FList^[Index2];
  FList^[Index2] := Item;
}

function TRectList.Get(Index: Integer): PRect;
{
  if (Index < 0) or (Index >= FCount) then
    Result := nil
  else
    Result := @FList^[Index];
}

void TRectList.Grow;
var
  Delta: Integer;
{
  if FCapacity > 128 then
    Delta := FCapacity div 4
  else
    if FCapacity > 8 then
      Delta := 32
    else
      Delta := 8;
  SetCapacity(FCapacity + Delta);
}

function TRectList.IndexOf(const Rect: TRect): Integer;
{
  Result := 0;
  while (Result < FCount) and not EqualRect(FList^[Result], Rect) do
    Inc(Result);
  if Result = FCount then
    Result := -1;
}

void TRectList.Insert(Index: Integer; const Rect: TRect);
{
  if FCount = FCapacity then
    Grow;
  if Index < FCount then
    System.Move(FList^[Index], FList^[Index + 1],
      (FCount - Index) * SizeOf(TRect));
  FList^[Index] := Rect;
  Inc(FCount);
}

void TRectList.Move(CurIndex, NewIndex: Integer);
var
  Item: TRect;
{
  if CurIndex <> NewIndex then
  {
    Item := Get(CurIndex)^;
    Delete(CurIndex);
    Insert(NewIndex, Item);
  }
}

function TRectList.Remove(const Rect: TRect): Integer;
{
  Result := IndexOf(Rect);
  if Result >= 0 then
    Delete(Result);
}

void TRectList.Pack;
var
  I: Integer;
{
  for I := FCount - 1 downto 0 do
    if Items[I] = nil then
      Delete(I);
}

void TRectList.SetCapacity(NewCapacity: Integer);
{
  if NewCapacity <> FCapacity then
  {
    ReallocMem(FList, NewCapacity * SizeOf(TRect));
    FCapacity := NewCapacity;
  }
}

void TRectList.SetCount(NewCount: Integer);
var
  I: Integer;
{
  if NewCount > FCapacity then
    SetCapacity(NewCount);
  if NewCount > FCount then
    FillChar(FList^[FCount], (NewCount - FCount) * SizeOf(TRect), 0)
  else
    for I := FCount - 1 downto NewCount do
      Delete(I);
  FCount := NewCount;
}

{ TClassList }

function TClassList.Add(AClass: TClass): Integer;
{
  Result := inherited Add(AClass);
}

function TClassList.Extract(Item: TClass): TClass;
{
  Result := TClass(inherited Extract(Item));
}

function TClassList.Find(AClassName: string): TClass;
var
  I: Integer;
{
  Result := nil;
  for I := 0 to Count - 1 do
    if TClass(List[I]).ClassName = AClassName then
    {
      Result := TClass(List[I]);
      Break;
    }
}

function TClassList.First: TClass;
{
  Result := TClass(inherited First);
}

void TClassList.GetClassNames(Strings: TStrings);
var
  I: Integer;
{
  for I := 0 to Count - 1 do
    Strings.Add(TClass(List[I]).ClassName);
}

function TClassList.GetItems(Index: Integer): TClass;
{
  Result := TClass(inherited Items[Index]);
}

function TClassList.IndexOf(AClass: TClass): Integer;
{
  Result := inherited IndexOf(AClass);
}

void TClassList.Insert(Index: Integer; AClass: TClass);
{
  inherited Insert(Index, AClass);
}

function TClassList.Last: TClass;
{
  Result := TClass(inherited Last);
}

function TClassList.Remove(AClass: TClass): Integer;
{
  Result := inherited Remove(AClass);
}

void TClassList.SetItems(Index: Integer; AClass: TClass);
{
  inherited Items[Index] := AClass;
}

{ TLinkedList }

function TLinkedList.Add: PLinkedNode;
{
  New(Result);
  Result.Data := nil;
  Result.Next := nil;
  Result.Prev := nil;
  if Head = nil then
  {
    Head := Result;
    Tail := Result;
  end
  else
    InsertAfter(FTail, Result);
}

void TLinkedList.Clear;
var
  P, NextP: PLinkedNode;
{
  P := Head;
  while Assigned(P) do
  {
    NextP := P.Next;
    DoFreeData(P.Data);
    Dispose(P);
    P := NextP;
  }
  Head := nil;
  Tail := nil;
  Count := 0;
}

destructor TLinkedList.Destroy;
{
  Clear;
}

void TLinkedList.DoFreeData(Data: Pointer);
{
  if Assigned(FOnFreeData) then FOnFreeData(Data);
}

void TLinkedList.Exchange(Node1, Node2: PLinkedNode);
{
  if Assigned(Node1) and Assigned(Node2) and (Node1 <> Node2) then
  {
    if Assigned(Node1.Prev) then Node1.Prev.Next := Node2;
    if Assigned(Node1.Next) then Node1.Next.Prev := Node2;
    if Assigned(Node2.Prev) then Node2.Prev.Next := Node1;
    if Assigned(Node2.Next) then Node2.Next.Prev := Node1;
    if Head = Node1 then Head := Node2 else if Head = Node2 then Head := Node1;
    if Tail = Node1 then Tail := Node2 else if Tail = Node2 then Tail := Node1;
    Swap(Pointer(Node1.Next), Pointer(Node2.Next));
    Swap(Pointer(Node1.Prev), Pointer(Node2.Prev));
  }
}

function TLinkedList.GetNode(Index: Integer): PLinkedNode;
{
  Result := Head;
  Advance(Result, Index);
}

function TLinkedList.IndexOf(Node: PLinkedNode): Integer;
var
  I: Integer;
  P: PLinkedNode;
{
  Result := -1;
  P := Head;
  for I := 0 to Count - 1 do
  {
    if P = Node then
    {
      Result := I;
      Exit;
    }
    P := P.Next;
  }
}

void TLinkedList.InsertAfter(Node, NewNode: PLinkedNode);
{
  if Assigned(Node) and Assigned(NewNode) then
  {
    NewNode.Prev := Node;
    NewNode.Next := Node.Next;
    if Assigned(Node.Next) then Node.Next.Prev := NewNode;
    Node.Next := NewNode;
    if Node = Tail then Tail := NewNode;
    Inc(FCount);
  }
}

void TLinkedList.InsertBefore(Node, NewNode: PLinkedNode);
{
  if Assigned(Node) and Assigned(NewNode) then
  {
    NewNode.Next := Node;
    NewNode.Prev := Node.Prev;
    if Assigned(Node.Prev) then Node.Prev.Next := NewNode;
    Node.Prev := NewNode;
    if Node = Head then Head := NewNode;
    Inc(FCount);
  }
}

void TLinkedList.IterateList(CallBack: TIteratorProc);
var
  I: Integer;
  P: PLinkedNode;
{
  P := Head;
  for I := 0 to Count - 1 do
  {
    CallBack(P, I);
    P := P.Next;
  }
}

void TLinkedList.Remove(Node: PLinkedNode);
{
  if Assigned(Node) then
  {
    DoFreeData(Node.Data);
    if Assigned(Node.Prev) then Node.Prev.Next := Node.Next;
    if Assigned(Node.Next) then Node.Next.Prev := Node.Prev;
    if Node = Head then Head := Node.Next;
    if Node = Tail then Tail := Node.Prev;
    Dispose(Node);
    Dec(FCount);
  }
}

end.
