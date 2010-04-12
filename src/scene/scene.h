
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


#include "io/file.h"

#include <SDL/SDL.h>


// Enums
enum
{
    ESignatureLength = 0x13,
    EScriptStartTag = 0x50,  
    EAnimationData = 0x4e41
};

// These are the known script types
enum {
	ESceneYesNo = 0x23,
	ESceneMusic = 0x2A,
	ESceneStopMusic = 0x2D,
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
	ESceneAnimationPlayAndContinue = 0xA7,
	ESceneAnimationSetting = 0xA6,
	ESceneTextSetting = 0xb1,
	ESceneTextSomething = 0xd9,
	ESceneTextShadow = 0xdb

};


// Classes

class Font;

class SceneText {

	public:
		unsigned char* text;
		int            alignment;
		int            fontId;
		int            x;
		int            y;
		SDL_Rect       textRect;
		int            extraLineHeight;
		int			   shadowColour;
		SceneText  ();
		~SceneText ();

};

class ScenePage {

	public:
		int                backgrounds;
		int                bgIndex[30];
		unsigned short int bgX[30];
		unsigned short int bgY[30];
	
		int animLoops;
		int animSpeed;
		int animIndex;	
		
		// Length of the scene in seconds, or if zero = anim complete, or 256 = user interaction
		int                pageTime;
		SceneText          texts[100];
		int                nTexts;
		char*              musicFile;
		int                paletteIndex;
		int				   askForYesNo;
		int				   stopMusic;
		ScenePage();
		~ScenePage();

};

class SceneImage {

	public:
		SceneImage*  next;
		SDL_Surface* image;
		int id;

		SceneImage  (SceneImage* newNext);
		~SceneImage ();

};

class ScenePalette {

	public:
		ScenePalette *next;
		SDL_Color palette[256];
		int id;

		ScenePalette  (ScenePalette* newNext);
		~ScenePalette ();

};

class SceneFont {

	public:
		Font *font;
		int   id;

};

class SceneAnimation
	{
public:
		SDL_Surface*       background;
		SceneAnimation*  next;		
		int id;
		int noSounds;
		char soundNames[16][10];
		SceneAnimation  (SceneAnimation* newNext);
		~SceneAnimation ();			
	};

class Scene {

	private:		
		SceneAnimation*    animations;
		SceneImage*        images;
		ScenePalette*      palettes;
		SceneFont          fonts[5];
		int                nFonts;
		unsigned short int scriptItems;
		unsigned short int dataItems;
		signed long int*   scriptStarts;
		signed long int*   dataOffsets;

		// Scripts all information needed to render script pages, text etc
		ScenePage*         pages;

		void loadScripts (File* f);
		void loadData    (File* f);
		void loadAni     (File* f, int dataIndex);
		void LoadCompacted(int& size, File* f, unsigned char* pixdata, int width, int height);
	public:
		Scene    (const char* fileName);
		~Scene   ();

		int play ();

};

#endif

