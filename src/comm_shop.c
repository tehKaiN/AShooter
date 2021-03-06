/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "comm_shop.h"
#include <ace/managers/key.h>
#include <ace/managers/game.h>
#include "comm.h"
#include "core.h"
#include "game.h"
#include "warehouse.h"
#include "mineral.h"
#include "button.h"
#include "vehicle.h"
#include "hud.h"
#include "tutorial.h"

static UBYTE s_isShown;
static UBYTE s_isBtnPress = 0;
tBitMap *s_pBmDraw;
tCommLed s_eTab;

//----------------------------------------------------------------------- OFFICE

static void commShopDrawOffice(void) {

}

//--------------------------------------------------------------------- WORKSHOP

static void commShopDrawWorkshop(void) {

}

//-------------------------------------------------------------------- WAREHOUSE

static UBYTE s_ubPosCurr = 0, s_ubPosCount = 0;
static const char *s_pColNames[4] = {"Mineral", "Sell", "Stock", "Plan"};
static const UBYTE s_pColOffs[4] = {0,  50, 85, 130};
static UBYTE s_pMineralsOnList[MINERAL_TYPE_COUNT];

static UWORD s_pTmpSell[MINERAL_TYPE_COUNT];
static UWORD s_pTmpPlan[MINERAL_TYPE_COUNT];
static UWORD s_pTmpStock[MINERAL_TYPE_COUNT];

static UBYTE getMineralsOnList(const tPlan *pPlan, UBYTE *pMineralsOnList) {
	UBYTE ubCount = 0;
	for(UBYTE i = 0; i < MINERAL_TYPE_COUNT; ++i) {
		// Omit minerals not in plan
		if(pPlan->pMinerals[i].uwTargetCount || warehouseGetStock(i)) {
			pMineralsOnList[ubCount] = i;
			s_pTmpStock[i] = warehouseGetStock(i);
			s_pTmpPlan[i] = pPlan->pMinerals[i].uwCurrentCount;
			s_pTmpSell[i] = 0;
			++ubCount;
		}
	}
	return ubCount;
}

static void commShopDrawWarehouseRow(UBYTE ubPos, const tPlan *pPlan) {
	UBYTE ubMineral = s_pMineralsOnList[ubPos];
	UBYTE ubColor = (
		ubPos == s_ubPosCurr ?
		COMM_DISPLAY_COLOR_TEXT : COMM_DISPLAY_COLOR_TEXT_DARK
	);

	UWORD uwRowOffsY = 11 + ubPos * 10;

	// Erase
	commErase(0, uwRowOffsY, COMM_DISPLAY_WIDTH, 10);

	// Name
	commDrawText(
		s_pColOffs[0], uwRowOffsY, g_pMinerals[ubMineral].szName,
		FONT_COOKIE, ubColor
	);

	// Sell
	char szBfr[10];
	UWORD uwMineralReward = s_pTmpSell[ubMineral] * g_pMinerals[ubMineral].ubReward;
	sprintf(szBfr, "%hu\x1F", uwMineralReward);
	commDrawText(s_pColOffs[1], uwRowOffsY, szBfr, FONT_COOKIE, ubColor);

	// Stock
	UBYTE ubStockCenter = fontMeasureText(g_pFont, s_pColNames[2]).uwX / 2;
	sprintf(szBfr, "%hu", s_pTmpStock[ubMineral]);
	UBYTE ubValWidthHalf = fontMeasureText(g_pFont, szBfr).uwX / 2;

	if(ubPos == s_ubPosCurr) {
		commDrawText(
			s_pColOffs[2] + ubStockCenter + ubValWidthHalf + 3, uwRowOffsY, ">",
			FONT_COOKIE | FONT_LEFT, ubColor
		);
		commDrawText(
			s_pColOffs[2] + ubStockCenter - ubValWidthHalf - 3, uwRowOffsY, "<",
			FONT_COOKIE | FONT_RIGHT, ubColor
		);
	}
	else {
		commErase(
			s_pColOffs[2] + ubStockCenter + ubValWidthHalf + 3, uwRowOffsY,
			5, g_pFont->uwHeight
		);
		commErase(
			s_pColOffs[2] + ubStockCenter + ubValWidthHalf - 3 - 5, uwRowOffsY,
			5, g_pFont->uwHeight
		);
	}
	commDrawText(
		s_pColOffs[2] + ubStockCenter - ubValWidthHalf, uwRowOffsY, szBfr,
		FONT_COOKIE, ubColor
	);

	// Plan
	sprintf(
		szBfr, "%hu/%hu",
		s_pTmpPlan[ubMineral], pPlan->pMinerals[ubMineral].uwTargetCount
	);
	commDrawText(s_pColOffs[3], uwRowOffsY, szBfr, FONT_COOKIE, ubColor);
}

