/* Output of mkstrtable.awk.  DO NOT EDIT.  */

/* err-codes.h - List of error codes and their description.
   Copyright (C) 2003, 2004 g10 Code GmbH

   This file is part of libgpg-error.

   libgpg-error is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public License
   as published by the Free Software Foundation; either version 2.1 of
   the License, or (at your option) any later version.
 
   libgpg-error is distributed in the hope that it will be useful, but
   WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.
 
   You should have received a copy of the GNU Lesser General Public
   License along with libgpg-error; if not, write to the Free
   Software Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
   02111-1307, USA.  */


/* The purpose of this complex string table is to produce
   optimal code with a minimum of relocations.  */

static const char msgstr[] = 
  "GPG_ERR_NO_ERROR" "\0"
  "GPG_ERR_GENERAL" "\0"
  "GPG_ERR_UNKNOWN_PACKET" "\0"
  "GPG_ERR_UNKNOWN_VERSION" "\0"
  "GPG_ERR_PUBKEY_ALGO" "\0"
  "GPG_ERR_DIGEST_ALGO" "\0"
  "GPG_ERR_BAD_PUBKEY" "\0"
  "GPG_ERR_BAD_SECKEY" "\0"
  "GPG_ERR_BAD_SIGNATURE" "\0"
  "GPG_ERR_NO_PUBKEY" "\0"
  "GPG_ERR_CHECKSUM" "\0"
  "GPG_ERR_BAD_PASSPHRASE" "\0"
  "GPG_ERR_CIPHER_ALGO" "\0"
  "GPG_ERR_KEYRING_OPEN" "\0"
  "GPG_ERR_INV_PACKET" "\0"
  "GPG_ERR_INV_ARMOR" "\0"
  "GPG_ERR_NO_USER_ID" "\0"
  "GPG_ERR_NO_SECKEY" "\0"
  "GPG_ERR_WRONG_SECKEY" "\0"
  "GPG_ERR_BAD_KEY" "\0"
  "GPG_ERR_COMPR_ALGO" "\0"
  "GPG_ERR_NO_PRIME" "\0"
  "GPG_ERR_NO_ENCODING_METHOD" "\0"
  "GPG_ERR_NO_ENCRYPTION_SCHEME" "\0"
  "GPG_ERR_NO_SIGNATURE_SCHEME" "\0"
  "GPG_ERR_INV_ATTR" "\0"
  "GPG_ERR_NO_VALUE" "\0"
  "GPG_ERR_NOT_FOUND" "\0"
  "GPG_ERR_VALUE_NOT_FOUND" "\0"
  "GPG_ERR_SYNTAX" "\0"
  "GPG_ERR_BAD_MPI" "\0"
  "GPG_ERR_INV_PASSPHRASE" "\0"
  "GPG_ERR_SIG_CLASS" "\0"
  "GPG_ERR_RESOURCE_LIMIT" "\0"
  "GPG_ERR_INV_KEYRING" "\0"
  "GPG_ERR_TRUSTDB" "\0"
  "GPG_ERR_BAD_CERT" "\0"
  "GPG_ERR_INV_USER_ID" "\0"
  "GPG_ERR_UNEXPECTED" "\0"
  "GPG_ERR_TIME_CONFLICT" "\0"
  "GPG_ERR_KEYSERVER" "\0"
  "GPG_ERR_WRONG_PUBKEY_ALGO" "\0"
  "GPG_ERR_TRIBUTE_TO_D_A" "\0"
  "GPG_ERR_WEAK_KEY" "\0"
  "GPG_ERR_INV_KEYLEN" "\0"
  "GPG_ERR_INV_ARG" "\0"
  "GPG_ERR_BAD_URI" "\0"
  "GPG_ERR_INV_URI" "\0"
  "GPG_ERR_NETWORK" "\0"
  "GPG_ERR_UNKNOWN_HOST" "\0"
  "GPG_ERR_SELFTEST_FAILED" "\0"
  "GPG_ERR_NOT_ENCRYPTED" "\0"
  "GPG_ERR_NOT_PROCESSED" "\0"
  "GPG_ERR_UNUSABLE_PUBKEY" "\0"
  "GPG_ERR_UNUSABLE_SECKEY" "\0"
  "GPG_ERR_INV_VALUE" "\0"
  "GPG_ERR_BAD_CERT_CHAIN" "\0"
  "GPG_ERR_MISSING_CERT" "\0"
  "GPG_ERR_NO_DATA" "\0"
  "GPG_ERR_BUG" "\0"
  "GPG_ERR_NOT_SUPPORTED" "\0"
  "GPG_ERR_INV_OP" "\0"
  "GPG_ERR_TIMEOUT" "\0"
  "GPG_ERR_INTERNAL" "\0"
  "GPG_ERR_EOF_GCRYPT" "\0"
  "GPG_ERR_INV_OBJ" "\0"
  "GPG_ERR_TOO_SHORT" "\0"
  "GPG_ERR_TOO_LARGE" "\0"
  "GPG_ERR_NO_OBJ" "\0"
  "GPG_ERR_NOT_IMPLEMENTED" "\0"
  "GPG_ERR_CONFLICT" "\0"
  "GPG_ERR_INV_CIPHER_MODE" "\0"
  "GPG_ERR_INV_FLAG" "\0"
  "GPG_ERR_INV_HANDLE" "\0"
  "GPG_ERR_TRUNCATED" "\0"
  "GPG_ERR_INCOMPLETE_LINE" "\0"
  "GPG_ERR_INV_RESPONSE" "\0"
  "GPG_ERR_NO_AGENT" "\0"
  "GPG_ERR_AGENT" "\0"
  "GPG_ERR_INV_DATA" "\0"
  "GPG_ERR_ASSUAN_SERVER_FAULT" "\0"
  "GPG_ERR_ASSUAN" "\0"
  "GPG_ERR_INV_SESSION_KEY" "\0"
  "GPG_ERR_INV_SEXP" "\0"
  "GPG_ERR_UNSUPPORTED_ALGORITHM" "\0"
  "GPG_ERR_NO_PIN_ENTRY" "\0"
  "GPG_ERR_PIN_ENTRY" "\0"
  "GPG_ERR_BAD_PIN" "\0"
  "GPG_ERR_INV_NAME" "\0"
  "GPG_ERR_BAD_DATA" "\0"
  "GPG_ERR_INV_PARAMETER" "\0"
  "GPG_ERR_WRONG_CARD" "\0"
  "GPG_ERR_NO_DIRMNGR" "\0"
  "GPG_ERR_DIRMNGR" "\0"
  "GPG_ERR_CERT_REVOKED" "\0"
  "GPG_ERR_NO_CRL_KNOWN" "\0"
  "GPG_ERR_CRL_TOO_OLD" "\0"
  "GPG_ERR_LINE_TOO_LONG" "\0"
  "GPG_ERR_NOT_TRUSTED" "\0"
  "GPG_ERR_CANCELED" "\0"
  "GPG_ERR_BAD_CA_CERT" "\0"
  "GPG_ERR_CERT_EXPIRED" "\0"
  "GPG_ERR_CERT_TOO_YOUNG" "\0"
  "GPG_ERR_UNSUPPORTED_CERT" "\0"
  "GPG_ERR_UNKNOWN_SEXP" "\0"
  "GPG_ERR_UNSUPPORTED_PROTECTION" "\0"
  "GPG_ERR_CORRUPTED_PROTECTION" "\0"
  "GPG_ERR_AMBIGUOUS_NAME" "\0"
  "GPG_ERR_CARD" "\0"
  "GPG_ERR_CARD_RESET" "\0"
  "GPG_ERR_CARD_REMOVED" "\0"
  "GPG_ERR_INV_CARD" "\0"
  "GPG_ERR_CARD_NOT_PRESENT" "\0"
  "GPG_ERR_NO_PKCS15_APP" "\0"
  "GPG_ERR_NOT_CONFIRMED" "\0"
  "GPG_ERR_CONFIGURATION" "\0"
  "GPG_ERR_NO_POLICY_MATCH" "\0"
  "GPG_ERR_INV_INDEX" "\0"
  "GPG_ERR_INV_ID" "\0"
  "GPG_ERR_NO_SCDAEMON" "\0"
  "GPG_ERR_SCDAEMON" "\0"
  "GPG_ERR_UNSUPPORTED_PROTOCOL" "\0"
  "GPG_ERR_BAD_PIN_METHOD" "\0"
  "GPG_ERR_CARD_NOT_INITIALIZED" "\0"
  "GPG_ERR_UNSUPPORTED_OPERATION" "\0"
  "GPG_ERR_WRONG_KEY_USAGE" "\0"
  "GPG_ERR_NOTHING_FOUND" "\0"
  "GPG_ERR_WRONG_BLOB_TYPE" "\0"
  "GPG_ERR_MISSING_VALUE" "\0"
  "GPG_ERR_HARDWARE" "\0"
  "GPG_ERR_PIN_BLOCKED" "\0"
  "GPG_ERR_USE_CONDITIONS" "\0"
  "GPG_ERR_PIN_NOT_SYNCED" "\0"
  "GPG_ERR_INV_CRL" "\0"
  "GPG_ERR_BAD_BER" "\0"
  "GPG_ERR_INV_BER" "\0"
  "GPG_ERR_ELEMENT_NOT_FOUND" "\0"
  "GPG_ERR_IDENTIFIER_NOT_FOUND" "\0"
  "GPG_ERR_INV_TAG" "\0"
  "GPG_ERR_INV_LENGTH" "\0"
  "GPG_ERR_INV_KEYINFO" "\0"
  "GPG_ERR_UNEXPECTED_TAG" "\0"
  "GPG_ERR_NOT_DER_ENCODED" "\0"
  "GPG_ERR_NO_CMS_OBJ" "\0"
  "GPG_ERR_INV_CMS_OBJ" "\0"
  "GPG_ERR_UNKNOWN_CMS_OBJ" "\0"
  "GPG_ERR_UNSUPPORTED_CMS_OBJ" "\0"
  "GPG_ERR_UNSUPPORTED_ENCODING" "\0"
  "GPG_ERR_UNSUPPORTED_CMS_VERSION" "\0"
  "GPG_ERR_UNKNOWN_ALGORITHM" "\0"
  "GPG_ERR_INV_ENGINE" "\0"
  "GPG_ERR_PUBKEY_NOT_TRUSTED" "\0"
  "GPG_ERR_DECRYPT_FAILED" "\0"
  "GPG_ERR_KEY_EXPIRED" "\0"
  "GPG_ERR_SIG_EXPIRED" "\0"
  "GPG_ERR_ENCODING_PROBLEM" "\0"
  "GPG_ERR_INV_STATE" "\0"
  "GPG_ERR_DUP_VALUE" "\0"
  "GPG_ERR_MISSING_ACTION" "\0"
  "GPG_ERR_MODULE_NOT_FOUND" "\0"
  "GPG_ERR_INV_OID_STRING" "\0"
  "GPG_ERR_INV_TIME" "\0"
  "GPG_ERR_INV_CRL_OBJ" "\0"
  "GPG_ERR_UNSUPPORTED_CRL_VERSION" "\0"
  "GPG_ERR_INV_CERT_OBJ" "\0"
  "GPG_ERR_UNKNOWN_NAME" "\0"
  "GPG_ERR_LOCALE_PROBLEM" "\0"
  "GPG_ERR_NOT_LOCKED" "\0"
  "GPG_ERR_PROTOCOL_VIOLATION" "\0"
  "GPG_ERR_INV_MAC" "\0"
  "GPG_ERR_INV_REQUEST" "\0"
  "GPG_ERR_UNKNOWN_EXTN" "\0"
  "GPG_ERR_UNKNOWN_CRIT_EXTN" "\0"
  "GPG_ERR_LOCKED" "\0"
  "GPG_ERR_UNKNOWN_OPTION" "\0"
  "GPG_ERR_UNKNOWN_COMMAND" "\0"
  "GPG_ERR_NOT_OPERATIONAL" "\0"
  "GPG_ERR_NO_PASSPHRASE" "\0"
  "GPG_ERR_NO_PIN" "\0"
  "GPG_ERR_NOT_ENABLED" "\0"
  "GPG_ERR_NO_ENGINE" "\0"
  "GPG_ERR_MISSING_KEY" "\0"
  "GPG_ERR_TOO_MANY" "\0"
  "GPG_ERR_LIMIT_REACHED" "\0"
  "GPG_ERR_UNFINISHED" "\0"
  "GPG_ERR_BUFFER_TOO_SHORT" "\0"
  "GPG_ERR_SEXP_INV_LEN_SPEC" "\0"
  "GPG_ERR_SEXP_STRING_TOO_LONG" "\0"
  "GPG_ERR_SEXP_UNMATCHED_PAREN" "\0"
  "GPG_ERR_SEXP_NOT_CANONICAL" "\0"
  "GPG_ERR_SEXP_BAD_CHARACTER" "\0"
  "GPG_ERR_SEXP_BAD_QUOTATION" "\0"
  "GPG_ERR_SEXP_ZERO_PREFIX" "\0"
  "GPG_ERR_SEXP_NESTED_DH" "\0"
  "GPG_ERR_SEXP_UNMATCHED_DH" "\0"
  "GPG_ERR_SEXP_UNEXPECTED_PUNC" "\0"
  "GPG_ERR_SEXP_BAD_HEX_CHAR" "\0"
  "GPG_ERR_SEXP_ODD_HEX_NUMBERS" "\0"
  "GPG_ERR_SEXP_BAD_OCT_CHAR" "\0"
  "GPG_ERR_ASS_GENERAL" "\0"
  "GPG_ERR_ASS_ACCEPT_FAILED" "\0"
  "GPG_ERR_ASS_CONNECT_FAILED" "\0"
  "GPG_ERR_ASS_INV_RESPONSE" "\0"
  "GPG_ERR_ASS_INV_VALUE" "\0"
  "GPG_ERR_ASS_INCOMPLETE_LINE" "\0"
  "GPG_ERR_ASS_LINE_TOO_LONG" "\0"
  "GPG_ERR_ASS_NESTED_COMMANDS" "\0"
  "GPG_ERR_ASS_NO_DATA_CB" "\0"
  "GPG_ERR_ASS_NO_INQUIRE_CB" "\0"
  "GPG_ERR_ASS_NOT_A_SERVER" "\0"
  "GPG_ERR_ASS_NOT_A_CLIENT" "\0"
  "GPG_ERR_ASS_SERVER_START" "\0"
  "GPG_ERR_ASS_READ_ERROR" "\0"
  "GPG_ERR_ASS_WRITE_ERROR" "\0"
  "GPG_ERR_ASS_TOO_MUCH_DATA" "\0"
  "GPG_ERR_ASS_UNEXPECTED_CMD" "\0"
  "GPG_ERR_ASS_UNKNOWN_CMD" "\0"
  "GPG_ERR_ASS_SYNTAX" "\0"
  "GPG_ERR_ASS_CANCELED" "\0"
  "GPG_ERR_ASS_NO_INPUT" "\0"
  "GPG_ERR_ASS_NO_OUTPUT" "\0"
  "GPG_ERR_ASS_PARAMETER" "\0"
  "GPG_ERR_ASS_UNKNOWN_INQUIRE" "\0"
  "GPG_ERR_USER_1" "\0"
  "GPG_ERR_USER_2" "\0"
  "GPG_ERR_USER_3" "\0"
  "GPG_ERR_USER_4" "\0"
  "GPG_ERR_USER_5" "\0"
  "GPG_ERR_USER_6" "\0"
  "GPG_ERR_USER_7" "\0"
  "GPG_ERR_USER_8" "\0"
  "GPG_ERR_USER_9" "\0"
  "GPG_ERR_USER_10" "\0"
  "GPG_ERR_USER_11" "\0"
  "GPG_ERR_USER_12" "\0"
  "GPG_ERR_USER_13" "\0"
  "GPG_ERR_USER_14" "\0"
  "GPG_ERR_USER_15" "\0"
  "GPG_ERR_USER_16" "\0"
  "GPG_ERR_MISSING_ERRNO" "\0"
  "GPG_ERR_UNKNOWN_ERRNO" "\0"
  "GPG_ERR_EOF" "\0"
  "GPG_ERR_CODE_DIM";

