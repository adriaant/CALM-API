/*
	Author:			Adriaan Tijsseling (AGT)
	Copyright: 		(c) Copyright 2002-2011 Adriaan Tijsseling. All rights reserved.
	Description:	Implementation for CALMUnit subclass
*/

#include	<iostream>
using namespace std;
#include "CALMGlobal.h"
#include "AUnit.h"

void AUnit::SetActivation( data_type actR, data_type actV )
{
	mActNew = mParameters[HIGH] * actV + mParameters[LOW] * actR;
	Update();
}

