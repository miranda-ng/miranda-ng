/*

Tlen Protocol Plugin for Miranda NG
Copyright (C) 2004-2007  Piotr Piastucki

This program is based on GSM 06.10 source code developed by 
Jutta Degener and Carsten Bormann,
Copyright 1992, 1993, 1994 by Jutta Degener and Carsten Bormann,
Technische Universitaet Berlin 
  
This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

*/
#include "gsm.h"
#include <stdio.h>
#include <math.h>
#include <stdlib.h>

/* Decision levels */
const static int gsm_DLB[4] = {  6554,    16384,   26214,      32767   };
/* Quantization levels */
const static int gsm_QLB[4] = {  3277,    11469,   21299,      32767   };

/*
 * Long term parameters calculation - lag (Nc) and gain (bc).
 */
static void longTermLTPCalculation(gsm_state *state, short *in, short *in_1)
{
	int  	k, lambda;
   	int	   	Nc, bc;
   	float   in_1_float_base[120], * in_1_float = in_1_float_base + 120;
   	float   in_float[40];
 	float   L_result, L_max, L_power;

	for (k = -120; k < 0; k++) in_1_float[k] = (float)in_1[k];
	for (k =    0; k < 40; k++) in_float[k] =  (float)in[k];

	/* Search for the maximum cross-correlation and coding of the LTP lag */

	L_max = 0;
	Nc    = 40;   /* index for the maximum cross-correlation */

	for (lambda = 40; lambda < 121; lambda++) {
		for (L_result = 0, L_power = 0, k = 0; k < 40; k++) {
			L_result += in_float[k] * in_1_float[k - lambda];
//			L_power += in_1_float[k - lambda] * in_1_float[k - lambda];
		}
//		L_result = L_result / L_power;
		if (L_result > L_max) {
			Nc    = lambda;
			L_max = L_result;
		}
	}
	state->Nc[state->subframe] = Nc;
	if (L_max <= 0.)  {
		state->bc[state->subframe] = 0;
		return;
	}
	/*  Compute the power of the reconstructed short term residual signal dp[..] */
	in_1_float -= Nc;
	L_power = 0;
	for (k = 0; k < 40; k++) {
		float f = in_1_float[k];
		L_power += f * f;
	}
	if (L_max >= L_power) { //1.0
		state->bc[state->subframe] = 3;
		return;
	}
   /*  Coding of the LTP gain
    *  Table gsm_DLB must be used to obtain the level DLB[i] for the
    *  quantization of the LTP gain b to get the coded version bc.
    */
//	lambda = L_max * 32768.;
	lambda = (int)(L_max * 32768. / L_power);
	for (bc = 0; bc <= 2; ++bc) if (lambda <= gsm_DLB[bc]) break;
	state->bc[state->subframe] = bc;
}

/*
 *  Here we decode the bc parameter and compute samples of the estimate out[0..39].
 *  The decoding of bc needs the gsm_QLB table.
 *  The long term residual signal e[0..39] is then calculated to be fed to the
 *  RPE encoding section.
 */
static void longTermAnalysisFilter(int Nc, int bc, short *in, short *in_1, short *out, int *e)
{
	int	gain, k, l;
	gain = gsm_QLB[bc];
	for (k = 0; k < 40; k++) {
		l = (gain * (int)in_1[k - Nc] + 16384) >> 15;
		if (l < -32768) l = -32768;
		else if (l > 32767) l = 32767;
		out[k]  = l;
		e[k]   = in[k] - l;
	}
}

/*
 *  This procedure uses the bcr and Ncr parameter to realize the
 *  long term synthesis filtering.  The decoding of bcr needs table gsm_QLB.
 */
static void longTermSynthesisFilter(gsm_state * state, int Ncr, int bcr, int *e, short *out)
{
	int 	k;
	int 	brp, Nr;

	/*  Check the limits of Nr. */
	Nr = Ncr < 40 || Ncr > 120 ? state->nrp : Ncr;
	state->nrp = Nr;
	/*  Decoding of the LTP gain bcr */
	brp = gsm_QLB[ bcr ];

   /*  Computation of the reconstructed short term residual
    *  signal drp[0..39]
    */
	for (k = 0; k < 40; k++) {
		int temp = (brp * (int)out[ k-Nr ] + 16384) >> 15;
		out[k] = e[k] + temp;
   }
}

/*
 *  This procedure performs long term analysis.
 */
void longTermAnalysis(gsm_state *state, short *in)
{
	short *in_1 = state->dp0 + 120 + state->subframe * 40;
	short *out = state->dp0 + 120 + state->subframe * 40;
	int *e = state->e + state->subframe * 40;
/*

	int   i;
	printf("Long-term in: \n");
	for (i=0;i<40;i++) {
		printf("%7d ", in[i]);
	}
	printf("\n");
	*/
	longTermLTPCalculation(state, in, in_1);
	/* printf("Nc: %d, bc: %d \n", state->Nc[state->subframe], state->bc[state->subframe]);
	*/
	longTermAnalysisFilter(state->Nc[state->subframe], state->bc[state->subframe], in, in_1, out, e);
/*
	printf("Long-term out: \n");
	for (i=0;i<40;i++) {
		printf("%7d ", out[i]);
	}
	printf("\n");
	*/
}

/*
 *  This procedure performs long term synthesis.
 */
void longTermSynthesis(gsm_state *state)
{
	int Nc = state->Nc[state->subframe];
	int bc = state->bc[state->subframe];
	int *e = state->e + state->subframe * 40;
	short *out = state->dp0 + 120 + state->subframe * 40;
	//int i;
	longTermSynthesisFilter(state, Nc, bc, e, out);
	/*
	printf("Long-term reconstructed: \n");
	for (i=0;i<160;i++) {
		printf("%7d ", state->dp0[i]);
	}
	printf("\n");
	*/
}
