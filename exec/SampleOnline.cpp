/*
	Author:			Adriaan Tijsseling (AGT)
	Copyright: 		(c) Copyright 2002-2013 Adriaan Tijsseling. All rights reserved.
	Description:	Sample simulation using the CALM API. This particular code trains a CALM
					network specified in the command line options with a "online" patterns, 
					i.e. patterns that are not provided with pattern files but that are received
					online (useful demo for using the API in a larger project). 
					This simulation also shows how the AnalysisTools class can be used.
					Invoke with e.g.:
					
						calm -r 1 -e 10 -i 50 -b calm -d simulations/online -v 0
*/

#include <stdlib.h>
#include "CALMGlobal.h"
#include "CALM.h"		// the interface file to the CALM API Library
#include "AnalysisTools.h"
#include "Rnd.h"

// GLOBALS
extern CALMAPI*	gCALMAPI;	// pointer to API interface
extern bool		gSwitch;	// handy switch for selecting only test phase

// PROTOTYPES
bool	InitNetwork( void );
void 	DoSimulation( void );


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
		
// tell the API we are using it in online mode
	gCALMAPI->CALMOnlinePatterns();

// OPTIONAL: record display simulation environment to log file
	gCALMAPI->CALMShow();

	return true;
}


// the actual training regiment
void DoSimulation( void )
{
	AnalysisTools convPlot;
	AnalysisTools bifPlot;
	AnalysisTools phasePlot;
	char filename[32];
	
// start clean
	gCALMAPI->CALMReset( O_WT | O_WIN | O_TIME );
	
// record duration of simulation
	gCALMAPI->CALMDuration( kStart );

	if ( ! gSwitch )
	{
	// TRAIN network for a given number of epochs
		*(gCALMAPI->GetCALMLog()) << "\ntraining random patterns:" << endl;
		for ( int epoch = 0; epoch < gCALMAPI->CALMGetNumEpochs(); epoch++ ) 
		{
		// the pattern will just be random 
			for ( int i = 0; i < gCALMAPI->CALMGetInputLen(); i++ )
			{
			// set the input to a random value between 0 and 1
				gCALMAPI->CALMSetOnlineInput( i, PseudoRNG() );
			// show the input
				*(gCALMAPI->GetCALMLog()) << "  " << gCALMAPI->CALMGetOnlineInput(i) << "\t";
			}
			*(gCALMAPI->GetCALMLog()) << endl;
			
		// train single pass of the pattern
			gCALMAPI->CALMTrainSingle( epoch );
		}
	
	// end time recording, display duration
		gCALMAPI->CALMDuration( kEnd );
	
	// write out the final weights (file extension is added by the API)
		strcpy( filename, "final" );
		gCALMAPI->CALMSaveWeights( filename );
	
	// ANALYZE the network's performance
	
	// first set the input to 0.0
		for ( int i = 0; i < gCALMAPI->CALMGetInputLen(); i++ ) gCALMAPI->CALMSetOnlineInput( i, 0.0 );
	
	// this one will show the winners for every possible <x,y> input	
		convPlot.BoundaryForOnline( 0, gCALMAPI->CALMGetOnlineInput(), 
			"A", "pat", 200, 200, 100 );
	}
	else // only do other tests if explicitly set to
	{
	// load the learned weights
		gCALMAPI->CALMLoadWeights("final");

	// set the input to 0.0
		for ( int i = 0; i < gCALMAPI->CALMGetInputLen(); i++ ) gCALMAPI->CALMSetOnlineInput( i, 0.0 );
	
	// this one shows the effect of the variation of one input value on the dynamics of the network
		bifPlot.BifurcationForOnline( 0, gCALMAPI->CALMGetOnlineInput(), 
			"pat", 400, 400, 100, 1000, 0.01, 0.81, 0.1 );
	
	// this one shows the effect of a given input on the dynamics of the net
		phasePlot.PhaseForOnline( 0, gCALMAPI->CALMGetOnlineInput(), "pat", 250, 10, 1000, 0.0, 0.01, 0.81, 0.1 );
	}
}


