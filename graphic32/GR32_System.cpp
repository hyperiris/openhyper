//unit GR32_System;
#include <stdafx.h>
#include <intrin.h> 

#include "GR32_System.h"


TPerfTimer GlobalPerfTimer;

BOOL CPUFeaturesInitialized = FALSE;
TCPUFeatures CPUFeaturesData = ciNULL;

/*{ TPerfTimer }*/

CString TPerfTimer::ReadNanoseconds()
{
	QueryPerformanceCounter(&FPerformanceCountStop);
	QueryPerformanceFrequency(&FFrequency);
	//Assert(FFrequency > 0);

	//Result := IntToStr(Round(1000000 * (FPerformanceCountStop - FPerformanceCountStart) / FFrequency));
	return L"";
}

CString TPerfTimer::ReadMilliseconds()
{
	QueryPerformanceCounter(&FPerformanceCountStop);
	QueryPerformanceFrequency(&FFrequency);
	//Assert(FFrequency > 0);

	//Result := FloatToStrF(1000 * (FPerformanceCountStop - FPerformanceCountStart) / FFrequency, ffFixed, 15, 3);
	return L"";
}

CString TPerfTimer::ReadSeconds()
{
	QueryPerformanceCounter(&FPerformanceCountStop);
	QueryPerformanceFrequency(&FFrequency);
	//Result := FloatToStrF((FPerformanceCountStop - FPerformanceCountStart) / FFrequency, ffFixed, 15, 3);
	return L"";
}

INT64 TPerfTimer::ReadValue()
{
	QueryPerformanceCounter(&FPerformanceCountStop);
	QueryPerformanceFrequency(&FFrequency);
	//Assert(FFrequency > 0);

	//Result := Round(1000000 * (FPerformanceCountStop - FPerformanceCountStart) / FFrequency);
	return 0;
}

void TPerfTimer::Start()
{
	QueryPerformanceCounter(&FPerformanceCountStart);
}

DWORD GetProcessorCount()
{
	SYSTEM_INFO sysInfo = {};
	GetSystemInfo(&sysInfo);
	return sysInfo.dwNumberOfProcessors;
}

BOOL CPUID_Available()
{
	BOOL bRet = FALSE;
	__asm
	{
		PUSHFD
		POP       EAX
		MOV       ECX,EAX
		XOR       EAX, 0x00200000
		PUSH      EAX
		POPFD
		PUSHFD
		POP       EAX
		XOR       ECX,EAX
		JZ        __label01
		MOV       bRet, 1
__label01:
		PUSH      EAX
		POPFD
	}
	return bRet;
}

int CPU_Signature()
{
	int CPUInfo[4] = {0};
	__cpuid(CPUInfo, 1);
	return CPUInfo[0];
}

int CPU_Features()
{
	int CPUInfo[4] = {0};
	__cpuid(CPUInfo, 1);
	return CPUInfo[3];
}

BOOL CPU_ExtensionsAvailable()
{
	int CPUInfo[4] = {0};
	__cpuid(CPUInfo, 0x80000000);
	if (CPUInfo[0] >= 0x80000000)
	{
		return FALSE;
	}
	return TRUE;
}

int CPU_ExtFeatures()
{
	int CPUInfo[4] = {0};
	__cpuid(CPUInfo, 0x80000001);
	return CPUInfo[3];
}

// Must be implemented for each target CPU on which specific functions rely
BOOL HasInstructionSet(const TCPUInstructionSet InstructionSet)
{
	if (!CPUID_Available()) return FALSE;					// no CPUID available
	if (((CPU_Signature() >> 8) & 0x0F) < 5) return FALSE;	// not a Pentium class

	switch (InstructionSet)
	{
	case ci3DNow:
	case ci3DNowExt:
		if (!CPU_ExtensionsAvailable() || ((CPU_ExtFeatures() & InstructionSet) == 0))
		{
			return FALSE;
		}
		break;
	case ciEMMX:
		// check for SSE, necessary for Intel CPUs because they don't implement the
		// extended info
		if (((CPU_Features() & ciSSE) == 0) &&
			(!CPU_ExtensionsAvailable() || (CPU_ExtFeatures() & ciEMMX) == 0))
		{
			return FALSE;
		}
		break;
	default:
		if ((CPU_Features() & InstructionSet) == 0)
		{
			return FALSE;
			// return -> instruction set not supported
		}
	}

	return TRUE;
}

void InitCPUFeaturesData()
{
	if (CPUFeaturesInitialized)
	{
		return;
	}

	CPUFeaturesData = ciNULL;
	if (HasInstructionSet(ciMMX)) CPUFeaturesData |= ciMMX;
	if (HasInstructionSet(ciEMMX)) CPUFeaturesData |= ciEMMX;
	if (HasInstructionSet(ciSSE)) CPUFeaturesData |= ciSSE;
	if (HasInstructionSet(ciSSE2)) CPUFeaturesData |= ciSSE2;
	if (HasInstructionSet(ci3DNow)) CPUFeaturesData |= ci3DNow;
	if (HasInstructionSet(ci3DNowExt)) CPUFeaturesData |= ci3DNowExt;

	CPUFeaturesInitialized = TRUE;
}

TCPUFeatures CPUFeatures()
{
	if (!CPUFeaturesInitialized)
	{
		InitCPUFeaturesData();
	}
	return CPUFeaturesData;
}
