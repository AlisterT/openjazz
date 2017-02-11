// VisualBoyAdvance - Nintendo Gameboy/GameboyAdvance (TM) emulator.
// Copyright (C) 1999-2003 Forgotten
// Copyright (C) 2004 Forgotten and the VBA development team

// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2, or(at your option)
// any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software Foundation,
// Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.

/* 
 * Code adapted To openjazz by Pickle (from Openbor adapted by SX
 * simple2x.c - Trying to scale 2x.
 *
 *
 */

#include "scalebit.h"

void Simple2x(unsigned char *srcPtr, unsigned int srcPitch, unsigned char *deltaPtr, unsigned char *dstPtr, unsigned int dstPitch, int width, int height)
{
	(void)deltaPtr;

    unsigned char *nextLine, *finish;
  
	nextLine = dstPtr + dstPitch;
  
	do 
	{
		unsigned char *bP = (unsigned char *) srcPtr;
		unsigned char *dP = (unsigned char *) dstPtr;
		unsigned char *nL = (unsigned char *) nextLine;
		unsigned char currentPixel;
    
		finish = (unsigned char *) bP + ((width+2) << 1);
		currentPixel = *bP++;
    
		do 
		{
#ifdef BIG_ENDIAN
			unsigned char color = currentPixel >> 16;
#else
			unsigned char color = currentPixel & 0xffff;
#endif

			color = color | (color << 16);

			*(dP) = color;
			*(nL) = color;

//#ifdef BIG_ENDIAN
//			color = currentPixel & 0xffff;
//#else
//			color = currentPixel >> 16;
//#endif
			color = color| (color << 16);      
			*(dP + 1) = color;
			*(nL + 1) = color;
      
			currentPixel = *bP++;
      
			dP += 2;
			nL += 2;
		}
		while ((unsigned char *) bP < finish);
    
		srcPtr += srcPitch;
		dstPtr += dstPitch << 1;
		nextLine += dstPitch << 1;
	}
	while (--height);
}

void Simple2x32(unsigned char *srcPtr, unsigned int srcPitch, unsigned char *deltaPtr, unsigned char *dstPtr, unsigned int dstPitch, int width, int height)
{
	(void)deltaPtr;

    unsigned char *nextLine, *finish;
  
	nextLine = dstPtr + dstPitch;
  
	do 
	{
		unsigned int *bP = (unsigned int *) srcPtr;
	    unsigned int *dP = (unsigned int *) dstPtr;
		unsigned int *nL = (unsigned int *) nextLine;
		unsigned int currentPixel;
    
		finish = (unsigned char *) bP + ((width+1) << 2);
		currentPixel = *bP++;
    
		do 
		{
			unsigned int color = currentPixel;

		    *(dP) = color;
			*(dP+1) = color;
			*(nL) = color;
			*(nL + 1) = color;
      
			currentPixel = *bP++;
	      
			dP += 2;
			nL += 2;
		}
		while ((unsigned char *) bP < finish);
    
		srcPtr += srcPitch;
		dstPtr += dstPitch << 1;
		nextLine += dstPitch << 1;
	}
	while (--height);
}
