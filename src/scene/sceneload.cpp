
/*
 *
 * sceneload.cpp
 *
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
 * Deals with the loading of cutscene data.
 *
 */


#include "scene/scene.h"

#include "io/file.h"
#include "io/gfx/font.h"
#include "io/gfx/video.h"

#include <string.h>


void Scene::loadAni (File *f, int dataIndex) {

	unsigned short int aniType = f->loadShort();// should be 0x02
	LOG("ParseAni DataLen", aniType);
	unsigned short int aniOffset = f->loadShort();// unknown, number of frames?
	LOG("ParseAni Frames?", aniOffset);
	unsigned short int type = 0;//
	int loop;

	while (type != 0x4C50) {

		type = f->loadShort();

		if (type == 0x4C53) { // SL

			/*unsigned short int offset =*/ f->loadShort();
			unsigned char noSounds = f->loadChar();

			for(loop = 0;loop<noSounds;loop++) {

				char* soundName = f->loadString();
				LOG("Soundname ", soundName);
				delete[] soundName;

			}

		} else if (type == 0x4C50) {// PL

			int pos = f->tell();
			int nextPos = f->tell();
			LOG("PL Read position", pos);
			unsigned short int len = f->loadShort();
			unsigned char* buffer = f->loadBlock(len);

			for (int count = 0; count < 256; count++) {

				// Palette entries are 6-bit
				// Shift them upwards to 8-bit, and fill in the lower 2 bits
				paletteInfos[paletteIndex].palette[count].r =
					(buffer[count * 3] << 2) + (buffer[count * 3] >> 4);
				paletteInfos[paletteIndex].palette[count].g =
					(buffer[(count * 3) + 1] << 2) + (buffer[(count * 3) + 1] >> 4);
				paletteInfos[paletteIndex].palette[count].b =
					(buffer[(count * 3) + 2] << 2) + (buffer[(count * 3) + 2] >> 4);


			}

			delete[] buffer;
			paletteInfos[paletteIndex].dataIndex = dataIndex;
			paletteIndex++;

			unsigned short int value = 0x4646;
			int items = 0;
			int validValue = true;
			pos = f->tell();

			LOG("PL Read position start", pos);

			while (validValue) {

				value = f->loadShort();
				LOG("PL Read block start tag", value);
				unsigned short int size= f->loadShort();
				LOG("PL Anim block size", size);
				nextPos = f->tell();
				// next pos is intial position + size and four bytes header
				nextPos+=(size);

				switch (value) {

					case 0x455F:

						validValue = false;

						break;

					case 0x3131:

						{

							// Skip back size header, this is read by the surface reader
							f->seek(-2, false);

							SDL_Surface *image = f->loadSurface(320, 200, true);
							SDL_BlitSurface(image, NULL, canvas, NULL);
							SDL_FreeSurface(image);
							SDL_SetPalette(screen, SDL_PHYSPAL, paletteInfos[paletteIndex-1].palette, 0, 256);
							currentPalette = paletteInfos[paletteIndex-1].palette;

						}

						break;

					case 0x4c31:

						{

							int longvalue = f->loadInt();
							LOG("PL Anim block value", longvalue);
							// Skip back size header, this is read by the surface reader
							//f->seek(-2, false);

							while (size) {

								size--;
								unsigned char header = f->loadChar();
								LOG("PL 4c31 block header", header);

								switch (header) {

									case 0x7F:

										{

											unsigned short fillWidth = f->loadShort();
											unsigned char fillColor = f->loadChar();
											LOG("PL Fillblock width", fillWidth);
											LOG("PL Fillblock with color", fillColor);
											size -= 3;

										}

										break;

									case 0xff:

										{

											unsigned char x = f->loadChar();
											unsigned char y = f->loadChar();
											LOG("PL block x", x);
											LOG("PL block y", y);
											size -= 2;

										}

										break;

									default:

										LOG("PL Unknown type", header);
										break;

								}

							}

						}

						break;

					case 0x4646:

						while (size) {

							unsigned char header = f->loadChar();
							LOG("PL 4646 block header", header);

							switch (header) {

								case 0x7F:

									{

										unsigned short fillWidth = f->loadShort();
										unsigned char fillColor = f->loadChar();
										LOG("PL Fillblock width", fillWidth);
										LOG("PL Fillblock with color", fillColor);
										size -= 3;

									}

									break;

								case 0xff:

									{

										unsigned char x = f->loadChar();
										unsigned char y = f->loadChar();
										LOG("PL block x", x);
										LOG("PL block y", y);
										size -= 2;

									}

									break;

								default:

									LOG("PL Unknown type", header);

									break;

							}

							size--;

						}

						break;

					case 0x4e52:
					case 0x4252:
					case 0x4352:
					case 0x4c52:
					case 0x4e41:
					case 0x584d:
					case 0x5252:

						break;

					case 0x5b5d:

						{

							unsigned char header = f->loadChar();
							LOG("PL 5b5d block header", header);
							unsigned short int value = f->loadShort();
							LOG("PL 5b5d block value", value);

						}

						break;

					case 0x5453:

						{

							unsigned char soundIndex = f->loadChar();
							unsigned char soundNote = f->loadChar();
							unsigned char soundOffset = f->loadChar();
							LOG("PL Audio tag with index", soundIndex);
							LOG("PL Audio tag play at ", soundNote);
							LOG("PL Audio tag play offset ", soundOffset);

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

						LOG("PL Read Unknown type", value);
						validValue = false;

						break;

				}

				pos = f->tell();
				LOG("PL Read position after block should be", nextPos);
				f->seek(nextPos, true);

				if(validValue) items++;

			}

			LOG("PL Parsed through number of items skipping 0 items", items);
			pos = f->tell();
			LOG("PL Read position after parsing anim blocks", pos);

		}

	}

}


void Scene::loadData (File *f) {

	int loop;

	for(loop = 0; loop < dataItems; loop++) {

		f->seek(dataOffsets[loop], true); // Seek to data start
		unsigned short int dataLen = f->loadShort(); // Get get the length of the datablock
		LOG("Data dataLen", dataLen);
		// AN

		if (dataLen == 0x4e41) {

			LOG("Data Type", "ANI");
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
						unsigned short int width = f->loadShort(); // get width
						unsigned short int height;

						if (type == 3) height = f->loadChar(); // Get height
						else height = f->loadShort(); // Get height

						if (imageIndex<100) {

							f->seek(-2, false);
							imageInfos[imageIndex].image = f->loadSurface(width, height);
							imageInfos[imageIndex].dataIndex = loop;
							imageIndex++;

						}

					}

					break;

				default:

					{

						LOG("Data Type", "Palette");
						LOG("Data Type Palette index", loop);
						f->seek(-3, false);
						f->loadPalette(paletteInfos[paletteIndex].palette);
						paletteInfos[paletteIndex].dataIndex = loop;
						paletteIndex++;

					}

					break;

			}

		}

	}

}


