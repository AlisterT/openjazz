
/**
 *
 * @file jj2layer.cpp
 *
 * Part of the OpenJazz project
 *
 * @par History:
 * - 23rd August 2005: Created level.c
 * - 3rd February 2009: Renamed level.c to level.cpp
 * - 19th July 2009: Created levelframe.cpp from parts of level.cpp
 * - 29th June 2010: Created jj2levelframe.cpp from parts of levelframe.cpp
 * - 30th June 2010: Created jj2layer.cpp from parts of jj2levelframe.cpp
 *
 * @par Licence:
 * Copyright (c) 2005-2012 Alister Thomson
 *
 * OpenJazz is distributed under the terms of
 * the GNU General Public License, version 2.0
 *
 * @par Description:
 * Handles JJ2 level layers.
 *
 */


#include "jj2level.h"

#include "io/gfx/video.h"


/**
 * Create a blank 1-by-1 layer.
 */
JJ2Layer::JJ2Layer () {

	width = height = 1;

	grid = new JJ2Tile *[1];
	*grid = new JJ2Tile[1];

	(*grid)->tile = 0;

	return;

}


/**
 * Create a blank layer.
 *
 * @param newWidth The width of the layer (in tiles)
 * @param newHeight The height of the layer (in tiles)
 * @param newXSpeed The relative horizontal speed of the layer
 * @param newYSpeed The relative vertical speed of the layer
 * @param flags Layer flags
 */
JJ2Layer::JJ2Layer (int flags, int newWidth, int newHeight, fixed newXSpeed, fixed newYSpeed) {

	int row;

	width = newWidth;
	height = newHeight;

	grid = new JJ2Tile *[height];
	*grid = new JJ2Tile[width * height];

	for (row = 0; row < height; row++) grid[row] = *grid + (row * width);

	tileX = flags & 1;
	tileY = flags & 2;
	limit = flags & 4;
	warp = flags & 8;

	xSpeed = newXSpeed;
	ySpeed = newYSpeed;

	return;

}


/**
 * Delete the layer.
 */
JJ2Layer::~JJ2Layer () {

	delete[] *grid;
	delete[] grid;

	return;

}


/**
 * Get flipped. We aim to offend!
 *
 * @param x The x-coordinate of the tile (in tiles)
 * @param y The y-coordinate of the tile (in tiles)
 *
 * @return Whether or not the tile is flipped horizontally
 */
bool JJ2Layer::getFlipped (int x, int y) {

	if ((x < 0) || (y < 0) || (x >= width) || (y >= height)) return false;

	return grid[y][x].flipped;

}


/**
 * Get the height of the layer.
 *
 * @return The height of the layer (in tiles)
 */
int JJ2Layer::getHeight () {

	return height;

}


/**
 * Get the tile for the given co-ordinates.
 *
 * @param x The x-coordinate of the tile (in tiles)
 * @param y The y-coordinate of the tile (in tiles)
 *
 * @return The number of the tile
 */
int JJ2Layer::getTile (int x, int y) {

	if ((x < 0) || (y < 0)) return 0;

	if ((x >= width) && !tileX) return 0;
	if ((y >= height) && !tileY) return 0;

	return grid[y % height][x % width].tile;

}


/**
 * Get the width of the layer.
 *
 * @return The width of the layer (in tiles)
 */
int JJ2Layer::getWidth () {

	return width;

}


/**
 * Set the frame of the tile at the given co-ordinates.
 *
 * @param x The x-coordinate of the tile (in tiles)
 * @param y The y-coordinate of the tile (in tiles)
 * @param frame The new frame
 */
void JJ2Layer::setFrame (int x, int y, unsigned char frame) {

	grid[y][x].frame = frame;

	return;

}


/**
 * Set the tile at the given co-ordinates.
 *
 * @param x The x-coordinate of the tile (in tiles)
 * @param y The y-coordinate of the tile (in tiles)
 * @param tile The number of the tile
 * @param TSF Whether or not this is a TSF tile
 * @param tiles The total number of tiles
 */
void JJ2Layer::setTile (int x, int y, unsigned short int tile, bool TSF, int tiles) {

	JJ2Tile* ge;

	ge = grid[y] + x;

	if (TSF) {

		ge->flipped = tile & 0x1000;
		ge->tile = tile & 0xFFF;

	} else {

		ge->flipped = tile & 0x400;
		ge->tile = tile & 0x3FF;

	}

	if (ge->tile > tiles) ge->tile = 0;

	ge->frame = 0;

	return;

}


/**
 * Draw the layer.
 *
 * @param tileSet The tile set to use for non-flipped tiles
 * @param flippedTileSet The tile set to use for flipped tiles
 */
void JJ2Layer::draw (SDL_Surface* tileSet, SDL_Surface* flippedTileSet) {

	SDL_Rect src, dst;
	int vX, vY;
	int x, y;

	// Set tile drawing dimensions
	src.w = TTOI(1);
	src.h = TTOI(1);
	src.x = 0;


	// Calculate the layer view
	vX = FTOI(FTOI(viewX) * xSpeed);
	vY = FTOI(FTOI(viewY) * ySpeed);

	if (limit) {

		if (!tileX) {

			if (vX + canvasW > TTOI(width)) vX = TTOI(width) - canvasW;

		}

		if (!tileY) {

			vY -= canvasH - SH;
			if (vY + canvasH > TTOI(height)) vY = TTOI(height) - canvasH;

		}

	}

	for (y = 0; y <= ITOT(canvasH - 1) + 1; y++) {

		for (x = 0; x <= ITOT(canvasW - 1) + 1; x++) {

			dst.x = TTOI(x) - (vX & 31);
			dst.y = TTOI(y) - (vY & 31);
			src.y = TTOI(getTile(x + ITOT(vX), y + ITOT(vY)));
			if (src.y) SDL_BlitSurface(getFlipped(x + ITOT(vX), y + ITOT(vY))? flippedTileSet: tileSet, &src, canvas, &dst);

		}

	}

	return;

}

