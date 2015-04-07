#ifndef __LOADLIB_H__
#define __LOADLIB_H__

BOOL loadlib(void);
void freelib();

typedef HANDLE (__cdecl * _cpp_create_context) (int);
typedef void   (__cdecl * _cpp_delete_context) (HANDLE);
typedef void   (__cdecl * _cpp_reset_context) (HANDLE);
typedef LPSTR  (__cdecl * _cpp_init_keya) (HANDLE,int);
typedef int    (__cdecl * _cpp_init_keyb) (HANDLE,LPCSTR);
typedef int    (__cdecl * _cpp_calc_keyx) (HANDLE);
typedef int    (__cdecl * _cpp_init_keyp) (HANDLE,LPCSTR);
typedef LPSTR  (__cdecl * _cpp_encodeA) (HANDLE,LPCSTR);
typedef LPSTR  (__cdecl * _cpp_encodeW) (HANDLE,LPWSTR);
typedef LPSTR  (__cdecl * _cpp_encodeU) (HANDLE,LPCSTR);
typedef LPSTR  (__cdecl * _cpp_decode) (HANDLE,LPCSTR);
typedef LPSTR  (__cdecl * _cpp_decodeU) (HANDLE,LPCSTR);
typedef int    (__cdecl * _cpp_encrypt_file) (HANDLE,LPCSTR,LPCSTR);
typedef int    (__cdecl * _cpp_decrypt_file) (HANDLE,LPCSTR,LPCSTR);
typedef int    (__cdecl * _cpp_get_features) (HANDLE);
typedef int    (__cdecl * _cpp_get_error) (HANDLE);
typedef int    (__cdecl * _cpp_get_version) (void);
typedef int    (__cdecl * _cpp_size_keyx) (void);
typedef void   (__cdecl * _cpp_get_keyx) (HANDLE,BYTE*);
typedef void   (__cdecl * _cpp_set_keyx) (HANDLE,BYTE*);
typedef int    (__cdecl * _cpp_size_keyp) (void);
typedef void   (__cdecl * _cpp_get_keyp) (HANDLE,BYTE*);
typedef void   (__cdecl * _cpp_set_keyp) (HANDLE,BYTE*);
typedef int    (__cdecl * _cpp_keya) (HANDLE);
typedef int    (__cdecl * _cpp_keyb) (HANDLE);
typedef int    (__cdecl * _cpp_keyx) (HANDLE);
typedef int    (__cdecl * _cpp_keyp) (HANDLE);

typedef int    (__cdecl * _pgp_init) (void);
typedef int    (__cdecl * _pgp_done) (void);
typedef int    (__cdecl * _pgp_open_keyrings) (LPSTR,LPSTR);
typedef int    (__cdecl * _pgp_close_keyrings) (void);
typedef int    (__cdecl * _pgp_get_version) (void);
typedef LPSTR  (__cdecl * _pgp_get_error) (void);
typedef int    (__cdecl * _pgp_set_priv_key) (LPCSTR);
typedef int    (__cdecl * _pgp_set_key) (HANDLE,LPCSTR);
typedef int    (__cdecl * _pgp_set_keyid) (HANDLE,PVOID);
typedef int    (__cdecl * _pgp_size_keyid) (void);
typedef PVOID  (__cdecl * _pgp_select_keyid) (HWND,LPSTR);
typedef LPSTR  (__cdecl * _pgp_encode) (HANDLE,LPCSTR);
typedef LPSTR  (__cdecl * _pgp_decode) (HANDLE,LPCSTR);

typedef int    (__cdecl * _gpg_init) (void);
typedef int    (__cdecl * _gpg_done) (void);
typedef int    (__cdecl * _gpg_open_keyrings) (LPSTR,LPSTR);
typedef int    (__cdecl * _gpg_close_keyrings) (void);
typedef void   (__cdecl * _gpg_set_log) (LPCSTR);
typedef void   (__cdecl * _gpg_set_tmp) (LPCSTR);
typedef LPSTR  (__cdecl * _gpg_get_error) (void);
typedef int    (__cdecl * _gpg_set_keyid) (HANDLE,LPCSTR);
typedef int    (__cdecl * _gpg_size_keyid) (void);
typedef int    (__cdecl * _gpg_select_keyid) (HWND,LPSTR);
typedef LPSTR  (__cdecl * _gpg_encode) (HANDLE,LPCSTR);
typedef LPSTR  (__cdecl * _gpg_decode) (HANDLE,LPCSTR);
typedef LPSTR  (__cdecl * _gpg_get_passphrases) (void);
typedef void   (__cdecl * _gpg_set_passphrases) (LPCSTR);

typedef int    (__cdecl * _rsa_init) (pRSA_EXPORT*,pRSA_IMPORT);
typedef int    (__cdecl * _rsa_done) (void);


