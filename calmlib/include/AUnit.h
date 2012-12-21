/*
	Author:			Adriaan Tijsseling (AGT)
	Copyright: 		(c) Copyright 2002-2013 Adriaan Tijsseling. All rights reserved.
	Description:	Class definition for a CALM Arousal unit
*/


#ifndef __AUNIT__
#define __AUNIT__

#include "CALMUnit.h"

class AUnit : public CALMUnit
{

public:

	AUnit() {}
	~AUnit() {}

	inline void	SetActivation( data_type ) { cerr << "Illegal function use: AUnit::SetActivation( data_type )" << endl; }
	// NOTE: A-node activation swapped immediately. 
	// After all, E-node should base stimulation on current competition, not a past one. 
	void		SetActivation( data_type actR, data_type actV );
};

#endif
