/*
	Author:			Adriaan Tijsseling (AGT)
	Copyright: 		(c) Copyright 2002-2015 Adriaan Tijsseling. All rights reserved.
	Description:	Abstract class definition for a CALMMap module
*/


#ifndef __MODULEMAP__
#define __MODULEMAP__

#include "Module.h"


class ModuleMap : public Module
{
public:

	ModuleMap() { mModuleSize = 0; mNumInConn = 0; mModuleType = O_MAP; }
	~ModuleMap();
	
	void 		Initialize( int mModuleSize, char* moduleName, data_type* pars, int mtype, int idx );
	void		SetInhibitionMap( void );
	void		UpdateActivation( void );
	void		UpdateActivationTest( void );
	void		ConvCheck( int t, int* winner, int* convtime );
	
protected:

	data_type**		mMapWeights;	// matrix holding the V-weights
};

#endif
