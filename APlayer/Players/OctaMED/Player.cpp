/******************************************************************************/
/* Player Interface.                                                          */
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
#include "MEDTypes.h"
#include "OctaMED.h"
#include "Block.h"
#include "Instr.h"
#include "PlayPosition.h"
#include "Song.h"
#include "SubSong.h"
#include "Player.h"


/******************************************************************************/
/* Tables                                                                     */
/******************************************************************************/
const int8 Player::sineTable[32] =
{
	0, 25, 49, 71, 90, 106, 117, 125, 127, 125, 117, 106, 90, 71, 49, 25,
	0, -25, -49, -71, -90, -106, -117, -125, -127, -125, -117, -106, -90, -71, -49, -25
};



/******************************************************************************/
/* Constructor                                                                */
/******************************************************************************/
Player::Player(OctaMED *octaMED) : MED_Mixer(octaMED), plrPos(octaMED)
{
	TRACK_NUM cnt;

	memset(td, 0, sizeof(TrackData) * MAX_TRACKS);

	for (cnt = 0; cnt < MAX_TRACKS; cnt++)
		EnableTrack(cnt, true);

	plrSong = NULL;
}



/******************************************************************************/
/* Destructor                                                                 */
/******************************************************************************/
Player::~Player(void)
{
}



/******************************************************************************/
/* PlaySong() initialize the player so it's ready to play the song.           */
/*                                                                            */
/* Input:  "song" is a pointer to the song to play.                           */
/******************************************************************************/
void Player::PlaySong(Song *song)
{
	PlayPosition pos(med);
	SubSong *css;
	uint32 cnt;

	for (cnt = 0; cnt < MAX_TRACKS; cnt++)
	{
		memset(&td[cnt], 0, sizeof(TrackData));
		td[cnt].trkNoteOffCnt = -1;
	}

	css = song->CurrSS();

	plrBlockDelay    = 0;
	plrSong          = song;
	plrFxLine        = 0;
	plrFxBlock       = 0;
	plrNextLine      = 0;						// TN: Added
	plrBreak         = normal;
	plrRepeatLine    = 0;
	plrRepeatCounter = 0;
	plrDelayedStop   = false;
	plrChannels      = css->GetNumChannels();
	plrPulseCtr      = css->GetTempoTPL();		// To make sure that a new note is immediately triggered
	med->amigaFilter = css->GetAmigaFilter();	// TN: Added filter support

	pos = css->Pos();
	pos = PlayPosition::songStart;
	pos.SetAdvMode(PlayPosition::advSong);
	css->SetPos(pos);

	plrPos = css->Pos();		// Copy for our private use

//XX	SetEffectMaster(&css->fx);

	Start(plrChannels);
}



