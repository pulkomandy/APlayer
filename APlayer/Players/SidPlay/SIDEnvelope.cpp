/******************************************************************************/
/* SIDEnvelope implementation file.                                           */
/******************************************************************************/
/* This source, or parts thereof, may be used in any software as long the     */
/* license of APlayer is keep. See the LICENSE file for more information.     */
/*                                                                            */
/* Copyright (C) 1998-2002 by The APlayer-Team.                               */
/* All rights reserved.                                                       */
/******************************************************************************/


// PolyKit headers
#include "POS.h"

// Player headers
#include "SIDOperator.h"
#include "SIDEnvelope.h"


/******************************************************************************/
/* Hardware:                                                                  */
/* C64 C PAL, MOS 6581R4 1686 S                                               */
/******************************************************************************/
static uint8 releaseTab[] =
{
	255, 255, 254, 254, 253, 253, 252, 252, 251, 251, 250, 250, 249, 249, 248,
	248, 247, 247, 246, 246, 245, 245, 244, 244, 243, 243, 242, 242, 241, 241,
	240, 240, 239, 239, 238, 238, 237, 237, 236, 236, 235, 235, 234, 234, 233,
	233, 232, 232, 231, 231, 230, 230, 229, 229, 228, 228, 227, 227, 226, 226,
	225, 225, 224, 224, 223, 223, 222, 222, 221, 221, 220, 220, 219, 219, 218,
	218, 217, 217, 216, 216, 215, 215, 214, 214, 213, 213, 212, 212, 211, 211,
	210, 210, 209, 209, 208, 208, 207, 207, 206, 206, 205, 205, 204, 204, 203,
	203, 202, 202, 201, 201, 200, 200, 199, 199, 198, 198, 197, 197, 196, 196,
	195, 195, 194, 194, 193, 193, 192, 192, 191, 191, 190, 190, 189, 189, 188,
	188, 187, 187, 186, 186, 185, 185, 184, 184, 183, 183, 182, 182, 181, 181,
	180, 180, 179, 179, 178, 178, 177, 177, 176, 176, 175, 175, 174, 174, 173,
	173, 172, 172, 171, 171, 170, 170, 169, 169, 168, 168, 167, 167, 166, 166,
	165, 165, 164, 164, 163, 163, 162, 162, 161, 161, 160, 160, 159, 159, 158,
	158, 157, 157, 156, 156, 155, 155, 154, 154, 153, 153, 152, 152, 151, 151,
	150, 150, 149, 149, 148, 148, 147, 147, 146, 146, 145, 145, 144, 144, 143,
	143, 142, 142, 141, 141, 140, 140, 139, 139, 138, 138, 137, 137, 136, 136,
	135, 135, 134, 134, 133, 133, 132, 132, 131, 131, 130, 130, 129, 129, 128,
	128, 127, 127, 126, 126, 125, 125, 124, 124, 123, 123, 122, 122, 121, 121,
	120, 120, 119, 119, 118, 118, 117, 117, 116, 116, 115, 115, 114, 114, 113,
	113, 112, 112, 111, 111, 110, 110, 109, 109, 108, 108, 107, 107, 106, 106,
	105, 105, 104, 104, 103, 103, 102, 102, 101, 101, 100, 100, 99, 99, 98,
	98, 97, 97, 96, 96, 95, 95, 94, 94, 94, 94, 93, 93, 93, 93,
	92, 92, 92, 92, 91, 91, 91, 91, 90, 90, 90, 90, 89, 89, 89,
	89, 88, 88, 88, 88, 87, 87, 87, 87, 86, 86, 86, 86, 85, 85,
	85, 85, 84, 84, 84, 84, 83, 83, 83, 83, 82, 82, 82, 82, 81,
	81, 81, 81, 80, 80, 80, 80, 79, 79, 79, 79, 78, 78, 78, 78,
	77, 77, 77, 77, 76, 76, 76, 76, 75, 75, 75, 75, 74, 74, 74,
	74, 73, 73, 73, 73, 72, 72, 72, 72, 71, 71, 71, 71, 70, 70,
	70, 70, 69, 69, 69, 69, 68, 68, 68, 68, 67, 67, 67, 67, 66,
	66, 66, 66, 65, 65, 65, 65, 64, 64, 64, 64, 63, 63, 63, 63,
	62, 62, 62, 62, 61, 61, 61, 61, 60, 60, 60, 60, 59, 59, 59,
	59, 58, 58, 58, 58, 57, 57, 57, 57, 56, 56, 56, 56, 55, 55,
	55, 55, 55, 55, 55, 55, 54, 54, 54, 54, 54, 54, 54, 54, 53,
	53, 53, 53, 53, 53, 53, 53, 52, 52, 52, 52, 52, 52, 52, 52,
	51, 51, 51, 51, 51, 51, 51, 51, 50, 50, 50, 50, 50, 50, 50,
	50, 49, 49, 49, 49, 49, 49, 49, 49, 48, 48, 48, 48, 48, 48,
	48, 48, 47, 47, 47, 47, 47, 47, 47, 47, 46, 46, 46, 46, 46,
	46, 46, 46, 45, 45, 45, 45, 45, 45, 45, 45, 44, 44, 44, 44,
	44, 44, 44, 44, 43, 43, 43, 43, 43, 43, 43, 43, 42, 42, 42,
	42, 42, 42, 42, 42, 41, 41, 41, 41, 41, 41, 41, 41, 40, 40,
	40, 40, 40, 40, 40, 40, 39, 39, 39, 39, 39, 39, 39, 39, 38,
	38, 38, 38, 38, 38, 38, 38, 37, 37, 37, 37, 37, 37, 37, 37,
	36, 36, 36, 36, 36, 36, 36, 36, 35, 35, 35, 35, 35, 35, 35,
	35, 34, 34, 34, 34, 34, 34, 34, 34, 33, 33, 33, 33, 33, 33,
	33, 33, 32, 32, 32, 32, 32, 32, 32, 32, 31, 31, 31, 31, 31,
	31, 31, 31, 30, 30, 30, 30, 30, 30, 30, 30, 29, 29, 29, 29,
	29, 29, 29, 29, 28, 28, 28, 28, 28, 28, 28, 28, 27, 27, 27,
	27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 26, 26,
	26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 25,
	25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25,
	24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24,
	24, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23,
	23, 23, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22,
	22, 22, 22, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21,
	21, 21, 21, 21, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20,
	20, 20, 20, 20, 20, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19,
	19, 19, 19, 19, 19, 19, 18, 18, 18, 18, 18, 18, 18, 18, 18,
	18, 18, 18, 18, 18, 18, 18, 17, 17, 17, 17, 17, 17, 17, 17,
	17, 17, 17, 17, 17, 17, 17, 17, 16, 16, 16, 16, 16, 16, 16,
	16, 16, 16, 16, 16, 16, 16, 16, 16, 15, 15, 15, 15, 15, 15,
	15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15,
	15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 14, 14, 14, 14,
	14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14,
	14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 13, 13,
	13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13,
	13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	12, 12, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11,
	11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11,
	11, 11, 11, 11, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10,
	10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10,
	10, 10, 10, 10, 10, 10, 9, 9, 9, 9, 9, 9, 9, 9, 9,
	9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9,
	9, 9, 9, 9, 9, 9, 9, 9, 8, 8, 8, 8, 8, 8, 8,
	8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
	8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 7, 7, 7, 7, 7,
	7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
	7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
	7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
	7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 6, 6, 6, 6, 6,
	6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6,
	6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6,
	6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6,
	6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 5, 5, 5, 5, 5,
	5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5,
	5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5,
	5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5,
	5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 4, 4, 4, 4, 4,
	4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4,
	4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4,
	4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4,
	4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 3, 3, 3, 3, 3,
	3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,
	3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,
	3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,
	3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 2, 2, 2, 2, 2,
	2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
	2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
	2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
	2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 1, 1, 1, 1, 1,
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0
};



