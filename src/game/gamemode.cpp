
/**
 *
 * @file gamemode.cpp
 *
 * Part of the OpenJazz project
 *
 * @par History:
 * - 23rd August 2005: Created level.c and menu.c
 * - 3rd of February 2009: Renamed level.c to level.cpp and menu.c to menu.cpp
 * - 9th March 2009: Created game.cpp from parts of menu.cpp and level.cpp
 * - 18th July 2009: Created servergame.cpp from parts of game.cpp
 * - 2nd August 2009: Created gamemode.cpp from parts of servergame.cpp
 *
 * @par Licence:
 * Copyright (c) 2005-2017 Alister Thomson
 *
 * OpenJazz is distributed under the terms of
 * the GNU General Public License, version 2.0
 *
 */


#include "game.h"
#include "gamemode.h"

#include "io/gfx/font.h"
#include "player/player.h"


/**
 * Destroy game mode
 */
GameMode::~GameMode () {
}


/**
 * Outcome of player being hit
 *
 * @param source Player responsible for the hit
 * @param victim Player victim of the hit
 *
 * @return Whether or not the hit should result in energy loss (true)
 */
bool GameMode::hit (Player *source, Player *victim) {

	(void)source;
	(void)victim;

	return true;

}


/**
 * Outcome of player being killed
 *
 * @param game The current game
 * @param source Player responsible for the kill
 * @param victim Player victim of the kill
 *
 * @return Whether or not the player should be be killed (true)
 */
bool GameMode::kill (Game* game, Player *source, Player *victim) {

	if (source && (victim == localPlayer)) game->score(source->getTeam());

	return true;

}


/**
 * Outcome of level being completed
 *
 * @param game The current game
 * @param player Player that has completed level
 * @param gridX X-coordinate (in tiles) of finishing position
 * @param gridY Y-coordinate (in tiles) of finishing position
 *
 * @return Whether or not the level should end (true)
 */
bool GameMode::endOfLevel (Game* game, Player *player, int gridX, int gridY) {

	(void)player;

	game->setCheckpoint(gridX, gridY);

	return true;

}


/**
 * Outcome of time running out
 */
void GameMode::outOfTime () {

	return;

}


/**
 * Get the game mode type
 *
 * @return Game mode type (M_SINGLE)
 */
GameModeType SingleGameMode::getMode () {

	return M_SINGLE;

}


/**
 * Choose a team for a new player
 *
 * @return New player's team (0)
 */
unsigned char SingleGameMode::chooseTeam () {

	return 0;

}


/**
 * Draw the player's team's score (not in single-player mode)
 *
 * @param font Font to use to draw score
 */
void SingleGameMode::drawScore (Font* font) {

	(void)font;

	return;

}


/**
 * Choose a team for a new player
 *
 * @return New player's team (0)
 */
unsigned char CooperativeGameMode::chooseTeam () {

	// All players are on the same team

	return 0;

}


/**
 * Draw the player's team's score (not in cooperative mode)
 *
 * @param font Font to use to draw score
 */
void CooperativeGameMode::drawScore (Font* font) {

	(void)font;

	// Do nothing

	return;

}


/**
 * Choose a team for a new player
 *
 * @return New player's team (unique)
 */
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


/**
 * Draw the player's team's score
 *
 * @param font Font to use to draw score
 */
void FreeForAllGameMode::drawScore (Font* font) {

	font->showNumber(localPlayer->teamScore, 64, 4);

	return;

}


/**
 * Choose a team for a new player
 *
 * @return New player's team (0 or 1)
 */
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


/**
 * Draw the player's team's score
 *
 * @param font Font to use to draw score
 */
void TeamGameMode::drawScore (Font* font) {

	font->showNumber(localPlayer->teamScore, 64, 4);

	return;

}


/**
 * Get the game mode type
 *
 * @return Game mode type (M_COOP)
 */
GameModeType CoopGameMode::getMode () {

	return M_COOP;

}


/**
 * Get the game mode type
 *
 * @return Game mode type (M_BATTLE)
 */
GameModeType BattleGameMode::getMode () {

	return M_BATTLE;

}


/**
 * Get the game mode type
 *
 * @return Game mode type (M_TEAMBATTLE)
 */
GameModeType TeamBattleGameMode::getMode () {

	return M_TEAMBATTLE;

}


/**
 * Get the game mode type
 *
 * @return Game mode type (M_RACE)
 */
GameModeType RaceGameMode::getMode () {

	return M_RACE;

}


/**
 * Outcome of player being hit
 *
 * @param source Player responsible for the hit
 * @param victim Player victim of the hit
 *
 * @return Whether or not the hit should result in energy loss (false)
 */
bool RaceGameMode::hit (Player *source, Player *victim) {

	(void)source;
	(void)victim;

	return false;

}


/**
 * Outcome of level being completed
 *
 * @param game The current game
 * @param player Player that has completed level
 * @param gridX X-coordinate (in tiles) of finishing position
 * @param gridY Y-coordinate (in tiles) of finishing position
 *
 * @return Whether or not the level should end (false)
 */
bool RaceGameMode::endOfLevel (Game* game, Player *player, int gridX, int gridY) {

	(void)gridX;
	(void)gridY;

	if (player == localPlayer) game->score(localPlayer->getTeam());

	game->resetPlayer(player);

	return false;

}

