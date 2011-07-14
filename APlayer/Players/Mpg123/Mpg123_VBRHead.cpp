/******************************************************************************/
/* MPG123Player VBRHead functions.                                            */
/******************************************************************************/


// Player headers
#include "Mpg123Player.h"


/******************************************************************************/
/* Get32Bits() read a 32-bit number and return it.                            */
/*                                                                            */
/* Input:  "buf" is a pointer to the data to parse.                           */
/*                                                                            */
/* Output: The number read.                                                   */
/******************************************************************************/
uint32 MPG123Player::Get32Bits(uint8 *buf)
{
	uint32 ret;

	ret = (((uint32)buf[0]) << 24) | (((uint32)buf[1]) << 16) | (((uint32)buf[2]) << 8) | ((uint32)buf[3]);

	return (ret);
}



/******************************************************************************/
/* GetVBRHeader() read and parse the VBR header.                              */
/*                                                                            */
/* Input:  "head" is a pointer to the VBR structure to use.                   */
/*         "buf" is a pointer to the data to parse.                           */
/*         "fr" is a pointer to the frame structure to use.                   */
/*                                                                            */
/* Output: True if VBR header is found, false if not.                         */
/******************************************************************************/
bool MPG123Player::GetVBRHeader(VBRHeader *head, uint8 *buf, Frame *fr)
{
	int32 ssize;

	if (fr->lay != 3)
		return (false);

	if (fr->lsf)
		ssize = (fr->stereo == 1) ? 9 : 17;
	else
		ssize = (fr->stereo == 1) ? 17 : 32;

	buf += ssize;

	if ((buf[0] != 'X') || (buf[1] != 'i') || (buf[2] != 'n') || (buf[3] != 'g'))
		return (false);

	buf += 4;

	head->flags = Get32Bits(buf);
	buf += 4;

	if (head->flags & VBR_FRAMES_FLAG)
	{
		head->frames = Get32Bits(buf);
		buf += 4;
	}

	if (head->flags & VBR_BYTES_FLAG)
	{
		head->bytes = Get32Bits(buf);
		buf += 4;
	}

	if (head->flags & VBR_TOC_FLAG)
	{
		memcpy(head->toc, buf, 100);
		buf += 100;
	}

	if (head->flags & VBR_SCALE_FLAG)
	{
		head->scale = Get32Bits(buf);
		buf += 4;
	}

	return (true);
}
