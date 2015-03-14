/*
 *  Off-the-Record Messaging library
 *  Copyright (C) 2004-2008  Ian Goldberg, Chris Alexander, Nikita Borisov
 *                           <otr@cypherpunks.ca>
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of version 2.1 of the GNU Lesser General
 *  Public License as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

/* system headers */
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h> 

/* libgcrypt headers */
#include <gcrypt.h>

/* libotr headers */
#include "sm.h"
#include "serial.h"

static const int SM_MSG1_LEN = 6;
static const int SM_MSG2_LEN = 11;
static const int SM_MSG3_LEN = 8;
static const int SM_MSG4_LEN = 3;

/* The modulus p */
static const char* SM_MODULUS_S = "0x"
    "FFFFFFFFFFFFFFFFC90FDAA22168C234C4C6628B80DC1CD1"
    "29024E088A67CC74020BBEA63B139B22514A08798E3404DD"
    "EF9519B3CD3A431B302B0A6DF25F14374FE1356D6D51C245"
    "E485B576625E7EC6F44C42E9A637ED6B0BFF5CB6F406B7ED"
    "EE386BFB5A899FA5AE9F24117C4B1FE649286651ECE45B3D"
    "C2007CB8A163BF0598DA48361C55D39A69163FA8FD24CF5F"
    "83655D23DCA3AD961C62F356208552BB9ED529077096966D"
    "670C354E4ABC9804F1746C08CA237327FFFFFFFFFFFFFFFF";
/* The order of the group q = (p-1)/2 */
static const char* SM_ORDER_S = "0x"
    "7FFFFFFFFFFFFFFFE487ED5110B4611A62633145C06E0E68"
    "948127044533E63A0105DF531D89CD9128A5043CC71A026E"
    "F7CA8CD9E69D218D98158536F92F8A1BA7F09AB6B6A8E122"
    "F242DABB312F3F637A262174D31BF6B585FFAE5B7A035BF6"
    "F71C35FDAD44CFD2D74F9208BE258FF324943328F6722D9E"
    "E1003E5C50B1DF82CC6D241B0E2AE9CD348B1FD47E9267AF"
    "C1B2AE91EE51D6CB0E3179AB1042A95DCF6A9483B84B4B36"
    "B3861AA7255E4C0278BA36046511B993FFFFFFFFFFFFFFFF";
static const char *SM_GENERATOR_S = "0x02";
static const int SM_MOD_LEN_BITS = 1536;
static const int SM_MOD_LEN_BYTES = 192;

static gcry_mpi_t SM_MODULUS = NULL;
static gcry_mpi_t SM_GENERATOR = NULL;
static gcry_mpi_t SM_ORDER = NULL;
static gcry_mpi_t SM_MODULUS_MINUS_2 = NULL;

/*
 * Call this once, at plugin load time.  It sets up the modulus and
 * generator MPIs.
 */
void otrl_sm_init(void)
{
    gcry_check_version(NULL);
    gcry_mpi_scan(&SM_MODULUS, GCRYMPI_FMT_HEX, SM_MODULUS_S, 0, NULL);
    gcry_mpi_scan(&SM_ORDER, GCRYMPI_FMT_HEX, SM_ORDER_S, 0, NULL);
    gcry_mpi_scan(&SM_GENERATOR, GCRYMPI_FMT_HEX, SM_GENERATOR_S,
	    0, NULL);
    SM_MODULUS_MINUS_2 = gcry_mpi_new(SM_MOD_LEN_BITS);
    gcry_mpi_sub_ui(SM_MODULUS_MINUS_2, SM_MODULUS, 2);
}

/*
 * Initialize the fields of a SM state.
 */
void otrl_sm_state_new(OtrlSMState *smst)
{
    smst->secret = NULL;
    smst->x2 = NULL;
    smst->x3 = NULL;
    smst->g1 = NULL;
    smst->g2 = NULL;
    smst->g3 = NULL;
    smst->g3o = NULL;
    smst->p = NULL;
    smst->q = NULL;
    smst->pab = NULL;
    smst->qab = NULL;
    smst->nextExpected = OTRL_SMP_EXPECT1;
    smst->received_question = 0;
    smst->sm_prog_state = OTRL_SMP_PROG_OK;
}

/*
 * Initialize the fields of a SM state.  Called the first time that
 * a user begins an SMP session.
 */
void otrl_sm_state_init(OtrlSMState *smst)
{
    otrl_sm_state_free(smst);
    smst->secret = gcry_mpi_new(SM_MOD_LEN_BITS);
    smst->x2 = NULL;
    smst->x3 = NULL;
    smst->g1 = gcry_mpi_copy(SM_GENERATOR);
    smst->g2 = gcry_mpi_new(SM_MOD_LEN_BITS);
    smst->g3 = gcry_mpi_new(SM_MOD_LEN_BITS);
    smst->g3o = gcry_mpi_new(SM_MOD_LEN_BITS);
    smst->p = gcry_mpi_new(SM_MOD_LEN_BITS);
    smst->q = gcry_mpi_new(SM_MOD_LEN_BITS);
    smst->pab = gcry_mpi_new(SM_MOD_LEN_BITS);
    smst->qab = gcry_mpi_new(SM_MOD_LEN_BITS);
    smst->received_question = 0;
    smst->sm_prog_state = OTRL_SMP_PROG_OK;
}