/******************************************************************************/
/* PlrCallBack() is the main player routine.                                  */
/******************************************************************************/
void Player::PlrCallBack(void)
{
	PAGE_NUM pageCnt;
	TRACK_NUM trkCnt;
	MED_Block *blk;

	SubSong *ss   = plrSong->CurrSS();
	LINE_NUM line = plrPos.Line();

	// Advance & check pulse counter
	++plrPulseCtr;
	if (plrPulseCtr >= ss->GetTempoTPL())
	{
		blk         = &ss->Block(plrPos.Block());
		plrPulseCtr = 0;

		if (plrDelayedStop)
		{
			// End of module
			med->endReached = true;
		}

		if ((plrBlockDelay != 0) && (--plrBlockDelay != 0))
			;	// Dont get a new line
		else
		{
			// Read in notes
			for (trkCnt = 0; trkCnt < blk->Tracks(); trkCnt++)
			{
				TrackData &trkd = td[trkCnt];
				if (!IsTrackEnabled(trkCnt))
				{
					// Muted, skip!
					trkd.trkFxType = TrackData::noPlay;
					continue;	// Next track, please!
				}

				trkd.trkFxType  = TrackData::normal;
				MED_Note &currN = blk->Note(line, trkCnt);

				// Get current note of this track
				if ((currN.noteNum >= NOTE_DEF) && (currN.noteNum <= NOTE_44k))
					trkd.trkCurrNote = currN.noteNum;
				else
				{
					if ((trkd.trkCurrNote = currN.noteNum) > 0x7f)
						trkd.trkCurrNote = 0;
				}

				// If not empty, set trkPrevNote also
				if (trkd.trkCurrNote != 0)
					trkd.trkPrevNote = trkd.trkCurrNote;

				if ((currN.instrNum != 0) && (currN.instrNum <= MAX_INSTRS))
				{
					trkd.trkPrevINum = currN.instrNum - 1;
					Instr *currI = plrSong->GetInstr(trkd.trkPrevINum);
					ExtractInstrData(trkd, currI);
				}

				// STP? (ExtractInstrData clears miscFlags)
				if (currN.noteNum == NOTE_STP)
					trkd.trkMiscFlags |= TrackData::STOPNOTE;
			}

			// Pre-fx
			for (pageCnt = 0; pageCnt < blk->Pages(); pageCnt++)
			{
				for (trkCnt = 0; trkCnt < blk->Tracks(); trkCnt++)
				{
					TrackData &trkd = td[trkCnt];
					if (trkd.trkFxType == TrackData::none)
						continue;

					MED_Cmd &cmd = blk->Cmd(line, trkCnt, pageCnt);
					uint8 data   = cmd.GetDataB();
					uint16 dataW = cmd.GetData();

					switch (cmd.GetCmd())
					{
						// Portamento
						case 0x03:
						{
							if (trkd.trkCurrNote != 0)
							{
								NOTE_NUM dest = trkd.trkCurrNote;
								if (dest < 0x80)
								{
									int32 dn = (int32)dest + (ss->GetPlayTranspose() + trkd.trkSTransp);

									while (dn >= 0x80)
										dn -= 12;

									while (dn < 1)
										dn += 12;

									dest = (NOTE_NUM)dn;
								}

								trkd.trkPortTargetFreq = GetInstrNoteFreq(dest, plrSong->GetInstr(trkd.trkPrevINum));
							}

							if (dataW != 0)
								trkd.trkPortSpeed = dataW;

							trkd.trkFxType = TrackData::noPlay;
							break;
						}

						// Set hold/decay
						case 0x08:
						{
							if (plrSong->GetInstr(trkd.trkPrevINum)->IsMIDI())
							{
								// Two digits used for hold with MIDI instruments
								trkd.trkInitHold = data;
							}
							else
							{
								trkd.trkInitHold  = data & 0x0f;
								trkd.trkInitDecay = data >> 4;
							}
							break;
						}

						// Set ticks per line
						case 0x09:
						{
							ss->SetTempoTPL(data & 0x1f);
							ChangePlayFreq();
							break;
						}

						// Position jump
						case 0x0b:
						{
							plrBreak    = positionJump;
							plrNextLine = data;
							break;
						}

						// Volume
						case 0x0c:
						{
							if (data < 0x80)
								trkd.trkPrevVol = data;
							else
							{
								// Set default volume
								data &= 0x7f;
								trkd.trkPrevVol = data;
								plrSong->GetInstr(trkd.trkPrevINum)->SetVol(data);
							}
							break;
						}

						// Set synth waveform sequence position
						case 0x0e:
						{
							trkd.trkSy.wfCmdPos = data;
							trkd.trkMiscFlags  |= TrackData::NO_SYNTH_WFPTR_RESET;
							break;
						}

						// Misc/Main tempo
						case 0x0f:
						{
							switch (data)
							{
								case 0x00:
								{
									plrBreak    = patternBreak;
									plrNextLine = 0;
									break;
								}

								case 0xf2:
								case 0xf4:
								case 0xf5:
									goto delay_note;

								case 0xf7:	// Wait until MIDI messages sent
									break;

								// TN: Filter support added
								case 0xf8:	// Amiga filter off
								{
									med->amigaFilter = false;
									break;
								}

								case 0xf9:	// Amiga filter on
								{
									med->amigaFilter = true;
									break;
								}

								case 0xfd:	// Change frequency
								{
									if (trkd.trkCurrNote != 0)
									{
										NOTE_NUM dest = trkd.trkCurrNote;
										if (dest < 0x80)
										{
											int32 dn = (int32)dest + (ss->GetPlayTranspose() + trkd.trkSTransp);

											while (dn >= 0x80)
												dn -= 12;

											while (dn < 1)
												dn += 12;

											dest = (NOTE_NUM)dn;
										}

										trkd.trkFrequency = GetInstrNoteFreq(dest, plrSong->GetInstr(trkd.trkPrevINum));
									}
									break;
								}

								case 0xfe:	// Stop!
								{
									plrDelayedStop  = true;
									med->endReached = true;
									break;
								}

								case 0xff:
								{
									MuteChannel(trkCnt);
									break;
								}

								default:	// Change tempo
								{
									if (data <= 240)
									{
										ss->SetTempoBPM(data);
										ChangePlayFreq();
									}
									break;
								}
							}
							break;
						}

						// Send custom MIDI/SYSX message
						case 0x10:
						{
							break;
						}

						// Finetune
						case 0x15:
						{
							int8 sData = (int8)data;

							if ((sData >= -8) && (sData <= 7))
								trkd.trkFineTune = sData;

							break;
						}

						// Repeat loop
						case 0x16:
						{
							if (data != 0)
							{
								if (plrRepeatCounter == 0)
									plrRepeatCounter = data;	// Init loop
								else
								{
									if (--plrRepeatCounter == 0)
										break;					// Continue
								}

								plrNextLine = plrRepeatLine;	// Jump to beginning of loop
								plrBreak    = loop;
							}
							else
								plrRepeatLine = line;			// Store line number

							break;
						}

						// Sample offset
						case 0x19:
						{
							trkd.trkSOffset = (uint32)dataW << 8;
							break;
						}

						// Change MIDI preset
						case 0x1c:
						{
							break;
						}

						// Next pattern
						case 0x1d:
						{
							plrBreak    = patternBreak;
							plrNextLine = data;
							break;
						}

						// Block delay
						case 0x1e:
						{
							if (plrBlockDelay == 0)
								plrBlockDelay = data + 1;

							break;
						}

						// Delay/Retrig
						case 0x1f:
						{
delay_note:
							if (!(trkd.trkNoteOffCnt = trkd.trkInitHold))
								trkd.trkNoteOffCnt = -1;

							trkd.trkFxType = TrackData::noPlay;
							break;
						}

						// Sample backwards
						case 0x20:
						{
							if (dataW == 0)
								trkd.trkMiscFlags |= TrackData::BACKWARDS;

							break;
						}

						// Filter sweep (CutOff)
						case 0x23:
						{
							if (dataW == 0)
								trkd.trkCutOffTarget = 0;
							else
							{
								trkd.trkCutOffTarget  = ((int32)cmd.GetDataB() + 1) << 8;
								trkd.trkCutOffSwSpeed = cmd.GetData2() * 20;
								trkd.trkCutOffLogPos  = 0;		// Filled by the sweep code
							}
							break;
						}

						// Set filter cutoff frequency
						case 0x24:
						{
//XX							EffectGroup *eg = &ss->fx.GetGroup(ss->fx.GetTrackGroup(trkCnt));
/*							if (dataW == 0)
								eg->FilterOff();
							else
								eg->SetFilter(dataW);
*/
							trkd.trkCutOffTarget = 0;			// Stop any sweep
							break;
						}

						// Set filter resonance + type
						case 0x25:
						{
//XX							EffectGroup *eg = &ss->fx.GetGroup(ss->fx.GetTrackGroup(trkCnt));
/*							eg->SetResonance(dataW >> 4);

							uint8 fType = (uint8)dataW & 0x0f;
							if (fType == 1)
								eg->SetFilterType(EffectGroup::LP);
							else
							{
								if (fType == 2)
									eg->SetFilterType(EffectGroup::HP);
							}
*/							break;
						}

						// ARexx trigger (only on Amiga)
						case 0x2d:
							break;

						// Panpot
						case 0x2e:
						{
							if (((int8)data >= -16) && ((int8)data <= 16))
								ss->SetTrackPan(trkCnt, (int8)data);

							break;
						}
					}
				}
			}

			// Play notes
			for (trkCnt = 0; trkCnt < blk->Tracks(); trkCnt++)
			{
				TrackData &trkd = td[trkCnt];

				if (trkd.trkFxType == TrackData::noPlay)
					continue;

				if (trkd.trkCurrNote != 0)
				{
					if (trkd.trkInitHold != 0)
						trkd.trkNoteOffCnt = trkd.trkInitHold;
					else
						trkd.trkNoteOffCnt = -1;

					PlrPlayNote(trkCnt, trkd.trkCurrNote - 1, trkd.trkPrevINum);
				}
			}

			// Take commands from current line/block
			plrFxLine  = line;
			plrFxBlock = plrPos.Block();

			// Advance play position
			if (!plrDelayedStop)
			{
				switch (plrBreak)
				{
					case loop:
					{
						plrPos.Line(plrNextLine);
						plrBreak = normal;
						break;
					}

					case patternBreak:
					{
						plrPos.PatternBreak(plrNextLine, PSeqCmdHandler);
						plrBreak = normal;
						break;
					}

					case positionJump:
					{
						plrPos.PositionJump(plrNextLine, PSeqCmdHandler);
						plrBreak = normal;
						break;
					}

					default:
					{
						plrPos.AdvancePos(PSeqCmdHandler);
						break;
					}
				}
			}

			blk = &ss->Block(plrPos.Block());	// Block may have changed
		}

		// Check holding
		LINE_NUM currLine = plrPos.Line();

		for (trkCnt = 0; trkCnt < blk->Tracks(); trkCnt++)
		{
			if (td[trkCnt].trkNoteOffCnt >= 0)
			{
				MED_Note &note = blk->Note(currLine, trkCnt);

				// Continue hold if:
				// * "hold" symbol (no note, instr. num) OR
				// * note and command 03 on page 1
				if (((note.noteNum == 0) && (note.instrNum != 0)) || ((note.noteNum != 0) && (blk->Cmd(currLine, trkCnt, 0).GetCmd() == 0x03)))
					td[trkCnt].trkNoteOffCnt += ss->GetTempoTPL();
			}
		}
	}

	// Back to processing for every tick...
	if (plrFxBlock >= ss->NumBlocks())
		plrFxBlock = ss->NumBlocks() - 1;

	blk = &ss->Block(plrFxBlock);

	// Hold and fade handling
	for (trkCnt = 0; trkCnt < blk->Tracks(); trkCnt++)
	{
		TrackData &trkd = td[trkCnt];

		if ((trkd.trkPrevMidiN > 0) && (trkd.trkPrevMidiN <= 0x80))
		{
			trkd.trkFxType = TrackData::MIDI;
			if ((trkd.trkMiscFlags & TrackData::STOPNOTE) || ((trkd.trkNoteOffCnt >= 0) && (--trkd.trkNoteOffCnt < 0)))
			{
				trkd.trkMiscFlags &= ~TrackData::STOPNOTE;
				MuteChannel(trkCnt);
			}
		}
		else
		{
			if ((trkd.trkMiscFlags & TrackData::STOPNOTE) || ((trkd.trkNoteOffCnt >= 0) && (--trkd.trkNoteOffCnt < 0)))
			{
				trkd.trkMiscFlags &= ~TrackData::STOPNOTE;
				if (trkd.trkSy.synthType != SynthData::none)
				{
					// A synth/hybrid sound has special way of decay (JMP)
					trkd.trkSy.volCmdPos = trkd.trkDecay;
					trkd.trkSy.volWait   = 0;
				}
				else
				{
					// A normal decay... just set fadespeed, and if 0, mute immediately
					if ((trkd.trkFadeSpeed = trkd.trkDecay * 2) == 0)
						MuteChannel(trkCnt);
				}
			}

			if (trkd.trkFadeSpeed != 0)
			{
				if (trkd.trkPrevVol > trkd.trkFadeSpeed)
					trkd.trkPrevVol -= (uint8)trkd.trkFadeSpeed;
				else
				{
					trkd.trkPrevVol   = 0;
					trkd.trkFadeSpeed = 0;
				}
			}

			trkd.trkFxType = TrackData::normal;
		}
	}

	if (plrFxLine >= blk->Lines())
		plrFxLine = blk->Lines() - 1;

	// Effect handling (once per timing pulse)
	for (pageCnt = 0; pageCnt < blk->Pages(); pageCnt++)
	{
		for (trkCnt = 0; trkCnt < blk->Tracks(); trkCnt++)
		{
			TrackData &trkd = td[trkCnt];

			if (trkd.trkFxType == TrackData::none)
				continue;

			MED_Cmd &cmd = blk->Cmd(plrFxLine, trkCnt, pageCnt);

			// Call MIDI command handler and skip normal cmd handling if
			// MIDI command handled by this routine
			if (trkd.trkLastNoteMidi && MIDICommand(trkd, cmd))
				continue;

			uint8 data = cmd.GetDataB();
			switch (cmd.GetCmd())
			{
				// Arpeggio
				case 0x00:
				{
					if (cmd.GetData2())
					{
						NOTE_NUM base = trkd.trkPrevNote;
						if (base > 0x80)
							break;

						switch (plrPulseCtr % 3)
						{
							case 0:
							{
								base += cmd.GetData2() >> 4;
								break;
							}

							case 1:
							{
								base += cmd.GetData2() & 0x0f;
								break;
							}
						}

						base += (uint8)(ss->GetPlayTranspose() - 1 + trkd.trkSTransp);
						int32 freq = GetNoteFrequency(base, trkd.trkFineTune);
						trkd.trkArpAdjust = freq - trkd.trkFrequency;	// Arpeggio difference
					}
					break;
				}

				// Slide up (once)
				case 0x11:
				{
					if (plrPulseCtr != 0)
						break;
					else
						goto cmd1_do;
				}

				// Slide up
				case 0x01:
				{
					if ((plrPulseCtr == 0) && ss->GetSlide1st())
						break;

cmd1_do:			if (trkd.trkFrequency > 0)
					{
						int32 div = 3579545 * 256 / trkd.trkFrequency - (int32)((uint16)cmd.GetData());
						if (div > 0)
							trkd.trkFrequency = 3579545 * 256 / div;
					}
					break;
				}

				// Slide down (once)
				case 0x12:
				{
					if (plrPulseCtr != 0)
						break;
					else
						goto cmd2_do;
				}

				// Slide down
				case 0x02:
				{
					if ((plrPulseCtr == 0) && ss->GetSlide1st())
						break;

cmd2_do:			if (trkd.trkFrequency > 0)
					{
						int32 div = 3579545 * 256 / trkd.trkFrequency + (int32)((uint16)cmd.GetData());
						if (div > 0)
							trkd.trkFrequency = 3579545 * 256 / div;
					}
					break;
				}

				// Portamento
				case 0x03:
				{
					if ((plrPulseCtr == 0) && ss->GetSlide1st())
						break;

do_portamento:		if ((trkd.trkPortTargetFreq == 0) || (trkd.trkFrequency <= 0))
						break;

					int32 newFreq = trkd.trkFrequency, div = 3579545 * 256 / newFreq;

					if (trkd.trkFrequency > trkd.trkPortTargetFreq)
					{
						div += (int32)trkd.trkPortSpeed;
						if (div != 0)
						{
							newFreq = 3579545 * 256 / div;
							if (newFreq <= trkd.trkPortTargetFreq)
							{
								newFreq = trkd.trkPortTargetFreq;
								trkd.trkPortTargetFreq = 0;
							}
						}
					}
					else
					{
						if (div > (int32)trkd.trkPortSpeed)
						{
							div -= (int32)trkd.trkPortSpeed;
							newFreq = 3579545 * 256 / div;
						}
						else
							newFreq = trkd.trkPortTargetFreq;

						if (newFreq >= trkd.trkPortTargetFreq)
						{
							newFreq = trkd.trkPortTargetFreq;
							trkd.trkPortTargetFreq = 0;
						}
					}

					trkd.trkFrequency = newFreq;
					break;
				}

				// Volume slide
				case 0x0d:
				case 0x0a:
				case 0x06:		// (with vibrato)
				case 0x05:		// (or portamento)
				{
					if ((plrPulseCtr == 0) && ss->GetSlide1st())
						break;

					if (data & 0xf0)
					{
						trkd.trkPrevVol += (data >> 4) * 2;
						if (trkd.trkPrevVol > 127)
							trkd.trkPrevVol = 127;
					}
					else
					{
						if (((data & 0x0f) * 2) > trkd.trkPrevVol)
							trkd.trkPrevVol = 0;
						else
							trkd.trkPrevVol -= (data & 0x0f) * 2;
					}

					if (cmd.GetCmd() == 0x06)
						goto do_vibrato;	// Command 06
					else
					{
						if (cmd.GetCmd() == 0x05)
							goto do_portamento;	// Command 05
					}
					break;
				}

				// Vibrato (deeper)
				case 0x04:
				{
					trkd.trkVibShift = 5;
					goto vib_cont;
				}

				// Vibrato (shallower)
				case 0x14:
				{
					trkd.trkVibShift = 6;
vib_cont:			if ((plrPulseCtr == 0) && (data != 0))
					{
						// Check data on pulse #0 for possible new values
						if (data & 0x0f)
							trkd.trkVibSize = data & 0x0f;	// New vibrato size

						if (data >> 4)
							trkd.trkVibSpeed = (data >> 4) * 2;	// New vibrato speed
					}

do_vibrato:			if (trkd.trkFrequency > 0)
					{
						// Another piece of Amiga period emulation code
						int32 per = 3579545 / trkd.trkFrequency;
						per += (sineTable[(trkd.trkVibOffs >> 2) & 0x1f] * trkd.trkVibSize) >> trkd.trkVibShift;

						if (per > 0)
							trkd.trkVibrAdjust = 3579545 / per - trkd.trkFrequency;
					}

					trkd.trkVibOffs += trkd.trkVibSpeed;
					break;
				}

				// Simple pulse vibrato
				case 0x13:
				{
					if (plrPulseCtr < 3)
						trkd.trkVibrAdjust = -(int32)data;

					break;
				}

				// Cut note
				case 0x18:
				{
					if (plrPulseCtr == data)
						trkd.trkPrevVol = 0;

					break;
				}

				// Volume slide up (small)
				case 0x1a:
				{
					if (plrPulseCtr == 0)
					{
						uint8 incr = data + (cmd.GetData2() >= 0x80 ? 1 : 0);
						if ((trkd.trkPrevVol + incr) < 127)
							trkd.trkPrevVol += incr;
						else
							trkd.trkPrevVol = 127;
					}
					break;
				}

				// Volume slide down (small)
				case 0x1b:
				{
					if (plrPulseCtr == 0)
					{
						uint8 decr = data - (cmd.GetData2() >= 0x80 ? 1 : 0);
						if (trkd.trkPrevVol > decr)
							trkd.trkPrevVol -= decr;
						else
							trkd.trkPrevVol = 0;
					}
					break;
				}

				// Misc retrig commands
				case 0x0f:
				{
					switch (data)
					{
						case 0xf1:
						case 0xf2:
						{
							if (plrPulseCtr == 3)
								PlayFXNote(trkCnt, trkd);

							break;
						}

						case 0xf3:
						{
							if ((plrPulseCtr == 2) || (plrPulseCtr == 4))
								PlayFXNote(trkCnt, trkd);

							break;
						}

						case 0xf4:
						{
							if ((ss->GetTempoTPL() / 3) == plrPulseCtr)
								PlayFXNote(trkCnt, trkd);

							break;
						}

						case 0xf5:
						{
							if (((ss->GetTempoTPL() * 2) / 3) == plrPulseCtr)
								PlayFXNote(trkCnt, trkd);

							break;
						}

						case 0xf7:
						{
							break;
						}
					}
					break;
				}

				// Note delay/retrig
				case 0x1f:
				{
					if (data >> 4)
					{
						// There's note delay specified
						if (plrPulseCtr < (data >> 4))
							break;		// Delay still going on...

						if (plrPulseCtr == (data >> 4))
						{
							PlayFXNote(trkCnt, trkd);
							break;
						}
					}

					if ((data & 0x0f) && (!(plrPulseCtr % (data & 0x0f))))
						PlayFXNote(trkCnt, trkd);

					break;
				}

				// Change sample position
				case 0x20:
				{
					if ((plrPulseCtr == 0) && (data != 0))
						ChangeSamplePosition(trkCnt, (int32)((int16)cmd.GetData()));

					break;
				}

				// Slide up (const. rate)
				case 0x21:
				{
					trkd.trkFrequency += (trkd.trkFrequency * data) >> 11;
					if (trkd.trkFrequency > 65535)
						trkd.trkFrequency = 65535;

					break;
				}

				// Slide down (const. rate)
				case 0x22:
				{
					if ((((trkd.trkFrequency * data) >> 11) + 1) < trkd.trkFrequency)
						trkd.trkFrequency -= (trkd.trkFrequency * data) >> 11;
					else
						trkd.trkFrequency = 1;

					break;
				}

				// Change sample position II (relative to sample length)
				case 0x29:
				{
					if (plrPulseCtr == 0)
					{
						Sample *smp = plrSong->GetSample(trkd.trkPrevINum);
						uint8 div   = cmd.GetData2();

						if (div == 0)
							div = 0x10;		// Default divisor is 16

						if ((smp != NULL) && !smp->IsSynthSound() && (data < div))
						{
							int32 len = smp->GetLength();
							SetSamplePosition(trkCnt, (data * len) / div);
						}
					}
					break;
				}
			}

			// Filter sweep
			if (trkd.trkCutOffTarget != 0)
			{
				//XX
				;
			}
		}
	}

	for (trkCnt = 0; trkCnt < blk->Tracks(); trkCnt++)
		UpdateFreqVolPan(ss, trkCnt);
}



