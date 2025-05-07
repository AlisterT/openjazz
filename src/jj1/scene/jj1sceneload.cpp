
/**
 *
 * @file jj1sceneload.cpp
 *
 * Part of the OpenJazz project
 *
 * @par History:
 * - 23rd August 2005: Created scene.c
 * - 3rd February 2009: Created scene.h from parts of scene.c
 * - 3rd February 2009: Renamed scene.c to scene.cpp
 * - 27th March 2010: Created sceneload.cpp from parts of scene.cpp
 * - 1st August 2012: Renamed sceneload.cpp to jj1sceneload.cpp
 *
 * @par Licence:
 * Copyright (c) 2005-2012 AJ Thomson
 * Copyright (c) 2015-2024 Carsten Teibes
 *
 * OpenJazz is distributed under the terms of
 * the GNU General Public License, version 2.0
 *
 * @par Description:
 * Deals with the loading of cutscene data.
 *
 */


#include "jj1scene.h"

#include "io/file.h"
#include "io/gfx/font.h"
#include "io/gfx/video.h"
#include "io/sound.h"
#include "util.h"
#include "io/log.h"

#include <string.h>

#define DEBUG_CM 0
#define DEBUG_FF 0
#define DEBUG_ANIM 0
#define DEBUG_SCRIPT 0

#if DEBUG_CM
	#define LOG_CM(...) LOG_MAX(__VA_ARGS__)
#else
	#define LOG_CM(...)
#endif
#if DEBUG_FF
	#define LOG_FF(...) LOG_MAX(__VA_ARGS__)
#else
	#define LOG_FF(...)
#endif
#if DEBUG_ANIM
	#define LOG_ANIM(...) LOG_MAX(__VA_ARGS__)
#else
	#define LOG_ANIM(...)
#endif
#if DEBUG_SCRIPT
	#define LOG_SCRIPT(...) LOG_MAX(__VA_ARGS__)
#else
	#define LOG_SCRIPT(...)
#endif

/**
 * Load a short from a buffer and advance the pointer past it.
 *
 * @param data Pointer to data in a buffer
 *
 * @return The loaded short
 */
unsigned short int JJ1Scene::loadShortMem (unsigned char** data) {

	unsigned short int val;

	val = **data;
	(*data)++;
	val += ((unsigned short)(**data)) << 8;
	(*data)++;

	return val;

}


/**
 * Decompress JJ1 cutscene graphical data.
 *
 * @param size The size of the compressed data
 * @param frameData The compressed data
 * @param pixels Buffer to contain the decompressed data
 */
void JJ1Scene::loadFullFrameMem (int size, unsigned char* frameData, unsigned char* pixels) {
	unsigned char* nextPixel = pixels;
	unsigned char* nextData = frameData;
	int fillWidth = 0;
	bool trans = true;

#if DEBUG_FF
	FILE* out = fopen("ff-output.dat", "wb");
	fwrite(frameData, size, 1, out);
	fclose(out);
#endif

	while ((nextData < frameData + size) && (nextPixel < pixels + (SW * SH))) {
		unsigned char header = *nextData;
		nextData++;
		LOG_FF("PL FF Frame header: 0x%x", header);

		if ((header & 0x7F) == 0x7F) {
			fillWidth = loadShortMem(&nextData);
			if (trans) fillWidth += 255;

			LOG_FF("PL FF Skip %d pixels", fillWidth);
		} else if (header) {

			if(trans) {
				fillWidth = header;
				LOG_FF("PL FF SKIP %d transparent pixels", fillWidth);
			} else {
				fillWidth = header & 0x1F;

				switch (header & 0x60) {
				default:
					LOG_FF("PL FF Unknown operation: 0x%x", header);
					break;

				case 0x00:
					LOG_FF("PL FF Copy %d pixels", fillWidth);
					memcpy(nextPixel, nextData, fillWidth);
					nextData += fillWidth;
					break;

				case 0x20:
					LOG_FF("PL FF Copy %d pixels of previous line", fillWidth);
					if (nextPixel - SW >= pixels)
						memcpy(nextPixel, nextPixel - SW, fillWidth);
					break;

				case 0x60:
					fillWidth = header&0x3F;
					/* FALLTHROUGH */
				case 0x40:
					LOG_FF("PL FF Fill %d pixels of row with color 0x%x", fillWidth, *nextData);
					memset(nextPixel, *nextData, fillWidth);
					nextData++;
					break;
				}
			}
		} else {
			LOG_FF("PL FF FAULTY END OF STREAM: %d", size);
			return;
		}

		nextPixel += fillWidth;

		if (header & 0x80) trans = false;
		else trans = !trans;
	}

	LOG_FF("PL FF pixels: %d", (unsigned int)(nextPixel - pixels));
}


