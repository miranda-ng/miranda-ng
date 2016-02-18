/** @file 
 * @brief Объявление функций реализующих преобразования из алгоритма "Кузнечик"
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

/** @brief Преобразование X
 *
 * @param[in] a входной параметр преобразования
 * @param[in] b входной параметр преобразования
 * @param[out] outdata результат параметр преобразования
 * @param[in] print функция логирования
 * @return 0 если все преобразование прошло успешно
 * @return -1 если произошла ошибка
 */
int funcX(unsigned char* a, unsigned char* b, unsigned char* outdata);

/** @brief Преобразование S
 * 
 * @param[in] indata входной параметр преобразования
 * @param[out] outdata результат параметр преобразования
 * @param[in] print функция логирования
 * @return 0 если все преобразование прошло успешно
 * @return -1 если произошла ошибка
 */
int funcS(unsigned char* indata, unsigned char* outdata);

/** @brief Преобразование обратное к преобразованию  S
 * 
 * @param[in] indata входной параметр преобразования
 * @param[out] outdata результат параметр преобразования
 * @param[in] print функция логирования
 * @return 0 если все преобразование прошло успешно
 * @return -1 если произошла ошибка
 */
int funcReverseS(unsigned char* indata, unsigned char*  outdata);

/** @brief Преобразование R
 * 
 * @param[in] indata входной параметр преобразования
 * @param[out] outdata результат параметр преобразования
 * @param[in] print функция логирования
 * @return 0 если все преобразование прошло успешно
 * @return -1 если произошла ошибка
 */
int funcR(unsigned char* indata, unsigned char* outdata);

/** @brief Преобразование обратное к преобразованию  R
 * 
 * @param[in] indata входной параметр преобразования
 * @param[out] outdata результат параметр преобразования
 * @param[in] print функция логирования
 * @return 0 если все преобразование прошло успешно
 * @return -1 если произошла ошибка
 */
int funcReverseR(unsigned char* indata, unsigned char* outdata);

/** @brief Преобразование L
 * 
 * @param[in] indata входной параметр преобразования
 * @param[out] outdata результат параметр преобразования
 * @param[in] print функция логирования
 * @return 0 если все преобразование прошло успешно
 * @return -1 если произошла ошибка
 */
int  funcL(unsigned char* indata, unsigned char* outdata);

/** @brief Преобразование обратное к преобразованию  L
 * 
 * @param[in] indata входной параметр преобразования
 * @param[out] outdata результат параметр преобразования
 * @param[in] print функция логирования
 * @return 0 если все преобразование прошло успешно
 * @return -1 если произошла ошибка
 */
int  funcReverseL(unsigned char* indata, unsigned char* outdata);

/** @brief Преобразование LSX
 * 
 * @param[in] a входной параметр преобразования
 * @param[in] b входной параметр преобразования
 * @param[out] outdata результат параметр преобразования
 * @param[in] print функция логирования
 * @return 0 если все преобразование прошло успешно
 * @return -1 если произошла ошибка
 */
int  funcLSX(unsigned char* a, unsigned char* b, unsigned char* outdata);

/** @brief Преобразование обратное к преобразованию  LSX
 * 
 * @param[in] a входной параметр преобразования
 * @param[in] b входной параметр преобразования
 * @param[out] outdata результат параметр преобразования
 * @param[in] print функция логирования
 * @return 0 если все преобразование прошло успешно
 * @return -1 если произошла ошибка
 */
int  funcReverseLSX(unsigned char* a, unsigned char* b, unsigned char* outdata);

/** @brief Преобразование F
 * 
 * @param[in] inputKey Первый ключ  из пары ключей полученной в предедущей итерации
 * @param[in] inputKeySecond Второй ключ  из пары ключей полученной в предедущей итерации
 * @param[in] iterationConst Итерационная константа
 * @param[out] outputKey Первый ключ
 * @param[out] outputKeySecond Второй ключ
 * @param[in] print функция логирования
 * @return 0 если все преобразование прошло успешно
 * @return -1 если произошла ошибка
 */
int  funcF(unsigned char* inputKey, unsigned char* inputKeySecond, unsigned char* iterationConst, unsigned char* outputKey, unsigned char* outputKeySecond);

/** @brief Вычисление итерационной константы С 
 * 
 * @param[in] number номер константы
 * @param[out] output итерационная константа
 * @param[in] print функция логирования
 * @return 0 если все преобразование прошло успешно
 * @return -1 если произошла ошибка
 */
int  funcC(unsigned char number, unsigned char* outputW);

/** @brief Развертка ключа
 * 
 * @param[in] masterKey Мастер ключ
 * @param[out] keys массив развернутых ключей
 * @param[in] print функция логирования
 * @return 0 если все преобразование прошло успешно
 * @return -1 если произошла ошибка
 */
int ExpandKey(unsigned char* masterKey, unsigned char* keys);

/** @brief Выполнение зашифрования блока
 * 
 * @param[in] plainText Исходный блок
 * @param[out] chipherText Зашифрованный блок
 * @param[in] keys Развернутые ключи
 * @param[in] print функция логирования
 * @param[in] print_uint функция логирования
 * @return 0 если все преобразование прошло успешно
 * @return -1 если произошла ошибка
 */
int Encrypt_14(unsigned char* plainText, unsigned char* chipherText, unsigned char* keys);

/** @brief Выполение расшифрования блока
 * 
 * @param[in] chipherText Зашифрованный блок
 * @param[out] plainText Расшифрованный блок
 * @param[in] keys Развернутые ключи
 * @param[in] print функция логирования
 * @param[in] print_uint функция логирования
 * @return 0 если все преобразование прошло успешно
 * @return -1 если произошла ошибка
 */
int Decrypt_14(unsigned char* chipherText, unsigned char* plainText, unsigned char* keys);

#ifdef __cplusplus
}
#endif

#endif
