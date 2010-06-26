
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
	ESceneAnimation = 0xA6,
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
		int nextPageAfterAnim;
		
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

class SceneFrame
	{
public:
	SceneFrame(int frameType, unsigned char* frameData, int frameSize);
	~SceneFrame();
	int soundId;
	unsigned int frameType;
	unsigned char* frameData;
	int frameSize;
	SceneFrame*  next;
	SceneFrame*  prev;
	};

class SceneAnimation
	{
public:
		SceneAnimation  (SceneAnimation* newNext);
		~SceneAnimation ();
		void addFrame(int frameType, unsigned char* frameData, int frameSize);
		SDL_Surface*       background;
		SceneAnimation*  next;		
		int id;
		int noSounds;
		char soundNames[16][10];
		SceneFrame* sceneFrames;
		SceneFrame* lastFrame;
		int frames;
		int reverseAnimation;
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

		void               loadScripts      (File* f);
		void               loadData         (File* f);
		void               loadAni          (File* f, int dataIndex);
		void               loadCompactedMem (int size, unsigned char* frameData, unsigned char* pixdata, int width, int height);
		void               loadFFMem        (int size, unsigned char* frameData, unsigned char* pixdata);
		unsigned short int loadShortMem     (unsigned char **data);

	public:
		Scene    (const char* fileName);
		~Scene   ();

		int play ();

};

#endif

