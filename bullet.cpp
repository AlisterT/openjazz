
/*
 *
 * bullet.cpp
 *
 * Created on the 11th of February 2009 from parts of events.cpp
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

#include "OpenJazz.h"


Bullet::Bullet (Player *sourcePlayer, bool lower, int ticks,
	Bullet *nextBullet) {

	// Properties based on the player

	source = sourcePlayer;
	type = source->getAmmo(false) + 1;

	if (!lower && (level->getBullet(type)[B_XSPEED | 2] != 0)) {

		// Create the other bullet
		next = new Bullet(source, true, ticks, nextBullet);

	} else {

		next = nextBullet;

	}

	direction = source->getFacing()? 1: 0;
	direction |= lower? 2: 0;
	x = source->getX() + (source->getFacing()? PXO_R: PXO_L);
	y = source->getY() - F8;

	if (type == 4) {

		// TNT
		type = -1;
		dy = 0;
		time = ticks + T_TNT;

	} else {

		dy = level->getBullet(type)[B_YSPEED + direction] * 250 * F1;
		time = ticks + T_BULLET;

	}

	return;

}


Bullet::Bullet (Event *sourceEvent, int ticks, Bullet *nextBullet) {

	// Properties based on the event

	next = nextBullet;
	source = NULL;
	type = sourceEvent->getProperty(E_BULLET);
	direction = sourceEvent->getFacing()? 1: 0;

	x = sourceEvent->getX() + (sourceEvent->getWidth() >> 1);
	y = sourceEvent->getY() + (sourceEvent->getHeight() >> 1);
	dy = level->getBullet(type)[B_YSPEED + direction] * 250 * F1;
	time = ticks + T_BULLET;


	return;

}


Bullet::Bullet (Bird *sourceBird, bool lower, int ticks,
	Bullet *nextBullet) {

	// Properties based on the bird and its player

	source = sourceBird->getPlayer();

	if (!lower) {

		// Create the other bullet
		next = new Bullet(sourceBird, true, ticks, nextBullet);

	} else {

		next = nextBullet;

	}

	type = 30;
	direction = source->getFacing()? 1: 0;
	direction |= lower? 2: 0;
	x = sourceBird->getX() + (source->getFacing()? PXO_R: PXO_L);
	y = sourceBird->getY();
	dy = level->getBullet(type)[B_YSPEED + direction] * 250 * F1;
	time = ticks + T_BULLET;

	return;

}


Bullet::~Bullet () {

	return;

}


Bullet * Bullet::getNext () {

	return next;

}


void Bullet::removeNext () {

	Bullet *newNext;

	if (next) {

		newNext = next->getNext();
		delete next;
		next = newNext;

	}

	return;

}


Player * Bullet::getSource () {

	return source;

}


bool Bullet::isIn (fixed left, fixed top, fixed width, fixed height) {

	return (x >= left) && (x < left + width) && (y >= top) &&
		(y < top + height);

}


bool Bullet::playFrame (int ticks) {

	unsigned char buffer[MTL_G_ROAST];
	signed char *set;
	int cx, cy;

	// Process the next bullet
	if (next) {

		if (next->playFrame(ticks)) removeNext();

	}


	// If the time has expired, destroy the bullet
	if (ticks > time) {

		// If the bullet is TNT, kill all killable events nearby
		if (type == -1) {

			for (cy = (y - F160) >> 15; cy < (y + F160) >> 15; cy++) {

				for (cx = (x - F100) >> 15; cx < (x + F100) >> 15; cx++) {

					if ((cy >= 0) && (cy < 64) && (cx >= 0) && (cx < 256))
						level->hitEvent(cx, cy, true);

				}

			}

		}

		// Destroy the bullet
		return true;

	}


	// If this is TNT, don't need to do anything else
	if (type == -1) return false;


	set = level->getBullet(type);

	// If a player has been hit, destroy the bullet
	for (cx = 0; cx < nPlayers; cx++) {

		if (players[cx].isIn(x, y, F1, F1)) {

			if (!source) {

				// The bullet came from an event

				// If the hit was successful, destroy the bullet
				if (players[cx].hit(ticks)) return true;

			}

			if (source->getTeam() != players[cx].getTeam()) {

				// The bullet came from another player or their bird

				// If the hit was successful, destroy the bullet
				if (players[cx].hit(ticks)) {

					if ((players + cx == localPlayer) &&
						!localPlayer->getEnergy()) {

						// If the player has been roasted, inform server/clients

						buffer[0] = MTL_G_ROAST;
						buffer[1] = MT_G_ROAST;
						buffer[2] = source->getTeam();
						game->send(buffer);

						// If it is the server's player, inform self
						if (localPlayer == players) {

							for (cy = 0; cy < nPlayers; cy++) {

								if (players[cy].getTeam() == buffer[2])
									players[cy].teamScore++;

							}

						}

					}

					return true;

				}

			}

		}

	}


	// If an obstacle has been hit and this is not a bouncer, destroy the bullet
	if (level->checkMask(x, y) && (set[B_BEHAVIOUR] != 4)) return true;


	// Calculate trajectory
	if (set[B_BEHAVIOUR] == 4) {

		// Bounce the bullet
		if (level->checkMaskDown(x, y - F4) && (dy < 0)) dy = 0;
		else if (level->checkMaskDown(x, y + F4)) dy = -600 * F1;
		else if (level->checkMaskDown(x - F4, y)) direction |= 1;
		else if (level->checkMaskDown(x + F4, y)) direction &= ~1;
		else dy += 6400 * mspf * set[B_GRAVITY];

	} else dy += 6400 * mspf * set[B_GRAVITY];


	// Apply trajectory
	x += (set[B_XSPEED + direction] * 500 * F1 * mspf) >> 10;
	y += (dy * mspf) >> 10;


	// Do not destroy the bullet
	return false;

}


void Bullet::draw () {

	Sprite *sprite;

	if (next) next->draw();

	if (type == -1) sprite = level->getSprite(130);
	else sprite =
		level->getSprite(((unsigned char *)level->getBullet(type))
			[B_SPRITE + direction]);

	// Show the bullet
	sprite->draw((x >> 10) - (sprite->getWidth() >> 1) - (viewX >> 10),
		(y >> 10) - (sprite->getWidth() >> 1) - (viewY >> 10));

	return;

}



