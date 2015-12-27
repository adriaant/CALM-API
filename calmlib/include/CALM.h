/*
	Author:			Adriaan Tijsseling (AGT)
	Copyright: 		(c) Copyright 2002-2015 Adriaan Tijsseling. All rights reserved.
	Description:	API interface for the Categorization And Learning Module Network
*/

#ifndef __CALM__
#define __CALM__

#include <sys/types.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>
#include <string.h>
#include <fstream>
#include <iostream>
using namespace std;
#include <stdio.h>

#include "CALMNetwork.h"

// Class definition for the CALM API. 
class CALMAPI
{
public:
	
	CALMAPI();
	~CALMAPI();

		// create a log file
	int 				OpenCALMLog( const char* logname );
	inline void 		SetCALMLog( ofstream* log ) { mCALMLog = log; }
	inline void 		SetCALMLog( ostream* log ) { mCALMLog = log; }
	inline ostream*		GetCALMLog( void ) { return mCALMLog; }
	void				CloseCALMLog( void );	
		// maintain different directories
	int					CALMSetLogDirectory( char* dirname );
	int 				CALMDirectory( bool originalDir );
		// show the simulation parameters
	void				CALMShow( void );
	void 				CALMShowPatterns( void );
	void				CALMShowVerbosity( int verbosity );
		// creates CALM network based on network files passed to program
	void				CALMSetupNetwork( int* errFlags );
	void				CALMSetupNetwork( int* errFlags, char* filename );
		// writes current network to file
	void				CALMWriteNetwork( int* errFlags, char* filename );
		// loads pattern file
	int					CALMLoadPatterns( void );
		// loads feedback patterns file
	int 				CALMLoadFeedback( void );
		// loads parameter file
	int					CALMLoadParameters( void );
		// tell API that it will be used in online mode
	void				CALMOnlinePatterns( void );
		// trains one single pattern, which should be passed to API before
	int 				CALMTrainSingle( int epoch );
		// train the loaded pattern file
	int 				CALMTrainFile( int epoch );
		// tests one single pattern, which should be passed to API before
	int 				CALMTestSingle( int epoch );
	int 				CALMTestSingle( int epoch, bool useNoise );
		// test the loaded pattern file
	int 				CALMTestFile( int epoch );
		// Basic test. Assumes user has set input and resets values
	bool				CALMTest( int i, bool useNoise = false );
		// for saving changes in either weights, total act, and/or learning rate
	void				CALMSaveChanges( void );
		// saving/loading weights
	void				CALMSaveWeights( char const* filename );
	int					CALMLoadWeights( char const* filename );

//	INLINES	
		// R-unit clamping routines, pass index "idx" of module, index "node" of R-unit, and 
		// if clamping is true, then also pass the fixed activation value to use
	inline void			ClampUnit( int idx, int node, data_type val ) { mNetwork->ClampUnit( idx, node, val ); }
	inline void			ClampUnit( int idx, int node ) { mNetwork->ClampUnit( idx, node ); }
	inline bool			IsClamped( int idx, int node ) { return mNetwork->IsClamped( idx, node ); }
		
		// resizes modules
	inline bool			CALMResizeModule( void ){ return mNetwork->ResizeModule(); }
	inline void			CALMResizeModule( int idx ){ mNetwork->ResizeModule( idx ); }
	inline void			CALMResizeModule( int idx, int newsize ) { mNetwork->ResizeModule( idx, newsize ); }
		// show module sizes
	inline void			CALMShowSizes( void ) { mNetwork->PrintSizes( mCALMLog ); }

		// resets network: activations, weights, and/or winning nodes
	inline void			CALMReset( int type ) { mNetwork->Reset( type ); }
		// resets input activations to zero
	inline void			CALMReset( void ) { mNetwork->Reset(); }
		// force update of time delay connections (TESTING only)
	inline void			CALMUpdateTimeDelay( void ) { mNetwork->UpdateTimeDelay(); }
		// tools for recording weight changes
	inline void			CALMResetWeightChangeSum( void ) { mNetwork->ResetWtChangeSum(); }
	inline data_type	CALMGetWeightChangeSum( void ) { return mNetwork->GetWtChangeSum(); }
	inline void			CALMSaveWeightChangesComment( char* text ) { if ( mVerbosity & O_SAVEDWT ) mNetwork->SaveWeightChangesComment( text ); }

