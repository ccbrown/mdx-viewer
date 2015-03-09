#include "Compression.h"
#include "MPQ.h"

#include <stdlib.h>

const char* gMPQStandardMPQNames[] = {
	"/Volumes/Macintosh HD/Applications/World of Warcraft/Data/expansion4.MPQ",
	"/Volumes/Macintosh HD/Applications/World of Warcraft/Data/expansion3.MPQ",
	"/Volumes/Macintosh HD/Applications/World of Warcraft/Data/expansion2.MPQ",
	"/Volumes/Macintosh HD/Applications/World of Warcraft/Data/expansion1.MPQ",
	"/Volumes/Macintosh HD/Applications/World of Warcraft/Data/enUS/locale-enUS.MPQ",
	"/Volumes/Macintosh HD/Applications/World of Warcraft/Data/sound.MPQ",
	"/Volumes/Macintosh HD/Applications/World of Warcraft/Data/world.MPQ",
	"/Volumes/Macintosh HD/Applications/World of Warcraft/Data/misc.MPQ",
	"/Volumes/Macintosh HD/Applications/World of Warcraft/Data/model.MPQ",
	"/Volumes/Macintosh HD/Applications/World of Warcraft/Data/texture.MPQ",
	"/Volumes/Macintosh HD/Applications/World of Warcraft/Data/itemtexture.MPQ",
};

std::list<MPQ*> gMPQChain;
std::list<MPQ*> gMPQStandardMPQs;
uint32_t        gMPQDecryptTable[0x500];

MPQFile::MPQFile(const char* name) {
	_data = NULL;
	_size = 0;

	MPQ* mpq = NULL;
	size_t size;
	uint32_t flags;
	
	std::list<MPQ*> patchChain;
	
	for (std::list<MPQ*>::iterator it = gMPQChain.begin(); it != gMPQChain.end(); ++it) {
		if ((*it)->getFileInfo(name, &size, &flags)) {
			if ((flags & MPQ_FILE_EXISTS) && (flags & MPQ_FILE_PATCH_FILE)) {
				patchChain.push_back(*it);
			} else if ((flags & MPQ_FILE_EXISTS) || (flags & MPQ_FILE_DELETE_MARKER)) {
				mpq = *it;
				break;
			}
		}
	}
	
	if (!mpq || flags & MPQ_FILE_DELETE_MARKER) {
		printf("Couldn't open %s\n", name);
		return;
	}

	// TODO: patch files	
//	if (patchChain.size() > 0) {
//		printf("WARNING: Patch files not supported yet. Using unpatched %s\n", name);
//		printf("base:  %s\n", mpq->name());
//		for (std::list<MPQ*>::reverse_iterator it = patchChain.rbegin(); it != patchChain.rend(); ++it) {
//			printf("patch: %s\n", (*it)->name());
//		}
//	}
	
	_size = size;
	_data = (char*)malloc(_size);

	if (!_data) {
		printf("Couldn't allocate memory for %s\n", name);
		_size = 0;
		return;
	}

	_size = mpq->readFile(_data, _size, name);
	
//	FILE* fout = fopen(name, "wb");
//	fwrite(_data, _size, 1, fout);
//	fclose(fout);
}

MPQFile::~MPQFile() {
	free(_data);
}

char* MPQFile::data() {
	return _data;
}

size_t MPQFile::size() {
	return _size;
}

