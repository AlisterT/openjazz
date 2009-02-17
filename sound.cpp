
/*
 *
 * sound.cpp
 *
 * Created as sound.c on the 23rd of August 2005
 * Renamed sound.cpp on the 3rd of February 2009
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

/*
 * Deals with the loading, playing and freeing of music and sound effects.
 *
 */


#include "OpenJazz.h"
#include <SDL/SDL_audio.h>

#ifdef USE_MODPLUG

#include <modplug.h>


ModPlugFile   *musicFile;
SDL_AudioSpec  audioSpec;


void audioCallback (void * userdata, unsigned char * stream, int len) {

	unsigned char *musicData;

	if (musicFile) {

		// Allocate memory in which to read the music
		musicData = new unsigned char[len];

		// Read the next portion of music
		ModPlug_Read(musicFile, musicData, len);

		// Add the next portion of music to the audio stream
		SDL_MixAudio(stream, musicData, len, SDL_MIX_MAXVOLUME);

		// Free the music data
		delete[] musicData;

	}

	return;

}

#endif


void openAudio () {

	SDL_AudioSpec asDesired;

	musicFile = NULL;


	// Set up SDL audio

	asDesired.freq = 44100;
	asDesired.format = AUDIO_S16;
	asDesired.channels = 2;
	asDesired.samples = 2048;
	asDesired.callback = audioCallback;
	asDesired.userdata = NULL;

	if (SDL_OpenAudio(&asDesired, &audioSpec) < 0) {

		fprintf(stderr, "Unable to open audio: %s\n", SDL_GetError());

		return;

	}

	return;

}

void closeAudio () {

	stopMusic();

	SDL_CloseAudio();

	return;

}


void playMusic (char * fn) {

#ifdef USE_MODPLUG

	File *f;
	ModPlug_Settings settings;
	unsigned char *psmData;
	int size;

	// Stop any existing music playing
	stopMusic();


	// Load the music file

	try {

		f = new File(fn, false);

	} catch (int e) {

		return;

	}

	// Find the size of the file
	size = f->getSize();

	// Read the entire file into memory
	f->seek(0, true);
	psmData = f->loadBlock(size);

	delete f;


	// Set up libmodplug

	settings.mFlags = MODPLUG_ENABLE_NOISE_REDUCTION | MODPLUG_ENABLE_REVERB |
		MODPLUG_ENABLE_MEGABASS | MODPLUG_ENABLE_SURROUND;
	settings.mChannels = audioSpec.channels;

	if ((audioSpec.format == AUDIO_U8) || (audioSpec.format == AUDIO_S8))
		settings.mBits = 8;
	else settings.mBits = 16;

	settings.mFrequency = audioSpec.freq;
	settings.mResamplingMode = MODPLUG_RESAMPLE_FIR;
	settings.mReverbDepth = 25;
	settings.mReverbDelay = 40;
	settings.mBassAmount = 50;
	settings.mBassRange = 10;
	settings.mSurroundDepth = 50;
	settings.mSurroundDelay = 40;
	settings.mLoopCount = -1;

	ModPlug_SetSettings(&settings);

	// Load the file into libmodplug
	musicFile = ModPlug_Load(psmData, size);

	delete[] psmData;

	if (!musicFile) {

		fprintf(stderr, "Could not play music file %s\n", fn);

		return;

	}

	// Start the music playing
	SDL_PauseAudio(0);

#endif

	return;

}


void stopMusic () {

#ifdef USE_MODPLUG

	// Stop the music playing
	SDL_PauseAudio(~0);

	if (musicFile) {

		ModPlug_Unload(musicFile);
		musicFile = NULL;

	}

#endif

	return;

}


int loadSounds (char *fn) {

	File *f;

	try {

		f = new File(fn, false);

	} catch (int e) {

		return FAILURE;

	}

	// To do

	delete f;

	return SUCCESS;

}


void freeSounds () {

  // To do

  return;

}


void playSound (int sound) {

  // To do

  return;

}