typedef struct {
	_cpp_create_context cc;
	_cpp_delete_context dc;
	_cpp_reset_context rc;
	_cpp_init_keya ika;
	_cpp_init_keyb ikb;
	_cpp_calc_keyx ckx;
	_cpp_init_keyp ikp;
	_cpp_encodeA ea;
	_cpp_encodeW ew;
	_cpp_encodeU eu;
	_cpp_decode daw;
	_cpp_decodeU du;
	_cpp_encrypt_file ef;
	_cpp_decrypt_file df;
	_cpp_get_features gf;
	_cpp_get_error ge;
	_cpp_get_version gv;
	_cpp_size_keyx kxs;
	_cpp_get_keyx gkx;
	_cpp_set_keyx skx;
	_cpp_size_keyp kps;
	_cpp_get_keyp gkp;
	_cpp_set_keyp skp;
	_cpp_keya ka;
	_cpp_keyb kb;
	_cpp_keyx kx;
	_cpp_keyp kp;
	_pgp_init			pgp_i;
	_pgp_done			pgp_d;
	_pgp_open_keyrings	pgp_ok;
	_pgp_close_keyrings	pgp_ck;
	_pgp_get_version	pgp_gv;
	_pgp_get_error		pgp_ge;
	_pgp_set_keyid		pgp_ski;
	_pgp_set_key		pgp_sk;
	_pgp_set_priv_key	pgp_spk;
	_pgp_size_keyid		pgp_szki;
	_pgp_select_keyid	pgp_slki;
	_pgp_encode		pgp_eu;
	_pgp_decode			pgp_de;
	_gpg_init			gpg_i;
	_gpg_done			gpg_d;
	_gpg_open_keyrings	gpg_ok;
	_gpg_close_keyrings	gpg_ck;
	_gpg_set_log		gpg_sl;
	_gpg_set_tmp		gpg_st;
	_gpg_get_error		gpg_ge;
	_gpg_set_keyid		gpg_ski;
	_gpg_size_keyid		gpg_szki;
	_gpg_select_keyid	gpg_slki;
	_gpg_encode		gpg_eu;
	_gpg_decode			gpg_de;
	_gpg_get_passphrases gpg_gph;
	_gpg_set_passphrases gpg_sph;
	_rsa_init			rsa_i;
	_rsa_done			rsa_d;
} CRYPTOPP_INFO;

extern CRYPTOPP_INFO cpp;

#define cpp_create_context	cpp.cc
#define cpp_delete_context	cpp.dc
#define cpp_reset_context	cpp.rc
#define cpp_init_keya		cpp.ika
#define cpp_init_keyb		cpp.ikb
#define cpp_calc_keyx		cpp.ckx
#define cpp_init_keyp		cpp.ikp
#define cpp_encodeA			cpp.ea
#define cpp_encodeW			cpp.ew
#define cpp_encodeU			cpp.eu
#define cpp_decode			cpp.daw
#define cpp_decodeU			cpp.du
#define cpp_encrypt_file	cpp.ef
#define cpp_decrypt_file	cpp.df
#define cpp_get_features	cpp.gf
#define cpp_get_error		cpp.ge
#define cpp_get_version		cpp.gv
#define cpp_size_keyx		cpp.kxs
#define cpp_get_keyx		cpp.gkx
#define cpp_set_keyx		cpp.skx
#define cpp_size_keyp		cpp.kps
#define cpp_get_keyp		cpp.gkp
#define cpp_set_keyp		cpp.skp
#define cpp_keya			cpp.ka
#define cpp_keyb			cpp.kb
#define cpp_keyx			cpp.kx
#define cpp_keyp			cpp.kp
#define pgp_init			cpp.pgp_i
#define pgp_done			cpp.pgp_d
#define pgp_open_keyrings	cpp.pgp_ok
#define pgp_close_keyrings	cpp.pgp_ck
#define pgp_get_version		cpp.pgp_gv
#define pgp_get_error		cpp.pgp_ge
#define pgp_set_priv_key	cpp.pgp_spk
#define pgp_set_key		cpp.pgp_sk
#define pgp_set_keyid		cpp.pgp_ski
#define pgp_size_keyid		cpp.pgp_szki
#define	pgp_select_keyid	cpp.pgp_slki
#define pgp_encode			cpp.pgp_eu
#define pgp_decode			cpp.pgp_de
#define gpg_init			cpp.gpg_i
#define gpg_done			cpp.gpg_d
#define gpg_open_keyrings	cpp.gpg_ok
#define gpg_close_keyrings	cpp.gpg_ck
#define gpg_set_log			cpp.gpg_sl
#define gpg_set_tmp			cpp.gpg_st
#define gpg_get_error		cpp.gpg_ge
#define gpg_set_keyid		cpp.gpg_ski
#define gpg_size_keyid		cpp.gpg_szki
#define	gpg_select_keyid	cpp.gpg_slki
#define gpg_encode			cpp.gpg_eu
#define gpg_decode			cpp.gpg_de
#define gpg_get_passphrases	cpp.gpg_gph
#define gpg_set_passphrases	cpp.gpg_sph
#define rsa_init			cpp.rsa_i
#define rsa_done			cpp.rsa_d

#endif