/*
 * Initialize the fields of a SM message1.
 * [0] = g2a, [1] = c2, [2] = d2, [3] = g3a, [4] = c3, [5] = d3
 */
void otrl_sm_msg1_init(gcry_mpi_t **msg1)
{
    gcry_mpi_t *msg = malloc(SM_MSG1_LEN * sizeof(gcry_mpi_t));
    msg[0] = gcry_mpi_new(SM_MOD_LEN_BITS);
    msg[1] = NULL;
    msg[2] = gcry_mpi_new(SM_MOD_LEN_BITS);
    msg[3] = gcry_mpi_new(SM_MOD_LEN_BITS);
    msg[4] = NULL;
    msg[5] = gcry_mpi_new(SM_MOD_LEN_BITS);

    *msg1 = msg;
}

/*
 * Initialize the fields of a SM message2.
 * [0] = g2b, [1] = c2, [2] = d2, [3] = g3b, [4] = c3, [5] = d3
 * [6] = pb, [7] = qb, [8] = cp, [9] = d5, [10] = d6
 */
void otrl_sm_msg2_init(gcry_mpi_t **msg2)
{
    gcry_mpi_t *msg = malloc(SM_MSG2_LEN * sizeof(gcry_mpi_t));
    msg[0] = gcry_mpi_new(SM_MOD_LEN_BITS);
    msg[1] = NULL;
    msg[2] = gcry_mpi_new(SM_MOD_LEN_BITS);
    msg[3] = gcry_mpi_new(SM_MOD_LEN_BITS);
    msg[4] = NULL;
    msg[5] = gcry_mpi_new(SM_MOD_LEN_BITS);
    msg[6] = gcry_mpi_new(SM_MOD_LEN_BITS);
    msg[7] = gcry_mpi_new(SM_MOD_LEN_BITS);
    msg[8] = NULL;
    msg[9] = gcry_mpi_new(SM_MOD_LEN_BITS);
    msg[10] = gcry_mpi_new(SM_MOD_LEN_BITS); 

    *msg2 = msg;
}

/*
 * Initialize the fields of a SM message3.
 * [0] = pa, [1] = qa, [2] = cp, [3] = d5, [4] = d6, [5] = ra,
 * [6] = cr, [7] = d7
 */
void otrl_sm_msg3_init(gcry_mpi_t **msg3)
{
    gcry_mpi_t *msg = malloc(SM_MSG3_LEN * sizeof(gcry_mpi_t));
    msg[0] = gcry_mpi_new(SM_MOD_LEN_BITS);
    msg[1] = gcry_mpi_new(SM_MOD_LEN_BITS);
    msg[2] = NULL;
    msg[3] = gcry_mpi_new(SM_MOD_LEN_BITS);
    msg[4] = gcry_mpi_new(SM_MOD_LEN_BITS);
    msg[5] = gcry_mpi_new(SM_MOD_LEN_BITS);
    msg[6] = NULL;
    msg[7] = gcry_mpi_new(SM_MOD_LEN_BITS);

    *msg3 = msg;
}

/*
 * Initialize the fields of a SM message4.
 * [0] = rb, [1] = cr, [2] = d7
 */
void otrl_sm_msg4_init(gcry_mpi_t **msg4)
{
    gcry_mpi_t *msg = malloc(SM_MSG4_LEN * sizeof(gcry_mpi_t));
    msg[0] = gcry_mpi_new(SM_MOD_LEN_BITS);
    msg[1] = NULL;
    msg[2] = gcry_mpi_new(SM_MOD_LEN_BITS);

    *msg4 = msg;
}

/*
 * Deallocate the contents of a OtrlSMState (but not the OtrlSMState
 * itself)
 */
void otrl_sm_state_free(OtrlSMState *smst)
{
    gcry_mpi_release(smst->secret);
    gcry_mpi_release(smst->x2);
    gcry_mpi_release(smst->x3);
    gcry_mpi_release(smst->g1);
    gcry_mpi_release(smst->g2);
    gcry_mpi_release(smst->g3);
    gcry_mpi_release(smst->g3o);
    gcry_mpi_release(smst->p);
    gcry_mpi_release(smst->q);
    gcry_mpi_release(smst->pab);
    gcry_mpi_release(smst->qab);
    otrl_sm_state_new(smst);
}

/*
 * Deallocate the contents of a message
 */
void otrl_sm_msg_free(gcry_mpi_t **message, int msglen)
{
    gcry_mpi_t *msg = *message;
    int i;
    for (i=0; i<msglen; i++) {
        gcry_mpi_release(msg[i]);
    }
    free(msg);
    *message = NULL;
}

static gcry_mpi_t randomExponent(void)
{
    unsigned char *secbuf = NULL;
    gcry_mpi_t randexpon = NULL;

    /* Generate a random exponent */
    secbuf = gcry_random_bytes_secure(SM_MOD_LEN_BYTES, GCRY_STRONG_RANDOM);
    gcry_mpi_scan(&randexpon, GCRYMPI_FMT_USG, secbuf, SM_MOD_LEN_BYTES, NULL);
    gcry_free(secbuf);

    return randexpon;
}

/*
 * Hash one or two mpis.  To hash only one mpi, b may be set to NULL.
 */
