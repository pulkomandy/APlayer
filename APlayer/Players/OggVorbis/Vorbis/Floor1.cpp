/******************************************************************************/
/* Vorbis floor1 functions.                                                   */
/******************************************************************************/


// Player headers
#include "Registry.h"
#include "OggVorbis.h"


/******************************************************************************/
/* Tables                                                                     */
/******************************************************************************/
const float floor1FromdBLookup[256] =
{
	1.0649863e-07F, 1.1341951e-07F, 1.2079015e-07F, 1.2863978e-07F, 
	1.3699951e-07F, 1.4590251e-07F, 1.5538408e-07F, 1.6548181e-07F, 
	1.7623575e-07F, 1.8768855e-07F, 1.9988561e-07F, 2.128753e-07F, 
	2.2670913e-07F, 2.4144197e-07F, 2.5713223e-07F, 2.7384213e-07F, 
	2.9163793e-07F, 3.1059021e-07F, 3.3077411e-07F, 3.5226968e-07F, 
	3.7516214e-07F, 3.9954229e-07F, 4.2550680e-07F, 4.5315863e-07F, 
	4.8260743e-07F, 5.1396998e-07F, 5.4737065e-07F, 5.8294187e-07F, 
	6.2082472e-07F, 6.6116941e-07F, 7.0413592e-07F, 7.4989464e-07F, 
	7.9862701e-07F, 8.5052630e-07F, 9.0579828e-07F, 9.6466216e-07F, 
	1.0273513e-06F, 1.0941144e-06F, 1.1652161e-06F, 1.2409384e-06F, 
	1.3215816e-06F, 1.4074654e-06F, 1.4989305e-06F, 1.5963394e-06F, 
	1.7000785e-06F, 1.8105592e-06F, 1.9282195e-06F, 2.0535261e-06F, 
	2.1869758e-06F, 2.3290978e-06F, 2.4804557e-06F, 2.6416497e-06F, 
	2.8133190e-06F, 2.9961443e-06F, 3.1908506e-06F, 3.3982101e-06F, 
	3.6190449e-06F, 3.8542308e-06F, 4.1047004e-06F, 4.3714470e-06F, 
	4.6555282e-06F, 4.9580707e-06F, 5.2802740e-06F, 5.6234160e-06F, 
	5.9888572e-06F, 6.3780469e-06F, 6.7925283e-06F, 7.2339451e-06F, 
	7.7040476e-06F, 8.2047000e-06F, 8.7378876e-06F, 9.3057248e-06F, 
	9.9104632e-06F, 1.0554501e-05F, 1.1240392e-05F, 1.1970856e-05F, 
	1.2748789e-05F, 1.3577278e-05F, 1.4459606e-05F, 1.5399272e-05F, 
	1.6400004e-05F, 1.7465768e-05F, 1.8600792e-05F, 1.9809576e-05F, 
	2.1096914e-05F, 2.2467911e-05F, 2.3928002e-05F, 2.5482978e-05F, 
	2.7139006e-05F, 2.8902651e-05F, 3.0780908e-05F, 3.2781225e-05F, 
	3.4911534e-05F, 3.7180282e-05F, 3.9596466e-05F, 4.2169667e-05F, 
	4.4910090e-05F, 4.7828601e-05F, 5.0936773e-05F, 5.4246931e-05F, 
	5.7772202e-05F, 6.1526565e-05F, 6.5524908e-05F, 6.9783085e-05F, 
	7.4317983e-05F, 7.9147585e-05F, 8.4291040e-05F, 8.9768747e-05F, 
	9.5602426e-05F, 0.00010181521F, 0.00010843174F, 0.00011547824F, 
	0.00012298267F, 0.00013097477F, 0.00013948625F, 0.00014855085F, 
	0.00015820453F, 0.00016848555F, 0.00017943469F, 0.00019109536F, 
	0.00020351382F, 0.00021673929F, 0.00023082423F, 0.00024582449F, 
	0.00026179955F, 0.00027881276F, 0.00029693158F, 0.00031622787F, 
	0.00033677814F, 0.00035866388F, 0.00038197188F, 0.00040679456F, 
	0.00043323036F, 0.00046138411F, 0.00049136745F, 0.00052329927F, 
	0.00055730621F, 0.00059352311F, 0.00063209358F, 0.00067317058F, 
	0.00071691700F, 0.00076350630F, 0.00081312324F, 0.00086596457F, 
	0.00092223983F, 0.00098217216F, 0.0010459992F, 0.0011139742F, 
	0.0011863665F, 0.0012634633F, 0.0013455702F, 0.0014330129F, 
	0.0015261382F, 0.0016253153F, 0.0017309374F, 0.0018434235F, 
	0.0019632195F, 0.0020908006F, 0.0022266726F, 0.0023713743F, 
	0.0025254795F, 0.0026895994F, 0.0028643847F, 0.0030505286F, 
	0.0032487691F, 0.0034598925F, 0.0036847358F, 0.0039241906F, 
	0.0041792066F, 0.0044507950F, 0.0047400328F, 0.0050480668F, 
	0.0053761186F, 0.0057254891F, 0.0060975636F, 0.0064938176F, 
	0.0069158225F, 0.0073652516F, 0.0078438871F, 0.0083536271F, 
	0.0088964928F, 0.009474637F, 0.010090352F, 0.010746080F, 
	0.011444421F, 0.012188144F, 0.012980198F, 0.013823725F, 
	0.014722068F, 0.015678791F, 0.016697687F, 0.017782797F, 
	0.018938423F, 0.020169149F, 0.021479854F, 0.022875735F, 
	0.024362330F, 0.025945531F, 0.027631618F, 0.029427276F, 
	0.031339626F, 0.033376252F, 0.035545228F, 0.037855157F, 
	0.040315199F, 0.042935108F, 0.045725273F, 0.048696758F, 
	0.051861348F, 0.055231591F, 0.058820850F, 0.062643361F, 
	0.066714279F, 0.071049749F, 0.075666962F, 0.080584227F, 
	0.085821044F, 0.091398179F, 0.097337747F, 0.10366330F, 
	0.11039993F, 0.11757434F, 0.12521498F, 0.13335215F, 
	0.14201813F, 0.15124727F, 0.16107617F, 0.17154380F, 
	0.18269168F, 0.19456402F, 0.20720788F, 0.22067342F, 
	0.23501402F, 0.25028656F, 0.26655159F, 0.28387361F, 
	0.30232132F, 0.32196786F, 0.34289114F, 0.36517414F, 
	0.38890521F, 0.41417847F, 0.44109412F, 0.46975890F, 
	0.50028648F, 0.53279791F, 0.56742212F, 0.60429640F, 
	0.64356699F, 0.68538959F, 0.72993007F, 0.77736504F, 
	0.82788260F, 0.88168307F, 0.9389798F, 1.F,
};



