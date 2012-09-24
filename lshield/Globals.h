/********************************************************************
created:	2007/03/03
filename: 	Globals.h
author:		HyperIris

product:	Lachesis Shield
purpose:	Global data
*********************************************************************/
#pragma once

extern HINSTANCE  g_hInst;
extern UINT       g_DllRefCount;

//Shield State
#define Disable		0
#define Normal		1
#define High		2
#define OutOfRange	3

#define ID_TIMER	0

#define TIME_LENGTH	300000

extern int g_ShieldLevel;
extern bool g_Hooked;
extern HMODULE	g_hMod;
