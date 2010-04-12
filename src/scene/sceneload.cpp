
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

/**
11
1L
/0/0
PB
FF 
RN
RB
RC
RL
RR
][
PL
AN
_E
MX
ST
SL 
*/
enum ANIHeaders
	{
	E11AniHeader = 0x3131, // Background/start image
	E1LAniHeader = 0x4c31,
	EPBAniHeader = 0x4250,
	EFFAniHeader = 0x4646, // Floodfill? or full frame?
	ERNAniHeader = 0x4e52,
	ERBAniHeader = 0x4252,
	ERCAniHeader = 0x4352,
	ERLAniHeader = 0x4c52,
	ERRAniHeader = 0x5252,
	E_EHeader = 0x455F, // ANI End
	ESquareAniHeader = 0x5b5d,
	EMXAniHeader = 0x584d,
	ESTAniHeader = 0x5453, // Sound tag
	ESoundListAniHeader = 0x4C53,
	EPlayListAniHeader = 0x4C50
	};

/*							
 * $0x $...	Next x + 1 bytes are 'literals'; each byte colors 1 column (Max val $3F)									
 * $4x $yy        Next x + 1 columns drawn in color yy (Max value $7E)
 * $7F $xxxx $yy	Next xxxx columns colored with color yy						
 * $8x		Next x + 1 pixels are skipped, they're already the right color (Max val $FE)
 * $FF $xxxx	Skip next xxxx pixels of picture, they're already the right color
 */														
void Scene::LoadCompacted(int& size, File* f, unsigned char* pixdata, int width, int height) {
	int pixels = 0;
	unsigned char* endpixdata = pixdata+width*height;
	unsigned char* fillstart = NULL;
	while (size > 0) {
		unsigned char header = f->loadChar();		
	
		switch (header)  {
			case 0x7F: {
				unsigned short fillWidth = f->loadShort();
				unsigned char fillColor = f->loadChar();
								
				fillstart = pixdata;
				while(fillstart+fillWidth < endpixdata) {
					memset(fillstart, fillColor, fillWidth);
					fillstart+=width;
					}
				
				pixdata+=fillWidth;				
				pixels+=fillWidth;
				size -= 3;
				}	
				break;
	
			case 0xff: {
				unsigned short skip = f->loadShort();				
				pixels+=skip;
				pixdata+=skip;
				size -= 2;
				}
	
				break;
	
			default:
				if(header&0x80) {
					unsigned short skip =(header-0x80)+1;										
					pixels+=skip;
					pixdata+=skip;
					}
				else if(header&0x40) {					
					unsigned char fillColor = f->loadChar();
					unsigned char fillWidth = ((header-0x40)+1);										
					memset(pixdata, fillColor, fillWidth);
					
					fillstart = pixdata;
				
					while(fillstart+fillWidth < endpixdata) {
						memset(fillstart, fillColor, fillWidth);
						fillstart+=width;
						}
					
					pixdata+=fillWidth;
					pixels+=fillWidth;
					size--;
					}
				else {
					int copyWidth = (header & 0x3f)+1;
					unsigned char color;
					
					for(int col = 0;col < copyWidth;col++) {						
						color= f->loadChar();
					
						fillstart = pixdata;
						
						while(fillstart < endpixdata) {
							*fillstart = color;
							fillstart+=width;
							}
						
						pixdata++;
						pixels++;
						size--;
						}
					}			
				break;
		}
	
		size--;
	}
	
	LOG("PL Compacts pixels", pixels);
}

