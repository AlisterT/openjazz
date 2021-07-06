
/**
 *
 * @file controls.cpp
 *
 * Part of the OpenJazz project
 *
 * @par History:
 * - 23rd August 2005: Created main.c
 * - 22nd July 2008: Created util.c from parts of main.c
 * - 3rd February 2009: Renamed main.c to main.cpp
 * - 13th July 2009: Created controls.cpp from parts of main.cpp
 *
 * @par Licence:
 * Copyright (c) 2005-2017 Alister Thomson
 *
 * OpenJazz is distributed under the terms of
 * the GNU General Public License, version 2.0
 *
 * @par Description:
 * Deals with input.
 *
 */


#include "controls.h"
#include "gfx/video.h"

#include "loop.h"

#define DEFAULT_KEY_UP                  (SDLK_UP)
#define DEFAULT_KEY_DOWN                (SDLK_DOWN)
#define DEFAULT_KEY_LEFT                (SDLK_LEFT)
#define DEFAULT_KEY_RIGHT               (SDLK_RIGHT)
#if defined (_WIN32)
    #define DEFAULT_KEY_JUMP            (SDLK_RALT)
    #define DEFAULT_KEY_SWIM            (SDLK_RALT)
    #define DEFAULT_KEY_FIRE            (SDLK_SPACE)
    #define DEFAULT_KEY_CHANGE          (SDLK_RCTRL)
    #define DEFAULT_KEY_ENTER           (SDLK_RETURN)
    #define DEFAULT_KEY_ESCAPE          (SDLK_ESCAPE)
    #define DEFAULT_KEY_STATS           (SDLK_F9)
    #define DEFAULT_KEY_PAUSE           (SDLK_p)
    #define DEFAULT_KEY_YES             (SDLK_y)
    #define DEFAULT_KEY_NO              (SDLK_n)
#elif defined(DINGOO)
    #define DEFAULT_KEY_JUMP            (SDLK_LCTRL)
    #define DEFAULT_KEY_SWIM            (SDLK_LCTRL)
    #define DEFAULT_KEY_FIRE            (SDLK_LALT)
    #define DEFAULT_KEY_CHANGE          (SDLK_LSHIFT)
    #define DEFAULT_KEY_ENTER           (SDLK_LCTRL)
    #define DEFAULT_KEY_ESCAPE          (SDLK_ESCAPE)
    #define DEFAULT_KEY_STATS           (SDLK_TAB)
    #define DEFAULT_KEY_PAUSE           (SDLK_RETURN)
    #define DEFAULT_KEY_YES             (SDLK_LCTRL)
    #define DEFAULT_KEY_NO              (SDLK_LALT)
#elif defined (ANDROID)
    #define DEFAULT_KEY_JUMP            (SDLK_SPACE)
    #define DEFAULT_KEY_SWIM            (SDLK_SPACE)
    #define DEFAULT_KEY_FIRE            (SDLK_LALT)
    #define DEFAULT_KEY_CHANGE          (SDLK_RETURN)
    #define DEFAULT_KEY_ENTER           (SDLK_SPACE)
    #define DEFAULT_KEY_ESCAPE          (SDLK_ESCAPE)
    #define DEFAULT_KEY_STATS           (SDLK_F9)
    #define DEFAULT_KEY_PAUSE           (SDLK_p)
    #define DEFAULT_KEY_YES             (SDLK_y)
    #define DEFAULT_KEY_NO              (SDLK_n)
#elif defined(GAMESHELL)
    #define DEFAULT_KEY_JUMP            (SDLK_j)      /* A button */
    #define DEFAULT_KEY_SWIM            (SDLK_j)      /* A button */
    #define DEFAULT_KEY_FIRE            (SDLK_k)      /* B button */
    #define DEFAULT_KEY_CHANGE          (SDLK_o)      /* Y button */
    #define DEFAULT_KEY_ENTER           (SDLK_RETURN) /* START button */
    #define DEFAULT_KEY_ESCAPE          (SDLK_ESCAPE) /* MENU button */
    #define DEFAULT_KEY_STATS           (SDLK_SPACE)  /* SELECT button */
    #define DEFAULT_KEY_PAUSE           (SDLK_p)
    #define DEFAULT_KEY_YES             (SDLK_y)
    #define DEFAULT_KEY_NO              (SDLK_n)
