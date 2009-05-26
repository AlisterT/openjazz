
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


#include "file.h"
#include <SDL/SDL_audio.h>

#ifdef USE_MODPLUG
#include <modplug.h>


ModPlugFile   *musicFile;
#endif

SDL_AudioSpec  audioSpec;
int            currentSound;
int            soundPos;


void audioCallback (void * userdata, unsigned char * stream, int len) {

#ifdef USE_MODPLUG
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
#endif

	if (currentSound >= 0) {

		// Add the next portion of the sound clip to the audio stream

		if (len < sounds[currentSound].length - soundPos) {

			// Play as much of the clip as possible

			SDL_MixAudio(stream, sounds[currentSound].data + soundPos, len,
				SDL_MIX_MAXVOLUME >> 1);

			soundPos += len;

		} else {

			// Play the remainder of the clip

			SDL_MixAudio(stream, sounds[currentSound].data + soundPos,
				sounds[currentSound].length - soundPos, SDL_MIX_MAXVOLUME >> 1);

			currentSound = -1;

		}

	}

	return;

}


void openAudio () {

	SDL_AudioSpec asDesired;

#ifdef USE_MODPLUG
	musicFile = NULL;
#endif


	// Set up SDL audio

	asDesired.freq = 44100;
	asDesired.format = AUDIO_S16;
	asDesired.channels = 2;
	asDesired.samples = 2048;
	asDesired.callback = audioCallback;
	asDesired.userdata = NULL;

	if (SDL_OpenAudio(&asDesired, &audioSpec) < 0)
		fprintf(stderr, "Unable to open audio: %s\n", SDL_GetError());


	// Load sounds

	if (loadSounds("sounds.000") != E_NONE) sounds = NULL;


	return;

}

void closeAudio () {

	stopMusic();

	SDL_CloseAudio();

	freeSounds();

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
	unsigned char *clip;
	int count, sampleCount, rsFactor, offset, headerOffset;

	currentSound = -1;

	try {

		f = new File(fn, false);

	} catch (int e) {

		return e;

	}

	// Calculate the resampling factor
	if ((audioSpec.format == AUDIO_U8) || (audioSpec.format == AUDIO_S8))
		rsFactor = audioSpec.freq / 5512;
	else rsFactor = (audioSpec.freq / 5512) * 2;

	// Locate the header data
	f->seek(f->getSize() - 4, true);
	headerOffset = f->loadInt();

	// Calculate number of sounds
	nSounds = (f->getSize() - headerOffset) / 18;

	// Load sound clips

	sounds = new Sound[nSounds];

	for(count = 0; count < nSounds; count++) {

		f->seek(headerOffset + (count * 18), true);

		// Read the name of the clip
		sounds[count].name = (char *)(f->loadBlock(12));

		// Read the offset of the clip
		offset = f->loadInt();

		// Read the length of the clip
		sounds[count].length = f->loadShort();

		// Read the clip
		f->seek(offset, true);
		clip = f->loadBlock(sounds[count].length);

		sounds[count].length *= rsFactor;

		// Resample the clip
		sounds[count].data = new unsigned char[sounds[count].length];

		for (sampleCount = 0; sampleCount < sounds[count].length; sampleCount++)
			sounds[count].data[sampleCount] = clip[sampleCount / rsFactor];

		delete[] clip;

	}

	delete f;

	return E_NONE;

}


void freeSounds () {

	int count;

	if (sounds != NULL) {

		for (count = 0; count < nSounds; count++) {

			delete[] sounds[count].data;
			delete[] sounds[count].name;

		}

		delete[] sounds;

	}

	return;

}


void playSound (int newSound) {

	// Set the sound to be played

	if (sounds != NULL) {

		currentSound = newSound;
		soundPos = 0;

	}

	return;

}


