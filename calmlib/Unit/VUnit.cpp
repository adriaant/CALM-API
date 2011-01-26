/*
	Author:			Adriaan Tijsseling (AGT)
	Copyright: 		(c) Copyright 2002-2011 Adriaan Tijsseling. All rights reserved.
	Description:	Implementation for CALMUnit subclass
*/

#include "CALMGlobal.h"
#include "VUnit.h"

void VUnit::Reset( void )
{
	if ( mClamped == false )
	{
		mActCurrent = 0.0;
		mActNew = 0.0;
	}
}


void VUnit::SetActivation( data_type inAct )
{
	if ( mClamped ) return;
	mActNew = inAct;
	Update();
}

// clamps a unit
void VUnit::ClampUnit( data_type val )
{
	mClamped = true;
	mActCurrent = mActNew = val;
}