static gcry_error_t otrl_sm_hash(gcry_mpi_t* hash, int version,
	const gcry_mpi_t a, const gcry_mpi_t b)
{
    unsigned char* input;
    unsigned char output[SM_DIGEST_SIZE];
    size_t sizea;
    size_t sizeb;
    size_t totalsize;
    unsigned char* dataa;
    unsigned char* datab;
    
    gcry_mpi_aprint(GCRYMPI_FMT_USG, &dataa, &sizea, a);
    totalsize = 1 + 4 + sizea;
    if (b) {
	gcry_mpi_aprint(GCRYMPI_FMT_USG, &datab, &sizeb, b);
	totalsize += 4 + sizeb;
    } else {
	sizeb = 0;
    }

    input = malloc(totalsize);
    input[0] = (unsigned char)version;
    input[1] = (unsigned char)((sizea >> 24) & 0xFF);
    input[2] = (unsigned char)((sizea >> 16) & 0xFF);
    input[3] = (unsigned char)((sizea >> 8) & 0xFF);
    input[4] = (unsigned char)(sizea & 0xFF);
    memmove(input + 5, dataa, sizea);
    if (b) {
	input[5 + sizea] = (unsigned char)((sizeb >> 24) & 0xFF);
	input[6 + sizea] = (unsigned char)((sizeb >> 16) & 0xFF);
	input[7 + sizea] = (unsigned char)((sizeb >> 8) & 0xFF);
	input[8 + sizea] = (unsigned char)(sizeb & 0xFF);
	memmove(input + 9 + sizea, datab, sizeb);
    }

    gcry_md_hash_buffer(SM_HASH_ALGORITHM, output, input, totalsize);
    gcry_mpi_scan(hash, GCRYMPI_FMT_USG, output, SM_DIGEST_SIZE, NULL);
    free(input);
    input = NULL;

    /* free memory */
    gcry_free(dataa);
    if (b) gcry_free(datab);

    return gcry_error(GPG_ERR_NO_ERROR);
}

/* This method should be passed a pointer to an uninitialized buffer,
 * and a list of mpis with a list length.  When returns, the buffer will
 * point to newly-allocated memory (using malloc) containing a
 * reversible serialization. */
static gcry_error_t serialize_mpi_array(unsigned char **buffer, int *buflen,
	unsigned int count, gcry_mpi_t *mpis)
{
    size_t totalsize = 0, lenp, nextsize;
    unsigned int i, j;
    size_t *list_sizes = malloc(count * sizeof(size_t));
    unsigned char **tempbuffer = malloc(count * sizeof(unsigned char *));
    unsigned char *bufp;

    for (i=0; i<count; i++) {
        gcry_mpi_aprint(GCRYMPI_FMT_USG, &(tempbuffer[i]), &(list_sizes[i]),
		mpis[i]);
        totalsize += list_sizes[i];
    }

    *buflen = (count+1)*4 + totalsize;
    *buffer = malloc(*buflen * sizeof(char));

    bufp = *buffer;
    lenp = totalsize;

    write_int(count);
    for(i=0; i<count; i++)
    {
        nextsize = list_sizes[i];
        write_int(nextsize);
        
        for(j=0; j<nextsize; j++)
            bufp[j] = tempbuffer[i][j];
        
	bufp += nextsize;
        lenp -= nextsize;
        gcry_free(tempbuffer[i]);
    }
    free(tempbuffer);
    free(list_sizes);

    return gcry_error(GPG_ERR_NO_ERROR);
}

/* Takes a buffer containing serialized and concatenated mpis
 * and converts it to an array of gcry_mpi_t structs.
 * The buffer is assumed to consist of a 4-byte int containing the
 * number of mpis in the array, followed by {size, data} pairs for
 * each mpi.  If malformed, method returns GCRY_ERROR_INV_VALUE */
static gcry_error_t unserialize_mpi_array(gcry_mpi_t **mpis,
	unsigned int expcount, const unsigned char *buffer, const int buflen)
{
    int i;
    int lenp = buflen;
    unsigned int thecount = 0;
    const unsigned char* bufp = buffer;
    *mpis = NULL;

    read_int(thecount);
    if (thecount != expcount) goto invval;

    *mpis = malloc(thecount * sizeof(gcry_mpi_t));

    for (i=0; i<thecount; i++) {
	(*mpis)[i] = NULL;
    }

    for (i=0; i<thecount; i++) {
	read_mpi((*mpis)[i]);
    }

    return gcry_error(GPG_ERR_NO_ERROR);

invval:
    if (*mpis) {
	for (i=0; i<thecount; i++) {
	    gcry_mpi_release((*mpis)[i]);
	}
	free(*mpis);
	*mpis = NULL;
    }
    return gcry_error(GPG_ERR_INV_VALUE);
}

/* Check that an MPI is in the right range to be a (non-unit) group
 * element */
static int check_group_elem(gcry_mpi_t g)
{
    if (gcry_mpi_cmp_ui(g, 2) < 0 ||
	    gcry_mpi_cmp(g, SM_MODULUS_MINUS_2) > 0) {
	return 1;
    }
    return 0;
}

/* Check that an MPI is in the right range to be a (non-zero) exponent */
static int check_expon(gcry_mpi_t x)
{
    if (gcry_mpi_cmp_ui(x, 1) < 0 ||
	    gcry_mpi_cmp(x, SM_ORDER) >= 0) {
	return 1;
    }
    return 0;
}

/*
 * Proof of knowledge of a discrete logarithm
 */