/******************************************************************************/
/* Other tables                                                               */
/******************************************************************************/
extern const uint8 masterVolumeLevels[16] =
{
	0,  17,  34,  51,  68,  85, 102, 119,
	136, 153, 170, 187, 204, 221, 238, 255
};


static const float attackTimes[16] =
{
	// Milliseconds
	2.2528606f, 8.0099577f, 15.7696042f, 23.7795619f, 37.2963655f, 55.0684591f,
	66.8330845f, 78.3473987f,
	98.1219818f, 244.554021f, 489.108042f, 782.472742f, 977.715461f, 2933.64701f,
	4889.07793f, 7822.72493f
};


static const float decayReleaseTimes[16] =
{
	// Milliseconds
	8.91777693f, 24.594051f, 48.4185907f, 73.0116639f, 114.512475f, 169.078356f,
	205.199432f, 240.551975f,
	301.266125f, 750.858245f, 1501.71551f, 2402.43682f, 3001.89298f, 9007.21405f,
	15010.998f, 24018.2111f
};


#define AttackTabLen			255



/******************************************************************************/
/* Function table                                                             */
/******************************************************************************/
extern const SidUWordFunc enveModeTable[] =
{
	// 0
	SIDEnvelope::EnveEmuStartAttack, SIDEnvelope::EnveEmuStartRelease,
	SIDEnvelope::EnveEmuAttack, SIDEnvelope::EnveEmuDecay, SIDEnvelope::EnveEmuSustain, SIDEnvelope::EnveEmuRelease,
	SIDEnvelope::EnveEmuSustainDecay, SIDEnvelope::EnveEmuMute,
	// 16
	SIDEnvelope::EnveEmuStartShortAttack,
	SIDEnvelope::EnveEmuMute, SIDEnvelope::EnveEmuMute, SIDEnvelope::EnveEmuMute, 
	SIDEnvelope::EnveEmuMute, SIDEnvelope::EnveEmuMute, SIDEnvelope::EnveEmuMute, SIDEnvelope::EnveEmuMute,
	// 32
	SIDEnvelope::EnveEmuStartAttack, SIDEnvelope::EnveEmuStartRelease,
	SIDEnvelope::EnveEmuAlterAttack, SIDEnvelope::EnveEmuAlterDecay, SIDEnvelope::EnveEmuAlterSustain, SIDEnvelope::EnveEmuAlterRelease,
	SIDEnvelope::EnveEmuAlterSustainDecay, SIDEnvelope::EnveEmuMute,
	// 48
	SIDEnvelope::EnveEmuStartShortAttack,
	SIDEnvelope::EnveEmuMute, SIDEnvelope::EnveEmuMute, SIDEnvelope::EnveEmuMute,
	SIDEnvelope::EnveEmuMute, SIDEnvelope::EnveEmuMute, SIDEnvelope::EnveEmuMute, SIDEnvelope::EnveEmuMute
};



