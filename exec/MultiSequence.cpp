/*
	Author:			Adriaan Tijsseling (AGT)
	Copyright: 		(c) Copyright 2002-2013 Adriaan Tijsseling. All rights reserved.
	Description:	Class definition for multiple sequence learning using CALM networks
					with time-delay connections.
*/

#include "MultiSequence.h"
#include "Rnd.h"

#define FEEDBACK	1	// set whether to use feedback for training
#define	GROWING		1	// set whether to grow/prune modules
#define GROWCHECK	5	// number of epochs after which the network is checked for resizing
#define	PLOT3D		1	// plot with GNUPlot (needs X11 server to be running)

extern CALMAPI* gCALMAPI;		// pointer to API interface


MultiSequence::MultiSequence( int numfiles, int fileIdx, int epochs, const char* fbname )
{
	mNumFiles = numfiles;	// number of files to train
	mFileIdx = fileIdx;		// index of file to start training with
	mEpochs = epochs;		// max number of epochs for training

// store index of feedback module
	mOutIdx = gCALMAPI->CALMGetModuleIndex( fbname );

	mPatterns = NULL;
	mNumPats = NULL;
}


MultiSequence::~MultiSequence()
{
	if ( mPatterns != NULL )
	{
		for ( int i = 0; i < mNumFiles; i++ ) DisposeMatrix( mPatterns[i],  mNumPats[i] );
		delete[] mPatterns;
	}
	delete[] mNumPats;
}


// Here we load all the pattern files and store each file's data in a single storage object.
// With this method, pattern access will be faster than repeated reloading of files.
int	MultiSequence::LoadPatternFiles( void )
{
	int			i, j, k;
	int 		err = kNoErr;
	char		filename[256];
	char		stridx[5];
	ifstream	infile;
	int			numBits = gCALMAPI->CALMGetInputLen(); // make sure CALMOnlinePatterns was called earlier
	
	cerr << "input len is " << numBits << endl;
	
	// allocate storage for all pattern files
	mPatterns = new data_type**[mNumFiles];
	mNumPats = new int[mNumFiles];

	for ( i = 0; i < mNumFiles; i++ )
	{
	// load the current pattern file
		strcpy( filename, gCALMAPI->CALMGetBasename() );
		sprintf( stridx, "-%d", i );
		strcat( filename, stridx );
		strcat( filename, ".pat" );
	// open the file
		infile.open( filename );
		if ( infile.fail() )
		{
			FileOpenError( filename );
			return kCALMFileError;
		}
	// read the number of patterns
		SkipComments( &infile );
		infile >> mNumPats[i];
	// create storage for this file
		mPatterns[i] = CreateMatrix( 0.0, mNumPats[i], numBits );
	// read in each pattern
		for ( j = 0; j < mNumPats[i]; j++ )
		{
			for ( k = 0; k < numBits; k++ )
			{
				SkipComments( &infile );
				infile >> mPatterns[i][j][k];
			}
		}
	// close and return
		infile.close();
	}

	return err;
}


