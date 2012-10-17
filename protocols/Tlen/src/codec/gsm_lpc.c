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

/* LPC- and Reflection Coefficients
 * The next two functions calculate linear prediction coefficients
 * and/or the related reflection coefficients from the first P_MAX+1
 * values of the autocorrelation function.
 */
#include "gsm.h"     /* for P_MAX */
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

/* Compute the autocorrelation
 *          ,--,
 *   ac(l) = >  x(i) * x(i-l)  for all i
 *          `--'
 * for lags l between 0 and lag-1, and x(i) == 0 for i < 0 or i >= n
 * @param n: number of samples
 * @param lax: max lag (order)
 * @param in: [0...n-1] samples x
 * @param out: [0...lag-1] autocorrelation
 */
static void autocorrelation(int n, float const *x, int lag, float *ac)
{
    float d;
    int i, j;
    for (j = 0; j < lag; j++) {
        for (i = j, d = 0; i < n; i++) d += x[i] * x[i-j];
        ac[j] = d;
    }
	d = (float)fabs(ac[0]);
	if (d != 0) {
		for (j = 0; j < lag; j++) ac[j] /= d;
	}
}

/* The Levinson-Durbin algorithm was invented by N. Levinson in 1947
 * and modified by J. Durbin in 1959.
 * @param in: [0...p] autocorrelation values
 * @param out: [0...p-1] reflection coefficients
 * @param lpc:  [0...p-1] LPC coefficients
 * @return minimum mean square error
 */
/*
static float levinson_durbin (float const *ac, float *ref, float *lpc)
{
    int i, j;
    float r, error = ac[0];

    if (ac[0] == 0) {
        for (i = 0; i < P_MAX; i++) ref[i] = 0;
        return 0;
    }

    for (i = 0; i < P_MAX; i++) {

        r = -ac[i + 1];
        for (j = 0; j < i; j++) r -= lpc[j] * ac[i - j];
        ref[i] = r /= error;

        // Update LPC coefficients and total error. 
        lpc[i] = r;
        for (j = 0; j < i / 2; j++) {
            float tmp      = lpc[j];
            lpc[j]          = r * lpc[i - 1 - j];
            lpc[i - 1 - j] += r * tmp;
        }
        if (i % 2) lpc[j] += lpc[j] * r;

        error *= 1 - r * r;
    }
    return error;
}
*/
/* I. Schur's recursion from 1917 is related to the Levinson-Durbin method,
 * but faster on parallel architectures; where Levinson-Durbin would take time
 * proportional to p * log(p), Schur only requires time proportional to p. The
 * GSM coder uses an integer version of the Schur recursion.
 * @param in: [0...p] autocorrelation values
 * @param out: [0...p-1] reflection coefficients
 * @return the minimum mean square error
 */
static float schur(float const *ac, float *ref)
{
    int i, m;
    float error=ac[0], r, G[2][P_MAX];

    if (ac[0] == 0.0) {
        for (i = 0; i < P_MAX; i++) ref[i] = 0;
        return 0;
    }

    /* Initialize the rows of the generator matrix G to ac[1...p]. */
    for (i = 0; i < P_MAX; i++) G[0][i] = G[1][i] = ac[i + 1];

    for (i = 0;;) {
        /* Calculate this iteration's reflection coefficient and error. */
        ref[i] = r = -G[1][0] / error;
        error += G[1][0] * r;

        if (++i >= P_MAX) return error;

        /* Update the generator matrix. Unlike Levinson-Durbin's summing of
         * reflection coefficients, this loop could be executed in parallel
         * by p processors in constant time.
         */
        for (m = 0; m < P_MAX - i; m++) {
            G[1][m] = G[1][m + 1] + r * G[0][m];
            G[0][m] = G[1][m + 1] * r + G[0][m];
        }
    }
}


 /* 0..7      IN/OUT */
static void rToLAR(int *r)
/*
 *  The following scaling for r[..] and LAR[..] has been used:
 *
 *  r[..]   = integer( real_r[..]*32768. ); -1 <= real_r < 1.
 *  LAR[..] = integer( real_LAR[..] * 16384 );
 *  with -1.625 <= real_LAR <= 1.625
 */
{
   int   temp;
   int   i;

   /* Computation of the LAR[0..7] from the r[0..7]
    */
   for (i = 0; i < 8; i++) {
      temp = abs(r[i]); //GSM_ABS(temp);
      if (temp < 22118) {
         temp >>= 1;
      } else if (temp < 31130) {
		 temp -= 11059;
      } else {
         temp -= 26112;
         temp <<= 2;
      }
      r[i] = r[i] < 0 ? -temp : temp;
   }
}

static void quantizeLARs(int *LARs, int *qLARs)
{	int qA[]={20480, 20480, 20480, 20480, 13964, 15360, 8534, 9036};
	int qB[]={0, 0, 2048, -2560, 94, -1792, -341, -1144};
	int qMin[]={-32, -32, -16, -16, -8, -8, -4, -4};
	int qMax[]={31, 31, 15, 15, 7, 7, 3, 3};
	int i;
	for (i = 0; i < 8; i++) {
		int temp;
		temp = (((LARs[i] * qA[i]) >> 15) + qB[i] + 256) >> 9;
		if (temp < qMin[i]) temp = 0;
		else if (temp > qMax[i]) temp = qMax[i] - qMin[i];
		else temp = temp - qMin[i];
		qLARs[i] = temp;
	}
}

void lpcAnalysis(gsm_state *state, short *in)
{	float error;
	float samples[160];
	float ac[9];
	float ref[8];
	int i;
	int * qLARs = state->qLARs;
	for (i = 0; i < 160; i++) {
		samples[i] = in[i];
	}
	autocorrelation(160, samples, 9, ac);
	error=schur(ac, ref);
	/*
	printf("reff:  ");
	for (i=0;i<8;i++) {
		printf("%5.5f  ", ref[i]);//((float)i_lpc[i])/32768.0f);
	}
	printf("\n");
	*/
	for (i = 0; i < 8; i++) {
		qLARs[i] = (int) (32768 * ref[i]);
	}
	rToLAR(qLARs);
	quantizeLARs(qLARs, qLARs);
}