/******************************************************************************/
/* Constructor                                                                */
/******************************************************************************/
SIDEnvelope::SIDEnvelope(void)
{
}



/******************************************************************************/
/* Destructor                                                                 */
/******************************************************************************/
SIDEnvelope::~SIDEnvelope(void)
{
}



/******************************************************************************/
/* EnveEmuInit()                                                              */
/******************************************************************************/
void SIDEnvelope::EnveEmuInit(uint32 updateFreq, bool measuredValues)
{
	uint32 i, j, k;

	releaseTabLen = sizeof(releaseTab);

	for (i = 0; i < 256; i++)
	{
		j = 0;

		while ((j < releaseTabLen) && (releaseTab[j] > i))
			j++;

		if (j < releaseTabLen)
			releasePos[i] = j;
		else
			releasePos[i] = releaseTabLen - 1;
	}

	k = 0;

	for (i = 0; i < 16; i++)
	{
		for (j = 0; j < 256; j++)
		{
			uint16 tmpVol = j;

			if (measuredValues)
			{
				tmpVol = (uint16)((293.0 * (1 - exp(j / -130.0))) + 4.0);
				if (j == 0)
					tmpVol = 0;

				if (tmpVol > 255)
					tmpVol = 255;
			}

			// Want the modulated volume value in the high byte
			masterAmplModTable[k++] = ((tmpVol * masterVolumeLevels[i]) / 255) << 8;
		}
	}

	for (i = 0; i < 16; i++)
	{
		uint32 scaledenvelen = (uint32)floor((attackTimes[i] * updateFreq) / 1000UL);
		if (scaledenvelen == 0)
			scaledenvelen = 1;

		attackRates[i] = (AttackTabLen << 16) / scaledenvelen;

		scaledenvelen = (uint32)floor((decayReleaseTimes[i] * updateFreq) / 1000UL);
		if (scaledenvelen == 0)
			scaledenvelen = 1;

		decayReleaseRates[i] = (releaseTabLen << 16) / scaledenvelen;
	}
}



