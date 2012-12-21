/*
	Author:			Adriaan Tijsseling (AGT)
	Copyright: 		(c) Copyright 2002-2013 Adriaan Tijsseling. All rights reserved.
	Description:	Class definition for a CALM Veto unit
*/


#ifndef __VUNIT__
#define __VUNIT__

#include "CALMUnit.h"

class VUnit : public CALMUnit
{

public:

	VUnit() { mClamped = false; }
	~VUnit() {}

	void 			Reset( void );
	void			SetActivation( data_type inAct );
	void			ClampUnit( data_type val );
	inline void		ClampUnit( void ) { mClamped = false; }
	inline bool		IsClamped( void ) { return mClamped; }

protected:

	bool		mClamped;		// is the activation of this cell clamped?
};

#endif
