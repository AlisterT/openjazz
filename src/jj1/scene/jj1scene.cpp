
/**
 *
 * @file jj1scene.cpp
 *
 * Part of the OpenJazz project
 *
 * @par Licence:
 * Copyright (c) 2005-2017 Alister Thomson
 * Copyright (c) 2015-2024 Carsten Teibes
 *
 * OpenJazz is distributed under the terms of
 * the GNU General Public License, version 2.0
 *
 * @par Description:
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
#include "io/log.h"

#include <string.h>
#include <algorithm> // std::find_if


/**
 * Create a JJ1 cutscne frame.
 *
 * @param newFrameType The type of the frame
 * @param newFrameData The frame's data buffer
 * @param newFrameSize The size of the frame's data buffer
 */
JJ1SceneFrame::JJ1SceneFrame(int newFrameType, unsigned char* newFrameData, int newFrameSize) :
	next(nullptr), prev(nullptr), frameData(newFrameData),
	frameSize(newFrameSize), frameType(newFrameType), soundId(SE::NONE) {

}


/**
 * Delete the JJ1 cutscene frame.
 */
JJ1SceneFrame::~JJ1SceneFrame() {
	delete[] frameData;
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
 */
JJ1SceneAnimation::JJ1SceneAnimation (int id) :
	sceneFrames(nullptr), lastFrame(nullptr), background(nullptr),
	id(id), frames(0), reverseAnimation(0) {

	scratch = createSurface(nullptr, SW, SH);
}


/**
 * Delete the JJ1 cutscene animation.
 */
JJ1SceneAnimation::~JJ1SceneAnimation () {
	if(sceneFrames) {
		JJ1SceneFrame* frame = sceneFrames;
		JJ1SceneFrame* nextFrame = NULL;
		while(frame) {
			nextFrame = frame->next;
			delete frame;
			frame = NULL;
			frame = nextFrame;
		}
	}

	if (background) {
		SDL_FreeSurface(background);
		background = nullptr;
	}
	if (scratch) {
		SDL_FreeSurface(scratch);
		scratch = nullptr;
	}
}


/**
 * Create a JJ1 cutscene image.
 */
JJ1SceneImage::JJ1SceneImage (int id) :
	image(nullptr), id(id) {

}


/**
 * Delete the JJ1 cutscene image.
 */
JJ1SceneImage::~JJ1SceneImage () {
	if (image) {
		SDL_FreeSurface(image);
		image = nullptr;
	}
}


/**
 * Create a JJ1 cutscene palette.
 */
JJ1ScenePalette::JJ1ScenePalette(int id) :
	id(id) {

}

/**
 * Create a JJ1 cutscene font.
 */
JJ1SceneFont::JJ1SceneFont(int id) :
	id(id) {

}

/**
 * Create a JJ1 cutscene text object.
 */
JJ1SceneText::JJ1SceneText() :
	text(nullptr), alignment(0), fontId(-1), x(-1), y(-1),
	textRect({-1, -1, 0, 0}), extraLineHeight(-1),
	textColour(0), shadowColour(-1) {

}


/**
 * Delete the JJ1 cutscene text object.
 */
JJ1SceneText::~JJ1SceneText() {
	if (text) {
		delete[] text;
		text = nullptr;
	}
}


/**
 * Create a JJ1 cutscene page.
 */
JJ1ScenePage::JJ1ScenePage() :
	backgrounds(0), animLoops(0), animSpeed(0), animIndex(-1), // no anim
	nextPageAfterAnim(0), pageTime(0), musicFile(nullptr),
	paletteIndex(0), askForYesNo(0), stopMusic(0), backgroundFade(255)
	{

}


/**
 * Delete the JJ1 cutscene page.
 */
JJ1ScenePage::~JJ1ScenePage() {
	if (musicFile) {
		delete[] musicFile;
		musicFile = nullptr;
	}
}


/**
 * Create a JJ1 cutscene.
 *
 * @param fileName Name of the file containing the cutscene data
 */
JJ1Scene::JJ1Scene (const char * fileName) {

	FilePtr file;
	LOG_TRACE("Scene: %s", fileName);

	try {

		file = std::make_unique<File>(fileName, PATH_TYPE_GAME);

	} catch (int e) {

		throw;

	}

	// Checking scene file
	unsigned char *identifier1 = file->loadBlock(18);
	char identifier2 = file->loadChar();
	if (memcmp(identifier1, "Digital Dimensions", 18) != 0 || identifier2 != 0x1A) {
		LOG_ERROR("Scene not valid!");
		delete[] identifier1;
		return;
	}
	delete[] identifier1;

	signed long int dataOffset = file->loadInt(); // Get offset pointer to first data block

	scriptItems = file->loadShort(); // Get number of script items
	scriptStarts.resize(scriptItems);
	pages.resize(scriptItems);
	LOG_TRACE("Scene: Script items: %d", scriptItems);
	for (int i = 0; i < scriptItems; i++) {
		scriptStarts[i] = file->loadInt(); // Load offset to script
		LOG_MAX("scriptStart: %ld", scriptStarts[i]);
	}

	// Seek to datastart now
	file->seek(dataOffset, true); // Seek to data offsets
	dataItems = file->loadShort() + 1; // Get number of data items
	LOG_TRACE("Scene: Data items %d", dataItems);
	dataOffsets.resize(dataItems);
	for (int i = 0; i < dataItems; i++) {
		dataOffsets[i] = file->loadInt(); // Load offset to script
		LOG_MAX("dataOffsets: %ld", dataOffsets[i]);
	}

	loadData(file.get());
	loadScripts(file.get());
}


/**
 * Play the JJ1 cutscene.
 *
 * @return Error code
 */
int JJ1Scene::play () {

	SDL_Rect dst;
	unsigned int sceneIndex = 0;
	JJ1SceneFrame* currentFrame = NULL;
	auto sceneAnimation = animations.end();
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

		SDL_Delay(T_MENU_FRAME);


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

			auto scenePalette = std::find_if(palettes.begin(), palettes.end(),
				[&](const auto& pal) { return pal.id == pages[sceneIndex].paletteIndex; });
			if (scenePalette != palettes.end()) {

				video.setPalette(scenePalette->palette);

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

				const auto sceneImage = std::find_if(images.begin(), images.end(),
					[&](const auto& img) { return img.id == pages[sceneIndex].bgIndex[bg]; });
				if (sceneImage != images.end()) {

					dst.x = pages[sceneIndex].bgX[bg] + ((canvasW - SW) >> 1);
					dst.y = pages[sceneIndex].bgY[bg] + ((canvasH - SH) >> 1);
					SDL_BlitSurface(sceneImage->image, nullptr, canvas, &dst);

				}

			}

		} else if (pages[sceneIndex].animIndex != -1) {

			if (currentFrame == NULL) {

				sceneAnimation = std::find_if(animations.begin(), animations.end(),
					[&](const auto& anim) { return anim.id == pages[sceneIndex].animIndex; });
				if (sceneAnimation != animations.end() && sceneAnimation->background) {

					dst.x = (canvasW - SW) >> 1;
					dst.y = (canvasH - SH) >> 1;
					frameDelay = 1000 / (pages[sceneIndex].animSpeed >> 8);
					SDL_BlitSurface(sceneAnimation->background, NULL, canvas, &dst);
					SDL_BlitSurface(sceneAnimation->background, NULL, sceneAnimation->scratch, NULL);
					currentFrame = sceneAnimation->sceneFrames;
					SDL_Delay(frameDelay);

				}

			} else {

				// Upload pixel data to the surface
				if (SDL_MUSTLOCK(sceneAnimation->scratch)) SDL_LockSurface(sceneAnimation->scratch);

				switch (currentFrame->frameType) {

					case ECompactedAniHeader:

						loadCompactedMem(currentFrame->frameSize, currentFrame->frameData, static_cast<unsigned char*>(sceneAnimation->scratch->pixels));

						break;

					case EFullFrameAniHeader:

						loadFullFrameMem(currentFrame->frameSize, currentFrame->frameData, static_cast<unsigned char*>(sceneAnimation->scratch->pixels));

						break;

					default:

						LOG_DEBUG("Scene::Play unknown type: %d", currentFrame->frameType);

						break;

				}

				if (SDL_MUSTLOCK(sceneAnimation->scratch)) SDL_UnlockSurface(sceneAnimation->scratch);

				dst.x = (canvasW - SW) >> 1;
				dst.y = (canvasH - SH) >> 1;
				SDL_BlitSurface(sceneAnimation->scratch, NULL, canvas, &dst);

				playSound(currentFrame->soundId);

				if (prevFrame) currentFrame = currentFrame->prev;
				else currentFrame = currentFrame->next;

				SDL_Delay(frameDelay);

				if (currentFrame == NULL && sceneAnimation->reverseAnimation) {

					//prevFrame = 1 - prevFrame;

					/*if(prevFrame) currentFrame = lastFrame->prev;
					else currentFrame = lastFrame->next;*/
					currentFrame = NULL;//sceneAnimation->sceneFrames;

				} else if (currentFrame == NULL && !pageTime && !pages[sceneIndex].askForYesNo && pages[sceneIndex].nextPageAfterAnim) {

					continueToNextPage = 1;

				}

			}

		} else video.clearScreen(0);


		// Draw the texts associated with this page
		x = 0;
		y = 0;
		int extraLineHeight = 0;

		for (const auto& sceneText : pages[sceneIndex].texts) {

			const auto sceneFont = std::find_if(fonts.begin(), fonts.end(),
				[&](const auto& fnt) { return fnt.id == sceneText.fontId; });
			if (sceneFont != fonts.end()) {
				Font* font = sceneFont->font;

				if (sceneText.x != -1) {

					x = sceneText.x;
					y = sceneText.y;

				}

				if (sceneText.textRect.x != -1) {

					textRect = sceneText.textRect;
					x = 0;
					y = 0;

				}

				if (sceneText.extraLineHeight != -1) {

					extraLineHeight = sceneText.extraLineHeight;

				}

				// Actual drawing is only needed when there is text provided
				if(sceneText.text) {
					int xOffset = ((canvasW - SW) >> 1) + textRect.x + x;
					int yOffset = ((canvasH - SH) >> 1) + textRect.y + y;

					switch (sceneText.alignment) {

						case 0: // left

							break;

						case 1: // right

							xOffset += textRect.w - font->getSceneStringWidth(sceneText.text);

							break;

						case 2: // center

							xOffset += (textRect.w - font->getSceneStringWidth(sceneText.text)) >> 1;

							break;

					}

					// Drop shadow
					if(sceneText.shadowColour != -1) {
						font->mapPalette(0, MAX_PALETTE_COLORS, 0, 1);
						font->showSceneString(sceneText.text, xOffset - 1, yOffset + 2);
						font->restorePalette();
					}

					// Text itself
					if(sceneText.textColour) {
						font->mapPalette(0, MAX_PALETTE_COLORS, sceneText.textColour, MAX_PALETTE_COLORS);
					}
					font->showSceneString(sceneText.text, xOffset, yOffset);
					if(sceneText.textColour) {
						font->restorePalette();
					}
				}

				y += extraLineHeight + font->getHeight();
			}
		}

	}

	return E_NONE;

}
