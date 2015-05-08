/*
	Author:			Adriaan Tijsseling (AGT)
	Copyright: 		(c) Copyright 2002-2015 Adriaan Tijsseling. All rights reserved.
	Description:	API interface for the Categorization And Learning Module Network
*/

#ifndef __GLOBAL__
#define __GLOBAL__

// define the data type to use for calculations. Note that most compilers
// will implicitly convert float to double. If you use float, you will need
// to explicitly suppress these automatic conversions or revert to using
// double. As I prefer double precision, I'll define double as the datatype to use

#define		data_type		float

// already defined on MacOS, so will be defined as zero on other platforms
#ifndef	TARGET_OS_MAC
	#define 	TARGET_OS_MAC   0
#endif

#if !TARGET_OS_MAC
	#define         SInt16                  int
	#define         SInt32                  long
	#define         nil                     NULL
#endif

// Quick access to maximum and minimum values
#define		Max(a,b)	(a>b?a:b)
#define		Min(a,b)	(a<b?a:b)

#define		kUndefined	-1
#define		kTo 		1
#define 	kFrom		-1

// an identifier to notify if convergence times (in # iterations) should also be printed
// when printing out the winners. Set it to 1 if you want to know the convergence times
#define PRINT_TIME 0

// Error codes for file loading: internal use only
enum
{
	kNoErr = 0,
	kCALMFileError = 1
};


// log dir routine identifiers: internal use only
enum
{
	kLogDir = 0,
	kOriginalDir = 1
};


// Reset Options
enum 
{
	O_ACT	= 0x01,		// reset activations
	O_WT	= 0x02,		// reset weights
	O_WIN	= 0x04,		// reset winner infos maintained in Network class
	O_TIME	= 0x08		// reset time delay
};

// Switch types
enum
{
	kOff			= 0,	// turn off
	kOn				= 1		// turn on
};

// Pattern presentation types and other miscellaneous identifiers
enum
{
	kNoWinner		= -1,	// indicates module has not converged
	kLinear			= 0,	// each pattern presented in sequence
	kPermuted		= 1,	// permute pattern sets
	kNormalLink		= 2,	// standard connection
	kDelayLink		= 3		// time delay connection
};

// Intelligible macro to pass to the SpeedTest function. This calculates
// the amount of time a simulation took between, of course, kStart and kEnd
enum
{
	kStart = 0,
	kEnd = 1
};

// Module types (expandable)
enum
{
	O_CALM	= 0x01,		// vanilla CALM
	O_MAP	= 0x02,		// CALMMap
	O_INP	= 0x04,		// input module
	O_FB	= 0x08		// feedback module
};

// parameters for internal module weights, learning, activation, and more!
const int gNumPars = 27;				// expandable

// Parameter identifiers
enum
{
	UP = 0,
	DOWN,
	CROSS,
	FLAT,
	HIGH,
	LOW,
	AE,
	ER,
	INITWT,
	LOWCRIT,
	HIGHCRIT,
	K_A,
	K_Lmax,
	K_Lmin,
	L_L,
	D_L,
	WMUE_L,
	G_L,
	G_W,
	F_Bw,		// Feedback weight update
	F_Ba,		// Feedback activation enforcer
	SIGMA,
	P_G,		// Grow parameter
	P_S,		// Shrink parameter
	U_L,
	AMAP,
	BMAP
};	

// For cout
enum
{
	kTab,
	kReturn,
	kIntend,
	kSpace
};

// Verbosity options
enum 
{
	O_NONE 			= 0x00,		// silent mode
	O_WINNER		= 0x01,		// all data
	O_ACTASIS		= 0x02,		// output as is
	O_ACTPLUS	 	= 0x04,		// formatted output (when training files it only reports
								// activations at the end of each epoch)
	O_POT			= 0x08,		// potential of R-nodes
	O_WEIGHTS 		= 0x10,		// weights
	O_SAVEDWT		= 0x20,		// save changes in weights to file
	O_SAVEACT		= 0x40,		// save total act to file
	O_SAVEMU		= 0x80		// save learning rate change
};


#endif