static const int msgidx[] =
  {
    0,
    17,
    33,
    56,
    80,
    100,
    120,
    139,
    158,
    180,
    198,
    215,
    238,
    258,
    279,
    298,
    316,
    335,
    353,
    374,
    390,
    409,
    426,
    453,
    482,
    510,
    527,
    544,
    562,
    586,
    601,
    617,
    640,
    658,
    681,
    701,
    717,
    734,
    754,
    773,
    795,
    813,
    839,
    862,
    879,
    898,
    914,
    930,
    946,
    962,
    983,
    1007,
    1029,
    1051,
    1075,
    1099,
    1117,
    1140,
    1161,
    1177,
    1189,
    1211,
    1226,
    1242,
    1259,
    1278,
    1294,
    1312,
    1330,
    1345,
    1369,
    1386,
    1410,
    1427,
    1446,
    1464,
    1488,
    1509,
    1526,
    1540,
    1557,
    1585,
    1600,
    1624,
    1641,
    1671,
    1692,
    1710,
    1726,
    1743,
    1760,
    1782,
    1801,
    1820,
    1836,
    1857,
    1878,
    1898,
    1920,
    1940,
    1957,
    1977,
    1998,
    2021,
    2046,
    2067,
    2098,
    2127,
    2150,
    2163,
    2182,
    2203,
    2220,
    2245,
    2267,
    2289,
    2311,
    2335,
    2353,
    2368,
    2388,
    2405,
    2434,
    2457,
    2486,
    2516,
    2540,
    2562,
    2586,
    2608,
    2625,
    2645,
    2668,
    2691,
    2707,
    2723,
    2739,
    2765,
    2794,
    2810,
    2829,
    2849,
    2872,
    2896,
    2915,
    2935,
    2959,
    2987,
    3016,
    3048,
    3074,
    3093,
    3120,
    3143,
    3163,
    3183,
    3208,
    3226,
    3244,
    3267,
    3292,
    3315,
    3332,
    3352,
    3384,
    3405,
    3426,
    3449,
    3468,
    3495,
    3511,
    3531,
    3552,
    3578,
    3593,
    3616,
    3640,
    3664,
    3686,
    3701,
    3721,
    3739,
    3759,
    3776,
    3798,
    3817,
    3842,
    3868,
    3897,
    3926,
    3953,
    3980,
    4007,
    4032,
    4055,
    4081,
    4110,
    4136,
    4165,
    4191,
    4211,
    4237,
    4264,
    4289,
    4311,
    4339,
    4365,
    4393,
    4416,
    4442,
    4467,
    4492,
    4517,
    4540,
    4564,
    4590,
    4617,
    4641,
    4660,
    4681,
    4702,
    4724,
    4746,
    4774,
    4789,
    4804,
    4819,
    4834,
    4849,
    4864,
    4879,
    4894,
    4909,
    4925,
    4941,
    4957,
    4973,
    4989,
    5005,
    5021,
    5043,
    5065,
    5077
  };

static inline int
msgidxof (int code)
{
  return (0 ? 0
  : ((code >= 0) && (code <= 183)) ? (code - 0)
  : ((code >= 199) && (code <= 213)) ? (code - 15)
  : ((code >= 257) && (code <= 271)) ? (code - 58)
  : ((code >= 273) && (code <= 281)) ? (code - 59)
  : ((code >= 1024) && (code <= 1039)) ? (code - 801)
  : ((code >= 16381) && (code <= 16383)) ? (code - 16142)
  : 16384 - 16142);
}
