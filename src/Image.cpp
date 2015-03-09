#include "Image.h"

#include <OpenGL/gl.h>
#include <png.h>
#include <stdlib.h>

bool LoadPNGImage(char* path, int &outWidth, int &outHeight, bool &outHasAlpha, GLubyte** outData) {
	png_structp png_ptr;
	png_infop info_ptr;
	unsigned int sig_read = 0;
	int color_type, interlace_type;
	FILE *fp;
	 
	if ((fp = fopen(path, "rb")) == NULL) {
		return false;
	}

	png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
	 
	if (png_ptr == NULL) {
		fclose(fp);
		return false;
	}
	 
	info_ptr = png_create_info_struct(png_ptr);
	if (info_ptr == NULL) {
		png_destroy_read_struct(&png_ptr, NULL, NULL);
		fclose(fp);
		return false;
	}
 
	if (setjmp(png_jmpbuf(png_ptr))) {
		png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
		fclose(fp);
		return false;
	}

	png_init_io(png_ptr, fp);
	png_set_sig_bytes(png_ptr, sig_read);

	png_read_png(png_ptr, info_ptr, PNG_TRANSFORM_STRIP_16 | PNG_TRANSFORM_PACKING | PNG_TRANSFORM_EXPAND, NULL);

	outWidth = png_get_image_width(png_ptr, info_ptr);
	outHeight = png_get_image_height(png_ptr, info_ptr);
	
	switch (png_get_color_type(png_ptr, info_ptr)) {
		case PNG_COLOR_TYPE_RGBA:
			outHasAlpha = true;
			break;
		case PNG_COLOR_TYPE_RGB:
			outHasAlpha = false;
			break;
		default:
			printf("Color type not supported!\n");
			png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
			fclose(fp);
			return false;
	}

	unsigned int row_bytes = png_get_rowbytes(png_ptr, info_ptr);
	*outData = (unsigned char*)malloc(row_bytes * outHeight);

	png_bytepp row_pointers = png_get_rows(png_ptr, info_ptr);

	for (int i = 0; i < outHeight; i++) {
		memcpy(*outData+(row_bytes * (outHeight-1-i)), row_pointers[i], row_bytes);
	}

	png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
	fclose(fp);

	return true;
}