/*
	Author:			Adriaan Tijsseling (AGT)
	Copyright: 		(c) Copyright 2002-2013 Adriaan Tijsseling. All rights reserved.
	Description:	Generic utilities
*/

#include	<unistd.h>
#include	<stdlib.h>
#include	"CALMGlobal.h"
#include	"Utilities.h"

long	gPrecision;
long	gWidth;

// for Permute
struct	tmp
{
	int p;		/* permutation		*/
	int r;		/* random number	*/
};


// Permutes an array
void Permute( int* array, size_t size )
{
	struct tmp 	*t;
	size_t 		i;
	
	t = new tmp[size];
	for( i = 0; i < size; i++ ) 	// load up struct with data
	{
		t[i].r = rand();
		t[i].p = array[i];
	}	
	qsort( t, size, sizeof(struct tmp), cmp );	// shuffle
	
	// data back to original array
	for( i = 0; i < size; i++ ) array[i] = t[i].p;
	
	delete[] t;
}

// sort an array and return number of unique items
int Unique( ostream* os, int* array, size_t size, int msize, bool *sorted )
{
	struct tmp 	*t;
	int 		last, counter;
	size_t		i;
	int 		dist, maxi = -1, mini = 10000;
	bool		ismap = *sorted;
	
	// first check if correct ordering (for CALMMap)
	if ( ismap )
	{
		// find the smallest and largest node index
		maxi = -1;
		mini = 10000;
		for ( i = 0; i < size; i++ )
		{
			if ( array[i] > maxi ) maxi = array[i];
			if ( array[i] < mini ) mini = array[i];
		}
		// this is really crude, but we just compare the direction
		// of node change for each subsequent adjacent pair of indices.
		// if the direction changes, then no linear ordering
		int sign1 = 0, sign2;
		for ( i = 0; i < size-1; i++ )
		{
			// ring topology, so end of module may loop to beginning
			if ( ( array[i] == maxi && array[i+1] == mini ) ||
				 ( array[i] == mini && array[i+1] == maxi ) )
				continue;
			if ( (array[i+1]-array[i]) < 0 ) 
				sign2 = -1;
			else if ( (array[i+1]-array[i]) > 0 ) 
				sign2 = 1;
			else
				continue;
			if ( sign1 == 0 )
				sign1 = sign2;
			else if ( sign1 != sign2 )
			{
				*sorted = false;
				break;
			}
		}		
	}
	
	// here we sort then count unique indices. That determines
	// the number of nodes that are used to represent inputs
	// we also check the spacing between representations to calculate
	// the average spread of representations (calmmap only)
	t = new tmp[size];
	for( i = 0; i < size; i++ ) 	// load up struct with data
	{
		t[i].r = array[i];
		t[i].p = i;
	}	
	qsort( t, size, sizeof(struct tmp), cmp );	// sort it

	last = t[0].r;
	counter = 1;
	maxi = 0;
	for ( i = 1; i < size; i++ )
	{
		if ( t[i].r != last )
		{
			counter++;
			last = t[i].r;
		}
		if ( ismap )
		{
			dist = SafeAbs(t[i].r - t[i-1].r);
			*os << ' ' << dist;
			if ( dist > maxi ) maxi = dist;
		}
	}
	if ( ismap )
	{
		dist = (( msize - t[size-1].r ) + t[0].r );
		if ( dist > maxi ) maxi = dist;
		*os << ' ' << dist << ": " << maxi;	
	}
	delete[] t;
	return counter;
}

// use for permuted qsort
int cmp( const void *s1, const void *s2 )
{
	struct tmp *a1 = (struct tmp *)s1;
	struct tmp *a2 = (struct tmp *)s2;
	
	return((a1->r) - (a2->r));
}


data_type Heavyside( data_type a ) 
{
	// if a is larger than 0.5, return 1.0, else return 0.0
	return( ( a > 0.5 ) ? 1.0 : 0.0 );
}


data_type Sigmoid( data_type act )
{
	return ( 1.0 / ( 1.0 + exp( -1.0 * act ) ) );
}


// beta must be negative
// untested
data_type Sigmoid( data_type beta, data_type a_pot ) 
{
	return ( 1.0 / ( 1.0 + exp( beta * a_pot ) ) );
}

data_type Sigmoid( data_type beta, data_type a_pot, data_type thresh ) 
{
	return ( 1.0 / ( 1.0 + exp( beta * a_pot + thresh ) ) );
}