static void commShopDrawWarehouse(void) {
	for(UBYTE ubCol = 0; ubCol < 4; ++ubCol) {
		commDrawText(
			s_pColOffs[ubCol], 0, s_pColNames[ubCol],
			FONT_COOKIE, COMM_DISPLAY_COLOR_TEXT
		);
	}

	const tUwCoordYX sPosDisplay = commGetOriginDisplay();
	const UBYTE ubLineHeight = g_pFont->uwHeight + 1;
	blitRect(
		s_pBmDraw, sPosDisplay.uwX + s_pColOffs[0], sPosDisplay.uwY + ubLineHeight,
		COMM_DISPLAY_WIDTH, 1, COMM_DISPLAY_COLOR_TEXT
	);

	const tPlan *pPlan = warehouseGetPlan();
	s_ubPosCount = getMineralsOnList(pPlan, s_pMineralsOnList);
	s_ubPosCurr = s_ubPosCount; // move to buttons on start
	for(UBYTE i = 0; i < s_ubPosCount; ++i) {
		commShopDrawWarehouseRow(i, pPlan);
	}

	// Confirm button
	UWORD uwBtnX = COMM_DISPLAY_WIDTH / 3;
	UWORD uwBtnY = COMM_DISPLAY_HEIGHT - 5 * ubLineHeight;
	buttonRmAll();
	buttonAdd("Confirm", uwBtnX, uwBtnY);
	buttonAdd("Exit", uwBtnX * 2, uwBtnY);
	buttonSelect(0);
	buttonDrawAll(s_pBmDraw);

	commDrawText(
		COMM_DISPLAY_WIDTH - 25, COMM_DISPLAY_HEIGHT - 2 * ubLineHeight,
		"Time remaining:", FONT_COOKIE | FONT_RIGHT, COMM_DISPLAY_COLOR_TEXT
	);
	char szBfr[5];
	sprintf(szBfr, "%d", (pPlan->wTimeRemaining + 9) / 10);
	commDrawText(
		COMM_DISPLAY_WIDTH - 25 + 5, COMM_DISPLAY_HEIGHT - 2 * ubLineHeight, szBfr,
		FONT_COOKIE, COMM_DISPLAY_COLOR_TEXT
	);
}

static void commShopProcessWarehouse() {
	UBYTE isButtonRefresh = 0;
	UBYTE ubPosPrev = s_ubPosCurr;
	if(commNavUse(COMM_NAV_UP) && s_ubPosCurr) {
		 s_ubPosCurr = MAX(0, s_ubPosCurr - 1);
	}
	else if(commNavUse(COMM_NAV_DOWN) && s_ubPosCurr < s_ubPosCount) {
		s_ubPosCurr = MIN(s_ubPosCount, s_ubPosCurr + 1);
	}

	if(s_ubPosCurr != ubPosPrev) {
		// Deselect previous pos
		if(ubPosPrev < s_ubPosCount) {
			commShopDrawWarehouseRow(ubPosPrev, warehouseGetPlan());
		}
		// Select new pos
		if(s_ubPosCurr < s_ubPosCount) {
			commShopDrawWarehouseRow(s_ubPosCurr, warehouseGetPlan());
			if(ubPosPrev >= s_ubPosCount) {
				buttonSelect(BUTTON_INVALID);
				isButtonRefresh = 1;
			}
		}
		else {
			buttonSelect(0);
			isButtonRefresh = 1;
		}
	}
	else if(s_ubPosCurr < s_ubPosCount) {
		UBYTE ubMineral = s_pMineralsOnList[s_ubPosCurr];
		// Process moving stock
		if(commNavUse(COMM_NAV_LEFT) && s_pTmpStock[ubMineral]) {
			++s_pTmpSell[ubMineral];
			--s_pTmpStock[ubMineral];
			commShopDrawWarehouseRow(ubPosPrev, warehouseGetPlan());
		}
		else if(commNavUse(COMM_NAV_RIGHT) && s_pTmpStock[ubMineral]) {
			++s_pTmpPlan[ubMineral];
			--s_pTmpStock[ubMineral];
			commShopDrawWarehouseRow(ubPosPrev, warehouseGetPlan());
		}
	}
	else {
		// Navigation between buttons
		if(commNavUse(COMM_NAV_RIGHT)) {
			buttonSelect(1);
			isButtonRefresh = 1;
		}
		else if(commNavUse(COMM_NAV_LEFT)) {
			buttonSelect(0);
			isButtonRefresh = 1;
		}
		else if(ubPosPrev < s_ubPosCount) {
			buttonSelect(0);
			isButtonRefresh = 1;
		}
	}
	if(isButtonRefresh) {
		buttonDrawAll(s_pBmDraw);
	}

	// Process button press
	if(s_isBtnPress) {
		switch(buttonGetSelected()) {
			case 0:
				// Confirm
				for(UBYTE i = 0; i < s_ubPosCount; ++i) {
					UBYTE ubMineral = s_pMineralsOnList[i];
					warehouseSetStock(ubMineral, s_pTmpStock[ubMineral]);
					warehouseReserveMineralsForPlan(ubMineral, s_pTmpPlan[ubMineral]);
					g_pVehicles[0].lCash += g_pMinerals[ubMineral].ubReward * s_pTmpSell[ubMineral];
					s_pTmpSell[ubMineral] = 0;
					s_pTmpPlan[ubMineral] = 0;
					s_pTmpStock[ubMineral] = 0;
					hudSetCash(0, g_pVehicles[0].lCash);
				}
				if(warehouseIsPlanFulfilled()) {
					warehouseNewPlan(1, g_is2pPlaying);
				}
				commEraseAll();
				commShopDrawWarehouse();
				break;
			case 1:
				// Exit
				gamePopState();
				return;
			default:
				break;
		};
	}
}

