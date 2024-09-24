
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
 * Copyright (c) 2005-2012 Alister Thomson
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
void JJ1Scene::loadFFMem (int size, unsigned char* frameData, unsigned char* pixels) {
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
		LOG_FF("PL FF Frame header: %x", header);

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
					LOG_FF("PL FF Unknown operation: %x", header);
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
 * @param f File from which to load animation
 * @param dataIndex Index
 */
void JJ1Scene::loadAni (File *f, int dataIndex) {

	LOG_ANIM("ParseAni DataLen: %x", f->loadShort()); // should be 0x02
	LOG_ANIM("ParseAni Frames: %d", f->loadShort());
	unsigned short int type = 0;
	int loop;

	while (type != EPlayListAniHeader) {

		type = f->loadShort();

		if (type == ESoundListAniHeader) { // SL

			/*unsigned short int offset =*/ f->loadShort();
			unsigned char nSounds = f->loadChar();

			for(loop = 0; loop < nSounds; loop++) {

				char* soundName = f->loadString();
				LOG_ANIM("Soundname: %s", soundName);
				resampleSound(loop, soundName, 11025);
				delete[] soundName;

			}

		} else if (type == EPlayListAniHeader) {// PL
			int nextPos = f->tell();
			LOG_ANIM("PL Read position: %d", nextPos);
			f->loadShort(); // Length

			palettes = new JJ1ScenePalette(palettes);

			f->loadPalette(palettes->palette, false);

			palettes->id = dataIndex;

			int items = 0;
			int validValue = true;

			LOG_ANIM("PL Read position start: %d", f->tell());

			while (validValue) {

				unsigned short int value = f->loadShort();
				LOG_ANIM("PL Read block start tag: %x", value);
			    int size = f->loadShort();
				LOG_ANIM("PL Anim block size: %d", size);
				nextPos = f->tell();
				// next pos is intial position + size and four bytes header
				nextPos += size;

				switch (value) {

					case E_EHeader: // END MARKER
						validValue = false;
						break;

					case E11AniHeader: //11
						LOG_ANIM("PL 11 Background Type: 0");
						animations->background = f->loadSurface(SW, SH, false);

						// Use the most recently loaded palette
						video.setPalette(palettes->palette);
						break;

					case E1LAniHeader: {
						LOG_ANIM("PL 1L Background Type: 0");
						unsigned char* pixels;
						pixels = new unsigned char[SW* SH];
						memset(pixels, 0, SW*SH);
						unsigned char* frameData;
						frameData = f->loadBlock(size);
						loadCompactedMem(size, frameData, pixels);
						delete[] frameData;
						animations->background = createSurface(pixels, SW, SH);
						delete[] pixels;
						// Use the most recently loaded palette
						video.setPalette(palettes->palette);
						}
						break;

					case EFFAniHeader:
						{
						unsigned char* blockData = f->loadBlock(size);
						animations->addFrame(EFFAniHeader, blockData, size);
						}
						break;

					case ERNAniHeader:
					case ERBAniHeader:
					case ERLAniHeader:
					case EMXAniHeader:
						LOG_DEBUG("PL Read Unsupported animation type 0x%x", value);
						break;

					case ERRAniHeader: // Reverse animation when end found
						animations->reverseAnimation = 1;
						break;

					case ERCAniHeader:
						{
						unsigned char* blockData = f->loadBlock(size);
						animations->addFrame(ERCAniHeader, blockData, size);
						}
						break;

					case ESquareAniHeader: // Full screen animation frame, that does n't clear the screen first.
						{
							unsigned char* blockData = f->loadBlock(size);
							animations->addFrame(ESquareAniHeader, blockData, size);
						}
						break;

					case ESTAniHeader: // Sound item
						{
							auto se = static_cast<SE::Type>(f->loadChar());
							if (!isValidSoundIndex(se)) {
								LOG_WARN("PL Audio tag with invalid index: %d", se);
								animations->lastFrame->soundId = SE::NONE;
							} else {
								LOG_ANIM("PL Audio tag with index: %d", se);
								animations->lastFrame->soundId = se;
							}
							LOG_ANIM("PL Audio tag play at: %x", f->loadChar());
							LOG_ANIM("PL Audio tag play offset: %x", f->loadChar());
						}
						break;

					case 0:
						{
							int longvalue = f->loadInt();

							while (longvalue == 0) {

								longvalue = f->loadInt();
								nextPos += 4;

							}

							f->seek(-4, false);
							value = longvalue;

							LOG_ANIM("PL Read Long: %x", value);
						}
						break;

					default:

						LOG_ANIM("PL Read Unknown type: %x", value);
						validValue = false;

						break;

				}

				LOG_ANIM("PL Read position after block should be: %d", nextPos);
				f->seek(nextPos, true);

				if(validValue) items++;

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

	int loop;

	for (loop = 0; loop < dataItems; loop++) {

		f->seek(dataOffsets[loop], true); // Seek to data start
		unsigned short int dataLen = f->loadShort(); // Get get the length of the datablock
		LOG_MAX("Data dataLen: %d", dataLen);
		// AN

		if (dataLen == 0x4e41) {

			LOG_MAX("Data Type: ANI");
			animations = new JJ1SceneAnimation(animations);
			animations->id = loop;
			loadAni(f, loop);

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

						images = new JJ1SceneImage(images);
						if (type >= 5 && type <= 7) {
							f->seek(-5, false); // account for metadata 2 + 2
							unsigned char* pixels = unpackRLE(f->loadBlock(dataLen), dataLen, width * height + 4);
							images->image = createSurface(pixels + 4, width, height);
							delete[] pixels;
						} else {
							images->image = f->loadSurface(width, height, false);
						}

						images->id = loop;

					}

					break;

				default:

					LOG_MAX("Data Type: Palette, index: %d", loop);
					f->seek(-3, false); // fake an RLE block size marker and use first palette byte

					palettes = new JJ1ScenePalette(palettes);
					f->loadPalette(palettes->palette);
					palettes->id = loop;

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

	int loop;
	/*int bgIndex = 0;*/
	int textAlignment = 0;
	int textFont = 0;
	int textShadow = -1;

	for(loop = 0; loop < scriptItems; loop++) {

	    LOG_MAX("Parse Script: %d", loop);

	    SDL_Rect textRect = { 0,0,0,0 };
		f->seek(scriptStarts[loop], true); // Seek to data start

		if (f->loadChar() == 0x50) { // Script tag

			LOG_MAX("Script id: %x", f->loadShort());
			int palette = f->loadShort();
			LOG_MAX("Script default palette: %x", palette);
			pages[loop].paletteIndex = palette;

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
						pages[loop].askForYesNo = 1;
						LOG_MAX("ESceneYesNo");
						}break;
					case ESceneStopMusic:
						{
						pages[loop].stopMusic = 1;
						LOG_MAX("ESceneStopMusic");
						}break;
					case ESceneAnimation:
						{
							pages[loop].animLoops = f->loadInt();
							pages[loop].animSpeed = f->loadShort();
							pages[loop].animIndex = f->loadShort();

							LOG_MAX("ESceneAnimation - loops: %d", pages[loop].animLoops);
							LOG_MAX("ESceneAnimation - speed: %d", pages[loop].animSpeed);
							LOG_MAX("ESceneAnimation - anim num: %d", pages[loop].animIndex);
						}
						break;

					case ESceneAnimationPlayAndContinue:
						{
						pages[loop].nextPageAfterAnim = f->loadChar();
						LOG_MAX("ESceneAnimationPlayAndContinue: %d", pages[loop].nextPageAfterAnim);
						}

						break;

					case ESceneFadeType:

						{
							LOG_MAX("ESceneFadeType: %x", f->loadChar());

						}

						break;

					case ESceneBackground:

						pages[loop].bgX[pages[loop].backgrounds] = f->loadShort();
						pages[loop].bgY[pages[loop].backgrounds] = f->loadShort();
						pages[loop].bgIndex[pages[loop].backgrounds] = f->loadShort();

						LOG_MAX("ESceneBackground - xpos: %d", pages[loop].bgX[pages[loop].backgrounds]);
						LOG_MAX("ESceneBackground - ypos: %d", pages[loop].bgY[pages[loop].backgrounds]);
						LOG_MAX("ESceneBackground - index: %d", pages[loop].bgIndex[pages[loop].backgrounds]);

						pages[loop].backgrounds++;

						break;

					case ESceneMusic:

						// Music file name
						pages[loop].musicFile = f->loadString();
						LOG_MAX("ESceneMusic: %s", pages[loop].musicFile);

						break;

					case ESceneSomethingElse:

						{

							LOG_MAX("ESceneSomethingElse");

						}

						break;

					case ESceneTextRect: // String

						textRect.x = f->loadShort();
						textRect.y = f->loadShort();
						textRect.w = f->loadShort() - textRect.x;
						textRect.h = f->loadShort() - textRect.y;
						textRectValid = true;
						LOG_MAX("Text rectangle (x, y, w, h): %d, %d, %d, %d",
							textRect.x, textRect.y, textRect.w, textRect.h);

						break;

					case ESceneFontDefine: // Font defnition

						if (nFonts < 5) {

							fonts[nFonts].id = f->loadShort();
							char *fontname = f->loadString();

							LOG_MAX("ESceneFontDefine: %s", fontname);
							LOG_MAX("ESceneFontDefine with id: %d", fonts[nFonts].id);

							if (strcmp(fontname, "FONT2") == 0)
								fonts[nFonts].font = font2;
							else if (strcmp(fontname, "FONTBIG") == 0)
								fonts[nFonts].font = fontbig;
							else if (strcmp(fontname, "FONTTINY") == 0)
								fonts[nFonts].font = fontiny;
							else if (strcmp(fontname, "FONTMN1") == 0)
								fonts[nFonts].font = fontmn1;
							else if (strcmp(fontname, "FONTMN2") == 0)
								fonts[nFonts].font = fontmn2;
							else fonts[nFonts].font = font2;

							nFonts++;

							delete[] fontname;

						}

						break;

					case ESceneTextPosition:

						textPosX = f->loadShort();
						textPosY = f->loadShort();
						LOG_MAX("TextPosition (x, y): %d, %d", textPosX, textPosY);

						break;

					case ESceneTextColour:

						{

							LOG_MAX("ESceneTextColour: %x", f->loadShort());

						}

						break;

					case ESceneFontFun:

						{

							LOG_MAX("ESceneFontFun len: %d", f->loadShort());

							/*while (len) {

								unsigned char data = f->loadChar();
								len--;

							}*/

						}

						break;

					case ESceneFontIndex:

						textFont = f->loadShort();
						LOG_MAX("ESceneFontIndex: %d", textFont);

						break;

					case ESceneTextVAdjust:

						extraheight = f->loadShort();
						LOG_MAX("ESceneTextVAdjust: %d", extraheight);

						break;

					case ESceneBackgroundFade:

						{
							pages[loop].backgroundFade =  f->loadShort();
							LOG_MAX("ESceneBackgroundFade: %d", pages[loop].backgroundFade);
						}

						break;

					case ESceneTextShadow:

						{
							char enableShadow = f->loadChar();
							if(enableShadow) {
								textShadow = f->loadChar();
							}
							else
								{
								f->loadChar(); // Skip this value since shadows are turned off
								textShadow = -1; // Turn off shadow , -1 means no shadow colour
								}

							LOG_MAX("ESceneTextShadow: %d", textShadow);
						}

						break;

					case ESceneTextAlign:

						textAlignment = f->loadChar();
						LOG_MAX("ESceneTextAlign: %d", textAlignment);

						break;

					case ESceneTextAlign2:

						{
							char a = f->loadChar();
							int b = f->loadShort();
							LOG_MAX("ESceneTextAlign2 (a, b): %x, %x", a, b);

						}

						break;

					case ESceneTextSomething:

						{

							char a = f->loadChar();
							int b = f->loadShort();
							LOG_MAX("ESceneTextSomething (a, b): %x, %x", a, b);

						}

						break;

					case ESceneTextLine:
					case ESceneTextBlock:

						{
							unsigned char datalen = f->loadChar();
							LOG_MAX("Text len: %d", datalen);

							JJ1SceneText *text = pages[loop].texts + pages[loop].nTexts;

							if (datalen > 0) {

								text->text = f->loadBlock(datalen + 1);
								f->seek(-1, false);

								// Convert number placeholders
								for (int textPos = 1; textPos < datalen; textPos++) {

									if (text->text[textPos] == 0x8B) {

										if (loop >= 9)
											text->text[textPos - 1] = ((loop + 1) / 10) + 53;

										text->text[textPos] = ((loop + 1) % 10) + 53;

									} else if (text->text[textPos] == 0x8A) {

										if (scriptItems >= 10)
											text->text[textPos - 1] = (scriptItems / 10) + 53;

										text->text[textPos] = (scriptItems % 10) + 53;

									}

								}

								text->text[datalen] = 0;

							} else {

								text->text = new unsigned char[1];
								text->text[0] = 0;

							}

							text->alignment = textAlignment;
							text->fontId = textFont;
							text->shadowColour = textShadow;

							if(textPosX != -1) {

								text->x = textPosX;
								text->y = textPosY;
								textPosX = -1;
								textPosY = -1;

							}

							if(textRectValid) {

								text->textRect = textRect;
								textRectValid = false;

							}

							if(extraheight != -1) {

								text->extraLineHeight = extraheight;
								extraheight = -1;

							}

							pages[loop].nTexts++;

						}

						break;

					case ESceneTime:

						pages[loop].pageTime = (f->loadShort());
						LOG_MAX("Scene time: %d", pages[loop].pageTime);
						pages[loop].pageTime&=255;
						break;

					case ESceneBreaker:
					case 0x3e:

						pos = f->tell();
						LOG_MAX("Parse script end at position: %d, with: %x", pos, type);
						breakloop = true;
						f->loadChar();

						break;

					default:

						pos = f->tell();
						LOG_MAX("Parse script end at position: %d, breaker: %x", pos, type);
						breakloop = true;

						break;

				}

				pos = f->tell();

			}

		}

	}

}
