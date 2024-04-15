
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
#include "platforms/platforms.h"
#include <SDL_audio.h>
#include <psmplug.h>
#include <cassert>

// default configuration

#ifndef SOUND_FREQ
	#define SOUND_FREQ 44100
#endif
#ifndef SOUND_SAMPLES
	#define SOUND_SAMPLES 2048
#endif
#if MUSIC_SETTINGS == 0
	// low
	#define MUSIC_RESAMPLEMODE MODPLUG_RESAMPLE_LINEAR
	#define MUSIC_FLAGS 0
#elif MUSIC_SETTINGS == 1
	// mid
	#define MUSIC_RESAMPLEMODE MODPLUG_RESAMPLE_LINEAR
	#define MUSIC_FLAGS MODPLUG_ENABLE_MEGABASS
#else
	// high
	#define MUSIC_RESAMPLEMODE MODPLUG_RESAMPLE_FIR
	#define MUSIC_FLAGS MODPLUG_ENABLE_NOISE_REDUCTION | MODPLUG_ENABLE_REVERB | MODPLUG_ENABLE_MEGABASS | MODPLUG_ENABLE_SURROUND
#endif

#define clamp_vol(vol, min, max) (((vol) < (min)) ? (min) : (((vol) > (max)) ? (max) : (vol)))

namespace {

	// Datatype

	/// Raw sound effect data
	typedef struct {
		unsigned char *data;
		char          *name;
		int            length;
	} RawSound;

	/// Resampled sound effect data
	typedef struct {
		unsigned char *data;
		int            length;
		int            position;
	} Sound;

	// Variables

	static RawSound *rawSounds;
	static int nRawSounds;
	static Sound sounds[SE::MAX] = {};
	static bool soundsLoaded = false;
	static ModPlugFile *musicFile = nullptr;
	static SDL_AudioSpec audioSpec = {};
	static bool musicPaused = false;
	static int musicVolume = MAX_VOLUME >> 1; // 50%
	static int soundVolume = MAX_VOLUME >> 2; // 25%
	static char *currentMusic = nullptr;
	static MusicTempo musicTempo = MusicTempo::NORMAL;

	#if OJ_SDL2
	static SDL_AudioDeviceID audioDevice = 0;
	#endif

	// Helpers

	static void LockAudio() {
	#if OJ_SDL2
		SDL_LockAudioDevice(audioDevice);
	#else
		SDL_LockAudio();
	#endif
	}
	static void UnlockAudio() {
	#if OJ_SDL2
		SDL_UnlockAudioDevice(audioDevice);
	#else
		SDL_UnlockAudio();
	#endif
	}
	#if !OJ_SDL2
	static int SDL_AUDIO_BITSIZE(int format) {
		if (format == AUDIO_U8 || audioSpec.format == AUDIO_S8)
			return 8;
		else if (format == AUDIO_S16MSB || audioSpec.format == AUDIO_S16LSB ||
			format == AUDIO_U16MSB || audioSpec.format == AUDIO_U16LSB)
			return 16;

		LOG_ERROR("Unsupported Audio format.");
		return 0;
	}
	#endif

	/**
	 * Callback used to provide data to the audio subsystem.
	 *
	 * @param userdata N/A
	 * @param stream Output stream
	 * @param len Length of data to be placed in the output stream
	 */
	static void audioCallback (void * /*userdata*/, unsigned char * stream, int len) {
		// Clear audio buffer
		memset(stream, '\0', len * sizeof(unsigned char));

		if (musicFile && !musicPaused) {
			// Read the next portion of music into the stream
			ModPlug_Read(musicFile, stream, len);
		}

		if (!soundsLoaded) return;

		for (int i = SE::NONE; i < SE::MAX; i++) {
			if (!sounds[i].data || sounds[i].position < 0) continue;

			int rest = sounds[i].length - sounds[i].position;
			int length = 0;
			int position = sounds[i].position;

			if (len < rest) {
				// Play as much of the clip as possible
				length = len;
				sounds[i].position += len;
			} else {
				// Play the remainder of the clip
				length = rest;
				sounds[i].position = -1;
			}

			// Add the next portion of the sound clip to the audio stream
	#if OJ_SDL2
			SDL_MixAudioFormat(stream, sounds[i].data + position, audioSpec.format,
				length, soundVolume * SDL_MIX_MAXVOLUME / MAX_VOLUME);
	#else
			SDL_MixAudio(stream, sounds[i].data + position, length,
				soundVolume * SDL_MIX_MAXVOLUME / MAX_VOLUME);
	#endif
		}
	}

}


/**
 * Initialise audio.
 */