void Scene::loadScripts (File *f) {

	int loop;
	char *string;
	/*int bgIndex = 0;*/
	int textAlignment = 0;
	int textFont = 0;

	for(loop = 0; loop < scriptItems; loop++) {

	    LOG("\nParse Script", loop);
	    int textPosX = -1;
	    int textPosY = -1;

	    int extraheight = -1;
	    SDL_Rect textRect;
	    bool textRectValid = false;
		f->seek(scriptStarts[loop], true); // Seek to data start

		if (f->loadChar() == 0x50) { // Script tag

			unsigned short int scriptid = f->loadShort();
			LOG("Script id:", scriptid);
			int palette = f->loadShort();
			LOG("Script default palette", palette);
			scriptPages[loop].paletteIndex = palette;

			unsigned char type = 0;
			bool breakloop = false;
			int pos = f->tell();

			while(!breakloop && pos < dataOffsets[0]) {

				type = f->loadChar();

				switch(type) {

					case ESceneAnimationSetting:

						{

							signed long int something = f->loadInt();
							signed long int something2 = f->loadInt();
							LOG("ESceneAnimationSetting1", something);
							LOG("ESceneAnimationSetting2", something2);

						}

						break;

					case ESceneAnimationIndex:

						{
							unsigned char aniIndex = f->loadChar();
							LOG("ESceneAnimationIndex:", aniIndex);

						}

						break;

					case ESceneFadeType:

						{
							unsigned char fadein = f->loadChar();
							LOG("ESceneFadeType:", fadein);

						}

						break;

					case ESceneBackground:

						{

							unsigned short int xpos = f->loadShort();
							unsigned short int ypos = f->loadShort();
							unsigned short bgImageIndex = f->loadShort();
							LOG("ESceneBackground: index", bgImageIndex);
							LOG("ESceneBackground: xpos", xpos);
							LOG("ESceneBackground: ypos", ypos);
							scriptPages[loop].bgIndex[scriptPages[loop].backgrounds] = bgImageIndex;
							scriptPages[loop].bgPos[scriptPages[loop].backgrounds] = xpos|(ypos<<16);
							scriptPages[loop].backgrounds++;

						}

						break;

					case ESceneMusic:

						{

							// Music file name
							string = f->loadString();
							LOG("ESceneMusic: ", string);
							scriptPages[loop].musicfile = createString(string);
							delete[] string;

						}

						break;

					case ESceneSomethingElse:

						{

							unsigned char value = 0;//f->loadChar();
							LOG("ESceneSomethingElse", value);

						}

						break;

					case ESceneTextRect: // String

						{

							unsigned short x = textRect.x = f->loadShort();
							unsigned short y = textRect.y = f->loadShort();
							unsigned short w = textRect.w = f->loadShort() - x;
							unsigned short h = textRect.h = f->loadShort() - y;
							textRectValid = true;
							LOG("Text rectangle xpos:", x);
							LOG("Text rectangle ypos:", y);
							LOG("Text rectangle w:", w);
							LOG("Text rectangle h:", h);

						}

						break;

					case ESceneFontDefine: // Font defnition

						{

							unsigned short fontid = f->loadShort();
							char* fontname = f->loadString();
							LOG("ESceneFontDefine", fontname);
							LOG("ESceneFontDefine with id=", fontid);

							if (strcmp(fontname, "FONT2") == 0)
								scriptFonts[noScriptFonts].font = font2;
							else if (strcmp(fontname, "FONTBIG") == 0)
								scriptFonts[noScriptFonts].font = fontbig;
							else if (strcmp(fontname, "FONTTINY") == 0)
								scriptFonts[noScriptFonts].font = fontiny;
							else if (strcmp(fontname, "FONTMN1") == 0)
								scriptFonts[noScriptFonts].font = fontmn1;
							else if (strcmp(fontname, "FONTMN2") == 0)
								scriptFonts[noScriptFonts].font = fontmn2;
							else scriptFonts[noScriptFonts].font = font2;

							scriptFonts[noScriptFonts].fontId = fontid;
							noScriptFonts++;
							delete[] fontname;

						}

						break;

					case ESceneTextPosition:

						{

							unsigned short newx = f->loadShort();
							unsigned short newy = f->loadShort();
							LOG("TextPosition x", newx);
							LOG("TextPosition y", newy);
							textPosX = newx;
							textPosY = newy;

						}

						break;

					case ESceneTextColour:

						{

							unsigned short value = f->loadShort();
							LOG("ESceneTextColour", value);

						}

						break;

					case ESceneFontFun:

						{

							unsigned short len = f->loadShort();
							LOG("ESceneFontFun len", len);

							/*while (len) {

								unsigned char data = f->loadChar();
								len--;

							}*/

						}

						break;

					case ESceneFontIndex:

						{

							unsigned short value = f->loadShort();
							LOG("ESceneFontIndex", value);
							textFont = value;

						}

						break;

					case ESceneTextVAdjust:

						{

							unsigned short value = f->loadShort();
							LOG("ESceneTextVAdjust", value);
							extraheight = value;

						}

						break;

					case ESceneTextSetting:

						{

							unsigned short value = f->loadShort();
							LOG("ESceneTextSetting", value);

						}

						break;

					case ESceneTextShadow:

						{

							unsigned short value = f->loadShort();
							LOG("ESceneTextVAdjust", value);

						}

						break;

					case ESceneTextAlign:

						{

							unsigned char alignment = f->loadChar();
							LOG("ESceneTextAlign", alignment);
							textAlignment = alignment;

						}

						break;

					case ESceneTextAlign2:

						{

							unsigned char a = f->loadChar();
							unsigned short b = f->loadShort();
							LOG("ESceneTextAlign2 a", a);
							LOG("ESceneTextAlign2 b", b);

						}

						break;

					case ESceneTextSomething:

						{

							unsigned char a = f->loadChar();
							unsigned short b = f->loadShort();
							LOG("ESceneTextSomething a", a);
							LOG("ESceneTextSomething b", b);

						}

						break;

					case ESceneTextLine:
					case ESceneTextBlock:

						{

							unsigned char datalen = f->loadChar();
							LOG("Text len=", datalen);

							if (datalen > 0) {

								f->seek(-1, false);
								char *block = f->loadString();

								// Convert number placeholders
								for (int pos = 1; pos < datalen; pos++) {

									if (block[pos] == -117) {

										if (loop >= 9)
											block[pos - 1] = ((loop + 1) / 10) + 53;

										block[pos] = ((loop + 1) % 10) + 53;

									} else if (block[pos] == -118) {

										if (scriptItems >= 10)
											block[pos - 1] = (scriptItems / 10) + 53;

										block[pos] = (scriptItems % 10) + 53;

									}

								}

								scriptPages[loop].scriptTexts[scriptPages[loop].noScriptTexts].text = block;

								LOG("Text data",(char*) scriptPages[loop].scriptTexts[scriptPages[loop].noScriptTexts].text);

							} else {

								scriptPages[loop].scriptTexts[scriptPages[loop].noScriptTexts].text = new char[1];
								scriptPages[loop].scriptTexts[scriptPages[loop].noScriptTexts].text[0] = 0;

								LOG("Text data", "Empty line");

							}

							scriptPages[loop].scriptTexts[scriptPages[loop].noScriptTexts].alignment = textAlignment;
							scriptPages[loop].scriptTexts[scriptPages[loop].noScriptTexts].fontId = textFont;

							if(textPosX != -1) {

								scriptPages[loop].scriptTexts[scriptPages[loop].noScriptTexts].x = textPosX;
								scriptPages[loop].scriptTexts[scriptPages[loop].noScriptTexts].y = textPosY;
								textPosX = -1;
								textPosY = -1;

							}

							if(textRectValid) {

								scriptPages[loop].scriptTexts[scriptPages[loop].noScriptTexts].textRect = textRect;
								textRectValid = false;

							}

							if(extraheight != -1) {

								scriptPages[loop].scriptTexts[scriptPages[loop].noScriptTexts].extraLineHeight = extraheight;
								extraheight = -1;

							}

							scriptPages[loop].noScriptTexts++;

						}

						break;

					case ESceneTime:

						{
							unsigned short int sceneTime = f->loadShort();
							LOG("Scene time",sceneTime&255);
							scriptPages[loop].pageTime = sceneTime&255;

						}

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