static gcry_error_t otrl_sm_proof_know_log(gcry_mpi_t *c, gcry_mpi_t *d, const gcry_mpi_t g, const gcry_mpi_t x, int version)
{
    gcry_mpi_t r = randomExponent();
    gcry_mpi_t temp = gcry_mpi_new(SM_MOD_LEN_BITS);
    gcry_mpi_powm(temp, g, r, SM_MODULUS);
    otrl_sm_hash(c, version, temp, NULL);
    gcry_mpi_mulm(temp, x, *c, SM_ORDER);
    gcry_mpi_subm(*d, r, temp, SM_ORDER);
    gcry_mpi_release(temp);
    gcry_mpi_release(r);

    return gcry_error(GPG_ERR_NO_ERROR);
}

/*
 * Verify a proof of knowledge of a discrete logarithm.  Checks that c = h(g^d x^c)
 */
static int otrl_sm_check_know_log(const gcry_mpi_t c, const gcry_mpi_t d, const gcry_mpi_t g, const gcry_mpi_t x, int version)
{
    int comp;

    gcry_mpi_t gd = gcry_mpi_new(SM_MOD_LEN_BITS);  /* g^d */
    gcry_mpi_t xc = gcry_mpi_new(SM_MOD_LEN_BITS);  /* x^c */
    gcry_mpi_t gdxc = gcry_mpi_new(SM_MOD_LEN_BITS);   /* (g^d x^c) */
    gcry_mpi_t hgdxc = NULL;   /* h(g^d x^c) */

    gcry_mpi_powm(gd, g, d, SM_MODULUS);
    gcry_mpi_powm(xc, x, c, SM_MODULUS);
    gcry_mpi_mulm(gdxc, gd, xc, SM_MODULUS);
    otrl_sm_hash(&hgdxc, version, gdxc, NULL);
    
    comp = gcry_mpi_cmp(hgdxc, c);
    gcry_mpi_release(gd);
    gcry_mpi_release(xc);
    gcry_mpi_release(gdxc);
    gcry_mpi_release(hgdxc);

    return comp;
}

/*
 * Proof of knowledge of coordinates with first components being equal
 */
static gcry_error_t otrl_sm_proof_equal_coords(gcry_mpi_t *c, gcry_mpi_t *d1, gcry_mpi_t *d2, const OtrlSMState *state, const gcry_mpi_t r, int version)
{
    gcry_mpi_t r1 = randomExponent();
    gcry_mpi_t r2 = randomExponent();
    gcry_mpi_t temp1 = gcry_mpi_new(SM_MOD_LEN_BITS);
    gcry_mpi_t temp2 = gcry_mpi_new(SM_MOD_LEN_BITS);

    /* Compute the value of c, as c = h(g3^r1, g1^r1 g2^r2) */
    gcry_mpi_powm(temp1, state->g1, r1, SM_MODULUS);
    gcry_mpi_powm(temp2, state->g2, r2, SM_MODULUS);
    gcry_mpi_mulm(temp2, temp1, temp2, SM_MODULUS);
    gcry_mpi_powm(temp1, state->g3, r1, SM_MODULUS);
    otrl_sm_hash(c, version, temp1, temp2);

    /* Compute the d values, as d1 = r1 - r c, d2 = r2 - secret c */
    gcry_mpi_mulm(temp1, r, *c, SM_ORDER);
    gcry_mpi_subm(*d1, r1, temp1, SM_ORDER);

    gcry_mpi_mulm(temp1, state->secret, *c, SM_ORDER);
    gcry_mpi_subm(*d2, r2, temp1, SM_ORDER);

    /* All clear */
    gcry_mpi_release(r1);
    gcry_mpi_release(r2);
    gcry_mpi_release(temp1);
    gcry_mpi_release(temp2);
    return gcry_error(GPG_ERR_NO_ERROR);
}

/*
 * Verify a proof of knowledge of coordinates with first components being equal
 */
static gcry_error_t otrl_sm_check_equal_coords(const gcry_mpi_t c, const gcry_mpi_t d1, const gcry_mpi_t d2, const gcry_mpi_t p, const gcry_mpi_t q, const OtrlSMState *state, int version)
{
    int comp;

    gcry_mpi_t temp1 = gcry_mpi_new(SM_MOD_LEN_BITS);
    gcry_mpi_t temp2 = gcry_mpi_new(SM_MOD_LEN_BITS);
    gcry_mpi_t temp3 = gcry_mpi_new(SM_MOD_LEN_BITS);
    gcry_mpi_t cprime = NULL;

    /* To verify, we test that hash(g3^d1 * p^c, g1^d1 * g2^d2 * q^c) = c
     * If indeed c = hash(g3^r1, g1^r1 g2^r2), d1 = r1 - r*c,
     * d2 = r2 - secret*c.  And if indeed p = g3^r, q = g1^r * g2^secret
     * Then we should have that:
     *   hash(g3^d1 * p^c, g1^d1 * g2^d2 * q^c)
     * = hash(g3^(r1 - r*c + r*c), g1^(r1 - r*c + q*c) *
     *      g2^(r2 - secret*c + secret*c))
     * = hash(g3^r1, g1^r1 g2^r2)
     * = c
     */
    gcry_mpi_powm(temp2, state->g3, d1, SM_MODULUS);
    gcry_mpi_powm(temp3, p, c, SM_MODULUS);
    gcry_mpi_mulm(temp1, temp2, temp3, SM_MODULUS);

    gcry_mpi_powm(temp2, state->g1, d1, SM_MODULUS);
    gcry_mpi_powm(temp3, state->g2, d2, SM_MODULUS);
    gcry_mpi_mulm(temp2, temp2, temp3, SM_MODULUS);
    gcry_mpi_powm(temp3, q, c, SM_MODULUS);
    gcry_mpi_mulm(temp2, temp3, temp2, SM_MODULUS);

    otrl_sm_hash(&cprime, version, temp1, temp2);

    comp = gcry_mpi_cmp(c, cprime);
    gcry_mpi_release(temp1);
    gcry_mpi_release(temp2);
    gcry_mpi_release(temp3);
    gcry_mpi_release(cprime);

    return comp;
}

