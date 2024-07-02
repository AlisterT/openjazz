
/**
 *
 * @file setup.cpp
 *
 * Part of the OpenJazz project
 *
 * @par History:
 * - 23rd of August 2005: Created main.c and menu.c
 * - 3rd of February 2009: Renamed main.c to main.cpp and menu.c to menu.cpp
 * - 18th July 2009: Created menusetup.cpp from parts of menu.cpp
 * - 26th July 2009: Renamed menusetup.cpp to setupmenu.cpp
 * - 21st July 2013: Created setup.cpp from parts of main.cpp and setupmenu.cpp
 *
 * @par Licence:
 * Copyright (c) 2005-2017 Alister Thomson
 *
 * OpenJazz is distributed under the terms of
 * the GNU General Public License, version 2.0
 *
 * @par Description:
 * Deals with the running of setup menus.
 *
 */


#include "io/controls.h"
#include "io/file.h"
#include "io/gfx/video.h"
#include "io/sound.h"
#include "player/player.h"
#include "setup.h"
#include "util.h"
#include "io/log.h"
#ifdef USE_CONFUSE
#include <confuse.h>
#endif

#define CONFIG_FILE "openjazz.cfg"

#ifdef USE_CONFUSE
namespace {
	// enumerations
	const char *controlNames[CONTROLS] = {
		"Up", "Down", "Left", "Right", "Jump", "SwimUp", "Fire", "ChangeWeapon",
		"Enter", "Escape", "Blaster", "Toaster", "Missile", "Bouncer", "TNT",
		"Stats", "Pause", "Yes", "No"
	};

	// validation/callback/print functions
	int cfg_skip_values(cfg_t *cfg, cfg_opt_t *opt) {
		int size = cfg_opt_size(opt);
		// skip unset
		if(size == 0) return 1;
		// do not skip lists
		if(size > 1) return 0;

		// skip defaults
		switch (opt->type) {
		case CFGT_INT:
			return opt->values[0]->number == opt->def.number ? 1 : 0;
		case CFGT_FLOAT:
			return opt->values[0]->fpnumber == opt->def.fpnumber ? 1 : 0;
		case CFGT_BOOL:
			return opt->values[0]->boolean == opt->def.boolean ? 1 : 0;

		// do not skip changed sections
		case CFGT_SEC:
			return opt->flags&CFGF_MODIFIED ? 0 : 1;

		// notify programming errors
		case CFGT_STR:
		default:
			cfg_error(cfg, "possible problem in cfg_skip_values(%s)", opt->name);
			break;
		}
		// do not skip rest
		return 0;
	}

#ifndef FULLSCREEN_ONLY
	int cfg_cb_mode(cfg_t *cfg, cfg_opt_t *opt, const char *value, void *result) {
		if (strcmp(value, "Window") == 0)
			*(cfg_bool_t *)result = cfg_false;
		else if (strcmp(value, "Fullscreen") == 0)
			*(cfg_bool_t *)result = cfg_true;
		else {
			cfg_error(cfg, "Invalid value for option %s: %s", opt->name, value);
			return -1;
		}
		return 0;
	}
	void cfg_print_mode(cfg_opt_t *opt, unsigned int index, FILE *fp) {
		cfg_bool_t value = cfg_opt_getnbool(opt, index);
		switch (value) {
		case cfg_false:
			fprintf(fp, "Window");
			break;
		case cfg_true:
		default:
			fprintf(fp, "Fullscreen");
			break;
		}
	}
#endif

	int cfg_cb_hatDir(cfg_t *cfg, cfg_opt_t *opt, const char *value, void *result) {
		if (strcmp(value, "Up") == 0)
			*(long int *)result = 1;
		else if (strcmp(value, "Right") == 0)
			*(long int *)result = 2;
		else if (strcmp(value, "Down") == 0)
			*(long int *)result = 4;
		else if (strcmp(value, "Left") == 0)
			*(long int *)result = 8;
		else {
			cfg_error(cfg, "Invalid value for option %s: %s", opt->name, value);
			return -1;
		}
		return 0;
	}
	void cfg_print_hatDir(cfg_opt_t *opt, unsigned int index, FILE *fp) {
		int value = cfg_opt_getnint(opt, index);
		switch (value) {
		case 1:
			fprintf(fp, "Up");
			break;
		case 2:
			fprintf(fp, "Right");
			break;
		case 4:
			fprintf(fp, "Down");
			break;
		case 8:
		default:
			fprintf(fp, "Left");
			break;
		}
	}

