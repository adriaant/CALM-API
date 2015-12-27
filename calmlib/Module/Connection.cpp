/*
	Author:			Adriaan Tijsseling (AGT)
	Copyright: 		(c) Copyright 2002-2015 Adriaan Tijsseling. All rights reserved.
	Description:	Implementation of Connection class
*/

#include "CALMGlobal.h"
#include "Utilities.h"
#include "Connection.h"
#include "Feedback.h"
#include "Rnd.h"

// Free the weight matrix
Connection::~Connection()
{
	for ( int i = 0; i < *mToSize; i++ ) delete[] mWeights[i];
	delete[] mWeights;
	delete[] mWtAct;
}
	

// Make pointer members point to relevant addresses and allocate weights
void Connection::Initialize( Module* inModule, int* toSize, int linkType, int delay, data_type* pars )
{
	mToSize = toSize;
	mInModule = inModule;
	mType = linkType;
	mParameters = pars;
	
	// set weighted delay function according to type of connection
	if ( mType == kNormalLink )
	{
		mWeightedAct = &Connection::WeightedActivation;
		mInAct = &Connection::NormalActivation;
		mUpdate = &Connection::UpdateNormal;
		mDelay = 0;		// not a time-delay
	}
	else
	{
		mWeightedAct = &Connection::WeightedDelay;
		mInAct = &Connection::DelayedActivation;
		mUpdate = &Connection::UpdateDelay;
		// set time delay specifics
		mDelay = delay;		// delay of connection
	}

	mTime = 0;			// "internal clock"
	// local copy of previous calculated weighted activation
	mWtAct = new data_type[*mToSize];
	for ( int i = 0; i < *mToSize; i++ ) mWtAct[i] = 0.0;
	
	// allocate memory for weights
	mWeights = new CALMWeight*[ *mToSize ];
	for ( int i = 0; i < *mToSize; i++ )
	{
		mWeights[i] = new CALMWeight[ mInModule->GetModuleSize() ];
		for ( int j = 0; j < mInModule->GetModuleSize(); j++ )
			mWeights[i][j].Reset( mParameters[INITWT] );
	}
}


void Connection::ResizeConnection( int fromsize, int tosize, int node, int direction )
{
	data_type		wtavg = 0.0;
	CALMWeight**	newWts;
	
	// we need to resize the mWtAct array and the weight matrix
	// for the mWtAct array, we just delete and new. No need to copy data.
	delete[] mWtAct;
	mWtAct = new data_type[tosize];
	for ( int i = 0; i < tosize; i++ ) mWtAct[i] = 0.0;

	// weights have to be copied over
	// we are going to set the new weights to the average of the old weights
	// get this average... 
	// but this is unnecessary if we are downsizing:
	if ( node == kUndefined )
	{
		data_type	curWt;
		data_type	minWt = 1000000;
		data_type	maxWt = -1000000;
				
		for ( int i = 0; i < *mToSize; i++ )
			for ( int j = 0; j < mInModule->GetModuleSize(); j++ )
			{
				curWt = mWeights[i][j].GetWeight();
				wtavg += curWt;
				if ( curWt < minWt ) minWt = curWt;
				if ( curWt > maxWt ) maxWt = curWt;
			}
		wtavg = wtavg / ( (*mToSize) * mInModule->GetModuleSize() );

		// first initialize a new matrix and reset to default
		newWts = new CALMWeight*[ tosize ];
		for ( int i = 0; i < tosize; i++ )
		{
			newWts[i] = new CALMWeight[ fromsize ];
			for ( int j = 0; j < fromsize; j++ )
			{
				curWt = PseudoRNG( minWt, maxWt );
				newWts[i][j].Reset( curWt );
			}
		}
	}
	else
	{	
		// first initialize a new matrix and reset to default
		newWts = new CALMWeight*[ tosize ];
		for ( int i = 0; i < tosize; i++ )
		{
			newWts[i] = new CALMWeight[ fromsize ];
			for ( int j = 0; j < fromsize; j++ )
				newWts[i][j].Reset( 0.0 );
		}
	}
	
	// copy over the old weights
	if ( node > kUndefined ) 
		// we're downsizing, so don't copy the weights from the deleted node
	{
		int k = 0;
		if ( direction == kTo ) // we need to know which layer the node is from
		{
			for ( int i = 0; i < *mToSize; i++ )
			{
				if ( i != node )
				{
					for ( int j = 0; j < mInModule->GetModuleSize(); j++ )
						newWts[k][j] = mWeights[i][j];
					k++;
				}
			}
		}
		else
		{
			for ( int i = 0; i < *mToSize; i++ )
			{
				k = 0;
				for ( int j = 0; j < mInModule->GetModuleSize(); j++ )
				{
					if ( j != node )
					{
						newWts[i][k] = mWeights[i][j];
						k++;
					}
				}
			}
		}		
	}
	else
	{
		int wlim = ( ( tosize > *mToSize ) ? *mToSize : tosize );
		int hlim = ( ( fromsize > mInModule->GetModuleSize() ) ? mInModule->GetModuleSize() : fromsize );
		
		for ( int i = 0; i < wlim; i++ )
			for ( int j = 0; j < hlim; j++ )
				newWts[i][j] = mWeights[i][j];
	}
	
	// make the old pointer point to the new data, after deleting old data first
	for ( int i = 0; i < *mToSize; i++ ) delete[] mWeights[i];
	delete[] mWeights;
	mWeights = newWts;
}


