/*
	Author:			Adriaan Tijsseling (AGT)
	Copyright: 		(c) Copyright 2002-2015 Adriaan Tijsseling. All rights reserved.
	Description:	Pattern class. Contains everything related to reaading in,
					storing, and accessing patterns. A subclass can be derived 
					if you want to add preprocessing functions
*/


#ifndef __CALMPATTERNS__
#define __CALMPATTERNS__

#include	<fstream>
using namespace std;

class CALMPatterns
{

public:

	CALMPatterns(){ mModuleSize = 0; mNumPatterns = 0; }

	~CALMPatterns(){ DeletePatterns(); }
	
	// Loading/Deleting/Resetting
	void	LoadPatterns( ifstream* infile, int numPatterns, int moduleSize );
	void	DeletePatterns( void );
	
	// Display function
	void	Print( ostream *os );
	
	// Accessor functions
	inline data_type*	GetPattern( int i ) { return (data_type*)mPatterns[i]; }
	inline data_type	GetPattern( int i, int j ) { return mPatterns[i][j]; }	
	inline int			GetNumPatterns( void ) { return mNumPatterns; }

	friend ostream &operator<<( ostream &os, CALMPatterns &m );
	
protected:

	data_type**		mPatterns;
	int				mNumPatterns;
	int				mModuleSize;
};

#endif
