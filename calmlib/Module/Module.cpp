/*
	Author:			Adriaan Tijsseling (AGT)
	Copyright: 		(c) Copyright 2002-2015 Adriaan Tijsseling. All rights reserved.
	Description:	Implementation for Module class
*/

#include <string.h>
#include "CALMGlobal.h"
#include "Utilities.h"
#include "Module.h"
#include "Connection.h"


Module::~Module()
{
	// Clean up the R and V arrays
	if ( mModuleSize != 0 )
	{
		delete[] mR;
		delete[] mV;
	}
	if ( mNumInConn != 0 ) delete[] mInConn;
}


// Initialize the basic members of a module
// Derived classes need to call this function before doing own Initialization routine
void Module::Initialize( int moduleSize, char* moduleName, data_type* pars, int mtype, int idx )
{
	mModuleIndex = idx;
	mModuleSize = moduleSize;
	strcpy( mModuleName, moduleName );
	mParameters = pars;
	mModuleType = mtype;
	
	// Initialize R and V layers
	mR = new RUnit[mModuleSize];
	mV = new VUnit[mModuleSize];
	
	for ( int i = 0; i < mModuleSize; i++ )
	{
		mR[i].SetParameter( mParameters );
		mV[i].SetParameter( mParameters );
	}
	mA.SetParameter( mParameters );
	mE.SetParameter( mParameters );
}


// Set the number of incoming connections and allocate array
void Module::SetNumConn( int numInConn )
{
	mNumInConn = numInConn;
	// Initialize array of incoming connections
	if ( mNumInConn != 0 ) mInConn = new Connection[mNumInConn];
}


// Set one connection
void Module::Connect( int idx, Module* fromModule, int link, int delay )
{
	mInConn[idx].Initialize( fromModule, &mModuleSize, link, delay, mParameters );
}


// check if a module needs to grow or shrink, depending on each R-nodes internal potential
bool Module::NeedsResizing( int* node )
{
	int			i;
	bool 		resiz = false;
	data_type	maxval = 0.0;
	data_type	nextmax = 0.0;
	data_type	tmpval;
	
	for ( i = 0; i < mModuleSize; i++ )
	{
		// find max and runnerup
		tmpval = mR[i].GetPotential();
		if ( tmpval > maxval )
		{
			nextmax = maxval;
			maxval = tmpval;
		}
		else if ( tmpval > nextmax && tmpval < maxval )
			nextmax = tmpval;
		
		// check if potential is below threshold. 
		// if so, mark node for deletion (single node at a time)
		if ( mR[i].GetPotential() < mParameters[P_S] ) 
		{
			*node = i;
		}
	}
	// compare max with runnerup
	tmpval = ( ( maxval - nextmax ) / maxval ) * 100.0;
	if ( tmpval >= mParameters[P_G] ) resiz = true;
	
	// no growing and shrinking at the same time
	if ( resiz && *node != kUndefined )
	{
		resiz = false;
		goto bail;
	}
	
	// set to prune if possible
	if ( *node != kUndefined && mModuleSize > 2 ) 
	{
		resiz = true;
		goto bail;
	}
	
	// indicate the grow factors and return for growing
	if ( resiz && *node == kUndefined )
	{
		cerr << "max vs runnerup: " << maxval << " " << nextmax << " = " << tmpval << endl;
	}
	
bail:
	if ( resiz ) PrintPotentials( &cerr );
	return resiz;
}


