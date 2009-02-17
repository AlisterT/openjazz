
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

	source = sourcePlayer;
	type = source->getAmmo(false) + 1;
	direction = source->getFacing()? 1: 0;
	direction |= lower? 2: 0;
	dy = levelInst->getBullet(type)[B_GRAVITY + direction] * F1;
	time = ticks + 1000;
	x = source->getX() + (source->getFacing()? PXO_R: PXO_L);
	y = source->getY() - F8;

	if (type == 4) {

		// TNT
		type = -1;
		dy = 0;
		time = ticks + 3000;

	}

	if (!lower && (type == 2)) {

		// Create the other RF missile
		next = new Bullet(source, true, ticks, nextBullet);

	} else {

		next = nextBullet;

	}

	return;

}


Bullet::Bullet (Event *sourceEvent, int ticks, Bullet *nextBullet) {

	next = nextBullet;

	if (sourceEvent->getProperty(E_LEFTANIM) == 51) source = players;
	else source = NULL;

	type = sourceEvent->getProperty(E_BULLET);
	direction = sourceEvent->getFacing()? 1: 0;
	dy = levelInst->getBullet(type)[B_GRAVITY + direction] * F1;
	time = ticks + 1000;

	if (sourceEvent->getFacing())
		x = sourceEvent->getX() + sourceEvent->getWidth() + F8;
	else x = sourceEvent->getX() - F8;

	y = sourceEvent->getY() + (sourceEvent->getHeight() >> 1);

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

					if ((cy >= 0) && (cy < 64) && (cx >= 0) && (cx < 256)) {

						if (levelInst->getEvent(cx, cy)[E_HITSTOKILL])
							levelInst->getGrid(cx, cy)->hits = 254;

					}

				}

			}

		}

		// Destroy the bullet
		return true;

	}


	// If this is TNT, don't need to do anything else
	if (type == -1) return false;


	set = levelInst->getBullet(type);

	// If an obstacle has been hit and this is not a bouncer, destroy the bullet
	if (levelInst->checkMask(x, y) && (set[B_BEHAVIOUR] != 4)) return true;


	// Calculate trajectory
	if (set[B_BEHAVIOUR] == 4) {

		// Bounce the bullet
		if (levelInst->checkMaskDown(x, y - F4) && (dy < 0)) dy = 0;
		else if (levelInst->checkMaskDown(x, y + F4)) dy = -600 * F1;
		else if (levelInst->checkMaskDown(x - F4, y)) direction |= 1;
		else if (levelInst->checkMaskDown(x + F4, y)) direction &= ~1;
		else dy += 3200 * mspf;

	} else if (set[B_BEHAVIOUR] >= 2) {

		dy += 3200 * mspf;

	} else {

		dy = set[B_YSPEED + direction] * 250 * F1;

	}


	// Apply trajectory
	x += (set[B_XSPEED + direction] * 500 * F1 * mspf) >> 10;
	y += (dy * mspf) >> 10;


	// Do not destroy the bullet
	return false;

}


void Bullet::draw () {

	SDL_Rect dst;
	Player *localPlayer;
	Sprite *bulletSprite;

	if (next) next->draw();

	localPlayer = players;

	if (type == -1) bulletSprite = levelInst->getSprite(130);
	else bulletSprite = levelInst->getSprite(levelInst->getBullet(type)
			[B_SPRITE + direction]);

	dst.x = (x >> 10) - (bulletSprite->pixels->w >> 1) -
		(localPlayer->viewX >> 10);
	dst.y = (y >> 10) - (bulletSprite->pixels->h >> 1) -
		(localPlayer->viewY >> 10);

	// Show the bullet
	SDL_BlitSurface(bulletSprite->pixels, NULL, screen, &dst);

	return;

}



