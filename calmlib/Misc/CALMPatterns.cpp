/*
	Author:			Adriaan Tijsseling (AGT)
	Copyright: 		(c) Copyright 2002-2015 Adriaan Tijsseling. All rights reserved.
	Description:	Implementation of Pattern class
*/

#include "CALMGlobal.h"
#include "Utilities.h"
#include "CALMPatterns.h"


// free up memory
void CALMPatterns::DeletePatterns( void )
{
	// clean up the patterns storage matrix
	if ( mNumPatterns != 0 )
		DisposeMatrix( mPatterns, mNumPatterns );
	mNumPatterns = 0;
}


// Load patterns from file: called within CALMNetwork::LoadPatterns
void CALMPatterns::LoadPatterns( ifstream* infile, int numPatterns, int moduleSize ) 
{
	// read the number of patterns
	mNumPatterns = numPatterns;
	mModuleSize  = moduleSize;
		
	// create the storage matrix
	mPatterns = CreateMatrix( 0.0, mNumPatterns, mModuleSize );

	// read each pattern in
	for ( int i = 0; i < mNumPatterns; i++ )
	{
		for ( int j = 0; j < mModuleSize; j++ )
		{
			SkipComments( infile );	// ignore any comments;
			*infile >> mPatterns[i][j];
		}
	}
}


// useless function, but let's keep it in
void CALMPatterns::Print( ostream *os ) 
{
	for ( int i = 0; i < mNumPatterns; i++ ) 
	{
		*os << '\t';
		for ( int j = 0; j < mModuleSize; j++ ) 
			*os << mPatterns[i][j] << " ";
		*os << endl;
	}
}


// overload the << operator
ostream &operator<<( ostream &os, CALMPatterns &m )
{
	m.Print( &os );
	return os;
}

