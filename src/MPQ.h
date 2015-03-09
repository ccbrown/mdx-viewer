#ifndef MPQ_H
#define MPQ_H

#include <stdint.h>
#include <stdio.h>
#include <list>

#define MPQ_FORMAT_V1 0
#define MPQ_FORMAT_V2 1
#define MPQ_FORMAT_V3 2
#define MPQ_FORMAT_V4 3

#define MPQ_MAX_TABLE_ENTRIES          1024 * 1024 * 1024

#define MPQ_COMPRESSION_LZMA           0x12 // not a combination of flags

#define MPQ_COMPRESSION_HUFFMAN        0x01
#define MPQ_COMPRESSION_ZLIB           0x02
#define MPQ_COMPRESSION_PKWARE         0x08
#define MPQ_COMPRESSION_BZIP2          0x10
#define MPQ_COMPRESSION_SPARSE         0x20
#define MPQ_COMPRESSION_ADPCM_MONO     0x40
#define MPQ_COMPRESSION_ADPCM_STEREO   0x80

#define MPQ_FILE_COMPRESSED            0x00000200
#define MPQ_FILE_PATCH_FILE            0x00100000
#define MPQ_FILE_SINGLE_UNIT           0x01000000
#define MPQ_FILE_DELETE_MARKER         0x02000000
#define MPQ_FILE_SECTOR_CRC            0x04000000
#define MPQ_FILE_EXISTS                0x80000000

#pragma pack(push, 1)

typedef struct {
	uint32_t magic;
	uint32_t headerSize;
	uint32_t archiveSize;
	uint16_t format;
	uint16_t blockSize;
	uint32_t hashTableOffset;
	uint32_t blockTableOffset;
	uint32_t hashTableEntries;
	uint32_t blockTableEntries;
} MPQHeader;

typedef struct {
	uint64_t highBlockTableOffset;
	uint16_t hashTableOffsetHigh;
	uint16_t blockTableOffsetHigh;
} MPQV2Header;

typedef struct {
	uint64_t archiveSize;
	uint64_t betOffset;
	uint64_t hetOffset;
} MPQV3Header;

typedef struct {
	uint64_t compressedHashTableSize;
	uint64_t compressedBlockTableSize;
	uint64_t compressedHighBlockTableSize;
	uint64_t compressedHETTableSize;
	uint64_t compressedBETTableSize;
	uint32_t md5ChunkSize;
	unsigned char blockTableMD5[16];
	unsigned char hashTableMD5[16];
	unsigned char highBlockTableMD5[16];
	unsigned char BETTableMD5[16];
	unsigned char HETTableMD5[16];
	unsigned char headerMD5[16];
} MPQV4Header;

typedef struct {
	uint32_t hash1;
	uint32_t hash2;
	uint16_t locale;
	uint16_t platform;
	uint32_t blockIndex;
} MPQHashTableEntry;

typedef struct {
	uint32_t offset;
	uint32_t compressedSize;
	uint32_t uncompressedSize;
	uint32_t flags;
} MPQBlockTableEntry;

typedef struct {
	uint16_t offsetHigh;
} MPQHighBlockTableEntry;

typedef enum {
	MPQHashTypeStart,
	MPQHashTypeHash1,
	MPQHashTypeHash2,
	MPQHashTypeTableKey,
	MPQHashTypeCount
} MPQHashType;

#pragma pack(pop)

class MPQFile {

	public:
	
		MPQFile(const char* name);
		~MPQFile();
		
		char*  data();
		size_t size();

	private:
	
		char*  _data;
		size_t _size;
};

class MPQ {
	
	public:
	
		MPQ(const char* name);
		~MPQ();
		
		bool isOpen();
		const char* name();
		
		bool getFileInfo(const char* name, size_t* size, uint32_t* flags);
		uint32_t readFile(void* dest, size_t destlen, const char* name);

		void addToChain();
		void removeFromChain();
		
	private:
	
		uint32_t _getBlockIndex(const char* name);
	
		FILE* _f;
		bool _isOpen;
		char* _name;

		MPQHeader   _header;
		MPQV2Header _v2Header;
		MPQV3Header _v3Header;
		MPQV4Header _v4Header;
		
		MPQHashTableEntry*      _hashTable;
		MPQBlockTableEntry*     _blockTable;
		MPQHighBlockTableEntry* _highBlockTable;
		
		unsigned char* _blockBuffer;
};

void MPQInit();
void MPQDestroy();

uint32_t MPQHashString(const char* string, MPQHashType type);
void MPQDecryptTable(void* table, size_t size, const char* key);
size_t MPQDecompress(void* dest, size_t destlen, void* source, size_t sourcelen);

#endif