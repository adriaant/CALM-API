/*
	Author:			Adriaan Tijsseling (AGT)
	Copyright: 		(c) Copyright 2002-2013 Adriaan Tijsseling. All rights reserved.
	Description:	Implementation of CALMNetwork class
*/

#include "CALMGlobal.h"
#include "Utilities.h"
#include "CALMPatterns.h"
#include "ModuleMap.h"
#include "Feedback.h"
#include "Rnd.h"
#include "CALMNetwork.h"


// constructor just sets everything to zero
CALMNetwork::CALMNetwork()
{ 
	mNumModules = 0;
	mNumInputModules = 0;
	mNumPatterns = 0;
	mWtChangeSum = 0.0;
	mPatternOrder = kPermuted;
	mFeedback = kNoWinner;
	mModules = NULL;
	mPatternList = NULL;
	mFeedbackList = NULL;
	mPermutations = NULL;
	mWinners = NULL;
	mConvTimes = NULL;
	mGnuPlot = NULL;
	m3DPlot = NULL;
}

// destructor: Free up allocated memory
CALMNetwork::~CALMNetwork() 
{
	int i;

	// thanks to polymorphism we can get rid of all modules in once, whether they
	// are derived Module classes or not. 
	if ( mModules != NULL )
	{
		for ( i = 0; i < mNumModules+mNumInputModules; i++ ) delete mModules[i];
		delete[] mModules;
	}
	if ( mWinners != NULL )
	{
		for ( i = 0; i < mNumModules; i++ ) delete[] mWinners[i];
		delete[] mWinners;
	}
	if ( mConvTimes != NULL )
	{
		for ( i = 0; i < mNumModules; i++ ) delete[] mConvTimes[i];
		delete[] mConvTimes;
	}

	delete[] mPatternList;
	delete[] mPermutations;

	delete mFeedbackList;
	
	if ( mGnuPlot != NULL ) delete mGnuPlot;

	if ( mWeightChangeFile.is_open() ) mWeightChangeFile.close();
	if ( mActChangeFile.is_open() ) mActChangeFile.close();
	if ( mMuChangeFile.is_open() ) mMuChangeFile.close();
}


// set up array for modules
void CALMNetwork::SetNumModules( int numModules, int numInputs )
{
	// define number of modules
	mNumModules	= numModules;
	// define number of input modules
	mNumInputModules = numInputs;
	// create array of CALM(Map) modules, but we still need to initialize each one!
	mModules = new Module*[ mNumModules+mNumInputModules ];
}


// initialize a module
void CALMNetwork::InitializeModule( int idx, int calmType, int moduleSize, char* moduleName )
{
	switch ( calmType )	// to be expanded
	{
		case O_CALM:
		case O_INP:
			mModules[idx] = new Module;
			break;
		case O_MAP:
			mModules[idx] = new ModuleMap;
			break;
		case O_FB:
			mModules[idx] = new Feedback;
			mFeedback = idx;
			break;
	}
	mModules[idx]->Initialize( moduleSize, moduleName, mParameters, calmType, idx );
}


// finds the idx of a module from a given name
int CALMNetwork::GetModuleIndex( char const *mdlname )
{
	for ( int i = 0; i < mNumInputModules+mNumModules; i++ )
	{
		if ( strcmp( mdlname, mModules[i]->GetModuleName() ) == 0 )
			return i;
	}
	cerr << "\tError: No module with name " << mdlname << endl;
	return 0;
}


// set the number of connections for a given module
void CALMNetwork::SetNumConnections( int idx, int numConn )
{
	// just allocates the connections array for further initialization
	mModules[idx]->SetNumConn( numConn );
}

// connect two modules
void CALMNetwork::ConnectModules( int idx, int toIdx, int fromIdx, int link, int delay )
{
	// initializes a given connection
	mModules[toIdx]->Connect( idx, mModules[fromIdx], link, delay );
}