/******************************************************************************/
/* Structures                                                                 */
/******************************************************************************/
typedef struct VorbisLookFloor1
{
	int32 sortedIndex[VIF_POSIT + 2];
	int32 forwardIndex[VIF_POSIT + 2];
	int32 reverseIndex[VIF_POSIT + 2];

	int32 hiNeighbor[VIF_POSIT];
	int32 loNeighbor[VIF_POSIT];
	int32 posts;

	int32 n;
	int32 quantQ;
	VorbisInfoFloor1 *vi;
} VorbisLookFloor1;



/******************************************************************************/
/* icomp()                                                                    */
/******************************************************************************/
int OggVorbis::icomp(const void *a, const void *b)
{
	return (**(int32 **)a - **(int32 **)b);
}



/******************************************************************************/
/* Floor1_Unpack()                                                            */
/******************************************************************************/
VorbisInfoFloor *OggVorbis::Floor1_Unpack(OggVorbis *obj, VorbisInfo *vi, OggPackBuffer *opb)
{
	CodecSetupInfo *ci = (CodecSetupInfo *)vi->codecSetup;
	int32 j, k, count = 0, maxClass = -1, rangeBits;

	VorbisInfoFloor1 *info = (VorbisInfoFloor1 *)_ogg_calloc(1, sizeof(*info));

	// Read partitions
	info->partitions = obj->OggPack_Read(opb, 5);		// Only 0 to 31 legal
	for (j = 0; j < info->partitions; j++)
	{
		info->partitionClass[j] = obj->OggPack_Read(opb, 4);	// Only 0 to 15 legal
		if (maxClass < info->partitionClass[j])
			maxClass = info->partitionClass[j];
	}

	// Read partition classes
	for (j = 0; j < maxClass + 1; j++)
	{
		info->classDim[j]  = obj->OggPack_Read(opb, 3) + 1;	// 1 to 8
		info->classSubs[j] = obj->OggPack_Read(opb, 2);		// 0,1,2,3 bits

		if (info->classSubs[j] < 0)
			goto err_out;

		if (info->classSubs[j])
			info->classBook[j] = obj->OggPack_Read(opb, 8);

		if ((info->classBook[j] < 0) || (info->classBook[j] >= ci->books))
			goto err_out;

		for (k = 0; k < (1 << info->classSubs[j]); k++)
		{
			info->classSubBook[j][k] = obj->OggPack_Read(opb, 8) - 1;
			if ((info->classSubBook[j][k] < -1) || (info->classSubBook[j][k] >= ci->books))
				goto err_out;
		}
	}

	// Read the post list
	info->mult = obj->OggPack_Read(opb, 2) + 1;		// Only 1,2,3,4 legal now
	rangeBits  = obj->OggPack_Read(opb, 4);

	for (j = 0, k = 0; j < info->partitions; j++)
	{
		count += info->classDim[info->partitionClass[j]];
		for (; k < count; k++)
		{
			int32 t = info->postList[k + 2] = obj->OggPack_Read(opb, rangeBits);
			if ((t < 0) || (t >= (1 << rangeBits)))
				goto err_out;
		}
	}

	info->postList[0] = 0;
	info->postList[1] = 1 << rangeBits;

	return (info);

err_out:
	Floor1_FreeInfo(info);
	return (NULL);
}