/******************************************************************************/
/* EnveEmuMute() is only used in the beginning.                               */
/******************************************************************************/
uint16 SIDEnvelope::EnveEmuMute(SIDOperator *pVoice, SIDEnvelope *enve)
{
	return (0);
}



/******************************************************************************/
/* EnveEmuStartAttack()                                                       */
/******************************************************************************/
uint16 SIDEnvelope::EnveEmuStartAttack(SIDOperator *pVoice, SIDEnvelope *enve)
{
	pVoice->adsrCtrl       = ENVE_ATTACK;
	pVoice->enveStep.w[HI] = pVoice->enveVol;
	pVoice->enveStep.w[LO] = 0;

	return (EnveEmuAlterAttack(pVoice, enve));
}



/******************************************************************************/
/* EnveEmuStartShortAttack()                                                  */
/******************************************************************************/
uint16 SIDEnvelope::EnveEmuStartShortAttack(SIDOperator *pVoice, SIDEnvelope *enve)
{
	pVoice->adsrCtrl             = ENVE_SHORTATTACK;
	pVoice->enveStep.w[HI]       = pVoice->enveVol;
	pVoice->enveStep.w[LO]       = 0;
	pVoice->enveShortAttackCount = 65535;	// Unused
	return (EnveEmuAlterShortAttack(pVoice, enve));
}



/******************************************************************************/
/* EnveEmuAttack()                                                            */
/******************************************************************************/
uint16 SIDEnvelope::EnveEmuAttack(SIDOperator *pVoice, SIDEnvelope *enve)
{
	if (pVoice->enveStep.w[HI] > AttackTabLen)
		return (EnveEmuStartDecay(pVoice, enve));
	else
	{
		pVoice->enveVol = pVoice->enveStep.w[HI];
		enve->EnveEmuEnveAdvance(pVoice);
		return (enve->masterAmplModTable[enve->masterVolumeAmplIndex + pVoice->enveVol]);
	}
}



/******************************************************************************/
/* EnveEmuShortAttack()                                                       */
/******************************************************************************/
uint16 SIDEnvelope::EnveEmuShortAttack(SIDOperator *pVoice, SIDEnvelope *enve)
{
	if ((pVoice->enveStep.w[HI] > AttackTabLen) || (pVoice->enveShortAttackCount == 0))
		return (EnveEmuStartDecay(pVoice, enve));
	else
	{
		pVoice->enveVol = pVoice->enveStep.w[HI];
		pVoice->enveShortAttackCount--;
		enve->EnveEmuEnveAdvance(pVoice);
		return (enve->masterAmplModTable[enve->masterVolumeAmplIndex + pVoice->enveVol]);
	}
}



