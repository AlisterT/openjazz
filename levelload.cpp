
/*
 *
 * levelload.cpp
 *
 * Created as levelload.c on the 22nd of July 2008 from parts of level.c
 * Renamed levelload.cpp on the 3rd of February 2009
 *
 * Part of the OpenJazz project
 *
 *
 * Copyright (c) 2005-2009 Alister Thomson
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
 * Deals with the loading of ordinary levels.
 *
 */


#include "OpenJazz.h"
#include <string.h>


int Level::loadSprites (char * fn) {

	File *f, *mf, *sf;
	unsigned char *pixels, *sorted;
	int mposition, sposition;
	int count, x, y, width, height, m;

	// Open fn
	try {

		sf = new File(fn, false);

	} catch (int e) {

		return FAILURE;

	}

	// This function loads all the sprites, not fust those in fn
	// Note: Lower case is necessary for Unix support
	try {

		mf = new File("mainchar.000", false);

	} catch (int e) {

		delete sf;

		return FAILURE;

	}

	sprites = sf->loadShort();

	// Include space in the sprite set for the blank sprite at the end
	spriteSet = new Sprite[sprites + 1];

	// Read horizontal offsets
	for (count = 0; count < sprites; count++)
		spriteSet[count].x = sf->loadChar() << 2;

	// Read vertical offsets
	for (count = 0; count < sprites; count++)
		spriteSet[count].y = sf->loadChar();

	// Find where the sprites start in fn
	sposition = sf->tell();

	// Find where the sprites start in mainchar.000
	mposition = 2;

	// Loop through all the sprites to be loaded
	for (count = 0; count < sprites; count++) {

		// Go to the start of the current sprite or file indicator
		sf->seek(sposition, true);
		mf->seek(mposition, true);

		// If both fn and mainchar.000 have file indicators, create a blank
		// sprite
		while ((sf->loadChar() == 0xFF) && (mf->loadChar() == 0xFF)) {

			// Go to the next sprite/file indicator
			sf->seek(1, false);
			mf->seek(1, false);

			// set the position of the next sprite/file indicators
			sposition += 2;
			mposition += 2;

			// Create a blank sprite
			spriteSet[count].pixels = createBlankSurface();
			count++;

		}

		// Return to the start of the sprite/file indicators
		sf->seek(sposition, true);
		mf->seek(mposition, true);

		// Unless otherwise stated, load from fn
		f = sf;

		// Check if otherwise stated
		if (f->loadChar() == 0xFF) {

			f = mf;

		} else f->seek(-1, false);

		width = f->loadShort() << 2;
		height = f->loadShort();

		// Position of the next sprite or file indicator in each file
		if (f == sf) {

			mposition += 2;

			sposition += 10 + (f->loadShort() << 2);

		} else {

			sposition += 2;

			mposition += 10 + (f->loadShort() << 2);

		}

		// m is for MAGIC
		m = f->loadShort();

		// Allocate space for descrambling
		sorted = new unsigned char[width * height];

		// Actually, m is for mask offset.
		// Sprites can be either masked or not masked.
		if (!m) {

			// Not masked
			// Load the pixel data directly for descrambling

			f->seek(2, false);

			// Read pixel data
			pixels = f->loadBlock(width * height);


		} else {

			// Allocate space for pixel data
			pixels = new unsigned char[width * height];

			// Masked
			// Load the pixel data according to the mask

			// Masked sprites have their own next sprite offsets
			if (f == sf) {

				sposition = f->loadChar() << 2;
				sposition += f->loadChar() << 10;

			} else {

				mposition = f->loadChar() << 2;
				mposition += f->loadChar() << 10;

			}

			// Skip to mask
			f->seek(m, false);

			// Read the mask
			// Each mask pixel is either 0 or 1
			// Four pixels are packed into the lower end of each byte
			for (y = 0; y < height; y++) {

				for (x = 0; x < width; x++) {

					if (!(x & 3)) m = f->loadChar();
					pixels[(y * width) + x] = (m >> (x & 3)) & 1;

				}

			}

			// Pixels are loaded if the corresponding mask pixel is 1, otherwise
			// the transparent index is used. Pixels are scrambled, so the mask
			// has to be scrambled the same way.
			for (y = 0; y < height; y++) {

				for (x = 0; x < width; x++) {

					sorted[(((y >> 2) + ((x & 3) * (height >> 2))) * width) +
						(x >> 2) +
						(((y & 3) + ((height & 3) * (x & 3))) * (width >> 2))] =
						pixels[(y * width) + x];

				}

			}

			// Skip to pixels
			f->seek((width / 4), false);

			// Next sprite offsets are relative to here
			if (f == sf) sposition += f->tell();
			else mposition += f->tell();

			// Read pixels according to the scrambled mask
			for (y = 0; y < height; y++) {

				for (x = 0; x < width; x++) {

					if (sorted[(y * width) + x] == 1) {

						// The unmasked portions are transparent, so no masked
						// portion should be transparent.
						m = SKEY;

						while (m == SKEY) m = f->loadChar();

						// Use the acceptable pixel
						pixels[(y * width) + x] = m;

					} else {

						// Use the transparent pixel
						pixels[(y * width) + x] = SKEY;

					}

				}

			}


		}

		// Rearrange pixels in correct order
		for (y = 0; y < height; y++) {

			for (x = 0; x < width; x++) {

				sorted[(y * width) + x] =
					pixels[(((y >> 2) + ((x & 3) * (height >> 2))) * width) +
						(x >> 2) +
						(((y & 3) + ((height & 3) * (x & 3))) * (width >> 2))];

			}

		}


		// Convert the sprite to an SDL surface
		spriteSet[count].pixels = createSurface(sorted, width, height);
		SDL_SetColorKey(spriteSet[count].pixels, SDL_SRCCOLORKEY, SKEY);

		// Free redundant data
		delete[] pixels;


		// Check if the next sprite exists
		// If not, create blank sprites for the remainder
		if (sposition >= f->getSize()) {

			for (count++; count < sprites; count++) {

				spriteSet[count].pixels = createBlankSurface();

			}

		} else {

			sf->seek(sposition, true);

		}

	}

	delete mf;
	delete sf;


	// Include a blank sprite at the end

	spriteSet[sprites].pixels = createBlankSurface();
	spriteSet[sprites].x = 0;
	spriteSet[sprites].y = 0;

	return SUCCESS;

}


