
/*
 *
 * levelloadjj2.cpp
 *
 * 28th June 2010: Created levelloadjj2.cpp from parts of levelload.cpp
 * 29th June 2010: Renamed levelloadjj2.cpp to jj2levelload.cpp
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
 * Deals with the loading of JJ2 level data.
 *
 */


#include "jj2event/jj2event.h"
#include "jj2level.h"

#include "game/game.h"
#include "io/file.h"
#include "io/gfx/font.h"
#include "io/gfx/sprite.h"
#include "io/gfx/video.h"
#include "io/sound.h"
#include "menu/menu.h"
#include "player/jj2levelplayer.h"
#include "loop.h"
#include "util.h"

#include <string.h>


#define SKEY 254 /* Sprite colour key */


void JJ2Level::loadSprite (File* file, Sprite* sprite) {

	// TODO: Load sprites from JJ2 files

	unsigned char* pixels;
	int pos, maskOffset;
	int width, height;

	// Load dimensions
	width = file->loadShort() << 2;
	height = file->loadShort();

	file->seek(2, false);

	maskOffset = file->loadShort();

	pos = file->loadShort() << 2;

	// Sprites can be either masked or not masked.
	if (maskOffset) {

		// Masked

		height++;

		// Skip to mask
		file->seek(maskOffset, false);

		// Find the end of the data
		pos += file->tell() + ((width >> 2) * height);

		// Read scrambled, masked pixel data
		pixels = file->loadPixels(width * height, SKEY);
		sprite->setPixels(pixels, width, height, SKEY);

		delete[] pixels;

		file->seek(pos, true);

	} else if (width) {

		// Not masked

		// Read scrambled pixel data
		pixels = file->loadPixels(width * height);
		sprite->setPixels(pixels, width, height, SKEY);

		delete[] pixels;

	}


	return;

}


int JJ2Level::loadSprites (char * fileName) {

	// TODO: Load sprites from JJ2 files

	File* mainFile = NULL;
	File* specFile = NULL;
	int count;


	// Open fileName
	try {

		specFile = new File(fileName, false);

	} catch (int e) {

		return e;

	}


	// This function loads all the sprites, not fust those in fileName
	try {

		mainFile = new File(F_MAINCHAR, false);

	} catch (int e) {

		delete specFile;

		return e;

	}


	sprites = specFile->loadShort();

	// Include space in the sprite set for the blank sprite at the end
	spriteSet = new Sprite[sprites + 1];

	// Read horizontal offsets
	for (count = 0; count < sprites; count++)
		spriteSet[count].xOffset = specFile->loadChar() << 2;

	// Read vertical offsets
	for (count = 0; count < sprites; count++)
		spriteSet[count].yOffset = specFile->loadChar();


	// Skip to where the sprites start in mainchar.000
	mainFile->seek(2, true);


	// Loop through all the sprites to be loaded
	for (count = 0; count < sprites; count++) {

		if (specFile->loadChar() == 0xFF) {

			// Go to the next sprite/file indicator
			specFile->seek(1, false);

			if (mainFile->loadChar() == 0xFF) {

				// Go to the next sprite/file indicator
				mainFile->seek(1, false);

				/* Both fileName and mainchar.000 have file indicators, so
				create a blank sprite */
				spriteSet[count].clearPixels();

				continue;

			} else {

				// Return to the start of the sprite
				mainFile->seek(-1, false);

				// Load the individual sprite data
				loadSprite(mainFile, spriteSet + count);

			}

		} else {

			// Return to the start of the sprite
			specFile->seek(-1, false);

			// Go to the main file's next sprite/file indicator
			mainFile->seek(2, false);

			// Load the individual sprite data
			loadSprite(specFile, spriteSet + count);

		}


		// Check if the next sprite exists
		// If not, create blank sprites for the remainder
		if (specFile->tell() >= specFile->getSize()) {

			for (count++; count < sprites; count++) {

				spriteSet[count].clearPixels();

			}

		}

	}

	delete mainFile;
	delete specFile;


	// Include a blank sprite at the end
	spriteSet[sprites].clearPixels();
	spriteSet[sprites].xOffset = 0;
	spriteSet[sprites].yOffset = 0;

	return E_NONE;

}


