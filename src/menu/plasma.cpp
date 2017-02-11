
/**
 *
 * @file plasma.cpp
 *
 * Part of the OpenJazz project
 *
 * @section History
 * 23rd June 2010: Created plasma.cpp
 *
 * @section Licence
 * Copyright (c) 2010 Alireza Nejati
 *
 * OpenJazz is distributed under the terms of
 * the GNU General Public License, version 2.0
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 * @section Description
 * Cool plasma effects for the main menu.
 *
 */


#include "plasma.h"

#include "level/level.h"
#include "util.h"
#include "io/gfx/video.h"
#include <SDL/SDL.h>

#ifdef SCALE
	#include "io/gfx/scale2x/scalebit.h"
#endif


/**
 * Create the plasma.
 */
Plasma::Plasma(){

	p0=0;
	p1=0;
	p2=0;
	p3=0;

	//fSin, fCos: pi = 512
	// -1024 < out < 1024
}

/**
 * Draw the plasma.
 *
 * @return Error code
 */
int Plasma::draw(){
	int x,y;

	int t1,t2,t3,t4;
	int w,h,pitch;
	unsigned char *px;
	unsigned char colour;
	unsigned int colb;

	// draw plasma

	SDL_LockSurface(canvas);

	w 		= canvas->w;
	h 		= canvas->h;
	pitch 	= canvas->pitch;

	px = (unsigned char *)canvas->pixels;

    t1 = p0;
    t2 = p1;
    for(y=0;y<h;y++){
        t3 = p2;
        t4 = p3;
		colb = (fCos(t1*4)<<3)+(fCos(t2*4)<<3)+(32<<10);
        for(x=0;x<w;x++){

			colour = ((colb+(fCos(t3*4)<<3)+(fCos(t4*4)<<3))>>10) & 0xF;

            t3 += 3;
            t4 += 2;

            px[x] = colour;
		}
		// go to next row
		px += pitch;
        t1 += 2;
        t2 += 1;
	}

	p0 = p0 < 256 ? p0+1 : 1;
	p1 = p1 < 256 ? p1+2 : 2;
	p2 = p2 < 256 ? p2+3 : 3;
	p3 = p3 < 256 ? p3+4 : 4;

	SDL_UnlockSurface(canvas);

	return E_NONE;

}