int Level::loadTiles (char * fn) {

	File *f;
	unsigned char *buffer;
	int rle, pos, index, count;
	int tiles;


	try {

		f = new File(fn, false);

	} catch (int e) {

		return 0;

	}


	// Load the palette
	f->loadPalette(levelPalette);


	// Load the background palette
	f->loadPalette(skyPalette);


	// Skip the second, identical, background palette
	f->skipRLE();


	// Load the tile pixel indices

	tiles = 240; // Never more than 240 tiles

	buffer = new unsigned char[tiles * TW * TH];

	f->seek(4, false);

	pos = 0;

	// Read the RLE pixels
	// file::loadRLE() cannot be used, for reasons that will become clear
	while ((pos < TW * TH * tiles) && (f->tell() < f->getSize())) {

		rle = f->loadChar();

		if (rle & 128) {

			index = f->loadChar();

			for (count = 0; count < (rle & 127); count++) buffer[pos++] = index;

		} else if (rle) {

			for (count = 0; count < rle; count++) buffer[pos++] = f->loadChar();

		} else { // This happens at the end of each tile

			// 0 pixels means 1 pixel, apparently
			buffer[pos++] = f->loadChar();

			f->seek(2, false); // I assume this is the length of the next tile
			                   // block
			if (pos == TW * TH * 60) f->seek(2, false); // Maybe these say
			if (pos == TW * TH * 120) f->seek(2, false); // whether or not there
			if (pos == TW * TH * 180) f->seek(2, false); // are any more tiles

		}

	}

	delete f;

	// Work out how many tiles were actually loaded
	// Should be a multiple of 60
	tiles = pos / (TW * TH);

	tileSet = createSurface(buffer, TW, TH * tiles);
	SDL_SetColorKey(tileSet, SDL_SRCCOLORKEY, TKEY);


	return tiles;

}