MPQ::MPQ(const char* name) {
	_isOpen = false;
	_name = (char*)malloc(strlen(name) + 1);
	strcpy(_name, name);

	_hashTable      = NULL;
	_blockTable     = NULL;
	_highBlockTable = NULL;
	
	_blockBuffer = NULL;

	_f = fopen(name, "rb");
	
	if (!_f) {
		printf("Couldn't open %s\n", name);
		return;
	}

	off_t headerOffset = 0;
	while (true) {
		if (feof(_f)) {
			printf("Couldn't read header.\n");
			return;
		}
		uint32_t w;
		if (fread(&w, 4, 1, _f) != 1) {
			printf("Couldn't read header.\n");
			return;
		}
		if (w == '\x1bQPM') {
			fseeko(_f, 4, SEEK_CUR);
			if (fread(&w, 4, 1, _f) != 1) {
				printf("Couldn't read shunt.\n");
				return;
			}
			headerOffset += w;
			break;
		} else if (w == '\x1aQPM') {
			break;
		}
		
		headerOffset += 0x200;
		fseeko(_f, 0x200, SEEK_CUR);
	}

	fseeko(_f, headerOffset, SEEK_SET);
	if (fread(&_header, sizeof(MPQHeader), 1, _f) != 1) {
		printf("Couldn't read header.\n");
		return;
	}
	
	if (_header.blockSize > 16) {
		printf("Couldn't read header.\n");
		return;
	}
	
	free(_blockBuffer);
	_blockBuffer = (unsigned char*)malloc(0x200 << _header.blockSize);
	
	off_t hashTableOffset  = _header.hashTableOffset;
	off_t blockTableOffset = _header.blockTableOffset;

	if (_header.format >= MPQ_FORMAT_V2) {
		if (fread(&_v2Header, sizeof(MPQV2Header), 1, _f) != 1) {
			printf("Couldn't read V2 header.\n");
			return;
		}
		
		hashTableOffset  += ((off_t)_v2Header.hashTableOffsetHigh  << 32);
		blockTableOffset += ((off_t)_v2Header.blockTableOffsetHigh << 32);
		
		if (_header.format >= MPQ_FORMAT_V3) {
			if (fread(&_v3Header, sizeof(MPQV3Header), 1, _f) != 1) {
				printf("Couldn't read V3 header.\n");
				return;
			}
			
			if (_header.format >= MPQ_FORMAT_V4) {
				if (fread(&_v4Header, sizeof(MPQV4Header), 1, _f) != 1) {
					printf("Couldn't read V4 header.\n");
					return;
				}
			}
		}
	}
		
	// read the hash table
	
	if (_header.hashTableEntries > MPQ_MAX_TABLE_ENTRIES || (_header.hashTableEntries & (_header.hashTableEntries - 1))) {
		printf("Couldn't read hash table.\n");
		return;
	}
	
	size_t hashTableSize = sizeof(MPQHashTableEntry) * _header.hashTableEntries;
	
	free(_hashTable); // just in case we move this code
	_hashTable = (MPQHashTableEntry*)malloc(hashTableSize);
	
	if (!_hashTable) {
		printf("Couldn't allocate memory.\n");
		return;
	}
	
	if (_header.format >= MPQ_FORMAT_V4 && _v4Header.compressedHashTableSize != hashTableSize) {
		if (_v4Header.compressedHashTableSize > hashTableSize) {
			printf("Couldn't read hash table.\n");
			return;
		}
		fseeko(_f, hashTableOffset, SEEK_SET);
		if (fread(_hashTable, _v4Header.compressedHashTableSize, 1, _f) != 1) {
			printf("Couldn't read hash table.\n");
			return;
		}
		MPQDecryptTable(_hashTable, _v4Header.compressedHashTableSize, "(hash table)");
		if (hashTableSize != MPQDecompress(_hashTable, hashTableSize, _hashTable, _v4Header.compressedHashTableSize)) {
			printf("Couldn't decompress hash table.\n");
			return;
		}
	} else {
		fseeko(_f, hashTableOffset, SEEK_SET);
		if (fread(_hashTable, hashTableSize, 1, _f) != 1) {
			printf("Couldn't read hash table.\n");
			return;
		}	
		MPQDecryptTable(_hashTable, hashTableSize, "(hash table)");
	}
	
	// read the block table

	if (_header.blockTableEntries > MPQ_MAX_TABLE_ENTRIES) {
		printf("Couldn't read block table.\n");
		return;
	}

	size_t blockTableSize = sizeof(MPQBlockTableEntry) * _header.blockTableEntries;
	
	free(_blockTable); // just in case we move this code
	_blockTable = (MPQBlockTableEntry*)malloc(blockTableSize);
	
	if (!_blockTable) {
		printf("Couldn't allocate memory.\n");
		return;
	}
	
	if (_header.format >= MPQ_FORMAT_V4 && _v4Header.compressedBlockTableSize != blockTableSize) {
		if (_v4Header.compressedBlockTableSize > blockTableSize) {
			printf("Couldn't read block table.\n");
			return;
		}
		fseeko(_f, blockTableOffset, SEEK_SET);
		if (fread(_blockTable, _v4Header.compressedBlockTableSize, 1, _f) != 1) {
			printf("Couldn't read block table.\n");
			return;
		}
		MPQDecryptTable(_blockTable, _v4Header.compressedBlockTableSize, "(block table)");
		if (blockTableSize != MPQDecompress(_blockTable, blockTableSize, _blockTable, _v4Header.compressedBlockTableSize)) {
			printf("Couldn't decompress block table.\n");
			return;
		}
	} else {
		fseeko(_f, blockTableOffset, SEEK_SET);
		if (fread(_blockTable, blockTableSize, 1, _f) != 1) {
			printf("Couldn't read block table.\n");
			return;
		}
		MPQDecryptTable(_blockTable, blockTableSize, "(block table)");
	}
	
	// read the high block table if it exists
	
	if (_header.format >= MPQ_FORMAT_V2 && _v2Header.highBlockTableOffset) {
		size_t highBlockTableSize = sizeof(MPQHighBlockTableEntry) * _header.blockTableEntries;
		
		free(_highBlockTable); // just in case we move this code
		_highBlockTable = (MPQHighBlockTableEntry*)malloc(highBlockTableSize);
		
		if (!_highBlockTable) {
			printf("Couldn't allocate memory.\n");
			return;
		}
		
		if (_header.format >= MPQ_FORMAT_V4 && _v4Header.compressedHighBlockTableSize != highBlockTableSize) {
			if (_v4Header.compressedHighBlockTableSize > highBlockTableSize) {
				printf("Couldn't read high block table.\n");
				return;
			}
			fseeko(_f, _v2Header.highBlockTableOffset, SEEK_SET);
			if (fread(_highBlockTable, _v4Header.compressedHighBlockTableSize, 1, _f) != 1) {
				printf("Couldn't read high block table.\n");
				return;
			}
			if (highBlockTableSize != MPQDecompress(_highBlockTable, highBlockTableSize, _highBlockTable, _v4Header.compressedHighBlockTableSize)) {
				printf("Couldn't decompress high block table.\n");
				return;
			}
		} else {
			fseeko(_f, _v2Header.highBlockTableOffset, SEEK_SET);
			if (fread(_highBlockTable, highBlockTableSize, 1, _f) != 1) {
				printf("Couldn't read high block table.\n");
				return;
			}
		}
	}
	
	_isOpen = true;
	printf("Loaded %s\n", name);
}

