/** @file 
 * @brief ���������� ������� ����������� ������ ������ ������� ����������
 *
 * @copyright InfoTeCS. All rights reserved.
 */

#ifndef BLOCK_CHIPHER_H
#define BLOCK_CHIPHER_H

#define DLL_IMPORT

/** @brief ������ ��������� ��� ������ ���������� ECB ��������� "��������" */
#define kEcb14ContextLen 0x28

/** @brief ������ ��������� ��� ������ ���������� ECB ��������� 28147-89 */
#define kEcb89ContextLen 0x28

/** @brief ������ ��������� ��� ������ ���������� CBC ��������� "��������" */
#define kCbc14ContextLen 0x38

/** @brief ������ ��������� ��� ������ ���������� CBC ��������� 28147-89 */
#define kCbc89ContextLen 0x38

/** @brief ������ ��������� ��� ������ ���������� CRT ��������� "��������" */
#define kCrt14ContextLen 0x30

/** @brief ������ ��������� ��� ������ ���������� CRT ��������� 28147-89 */
#define kCrt89ContextLen 0x30

/** @brief ������ ��������� ��� ������ ���������� OFB ��������� "��������" */
#define kOfb14ContextLen 0x40

/** @brief ������ ��������� ��� ������ ���������� OFB ��������� 28147-89 */
#define kOfb89ContextLen 0x40

/** @brief ������ ��������� ��� ������ ���������� CFB ��������� "��������" */
#define kCfb14ContextLen 0x40

/** @brief ������ ��������� ��� ������ ���������� CFB ��������� 28147-89 */
#define kCfb89ContextLen 0x40

/** @brief ������ ��������� ��� ������ ������������ ������������ ��������� "��������" */
#define kImit14ContextLen 0x68

/** @brief ������ ��������� ��� ������ ������������ ������������ ��������� 28147-89 */
#define kImit89ContextLen 0x68

/** @brief ������ ����� ��������� "��������" */
#define kBlockLen14 16

/** @brief ������ ����� ��������� 28147-89 */
#define kBlockLen89 8

/** @brief ������ ����� ��������� 28147-89 */
#define kKeyLen89 32

