
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

#ifndef _SOUND_H
#define _SOUND_H


#include "OpenJazz.h"


// Constants

// Sound effects
#define S_INVULN   1
#define S_MACHGUN  2
#define S_BOOM     3
#define S_OW       4
#define S_YUM      5
#define S_FIRE     6
#define S_UPLOOP   7
#define S_1UP      8
#define S_PHOTON   9
#define S_WAIT    10
#define S_ORB     11
#define S_JUMPA   12
#define S_GODLIKE 13
#define S_YEAHOO  14
#define S_BIRDY   15
#define S_FLAMER  16
#define S_ELECTR  17
#define S_SPRING  18
#define S_ROCKET  19
#define S_STOP    20
#define S_BLOCK   21

#define MAX_VOLUME   100
#define MUSIC_NORMAL   0
#define MUSIC_FAST     1


// Datatype

/// Raw sound effect data
typedef struct {

	unsigned char *data;
	char          *name;
	int            length;

} RawSound;


/// Resampled sound effect data
typedef struct {

	unsigned char *data;
	int            length;
	int            position;

} Sound;


// Variables

EXTERN RawSound *rawSounds;
EXTERN int       nRawSounds;
EXTERN Sound    *sounds;

#if defined(WIZ) || defined(GP2X)
EXTERN int volume;
EXTERN int volume_direction;
#endif


// Functions

EXTERN void openAudio      ();
EXTERN void closeAudio     ();
EXTERN void playMusic      (const char *fileName, bool restart = false);
EXTERN void pauseMusic     (bool pause);
EXTERN void stopMusic      ();
EXTERN int  getMusicVolume ();
EXTERN void setMusicVolume (int volume);
EXTERN int  getMusicTempo  ();
EXTERN void setMusicTempo  (int tempo);
EXTERN int  loadSounds     (const char *fileName);
EXTERN void resampleSound  (int index, const char* name, int rate);
EXTERN void resampleSounds ();
EXTERN void freeSounds     ();
EXTERN void playSound      (char index);
EXTERN bool isSoundPlaying (char index);
EXTERN int  getSoundVolume ();
EXTERN void setSoundVolume (int volume);

#endif

