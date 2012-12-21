/*
	Author:			Adriaan Tijsseling (AGT)
	Copyright: 		(c) Copyright 2002-2013 Adriaan Tijsseling. All rights reserved.
	Description:	API interface for the Categorization And Learning Module Network
*/

#include "CALMGlobal.h"	// contains project wide definitions and the like
#include "CALM.h"		// class definition for API interface
#include "Utilities.h"
#include "Rnd.h"


// This is the inititialization routine. This has to be called first!
CALMAPI::CALMAPI( void )
{
	// store stream defaults, since we will be manipulating width and precision
	GetStreamDefaults();
	
	// initialize the random number generator for the E-node activation
	SetSeed( GetSeed() );
	
	// store the current working directory
	getcwd( mCALMCurDir, FILENAME_MAX );
	strcpy( mCALMLogDir, mCALMCurDir );		// init log dir to same dir
	
	// Initialize and set the global network specifications data
	mNetwork = new CALMNetwork;	
	mInput = NULL;
	mInputLen = 0;
	mNumRuns = 1;
	mNumEpochs = 50;
	mNumIterations = 100;
	mOrder = kPermuted;
    mVerbosity = O_WINNER;
    mConvstop = false;
	mFBOn = false;
	strcpy( mBasename, "calm" );
	strcpy( mDirname, "." );
	mLogFile = NULL;
	mCALMLog = &cout;
}


// To terminate usage of the API
CALMAPI::~CALMAPI( void )
{
	// close the log file (if it was opened)
	CloseCALMLog();
	
	// Clean up!	
	if ( mNetwork != nil ) delete mNetwork;
	if ( mInput != nil ) delete[] mInput;

	// change back to original working directory
	chdir( mCALMCurDir );
}


// logfiles: specify a new log file for writing out network text date. 
	// pass the ofstream pointer and a file name
	// user should have set the directory beforehand, e.g. using CALMSetWorkingDirectory()
int CALMAPI::OpenCALMLog( char* logname )
{
	if ( logname == NULL )
	{
		cerr << "you need to pass a valid file name!" << endl;
		return kCALMFileError;
	}
	
	// close old file if still open
	CloseCALMLog();
	
	// create new file
	mLogFile = new ofstream( logname );
		
	// open a new logfile into the current directory
	if ( ! mLogFile->is_open() )
	{
		mLogFile->open( logname );
		if ( mLogFile->fail() )
		{
			cerr << "cannot create log file!" << endl;
			return kCALMFileError;
		}
	}
	// point internal log to this file
	mCALMLog = mLogFile;
	return kNoErr;
}

// close the redirected stdout and point the internal log to cout
void CALMAPI::CloseCALMLog( void )
{
	if ( mLogFile == NULL ) return;
	if ( mLogFile->is_open() ) mLogFile->close();
	delete mLogFile;
	mLogFile = NULL;
	mCALMLog = &cout;
}


int CALMAPI::CALMSetLogDirectory( char* dirname )
{
	// make sure we are in the right directory
	chdir( mCALMCurDir );
	
	// create the new directory if necessary
	if ( dirname != NULL )
	{
		strcpy( mCALMLogDir, dirname );
		mkdir( mCALMLogDir, S_IRWXU | S_IRWXG );
		if ( chdir( mCALMLogDir ) )
		{
			cerr << "cannot create or change to new directory!" << endl;
			return kCALMFileError;
		}
	}
	else
		strcpy( mCALMLogDir, mCALMCurDir );
	
	return kNoErr;
}

// This routine is for changing directories temporarily back to original working directory
// and back again to the log directory. Use it if opening or saving files
int CALMAPI::CALMDirectory( bool originalDir )
{
	if ( originalDir == kOriginalDir )
	{
		if ( chdir( mCALMCurDir ) )
		{
			cerr << "cannot change directories!" << endl;
			return kCALMFileError;
		}
	}
	else
	{
		if ( chdir( mCALMLogDir ) )
		{
			cerr << "cannot change directories!" << endl;
			return kCALMFileError;
		}
	}
	return kNoErr;
}