#ifdef __cplusplus
extern "C" {
#endif

/* *** ������ ���������� ***
 * ������ ���������� �������� � �������������� �������� 
 * ������������������ ��������������
 */

/** @brief ������������� ��������� ���������� � ������ ECB ��� ��������� "��������"
 *
 * @param[in] key ����
 * @param[out] ctx �������� cbc
 * @param[in] print ������� �����������
 * @param[in] print_uint ������� �����������
 * @return 0 ���� ��� �������������� ������ �������
 * @return -1 ���� ��������� ������
 */
int DLL_IMPORT init_ecb_14(unsigned char *key, void* ctx);

/** @brief �������� ��������� ecb
 *
 * @param[in] ctx �������� ecb
 * @return 0 ���� ��� �������������� ������ �������
 * @return -1 ���� ��������� ������
 */
void DLL_IMPORT free_ecb(void* ctx);

/** @brief ������������� ��������� ���������� � ������ CBC ��� ��������� "��������"
 *
 * @param[in] key ����
 * @param[out] ctx �������� cbc
 * @param[in] iv �������������
 * @param[in] ivLength ������ �������������
 * @param[in] print ������� �����������
 * @param[in] print_uint ������� �����������
 * @return 0 ���� ��� �������������� ������ �������
 * @return -1 ���� ��������� ������
 */
int DLL_IMPORT  init_cbc_14(unsigned char *key, void* ctx, unsigned char *iv, size_t ivLength);

/** @brief �������� ��������� cbc
 *
 * @param[in] ctx �������� cbc
 * @return 0 ���� ��� �������������� ������ �������
 * @return -1 ���� ��������� ������
 */
void DLL_IMPORT free_cbc(void* ctx);

/** @brief ������������� ��������� ���������� � ������ CRT ��� ��������� "��������"
 *
 * @param[in] key ����
 * @param[out] ctx �������� crt
 * @param[in] iv �������������
 * @param[in] length ������ �������������
 * @param[in] print ������� �����������
 * @param[in] print_uint ������� �����������
 * @return 0 ���� ��� �������������� ������ �������
 * @return -1 ���� ��������� ������
 */
int DLL_IMPORT init_crt_14(unsigned char* key, unsigned char *iv, size_t length, void *ctx);

/** @brief �������� ��������� crt
 *
 * @param[in] ctx �������� crt
 * @return 0 ���� ��� �������������� ������ �������
 * @return -1 ���� ��������� ������
 */
void DLL_IMPORT free_crt(void* ctx);

/** @brief ������������� ��������� ���������� � ������ OFB ��� ��������� "��������"
 *
 * @param[in] key ����
 * @param[out] ctx �������� ofb
 * @param[in] s �������� S
 * @param[in] iv �������������
 * @param[in] ivLength ������ �������������
 * @param[in] print ������� �����������
 * @param[in] print_uint ������� �����������
 * @return 0 ���� ��� �������������� ������ �������
 * @return -1 ���� ��������� ������
 */
int DLL_IMPORT init_ofb_14(unsigned char *key, void *ctx, size_t s, unsigned char *iv, size_t ivLength);


/** @brief �������� ��������� ofb
 *
 * @param[in] ctx �������� ofb
 * @return 0 ���� ��� �������������� ������ �������
 * @return -1 ���� ��������� ������
 */
void DLL_IMPORT free_ofb(void* ctx);

/** @brief ������������� ��������� ���������� � ������ CFB ��� ��������� "��������"
 *
 * @param[in] key ����
 * @param[out] ctx �������� cfb
 * @param[in] s �������� S
 * @param[in] iv �������������
 * @param[in] ivLength ������ �������������
 * @param[in] print ������� �����������
 * @param[in] print_uint ������� �����������
 * @return 0 ���� ��� �������������� ������ �������
 * @return -1 ���� ��������� ������
 */
int DLL_IMPORT init_cfb_14(unsigned char *key, void *ctx, size_t s, unsigned char *iv, size_t ivLength);

/** @brief ������������� ��������� ���������� � ������ CFB ��� ��������� 28147-89
 *
 * @param[in] key ����
 * @param[out] ctx �������� cfb
 * @param[in] s �������� S
 * @param[in] iv �������������
 * @param[in] ivLength ������ �������������
 * @param[in] print ������� �����������
 * @param[in] print_uint ������� �����������
 * @return 0 ���� ��� �������������� ������ �������
 * @return -1 ���� ��������� ������
 */
int DLL_IMPORT init_cfb_89(unsigned char *key, void *ctx, size_t s, unsigned char *iv, size_t ivLength);

/** @brief �������� ��������� cfb
 *
 * @param[in] ctx �������� cfb
 * @return 0 ���� ��� �������������� ������ �������
 * @return -1 ���� ��������� ������
 */
void DLL_IMPORT free_cfb(void* ctx);

/** @brief ������������� ��������� ����������� ��� ��������� "��������"
 *
 * @param[out] ctx �������� ������������
 * @param[in] key ����
 * @param[in] s �������� S
 * @param[in] print ������� �����������
 * @param[in] print_uint ������� �����������
 * @return 0 ���� ��� �������������� ������ �������
 * @return -1 ���� ��������� ������
 */
int DLL_IMPORT init_imit_14(unsigned char *key, size_t s, void *ctx);

/** @brief ������������� ��������� ����������� ��� ��������� 28147-89
 *
 * @param[in] key ����
 * @param[in] s �������� S
 * @param[out] ctx �������� ������������
 * @param[in] print ������� �����������
 * @param[in] print_uint ������� �����������
 * @return 0 ���� ��� �������������� ������ �������
 * @return -1 ���� ��������� ������
 */
int DLL_IMPORT init_imit_89(unsigned char *key, size_t s, void *ctx);

/** @brief �������� ��������� ������������
 *
 * @param[in] ctx �������� ������������
 * @return 0 ���� ��� �������������� ������ �������
 * @return -1 ���� ��������� ������
 */
void DLL_IMPORT free_imit(void* ctx);

/** @brief ���������� ������������ ���������� � ������ ������� ������ ��� ������ ������� ������� �����
 *
 * @param[in] ctx �������� ECB
 * @param[in] indata �������� �����
 * @param[out] outdata ������������� �����
 * @param[in] length ������ ������
 * @return 0 ���� ��� �������������� ������ �������
 * @return -1 ���� ��������� ������
 */
int DLL_IMPORT encrypt_ecb(void *ctx, unsigned char *indata, unsigned char *outdata, size_t length);

/** @brief ���������� ������������� ���������� � ������ ������� ������ ��� ������ ������� ������� �����
 *
 * @param[in] ctx �������� ECB
 * @param[in] indata �������� �����
 * @param[out] outdata ������������� �����
 * @param[in] length ������ ������
 * @return 0 ���� ��� �������������� ������ �������
 * @return -1 ���� ��������� ������
 */
int DLL_IMPORT decrypt_ecb(void *ctx, unsigned char *indata, unsigned char *outdata, size_t length);

/** @brief ���������� ������������ ���������� � ������ ������� ������ � ����������� ��� ������ ������� ������� �����
 *
 * @param[in] ctx �������� CBC
 * @param[in] indata �������� �����
 * @param[out] outdata ������������� �����
 * @param[in] length ������ ������
 * @return 0 ���� ��� �������������� ������ �������
 * @return -1 ���� ��������� ������
 */
int DLL_IMPORT encrypt_cbc(void *ctx, unsigned char *indata, unsigned char *outdata, size_t length);

/** @brief ���������� ������������ ���������� � ������ ������� ������ � ����������� ��� ������ ������� ������� �����
 *
 * @param[in] ctx �������� CBC
 * @param[in] indata ������������� �����
 * @param[out] outdata �������������� �����
 * @param[in] length ������ ������
 * @return 0 ���� ��� �������������� ������ �������
 * @return -1 ���� ��������� ������
 */
int DLL_IMPORT decrypt_cbc(void *ctx, unsigned char *indata, unsigned char *outdata, size_t length);

/** @brief ���������� ���������� (������������ ��� �������������) � ������ ������������
 *
 * @param[in] ctx �������� CRT
 * @param[in] indata ������� ���������
 * @param[out] outdata ���������
 * @param[in] length ������ ���������
 * @return 0 ���� ��� �������������� ������ �������
 * @return -1 ���� ��������� ������
 */
int DLL_IMPORT crypt_crt(void *ctx, unsigned char *indata, unsigned char *outdata, size_t length);

/** @brief ���������� ������� ���������� ���������� � ������ ������������ � �������� ������
 *
 * @param[in] ctx �������� OFB
 * @param[in] indata ������� ����
 * @param[out] outdata ��������� ��������������
 * @param[in] inlength ������ ������
 * @return 0 ���� ��� �������������� ������ �������
 * @return -1 ���� ��������� ������
 */
int DLL_IMPORT crypt_ofb(void *ctx, unsigned char *indata, unsigned char *outdata, size_t inlength);

/** @brief ���������� ������������ ���������� � ������ ������������ � �������� ������
 *
 * @param[in] ctx �������� OFB
 * @param[in] indata �������� �����
 * @param[out] outdata ������������� �����
 * @param[in] inlength ������ ������
 * @return 0 ���� ��� �������������� ������ �������
 * @return -1 ���� ��������� ������
 */
int DLL_IMPORT encrypt_ofb(void *ctx, unsigned char *indata, unsigned char *outdata, size_t inlength);

/** @brief ���������� ������������� ���������� � ������ ������������ � �������� ������
 *
 * @param[in] ctx �������� OFB
 * @param[in] indata ������������� �����
 * @param[out] outdata �������������� �����
 * @param[in] inlength ������ ������
 * @return 0 ���� ��� �������������� ������ �������
 * @return -1 ���� ��������� ������
 */
int DLL_IMPORT decrypt_ofb(void *ctx, unsigned char *indata, unsigned char *outdata, size_t inlength);

/** @brief ���������� ������������ ���������� � ������ ������������ � �������� ������ �� ����������
 *
 * @param[in] ctx �������� CFB
 * @param[in] indata �������� �����
 * @param[out] outdata ������������� �����
 * @param[in] inlength ������ ������
 * @return 0 ���� ��� �������������� ������ �������
 * @return -1 ���� ��������� ������
 */
int DLL_IMPORT encrypt_cfb(void *ctx, unsigned char *indata, unsigned char *outdata, size_t inlength);

/** @brief ���������� ������������� ���������� � ������ ������������ � �������� ������ �� ����������
 *
 * @param[in] ctx �������� CFB
 * @param[in] indata ������������� �����
 * @param[out] outdata �������������� �����
 * @param[in] inlength ������ ������
 * @return 0 ���� ��� �������������� ������ �������
 * @return -1 ���� ��������� ������
 */
int DLL_IMPORT decrypt_cfb(void *ctx, unsigned char *indata, unsigned char *outdata, size_t inlength);

/** @brief ���������� ���������� ������������ �� ������ ������� ������� �����
 *
 * @param[in] ctx �������� ������������
 * @param[in] indata �������� �����
 * @param[in] length ������ ������
 * @return 0 ���� ��� �������������� ������ �������
 * @return -1 ���� ��������� ������
 */
int DLL_IMPORT imit(void *ctx, unsigned char *indata, size_t length);

/** @brief ���������� ��������� ������������
 *
 * @param[in] ctx �������� ������������
 * @param[out] value
 * @return 0 ���� ��� �������������� ������ �������
 * @return -1 ���� ��������� ������
 */
int DLL_IMPORT done_imit(void *ctx, unsigned char *value);

/** @brief ���������� ������ �� ������� �����. 
 *
 * @param[in] data ���������. ������ ��� ������ data ������ ���� ��������, ����������� ��� ����������.
 * @param[in] length ������ ���������
 * @param[in] blockLen ������ �����
 * @return ������ ���������
 */
int DLL_IMPORT padd(unsigned char *data, size_t length, size_t blockLen);

/** @brief �������� ����������� ������. ��� ������� ������������ �������� -1
 *
 * @param[in] data ���������
 * @param[in] length ������ ���������
 * @return ������ ���������
 */
int DLL_IMPORT unpadd(unsigned char *data, size_t length);

#ifdef __cplusplus
}
#endif

#endif