// loops through all modules and checks if they need to grow or shrink
bool CALMNetwork::ResizeModule( void )
{
	int		node, newsize;
	bool	has_resized = false;
	
	for ( int i = mNumInputModules; i < mNumInputModules+mNumModules; i++ )
	{
		// CALMMap modules cannot be resized
		if ( mModules[i]->GetModuleType() == O_MAP || mModules[i]->GetModuleType() == O_FB ) continue;

		node = kUndefined;
		if ( mModules[i]->NeedsResizing( &node ) )
		{
			has_resized = true;
			newsize = mModules[i]->GetModuleSize();
			if ( node == kUndefined )
			{
				newsize = newsize + 1;
				cerr << mModules[i]->GetModuleName() << " +1 -> " << newsize << endl;
			}
			else
			{
				newsize = newsize - 1;
				if ( newsize != 1 )
					cerr << mModules[i]->GetModuleName() << " -1 @ " << node << " -> " << newsize << endl;
			}
			// minimum module size should be 2
			if ( newsize != 1 )
			{
				// first resize weight matrices on connections from resized module
				for ( int j = mNumInputModules; j < mNumInputModules+mNumModules; j++ )
					mModules[j]->ResizeConnection( newsize, node, i );
				
				// resize the specified module
				mModules[i]->ResizeModule( newsize, node );
			}
		}
	}
	return has_resized;
}

// checks if indicated module needs to grow or shrink
void CALMNetwork::ResizeModule( int idx )
{
	int	node, newsize;
	
	// CALMMap modules cannot be resized
	if ( mModules[idx]->GetModuleType() == O_MAP ) return;
	
	if ( mModules[idx]->NeedsResizing( &node ) )
	{
		newsize = mModules[idx]->GetModuleSize();
		if ( node == kUndefined )
			newsize = newsize + 1;
		else
			newsize = newsize - 1;
		
		// minimum module size should be 2
		if ( newsize != 1 )
		{
			// first resize weight matrices on connections from resized module
			for ( int j = mNumInputModules; j < mNumInputModules+mNumModules; j++ )
				mModules[j]->ResizeConnection( newsize, node, idx );
	
			// resize the specified module
			mModules[idx]->ResizeModule( newsize, node );
		}
	}
}

// grows a module to "newsize"
void CALMNetwork::ResizeModule( int idx, int newsize )
{
	// first resize weight matrices on connections from resized module
	for ( int i = mNumInputModules; i < mNumInputModules+mNumModules; i++ )
		mModules[i]->ResizeConnection( newsize, kUndefined, idx );
	
	// resize the specified module
	mModules[idx]->ResizeModule( newsize, kUndefined );
}


// in online mode we will not have a list of patterns ready
// so by default, we will use a list of only one pattern
void CALMNetwork::OnlinePatterns( void )
{
	int i;
	
	// delete the old list
	if ( mFeedbackList != NULL ) delete mFeedbackList;
	if ( mPatternList  != NULL ) delete[] mPatternList;
	if ( mPermutations != NULL ) delete[] mPermutations;
	if ( mWinners != NULL )
	{
		for ( i = 0; i < mNumModules; i++ ) delete[] mWinners[i];
		delete[] mWinners;
	}
	if ( mConvTimes != NULL )
	{
		for ( i = 0; i < mNumModules; i++ ) delete[] mConvTimes[i];
		delete[] mConvTimes;
	}

	mNumPatterns = 1;
	// create winners data storage for just one single pattern
	mWinners = new int*[mNumModules];
	for ( i = 0; i < mNumModules; i++ )
		mWinners[i] = new int[mNumPatterns];
	// create convtime data storage
	mConvTimes = new int*[mNumModules];
	for ( i = 0; i < mNumModules; i++ )
		mConvTimes[i] = new int[mNumPatterns];
}


