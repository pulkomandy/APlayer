/******************************************************************************/
/* OctaMED Player Interface.                                                  */
/*                                                                            */
/* Original player by Teijo Kinnunen.                                         */
/* APlayer add-on by Thomas Neumann.                                          */
/******************************************************************************/
/* This source, or parts thereof, may be used in any software as long the     */
/* license of APlayer is keep. See the LICENSE file for more information.     */
/*                                                                            */
/* Copyright (C) 1998-2002 by The APlayer-Team.                               */
/* All rights reserved.                                                       */
/******************************************************************************/


// PolyKit headers
#include "POS.h"
#include "PException.h"
#include "PString.h"
#include "PResource.h"
#include "PFile.h"
#include "PTime.h"

// APlayerKit headers
#include "APGlobalData.h"
#include "APAddOns.h"
#include "APChannel.h"

// Player headers
#include "MEDTypes.h"
#include "OctaMED.h"
#include "Block.h"
#include "EffectMaster.h"
#include "Instr.h"
#include "MMDSampleHdr.h"
#include "Player.h"
#include "ScanSong.h"
#include "Song.h"
#include "SubSong.h"
#include "TagsLoad.h"
#include "Tempo.h"
#include "ResourceIDs.h"


/******************************************************************************/
/* Version                                                                    */
/******************************************************************************/
#define PlayerVersion		2.04f



/******************************************************************************/
/* Constructor                                                                */
/******************************************************************************/
OctaMED::OctaMED(APGlobalData *global, PString fileName) : APAddOnPlayer(global)
{
	// Fill out the version we have compiled under
	aplayerVersion = APLAYER_CURRENT_VERSION;

	// Initialize member variables
	sg  = NULL;
	plr = NULL;

	// Allocate the resource object
	res = new PResource(fileName);
	if (res == NULL)
		throw PMemoryException();
}



/******************************************************************************/
/* Destructor                                                                 */
/******************************************************************************/
OctaMED::~OctaMED(void)
{
	// Delete the resource object
	delete res;
}



/******************************************************************************/
/* GetVersion() returns the version of the add-on.                            */
/*                                                                            */
/* Output: The add-on version.                                                */
/******************************************************************************/
float OctaMED::GetVersion(void)
{
	return (PlayerVersion);
}



/******************************************************************************/
/* GetCount() returns the number of add-ons in the add-on.                    */
/*                                                                            */
/* Output: Is the number of the add-ons.                                      */
/******************************************************************************/
int32 OctaMED::GetCount(void)
{
	return (5);
}



/******************************************************************************/
/* GetSupportFlags() returns some flags telling what the add-on supports.     */
/*                                                                            */
/* Input:  "index" is the add-on index starting from 0.                       */
/*                                                                            */
/* Output: Is the flags.                                                      */
/******************************************************************************/
uint32 OctaMED::GetSupportFlags(int32 index)
{
	return (appSetPosition);
}



/******************************************************************************/
/* GetName() returns the name of the current add-on.                          */
/*                                                                            */
/* Input:  "index" is the add-on index starting from 0.                       */
/*                                                                            */
/* Output: The add-on name.                                                   */
/******************************************************************************/
PString OctaMED::GetName(int32 index)
{
	PString name;

	name.LoadString(res, IDS_MED_NAME + index);
	return (name);
}



/******************************************************************************/
/* GetDescription() returns the description of the current add-on.            */
/*                                                                            */
/* Input:  "index" is the add-on index starting from 0.                       */
/*                                                                            */
/* Output: The add-on description.                                            */
/******************************************************************************/
PString OctaMED::GetDescription(int32 index)
{
	PString description;

	description.LoadString(res, IDS_MED_DESCRIPTION + index);
	return (description);
}



/******************************************************************************/
/* GetModTypeString() returns the module type string for the current player.  */
/*                                                                            */
/* Input:  "index" is the player index number.                                */
/*                                                                            */
/* Output: The module type string.                                            */
/******************************************************************************/
PString OctaMED::GetModTypeString(int32 index)
{
	PString type;

	type.LoadString(res, IDS_MED_MIME);
	return (type);
}



/******************************************************************************/
/* ModuleCheck() tests the module to see which type of module it is.          */
/*                                                                            */
/* Input:  "index" is the player index number.                                */
/*         "file" is a pointer to the file object with the file to check.     */
/*                                                                            */
/* Output: An APlayer result code.                                            */
/******************************************************************************/
ap_result OctaMED::ModuleCheck(int32 index, PFile *file)
{
	MedType checkType;

	// Check the module
	checkType = TestModule(file);

	if (checkType == index)
		return (AP_OK);

	// We couldn't recognize it
	return (AP_UNKNOWN);
}



