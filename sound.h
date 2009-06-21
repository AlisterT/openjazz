
/*
 *
 * sound.h
 *
 * Created on the 2nd of June 2009 from parts of OpenJazz.h
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


// Datatype

typedef struct {

	unsigned char *data;
	char          *name;
	int            length;
	int            position;

} Sound;


// Variables

Extern Sound *sounds;
Extern int    nSounds;


// Functions

Extern void openAudio  ();
Extern void closeAudio ();
Extern void playMusic  (char *fileName);
Extern void stopMusic  ();
Extern int  loadSounds (char *fileName);
Extern void freeSounds ();
Extern void playSound  (int sound);

#endif

