/** @file 
 * @brief ���������� ������� ����������� �������������� �� ��������� "��������"
 *
 * @copyright InfoTeCS. All rights reserved.
 */

#ifndef C_28147_14_H
#define C_28147_14_H

//#include "dll_import.h"
//#include "callback_print.h"


#ifdef __cplusplus
extern "C" {
#endif

/** @brief �������������� X
 *
 * @param[in] a ������� �������� ��������������
 * @param[in] b ������� �������� ��������������
 * @param[out] outdata ��������� �������� ��������������
 * @param[in] print ������� �����������
 * @return 0 ���� ��� �������������� ������ �������
 * @return -1 ���� ��������� ������
 */
int funcX(unsigned char* a, unsigned char* b, unsigned char* outdata);

/** @brief �������������� S
 * 
 * @param[in] indata ������� �������� ��������������
 * @param[out] outdata ��������� �������� ��������������
 * @param[in] print ������� �����������
 * @return 0 ���� ��� �������������� ������ �������
 * @return -1 ���� ��������� ������
 */
int funcS(unsigned char* indata, unsigned char* outdata);

/** @brief �������������� �������� � ��������������  S
 * 
 * @param[in] indata ������� �������� ��������������
 * @param[out] outdata ��������� �������� ��������������
 * @param[in] print ������� �����������
 * @return 0 ���� ��� �������������� ������ �������
 * @return -1 ���� ��������� ������
 */
int funcReverseS(unsigned char* indata, unsigned char*  outdata);

/** @brief �������������� R
 * 
 * @param[in] indata ������� �������� ��������������
 * @param[out] outdata ��������� �������� ��������������
 * @param[in] print ������� �����������
 * @return 0 ���� ��� �������������� ������ �������
 * @return -1 ���� ��������� ������
 */
int funcR(unsigned char* indata, unsigned char* outdata);

/** @brief �������������� �������� � ��������������  R
 * 
 * @param[in] indata ������� �������� ��������������
 * @param[out] outdata ��������� �������� ��������������
 * @param[in] print ������� �����������
 * @return 0 ���� ��� �������������� ������ �������
 * @return -1 ���� ��������� ������
 */
int funcReverseR(unsigned char* indata, unsigned char* outdata);

/** @brief �������������� L
 * 
 * @param[in] indata ������� �������� ��������������
 * @param[out] outdata ��������� �������� ��������������
 * @param[in] print ������� �����������
 * @return 0 ���� ��� �������������� ������ �������
 * @return -1 ���� ��������� ������
 */
int  funcL(unsigned char* indata, unsigned char* outdata);

/** @brief �������������� �������� � ��������������  L
 * 
 * @param[in] indata ������� �������� ��������������
 * @param[out] outdata ��������� �������� ��������������
 * @param[in] print ������� �����������
 * @return 0 ���� ��� �������������� ������ �������
 * @return -1 ���� ��������� ������
 */
int  funcReverseL(unsigned char* indata, unsigned char* outdata);

/** @brief �������������� LSX
 * 
 * @param[in] a ������� �������� ��������������
 * @param[in] b ������� �������� ��������������
 * @param[out] outdata ��������� �������� ��������������
 * @param[in] print ������� �����������
 * @return 0 ���� ��� �������������� ������ �������
 * @return -1 ���� ��������� ������
 */
int  funcLSX(unsigned char* a, unsigned char* b, unsigned char* outdata);

/** @brief �������������� �������� � ��������������  LSX
 * 
 * @param[in] a ������� �������� ��������������
 * @param[in] b ������� �������� ��������������
 * @param[out] outdata ��������� �������� ��������������
 * @param[in] print ������� �����������
 * @return 0 ���� ��� �������������� ������ �������
 * @return -1 ���� ��������� ������
 */
int  funcReverseLSX(unsigned char* a, unsigned char* b, unsigned char* outdata);

/** @brief �������������� F
 * 
 * @param[in] inputKey ������ ����  �� ���� ������ ���������� � ���������� ��������
 * @param[in] inputKeySecond ������ ����  �� ���� ������ ���������� � ���������� ��������
 * @param[in] iterationConst ������������ ���������
 * @param[out] outputKey ������ ����
 * @param[out] outputKeySecond ������ ����
 * @param[in] print ������� �����������
 * @return 0 ���� ��� �������������� ������ �������
 * @return -1 ���� ��������� ������
 */
int  funcF(unsigned char* inputKey, unsigned char* inputKeySecond, unsigned char* iterationConst, unsigned char* outputKey, unsigned char* outputKeySecond);

/** @brief ���������� ������������ ��������� � 
 * 
 * @param[in] number ����� ���������
 * @param[out] output ������������ ���������
 * @param[in] print ������� �����������
 * @return 0 ���� ��� �������������� ������ �������
 * @return -1 ���� ��������� ������
 */
int  funcC(unsigned char number, unsigned char* outputW);

/** @brief ��������� �����
 * 
 * @param[in] masterKey ������ ����
 * @param[out] keys ������ ����������� ������
 * @param[in] print ������� �����������
 * @return 0 ���� ��� �������������� ������ �������
 * @return -1 ���� ��������� ������
 */
int ExpandKey(unsigned char* masterKey, unsigned char* keys);

/** @brief ���������� ������������ �����
 * 
 * @param[in] plainText �������� ����
 * @param[out] chipherText ������������� ����
 * @param[in] keys ����������� �����
 * @param[in] print ������� �����������
 * @param[in] print_uint ������� �����������
 * @return 0 ���� ��� �������������� ������ �������
 * @return -1 ���� ��������� ������
 */
int Encrypt_14(unsigned char* plainText, unsigned char* chipherText, unsigned char* keys);

/** @brief ��������� ������������� �����
 * 
 * @param[in] chipherText ������������� ����
 * @param[out] plainText �������������� ����
 * @param[in] keys ����������� �����
 * @param[in] print ������� �����������
 * @param[in] print_uint ������� �����������
 * @return 0 ���� ��� �������������� ������ �������
 * @return -1 ���� ��������� ������
 */
int Decrypt_14(unsigned char* chipherText, unsigned char* plainText, unsigned char* keys);

#ifdef __cplusplus
}
#endif

#endif