void openAudio () {
	// Set up SDL audio
	SDL_AudioSpec asDesired = {};
	asDesired.freq = SOUND_FREQ;
	asDesired.format = AUDIO_S16;
	asDesired.channels = 2;
	asDesired.samples = SOUND_SAMPLES;
	asDesired.callback = audioCallback;
	asDesired.userdata = nullptr;

	bool audioOk = false;

#if OJ_SDL2
	audioDevice = SDL_OpenAudioDevice(nullptr, 0, &asDesired, &audioSpec,
		SDL_AUDIO_ALLOW_ANY_CHANGE);

	if(!audioDevice || SDL_AUDIO_ISFLOAT(audioSpec.format) ||
		(SDL_AUDIO_BITSIZE(audioSpec.format) != 8 && SDL_AUDIO_BITSIZE(audioSpec.format) != 16)) {
		LOG_DEBUG("SDL audio format unsupported, letting SDL convert it.");

		if(audioDevice) SDL_CloseAudioDevice(audioDevice);

		audioDevice = SDL_OpenAudioDevice(nullptr, 0, &asDesired, &audioSpec, 0);
	}
	audioOk = (audioDevice != 0);
#else
	audioOk = (SDL_OpenAudio(&asDesired, &audioSpec) == 0);
#endif

	if(!audioOk) {
		LOG_ERROR("Unable to open audio: %s", SDL_GetError());
		return;
	}

	LOG_DEBUG("Opened %dHz Audio at %d bit, %d channels with %d samples",
		audioSpec.freq, SDL_AUDIO_BITSIZE(audioSpec.format), audioSpec.channels, audioSpec.samples);

	// Load sounds
	soundsLoaded = loadSounds("SOUNDS.000");

	// Start audio for sfx to work
#if OJ_SDL2
	SDL_PauseAudioDevice(audioDevice, 0);
#else
	SDL_PauseAudio(0);
#endif
}


/**
 * Stop audio.
 */
void closeAudio () {
	stopMusic();

#if OJ_SDL2
	SDL_CloseAudioDevice(audioDevice);
	audioDevice = 0;
#else
	SDL_CloseAudio();
#endif

	if (rawSounds) {
		for (int i = 0; i < nRawSounds; i++) {
			delete[] rawSounds[i].data;
			delete[] rawSounds[i].name;
		}

		delete[] rawSounds;
	}

	if (soundsLoaded) freeSounds();
}


/**
 * Play music from the specified file.
 *
 * @param fileName Name of a file containing music data.
 * @param restart Restart music when same file is played.
 */
void playMusic (const char * fileName, bool restart) {
	/* Only stop any existing music playing, if a different file
	   should be played or a restart has been requested. */
	if ((currentMusic && (strcmp(fileName, currentMusic) == 0)) && !restart)
		return;

	stopMusic();

	LockAudio();

	// Load the music file
	File *file;
	try {
		file = new File(fileName, PATH_TYPE_GAME);
	} catch (int e) {
		UnlockAudio();
		return;
	}

	// Save current music filename
	if (currentMusic) delete[] currentMusic;
	currentMusic = createString(fileName);

	// Find the size of the file
	size_t size = file->getSize();

	// Read the entire file into memory
	file->seek(0, true);
	unsigned char *psmData = file->loadBlock(size);

	delete file;

	// Set up libpsmplug
	ModPlug_Settings settings = {};
	settings.mFlags = MUSIC_FLAGS;
	settings.mChannels = audioSpec.channels;
	settings.mBits = SDL_AUDIO_BITSIZE(audioSpec.format);
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
	delete[] psmData;

	if (!musicFile) {
		LOG_ERROR("Could not play music file: %s", fileName);
		delete[] currentMusic;
		currentMusic = nullptr;
	}

	// Re-apply volume setting
	setMusicVolume(musicVolume);

	// Start the audio playing
	musicPaused = false;

	UnlockAudio();
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
	LockAudio();

	if (musicFile) {
		ModPlug_Unload(musicFile);
		musicFile = nullptr;
	}

	// Cleanup
	if (currentMusic) {
		delete[] currentMusic;
		currentMusic = nullptr;
	}

	UnlockAudio();
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
	musicVolume = clamp_vol(volume, 0, MAX_VOLUME);

	// do not access music player settings when not playing
	if (!musicFile) return;

	ModPlug_SetMasterVolume(musicFile, musicVolume * 2.56);
}


/**
 * Gets the current music tempo
 *
 * @return music tempo (MUSIC_NORMAL, MUSIC_FAST)
 */
MusicTempo getMusicTempo () {
	return musicTempo;
}


/**
 * Sets the music tempo
 *
 * @param tempo new tempo (MUSIC_NORMAL, MUSIC_FAST)
 */
void setMusicTempo (MusicTempo tempo) {
	musicTempo = tempo;

	// do not access music player settings when not playing
	if (!musicFile) return;

	ModPlug_SetMusicTempoFactor(musicFile, static_cast<unsigned int>(tempo));
}


/**
 * Load raw sound clips from the specified file.
 *
 * @param fileName Name of a file containing sound clips
 */
