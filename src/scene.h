
/*
 *
 * scene.h
 *
 * 3rd February 2009: Created scene.h from parts of scene.c
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

#include <io/file.h>
#include <sdl.h>
// Class

class Scene {

	private:
		SDL_Surface *sceneBGs[100];
		SDL_Color    scenePalette[256];
		unsigned short int scriptItems;
		unsigned short int dataItems;
		signed long int* scriptStarts;
		signed long int* dataOffsets;
		int imageIndex;

	protected:
		void ParseScripts(File *f);
		void ParseData(File *f);
		void ParseAni(File* f);
	public:
		Scene    (char * fileName);
		~Scene   ();

		int play ();

};

#endif

