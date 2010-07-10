
/*
 *
 * bonusplayer.cpp
 *
 * 24th June 2010: Created bonusplayer.cpp from parts of player.cpp and
 *                 playerframe.cpp
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

/*
 * Deals with players in bonus levels.
 *
 */


#include "bonusplayer.h"

#include "bonus/bonus.h"
#include "io/controls.h"
#include "io/gfx/sprite.h"
#include "io/gfx/video.h"
#include "util.h"

#include <string.h>


BonusPlayer::BonusPlayer (Player* parent, Anim **newAnims, unsigned char startX, unsigned char startY) {

	int count;


	player = parent;

	memcpy(anims, newAnims, BPANIMS * sizeof(Anim*));

	x = TTOF(startX) + F16;
	y = TTOF(startY) + F16;

	direction = FQ;
	dr = 0;
	gems = 0;


	// Create the player's palette

	for (count = 0; count < 256; count++)
		palette[count].r = palette[count].g = palette[count].b = count;

	// TODO: Custom colours


	return;

}


BonusPlayer::~BonusPlayer () {

	return;

}


void BonusPlayer::addGem () {

	gems++;

	return;

}


fixed BonusPlayer::getDirection () {

	return direction;

}


int BonusPlayer::getGems () {

	return gems;

}


fixed BonusPlayer::getX () {

	return x;

}


fixed BonusPlayer::getY () {

	return y;

}


void BonusPlayer::step (unsigned int ticks, int msps, Bonus* bonus) {

	fixed cdx, cdy;

	// Bonus stages use polar coordinates for movement (but not position)

	if (player->getControl(C_UP)) {

		// Walk/run forwards

		if (dr < 0) dr += PRA_REVERSE * msps;
		else if (dr < PRS_RUN) dr += PRA_RUN * msps;

		animType = PA_WALK;

	} else if (player->getControl(C_DOWN)) {

		// Walk/run back

		if (dr > 0) dr -= PRA_REVERSE * msps;
		else if (dr > PRS_REVERSE) dr -= PRA_RUN * msps;

		animType = PA_WALK;

	} else {

		// Slow down

		if (dr > 0) {

			if (dr < PRA_STOP * msps) dr = 0;
			else dr -= PRA_STOP * msps;

		}

		if (dr < 0) {

			if (dr > -PRA_STOP * msps) dr = 0;
			else dr += PRA_STOP * msps;

		}

		animType = PA_OTHER;

	}

	if (player->getControl(C_LEFT)) {

		direction -= msps >> 2;
		animType = PA_LEFT;

	}

	if (player->getControl(C_RIGHT)) {

		direction += msps >> 2;
		animType = PA_RIGHT;

	}


	// Apply trajectory
	cdx = (MUL(fSin(direction), dr) * msps) >> 10;
	cdy = (MUL(-fCos(direction), dr) * msps) >> 10;

	if (!bonus->checkMask(x + cdx, y)) x += cdx;
	if (!bonus->checkMask(x, y + cdy)) y += cdy;

	return;

}


void BonusPlayer::draw (unsigned int ticks) {

	Anim* anim;

	anim = anims[animType];
	anim->disableDefaultOffset();
	anim->setFrame(ticks / 75, true);
	if (canvasW <= SW) anim->draw(ITOF((canvasW - anim->getWidth()) >> 1), ITOF(canvasH - anim->getHeight() - 28));
	else anim->drawScaled(ITOF(canvasW >> 1), ITOF(canvasH - ((((anim->getHeight() >> 1) + 28) * canvasW) / SW)), ITOF(canvasW) / SW);

	return;

}