/******************************************************************************/
/* EnableTrack() will enable or disable a single track.                       */
/*                                                                            */
/* Input:  "tNum" is the track number to change the state on.                 */
/*         "en" indicate if you want to enable (true) or disable the track.   */
/******************************************************************************/
void Player::EnableTrack(TRACK_NUM tNum, bool en)
{
	if (!(plrTrackEnabled[tNum] = en))
		MuteChannel(tNum);
}



/******************************************************************************/
/* IsTrackEnabled() will tell if a specified track is enabled or disabled.    */
/*                                                                            */
/* Input:  "tNum" is the track number to check.                               */
/*                                                                            */
/* Output: True if the track is enabled, false if not.                        */
/******************************************************************************/
bool Player::IsTrackEnabled(TRACK_NUM tNum) const
{
	return (plrTrackEnabled[tNum]);
}



/******************************************************************************/
/* RecalcVolAdjust() will recalculate the volume adjustments.                 */
/******************************************************************************/
void Player::RecalcVolAdjust(void)
{
}



/******************************************************************************/
/* ChangePlayFreq() will tell APlayer about the play frequency change.        */
/******************************************************************************/
void Player::ChangePlayFreq(void)
{
	PString freq;

	// Change the module info
	freq.Format("%.02f", med->playFreq);
	med->ChangeModuleInfo(3, freq);
}



