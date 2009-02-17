
/*
 *
 * palette.h
 *
 * Created on the 4th of February 2009 from parts of OpenJazz.h
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


// Constants

// Types of palette effect
#define PE_FADE   0 /* Fades to black, then remains black */
#define PE_ROTATE 1 /* Cyclical colour animation */

// Level background palette effects
#define PE_SKY    2 /* Transfers the appropriate section of the background
                       palette to the main palette */
#define PE_2D     8 /* Real parallaxing background */
#define PE_1D     9 /* Diagonal lines parallaxing background */
#define PE_WATER  11 /* The deeper below water, the darker it gets */


// Class

class PaletteEffect {

	private:
		PaletteEffect *next;     // Next effect to use
		unsigned char  type;     /* Type of effect, see PE constants */
		unsigned char  first;    /* The first palette index affected by the
			effect */
		unsigned char  amount;   /* The number of (consecutive) palette indices
			affected by the effect */
		fixed          speed;    /* When type is:
			PE_FADE - Number of seconds the fade lasts
			PE_ROTATE - Rotations per second
			PE_SKY - Relative Y speed - as in Jazz 2
			PE_1D - Relative X & Y speed - as in Jazz 2
			PE_2D - Relative X & Y speed - as in Jazz 2
			PE_WATER - Number of pixels between water surface and total darkness
			*/
		fixed          position; /* When type is:
			PE_FADE - Brightness of palette in 0-1 range
			PE_ROTATE - Number of colours rotated
			PE_SKY - Position taken from player's viewport coordinates, in
				pixels
			PE_1D, PE_2D - Position taken from player's viewport coordinates as
				short ints (in pixels), then packed into the fixed's 4 bytes
			PE_WATER - Position of the water surface in pixels */

	public:
		PaletteEffect             (unsigned char newType,
			unsigned char newFirst, unsigned char newAmount, fixed newSpeed,
			PaletteEffect * nextPE);
		~PaletteEffect            ();
		unsigned char getType     ();
		void          setPosition (fixed newPosition);
		void          apply       (SDL_Color *shownPalette, bool direct);

};


