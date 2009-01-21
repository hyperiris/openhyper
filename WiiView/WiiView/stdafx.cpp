// stdafx.cpp : source file that includes just the standard includes
// WiiView.pch will be the pre-compiled header
// stdafx.obj will contain the pre-compiled type information

#include "stdafx.h"

WORD toWORD(WORD w)
{
	unsigned char w1 = w & 0xff;
	unsigned char w2 = w >> 8;
	return w = (w1 << 8) | w2;
}

DWORD toDWORD(DWORD d)
{
	unsigned char w1 = d & 0xff;
	unsigned char w2 = (d >> 8) & 0xff;
	unsigned char w3 = (d >> 16) & 0xff;
	unsigned char w4 = d >> 24;
	return d = (w1 << 24) | (w2 << 16) | (w3 << 8) | w4;
}