/******************************************************************************/
/* PlrPlayNote() will trig a note and begin playing it.                       */
/*                                                                            */
/* Input:  "trk" is the track number to play in.                              */
/*         "note" is the note to play.                                        */
/*         "iNum" is the instrument to use.                                   */
/******************************************************************************/
void Player::PlrPlayNote(TRACK_NUM trk, NOTE_NUM note, INST_NUM iNum)
{
	TrackData &trkd = td[trk];

	if (!plrSong->InstrSlotUsed(iNum) || (trk >= MAX_TRACKS))
		return;

	Instr *currI = plrSong->GetInstr(iNum);
	if (currI->i_flags & Instr::DISABLED)
	{
		MuteChannel(trk);
		return;
	}

	if (note < 0x80)
	{
		int32 nt = note;
		nt += plrSong->CurrSS()->GetPlayTranspose();
		nt += currI->GetTransp();

		while (nt >= 0x80)
			nt -= 12;

		while (nt < 0)
			nt += 12;

		note = (NOTE_NUM)nt;
	}

	if ((trkd.trkPrevMidiN > 0) && (trkd.trkPrevMidiN <= 0x80))
		trkd.trkPrevMidiN = 0;

	if (currI->IsMIDI())
	{
		trkd.trkLastNoteMidi = true;
		return;
	}

	if (trk > plrChannels)
		return;

	if (note < 0x80)
	{
		while (note > 71)
			note -= 12;
	}

	trkd.trkDecay        = trkd.trkInitDecay;
	trkd.trkFadeSpeed    = 0;
	trkd.trkVibOffs      = 0;
	trkd.trkFrequency    = GetInstrNoteFreq(note + 1, currI);
	trkd.trkLastNoteMidi = false;

	SynthData &sy = trkd.trkSy;

	if (plrSong->GetSample(iNum)->IsSynthSound())
	{
		// Find sample/synth type
		//
		// If next sound is synthsound, and the previous wasn't, prepare synth play
		if (trkd.trkSy.synthType != SynthData::synth)
			PrepareSynthSound(trk);

		if (plrSong->GetSample(iNum)->GetLength() != 0)
			sy.synthType = SynthData::hybrid;
		else
			sy.synthType = SynthData::synth;
	}
	else
		sy.synthType = SynthData::none;

	if (sy.synthType != SynthData::synth)
	{
		// An ordinary sample or hybrid
		Play(trk, plrSong->GetSample(iNum), trkd.trkFrequency, trkd.trkSOffset, currI->GetRepeat(), currI->GetRepeatLen(),
			((currI->i_flags & Instr::LOOP) ? MIXER_PLAY_LOOP : 0) | ((trkd.trkMiscFlags & TrackData::BACKWARDS) ? MIXER_PLAY_BACKWARDS : 0) |
			((currI->i_flags & Instr::PINGPONG) ? MIXER_PLAY_PINGPONGLOOP : 0));
	}

	if (sy.synthType != SynthData::none)
	{
		sy.noteNumber = note;
		sy.arpOffs    = 0;
		sy.arpStart   = 0;
		sy.volXCnt    = 0;
		sy.wfXCnt     = 0;
		sy.volCmdPos  = 0;

		if (!(trkd.trkMiscFlags & TrackData::NO_SYNTH_WFPTR_RESET))
			sy.wfCmdPos = 0;

		sy.volWait      = 0;
		sy.wfWait       = 0;
		sy.vibSpeed     = 0;
		sy.vibDep       = 0;
		sy.vibWFNum     = 0;
		sy.vibOffs      = 0;
		sy.periodChange = 0;
		sy.volChgSpeed  = 0;
		sy.wfChgSpeed   = 0;
		sy.volXSpeed    = plrSong->GetSynthSound(iNum)->GetVolSpeed();
		sy.wfXSpeed     = plrSong->GetSynthSound(iNum)->GetWFSpeed();
		sy.envWFNum     = 0;
	}
}



