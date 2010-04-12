
/*
 *
 * scene.cpp
 *
 * 23rd August 2005: Created scene.c
 * 3rd February 2009: Created scene.h from parts of scene.c
 * 3rd February 2009: Renamed scene.c to scene.cpp
 * 27th March 2010: Created sceneload.cpp from parts of scene.cpp
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
 * Deals with the displaying and freeing of the cutscenes.
 *
 */


#include "scene/scene.h"

#include "io/controls.h"
#include "io/gfx/font.h"
#include "io/gfx/paletteeffects.h"
#include "io/gfx/video.h"
#include "io/sound.h"

/**
 *  This is the 0sc format
 *  Offset, Size (hex), type
 *  0, 0x13, "Digital Dimensions\1a" header
 *  0x13, 0x04, Offset to datablock
 *  0x17, 0x02, Number of script items
 *  0x19, 0x4* Number of script items, Offset to Scripts
 *
 *  ----, 0x02 Number of data items
 *  *  0x19, 0x4* Number of script items, Offset to datablocks
 *
 */

SceneImage::SceneImage (SceneImage *newNext) {

	next = newNext;
	image = NULL;

}


SceneImage::~SceneImage () {

	if (next) delete next;

	if (image) SDL_FreeSurface(image);

}


ScenePalette::ScenePalette (ScenePalette *newNext) {

	next = newNext;

}


ScenePalette::~ScenePalette () {

	if (next) delete next;

}


SceneText::SceneText() {

		x = -1;
		y = -1;
		textRect.x = -1;
		textRect.y = -1;
		extraLineHeight = -1;
		text = NULL;
		shadowColour = 0;
}

SceneText::~SceneText() {

	if (text) delete[] text;

}

ScenePage::ScenePage() {

	pageTime = 0;
	nTexts = 0;
	backgrounds = 0;
	musicFile = NULL;
	paletteIndex = 0;
	askForYesNo = 0;
	stopMusic = 0;
}

ScenePage::~ScenePage() {

	if (musicFile) delete[] musicFile;

}


Scene::Scene (const char * fileName) {

	File *file;
    int loop;

    nFonts = 0;
    LOG("\nScene", fileName);

	try {

		file = new File(fileName, false);

	} catch (int e) {

		throw e;

	}

	background = NULL;
	images = NULL;
	palettes = NULL;

	file->seek(ESignatureLength, true); // Skip Digital Dimensions header
	signed long int dataOffset = file->loadInt(); //get offset pointer to first data block

	scriptItems = file->loadShort(); // Get number of script items
	scriptStarts = new signed long int[scriptItems];
	pages = new ScenePage[scriptItems];

	LOG("Scene: Script items", scriptItems);

	for (loop = 0; loop < scriptItems; loop++) {

		scriptStarts[loop] = file->loadInt();// Load offset to script
		LOG("scriptStart", scriptStarts[loop]);

	}

	// Seek to datastart now
	file->seek(dataOffset, true); // Seek to data offsets
	dataItems = file->loadShort() + 1; // Get number of data items
	LOG("Scene: Data items", dataItems);
	dataOffsets = new signed long int[dataItems];

	for (loop = 0; loop < dataItems; loop++) {

		dataOffsets[loop] = file->loadInt();// Load offset to script
		LOG("dataOffsets", dataOffsets[loop]);

	}

	loadData(file);
	loadScripts(file);

	delete[] scriptStarts;
	delete[] dataOffsets;
	delete file;

	return;

}


Scene::~Scene () {

	delete[] pages;

	if (background) SDL_FreeSurface(background);

	if (images) delete images;
	if (palettes) delete palettes;

}