// Loads patterns from specified file
bool CALMNetwork::LoadPatterns( const char* filename )
{
	ifstream	infile;
	int			i;

	// delete the old list
	if ( mPatternList  != NULL ) delete[] mPatternList;
	if ( mPermutations != NULL ) delete[] mPermutations;
	if ( mWinners != NULL )
	{
		for ( i = 0; i < mNumModules; i++ ) delete[] mWinners[i];
		delete[] mWinners;
	}
	if ( mConvTimes != NULL )
	{
		for ( i = 0; i < mNumModules; i++ ) delete[] mConvTimes[i];
		delete[] mConvTimes;
	}
			
	// store filename for later reference
	strcpy( mPatternFileName, filename );

	// allocate memory for Patterns array
	mPatternList = new CALMPatterns[mNumInputModules];
	
	// open the file
	infile.open( mPatternFileName );
	if ( infile.fail() )
	{
		FileOpenError( mPatternFileName );
		return false;
	}

	// read the number of patterns: should be the same for all input modules
	SkipComments( &infile );
	infile >> mNumPatterns;

	// read pattern set for each input module
	for ( i = 0; i < mNumInputModules; i++ )
		mPatternList[i].LoadPatterns( &infile, mNumPatterns, GetModuleSize(i) );
	
	// close file
	infile.close();
	
	// allocate permutations array
	mPermutations = new int[mNumPatterns];
	for ( i = 0; i < mNumPatterns; i++ ) mPermutations[i] = i;

	// create winners data storage
	mWinners = new int*[mNumModules];
	for ( i = 0; i < mNumModules; i++ )
		mWinners[i] = new int[mNumPatterns];
	// create convtime data storage
	mConvTimes = new int*[mNumModules];
	for ( i = 0; i < mNumModules; i++ )
		mConvTimes[i] = new int[mNumPatterns];
	// reset winners infos
	Reset( O_WIN );

	return true;
}


// return pattern
data_type* CALMNetwork::GetPattern( int mIdx, int pIdx )
{
	if ( mPermutations != NULL ) pIdx = mPermutations[pIdx];
	return mPatternList[mIdx].GetPattern( pIdx );
}

data_type CALMNetwork::GetPattern( int mIdx, int pIdx, int idx )
{
	if ( mPermutations != NULL ) pIdx = mPermutations[pIdx];
	return mPatternList[mIdx].GetPattern( pIdx, idx );
}


// sets the order of pattern presentations to either linear or permuted
void CALMNetwork::SetPatternOrder( int order )
{ 
	mPatternOrder = order;
	if ( mPatternOrder == kLinear )
		// set to ordered indices
		for ( int i = 0; i < mNumPatterns; i++ ) mPermutations[i] = i;
}


// Permutes the indices for the patterns so we can present patterns in shuffled order
void CALMNetwork::PermutePatterns( void )
{
	// set back ordered indices
	for ( int i = 0; i < mNumPatterns; i++ ) mPermutations[i] = i;
	// shuffle
	Permute( mPermutations, mNumPatterns );
}


// Loads patterns from specified file
bool CALMNetwork::LoadFeedback( const char* filename )
{
	ifstream	infile;
	int			numPatterns;

	// check if some module has been designated for feedback
	if ( mFeedback == kNoWinner )
	{
		cerr << "\tError: There is no module designated for feedback!\n";
		return false;
	}

	// delete the old list
	if ( mFeedbackList != NULL ) delete mFeedbackList;

	// open the file
	infile.open( filename );
	if ( infile.fail() )
	{
		FileOpenError( (char*)filename );
		return false;
	}

	// read the number of patterns: should be the same for all input modules
	// and should equal mNumPatterns
	SkipComments( &infile );
	infile >> numPatterns;
	if ( numPatterns != mNumPatterns )
	{
		cerr << "\tError: Number of feedback entries does not match number of patterns!\n";
		infile.close();
		return false;
	}

	// allocate memory for feedback array
	mFeedbackList = new int[mNumPatterns];

	// read each pattern in
	for ( int i = 0; i < mNumPatterns; i++ )
	{
		SkipComments( &infile );	// ignore any comments;
		infile >> mFeedbackList[i];
	}

	// close file
	infile.close();
	
	return true;
}


// return feedback
int CALMNetwork::GetFeedback( int pIdx )
{
	if ( mPermutations != NULL ) pIdx = mPermutations[pIdx];
	return mFeedbackList[pIdx];
}


// Set the pattern from preloaded patterndata
void CALMNetwork::SetInput( int pIdx )
{
	pIdx = mPermutations[pIdx];	// permutations array holds the correct order
	for ( int i = 0; i < mNumInputModules; i++ )
		mModules[i]->SetInput( mPatternList[i].GetPattern( pIdx ) );
}

// Set the pattern from preloaded patterndata
void CALMNetwork::SetInput( int mIdx, int pIdx )
{
	pIdx = mPermutations[pIdx];
	mModules[mIdx]->SetInput( mPatternList[mIdx].GetPattern( pIdx ) );
}

