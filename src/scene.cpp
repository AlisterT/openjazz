
/*
 *
 * scene.cpp
 *
 * 23rd August 2005: Created scene.c
 * 3rd February 2009: Created scene.h from parts of scene.c
 * 3rd February 2009: Renamed scene.c to scene.cpp
 *
 * Part of the OpenJazz project
 *
 *
 * Copyright (c) 2005-2009 Alister Thomson
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
 * Deals with the loading, displaying and freeing of the cutscenes.
 *
 */


#include "scene.h"

#include "io/controls.h"
#include "io/file.h"
#include "io/gfx/video.h"
#include "io/sound.h"


enum
{
	ESceneMusic = 0x2A,
	ESceneFadeType = 0x3F,
	ESceneFontIndex = 0x46,
	ESceneTextBlock = 0x40,
	ESceneTextColour = 0x41,
	ESceneTextPosition = 0x47,
	ESceneTextAlign = 0x4A,
	ESceneBackground =0x4c,
	ESceneTextRect = 0x57,
	ESceneFontDefine = 0x58,
	ESceneLength = 0x5d,
	ESceneTextLine = 0x5e,
	ESceneTextVAdjust = 0x5f,
	ESceneAnimationIndex = 0xA7,
	ESceneAnimationSetting = 0xA6,
	ESceneTextShadow = 0xdb
};

void Scene::ParseAni(File* f) {
	unsigned short int aniType = f->loadShort();// should be 0x20
	unsigned short int aniOffset = f->loadShort();// unknown
	unsigned short int type = 0;//
	int loop;

	while(type != 0x4C50) {
		type = f->loadShort();
		
		if(type == 0x4C53) { // SL 
			unsigned short int offset = f->loadShort();
			unsigned char noSounds = f->loadChar();
			for(loop = 0;loop<noSounds;loop++) {
				char* soundName = f->loadString();
				log("Soundname %s", soundName);
				free(soundName);
			}
		}
		else if(type == 0x4C50) // PL
		{
			unsigned short int len = f->loadShort();
			unsigned char* buffer = f->loadBlock(len);
			for (int count = 0; count < 256; count++) {
				
				// Palette entries are 6-bit
				// Shift them upwards to 8-bit, and fill in the lower 2 bits
				scenePalette[count].r = (buffer[count * 3] << 2) + (buffer[count * 3] >> 4);
				scenePalette[count].g = (buffer[(count * 3) + 1] << 2) +
					(buffer[(count * 3) + 1] >> 4);
				scenePalette[count].b = (buffer[(count * 3) + 2] << 2) +
					(buffer[(count * 3) + 2] >> 4);
				
			}

			delete[] buffer;
		
			int pos = f->tell();		
			unsigned short int value = f->loadShort(); 
			sceneBGs[imageIndex] = f->loadSurface(320, 200, true);
			imageIndex++;			
			pos = f->tell();
		}
	}
}

void Scene::ParseData(File *f) {
	int loop;
	for(loop = 0;loop < dataItems; loop++) {
		f->seek(dataOffsets[loop], true); // Seek to data start
		unsigned short int dataLen = f->loadShort(); // Get get the length of the datablock		
		if(dataLen == 0x4e41) {
			ParseAni(f);			
		}
		else
		{
			unsigned char type = f->loadChar();
			log("Type:", type);
			switch(type)
			{	
			case 3: 
			case 4: // image
			case 5:
				{
					unsigned short int width = f->loadShort(); // get width
					unsigned short int height;
					if(type == 3)
						height = f->loadChar(); // Get height
					else
						height = f->loadShort(); // Get height
					if(imageIndex<100)
					{
						f->seek(-2, false);
						sceneBGs[imageIndex] = f->loadSurface(width, height);
						imageIndex++;
					}
				}break;
				
			default:
				{
					
					f->seek(-3, false);
					f->loadPalette(scenePalette);				
					usePalette(scenePalette);					
				}
				break;
			}
		}
	}
}


