#include "ground_layer.h"
#include <ace/utils/extview.h>
#include <ace/utils/palette.h>

#define RGB(r,g,b) ((((r) >> 4) << 8) | (((g) >> 4) << 4) | (((b) >> 4) << 0))

#define LAYER_COLOR_START 27
#define LAYER_COLOR_COUNT (32 - LAYER_COLOR_START)

static tCopBlock *s_pColorsAbove;
static tCopBlock *s_pColorsBelow;
static UBYTE s_isCopperActive = 0, s_ubPrevLevel = 0;
static UWORD s_uwVpHeight, s_uwVpStartY;
static tCopList *s_pCopList;
tCopBlock *s_pDisableNext = 0;
static UBYTE s_ubLowerLayer = 1;

typedef struct _tGroundLayer {
	UWORD pColors[LAYER_COLOR_COUNT];
	UWORD uwTop;
	UBYTE ubDifficulty;
} tGroundLayer;

static const tGroundLayer s_pLayers[] = {
	{
		// Stock
		{
			RGB(51, 0, 0), RGB(102, 34, 0), RGB(153, 68, 17),
			RGB(204, 102, 34), RGB(255, 136, 51)
		},
		0, 0
	},
	{
		// A
		{
			RGB(51, 34, 0), RGB(102, 68, 0), RGB(153, 102, 17),
			RGB(204, 136, 34), RGB(255, 170, 51)
		},
		2048 + 32, 0
	},
	{
		// B
		{
			RGB(51, 34, 17), RGB(102, 68, 34), RGB(153, 102, 51),
			RGB(204, 136, 68), RGB(255, 170, 85)
		},
		4096 + 32, 0
	},
	{
		// C
		{
			RGB(17, 34, 17), RGB(68, 68, 34), RGB(119, 102, 51),
			RGB(170, 136, 68), RGB(221, 170, 85)
		},
		6144 + 32, 0
	},
	{
		// D
		{
			RGB(17, 34, 34), RGB(68, 68, 51), RGB(119, 102, 68),
			RGB(170, 136, 85), RGB(221, 170, 102)
		},
		8192 + 32, 0
	}
};

static const UBYTE s_ubLayerCount = sizeof(s_pLayers) / sizeof(s_pLayers[0]);

static void groundLayerSetColorRegs(
	const tGroundLayer *pLayer, UBYTE ubColorLevel
) {
	volatile UWORD *pColorRegs = &g_pCustom->color[LAYER_COLOR_START];
	for(UBYTE i = 0; i < LAYER_COLOR_COUNT; ++i) {
		pColorRegs[i] = paletteColorDim(pLayer->pColors[i], ubColorLevel);
	}
}

void groundLayerCreate(const tVPort *pVp) {
	logBlockBegin("groundLayerCreate(pVp: %p)", pVp);
	tView *pView = pVp->pView;
	s_pCopList = pView->pCopList;
	s_uwVpStartY = pVp->uwOffsY + 0x2C;
	s_uwVpHeight = pVp->uwHeight;
	s_isCopperActive = 0;
	s_pColorsBelow = copBlockCreate(pView->pCopList, s_ubLayerCount, 0, 0);
	s_pColorsAbove = copBlockCreate(pView->pCopList, s_ubLayerCount, 0, 0);
	groundLayerReset(1);
	logBlockEnd("groundLayerCreate()");
}

void groundLayerReset(UBYTE ubLowerLayer) {
	s_pColorsBelow->ubDisabled = 1;
	s_ubLowerLayer = ubLowerLayer;
	const tGroundLayer *pLayerCurrent = &s_pLayers[ubLowerLayer - 1];
	volatile UWORD *pColorRegs = &g_pCustom->color[LAYER_COLOR_START];
	s_ubPrevLevel = 0xF;
	groundLayerSetColorRegs(pLayerCurrent, s_ubPrevLevel);
}

static void layerCopyColorsToBlock(
	const tGroundLayer *pLayer, tCopBlock *pBlock, UBYTE ubColorLevel
) {
	volatile UWORD *pColorRegs = &g_pCustom->color[LAYER_COLOR_START];
	pBlock->uwCurrCount = 0;
	for(UBYTE i = 0; i < LAYER_COLOR_COUNT; ++i) {
		copMove(
			s_pCopList, pBlock, &pColorRegs[i],
			paletteColorDim(pLayer->pColors[i], ubColorLevel)
		);
	}
}

void groundLayerProcess(UWORD uwCameraY, UBYTE ubColorLevel) {
	if(uwCameraY < s_pLayers[s_ubLowerLayer-1].uwTop) {
		--s_ubLowerLayer;
	}
	else if(
		uwCameraY + s_uwVpHeight >= s_pLayers[s_ubLowerLayer+1].uwTop &&
		s_ubLowerLayer < s_ubLayerCount - 1
	) {
		++s_ubLowerLayer;
	}

	// Transition between layers on screen
	const tGroundLayer *pLayerLower = &s_pLayers[s_ubLowerLayer];
	const tGroundLayer *pLayerUpper = &s_pLayers[s_ubLowerLayer - 1];
	const UWORD uwSeamPos = pLayerLower->uwTop;
	if(uwCameraY < uwSeamPos && uwSeamPos < uwCameraY + s_uwVpHeight) {
		if(!s_isCopperActive) {
			copBlockEnable(s_pCopList, s_pColorsBelow);
			layerCopyColorsToBlock(pLayerLower, s_pColorsBelow, ubColorLevel);

			copBlockEnable(s_pCopList, s_pColorsAbove);
			layerCopyColorsToBlock(pLayerUpper, s_pColorsAbove, ubColorLevel);
			copBlockWait(
				s_pCopList, s_pColorsAbove, 0xE2, s_uwVpStartY + s_uwVpHeight - 1
			);
			s_isCopperActive = 1;
		}
		else if(s_ubPrevLevel != ubColorLevel) {
			layerCopyColorsToBlock(pLayerLower, s_pColorsBelow, ubColorLevel);
			layerCopyColorsToBlock(pLayerUpper, s_pColorsAbove, ubColorLevel);
		}
		copBlockWait(
			s_pCopList, s_pColorsBelow, 0, s_uwVpStartY + uwSeamPos - uwCameraY
		);
	}
	else {
		if(s_isCopperActive) {
			if(s_pDisableNext) {
				// Disable in next frame so that color regs will have colors of this layer
				copBlockDisable(s_pCopList, s_pDisableNext);
				s_pDisableNext = 0;
				s_isCopperActive = 0;
			}
			else {
				if(uwCameraY >= uwSeamPos) {
					// Going down - disable copBlock of upper layer
					copBlockDisable(s_pCopList, s_pColorsAbove);
					s_pDisableNext = s_pColorsBelow;
				}
				else {
					// Going uo - disable copBlock of lower layer
					copBlockDisable(s_pCopList, s_pColorsBelow);
					s_pDisableNext = s_pColorsAbove;
				}
			}
		}
		else if(s_ubPrevLevel != ubColorLevel) {
			groundLayerSetColorRegs(pLayerUpper, ubColorLevel);
		}
	}
	s_ubPrevLevel = ubColorLevel;
}
