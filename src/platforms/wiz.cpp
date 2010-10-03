
/**
 *
 * @file wiz.cpp
 *
 * Part of the OpenJazz project
 *
 * @section Licence
 * OpenJazz is distributed under the terms of
 * the GNU General Public License, version 2.0
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */


#include "wiz.h"

#if defined(WIZ) || defined(GP2X)
#include <cstdio>
#include <sys/ioctl.h>
#include <sys/soundcard.h>
#include <fcntl.h>
#include <unistd.h>
#include "io/sound.h"

void WIZ_AdjustVolume( int direction )
{
	if( direction != VOLUME_NOCHG )
	{
		if( volume <= 10 )
		{
			if( direction == VOLUME_UP )   volume += VOLUME_CHANGE_RATE/2;
			if( direction == VOLUME_DOWN ) volume -= VOLUME_CHANGE_RATE/2;
		}
		else
		{
			if( direction == VOLUME_UP )   volume += VOLUME_CHANGE_RATE;
			if( direction == VOLUME_DOWN ) volume -= VOLUME_CHANGE_RATE;
		}

		if( volume < VOLUME_MIN ) volume = VOLUME_MIN;
		if( volume > VOLUME_MAX ) volume = VOLUME_MAX;

		printf( "Volume Change: %i\n", volume );
	}
	unsigned long soundDev = open("/dev/mixer", O_RDWR);
	if(soundDev)
	{
		int vol = ((volume << 8) | volume);
		ioctl(soundDev, SOUND_MIXER_WRITE_PCM, &vol);
		close(soundDev);
	}
}
#endif