/*
 * Proof of knowledge of logs with exponents being equal
 */
static gcry_error_t otrl_sm_proof_equal_logs(gcry_mpi_t *c, gcry_mpi_t *d, OtrlSMState *state, int version)
{
    gcry_mpi_t r = randomExponent();
    gcry_mpi_t temp1 = gcry_mpi_new(SM_MOD_LEN_BITS);
    gcry_mpi_t temp2 = gcry_mpi_new(SM_MOD_LEN_BITS);

    /* Compute the value of c, as c = h(g1^r, (Qa/Qb)^r) */
    gcry_mpi_powm(temp1, state->g1, r, SM_MODULUS);
    gcry_mpi_powm(temp2, state->qab, r, SM_MODULUS);
    otrl_sm_hash(c, version, temp1, temp2);

    /* Compute the d values, as d = r - x3 c */
    gcry_mpi_mulm(temp1, state->x3, *c, SM_ORDER);
    gcry_mpi_subm(*d, r, temp1, SM_ORDER);

    /* All clear */
    gcry_mpi_release(r);
    gcry_mpi_release(temp1);
    gcry_mpi_release(temp2);
    return gcry_error(GPG_ERR_NO_ERROR);
}

/*
 * Verify a proof of knowledge of logs with exponents being equal
 */
static gcry_error_t otrl_sm_check_equal_logs(const gcry_mpi_t c, const gcry_mpi_t d, const gcry_mpi_t r, const OtrlSMState *state, int version)
{
    int comp;

    gcry_mpi_t temp1 = gcry_mpi_new(SM_MOD_LEN_BITS);
    gcry_mpi_t temp2 = gcry_mpi_new(SM_MOD_LEN_BITS);
    gcry_mpi_t temp3 = gcry_mpi_new(SM_MOD_LEN_BITS);
    gcry_mpi_t cprime = NULL;

    /* Here, we recall the exponents used to create g3.
     * If we have previously seen g3o = g1^x where x is unknown
     * during the DH exchange to produce g3, then we may proceed with:
     * 
     * To verify, we test that hash(g1^d * g3o^c, qab^d * r^c) = c
     * If indeed c = hash(g1^r1, qab^r1), d = r1- x * c
     * And if indeed r = qab^x
     * Then we should have that:
     *   hash(g1^d * g3o^c, qab^d r^c)
     * = hash(g1^(r1 - x*c + x*c), qab^(r1 - x*c + x*c))
     * = hash(g1^r1, qab^r1)
     * = c
     */
    gcry_mpi_powm(temp2, state->g1, d, SM_MODULUS);
    gcry_mpi_powm(temp3, state->g3o, c, SM_MODULUS);
    gcry_mpi_mulm(temp1, temp2, temp3, SM_MODULUS);

    gcry_mpi_powm(temp3, state->qab, d, SM_MODULUS);
    gcry_mpi_powm(temp2, r, c, SM_MODULUS);
    gcry_mpi_mulm(temp2, temp3, temp2, SM_MODULUS);

    otrl_sm_hash(&cprime, version, temp1, temp2);

    comp = gcry_mpi_cmp(c, cprime);
    gcry_mpi_release(temp1);
    gcry_mpi_release(temp2);
    gcry_mpi_release(temp3);
    gcry_mpi_release(cprime);

    return comp;
}

/* Create first message in SMP exchange.  Input is Alice's secret value
 * which this protocol aims to compare to Bob's.  Output is a serialized
 * mpi array whose elements correspond to the following:
 * [0] = g2a, Alice's half of DH exchange to determine g2
 * [1] = c2, [2] = d2, Alice's ZK proof of knowledge of g2a exponent
 * [3] = g3a, Alice's half of DH exchange to determine g3
 * [4] = c3, [5] = d3, Alice's ZK proof of knowledge of g3a exponent */
gcry_error_t otrl_sm_step1(OtrlSMAliceState *astate,
	const unsigned char* secret, int secretlen,
	unsigned char** output, int* outputlen)
{
    /* Initialize the sm state or update the secret */
    gcry_mpi_t secret_mpi = NULL;
    gcry_mpi_t *msg1;

    *output = NULL;
    *outputlen = 0;
    
    gcry_mpi_scan(&secret_mpi, GCRYMPI_FMT_USG, secret, secretlen, NULL);

    if (! astate->g1) {
        otrl_sm_state_init(astate);
    }
    gcry_mpi_set(astate->secret, secret_mpi);
    gcry_mpi_release(secret_mpi);
    astate->received_question = 0;

    otrl_sm_msg1_init(&msg1);

    astate->x2 = randomExponent();
    astate->x3 = randomExponent();

    gcry_mpi_powm(msg1[0], astate->g1, astate->x2, SM_MODULUS);
    otrl_sm_proof_know_log(&(msg1[1]), &(msg1[2]), astate->g1, astate->x2, 1);

    gcry_mpi_powm(msg1[3], astate->g1, astate->x3, SM_MODULUS);
    otrl_sm_proof_know_log(&(msg1[4]), &(msg1[5]), astate->g1, astate->x3, 2);

    serialize_mpi_array(output, outputlen, SM_MSG1_LEN, msg1);
    otrl_sm_msg_free(&msg1, SM_MSG1_LEN);
    astate->sm_prog_state = OTRL_SMP_PROG_OK;
    return gcry_error(GPG_ERR_NO_ERROR);
}

