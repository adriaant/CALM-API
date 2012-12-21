/*
	Author:			Adriaan Tijsseling (AGT)
	Copyright: 		(c) Copyright 2002-2013 Adriaan Tijsseling. All rights reserved.
	Description:	Sample simulation file for multiple sequence learning using CALM networks
					with time-delay connections. Requires the MultiSequence class files.
*/

#include <stdlib.h>
#include "CALMGlobal.h"
#include "CALM.h"			// the interface file to the CALM API Library
#include "MultiSequence.h"	// for using multi sequence learning

// LOCAL GLOBALS
extern CALMAPI*	gCALMAPI;	// pointer to API interface

// PROTOTYPES
bool	InitNetwork( void );
void 	DoSimulation( void );


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
		
// tell the API we are in online mode
	gCALMAPI->CALMOnlinePatterns();
	
// OPTIONAL: record display simulation environment to log file
	gCALMAPI->CALMShow();

	return true;
}

void DoSimulation( void )
{
	int		calmErr;		// any errors passed from API (currently nothing...)
	char	filename[32];
	char	stridx[5];
	bool	test = false;

// create the multi sequence training instance
	// 5 sequences, training starts with first sequence, 1000 epochs, "out" is fb module
	MultiSequence* multiSeq = new MultiSequence( 5, 0, 1000, "out" );
	
// load all the sequences from files for faster training
	cerr << "\nloading all training files" << endl;
	if ( multiSeq->LoadPatternFiles() != kNoErr ) return;
	cerr << "files loaded" << endl;

// if trained before, set "test" to true to just check the performance of the network	
	if ( test )
	{
	// if you used feedback training, then you need to make sure the feedback
	// module size as defined in the network specification file is resized to the
	// final size after training (most likely equal to the number of sequences)
	// otherwise, weights cannot be loaded!
		gCALMAPI->CALMResizeModule( gCALMAPI->CALMGetModuleIndex( "out" ), 10 );
		gCALMAPI->CALMLoadWeights( "final-0" );

	// set the log file for clamp data
/*		calmErr = gCALMAPI->OpenCALMLog( "nofb-grow-clamp.txt" );
		if ( calmErr != kNoErr ) return;
		multiSeq->ClampTest();		
*/
	//  oscillation data
		calmErr = gCALMAPI->OpenCALMLog( "nofb-grow-osc.txt" );
		if ( calmErr != kNoErr ) return;
		multiSeq->OscillationTest();
/*
	//  winner data
		calmErr = gCALMAPI->OpenCALMLog( "nofb-grow-win.txt" );
		if ( calmErr != kNoErr ) return;
		multiSeq->WinnerTest();
*/

	// set API internal log redirection back to cout
		gCALMAPI->SetCALMLog( &cout );
		
	// bail out
		return;
	}
	
// if not testing, we are training the network on the sequences
	for ( int r = 0; r < gCALMAPI->CALMGetNumRuns(); r++ )
	{
	// record duration of simulation
		gCALMAPI->CALMDuration( kStart );
	
	// show current run
		*(gCALMAPI->GetCALMLog()) << "\nRUN " << r << endl;		
	
	// start clean
		gCALMAPI->CALMReset( O_WT | O_TIME | O_WIN );
	
	// run the multi-sequence learning procedure
		multiSeq->RunMultiSequenceSimulation();
	
	// end time recording, display duration
		gCALMAPI->CALMDuration( kEnd );	

	// write out the final weights (file extension is added by the API)
		strcpy( filename, "final" );
		sprintf( stridx, "-%d", r );
		strcat( filename, stridx );
		gCALMAPI->CALMSaveWeights( filename );

	// output final module sizes (useful if growing was on), also save network!
		// (file extension is added by the API)
		*(gCALMAPI->GetCALMLog()) << "\nFinal sizes:" << endl;	
		gCALMAPI->CALMShowSizes();	
		gCALMAPI->CALMWriteNetwork( &calmErr, filename );
		if ( calmErr != kNoErr ) break;
		
	// reset feedback module size for the next run
		gCALMAPI->CALMResizeModule( gCALMAPI->CALMGetModuleIndex( "out" ), 2 );
		cerr << "feedback module resized to 2" << endl;
	}

	*(gCALMAPI->GetCALMLog()) << endl;
}


