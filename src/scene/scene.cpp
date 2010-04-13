
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

SceneFrame::SceneFrame(int frameType, unsigned char* frameData, int frameSize) {
	soundId = -1;
	this->frameData = frameData;
	this->frameType = frameType;
	this->frameSize = frameSize;		
	prev = NULL;
	next = NULL;
	}

SceneFrame::~SceneFrame() {
	delete [] frameData;
	if (next) delete next;
}

void SceneAnimation::addFrame(int frameType, unsigned char* frameData, int frameSize) {
	SceneFrame* frame = new SceneFrame(frameType, frameData, frameSize);
	if(sceneFrames == NULL) {
		sceneFrames = frame;
	}
	else {
		frame->prev = lastFrame;
		lastFrame->next = frame;
	}
	
	lastFrame = frame;
	frames++;
}

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

SceneAnimation::SceneAnimation  (SceneAnimation* newNext)
	{
	next = newNext;
	background = NULL;
	lastFrame = NULL;
	sceneFrames = NULL;
	frames = 0;
	reverseAnimation = 0;
	}

SceneAnimation::~SceneAnimation ()
	{
	if (next) delete next;
	
	if(sceneFrames) delete sceneFrames;
	
	if (background) SDL_FreeSurface(background);
	}

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
	animIndex = -1; // no anim
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
	
	images = NULL;
	palettes = NULL;
	animations = NULL;
	
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

	if (images) delete images;
	if (palettes) delete palettes;

}

int Scene::play () {

	SDL_Rect dst;
	unsigned int sceneIndex = 0;
	SceneImage *image;
	SceneAnimation* animation;
	SceneFrame* currentFrame = NULL;
	SceneFrame* lastFrame = NULL;
	int	frameDelay = 0;
	int prevFrame = 0;
	int continueToNextPage = 0;
	
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
			 downOrRight || continueToNextPage ||
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
			continueToNextPage = 0;
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

		} else if (pages[sceneIndex].animIndex != -1) {

			if(currentFrame == NULL) {
					animation = animations;
			
					while (animation && (animation->id != pages[sceneIndex].animIndex))
						animation = animation->next;
			
					if (animation && animation->background) {
			
					dst.x = (canvasW - SW) >> 1;
					dst.y = (canvasH - SH) >> 1;
					frameDelay = 1000/(pages[sceneIndex].animSpeed>>8);
					SDL_BlitSurface(animation->background, NULL, canvas, &dst);
					currentFrame = animation->sceneFrames;
					SDL_Delay(frameDelay);
					}
				}
				else {
					// Upload pixel data to the surface
					if (SDL_MUSTLOCK(animation->background)) SDL_LockSurface(animation->background);
												
					switch(currentFrame->frameType)
						{						
						case ESquareAniHeader:
							{
							loadCompactedMem(currentFrame->frameSize, currentFrame->frameData,(unsigned char*) animation->background->pixels, SW, SH);
							}
							break;
						default:
							LOG("Scene::Play unknown type", currentFrame->frameType);
							break;
						}
					
					if (SDL_MUSTLOCK(animation->background)) SDL_UnlockSurface(animation->background);
					
					dst.x = (canvasW - SW) >> 1;
					dst.y = (canvasH - SH) >> 1;
					SDL_BlitSurface(animation->background, NULL, canvas, &dst);
					
					if(currentFrame->soundId != -1 && animation->noSounds > 0) {
							LOG("PLAY SOUND NAME",animation->soundNames[currentFrame->soundId-1]);
							// Search for matching sound
							for (int y = 0; y < nSounds ; y++) {
								if (!strcmp(animation->soundNames[currentFrame->soundId-1], sounds[y].name)) {
									playSound(y);
									break;
								}
							}
						}
					
					lastFrame = currentFrame;
					if(prevFrame) {
						currentFrame = currentFrame->prev;
						}
					else {
						currentFrame = currentFrame->next;
					}
					SDL_Delay(frameDelay);
					if(currentFrame == NULL && animation->reverseAnimation) {
						prevFrame = 1-prevFrame;
						if(prevFrame) {
							currentFrame = lastFrame->prev;
							}
						else {
							currentFrame = lastFrame->next;
							}
						}
					else if(currentFrame == NULL && !pageTime && !pages[sceneIndex].askForYesNo && pages[sceneIndex].nextPageAfterAnim) {
						continueToNextPage = 1;
					}
				}											

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


