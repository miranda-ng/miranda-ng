#ifndef __CPP_RSAM_H__
#define __CPP_RSAM_H__

int __cdecl rsa_gen_keypair(short);
int __cdecl rsa_get_keypair(short,PBYTE,int*,PBYTE,int*);
int __cdecl rsa_get_keyhash(short,PBYTE,int*,PBYTE,int*);
int __cdecl rsa_set_keypair(short,PBYTE,int);
int __cdecl rsa_get_pubkey(HANDLE,PBYTE,int*);
int __cdecl rsa_set_pubkey(HANDLE,PBYTE,int);
void __cdecl rsa_set_timeout(int);
int __cdecl rsa_get_state(HANDLE);
int __cdecl rsa_get_hash(PBYTE,int,PBYTE,int*);
int __cdecl rsa_connect(HANDLE);
int __cdecl rsa_disconnect(HANDLE);
int __cdecl rsa_disabled(HANDLE);
LPSTR __cdecl rsa_recv(HANDLE,LPCSTR);
int __cdecl rsa_send(HANDLE,LPCSTR);
int __cdecl rsa_encrypt_file(HANDLE,LPCSTR,LPCSTR);
int __cdecl rsa_decrypt_file(HANDLE,LPCSTR,LPCSTR);
int __cdecl rsa_export_keypair(short,LPSTR,LPSTR,LPSTR);
int __cdecl rsa_import_keypair(short,LPSTR,LPSTR);
int __cdecl rsa_export_pubkey(HANDLE,LPSTR);
int __cdecl rsa_import_pubkey(HANDLE,LPSTR);


void inject_msg(HANDLE,int,const string&);
string encode_msg(short,pRSADATA,string&);
string decode_msg(pRSADATA,string&);
string encode_rsa(short,pRSADATA,pRSAPRIV,string&);
string decode_rsa(pRSADATA,pRSAPRIV,string&);
string gen_aes_key_iv(short,pRSADATA,pRSAPRIV);
void init_priv(pRSAPRIV,string&);
void init_pub(pRSADATA,string&);
void null_msg(HANDLE,int,int);

void rsa_timeout(HANDLE,pRSADATA);
void rsa_alloc(pCNTX);
int  rsa_free(pCNTX);
void rsa_free_thread(pRSADATA);
void clear_queue(pRSADATA);


unsigned __stdcall sttConnectThread(LPVOID);

#endif
