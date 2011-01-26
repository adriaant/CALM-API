/*
	Author:			Adriaan Tijsseling (AGT)
	Copyright: 		(c) Copyright 2002-2011 Adriaan Tijsseling. All rights reserved.
	Description:	Implementation for ModuleMap class
*/

#include "CALMGlobal.h"
#include "Utilities.h"
#include "ModuleMap.h"
#include "Connection.h"

ModuleMap::~ModuleMap()
{
	// delete the map weights
	DisposeMatrix( mMapWeights, mModuleSize );
}


// Initialize the basic members of a module
// Derived classes need to call this function before doing own Initialization routine
void ModuleMap::Initialize( int moduleSize, char* moduleName, data_type* pars, int mtype, int idx )
{
	Module::Initialize( moduleSize, moduleName, pars, mtype, idx );
	// create the map weights matrix
	mMapWeights = CreateMatrix( 0.0, moduleSize, moduleSize );
	
	// set the inhibition weights
	SetInhibitionMap();
	
}


// Set the inhibition map of the V-node weights
void ModuleMap::SetInhibitionMap( void )
{
	int			i, j, middle, dist;
	data_type	denom, sigma;

	// set the values for the map weights
	middle = (int)floor( mModuleSize / 2.0 );	// find half width of module
	denom = (data_type)(mModuleSize);

	// calculate optimal sigma
	sigma = (-4.0/denom) * log( ( 0.01 + exp( - 0.25 * denom ) ) / (denom+1.0) );
	cerr << sigma << endl;

	for ( i = 0; i < mModuleSize; i++ )
	{
		for ( j = 0; j < mModuleSize; j++ )
		{
			dist = SafeAbs( i, j );	// get distance between R and V node
			if ( dist > middle ) dist = mModuleSize - dist;	// correct for size

			// in this function SIGMA depends on module size. A module size up to 20 has
			// experimentally been defined to have an optimal sigma around 0.06. With more nodes,
			// this sigma slightly increases. With 64 nodes, sigma should be picked around 0.15
			mMapWeights[i][j] = (denom+1.0) * 
				exp( 0.0 - ( sigma*dist*dist ) / denom ) - denom - 1.0 + mParameters[DOWN];

			// the old version, published in Phaf et al. Somewhat less elegant.
			// Its sigma can range between 1 and 15 for good results. AMAP and BMAP
			// need to be defined such that BMAP < AMAP and AMAP - BMAP is of a reasonably
			// wide range. 
//			mMapWeights[i][j] = 8.8 * exp( 0.0 - (dist*dist)/(2*sigma*sigma)) - 10.0;
		}		
	}
/*
	cerr << "map for '" << GetModuleName() << "': ";
	AdjustStream( cerr, 3, 0, kLeft, true );
	for ( i = 0; i < mModuleSize; i++ ) cerr << mMapWeights[i][0] << ' ';
	cerr << endl;
	SetStreamDefaults( cerr );
*/
}


// Update activations in the module
void ModuleMap::UpdateActivation( void )
{
	data_type	totalVact, totalRact, newAct;
	int			i, j, k;
	
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
		for ( k = 0; k < mNumInConn; k++ )
			newAct += (mInConn[k].*mInConn[k].mWeightedAct)( i );
		
		// weighted V-node acts
		for ( j = 0; j < mModuleSize; j++ )
			newAct += mMapWeights[i][j] * mV[j].GetActivation();
		
		// Get E-node activation (with random noise)
		newAct += mE.RandomizedActivation();
		
		// Run activation function on the new inputs
		mR[i].SetActivation( newAct );
	}
	
	// update V-node activations
	for ( i = 0; i < mModuleSize; i++ )
	{
		newAct = 0.0;		
		// from paired R-node
		newAct += mParameters[UP] * mR[i].GetActivation();
		// from other V-nodes
		newAct += mParameters[FLAT] * ( totalVact - mV[i].GetActivation() );
	
		// Run activation function on the new inputs
		mV[i].SetActivation( newAct );
	}

	// update A- and E-node
	mA.SetActivation( totalRact, totalVact );
	mE.SetActivation( mA.GetActivation() );
}


// Update activations in the module
void ModuleMap::UpdateActivationTest( void )
{
	data_type	totalVact, totalRact, newAct;
	int			i, j, k;
	
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
		for ( k = 0; k < mNumInConn; k++ )
			newAct += (mInConn[k].*mInConn[k].mWeightedAct)( i );
		
		// weighted V-node acts
		for ( j = 0; j < mModuleSize; j++ )
			newAct += mMapWeights[i][j] * mV[j].GetActivation();
		
		// Get E-node activation (with random noise)
	//	newAct += mE.RandomizedActivation();
		
		// Run activation function on the new inputs
		mR[i].SetActivation( newAct );
	}
	
	// update V-node activations
	for ( i = 0; i < mModuleSize; i++ )
	{
		newAct = 0.0;		
		// from paired R-node
		newAct += mParameters[UP] * mR[i].GetActivation();
		// from other V-nodes
		newAct += mParameters[FLAT] * ( totalVact - mV[i].GetActivation() );
	
		// Run activation function on the new inputs
		mV[i].SetActivation( newAct );
	}

	// update A- and E-node
	mA.SetActivation( totalRact, totalVact );
	mE.SetActivation( mA.GetActivation() );
}


// Function to determine winning nodes in the module
// For CALMMap it is more accurate to use the V-nodes
void ModuleMap::ConvCheck( int t, int* winner, int* convtime )
{
	int		i, num, win;
	
	// first get a count of nodes with activation above LOWCRIT
	num = 0;
	for ( i = 0; i < mModuleSize; i++ )
	{
		if ( mV[i].GetActivation() >= mParameters[LOWCRIT] )
		{
			num++;
			if ( num > 1 ) break;
			win = i;
		}
	}	
	// if there is only one candidate, check if it matches HIGHCRIT
	// otherwise set winner to non-existent idx
	if ( num != 1 )
	{
		SetWinner( kNoWinner );
		SetConvTime( kNoWinner );
	}	
	else if ( mV[win].GetActivation() >= mParameters[HIGHCRIT] )
	{
		if ( win != mWinner )	// perhaps converged before?
		{
			SetWinner( win );
			SetConvTime( t );	// record the convergence time (in iterations)
		}
	}	
	// pass results back
	*winner = GetWinner();
	*convtime = GetConvTime();
}


