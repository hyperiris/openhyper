//unit GR32_Backends;
#include <stdafx.h>
#include "GR32_Backends.h"

#include "GR32_LowLevel.h"

void RequireBackendSupport(TargetBitmap: TCustomBitmap32;
  RequiredInterfaces: array of TGUID;
  Mode: TRequireOperatorMode; UseOptimizedDestructiveSwitchMethod: Boolean;
  out ReleasedBackend: TCustomBackend);
{
	int I;
	BOOL Supported = False;

	for I : = Low(RequiredInterfaces) to High(RequiredInterfaces) do
	{
	Supported: = Supports(TargetBitmap.Backend, RequiredInterfaces[I]);
		if ((Mode = romAnd) and not Supported) or
			((Mode = romOr) and Supported) then
			Break;
	}

	if not Supported then
	{
		if UseOptimizedDestructiveSwitchMethod then
		TargetBitmap.SetSize(0, 0); // Reset size so we avoid the buffer copy during back-end switch

	ReleasedBackend: = TargetBitmap.ReleaseBack }

		// TODO: Try to find a back-end that supports the required interfaces
		//       instead of resorting to the default platform back-end class...
		TargetBitmap.Backend : = GetPlatformBackendClass.Create;
		end
	else
	
	ReleasedBackend : = nil;
}

void RestoreBackend(TargetBitmap: TCustomBitmap32; const SavedBackend: TCustomBackend);
{
	if Assigned(SavedBackend)
		TargetBitmap.Backend : = SavedBack
}
