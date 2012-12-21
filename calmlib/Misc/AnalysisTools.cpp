/*
	Author:			Adriaan Tijsseling (AGT)
	Copyright: 		(c) Copyright 2002-2013 Adriaan Tijsseling. All rights reserved.
	Description:	Analysis tools for the CALM neural network. These routines explore the
					nonlinear dynamics behavior of CALM nets, producing images as output.
*/

#include "CALMGlobal.h"	// contains project wide definitions and the like
#include "Utilities.h"
#include "AnalysisTools.h"

// LOCAL GLOBALS
extern CALMAPI* gCALMAPI;		// struct contains the data to be passed to the API

rgb colors[10] = 
{ 
	{ 0.75, 0, 0 },
	{ 0, 0, 0.75 },
	{ 0, 0.75, 0 },
	{ 0.75, 0.75, 0 },
	{ 0, 0.75, 0 },
	{ 0.9, 0.4, 0.3 },
	{ 0.6, 0.7, 0.55 },
	{ 0.55, 0.31, 0.5 },
	{ 0, 0.7, 0.8 },
	{ 0.4, 0.44, 0.2 }
};


AnalysisTools::AnalysisTools()
{
	mInput = NULL;
	mGrayPixels = NULL;
	mRGBPixels = NULL;
	mBifurcations = NULL;
	mPhases = NULL;
	pgmImage = new PGMImage;
	mTransients = 0;
	mIterations = 1000;
}

AnalysisTools::~AnalysisTools()
{
	if ( mInput != NULL ) delete[] mInput;

	if ( mRGBPixels != NULL )
	{
		for ( int i = 0; i < 3; i++ )
			DisposeMatrix( mRGBPixels[i], mYRes );
		delete[] mRGBPixels;
	}

	if ( mGrayPixels != NULL ) DisposeMatrix( mGrayPixels, mYRes );
	if ( mBifurcations != NULL ) DisposeMatrix( mBifurcations, mIterations );

	if ( mPhases != NULL ) delete mPhases;
	if ( pgmImage != NULL ) delete pgmImage;
}


void AnalysisTools::InitializeBoundaryMatrix( char const *mod, char const *inp, int xres, int yres, int iters )
{
	mPatIdx = gCALMAPI->CALMGetModuleIndex( inp );
	mModIdx = gCALMAPI->CALMGetModuleIndex( mod );
	
	mXRes = xres;
	mYRes = yres;
	mXStep = 1.0 / (data_type)mXRes;
	mYStep = 1.0 / (data_type)mYRes;
	mIterations = iters;

	// set empty input pattern
	mInputLength = gCALMAPI->CALMGetModuleSize( mPatIdx );
	mInput = new data_type[mInputLength];
	
	// create directory for images
	mkdir( "convmaps", S_IRWXU | S_IRWXG );
	strcpy( mDirName, "convmaps/" );
		
	// allocate pixels for convergencemap
	mRGBPixels = new data_type**[3];
	for ( int i = 0; i < 3; i++ )
		mRGBPixels[i] = CreateMatrix( 1.0, mYRes * (mInputLength-1), mXRes * (mInputLength-1));
}

void AnalysisTools::FillBoundaryMatrix( int p, int q )
{
	data_type	x, y;
	int			i, j, epoch, ite;
	int			winner;

	for ( i = 0; i < mYRes; i++ ) 
	{
		y = i * mYStep;
		mInput[p] = y;

		for ( j = 0; j < mXRes; j++ ) 
		{
			x = j * mXStep;
			mInput[q] = x;

			gCALMAPI->CALMReset( O_TIME | O_ACT | O_WIN );

			gCALMAPI->CALMSetInput( mPatIdx, mInput );	// set custom input

			for ( epoch = 0; epoch < 10; epoch++ )
			{
				gCALMAPI->CALMReset( O_ACT | O_WIN ); // clean winners and activations
				for ( ite = 0; ite < mIterations; ite++ ) gCALMAPI->CALMTest( ite, false );
			}
			winner = gCALMAPI->CALMGetWinnerForModule( mModIdx );
			if ( winner != kNoWinner )
			{
				mRGBPixels[0][i+mYRes*p][j+mXRes*(q-1)] = colors[winner].r;
				mRGBPixels[1][i+mYRes*p][j+mXRes*(q-1)] = colors[winner].g;
				mRGBPixels[2][i+mYRes*p][j+mXRes*(q-1)] = colors[winner].b;
			}
		}
	}
}