// Core routine for training multiple sequences
bool MultiSequence::RunMultiSequenceSimulation( void )
{		
	int*	array;
	int		winner;
	int		i, idx, epochCtr = 0, totalEpochs = 0, epoch;
	bool	done = true, trained = false;
	int		currentFileIdx = mFileIdx+1;
	char	comment[32];
	char	dummy[5];
	bool	resized;

#if PLOT3D			
// tell API to start a 3D weight plot for weights between two selected modules
	// be sure to modify the names if you use a different network file!
	gCALMAPI->CALMInit3DPlot( "agg", "out" );
	gCALMAPI->CALM3DPlot();
#endif

#if ! FEEDBACK
	int	maxepochs = ((currentFileIdx-1) * mEpochs) + mEpochs;
#else
	int maxepochs = mEpochs;
#endif
	
	if ( gCALMAPI->CALMGetOrder() == kPermuted ) // permuted order
	{
		array = new int[mNumFiles];			// array for permuted patterns
		for ( i = 0; i < mNumFiles; i++ ) array[i] = i;
		Permute( array, mNumFiles );		// permute indices
			
		*(gCALMAPI->GetCALMLog()) << "order of sequence presentation will be: ";
		for ( i = 0; i < mNumFiles; i++ ) *(gCALMAPI->GetCALMLog()) << array[i] << " ";
		*(gCALMAPI->GetCALMLog()) << endl;
	}

	// user feedback
	*(gCALMAPI->GetCALMLog()) << "\nTraining multiple sequences from patterns " << gCALMAPI->CALMGetBasename() << endl;

	while ( true )
	{
	// indicate whether all sequences were trained successfully
		done = true;
		trained = false;
		
	// start training a set containing the first sequence, and then increment
		for ( i = 0; i <= currentFileIdx; i++ )
		{
		// the sequence added is indicated differently based on presentation type
			if ( gCALMAPI->CALMGetOrder() == kPermuted )
				idx = array[i];
			else
				idx = i;

		#if FEEDBACK	
		// the feedback module grows with each new supervision signal
			if ( (i+1) > gCALMAPI->CALMGetModuleSize( mOutIdx ) )
			{
				gCALMAPI->CALMResizeModule( mOutIdx, gCALMAPI->CALMGetModuleSize( mOutIdx ) + 1 );
		#if PLOT3D			
				gCALMAPI->CALMResize3DPlot( "agg", "out" );
		#endif
			}			
		// set the feedback signal
			gCALMAPI->CALMSetFeedback(i);
		#endif

		// each time we're back at the initial sequence, it is recorded as an epoch
			if ( i == 0 )
			{
				epochCtr += 1;
				totalEpochs += 1;
				AdjustStream( *(gCALMAPI->GetCALMLog()), 0, 5, kRight, false );
				*(gCALMAPI->GetCALMLog()) << totalEpochs << " ";
				SetStreamDefaults( *(gCALMAPI->GetCALMLog()) );
			}

		// reset timing info in CALM
			gCALMAPI->CALMReset( O_TIME );

			for ( epoch = 0; epoch < gCALMAPI->CALMGetNumEpochs(); epoch++ )
			{
			// first test the sequence
				TestCurrentSequence( idx, &winner );			
			// if winner matches feedback signal (set to current file idx) 
			// proceed with new sequence, otherwise retrain sequences
			#if FEEDBACK
				if ( winner == i )  break;
			#endif
				TrainCurrentSequence( idx );
				trained = true;
			}
		#if PLOT3D			
			// show changed weights
			gCALMAPI->CALM3DPlot();
		#endif
			// no adding sequences until all winners are correct and no training was required
			if ( winner != i ) done = false;		
			*(gCALMAPI->GetCALMLog()) << "[" << idx << ": " << winner;
		#if FEEDBACK			
			*(gCALMAPI->GetCALMLog()) << " " << epoch << "] ";
		#else
			*(gCALMAPI->GetCALMLog()) << "] ";
		#endif
		}
	
		*(gCALMAPI->GetCALMLog()) << endl;
		// when done, add a new sequence or terminate. Else: just repeat
		// additional requirements is that there have been at least two epochs. This is 
		// to make sure that after adding a new sequence, the old ones are still ok
		if ( ( epochCtr > 1 && done && !trained ) || epochCtr >= maxepochs )
		{
			currentFileIdx += 1;
			// prepare to move to the next sequence, if present
		#if FEEDBACK
/*			if ( currentFileIdx == mNumFiles-1 )
			{
			// for final sequence we use a larger number of epochs to train the full set
				maxepochs = 2 * mEpochs; 
			}
*/
		#else
			// for unsupervised learning, we increment the number of epochs with each new sequence
			maxepochs = ((currentFileIdx-1) * mEpochs) + mEpochs;
		#endif			
			// write current stats
			*(gCALMAPI->GetCALMLog()) << epochCtr << endl;
			// if all sequences trained, bail out
			if ( currentFileIdx >= mNumFiles ) goto bail;
			// mark weight change file
			strcpy( comment, "# adding new sequence " );
			sprintf( dummy, "-%d", currentFileIdx );
			strcat( comment, dummy );
			gCALMAPI->CALMSaveWeightChangesComment( comment );

			epochCtr = 0;
		
		#if GROWING
		// grow or prune when necessary
	//		gCALMAPI->CALMResizeModule();
		#endif
		}

	#if GROWING
		// grow or prune when necessary: we only do this every number of passes through the current
		// set of sequences, such that after a change in size, the net has the change to adapt
		// the representations before it resizes again.
		if ( totalEpochs % GROWCHECK == 0 )
		{
			resized = gCALMAPI->CALMResizeModule();
		#if PLOT3D			
			if ( resized ) gCALMAPI->CALMResize3DPlot( "agg", "out" );
		#endif
		}
	#endif
	}

bail:
#if FEEDBACK
	*(gCALMAPI->GetCALMLog()) << "final test" << endl;
	// final run of testing
	gCALMAPI->CALMSetVerbosity( O_WINNER );
	for ( i = 0; i < mNumFiles; i++ )
	{			
		if ( gCALMAPI->CALMGetOrder() == kPermuted )
			idx = array[i];
		else
			idx = i;

		*(gCALMAPI->GetCALMLog()) << "i: " << idx << " fb: " << i << endl;
		gCALMAPI->CALMSetFeedback( i );
		gCALMAPI->CALMReset( O_TIME );
		for ( epoch = 0; epoch < 100; epoch++ )
		{
			TestCurrentSequence( idx, &winner );			
		}
	}
	*(gCALMAPI->GetCALMLog()) << endl;
#endif

	// clean up the mess
	if ( gCALMAPI->CALMGetOrder() == kPermuted ) delete[] array;

#if PLOT3D			
	// stop 3D plotting
	gCALMAPI->CALMEnd3DPlot();
#endif
	return true;
}


