/*
 * ShpLoader.h
 *
 *  Created on: 02.08.2018
 *      Author: marek
 */

#ifndef SHPLOADER_H_
#define SHPLOADER_H_

#include <stdint.h>
#include <memory>
#include <vector>

using namespace std;

class ShpLoader
{
	enum Format
	{
		XORPrev = 0x20, XORLCW = 0x40, LCW = 0x80
	};

	struct ShpImage
	{
	public:
		ShpImage(ifstream &file);
		ShpImage(const ShpImage &obj) = delete;
		ShpImage(ShpImage &&obj);
		virtual ~ShpImage();

		uint32_t offset;
		Format format;
		Format refFormat;
		uint16_t refOffset;
		ShpImage *refImage = nullptr;
		unique_ptr<char[]> data;
	};

public:
	ShpLoader(const char *path);

	virtual ~ShpLoader();

	vector<ShpImage> images;
	uint16_t width = 0;
	uint16_t height = 0;
private:
	uint16_t imgCount = 0;

	int recurseDepth = 0;
	ifstream file;

	void decompress(ShpImage &img);
	int lcwDecode(ShpImage &img);
	int xorDeltaDecode(ShpImage &img);
};

#endif /* SHPLOADER_H_ */