// Routine to modify module sizes
void Module::ResizeModule( int newsize, int node )
{
	// resize the R and V arrays
	// we create a temporary array first to preserve data (i.e. delayed activations, potential)	
	RUnit*	newR;
	int		i, k;

	// temporary array to hold old values
	newR = new RUnit[mModuleSize];
	// copy over old values
	for ( i = 0; i < mModuleSize; i++ ) newR[i] = mR[i];

	// reinitialize mR and mV arrays
	delete[] mV;
	delete[] mR;
	mR = new RUnit[newsize];	
	mV = new VUnit[newsize];
	
	// set parameters
	for ( i = 0; i < newsize; i++ )
	{
		mR[i].SetParameter( mParameters );
		mV[i].SetParameter( mParameters );
	}

	// copy over saved data
	if ( node > kUndefined )  // ignore data from pruned node
	{
		k = 0;
		for ( i = 0; i < mModuleSize; i++ )
		{
			if ( i != node )
			{
				mR[k] = newR[i];
				k++;
			}
		}
	}
	else // module is growing or no node is specified
	{
		if ( mModuleSize < newsize )
			for ( i = 0; i < mModuleSize; i++ ) mR[i] = newR[i];
		else
			for ( i = 0; i < newsize; i++ ) mR[i] = newR[i];
	}
	// delete temporary array
	delete[] newR;

	// we need to adjust the weight matrices for incoming connections
	int fromsize;
	for ( k = 0; k < mNumInConn; k++ )
	{
		fromsize = mInConn[k].GetModuleSize();
		mInConn[k].ResizeConnection( fromsize, newsize, node, kTo );
	}	
	mModuleSize = newsize;

// NEW!
	// reset the potentials 
	for ( i = 0; i < mModuleSize; i++ )
	{
		mR[i].Reset( O_WT );
	}
}



// Resize connections from resized module
void Module::ResizeConnection( int newsize, int node, int idx )
{
	for ( int k = 0; k < mNumInConn; k++ )
	{
		// if incoming module is the indicated module, then adjust weight matrix
		if ( mInConn[k].GetModuleIndex() == idx )
			mInConn[k].ResizeConnection( newsize, mModuleSize, node, kFrom );
	}
}


// Function to reset activations and/or weights. 
// Do not override, but supplement instead.
void Module::Reset( SInt16 resetOption )
{
	int	i;
	
	// reset activation values of all nodes
	if ( resetOption & O_ACT )
	{
		for ( i = 0; i < mModuleSize; i++ )
		{
			mR[i].Reset( resetOption );
			mV[i].Reset();
		}
		mA.Reset();
		mE.Reset();
		mWinner = kNoWinner;
		mConvTime = kNoWinner;
		// update internal clock for time delay connections
		// since activations are reset at every input presentation, we for sake of ease
		// take this as one time step
		for ( int k = 0; k < mNumInConn; k++ ) mInConn[k].TickClock();
	}
	
	// make sure winner is not specified
	if ( resetOption & O_WIN )
	{
		mWinner = kNoWinner;
		mConvTime = kNoWinner;
	}
	
	// reset weights in all connections
	if ( resetOption & O_WT )
	{		
		for ( i = 0; i < mNumInConn; i++ ) mInConn[i].Reset();
		for ( i = 0; i < mModuleSize; i++ )
		{
			mR[i].Reset( resetOption );
		}
	}
	
	// reset time delay
	if ( resetOption & O_TIME )
	{
		for ( i = 0; i < mNumInConn; i++ ) mInConn[i].Reset( O_TIME );
		for ( i = 0; i < mModuleSize; i++ ) mR[i].Reset( resetOption );
	}
}

// Function to reset input activations
void Module::Reset( void )
{
	for ( int i = 0; i < mModuleSize; i++ ) mR[i].Reset( O_ACT );
}


// Function to reset past input values
void Module::ResetInput( void )
{
	for ( int i = 0; i < mModuleSize; i++ ) mR[i].ResetInput();
}


void Module::UpdateTimeDelay( void )
{
	for ( int i = 0; i < mModuleSize; i++ ) mR[i].UpdateTimeDelay();
}


inline void Module::SetInput( data_type* input )
{
	for ( int i = 0; i < mModuleSize; i++ )
		mR[i].SetInput( input[i] );
}


inline void Module::SetInput( data_type input, int i )
{
	mR[i].SetInput(input);
}


// Update activations in the module
void Module::UpdateActivation( void )
{
	data_type	totalVact, totalRact, newAct;
	int			i, k;

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
		newAct += mParameters[CROSS] * ( totalVact - mV[i].GetActivation() );
		newAct += mParameters[DOWN] * mV[i].GetActivation();
		
		// Get E-node activation (with random noise)
		newAct += mE.RandomizedActivation();
		
		// Run activation function on the new inputs
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
	
		// Run activation function on the new inputs
		mV[i].SetActivation( newAct );
	}

	// update A- and E-node
	mA.SetActivation( totalRact, totalVact );
	mE.SetActivation( mA.GetActivation() );
}