// Display simulation environment to cerr
void CALMAPI::CALMShow( void )
{
	*mCALMLog << "Simulation specifics:" << endl;
	*mCALMLog << "    number of runs           : " << mNumRuns << endl;
	*mCALMLog << "    number of epochs         : " << mNumEpochs << endl;
	*mCALMLog << "    number of iterations     : " << mNumIterations << endl;
	*mCALMLog << "    presentation order       : ";
	if ( mOrder == kLinear )
		*mCALMLog << "linear" << endl;
	else
		*mCALMLog << "permuted" << endl;
	*mCALMLog << "    terminate at convergence : ";
	if ( mConvstop )
		*mCALMLog << "yes" << endl;
	else
		*mCALMLog << "no" << endl;
	*mCALMLog << "    verbosity                : ";
	CALMShowVerbosity( mVerbosity );
	*mCALMLog << "    file name                : " << mBasename << endl;
	*mCALMLog << "    directory                : " << mDirname << endl;
}


// Display pattern file
void CALMAPI::CALMShowPatterns( void )
{
	*mCALMLog << "    loaded patterns:" << endl;
	mNetwork->PrintPatterns( mCALMLog );
	if ( mFBOn )
	{
		*mCALMLog << "    loaded feedback: ";
		mNetwork->PrintFeedback( mCALMLog );
	}
}


// Output the verbosity level in readable format
void CALMAPI::CALMShowVerbosity( int verbosity )
{
	if ( verbosity == O_NONE ) *mCALMLog << "silent mode";
	if ( verbosity & O_WINNER ) *mCALMLog << "winner ";
	if ( verbosity & O_ACTASIS ) *mCALMLog << "act ";
	if ( verbosity & O_ACTPLUS ) *mCALMLog << "act+ ";
	if ( verbosity & O_POT ) *mCALMLog << "volt ";
	if ( verbosity & O_WEIGHTS ) *mCALMLog << "wts ";
	if ( verbosity & O_SAVEDWT ) *mCALMLog << "dwt ";
	if ( verbosity & O_SAVEACT ) *mCALMLog << "dact ";
	if ( verbosity & O_SAVEMU ) *mCALMLog << "dmu ";
	*mCALMLog << endl;
}	


// Creates network
void CALMAPI::CALMSetupNetwork( int* errFlags )
{
	char filename[256];
	
	strcpy( filename, mDirname );
	strcat( filename, "/" );
	strcat( filename, mBasename );
	strcat( filename, ".net" );
	
	// initialize IO interface
	if ( mInput != NULL )
	{
		delete[] mInput;
		mInput = NULL;
	}
		
	if ( chdir( mCALMCurDir ) ) cerr << "cannot change directories!" << endl;
	// open the network specs file and read in the details
	if ( CALMReadSpecs( filename ) ) 
	{
		*errFlags = kCALMFileError;
		return;
	}
	if ( chdir( mCALMLogDir ) ) cerr << "cannot change directories!" << endl;

	// if changes have to be saved, open the necessary file
	if ( mVerbosity & O_SAVEDWT ) mNetwork->SetWeightChangeFile( mBasename );
	if ( mVerbosity & O_SAVEACT ) mNetwork->SetActChangeFile( mBasename );
	if ( mVerbosity & O_SAVEMU )  mNetwork->SetMuChangeFile( mBasename );

	*errFlags = kNoErr;
}


// Creates network
void CALMAPI::CALMSetupNetwork( int* errFlags, char* file )
{
	char filename[256];
	
	strcpy( filename, mDirname );
	strcat( filename, "/" );
	strcat( filename, file );
	strcat( filename, ".net" );

	// delete old network
	if ( mNetwork  != nil ) delete mNetwork;
	mNetwork = new CALMNetwork;	
	
	if ( CALMLoadParameters() != kNoErr )
	{
		*errFlags = kCALMFileError;
		return;
	}
	
	// initialize IO interface
	if ( mInput != NULL )
	{
		delete[] mInput;
		mInput = NULL;
	}
		
	if ( chdir( mCALMCurDir ) ) cerr << "cannot change directories!" << endl;
	// open the network specs file and read in the details
	if ( CALMReadSpecs( filename ) ) 
	{
		*errFlags = kCALMFileError;
		return;
	}
	if ( chdir( mCALMLogDir ) ) cerr << "cannot change directories!" << endl;

	// if changes have to be saved, open the necessary file
	if ( mVerbosity & O_SAVEDWT ) mNetwork->SetWeightChangeFile( mBasename );
	if ( mVerbosity & O_SAVEACT ) mNetwork->SetActChangeFile( mBasename );
	if ( mVerbosity & O_SAVEMU )  mNetwork->SetMuChangeFile( mBasename );

	*errFlags = kNoErr;
}


