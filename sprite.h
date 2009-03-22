
/*
 *
 * level.h
 *
 * Created on the 19th of March 2009 from parts of level.h
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


// Classes

class Sprite {

	private:
		SDL_Surface *pixels;

	public:
		unsigned char xOffset;
		unsigned char yOffset;

		Sprite              ();
		~Sprite             ();

		void clearPixels    ();
		void setPixels      (unsigned char *data, int width, int height);
		int  getWidth       ();
		int  getHeight      ();
		void draw           (int x, int y);
		void setPalette     (SDL_Color *palette, int start, int amount);
		void flashPalette   (int index);
		void restorePalette ();

};

class Anim {

	private:
		Sprite *sprites[19]; // Pointers to members of the sprite set
		signed char   offsets[19];  // Vertical offsets (for pickup wobble effect)
		unsigned char frames; // Number of frames
		unsigned char frame;

	public:
		Anim                ();
		~Anim               ();

		void setFrames      (int newFrames);
		void setFrame       (int newFrame, bool looping);
		void setData        (Sprite *frameSprite, signed char frameY);
		int  getOffset      ();
		int  getWidth       ();
		int  getHeight      ();
		void draw           (int x, int y);
		void setPalette     (SDL_Color *palette, int start, int amount);
		void flashPalette   (int index);
		void restorePalette ();

};