void Level::createPEs (int bgType) {

	switch (bgType) {

		case 2:

			// Sky background effect
			bgPE = new PaletteEffect(PE_SKY, 156, 100, FH, NULL);

			break;

		case 8:

			// Parallaxing background effect
			bgPE = new PaletteEffect(PE_2D, 128, 64, FE, NULL);

			break;

		case 9:

			// Diagonal stripes "parallaxing" background effect
			bgPE = new PaletteEffect(PE_1D, 128, 32, FH, NULL);

			break;

		case 11:

			// The deeper below water, the darker it gets
			bgPE = new PaletteEffect(PE_WATER, 1, 250, F32 * 32, NULL);

			break;

		default:

			// No effect, but bgPE must exist so here is a dummy animation
			bgPE = new PaletteEffect(PE_ROTATE, 255, 1, F1, NULL);

			break;

	}

	// Palette animations
	// These are applied to every level without a conflicting background effect
	// As a result, there are a few levels with things animated that shouldn't
	// be

	// In Diamondus: The red/yellow palette animation
	firstPE = new PaletteEffect(PE_ROTATE, 112, 4, F32, bgPE);

	// In Diamondus: The waterfall palette animation
	firstPE = new PaletteEffect(PE_ROTATE, 116, 8, F16, firstPE);

	// The following were discoverd by Unknown/Violet

	firstPE = new PaletteEffect(PE_ROTATE, 124, 3, F16, firstPE);

	if ((bgType != PE_1D) && (bgType != PE_2D))
		firstPE = new PaletteEffect(PE_ROTATE, 132, 8, F16, firstPE);

	if ((bgType != PE_SKY) && (bgType != PE_2D))
		firstPE = new PaletteEffect(PE_ROTATE, 160, 32, -F16, firstPE);

	if (bgType != PE_SKY) {

		firstPE = new PaletteEffect(PE_ROTATE, 192, 32, F32, firstPE);
		firstPE = new PaletteEffect(PE_ROTATE, 224, 16, F16, firstPE);

	}

	return;

}