MPQ::~MPQ() {
	removeFromChain();
	fclose(_f);
	
	free(_hashTable);
	free(_blockTable);
	free(_highBlockTable);

	free(_blockBuffer);

	free(_name);
}

bool MPQ::isOpen() {
	return _isOpen;
}

const char* MPQ::name() {
	return _name;
}

bool MPQ::getFileInfo(const char* name, size_t* size, uint32_t* flags) {
	if (!_isOpen) {
		return false;
	}

	uint32_t blockIndex = _getBlockIndex(name);
	
	if (blockIndex >= _header.blockTableEntries) {
		return false;
	}
	
	if (size) {
		*size = _blockTable[blockIndex].uncompressedSize;
	}
	
	if (flags) {
		*flags = _blockTable[blockIndex].flags;
	}

	return true;
}

uint32_t MPQ::readFile(void* dest, size_t destlen, const char* name) {
	if (!_isOpen) {
		return 0;
	}

	uint32_t blockIndex = _getBlockIndex(name);
	
	if (blockIndex >= _header.blockTableEntries) {
		return 0;
	}
	
	if (_blockTable[blockIndex].uncompressedSize > destlen || _blockTable[blockIndex].compressedSize > destlen) {
		return 0;
	}
	
	uint32_t flags = _blockTable[blockIndex].flags;
	
	off_t offset = _blockTable[blockIndex].offset;
	if (_highBlockTable) {
		offset |= ((uint64_t)_highBlockTable[blockIndex].offsetHigh << 32);
	}

	size_t blockSize = 0x200 << _header.blockSize;
	unsigned int blocks = (_blockTable[blockIndex].uncompressedSize + blockSize - 1) / blockSize;

	unsigned char* d = (unsigned char*)dest;
	size_t dbytes = 0;

	if (flags & MPQ_FILE_COMPRESSED) {
		if (flags & MPQ_FILE_SINGLE_UNIT) {
			if (_blockTable[blockIndex].compressedSize > (0x200 << _header.blockSize)) {
				printf("Couldn't read file unit.\n");
				return 0;
			}
			fseeko(_f, offset, SEEK_SET);
			if (fread(_blockBuffer, _blockTable[blockIndex].compressedSize, 1, _f) != 1) {
				printf("Couldn't read file unit.\n");
				return 0;
			}
			dbytes = MPQDecompress(d, destlen, _blockBuffer, _blockTable[blockIndex].compressedSize);
		} else {
			off_t boo = offset;
			
			uint32_t currentBlock, nextBlock;
			
			fseeko(_f, boo, SEEK_SET);
			if (fread(&currentBlock, 4, 1, _f) != 1) {
				printf("Couldn't read file blocks.\n");
				return 0;
			}
			boo += 4;
			for (unsigned int i = 0; i < blocks; ++i, currentBlock = nextBlock) {
				fseeko(_f, boo, SEEK_SET);
				if (fread(&nextBlock, 4, 1, _f) != 1) {
					printf("Couldn't read file blocks.\n");
					return 0;
				}
				boo += 4;
				fseeko(_f, offset + currentBlock, SEEK_SET);
				size_t bs = nextBlock - currentBlock;
				if (bs > blockSize) {
					printf("Couldn't read file block.\n");
					return 0;
				}
				fread(_blockBuffer, bs, 1, _f);
				size_t bytes = MPQDecompress(d, destlen - dbytes, _blockBuffer, bs);
				if (!bytes) {
					printf("Couldn't decompress file block.\n");
					return 0;
				}
				dbytes += bytes;
				d += bytes;
			}
		}
			
		if (dbytes != _blockTable[blockIndex].uncompressedSize) {
			printf("Couldn't read file.\n");
			return 0;
		}
	} else {
		printf("Uncompressed files are not supported yet.\n");
		return 0;
	}
	
	return dbytes;
}

