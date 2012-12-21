/*
	Author:			Adriaan Tijsseling (AGT)
	Copyright: 		(c) Copyright 2002-2013 Adriaan Tijsseling. All rights reserved.
	Description:	Class definition for a CALM External unit
*/


#ifndef __EUNIT__
#define __EUNIT__

#include "CALMUnit.h"

class EUnit : public CALMUnit
{

public:

	EUnit() {}
	~EUnit() {}

	data_type	RandomizedActivation( void );
	void		SetActivation( data_type actA );
};

#endif
