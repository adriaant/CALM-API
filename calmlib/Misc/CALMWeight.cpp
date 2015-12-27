/*
	Author:			Adriaan Tijsseling (AGT)
	Copyright: 		(c) Copyright 2002-2015 Adriaan Tijsseling. All rights reserved.
	Description:	Implementation of CALMWeight class
*/

#include "CALMWeight.h"


void CALMWeight::SetWeight( data_type newWeight, data_type maxval, data_type minval )
{
	// record weight changes
	mWeightChange = newWeight;
	
	// apply weight update rule
	mWeightValue = Max( Min( mWeightValue + mWeightChange, maxval ), minval );
}


void CALMWeight::operator=( CALMWeight &source )
{
	if ( this == &source ) return;
	
	mWeightValue = source.mWeightValue;
	mWeightChange = source.mWeightChange;
	mClamped = source.mClamped;
}

