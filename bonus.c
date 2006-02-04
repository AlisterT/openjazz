
/*
 *
 * bonus.c
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
 * Deals with the loading, running and freeing of bonus levels.
 *
 */


#include "OpenJazz.h"


int loadBonus (char * fn) {

  FILE *f;

  f = fopenFromPath(fn);

  if (f == NULL) return FAILURE;

  // To do

  fclose(f);

  return SUCCESS;

}


void freeBonus (void) {

  // To do

  return;

}


int runBonus (char * fn) {

  if (loadBonus(fn)) return FAILURE;

  while (1) {

    if (loop() == QUIT) {

      freeBonus();

      return QUIT;

    }

    if (controls[C_ESCAPE].state == SDL_PRESSED) {

      releaseControl(C_ESCAPE);

      freeBonus();

      return SUCCESS;

    }

    // To do

  }

  return SUCCESS;

}
