
/**
 *
 * @file sound.h
 *
 * Part of the OpenJazz project
 *
 * @section History
 * 23rd August 2005: Created OpenJazz.h
 * 2nd June 2009: Created sound.h from parts of OpenJazz.h
 *
 * @section Licence
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

#ifndef _SOUND_H
#define _SOUND_H


#include "OpenJazz.h"


// Constants

// Sound effects
#define S_INVULN   0
#define S_MACHGUN  1
#define S_BOOM     2
#define S_OW       3
#define S_YUM      4
#define S_FIRE     5
#define S_UPLOOP   6
#define S_1UP      7
#define S_PHOTON   8
#define S_WAIT     9
#define S_ORB     10
#define S_JUMPA   11
#define S_GODLIKE 12
#define S_YEAHOO  13
#define S_BIRDY   14
#define S_FLAMER  15
#define S_ELECTR  16
#define S_SPRING  17
#define S_ROCKET  18
#define S_STOP    19
#define S_BLOCK   20

#define MAX_VOLUME 100


// Datatype

/// Sound effect
typedef struct {

	unsigned char *data;
	char          *name;
	int            length;
	int            position;

} Sound;


// Variables

EXTERN Sound *sounds;
EXTERN int    nSounds;
EXTERN char   soundsVolume;

#if defined(WIZ) || defined(GP2X)
EXTERN int volume;
EXTERN int volume_direction;
#endif


// Functions

EXTERN void openAudio  ();
EXTERN void closeAudio ();
EXTERN void playMusic  (const char *fileName);
EXTERN void stopMusic  ();
EXTERN int  loadSounds (const char *fileName);
EXTERN void freeSounds ();
EXTERN void playSound  (int sound);

#endif

