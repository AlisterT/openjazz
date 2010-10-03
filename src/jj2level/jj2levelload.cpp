
/**
 *
 * @file jj2levelload.cpp
 *
 * Part of the OpenJazz project
 *
 * @section History
 * 28th June 2010: Created levelloadjj2.cpp from parts of levelload.cpp
 * 29th June 2010: Renamed levelloadjj2.cpp to jj2levelload.cpp
 *
 * @section Licence
 * Copyright (c) 2005-2010 Alister Thomson
 *
 * OpenJazz is distributed under the terms of
 * the GNU General Public License, version 2.0
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 * @section Description
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


void JJ2Level::loadSprite (unsigned char* parameters, unsigned char* compressedPixels, Sprite* sprite, Sprite* flippedSprite) {

	unsigned char* pixels;
	int width, height;
	int srcPos, dstPos, rle;
	int x, y;

	// Load dimensions
	width = createShort(parameters);
	height = createShort(parameters + 2);

	if ((width == 0) || (height == 0)) {

		sprite->clearPixels();

		return;

	}


	// Decompress pixels

	pixels = new unsigned char[width * height];
	memset(pixels, 0, width * height);

	srcPos = createInt(parameters + 16);
	dstPos = 0;

	while (dstPos < width * height) {

		rle = compressedPixels[srcPos];
		srcPos++;

		if (rle > 128) {

			rle &= 127;

			if (dstPos + rle < width * height)
				memcpy(pixels + dstPos, compressedPixels + srcPos, rle);

			srcPos += rle;
			dstPos += rle;

		} else if (rle == 128) {

			rle = dstPos % width;

			if (rle) dstPos = dstPos + width - rle;

		} else {

			dstPos += rle;

		}

	}


	// Set sprite data
	sprite->xOffset = createShort(parameters + 8);
	sprite->yOffset = createShort(parameters + 10);
	sprite->setPixels(pixels, width, height, 0);

	// Flip sprite
	for (y = 0; y < height; y++) {

		for (x = 0; x < (width >> 1); x++) {

			rle = pixels[(y * width) + x];
			pixels[(y * width) + x] = pixels[(y * width) + (width - 1) - x];
			pixels[(y * width) + (width - 1) - x] = rle;

		}

	}

	// Set flipped sprite data
	flippedSprite->xOffset = -createShort(parameters + 8) - width;
	flippedSprite->yOffset = createShort(parameters + 10);
	flippedSprite->setPixels(pixels, width, height, 0);

	delete[] pixels;

	return;

}


int JJ2Level::loadSprites () {

	File* file;
	unsigned char* aBuffer;
	unsigned char* bBuffer;
	unsigned char* cBuffer;
	int* setOffsets;
	int aCLength, bCLength, cCLength, dCLength;
	int aLength, bLength, cLength, dLength;
	int setAnims, sprites, animSprites;
	int set, anim, sprite, setSprite;

	// Thanks to Neobeo for working out the .j2a format


	try {

		file = new File(F_ANIMS_J2A, false);

	} catch (int e) {

		return e;

	}

	file->seek(24, true);

	nAnimSets = file->loadInt();

	setOffsets = new int[nAnimSets];

	for (set = 0; set < nAnimSets; set++) setOffsets[set] = file->loadInt();


	// Count number of sprites

	sprites = 0;

	for (set = 0; set < nAnimSets; set++) {

		file->seek(setOffsets[set] + 6, true);

		sprites += file->loadShort();

	}

	spriteSet = new Sprite[sprites];
	flippedSpriteSet = new Sprite[sprites];
	animSets = new Anim *[nAnimSets];
	flippedAnimSets = new Anim *[nAnimSets];


	// Load animations and sprites

	sprites = 0;

	for (set = 0; set < nAnimSets; set++) {

		file->seek(setOffsets[set] + 4, true);

		setAnims = file->loadChar();

		if (setAnims) {

			animSets[set] = new Anim[setAnims];
			flippedAnimSets[set] = new Anim[setAnims];

		} else {

			animSets[set] = NULL;
			flippedAnimSets[set] = NULL;

		}

		file->seek(7, false);

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

		setSprite = 0;

		for (anim = 0; anim < setAnims; anim++) {

			animSprites = createShort(aBuffer + (anim * 8));

			// Fonts are loaded separately
			if (animSprites == 224) animSprites = 1;

			animSets[set][anim].setData(animSprites, 0, 0, 0, 0, 0, 0);
			flippedAnimSets[set][anim].setData(animSprites, 0, 0, 0, 0, 0, 0);

			for (sprite = 0; sprite < animSprites; sprite++) {

				loadSprite(bBuffer + (setSprite * 24), cBuffer, spriteSet + sprites, flippedSpriteSet + sprites);

				animSets[set][anim].setFrame(sprite, false);
				animSets[set][anim].setFrameData(spriteSet + sprites, 0, 0);
				flippedAnimSets[set][anim].setFrame(sprite, false);
				flippedAnimSets[set][anim].setFrameData(flippedSpriteSet + sprites, 0, 0);

				setSprite++;
				sprites++;

			}

		}

		delete[] cBuffer;
		delete[] bBuffer;
		delete[] aBuffer;

	}


	delete[] setOffsets;

	delete file;


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
	for (count = 0; count < TTOI(tiles); count++) {

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


void JJ2Level::createEvent (int x, int y, unsigned char* data) {

	unsigned char type;
	int properties;

	type = *data;
	properties = (data[1] >> 4) + (data[2] << 4) + (data[3] << 12);

	if ((type < 33) || ((type >= 206) && (type <= 208)) || (type == 230) || (type == 240) || (type == 245)) {

		mods[y][x].type = type;
		mods[y][x].properties = properties;

		return;

	}

	mods[y][x].type = 0;

	if (type < 41) {

		events = new AmmoJJ2Event(events, x, y, type);

	} else if (type < 44) {

		events = new OtherJJ2Event(events, x, y, type, properties);

	} else if (type < 46) {

		events = new CoinGemJJ2Event(events, x, y, type);

	} else if (type < 63) {

		events = new OtherJJ2Event(events, x, y, type, properties);

	} else if (type < 67) {

		events = new CoinGemJJ2Event(events, x, y, type);

	} else if (type < 72) {

		events = new OtherJJ2Event(events, x, y, type, properties);

	} else if (type < 74) {

		events = new FoodJJ2Event(events, x, y, type);

	} else if (type == 80) {

		events = new FoodJJ2Event(events, x, y, type);

	} else if (type < 141) {

		events = new OtherJJ2Event(events, x, y, type, properties);

	} else if (type < 148) {

		events = new FoodJJ2Event(events, x, y, type);

	} else if (type < 154) {

		events = new OtherJJ2Event(events, x, y, type, properties);

	} else if (type < 183) {

		events = new FoodJJ2Event(events, x, y, type);

	} else {

		events = new OtherJJ2Event(events, x, y, type, properties);

	}

	return;

}


int JJ2Level::load (char *fileName, unsigned char diff, bool checkpoint) {

	Anim* pAnims[2];
	File *file;
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

	video.clearScreen(0);

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

		if (pitch & 3) pitch += 4;

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

			// Create event or assign modifier
			createEvent(x, y, bBuffer + (((y * width) + x) << 2));

			if (mods[y][x].type == 29) {

				// Jazz start pos

				startX = x;
				startY = y;

			}

		}

	}

	delete[] dBuffer;
	delete[] cBuffer;
	delete[] bBuffer;
	delete[] aBuffer;


	// Load anims from anims.j2a

	if (loadSprites() < 0) {

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


	// Set initial water level
	waterLevelTarget = TTOF(layer->getHeight() + 1);
	waterLevel = waterLevelTarget - F8;
	waterLevelSpeed = 0;


	// Generate player's animation set references

	if (multiplayer) {

		string = new char[MTL_P_ANIMS + 1];

		string[0] = MTL_P_ANIMS + 1;
		string[1] = MT_P_ANIMS;
		string[2] = 0;
		string[3] = 54;

		game->send((unsigned char *)string);

		delete[] string;

	}

	// Set the players' initial values

	pAnims[0] = animSets[54];
	pAnims[1] = flippedAnimSets[54];

	if (game) {

		if (!checkpoint) game->setCheckpoint(startX, startY);

		for (count = 0; count < nPlayers; count++) game->resetPlayer(players + count, LT_JJ2LEVEL, pAnims);

	} else {

		localPlayer->reset(LT_JJ2LEVEL, pAnims, startX, startY);

	}


	// And that's us done!


	// Set the tick at which the level will end, though this is not used
	endTime = (5 - difficulty) * 2 * 60 * 1000;


	// Adjust panel fonts to use bonus level palette
	panelBigFont->mapPalette(0, 32, 64, 8);
	panelSmallFont->mapPalette(75, 5, 64, 8);


	return E_NONE;

}

