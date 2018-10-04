/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef _HUD_H_
#define _HUD_H_

#include <ace/utils/extview.h>
#include <ace/utils/font.h>

void hudCreate(tView *pView, tFont *pFont);

void hudDestroy(void);

void hudSetDepth(UWORD uwDepth);

void hudSetScore(ULONG ulCash);

void hudSetCargo(UBYTE ubCargo);

void hudSetFuel(UWORD uwFuel);

void hudUpdate(void);

#endif // _HUD_H_
