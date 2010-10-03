
/**
 *
 * @file wiz.h
 *
 * Part of the OpenJazz project
 *
 * @section Licence
 * OpenJazz is distributed under the terms of
 * the GNU General Public License, version 2.0
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */


#ifndef _WIZ_H
#define _WIZ_H

#if defined(CAANOO)
#define GP2X_BUTTON_UP              (11)    // Directional dummies for Caanoo (not used)
#define GP2X_BUTTON_DOWN            (12)
#define GP2X_BUTTON_LEFT            (13)
#define GP2X_BUTTON_RIGHT           (14)
#define GP2X_BUTTON_UPLEFT          (15)
#define GP2X_BUTTON_UPRIGHT         (16)
#define GP2X_BUTTON_DOWNLEFT        (17)
#define GP2X_BUTTON_DOWNRIGHT       (18)
#define GP2X_BUTTON_CLICK           (19)
#define GP2X_BUTTON_A               (0)
#define GP2X_BUTTON_B               (2)
#define GP2X_BUTTON_X               (1)
#define GP2X_BUTTON_Y               (3)
#define GP2X_BUTTON_L               (4)
#define GP2X_BUTTON_R               (5)
#define GP2X_BUTTON_START           (9)     // Help 2
#define GP2X_BUTTON_SELECT          (8)     // Help 1
#define GP2X_BUTTON_HOME            (6)     // Home (caanoo only)
#define GP2X_BUTTON_VOLUP           (20)    // doesnt exist on caanoo, but need a define
#define GP2X_BUTTON_VOLDOWN         (21)    // same as above
#else   // WIZ || GP2X
#define GP2X_BUTTON_UP              (0)
#define GP2X_BUTTON_DOWN            (4)
#define GP2X_BUTTON_LEFT            (2)
#define GP2X_BUTTON_RIGHT           (6)
#define GP2X_BUTTON_UPLEFT          (1)
#define GP2X_BUTTON_UPRIGHT         (7)
#define GP2X_BUTTON_DOWNLEFT        (3)
#define GP2X_BUTTON_DOWNRIGHT       (5)
#define GP2X_BUTTON_CLICK           (18)
#define GP2X_BUTTON_A               (12)
#define GP2X_BUTTON_B               (13)
#define GP2X_BUTTON_X               (14)
#define GP2X_BUTTON_Y               (15)
#define GP2X_BUTTON_L               (10)
#define GP2X_BUTTON_R               (11)
#define GP2X_BUTTON_START           (8)
#define GP2X_BUTTON_SELECT          (9)
#define GP2X_BUTTON_VOLUP           (16)
#define GP2X_BUTTON_VOLDOWN         (17)
#endif

#if defined(WIZ) || defined(GP2X)
#define VOLUME_MIN 0
#define VOLUME_MAX 100
#define VOLUME_CHANGE_RATE 2
#define VOLUME_NOCHG 0
#define VOLUME_DOWN 1
#define VOLUME_UP 2

void WIZ_AdjustVolume( int direction );
#endif

#endif