int JJ2Level::loadTiles (char* fileName) {

	File* file;
	unsigned char* aBuffer;
	unsigned char* bBuffer;
	unsigned char* dBuffer;
	unsigned char* tileBuffer;
	int aCLength, bCLength, cCLength, dCLength;
	int aLength, bLength, cLength, dLength;
	int count, x, y;
	int maxTiles;
	int tiles;

	// Thanks to Neobeo for working out the most of the .j2t format


	try {

		file = new File(fileName, false);

	} catch (int e) {

		return e;

	}

	// Skip to version indicator
	file->seek(220, true);

	maxTiles = file->loadShort();

	if (maxTiles == 0x201) maxTiles = 4096;
	else maxTiles = 1024;


	// Skip to compressed block lengths
	file->seek(8, false);
	aCLength = file->loadInt();
	aLength = file->loadInt();
	bCLength = file->loadInt();
	bLength = file->loadInt();
	cCLength = file->loadInt();
	cLength = file->loadInt();
	dCLength = file->loadInt();
	dLength = file->loadInt();

	aBuffer = file->loadLZ(aCLength, aLength);
	bBuffer = file->loadLZ(bCLength, bLength);
	file->seek(cCLength, false); // Don't need this block
	dBuffer = file->loadLZ(dCLength, dLength);

	delete file;


	// Load the palette
	for (count = 0; count < 256; count++) {

		palette[count].r = aBuffer[count << 2];
		palette[count].g = aBuffer[(count << 2) + 1];
		palette[count].b = aBuffer[(count << 2) + 2];

	}


	// Load tiles

	tiles = createShort(aBuffer + 1024);
	tileBuffer = new unsigned char[tiles << 10];

	for (count = 0; count < tiles; count++) {

		memcpy(tileBuffer + (count << 10), bBuffer + createInt(aBuffer + 1028 + (maxTiles << 1) + (count << 2)), 1024);

	}

	tileSet = createSurface(tileBuffer, TTOI(1), TTOI(tiles));
	SDL_SetColorKey(tileSet, SDL_SRCCOLORKEY, 0);

	// Flip tiles
	for (count = 0; count < tiles * 32; count++) {

		for (x = 0; x < 16; x++) {

			y = tileBuffer[(count * 32) + x];
			tileBuffer[(count * 32) + x] = tileBuffer[(count * 32) + 31 - x];
			tileBuffer[(count * 32) + 31 - x] = y;

		}

	}

	flippedTileSet = createSurface(tileBuffer, TTOI(1), TTOI(tiles));
	SDL_SetColorKey(flippedTileSet, SDL_SRCCOLORKEY, 0);

	delete[] tileBuffer;


	// Load mask

	mask = new char[tiles << 10];

	// Unpack bits
	for (count = 0; count < tiles; count++) {

		for (y = 0; y < 32; y++) {

			for (x = 0; x < 32; x++)
				mask[(count << 10) + (y << 5) + x] = (dBuffer[createInt(aBuffer + 1028 + (maxTiles * 18) + (count << 2)) + (y << 2) + (x >> 3)] >> (x & 7)) & 1;

		}

	}

	flippedMask = new char[tiles << 10];

	// Unpack bits
	for (count = 0; count < tiles; count++) {

		for (y = 0; y < 32; y++) {

			for (x = 0; x < 32; x++)
				flippedMask[(count << 10) + (y << 5) + x] = (dBuffer[createInt(aBuffer + 1028 + (maxTiles * 22) + (count << 2)) + (y << 2) + (x >> 3)] >> (x & 7)) & 1;

		}

	}

	delete[] dBuffer;
	delete[] bBuffer;
	delete[] aBuffer;


	/* Uncomment the code below if you want to see the mask instead of the tile
	graphics during gameplay */

	/*if (SDL_MUSTLOCK(tileSet)) SDL_LockSurface(tileSet);
	if (SDL_MUSTLOCK(flippedTileSet)) SDL_LockSurface(flippedTileSet);

	for (count = 0; count < tiles; count++) {

		for (y = 0; y < 32; y++) {

			for (x = 0; x < 32; x++) {

				if (mask[(count << 10) + (y << 5) + x] == 1)
					((char *)(tileSet->pixels))[(count << 10) + (y << 5) + x] = 43;

				if (flippedMask[(count << 10) + (y << 5) + x] == 1)
					((char *)(flippedTileSet->pixels))[(count << 10) + (y << 5) + x] = 88;

			}

		}

	}

	if (SDL_MUSTLOCK(tileSet)) SDL_UnlockSurface(tileSet);
	if (SDL_MUSTLOCK(flippedTileSet)) SDL_UnlockSurface(flippedTileSet);*/


	return tiles | (maxTiles << 16);

}


