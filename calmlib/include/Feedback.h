/*
	Author:			Adriaan Tijsseling (AGT)
	Copyright: 		(c) Copyright 2002-2013 Adriaan Tijsseling. All rights reserved.
	Description:	Abstract class definition for a Feedback module
*/


#ifndef __FEEDBACK__
#define __FEEDBACK__

#include "Module.h"


class Feedback : public Module
{
public:

	Feedback() { mModuleSize = 0; mNumInConn = 0; mModuleType = O_FB; mFeedback = 0; }
	~Feedback() {}
	
	void 		Initialize( int mModuleSize, char* moduleName, data_type* pars, int mtype, int idx );
	void		UpdateActivation( void );
	void		UpdateWeights( data_type &dw_sum );

	inline void	SetFeedback( int fb ) { mFeedback = fb; }
	inline int	GetFeedback( void ) { return mFeedback; }
	
protected:

	int		mFeedback;	// index of R-node designated to be the winner
};

#endif