int Scene::play () {

	SDL_Rect dst;
	unsigned int sceneIndex = 0;
	SceneImage *image;
	unsigned int pageTime = pages[sceneIndex].pageTime;
	unsigned int lastTicks = globalTicks;
	int newpage = true;
	SDL_Rect textRect = {0, 0, SW, SH};

	clearScreen(0);

	while (true) {

		if (loop(NORMAL_LOOP) == E_QUIT) return E_QUIT;

		if (controls.release(C_ESCAPE) || (controls.release(C_NO) && pages[sceneIndex].askForYesNo)) return E_NONE;

		SDL_Delay(T_FRAME);

		int upOrLeft = 0;
		int downOrRight = 0;
		
		if(pages[sceneIndex].askForYesNo) {
			// Should check for Y also
			downOrRight = controls.release(C_ENTER) || controls.release(C_YES);;
		} else {
			upOrLeft = (controls.release(C_UP) || controls.release(C_LEFT));
			downOrRight = (controls.release(C_RIGHT) || controls.release(C_DOWN) || controls.release(C_ENTER));			
		}
		
		if ((sceneIndex > 0 && upOrLeft) ||
			 downOrRight ||
			((globalTicks-lastTicks) >= pageTime * 1000 && pageTime != 256 && pageTime != 0)) {

			if(pages[sceneIndex].stopMusic) {
				stopMusic();
			}
			
			if (upOrLeft) sceneIndex--;
			else sceneIndex++;

			if (sceneIndex == scriptItems) return E_NONE;

			lastTicks = globalTicks;
			// Get bg for this page
			newpage = true;

			pageTime = pages[sceneIndex].pageTime;

		}

		if (newpage) {

			//paletteEffects = new FadeOutPaletteEffect(250, paletteEffects);

			textRect.x = 0;
			textRect.y = 0;
			textRect.w = SW;
			textRect.h = SH;
			ScenePalette *palette = palettes;

			while (palette && (palette->id != pages[sceneIndex].paletteIndex)) palette = palette->next;

			if (palette) {

				video.setPalette(palette->palette);

				// Fade in from black
				paletteEffects = new FadeInPaletteEffect(250, paletteEffects);

			}

			if(pages[sceneIndex].musicFile) {				
				playMusic(pages[sceneIndex].musicFile);
			}						
			
			newpage = 0;

		}

		// First draw the backgrounds associated with this page
		if (pages[sceneIndex].backgrounds > 0) {

			for (int bg = 0; bg < pages[sceneIndex].backgrounds; bg++) {

				image = images;

				while (image && (image->id != pages[sceneIndex].bgIndex[bg]))
					image = image->next;

				if (image) {

					dst.x = pages[sceneIndex].bgX[bg] + ((canvasW - SW) >> 1);
					dst.y = pages[sceneIndex].bgY[bg] + ((canvasH - SH) >> 1);
					SDL_BlitSurface(image->image, NULL, canvas, &dst);

				}

			}

		} else if (background) {

			dst.x = (canvasW - SW) >> 1;
			dst.y = (canvasH - SH) >> 1;
			SDL_BlitSurface(background, NULL, canvas, &dst);

		} else clearScreen(0);


		// Draw the texts associated with this page
		int x = 0;
		int y = 0;
		int extraLineHeight = 0;

		for (int count = 0; count < pages[sceneIndex].nTexts; count++) {

			SceneText *text = pages[sceneIndex].texts + count;
			Font *font = NULL;
			int xOffset, yOffset;

			for (int index = 0; index < nFonts; index++) {

				if (text->fontId == fonts[index].id) {

					font = fonts[index].font;

					continue;

				}

			}

			if (text->x != -1) {

				x = text->x;
				y = text->y;

			}

			if (text->textRect.x != -1) {

				textRect = text->textRect;
				x = 0;
				y = 0;

			}

			if (text->extraLineHeight != -1) {

				extraLineHeight = text->extraLineHeight;

			}

			xOffset = ((canvasW - SW) >> 1) + textRect.x + x;
			yOffset = ((canvasH - SH) >> 1) + textRect.y + y;

			switch (text->alignment) {

				case 0: // left

					break;

				case 1: // right

					xOffset += textRect.w - font->getSceneStringWidth(text->text);

					break;

				case 2: // center

					xOffset += (textRect.w - font->getSceneStringWidth(text->text)) >> 1;

					break;

			}

			// Drop shadow
			font->mapPalette(0, 256, 0, 1);
			font->showSceneString(text->text, xOffset + 1, yOffset + 1);
			font->restorePalette();

			// Text itself
			font->showSceneString(text->text, xOffset, yOffset);

			y += extraLineHeight + font->getHeight() / 2;

		}

	}

	return E_NONE;

}


