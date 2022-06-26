
/**
 *
 * @file jj1scene.h
 *
 * Part of the OpenJazz project
 *
 * @par History:
 * - 23rd August 2005: Created scene.c
 * - 3rd February 2009: Created scene.h from parts of scene.c
 * - 1st August 2012: Renamed scene.h to jj1scene.h
 *
 * @par Licence:
 * Copyright (c) 2005-2017 Alister Thomson
 *
 * OpenJazz is distributed under the terms of
 * the GNU General Public License, version 2.0
 *
 */


#ifndef _SCENE_H
#define _SCENE_H


#include "io/file.h"


// Enums

/**
 * Cutscene file animation headers
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
	E11AniHeader = 0x3131, ///< Background/start image
	E1LAniHeader = 0x4c31,
	EPBAniHeader = 0x4250,
	EFFAniHeader = 0x4646, ///< Floodfill? or full frame?
	ERNAniHeader = 0x4e52,
	ERBAniHeader = 0x4252,
	ERCAniHeader = 0x4352,
	ERLAniHeader = 0x4c52,
	ERRAniHeader = 0x5252,
	E_EHeader = 0x455F, ///< ANI End
	ESquareAniHeader = 0x5b5d,
	EMXAniHeader = 0x584d,
	ESTAniHeader = 0x5453, ///< Sound tag
	ESoundListAniHeader = 0x4C53,
	EPlayListAniHeader = 0x4C50
	};

/// Cutscene script types - these are the known types
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
	ESceneBackgroundFade = 0xb1,
	ESceneTextSomething = 0xd9,
	ESceneTextShadow = 0xdb

};


// Classes

class Font;

/// Cutscene page text
class JJ1SceneText {

	public:
		unsigned char* text;
		int            alignment;
		int            fontId;
		int            x;
		int            y;
		SDL_Rect       textRect;
		int            extraLineHeight;
		int			   shadowColour;

		JJ1SceneText  ();
		~JJ1SceneText ();

};

/// Cutscene page
class JJ1ScenePage {

	public:
		int                backgrounds;
		int                bgIndex[30];
		unsigned short int bgX[30];
		unsigned short int bgY[30];

		int animLoops;
		int animSpeed;
		int animIndex;
		int nextPageAfterAnim;

		/// Length of the scene in seconds, or if zero = anim complete, or 256 = user interaction
		int                pageTime;
		JJ1SceneText       texts[100];
		int                nTexts;
		char*              musicFile;
		int                paletteIndex;
		int				   askForYesNo;
		int				   stopMusic;
		int					backgroundFade;
		JJ1ScenePage  ();
		~JJ1ScenePage ();

};

/// Cutscene background image
class JJ1SceneImage {

	public:
		JJ1SceneImage* next;
		SDL_Surface* image;
		int id;

		JJ1SceneImage  (JJ1SceneImage* newNext);
		~JJ1SceneImage ();

};

/// Cutscene palette
class JJ1ScenePalette {

	public:
		JJ1ScenePalette* next;
		SDL_Color palette[256];
		int id;

		JJ1ScenePalette  (JJ1ScenePalette* newNext);
		~JJ1ScenePalette ();

};

/// Cutscene font
class JJ1SceneFont {

	public:
		Font *font;
		int   id;

};

/// Cutscene animation frame
class JJ1SceneFrame {

	public:
		JJ1SceneFrame* next;
		JJ1SceneFrame* prev;
		unsigned char* frameData;
		int            frameSize;
		unsigned int   frameType;
		unsigned char  soundId;

		JJ1SceneFrame  (int frameType, unsigned char* frameData, int frameSize);
		~JJ1SceneFrame ();

};

/// Cutscene animation
class JJ1SceneAnimation {

	public:
		JJ1SceneAnimation*  next;
		JJ1SceneFrame*      sceneFrames;
		JJ1SceneFrame*      lastFrame;

		SDL_Surface*       background;
		int id;
		int frames;
		int reverseAnimation;

		JJ1SceneAnimation  (JJ1SceneAnimation* newNext);
		~JJ1SceneAnimation ();

		void addFrame (int frameType, unsigned char* frameData, int frameSize);

};

/// Cutscene
class JJ1Scene {

	private:
		JJ1SceneAnimation* animations;
		JJ1SceneImage*     images;
		JJ1ScenePalette*   palettes;
		JJ1SceneFont       fonts[5];
		int                nFonts;
		unsigned short int scriptItems;
		unsigned short int dataItems;
		signed long int*   scriptStarts;
		signed long int*   dataOffsets;

		/// Scripts all information needed to render script pages, text etc
		JJ1ScenePage*      pages;

		void               loadScripts      (File* f);
		void               loadData         (File* f);
		void               loadAni          (File* f, int dataIndex);
		void               loadCompactedMem (int size, unsigned char* frameData, unsigned char* pixdata);
		void               loadFFMem        (int size, unsigned char* frameData, unsigned char* pixdata);
		unsigned short int loadShortMem     (unsigned char **data);

	public:
		JJ1Scene  (const char* fileName);
		~JJ1Scene ();

		int play ();

};

#endif

