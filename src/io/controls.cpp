
/**
 *
 * @file controls.cpp
 *
 * Part of the OpenJazz project
 *
 * @section History
 * 23rd August 2005: Created main.c
 * 22nd July 2008: Created util.c from parts of main.c
 * 3rd February 2009: Renamed main.c to main.cpp
 * 13th July 2009: Created controls.cpp from parts of main.cpp
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
 * @section Description
 * Deals with input.
 *
 */


#include "controls.h"
#include "gfx/video.h"

#include "loop.h"

#if defined(CAANOO) || defined(WIZ) || defined(GP2X)
#include "platforms/wiz.h"
#endif


/**
 * Set up the default controls.
 */
Controls::Controls () {

	int count;

	keys[C_UP].key     = SDLK_UP;
	keys[C_DOWN].key   = SDLK_DOWN;
	keys[C_LEFT].key   = SDLK_LEFT;
	keys[C_RIGHT].key  = SDLK_RIGHT;
#if defined(DINGOO)
	keys[C_JUMP].key   = SDLK_LCTRL;
	keys[C_FIRE].key   = SDLK_LALT;
	keys[C_CHANGE].key = SDLK_LSHIFT;
	keys[C_ENTER].key  = SDLK_LCTRL;
	keys[C_ESCAPE].key = SDLK_ESCAPE;
	keys[C_STATS].key  = SDLK_TAB;
	keys[C_PAUSE].key  = SDLK_RETURN;
	keys[C_YES].key    = SDLK_LCTRL;
	keys[C_NO].key     = SDLK_LALT;
#else
	#ifdef WIN32
	keys[C_JUMP].key   = SDLK_RALT;
	keys[C_FIRE].key   = SDLK_SPACE;
	#else
	keys[C_JUMP].key   = SDLK_SPACE;
	keys[C_FIRE].key   = SDLK_LALT;
	#endif
	#ifdef ANDROID
	keys[C_CHANGE].key = SDLK_RETURN;
	keys[C_ENTER].key  = SDLK_SPACE;
	#else
	keys[C_CHANGE].key = SDLK_RCTRL;
	keys[C_ENTER].key  = SDLK_RETURN;
	#endif
	keys[C_ESCAPE].key = SDLK_ESCAPE;
	keys[C_STATS].key  = SDLK_F9;
	keys[C_PAUSE].key  = SDLK_p;
	keys[C_YES].key    = SDLK_y;
	keys[C_NO].key     = SDLK_n;
#endif
	keys[C_SWIM].key   = keys[C_JUMP].key;


#if defined(CAANOO) || defined(WIZ) || defined(GP2X)
	buttons[C_UP].button            = GP2X_BUTTON_UP;
	buttons[C_DOWN].button          = GP2X_BUTTON_DOWN;
	buttons[C_LEFT].button          = GP2X_BUTTON_LEFT;
	buttons[C_RIGHT].button         = GP2X_BUTTON_RIGHT;
	buttons[C_JUMP].button          = GP2X_BUTTON_A;
	buttons[C_FIRE].button          = GP2X_BUTTON_X;
	buttons[C_CHANGE].button        = GP2X_BUTTON_Y;
	buttons[C_ENTER].button         = GP2X_BUTTON_R;
	buttons[C_ESCAPE].button        = GP2X_BUTTON_L;
	buttons[C_STATS].button         = GP2X_BUTTON_SELECT;
	buttons[C_PAUSE].button         = GP2X_BUTTON_START;
#else
	buttons[C_UP].button = -1;
	buttons[C_DOWN].button = -1;
	buttons[C_LEFT].button = -1;
	buttons[C_RIGHT].button = -1;
	buttons[C_JUMP].button = 1;
	buttons[C_FIRE].button = 0;
	buttons[C_CHANGE].button = 3;
	buttons[C_ENTER].button = 0;
	buttons[C_ESCAPE].button = -1;
	buttons[C_STATS].button = -1;
	buttons[C_PAUSE].button = -1;
	buttons[C_YES].button = -1;
	buttons[C_NO].button = -1;
#endif
	buttons[C_SWIM].button = buttons[C_JUMP].button;


	axes[C_UP].axis = 1;
	axes[C_UP].direction = false;
	axes[C_DOWN].axis = 1;
	axes[C_DOWN].direction = true;
	axes[C_LEFT].axis = 0;
	axes[C_LEFT].direction = false;
	axes[C_RIGHT].axis = 0;
	axes[C_RIGHT].direction = true;
	axes[C_JUMP].axis = -1;
	axes[C_SWIM].axis = -1;
	axes[C_FIRE].axis = -1;
	axes[C_CHANGE].axis = -1;
	axes[C_ENTER].axis = -1;
	axes[C_ESCAPE].axis = -1;
	axes[C_STATS].axis = -1;
	axes[C_PAUSE].axis = -1;
	axes[C_YES].axis = -1;
	axes[C_NO].axis = -1;


	for (count = 0; count < CONTROLS; count++) {

		keys[count].pressed = false;
		buttons[count].pressed = false;
		axes[count].pressed = false;

		controls[count].time = 0;
		controls[count].state = false;

	}

	cursorPressed = false;
	cursorReleased = false;

	return;

}


