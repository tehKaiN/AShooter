/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef _TILE_H_
#define _TILE_H_

#include <ace/types.h>
#include "vehicle.h"

#define TILE_ROW_BASE_DIRT 8
#define TILE_ROW_CHALLENGE_CHECKPOINT_1 (TILE_ROW_BASE_DIRT + 15)
#define TILE_ROW_CHALLENGE_CHECKPOINT_2 (TILE_ROW_CHALLENGE_CHECKPOINT_1 + 15)
#define TILE_ROW_CHALLENGE_CHECKPOINT_3 (TILE_ROW_CHALLENGE_CHECKPOINT_2 + 20)
#define TILE_ROW_CHALLENGE_FINISH (TILE_ROW_CHALLENGE_CHECKPOINT_3 + 15)

typedef enum _tTile {
	TILE_BASE_BG_FIRST = 0,
	TILE_BASE_BG_LAST = 32,
	TILE_BASE_SHAFT,
	TILE_BASE_GROUND_1, TILE_BASE_GROUND_2, TILE_BASE_GROUND_3, TILE_BASE_GROUND_4,
	TILE_BASE_GROUND_5, TILE_BASE_GROUND_6, TILE_BASE_GROUND_7, TILE_BASE_GROUND_8,
	TILE_BASE_GROUND_9,
	TILE_CAVE_BG,
	TILE_STONE_1 = TILE_CAVE_BG + 16,
	TILE_STONE_2, TILE_STONE_3, TILE_STONE_4,
	TILE_ROCK_1, TILE_ROCK_2,
	TILE_GOLD_1, TILE_GOLD_2, TILE_GOLD_3,
	TILE_SILVER_1, TILE_SILVER_2, TILE_SILVER_3,
	TILE_EMERALD_1, TILE_EMERALD_2, TILE_EMERALD_3,
	TILE_RUBY_1, TILE_RUBY_2, TILE_RUBY_3,
	TILE_MOONSTONE_1, TILE_MOONSTONE_2, TILE_MOONSTONE_3,
	TILE_COAL_1, TILE_COAL_2, TILE_COAL_3,
	TILE_CHECKPOINT_1,
	TILE_BONE_HEAD = TILE_CHECKPOINT_1 + 10, TILE_BONE_1,
	TILE_MAGMA_1, TILE_MAGMA_2,
	TILE_COUNT
} tTile;
// TODO sapphire, topaz

typedef struct _tTileDef {
	UBYTE ubMineral;
	UBYTE ubSlots;
	const char *szMsg;
} tTileDef;

UBYTE tileIsSolid(UWORD uwX, UWORD uwY);

UBYTE tileIsDrillable(UWORD uwX, UWORD uwY);

UBYTE tileIsExplodable(UWORD uwX, UWORD uwY);

void tileInit(UBYTE isCoalOnly, UBYTE isChallenge);

void tileExcavate(UWORD uwX, UWORD uwY);

extern const tTileDef const g_pTileDefs[TILE_COUNT];

#endif // _TILE_H_
