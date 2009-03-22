
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

	protected:
		PaletteEffect *next;     // Next effect to use
		unsigned char  first;    /* The first palette index affected by the
			effect */
		int            amount;   /* The number of (consecutive) palette indices
			affected by the effect */
		fixed          speed;    /* When class is:
			WhiteInPaletteEffect - Number of seconds the effect lasts
			WhiteInPaletteEffect - Number of seconds the effect lasts
			RotatePaletteEffect - Rotations per second
			SkyPaletteEffect - Relative Y speed - as in Jazz 2
			P1DPaletteEffect - Relative X & Y speed - as in Jazz 2
			P2DPaletteEffect - Relative X & Y speed - as in Jazz 2
			WaterPaletteEffect - Number of pixels between water surface and
				total darkness
			*/

	public:
		PaletteEffect          (unsigned char newFirst, int newAmount,
			fixed newSpeed, PaletteEffect * nextPE);
		virtual ~PaletteEffect ();

		virtual void apply     (SDL_Color *shownPalette, bool direct);

};


class WhiteInPaletteEffect : public PaletteEffect {

	private:
		fixed whiteness;

	public:
		WhiteInPaletteEffect (unsigned char newFirst, int newAmount,
			fixed newSpeed, PaletteEffect * nextPE);

		void apply           (SDL_Color *shownPalette, bool direct);

};


class FadeInPaletteEffect : public PaletteEffect {

	private:
		fixed blackness;

	public:
		FadeInPaletteEffect (unsigned char newFirst, int newAmount,
			fixed newSpeed, PaletteEffect * nextPE);

		void apply          (SDL_Color *shownPalette, bool direct);

};


class WhiteOutPaletteEffect : public PaletteEffect {

	private:
		fixed whiteness;

	public:
		WhiteOutPaletteEffect (unsigned char newFirst, int newAmount,
			fixed newSpeed, PaletteEffect * nextPE);

		void apply            (SDL_Color *shownPalette, bool direct);

};


class FadeOutPaletteEffect : public PaletteEffect {

	private:
		fixed blackness;

	public:
		FadeOutPaletteEffect (unsigned char newFirst, int newAmount,
			fixed newSpeed, PaletteEffect * nextPE);

		void apply           (SDL_Color *shownPalette, bool direct);

};


class RotatePaletteEffect : public PaletteEffect {

	private:
		fixed position;

	public:
		RotatePaletteEffect (unsigned char newFirst, int newAmount,
			fixed newSpeed, PaletteEffect * nextPE);

		void apply          (SDL_Color *shownPalette, bool direct);

};


class SkyPaletteEffect : public PaletteEffect {

	private:
		SDL_Color *skyPalette;

	public:
		SkyPaletteEffect (unsigned char newFirst, int newAmount,
			fixed newSpeed, SDL_Color *newSkyPalette, PaletteEffect * nextPE);

		void apply       (SDL_Color *shownPalette, bool direct);

};


class P2DPaletteEffect : public PaletteEffect {

	public:
		P2DPaletteEffect (unsigned char newFirst, int newAmount,
			fixed newSpeed, PaletteEffect * nextPE);

		void apply       (SDL_Color *shownPalette, bool direct);

};


class P1DPaletteEffect : public PaletteEffect {

	public:
		P1DPaletteEffect (unsigned char newFirst, int newAmount,
			fixed newSpeed, PaletteEffect * nextPE);

		void apply       (SDL_Color *shownPalette, bool direct);

};


class WaterPaletteEffect : public PaletteEffect {

	public:
		WaterPaletteEffect (unsigned char newFirst, int newAmount,
			fixed newSpeed, PaletteEffect * nextPE);

		void apply         (SDL_Color *shownPalette, bool direct);

};


