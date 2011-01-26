/*
	Author:			Adriaan Tijsseling (AGT)
	Copyright: 		(c) Copyright 2002 Adriaan Tijsseling. All rights reserved.
	Description:	Implementation for Feedback class
*/

#include "CALMGlobal.h"
#include "Feedback.h"
#include "Connection.h"


// Initialize the basic members of a module
// Derived classes need to call this function before doing own Initialization routine
void Feedback::Initialize( int moduleSize, char* moduleName, data_type* pars, int mtype, int idx )
{
	Module::Initialize( moduleSize, moduleName, pars, mtype, idx );
	// create the map weights matrix
	mFeedback = 0;
}


// Update activations in the module
void Feedback::UpdateActivation( void )
{
	// in case there is no feedback information, update the module as a normal CALM
	if ( mFeedback == kNoWinner )
	{
		Module::UpdateActivation();
		return;
	}
	
	// otherwise, we proceed designing a winner
	
	data_type	totalVact, totalRact, newAct;
	int			i;
	
	// first we record the sum of V-node activations and R-node activations
	totalVact = 0.0;
	totalRact = 0.0;
	for ( i = 0; i < mModuleSize; i++ )
	{
		totalVact += mV[i].GetActivation();
		totalRact += mR[i].GetActivation();
	}
	
	// update R-node activations
	for ( i = 0; i < mModuleSize; i++ )
	{
		// loop through incoming connections
		newAct = 0.0;
		for ( int k = 0; k < mNumInConn; k++ )
			newAct += (mInConn[k].*mInConn[k].mWeightedAct)( i );
		// weighted V-node acts
		newAct += mParameters[CROSS] * ( totalVact - mV[i].GetActivation() );
		newAct += mParameters[DOWN] * mV[i].GetActivation();

	// Provide feedback to node
		// Basically this will amplify the designated winner's node by the fixed parameter ER
		// multiplied by - a for now fixed - value (but may become a parameter) and increment the 
		// activation of all other nodes with a negative value. We do this instead of simply setting
		// one R-node to 1 and others to 0 in order to have a more natural weight update that will not
		// stifle feedback-free categorization. 
		if ( i == mFeedback )
			newAct += mParameters[ER] * mParameters[F_Ba];
		else
			newAct -= mParameters[ER] * mParameters[F_Ba];
		
		// run activation function on the new inputs
		mR[i].SetActivation( newAct );
		mR[i].Potential( mE.GetActivation() );
	}
	
	// update V-node activations
	for ( i = 0; i < mModuleSize; i++ )
	{
		newAct = 0.0;		
		// from paired R-node
		newAct += mParameters[UP] * mR[i].GetActivation();
		// from other V-nodes
		newAct += mParameters[FLAT] * ( totalVact - mV[i].GetActivation() );	

	// Provide feedback to node
		// Basically this will amplify the designated winner's node by the fixed parameter ER
		// multiplied by - a for now fixed - value (but may become a parameter) and increment the 
		// activation of all other nodes with a negative value. We do this instead of simply setting
		// one R-node to 1 and others to 0 in order to have a more natural weight update that will not
		// stifle feedback-free categorization. 
		if ( i == mFeedback )
			newAct += mParameters[ER] * mParameters[F_Ba];
		else
			newAct -= mParameters[ER] * mParameters[F_Ba];

		// run activation function on the new inputs
		mV[i].SetActivation( newAct );
	}

	// update A- and E-node
	mA.SetActivation( totalRact, totalVact );
	mE.SetActivation( mA.GetActivation() );
}


// Function to update the weights on all incoming connections
void Feedback::UpdateWeights( data_type &dw_sum )
{
	// in case there is no feedback information, update the module as a normal CALM
	if ( mFeedback == kNoWinner )
	{
		Module::UpdateWeights( dw_sum );
		return;
	}

	data_type	backAct, E;
	int			i, k;
	
	// dynamic Gaussian learning rate
	E = ( mE.GetActivation() - mParameters[G_L] ) * ( mE.GetActivation() - mParameters[G_L] );
	mMu = mParameters[D_L] + mParameters[WMUE_L] * ( 1.0 - E / mParameters[G_W] );
	mMu = Max( mMu, 0.0 );
	
	// learning rate down-adjustment for feedback module may be necessary in order for 
	// the feedback information to overcome possibly ambiguous "perceptual" information
	mMu = mMu / mParameters[F_Bw];

	for ( i = 0; i < mModuleSize; i++ )
	{
		// get all incoming weighted activations
		backAct = 0.0;
		for ( k = 0; k < mNumInConn; k++ )
			backAct += (mInConn[k].*mInConn[k].mWeightedAct)( i );

		// update each connection separately
		// note that background activation applies to each connection
		for ( k = 0; k < mNumInConn; k++ )
			(mInConn[k].*mInConn[k].mUpdate)( i, mR[i].GetActivation(), mMu, backAct, dw_sum );
	}		
}