/**
 * Decompress JJ1 cutscene graphical data.
 *
 * @param size The size of the compressed data
 * @param frameData The compressed data
 * @param pixels Buffer to contain the decompressed data
 */
void JJ1Scene::loadCompactedMem (int size, unsigned char* frameData, unsigned char* pixels) {

	unsigned char* nextPixel = pixels;
	unsigned char* endpixdata = pixels + (SW * SH);
	unsigned char* fillstart = NULL;
	int fillWidth;

#if DEBUG_CM
	FILE* out = fopen("cm-output.dat", "wb");
	fwrite(frameData, size, 1, out);
	fclose(out);
#endif

	auto fillColumns = [&](unsigned char color, int columns) {
		LOG_CM("PL CM Fill %d columns with color 0x%x", columns, color);

		fillstart = nextPixel;
		while (fillstart + columns <= endpixdata) {
			memset(fillstart, color, columns);
			fillstart += SW;
		}
		if(fillstart < endpixdata) {
			// Filling bottom row
			memset(fillstart, color, endpixdata-fillstart);
		}
	};

	while (size > 0) {

		unsigned char header = *frameData;
		frameData++;

		if (header == 0x7F) {
			fillWidth = loadShortMem(&frameData);
			unsigned char fillColor = *frameData;
			frameData++;
			fillColumns(fillColor, fillWidth);

			size -= 3;
		} else if (header == 0xFF) {
			fillWidth = loadShortMem(&frameData);
			LOG_CM("PL CM Skip %d pixels", fillWidth);

			size -= 2;
		} else if (header & 0x80) {
			fillWidth = (header - 0x80) + 1; // max 0xFE
			LOG_CM("PL CM Skip %d pixels", fillWidth);
		} else if (header & 0x40) {
			unsigned char fillColor = *frameData;
			frameData++;
			fillWidth = (header - 0x40) + 1; // max 0x7E
			fillColumns(fillColor, fillWidth);

			size--;
		} else {
			fillWidth = (header & 0x3F) + 1; // max 0x3F
			LOG_CM("PL CM Fill %d columns with raw pixels", fillWidth);

			for (int col = 0; col < fillWidth; col++) {
				unsigned char color = *frameData;
				frameData++;

				if (color != 0xFF) {
					fillstart = nextPixel + col;

					while (fillstart < endpixdata) {
						*fillstart = color;
						fillstart += SW;
					}
				}

				size--;
			}
		}

		nextPixel += fillWidth;
		size--;
	}

	LOG_CM("PL CM pixels: %d", (unsigned int)(nextPixel - pixels));
}


/**
 * Load JJ1 cutscene animation.
 *
 * @param scene Object to hold animation data
 * @param f File from which to load animation
 * @param dataIndex Index
 */
