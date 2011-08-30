#pragma once
#define WIN32_LEAN_AND_MEAN
#define WINVER       0x0502 // WinXP++    
#define _WIN32_WINNT 0x0502

#include <windows.h>
#include <time.h>

#include <crtdbg.h>

// IDA libs
//#define __NOT_ONLY_PRO_FUNCS__
#include <pro.h>
#include <ida.hpp>
#include <idp.hpp>
#include <auto.hpp>
#include <expr.hpp>
#include <bytes.hpp>
#include <ua.hpp>
#include <loader.hpp>
#include <kernwin.hpp>
#include <diskio.hpp>
#include <funcs.hpp>
#include <search.hpp>

#include "Utility.h"

#define JM_VERSION	"0.1"
