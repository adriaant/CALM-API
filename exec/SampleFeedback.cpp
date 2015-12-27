/*
	Author:			Adriaan Tijsseling (AGT)
	Copyright: 		(c) Copyright 2002-2015 Adriaan Tijsseling. All rights reserved.
	Description:	Sample simulation using the CALM API. 
					This particular code switches between two modes. In the first one, 
					the auto-association phase, the network is trained on a set of 
					stimuli unsupervised. Then, in the second mode, feedback is
					provided to the "output" module (the .net file needs to be changed 
					in between to correct module type). 
					Invoke with e.g.:
					
						calm -r 1 -e 100 -i 100 -b calm -d simulations/feedback -v 0
*/

#include <stdlib.h>
#include "CALMGlobal.h"
#include "CALM.h"		// the interface file to the CALM API Library
#include "AnalysisTools.h"

#define AA 0

// LOCAL GLOBALS
extern CALMAPI*	gCALMAPI;	// pointer to API interface

// PROTOTYPES
bool	InitNetwork( void );
void 	DoSimulation( void );
void	Train( void );
void	Test( int );


// initializes the network and creates the online pattern storage
bool InitNetwork( void )
{
	int calmErr;
	
// we use cout for the simulation report
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
	// load the feedback file if we are using a feedback module
#if !AA
	if ( gCALMAPI->CALMLoadFeedback() != kNoErr ) return false;
#endif

	// tell the API we want our patterns to be permuted every epoch (or choose kLinear)
	gCALMAPI->CALMPatternOrder( gCALMAPI->CALMGetOrder() );
		
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
	AnalysisTools dataPlot, bifPlot, phasePlot;
	char	filename[32];
	char	stridx[5];
	
// repeat for specified number of runs
	for ( int run = 0; run < gCALMAPI->CALMGetNumRuns(); run++ )
	{
		*(gCALMAPI->GetCALMLog()) << "\nRUN " << run << endl;
	
#if AA
	// start clean
		gCALMAPI->CALMReset( O_WT | O_TIME | O_WIN );
#else
	// opt to run feedback training based on unsupervised weights, to see how quick
	// correct categorization can be achieved
		strcpy( filename, "aa" );
		sprintf( stridx, "-%d", run );
		strcat( filename, stridx );
		gCALMAPI->CALMLoadWeights( filename );
		gCALMAPI->CALMReset( O_TIME | O_WIN );
		Test( run );
#endif	

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

	// write out the final weights (file extension is added by the API)
	#if AA
		strcpy( filename, "aa" );
	#else
		strcpy( filename, "cat" );
	#endif
		sprintf( stridx, "-%d", run );
		strcat( filename, stridx );
		gCALMAPI->CALMSaveWeights( filename );

	// produce a convergence plot which shows the winner for every input combination
		/*
			current run:   run
			target module: A
			input module:  pat
			dimensions:    100x100
			iterations:    100
			pattern index: 0
			x variable:    0 = index of input node
			y variable:    1
		*/
		dataPlot.BoundaryForOffline( run, "A", "pat", 100, 100, 100, 0, 0, 1 );
		dataPlot.BoundaryForOffline( run, "A", "pat", 100, 100, 100, 1, 1, 2 );
		dataPlot.BoundaryForOffline( run, "A", "pat", 100, 100, 100, 2, 2, 3 );
		dataPlot.BoundaryForOffline( run, "A", "pat", 100, 100, 100, 3, 3, 4 );
		dataPlot.BoundaryForOffline( run, "A", "pat", 100, 100, 100, 4, 4, 5 );
		dataPlot.BoundaryForOffline( run, "A", "pat", 100, 100, 100, 5, 5, 6 );
		dataPlot.BoundaryForOffline( run, "A", "pat", 100, 100, 100, 6, 6, 7 );
		
	// produce a bifurcation plot
		/*
			current run:   run
			input module:  pat
			dimensions:    400x300
			transients:    10 (iterations to skip)
			iterations:    1000
			start value:   0.0
			end value:     1.0
			bif parameter: 0.5
			pattern index: 0
			x variable:    0 = index of input node
		*/
		bifPlot.BifurcationForOffline( run, "pat", 400, 300, 10, 1000, 0.0, 1.0, 0.5, 0, 0 );
		bifPlot.BifurcationForOffline( run, "pat", 400, 300, 10, 1000, 0.0, 1.0, 0.5, 1, 1 );
		bifPlot.BifurcationForOffline( run, "pat", 400, 300, 10, 1000, 0.0, 1.0, 0.5, 2, 2 );
		bifPlot.BifurcationForOffline( run, "pat", 400, 300, 10, 1000, 0.0, 1.0, 0.5, 3, 3 );
		bifPlot.BifurcationForOffline( run, "pat", 400, 300, 10, 1000, 0.0, 1.0, 0.5, 4, 4 );
		bifPlot.BifurcationForOffline( run, "pat", 400, 300, 10, 1000, 0.0, 1.0, 0.5, 5, 5 );
		bifPlot.BifurcationForOffline( run, "pat", 400, 300, 10, 1000, 0.0, 1.0, 0.5, 6, 6 );
		
	// produce orbits
		/*
			current run:   run
			input module:  pat
			dimensions:    250x250
			transients:    100 (iterations to skip)
			iterations:    5000
			start value:   0.0
			step value:    0.05
			end value:     1.0
			parameter:     0.5
			pattern index: 0
			x variable:    0 = index of input node
		*/
		phasePlot.PhaseForOffline( run, "pat", 250, 100, 5000, 0.0, 0.05, 1.0, 0.5, 0, 0 );
		phasePlot.PhaseForOffline( run, "pat", 250, 100, 5000, 0.0, 0.05, 1.0, 0.5, 1, 1 );
		phasePlot.PhaseForOffline( run, "pat", 250, 100, 5000, 0.0, 0.05, 1.0, 0.5, 2, 2 );
		phasePlot.PhaseForOffline( run, "pat", 250, 100, 5000, 0.0, 0.05, 1.0, 0.5, 3, 3 );
		phasePlot.PhaseForOffline( run, "pat", 250, 100, 5000, 0.0, 0.05, 1.0, 0.5, 4, 4 );
		phasePlot.PhaseForOffline( run, "pat", 250, 100, 5000, 0.0, 0.05, 1.0, 0.5, 5, 5 );
		phasePlot.PhaseForOffline( run, "pat", 250, 100, 5000, 0.0, 0.05, 1.0, 0.5, 6, 6 );
	}
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
	if ( gCALMAPI->CALMGetVerbosity() == O_NONE )
	{
		gCALMAPI->CALMShowModules();
		gCALMAPI->CALMShowWinners();
	}

// set back original pattern presentation order
	gCALMAPI->CALMPatternOrder( gCALMAPI->CALMGetOrder() );	
}