/* Receive the first message in SMP exchange, which was generated by
 * otrl_sm_step1.  Input is saved until the user inputs their secret
 * information.  No output. */
gcry_error_t otrl_sm_step2a(OtrlSMBobState *bstate, const unsigned char* input, const int inputlen, int received_question)
{
    gcry_mpi_t *msg1;
    gcry_error_t err;

    /* Initialize the sm state if needed */
    if (! bstate->g1) {
	otrl_sm_state_init(bstate);
    }
    bstate->received_question = received_question;
    bstate->sm_prog_state = OTRL_SMP_PROG_CHEATED;

    /* Read from input to find the mpis */
    err = unserialize_mpi_array(&msg1, SM_MSG1_LEN, input, inputlen);
    
    if (err != gcry_error(GPG_ERR_NO_ERROR)) return err;

    if (check_group_elem(msg1[0]) || check_expon(msg1[2]) ||
	    check_group_elem(msg1[3]) || check_expon(msg1[5])) {
        return gcry_error(GPG_ERR_INV_VALUE);
    }

    /* Store Alice's g3a value for later in the protocol */
    gcry_mpi_set(bstate->g3o, msg1[3]);
    
    /* Verify Alice's proofs */
    if (otrl_sm_check_know_log(msg1[1], msg1[2], bstate->g1, msg1[0], 1) || 
        otrl_sm_check_know_log(msg1[4], msg1[5], bstate->g1, msg1[3], 2)) {
        return gcry_error(GPG_ERR_INV_VALUE);
    }

    /* Create Bob's half of the generators g2 and g3 */
    bstate->x2 = randomExponent();
    bstate->x3 = randomExponent();

    /* Combine the two halves from Bob and Alice and determine g2 and g3 */
    gcry_mpi_powm(bstate->g2, msg1[0], bstate->x2, SM_MODULUS);
    gcry_mpi_powm(bstate->g3, msg1[3], bstate->x3, SM_MODULUS);

    bstate->sm_prog_state = OTRL_SMP_PROG_OK;
    return gcry_error(GPG_ERR_NO_ERROR);
}

/* Create second message in SMP exchange.  Input is Bob's secret value.
 * Information from earlier steps in the exchange is taken from Bob's
 * state.  Output is a serialized mpi array whose elements correspond
 * to the following:
 * [0] = g2b, Bob's half of DH exchange to determine g2
 * [1] = c2, [2] = d2, Bob's ZK proof of knowledge of g2b exponent
 * [3] = g3b, Bob's half of DH exchange to determine g3
 * [4] = c3, [5] = d3, Bob's ZK proof of knowledge of g3b exponent
 * [6] = pb, [7] = qb, Bob's halves of the (Pa/Pb) and (Qa/Qb) values
 * [8] = cp, [9] = d5, [10] = d6, Bob's ZK proof that pb, qb formed correctly */
gcry_error_t otrl_sm_step2b(OtrlSMBobState *bstate, const unsigned char* secret, int secretlen, unsigned char **output, int* outputlen)
{
    /* Convert the given secret to the proper form and store it */
    gcry_mpi_t r, qb1, qb2;
    gcry_mpi_t *msg2;
    gcry_mpi_t secret_mpi = NULL;

    *output = NULL;
    *outputlen = 0;
    
    gcry_mpi_scan(&secret_mpi, GCRYMPI_FMT_USG, secret, secretlen, NULL);
    gcry_mpi_set(bstate->secret, secret_mpi);
    gcry_mpi_release(secret_mpi);

    otrl_sm_msg2_init(&msg2);

    gcry_mpi_powm(msg2[0], bstate->g1, bstate->x2, SM_MODULUS);
    otrl_sm_proof_know_log(&(msg2[1]), &(msg2[2]), bstate->g1, bstate->x2, 3);

    gcry_mpi_powm(msg2[3], bstate->g1, bstate->x3, SM_MODULUS);
    otrl_sm_proof_know_log(&(msg2[4]), &(msg2[5]), bstate->g1, bstate->x3, 4);

    /* Calculate P and Q values for Bob */
    r = randomExponent();
    qb1 = gcry_mpi_new(SM_MOD_LEN_BITS);
    qb2 = gcry_mpi_new(SM_MOD_LEN_BITS);
    gcry_mpi_powm(bstate->p, bstate->g3, r, SM_MODULUS);
    gcry_mpi_set(msg2[6], bstate->p);
    gcry_mpi_powm(qb1, bstate->g1, r, SM_MODULUS);
    gcry_mpi_powm(qb2, bstate->g2, bstate->secret, SM_MODULUS);
    gcry_mpi_mulm(bstate->q, qb1, qb2, SM_MODULUS);
    gcry_mpi_set(msg2[7], bstate->q);

    otrl_sm_proof_equal_coords(&(msg2[8]), &(msg2[9]), &(msg2[10]), bstate, r, 5);

    /* Convert to serialized form */
    serialize_mpi_array(output, outputlen, SM_MSG2_LEN, msg2);

    /* Free up memory for unserialized and intermediate values */
    gcry_mpi_release(r);
    gcry_mpi_release(qb1);
    gcry_mpi_release(qb2);
    otrl_sm_msg_free(&msg2, SM_MSG2_LEN);

    return gcry_error(GPG_ERR_NO_ERROR);
}

