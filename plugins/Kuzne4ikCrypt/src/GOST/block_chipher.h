/** @file 
 * @brief Объявление функций реализующих режимы работы блочных алгоритмов
 *
 * @copyright InfoTeCS. All rights reserved.
 */

#ifndef BLOCK_CHIPHER_H
#define BLOCK_CHIPHER_H

#define DLL_IMPORT

/** @brief Размер контекста для режима шифрования ECB алгоритма "кузнечик" */
#define kEcb14ContextLen 0x28

/** @brief Размер контекста для режима шифрования ECB алгоритма 28147-89 */
#define kEcb89ContextLen 0x28

/** @brief Размер контекста для режима шифрования CBC алгоритма "кузнечик" */
#define kCbc14ContextLen 0x38

/** @brief Размер контекста для режима шифрования CBC алгоритма 28147-89 */
#define kCbc89ContextLen 0x38

/** @brief Размер контекста для режима шифрования CRT алгоритма "кузнечик" */
#define kCrt14ContextLen 0x30

/** @brief Размер контекста для режима шифрования CRT алгоритма 28147-89 */
#define kCrt89ContextLen 0x30

/** @brief Размер контекста для режима шифрования OFB алгоритма "кузнечик" */
#define kOfb14ContextLen 0x40

/** @brief Размер контекста для режима шифрования OFB алгоритма 28147-89 */
#define kOfb89ContextLen 0x40

/** @brief Размер контекста для режима шифрования CFB алгоритма "кузнечик" */
#define kCfb14ContextLen 0x40

/** @brief Размер контекста для режима шифрования CFB алгоритма 28147-89 */
#define kCfb89ContextLen 0x40

/** @brief Размер контекста для режима формирования имитовставки алгоритма "кузнечик" */
#define kImit14ContextLen 0x68

/** @brief Размер контекста для режима формирования имитовставки алгоритма 28147-89 */
#define kImit89ContextLen 0x68

/** @brief Размер блока алгоритма "кузнечик" */
#define kBlockLen14 16

/** @brief Размер блока алгоритма 28147-89 */
#define kBlockLen89 8

/** @brief Размер ключа алгоритма 28147-89 */
#define kKeyLen89 32