void AnalysisTools::MatrixBoundaryForOnline( int run, data_type* pat, char const *mod, char const *inp, int xres, int yres, int iters )
{
	int	p, q, h;

	InitializeBoundaryMatrix( mod, inp, xres, yres, iters );
	
	for ( p = 0; p < mInputLength; p++ )
	{
		for ( q = p+1; q < mInputLength; q++ )
		{
			for ( h = 0; h < mInputLength; h++ )
			{
				mInput[h] = pat[h];
				if ( h == p ) cerr << "y ";
				else if ( h == q ) cerr << "x ";
				else cerr << mInput[h] << " ";
			}
			cerr << endl;
			
			FillBoundaryMatrix( p, q );
						
		}
	}
	
	sprintf( mSuffix, "img%d.ppm", run+1 );
	WriteMatrixToFile( mRGBPixels );
	if ( mRGBPixels != NULL )
	{
		for ( int i = 0; i < 3; i++ )
			DisposeMatrix( mRGBPixels[i], mYRes * (mInputLength-1) );
		delete[] mRGBPixels;
	}
	mRGBPixels = NULL;
}


void AnalysisTools::InitializeBoundary( char const *mod, char const *inp, int xres, int yres, int iters )
{
	mPatIdx = gCALMAPI->CALMGetModuleIndex( inp );
	mModIdx = gCALMAPI->CALMGetModuleIndex( mod );
	
	mXRes = xres;
	mYRes = yres;
	mXStep = 1.0 / (data_type)mXRes;
	mYStep = 1.0 / (data_type)mYRes;
	mIterations = iters;
	
	// create directory for images
	mkdir( "convmaps", S_IRWXU | S_IRWXG );
	strcpy( mDirName, "convmaps/" );
		
	// allocate pixels for convergencemap
	mRGBPixels = new data_type**[3];
	for ( int i = 0; i < 3; i++ ) mRGBPixels[i] = CreateMatrix( 0.0, mYRes, mXRes );

	// set empty input pattern
	mInputLength = gCALMAPI->CALMGetModuleSize( mPatIdx );
	mInput = new data_type[mInputLength];
}


void AnalysisTools::FillBoundary( int p, int q )
{
	data_type	x, y;
	int			i, j, ite;
	int			winner;

	for ( i = 0; i < mYRes; i++ ) 
	{
		y = i * mYStep;
		mInput[p] = y;

		for ( j = 0; j < mXRes; j++ ) 
		{
			x = j * mXStep;
			mInput[q] = x;

			gCALMAPI->CALMReset( O_ACT | O_WIN ); 	   // clean winners and activations
			gCALMAPI->CALMSetInput( mPatIdx, mInput ); // set custom input

			for ( ite = 0; ite < mIterations; ite++ )
			{
				gCALMAPI->CALMTest( ite, false );
				winner = gCALMAPI->CALMGetWinnerForModule( mModIdx );
				if ( winner != kNoWinner )
				{
					mRGBPixels[0][i][j] = colors[winner].r;
					mRGBPixels[1][i][j] = colors[winner].g;
					mRGBPixels[2][i][j] = colors[winner].b;
					break;
				}
			}
		}
	}
}


