
/*
 *
 * gamemode.cpp
 *
 * 2nd August 2009: Created gamemode.cpp from parts of servergame.cpp
 *
 * Part of the OpenJazz project
 *
 *
 * Copyright (c) 2005-2010 Alister Thomson
 *
 * OpenJazz is distributed under the terms of
 * the GNU General Public License, version 2.0
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */


#include "game.h"
#include "gamemode.h"

#include "io/gfx/font.h"
#include "level/level.h"
#include "player/player.h"


bool GameMode::hit (Player *source, Player *victim) {

	return true;

}


bool GameMode::kill (Player *source, Player *victim) {

	if (source && (victim == localPlayer)) game->score(source->getTeam());

	return true;

}


bool GameMode::endOfLevel (Player *player, unsigned char gridX,
	unsigned char gridY) {

	game->setCheckpoint(gridX, gridY);

	level->setStage(LS_END);

	return true;

}


void GameMode::outOfTime () {

	return;

}


unsigned char CooperativeGameMode::chooseTeam () {

	// All players are on the same team

	return 0;

}


void CooperativeGameMode::drawScore () {

	// Do nothing

	return;

}


unsigned char FreeForAllGameMode::chooseTeam () {

	// Every player is on a separate team

	int count;
	unsigned char team;

	team = 1;

	// Find a team number higher than any other
	for (count = nPlayers - 1; count >= 0; count--) {

		if (players[count].getTeam() > team)
			team = players[count].getTeam() + 1;

	}

	return team;

}


void FreeForAllGameMode::drawScore () {

	fontsFont->showNumber(localPlayer->teamScore, 64, 4);

	return;

}


unsigned char TeamGameMode::chooseTeam () {

	// Players are split between two teams

	int count, difference;

	// Calculate team imbalance

	difference = 0;

	for (count = 0; count < nPlayers; count++) {

		if (players[count].getTeam()) difference++;
		else difference--;

	}

	// Assign to the team with the least players

	if (difference >= 0) return 0;

	return 1;

}


void TeamGameMode::drawScore () {

	fontsFont->showNumber(localPlayer->teamScore, 64, 4);

	return;

}


GameModeType CoopGameMode::getMode () {

	return M_COOP;

}


bool CoopGameMode::endOfLevel (Player *player, unsigned char gridX,
	unsigned char gridY) {

	game->setCheckpoint(gridX, gridY);

	level->setStage(LS_END);

	return true;

}


GameModeType BattleGameMode::getMode () {

	return M_BATTLE;

}


GameModeType TeamBattleGameMode::getMode () {

	return M_TEAMBATTLE;

}


GameModeType RaceGameMode::getMode () {

	return M_RACE;

}


bool RaceGameMode::hit (Player *source, Player *victim) {

	return false;

}


bool RaceGameMode::endOfLevel (Player *player, unsigned char gridX,
	unsigned char gridY) {

	if (player == localPlayer) game->score(player->getTeam());

	game->resetPlayer(player, false);

	return false;

}


GameMode * createGameMode (GameModeType mode) {

	switch (mode) {

		case M_SINGLE:

			return NULL;

		case M_COOP:

			return new CoopGameMode();

		case M_BATTLE:

			return new BattleGameMode();

		case M_TEAMBATTLE:

			return new TeamBattleGameMode();

		case M_RACE:

			return new RaceGameMode();

	}

	return NULL;

}


