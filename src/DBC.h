#ifndef DBC_H
#define DBC_H

#include "MPQ.h"

typedef struct {
	uint32_t id;
	uint32_t name;
	uint32_t weaponFlags;
	uint32_t bodyFlags;
	uint32_t flags;
	uint32_t unknown;
	uint32_t fallback;
	uint32_t behaviorId;
	uint32_t behaviorTier;
} DBCAnimationDataRecord;

typedef struct {
	uint32_t id;
	uint32_t model;
	uint32_t sound;
	uint32_t extra;
	float    scale;
	uint32_t opacity;
	uint32_t skin1;
	uint32_t skin2;
	uint32_t skin3;
	// more
} DBCCreatureDisplayInfoRecord;

typedef struct {
	uint32_t id;
	uint32_t flags;
	uint32_t path;
	// more
} DBCCreatureModelDataRecord;

typedef struct {
	uint32_t magic;
	uint32_t records;
	uint32_t fields;
	uint32_t recordSize;
	uint32_t stringBlockSize;
} DBCHeader;

template <class T>
class DBC {
	
	public:
	
		DBC(const char* name);
		~DBC();
		
		bool isOpen();
		
		T* record(unsigned int index);
		
		T* recordWithId(unsigned int id);
		
		const char* string(unsigned int offset);

	private:
	
		bool _isOpen;
		
		MPQFile* _file;
		DBCHeader* _header;
		
};

void DBCInit();
void DBCDestroy();

DBC<DBCAnimationDataRecord>*        DBCAnimationData();
DBC<DBCCreatureDisplayInfoRecord>*  DBCCreatureDisplayInfo();
DBC<DBCCreatureModelDataRecord>*    DBCCreatureModelData();

// DBC template

template <class T>
DBC<T>::DBC(const char* name) {
	_isOpen = false;
	
	_file = new MPQFile(name);
	
	char* data  = _file->data();
	size_t size = _file->size();
	
	if (!data) {
		printf("Couldn't open DBC.\n");
		return;
	}
	
	_header = (DBCHeader*)data;

	if (size < sizeof(DBCHeader) || _header->magic != 'CBDW') {
		printf("Couldn't read DBC header.");
		return;
	}
			
	_isOpen = true;
}

template <class T>
DBC<T>::~DBC() {
	delete _file;
}

template <class T>
bool DBC<T>::isOpen() {
	return _isOpen;
}

template <class T>
T* DBC<T>::record(unsigned int index) {
	off_t offset = sizeof(DBCHeader) + _header->recordSize * index;
	if (index >= _header->records || offset + _header->recordSize > _file->size()) {
		return NULL;
	}
	return (T*)(_file->data() + offset);
}

template <class T>
T* DBC<T>::recordWithId(unsigned int id) {
	// are the records guaranteed to be in order by id? can we do a binary search instead?
	for (unsigned int i = 0; i < _header->records; ++i) {
		T* r = record(i);
		if (*(uint32_t*)r == id) {
			return r;
		}
	}
	return NULL;
}

template <class T>
const char* DBC<T>::string(unsigned int offset) {
	off_t stringOffset = sizeof(DBCHeader) + _header->recordSize * _header->records + offset;
	if (stringOffset > _file->size()) {
		return NULL;
	}
	return _file->data() + stringOffset;
}

#endif