// Reset internal clock and stored value
void Connection::Reset( int val )
{
	if ( val != O_TIME ) return;	
	mTime = 0;
	for ( int i = 0; i < *mToSize; i++ ) mWtAct[i] = 0.0;
}

// Reset weights with custom value
void Connection::Reset( data_type val )
{
	for ( int i = 0; i < *mToSize; i++ )
		for ( int j = 0; j < mInModule->GetModuleSize(); j++ )
			mWeights[i][j].Reset( val );
}

// Reset weights with default value
void Connection::Reset( void )
{
	for ( int i = 0; i < *mToSize; i++ )
		for ( int j = 0; j < mInModule->GetModuleSize(); j++ )
			mWeights[i][j].Reset( mParameters[INITWT] );
}


// Calculated weighted sum of incoming activations for normal connection
data_type Connection::WeightedActivation( int idx )
{
	data_type newAct = 0.0;

	for ( int j = 0; j < mInModule->GetModuleSize(); j++ )
		newAct += GetWeight( idx, j ) * mInModule->GetActivationR( j );
	return newAct;
}


// Calculated weighted sum of incoming activations for time-delay connection
data_type Connection::WeightedDelay( int idx )
{
	// if the internal clock is indicating mDelay updates have passed, get incoming act
	if ( mTime == mDelay )
	{
		mWtAct[idx] = 0.0;
		for ( int j = 0; j < mInModule->GetModuleSize(); j++ )
			mWtAct[idx] += GetWeight( idx, j ) * mInModule->GetDelayAct( j );
	}

	return mWtAct[idx];
}


void Connection::TickClock( void )
{
	mTime = mTime + 1;
	if ( mTime > mDelay ) mTime = 1;	// reset time		
}


// Update weights
void Connection::UpdateNormal( int idx, data_type act, data_type mu, data_type backAct, data_type &dw_sum )
{
	int			j;
	data_type	w, dw, inAct;
	
	for ( j = 0; j < mInModule->GetModuleSize(); j++ )
	{
		w = GetWeight( idx, j );
		inAct = (*this.*mInAct)( j );
	// learning rate up-adjustment for feedback module may be necessary in order for 
	// the feedback information to overcome possibly ambiguous "perceptual" information
		if ( mInModule->GetModuleType() == O_FB )
		{
			if ( dynamic_cast<Feedback*>(mInModule)->GetFeedback() != kNoWinner )
			{
				mu = mu * mParameters[F_Bw];
			}
		}
		
		// apply the Grossberg learning rule
		dw = mu * act * (
			 ( mParameters[K_Lmax] - w ) * inAct - 
			   mParameters[L_L] * ( w - mParameters[K_Lmin] ) * ( backAct - w * inAct ) );

		// Koutnik variant
//		dw = mu * act * (inAct - w);

		// add to sum of weight changes
		dw_sum += dw;
		
		// set the new weight
		SetWeight( idx, j, dw );
	}
}

void Connection::UpdateDelay( int idx, data_type act, data_type mu, data_type backAct, data_type &dw_sum )
{
	// only update if delay has passed
	if ( mTime == mDelay ) UpdateNormal( idx, act, mu, backAct, dw_sum );
}


// copy weights to pre-allocated buffer
void Connection::CopyWeights( double** matrix )
{
	for ( int i = 0; i < *mToSize; i++ )
		for ( int j = 0; j < mInModule->GetModuleSize(); j++ )
			matrix[i][j] = GetWeight(i,j);
}


// return sum of weight changes stored in every CALMWeight instance
void Connection::SumWeightChanges( data_type &dw_sum )
{
	for ( int i = 0; i < *mToSize; i++ )
		for ( int j = 0; j < mInModule->GetModuleSize(); j++ )
			dw_sum += GetWeightChange(i,j);
}


// Write out weight values to an output stream
void Connection::SaveWeights( ofstream *outfile )
{
	for ( int i = 0; i < *mToSize; i++ )
	{
		for ( int j = 0; j < mInModule->GetModuleSize(); j++ )
		{
			*outfile << GetWeight(i,j) << " ";
		}
		*outfile << endl;
	}
}


// Write out weight values to an output stream
void Connection::LoadWeights( ifstream *infile )
{
	data_type 	wt;
	bool		garbage = true;
	char		c;
	
	// we have to strip the headers first
	while ( garbage )
	{
		// ignore any line feeds left in the stream
		while ( infile->peek() == '\n' || infile->peek() == ' ' || infile->peek() == '\t' ) infile->get();	
		while ( infile->peek() == '#' || infile->peek() == 'S' || infile->peek() == 'T' ) infile->ignore( 1000, '\n' );
		infile->get(c);
		if ( c == '\n' || c == '\t' || c == ' ' || c == '#' || c == 'S' || c == 'T' )
			garbage = true;
		else
			garbage = false;
		infile->putback(c);
	}
		
	for ( int i = 0; i < *mToSize; i++ )
	{
		for ( int j = 0; j < mInModule->GetModuleSize(); j++ )
		{
			*infile >> wt;
			mWeights[i][j].SetWeight( wt );
		}
	}
}


// Write out weight values to an output stream
void Connection::Print( ostream *os )
{
	*os << mInModule->GetModuleName() << endl;
	AdjustStream( *os, 3, 6, kLeft, true );
	for ( int i = 0; i < *mToSize; i++ )
	{
		for ( int j = 0; j < mInModule->GetModuleSize(); j++ )
			*os << GetWeight(i,j) << "\t";
		*os << endl;
	}
	SetStreamDefaults( *os );
}


// overload the << operator
ostream &operator<<( ostream &os, Connection &c )
{
	c.Print( &os );
	return os;
}


