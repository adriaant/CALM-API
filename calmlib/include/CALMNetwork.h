/*
	Author:			Adriaan Tijsseling (AGT)
	Copyright: 		(c) Copyright 2002-2013 Adriaan Tijsseling. All rights reserved.
	Description:	Class definition for CALM network
*/

#ifndef __CALMNET__
#define __CALMNET__

#include <fstream>
using namespace std;
#include <string.h>
#include "CALMPatterns.h"
#include "Module.h"
#include "GnuPlot.h"

class CALMNetwork 
{   
public:

	CALMNetwork();
	~CALMNetwork();

// CREATERS
						// set number of modules and create array
	void				SetNumModules( int numModules, int numInputs );
	void 				SetNumConnections( int idx, int numConn );
	void				ConnectModules( int idx, int toIdx, int fromIdx, int link, int delay );
						// initializes each module
	void				InitializeModule( int idx, int calmType, int moduleSize, char* moduleName );
	
// MODULE RESIZERS
	void				ResizeModule( int idx, int newsize );
	void				ResizeModule( int idx );
	bool				ResizeModule( void );
	
// RESETTERS
	void				Reset( SInt16 resetOption );
	void				Reset( void );
	void				UpdateTimeDelay( void );
	
// PARAMETERS	
	bool				LoadParameters( const char* filename );
	void 				SetParameter( const int parType, const data_type val );
	data_type			GetParameter( const int parType );
	
// PATTERNS
	bool				LoadFeedback( const char* filename );
	bool				LoadPatterns( const char* filename );
	void				OnlinePatterns( void );
	void				PermutePatterns( void );
	void				SetPatternOrder( int order );
	inline int			GetPatternOrder( void ){ return mPatternOrder; }
	
// TRAINERS
	void 				Learn( void );
	void				Test( void );
	void				Test( bool useNoise );
	void				SetInput( int patIdx );
	void				SetInput( int moduleIdx, int patIdx );
	void				SetInput( data_type* input );
	void				SetInput( int patIdx, data_type* input );
	void				SetOnlineFeedback( int fb );
	void				SetFeedback( int pIdx );
	bool				CollectWinners( int patIdx, int ite );
	bool				CollectWinnersOnTheFly( int ite, int mIdx, int* convInfo );
	inline void			SetWinner( int i, int j, int idx ) { mWinners[i][j] = idx; };
	inline int			GetWinner( int i, int j ) { return mWinners[i][j]; };
	inline int			GetWinner( int i ) { return GetWinner( i-mNumInputModules, 0 ); }
	inline void			SetConvTime( int i, int j, int idx ) { mConvTimes[i][j] = idx; };
	inline int			GetConvTime( int i, int j ) { return mConvTimes[i][j]; };
	inline data_type	GetWtChangeSum( void ) { return mWtChangeSum; }
	inline void			ResetWtChangeSum( void ) { mWtChangeSum = 0.0; }
	
// SAVERS
	void				SetWeightChangeFile( char* filename );
	void				SaveWeightChanges( void );
	inline void			SaveWeightChangesComment( char* text ) { mWeightChangeFile << text << endl; }
	void				SetActChangeFile( char* filename );
	void				SaveActChanges( void );
	data_type			SumActivation( void );
	data_type			SumActivation( int mdl );
	data_type			SumActivationR( void );
	data_type			SumActivationR( int mdl );
	data_type			SumActivationV( void );
	data_type			SumActivationV( int mdl );
	void				SetMuChangeFile( char* filename );
	void				SaveMuChanges( void );
	void				SaveWeights( char* filename );
	bool				LoadWeights( char* filename );

// MISC			
	inline void			ClampUnit( int idx, int node, data_type val ) { mModules[idx]->ClampUnit( node, val ); }
	inline void			ClampUnit( int idx, int node ) { mModules[idx]->ClampUnit( node ); }
	inline bool			IsClamped( int idx, int node ) { return mModules[idx]->IsClamped( node ); }

// GNUPLOT link
	void 	 			Init3DPlot( char* fromMdl, char* toMdl );
	void				Resize3DPlot( char* fromMdl, char* toMdl );
	void 	 			End3DPlot( void );
	void 	 			Do3DPlot( void );

// PRINTERS
	bool				WriteSpecs( char* filename );
	const char*			GetTypeString( int modtype );
	int					GetMaximumSize( void );
	void				PrintCommitted( ostream* os );
	void				PrintModules( ostream* os );
	void				PrintWinners( ostream* os );
	void				PrintCurrentWinners( ostream* os );
	void				PrintWeights( ostream* os, int epoch, int pIdx );
	void				PrintWeights( ostream* os );
	void				PrintActs( ostream* os, int epoch, int ite, int format, bool withInp );
	void				PrintActs( ostream* os, int pat, int format, bool withInp );
	void				PrintPotentials( ostream* os );
	void				PrintSizes( ostream* os );
	void				PrintPatterns( ostream* os );
	void				PrintFeedback( ostream* os );
	void 				Print( ostream *os );

// GETTERS	
	inline int			GetNumModules( void ) { return mNumModules; }
	inline int			GetNumInputs( void ) { return mNumInputModules; }
	inline int			GetModuleSize( int idx ) { return mModules[idx]->GetModuleSize(); }
	inline Module*		GetModule( int idx ) { return mModules[idx]; }
	int 				GetModuleIndex( char const *mdlname );
	inline data_type	GetModuleActivation( int idx, int i ) { return mModules[idx]->GetActivationR(i); }
	inline int			GetNumPatterns( void ){ return mNumPatterns; }
	data_type*			GetPattern( int mIdx, int pIdx );
	data_type			GetPattern( int mIdx, int pIdx, int idx );
	inline int			GetFeedbackModule( void ) { return mFeedback; }
	int					GetFeedback( int pIdx );
	inline data_type	GetMu();

	friend ostream &operator<<( ostream &os, CALMNetwork *m );
	
private:

	data_type		mWtChangeSum;			// sum of weight changes
	data_type 		mParameters[gNumPars];	// array to hold the values
	int   			mNumModules;			// number of modules
	int				mNumInputModules;		// number of input modules
	Module**		mModules;				// array of modules
	char			mPatternFileName[256];	// name of loaded pattern file
	CALMPatterns*	mPatternList;			// array of Patterns for each input module
	int*			mFeedbackList;			// list of feedback data
	int				mFeedback;				// index of module designated to receive feedback
	int				mNumPatterns;			// number of patterns
	int				mPatternOrder;			// present patterns permuted or ordered
	int*			mPermutations;			// permuted array of pattern indexes
	int**			mWinners;				// store winners for each pattern and module
	int**			mConvTimes;				// store time of convergence
	ofstream		mWeightChangeFile;		// file to store changes in weights
	ofstream		mActChangeFile;			// file to store changes in activation
	ofstream		mMuChangeFile;			// file to store changes in learning rate
	
	GnuPlot* 		mGnuPlot;
	GnuPlot*		m3DPlot;
	int				m3DTo;
	int				m3DIdx;
};


#endif

