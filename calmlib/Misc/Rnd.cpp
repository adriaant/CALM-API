/*
	STandard RAndom Number GEneratoR ( STRANGER )
	Version 1.2.4, R. Alexander Raccuglia ( 1993, 1994, 1995 )

	provides a standard pseudo_random_number_generation capability

	pseudo_random_number_generator by: Wichmann and Hill ( 1982, 1984, 1987 )
	see also Brysbaert ( 1991 ), MacLaren ( 1989 ), McLeod ( 1985 ), and Zeisel ( 1986 )

	References

	Brysbaert, M. ( 1991 ). Algorithms for randomness in the behavioral sciences:
	a tutorial. Behavior Research Methods, Instruments, & Computers, 23, 45-60.

	MacLaren, N. M. ( 1989 ). The generation of multiple independent sequences
	of pseudorandom numbers. Applied Statistics , 38, 351-359.

	McLeod, A. I. ( 1985 ). A remark on Algorithm AS183: An efficient and
	portable pseudo-random number generator. Applied Statistics, 34, 198-200.

	Wichmann, B. A., & Hill, J. D. ( 1982 ). Algorithm AS183: An efficient and
	portable pseudo random number generator. Applied Statistics, 31, 188-190.

	Wichmann, B. A., & Hill, J. D. ( 1984 ). An efficient and portable pseudo
	random number generator: Correction. Applied Statistics, 33, 123.

	Wichmann, B. A., & Hill, J. D. ( 1987, March ). Building a random-number
	generator. Byte, pp. 127-128.

	Zeisel, H. ( 1986 ). A remark on Algorithm AS183: An efficient and portable
	pseudo-random number generator. Applied Statistics, 35, 89.
*/


#if TARGET_OS_MAC
        #include <time.h>
#else
        #include <sys/time.h>
        #include <unistd.h>
#endif
#include <math.h>
#include <stdlib.h>
#include "CALMGlobal.h"
#include "Rnd.h"


// drand48 and srand48 are not defined on MacOS
#if !defined drand48
	#define drand48()	( (float)rand() )/( (float)RAND_MAX )
	#define srand48(x)	srand(x)
#endif

#define Randomize()  srand((int)time(NULL))

int		s = 0;
int		seed1 = 1, seed2 = 1, seed3 = 1;

void	SetSeed( int );

// returns a random float in the range [-l,h].
float PseudoRNG( float low, float high )
{
	double rp;
	double ipart;

	seed1 = 171 * ( seed1 % 177 ) -  2 * ( seed1 / 177 );
	seed2 = 172 * ( seed2 % 176 ) - 35 * ( seed2 / 176 );
	seed3 = 170 * ( seed3 % 178 ) - 63 * ( seed3 / 178 );
	if ( seed1 < 0 ) seed1 += 30269;
	if ( seed2 < 0 ) seed2 += 30307;
	if ( seed3 < 0 ) seed3 += 30323;
	rp = modf( ( seed1 / 30269.0 + seed2 / 30307.0 + seed3 / 30323.0 ), &ipart );

	// move to interval
	rp = rp * ( high - low ) + low;
	return( (float) rp );
}


// returns a random double in the range [-x,x].
float PseudoRNG( float range )
{
	double rp;
	double ipart;

	seed1 = 171 * ( seed1 % 177 ) -  2 * ( seed1 / 177 );
	seed2 = 172 * ( seed2 % 176 ) - 35 * ( seed2 / 176 );
	seed3 = 170 * ( seed3 % 178 ) - 63 * ( seed3 / 178 );
	if ( seed1 < 0 ) seed1 += 30269;
	if ( seed2 < 0 ) seed2 += 30307;
	if ( seed3 < 0 ) seed3 += 30323;
	rp = modf( ( seed1 / 30269.0 + seed2 / 30307.0 + seed3 / 30323.0 ), &ipart );
	if( rp <  0.0 ) rp = 0.000000000000000;
	if( rp >= 1.0 ) rp = 1.000000000000000;

	// move to interval [ -1, 1 ]
	rp = ( rp * 2.0 ) - 1.0;
	// move to range [-x,x]
	rp *= range;
	
	return( (float)rp );
}


// returns a random double in the range [0,1].
float PseudoRNG( void )
{
	double rp;
	double ipart;

	seed1 = 171 * ( seed1 % 177 ) -  2 * ( seed1 / 177 );
	seed2 = 172 * ( seed2 % 176 ) - 35 * ( seed2 / 176 );
	seed3 = 170 * ( seed3 % 178 ) - 63 * ( seed3 / 178 );
	if ( seed1 < 0 ) seed1 += 30269;
	if ( seed2 < 0 ) seed2 += 30307;
	if ( seed3 < 0 ) seed3 += 30323;
	rp = modf( ( seed1 / 30269.0 + seed2 / 30307.0 + seed3 / 30323.0 ), &ipart );
	if( rp <  0.0 ) rp = 0.000000000000000;
	if( rp >= 1.0 ) rp = 1.000000000000000;

	return( (float)rp );
}


long GetSeed( void )
{
#if TARGET_OS_MAC
	unsigned long	seed;	
	GetDateTime( &seed );
	return seed;
#else
	struct timeval now;
	gettimeofday( &now, NULL );
	return now.tv_usec;
#endif	
}


/* This function allows the user to set the 32 bits of the internal seed.
 * The seed is used for the initialization of seed1, seed2, & seed3, to
 * be used by frand().  [RAND_MAX = 65536] */
void SetSeed( long seed )
{
	if ( seed != 0 )
	{
		srand48( seed );
		seed1 = (int)( drand48() * 30268.0 ) + 1;
		seed2 = (int)( drand48() * 65536.0 ) + 1;
		seed3 = (int)( drand48() * 65536.0 ) + 1;
	}
	else
		seed1 = 1; seed2 = 1; seed3 = 1;
}