/******************************************************************************/
/* Floor1_Look()                                                              */
/******************************************************************************/
VorbisLookFloor *OggVorbis::Floor1_Look(VorbisDSPState *vd, VorbisInfoFloor *in)
{
	int32 *sortPointer[VIF_POSIT + 2];
	VorbisInfoFloor1 *info = (VorbisInfoFloor1 *)in;
	VorbisLookFloor1 *look = (VorbisLookFloor1 *)_ogg_calloc(1, sizeof(*look));
	int32 i, j, n = 0;

	look->vi = info;
	look->n  = info->postList[1];

	// We drop each position value in-between already decoded values,
	// and use linear interpolation to predict each new value past the
	// edges. The positions are read in the order of the position
	// list... We precompute the bounding positions in the lookup.
	// Ofcouse, the neighbors can change (if a position is declined), but
	// this is an initial mapping
	for (i = 0; i < info->partitions; i++)
		n += info->classDim[info->partitionClass[i]];

	n += 2;
	look->posts = n;

	// Also store a sorted position index
	for (i = 0; i < n; i++)
		sortPointer[i] = info->postList + i;

	qsort(sortPointer, n, sizeof(*sortPointer), icomp);

	// Points from sort order back to range number
	for (i = 0; i < n; i++)
		look->forwardIndex[i] = sortPointer[i] - info->postList;

	// Points from range order to sorted position
	for (i = 0; i < n; i++)
		look->reverseIndex[look->forwardIndex[i]] = i;

	// We actually need the post values too
	for (i = 0; i < n; i++)
		look->sortedIndex[i] = info->postList[look->forwardIndex[i]];

	// Quantize values to multiplier spec
	switch (info->mult)
	{
		// 1024 -> 256
		case 1:
		{
			look->quantQ = 256;
			break;
		}

		// 1024 -> 128
		case 2:
		{
			look->quantQ = 128;
			break;
		}

		// 1024 -> 86
		case 3:
		{
			look->quantQ = 86;
			break;
		}

		// 1024 -> 64
		case 4:
		{
			look->quantQ = 64;
			break;
		}
	}

	// Discover our neighbors for decode where we don't use fit flags
	// (that would push the neighbors outward)
	for (i = 0; i < n - 2; i++)
	{
		int32 lo = 0;
		int32 hi = 1;
		int32 lx = 0;
		int32 hx = look->n;
		int32 currentX = info->postList[i + 2];

		for (j = 0; j < i + 2; j++)
		{
			int32 x = info->postList[j];
			if ((x > lx) && (x < currentX))
			{
				lo = j;
				lx = x;
			}

			if ((x < hx) && (x > currentX))
			{
				hi = j;
				hx = x;
			}
		}

		look->loNeighbor[i] = lo;
		look->hiNeighbor[i] = hi;
	}

	return (look);
}