		// show module names
	inline void			CALMShowModules( void ) { mNetwork->PrintModules( mCALMLog ); }
	inline void			CALMShowModules( ostream* os ) { mNetwork->PrintModules( os ); }

		// show winning nodes at end of training, e.g, when verbosity is O_NONE
	inline void			CALMShowWinners( void ) { mNetwork->PrintWinners( mCALMLog ); }
	inline void			CALMShowOnlineWinners( void ) { mNetwork->PrintCurrentWinners( mCALMLog ); }
	inline void			CALMShowWinners( ostream* os ) { mNetwork->PrintWinners( os ); }
	inline void			CALMShowOnlineWinners( ostream* os ) { mNetwork->PrintCurrentWinners( os ); }

		// show activations at end of training when verbosity is O_NONE
	inline void			CALMShowActivations( int pat, int format, bool withInp ) { mNetwork->PrintActs( mCALMLog, pat, format, withInp ); }
		// show weights at end of training when verbosity is O_NONE
	inline void			CALMShowWeights( void ) { mNetwork->PrintWeights( mCALMLog ); }
		// show number of committed nodes and in case of maps
		// whether the map is ordered
	inline void			CALMShowCommitted( void ) { mNetwork->PrintCommitted( mCALMLog ); }

		// Get number of patterns
	inline int			CALMNumPatterns( void ) { return mNetwork->GetNumPatterns(); }
	inline void			CALMPatternOrder( int order ) { mNetwork->SetPatternOrder( order ); }
	inline void			CALMPermutePatterns( void ) { mNetwork->PermutePatterns(); }

		// return total activation over all R- and V-nodes
	inline data_type	CALMSumActivation( void ){ return mNetwork->SumActivation(); }	
		// same, but only for selected module	
	inline data_type	CALMSumActivation( int mdl ){ return mNetwork->SumActivation( mdl ); }	
		// return total activation over all R-nodes
	inline data_type	CALMSumActivationR( void ){ return mNetwork->SumActivationR(); }	
		// same, but only for selected module	
	inline data_type	CALMSumActivationR( int mdl ){ return mNetwork->SumActivationR( mdl ); }	
		// return total activation over all V-nodes
	inline data_type	CALMSumActivationV( void ){ return mNetwork->SumActivationV(); }	
		// same, but only for selected module	
	inline data_type	CALMSumActivationV( int mdl ){ return mNetwork->SumActivationV( mdl ); }	

		// To record the duration of a simulation, call this function with 
		// argument "kStart" to start recording time and "kEnd" to output
		// a formatted string displaying the duration of a simulation
	inline void 	 	CALMDuration( int start ) { CALMSpeedTest( start ); }

