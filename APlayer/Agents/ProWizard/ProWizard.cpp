/******************************************************************************/
/* ProWizard Converter class.                                                 */
/******************************************************************************/
/* This source, or parts thereof, may be used in any software as long the     */
/* license of APlayer is keep. See the LICENSE file for more information.     */
/*                                                                            */
/* Copyright (C) 1998-2002 by The APlayer-Team.                               */
/* All rights reserved.                                                       */
/******************************************************************************/


// PolyKit headers
#include "POS.h"

// Agent headers
#include "ProWizard.h"


/******************************************************************************/
/* Tables                                                                     */
/******************************************************************************/
int16 tuning[16][36] =
{
	{	856, 808, 762, 720, 678, 640, 604, 570, 538, 508, 480, 453,
		428, 404, 381, 360, 339, 320, 302, 285, 269, 254, 240, 226,
		214, 202, 190, 180, 170, 160, 151, 143, 135, 127, 120, 113
	},

	{	850, 802, 757, 715, 674, 637, 601, 567, 535, 505, 477, 450,
		425, 401, 379, 357, 337, 318, 300, 284, 268, 253, 239, 225,
		213, 201, 189, 179, 169, 159, 150, 142, 134, 126, 119, 113
	},

	{	844, 796, 752, 709, 670, 632, 597, 563, 532, 502, 474, 447,
		422, 398, 376, 355, 335, 316, 298, 282, 266, 251, 237, 224,
		211, 199, 188, 177, 167, 158, 149, 141, 133, 125, 118, 112
	},

	{	838, 791, 746, 704, 665, 628, 592, 559, 528, 498, 470, 444,
		419, 395, 373, 352, 332, 314, 296, 280, 264, 249, 235, 222,
		209, 198, 187, 176, 166, 157, 148, 140, 132, 125, 118, 111
	},

	{	832, 785, 741, 699, 660, 623, 588, 555, 524, 495, 467, 441,
		416, 392, 370, 350, 330, 312, 294, 278, 262, 247, 233, 220,
		208, 196, 185, 175, 165, 156, 147, 139, 131, 124, 117, 110
	},

	{	826, 779, 736, 694, 655, 619, 584, 551, 520, 491, 463, 437,
		413, 390, 368, 347, 328, 309, 292, 276, 260, 245, 232, 219,
		206, 195, 184, 174, 164, 155, 146, 138, 130, 123, 116, 109
	},

	{	820, 774, 730, 689, 651, 614, 580, 547, 516, 487, 460, 434,
		410, 387, 365, 345, 325, 307, 290, 274, 258, 244, 230, 217,
		205, 193, 183, 172, 163, 154, 145, 137, 129, 122, 115, 109
	},

	{	814, 768, 725, 684, 646, 610, 575, 543, 513, 484, 457, 431,
		407, 384, 363, 342, 323, 305, 288, 272, 256, 242, 228, 216,
		204, 192, 181, 171, 161, 152, 144, 136, 128, 121, 114, 108
	},

	{	907, 856, 808, 762, 720, 678, 640, 604, 570, 538, 508, 480,
		453, 428, 404, 381, 360, 339, 320, 302, 285, 269, 254, 240,
		226, 214, 202, 190, 180, 170, 160, 151, 143, 135, 127, 120
	},

	{	900, 850, 802, 757, 715, 675, 636, 601, 567, 535, 505, 477,
		450, 425, 401, 379, 357, 337, 318, 300, 284, 268, 253, 238,
		225, 212, 200, 189, 179, 169, 159, 150, 142, 134, 126, 119
	},

	{	894, 844, 796, 752, 709, 670, 632, 597, 563, 532, 502, 474,
		447, 422, 398, 376, 355, 335, 316, 298, 282, 266, 251, 237,
		223, 211, 199, 188, 177, 167, 158, 149, 141, 133, 125, 118
	},

	{	887, 838, 791, 746, 704, 665, 628, 592, 559, 528, 498, 470,
		444, 419, 395, 373, 352, 332, 314, 296, 280, 264, 249, 235,
		222, 209, 198, 187, 176, 166, 157, 148, 140, 132, 125, 118
	},

	{	881, 832, 785, 741, 699, 660, 623, 588, 555, 524, 494, 467,
		441, 416, 392, 370, 350, 330, 312, 294, 278, 262, 247, 233,
		220, 208, 196, 185, 175, 165, 156, 147, 139, 131, 123, 117
	},

	{	875, 826, 779, 736, 694, 655, 619, 584, 551, 520, 491, 463,
		437, 413, 390, 368, 347, 328, 309, 292, 276, 260, 245, 232,
		219, 206, 195, 184, 174, 164, 155, 146, 138, 130, 123, 116
	},

	{	868, 820, 774, 730, 689, 651, 614, 580, 547, 516, 487, 460,
		434, 410, 387, 365, 345, 325, 307, 290, 274, 258, 244, 230,
		217, 205, 193, 183, 172, 163, 154, 145, 137, 129, 122, 115
	},

	{	862, 814, 768, 725, 684, 646, 610, 575, 543, 513, 484, 457,
		431, 407, 384, 363, 342, 323, 305, 288, 272, 256, 242, 228,
		216, 203, 192, 181, 171, 161, 152, 144, 136, 128, 121, 114
	}
};