	int cfg_cb_axisDir(cfg_t *cfg, cfg_opt_t *opt, const char *value, void *result) {
		if (strcmp(value, "Negative") == 0)
			*(long int *)result = 0;
		else if (strcmp(value, "Positive") == 0)
			*(long int *)result = 1;
		else {
			cfg_error(cfg, "Invalid value for option %s: %s", opt->name, value);
			return -1;
		}
		return 0;
	}
	void cfg_print_axisDir(cfg_opt_t *opt, unsigned int index, FILE *fp) {
		int value = cfg_opt_getnint(opt, index);
		switch (value) {
		case 0:
			fprintf(fp, "Negative");
			break;
		case 1:
		default:
			fprintf(fp, "Positive");
			break;
		}
	}

	void cfg_error_logger(cfg_t *cfg, const char *fmt, va_list ap) {
		// use an immediate buffer for output
		char outbuffer[1024];
		vsprintf(outbuffer, fmt, ap);
		LOG_WARN("configuration parser: %s", outbuffer);
		if (cfg && cfg->filename && cfg->line)
			LOG_TRACE("(in %s at line %d)", cfg->filename, cfg->line);
		else if (cfg && cfg->filename)
			LOG_TRACE("(in %s)", cfg->filename);
	}

	// configuration format
	cfg_opt_t player_color_opts[] = {
		CFG_INT("fur", CHAR_FUR, CFGF_NONE),
		CFG_INT("bandana", CHAR_BAND, CFGF_NONE),
		CFG_INT("gun", CHAR_GUN, CFGF_NONE),
		CFG_INT("wristband", CHAR_WBAND, CFGF_NONE),
		CFG_END()
	};
	cfg_opt_t player_opts[] = {
		CFG_STR("name", CHAR_NAME, CFGF_NONE),
		CFG_SEC("colors", player_color_opts, CFGF_NONE),
		CFG_END()
	};
	cfg_opt_t game_opts[] = {
		CFG_BOOL("unlimited-birds", cfg_false, CFGF_NONE),
		CFG_BOOL("leave-extra-items", cfg_true, CFGF_NONE),
		CFG_BOOL("slow-motion", cfg_false, CFGF_NONE),
		CFG_BOOL("scale2x", cfg_true, CFGF_NONE),
		CFG_END()
	};
	cfg_opt_t video_opts[] = {
		CFG_INT("width", DEFAULT_SCREEN_WIDTH, CFGF_NONE),
		CFG_INT("height", DEFAULT_SCREEN_HEIGHT, CFGF_NONE),
		CFG_INT("scale", 1, CFGF_NONE),
#ifndef FULLSCREEN_ONLY
		CFG_BOOL_CB("mode", cfg_false, CFGF_NONE, &cfg_cb_mode),
#endif
		CFG_END()
	};
	cfg_opt_t audio_music_opts[] = {
		CFG_INT("volume", 0, CFGF_NODEFAULT),
		CFG_END()
	};
	cfg_opt_t audio_sound_opts[] = {
		CFG_INT("volume", 0, CFGF_NODEFAULT),
		CFG_END()
	};
	cfg_opt_t audio_opts[] = {
		CFG_SEC("music", audio_music_opts, CFGF_NONE),
		CFG_SEC("sound", audio_sound_opts, CFGF_NONE),
		CFG_END()
	};
	cfg_opt_t input_axis_opts[] = {
		CFG_INT("number", -1, CFGF_NODEFAULT),
		CFG_INT_CB("direction", -1, CFGF_NODEFAULT, &cfg_cb_axisDir),
		CFG_END()
	};
	cfg_opt_t input_hat_opts[] = {
		CFG_INT("number", -1, CFGF_NODEFAULT),
		CFG_INT_CB("direction", -1, CFGF_NODEFAULT, &cfg_cb_hatDir),
		CFG_END()
	};
	cfg_opt_t input_controller_opts[] = {
		CFG_INT("button", -1, CFGF_NODEFAULT),
		CFG_SEC("axis", input_axis_opts, CFGF_NONE),
		CFG_SEC("hat", input_hat_opts, CFGF_NONE),
		CFG_END()
	};
	cfg_opt_t input_control_opts[] = {
#if OJ_SDL2
		CFG_STR("keyboard", NULL, CFGF_NODEFAULT),
#else
		CFG_INT("keyboard", -1, CFGF_NODEFAULT),
#endif
		CFG_SEC("controller", input_controller_opts, CFGF_NONE),
		CFG_END()
	};
	cfg_opt_t input_opts[] = {
		CFG_SEC("control", input_control_opts, CFGF_MULTI|CFGF_TITLE),
		CFG_END()
	};
	cfg_opt_t opts[] = {
		CFG_INT("version", 7, CFGF_NONE),
		CFG_SEC("player", player_opts, CFGF_NONE),
		CFG_SEC("game", game_opts, CFGF_NONE),
		CFG_SEC("video", video_opts, CFGF_NONE),
		CFG_SEC("audio", audio_opts, CFGF_NONE),
		CFG_SEC("input", input_opts, CFGF_NONE),
		CFG_END()
	};