int JJ2Level::load (char *fileName, unsigned char diff, bool checkpoint) {

	File *file;
	unsigned char *buffer;
	char *string;
	unsigned char* aBuffer;
	unsigned char* bBuffer;
	unsigned char* cBuffer;
	unsigned char* dBuffer;
	int aCLength, bCLength, cCLength, dCLength;
	int aLength, bLength, cLength, dLength;
	int tiles;
	int count, x, y;
	unsigned char tileQuad[8];
	short int* quadRefs;
	int flags, width, pitch, height;
	int worldNum;
	unsigned char startX, startY;

	// Thanks to Neobeo for working out the most of the .j2l format


	try {

		font = new Font(false);

	} catch (int e) {

		throw e;

	}


	difficulty = diff;


	// Open JJ2 level file

	try {

		file = new File(fileName, false);

	} catch (int e) {

		delete font;

		return e;

	}

	// Load level name
	file->seek(188, true);

	string = (char *)file->loadBlock(32);


	// Show loading screen

	video.setPalette(menuPalette);

	clearScreen(0);

	x = (canvasW >> 1) - ((strlen(string) + 6) << 2);
	x = fontmn2->showString("LOADING ", x - 60, (canvasH >> 1) - 16);
	fontmn2->showString(string, x, (canvasH >> 1) - 16);

	delete[] string;

	if (::loop(NORMAL_LOOP) == E_QUIT) return E_QUIT;


	// Skip to compressed block lengths
	file->seek(230, true);
	aCLength = file->loadInt();
	aLength = file->loadInt();
	bCLength = file->loadInt();
	bLength = file->loadInt();
	cCLength = file->loadInt();
	cLength = file->loadInt();
	dCLength = file->loadInt();
	dLength = file->loadInt();

	aBuffer = file->loadLZ(aCLength, aLength);
	bBuffer = file->loadLZ(bCLength, bLength);
	cBuffer = file->loadLZ(cCLength, cLength);
	dBuffer = file->loadLZ(dCLength, dLength);

	delete file;

	// Load tile set from given file

	tiles = loadTiles((char *)aBuffer + 51);

	if (tiles < 0) {

		delete[] dBuffer;
		delete[] cBuffer;
		delete[] bBuffer;
		delete[] aBuffer;

		delete font;

		return tiles;

	}

	TSF = tiles >> 28;
	tiles = tiles & 0xFFFF;


	// Next level
	string = (char *)aBuffer + 115;

	if (fileExists(string)) nextLevel = createString(string);
	else nextLevel = createString(string, ".j2l");


	// Music file
	string = (char *)aBuffer + 179;

	if (fileExists(string)) musicFile = createString(string);
	else musicFile = createString(string, ".j2b");


	// Create layers

	quadRefs = (short int *)dBuffer;

	for (count = 0; count < LAYERS; count++) {

		flags = aBuffer[8403 + (count << 2)];
		width = createInt(aBuffer + 8403 + 48 + (count << 2));
		pitch = createInt(aBuffer + 8403 + 80 + (count << 2));
		height = createInt(aBuffer + 8403 + 112 + (count << 2));

		if (aBuffer[8403 + 40 + count]) {

			layers[count] = new JJ2Layer(width, height, flags);

			for (y = 0; y < height; y++) {

				for (x = 0; x < width; x++) {

					if ((x & 3) == 0) memcpy(tileQuad, cBuffer + (quadRefs[x >> 2] << 3), 8);

					layers[count]->setTile(x, y, createShort(tileQuad + ((x & 3) << 1)), TSF? -tiles: tiles);

				}

				quadRefs += pitch >> 2;

			}

		} else {

			// No tile data

			layers[count] = new JJ2Layer();

		}

	}

	layer = layers[3];
	width = layer->getWidth();
	height = layer->getHeight();


	// Load events
	startX = 1;
	startY = 1;

	mods = new JJ2Modifier *[height];
	*mods = new JJ2Modifier[width * height];

	events = NULL;

	for (y = 0; y < height; y++) {

		mods[y] = *mods + (y * width);

		for (x = 0; x < width; x++) {

			count = bBuffer[((y * width) + x) << 2];

			if ((count < 33) || (count == 230)) {

				mods[y][x].type = count;
				mods[y][x].property = bBuffer[(((y * width) + x) << 2) + 1];

				if (count == 29) {

					// Jazz start pos

					startX = x;
					startY = y;

				}

			} else {

				mods[y][x].type = 0;

				events = new JJ2Event(events, x, y, bBuffer + (((y * width) + x) << 2));

			}

		}

	}

	delete[] dBuffer;
	delete[] cBuffer;
	delete[] bBuffer;
	delete[] aBuffer;



	// Open JJ1 level file for remaining data

	// Use first level
	worldNum = 0;

	string = createFileName(F_LEVEL, 0, worldNum);

	try {

		file = new File(string, false);

	} catch (int e) {

		// Failed to load first level, so try loading first Christmas level

		worldNum = 50;
		string[8] = '5';

		try {

			file = new File(string, false);

		} catch (int e) {

			delete[] string;

			if (events) delete events;
			delete[] *mods;
			delete[] mods;

			for (count = 0; count < LAYERS; count++) delete layers[count];

			delete[] flippedMask;
			delete[] mask;

			delete[] musicFile;
			delete[] nextLevel;

			SDL_FreeSurface(flippedTileSet);
			SDL_FreeSurface(tileSet);

			delete font;

			return e;

		}

	}

	delete[] string;


	// Load sprite set from corresponding Sprites.###

	string = createFileName(F_SPRITES, worldNum);

	count = loadSprites(string);

	delete[] string;

	if (count < 0) {

		delete file;

		if (events) delete events;
		delete[] *mods;
		delete[] mods;

		for (x = 0; x < LAYERS; x++) delete layers[x];

		delete[] flippedMask;
		delete[] mask;

		delete[] musicFile;
		delete[] nextLevel;

		SDL_FreeSurface(flippedTileSet);
		SDL_FreeSurface(tileSet);

		delete font;

		return count;

	}


	// Skip to tile and event reference data
	file->seek(39, true);

	// Skip tile references
	file->skipRLE();

	// A mysterious block of mystery
	file->skipRLE();

	// Skip mask data
	file->skipRLE();

	// Skip special event path
	file->skipRLE();

	// Skip event set
	file->skipRLE();

	// Yet more doubtless essential data
	file->skipRLE();


	// Load animation set

	buffer = file->loadRLE(128 << 6);

	// Create animation set based on that data
	for (count = 0; count < 128; count++) {

		animSet[count].setData(buffer[(count << 6) + 6],
			buffer[count << 6], buffer[(count << 6) + 1],
			buffer[(count << 6) + 3], buffer[(count << 6) + 4],
			buffer[(count << 6) + 2], buffer[(count << 6) + 5]);

		for (y = 0; y < buffer[(count << 6) + 6]; y++) {

			// Get frame
			x = buffer[(count << 6) + 7 + y];
			if (x > sprites) x = sprites;

			// Assign sprite and vertical offset
			animSet[count].setFrame(y, true);
			animSet[count].setFrameData(spriteSet + x,
				buffer[(count << 6) + 26 + y], buffer[(count << 6) + 45 + y]);

		}

	}

	delete[] buffer;


	// At general data

	// There's a a whole load of unknown data around here

	// Like another one of those pesky RLE blocks
	file->skipRLE();

	// And 217 bytes of DOOM
	file->seek(217, false);


	// Load sound map

	x = file->tell();

	for (count = 0; count < 32; count++) {

		file->seek(x + (count * 9), true);

		string = file->loadString();

		soundMap[count] = -1;

		// Search for matching sound

		for (y = 0; (y < nSounds) && (soundMap[count] == -1); y++) {

			if (!strcmp(string, sounds[y].name)) soundMap[count] = y;

		}

		delete[] string;

	}

	// Skip file names
	file->seek(x + 366, true);

	// Skip the players' initial coordinates
	file->seek(4, false);

	// Skip next level numbers
	file->seek(2, false);


	// Thanks to Doubble Dutch for the water level bytes
	file->seek(4, false);
	waterLevelTarget = ITOF(file->loadShort());
	waterLevel = waterLevelTarget - F8;
	waterLevelSpeed = 0;


	// Thanks to Feline and the JCS94 team for the next bits:

	file->seek(3, false);

	// Now at "Section 15"


	// Load player's animation set references

	buffer = file->loadRLE(PANIMS * 2);
	string = new char[PANIMS + 3];

	for (x = 0; x < PANIMS; x++) string[x + 3] = buffer[x << 1];

	delete[] buffer;

	if (gameMode) {

		string[0] = MTL_P_ANIMS;
		string[1] = MT_P_ANIMS;
		string[2] = 0;
		game->send((unsigned char *)string);

	}

	// Set the players' initial values
	if (game) {

		if (!checkpoint) game->setCheckpoint(startX, startY);

		for (count = 0; count < nPlayers; count++) game->resetPlayer(players + count, LT_JJ2LEVEL, string + 3);

	} else {

		localPlayer->reset(LT_JJ2LEVEL, string + 3, startX, startY);

	}

	delete[] string;


	// And that's us done!

	delete file;


	// Set the tick at which the level will end, though this is not used
	endTime = (5 - difficulty) * 2 * 60 * 1000;


	// Adjust panel fonts to use bonus level palette
	panelBigFont->mapPalette(0, 32, 64, 8);
	panelSmallFont->mapPalette(75, 5, 64, 8);


	return E_NONE;

}

