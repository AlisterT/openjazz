
/*
 *
 * jj2layer.cpp
 *
 * 30th June 2010: Created jj2layer.cpp from parts of jj2levelframe.cpp
 *
 * Part of the OpenJazz project
 *
 *
 * Copyright (c) 2005-2010 Alister Thomson
 *
 * OpenJazz is distributed under the terms of
 * the GNU General Public License, version 2.0
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */

/*
 * Handles JJ2 level layers.
 *
 */


#include "jj2level.h"

#include "io/gfx/video.h"


JJ2Layer::JJ2Layer () {

	// Create a blank layer

	width = height = 1;

	grid = new JJ2Tile *[1];
	*grid = new JJ2Tile[1];

	(*grid)->tile = 0;

	return;

}


JJ2Layer::JJ2Layer (int newWidth, int newHeight, int flags) {

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

	return;

}


JJ2Layer::~JJ2Layer () {

	delete[] *grid;
	delete[] grid;

	return;

}


bool JJ2Layer::getFlipped (int x, int y) {

	// Get flipped. We aim to offend!

	if ((x < 0) || (y < 0) || (x >= width) || (y >= height)) return false;

	return grid[y][x].flipped;

}


int JJ2Layer::getHeight () {

	return height;

}


int JJ2Layer::getTile (int x, int y) {

	if (((x < 0) || (x >= width)) && !tileX) return 0;
	if (((y < 0) || (y >= height)) && !tileY) return 0;

	return grid[tileY? y % height: y][tileX? x % width: x].tile;

}


int JJ2Layer::getWidth () {

	return width;

}


void JJ2Layer::setFrame (unsigned char x, unsigned char y, unsigned char frame) {

	grid[y][x].frame = frame;

	return;

}


void JJ2Layer::setTile (unsigned char x, unsigned char y, unsigned short int tile, int tiles) {

	JJ2Tile* ge;

	ge = grid[y] + x;

	if (tiles > 0) {

		ge->flipped = tile & 0x400;
		ge->tile = tile & 0x3FF;

		if (ge->tile > tiles) ge->tile = 0;

	} else {

		// TSF

		ge->flipped = tile & 0x1000;
		ge->tile = tile & 0xFFF;

		if (ge->tile > -tiles) ge->tile = 0;

	}

	ge->frame = 0;

	return;

}


void JJ2Layer::draw (SDL_Surface* tileSet, SDL_Surface* flippedTileSet) {

	SDL_Rect src, dst;
	int vX, vY;
	int x, y;

	// Set tile drawing dimensions
	src.w = TTOI(1);
	src.h = TTOI(1);
	src.x = 0;


	// Calculate the layer view
	if (width <= 30) {

		vX = 0;
		vY = 0;

	} else {

		vX = FTOI(viewX);
		vY = FTOI(viewY);

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