/******************************************************************************/
/* LoadModule() will load the module into the memory.                         */
/*                                                                            */
/* Input:  "index" is the player index number.                                */
/*         "file" is a pointer to a file object with the file to check.       */
/*         "errorStr" is a reference to a string, where you should store the  */
/*         error if you can't load the module.                                */
/*                                                                            */
/* Output: An APlayer result code.                                            */
/******************************************************************************/
ap_result OctaMED::LoadModule(int32 index, PFile *file, PString &errorStr)
{
	MMD_Hdr hdr0, hdrX;
	MMD_Hdr *currHdr;
	bool mixConv = false;			// Perform conversion from non-mix mode?
	bool eightChConv = false;		// Or eight channel mode?
	ap_result retVal = AP_ERROR;

	// Allocate the needed objects
	sg = new Song(this);
	if (sg == NULL)
	{
		errorStr.LoadString(res, IDS_MED_ERR_MEMORY);
		return (AP_ERROR);
	}

	plr = new Player(this);
	if (plr == NULL)
	{
		delete sg;
		sg = NULL;

		errorStr.LoadString(res, IDS_MED_ERR_MEMORY);
		return (AP_ERROR);
	}

	try
	{
		// Initialize member variables
		numChannels = 0;

		// Initialize the current header
		currHdr = &hdr0;

		do
		{
			MMD0SongData song0;
			MMD2SongData song2;
			MMD0ExpData currExp;
			uint32 cnt;
			TRACK_NUM songTracks = 0;
			uint32 numBlocks;
			SubSong *css;
			uint32 *blkArray;
			bool usesHexVol;

			// Read the module header
			ReadMMDHeader(file, currHdr);
			if (file->IsEOF())
			{
				errorStr.LoadString(res, IDS_MED_ERR_LOADING_HEADER);
				throw PUserException();
			}

			// Seek to the song structure
			file->Seek(currHdr->songOffs, PFile::pSeekBegin);

			// New (empty) subsong
			sg->AppendNew(true);
			(*sg)++;
			css = sg->CurrSS();

			for (cnt = 0; cnt < 63; cnt++)
			{
				MMD0Sample loadSmp;
				Instr *ci;

				// Read the sample structure
				ReadMMD0Sample(file, &loadSmp);
				if (file->IsEOF())
				{
					errorStr.LoadString(res, IDS_MED_ERR_LOADING_INSTRUMENTS);
					throw PUserException();
				}

				// Set the instrument information
				ci = sg->GetInstr(cnt);
				ci->SetRepeat(loadSmp.rep << 1);
				ci->SetRepeatLen(loadSmp.repLen << 1);
				ci->SetTransp(loadSmp.sTrans);
				ci->SetVol(loadSmp.volume * 2);
				ci->SetMIDICh(loadSmp.midiCh);
			}

			if (mark < '2')
			{
				// Type MMD0 or MMD1
				ReadMMD0Song(file, &song0);

				numSamples = min(song0.numSamples, MAX_INSTRS);

				if (!(song0.flags2 & MMD_FLAG2_MIX))
					mixConv = true;

				if (song0.flags & MMD_FLAG_VOLHEX)
					usesHexVol = true;
				else
					usesHexVol = false;

				if (song0.flags & MMD_FLAG_STSLIDE)
					css->SetSlide1st(true);

				if (song0.flags & MMD_FLAG_8CHANNEL)
					eightChConv = true;

				css->SetTempoBPM(song0.defTempo);
				css->SetTempoTPL(song0.tempo2);
				css->SetTempoLPB((song0.flags2 & MMD_FLAG2_BMASK) + 1);
				css->SetTempoMode((song0.flags2 & MMD_FLAG2_BPM) ? true : false);
				css->SetPlayTranspose(song0.playTransp);
				css->SetNumChannels(4);
				css->SetAmigaFilter((song0.flags & MMD_FLAG_FILTERON) ? true : false);

				for (cnt = 0; cnt < 16; cnt++)
					css->SetTrackVol(cnt, song0.trkVol[cnt]);

				css->SetMasterVol(song0.masterVol);

				// No sections here...
				css->AppendNewSec(0);

				// Playing sequence...
				PlaySeq *newPSeq = new PlaySeq();
				css->Append(newPSeq);

				for (cnt = 0; cnt < song0.songLen; cnt++)
					newPSeq->AddTail(new PlaySeqEntry((uint16)song0.playSeq[cnt]));
			}
			else
			{
				// MMD2 or MMD3
				uint32 *pSqTbl;

				ReadMMD2Song(file, &song2);

				numSamples = min(song2.numSamples, MAX_INSTRS);

				if (!(song2.flags2 & MMD_FLAG2_MIX))
					mixConv = true;

				if (song2.flags & MMD_FLAG_VOLHEX)
					usesHexVol = true;
				else
					usesHexVol = false;

				if (song2.flags & MMD_FLAG_STSLIDE)
					css->SetSlide1st(true);

				if (song2.flags & MMD_FLAG_8CHANNEL)
					eightChConv = true;

				if (song2.flags3 & MMD_FLAG3_STEREO)
					css->SetStereo(true);

				if (song2.flags3 & MMD_FLAG3_FREEPAN)
					css->SetFreePan(true);

				if (song2.flags3 & MMD_FLAG3_GM)
					css->SetGM(true);

				css->SetTempoBPM(song2.defTempo);
				css->SetTempoTPL(song2.tempo2);
				css->SetTempoLPB((song2.flags2 & MMD_FLAG2_BMASK) + 1);
				css->SetTempoMode((song2.flags2 & MMD_FLAG2_BPM) ? true : false);
				css->SetPlayTranspose(song2.playTransp);
				css->SetNumChannels(song2.channels != 0 ? song2.channels : 4);
				css->SetAmigaFilter((song2.flags & MMD_FLAG_FILTERON) ? true : false);
				css->SetVolAdjust(song2.volAdj != 0 ? song2.volAdj : 100);

				// Effects
//XX				switch (song2.mixEchoType)
/*				{
					case 1:
					{
						css->fx.GlobalGroup().SetEchoMode(EffectGroup::NORMAL);
						break;
					}

					case 2:
					{
						css->fx.GlobalGroup().SetEchoMode(EffectGroup::XECHO);
						break;
					}

					default:
					{
						css->fx.GlobalGroup().SetEchoMode(EffectGroup::NONE);
						break;
					}
				}

				if ((song2.mixEchoDepth >= 1) && (song2.mixEchoDepth <= 6))
					css->fx.GlobalGroup().SetEchoDepth(song2.mixEchoDepth);

				if (song2.mixEchoLen > 0)
					css->fx.GlobalGroup().SetEchoLength(song2.mixEchoLen);

				if ((song2.mixStereoSep >= -4) && (song2.mixStereoSep <= 4))
					css->fx.GlobalGroup().SetStereoSeparation(song2.mixStereoSep);
*/
				// Read track volumes
				songTracks = min((uint16)MAX_TRACKS, song2.numTracks);
				file->Seek(song2.trackVolsOffs, PFile::pSeekBegin);

				for (cnt = 0; cnt < (uint32)songTracks; cnt++)
					css->SetTrackVol(cnt, file->Read_UINT8());

				css->SetMasterVol(song2.masterVol);

				// And track pans
				file->Seek(song2.trackPansOffs, PFile::pSeekBegin);

				for (cnt = 0; cnt < (uint32)songTracks; cnt++)
					css->SetTrackPan(cnt, file->Read_UINT8());

				// Read in section table
				file->Seek(song2.sectionTableOffs, PFile::pSeekBegin);

				for (cnt = 0; cnt < song2.numSections; cnt++)
					css->AppendNewSec(file->Read_B_UINT16());

				// Read playing sequences
				file->Seek(song2.playSeqTableOffs, PFile::pSeekBegin);

				pSqTbl = new uint32[song2.numPlaySeqs];
				if (pSqTbl == NULL)
					throw PMemoryException();

				try
				{
					uint32 cmdPtr, cnt2;
					uint16 seqLen, seqNum;
					char name[32];

					file->ReadArray_B_UINT32s(pSqTbl, song2.numPlaySeqs);

					for (cnt = 0; cnt < song2.numPlaySeqs; cnt++)
					{
						PlaySeq *newSeq = new PlaySeq();
						if (newSeq == NULL)
							throw PMemoryException();

						css->Append(newSeq);
						file->Seek(pSqTbl[cnt], PFile::pSeekBegin);

						// Read PlaySeq name
						file->Read(name, 32);
						name[31] = 0x00;

						// Get pointer to playseq commands
						cmdPtr = file->Read_B_UINT32();

						// Skip reserved fields
						file->Seek(4, PFile::pSeekCurrent);

						newSeq->SetName(name);

						// Read PlaySeq length
						seqLen = file->Read_B_UINT16();

						for (cnt2 = 0; cnt2 < seqLen; cnt2++)
						{
							seqNum = file->Read_B_UINT16();
							if (seqNum < 0x8000)
								newSeq->AddTail(new PlaySeqEntry(seqNum));
						}

						// Read commands, if any
						if (cmdPtr != 0)
						{
							uint16 offs;
							uint8 cmdNum, extraBytes;

							file->Seek(cmdPtr, PFile::pSeekBegin);

							for (;;)
							{
								offs       = file->Read_B_UINT16();
								cmdNum     = file->Read_UINT8();
								extraBytes = file->Read_UINT8();

								if ((offs == 0xffff) && (cmdNum == 0) && (extraBytes == 0))
									break;

								PlaySeqEntry *pse = newSeq->GetItem(offs);
								if (pse != NULL)
								{
									switch (cmdNum)
									{
										case PSEQCMD_STOP:
											pse->SetCmd(PSEQCMD_STOP, 0);

										default:
										{
											if (extraBytes != 0)
												file->Seek(extraBytes, PFile::pSeekCurrent);

											break;
										}

										case PSEQCMD_POSJUMP:
										{
											pse->SetCmd(PSEQCMD_POSJUMP, file->Read_B_UINT16());
											if (extraBytes > 2)
												file->Seek(extraBytes - 2, PFile::pSeekCurrent);

											break;
										}
									}
								}
							}
						}
					}
				}
				catch(...)
				{
					delete[] pSqTbl;
					throw;
				}

				delete[] pSqTbl;
			}

			if (file->IsEOF())
			{
				errorStr.LoadString(res, IDS_MED_ERR_LOADING_HEADER);
				throw PUserException();
			}

			// Read blocks
			file->Seek(currHdr->blocksOffs, PFile::pSeekBegin);

			if (mark <= '1')
			{
				// MMD0 or MMD1
				numBlocks = song0.numBlocks;
			}
			else
			{
				// MMD2 or MMD3
				numBlocks = song2.numBlocks;
			}

			blkArray = new uint32[numBlocks];
			if (blkArray == NULL)
				throw PMemoryException();

			try
			{
				// Read all the block pointers
				file->ReadArray_B_UINT32s(blkArray, numBlocks);

				if (mark == '0')
				{
					// MMD0
					for (cnt = 0; cnt < numBlocks; cnt++)
					{
						TRACK_NUM tracks, trkCnt;
						LINE_NUM lines, lineCnt;
						MED_Block *blk;

						// Seek to the block data
						file->Seek(blkArray[cnt], PFile::pSeekBegin);

						// Get the block header
						tracks      = file->Read_UINT8();
						lines       = file->Read_UINT8() + 1;
						numChannels = max(tracks, numChannels);		// TN fix

						// Allocate the block
						blk = new MED_Block(lines, tracks);
						if (blk == NULL)
							throw PMemoryException();

						sg->CurrSS()->Append(blk);

						for (lineCnt = 0; lineCnt < lines; lineCnt++)
						{
							for (trkCnt = 0; trkCnt < tracks; trkCnt++)
							{
								uint8 mmd0Note[3];
								MED_Note &dn = blk->Note(lineCnt, trkCnt);

								file->Read(mmd0Note, 3);
								dn.noteNum  = mmd0Note[0] & 0x3f;
								dn.instrNum = (mmd0Note[1] >> 4) | ((mmd0Note[0] & 0x80) ? 0x10 : 0x00) | ((mmd0Note[0] & 0x40) ? 0x20 : 0x00);

								if (!(mmd0Note[1] & 0x0f))
									blk->Cmd(lineCnt, trkCnt, 0).SetCmdData(0, 0, mmd0Note[2]);
								else
									blk->Cmd(lineCnt, trkCnt, 0).SetCmdData(mmd0Note[1] & 0x0f, mmd0Note[2], 0);
							}
						}
					}
				}
				else
				{
					// MMD1, MMD2, or MMD3
					MMD1BlockInfo blkInfo;

					for (cnt = 0; cnt < numBlocks; cnt++)
					{
						TRACK_NUM tracks, trkCnt;
						LINE_NUM lines, lineCnt;
						uint32 blockInfoOffs;
						uint32 skipTracks = 0;
						MED_Block *blk;

						// Seek to the block data
						file->Seek(blkArray[cnt], PFile::pSeekBegin);

						// Read the block header
						tracks        = file->Read_B_UINT16();
						lines         = file->Read_B_UINT16() + 1;
						blockInfoOffs = file->Read_B_UINT32();

						// More than 64 tracks.. skip the high tracks
						if (tracks > MAX_TRACKS)
						{
							skipTracks = tracks - MAX_TRACKS;
							tracks     = MAX_TRACKS;
						}

						numChannels = max(tracks, numChannels);		// TN fix

						// Read the block info
						if (blockInfoOffs != 0)
						{
							int64 notePos = file->GetPosition();
							file->Seek(blockInfoOffs, PFile::pSeekBegin);
							ReadMMD1BlockInfo(file, &blkInfo);

							// Back to notes
							file->Seek(notePos, PFile::pSeekBegin);
						}

						blk = new MED_Block(lines, tracks);
						if (blk == NULL)
							throw PMemoryException();

						sg->CurrSS()->Append(blk);

						for (lineCnt = 0; lineCnt < lines; lineCnt++)
						{
							for (trkCnt = 0; trkCnt < tracks; trkCnt++)
							{
								uint8 mmdNote[4];
								MED_Note &dn = blk->Note(lineCnt, trkCnt);
								file->Read(mmdNote, 4);

								if (mmdNote[0] <= NOTE_44k)
									dn.noteNum = mmdNote[0];

								dn.instrNum = mmdNote[1];

								// Convert cmds 00 and 19 (if only one cmd byte)
								if ((mmdNote[2] == 0x19) || (mmdNote[2] == 0x00))
									blk->Cmd(lineCnt, trkCnt, 0).SetCmdData(mmdNote[2], 0, mmdNote[3]);
								else
									blk->Cmd(lineCnt, trkCnt, 0).SetCmdData(mmdNote[2], mmdNote[3], 0);
							}

							if (skipTracks != 0)
								file->Seek(skipTracks * 4, PFile::pSeekCurrent);
						}

						if (blockInfoOffs != 0)
						{
							// Read and set block name
							if ((blkInfo.blockName != 0) && (blkInfo.blockNameLen != 0))
							{
								char *tmpTxt;

								file->Seek(blkInfo.blockName, PFile::pSeekBegin);
								tmpTxt = new char[blkInfo.blockNameLen];
								if (tmpTxt == NULL)
									throw PMemoryException();

								try
								{
									file->Read(tmpTxt, blkInfo.blockNameLen);
									blk->SetName(tmpTxt);
								}
								catch(...)
								{
									delete[] tmpTxt;
									throw;
								}

								delete[] tmpTxt;
							}

							// Read extra command pages
							if (blkInfo.pageTable != 0)
							{
								PAGE_NUM numPages, pageCnt;
								uint32 *pages;

								file->Seek(blkInfo.pageTable, PFile::pSeekBegin);

								numPages = file->Read_B_UINT16();
								blk->SetCmdPages(numPages + 1);

								// Skip reserved fields
								file->Seek(2, PFile::pSeekCurrent);

								pages = new uint32[numPages];
								if (pages == NULL)
									throw PMemoryException();

								try
								{
									uint8 cmdNum, cmdArg;

									file->ReadArray_B_UINT32s(pages, numPages);

									for (pageCnt = 0; pageCnt < numPages; pageCnt++)
									{
										file->Seek(pages[pageCnt], PFile::pSeekBegin);

										for (lineCnt = 0; lineCnt < lines; lineCnt++)
										{
											for (trkCnt = 0; trkCnt < tracks; trkCnt++)
											{
												cmdNum = file->Read_UINT8();
												cmdArg = file->Read_UINT8();

												// Convert cmds 00 and 19 (if only one cmd byte)
												if ((cmdNum == 0x19) || (cmdNum == 0x00))
													blk->Cmd(lineCnt, trkCnt, pageCnt + 1).SetCmdData(cmdNum, 0, cmdArg);
												else
													blk->Cmd(lineCnt, trkCnt, pageCnt + 1).SetCmdData(cmdNum, cmdArg, 0);
											}
										}
									}
								}
								catch(...)
								{
									delete[] pages;
									throw;
								}

								delete[] pages;
							}

							// Read extended command values
							if (blkInfo.cmdExtTable != 0)
							{
								// This song knows about the command 0c00xx kludge
								uint32 *cmdExt;

								file->Seek(blkInfo.cmdExtTable, PFile::pSeekBegin);

								cmdExt = new uint32[blk->Pages()];
								if (cmdExt == NULL)
									throw PMemoryException();

								try
								{
									uint8 arg2;
									PAGE_NUM pCnt;

									file->ReadArray_B_UINT32s(cmdExt, blk->Pages());

									for (pCnt = 0; pCnt < blk->Pages(); pCnt++)
									{
										file->Seek(cmdExt[pCnt], PFile::pSeekBegin);

										for (lineCnt = 0; lineCnt < lines; lineCnt++)
										{
											for (trkCnt = 0; trkCnt < tracks; trkCnt++)
											{
												MED_Cmd &cmd = blk->Cmd(lineCnt, trkCnt, pCnt);
												arg2         = file->Read_UINT8();

												if ((cmd.GetCmd() == 0x00) || (cmd.GetCmd() == 0x19))
													cmd.SetData(arg2);
												else
													cmd.SetData2(arg2);
											}
										}
									}
								}
								catch(...)
								{
									delete[] cmdExt;
									throw;
								}

								delete[] cmdExt;
							}
						}

						// Convert volume command 0C to handle the new 127 volume levels
						ScanConvertOldVolToNew cVol(usesHexVol);
						cVol.DoBlock(*blk);
					}
				}
			}
			catch(...)
			{
				delete[] blkArray;
				throw;
			}

			// Done with the block array
			delete[] blkArray;

			if (file->IsEOF())
			{
				errorStr.LoadString(res, IDS_MED_ERR_LOADING_HEADER);
				throw PUserException();
			}

			// Read ExpData
			if (currHdr->expDataOffs != 0)
			{
				file->Seek(currHdr->expDataOffs, PFile::pSeekBegin);
				ReadMMD0ExpData(file, &currExp);

				// Read EXP tags
				TagsLoad expTg(file);

				// Read InstExt
				for (cnt = 0; cnt < (uint32)min(currExp.insTextEntries, MAX_INSTRS); cnt++)
				{
					MMD_InstrExt currIE;
					Instr *ci;

					file->Seek(currExp.insTextOffs + cnt * currExp.insTextEntrySize, PFile::pSeekBegin);
					ci = sg->GetInstr(cnt);

					if (currExp.insTextEntrySize >= 2)
					{
						ci->SetHold(file->Read_UINT8());
						ci->SetDecay(file->Read_UINT8());
					}

					if (currExp.insTextEntrySize >= 4)
					{
						file->Read_UINT8();		// TN: Skip MIDI suppress
						ci->SetFineTune((int16)(int8)file->Read_UINT8());
					}

					if (currExp.insTextEntrySize >= 8)
					{
						currIE.defaultPitch   = file->Read_UINT8();
						currIE.instrFlags     = file->Read_UINT8();
						currIE.longMidiPreset = file->Read_B_UINT16();

						ci->SetDefPitch(currIE.defaultPitch);
						ci->i_flags |=  ((currIE.instrFlags & MMD_INSTRFLAG_LOOP) ? Instr::LOOP : 0) |
										((currIE.instrFlags & MMD_INSTRFLAG_EXTMIDIPSET) ? Instr::MIDI_EXT_PSET : 0) |
										((currIE.instrFlags & MMD_INSTRFLAG_DISABLED) ? Instr::DISABLED : 0) |
										((currIE.instrFlags & MMD_INSTRFLAG_PINGPONG) ? Instr::PINGPONG : 0);
					}
					else
					{
						if (ci->GetRepeatLen() > 2)
							ci->i_flags = Instr::LOOP;
					}

					if (currExp.insTextEntrySize >= 18)
					{
						currIE.outputDevice = file->Read_UINT8();
						currIE.reserved     = file->Read_UINT8();
						currIE.longRepeat   = file->Read_B_UINT32();
						currIE.longRepLen   = file->Read_B_UINT32();

						ci->SetRepeat(currIE.longRepeat);
						ci->SetRepeatLen(currIE.longRepLen);
					}

					if (currExp.insTextEntrySize >= 19)
					{
						uint8 vol = file->Read_UINT8();
						ci->SetVol(min(vol, 127));
					}

					if (currExp.insTextEntrySize >= 20)
					{
						file->Read_UINT8();		// TN: Skip the port number
					}

					if (currExp.insTextEntrySize >= 24)
					{
						file->Read_B_UINT32();	// TN: Skip the midi bank
					}

					if (file->IsEOF())
					{
						errorStr.LoadString(res, IDS_MED_ERR_LOADING_HEADER);
						throw PUserException();
					}
				}

				// Read InstInfo
				for (cnt = 0; cnt < (uint32)min(currExp.instInfoEntries, MAX_INSTRS); cnt++)
				{
					file->Seek(currExp.instInfoOffs + cnt * currExp.instInfoEntrySize, PFile::pSeekBegin);
					if (currExp.instInfoEntrySize >= 40)
					{
						char sName[42];
						file->Read(sName, 40);
						sg->GetInstr(cnt)->SetName(sName);
					}

					if (file->IsEOF())
					{
						errorStr.LoadString(res, IDS_MED_ERR_LOADING_HEADER);
						throw PUserException();
					}
				}

				// Read song's name
				if (currExp.songNameOffs != 0)
				{
					char *tmpTxt;

					file->Seek(currExp.songNameOffs, PFile::pSeekBegin);
					tmpTxt = new char[currExp.songNameLen];
					if (tmpTxt == NULL)
						throw PMemoryException();

					try
					{
						file->Read(tmpTxt, currExp.songNameLen);
						css->SetSongName(tmpTxt);
					}
					catch(...)
					{
						delete[] tmpTxt;
						throw;
					}

					delete[] tmpTxt;

					if (file->IsEOF())
					{
						errorStr.LoadString(res, IDS_MED_ERR_LOADING_HEADER);
						throw PUserException();
					}
				}

				// Channel split in 5-8 channel modules
				if (eightChConv)
				{
					for (cnt = 0; cnt < 4; cnt++)
					{
						if (!currExp.channelSplit[cnt])
							css->SetNumChannels(css->GetNumChannels() + 1);
					}
				}

				// Read annotation text
				if (currExp.annoTextOffs != 0)
				{
					char *loadText;

					file->Seek(currExp.annoTextOffs, PFile::pSeekBegin);
					loadText = new char[currExp.annoTextLength];
					if (loadText == NULL)
						throw PMemoryException();

					try
					{
						file->Read(loadText, currExp.annoTextLength);
						sg->SetAnnoText(loadText);
					}
					catch(...)
					{
						delete[] loadText;
						throw;
					}

					delete[] loadText;

					if (file->IsEOF())
					{
						errorStr.LoadString(res, IDS_MED_ERR_LOADING_HEADER);
						throw PUserException();
					}
				}

				// Effect groups
				// TN: Not supported yet!
//XX				if (expTg.TagVal(MMDTAG_EXP_NUMFXGROUPS, 1) > 1)
/*				{
					uint32 numGrps;

					numGrps = expTg.TagVal(MMDTAG_EXP_NUMFXGROUPS);
					for (cnt = 1; cnt < numGrps; cnt++)
						css->fx.AddGroup();
				}

				if (currExp.effectInfoOffs != 0)
				{
					uint32 numGrps;
					uint32 *grpPos;

					file->Seek(currExp.effectInfoOffs, PFile::pSeekBegin);
					numGrps = css->fx.GetNumGroups();
					grpPos  = new uint32[numGrps];
					if (grpPos == NULL)
						throw PMemoryException();

					try
					{
						file->ReadArray_B_UINT32s(grpPos, numGrps);

						for (cnt = 0; cnt < numGrps; cnt++)
						{
							if (grpPos[cnt] != 0)
							{
								uint32 val;
								int8 stSep;

								file->Seek(grpPos[cnt], PFile::pSeekBegin);
								TagsLoad tg(file);
								EffectGroup &grp = css->fx.GetGroup(cnt);

								if (tg.TagExists(MMDTAG_FX_GROUPNAME) && tg.TagExists(MMDTAG_FX_GRPNAMELEN))
								{
									char tmpBuff[80];

									tg.SeekToTag(MMDTAG_FX_GROUPNAME);
									file->Read(tmpBuff, min(tg.TagVal(MMDTAG_FX_GRPNAMELEN), 80));
									tmpBuff[79] = 0;
									grp.SetName(tmpBuff);
								}

								val = tg.TagVal(MMDTAG_FX_ECHOTYPE, 0);
								if (val > 2)
									val = 0;		// Any future types ignored

								grp.SetEchoMode((EffectGroup::ECHO)val);
								grp.SetEchoLength((uint16)tg.TagVal(MMDTAG_FX_ECHOLEN, 300));
								grp.SetEchoDepth((uint8)tg.TagVal(MMDTAG_FX_ECHODEPTH, 2));

								stSep = (int8)((int32)tg.TagVal(MMDTAG_FX_STEREOSEP, 0));
								grp.SetStereoSeparation(stSep);
//								tg.CheckUnused(warn);		// TN: We do not use this
							}
						}
					}
					catch(...)
					{
						delete[] grpPos;
						throw;
					}

					delete[] grpPos;

					if (file->IsEOF())
					{
						errorStr.LoadString(res, IDS_MED_ERR_LOADING_HEADER);
						throw PUserException();
					}
				}

				// Read track info
				if (currExp.trackInfoOffs != 0)
				{
					uint32 *trkPos;

					file->Seek(currExp.trackInfoOffs, PFile::pSeekBegin);
					trkPos = new uint32[songTracks];
					if (trkPos == NULL)
						throw PMemoryException();

					try
					{
						file->ReadArray_B_UINT32s(trkPos, songTracks);

						for (cnt = 0; cnt < (uint32)songTracks; cnt++)
						{
							if (trkPos[cnt] != 0)
							{
								file->Seek(trkPos[cnt], PFile::pSeekBegin);
								TagsLoad tg(file);

								if (tg.TagExists(MMDTAG_TRK_NAME) && (tg.TagExists(MMDTAG_TRK_NAMELEN)))
								{
									char tmpBuff[82];

									tg.SeekToTag(MMDTAG_TRK_NAME);
									file->Read(tmpBuff, min(tg.TagVal(MMDTAG_TRK_NAMELEN), 81));
									tmpBuff[81] = 0;
									css->SetTrackName(cnt, tmpBuff);
								}

								css->fx.SetTrackGroup(cnt, tg.TagVal(MMDTAG_TRK_FXGROUP));
//								tg.CheckUnused(warn);		// TN: We do not use this
							}
						}
					}
					catch(...)
					{
						delete[] trkPos;
						throw;
					}

					delete[] trkPos;

					if (file->IsEOF())
					{
						errorStr.LoadString(res, IDS_MED_ERR_LOADING_HEADER);
						throw PUserException();
					}
				}
*/
				file->Seek(currExp.nextHdr, PFile::pSeekBegin);
				currHdr = &hdrX;		// Don't overwrite the first MMD header
			}
			else
			{
				if (sg->CurrInstr()->GetRepeatLen() > 2)
					sg->CurrInstr()->i_flags = Instr::LOOP;
			}
		}
		while (hdr0.extraSongs--);

		// Read samples etc.
		if ((hdr0.samplesOffs != 0) && (numSamples != 0))
		{
			uint32 cnt, where;
			uint32 *smpArray;
			Sample *dest;

			file->Seek(hdr0.samplesOffs, PFile::pSeekBegin);

			smpArray = new uint32[numSamples];
			if (smpArray == NULL)
				throw PMemoryException();

			try
			{
				dest = NULL;
				file->ReadArray_B_UINT32s(smpArray, numSamples);

				for (cnt = 0; cnt < numSamples; cnt++)
				{
					dest = NULL;

					if (smpArray[cnt] == 0)
						continue;

					where = smpArray[cnt];
					file->Seek(where, PFile::pSeekBegin);

					MMDSampleHdr sHdr(this, file, errorStr, res);

					if (sHdr.IsStereo())
					{
						errorStr.LoadString(res, IDS_MED_ERR_HAVE_STEREO_SAMPLE);
						throw PUserException();
					}

					if (sHdr.IsSample())
					{
						dest = sHdr.AllocSample();
						if (dest == NULL)
							throw PMemoryException();

						sHdr.ReadSampleData(file, dest);
						sg->SetSample(cnt, dest);
					}
					else
					{
						if (sHdr.IsSynth() || sHdr.IsHybrid())
							sg->ReadSynthSound(cnt, file, errorStr, res, sHdr.IsHybrid());
					}

					if (file->IsEOF())
					{
						errorStr.LoadString(res, IDS_MED_ERR_LOADING_SAMPLES);
						throw PUserException();
					}
				}
			}
			catch(...)
			{
				delete dest;
				delete[] smpArray;
				throw;
			}

			delete[] smpArray;
		}

		// Do the conversion for old 4-8 channel modules
		if (mixConv)
		{
			ScanSongConvertToMixMode cmm;
			cmm.Do(*sg, mark == '0');
		}

		// And special 5-8 channel tempo conversion
		if (eightChConv)
		{
			ScanSongConvertTempo ct;
			ct.DoSong(*sg);
		}

		// Everything is loaded alright
		retVal = AP_OK;
	}
	catch(PMemoryException e)
	{
		// Out of memory
		errorStr.LoadString(res, IDS_MED_ERR_MEMORY);
		Cleanup();
	}
	catch(PUserException e)
	{
		// Just delete the exception and clean up
		Cleanup();
	}
	catch(...)
	{
		// Clean up
		Cleanup();
		throw;
	}

	return (retVal);
}