// Set pattern from stream: Note that this assumes user has taken note
// of the order input-modules will be read in from the .net file
void CALMNetwork::SetInput( data_type* input )
{
	int i, j, k = 0;
	for ( i = 0; i < mNumInputModules; i++ )
	{
		for ( j = 0; j < GetModuleSize(i); j++ )
		{
			mModules[i]->SetInput( input[k], j );
			k++;
		}
	}
}

// Set pattern of given input module to double vector
void CALMNetwork::SetInput( int mIdx, data_type* inp )
{
	mModules[mIdx]->SetInput( inp );
}


// Set feedback of given feedback module to index of node
void CALMNetwork::SetOnlineFeedback( int fb )
{
	dynamic_cast<Feedback*>(mModules[mFeedback])->SetFeedback( fb );
}

// Set feedback of given feedback module to index of node
void CALMNetwork::SetFeedback( int pIdx )
{
	pIdx = mPermutations[pIdx];	// permutations array holds the correct order
	dynamic_cast<Feedback*>(mModules[mFeedback])->SetFeedback( mFeedbackList[pIdx] );
}


// creates and opens the file for recording weightchanges
void CALMNetwork::SetWeightChangeFile( char* filename )
{
	char tmpname[256];
	
	strcpy( tmpname, filename );
	strcat( tmpname, ".dwt" );
	if ( mWeightChangeFile.is_open() ) mWeightChangeFile.close();
	mWeightChangeFile.open( tmpname );
	if ( mWeightChangeFile.fail() ) FileCreateError( tmpname );
	
	mGnuPlot = new GnuPlot( 0, 500, -2, 2 );
}

// sums all changes in weights and saves them to file.
// this has to be called after every weight update, if desired
void CALMNetwork::SaveWeightChanges( void )
{
	mWeightChangeFile << mWtChangeSum << endl;
	
	mGnuPlot->plot( mWtChangeSum );
}


// creates and opens the file for recording total activation after each iteration
void CALMNetwork::SetActChangeFile( char* filename )
{
	char tmpname[256];
	
	strcpy( tmpname, filename );
	strcat( tmpname, ".dact" );
	if ( mActChangeFile.is_open() ) mActChangeFile.close();
	mActChangeFile.open( tmpname );
	if ( mActChangeFile.fail() ) FileCreateError( tmpname );
}

// sums activation and saves it to file.
// this has to be called after every iteration, if desired
void CALMNetwork::SaveActChanges( void )
{
	// sum all act changes and write to file
	mActChangeFile << SumActivation() << endl;
}

// sums activation
data_type CALMNetwork::SumActivation( void )
{
	data_type act_sum = 0.0;	
	// sum all act changes
	for ( int i = mNumInputModules; i < mNumInputModules+mNumModules; i++ )
		mModules[i]->SumActivation( act_sum );
	return act_sum;
}

// sums activation for selected module only
data_type CALMNetwork::SumActivation( int mdl )
{
	data_type act_sum = 0.0;	
	// sum all act changes
	mModules[mdl]->SumActivation( act_sum );
	return act_sum;
}

// sums activation
data_type CALMNetwork::SumActivationR( void )
{
	data_type act_sum = 0.0;	
	// sum all act changes
	for ( int i = mNumInputModules; i < mNumInputModules+mNumModules; i++ )
		mModules[i]->SumActivationR( act_sum );
	return act_sum;
}

// sums activation for selected module only
data_type CALMNetwork::SumActivationR( int mdl )
{
	data_type act_sum = 0.0;	
	// sum all act changes
	mModules[mdl]->SumActivationR( act_sum );
	return act_sum;
}

// sums activation
data_type CALMNetwork::SumActivationV( void )
{
	data_type act_sum = 0.0;	
	// sum all act changes
	for ( int i = mNumInputModules; i < mNumInputModules+mNumModules; i++ )
		mModules[i]->SumActivationV( act_sum );
	return act_sum;
}

// sums activation for selected module only
data_type CALMNetwork::SumActivationV( int mdl )
{
	data_type act_sum = 0.0;	
	// sum all act changes
	mModules[mdl]->SumActivationV( act_sum );
	return act_sum;
}


