//unit GR32_Backends_Generic;
#include "stdafx.h"
#include "GR32_Backends_Generic.h"

#include "GR32_LowLevel.h"

var
  TempPath: TFileName;

resourcestring
  RCStrFailedToMapFile = 'Failed to map file';
  RCStrFailedToCreateMapFile = 'Failed to create map file (%s)';
  RCStrFailedToMapViewOfFile = 'Failed to map view of file.';

function GetTempPath: TFileName;
var
  PC: PChar;
{
  PC := StrAlloc(MAX_PATH + 1);
  try
    Windows.GetTempPath(MAX_PATH, PC);
    Result := TFileName(PC);
  finally
    StrDispose(PC);
  }
}

//{ TMemoryBackend }

void TMemoryBackend::InitializeSurface(NewWidth, NewHeight: Integer; ClearBuffer: Boolean);
{
  GetMem(FBits, NewWidth * NewHeight * 4);
  if ClearBuffer then
    FillLongword(FBits[0], NewWidth * NewHeight, clBlack32);
}

void TMemoryBackend::FinalizeSurface;
{
  if Assigned(FBits) then
  {
    FreeMem(FBits);
    FBits := nil;
  }
}

//{ TMMFBackend }

constructor TMMFBackend.Create(Owner: TCustomBitmap32; IsTemporary: Boolean = True; const MapFileName: string = '');
{
  FMapFileName := MapFileName;
  FMapIsTemporary := IsTemporary;
  InitializeFileMapping(FMapHandle, FMapFileHandle, FMapFileName);
  inherited Create(Owner);
}

destructor TMMFBackend.Destroy;
{
  DeinitializeFileMapping(FMapHandle, FMapFileHandle, FMapFileName);
  inherited;
}

procedure TMMFBackend.FinalizeSurface;
{
  if Assigned(FBits) then
  {
    UnmapViewOfFile(FBits);
    FBits := nil;
  }
}

procedure TMMFBackend.InitializeSurface(NewWidth, NewHeight: Integer; ClearBuffer: Boolean);
{
  CreateFileMapping(FMapHandle, FMapFileHandle, FMapFileName, FMapIsTemporary, NewWidth, NewHeight);
  FBits := MapViewOfFile(FMapHandle, FILE_MAP_ALL_ACCESS, 0, 0, 0);

  if not Assigned(FBits) then
    raise Exception.Create(RCStrFailedToMapViewOfFile);

  if ClearBuffer then
    FillLongword(FBits[0], NewWidth * NewHeight, clBlack32);
}


void TMMFBackend::InitializeFileMapping(var MapHandle, MapFileHandle: THandle; var MapFileName: string);
{
  MapHandle := INVALID_HANDLE_VALUE;
  MapFileHandle := INVALID_HANDLE_VALUE;
  if MapFileName <> '' then
    ForceDirectories(IncludeTrailingPathDelimiter(ExtractFilePath(MapFileName)));
}

void TMMFBackend::DeinitializeFileMapping(MapHandle, MapFileHandle: THandle; const MapFileName: string);
{
  if MapFileName <> '' then
  {
    CloseHandle(MapHandle);
    CloseHandle(MapFileHandle);
    if FileExists(MapFileName) then
      DeleteFile(MapFileName);
  }
}

void TMMFBackend::CreateFileMapping(var MapHandle, MapFileHandle: THandle;
  var MapFileName: string; IsTemporary: Boolean; NewWidth, NewHeight: Integer);
var
  Flags: Cardinal;


{$IFDEF USE_GUIDS_IN_MMF}

  function GetTempFileName(const Prefix: string): string;
  var
    GUID: TGUID;
  {
    repeat
      CoCreateGuid(GUID);
      Result := TempPath + Prefix + GUIDToString(GUID);
    until not FileExists(Result);
  }

{$ELSE}

  function GetTempFileName(const Prefix: string): string;
  var
    PC: PChar;
  {
    PC := StrAlloc(MAX_PATH + 1);
    Windows.GetTempFileName(PChar(GetTempPath), PChar(Prefix), 0, PC);
    Result := string(PC);
    StrDispose(PC);
  }

{$ENDIF}

{
  // close previous handles
  if MapHandle <> INVALID_HANDLE_VALUE then
  {
    CloseHandle(MapHandle);
    MapHandle := INVALID_HANDLE_VALUE;
  }

  if MapFileHandle <> INVALID_HANDLE_VALUE then
  {
    CloseHandle(MapFileHandle);
    MapHandle := INVALID_HANDLE_VALUE;
  }

  // Do we want to use an external map file?
  if (MapFileName <> '') or IsTemporary then
  {
    if MapFileName = '' then
    {$IFDEF HAS_NATIVEINT}
      MapFileName := GetTempFileName(IntToStr(NativeUInt(Self)));
    {$ELSE}
      MapFileName := GetTempFileName(IntToStr(Cardinal(Self)));
    {$ENDIF}

    // delete file if exists
    if FileExists(MapFileName) then
      DeleteFile(MapFileName);

    // open file
    if IsTemporary then
      Flags := FILE_ATTRIBUTE_TEMPORARY OR FILE_FLAG_DELETE_ON_CLOSE
    else
      Flags := FILE_ATTRIBUTE_NORMAL;

    MapFileHandle := CreateFile(PChar(MapFileName), GENERIC_READ or GENERIC_WRITE,
      0, nil, CREATE_ALWAYS, Flags, 0);

    if MapFileHandle = INVALID_HANDLE_VALUE then
    {
      if not IsTemporary then
        raise Exception.CreateFmt(RCStrFailedToCreateMapFile, [MapFileName])
      else
      {
        // Reset and fall back to allocating in the system's paging file...

        // delete file if exists
        if FileExists(MapFileName) then
          DeleteFile(MapFileName);
          
        MapFileName := '';
      }
    }
  end
  else // use the system's paging file
    MapFileHandle := INVALID_HANDLE_VALUE;

  // create map
  MapHandle := Windows.CreateFileMapping(MapFileHandle, nil, PAGE_READWRITE, 0, NewWidth * NewHeight * 4, nil);

  if MapHandle = 0 then
    raise Exception.Create(RCStrFailedToMapFile);
}

initialization
  TempPath := IncludeTrailingPathDelimiter(GetTempPath);

finalization
  TempPath := '';
