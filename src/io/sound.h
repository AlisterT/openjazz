
/**
 *
 * @file sound.h
 *
 * Part of the OpenJazz project
 *
 * @par History:
 * - 23rd August 2005: Created OpenJazz.h
 * - 2nd June 2009: Created sound.h from parts of OpenJazz.h
 *
 * @par Licence:
 * Copyright (c) 2005-2010 Alister Thomson
 *
 * OpenJazz is distributed under the terms of
 * the GNU General Public License, version 2.0
 *
 */

#ifndef OJ_SOUND_H
#define OJ_SOUND_H

#include "OpenJazz.h"

// Constants

// Sound effects

namespace SE {
	enum Type : int {
		NONE = 0,
		INVULN,
		MACHGUN,
		BOOM,
		OW,
		YUM,
		FIRE,
		UPLOOP,
		ONEUP,
		PHOTON,
		WAIT,
		ORB,
		JUMPA,
		GODLIKE,
		YEAHOO,
		BIRDY,
		FLAMER,
		ELECTR,
		SPRING,
		ROCKET,
		STOP,
		BLOCK,
		CUSTOM_22,
		CUSTOM_23,
		CUSTOM_24,
		CUSTOM_25,
		CUSTOM_26,
		CUSTOM_27,
		CUSTOM_28,
		CUSTOM_29,
		CUSTOM_30,
		CUSTOM_31,
		CUSTOM_32,
		MAX
	};
};

#define MAX_VOLUME   100
enum class MusicTempo { NORMAL = 128, FAST = 80 };


// Variables

#if defined(WIZ) || defined(GP2X)
EXTERN int volume;
EXTERN int volume_direction;
#endif


// Functions

void openAudio();
void closeAudio();
void playMusic(const char *fileName, bool restart = false);
void pauseMusic(bool pause);
void stopMusic();
int getMusicVolume();
void setMusicVolume(int volume);
MusicTempo getMusicTempo();
void setMusicTempo(MusicTempo tempo);
bool loadSounds(const char *fileName);
void resampleSound(int index, const char* name, int rate);
void resampleSounds();
void freeSounds();
void playSound(SE::Type se);
bool isSoundPlaying(SE::Type se);
int getSoundVolume();
void setSoundVolume(int volume);

inline void playConfirmSound() { playSound(SE::ORB); }
inline bool isValidSoundIndex(SE::Type index) { return (index >= SE::NONE && index < SE::MAX); }

#endif
