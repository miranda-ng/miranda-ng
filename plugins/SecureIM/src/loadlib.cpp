#include "commonheaders.h"

CRYPTOPP_INFO cpp;

HMODULE h;
BOOL loadlib(void)
{
	h = LoadLibrary(_T("plugins/cryptopp.dll"));
	if (h == NULL) {
		h = LoadLibrary(_T("cryptopp.dll"));
		if (h == NULL) return 0;
	}

	cpp.cc = (_cpp_create_context)GetProcAddress(h, "cpp_create_context");
	cpp.dc = (_cpp_delete_context)GetProcAddress(h, "cpp_delete_context");
	cpp.rc = (_cpp_reset_context)GetProcAddress(h, "cpp_reset_context");
	cpp.ika = (_cpp_init_keya)GetProcAddress(h, "cpp_init_keya");
	cpp.ikb = (_cpp_init_keyb)GetProcAddress(h, "cpp_init_keyb");
	cpp.ckx = (_cpp_calc_keyx)GetProcAddress(h, "cpp_calc_keyx");
	cpp.ikp = (_cpp_init_keyp)GetProcAddress(h, "cpp_init_keyp");
	cpp.ea = (_cpp_encodeA)GetProcAddress(h, "cpp_encodeA");
	cpp.ew = (_cpp_encodeW)GetProcAddress(h, "cpp_encodeW");
	cpp.eu = (_cpp_encodeU)GetProcAddress(h, "cpp_encodeU");
	cpp.daw = (_cpp_decode)GetProcAddress(h, "cpp_decode");
	cpp.du = (_cpp_decodeU)GetProcAddress(h, "cpp_decodeU");
	cpp.ef = (_cpp_encrypt_file)GetProcAddress(h, "cpp_encrypt_file");
	cpp.df = (_cpp_decrypt_file)GetProcAddress(h, "cpp_decrypt_file");
	cpp.gf = (_cpp_get_features)GetProcAddress(h, "cpp_get_features");
	cpp.ge = (_cpp_get_error)GetProcAddress(h, "cpp_get_error");
	cpp.gv = (_cpp_get_version)GetProcAddress(h, "cpp_get_version");
	cpp.kxs = (_cpp_size_keyx)GetProcAddress(h, "cpp_size_keyx");
	cpp.gkx = (_cpp_get_keyx)GetProcAddress(h, "cpp_get_keyx");
	cpp.skx = (_cpp_set_keyx)GetProcAddress(h, "cpp_set_keyx");
	cpp.kps = (_cpp_size_keyp)GetProcAddress(h, "cpp_size_keyp");
	cpp.gkp = (_cpp_get_keyp)GetProcAddress(h, "cpp_get_keyp");
	cpp.skp = (_cpp_set_keyp)GetProcAddress(h, "cpp_set_keyp");
	cpp.ka = (_cpp_keya)GetProcAddress(h, "cpp_keya");
	cpp.kb = (_cpp_keyb)GetProcAddress(h, "cpp_keyb");
	cpp.kx = (_cpp_keyx)GetProcAddress(h, "cpp_keyx");
	cpp.kp = (_cpp_keyp)GetProcAddress(h, "cpp_keyp");

	cpp.pgp_i = (_pgp_init)GetProcAddress(h, "pgp_init");
	cpp.pgp_d = (_pgp_done)GetProcAddress(h, "pgp_done");
	cpp.pgp_ok = (_pgp_open_keyrings)GetProcAddress(h, "pgp_open_keyrings");
	cpp.pgp_ck = (_pgp_close_keyrings)GetProcAddress(h, "pgp_close_keyrings");
	cpp.pgp_gv = (_pgp_get_version)GetProcAddress(h, "pgp_get_version");
	cpp.pgp_ge = (_pgp_get_error)GetProcAddress(h, "pgp_get_error");
	cpp.pgp_spk = (_pgp_set_priv_key)GetProcAddress(h, "pgp_set_priv_key");
	cpp.pgp_sk = (_pgp_set_key)GetProcAddress(h, "pgp_set_key");
	cpp.pgp_ski = (_pgp_set_keyid)GetProcAddress(h, "pgp_set_keyid");
	cpp.pgp_szki = (_pgp_size_keyid)GetProcAddress(h, "pgp_size_keyid");
	cpp.pgp_slki = (_pgp_select_keyid)GetProcAddress(h, "pgp_select_keyid");
	cpp.pgp_eu = (_pgp_encode)GetProcAddress(h, "pgp_encode");
	cpp.pgp_de = (_pgp_decode)GetProcAddress(h, "pgp_decode");

	cpp.gpg_i = (_gpg_init)GetProcAddress(h, "gpg_init");
	cpp.gpg_d = (_gpg_done)GetProcAddress(h, "gpg_done");
	cpp.gpg_ok = (_gpg_open_keyrings)GetProcAddress(h, "gpg_open_keyrings");
	cpp.gpg_ck = (_gpg_close_keyrings)GetProcAddress(h, "gpg_close_keyrings");
	cpp.gpg_sl = (_gpg_set_log)GetProcAddress(h, "gpg_set_log");
	cpp.gpg_st = (_gpg_set_tmp)GetProcAddress(h, "gpg_set_tmp");
	cpp.gpg_ge = (_gpg_get_error)GetProcAddress(h, "gpg_get_error");
	cpp.gpg_ski = (_gpg_set_keyid)GetProcAddress(h, "gpg_set_keyid");
	cpp.gpg_szki = (_gpg_size_keyid)GetProcAddress(h, "gpg_size_keyid");
	cpp.gpg_slki = (_gpg_select_keyid)GetProcAddress(h, "gpg_select_keyid");
	cpp.gpg_eu = (_gpg_encode)GetProcAddress(h, "gpg_encode");
	cpp.gpg_de = (_gpg_decode)GetProcAddress(h, "gpg_decode");
	cpp.gpg_gph = (_gpg_get_passphrases)GetProcAddress(h, "gpg_get_passphrases");
	cpp.gpg_sph = (_gpg_set_passphrases)GetProcAddress(h, "gpg_set_passphrases");

	cpp.rsa_i = (_rsa_init)GetProcAddress(h, "rsa_init");
	cpp.rsa_d = (_rsa_done)GetProcAddress(h, "rsa_done");

	return (cpp_get_version() >= 0x01000403);
}

void freelib() {
	FreeLibrary(h);
}