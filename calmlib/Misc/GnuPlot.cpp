/*
	Original Author:	Max Lungarella
	Modifications by:	Adriaan Tijsseling (AGT)
	Copyright: 			(c) Copyright 2003 MECHwA Team. All rights reserved.
	Description:		online plotting routines using GNUPlot. Supports 3D plots.
						NOTE: the GnuPlot program path and name must be adapted to your system!
*/

#include "GnuPlot.h"
#include	<iostream>

#define USE_AQUA		1	// plot using AquaTerm if on Mac, if desired.

using namespace std;

extern double SafeAbs( double val );

//  for 2d plots: displays dynamically updated graph
GnuPlot::GnuPlot( int xr1, int xr2, int yr1, int yr2 )
{
	mCounter = 0;
	mBounds = xr2;
	mMax = -1000000; 
	mMin =  1000000;
	
// store a range of graph values
	mBuffer = new double[mBounds];
// not going to use a 3d plot
	mMatrix = NULL;
// open the pipe to the gnuplot program
	mGnuPipe = popen( "/usr/local/bin/gnuplot --persist", "w" );
#if USE_AQUA
	fprintf(mGnuPipe, "set term aqua\n");
#endif
// indicate the initial range for the plot
	fprintf( mGnuPipe, "set xrange [%d:%d]\n", xr1, xr2 ); 
	fprintf( mGnuPipe, "set yrange [%d:%d]\n", yr1, yr2 );
}


// for 3d matrix plots
GnuPlot::GnuPlot( int r, int c )
{
	mRows = r;
	mCols = c;
// not going to use the 2d plot
	mBuffer = NULL;
// allocate memory for matrix
	mMatrix = new double*[ mRows ];
	for ( int i = 0; i < mRows; i++ ) mMatrix[i] = new double[ mCols ];
// open the pipe to the gnuplot program
	mGnuPipe = popen( "/usr/local/bin/gnuplot --persist", "w" );
#if USE_AQUA
	fprintf(mGnuPipe, "set term aqua\n");
#endif
// set options for 3d plot
	fprintf( mGnuPipe, "set hidden3d\n" );	
	fprintf( mGnuPipe, "set zrange [0:1]\n" );
}


GnuPlot::~GnuPlot( )
{
	if ( mBuffer != NULL ) delete[] mBuffer;
	if ( mMatrix != NULL )
	{
		for ( int i = 0; i < mRows; i++ ) delete[] mMatrix[i];
		delete[] mMatrix;
	}
	pclose( mGnuPipe );
}


// only for 3d plot. Use it if the dimensions of the plot should be changed
void GnuPlot::resizePlot( int r, int c )
{
	if ( mMatrix != NULL )
	{
		for ( int i = 0; i < mRows; i++ ) delete[] mMatrix[i];
		delete[] mMatrix;
	}
	mRows = r;
	mCols = c;
	mMatrix = new double*[ mRows ];
	for ( int i = 0; i < mRows; i++ ) mMatrix[i] = new double[ mCols ];
	fprintf( mGnuPipe, "reset\n" );	
	fprintf( mGnuPipe, "set hidden3d\n" );	
	fprintf( mGnuPipe, "set zrange [0:1]\n" );
}


// plot a single value in a 2d plot
void GnuPlot::plot( double val ) 
{
	int i;
	
// adjust vertical mBounds
	if ( mMax < val ) mMax = val;
	if ( mMin > val ) mMin = val;

// pipe the new range to gnuplot, leaving space for top and bottom	
	fprintf( mGnuPipe, "set yrange [%f:%f]\n", mMin-SafeAbs(mMin/10.0), mMax+SafeAbs(mMax/10.0) );

// add the new value to the end of the buffer
	mBuffer[mCounter] = val;

	fprintf( mGnuPipe, "plot '-' with lines\n" );
	for ( i = 0; i < mCounter+1; i++ ) fprintf( mGnuPipe, "%f\n", mBuffer[i] );
	fprintf( mGnuPipe, "e\n" );
	
	if ( mCounter == mBounds-1 )
	{
	// move the array down and create space for the next plot value
		double oldval = mBuffer[0];
		for ( i = 0; i < mCounter; i++ ) mBuffer[i] = mBuffer[i+1];
	// if necessary, find the new minimum and maximum value
		if ( oldval == mMin )
		{
			mMin = mBuffer[0];
			for ( i = 1; i < mCounter; i++ ) if ( mMin > mBuffer[i] ) mMin = mBuffer[i];
		}
		if ( oldval == mMax )
		{
			mMax = mBuffer[0];
			for ( i = 1; i < mCounter; i++ ) if ( mMax < mBuffer[i] ) mMax = mBuffer[i];
		}
	}
	else
		mCounter++;
// make sure the plot is updated
	fflush( mGnuPipe );
}


// plot a whole vector of values into a 2d graph
void GnuPlot::plot( double *buf, int bufSize ) 
{
	fprintf( mGnuPipe, "plot '-' with lines\n" );	
	for ( int i = 0; i < bufSize; i++ ) fprintf( mGnuPipe, "%f\n", buf[i] );
	fprintf( mGnuPipe, "e\n" );
	fflush( mGnuPipe );
}


// plot two value arrays into a 2d graph
void GnuPlot::plot( double *buf1, double *buf2, int bufSize )
{
	fprintf( mGnuPipe, "plot '-' with lines, '-' with lines\n" );	

	for ( int i = 0; i < bufSize; i++ ) fprintf( mGnuPipe, "%f\n", buf1[i] );
	fprintf( mGnuPipe, "e\n" );

	for ( int i = 0; i < bufSize; i++ ) fprintf( mGnuPipe, "%f\n", buf2[i] );
	fprintf( mGnuPipe, "e\n" );

	fflush( mGnuPipe );
}


// plot a 3D display of matrix values
void GnuPlot::plot( void ) 
{
	fprintf( mGnuPipe, "splot '-' matrix notitle with lines\n" );
	for ( int i = 0; i < mRows; i++ )
	{
		for ( int j = 0; j < mCols; j++ ) {
			fprintf( mGnuPipe, "%f ", mMatrix[i][j] );
		}
		fprintf( mGnuPipe, "\n" );
	}
	fprintf( mGnuPipe, "e\n" );
	fprintf( mGnuPipe, "e\n" );
	fflush( mGnuPipe );
}


