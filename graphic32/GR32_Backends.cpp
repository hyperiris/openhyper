#include <stdafx.h>

#include "GR32_Backends.h"
//unit GR32_Backends;


uses
  GR32_LowLevel;

procedure RequireBackendSupport(TargetBitmap: TCustomBitmap32;
  RequiredInterfaces: array of TGUID;
  Mode: TRequireOperatorMode; UseOptimizedDestructiveSwitchMethod: Boolean;
  out ReleasedBackend: TCustomBackend);
var
  I: Integer;
  Supported: Boolean;
begin
  Supported := False;
  for I := Low(RequiredInterfaces) to High(RequiredInterfaces) do
  begin
    Supported := Supports(TargetBitmap.Backend, RequiredInterfaces[I]);
    if ((Mode = romAnd) and not Supported) or
      ((Mode = romOr) and Supported) then
      Break;
  end;

  if not Supported then
  begin
    if UseOptimizedDestructiveSwitchMethod then
      TargetBitmap.SetSize(0, 0); // Reset size so we avoid the buffer copy during back-end switch

    ReleasedBackend := TargetBitmap.ReleaseBackend;

    // TODO: Try to find a back-end that supports the required interfaces
    //       instead of resorting to the default platform back-end class...
    TargetBitmap.Backend := GetPlatformBackendClass.Create;
  end
  else
    ReleasedBackend := nil;
end;

procedure RestoreBackend(TargetBitmap: TCustomBitmap32; const SavedBackend: TCustomBackend);
begin
  if Assigned(SavedBackend) then
    TargetBitmap.Backend := SavedBackend;
end;

end.