/******************************************************************************/
/* InitPeriods() initialize the period table.                                 */
/******************************************************************************/
void ProWizard::InitPeriods(void)
{
	period[0][0]=0x03,	period[0][1]=0x58;
	period[1][0]=0x03,	period[1][1]=0x28;
	period[2][0]=0x02,	period[2][1]=0xfa;
	period[3][0]=0x02,	period[3][1]=0xd0;
	period[4][0]=0x02,	period[4][1]=0xa6;
	period[5][0]=0x02,	period[5][1]=0x80;
	period[6][0]=0x02,	period[6][1]=0x5c;
	period[7][0]=0x02,	period[7][1]=0x3a;
	period[8][0]=0x02,	period[8][1]=0x1a;
	period[9][0]=0x01,	period[9][1]=0xfc;
	period[10][0]=0x01,	period[10][1]=0xe0;
	period[11][0]=0x01,	period[11][1]=0xc5;

	period[12][0]=0x01,	period[12][1]=0xac;
	period[13][0]=0x01,	period[13][1]=0x94;
	period[14][0]=0x01,	period[14][1]=0x7d;
	period[15][0]=0x01,	period[15][1]=0x68;
	period[16][0]=0x01,	period[16][1]=0x53;
	period[17][0]=0x01,	period[17][1]=0x40;
	period[18][0]=0x01,	period[18][1]=0x2e;
	period[19][0]=0x01,	period[19][1]=0x1d;
	period[20][0]=0x01,	period[20][1]=0x0d;
	period[21][0]=0x00,	period[21][1]=0xfe;
	period[22][0]=0x00,	period[22][1]=0xf0;
	period[23][0]=0x00,	period[23][1]=0xe2;

	period[24][0]=0x00,	period[24][1]=0xd6;
	period[25][0]=0x00,	period[25][1]=0xca;
	period[26][0]=0x00,	period[26][1]=0xbe;
	period[27][0]=0x00,	period[27][1]=0xb4;
	period[28][0]=0x00,	period[28][1]=0xaa;
	period[29][0]=0x00,	period[29][1]=0xa0;
	period[30][0]=0x00,	period[30][1]=0x97;
	period[31][0]=0x00,	period[31][1]=0x8f;
	period[32][0]=0x00,	period[32][1]=0x87;
	period[33][0]=0x00,	period[33][1]=0x7f;
	period[34][0]=0x00,	period[34][1]=0x78;
	period[35][0]=0x00,	period[35][1]=0x71;

	period[36][0]=0x00,	period[36][1]=0x71;
	period[37][0]=0x00,	period[37][1]=0x71;
	period[38][0]=0x00,	period[38][1]=0x71;
	period[39][0]=0x00,	period[39][1]=0x71;
	period[40][0]=0x00,	period[40][1]=0x71;
	period[41][0]=0x00,	period[41][1]=0x71;
	period[42][0]=0x00,	period[42][1]=0x71;
	period[43][0]=0x00,	period[43][1]=0x71;
	period[44][0]=0x00,	period[44][1]=0x71;
}



/******************************************************************************/
/* GetPGP() finds the heighest pattern number.                                */
/*                                                                            */
/* Input:   "posTable" is a pointer to the position table.                    */
/*          "posSize" is the number of positions.                             */
/*                                                                            */
/* Output:  The heighest pattern number.                                      */
/******************************************************************************/
uint16 ProWizard::GetPGP(const uint8 *posTable, uint16 posSize)
{
	uint16 i;

	pattNum = 0;
	for (i = 0; i < posSize; i++)
	{
		if (posTable[i] > pattNum)
			pattNum = posTable[i];
	}

	pattNum++;
	return (pattNum);
}