// Update activations in the module
void Module::UpdateActivationTest( void )
{
	data_type	totalVact, totalRact, newAct;
	int			i, k;

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
		newAct += mParameters[CROSS] * ( totalVact - mV[i].GetActivation() );
		newAct += mParameters[DOWN] * mV[i].GetActivation();
		
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
void Module::UpdateActivationTest( bool useNoise )
{
	data_type	totalVact, totalRact, newAct;
	int			i, k;

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
		if ( mR[i].IsClamped() ) continue;	// do not touch clamped units
		
		// loop through incoming connections
		newAct = 0.0;
		for ( k = 0; k < mNumInConn; k++ )
			newAct += (mInConn[k].*mInConn[k].mWeightedAct)( i );
		// weighted V-node acts
		newAct += mParameters[CROSS] * ( totalVact - mV[i].GetActivation() );
		newAct += mParameters[DOWN] * mV[i].GetActivation();
		
		// Get E-node activation (with random noise)
		if ( useNoise ) newAct += mE.RandomizedActivation();
		
		// Run activation function on the new inputs
		mR[i].SetActivation( newAct );
	}
	
	// update V-node activations
	for ( i = 0; i < mModuleSize; i++ )
	{
		if ( mV[i].IsClamped() ) continue;	// do not touch clamped units

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

// Function to update the weights on all incoming connections
void Module::UpdateWeights( data_type &dw_sum )
{
	data_type	backAct, E;
	int			i, k;
	
	// dynamic Gaussian learning rate
	E = ( mE.GetActivation() - mParameters[G_L] ) * ( mE.GetActivation() - mParameters[G_L] );
	mMu = mParameters[D_L] + mParameters[WMUE_L] * ( 1.0 - E / mParameters[G_W] );
	mMu = Max( mMu, 0.0 );
	
	// this is the original CALM learning rate:
//	mMu = D_L + WMUE_L * mE.GetActivation(); 
	
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


// Swap old activations for new: only necessary for R and V nodes
inline void Module::SwapActs( void )
{
	for ( int i = 0;  i < mModuleSize; i++ )
	{
		mR[i].Swap();
		mV[i].Swap();
	}
	mA.Swap();
	mE.Swap();
}


// Function to determine winning nodes in the module
// For CALMMap it is more accurate to use the V-nodes, but below R-nodes are used
void Module::ConvCheck( int t, int* winner, int* convtime )
{
	int		i, num, win;

	// first get a count of nodes with activation above LOWCRIT
	num = 0;
	for ( i = 0; i < mModuleSize; i++ )
	{
		if ( mR[i].GetActivation() >= mParameters[LOWCRIT] )
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
	else if ( mR[win].GetActivation() >= mParameters[HIGHCRIT] )
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


inline data_type Module::GetWeight( int inConIdx, int i, int j )
{
	return mInConn[inConIdx].GetWeight( i, j );
}

// next few statements won't compile if inlined in Module.h
char*	Module::GetConnModuleName( int idx ) { return mInConn[idx].GetModuleName(); }
int 	Module::GetConnType( int idx ) { return mInConn[idx].GetType(); }
int 	Module::GetConnDelay( int idx ) { return mInConn[idx].GetDelay(); }
void 	Module::CopyWeights( int idx, double** matrix ) { mInConn[idx].CopyWeights( matrix ); }


// returns sum of weight changes on all connections
void Module::SumWeightChanges( data_type &dw_sum )
{
	for ( int i = 0; i < mNumInConn; i++ )
		mInConn[i].SumWeightChanges( dw_sum );
}


void Module::SaveWeights( ofstream *outfile )
{
	for ( int i = 0; i < mNumInConn; i++ )
	{
		*outfile << "# " << mModuleName << " <- " << mInConn[i].GetModuleName() << endl;
#if DYNAGRAPH
		*outfile << "TITLE \"" << mModuleName << " <- " << mInConn[i].GetModuleName() << "\"" << endl;
#endif
		mInConn[i].SaveWeights( outfile );
	}
}


void Module::LoadWeights( ifstream *infile )
{
	for ( int i = 0; i < mNumInConn; i++ )
		mInConn[i].LoadWeights( infile );
}


void Module::PrintWeights( ostream* os )
{
	*os << GetModuleName() << ":" << endl;
	for ( int i = 0; i < mNumInConn; i++ )
		*os << mInConn[i];
}


void Module::SumActivation( data_type &act_sum )
{
	int	i;
	for ( i = 0; i < mModuleSize; i++ ) act_sum += mV[i].GetActivation();
	for ( i = 0; i < mModuleSize; i++ ) act_sum += mR[i].GetActivation();
}

void Module::SumActivationR( data_type &act_sum )
{
	int	i;
	for ( i = 0; i < mModuleSize; i++ ) act_sum += mR[i].GetActivation();
}

void Module::SumActivationV( data_type &act_sum )
{
	int	i;
	for ( i = 0; i < mModuleSize; i++ ) act_sum += mV[i].GetActivation();
}


void Module::PrintActs( ostream* os, int format )
{
	int i;
	int	spacing = (mModuleSize / 10) + 4;
	
	*os << GetModuleName() << endl;
	if ( mModuleType == O_INP )	
	{
		if ( format & O_ACTASIS )
		{
			AdjustStream( *os, 3, 5, kLeft, true );
			for ( i = 0;  i < mModuleSize; i++ )
				*os << mR[i].GetActivation() << ' ';
			*os << endl;
		}
		else
		{
			for ( i = 0;  i < mModuleSize; i++ )
			{
				AdjustStream( *os, 0, 1, kLeft, false );
				PrintRoundedValue( os, mR[i].GetActivation() );
				*os << ' ';
			}
			*os << endl;
		}		
	}
	else
	{
		if ( format & O_ACTASIS )
		{
			AdjustStream( *os, 3, 5, kLeft, true );
			for ( i = 0;  i < mModuleSize; i++ )
				*os << mV[i].GetActivation() << '\t';
			*os << mA.GetActivation() << endl;
			for ( i = 0;  i < mModuleSize; i++ )
				*os << mR[i].GetActivation() << '\t';
			*os << mE.GetActivation() << endl;
			*os << endl;
		}
		else
		{
			for ( i = 0;  i < mModuleSize; i++ )
			{
				AdjustStream( *os, 0, spacing, kLeft, false );
				PrintRoundedValue( os, mV[i].GetActivation() );
			}
			AdjustStream( *os, 0, spacing, kLeft, false );
			PrintRoundedValue( os, mA.GetActivation() );
			*os << endl;
			for ( i = 0;  i < mModuleSize; i++ )
			{
				AdjustStream( *os, 0, spacing, kLeft, false );
				PrintRoundedValue( os, mR[i].GetActivation() );
			}
			AdjustStream( *os, 0, spacing, kLeft, false );
			PrintRoundedValue( os, mE.GetActivation() );
			*os << endl;
		}	
	}
	SetStreamDefaults( *os );
}

void Module::PrintPotentials( ostream* os )
{
	int i;

	*os << GetModuleName() << endl;
	AdjustStream( *os, 3, 6, kLeft, true );
	for ( i = 0;  i < mModuleSize; i++ )
		*os <<  mR[i].GetPotential() << " ";
	*os << endl;
	SetStreamDefaults( *os );
}

void Module::PrintSizes( ostream* os )
{
	AdjustStream( *os, 0, 10, kLeft, false );
	*os << GetModuleName();
	AdjustStream( *os, 0, 1, kLeft, false );
	*os << ": ";
	AdjustStream( *os, 0, 0, kLeft, false );
	*os << GetModuleSize() << endl;
	SetStreamDefaults( *os );
}


void Module::Print( ostream *os )
{
	int i;
	
	*os << mModuleName << endl;
	AdjustStream( *os, 3, 5, kLeft, true );
	for ( i = 0; i < mModuleSize; i++ )
		*os << mV[i].GetActivation() << '\t';
	*os << mA.GetActivation() << endl;
	for ( i = 0; i < mModuleSize; i++ )
		*os << mR[i].GetActivation() << '\t';
	*os << mE.GetActivation() << endl;

	for ( i = 0; i < mNumInConn; i++ )
		*os << mInConn[i];
	
	SetStreamDefaults( *os );
}


// overload the << operator
ostream &operator<<( ostream &os, Module *m )
{
	m->Print( &os );
	return os;
}

