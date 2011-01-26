/*
	Author:			Adriaan Tijsseling (AGT)
	Copyright: 		© Copyright 2002-2011 Adriaan Tijsseling. All rights reserved.
	Description:	Class definition for connections between modules
*/

#ifndef __CONN__
#define __CONN__

#include "CALMWeight.h"
#include "Module.h"


class Connection
{

public:

	Connection() {}
	~Connection();
	
	void 		Initialize( Module* inModule, int* toSize, int linkType, int delay, data_type* pars );
	void		ResizeConnection( int fromsize, int tosize, int node, int direction );
	void		Reset( data_type );
	void		Reset( int );
	void		Reset( void );

	data_type	WeightedActivation( int idx );
	data_type	WeightedDelay( int idx );

	inline data_type NormalActivation( int idx ){ return mInModule->GetActivationR( idx ); }
	inline data_type DelayedActivation( int idx ){ return mInModule->GetDelayAct( idx ); }
	
	void		TickClock( void );
	void		UpdateNormal( int idx, data_type act, data_type mu, data_type backAct, data_type &dw_sum );
	void		UpdateDelay( int idx, data_type act, data_type mu, data_type backAct, data_type &dw_sum );
	
	void		SumWeightChanges( data_type &dw_sum );
	
	void		CopyWeights( double** matrix );
	void		SaveWeights( ofstream *outfile );
	void		LoadWeights( ifstream *outfile );
	void 		Print( ostream *os );	
	
	inline void			SetWeight( int i, int j, data_type dw ) { mWeights[i][j].SetWeight( dw, mParameters[K_Lmax], mParameters[K_Lmin] ); }
	inline data_type	GetWeight( int i, int j ) { return mWeights[i][j].GetWeight(); }	
	inline data_type	GetWeightChange( int i, int j ) { return mWeights[i][j].GetWeightChange(); }	
	inline Module*		GetInModule( void ) { return mInModule; }
	inline int			GetModuleSize( void ) { return mInModule->GetModuleSize(); }
	inline int			GetModuleIndex( void ) { return mInModule->GetModuleIndex(); }
	inline int			GetModuleType( void ) { return mInModule->GetModuleType(); }
	inline char*		GetModuleName( void ) { return mInModule->GetModuleName(); }
	inline int			GetDelay( void ) { return mDelay; }
	inline int			GetType( void ) { return mType; }
	inline void			SetType( int linkType ) { mType = linkType; }

	friend ostream &operator<<( ostream &os, Connection &c );

	// function pointer to speed up learning
	data_type		(Connection::*mWeightedAct)( int idx );	
	data_type		(Connection::*mInAct)( int idx );	
	void			(Connection::*mUpdate)( int idx, data_type act, data_type mu, data_type backAct, data_type &dw_sum );	
	
protected:

	int*			mToSize;		// number of R-nodes in to-Module
	Module*			mInModule;		// from-Module
	CALMWeight**	mWeights;		// weights on this connection
	int				mType;			// normal or time-delay connection
	// for time delay
	int				mDelay;			// delay of connection
	int				mTime;			// current time (in updates)
	data_type*		mWtAct;			// local copy of weighted activation
	data_type*		mParameters;	// pointer to Network's storage of parameters
};

#endif
