
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
 * Copyright (c) 2005-2017 AJ Thomson
 * Copyright (c) 2015-2024 Carsten Teibes
 *
 * OpenJazz is distributed under the terms of
 * the GNU General Public License, version 2.0
 *
 */


#ifndef _SCENE_H
#define _SCENE_H

#include "io/file.h"
#include "types.h"
#include <list>
#include <vector>

// Enums

#define MAKE_0SC_HEADER(BEG, END) (BEG | END << 8)

/// Cutscene file animation headers
enum ANIHeaders	{
	EAniHeader = MAKE_0SC_HEADER('A', 'N'),
	E11AniHeader = MAKE_0SC_HEADER('1', '1'), ///< Background/start image
	E1LAniHeader = MAKE_0SC_HEADER('1', 'L'),
	E00AniHeader = MAKE_0SC_HEADER(0, 0),
	EPBAniHeader = MAKE_0SC_HEADER('P', 'B'),
	EFullFrameAniHeader = MAKE_0SC_HEADER('F', 'F'), ///< Floodfill?
	ERNAniHeader = MAKE_0SC_HEADER('R', 'N'),
	ERBAniHeader = MAKE_0SC_HEADER('R', 'B'),
	ERCAniHeader = MAKE_0SC_HEADER('R', 'C'),
	ERLAniHeader = MAKE_0SC_HEADER('R', 'L'),
	EReverseAniHeader = MAKE_0SC_HEADER('R', 'R'), ///< Reverse animation when end found
	EEndAniHeader = MAKE_0SC_HEADER('_', 'E'), ///< End of animation
	ECompactedAniHeader = MAKE_0SC_HEADER(']', '['), ///< Full screen animation frame, does not clear the screen first
	EMXAniHeader = MAKE_0SC_HEADER('M', 'X'),
	ESoundTagAniHeader = MAKE_0SC_HEADER('S', 'T'), ///< Sound tag
	ESoundListAniHeader = MAKE_0SC_HEADER('S', 'L'), ///< Sound list
	EPlayListAniHeader = MAKE_0SC_HEADER('P', 'L') ///< Playlist
};

/// Cutscene script types - these are the known types
enum {
	ESceneYesNo = 0x23, // #
	ESceneMusic = 0x2A, // *
	ESceneStopMusic = 0x2D, // -
	ESceneTransitionType = 0x3F, // ?
	ESceneTextBlock = 0x40, // @
	ESceneTextColour = 0x41, // A
	ESceneFontFun = 0x45, // E
	ESceneFontIndex = 0x46, // F
	ESceneTextPosition = 0x47, // G
	ESceneTextAlign = 0x4A, // J
	ESceneTextAlign2 = 0x4B, // K
	ESceneBackground = 0x4C, // L
	ESceneBreaker = 0x50, // P
	ESceneMusicTransition = 0x51, // Q
	ESceneTextRect = 0x57, // W
	ESceneFontDefine = 0x58, // X
	ESceneTime = 0x5D, // ]
	ESceneTextLine = 0x5E, // ^
	ESceneTextVAdjust = 0x5F, // _ = move "line"
	ESceneAnimation = 0xA6,
	ESceneAnimationPlayAndContinue = 0xA7,
	ESceneBackgroundFade = 0xB1,
	ESceneTextSomething = 0xD9,
	ESceneTextShadow = 0xDB
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
		int            textColour;
		int            shadowColour;

		JJ1SceneText ();
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
		int                     pageTime;
		std::list<JJ1SceneText> texts;
		char*                   musicFile;
		int                     paletteIndex;
		bool                    askForYesNo;
		bool                    stopMusic;
		int                     backgroundFade;
		int                     transitionType;

		JJ1ScenePage ();
		~JJ1ScenePage ();
};

/// Cutscene background image
class JJ1SceneImage {

	public:
		SDL_Surface* image;
		int id;

		explicit JJ1SceneImage (int id);
		~JJ1SceneImage ();
		JJ1SceneImage (const JJ1SceneImage&) = delete; // non construction-copyable
		JJ1SceneImage& operator=(const JJ1SceneImage&) = delete; // non copyable
};

/// Cutscene palette
class JJ1ScenePalette {

	public:
		SDL_Color palette[MAX_PALETTE_COLORS];
		int id;

		explicit JJ1ScenePalette (int id);
		JJ1ScenePalette (const JJ1ScenePalette&) = delete; // non construction-copyable
		JJ1ScenePalette& operator=(const JJ1ScenePalette&) = delete; // non copyable
};

/// Cutscene font
class JJ1SceneFont {

	public:
		Font *font;
		int   id;

		explicit JJ1SceneFont (int id);
		JJ1SceneFont (const JJ1SceneFont&) = delete; // non construction-copyable
		JJ1SceneFont& operator=(const JJ1SceneFont&) = delete; // non copyable
};

/// Cutscene animation frame
class JJ1SceneFrame {

	public:
		JJ1SceneFrame* next;
		JJ1SceneFrame* prev;
		unsigned char* frameData;
		int            frameSize;
		unsigned int   frameType;
		SE::Type       soundId;

		JJ1SceneFrame (int frameType, unsigned char* frameData, int frameSize);
		~JJ1SceneFrame ();
		JJ1SceneFrame (const JJ1SceneFrame&) = delete; // non construction-copyable
		JJ1SceneFrame& operator=(const JJ1SceneFrame&) = delete; // non copyable
};

/// Cutscene animation
class JJ1SceneAnimation {

	public:
		JJ1SceneFrame*      sceneFrames;
		JJ1SceneFrame*      lastFrame;

		SDL_Surface*        background;
		SDL_Surface*        scratch;
		int id;
		int frames;
		int reverseAnimation;

		explicit JJ1SceneAnimation (int id);
		~JJ1SceneAnimation ();
		JJ1SceneAnimation (const JJ1SceneAnimation&) = delete; // non construction-copyable
		JJ1SceneAnimation& operator=(const JJ1SceneAnimation&) = delete; // non copyable

		void addFrame (int frameType, unsigned char* frameData, int frameSize);
};

/// Cutscene
class JJ1Scene {

	private:
		std::list<JJ1SceneAnimation> animations;
		std::list<JJ1SceneImage>     images;
		std::list<JJ1ScenePalette>   palettes;
		std::list<JJ1SceneFont>      fonts;

		/// Scripts all information needed to render script pages, text etc
		std::vector<JJ1ScenePage>      pages;

		unsigned short int scriptItems, dataItems;
		std::vector<signed long int> scriptStarts, dataOffsets;

		void               loadScripts      (File* f);
		void               loadData         (File* f);
		void               loadAni          (JJ1SceneAnimation &animation, File* f, int dataIndex);
		void               loadCompactedMem (int size, unsigned char* frameData, unsigned char* pixdata);
		void               loadFullFrameMem (int size, unsigned char* frameData, unsigned char* pixdata);
		unsigned short int loadShortMem     (unsigned char **data);

	public:
		explicit JJ1Scene (const char* fileName);
		JJ1Scene (const JJ1Scene&) = delete; // non construction-copyable
		JJ1Scene& operator=(const JJ1Scene&) = delete; // non copyable

		int play ();
};

#endif
