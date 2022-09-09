
/**
 *
 * @file sound.cpp
 *
 * Part of the OpenJazz project
 *
 * @par History:
 * - 23rd August 2005: Created sound.c
 * - 3rd February 2009: Renamed sound.c to sound.cpp
 *
 * @par Licence:
 * Copyright (c) 2005-2017 Alister Thomson
 *
 * OpenJazz is distributed under the terms of
 * the GNU General Public License, version 2.0
 *
 * @par Description:
 * Deals with the loading, playing and freeing of music and sound effects.
 *
 */


#include "file.h"
#include "sound.h"
#include "util.h"
#include "io/log.h"

#include <SDL_audio.h>
#include <psmplug.h>
#include <cstring>

#if defined(__SYMBIAN32__) || defined(_3DS) || defined(PSP) || defined(__vita__)
	#define SOUND_FREQ 22050
#else
	#define SOUND_FREQ 44100
#endif

#if defined(GP2X) || defined(PSP) || defined(_3DS) || defined(__vita__)
	#define SOUND_SAMPLES 512
#else
	#define SOUND_SAMPLES 2048
#endif

#ifdef __SYMBIAN32__
	#define MUSIC_RESAMPLEMODE MODPLUG_RESAMPLE_LINEAR
	#define MUSIC_FLAGS MODPLUG_ENABLE_MEGABASS
#elif defined(CAANOO) || defined(WIZ) || defined(GP2X) || defined(DINGOO) || defined(PSP)
	#define MUSIC_RESAMPLEMODE MODPLUG_RESAMPLE_LINEAR
	#define MUSIC_FLAGS 0
#else
	#define MUSIC_RESAMPLEMODE MODPLUG_RESAMPLE_FIR
	#define MUSIC_FLAGS MODPLUG_ENABLE_NOISE_REDUCTION | MODPLUG_ENABLE_REVERB | MODPLUG_ENABLE_MEGABASS | MODPLUG_ENABLE_SURROUND
#endif

ModPlugFile *musicFile;
SDL_AudioSpec  audioSpec;
bool musicPaused = false;
int musicVolume = MAX_VOLUME >> 1; // 50%
int soundVolume = MAX_VOLUME >> 2; // 25%
char *currentMusic = NULL;
int musicTempo = MUSIC_NORMAL;


/**
 * Callback used to provide data to the audio subsystem.
 *
 * @param userdata N/A
 * @param stream Output stream
 * @param len Length of data to be placed in the output stream
 */
void audioCallback (void * userdata, unsigned char * stream, int len) {

	(void)userdata;

	int count;

	if (!musicPaused) {

		// Read the next portion of music into the audio stream

		if (musicFile) ModPlug_Read(musicFile, stream, len);

	}

	if (!sounds) return;

	for (count = 0; count < 32; count++) {

		if (sounds[count].data && (sounds[count].position >= 0)) {

			// Add the next portion of the sound clip to the audio stream

			if (len < sounds[count].length - sounds[count].position) {

				// Play as much of the clip as possible

				SDL_MixAudio(stream,
					sounds[count].data + sounds[count].position, len,
					soundVolume * SDL_MIX_MAXVOLUME / MAX_VOLUME);

				sounds[count].position += len;

			} else {

				// Play the remainder of the clip

				SDL_MixAudio(stream,
					sounds[count].data + sounds[count].position,
					sounds[count].length - sounds[count].position,
					soundVolume * SDL_MIX_MAXVOLUME / MAX_VOLUME);

				sounds[count].position = -1;

			}

		}

	}

	return;

}


/**
 * Initialise audio.
 */
void openAudio () {

	SDL_AudioSpec asDesired;
	musicFile = NULL;

	// Set up SDL audio

	asDesired.freq = SOUND_FREQ;
	asDesired.format = AUDIO_S16;
	asDesired.channels = 2;
	asDesired.samples = SOUND_SAMPLES;
	asDesired.callback = audioCallback;
	asDesired.userdata = NULL;

	if (SDL_OpenAudio(&asDesired, &audioSpec) < 0)
		LOG_ERROR("Unable to open audio: %s", SDL_GetError());


	// Load sounds

	if (loadSounds("SOUNDS.000") != E_NONE) sounds = NULL;

	// Start audio for sfx to work

	SDL_PauseAudio(0);

	return;

}