// Creates network
void CALMAPI::CALMWriteNetwork( int* errFlags, char* newname )
{
	char filename[256];
	
	strcpy( filename, mDirname );
	strcat( filename, "/" );
	strcat( filename, newname );
	strcat( filename, ".net" );
	
	if ( chdir( mCALMCurDir ) ) cerr << "cannot change directories!" << endl;
	// open the network specs file and read in the details
	if ( ! mNetwork->WriteSpecs( filename ) ) 
	{
		*errFlags = kCALMFileError;
		return;
	}
	if ( chdir( mCALMLogDir ) ) cerr << "cannot change directories!" << endl;

	*errFlags = kNoErr;
}


// Load a pattern file
int CALMAPI::CALMLoadPatterns( void )
{
	int 	err = kNoErr;
	char	filename[256];
	
	strcpy( filename, mDirname );
	strcat( filename, "/" );
	strcat( filename, mBasename );
	strcat( filename, ".pat" );

	if ( chdir( mCALMCurDir ) )
	{
		cerr << "cannot change directories!" << endl;
		return kCALMFileError;
	}
	if ( ! mNetwork->LoadPatterns( filename ) )
	{
		err = kCALMFileError;
	}
	if ( chdir( mCALMLogDir ) )
	{
		cerr << "cannot change directories" << filename << endl;
		err = kCALMFileError;
	}
	return err;
}


// Load a feedback file
int CALMAPI::CALMLoadFeedback( void )
{
	int err = kNoErr;

	if ( mFBOn == false )
	{
		cerr << "no feedback module has been set. Skipping." << endl;
		return err;
	}
	
	char filename[256];
	
	strcpy( filename, mDirname );
	strcat( filename, "/" );
	strcat( filename, mBasename );
	strcat( filename, ".fb" );

	if ( chdir( mCALMCurDir ) )
	{
		cerr << "cannot change directories!" << endl;
		return kCALMFileError;
	}
	if ( ! mNetwork->LoadFeedback( filename ) )
	{
		err = kCALMFileError;
	}
	if ( chdir( mCALMLogDir ) )
	{
		cerr << "cannot change directories" << filename << endl;
		err = kCALMFileError;
	}
	return err;
}


// Load parameter file
int CALMAPI::CALMLoadParameters( void )
{
	int 	err = kNoErr;
	char	filename[256];
	
	strcpy( filename, mDirname );
	strcat( filename, "/" );
	strcat( filename, mBasename );
	strcat( filename, ".par" );

	if ( chdir( mCALMCurDir ) )
	{
		cerr << "cannot change directories!" << endl;
		return kCALMFileError;
	}
	if ( ! mNetwork->LoadParameters( filename ) )
	{
		err = kCALMFileError;
	}
	if ( chdir( mCALMLogDir ) )
	{
		cerr << "cannot change directories!" << endl;
		err = kCALMFileError;
	}
	return err;
}


// When using online patterns (i.e. a continuous stream), set it explicitly
// It will also allocate the mInput datastream, the size of which
// is the sum off all input nodes of all inputmodules
void CALMAPI::CALMOnlinePatterns( void )
{
	int numInputNodes = 0;
	
	for ( int i = 0; i < mNumInputs; i++ )
		numInputNodes += mNetwork->GetModuleSize(i);
	mNetwork->OnlinePatterns();
	mInput = new data_type[numInputNodes];
	mInputLen = numInputNodes;
}


