/*
Description:	Growing/pruning test using the CALM API
Author:			Adriaan Tijsseling (AGT)
Copyright: 		(c) Copyright 2002-2015 Adriaan Tijsseling. All rights reserved.
Change History (most recent first):
	31/03/2002 - AGT - initial version
*/

#include <stdlib.h>
#include "CALMGlobal.h"
#include "CALM.h"		// the interface file to the CALM API Library

// LOCAL GLOBALS
extern CALMAPI*	gCALMAPI;	// pointer to API interface

// PROTOTYPES
bool	InitNetwork( void );
void 	DoSimulation( void );
void	Train( void );
void	Test( int run );


// initializes the network and creates the online pattern storage
bool InitNetwork( void )
{
	int calmErr;
	
	// set the API console output to cout
	gCALMAPI->SetCALMLog( &cout );
	
	// load the parameter file
	// NOTE: The parameters file should be loaded BEFORE setting up the network
	if ( gCALMAPI->CALMLoadParameters() != kNoErr ) return false;
	
	// create the CALM network
	gCALMAPI->CALMSetupNetwork( &calmErr );
	if ( calmErr != kNoErr ) return false;
	
	// load the pattern file
	// NOTE: The patterns file should be loaded AFTER creating the network
	if ( gCALMAPI->CALMLoadPatterns() != kNoErr ) return false;
	
	// tell the API how we want to present patterns, permuted or linear. By default the
	// presentation of patterns is in permuted order; override in command line option.
	// NOTE: This function can only be used in offline mode!
	gCALMAPI->CALMPatternOrder( gCALMAPI->CALMGetOrder() );
	
	// OPTIONAL: record display simulation environment to log file
	gCALMAPI->CALMShow();
	gCALMAPI->CALMShowPatterns();
	
	return true;
}


void DoSimulation( void )
{
	// reset the network, train patterns and test performance. Repeat for desired number of runs
	cerr << "run: ";
	for ( int run = 0; run < gCALMAPI->CALMGetNumRuns(); run++ )
	{
		int calmErr;

		*(gCALMAPI->GetCALMLog()) << "\nRUN " << run << endl;
		cerr << run << ' ';
		
		// start clean
		gCALMAPI->CALMReset( O_WT | O_TIME | O_WIN );
		
		// record duration of simulation
		gCALMAPI->CALMDuration( kStart );
		*(gCALMAPI->GetCALMLog()) << "\nTRAINING" << endl;
		
		// train the network on pattern file
		Train();
		
		*(gCALMAPI->GetCALMLog()) << "\nTESTING" << endl;

		// reset winning node information as well as time-delay activations
		// (the latter only applies if time-delay connections are used)
		gCALMAPI->CALMReset( O_TIME | O_WIN );
		
		// test the network on pattern file
		Test( run );

		// print out the final weight configuration
		gCALMAPI->CALMShowWeights();
	//	gCALMAPI->CALMSaveWeights( "final" );

		// output final module sizes
		gCALMAPI->CALMShowSizes();
	
		gCALMAPI->CALMDuration( kEnd );			// end time recording, display duration
		
		// reload network for next run
		gCALMAPI->CALMSetupNetwork( &calmErr, gCALMAPI->CALMGetBasename() );
		if ( gCALMAPI->CALMLoadPatterns() != kNoErr ) return;
	}

	cout << endl;
}


void Train( void )
{
	// train the patterns from file
	for ( int epoch = 0; epoch < gCALMAPI->CALMGetNumEpochs(); epoch++ )
	{
		// permute pattern set (only if kPermuted was chosen)
		if ( gCALMAPI->CALMGetOrder() == kPermuted ) gCALMAPI->CALMPermutePatterns();
		// train single pass through sequence
		gCALMAPI->CALMTrainFile( epoch );
		
		// grow or prune when necessary
		if ( epoch % 2 == 0 ) gCALMAPI->CALMResizeModule();
	}
}


// testing routine
void Test( int run )
{
	int	numIters = gCALMAPI->CALMGetNumIterations();
	
	// Test the pattern, one by one, according to order in file
	gCALMAPI->CALMPatternOrder( kLinear );
	gCALMAPI->CALMTestFile( run );

	// show winners at end of training in case verbosity is set to 0
	if ( gCALMAPI->CALMGetVerbosity() == O_NONE ) gCALMAPI->CALMShowWinners();

	gCALMAPI->CALMPatternOrder( gCALMAPI->CALMGetOrder() );	// set back our desired ordering

	gCALMAPI->CALMSetNumIterations( 50 );

	*(gCALMAPI->GetCALMLog()) << "\n\nCLAMP TEST" << endl;
	// reset input modules to zero
	gCALMAPI->CALMReset();
	// make sure the API is giving us the winners
	gCALMAPI->CALMSetVerbosity( O_WINNER );
	// get index of output module
	int mOutIdx = gCALMAPI->CALMGetModuleIndex( "out" );
	// get size of output module
	int mOutSize = gCALMAPI->CALMGetModuleSize( mOutIdx );
	for ( int i = 0; i < mOutSize; i++ )
	{
		gCALMAPI->ClampUnit( mOutIdx, i, 1.0 );	// clamp the unit
		gCALMAPI->CALMTestSingle( 0, false );
		gCALMAPI->ClampUnit( mOutIdx, i );	// this unclamps it
	}
	
	gCALMAPI->CALMSetVerbosity( O_NONE );
	gCALMAPI->CALMSetNumIterations( numIters );
}