// creates and opens the file for recording total activation after each iteration
void CALMNetwork::SetMuChangeFile( char* filename )
{
	char tmpname[256];
	
	strcpy( tmpname, filename );
	strcat( tmpname, ".dmu" );
	if ( mMuChangeFile.is_open() ) mMuChangeFile.close();
	mMuChangeFile.open( tmpname );
	if ( mMuChangeFile.fail() ) FileCreateError( tmpname );
}

// averages learning rate and saves it to file.
// this has to be called after every iteration, if desired
void CALMNetwork::SaveMuChanges( void )
{
	data_type mu_sum = 0.0;
	
	// sum all weight changes
	for ( int i = mNumInputModules; i < mNumInputModules+mNumModules; i++ )
		mu_sum += mModules[i]->GetLearningRate();
	mu_sum = mu_sum / (data_type)mNumModules;
	// write it to file
	mMuChangeFile << mu_sum << endl;
}


void CALMNetwork::SaveWeights( char* filename )
{
	ofstream outfile;
	
	outfile.open( filename );
	if ( outfile.fail() )
	{
		FileCreateError( filename );
		return;
	}
	for ( int i = mNumInputModules; i < mNumInputModules+mNumModules; i++ )
		mModules[i]->SaveWeights( &outfile );
	outfile.close();
}


bool CALMNetwork::LoadWeights( char* filename )
{
	ifstream infile;
	
	infile.open( filename );
	if ( infile.fail() )
	{
		FileOpenError( filename );
		return false;
	}
	for ( int i = mNumInputModules; i < mNumInputModules+mNumModules; i++ )
		mModules[i]->LoadWeights( &infile );
	infile.close();
	return true;
}


/*--------------------------------------*
 *		      RESET FUNCTION			*
 *--------------------------------------*/

void CALMNetwork::Reset( SInt16 resetOption )
{
	int i;

	if ( resetOption & O_TIME )
	{
		for ( i = 0; i < mNumInputModules; i++ ) mModules[i]->ResetInput();
	}
	
	for ( i = mNumInputModules; i < mNumModules+mNumInputModules; i++ )
		mModules[i]->Reset( resetOption );
		
	if ( resetOption & O_WIN )
	{
		for ( i = 0; i < mNumModules; i++ )
		{
			for ( int j = 0; j < mNumPatterns; j++ )
			{
				mWinners[i][j] = kNoWinner;
				mConvTimes[i][j] = kNoWinner;
			}
		}
	}
}

// this resets only input module activations to zero
void CALMNetwork::Reset( void )
{
	for (int i = 0; i < mNumInputModules; i++ ) mModules[i]->Reset();
}

// this resets only input module activations to zero
void CALMNetwork::UpdateTimeDelay( void )
{
	for ( int i = mNumInputModules; i < mNumModules+mNumInputModules; i++ )
		mModules[i]->UpdateTimeDelay();
}


/*--------------------------------------*
 *		  SETTINGS FUNCTIONS 			*
 *--------------------------------------*/

bool CALMNetwork::LoadParameters( const char* filename )
{
	ifstream infile;

	// open the file
	infile.open( filename );
	if ( infile.fail() )
	{
		FileOpenError( (char*)filename );
		return false;
	}
	// read in each parameter
	for ( int i = 0; i < gNumPars; i++ )
	{
		// skip comments
		SkipComments( &infile );
		// read parameter
		infile >> mParameters[i];
	}

	// close the file
	infile.close();	
	return true;
}


void CALMNetwork::SetParameter( int parType, data_type val )
{
	mParameters[parType] = val;
	if ( parType == SIGMA )
	{
		for ( int i = mNumInputModules; i < mNumModules+mNumInputModules; i++ )
		{
			if ( mModules[i]->GetModuleType() == O_MAP )
				dynamic_cast<ModuleMap *>(mModules[i])->SetInhibitionMap();
		}
	}
}

data_type CALMNetwork::GetParameter( int parType )
{
	return mParameters[parType];
}


/*--------------------------------------*
 *		  TRAINING FUNCTIONS 			*
 *--------------------------------------*/

