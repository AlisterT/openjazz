
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

ImageInfo::ImageInfo () {

	image = NULL;

}

ImageInfo::~ImageInfo () {

	if (image != NULL) SDL_FreeSurface(image);

}

ScriptText::ScriptText() {

		x = -1;
		y = -1;
		textRect.x = -1;
		textRect.y = -1;
		extraLineHeight = -1;
		text = NULL;

}

ScriptText::~ScriptText() {

	if (text) delete[] text;

}

ScriptPage::ScriptPage() {

	pageTime = 0;
	noScriptTexts = 0;
	backgrounds = 0;
	musicfile = NULL;
	paletteIndex = 0;

}

ScriptPage::~ScriptPage() {

	if (musicfile) delete[] musicfile;

}


Scene::Scene (const char * fileName) {

	File *file;
    int loop;

    noScriptFonts = 0;
    LOG("\nScene", fileName);

	try {

		file = new File(fileName, false);

	} catch (int e) {

		throw e;

	}

	imageIndex = 0;
	paletteIndex = 0;

	file->seek(0x13, true); // Skip Digital Dimensions header
	signed long int dataOffset = file->loadInt(); //get offset pointer to first data block

	scriptItems = file->loadShort(); // Get number of script items
	scriptStarts = new signed long int[scriptItems];
	scriptPages = new ScriptPage[scriptItems];

	LOG("Scene: Script items", scriptItems);

	for (loop = 0; loop < scriptItems; loop++) {

		scriptStarts[loop] = file->loadInt();// Load offset to script
		LOG("scriptStart:", scriptStarts[loop]);

	}

	// Seek to datastart now
	file->seek(dataOffset, true); // Seek to data offsets
	dataItems = file->loadShort() + 1; // Get number of data items
	LOG("Scene: Data items", dataItems);
	dataOffsets = new signed long int[dataItems];

	for (loop = 0; loop < dataItems; loop++) {

		dataOffsets[loop] = file->loadInt();// Load offset to script
		LOG("dataOffsets:", dataOffsets[loop]);

	}

	loadData(file);
	loadScripts(file);

	delete[] scriptStarts;
	delete[] dataOffsets;
	delete file;

	return;

}


Scene::~Scene () {

	delete[] scriptPages;

}

ImageInfo * Scene::FindImage (int dataIndex) {

	int loop;

	for (loop = 0; loop < imageIndex; loop++) {

		if (imageInfos[loop].dataIndex == dataIndex) return imageInfos + loop;

	}

	return NULL;

}

int Scene::play () {

	SDL_Rect dst;
	unsigned int sceneIndex = 0;
	ImageInfo* imageInfo;
	unsigned int pageTime = scriptPages[sceneIndex].pageTime;
	unsigned int lastTicks = globalTicks;
	int newpage = true;
	int fadein = false;
	SDL_Rect textRect = {0,0,320,200};

	while (true) {

		if (loop(NORMAL_LOOP) == E_QUIT) return E_QUIT;

		if (controls.release(C_ESCAPE)) return E_NONE;

		SDL_Delay(T_FRAME);

		int upOrLeft = (controls.release(C_UP) || controls.release(C_LEFT));

		if ((sceneIndex > 0 && upOrLeft) ||
			controls.release(C_RIGHT) || controls.release(C_DOWN) || controls.release(C_ENTER) ||
			((globalTicks-lastTicks) >= pageTime * 1000 && pageTime != 256 && pageTime != 0)) {

			if (upOrLeft) sceneIndex--;
			else sceneIndex++;

			if (sceneIndex == scriptItems) return E_NONE;

			lastTicks = globalTicks;
			// Get bg for this page
			newpage = true;

			pageTime = scriptPages[sceneIndex].pageTime;

		}

		if (newpage) {

			//firstPE = new FadeOutPaletteEffect(250, firstPE);

			textRect.x = 0;
			textRect.y = 0;
			textRect.w = 320;
			textRect.h = 200;
			PaletteInfo* paletteInfo = NULL;

			for (int palette = 0; palette < paletteIndex; palette++) {

				if (paletteInfos[palette].dataIndex == scriptPages[sceneIndex].paletteIndex) {

					paletteInfo = &paletteInfos[palette];

					break;

				}

			}

			if (paletteInfo != NULL) {

				// usePalette(paletteInfo->palette);
				currentPalette = paletteInfo->palette;
				fadein = true;

			} else restorePalette(screen);

			newpage = 0;

		}

		// First draw the backgrounds associated with this page
		if (scriptPages[sceneIndex].backgrounds > 0) {

			for (int bg = 0; bg < scriptPages[sceneIndex].backgrounds; bg++) {

				imageInfo = FindImage(scriptPages[sceneIndex].bgIndex[bg]);

				if (imageInfo != NULL) {

					dst.x = (scriptPages[sceneIndex].bgPos[bg] & 65535)*2 + (canvasW - 320) >> 1;
					dst.y = ((scriptPages[sceneIndex].bgPos[bg] & (~65535))>>16)*2 + (canvasH - 200) >> 1;
					SDL_BlitSurface(imageInfo->image, NULL, canvas, &dst);

				}

			}

		} else clearScreen(0);


		// Draw the texts associated with this page
		int x = 0;
		int y = 0;
		int extraLineHeight = 0;

		for (int text = 0; text < scriptPages[sceneIndex].noScriptTexts; text++) {

			Font *font = NULL;
			int xOffset, yOffset;

			for (int index = 0; index < noScriptFonts; index++) {

				if (scriptPages[sceneIndex].scriptTexts[text].fontId == scriptFonts[index].fontId) {

					font = scriptFonts[index].font;

					continue;

				}

			}

			if (scriptPages[sceneIndex].scriptTexts[text].x != -1) {

				x = scriptPages[sceneIndex].scriptTexts[text].x;
				y = scriptPages[sceneIndex].scriptTexts[text].y;

			}

			if (scriptPages[sceneIndex].scriptTexts[text].textRect.x != -1) {

				textRect = scriptPages[sceneIndex].scriptTexts[text].textRect;
				x = 0;
				y = 0;

			}

			if (scriptPages[sceneIndex].scriptTexts[text].extraLineHeight != -1) {

				extraLineHeight = scriptPages[sceneIndex].scriptTexts[text].extraLineHeight;

			}

			xOffset = ((canvasW - 320) >> 1) + textRect.x + x;
			yOffset = ((canvasH - 200) >> 1) + textRect.y + y;

			switch (scriptPages[sceneIndex].scriptTexts[text].alignment) {

				case 0: // left

					break;

				case 1: // right

					xOffset += textRect.w - font->getSceneStringWidth(scriptPages[sceneIndex].scriptTexts[text].text);

					break;

				case 2: // center

					xOffset += (textRect.w - font->getSceneStringWidth(scriptPages[sceneIndex].scriptTexts[text].text)) >> 1;

					break;

			}

			// Drop shadow
			font->mapPalette(0, 256, 0, 1);
			font->showSceneString(scriptPages[sceneIndex].scriptTexts[text].text, xOffset + 1, yOffset + 1);
			font->restorePalette();

			// Text itself
			font->showSceneString(scriptPages[sceneIndex].scriptTexts[text].text, xOffset, yOffset);

			y += extraLineHeight + font->getHeight() / 2;

		}


		// Fade in from black
		if (fadein) {

			fadein = false;
			firstPE = new FadeInPaletteEffect(250, firstPE);
			clearScreen(0);

		}

	}

	return E_NONE;

}