	// helpers
	int setBool(cfg_t *cfg, const char *name, bool value) {
		cfg_bool_t val = value ? cfg_true : cfg_false;
		return cfg_setbool(cfg, name, val);
	}
	bool getBool(cfg_t *cfg, const char *name) {
		return cfg_getbool(cfg, name) == cfg_true;
	}
}
#endif

namespace {
	// enumerations
	struct {
		const char *name;
		int color;
	} colorInfo[] = {
		{ "fur",       CHAR_FUR },
		{ "bandana",   CHAR_BAND },
		{ "gun",       CHAR_GUN },
		{ "wristband", CHAR_WBAND }
	};
}

/**
 * Create default setup
 */
Setup::Setup () {

	// Create the player's name
	characterName = createEditableString(CHAR_NAME);

	// Assign the player's colour
	for (int i = 0; i < 4; i++)
		characterCols[i] = colorInfo[i].color;

	// defaults
	scale2x = true;
	manyBirds = false;
	leaveUnneeded = true;
	slowMotion = false;

}


/**
 * Delete the setup data
 */
Setup::~Setup () {

	delete[] characterName;

}


/**
 * Load settings from config file.
 */
SetupOptions Setup::load () {
	File* file;
	SetupOptions cfg = { false, 0, 0, false, 0 };
#ifdef FULLSCREEN_ONLY
	cfg.fullScreen = true;
#endif

	// Open config file
	try {
		file = new File(CONFIG_FILE, PATH_TYPE_CONFIG, false);
	} catch (int e) {
		LOG_INFO("Configuration file not found. Using defaults.");

		return cfg;
	}

#ifdef USE_CONFUSE
	cfg_t *handle, *sec, *subSec;
	handle = cfg_init(opts, CFGF_IGNORE_UNKNOWN);
	cfg_set_error_function(handle, cfg_error_logger);

	if(cfg_parse_fp(handle, file->getFP()) != CFG_PARSE_ERROR) {
		// Read video settings
		sec = cfg_getsec(handle, "video");
		cfg.videoWidth = cfg_getint(sec, "width");
		cfg.videoHeight = cfg_getint(sec, "height");
#ifndef FULLSCREEN_ONLY
		cfg.fullScreen = cfg_getbool(sec, "mode");
#endif
#ifdef SCALE
		cfg.videoScale = cfg_getint(sec, "scale");
#endif
		cfg.valid = true;

		// Read controls
		for (int i = 0; i < CONTROLS; i++) {
			sec = cfg_gettsec(handle, "input|control", controlNames[i]);
			if(cfg_size(sec, "keyboard") == 1) {
#if OJ_SDL2
				char *keyName = cfg_getstr(sec, "keyboard");
				SDL_Keycode key = SDL_GetKeyFromName(keyName);
				if(key != SDLK_UNKNOWN)
					controls.setKey(i, key);
				else
					LOG_DEBUG("Cannot translate key name \"%s\", using default.", keyName);
#else
				controls.setKey(i, cfg_getint(sec, "keyboard"));
#endif
			}

			subSec = cfg_getsec(sec, "controller");
			if(cfg_size(subSec, "button") == 1)
				controls.setButton(i, cfg_getint(subSec, "button"));

			if(cfg_size(subSec, "axis|number") == 1 &&
				cfg_size(subSec, "axis|direction") == 1) {
				int n = cfg_getint(subSec, "axis|number");
				int d = cfg_getint(subSec, "axis|direction");
				controls.setAxis(i, n, d);
			}

			if(cfg_size(subSec, "hat|number") == 1 &&
				cfg_size(subSec, "hat|direction") == 1) {
				int n = cfg_getint(subSec, "hat|number");
				int d = cfg_getint(subSec, "hat|direction");
				controls.setHat(i, n, d);
			}
		}

		// Read the player's name
		sec = cfg_getsec(handle, "player");
		strcpy(characterName, cfg_getstr(sec, "name"));

		// Read the player's colours
		subSec = cfg_getsec(sec, "colors");
		for (int i = 0; i < 4; i++) {
			characterCols[i] = cfg_getint(subSec, colorInfo[i].name);
		}

		// Read the music and sound effect volume
		sec = cfg_getsec(handle, "audio");
		if(cfg_size(sec, "music|volume") == 1)
			setMusicVolume(cfg_getint(sec, "music|volume"));
		if(cfg_size(sec, "sound|volume") == 1)
			setSoundVolume(cfg_getint(sec, "sound|volume"));

		// Read gameplay options
		sec = cfg_getsec(handle, "game");
		manyBirds = getBool(sec, "unlimited-birds");
		leaveUnneeded = getBool(sec, "leave-extra-items");
		slowMotion = getBool(sec, "slow-motion");
		scale2x = getBool(sec, "scale2x");
	}

	// close parser
	cfg_free(handle);
	if(cfg.valid) {
		delete file;
		return cfg;
	}

	// Invalid config, rewind and try old format
	file->seek(0, true);
	LOG_DEBUG("Invalid configuration format, trying old format.");
#endif

	// Check that the config file has the correct version
	if (file->loadChar() != 6) {
		LOG_WARN("Valid configuration file not found.");

		delete file;
		return cfg;
	}

	// Read video settings
	cfg.videoWidth = file->loadShort(MAX_SCREEN_WIDTH);
	cfg.videoHeight = file->loadShort(MAX_SCREEN_HEIGHT);
	int vOpt = file->loadChar();
#ifndef FULLSCREEN_ONLY
	cfg.fullScreen = vOpt & 1;
#endif
#ifdef SCALE
	if (vOpt >= 10) vOpt = 2;
	cfg.videoScale = vOpt >> 1;
#endif
	(void)vOpt;
	cfg.valid = true;

	// Read controls
	for (int i = 0; i < CONTROLS - 4; i++)
		controls.setKey(i, file->loadInt());

	for (int i = 0; i < CONTROLS; i++)
		controls.setButton(i, file->loadInt());

	for (int i = 0; i < CONTROLS; i++) {
		int a = file->loadInt();
		int d = file->loadInt();
		controls.setAxis(i, a, d);
	}

	for (int i = 0; i < CONTROLS; i++) {
		int h = file->loadInt();
		int d = file->loadInt();
		controls.setHat(i, h, d);
	}

	// Read the player's name
	for (int i = 0; i < STRING_LENGTH; i++)
		characterName[i] = file->loadChar();
	characterName[STRING_LENGTH] = 0;

	// Read the player's colours
	characterCols[0] = file->loadChar();
	characterCols[1] = file->loadChar();
	characterCols[2] = file->loadChar();
	characterCols[3] = file->loadChar();

	// Read the music and sound effect volume
	setMusicVolume(file->loadChar());
	setSoundVolume(file->loadChar());

	// Read gameplay options
	int opt = file->loadChar();
	manyBirds = ((opt & 1) != 0);
	leaveUnneeded = ((opt & 2) != 0);
	slowMotion = ((opt & 4) != 0);
	scale2x = ((opt & 8) == 0);

	delete file;
	return cfg;
}