/**
 * Set the key to use for the specified control.
 *
 * @param control The control
 * @param key The key to use
 */
void Controls::setKey (int control, int key) {

	keys[control].key = key;
	keys[control].pressed = false;

	return;

}


/**
 * Set the button to use for the specified control.
 *
 * @param control The control
 * @param button The button to use
 */
void Controls::setButton (int control, int button) {

	buttons[control].button = button;
	buttons[control].pressed = false;

	return;

}


/**
 * Set the axis and direction to use for the specified control.
 *
 * @param control The control
 * @param axis The axis to use
 * @param direction Whether or not to use positive axis values
 */
void Controls::setAxis (int control, int axis, bool direction) {

	axes[control].axis = axis;
	axes[control].direction = direction;
	axes[control].pressed = false;

	return;

}


/**
 * Get the key being used for the specified control.
 *
 * @param control The control
 *
 * @return The key being used
 */
int Controls::getKey (int control) {

	return keys[control].key;

}


/**
 * Get the button being used for the specified control.
 *
 * @param control The control
 *
 * @return The button being used
 */
int Controls::getButton (int control) {

	return buttons[control].button;

}


/**
 * Get the axis being used for the specified control.
 *
 * @param control The control
 *
 * @return The axis being used
 */
int Controls::getAxis (int control) {

	return axes[control].axis;

}


/**
 * Get the direction of the axis being used for the specified control.
 *
 * @param control The control
 *
 * @return True if positive values of the axis are being used
 */
int Controls::getAxisDirection (int control) {

	return axes[control].direction;

}


/**
 * Set the position and state of the cursor.
 *
 * @param x The x-coordinate of the cursor
 * @param y The y-coordinate of the cursor
 * @param pressed The state of the cursor
 */
void Controls::setCursor(int x, int y, bool pressed) {

	cursorX = x;
	cursorY = y;
	cursorPressed = pressed;
	cursorReleased = !pressed;

	return;

}


/**
 * Update controls based on a system event.
 *
 * @param event The system event. Non-input events will be ignored
 * @param type Type of loop. Normal, typing, or input configuration
 *
 * @return Error code
 */
