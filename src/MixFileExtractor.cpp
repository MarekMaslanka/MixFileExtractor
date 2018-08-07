//============================================================================
// Name        : MixFileExtractor.cpp
// Author      : Marek Maślanka
// Version     :
// Copyright   : 
// Description : MIX file extractor
//============================================================================

#include <iostream>
#include <fstream>
#include <stdint.h>
#include <memory>
#include <functional>
#include <png.h>

#include "Palette.h"
#include "ShpLoader.h"

using namespace std;

int saveToPngFile(uint8_t *data, uint16_t width, uint16_t height, Palette &palette, const char *path);

int main(int argc, char **argv)
{
	if(argc < 4)
	{
		cerr << "Usage: program <*.shp> <*.pal> <*.png>" << endl;
		return 1;
	}
	ShpLoader shp(argv[1]);
	Palette palette(argv[2]);
	saveToPngFile(reinterpret_cast<uint8_t *>(shp.images[0].data.get()), shp.width, shp.height, palette, argv[3]);
	return 0;
}

int saveToPngFile(uint8_t *data, uint16_t width, uint16_t height, Palette &palette, const char *path)
{
	png_structp png_ptr = NULL;
	png_infop info_ptr = NULL;
	size_t x, y;
	png_bytepp row_pointers;
	int pixel_size = 4;
	int depth = 8;

	std::unique_ptr<FILE, int (*)(FILE *)> fp(fopen(path, "wb"), fclose);
	if(!fp)
	{
		return -1;
	}

	png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
	if(!png_ptr)
	{
		return -1;
	}

	unique_ptr<png_info, function<void(png_infop)>> ip(png_create_info_struct(png_ptr), [&png_ptr](png_infop info_ptr)
	{
		png_destroy_write_struct(&png_ptr, &info_ptr);
	});
	if(!ip)
	{
		return -1;
	}
	info_ptr = ip.get();
	png_set_IHDR(png_ptr, info_ptr, width, height, depth, PNG_COLOR_TYPE_RGBA, PNG_INTERLACE_NONE,
				 PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);

	row_pointers = static_cast<png_byte **>(png_malloc(png_ptr, height * sizeof(png_byte *)));
	for(y = 0; y < height; ++y)
	{
		png_bytep row = static_cast<png_byte *>(png_malloc(png_ptr, sizeof(uint8_t) * width * pixel_size));
		row_pointers[y] = row;
		for(x = 0; x < width; ++x)
		{
			// TODO: use libpng palette
			int c = palette.colors[data[y * height + x]];
			*row++ = c >> 16 & 0xFF;
			*row++ = c >> 8 & 0xFF;
			*row++ = c >> 0 & 0xFF;
			*row++ = c >> 24 & 0xFF;
		}
	}

	png_init_io(png_ptr, fp.get());
	png_set_rows(png_ptr, info_ptr, row_pointers);
	png_write_png(png_ptr, info_ptr, PNG_TRANSFORM_IDENTITY, NULL);

	for(y = 0; y < height; y++)
	{
		png_free(png_ptr, row_pointers[y]);
	}
	png_free(png_ptr, row_pointers);

	return 0;
}