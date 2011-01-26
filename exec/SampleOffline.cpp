/*
	Author:			Adriaan Tijsseling (AGT)
	Copyright: 		(c) Copyright 2002-2011 Adriaan Tijsseling. All rights reserved.
	Description:	Sample simulation using the CALM API. This particular code trains a CALM
					network specified in the command line options. 
					Patterns are loaded from file. Invoke with e.g.:
					
						calm -r 1 -e 10 -i 50 -b calm -d simulations/offline -v 0
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
	
// OPTIONAL: create a new log file 
	// (by default all output goes to cout, so log files do not have to be created)
	// here we redirect cout to log.txt in the directory containing the network files
	// passing -d flag sets the directory for the log file (see Main.cpp)
	// NOTE: termination of the API automatically closes the file.
	calmErr = gCALMAPI->OpenCALMLog( "log.txt" );
	if ( calmErr != kNoErr ) return false;

// show where the output goes	
	cerr << "data directed to " << gCALMAPI->CALMGetCurDir() << "/";
	cerr << gCALMAPI->CALMGetDirectory() << "/log.txt" << endl;
	
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


// the actual training regiment
void DoSimulation( void )
{
// reset the network, train patterns and test performance. Repeat for desired number of runs
	cerr << "run: ";
	for ( int run = 0; run < gCALMAPI->CALMGetNumRuns(); run++ )
	{
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

	// end time recording, display duration
		gCALMAPI->CALMDuration( kEnd );

	// print out the final weight configuration	
		gCALMAPI->CALMShowWeights();
	}
	cerr << endl;
}

// training routine
void Train( void )
{
// train the patterns from file
	for ( int epoch = 0; epoch < gCALMAPI->CALMGetNumEpochs(); epoch++ ) 
	{
		// permute pattern set (only if kPermuted was chosen)
		if ( gCALMAPI->CALMGetOrder() == kPermuted ) gCALMAPI->CALMPermutePatterns();
		// train single pass through sequence
		gCALMAPI->CALMTrainFile( epoch ); 
	}
}

// testing routine
void Test( int run )
{
// test the patterns, one by one, according to order in file
	gCALMAPI->CALMPatternOrder( kLinear );
	gCALMAPI->CALMTestFile( run ); 

// show winners at end of testing in case verbosity is set to 0
	if ( gCALMAPI->CALMGetVerbosity() == O_NONE ) gCALMAPI->CALMShowWinners();

// set back original pattern presentation order
	gCALMAPI->CALMPatternOrder( gCALMAPI->CALMGetOrder() );	
}

