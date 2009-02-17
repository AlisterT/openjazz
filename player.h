
/*
 *
 * player.h
 *
 * Created on the 31st of January 2006 from parts of OpenJazz.h
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


/* "Tile" is a flexible term. Here it is used to refer specifically to the
   individual elements of the tile set.
   "Tiles" in the context of level units are referred to as grid elements. */


// Constants

// Player animations
#define PA_LWALK    0
#define PA_RWALK    1
#define PA_LJUMP    2
#define PA_RJUMP    3
#define PA_LSPIN    4
#define PA_RSPIN    5
#define PA_LSHOOT   6
#define PA_RSHOOT   7
#define PA_LCROUCH  8
#define PA_RCROUCH  9
#define PA_LFALL    10
#define PA_RFALL    11
#define PA_LHURT    12
#define PA_RHURT    13
#define PA_LLEAN    14
#define PA_RLEAN    15
#define PA_LBOARD   16
#define PA_RBOARD   17
#define PA_LSTAND   18
#define PA_RSTAND   19
#define PA_LEAT     20
#define PA_REAT     21
#define PA_LEDGE    22
#define PA_REDGE    23
#define PA_LOOKUP   24
#define PA_LOOKDOWN 25
#define PA_LSWIM    26
#define PA_RSWIM    27
#define PA_LRUN     28
#define PA_RRUN     29
#define PA_LDIE     30
#define PA_RDIE     31
#define PA_LSTOP    32
#define PA_RSTOP    33
#define PA_LHALT    34 /* Yeah, I was wondering the same thing... */
#define PA_RHALT    35
#define PA_RSPRING  36
#define PA_LSPRING  37 /* Surely these are the wrong way round? */

// Player facing
#define PF_LEFT  0
#define PF_RIGHT 1

// Player reactions
#define PR_NONE       0
#define PR_HURT       1
#define PR_KILLED     2
#define PR_INVINCIBLE 3
#define PR_WON        4

// Player reaction times
#define PRT_NONE       0
#define PRT_HURT       1000
#define PRT_KILLED     2000
#define PRT_INVINCIBLE 10000
#define PRT_WON        6000

// Player offsets
#define PXO_L   (F16 - F10)
#define PXO_ML  F12
#define PXO_MID F16
#define PXO_MR  F20
#define PXO_R   (F16 + F10)
#define PYO_TOP (-F20)
#define PYO_MID (-F10)

// Player speeds
#define PXS_WALK (300 * F1)
#define PXS_RUN  (325 * F1)
#define PYS_FALL (350 * F1)
#define PYS_SINK (150 * F1)
#define PYS_JUMP (-350 * F1)

// Player accelerations
#define PXA_REVERSE 600
#define PXA_STOP    1000
#define PXA_WALK    300
#define PXA_RUN     200
#define PYA_GRAVITY 2750
#define PYA_SINK    1000


// General
#define PANIMS     38 /* Number of player animations. Is probably higher. */
#define PCONTROLS   7 /* Number of player controls. */


// Class

class Player {

	private:
		char         *name;
		signed char  *event;    /* A member of the event set (spring, float up,
			belt, platform) */
		char          anims[PANIMS];
		bool          pcontrols[PCONTROLS];
		int           ammo[4];
		int           ammoType; /* -1 = blaster, 0 = toaster, 1 = missiles,
			2 = bouncer 3 = TNT */
		int           score;
		int           energy;
		fixed         energyBar;
		int           lives;
		int           shield; /* 0 = none, 1 = 1 yellow, 2 = 2 yellow,
			3 = 1 orange, 4 = 2 orange, 5 = 3 orange, 6 = 4 orange */
		bool          floating; // false = normal, true = boarding/bird/etc.
		bool          facing;
		int           lookTime; /* Negative if looking up, positive if looking
			down, 0 if neither */
		int           reaction;
		int           reactionTime;
		int           fireSpeed;
		int           fireTime;
		fixed         jumpHeight;
		int           jumpY;
		fixed         x, y, dx, dy;
		unsigned char checkX, checkY;

	public:
		fixed viewX, viewY;
		int   viewW, viewH;
		int   enemies, items;

		Player               ();
		~Player              ();

		void   setAnim       (int index, char anim);
		void   setName       (char * playerName);
		char * getName       ();
		void   reset         ();
		void   setCheckpoint (unsigned char newX, unsigned char newY);
		void   setControl    (int control, bool state);
		void   addScore      (int addedScore);
		int    getScore      ();
		void   addCarrot     ();
		void   hit           (int ticks);
		int    getEnergy     ();
		int    getEnergyBar  ();
		void   addLife       ();
		void   kill          (int ticks);
		int    getLives      ();
		void   addAmmo       (int type, int amount);
		void   changeAmmo    ();
		void   fire          (int ticks);
		int    getAmmo       (bool amount);
		void   addRapidFire  ();
		void   addFeet       ();
		void   addStar       (int ticks);
		void   addShield     (bool four);
		void   win           (int ticks);
		fixed  getX          ();
		fixed  getY          ();
		bool   isIn          (fixed left, fixed top, fixed width, fixed height);
		void   setPosition   (fixed newX, fixed newY);
		void   setSpeed      (fixed newDx, fixed newDy);
		void   setFloating   (bool newFloating);
		bool   getFacing     ();
		void   floatUp       (signed char *newEvent);
		void   belt          (int speed);
		void   setEvent      (signed char *newEvent);
		void   clearEvent    (signed char *newEvent, unsigned char property);
		void   control       (int ticks);
		void   move          (int ticks);
		void   view          (int ticks);
		void   draw          (int ticks);
		int    reacted       (int ticks);

};


