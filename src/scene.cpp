
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
#include "io/gfx/font.h"
#include "io/sound.h"

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
// These are the known script types
enum
{
	ESceneMusic = 0x2A,
	ESceneFadeType = 0x3F,	
	ESceneTextBlock = 0x40,
	ESceneTextColour = 0x41,
	ESceneFontFun = 0x45,
	ESceneFontIndex = 0x46,
	ESceneTextPosition = 0x47,
	ESceneTextAlign = 0x4A,
	ESceneTextAlign2 = 0x4B,
	ESceneBackground =0x4c,
	ESceneBreaker = 0x50,
	ESceneSomethingElse = 0x51,
	ESceneTextRect = 0x57,
	ESceneFontDefine = 0x58,
	ESceneTime = 0x5d,
	ESceneTextLine = 0x5e,
	ESceneTextVAdjust = 0x5f,
	ESceneAnimationIndex = 0xA7,
	ESceneAnimationSetting = 0xA6,
	ESceneTextSetting = 0xb1,
	ESceneTextSomething = 0xd9,
	ESceneTextShadow = 0xdb
};

void Scene::ParseAni(File* f) {
	unsigned short int aniType = f->loadShort();// should be 0x02
	log("ParseAni DataLen", aniType);
	unsigned short int aniOffset = f->loadShort();// unknown, number of frames?
	log("ParseAni Frames?", aniOffset);
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
		else if(type == 0x4C50) {// PL		
			int pos = f->tell();
			log("PL Read position", pos);
			unsigned short int len = f->loadShort();
			unsigned char* buffer = f->loadBlock(len);
			for (int count = 0; count < 256; count++) {
				
				// Palette entries are 6-bit
				// Shift them upwards to 8-bit, and fill in the lower 2 bits
				imageInfo[imageIndex].palette[count].r = (buffer[count * 3] << 2) + (buffer[count * 3] >> 4);
				imageInfo[imageIndex].palette[count].g = (buffer[(count * 3) + 1] << 2) +
					(buffer[(count * 3) + 1] >> 4);
				imageInfo[imageIndex].palette[count].b = (buffer[(count * 3) + 2] << 2) +
					(buffer[(count * 3) + 2] >> 4);
				
			}

			delete[] buffer;
		
			pos = f->tell();
			log("PL Read position after", pos);
			unsigned short int value = f->loadChar();
			log("PL Read block start", value);
			imageInfo[imageIndex].image = f->loadSurface(320, 200, true);
			imageInfo[imageIndex].dataIndex = -1;
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
		log("Data dataLen", dataLen);
		// AN
		if(dataLen == 0x4e41) {
			log("Data Type:", "ANI");
			ParseAni(f);			
		}
		else {
			unsigned char type = f->loadChar();
			log("Data Type:", type);
			switch(type)
			{	
			case 3: 
			case 4: // image
			case 5:
				{
					log("Data Type:", "Image");
					unsigned short int width = f->loadShort(); // get width
					unsigned short int height;
					if(type == 3)
						height = f->loadChar(); // Get height
					else
						height = f->loadShort(); // Get height
					if(imageIndex<100) {
						f->seek(-2, false);
						imageInfo[imageIndex].image = f->loadSurface(width, height);
						imageInfo[imageIndex].dataIndex = loop;
						imageIndex++;
					}
				}break;
				
			default:
				{
					
					f->seek(-3, false);
					f->loadPalette(imageInfo[imageIndex].palette);				
					//usePalette(scenePalette[imageIndex]);					
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
	    log("\nParse Script", loop);
		f->seek(scriptStarts[loop], true); // Seek to data start
		if(f->loadChar() == 0x50) { // Script tag		
			unsigned short int scriptid = f->loadShort();
			log("Script id:", scriptid);
			int unknown = f->loadShort();
			log("Script unknown", unknown);
			unsigned char type = 0;
			bool breakloop = false;
			int pos = f->tell();
			while(!breakloop && pos< dataOffsets[0]) {			
				type = f->loadChar();
				switch(type)
				{
				case ESceneAnimationSetting:
					{
						signed long int something = f->loadInt();
						signed long int something2 = f->loadInt();
						log("ESceneAnimationSetting1", something);
						log("ESceneAnimationSetting2", something2);
					}break;
				case ESceneAnimationIndex:
					{
						unsigned char aniIndex = f->loadChar();
						log("ESceneAnimationIndex:", aniIndex);												
					}break;
				case ESceneFadeType:
					{
						unsigned char fadein = f->loadChar();
						log("ESceneFadeType:", fadein);	
					}break;
				case ESceneBackground:
					{
						signed long int something = f->loadInt();
						unsigned short bgImageIndex = f->loadShort();
						log("ESceneBackground: index", bgImageIndex);
						log("ESceneBackground: something", something);
						scriptPages[loop].bgIndex = bgImageIndex;
					}break;
				case ESceneMusic:
					{						
						// Music file name
						string = f->loadString();
						log("ESceneMusic: ", string);
						playMusic(string);
						delete[] string;
					}break;				
				case ESceneSomethingElse:
					{
					unsigned char value = 0;//f->loadChar();
					log("ESceneSomethingElse", value);
					}break;
				case ESceneTextRect: // String
					{
						unsigned short x = f->loadShort();
						unsigned short y = f->loadShort();
						unsigned short w = f->loadShort();
						unsigned short h = f->loadShort();
						log("Text rectangle xpos:", x);
						log("Text rectangle ypos:", y);
						log("Text rectangle w:", w);
						log("Text rectangle h:", h);
					}break;
				case ESceneFontDefine: // Font defnition
					{
						unsigned short fontid = f->loadShort();
						char* fontname = f->loadString();
						log("ESceneFontDefine", fontname);
						log("ESceneFontDefine with id=", fontid);
						if(strcmp(fontname, "FONT2") == 0)
							{
							scriptFonts[noScriptFonts].fontType = EFONT2Type;
							}
						else if(strcmp(fontname, "FONTBIG") == 0)
							{
							scriptFonts[noScriptFonts].fontType = EFONTBIGType;
							}
						else if(strcmp(fontname, "FONTTINY") == 0)
							{
							scriptFonts[noScriptFonts].fontType = EFONTINYType;
							}
						else if(strcmp(fontname, "FONTMN1") == 0)
							{
							scriptFonts[noScriptFonts].fontType = EFONTMN1Type;
							}
						else if(strcmp(fontname, "FONTMN2") == 0)
							{
							scriptFonts[noScriptFonts].fontType = EFONTMN2Type;
							}
						
						scriptFonts[noScriptFonts].fontId = fontid;
						noScriptFonts++;
						free(fontname);
					}break;
				case ESceneTextPosition:
					{
						unsigned short newx = f->loadShort();
						unsigned short newy = f->loadShort();
						log("TextPosition x", newx);
						log("TextPosition y", newy);
						textPosX = newx;
						textPosY = newy;
					}
					break;
				case ESceneTextColour:
					unsigned short value = f->loadShort();
					log("ESceneTextColour", value);
					break;
				case ESceneFontFun:
					{					
					log("ESceneFontFun");
					unsigned short len = f->loadShort();
					log("ESceneFontFun len", len);
					/*while(len)
						{
						unsigned char data = f->loadChar();
						len--;
						}*/
					}break;
				case ESceneFontIndex:
					{
					unsigned short value = f->loadShort();
					log("ESceneFontIndex", value);
					textFont = value;
					}
					break;					
				case ESceneTextVAdjust:
					{
					unsigned short value = f->loadShort();
					log("ESceneTextVAdjust", value);
					}
					break;	
				case ESceneTextSetting:
					{
					unsigned short value = f->loadShort();
					log("ESceneTextSetting", value);
					}break;
				case ESceneTextShadow:
					{
					unsigned short value = f->loadShort();
					log("ESceneTextVAdjust", value);
					}break;
				case ESceneTextAlign:
					{
					unsigned char alignment = f->loadChar();
					log("ESceneTextAlign", alignment);
					textAlignment = alignment;
					}break;
				case ESceneTextAlign2:
					{
					unsigned char a = f->loadChar();
					unsigned short b = f->loadShort();
					log("ESceneTextAlign2 a", a);
					log("ESceneTextAlign2 b", b);
					}break;
				case ESceneTextSomething:
					{
					unsigned char a = f->loadChar();
					unsigned short b = f->loadShort();
					log("ESceneTextSomething a", a);
					log("ESceneTextSomething b", b);
					}break;
				case ESceneTextLine:
				case ESceneTextBlock:
					{						
						unsigned char datalen = f->loadChar();
						unsigned char* block = f->loadBlock(datalen);
						unsigned char pos = 0;
						unsigned char orgdatalen = datalen;
						log("Text len=", datalen);
						// Convert to ascii
						while(datalen>0) {
							if(block[pos]>='a'-70 && block[pos]<='z'-70) {
								block[pos]+=70;
							}
							else if(block[pos]>='A'-64 && block[pos]<='Z'-64) {
								block[pos]+=64;
							}
							else if(block[pos]>='0'+5 && block[pos]<='9'+5) {
								block[pos]-=5;
							}
							else if(block[pos] == 0x7f) {
								block[pos]=' ';
							}
							else if(block[pos] == 0x73) {
								block[pos]='\'';
							}
							else if(block[pos] == 'r') {
								block[pos]=':';
							}
							else if(block[pos] == 'o') {
								block[pos]='(';
							}
							else if(block[pos] == 'p') {
								block[pos]=')';
							}
							else if(block[pos] == 0x6b) {
								block[pos]='!';
							}
							else if(block[pos] == 0x68) {
								block[pos]='-';
							}
							else if(block[pos] == 0x67) {
								block[pos]='?';
							}						
							else if(block[pos] == 0x66) {
								block[pos]='.';
							}
							else if(block[pos] == 0x65) {
								block[pos]=',';
							}
							else if(block[pos] == '?') {
								block[pos]='$';
							}
							else if(block[pos] == 't') {
								block[pos]='\'';
							}

							pos++;
							datalen--;
						}	
						
						if(orgdatalen > 0) {							
							scriptPages[loop].scriptTexts[scriptPages[loop].noScriptTexts].text =(char*) malloc(orgdatalen+1);
							memcpy(scriptPages[loop].scriptTexts[scriptPages[loop].noScriptTexts].text, block, orgdatalen);
							scriptPages[loop].scriptTexts[scriptPages[loop].noScriptTexts].text[orgdatalen] = 0;
							
							log("Text data",(char*) scriptPages[loop].scriptTexts[scriptPages[loop].noScriptTexts].text);																		
							}
						else {
							scriptPages[loop].scriptTexts[scriptPages[loop].noScriptTexts].text =(char*) malloc(1);							
							scriptPages[loop].scriptTexts[scriptPages[loop].noScriptTexts].text[0] = 0;							
							log("Text data", "Empty line");
							}
						scriptPages[loop].scriptTexts[scriptPages[loop].noScriptTexts].alignment = textAlignment;
						scriptPages[loop].scriptTexts[scriptPages[loop].noScriptTexts].fontId = textFont;
						if(textPosX != -1) {
							scriptPages[loop].scriptTexts[scriptPages[loop].noScriptTexts].x = textPosX;
							scriptPages[loop].scriptTexts[scriptPages[loop].noScriptTexts].y = textPosY;
							textPosX = -1;
							textPosY = -1;
							}
						scriptPages[loop].noScriptTexts++;
					}break;
				case ESceneTime:
					unsigned short int sceneTime = f->loadShort();
					log("Scene time",sceneTime);
					scriptPages[loop].pageTime = sceneTime;
					break;	
				case ESceneBreaker:
				case 0x3e:									
					pos = f->tell();
					log("Parse script end at position", pos);
					log("Parse script end with", type);
					breakloop = true;
					f->loadChar();
					break;			
				default:
					{
						pos = f->tell();
						log("Parse script end at position", pos);
						log("Parse script breaker", type);
						breakloop = true;												
					}
					break;
				}
				pos = f->tell();
			}
		}
	}

}

Scene::Scene (char * fileName) {

	File *file;
	char *string;
	int type;
    int loop;
    noScriptFonts = 0;
    textAlignment = 0;
    textFont = 0;
    textPosX = -1;
    textPosY = -1;
    
    log("\nScene", fileName);
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
	scriptPages = new ScriptPage[scriptItems];
	log("Scene: Script items", scriptItems);
	for(loop = 0;loop < scriptItems; loop++) {
		scriptStarts[loop] = file->loadInt();// Load offset to script
		log("scriptStart:", scriptStarts[loop]);
	}

	// Seek to datastart now
	file->seek(dataOffset, true); // Seek to data offsets
	dataItems = file->loadShort()+1; // Get number of data items
	log("Scene: Data items", dataItems);
	dataOffsets = new signed long int[dataItems];
	for(loop = 0;loop < dataItems; loop++) {
		dataOffsets[loop] = file->loadInt();// Load offset to script
		log("dataOffsets:", dataOffsets[loop]);
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

	//SDL_FreeSurface(sceneBGs[0]); // Temporary

	return;

}

ImageInfo* Scene::FindImage(int dataIndex) {
	int loop = 0;
	while(loop<imageIndex)
		{
		if(imageInfo[loop].dataIndex == dataIndex)
			{
			return &imageInfo[loop];
			}
			loop++;
		}
	return NULL;
	}

int Scene::play () {

	SDL_Rect dst;
	unsigned int sceneIndex = 0;
	ImageInfo* imageInfo = NULL;
	unsigned int pageTime = scriptPages[sceneIndex].pageTime;
	unsigned int lastTicks = globalTicks;
	if(scriptPages[sceneIndex].bgIndex != -1)
		{
		imageInfo = FindImage(scriptPages[sceneIndex].bgIndex);
		}
	
	if(imageInfo != NULL)
		usePalette(imageInfo[0].palette);
	while (true) {
		if (loop(NORMAL_LOOP) == E_QUIT) return E_QUIT;

		if (controls.release(C_ESCAPE)) {
									
			return E_NONE;

		}

		if(controls.release(C_ENTER) || ((globalTicks-lastTicks)>=pageTime*1000 && pageTime != 256 && pageTime != 0)) {			
			sceneIndex++;			
			if(sceneIndex == scriptItems) {
				return E_NONE;
			}
			lastTicks = globalTicks;
			// Get bg for this page
			if(scriptPages[sceneIndex].bgIndex != -1) {
				imageInfo = FindImage(scriptPages[sceneIndex].bgIndex);
				if(imageInfo != NULL) {
					usePalette(imageInfo[0].palette);
					}
				else
					{
					restorePalette(screen);
					}
				}
			
			pageTime = scriptPages[sceneIndex].pageTime;
		}

		// Temporary stuff		
		dst.x = (screenW - 320) >> 1;
		dst.y = (screenH - 200) >> 1;
		if(imageInfo != NULL) {			
		SDL_BlitSurface(imageInfo->image, NULL, screen, &dst);
		}
		else {			
		clearScreen(BLACK);
		}
		int x = 0;
		int y = 0;
		for(int text = 0;text<scriptPages[sceneIndex].noScriptTexts;text++) {
		Font* font = NULL;

		for(int index = 0; index < noScriptFonts; index++) {
		if( scriptPages[sceneIndex].scriptTexts[text].fontId == scriptFonts[index].fontId) {
		switch(scriptFonts[index].fontType)
			{
			case EFONT2Type:
				font = font2;
				break;
			case EFONTBIGType:
				font = fontbig;
				break;
			case EFONTINYType:
				font = fontiny;
				break;
			case EFONTMN1Type:
				font = fontmn1;
				break;
			case EFONTMN2Type:
				font = fontmn2;
				break;
			}
		continue;
		}
		}

		if(scriptPages[sceneIndex].scriptTexts[text].x != -1) {
		x = scriptPages[sceneIndex].scriptTexts[text].x;
		y = scriptPages[sceneIndex].scriptTexts[text].y;
		}

		switch(scriptPages[sceneIndex].scriptTexts[text].alignment)
			{
			case 0: // left
			font->showString(scriptPages[sceneIndex].scriptTexts[text].text, x,y);
			break;
			case 1: // right
			int width = font->calcStringWidth(scriptPages[sceneIndex].scriptTexts[text].text);
			font->showString(scriptPages[sceneIndex].scriptTexts[text].text, 320-width, y);						
			break;
			case 2: // center
				{
				int width = font->calcStringWidth(scriptPages[sceneIndex].scriptTexts[text].text)/2;
				font->showString(scriptPages[sceneIndex].scriptTexts[text].text, 160-width, y);
				}
				break;
			}
		y+=font->fontHeight()/2;
		}

		SDL_Delay(T_FRAME);

	}

	return E_NONE;

}


