
#if defined(OS400) && (__OS400_TGTVRM__ >= 510)
#pragma datamodel(P128)
#endif

/*********************************************************************/
/*                                                                   */
/* Licensed Materials - Property of IBM                              */
/*                                                                   */
/* L-GHUS-5VMPGW, L-GHUS-5S3PEE                                      */
/* (C) Copyright IBM Corp. 1995, 2005  All Rights Reserved           */
/*                                                                   */
/* US Government Users Restricted Rights - Use, duplication or       */
/* disclosure restricted by GSA ADP Schedule Contract with           */
/* IBM Corp.                                                         */
/*                                                                   */
/*********************************************************************/




#if   !defined(NLS_H)
#define  NLS_H

#ifdef __cplusplus
extern "C" {
#endif


/********************************************************************
**                TYPES AND DEFINITIONS
*/

typedef  WORD  NLS_STATUS;
            
typedef void NLS_INFO;        
typedef void far *NLS_PINFO;        

/********************************************************************
**                PROTOS
*/

NLS_STATUS  LNPUBLIC NLS_load_charset     (WORD CSID,
                                    NLS_PINFO FAR *   ppInfo);

NLS_STATUS  LNPUBLIC NLS_unload_charset      (NLS_PINFO     pInfo);

NLS_STATUS  LNPUBLIC    NLS_translate        (BYTE far *       pString, 
                                       WORD           Len, 
                                       BYTE far *        pStringTarget, 
                                       WORD far *        pSize, 
                                       WORD           ControlFlags,
                                       NLS_PINFO         pInfo);

NLS_STATUS LNPUBLIC     NLS_find          (BYTE far * far *    ppString, 
                                       WORD           Len, 
                                       const BYTE far *        pSetOfChars, 
                                       WORD           ControlFlags,
                                       NLS_PINFO         pInfo);

NLS_STATUS LNPUBLIC     NLS_find_substr      (BYTE far * far *    ppString, 
                                       WORD           Len1, 
                                       const BYTE far *        pSubString, 
                                       WORD           Len2, 
                                       NLS_PINFO         pInfo);

NLS_STATUS LNPUBLIC     NLS_get           (BYTE far * far *    ppString, 
                                    WORD           Len, 
                                    BYTE far *        pCharacter, 
                                    NLS_PINFO         pInfo);

WORD     LNPUBLIC NLS_goto_next        (BYTE far * far *    ppString, 
                                    WORD           Len,
                                    NLS_PINFO         pInfo);

WORD     LNPUBLIC NLS_goto_prev        (BYTE far * far *    ppString, 
                                    const BYTE far *        pStrStart, 
                                    NLS_PINFO         pInfo);
WORD     LNPUBLIC NLS_goto_next_word_end  (BYTE far * far *    ppString, 
                                    WORD           Len, 
                                    NLS_PINFO         pInfo);

WORD     LNPUBLIC NLS_goto_next_word_start(BYTE far * far *    ppString, 
                                    WORD           Len, 
                                    NLS_PINFO         pInfo);

WORD     LNPUBLIC NLS_goto_next_break  (BYTE far * far *    ppString,
                                    WORD              Len,
                                    NLS_PINFO         pInfo);

WORD     LNPUBLIC NLS_goto_prev_word_end  (BYTE far * far *    ppString, 
                                    const BYTE far *        pStrStart, 
                                    NLS_PINFO         pInfo);

WORD     LNPUBLIC NLS_goto_prev_word_start(BYTE far * far *    ppString, 
                                    const BYTE far *        pStrStart, 
                                    NLS_PINFO         pInfo);

NLS_STATUS  LNPUBLIC NLS_isalpha          (const BYTE far *       pCharacter,
                                    NLS_PINFO         pInfo);

NLS_STATUS  LNPUBLIC NLS_isalnum          (const BYTE far *       pCharacter,
                                    NLS_PINFO         pInfo);

NLS_STATUS  LNPUBLIC NLS_isarith          (const BYTE far *       pCharacter,
                                    NLS_PINFO         pInfo);

NLS_STATUS  LNPUBLIC NLS_iscntrl          (const BYTE far *       pCharacter,
                                    NLS_PINFO         pInfo);

NLS_STATUS  LNPUBLIC NLS_isdigit          (const BYTE far *       pCharacter,
                                    NLS_PINFO         pInfo);

NLS_STATUS  LNPUBLIC NLS_islower          (const BYTE far *       pCharacter,
                                    NLS_PINFO         pInfo);

NLS_STATUS  LNPUBLIC NLS_ispunct          (const BYTE far *       pCharacter,
                                    NLS_PINFO         pInfo);

NLS_STATUS  LNPUBLIC NLS_isspace          (const BYTE far *       pCharacter,
                                    NLS_PINFO         pInfo);

NLS_STATUS  LNPUBLIC NLS_isupper          (const BYTE far *       pCharacter,
                                    NLS_PINFO         pInfo);

NLS_STATUS  LNPUBLIC NLS_isleadbyte          (BYTE          Character,
                                    NLS_PINFO         pInfo);

NLS_STATUS  LNPUBLIC NLS_string_chars     (const BYTE far *       pString, 
                                    WORD           NumBytes,
                                    WORD far *        pNumChars,
                                    NLS_PINFO         pInfo);

NLS_STATUS  LNPUBLIC NLS_string_bytes     (const BYTE far *       pString,
                                    WORD           NumChars,
                                    WORD far *        pNumBytes,
                                    NLS_PINFO         pInfo);
NLS_STATUS  LNPUBLIC    NLS_put           (BYTE far * far *    ppString, 
                                    const BYTE far *        pCharacter, 
                                    NLS_PINFO         pInfo);

NLS_STATUS  LNPUBLIC    NLS_put_term         (BYTE far * far *    ppString, 
                                    const BYTE far *        pCharacter, 
                                    NLS_PINFO         pInfo);

/********************************************************************
**             RETURN CODES
*/
                           
#define  NLS_SUCCESS             0x0
#define  NLS_BADPARM             0xABAD 
#define  NLS_BUFFERTOOSMALL         0xA000 
#define  NLS_CHARSSTRIPPED          0x1000 
#define  NLS_ENDOFSTRING            0x1001 
#define  NLS_FALLBACKUSED        0x1002 
#define  NLS_FILEINVALID            0xA001 
#define  NLS_FILENOTFOUND        0xA002 
#define  NLS_FINDFAILED          0x1005
#define  NLS_INVALIDCHARACTER    0xA003 
#define  NLS_INVALIDDATA            0xA004 
#define  NLS_INVALIDENTRY        0xA005 
#define  NLS_INVALIDTABLE        0xA006 
#define  NLS_PROPNOTFOUND        0x1003 
#define  NLS_STARTOFSTRING          0x1004 
#define  NLS_STRINGSIZECHANGED         0x1006 
#define  NLS_TABLEHEADERINVALID     0xA007 
#define  NLS_TABLENOTFOUND       0x1007 

/********************************************************************
**                FLAGS
*/

/*    NLS_find */
#define  NLS_FIND_CHARINSET            1
#define  NLS_FIND_CHARNOTINSET      2
/*    NLS_translate */

#define  NLS_NONULLTERMINATE        0x1
#define  NLS_NULLTERMINATE       0x2
#define  NLS_STRIPUNKNOWN        0x4
#define  NLS_TARGETISLMBCS       0x8
#define  NLS_SOURCEISLMBCS       0x10
#define  NLS_TARGETISUNICODE        0x20
#define  NLS_SOURCEISUNICODE        0x40
#define  NLS_TARGETISPLATFORM    0x80
#define  NLS_SOURCEISPLATFORM    0x100


/********************************************************************
**                MISC DEFINES
*/

#define  NLS_NULLTERM         (WORD) -1
#define  NLS_NULLTERM32       (DWORD) -1

#define  NLS_SIZEOFNULL       1 /* for LMBCS implementation */

#define  NLS_MAXBYTESPERCHAR     3

#define  NLS_MAXBYTESPERCHAR_WITH_SURROGATE 6
#define  NLS_MAXRATIO_XLATE_FROM_LMBCS      4
#define  NLS_MAXRATIO_XLATE_TO_LMBCS        3


/********************************************************************
**                CHARACTER SET ID'S
*/

#if   !defined(NLS_NOCHARSETIDS)

#define NLS_CS_DEFAULT         0xFFFF
#define NLS_CS_LICS              0x0000 /* Lotus Intl Char Set (WK1) */
#define NLS_CS_IBMCP851          0x0001
#define NLS_CS_IBMCP852          0x0002
#define NLS_CS_IBMCP853          0x0003
#define NLS_CS_IBMCP857          0x0004
#define NLS_CS_IBMCP862          0x0005
#define NLS_CS_IBMCP864          0x0006
#define NLS_CS_IBMCP866          0x0007
#define NLS_CS_IBMCP437          0x0008
#define NLS_CS_IBMCP850          0x0009
#define NLS_CS_IBMCP855          0x000A
#define NLS_CS_IBMCP860          0x000B
#define NLS_CS_IBMCP861          0x000C
#define NLS_CS_IBMCP863          0x000D
#define NLS_CS_IBMCP865          0x000E
#define NLS_CS_IBMCP869          0x000F
#define NLS_CS_IBMCP874          0x0090
#define NLS_CS_IBMCP899          0x0011
#define NLS_CS_IBMCP932          0x0012
#define NLS_CS_IBMCP942          0x0012 /* 932  942 for Lotus */
#define NLS_CS_IBMCP943          0x0012
#define NLS_CS_IBMCP5039         0x0012
#define NLS_CS_IBMCP891          0x0013
#define NLS_CS_DECMCS            0x0014 /* DEC Multinational Char Set */

#define NLS_CS_EUC               0x0017 /* Extended Unix Code */
#define NLS_CS_KS                0x0018 /* Korean - KSC 5601 */
#define NLS_CS_IBMCP949          0x0018
#define NLS_CS_TCA               0x0019
#define NLS_CS_BIG5              0x001A /* Taiwan Chinese - traditional */
#define NLS_CS_IBMCP950          0x001A
#define NLS_CS_GB                 0x001B /* PRC Chinese - simplified */
#define NLS_CS_IBMCP936          0x001B
#define NLS_CS_NECESJIS          0x001C /* NEC Extended Shift-JIS */
#define NLS_CS_ISO646            0x001F /* aka 'ASCII' */
#define NLS_CS_ASCII             0x001F
#define NLS_CS_ISO88591          0x0020 /* ISO Latin-1 */
#define NLS_CS_IBMCP819          0x0020
#define NLS_CS_ISO88592          0x0021 /* ISO Latin-2 (E. Europe) */
#define NLS_CS_IBMCP912          0x0021
#define NLS_CS_ISO88593          0x0022
#define NLS_CS_ISO88594          0x0023
#define NLS_CS_ISO88595          0x0024
#define NLS_CS_IBMCP915          0x0024
#define NLS_CS_ISO88596          0x0025
#define NLS_CS_IBMCP1008         0x0025
#define NLS_CS_ISO88597          0x0026
#define NLS_CS_IBMCP813          0x0026
#define NLS_CS_ISO88598          0x0027
#define NLS_CS_IBMCP916          0x0027
#define NLS_CS_ISO88599          0x0028
#define NLS_CS_IBMCP920          0x0028

#define NLS_CS_HPROMAN           0x0030 /* HP Roman (LaserJet) */
#define NLS_CS_HPGREEK           0x0031 /* HP Roman (LaserJet) */
#define NLS_CS_HPTURKISH         0x0032 /* HP Roman (LaserJet) */
#define NLS_CS_HPHEBREW          0x0034
#define NLS_CS_HPARABIC          0x0035
#define NLS_CS_HPTHAI            0x0036
#define NLS_CS_HPJAPAN           0x0037
#define NLS_CS_HPKANA            0x0038
#define NLS_CS_HPKOREA           0x0039
#define NLS_CS_HPPRC             0x003A
#define NLS_CS_HPROC             0x003B  /* Traditional Chinese */


#define NLS_CS_IBMCP37           0x0040 /* EBCDIC */
#define NLS_CS_IBMCP273          0x0041
#define NLS_CS_IBMCP278          0x0042
#define NLS_CS_IBMCP280          0x0043
#define NLS_CS_IBMCP284          0x0044
#define NLS_CS_IBMCP285          0x0045
#define NLS_CS_IBMCP290          0x0046
#define NLS_CS_IBMCP297          0x0047
#define NLS_CS_IBMCP500          0x0048
#define NLS_CS_IBMCP277          0x004C
#define NLS_CS_IBMCP1047         0x004D
#define NLS_CS_IBMCP1250         0x0050 /* Windows ANSI */
#define NLS_CS_IBMCP1251         0x0051
#define NLS_CS_IBMCP1252         0x0052
#define NLS_CS_ANSI              0X0052
#define NLS_CS_IBMCP1253         0x0053
#define NLS_CS_IBMCP1254         0x0054
#define NLS_CS_IBMCP1255         0x0055
#define NLS_CS_IBMCP1256         0x0056
#define NLS_CS_IBMCP1257         0x0057
#define NLS_CS_MACSCRIPT0        0x0060 /* Mac Roman */
#define NLS_CS_MACSCRIPT1        NLS_CS_IBMCP932  /*0x0061*/
#define NLS_CS_MACSCRIPT2        NLS_CS_GB      /*0x0062*/
#define NLS_CS_MACSCRIPT3        NLS_CS_KS      /*0x0063*/
#define NLS_CS_MACSCRIPT4        NLS_CS_ISO88596  /*0x0064*/
#define NLS_CS_MACSCRIPT5        NLS_CS_ISO88598  /*0x0065*/
#define NLS_CS_MACSCRIPT6        0x0066 /* cckSTRCharSetISO88597 */
#define NLS_CS_MACSCRIPT7        0x0067 /* cckSTRCharSetISO88595 */
#define NLS_CS_MACSCRIPT8        0x0068
#define NLS_CS_MACSCRIPT9        0x0069
#define NLS_CS_MACSCRIPT10       0x006A
#define NLS_CS_MACSCRIPT11       0x006B
#define NLS_CS_MACSCRIPT12       0x006C
#define NLS_CS_MACSCRIPT13       0x006D
#define NLS_CS_MACSCRIPT14       0x006E
#define NLS_CS_MACSCRIPT15       0x006F
#define NLS_CS_MACSCRIPT16       0x0070
#define NLS_CS_MACSCRIPT17       0x0071
#define NLS_CS_MACSCRIPT18       0x0072
#define NLS_CS_MACSCRIPT19       0x0073
#define NLS_CS_MACSCRIPT20       0x0074
#define NLS_CS_MACSCRIPT21       0x0075
#define NLS_CS_MACSCRIPT22       0x0076
#define NLS_CS_MACSCRIPT23       0x0077
#define NLS_CS_MACSCRIPT24       0x0078
#define NLS_CS_MACSCRIPT25       0x0079
#define NLS_CS_MACSCRIPT26       0x007A
#define NLS_CS_MACSCRIPT27       0x007B
#define NLS_CS_MACSCRIPT28       0x007C
#define NLS_CS_MACSCRIPT29       0x007D
#define NLS_CS_MACSCRIPT30       0x007E
#define NLS_CS_MACSCRIPT31       0x007F
#define NLS_CS_MACSCRIPT32       0x0080
#define NLS_CS_MACSCRIPT0CROATIAN    0x0081
#define NLS_CS_MACSCRIPT0GREEK       0x0082
#define NLS_CS_MACSCRIPT0ICELANDIC   0x0083
#define NLS_CS_MACSCRIPT0ROMANIAN    0x0084
#define NLS_CS_MACSCRIPT0TURKISH     0x0085
#define NLS_CS_THAI              0x0090 /* MS Thai Windows */
#define NLS_CS_IBMCP1200         0x00A0 /* Unicode/ISO 10646 */
#define NLS_CS_UNICODE           0x00A0
#define NLS_CS_UNICODE           0x00A0
#define NLS_CS_ISO10646          0x00A0 /* Also Unicode */
#define NLS_CS_UTF7              0x00AA /* Unicode Transformation Formats */
#define NLS_CS_UTF8              0x00AB
#define NLS_CS_LMBCS10           0x0100 /* Version 1.0 is the only one */
#define  NLS_CS_LMBCS11        0x0101
#define  NLS_CS_LMBCS12           0x0102
#define NLS_CS_LMBCS             0x0100
#define NLS_CS_DECNRCUK          0x0A00 /* DEC National Replacement Char */
#define NLS_CS_DECNRCDUTCH         0x0A01
#define NLS_CS_DECNRCFINNISH       0x0A02
#define NLS_CS_DECNRCFRENCH        0x0A03
#define NLS_CS_DECNRCFRENCHCANADIAN  0x0A04
#define NLS_CS_DECNRCGERMAN        0x0A05
#define NLS_CS_DECNRCITALIAN       0x0A06
#define NLS_CS_DECNRCNORWEGIANDANISH   0x0A07
#define NLS_CS_DECNRCPORTUGUESE      0x0A08
#define NLS_CS_DECNRCSPANISH       0x0A09
#define NLS_CS_DECNRCSWEDISH       0x0A0A
#define NLS_CS_DECNRCSWISS         0x0A0B
#define NLS_CS_T61               0x0B00
#define NLS_CS_T50               0x0B01
#define NLS_CS_ASN1              0x0B10
#define NLS_CS_IBMCP856          0x0C00
#define NLS_CS_IBMCP1004         0x0C01
#define NLS_CS_IBMCP1002         0x0CA0
#define NLS_CS_IBMCP1003         0x0CA1
#define NLS_CS_IBMCP1025         0x0CA2
#define NLS_CS_IBMCP1026         0x0CA3
#define NLS_CS_IBMCP1028         0x0CA4
#define NLS_CS_IBMCP256          0x0CA5
#define NLS_CS_IBMCP259          0x0CA6
#define NLS_CS_IBMCP274          0x0CA7
#define NLS_CS_IBMCP275          0x0CA8
#define NLS_CS_IBMCP281          0x0CA9
#define NLS_CS_IBMCP282          0x0CAA
#define NLS_CS_IBMCP361          0x0CAB

#define NLS_CS_IBMCP382          0x0CAD
#define NLS_CS_IBMCP383          0x0CAE
#define NLS_CS_IBMCP384          0x0CAF
#define NLS_CS_IBMCP385          0x0CB0
#define NLS_CS_IBMCP386          0x0CB1
#define NLS_CS_IBMCP387          0x0CB2
#define NLS_CS_IBMCP388          0x0CB3
#define NLS_CS_IBMCP389          0x0CB4
#define NLS_CS_IBMCP390          0x0CB5
#define NLS_CS_IBMCP391          0x0CB6
#define NLS_CS_IBMCP392          0x0CB7
#define NLS_CS_IBMCP393          0x0CB8
#define NLS_CS_IBMCP394          0x0CB9
#define NLS_CS_IBMCP395          0x0CBA
#define NLS_CS_IBMCP423          0x0CBB
#define NLS_CS_IBMCP424          0x0CBC
#define NLS_CS_IBMCP803          0x0CBD
#define NLS_CS_IBMCP870          0x0CBE
#define NLS_CS_IBMCP871          0x0CBF
#define NLS_CS_IBMCP875          0x0CC0
#define NLS_CS_IBMCP880          0x0CC1
#define NLS_CS_IBMCP905          0x0CC2
#define NLS_CS_IBMCP948          0x0CC4
#define NLS_CS_IBMCP938          0x0CC5
#define NLS_CS_IBMCP1381         NLS_CS_GB  /* 0x0CC8 */
#define NLS_CS_IBMCP1386         NLS_CS_GB
#define NLS_CS_EACC              0x0CCB
#define NLS_CS_ISO2022JP         0x0CCC /* do not use this. use JIS */
#define NLS_CS_JIS               0x0CCD
#define NLS_CS_CCCII             0x0CCE
#define NLS_CS_XEROXCJK          0x0CCF
#define NLS_CS_IBMCP944          0x0CD1
#define NLS_CS_IBMCP934          0x0CD2
#define NLS_CS_IBMCP737          0x0CE0
#define NLS_CS_IBMCP775          0x0CE1
#define NLS_CS_ISO6937           0x0CE2
#define NLS_CS_BASE64            0x0CE3
#define NLS_CS_JIS2              0x0CE4
#define NLS_CS_EUCJ              0x0CE5
#define NLS_CS_EUCT              0x0CE6
#define NLS_CS_ISOKR             0x0CE7
#define NLS_CS_EUCK              NLS_CS_ISOKR
#define NLS_CS_EUCC              0x0CE8

#define NLS_CS_IA5JIS            0x0CE9   /* Dummy */

#define NLS_CS_IBMCP921          0x0CEA /* Replacement for Lithuanian */
#define NLS_CS_IBMCP922          0x0CEB /* More White Russian */

#define NLS_CS_KOI8              0x0CEC /* Cyrillic Internet support */ 
#define NLS_CS_IBMCP720          0x0CED

#define NLS_CS_IBMCP1258              0x0CEE /* Windows Vietnamese */
#define NLS_CS_ISO885910              0x0CEF /* Sami, etc. */
    
#define NLS_CS_JP1TEXT                0x0CF0 /* OSI/JIS X 5003-1987 X.400 Japanese ISP */
#define NLS_CS_VIQRI                  0x0CF1 /* Vietnamese Quoted Readable*/
#define NLS_CS_VISCII                 0x0CF2 /* Vietnamese VISCII 1.1 */
#define NLS_CS_VISCII1                0x0CF3 /* TCVN Viet 1 */
#define NLS_CS_VISCII2                0x0CF4 /* TCVN Viet 2 */
#define NLS_CS_IBMCP838               0x0CF5
#define NLS_CS_IBMCP9030              NLS_CS_IBMCP838
#define NLS_CS_IBMCP833               0x0CF7
#define NLS_CS_IBMCP836               0x0CFA
#define NLS_CS_IBMCP1027              0x0CFD
#define NLS_CS_IBMCP420               0x0CFE
#define NLS_CS_IBMCP918               0x0CFF
#define NLS_CS_IBMCP1097              0x0D00
#define NLS_CS_IBMCP1112              0x0D01
#define NLS_CS_IBMCP1122              0x0D02
#define NLS_CS_IBMCP1123              0x0D03
#define NLS_CS_IBMCP1129              0x0D04
#define NLS_CS_IBMCP1130              0x0D05
#define NLS_CS_IBMCP1132              0x0D06
#define NLS_CS_IBMCP1133              0x0D07

#define NLS_CS_IBMCP806               0x0D08
#define NLS_CS_IBMCP1137              0x0D09
#define NLS_CS_VISCII3                0x0D0A /* Vietnamese TCVN3 */
#define NLS_CS_TCVN3                  NLS_CS_VISCII3

#define NLS_CS_IBMCP858               0x0D10 /* Euro: 850 with D5 = Euro */
#define NLS_CS_IBMCP1140              0x0D11 /* Euro version of CP37 */
#define NLS_CS_IBMCP1141              0x0D12 /* Euro version of CP273 */
#define NLS_CS_IBMCP1142              0x0D13 /* Euro version of CP277 */
#define NLS_CS_IBMCP1143              0x0D14 /* Euro version of CP278 */
#define NLS_CS_IBMCP1144              0x0D15 /* Euro version of CP280 */
#define NLS_CS_IBMCP1145              0x0D16 /* Euro version of CP284 */
#define NLS_CS_IBMCP1146              0x0D17 /* Euro version of CP285 */
#define NLS_CS_IBMCP1147              0x0D18 /* Euro version of CP297 */
#define NLS_CS_IBMCP1148              0x0D19 /* Euro version of CP500 */
#define NLS_CS_IBMCP1149              0x0D1A /* Euro version of CP871 */
#define NLS_CS_IBMCP924               0x0D1B /* EBCDIC Euro cp */
#define NLS_CS_ISO88598i              0x0D1C /* logical bidi */
#define NLS_CS_ISO88598e              0x0D1D /* explicit bidi */
#define NLS_CS_ISCII                  0x0D1E /* ISCII */
#define NLS_CS_GB18030                0x0D31 /* GB18030 */	

/*****************************************************/
/*** THIS RANGE RESERVED FOR EBCDIC DBCS CODEPAGES ***/
#define NLS_CS_EBCDICDBCS_START       0x0E00

/* Dual codepages - really these are CCSID's */
#define NLS_CS_IBMCP930               0x0E00      /* Japan  */
#define NLS_CS_IBMCP933               0x0E01      /* Korea  */
#define NLS_CS_IBMCP935               0x0E02      /* PRC    */
#define NLS_CS_IBMCP937               0x0E03      /* Taiwan */
#define NLS_CS_IBMCP939               0x0E04      /* Japan  */
#define NLS_CS_IBMCP931            0x0E05   /* PRC    */
#if defined(OS390)
#define NLS_CS_IBMCP1388              0x0E06      /* PRC    */
#else
#define NLS_CS_IBMCP1388              NLS_CS_IBMCP935
#endif
#define NLS_CS_IBMCP5026              NLS_CS_IBMCP930
#define NLS_CS_IBMCP5035              NLS_CS_IBMCP939

#define NLS_CS_MIXED_END              0x0E7F

/* DBCS-only */
#define NLS_CS_IBMCP300               0x0E80      /* Japan  */
#define NLS_CS_IBMCP834               0x0E81      /* Korea  */
#define NLS_CS_IBMCP835               0x0E82      /* Taiwan */
#define NLS_CS_IBMCP837               0x0E83      /* PRC    */
#define NLS_CS_IBMCP930X              0x0E84      /* Japan  */
#define NLS_CS_IBMCP933X              0x0E85      /* Korea  */
#define NLS_CS_IBMCP935X              0x0E86      /* PRC    */
#define NLS_CS_IBMCP937X              0x0E87      /* Taiwan */
#define NLS_CS_IBMCP939X              0x0E88      /* Japan  */
#define NLS_CS_IBMCP931X              0x0E89      /* PRC    */
#define NLS_CS_IBMCP1388X             NLS_CS_IBMCP935X 
#define NLS_CS_IBMCP1364              0x0E8A      /* Korea  */
#define NLS_CS_IBMCP1399              0x0E8B      /* Japan  */

#define NLS_CS_EBCDICDBCS_END         0x0EFF
/*****************************************************/

#define NLS_CS_ANYCS                  0xFFFE
#define NLS_CS_NOCS                   0xFFFF


#endif   /* #if !defined(NLS_NOCHARSETIDS) */

#ifdef __cplusplus
}
#endif

#endif    /*   if !defined(NLS_H) */








#if defined(OS400) && (__OS400_TGTVRM__ >= 510)
#pragma datamodel(pop)
#endif

