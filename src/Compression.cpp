#include <zlib.h>

#include "Compression.h"

int ZLIBInflate(void* dest, size_t dest_bytes, const void* source, size_t source_bytes) {
	z_stream strm;
	strm.zalloc = Z_NULL;
	strm.zfree = Z_NULL;
	strm.opaque = Z_NULL;

	if (inflateInit(&strm) != Z_OK) {
		return 0;
	}

	strm.avail_in  = source_bytes;
	strm.next_in   = (Bytef*)source;
	strm.avail_out = dest_bytes;
	strm.next_out  = (Bytef*)dest;
	
	if (inflate(&strm, Z_FINISH) != Z_STREAM_END) {
		inflateEnd(&strm);
		return 0;
	}

	inflateEnd(&strm);
	return dest_bytes - strm.avail_out;
}
