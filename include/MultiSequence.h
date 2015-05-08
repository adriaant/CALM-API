/*
	Author:			Adriaan Tijsseling (AGT)
	Copyright: 		(c) Copyright 2002-2015 Adriaan Tijsseling. All rights reserved.
	Description:	Class definition for multiple sequence learning using CALM networks
					with time-delay connections
*/

#ifndef __MULTISEQCALM__
#define __MULTISEQCALM__

#include <stdio.h>
#include "CALMGlobal.h"
#include "Utilities.h"
#include "CALM.h"			// the interface file to the CALM API Library


class MultiSequence
{
public:

	MultiSequence( void );
	MultiSequence( int numfiles, int fileIdx, int epochs, const char* fbname );
	~MultiSequence();
	
	int			LoadPatternFiles( void );
	bool		RunMultiSequenceSimulation( void );
	void		TrainCurrentSequence( int idx );
	void		TestCurrentSequence( int idx, int* winner );
	void		ClampTest( void );
	void		OscillationTest( void );
	void		WinnerTest( void );
	
protected:

	data_type***	mPatterns;			// array of pattern matrices
	int*			mNumPats;
	int				mNumFiles;			// number of pattern files
	int				mFileIdx;			// index of file to start training with
	int				mOutIdx;			// reference to feedback module
	int				mEpochs;			// max number of epochs to train
};

#endif