Level::Level (char *fn, unsigned char diff, bool checkpoint) {

	File *f;
	unsigned char *buffer;
	char *string, *ext;
	unsigned char birdEvent[ELENGTH] = {0, 0, 0, 0, 8, 51, 52, 0, 0, 0, 7, 0,
		30, 25, 1, 1, 0, 10, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 51, 52};
	unsigned char tiles;
	int count, x, y;


	difficulty = diff;


	// Show loading screen

	// Open planet.### file

	string = new char[11];

	sprintf(string, "planet.%3s", fn + strlen(fn) - 3);

	try {

		f = new File(string, false);

	} catch (int e) {

		delete[] string;

		throw FAILURE;

	}

	delete[] string;

	f->seek(2, true);
	string = f->loadString();

	for (count = 0; string[count]; count++) string[count] |= 32;


	SDL_FillRect(screen, NULL, 0);
	fontmn2->showString("loading", (screenW >> 1) - 96, (screenH >> 1) - 16);
	fontmn2->showString(string, (screenW >> 1) - 16, (screenH >> 1) - 32);

	delete[] string;

	switch (fn[5]) {

		case 48:

			string = "level one";

			break;

		case 49:

			string = "level two";

			break;

		default:

			string = "secret level";

			break;

	}

	fontmn2->showString(string, (screenW >> 1) - 16, screenH >> 1);

	update();

	delete f;



	// Open level file

	try {

		f = new File(fn, false);

	} catch (int e) {

		throw FAILURE;

	}

	localPlayer = players;

	// Load level data from a Level#.### file

	// Load the blocks.### extension
	// Thanks to Doubble Dutch for this bit
	f->seek(f->getSize() - 6, true);
	while (f->loadChar() != 3) f->seek(-2, false);
	f->seek(-1, false);
	ext = f->loadString();
	worldNum = (ext[0] * 100) + (ext[1] * 10) + ext[0] - (4800 + 480 + 48);


	// Get the level number from the file name
	levelNum = fn[strlen(fn) - 5];
	if (levelNum < 48 || levelNum > 57) levelNum = 0;
	else levelNum -= 48;


	// Allocate space for file names
	string = new char[12];


	// Load tile set from appropriate blocks.###
	// Note: Lower case is required for Unix support

	if (worldNum == 999) {

		delete[] ext;

		ext = fn + strlen(fn) - 3;
		worldNum = (ext[0] * 100) + (ext[1] * 10) + ext[0];

		sprintf(string, "blocks.%3s", ext);

	} else {

		sprintf(string, "blocks.%3s", ext);

		delete[] ext;

	}

	tiles = loadTiles(string);

	if (!tiles) {

		delete[] string;
		delete f;

		throw FAILURE;

	}


	// Load sprite set from corresponding Sprites.###
	// Note: Lower case is required for Unix support
	sprintf(string, "sprites.%3s", fn + strlen(fn) - 3);

	if (loadSprites(string) == FAILURE) {

		SDL_FreeSurface(tileSet);
		delete[] string;
		delete f;

		throw FAILURE;

	}

	delete[] string;


	// Skip to tile and event reference data
	f->seek(39, true);

	// Load tile and event references

	buffer = f->loadRLE(LW * LH * 2);

	// Create grid from data
	for (x = 0; x < LW; x++) {

		for (y = 0; y < LH; y++) {

			grid[y][x].tile = buffer[(y + (x * LH)) << 1];
			grid[y][x].bg = buffer[((y + (x * LH)) << 1) + 1] >> 7;
			grid[y][x].event = buffer[((y + (x * LH)) << 1) + 1] & 127;
			grid[y][x].hits = 0;

		}

	}

	delete[] buffer;

	// A mysterious block of mystery
	f->skipRLE();


	// Load mask data

	buffer = f->loadRLE(tiles * 8);

	// Unpack bits
	for (count = 0; count < tiles; count++) {

		for (y = 0; y < 8; y++) {

			for (x = 0; x < 8; x++)
				mask[count][(y << 3) + x] = (buffer[(count << 3) + y] >> x) & 1;

		}

	}

	delete[] buffer;

/* Uncomment the code below if you want to see the mask instead of the tile
   graphics during gameplay */

/*   if (SDL_MUSTLOCK(tileSet)) SDL_LockSurface(tileSet);

	for (count = 0; count < nTiles; count++) {

		for (y = 0; y < TH; y++) {

			for (x = 0; x < TW; x++) {

				if (mask[count][((y / 4) * 8) + (x / 4)] == 1)
					((char *)(tileSet->pixels))
						[(count * 32 * 32) + (y * 32) + x] = 88;

			}

		}

	}

	if (SDL_MUSTLOCK(tileSet)) SDL_UnlockSurface(tileSet);*/


	// Load special event path

	buffer = f->loadRLE(8192);
	pathLength = buffer[0] + (buffer[1] << 8);
	pathNode = 0;
	if (pathLength < 1) pathLength = 1;
	pathX = new signed char[pathLength];
	pathY = new signed char[pathLength];

	for (count = 0; count < pathLength; count++) {

		pathX[count] = ((signed char *)buffer)[(count << 1) + 3];
		pathY[count] = ((signed char *)buffer)[(count << 1) + 2];

	}

	delete[] buffer;


	// Load event set

	buffer = f->loadRLE(EVENTS * ELENGTH);

	// Set event 0
	memset(eventSet[0], 0, ELENGTH);

	// Fill event set with data
	for (count = 1; count < EVENTS; count++) {

		memcpy(eventSet[count], buffer + (count * ELENGTH), ELENGTH);
		eventSet[count][E_MOVEMENTSP]++;

	}

	delete[] buffer;

	// Create the bird
	memcpy(eventSet[121], birdEvent, ELENGTH);


	// Process grid

	enemies = items = 0;

	for (x = 0; x < LW; x++) {

		for (y = 0; y < LH; y++) {

			// Eliminate event references for events of too high a difficulty
			if (eventSet[grid[y][x].event][E_DIFFICULTY] > difficulty)
				grid[y][x].event = 0;

			// If the event hurts and can be killed, it is an enemy
			// Anything else that scores is an item
			if ((eventSet[grid[y][x].event][E_MODIFIER] == 0) &&
				eventSet[grid[y][x].event][E_HITSTOKILL]) enemies++;
			else if (eventSet[grid[y][x].event][E_ADDEDSCORE]) items++;

		}

	}


	// Yet more doubtless essential data
	f->skipRLE();


	// Load animation set

	buffer = f->loadRLE(ANIMS * 64);

	// Create animation set based on that data
	for (count = 0; count < ANIMS; count++) {

		animSet[count].frames = buffer[(count * 64) + 6];

		for (y = 0; y < animSet[count].frames; y++) {

			// Get frame
			x = buffer[(count * 64) + 7 + y];
			if (x > sprites) x = sprites;

			animSet[count].sprites[y] = spriteSet[x];

			// Get vertical offset
			animSet[count].y[y] = buffer[(count * 64) + 45 + y];

		}

	}

	delete[] buffer;


	// At general data

	// There's a a whole load of unknown data around here

	// Like another one of those pesky RLE blocks
	f->skipRLE();

	// And 505 bytes of DOOM
	f->seek(505, false);

	// Music file
	string = f->loadString();
	playMusic(string);

	// 25 bytes of undiscovered usefulness, less the music file name
	f->seek(25 - strlen(string), false);
	delete[] string;

	// End of episode cutscene
	sceneFile = f->loadString();

	// 51 bytes of undiscovered usefulness, less the cutscene file name
	f->seek(51 - strlen(sceneFile), false);

	// Finally, some data I know how to use!

	// First up, the player's coordinates
	x = f->loadShort();
	y = f->loadShort() + 1;

	if (!checkpoint) localPlayer->setCheckpoint(x, y);

	// Set the player's initial values
	localPlayer->reset();


	// Store fn, as it is about to become invalid
	currentFile = new char[strlen(fn) + 1];
	strcpy(currentFile, fn);

	// Next level
	x = f->loadChar();
	y = f->loadChar();

	if (x != 99) {

		setNext(x, y);

	} else {

		string = new char[8];
		sprintf(string, "endepis"); // Just like Jazz 2
		menuInst->setNextLevel(string);

	}


	// Thanks to Doubble Dutch for this next bit
	f->seek(4, false);
	waterLevel = f->loadShort() << 10;


	// Thanks to Feline and the JCS94 team for the next bits:

	f->seek(3, false);

	// Now at "Section 15"


	// Load player's animation set references

	buffer = f->loadRLE(PANIMS * 2);

	for (count = 0; count < PANIMS; count++)
		localPlayer->setAnim(count, buffer[count << 1]);

	delete[] buffer;


	// Now at "Section 16"

	// Skip to "Section 17" - .atk
	f->seek(4, false);


	// Load bullet set
	buffer = f->loadRLE(BULLETS * BLENGTH);

	for (count = 0; count < BULLETS; count++)
		memcpy(bulletSet[count], buffer + (count * BLENGTH), BLENGTH);

	delete[] buffer;


	// Now at "Section 18." More skippability.
	f->skipRLE();


	// Now at "Section 19," THE MAGIC SECTION

	// First byte should be the bg mode
	createPEs(f->loadChar());

	f->seek(1, false);

	skyOrb = f->loadChar(); // A.k.a the sun, the moon, the brightest star, that
	                        // red planet with blue veins...


	// And that's us done!

	delete f;


	// Apply the palette to surfaces that already exist, e.g. fonts
	usePalette(levelPalette);

	// Adjust fontmn1 to use level palette
	fontmn1->scalePalette(-F2, (16 * -2) + 240);


	// Set the tick at which the level will end
	endTime = (5 - difficulty) * 2 * 60 * 1000;
	winTime = 0;


	firstBullet = NULL;
	firstEvent = NULL;


	return;

}


Level::~Level () {

	// Free all data

	int count;

	stopMusic();

	// Free the palette effects
	if (firstPE) {

		delete firstPE;
		firstPE = NULL;

	}

	// Free events
	if (firstEvent) {

		while (firstEvent->getNext()) firstEvent->removeNext();

		delete firstEvent;
		firstEvent = NULL;

	}

	// Free bullets
	if (firstBullet) {

		while (firstBullet->getNext()) firstBullet->removeNext();

		delete firstBullet;
		firstBullet = NULL;

	}

	delete[] pathX;
	delete[] pathY;

	SDL_FreeSurface(tileSet);

	for (count = 0; count <= sprites; count++)
		SDL_FreeSurface(spriteSet[count].pixels);

	delete[] spriteSet;

	fontmn1->restorePalette();

	delete[] sceneFile;

	delete[] currentFile;

	return;

}


