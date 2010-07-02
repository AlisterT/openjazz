
/*
 *
 * jj2eventframe.cpp
 *
 * 2nd July 2010: Created jj2eventframe.cpp from parts of jj2level.cpp
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
 * Deals with the playing of JJ2 levels.
 *
 */


#include "jj2event.h"
#include "../jj2level.h"

#include "io/gfx/anim.h"
#include "io/gfx/video.h"
#include "player/jj2levelplayer.h"


JJ2Event* JJ2Event::step (int ticks, int msps) {

	JJ2LevelPlayer *levelPlayer;
	int count;

	if (next) next = next->step(ticks, msps);


	// Handle behaviour

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


	// Handle contact with player

	for (count = 0; count < nPlayers; count++) {

		levelPlayer = players[count].getJJ2LevelPlayer();

		// Check if the player is touching the event
		if (levelPlayer->overlap(x, y, F32, F32)) {

			// If the player picks up the event, destroy it
			if (levelPlayer->touchEvent(this, ticks, msps))
				destroy(ticks);

		}

	}


	return this;

}


void JJ2Event::draw (int change) {

	Anim* an;
	int drawX, drawY;

	if (next) next->draw(change);

	// Don't draw if too far off-screen
	if ((x < viewX - F64) || (y < viewY - F64) ||
		(x > viewX + ITOF(viewW) + F64) || (y > viewY + ITOF(viewH) + F64)) return;

	drawX = getDrawX(change);
	drawY = getDrawY(change);

	switch (type) {

		case 60: // Frozen green spring

			an = jj2Level->getAnim(35);

			break;

		case 62: // Spring crate

			an = jj2Level->getAnim(37);

			break;

		case 83: // Checkpoint

			an = jj2Level->getAnim(49);

			break;

		case 85: // Red spring

			an = jj2Level->getAnim(32);

			break;

		case 86: // Green spring

			an = jj2Level->getAnim(35);

			break;

		case 87: // Blue spring

			an = jj2Level->getAnim(37);

			break;

		default:

			drawRect(FTOI(drawX) + 8, FTOI(drawY) + 8, 16, 16, type);

			return;

	}

	an->draw(drawX, drawY);

	return;

}