void MPQ::addToChain() {
	gMPQChain.push_back(this);
}

void MPQ::removeFromChain() {
	gMPQChain.remove(this);
}


uint32_t MPQ::_getBlockIndex(const char* name) {
	if (!_isOpen) {
		return 0xFFFFFFFF;
	}

	uint32_t index      = MPQHashString(name, MPQHashTypeStart) & (_header.hashTableEntries - 1);	
	uint32_t hash1      = MPQHashString(name, MPQHashTypeHash1);
	uint32_t hash2      = MPQHashString(name, MPQHashTypeHash2);
	
	MPQHashTableEntry* entry = &_hashTable[index];
	while ((hash1 != entry->hash1 || hash2 != entry->hash2) && entry->blockIndex != 0xFFFFFFFF) {
		++entry;
		if (++index >= _header.hashTableEntries) {
			return 0xFFFFFFFF;
		}
	}
	
	return entry->blockIndex;
}

void MPQInit() {
	// initialize the decrypt table
	unsigned int seed = 0x00100001;
	for (unsigned int i = 0; i < 0x100; ++i) {
		for (unsigned int j = i, k = 0; k < 5; ++k, j += 0x100) {
			unsigned int a, b;
			seed  = (seed * 125 + 3) % 0x2AAAAB;
			a     = (seed & 0xFFFF) << 0x10;

			seed  = (seed * 125 + 3) % 0x2AAAAB;
			b     = (seed & 0xFFFF);

			gMPQDecryptTable[j] = (a | b);
		}
	}
		
	// load the standard MPQ chain
	for (int i = 0; i < sizeof(gMPQStandardMPQNames) / sizeof(char*); ++i) {
		MPQ* mpq = new MPQ(gMPQStandardMPQNames[i]);
		if (mpq->isOpen()) {
			mpq->addToChain();
			gMPQStandardMPQs.push_back(mpq);
		} else {
			delete mpq;
		}
	}
}

