
/**
 *
 * @file jj2eventframe.cpp
 *
 * Part of the OpenJazz project
 *
 * @section History
 * 2nd July 2010: Created jj2eventframe.cpp from parts of jj2level.cpp
 *
 * @section Licence
 * Copyright (c) 2005-2010 Alister Thomson
 *
 * OpenJazz is distributed under the terms of
 * the GNU General Public License, version 2.0
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 * @section Description
 * Deals with the playing of JJ2 levels.
 *
 */


#include "jj2event.h"
#include "../jj2level.h"
#include "../jj2levelplayer/jj2levelplayer.h"

#include "io/gfx/anim.h"
#include "io/gfx/font.h"
#include "io/gfx/video.h"


/// Look-up table for ammo animations (in animSet 1)
const unsigned char ammoAnims[] = {
	28, // Ice
	24, // Bouncer
	33, // Seeker
	48, // RF
	56, // Toaster
	58, // TNT
	61, // Pellets
	67 // Sparks
};

/// Look-up table for food etc. animations (in animSet 67)
const unsigned char pickupAnims[] = {
	0, // 0
	0, // 1
	0, // 2
	0, // 3
	0, // 4
	0, // 5
	0, // 6
	0, // 7
	0, // 8
	0, // 9
	0, // 10
	0, // 11
	0, // 12
	0, // 13
	0, // 14
	0, // 15
	0, // 16
	0, // 17
	0, // 18
	0, // 19
	0, // 20
	0, // 21
	0, // 22
	0, // 23
	0, // 24
	0, // 25
	0, // 26
	0, // 27
	0, // 28
	0, // 29
	0, // 30
	0, // 31
	0, // 32
	0, // 33
	0, // 34
	0, // 35
	0, // 36
	0, // 37
	0, // 38
	0, // 39
	0, // 40
	0, // 41
	0, // 42
	0, // 43
	84, // Silver coin
	37, // Gold coin
	0, // 46
	0, // 47
	0, // 48
	0, // 49
	0, // 50
	0, // 51
	0, // 52
	55, // Ice crate
	54, // Bouncer crate
	56, // Seeker crate
	57, // RF crate
	58, // Toaster crate
	90, // Armed TNT
	36, // Board
	0, // 60
	29, // Rapid fire (can also be 30)
	0, // 62
	35, // Red gem
	35, // Green gem
	35, // Blue gem
	35, // Purple gem
	34, // Large red gem
	0, // 68
	3, // Ammo barrel
	0, // 70
	0, // 71
	82, // Energy
	72, // Full energy
	31, // Fire shield
	10, // Bubble shield
	51, // Plasma shield
	0, // 77
	0, // 78
	33, // High jump
	0, // 1-up
	28, // exit signpost
	0, // 82
	14, // Checkpoint
	0, // 84
	0, // 85
	0, // 86
	0, // 87
	0, // 88
	87, // Extra time
	42, // Freeze
	0, // 91
	0, // 92
	0, // 93
	0, // 94
	52, // Trigger crate
	0, // 96
	0, // 97
	0, // 98
	0, // 99
	0, // 100
	0, // 101
	0, // 102
	0, // 103
	0, // 104
	0, // 105
	0, // 106
	0, // 107
	0, // 108
	0, // 109
	0, // 110
	0, // 111
	0, // 112
	0, // 113
	0, // 114
	0, // 115
	0, // 116
	0, // 117
	0, // 118
	0, // 119
	0, // 120
	0, // 121
	0, // 122
	0, // 123
	0, // 124
	0, // 125
	0, // 126
	0, // 127
	0, // 128
	0, // 129
	0, // 130
	60, // Blaster PU (can also be 83)
	61, // Bouncer PU
	62, // Ice PU
	63, // Seeker PU
	64, // RF PU
	65, // Toaster PU
	0, // 137
	0, // 138
	0, // 139
	0, // 140
	1, // Apple
	2, // Banana
	16, // Cherry
	71, // Orange
	74, // Pear
	79, // Pretzel
	81, // Strawberry
	0, // 148
	0, // 149
	0, // 150
	0, // 151
	0, // 152
	0, // 153
	48, // Lemon
	50, // Lime
	89, // Thing
	92, // Watermelon
	73, // Peach
	38, // Grapes
	49, // Lettuce
	26, // Aubergine
	23, // Cucumber
	75, // Jazzade
	20, // Cola
	53, // Milk
	76, // Tart
	12, // Cake
	25, // Doughnut
	24, // Cupcake
	18, // Crisps
	13, // Sweet
	19, // Chocolate
	43, // Ice cream
	11, // Burger
	77, // Pizza
	32, // Chips
	17, // Chicken drumstick
	80, // Sandwich
	88, // Taco
	91, // Hot dog
	39, // Ham
	15, // Cheese
	0, // 183
	0, // 184
	0, // 185
	0, // 186
	0, // 187
	0, // 188
	0, // 189
	0, // 190
	0, // 191
	0, // 192
	0, // 193
	0, // 194
	0, // 195
	0, // 196
	0, // 197
	0, // 198
	0, // 199
	0, // 200
	0, // 201
	0, // 202
	0, // 203
	0, // 204
	0, // 205
	0, // 206
	0, // 207
	0, // 208
	0, // 209
	0, // 210
	0, // 211
	0, // 212
	0, // 213
	0, // 214
	0, // 215
	0, // 216
	0, // 217
	0, // 218
	0, // 219
	66, // Pellet PU
	67, // Sparks PU
};


