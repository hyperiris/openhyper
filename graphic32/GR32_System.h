//unit GR32_System;
#pragma once
#include <atlstr.h>

class TPerfTimer
{
public:
	void Start();
	CString ReadNanoseconds();
	CString ReadMilliseconds();
	CString ReadSeconds();
	INT64 ReadValue();
private:
	LARGE_INTEGER FFrequency;
	LARGE_INTEGER FPerformanceCountStart;
	LARGE_INTEGER FPerformanceCountStop;
};
    
/*{ Returns the number of processors configured by the operating system. }*/
DWORD GetProcessorCount();

/*{ TCPUInstructionSet, defines specific CPU technologies }*/
enum TCPUInstructionSet
{
	ciNULL = 0,
	ciMMX = 0x800000,
	ciEMMX = 0x400000,
	ciSSE = 0x2000000,
	ciSSE2 = 0x4000000,
	ci3DNow = 0x80000000,
	ci3DNowExt = 0x40000000,
};

typedef DWORD TCPUFeatures;
typedef TCPUFeatures* PCPUFeatures;

/*{ General function that returns whether a particular instruction set is
  supported for the current CPU or not }*/
BOOL HasInstructionSet(const TCPUInstructionSet InstructionSet);
TCPUFeatures CPUFeatures();

extern TPerfTimer GlobalPerfTimer;