/******************************************************************************/
/* ExtractInstrData() extract all the instrument info from the track.         */
/*                                                                            */
/* Input:  "trkd" is a reference to the track data to store the instr info.   */
/*         "instr" is a pointer to the instrument.                            */
/******************************************************************************/
void Player::ExtractInstrData(TrackData &trkd, Instr *currI)
{
	trkd.trkPrevVol   = (uint8)currI->GetVol();
	trkd.trkInitHold  = currI->GetHold();
	trkd.trkInitDecay = currI->GetDecay();
	trkd.trkSTransp   = currI->GetTransp();
	trkd.trkFineTune  = currI->GetFineTune();
	trkd.trkSOffset   = 0;
	trkd.trkMiscFlags = 0;
}



/******************************************************************************/
/* PlayFXNote() will retrig the current note on the track.                    */
/*                                                                            */
/* Input:  "trkNum" is the track number to retrig.                            */
/*         "trkd" is a reference to the track data.                           */
/******************************************************************************/
void Player::PlayFXNote(TRACK_NUM trkNum, TrackData &trkd)
{
	if (trkd.trkCurrNote != 0)
	{
		if (trkd.trkNoteOffCnt >= 0)
			trkd.trkNoteOffCnt += plrPulseCtr;
		else
		{
			if (trkd.trkInitHold != 0)
				trkd.trkNoteOffCnt = trkd.trkInitHold;
			else
				trkd.trkNoteOffCnt = -1;
		}

		PlrPlayNote(trkNum, trkd.trkCurrNote - 1, trkd.trkPrevINum);
	}
}