void AnalysisTools::BoundaryForOnline( int run, data_type* pat, char const *mod, char const *inp, int xres, int yres, int iters )
{
	int	p, q, h;

	InitializeBoundary( mod, inp, xres, yres, iters );
	
	for ( p = 0; p < mInputLength; p++ )
	{
		for ( q = p+1; q < mInputLength; q++ )
		{
			for ( h = 0; h < mInputLength; h++ )
			{
				mInput[h] = pat[h];
				if ( h == p ) cerr << "y ";
				else if ( h == q ) cerr << "x ";
				else cerr << mInput[h] << " ";
			}
			cerr << endl;
			
			FillBoundary( p, q );
						
			sprintf( mSuffix, "img%d_%d_%d.ppm", run+1, p+1, q+1 );
			WriteToFile( mRGBPixels );
		}
	}
}


void AnalysisTools::BoundaryForOffline( int run, char const *mod, char const *inp, int xres, int yres, int iters, int patIdx, int x, int y )
{
	int	h, i;

	InitializeBoundary( mod, inp, xres, yres, iters );

	// get loaded pattern
	for ( i = 0; i < mInputLength; i++ ) mInput[i] = gCALMAPI->CALMGetPattern( mPatIdx, patIdx, i );
	cerr << "pattern: " << patIdx+1 << endl;
	
	for ( h = 0; h < mInputLength; h++ )
	{
		mInput[h] = gCALMAPI->CALMGetPattern( mPatIdx, patIdx, h );
		if ( h == x ) cerr << "x ";
		else if ( h == y ) cerr << "y ";
		else cerr << mInput[h] << " ";
	}
	cerr << endl;
	
	FillBoundary( x, y );
	
	sprintf( mSuffix, "img%d_%d_%d_%d.ppm", run+1, patIdx+1, x+1, y+1 );
	WriteToFile( mRGBPixels );
}


void AnalysisTools::InitializeBifurcationClamp( int xres, int yres, int trans, 
										   int iters, data_type start, data_type end )
{
	mXRes = xres;
	mYRes = yres;
	mXStep = 1.0 / (data_type)mXRes;
	mYStep = 1.0 / (data_type)mYRes;
	mIterations = iters;
	mTransients = trans;
	mStart = start;
	mEnd = end;
	mStep = ( mEnd - mStart ) / (data_type)mXRes;
	
	// create directory for images
	mkdir( "bifs", S_IRWXU | S_IRWXG );
	strcpy( mDirName, "bifs/" );
		
	// allocate pixels for bifurcation plot
	mGrayPixels = CreateMatrix( 255.0, mYRes, mXRes );
	
	// allocate buffer for bifurcations
	mBifurcations = CreateMatrix( 1.0, mIterations, mXRes );
}


void AnalysisTools::FillBifurcationClamp( data_type** maxmins, int outIdx, int unit )
{
	int				bit;
	data_type		maxmin;
	int				i, j, ite;

	for ( j = 0; j < mXRes; j++ )
	{
		maxmins[0][j] = 0.0;
		maxmins[1][j] = INT_MAX;
	}

// set input to zeros
	for ( int k = 0; k < gCALMAPI->CALMGetInputLen(); k++ )
		gCALMAPI->CALMSetOnlineInput( k, 0.0 );

	for ( j = 0; j < mXRes; j++ ) 
	{
	// clean winners and activations
		gCALMAPI->CALMReset( O_ACT | O_WIN );

	// clamp the desired unit to the current value
		gCALMAPI->ClampUnit( outIdx, unit, j * mStep + mStart );

	// remove transients
		for ( ite = 0; ite < mTransients; ite++ ) gCALMAPI->CALMTest( ite, false );
	// collect summed acts
		for ( ite = 0; ite < mIterations; ite++ )
		{
			gCALMAPI->CALMTest( ite, false );
			mBifurcations[ite][j] = gCALMAPI->CALMSumActivation();
			if ( mBifurcations[ite][j] > maxmins[0][j] ) maxmins[0][j] = mBifurcations[ite][j];
			if ( mBifurcations[ite][j] < maxmins[1][j] ) maxmins[1][j] = mBifurcations[ite][j];
		}
	}
	
// find scale
	for ( j = 0; j < mXRes; j++ )
	{
		maxmin = maxmins[0][j]-maxmins[1][j];
		if ( maxmin == 0.00000000 ) maxmin = 1.0;
	
		for ( i = 0; i < mIterations; i++ )
		{
			bit = (int)( mYRes * ( (mBifurcations[i][j] - maxmins[1][j] ) / maxmin ) );
			if ( bit > ( mYRes - 1 ) ) bit = mYRes - 1;
			mGrayPixels[bit][j] = 0.0;
			mBifurcations[i][j] = 1.0; 	// reset
		}
	}
}