static void commShopShowTab(tCommLed eTab) {
	s_eTab = eTab;
	commSetActiveLed(eTab);
	commEraseAll();
	switch(eTab) {
		case COMM_LED_OFFICE:
			commShopDrawOffice();
			break;
		case COMM_LED_WORKSHOP:
			commShopDrawWorkshop();
			break;
		case COMM_LED_WAREHOUSE:
			commShopDrawWarehouse();
			break;
		default:
		break;
	}
}

void commShopGsCreate(void) {
	s_isBtnPress = 0;
	s_isShown = commShow();
	if(!s_isShown) {
		// Camera not placed properly
		gamePopState();
		return;
	}

	s_pBmDraw = g_pMainBuffer->pScroll->pBack;

	s_eTab = COMM_LED_WAREHOUSE;
	commShopShowTab(s_eTab);

	// Process managers once so that backbuffer becomes front buffer
	// Single buffering from now!
	viewProcessManagers(g_pMainBuffer->sCommon.pVPort->pView);
	copProcessBlocks();
	vPortWaitForEnd(g_pMainBuffer->sCommon.pVPort);
}

void commShopGsLoop(void) {
	static UBYTE isShift = 0;
	static UBYTE wasShiftAction = 0;
	commProcess();

	tutorialProcess();

	tCommLed eOldTab = s_eTab;
	s_isBtnPress = 0;
	if(commNavCheck(COMM_NAV_BTN)) {
		isShift = 1;
	}
	else if(isShift) {
		if(!wasShiftAction) {
			// Btn released and no other pressed in the meantime
			s_isBtnPress = 1;
		}
		isShift = 0;
		wasShiftAction = 0;
	}

	// Tab nav using shift+left / shift+right
	if(isShift) {
		if(commNavUse(COMM_NAV_LEFT)) {
			if(s_eTab) {
				--s_eTab;
			}
			else {
				s_eTab = COMM_LED_COUNT - 1;
			}
			wasShiftAction = 1;
		}
		else if(commNavUse(COMM_NAV_RIGHT)) {
			if(s_eTab < COMM_LED_COUNT - 1) {
				++s_eTab;
			}
			else {
				s_eTab = 0;
			}
			wasShiftAction = 1;
		}
	}

	hudUpdate();
	// Process only managers of HUD because we want single buffering on main one
	vPortProcessManagers(g_pMainBuffer->sCommon.pVPort->pView->pFirstVPort);
	copProcessBlocks();
	vPortWaitForEnd(g_pMainBuffer->sCommon.pVPort);

	if(s_eTab != eOldTab) {
		commShopShowTab(s_eTab);
	}
	else {
		switch(s_eTab) {
			case COMM_LED_OFFICE:
				break;
			case COMM_LED_WORKSHOP:
				break;
			case COMM_LED_WAREHOUSE:
				commShopProcessWarehouse();
				break;
			default:
				break;
		}
	}
}

void commShopGsDestroy(void) {
	if(!s_isShown) {
		return;
	}
	viewProcessManagers(g_pMainBuffer->sCommon.pVPort->pView);
	copProcessBlocks();
	vPortWaitForEnd(g_pMainBuffer->sCommon.pVPort);
	commHide();
}

UBYTE commShopIsActive(void) {
	return s_isShown;
}
