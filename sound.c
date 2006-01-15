
/*
 *
 * sound.h
 * Created on the 23rd of August 2005
 *
 * Part of the OpenJazz project
 *
 *
 * Copyright (c) 2005 Alister Thomson
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


unsigned char *psmData;
ModPlugFile   *musicFile;


void audioCallback (void * userdata, unsigned char * stream, int len) {

  unsigned char *musicData;

  // Allocate memory in which to read the music
  musicData = malloc(len);

  // Read the next portion of music
  ModPlug_Read(musicFile, musicData, len);

  // Add the next portion of music to the audio stream
  SDL_MixAudio(stream, musicData, len, SDL_MIX_MAXVOLUME);

  // Free the music data
  free(musicData);

}

#endif

int loadMusic (char * fn) {

#ifdef USE_MODPLUG

  FILE *f;
  SDL_AudioSpec audioSpec;
  ModPlug_Settings settings;
  int size;


  f = fopenFromPath(fn);

  // Find the size of the file
  fseek(f, 0, SEEK_END);
  size = ftell(f);

  // Allocate memory into which the file will be read
  psmData = malloc(size);

  // Read the entire file into memory
  fseek(f, 0, SEEK_SET);
  fread(psmData, size, 1, f);

  fclose(f);

  // Set up libmodplug

  settings.mFlags = MODPLUG_ENABLE_OVERSAMPLING |
                    MODPLUG_ENABLE_NOISE_REDUCTION |
                    MODPLUG_ENABLE_REVERB | MODPLUG_ENABLE_MEGABASS |
                    MODPLUG_ENABLE_SURROUND;
  settings.mChannels = 2;
  settings.mBits = 16;
  settings.mFrequency = 22050;
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

  // Set up SDL audio

  audioSpec.freq = 22050;
  audioSpec.format = AUDIO_S16;
  audioSpec.channels = 2;
  audioSpec.samples = 512;
  audioSpec.callback = audioCallback;
  audioSpec.userdata = NULL;

  if (SDL_OpenAudio(&audioSpec, NULL) < 0) {

      fprintf(stderr, "Unable to open audio: %s\n", SDL_GetError());
      return RETURN;

  }

  // Start the music playing
  SDL_PauseAudio(0);

#endif

  return CONTINUE;

}


void freeMusic (void) {

#ifdef USE_MODPLUG

  SDL_CloseAudio();

  ModPlug_Unload(musicFile);

  free(psmData);

#endif

  return;

}


int loadSounds (char *fn) {

  FILE *f;

  f = fopenFromPath(fn);

  // To do

  fclose(f);

  return -1;

}


void freeSounds (void) {

  // To do

  return;

}


void playSound (int sound) {

  // To do

  return;

}