#else
    #define DEFAULT_KEY_JUMP            (SDLK_SPACE)
    #define DEFAULT_KEY_SWIM            (SDLK_SPACE)
    #define DEFAULT_KEY_FIRE            (SDLK_LALT)
    #define DEFAULT_KEY_CHANGE          (SDLK_RCTRL)
    #define DEFAULT_KEY_ENTER           (SDLK_RETURN)
    #define DEFAULT_KEY_ESCAPE          (SDLK_ESCAPE)
    #define DEFAULT_KEY_STATS           (SDLK_F9)
    #define DEFAULT_KEY_PAUSE           (SDLK_p)
    #define DEFAULT_KEY_YES             (SDLK_y)
    #define DEFAULT_KEY_NO              (SDLK_n)
#endif

/* These are optional */
#define DEFAULT_KEY_BLASTER             (SDLK_1)
#define DEFAULT_KEY_TOASTER             (SDLK_2)
#define DEFAULT_KEY_MISSILE             (SDLK_3)
#define DEFAULT_KEY_BOUNCER             (SDLK_4)
#define DEFAULT_KEY_TNT                 (SDLK_5)

#if defined(GP2X) || defined(WIZ)
    #define DEFAULT_BUTTON_UP           (0)
    #define DEFAULT_BUTTON_DOWN         (4)
    #define DEFAULT_BUTTON_LEFT         (2)
    #define DEFAULT_BUTTON_RIGHT        (6)
    #define DEFAULT_BUTTON_JUMP         (12)    /* A */
    #define DEFAULT_BUTTON_SWIM         (12)    /* A */
    #define DEFAULT_BUTTON_FIRE         (14)    /* X */
    #define DEFAULT_BUTTON_CHANGE       (15)    /* Y */
    #define DEFAULT_BUTTON_ESCAPE       (10)    /* L */
    #define DEFAULT_BUTTON_ENTER        (11)    /* R */
    #define DEFAULT_BUTTON_PAUSE        (8)     /* Start */
    #define DEFAULT_BUTTON_STATS        (9)     /* Select */
    #define DEFAULT_BUTTON_YES          (-1)
    #define DEFAULT_BUTTON_NO           (-1)
#elif defined(CAANOO)
    #define DEFAULT_BUTTON_UP           (-1)
    #define DEFAULT_BUTTON_DOWN         (-1)
    #define DEFAULT_BUTTON_LEFT         (-1)
    #define DEFAULT_BUTTON_RIGHT        (-1)
    #define DEFAULT_BUTTON_JUMP         (0)     /* A? */
    #define DEFAULT_BUTTON_SWIM         (0)     /* A? */
    #define DEFAULT_BUTTON_FIRE         (1)     /* X? */
    #define DEFAULT_BUTTON_CHANGE       (3)     /* Y? */
    #define DEFAULT_BUTTON_ESCAPE       (6)     /* Home */
    #define DEFAULT_BUTTON_ENTER        (5)     /* R? */
    #define DEFAULT_BUTTON_PAUSE        (9)     /* Help 2 */
    #define DEFAULT_BUTTON_STATS        (8)     /* Help 1 */
    #define DEFAULT_BUTTON_YES          (-1)
    #define DEFAULT_BUTTON_NO           (-1)
#elif defined(PSP)
    #define DEFAULT_BUTTON_UP           (8)
    #define DEFAULT_BUTTON_DOWN         (6)
    #define DEFAULT_BUTTON_LEFT         (7)
    #define DEFAULT_BUTTON_RIGHT        (9)
    #define DEFAULT_BUTTON_JUMP         (2)
    #define DEFAULT_BUTTON_SWIM         (2)
    #define DEFAULT_BUTTON_FIRE         (3)
    #define DEFAULT_BUTTON_CHANGE       (0)
    #define DEFAULT_BUTTON_ENTER        (5)
    #define DEFAULT_BUTTON_ESCAPE       (4)
    #define DEFAULT_BUTTON_STATS        (10)
    #define DEFAULT_BUTTON_PAUSE        (11)
    #define DEFAULT_BUTTON_YES          (-1)
    #define DEFAULT_BUTTON_NO           (-1)
#elif defined(WII)
    #define DEFAULT_BUTTON_UP           (-1)
    #define DEFAULT_BUTTON_DOWN         (-1)
    #define DEFAULT_BUTTON_LEFT         (-1)
    #define DEFAULT_BUTTON_RIGHT        (-1)
    #define DEFAULT_BUTTON_JUMP         (2)
    #define DEFAULT_BUTTON_SWIM         (2)
    #define DEFAULT_BUTTON_FIRE         (3)
    #define DEFAULT_BUTTON_CHANGE       (0)
    #define DEFAULT_BUTTON_ENTER        (3)
    #define DEFAULT_BUTTON_ESCAPE       (6)
    #define DEFAULT_BUTTON_STATS        (4)
    #define DEFAULT_BUTTON_PAUSE        (5)
    #define DEFAULT_BUTTON_YES          (-1)
    #define DEFAULT_BUTTON_NO           (-1)