void AnalysisTools::BifurcationForOnlineClamp( int run, int outIdx, int unit, int xres, int yres,
											   int trans, int iters, data_type start, data_type end )
{
	data_type** maxmins;

	InitializeBifurcationClamp( xres, yres, trans, iters, start, end );
	 
	// allocate array for max and min
	maxmins = CreateMatrix( 0.0, 2, xres );
	for ( int j = 0; j < mXRes; j++ ) maxmins[1][j] = INT_MAX;
	
	FillBifurcationClamp( maxmins, outIdx, unit );
		
	sprintf( mSuffix, "img%d.ppm", run+1 );
	WriteToFile( mGrayPixels );

	DisposeMatrix( maxmins, 2 );

	gCALMAPI->ClampUnit( outIdx, unit );
}


void AnalysisTools::InitializeBifurcation( char const *inp, int xres, int yres, int trans,
										   int iters, data_type start, data_type end, data_type par )
{
	mPatIdx = gCALMAPI->CALMGetModuleIndex( inp );
	
	mXRes = xres;
	mYRes = yres;
	mXStep = 1.0 / (data_type)mXRes;
	mYStep = 1.0 / (data_type)mYRes;
	mIterations = iters;
	mTransients = trans;
	mStart = start;
	mEnd = end;
	mStep = ( mEnd - mStart ) / (data_type)mXRes;
	mPar = par;
	
	// create directory for images
	mkdir( "bifs", S_IRWXU | S_IRWXG );
	strcpy( mDirName, "bifs/" );
		
	// allocate pixels for bifurcation plot
	mGrayPixels = CreateMatrix( 255.0, mYRes, mXRes );
	
	// allocate buffer for bifurcations
	mBifurcations = CreateMatrix( 1.0, mIterations, mXRes );
		
	// set empty input pattern
	mInputLength = gCALMAPI->CALMGetModuleSize( mPatIdx );
	mInput = new data_type[mInputLength];
}


void AnalysisTools::FillBifurcation( int p, data_type** maxmins )
{
	int				bit;
	data_type		maxmin;
	int				i, j, ite;

	for ( j = 0; j < mXRes; j++ )
	{
		maxmins[0][j] = 0.0;
		maxmins[1][j] = INT_MAX;
	}
	for ( j = 0; j < mXRes; j++ ) 
	{
		mInput[p] = j * mStep + mStart;
		gCALMAPI->CALMSetInput( mPatIdx, mInput );	// set custom input
		gCALMAPI->CALMReset( O_ACT | O_WIN );			// clean winners and activations
		// remove transients
		for ( ite = 0; ite < mTransients; ite++ ) gCALMAPI->CALMTest( ite, false );
		// collect summed acts
		for ( ite = 0; ite < mIterations; ite++ )
		{
			gCALMAPI->CALMTest( ite, false );
			mBifurcations[ite][j] = gCALMAPI->CALMSumActivation();
			if ( mBifurcations[ite][j] > maxmins[0][j] ) maxmins[0][j] = mBifurcations[ite][j];
			if ( mBifurcations[ite][j] < maxmins[1][j] ) maxmins[1][j] = mBifurcations[ite][j];
		}
	}

	// find scale
	for ( j = 0; j < mXRes; j++ )
	{
		maxmin = maxmins[0][j]-maxmins[1][j];
		if ( maxmin == 0.00000000 ) maxmin = 1.0;
	
		for ( i = 0; i < mIterations; i++ )
		{
			bit = (int)( mYRes * ( (mBifurcations[i][j] - maxmins[1][j] ) / maxmin ) );
			if ( bit > ( mYRes - 1 ) ) bit = mYRes - 1;
			mGrayPixels[bit][j] = 0.0;
			mBifurcations[i][j] = 1.0; 	// reset
		}
	}
}