/******************************************************************************/
/* InitPlayer() initialize the player.                                        */
/*                                                                            */
/* Input:  "index" is the player index number.                                */
/*                                                                            */
/* Output: True for success, false for an error.                              */
/******************************************************************************/
bool OctaMED::InitPlayer(int32 index)
{
	SongTime *songTime;
	PosInfo posInfo;
	SubSong *ss;
	uint32 numSongs, numSect, numSeq, repeatLine, loopCount;
	int32 loopLine;
	LINE_NUM numLines, line, startLine, newLine;
	TRACK_NUM numTracks, track;
	PAGE_NUM numPages, page;
	uint32 i, j, k;
	bool pattBreak, stopPlaying;
	float total;

	// Get the number of subsongs
	numSongs = sg->NumSubSongs();

	// Now loop all the songs
	for (i = 0; i < numSongs; i++)
	{
		// Allocate a new sub song structure
		songTime = new SongTime;
		if (songTime == NULL)
			return (false);

		// Get the subsong pointer
		ss = sg->GetSubSong(i);

		// Initialize the start tempo
		posInfo.tempo = ss->GetTempo();
		ss->SetStartTempo(posInfo.tempo);
		plr->SetMixTempo(posInfo.tempo);

		// Initialize the start variables
		total       = 0.0f;
		loopCount   = 0;
		loopLine    = 0;
		startLine   = 0;
		newLine     = -1;
		stopPlaying = false;

		// Get the number of sections
		numSect = ss->NumSections();

		for (j = 0; j < numSect; j++)
		{
			// Now get the play sequence
			PlaySeq &playSeq = ss->PSeq(ss->Sect(j));
			numSeq = playSeq.CountItems();

			for (k = 0; k < numSeq; k++)
			{
				// Set the position information
				posInfo.time.SetTimeSpan(total);

				if (k >= (uint32)songTime->posInfoList.CountItems())
					songTime->posInfoList.AddTail(posInfo);

				// Get the number of lines
				PlaySeqEntry &pse = *playSeq.GetItem(k);
				MED_Block &block  = ss->Block((BLOCK_NUM)pse);
				numLines          = block.Lines();
				numTracks         = block.Tracks();
				numPages          = block.Pages();

				for (line = startLine; line < numLines; line++)
				{
					// Initialize sequence variables
					startLine  = 0;
					repeatLine = 1;
					pattBreak  = false;

					for (track = 0; track < numTracks; track++)
					{
						for (page = 0; page < numPages; page++)
						{
							MED_Cmd &cmd = block.Cmd(line, track, page);
							uint8 data   = cmd.GetDataB();

							switch (cmd.GetCmd())
							{
								// Set second tempo
								case 0x09:
								{
									posInfo.tempo.ticksPerLine = data & 0x1f;
									plr->SetMixTempo(posInfo.tempo);
									break;
								}

								// Jump to play sequence
								case 0x0b:
								{
									if (data <= k)
										stopPlaying = true;

									k         = data - 1;
									pattBreak = true;
									break;
								}

								// Misc
								case 0x0f:
								{
									if (data == 0x00)
									{
										// Pattern break
										startLine = 0;
										pattBreak = true;
									}
									else if (data == 0xfe)
									{
										// Stop playing
										stopPlaying = true;
									}
									else if (data <= 0xf0)
									{
										// Change tempo
										posInfo.tempo.tempo = data;
										plr->SetMixTempo(posInfo.tempo);
									}
									break;
								}

								// Loop block
								case 0x16:
								{
									if (data != 0)
									{
										if (loopCount == 0)
											loopCount = data;		// Init loop
										else
										{
											if (--loopCount == 0)
												break;				// Continue
										}

										newLine = loopLine;			// Jump to beginning of loop
									}
									else
										loopLine = line;			// Store line number

									break;
								}

								// Pattern break with new line
								case 0x1d:
								{
									startLine = data;
									pattBreak = true;
									break;
								}

								// Repeat line
								case 0x1e:
								{
									repeatLine = data + 1;
									break;
								}
							}
						}
					}

					// Add the line time to the total
					total += (repeatLine * (1.0f / playFreq) * posInfo.tempo.ticksPerLine * 1000.0f);

					if (pattBreak || stopPlaying)
						break;

					if (newLine != -1)
					{
						line    = newLine - 1;
						newLine = -1;
					}
				}

				if (stopPlaying)
					break;
			}

			if (stopPlaying)
				break;
		}

		// Set the total time
		songTime->totalTime.SetTimeSpan(total);

		// And add the song time in the list
		songTimeList.AddTail(songTime);
	}

	return (true);
}



