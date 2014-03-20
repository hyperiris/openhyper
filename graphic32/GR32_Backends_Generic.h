//unit GR32_Backends_Generic;
#pragma once

#include "GR32.h"
#include "GR32_Backends.h"

//{ TMemoryBackend }
//{ A backend that keeps the backing buffer entirely in memory.}

class TMemoryBackend : public TCustomBackend
{
protected:
    void InitializeSurface(NewWidth, NewHeight: Integer; ClearBuffer: Boolean); override;
    void FinalizeSurface; override;
};


//{ TMMFBackend }
//{ A backend that uses memory mapped files or mapped swap space for the
//  backing buffer.}

class TMMFBackend : public TMemoryBackend
{
  private
    FMapHandle: THandle;
    FMapIsTemporary: boolean;
    FMapFileHandle: THandle;
    FMapFileName: string;
  protected
    void InitializeSurface(NewWidth, NewHeight: Integer; ClearBuffer: Boolean); override;
    void FinalizeSurface; override;
  public
    constructor Create(Owner: TCustomBitmap32; IsTemporary: Boolean = True; const MapFileName: string = ''); virtual;
    destructor Destroy; override;

    static void InitializeFileMapping(var MapHandle, MapFileHandle: THandle; var MapFileName: string);
	static void DeinitializeFileMapping(MapHandle, MapFileHandle: THandle; const MapFileName : string);
	static void CreateFileMapping(var MapHandle, MapFileHandle: THandle; var MapFileName : string; IsTemporary: Boolean; NewWidth, NewHeight: Integer);
};

