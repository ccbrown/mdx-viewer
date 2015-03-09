#ifndef COMPRESSION_H
#define COMPRESSION_H

#include <sys/types.h>

// These return the number of bytes written to dest or 0 if an error occured.
int ZLIBInflate(void* dest, size_t dest_bytes, const void* source, size_t source_byte);

#endif