bool JJ2Event::prepareStep (unsigned int ticks, int msps) {

	JJ2LevelPlayer *levelPlayer;
	int count;


	// Process next event(s)
	if (next) next = next->step(ticks, msps);


	// If the reaction time has expired
	if (endTime && (ticks > endTime)) {

		return true;

	}


	if (endTime) return false;


	// Handle contact with player

	for (count = 0; count < nPlayers; count++) {

		levelPlayer = players[count].getJJ2LevelPlayer();

		// Check if the player is touching the event
		if (levelPlayer->overlap(x, y, F32, F32)) {

			// If the player picks up the event, destroy it
			if (levelPlayer->touchEvent(this, ticks, msps)) destroy(ticks);

		}

	}

	return false;

}


bool JJ2Event::prepareDraw (unsigned int ticks, int change) {

	// Draw next event(s)
	if (next) next->draw(ticks, change);

	// Don't draw if too far off-screen
	if ((x < viewX - F64) || (y < viewY - F64) ||
		(x > viewX + ITOF(canvasW) + F64) || (y > viewY + ITOF(canvasH) + F64)) return true;

	return false;

}


JJ2Event* PickupJJ2Event::step (unsigned int ticks, int msps) {

	if (prepareStep(ticks, msps)) return remove();

	if (!floating) {

		if (jj2Level->checkMaskDown(x, y + F4, false)) {

			dy += 1000 / msps;

		} else {

			dy = 0;

		}

	} else {

		/// @todo Check for bullet overlap
		// floating = false;

	}

	return this;

}


void AmmoJJ2Event::draw (unsigned int ticks, int change) {

	Anim* an;
	int drawX, drawY;

	if (prepareDraw(ticks, change)) return;

	drawX = getDrawX(change);
	drawY = getDrawY(change);

	/// @todo Check if ammo is powered up
	if (!endTime)an = jj2Level->getAnim(0, ammoAnims[type - 33] + 1, flipped);
	else an = jj2Level->getAnim(67, 86, flipped);

	an->setFrame((int)ticks / 60, true);
	an->draw(drawX + F16, drawY + F16 + F32);

	return;

}


void CoinGemJJ2Event::mapPalette (Anim* anim, int start) {

	SDL_Color palette[256];
	int count;

	for (count = 0; count < 112; count++)
		palette[count + 128].r = palette[count + 128].g = palette[count + 128].b = start + 6 - (count >> 4);
	for (; count < 128; count++)
		palette[count + 128].r = palette[count + 128].g = palette[count + 128].b = 255;

	anim->setPalette(palette, 128, 128);

	return;

}