/******************************************************************************/
/* EndPlayer() ends the use of the player.                                    */
/*                                                                            */
/* Input:  "index" is the player index number.                                */
/******************************************************************************/
void OctaMED::EndPlayer(int32 index)
{
	Cleanup();
}



/******************************************************************************/
/* InitSound() initialize the current song.                                   */
/*                                                                            */
/* Input:  "index" is the player index number.                                */
/*         "songNum" is the subsong to play.                                  */
/******************************************************************************/
void OctaMED::InitSound(int32 index, uint16 songNum)
{
	SubSong *ss;

	// Set the current song
	sg->SetSSNum(songNum);
	currentSong = songNum;

	// Now set the tempo in the player
	ss = sg->CurrSS();
	plr->SetMixTempo(ss->GetStartTempo());

	// Initialize the player
	plr->PlaySong(sg);
}



/******************************************************************************/
/* Play() is the main player function.                                        */
/******************************************************************************/
void OctaMED::Play(void)
{
	plr->PlrCallBack();
}



/******************************************************************************/
/* GetModuleName() returns the name of the module.                            */
/*                                                                            */
/* Output: Is the module name.                                                */
/******************************************************************************/
PString OctaMED::GetModuleName(void)
{
	return (sg->CurrSS()->GetSongName());
}