#elif defined (_3DS)
    #define DEFAULT_BUTTON_UP           (-1)
    #define DEFAULT_BUTTON_DOWN         (-1)
    #define DEFAULT_BUTTON_LEFT         (-1)
    #define DEFAULT_BUTTON_RIGHT        (-1)
    #define DEFAULT_BUTTON_JUMP         (2)    /* B */
    #define DEFAULT_BUTTON_SWIM         (2)    /* B */
    #define DEFAULT_BUTTON_FIRE         (4)    /* Y */
    #define DEFAULT_BUTTON_CHANGE       (3)    /* X */
    #define DEFAULT_BUTTON_ENTER        (1)    /* A */
    #define DEFAULT_BUTTON_ESCAPE       (7)    /* Select */
    #define DEFAULT_BUTTON_STATS        (5)    /* L */
    #define DEFAULT_BUTTON_PAUSE        (8)    /* Start */
    #define DEFAULT_BUTTON_YES          (-1)
    #define DEFAULT_BUTTON_NO           (-1)
#elif defined(__vita__)
    #define DEFAULT_BUTTON_UP           (8)
    #define DEFAULT_BUTTON_DOWN         (6)
    #define DEFAULT_BUTTON_LEFT         (7)
    #define DEFAULT_BUTTON_RIGHT        (9)
    #define DEFAULT_BUTTON_JUMP         (2)
    #define DEFAULT_BUTTON_SWIM         (1)
    #define DEFAULT_BUTTON_FIRE         (3)
    #define DEFAULT_BUTTON_CHANGE       (0)
    #define DEFAULT_BUTTON_ENTER        (5)
    #define DEFAULT_BUTTON_ESCAPE       (4)
    #define DEFAULT_BUTTON_STATS        (10)
    #define DEFAULT_BUTTON_PAUSE        (11)
    #define DEFAULT_BUTTON_YES          (-1)
    #define DEFAULT_BUTTON_NO           (-1)
#else
    #define DEFAULT_BUTTON_UP           (-1)
    #define DEFAULT_BUTTON_DOWN         (-1)
    #define DEFAULT_BUTTON_LEFT         (-1)
    #define DEFAULT_BUTTON_RIGHT        (-1)
    #define DEFAULT_BUTTON_JUMP         (1)
    #define DEFAULT_BUTTON_SWIM         (1)
    #define DEFAULT_BUTTON_FIRE         (0)
    #define DEFAULT_BUTTON_CHANGE       (3)
    #define DEFAULT_BUTTON_ENTER        (0)
    #define DEFAULT_BUTTON_ESCAPE       (5)
    #define DEFAULT_BUTTON_STATS        (-1)
    #define DEFAULT_BUTTON_PAUSE        (4)
    #define DEFAULT_BUTTON_YES          (-1)
    #define DEFAULT_BUTTON_NO           (-1)
#endif


/**
 * Set up the default controls.
 */
