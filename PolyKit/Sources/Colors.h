// Useful until be gets around to making these sorts of things
// globals akin to be_plain_font, etc.

#if !defined(__Colors_h) && !defined(_SGB_COLORS_H_)
#define __Colors_h
#define _SGB_COLORS_H_ // This header defines the same things that SGB does

#include "POS.h"

/** @name Be standard UI colors */
//@{
/// BeBackgroundGrey
static const rgb_color BeBackgroundGrey = 		{216,216,216,	255};
/// BeInactiveControlGrey
static const rgb_color BeInactiveControlGrey =	{240,240,240,	255};
/// BeFocusBlue
static const rgb_color BeFocusBlue =			{  0,  0,229,	255};
/// BeHighlight
static const rgb_color BeHighlight =			{255,255,255,	255};
/// BeShadow
static const rgb_color BeShadow =				{152,152,152,	255};
/// BeDarkShadow
static const rgb_color BeDarkShadow =			{108,108,108,	255};
/// BeLightShadow
static const rgb_color BeLightShadow =			{194,194,194,	255};
/// BeButtonGrey
static const rgb_color BeButtonGrey =			{232,232,232,	255};
/// BeInactiveGrey
static const rgb_color BeInactiveGrey =			{127,127,127,	255};
/// BeListSelectGrey
static const rgb_color BeListSelectGrey =		{178,178,178,	255};
/// BeTitleBarYellow
static const rgb_color BeTitleBarYellow =		{255,203,  0,	255};
//@}

/** @name Common colors */
//@{
/// Black
static const rgb_color Black =					{  0,  0,  0,	255};
/// White
static const rgb_color White =					{255,255,255,	255};
/// Red
static const rgb_color Red =					{255,  0,  0,	255};
/// Green
static const rgb_color Green =					{  0,167,  0,	255};
/// LightGreen
static const rgb_color LightGreen =				{ 90,240, 90,	255};
/// Blue
static const rgb_color Blue =					{ 49, 61,225,	255};
/// LightBlue
static const rgb_color LightBlue =				{ 64,162,255,	255};
/// Purple
static const rgb_color Purple =					{144, 64,221,	255};
/// LightPurple
static const rgb_color LightPurple =			{166, 74,255,	255};
/// Lavender
static const rgb_color Lavender =				{193,122,255,	255};
/// Yellow
static const rgb_color Yellow =					{255,203,  0,	255};
/// Orange
static const rgb_color Orange =					{255,163,  0,	255};
/// Flesh
static const rgb_color Flesh =					{255,231,186,	255};
/// Tan
static const rgb_color Tan =					{208,182,121,	255};
/// Brown
static const rgb_color Brown =					{154,110, 45,	255};
/// LightMetallicBlue
static const rgb_color LightMetallicBlue =		{143,166,240,	255};
/// MedMetallicBlue
static const rgb_color MedMetallicBlue =		{ 75, 96,154,	255};
/// DarkMetallicBlue
static const rgb_color DarkMetallicBlue =		{ 78, 89,126,	255};
//@}

/** @name Additional colors */
//@{
/// LightYellow
static const rgb_color LightYellow =			{255,255,152,	255};
//@}

#endif