/******************************************************************************/
/* EnveEmuAlterAttack()                                                       */
/******************************************************************************/
uint16 SIDEnvelope::EnveEmuAlterAttack(SIDOperator *pVoice, SIDEnvelope *enve)
{
	uint8 attack = pVoice->sidAD >> 4;

	pVoice->enveStepAdd.l = enve->attackRates[attack];
	pVoice->adsrProc      = EnveEmuAttack;
	return (EnveEmuAttack(pVoice, enve));
}



/******************************************************************************/
/* EnveEmuAlterShortAttack()                                                  */
/******************************************************************************/
uint16 SIDEnvelope::EnveEmuAlterShortAttack(SIDOperator *pVoice, SIDEnvelope *enve)
{
	uint8 attack = pVoice->sidAD >> 4;

	pVoice->enveStepAdd.l = enve->attackRates[attack];
	pVoice->adsrProc      = EnveEmuShortAttack;
	return (EnveEmuShortAttack(pVoice, enve));
}



/******************************************************************************/
/* EnveEmuStartDecay()                                                        */
/******************************************************************************/
uint16 SIDEnvelope::EnveEmuStartDecay(SIDOperator *pVoice, SIDEnvelope *enve)
{
	pVoice->adsrCtrl   = ENVE_DECAY;
	pVoice->enveStep.l = 0;
	return (EnveEmuAlterDecay(pVoice, enve));
}



/******************************************************************************/
/* EnveEmuDecay()                                                             */
/******************************************************************************/
uint16 SIDEnvelope::EnveEmuDecay(SIDOperator *pVoice, SIDEnvelope *enve)
{
	if (pVoice->enveStep.w[HI] >= enve->releaseTabLen)
	{
		pVoice->enveVol = pVoice->enveSusVol;
		return (EnveEmuAlterSustain(pVoice, enve));	// Start sustain
	}
	else
	{
		pVoice->enveVol = releaseTab[pVoice->enveStep.w[HI]];

		// Will be controlled from SidEmuSet2()
		if (pVoice->enveVol <= pVoice->enveSusVol)
		{
			pVoice->enveVol = pVoice->enveSusVol;
			return (EnveEmuAlterSustain(pVoice, enve));	// Start sustain
		}
		else
		{
			enve->EnveEmuEnveAdvance(pVoice);
			return (enve->masterAmplModTable[enve->masterVolumeAmplIndex + pVoice->enveVol]);
		}
	}
}



/******************************************************************************/
/* EnveEmuAlterDecay()                                                        */
/******************************************************************************/
uint16 SIDEnvelope::EnveEmuAlterDecay(SIDOperator *pVoice, SIDEnvelope *enve)
{
	uint8 decay = pVoice->sidAD & 0x0F;

	pVoice->enveStepAdd.l = enve->decayReleaseRates[decay];
	pVoice->adsrProc      = EnveEmuDecay;
	return (EnveEmuDecay(pVoice, enve));
}



/******************************************************************************/
/* EnveEmuSustain()                                                           */
/******************************************************************************/
uint16 SIDEnvelope::EnveEmuSustain(SIDOperator *pVoice, SIDEnvelope *enve)
{
	return (enve->masterAmplModTable[enve->masterVolumeAmplIndex + pVoice->enveVol]);
}



/******************************************************************************/
/* EnveEmuSustainDecay()                                                      */
/******************************************************************************/
uint16 SIDEnvelope::EnveEmuSustainDecay(SIDOperator *pVoice, SIDEnvelope *enve)
{
	if (pVoice->enveStep.w[HI] >= enve->releaseTabLen)
	{
		pVoice->enveVol = releaseTab[enve->releaseTabLen - 1];
		return (EnveEmuAlterSustain(pVoice, enve));
	}
	else
	{
		pVoice->enveVol = releaseTab[pVoice->enveStep.w[HI]];

		// Will be controlled from SidEmuSet2()
		if (pVoice->enveVol <= pVoice->enveSusVol)
		{
			pVoice->enveVol = pVoice->enveSusVol;
			return (EnveEmuAlterSustain(pVoice, enve));
		}
		else
		{
			enve->EnveEmuEnveAdvance(pVoice);
			return (enve->masterAmplModTable[enve->masterVolumeAmplIndex + pVoice->enveVol]);
		}
	}
}



