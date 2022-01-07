
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

#include <string.h>


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
	unsigned char header;
	bool trans = true;

	/*FILE* out = fopen("c:\\output.dat", "wb");
	fwrite(frameData, size, 1, out);
	fclose(out);*/

	while ((nextData < frameData + size) && (nextPixel < pixels + (SW * SH))) {
		header = *nextData;
		nextData++;
		LOG("PL FF frame header", header);

		if ((header & 0x7F) == 0x7F) {

			fillWidth = loadShortMem(&nextData);

			if (trans) fillWidth += 255;

			LOG("PL FF 0x7f skip", fillWidth);

		} else if (header) {

			if(trans) {
				fillWidth = header;
				LOG("PL FF SKIP bytes", fillWidth);
			}
			else {
				fillWidth = header & 0x1F;

				switch (header & 0x60) {
				default:
					break;

				case 0x00:
					LOG("PL FF 0x00 Copy bytes", header);
					memcpy(nextPixel, nextData, fillWidth);
					nextData += fillWidth;
					break;

				case 0x20:
					LOG("PL FF 0x20 copy previous line op", fillWidth);
					if (nextPixel - 320 >= pixels) memcpy(nextPixel, nextPixel - 320, fillWidth);
					break;

				case 0x40:
					LOG("PL FF 0x40 fillWidth", fillWidth);
					memset(nextPixel, *nextData, fillWidth);
					nextData++;
					break;

				case 0x60:
					LOG("PL FF 0x60 header", header);
					fillWidth = header&0x3F;
					memset(nextPixel, *nextData, fillWidth);
					nextData++;
					break;
				}
			}
		} else {
			LOG("PL FF FAULTY END OF STREAM", size);
			return;
		}

		nextPixel += fillWidth;

		if (header & 0x80) trans = false;
		else trans = !trans;
	}

	LOG("PL FF pixels", nextPixel - pixels);

}


