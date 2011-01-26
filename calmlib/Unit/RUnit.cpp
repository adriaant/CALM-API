/*
	Author:			Adriaan Tijsseling (AGT)
	Copyright: 		(c) Copyright 2002-2011 Adriaan Tijsseling. All rights reserved.
	Description:	Implementation for CALMUnit subclass
*/

#include "CALMGlobal.h"
#include "RUnit.h"
#include <iostream>
using namespace std;

void RUnit::Reset( int win )
{
	if ( win & O_TIME )	// for delay connections only
	{
		if ( mClamped == false )
		{
			mActDelay = 0.0;
			mActCurrent = 0.0;
			mActNew = 0.0;
		}
	}
	else if ( win & O_WT )	// resets stored long term activation
	{
		mVCounter = 0; 
		mPotential = 0;
	}
	else
	{
		if ( mClamped == false )
		{
			mActDelay = mActCurrent; // make sure to store activation at time delay
			mActCurrent = 0.0;
			mActNew = 0.0;
		}
	}
}


// force update of time-delay activation (for TESTING only)
void RUnit::UpdateTimeDelay( void )
{
	if ( mClamped == false )
		mActDelay = mActCurrent;
}


// needed to erase past inputs in input modules
void RUnit::ResetInput( void )
{
	if ( mClamped ) return;
	mActDelay = 0.0;
	mActCurrent = 0.0;
}


// internal routine
void RUnit::SetActivation( data_type inAct )
{
	if ( mClamped ) return;
	mActNew = inAct;
	Update();
}


// clamps a unit
void RUnit::ClampUnit( data_type val )
{
	mClamped = true;
	mActCurrent = mActNew = mActDelay = val;
}


void RUnit::Potential( data_type act )
{
	// variation of moving average
	mPotential = mPotential * mVCounter + mActNew * act;
	mVCounter += 1;
	mPotential = mPotential / mVCounter;
}


void RUnit::operator=( RUnit &source )
{
	if ( this == &source ) return;

	mActCurrent = source.mActCurrent;
	mActNew = source.mActNew;
	mActDelay = source.mActDelay;
	mPotential = source.mPotential;
	mVCounter = source.mVCounter;
	mClamped = mClamped;
}