/* Create third message in SMP exchange.  Input is a message generated
 * by otrl_sm_step2b. Output is a serialized mpi array whose elements
 * correspond to the following:
 * [0] = pa, [1] = qa, Alice's halves of the (Pa/Pb) and (Qa/Qb) values
 * [2] = cp, [3] = d5, [4] = d6, Alice's ZK proof that pa, qa formed correctly
 * [5] = ra, calculated as (Qa/Qb)^x3 where x3 is the exponent used in g3a
 * [6] = cr, [7] = d7, Alice's ZK proof that ra is formed correctly */
gcry_error_t otrl_sm_step3(OtrlSMAliceState *astate, const unsigned char* input, const int inputlen, unsigned char **output, int* outputlen)
{
    /* Read from input to find the mpis */
    gcry_mpi_t r, qa1, qa2, inv;
    gcry_mpi_t *msg2;
    gcry_mpi_t *msg3;
    gcry_error_t err;
    
    *output = NULL;
    *outputlen = 0;
    astate->sm_prog_state = OTRL_SMP_PROG_CHEATED;
    
    err = unserialize_mpi_array(&msg2, SM_MSG2_LEN, input, inputlen);
    if (err != gcry_error(GPG_ERR_NO_ERROR)) return err;

    if (check_group_elem(msg2[0]) || check_group_elem(msg2[3]) ||
	    check_group_elem(msg2[6]) || check_group_elem(msg2[7]) ||
	    check_expon(msg2[2]) || check_expon(msg2[5]) ||
	    check_expon(msg2[9]) || check_expon(msg2[10])) {
        return gcry_error(GPG_ERR_INV_VALUE);
    }

    otrl_sm_msg3_init(&msg3);

    /* Store Bob's g3a value for later in the protocol */
    gcry_mpi_set(astate->g3o, msg2[3]);

    /* Verify Bob's knowledge of discreet log proofs */
    if (otrl_sm_check_know_log(msg2[1], msg2[2], astate->g1, msg2[0], 3) || 
        otrl_sm_check_know_log(msg2[4], msg2[5], astate->g1, msg2[3], 4)) {
        return gcry_error(GPG_ERR_INV_VALUE);
    }

    /* Combine the two halves from Bob and Alice and determine g2 and g3 */
    gcry_mpi_powm(astate->g2, msg2[0], astate->x2, SM_MODULUS);
    gcry_mpi_powm(astate->g3, msg2[3], astate->x3, SM_MODULUS);

    /* Verify Bob's coordinate equality proof */
    if (otrl_sm_check_equal_coords(msg2[8], msg2[9], msg2[10], msg2[6], msg2[7], astate, 5))
        return gcry_error(GPG_ERR_INV_VALUE);

    /* Calculate P and Q values for Alice */
    r = randomExponent();
    qa1 = gcry_mpi_new(SM_MOD_LEN_BITS);
    qa2 = gcry_mpi_new(SM_MOD_LEN_BITS);
    gcry_mpi_powm(astate->p, astate->g3, r, SM_MODULUS);
    gcry_mpi_set(msg3[0], astate->p);
    gcry_mpi_powm(qa1, astate->g1, r, SM_MODULUS);
    gcry_mpi_powm(qa2, astate->g2, astate->secret, SM_MODULUS);
    gcry_mpi_mulm(astate->q, qa1, qa2, SM_MODULUS);
    gcry_mpi_set(msg3[1], astate->q);

    otrl_sm_proof_equal_coords(&(msg3[2]), &(msg3[3]), &(msg3[4]), astate, r, 6);

    /* Calculate Ra and proof */
    inv = gcry_mpi_new(SM_MOD_LEN_BITS);
    gcry_mpi_invm(inv, msg2[6], SM_MODULUS);
    gcry_mpi_mulm(astate->pab, astate->p, inv, SM_MODULUS);
    gcry_mpi_invm(inv, msg2[7], SM_MODULUS);
    gcry_mpi_mulm(astate->qab, astate->q, inv, SM_MODULUS);
    gcry_mpi_powm(msg3[5], astate->qab, astate->x3, SM_MODULUS);
    otrl_sm_proof_equal_logs(&(msg3[6]), &(msg3[7]), astate, 7);

    serialize_mpi_array(output, outputlen, SM_MSG3_LEN, msg3);
    otrl_sm_msg_free(&msg2, SM_MSG2_LEN);
    otrl_sm_msg_free(&msg3, SM_MSG3_LEN);

    gcry_mpi_release(r);
    gcry_mpi_release(qa1);
    gcry_mpi_release(qa2);
    gcry_mpi_release(inv);

    astate->sm_prog_state = OTRL_SMP_PROG_OK;
    return gcry_error(GPG_ERR_NO_ERROR);
}

