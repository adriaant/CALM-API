/*
	Author:			Adriaan Tijsseling (AGT)
	Copyright: 		(c) Copyright 2002-2013 Adriaan Tijsseling. All rights reserved.
	Description:	Abstract class definition for a generic CALM module
*/


#ifndef __MODULE__
#define __MODULE__

#include	<fstream>
#include	<iostream>
using namespace std;
#include "AUnit.h"
#include "EUnit.h"
#include "RUnit.h"
#include "VUnit.h"

class Connection;

class Module
{
	friend class Connection;
	
public:

	Module( ) { mModuleSize = 0; mNumInConn = 0; mModuleType = O_CALM; mModuleIndex = kUndefined; }
	~Module();
	
	virtual void		Initialize( int mModuleSize, char* moduleName, data_type* pars, int mtype, int idx );
	void				Connect( int idx, Module* fromModule, int link, int delay );
	bool				NeedsResizing( int* node );
	void				ResizeModule( int newsize, int node );
	void				ResizeConnection( int newsize, int node, int idx );
	void				Reset( SInt16 resetOption );
	void				Reset( void );
	void				ResetInput( void );
	inline void			ClampUnit( int idx, data_type val ) { mR[idx].ClampUnit( val ); mV[idx].ClampUnit( val ); }
	inline void			ClampUnit( int idx ) { mR[idx].ClampUnit();mV[idx].ClampUnit(); }
	inline bool			IsClamped( int idx ) { return mR[idx].IsClamped(); }
	void				UpdateTimeDelay( void );
	
	virtual void		SetInput( data_type* input );
	virtual void		SetInput( data_type input, int i );
	virtual void		UpdateActivation( void );
	virtual void		UpdateActivationTest( void );
	virtual void		UpdateActivationTest( bool );
	virtual void		UpdateWeights( data_type &dw_sum );
	virtual void		SwapActs( void );
	virtual void		ConvCheck( int t, int* winner, int* convtime );
	
	void				SumWeightChanges( data_type &dw_sum );
	void				SumActivation( data_type &act_sum );
	void				SumActivationR( data_type &act_sum );
	void				SumActivationV( data_type &act_sum );
	
	void				PrintWeights( ostream* os );
	void				PrintActs( ostream* os, int format );
	void				PrintPotentials( ostream* os );
	void				PrintSizes( ostream* os );
	virtual void 		Print( ostream *os );
	void				SaveWeights( ofstream *outfile );
	void				LoadWeights( ifstream *infile );
	
	inline int			GetModuleIndex( void ) { return mModuleIndex; }	
	inline int			GetModuleType( void ) { return mModuleType; }
	inline int			GetModuleSize( void ) { return mModuleSize; }
	inline char*		GetModuleName( void ) { return mModuleName; }
	inline int			GetNumInConn( void ) { return mNumInConn; }
	inline int			GetWinner( void ) { return mWinner; }
	inline int			GetConvTime( void ) { return mConvTime; }
	inline data_type	GetDelayAct( int i ) { return mR[i].GetDelayAct(); }
	inline data_type	GetActivationR( int i ) { return mR[i].GetActivation(); }
	inline data_type	GetActivationV( int i ) { return mV[i].GetActivation(); }
	inline data_type	GetActivationA( void ) { return mA.GetActivation(); }
	inline data_type	GetActivationE( void ) { return mE.GetActivation(); }
	virtual data_type	GetWeight( int inConIdx, int i, int j );
	inline data_type	GetLearningRate( void ) { return mMu; }
	
	char*				GetConnModuleName( int idx );
	int					GetConnType( int idx );
	int					GetConnDelay( int idx );
	void				CopyWeights( int idx, double** matrix );

	void				SetNumConn( int numInConn );
	inline void			SetWinner( int idx ) { mWinner = idx; }
	inline void			SetConvTime( int t ) { mConvTime = t; }

	friend ostream &operator<<( ostream &os, Module *m );
	
protected:

	int			mModuleIndex;		// reference index of this module
	int			mModuleType;		// type of module
	char		mModuleName[32];	// name of this module
	int			mModuleSize;		// number of RV-pairs in the module
	int			mNumInConn;			// number of incoming connections
	Connection*	mInConn;			// array of incoming connections
	int			mWinner;			// winning RV-pair
	int			mConvTime;			// epoch of convergence
	RUnit*		mR;					// array of R-nodes
	VUnit*		mV;					// array of V-nodes
	AUnit		mA;					// A-node
	EUnit		mE;					// E-node
	data_type	mMu;				// copy of current learning rate
	data_type*	mParameters;		// pointer to Network's storage of parameters
};

#endif