// Train one single input. That's all.
// Input should be specified in the CALMSpec, any output data is returned
// in the same data struct.
// In case you use feedback, make sure to set the winning node with CALMSetFeedback
int CALMAPI::CALMTrainSingle( int epoch )
{
	int 	i;
	bool	converged;

	// reset activations and winning node info
	mNetwork->Reset( O_ACT | O_WIN );
	mNetwork->ResetWtChangeSum();
	// set the current pattern
	mNetwork->SetInput( mInput );
	// iterate the pattern
	for ( i = 0; i < mNumIterations; i++ )
	{
		mNetwork->Learn();
		// save changes in weights if required
		// collect the winners
		converged = mNetwork->CollectWinners( 0, i );
		if ( converged && mConvstop ) break;
		// text output if necessary
		if ( mVerbosity & O_ACTASIS || mVerbosity & O_ACTPLUS )
			mNetwork->PrintActs( mCALMLog, epoch, i, mVerbosity, true );
		if ( mVerbosity & O_POT )
			mNetwork->PrintPotentials( mCALMLog );
	}
	// save changes in weights if required
	CALMSaveChanges();
	// text output if necessary
	if ( mVerbosity & O_WEIGHTS )
		mNetwork->PrintWeights( mCALMLog, epoch, i );
	if ( mVerbosity & O_WINNER )
		mNetwork->PrintWinners( mCALMLog );
	// should return any errors...
	return 0;
}


// Train sequence from pattern file
int CALMAPI::CALMTrainFile( int epoch )
{
	int		numPatterns = mNetwork->GetNumPatterns();
	int		i, j;
	bool	converged = false;
	
	mNetwork->Reset( O_WIN );
	
	for ( i = 0; i < numPatterns; i++ )
	{
		// reset activations and winning node info
		mNetwork->Reset( O_ACT );
		mNetwork->ResetWtChangeSum();
		// set the current pattern
		mNetwork->SetInput(i);
		if ( mFBOn ) mNetwork->SetFeedback(i);	
		// iterate the pattern
		for ( j = 0; j < mNumIterations; j++ )
		{
			mNetwork->Learn();
			// save changes in weights if required
			// collect the winners
			converged = mNetwork->CollectWinners( i, j );
			if ( converged && mConvstop ) break;
			// text output if necessary
			if ( mVerbosity & O_ACTASIS )
				mNetwork->PrintActs( mCALMLog, epoch, j, mVerbosity, true );
		}	
		// save changes in weights if required
		CALMSaveChanges();
		// text output if necessary
		if ( mVerbosity & O_ACTPLUS )
			mNetwork->PrintActs( mCALMLog, epoch, mNumIterations, mVerbosity, true );
		if ( mVerbosity & O_POT )
			mNetwork->PrintPotentials( mCALMLog );
		if ( mVerbosity & O_WEIGHTS )
			mNetwork->PrintWeights( mCALMLog, epoch, i );
	}	
	if ( mVerbosity & O_WINNER )
		mNetwork->PrintWinners( mCALMLog );
	
	// should return any errors...
	return 0;
}


// Test one single input. That's all.
// Input should be specified in the CALMSpec, any output data is returned
// in the same data struct.
int CALMAPI::CALMTestSingle( int epoch )
{
	int 	i;
	bool	converged;
	
	// reset activations and winning node info
	mNetwork->Reset( O_ACT | O_WIN );
	// set the current pattern
	mNetwork->SetInput( mInput );
	// iterate the pattern
	for ( i = 0; i < mNumIterations; i++ )
	{
		mNetwork->Test();
		// collect the winners
		converged = mNetwork->CollectWinners( 0, i );
		if ( converged && mConvstop ) break;
		// text output if necessary
		if ( mVerbosity & O_ACTASIS || mVerbosity & O_ACTPLUS )
			mNetwork->PrintActs( mCALMLog, epoch, i, mVerbosity, true );
	}
	if ( mVerbosity & O_WINNER )
		mNetwork->PrintWinners( mCALMLog );
		
	// should return any errors...
	return 0;
}


// Test a network with clamped units (no input should be set)
// Pass boolean value to specify whether to use noise
int CALMAPI::CALMTestSingle( int epoch, bool useNoise )
{
	int 	i;
	bool	converged;
	
	// reset activations and winning node info except
	// for clamped units (note that this will also set input to zeros
	mNetwork->Reset( O_ACT | O_WIN );

	// iterate the pattern
	for ( i = 0; i < mNumIterations; i++ )
	{
		mNetwork->Test( useNoise );
		// collect the winners
		converged = mNetwork->CollectWinners( 0, i );
		if ( converged && mConvstop ) break;
		// text output if necessary
		if ( mVerbosity & O_ACTASIS || mVerbosity & O_ACTPLUS )
			mNetwork->PrintActs( mCALMLog, epoch, i, mVerbosity, true );
	}
	if ( mVerbosity & O_WINNER ) mNetwork->PrintCurrentWinners( mCALMLog );
		
	// should return any errors...
	return 0;
}


