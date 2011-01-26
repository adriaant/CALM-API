/*
	Author:			Adriaan Tijsseling (AGT)
	Copyright: 		(c) Copyright 2002-2011 Adriaan Tijsseling. All rights reserved.
	Description:	API interface for Sequence Learning Network
*/

float	PseudoRNG( float low, float high );
float	PseudoRNG( float range );
float	PseudoRNG( void );
void	SetSeed( long );
long	GetSeed( void );