void JJ1Scene::loadAni (JJ1SceneAnimation &scene, File *f, int dataIndex) {

	LOG_ANIM("ParseAni DataLen: 0x%x", f->loadShort()); // should be 0x02
	LOG_ANIM("ParseAni Frames: %d", f->loadShort());
	unsigned short int type = 0;
	int loop;

	while (type != EPlayListAniHeader) {

		type = f->loadShort();

		if (type == ESoundListAniHeader) {

			/*unsigned short int offset =*/ f->loadShort();
			unsigned char nSounds = f->loadChar();

			for(loop = 0; loop < nSounds; loop++) {

				char* soundName = f->loadTerminatedString();
				LOG_ANIM("Soundname: %s", soundName);
				resampleSound(loop, soundName, 11025);
				delete[] soundName;

			}

		} else if (type == EPlayListAniHeader) {
			int nextPos = f->tell();
			LOG_ANIM("PL Read position: %d", nextPos);
			f->loadShort(); // Length

			palettes.emplace_back(dataIndex);
			auto &scenePalette = palettes.back();
			f->loadPalette(scenePalette.palette, false);

#if DEBUG_ANIM
			int items = 0;
#endif
			LOG_ANIM("PL Read position start: %d", f->tell());

			int validValue = true;
			while (validValue) {

				unsigned short int value = f->loadShort();
				LOG_ANIM("PL Read block start tag: 0x%x", value);
			    int size = f->loadShort();
				LOG_ANIM("PL Anim block size: %d", size);
				nextPos = f->tell();
				// next pos is intial position + size and four bytes header
				nextPos += size;

				switch (value) {

					case EEndAniHeader: // END MARKER
						validValue = false;
						break;

					case E11AniHeader:
						LOG_ANIM("PL 11 Background Type: 0");
						scene.background = f->loadSurface(SW, SH, false);

						// Use the most recently loaded palette
						video.setPalette(scenePalette.palette);
						break;

					case E1LAniHeader:
						{
							LOG_ANIM("PL 1L Background Type: 0");
							unsigned char* pixels = new unsigned char[SW* SH];
							memset(pixels, 0, SW*SH);
							unsigned char* frameData = f->loadBlock(size);
							loadCompactedMem(size, frameData, pixels);
							delete[] frameData;
							scene.background = createSurface(pixels, SW, SH);
							delete[] pixels;

							// Use the most recently loaded palette
							video.setPalette(scenePalette.palette);
						}
						break;

					case EFullFrameAniHeader:
						{
							unsigned char* blockData = f->loadBlock(size);
							scene.addFrame(EFullFrameAniHeader, blockData, size);
						}
						break;

					case ERNAniHeader:
					case ERBAniHeader:
					case ERLAniHeader:
					case EMXAniHeader:
						LOG_DEBUG("PL Read Unsupported animation type 0x%x", value);
						break;

					case EReverseAniHeader:
						scene.reverseAnimation = 1;
						break;

					case ERCAniHeader:
						{
							unsigned char* blockData = f->loadBlock(size);
							scene.addFrame(ERCAniHeader, blockData, size);
						}
						break;

					case ECompactedAniHeader:
						{
							unsigned char* blockData = f->loadBlock(size);
							scene.addFrame(ECompactedAniHeader, blockData, size);
						}
						break;

					case ESoundTagAniHeader:
						{
							auto se = static_cast<SE::Type>(f->loadChar());
							if (!isValidSoundIndex(se)) {
								LOG_WARN("PL Audio tag with invalid index: %d", se);
								scene.lastFrame->soundId = SE::NONE;
							} else {
								LOG_ANIM("PL Audio tag with index: %d", se);
								scene.lastFrame->soundId = se;
							}
							LOG_ANIM("PL Audio tag play at: 0x%x", f->loadChar());
							LOG_ANIM("PL Audio tag play offset: 0x%x", f->loadChar());
						}
						break;

					case E00AniHeader: // FIXME: wait command?
						{
							int longvalue = f->loadInt();

							while (longvalue == 0) {

								longvalue = f->loadInt();
								nextPos += 4;

							}

							f->seek(-4, false);
							value = longvalue;

							LOG_ANIM("PL Read Long: 0x%x", value);
						}
						break;

					default:

						LOG_DEBUG("PL Read Unknown type: 0x%x", value);
						validValue = false;

						break;

				}

				LOG_ANIM("PL Read position after block should be: %d", nextPos);
				f->seek(nextPos, true);

#if DEBUG_ANIM
				if(validValue)
					items++;
#endif
			}

			LOG_ANIM("PL Parsed through number of items skipping 0 items: %d", items);
			LOG_ANIM("PL Read position after parsing anim blocks: %d", f->tell());

		}

	}

}


/**
 * Load JJ1 cutscene data.
 *
 * @param f File from which to load the data
 */
void JJ1Scene::loadData (File *f) {

	for (int loop = 0; loop < dataItems; loop++) {

		f->seek(dataOffsets[loop], true); // Seek to data start
		unsigned short int dataLen = f->loadShort(); // Get get the length of the datablock
		LOG_MAX("Data dataLen: %d", dataLen);

		if (dataLen == EAniHeader) {

			LOG_MAX("Data Type: ANI");
			animations.emplace_back(loop);
			auto &sceneAnimation = animations.back();
			loadAni(sceneAnimation, f, loop);

		} else {

			unsigned char type = f->loadChar();
			LOG_MAX("Data Type: %d", type);

			switch (type) {

				case 3:
				case 4: // image
				case 5:
				case 6:
				case 7:
					{
						LOG_MAX("Data Type: Image, index: %d", loop);
						unsigned short int width = f->loadShort(SW); // Get width
						unsigned short int height;

						// Get height
						if (type == 3) height = f->loadChar();
						else height = f->loadShort(SH);

						images.emplace_back(loop);
						auto& sceneImage = images.back();

						if (type >= 5 && type <= 7) {
							f->seek(-5, false); // account for metadata 2 + 2
							unsigned char* pixels = unpackRLE(f->loadBlock(dataLen), dataLen, width * height + 4);
							sceneImage.image = createSurface(pixels + 4, width, height);
							delete[] pixels;
						} else {
							sceneImage.image = f->loadSurface(width, height, false);
						}
					}
					break;

				default:

					LOG_MAX("Data Type: Palette, index: %d", loop);
					f->seek(-3, false); // fake an RLE block size marker and use first palette byte

					palettes.emplace_back(loop);
					auto& palette = palettes.back();
					f->loadPalette(palette.palette);

					break;

			}

		}

	}

}


