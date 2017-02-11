
/**
 *
 * @file controls.h
 *
 * Part of the OpenJazz project
 *
 * @section History
 * 23rd August 2005: Created OpenJazz.h
 * 13th July 2009: Created controls.h from parts of OpenJazz.h
 *
 * @section Licence
 * Copyright (c) 2005-2012 Alister Thomson
 *
 * OpenJazz is distributed under the terms of
 * the GNU General Public License, version 2.0
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */

#ifndef _INPUT_H
#define _INPUT_H


#include "loop.h"
#include "OpenJazz.h"

#include <SDL/SDL.h>


// Constants

// Indexes for the keys / buttons / axes player controls arrays
#define C_UP      0
#define C_DOWN    1
#define C_LEFT    2
#define C_RIGHT   3
#define C_JUMP    4
#define C_SWIM    5
#define C_FIRE    6
#define C_CHANGE  7 /* Change weapon */
#define C_ENTER   8
#define C_ESCAPE  9
#define C_STATS  10
#define C_PAUSE  11
#define C_YES    12
#define C_NO     13
// Size of those arrays
#define CONTROLS 14

// Time interval
#define T_KEY   200


// Class

/// Keeps track of all control input
class Controls {

	private:
		struct {

			int  key; ///< Keyboard key
			bool pressed; ///< Whether or not the key is pressed

		} keys[CONTROLS];

		struct {

			int  button; ///< Joystick button
			bool pressed; ///< Whether or not the button is pressed

		} buttons[CONTROLS];

		struct {

			int  axis; ///< Joystick axis
			bool direction; ///< Axis direction
			bool pressed; ///< Whether or not the axis is pressed in the given direction

		} axes[CONTROLS];

		struct {

			unsigned int time; ///< The time from which the control will respond to being pressed
			bool         state; ///< Whether or not the control is being used

		} controls[CONTROLS];

		int          cursorX; ///< X-coordinate of the cursor
		int          cursorY; ///< Y-coordinate of the cursor
		bool         cursorPressed; ///< Whether or not the cursor is being pressed
		bool         cursorReleased; ///< Whether or not the cursor has been released
		int          wheelUp; ///< How many times the wheel has been scrolled upwards
		int          wheelDown; ///< How many times the wheel has been scrolled downwards

		void setCursor (int x, int y, bool pressed);

	public:
		Controls ();

		void setKey           (int control, int key);
		void setButton        (int control, int button);
		void setAxis          (int control, int axis, bool direction);
		int  getKey           (int control);
		int  getButton        (int control);
		int  getAxis          (int control);
		int  getAxisDirection (int control);

		int  update           (SDL_Event *event, LoopType type);
		void loop             ();

		bool getState          (int control);
		bool release           (int control);
		bool getCursor         (int& x, int& y);
		bool wasCursorReleased ();

};


// Variable

EXTERN Controls controls;

#endif
