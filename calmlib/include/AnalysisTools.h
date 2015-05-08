/*
Description:	Abstract class definition for analysis tools
Author:			Adriaan Tijsseling (AGT)
Copyright: 		(c) Copyright 2002-2015 Adriaan Tijsseling. All rights reserved.
Change History (most recent first):
	31/03/2002 - AGT - initial version
*/


#ifndef __ANALYSIS__
#define __ANALYSIS__

#include <string.h>
#include "CALM.h"		// the interface file to the CALM API Library
#include "PGMImage.h"


// color definition
struct rgb
{
	data_type r;
	data_type g;
	data_type b;
};


class AnalysisTools
{
public:

	AnalysisTools();
	~AnalysisTools();
	
	void	InitializeBoundaryMatrix( char const *mod, char const *inp, int xres, int yres, int iters );
	void	MatrixBoundaryForOnline( int run, data_type* pat, char const *mod, char const *inp, int xres, int yres, int iters );

	void	InitializeBoundary( char const *mod, char const *inp, int xres, int yres, int iters );
	void	BoundaryForOnline( int run, data_type* pat, char const *mod, char const *inp, int xres, int yres, int iters );
	void	BoundaryForOffline( int run, char const *mod, char const *inp, int xres, int yres, int iters,
										int patIdx, int x, int y );

	void	InitializeBifurcationClamp( int xres, int yres, int trans, 
										   int iters, data_type start, data_type end );
	void	InitializeBifurcation( char const *inp, int xres, int yres, int trans,
								   int iters, data_type start, data_type end, data_type par );
	void	BifurcationForOnlineClamp( int run, int outIdx, int unit, int xres, int yres,
									   int trans, int iters, data_type start, data_type end );
	void	BifurcationForOnline( int run, data_type* pat, char const *inp, int xres, int yres, int trans,
								  int iters, data_type start, data_type end, data_type par );
	void	BifurcationForOffline( int run, char const *inp, int xres, int yres, int trans, int iters,
								   data_type start, data_type end, data_type par, int patIdx, int x );
	void	InitializePhaseClamp( int xres, int trans, int iters, data_type start, 
								data_type step, data_type end );
	void	InitializePhase( char const *inp, int xres, int trans, int iters,
							 data_type start, data_type step, data_type end, data_type par );
	void	PhaseForOnlineClamp( int run, int outIdx, int unit, int xres, int trans, 
							int iters, data_type start, data_type step, data_type end );
	void	PhaseForOnline( int run, data_type* pat, char const *inp, int xres, int trans, int iters,
							data_type start, data_type step, data_type end, data_type par );
	void	PhaseForOffline( int run, char const *inp, int xres, int trans, int iters,
							 data_type start, data_type step, data_type end,
							 data_type par, int patIdx, int p );

protected:

	void	FillBoundary( int p, int q );
	void	FillBoundaryMatrix( int p, int q );
	void	FillBifurcation( int p, data_type** maxmins );
	void	FillBifurcationClamp( data_type** maxmins, int outIdx, int unit );
	bool	FillPhase( data_type x, int p, data_type rgbStep );
	bool	FillPhaseClamp( data_type x, data_type rgbStep, int outIdx, int unit );
	void	ResetPixels( data_type*** pixels );
	void	ResetPixels( data_type** pixels );
	void	WriteMatrixToFile( data_type*** pixels );
	void	WriteToFile( data_type*** pixels );
	void	WriteToFile( data_type** pixels );
	double	HuetoRGB( double m1, double m2, double h );
	void	HSLtoRGB( int i, int j, data_type hue );
	
	int				mPatIdx;		// index of input module to use
	int				mModIdx;		// index of CALM module to use
	data_type*		mInput;			// buffer for input pattern
	int				mInputLength;	// length of input pattern
	data_type**		mGrayPixels;	// storage for monochrome pixel image
	data_type***	mRGBPixels;		// storage for RGB pixels
	data_type**		mBifurcations;	// storage for bifurcation points
	data_type*		mPhases;		// storage for phaseplots
	int				mXRes;			// horizontal resolution
	int				mYRes;			// vertical resolution
	data_type		mXStep;			// horizontal iterator
	data_type		mYStep;			// vertical iterator
	data_type		mStart;			// for bifurcation
	data_type		mStep;
	data_type		mEnd;
	data_type		mPar;			// bifurcation or phase parameter: sets neighbour 
									// input value to mPar
	int				mTransients;	// iterations to skip for analysis
	int				mIterations;	// number of iterations to use for analysis
	PGMImage*		pgmImage;		// final image
	char			mDirName[256];
	char			mFileName[256];
	char			mSuffix[32];
};

#endif
