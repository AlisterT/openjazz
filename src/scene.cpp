
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
#include "io/gfx/paletteeffects.h"

#include <string.h>

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

ImageInfo::ImageInfo() {
	image = NULL;
}

ImageInfo::~ImageInfo() {
	if(image != NULL) {
		SDL_FreeSurface(image);
		image = NULL;
	}
}

ScriptText::ScriptText() {
		x = -1;
		y = -1;
		textRect.x = -1;
		textRect.y = -1;
		extraLineHeight = -1;
		text = NULL;
}
	
ScriptText::~ScriptText() {

	if (text) delete[] text;

}

ScriptPage::ScriptPage() {		
	pageTime = 0;
	noScriptTexts = 0;
	backgrounds = 0;
	musicfile = NULL;
	paletteIndex = 0;
}

ScriptPage::~ScriptPage() {

	if (musicfile) delete[] musicfile;

}

void Scene::ParseAni(File* f, int dataIndex) {
	unsigned short int aniType = f->loadShort();// should be 0x02
	log("ParseAni DataLen", aniType);
	unsigned short int aniOffset = f->loadShort();// unknown, number of frames?
	log("ParseAni Frames?", aniOffset);
	unsigned short int type = 0;//
	int loop;

	while(type != 0x4C50) {
		type = f->loadShort();
		
		if(type == 0x4C53) { // SL 
			/*unsigned short int offset =*/ f->loadShort();
			unsigned char noSounds = f->loadChar();
			for(loop = 0;loop<noSounds;loop++) {
				char* soundName = f->loadString();
				log("Soundname ", soundName);
				delete[] soundName;
			}
		}
		else if(type == 0x4C50) {// PL		
			int pos = f->tell();
			int nextPos = f->tell();
			log("PL Read position", pos);
			unsigned short int len = f->loadShort();
			unsigned char* buffer = f->loadBlock(len);
			for (int count = 0; count < 256; count++) {
				
				// Palette entries are 6-bit
				// Shift them upwards to 8-bit, and fill in the lower 2 bits
				paletteInfos[paletteIndex].palette[count].r = (buffer[count * 3] << 2) + (buffer[count * 3] >> 4);
				paletteInfos[paletteIndex].palette[count].g = (buffer[(count * 3) + 1] << 2) +
					(buffer[(count * 3) + 1] >> 4);
				paletteInfos[paletteIndex].palette[count].b = (buffer[(count * 3) + 2] << 2) +
					(buffer[(count * 3) + 2] >> 4);
				
				
			}
			
			delete[] buffer;
			paletteInfos[paletteIndex].dataIndex = dataIndex; 
			paletteIndex++;
			
			unsigned short int value = 0x4646;
			int items = 0;
			int validValue = true;
			pos = f->tell();
							
			log("PL Read position start", pos);
			while(validValue)
				{					
				value = f->loadShort();
				log("PL Read block start tag", value);
				unsigned short int size= f->loadShort();
				log("PL Anim block size", size);
				nextPos = f->tell();
				// next pos is intial position + size and four bytes header
				nextPos+=(size);
				switch(value)
					{					
					case 0x455F:				
						validValue = false;
						break;
					case 0x3131:
						{						
						// Skip back size header, this is read by the surface reader
						f->seek(-2, false);						

						SDL_Surface* image = f->loadSurface(320, 200, true);
						SDL_Rect dst;
						dst.x = 0;
						dst.y = 0;
						SDL_BlitSurface(image, NULL, screen, &dst);
						SDL_SetPalette(screen, SDL_PHYSPAL, paletteInfos[paletteIndex-1].palette, 0, 256);
						currentPalette = paletteInfos[paletteIndex-1].palette;						
						}break;
					case 0x4c31:
						{								
						int longvalue = f->loadInt();
						log("PL Anim block value", longvalue);
						// Skip back size header, this is read by the surface reader
						//f->seek(-2, false);						
						while(size) {
							size--;
							}
						}break;						
					case 0x4646:
						{						
						while(size) {
							unsigned char header = f->loadChar();
							log("PL 4646 block header", header);
							switch(header)
								{
								case 0xff:
									{
									unsigned char x= f->loadChar();
									unsigned char y= f->loadChar();								
									log("PL block x", x);
									log("PL block y", y);
									size-=2;
									}break;
								}
							size--;
							}
						}break;
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
						log("PL 5b5d block header", header);
						unsigned short int value = f->loadShort();
						log("PL 5b5d block value", value);
						}
						break;
					case 0x5453:
						{
						unsigned char soundIndex = f->loadChar();
						unsigned char soundNote = f->loadChar();
						unsigned char soundOffset = f->loadChar(); 
						log("PL Audio tag with index", soundIndex);
						log("PL Audio tag play at ", soundNote);
						log("PL Audio tag play offset ", soundOffset);
						}
						break;
					
					case 0:
						{
						int longvalue = f->loadInt();
						while(longvalue == 0) {
							longvalue = f->loadInt();
							nextPos+=4;
						}
						f->seek(-4, false);
						value = longvalue;
						}
						break;
					default:
						log("PL Read Unknown type", value);
						validValue = false;
						break;
					}

				pos = f->tell();				
				log("PL Read position after block should be", nextPos);
				f->seek(nextPos, true);
					if(validValue) {
					items++;
					}
				}
			
			log("PL Parsed through number of items skipping 0 items", items);		
			pos = f->tell();
			log("PL Read position after parsing anim blocks", pos);
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
			log("Data Type", "ANI");
			ParseAni(f, loop);			
		}
		else {
			unsigned char type = f->loadChar();
			log("Data Type", type);
			switch(type)
			{	
			case 3: 
			case 4: // image
			case 5:
			case 6:
				{
					log("Data Type", "Image");
					log("Data Type Image index", loop);
					unsigned short int width = f->loadShort(); // get width
					unsigned short int height;
					if(type == 3)
						height = f->loadChar(); // Get height
					else
						height = f->loadShort(); // Get height
					if(imageIndex<100) {
						f->seek(-2, false);
						imageInfos[imageIndex].image = f->loadSurface(width, height);
						imageInfos[imageIndex].dataIndex = loop;
						imageIndex++;
					}
				}break;
				
			default:
				{					
					log("Data Type", "Palette");
					log("Data Type Palette index", loop);
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


void Scene::ParseScripts(File *f) {
	int loop;
	char *string;
	/*int bgIndex = 0;*/
	int textAlignment = 0;
	int textFont = 0;   
	for(loop = 0;loop < scriptItems; loop++)
	{
	    log("\nParse Script", loop);
	    int textPosX = -1;
	    int textPosY = -1;
	    	    
	    int extraheight = -1;
	    SDL_Rect textRect;
	    bool textRectValid = false;
		f->seek(scriptStarts[loop], true); // Seek to data start
		if(f->loadChar() == 0x50) { // Script tag		
			unsigned short int scriptid = f->loadShort();
			log("Script id:", scriptid);
			int palette = f->loadShort();
			log("Script default palette", palette);
			scriptPages[loop].paletteIndex = palette;
			
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
						unsigned short int xpos = f->loadShort();
						unsigned short int ypos = f->loadShort();
						unsigned short bgImageIndex = f->loadShort();
						log("ESceneBackground: index", bgImageIndex);
						log("ESceneBackground: xpos", xpos);
						log("ESceneBackground: ypos", ypos);
						scriptPages[loop].bgIndex[scriptPages[loop].backgrounds] = bgImageIndex;
						scriptPages[loop].bgPos[scriptPages[loop].backgrounds] = xpos|(ypos<<16);
						scriptPages[loop].backgrounds++;
					}break;
				case ESceneMusic:
					{						
						// Music file name
						string = f->loadString();
						log("ESceneMusic: ", string);
						scriptPages[loop].musicfile = createString(string);
						delete[] string;
					}break;				
				case ESceneSomethingElse:
					{
					unsigned char value = 0;//f->loadChar();
					log("ESceneSomethingElse", value);
					}break;
				case ESceneTextRect: // String
					{
						unsigned short x = textRect.x = f->loadShort();
						unsigned short y = textRect.y = f->loadShort();
						unsigned short w = textRect.w = (f->loadShort()-x);
						unsigned short h = textRect.h = (f->loadShort()-y);
						textRectValid = true;
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
						delete[] fontname;
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
					{
					unsigned short value = f->loadShort();
					log("ESceneTextColour", value);
					}
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
					extraheight = value;
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
						char pagebuf[3];
						log("Text len=", datalen);
						// Convert to ascii
						while(datalen>0) {
							if(block[pos] == 0x8b) {
							sprintf(pagebuf, "%2d", loop+1);
							memcpy(&block[pos-1], pagebuf,2);							
							}
							else if(block[pos] == 0x8a) {
							sprintf(pagebuf, "%2d", scriptItems);
							memcpy(&block[pos-1], pagebuf,2);							
							}
							else if(block[pos] == 'C') {
								block[pos]=' ';							
							}
							else if(block[pos]>='a'-70 && block[pos]<='z'-70) {
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
							scriptPages[loop].scriptTexts[scriptPages[loop].noScriptTexts].text = new char[orgdatalen + 1];
							memcpy(scriptPages[loop].scriptTexts[scriptPages[loop].noScriptTexts].text, block, orgdatalen);
							scriptPages[loop].scriptTexts[scriptPages[loop].noScriptTexts].text[orgdatalen] = 0;
							
							log("Text data",(char*) scriptPages[loop].scriptTexts[scriptPages[loop].noScriptTexts].text);																		
							}
						else {
							scriptPages[loop].scriptTexts[scriptPages[loop].noScriptTexts].text = new char[1];							
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
						if(textRectValid)
							{
							scriptPages[loop].scriptTexts[scriptPages[loop].noScriptTexts].textRect = textRect;
							textRectValid = false;
							}
						if(extraheight != -1)
							{
							scriptPages[loop].scriptTexts[scriptPages[loop].noScriptTexts].extraLineHeight = extraheight;
							extraheight = -1;
							}
						scriptPages[loop].noScriptTexts++;
					}break;
				case ESceneTime:
					{
					unsigned short int sceneTime = f->loadShort();
					log("Scene time",sceneTime&255);
					scriptPages[loop].pageTime = sceneTime&255;
					}
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

Scene::Scene (const char * fileName) {

	File *file;	
    int loop;
    noScriptFonts = 0;        
    log("\nScene", fileName);
	try {

		file = new File(fileName, false);

	} catch (int e) {

		throw e;

	}
	imageIndex = 0;
	paletteIndex = 0;
	
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
	delete file;

	return;

}


Scene::~Scene () {
	delete []scriptPages;
}

ImageInfo* Scene::FindImage(int dataIndex) {
	int loop = 0;
	while(loop<imageIndex)
		{
		if(imageInfos[loop].dataIndex == dataIndex)
			{
			return &imageInfos[loop];
			}
			loop++;
		}
	return NULL;
	}

int Scene::play () {

	SDL_Rect dst;
	unsigned int sceneIndex = 0;
	ImageInfo* imageInfo;
	unsigned int pageTime = scriptPages[sceneIndex].pageTime;
	unsigned int lastTicks = globalTicks;
	int newpage = true;
	int fadein = false;
	SDL_Rect textRect = {0,0,320,200};
	 
	while (true) {
		if (loop(NORMAL_LOOP) == E_QUIT) return E_QUIT;

		if (controls.release(C_ESCAPE)) {
									
			return E_NONE;

		}
		SDL_Delay(T_FRAME);
		int upOrLeft = (controls.release(C_UP) || controls.release(C_LEFT));
		if((sceneIndex > 0 && upOrLeft) || controls.release(C_RIGHT) || controls.release(C_DOWN) || controls.release(C_ENTER) || 
			((globalTicks-lastTicks)>=pageTime*1000 && pageTime != 256 && pageTime != 0)) {
			if(upOrLeft) {
				sceneIndex--;
			}
			else {
				sceneIndex++;
			}
			if(sceneIndex == scriptItems) {
				return E_NONE;
			}
			lastTicks = globalTicks;
			// Get bg for this page
			newpage = true;			

			pageTime = scriptPages[sceneIndex].pageTime;
		}
		if(newpage) {			
			//firstPE = new FadeOutPaletteEffect(250, firstPE);				
			
			textRect.x = 0;
			textRect.y = 0;
			textRect.w = 320;
			textRect.h = 200;
			PaletteInfo* paletteInfo = NULL; 
			for(int palette = 0;palette<paletteIndex;palette++) {
				if(paletteInfos[palette].dataIndex == scriptPages[sceneIndex].paletteIndex) {
					paletteInfo = &paletteInfos[palette];
					break;
					}
					
				}
			
			if(paletteInfo != NULL) {									
					// usePalette(paletteInfo->palette);
					currentPalette = paletteInfo->palette;
					fadein = true;
				}
			else {
				restorePalette(screen);				
				}
						
			newpage = 0;
			}
		
		// First draw the backgrounds associated with this page
		if(scriptPages[sceneIndex].backgrounds > 0) {
			for(int bg = 0;bg<scriptPages[sceneIndex].backgrounds;bg++) {
				imageInfo = FindImage(scriptPages[sceneIndex].bgIndex[bg]);
				if(imageInfo != NULL) {
					dst.x = (scriptPages[sceneIndex].bgPos[bg] & 65535)*2+(screenW - 320) >> 1;
					dst.y = ((scriptPages[sceneIndex].bgPos[bg] & (~65535))>>16)*2+(screenH - 200) >> 1;
					SDL_BlitSurface(imageInfo->image, NULL, screen, &dst);
					}		
				}
		} else {
			clearScreen(0);
		}
		if(fadein)
			{
			fadein = false;
			firstPE = new FadeInPaletteEffect(250, firstPE);			
			}
		// Draw the texts associated with this page
		int x = 0;
		int y = 0;
		int extralineheight = 0;
		for(int text = 0;text<scriptPages[sceneIndex].noScriptTexts;text++) {
		Font* font = NULL;

		for(int index = 0; index < noScriptFonts; index++) {
		if( scriptPages[sceneIndex].scriptTexts[text].fontId == scriptFonts[index].fontId) {
			switch(scriptFonts[index].fontType) {
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

		if(scriptPages[sceneIndex].scriptTexts[text].textRect.x != -1) {
			textRect = scriptPages[sceneIndex].scriptTexts[text].textRect;
			x = 0;
			y = 0;
			}
		
		if(scriptPages[sceneIndex].scriptTexts[text].extraLineHeight != -1) {
			extralineheight = scriptPages[sceneIndex].scriptTexts[text].extraLineHeight;
		}
		
		switch(scriptPages[sceneIndex].scriptTexts[text].alignment)
			{
			case 0: // left
				font->showString(scriptPages[sceneIndex].scriptTexts[text].text, textRect.x+x,textRect.y+y);
			break;
			case 1: // right
				{
				int width = font->getStringWidth(scriptPages[sceneIndex].scriptTexts[text].text);
				font->showString(scriptPages[sceneIndex].scriptTexts[text].text, textRect.x+textRect.w-width, textRect.y+y);						
				}
			break;
			case 2: // center
				{
				int width = font->getStringWidth(scriptPages[sceneIndex].scriptTexts[text].text)/2;
				font->showString(scriptPages[sceneIndex].scriptTexts[text].text, textRect.x+(textRect.w/2)-width,textRect.y+ y);
				}
				break;
			}
		y+=(extralineheight+font->getHeight()/2);
		}

	}

	return E_NONE;

}


