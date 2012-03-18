#include "stdafx.h"
#include <wincrypt.h>
#include "md5.h"

#define  FILE_BUFFER_SIZE (1*1024*1024)
//4096
unsigned char fileBuffer[FILE_BUFFER_SIZE] = {0};

bool CalculateMD5( LPCTSTR filepath, __int64 /*size*/, unsigned char* md5 )
{
	HCRYPTPROV hCryptProv = 0;
	HCRYPTHASH hHash = 0;
	if(CryptAcquireContext(&hCryptProv, NULL, MS_ENHANCED_PROV, PROV_RSA_FULL, CRYPT_VERIFYCONTEXT) || CryptAcquireContext(&hCryptProv, NULL, MS_ENHANCED_PROV, PROV_RSA_FULL, 0))
	{
		FILE* pFile = _tfopen(filepath, TEXT("rb"));
		if (pFile)
		{
			CryptCreateHash(hCryptProv, CALG_MD5, 0, 0, &hHash);
			for (int i=0; i<256; i++)
			{
				while(!feof(pFile))
				{
					memset(fileBuffer, 0, FILE_BUFFER_SIZE);
					size_t count = fread(fileBuffer, 1, FILE_BUFFER_SIZE, pFile);
					CryptHashData(hHash, fileBuffer, count, 0);
				}
			}
			fclose(pFile);

			DWORD dwCount = 16;
			CryptGetHashParam(hHash, HP_HASHVAL, md5, &dwCount, 0);
			CryptDestroyHash(hHash);
		}
		CryptReleaseContext(hCryptProv, 0);
    }
    return false;
}
