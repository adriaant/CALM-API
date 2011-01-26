/*
	Author:			Adriaan Tijsseling (AGT)
	Copyright: 		(c) Copyright 2002-2011 Adriaan Tijsseling. All rights reserved.
	Description:	Main file for using the CALM API
					This code creates an executable that uses the CALM API. 
					The user has to provide the functions InitNetwork() and DoSimulation().
*/

#include <stdlib.h>
#include "CALMGlobal.h"
#include "CALM.h"		// the interface file to the CALM API Library

// defined in user-provided simulation file
extern bool	InitNetwork( void );
extern void DoSimulation( void );

// PROTOTYPE
void Usage( void );

// GLOBALS
CALMAPI* gCALMAPI;	// pointer to API interface
bool	 gSwitch;	// handy switch for selecting only test phase

int main( int argc, char *argv[] )
{
	int		arg;
	
// initialize the API instance. This MUST be called at the start of the program!
	gCALMAPI = new CALMAPI;
	
	/* process command-line arguments. These should contain either:
		-r	: runs
		-e	: epochs
		-i	: iterations
		-p	: presentation order: 0 (linear) or 1 (permuted)
		-v	: verbosity level
		-c	: whether to stop training current pattern after convergence
		-b	: base name of network files
		-d	: directory with network files
	*/
	if ( argc > 1 )
	{
		// run down each argument
		arg = 1;
		while( arg < argc )
		{
			// check if -h is called
			if( strcmp( argv[arg], "-h") == 0 )
			{
				arg++;
				Usage();
			}
			// perhaps -r			
			if( strcmp( argv[arg], "-r") == 0 )
			{
				arg++;
				if ( argv[arg] == nil ) Usage();
				gCALMAPI->CALMSetNumRuns( atoi( argv[arg] ) );
				goto loop;
			}
			// perhaps -e			
			if( strcmp( argv[arg], "-e") == 0 )
			{
				arg++;
				if ( argv[arg] == nil ) Usage();
				gCALMAPI->CALMSetNumEpochs( atoi( argv[arg] ) );
				goto loop;
			}
			// perhaps -i			
			if( strcmp( argv[arg], "-i") == 0 )
			{
				arg++;
				if ( argv[arg] == nil ) Usage();
				gCALMAPI->CALMSetNumIterations( atoi( argv[arg] ) );
				goto loop;
			}
			// perhaps -p			
			if( strcmp( argv[arg], "-p") == 0 )
			{
				arg++;
				if ( argv[arg] == nil ) Usage();
				gCALMAPI->CALMSetOrder( atoi( argv[arg] ) );
				goto loop;
			}
			// perhaps -S			
			if( strcmp( argv[arg], "-S") == 0 )
			{
				arg++;
				if ( argv[arg] == nil ) Usage();
				gSwitch = atoi( argv[arg] );
				goto loop;
			}
			// stop at convergence -c		
			if( strcmp( argv[arg], "-c") == 0 )
			{
				arg++;
				if ( argv[arg] == nil ) Usage();
				gCALMAPI->CALMSetConvStop( (bool)atoi( argv[arg] ) );
				goto loop;
			}
			// base file name -b
			if( strcmp( argv[arg], "-b") == 0 )
			{
				arg++;
				if ( argv[arg] == nil ) Usage();
				gCALMAPI->CALMSetBasename( argv[arg] );
				goto loop;
			}
			// directory -d for files
			if( strcmp( argv[arg], "-d") == 0 )
			{
				arg++;
				if ( argv[arg] == nil ) Usage();
				gCALMAPI->CALMSetDirectory( argv[arg] );
				// set the directory we want to save the files in (if you don't call this
				// then by default the files are created in the working directory)
				gCALMAPI->CALMSetLogDirectory( gCALMAPI->CALMGetDirectory() );
				goto loop;
			}
			// verbosity -v		
			if( strcmp( argv[arg], "-v") == 0 )
			{
				arg++;
				if ( argv[arg] == nil ) Usage();
				gCALMAPI->CALMSetVerbosity( atoi( argv[arg] ) );
				goto loop;
			}
loop:
			arg++;
		}
	}

// initialize network
	if ( InitNetwork() )
	{
	// run simulation (defined in simulation code file)
		DoSimulation();
	}
		
// clean the API instance
	delete gCALMAPI;
	
	return 0;
}


void Usage( void )
{
    cerr << "Usage: calm (-OPTIONS) [default]" << endl;
    cerr << "    -h        = display this help and exit" << endl;
    cerr << "    -r [1]    = number of simulations to run" << endl;
    cerr << "    -e [50]   = number of epochs to present each set of patterns" << endl;
    cerr << "    -i [100]  = number of iterations to train a single pattern" << endl;
    cerr << "    -p [1]    = presentation order: 0 (linear) or 1 (permuted)" << endl;
    cerr << "    -b [calm] = base file name for network files. Files should be suffixed as:" << endl;
    cerr << "                .net : network definition" << endl;
    cerr << "                .par : network parameters" << endl;
    cerr << "                .pat : pattern file" << endl;
    cerr << "                .fb  : feedback file" << endl;
    cerr << "    -d [.]    = directory containing the network files" << endl;
    cerr << "    -c [0]    = stop training of current pattern after convergence" << endl;
	cerr << "    -S [0]    = set to 1 to load weights and only do a test phase" << endl;
    cerr << "    -v [1]    = verbosity level, defined as a bitwise operation of:" << endl;
    cerr << "                0  : silent mode" << endl;
    cerr << "                1  : winning nodes" << endl;
    cerr << "                2  : activations" << endl;
    cerr << "                4  : rounded activations" << endl;
    cerr << "                8  : potential of R-units (used for growing/pruning)" << endl;
    cerr << "                16 : weights" << endl;
    cerr << "                32 : save weight-changes" << endl;
    cerr << "                64 : save total activation-changes" << endl;
    cerr << "                128: save learning-rate-changes" << endl;
	exit(0);
}