/******************************************************************************/
/* Floor1_FreeInfo()                                                          */
/******************************************************************************/
void OggVorbis::Floor1_FreeInfo(VorbisInfoFloor *i)
{
	VorbisInfoFloor1 *info = (VorbisInfoFloor1 *)i;
	_ogg_free(info);
}



/******************************************************************************/
/* Floor1_FreeLook()                                                          */
/******************************************************************************/
void OggVorbis::Floor1_FreeLook(VorbisLookFloor *i)
{
	VorbisLookFloor1 *look = (VorbisLookFloor1 *)i;
	_ogg_free(look);
}



/******************************************************************************/
/* Floor1_Inverse1()                                                          */
/******************************************************************************/
void *OggVorbis::Floor1_Inverse1(OggVorbis *obj, VorbisBlock *vb, VorbisLookFloor *in)
{
	VorbisLookFloor1 *look = (VorbisLookFloor1 *)in;
	VorbisInfoFloor1 *info = look->vi;
	CodecSetupInfo *ci     = (CodecSetupInfo *)vb->vd->vi->codecSetup;

	int32 i, j, k;
	Codebook *books = ci->fullBooks;

	// Unpack wrapped/predicted values from stream
	if (obj->OggPack_Read(&vb->opb, 1) == 1)
	{
		int32 *fitValue = (int32 *)obj->_Vorbis_Block_Alloc(vb, (look->posts) * sizeof(*fitValue));

		fitValue[0] = obj->OggPack_Read(&vb->opb, ilog(look->quantQ - 1));
		fitValue[1] = obj->OggPack_Read(&vb->opb, ilog(look->quantQ - 1));

		// Partition by partition
		for (i = 0, j = 2; i < info->partitions; i++)
		{
			int32 pClass   = info->partitionClass[i];
			int32 cDim     = info->classDim[pClass];
			int32 cSubBits = info->classSubs[pClass];
			int32 cSub     = 1 << cSubBits;
			int32 cVal     = 0;

			// Decode the partition's first stage cascade value
			if (cSubBits)
			{
				cVal = obj->Vorbis_Book_Decode(books + info->classBook[pClass], &vb->opb);
				if (cVal == -1)
					goto eop;
			}

			for (k = 0; k < cDim; k++)
			{
				int32 book = info->classSubBook[pClass][cVal & (cSub - 1)];
				cVal >>= cSubBits;

				if (book >= 0)
				{
					if ((fitValue[j + k] = obj->Vorbis_Book_Decode(books + book, &vb->opb)) == -1)
						goto eop;
				}
				else
					fitValue[j + k] = 0;
			}

			j += cDim;
		}

		// Unwrap positive values and reconsitute via linear interpolation
		for (i = 2; i < look->posts; i++)
		{
			int32 predicted = obj->RenderPoint(info->postList[look->loNeighbor[i - 2]], info->postList[look->hiNeighbor[i - 2]], fitValue[look->loNeighbor[i - 2]], fitValue[look->hiNeighbor[i - 2]], info->postList[i]);
			int32 hiRoom    = look->quantQ - predicted;
			int32 loRoom    = predicted;
			int32 room      = (hiRoom < loRoom ? hiRoom : loRoom) << 1;
			int32 val       = fitValue[i];

			if (val)
			{
				if (val >= room)
				{
					if (hiRoom > loRoom)
						val = val - loRoom;
					else
						val = -1 - (val - hiRoom);
				}
				else
				{
					if (val & 1)
						val = -((val + 1) >> 1);
					else
						val >>= 1;
				}

				fitValue[i] = val + predicted;
				fitValue[look->loNeighbor[i - 2]] &= 0x7fff;
				fitValue[look->hiNeighbor[i - 2]] &= 0x7fff;
			}
			else
				fitValue[i] = predicted | 0x8000;
		}

		return (fitValue);
	}

eop:
	return (NULL);
}