/******************************************************************************/
/* EnveEmuAlterSustain() is the same as EnveEmuStartSustain().                */
/******************************************************************************/
uint16 SIDEnvelope::EnveEmuAlterSustain(SIDOperator *pVoice, SIDEnvelope *enve)
{
	if (pVoice->enveVol > pVoice->enveSusVol)
	{
		pVoice->adsrCtrl = ENVE_SUSTAINDECAY;
		pVoice->adsrProc = EnveEmuSustainDecay;
		return (EnveEmuAlterSustainDecay(pVoice, enve));
	}
	else
	{
		pVoice->adsrCtrl = ENVE_SUSTAIN;
		pVoice->adsrProc = EnveEmuSustain;
		return (EnveEmuSustain(pVoice, enve));
	}
}



/******************************************************************************/
/* EnveEmuAlterSustainDecay() is the same as EnveEmuStartSustainDecay().      */
/******************************************************************************/
uint16 SIDEnvelope::EnveEmuAlterSustainDecay(SIDOperator *pVoice, SIDEnvelope *enve)
{
	uint8 decay = pVoice->sidAD & 0x0F;

	pVoice->enveStepAdd.l = enve->decayReleaseRates[decay];
	pVoice->adsrProc      = EnveEmuSustainDecay;
	return (EnveEmuSustainDecay(pVoice, enve));
}



/******************************************************************************/
/* EnveEmuStartRelease()                                                      */
/******************************************************************************/
uint16 SIDEnvelope::EnveEmuStartRelease(SIDOperator *pVoice, SIDEnvelope *enve)
{
	pVoice->adsrCtrl = ENVE_RELEASE;
	pVoice->enveStep.w[HI] = enve->releasePos[pVoice->enveVol];
	pVoice->enveStep.w[LO] = 0;

	return (EnveEmuAlterRelease(pVoice, enve));
}



/******************************************************************************/
/* EnveEmuRelease()                                                           */
/******************************************************************************/
uint16 SIDEnvelope::EnveEmuRelease(SIDOperator *pVoice, SIDEnvelope *enve)
{
	if (pVoice->enveStep.w[HI] >= enve->releaseTabLen)
	{
		pVoice->enveVol = releaseTab[enve->releaseTabLen - 1];
		return (enve->masterAmplModTable[enve->masterVolumeAmplIndex + pVoice->enveVol]);
	}
	else
	{
		pVoice->enveVol = releaseTab[pVoice->enveStep.w[HI]];
		enve->EnveEmuEnveAdvance(pVoice);
		return (enve->masterAmplModTable[enve->masterVolumeAmplIndex + pVoice->enveVol]);
	}
}



/******************************************************************************/
/* EnveEmuAlterRelease()                                                      */
/******************************************************************************/
uint16 SIDEnvelope::EnveEmuAlterRelease(SIDOperator *pVoice, SIDEnvelope *enve)
{
	uint8 release = pVoice->sidSR & 0x0F;

	pVoice->enveStepAdd.l = enve->decayReleaseRates[release];
	pVoice->adsrProc      = EnveEmuRelease;
	return (EnveEmuRelease(pVoice, enve));
}



/******************************************************************************/
/* EnveEmuEnveAdvance()                                                       */
/******************************************************************************/
inline void SIDEnvelope::EnveEmuEnveAdvance(SIDOperator *pVoice)
{
	pVoice->enveStep.l += pVoice->enveStepAdd.l;
}