void Scene::ParseScripts(File *f) {
	int loop;
	char *string;

	for(loop = 0;loop < scriptItems; loop++)
	{
		f->seek(scriptStarts[loop], true); // Seek to data start
		if(f->loadChar() == 0x50) // Script tag
		{
			signed long int scriptid = f->loadInt();
			unsigned char type = 0;
			bool breakloop = false;
			while(!breakloop)
			{
				type = f->loadChar();
				switch(type)
				{
				case ESceneAnimationSetting:
					{
						signed long int something = f->loadInt();
						signed long int something2 = f->loadInt();
					}break;
				case ESceneAnimationIndex:
					{
						unsigned char aniIndex = f->loadChar();
					}break;
				case ESceneFadeType:
					{
						unsigned char fadein = f->loadChar();
					}break;
				case ESceneBackground:
					{
						signed long int something = f->loadInt();
						unsigned short bgImageIndex = f->loadShort();
					}break;
				case ESceneMusic:
					{						
						// Music file name
						string = f->loadString();
						playMusic(string);
						delete[] string;
					}break;
				case ESceneTextRect: // String
					{
						unsigned short x = f->loadShort();
						unsigned short y = f->loadShort();
						unsigned short w = f->loadShort();
						unsigned short h = f->loadShort();
						log("Text rectangle xpos:", x);
					}break;
				case ESceneFontDefine: // Font defnition
					{
						unsigned short fontindex = f->loadShort();
						char* fontname = f->loadString();
						log("Fontname", fontname);
						free(fontname);
					}break;
				case ESceneTextPosition:
					{
						unsigned short newx = f->loadShort();
						unsigned short newy = f->loadShort();
						log("TextPosition x*y", newx*newy);
					}
					break;
				case ESceneTextColour:
				case ESceneFontIndex:
				case ESceneTextVAdjust:
				case ESceneTextShadow:
					{
						unsigned short value = f->loadShort();
					}break;
				case ESceneTextAlign:
					{
						unsigned char alignment = f->loadChar();
					}break;
				case ESceneTextLine:
				case ESceneTextBlock:
					{
						unsigned char datalen = f->loadChar();
						unsigned char* block = f->loadBlock(datalen);
						unsigned char pos = 0;
						// Convert to ascii
						while(datalen>0)
						{
							if(block[pos]>='a'-70 && block[pos]<='z'-70)
							{
								block[pos]+=70;
							}
							else if(block[pos]>='A'-64 && block[pos]<='Z'-64)
							{
								block[pos]+=64;
							}
							else if(block[pos]>='0'+5 && block[pos]<='9'+5)
							{
								block[pos]-=5;
							}
							else if(block[pos] == 0x7f)
							{
								block[pos]=' ';
							}
							else if(block[pos] == 0x73)
							{
								block[pos]='\'';
							}
							else if(block[pos] == 'r')
							{
								block[pos]=':';
							}
							else if(block[pos] == 'o')
							{
								block[pos]='(';
							}
							else if(block[pos] == 'p')
							{
								block[pos]=')';
							}
							else if(block[pos] == 0x6b)
							{
								block[pos]='!';
							}
							else if(block[pos] == 0x68)
							{
								block[pos]='-';
							}
							else if(block[pos] == 0x67)
							{
								block[pos]='?';
							}						
							else if(block[pos] == 0x66)
							{
								block[pos]='.';
							}
							else if(block[pos] == 0x65)
							{
								block[pos]=',';
							}
							else if(block[pos] == '?')
							{
								block[pos]='$';
							}						
							
							pos++;
							datalen--;
						}						
					}break;
				case ESceneLength:
					unsigned short int sceneLength = f->loadShort();
					break;	
				case 0x3e:					
				case 0x51:
					breakloop = true;
					f->loadChar();
					break;			
				default:
					{
						breakloop = true;												
					}
					break;
				}
			}
		}
	}

}