/******************************************************************************/
/* UpdateFreqVolPan() will set/change the frequency, volume, and panning.     */
/*                                                                            */
/* Input:  "ss" is a pointer to the subsong playing.                          */
/*         "trkNum" is the track number of update.                            */
/******************************************************************************/
void Player::UpdateFreqVolPan(SubSong *ss, TRACK_NUM trkNum)
{
	uint8 usedVolume;
	TrackData &trkd = td[trkNum];

	if (trkd.trkFxType != TrackData::normal)
		return;

	SetChannelFreq(trkNum, (trkd.trkSy.synthType == SynthData::none ? trkd.trkFrequency : SynthHandler(trkNum, trkd, plrSong->GetSynthSound(trkd.trkPrevINum))) + trkd.trkArpAdjust + trkd.trkVibrAdjust);

	if (trkd.trkTempVol != 0)
	{
		usedVolume      = trkd.trkTempVol - 1;
		trkd.trkTempVol = 0;
	}
	else
		usedVolume = trkd.trkPrevVol;

	SetChannelVolPan(trkNum, (int16)((usedVolume * ss->GetTrackVol(trkNum) * ss->GetMasterVol()) / (64 * 64)), (int16)ss->GetTrackPan(trkNum));

	trkd.trkArpAdjust  = 0;
	trkd.trkVibrAdjust = 0;
}