/******************************************************************************/
/* GetModuleChannels() returns the number of channels the module use.         */
/*                                                                            */
/* Output: Is the number of channels.                                         */
/******************************************************************************/
uint16 OctaMED::GetModuleChannels(void)
{
	return (numChannels);
}



/******************************************************************************/
/* GetSubSongs() returns the number of sub songs the module have.             */
/*                                                                            */
/* Output: Is a pointer to a subsong array.                                   */
/******************************************************************************/
const uint16 *OctaMED::GetSubSongs(void)
{
	songTab[0] = sg->NumSubSongs();			// Number of subsongs
	songTab[1] = 0;							// Default start song

	return (songTab);
}



/******************************************************************************/
/* GetSongLength() returns the length of the current song.                    */
/*                                                                            */
/* Output: Is the length of the current song.                                 */
/******************************************************************************/
int16 OctaMED::GetSongLength(void)
{
	SubSong *ss;
	uint32 i, numSec;
	PSEQ_NUM seq;
	int16 len = 0;

	ss     = sg->CurrSS();
	numSec = ss->NumSections();

	for (i = 0; i < numSec; i++)
	{
		seq  = ss->Sect(i);
		len += ss->PSeq(seq).CountItems();
	}

	return (len);
}



/******************************************************************************/
/* GetSongPosition() returns the current position of the playing song.        */
/*                                                                            */
/* Output: Is the current position.                                           */
/******************************************************************************/
int16 OctaMED::GetSongPosition(void)
{
	SubSong *ss;
	uint32 i, sectNum;
	PSEQ_NUM seq;
	int16 pos = 0;

	ss      = sg->CurrSS();
	sectNum = plr->plrPos.PSectPos();

	for (i = 0; i < sectNum; i++)
	{
		seq  = ss->Sect(i);
		pos += ss->PSeq(seq).CountItems();
	}

	return (pos + plr->plrPos.PSeqPos());
}



