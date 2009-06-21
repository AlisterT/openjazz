
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
 * For music, USE_MODPLUG must be defined.
 *
 */


#include "file.h"
#include "sound.h"
#include <SDL/SDL_audio.h>

#ifdef USE_MODPLUG
#include <modplug.h>


ModPlugFile   *musicFile;
#endif

SDL_AudioSpec  audioSpec;


void audioCallback (void * userdata, unsigned char * stream, int len) {

	int count;

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

	for (count = 0; count < nSounds; count++) {

		if (sounds[count].position >= 0) {

			// Add the next portion of the sound clip to the audio stream

			if (len < sounds[count].length - sounds[count].position) {

				// Play as much of the clip as possible

				SDL_MixAudio(stream,
					sounds[count].data + sounds[count].position, len,
					SDL_MIX_MAXVOLUME >> 1);

				sounds[count].position += len;

			} else {

				// Play the remainder of the clip

				SDL_MixAudio(stream,
					sounds[count].data + sounds[count].position,
					sounds[count].length - sounds[count].position,
					SDL_MIX_MAXVOLUME >> 1);

				sounds[count].position = -1;

			}

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


void playMusic (char * fileName) {

#ifdef USE_MODPLUG

	File *file;
	ModPlug_Settings settings;
	unsigned char *psmData;
	int size;

	// Stop any existing music playing
	stopMusic();


	// Load the music file

	try {

		file = new File(fileName, false);

	} catch (int e) {

		return;

	}

	// Find the size of the file
	size = file->getSize();

	// Read the entire file into memory
	file->seek(0, true);
	psmData = file->loadBlock(size);

	delete file;


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

		fprintf(stderr, "Could not play music file %s\n", fileName);

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


int loadSounds (char *fileName) {

	File *file;
	unsigned char *clip;
	int count, sampleCount, rsFactor, offset, headerOffset;

	try {

		file = new File(fileName, false);

	} catch (int e) {

		return e;

	}

	// Calculate the resampling factor
	if ((audioSpec.format == AUDIO_U8) || (audioSpec.format == AUDIO_S8))
		rsFactor = audioSpec.freq / 5512;
	else rsFactor = (audioSpec.freq / 5512) * 2;

	// Locate the header data
	file->seek(file->getSize() - 4, true);
	headerOffset = file->loadInt();

	// Calculate number of sounds
	nSounds = (file->getSize() - headerOffset) / 18;

	// Load sound clips

	sounds = new Sound[nSounds];

	for(count = 0; count < nSounds; count++) {

		file->seek(headerOffset + (count * 18), true);

		// Read the name of the clip
		sounds[count].name = (char *)(file->loadBlock(12));

		// Read the offset of the clip
		offset = file->loadInt();

		// Read the length of the clip
		sounds[count].length = file->loadShort();

		// Read the clip
		file->seek(offset, true);
		clip = file->loadBlock(sounds[count].length);

		sounds[count].length *= rsFactor;

		// Resample the clip
		sounds[count].data = new unsigned char[sounds[count].length];

		for (sampleCount = 0; sampleCount < sounds[count].length; sampleCount++)
			sounds[count].data[sampleCount] = clip[sampleCount / rsFactor];

		delete[] clip;

		sounds[count].position = -1;

	}

	delete file;

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

	if ((sounds != NULL) && (newSound >= 0)) sounds[newSound].position = 0;

	return;

}