#ifdef __cplusplus
extern "C" {
#endif

/* *** Режимы шифрования ***
 * Режимы шифрования работают с использованием базового 
 * криптографического преобразования
 */

/** @brief Инициализация контекста шифрования в режиме ECB для алгоритма "кузнечик"
 *
 * @param[in] key ключ
 * @param[out] ctx контекст cbc
 * @param[in] print функция логирования
 * @param[in] print_uint функция логирования
 * @return 0 если все преобразование прошло успешно
 * @return -1 если произошла ошибка
 */
int DLL_IMPORT init_ecb_14(unsigned char *key, void* ctx);

/** @brief Удаление контекста ecb
 *
 * @param[in] ctx контекст ecb
 * @return 0 если все преобразование прошло успешно
 * @return -1 если произошла ошибка
 */
void DLL_IMPORT free_ecb(void* ctx);

/** @brief Инициализация контекста шифрования в режиме CBC для алгоритма "кузнечик"
 *
 * @param[in] key ключ
 * @param[out] ctx контекст cbc
 * @param[in] iv синхропосылка
 * @param[in] ivLength длинна синхропосылки
 * @param[in] print функция логирования
 * @param[in] print_uint функция логирования
 * @return 0 если все преобразование прошло успешно
 * @return -1 если произошла ошибка
 */
int DLL_IMPORT  init_cbc_14(unsigned char *key, void* ctx, const unsigned char *iv, size_t ivLength);

/** @brief Удаление контекста cbc
 *
 * @param[in] ctx контекст cbc
 * @return 0 если все преобразование прошло успешно
 * @return -1 если произошла ошибка
 */
void DLL_IMPORT free_cbc(void* ctx);

/** @brief Инициализация контекста шифрования в режиме CRT для алгоритма "кузнечик"
 *
 * @param[in] key ключ
 * @param[out] ctx контекст crt
 * @param[in] iv синхропосылка
 * @param[in] length длинна синхропосылки
 * @param[in] print функция логирования
 * @param[in] print_uint функция логирования
 * @return 0 если все преобразование прошло успешно
 * @return -1 если произошла ошибка
 */
int DLL_IMPORT init_crt_14(unsigned char* key, unsigned char *iv, size_t length, void *ctx);

/** @brief Удаление контекста crt
 *
 * @param[in] ctx контекст crt
 * @return 0 если все преобразование прошло успешно
 * @return -1 если произошла ошибка
 */
void DLL_IMPORT free_crt(void* ctx);

/** @brief Инициализация контекста шифрования в режиме OFB для алгоритма "кузнечик"
 *
 * @param[in] key ключ
 * @param[out] ctx контекст ofb
 * @param[in] s параметр S
 * @param[in] iv синхропосылка
 * @param[in] ivLength длинна синхропосылки
 * @param[in] print функция логирования
 * @param[in] print_uint функция логирования
 * @return 0 если все преобразование прошло успешно
 * @return -1 если произошла ошибка
 */
int DLL_IMPORT init_ofb_14(unsigned char *key, void *ctx, size_t s, unsigned char *iv, size_t ivLength);


/** @brief Удаление контекста ofb
 *
 * @param[in] ctx контекст ofb
 * @return 0 если все преобразование прошло успешно
 * @return -1 если произошла ошибка
 */
void DLL_IMPORT free_ofb(void* ctx);

/** @brief Инициализация контекста шифрования в режиме CFB для алгоритма "кузнечик"
 *
 * @param[in] key ключ
 * @param[out] ctx контекст cfb
 * @param[in] s параметр S
 * @param[in] iv синхропосылка
 * @param[in] ivLength длинна синхропосылки
 * @param[in] print функция логирования
 * @param[in] print_uint функция логирования
 * @return 0 если все преобразование прошло успешно
 * @return -1 если произошла ошибка
 */
int DLL_IMPORT init_cfb_14(unsigned char *key, void *ctx, size_t s, unsigned char *iv, size_t ivLength);

/** @brief Инициализация контекста шифрования в режиме CFB для алгоритма 28147-89
 *
 * @param[in] key ключ
 * @param[out] ctx контекст cfb
 * @param[in] s параметр S
 * @param[in] iv синхропосылка
 * @param[in] ivLength длинна синхропосылки
 * @param[in] print функция логирования
 * @param[in] print_uint функция логирования
 * @return 0 если все преобразование прошло успешно
 * @return -1 если произошла ошибка
 */
int DLL_IMPORT init_cfb_89(unsigned char *key, void *ctx, size_t s, unsigned char *iv, size_t ivLength);

/** @brief Удаление контекста cfb
 *
 * @param[in] ctx контекст cfb
 * @return 0 если все преобразование прошло успешно
 * @return -1 если произошла ошибка
 */
void DLL_IMPORT free_cfb(void* ctx);

/** @brief Инициализация контекста имтовставки для алгоритма "кузнечик"
 *
 * @param[out] ctx контекст имитовставки
 * @param[in] key ключ
 * @param[in] s параметр S
 * @param[in] print функция логирования
 * @param[in] print_uint функция логирования
 * @return 0 если все преобразование прошло успешно
 * @return -1 если произошла ошибка
 */
int DLL_IMPORT init_imit_14(unsigned char *key, size_t s, void *ctx);

/** @brief Инициализация контекста имтовставки для алгоритма 28147-89
 *
 * @param[in] key ключ
 * @param[in] s параметр S
 * @param[out] ctx контекст имитовставки
 * @param[in] print функция логирования
 * @param[in] print_uint функция логирования
 * @return 0 если все преобразование прошло успешно
 * @return -1 если произошла ошибка
 */
int DLL_IMPORT init_imit_89(unsigned char *key, size_t s, void *ctx);

/** @brief Удаление контекста имитовставки
 *
 * @param[in] ctx контекст имитовставки
 * @return 0 если все преобразование прошло успешно
 * @return -1 если произошла ошибка
 */
void DLL_IMPORT free_imit(void* ctx);

/** @brief Выполнение зашифрования информации в режиме простой замены для данных кратных размеру блока
 *
 * @param[in] ctx контекст ECB
 * @param[in] indata открытый текст
 * @param[out] outdata зашифрованный текст
 * @param[in] length длинна текста
 * @return 0 если все преобразование прошло успешно
 * @return -1 если произошла ошибка
 */
int DLL_IMPORT encrypt_ecb(void *ctx, unsigned char *indata, unsigned char *outdata, size_t length);

/** @brief Выполнение расшифрования информации в режиме простой замены для данных кратных размеру блока
 *
 * @param[in] ctx контекст ECB
 * @param[in] indata открытый текст
 * @param[out] outdata зашифрованный текст
 * @param[in] length длинна текста
 * @return 0 если все преобразование прошло успешно
 * @return -1 если произошла ошибка
 */
int DLL_IMPORT decrypt_ecb(void *ctx, unsigned char *indata, unsigned char *outdata, size_t length);

/** @brief Выполнение зашифрования информации в режиме простой замены с зацеплением для данных кратных размеру блока
 *
 * @param[in] ctx контекст CBC
 * @param[in] indata открытый текст
 * @param[out] outdata зашифрованный текст
 * @param[in] length длинна текста
 * @return 0 если все преобразование прошло успешно
 * @return -1 если произошла ошибка
 */
int DLL_IMPORT encrypt_cbc(void *ctx, unsigned char *indata, unsigned char *outdata, size_t length);

/** @brief Выполнение рсшифрования информации в режиме простой замены с зацеплением для данных кратных размеру блока
 *
 * @param[in] ctx контекст CBC
 * @param[in] indata зашифрованный текст
 * @param[out] outdata расшифрованный текст
 * @param[in] length длинна текста
 * @return 0 если все преобразование прошло успешно
 * @return -1 если произошла ошибка
 */
int DLL_IMPORT decrypt_cbc(void *ctx, unsigned char *indata, unsigned char *outdata, size_t length);

/** @brief Выполнение шифрования (зашифрования или расшифрования) в режиме гаммирования
 *
 * @param[in] ctx контекст CRT
 * @param[in] indata входное сообщение
 * @param[out] outdata результат
 * @param[in] length длинна сообщения
 * @return 0 если все преобразование прошло успешно
 * @return -1 если произошла ошибка
 */
int DLL_IMPORT crypt_crt(void *ctx, unsigned char *indata, unsigned char *outdata, size_t length);

/** @brief Выполнение шаговой шифрования информации в режиме гаммирования с обратной связью
 *
 * @param[in] ctx контекст OFB
 * @param[in] indata входной блок
 * @param[out] outdata результат преобразования
 * @param[in] inlength длинна текста
 * @return 0 если все преобразование прошло успешно
 * @return -1 если произошла ошибка
 */
int DLL_IMPORT crypt_ofb(void *ctx, unsigned char *indata, unsigned char *outdata, size_t inlength);

/** @brief Выполнение зашифрования информации в режиме гаммирования с обратной связью
 *
 * @param[in] ctx контекст OFB
 * @param[in] indata открытый текст
 * @param[out] outdata зашифрованный текст
 * @param[in] inlength длинна текста
 * @return 0 если все преобразование прошло успешно
 * @return -1 если произошла ошибка
 */
int DLL_IMPORT encrypt_ofb(void *ctx, unsigned char *indata, unsigned char *outdata, size_t inlength);

/** @brief Выполнение расшифрования информации в режиме гаммирования с обратной связью
 *
 * @param[in] ctx контекст OFB
 * @param[in] indata зашифрованный текст
 * @param[out] outdata расшифрованный текст
 * @param[in] inlength длинна текста
 * @return 0 если все преобразование прошло успешно
 * @return -1 если произошла ошибка
 */
int DLL_IMPORT decrypt_ofb(void *ctx, unsigned char *indata, unsigned char *outdata, size_t inlength);

/** @brief Выполнение зашифрования информации в режиме гаммирования с обратной связью по шифртексту
 *
 * @param[in] ctx контекст CFB
 * @param[in] indata открытый текст
 * @param[out] outdata зашифрованный текст
 * @param[in] inlength длинна текста
 * @return 0 если все преобразование прошло успешно
 * @return -1 если произошла ошибка
 */
int DLL_IMPORT encrypt_cfb(void *ctx, unsigned char *indata, unsigned char *outdata, size_t inlength);

/** @brief Выполнение расшифрования информации в режиме гаммирования с обратной связью по шифртексту
 *
 * @param[in] ctx контекст CFB
 * @param[in] indata зашифрованный текст
 * @param[out] outdata расшифрованный текст
 * @param[in] inlength длинна текста
 * @return 0 если все преобразование прошло успешно
 * @return -1 если произошла ошибка
 */
int DLL_IMPORT decrypt_cfb(void *ctx, unsigned char *indata, unsigned char *outdata, size_t inlength);

/** @brief Выполнение вычисления имитовставки по данным кратным размеру блока
 *
 * @param[in] ctx контекст имитовставки
 * @param[in] indata открытый текст
 * @param[in] length длинна текста
 * @return 0 если все преобразование прошло успешно
 * @return -1 если произошла ошибка
 */
int DLL_IMPORT imit(void *ctx, unsigned char *indata, size_t length);

/** @brief Завершение выроботки имитовставки
 *
 * @param[in] ctx контекст имитовставки
 * @param[out] value
 * @return 0 если все преобразование прошло успешно
 * @return -1 если произошла ошибка
 */
int DLL_IMPORT done_imit(void *ctx, unsigned char *value);

/** @brief Дополнение данных до размера блока. 
 *
 * @param[in] data сообщение. Память под данные data должна быть выделена, достаточная для дополнения.
 * @param[in] length размер сообщения
 * @param[in] blockLen длинна блока
 * @return размер сообщения
 */
int DLL_IMPORT padd(unsigned char *data, size_t length, size_t blockLen);

/** @brief Удаление дополненных данных. При ошибках возвращается значение -1
 *
 * @param[in] data сообщение
 * @param[in] length размер сообщения
 * @return размер сообщения
 */
int DLL_IMPORT unpadd(unsigned char *data, size_t length);

#ifdef __cplusplus
}
#endif

#endif