// Test sequence from pattern file
int CALMAPI::CALMTestFile( int epoch )
{
	int			numPatterns = mNetwork->GetNumPatterns();
	int			i, j;
	bool		converged = false;
//	data_type	tmpER = CALMGetParameter( ER );
	
	mNetwork->Reset( O_WIN );
	for ( i = 0; i < numPatterns; i++ )
	{
		// reset activations and winning node info
		mNetwork->Reset( O_ACT );
		// set the current pattern
		mNetwork->SetInput(i);
		// iterate the pattern
		for ( j = 0; j < mNumIterations; j++ )
		{
			mNetwork->Test();
			// collect the winners
			converged = mNetwork->CollectWinners( i, j );
			if ( converged && mConvstop ) break;
			// text output if necessary
			if ( mVerbosity & O_ACTASIS )
				mNetwork->PrintActs( mCALMLog, epoch, j, mVerbosity, true );
		}			
		if ( mVerbosity & O_ACTPLUS )
			mNetwork->PrintActs( mCALMLog, epoch, mNumIterations, mVerbosity, true );
	}
	if ( mVerbosity & O_WINNER )
		mNetwork->PrintWinners( mCALMLog );
	
	// should return any errors...
	return 0;
}


// Test a network for only one pass. Assumes user has set input and resets values
// Requires iteration as parameter
bool CALMAPI::CALMTest( int i, bool useNoise )
{
	bool	converged;
	
	mNetwork->Test( useNoise );
	// collect the winners
	converged = mNetwork->CollectWinners( 0, i );
	if ( converged && mConvstop ) return true;
	// text output if necessary
	if ( mVerbosity & O_ACTASIS || mVerbosity & O_ACTPLUS )
		mNetwork->PrintActs( mCALMLog, 0, i, mVerbosity, true );
	return false;
}


// Called internally from within any training function. 
// Will save various data to file if specified
void CALMAPI::CALMSaveChanges( void )
{
	if ( mVerbosity & O_SAVEDWT ) mNetwork->SaveWeightChanges();		
	if ( mVerbosity & O_SAVEACT ) mNetwork->SaveActChanges();		
	if ( mVerbosity & O_SAVEMU )  mNetwork->SaveMuChanges();		
}


void CALMAPI::CALMSetVerbosity( int level )
{
	mVerbosity = level;
	// if changes have to be saved, open the necessary file
	if ( mVerbosity & O_SAVEDWT ) mNetwork->SetWeightChangeFile( mBasename );
	if ( mVerbosity & O_SAVEACT ) mNetwork->SetActChangeFile( mBasename );
	if ( mVerbosity & O_SAVEMU )  mNetwork->SetMuChangeFile( mBasename );
}


// Saves weights to file. Only pass base name without suffix. 
// The file will be created in the network files directory with
// .wts suffixed. 
void CALMAPI::CALMSaveWeights( char const *filename )
{
	char tmpname[256];

	strcpy( tmpname, mDirname );
	strcat( tmpname, "/" );
	strcpy( tmpname, filename );
	strcat( tmpname, ".wts" );
	mNetwork->SaveWeights( tmpname );
}


// Loads weights from file. Only pass base name without suffix. 
// The file will be loaded from the network files directory with
// .wts suffixed. 
int CALMAPI::CALMLoadWeights( char const *filename )
{
	char tmpname[256];
	
	strcpy( tmpname, mDirname );
	strcat( tmpname, "/" );
	strcpy( tmpname, filename );
	strcat( tmpname, ".wts" );
	if ( mNetwork->LoadWeights( tmpname ) )
		return kNoErr;
	else
		return kCALMFileError;
}


