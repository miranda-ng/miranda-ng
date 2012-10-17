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
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
extern void lpcAnalysis(gsm_state *state, short *in);

/* 
 * Dequantize LAR coefficients
 */
static void dequantizeLARs(int *qLARs, int *LARs)
{
	int dqA[]={13107, 13107, 13107, 13107, 19223, 17476, 31454, 29708};
	int dqB[]={0, 0, 2048, -2560, 94, -1792, -341, -1144};
	int dqMin[]={-32, -32, -16, -16, -8, -8, -4, -4};
	int i;
	for (i = 0; i < 8; i++) {
		int temp;
		temp = ((((qLARs[i] + dqMin[i]) << 10) - (dqB[i] << 1)) * dqA[i] + 16384) >> 15;
		LARs[i] = temp + temp;
	}
}
/*
 * Interpolate LAR coefficients (samples 0..12)
 */
static void interpolateLARs_0_12 (int *LARpp_j_1, int *LARpp_j, int *LARp)
{	int i;
	for (i = 0; i < 8; i++) {
		LARp[i] = (LARpp_j_1[i] >> 1) + (LARpp_j_1[i] >> 2) + (LARpp_j[i] >> 2);
	}
}
/*
 * Interpolate LAR coefficients (samples 13..26)
 */
static void interpolateLARs_13_26 (int *LARpp_j_1, int *LARpp_j, int *LARp)
{	int i;
	for (i = 0; i < 8; i++) {
		LARp[i] = (LARpp_j_1[i] >> 1) + (LARpp_j[i] >> 1);
	}
}
/*
 * Interpolate LAR coefficients (samples 27..39)
 */
static void interpolateLARs_27_39 (int *LARpp_j_1, int *LARpp_j, int *LARp)
{	int i;
	for (i = 0; i < 8; i++) {
		LARp[i] = (LARpp_j[i] >> 1) + (LARpp_j_1[i] >> 2) + (LARpp_j[i] >> 2);
	}
}
/*
 * Interpolate LAR coefficients (samples 40..159)
 */
static void interpolateLARs_40_159 (int *LARpp_j_1, int *LARpp_j, int *LARp)
{	int i;
	for (i = 0; i < 8; i++) {
		LARp[i] = LARpp_j[i];
	}
}
/*
 * Convert LAR coefficients to reflection coefficients
 */
static void LARToR(	int * LARp) 
{
	int 		i;
	int 		temp;
	for (i = 0; i < 8; i++) {
		temp = abs(LARp[i]);
		if (temp < 11059) temp <<= 1;
		else if (temp < 20070) temp += 11059;
		else temp = (temp >> 2) + 26112;
		LARp[i] = LARp[i] < 0 ? -temp : temp;
	}
}

/*
 *  This procedure computes the short term residual signal d[..] to be fed
 *  to the RPE-LTP loop from the s[..] signal and from the local rp[..]
 *  array (quantized reflection coefficients).  As the call of this
 *  procedure can be done in many ways (see the interpolation of the LAR
 *  coefficient), it is assumed that the computation begins with index
 *  k_start (for arrays d[..] and s[..]) and stops with index k_end
 *  (k_start and k_end are defined in 4.2.9.1).  This procedure also
 *  needs to keep the array u[0..7] in memory for each call.
 */
static void shortTermAnalysisFilter (gsm_state * state, int * rp, int len, short * in) 
{
	int		* u = state->u;
	int		i;
	int		di, ui, sav, rpi;
	for (; len--; in++) {
		di = sav = *in;
		for (i = 0; i < 8; i++) {	
			ui    = u[i];
			rpi   = rp[i];
			u[i]  = sav;
			sav   = ui + ((rpi * di + 16384) >> 15);
			di    = di + ((rpi * ui + 16384) >> 15);
		}
		*in = di;
	}
}

static void shortTermSynthesisFilter(gsm_state * state, int	* rrp, int	len, short * in, short *out)
{
	int 	* v = state->v;
	int		i;
	int 	sri;
	for (; len--; in++, out++) {
		sri = *in;
		for (i = 8; i--;) {
			sri -= (rrp[i] * v[i] + 16384) >> 15;
			if (sri < -32768) sri = -32768;
			else if (sri > 32767) sri = 32767;
			v[i+1] = v[i] + ((rrp[i] * sri + 16384) >> 15);
			if (v[i+1] < -32768) v[i+1] = -32768;
			else if (v[i+1] > 32767) v[i+1] = 32767;
		}
		*out = v[0] = sri;
	}
}

void shortTermAnalysis(gsm_state *state, short *in) 
{
	int * qLARs = state->qLARs;
	int * LARpp_j = state->LARpp[state->j];
	int * LARpp_j_1 = state->LARpp[state->j ^1];
	int	LARp[8];
//	int i;
	
	lpcAnalysis(state, in);//i_samples);
/*
	printf("short term in: \n");
	for (i=0;i<160;i++) {
		printf("%7d ", in[i]);//((float)i_lpc[i])/32768.0f);
	}
	printf("\n");
	*/
	dequantizeLARs(qLARs, LARpp_j);
	interpolateLARs_0_12(LARpp_j_1, LARpp_j, LARp);
	LARToR(LARp);
	shortTermAnalysisFilter(state, LARp, 13, in);
	interpolateLARs_13_26(LARpp_j_1, LARpp_j, LARp);
	LARToR(LARp);
	shortTermAnalysisFilter(state, LARp, 14, in+13);
	interpolateLARs_27_39(LARpp_j_1, LARpp_j, LARp);
	LARToR(LARp);
	shortTermAnalysisFilter(state, LARp, 13, in+27);
	interpolateLARs_40_159(LARpp_j_1, LARpp_j, LARp);
	LARToR(LARp);
	shortTermAnalysisFilter(state, LARp, 120, in+40);
	/*
	printf("short-term residuals: \n");
	for (i=0;i<160;i++) {
		printf("%7d ", in[i]);//((float)i_lpc[i])/32768.0f);
	}
	printf("\n");
	*/
}

void shortTermSynthesis(gsm_state *state, short *in, short *out) 
{
	int * qLARs = state->qLARs;
	int * LARpp_j = state->LARpp[state->j];
	int * LARpp_j_1 = state->LARpp[state->j ^1];
	int	LARp[8];
//	int i;

	dequantizeLARs(qLARs, LARpp_j);
	interpolateLARs_0_12(LARpp_j_1, LARpp_j, LARp);
	LARToR(LARp);
	shortTermSynthesisFilter(state, LARp, 13, in, out);
	interpolateLARs_13_26(LARpp_j_1, LARpp_j, LARp);
	LARToR(LARp);
	shortTermSynthesisFilter(state, LARp, 14, in+13, out+13);
	interpolateLARs_27_39(LARpp_j_1, LARpp_j, LARp);
	LARToR(LARp);
	shortTermSynthesisFilter(state, LARp, 13, in+27, out+27);
	interpolateLARs_40_159(LARpp_j_1, LARpp_j, LARp);
	LARToR(LARp);
	shortTermSynthesisFilter(state, LARp, 120, in+40, out+40);
	/*
	printf("samples[reconstructed]: \n");
	for (i=0;i<160;i++) {
		printf("%7d ", out[i]);
	}
	*/
}
