
/*
 *
 * planet.c
 * Created on the 23rd of August 2005
 *
 * Part of the OpenJazz project
 *
 *
 * Copyright (c) 2005 Alister Thomson
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
 * Deals with the loading, displaying and freeing of the planet landing
 * sequence.
 *
 */


#include "OpenJazz.h"


int loadPlanet (char * fn) {

  FILE *f;

  f = fopenFromPath(fn);

  if (f == NULL) return FAILURE;

  // To do

  fclose(f);

  return SUCCESS;

}


void freePlanet (void) {

  // To do

  return;

}


int runPlanet (char * fn) {

  if (loadPlanet(fn)) return FAILURE;

  while (1) {

    if (loop() == QUIT) {

      freePlanet();

      return QUIT;

    }

    if (controls[C_ESCAPE].state == SDL_PRESSED) {

      releaseControl(C_ESCAPE);

      freePlanet();

      return SUCCESS;

    }

    // To do

  }

  return SUCCESS;

}