void Scene::loadAni (File *f, int dataIndex) {

	unsigned short int aniType = f->loadShort();// should be 0x02
	LOG("ParseAni DataLen", aniType);
	unsigned short int aniOffset = f->loadShort();// unknown, number of frames?
	LOG("ParseAni Frames?", aniOffset);
	unsigned short int type = 0;//
	int loop;

	while (type != EPlayListAniHeader) {

		type = f->loadShort();

		if (type == ESoundListAniHeader) { // SL

			/*unsigned short int offset =*/ f->loadShort();
			animations->noSounds = f->loadChar();

			for(loop = 0;loop<animations->noSounds;loop++) {

				char* soundName = f->loadString();
				LOG("Soundname ", soundName);
				strcpy(animations->soundNames[loop], soundName);
				delete[] soundName;

			}

		} else if (type == EPlayListAniHeader) {// PL

			int pos = f->tell();
			int nextPos = f->tell();
			LOG("PL Read position", pos);
			unsigned short int len = f->loadShort();
			unsigned char* buffer = f->loadBlock(len);

			palettes = new ScenePalette(palettes);

			for (int count = 0; count < 256; count++) {

				// Palette entries are 6-bit
				// Shift them upwards to 8-bit, and fill in the lower 2 bits
				palettes->palette[count].r = (buffer[count * 3] << 2) + (buffer[count * 3] >> 4);
				palettes->palette[count].g = (buffer[(count * 3) + 1] << 2) + (buffer[(count * 3) + 1] >> 4);
				palettes->palette[count].b = (buffer[(count * 3) + 2] << 2) + (buffer[(count * 3) + 2] >> 4);
			}

			delete[] buffer;
			palettes->id = dataIndex;

			unsigned short int value = 0;
			int items = 0;
			int validValue = true;
			pos = f->tell();

			LOG("PL Read position start", pos);

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
						LOG("PL 11 Background Type", 1);
						f->seek(-2, false);

						animations->background = f->loadSurface(SW, SH);

						// Use the most recently loaded palette
						video.setPalette(palettes->palette);

						break;

					case E1LAniHeader: {
						LOG("PL 1L Background Type", 1);						
						unsigned char* pixels;
						pixels = new unsigned char[SW* SH];							
						memset(pixels, 0, SW*SH);
						LoadCompacted(size, f, pixels, SW, SH);
						animations->background = createSurface(pixels, SW, SH);
						delete[] pixels;
						// Use the most recently loaded palette
						video.setPalette(palettes->palette);					
						}
						break;

					case EFFAniHeader:
						{
						//LoadCompacted(size, f);
						}
						break;

					case ERNAniHeader:
					case ERBAniHeader:
					case ERCAniHeader:
					case ERLAniHeader:					
					case EMXAniHeader:
					case ERRAniHeader:

						break;

					case ESquareAniHeader: // ][ Flip??

						{
							unsigned char header = f->loadChar();
							LOG("PL 5b5d block header", header);
							unsigned short int value = f->loadShort();
							LOG("PL 5b5d block value", value);

						}

						break;

					case ESTAniHeader: // Sound item

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

						//LOG("PL Read Unknown type", value);
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

	for (loop = 0; loop < dataItems; loop++) {

		f->seek(dataOffsets[loop], true); // Seek to data start
		unsigned short int dataLen = f->loadShort(); // Get get the length of the datablock
		LOG("Data dataLen", dataLen);
		// AN

		if (dataLen == EAnimationData) {

			LOG("Data Type", "ANI");
			animations = new SceneAnimation(animations);
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
						unsigned short int width = f->loadShort(); // get width
						unsigned short int height;

						if (type == 3) height = f->loadChar(); // Get height
						else height = f->loadShort(); // Get height

						f->seek(-2, false);
						images = new SceneImage(images);
						images->image = f->loadSurface(width, height);
						images->id = loop;

					}

					break;

				default:

					LOG("Data Type", "Palette");
					LOG("Data Type Palette index", loop);
					f->seek(-3, false);

					palettes = new ScenePalette(palettes);
					f->loadPalette(palettes->palette);
					palettes->id = loop;

					break;

			}

		}

	}

}


void Scene::loadScripts (File *f) {

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
	    SDL_Rect textRect;
	    bool textRectValid = false;
		f->seek(scriptStarts[loop], true); // Seek to data start

		if (f->loadChar() == EScriptStartTag) { // Script tag

			unsigned short int scriptid = f->loadShort();
			LOG("Script id", scriptid);
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
					case ESceneAnimationSetting:
						{
							pages[loop].animLoops = f->loadInt();
							pages[loop].animSpeed = f->loadShort();							
							pages[loop].animIndex = f->loadShort();
							
							LOG("ESceneAnimationSetting loops", pages[loop].animLoops);
							LOG("ESceneAnimationSetting speed", pages[loop].animSpeed);
							LOG("ESceneAnimationSetting anim num", pages[loop].animIndex);							
						}
						break;

					case ESceneAnimationPlayAndContinue:
						{
							unsigned char playAndContinue = f->loadChar();
							LOG("ESceneAnimationPlayAndContinue", playAndContinue);
						}

						break;

					case ESceneFadeType:

						{
							unsigned char fadein = f->loadChar();
							LOG("ESceneFadeType", fadein); 

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

							unsigned char value = 0;//f->loadChar();
							LOG("ESceneSomethingElse", value);

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

						textFont = f->loadShort();
						LOG("ESceneFontIndex", textFont);

						break;

					case ESceneTextVAdjust:

						extraheight = f->loadShort();
						LOG("ESceneTextVAdjust", extraheight);

						break;

					case ESceneTextSetting:

						{

							unsigned short value = f->loadShort();
							LOG("ESceneTextSetting", value);

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
							LOG("Text len", datalen);

							SceneText *text = pages[loop].texts + pages[loop].nTexts;

							if (datalen > 0) {

								text->text = f->loadBlock(datalen + 1);
								f->seek(-1, false);

								// Convert number placeholders
								for (int pos = 1; pos < datalen; pos++) {

									if (text->text[pos] == 0x8B) {

										if (loop >= 9)
											text->text[pos - 1] = ((loop + 1) / 10) + 53;

										text->text[pos] = ((loop + 1) % 10) + 53;

									} else if (text->text[pos] == 0x8A) {

										if (scriptItems >= 10)
											text->text[pos - 1] = (scriptItems / 10) + 53;

										text->text[pos] = (scriptItems % 10) + 53;

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

						pages[loop].pageTime = f->loadShort() & 255;
						LOG("Scene time", pages[loop].pageTime);

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