/*
 * $0x $...	Next x + 1 bytes are 'literals'; each byte colors 1 column (Max val $3F)
 * $4x $yy        Next x + 1 columns drawn in color yy (Max value $7E)
 * $7F $xxxx $yy	Next xxxx columns colored with color yy
 * $8x		Next x + 1 pixels are skipped, they're already the right color (Max val $FE)
 * $FF $xxxx	Skip next xxxx pixels of picture, they're already the right color
 */
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
	int fillWidth = 0;
	unsigned char header;

	while (size > 0) {

		header = *frameData;
		frameData++;

		if (header == 0x7F) {

			fillWidth = loadShortMem(&frameData);
			unsigned char fillColor = *frameData;
			frameData++;

			fillstart = nextPixel;

			while (fillstart + fillWidth <= endpixdata) {

				memset(fillstart, fillColor, fillWidth);
				fillstart += SW;

			}

			size -= 3;

		} else if (header == 0xFF) {

			fillWidth = loadShortMem(&frameData);
			size -= 2;

		} else if (header & 0x80) {

			fillWidth = (header - 0x80) + 1;

		} else if (header & 0x40) {

			unsigned char fillColor = *frameData;
			frameData++;
			fillWidth = (header - 0x40) + 1;

			fillstart = nextPixel;

			while (fillstart + fillWidth <= endpixdata) {

				memset(fillstart, fillColor, fillWidth);
				fillstart += SW;

			}

			size--;

		} else {

			fillWidth = (header & 0x3F) + 1;
			unsigned char color;

			for (int col = 0; col < fillWidth; col++) {

				color = *frameData;
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

	LOG("PL Compacts pixels", nextPixel - pixels);

}


/**
 * Load JJ1 cutscene animation.
 *
 * @param f File from which to load animation
 * @param dataIndex Index
 */
void JJ1Scene::loadAni (File *f, int dataIndex) {

	LOGRESULT("ParseAni DataLen", f->loadShort()); // should be 0x02
	LOGRESULT("ParseAni Frames?", f->loadShort()); // unknown, number of frames?
	unsigned short int type = 0;//
	int loop;

	while (type != EPlayListAniHeader) {

		type = f->loadShort();

		if (type == ESoundListAniHeader) { // SL

			/*unsigned short int offset =*/ f->loadShort();
			unsigned char nSounds = f->loadChar();

			for(loop = 0; loop < nSounds; loop++) {

				char* soundName = f->loadString();
				LOG("Soundname ", soundName);
				resampleSound(loop, soundName, 11025);
				delete[] soundName;

			}

		} else if (type == EPlayListAniHeader) {// PL
			int nextPos = f->tell();
			LOG("PL Read position", nextPos);
			f->loadShort(); // Length

			palettes = new JJ1ScenePalette(palettes);

			f->loadPalette(palettes->palette, false);

			palettes->id = dataIndex;

			unsigned short int value = 0;
			int items = 0;
			int validValue = true;

			LOG("PL Read position start", f->tell());

			while (validValue) {

				value = f->loadShort();
				LOG("PL Read block start tag", value);
			    int size = f->loadShort();
				LOG("PL Anim block size", size);
				nextPos = f->tell();
				// next pos is intial position + size and four bytes header
				nextPos += size;

				switch (value) {

					case E_EHeader: // END MARKER

						validValue = false;

						break;

					case E11AniHeader: //11

						// Skip back size header, this is read by the surface reader
						LOG("PL 11 Background Type", 0);
						f->seek(-2, false);

						animations->background = f->loadSurface(SW, SH);

						// Use the most recently loaded palette
						video.setPalette(palettes->palette);

						break;

					case E1LAniHeader: {
						LOG("PL 1L Background Type", 0);
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
						break;
					case ERRAniHeader: // Reverse animation when end found
						animations->reverseAnimation = 1;
						break;

					case ERCAniHeader:
						{
						unsigned char* blockData = f->loadBlock(size);
						animations->addFrame(ERCAniHeader, blockData, size);
						}break;
					case ESquareAniHeader: // Full screen animation frame, that does n't clear the screen first.

						{
							unsigned char* blockData = f->loadBlock(size);
							animations->addFrame(ESquareAniHeader, blockData, size);
						}

						break;

					case ESTAniHeader: // Sound item

						{
							unsigned char soundIndex = f->loadChar();
							animations->lastFrame->soundId = soundIndex;
							LOG("PL Audio tag with index", soundIndex);
							LOGRESULT("PL Audio tag play at ", f->loadChar());
							LOGRESULT("PL Audio tag play offset ", f->loadChar());
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

						}

						break;

					default:

						//LOG("PL Read Unknown type", value);
						validValue = false;

						break;

				}

				LOG("PL Read position after block should be", nextPos);
				f->seek(nextPos, true);

				if(validValue) items++;

			}

			LOG("PL Parsed through number of items skipping 0 items", items);
			LOG("PL Read position after parsing anim blocks", f->tell());

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
		LOG("Data dataLen", dataLen);
		// AN

		if (dataLen == 0x4e41) {

			LOG("Data Type", "ANI");
			animations = new JJ1SceneAnimation(animations);
			animations->id = loop;
			loadAni(f, loop);

		} else {

			unsigned char type = f->loadChar();
			LOG("Data Type", type);

			switch (type) {

				case 3:
				case 4: // image
				case 5:
				case 6:

					{

						LOG("Data Type", "Image");
						LOG("Data Type Image index", loop);
						unsigned short int width = f->loadShort(SW); // get width
						unsigned short int height;

						if (type == 3) height = f->loadChar(); // Get height
						else height = f->loadShort(SH); // Get height

						f->seek(-2, false);
						images = new JJ1SceneImage(images);
						images->image = f->loadSurface(width, height);
						images->id = loop;

					}

					break;

				default:

					LOG("Data Type", "Palette");
					LOG("Data Type Palette index", loop);
					f->seek(-3, false);

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

	    LOG("\nParse Script", loop);
	    int textPosX = -1;
	    int textPosY = -1;

	    int extraheight = -1;
	    SDL_Rect textRect = { 0,0,0,0 };
	    bool textRectValid = false;
		f->seek(scriptStarts[loop], true); // Seek to data start

		if (f->loadChar() == 0x50) { // Script tag

			LOGRESULT("Script id", f->loadShort());
			int palette = f->loadShort();
			LOG("Script default palette", palette);
			pages[loop].paletteIndex = palette;

			unsigned char type = 0;
			bool breakloop = false;
			int pos = f->tell();

			while(!breakloop && pos < dataOffsets[0]) {

				type = f->loadChar();

				switch(type) {

					case ESceneYesNo:
						{
						pages[loop].askForYesNo = 1;
						LOG("ESceneYesNo", 1);
						}break;
					case ESceneStopMusic:
						{
						pages[loop].stopMusic = 1;
						LOG("ESceneStopMusic", 1);
						}break;
					case ESceneAnimation:
						{
							pages[loop].animLoops = f->loadInt();
							pages[loop].animSpeed = f->loadShort();
							pages[loop].animIndex = f->loadShort();

							LOG("ESceneAnimation loops", pages[loop].animLoops);
							LOG("ESceneAnimation speed", pages[loop].animSpeed);
							LOG("ESceneAnimation anim num", pages[loop].animIndex);
						}
						break;

					case ESceneAnimationPlayAndContinue:
						{
						pages[loop].nextPageAfterAnim = f->loadChar();
						LOG("ESceneAnimationPlayAndContinue", pages[loop].nextPageAfterAnim);
						}

						break;

					case ESceneFadeType:

						{
							LOGRESULT("ESceneFadeType", f->loadChar());

						}

						break;

					case ESceneBackground:

						pages[loop].bgX[pages[loop].backgrounds] = f->loadShort();
						pages[loop].bgY[pages[loop].backgrounds] = f->loadShort();
						pages[loop].bgIndex[pages[loop].backgrounds] = f->loadShort();

						LOG("ESceneBackground: xpos", pages[loop].bgX[pages[loop].backgrounds]);
						LOG("ESceneBackground: ypos", pages[loop].bgY[pages[loop].backgrounds]);
						LOG("ESceneBackground: index", pages[loop].bgIndex[pages[loop].backgrounds]);

						pages[loop].backgrounds++;

						break;

					case ESceneMusic:

						// Music file name
						pages[loop].musicFile = f->loadString();
						LOG("ESceneMusic", pages[loop].musicFile);

						break;

					case ESceneSomethingElse:

						{

							LOG("ESceneSomethingElse", 0);

						}

						break;

					case ESceneTextRect: // String

						textRect.x = f->loadShort();
						textRect.y = f->loadShort();
						textRect.w = f->loadShort() - textRect.x;
						textRect.h = f->loadShort() - textRect.y;
						textRectValid = true;
						LOG("Text rectangle xpos", textRect.x);
						LOG("Text rectangle ypos", textRect.y);
						LOG("Text rectangle w", textRect.w);
						LOG("Text rectangle h", textRect.h);

						break;

					case ESceneFontDefine: // Font defnition

						if (nFonts < 5) {

							fonts[nFonts].id = f->loadShort();
							char *fontname = f->loadString();

							LOG("ESceneFontDefine", fontname);
							LOG("ESceneFontDefine with id", fonts[nFonts].id);

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
						LOG("TextPosition x", textPosX);
						LOG("TextPosition y", textPosY);

						break;

					case ESceneTextColour:

						{

							LOGRESULT("ESceneTextColour", f->loadShort());

						}

						break;

					case ESceneFontFun:

						{

							LOGRESULT("ESceneFontFun len", f->loadShort());

							/*while (len) {

								unsigned char data = f->loadChar();
								len--;

							}*/

						}

						break;

					case ESceneFontIndex:

						textFont = f->loadShort();
						LOG("ESceneFontIndex", textFont);

						break;

					case ESceneTextVAdjust:

						extraheight = f->loadShort();
						LOG("ESceneTextVAdjust", extraheight);

						break;

					case ESceneBackgroundFade:

						{
							pages[loop].backgroundFade =  f->loadShort();
							LOG("ESceneBackgroundFade", pages[loop].backgroundFade);
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

							LOG("ESceneTextShadow", textShadow);
						}

						break;

					case ESceneTextAlign:

						textAlignment = f->loadChar();
						LOG("ESceneTextAlign", textAlignment);

						break;

					case ESceneTextAlign2:

						{

							LOGRESULT("ESceneTextAlign2 a", f->loadChar());
							LOGRESULT("ESceneTextAlign2 b", f->loadShort());

						}

						break;

					case ESceneTextSomething:

						{

							LOGRESULT("ESceneTextSomething a", f->loadChar());
							LOGRESULT("ESceneTextSomething b", f->loadShort());

						}

						break;

					case ESceneTextLine:
					case ESceneTextBlock:

						{
							unsigned char datalen = f->loadChar();
							LOG("Text len", datalen);

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
						LOG("Scene time", pages[loop].pageTime);
						pages[loop].pageTime&=255;
						break;

					case ESceneBreaker:
					case 0x3e:

						pos = f->tell();
						LOG("Parse script end at position", pos);
						LOG("Parse script end with", type);
						breakloop = true;
						f->loadChar();

						break;

					default:

						pos = f->tell();
						LOG("Parse script end at position", pos);
						LOG("Parse script breaker", type);
						breakloop = true;

						break;

				}

				pos = f->tell();

			}

		}

	}

}
