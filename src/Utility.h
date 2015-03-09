#ifndef UTILITY_H
#define UTILITY_H

#include <stdint.h>
#include <sys/time.h>

// Modifies the string in-place.
char* StringTrim(char* string);

// The returned string must be free'd by the caller.
char* StringDuplicate(char* string);
char* StringDuplicate(const char* string);

bool StringIsEmpty(char* string);

char* FileKey(const char* file);

long long unsigned int TimeUSecs();

inline int NextPowerOf2(int a) {
	int rval = 1;
	while (rval < a) {
		rval<<=1;
	}
	return rval;
}

inline bool is_little_endian() {
	union {
		uint32_t i;
		char c[4];
	} n = {0x01020304};

	return (n.c[0] != 1);
}

inline uint16_t htoans(uint16_t n) {
	if (is_little_endian()) {
		return n;
	}

	return (n << 8) | (n >> 8);
}

inline uint32_t htoanl(uint32_t n) {
	if (is_little_endian()) {
		return n;
	}

    return (n << 24) | ((n << 8) & 0x00FF0000) | ((n >> 8) & 0x0000FF00) | (n >> 24);
}

inline uint16_t antohs(uint16_t n) {
	return htoans(n);
}

inline uint32_t antohl(uint32_t n) {
	return htoanl(n);
}

char* ReadFile(const char* filename, char* buffer, long int* size);

#endif