void AnalysisTools::BifurcationForOnline( int run, data_type* pat, char const* inp, int xres, int yres, int trans,
										  int iters, data_type start, data_type end, data_type par )
{
	data_type**		maxmins;
	int				p, h, j;

	InitializeBifurcation( inp, xres, yres, trans, iters, start, end, par );
	 
	// allocate array for max and min
	maxmins = CreateMatrix( 0.0, 2, xres );
	for ( j = 0; j < mXRes; j++ ) maxmins[1][j] = INT_MAX;
	
	for ( p = 0; p < mInputLength; p++ )
	{
		for ( h = 0; h < mInputLength; h++ ) mInput[h] = pat[h];
		if ( p == mInputLength-1 )
			mInput[0] = mPar;
		else
			mInput[p+1] = mPar;
		for ( h = 0; h < mInputLength; h++ )
		{
			if ( h == p ) cerr << "x ";
			else cerr << mInput[h] << " ";
		}
		cerr << endl;

		FillBifurcation( p, maxmins );
		
		sprintf( mSuffix, "img%d_%d.ppm", run+1, p+1 );
		WriteToFile( mGrayPixels );
	}

	DisposeMatrix( maxmins, 2 );
}


void AnalysisTools::BifurcationForOffline( int run, char const *inp, int xres, int yres, int trans, int iters,
										   data_type start, data_type end, data_type par, int patIdx, int x )
{
	data_type **maxmins;
	int		  h, j;

	InitializeBifurcation( inp, xres, yres, trans, iters, start, end, par );

	// allocate array for max and min
	maxmins = CreateMatrix( 0.0, 2, xres );
	for ( j = 0; j < mXRes; j++ ) maxmins[1][j] = INT_MAX;

	cerr << "pattern: " << patIdx+1 << endl;

	for ( h = 0; h < mInputLength; h++ ) 
		mInput[h] = gCALMAPI->CALMGetPattern( mPatIdx, patIdx, h );
	
	if ( x == mInputLength-1 )
		mInput[0] = mPar;
	else
		mInput[x+1] = mPar;
	
	for ( h = 0; h < mInputLength; h++ )
	{
		if ( h == x ) cerr << "x ";
		else cerr << mInput[h] << " ";
	}
	cerr << endl;
	
	FillBifurcation( x, maxmins );
	
	sprintf( mSuffix, "img%d_%d_%d.ppm", run+1, patIdx+1, x+1 );
	WriteToFile( mGrayPixels );			

	DisposeMatrix( maxmins, 2 );
}


void AnalysisTools::InitializePhaseClamp( int xres, int trans, int iters, data_type start, data_type step, data_type end )
{
	mXRes = xres;
	mYRes = xres;
	mXStep = 1.0 / (data_type)mXRes;
	mYStep = 1.0 / (data_type)mYRes;
	mIterations = iters;
	mTransients = trans;
	mStart = start;
	mEnd = end;
	mStep = step;
	
	// create directory for images
	mkdir( "phases", S_IRWXU | S_IRWXG );
	strcpy( mDirName, "phases/" );
		
	// allocate pixels for bifurcation plot
	mRGBPixels = new data_type**[3];
	for ( int i = 0; i < 3; i++ )
		mRGBPixels[i] = CreateMatrix( 0.0, mYRes, mXRes );
	
	// allocate buffer for bifurcations
	mPhases = new data_type[mIterations];
}