// Train single pass
void CALMNetwork::Learn( void )
{
	int i;
	
	for ( i = mNumInputModules; i < mNumModules+mNumInputModules; i++ )
		mModules[i]->UpdateActivation();
	for ( i = mNumInputModules; i < mNumModules+mNumInputModules; i++ )
		mModules[i]->UpdateWeights( mWtChangeSum );
	for ( i = mNumInputModules; i < mNumModules+mNumInputModules; i++ )
		mModules[i]->SwapActs();
}


// Test single pass
void CALMNetwork::Test( void )
{
	int i;
	
	for ( i = mNumInputModules; i < mNumModules+mNumInputModules; i++ )
		mModules[i]->UpdateActivationTest();
	for ( i = mNumInputModules; i < mNumModules+mNumInputModules; i++ )
		mModules[i]->SwapActs();
}


// Test single pass for clamp routine; indicate whether to use noise
void CALMNetwork::Test( bool useNoise )
{
	int i;
	
	for ( i = mNumInputModules; i < mNumModules+mNumInputModules; i++ )
		mModules[i]->UpdateActivationTest( useNoise );
	for ( i = mNumInputModules; i < mNumModules+mNumInputModules; i++ )
		mModules[i]->SwapActs();
}


// collect winners for each module, store them, and return convergence info
bool CALMNetwork::CollectWinners( int pIdx, int ite )
{
	bool converged = true;
	int	 i, winner, convtime;

	// in case of online learning, there are no permutations
	if ( mPermutations != NULL ) pIdx = mPermutations[pIdx];
		
	for ( i = mNumInputModules; i < mNumInputModules+mNumModules; i++ )
	{
		mModules[i]->ConvCheck( ite, &winner, &convtime );
		if ( winner != kNoWinner )
		{
			SetWinner( i-mNumInputModules, pIdx, winner );
			SetConvTime( i-mNumInputModules, pIdx, convtime );
		}
		else
			converged = false;
	}
	return converged;
}


/*--------------------------------------*
 *		      3D Plot FUNCTIONS 	    *
 *--------------------------------------*/

void CALMNetwork::Init3DPlot( const char* fromMdl, const char* toMdl )
{
	m3DTo = GetModuleIndex( toMdl );	
	for ( int j = 0; j < mModules[m3DTo]->GetNumInConn(); j++ )
	{	
		if ( strcmp( fromMdl,  mModules[m3DTo]->GetConnModuleName(j) ) == 0 )
		{
			m3DIdx = j;
			break;
		}
	}			
	m3DPlot = new GnuPlot( GetModuleSize( GetModuleIndex( toMdl ) ),
						   GetModuleSize( GetModuleIndex( fromMdl ) ) );
}

void CALMNetwork::Resize3DPlot( const char* fromMdl, const char* toMdl )
{
	m3DTo = GetModuleIndex( toMdl );	
	for ( int j = 0; j < mModules[m3DTo]->GetNumInConn(); j++ )
	{	
		if ( strcmp( fromMdl,  mModules[m3DTo]->GetConnModuleName(j) ) == 0 )
		{
			m3DIdx = j;
			break;
		}
	}			
	m3DPlot->resizePlot( GetModuleSize( GetModuleIndex( toMdl ) ),
						   GetModuleSize( GetModuleIndex( fromMdl ) ) );
}

void CALMNetwork::End3DPlot( void )
{
	if ( m3DPlot != NULL ) delete m3DPlot;
}

void CALMNetwork::Do3DPlot( void )
{
	mModules[m3DTo]->CopyWeights( m3DIdx, m3DPlot->GetMatrix() );
	m3DPlot->plot();
}


/*--------------------------------------*
 *		   MISCELLANY FUNCTIONS 	    *
 *--------------------------------------*/

