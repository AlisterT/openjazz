
/*
 *
 * scene.h
 *
 * 3rd February 2009: Created scene.h from parts of scene.c
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


#ifndef _SCENE_H
#define _SCENE_H

#include <io/file.h>
#include <sdl.h>

enum EScriptFontTypes
	{
	EFONT2Type,
	EFONTBIGType,
	EFONTINYType,
	EFONTMN1Type,
	EFONTMN2Type
	};

class ScriptFont
	{
public:
	// This maps to any of the available types (already loaded)
	EScriptFontTypes fontType;
	
	// Font id given this font
	int fontId;
	};

class ScriptText
	{
public:
	ScriptText();
	~ScriptText();		
	char* text;
	int alignment;
	int fontId;
	int x;
	int y;
	SDL_Rect textRect;
	int extraLineHeight;
	};

// Class
class ScriptPage
	{
public:
	ScriptPage();
	~ScriptPage();
	int backgrounds;
	int bgIndex[30];
	unsigned int bgPos[30];
	// Length of the scene in seconds, or if zero = anim complete, or 256 = user interaction
	int pageTime;
	ScriptText scriptTexts[100];
	int noScriptTexts;
	char* musicfile;
	int paletteIndex;
	};

class ImageInfo
	{
public:
	ImageInfo();
	~ImageInfo();
	// SDL_Surface with the image
	SDL_Surface *image;
	
	// data index of the image (not the palette) to compare with scripts
	int dataIndex;
	};

class PaletteInfo
	{
public:
	// Palette associated with the image
	SDL_Color    palette[256];
	
	// data index of the image (not the palette) to compare with scripts
	int dataIndex;
	};

class Scene {
	private:
		ImageInfo imageInfos[100];
		PaletteInfo paletteInfos[100];
		unsigned short int scriptItems;
		unsigned short int dataItems;
		signed long int* scriptStarts;
		signed long int* dataOffsets;
		int imageIndex;
		int paletteIndex;
		// Scripts all information needed to render script pages, text etc
		ScriptPage* scriptPages;
		ImageInfo* FindImage(int dataIndex);
		// DataBlock
	protected:
		void ParseScripts(File *f);
		void ParseData(File *f);
		void ParseAni(File* f, int dataIndex);
	public:
		Scene    (char * fileName);
		~Scene   ();

		int play ();
		ScriptFont scriptFonts[5];
		int noScriptFonts;					
};

#endif