/******************************************************************************/
/* SetSongPosition() sets the current position of the playing song.           */
/*                                                                            */
/* Input:  "pos" is the new position.                                         */
/******************************************************************************/
void OctaMED::SetSongPosition(int16 pos)
{
	SongTime *songTime;
	PosInfo posInfo;
	SubSong *ss;
	uint32 i, numSec, seqNum;
	PSEQ_NUM seq;

	ss     = sg->CurrSS();
	numSec = ss->NumSections();

	// Calculate the different sequence, section etc. positions
	for (i = 0; i < numSec; i++)
	{
		seq    = ss->Sect(i);
		seqNum = ss->PSeq(seq).CountItems();

		if ((uint32)pos < seqNum)
			break;

		pos -= seqNum;
	}

	// Now set the player position
	plr->plrPos.PSectPos(i);
	plr->plrPos.PSeqPos(pos);

	seq = ss->Sect(i);
	PlaySeqEntry &pse = *ss->PSeq(seq).GetItem(pos);
	plr->plrPos.Block((BLOCK_NUM)pse);

	// Get the song time structure
	songTime = songTimeList.GetItem(currentSong);

	// Change the tempo
	posInfo = songTime->posInfoList.GetItem(pos);
	sg->CurrSS()->SetTempo(posInfo.tempo);
	plr->ChangePlayFreq();
}



