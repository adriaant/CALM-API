/*
	Author:			Adriaan Tijsseling (AGT)
	Copyright: 		(c) Copyright 2002-2013 Adriaan Tijsseling. All rights reserved.
	Description:	Implementation for CALMUnit class
*/

#include "CALMGlobal.h"
#include "Utilities.h"
#include "CALMUnit.h"

// Applies activation function to new input: Used in all kinds of nodes. 
void CALMUnit::Update( void )
{
	data_type decay = ( 1.0 - mParameters[K_A] ) * mActCurrent;
	
	if ( mActNew >= 0.0 )
		mActNew = decay +  ( mActNew / ( 1.0 + mActNew ) ) * ( 1.0 - decay );
	else
		mActNew = decay + ( mActNew / ( 1.0 - mActNew ) ) * decay;
}