void CoinGemJJ2Event::draw (unsigned int ticks, int change) {

	Anim* an;
	int drawX, drawY;

	if (prepareDraw(ticks, change)) return;

	drawX = getDrawX(change);
	drawY = getDrawY(change);

	if (endTime) {

		an = jj2Level->getAnim(67, 86, flipped);
		an->setFrame((int)ticks / 60, true);
		an->draw(drawX + F16, drawY + F16 + F32);

	}

	if (type == 44) an = jj2Level->getAnim(67, 84, flipped); // Silver coin
	else if (type == 45) an = jj2Level->getAnim(67, 37, flipped); // Gold coin
	else an = jj2Level->getAnim(67, 35, flipped); // Gem

	an->setFrame((int)ticks / 60, true);

	switch (type) {

		case 63: // Red gem

			mapPalette(an, 48);

			break;

		case 64: // Green gem

			mapPalette(an, 80);

			break;

		case 65: // Blue gem

			mapPalette(an, 32);

			break;

		case 66: // Purple gem

			mapPalette(an, 88);

			break;

	}

	if (endTime) {

		drawX += (ticks + 500 - endTime) << 8;
		drawY += (ticks + 500 - endTime) << 10;

	}

	an->draw(drawX + F16, drawY + F16 + F32);

	return;

}


void FoodJJ2Event::draw (unsigned int ticks, int change) {

	Anim* an;
	int drawX, drawY;

	if (prepareDraw(ticks, change)) return;

	drawX = getDrawX(change);
	drawY = getDrawY(change);

	// Use look-up table
	if (!endTime) an = jj2Level->getAnim(67, pickupAnims[type], flipped);
	else an = jj2Level->getAnim(67, 86, flipped);

	an->setFrame((int)ticks / 60, true);
	an->draw(drawX + F16, drawY + F16 + F32);

	return;

}


JJ2Event* OtherJJ2Event::step (unsigned int ticks, int msps) {

	if (prepareStep(ticks, msps)) return remove();

	switch (type) {

		case 60: // Frozen green spring
		case 62: // Spring crate
		case 83: // Checkpoint
		case 85: // Red spring
		case 86: // Green spring
		case 87: // Blue spring

			if (!jj2Level->checkMaskDown(x, y + F1, true)) y += F1;

			break;

		default:

			break;

	}

	return this;

}


void OtherJJ2Event::draw (unsigned int ticks, int change) {

	Anim* an;
	int drawX, drawY;

	if (prepareDraw(ticks, change)) return;

	drawX = getDrawX(change);
	drawY = getDrawY(change);

	switch (type) {

		case 60: // Frozen green spring

			an = jj2Level->getAnim(92, 5, flipped);
			an->setFrame(0, true);

			break;

		case 62: // Spring crate

			an = jj2Level->getAnim(92, 0, flipped);
			an->setFrame(0, true);

			break;

		case 85: // Red spring

			an = jj2Level->getAnim(92, 7, flipped);
			an->setFrame(0, true);

			break;

		case 86: // Green spring

			an = jj2Level->getAnim(92, 5, flipped);
			an->setFrame(0, true);

			break;

		case 87: // Blue spring

			an = jj2Level->getAnim(92, 0, flipped);
			an->setFrame(0, true);

			break;

		default:

			if ((type <= 221) && pickupAnims[type]) {

				an = jj2Level->getAnim(67, pickupAnims[type], flipped);
				an->setFrame((int)ticks / 60, true);
				an->draw(drawX + F16, drawY + F16 + F32);

			} else if (!endTime) {

				drawRect(FTOI(drawX) + 8, FTOI(drawY) + 8, 16, 16, type);

			}

			panelBigFont->showNumber(type, FTOI(drawX) + 24, FTOI(drawY) + 12);

			return;

	}

	an->draw(drawX + F16, drawY + F16);

	return;

}

