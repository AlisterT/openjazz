
/**
 *
 * @file wiz.h
 *
 * Part of the OpenJazz project
 *
 * @par Licence:
 * OpenJazz is distributed under the terms of
 * the GNU General Public License, version 2.0
 *
 */


#ifndef _WIZ_H
#define _WIZ_H

#if defined(WIZ) || defined(GP2X)

#define GP2X_BUTTON_VOLUP           (16)
#define GP2X_BUTTON_VOLDOWN         (17)

#define VOLUME_MIN 0
#define VOLUME_MAX 100
#define VOLUME_CHANGE_RATE 2
#define VOLUME_NOCHG 0
#define VOLUME_DOWN 1
#define VOLUME_UP 2

void WIZ_AdjustVolume( int direction );

#endif

#endif