/******************************************************************************/
/* Floor1_Inverse2()                                                          */
/******************************************************************************/
int32 OggVorbis::Floor1_Inverse2(OggVorbis *obj, VorbisBlock *vb, VorbisLookFloor *in, void *memo, float *out)
{
	VorbisLookFloor1 *look = (VorbisLookFloor1 *)in;
	VorbisInfoFloor1 *info = look->vi;

	CodecSetupInfo *ci = (CodecSetupInfo *)vb->vd->vi->codecSetup;
	int32 n = ci->blockSizes[vb->w] / 2;
	int32 j;

	if (memo)
	{
		// Render the lines
		int32 *fitValue = (int32 *)memo;
		int32 hx = 0;
		int32 lx = 0;
		int32 ly = fitValue[0] * info->mult;

		for (j = 1; j < look->posts; j++)
		{
			int32 current = look->forwardIndex[j];
			int32 hy      = fitValue[current] & 0x7fff;

			if (hy == fitValue[current])
			{
				hy *= info->mult;
				hx  = info->postList[current];

				obj->RenderLine(lx, hx, ly, hy, out);

				lx = hx;
				ly = hy;
			}
		}

		for (j = hx; j < n; j++)
			out[j] *= floor1FromdBLookup[ly];	// Be certain

		return (1);
	}

	memset(out, 0, sizeof(*out) * n);
	return (0);
}



/******************************************************************************/
/* RenderPoint()                                                              */
/******************************************************************************/
int32 OggVorbis::RenderPoint(int32 x0, int32 x1, int32 y0, int32 y1, int32 x)
{
	y0 &= 0x7fff;		// Mask off flag
	y1 &= 0x7fff;

	{
		int32 dy  = y1 - y0;
		int32 adx = x1 - x0;
		int32 ady = abs(dy);
		int32 err = ady * (x - x0);

		int32 off = err / adx;
		if (dy < 0)
			return (y0 - off);

		return (y0 + off);
	}
}



/******************************************************************************/
/* RenderLine()                                                               */
/******************************************************************************/
void OggVorbis::RenderLine(int32 x0, int32 x1, int32 y0, int32 y1, float *d)
{
	int32 dy   = y1 - y0;
	int32 adx  = x1 - x0;
	int32 ady  = abs(dy);
	int32 base = dy / adx;
	int32 sy   = (dy < 0 ? base - 1 : base + 1);
	int32 x    = x0;
	int32 y    = y0;
	int32 err  = 0;

	ady -= abs(base * adx);

	d[x] *= floor1FromdBLookup[y];

	while (++x < x1)
	{
		err = err + ady;
		if (err >= adx)
		{
			err -= adx;
			y   += sy;
		}
		else
			y += base;

		d[x] *= floor1FromdBLookup[y];
	}
}



// Export hooks
Vorbis_Func_Floor floor1_exportBundle =
{
	&OggVorbis::Floor1_Unpack,
	&OggVorbis::Floor1_Look,
	&OggVorbis::Floor1_FreeInfo,
	&OggVorbis::Floor1_FreeLook,
	&OggVorbis::Floor1_Inverse1,
	&OggVorbis::Floor1_Inverse2
};