void MPQDestroy() {
	for (std::list<MPQ*>::iterator it = gMPQStandardMPQs.begin(); it != gMPQStandardMPQs.end(); ++it) {
		delete *it;
	}
	gMPQStandardMPQs.clear();
}

uint32_t MPQHashString(const char* string, MPQHashType type) {
	if (type >= MPQHashTypeCount) {
		return 0;
	}
	
	unsigned int seed1 = 0x7FED7FED;
	unsigned int seed2 = 0xEEEEEEEE;

	while (*string != 0) {
		unsigned int c = toupper(*string++);
		seed1 = gMPQDecryptTable[(type << 8) + c] ^ (seed1 + seed2);
		seed2 = c + seed1 + seed2 + (seed2 << 5) + 3;
	}

	return seed1;
}

void MPQDecryptTable(void* table, size_t size, const char* key) {
    unsigned int seed1 = MPQHashString(key, MPQHashTypeTableKey);
	unsigned int seed2 = 0xEEEEEEEE;
	
	uint32_t* t = (uint32_t*)table;
	
	while (size >= 4) {
		seed2 += gMPQDecryptTable[0x400 + (seed1 & 0xFF)];
		unsigned int c = *t ^ (seed1 + seed2);
		seed1  = ((~seed1 << 0x15) + 0x11111111) | (seed1 >> 0x0B);
		seed2  = c + seed2 + (seed2 << 5) + 3;
		*t++ = c;
		size -= 4;
	}
}

size_t MPQDecompress(void* dest, size_t destlen, void* source, size_t sourcelen) {
	if (sourcelen < 2) {
		return false;
	}
	
	size_t blen = destlen;
	unsigned char* buf1 = (unsigned char*)malloc(blen);
	
	size_t slen = sourcelen - 1;
	memcpy(buf1, (char*)source + 1, slen);	
	
	unsigned char compression = *(char*)source;
	
	if (compression == MPQ_COMPRESSION_LZMA) {
		printf("ZLMA not supported yet.\n");
		free(buf1);
		return 0;
	}

	unsigned char* buf2 = (unsigned char*)malloc(blen);
	unsigned char* s = buf1;
	unsigned char* d = buf2;
	unsigned char* t;
	
	if (compression & MPQ_COMPRESSION_ZLIB) {
		slen = ZLIBInflate(d, blen, s, slen);
		compression &= ~MPQ_COMPRESSION_ZLIB;
		t = d;
		d = s;
		s = t;
	}

	if (compression) {
		printf("Unknown compression type (0x%02X).\n", compression);
		free(buf1);
		free(buf2);
		return 0;
	}
	
	memcpy(dest, s, slen);

	free(buf1);
	free(buf2);

	return slen;
}