bool loadSounds (const char *fileName) {
	File *file;

	try {
		file = new File(fileName, PATH_TYPE_GAME);
	} catch (int e) {
		return false;
	}

	// Locate the header data
	file->seek(file->getSize() - 4, true);
	size_t headerOffset = file->loadInt();

	// Calculate number of sounds
	nRawSounds = (file->getSize() - headerOffset) / 18;
	LOG_TRACE("Loading %d sounds...", nRawSounds);

	assert(nRawSounds < SE::MAX);

	// Load sound clips
	rawSounds = new RawSound[nRawSounds];

	for (int i = 0; i < nRawSounds; i++) {

		file->seek(headerOffset + (i * 18), true);

		// Read the name of the clip
		rawSounds[i].name = reinterpret_cast<char*>(file->loadBlock(12));

		// Read the offset of the clip
		int offset = file->loadInt();

		// Read the length of the clip
		rawSounds[i].length = file->loadShort();

		// Read the clip
		file->seek(offset, true);
		rawSounds[i].data = file->loadBlock(rawSounds[i].length);

	}
	delete file;

	resampleSounds();

	return true;
}


/**
 * Resample sound clip data.
 */
void resampleSound (int index, const char* name, int rate) {
	// Skip SE::NONE
	int se = index + 1;

	if(!isValidSoundIndex(static_cast<SE::Type>(se))) {
		LOG_ERROR("Cannot resample Sound Index %d", se);
		return;
	}

	if (sounds[se].data) {
		LOG_TRACE("Overwriting Sound index %d: %s", se, name);

		delete[] sounds[se].data;
		sounds[se].data = nullptr;
	}

	// Search for matching sound
	for (int i = 0; i < nRawSounds; i++) {
		if (strcmp(name, rawSounds[i].name) != 0) continue;

#if OJ_SDL2
		// We let SDL2 resample as needed
		SDL_AudioCVT cvt;
		int res = SDL_BuildAudioCVT(&cvt, AUDIO_S8, 1, rate, audioSpec.format,
			audioSpec.channels, audioSpec.freq);
		if (res >= 0) {
			cvt.len = rawSounds[i].length;
			cvt.buf = new unsigned char[cvt.len * cvt.len_mult];
			if(!cvt.buf) {
				LOG_ERROR("Cannot create conversion buffer.");
				return;
			}
			memcpy(cvt.buf, rawSounds[i].data, cvt.len);
			sounds[se].length = cvt.len;
			// only convert, if needed
			if (res > 0) {
				if((res = SDL_ConvertAudio(&cvt)) == 0) {
					// successful
					sounds[se].length = cvt.len_cvt;
				}
			}
		}
		if(res < 0) {
			LOG_WARN("Cannot resample sound effect: %s", SDL_GetError());
			return;
		}
		// From here it does not matter, if converted or already right samplerate
		sounds[se].data = new unsigned char[sounds[se].length];
		if(!sounds[se].data) {
			LOG_ERROR("Cannot create buffer for resampled sound effect.");
			return;
		}
		// Copy data over
		memcpy(sounds[se].data, cvt.buf, sounds[se].length * sizeof(unsigned char));
		delete[](cvt.buf);
#else
		// Calculate the resampling factor
		int rsFactor;
		if (SDL_AUDIO_BITSIZE(audioSpec.format) == 8)
			rsFactor = (F2 * audioSpec.freq) / rate;
		else
			rsFactor = (F4 * audioSpec.freq) / rate;

		sounds[se].length = MUL(rawSounds[i].length, rsFactor);

		// Allocate the buffer for the resampled clip
		sounds[se].data = new unsigned char[sounds[se].length];
		if(!sounds[se].data) {
			LOG_ERROR("Cannot create buffer for resampled sound effect.");
			return;
		}

		// Resample the clip
		for (int sample = 0; sample < sounds[se].length; sample++)
			sounds[se].data[sample] = rawSounds[i].data[DIV(sample, rsFactor)];
#endif
		sounds[se].position = -1;

		return;
	}
}


/**
 * Resample all sound clips to matching indices.
 */
void resampleSounds() {
	for (int i = 0; i < nRawSounds; i++) {
		resampleSound(i, rawSounds[i].name, 11025);
	}
}


/**
 * Delete resampled sound clip data.
 */
void freeSounds() {
	if (!soundsLoaded) return;

	for (int i = SE::NONE; i < SE::MAX; i++) {
		if (sounds[i].data) delete[] sounds[i].data;
	}
}


/**
 * Set the sound clip to be played.
 *
 * @param index Number of the sound to play
 */
void playSound(SE::Type index) {
	if (!soundsLoaded) return;

	if (!isValidSoundIndex(index)) {
		LOG_WARN("Cannot play invalid sound %d", index);
		return;
	}

	sounds[index].position = 0;
}


/**
 * Check if a sound clip is playing.
 *
 * @param index Number of the sound to check
 *
 * @return Whether the sound is playing
 */
bool isSoundPlaying (SE::Type index) {
	if (!soundsLoaded || !isValidSoundIndex(index)) return false;

	return (sounds[index].position > 0);
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
	soundVolume = clamp_vol(volume, 0, MAX_VOLUME);
}
