//unit GR32_Backends_Generic;
#pragma once

#include "GR32.h"
#include "GR32_Backends.h"

type
  { TMemoryBackend }
  { A backend that keeps the backing buffer entirely in memory.}

  TMemoryBackend = class(TCustomBackend)
  protected
    procedure InitializeSurface(NewWidth, NewHeight: Integer; ClearBuffer: Boolean); override;
    procedure FinalizeSurface; override;
  end;

{$IFDEF Windows}

  { TMMFBackend }
  { A backend that uses memory mapped files or mapped swap space for the
    backing buffer.}

  TMMFBackend = class(TMemoryBackend)
  private
    FMapHandle: THandle;
    FMapIsTemporary: boolean;
    FMapFileHandle: THandle;
    FMapFileName: string;
  protected
    procedure InitializeSurface(NewWidth, NewHeight: Integer; ClearBuffer: Boolean); override;
    procedure FinalizeSurface; override;
  public
    constructor Create(Owner: TCustomBitmap32; IsTemporary: Boolean = True; const MapFileName: string = ''); virtual;
    destructor Destroy; override;

    class procedure InitializeFileMapping(var MapHandle, MapFileHandle: THandle; var MapFileName: string);
    class procedure DeinitializeFileMapping(MapHandle, MapFileHandle: THandle; const MapFileName: string);
    class procedure CreateFileMapping(var MapHandle, MapFileHandle: THandle; var MapFileName: string; IsTemporary: Boolean; NewWidth, NewHeight: Integer);
  end;

{$ENDIF}

implementation