/**
 * Load JJ1 cutscene scripts.
 *
 * @param f File from which to load the scripts
 */
void JJ1Scene::loadScripts (File *f) {

	int textAlignment = 0;
	int textFont = 0;
	int textShadow = -1;
	int textColour = 0;
	int loop = 0;

	for(auto &page : pages) {
	    LOG_MAX("Parse Script: %d", loop);

	    SDL_Rect textRect = { 0,0,0,0 };
		f->seek(scriptStarts[loop], true); // Seek to data start

		if (f->loadChar() == 0x50) { // Script tag

			LOG_MAX("Script id: 0x%x", f->loadShort());
			int palette = f->loadShort();
			LOG_SCRIPT("Script default palette: %d", palette);
			page.paletteIndex = palette;

			bool breakloop = false;
			int pos = f->tell();

			int textPosX = -1;
			int textPosY = -1;
			int extraheight = -1;
			bool textRectValid = false;

			while(!breakloop && pos < dataOffsets[0]) {

				unsigned char type = f->loadChar();

				switch(type) {

					case ESceneYesNo:
						{
							page.askForYesNo = 1;
							LOG_SCRIPT("ESceneYesNo");
						}
						break;
					case ESceneStopMusic:
						{
							page.stopMusic = 1;
							LOG_SCRIPT("ESceneStopMusic");
						}
						break;
					case ESceneAnimation:
						{
							page.animLoops = f->loadInt();
							page.animSpeed = f->loadShort();
							page.animIndex = f->loadShort();

							LOG_SCRIPT("ESceneAnimation: loops %d, speed %d, anim num %d",
								page.animLoops, page.animSpeed, page.animIndex);
						}
						break;

					case ESceneAnimationPlayAndContinue:
						{
							page.nextPageAfterAnim = f->loadChar();
							LOG_SCRIPT("ESceneAnimationPlayAndContinue: %d", page.nextPageAfterAnim);
						}
						break;

					case ESceneFadeType:
						{
							LOG_TRACE("Unimplemented ESceneFadeType: %x", f->loadChar());
						}
						break;

					case ESceneBackground:

						page.bgX[page.backgrounds] = f->loadShort();
						page.bgY[page.backgrounds] = f->loadShort();
						page.bgIndex[page.backgrounds] = f->loadShort();

						LOG_SCRIPT("ESceneBackground: x %d, y %d, index %d", page.bgX[page.backgrounds],
							page.bgY[page.backgrounds], page.bgIndex[page.backgrounds]);

						page.backgrounds++;

						break;

					case ESceneMusic:

						page.musicFile = f->loadTerminatedString();
						LOG_SCRIPT("ESceneMusic: %s", page.musicFile);

						break;

					case ESceneSomethingElse:
						{
							LOG_TRACE("Unimplemented ESceneSomethingElse");
						}
						break;

					case ESceneTextRect:

						textRect.x = f->loadShort();
						textRect.y = f->loadShort();
						textRect.w = f->loadShort() - textRect.x;
						textRect.h = f->loadShort() - textRect.y;
						textRectValid = true;
						LOG_SCRIPT("ESceneTextRect: x %d, y %d, w %d, h %d",
							textRect.x, textRect.y, textRect.w, textRect.h);

						break;

					case ESceneFontDefine:

						if (fonts.size() < 5) {
							int id = f->loadShort();
							fonts.emplace_back(id);
							auto &sceneFont = fonts.back();
							char *fontname = f->loadTerminatedString();

							LOG_SCRIPT("ESceneFontDefine: %s with id %d", fontname, sceneFont.id);

							if (strcmp(fontname, "FONT2") == 0)
								sceneFont.font = font2;
							else if (strcmp(fontname, "FONTBIG") == 0)
								sceneFont.font = fontbig;
							else if (strcmp(fontname, "FONTINY") == 0)
								sceneFont.font = fontiny;
							else if (strcmp(fontname, "FONTMN1") == 0)
								sceneFont.font = fontmn1;
							else if (strcmp(fontname, "FONTMN2") == 0)
								sceneFont.font = fontmn2;
							else {
								LOG_WARN("ESceneFontDefine: Unknown font name, using font2");
								sceneFont.font = font2;
							}

							delete[] fontname;
						}
						break;

					case ESceneTextPosition:

						textPosX = f->loadShort();
						textPosY = f->loadShort();
						LOG_SCRIPT("ESceneTextPosition: x %d, y %d", textPosX, textPosY);

						break;

					case ESceneTextColour:

						textColour = f->loadShort();
						LOG_SCRIPT("ESceneTextColour: %d", textColour);

						break;

					case ESceneFontFun:
						{
							LOG_TRACE("Unimplemented ESceneFontFun len: %d", f->loadShort());

							/*while (len) {

								unsigned char data = f->loadChar();
								len--;

							}*/
						}
						break;

					case ESceneFontIndex:

						textFont = f->loadShort();
						LOG_SCRIPT("ESceneFontIndex: %d", textFont);

						break;

					case ESceneTextVAdjust:

						extraheight = f->loadShort();
						LOG_SCRIPT("ESceneTextVAdjust: %d", extraheight);

						break;

					case ESceneBackgroundFade:
						{
							page.backgroundFade =  f->loadShort();
							LOG_SCRIPT("ESceneBackgroundFade: %d", page.backgroundFade);
						}
						break;

					case ESceneTextShadow:
						{
							char enableShadow = f->loadChar();
							if(enableShadow) {
								textShadow = f->loadChar();
							} else {
								f->loadChar(); // Skip this value since shadows are turned off
								textShadow = -1; // Turn off shadow , -1 means no shadow colour
							}

							LOG_SCRIPT("ESceneTextShadow: %d", textShadow);
						}
						break;

					case ESceneTextAlign:

						textAlignment = f->loadChar();
						LOG_SCRIPT("ESceneTextAlign: %d", textAlignment);

						break;

					case ESceneTextAlign2:
						{
							char a = f->loadChar();
							int b = f->loadShort();
							LOG_TRACE("Unimplemented ESceneTextAlign2: a 0x%x, b 0x%x", a, b);
						}
						break;

					case ESceneTextSomething:
						{
							char a = f->loadChar();
							int b = f->loadShort();
							LOG_TRACE("Unimplemented ESceneTextSomething: a 0x%x, b 0x%x", a, b);
						}
						break;

					case ESceneTextLine:
					case ESceneTextBlock:
						{
							unsigned char datalen = f->loadChar();
							LOG_SCRIPT("Text len: %d", datalen);

							page.texts.emplace_back();
							auto &sceneText = page.texts.back();

							if (datalen > 0) {
								sceneText.text = reinterpret_cast<unsigned char *>(f->loadString(datalen));

								// Convert number placeholders
								if (type == ESceneTextLine) {
									for (int textPos = 1; textPos < datalen; textPos++) {

										if (sceneText.text[textPos] == 0x8B) {
											// Current page
											if (loop >= 9)
												sceneText.text[textPos - 1] = ((loop + 1) / 10) + 53;

											sceneText.text[textPos] = ((loop + 1) % 10) + 53;
										} else if (sceneText.text[textPos] == 0x8A) {
											// Number of pages
											if (scriptItems >= 10)
												sceneText.text[textPos - 1] = (scriptItems / 10) + 53;

											sceneText.text[textPos] = (scriptItems % 10) + 53;
										} else if (sceneText.text[textPos] == 0x8C) {
											// FIXME: seems not used at all
											sceneText.text[textPos-2] = '0';
											sceneText.text[textPos-1] = '0';
											sceneText.text[textPos] = '0';
										} else if (sceneText.text[textPos] > 0x8C) {
											// internal
											sceneText.text[textPos] = '0';
										}
									}
								}
							}

							sceneText.alignment = textAlignment;
							sceneText.fontId = textFont;
							sceneText.textColour = textColour;
							sceneText.shadowColour = textShadow;

							if(textPosX != -1) {

								sceneText.x = textPosX;
								sceneText.y = textPosY;
								textPosX = -1;
								textPosY = -1;

							}

							if(textRectValid) {

								sceneText.textRect = textRect;
								textRectValid = false;

							}

							if(extraheight != -1) {

								sceneText.extraLineHeight = extraheight;
								extraheight = -1;

							}
						}

						break;

					case ESceneTime:

						page.pageTime = (f->loadShort());
						LOG_SCRIPT("Scene time: %d", page.pageTime);
						page.pageTime&=255;
						break;

					case ESceneBreaker:
					case 0x3e:

						pos = f->tell();
						LOG_SCRIPT("Parse script end at position: %d, with: 0x%x", pos, type);
						breakloop = true;
						f->loadChar();

						break;

					default:

						pos = f->tell();
						LOG_SCRIPT("Parse script end at position: %d, breaker: 0x%x", pos, type);
						breakloop = true;

						break;

				}

				pos = f->tell();

			}

		}

		loop++;
	}

}
