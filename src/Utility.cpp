#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "Utility.h"

char* StringTrim(char* string) {
	char* l;
	for (l = string + strlen(string) - 1; l >= string; --l) {
		if (*l == ' ') {
			*l = NULL;
		} else {
			break;
		}
	}
	if (*l == '\0') {
		return NULL;
	}
	char* f;
	for (f = string; f <= l; ++f) {
		if (*f != ' ') {
			break;
		}
	}
	return f;
}

char* StringDuplicate(const char* string) {
	StringDuplicate((char*)string);
}

char* StringDuplicate(char* string) {
	int len = strlen(string);
	char* ret = (char*)malloc(len + 1);
	memcpy(ret, string, len);
	ret[len] = '\0';
	return ret;
}

bool StringIsEmpty(char* string) {
	for (int i = 0, n = strlen(string); i < n; ++i) {
		if (string[i] > 0x32 && string[i] != 0x7F && (unsigned char)string[i] != 0xFF) {
			return false;
		}
	}
	
	return true;
}

char* FileKey(const char* file) {
	char* key = (char*)malloc(strlen(file) + 1);
	strcpy(key, file);
	
	char** component = (char**)malloc(sizeof(char*) * strlen(key));
	int c = 0;
	
	char * pch = strtok(key, "/\\");
	while (pch != NULL) {
		if (!strcmp(pch, "..")) {
			if (c <= 0) {
				break;
			}
			--c;
		} else if (strcmp(pch, ".")) {
			component[c++] = pch;
		}
		pch = strtok(NULL, "/\\");
	}

	if (c <= 0) {
		free(key);
		free(component);
		return NULL;
	}
	
	int j = 0;
	for (int i = 0; i < c; ++i) {
		int l = strlen(component[i]);
		memcpy(key + j, component[i], l);
		j += l;
		key[j++] = '/';
	}
	if (j > 0) {
		key[j-1] = '\0';
	} else {
		key[0] = '\0';
	}

	free(component);
	
	return key;
}

long long unsigned int TimeUSecs() {
	timeval tv;
	gettimeofday(&tv, NULL);
	return (tv.tv_sec * 1000) + (tv.tv_usec / 1000);
}

char* ReadFile(const char* filename, char* buffer, long int* size) {
	FILE* f = fopen(filename, "rb");
	
	if (f == NULL) {
		return NULL;
	}
	
	fseek(f, 0, SEEK_END);
	long fsize = ftell(f);
	rewind(f);
	
	if (fsize > *size) {
		fclose(f);
		return NULL;
	}
	
	*size = fread(buffer, 1, fsize, f);
	
	fclose(f);
	
	return buffer;
}