bool AnalysisTools::FillPhaseClamp( data_type x, data_type rgbStep, int outIdx, int unit )
{
	data_type	maxi = 0, mini = INT_MAX, maxmin;
	int			i, ite;
	int			bit1, bit2;

	maxi = 0;
	mini = INT_MAX;

	gCALMAPI->CALMReset( O_TIME );
// set input to zeros
	for ( i = 0; i < gCALMAPI->CALMGetInputLen(); i++ ) gCALMAPI->CALMSetOnlineInput( i, 0.0 );
// clamp the desired unit
	gCALMAPI->ClampUnit( outIdx, unit, x );

	gCALMAPI->CALMReset( O_ACT | O_WIN );		// clean winners and activations
	// remove transients
	for ( ite = 0; ite < mTransients; ite++ ) gCALMAPI->CALMTest( ite, false );
	// collect summed acts
	for ( ite = 0; ite < mIterations; ite++ )
	{
		gCALMAPI->CALMTest( ite, false );
		mPhases[ite] = gCALMAPI->CALMSumActivation();
		if ( mPhases[ite] > maxi ) maxi = mPhases[ite];
		if ( mPhases[ite] < mini ) mini = mPhases[ite];
	}

	// find scale
	maxmin = maxi - mini;
	if ( maxmin == 0.00000000 ) return false;

	for ( i = 0; i < mIterations-1; i++ )
	{
		bit1 = (int)( mXRes * ( (mPhases[i] - mini ) / maxmin ) );
		if ( bit1 > ( mXRes - 1 ) ) bit1 = mXRes - 1;
		
		bit2 = (int)( mXRes * ( (mPhases[i+1] - mini ) / maxmin ) );
		if ( bit2 > ( mXRes - 1 ) ) bit2 = mXRes - 1;
		
		HSLtoRGB( bit1, bit2, i*rgbStep );
//		mRGBPixels[0][bit1][bit2] = i * rgbStep;
//		mRGBPixels[1][bit1][bit2] = 0.0;
//		mRGBPixels[2][bit1][bit2] = 0.0;
		mPhases[i] = 1.0; 	// reset
	}
	return true;
}


void AnalysisTools::PhaseForOnlineClamp( int run, int outIdx, int unit, int xres, int trans, int iters, 
									data_type start, data_type step, data_type end )
{
	data_type x, rgbStep;

	InitializePhaseClamp( xres, trans, iters, start, step, end );
	// color iterator
	rgbStep = 1.0 / (data_type)mIterations;

	for ( x = mStart; x < mEnd; x = x + mStep ) 
	{
		if ( FillPhaseClamp( x, rgbStep, outIdx, unit ) )
		{
			sprintf( mSuffix, "img%d_%f.ppm", run+1, x );
			WriteToFile( mRGBPixels );
		}
	}
}


void AnalysisTools::InitializePhase( char const *inp, int xres, int trans, int iters,
									 data_type start, data_type step, data_type end, data_type par )
{
	mPatIdx = gCALMAPI->CALMGetModuleIndex( inp );
	
	mXRes = xres;
	mYRes = xres;
	mXStep = 1.0 / (data_type)mXRes;
	mYStep = 1.0 / (data_type)mYRes;
	mIterations = iters;
	mTransients = trans;
	mStart = start;
	mEnd = end;
	mStep = step;
	mPar = par;
	
	// create directory for images
	mkdir( "phases", S_IRWXU | S_IRWXG );
	strcpy( mDirName, "phases/" );
		
	// allocate pixels for bifurcation plot
	mRGBPixels = new data_type**[3];
	for ( int i = 0; i < 3; i++ )
		mRGBPixels[i] = CreateMatrix( 0.0, mYRes, mXRes );
	
	// allocate buffer for bifurcations
	mPhases = new data_type[mIterations];
		
	// set empty input pattern
	mInputLength = gCALMAPI->CALMGetModuleSize( mPatIdx );
	mInput = new data_type[mInputLength];
}


