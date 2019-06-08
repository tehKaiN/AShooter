/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef _COMM_H_
#define _COMM_H_

#include <ace/types.h>

#define COMM_WIDTH (320-64)
#define COMM_HEIGHT (192)

#define COMM_DISPLAY_X 23
#define COMM_DISPLAY_Y 32
#define COMM_DISPLAY_WIDTH 168
#define COMM_DISPLAY_HEIGHT 120
#define COMM_DISPLAY_COLOR_BG 11
#define COMM_DISPLAY_COLOR_TEXT 14

typedef enum _tCommNav {
	COMM_NAV_UP,
	COMM_NAV_DOWN,
	COMM_NAV_LEFT,
	COMM_NAV_RIGHT,
	COMM_NAV_BTN,
	COMM_NAV_COUNT
} tCommNav;

void commInit(void);

void commDeinit(void);

void commProcess(void);

UBYTE commShow(void);

void commHide(void);

UBYTE commNavCheck(tCommNav eNav);

UBYTE commNavUse(tCommNav eNav);

tUwCoordYX commGetOrigin(void);

#endif // _COMM_H_
