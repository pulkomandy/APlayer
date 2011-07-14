/******************************************************************************/
/* PDebug header file.                                                        */
/******************************************************************************/
/* This source, or parts thereof, may be used in any software as long the     */
/* license of PolyKit is keep. See the LICENSE file for more information.     */
/*                                                                            */
/* Copyright (C) 1998-2002 by Polycode.                                       */
/* All rights reserved.                                                       */
/******************************************************************************/


#ifndef __PDebug_h
#define __PDebug_h

// Uncomment this block if you want to build a debug version via the project files
//#ifndef _DEBUG
//#define _DEBUG
//#endif

// Should we build a debug version?
#ifdef _DEBUG
#define PDEBUG
#endif


/******************************************************************************/
/* Define macros                                                              */
/******************************************************************************/

//
// BeOS versions
//
#if __p_os == __p_beos

#ifdef PDEBUG

//
// DEBUG VERSIONS
//
#undef ASSERT
#undef VERIFY

/******************************************************************************/
/**	The ASSERT macro indicates that the specified condition is expected to be
 *	true at this point in the program. In the debug version, the ASSERT macro
 *	evaluates its argument. If the result is false (0), the macro prints a
 *	diagnostic message and halts the program. If the condition is true, it does
 *	nothing. In the release version, ASSERT does not evaluate the expression
 *	and thus will not interrupt the program. If the expression must be
 *	evaluated regardless of enviroment, use the VERIFY macro in place of
 *	ASSERT.
 *
 *	@param c is the condition that is expected to be true.
 *//***************************************************************************/
#define ASSERT(c) \
	{ \
		if (!(c)) \
		{ \
			char buf[1024]; \
			sprintf(buf, "Assertion (%s) failed in \"%s\" on line %d\nProgram will be aborted!!\n", #c, __FILE__, __LINE__); \
			printf(buf); \
			debugger(buf); \
			abort(); \
		} \
	}



/******************************************************************************/
/**	The VERIFY macro indicates that the specified condition is expected to be
 *	true at this point in the program. In the debug version, the VERIFY macro
 *	evaluates its argument. If the result is false (0), the macro prints a
 *	diagnostic message and halts the program. If the condition is true, it does
 *	nothing. In the release version, VERIFY evaluates the expression but does
 *	not print or interrupt the program. For example, if the expression is a
 *	function call, the call will be made.
 *
 *	@param c is the condition that is expected to be true.
 *//***************************************************************************/
#define VERIFY(c)		ASSERT(c)

#else

//
// RELEASE VERSIONS
//
#undef ASSERT
#undef VERIFY

#define ASSERT(c)		((void)0)
#define VERIFY(c)		((void)(c))

#endif

#endif

#endif