bool AnalysisTools::FillPhase( data_type x, int p, data_type rgbStep )
{
	data_type	maxi = 0, mini = INT_MAX, maxmin;
	int			i, ite;
	int			bit1, bit2;
	
	maxi = 0;
	mini = INT_MAX;
	mInput[p] = x;
	gCALMAPI->CALMSetInput( mPatIdx, mInput );	// set custom input
	gCALMAPI->CALMReset( O_ACT | O_WIN );		// clean winners and activations
	// remove transients
	for ( ite = 0; ite < mTransients; ite++ ) gCALMAPI->CALMTest( ite, false );
	// collect summed acts
	for ( ite = 0; ite < mIterations; ite++ )
	{
		gCALMAPI->CALMTest( ite, false );
	/*
		// plot R versus V
		if ( ite % 2 == 0 )
			mPhases[ite] = gCALMAPI->CALMSumActivationR();
		else
			mPhases[ite] = gCALMAPI->CALMSumActivationV();
	*/
		mPhases[ite] = gCALMAPI->CALMSumActivationR();
		if ( mPhases[ite] > maxi ) maxi = mPhases[ite];
		if ( mPhases[ite] < mini ) mini = mPhases[ite];
	}

	// find scale
	maxmin = maxi - mini;
	if ( maxmin == 0.00000000 ) return false;

	for ( i = 0; i < mIterations-1; i++ )
	{
		bit1 = (int)( mXRes * ( (mPhases[i] - mini ) / maxmin ) );
		if ( bit1 > ( mXRes - 1 ) ) bit1 = mXRes - 1;
		
		bit2 = (int)( mXRes * ( (mPhases[i+1] - mini ) / maxmin ) );
		if ( bit2 > ( mXRes - 1 ) ) bit2 = mXRes - 1;
		
		HSLtoRGB( bit1, bit2, i*rgbStep );
//		mRGBPixels[0][bit1][bit2] = i * rgbStep;
//		mRGBPixels[1][bit1][bit2] = 0.0;
//		mRGBPixels[2][bit1][bit2] = 0.0;
		mPhases[i] = 1.0; 	// reset
	}
	
	return true;
}


void AnalysisTools::PhaseForOnline( int run, data_type* pat, char const *inp, int xres, int trans, int iters,
									data_type start, data_type step, data_type end, data_type par )
{
	data_type	x, rgbStep;
	int			p, h;

	InitializePhase( inp, xres, trans, iters, start, step, end, par );
	// color iterator
	rgbStep = 1.0 / (data_type)mIterations;

	for ( p = 0; p < mInputLength; p++ )
	{
		for ( h = 0; h < mInputLength; h++ ) mInput[h] = pat[h];
		
		if ( p == mInputLength-1 )
			mInput[0] = mPar;
		else
			mInput[p+1] = mPar;
			
		for ( h = 0; h < mInputLength; h++ )
		{
			if ( h == p ) cerr << "x ";
			else cerr << mInput[h] << " ";
		}
		cerr << endl;
			
		for ( x = mStart; x < mEnd; x = x + mStep ) 
		{
			if ( FillPhase( x, p, rgbStep ) )
			{
				sprintf( mSuffix, "img%d_%d_%f.ppm", run+1, p+1, x );
				WriteToFile( mRGBPixels );
			}
		}
	}
}


