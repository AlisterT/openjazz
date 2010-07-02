
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

#include "io/gfx/video.h"


JJ2Event* JJ2Event::step (int msps) {

	if (next) next = next->step(msps);

	return this;

}


void JJ2Event::draw (int change) {

	int drawX, drawY;

	if (next) next->draw(change);

	drawX = getDrawX(change);
	drawY = getDrawY(change);

	if (type > 2) drawRect(FTOI(drawX) + 8, FTOI(drawY) + 8, 16, 16, type);

	return;

}

