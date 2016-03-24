/*  
 * Skype Login
 * 
 * Based on:
 *   FakeSkype : Skype reverse engineering proof-of-concept client
 *               Ouanilo MEDEGAN (c) 2006   http://www.oklabs.net
 *   pyskype   : Skype login Python script by uunicorn
 *
 * Written by:   leecher@dose.0wnz.at (c) 2015 
 *
 * Module:       RC4 wrapped communication layer
 *
 */
#include "common.h"
#include "rc4comm.h"
#ifdef USE_RC4
#ifdef _WIN32
#define alloca _alloca
#endif

static char	*SkypeDHModulus348 = "ffffffffffffffffc90fdaa22168c234c4c6628b80dc1cd129024e088a67cc74020bbea63b13b202ffffffffffffffff";

int RC4Comm_Init(LSConnection *pConn)
{
	DH		*dh;
	char    pub_key[96], sec_key[48], ls_keybuf[1024];
	BIGNUM  *ls_key;
	int		ret = 0;

	if (!(dh = DH_new())) return -1;
	BN_hex2bn(&dh->g, "2");
	BN_hex2bn(&dh->p, SkypeDHModulus348);
	DH_generate_key(dh);
	if (send(pConn->LSSock, pub_key, BN_bn2bin(dh->pub_key, pub_key), 0)>0)
	{
		if (recv(pConn->LSSock, ls_keybuf, sizeof(ls_keybuf), 0)>0)
		{
			ls_key = BN_bin2bn(ls_keybuf, 48, NULL);
			if (DH_compute_key(sec_key, ls_key, dh)<=0) ret = -3; else
			{
				MD5_CTX	Context;
				uchar BufHash[MD5_DIGEST_LENGTH];

				MD5_Init(&Context);
				MD5_Update(&Context, "O", 1);
				MD5_Update(&Context, sec_key, 48);
				MD5_Final(BufHash, &Context);
				if (send(pConn->LSSock, BufHash, 8, 0)>0)
				{
					/* Setup RC4 */
					RC4_set_key(&pConn->rc4_send, 48, sec_key);
					if (sec_key[0] == 0xFF) {pInst->pfLog(pInst->pLogStream, "First byte overflow, negotiation failed?");}
					sec_key[0]++;
					RC4_set_key(&pConn->rc4_recv, 48, sec_key);
				} else
					ret=-1;
			}
			BN_free(ls_key);
		} else 
			ret=-1;
	} else 
		ret=-1;

	DH_free(dh);
	return ret;
}

int RC4Comm_Send (LSConnection *pConn, const char * buf, int len)
{
	char *out = alloca(len);

	RC4(&pConn->rc4_send, len, buf, out);
	return send(pConn->LSSock, out, len, 0);
}

int RC4Comm_Recv (LSConnection *pConn, char * buf, int len)
{
	int ret = recv(pConn->LSSock, buf, len, 0);
	if (ret > 0) RC4(&pConn->rc4_recv, ret, buf, buf);
	return ret;
}

#endif
