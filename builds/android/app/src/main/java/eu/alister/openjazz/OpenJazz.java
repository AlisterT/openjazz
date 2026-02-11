package eu.alister.openjazz;

import org.libsdl.app.SDLActivity;

/**
 * Wrapper class over SDLActivity
 */

public class OpenJazz extends SDLActivity {
	@Override
	protected String[] getLibraries() {
		return new String[]{
			"SDL2",
			"OpenJazz"
		};
	}
}