int	CALMAPI::CALMReadSpecs( char* filename )
{
	ifstream	infile;
	char		dummy[32];
	char		mdlname[32];
	int			mdltype, mdlsize, mdlidx, mdlconn, link, delay;
	int			i, j;
	
	// open the file
	infile.open( filename );
	if ( infile.fail() ) 
	{
		FileOpenError( filename );
		return kCALMFileError;
	}
	
	// read in number of CALM modules
	SkipComments( &infile );		// ignore any strings starting with #
	infile >> mNumModules;
	// read in number of input modules
	SkipComments( &infile );
	infile >> mNumInputs;

	// set up the module array
	mNetwork->SetNumModules( mNumModules, mNumInputs );
	
	// read in each module. Pattern modules should be specified first, 
	// before all other module types.
	for ( i = 0; i < mNumModules+mNumInputs; i++ )
	{
		// read name of module
		SkipComments( &infile );
		infile >> mdlname;
		// read module type
		SkipComments( &infile );
		infile >> dummy;
		mdltype = ModuleType( dummy );
		// read number of nodes
		SkipComments( &infile );
		infile >> mdlsize;
		// initialize the module
		mNetwork->InitializeModule( i, mdltype, mdlsize, mdlname );
	}
	// read in connections
	for ( int i = 0; i < mNumModules; i++ )
	{
		// read name of to-module
		SkipComments( &infile );
		infile >> mdlname;
		// get idx for to-module
		mdlidx = mNetwork->GetModuleIndex( mdlname );
		// read number of connections
		SkipComments( &infile );
		infile >> mdlconn;
		// set connections array for module
		mNetwork->SetNumConnections( mdlidx, mdlconn );
		// set up each single connection
		for ( j = 0; j < mdlconn; j++ )
		{	
			// read from-module
			SkipComments( &infile );
			infile >> mdlname;
			// get connection type
			SkipComments( &infile );
			infile >> dummy;
			link = ConnectionType( dummy );
			// with time delay connections, the time constant should be provided
			if ( link == kDelayLink )
			{
				SkipComments( &infile );
				infile >> delay;
			}
			else delay = 0;
			// connect the modules
			mNetwork->ConnectModules( j, mdlidx, mNetwork->GetModuleIndex( mdlname ), link, delay );
		}			
	}
	// close the file
	infile.close();
		
	return kNoErr;
}


int	CALMAPI::ModuleType( char* typeStr )
{
	if ( strcmp( "input", typeStr ) == 0 ) return O_INP;
	if ( strcmp( "calm", typeStr ) == 0 ) return O_CALM;
	if ( strcmp( "map", typeStr ) == 0 ) return O_MAP;
	if ( strcmp( "fb", typeStr ) == 0 )
	{
		mFBOn = true;
		return O_FB;
	}
	cerr << "Invalid module type :" << typeStr << endl;
	return 0;
}


int	CALMAPI::ConnectionType( char* typeStr )
{
	if ( strcmp( "delay", typeStr ) == 0 ) return kDelayLink;

	return kNormalLink;
}


void CALMAPI::CALMSpeedTest( bool start )
{
	if ( start == kStart )
	{
		mCALMStartTime = clock();
	}
	else
	{
		long tmpHrs, tmpMins, tmpSecs, tmpMicro;
		
		mCALMEndTime = clock();
		mCALMEndTime = mCALMEndTime - mCALMStartTime;
		tmpSecs = mCALMEndTime / CLOCKS_PER_SEC;
		tmpMins = tmpSecs / 60;
		tmpHrs  = tmpMins / 60;
		tmpSecs = tmpSecs % 60;
		tmpMins = tmpMins % 60;
		tmpMicro = (long)mCALMEndTime;
		tmpMicro = tmpMicro % 60;
		PrintNext( mCALMLog, kReturn );
		PrintNext( mCALMLog, kIntend );
		cerr << "\nsimulation took ";
		if ( tmpHrs > 0 ) 
			cerr << tmpHrs << " hours, " << tmpMins << " minutes, " << tmpSecs << " seconds and " << tmpMicro << " microsecs\n";
		else if ( tmpMins > 0 )
			cerr << tmpMins << " minutes, " << tmpSecs << " seconds and " << tmpMicro << " microsecs\n";
		else if ( tmpSecs > 0 )
			cerr << tmpSecs << " seconds and " << tmpMicro << " microsecs\n";
		else
			cerr << tmpMicro << " microseconds\n";
	}
}

