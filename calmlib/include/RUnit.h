/*
	Author:			Adriaan Tijsseling (AGT)
	Copyright: 		(c) Copyright 2002-2015 Adriaan Tijsseling. All rights reserved.
	Description:	Class definition for a CALM Representation unit
*/


#ifndef __RUNIT__
#define __RUNIT__

#include "CALMUnit.h"

class RUnit : public CALMUnit
{

public:

	RUnit() { mActDelay = 0.0; mVCounter = 0; mPotential = 0; mClamped = false; }
	~RUnit() {}

	inline void			SetInput( data_type inAct ) { mActDelay = mActCurrent; mActCurrent = inAct; }
	void				SetActivation( data_type inAct );

	void				ClampUnit( data_type val );
	inline void			ClampUnit( void ) { mClamped = false; }
	inline bool			IsClamped( void ) { return mClamped; }
	
	void 				Reset( int win );
	void				ResetInput( void );
	void				UpdateTimeDelay( void );
	
	void				Potential( data_type act );
	inline data_type	GetPotential( void ) { return mPotential; }
		
	inline data_type 	GetDelayAct( void ) { return mActDelay; }
	inline void			SetDelayAct( void ) { mActDelay = mActCurrent; }

	void operator=(RUnit &source);
	
protected:

	data_type	mActDelay;		// Unit activation at time t-delay
	data_type	mPotential;		// Internal potential for measuring cell's 
								// activity over time. Long inactivity will
								// prune the cell
	int			mVCounter;
	bool		mClamped;		// is the activation of this cell clamped?
};

#endif