Controls::Controls () {

	int count;

	keys[C_UP].key = DEFAULT_KEY_UP;
	keys[C_DOWN].key = DEFAULT_KEY_DOWN;
	keys[C_LEFT].key = DEFAULT_KEY_LEFT;
	keys[C_RIGHT].key = DEFAULT_KEY_RIGHT;
	keys[C_JUMP].key = DEFAULT_KEY_JUMP;
	keys[C_SWIM].key = DEFAULT_KEY_SWIM;
	keys[C_FIRE].key = DEFAULT_KEY_FIRE;
	keys[C_CHANGE].key = DEFAULT_KEY_CHANGE;
	keys[C_ENTER].key = DEFAULT_KEY_ENTER;
	keys[C_ESCAPE].key = DEFAULT_KEY_ESCAPE;
	keys[C_BLASTER].key = DEFAULT_KEY_BLASTER;
	keys[C_TOASTER].key = DEFAULT_KEY_TOASTER;
	keys[C_MISSILE].key = DEFAULT_KEY_MISSILE;
	keys[C_BOUNCER].key = DEFAULT_KEY_BOUNCER;
	keys[C_TNT].key = DEFAULT_KEY_TNT;
	keys[C_STATS].key = DEFAULT_KEY_STATS;
	keys[C_PAUSE].key = DEFAULT_KEY_PAUSE;
	keys[C_YES].key = DEFAULT_KEY_YES;
	keys[C_NO].key = DEFAULT_KEY_NO;


	buttons[C_UP].button = DEFAULT_BUTTON_UP;
	buttons[C_DOWN].button = DEFAULT_BUTTON_DOWN;
	buttons[C_LEFT].button = DEFAULT_BUTTON_LEFT;
	buttons[C_RIGHT].button = DEFAULT_BUTTON_RIGHT;
	buttons[C_JUMP].button = DEFAULT_BUTTON_JUMP;
	buttons[C_SWIM].button = DEFAULT_BUTTON_SWIM;
	buttons[C_FIRE].button = DEFAULT_BUTTON_FIRE;
	buttons[C_CHANGE].button = DEFAULT_BUTTON_CHANGE;
	buttons[C_ENTER].button = DEFAULT_BUTTON_ENTER;
	buttons[C_ESCAPE].button = DEFAULT_BUTTON_ESCAPE;
	buttons[C_BLASTER].button = -1;
	buttons[C_TOASTER].button = -1;
	buttons[C_MISSILE].button = -1;
	buttons[C_BOUNCER].button = -1;
	buttons[C_TNT].button = -1;
	buttons[C_STATS].button = DEFAULT_BUTTON_STATS;
	buttons[C_PAUSE].button = DEFAULT_BUTTON_PAUSE;
	buttons[C_YES].button = DEFAULT_BUTTON_YES;
	buttons[C_NO].button = DEFAULT_BUTTON_NO;


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
	axes[C_BLASTER].axis = -1;
	axes[C_TOASTER].axis = -1;
	axes[C_MISSILE].axis = -1;
	axes[C_BOUNCER].axis = -1;
	axes[C_TNT].axis = -1;
	axes[C_STATS].axis = -1;
	axes[C_PAUSE].axis = -1;
	axes[C_YES].axis = -1;
	axes[C_NO].axis = -1;


	hats[C_UP].hat = 0;
	hats[C_UP].direction = SDL_HAT_UP;
	hats[C_DOWN].hat = 0;
	hats[C_DOWN].direction = SDL_HAT_DOWN;
	hats[C_LEFT].hat = 0;
	hats[C_LEFT].direction = SDL_HAT_LEFT;
	hats[C_RIGHT].hat = 0;
	hats[C_RIGHT].direction = SDL_HAT_RIGHT;
	hats[C_JUMP].hat = -1;
	hats[C_SWIM].hat = -1;
	hats[C_FIRE].hat = -1;
	hats[C_CHANGE].hat = -1;
	hats[C_ENTER].hat = -1;
	hats[C_ESCAPE].hat = -1;
	hats[C_BLASTER].hat = -1;
	hats[C_TOASTER].hat = -1;
	hats[C_MISSILE].hat = -1;
	hats[C_BOUNCER].hat = -1;
	hats[C_TNT].hat = -1;
	hats[C_STATS].hat = -1;
	hats[C_PAUSE].hat = -1;
	hats[C_YES].hat = -1;
	hats[C_NO].hat = -1;


	for (count = 0; count < CONTROLS; count++) {

		keys[count].pressed = false;
		buttons[count].pressed = false;
		axes[count].pressed = false;
		hats[count].pressed = false;

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
 * Set the hat and direction to use for the specified control.
 *
 * @param control The control
 * @param hat The hat to use
 * @param direction The direction to use
 */
void Controls::setHat (int control, int hat, int direction) {

	hats[control].hat = hat;
	hats[control].direction = direction;
	hats[control].pressed = false;

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
 * Get the hat being used for the specified control.
 *
 * @param control The control
 *
 * @return The hat being used
 */
int Controls::getHat (int control) {

	return hats[control].hat;

}


/**
 * Get the direction of the hat being used for the specified control.
 *
 * @param control The control
 *
 * @return hat direction
 */
int Controls::getHatDirection (int control) {

	return hats[control].direction;

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

		case SDL_JOYHATMOTION:

			if (type == SET_JOYSTICK_LOOP) {

				switch(event->jhat.value) {
					case SDL_HAT_UP:
						return JOYSTICKHUP  | event->jhat.hat;
					case SDL_HAT_LEFT:
						return JOYSTICKHLFT | event->jhat.hat;
					case SDL_HAT_RIGHT:
						return JOYSTICKHRHT | event->jhat.hat;
					case SDL_HAT_DOWN:
						return JOYSTICKHDWN | event->jhat.hat;
				}
			}

			for(count = 0; count < CONTROLS; count++)
				if (event->jhat.hat == hats[count].hat) {

					if (hats[count].direction & event->jhat.value)
						hats[count].pressed = true;
					else
						hats[count].pressed = false;

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
			(keys[count].pressed || buttons[count].pressed ||
			axes[count].pressed || hats[count].pressed);

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

