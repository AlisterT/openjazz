
/**
 *
 * @file jj1scene.cpp
 *
 * Part of the OpenJazz project
 *
 * @section History
 * 23rd August 2005: Created scene.c
 * 3rd February 2009: Created scene.h from parts of scene.c
 * 3rd February 2009: Renamed scene.c to scene.cpp
 * 27th March 2010: Created sceneload.cpp from parts of scene.cpp
 * 1st August 2012: Renamed scene.cpp to jj1scene.cpp
 *
 * @section Licence
 * Copyright (c) 2005-2012 Alister Thomson
 *
 * OpenJazz is distributed under the terms of
 * the GNU General Public License, version 2.0
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 * @section Description
 * Deals with the displaying and freeing of the cutscenes.
 *
 */


#include "jj1scene.h"

#include "io/controls.h"
#include "io/gfx/font.h"
#include "io/gfx/paletteeffects.h"
#include "io/gfx/video.h"
#include "io/sound.h"
#include "loop.h"
#include "util.h"

#include <string.h>


/**
 * Create a JJ1 cutscne frame.
 *
 * @param frameType The type of the frame
 * @param frameData The frame's data buffer
 * @param frameSize The size of the frame's data buffer
 */
JJ1SceneFrame::JJ1SceneFrame(int frameType, unsigned char* frameData, int frameSize) {

	soundId = -1;
	this->frameData = frameData;
	this->frameType = frameType;
	this->frameSize = frameSize;
	prev = NULL;
	next = NULL;

}


/**
 * Delete the JJ1 cutscene frame.
 */
JJ1SceneFrame::~JJ1SceneFrame() {

	delete [] frameData;

}


/**
 * Add a frame to the JJ1 cutscene animation.
 *
 * @param frameType The type of the frame
 * @param frameData The frame's data buffer
 * @param frameSize The size of the frame's data buffer
 */
void JJ1SceneAnimation::addFrame(int frameType, unsigned char* frameData, int frameSize) {

	JJ1SceneFrame* frame = new JJ1SceneFrame(frameType, frameData, frameSize);

	if(sceneFrames == NULL) {

		sceneFrames = frame;

	} else {

		frame->prev = lastFrame;
		lastFrame->next = frame;

	}

	lastFrame = frame;
	frames++;

}


/**
 * Create a JJ1 cutscene animation.
 *
 * @param newNext The next animation
 */
JJ1SceneAnimation::JJ1SceneAnimation  (JJ1SceneAnimation* newNext) {

	next = newNext;
	background = NULL;
	lastFrame = NULL;
	sceneFrames = NULL;
	frames = 0;
	reverseAnimation = 0;

}


/**
 * Delete the JJ1 cutscene animation.
 */
JJ1SceneAnimation::~JJ1SceneAnimation () {

	if (next) delete next;

	if(sceneFrames) {
		JJ1SceneFrame* frame = sceneFrames;
		JJ1SceneFrame* nextFrame = NULL;
		while(frame)
			{
			nextFrame = frame->next;
			delete frame;
			frame = NULL;
			frame = nextFrame;
			}
		}

	if (background) SDL_FreeSurface(background);

}


/**
 * Create a JJ1 cutscene image.
 *
 * @param newNext The next image
 */
JJ1SceneImage::JJ1SceneImage (JJ1SceneImage *newNext) {

	next = newNext;
	image = NULL;

}


/**
 * Delete the JJ1 cutscene image.
 */
JJ1SceneImage::~JJ1SceneImage () {

	if (next) delete next;

	if (image) SDL_FreeSurface(image);

}


/**
 * Create a JJ1 cutscene palette.
 *
 * @param newNext The next palette
 */
JJ1ScenePalette::JJ1ScenePalette (JJ1ScenePalette *newNext) {

	next = newNext;

}


/**
 * Delete the JJ1 cutscene palette.
 */
JJ1ScenePalette::~JJ1ScenePalette () {

	if (next) delete next;

}


/**
 * Create a JJ1 cutscene text object.
 */
JJ1SceneText::JJ1SceneText() {

		x = -1;
		y = -1;
		textRect.x = -1;
		textRect.y = -1;
		extraLineHeight = -1;
		text = NULL;
		shadowColour = 0;

}


/**
 * Delete the JJ1 cutscene text object.
 */
JJ1SceneText::~JJ1SceneText() {

	if (text) delete[] text;

}


/**
 * Create a JJ1 cutscene page.
 */
JJ1ScenePage::JJ1ScenePage() {

	pageTime = 0;
	nTexts = 0;
	backgrounds = 0;
	musicFile = NULL;
	paletteIndex = 0;
	askForYesNo = 0;
	stopMusic = 0;
	animIndex = -1; // no anim

}


/**
 * Delete the JJ1 cutscene page.
 */
JJ1ScenePage::~JJ1ScenePage() {

	if (musicFile) delete[] musicFile;

}


/**
 * Create a JJ1 cutscene.
 *
 * @param fileName Name of the file containing the cutscene data
 */
