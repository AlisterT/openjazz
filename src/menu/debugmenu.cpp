
/**
 *
 * @file debugmenu.cpp
 *
 * Part of the OpenJazz project
 *
 * @par Licence:
 * Copyright (c) 2015-2026 Carsten Teibes
 *
 * OpenJazz is distributed under the terms of
 * the GNU General Public License, version 2.0
 *
 * @par Description:
 * Deals with the running of the debug menu.
 *
 */

#include "menu.h"

#include "level/level.h"
#include "io/controls.h"
#include "io/gfx/font.h"
#include "io/gfx/video.h"
#include "io/sound.h"
#include "debug.h"
#include "loop.h"
#include "util.h"
#include "io/log.h"
#include <array>

#if OJ_DEBUG

class DebugPage {
	public:
		explicit DebugPage(DebugMenu *owner) : owner(owner) {}
		virtual ~DebugPage() = default;

		virtual void handleUpDown(bool isUp) = 0;
		virtual void handleLeftRight(bool isLeft) {
			if (isLeft)
				owner->showPage(owner->getPage()-1);
			else
				owner->showPage(owner->getPage()+1);
		}
		virtual void handleCursor(Point &position) = 0;
		virtual int handleEnter() = 0;

		virtual void draw() = 0;

	protected:
		DebugPage();
		DebugMenu *owner;
};

class PageDisplay final : public DebugPage {
	int chosen = 0;
	static constexpr int nOptions = 7;

	std::array<ShowFlags, nOptions> debugFlags = { DS_TILEMASK, DS_BGTILES, DS_FGTILES, DS_ANIMTILES,
		DS_PLAYERAREA, DS_PLAYERBASE, DS_PLAYEREVENTTILE };
	const char *debugOptions[nOptions] = { "tilemask", "background tiles", "foreground tiles",
		"animated tiles", "player area", "player base", "player event tile" };

	void toggleFlag() {
		if(debug::showFlags & debugFlags[chosen])
			debug::showFlags &= ~(debugFlags[chosen]); // clear
		else
			debug::showFlags |= debugFlags[chosen]; // set

		playConfirmSound();
	};

	public:
		explicit PageDisplay(DebugMenu *owner) : DebugPage(owner) {}

		void handleUpDown(bool isUp) override {
			if (isUp)
				chosen = (chosen + nOptions - 1) % nOptions;
			else
				chosen = (chosen + 1) % nOptions;
		}

		void handleCursor(Point &position) override {
			position.x -= canvasW >> 2;
			position.y -= (canvasH >> 1) - (nOptions << 3);

			if ((position.x >= 0) && (position.x < 256) && (position.y >= 0) && (position.y < (nOptions << 4))) {
				chosen = position.y >> 4;

				if (controls.wasCursorReleased())
					toggleFlag();
			}
		}
		int handleEnter() override {
			toggleFlag();
			return E_NONE;
		}

		void draw() override {
			fontmn2->showString("DISPLAY DEBUG MENU", canvasW >> 1, (canvasH >> 1) - (nOptions << 3) - 32, alignX::Center);

			for (int i = 0; i < nOptions; i++) {

				if (i == chosen) fontmn2->mapPalette(240, 8, 114, 16);

				fontmn2->showString(debugOptions[i], (canvasW >> 1) - 100,
					(canvasH >> 1) + (i << 4) - (nOptions << 3));

				fontmn2->showString(debug::showFlags & debugFlags[i] ? "show" : "hide", (canvasW >> 1) + 60,
					(canvasH >> 1) + (i << 4) - (nOptions << 3));

				if (i == chosen) fontmn2->restorePalette();

			}
		}
};

class PageCheats final : public DebugPage {
	int chosen = 0;
	static constexpr int nOptions = 8;

	std::array<CheatFlags, nOptions> debugFlags = { DC_INVINCIBLE, DC_AMMO, DC_RUNNING,
		DC_AIRBOARD, DC_BIRD, DC_LEVELSKIP, DC_BONUS, DC_KILL };
	const char *debugOptions[nOptions] = { "invincible", "unlimited ammo", "always running",
		"toggle airboard", "add bird", "skip level", "enter bonus", "kill jazz" };

