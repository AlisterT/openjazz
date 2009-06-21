
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


#include "font.h"
#include "game.h"
#include "level.h"
#include "palette.h"
#include "sound.h"
#include <string.h>


int Level::loadSprites (char * fileName) {

	File *file, *mainFile, *specFile;
	unsigned char *pixels, *sorted;
	int mainPos, specPos;
	int count, x, y, width, height, m;

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

	// Find where the sprites start in fileName
	specPos = specFile->tell();

	// Find where the sprites start in mainchar.000
	mainPos = 2;

	// Loop through all the sprites to be loaded
	for (count = 0; count < sprites; count++) {

		// Go to the start of the current sprite or file indicator
		specFile->seek(specPos, true);
		mainFile->seek(mainPos, true);

		// If both fileName and mainchar.000 have file indicators, create a blank
		// sprite
		while ((specFile->loadChar() == 0xFF) &&
			(mainFile->loadChar() == 0xFF)) {

			// Go to the next sprite/file indicator
			specFile->seek(1, false);
			mainFile->seek(1, false);

			// set the position of the next sprite/file indicators
			specPos += 2;
			mainPos += 2;

			// Create a blank sprite
			spriteSet[count].clearPixels();
			count++;

		}

		// Return to the start of the sprite/file indicators
		specFile->seek(specPos, true);
		mainFile->seek(mainPos, true);

		// Unless otherwise stated, load from fileName
		file = specFile;

		// Check if otherwise stated
		if (file->loadChar() == 0xFF) {

			file = mainFile;

		} else file->seek(-1, false);

		width = file->loadShort() << 2;
		height = file->loadShort();

		// Position of the next sprite or file indicator in each file
		if (file == specFile) {

			mainPos += 2;

			specPos += 10 + (file->loadShort() << 2);

		} else {

			specPos += 2;

			mainPos += 10 + (file->loadShort() << 2);

		}

		// m is for MAGIC
		m = file->loadShort();

		// Allocate space for descrambling
		sorted = new unsigned char[width * height];

		// Actually, m is for mask offset.
		// Sprites can be either masked or not masked.
		if (!m) {

			// Not masked
			// Load the pixel data directly for descrambling

			file->seek(2, false);

			// Read pixel data
			pixels = file->loadBlock(width * height);


		} else {

			// Allocate space for pixel data
			pixels = new unsigned char[width * height];

			// Masked
			// Load the pixel data according to the mask

			// Masked sprites have their own next sprite offsets
			if (file == specFile) {

				specPos = file->loadShort() << 2;

			} else {

				mainPos = file->loadShort() << 2;

			}

			// Skip to mask
			file->seek(m, false);

			// Read the mask
			// Each mask pixel is either 0 or 1
			// Four pixels are packed into the lower end of each byte
			for (y = 0; y < height; y++) {

				for (x = 0; x < width; x++) {

					if (!(x & 3)) m = file->loadChar();
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
			file->seek(width >> 2, false);

			// Next sprite offsets are relative to here
			if (file == specFile) specPos += file->tell();
			else mainPos += file->tell();

			// Read pixels according to the scrambled mask
			for (y = 0; y < height; y++) {

				for (x = 0; x < width; x++) {

					if (sorted[(y * width) + x] == 1) {

						// The unmasked portions are transparent, so no masked
						// portion should be transparent.
						m = SKEY;

						while (m == SKEY) m = file->loadChar();

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
		spriteSet[count].setPixels(sorted, width, height);

		// Free redundant data
		delete[] pixels;


		// Check if the next sprite exists
		// If not, create blank sprites for the remainder
		if (specPos >= file->getSize()) {

			for (count++; count < sprites; count++) {

				spriteSet[count].clearPixels();

			}

		} else {

			specFile->seek(specPos, true);

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


int Level::loadTiles (char * fileName) {

	File *file;
	unsigned char *buffer;
	int rle, pos, index, count, fileSize;
	int tiles;


	try {

		file = new File(fileName, false);

	} catch (int e) {

		return e;

	}


	// Load the palette
	file->loadPalette(palette);


	// Load the background palette
	file->loadPalette(skyPalette);


	// Skip the second, identical, background palette
	file->skipRLE();


	// Load the tile pixel indices

	tiles = 240; // Never more than 240 tiles

	buffer = new unsigned char[tiles * TW * TH];

	file->seek(4, false);

	pos = 0;
	fileSize = file->getSize();

	// Read the RLE pixels
	// file::loadRLE() cannot be used, for reasons that will become clear
	while ((pos < TW * TH * tiles) && (file->tell() < fileSize)) {

		rle = file->loadChar();

		if (rle & 128) {

			index = file->loadChar();

			for (count = 0; count < (rle & 127); count++) buffer[pos++] = index;

		} else if (rle) {

			for (count = 0; count < rle; count++)
				buffer[pos++] = file->loadChar();

		} else { // This happens at the end of each tile

			// 0 pixels means 1 pixel, apparently
			buffer[pos++] = file->loadChar();

			file->seek(2, false); /* I assume this is the length of the next
				tile block */

			if (pos == TW * TH * 60) file->seek(2, false);
			if (pos == TW * TH * 120) file->seek(2, false);
			if (pos == TW * TH * 180) file->seek(2, false);

		}

	}

	delete file;

	// Work out how many tiles were actually loaded
	// Should be a multiple of 60
	tiles = pos / (TW * TH);

	tileSet = createSurface(buffer, TW, TH * tiles);
	SDL_SetColorKey(tileSet, SDL_SRCCOLORKEY, TKEY);

	return tiles;

}


int Level::load (char *fileName, unsigned char diff, bool checkpoint) {

	File *file;
	unsigned char *buffer;
	char *string, *ext;
	int tiles;
	int count, x, y, type;


	difficulty = diff;


	// Show loading screen

	// Open planet.### file

	if (!strcmp(fileName, LEVEL_FILE)) {

		// Using the downloaded level file

		string = cloneString("DOWNLOADED");

	} else {

		// Load the planet's name from the planet.### file

		string = new char[11];
		sprintf(string, F_PLANET, fileName + strlen(fileName) - 3);

		try {

			file = new File(string, false);

		} catch (int e) {

			delete[] string;

			return e;

		}

		delete[] string;

		file->seek(2, true);
		string = file->loadString();

		delete file;

	}

	switch (fileName[5]) {

		case '0':

			ext = " LEVEL ONE";

			break;

		case '1':

			ext = " LEVEL TWO";

			break;

		case '2':

			string[0] = 0;
			ext = "SECRET LEVEL";

			break;

		default:

			ext = " LEVEL";

			break;

	}

	clearScreen(0);

	x = (screenW >> 1) - ((strlen(string) + strlen(ext)) << 2);
	x = fontmn2->showString("LOADING ", x - 60, (screenH >> 1) - 16);
	x = fontmn2->showString(string, x, (screenH >> 1) - 16);
	fontmn2->showString(ext, x, (screenH >> 1) - 16);

	delete[] string;

	if (loop(NORMAL_LOOP) == E_QUIT) return E_QUIT;



	// Open level file

	try {

		file = new File(fileName, false);

	} catch (int e) {

		return e;

	}


	// Load level data from a Level#.### file

	// Load the blocks.### extension

	// Skip past all level data
	file->seek(39, true);
	file->skipRLE();
	file->skipRLE();
	file->skipRLE();
	file->skipRLE();
	file->skipRLE();
	file->skipRLE();
	file->skipRLE();
	file->skipRLE();
	file->seek(598, false);
	file->skipRLE();
	file->seek(4, false);
	file->skipRLE();
	file->skipRLE();
	file->seek(25, false);
	file->skipRLE();
	file->seek(3, false);

	// Load the level number
	levelNum = file->loadChar() ^ 210;

	// Load the world number
	worldNum = file->loadChar() ^ 4;


	// Load tile set from appropriate blocks.###

	// Load tile set extension
	file->seek(8, false);
	ext = file->loadString();

	if (!strcmp(ext, "999")) {

		// Use the level file's extension instead
		delete[] ext;
		ext = cloneString(fileName + strlen(fileName) - 3);

	}

	// Allocate space for file names
	string = new char[12];

	sprintf(string, F_BLOCKS, ext);

	delete[] ext;

	tiles = loadTiles(string);

	if (tiles < 0) {

		delete[] string;
		delete file;

		return tiles;

	}


	// Load sprite set from corresponding Sprites.###

	sprintf(string, F_SPRITES, worldNum);

	count = loadSprites(string);

	if (count < 0) {

		SDL_FreeSurface(tileSet);
		delete[] string;
		delete file;

		return count;

	}

	delete[] string;


	// Skip to tile and event reference data
	file->seek(39, true);

	// Load tile and event references

	buffer = file->loadRLE(LW * LH * 2);

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
	file->skipRLE();


	// Load mask data

	buffer = file->loadRLE(tiles * 8);

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

				if (mask[count][((y >> 2) << 3) + (x >> 2)] == 1)
					((char *)(tileSet->pixels))
						[(count * TW * TH) + (y * TW) + x] = 88;

			}

		}

	}

	if (SDL_MUSTLOCK(tileSet)) SDL_UnlockSurface(tileSet);*/


	// Load special event path

	buffer = file->loadRLE(8192);
	pathLength = buffer[0] + (buffer[1] << 8);
	pathNode = 0;
	if (pathLength < 1) pathLength = 1;
	pathX = new int[pathLength];
	pathY = new int[pathLength];

	for (count = 0; count < pathLength; count++) {

		pathX[count] = ((signed char *)buffer)[(count << 1) + 3] << 2;
		pathY[count] = ((signed char *)buffer)[(count << 1) + 2];

	}

	delete[] buffer;


	// Load event set

	buffer = file->loadRLE(EVENTS * ELENGTH);

	// Fill event set with data
	for (count = 0; count < EVENTS; count++) {

		memcpy(eventSet[count], buffer + (count * ELENGTH), ELENGTH);
		eventSet[count][E_MOVEMENTSP]++;

	}

	delete[] buffer;


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
	file->skipRLE();


	// Load animation set

	buffer = file->loadRLE(ANIMS * 64);

	// Create animation set based on that data
	for (count = 0; count < ANIMS; count++) {

		animSet[count].setData(buffer[(count * 64) + 6],
			buffer[(count * 64) + 4], buffer[(count * 64) + 5]);

		for (y = 0; y < buffer[(count * 64) + 6]; y++) {

			// Get frame
			x = buffer[(count * 64) + 7 + y];
			if (x > sprites) x = sprites;

			// Assign sprite and vertical offset
			animSet[count].setFrame(y, true);
			animSet[count].setFrameData(spriteSet + x,
				buffer[(count * 64) + 26 + y], buffer[(count * 64) + 45 + y]);

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

	file->seek(x + 288, true);

	// Music file
	string = file->loadString();
	playMusic(string);

	// 25 bytes of undiscovered usefulness, less the music file name
	file->seek(25 - strlen(string), false);
	delete[] string;

	// End of episode cutscene
	sceneFile = file->loadString();

	// 51 bytes of undiscovered usefulness, less the cutscene file name
	file->seek(51 - strlen(sceneFile), false);

	// The players' coordinates
	if (!checkpoint) {

		checkX = file->loadShort();
		checkY = file->loadShort() + 1;

	} else file->seek(4, false);

	// Set the players' initial values
	for (count = 0; count < nPlayers; count++) {

		players[count].reset();
		players[count].setPosition(checkX << 15, checkY << 15);

	}

	// Next level
	x = file->loadChar();
	y = file->loadChar();
	setNext(x, y);


	// Thanks to Doubble Dutch for this next bit
	file->seek(4, false);
	waterLevel = file->loadShort() << 10;


	// Thanks to Feline and the JCS94 team for the next bits:

	file->seek(3, false);

	// Now at "Section 15"


	// Load player's animation set references

	buffer = file->loadRLE(PANIMS * 2);
	string = new char[PANIMS + 3];

	for (x = 0; x < PANIMS; x++) string[x + 3] = buffer[x << 1];

	for (x = 0; x < nPlayers; x++) players[x].setAnims(string + 3);

	if (gameMode != M_SINGLE) {

		string[0] = MTL_P_ANIMS;
		string[1] = MT_P_ANIMS;
		string[2] = 0;
		game->send((unsigned char *)string);

	}

	delete[] string;
	delete[] buffer;


	// Load Skip to bullet set
	miscAnims[0] = file->loadChar();
	miscAnims[1] = file->loadChar();
	miscAnims[2] = file->loadChar();
	miscAnims[3] = file->loadChar();


	// Load bullet set
	buffer = file->loadRLE(BULLETS * BLENGTH);

	for (count = 0; count < BULLETS; count++) {

		memcpy(bulletSet[count], buffer + (count * BLENGTH), BLENGTH);

		// Make sure bullets go in the right direction

		if (bulletSet[count][B_XSPEED] > 0)
			bulletSet[count][B_XSPEED] = -bulletSet[count][B_XSPEED];

		if (bulletSet[count][B_XSPEED | 1] < 0)
			bulletSet[count][B_XSPEED | 1] = -bulletSet[count][B_XSPEED | 1];

		if (bulletSet[count][B_XSPEED | 2] > 0)
			bulletSet[count][B_XSPEED | 2] = -bulletSet[count][B_XSPEED | 2];

		if (bulletSet[count][B_XSPEED | 3] < 0)
			bulletSet[count][B_XSPEED | 3] = -bulletSet[count][B_XSPEED | 3];

	}

	delete[] buffer;


	// Now at "Section 18." More skippability.
	file->skipRLE();


	// Now at "Section 19," THE MAGIC SECTION

	// First byte is the background palette effect type
	type = file->loadChar();

	sky = false;

	// Free any existing palette effects
	if (firstPE) delete firstPE;

	switch (type) {

		case 2:

			sky = true;

			// Sky background effect
			firstPE = new SkyPaletteEffect(156, 100, FH, skyPalette, NULL);

			break;

		case 8:

			// Parallaxing background effect
			firstPE = new P2DPaletteEffect(128, 64, FE, NULL);

			break;

		case 9:

			// Diagonal stripes "parallaxing" background effect
			firstPE = new P1DPaletteEffect(128, 32, FH, NULL);

			break;

		case 11:

			// The deeper below water, the darker it gets
			firstPE = new WaterPaletteEffect(F32 * 32, NULL);

			break;

		default:

			// No effect
			firstPE = NULL;

			break;

	}

	// Palette animations
	// These are applied to every level without a conflicting background effect
	// As a result, there are a few levels with things animated that shouldn't
	// be

	// In Diamondus: The red/yellow palette animation
	firstPE = new RotatePaletteEffect(112, 4, F32, firstPE);

	// In Diamondus: The waterfall palette animation
	firstPE = new RotatePaletteEffect(116, 8, F16, firstPE);

	// The following were discoverd by Unknown/Violet

	firstPE = new RotatePaletteEffect(124, 3, F16, firstPE);

	if ((type != PE_1D) && (type != PE_2D))
		firstPE = new RotatePaletteEffect(132, 8, F16, firstPE);

	if ((type != PE_SKY) && (type != PE_2D))
		firstPE = new RotatePaletteEffect(160, 32, -F16, firstPE);

	if (type != PE_SKY) {

		firstPE = new RotatePaletteEffect(192, 32, -F32, firstPE);
		firstPE = new RotatePaletteEffect(224, 16, F16, firstPE);

	}

	// Level fade-in/white-in effect
	if (checkpoint) firstPE = new FadeInPaletteEffect(T_START, firstPE);
	else firstPE = new WhiteInPaletteEffect(T_START, firstPE);


	file->seek(1, false);

	skyOrb = file->loadChar(); /* A.k.a the sun, the moon, the brightest star,
		that red planet with blue veins... */


	// And that's us done!

	delete file;


	// Apply the palette to surfaces that already exist, e.g. fonts
	usePalette(palette);

	// Adjust fontmn1 to use level palette
	fontmn1->mapPalette(224, 8, 14, -16);


	// Set the tick at which the level will end
	endTime = (5 - difficulty) * 2 * 60 * 1000;
	timeBonus = -1;


	firstBullet = NULL;
	firstEvent = NULL;

	energyBar = 0;

	return E_NONE;

}


Level::Level () {

	// Do nothing

	return;

}


Level::Level (char *fileName, unsigned char diff, bool checkpoint) {

	int ret;

	gameMode = game->getMode();

	// Load level data

	ret = load(fileName, diff, checkpoint);

	if (ret < 0) throw ret;

	return;

}


Level::~Level () {

	// Free all data

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

	delete[] spriteSet;

	fontmn1->restorePalette();

	delete[] sceneFile;

	return;

}


DemoLevel::DemoLevel (char *fileName) {

	File *file;
	char levelFile[11];
	int lNum, wNum, diff, ret;

	gameMode = M_SINGLE;

	try {

		file = new File(fileName, false);

	} catch (int e) {

		throw e;

	}

	// Check this is a normal level
	if (file->loadShort() == 0) throw E_DEMOTYPE;

	// Level file to load
	lNum = file->loadShort();
	wNum = file->loadShort();
	sprintf(levelFile, F_LEVEL, lNum, wNum);

	// Difficulty
	diff = file->loadShort();

	macro = file->loadBlock(1024);

	// Load level data

	ret = load(levelFile, diff, false);

	if (ret < 0) throw ret;

	return;

}


DemoLevel::~DemoLevel () {

	delete[] macro;

	return;

}


