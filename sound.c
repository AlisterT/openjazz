
/*
 *
 * sound.h
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
 * Deals with the loading, playing and freeing of music and sound effects.
 *
 */


#include "OpenJazz.h"


int loadMusic (char * fn) {

  FILE *f;

  f = fopenFromPath(fn);

  // To do

  fclose(f);

  // To do: Play music

  return -1;

}


void freeMusic (void) {

  // To do: Stop music

  // To do

  return;

}


int loadSounds (char *fn) {

  FILE *f;

  f = fopenFromPath(fn);

  // To do

  fclose(f);

  return -1;

}


void freeSounds (void) {

  // To do

  return;

}


void playSound (int sound) {

  // To do

  return;

}

