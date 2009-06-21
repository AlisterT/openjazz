
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


#ifndef _PALETTE_H
#define _PALETTE_H


#include "OpenJazz.h"


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
		PaletteEffect *next; // Next effect to use

	public:
		PaletteEffect          (PaletteEffect * nextPE);
		virtual ~PaletteEffect ();

		virtual void apply     (SDL_Color *shownPalette, bool direct);

};


class WhiteInPaletteEffect : public PaletteEffect {

	private:
		int   duration;  // Number of milliseconds the effect lasts
		fixed whiteness;

	public:
		WhiteInPaletteEffect (int newDuration, PaletteEffect * nextPE);

		void apply           (SDL_Color *shownPalette, bool direct);

};


class FadeInPaletteEffect : public PaletteEffect {

	private:
		int   duration;  // Number of milliseconds the effect lasts
		fixed blackness;

	public:
		FadeInPaletteEffect (int newDuration, PaletteEffect * nextPE);

		void apply          (SDL_Color *shownPalette, bool direct);

};


class WhiteOutPaletteEffect : public PaletteEffect {

	private:
		int   duration;  // Number of milliseconds the effect lasts
		fixed whiteness;

	public:
		WhiteOutPaletteEffect (int newDuration, PaletteEffect * nextPE);

		void apply            (SDL_Color *shownPalette, bool direct);

};


class FadeOutPaletteEffect : public PaletteEffect {

	private:
		int   duration;  // Number of milliseconds the effect lasts
		fixed blackness;

	public:
		FadeOutPaletteEffect (int newDuration, PaletteEffect * nextPE);

		void apply           (SDL_Color *shownPalette, bool direct);

};


class FlashPaletteEffect : public PaletteEffect {

	private:
		int           duration;  // Number of milliseconds the effect lasts
		fixed         progress;
		unsigned char red, green, blue; // Flash colour

	public:
		FlashPaletteEffect (unsigned char newRed, unsigned char newGreen,
			unsigned char newBlue, int newDuration, PaletteEffect * nextPE);

		void apply         (SDL_Color *shownPalette, bool direct);

};


class RotatePaletteEffect : public PaletteEffect {

	private:
		unsigned char first;    /* The first palette index affected by the
			effect */
		int           amount;   /* The number of (consecutive) palette indices
			affected by the effect */
		fixed         speed;    // Rotations per second
		fixed         position;

	public:
		RotatePaletteEffect (unsigned char newFirst, int newAmount,
			fixed newSpeed, PaletteEffect * nextPE);

		void apply          (SDL_Color *shownPalette, bool direct);

};


class SkyPaletteEffect : public PaletteEffect {

	private:
		SDL_Color     *skyPalette;
		unsigned char  first;      /* The first palette index affected by the
			effect */
		int            amount;     /* The number of (consecutive) palette
			indices affected by the effect */
		fixed          speed;      // Relative Y speed - as in Jazz 2

	public:
		SkyPaletteEffect (unsigned char newFirst, int newAmount,
			fixed newSpeed, SDL_Color *newSkyPalette, PaletteEffect * nextPE);

		void apply       (SDL_Color *shownPalette, bool direct);

};


class P2DPaletteEffect : public PaletteEffect {

	private:
		unsigned char first;  /* The first palette index affected by the
			effect */
		int           amount; /* The number of (consecutive) palette indices
			affected by the effect */
		fixed         speed;  // Relative X & Y speed - as in Jazz 2

	public:
		P2DPaletteEffect (unsigned char newFirst, int newAmount,
			fixed newSpeed, PaletteEffect * nextPE);

		void apply       (SDL_Color *shownPalette, bool direct);

};


class P1DPaletteEffect : public PaletteEffect {

	private:
		unsigned char first;    /* The first palette index affected by the
			effect */
		int           amount;   /* The number of (consecutive) palette indices
			affected by the effect */
		fixed         speed;    // Relative X & Y speed - as in Jazz 2

	public:
		P1DPaletteEffect (unsigned char newFirst, int newAmount,
			fixed newSpeed, PaletteEffect * nextPE);

		void apply       (SDL_Color *shownPalette, bool direct);

};


class WaterPaletteEffect : public PaletteEffect {

	private:
		fixed depth; /* Number of pixels between water surface and total
			darkness */

	public:
		WaterPaletteEffect (fixed newDepth, PaletteEffect * nextPE);

		void apply         (SDL_Color *shownPalette, bool direct);

};


// Variables

Extern SDL_Color     *currentPalette;
Extern SDL_Color      logicalPalette[256];
Extern PaletteEffect *firstPE;


// Functions

Extern void usePalette     (SDL_Color *palette);
Extern void restorePalette (SDL_Surface *surface);

#endif

