/*
	Original Author:	Max Lungarella
	Modifications by:	Adriaan Tijsseling (AGT)
	Copyright: 			(c) Copyright 2003 MECHwA Team. All rights reserved.
	Description:		online plotting routines using GNUPlot. Supports 3D plots.
*/

#ifndef _GNUPLOT_H_
#define _GNUPLOT_H

#include <cstdio>
#include <cstring>
#include <cstdlib>

class GnuPlot 
{

public:
	// Constructor and destructor
	GnuPlot() {}   
	GnuPlot( int, int, int, int );
	GnuPlot( int, int );
	~GnuPlot();
	
	// Functions
	void resizePlot( int r, int c );

	void plot( double val );
	void plot( double*, int );
	void plot( double*, double*, int );
	void plot( void );

	inline double**	GetMatrix( void ) { return mMatrix; }
	
private:
	int			mBounds;
	int			mCounter;
	FILE*		mGnuPipe;
	double*		mBuffer;
	double**	mMatrix;
	double		mMax;
	double		mMin;
	int			mRows;
	int			mCols;

};

#endif

