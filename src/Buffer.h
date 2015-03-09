#ifndef BUFFER_H
#define BUFFER_H

#include <stdlib.h>
#include <string.h>

// this is a convenience template
// there's no bounds or overflow protection done here
template<class T>
class Buffer {

	public:
	
		Buffer<T>() {
			_size = 0;
			_contents = NULL;
			_isConstructed = false;
		}
		
		~Buffer<T>() {
			dealloc();
		}
		
		size_t size() {
			return _size;
		}
				
		T* alloc(size_t size) {
			dealloc();
			_isConstructed = false;
			if (_contents = (T*)malloc(size)) {
				_size = size;
			} else {
				_size = 0;
			}
			return _contents;
		}

		T* construct(size_t elements) {
			dealloc();
			_isConstructed = true;
			_contents = new T[elements];
			_size = sizeof(T) * elements;
			return _contents;
		}
		
		void dealloc() {
			if (_isConstructed) {
				delete [] _contents;
			} else {
				free(_contents);
			}
		}
		
		T* write(size_t offset, const void* data, size_t size) {
			if (offset + size > _size) {
				return NULL;
			}
			memcpy((char*)_contents + offset, data, size);
			return _contents;
		}

		T* set(const void* contents, size_t size) {
			if (!alloc(size)) {
				return NULL;
			}
			memcpy(_contents, contents, size);
			return _contents;
		}
		
		unsigned int count() {
			return (_size / sizeof(T));
		}
						
		T& operator[](const int n) {
			return _contents[n];
		}

		T* operator*() {
			return _contents;
		}

		T* operator+(const int right) {
			return &_contents[right];
		}
						
	private:

		size_t _size;
		T*     _contents;
		
		bool _isConstructed;
		
};

#endif