/******************************************************************************/
/* GetTimeTable() will calculate the position time for each position and      */
/*      store them in the list given.                                         */
/*                                                                            */
/* Input:  "songNum" is the subsong number to get the time table for.         */
/*         "posTimes" is a reference to the list where you should store the   */
/*         start time for each position.                                      */
/*                                                                            */
/* Output: The total module time or 0 if time table is not supported.         */
/******************************************************************************/
PTimeSpan OctaMED::GetTimeTable(uint16 songNum, PList<PTimeSpan> &posTimes)
{
	SongTime *songTime;
	int32 i, count;

	// Get the song time structure
	songTime = songTimeList.GetItem(songNum);

	// Copy the position times
	count = songTime->posInfoList.CountItems();
	for (i = 0; i < count; i++)
		posTimes.AddTail(songTime->posInfoList.GetItem(i).time);

	return (songTime->totalTime);
}



/******************************************************************************/
/* GetInfoString() returns the description and value string on the line       */
/*      given. If the line is out of range, false is returned.                */
/*                                                                            */
/* Input:  "line" is the line starting from 0.                                */
/*         "description" is a reference to where to store the description.    */
/*         "value" is a reference to where to store the value.                */
/*                                                                            */
/* Output: True if the information are returned, false if not.                */
/******************************************************************************/
bool OctaMED::GetInfoString(uint32 line, PString &description, PString &value)
{
	// Is the line number out of range?
	if (line >= 4)
		return (false);

	// Find out which line to take
	switch (line)
	{
		// Song Length
		case 0:
		{
			description.LoadString(res, IDS_MED_INFODESCLINE0);
			value.SetUNumber(GetSongLength());
			break;
		}

		// Used Blocks
		case 1:
		{
			description.LoadString(res, IDS_MED_INFODESCLINE1);
			value.SetUNumber(sg->CurrSS()->NumBlocks());
			break;
		}

		// Used Samples
		case 2:
		{
			description.LoadString(res, IDS_MED_INFODESCLINE2);
			value.SetUNumber(numSamples);
			break;
		}

		// Actual Speed (Hz)
		case 3:
		{
			description.LoadString(res, IDS_MED_INFODESCLINE3);
			value.Format("%.02f", playFreq);
			break;
		}
	}

	return (true);
}



/******************************************************************************/
/* GetSampleInfo() fills out the APSampleInfo structure given with the sample */
/*      information of the sample number given.                               */
/*                                                                            */
/* Input:  "num" is the sample number starting from 0.                        */
/*         "info" is a pointer to an APSampleInfo structure to fill out.      */
/*                                                                            */
/* Output: True if the information are returned, false if not.                */
/******************************************************************************/
bool OctaMED::GetSampleInfo(uint32 num, APSampleInfo *info)
{
	Instr *inst;
	Sample *sample;

	// First check the sample number for "out of range"
	if (num >= numSamples)
		return (false);

	// Get the pointer to the sample data
	inst   = sg->GetInstr(num);
	sample = inst->GetSample();
	if (sample == NULL)
	{
		// Well, fill out an empty sample
		info->name.MakeEmpty();
		info->flags      = 0;
		info->type       = apSample;
		info->bitSize    = 8;
		info->middleC    = plr->GetNoteFrequency(36, inst->GetFineTune());
		info->volume     = inst->GetVol() * 2;
		info->panning    = -1;
		info->address    = NULL;
		info->length     = 0;
		info->loopStart  = inst->GetRepeat();
		info->loopLength = inst->GetRepeatLen();

		return (true);
	}

	// Find out the type of the sample
	if (sample->IsSynthSound())
	{
		if (sample->GetLength() == 0)
			info->type = apSynth;
		else
			info->type = apHybrid;
	}
	else
		info->type = apSample;

	// Find out the loop information
	info->flags = (inst->i_flags & Instr::LOOP ? APSAMP_LOOP : 0);
	if (inst->i_flags & Instr::PINGPONG)
		info->flags |= APSAMP_PINGPONG;

	// Fill out the sample info structure
	info->name       = inst->GetName();
	info->bitSize    = sample->Is16Bit() ? 16 : 8;
	info->middleC    = plr->GetNoteFrequency(36, inst->GetFineTune());
	info->volume     = inst->GetVol() * 2;
	info->panning    = -1;
	info->address    = sample->GetSampleAddress(0);
	info->length     = sample->GetLength();
	info->loopStart  = inst->GetRepeat();
	info->loopLength = inst->GetRepeatLen();

	return (true);
}



/******************************************************************************/
/* TestModule() tests the module to see which type of module it is.           */
/*                                                                            */
/* Input:  "file" is a pointer to the file object with the file to check.     */
/*                                                                            */
/* Output: medUnknown for unknown or the module type.                         */
/******************************************************************************/
MedType OctaMED::TestModule(PFile *file)
{
	uint32 temp;

	// First check the length
	if (file->GetLength() < 840)
		return (medUnknown);

	// Now check the mark
	file->SeekToBegin();
	mark = file->Read_B_UINT32();

	// Check the mark
	if ((mark & 0xffffff00) != 'MMD\0')
		return (medUnknown);

	// Mask out the mark and leave the version
	mark &= 0x000000ff;

	if ((mark < '0') || (mark > '3'))
		return (medUnknown);

	// Okay, check the module length
	temp = file->Read_B_UINT32();
	if (temp > file->GetLength())
		return (medUnknown);

	if (mark == '0')
	{
		// Well, it's either a MED or OctaMED module, find out which one
		//
		// Seek to the song structure + skip until the flags argument
		temp = file->Read_B_UINT32();
		file->Seek(temp + 767, PFile::pSeekBegin);

		if (file->Read_UINT8() & 0x40)
			return (medOctaMED);
		else
			return (medMED);
	}

	if (mark == '1')
		return (medOctaMED_Professional4);

	if (mark == '2')
		return (medOctaMED_Professional6);

	if (mark == '3')
		return (medOctaMED_SoundStudio);

	return (medUnknown);
}



