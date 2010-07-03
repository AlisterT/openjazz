
/*
 *
 * bullet.cpp
 *
 * 11th February 2009: Created bullet.cpp from parts of events.cpp
 *
 * Part of the OpenJazz project
 *
 *
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


#include "bullet.h"
#include "event/event.h"
#include "level.h"

#include "game/game.h"
#include "io/gfx/paletteeffects.h"
#include "io/gfx/sprite.h"
#include "io/gfx/video.h"
#include "player/bird.h"
#include "player/levelplayer.h"


Bullet::Bullet (LevelPlayer* sourcePlayer, bool lower, unsigned int ticks) {

	Anim* anim;

	// Properties based on the player

	source = sourcePlayer;
	type = source->player->getAmmo(false) + 1;

	if (!lower && (level->getBullet(type)[B_XSPEED | 2] != 0)) {

		// Create the other bullet
		next = new Bullet(source, true, ticks);

	} else {

		next = level->bullets;

	}

	direction = source->getFacing()? 1: 0;
	direction |= lower? 2: 0;

	if (type == 4) {

		// TNT
		type = -1;

		sprite = level->getSprite(130);
		dx = 0;
		dy = 0;
		time = ticks + T_TNT;

		// Red flash
		level->flash(255, 0, 0, T_TNT);

	} else {

		sprite = level->getSprite(((unsigned char *)level->getBullet(type)) [B_SPRITE + direction]);
		dx = level->getBullet(type)[B_XSPEED + direction] * 500 * F1;
		dy = level->getBullet(type)[B_YSPEED + direction] * 250 * F1;
		time = ticks + T_BULLET;

		level->playSound(level->getBullet(type)[B_STARTSOUND]);

	}

	anim = level->getAnim(source->getAnim());
	x = source->getX() + anim->getShootX() + PXO_MID - F4;
	y = source->getY() + anim->getShootY() - F4;

	return;

}


Bullet::Bullet (Event* sourceEvent, bool facing, unsigned int ticks) {

	Anim* anim;

	// Properties based on the event

	next = level->bullets;
	source = NULL;
	type = sourceEvent->getProperty(E_BULLET);
	direction = facing? 1: 0;
	sprite = level->getSprite(((unsigned char *)level->getBullet(type))[B_SPRITE + direction]);

	anim = level->getAnim(sourceEvent->getProperty(facing? E_LSHOOTANIM: E_RSHOOTANIM));
	x = sourceEvent->getX() + anim->getShootX();
	y = sourceEvent->getY() + anim->getShootY() - ITOF((sprite->getHeight() / 2) - 2);
	dx = level->getBullet(type)[B_XSPEED + direction] * 500 * F1;
	dy = level->getBullet(type)[B_YSPEED + direction] * 250 * F1;
	time = ticks + T_BULLET;

	level->playSound(level->getBullet(type)[B_STARTSOUND]);

	return;

}


Bullet::Bullet (Bird* sourceBird, bool lower, unsigned int ticks) {

	// Properties based on the bird and its player

	source = sourceBird->getPlayer();

	if (!lower) {

		// Create the other bullet
		next = new Bullet(sourceBird, true, ticks);

	} else {

		next = level->bullets;

	}

	type = 30;
	direction = source->getFacing()? 1: 0;
	direction |= lower? 2: 0;
	sprite = level->getSprite(((unsigned char *)level->getBullet(type))[B_SPRITE + direction]);
	x = sourceBird->getX() + (source->getFacing()? PXO_R: PXO_L);
	y = sourceBird->getY();
	dx = level->getBullet(type)[B_XSPEED + direction] * 500 * F1;
	dy = level->getBullet(type)[B_YSPEED + direction] * 250 * F1;
	time = ticks + T_BULLET;

	level->playSound(level->getBullet(type)[B_STARTSOUND]);

	return;

}


Bullet::~Bullet () {

	if (next) delete next;

	return;

}


Bullet* Bullet::remove () {

	Bullet* oldNext;

	oldNext = next;
	next = NULL;
	delete this;

	return oldNext;

}


LevelPlayer* Bullet::getSource () {

	return source;

}


Bullet* Bullet::step (unsigned int ticks, int msps) {

	signed char* set;
	Event* event;
	int count;

	// Process the next bullet
	if (next) next = next->step(ticks, msps);


	if (level->getStage() != LS_END) {


		// If the time has expired, destroy the bullet
		if (ticks > time) {

			// If the bullet is TNT, hit all destructible events nearby twice
			if (type == -1) {

				event = level->getEvents();

				while (event) {

					// If the event is within range, hit it
					if (event->overlap(x - F160, y - F100, 2 * F160, 2 * F100)) {

						event->hit(source, ticks);
						event->hit(source, ticks);

					}

					event = event->getNext();

				}

			}

			// Destroy the bullet
			return remove();

		}


		// If this is TNT, don't need to do anything else
		if (type == -1) return this;


		// Check if a player has been hit
		for (count = 0; count < nPlayers; count++) {

			if (players[count].getLevelPlayer()->overlap(x, y,
				ITOF(sprite->getWidth()), ITOF(sprite->getHeight()))) {

				// If the hit was successful, destroy the bullet
				if (players[count].getLevelPlayer()->hit(source? source->player: NULL, ticks)) return remove();

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
					if (event->hit(source, ticks)) return remove();

				}

				event = event->getNext();

			}

		}


	}


	set = level->getBullet(type);

	// If the scenery has been hit and this is not a bouncer, destroy the bullet
	if (level->checkMaskUp(x, y) && (set[B_BEHAVIOUR] != 4)) {

		level->playSound(set[B_FINISHSOUND]);

		return remove();

	}


	// Calculate trajectory
	if (set[B_BEHAVIOUR] == 4) {

		// Bounce the bullet
		if (level->checkMaskDown(x, y - F4) && (dy < 0)) dy = 0;
		else if (level->checkMaskDown(x, y + F4)) dy = -600 * F1;
		else if (level->checkMaskDown(x - F4, y)) direction |= 1;
		else if (level->checkMaskDown(x + F4, y)) direction &= ~1;
		else dy += 6400 * msps * set[B_GRAVITY];

	} else dy += 6400 * msps * set[B_GRAVITY];


	// Apply trajectory
	x += (dx * msps) >> 10;
	y += (dy * msps) >> 10;


	// Do not destroy the bullet
	return this;

}


void Bullet::draw (int change) {

	if (next) next->draw(change);

	// Show the bullet
	sprite->draw(FTOI(getDrawX(change)), FTOI(getDrawY(change)), false);

	return;

}



