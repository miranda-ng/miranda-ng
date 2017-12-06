/************************************************************************
Speech.H - Header file to use the Microsoft Speech APIs.

Copyright 1994 -1998 by Microsoft corporation.All rights reserved.
*/

#ifndef _SPEECH_
#define _SPEECH_

// Disable the warning for zero-length arrays in structures
#pragma warning(disable:4200)


/************************************************************************
Defines common to all of the speech APIs.
*/

// Application  Speech API   Compiler Defines                   _S_UNICODE
// -----------------------------------------------------------------------------
//   ANSI        ANSI        <none>                             undefined
//   ANSI        Unicode     _S_UNICODE                         defined
//   Unicode     ANSI        (UNICODE || _UNICODE) && _S_ANSI   undefined
//   Unicode     Unicode     (UNICODE || _UNICODE)              defined

#if (defined(UNICODE) || defined(_UNICODE)) && !defined(_S_ANSI)
#ifndef _S_UNICODE
#define _S_UNICODE
#endif
#endif

/************************************************************************
defines */
#define  SVFN_LEN    (262)
#define  LANG_LEN    (64)
#define  EI_TITLESIZE   (128)
#define  EI_DESCSIZE    (512)
#define  EI_FIXSIZE     (512)
#define  SVPI_MFGLEN    (64)
#define  SVPI_PRODLEN   (64)
#define  SVPI_COMPLEN   (64)
#define  SVPI_COPYRIGHTLEN (128)
#define  SVI_MFGLEN     (SVPI_MFGLEN)
#define  SETBIT(x)      ((DWORD)1 << (x))


// Error Macros
#define  FACILITY_SPEECH   (FACILITY_ITF)
#define  SPEECHERROR(x)    MAKE_SCODE(SEVERITY_ERROR,   FACILITY_SPEECH, (x)+0x200)
#define  AUDERROR(x)       MAKE_SCODE(SEVERITY_ERROR,   FACILITY_SPEECH, (x)+0x300)
#define  SRWARNING(x)      MAKE_SCODE(SEVERITY_SUCCESS, FACILITY_SPEECH, (x)+0x400)
#define  SRERROR(x)        MAKE_SCODE(SEVERITY_ERROR,   FACILITY_SPEECH, (x)+0x400)
#define  TTSERROR(x)       MAKE_SCODE(SEVERITY_ERROR,   FACILITY_SPEECH, (x)+0x500)
#define  VCMDERROR(x)      MAKE_SCODE(SEVERITY_ERROR,   FACILITY_SPEECH, (x)+0x600)
#define  VTXTERROR(x)      MAKE_SCODE(SEVERITY_ERROR,   FACILITY_SPEECH, (x)+0x700)
#define  LEXERROR(x)       MAKE_SCODE(SEVERITY_ERROR,   FACILITY_SPEECH, (x)+0x800)
#define  LEXWARNING(x)     MAKE_SCODE(SEVERITY_SUCCESS, FACILITY_SPEECH, (x)+0x800)

// Audio Errors
#define  AUDERR_NONE                      S_OK                          // 0x00000000
#define  AUDERR_BADDEVICEID               AUDERROR(1)                   // 0x80040301
#define  AUDERR_NEEDWAVEFORMAT            AUDERROR(2)                   // 0x80040302
#define  AUDERR_NOTSUPPORTED              E_NOTIMPL                     // 0x80004001
#define  AUDERR_NOTENOUGHDATA             SPEECHERROR(1)                // 0x80040201
#define  AUDERR_NOTPLAYING                AUDERROR(6)                   // 0x80040306
#define  AUDERR_INVALIDPARAM              E_INVALIDARG                  // 0x80070057
#define  AUDERR_WAVEFORMATNOTSUPPORTED    SPEECHERROR(2)                // 0x80040202
#define  AUDERR_WAVEDEVICEBUSY            SPEECHERROR(3)                // 0x80040203
#define  AUDERR_WAVEDEVNOTSUPPORTED       AUDERROR(18)                  // 0x80040312
#define  AUDERR_NOTRECORDING              AUDERROR(19)                  // 0x80040313
#define  AUDERR_INVALIDFLAG               SPEECHERROR(4)                // 0x80040204
#define  AUDERR_INVALIDHANDLE             E_HANDLE                      // 0x80070006
#define  AUDERR_NODRIVER                  AUDERROR(23)                  // 0x80040317
#define  AUDERR_HANDLEBUSY                AUDERROR(24)                  // 0x80040318
#define  AUDERR_INVALIDNOTIFYSINK         AUDERROR(25)                  // 0x80040319
#define  AUDERR_WAVENOTENABLED            AUDERROR(26)                  // 0x8004031A
#define  AUDERR_ALREADYCLAIMED            AUDERROR(29)                  // 0x8004031D
#define  AUDERR_NOTCLAIMED                AUDERROR(30)                  // 0x8004031E
#define  AUDERR_STILLPLAYING              AUDERROR(31)                  // 0x8004031F
#define  AUDERR_ALREADYSTARTED            AUDERROR(32)                  // 0x80040320
#define  AUDERR_SYNCNOTALLOWED            AUDERROR(33)                  // 0x80040321

// Speech Recognition Warnings
#define  SRWARN_BAD_LIST_PRONUNCIATION    SRWARNING(1)

// Speech Recognition Errors
#define  SRERR_NONE                       S_OK                          // 0x00000000
#define  SRERR_OUTOFDISK                  SPEECHERROR(5)                // 0x80040205
#define  SRERR_NOTSUPPORTED               E_NOTIMPL                     // 0x80004001
#define  SRERR_NOTENOUGHDATA              AUDERR_NOTENOUGHDATA          // 0x80040201
#define  SRERR_VALUEOUTOFRANGE            E_UNEXPECTED                  // 0x8000FFFF
#define  SRERR_GRAMMARTOOCOMPLEX          SRERROR(6)                    // 0x80040406
#define  SRERR_GRAMMARWRONGTYPE           SRERROR(7)                    // 0x80040407
#define  SRERR_INVALIDWINDOW              OLE_E_INVALIDHWND             // 0x8004000F
#define  SRERR_INVALIDPARAM               E_INVALIDARG                  // 0x80070057
#define  SRERR_INVALIDMODE                SPEECHERROR(6)                // 0x80040206
#define  SRERR_TOOMANYGRAMMARS            SRERROR(11)                   // 0x8004040B
#define  SRERR_INVALIDLIST                SPEECHERROR(7)                // 0x80040207
#define  SRERR_WAVEDEVICEBUSY             AUDERR_WAVEDEVICEBUSY         // 0x80040203
#define  SRERR_WAVEFORMATNOTSUPPORTED     AUDERR_WAVEFORMATNOTSUPPORTED // 0x80040202
#define  SRERR_INVALIDCHAR                SPEECHERROR(8)                // 0x80040208
#define  SRERR_GRAMTOOCOMPLEX             SRERR_GRAMMARTOOCOMPLEX       // 0x80040406
#define  SRERR_GRAMTOOLARGE               SRERROR(17)                   // 0x80040411
#define  SRERR_INVALIDINTERFACE           E_NOINTERFACE                 // 0x80004002
#define  SRERR_INVALIDKEY                 SPEECHERROR(9)                // 0x80040209
#define  SRERR_INVALIDFLAG                AUDERR_INVALIDFLAG            // 0x80040204
#define  SRERR_GRAMMARERROR               SRERROR(22)                   // 0x80040416
#define  SRERR_INVALIDRULE                SRERROR(23)                   // 0x80040417
#define  SRERR_RULEALREADYACTIVE          SRERROR(24)                   // 0x80040418
#define  SRERR_RULENOTACTIVE              SRERROR(25)                   // 0x80040419
#define  SRERR_NOUSERSELECTED             SRERROR(26)                   // 0x8004041A
#define  SRERR_BAD_PRONUNCIATION          SRERROR(27)                   // 0x8004041B
#define  SRERR_DATAFILEERROR              SRERROR(28)                   // 0x8004041C
#define  SRERR_GRAMMARALREADYACTIVE       SRERROR(29)                   // 0x8004041D
#define  SRERR_GRAMMARNOTACTIVE           SRERROR(30)                   // 0x8004041E
#define  SRERR_GLOBALGRAMMARALREADYACTIVE SRERROR(31)                   // 0x8004041F
#define  SRERR_LANGUAGEMISMATCH           SRERROR(32)                   // 0x80040420
#define  SRERR_MULTIPLELANG               SRERROR(33)                   // 0x80040421
#define  SRERR_LDGRAMMARNOWORDS           SRERROR(34)                   // 0x80040422
#define  SRERR_NOLEXICON                  SRERROR(35)                   // 0x80040423
#define  SRERR_SPEAKEREXISTS              SRERROR(36)                   // 0x80040424
#define  SRERR_GRAMMARENGINEMISMATCH      SRERROR(37)                   // 0x80040425
#define  SRERR_BOOKMARKEXISTS             SRERROR(38)                   // 0x80040426
#define  SRERR_BOOKMARKDOESNOTEXIST       SRERROR(39)                   // 0x80040427
#define  SRERR_MICWIZARDCANCELED          SRERROR(40)                   // 0x80040428
#define  SRERR_WORDTOOLONG                SRERROR(41)                   // 0x80040429
#define  SRERR_BAD_WORD                   SRERROR(42)                   // 0x8004042A
#define  E_WRONGTYPE                      SPEECHERROR(13)               // 0x8004020C
#define  E_BUFFERTOOSMALL                 SPEECHERROR(14)               // 0x8004020D


// Text to Speech Errors
#define  TTSERR_NONE                      S_OK                          // 0x00000000
#define  TTSERR_INVALIDINTERFACE          E_NOINTERFACE                 // 0x80004002
#define  TTSERR_OUTOFDISK                 SRERR_OUTOFDISK               // 0x80040205
#define  TTSERR_NOTSUPPORTED              E_NOTIMPL                     // 0x80004001
#define  TTSERR_VALUEOUTOFRANGE           E_UNEXPECTED                  // 0x8000FFFF
#define  TTSERR_INVALIDWINDOW             OLE_E_INVALIDHWND             // 0x8004000F
#define  TTSERR_INVALIDPARAM              E_INVALIDARG                  // 0x80070057
#define  TTSERR_INVALIDMODE               SRERR_INVALIDMODE             // 0x80040206
#define  TTSERR_INVALIDKEY                SRERR_INVALIDKEY              // 0x80040209
#define  TTSERR_WAVEFORMATNOTSUPPORTED    AUDERR_WAVEFORMATNOTSUPPORTED // 0x80040202
#define  TTSERR_INVALIDCHAR               SRERR_INVALIDCHAR             // 0x80040208
#define  TTSERR_QUEUEFULL                 SPEECHERROR(10)               // 0x8004020A
#define  TTSERR_WAVEDEVICEBUSY            AUDERR_WAVEDEVICEBUSY         // 0x80040203
#define  TTSERR_NOTPAUSED                 TTSERROR(1)                   // 0x80040501
#define  TTSERR_ALREADYPAUSED             TTSERROR(2)                   // 0x80040502


// Voice Command Errors

/*
 *  Everything worked
 */
#define  VCMDERR_NONE                     S_OK                          // 0x00000000

/*
 *  Voice Commands could not allocate memory
 */
#define  VCMDERR_OUTOFMEM                 E_OUTOFMEMORY                 // 0x8007000E

/*
 *  Voice Commands could not store/retrieve a command set from the database
 */
#define  VCMDERR_OUTOFDISK                SRERR_OUTOFDISK               // 0x80040205

/*
 *  Function not implemented
 */
#define  VCMDERR_NOTSUPPORTED             E_NOTIMPL                     // 0x80004001

/*
 *  A parameter was passed that was out of the ranged of accepted values
 */
#define  VCMDERR_VALUEOUTOFRANGE          E_UNEXPECTED                  // 0x8000FFFF

/*
 *  A menu was too complex to compile a context-free grammar
 */
#define  VCMDERR_MENUTOOCOMPLEX           VCMDERROR(0x06)               //  0x80040606

/*
 *  Language mismatch between the speech recognition mode and menu trying
 *  to create
 */
#define  VCMDERR_MENUWRONGLANGUAGE        VCMDERROR(0x07)               // 0x80040607

/*
 *  An invalid window handle was passed to Voice Commands
 */
#define  VCMDERR_INVALIDWINDOW            OLE_E_INVALIDHWND             // 0x8004000F

/*
 *  Voice Commands detected a bad function parameter
 */
#define  VCMDERR_INVALIDPARAM             E_INVALIDARG                  // 0x80070057

/*
 *  This function cannot be completed right now, usually when trying to do
 *  some operation while no speech recognition site is established
 */
#define  VCMDERR_INVALIDMODE              SRERR_INVALIDMODE             // 0x80040206

/*
 *  There are too many Voice Commands menu
 */                                                                     // 0x8004060B
#define  VCMDERR_TOOMANYMENUS             VCMDERROR(0x0B)

/*
 *  Invalid list passed to ListSet/ListGet
 */
#define  VCMDERR_INVALIDLIST              SRERR_INVALIDLIST             // 0x80040207

/*
 *  Trying to open an existing menu that is not in the Voice Commands database
 */
#define  VCMDERR_MENUDOESNOTEXIST         VCMDERROR(0x0D)               // 0x8004060D

/*
 *  The function could not be completed because the menu is actively 
 *  listening for commands
 */
#define  VCMDERR_MENUACTIVE               VCMDERROR(0x0E)               // 0x8004060E

/*
 *  No speech recognition engine is started
 */
#define  VCMDERR_NOENGINE                 VCMDERROR(0x0F)               // 0x8004060F

/*
 *  Voice Commands could not acquire a Grammar interface from the speech
 *  recognition engine
 */
#define  VCMDERR_NOGRAMMARINTERFACE       VCMDERROR(0x10)               // 0x80040610

/*
 *  Voice Commands could not acquire a Find interface from the speech
 *  recognition engine
 */
#define  VCMDERR_NOFINDINTERFACE          VCMDERROR(0x11)               // 0x80040611

/*
 *  Voice Commands could not create a speech recognition enumerator
 */
#define  VCMDERR_CANTCREATESRENUM         VCMDERROR(0x12)               // 0x80040612

/*
 *  Voice Commands could get the appropriate site information to start a
 *  speech recognition engine
 */
#define  VCMDERR_NOSITEINFO               VCMDERROR(0x13)               // 0x80040613

/*
 *  Voice Commands could not find a speech recognition engine
 */
#define  VCMDERR_SRFINDFAILED             VCMDERROR(0x14)               // 0x80040614

/*
 *  Voice Commands could not create an audio source object
 */
#define  VCMDERR_CANTCREATEAUDIODEVICE    VCMDERROR(0x15)               // 0x80040615

/*
 *  Voice Commands could not set the appropriate device number in the
 *  audio source object
 */
#define  VCMDERR_CANTSETDEVICE            VCMDERROR(0x16)               // 0x80040616

/*
 *  Voice Commands could not select a speech recognition engine. Usually the
 *  error will occur when Voice Commands has enumerated and found an
 *  appropriate speech recognition engine, then it is not able to actually
 *  select/start the engine. There are different reasons that the engine won't
 *  start, but the most common is that there is no wave in device.
 */
#define  VCMDERR_CANTSELECTENGINE         VCMDERROR(0x17)               // 0x80040617

/*
 *  Voice Commands could not create a notfication sink for engine
 *  notifications
 */
#define  VCMDERR_CANTCREATENOTIFY         VCMDERROR(0x18)               // 0x80040618

/*
 *  Voice Commands could not create internal data structures.
 */
#define  VCMDERR_CANTCREATEDATASTRUCTURES VCMDERROR(0x19)               // 0x80040619

/*
 *  Voice Commands could not initialize internal data structures
 */
#define  VCMDERR_CANTINITDATASTRUCTURES   VCMDERROR(0x1A)               // 0x8004061A

/*
 *  The menu does not have an entry in the Voice Commands cache
 */
#define  VCMDERR_NOCACHEDATA              VCMDERROR(0x1B)               // 0x8004061B

/*
 *  The menu does not have commands
 */
#define  VCMDERR_NOCOMMANDS               VCMDERROR(0x1C)               // 0x8004061C

/*
 *  Voice Commands cannot extract unique words needed for the engine grammar
 */
#define  VCMDERR_CANTXTRACTWORDS          VCMDERROR(0x1D)               // 0x8004061D

/*
 *  Voice Commands could not get the command set database name
 */
#define  VCMDERR_CANTGETDBNAME            VCMDERROR(0x1E)               // 0x8004061E

/*
 *  Voice Commands could not create a registry key
 */
#define  VCMDERR_CANTCREATEKEY            VCMDERROR(0x1F)               // 0x8004061F

/*
 *  Voice Commands could not create a new database name
 */
#define  VCMDERR_CANTCREATEDBNAME         VCMDERROR(0x20)               // 0x80040620

/*
 *  Voice Commands could not update the registry
 */
#define  VCMDERR_CANTUPDATEREGISTRY       VCMDERROR(0x21)               // 0x80040621

/*
 *  Voice Commands could not open the registry
 */
#define  VCMDERR_CANTOPENREGISTRY         VCMDERROR(0x22)               // 0x80040622

/*
 *  Voice Commands could not open the command set database
 */
#define  VCMDERR_CANTOPENDATABASE         VCMDERROR(0x23)               // 0x80040623

/*
 *  Voice Commands could not create a database storage object
 */
#define  VCMDERR_CANTCREATESTORAGE        VCMDERROR(0x24)               // 0x80040624

/*
 *  Voice Commands could not do CmdMimic
 */
#define  VCMDERR_CANNOTMIMIC              VCMDERROR(0x25)               // 0x80040625

/*
 *  A menu of this name already exist
 */
#define  VCMDERR_MENUEXIST                VCMDERROR(0x26)               // 0x80040626

/*
 *  A menu of this name is open and cannot be deleted right now
 */
#define  VCMDERR_MENUOPEN                 VCMDERROR(0x27)               // 0x80040627


// Voice Text Errors
#define  VTXTERR_NONE                     S_OK                          // 0x00000000

/*
 *  Voice Text failed to allocate memory it needed
 */
#define  VTXTERR_OUTOFMEM                 E_OUTOFMEMORY                 // 0x8007000E

/*
 *  An empty string ("") was passed to the Speak function
 */
#define  VTXTERR_EMPTYSPEAKSTRING         SPEECHERROR(0x0b)             // 0x8004020B

/*
 *  An invalid parameter was passed to a Voice Text function
 */
#define  VTXTERR_INVALIDPARAM             E_INVALIDARG                  // 0x80070057

/*
 *  The called function cannot be done at this time. This usually occurs
 *  when trying to call a function that needs a site, but no site has been
 *  registered.
 */
#define  VTXTERR_INVALIDMODE              SRERR_INVALIDMODE             // 0x80040206

/*
 *  No text-to-speech engine is started
 */
#define  VTXTERR_NOENGINE                 VTXTERROR(0x0F)               // 0x8004070F

/*
 *  Voice Text could not acquire a Find interface from the text-to-speech
 *  engine
 */
#define  VTXTERR_NOFINDINTERFACE          VTXTERROR(0x11)               // 0x80040711

/*
 *  Voice Text could not create a text-to-speech enumerator
 */
#define  VTXTERR_CANTCREATETTSENUM        VTXTERROR(0x12)               // 0x80040712

/*
 *  Voice Text could get the appropriate site information to start a
 *  text-to-speech engine
 */
#define  VTXTERR_NOSITEINFO               VTXTERROR(0x13)               // 0x80040713

/*
 *  Voice Text could not find a text-to-speech engine
 */
#define  VTXTERR_TTSFINDFAILED            VTXTERROR(0x14)               // 0x80040714

/*
 *  Voice Text could not create an audio destination object
 */
#define  VTXTERR_CANTCREATEAUDIODEVICE    VTXTERROR(0x15)               // 0x80040715

/*
 *  Voice Text could not set the appropriate device number in the
 *  audio destination object
 */
#define  VTXTERR_CANTSETDEVICE            VTXTERROR(0x16)               // 0x80040716

/*
 *  Voice Text could not select a text-to-speech engine. Usually the
 *  error will occur when Voice Text has enumerated and found an
 *  appropriate text-to-speech engine, then it is not able to actually
 *  select/start the engine.
 */
#define  VTXTERR_CANTSELECTENGINE         VTXTERROR(0x17)               // 0x80040717

/*
 *  Voice Text could not create a notfication sink for engine
 *  notifications
 */
#define  VTXTERR_CANTCREATENOTIFY         VTXTERROR(0x18)               // 0x80040718

/*
 *  Voice Text is disabled at this time
 */
#define  VTXTERR_NOTENABLED               VTXTERROR(0x19)               // 0x80040719

#define  VTXTERR_OUTOFDISK                SRERR_OUTOFDISK               // 0x80040205
#define  VTXTERR_NOTSUPPORTED             E_NOTIMPL                     // 0x80004001
#define  VTXTERR_NOTENOUGHDATA            AUDERR_NOTENOUGHDATA          // 0x80040201
#define  VTXTERR_QUEUEFULL                TTSERR_QUEUEFULL              // 0x8004020A
#define  VTXTERR_VALUEOUTOFRANGE          E_UNEXPECTED                  // 0x8000FFFF
#define  VTXTERR_INVALIDWINDOW            OLE_E_INVALIDHWND             // 0x8004000F
#define  VTXTERR_WAVEDEVICEBUSY           AUDERR_WAVEDEVICEBUSY         // 0x80040203
#define  VTXTERR_WAVEFORMATNOTSUPPORTED   AUDERR_WAVEFORMATNOTSUPPORTED // 0x80040202
#define  VTXTERR_INVALIDCHAR              SRERR_INVALIDCHAR             // 0x80040208


// ILexPronounce errors
#define  LEXERR_INVALIDTEXTCHAR           LEXERROR(0x01)                // 0x80040801
#define  LEXERR_INVALIDSENSE              LEXERROR(0x02)                // 0x80040802
#define  LEXERR_NOTINLEX                  LEXERROR(0x03)                // 0x80040803
#define  LEXERR_OUTOFDISK                 LEXERROR(0x04)                // 0x80040804
#define  LEXERR_INVALIDPRONCHAR           LEXERROR(0x05)                // 0x80040805
#define  LEXERR_ALREADYINLEX              LEXWARNING(0x06)              // 0x00040806
#define  LEXERR_PRNBUFTOOSMALL            LEXERROR(0x07)                // 0x80040807
#define  LEXERR_ENGBUFTOOSMALL            LEXERROR(0x08)                // 0x80040808
#define  LEXERR_INVALIDLEX                LEXERROR(0x09)                // 0x80040809



/************************************************************************
typedefs */

typedef LPUNKNOWN FAR * PIUNKNOWN;

typedef struct {
   PVOID    pData;
   DWORD    dwSize;
   } SDATA, * PSDATA;



typedef struct {
   LANGID   LanguageID;
   WCHAR    szDialect[LANG_LEN];
   } LANGUAGEW, FAR * PLANGUAGEW;

typedef struct {
   LANGID   LanguageID;
   CHAR     szDialect[LANG_LEN];
   } LANGUAGEA, FAR * PLANGUAGEA;

#ifdef  _S_UNICODE
#define LANGUAGE    LANGUAGEW
#define PLANGUAGE   PLANGUAGEW
#else
#define LANGUAGE    LANGUAGEA
#define PLANGUAGE   PLANGUAGEA
#endif  // _S_UNICODE


typedef struct {
   HRESULT  hRes;
   WCHAR    szStrings[512];
   } SPCHERRORW, FAR * PSPCHERRORW;

typedef struct {
   HRESULT  hRes;
   CHAR     szStrings[512];
   } SPCHERRORA, FAR * PSPCHERRORA;

#ifdef  _S_UNICODE
#define SPCHERROR    SPCHERRORW
#define PSPCHERROR   PSPCHERRORW
#else
#define SPCHERROR    SPCHERRORA
#define PSPCHERROR   PSPCHERRORA
#endif  // _S_UNICODE



typedef unsigned __int64 QWORD, * PQWORD;

typedef enum {
   CHARSET_TEXT           = 0,
   CHARSET_IPAPHONETIC    = 1,
   CHARSET_ENGINEPHONETIC = 2
   } VOICECHARSET;

typedef enum _VOICEPARTOFSPEECH {
   VPS_UNKNOWN = 0,
   VPS_NOUN = 1,
   VPS_VERB = 2,
   VPS_ADVERB = 3,
   VPS_ADJECTIVE = 4,
   VPS_PROPERNOUN = 5,
   VPS_PRONOUN = 6,
   VPS_CONJUNCTION = 7,
   VPS_CARDINAL = 8,
   VPS_ORDINAL = 9,
   VPS_DETERMINER = 10,
   VPS_QUANTIFIER = 11,
   VPS_PUNCTUATION = 12,
   VPS_CONTRACTION = 13,
   VPS_INTERJECTION = 14,
   VPS_ABBREVIATION = 15,
   VPS_PREPOSITION = 16
   } VOICEPARTOFSPEECH;


typedef struct {
   DWORD   dwNextPhonemeNode;
   DWORD   dwUpAlternatePhonemeNode;
   DWORD   dwDownAlternatePhonemeNode;
   DWORD   dwPreviousPhonemeNode;
   DWORD   dwWordNode;
   QWORD   qwStartTime;
   QWORD   qwEndTime;
   DWORD   dwPhonemeScore;
   WORD    wVolume;
   WORD    wPitch;
   } SRRESPHONEMENODE, *PSRRESPHONEMENODE;


typedef struct {
   DWORD   dwNextWordNode;
   DWORD   dwUpAlternateWordNode;
   DWORD   dwDownAlternateWordNode;
   DWORD   dwPreviousWordNode;
   DWORD   dwPhonemeNode;
   QWORD   qwStartTime;
   QWORD   qwEndTime;
   DWORD   dwWordScore;
   WORD      wVolume;
   WORD      wPitch;
   VOICEPARTOFSPEECH   pos;
   DWORD   dwCFGParse;
   DWORD   dwCue;
   } SRRESWORDNODE, * PSRRESWORDNODE;


/************************************************************************
interfaces */

/*
 * IAttributes
 */

/* attribute values */
#define  TTSBASEATTR             0x1000
#define  SRBASEATTR              0x2000
#define  VDCTBASEATTR            0x3000
#define  VCMDBASEATTR            0x4000
#define  VTXTBASEATTR            0x5000
#define  AUDBASEATTR             0x6000

#define  TTSATTR_PITCH           (1)
#define  TTSATTR_REALTIME        (0)
#define  TTSATTR_SPEED           (2)
#define  TTSATTR_VOLUME          (3)
#define  TTSATTR_PITCHRANGE      (TTSBASEATTR + 5)
#define  TTSATTR_PITCHRANGEDEFAULT (TTSBASEATTR + 6)
#define  TTSATTR_PITCHRANGEMAX   (TTSBASEATTR + 7)
#define  TTSATTR_PITCHRANGEMIN   (TTSBASEATTR + 8)
#define  TTSATTR_PITCHRANGERELATIVE (TTSBASEATTR + 9)
#define  TTSATTR_PITCHRANGERELATIVEMAX (TTSBASEATTR + 10)
#define  TTSATTR_PITCHRANGERELATIVEMIN (TTSBASEATTR + 11)
#define  TTSATTR_PITCHRELATIVE   (TTSBASEATTR + 12)
#define  TTSATTR_PITCHRELATIVEMAX (TTSBASEATTR + 13)
#define  TTSATTR_PITCHRELATIVEMIN (TTSBASEATTR + 14)
#define  TTSATTR_PITCHDEFAULT    (TTSBASEATTR + 15)
#define  TTSATTR_PITCHMAX        (TTSBASEATTR + 16)
#define  TTSATTR_PITCHMIN        (TTSBASEATTR + 17)
#define  TTSATTR_SPEEDRELATIVE   (TTSBASEATTR + 18)
#define  TTSATTR_SPEEDRELATIVEMAX (TTSBASEATTR + 19)
#define  TTSATTR_SPEEDRELATIVEMIN (TTSBASEATTR + 20)
#define  TTSATTR_SPEEDDEFAULT    (TTSBASEATTR + 21)
#define  TTSATTR_SPEEDMAX        (TTSBASEATTR + 22)
#define  TTSATTR_SPEEDMIN        (TTSBASEATTR + 23)
#define  TTSATTR_THREADPRIORITY  (TTSBASEATTR + 24)
#define  TTSATTR_SINKFLAGS       (TTSBASEATTR + 25)
#define  TTSATTR_VOLUMEDEFAULT   (TTSBASEATTR + 26)

#define  SRATTR_AUTOGAIN         (1)
#define  SRATTR_ECHO             (3)
#define  SRATTR_ENERGYFLOOR      (4)
#define  SRATTR_MICROPHONE       (5)
#define  SRATTR_REALTIME         (6)
#define  SRATTR_SPEAKER          (7)
#define  SRATTR_TIMEOUT_COMPLETE (8)
#define  SRATTR_TIMEOUT_INCOMPLETE (SRBASEATTR + 8)
#define  SRATTR_THRESHOLD        (2)
#define  SRATTR_ACCURACYSLIDER   (SRBASEATTR + 10)
#define  SRATTR_LEVEL            (SRBASEATTR + 11)
#define  SRATTR_LISTENINGSTATE   (SRBASEATTR + 12)
#define  SRATTR_RESULTSINFO      (SRBASEATTR + 13)
#define  SRATTR_RESULTSINFO_POSSIBLE (SRBASEATTR + 14)
#define  SRATTR_SINKFLAGS        (SRBASEATTR + 15)
#define  SRATTR_THREADPRIORITY   (SRBASEATTR + 16)

#define  VDCTATTR_AWAKESTATE     (VDCTBASEATTR + 1)
#define  VDCTATTR_MODE           (VDCTBASEATTR + 2)
#define  VDCTATTR_MEMORY         (VDCTBASEATTR + 3)
#define  VDCTATTR_CORRECTIONRECT (VDCTBASEATTR + 4)
#define  VDCTATTR_VISIBLETEXTSTART (VDCTBASEATTR + 5)
#define  VDCTATTR_VISIBLETEXTCHARS (VDCTBASEATTR + 6)
#define  VDCTATTR_INSERTIONGUI   (VDCTBASEATTR + 7)   // turn on ISRGramInsertionGUI

#define  VCMDATTR_AWAKESTATE     (VCMDBASEATTR + 1)
#define  VCMDATTR_DEVICE         (VCMDBASEATTR + 2)
#define  VCMDATTR_ENABLED        (VCMDBASEATTR + 3)
#define  VCMDATTR_SRMODE         (VCMDBASEATTR + 4)

#define  AUDATTR_USELOWPRIORITY  (AUDBASEATTR + 1) // MMAudioSource: if true, send low priority message to card when starting
#define  AUDATTR_AUTORETRY       (AUDBASEATTR + 2) // MMAudioSource: if true, keep retrying to open wave device if can't initially
#define  AUDATTR_TIMERMS         (AUDBASEATTR + 3) // MMAudioDest: milliseconds between timer ticks, when send out buffers, etc.
#define  AUDATTR_DIRECTFLAGS     (AUDBASEATTR + 4) // DirectSoundAudioDest: Flages to send into CreateSoundBuffer
                                                   // set these values before waveformatset. Use to do 3-d tts

// SRATTR_RESULTSINFO flags
#define  SRRI_AUDIO		 SETBIT(0)
#define  SRRI_AUDIO_UNCOMPRESSED SETBIT(1)
#define  SRRI_ALTERNATIVES       SETBIT(2)
#define  SRRI_WORDGRAPH          SETBIT(3)
#define  SRRI_PHONEMEGRAPH       SETBIT(4)

// SRATTR_SINKFLAGS flags
#define  SRASF_ATTRIBUTES        SETBIT(0)
#define  SRASF_INTERFERENCE      SETBIT(1)
#define  SRASF_SOUND             SETBIT(2)
#define  SRASF_UTTERANCEBEGIN    SETBIT(3)
#define  SRASF_UTTERANCEEND      SETBIT(4)
#define  SRASF_VUMETER           SETBIT(5)
#define  SRASF_PHRASEHYPOTHESIS  SETBIT(6)
#define  SRASF_TRAINING          SETBIT(7)
#define  SRASF_ERRORWARNING      SETBIT(8)

#undef   INTERFACE
#define  INTERFACE   IAttributesW

// {1D59DED1-E367-11d1-BED7-006008317CE8}
DEFINE_GUID(IID_IAttributesW, 
0x1d59ded1, 0xe367, 0x11d1, 0xbe, 0xd7, 0x0, 0x60, 0x8, 0x31, 0x7c, 0xe8);

DECLARE_INTERFACE_ (IAttributesW, IUnknown) {

   // IUnknown members
   STDMETHOD(QueryInterface)  (THIS_ REFIID riid, LPVOID FAR* ppvObj) PURE;
   STDMETHOD_(ULONG,AddRef)   (THIS) PURE;
   STDMETHOD_(ULONG,Release)  (THIS) PURE;

   // AttributesW members
   STDMETHOD (DWORDGet)       (DWORD, DWORD*) PURE;
   STDMETHOD (DWORDSet)       (DWORD, DWORD) PURE;
   STDMETHOD (StringGet)      (DWORD, PWSTR, DWORD, DWORD *) PURE;
   STDMETHOD (StringSet)      (DWORD, PCWSTR) PURE;
   STDMETHOD (MemoryGet)      (DWORD, PVOID*, DWORD*) PURE;
   STDMETHOD (MemorySet)      (DWORD, PVOID, DWORD) PURE;
   };

typedef IAttributesW FAR * PIATTRIBUTESW;


#undef   INTERFACE
#define  INTERFACE   IAttributesA

// {1D59DED2-E367-11d1-BED7-006008317CE8}
DEFINE_GUID(IID_IAttributesA, 
0x1d59ded2, 0xe367, 0x11d1, 0xbe, 0xd7, 0x0, 0x60, 0x8, 0x31, 0x7c, 0xe8);

DECLARE_INTERFACE_ (IAttributesA, IUnknown) {

   // IUnknown members
   STDMETHOD(QueryInterface)  (THIS_ REFIID riid, LPVOID FAR* ppvObj) PURE;
   STDMETHOD_(ULONG,AddRef)   (THIS) PURE;
   STDMETHOD_(ULONG,Release)  (THIS) PURE;

   // AttributesA members
   STDMETHOD (DWORDGet)       (DWORD, DWORD*) PURE;
   STDMETHOD (DWORDSet)       (DWORD, DWORD) PURE;
   STDMETHOD (StringGet)      (DWORD, PSTR, DWORD, DWORD *) PURE;
   STDMETHOD (StringSet)      (DWORD, PCSTR) PURE;
   STDMETHOD (MemoryGet)      (DWORD, PVOID*, DWORD*) PURE;
   STDMETHOD (MemorySet)      (DWORD, PVOID, DWORD) PURE;
   };

typedef IAttributesA FAR * PIATTRIBUTESA;


#ifdef _S_UNICODE
 #define IAttributes        IAttributesW
 #define IID_IAttributes    IID_IAttributesW
 #define PIATTRIBUTES       PIATTRIBUTESW

#else
 #define IAttributes        IAttributesA
 #define IID_IAttributes    IID_IAttributesA
 #define PIATTRIBUTES       PIATTRIBUTESA

#endif   // _S_UNICODE


/*
 * ISpchError
 */

#undef   INTERFACE
#define  INTERFACE   ISpchErrorW

// {9B445335-E39F-11d1-BED7-006008317CE8}
DEFINE_GUID(IID_ISpchErrorW, 
0x9b445335, 0xe39f, 0x11d1, 0xbe, 0xd7, 0x0, 0x60, 0x8, 0x31, 0x7c, 0xe8);

DECLARE_INTERFACE_ (ISpchErrorW, IUnknown) {

   // IUnknown members
   STDMETHOD(QueryInterface)  (THIS_ REFIID riid, LPVOID FAR* ppvObj) PURE;
   STDMETHOD_(ULONG,AddRef)   (THIS) PURE;
   STDMETHOD_(ULONG,Release)  (THIS) PURE;

   // SpchError members
   STDMETHOD (LastErrorGet) (THIS_ SPCHERRORW *) PURE;
   STDMETHOD (ErrorMessageGet) (THIS_ WCHAR *, DWORD, DWORD *) PURE;
   };

typedef ISpchErrorW FAR * PISPCHERRORW;


#undef   INTERFACE
#define  INTERFACE   ISpchErrorA

// {9B445336-E39F-11d1-BED7-006008317CE8}
DEFINE_GUID(IID_ISpchErrorA, 
0x9b445336, 0xe39f, 0x11d1, 0xbe, 0xd7, 0x0, 0x60, 0x8, 0x31, 0x7c, 0xe8);

DECLARE_INTERFACE_ (ISpchErrorA, IUnknown) {

   // IUnknown members
   STDMETHOD(QueryInterface)  (THIS_ REFIID riid, LPVOID FAR* ppvObj) PURE;
   STDMETHOD_(ULONG,AddRef)   (THIS) PURE;
   STDMETHOD_(ULONG,Release)  (THIS) PURE;

   // SpchError members
   STDMETHOD (LastErrorGet) (THIS_ SPCHERRORA *) PURE;
   STDMETHOD (ErrorMessageGet) (THIS_ CHAR *, DWORD, DWORD *) PURE;
   };

typedef ISpchErrorA FAR * PISPCHERRORA;
#ifdef _S_UNICODE
 #define ISpchError        ISpchErrorW
 #define IID_ISpchError    IID_ISpchErrorW
 #define PISPCHERROR       PISPCHERRORW

#else
 #define ISpchError        ISpchErrorA
 #define IID_ISpchError    IID_ISpchErrorA
 #define PISPCHERROR       PISPCHERRORA

#endif   // _S_UNICODE





/*
 * ILexPronounce
 */

#undef   INTERFACE
#define  INTERFACE   ILexPronounceW

DEFINE_GUID(IID_ILexPronounceW, 0x090CD9A2, 0xDA1A, 0x11CD, 0xB3, 0xCA, 0x0, 0xAA, 0x0, 0x47, 0xBA, 0x4F);

DECLARE_INTERFACE_ (ILexPronounceW, IUnknown) {

   // IUnknown members
   STDMETHOD(QueryInterface)  (THIS_ REFIID riid, LPVOID FAR* ppvObj) PURE;
   STDMETHOD_(ULONG,AddRef)   (THIS) PURE;
   STDMETHOD_(ULONG,Release)  (THIS) PURE;

   // LexPronounceW members
   STDMETHOD (Add)            (THIS_ VOICECHARSET, PCWSTR, PCWSTR, 
			       VOICEPARTOFSPEECH, PVOID, DWORD) PURE;
   STDMETHOD (Get)            (THIS_ VOICECHARSET, PCWSTR, WORD, PWSTR, 
			       DWORD, DWORD *, VOICEPARTOFSPEECH *, PVOID, 
			       DWORD, DWORD *) PURE;
   STDMETHOD (Remove)         (THIS_ PCWSTR, WORD) PURE;
   };

typedef ILexPronounceW FAR * PILEXPRONOUNCEW;


#undef   INTERFACE
#define  INTERFACE   ILexPronounceA

DEFINE_GUID(IID_ILexPronounceA, 0x2F26B9C0L, 0xDB31, 0x11CD, 0xB3, 0xCA, 0x00, 0xAA, 0x00, 0x47, 0xBA, 0x4F);

DECLARE_INTERFACE_ (ILexPronounceA, IUnknown) {

   // IUnknown members
   STDMETHOD(QueryInterface)  (THIS_ REFIID riid, LPVOID FAR* ppvObj) PURE;
   STDMETHOD_(ULONG,AddRef)   (THIS) PURE;
   STDMETHOD_(ULONG,Release)  (THIS) PURE;

   // LexPronounceA members
   STDMETHOD (Add)            (THIS_ VOICECHARSET, PCSTR, PCSTR, 
			       VOICEPARTOFSPEECH, PVOID, DWORD) PURE;
   STDMETHOD (Get)            (THIS_ VOICECHARSET, PCSTR, WORD, PSTR, 
			       DWORD, DWORD *, VOICEPARTOFSPEECH *, PVOID, 
			       DWORD, DWORD *) PURE;
   STDMETHOD (Remove)         (THIS_ PCSTR, WORD) PURE;
   };

typedef ILexPronounceA FAR * PILEXPRONOUNCEA;


#ifdef _S_UNICODE
 #define ILexPronounce        ILexPronounceW
 #define IID_ILexPronounce    IID_ILexPronounceW
 #define PILEXPRONOUNCE       PILEXPRONOUNCEW

#else
 #define ILexPronounce        ILexPronounceA
 #define IID_ILexPronounce    IID_ILexPronounceA
 #define PILEXPRONOUNCE       PILEXPRONOUNCEA

#endif   // _S_UNICODE


/*
 * ILexPronounce2
 */

#define  ILP2_ACTIVE          SETBIT(0)
#define  ILP2_USER            SETBIT(1)
#define  ILP2_BACKUP          SETBIT(2)
#define  ILP2_LTS             SETBIT(3)

#undef   INTERFACE
#define  INTERFACE   ILexPronounce2W

// {D6E56341-B3EA-11d1-AFA5-0000F81E880D}
DEFINE_GUID(IID_ILexPronounce2W, 0xd6e56341, 0xb3ea, 0x11d1, 0xaf, 0xa5, 0x0, 0x0, 0xf8, 0x1e, 0x88, 0xd);

DECLARE_INTERFACE_ (ILexPronounce2W, ILexPronounceW) {

   // LexPronounceW members
   STDMETHOD (Add)            (THIS_ VOICECHARSET, PCWSTR, PCWSTR, 
			       VOICEPARTOFSPEECH, PVOID, DWORD) PURE;
   STDMETHOD (Get)            (THIS_ VOICECHARSET, PCWSTR, WORD, PWSTR, 
			       DWORD, DWORD *, VOICEPARTOFSPEECH *, PVOID, 
			       DWORD, DWORD *) PURE;
   STDMETHOD (Remove)         (THIS_ PCWSTR, WORD) PURE;

   // LexPronounce2W members
   STDMETHOD (AddTo)            (THIS_ DWORD, VOICECHARSET, PCWSTR, PCWSTR, 
			       VOICEPARTOFSPEECH, PVOID, DWORD) PURE;
   STDMETHOD (GetFrom)            (THIS_ DWORD, VOICECHARSET, PCWSTR, WORD, PWSTR, 
			       DWORD, DWORD *, VOICEPARTOFSPEECH *, PVOID, 
			       DWORD, DWORD *) PURE;
   STDMETHOD (RemoveFrom)         (THIS_ DWORD, PCWSTR, WORD) PURE;
   STDMETHOD (QueryLexicons)  (THIS_ BOOL, DWORD *) PURE;
   STDMETHOD (ChangeSpelling) (THIS_ DWORD, PCWSTR, PCWSTR) PURE;
   };

typedef ILexPronounce2W FAR * PILEXPRONOUNCE2W;


#undef   INTERFACE
#define  INTERFACE   ILexPronounce2A

// {D6E56342-B3EA-11d1-AFA5-0000F81E880D}
DEFINE_GUID(IID_ILexPronounce2A, 0xd6e56342, 0xb3ea, 0x11d1, 0xaf, 0xa5, 0x0, 0x0, 0xf8, 0x1e, 0x88, 0xd);

DECLARE_INTERFACE_ (ILexPronounce2A, ILexPronounceA) {

   // LexPronounceA members
   STDMETHOD (Add)            (THIS_ VOICECHARSET, PCSTR, PCSTR, 
			       VOICEPARTOFSPEECH, PVOID, DWORD) PURE;
   STDMETHOD (Get)            (THIS_ VOICECHARSET, PCSTR, WORD, PSTR, 
			       DWORD, DWORD *, VOICEPARTOFSPEECH *, PVOID, 
			       DWORD, DWORD *) PURE;
   STDMETHOD (Remove)         (THIS_ PCSTR, WORD) PURE;

   // LexPronounceA members
   STDMETHOD (AddTo)            (THIS_ DWORD, VOICECHARSET, PCSTR, PCSTR, 
			       VOICEPARTOFSPEECH, PVOID, DWORD) PURE;
   STDMETHOD (GetFrom)            (THIS_ DWORD, VOICECHARSET, PCSTR, WORD, PSTR, 
			       DWORD, DWORD *, VOICEPARTOFSPEECH *, PVOID, 
			       DWORD, DWORD *) PURE;
   STDMETHOD (RemoveFrom)         (THIS_ DWORD, PCSTR, WORD) PURE;
   STDMETHOD (QueryLexicons)  (THIS_ BOOL, DWORD *) PURE;
   STDMETHOD (ChangeSpelling) (THIS_ DWORD, PCSTR, PCSTR) PURE;
   };

typedef ILexPronounce2A FAR * PILEXPRONOUNCE2A;


#ifdef _S_UNICODE
 #define ILexPronounce2        ILexPronounce2W
 #define IID_ILexPronounce2    IID_ILexPronounce2W
 #define PILEXPRONOUNCE2       PILEXPRONOUNCE2W

#else
 #define ILexPronounce2        ILexPronounce2A
 #define IID_ILexPronounce2    IID_ILexPronounce2A
 #define PILEXPRONOUNCE2       PILEXPRONOUNCE2A

#endif   // _S_UNICODE

/************************************************************************
Speech Tools
*/

/* defines */
// ISTMicWizard->Wizard
#define STMWU_CNC          0
#define STMWU_DICTATION    1
#define STMWU_LOWERGAIN    0x10000     // if on, tries for a lower gain
#define STMWU_NOAUTOGAIN   0x20000     // if on, doesn't adjust the autogain bit

#define STMWF_CANSKIP      SETBIT(0)

// MICWIZARDINFO
#define STMWI_UNKNOWN      0
#define STMWI_CLOSETALK    1
#define STMWI_EARPIECE     2
#define STMWI_HANDSET      3
#define STMWI_CLIPON       4
#define STMWI_DESKTOP      5
#define STMWI_HANDHELD     6
#define STMWI_TOPMONITOR   7
#define STMWI_INMONITOR    8
#define STMWI_KEYBOARD     9
#define STMWI_REMOTE       10

#define STMWIS_UNKNOWN     0
#define STMWIS_SPEAKERS    1
#define STMWIS_HEADPHONES  2
#define STMWIS_BOTH        3

/* structures */

typedef struct {
   BOOL     fHasRunMicWizard;
   DWORD    dwWaveInDevice;
   DWORD    dwMicrophoneType;
   WCHAR    szMicString[64];
} MICWIZARDINFO, *PMICWIZARDINFO;

typedef struct {
   BOOL     fHasRunMicWizard;
   DWORD    dwWaveInDevice;
   DWORD    dwMicrophoneType;
   WCHAR    szMicString[64];
   DWORD    dwSpeakers;
   DWORD    dwLastMicLevel;
   DWORD    dwMicLevelDeviceID;
} MICWIZARDINFO2, *PMICWIZARDINFO2;

// STLexDlg
#define  STLD_DISABLEREMOVE   SETBIT(0)
#define  STLD_DISABLEADD      SETBIT(1)
#define  STLD_FORCEEDIT       SETBIT(2)
#define  STLD_DISABLEPRONADDREMOVE SETBIT(3)
#define  STLD_TEST            SETBIT(4)
#define  STLD_DISABLERENAME   SETBIT(5)
#define  STLD_CHANGEPRONADDS  SETBIT(6)

/* class IDs */
// {B9F11A95-90E3-11d0-8D77-00A0C9034A7E}
DEFINE_GUID(CLSID_STMicWizard, 
0xb9f11a95, 0x90e3, 0x11d0, 0x8d, 0x77, 0x0, 0xa0, 0xc9, 0x3, 0x4a, 0x7e);

// {92655FB1-ADF9-11d1-BEB9-006008317CE8}
DEFINE_GUID(CLSID_STLexDlg, 
0x92655fb1, 0xadf9, 0x11d1, 0xbe, 0xb9, 0x0, 0x60, 0x8, 0x31, 0x7c, 0xe8);

// {B9F11A96-90E3-11d0-8D77-00A0C9034A7E}
DEFINE_GUID(CLSID_STPhoneConv, 
0xb9f11a96, 0x90e3, 0x11d0, 0x8d, 0x77, 0x0, 0xa0, 0xc9, 0x3, 0x4a, 0x7e);


/* interfaces */
// ISTMicWizard Interface

#undef   INTERFACE
#define  INTERFACE   ISTMicWizard

// {B9F11A97-90E3-11d0-8D77-00A0C9034A7E}
DEFINE_GUID(IID_ISTMicWizard, 
0xb9f11a97, 0x90e3, 0x11d0, 0x8d, 0x77, 0x0, 0xa0, 0xc9, 0x3, 0x4a, 0x7e);

DECLARE_INTERFACE_ (ISTMicWizard, IUnknown) {
   // IUnknown members
   STDMETHOD(QueryInterface)  (THIS_ REFIID riid, LPVOID FAR* ppvObj) PURE;
   STDMETHOD_(ULONG,AddRef)   (THIS) PURE;
   STDMETHOD_(ULONG,Release)  (THIS) PURE;

   // ISTMicWizard members
   STDMETHOD (InfoGet) (THIS_ PMICWIZARDINFO) PURE;
   STDMETHOD (InfoSet) (THIS_ PMICWIZARDINFO) PURE;
   STDMETHOD (Wizard) (THIS_ HWND, DWORD, DWORD, DWORD, DWORD) PURE;
   };

typedef ISTMicWizard FAR * PISTMICWIZARD;


// ISTLexDlg Interface

#undef   INTERFACE
#define  INTERFACE   ISTLexDlg

// {A27B8A01-ADF9-11d1-BEB9-006008317CE8}
DEFINE_GUID(IID_ISTLexDlg, 
0xa27b8a01, 0xadf9, 0x11d1, 0xbe, 0xb9, 0x0, 0x60, 0x8, 0x31, 0x7c, 0xe8);

DECLARE_INTERFACE_ (ISTLexDlg, IUnknown) {
   // IUnknown members
   STDMETHOD(QueryInterface)  (THIS_ REFIID riid, LPVOID FAR* ppvObj) PURE;
   STDMETHOD_(ULONG,AddRef)   (THIS) PURE;
   STDMETHOD_(ULONG,Release)  (THIS) PURE;

   // ISTLexDlg members
   STDMETHOD (LexDlg) (THIS_ HWND, PCWSTR, DWORD, LANGID,
      LPUNKNOWN, LPUNKNOWN, LPUNKNOWN, PCWSTR, PCWSTR) PURE;
   };

typedef ISTLexDlg FAR * PISTLEXDLG;

// ISTPhoneConv Interface

#undef   INTERFACE
#define  INTERFACE   ISTPhoneConv

// {B9F11A98-90E3-11d0-8D77-00A0C9034A7E}
DEFINE_GUID(IID_ISTPhoneConv, 
0xb9f11a98, 0x90e3, 0x11d0, 0x8d, 0x77, 0x0, 0xa0, 0xc9, 0x3, 0x4a, 0x7e);

DECLARE_INTERFACE_ (ISTPhoneConv, IUnknown) {
   // IUnknown members
   STDMETHOD(QueryInterface)  (THIS_ REFIID riid, LPVOID FAR* ppvObj) PURE;
   STDMETHOD_(ULONG,AddRef)   (THIS) PURE;
   STDMETHOD_(ULONG,Release)  (THIS) PURE;

   // ISTPhoneConv members
   STDMETHOD (FromIPA) (THIS_ LPCWSTR, DWORD *, LPWSTR, DWORD, DWORD*) PURE;
   STDMETHOD (Get) (THIS_ SDATA *) PURE;
   STDMETHOD (SetByLang) (THIS_ LANGID) PURE;
   STDMETHOD (SetByText) (THIS_ LPCWSTR) PURE;
   STDMETHOD (ToIPA) (THIS_ LPCWSTR, DWORD *, LPWSTR, DWORD, DWORD*) PURE;
   };

typedef ISTPhoneConv FAR * PISTPHONECONV;



/************************************************************************
Audio source/destiantion API
*/

/************************************************************************
defines */

// AudioStop
#define      IANSRSN_NODATA             0
#define      IANSRSN_PRIORITY           1
#define      IANSRSN_INACTIVE           2
#define      IANSRSN_EOF                3

// IAudioSourceInstrumented::StateSet
#define          IASISTATE_PASSTHROUGH      0
#define          IASISTATE_PASSNOTHING      1
#define          IASISTATE_PASSREADFROMWAVE 2
#define          IASISTATE_PASSWRITETOWAVE  3

/************************************************************************
typedefs */

/************************************************************************
Class IDs */
// {CB96B400-C743-11cd-80E5-00AA003E4B50}
DEFINE_GUID(CLSID_MMAudioDest, 
0xcb96b400, 0xc743, 0x11cd, 0x80, 0xe5, 0x0, 0xaa, 0x0, 0x3e, 0x4b, 0x50);

// {D24FE500-C743-11cd-80E5-00AA003E4B50}
DEFINE_GUID(CLSID_MMAudioSource, 
0xd24fe500, 0xc743, 0x11cd, 0x80, 0xe5, 0x0, 0xaa, 0x0, 0x3e, 0x4b, 0x50);

// {D4023720-E4B9-11cf-8D56-00A0C9034A7E}
DEFINE_GUID(CLSID_InstAudioSource, 
0xd4023720, 0xe4b9, 0x11cf, 0x8d, 0x56, 0x0, 0xa0, 0xc9, 0x3, 0x4a, 0x7e);

// {D4123720-E4B9-11cf-8D56-00A0C9034A7E}
DEFINE_GUID(CLSID_SuspAudioSource, 
0xd4123720, 0xe4b9, 0x11cf, 0x8d, 0x56, 0x0, 0xa0, 0xc9, 0x3, 0x4a, 0x7e);

// {D4523720-E4B9-11cf-8D56-00A0C9034A7E}
DEFINE_GUID(CLSID_AudioSourceFile, 
0xd4523720, 0xe4b9, 0x11cf, 0x8d, 0x56, 0x0, 0xa0, 0xc9, 0x3, 0x4a, 0x7e);

// {D4523720-E4B9-11cf-8D56-00A0C9034A7E}
DEFINE_GUID(CLSID_AudioDestFile, 
0xd4623720, 0xe4b9, 0x11cf, 0x8d, 0x56, 0x0, 0xa0, 0xc9, 0x3, 0x4a, 0x7e);

// {B9F11A90-90E3-11d0-8D77-00A0C9034A7E}
DEFINE_GUID(CLSID_AudioDestDirect, 
0xb9f11a90, 0x90e3, 0x11d0, 0x8d, 0x77, 0x0, 0xa0, 0xc9, 0x3, 0x4a, 0x7e);

// {32C35401-D04F-11d0-99B3-00AA004CD65C}
DEFINE_GUID(CLSID_AudioSourceDirect, 
0x32c35401, 0xd04f, 0x11d0, 0x99, 0xb3, 0x0, 0xaa, 0x0, 0x4c, 0xd6, 0x5c);


/************************************************************************
interfaces */

// IAudio
#undef   INTERFACE
#define  INTERFACE   IAudio

// {F546B340-C743-11cd-80E5-00AA003E4B50}
DEFINE_GUID(IID_IAudio, 
0xf546b340, 0xc743, 0x11cd, 0x80, 0xe5, 0x0, 0xaa, 0x0, 0x3e, 0x4b, 0x50);

DECLARE_INTERFACE_ (IAudio, IUnknown) {
   // IUnknown members
   STDMETHOD(QueryInterface)  (THIS_ REFIID riid, LPVOID FAR* ppvObj) PURE;
   STDMETHOD_(ULONG,AddRef)   (THIS) PURE;
   STDMETHOD_(ULONG,Release)  (THIS) PURE;

   // IAudio members
   STDMETHOD (Flush)          (THIS) PURE;
   STDMETHOD (LevelGet)       (THIS_ DWORD *) PURE;
   STDMETHOD (LevelSet)       (THIS_ DWORD) PURE;
   STDMETHOD (PassNotify)     (THIS_ PVOID, IID) PURE;
   STDMETHOD (PosnGet)        (THIS_ PQWORD) PURE;
   STDMETHOD (Claim)          (THIS) PURE;
   STDMETHOD (UnClaim)        (THIS) PURE;
   STDMETHOD (Start)          (THIS) PURE;
   STDMETHOD (Stop)           (THIS) PURE;
   STDMETHOD (TotalGet)       (THIS_ PQWORD) PURE;
   STDMETHOD (ToFileTime)     (THIS_ PQWORD, FILETIME *) PURE;
   STDMETHOD (WaveFormatGet)  (THIS_ PSDATA) PURE;
   STDMETHOD (WaveFormatSet)  (THIS_ SDATA) PURE;
   };

typedef IAudio FAR * PIAUDIO;

// IAudioDest

#undef   INTERFACE
#define  INTERFACE   IAudioDest

// {2EC34DA0-C743-11cd-80E5-00AA003E4B50}
DEFINE_GUID(IID_IAudioDest, 
0x2ec34da0, 0xc743, 0x11cd, 0x80, 0xe5, 0x0, 0xaa, 0x0, 0x3e, 0x4b, 0x50);

DECLARE_INTERFACE_ (IAudioDest, IUnknown) {

   // IUnknown members
   STDMETHOD (QueryInterface) (THIS_ REFIID riid, LPVOID FAR* ppvObj) PURE;
   STDMETHOD_(ULONG,AddRef)   (THIS) PURE;
   STDMETHOD_(ULONG,Release)  (THIS) PURE;

   // IAudioDest members
   STDMETHOD (FreeSpace)      (THIS_ DWORD *, BOOL *) PURE;
   STDMETHOD (DataSet)        (THIS_ PVOID, DWORD) PURE;
   STDMETHOD (BookMark)       (THIS_ DWORD) PURE;
   };

typedef IAudioDest FAR * PIAUDIODEST;



// IAudioDestNotifySink

#undef   INTERFACE
#define  INTERFACE   IAudioDestNotifySink

// {ACB08C00-C743-11cd-80E5-00AA003E4B50}
DEFINE_GUID(IID_IAudioDestNotifySink, 
0xacb08c00, 0xc743, 0x11cd, 0x80, 0xe5, 0x0, 0xaa, 0x0, 0x3e, 0x4b, 0x50);

DECLARE_INTERFACE_ (IAudioDestNotifySink, IUnknown) {
   // IUnknown members
   STDMETHOD (QueryInterface) (THIS_ REFIID riid, LPVOID FAR* ppvObj) PURE;
   STDMETHOD_(ULONG,AddRef)   (THIS) PURE;
   STDMETHOD_(ULONG,Release)  (THIS) PURE;

   // IAudioDestNotifySink members
   STDMETHOD (AudioStop)      (THIS_ WORD) PURE;
   STDMETHOD (AudioStart)     (THIS) PURE;
   STDMETHOD (FreeSpace)      (THIS_ DWORD, BOOL) PURE;
   STDMETHOD (BookMark)       (THIS_ DWORD, BOOL) PURE;
   };

typedef IAudioDestNotifySink FAR * PIAUDIODESTNOTIFYSINK;



// IAudioMultiMediaDevice

#undef   INTERFACE
#define  INTERFACE   IAudioMultiMediaDevice

// {B68AD320-C743-11cd-80E5-00AA003E4B50}
DEFINE_GUID(IID_IAudioMultiMediaDevice, 
0xb68ad320, 0xc743, 0x11cd, 0x80, 0xe5, 0x0, 0xaa, 0x0, 0x3e, 0x4b, 0x50);

DECLARE_INTERFACE_ (IAudioMultiMediaDevice, IUnknown) {
   // IUnknown members
   STDMETHOD (QueryInterface) (THIS_ REFIID riid, LPVOID FAR* ppvObj) PURE;
   STDMETHOD_(ULONG,AddRef)   (THIS) PURE;
   STDMETHOD_(ULONG,Release)  (THIS) PURE;

   // IAudioMultiMediaDevice members
   STDMETHOD (CustomMessage)  (THIS_ UINT, SDATA) PURE;
   STDMETHOD (DeviceNumGet)   (THIS_ DWORD*) PURE;
   STDMETHOD (DeviceNumSet)   (THIS_ DWORD) PURE;
   };

typedef IAudioMultiMediaDevice FAR * PIAUDIOMULTIMEDIADEVICE;



// IAudioSource
#undef   INTERFACE
#define  INTERFACE   IAudioSource

// {BC06A220-C743-11cd-80E5-00AA003E4B50}
DEFINE_GUID(IID_IAudioSource, 
0xbc06a220, 0xc743, 0x11cd, 0x80, 0xe5, 0x0, 0xaa, 0x0, 0x3e, 0x4b, 0x50);

DECLARE_INTERFACE_ (IAudioSource, IUnknown) {
   // IUnknown members
   STDMETHOD (QueryInterface) (THIS_ REFIID riid, LPVOID FAR* ppvObj) PURE;
   STDMETHOD_(ULONG,AddRef)   (THIS) PURE;
   STDMETHOD_(ULONG,Release)  (THIS) PURE;

   // IAudioSource members
   STDMETHOD (DataAvailable)  (THIS_ DWORD *, BOOL *) PURE;
   STDMETHOD (DataGet)        (THIS_ PVOID, DWORD, DWORD *) PURE;
   };

typedef IAudioSource FAR * PIAUDIOSOURCE;



// IAudioSourceInstrumented
#undef   INTERFACE
#define  INTERFACE   IAudioSourceInstrumented

// {D4023721-E4B9-11cf-8D56-00A0C9034A7E}
DEFINE_GUID(IID_IAudioSourceInstrumented, 
0xd4023721, 0xe4b9, 0x11cf, 0x8d, 0x56, 0x0, 0xa0, 0xc9, 0x3, 0x4a, 0x7e);

DECLARE_INTERFACE_ (IAudioSourceInstrumented, IUnknown) {
   // IUnknown members
   STDMETHOD (QueryInterface) (THIS_ REFIID riid, LPVOID FAR* ppvObj) PURE;
   STDMETHOD_(ULONG,AddRef)   (THIS) PURE;
   STDMETHOD_(ULONG,Release)  (THIS) PURE;

   // IAudioSourceInstrumented members
   STDMETHOD (AudioSource)    (THIS_ LPUNKNOWN) PURE;
   STDMETHOD (RegistryGet)    (THIS_ PWSTR, DWORD, DWORD *) PURE;
   STDMETHOD (RegistrySet)    (THIS_ PCWSTR) PURE;
   STDMETHOD (StateGet)       (THIS_ DWORD*) PURE;
   STDMETHOD (StateSet)       (THIS_ DWORD) PURE;
   STDMETHOD (WaveFileReadGet)(THIS_ PWSTR, DWORD, DWORD *) PURE;
   STDMETHOD (WaveFileReadSet)(THIS_ PCWSTR) PURE;
   STDMETHOD (WaveFileWriteGet)(THIS_ PWSTR, DWORD, DWORD *) PURE;
   STDMETHOD (WaveFileWriteSet)(THIS_ PCWSTR) PURE;
   };

typedef IAudioSourceInstrumented FAR * PIAUDIOSOURCEINSTRUMENTED;


// IAudioSourceSusp
#undef   INTERFACE
#define  INTERFACE   IAudioSourceSusp

// {D4123721-E4B9-11cf-8D56-00A0C9034A7E}
DEFINE_GUID(IID_IAudioSourceSusp, 
0xd4123721, 0xe4b9, 0x11cf, 0x8d, 0x56, 0x0, 0xa0, 0xc9, 0x3, 0x4a, 0x7e);

DECLARE_INTERFACE_ (IAudioSourceSusp, IUnknown) {
   // IUnknown members
   STDMETHOD (QueryInterface) (THIS_ REFIID riid, LPVOID FAR* ppvObj) PURE;
   STDMETHOD_(ULONG,AddRef)   (THIS) PURE;
   STDMETHOD_(ULONG,Release)  (THIS) PURE;

   // IAudioSourceSusp members
   STDMETHOD (AudioSource)    (THIS_ LPUNKNOWN) PURE;
   STDMETHOD (Pause)          (THIS) PURE;
   STDMETHOD (Resume)         (THIS) PURE;
   };

typedef IAudioSourceSusp FAR * PIAUDIOSOURCESUSP;


// IAudioSourceNotifySink
#undef   INTERFACE
#define  INTERFACE   IAudioSourceNotifySink

// {C0BD9A80-C743-11cd-80E5-00AA003E4B50}
DEFINE_GUID(IID_IAudioSourceNotifySink, 
0xc0bd9a80, 0xc743, 0x11cd, 0x80, 0xe5, 0x0, 0xaa, 0x0, 0x3e, 0x4b, 0x50);

DECLARE_INTERFACE_ (IAudioSourceNotifySink, IUnknown) {
   // IUnknown members
   STDMETHOD(QueryInterface)  (THIS_ REFIID riid, LPVOID FAR* ppvObj) PURE;
   STDMETHOD_(ULONG,AddRef)   (THIS) PURE;
   STDMETHOD_(ULONG,Release)  (THIS) PURE;

   // IAudioSourceNotifySink members
   STDMETHOD (AudioStop)      (THIS_ WORD) PURE;
   STDMETHOD (AudioStart)     (THIS) PURE;
   STDMETHOD (DataAvailable)  (THIS_ DWORD, BOOL) PURE;
   STDMETHOD (Overflow)       (THIS_ DWORD) PURE;
   };

typedef IAudioSourceNotifySink FAR * PIAUDIOSOURCENOTIFYSINK;

// IAudioFileNotifySink Interface
#undef   INTERFACE
#define  INTERFACE   IAudioFileNotifySink

// {492FE490-51E7-11b9-C000-FED6CBA3B1A9}
DEFINE_GUID( IID_IAudioFileNotifySink, 0x492fe490, 0x51e7, 0x11b9, 0xc0, 0x0, 0xfe, 0xd6, 0xcb, 0xa3, 0xb1, 0xa9);

DECLARE_INTERFACE_ (IAudioFileNotifySink, IUnknown) {
   // IUnknown members
   STDMETHOD(QueryInterface)  (THIS_ REFIID riid, LPVOID FAR* ppvObj) PURE;
   STDMETHOD_(ULONG,AddRef)   (THIS) PURE;
   STDMETHOD_(ULONG,Release)  (THIS) PURE;

   // IAudioSourceNotifySink members
   STDMETHOD (FileBegin)      (THIS_ DWORD) PURE;
   STDMETHOD (FileEnd)        (THIS_ DWORD) PURE;
   STDMETHOD (QueueEmpty)     (THIS) PURE;
   STDMETHOD (Posn)           (THIS_ QWORD, QWORD) PURE;
   };

typedef IAudioFileNotifySink FAR * PIAUDIOFILENOTIFYSINK;



// IAudioFile Interface

#undef   INTERFACE
#define  INTERFACE   IAudioFile

// {FD7C2320-3D6D-11b9-C000-FED6CBA3B1A9}
DEFINE_GUID( IID_IAudioFile, 0xfd7c2320, 0x3d6d, 0x11b9, 0xc0, 0x0, 0xfe, 0xd6, 0xcb, 0xa3, 0xb1, 0xa9);

DECLARE_INTERFACE_ (IAudioFile, IUnknown) {
   // IUnknown members
   STDMETHOD(QueryInterface)  (THIS_ REFIID riid, LPVOID FAR* ppvObj) PURE;
   STDMETHOD_(ULONG,AddRef)   (THIS) PURE;
   STDMETHOD_(ULONG,Release)  (THIS) PURE;

   // IAudioFile members
   STDMETHOD (Register) (THIS_ PIAUDIOFILENOTIFYSINK) PURE;
   STDMETHOD (Set) (THIS_ LPCWSTR pszFile, DWORD dwID) PURE;
   STDMETHOD (Add) (THIS_ LPCWSTR pszFile, DWORD dwID) PURE;
   STDMETHOD (Flush) (THIS) PURE;
   STDMETHOD (RealTimeSet) (THIS_ WORD wTime) PURE;
   STDMETHOD (RealTimeGet) (THIS_ WORD *pwTime) PURE;
   };

typedef IAudioFile FAR * PIAUDIOFILE;


// IAudioDirect Interface

#undef   INTERFACE
#define  INTERFACE   IAudioDirect

// {B9F11A94-90E3-11d0-8D77-00A0C9034A7E}
DEFINE_GUID(IID_IAudioDirect, 
0xb9f11a94, 0x90e3, 0x11d0, 0x8d, 0x77, 0x0, 0xa0, 0xc9, 0x3, 0x4a, 0x7e);

DECLARE_INTERFACE_ (IAudioDirect, IUnknown) {
   // IUnknown members
   STDMETHOD(QueryInterface)  (THIS_ REFIID riid, LPVOID FAR* ppvObj) PURE;
   STDMETHOD_(ULONG,AddRef)   (THIS) PURE;
   STDMETHOD_(ULONG,Release)  (THIS) PURE;

   // IAudioDirect members
   STDMETHOD (Init) (THIS_ PVOID, REFIID) PURE;
   };

typedef IAudioDirect FAR * PIAUDIODIRECT;



/************************************************************************
defines */
/* SRINFO */
#define  SRMI_NAMELEN                  SVFN_LEN

#define  SRSEQUENCE_DISCRETE           (0)
#define  SRSEQUENCE_CONTINUOUS          (1)
#define  SRSEQUENCE_WORDSPOT            (2)
#define  SRSEQUENCE_CONTCFGDISCDICT     (3)

#define  SRGRAM_CFG                    SETBIT(0)
#define  SRGRAM_DICTATION              SETBIT(1)
#define  SRGRAM_LIMITEDDOMAIN          SETBIT(2)

#define  SRFEATURE_INDEPSPEAKER        SETBIT(0)
#define  SRFEATURE_INDEPMICROPHONE     SETBIT(1)
#define  SRFEATURE_TRAINWORD           SETBIT(2)
#define  SRFEATURE_TRAINPHONETIC       SETBIT(3)
#define  SRFEATURE_WILDCARD            SETBIT(4)
#define  SRFEATURE_ANYWORD             SETBIT(5)
#define  SRFEATURE_PCOPTIMIZED         SETBIT(6)
#define  SRFEATURE_PHONEOPTIMIZED      SETBIT(7)
#define  SRFEATURE_GRAMLIST            SETBIT(8)
#define  SRFEATURE_GRAMLINK            SETBIT(9)
#define  SRFEATURE_MULTILINGUAL        SETBIT(10)
#define  SRFEATURE_GRAMRECURSIVE       SETBIT(11)
#define  SRFEATURE_IPAUNICODE          SETBIT(12)
#define	SRFEATURE_SINGLEINSTANCE      SETBIT(13)
#define	SRFEATURE_THREADSAFE          SETBIT(14)
#define  SRFEATURE_FIXEDAUDIO          SETBIT(15)
#define  SRFEATURE_IPAWORD             SETBIT(16)
#define  SRFEATURE_SAPI4               SETBIT(17)

#define  SRI_ILEXPRONOUNCE             SETBIT(0)
#define  SRI_ISRATTRIBUTES             SETBIT(1)
#define  SRI_ISRCENTRAL                SETBIT(2)
#define  SRI_ISRDIALOGS                SETBIT(3)
#define  SRI_ISRGRAMCOMMON             SETBIT(4)
#define  SRI_ISRGRAMCFG                SETBIT(5)
#define  SRI_ISRGRAMDICTATION          SETBIT(6)
#define  SRI_ISRGRAMINSERTIONGUI       SETBIT(7)
#define  SRI_ISRESBASIC                SETBIT(8)
#define  SRI_ISRESMERGE                SETBIT(9)
#define  SRI_ISRESAUDIO                SETBIT(10)
#define  SRI_ISRESCORRECTION           SETBIT(11)
#define  SRI_ISRESEVAL                 SETBIT(12)
#define  SRI_ISRESGRAPH                SETBIT(13)
#define  SRI_ISRESMEMORY               SETBIT(14)
#define  SRI_ISRESMODIFYGUI            SETBIT(15)
#define  SRI_ISRESSPEAKER              SETBIT(16)
#define  SRI_ISRSPEAKER                SETBIT(17)
#define  SRI_ISRESSCORES               SETBIT(18)
#define  SRI_ISRESAUDIOEX              SETBIT(19)
#define  SRI_ISRGRAMLEXPRON            SETBIT(20)
#define  SRI_ISRRESGRAPHEX             SETBIT(21)
#define  SRI_ILEXPRONOUNCE2            SETBIT(22)
#define  SRI_IATTRIBUTES               SETBIT(23)
#define  SRI_ISRSPEAKER2               SETBIT(24)
#define  SRI_ISRDIALOGS2               SETBIT(25)


// ISRGramCommon::TrainQuery
#define   SRGRAMQ_NONE                    0
#define   SRGRAMQ_GENERALTRAIN            1
#define   SRGRAMQ_PHRASE                  2
#define   SRGRAMQ_DIALOG                  3

// ISRGramNotifySink::PhraseFinish
#define   ISRNOTEFIN_RECOGNIZED         SETBIT(0)
#define   ISRNOTEFIN_THISGRAMMAR        SETBIT(1)
#define   ISRNOTEFIN_FROMTHISGRAMMAR    SETBIT(2)

// ISRGramNotifySink::Training
#define   SRGNSTRAIN_GENERAL            SETBIT(0)
#define   SRGNSTRAIN_GRAMMAR            SETBIT(1)
#define   SRGNSTRAIN_MICROPHONE         SETBIT(2)

// ISRNotifySink::AttribChange
#define   ISRNSAC_AUTOGAINENABLE        1
#define   ISRNSAC_THRESHOLD             2
#define   ISRNSAC_ECHO                  3
#define   ISRNSAC_ENERGYFLOOR           4
#define   ISRNSAC_MICROPHONE            5
#define   ISRNSAC_REALTIME              6
#define   ISRNSAC_SPEAKER               7
#define   ISRNSAC_TIMEOUT               8
#define   ISRNSAC_STARTLISTENING        9
#define   ISRNSAC_STOPLISTENING        10

/* Interference */
#define  SRMSGINT_NOISE                (0x0001)
#define  SRMSGINT_NOSIGNAL             (0x0002)
#define  SRMSGINT_TOOLOUD              (0x0003)
#define  SRMSGINT_TOOQUIET             (0x0004)
#define  SRMSGINT_AUDIODATA_STOPPED    (0x0005)
#define  SRMSGINT_AUDIODATA_STARTED    (0x0006)
#define  SRMSGINT_IAUDIO_STARTED       (0x0007)
#define  SRMSGINT_IAUDIO_STOPPED       (0x0008)

// Grammar header values
#define   SRHDRTYPE_CFG                  0
#define   SRHDRTYPE_LIMITEDDOMAIN        1
#define   SRHDRTYPE_DICTATION            2

#define   SRHDRFLAG_UNICODE              SETBIT(0)  

// results objects defines
#define   SRRESCUE_COMMA                  1
#define   SRRESCUE_DECLARATIVEBEGIN       2
#define   SRRESCUE_DECLARATIVEEND         3
#define   SRRESCUE_IMPERATIVEBEGIN        4
#define   SRRESCUE_IMPERATIVEEND          5
#define   SRRESCUE_INTERROGATIVEBEGIN     6
#define   SRRESCUE_INTERROGATIVEEND       7
#define   SRRESCUE_NOISE                  8
#define   SRRESCUE_PAUSE                  9
#define   SRRESCUE_SENTENCEBEGIN          10
#define   SRRESCUE_SENTENCEEND            11
#define   SRRESCUE_UM                     12
#define   SRRESCUE_WILDCARD               13
#define   SRRESCUE_WORD                   14

/* SRCFGSYMBOL */
#define  SRCFG_STARTOPERATION          (1)
#define  SRCFG_ENDOPERATION            (2)
#define  SRCFG_WORD                    (3)
#define  SRCFG_RULE                    (4)
#define  SRCFG_WILDCARD                (5)
#define  SRCFG_LIST                    (6)

#define  SRCFGO_SEQUENCE               (1)
#define  SRCFGO_ALTERNATIVE            (2)
#define  SRCFGO_REPEAT                 (3)
#define  SRCFGO_OPTIONAL               (4)


// Grammar-chunk IDs
#define   SRCK_LANGUAGE                  1
#define   SRCKCFG_WORDS                  2
#define   SRCKCFG_RULES                  3
#define   SRCKCFG_EXPORTRULES            4
#define   SRCKCFG_IMPORTRULES            5
#define   SRCKCFG_LISTS                  6
#define   SRCKD_TOPIC                    7
#define   SRCKD_COMMON                   8
#define   SRCKD_GROUP                    9
#define   SRCKD_SAMPLE                   10
#define   SRCKLD_WORDS                   11
#define   SRCKLD_GROUP                   12
#define   SRCKLD_SAMPLE                  13 
#define   SRCKD_WORDCOUNT                14
#define   SRCKD_NGRAM                    15

/* TrainQuery */
#define  SRTQEX_REQUIRED               (0x0000)
#define  SRTQEX_RECOMMENDED            (0x0001)

/* ISRResAudioEx */
#define  SRAUDIOTIMESTAMP_DEFAULT      ((QWORD) -1)

/* ISRResCorrection */
#define  SRCORCONFIDENCE_SOME          (0x0001)
#define  SRCORCONFIDENCE_VERY          (0x0002)

/* ISRResGraphEx */
DEFINE_GUID(SRGARC_ACOUSTICSCORE, 0xbf2f5b20, 0xb91d, 0x11d1, 0xb7, 0xbc, 0x0, 0x60, 0x97, 0x98, 0x41, 0xb4);
DEFINE_GUID(SRGARC_WORDTEXT, 0xbf2f5b21, 0xb91d, 0x11d1, 0xb7, 0xbc, 0x0, 0x60, 0x97, 0x98, 0x41, 0xb4);
DEFINE_GUID(SRGARC_NONLEXICALTEXT, 0x62ec00, 0xbaa0, 0x11d1, 0xb7, 0xbd, 0x0, 0x60, 0x97, 0x98, 0x41, 0xb4);
DEFINE_GUID(SRGARC_WORDPRONUNCIATION, 0xbf2f5b22, 0xb91d, 0x11d1, 0xb7, 0xbc, 0x0, 0x60, 0x97, 0x98, 0x41, 0xb4);
DEFINE_GUID(SRGNODE_TIME, 0xbf2f5b23, 0xb91d, 0x11d1, 0xb7, 0xbc, 0x0, 0x60, 0x97, 0x98, 0x41, 0xb4);

#define  SRGEX_ACOUSTICONLY            SETBIT(0)
#define  SRGEX_LMONLY                  SETBIT(1)
#define  SRGEX_ACOUSTICANDLM           SETBIT(2)

/* ISRResMemory constants */
#define  SRRESMEMKIND_AUDIO            SETBIT(0)
#define  SRRESMEMKIND_CORRECTION       SETBIT(1)
#define  SRRESMEMKIND_EVAL             SETBIT(2)
#define  SRRESMEMKIND_PHONEMEGRAPH     SETBIT(3)
#define  SRRESMEMKIND_WORDGRAPH        SETBIT(4)

// Attribute minimums and maximums
#define  SRATTR_MINAUTOGAIN               0
#define  SRATTR_MAXAUTOGAIN               100
#define  SRATTR_MINENERGYFLOOR            0
#define  SRATTR_MAXENERGYFLOOR            0xffff
#define  SRATTR_MINREALTIME               0
#define  SRATTR_MAXREALTIME               0xffffffff
#define  SRATTR_MINTHRESHOLD              0
#define  SRATTR_MAXTHRESHOLD              100
#define  SRATTR_MINTOINCOMPLETE           0
#define  SRATTR_MAXTOINCOMPLETE           0xffffffff
#define  SRATTR_MINTOCOMPLETE             0
#define  SRATTR_MAXTOCOMPLETE             0xffffffff


/************************************************************************
typedefs */

typedef struct {
   DWORD    dwSize;
   DWORD    dwUniqueID;
   BYTE     abData[0];
   } SRCFGRULE, * PSRCFGRULE;



typedef struct {
   DWORD    dwSize;
   DWORD    dwRuleNum;
   WCHAR    szString[0];
   } SRCFGIMPRULEW, * PSRCFGIMPRULEW;

typedef struct {
   DWORD    dwSize;
   DWORD    dwRuleNum;
   CHAR     szString[0];
   } SRCFGIMPRULEA, * PSRCFGIMPRULEA;

#ifdef  _S_UNICODE
#define  SRCFGIMPRULE      SRCFGIMPRULEW
#define  PSRCFGIMPRULE     PSRCFGIMPRULEW
#else
#define  SRCFGIMPRULE      SRCFGIMPRULEA
#define  PSRCFGIMPRULE     PSRCFGIMPRULEA
#endif  // _S_UNICODE



typedef struct {
   DWORD    dwSize;
   DWORD    dwRuleNum;
   WCHAR    szString[0];
   } SRCFGXRULEW, * PSRCFGXRULEW;

typedef struct {
   DWORD    dwSize;
   DWORD    dwRuleNum;
   CHAR     szString[0];
   } SRCFGXRULEA, * PSRCFGXRULEA;

#ifdef  _S_UNICODE
#define  SRCFGXRULE     SRCFGXRULEW
#define  PSRCFGXRULE    PSRCFGXRULEW
#else
#define  SRCFGXRULE     SRCFGXRULEA
#define  PSRCFGXRULE    PSRCFGXRULEA
#endif  // _S_UNICODE



typedef struct {
   DWORD    dwSize;
   DWORD    dwListNum;
   WCHAR    szString[0];
   } SRCFGLISTW, * PSRCFGLISTW;

typedef struct {
   DWORD    dwSize;
   DWORD    dwListNum;
   CHAR     szString[0];
   } SRCFGLISTA, * PSRCFGLISTA;

#ifdef  _S_UNICODE
#define  SRCFGLIST      SRCFGLISTW
#define  PSRCFGLIST     PSRCFGLISTW
#else
#define  SRCFGLIST      SRCFGLISTA
#define  PSRCFGLIST     PSRCFGLISTA
#endif  // _S_UNICODE



typedef struct {
   WORD     wType;
   WORD     wProbability;
   DWORD    dwValue;
   } SRCFGSYMBOL, * PSRCFGSYMBOL;



typedef struct {
   DWORD    dwSize;
   DWORD    dwWordNum;
   WCHAR    szWord[0];
   } SRWORDW, * PSRWORDW;

typedef struct {
   DWORD    dwSize;
   DWORD    dwWordNum;
   CHAR     szWord[0];
   } SRWORDA, * PSRWORDA;

#ifdef  _S_UNICODE
#define  SRWORD      SRWORDW
#define  PSRWORD     PSRWORDW
#else
#define  SRWORD      SRWORDA
#define  PSRWORD     PSRWORDA
#endif  // _S_UNICODE



typedef struct {
   DWORD    dwSize;
   BYTE     abWords[0];
   } SRPHRASEW, * PSRPHRASEW;

typedef struct {
   DWORD    dwSize;
   BYTE     abWords[0];
   } SRPHRASEA, * PSRPHRASEA;

#ifdef  _S_UNICODE
#define  SRPHRASE    SRPHRASEW
#define  PSRPHRASE   PSRPHRASEW
#else
#define  SRPHRASE    SRPHRASEA
#define  PSRPHRASE   PSRPHRASEA
#endif  // _S_UNICODE



typedef struct {
   DWORD      dwType;
   DWORD      dwFlags;
   } SRHEADER, *PSRHEADER;

typedef struct {
   DWORD      dwChunkID;
   DWORD      dwChunkSize;
   BYTE       avInfo[0];
   } SRCHUNK, *PSRCHUNK;



typedef struct {
   GUID       gEngineID;
   WCHAR      szMfgName[SRMI_NAMELEN];
   WCHAR      szProductName[SRMI_NAMELEN];
   GUID       gModeID;
   WCHAR      szModeName[SRMI_NAMELEN];
   LANGUAGEW  language;
   DWORD      dwSequencing;
   DWORD      dwMaxWordsVocab;
   DWORD      dwMaxWordsState;
   DWORD      dwGrammars;
   DWORD      dwFeatures;
   DWORD      dwInterfaces;
   DWORD      dwEngineFeatures;
   } SRMODEINFOW, * PSRMODEINFOW;

typedef struct {
   GUID       gEngineID;
   CHAR       szMfgName[SRMI_NAMELEN];
   CHAR       szProductName[SRMI_NAMELEN];
   GUID       gModeID;
   CHAR       szModeName[SRMI_NAMELEN];
   LANGUAGEA  language;
   DWORD      dwSequencing;
   DWORD      dwMaxWordsVocab;
   DWORD      dwMaxWordsState;
   DWORD      dwGrammars;
   DWORD      dwFeatures;
   DWORD      dwInterfaces;
   DWORD      dwEngineFeatures;
   } SRMODEINFOA, * PSRMODEINFOA;

#ifdef  _S_UNICODE
#define  SRMODEINFO     SRMODEINFOW
#define  PSRMODEINFO    PSRMODEINFOW
#else
#define  SRMODEINFO     SRMODEINFOA
#define  PSRMODEINFO    PSRMODEINFOA
#endif  // _S_UNICODE



typedef struct {
   DWORD      dwEngineID;
   DWORD      dwMfgName;
   DWORD      dwProductName;
   DWORD      dwModeID;
   DWORD      dwModeName;
   DWORD      dwLanguage;
   DWORD      dwDialect;
   DWORD      dwSequencing;
   DWORD      dwMaxWordsVocab;
   DWORD      dwMaxWordsState;
   DWORD      dwGrammars;
   DWORD      dwFeatures;
   DWORD      dwInterfaces;
   DWORD      dwEngineFeatures;
   } SRMODEINFORANK, * PSRMODEINFORANK;



// speech recognition enumeration sharing object
typedef struct {
   QWORD        qwInstanceID;
   DWORD        dwDeviceID;
   SRMODEINFOW  srModeInfo;
} SRSHAREW, * PSRSHAREW;

typedef struct {
   QWORD        qwInstanceID;
   DWORD        dwDeviceID;
   SRMODEINFOA  srModeInfo;
} SRSHAREA, * PSRSHAREA;

#ifdef  _S_UNICODE
#define  SRSHARE    SRSHAREW
#define  PSRSHARE   PSRSHAREW
#else
#define  SRSHARE    SRSHAREA
#define  PSRSHARE   PSRSHAREA
#endif  // _S_UNICODE




// ISRCentral::GrammarLoad
typedef enum {
   SRGRMFMT_CFG = 0x0000,
   SRGRMFMT_LIMITEDDOMAIN = 0x0001,
   SRGRMFMT_DICTATION = 0x0002,
   SRGRMFMT_CFGNATIVE = 0x8000,
   SRGRMFMT_LIMITEDDOMAINNATIVE = 0x8001,
   SRGRMFMT_DICTATIONNATIVE = 0x8002,
   SRGRMFMT_DRAGONNATIVE1 = 0x8101,
   SRGRMFMT_DRAGONNATIVE2 = 0x8102,
   SRGRMFMT_DRAGONNATIVE3 = 0x8103
   } SRGRMFMT, * PSRGRMFMT;

// SRCKD_NGRAM
typedef struct {
	QWORD   dwTotalCounts;
	DWORD   dwNumWordsClasses;
	DWORD   dwWordClassNameOffset;
	DWORD   dwNumContextGroups;
	DWORD   dwContextGroupOffset;
	DWORD   dwNumClasses;
	DWORD   dwClassOffset;
	BYTE    bBitsPerWord;
	BYTE    abFiller[3];
	DWORD   adwProbability[256];
	} NGRAMHDR, * PNGRAMHDR;

typedef struct {
	DWORD   dwSize;
	QWORD   dwTotalCounts;
	DWORD   adwWordClassID[4];
	DWORD   dwNumSequential;
	DWORD   dwNumRandomAccess;
	BYTE    bBackOffProbToken;
	BYTE    abFiller[3];
	// WORD or DWORD awRandomAccessWordsClasses[dwNumRandomAccess];
	// BYTE abProbToken[dwNumSequential+dwNumRandomAccess];
	} NGRAMCGHDR, *PNGRAMCGHDR;


/************************************************************************
Class IDs */


// {E02D16C0-C743-11cd-80E5-00AA003E4B50}
DEFINE_GUID(CLSID_SREnumerator, 
0xe02d16c0, 0xc743, 0x11cd, 0x80, 0xe5, 0x0, 0xaa, 0x0, 0x3e, 0x4b, 0x50);


/************************************************************************
interfaces */


/*
 * ISRAttributes
 */

#undef   INTERFACE
#define  INTERFACE   ISRAttributesW

DEFINE_GUID(IID_ISRAttributesW, 0x68A33AA0L, 0x44CD, 0x101B, 0x90, 0xA8, 0x00, 0xAA, 0x00, 0x3E, 0x4B, 0x50);

DECLARE_INTERFACE_ (ISRAttributesW, IUnknown) {

   // IUnknown members
   STDMETHOD(QueryInterface)  (THIS_ REFIID riid, LPVOID FAR* ppvObj) PURE;
   STDMETHOD_(ULONG,AddRef)   (THIS) PURE;
   STDMETHOD_(ULONG,Release)  (THIS) PURE;

   // ISRAttributesW members
   STDMETHOD (AutoGainEnableGet) (THIS_ DWORD *) PURE;
   STDMETHOD (AutoGainEnableSet) (THIS_ DWORD) PURE;
   STDMETHOD (EchoGet)           (THIS_ BOOL *) PURE;
   STDMETHOD (EchoSet)           (THIS_ BOOL) PURE;
   STDMETHOD (EnergyFloorGet)    (THIS_ WORD *) PURE;
   STDMETHOD (EnergyFloorSet)    (THIS_ WORD) PURE;
   STDMETHOD (MicrophoneGet)     (THIS_ PWSTR, DWORD, DWORD *) PURE;
   STDMETHOD (MicrophoneSet)     (THIS_ PCWSTR) PURE;
   STDMETHOD (RealTimeGet)       (THIS_ DWORD *) PURE;
   STDMETHOD (RealTimeSet)       (THIS_ DWORD) PURE;
   STDMETHOD (SpeakerGet)        (THIS_ PWSTR, DWORD, DWORD *) PURE;
   STDMETHOD (SpeakerSet)        (THIS_ PCWSTR) PURE;
   STDMETHOD (TimeOutGet)        (THIS_ DWORD *, DWORD *) PURE;
   STDMETHOD (TimeOutSet)        (THIS_ DWORD, DWORD) PURE;
   STDMETHOD (ThresholdGet)      (THIS_ DWORD *) PURE;
   STDMETHOD (ThresholdSet)      (THIS_ DWORD) PURE;
   };

typedef ISRAttributesW FAR * PISRATTRIBUTESW;


#undef   INTERFACE
#define  INTERFACE   ISRAttributesA

DEFINE_GUID(IID_ISRAttributesA, 0x2F26B9C1L, 0xDB31, 0x11CD, 0xB3, 0xCA, 0x00, 0xAA, 0x00, 0x47, 0xBA, 0x4F);

DECLARE_INTERFACE_ (ISRAttributesA, IUnknown) {

   // IUnknown members
   STDMETHOD(QueryInterface)  (THIS_ REFIID riid, LPVOID FAR* ppvObj) PURE;
   STDMETHOD_(ULONG,AddRef)   (THIS) PURE;
   STDMETHOD_(ULONG,Release)  (THIS) PURE;

   // ISRAttributesA members
   STDMETHOD (AutoGainEnableGet) (THIS_ DWORD *) PURE;
   STDMETHOD (AutoGainEnableSet) (THIS_ DWORD) PURE;
   STDMETHOD (EchoGet)           (THIS_ BOOL *) PURE;
   STDMETHOD (EchoSet)           (THIS_ BOOL) PURE;
   STDMETHOD (EnergyFloorGet)    (THIS_ WORD *) PURE;
   STDMETHOD (EnergyFloorSet)    (THIS_ WORD) PURE;
   STDMETHOD (MicrophoneGet)     (THIS_ PSTR, DWORD, DWORD *) PURE;
   STDMETHOD (MicrophoneSet)     (THIS_ PCSTR) PURE;
   STDMETHOD (RealTimeGet)       (THIS_ DWORD *) PURE;
   STDMETHOD (RealTimeSet)       (THIS_ DWORD) PURE;
   STDMETHOD (SpeakerGet)        (THIS_ PSTR, DWORD, DWORD *) PURE;
   STDMETHOD (SpeakerSet)        (THIS_ PCSTR) PURE;
   STDMETHOD (TimeOutGet)        (THIS_ DWORD *, DWORD *) PURE;
   STDMETHOD (TimeOutSet)        (THIS_ DWORD, DWORD) PURE;
   STDMETHOD (ThresholdGet)      (THIS_ DWORD *) PURE;
   STDMETHOD (ThresholdSet)      (THIS_ DWORD) PURE;
   };

typedef ISRAttributesA FAR * PISRATTRIBUTESA;


#ifdef _S_UNICODE
 #define ISRAttributes        ISRAttributesW
 #define IID_ISRAttributes    IID_ISRAttributesW
 #define PISRATTRIBUTES       PISRATTRIBUTESW

#else
 #define ISRAttributes        ISRAttributesA
 #define IID_ISRAttributes    IID_ISRAttributesA
 #define PISRATTRIBUTES       PISRATTRIBUTESA

#endif // _S_UNICODE

/*
 * ISRCentral
 */

#undef   INTERFACE
#define  INTERFACE   ISRCentralW

DEFINE_GUID(IID_ISRCentralW, 0xB9BD3860L, 0x44DB, 0x101B, 0x90, 0xA8, 0x00, 0xAA, 0x00, 0x3E, 0x4B, 0x50);

DECLARE_INTERFACE_ (ISRCentralW, IUnknown) {

   // IUnknown members
   STDMETHOD(QueryInterface)  (THIS_ REFIID riid, LPVOID FAR* ppvObj) PURE;
   STDMETHOD_(ULONG,AddRef)   (THIS) PURE;
   STDMETHOD_(ULONG,Release)  (THIS) PURE;

   // ISRCentralW members
   STDMETHOD (ModeGet)        (THIS_ PSRMODEINFOW) PURE;
   STDMETHOD (GrammarLoad)    (THIS_ SRGRMFMT, SDATA, PVOID, IID, LPUNKNOWN *) PURE;
   STDMETHOD (Pause)          (THIS) PURE;
   STDMETHOD (PosnGet)        (THIS_ PQWORD) PURE;
   STDMETHOD (Resume)         (THIS) PURE;
   STDMETHOD (ToFileTime)     (THIS_ PQWORD, FILETIME *) PURE;
   STDMETHOD (Register)       (THIS_ PVOID, IID, DWORD*) PURE;
   STDMETHOD (UnRegister)     (THIS_ DWORD) PURE;
   };

typedef ISRCentralW FAR * PISRCENTRALW;


#undef   INTERFACE
#define  INTERFACE   ISRCentralA

DEFINE_GUID(IID_ISRCentralA, 0x2F26B9C2L, 0xDB31, 0x11CD, 0xB3, 0xCA, 0x00, 0xAA, 0x00, 0x47, 0xBA, 0x4F);

DECLARE_INTERFACE_ (ISRCentralA, IUnknown) {

   // IUnknown members
   STDMETHOD(QueryInterface)  (THIS_ REFIID riid, LPVOID FAR* ppvObj) PURE;
   STDMETHOD_(ULONG,AddRef)   (THIS) PURE;
   STDMETHOD_(ULONG,Release)  (THIS) PURE;

   // ISRCentralA members
   STDMETHOD (ModeGet)        (THIS_ PSRMODEINFOA) PURE;
   STDMETHOD (GrammarLoad)    (THIS_ SRGRMFMT, SDATA, PVOID, IID, LPUNKNOWN *) PURE;
   STDMETHOD (Pause)          (THIS) PURE;
   STDMETHOD (PosnGet)        (THIS_ PQWORD) PURE;
   STDMETHOD (Resume)         (THIS) PURE;
   STDMETHOD (ToFileTime)     (THIS_ PQWORD, FILETIME *) PURE;
   STDMETHOD (Register)       (THIS_ PVOID, IID, DWORD*) PURE;
   STDMETHOD (UnRegister)     (THIS_ DWORD) PURE;
   };

typedef ISRCentralA FAR * PISRCENTRALA;


#ifdef _S_UNICODE
 #define ISRCentral           ISRCentralW
 #define IID_ISRCentral       IID_ISRCentralW
 #define PISRCENTRAL          PISRCENTRALW

#else
 #define ISRCentral           ISRCentralA
 #define IID_ISRCentral       IID_ISRCentralA
 #define PISRCENTRAL          PISRCENTRALA

#endif   // _S_UNICODE



/*
 * ISRDialogs
 */

#undef   INTERFACE
#define  INTERFACE   ISRDialogsW

DEFINE_GUID(IID_ISRDialogsW, 0xBCFB4C60L, 0x44DB, 0x101B, 0x90, 0xA8, 0x00, 0xAA, 0x00, 0x3E, 0x4B, 0x50);

DECLARE_INTERFACE_ (ISRDialogsW, IUnknown) {

   // IUnknown members
   STDMETHOD(QueryInterface)  (THIS_ REFIID riid, LPVOID FAR* ppvObj) PURE;
   STDMETHOD_(ULONG,AddRef)   (THIS) PURE;
   STDMETHOD_(ULONG,Release)  (THIS) PURE;

   // ISRDialogsW members
   STDMETHOD (AboutDlg)       (THIS_ HWND, PCWSTR) PURE;
   STDMETHOD (GeneralDlg)     (THIS_ HWND, PCWSTR) PURE;
   STDMETHOD (LexiconDlg)     (THIS_ HWND, PCWSTR) PURE;
   STDMETHOD (TrainMicDlg)    (THIS_ HWND, PCWSTR) PURE;
   STDMETHOD (TrainGeneralDlg)(THIS_ HWND, PCWSTR) PURE;
   };

typedef ISRDialogsW FAR * PISRDIALOGSW;


#undef   INTERFACE
#define  INTERFACE   ISRDialogsA

DEFINE_GUID(IID_ISRDialogsA, 0x05EB6C60L, 0xDBAB, 0x11CD, 0xB3, 0xCA, 0x00, 0xAA, 0x00, 0x47, 0xBA, 0x4F);

DECLARE_INTERFACE_ (ISRDialogsA, IUnknown) {

   // IUnknown members
   STDMETHOD(QueryInterface)  (THIS_ REFIID riid, LPVOID FAR* ppvObj) PURE;
   STDMETHOD_(ULONG,AddRef)   (THIS) PURE;
   STDMETHOD_(ULONG,Release)  (THIS) PURE;

   // ISRDialogsA members
   STDMETHOD (AboutDlg)       (THIS_ HWND, PCSTR) PURE;
   STDMETHOD (GeneralDlg)     (THIS_ HWND, PCSTR) PURE;
   STDMETHOD (LexiconDlg)     (THIS_ HWND, PCSTR) PURE;
   STDMETHOD (TrainMicDlg)    (THIS_ HWND, PCSTR) PURE;
   STDMETHOD (TrainGeneralDlg)(THIS_ HWND, PCSTR) PURE;
   };

typedef ISRDialogsA FAR * PISRDIALOGSA;


#ifdef _S_UNICODE
 #define ISRDialogs        ISRDialogsW
 #define IID_ISRDialogs    IID_ISRDialogsW
 #define PISRDIALOGS       PISRDIALOGSW

#else
 #define ISRDialogs        ISRDialogsA
 #define IID_ISRDialogs    IID_ISRDialogsA
 #define PISRDIALOGS       PISRDIALOGSA

#endif


/*
 * ISRDialogs2
 */

#undef   INTERFACE
#define  INTERFACE   ISRDialogs2W

// {9B445330-E39F-11d1-BED7-006008317CE8}
DEFINE_GUID(IID_ISRDialogs2W, 
0x9b445330, 0xe39f, 0x11d1, 0xbe, 0xd7, 0x0, 0x60, 0x8, 0x31, 0x7c, 0xe8);

DECLARE_INTERFACE_ (ISRDialogs2W, ISRDialogsW) {

   // IUnknown members
   STDMETHOD(QueryInterface)  (THIS_ REFIID riid, LPVOID FAR* ppvObj) PURE;
   STDMETHOD_(ULONG,AddRef)   (THIS) PURE;
   STDMETHOD_(ULONG,Release)  (THIS) PURE;

   // ISRDialogs2W members
   STDMETHOD (AboutDlg)       (THIS_ HWND, PCWSTR) PURE;
   STDMETHOD (GeneralDlg)     (THIS_ HWND, PCWSTR) PURE;
   STDMETHOD (LexiconDlg)     (THIS_ HWND, PCWSTR) PURE;
   STDMETHOD (TrainMicDlg)    (THIS_ HWND, PCWSTR) PURE;
   STDMETHOD (TrainGeneralDlg)(THIS_ HWND, PCWSTR) PURE;

   STDMETHOD (TrainPhrasesDlg)(THIS_ HWND, PCWSTR, PCWSTR) PURE;
   };

typedef ISRDialogs2W FAR * PISRDIALOGS2W;


#undef   INTERFACE
#define  INTERFACE   ISRDialogs2A

// {9B445331-E39F-11d1-BED7-006008317CE8}
DEFINE_GUID(IID_ISRDialogs2A, 
0x9b445331, 0xe39f, 0x11d1, 0xbe, 0xd7, 0x0, 0x60, 0x8, 0x31, 0x7c, 0xe8);

DECLARE_INTERFACE_ (ISRDialogs2A, ISRDialogsA) {

   // IUnknown members
   STDMETHOD(QueryInterface)  (THIS_ REFIID riid, LPVOID FAR* ppvObj) PURE;
   STDMETHOD_(ULONG,AddRef)   (THIS) PURE;
   STDMETHOD_(ULONG,Release)  (THIS) PURE;

   // ISRDialogs2A members
   STDMETHOD (AboutDlg)       (THIS_ HWND, PCSTR) PURE;
   STDMETHOD (GeneralDlg)     (THIS_ HWND, PCSTR) PURE;
   STDMETHOD (LexiconDlg)     (THIS_ HWND, PCSTR) PURE;
   STDMETHOD (TrainMicDlg)    (THIS_ HWND, PCSTR) PURE;
   STDMETHOD (TrainGeneralDlg)(THIS_ HWND, PCSTR) PURE;

   STDMETHOD (TrainPhrasesDlg)(THIS_ HWND, PCSTR, PCSTR) PURE;
   };

typedef ISRDialogs2A FAR * PISRDIALOGS2A;


#ifdef _S_UNICODE
 #define ISRDialogs2        ISRDialogs2W
 #define IID_ISRDialogs2    IID_ISRDialogs2W
 #define PISRDIALOGS2       PISRDIALOGS2W

#else
 #define ISRDialogs2        ISRDialogs2A
 #define IID_ISRDialogs2    IID_ISRDialogs2A
 #define PISRDIALOGS2       PISRDIALOGS2A

#endif


/*
 *  ISREnum
 */

#undef   INTERFACE
#define  INTERFACE   ISREnumW

DEFINE_GUID(IID_ISREnumW, 0xBFA9F1A0L, 0x44DB, 0x101B, 0x90, 0xA8, 0x00, 0xAA, 0x00, 0x3E, 0x4B, 0x50);

DECLARE_INTERFACE_ (ISREnumW, IUnknown) {
   // IUnknown members
   STDMETHOD(QueryInterface)  (THIS_ REFIID riid, LPVOID FAR* ppvObj) PURE;
   STDMETHOD_(ULONG,AddRef)   (THIS) PURE;
   STDMETHOD_(ULONG,Release)  (THIS) PURE;

   // ISREnumW members
   STDMETHOD (Next)           (THIS_ ULONG, PSRMODEINFOW, ULONG *) PURE;
   STDMETHOD (Skip)           (THIS_ ULONG) PURE;
   STDMETHOD (Reset)          (THIS) PURE;
   STDMETHOD (Clone)          (THIS_ ISREnumW * FAR *) PURE;
   STDMETHOD (Select)         (THIS_ GUID, PISRCENTRALW *, LPUNKNOWN) PURE;
   };

typedef ISREnumW FAR * PISRENUMW;


#undef   INTERFACE
#define  INTERFACE   ISREnumA

DEFINE_GUID(IID_ISREnumA, 0x05EB6C61L, 0xDBAB, 0x11CD, 0xB3, 0xCA, 0x00, 0xAA, 0x00, 0x47, 0xBA, 0x4F);

DECLARE_INTERFACE_ (ISREnumA, IUnknown) {

   // IUnknown members
   STDMETHOD(QueryInterface)  (THIS_ REFIID riid, LPVOID FAR* ppvObj) PURE;
   STDMETHOD_(ULONG,AddRef)   (THIS) PURE;
   STDMETHOD_(ULONG,Release)  (THIS) PURE;

   // ISREnumA members
   STDMETHOD (Next)           (THIS_ ULONG, PSRMODEINFOA, ULONG *) PURE;
   STDMETHOD (Skip)           (THIS_ ULONG) PURE;
   STDMETHOD (Reset)          (THIS) PURE;
   STDMETHOD (Clone)          (THIS_ ISREnumA * FAR *) PURE;
   STDMETHOD (Select)         (THIS_ GUID, PISRCENTRALA *, LPUNKNOWN) PURE;
   };

typedef ISREnumA FAR * PISRENUMA;


#ifdef _S_UNICODE
 #define ISREnum           ISREnumW
 #define IID_ISREnum       IID_ISREnumW
 #define PISRENUM          PISRENUMW

#else
 #define ISREnum           ISREnumA
 #define IID_ISREnum       IID_ISREnumA
 #define PISRENUM          PISRENUMA

#endif



/*
 * ISRFind
 */

#undef   INTERFACE
#define  INTERFACE   ISRFindW

DEFINE_GUID(IID_ISRFindW, 0xC2835060L, 0x44DB, 0x101B, 0x90, 0xA8, 0x00, 0xAA, 0x00, 0x3E, 0x4B, 0x50);

DECLARE_INTERFACE_ (ISRFindW, IUnknown) {

   // IUnknown members
   STDMETHOD(QueryInterface)  (THIS_ REFIID riid, LPVOID FAR* ppvObj) PURE;
   STDMETHOD_(ULONG,AddRef)   (THIS) PURE;
   STDMETHOD_(ULONG,Release)  (THIS) PURE;

   // ISRFindW members
   STDMETHOD (Find)           (THIS_ PSRMODEINFOW, PSRMODEINFORANK, PSRMODEINFOW) PURE;
   STDMETHOD (Select)         (THIS_ GUID, PISRCENTRALW *, LPUNKNOWN) PURE;
   };

typedef ISRFindW FAR * PISRFINDW;


#undef   INTERFACE
#define  INTERFACE   ISRFindA

DEFINE_GUID(IID_ISRFindA, 0x05EB6C62L, 0xDBAB, 0x11CD, 0xB3, 0xCA, 0x00, 0xAA, 0x00, 0x47, 0xBA, 0x4F);

DECLARE_INTERFACE_ (ISRFindA, IUnknown) {

   // IUnknown members
   STDMETHOD(QueryInterface)  (THIS_ REFIID riid, LPVOID FAR* ppvObj) PURE;
   STDMETHOD_(ULONG,AddRef)   (THIS) PURE;
   STDMETHOD_(ULONG,Release)  (THIS) PURE;

   // ISRFindA members
   STDMETHOD (Find)           (THIS_ PSRMODEINFOA, PSRMODEINFORANK, PSRMODEINFOA) PURE;
   STDMETHOD (Select)         (THIS_ GUID, PISRCENTRALA *, LPUNKNOWN) PURE;
   };

typedef ISRFindA FAR * PISRFINDA;


#ifdef _S_UNICODE
 #define ISRFind           ISRFindW
 #define IID_ISRFind       IID_ISRFindW
 #define PISRFIND          PISRFINDW

#else
 #define ISRFind           ISRFindA
 #define IID_ISRFind       IID_ISRFindA
 #define PISRFIND          PISRFINDA

#endif


/*
 * ISRGramLexPron
 */

#undef   INTERFACE
#define  INTERFACE   ISRGramLexPronW

// {7EB4B631-19A7-11d1-AF95-0000F81E880D}
DEFINE_GUID(IID_ISRGramLexPronW, 0x7eb4b631, 0x19a7, 0x11d1, 0xaf, 0x95, 0x0, 0x0, 0xf8, 0x1e, 0x88, 0xd);

DECLARE_INTERFACE_ (ISRGramLexPronW, IUnknown) {

   // IUnknown members
   STDMETHOD(QueryInterface)  (THIS_ REFIID riid, LPVOID FAR* ppvObj) PURE;
   STDMETHOD_(ULONG,AddRef)   (THIS) PURE;
   STDMETHOD_(ULONG,Release)  (THIS) PURE;

   // ISRGramLexPronW members
   STDMETHOD (Add)            (THIS_ VOICECHARSET, PCWSTR, PCWSTR, 
			       VOICEPARTOFSPEECH, PVOID, DWORD, BOOL) PURE;
   STDMETHOD (Get)            (THIS_ VOICECHARSET, PCWSTR, WORD, PWSTR, 
			       DWORD, DWORD *, VOICEPARTOFSPEECH *, PVOID, 
			       DWORD, DWORD *) PURE;
   STDMETHOD (Remove)         (THIS_ PCWSTR, WORD) PURE;
   STDMETHOD (LexiconDlg)     (THIS_ HWND, PCWSTR, PCWSTR) PURE;
   };

typedef ISRGramLexPronW FAR * PISRGRAMLEXPRONW;


#undef   INTERFACE
#define  INTERFACE   ISRGramLexPronA

// {7EB4B632-19A7-11d1-AF95-0000F81E880D}
DEFINE_GUID(IID_ISRGramLexPronA, 0x7eb4b632, 0x19a7, 0x11d1, 0xaf, 0x95, 0x0, 0x0, 0xf8, 0x1e, 0x88, 0xd);

DECLARE_INTERFACE_ (ISRGramLexPronA, IUnknown) {

   // IUnknown members
   STDMETHOD(QueryInterface)  (THIS_ REFIID riid, LPVOID FAR* ppvObj) PURE;
   STDMETHOD_(ULONG,AddRef)   (THIS) PURE;
   STDMETHOD_(ULONG,Release)  (THIS) PURE;

   // LexPronA members
   STDMETHOD (Add)            (THIS_ VOICECHARSET, PCSTR, PCSTR, 
			       VOICEPARTOFSPEECH, PVOID, DWORD, BOOL) PURE;
   STDMETHOD (Get)            (THIS_ VOICECHARSET, PCSTR, WORD, PSTR, 
			       DWORD, DWORD *, VOICEPARTOFSPEECH *, PVOID, 
			       DWORD, DWORD *) PURE;
   STDMETHOD (Remove)         (THIS_ PCSTR, WORD) PURE;
   STDMETHOD (LexiconDlg)     (THIS_ HWND, PCSTR, PCSTR) PURE;
   };

typedef ISRGramLexPronA FAR * PISRGRAMLEXPRONA;


#ifdef _S_UNICODE
 #define ISRGramLexPron        ISRGramLexPronW
 #define IID_ISRGramLexPron    IID_ISRGramLexPronW
 #define PISRGRAMLEXPRON       PISRGRAMLEXPRONW

#else
 #define ISRGramLexPron        ISRGramLexPronA
 #define IID_ISRGramLexPron    IID_ISRGramLexPronA
 #define PISRGRAMLEXPRON       PISRGRAMLEXPRONA

#endif   // _S_UNICODE


/*
 * ISRGramCommon
 */

#undef   INTERFACE
#define  INTERFACE   ISRGramCommonW

DEFINE_GUID(IID_ISRGramCommonW, 0xe8c3e160, 0xc743, 0x11cd, 0x80, 0xe5, 0x0, 0xaa, 0x0, 0x3e, 0x4b, 0x50);

DECLARE_INTERFACE_ (ISRGramCommonW, IUnknown) {

   // IUnknown members
   STDMETHOD(QueryInterface)    (THIS_ REFIID riid, LPVOID FAR* ppvObj) PURE;
   STDMETHOD_(ULONG,AddRef)     (THIS) PURE;
   STDMETHOD_(ULONG,Release)    (THIS) PURE;

   // ISRGramCommonW members
   STDMETHOD (Activate)         (THIS_ HWND, BOOL, PCWSTR) PURE;
   STDMETHOD (Archive)          (THIS_ BOOL, PVOID, DWORD, DWORD *) PURE;
   STDMETHOD (BookMark)         (THIS_ QWORD, DWORD) PURE;
   STDMETHOD (Deactivate)       (THIS_ PCWSTR) PURE;
   STDMETHOD (DeteriorationGet) (THIS_ DWORD *, DWORD *, DWORD *) PURE;
   STDMETHOD (DeteriorationSet) (THIS_ DWORD, DWORD, DWORD) PURE;
   STDMETHOD (TrainDlg)         (THIS_ HWND, PCWSTR) PURE;
   STDMETHOD (TrainPhrase)      (THIS_ DWORD, PSDATA) PURE;
   STDMETHOD (TrainQuery)       (THIS_ DWORD *) PURE;
   };

typedef ISRGramCommonW FAR * PISRGRAMCOMMONW;


#undef   INTERFACE
#define  INTERFACE   ISRGramCommonA

DEFINE_GUID(IID_ISRGramCommonA, 0x05EB6C63L, 0xDBAB, 0x11CD, 0xB3, 0xCA, 0x00, 0xAA, 0x00, 0x47, 0xBA, 0x4F);

DECLARE_INTERFACE_ (ISRGramCommonA, IUnknown) {

   // IUnknown members
   STDMETHOD(QueryInterface)    (THIS_ REFIID riid, LPVOID FAR* ppvObj) PURE;
   STDMETHOD_(ULONG,AddRef)     (THIS) PURE;
   STDMETHOD_(ULONG,Release)    (THIS) PURE;

   // ISRGramCommonA members
   STDMETHOD (Activate)         (THIS_ HWND, BOOL, PCSTR) PURE;
   STDMETHOD (Archive)          (THIS_ BOOL, PVOID, DWORD, DWORD *) PURE;
   STDMETHOD (BookMark)         (THIS_ QWORD, DWORD) PURE;
   STDMETHOD (Deactivate)       (THIS_ PCSTR) PURE;
   STDMETHOD (DeteriorationGet) (THIS_ DWORD *, DWORD *, DWORD *) PURE;
   STDMETHOD (DeteriorationSet) (THIS_ DWORD, DWORD, DWORD) PURE;
   STDMETHOD (TrainDlg)         (THIS_ HWND, PCSTR) PURE;
   STDMETHOD (TrainPhrase)      (THIS_ DWORD, PSDATA) PURE;
   STDMETHOD (TrainQuery)       (THIS_ DWORD *) PURE;
   };

typedef ISRGramCommonA FAR * PISRGRAMCOMMONA;


#ifdef _S_UNICODE
 #define ISRGramCommon        ISRGramCommonW
 #define IID_ISRGramCommon    IID_ISRGramCommonW
 #define PISRGRAMCOMMON       PISRGRAMCOMMONW

#else
 #define ISRGramCommon        ISRGramCommonA
 #define IID_ISRGramCommon    IID_ISRGramCommonA
 #define PISRGRAMCOMMON       PISRGRAMCOMMONA

#endif



/*
 * ISRGramCFG
 */

#undef   INTERFACE
#define  INTERFACE   ISRGramCFGW

DEFINE_GUID(IID_ISRGramCFGW, 0xecc0b180, 0xc743, 0x11cd, 0x80, 0xe5, 0x0, 0xaa, 0x0, 0x3e, 0x4b, 0x50);

DECLARE_INTERFACE_ (ISRGramCFGW, IUnknown) {

   // IUnknown members
   STDMETHOD(QueryInterface)  (THIS_ REFIID riid, LPVOID FAR* ppvObj) PURE;
   STDMETHOD_(ULONG,AddRef)   (THIS) PURE;
   STDMETHOD_(ULONG,Release)  (THIS) PURE;

   // ISRGramCFGW members
   STDMETHOD (LinkQuery)      (THIS_ PCWSTR, BOOL *) PURE;
   STDMETHOD (ListAppend)     (THIS_ PCWSTR, SDATA) PURE;
   STDMETHOD (ListGet)        (THIS_ PCWSTR, PSDATA) PURE;
   STDMETHOD (ListRemove)     (THIS_ PCWSTR, SDATA) PURE;
   STDMETHOD (ListSet)        (THIS_ PCWSTR, SDATA) PURE;
   STDMETHOD (ListQuery)      (THIS_ PCWSTR, BOOL *) PURE;
   };

typedef ISRGramCFGW FAR * PISRGRAMCFGW;


#undef   INTERFACE
#define  INTERFACE   ISRGramCFGA

DEFINE_GUID(IID_ISRGramCFGA, 0x05EB6C64L, 0xDBAB, 0x11CD, 0xB3, 0xCA, 0x00, 0xAA, 0x00, 0x47, 0xBA, 0x4F);

DECLARE_INTERFACE_ (ISRGramCFGA, IUnknown) {

   // IUnknown members
   STDMETHOD(QueryInterface)  (THIS_ REFIID riid, LPVOID FAR* ppvObj) PURE;
   STDMETHOD_(ULONG,AddRef)   (THIS) PURE;
   STDMETHOD_(ULONG,Release)  (THIS) PURE;

   // ISRGramCFGA members
   STDMETHOD (LinkQuery)      (THIS_ PCSTR, BOOL *) PURE;
   STDMETHOD (ListAppend)     (THIS_ PCSTR, SDATA) PURE;
   STDMETHOD (ListGet)        (THIS_ PCSTR, PSDATA) PURE;
   STDMETHOD (ListRemove)     (THIS_ PCSTR, SDATA) PURE;
   STDMETHOD (ListSet)        (THIS_ PCSTR, SDATA) PURE;
   STDMETHOD (ListQuery)      (THIS_ PCSTR, BOOL *) PURE;
   };

typedef ISRGramCFGA FAR * PISRGRAMCFGA;


#ifdef _S_UNICODE
 #define ISRGramCFG        ISRGramCFGW
 #define IID_ISRGramCFG    IID_ISRGramCFGW
 #define PISRGRAMCFG       PISRGRAMCFGW

#else
 #define ISRGramCFG        ISRGramCFGA
 #define IID_ISRGramCFG    IID_ISRGramCFGA
 #define PISRGRAMCFG       PISRGRAMCFGA

#endif



/* 
 * ISRGramDictation
 */

#undef   INTERFACE
#define  INTERFACE   ISRGramDictationW

DEFINE_GUID(IID_ISRGramDictationW, 0x090CD9A3, 0xDA1A, 0x11CD, 0xB3, 0xCA, 0x0, 0xAA, 0x0, 0x47, 0xBA, 0x4F);

DECLARE_INTERFACE_ (ISRGramDictationW, IUnknown) {

   // IUnknown members
   STDMETHOD (QueryInterface) (THIS_ REFIID riid, LPVOID FAR* ppvObj) PURE;
   STDMETHOD_(ULONG,AddRef)   (THIS) PURE;
   STDMETHOD_(ULONG,Release)  (THIS) PURE;

   // ISRGramDictationW members
   STDMETHOD (Context)        (THIS_ PCWSTR, PCWSTR) PURE;
   STDMETHOD (Hint)           (THIS_ PCWSTR) PURE;
   STDMETHOD (Words)          (THIS_ PCWSTR) PURE;
   };

typedef ISRGramDictationW FAR *PISRGRAMDICTATIONW;


#undef   INTERFACE
#define  INTERFACE   ISRGramDictationA

DEFINE_GUID(IID_ISRGramDictationA, 0x05EB6C65L, 0xDBAB, 0x11CD, 0xB3, 0xCA, 0x00, 0xAA, 0x00, 0x47, 0xBA, 0x4F);

DECLARE_INTERFACE_ (ISRGramDictationA, IUnknown) {

   // IUnknown members
   STDMETHOD (QueryInterface) (THIS_ REFIID riid, LPVOID FAR* ppvObj) PURE;
   STDMETHOD_(ULONG,AddRef)   (THIS) PURE;
   STDMETHOD_(ULONG,Release)  (THIS) PURE;

   // ISRGramDictationA members
   STDMETHOD (Context)        (THIS_ PCSTR, PCSTR) PURE;
   STDMETHOD (Hint)           (THIS_ PCSTR) PURE;
   STDMETHOD (Words)          (THIS_ PCSTR) PURE;
   };

typedef ISRGramDictationA FAR *PISRGRAMDICTATIONA;


#ifdef _S_UNICODE
 #define ISRGramDictation        ISRGramDictationW
 #define IID_ISRGramDictation    IID_ISRGramDictationW
 #define PISRGRAMDICTATION       PISRGRAMDICTATIONW

#else
 #define ISRGramDictation        ISRGramDictationA
 #define IID_ISRGramDictation    IID_ISRGramDictationA
 #define PISRGRAMDICTATION       PISRGRAMDICTATIONA

#endif



// ISRGramInsertionGUI
// This does not need an ANSI/UNICODE interface because no characters are passed
#undef   INTERFACE
#define  INTERFACE   ISRGramInsertionGUI

// {090CD9A4-DA1A-11CD-B3CA-00AA0047BA4F}
DEFINE_GUID(IID_ISRGramInsertionGUI,
0x090CD9A4, 0xDA1A, 0x11CD, 0xB3, 0xCA, 0x0, 0xAA, 0x0, 0x47, 0xBA, 0x4F);

DECLARE_INTERFACE_ (ISRGramInsertionGUI, IUnknown) {
   // IUnknown members
   STDMETHOD (QueryInterface) (THIS_ REFIID riid, LPVOID FAR* ppvObj) PURE;
   STDMETHOD_(ULONG,AddRef)   (THIS) PURE;
   STDMETHOD_(ULONG,Release)  (THIS) PURE;

   // ISRGramInsertionGUI members
   STDMETHOD (Hide)           (THIS) PURE;
   STDMETHOD (Move)           (THIS_ RECT) PURE;
   STDMETHOD (Show)           (THIS_ HWND) PURE;
   };

typedef ISRGramInsertionGUI FAR *PISRGRAMINSERTIONGUI;



/*
 * ISRGramNotifySink
 */

#undef   INTERFACE
#define  INTERFACE   ISRGramNotifySinkW

DEFINE_GUID(IID_ISRGramNotifySinkW,  0xf106bfa0, 0xc743, 0x11cd, 0x80, 0xe5, 0x0, 0xaa, 0x0, 0x3e, 0x4b, 0x50);

DECLARE_INTERFACE_ (ISRGramNotifySinkW, IUnknown) {
   // IUnknown members
   STDMETHOD(QueryInterface)  (THIS_ REFIID riid, LPVOID FAR* ppvObj) PURE;
   STDMETHOD_(ULONG,AddRef)   (THIS) PURE;
   STDMETHOD_(ULONG,Release)  (THIS) PURE;

   // ISRGramNotifySinkW members
   STDMETHOD (BookMark)       (THIS_ DWORD) PURE;
   STDMETHOD (Paused)         (THIS) PURE;
   STDMETHOD (PhraseFinish)   (THIS_ DWORD, QWORD, QWORD, PSRPHRASEW, LPUNKNOWN) PURE;
   STDMETHOD (PhraseHypothesis)(THIS_ DWORD, QWORD, QWORD, PSRPHRASEW, LPUNKNOWN) PURE;
   STDMETHOD (PhraseStart)    (THIS_ QWORD) PURE;
   STDMETHOD (ReEvaluate)     (THIS_ LPUNKNOWN) PURE;
   STDMETHOD (Training)       (THIS_ DWORD) PURE;
   STDMETHOD (UnArchive)      (THIS_ LPUNKNOWN) PURE;
   };

typedef ISRGramNotifySinkW FAR * PISRGRAMNOTIFYSINKW;


// ISRGramNotifySinkA
#undef   INTERFACE
#define  INTERFACE   ISRGramNotifySinkA

// {EFEEA350-CE5E-11cd-9D96-00AA002FC7C9}
DEFINE_GUID(IID_ISRGramNotifySinkA, 
0xefeea350, 0xce5e, 0x11cd, 0x9d, 0x96, 0x0, 0xaa, 0x0, 0x2f, 0xc7, 0xc9);

DECLARE_INTERFACE_ (ISRGramNotifySinkA, IUnknown) {
   // IUnknown members
   STDMETHOD(QueryInterface)  (THIS_ REFIID riid, LPVOID FAR* ppvObj) PURE;
   STDMETHOD_(ULONG,AddRef)   (THIS) PURE;
   STDMETHOD_(ULONG,Release)  (THIS) PURE;

   // ISRGramNotifySinkA members
   STDMETHOD (BookMark)       (THIS_ DWORD) PURE;
   STDMETHOD (Paused)         (THIS) PURE;
   STDMETHOD (PhraseFinish)   (THIS_ DWORD, QWORD, QWORD, PSRPHRASEA, LPUNKNOWN) PURE;
   STDMETHOD (PhraseHypothesis)(THIS_ DWORD, QWORD, QWORD, PSRPHRASEA, LPUNKNOWN) PURE;
   STDMETHOD (PhraseStart)    (THIS_ QWORD) PURE;
   STDMETHOD (ReEvaluate)     (THIS_ LPUNKNOWN) PURE;
   STDMETHOD (Training)       (THIS_ DWORD) PURE;
   STDMETHOD (UnArchive)      (THIS_ LPUNKNOWN) PURE;
   };

typedef ISRGramNotifySinkA FAR * PISRGRAMNOTIFYSINKA;


#ifdef _S_UNICODE
 #define ISRGramNotifySink       ISRGramNotifySinkW
 #define IID_ISRGramNotifySink   IID_ISRGramNotifySinkW
 #define PISRGRAMNOTIFYSINK      PISRGRAMNOTIFYSINKW

#else
 #define ISRGramNotifySink       ISRGramNotifySinkA
 #define IID_ISRGramNotifySink   IID_ISRGramNotifySinkA
 #define PISRGRAMNOTIFYSINK      PISRGRAMNOTIFYSINKA

#endif   // _S_UNICODE



// ISRNotifySink
// This does not need an ANSI/UNICODE interface because no characters are passed
#undef   INTERFACE
#define  INTERFACE   ISRNotifySink

DEFINE_GUID(IID_ISRNotifySink,
0x090CD9B0L, 0xDA1A, 0x11CD, 0xB3, 0xCA, 0x00, 0xAA, 0x00, 0x47, 0xBA, 0x4F);

DECLARE_INTERFACE_ (ISRNotifySink, IUnknown) {
   // IUnknown members
   STDMETHOD(QueryInterface)  (THIS_ REFIID riid, LPVOID FAR* ppvObj) PURE;
   STDMETHOD_(ULONG,AddRef)   (THIS) PURE;
   STDMETHOD_(ULONG,Release)  (THIS) PURE;

   // ISRNotifySink members
   STDMETHOD (AttribChanged)  (THIS_ DWORD) PURE;
   STDMETHOD (Interference)   (THIS_ QWORD, QWORD, DWORD) PURE;
   STDMETHOD (Sound)          (THIS_ QWORD, QWORD) PURE;
   STDMETHOD (UtteranceBegin) (THIS_ QWORD) PURE;
   STDMETHOD (UtteranceEnd)   (THIS_ QWORD, QWORD) PURE;
   STDMETHOD (VUMeter)        (THIS_ QWORD, WORD) PURE;
   };

typedef ISRNotifySink FAR *PISRNOTIFYSINK;

// Just in case anyone uses the wide/ansi versions
#define ISRNotifySinkW       ISRNotifySink
#define IID_ISRNotifySinkW   IID_ISRNotifySink
#define PISRNOTIFYSINKW      PISRNOTIFYSINK
#define ISRNotifySinkA       ISRNotifySink
#define IID_ISRNotifySinkA   IID_ISRNotifySink
#define PISRNOTIFYSINKA      PISRNOTIFYSINK


// ISRNotifySink2
// This does not need an ANSI/UNICODE interface because no characters are passed
#undef   INTERFACE
#define  INTERFACE   ISRNotifySink2

// {9B445332-E39F-11d1-BED7-006008317CE8}
DEFINE_GUID(IID_ISRNotifySink2, 
0x9b445332, 0xe39f, 0x11d1, 0xbe, 0xd7, 0x0, 0x60, 0x8, 0x31, 0x7c, 0xe8);

DECLARE_INTERFACE_ (ISRNotifySink2, ISRNotifySink) {
   // IUnknown members
   STDMETHOD(QueryInterface)  (THIS_ REFIID riid, LPVOID FAR* ppvObj) PURE;
   STDMETHOD_(ULONG,AddRef)   (THIS) PURE;
   STDMETHOD_(ULONG,Release)  (THIS) PURE;

   // ISRNotifySink2 members
   STDMETHOD (AttribChanged)  (THIS_ DWORD) PURE;
   STDMETHOD (Interference)   (THIS_ QWORD, QWORD, DWORD) PURE;
   STDMETHOD (Sound)          (THIS_ QWORD, QWORD) PURE;
   STDMETHOD (UtteranceBegin) (THIS_ QWORD) PURE;
   STDMETHOD (UtteranceEnd)   (THIS_ QWORD, QWORD) PURE;
   STDMETHOD (VUMeter)        (THIS_ QWORD, WORD) PURE;

   STDMETHOD (Training)       (THIS) PURE;
   STDMETHOD (Error)          (THIS_ LPUNKNOWN) PURE;
   STDMETHOD (Warning)        (THIS_ LPUNKNOWN) PURE;
   };

typedef ISRNotifySink2 FAR *PISRNOTIFYSINK2;


/*
 * ISRResBasic
 */

#undef   INTERFACE
#define  INTERFACE   ISRResBasicW

DEFINE_GUID(IID_ISRResBasicW, 0x090CD9A5, 0xDA1A, 0x11CD, 0xB3, 0xCA, 0x0, 0xAA, 0x0, 0x47, 0xBA, 0x4F);

DECLARE_INTERFACE_ (ISRResBasicW, IUnknown) {

   // IUnknown members
   STDMETHOD(QueryInterface)  (THIS_ REFIID riid, LPVOID FAR* ppvObj) PURE;
   STDMETHOD_(ULONG,AddRef)   (THIS) PURE;
   STDMETHOD_(ULONG,Release)  (THIS) PURE;

   // ISRResBasicW members
   STDMETHOD (PhraseGet)      (THIS_ DWORD, PSRPHRASEW, DWORD,  DWORD *) PURE;
   STDMETHOD (Identify)       (THIS_ GUID *) PURE;
   STDMETHOD (TimeGet)        (THIS_ PQWORD, PQWORD) PURE;
   STDMETHOD (FlagsGet)       (THIS_ DWORD, DWORD *) PURE;
   };

typedef ISRResBasicW FAR *PISRRESBASICW;


#undef   INTERFACE
#define  INTERFACE   ISRResBasicA

DEFINE_GUID(IID_ISRResBasicA, 0x05EB6C66L, 0xDBAB, 0x11CD, 0xB3, 0xCA, 0x00, 0xAA, 0x00, 0x47, 0xBA, 0x4F);

DECLARE_INTERFACE_ (ISRResBasicA, IUnknown) {

   // IUnknown members
   STDMETHOD(QueryInterface)  (THIS_ REFIID riid, LPVOID FAR* ppvObj) PURE;
   STDMETHOD_(ULONG,AddRef)   (THIS) PURE;
   STDMETHOD_(ULONG,Release)  (THIS) PURE;

   // ISRResBasicA members
   STDMETHOD (PhraseGet)      (THIS_ DWORD, PSRPHRASEA, DWORD,  DWORD *) PURE;
   STDMETHOD (Identify)       (THIS_ GUID *) PURE;
   STDMETHOD (TimeGet)        (THIS_ PQWORD, PQWORD) PURE;
   STDMETHOD (FlagsGet)       (THIS_ DWORD, DWORD *) PURE;
   };

typedef ISRResBasicA FAR *PISRRESBASICA;


#ifdef _S_UNICODE
 #define ISRResBasic             ISRResBasicW
 #define IID_ISRResBasic         IID_ISRResBasicW
 #define PISRRESBASIC            PISRRESBASICW

#else
 #define ISRResBasic             ISRResBasicA
 #define IID_ISRResBasic         IID_ISRResBasicA
 #define PISRRESBASIC            PISRRESBASICA

#endif   // _S_UNICODE


/*
 * ISRResScore
 * This does not need an ANSI/UNICODE interface because no characters are passed
 */

#undef INTERFACE
#define INTERFACE       ISRResScores


// {0B37F1E0-B8DE-11cf-B22E-00AA00A215ED}
DEFINE_GUID(IID_ISRResScores, 0xb37f1e0, 0xb8de, 0x11cf, 0xb2, 0x2e, 0x0, 0xaa, 0x0, 0xa2, 0x15, 0xed);

DECLARE_INTERFACE_ (ISRResScores, IUnknown) {

	// IUnknown members
	STDMETHOD (QueryInterface)      (THIS_ REFIID riid, LPVOID FAR* ppvObj) PURE;
	STDMETHOD_(ULONG,AddRef)   (THIS) PURE;
	STDMETHOD_(ULONG,Release)  (THIS) PURE;

	// ISRResScores members
	STDMETHOD (GetPhraseScore) (THIS_ DWORD, long FAR *) PURE;
	STDMETHOD (GetWordScores)  (THIS_ DWORD, long FAR *, DWORD, LPDWORD) PURE;
};

typedef ISRResScores FAR* PISRRESSCORES;

// In case someone uses the A/W versions...

#define ISRResScoresW           ISRResScores
#define IID_ISRResScoresW       IID_ISRResScores
#define PISRRESSCORESW          PISRRESSCORES
#define ISRResScoresA           ISRResScores
#define IID_ISRResScoresA       IID_ISRResScores
#define PISRRESSCORESA          PISRRESSCORES



/*
 * ISRResMerge
 * This does not need an ANSI/UNICODE interface because no characters are passed
 */

#undef   INTERFACE
#define  INTERFACE   ISRResMerge

DEFINE_GUID(IID_ISRResMerge, 0x090CD9A6, 0xDA1A, 0x11CD, 0xB3, 0xCA, 0x0, 0xAA, 0x0, 0x47, 0xBA, 0x4F);

DECLARE_INTERFACE_ (ISRResMerge, IUnknown) {

   // IUnknown members
   STDMETHOD (QueryInterface) (THIS_ REFIID riid, LPVOID FAR* ppvObj) PURE;
   STDMETHOD_(ULONG,AddRef)   (THIS) PURE;
   STDMETHOD_(ULONG,Release)  (THIS) PURE;

   // ISRResMerge members
   STDMETHOD (Merge)          (THIS_ LPUNKNOWN, PIUNKNOWN ) PURE;
   STDMETHOD (Split)          (THIS_ QWORD, PIUNKNOWN , PIUNKNOWN ) PURE;
   };

typedef ISRResMerge FAR *PISRRESMERGE;



/*
 * ISRResAudio
 * This does not need an ANSI/UNICODE interface because no characters are passed
 */

#undef   INTERFACE
#define  INTERFACE   ISRResAudio

DEFINE_GUID(IID_ISRResAudio, 0x090CD9A7, 0xDA1A, 0x11CD, 0xB3, 0xCA, 0x0, 0xAA, 0x0, 0x47, 0xBA, 0x4F);

DECLARE_INTERFACE_ (ISRResAudio, IUnknown) {

   // IUnknown members
   STDMETHOD (QueryInterface) (THIS_ REFIID riid, LPVOID FAR* ppvObj) PURE;
   STDMETHOD_(ULONG,AddRef)   (THIS) PURE;
   STDMETHOD_(ULONG,Release)  (THIS) PURE;

   // ISRResAudio members
   STDMETHOD (GetWAV)         (THIS_ PSDATA) PURE;
   };

typedef ISRResAudio FAR *PISRRESAUDIO;



/*
 * ISRResAudioEx
 * This does not need an ANSI/UNICODE interface because no characters are passed
 */

#undef   INTERFACE
#define  INTERFACE   ISRResAudioEx

DEFINE_GUID(IID_ISRResAudioEx, 0xaad74c30, 0x4b02, 0x11d1, 0xb7, 0x92, 0x0, 0x60, 0x97, 0x98, 0x41, 0xb4);

DECLARE_INTERFACE_ (ISRResAudioEx, IUnknown) {

   // IUnknown members
   STDMETHOD (QueryInterface) (THIS_ REFIID riid, LPVOID FAR* ppvObj) PURE;
   STDMETHOD_(ULONG,AddRef)   (THIS) PURE;
   STDMETHOD_(ULONG,Release)  (THIS) PURE;

   // ISRResAudioEx members
   STDMETHOD (GetWAV)         (THIS_ PSDATA, QWORD, QWORD) PURE;
   };

typedef ISRResAudioEx FAR *PISRRESAUDIOEX;



/*
 * ISRResCorrection
 */

#undef   INTERFACE
#define  INTERFACE   ISRResCorrectionW

DEFINE_GUID(IID_ISRResCorrectionW, 0x090CD9A8L, 0xDA1A, 0x11CD, 0xB3, 0xCA, 0x00, 0xAA, 0x00, 0x47, 0xBA, 0x4F);

DECLARE_INTERFACE_ (ISRResCorrectionW, IUnknown) {

   // IUnknown members
   STDMETHOD (QueryInterface) (THIS_ REFIID riid, LPVOID FAR* ppvObj) PURE;
   STDMETHOD_(ULONG,AddRef)   (THIS) PURE;
   STDMETHOD_(ULONG,Release)  (THIS) PURE;

   // ISRResCorrectionW members
   STDMETHOD (Correction)         (THIS_ PSRPHRASEW, WORD) PURE;
   STDMETHOD (Validate)           (THIS_ WORD) PURE;
   };

typedef ISRResCorrectionW FAR *PISRRESCORRECTIONW;


#undef   INTERFACE
#define  INTERFACE   ISRResCorrectionA

DEFINE_GUID(IID_ISRResCorrectionA, 0x05EB6C67L, 0xDBAB, 0x11CD, 0xB3, 0xCA, 0x00, 0xAA, 0x00, 0x47, 0xBA, 0x4F);

DECLARE_INTERFACE_ (ISRResCorrectionA, IUnknown) {

   // IUnknown members
   STDMETHOD (QueryInterface) (THIS_ REFIID riid, LPVOID FAR* ppvObj) PURE;
   STDMETHOD_(ULONG,AddRef)   (THIS) PURE;
   STDMETHOD_(ULONG,Release)  (THIS) PURE;

   // ISRResCorrectionA members
   STDMETHOD (Correction)         (THIS_ PSRPHRASEA, WORD) PURE;
   STDMETHOD (Validate)           (THIS_ WORD) PURE;
   };

typedef ISRResCorrectionA FAR *PISRRESCORRECTIONA;


#ifdef _S_UNICODE
 #define ISRResCorrection        ISRResCorrectionW
 #define IID_ISRResCorrection    IID_ISRResCorrectionW
 #define PISRRESCORRECTION       PISRRESCORRECTIONW

#else
 #define ISRResCorrection        ISRResCorrectionA
 #define IID_ISRResCorrection    IID_ISRResCorrectionA
 #define PISRRESCORRECTION       PISRRESCORRECTIONA

#endif   // _S_UNICODE



// ISRResEval
// This does not need an ANSI/UNICODE interface because no characters are passed
#undef   INTERFACE
#define  INTERFACE   ISRResEval

// {90CD9A9-DA1A-11CD-B3CA-00AA0047BA4F}
DEFINE_GUID(IID_ISRResEval,
0x090CD9A9, 0xDA1A, 0x11CD, 0xB3, 0xCA, 0x0, 0xAA, 0x0, 0x47, 0xBA, 0x4F);

DECLARE_INTERFACE_ (ISRResEval, IUnknown) {
   // IUnknown members
   STDMETHOD (QueryInterface) (THIS_ REFIID riid, LPVOID FAR* ppvObj) PURE;
   STDMETHOD_(ULONG,AddRef)   (THIS) PURE;
   STDMETHOD_(ULONG,Release)  (THIS) PURE;

   //  SRResEval members
   STDMETHOD (ReEvaluate)     (THIS_ BOOL *) PURE;
   };

typedef ISRResEval FAR *PISRRESEVAL;



/*
 * ISRResGraph
 */

#undef   INTERFACE
#define  INTERFACE ISRResGraphW

DEFINE_GUID(IID_ISRResGraphW, 0x090CD9AA, 0xDA1A, 0x11CD, 0xB3, 0xCA, 0x0, 0xAA, 0x0, 0x47, 0xBA, 0x4F);

DECLARE_INTERFACE_ (ISRResGraphW, IUnknown) {

   // IUnknown members
   STDMETHOD (QueryInterface)     (THIS_ REFIID riid, LPVOID FAR* ppvObj) PURE;
   STDMETHOD_(ULONG,AddRef)       (THIS) PURE;
   STDMETHOD_(ULONG,Release)      (THIS) PURE;

   // ISRResGraphW members         
   STDMETHOD (BestPathPhoneme)    (THIS_ DWORD, DWORD *, DWORD, DWORD *) PURE;
   STDMETHOD (BestPathWord)       (THIS_ DWORD, DWORD *, DWORD, DWORD *) PURE;
   STDMETHOD (GetPhonemeNode)     (THIS_ DWORD, PSRRESPHONEMENODE, PWCHAR, 
				   PWCHAR) PURE;
   STDMETHOD (GetWordNode)        (THIS_ DWORD, PSRRESWORDNODE, PSRWORDW, DWORD, 
				   DWORD *) PURE;
   STDMETHOD (PathScorePhoneme)   (THIS_ DWORD *, DWORD, LONG *) PURE;
   STDMETHOD (PathScoreWord)      (THIS_ DWORD *, DWORD, LONG *) PURE;
   };

typedef ISRResGraphW FAR *PISRRESGRAPHW;


#undef   INTERFACE
#define  INTERFACE ISRResGraphA

DEFINE_GUID(IID_ISRResGraphA, 0x05EB6C68L, 0xDBAB, 0x11CD, 0xB3, 0xCA, 0x00, 0xAA, 0x00, 0x47, 0xBA, 0x4F);

DECLARE_INTERFACE_ (ISRResGraphA, IUnknown) {

   // IUnknown members
   STDMETHOD (QueryInterface)     (THIS_ REFIID riid, LPVOID FAR* ppvObj) PURE;
   STDMETHOD_(ULONG,AddRef)       (THIS) PURE;
   STDMETHOD_(ULONG,Release)      (THIS) PURE;

   // ISRResGraphA members         
   STDMETHOD (BestPathPhoneme)    (THIS_ DWORD, DWORD *, DWORD, DWORD *) PURE;
   STDMETHOD (BestPathWord)       (THIS_ DWORD, DWORD *, DWORD, DWORD *) PURE;
   STDMETHOD (GetPhonemeNode)     (THIS_ DWORD, PSRRESPHONEMENODE, PWCHAR, 
				   PCHAR) PURE;
   STDMETHOD (GetWordNode)        (THIS_ DWORD, PSRRESWORDNODE, PSRWORDA, DWORD, 
				   DWORD *) PURE;
   STDMETHOD (PathScorePhoneme)   (THIS_ DWORD *, DWORD, LONG *) PURE;
   STDMETHOD (PathScoreWord)      (THIS_ DWORD *, DWORD, LONG *) PURE;
   };

typedef ISRResGraphA FAR *PISRRESGRAPHA;


#ifdef _S_UNICODE
 #define ISRResGraph             ISRResGraphW
 #define IID_ISRResGraph         IID_ISRResGraphW
 #define PISRRESGRAPH            PISRRESGRAPHW

#else
 #define ISRResGraph             ISRResGraphA
 #define IID_ISRResGraph         IID_ISRResGraphA
 #define PISRRESGRAPH            PISRRESGRAPHA

#endif   // _S_UNICODE



/*
 * ISRResGraphEx
 */

#undef   INTERFACE
#define  INTERFACE ISRResGraphEx

DEFINE_GUID(IID_ISRResGraphEx, 0xbe8f63a0, 0xb915, 0x11d1, 0xb7, 0xbc, 0x0, 0x60, 0x97, 0x98, 0x41, 0xb4);

DECLARE_INTERFACE_ (ISRResGraphEx, IUnknown) {

   // IUnknown members
   STDMETHOD (QueryInterface)    (THIS_ REFIID riid, LPVOID FAR* ppvObj) PURE;
   STDMETHOD_(ULONG,AddRef)      (THIS) PURE;
   STDMETHOD_(ULONG,Release)     (THIS) PURE;

   // ISRResGraphEx members
   STDMETHOD (NodeStartGet)      (THIS_ DWORD *) PURE;
   STDMETHOD (NodeEndGet)        (THIS_ DWORD *) PURE;
   STDMETHOD (ArcEnum)           (THIS_ DWORD, DWORD *, DWORD, DWORD *, BOOL) PURE;
   STDMETHOD (NodeGet)           (THIS_ DWORD, DWORD *, BOOL) PURE;
   STDMETHOD (DWORDGet)          (THIS_ DWORD, GUID, DWORD *) PURE;
   STDMETHOD (DataGet)           (THIS_ DWORD, GUID, SDATA *) PURE;
   STDMETHOD (ScoreGet)          (THIS_ DWORD, DWORD *, DWORD, DWORD, DWORD, DWORD *) PURE;
   STDMETHOD (BestPathEnum)      (THIS_ DWORD, DWORD *, DWORD, DWORD *, DWORD, BOOL, SDATA *) PURE;
   STDMETHOD (GetAllArcs)        (THIS_ DWORD *, DWORD, DWORD *, DWORD *) PURE;
   STDMETHOD (GetAllNodes)       (THIS_ DWORD *, DWORD, DWORD *, DWORD *) PURE;
};

typedef ISRResGraphEx FAR *PISRRESGRAPHEX;



// ISRResMemory
// This does not need an ANSI/UNICODE interface because no characters are passed
#undef   INTERFACE
#define  INTERFACE   ISRResMemory

DEFINE_GUID(IID_ISRResMemory, 0x090CD9AB, 0xDA1A, 0x11CD, 0xB3, 0xCA, 0x0, 0xAA, 0x0, 0x47, 0xBA, 0x4F);

DECLARE_INTERFACE_ (ISRResMemory, IUnknown) {

   // IUnknown members
   STDMETHOD (QueryInterface) (THIS_ REFIID riid, LPVOID FAR* ppvObj) PURE;
   STDMETHOD_(ULONG,AddRef)   (THIS) PURE;
   STDMETHOD_(ULONG,Release)  (THIS) PURE;

   // ISRResMemory members
   STDMETHOD (Free)           (THIS_ DWORD) PURE;
   STDMETHOD (Get)            (THIS_ DWORD *, DWORD *) PURE;
   STDMETHOD (LockGet)        (THIS_ BOOL *) PURE;
   STDMETHOD (LockSet)        (THIS_ BOOL) PURE;
   };

typedef ISRResMemory FAR *PISRRESMEMORY;



// ISRResModifyGUI
// This does not need an ANSI/UNICODE interface because no characters are passed
#undef   INTERFACE
#define  INTERFACE   ISRResModifyGUI

DEFINE_GUID(IID_ISRResModifyGUI, 0x090CD9AC, 0xDA1A, 0x11CD, 0xB3, 0xCA, 0x0, 0xAA, 0x0, 0x47, 0xBA, 0x4F);

DECLARE_INTERFACE_ (ISRResModifyGUI, IUnknown) {

   // IUnknown members
   STDMETHOD (QueryInterface)    (THIS_ REFIID riid, LPVOID FAR* ppvObj) PURE;
   STDMETHOD_(ULONG,AddRef)      (THIS) PURE;
   STDMETHOD_(ULONG,Release)     (THIS) PURE;

   // ISRResModifyGUI members
   STDMETHOD (Hide)              (THIS) PURE;
   STDMETHOD (Move)              (THIS_ RECT *) PURE;
   STDMETHOD (Show)              (THIS_ HWND) PURE;
   };

typedef ISRResModifyGUI FAR *PISRRESMODIFYGUI;



/*
 * ISRResSpeakerW
 */

#undef   INTERFACE
#define  INTERFACE   ISRResSpeakerW

DEFINE_GUID(IID_ISRResSpeakerW, 0x090CD9AD, 0xDA1A, 0x11CD, 0xB3, 0xCA, 0x0, 0xAA, 0x0, 0x47, 0xBA, 0x4F);

DECLARE_INTERFACE_ (ISRResSpeakerW, IUnknown) {

   // IUnknown members
   STDMETHOD (QueryInterface)    (THIS_ REFIID riid, LPVOID FAR* ppvObj) PURE;
   STDMETHOD_(ULONG,AddRef)      (THIS) PURE;
   STDMETHOD_(ULONG,Release)     (THIS) PURE;

   // ISRResSpeakerW members
   STDMETHOD (Correction)        (THIS_ PCWSTR, WORD) PURE;
   STDMETHOD (Validate)          (THIS_ WORD) PURE;
   STDMETHOD (Identify)          (THIS_ DWORD, PWSTR, DWORD, DWORD *, 
				  LONG *) PURE;
   STDMETHOD (IdentifyForFree)   (THIS_ BOOL *) PURE;
   };

typedef ISRResSpeakerW FAR *PISRRESSPEAKERW;


#undef   INTERFACE
#define  INTERFACE   ISRResSpeakerA

DEFINE_GUID(IID_ISRResSpeakerA, 0x05EB6C69L, 0xDBAB, 0x11CD, 0xB3, 0xCA, 0x00, 0xAA, 0x00, 0x47, 0xBA, 0x4F);

DECLARE_INTERFACE_ (ISRResSpeakerA, IUnknown) {

   // IUnknown members
   STDMETHOD (QueryInterface)    (THIS_ REFIID riid, LPVOID FAR* ppvObj) PURE;
   STDMETHOD_(ULONG,AddRef)      (THIS) PURE;
   STDMETHOD_(ULONG,Release)     (THIS) PURE;

   // ISRResSpeakerA members
   STDMETHOD (Correction)        (THIS_ PCSTR, WORD) PURE;
   STDMETHOD (Validate)          (THIS_ WORD) PURE;
   STDMETHOD (Identify)          (THIS_ DWORD, PSTR, DWORD, DWORD *, 
				  LONG *) PURE;
   STDMETHOD (IdentifyForFree)   (THIS_ BOOL *) PURE;
   };

typedef ISRResSpeakerA FAR *PISRRESSPEAKERA;


#ifdef _S_UNICODE
 #define ISRResSpeaker           ISRResSpeakerW
 #define IID_ISRResSpeaker       IID_ISRResSpeakerW
 #define PISRRESSPEAKER          PISRRESSPEAKERW

#else
 #define ISRResSpeaker           ISRResSpeakerA
 #define IID_ISRResSpeaker       IID_ISRResSpeakerA
 #define PISRRESSPEAKER          PISRRESSPEAKERA

#endif   // _S_UNICODE



/*
 * ISRSpeaker
 */

#undef   INTERFACE
#define  INTERFACE   ISRSpeakerW

DEFINE_GUID(IID_ISRSpeakerW, 0x090CD9AE, 0xDA1A, 0x11CD, 0xB3, 0xCA, 0x0, 0xAA, 0x0, 0x47, 0xBA, 0x4F);

DECLARE_INTERFACE_ (ISRSpeakerW, IUnknown) {

   // IUnknown members
   STDMETHOD(QueryInterface)  (THIS_ REFIID riid, LPVOID FAR* ppvObj) PURE;
   STDMETHOD_(ULONG,AddRef)   (THIS) PURE;
   STDMETHOD_(ULONG,Release)  (THIS) PURE;

   // ISRSpeakerW members
   STDMETHOD (Delete)         (THIS_ PCWSTR) PURE;
   STDMETHOD (Enum)           (THIS_ PWSTR *, DWORD *) PURE;
   STDMETHOD (Merge)          (THIS_ PCWSTR, PVOID, DWORD) PURE;
   STDMETHOD (New)            (THIS_ PCWSTR) PURE;
   STDMETHOD (Query)          (THIS_ PWSTR, DWORD, DWORD *) PURE;
   STDMETHOD (Read)           (THIS_ PCWSTR, PVOID *, DWORD *) PURE;
   STDMETHOD (Revert)         (THIS_ PCWSTR) PURE;
   STDMETHOD (Select)         (THIS_ PCWSTR, BOOL) PURE;
   STDMETHOD (Write)          (THIS_ PCWSTR, PVOID, DWORD) PURE;
   };

typedef ISRSpeakerW FAR *PISRSPEAKERW;


#undef   INTERFACE
#define  INTERFACE   ISRSpeakerA

DEFINE_GUID(IID_ISRSpeakerA, 0x090CD9AF, 0xDA1A, 0x11CD, 0xB3, 0xCA, 0x0, 0xAA, 0x0, 0x47, 0xBA, 0x4F);

DECLARE_INTERFACE_ (ISRSpeakerA, IUnknown) {

   // IUnknown members
   STDMETHOD(QueryInterface)  (THIS_ REFIID riid, LPVOID FAR* ppvObj) PURE;
   STDMETHOD_(ULONG,AddRef)   (THIS) PURE;
   STDMETHOD_(ULONG,Release)  (THIS) PURE;

   // ISRSpeakerA members
   STDMETHOD (Delete)         (THIS_ PCSTR) PURE;
   STDMETHOD (Enum)           (THIS_ PSTR *, DWORD *) PURE;
   STDMETHOD (Merge)          (THIS_ PCSTR, PVOID, DWORD) PURE;
   STDMETHOD (New)            (THIS_ PCSTR) PURE;
   STDMETHOD (Query)          (THIS_ PSTR, DWORD, DWORD *) PURE;
   STDMETHOD (Read)           (THIS_ PCSTR, PVOID *, DWORD *) PURE;
   STDMETHOD (Revert)         (THIS_ PCSTR) PURE;
   STDMETHOD (Select)         (THIS_ PCSTR, BOOL) PURE;
   STDMETHOD (Write)          (THIS_ PCSTR, PVOID, DWORD) PURE;
   };

typedef ISRSpeakerA FAR *PISRSPEAKERA;


#ifdef _S_UNICODE
 #define ISRSpeaker              ISRSpeakerW
 #define IID_ISRSpeaker          IID_ISRSpeakerW
 #define PISRSPEAKER             PISRSPEAKERW

#else
 #define ISRSpeaker              ISRSpeakerA
 #define IID_ISRSpeaker          IID_ISRSpeakerA
 #define PISRSPEAKER             PISRSPEAKERA

#endif   // _S_UNICODE


/*
 * ISRSpeaker2
 */

#undef   INTERFACE
#define  INTERFACE   ISRSpeaker2W

// {9B445333-E39F-11d1-BED7-006008317CE8}
DEFINE_GUID(IID_ISRSpeaker2W, 
0x9b445333, 0xe39f, 0x11d1, 0xbe, 0xd7, 0x0, 0x60, 0x8, 0x31, 0x7c, 0xe8);

DECLARE_INTERFACE_ (ISRSpeaker2W, ISRSpeakerW) {

   // IUnknown members
   STDMETHOD(QueryInterface)  (THIS_ REFIID riid, LPVOID FAR* ppvObj) PURE;
   STDMETHOD_(ULONG,AddRef)   (THIS) PURE;
   STDMETHOD_(ULONG,Release)  (THIS) PURE;

   // ISRSpeaker2W members
   STDMETHOD (Delete)         (THIS_ PCWSTR) PURE;
   STDMETHOD (Enum)           (THIS_ PWSTR *, DWORD *) PURE;
   STDMETHOD (Merge)          (THIS_ PCWSTR, PVOID, DWORD) PURE;
   STDMETHOD (New)            (THIS_ PCWSTR) PURE;
   STDMETHOD (Query)          (THIS_ PWSTR, DWORD, DWORD *) PURE;
   STDMETHOD (Read)           (THIS_ PCWSTR, PVOID *, DWORD *) PURE;
   STDMETHOD (Revert)         (THIS_ PCWSTR) PURE;
   STDMETHOD (Select)         (THIS_ PCWSTR, BOOL) PURE;
   STDMETHOD (Write)          (THIS_ PCWSTR, PVOID, DWORD) PURE;

   STDMETHOD (Commit)         (THIS) PURE;
   STDMETHOD (Rename)         (THIS_ PCWSTR, PCWSTR) PURE;
   STDMETHOD (GetChangedInfo) (THIS_ BOOL *, FILETIME*) PURE;
   };

typedef ISRSpeaker2W FAR *PISRSPEAKER2W;


#undef   INTERFACE
#define  INTERFACE   ISRSpeaker2A

// {9B445334-E39F-11d1-BED7-006008317CE8}
DEFINE_GUID(IID_ISRSpeaker2A,
0x9b445334, 0xe39f, 0x11d1, 0xbe, 0xd7, 0x0, 0x60, 0x8, 0x31, 0x7c, 0xe8);

DECLARE_INTERFACE_ (ISRSpeaker2A, ISRSpeakerA) {

   // IUnknown members
   STDMETHOD(QueryInterface)  (THIS_ REFIID riid, LPVOID FAR* ppvObj) PURE;
   STDMETHOD_(ULONG,AddRef)   (THIS) PURE;
   STDMETHOD_(ULONG,Release)  (THIS) PURE;

   // ISRSpeaker2A members
   STDMETHOD (Delete)         (THIS_ PCSTR) PURE;
   STDMETHOD (Enum)           (THIS_ PSTR *, DWORD *) PURE;
   STDMETHOD (Merge)          (THIS_ PCSTR, PVOID, DWORD) PURE;
   STDMETHOD (New)            (THIS_ PCSTR) PURE;
   STDMETHOD (Query)          (THIS_ PSTR, DWORD, DWORD *) PURE;
   STDMETHOD (Read)           (THIS_ PCSTR, PVOID *, DWORD *) PURE;
   STDMETHOD (Revert)         (THIS_ PCSTR) PURE;
   STDMETHOD (Select)         (THIS_ PCSTR, BOOL) PURE;
   STDMETHOD (Write)          (THIS_ PCSTR, PVOID, DWORD) PURE;

   STDMETHOD (Commit)         (THIS) PURE;
   STDMETHOD (Rename)         (THIS_ PCSTR, PCSTR) PURE;
   STDMETHOD (GetChangedInfo) (THIS_ BOOL *, FILETIME*) PURE;
   };

typedef ISRSpeaker2A FAR *PISRSPEAKER2A;


#ifdef _S_UNICODE
 #define ISRSpeaker2              ISRSpeaker2W
 #define IID_ISRSpeaker2          IID_ISRSpeaker2W
 #define PISRSPEAKER2             PISRSPEAKER2W

#else
 #define ISRSpeaker2              ISRSpeaker2A
 #define IID_ISRSpeaker2          IID_ISRSpeaker2A
 #define PISRSPEAKER2             PISRSPEAKER2A

#endif   // _S_UNICODE


/************************************************************************
Low-Level text-to-speech API
*/


/************************************************************************
defines */

#define  TTSI_NAMELEN                   SVFN_LEN
#define  TTSI_STYLELEN                  SVFN_LEN

#define  GENDER_NEUTRAL                 (0)
#define  GENDER_FEMALE                  (1)
#define  GENDER_MALE                    (2)

#define  TTSFEATURE_ANYWORD             SETBIT(0)
#define  TTSFEATURE_VOLUME              SETBIT(1)
#define  TTSFEATURE_SPEED               SETBIT(2)
#define  TTSFEATURE_PITCH               SETBIT(3)
#define  TTSFEATURE_TAGGED              SETBIT(4)
#define  TTSFEATURE_IPAUNICODE          SETBIT(5)
#define  TTSFEATURE_VISUAL              SETBIT(6)
#define  TTSFEATURE_WORDPOSITION        SETBIT(7)
#define  TTSFEATURE_PCOPTIMIZED         SETBIT(8)
#define  TTSFEATURE_PHONEOPTIMIZED      SETBIT(9)
#define  TTSFEATURE_FIXEDAUDIO          SETBIT(10)
#define	TTSFEATURE_SINGLEINSTANCE      SETBIT(11)
#define	TTSFEATURE_THREADSAFE          SETBIT(12)
#define	TTSFEATURE_IPATEXTDATA         SETBIT(13)
#define	TTSFEATURE_PREFERRED           SETBIT(14)
#define	TTSFEATURE_TRANSPLANTED        SETBIT(15)
#define	TTSFEATURE_SAPI4               SETBIT(16)

#define  TTSI_ILEXPRONOUNCE             SETBIT(0)
#define  TTSI_ITTSATTRIBUTES            SETBIT(1)
#define  TTSI_ITTSCENTRAL               SETBIT(2)
#define  TTSI_ITTSDIALOGS               SETBIT(3)
#define  TTSI_ATTRIBUTES                SETBIT(4)
#define  TTSI_IATTRIBUTES                SETBIT(4)
#define	TTSI_ILEXPRONOUNCE2             SETBIT(5)

#define  TTSDATAFLAG_TAGGED             SETBIT(0)

#define   TTSBNS_ABORTED                   SETBIT(0)

// ITTSNotifySink
#define   TTSNSAC_REALTIME               0
#define   TTSNSAC_PITCH                  1
#define   TTSNSAC_SPEED                  2
#define   TTSNSAC_VOLUME                 3


#define   TTSNSHINT_QUESTION             SETBIT(0)
#define   TTSNSHINT_STATEMENT            SETBIT(1)
#define   TTSNSHINT_COMMAND              SETBIT(2)
#define   TTSNSHINT_EXCLAMATION          SETBIT(3)
#define   TTSNSHINT_EMPHASIS             SETBIT(4)


// Ages
#define  TTSAGE_BABY                   1
#define  TTSAGE_TODDLER                3
#define  TTSAGE_CHILD                  6
#define  TTSAGE_ADOLESCENT             14
#define  TTSAGE_ADULT                  30
#define  TTSAGE_ELDERLY                70

// Attribute minimums and maximums
#define  TTSATTR_MINPITCH              0
#define  TTSATTR_MAXPITCH              0xffff
#define  TTSATTR_MINREALTIME           0
#define  TTSATTR_MAXREALTIME           0xffffffff
#define  TTSATTR_MINSPEED              0
#define  TTSATTR_MAXSPEED              0xffffffff
#define  TTSATTR_MINVOLUME             0
#define  TTSATTR_MAXVOLUME             0xffffffff


/************************************************************************
typedefs */

typedef struct {
   BYTE     bMouthHeight;
   BYTE     bMouthWidth;
   BYTE     bMouthUpturn;
   BYTE     bJawOpen;
   BYTE     bTeethUpperVisible;
   BYTE     bTeethLowerVisible;
   BYTE     bTonguePosn;
   BYTE     bLipTension;
   } TTSMOUTH, *PTTSMOUTH;



typedef struct {
   GUID       gEngineID;
   WCHAR      szMfgName[TTSI_NAMELEN];
   WCHAR      szProductName[TTSI_NAMELEN];
   GUID       gModeID;
   WCHAR      szModeName[TTSI_NAMELEN];
   LANGUAGEW  language;
   WCHAR      szSpeaker[TTSI_NAMELEN];
   WCHAR      szStyle[TTSI_STYLELEN];
   WORD       wGender;
   WORD       wAge;
   DWORD      dwFeatures;
   DWORD      dwInterfaces;
   DWORD      dwEngineFeatures;
   } TTSMODEINFOW, *PTTSMODEINFOW;

typedef struct {
   GUID       gEngineID;
   CHAR       szMfgName[TTSI_NAMELEN];
   CHAR       szProductName[TTSI_NAMELEN];
   GUID       gModeID;
   CHAR       szModeName[TTSI_NAMELEN];
   LANGUAGEA  language;
   CHAR       szSpeaker[TTSI_NAMELEN];
   CHAR       szStyle[TTSI_STYLELEN];
   WORD       wGender;
   WORD       wAge;
   DWORD      dwFeatures;
   DWORD      dwInterfaces;
   DWORD      dwEngineFeatures;
   } TTSMODEINFOA, *PTTSMODEINFOA;

#ifdef _S_UNICODE
 #define TTSMODEINFO         TTSMODEINFOW
 #define PTTSMODEINFO        PTTSMODEINFOW

#else
 #define TTSMODEINFO         TTSMODEINFOA
 #define PTTSMODEINFO        PTTSMODEINFOA

#endif   // _S_UNICODE



typedef struct {
   DWORD      dwEngineID;
   DWORD      dwMfgName;
   DWORD      dwProductName;
   DWORD      dwModeID;
   DWORD      dwModeName;
   DWORD      dwLanguage;
   DWORD      dwDialect;
   DWORD      dwSpeaker;
   DWORD      dwStyle;
   DWORD      dwGender;
   DWORD      dwAge;
   DWORD      dwFeatures;
   DWORD      dwInterfaces;
   DWORD      dwEngineFeatures;
   } TTSMODEINFORANK, * PTTSMODEINFORANK;

/************************************************************************
Class IDs */
// {D67C0280-C743-11cd-80E5-00AA003E4B50}
DEFINE_GUID(CLSID_TTSEnumerator, 
0xd67c0280, 0xc743, 0x11cd, 0x80, 0xe5, 0x0, 0xaa, 0x0, 0x3e, 0x4b, 0x50);

/************************************************************************
interfaces */

// ITTSAttributes

#undef   INTERFACE
#define  INTERFACE   ITTSAttributesW

DEFINE_GUID(IID_ITTSAttributesW, 0x1287A280L, 0x4A47, 0x101B, 0x93, 0x1A, 0x00, 0xAA, 0x00, 0x47, 0xBA, 0x4F);

DECLARE_INTERFACE_ (ITTSAttributesW, IUnknown) {

// IUnknown members

   STDMETHOD(QueryInterface)  (THIS_ REFIID riid, LPVOID FAR* ppvObj) PURE;
   STDMETHOD_(ULONG,AddRef)   (THIS) PURE;
   STDMETHOD_(ULONG,Release)  (THIS) PURE;

// ITTSAttributes members

   STDMETHOD (PitchGet)       (THIS_ WORD *) PURE;
   STDMETHOD (PitchSet)       (THIS_ WORD) PURE;  
   STDMETHOD (RealTimeGet)    (THIS_ DWORD *) PURE;
   STDMETHOD (RealTimeSet)    (THIS_ DWORD) PURE;  
   STDMETHOD (SpeedGet)       (THIS_ DWORD *) PURE;
   STDMETHOD (SpeedSet)       (THIS_ DWORD) PURE;  
   STDMETHOD (VolumeGet)      (THIS_ DWORD *) PURE;
   STDMETHOD (VolumeSet)      (THIS_ DWORD) PURE;  
   };

typedef ITTSAttributesW FAR * PITTSATTRIBUTESW;


#undef   INTERFACE
#define  INTERFACE   ITTSAttributesA

DEFINE_GUID(IID_ITTSAttributesA,
0x0FD6E2A1L, 0xE77D, 0x11CD, 0xB3, 0xCA, 0x00, 0xAA, 0x00, 0x47, 0xBA, 0x4F);

DECLARE_INTERFACE_ (ITTSAttributesA, IUnknown) {

// IUnknown members

   STDMETHOD(QueryInterface)  (THIS_ REFIID riid, LPVOID FAR* ppvObj) PURE;
   STDMETHOD_(ULONG,AddRef)   (THIS) PURE;
   STDMETHOD_(ULONG,Release)  (THIS) PURE;

// ITTSAttributes members

   STDMETHOD (PitchGet)       (THIS_ WORD *) PURE;
   STDMETHOD (PitchSet)       (THIS_ WORD) PURE;  
   STDMETHOD (RealTimeGet)    (THIS_ DWORD *) PURE;
   STDMETHOD (RealTimeSet)    (THIS_ DWORD) PURE;  
   STDMETHOD (SpeedGet)       (THIS_ DWORD *) PURE;
   STDMETHOD (SpeedSet)       (THIS_ DWORD) PURE;  
   STDMETHOD (VolumeGet)      (THIS_ DWORD *) PURE;
   STDMETHOD (VolumeSet)      (THIS_ DWORD) PURE;  
   };

typedef ITTSAttributesA FAR * PITTSATTRIBUTESA;


#ifdef _S_UNICODE
 #define ITTSAttributes          ITTSAttributesW
 #define IID_ITTSAttributes      IID_ITTSAttributesW
 #define PITTSATTRIBUTES         PITTSATTRIBUTESW

#else
 #define ITTSAttributes          ITTSAttributesA
 #define IID_ITTSAttributes      IID_ITTSAttributesA
 #define PITTSATTRIBUTES         PITTSATTRIBUTESA

#endif   // _S_UNICODE



// ITTSBufNotifySink

#undef   INTERFACE
#define  INTERFACE   ITTSBufNotifySink

// {E4963D40-C743-11cd-80E5-00AA003E4B50}
DEFINE_GUID(IID_ITTSBufNotifySink, 
0xe4963d40, 0xc743, 0x11cd, 0x80, 0xe5, 0x0, 0xaa, 0x0, 0x3e, 0x4b, 0x50);

DECLARE_INTERFACE_ (ITTSBufNotifySink, IUnknown) {

// IUnknown members

   STDMETHOD(QueryInterface)  (THIS_ REFIID riid, LPVOID FAR* ppvObj) PURE;
   STDMETHOD_(ULONG,AddRef)   (THIS) PURE;
   STDMETHOD_(ULONG,Release)  (THIS) PURE;

// ITTSBufNotifySink members

   STDMETHOD (TextDataDone)   (THIS_ QWORD, DWORD) PURE;
   STDMETHOD (TextDataStarted)(THIS_ QWORD) PURE;
   STDMETHOD (BookMark)       (THIS_ QWORD, DWORD) PURE;  
   STDMETHOD (WordPosition)   (THIS_ QWORD, DWORD) PURE;
   };

typedef ITTSBufNotifySink FAR * PITTSBUFNOTIFYSINK;

// In case anyone uses the W or A interface
#define ITTSBufNotifySinkW          ITTSBufNotifySink
#define IID_ITTSBufNotifySinkW      IID_ITTSBufNotifySink
#define PITTSBUFNOTIFYSINKW         PITTSBUFNOTIFYSINK
#define ITTSBufNotifySinkA          ITTSBufNotifySink
#define IID_ITTSBufNotifySinkA      IID_ITTSBufNotifySink
#define PITTSBUFNOTIFYSINKA         PITTSBUFNOTIFYSINK



/*
 * ITTSCentral
 */

#undef   INTERFACE
#define  INTERFACE   ITTSCentralW

DEFINE_GUID(IID_ITTSCentralW, 0x28016060L, 0x4A47, 0x101B, 0x93, 0x1A, 0x00, 0xAA, 0x00, 0x47, 0xBA, 0x4F);

DECLARE_INTERFACE_ (ITTSCentralW, IUnknown) {

   // IUnknown members
   STDMETHOD (QueryInterface) (THIS_ REFIID riid, LPVOID FAR* ppvObj) PURE;
   STDMETHOD_(ULONG,AddRef)   (THIS) PURE;
   STDMETHOD_(ULONG,Release)  (THIS) PURE;

   // ITTSCentralW members
   STDMETHOD (Inject)         (THIS_ PCWSTR) PURE;
   STDMETHOD (ModeGet)        (THIS_ PTTSMODEINFOW) PURE;
   STDMETHOD (Phoneme)        (THIS_ VOICECHARSET, DWORD, SDATA, PSDATA) PURE;
   STDMETHOD (PosnGet)        (THIS_ PQWORD) PURE;
   STDMETHOD (TextData)       (THIS_ VOICECHARSET, DWORD, SDATA, PVOID, IID) PURE;
   STDMETHOD (ToFileTime)     (THIS_ PQWORD, FILETIME *) PURE;
   STDMETHOD (AudioPause)     (THIS) PURE;
   STDMETHOD (AudioResume)    (THIS) PURE;
   STDMETHOD (AudioReset)     (THIS) PURE;
   STDMETHOD (Register)       (THIS_ PVOID, IID, DWORD*) PURE;
   STDMETHOD (UnRegister)     (THIS_ DWORD) PURE;
   };

typedef ITTSCentralW FAR * PITTSCENTRALW;


#undef   INTERFACE
#define  INTERFACE   ITTSCentralA

DEFINE_GUID(IID_ITTSCentralA, 0x05EB6C6AL, 0xDBAB, 0x11CD, 0xB3, 0xCA, 0x00, 0xAA, 0x00, 0x47, 0xBA, 0x4F);

DECLARE_INTERFACE_ (ITTSCentralA, IUnknown) {

   // IUnknown members
   STDMETHOD (QueryInterface) (THIS_ REFIID riid, LPVOID FAR* ppvObj) PURE;
   STDMETHOD_(ULONG,AddRef)   (THIS) PURE;
   STDMETHOD_(ULONG,Release)  (THIS) PURE;

   // ITTSCentralA members
   STDMETHOD (Inject)         (THIS_ PCSTR) PURE;
   STDMETHOD (ModeGet)        (THIS_ PTTSMODEINFOA) PURE;
   STDMETHOD (Phoneme)        (THIS_ VOICECHARSET, DWORD, SDATA, PSDATA) PURE;
   STDMETHOD (PosnGet)        (THIS_ PQWORD) PURE;
   STDMETHOD (TextData)       (THIS_ VOICECHARSET, DWORD, SDATA, PVOID, IID) PURE;
   STDMETHOD (ToFileTime)     (THIS_ PQWORD, FILETIME *) PURE;
   STDMETHOD (AudioPause)     (THIS) PURE;
   STDMETHOD (AudioResume)    (THIS) PURE;
   STDMETHOD (AudioReset)     (THIS) PURE;
   STDMETHOD (Register)       (THIS_ PVOID, IID, DWORD*) PURE;
   STDMETHOD (UnRegister)     (THIS_ DWORD) PURE;
   };

typedef ITTSCentralA FAR * PITTSCENTRALA;


#ifdef _S_UNICODE
 #define ITTSCentral             ITTSCentralW
 #define IID_ITTSCentral         IID_ITTSCentralW
 #define PITTSCENTRAL            PITTSCENTRALW

#else
 #define ITTSCentral             ITTSCentralA
 #define IID_ITTSCentral         IID_ITTSCentralA
 #define PITTSCENTRAL            PITTSCENTRALA

#endif   // _S_UNICODE



/*
 * ITTSDialogsW
 */

#undef   INTERFACE
#define  INTERFACE   ITTSDialogsW

DEFINE_GUID(IID_ITTSDialogsW, 0x47F59D00L, 0x4A47, 0x101B, 0x93, 0x1A, 0x00, 0xAA, 0x00, 0x47, 0xBA, 0x4F);

DECLARE_INTERFACE_ (ITTSDialogsW, IUnknown) {

   // IUnknown members
   STDMETHOD (QueryInterface) (THIS_ REFIID riid, LPVOID FAR* ppvObj) PURE;
   STDMETHOD_(ULONG,AddRef)   (THIS) PURE;
   STDMETHOD_(ULONG,Release)  (THIS) PURE;

   // ITTSDialogsW members
   STDMETHOD (AboutDlg)       (THIS_ HWND, PCWSTR) PURE;
   STDMETHOD (LexiconDlg)     (THIS_ HWND, PCWSTR) PURE;
   STDMETHOD (GeneralDlg)     (THIS_ HWND, PCWSTR) PURE;
   STDMETHOD (TranslateDlg)   (THIS_ HWND, PCWSTR) PURE;
   };

typedef ITTSDialogsW FAR * PITTSDIALOGSW;


#undef   INTERFACE
#define  INTERFACE   ITTSDialogsA

DEFINE_GUID(IID_ITTSDialogsA, 0x05EB6C6BL, 0xDBAB, 0x11CD, 0xB3, 0xCA, 0x00, 0xAA, 0x00, 0x47, 0xBA, 0x4F);

DECLARE_INTERFACE_ (ITTSDialogsA, IUnknown) {

   // IUnknown members
   STDMETHOD (QueryInterface) (THIS_ REFIID riid, LPVOID FAR* ppvObj) PURE;
   STDMETHOD_(ULONG,AddRef)   (THIS) PURE;
   STDMETHOD_(ULONG,Release)  (THIS) PURE;

   // ITTSDialogsA members
   STDMETHOD (AboutDlg)       (THIS_ HWND, PCSTR) PURE;
   STDMETHOD (LexiconDlg)     (THIS_ HWND, PCSTR) PURE;
   STDMETHOD (GeneralDlg)     (THIS_ HWND, PCSTR) PURE;
   STDMETHOD (TranslateDlg)   (THIS_ HWND, PCSTR) PURE;
   };

typedef ITTSDialogsA FAR * PITTSDIALOGSA;


#ifdef _S_UNICODE
 #define ITTSDialogs          ITTSDialogsW
 #define IID_ITTSDialogs      IID_ITTSDialogsW
 #define PITTSDIALOGS         PITTSDIALOGSW

#else
 #define ITTSDialogs          ITTSDialogsA
 #define IID_ITTSDialogs      IID_ITTSDialogsA
 #define PITTSDIALOGS         PITTSDIALOGSA

#endif



/*
 * ITTSEnum
 */

#undef   INTERFACE
#define  INTERFACE   ITTSEnumW

DEFINE_GUID(IID_ITTSEnumW, 0x6B837B20L, 0x4A47, 0x101B, 0x93, 0x1A, 0x00, 0xAA, 0x00, 0x47, 0xBA, 0x4F);

DECLARE_INTERFACE_ (ITTSEnumW, IUnknown) {

   // IUnknown members
   STDMETHOD (QueryInterface) (THIS_ REFIID riid, LPVOID FAR* ppvObj) PURE;
   STDMETHOD_(ULONG,AddRef)   (THIS) PURE;
   STDMETHOD_(ULONG,Release)  (THIS) PURE;

   // ITTSEnumW members
   STDMETHOD (Next)           (THIS_ ULONG, PTTSMODEINFOW, ULONG *) PURE;
   STDMETHOD (Skip)           (THIS_ ULONG) PURE;
   STDMETHOD (Reset)          (THIS) PURE;
   STDMETHOD (Clone)          (THIS_ ITTSEnumW * FAR *) PURE;
   STDMETHOD (Select)         (THIS_ GUID, PITTSCENTRALW *, LPUNKNOWN) PURE;
   };

typedef ITTSEnumW FAR * PITTSENUMW;


#undef   INTERFACE
#define  INTERFACE   ITTSEnumA

DEFINE_GUID(IID_ITTSEnumA, 0x05EB6C6DL, 0xDBAB, 0x11CD, 0xB3, 0xCA, 0x00, 0xAA, 0x00, 0x47, 0xBA, 0x4F);

DECLARE_INTERFACE_ (ITTSEnumA, IUnknown) {

   // IUnknown members
   STDMETHOD (QueryInterface) (THIS_ REFIID riid, LPVOID FAR* ppvObj) PURE;
   STDMETHOD_(ULONG,AddRef)   (THIS) PURE;
   STDMETHOD_(ULONG,Release)  (THIS) PURE;

   // ITTSEnumA members
   STDMETHOD (Next)           (THIS_ ULONG, PTTSMODEINFOA, ULONG *) PURE;
   STDMETHOD (Skip)           (THIS_ ULONG) PURE;
   STDMETHOD (Reset)          (THIS) PURE;
   STDMETHOD (Clone)          (THIS_ ITTSEnumA * FAR *) PURE;
   STDMETHOD (Select)         (THIS_ GUID, PITTSCENTRALA *, LPUNKNOWN) PURE;
   };

typedef ITTSEnumA FAR * PITTSENUMA;


#ifdef _S_UNICODE
 #define ITTSEnum             ITTSEnumW
 #define IID_ITTSEnum         IID_ITTSEnumW
 #define PITTSENUM            PITTSENUMW

#else
 #define ITTSEnum             ITTSEnumA
 #define IID_ITTSEnum         IID_ITTSEnumA
 #define PITTSENUM            PITTSENUMA

#endif



/*
 * ITTSFind
 */

#undef   INTERFACE
#define  INTERFACE   ITTSFindW

DEFINE_GUID(IID_ITTSFindW, 0x7AA42960L, 0x4A47, 0x101B, 0x93, 0x1A, 0x00, 0xAA, 0x00, 0x47, 0xBA, 0x4F);

DECLARE_INTERFACE_ (ITTSFindW, IUnknown) {

   // IUnknown members
   STDMETHOD(QueryInterface)  (THIS_ REFIID riid, LPVOID FAR* ppvObj) PURE;
   STDMETHOD_(ULONG,AddRef)   (THIS) PURE;
   STDMETHOD_(ULONG,Release)  (THIS) PURE;

   // ITTSFindW members
   STDMETHOD (Find)           (THIS_ PTTSMODEINFOW, PTTSMODEINFORANK, PTTSMODEINFOW) PURE;
   STDMETHOD (Select)         (THIS_ GUID, PITTSCENTRALW *, LPUNKNOWN) PURE;
   };

typedef ITTSFindW FAR * PITTSFINDW;


#undef   INTERFACE
#define  INTERFACE   ITTSFindA

DEFINE_GUID(IID_ITTSFindA, 0x05EB6C6EL, 0xDBAB, 0x11CD, 0xB3, 0xCA, 0x00, 0xAA, 0x00, 0x47, 0xBA, 0x4F);

DECLARE_INTERFACE_ (ITTSFindA, IUnknown) {

   // IUnknown members
   STDMETHOD(QueryInterface)  (THIS_ REFIID riid, LPVOID FAR* ppvObj) PURE;
   STDMETHOD_(ULONG,AddRef)   (THIS) PURE;
   STDMETHOD_(ULONG,Release)  (THIS) PURE;

   // ITTSFindA members
   STDMETHOD (Find)           (THIS_ PTTSMODEINFOA, PTTSMODEINFORANK, PTTSMODEINFOA) PURE;
   STDMETHOD (Select)         (THIS_ GUID, PITTSCENTRALA *, LPUNKNOWN) PURE;
   };

typedef ITTSFindA FAR * PITTSFINDA;


#ifdef _S_UNICODE
 #define ITTSFind             ITTSFindW
 #define IID_ITTSFind         IID_ITTSFindW
 #define PITTSFIND            PITTSFINDW

#else
 #define ITTSFind             ITTSFindA
 #define IID_ITTSFind         IID_ITTSFindA
 #define PITTSFIND            PITTSFINDA

#endif



/*
 * ITTSNotifySink
 */

#undef   INTERFACE
#define  INTERFACE   ITTSNotifySinkW

DEFINE_GUID(IID_ITTSNotifySinkW, 0xC0FA8F40L, 0x4A46, 0x101B, 0x93, 0x1A, 0x00, 0xAA, 0x00, 0x47, 0xBA, 0x4F);

DECLARE_INTERFACE_ (ITTSNotifySinkW, IUnknown) {

// IUnknown members

   STDMETHOD (QueryInterface) (THIS_ REFIID riid, LPVOID FAR* ppvObj) PURE;
   STDMETHOD_(ULONG,AddRef)   (THIS) PURE;
   STDMETHOD_(ULONG,Release)  (THIS) PURE;

// ITTSNotifySinkW members

   STDMETHOD (AttribChanged)  (THIS_ DWORD) PURE;
   STDMETHOD (AudioStart)     (THIS_ QWORD) PURE;
   STDMETHOD (AudioStop)      (THIS_ QWORD) PURE;
   STDMETHOD (Visual)         (THIS_ QWORD, WCHAR, WCHAR, DWORD, PTTSMOUTH) PURE;
   };

typedef ITTSNotifySinkW FAR * PITTSNOTIFYSINKW;


#undef   INTERFACE
#define  INTERFACE   ITTSNotifySinkA

DEFINE_GUID(IID_ITTSNotifySinkA, 0x05EB6C6FL, 0xDBAB, 0x11CD, 0xB3, 0xCA, 0x00, 0xAA, 0x00, 0x47, 0xBA, 0x4F);

DECLARE_INTERFACE_ (ITTSNotifySinkA, IUnknown) {

   // IUnknown members
   STDMETHOD (QueryInterface) (THIS_ REFIID riid, LPVOID FAR* ppvObj) PURE;
   STDMETHOD_(ULONG,AddRef)   (THIS) PURE;
   STDMETHOD_(ULONG,Release)  (THIS) PURE;

   // ITTSNotifySinkA members
   STDMETHOD (AttribChanged)  (THIS_ DWORD) PURE;
   STDMETHOD (AudioStart)     (THIS_ QWORD) PURE;
   STDMETHOD (AudioStop)      (THIS_ QWORD) PURE;
   STDMETHOD (Visual)         (THIS_ QWORD, CHAR, CHAR, DWORD, PTTSMOUTH) PURE;
   };

typedef ITTSNotifySinkA FAR * PITTSNOTIFYSINKA;


#ifdef _S_UNICODE
 #define ITTSNotifySink       ITTSNotifySinkW
 #define IID_ITTSNotifySink   IID_ITTSNotifySinkW
 #define PITTSNOTIFYSINK      PITTSNOTIFYSINKW

#else
 #define ITTSNotifySink       ITTSNotifySinkA
 #define IID_ITTSNotifySink   IID_ITTSNotifySinkA
 #define PITTSNOTIFYSINK      PITTSNOTIFYSINKA

#endif


/* ITTSNotifySink2 */

#undef   INTERFACE
#define  INTERFACE   ITTSNotifySink2W

// {599F77E1-E42E-11d1-BED8-006008317CE8}
DEFINE_GUID(IID_ITTSNotifySink2W, 
0x599f77e1, 0xe42e, 0x11d1, 0xbe, 0xd8, 0x0, 0x60, 0x8, 0x31, 0x7c, 0xe8);

DECLARE_INTERFACE_ (ITTSNotifySink2W, ITTSNotifySinkW) {

// IUnknown members

   STDMETHOD (QueryInterface) (THIS_ REFIID riid, LPVOID FAR* ppvObj) PURE;
   STDMETHOD_(ULONG,AddRef)   (THIS) PURE;
   STDMETHOD_(ULONG,Release)  (THIS) PURE;

// ITTSNotifySink2W members

   STDMETHOD (AttribChanged)  (THIS_ DWORD) PURE;
   STDMETHOD (AudioStart)     (THIS_ QWORD) PURE;
   STDMETHOD (AudioStop)      (THIS_ QWORD) PURE;
   STDMETHOD (Visual)         (THIS_ QWORD, WCHAR, WCHAR, DWORD, PTTSMOUTH) PURE;

   STDMETHOD (Error)          (THIS_ LPUNKNOWN) PURE;
   STDMETHOD (Warning)        (THIS_ LPUNKNOWN) PURE;
   STDMETHOD (VisualFuture)   (THIS_ DWORD, QWORD, WCHAR, WCHAR, DWORD, PTTSMOUTH) PURE;
   };

typedef ITTSNotifySink2W FAR * PITTSNOTIFYSINK2W;


#undef   INTERFACE
#define  INTERFACE   ITTSNotifySink2A

// {599F77E2-E42E-11d1-BED8-006008317CE8}
DEFINE_GUID(IID_ITTSNotifySink2A, 
0x599f77e2, 0xe42e, 0x11d1, 0xbe, 0xd8, 0x0, 0x60, 0x8, 0x31, 0x7c, 0xe8);

DECLARE_INTERFACE_ (ITTSNotifySink2A, ITTSNotifySinkA) {

   // IUnknown members
   STDMETHOD (QueryInterface) (THIS_ REFIID riid, LPVOID FAR* ppvObj) PURE;
   STDMETHOD_(ULONG,AddRef)   (THIS) PURE;
   STDMETHOD_(ULONG,Release)  (THIS) PURE;

   // ITTSNotifySink2A members
   STDMETHOD (AttribChanged)  (THIS_ DWORD) PURE;
   STDMETHOD (AudioStart)     (THIS_ QWORD) PURE;
   STDMETHOD (AudioStop)      (THIS_ QWORD) PURE;
   STDMETHOD (Visual)         (THIS_ QWORD, CHAR, CHAR, DWORD, PTTSMOUTH) PURE;

   STDMETHOD (Error)          (THIS_ LPUNKNOWN) PURE;
   STDMETHOD (Warning)        (THIS_ LPUNKNOWN) PURE;
   STDMETHOD (VisualFuture)   (THIS_ DWORD, QWORD, CHAR, CHAR, DWORD, PTTSMOUTH) PURE;
   };

typedef ITTSNotifySink2A FAR * PITTSNOTIFYSINK2A;


#ifdef _S_UNICODE
 #define ITTSNotifySink2       ITTSNotifySink2W
 #define IID_ITTSNotifySink2   IID_ITTSNotifySink2W
 #define PITTSNOTIFYSINK2      PITTSNOTIFYSINK2W

#else
 #define ITTSNotifySink2       ITTSNotifySink2A
 #define IID_ITTSNotifySink2   IID_ITTSNotifySink2A
 #define PITTSNOTIFYSINK2      PITTSNOTIFYSINK2A

#endif


/************************************************************************
High-Level command and control speech recognition API
*/

/************************************************************************
defines */


// VCMDNAME member lengths
#define VCMD_APPLEN             ((DWORD)32)
#define VCMD_STATELEN           VCMD_APPLEN
#define VCMD_MICLEN             VCMD_APPLEN
#define VCMD_SPEAKERLEN         VCMD_APPLEN

// dwFlags parameter of IVoiceCmd::MenuCreate
#define  VCMDMC_CREATE_TEMP     0x00000001
#define  VCMDMC_CREATE_NEW      0x00000002
#define  VCMDMC_CREATE_ALWAYS   0x00000004
#define  VCMDMC_OPEN_ALWAYS     0x00000008
#define  VCMDMC_OPEN_EXISTING   0x00000010

// dwFlags parameter of IVoiceCmd::Register
#define  VCMDRF_NOMESSAGES      0
#define  VCMDRF_ALLBUTVUMETER   0x00000001
#define  VCMDRF_VUMETER         0x00000002
#define  VCMDRF_ALLMESSAGES     (VCMDRF_ALLBUTVUMETER | VCMDRF_VUMETER)

// dwFlags parameter of IVoiceCmd::MenuEnum
#define  VCMDEF_DATABASE        0x00000000
#define  VCMDEF_ACTIVE          0x00000001
#define  VCMDEF_SELECTED        0x00000002
#define  VCMDEF_PERMANENT       0x00000004
#define  VCMDEF_TEMPORARY       0x00000008

// dwFlags parameter of IVCmdMenu::Activate
#define  VWGFLAG_ASLEEP         0x00000001

// wPriority parameter of IVCmdMenu::Activate
#define  VCMDACT_NORMAL         (0x8000)
#define  VCMDACT_LOW            (0x4000)
#define  VCMDACT_HIGH           (0xC000)

// dwFlags of the VCMDCOMMAND structure
#define  VCMDCMD_VERIFY         0x00000001
#define  VCMDCMD_DISABLED_TEMP  0x00000002
#define  VCMDCMD_DISABLED_PERM  0x00000004
#define  VCMDCMD_CANTRENAME     0x00000008

// parameter to any function that processes individual commands
#define  VCMD_BY_POSITION       0x00000001
#define  VCMD_BY_IDENTIFIER     0x00000002


// values for dwAttributes field of IVCmdNotifySink::AttribChanged
#define  IVCNSAC_AUTOGAINENABLE 0x00000001
#define  IVCNSAC_ENABLED        0x00000002
#define  IVCNSAC_AWAKE          0x00000004
#define  IVCNSAC_DEVICE         0x00000008
#define  IVCNSAC_MICROPHONE     0x00000010
#define  IVCNSAC_SPEAKER        0x00000020
#define  IVCNSAC_SRMODE         0x00000040
#define  IVCNSAC_THRESHOLD      0x00000080
#define  IVCNSAC_ORIGINAPP      0x00010000

// values for dwAttributes field of IVTxtNotifySink::AttribChanged
#define  IVTNSAC_DEVICE         0x00000001
#define  IVTNSAC_ENABLED        0x00000002
#define  IVTNSAC_SPEED          0x00000004
#define  IVTNSAC_VOLUME         0x00000008
#define  IVTNSAC_TTSMODE        0x00000010


// values used by IVXxxAttributes::SetMode to set the global speech
// recognition mode
#define  VSRMODE_OFF            0x00000002
#define  VSRMODE_DISABLED       0x00000001
#define  VSRMODE_CMDPAUSED      0x00000004
#define  VSRMODE_CMDONLY        0x00000010
#define  VSRMODE_DCTONLY        0x00000020
#define  VSRMODE_CMDANDDCT      0x00000040


/************************************************************************
typedefs */

// voice command structure - passed to command menu functions (IVCmdMenu)
typedef struct {
    DWORD   dwSize;         // size of struct including amount of abAction
    DWORD   dwFlags;
    DWORD   dwID;           // Command ID
    DWORD   dwCommand;      // DWORD aligned offset of command string
    DWORD   dwDescription;  // DWORD aligned offset of description string
    DWORD   dwCategory;     // DWORD aligned offset of category string
    DWORD   dwCommandText;  // DWORD aligned offset of command text string
    DWORD   dwAction;       // DWORD aligned offset of action data
    DWORD   dwActionSize;   // size of the action data (could be string or binary)
    BYTE    abData[1];      // command, description, category, and action data
			    // (action data is NOT interpreted by voice command)
} VCMDCOMMAND, * PVCMDCOMMAND;



// site information structure - possible parameter to IVoiceCmd::Register
typedef struct {
    DWORD   dwAutoGainEnable;
    DWORD   dwAwakeState;
    DWORD   dwThreshold;
    DWORD   dwDevice;
    DWORD   dwEnable;
    WCHAR   szMicrophone[VCMD_MICLEN];
    WCHAR   szSpeaker[VCMD_SPEAKERLEN];
    GUID    gModeID;
} VCSITEINFOW, *PVCSITEINFOW;

typedef struct {
    DWORD   dwAutoGainEnable;
    DWORD   dwAwakeState;
    DWORD   dwThreshold;
    DWORD   dwDevice;
    DWORD   dwEnable;
    CHAR    szMicrophone[VCMD_MICLEN];
    CHAR    szSpeaker[VCMD_SPEAKERLEN];
    GUID    gModeID;
} VCSITEINFOA, *PVCSITEINFOA;



// menu name structure
typedef struct {
    WCHAR   szApplication[VCMD_APPLEN]; // unique application name
    WCHAR   szState[VCMD_STATELEN];     // unique application state
} VCMDNAMEW, *PVCMDNAMEW;

typedef struct {
    CHAR    szApplication[VCMD_APPLEN]; // unique application name
    CHAR    szState[VCMD_STATELEN];     // unique application state
} VCMDNAMEA, *PVCMDNAMEA;



#ifdef  _S_UNICODE
 #define VCSITEINFO  VCSITEINFOW
 #define PVCSITEINFO PVCSITEINFOW
 #define VCMDNAME    VCMDNAMEW
 #define PVCMDNAME   PVCMDNAMEW
#else
 #define VCSITEINFO  VCSITEINFOA
 #define PVCSITEINFO PVCSITEINFOA
 #define VCMDNAME    VCMDNAMEA
 #define PVCMDNAME   PVCMDNAMEA
#endif  // _S_UNICODE

/************************************************************************
interfaces */

/*
 *  IVCmdNotifySink
 */
#undef   INTERFACE
#define  INTERFACE   IVCmdNotifySinkW

DEFINE_GUID(IID_IVCmdNotifySinkW, 0xCCFD7A60L, 0x604D, 0x101B, 0x99, 0x26, 0x00, 0xAA, 0x00, 0x3C, 0xFC, 0x2C);

DECLARE_INTERFACE_ (IVCmdNotifySinkW, IUnknown) {

   // IUnknown members
   STDMETHOD(QueryInterface)  (THIS_ REFIID riid, LPVOID FAR* ppvObj) PURE;
   STDMETHOD_(ULONG,AddRef)   (THIS) PURE;
   STDMETHOD_(ULONG,Release)  (THIS) PURE;

   // IVCmdNotifySink members

   STDMETHOD (CommandRecognize) (THIS_ DWORD, PVCMDNAMEW, DWORD, DWORD, PVOID, 
				 DWORD, PWSTR, PWSTR) PURE;
   STDMETHOD (CommandOther)     (THIS_ PVCMDNAMEW, PWSTR) PURE;
   STDMETHOD (CommandStart)     (THIS) PURE;
   STDMETHOD (MenuActivate)     (THIS_ PVCMDNAMEW, BOOL) PURE;
   STDMETHOD (UtteranceBegin)   (THIS) PURE;
   STDMETHOD (UtteranceEnd)     (THIS) PURE;
   STDMETHOD (VUMeter)          (THIS_ WORD) PURE;
   STDMETHOD (AttribChanged)    (THIS_ DWORD) PURE;
   STDMETHOD (Interference)     (THIS_ DWORD) PURE;
};

typedef IVCmdNotifySinkW FAR * PIVCMDNOTIFYSINKW;


#undef   INTERFACE
#define  INTERFACE   IVCmdNotifySinkA

// {80B25CC0-5540-11b9-C000-5611722E1D15}
DEFINE_GUID(IID_IVCmdNotifySinkA, 0x80b25cc0, 0x5540, 0x11b9, 0xc0, 0x0, 0x56, 0x11, 0x72, 0x2e, 0x1d, 0x15);

DECLARE_INTERFACE_ (IVCmdNotifySinkA, IUnknown) {

   // IUnknown members
   STDMETHOD(QueryInterface)  (THIS_ REFIID riid, LPVOID FAR* ppvObj) PURE;
   STDMETHOD_(ULONG,AddRef)   (THIS) PURE;
   STDMETHOD_(ULONG,Release)  (THIS) PURE;

   // IVCmdNotifySinkA members

   STDMETHOD (CommandRecognize) (THIS_ DWORD, PVCMDNAMEA, DWORD, DWORD, PVOID, 
				 DWORD, PSTR, PSTR) PURE;
   STDMETHOD (CommandOther)     (THIS_ PVCMDNAMEA, PSTR) PURE;
   STDMETHOD (CommandStart)     (THIS) PURE;
   STDMETHOD (MenuActivate)     (THIS_ PVCMDNAMEA, BOOL) PURE;
   STDMETHOD (UtteranceBegin)   (THIS) PURE;
   STDMETHOD (UtteranceEnd)     (THIS) PURE;
   STDMETHOD (VUMeter)          (THIS_ WORD) PURE;
   STDMETHOD (AttribChanged)    (THIS_ DWORD) PURE;
   STDMETHOD (Interference)     (THIS_ DWORD) PURE;
};

typedef IVCmdNotifySinkA FAR * PIVCMDNOTIFYSINKA;


#ifdef _S_UNICODE
 #define IVCmdNotifySink        IVCmdNotifySinkW
 #define IID_IVCmdNotifySink    IID_IVCmdNotifySinkW
 #define PIVCMDNOTIFYSINK       PIVCMDNOTIFYSINKW

#else
 #define IVCmdNotifySink        IVCmdNotifySinkA
 #define IID_IVCmdNotifySink    IID_IVCmdNotifySinkA
 #define PIVCMDNOTIFYSINK       PIVCMDNOTIFYSINKA

#endif // _S_UNICODE


/*
 *  IVCmdEnum
 */
#undef   INTERFACE
#define  INTERFACE   IVCmdEnumW

DEFINE_GUID(IID_IVCmdEnumW, 0xD3CC0820L, 0x604D, 0x101B, 0x99, 0x26, 0x00, 0xAA, 0x00, 0x3C, 0xFC, 0x2C);

DECLARE_INTERFACE_ (IVCmdEnumW, IUnknown) {
   // IUnknown members
   STDMETHOD(QueryInterface)  (THIS_ REFIID riid, LPVOID FAR* ppvObj) PURE;
   STDMETHOD_(ULONG,AddRef)   (THIS) PURE;
   STDMETHOD_(ULONG,Release)  (THIS) PURE;

   // IVCmdEnum members
   STDMETHOD (Next)           (THIS_ ULONG, PVCMDNAMEW, ULONG *) PURE;
   STDMETHOD (Skip)           (THIS_ ULONG) PURE;
   STDMETHOD (Reset)          (THIS) PURE;
   STDMETHOD (Clone)          (THIS_ IVCmdEnumW * FAR *) PURE;
};
typedef IVCmdEnumW FAR * PIVCMDENUMW;


#undef   INTERFACE
#define  INTERFACE   IVCmdEnumA

// {E86F9540-DCA2-11CD-A166-00AA004CD65C}
DEFINE_GUID(IID_IVCmdEnumA, 
0xE86F9540, 0xDCA2, 0x11CD, 0xA1, 0x66, 0x0, 0xAA, 0x0, 0x4C, 0xD6, 0x5C);

DECLARE_INTERFACE_ (IVCmdEnumA, IUnknown) {
   // IUnknown members
   STDMETHOD(QueryInterface)  (THIS_ REFIID riid, LPVOID FAR* ppvObj) PURE;
   STDMETHOD_(ULONG,AddRef)   (THIS) PURE;
   STDMETHOD_(ULONG,Release)  (THIS) PURE;

   // IVCmdEnum members
   STDMETHOD (Next)           (THIS_ ULONG, PVCMDNAMEA, ULONG *) PURE;
   STDMETHOD (Skip)           (THIS_ ULONG) PURE;
   STDMETHOD (Reset)          (THIS) PURE;
   STDMETHOD (Clone)          (THIS_ IVCmdEnumA * FAR *) PURE;
};
typedef IVCmdEnumA FAR * PIVCMDENUMA;


#ifdef _S_UNICODE
 #define IVCmdEnum              IVCmdEnumW
 #define IID_IVCmdEnum          IID_IVCmdEnumW
 #define PIVCMDENUM             PIVCMDENUMW

#else
 #define IVCmdEnum              IVCmdEnumA
 #define IID_IVCmdEnum          IID_IVCmdEnumA
 #define PIVCMDENUM             PIVCMDENUMA

#endif // _S_UNICODE


 
 
/*
 *  IEnumSRShare
 */
#undef   INTERFACE
#define  INTERFACE   IEnumSRShareW

// {E97F05C0-81B3-11ce-B763-00AA004CD65C}
DEFINE_GUID(IID_IEnumSRShareW,
0xe97f05c0, 0x81b3, 0x11ce, 0xb7, 0x63, 0x0, 0xaa, 0x0, 0x4c, 0xd6, 0x5c);

DECLARE_INTERFACE_ (IEnumSRShareW, IUnknown) {
   // IUnknown members
   STDMETHOD(QueryInterface)  (THIS_ REFIID riid, LPVOID FAR* ppvObj) PURE;
   STDMETHOD_(ULONG,AddRef)   (THIS) PURE;
   STDMETHOD_(ULONG,Release)  (THIS) PURE;

   // IEnumSRShare members
   STDMETHOD (Next)           (THIS_ ULONG, PSRSHAREW, ULONG *) PURE;
   STDMETHOD (Skip)           (THIS_ ULONG) PURE;
   STDMETHOD (Reset)          (THIS) PURE;
   STDMETHOD (Clone)          (THIS_ IEnumSRShareW * FAR *) PURE;
   STDMETHOD (New)            (THIS_ DWORD, GUID, PISRCENTRALW *, QWORD *) PURE;
   STDMETHOD (Share)          (THIS_ QWORD, PISRCENTRALW *) PURE;
   STDMETHOD (Detach)         (THIS_ QWORD) PURE;
};
typedef IEnumSRShareW FAR * PIENUMSRSHAREW;


#undef   INTERFACE
#define  INTERFACE   IEnumSRShareA

// {E97F05C1-81B3-11ce-B763-00AA004CD65C}
DEFINE_GUID(IID_IEnumSRShareA,
0xe97f05c1, 0x81b3, 0x11ce, 0xb7, 0x63, 0x0, 0xaa, 0x0, 0x4c, 0xd6, 0x5c);

DECLARE_INTERFACE_ (IEnumSRShareA, IUnknown) {
   // IUnknown members
   STDMETHOD(QueryInterface)  (THIS_ REFIID riid, LPVOID FAR* ppvObj) PURE;
   STDMETHOD_(ULONG,AddRef)   (THIS) PURE;
   STDMETHOD_(ULONG,Release)  (THIS) PURE;

   // IEnumSRShare members
   STDMETHOD (Next)           (THIS_ ULONG, PSRSHAREA, ULONG *) PURE;
   STDMETHOD (Skip)           (THIS_ ULONG) PURE;
   STDMETHOD (Reset)          (THIS) PURE;
   STDMETHOD (Clone)          (THIS_ IEnumSRShareA * FAR *) PURE;
   STDMETHOD (New)            (THIS_ DWORD, GUID, PISRCENTRALA *, QWORD *) PURE;
   STDMETHOD (Share)          (THIS_ QWORD, PISRCENTRALA *) PURE;
   STDMETHOD (Detach)         (THIS_ QWORD) PURE;
};
typedef IEnumSRShareA FAR * PIENUMSRSHAREA;


#ifdef _S_UNICODE
 #define IEnumSRShare              IEnumSRShareW
 #define IID_IEnumSRShare          IID_IEnumSRShareW
 #define PIENUMSRSHARE             PIENUMSRSHAREW

#else
 #define IEnumSRShare              IEnumSRShareA
 #define IID_IEnumSRShare          IID_IEnumSRShareA
 #define PIENUMSRSHARE             PIENUMSRSHAREA

#endif // _S_UNICODE


 
 
/*
 *  IVCmdMenu
 */
#undef   INTERFACE
#define  INTERFACE   IVCmdMenuW

DEFINE_GUID(IID_IVCmdMenuW, 0xDAC54F60L, 0x604D, 0x101B, 0x99, 0x26, 0x00, 0xAA, 0x00, 0x3C, 0xFC, 0x2C);

DECLARE_INTERFACE_ (IVCmdMenuW, IUnknown) {
   // IUnknown members
   STDMETHOD(QueryInterface)  (THIS_ REFIID riid, LPVOID FAR* ppvObj) PURE;
   STDMETHOD_(ULONG,AddRef)   (THIS) PURE;
   STDMETHOD_(ULONG,Release)  (THIS) PURE;

   // IVCmdMenu members
   STDMETHOD (Activate)       (THIS_ HWND, DWORD) PURE;
   STDMETHOD (Deactivate)     (THIS) PURE;
   STDMETHOD (TrainMenuDlg)   (THIS_ HWND, PCWSTR) PURE;
   STDMETHOD (Num)            (THIS_ DWORD *) PURE;
   STDMETHOD (Get)            (THIS_ DWORD, DWORD, DWORD, PSDATA, DWORD *) PURE;
   STDMETHOD (Set)            (THIS_ DWORD, DWORD, DWORD, SDATA) PURE;
   STDMETHOD (Add)            (THIS_ DWORD, SDATA, DWORD *) PURE;
   STDMETHOD (Remove)         (THIS_ DWORD, DWORD, DWORD) PURE;
   STDMETHOD (ListGet)        (THIS_ PCWSTR, PSDATA, DWORD *) PURE;
   STDMETHOD (ListSet)        (THIS_ PCWSTR, DWORD, SDATA) PURE;
   STDMETHOD (EnableItem)     (THIS_ DWORD, DWORD, DWORD) PURE;
   STDMETHOD (SetItem)        (THIS_ DWORD, DWORD, DWORD) PURE;
};

typedef IVCmdMenuW FAR * PIVCMDMENUW;


#undef   INTERFACE
#define  INTERFACE   IVCmdMenuA

// {746141E0-5543-11b9-C000-5611722E1D15}
DEFINE_GUID(IID_IVCmdMenuA, 0x746141e0, 0x5543, 0x11b9, 0xc0, 0x0, 0x56, 0x11, 0x72, 0x2e, 0x1d, 0x15);

DECLARE_INTERFACE_ (IVCmdMenuA, IUnknown) {
   // IUnknown members
   STDMETHOD(QueryInterface)  (THIS_ REFIID riid, LPVOID FAR* ppvObj) PURE;
   STDMETHOD_(ULONG,AddRef)   (THIS) PURE;
   STDMETHOD_(ULONG,Release)  (THIS) PURE;

   // IVCmdMenu members
   STDMETHOD (Activate)       (THIS_ HWND, DWORD) PURE;
   STDMETHOD (Deactivate)     (THIS) PURE;
   STDMETHOD (TrainMenuDlg)   (THIS_ HWND, PCSTR) PURE;
   STDMETHOD (Num)            (THIS_ DWORD *) PURE;
   STDMETHOD (Get)            (THIS_ DWORD, DWORD, DWORD, PSDATA, DWORD *) PURE;
   STDMETHOD (Set)            (THIS_ DWORD, DWORD, DWORD, SDATA) PURE;
   STDMETHOD (Add)            (THIS_ DWORD, SDATA, DWORD *) PURE;
   STDMETHOD (Remove)         (THIS_ DWORD, DWORD, DWORD) PURE;
   STDMETHOD (ListGet)        (THIS_ PCSTR, PSDATA, DWORD *) PURE;
   STDMETHOD (ListSet)        (THIS_ PCSTR, DWORD, SDATA) PURE;
   STDMETHOD (EnableItem)     (THIS_ DWORD, DWORD, DWORD) PURE;
   STDMETHOD (SetItem)        (THIS_ DWORD, DWORD, DWORD) PURE;
};

typedef IVCmdMenuA FAR * PIVCMDMENUA;


#ifdef _S_UNICODE
 #define IVCmdMenu      IVCmdMenuW
 #define IID_IVCmdMenu  IID_IVCmdMenuW
 #define PIVCMDMENU     PIVCMDMENUW

#else
 #define IVCmdMenu      IVCmdMenuA
 #define IID_IVCmdMenu  IID_IVCmdMenuA
 #define PIVCMDMENU     PIVCMDMENUA

#endif // _S_UNICODE



/*
 *  IVoiceCmd
 */
#undef   INTERFACE
#define  INTERFACE   IVoiceCmdW

DEFINE_GUID(IID_IVoiceCmdW, 0xE0DCC220L, 0x604D, 0x101B, 0x99, 0x26, 0x00, 0xAA, 0x00, 0x3C, 0xFC, 0x2C);

DECLARE_INTERFACE_ (IVoiceCmdW, IUnknown) {
   // IUnknown members
   STDMETHOD(QueryInterface)  (THIS_ REFIID riid, LPVOID FAR* ppvObj) PURE;
   STDMETHOD_(ULONG,AddRef)   (THIS) PURE;
   STDMETHOD_(ULONG,Release)  (THIS) PURE;

   // IVoiceCmd members
   STDMETHOD (Register)       (THIS_ PCWSTR, LPUNKNOWN, GUID, DWORD,
				     PVCSITEINFOW) PURE;
   STDMETHOD (MenuEnum)       (THIS_ DWORD, PCWSTR, PCWSTR, PIVCMDENUMW *) PURE;
   STDMETHOD (MenuCreate)     (THIS_ PVCMDNAMEW, PLANGUAGEW, DWORD,
				     PIVCMDMENUW *) PURE;
   STDMETHOD (MenuDelete)     (THIS_ PVCMDNAMEW) PURE;
   STDMETHOD (CmdMimic)       (THIS_ PVCMDNAMEW, PCWSTR) PURE;
};

typedef IVoiceCmdW FAR * PIVOICECMDW;


#undef   INTERFACE
#define  INTERFACE   IVoiceCmdA

// {C63A2B30-5543-11b9-C000-5611722E1D15}
DEFINE_GUID(IID_IVoiceCmdA, 0xc63a2b30, 0x5543, 0x11b9, 0xc0, 0x0, 0x56, 0x11, 0x72, 0x2e, 0x1d, 0x15);

DECLARE_INTERFACE_ (IVoiceCmdA, IUnknown) {
   // IUnknown members
   STDMETHOD(QueryInterface)  (THIS_ REFIID riid, LPVOID FAR* ppvObj) PURE;
   STDMETHOD_(ULONG,AddRef)   (THIS) PURE;
   STDMETHOD_(ULONG,Release)  (THIS) PURE;

   // IVoiceCmd members
   STDMETHOD (Register)       (THIS_ PCSTR, LPUNKNOWN, GUID, DWORD,
				     PVCSITEINFOA) PURE;
   STDMETHOD (MenuEnum)       (THIS_ DWORD, PCSTR, PCSTR, PIVCMDENUMA *) PURE;
   STDMETHOD (MenuCreate)     (THIS_ PVCMDNAMEA, PLANGUAGEA, DWORD,
				     PIVCMDMENUA *) PURE;
   STDMETHOD (MenuDelete)     (THIS_ PVCMDNAMEA) PURE;
   STDMETHOD (CmdMimic)       (THIS_ PVCMDNAMEA, PCSTR) PURE;
};

typedef IVoiceCmdA FAR * PIVOICECMDA;


#ifdef _S_UNICODE
 #define IVoiceCmd      IVoiceCmdW
 #define IID_IVoiceCmd  IID_IVoiceCmdW
 #define PIVOICECMD     PIVOICECMDW

#else
 #define IVoiceCmd      IVoiceCmdA
 #define IID_IVoiceCmd  IID_IVoiceCmdA
 #define PIVOICECMD     PIVOICECMDA

#endif //_S_UNICODE


/*
 *  IVCmdAttributes
 */
#undef   INTERFACE
#define  INTERFACE   IVCmdAttributesW

DEFINE_GUID(IID_IVCmdAttributesW, 0xE5F24680L, 0x6053, 0x101B, 0x99, 0x26, 0x00, 0xAA, 0x00, 0x3C, 0xFC, 0x2C);

DECLARE_INTERFACE_ (IVCmdAttributesW, IUnknown) {
   // IUnknown members
   STDMETHOD(QueryInterface)  (THIS_ REFIID riid, LPVOID FAR* ppvObj) PURE;
   STDMETHOD_(ULONG,AddRef)   (THIS) PURE;
   STDMETHOD_(ULONG,Release)  (THIS) PURE;

   // IVCmdAttributes members
   STDMETHOD (AutoGainEnableGet) (THIS_ DWORD *) PURE;
   STDMETHOD (AutoGainEnableSet) (THIS_ DWORD) PURE;
   STDMETHOD (AwakeStateGet)     (THIS_ DWORD *) PURE;
   STDMETHOD (AwakeStateSet)     (THIS_ DWORD) PURE;
   STDMETHOD (ThresholdGet)      (THIS_ DWORD *) PURE;
   STDMETHOD (ThresholdSet)      (THIS_ DWORD) PURE;
   STDMETHOD (DeviceGet)         (THIS_ DWORD *) PURE;
   STDMETHOD (DeviceSet)         (THIS_ DWORD) PURE;
   STDMETHOD (EnabledGet)        (THIS_ DWORD *) PURE;
   STDMETHOD (EnabledSet)        (THIS_ DWORD) PURE;
   STDMETHOD (MicrophoneGet)     (THIS_ PWSTR, DWORD, DWORD *) PURE;
   STDMETHOD (MicrophoneSet)     (THIS_ PCWSTR) PURE;
   STDMETHOD (SpeakerGet)        (THIS_ PWSTR, DWORD, DWORD *) PURE;
   STDMETHOD (SpeakerSet)        (THIS_ PCWSTR) PURE;
   STDMETHOD (SRModeGet)         (THIS_ GUID *) PURE;
   STDMETHOD (SRModeSet)         (THIS_ GUID) PURE;
};

typedef IVCmdAttributesW FAR * PIVCMDATTRIBUTESW;


#undef   INTERFACE
#define  INTERFACE   IVCmdAttributesA

// {FFF5DF80-5544-11b9-C000-5611722E1D15}
DEFINE_GUID(IID_IVCmdAttributesA, 0xfff5df80, 0x5544, 0x11b9, 0xc0, 0x0, 0x56, 0x11, 0x72, 0x2e, 0x1d, 0x15);

DECLARE_INTERFACE_ (IVCmdAttributesA, IUnknown) {
   // IUnknown members
   STDMETHOD(QueryInterface)  (THIS_ REFIID riid, LPVOID FAR* ppvObj) PURE;
   STDMETHOD_(ULONG,AddRef)   (THIS) PURE;
   STDMETHOD_(ULONG,Release)  (THIS) PURE;

   // IVCmdAttributes members
   STDMETHOD (AutoGainEnableGet) (THIS_ DWORD *) PURE;
   STDMETHOD (AutoGainEnableSet) (THIS_ DWORD) PURE;
   STDMETHOD (AwakeStateGet)     (THIS_ DWORD *) PURE;
   STDMETHOD (AwakeStateSet)     (THIS_ DWORD) PURE;
   STDMETHOD (ThresholdGet)      (THIS_ DWORD *) PURE;
   STDMETHOD (ThresholdSet)      (THIS_ DWORD) PURE;
   STDMETHOD (DeviceGet)         (THIS_ DWORD *) PURE;
   STDMETHOD (DeviceSet)         (THIS_ DWORD) PURE;
   STDMETHOD (EnabledGet)        (THIS_ DWORD *) PURE;
   STDMETHOD (EnabledSet)        (THIS_ DWORD) PURE;
   STDMETHOD (MicrophoneGet)     (THIS_ PSTR, DWORD, DWORD *) PURE;
   STDMETHOD (MicrophoneSet)     (THIS_ PCSTR) PURE;
   STDMETHOD (SpeakerGet)        (THIS_ PSTR, DWORD, DWORD *) PURE;
   STDMETHOD (SpeakerSet)        (THIS_ PCSTR) PURE;
   STDMETHOD (SRModeGet)         (THIS_ GUID *) PURE;
   STDMETHOD (SRModeSet)         (THIS_ GUID) PURE;
};

typedef IVCmdAttributesA FAR * PIVCMDATTRIBUTESA;


#ifdef _S_UNICODE
 #define IVCmdAttributes        IVCmdAttributesW
 #define IID_IVCmdAttributes    IID_IVCmdAttributesW
 #define PIVCMDATTRIBUTES       PIVCMDATTRIBUTESW

#else
 #define IVCmdAttributes        IVCmdAttributesA
 #define IID_IVCmdAttributes    IID_IVCmdAttributesA
 #define PIVCMDATTRIBUTES       PIVCMDATTRIBUTESA

#endif // _S_UNICODE



/*
 *  IVCmdDialog
 */
#undef   INTERFACE
#define  INTERFACE   IVCmdDialogsW

DEFINE_GUID(IID_IVCmdDialogsW, 0xEE39B8A0L, 0x6053, 0x101B, 0x99, 0x26, 0x00, 0xAA, 0x00, 0x3C, 0xFC, 0x2C);

DECLARE_INTERFACE_ (IVCmdDialogsW, IUnknown) {
   // IUnknown members
   STDMETHOD(QueryInterface)    (THIS_ REFIID riid, LPVOID FAR* ppvObj) PURE;
   STDMETHOD_(ULONG,AddRef)     (THIS) PURE;
   STDMETHOD_(ULONG,Release)    (THIS) PURE;

   // IVCmdDialogs members
   STDMETHOD (AboutDlg)         (THIS_ HWND, PCWSTR) PURE;
   STDMETHOD (GeneralDlg)       (THIS_ HWND, PCWSTR) PURE;
   STDMETHOD (LexiconDlg)       (THIS_ HWND, PCWSTR) PURE;
   STDMETHOD (TrainGeneralDlg)  (THIS_ HWND, PCWSTR) PURE;
   STDMETHOD (TrainMicDlg)      (THIS_ HWND, PCWSTR) PURE;
};

typedef IVCmdDialogsW FAR * PIVCMDDIALOGSW;


#undef   INTERFACE
#define  INTERFACE   IVCmdDialogsA

// {AA8FE260-5545-11b9-C000-5611722E1D15}
DEFINE_GUID(IID_IVCmdDialogsA, 0xaa8fe260, 0x5545, 0x11b9, 0xc0, 0x0, 0x56, 0x11, 0x72, 0x2e, 0x1d, 0x15);

DECLARE_INTERFACE_ (IVCmdDialogsA, IUnknown) {
   // IUnknown members
   STDMETHOD(QueryInterface)    (THIS_ REFIID riid, LPVOID FAR* ppvObj) PURE;
   STDMETHOD_(ULONG,AddRef)     (THIS) PURE;
   STDMETHOD_(ULONG,Release)    (THIS) PURE;

   // IVCmdDialogs members
   STDMETHOD (AboutDlg)         (THIS_ HWND, PCSTR) PURE;
   STDMETHOD (GeneralDlg)       (THIS_ HWND, PCSTR) PURE;
   STDMETHOD (LexiconDlg)       (THIS_ HWND, PCSTR) PURE;
   STDMETHOD (TrainGeneralDlg)  (THIS_ HWND, PCSTR) PURE;
   STDMETHOD (TrainMicDlg)      (THIS_ HWND, PCSTR) PURE;
};

typedef IVCmdDialogsA FAR * PIVCMDDIALOGSA;


#ifdef _S_UNICODE
 #define IVCmdDialogs       IVCmdDialogsW
 #define IID_IVCmdDialogs   IID_IVCmdDialogsW
 #define PIVCMDDIALOGS      PIVCMDDIALOGSW

#else
 #define IVCmdDialogs       IVCmdDialogsA
 #define IID_IVCmdDialogs   IID_IVCmdDialogsA
 #define PIVCMDDIALOGS      PIVCMDDIALOGSA

#endif // _S_UNICODE



/************************************************************************
class guids */

// DEFINE_GUID(CLSID_VCmd, 0x93898800L, 0x604D, 0x101B, 0x99, 0x26, 0x00, 0xAA, 0x00, 0x3C, 0xFC, 0x2C);
// {6D40D820-0BA7-11ce-A166-00AA004CD65C}
DEFINE_GUID(CLSID_VCmd, 
0x6d40d820, 0xba7, 0x11ce, 0xa1, 0x66, 0x0, 0xaa, 0x0, 0x4c, 0xd6, 0x5c);
// {89F70C30-8636-11ce-B763-00AA004CD65C}
DEFINE_GUID(CLSID_SRShare, 
0x89f70c30, 0x8636, 0x11ce, 0xb7, 0x63, 0x0, 0xaa, 0x0, 0x4c, 0xd6, 0x5c);



/************************************************************************
High-Level dictation speech recognition API
*/

/************************************************************************
defines */
#define     VDCT_TOPICNAMELEN       32

// bit flags for the dwReason field of IVDctNotifySink::TextChanged
#define     VDCT_TEXTADDED          0x00000001
#define     VDCT_TEXTREMOVED        0x00000002
#define     VDCT_TEXTREPLACED       0x00000004

// bit flags for the dwReason field of IVDctText::TextRemove/TextSet
#define     VDCT_TEXTCLEAN          0x00010000
#define     VDCT_TEXTKEEPRESULTS    0x00020000

// bit flags for dwFlags of IVDctGUI::FlagsSet
#define     VDCTGUIF_VISIBLE        0x00000001
#define     VDCTGUIF_DONTMOVE       0x00000002
#define     VDCTGUIF_ADDWORD        0x00000004

// bit flags for dwFX of IVDctText::FX
#define     VDCTFX_CAPFIRST         0x00000001
#define     VDCTFX_LOWERFIRST       0x00000002
#define     VDCTFX_TOGGLEFIRST      0x00000003
#define     VDCTFX_CAPALL           0x00000004
#define     VDCTFX_LOWERALL         0x00000005
#define     VDCTFX_REMOVESPACES     0x00000006
#define     VDCTFX_KEEPONLYFIRSTLETTER 0x00000007

/************************************************************************
typedefs */

// site information structure - used for IVoiceDictation::SiteInfoGet/Set
typedef struct {
    DWORD   dwAutoGainEnable;
    DWORD   dwAwakeState;
    DWORD   dwThreshold;
    DWORD   dwDevice;
    DWORD   dwEnable;
    WCHAR   szMicrophone[VCMD_MICLEN];
    WCHAR   szSpeaker[VCMD_SPEAKERLEN];
    GUID    gModeID;
} VDSITEINFOW, *PVDSITEINFOW;

typedef struct {
    DWORD   dwAutoGainEnable;
    DWORD   dwAwakeState;
    DWORD   dwThreshold;
    DWORD   dwDevice;
    DWORD   dwEnable;
    CHAR    szMicrophone[VCMD_MICLEN];
    CHAR    szSpeaker[VCMD_SPEAKERLEN];
    GUID    gModeID;
} VDSITEINFOA, *PVDSITEINFOA;


// topic structure used by the IVoiceDictation object
typedef struct {
    WCHAR       szTopic[VDCT_TOPICNAMELEN];
    LANGUAGEW   language;
} VDCTTOPICW, *PVDCTTOPICW;

typedef struct {
    CHAR        szTopic[VDCT_TOPICNAMELEN];
    LANGUAGEA   language;
} VDCTTOPICA, *PVDCTTOPICA;


#ifdef  _S_UNICODE
 #define VDSITEINFO  VDSITEINFOW
 #define PVDSITEINFO PVDSITEINFOW
 #define VDCTTOPIC   VDCTTOPICW
 #define PVDCTTOPIC  PVDCTTOPICW
#else
 #define VDSITEINFO  VDSITEINFOA
 #define PVDSITEINFO PVDSITEINFOA
 #define VDCTTOPIC   VDCTTOPICA
 #define PVDCTTOPIC  PVDCTTOPICA
#endif  // _S_UNICODE


// memory maintenance structure used by MemoryGet/Set in IVDctAttributes
typedef struct {
    DWORD   dwMaxRAM;
    DWORD   dwMaxTime;
    DWORD   dwMaxWords;
    BOOL    fKeepAudio;
    BOOL    fKeepCorrection;
    BOOL    fKeepEval;
} VDCTMEMORY, *PVDCTMEMORY;


// bookmark definition
typedef struct {
    DWORD   dwID;
    DWORD   dwPosn;
} VDCTBOOKMARK, *PVDCTBOOKMARK;



/************************************************************************
interfaces */

/*
 *  IVDctNotifySink
 */
#undef   INTERFACE
#define  INTERFACE   IVDctNotifySinkW

// {5FEB8800-67D5-11cf-9B8B-08005AFC3A41}
DEFINE_GUID(IID_IVDctNotifySinkW,
0x5feb8800, 0x67d5, 0x11cf, 0x9b, 0x8b, 0x8, 0x0, 0x5a, 0xfc, 0x3a, 0x41);

DECLARE_INTERFACE_ (IVDctNotifySinkW, IUnknown) {

   // IUnknown members
   STDMETHOD(QueryInterface)  (THIS_ REFIID riid, LPVOID FAR* ppvObj) PURE;
   STDMETHOD_(ULONG,AddRef)   (THIS) PURE;
   STDMETHOD_(ULONG,Release)  (THIS) PURE;

   // IVDctNotifySink members
   STDMETHOD (CommandBuiltIn)       (THIS_ PWSTR) PURE;
   STDMETHOD (CommandOther)         (THIS_ PWSTR) PURE;
   STDMETHOD (CommandRecognize)     (THIS_ DWORD, DWORD, DWORD, PVOID, PWSTR) PURE;
   STDMETHOD (TextSelChanged)       (THIS) PURE;
   STDMETHOD (TextChanged)          (THIS_ DWORD) PURE;
   STDMETHOD (TextBookmarkChanged)  (THIS_ DWORD) PURE;
   STDMETHOD (PhraseStart)          (THIS) PURE;
   STDMETHOD (PhraseFinish)         (THIS_ DWORD, PSRPHRASEW) PURE;
   STDMETHOD (PhraseHypothesis)     (THIS_ DWORD, PSRPHRASEW) PURE;
   STDMETHOD (UtteranceBegin)       (THIS) PURE;
   STDMETHOD (UtteranceEnd)         (THIS) PURE;
   STDMETHOD (VUMeter)              (THIS_ WORD) PURE;
   STDMETHOD (AttribChanged)        (THIS_ DWORD) PURE;
   STDMETHOD (Interference)         (THIS_ DWORD) PURE;
   STDMETHOD (Training)             (THIS_ DWORD) PURE;
   STDMETHOD (Dictating)            (THIS_ PCWSTR, BOOL) PURE;
};

typedef IVDctNotifySinkW FAR * PIVDCTNOTIFYSINKW;


#undef   INTERFACE
#define  INTERFACE   IVDctNotifySinkA

// {88AD7DC0-67D5-11cf-9B8B-08005AFC3A41}
DEFINE_GUID(IID_IVDctNotifySinkA,
0x88ad7dc0, 0x67d5, 0x11cf, 0x9b, 0x8b, 0x8, 0x0, 0x5a, 0xfc, 0x3a, 0x41);

DECLARE_INTERFACE_ (IVDctNotifySinkA, IUnknown) {

   // IUnknown members
   STDMETHOD(QueryInterface)  (THIS_ REFIID riid, LPVOID FAR* ppvObj) PURE;
   STDMETHOD_(ULONG,AddRef)   (THIS) PURE;
   STDMETHOD_(ULONG,Release)  (THIS) PURE;

   // IVDctNotifySinkA members
   STDMETHOD (CommandBuiltIn)       (THIS_ PSTR) PURE;
   STDMETHOD (CommandOther)         (THIS_ PSTR) PURE;
   STDMETHOD (CommandRecognize)     (THIS_ DWORD, DWORD, DWORD, PVOID, PSTR) PURE;
   STDMETHOD (TextSelChanged)       (THIS) PURE;
   STDMETHOD (TextChanged)          (THIS_ DWORD) PURE;
   STDMETHOD (TextBookmarkChanged)  (THIS_ DWORD) PURE;
   STDMETHOD (PhraseStart)          (THIS) PURE;
   STDMETHOD (PhraseFinish)         (THIS_ DWORD, PSRPHRASEA) PURE;
   STDMETHOD (PhraseHypothesis)     (THIS_ DWORD, PSRPHRASEA) PURE;
   STDMETHOD (UtteranceBegin)       (THIS) PURE;
   STDMETHOD (UtteranceEnd)         (THIS) PURE;
   STDMETHOD (VUMeter)              (THIS_ WORD) PURE;
   STDMETHOD (AttribChanged)        (THIS_ DWORD) PURE;
   STDMETHOD (Interference)         (THIS_ DWORD) PURE;
   STDMETHOD (Training)             (THIS_ DWORD) PURE;
   STDMETHOD (Dictating)            (THIS_ PCSTR, BOOL) PURE;
};

typedef IVDctNotifySinkA FAR * PIVDCTNOTIFYSINKA;


#ifdef _S_UNICODE
 #define IVDctNotifySink        IVDctNotifySinkW
 #define IID_IVDctNotifySink    IID_IVDctNotifySinkW
 #define PIVDCTNOTIFYSINK       PIVDCTNOTIFYSINKW

#else
 #define IVDctNotifySink        IVDctNotifySinkA
 #define IID_IVDctNotifySink    IID_IVDctNotifySinkA
 #define PIVDCTNOTIFYSINK       PIVDCTNOTIFYSINKA

#endif // _S_UNICODE




/*
 *  IVDctNotifySink2
 */
#undef   INTERFACE
#define  INTERFACE   IVDctNotifySink2W

// {599F77E3-E42E-11d1-BED8-006008317CE8}
DEFINE_GUID(IID_IVDctNotifySink2W, 
0x599f77e3, 0xe42e, 0x11d1, 0xbe, 0xd8, 0x0, 0x60, 0x8, 0x31, 0x7c, 0xe8);

DECLARE_INTERFACE_ (IVDctNotifySink2W, IVDctNotifySinkW) {

   // IUnknown members
   STDMETHOD(QueryInterface)  (THIS_ REFIID riid, LPVOID FAR* ppvObj) PURE;
   STDMETHOD_(ULONG,AddRef)   (THIS) PURE;
   STDMETHOD_(ULONG,Release)  (THIS) PURE;

   // IVDctNotifySink2 members
   STDMETHOD (CommandBuiltIn)       (THIS_ PWSTR) PURE;
   STDMETHOD (CommandOther)         (THIS_ PWSTR) PURE;
   STDMETHOD (CommandRecognize)     (THIS_ DWORD, DWORD, DWORD, PVOID, PWSTR) PURE;
   STDMETHOD (TextSelChanged)       (THIS) PURE;
   STDMETHOD (TextChanged)          (THIS_ DWORD) PURE;
   STDMETHOD (TextBookmarkChanged)  (THIS_ DWORD) PURE;
   STDMETHOD (PhraseStart)          (THIS) PURE;
   STDMETHOD (PhraseFinish)         (THIS_ DWORD, PSRPHRASEW) PURE;
   STDMETHOD (PhraseHypothesis)     (THIS_ DWORD, PSRPHRASEW) PURE;
   STDMETHOD (UtteranceBegin)       (THIS) PURE;
   STDMETHOD (UtteranceEnd)         (THIS) PURE;
   STDMETHOD (VUMeter)              (THIS_ WORD) PURE;
   STDMETHOD (AttribChanged)        (THIS_ DWORD) PURE;
   STDMETHOD (Interference)         (THIS_ DWORD) PURE;
   STDMETHOD (Training)             (THIS_ DWORD) PURE;
   STDMETHOD (Dictating)            (THIS_ PCWSTR, BOOL) PURE;

   STDMETHOD (Error)          (THIS_ LPUNKNOWN) PURE;
   STDMETHOD (Warning)        (THIS_ LPUNKNOWN) PURE;
};

typedef IVDctNotifySink2W FAR * PIVDCTNOTIFYSINK2W;


#undef   INTERFACE
#define  INTERFACE   IVDctNotifySink2A

// {599F77E4-E42E-11d1-BED8-006008317CE8}
DEFINE_GUID(IID_IVDctNotifySink2A, 
0x599f77e4, 0xe42e, 0x11d1, 0xbe, 0xd8, 0x0, 0x60, 0x8, 0x31, 0x7c, 0xe8);

DECLARE_INTERFACE_ (IVDctNotifySink2A, IVDctNotifySinkA) {

   // IUnknown members
   STDMETHOD(QueryInterface)  (THIS_ REFIID riid, LPVOID FAR* ppvObj) PURE;
   STDMETHOD_(ULONG,AddRef)   (THIS) PURE;
   STDMETHOD_(ULONG,Release)  (THIS) PURE;

   // IVDctNotifySink2A members
   STDMETHOD (CommandBuiltIn)       (THIS_ PSTR) PURE;
   STDMETHOD (CommandOther)         (THIS_ PSTR) PURE;
   STDMETHOD (CommandRecognize)     (THIS_ DWORD, DWORD, DWORD, PVOID, PSTR) PURE;
   STDMETHOD (TextSelChanged)       (THIS) PURE;
   STDMETHOD (TextChanged)          (THIS_ DWORD) PURE;
   STDMETHOD (TextBookmarkChanged)  (THIS_ DWORD) PURE;
   STDMETHOD (PhraseStart)          (THIS) PURE;
   STDMETHOD (PhraseFinish)         (THIS_ DWORD, PSRPHRASEA) PURE;
   STDMETHOD (PhraseHypothesis)     (THIS_ DWORD, PSRPHRASEA) PURE;
   STDMETHOD (UtteranceBegin)       (THIS) PURE;
   STDMETHOD (UtteranceEnd)         (THIS) PURE;
   STDMETHOD (VUMeter)              (THIS_ WORD) PURE;
   STDMETHOD (AttribChanged)        (THIS_ DWORD) PURE;
   STDMETHOD (Interference)         (THIS_ DWORD) PURE;
   STDMETHOD (Training)             (THIS_ DWORD) PURE;
   STDMETHOD (Dictating)            (THIS_ PCSTR, BOOL) PURE;

   STDMETHOD (Error)          (THIS_ LPUNKNOWN) PURE;
   STDMETHOD (Warning)        (THIS_ LPUNKNOWN) PURE;
};

typedef IVDctNotifySink2A FAR * PIVDCTNOTIFYSINK2A;


#ifdef _S_UNICODE
 #define IVDctNotifySink2        IVDctNotifySink2W
 #define IID_IVDctNotifySink2    IID_IVDctNotifySink2W
 #define PIVDCTNOTIFYSINK2       PIVDCTNOTIFYSINK2W

#else
 #define IVDctNotifySink2        IVDctNotifySink2A
 #define IID_IVDctNotifySink2    IID_IVDctNotifySink2A
 #define PIVDCTNOTIFYSINK2       PIVDCTNOTIFYSINK2A

#endif // _S_UNICODE


 
/*
 *  IVoiceDictation
 */
#undef   INTERFACE
#define  INTERFACE   IVoiceDictationW

// {88AD7DC3-67D5-11cf-9B8B-08005AFC3A41}
DEFINE_GUID(IID_IVoiceDictationW,
0x88ad7dc3, 0x67d5, 0x11cf, 0x9b, 0x8b, 0x8, 0x0, 0x5a, 0xfc, 0x3a, 0x41);

DECLARE_INTERFACE_ (IVoiceDictationW, IUnknown) {
   // IUnknown members
   STDMETHOD(QueryInterface)  (THIS_ REFIID riid, LPVOID FAR* ppvObj) PURE;
   STDMETHOD_(ULONG,AddRef)   (THIS) PURE;
   STDMETHOD_(ULONG,Release)  (THIS) PURE;

   // IVoiceDictation members
   STDMETHOD (Register)         (THIS_ PCWSTR, PCWSTR, LPSTORAGE, PCWSTR,
				                 PIVDCTNOTIFYSINK, GUID, DWORD) PURE;
   STDMETHOD (SiteInfoGet)      (THIS_ PCWSTR, PVDSITEINFOW) PURE;
   STDMETHOD (SiteInfoSet)      (THIS_ PCWSTR, PVDSITEINFOW) PURE;
   STDMETHOD (SessionSerialize) (THIS_ LPSTORAGE) PURE;
   STDMETHOD (SessionDeserialize) (THIS_ LPSTORAGE) PURE;
   STDMETHOD (TopicEnum)        (THIS_ PSDATA) PURE;
   STDMETHOD (TopicAddString)   (THIS_ PCWSTR, LANGUAGEW *) PURE;
   STDMETHOD (TopicRemove)      (THIS_ PCWSTR) PURE;
   STDMETHOD (TopicSerialize)   (THIS_ LPSTORAGE) PURE;
   STDMETHOD (TopicDeserialize) (THIS_ LPSTORAGE) PURE;
   STDMETHOD (Activate)         (THIS_ HWND) PURE;
   STDMETHOD (Deactivate)       (THIS) PURE;
   STDMETHOD (TopicAddGrammar)  (THIS_ PCWSTR, SDATA) PURE;
   STDMETHOD (CFGSet)           (THIS_ LANGID, PCWSTR, PCWSTR) PURE;
};

typedef IVoiceDictationW FAR * PIVOICEDICTATIONW;


#undef   INTERFACE
#define  INTERFACE   IVoiceDictationA

// {88AD7DC4-67D5-11cf-9B8B-08005AFC3A41}
DEFINE_GUID(IID_IVoiceDictationA,
0x88ad7dc4, 0x67d5, 0x11cf, 0x9b, 0x8b, 0x8, 0x0, 0x5a, 0xfc, 0x3a, 0x41);

DECLARE_INTERFACE_ (IVoiceDictationA, IUnknown) {
   // IUnknown members
   STDMETHOD(QueryInterface)  (THIS_ REFIID riid, LPVOID FAR* ppvObj) PURE;
   STDMETHOD_(ULONG,AddRef)   (THIS) PURE;
   STDMETHOD_(ULONG,Release)  (THIS) PURE;

   // IVoiceDictation members
   STDMETHOD (Register)         (THIS_ PCSTR, PCSTR, LPSTORAGE, PCSTR,
				 PIVDCTNOTIFYSINK, GUID, DWORD) PURE;
   STDMETHOD (SiteInfoGet)      (THIS_ PCSTR, PVDSITEINFOA) PURE;
   STDMETHOD (SiteInfoSet)      (THIS_ PCSTR, PVDSITEINFOA) PURE;
   STDMETHOD (SessionSerialize) (THIS_ LPSTORAGE) PURE;
   STDMETHOD (SessionDeserialize) (THIS_ LPSTORAGE) PURE;
   STDMETHOD (TopicEnum)        (THIS_ PSDATA) PURE;
   STDMETHOD (TopicAddString)   (THIS_ PCSTR, LANGUAGEA *) PURE;
   STDMETHOD (TopicRemove)      (THIS_ PCSTR) PURE;
   STDMETHOD (TopicSerialize)   (THIS_ LPSTORAGE) PURE;
   STDMETHOD (TopicDeserialize) (THIS_ LPSTORAGE) PURE;
   STDMETHOD (Activate)         (THIS_ HWND) PURE;
   STDMETHOD (Deactivate)       (THIS) PURE;
   STDMETHOD (TopicAddGrammar)  (THIS_ PCSTR, SDATA) PURE;
   STDMETHOD (CFGSet)           (THIS_ LANGID, PCSTR, PCSTR) PURE;
};

typedef IVoiceDictationA FAR * PIVOICEDICTATIONA;


#ifdef _S_UNICODE
 #define IVoiceDictation     IVoiceDictationW
 #define IID_IVoiceDictation IID_IVoiceDictationW
 #define PIVOICEDICTATION    PIVOICEDICTATIONW

#else
 #define IVoiceDictation     IVoiceDictationA
 #define IID_IVoiceDictation IID_IVoiceDictationA
 #define PIVOICEDICTATION    PIVOICEDICTATIONA

#endif //_S_UNICODE



/*
 *  IVDctText
 */
#undef   INTERFACE
#define  INTERFACE   IVDctTextW

// {6D62B3A0-6893-11cf-9B8B-08005AFC3A41}
DEFINE_GUID(IID_IVDctTextW,
0x6d62b3a0, 0x6893, 0x11cf, 0x9b, 0x8b, 0x8, 0x0, 0x5a, 0xfc, 0x3a, 0x41);

DECLARE_INTERFACE_ (IVDctTextW, IUnknown) {
   // IUnknown members
   STDMETHOD(QueryInterface)  (THIS_ REFIID riid, LPVOID FAR* ppvObj) PURE;
   STDMETHOD_(ULONG,AddRef)   (THIS) PURE;
   STDMETHOD_(ULONG,Release)  (THIS) PURE;

   // IVDctText members
   STDMETHOD (Lock)             (THIS) PURE;
   STDMETHOD (UnLock)           (THIS) PURE;
   STDMETHOD (TextGet)          (THIS_ DWORD, DWORD, PSDATA) PURE;
   STDMETHOD (TextSet)          (THIS_ DWORD, DWORD, PCWSTR, DWORD) PURE;
   STDMETHOD (TextMove)         (THIS_ DWORD, DWORD, DWORD, DWORD) PURE;
   STDMETHOD (TextRemove)       (THIS_ DWORD, DWORD, DWORD) PURE;
   STDMETHOD (TextSelSet)       (THIS_ DWORD, DWORD) PURE;
   STDMETHOD (TextSelGet)       (THIS_ DWORD *, DWORD *) PURE;
   STDMETHOD (TextLengthGet)    (THIS_ DWORD *) PURE;
   STDMETHOD (GetChanges)       (THIS_ DWORD *, DWORD *, DWORD *, DWORD *) PURE;
   STDMETHOD (BookmarkAdd)      (THIS_ PVDCTBOOKMARK) PURE;
   STDMETHOD (BookmarkRemove)   (THIS_ DWORD) PURE;
   STDMETHOD (BookmarkMove)     (THIS_ DWORD, DWORD) PURE;
   STDMETHOD (BookmarkQuery)    (THIS_ DWORD, PVDCTBOOKMARK) PURE;
   STDMETHOD (BookmarkEnum)     (THIS_ DWORD, DWORD, PVDCTBOOKMARK *,
				                 DWORD *) PURE;
   STDMETHOD (Hint)             (THIS_ PCWSTR) PURE;
   STDMETHOD (Words)            (THIS_ PCWSTR) PURE;
   STDMETHOD (ResultsGet)       (THIS_ DWORD, DWORD, DWORD *, DWORD *,
				                 LPUNKNOWN *) PURE;
   STDMETHOD (FX)               (THIS_ DWORD) PURE;
};
typedef IVDctTextW FAR * PIVDCTTEXTW;


#undef   INTERFACE
#define  INTERFACE   IVDctTextA

// {6D62B3A1-6893-11cf-9B8B-08005AFC3A41}
DEFINE_GUID(IID_IVDctTextA,
0x6d62b3a1, 0x6893, 0x11cf, 0x9b, 0x8b, 0x8, 0x0, 0x5a, 0xfc, 0x3a, 0x41);

DECLARE_INTERFACE_ (IVDctTextA, IUnknown) {
   // IUnknown members
   STDMETHOD(QueryInterface)  (THIS_ REFIID riid, LPVOID FAR* ppvObj) PURE;
   STDMETHOD_(ULONG,AddRef)   (THIS) PURE;
   STDMETHOD_(ULONG,Release)  (THIS) PURE;

   // IVDctText members
   STDMETHOD (Lock)             (THIS) PURE;
   STDMETHOD (UnLock)           (THIS) PURE;
   STDMETHOD (TextGet)          (THIS_ DWORD, DWORD, PSDATA) PURE;
   STDMETHOD (TextSet)          (THIS_ DWORD, DWORD, PCSTR, DWORD) PURE;
   STDMETHOD (TextMove)         (THIS_ DWORD, DWORD, DWORD, DWORD) PURE;
   STDMETHOD (TextRemove)       (THIS_ DWORD, DWORD, DWORD) PURE;
   STDMETHOD (TextSelSet)       (THIS_ DWORD, DWORD) PURE;
   STDMETHOD (TextSelGet)       (THIS_ DWORD *, DWORD *) PURE;
   STDMETHOD (TextLengthGet)    (THIS_ DWORD *) PURE;
   STDMETHOD (GetChanges)       (THIS_ DWORD *, DWORD *, DWORD *, DWORD *) PURE;
   STDMETHOD (BookmarkAdd)      (THIS_ PVDCTBOOKMARK) PURE;
   STDMETHOD (BookmarkRemove)   (THIS_ DWORD) PURE;
   STDMETHOD (BookmarkMove)     (THIS_ DWORD, DWORD) PURE;
   STDMETHOD (BookmarkQuery)    (THIS_ DWORD, PVDCTBOOKMARK) PURE;
   STDMETHOD (BookmarkEnum)     (THIS_ DWORD, DWORD, PVDCTBOOKMARK *,
				       DWORD *) PURE;
   STDMETHOD (Hint)             (THIS_ PCSTR) PURE;
   STDMETHOD (Words)            (THIS_ PCSTR) PURE;
   STDMETHOD (ResultsGet)       (THIS_ DWORD, DWORD, DWORD *, DWORD *,
				       LPUNKNOWN *) PURE;
   STDMETHOD (FX)               (THIS_ DWORD) PURE;
};
typedef IVDctTextA FAR * PIVDCTTEXTA;


#ifdef _S_UNICODE
 #define IVDctText      IVDctTextW
 #define IID_IVDctText  IID_IVDctTextW
 #define PIVDCTTEXT     PIVDCTTEXTW

#else
 #define IVDctText      IVDctTextA
 #define IID_IVDctText  IID_IVDctTextA
 #define PIVDCTTEXT     PIVDCTTEXTA

#endif //_S_UNICODE



/*
 *  IVDctText2
 */
#undef   INTERFACE
#define  INTERFACE   IVDctText2W

// {238004E1-F0C4-11d1-BED9-006008317CE8}
DEFINE_GUID(IID_IVDctText2W, 
0x238004e1, 0xf0c4, 0x11d1, 0xbe, 0xd9, 0x0, 0x60, 0x8, 0x31, 0x7c, 0xe8);

DECLARE_INTERFACE_ (IVDctText2W, IVDctTextW) {
   // IUnknown members
   STDMETHOD(QueryInterface)  (THIS_ REFIID riid, LPVOID FAR* ppvObj) PURE;
   STDMETHOD_(ULONG,AddRef)   (THIS) PURE;
   STDMETHOD_(ULONG,Release)  (THIS) PURE;

   // IVDctText members
   STDMETHOD (Lock)             (THIS) PURE;
   STDMETHOD (UnLock)           (THIS) PURE;
   STDMETHOD (TextGet)          (THIS_ DWORD, DWORD, PSDATA) PURE;
   STDMETHOD (TextSet)          (THIS_ DWORD, DWORD, PCWSTR, DWORD) PURE;
   STDMETHOD (TextMove)         (THIS_ DWORD, DWORD, DWORD, DWORD) PURE;
   STDMETHOD (TextRemove)       (THIS_ DWORD, DWORD, DWORD) PURE;
   STDMETHOD (TextSelSet)       (THIS_ DWORD, DWORD) PURE;
   STDMETHOD (TextSelGet)       (THIS_ DWORD *, DWORD *) PURE;
   STDMETHOD (TextLengthGet)    (THIS_ DWORD *) PURE;
   STDMETHOD (GetChanges)       (THIS_ DWORD *, DWORD *, DWORD *, DWORD *) PURE;
   STDMETHOD (BookmarkAdd)      (THIS_ PVDCTBOOKMARK) PURE;
   STDMETHOD (BookmarkRemove)   (THIS_ DWORD) PURE;
   STDMETHOD (BookmarkMove)     (THIS_ DWORD, DWORD) PURE;
   STDMETHOD (BookmarkQuery)    (THIS_ DWORD, PVDCTBOOKMARK) PURE;
   STDMETHOD (BookmarkEnum)     (THIS_ DWORD, DWORD, PVDCTBOOKMARK *,
				                 DWORD *) PURE;
   STDMETHOD (Hint)             (THIS_ PCWSTR) PURE;
   STDMETHOD (Words)            (THIS_ PCWSTR) PURE;
   STDMETHOD (ResultsGet)       (THIS_ DWORD, DWORD, DWORD *, DWORD *,
				                 LPUNKNOWN *) PURE;
   STDMETHOD (FX)               (THIS_ DWORD) PURE;

   // IVDctText2
   STDMETHOD (ITNApply)         (THIS_ DWORD, DWORD) PURE;
   STDMETHOD (ITNExpand)        (THIS_ DWORD, DWORD) PURE;
   STDMETHOD (ResultsGet2)      (THIS_ DWORD, DWORD, DWORD *, DWORD *,
				                     LPUNKNOWN *, DWORD*, DWORD*) PURE;
   STDMETHOD (ResultsSet)       (THIS_ DWORD, DWORD, 
				                     LPUNKNOWN, DWORD, DWORD) PURE;

};
typedef IVDctText2W FAR * PIVDCTTEXT2W;


#undef   INTERFACE
#define  INTERFACE   IVDctText2A

// {238004E2-F0C4-11d1-BED9-006008317CE8}
DEFINE_GUID(IID_IVDctText2A, 
0x238004e2, 0xf0c4, 0x11d1, 0xbe, 0xd9, 0x0, 0x60, 0x8, 0x31, 0x7c, 0xe8);

DECLARE_INTERFACE_ (IVDctText2A, IVDctTextA) {
   // IUnknown members
   STDMETHOD(QueryInterface)  (THIS_ REFIID riid, LPVOID FAR* ppvObj) PURE;
   STDMETHOD_(ULONG,AddRef)   (THIS) PURE;
   STDMETHOD_(ULONG,Release)  (THIS) PURE;

   // IVDctText members
   STDMETHOD (Lock)             (THIS) PURE;
   STDMETHOD (UnLock)           (THIS) PURE;
   STDMETHOD (TextGet)          (THIS_ DWORD, DWORD, PSDATA) PURE;
   STDMETHOD (TextSet)          (THIS_ DWORD, DWORD, PCSTR, DWORD) PURE;
   STDMETHOD (TextMove)         (THIS_ DWORD, DWORD, DWORD, DWORD) PURE;
   STDMETHOD (TextRemove)       (THIS_ DWORD, DWORD, DWORD) PURE;
   STDMETHOD (TextSelSet)       (THIS_ DWORD, DWORD) PURE;
   STDMETHOD (TextSelGet)       (THIS_ DWORD *, DWORD *) PURE;
   STDMETHOD (TextLengthGet)    (THIS_ DWORD *) PURE;
   STDMETHOD (GetChanges)       (THIS_ DWORD *, DWORD *, DWORD *, DWORD *) PURE;
   STDMETHOD (BookmarkAdd)      (THIS_ PVDCTBOOKMARK) PURE;
   STDMETHOD (BookmarkRemove)   (THIS_ DWORD) PURE;
   STDMETHOD (BookmarkMove)     (THIS_ DWORD, DWORD) PURE;
   STDMETHOD (BookmarkQuery)    (THIS_ DWORD, PVDCTBOOKMARK) PURE;
   STDMETHOD (BookmarkEnum)     (THIS_ DWORD, DWORD, PVDCTBOOKMARK *,
				       DWORD *) PURE;
   STDMETHOD (Hint)             (THIS_ PCSTR) PURE;
   STDMETHOD (Words)            (THIS_ PCSTR) PURE;
   STDMETHOD (ResultsGet)       (THIS_ DWORD, DWORD, DWORD *, DWORD *,
				       LPUNKNOWN *) PURE;
   STDMETHOD (FX)               (THIS_ DWORD) PURE;

   // IVDctText2
   STDMETHOD (ITNApply)         (THIS_ DWORD, DWORD) PURE;
   STDMETHOD (ITNExpand)        (THIS_ DWORD, DWORD) PURE;
   STDMETHOD (ResultsGet2)      (THIS_ DWORD, DWORD, DWORD *, DWORD *,
				                     LPUNKNOWN *, DWORD*, DWORD*) PURE;
   STDMETHOD (ResultsSet)       (THIS_ DWORD, DWORD, 
				                     LPUNKNOWN, DWORD, DWORD) PURE;
};
typedef IVDctText2A FAR * PIVDCTTEXT2A;


#ifdef _S_UNICODE
 #define IVDctText2      IVDctText2W
 #define IID_IVDctText2  IID_IVDctText2W
 #define PIVDCTTEXT2     PIVDCTTEXT2W

#else
 #define IVDctText2      IVDctText2A
 #define IID_IVDctText2  IID_IVDctText2A
 #define PIVDCTTEXT2     PIVDCTTEXT2A

#endif //_S_UNICODE




/*
 *  IVDctTextCache
 */
#undef   INTERFACE
#define  INTERFACE   IVDctTextCache

// {238004E3-F0C4-11d1-BED9-006008317CE8}
DEFINE_GUID(IID_IVDctTextCache, 
0x238004e3, 0xf0c4, 0x11d1, 0xbe, 0xd9, 0x0, 0x60, 0x8, 0x31, 0x7c, 0xe8);

DECLARE_INTERFACE_ (IVDctTextCache, IUnknown) {
   // IUnknown members
   STDMETHOD(QueryInterface)  (THIS_ REFIID riid, LPVOID FAR* ppvObj) PURE;
   STDMETHOD_(ULONG,AddRef)   (THIS) PURE;
   STDMETHOD_(ULONG,Release)  (THIS) PURE;

   // IVDctTextCache members
   STDMETHOD (CopyToBin)        (THIS_ DWORD, DWORD, DWORD) PURE;
   STDMETHOD (CopyToMemory)     (THIS_ DWORD, DWORD, PVOID*, DWORD*, LPUNKNOWN**, DWORD*) PURE;
   STDMETHOD (PasteFromBin)     (THIS_ DWORD, DWORD, DWORD) PURE;
   STDMETHOD (PasteFromMemory)  (THIS_ DWORD, DWORD, PVOID, DWORD, LPUNKNOWN*, DWORD) PURE;
};
typedef IVDctTextCache FAR * PIVDCTTEXTCACHE;



/*
 *  IVDctInvTextNorm
 */
#undef   INTERFACE
#define  INTERFACE   IVDctInvTextNormW

// {90A84EA0-6E51-11d0-9BC2-08005AFC3A41}
DEFINE_GUID(IID_IVDctInvTextNormW, 
0x90a84ea0, 0x6e51, 0x11d0, 0x9b, 0xc2, 0x8, 0x0, 0x5a, 0xfc, 0x3a, 0x41);

DECLARE_INTERFACE_ (IVDctInvTextNormW, IUnknown) {
   // IUnknown members
   STDMETHOD(QueryInterface)  (THIS_ REFIID riid, LPVOID FAR* ppvObj) PURE;
   STDMETHOD_(ULONG,AddRef)   (THIS) PURE;
   STDMETHOD_(ULONG,Release)  (THIS) PURE;

   // IVDctInvTextNorm members
   STDMETHOD    (OptionsEnum) (THIS_ PWSTR *, DWORD *) PURE;
   STDMETHOD    (OptionsGet)  (THIS_ PCWSTR, BOOL *) PURE;
   STDMETHOD    (OptionsSet)  (THIS_ PCWSTR, BOOL) PURE;
};

typedef IVDctInvTextNormW FAR * PIVDCTINVTEXTNORMW;


#undef   INTERFACE
#define  INTERFACE   IVDctInvTextNormA

// {90A84EA1-6E51-11d0-9BC2-08005AFC3A41}
DEFINE_GUID(IID_IVDctInvTextNormA, 
0x90a84ea1, 0x6e51, 0x11d0, 0x9b, 0xc2, 0x8, 0x0, 0x5a, 0xfc, 0x3a, 0x41);

DECLARE_INTERFACE_ (IVDctInvTextNormA, IUnknown) {
   // IUnknown members
   STDMETHOD(QueryInterface)  (THIS_ REFIID riid, LPVOID FAR* ppvObj) PURE;
   STDMETHOD_(ULONG,AddRef)   (THIS) PURE;
   STDMETHOD_(ULONG,Release)  (THIS) PURE;

   // IVDctInvTextNorm members
   STDMETHOD    (OptionsEnum) (THIS_ PSTR *, DWORD *) PURE;
   STDMETHOD    (OptionsGet)  (THIS_ PCSTR, BOOL *) PURE;
   STDMETHOD    (OptionsSet)  (THIS_ PCSTR, BOOL) PURE;
};

typedef IVDctInvTextNormA FAR * PIVDCTINVTEXTNORMA;


#ifdef _S_UNICODE
 #define IVDctInvTextNorm     IVDctInvTextNormW
 #define IID_IVDctInvTextNorm IID_IVDctInvTextNormW
 #define PIVDCTINVTEXTNORM    PIVDCTINVTEXTNORMW

#else
 #define IVDctInvTextNorm     IVDctInvTextNormA
 #define IID_IVDctInvTextNorm IID_IVDctInvTextNormA
 #define PIVDCTINVTEXTNORM    PIVDCTINVTEXTNORMA

#endif // _S_UNICODE



/*
 *  IVDctAttributes
 */
#undef   INTERFACE
#define  INTERFACE   IVDctAttributesW

// {88AD7DC5-67D5-11cf-9B8B-08005AFC3A41}
DEFINE_GUID(IID_IVDctAttributesW,
0x88ad7dc5, 0x67d5, 0x11cf, 0x9b, 0x8b, 0x8, 0x0, 0x5a, 0xfc, 0x3a, 0x41);

DECLARE_INTERFACE_ (IVDctAttributesW, IUnknown) {
   // IUnknown members
   STDMETHOD(QueryInterface)  (THIS_ REFIID riid, LPVOID FAR* ppvObj) PURE;
   STDMETHOD_(ULONG,AddRef)   (THIS) PURE;
   STDMETHOD_(ULONG,Release)  (THIS) PURE;

   // IVDctAttributes members
   STDMETHOD    (AutoGainEnableGet)  (THIS_ DWORD *) PURE;
   STDMETHOD    (AutoGainEnableSet)  (THIS_ DWORD) PURE;
   STDMETHOD    (AwakeStateGet)      (THIS_ DWORD *) PURE;
   STDMETHOD    (AwakeStateSet)      (THIS_ DWORD) PURE;
   STDMETHOD    (ThresholdGet)       (THIS_ DWORD *) PURE;
   STDMETHOD    (ThresholdSet)       (THIS_ DWORD) PURE;
   STDMETHOD    (EchoGet)            (THIS_ BOOL *) PURE;
   STDMETHOD    (EchoSet)            (THIS_ BOOL) PURE;
   STDMETHOD    (EnergyFloorGet)     (THIS_ WORD *) PURE;
   STDMETHOD    (EnergyFloorSet)     (THIS_ WORD) PURE;
   STDMETHOD    (RealTimeGet)        (THIS_ DWORD *) PURE;
   STDMETHOD    (RealTimeSet)        (THIS_ DWORD) PURE;
   STDMETHOD    (TimeOutGet)         (THIS_ DWORD *, DWORD *) PURE;
   STDMETHOD    (TimeOutSet)         (THIS_ DWORD, DWORD) PURE;
   STDMETHOD    (ModeGet)            (THIS_ DWORD *) PURE;
   STDMETHOD    (ModeSet)            (THIS_ DWORD) PURE;
   STDMETHOD    (MemoryGet)          (THIS_ VDCTMEMORY *) PURE;
   STDMETHOD    (MemorySet)          (THIS_ VDCTMEMORY *) PURE;
   STDMETHOD    (IsAnyoneDictating)  (THIS_ HWND, PWSTR, DWORD, DWORD *) PURE;
   STDMETHOD    (MicrophoneGet)      (THIS_ PWSTR, DWORD, DWORD *) PURE;
   STDMETHOD    (MicrophoneSet)      (THIS_ PCWSTR) PURE;
   STDMETHOD    (SpeakerGet)         (THIS_ PWSTR, DWORD, DWORD *) PURE;
   STDMETHOD    (SpeakerSet)         (THIS_ PCWSTR) PURE;
};

typedef IVDctAttributesW FAR * PIVDCTATTRIBUTESW;


#undef   INTERFACE
#define  INTERFACE   IVDctAttributesA

// {88AD7DC6-67D5-11cf-9B8B-08005AFC3A41}
DEFINE_GUID(IID_IVDctAttributesA,
0x88ad7dc6, 0x67d5, 0x11cf, 0x9b, 0x8b, 0x8, 0x0, 0x5a, 0xfc, 0x3a, 0x41);

DECLARE_INTERFACE_ (IVDctAttributesA, IUnknown) {
   // IUnknown members
   STDMETHOD(QueryInterface)  (THIS_ REFIID riid, LPVOID FAR* ppvObj) PURE;
   STDMETHOD_(ULONG,AddRef)   (THIS) PURE;
   STDMETHOD_(ULONG,Release)  (THIS) PURE;

   // IVDctAttributes members
   STDMETHOD    (AutoGainEnableGet)  (THIS_ DWORD *) PURE;
   STDMETHOD    (AutoGainEnableSet)  (THIS_ DWORD) PURE;
   STDMETHOD    (AwakeStateGet)      (THIS_ DWORD *) PURE;
   STDMETHOD    (AwakeStateSet)      (THIS_ DWORD) PURE;
   STDMETHOD    (ThresholdGet)       (THIS_ DWORD *) PURE;
   STDMETHOD    (ThresholdSet)       (THIS_ DWORD) PURE;
   STDMETHOD    (EchoGet)            (THIS_ BOOL *) PURE;
   STDMETHOD    (EchoSet)            (THIS_ BOOL) PURE;
   STDMETHOD    (EnergyFloorGet)     (THIS_ WORD *) PURE;
   STDMETHOD    (EnergyFloorSet)     (THIS_ WORD) PURE;
   STDMETHOD    (RealTimeGet)        (THIS_ DWORD *) PURE;
   STDMETHOD    (RealTimeSet)        (THIS_ DWORD) PURE;
   STDMETHOD    (TimeOutGet)         (THIS_ DWORD *, DWORD *) PURE;
   STDMETHOD    (TimeOutSet)         (THIS_ DWORD, DWORD) PURE;
   STDMETHOD    (ModeGet)            (THIS_ DWORD *) PURE;
   STDMETHOD    (ModeSet)            (THIS_ DWORD) PURE;
   STDMETHOD    (MemoryGet)          (THIS_ VDCTMEMORY *) PURE;
   STDMETHOD    (MemorySet)          (THIS_ VDCTMEMORY *) PURE;
   STDMETHOD    (IsAnyoneDictating)  (THIS_ HWND, PSTR, DWORD, DWORD *) PURE;
   STDMETHOD    (MicrophoneGet)      (THIS_ PSTR, DWORD, DWORD *) PURE;
   STDMETHOD    (MicrophoneSet)      (THIS_ PCSTR) PURE;
   STDMETHOD    (SpeakerGet)         (THIS_ PSTR, DWORD, DWORD *) PURE;
   STDMETHOD    (SpeakerSet)         (THIS_ PCSTR) PURE;
};

typedef IVDctAttributesA FAR * PIVDCTATTRIBUTESA;


#ifdef _S_UNICODE
 #define IVDctAttributes        IVDctAttributesW
 #define IID_IVDctAttributes    IID_IVDctAttributesW
 #define PIVDCTATTRIBUTES       PIVDCTATTRIBUTESW

#else
 #define IVDctAttributes        IVDctAttributesA
 #define IID_IVDctAttributes    IID_IVDctAttributesA
 #define PIVDCTATTRIBUTES       PIVDCTATTRIBUTESA

#endif // _S_UNICODE




/*
 * IVDctCommandsBuiltInW Interface
 */
// {D4E3F8E0-6521-11d1-8C35-006097DF5D01}
DEFINE_GUID(IID_IVDctCommandsBuiltInW, 
0xd4e3f8e0, 0x6521, 0x11d1, 0x8c, 0x35, 0x0, 0x60, 0x97, 0xdf, 0x5d, 0x1);

// {8763AFD1-7ADE-11d1-BEA7-006008317CE8}
DEFINE_GUID(IID_IVDctCommandsBuiltInA, 
0x8763afd1, 0x7ade, 0x11d1, 0xbe, 0xa7, 0x0, 0x60, 0x8, 0x31, 0x7c, 0xe8);

#undef   INTERFACE
#define  INTERFACE   IVDctCommandsBuiltInW

DECLARE_INTERFACE_ (IVDctCommandsBuiltInW, IUnknown) {
   // IUnknown members
   STDMETHOD(QueryInterface)  (THIS_ REFIID riid, LPVOID FAR* ppvObj) PURE;
   STDMETHOD_(ULONG,AddRef)   (THIS) PURE;
   STDMETHOD_(ULONG,Release)  (THIS) PURE;

   // IVDctCommandsBuiltInW members
   STDMETHOD (Activate) (THIS_ HWND, DWORD) PURE;
   STDMETHOD (Deactivate) (THIS) PURE;
   STDMETHOD (FromMemory) (THIS_ PVOID, DWORD) PURE;
   STDMETHOD (FromFile) (THIS_ PCWSTR) PURE;
   STDMETHOD (FromStream) (THIS_ IStream *) PURE;
   STDMETHOD (ListSet) (THIS_ PCWSTR, PVOID, DWORD) PURE;
   STDMETHOD (TextGet) (THIS_ LPWSTR*, DWORD*) PURE;
   STDMETHOD (ToMemory) (THIS_ PVOID*, DWORD*) PURE;
   STDMETHOD (ToFile) (THIS_ PCWSTR) PURE;
   STDMETHOD (ToStream) (THIS_ IStream *) PURE;
};

typedef IVDctCommandsBuiltInW * PIVDCTCOMMANDSBUILTINW;


#undef   INTERFACE
#define  INTERFACE   IVDctCommandsBuiltInA

DECLARE_INTERFACE_ (IVDctCommandsBuiltInA, IUnknown) {
   // IUnknown members
   STDMETHOD(QueryInterface)  (THIS_ REFIID riid, LPVOID FAR* ppvObj) PURE;
   STDMETHOD_(ULONG,AddRef)   (THIS) PURE;
   STDMETHOD_(ULONG,Release)  (THIS) PURE;

   // IVDctCommandsBuiltInA members
   STDMETHOD (Activate) (THIS_ HWND, DWORD) PURE;
   STDMETHOD (Deactivate) (THIS) PURE;
   STDMETHOD (FromMemory) (THIS_ PVOID, DWORD) PURE;
   STDMETHOD (FromFile) (THIS_ PCSTR) PURE;
   STDMETHOD (FromStream) (THIS_ IStream *) PURE;
   STDMETHOD (ListSet) (THIS_ PCSTR, PVOID, DWORD) PURE;
   STDMETHOD (TextGet) (THIS_ LPSTR*, DWORD*) PURE;
   STDMETHOD (ToMemory) (THIS_ PVOID*, DWORD*) PURE;
   STDMETHOD (ToFile) (THIS_ PCSTR) PURE;
   STDMETHOD (ToStream) (THIS_ IStream *) PURE;
};

typedef IVDctCommandsBuiltInA * PIVDCTCOMMANDSBUILTINA;

#ifdef _S_UNICODE
 #define IVDctCommandsBuiltIn        IVDctCommandsBuiltInW
 #define IID_IVDctCommandsBuiltIn    IID_IVDctCommandsBuiltInW
 #define PIVDCTCOMMANDSBUILTIN       PIVDCTCOMMANDSBUILTINW

#else
 #define IVDctCommandsBuiltIn        IVDctCommandsBuiltInA
 #define IID_IVDctCommandsBuiltIn    IID_IVDctCommandsBuiltInA
 #define PIVDCTCOMMANDSBUILTIN       PIVDCTCOMMANDSBUILTINA

#endif // _S_UNICODE



/*
 * IVDctCommandsApp Interface
 */
// {D4E3F8E1-6521-11d1-8C35-006097DF5D01}
DEFINE_GUID(IID_IVDctCommandsAppW, 
0xd4e3f8e1, 0x6521, 0x11d1, 0x8c, 0x35, 0x0, 0x60, 0x97, 0xdf, 0x5d, 0x1);

// {8763AFD2-7ADE-11d1-BEA7-006008317CE8}
DEFINE_GUID(IID_IVDctCommandsAppA, 
0x8763afd2, 0x7ade, 0x11d1, 0xbe, 0xa7, 0x0, 0x60, 0x8, 0x31, 0x7c, 0xe8);

typedef IVDctCommandsBuiltInA IVDctCommandsAppA;
typedef IVDctCommandsAppA * PIVDCTCOMMANDSAPPA;
typedef IVDctCommandsBuiltInW IVDctCommandsAppW;
typedef IVDctCommandsAppW * PIVDCTCOMMANDSAPPW;

#ifdef _S_UNICODE
 #define IVDctCommandsApp        IVDctCommandsAppW
 #define IID_IVDctCommandsApp    IID_IVDctCommandsAppW
 #define PIVDCTCOMMANDSAPP       PIVDCTCOMMANDSAPPW
#else
 #define IVDctCommandsApp        IVDctCommandsAppA
 #define IID_IVDctCommandsApp    IID_IVDctCommandsAppA
 #define PIVDCTCOMMANDSAPP       PIVDCTCOMMANDSAPPA
#endif // _S_UNICODE



/*
 *  IVDctCommands
 */
#undef   INTERFACE
#define  INTERFACE   IVDctCommandsW

// {A02C2CA0-AE50-11cf-833A-00AA00A21A29}
DEFINE_GUID(IID_IVDctCommandsW,
0xA02C2CA0, 0xAE50, 0x11cf, 0x83, 0x3A, 0x00, 0xAA, 0x00, 0xA2, 0x1A, 0x29);

DECLARE_INTERFACE_ (IVDctCommandsW, IUnknown) {
   // IUnknown members
   STDMETHOD(QueryInterface)  (THIS_ REFIID riid, LPVOID FAR* ppvObj) PURE;
   STDMETHOD_(ULONG,AddRef)   (THIS) PURE;
   STDMETHOD_(ULONG,Release)  (THIS) PURE;

   // IVDctCommands members
   STDMETHOD    (Add)      (THIS_ BOOL, DWORD, SDATA, DWORD *) PURE;
   STDMETHOD    (Remove)   (THIS_ BOOL, DWORD, DWORD, DWORD) PURE;
   STDMETHOD    (Get)      (THIS_ BOOL, DWORD, DWORD, DWORD, SDATA*, DWORD *) PURE;
   STDMETHOD    (Set)      (THIS_ BOOL, DWORD, DWORD, DWORD, SDATA) PURE;
   STDMETHOD    (EnableItem) (THIS_ BOOL, DWORD, DWORD, DWORD) PURE;
   STDMETHOD    (Num)      (THIS_ BOOL, DWORD *) PURE;
};

typedef IVDctCommandsW FAR * PIVDCTCOMMANDSW;


#undef   INTERFACE
#define  INTERFACE   IVDctCommandsA

// {A02C2CA1-AE50-11cf-833A-00AA00A21A29}
DEFINE_GUID(IID_IVDctCommandsA,
0xA02C2CA1, 0xAE50, 0x11cf, 0x83, 0x3A, 0x00, 0xAA, 0x00, 0xA2, 0x1A, 0x29);

DECLARE_INTERFACE_ (IVDctCommandsA, IUnknown) {
   // IUnknown members
   STDMETHOD(QueryInterface)  (THIS_ REFIID riid, LPVOID FAR* ppvObj) PURE;
   STDMETHOD_(ULONG,AddRef)   (THIS) PURE;
   STDMETHOD_(ULONG,Release)  (THIS) PURE;

   // IVDctCommands members
   STDMETHOD    (Add)      (THIS_ BOOL, DWORD, SDATA, DWORD *) PURE;
   STDMETHOD    (Remove)   (THIS_ BOOL, DWORD, DWORD, DWORD) PURE;
   STDMETHOD    (Get)      (THIS_ BOOL, DWORD, DWORD, DWORD, SDATA*, DWORD *) PURE;
   STDMETHOD    (Set)      (THIS_ BOOL, DWORD, DWORD, DWORD, SDATA) PURE;
   STDMETHOD    (EnableItem) (THIS_ BOOL, DWORD, DWORD, DWORD) PURE;
   STDMETHOD    (Num)      (THIS_ BOOL, DWORD *) PURE;
};

typedef IVDctCommandsA FAR * PIVDCTCOMMANDSA;


#ifdef _S_UNICODE
 #define IVDctCommands        IVDctCommandsW
 #define IID_IVDctCommands    IID_IVDctCommandsW
 #define PIVDCTCOMMANDS       PIVDCTCOMMANDSW

#else
 #define IVDctCommands        IVDctCommandsA
 #define IID_IVDctCommands    IID_IVDctCommandsA
 #define PIVDCTCOMMANDS       PIVDCTCOMMANDSA

#endif // _S_UNICODE



/*
 *  IVDctGlossary
 */
#undef   INTERFACE
#define  INTERFACE   IVDctGlossaryW

// {A02C2CA2-AE50-11cf-833A-00AA00A21A29}
DEFINE_GUID(IID_IVDctGlossaryW,
0xA02C2CA2, 0xAE50, 0x11cf, 0x83, 0x3A, 0x00, 0xAA, 0x00, 0xA2, 0x1A, 0x29);

DECLARE_INTERFACE_ (IVDctGlossaryW, IUnknown) {
   // IUnknown members
   STDMETHOD(QueryInterface)  (THIS_ REFIID riid, LPVOID FAR* ppvObj) PURE;
   STDMETHOD_(ULONG,AddRef)   (THIS) PURE;
   STDMETHOD_(ULONG,Release)  (THIS) PURE;

   // IVDctGlossary members
   STDMETHOD    (Add)      (THIS_ BOOL, DWORD, SDATA, DWORD *) PURE;
   STDMETHOD    (Remove)   (THIS_ BOOL, DWORD, DWORD, DWORD) PURE;
   STDMETHOD    (Get)      (THIS_ BOOL, DWORD, DWORD, DWORD, SDATA*, DWORD *) PURE;
   STDMETHOD    (Set)      (THIS_ BOOL, DWORD, DWORD, DWORD, SDATA) PURE;
   STDMETHOD    (EnableItem) (THIS_ BOOL, DWORD, DWORD, DWORD) PURE;
   STDMETHOD    (Num)      (THIS_ BOOL, DWORD *) PURE;
};

typedef IVDctGlossaryW FAR * PIVDCTGLOSSARYW;


#undef   INTERFACE
#define  INTERFACE   IVDctGlossaryA

// {A02C2CA3-AE50-11cf-833A-00AA00A21A29}
DEFINE_GUID(IID_IVDctGlossaryA,
0xA02C2CA3, 0xAE50, 0x11cf, 0x83, 0x3A, 0x00, 0xAA, 0x00, 0xA2, 0x1A, 0x29);

DECLARE_INTERFACE_ (IVDctGlossaryA, IUnknown) {
   // IUnknown members
   STDMETHOD(QueryInterface)  (THIS_ REFIID riid, LPVOID FAR* ppvObj) PURE;
   STDMETHOD_(ULONG,AddRef)   (THIS) PURE;
   STDMETHOD_(ULONG,Release)  (THIS) PURE;

   // IVDctGlossary members
   STDMETHOD    (Add)      (THIS_ BOOL, DWORD, SDATA, DWORD *) PURE;
   STDMETHOD    (Remove)   (THIS_ BOOL, DWORD, DWORD, DWORD) PURE;
   STDMETHOD    (Get)      (THIS_ BOOL, DWORD, DWORD, DWORD, SDATA*, DWORD *) PURE;
   STDMETHOD    (Set)      (THIS_ BOOL, DWORD, DWORD, DWORD, SDATA) PURE;
   STDMETHOD    (EnableItem) (THIS_ BOOL, DWORD, DWORD, DWORD) PURE;
   STDMETHOD    (Num)      (THIS_ BOOL, DWORD *) PURE;
};

typedef IVDctGlossaryA FAR * PIVDCTGLOSSARYA;


#ifdef _S_UNICODE
 #define IVDctGlossary        IVDctGlossaryW
 #define IID_IVDctGlossary    IID_IVDctGlossaryW
 #define PIVDCTGLOSSARY       PIVDCTGLOSSARYW

#else
 #define IVDctGlossary        IVDctGlossaryA
 #define IID_IVDctGlossary    IID_IVDctGlossaryA
 #define PIVDCTGLOSSARY       PIVDCTGLOSSARYA

#endif // _S_UNICODE




/*
 *  IVDctDialog
 */
#undef   INTERFACE
#define  INTERFACE   IVDctDialogsW

// {88AD7DC7-67D5-11cf-9B8B-08005AFC3A41}
DEFINE_GUID(IID_IVDctDialogsW,
0x88ad7dc7, 0x67d5, 0x11cf, 0x9b, 0x8b, 0x8, 0x0, 0x5a, 0xfc, 0x3a, 0x41);

DECLARE_INTERFACE_ (IVDctDialogsW, IUnknown) {
   // IUnknown members
   STDMETHOD(QueryInterface)    (THIS_ REFIID riid, LPVOID FAR* ppvObj) PURE;
   STDMETHOD_(ULONG,AddRef)     (THIS) PURE;
   STDMETHOD_(ULONG,Release)    (THIS) PURE;

   // IVDctDialogs members
   STDMETHOD (AboutDlg)         (THIS_ HWND, PCWSTR) PURE;
   STDMETHOD (GeneralDlg)       (THIS_ HWND, PCWSTR) PURE;
   STDMETHOD (LexiconDlg)       (THIS_ HWND, PCWSTR) PURE;
   STDMETHOD (TrainGeneralDlg)  (THIS_ HWND, PCWSTR) PURE;
   STDMETHOD (TrainMicDlg)      (THIS_ HWND, PCWSTR) PURE;
};

typedef IVDctDialogsW FAR * PIVDCTDIALOGSW;


#undef   INTERFACE
#define  INTERFACE   IVDctDialogsA

// {88AD7DC8-67D5-11cf-9B8B-08005AFC3A41}
DEFINE_GUID(IID_IVDctDialogsA,
0x88ad7dc8, 0x67d5, 0x11cf, 0x9b, 0x8b, 0x8, 0x0, 0x5a, 0xfc, 0x3a, 0x41);

DECLARE_INTERFACE_ (IVDctDialogsA, IUnknown) {
   // IUnknown members
   STDMETHOD(QueryInterface)    (THIS_ REFIID riid, LPVOID FAR* ppvObj) PURE;
   STDMETHOD_(ULONG,AddRef)     (THIS) PURE;
   STDMETHOD_(ULONG,Release)    (THIS) PURE;

   // IVDctDialogs members
   STDMETHOD (AboutDlg)         (THIS_ HWND, PCSTR) PURE;
   STDMETHOD (GeneralDlg)       (THIS_ HWND, PCSTR) PURE;
   STDMETHOD (LexiconDlg)       (THIS_ HWND, PCSTR) PURE;
   STDMETHOD (TrainGeneralDlg)  (THIS_ HWND, PCSTR) PURE;
   STDMETHOD (TrainMicDlg)      (THIS_ HWND, PCSTR) PURE;
};

typedef IVDctDialogsA FAR * PIVDCTDIALOGSA;


#ifdef _S_UNICODE
 #define IVDctDialogs       IVDctDialogsW
 #define IID_IVDctDialogs   IID_IVDctDialogsW
 #define PIVDCTDIALOGS      PIVDCTDIALOGSW

#else
 #define IVDctDialogs       IVDctDialogsA
 #define IID_IVDctDialogs   IID_IVDctDialogsA
 #define PIVDCTDIALOGS      PIVDCTDIALOGSA

#endif // _S_UNICODE


#undef   INTERFACE
#define  INTERFACE   IVDctGUI

// {8953F1A0-7E80-11cf-8D15-00A0C9034A7E}
DEFINE_GUID(IID_IVDctGUI,
0x8953f1a0, 0x7e80, 0x11cf, 0x8d, 0x15, 0x0, 0xa0, 0xc9, 0x3, 0x4a, 0x7e);

DECLARE_INTERFACE_ (IVDctGUI, IUnknown) {
   // IUnknown members
   STDMETHOD(QueryInterface)    (THIS_ REFIID riid, LPVOID FAR* ppvObj) PURE;
   STDMETHOD_(ULONG,AddRef)     (THIS) PURE;
   STDMETHOD_(ULONG,Release)    (THIS) PURE;

   // IVDctDialogs members
   STDMETHOD (SetSelRect)       (THIS_ RECT *) PURE;
   STDMETHOD (FlagsSet)         (THIS_ DWORD) PURE;
   STDMETHOD (FlagsGet)         (THIS_ DWORD *) PURE;
};

typedef IVDctGUI FAR * PIVDCTGUI;



/************************************************************************
class guids */

// {25522CA0-67CE-11cf-9B8B-08005AFC3A41}
DEFINE_GUID(CLSID_VDct, 
0x35522ca0, 0x67ce, 0x11cf, 0x9b, 0x8b, 0x8, 0x0, 0x5a, 0xfc, 0x3a, 0x41);



/************************************************************************
High-Level text-to-speech API
*/


/************************************************************************
defines */

#define  ONE                    (1)

// dwFlags parameter of IVoiceText::Register
#define  VTXTF_ALLMESSAGES      (ONE<<0)

/*
 *   dwFlags parameter of IVoiceText::Speak
 */

// type of speech
#define  VTXTST_STATEMENT       0x00000001
#define  VTXTST_QUESTION        0x00000002
#define  VTXTST_COMMAND         0x00000004
#define  VTXTST_WARNING         0x00000008
#define  VTXTST_READING         0x00000010
#define  VTXTST_NUMBERS         0x00000020
#define  VTXTST_SPREADSHEET     0x00000040

// priorities
#define  VTXTSP_VERYHIGH        0x00000080
#define  VTXTSP_HIGH            0x00000100
#define  VTXTSP_NORMAL          0x00000200

/************************************************************************
typedefs */

// possible parameter to IVoiceText::Register
typedef struct {
    DWORD   dwDevice;
    DWORD   dwEnable;
    DWORD   dwSpeed;
    GUID    gModeID;
} VTSITEINFO, *PVTSITEINFO;


/************************************************************************
interfaces */

/*
 *  IVCmdNotifySink
 */
#undef   INTERFACE
#define  INTERFACE   IVTxtNotifySinkW

// {FD3A2430-E090-11cd-A166-00AA004CD65C}
DEFINE_GUID(IID_IVTxtNotifySinkW, 0xfd3a2430, 0xe090, 0x11cd, 0xa1, 0x66, 0x0, 0xaa, 0x0, 0x4c, 0xd6, 0x5c);

DECLARE_INTERFACE_ (IVTxtNotifySinkW, IUnknown) {

   // IUnknown members
   STDMETHOD(QueryInterface)  (THIS_ REFIID riid, LPVOID FAR* ppvObj) PURE;
   STDMETHOD_(ULONG,AddRef)   (THIS) PURE;
   STDMETHOD_(ULONG,Release)  (THIS) PURE;

   // IVTxtNotifySinkW members
   STDMETHOD (AttribChanged)    (THIS_ DWORD) PURE;
   STDMETHOD (Visual)           (THIS_ WCHAR, WCHAR, DWORD, PTTSMOUTH) PURE;
   STDMETHOD (Speak)            (THIS_ PWSTR, PWSTR, DWORD) PURE;
   STDMETHOD (SpeakingStarted)  (THIS) PURE;
   STDMETHOD (SpeakingDone)     (THIS) PURE;
};

typedef IVTxtNotifySinkW FAR * PIVTXTNOTIFYSINKW;


#undef   INTERFACE
#define  INTERFACE   IVTxtNotifySinkA

// {D2C840E0-E092-11cd-A166-00AA004CD65C}
DEFINE_GUID(IID_IVTxtNotifySinkA, 0xd2c840e0, 0xe092, 0x11cd, 0xa1, 0x66, 0x0, 0xaa, 0x0, 0x4c, 0xd6, 0x5c);

DECLARE_INTERFACE_ (IVTxtNotifySinkA, IUnknown) {

   // IUnknown members
   STDMETHOD(QueryInterface)  (THIS_ REFIID riid, LPVOID FAR* ppvObj) PURE;
   STDMETHOD_(ULONG,AddRef)   (THIS) PURE;
   STDMETHOD_(ULONG,Release)  (THIS) PURE;

   // IVTxtNotifySinkA members
   STDMETHOD (AttribChanged)    (THIS_ DWORD) PURE;
   STDMETHOD (Visual)           (THIS_ WCHAR, CHAR, DWORD, PTTSMOUTH) PURE;
   STDMETHOD (Speak)            (THIS_ PSTR, PSTR, DWORD) PURE;
   STDMETHOD (SpeakingStarted)  (THIS) PURE;
   STDMETHOD (SpeakingDone)     (THIS) PURE;
};

typedef IVTxtNotifySinkA FAR * PIVTXTNOTIFYSINKA;


#ifdef _S_UNICODE
 #define IVTxtNotifySink        IVTxtNotifySinkW
 #define IID_IVTxtNotifySink    IID_IVTxtNotifySinkW
 #define PIVTXTNOTIFYSINK       PIVTXTNOTIFYSINKW

#else
 #define IVTxtNotifySink        IVTxtNotifySinkA
 #define IID_IVTxtNotifySink    IID_IVTxtNotifySinkA
 #define PIVTXTNOTIFYSINK       PIVTXTNOTIFYSINKA

#endif // _S_UNICODE



/*
 *  IVoiceText
 */
#undef   INTERFACE
#define  INTERFACE   IVoiceTextW

// {C4FE8740-E093-11cd-A166-00AA004CD65C}
DEFINE_GUID(IID_IVoiceTextW, 0xc4fe8740, 0xe093, 0x11cd, 0xa1, 0x66, 0x0, 0xaa, 0x0, 0x4c, 0xd6, 0x5c);

DECLARE_INTERFACE_ (IVoiceTextW, IUnknown) {
    // IUnknown members
    STDMETHOD(QueryInterface)   (THIS_ REFIID riid, LPVOID FAR* ppvObj) PURE;
    STDMETHOD_(ULONG,AddRef)    (THIS) PURE;
    STDMETHOD_(ULONG,Release)   (THIS) PURE;

    // IVoiceText members

    STDMETHOD (Register)         (THIS_ PCWSTR, PCWSTR, 
					PIVTXTNOTIFYSINK, GUID,
					DWORD, PVTSITEINFO) PURE;
    STDMETHOD (Speak)            (THIS_ PCWSTR, DWORD, PCWSTR) PURE;
    STDMETHOD (StopSpeaking)     (THIS) PURE;
    STDMETHOD (AudioFastForward) (THIS) PURE;
    STDMETHOD (AudioPause)       (THIS) PURE;
    STDMETHOD (AudioResume)      (THIS) PURE;
    STDMETHOD (AudioRewind)      (THIS) PURE;
};

typedef IVoiceTextW FAR * PIVOICETEXTW;


#undef   INTERFACE
#define  INTERFACE   IVoiceTextA

// {E1B7A180-E093-11cd-A166-00AA004CD65C}
DEFINE_GUID(IID_IVoiceTextA, 0xe1b7a180, 0xe093, 0x11cd, 0xa1, 0x66, 0x0, 0xaa, 0x0, 0x4c, 0xd6, 0x5c);

DECLARE_INTERFACE_ (IVoiceTextA, IUnknown) {
   // IUnknown members
   STDMETHOD(QueryInterface)  (THIS_ REFIID riid, LPVOID FAR* ppvObj) PURE;
   STDMETHOD_(ULONG,AddRef)   (THIS) PURE;
   STDMETHOD_(ULONG,Release)  (THIS) PURE;

   // IVoiceText members

    STDMETHOD (Register)         (THIS_ PCSTR, PCSTR, 
					PIVTXTNOTIFYSINK, GUID,
					DWORD, PVTSITEINFO) PURE;
    STDMETHOD (Speak)            (THIS_ PCSTR, DWORD, PCSTR) PURE;
    STDMETHOD (StopSpeaking)     (THIS) PURE;
    STDMETHOD (AudioFastForward) (THIS) PURE;
    STDMETHOD (AudioPause)       (THIS) PURE;
    STDMETHOD (AudioResume)      (THIS) PURE;
    STDMETHOD (AudioRewind)      (THIS) PURE;
};

typedef IVoiceTextA FAR * PIVOICETEXTA;


#ifdef _S_UNICODE
 #define IVoiceText      IVoiceTextW
 #define IID_IVoiceText  IID_IVoiceTextW
 #define PIVOICETEXT     PIVOICETEXTW

#else
 #define IVoiceText      IVoiceTextA
 #define IID_IVoiceText  IID_IVoiceTextA
 #define PIVOICETEXT     PIVOICETEXTA

#endif //_S_UNICODE



/*
 *  IVTxtAttributes
 */
#undef   INTERFACE
#define  INTERFACE   IVTxtAttributesW

// {6A8D6140-E095-11cd-A166-00AA004CD65C}
DEFINE_GUID(IID_IVTxtAttributesW, 0x6a8d6140, 0xe095, 0x11cd, 0xa1, 0x66, 0x0, 0xaa, 0x0, 0x4c, 0xd6, 0x5c);

DECLARE_INTERFACE_ (IVTxtAttributesW, IUnknown) {
   // IUnknown members
   STDMETHOD(QueryInterface)  (THIS_ REFIID riid, LPVOID FAR* ppvObj) PURE;
   STDMETHOD_(ULONG,AddRef)   (THIS) PURE;
   STDMETHOD_(ULONG,Release)  (THIS) PURE;

   // IVTxtAttributes members
   STDMETHOD (DeviceGet)         (THIS_ DWORD *) PURE;
   STDMETHOD (DeviceSet)         (THIS_ DWORD) PURE;
   STDMETHOD (EnabledGet)        (THIS_ DWORD *) PURE;
   STDMETHOD (EnabledSet)        (THIS_ DWORD) PURE;
   STDMETHOD (IsSpeaking)        (THIS_ BOOL *) PURE;
   STDMETHOD (SpeedGet)          (THIS_ DWORD *) PURE;
   STDMETHOD (SpeedSet)          (THIS_ DWORD) PURE;
   STDMETHOD (TTSModeGet)        (THIS_ GUID *) PURE;
   STDMETHOD (TTSModeSet)        (THIS_ GUID) PURE;
};

typedef IVTxtAttributesW FAR * PIVTXTATTRIBUTESW;


#undef   INTERFACE
#define  INTERFACE   IVTxtAttributesA

// {8BE9CC30-E095-11cd-A166-00AA004CD65C}
DEFINE_GUID(IID_IVTxtAttributesA, 0x8be9cc30, 0xe095, 0x11cd, 0xa1, 0x66, 0x0, 0xaa, 0x0, 0x4c, 0xd6, 0x5c);

DECLARE_INTERFACE_ (IVTxtAttributesA, IUnknown) {
   // IUnknown members
   STDMETHOD(QueryInterface)  (THIS_ REFIID riid, LPVOID FAR* ppvObj) PURE;
   STDMETHOD_(ULONG,AddRef)   (THIS) PURE;
   STDMETHOD_(ULONG,Release)  (THIS) PURE;

   // IVTxtAttributes members
   STDMETHOD (DeviceGet)         (THIS_ DWORD *) PURE;
   STDMETHOD (DeviceSet)         (THIS_ DWORD) PURE;
   STDMETHOD (EnabledGet)        (THIS_ DWORD *) PURE;
   STDMETHOD (EnabledSet)        (THIS_ DWORD) PURE;
   STDMETHOD (IsSpeaking)        (THIS_ BOOL *) PURE;
   STDMETHOD (SpeedGet)          (THIS_ DWORD *) PURE;
   STDMETHOD (SpeedSet)          (THIS_ DWORD) PURE;
   STDMETHOD (TTSModeGet)        (THIS_ GUID *) PURE;
   STDMETHOD (TTSModeSet)        (THIS_ GUID) PURE;
};

typedef IVTxtAttributesA FAR * PIVTXTATTRIBUTESA;


#ifdef _S_UNICODE
 #define IVTxtAttributes        IVTxtAttributesW
 #define IID_IVTxtAttributes    IID_IVTxtAttributesW
 #define PIVTXTATTRIBUTES       PIVTXTATTRIBUTESW

#else
 #define IVTxtAttributes        IVTxtAttributesA
 #define IID_IVTxtAttributes    IID_IVTxtAttributesA
 #define PIVTXTATTRIBUTES       PIVTXTATTRIBUTESA

#endif // _S_UNICODE



/*
 *  IVTxtDialog
 */
#undef   INTERFACE
#define  INTERFACE   IVTxtDialogsW

// {D6469210-E095-11cd-A166-00AA004CD65C}
DEFINE_GUID(IID_IVTxtDialogsW, 0xd6469210, 0xe095, 0x11cd, 0xa1, 0x66, 0x0, 0xaa, 0x0, 0x4c, 0xd6, 0x5c);

DECLARE_INTERFACE_ (IVTxtDialogsW, IUnknown) {
   // IUnknown members
   STDMETHOD(QueryInterface)  (THIS_ REFIID riid, LPVOID FAR* ppvObj) PURE;
   STDMETHOD_(ULONG,AddRef)   (THIS) PURE;
   STDMETHOD_(ULONG,Release)  (THIS) PURE;

   // IVTxtDialogs members

   STDMETHOD (AboutDlg)       (THIS_ HWND, PCWSTR) PURE;
   STDMETHOD (LexiconDlg)     (THIS_ HWND, PCWSTR) PURE;
   STDMETHOD (GeneralDlg)     (THIS_ HWND, PCWSTR) PURE;
   STDMETHOD (TranslateDlg)   (THIS_ HWND, PCWSTR) PURE;
};

typedef IVTxtDialogsW FAR * PIVTXTDIALOGSW;


#undef   INTERFACE
#define  INTERFACE   IVTxtDialogsA

// {E8F6FA20-E095-11cd-A166-00AA004CD65C}
DEFINE_GUID(IID_IVTxtDialogsA, 0xe8f6fa20, 0xe095, 0x11cd, 0xa1, 0x66, 0x0, 0xaa, 0x0, 0x4c, 0xd6, 0x5c);

DECLARE_INTERFACE_ (IVTxtDialogsA, IUnknown) {
   // IUnknown members
   STDMETHOD(QueryInterface)  (THIS_ REFIID riid, LPVOID FAR* ppvObj) PURE;
   STDMETHOD_(ULONG,AddRef)   (THIS) PURE;
   STDMETHOD_(ULONG,Release)  (THIS) PURE;

   // IVTxtDialogs members
   STDMETHOD (AboutDlg)       (THIS_ HWND, PCSTR) PURE;
   STDMETHOD (LexiconDlg)     (THIS_ HWND, PCSTR) PURE;
   STDMETHOD (GeneralDlg)     (THIS_ HWND, PCSTR) PURE;
   STDMETHOD (TranslateDlg)   (THIS_ HWND, PCSTR) PURE;
};

typedef IVTxtDialogsA FAR * PIVTXTDIALOGSA;


#ifdef _S_UNICODE
 #define IVTxtDialogs       IVTxtDialogsW
 #define IID_IVTxtDialogs   IID_IVTxtDialogsW
 #define PIVTXTDIALOGS      PIVTXTDIALOGSW

#else
 #define IVTxtDialogs       IVTxtDialogsA
 #define IID_IVTxtDialogs   IID_IVTxtDialogsA
 #define PIVTXTDIALOGS      PIVTXTDIALOGSA

#endif // _S_UNICODE



/************************************************************************
class guids */

// {080EB9D0-E096-11cd-A166-00AA004CD65C}
// DEFINE_GUID(CLSID_VTxt, 0x80eb9d0, 0xe096, 0x11cd, 0xa1, 0x66, 0x0, 0xaa, 0x0, 0x4c, 0xd6, 0x5c);
// {F1DC95A0-0BA7-11ce-A166-00AA004CD65C}
DEFINE_GUID(CLSID_VTxt, 
0xf1dc95a0, 0xba7, 0x11ce, 0xa1, 0x66, 0x0, 0xaa, 0x0, 0x4c, 0xd6, 0x5c);


// {70618F72-D1ED-11d0-8FAC-08002BE4E62A}
DEFINE_GUID(CLSID_STGramComp, 
0x70618f72, 0xd1ed, 0x11d0, 0x8f, 0xac, 0x8, 0x0, 0x2b, 0xe4, 0xe6, 0x2a);

// {EFD0E6BA-DB5F-11d0-8FAC-08002BE4E62A}
DEFINE_GUID(CLSID_STTTSQueue, 
0xefd0e6ba, 0xdb5f, 0x11d0, 0x8f, 0xac, 0x8, 0x0, 0x2b, 0xe4, 0xe6, 0x2a);

// {B3613D9F-E26E-11d0-8FAC-08002BE4E62A}
DEFINE_GUID(CLSID_STLog, 
0xb3613d9f, 0xe26e, 0x11d0, 0x8f, 0xac, 0x8, 0x0, 0x2b, 0xe4, 0xe6, 0x2a);

// {B3613DA0-E26E-11d0-8FAC-08002BE4E62A}
DEFINE_GUID(CLSID_AudioSourceLog, 
0xb3613da0, 0xe26e, 0x11d0, 0x8f, 0xac, 0x8, 0x0, 0x2b, 0xe4, 0xe6, 0x2a);

// {2EC5A8A5-E65B-11d0-8FAC-08002BE4E62A}
DEFINE_GUID(CLSID_AudioSourceTel, 
0x2ec5a8a5, 0xe65b, 0x11d0, 0x8f, 0xac, 0x8, 0x0, 0x2b, 0xe4, 0xe6, 0x2a);

// {2EC5A8A6-E65B-11d0-8FAC-08002BE4E62A}
DEFINE_GUID(CLSID_AudioDestTel, 
0x2ec5a8a6, 0xe65b, 0x11d0, 0x8f, 0xac, 0x8, 0x0, 0x2b, 0xe4, 0xe6, 0x2a);

// {61935832-FC85-11d0-8FAE-08002BE4E62A}
DEFINE_GUID(CLSID_STRecord, 
0x61935832, 0xfc85, 0x11d0, 0x8f, 0xae, 0x8, 0x0, 0x2b, 0xe4, 0xe6, 0x2a);

/* interfaces */
// ISTRecord Interface

#undef   INTERFACE
#define  INTERFACE   ISTRecord

// {61935833-FC85-11d0-8FAE-08002BE4E62A}
DEFINE_GUID(IID_ISTRecord, 
0x61935833, 0xfc85, 0x11d0, 0x8f, 0xae, 0x8, 0x0, 0x2b, 0xe4, 0xe6, 0x2a);

typedef struct {
   DWORD       dwBetweenUtt;  // acceptable time between utterance, milliseconds
   DWORD       dwInitialBuf;  // milliseconds of audio in the initial buffer
   DWORD       dwReallocBuf;  // amount to increase a buffer every time new audio
   DWORD       dwMaxBuf;      // maximum number of milliseconds that buffer can be
   DWORD       dwNoAnswer;    // milliseconds of silence before get no-answer
} STRECORDPARAM, *PSTRECORDPARAM;

DECLARE_INTERFACE_ (ISTRecord, IUnknown) {
   // IUnknown members
   STDMETHOD(QueryInterface)  (THIS_ REFIID riid, LPVOID FAR* ppvObj) PURE;
   STDMETHOD_(ULONG,AddRef)   (THIS) PURE;
   STDMETHOD_(ULONG,Release)  (THIS) PURE;

   // ISTGramComp members
   STDMETHOD (Init) (THIS_ LPUNKNOWN, DWORD, LPUNKNOWN) PURE;
   STDMETHOD (ParamsSet) (THIS_ PSTRECORDPARAM) PURE;
   STDMETHOD (Start) (THIS) PURE;
   STDMETHOD (Stop) (THIS_ DWORD) PURE;
   STDMETHOD (GetWAV) (THIS_ PVOID*, DWORD*) PURE;
   STDMETHOD (ClearWAV) (THIS) PURE;
   };

typedef ISTRecord FAR * PISTRECORD;


// ISTRecordNotifySink Interface

#undef   INTERFACE
#define  INTERFACE   ISTRecordNotifySink

// {61935834-FC85-11d0-8FAE-08002BE4E62A}
DEFINE_GUID(IID_ISTRecordNotifySink, 
0x61935834, 0xfc85, 0x11d0, 0x8f, 0xae, 0x8, 0x0, 0x2b, 0xe4, 0xe6, 0x2a);

DECLARE_INTERFACE_ (ISTRecordNotifySink, IUnknown) {
   // IUnknown members
   STDMETHOD(QueryInterface)  (THIS_ REFIID riid, LPVOID FAR* ppvObj) PURE;
   STDMETHOD_(ULONG,AddRef)   (THIS) PURE;
   STDMETHOD_(ULONG,Release)  (THIS) PURE;

   // ISTGramComp members
   STDMETHOD (AutoStop) (THIS) PURE;
   STDMETHOD (VU) (THIS_ WORD) PURE;
   };

typedef ISTRecordNotifySink FAR * PISTRECORDNOTIFYSINK;



// ISTGramComp Interface

#undef   INTERFACE
#define  INTERFACE   ISTGramComp

// {70618F73-D1ED-11d0-8FAC-08002BE4E62A}
DEFINE_GUID(IID_ISTGramComp, 
0x70618f73, 0xd1ed, 0x11d0, 0x8f, 0xac, 0x8, 0x0, 0x2b, 0xe4, 0xe6, 0x2a);

DECLARE_INTERFACE_ (ISTGramComp, IUnknown) {
   // IUnknown members
   STDMETHOD(QueryInterface)  (THIS_ REFIID riid, LPVOID FAR* ppvObj) PURE;
   STDMETHOD_(ULONG,AddRef)   (THIS) PURE;
   STDMETHOD_(ULONG,Release)  (THIS) PURE;

   // ISTGramComp members
   STDMETHOD (FromMemory) (THIS_ PVOID, DWORD) PURE;
   STDMETHOD (FromResource) (THIS_ HINSTANCE, DWORD) PURE;
   STDMETHOD (FromFile) (THIS_ PCWSTR) PURE;
   STDMETHOD (FromStream) (THIS_ IStream *) PURE;
   STDMETHOD (Compile) (THIS_ LPWSTR*, DWORD*) PURE;
   STDMETHOD (IsCompiled) (THIS_ BOOL*) PURE;
   STDMETHOD (ToMemory) (THIS_ PVOID*, DWORD*) PURE;
   STDMETHOD (ToFile) (THIS_ PCWSTR) PURE;
   STDMETHOD (ToStream) (THIS_ IStream *) PURE;
   STDMETHOD (TextGet) (THIS_ LPWSTR*, DWORD*) PURE;
   STDMETHOD (LanguageGet) (THIS_ LANGUAGEW*) PURE;
   STDMETHOD (GrammarFormatGet) (THIS_ SRGRMFMT*) PURE;
   STDMETHOD (GrammarDataGet) (THIS_ BOOL, PVOID*, DWORD*) PURE;
   STDMETHOD (GrammarDataSet) (THIS_ PVOID, DWORD) PURE;
   STDMETHOD (AutoList) (THIS_ PISRGRAMCFGW) PURE;
   STDMETHOD (PhraseParse) (THIS_ PSRPHRASEW, DWORD *, LPWSTR *, DWORD *) PURE;
   STDMETHOD (GrammarLoad) (THIS_ LPUNKNOWN, PVOID, IID, LPUNKNOWN *) PURE;
   STDMETHOD (PhraseParseAlt) (THIS_ DWORD, DWORD *, LPWSTR *, DWORD *) PURE;
   STDMETHOD (PhraseParseRule) (THIS_ PCWSTR, PSRPHRASEW, DWORD *, LPWSTR *, DWORD *) PURE;
   };

typedef ISTGramComp FAR * PISTGRAMCOMP;



// ISTTTSQueue Interface

#undef   INTERFACE
#define  INTERFACE   ISTTTSQueue

// {EFD0E6BB-DB5F-11d0-8FAC-08002BE4E62A}
DEFINE_GUID(IID_ISTTTSQueue, 
0xefd0e6bb, 0xdb5f, 0x11d0, 0x8f, 0xac, 0x8, 0x0, 0x2b, 0xe4, 0xe6, 0x2a);

DECLARE_INTERFACE_ (ISTTTSQueue, IUnknown) {
   // IUnknown members
   STDMETHOD(QueryInterface)  (THIS_ REFIID riid, LPVOID FAR* ppvObj) PURE;
   STDMETHOD_(ULONG,AddRef)   (THIS) PURE;
   STDMETHOD_(ULONG,Release)  (THIS) PURE;

   // ISTTTSQueue members
   STDMETHOD (WaveAddFromFile) (THIS_ PCWSTR, PCWSTR) PURE;
   STDMETHOD (WaveAddFromStream) (THIS_ PCWSTR, IStream *) PURE;
   STDMETHOD (WaveAddFromMemory) (THIS_ PCWSTR, PVOID, DWORD) PURE;
   STDMETHOD (WaveAddFromResource) (THIS_ PCWSTR, HMODULE, DWORD) PURE;
   STDMETHOD (WaveAddFromList) (THIS_ PCWSTR, HMODULE) PURE;
   STDMETHOD (WaveRemove) (THIS_ PCWSTR) PURE;
   STDMETHOD (WaveAudioDestSet) (THIS_ LPUNKNOWN) PURE;
   STDMETHOD (WaveLevelSet) (THIS_ DWORD) PURE;
   STDMETHOD (WaveLevelGet) (THIS_ DWORD *) PURE;
   STDMETHOD (TTSAdd) (THIS_ PCWSTR, LPUNKNOWN) PURE;
   STDMETHOD (TTSRemove) (THIS_ PCWSTR) PURE;
   STDMETHOD (TTSGet) (THIS_ PCWSTR, LPUNKNOWN *) PURE;
   STDMETHOD (Speak) (THIS_ PCWSTR, PCWSTR, DWORD) PURE;
   STDMETHOD (TextData) (THIS_ VOICECHARSET, DWORD, SDATA, PCWSTR, DWORD) PURE;
   STDMETHOD (AudioPause) (THIS) PURE;
   STDMETHOD (AudioResume) (THIS) PURE;
   STDMETHOD (AudioReset) (THIS) PURE;
   STDMETHOD (Register) (THIS_ PVOID, REFIID) PURE;
   STDMETHOD (UseLogging) (THIS_ LPUNKNOWN) PURE;
   STDMETHOD (IsSpeaking) (THIS_ DWORD*) PURE;
   };

typedef ISTTTSQueue FAR * PISTTTSQUEUE;


// ISTTTSQueueNotifySink Interface

#undef   INTERFACE
#define  INTERFACE   ISTTTSQueueNotifySink

// {8412FA0C-DB6A-11d0-8FAC-08002BE4E62A}
DEFINE_GUID(IID_ISTTTSQueueNotifySink, 
0x8412fa0c, 0xdb6a, 0x11d0, 0x8f, 0xac, 0x8, 0x0, 0x2b, 0xe4, 0xe6, 0x2a);

DECLARE_INTERFACE_ (ISTTTSQueueNotifySink, IUnknown) {
   // IUnknown members
   STDMETHOD(QueryInterface)  (THIS_ REFIID riid, LPVOID FAR* ppvObj) PURE;
   STDMETHOD_(ULONG,AddRef)   (THIS) PURE;
   STDMETHOD_(ULONG,Release)  (THIS) PURE;

   // ISTTTSQueueNotifySink members
   STDMETHOD (Start) (THIS) PURE;
   STDMETHOD (Stop) (THIS) PURE;
   STDMETHOD (SpeakID) (THIS_ DWORD) PURE;
   STDMETHOD (Error) (THIS_ HRESULT) PURE;
   STDMETHOD (Bookmark) (THIS_ DWORD) PURE;
   };

typedef ISTTTSQueueNotifySink FAR * PISTTTSQUEUENOTIFYSINK;


// ISTLog Interface

#undef   INTERFACE
#define  INTERFACE   ISTLog

// {B3613DA1-E26E-11d0-8FAC-08002BE4E62A}
DEFINE_GUID(IID_ISTLog, 
0xb3613da1, 0xe26e, 0x11d0, 0x8f, 0xac, 0x8, 0x0, 0x2b, 0xe4, 0xe6, 0x2a);

DECLARE_INTERFACE_ (ISTLog, IUnknown) {
   // IUnknown members
   STDMETHOD(QueryInterface)  (THIS_ REFIID riid, LPVOID FAR* ppvObj) PURE;
   STDMETHOD_(ULONG,AddRef)   (THIS) PURE;
   STDMETHOD_(ULONG,Release)  (THIS) PURE;

   // ISTLog members
   STDMETHOD (ToFile) (THIS_ PCWSTR) PURE;
   STDMETHOD (ToStream) (THIS_ IStream *) PURE;
   STDMETHOD (Text) (THIS_ PCWSTR, PCWSTR, WORD) PURE;
   STDMETHOD (Data) (THIS_ PCWSTR, PCWSTR, WORD, PVOID, DWORD) PURE;
   STDMETHOD (MaxDetailSet) (THIS_ WORD) PURE;
   STDMETHOD (MaxDetailGet) (THIS_ WORD*) PURE;
   STDMETHOD (ShowOutput) (THIS_ BOOL) PURE;
   };

typedef ISTLog FAR * PISTLOG;


// IAudioSourceLog Interface

#undef   INTERFACE
#define  INTERFACE   IAudioSourceLog

// {B3613DA2-E26E-11d0-8FAC-08002BE4E62A}
DEFINE_GUID(IID_IAudioSourceLog, 
0xb3613da2, 0xe26e, 0x11d0, 0x8f, 0xac, 0x8, 0x0, 0x2b, 0xe4, 0xe6, 0x2a);

DECLARE_INTERFACE_ (IAudioSourceLog, IUnknown) {
   // IUnknown members
   STDMETHOD(QueryInterface)  (THIS_ REFIID riid, LPVOID FAR* ppvObj) PURE;
   STDMETHOD_(ULONG,AddRef)   (THIS) PURE;
   STDMETHOD_(ULONG,Release)  (THIS) PURE;

   // IAudioSourceLog members
   STDMETHOD (AudioSource) (THIS_ LPUNKNOWN) PURE;
   STDMETHOD (UseLogging) (THIS_ LPUNKNOWN) PURE;
   };

typedef IAudioSourceLog FAR * PIAUDIOSOURCELOG;



// IAudioTel Interface

#undef   INTERFACE
#define  INTERFACE   IAudioTel

// {2EC5A8A7-E65B-11d0-8FAC-08002BE4E62A}
DEFINE_GUID(IID_IAudioTel, 
0x2ec5a8a7, 0xe65b, 0x11d0, 0x8f, 0xac, 0x8, 0x0, 0x2b, 0xe4, 0xe6, 0x2a);

DECLARE_INTERFACE_ (IAudioTel, IUnknown) {
   // IUnknown members
   STDMETHOD(QueryInterface)  (THIS_ REFIID riid, LPVOID FAR* ppvObj) PURE;
   STDMETHOD_(ULONG,AddRef)   (THIS) PURE;
   STDMETHOD_(ULONG,Release)  (THIS) PURE;

   // IAudioTel members
   STDMETHOD (AudioObject) (THIS_ LPUNKNOWN) PURE;
   STDMETHOD (WaveFormatSet) (THIS_ SDATA) PURE;
   };

typedef IAudioTel FAR * PIAUDIOTEL;





/************************************************************************
External speech synthesizer - This is for using external, hardware
based speech synthesizers with SAPI */


/************************************************************************
defines */


// Synthesizer Attributes
#define  TTS_LANGUAGE         SETBIT(0)
#define  TTS_VOICE            SETBIT(1)
#define  TTS_GENDER           SETBIT(2)
#define  TTS_VOLUME           SETBIT(3)
#define  TTS_PITCH            SETBIT(4)
#define  TTS_SPEED            SETBIT(5)
#define  TTS_ABBREVIATION     SETBIT(6)
#define  TTS_PUNCTUATION      SETBIT(7)
#define  TTS_PAUSEWORD        SETBIT(8)
#define  TTS_PAUSEPHRASE      SETBIT(9)
#define  TTS_PAUSESENTENCE    SETBIT(10)
#define  TTS_SPELLING         SETBIT(11)
#define  TTS_QUALITY          SETBIT(12)
#define  TTS_FRICATION        SETBIT(13)
#define  TTS_ASPIRATION       SETBIT(14)
#define  TTS_INTONATION       SETBIT(15)


// Attribute minimums and maximums
#define  TTSATTR_MINPAUSEWORD      0x00000000
#define  TTSATTR_MAXPAUSEWORD      0xffffffff
#define  TTSATTR_MINPAUSEPHRASE    0x00000000
#define  TTSATTR_MAXPAUSEPHRASE    0xffffffff
#define  TTSATTR_MINPAUSESENTENCE  0x00000000
#define  TTSATTR_MAXPAUSESENTENCE  0xffffffff
#define  TTSATTR_MINASPIRATION     0x00000000
#define  TTSATTR_MAXASPIRATION     0xffffffff
#define  TTSATTR_MINFRICATION      0x00000000
#define  TTSATTR_MAXFRICATION      0xffffffff
#define  TTSATTR_MININTONATION     0x00000000
#define  TTSATTR_MAXINTONATION     0xffffffff


// ITTSNotifySink
#define TTSNSAC_LANGUAGE         100
#define TTSNSAC_VOICE            101
#define TTSNSAC_GENDER           102
#define TTSNSAC_ABBREVIATION     103
#define TTSNSAC_PUNCTUATION      104
#define TTSNSAC_PAUSEWORD        105
#define TTSNSAC_PAUSEPHRASE      106
#define TTSNSAC_PAUSESENTENCE    107
#define TTSNSAC_SPELLING         108
#define TTSNSAC_QUALITY          109
#define TTSNSAC_FRICATION        110
#define TTSNSAC_ASPIRATION       111
#define TTSNSAC_INTONATION       112


// Interfaces
#define TTSI_ITTSEXTERNALSYNTHESIZER            SETBIT(4)


// TTSERR Result codes
#define TTSERR_SYNTHESIZERBUSY                  TTSERROR(100)
#define TTSERR_ALREADYDISPLAYED                 TTSERROR(101)
#define TTSERR_INVALIDATTRIB                    TTSERROR(102)
#define TTSERR_SYNTHESIZERACCESSERROR           TTSERROR(103)
#define TTSERR_DRIVERERROR                      TTSERROR(104)
#define TTSERR_UNRECOVERABLEERROR               TTSERROR(105)
#define TTSERR_DRIVERACCESSERROR                TTSERROR(106)
#define TTSERR_BUFFERTOOSMALL                   TTSERROR(107)
#define TTSERR_DRIVERNOTFOUND                   TTSERROR(108)
#define TTSERR_CANNOTREGISTER                   TTSERROR(109)
#define TTSERR_LANGUAGENOTSUPPORTED             TTSERROR(110)


/************************************************************************
Class IDs */

// {479DFDCB-01A8-11d1-A7A5-00C04FBBE8AB}
DEFINE_GUID(CLSID_SynthAudioDest, 
0x479dfdcb, 0x1a8, 0x11d1, 0xa7, 0xa5, 0x0, 0xc0, 0x4f, 0xbb, 0xe8, 0xab);

// {26C328C1-F26F-11d0-9DFF-006097622195}
DEFINE_GUID(CLSID_SynthEngine, 
0x26c328c1, 0xf26f, 0x11d0, 0x9d, 0xff, 0x0, 0x60, 0x97, 0x62, 0x21, 0x95);

// {AC8FCCA1-FFD7-11d0-9E22-006097622195}
DEFINE_GUID(CLSID_SynthEngineEnumerator, 
0xac8fcca1, 0xffd7, 0x11d0, 0x9e, 0x22, 0x0, 0x60, 0x97, 0x62, 0x21, 0x95);



/************************************************************************
typedefs */

// Definition of Preset
typedef struct
{
   LANGID          Language;
   DWORD           Voice;
   DWORD           Gender;

   DWORD           Volume;
   DWORD           Pitch;
   DWORD           Speed;

   DWORD           Abbreviation;
   DWORD           Punctuation;
   DWORD           PauseWord;
   DWORD           PausePhrase;
   DWORD           PauseSentence;

   DWORD           Spelling;
   DWORD           Quality;

   DWORD           Frication;
   DWORD           Aspiration;
   DWORD           Intonation;
} 
TTSPRESET, *PTTSPRESET;


/************************************************************************
interfaces */


/*
 * ITTSExternalSynthesizer
 */


#undef   INTERFACE
#define  INTERFACE   ITTSExternalSynthesizerW

// {A3F8B9C0-2010-11d1-A306-006097622195}
DEFINE_GUID(IID_ITTSExternalSynthesizerW, 
0xa3f8b9c0, 0x2010, 0x11d1, 0xa3, 0x6, 0x0, 0x60, 0x97, 0x62, 0x21, 0x95);

DECLARE_INTERFACE_(ITTSExternalSynthesizerW, IUnknown)
{
   STDMETHOD (QueryInterface) (THIS_ REFIID riid, LPVOID  * ppvObj) PURE;
   STDMETHOD_(ULONG,AddRef)   (THIS) PURE;
   STDMETHOD_(ULONG,Release)  (THIS) PURE;

   STDMETHOD (AttributeGet) (THIS_ DWORD dwAttribute, DWORD *pdwValue)   PURE;
   STDMETHOD (AttributeSet) (THIS_ DWORD dwAttribute, DWORD dwValue)     PURE;
   STDMETHOD (PresetGet)    (THIS_ PTTSPRESET pPreset, DWORD *pdwResult) PURE;
   STDMETHOD (PresetSet)    (THIS_ TTSPRESET Preset, DWORD *pdwResult)   PURE;

   STDMETHOD (GetAttributeElements) (THIS_ DWORD dwAttr, DWORD *pdwNumElems) PURE;
   STDMETHOD (GetAttributeRange)    (THIS_ DWORD dwAttr, DWORD *pdwMin, DWORD *pdwMax) PURE;
   STDMETHOD (GetAttributeValue)    (THIS_ DWORD dwAttr, DWORD dwIndex, DWORD *pdwValue, PWSTR szText, DWORD dwMaxChars) PURE;
   STDMETHOD (GetAttributeIndex)    (THIS_ DWORD dwAttr, DWORD dwValue, DWORD *pdwIndex, PWSTR szText, DWORD dwMaxChars) PURE;

   STDMETHOD (AudioFlush) (THIS) PURE;

   STDMETHOD (TextDataInterrupt)(THIS_ VOICECHARSET eCharacterSet, DWORD dwFlags, SDATA dText, PVOID pNotifyInterface, IID IIDNotifyInterface) PURE;
   STDMETHOD (TextDataCritical) (THIS_ SDATA) PURE;

   STDMETHOD (MiscellaneousDlg) (THIS_ HWND hWndParent, PCWSTR pszTitle) PURE;
};
typedef ITTSExternalSynthesizerW  *PITTSEXTERNALSYNTHESIZERW;


#undef   INTERFACE
#define  INTERFACE   ITTSExternalSynthesizerA

// {3D11C963-F210-11d0-A788-00C04FBBE8AB}
DEFINE_GUID(IID_ITTSExternalSynthesizerA,
0x3d11c963, 0xf210, 0x11d0, 0xa7, 0x88, 0x0, 0xc0, 0x4f, 0xbb, 0xe8, 0xab);

DECLARE_INTERFACE_(ITTSExternalSynthesizerA, IUnknown)
{
   STDMETHOD (QueryInterface) (THIS_ REFIID riid, LPVOID  * ppvObj) PURE;
   STDMETHOD_(ULONG,AddRef)   (THIS) PURE;
   STDMETHOD_(ULONG,Release)  (THIS) PURE;

   STDMETHOD (AttributeGet) (THIS_ DWORD dwAttribute, DWORD *pdwValue)   PURE;
   STDMETHOD (AttributeSet) (THIS_ DWORD dwAttribute, DWORD dwValue)     PURE;
   STDMETHOD (PresetGet)    (THIS_ PTTSPRESET pPreset, DWORD *pdwResult) PURE;
   STDMETHOD (PresetSet)    (THIS_ TTSPRESET Preset, DWORD *pdwResult)   PURE;

   STDMETHOD (GetAttributeElements) (THIS_ DWORD dwAttr, DWORD *pdwNumElems) PURE;
   STDMETHOD (GetAttributeRange)    (THIS_ DWORD dwAttr, DWORD *pdwMin, DWORD *pdwMax) PURE;
   STDMETHOD (GetAttributeValue)    (THIS_ DWORD dwAttr, DWORD dwIndex, DWORD *pdwValue, PSTR szText, DWORD dwMaxChars) PURE;
   STDMETHOD (GetAttributeIndex)    (THIS_ DWORD dwAttr, DWORD dwValue, DWORD *pdwIndex, PSTR szText, DWORD dwMaxChars) PURE;

   STDMETHOD (AudioFlush) (THIS) PURE;

   STDMETHOD (TextDataInterrupt)(THIS_ VOICECHARSET eCharacterSet, DWORD dwFlags, SDATA dText, PVOID pNotifyInterface, IID IIDNotifyInterface) PURE;
   STDMETHOD (TextDataCritical) (THIS_ SDATA) PURE;

   STDMETHOD (MiscellaneousDlg) (THIS_ HWND hWndParent, PCSTR pszTitle) PURE;
};
typedef ITTSExternalSynthesizerA  *PITTSEXTERNALSYNTHESIZERA;


#ifdef _S_UNICODE
 #define ITTSExternalSynthesizer            ITTSExternalSynthesizerW
 #define IID_ITTSExternalSynthesizer        IID_ITTSExternalSynthesizerW
 #define PITTSEXTERNALSYNTHESIZER           PITTSEXTERNALSYNTHESIZERW

#else
 #define ITTSExternalSynthesizer           ITTSExternalSynthesizerA
 #define IID_ITTSExternalSynthesizer       IID_ITTSExternalSynthesizerA
 #define PITTSEXTERNALSYNTHESIZER          PITTSEXTERNALSYNTHESIZERA

#endif   // _S_UNICODE



/*
 * IAudioExternalSynthesizer
 */

#undef   INTERFACE
#define  INTERFACE   IAudioExternalSynthesizer

// {33E0E76B-70CE-11d1-A826-00C04FBBE8AB}
DEFINE_GUID(IID_IAudioExternalSynthesizer, 
0x33e0e76b, 0x70ce, 0x11d1, 0xa8, 0x26, 0x0, 0xc0, 0x4f, 0xbb, 0xe8, 0xab);

DECLARE_INTERFACE_(IAudioExternalSynthesizer, IUnknown)
{
   STDMETHOD (QueryInterface) (THIS_ REFIID riid, LPVOID  *ppvObj) PURE;
   STDMETHOD_(ULONG,AddRef)   (THIS) PURE;
   STDMETHOD_(ULONG,Release)  (THIS) PURE;
};
typedef IAudioExternalSynthesizer *PIAUDIOEXTERNALSYNTHESIZER;


#endif    // _SPEECH_
