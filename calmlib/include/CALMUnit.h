/*
	Author:			Adriaan Tijsseling (AGT)
	Copyright: 		(c) Copyright 2002-2013 Adriaan Tijsseling. All rights reserved.
	Description:	Abstract class definition for a generic CALM unit
*/

#ifndef __CALMUNIT__
#define __CALMUNIT__

class CALMUnit 
{

public:

	CALMUnit() { mActCurrent = 0.0; mActNew = 0.0; }
	~CALMUnit() {}

	inline void 		Reset( void ) { mActCurrent = 0.0; mActNew = 0.0; }
	virtual void		SetActivation( data_type ) = 0;
	void				Update( void );
	inline void			Swap( void ) { mActCurrent = mActNew; }
	inline data_type 	GetActivation( void ) { return mActCurrent; }
	inline void			SetParameter( data_type* pars ) { mParameters = pars; }
	
protected:

	data_type	mActCurrent;	// Current unit activation
	data_type	mActNew;		// New unit activation
	data_type*	mParameters;	// pointer to Network's parameter storage
};

#endif
