/*
	Author:			Adriaan Tijsseling (AGT)
	Copyright: 		(c) Copyright 2002-2015 Adriaan Tijsseling. All rights reserved.
	Description:	Implementation for CALMUnit subclass
*/

#include "CALMGlobal.h"
#include "EUnit.h"
#include "Rnd.h"

// NOTE: E-node activation swapped immediately
void EUnit::SetActivation( data_type actA )
{ 
	mActNew = mParameters[AE] * actA; 
	Update(); 
}

// Activates R-units using random noise
data_type EUnit::RandomizedActivation( void ) 
{ 
	return( PseudoRNG() * mParameters[ER] * mActCurrent );
}