		// For 3D plots of weights
	inline void 	 	CALMInit3DPlot( const char* fromMdl, const char* toMdl ) { mNetwork->Init3DPlot( fromMdl, toMdl ); }
	inline void 	 	CALMResize3DPlot( const char* fromMdl, const char* toMdl ) { mNetwork->Resize3DPlot( fromMdl, toMdl ); }
	inline void 	 	CALMEnd3DPlot( void ) { mNetwork->End3DPlot(); }
	inline void 	 	CALM3DPlot( void ) { mNetwork->Do3DPlot(); }

//	GETTERS
		// retrieve pattern vector for selected module
	inline data_type*	CALMGetPattern( int mIdx, int pIdx ){ return mNetwork->GetPattern( mIdx, pIdx ); }
		// retrieve pattern value for selected node for selected module
	inline data_type	CALMGetPattern( int patIdx, int pIdx, int idx ){ return mNetwork->GetPattern( patIdx, pIdx, idx ); }
		// retrieve feedback signal for selected pattern
	inline int			CALMGetFeedback( int pIdx ){ return mNetwork->GetFeedback( pIdx ); }
		// Retrieve module index from module name
	inline int			CALMGetModuleIndex( char const *mdlname ){ return mNetwork->GetModuleIndex( mdlname ); }
		// Retrieve module size
	inline int			CALMGetModuleSize( int idx ){ return mNetwork->GetModuleSize( idx ); }
		// gets the winning node for a given module index
	inline int			CALMGetWinnerForModule( int mIdx ){ return mNetwork->GetWinner( mIdx ); }
		// get value of parameter
	inline data_type 	CALMGetParameter( int identifier ){ return mNetwork->GetParameter( identifier ); }
	inline char*		CALMGetDirectory( void ) 	 { return mDirname; 	  }
	inline char*		CALMGetCurDir( void ) 	 	 { return mCALMCurDir;    }
	inline char*		CALMGetBasename( void )		 { return mBasename;	  }
	inline int			CALMGetVerbosity( void )	 { return mVerbosity;	  }
	inline int			CALMGetNumModules( void ) 	 { return mNumModules; 	  }
	inline int			CALMGetNumInputs( void ) 	 { return mNumInputs; 	  }
	inline int			CALMGetNumRuns( void ) 		 { return mNumRuns; 	  }
	inline int			CALMGetNumEpochs(  void ) 	 { return mNumEpochs; 	  }
	inline int			CALMGetNumIterations( void ) { return mNumIterations; }
	inline int			CALMGetOrder( void ) 		 { return mOrder; 		  }
	inline int			CALMGetInputLen( void )		 { return mInputLen;	  }
	inline data_type	CALMGetOnlineInput( int i )	 { return mInput[i];	  }
	inline data_type*	CALMGetOnlineInput( void )	 { return mInput;	  	  }
	
//	SETTERS
		// Set the online input manually
	inline void	CALMSetInput( void ){ mNetwork->SetInput( mInput ); }		
		// Set the online input manually
	inline void	CALMSetInput( data_type* inp ){ mNetwork->SetInput( inp ); }		
		// To manually set a custom input pattern for given module
	inline void	CALMSetInput( int mIdx, data_type* inp ){ mNetwork->SetInput( mIdx, inp ); }
		// To manually set a custom feedback message
	inline void	CALMSetFeedback( int fb ){ mNetwork->SetOnlineFeedback( fb ); }
		// set value of parameter
	inline void CALMSetParameter( int identifier, data_type val ){ mNetwork->SetParameter( identifier, val ); }
		// set verbosity
	void		CALMSetVerbosity( int level );
	inline void	CALMSetNumRuns( int runs ) { mNumRuns = runs; }
	inline void	CALMSetNumEpochs( int epochs ) { mNumEpochs = epochs; }
	inline void	CALMSetNumIterations( int iters ) { mNumIterations = iters; }
	inline void	CALMSetOrder( int order ) { mOrder = order; }
	inline void	CALMSetConvStop( bool stop ) { mConvstop = stop; }
	inline void	CALMSetBasename( char* basename ) { strcpy( mBasename, basename ); }
	inline void	CALMSetDirectory( char* dirname ) { strcpy( mDirname, dirname ); }
	inline void	CALMSetOnlineInput( int i, data_type val ) { mInput[i] = val; }

private:

	int		CALMReadSpecs( char* newfilename );
	void	CALMSpeedTest( bool start );
	int		ModuleType( char* typeStr );
	int		ConnectionType( char* typeStr );

	ostream*  		mCALMLog;					// redirected cout
	ofstream*		mLogFile;					// file buffer
	char			mCALMCurDir[FILENAME_MAX];	// global to store the main directory
	char			mCALMLogDir[FILENAME_MAX];	// global to store log file directory
	clock_t			mCALMStartTime;				// to report duration of sims
	clock_t			mCALMEndTime;
	
	char			mBasename[32];	// the base name of the file, without suffix
	char			mDirname[128];	// directory with network files
	int				mVerbosity;		// indicates details to display in console
	bool			mConvstop;		// whether to stop training after convergence
	int				mNumModules;	// number of modules in network
	int				mNumInputs;		// number of input modules in network
	int				mNumRuns;		// number of runs to train the network
	int				mNumEpochs;		// number of epochs to train full patternset
	int				mNumIterations;	// number of iterations to present one single pattern
	int				mOrder;			// presentation type
	data_type*		mInput;			// custom input pattern
	int				mInputLen;		// length of input pattern (eq. total number of input nodes)
	bool			mFBOn;			// whether supervised learning is being used (set internally)
	
	CALMNetwork*	mNetwork;	// pointer to associated network 
};

#endif


