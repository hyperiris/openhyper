// duplicate.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "md5.h"

#define MD5_SIZE 16

struct FileInfo
{
	std::wstring filename;
	__int64 filesize;
	unsigned char md5[MD5_SIZE];
	bool dup;
	FileInfo():filesize(0),dup(false)
	{
		memset(md5, 0, MD5_SIZE);
	}
};

std::vector<FileInfo*> vFileInfo;

bool SortFileInfo(FileInfo* l, FileInfo* r)
{
	return l->filesize < r->filesize;
}


void FillFileInfoList(LPCTSTR path)
{
	WIN32_FIND_DATA ffd;
	HANDLE hFind = FindFirstFile(path, &ffd);
	if (hFind == INVALID_HANDLE_VALUE) 
	{
		//printf ("FindFirstFile failed (%d)\n", GetLastError());
		return;
	} 
	do
	{
		if (ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
		{
			if (wcscmp(L".", ffd.cFileName)!=0 && wcscmp(L"..", ffd.cFileName)!=0)
			{
				TCHAR buffer[MAX_PATH * 2] = {};
				_tcscpy(buffer, path);
				PathRemoveFileSpec(buffer);
				_tcscat(buffer, TEXT("\\"));
				_tcscat(buffer, ffd.cFileName);
				_tcscat(buffer, TEXT("\\*"));

				FillFileInfoList(buffer);
			}
		}
		else
		{
			TCHAR buffer[MAX_PATH * 2] = {};
			_tcscpy(buffer, path);
			PathRemoveFileSpec(buffer);
			_tcscat(buffer, TEXT("\\"));
			_tcscat(buffer, ffd.cFileName);

			__int64 fsize = (ffd.nFileSizeHigh * ((__int64)MAXDWORD+1)) + ffd.nFileSizeLow;
			if (fsize > 1 * 1024 * 1024)
			{
				FileInfo* pFileInfo = new FileInfo;
				pFileInfo->filename = buffer;
				pFileInfo->filesize = fsize;

				vFileInfo.push_back(pFileInfo);
			}
			//_tprintf(TEXT("  %s   %ld bytes\n"), ffd.cFileName, pFileInfo->filesize);
		}
	}
	while (FindNextFile(hFind, &ffd) != 0);
	FindClose(hFind);
}

int _tmain(int argc, _TCHAR* argv[])
{
	_tprintf(TEXT("Step1: get file list\n"));
	//列出所有文件
///*
	//FillFileInfoList(L"c:\\");
	FillFileInfoList(L"d:\\*");
	FillFileInfoList(L"e:\\*");
	FillFileInfoList(L"f:\\*");
	FillFileInfoList(L"g:\\*");
	FillFileInfoList(L"h:\\*");
	FillFileInfoList(L"i:\\*");
	//FillFileInfoList(L"j:\\");
	FillFileInfoList(L"k:\\*");
	FillFileInfoList(L"l:\\*");
	FillFileInfoList(L"m:\\*");
//*/
//	FillFileInfoList(L"p:\\*");

	_tprintf(TEXT("Step2: sort file size\n"));
	//按照文件大小排序
	std::sort(vFileInfo.begin(), vFileInfo.end(), SortFileInfo);

	_tprintf(TEXT("Step3: mark duplicate files\n"));
	//标记出尺寸相同的文件
	size_t i=0;
	while (i < vFileInfo.size())
	{
		FileInfo* pFileInfo = vFileInfo[i];
		if (pFileInfo->dup == true)
		{
			i++;
			continue;
		}
		__int64 size = pFileInfo->filesize;
		size_t j = i+1;
		while (j < vFileInfo.size())
		{
			FileInfo* pFileInfo2 = vFileInfo[j];
			if (pFileInfo2->filesize == size)
			{
				pFileInfo->dup = true;
				pFileInfo2->dup = true;
				j++;
			}
			else
			{
				break;
			}
		}
		i++;
	}

	_tprintf(TEXT("Step4: calculate file hash...\n"));
	for (size_t i=0; i<vFileInfo.size(); i++)
	{
		FileInfo* pFileInfo = vFileInfo[i];
		if (pFileInfo->dup)
		{
			CalculateMD5(pFileInfo->filename.c_str(), pFileInfo->filesize, pFileInfo->md5);
		}
	}

	_tprintf(TEXT("Step5: saving log...\n"));
	FILE* pFile = _tfopen(TEXT("c:\\file.log"), TEXT("wb"));
	WORD prefix = 0xFEFF;
	fwrite(&prefix, 2, 1, pFile);
	for (size_t i=0; i<vFileInfo.size(); i++)
	{
		FileInfo* pFileInfo = vFileInfo[i];
		if (pFileInfo->dup)
		{
			_ftprintf(pFile, TEXT("%s,%I64d,"), pFileInfo->filename.c_str(), pFileInfo->filesize);
			for (int j=0; j<MD5_SIZE; j++)
			{
				_ftprintf(pFile, TEXT("%x"), pFileInfo->md5[j]);
			}
			//_ftprintf(pFile, TEXT("%x"), pFileInfo->md5[15]);
			_ftprintf(pFile, TEXT("\n"));
		}
	}
	fclose(pFile);

	_tprintf(TEXT("Finished\n"));
	return 0;
}

