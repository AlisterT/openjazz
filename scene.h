
/*
 *
 * scene.h
 *
 * Created on the 3rd of February 2009 from parts of scene.c
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


#ifndef _SCENE_H
#define _SCENE_H


#include "OpenJazz.h"


// Class

class Scene {

	private:
		SDL_Surface *sceneBGs[1];
		SDL_Color    scenePalette[256];

	public:
		Scene   (char * fileName);
		~Scene  ();

		int run ();

};

#endif

