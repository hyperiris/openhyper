// Wii.h
//

#pragma once

#include <Windows.h>

#define OFFSET_ISO_HEAD				0x0
#define OFFSET_REGION_SETTING		0x4E000
#define OFFSET_PARTITIONS_INFO		0x40000

#define SIZE_CLUSTER				0x8000
#define SIZE_CLUSTER_HEAD			0x0400
#define SIZE_CLUSTER_DATA			0x7c00

#pragma pack(push, 1)
struct TIsoHead 
{
	unsigned char diskType;				//	0x0000000000 	1 	Disctype R = Revolution/Wii, G = GameCube, U = Utility-disc(?) GameCube (GBA-Player disc is using U) 
	char gameCode[2];					//	0x0000000001 	2 	Gamecode 
	char region;						//	0x0000000003 	1 	Region, E = USA, P = PAL, J = JAP 
	char makerCode[2];					//	0x0000000004 	2 	Maker Code
	unsigned char diskID;				//	0x0000000006 	1 	Disc ID 
	unsigned char version;				//	0x0000000007 	1 	Version 
	unsigned char audioStreaming;		//	0x0000000008 	1 	Audio Streaming 
	unsigned char streamingBufferSize;	//	0x0000000009 	1 	Streaming Buffersize 
	unsigned char unk[14];				//	0x000000000A 	14 	zero, unused? 
	unsigned int magicWord;				//	0x0000000018 	4 	magicword, 0x5D1C9EA3 
	unsigned int unk2;
	char gameTitle[0x60];				//	0x0000000020 	0x60 	Gametitle (though most docs claim it to be 0x400 the Wii only reads 0x60) 
	unsigned char paddedData[0x380];	//	0x0000000080 	0x380 	padded with 0
};

struct TRegionSetting
{
	unsigned int region;
	unsigned int unk[3];
	unsigned int unk2[4];
};

struct TPartitionsInfo 
{
	unsigned int partitionsCount;		//	0x0000040000 	4 	Total partitions in the disc 
	unsigned int partitionTableOffset;	//	0x0000040004 	4 	Partition info table offset, Address is (value << 2)
};

struct TPartitionsTableItem 
{
	unsigned int partitionOffset;		//	0x0000000000 	4 	Partition offset, Address is (value << 2) 
	unsigned int unk;					//	0x0000000004 	4 	Partitions to follow ?, partition type ? It's always 1 for the first entry
};

struct TFirstCertificationItem
{
	// alsoe called tik
	unsigned char unk[0x1bf];
	unsigned char encryptedPartitionKey[16];	//	0x000001BF 	16 	Encrypted partition key 
	unsigned char unk2[0xd];
	unsigned char firstHalfOfPartitionKeyIV[8];	//	0x000001DC 	8 	First half of partition key IV (last half is zero) 
	unsigned char unk3[0xbc];
	unsigned int partitionOffsets;				// 	0x000002A0 	0x20 	Partition offsets 

	// also called tmd...
	unsigned int secondCertificationSize;		//	0x000002A4 	4 	unknown, ie: 0x00000208 
	unsigned int secondCertificationOffset;		//	0x000002A8 	4 	2nd certification offset 
	// also called cert
	unsigned int thirdCertificationSize;		//	0x000002AC 	4 	Size of the last three certifications 
	unsigned int thirdCertificationOffset;		//	0x000002B0 	4 	3rd certification offset 
	// also called h3_offset
	unsigned int pGlobalShaTable;				//	0x000002B4 	4 	Pointer to the global SHA-1 table (value << 2) 

	unsigned int dataOffset;					//	0x000002B8 	4 	Data offset, Address is (value << 2) 
	unsigned int dataSize;						//	0x000002BC 	4 	Data size, Address is (value << 2)
};

struct TClusterData
{
	unsigned char shaHashes[0x26c];		//	0x000 	0x26B 	0x26C 	31 SHA-1 hashes (20 bytes each), one for each block of 0x400 bytes of the decrypted user data for this cluster. 
	unsigned char padding[0x014];		//	0x26C 	0x27F 	0x014 	20 bytes of 0x00 padding 
	unsigned char shaHahses2[0x0a0];	//	0x280 	0x31F 	0x0A0 	8 SHA-1 hashes, one for each cluster in this subgroup. Each hash is of the 0x000-0x26B bytes, that is, of the 31 hashes above. This means that each cluster carries a hash of the data cluster hashes for each of the clusters in its subgroup. Every cluster in the subgroup has identical data in this section. 
	unsigned char padding2[0x020];		//	0x320 	0x33F 	0x020 	32 bytes of 0x00 padding 
	unsigned char shaHashes3[0x0a0];	//	0x340 	0x3DF 	0x0A0 	8 SHA-1 hashes, one for each subgroup in this group. Each hash is of the 0x280-0x31F bytes above. This means that each cluster carries a hash of the subgroup hash data for each of the subgroups in its group. All 64 clusters in a group have identical data in this section. Bytes 0x3D0-0x3DF here, when encrypted, serve as the IV for the user data. 
	unsigned char padding3[0x020];		//	0x3E0 	0x3FF 	0x020 	32 bytes of 0x00 padding
	unsigned char userData[0x7c00];
};

struct TFSTItem 
{
	unsigned char ItemType;
	unsigned char FileName[3];
	DWORD FileOffset;
	DWORD FileSize;
};

#pragma pack(pop)