void AnalysisTools::PhaseForOffline( int run, char const *inp, int xres, int trans, int iters,
									data_type start, data_type step, data_type end,
									data_type par, int patIdx, int p )
{
	data_type	x, rgbStep;
	int			h, i;

	InitializePhase( inp, xres, trans, iters, start, step, end, par );
	// color iterator
	rgbStep = 1.0 / (data_type)mIterations;

	// obtain learned pattern (first one only)
	for ( i = 0; i < mInputLength; i++ ) mInput[i] = gCALMAPI->CALMGetPattern( mPatIdx, 0, i );

	cerr << "pattern: " << patIdx+1 << endl;

	for ( h = 0; h < mInputLength; h++ ) 
		mInput[h] = gCALMAPI->CALMGetPattern( mPatIdx, patIdx, h );
	if ( p == mInputLength-1 )
		mInput[0] = mPar;
	else
		mInput[p+1] = mPar;
	for ( h = 0; h < mInputLength; h++ )
	{
		if ( h == p ) cerr << "x ";
		else cerr << mInput[h] << " ";
	}
	cerr << endl;

	for ( x = mStart; x < mEnd; x = x + mStep ) 
	{
		if ( FillPhase( x, p, rgbStep ) )
		{
			sprintf( mSuffix, "img%d_%d_%d_%f.ppm", run+1, patIdx+1, p+1, x );
			WriteToFile( mRGBPixels );
		}
	}
}

void AnalysisTools::ResetPixels( data_type*** pixels )
{
	for ( int i = 0; i < mYRes; i++ )
	{
		for ( int j = 0; j < mXRes; j++ )
		{
			pixels[0][i][j] = 0.0;
			pixels[1][i][j] = 0.0;
			pixels[2][i][j] = 0.0;
		}
	}
}

void AnalysisTools::ResetPixels( data_type** pixels )
{
	for ( int i = 0; i < mYRes; i++ )
		for ( int j = 0; j < mXRes; j++ )
			pixels[i][j] = 255.0;
}

void AnalysisTools::WriteMatrixToFile( data_type*** pixels )
{
	strcpy( mFileName, mDirName );
	strcat( mFileName, mSuffix );
	pgmImage->Write( mFileName, pixels, mYRes * (mInputLength-1), mXRes * (mInputLength-1) );
	ResetPixels( pixels );
}

void AnalysisTools::WriteToFile( data_type*** pixels )
{
	strcpy( mFileName, mDirName );
	strcat( mFileName, mSuffix );
	pgmImage->Write( mFileName, pixels, mYRes, mXRes );
	ResetPixels( pixels );
}

void AnalysisTools::WriteToFile( data_type** pixels )
{
	strcpy( mFileName, mDirName );
	strcat( mFileName, mSuffix );
	pgmImage->Write( mFileName, pixels, mYRes, mXRes );
	ResetPixels( pixels );
}


double AnalysisTools::HuetoRGB( double m1, double m2, double h )
{
   if( h < 0 ) h += 1.0;
   if( h > 1 ) h -= 1.0;
   if( 6.0*h < 1 )
      return (m1+(m2-m1)*h*6.0);
   if( 2.0*h < 1 )
      return m2;
   if( 3.0*h < 2.0 )
      return (m1+(m2-m1)*((2.0/3.0)-h)*6.0);
   return m1;
}

void AnalysisTools::HSLtoRGB( int i, int j, data_type hue )
{
//	data_type	light = 0.45, saturation = 0.75;
	data_type	light = 0.45, saturation = 1.0;
	data_type	m1, m2;

	if( light <= 0.5 )
		m2 = light * ( 1.0 + saturation );
	else
		m2 = light + saturation - light * saturation;
	m1 = 2.0 * light - m2;
	mRGBPixels[0][i][j] = HuetoRGB( m1, m2, hue + 1.0 / 3.0 );
	mRGBPixels[1][i][j] = HuetoRGB( m1, m2, hue );
	mRGBPixels[2][i][j] = HuetoRGB( m1, m2, hue - 1.0 / 3.0 );
}