/* Create final message in SMP exchange.  Input is a message generated
 * by otrl_sm_step3. Output is a serialized mpi array whose elements
 * correspond to the following:
 * [0] = rb, calculated as (Qa/Qb)^x3 where x3 is the exponent used in g3b
 * [1] = cr, [2] = d7, Bob's ZK proof that rb is formed correctly
 * This method also checks if Alice and Bob's secrets were the same.  If
 * so, it returns NO_ERROR.  If the secrets differ, an INV_VALUE error is
 * returned instead. */
gcry_error_t otrl_sm_step4(OtrlSMBobState *bstate, const unsigned char* input, const int inputlen, unsigned char **output, int* outputlen)
{
    /* Read from input to find the mpis */
    int comp;
    gcry_mpi_t inv, rab;
    gcry_mpi_t *msg3;
    gcry_mpi_t *msg4;
    gcry_error_t err;
    err = unserialize_mpi_array(&msg3, SM_MSG3_LEN, input, inputlen);

    *output = NULL;
    *outputlen = 0;
    bstate->sm_prog_state = OTRL_SMP_PROG_CHEATED;
    
    if (err != gcry_error(GPG_ERR_NO_ERROR)) return err;

    otrl_sm_msg4_init(&msg4);

    if (check_group_elem(msg3[0]) || check_group_elem(msg3[1]) ||
	    check_group_elem(msg3[5]) || check_expon(msg3[3]) ||
	    check_expon(msg3[4]) || check_expon(msg3[7]))  {
        return gcry_error(GPG_ERR_INV_VALUE);
    }

    /* Verify Alice's coordinate equality proof */
    if (otrl_sm_check_equal_coords(msg3[2], msg3[3], msg3[4], msg3[0], msg3[1], bstate, 6))
        return gcry_error(GPG_ERR_INV_VALUE);

    /* Find Pa/Pb and Qa/Qb */
    inv = gcry_mpi_new(SM_MOD_LEN_BITS);
    gcry_mpi_invm(inv, bstate->p, SM_MODULUS);
    gcry_mpi_mulm(bstate->pab, msg3[0], inv, SM_MODULUS);
    gcry_mpi_invm(inv, bstate->q, SM_MODULUS);
    gcry_mpi_mulm(bstate->qab, msg3[1], inv, SM_MODULUS);

    /* Verify Alice's log equality proof */
    if (otrl_sm_check_equal_logs(msg3[6], msg3[7], msg3[5], bstate, 7))
        return gcry_error(GPG_ERR_INV_VALUE);

    /* Calculate Rb and proof */
    gcry_mpi_powm(msg4[0], bstate->qab, bstate->x3, SM_MODULUS);
    otrl_sm_proof_equal_logs(&(msg4[1]), &(msg4[2]), bstate, 8);

    serialize_mpi_array(output, outputlen, SM_MSG4_LEN, msg4);

    /* Calculate Rab and verify that secrets match */
    rab = gcry_mpi_new(SM_MOD_LEN_BITS);
    gcry_mpi_powm(rab, msg3[5], bstate->x3, SM_MODULUS);
    comp = gcry_mpi_cmp(rab, bstate->pab);

    /* Clean up everything allocated in this step */
    otrl_sm_msg_free(&msg3, SM_MSG3_LEN);
    otrl_sm_msg_free(&msg4, SM_MSG4_LEN);
    gcry_mpi_release(rab);
    gcry_mpi_release(inv);

    bstate->sm_prog_state = comp ? OTRL_SMP_PROG_FAILED :
	OTRL_SMP_PROG_SUCCEEDED;

    if (comp)
        return gcry_error(GPG_ERR_INV_VALUE);
    else
        return gcry_error(GPG_ERR_NO_ERROR);
}

/* Receives the final SMP message, which was generated in otrl_sm_step.
 * This method checks if Alice and Bob's secrets were the same.  If
 * so, it returns NO_ERROR.  If the secrets differ, an INV_VALUE error is
 * returned instead. */
gcry_error_t otrl_sm_step5(OtrlSMAliceState *astate, const unsigned char* input, const int inputlen)
{
    /* Read from input to find the mpis */
    int comp;
    gcry_mpi_t rab;
    gcry_mpi_t *msg4;
    gcry_error_t err;
    err = unserialize_mpi_array(&msg4, SM_MSG4_LEN, input, inputlen);
    astate->sm_prog_state = OTRL_SMP_PROG_CHEATED;
    
    if (err != gcry_error(GPG_ERR_NO_ERROR)) return err;

    if (check_group_elem(msg4[0]) || check_expon(msg4[2])) {
        return gcry_error(GPG_ERR_INV_VALUE);
    }

    /* Verify Bob's log equality proof */
    if (otrl_sm_check_equal_logs(msg4[1], msg4[2], msg4[0], astate, 8))
        return gcry_error(GPG_ERR_INV_VALUE);

    /* Calculate Rab and verify that secrets match */
    rab = gcry_mpi_new(SM_MOD_LEN_BITS);
    gcry_mpi_powm(rab, msg4[0], astate->x3, SM_MODULUS);

    comp = gcry_mpi_cmp(rab, astate->pab);
    gcry_mpi_release(rab);
    otrl_sm_msg_free(&msg4, SM_MSG4_LEN);

    astate->sm_prog_state = comp ? OTRL_SMP_PROG_FAILED :
	OTRL_SMP_PROG_SUCCEEDED;

    if (comp)
        return gcry_error(GPG_ERR_INV_VALUE);
    else
        return gcry_error(GPG_ERR_NO_ERROR);
}
