
/**
 *
 * @file jj1bullet.cpp
 *
 * Part of the OpenJazz project
 *
 * @par History:
 * - 23rd August 2005: Created level.c
 * - 1st January 2006: Created events.c from parts of level.c
 * - 3rd February 2009: Renamed events.c to events.cpp
 * - 11th February 2009: Created bullet.cpp from parts of events.cpp
 * - 1st August 2012: Renamed bullet.cpp to jj1bullet.cpp
 *
 * @par Licence:
 * Copyright (c) 2005-2012 Alister Thomson
 *
 * OpenJazz is distributed under the terms of
 * the GNU General Public License, version 2.0
 *
 */


#include "jj1bullet.h"
#include "jj1event/jj1event.h"
#include "jj1level.h"
#include "jj1levelplayer/jj1bird.h"
#include "jj1levelplayer/jj1levelplayer.h"

#include "game/game.h"
#include "io/gfx/sprite.h"
#include "io/gfx/video.h"
#include "io/sound.h"

#include <stdlib.h>


/**
 * Generic bullet constructor.
 *
 * @param nextBullet The next bullet
 * @param sourcePlayer The player that fired the bullet (if any)
 * @param startX The starting x-coordinate of the bullet
 * @param startY The starting y-coordinate of the bullet
 * @param bullet Type
 * @param newDirection The direction of the bullet
 * @param ticks Time
 */
JJ1Bullet::JJ1Bullet (JJ1Bullet* nextBullet, JJ1LevelPlayer* sourcePlayer, fixed startX, fixed startY, signed char* bullet, int newDirection, unsigned int ticks) {

	next = nextBullet;
	source = sourcePlayer;
	set = bullet;
	direction = newDirection;

	x = startX;
	y = startY;
	dx = set[B_XSPEED + direction] << 18;
	dy = set[B_YSPEED + direction] << 16;

	if (source) {

		if ((newDirection & 1) == 0) {

			if (dx > 0) dx = -dx;

		} else {

			if (dx < 0) dx = -dx;

		}

		if (set[B_BEHAVIOUR] == 4) dx += source->getXSpeed();

	}

	sprite = level->getSprite(((unsigned char *)set)[B_SPRITE + direction]);

	time = ticks + T_BULLET;

	return;

}


/**
 * Delete all bullets.
 */
JJ1Bullet::~JJ1Bullet () {

	if (next) delete next;

	return;

}


/**
 * Delete this bullet.
 *
 * @return The next bullet
 */
JJ1Bullet* JJ1Bullet::remove () {

	JJ1Bullet* oldNext;

	oldNext = next;
	next = NULL;
	delete this;

	return oldNext;

}


/**
 * Get the player responsible for this bullet.
 *
 * @return The player (NULL if fired by an event)
 */
JJ1LevelPlayer* JJ1Bullet::getSource () {

	return source;

}


/**
 * Bullet iteration.
 *
 * @param ticks Time
 *
 * @return Remaining bullet
 */
JJ1Bullet* JJ1Bullet::step (unsigned int ticks) {

	JJ1Event* event;
	int count;

	// Process the next bullet
	if (next) next = next->step(ticks);


	if (level->getStage() != LS_END) {

		// If the time has expired, destroy the bullet
		if (ticks > time) return remove();


		// Check if a player has been hit
		for (count = 0; count < nPlayers; count++) {

			if (players[count].getJJ1LevelPlayer()->overlap(x, y,
				ITOF(sprite->getWidth()), ITOF(sprite->getHeight()))) {

				// If the hit was successful, destroy the bullet
				if (players[count].getJJ1LevelPlayer()->hit(source? source->player: NULL, ticks)) return remove();

			}

		}


		if (source) {

			// Check if an event has been hit

			event = level->getEvents();

			while (event) {

				// Check if the event has been hit
				if (event->overlap(x, y,
					ITOF(sprite->getWidth()), ITOF(sprite->getHeight()))) {

					// If the event is hittable, hit it and destroy the bullet
					if (event->hit(source, 1, ticks)) return remove();

				}

				event = event->getNext();

			}

		}


	}


	// If the scenery has been hit and this is not a bouncer, destroy the bullet
	if (level->checkMaskUp(x, y) && (set[B_BEHAVIOUR] != 4)) {

		playSound(set[B_FINISHSOUND]);

		return remove();

	}


	// Calculate trajectory
	if (set[B_BEHAVIOUR] == 4) {

		if (level->checkMaskDown(x, y - F1)) {

			// Bounce the bullet away from a vertical surface
			if (dx < 0) direction |= 1;
			else direction &= ~1;

			dx = -dx;
			dy = 0;

		}

		if (level->checkMaskDown(x, y + (dy >> 6) - F1)) {

			// Bounce the bullet against a horizontal surface
			if (dy < 0) dy = 0;
			else dy = -dy - (abs(dx - (set[B_XSPEED + direction] << 18)) >> 1);

		} else {

			// Respond to gravity
			dy += F32 * set[B_GRAVITY];

		}

	} else {

		dy += F32 * set[B_GRAVITY];

		if (source && (abs(source->getXSpeed() + dx) > abs(dx))) {

			x += source->getXSpeed() >> 6;

		}

	}


	// Apply trajectory
	x += dx >> 6;
	y += dy >> 6;


	// Do not destroy the bullet
	return this;

}


/**
 * Draw the bullet.
 *
 * @param change Time since last iteration
 */
void JJ1Bullet::draw (int change) {

	if (next) next->draw(change);

	// Show the bullet
	sprite->draw(FTOI(getDrawX(change)), FTOI(getDrawY(change)), false);

	return;

}



