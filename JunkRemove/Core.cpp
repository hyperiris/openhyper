#include "stdafx.h"

BOOL CheckBreak();
LPCTSTR GetDisasmText(ea_t ea);
void SafeJumpTo(segment_t* pSegment, ea_t ea);

// Initialize
void CORE_Init()
{        
	// nothing
}

// Un-initialize
void CORE_Exit()
{
    set_user_defined_prefix(0, NULL);
}

// Plug-in process
void CORE_Process(int iArg)
{   
	// IDA must be IDLE 
	if(autoIsOk())
	{
		msg("\nJunkRemove: %s - %s, By HyperIris\n", JM_VERSION, __DATE__); 
		size_t iStartFuncCount = get_func_qty();
		if(iStartFuncCount > 0)
		{
			// Save start position
			ea_t uStartAddress = get_screen_ea();


			int iSegCount = get_segm_qty();              
			for(int iIndex = 0; iIndex < iSegCount; iIndex++)
			{      
				segment_t* pThisSeg = getnseg(iIndex);
				if(pThisSeg)
				{
					char szClass[16] = {0};
					get_segm_class(pThisSeg, szClass, (sizeof(szClass) - 1));
					if(strcmp(szClass, "CODE") == 0)
					{
						char szName[128] = {0};
						get_segm_name(pThisSeg, szName, (sizeof(szName) - 1));
						msg("JunkRemove: Processing segment: \"%s\", \"%s\".\n", szName, szClass);        

						// OK, now process CODE segment
						ea_t uCurrentAddress = pThisSeg->startEA;

						//while(uCurrentAddress < pThisSeg->endEA)
						while (true)
						{
							ea_t uAddress = find_text(uCurrentAddress, 0,0, " short near ptr loc_", (SEARCH_DOWN | SEARCH_CASE));
							if(uAddress < pThisSeg->endEA)
							{
								uCurrentAddress = uAddress;                        

								LPCTSTR pszLine = GetDisasmText(uCurrentAddress);
								int iLineLen = strlen(pszLine);

								// if it is we need
								if ((pszLine[0] == 'j') && (pszLine[iLineLen - 2] == '+'))
								{
									// calculate address
									char szAddress[128] = {0};
									const char* pszAddres = strstr(pszLine, "_");
									if (pszAddres)
									{
										strcpy(szAddress, pszAddres + 1);
										int iAddressLen = strlen(szAddress);
										int extraOffset = szAddress[iAddressLen-1] - 0x30;
										//szAddress[iAddressLen-1] = 0;
										szAddress[iAddressLen-2] = 0;

										ea_t oldAddress = 0;
										sscanf(szAddress, "%x", &oldAddress);
										ea_t newAddress = oldAddress + extraOffset;

										SafeJumpTo(pThisSeg, oldAddress);
										ea_t uEnd = next_head(oldAddress, pThisSeg->endEA);

										autoWait();
										do_unknown(oldAddress, DOUNK_EXPAND);
										//auto_mark_range(uCurrentAddress,  uEnd, AU_UNK);
										autoWait();
										auto_mark_range(newAddress, uEnd, AU_CODE); 
										autoWait();	
										msg("JunkRemove: Fixed %x.\n", newAddress);

										uCurrentAddress = newAddress;
									}
								}
								//break;
							}
							else
							{
								break;
							}
						}
					}
				}
			}
			autoWait();
			jumpto(uStartAddress, 0);
		}
		else
		{
			msg("JunkRemove: No functions in IDB\n");
			return;
		}
		msg("JunkRemove: Clean Finished.\n");
	}
	else
	{
		msg("JunkRemove: IDA must be IDLE.\n");
	}

}

// Safe "jumpto()" that dosn't cash on out of bounds input address
ALIGN(32) void SafeJumpTo(segment_t* pSegment, ea_t ea)
{
	if(pSegment)
	{	
		try
		{			
			if(ea < pSegment->startEA)
				ea = pSegment->startEA;
			else
			if(ea > pSegment->endEA)
				pSegment->endEA;

			jumpto(ea, 0);
		}
		catch(PVOID)
		{
		}	
	}
}

// Get a nice line of disassembled code text sans color tags
ALIGN(32) LPCTSTR GetDisasmText(ea_t ea)
{
    static char szBuff[MAXSTR];
    memset(szBuff, 0, MAXSTR);
    generate_disasm_line(ea, szBuff, (sizeof(szBuff) - 1));
    tag_remove(szBuff, szBuff, (sizeof(szBuff) - 1));
    return(szBuff);
}