/**
 * Stop audio.
 */
void closeAudio () {

	int count;

	stopMusic();

	SDL_CloseAudio();

	if (rawSounds) {

		for (count = 0; count < nRawSounds; count++) {

			delete[] rawSounds[count].data;
			delete[] rawSounds[count].name;

		}

		delete[] rawSounds;

	}

	if (sounds) {

		freeSounds();
		delete[] sounds;

	}

	return;

}


/**
 * Play music from the specified file.
 *
 * @param fileName Name of a file containing music data.
 * @param restart Restart music when same file is played.
 */
void playMusic (const char * fileName, bool restart) {

	File *file;
	unsigned char *psmData;
	int size;
	bool loadOk = false;
	ModPlug_Settings settings;

	/* Only stop any existing music playing, if a different file
	   should be played or a restart has been requested. */
	if ((currentMusic && (strcmp(fileName, currentMusic) == 0)) && !restart)
		return;

	stopMusic();

	// Load the music file

	try {

		file = new File(fileName, false);

	} catch (int e) {

		return;

	}

	// Save current music filename

	if (currentMusic) delete[] currentMusic;
	currentMusic = createString(fileName);

	// Find the size of the file
	size = file->getSize();

	// Read the entire file into memory
	file->seek(0, true);
	psmData = file->loadBlock(size);

	delete file;

	// Set up libpsmplug

	settings.mFlags = MUSIC_FLAGS;
	settings.mChannels = audioSpec.channels;

	if ((audioSpec.format == AUDIO_U8) || (audioSpec.format == AUDIO_S8))
		settings.mBits = 8;
	else settings.mBits = 16;

	settings.mFrequency = audioSpec.freq;
	settings.mResamplingMode = MUSIC_RESAMPLEMODE;
	settings.mReverbDepth = 25;
	settings.mReverbDelay = 40;
	settings.mBassAmount = 50;
	settings.mBassRange = 10;
	settings.mSurroundDepth = 50;
	settings.mSurroundDelay = 40;

	// unlimited looping
	settings.mLoopCount = -1;

	ModPlug_SetSettings(&settings);

	// Load the file into libmodplug
	musicFile = ModPlug_Load(psmData, size);
	loadOk = (musicFile != NULL);

	delete[] psmData;

	if (!loadOk) {

		LOG_ERROR("Could not play music file: %s", fileName);

		return;

	}

	// Re-apply volume setting
	setMusicVolume(musicVolume);

	// Start the audio playing
	SDL_PauseAudio(0);
	musicPaused = false;

	return;

}


/**
 * Pauses and Unpauses the current music.
 *
 * @param pause set to true to pause
 */
void pauseMusic (bool pause) {
	musicPaused = pause;
}


/**
 * Stop the current music.
 */
void stopMusic () {

	// Stop the music playing

	SDL_PauseAudio(~0);

	// Cleanup

	if (currentMusic) {

		delete[] currentMusic;
		currentMusic = NULL;

	}

	if (musicFile) {

		ModPlug_Unload(musicFile);
		musicFile = NULL;

	}

	SDL_PauseAudio(0);

	return;

}


/**
 * Gets the current music volume
 *
 * @return music volume (0-100)
 */
int getMusicVolume () {

	return musicVolume;

}


/**
 * Sets the music volume
 *
 * @param volume new volume (0-100)
 */
void setMusicVolume (int volume) {

	musicVolume = volume;
	if (volume < 1) musicVolume = 0;
	if (volume > MAX_VOLUME) musicVolume = MAX_VOLUME;

	// do not access music player settings when not playing

	if (musicFile) ModPlug_SetMasterVolume(musicFile, musicVolume * 2.56);

}


/**
 * Gets the current music tempo
 *
 * @return music tempo (MUSIC_NORMAL, MUSIC_FAST)
 */
int getMusicTempo () {

	return musicTempo;

}


/**
 * Sets the music tempo
 *
 * @param tempo new tempo (MUSIC_NORMAL, MUSIC_FAST)
 */