JJ1Scene::JJ1Scene (const char * fileName) {

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

	file->seek(0x13, true); // Skip Digital Dimensions header
	signed long int dataOffset = file->loadInt(); //get offset pointer to first data block

	scriptItems = file->loadShort(); // Get number of script items
	scriptStarts = new signed long int[scriptItems];
	pages = new JJ1ScenePage[scriptItems];

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


/**
 * Delete the JJ1 cutscene.
 */
JJ1Scene::~JJ1Scene () {

	delete[] pages;

	if (images) delete images;
	if (palettes) delete palettes;
	if (animations) delete animations;

}


/**
 * Play the JJ1 cutscene.
 *
 * @return Error code
 */
int JJ1Scene::play () {

	SDL_Rect dst;
	unsigned int sceneIndex = 0;
	JJ1SceneImage *image;
	JJ1SceneAnimation* animation = NULL;
	JJ1SceneFrame* currentFrame = NULL;
	JJ1SceneFrame* lastFrame = NULL;
	PaletteEffect* paletteEffect = NULL;
	int	frameDelay = 0;
	int prevFrame = 0;
	int continueToNextPage = 0;

	unsigned int pageTime = pages[sceneIndex].pageTime;
	unsigned int lastTicks = globalTicks;
	int newpage = true;
	SDL_Rect textRect = {0, 0, SW, SH};

	video.clearScreen(0);

	while (true) {

		bool upOrLeft = false;
		bool downOrRight = false;
		int x, y;

		if (loop(NORMAL_LOOP, paletteEffect) == E_QUIT) {

			if (paletteEffect) delete paletteEffect;

			return E_QUIT;

		}

		controls.getCursor(x, y);

		x -= (canvasW - SW) >> 1;
		y -= (canvasH - SH) >> 1;

		downOrRight = controls.wasCursorReleased();

		if (controls.release(C_ESCAPE) ||
			(controls.release(C_NO) && pages[sceneIndex].askForYesNo) ||
			(downOrRight && (x >= 0) && (x < 100) && (y >= SH - 12) && (y < SH))) {

			if (paletteEffect) delete paletteEffect;

			return E_NONE;

		}

		SDL_Delay(T_FRAME);


		if(pages[sceneIndex].askForYesNo) {
			downOrRight |= controls.release(C_ENTER) || controls.release(C_YES);
		} else {
			upOrLeft = (controls.release(C_UP) || controls.release(C_LEFT));
			downOrRight |= (controls.release(C_RIGHT) || controls.release(C_DOWN) || controls.release(C_ENTER));
		}

		if ((sceneIndex > 0 && upOrLeft) ||
			 downOrRight || continueToNextPage ||
			((globalTicks-lastTicks) >= pageTime * 1000 && pageTime != 256 && pageTime != 0)) {

			if(pages[sceneIndex].stopMusic) {
				stopMusic();
			}

			if (upOrLeft) sceneIndex--;
			else sceneIndex++;

			if (sceneIndex == scriptItems) {

				if (paletteEffect) delete paletteEffect;

				return E_NONE;

			}

			lastTicks = globalTicks;
			// Get bg for this page
			newpage = true;

			pageTime = pages[sceneIndex].pageTime;
			continueToNextPage = 0;
		}

		if (newpage) {

			//if (paletteEffect) delete paletteEffect;
			//paletteEffect = new FadeOutPaletteEffect(250, NULL);

			textRect.x = 0;
			textRect.y = 0;
			textRect.w = SW;
			textRect.h = SH;
			JJ1ScenePalette *palette = palettes;

			while (palette && (palette->id != pages[sceneIndex].paletteIndex)) palette = palette->next;

			if (palette) {

				video.setPalette(palette->palette);

				// Fade in from black
				if (paletteEffect) delete paletteEffect;
				paletteEffect = new FadeInPaletteEffect(250, NULL);

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

			if (currentFrame == NULL) {

				animation = animations;

				while (animation && (animation->id != pages[sceneIndex].animIndex))
					animation = animation->next;

				if (animation && animation->background) {

					dst.x = (canvasW - SW) >> 1;
					dst.y = (canvasH - SH) >> 1;
					frameDelay = 1000 / (pages[sceneIndex].animSpeed >> 8);
					SDL_BlitSurface(animation->background, NULL, canvas, &dst);
					currentFrame = animation->sceneFrames;
					SDL_Delay(frameDelay);

				}

			} else {

				// Upload pixel data to the surface
				if (SDL_MUSTLOCK(animation->background)) SDL_LockSurface(animation->background);

				switch (currentFrame->frameType) {

					case ESquareAniHeader:

						loadCompactedMem(currentFrame->frameSize, currentFrame->frameData, (unsigned char*)animation->background->pixels);

						break;

					case EFFAniHeader:

						loadFFMem(currentFrame->frameSize, currentFrame->frameData, (unsigned char*)animation->background->pixels);

						break;

					default:

						LOG("Scene::Play unknown type", currentFrame->frameType);

						break;

				}

				if (SDL_MUSTLOCK(animation->background)) SDL_UnlockSurface(animation->background);

				dst.x = (canvasW - SW) >> 1;
				dst.y = (canvasH - SH) >> 1;
				SDL_BlitSurface(animation->background, NULL, canvas, &dst);

				if (currentFrame->soundId != -1 && animation->noSounds > 0) {

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
				if (prevFrame) currentFrame = currentFrame->prev;
				else currentFrame = currentFrame->next;

				SDL_Delay(frameDelay);

				if (currentFrame == NULL && animation->reverseAnimation) {

					//prevFrame = 1 - prevFrame;

					/*if(prevFrame) currentFrame = lastFrame->prev;
					else currentFrame = lastFrame->next;*/
					currentFrame = NULL;//animation->sceneFrames;

				} else if (currentFrame == NULL && !pageTime && !pages[sceneIndex].askForYesNo && pages[sceneIndex].nextPageAfterAnim) {

					continueToNextPage = 1;

				}

			}

		} else video.clearScreen(0);


		// Draw the texts associated with this page
		x = 0;
		y = 0;
		int extraLineHeight = 0;

		for (int count = 0; count < pages[sceneIndex].nTexts; count++) {

			JJ1SceneText *text = pages[sceneIndex].texts + count;
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


