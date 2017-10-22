
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
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 * @par Description:
 * Deals with the loading, playing and freeing of music and sound effects.
 *
 * For music, USE_MODPLUG or USE_XMP must be defined.
 *
 */


#include "file.h"
#include "sound.h"

#include "util.h"

#include <SDL/SDL_audio.h>

#if defined(USE_MODPLUG)
    #ifdef _WIN32
        #include <modplug.h>
    #else
        #include <libmodplug/modplug.h>
    #endif
#elif defined(USE_XMP)
	#include <xmp.h>
#endif


#ifdef __SYMBIAN32__
	#define SOUND_FREQ 22050
#else
	#define SOUND_FREQ 44100
#endif

#if defined(USE_MODPLUG)

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

ModPlugFile   *musicFile;

#elif defined(USE_XMP)

	#define MUSIC_INTERPOLATION XMP_INTERP_SPLINE
	#define MUSIC_EFFECTS XMP_DSP_ALL

xmp_context xmpC;

#endif

SDL_AudioSpec  audioSpec;
bool music_paused = false;


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

	if (!music_paused) {
		// Read the next portion of music into the audio stream
#if defined(USE_MODPLUG)

		if (musicFile) {
			int bytes_read = ModPlug_Read(musicFile, stream, len);

			// poor mans loop (so modplug needs no patching)
			if (bytes_read < len) {
				ModPlug_Seek(musicFile, 0);
				ModPlug_Read(musicFile, stream + bytes_read, len - bytes_read);
			}
		}

#elif defined(USE_XMP)

		if (xmp_get_player(xmpC, XMP_PLAYER_STATE) == XMP_STATE_PLAYING)
			xmp_play_buffer(xmpC, stream, len, 0);

#endif
	}

	for (count = 0; count < nSounds; count++) {

		if (sounds[count].position >= 0) {

			// Add the next portion of the sound clip to the audio stream

			if (len < sounds[count].length - sounds[count].position) {

				// Play as much of the clip as possible

				SDL_MixAudio(stream,
					sounds[count].data + sounds[count].position, len,
					soundsVolume * SDL_MIX_MAXVOLUME / MAX_VOLUME);

				sounds[count].position += len;

			} else {

				// Play the remainder of the clip

				SDL_MixAudio(stream,
					sounds[count].data + sounds[count].position,
					sounds[count].length - sounds[count].position,
					soundsVolume * SDL_MIX_MAXVOLUME / MAX_VOLUME);

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

#if defined(USE_MODPLUG)
	musicFile = NULL;
#elif defined(USE_XMP)
	xmpC = xmp_create_context();
#endif


	// Set up SDL audio

	asDesired.freq = SOUND_FREQ;
	asDesired.format = AUDIO_S16;
	asDesired.channels = 2;
#if defined(GP2X) || defined(PSP)
	asDesired.samples = 512;
#else
	asDesired.samples = 2048;
#endif
	asDesired.callback = audioCallback;
	asDesired.userdata = NULL;

	if (SDL_OpenAudio(&asDesired, &audioSpec) < 0)
		logError("Unable to open audio", SDL_GetError());


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

	stopMusic();

#ifdef USE_XMP
	xmp_free_context(xmpC);
#endif

	SDL_CloseAudio();

	freeSounds();

	return;

}


/**
 * Play music from the specified file.
 *
 * @param fileName Name of a file containing music data.
 */
void playMusic (const char * fileName) {

	File *file;
	unsigned char *psmData;
	int size;
	bool loadOk = false;

#ifdef USE_MODPLUG
	ModPlug_Settings settings;
#endif

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


#ifdef USE_MODPLUG

	// Set up libmodplug

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
	settings.mLoopCount = -1;

	ModPlug_SetSettings(&settings);

	// Load the file into libmodplug
	musicFile = ModPlug_Load(psmData, size);
	loadOk = (musicFile != NULL);

#elif defined(USE_XMP)

	// Load the file into libxmp
	loadOk = (xmp_load_module_from_memory(xmpC, psmData, size) == 0);

#endif

	delete[] psmData;

	if (!loadOk) {

		logError("Could not play music file", fileName);

		return;

	}

#ifdef USE_XMP
	int playerFlags = 0;

	if ((audioSpec.format == AUDIO_U8) || (audioSpec.format == AUDIO_S8))
		playerFlags = playerFlags & XMP_FORMAT_8BIT;

	if ((audioSpec.format == AUDIO_U8) || (audioSpec.format == AUDIO_U16)
		|| (audioSpec.format == AUDIO_U16MSB) || (audioSpec.format == AUDIO_U16LSB))
		playerFlags = playerFlags & XMP_FORMAT_UNSIGNED;

	if (audioSpec.channels == 1)
		playerFlags = playerFlags & XMP_FORMAT_MONO;

	xmp_start_player(xmpC, audioSpec.freq, playerFlags);
	xmp_set_player(xmpC, XMP_PLAYER_INTERP, MUSIC_INTERPOLATION);
	xmp_set_player(xmpC, XMP_PLAYER_DSP, MUSIC_EFFECTS);

#endif

	// Start the audio playing
	SDL_PauseAudio(0);

	return;

}


/**
 * Pauses and Unpauses the current music.
 */
void pauseMusic (bool pause) {
	music_paused = pause;
}


/**
 * Stop the current music.
 */
void stopMusic () {

	// Stop the music playing
	SDL_PauseAudio(~0);

#if defined(USE_MODPLUG)

	if (musicFile) {

		ModPlug_Unload(musicFile);
		musicFile = NULL;

	}

#elif defined(USE_XMP)

	int state = xmp_get_player(xmpC, XMP_PLAYER_STATE);
	if (state == XMP_STATE_LOADED || state == XMP_STATE_PLAYING) {

		xmp_end_player(xmpC);
		xmp_release_module(xmpC);

	}

#endif

	SDL_PauseAudio(0);

	return;

}


/**
 * Load sound clips from the specified file.
 *
 * @param fileName Name of a file containing sound clips
 */
int loadSounds (const char *fileName) {

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


/**
 * Delete sound clip data.
 */
void freeSounds () {

	int count;

	if (sounds) {

		for (count = 0; count < nSounds; count++) {

			delete[] sounds[count].data;
			delete[] sounds[count].name;

		}

		delete[] sounds;

	}

	return;

}


/**
 * Set the sound clip to be played.
 */
void playSound (int newSound) {

	if (sounds && (newSound >= 0)) sounds[newSound].position = 0;

	return;

}