int Controls::update (SDL_Event *event, LoopType type) {

	int count;

	count = CONTROLS;

	switch (event->type) {

		case SDL_KEYDOWN:

			if (type == SET_KEY_LOOP) return event->key.keysym.sym;

			for (count = 0; count < CONTROLS; count++)
				if (event->key.keysym.sym == keys[count].key)
					keys[count].pressed = true;

			if (type == TYPING_LOOP) return event->key.keysym.sym;

			break;

		case SDL_KEYUP:

			for (count = 0; count < CONTROLS; count++)
				if (event->key.keysym.sym == keys[count].key)
					keys[count].pressed = false;

			break;

		case SDL_JOYBUTTONDOWN:

			if (type == SET_JOYSTICK_LOOP) return JOYSTICKB | event->jbutton.button;

			for (count = 0; count < CONTROLS; count++)
				if (event->jbutton.button == buttons[count].button)
					buttons[count].pressed = true;

			break;

		case SDL_JOYBUTTONUP:

			for (count = 0; count < CONTROLS; count++)
				if (event->jbutton.button == buttons[count].button)
					buttons[count].pressed = false;

			break;

		case SDL_JOYAXISMOTION:

			if (type == SET_JOYSTICK_LOOP) {

				if (event->jaxis.value < -16384)
					return JOYSTICKANEG | event->jaxis.axis;
				else if (event->jaxis.value > 16384)
					return JOYSTICKAPOS | event->jaxis.axis;

			}

			for (count = 0; count < CONTROLS; count++)
				if (event->jaxis.axis == axes[count].axis) {

					if (!axes[count].direction && (event->jaxis.value < -16384))
						axes[count].pressed = true;
					else if (axes[count].direction && (event->jaxis.value > 16384))
						axes[count].pressed = true;
					else
						axes[count].pressed = false;

				}

			break;

		case SDL_MOUSEMOTION:

			if (event->motion.state & SDL_BUTTON(1)) {

				setCursor(event->motion.x, event->motion.y, true);

			}

			break;

		case SDL_MOUSEBUTTONDOWN:

			if (event->button.button == SDL_BUTTON_LEFT) {

				setCursor(event->button.x, event->button.y, true);

			} else if (event->button.button == 4) {

				wheelUp++;

			} else if (event->button.button == 5) {

				wheelDown++;

			}

			break;

		case SDL_MOUSEBUTTONUP:

			if (event->button.button == SDL_BUTTON_LEFT) {

				setCursor(event->button.x, event->button.y, false);

			}

			break;

	}

	return E_NONE;

}


/**
 * Process input iteration.
 *
 * Called once per game iteration. Updates input.
 */
void Controls::loop () {

	int count;

	// Apply controls to universal control tracking
	for (count = 0; count < CONTROLS; count++)
		controls[count].state = (controls[count].time < globalTicks) &&
			(keys[count].pressed || buttons[count].pressed || axes[count].pressed);

	if (wheelUp) {

		controls[C_UP].state = true;
		wheelUp--;

	}

	if (wheelDown) {

		controls[C_DOWN].state = true;
		wheelDown--;

	}

	return;

}


/**
 * Determine whether or not the specified control is being used.
 *
 * @param control The control
 *
 * @return True if the control is being used
 */
bool Controls::getState (int control) {

	return controls[control].state;

}


/**
 * If it's being used, release the specified control.
 *
 * @param control The control
 *
 * @return True if the control was being used
 */
bool Controls::release (int control) {

	if (!controls[control].state) return false;

	controls[control].time = globalTicks + T_KEY;
	controls[control].state = false;

	return true;

}


/**
 * Get the position of the cursor, and determine whether or not it's being used.
 *
 * @param x Is set to the x-coordinate of the cursor
 * @param y Is set to the y-coordinate of the cursor
 *
 * @return True if the cursor was being used
 */
bool Controls::getCursor (int& x, int& y) {

#ifdef SCALE
	int scaleFactor = video.getScaleFactor();

	x = cursorX / scaleFactor;
	y = cursorY / scaleFactor;
#else
	x = cursorX;
	y = cursorY;
#endif

	return cursorPressed || cursorReleased;

}


/**
 * Determine whether or not the cursor has been released.
 *
 * @return True if the cursor has been released
 */
bool Controls::wasCursorReleased () {

	if (cursorReleased) {

		cursorReleased = false;

		return true;

	}

	return false;

}