// returns the size of the largest module; we use this to figure out text spacing in logs
int CALMNetwork::GetMaximumSize( void )
{
	int maxsize = 0;
	int modsize;
	
	for ( int i = mNumInputModules; i < mNumInputModules+mNumModules; i++ )
	{
		modsize = mModules[i]->GetModuleSize();
		if ( modsize > maxsize ) maxsize = modsize;
	}
	return maxsize;
}

	
bool CALMNetwork::WriteSpecs( char* filename )
{
	ofstream	outfile;
	int			i, j;
	
	// open the file
	outfile.open( filename );
	if ( outfile.fail() ) 
	{
		FileCreateError( filename );
		return false;
	}
	
	// write number of CALM modules
	outfile << "# number of calm modules\n" << mNumModules << endl;
	// write number of input modules
	outfile << "# number of input modules\n" << mNumInputModules << endl;

	outfile << "# module names, types and sizes" << endl;
	// write each module. Pattern modules should be specified first, 
	// before all other module types.
	for ( i = 0; i < mNumModules+mNumInputModules; i++ )
	{
		// write name of module
		outfile << mModules[i]->GetModuleName() << "\t";
		// write module type
		outfile << GetTypeString(mModules[i]->GetModuleType()) << "\t";
		// write number of nodes
		outfile << mModules[i]->GetModuleSize() << endl;
	}
	// write connections
	outfile << "# connections" << endl;
	for ( i = mNumInputModules; i < mNumModules+mNumInputModules; i++ )
	{
		// write name of to-module
		outfile << mModules[i]->GetModuleName() << "\t";
		// write number of connections
		outfile << mModules[i]->GetNumInConn() << "\t";
		// write each single connection
		for ( j = 0; j < mModules[i]->GetNumInConn(); j++ )
		{	
			// read from-module
			outfile << mModules[i]->GetConnModuleName(j) << " ";
			// get connection type
			// with time delay connections, the time constant should be provided
			if ( mModules[i]->GetConnType(j) == kDelayLink )
			{
				outfile << "delay ";
				outfile << mModules[i]->GetConnDelay(j) << " ";
			}
			else
				outfile << "normal ";
		}			
		outfile << endl;
	}
	// close the file
	outfile.close();
		
	return true;
}


// get the string for a module type
const char* CALMNetwork::GetTypeString( int modtype )
{
	if  ( modtype == O_INP ) return "input";
	if  ( modtype == O_CALM ) return "calm";
	if  ( modtype == O_MAP ) return "map";
	if  ( modtype == O_FB ) return "fb";
	
	cerr << "Invalid module type : " << modtype << endl;
	return "undefined";
}


// print out the number of used nodes in each module. For map modules, 
// we also check if the winners are ordered (obviously, this only works
// if the patterns are ordered to similarity themselves. It's only for 
// testing purposes I add this. 
void CALMNetwork::PrintCommitted( ostream* os )
{
	bool	sorted = true;
	int		i;
	
	AdjustStream( *os, 0, 2, kLeft, false );
	for ( i = 0; i < mNumModules; i++ )
	{
		*os << mModules[i+mNumInputModules]->GetModuleName() << ": ";
		if ( mModules[i+mNumInputModules]->GetModuleType() == O_MAP )
		{
			sorted = true;
			*os << '\t' << Unique( os, mWinners[i], mNumPatterns, 
									mModules[i+mNumInputModules]->GetModuleSize(), &sorted );
			if ( sorted )
				*os << ' ' << "OK" << '\t';
			else
				*os << ' ' << "NG" << '\t';
		}
		else
		{
			sorted = false;
			*os << '\t' << Unique( os, mWinners[i], mNumPatterns, mModules[i+mNumInputModules]->GetModuleSize(), &sorted );
		}		
		*os << endl;
	}
	SetStreamDefaults( *os );
}

void CALMNetwork::PrintModules( ostream* os )
{
	AdjustStream( *os, 0, 2, kLeft, false );
	*os << "   ";
	for ( int i = mNumInputModules; i < mNumInputModules+mNumModules; i++ )
		*os << mModules[i]->GetModuleName() << ' ';
	*os << endl;
	SetStreamDefaults( *os );
}

void CALMNetwork::PrintWinners( ostream* os )
{
	int 	i, j, node;
	int		spacing = GetMaximumSize() / 10 + 2;

	AdjustStream( *os, 0, 1, kLeft, false );
	for ( i = 0; i < mNumPatterns; i++ )
	{
		*os << i << ": ";
		for ( j = mNumInputModules; j < mNumInputModules+mNumModules; j++ )
		{
			node = GetWinner( j-mNumInputModules, i );
			AdjustStream( *os, 0, spacing, kLeft, false );
			if ( node != kNoWinner )
				*os << node;
			else
				*os << "*";
		#if PRINT_TIME
			AdjustStream( *os, 0, 1, kLeft, false );
			*os << "@ ";
			AdjustStream( *os, 0, 4, kLeft, false );
			*os << GetConvTime( j-mNumInputModules, i );
		#endif
		}
		*os << endl;
	}
	SetStreamDefaults( *os );
}