/******************************************************************************/
/* MIDICommand() will check the command and see if it's a MIDI command.       */
/*                                                                            */
/* Input:  "trkd" is a reference to the track data.                           */
/*         "cmd" is a reference to the command to check.                      */
/*                                                                            */
/* Output: True if the command is a MIDI command, false if not.               */
/******************************************************************************/
bool Player::MIDICommand(TrackData &trkd, MED_Cmd &cmd)
{
	switch (cmd.GetCmd())
	{
		case 0x00:		// Midi controller
		case 0x01:		// Pitch bend up
		case 0x02:		// Pitch bend down
		case 0x03:		// Set pitchbender
		case 0x13:		// -""-
		case 0x04:		// Modulation wheel
		case 0x05:		// Set controller
		case 0x0a:		// Polyphonic aftertouch
		case 0x0d:		// Channel aftertouch
		case 0x0e:		// Panpot
		case 0x17:		// Set volume
			return (true);

		case 0x0f:		// Miscellaneous
		{
			switch (cmd.GetDataB())
			{
				case 0x0fa:		// Hold pedal on
				case 0x0fb:		// Hold pedal off
					return (true);
			}
			break;
		}

		default:
			return (true);
	}

	return (false);
}



/******************************************************************************/
/* SynthHandler() handles all the synth sounds.                               */
/*                                                                            */
/* Input:  "chNum" is the channel number to play in.                          */
/*         "trkd" is a reference to the track data.                           */
/*         "snd" is a pointer to the synth sound object.                      */
/*                                                                            */
/* Output: The frequency to play the sound in.                                */
/******************************************************************************/
int32 Player::SynthHandler(uint32 chNum, TrackData &trkd, SynthSound *snd)
{
	SynthData &sy = trkd.trkSy;

	// Make sure that the synthsound hasn't disappeared meanwhile...
	if ((snd == NULL) || !snd->IsSynthSound())
	{
		sy.synthType = SynthData::none;
		MuteChannel(chNum);
		return (0);
	}

	if (sy.volXCnt-- <= 1)
	{
		sy.volXCnt = sy.volXSpeed;		// Reset volume execution counter

		if (sy.volChgSpeed != 0)
		{
			// CHU or CHD is active
			sy.vol += sy.volChgSpeed;
			if (sy.vol < 0)
				sy.vol = 0;
			else
			{
				if (sy.vol > 64)
					sy.vol = 64;
			}
		}

		if (sy.envWFNum != 0)
		{
			if ((sy.envWFNum == 1) && (snd->GetLength() != 0))
			{
				;
			}
			else
			{
				if (sy.envWFNum <= (uint32)snd->CountItems())
				{
					// Envelope wave's been set
					sy.vol = ((int32)snd->GetItem(sy.envWFNum - 1)->sywfData[sy.envCount] + 128) >> 2;

					if (++sy.envCount >= 128)
					{
						sy.envCount = 0;
						if (!sy.envLoop)
							sy.envWFNum = 0;
					}
				}
			}
		}

		if ((sy.volWait == 0) || (--sy.volWait == 0))
		{
			// Check WAI
			bool endCycle   = false;
			int32 instCount = 0;		// Instruction count prevents deadlocking

			while (!endCycle && (++instCount < 128))
			{
				uint8 cmd = snd->GetVolData(sy.volCmdPos++);

				if (cmd < 0x80)
				{
					sy.vol = cmd;
					break;		// Also end cycle
				}
				else
				{
					switch (cmd)
					{
						case SynthSound::CMD_JMP:
						{
							sy.volCmdPos = snd->GetVolData(sy.volCmdPos);
							break;
						}

						case SynthSound::CMD_SPD:
						{
							sy.volXSpeed = snd->GetVolData(sy.volCmdPos++);
							break;
						}

						case SynthSound::CMD_WAI:
						{
							sy.volWait = snd->GetVolData(sy.volCmdPos++);
							endCycle   = true;
							break;
						}

						case SynthSound::CMD_CHU:
						{
							sy.volChgSpeed = snd->GetVolData(sy.volCmdPos++);
							break;
						}

						case SynthSound::CMD_CHD:
						{
							sy.volChgSpeed = -snd->GetVolData(sy.volCmdPos++);
							break;
						}

						case SynthSound::VOLCMD_JWS:
						{
							sy.wfWait   = 0;
							sy.wfCmdPos = snd->GetVolData(sy.volCmdPos++);
							break;
						}

						case SynthSound::VOLCMD_EN1:
						{
							sy.envWFNum = snd->GetVolData(sy.volCmdPos++) + 1;
							sy.envLoop  = false;		// One-shot envelope
							sy.envCount = 0;
							break;
						}

						case SynthSound::VOLCMD_EN2:
						{
							sy.envWFNum = snd->GetVolData(sy.volCmdPos++) + 1;
							sy.envLoop  = true;			// Looped envelope
							sy.envCount = 0;
							break;
						}

						case SynthSound::CMD_RES:
						{
							sy.envWFNum = 0;
							break;
						}

						case SynthSound::CMD_END:
						case SynthSound::CMD_HLT:
						{
							sy.volCmdPos--;
						}

						default:
						{
							endCycle = true;
							break;
						}
					}
				}
			}
		}
	}

	// TN: Bug fix. Replaced the line below with this one.
	// This make sure that the Parasol Stars synth module can play correctly
	trkd.trkPrevVol = sy.vol;
//	trkd.trkTempVol = (uint8)((sy.vol * trkd.trkPrevVol) >> 6) + 1;

	if (sy.wfXCnt-- <= 1)
	{
		sy.wfXCnt = sy.wfXSpeed;

		if (sy.wfChgSpeed != 0)
			sy.periodChange += sy.wfChgSpeed;		// CHU or CHD active

		if ((sy.wfWait == 0) || (--sy.wfWait == 0))
		{
			// Check WAI
			bool endCycle   = false;
			int32 instCount = 0;

			while (!endCycle && (++instCount < 128))
			{
				uint8 cmd = snd->GetWFData(sy.wfCmdPos++);

				if (cmd < 0x80)
				{
					// Crash prevention
					if (cmd < snd->CountItems())
					{
						SynthWF &swf = *snd->GetItem(cmd);
						SetSynthWaveform(chNum, swf.sywfData, swf.sywfLength * 2);
					}
					break;		// Also end cycle
				}
				else
				{
					switch (cmd)
					{
						case SynthSound::WFCMD_VWF:
						{
							sy.vibWFNum = snd->GetWFData(sy.wfCmdPos++) + 1;
							break;
						}

						case SynthSound::CMD_JMP:
						{
							sy.wfCmdPos = snd->GetWFData(sy.wfCmdPos);
							break;
						}

						case SynthSound::WFCMD_ARP:
						{
							sy.arpOffs  = sy.wfCmdPos;
							sy.arpStart = sy.wfCmdPos;

							// Scan until next command (preferable ARE) found
							while (snd->GetWFData(sy.wfCmdPos) < 0x80)
								sy.wfCmdPos++;

							break;
						}

						case SynthSound::CMD_SPD:
						{
							sy.wfXSpeed = snd->GetWFData(sy.wfCmdPos++);
							break;
						}

						case SynthSound::CMD_WAI:
						{
							sy.wfWait = snd->GetWFData(sy.wfCmdPos++);
							endCycle  = true;
							break;
						}

						case SynthSound::WFCMD_VBD:
						{
							sy.vibDep = snd->GetWFData(sy.wfCmdPos++);
							break;
						}

						case SynthSound::WFCMD_VBS:
						{
							sy.vibSpeed = snd->GetWFData(sy.wfCmdPos++);
							break;
						}

						case SynthSound::CMD_CHD:
						{
							sy.wfChgSpeed = snd->GetWFData(sy.wfCmdPos++);
							break;
						}

						case SynthSound::CMD_CHU:
						{
							sy.wfChgSpeed = -snd->GetWFData(sy.wfCmdPos++);
							break;
						}

						case SynthSound::CMD_RES:
						{
							sy.periodChange = 0;
							break;
						}

						case SynthSound::WFCMD_JVS:
						{
							sy.volCmdPos = snd->GetWFData(sy.wfCmdPos++);
							sy.volWait   = 0;
							break;
						}

						case SynthSound::CMD_END:
						case SynthSound::CMD_HLT:
						{
							sy.wfCmdPos--;
						}

						default:
						{
							endCycle = true;
							break;
						}
					}
				}
			}
		}
	}

	int32 currFreq = trkd.trkFrequency;

	// Arpeggio
	if (sy.arpOffs != 0)
	{
		currFreq = GetNoteFrequency(sy.noteNumber + snd->GetWFData(sy.arpOffs), trkd.trkFineTune);
		if (snd->GetWFData(++sy.arpOffs) >= 0x80)
			sy.arpOffs = sy.arpStart;
	}

	// Vibrato
	int32 currPeriodChange = sy.periodChange;
	if (sy.vibDep != 0)
	{
		if ((sy.vibWFNum == 1) && (snd->GetLength() != 0))
		{
			;
		}
		else
		{
			if (sy.vibWFNum <= (uint32)snd->CountItems())
			{
				const int8 *vibWave = ((sy.vibWFNum != 0) && (sy.vibWFNum <= (uint32)snd->CountItems())) ? snd->GetItem(sy.vibWFNum - 1)->sywfData : sineTable;
				currPeriodChange   += (vibWave[(sy.vibOffs >> 4) & 0x1f] * sy.vibDep) / 256;
				sy.vibOffs         += sy.vibSpeed;
			}
		}
	}

	if ((currPeriodChange != 0) && (currFreq != 0))
	{
		int32 newPer = 3579545 / currFreq + currPeriodChange;

		if (newPer < 113)
			newPer = 113;

		currFreq = 3579545 / newPer;
	}

	return (currFreq);
}



/******************************************************************************/
/* MuteChannel() stops the channel given.                                     */
/******************************************************************************/
void Player::MuteChannel(uint32 chNum)
{
	TrackData &trkd      = td[chNum];
	trkd.trkSy.synthType = SynthData::none;
	uint8 prevMN         = trkd.trkPrevMidiN;

	if (prevMN != 0)
		trkd.trkPrevMidiN = 0xff;
	else
		MED_Mixer::MuteChannel(chNum);
}



/******************************************************************************/
/* PSeqCmdHandler()                                                           */
/******************************************************************************/
bool Player::PSeqCmdHandler(OctaMED *octaMED, PlaySeqEntry &pse)
{
	if (pse.GetCmd() == PSEQCMD_STOP)
	{
		octaMED->plr->plrDelayedStop = true;
		return (true);
	}

	return (false);
}
