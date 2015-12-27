/*
	Author:			Adriaan Tijsseling (AGT)
	Copyright: 		� Copyright 2002-2015 Adriaan Tijsseling. All rights reserved.
	Description:	CALMWeight class. Every connection between modules has a matrix of weights
*/


#ifndef __CALMWEIGHT__
#define __CALMWEIGHT__

#include "CALMGlobal.h"


class CALMWeight
{

public:

	CALMWeight() { mWeightValue = 0.0; mWeightChange = 0.0; mClamped = false; }
	CALMWeight( data_type resetValue ) { mWeightValue = resetValue; mWeightChange = 0.0; mClamped = false; }
	~CALMWeight() {}
	
	// Reset function. Either default or with supplied reset value
	inline void			Reset( data_type resetValue ) { mWeightValue = resetValue; mWeightChange = 0.0; }

	// CALMWeight adaptation function
	inline void 		AdaptWeight( data_type deltaWeight ) { mWeightValue += deltaWeight; }
	
	// Accessor functions
	inline data_type	GetWeight( void ) { return mWeightValue; }
	inline data_type	GetWeightChange( void ) { return mWeightChange; }
	void				SetWeight( data_type newWeight, data_type maxval, data_type minval );
	inline void			SetWeight( data_type newWeight ) { mWeightValue = newWeight; }
	inline bool			IsClamped( void ) { return mClamped; }
	
	void operator=(CALMWeight &source);
	
private:
	
	// well, duh, the value of the weight...
	data_type			mWeightValue;
	data_type			mWeightChange;
	// for future use: clamping weight values for analyses
	bool				mClamped;
};

#endif
