/*
 * Palette.cpp
 *
 *  Created on: 02.08.2018
 *      Author: marek
 */

#include <iostream>
#include <fstream>

#include "Palette.h"

Palette::Palette(const char *path)
{
	uint8_t r, g, b;

	ifstream file;
	file.open(path, ios::binary);
	if(!file.is_open())
	{
		cerr << "Can't open file: " << path << endl;
		return;
	}

	for(int i = 0; i < PaletteSize; i++)
	{
		file.read((char *) &r, sizeof(r));
		file.read((char *) &g, sizeof(g));
		file.read((char *) &b, sizeof(b));
		r <<= 2;
		g <<= 2;
		b <<= 2;
		colors[i] = (255 << 24) | (r << 16) | (g << 8) | b;
	}
	colors[0] = 0; // Convert black background to transparency.

	file.close();
}

Palette::~Palette()
{
}