// Create a matrix and fill it with constant given in parameter
data_type **CreateMatrix( data_type val, int row, int col ) 
{
	data_type **matrix = new data_type*[row];
	
	for ( int i = 0; i < row; i++ ) 
	{
		matrix[i] = new data_type[col];
		
		for ( int j = 0; j < col; j++ )
			matrix[i][j] = val;
	}
	return matrix;
}

// Reset a matrix with new value val
void ResetMatrix( data_type ** matrix, data_type val, int row, int col ) 
{
	for ( int i = 0; i < row; i++ )
	{
		for ( int j = 0; j < col; j++ )
		{
			matrix[i][j] = val;
		}
	}
}

// Dispose allocated matrix
void DisposeMatrix( data_type** matrix, int row )
{
	for ( int i = 0; i < row; i++ ) delete[] matrix[i];
	delete[] matrix;
}


// Returns Euclidean distance between two vectors
data_type ReturnDistance( data_type *pat1, data_type *pat2, int size ) 
{
	data_type dist = 0.0;
	
	for ( int i = 0; i < size; i++ )
		dist += ( pat1[i] - pat2[i] ) * ( pat1[i] - pat2[i] );
	
	return (data_type)( sqrt( dist ) / sqrt( (data_type)size ) );
}

// For file reading purposes. Skips blanks and lines starting with #
void SkipComments( ifstream* infile )
{
	bool	garbage = true;
	char	c;
	
	while ( garbage )
	{
		// ignore any line feeds left in the stream
		while ( infile->peek() == '\n' || infile->peek() == ' ' || infile->peek() == '\t' ) 
			infile->get();	
		while ( infile->peek() == '#' )infile->ignore( 1000, '\n' );
		infile->get(c);
		if ( c == '\n' || c == '\t' || c == ' ' || c == '#' )
			garbage = true;
		else
			garbage = false;
		infile->putback(c);
	}
}


void FileCreateError( char* filename )
{
	char folder[FILENAME_MAX];
	
	getcwd( folder, FILENAME_MAX );		
	cerr << "Error: Could not create file " << filename << " in directory ";
	cerr << folder << endl;
}

void FileOpenError( char* filename )
{
	char folder[FILENAME_MAX];
	
	getcwd( folder, FILENAME_MAX );		
	cerr << "Error: Could not open file " << filename << " in directory ";
	cerr << folder << endl;
}


// cout, cerr and ostream formatting utilities

void GetStreamDefaults( void )
{
	gWidth = cout.width();
	gPrecision = cout.precision();
}

void AdjustStream( ostream &os, int precision, int width, int pos, bool trailers )
{
	os.precision( precision );
	os.width( width );
	os.fill( ' ' );
	if ( trailers )
		os.setf( ios::showpoint,  ios::showpoint );
	else
		os.unsetf( ios::showpoint );
	if ( pos == kLeft )
		os.setf( ios::left, ios::adjustfield );
	else
		os.setf( ios::right, ios::adjustfield );
}

void SetStreamDefaults( ostream &os )
{
	os.precision( gPrecision );
	os.width( gWidth );
	os.unsetf( ios::showpoint );
	os.setf( ios::left, ios::adjustfield );
}


void PrintNext( ostream* os, int separator )
{
	switch ( separator )
	{
		case kTab:
			*os << '\t';
			break;
		case kReturn:
			*os << '\n';
			break;
		case kIntend:
			*os << "      ";
			break;
		case kSpace:
			*os << " ";
			break;
	}
}


void PrintRoundedValue( ostream* os, data_type val )
{
	if ( val > 0.1 )
		*os << val;
	else if ( val >= 0.05 && val <= 0.1 )
		*os << 0.1;
	else
		*os << 0.0;
}


double SafeAbs( double val1, double val2 )
{
	double diff = val1 - val2;
	
	if ( diff < 0.0 ) return ( 0.0 - diff );
	else return diff;
}

float SafeAbs( float val1, float val2 )
{
	float diff = val1 - val2;
	
	if ( diff < 0.0 ) return ( 0.0 - diff );
	else return diff;
}

int SafeAbs( int val1, int val2 )
{
	int diff = val1 - val2;
	
	if ( diff < 0 ) return ( 0 - diff );
	else return diff;
}

double SafeAbs( double val )
{
	if ( val < 0.0 ) return ( 0.0 - val );
	else return val;
}

float SafeAbs( float val )
{
	if ( val < 0.0 ) return ( 0.0 - val );
	else return val;
}

int SafeAbs( int val )
{
	if ( val < 0 ) return ( 0 - val );
	else return val;
}