void MultiSequence::TrainCurrentSequence( int idx )
{
// set the online pattern and train
	for ( int j = 0; j < mNumPats[idx]; j++ )
	{
		for ( int k = 0; k < gCALMAPI->CALMGetInputLen(); k++ )
		{
			gCALMAPI->CALMSetOnlineInput( k, mPatterns[idx][j][k] );
		}
		gCALMAPI->CALMTrainSingle( 0 );
		}
}


void MultiSequence::TestCurrentSequence( int idx, int* winner )
{
	// set the online pattern and test
	for ( int j = 0; j < mNumPats[idx]; j++ )
	{
		for ( int k = 0; k < gCALMAPI->CALMGetInputLen(); k++ )
		{
			gCALMAPI->CALMSetOnlineInput( k, mPatterns[idx][j][k] );
		}
		gCALMAPI->CALMTestSingle( 0 );
	}
// return the winner for the top module after presenting this whole sequence
	*winner = gCALMAPI->CALMGetWinnerForModule( mOutIdx );
}


// representation test: we clamp each output unit and run the net for an extended time
// to check the representation
void MultiSequence::ClampTest( void )
{
	data_type	acts[3];
	int			i, j, k, ite, ctr;
	
	*(gCALMAPI->GetCALMLog()) << "# clamp test" << endl;
												// set iterations to a high number
	int	numIters = gCALMAPI->CALMGetNumIterations();
	gCALMAPI->CALMSetNumIterations( 100 );
												// make sure the API is giving us the winners
	gCALMAPI->CALMSetVerbosity( O_WINNER );
												// set input to zeros
	for ( int k = 0; k < gCALMAPI->CALMGetInputLen(); k++ )
		gCALMAPI->CALMSetOnlineInput( k, 0.0 );
												// get size of output module
	int mOutSize = gCALMAPI->CALMGetModuleSize( mOutIdx );
	for ( i = 0; i < mOutSize; i++ )
	{
		*(gCALMAPI->GetCALMLog()) << "# clamp unit: " << i << endl;
		gCALMAPI->ClampUnit( mOutIdx, i, 1.0 );	// clamp the unit
		gCALMAPI->CALMReset( O_TIME );
		ctr = 0;
	// we have to present the same pattern repeatedly or the delay modules will remain inactive
		for ( j = 0; j < 1000; j++ )
		{
			gCALMAPI->CALMReset( O_ACT | O_WIN );
			
			for ( ite = 0; ite < gCALMAPI->CALMGetNumIterations(); ite++ )
			{
				gCALMAPI->CALMTest( ite, true );
				acts[ctr] = gCALMAPI->CALMSumActivation();
				ctr++;
				if ( ctr > 2 )
				{
					for ( k = 0; k < 3; k++ ) 
					{
						*(gCALMAPI->GetCALMLog()) << acts[k] << " ";
						if ( k != 2 ) acts[k] = acts[k+1];
					}
					*(gCALMAPI->GetCALMLog()) << endl;
				}
				ctr = 2;
			}
			
			gCALMAPI->CALMShowOnlineWinners( &cerr );
		}
		gCALMAPI->ClampUnit( mOutIdx, i );		// this unclamps it
		*(gCALMAPI->GetCALMLog()) << endl;
	}
												// reset the old settings
	gCALMAPI->CALMSetNumIterations( numIters );
}