// alternate version to bypass pattern storage: just give the winners for 
// the current run
void CALMNetwork::PrintCurrentWinners( ostream* os )
{
	int 	i, node;
	int		spacing = GetMaximumSize() / 10 + 2;

	for ( i = mNumInputModules; i < mNumInputModules+mNumModules; i++ )
	{
		node = mModules[i]->GetWinner();
	#if PRINT_TIME
		AdjustStream( *os, 0, 1, kLeft, false );
	#else
		AdjustStream( *os, 0, spacing, kLeft, false );
	#endif
		if ( node != kNoWinner )
			*os << node;
		else
			*os << "*";
	#if PRINT_TIME
		AdjustStream( *os, 0, 1, kLeft, false );
		*os << " @ ";
		AdjustStream( *os, 0, 4, kLeft, false );
		*os << mModules[i]->GetConvTime();
	#endif
	}
	*os << endl;
	SetStreamDefaults( *os );
}

void CALMNetwork::PrintWeights( ostream* os, int epoch, int pIdx )
{
	// in case of online learning, there are no permutations
	if ( mPermutations != NULL ) pIdx = mPermutations[pIdx];
	*os << endl << "Weights for epoch " << epoch << " and pattern " << pIdx << endl;
	for ( int i = mNumInputModules; i < mNumInputModules+mNumModules; i++ )
		mModules[i]->PrintWeights( os );
	*os << endl;
}

void CALMNetwork::PrintWeights( ostream* os )
{
	*os << endl << "Weights: " << endl;
	for ( int i = mNumInputModules; i < mNumInputModules+mNumModules; i++ )
		mModules[i]->PrintWeights( os );
	*os << endl;
}

void CALMNetwork::PrintActs( ostream* os, int epoch, int ite, int format, bool withInp )
{
	*os << endl << "Activations for epoch " << epoch << " and iteration " << ite << endl;
	if ( withInp )
		for ( int i = 0; i < mNumInputModules+mNumModules; i++ )
			mModules[i]->PrintActs( os, format );
	else
		for ( int i = mNumInputModules; i < mNumInputModules+mNumModules; i++ )
			mModules[i]->PrintActs( os, format );
	*os << endl;
}

void CALMNetwork::PrintActs( ostream* os, int pat, int format, bool withInp )
{
	*os << endl << "epoch " << pat << endl;
	if ( withInp )
		for ( int i = 0; i < mNumInputModules+mNumModules; i++ )
			mModules[i]->PrintActs( os, format );
	else
		for ( int i = mNumInputModules; i < mNumInputModules+mNumModules; i++ )
			mModules[i]->PrintActs( os, format );
}

void CALMNetwork::PrintPotentials( ostream* os )
{
	for ( int i = mNumInputModules; i < mNumInputModules+mNumModules; i++ )
		mModules[i]->PrintPotentials( os );
	*os << endl;
}

// prints out the sizes of the modules in the network (useful after pruning/growing)
void CALMNetwork::PrintSizes( ostream* os )
{
	for ( int i = mNumInputModules; i < mNumInputModules+mNumModules; i++ )
		mModules[i]->PrintSizes( os );	
}

void CALMNetwork::PrintPatterns( ostream* os )
{
	// read pattern set for each input module
	for ( int i = 0; i < mNumInputModules; i++ )
		*os << mPatternList[i];
}

void CALMNetwork::PrintFeedback( ostream* os )
{
	// read pattern set for each input module
	if ( mFeedbackList != NULL )
	{
		for ( int i = 0; i < mNumPatterns; i++ )
			*os << mFeedbackList[i] << " ";
		*os << endl;
	}
}

void CALMNetwork::Print( ostream *os )
{
	for ( int i = 0; i < mNumInputModules+mNumModules; i++ ) *os << mModules[i];
}

// overload the << operator
ostream &operator<<( ostream &os, CALMNetwork *m )
{
	m->Print( &os );
	return os;
}

