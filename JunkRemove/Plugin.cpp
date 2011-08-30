// Run IDA in plug in debug mode with -z20
#include "stdafx.h"

int IDAP_init();
void IDAP_term();
void IDAP_run(int arg);
extern void CORE_Init();
extern void CORE_Process(int iArg);
extern void CORE_Exit();

char IDAP_comment[] = "JunkRemove: Remove Junk code of SHK shell.";
char IDAP_help[] 	= "JunkRemove: Press the hot key to remove junk code.";
char IDAP_name[] 	= "JunkRemove";
char IDAP_hotkey[] 	= "Alt-5";		// Preferred/default

extern "C" ALIGN(32) plugin_t PLUGIN =
{
	IDP_INTERFACE_VERSION,	// IDA version plug-in is written for
	0, //PLUGIN_UNL,	    // Plug-in flags
	IDAP_init,	            // Initialization function
	IDAP_term,	            // Clean-up function
	IDAP_run,	            // Main plug-in body
	IDAP_comment,	        // Comment - unused
	IDAP_help,	            // As above - unused
	IDAP_name,	            // Plug-in name shown in Edit->Plugins menu
	IDAP_hotkey	            // Hot key to run the plug-in
};

int IDAP_init()
{
    CORE_Init();
    return(PLUGIN_OK);   
}

void IDAP_term()
{
    CORE_Exit();
}

void IDAP_run(int iArg)
{	
    CORE_Process(iArg);   
}
