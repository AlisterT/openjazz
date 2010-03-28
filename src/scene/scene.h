
/*
 *
 * scene.h
 *
 * 3rd February 2009: Created scene.h from parts of scene.c
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


#ifndef _SCENE_H
#define _SCENE_H


#include <io/file.h>

#include <SDL/SDL.h>


// Enums

// These are the known script types
enum {

	ESceneMusic = 0x2A,
	ESceneFadeType = 0x3F,
	ESceneTextBlock = 0x40,
	ESceneTextColour = 0x41,
	ESceneFontFun = 0x45,
	ESceneFontIndex = 0x46,
	ESceneTextPosition = 0x47,
	ESceneTextAlign = 0x4A,
	ESceneTextAlign2 = 0x4B,
	ESceneBackground = 0x4c,
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


// Classes

class Font;

class ScriptFont {

	public:
		// This points to any of the available types (already loaded)
		Font *font;

		// Font id given this font
		int   fontId;

};

class ScriptText {

	public:
		char     *text;
		int       alignment;
		int       fontId;
		int       x;
		int       y;
		SDL_Rect  textRect;
		int       extraLineHeight;

		ScriptText  ();
		~ScriptText ();

};

class ScriptPage {

	public:
		int           backgrounds;
		int           bgIndex[30];
		unsigned int  bgPos[30];

		// Length of the scene in seconds, or if zero = anim complete, or 256 = user interaction
		int           pageTime;
		ScriptText    scriptTexts[100];
		int           noScriptTexts;
		char         *musicfile;
		int           paletteIndex;

		ScriptPage();
		~ScriptPage();

};

class ImageInfo {

	public:
		// SDL_Surface with the image
		SDL_Surface *image;

		// data index of the image (not the palette) to compare with scripts
		int dataIndex;

		ImageInfo  ();
		~ImageInfo ();

};

class PaletteInfo {

	public:
		// Palette associated with the image
		SDL_Color palette[256];

		// data index of the image (not the palette) to compare with scripts
		int       dataIndex;

};

class Scene {

	private:
		ImageInfo           imageInfos[100];
		PaletteInfo         paletteInfos[100];
		unsigned short int  scriptItems;
		unsigned short int  dataItems;
		signed long int    *scriptStarts;
		signed long int    *dataOffsets;
		int                 imageIndex;
		int                 paletteIndex;

		// Scripts all information needed to render script pages, text etc
		ScriptPage         *scriptPages;

		void        loadScripts (File *f);
		void        loadData    (File *f);
		void        loadAni     (File* f, int dataIndex);
		ImageInfo * FindImage   (int dataIndex);

	public:
		ScriptFont scriptFonts[5];
		int        noScriptFonts;

		Scene    (const char * fileName);
		~Scene   ();

		int play ();

};

#endif