Scene::Scene (char * fileName) {

	File *file;
	char *string;
	int type;
    int loop;

	try {

		file = new File(fileName, false);

	} catch (int e) {

		throw e;

	}
	imageIndex = 0;
	
	file->seek(0x13, true); // Skip Digital Dimensions header
	signed long int dataOffset = file->loadInt(); //get offset pointer to first data block
	scriptItems = file->loadShort(); // Get number of script items
	scriptStarts = new signed long int[scriptItems];
	
	for(loop = 0;loop < scriptItems; loop++)
	{
		scriptStarts[loop] = file->loadInt();// Load offset to script
	}

	// Seek to datastart now
	file->seek(dataOffset, true); // Seek to data offsets
	dataItems = file->loadShort()+1; // Get number of data items
	dataOffsets = new signed long int[dataItems];
	for(loop = 0;loop < dataItems; loop++)
	{
		dataOffsets[loop] = file->loadInt();// Load offset to script
	}

	ParseData(file);
	ParseScripts(file);
	delete []scriptStarts;
	delete []dataOffsets;
/*
	// Skip to files
	file->seek(25, true);
	file->seek(file->loadChar(), true);

// At this point, next bytes should be 0x50 0x01 0x00 0x00 0x00
// Then, (0x3f 0x02)
//    OR (Nothing)
// Then, (0x2a
//        Then the length of the music file name
//        Then the bytes of the music file name)
//    OR (0x3f, then another byte)
//    OR (0x4c, not followed by any font stuff)
//    OR (0xa6, then 20 bytes?)
//    OR (Nothing)
// Then 0x58 0x01 0x00
// Then the length of a font file name
// Then a font file name
// Then 0x58 0x02 0x00
// Then the length of a font file name
// Then a font file name

	file->seek(5, false);
	type = file->loadChar();

	while (type == 0x3f) {

		file->seek(1, false);
		type = file->loadChar();

	}

	if (type != 0x4C) {

		if (type == 0x2A) {

			// Music file name
			string = file->loadString();
			playMusic(string);
			delete[] string;

		} else if (type == 0x63) {

			file->seek(1, false);

		} else if (type == 0xA6) {

			file->seek(20, false);

		} else file->seek(-1, false); // type should be 58

		while (file->loadChar() == 0x58) {

			// Font names (file names minus extensions)
			file->seek(2, false);
			string = file->loadString();

			// Do something with this

			delete[] string;

		}

	}

	file->seek(-1, false);

	while (file->loadChar() == 0x3f) {

		file->seek(1, false);

	}

	file->seek(-1, false);

// Then 0x4c 0x00 0x00 0x00 0x00 0x01 0x00
// Then, (0x46
//        Then a small number, e.g. 0x01, 0x02
//        Then 0x00 0x4a
//        Then (0x02 0x5d)
//          OR (0x01 0xdb)
//    OR (0x57 0x14...)

//	log("Initial search reached", file->tell());

	// Skip to the palette
	file->seek(23, true);
	type = file->loadChar();

	file->seek(19, true);

	file->skipRLE();

	file->seek((type * 4) - 11, false);

	// Load the palette

//	log("Palette", file->tell());

	file->loadPalette(scenePalette);

	usePalette(scenePalette);

	file->seek(4, false);

//	log("Pixels", file->tell());

	sceneBGs[0] = file->loadSurface(320, 200);
*/
	delete file;

	return;

}


Scene::~Scene () {

	SDL_FreeSurface(sceneBGs[0]); // Temporary

	return;

}



int Scene::play () {

	SDL_Rect dst;
	unsigned int sceneIndex = 0;
	while (true) {

		if (loop(NORMAL_LOOP) == E_QUIT) return E_QUIT;

		if (controls.release(C_ESCAPE)) {
									
			return E_NONE;

		}

		if(controls.release(C_ENTER))
		{			
			sceneIndex++;
			if(sceneIndex == scriptItems)
			{
				return E_NONE;
			}
		}

		// Temporary stuff
		clearScreen(BLACK);
		dst.x = (screenW - 320) >> 1;
		dst.y = (screenH - 200) >> 1;
		if(sceneBGs[0])
			SDL_BlitSurface(sceneBGs[0], NULL, screen, &dst);
		SDL_Delay(T_FRAME);

	}

	return E_NONE;

}


