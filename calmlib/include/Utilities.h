/*
	Author:			Adriaan Tijsseling (AGT)
	Copyright: 		(c) Copyright 2002-2011 Adriaan Tijsseling. All rights reserved.
	Description:	API interface for Sequence Learning Network
*/

#ifndef __UTILITIES__
#define __UTILITIES__

#include	<fstream>
#include	<iostream>
using namespace std;
#include	<stdio.h>
#include	<math.h>

enum
{
	kLeft = 0,
	kRight
};

void		Permute( int* array, size_t size );
int 		Unique( ostream* os, int* array, size_t size, int msize, bool *sorted );
int			cmp(const void *s1, const void *s2); 	// for qsort() function
data_type	Heavyside( data_type a );
data_type	Sigmoid( data_type act );
data_type	Sigmoid( data_type beta, data_type a_pot );
data_type	Sigmoid( data_type beta, data_type a_pot, data_type thresh );
data_type	**CreateMatrix( data_type val, int row, int col );
void		ResetMatrix( data_type ** matrix, data_type val, int row, int col );
void		DisposeMatrix( data_type** matrix, int row );
data_type 	ReturnDistance( data_type *pat1, data_type *pat2, int size );
void		GetStreamDefaults( void );
void 		AdjustStream( ostream &os, int precision, int width, int pos, bool trailers );
void		SetStreamDefaults( ostream &os );
void		PrintNext( ostream* os, int separator );
void 		PrintRoundedValue(  ostream* os, data_type val );
void 		SkipComments( ifstream* infile );
void 		FileCreateError( char* filename );
void 		FileOpenError( char* filename );
double		SafeAbs( double val1, double val2 );
float		SafeAbs( float val1, float val2 );
int			SafeAbs( int val1, int val2 );
double		SafeAbs( double val );
float		SafeAbs( float val );
int			SafeAbs( int val );

#endif

