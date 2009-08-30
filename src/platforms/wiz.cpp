#include "wiz.h"

#include <cstdio>
#include <sys/ioctl.h>
#include <sys/soundcard.h>
#include <fcntl.h>
#include <unistd.h>

int volume = 40;
int volume_direction;
 
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