// oscillation analysis: we collect sum of activations for each module
void MultiSequence::OscillationTest( void )
{
	int			i, j, k, ite, epoch, ctr, idx;
	float		noise;
	data_type	acts[2];
	
	*(gCALMAPI->GetCALMLog()) << "# oscillation test\n# ";
	
// set iterations to a high number
	int	numIters = gCALMAPI->CALMGetNumIterations();
	gCALMAPI->CALMSetNumIterations( 100 );

// no verbosity
	gCALMAPI->CALMSetVerbosity( O_NONE );

// print out module names
	gCALMAPI->CALMShowModules();
	
// run over all input patterns and print out the total activation for each module separately
	for ( i = 0; i < mNumFiles; i++ )
	{
		*(gCALMAPI->GetCALMLog()) << "\n# sequence " << i << endl;
		cerr << "\n# sequence " << i << endl;

		for ( noise = 0.1; noise <= 1.0; noise = noise + 0.05 )
		{	
			ctr = 0;
			idx = 0;
			*(gCALMAPI->GetCALMLog()) << "\n# noise = " << noise << endl;
			cerr << "\n# noise = " << noise << endl;
						
			gCALMAPI->CALMReset( O_TIME );
			for ( epoch = 0; epoch < 100; epoch++ )
			{
				cerr << epoch << endl;
				for ( j = 0; j < mNumPats[i]; j++ )
				{
					gCALMAPI->CALMReset( O_ACT | O_WIN );
				// create the input
					for ( k = 0; k < gCALMAPI->CALMGetInputLen(); k++ )
					{
						if ( mPatterns[i][j][k] > 0.9 )
							gCALMAPI->CALMSetOnlineInput( k, mPatterns[i][j][k] - noise * PseudoRNG() );
						else
							gCALMAPI->CALMSetOnlineInput( k, mPatterns[i][j][k] + noise * PseudoRNG() );
					}
				// set the input
					gCALMAPI->CALMSetInput();
				// test it	
					for ( ite = 0; ite < gCALMAPI->CALMGetNumIterations(); ite++ )
					{
						gCALMAPI->CALMTest( ite, false );
						acts[idx] = gCALMAPI->CALMSumActivation();
						idx++;
						if ( idx > 1 )
						{
							*(gCALMAPI->GetCALMLog()) << ctr++ << "\t";
							for ( k = 0; k < 2; k++ ) 
							{
								*(gCALMAPI->GetCALMLog()) << acts[k] << "\t";
								if ( k != 1 ) acts[k] = acts[k+1];
							}
							*(gCALMAPI->GetCALMLog()) << endl;
						}
						idx = 1;
					}
					cerr << j << "\t";
					gCALMAPI->CALMShowOnlineWinners( &cerr );
				}
			}
		}
	}
	gCALMAPI->CALMSetNumIterations( numIters );
}


// report winners for current input set
void MultiSequence::WinnerTest( void )
{
	int	i, j, k, ite, epoch;
	
	*(gCALMAPI->GetCALMLog()) << "# winners" << endl;

// set iterations to a high number
	int	numIters = gCALMAPI->CALMGetNumIterations();
	gCALMAPI->CALMSetNumIterations( 100 );

// no verbosity
	gCALMAPI->CALMSetVerbosity( O_WINNER );

// print out module names
	gCALMAPI->CALMShowModules();
	

// run over all input patterns and print out the winners
	for ( i = 0; i < mNumFiles; i++ )
	{			
		*(gCALMAPI->GetCALMLog()) << "# " << i << endl;
		gCALMAPI->CALMReset( O_TIME );
		for ( epoch = 0; epoch < 100; epoch++ )
		{
			for ( j = 0; j < mNumPats[i]; j++ )
			{
				gCALMAPI->CALMReset( O_ACT | O_WIN );
			// create the input
				for ( k = 0; k < gCALMAPI->CALMGetInputLen(); k++ )
					gCALMAPI->CALMSetOnlineInput( k, mPatterns[i][j][k] );
			// set the input
				gCALMAPI->CALMSetInput();
			// test it	
				for ( ite = 0; ite < gCALMAPI->CALMGetNumIterations(); ite++ )
					gCALMAPI->CALMTest( ite, false );
				gCALMAPI->CALMShowOnlineWinners();
			}
			*(gCALMAPI->GetCALMLog()) << endl;
		}
	}
	gCALMAPI->CALMSetNumIterations( numIters );
}