	void toggleFlag() {
		if(debug::cheatFlags & debugFlags[chosen])
			debug::cheatFlags &= ~(debugFlags[chosen]); // clear
		else
			debug::cheatFlags |= debugFlags[chosen]; // set

		playConfirmSound();
	};

	public:
		explicit PageCheats(DebugMenu *owner) : DebugPage(owner) {}

		void handleUpDown(bool isUp) override {
			if (isUp)
				chosen = (chosen + nOptions - 1) % nOptions;
			else
				chosen = (chosen + 1) % nOptions;
		}

		void handleCursor(Point &position) override {
			position.x -= canvasW >> 2;
			position.y -= (canvasH >> 1) - (nOptions << 3);

			if ((position.x >= 0) && (position.x < 256) && (position.y >= 0) && (position.y < (nOptions << 4))) {
				chosen = position.y >> 4;

				if (controls.wasCursorReleased())
					toggleFlag();
			}
		}
		int handleEnter() override {
			toggleFlag();
			return E_NONE;
		}

		void draw() override {
			fontmn2->showString("CHEAT MENU", canvasW >> 1, (canvasH >> 1) - (nOptions << 3) - 32, alignX::Center);

			for (int i = 0; i < nOptions; i++) {

				if (i == chosen) fontmn2->mapPalette(240, 8, 114, 16);

				fontmn2->showString(debugOptions[i], (canvasW >> 1) - 100,
					(canvasH >> 1) + (i << 4) - (nOptions << 3));

				if(i < 3)
					fontmn2->showString(debug::cheatFlags & debugFlags[i] ? "on" : "off",
						(canvasW >> 1) + 60, (canvasH >> 1) + (i << 4) - (nOptions << 3));

				if (i == chosen) fontmn2->restorePalette();

			}
		}
};

namespace {
	std::unique_ptr<PageDisplay> pageDisplay;
	std::unique_ptr<PageCheats> pageCheats;

	const char *pageText[] = { "display", "cheats" };
}

DebugMenu::DebugMenu() :
	page(0), numPages(2), debugPage(nullptr) {

	pageDisplay = std::make_unique<PageDisplay>(this);
	pageCheats = std::make_unique<PageCheats>(this);
}

int DebugMenu::run(int newPage) {
	int x, y;

	showPage(newPage);

	video.setPalette(menuPalette);

	while (true) {
		if (loop(NORMAL_LOOP) == E_QUIT) return E_QUIT;
		if (controls.release(C_ESCAPE)) return E_RETURN;
		if (controls.release(C_UP)) debugPage->handleUpDown(true);
		if (controls.release(C_DOWN)) debugPage->handleUpDown(false);
		if (controls.release(C_LEFT)) debugPage->handleLeftRight(true);
		if (controls.release(C_RIGHT)) debugPage->handleLeftRight(false);

		if (controls.release(C_ENTER)) {
			int res = debugPage->handleEnter();
			if (res == E_RETURN)
				return res;
		}

		if (controls.getCursor(x, y)) {
			if ((x < 100) && (y >= canvasH - 12) && controls.wasCursorReleased()) return E_RETURN;

			Point pos(x, y);
			debugPage->handleCursor(pos);
		}

		SDL_Delay(T_MENU_FRAME);

		video.clearScreen(0);
		debugPage->draw();

		if (page > 0) {
			fontmn2->showString(pageText[page - 1], (canvasW >> 1) - 96, (canvasH >> 1) + 96, alignX::Left);
			fontmn2->showString("left:", (canvasW >> 1) - 110, (canvasH >> 1) + 96, alignX::Right);
		}
		if (page < numPages - 1) {
			fontmn2->showString("right:", (canvasW >> 1) + 96, (canvasH >> 1) + 96, alignX::Right);
			fontmn2->showString(pageText[page + 1], (canvasW >> 1) + 110, (canvasH >> 1) + 96, alignX::Left);
		}

		showEscString();
	}

	return E_NONE;
}

int DebugMenu::showPage(int num) {
	int savedPage = page;

	if(num != -1)
		page = CLAMP(num, 0, numPages - 1);

	switch(page) {
	case DP_SHOWFLAGS:
		debugPage = pageDisplay.get();
		break;
	case DP_CHEATS:
		debugPage = pageCheats.get();
		break;
	default:
		LOG_ERROR("Unknown debug page");
	}

	return savedPage;
}

#endif