/**
 * Save settings to config file.
 */
void Setup::save () {
	File *file;

	// Open config file
	try {
		file = new File(CONFIG_FILE, PATH_TYPE_CONFIG, true);
	} catch (int e) {
		LOG_ERROR("Could not write configuration file: File could not be opened.");

		return;
	}

#ifdef USE_CONFUSE
	cfg_t *handle, *sec, *subSec;
	handle = cfg_init(opts, CFGF_COMMENTS);
	cfg_set_error_function(handle, cfg_error_logger);

	// Write the version number
	cfg_setint(handle, "version", 7);

	// Write video settings
	sec = cfg_getsec(handle, "video");
	cfg_setint(sec, "width", video.getWidth());
	cfg_setint(sec, "height", video.getHeight());
#ifdef SCALE
	cfg_setint(sec, "scale", video.getScaleFactor());
#endif
#ifndef FULLSCREEN_ONLY
	cfg_set_print_func(sec, "mode", cfg_print_mode);
	setBool(sec, "mode", video.isFullscreen());
#endif

	// Write controls
	for (int i = 0; i < CONTROLS; i++) {
		sec = cfg_addtsec(handle, "input|control", controlNames[i]);

#if OJ_SDL2
		cfg_setstr(sec, "keyboard", SDL_GetKeyName(controls.getKey(i)));
#else
		// For SDL1.2 we only add the name as comment,
		// since there is no reverse translation
		cfg_setint(sec, "keyboard", controls.getKey(i));
		cfg_setcomment(sec, "keyboard", SDL_GetKeyName((SDLKey)controls.getKey(i)));
#endif

		subSec = cfg_getsec(sec, "controller");
		cfg_set_print_filter_func(subSec, cfg_skip_values);

		cfg_setint(subSec, "button", controls.getButton(i));

		int axis = controls.getAxis(i);
		if(axis != -1) {
			cfg_set_print_func(subSec, "axis|direction", cfg_print_axisDir);
			cfg_setint(subSec, "axis|number", axis);
			cfg_setint(subSec, "axis|direction", controls.getAxisDirection(i));
		}

		int hat = controls.getHat(i);
		if(hat != -1) {
			cfg_set_print_func(subSec, "hat|direction", cfg_print_hatDir);
			cfg_setint(subSec, "hat|number", hat);
			cfg_setint(subSec, "hat|direction", controls.getHatDirection(i));
		}
	}

	// Write the player's name
	sec = cfg_getsec(handle, "player");
	if(strcmp(characterName, CHAR_NAME) != 0)
		cfg_setstr(sec, "name",	characterName);

	// Write the player's colour
	subSec = cfg_getsec(sec, "colors");
	for (int i = 0; i < 4; i++) {
		if(characterCols[i] != colorInfo[i].color)
			cfg_setint(subSec, colorInfo[i].name, characterCols[i]);
	}

	// Write the music and sound effect volume
	sec = cfg_getsec(handle, "audio");
	cfg_setint(sec, "music|volume", getMusicVolume());
	cfg_setint(sec, "sound|volume", getSoundVolume());

	// Write gameplay options
	sec = cfg_getsec(handle, "game");
	setBool(sec, "unlimited-birds", manyBirds);
	setBool(sec, "leave-extra-items", leaveUnneeded);
	setBool(sec, "slow-motion", slowMotion);
	setBool(sec, "scale2x", scale2x);

	// Add comments
	char commentBuf[1024];
	strcpy(commentBuf, "Keep the name short and lowercase");
	cfg_setcomment(handle, "player|name", commentBuf);
	strcpy(commentBuf, "Volumes can be 0-100");
	cfg_setcomment(handle, "audio", commentBuf);

	// Write preamble
	fprintf(file->getFP(), "# %s\n# %s\n\n",
		"OpenJazz configuration", "All options are lowercase");

	// write configuration
	cfg_print(handle, file->getFP());
	cfg_free(handle);
#else
	// Write the version number
	file->storeChar(6);

	// Write video settings
	file->storeShort(video.getWidth());
	file->storeShort(video.getHeight());
	int videoScale;
#ifdef SCALE
	videoScale = video.getScaleFactor();
#else
	videoScale = 1;
#endif
	videoScale <<= 1;
#ifndef FULLSCREEN_ONLY
	videoScale |= video.isFullscreen()? 1: 0;
#endif
	file->storeChar(videoScale);

	// Write controls
	int count;
	for (count = 0; count < CONTROLS - 4; count++)
		file->storeInt(controls.getKey(count));

	for (count = 0; count < CONTROLS; count++)
		file->storeInt(controls.getButton(count));

	for (count = 0; count < CONTROLS; count++) {
		file->storeInt(controls.getAxis(count));
		file->storeInt(controls.getAxisDirection(count));
	}

	for (count = 0; count < CONTROLS; count++) {
		file->storeInt(controls.getHat(count));
		file->storeInt(controls.getHatDirection(count));
	}

	// Write the player's name
	for (count = 0; count < STRING_LENGTH; count++)
		file->storeChar(characterName[count]);

	// Write the player's colour
	file->storeChar(characterCols[0]);
	file->storeChar(characterCols[1]);
	file->storeChar(characterCols[2]);
	file->storeChar(characterCols[3]);

	// Write the music and sound effect volume
	file->storeChar(getMusicVolume());
	file->storeChar(getSoundVolume());

	// Write gameplay options
	count = 0;
	if (manyBirds) count |= 1;
	if (leaveUnneeded) count |= 2;
	if (slowMotion) count |= 4;
	if (!scale2x) count |= 8;
	file->storeChar(count);
#endif

	delete file;
}