/******************************************************************************/
/* Cleanup() frees all the memory the player have allocated.                  */
/******************************************************************************/
void OctaMED::Cleanup(void)
{
	int32 i, count;

	// Delete the player
	delete plr;
	plr = NULL;

	// Delete the song
	delete sg;
	sg = NULL;

	// Empty the song time list
	count = songTimeList.CountItems();
	for (i = 0; i < count; i++)
		delete songTimeList.GetItem(i);

	songTimeList.MakeEmpty();
}



/******************************************************************************/
/* ReadMMDHeader() reads all the field in the MMD header.                     */
/*                                                                            */
/* Input:  "file" is where to read from.                                      */
/*         "header" is a pointer to where to store the values.                */
/******************************************************************************/
void OctaMED::ReadMMDHeader(PFile *file, MMD_Hdr *header)
{
	header->id          = file->Read_B_UINT32();
	header->modLen      = file->Read_B_UINT32();
	header->songOffs    = file->Read_B_UINT32();
	header->pSecNum     = file->Read_B_UINT16();
	header->pSeq        = file->Read_B_UINT16();
	header->blocksOffs  = file->Read_B_UINT32();
	header->mmdFlags    = file->Read_UINT8();
	file->Seek(3, PFile::pSeekCurrent);
	header->samplesOffs = file->Read_B_UINT32();
	file->Seek(4, PFile::pSeekCurrent);
	header->expDataOffs = file->Read_B_UINT32();
	file->Seek(4, PFile::pSeekCurrent);
	header->pState      = file->Read_B_UINT16();
	header->pBlock      = file->Read_B_UINT16();
	header->pLine       = file->Read_B_UINT16();
	header->pSeqNum     = file->Read_B_UINT16();
	header->actPlayLine = file->Read_B_UINT16();
	header->counter     = file->Read_UINT8();
	header->extraSongs  = file->Read_UINT8();
}



/******************************************************************************/
/* ReadMMD0Sample() reads all the field in the MMD0 sample structure.         */
/*                                                                            */
/* Input:  "file" is where to read from.                                      */
/*         "sample" is a pointer to where to store the values.                */
/******************************************************************************/
void OctaMED::ReadMMD0Sample(PFile *file, MMD0Sample *sample)
{
	sample->rep        = file->Read_B_UINT16();
	sample->repLen     = file->Read_B_UINT16();
	sample->midiCh     = file->Read_UINT8();
	sample->midiPreset = file->Read_UINT8();
	sample->volume     = file->Read_UINT8();
	sample->sTrans     = file->Read_UINT8();
}



/******************************************************************************/
/* ReadMMD0Song() reads all the field in the MMD0 song structure.             */
/*                                                                            */
/* Input:  "file" is where to read from.                                      */
/*         "song" is a pointer to where to store the values.                  */
/******************************************************************************/
void OctaMED::ReadMMD0Song(PFile *file, MMD0SongData *song)
{
	song->numBlocks  = file->Read_B_UINT16();
	song->songLen    = file->Read_B_UINT16();
	file->Read(song->playSeq, 256);
	song->defTempo   = file->Read_B_UINT16();
	song->playTransp = file->Read_UINT8();
	song->flags      = file->Read_UINT8();
	song->flags2     = file->Read_UINT8();
	song->tempo2     = file->Read_UINT8();
	file->Read(song->trkVol, 16);
	song->masterVol  = file->Read_UINT8();
	song->numSamples = file->Read_UINT8();
}



/******************************************************************************/
/* ReadMMD0ExpData() reads all the field in the MMD0 expansion structure.     */
/*                                                                            */
/* Input:  "file" is where to read from.                                      */
/*         "expData" is a pointer to where to store the values.               */
/******************************************************************************/
void OctaMED::ReadMMD0ExpData(PFile *file, MMD0ExpData *expData)
{
	expData->nextHdr           = file->Read_B_UINT32();
	expData->insTextOffs       = file->Read_B_UINT32();
	expData->insTextEntries    = file->Read_B_UINT16();
	expData->insTextEntrySize  = file->Read_B_UINT16();
	expData->annoTextOffs      = file->Read_B_UINT32();
	expData->annoTextLength    = file->Read_B_UINT32();
	expData->instInfoOffs      = file->Read_B_UINT32();
	expData->instInfoEntries   = file->Read_B_UINT16();
	expData->instInfoEntrySize = file->Read_B_UINT16();
	expData->obsolete0         = file->Read_B_UINT32();
	expData->obsolete1         = file->Read_B_UINT32();
	file->Read(expData->channelSplit, 4);
	expData->notInfoOffs       = file->Read_B_UINT32();
	expData->songNameOffs      = file->Read_B_UINT32();
	expData->songNameLen       = file->Read_B_UINT32();
	expData->dumpsOffs         = file->Read_B_UINT32();
	expData->mmdInfoOffs       = file->Read_B_UINT32();
	expData->mmdARexxOffs      = file->Read_B_UINT32();
	expData->mmdCmd3xOffs      = file->Read_B_UINT32();
	expData->trackInfoOffs     = file->Read_B_UINT32();
	expData->effectInfoOffs    = file->Read_B_UINT32();
}



/******************************************************************************/
/* ReadMMD1BlockInfo() reads all the field in the MMD1 block info structure.  */
/*                                                                            */
/* Input:  "file" is where to read from.                                      */
/*         "blockInfo" is a pointer to where to store the values.             */
/******************************************************************************/
void OctaMED::ReadMMD1BlockInfo(PFile *file, MMD1BlockInfo *blockInfo)
{
	blockInfo->hlMask       = file->Read_B_UINT32();
	blockInfo->blockName    = file->Read_B_UINT32();
	blockInfo->blockNameLen = file->Read_B_UINT32();
	blockInfo->pageTable    = file->Read_B_UINT32();
	blockInfo->cmdExtTable  = file->Read_B_UINT32();
}



/******************************************************************************/
/* ReadMMD2Song() reads all the field in the MMD2 song structure.             */
/*                                                                            */
/* Input:  "file" is where to read from.                                      */
/*         "song" is a pointer to where to store the values.                  */
/******************************************************************************/
void OctaMED::ReadMMD2Song(PFile *file, MMD2SongData *song)
{
	song->numBlocks        = file->Read_B_UINT16();
	song->numSections      = file->Read_B_UINT16();
	song->playSeqTableOffs = file->Read_B_UINT32();
	song->sectionTableOffs = file->Read_B_UINT32();
	song->trackVolsOffs    = file->Read_B_UINT32();
	song->numTracks        = file->Read_B_UINT16();
	song->numPlaySeqs      = file->Read_B_UINT16();
	song->trackPansOffs    = file->Read_B_UINT32();
	song->flags3           = file->Read_B_UINT32();
	song->volAdj           = file->Read_B_UINT16();
	song->channels         = file->Read_B_UINT16();
	song->mixEchoType      = file->Read_UINT8();
	song->mixEchoDepth     = file->Read_UINT8();
	song->mixEchoLen       = file->Read_B_UINT16();
	song->mixStereoSep     = file->Read_UINT8();
	file->Seek(223, PFile::pSeekCurrent);
	song->defTempo         = file->Read_B_UINT16();
	song->playTransp       = file->Read_UINT8();
	song->flags            = file->Read_UINT8();
	song->flags2           = file->Read_UINT8();
	song->tempo2           = file->Read_UINT8();
	file->Seek(16, PFile::pSeekCurrent);
	song->masterVol        = file->Read_UINT8();
	song->numSamples       = file->Read_UINT8();
}
