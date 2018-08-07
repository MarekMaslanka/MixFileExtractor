/*
 * Palette.h
 *
 *  Created on: 02.08.2018
 *      Author: marek
 */

#ifndef PALETTE_H_
#define PALETTE_H_

#include <stdint.h>

using namespace std;

static const int PaletteSize = 256;

class Palette
{
public:
	Palette(const char *path);
	virtual ~Palette();

	uint32_t colors[PaletteSize];
};

#endif /* PALETTE_H_ */