void setMusicTempo (int tempo) {

	if ((tempo != MUSIC_FAST) && (tempo != MUSIC_NORMAL))
		musicTempo = MUSIC_NORMAL;
	else
		musicTempo = tempo;

	// do not access music player settings when not playing

	if (musicFile) {

		if (musicTempo == MUSIC_FAST)
			ModPlug_SetMusicTempoFactor(musicFile, 80);
		else
			ModPlug_SetMusicTempoFactor(musicFile, 128);

	}

}


/**
 * Load raw sound clips from the specified file.
 *
 * @param fileName Name of a file containing sound clips
 */
int loadSounds (const char *fileName) {

	File *file;
	int count, offset, headerOffset;

	try {

		file = new File(fileName, false);

	} catch (int e) {

		return e;

	}

	sounds = new Sound[32];

	for (count = 0; count < 32; count++) {

		sounds[count].data = NULL;

	}

	// Locate the header data
	file->seek(file->getSize() - 4, true);
	headerOffset = file->loadInt();

	// Calculate number of sounds
	nRawSounds = (file->getSize() - headerOffset) / 18;

	// Load sound clips

	rawSounds = new RawSound[nRawSounds];

	for (count = 0; count < nRawSounds; count++) {

		file->seek(headerOffset + (count * 18), true);

		// Read the name of the clip
		rawSounds[count].name = (char *)(file->loadBlock(12));

		// Read the offset of the clip
		offset = file->loadInt();

		// Read the length of the clip
		rawSounds[count].length = file->loadShort();

		// Read the clip
		file->seek(offset, true);
		rawSounds[count].data = file->loadBlock(rawSounds[count].length);

	}

	delete file;

	resampleSounds();

	return E_NONE;

}


/**
 * Resample sound clip data.
 */
void resampleSound (int index, const char* name, int rate) {

	int count, rsFactor, sample;

    if (sounds[index].data) {

		delete[] sounds[index].data;
		sounds[index].data = NULL;

	}

	// Search for matching sound

	for (count = 0; count < nRawSounds; count++) {

		if (!strcmp(name, rawSounds[count].name)) {

			// Calculate the resampling factor
			if ((audioSpec.format == AUDIO_U8) || (audioSpec.format == AUDIO_S8))
				rsFactor = (F2 * audioSpec.freq) / rate;
			else rsFactor = (F4 * audioSpec.freq) / rate;

			sounds[index].length = MUL(rawSounds[count].length, rsFactor);

			// Allocate the buffer for the resampled clip
			sounds[index].data = new unsigned char[sounds[index].length];

			// Resample the clip
			for (sample = 0; sample < sounds[index].length; sample++)
				sounds[index].data[sample] = rawSounds[count].data[DIV(sample, rsFactor)];

			sounds[index].position = -1;

			return;

		}

	}

}


/**
 * Resample all sound clips to matching indices.
 */
void resampleSounds () {

	int count;

	for (count = 0; (count < 32) && (count < nRawSounds); count++) {

		resampleSound(count, rawSounds[count].name, 11025);

	}

	return;

}


/**
 * Delete resampled sound clip data.
 */
void freeSounds () {

	int count;

	if (sounds) {

		for (count = 0; count < 32; count++) {

			if (sounds[count].data) delete[] sounds[count].data;

		}

	}

	return;

}


/**
 * Set the sound clip to be played.
 *
 * @param index Number of the sound to play plus one (0 to play no sound)
 */
void playSound (char index) {

	if (sounds && (index > 0) && (index <= 32)) sounds[index - 1].position = 0;

	return;

}


/**
 * Check if a sound clip is playing.
 *
 * @param index Number of the sound to check plus one
 *
 * @return Whether the sound is playing
 */
bool isSoundPlaying (char index) {

	if (!sounds || (index <= 0) || (index > 32))
		return false;

	return (sounds[index - 1].position > 0);

}


/**
 * Gets the current sound effect volume
 *
 * @return sound volume (0-100)
 */
int getSoundVolume () {

	return soundVolume;

}


/**
 * Sets the sound effect volume
 *
 * @param volume new volume (0-100)
 */
void setSoundVolume (int volume) {

	soundVolume = volume;
	if (volume < 1) soundVolume = 0;
	if (volume > MAX_VOLUME) soundVolume = MAX_VOLUME;

}
