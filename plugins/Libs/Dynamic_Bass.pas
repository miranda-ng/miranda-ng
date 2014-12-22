{.$DEFINE CHECK_PROC}
{
  BASS 2.4 Delphi unit (dynamic)
  Copyright (c) 1999-2014 Un4seen Developments Ltd.

  See the BASS.CHM file for more detailed documentation

  How to install
  ----------------
  Copy DYNAMIC_BASS.PAS to the \LIB subdirectory of your Delphi path or your project dir

  Call Load_BASSDLL (eg. in FormCreate) to load BASS before using any functions, and
  Unload_BASSDLL (eg. in FormDestory) to unload it when you're done.

  NOTE: Delphi users should use the BASS_UNICODE flag where possible
}

unit Dynamic_Bass;

interface

uses
  Windows;

type
  pBASSRegRec = ^tBASSRegRec;
  tBASSRegRec = record
    next: pBASSRegRec;
    Init: function: bool;
  end;
const
  BASSRegRec:pBASSRegRec = nil;

const
  BASSVERSION     = $204; // API version
  BASSVERSIONTEXT = '2.4';

  // Use these to test for error from functions that return a DWORD or QWORD
  DW_ERROR = Cardinal(-1); // -1 (DWORD)
  QW_ERROR = Int64(-1);    // -1 (QWORD)

  // Error codes returned by BASS_ErrorGetCode()
  BASS_OK                 = 0;    // all is OK
  BASS_ERROR_MEM          = 1;    // memory error
  BASS_ERROR_FILEOPEN     = 2;    // can't open the file
  BASS_ERROR_DRIVER       = 3;    // can't find a free sound driver
  BASS_ERROR_BUFLOST      = 4;    // the sample buffer was lost
  BASS_ERROR_HANDLE       = 5;    // invalid handle
  BASS_ERROR_FORMAT       = 6;    // unsupported sample format
  BASS_ERROR_POSITION     = 7;    // invalid position
  BASS_ERROR_INIT         = 8;    // BASS_Init has not been successfully called
  BASS_ERROR_START        = 9;    // BASS_Start has not been successfully called

  BASS_ERROR_ALREADY      = 14;   // already initialized/paused/whatever

  BASS_ERROR_NOCHAN       = 18;   // can't get a free channel
  BASS_ERROR_ILLTYPE      = 19;   // an illegal type was specified
  BASS_ERROR_ILLPARAM     = 20;   // an illegal parameter was specified
  BASS_ERROR_NO3D         = 21;   // no 3D support
  BASS_ERROR_NOEAX        = 22;   // no EAX support
  BASS_ERROR_DEVICE       = 23;   // illegal device number
  BASS_ERROR_NOPLAY       = 24;   // not playing
  BASS_ERROR_FREQ         = 25;   // illegal sample rate

  BASS_ERROR_NOTFILE      = 27;   // the stream is not a file stream

  BASS_ERROR_NOHW         = 29;   // no hardware voices available

  BASS_ERROR_EMPTY        = 31;   // the MOD music has no sequence data
  BASS_ERROR_NONET        = 32;   // no internet connection could be opened
  BASS_ERROR_CREATE       = 33;   // couldn't create the file
  BASS_ERROR_NOFX         = 34;   // effects are not available

  BASS_ERROR_NOTAVAIL     = 37;   // requested data is not available
  BASS_ERROR_DECODE       = 38;   // the channel is/isn't a "decoding channel"
  BASS_ERROR_DX           = 39;   // a sufficient DirectX version is not installed
  BASS_ERROR_TIMEOUT      = 40;   // connection timedout
  BASS_ERROR_FILEFORM     = 41;   // unsupported file format
  BASS_ERROR_SPEAKER      = 42;   // unavailable speaker
  BASS_ERROR_VERSION      = 43;   // invalid BASS version (used by add-ons)
  BASS_ERROR_CODEC        = 44;   // codec is not available/supported
  BASS_ERROR_ENDED        = 45;   // the channel/file has ended
  BASS_ERROR_BUSY         = 46;   // the device is busy
  BASS_ERROR_UNKNOWN      = -1;   // some other mystery problem

  BASS_ERROR_MAXNUMBER    = 46;   // custom, just to have max error number

const
  BASS_ERRORS: array [0..46] of pAnsiChar = (
  {BASS_OK            } 'all is OK',
  {BASS_ERROR_MEM     } 'memory error',
  {BASS_ERROR_FILEOPEN} 'can''t open the file',
  {BASS_ERROR_DRIVER  } 'can''t find a free sound driver',
  {BASS_ERROR_BUFLOST } 'the sample buffer was lost',
  {BASS_ERROR_HANDLE  } 'invalid handle',
  {BASS_ERROR_FORMAT  } 'unsupported sample format',
  {BASS_ERROR_POSITION} 'invalid position',
  {BASS_ERROR_INIT    } 'BASS_Init has not been successfully called',
  {BASS_ERROR_START   } 'BASS_Start has not been successfully called',
  nil,
  nil,
  nil,
  nil,
  {BASS_ERROR_ALREADY } 'already initialized/paused/whatever',
  nil,
  nil,
  nil,
  {BASS_ERROR_NOCHAN  } 'can''t get a free channel',
  {BASS_ERROR_ILLTYPE } 'an illegal type was specified',
  {BASS_ERROR_ILLPARAM} 'an illegal parameter was specified',
  {BASS_ERROR_NO3D    } 'no 3D support',
  {BASS_ERROR_NOEAX   } 'no EAX support',
  {BASS_ERROR_DEVICE  } 'illegal device number',
  {BASS_ERROR_NOPLAY  } 'not playing',
  {BASS_ERROR_FREQ    } 'illegal sample rate',
  nil,
  {BASS_ERROR_NOTFILE } 'the stream is not a file stream',
  nil,
  {BASS_ERROR_NOHW    } 'no hardware voices available',
  nil,
  {BASS_ERROR_EMPTY   } 'the MOD music has no sequence data',
  {BASS_ERROR_NONET   } 'no internet connection could be opened',
  {BASS_ERROR_CREATE  } 'couldn''t create the file',
  {BASS_ERROR_NOFX    } 'effects are not enabled',
  nil,
  nil,
  {BASS_ERROR_NOTAVAIL} 'requested data is not available',
  {BASS_ERROR_DECODE  } 'the channel is a "decoding channel"',
  {BASS_ERROR_DX      } 'a sufficient DirectX version is not installed',
  {BASS_ERROR_TIMEOUT } 'connection timedout',
  {BASS_ERROR_FILEFORM} 'unsupported file format',
  {BASS_ERROR_SPEAKER } 'unavailable speaker',
  {BASS_ERROR_VERSION } 'invalid BASS version (used by add-ons)',
  {BASS_ERROR_CODEC   } 'codec is not available/supported',
  {BASS_ERROR_ENDED   } 'the channel/file has ended',
  {BASS_ERROR_BUSY    } 'the device is busy');

  // BASS_SetConfig options
  BASS_CONFIG_BUFFER           = 0;
  BASS_CONFIG_UPDATEPERIOD     = 1;
  BASS_CONFIG_GVOL_SAMPLE      = 4;
  BASS_CONFIG_GVOL_STREAM      = 5;
  BASS_CONFIG_GVOL_MUSIC       = 6;
  BASS_CONFIG_CURVE_VOL        = 7;
  BASS_CONFIG_CURVE_PAN        = 8;
  BASS_CONFIG_FLOATDSP         = 9;
  BASS_CONFIG_3DALGORITHM      = 10;
  BASS_CONFIG_NET_TIMEOUT      = 11;
  BASS_CONFIG_NET_BUFFER       = 12;
  BASS_CONFIG_PAUSE_NOPLAY     = 13;
  BASS_CONFIG_NET_PREBUF       = 15;
  BASS_CONFIG_NET_PASSIVE      = 18;
  BASS_CONFIG_REC_BUFFER       = 19;
  BASS_CONFIG_NET_PLAYLIST     = 21;
  BASS_CONFIG_MUSIC_VIRTUAL    = 22;
  BASS_CONFIG_VERIFY           = 23;
  BASS_CONFIG_UPDATETHREADS    = 24;
  BASS_CONFIG_DEV_BUFFER       = 27;
  BASS_CONFIG_DEV_DEFAULT      = 36;
  BASS_CONFIG_NET_READTIMEOUT  = 37;
  BASS_CONFIG_VISTA_SPEAKERS   = 38;
  BASS_CONFIG_IOS_SPEAKER      = 39;
  BASS_CONFIG_HANDLES          = 41;
  BASS_CONFIG_UNICODE          = 42;
  BASS_CONFIG_SRC              = 43;
  BASS_CONFIG_SRC_SAMPLE       = 44;
  BASS_CONFIG_ASYNCFILE_BUFFER = 45;
  BASS_CONFIG_OGG_PRESCAN      = 47;
  BASS_CONFIG_MF_VIDEO         = 48;
  BASS_CONFIG_AIRPLAY          = 49;
  BASS_CONFIG_DEV_NONSTOP      = 50;
  BASS_CONFIG_IOS_NOCATEGORY   = 51;
  BASS_CONFIG_VERIFY_NET       = 52;

  // BASS_SetConfigPtr options
  BASS_CONFIG_NET_AGENT     = 16;
  BASS_CONFIG_NET_PROXY     = 17;

  // Initialization flags
  BASS_DEVICE_8BITS       = 1;     // use 8 bit resolution, else 16 bit
  BASS_DEVICE_MONO        = 2;     // use mono, else stereo
  BASS_DEVICE_3D          = 4;     // enable 3D functionality
  BASS_DEVICE_LATENCY     = $100;  // calculate device latency (BASS_INFO struct)
  BASS_DEVICE_CPSPEAKERS  = $400;  // detect speakers via Windows control panel
  BASS_DEVICE_SPEAKERS    = $800;  // force enabling of speaker assignment
  BASS_DEVICE_NOSPEAKER   = $1000; // ignore speaker arrangement
  BASS_DEVICE_DMIX        = $2000; // use ALSA "dmix" plugin
  BASS_DEVICE_FREQ        = $4000; // set device sample rate

  // DirectSound interfaces (for use with BASS_GetDSoundObject)
  BASS_OBJECT_DS          = 1;   // IDirectSound
  BASS_OBJECT_DS3DL       = 2;   // IDirectSound3DListener

  // BASS_DEVICEINFO flags
  BASS_DEVICE_ENABLED     = 1;
  BASS_DEVICE_DEFAULT     = 2;
  BASS_DEVICE_INIT        = 4;

  BASS_DEVICE_TYPE_MASK        = $FF000000;
  BASS_DEVICE_TYPE_NETWORK     = $01000000;
  BASS_DEVICE_TYPE_SPEAKERS    = $02000000;
  BASS_DEVICE_TYPE_LINE        = $03000000;
  BASS_DEVICE_TYPE_HEADPHONES  = $04000000;
  BASS_DEVICE_TYPE_MICROPHONE  = $05000000;
  BASS_DEVICE_TYPE_HEADSET     = $06000000;
  BASS_DEVICE_TYPE_HANDSET     = $07000000;
  BASS_DEVICE_TYPE_DIGITAL     = $08000000;
  BASS_DEVICE_TYPE_SPDIF       = $09000000;
  BASS_DEVICE_TYPE_HDMI        = $0A000000;
  BASS_DEVICE_TYPE_DISPLAYPORT = $40000000;

  // BASS_GetDeviceInfo flags
  BASS_DEVICES_AIRPLAY         = $1000000;

  // BASS_INFO flags (from DSOUND.H)
  DSCAPS_CONTINUOUSRATE   = $00000010;     // supports all sample rates between min/maxrate
  DSCAPS_EMULDRIVER       = $00000020;     // device does NOT have hardware DirectSound support
  DSCAPS_CERTIFIED        = $00000040;     // device driver has been certified by Microsoft
  DSCAPS_SECONDARYMONO    = $00000100;     // mono
  DSCAPS_SECONDARYSTEREO  = $00000200;     // stereo
  DSCAPS_SECONDARY8BIT    = $00000400;     // 8 bit
  DSCAPS_SECONDARY16BIT   = $00000800;     // 16 bit

  // BASS_RECORDINFO flags (from DSOUND.H)
  DSCCAPS_EMULDRIVER = DSCAPS_EMULDRIVER;  // device does NOT have hardware DirectSound recording support
  DSCCAPS_CERTIFIED = DSCAPS_CERTIFIED;    // device driver has been certified by Microsoft

  // defines for formats field of BASS_RECORDINFO (from MMSYSTEM.H)
  WAVE_FORMAT_1M08       = $00000001;      // 11.025 kHz, Mono,   8-bit
  WAVE_FORMAT_1S08       = $00000002;      // 11.025 kHz, Stereo, 8-bit
  WAVE_FORMAT_1M16       = $00000004;      // 11.025 kHz, Mono,   16-bit
  WAVE_FORMAT_1S16       = $00000008;      // 11.025 kHz, Stereo, 16-bit
  WAVE_FORMAT_2M08       = $00000010;      // 22.05  kHz, Mono,   8-bit
  WAVE_FORMAT_2S08       = $00000020;      // 22.05  kHz, Stereo, 8-bit
  WAVE_FORMAT_2M16       = $00000040;      // 22.05  kHz, Mono,   16-bit
  WAVE_FORMAT_2S16       = $00000080;      // 22.05  kHz, Stereo, 16-bit
  WAVE_FORMAT_4M08       = $00000100;      // 44.1   kHz, Mono,   8-bit
  WAVE_FORMAT_4S08       = $00000200;      // 44.1   kHz, Stereo, 8-bit
  WAVE_FORMAT_4M16       = $00000400;      // 44.1   kHz, Mono,   16-bit
  WAVE_FORMAT_4S16       = $00000800;      // 44.1   kHz, Stereo, 16-bit

  BASS_SAMPLE_8BITS       = 1;   // 8 bit
  BASS_SAMPLE_FLOAT       = 256; // 32-bit floating-point
  BASS_SAMPLE_MONO        = 2;   // mono
  BASS_SAMPLE_LOOP        = 4;   // looped
  BASS_SAMPLE_3D          = 8;   // 3D functionality
  BASS_SAMPLE_SOFTWARE    = 16;  // not using hardware mixing
  BASS_SAMPLE_MUTEMAX     = 32;  // mute at max distance (3D only)
  BASS_SAMPLE_VAM         = 64;  // DX7 voice allocation & management
  BASS_SAMPLE_FX          = 128; // old implementation of DX8 effects
  BASS_SAMPLE_OVER_VOL    = $10000; // override lowest volume
  BASS_SAMPLE_OVER_POS    = $20000; // override longest playing
  BASS_SAMPLE_OVER_DIST   = $30000; // override furthest from listener (3D only)

  BASS_STREAM_PRESCAN     = $20000; // enable pin-point seeking/length (MP3/MP2/MP1)
  BASS_MP3_SETPOS         = BASS_STREAM_PRESCAN;
  BASS_STREAM_AUTOFREE	  = $40000; // automatically free the stream when it stop/ends
  BASS_STREAM_RESTRATE	  = $80000; // restrict the download rate of internet file streams
  BASS_STREAM_BLOCK       = $100000;// download/play internet file stream in small blocks
  BASS_STREAM_DECODE      = $200000;// don't play the stream, only decode (BASS_ChannelGetData)
  BASS_STREAM_STATUS      = $800000;// give server status info (HTTP/ICY tags) in DOWNLOADPROC

  BASS_MUSIC_FLOAT        = BASS_SAMPLE_FLOAT;
  BASS_MUSIC_MONO         = BASS_SAMPLE_MONO;
  BASS_MUSIC_LOOP         = BASS_SAMPLE_LOOP;
  BASS_MUSIC_3D           = BASS_SAMPLE_3D;
  BASS_MUSIC_FX           = BASS_SAMPLE_FX;
  BASS_MUSIC_AUTOFREE     = BASS_STREAM_AUTOFREE;
  BASS_MUSIC_DECODE       = BASS_STREAM_DECODE;
  BASS_MUSIC_PRESCAN      = BASS_STREAM_PRESCAN; // calculate playback length
  BASS_MUSIC_CALCLEN      = BASS_MUSIC_PRESCAN;
  BASS_MUSIC_RAMP         = $200;  // normal ramping
  BASS_MUSIC_RAMPS        = $400;  // sensitive ramping
  BASS_MUSIC_SURROUND     = $800;  // surround sound
  BASS_MUSIC_SURROUND2    = $1000; // surround sound (mode 2)
  BASS_MUSIC_FT2MOD       = $2000; // play .MOD as FastTracker 2 does
  BASS_MUSIC_PT1MOD       = $4000; // play .MOD as ProTracker 1 does
  BASS_MUSIC_NONINTER     = $10000; // non-interpolated sample mixing
  BASS_MUSIC_SINCINTER    = $800000; // sinc interpolated sample mixing
  BASS_MUSIC_POSRESET     = $8000; // stop all notes when moving position
  BASS_MUSIC_POSRESETEX   = $400000; // stop all notes and reset bmp/etc when moving position
  BASS_MUSIC_STOPBACK     = $80000; // stop the music on a backwards jump effect
  BASS_MUSIC_NOSAMPLE     = $100000; // don't load the samples

  // Speaker assignment flags
  BASS_SPEAKER_FRONT      = $1000000;  // front speakers
  BASS_SPEAKER_REAR       = $2000000;  // rear/side speakers
  BASS_SPEAKER_CENLFE     = $3000000;  // center & LFE speakers (5.1)
  BASS_SPEAKER_REAR2      = $4000000;  // rear center speakers (7.1)
  BASS_SPEAKER_LEFT       = $10000000; // modifier: left
  BASS_SPEAKER_RIGHT      = $20000000; // modifier: right
  BASS_SPEAKER_FRONTLEFT  = BASS_SPEAKER_FRONT  or BASS_SPEAKER_LEFT;
  BASS_SPEAKER_FRONTRIGHT = BASS_SPEAKER_FRONT  or BASS_SPEAKER_RIGHT;
  BASS_SPEAKER_REARLEFT   = BASS_SPEAKER_REAR   or BASS_SPEAKER_LEFT;
  BASS_SPEAKER_REARRIGHT  = BASS_SPEAKER_REAR   or BASS_SPEAKER_RIGHT;
  BASS_SPEAKER_CENTER     = BASS_SPEAKER_CENLFE or BASS_SPEAKER_LEFT;
  BASS_SPEAKER_LFE        = BASS_SPEAKER_CENLFE or BASS_SPEAKER_RIGHT;
  BASS_SPEAKER_REAR2LEFT  = BASS_SPEAKER_REAR2  or BASS_SPEAKER_LEFT;
  BASS_SPEAKER_REAR2RIGHT = BASS_SPEAKER_REAR2  or BASS_SPEAKER_RIGHT;

  BASS_ASYNCFILE          = $40000000;
  BASS_UNICODE            = $80000000;

  BASS_RECORD_PAUSE       = $8000; // start recording paused

  // DX7 voice allocation & management flags
  BASS_VAM_HARDWARE       = 1;
  BASS_VAM_SOFTWARE       = 2;
  BASS_VAM_TERM_TIME      = 4;
  BASS_VAM_TERM_DIST      = 8;
  BASS_VAM_TERM_PRIO      = 16;

  // BASS_CHANNELINFO types
  BASS_CTYPE_SAMPLE           = 1;
  BASS_CTYPE_RECORD           = 2;
  BASS_CTYPE_STREAM           = $10000;
  BASS_CTYPE_STREAM_OGG       = $10002;
  BASS_CTYPE_STREAM_MP1       = $10003;
  BASS_CTYPE_STREAM_MP2       = $10004;
  BASS_CTYPE_STREAM_MP3       = $10005;
  BASS_CTYPE_STREAM_AIFF      = $10006;
  BASS_CTYPE_STREAM_WAV       = $40000; // WAVE flag, LOWORD=codec
  BASS_CTYPE_STREAM_WAV_PCM   = $50001;
  BASS_CTYPE_STREAM_WAV_FLOAT = $50003;
  BASS_CTYPE_MUSIC_MOD        = $20000;
  BASS_CTYPE_MUSIC_MTM        = $20001;
  BASS_CTYPE_MUSIC_S3M        = $20002;
  BASS_CTYPE_MUSIC_XM         = $20003;
  BASS_CTYPE_MUSIC_IT         = $20004;
  BASS_CTYPE_MUSIC_MO3        = $00100; // MO3 flag

  // 3D channel modes
  BASS_3DMODE_NORMAL      = 0; // normal 3D processing
  BASS_3DMODE_RELATIVE    = 1; // position is relative to the listener
  BASS_3DMODE_OFF         = 2; // no 3D processing

  // software 3D mixing algorithms (used with BASS_CONFIG_3DALGORITHM)
  BASS_3DALG_DEFAULT      = 0;
  BASS_3DALG_OFF          = 1;
  BASS_3DALG_FULL         = 2;
  BASS_3DALG_LIGHT        = 3;

  // EAX environments, use with BASS_SetEAXParameters
  EAX_ENVIRONMENT_GENERIC           = 0;
  EAX_ENVIRONMENT_PADDEDCELL        = 1;
  EAX_ENVIRONMENT_ROOM              = 2;
  EAX_ENVIRONMENT_BATHROOM          = 3;
  EAX_ENVIRONMENT_LIVINGROOM        = 4;
  EAX_ENVIRONMENT_STONEROOM         = 5;
  EAX_ENVIRONMENT_AUDITORIUM        = 6;
  EAX_ENVIRONMENT_CONCERTHALL       = 7;
  EAX_ENVIRONMENT_CAVE              = 8;
  EAX_ENVIRONMENT_ARENA             = 9;
  EAX_ENVIRONMENT_HANGAR            = 10;
  EAX_ENVIRONMENT_CARPETEDHALLWAY   = 11;
  EAX_ENVIRONMENT_HALLWAY           = 12;
  EAX_ENVIRONMENT_STONECORRIDOR     = 13;
  EAX_ENVIRONMENT_ALLEY             = 14;
  EAX_ENVIRONMENT_FOREST            = 15;
  EAX_ENVIRONMENT_CITY              = 16;
  EAX_ENVIRONMENT_MOUNTAINS         = 17;
  EAX_ENVIRONMENT_QUARRY            = 18;
  EAX_ENVIRONMENT_PLAIN             = 19;
  EAX_ENVIRONMENT_PARKINGLOT        = 20;
  EAX_ENVIRONMENT_SEWERPIPE         = 21;
  EAX_ENVIRONMENT_UNDERWATER        = 22;
  EAX_ENVIRONMENT_DRUGGED           = 23;
  EAX_ENVIRONMENT_DIZZY             = 24;
  EAX_ENVIRONMENT_PSYCHOTIC         = 25;
  // total number of environments
  EAX_ENVIRONMENT_COUNT             = 26;

  BASS_STREAMPROC_END = $80000000; // end of user stream flag


  // BASS_StreamCreateFileUser file systems
  STREAMFILE_NOBUFFER     = 0;
  STREAMFILE_BUFFER       = 1;
  STREAMFILE_BUFFERPUSH   = 2;

  // BASS_StreamPutFileData options
  BASS_FILEDATA_END       = 0; // end & close the file

  // BASS_StreamGetFilePosition modes
  BASS_FILEPOS_CURRENT    = 0;
  BASS_FILEPOS_DECODE     = BASS_FILEPOS_CURRENT;
  BASS_FILEPOS_DOWNLOAD   = 1;
  BASS_FILEPOS_END        = 2;
  BASS_FILEPOS_START      = 3;
  BASS_FILEPOS_CONNECTED  = 4;
  BASS_FILEPOS_BUFFER     = 5;
  BASS_FILEPOS_SOCKET     = 6;
  BASS_FILEPOS_ASYNCBUF   = 7;
  BASS_FILEPOS_SIZE       = 8;

  // BASS_ChannelSetSync types
  BASS_SYNC_POS           = 0;
  BASS_SYNC_END           = 2;
  BASS_SYNC_META          = 4;
  BASS_SYNC_SLIDE         = 5;
  BASS_SYNC_STALL         = 6;
  BASS_SYNC_DOWNLOAD      = 7;
  BASS_SYNC_FREE          = 8;
  BASS_SYNC_SETPOS        = 11;
  BASS_SYNC_MUSICPOS      = 10;
  BASS_SYNC_MUSICINST     = 1;
  BASS_SYNC_MUSICFX       = 3;
  BASS_SYNC_OGG_CHANGE    = 12;
  BASS_SYNC_MIXTIME       = $40000000; // FLAG: sync at mixtime, else at playtime
  BASS_SYNC_ONETIME       = $80000000; // FLAG: sync only once, else continuously

  // BASS_ChannelIsActive return values
  BASS_ACTIVE_STOPPED = 0;
  BASS_ACTIVE_PLAYING = 1;
  BASS_ACTIVE_STALLED = 2;
  BASS_ACTIVE_PAUSED  = 3;

  // Channel attributes
  BASS_ATTRIB_FREQ                  = 1;
  BASS_ATTRIB_VOL                   = 2;
  BASS_ATTRIB_PAN                   = 3;
  BASS_ATTRIB_EAXMIX                = 4;
  BASS_ATTRIB_NOBUFFER              = 5;
  BASS_ATTRIB_VBR                   = 6;
  BASS_ATTRIB_CPU                   = 7;
  BASS_ATTRIB_SRC                   = 8;
  BASS_ATTRIB_NET_RESUME            = 9;
  BASS_ATTRIB_SCANINFO              = 10;
  BASS_ATTRIB_MUSIC_AMPLIFY         = $100;
  BASS_ATTRIB_MUSIC_PANSEP          = $101;
  BASS_ATTRIB_MUSIC_PSCALER         = $102;
  BASS_ATTRIB_MUSIC_BPM             = $103;
  BASS_ATTRIB_MUSIC_SPEED           = $104;
  BASS_ATTRIB_MUSIC_VOL_GLOBAL      = $105;
  BASS_ATTRIB_MUSIC_ACTIVE          = $106;
  BASS_ATTRIB_MUSIC_VOL_CHAN        = $200; // + channel #
  BASS_ATTRIB_MUSIC_VOL_INST        = $300; // + instrument #

  // BASS_ChannelGetData flags
  BASS_DATA_AVAILABLE      = 0;         // query how much data is buffered
  BASS_DATA_FIXED          = $20000000; // flag: return 8.24 fixed-point data
  BASS_DATA_FLOAT          = $40000000; // flag: return floating-point sample data
  BASS_DATA_FFT256         = $80000000; // 256 sample FFT
  BASS_DATA_FFT512         = $80000001; // 512 FFT
  BASS_DATA_FFT1024        = $80000002; // 1024 FFT
  BASS_DATA_FFT2048        = $80000003; // 2048 FFT
  BASS_DATA_FFT4096        = $80000004; // 4096 FFT
  BASS_DATA_FFT8192        = $80000005; // 8192 FFT
  BASS_DATA_FFT16384       = $80000006; // 16384 FFT
  BASS_DATA_FFT_INDIVIDUAL = $10;       // FFT flag: FFT for each channel, else all combined
  BASS_DATA_FFT_NOWINDOW   = $20;       // FFT flag: no Hanning window
  BASS_DATA_FFT_REMOVEDC   = $40;       // FFT flag: pre-remove DC bias
  BASS_DATA_FFT_COMPLEX    = $80;       // FFT flag: return complex data


  // BASS_ChannelGetTags types : what's returned
  BASS_TAG_ID3           = 0;      // ID3v1 tags : TAG_ID3 structure
  BASS_TAG_ID3V2         = 1;      // ID3v2 tags : variable length block
  BASS_TAG_OGG           = 2;      // OGG comments : series of null-terminated UTF-8 strings
  BASS_TAG_HTTP          = 3;      // HTTP headers : series of null-terminated ANSI strings
  BASS_TAG_ICY           = 4;      // ICY headers : series of null-terminated ANSI strings
  BASS_TAG_META          = 5;      // ICY metadata : ANSI string
  BASS_TAG_APE           = 6;      // APEv2 tags : series of null-terminated UTF-8 strings
  BASS_TAG_MP4           = 7;      // MP4/iTunes metadata : series of null-terminated UTF-8 strings
  BASS_TAG_VENDOR        = 9;      // OGG encoder : UTF-8 string
  BASS_TAG_LYRICS3       = 10;     // Lyric3v2 tag : ASCII string
  BASS_TAG_CA_CODEC      = 11;     // CoreAudio codec info : TAG_CA_CODEC structure
  BASS_TAG_MF            = 13;     // Media Foundation tags : series of null-terminated UTF-8 strings
  BASS_TAG_WAVEFORMAT    = 14;     // WAVE format : WAVEFORMATEEX structure
  BASS_TAG_RIFF_INFO     = $100;   // RIFF "INFO" tags : series of null-terminated ANSI strings
  BASS_TAG_RIFF_BEXT     = $101;   // RIFF/BWF "bext" tags : TAG_BEXT structure
  BASS_TAG_RIFF_CART     = $102;   // RIFF/BWF "cart" tags : TAG_CART structure
  BASS_TAG_RIFF_DISP     = $103;   // RIFF "DISP" text tag : ANSI string
  BASS_TAG_APE_BINARY    = $1000;  // + index #, binary APEv2 tag : TAG_APE_BINARY structure
  BASS_TAG_MUSIC_NAME    = $10000; // MOD music name : ANSI string
  BASS_TAG_MUSIC_MESSAGE = $10001; // MOD message : ANSI string
  BASS_TAG_MUSIC_INST    = $10100; // + instrument #, MOD instrument name : ANSI string
  BASS_TAG_MUSIC_SAMPLE  = $10300; // + sample #, MOD sample name : ANSI string

  // BASS_ChannelGetLength/GetPosition/SetPosition modes
  BASS_POS_BYTE           = 0;         // byte position
  BASS_POS_MUSIC_ORDER    = 1;         // order.row position, MAKELONG(order,row)
  BASS_POS_OGG            = 3;         // OGG bitstream number
  BASS_POS_INEXACT        = $08000000; // flag: allow seeking to inexact position
  BASS_POS_DECODE         = $10000000; // flag: get the decoding (not playing) position
  BASS_POS_DECODETO       = $20000000; // flag: decode to the position instead of seeking
  BASS_POS_SCAN           = $40000000; // flag: scan to the position

  // BASS_RecordSetInput flags
  BASS_INPUT_OFF    = $10000;
  BASS_INPUT_ON     = $20000;

  BASS_INPUT_TYPE_MASK    = $FF000000;
  BASS_INPUT_TYPE_UNDEF   = $00000000;
  BASS_INPUT_TYPE_DIGITAL = $01000000;
  BASS_INPUT_TYPE_LINE    = $02000000;
  BASS_INPUT_TYPE_MIC     = $03000000;
  BASS_INPUT_TYPE_SYNTH   = $04000000;
  BASS_INPUT_TYPE_CD      = $05000000;
  BASS_INPUT_TYPE_PHONE   = $06000000;
  BASS_INPUT_TYPE_SPEAKER = $07000000;
  BASS_INPUT_TYPE_WAVE    = $08000000;
  BASS_INPUT_TYPE_AUX     = $09000000;
  BASS_INPUT_TYPE_ANALOG  = $0A000000;

  BASS_FX_DX8_CHORUS	  = 0;
  BASS_FX_DX8_COMPRESSOR  = 1;
  BASS_FX_DX8_DISTORTION  = 2;
  BASS_FX_DX8_ECHO        = 3;
  BASS_FX_DX8_FLANGER     = 4;
  BASS_FX_DX8_GARGLE      = 5;
  BASS_FX_DX8_I3DL2REVERB = 6;
  BASS_FX_DX8_PARAMEQ     = 7;
  BASS_FX_DX8_REVERB      = 8;

  BASS_DX8_PHASE_NEG_180 = 0;
  BASS_DX8_PHASE_NEG_90  = 1;
  BASS_DX8_PHASE_ZERO    = 2;
  BASS_DX8_PHASE_90      = 3;
  BASS_DX8_PHASE_180     = 4;

type
  DWORD = cardinal;
  BOOL  = LongBool;
  QWORD = int64;    // 64-bit (replace "int64" with "comp" if using Delphi 3)

  HMUSIC   = DWORD; // MOD music handle
  HSAMPLE  = DWORD; // sample handle
  HCHANNEL = DWORD; // playing sample's channel handle
  HSTREAM  = DWORD; // sample stream handle
  HRECORD  = DWORD; // recording handle
  HSYNC    = DWORD; // synchronizer handle
  HDSP     = DWORD; // DSP handle
  HFX      = DWORD; // DX8 effect handle
  HPLUGIN  = DWORD; // Plugin handle

  // Device info structure
  BASS_DEVICEINFO = record
    name  : PAnsiChar; // description
    driver: PAnsiChar; // driver
    flags : DWORD;
  end;

  BASS_INFO = record
    flags    : DWORD; // device capabilities (DSCAPS_xxx flags)
    hwsize   : DWORD; // size of total device hardware memory
    hwfree   : DWORD; // size of free device hardware memory
    freesam  : DWORD; // number of free sample slots in the hardware
    free3d   : DWORD; // number of free 3D sample slots in the hardware
    minrate  : DWORD; // min sample rate supported by the hardware
    maxrate  : DWORD; // max sample rate supported by the hardware
    eax      : BOOL;  // device supports EAX? (always FALSE if BASS_DEVICE_3D was not used)
    minbuf   : DWORD; // recommended minimum buffer length in ms (requires BASS_DEVICE_LATENCY)
    dsver    : DWORD; // DirectSound version
    latency  : DWORD; // delay (in ms) before start of playback (requires BASS_DEVICE_LATENCY)
    initflags: DWORD; // BASS_Init "flags" parameter
    speakers : DWORD; // number of speakers available
    freq     : DWORD; // current output rate
  end;

  // Recording device info structure
  BASS_RECORDINFO = record
    flags   : DWORD; // device capabilities (DSCCAPS_xxx flags)
    formats : DWORD; // supported standard formats (WAVE_FORMAT_xxx flags)
    inputs  : DWORD; // number of inputs
    singlein: BOOL;  // only 1 input can be set at a time
    freq    : DWORD; // current input rate
  end;

  // Sample info structure
  BASS_SAMPLE = record
    freq    : DWORD;  // default playback rate
    volume  : Single; // default volume (0-100)
    pan     : Single; // default pan (-100=left, 0=middle, 100=right)
    flags   : DWORD;  // BASS_SAMPLE_xxx flags
    length  : DWORD;  // length (in samples, not bytes)
    max     : DWORD;  // maximum simultaneous playbacks
    origres : DWORD;  // original resolution
    chans   : DWORD;  // number of channels
    mingap  : DWORD;  // minimum gap (ms) between creating channels
    mode3d  : DWORD;  // BASS_3DMODE_xxx mode
    mindist : Single; // minimum distance
    maxdist : Single; // maximum distance
    iangle  : DWORD;  // angle of inside projection cone
    oangle  : DWORD;  // angle of outside projection cone
    outvol  : Single; // delta-volume outside the projection cone
    vam     : DWORD;  // voice allocation/management flags (BASS_VAM_xxx)
    priority: DWORD;  // priority (0=lowest, $ffffffff=highest)
  end;

  // Channel info structure
  BASS_CHANNELINFO = record
    freq    : DWORD;     // default playback rate
    chans   : DWORD;     // channels
    flags   : DWORD;     // BASS_SAMPLE/STREAM/MUSIC/SPEAKER flags
    ctype   : DWORD;     // type of channel
    origres : DWORD;     // original resolution
    plugin  : HPLUGIN;   // plugin
    sample  : HSAMPLE;   // sample
    {$IFDEF CPUX64}
    padding: DWORD;
    {$ENDIF}
    filename: PAnsiChar; // filename
  end;

  BASS_PLUGINFORM = record
    ctype: DWORD;     // channel type
    {$IFDEF CPUX64}
    padding: DWORD;
    {$ENDIF}
    name : PAnsiChar; // format description
    exts : PAnsiChar; // file extension filter (*.ext1;*.ext2;etc...)
  end;
  PBASS_PLUGINFORMS = ^TBASS_PLUGINFORMS;
  TBASS_PLUGINFORMS = array[0..maxInt div sizeOf(BASS_PLUGINFORM) - 1] of BASS_PLUGINFORM;

  PBASS_PLUGININFO = ^BASS_PLUGININFO;
  BASS_PLUGININFO = record
    version: DWORD;             // version (same form as BASS_GetVersion)
    formatc: DWORD;             // number of formats
    formats: PBASS_PLUGINFORMS; // the array of formats
  end;

  // 3D vector (for 3D positions/velocities/orientations)
  BASS_3DVECTOR = record
    x: Single;  // +=right, -=left
    y: Single;  // +=up   , -=down
    z: Single;  // +=front, -=behind
  end;

  // User file stream callback functions
  FILECLOSEPROC = procedure(user: Pointer); stdcall;
  FILELENPROC   = function(user: Pointer): QWORD; stdcall;
  FILEREADPROC  = function(buffer: Pointer; length: DWORD; user: Pointer): DWORD; stdcall;
  FILESEEKPROC  = function(offset: QWORD; user: Pointer): BOOL; stdcall;

  BASS_FILEPROCS = record
    close : FILECLOSEPROC;
    length: FILELENPROC;
    read  : FILEREADPROC;
    seek  : FILESEEKPROC;
  end;

  // ID3v1 tag structure
  PTAG_ID3 = ^TAG_ID3;
  TAG_ID3 = record
    id     : Array[0.. 2] of AnsiChar;
    title  : Array[0..29] of AnsiChar;
    artist : Array[0..29] of AnsiChar;
    album  : Array[0..29] of AnsiChar;
    year   : Array[0.. 3] of AnsiChar;
    comment: Array[0..29] of AnsiChar;
    genre  : Byte;
  end;

  // Binary APEv2 tag structure
  PTAG_APE_BINARY = ^TAG_APE_BINARY;
  TAG_APE_BINARY = record
    key   : PAnsiChar;
    data  : PAnsiChar;
    length: DWORD;
  end;

  // BWF "bext" tag structure
  PTAG_BEXT = ^TAG_BEXT;
  TAG_BEXT = packed record
    Description        : Array[0..255] of AnsiChar; // description
    Originator         : Array[0.. 31] of AnsiChar; // name of the originator
    OriginatorReference: Array[0.. 31] of AnsiChar; // reference of the originator
    OriginationDate    : Array[0..  9] of AnsiChar; // date of creation (yyyy-mm-dd)
    OriginationTime    : Array[0..  7] of AnsiChar; // time of creation (hh-mm-ss)
    TimeReference      : QWORD;	                    // first sample count since midnight (little-endian)
    Version            : Word;	                    // BWF version (little-endian)
    UMID               : Array[0.. 63] of Byte;	    // SMPTE UMID
    Reserved           : Array[0..189] of Byte;
    CodingHistory: Array[0..maxInt div 2 - 1] of AnsiChar; // history
  end;

  BASS_DX8_CHORUS = record
    fWetDryMix: Single;
    fDepth    : Single;
    fFeedback : Single;
    fFrequency: Single;
    lWaveform : DWORD; // 0=triangle, 1=sine
    fDelay    : Single;
    lPhase    : DWORD; // BASS_DX8_PHASE_xxx
  end;

  BASS_DX8_COMPRESSOR = record
    fGain     : Single;
    fAttack   : Single;
    fRelease  : Single;
    fThreshold: Single;
    fRatio    : Single;
    fPredelay : Single;
  end;

  BASS_DX8_DISTORTION = record
    fGain                 : Single;
    fEdge                 : Single;
    fPostEQCenterFrequency: Single;
    fPostEQBandwidth      : Single;
    fPreLowpassCutoff     : Single;
  end;

  BASS_DX8_ECHO = record
    fWetDryMix : Single;
    fFeedback  : Single;
    fLeftDelay : Single;
    fRightDelay: Single;
    lPanDelay  : BOOL;
  end;

  BASS_DX8_FLANGER = record
    fWetDryMix: Single;
    fDepth    : Single;
    fFeedback : Single;
    fFrequency: Single;
    lWaveform : DWORD;   // 0=triangle, 1=sine
    fDelay    : Single;
    lPhase    : DWORD;   // BASS_DX8_PHASE_xxx
  end;

  BASS_DX8_GARGLE = record
    dwRateHz   : DWORD;  // Rate of modulation in hz
    dwWaveShape: DWORD;  // 0=triangle, 1=square
  end;

  BASS_DX8_I3DL2REVERB = record
    lRoom              : Longint; // [-10000, 0      ] default: -1000 mB
    lRoomHF            : Longint; // [-10000, 0      ] default: 0 mB
    flRoomRolloffFactor: Single;  // [0.0   , 10.0   ] default: 0.0
    flDecayTime        : Single;  // [0.1   , 20.0   ] default: 1.49s
    flDecayHFRatio     : Single;  // [0.1   , 2.0    ] default: 0.83
    lReflections       : Longint; // [-10000, 1000   ] default: -2602 mB
    flReflectionsDelay : Single;  // [0.0   , 0.3    ] default: 0.007 s
    lReverb            : Longint; // [-10000, 2000   ] default: 200 mB
    flReverbDelay      : Single;  // [0.0   , 0.1    ] default: 0.011 s
    flDiffusion        : Single;  // [0.0   , 100.0  ] default: 100.0 %
    flDensity          : Single;  // [0.0   , 100.0  ] default: 100.0 %
    flHFReference      : Single;  // [20.0  , 20000.0] default: 5000.0 Hz
  end;

  BASS_DX8_PARAMEQ = record
    fCenter   : Single;
    fBandwidth: Single;
    fGain     : Single;
  end;

  BASS_DX8_REVERB = record
    fInGain         : Single; // [-96.0,0.0   ] default: 0.0 dB
    fReverbMix      : Single; // [-96.0,0.0   ] default: 0.0 db
    fReverbTime     : Single; // [0.001,3000.0] default: 1000.0 ms
    fHighFreqRTRatio: Single; // [0.001,0.999 ] default: 0.001
  end;

  // callback function types
  STREAMPROC = function(handle: HSTREAM; buffer: Pointer; length: DWORD; user: Pointer): DWORD; stdcall;
  {
    User stream callback function. NOTE: A stream function should obviously be as
    quick as possible, other streams (and MOD musics) can't be mixed until
    it's finished.
    handle : The stream that needs writing
    buffer : Buffer to write the samples in
    length : Number of bytes to write
    user   : The 'user' parameter value given when calling BASS_StreamCreate
    RETURN : Number of bytes written. Set the BASS_STREAMPROC_END flag to end
             the stream.
  }

const
  // special STREAMPROCs
  STREAMPROC_DUMMY : pointer = pointer(0);  // "dummy" stream
  STREAMPROC_PUSH  : pointer = pointer(-1); // push stream

type

  DOWNLOADPROC = procedure(buffer: Pointer; length: DWORD; user: Pointer); stdcall;
  {
    Internet stream download callback function.
    buffer : Buffer containing the downloaded data... NULL=end of download
    length : Number of bytes in the buffer
    user   : The 'user' parameter value given when calling BASS_StreamCreateURL
  }

  SYNCPROC = procedure(handle: HSYNC; channel, data: DWORD; user: Pointer); stdcall;
  {
    Sync callback function. NOTE: a sync callback function should be very
    quick as other syncs cannot be processed until it has finished. If the
    sync is a "mixtime" sync, then other streams and MOD musics can not be
    mixed until it's finished either.
    handle : The sync that has occured
    channel: Channel that the sync occured in
    data   : Additional data associated with the sync's occurance
    user   : The 'user' parameter given when calling BASS_ChannelSetSync
  }

  DSPPROC = procedure(handle: HDSP; channel: DWORD; buffer: Pointer; length: DWORD; user: Pointer); stdcall;
  {
    DSP callback function. NOTE: A DSP function should obviously be as quick
    as possible... other DSP functions, streams and MOD musics can not be
    processed until it's finished.
    handle : The DSP handle
    channel: Channel that the DSP is being applied to
    buffer : Buffer to apply the DSP to
    length : Number of bytes in the buffer
    user   : The 'user' parameter given when calling BASS_ChannelSetDSP
  }

  RECORDPROC = function(handle: HRECORD; buffer: Pointer; length: DWORD; user: Pointer): BOOL; stdcall;
  {
    Recording callback function.
    handle : The recording handle
    buffer : Buffer containing the recorded sample data
    length : Number of bytes
    user   : The 'user' parameter value given when calling BASS_RecordStart
    RETURN : TRUE = continue recording, FALSE = stop
  }


// Vars that will hold our dynamically loaded functions...
var BASS_SetConfig      :function(option, value: DWORD): BOOL; stdcall;
var BASS_GetConfig      :function(option: DWORD): DWORD; stdcall;
var BASS_SetConfigPtr   :function(option: DWORD; value: Pointer): BOOL; stdcall;
var BASS_GetConfigPtr   :function(option: DWORD): Pointer; stdcall;
var BASS_GetVersion     :function: DWORD; stdcall;
var BASS_ErrorGetCode   :function: Integer; stdcall;
var BASS_GetDeviceInfo  :function(device: DWORD; var info: BASS_DEVICEINFO): BOOL; stdcall;
var BASS_Init           :function(device: Integer; freq, flags: DWORD; win: HWND; clsid: PGUID): BOOL; stdcall;
var BASS_SetDevice      :function(device: DWORD): BOOL; stdcall;
var BASS_GetDevice      :function: DWORD; stdcall;
var BASS_Free           :function: BOOL; stdcall;
var BASS_GetDSoundObject:function(obj: DWORD): Pointer; stdcall;
var BASS_GetInfo        :function(var info: BASS_INFO): BOOL; stdcall;
var BASS_Update         :function(length: DWORD): BOOL; stdcall;
var BASS_GetCPU         :function: Single; stdcall;
var BASS_Start          :function: BOOL; stdcall;
var BASS_Stop           :function: BOOL; stdcall;
var BASS_Pause          :function: BOOL; stdcall;
var BASS_SetVolume      :function(volume: Single): BOOL; stdcall;
var BASS_GetVolume      :function: Single; stdcall;

function BASS_PluginLoad   (filename: PAnsiChar; flags: DWORD): HPLUGIN; stdcall;
function BASS_PluginFree   (handle: HPLUGIN): BOOL; stdcall;
function BASS_PluginGetInfo(handle: HPLUGIN): PBASS_PLUGININFO; stdcall;

var BASS_Set3DFactors    :function(distf, rollf, doppf: Single): BOOL; stdcall;
var BASS_Get3DFactors    :function(var distf, rollf, doppf: Single): BOOL; stdcall;
var BASS_Set3DPosition   :function(var pos, vel, front, top: BASS_3DVECTOR): BOOL; stdcall;
var BASS_Get3DPosition   :function(var pos, vel, front, top: BASS_3DVECTOR): BOOL; stdcall;
var BASS_Apply3D         :procedure; stdcall;
var BASS_SetEAXParameters:function(env: Integer; vol, decay, damp: Single): BOOL; stdcall;
var BASS_GetEAXParameters:function(var env: DWORD; var vol, decay, damp: Single): BOOL; stdcall;

var BASS_MusicLoad:function(mem: BOOL; f: Pointer; offset: QWORD; length, flags, freq: DWORD): HMUSIC; stdcall;
var BASS_MusicFree:function(handle: HMUSIC): BOOL; stdcall;

var BASS_SampleLoad       :function(mem: BOOL; f: Pointer; offset: QWORD; length, max, flags: DWORD): HSAMPLE; stdcall;
var BASS_SampleCreate     :function(length, freq, chans, max, flags: DWORD): HSAMPLE; stdcall;
var BASS_SampleFree       :function(handle: HSAMPLE): BOOL; stdcall;
var BASS_SampleSetData    :function(handle: HSAMPLE; buffer: Pointer): BOOL; stdcall;
var BASS_SampleGetData    :function(handle: HSAMPLE; buffer: Pointer): BOOL; stdcall;
var BASS_SampleGetInfo    :function(handle: HSAMPLE; var info: BASS_SAMPLE): BOOL; stdcall;
var BASS_SampleSetInfo    :function(handle: HSAMPLE; var info: BASS_SAMPLE): BOOL; stdcall;
var BASS_SampleGetChannel :function(handle: HSAMPLE; onlynew: BOOL): HCHANNEL; stdcall;
var BASS_SampleGetChannels:function(handle: HSAMPLE; channels: Pointer): DWORD; stdcall;
var BASS_SampleStop       :function(handle: HSAMPLE): BOOL; stdcall;

var BASS_StreamCreate         :function(freq, chans, flags: DWORD; proc: STREAMPROC; user: Pointer): HSTREAM; stdcall;
var BASS_StreamCreateFile     :function(mem: BOOL; f: Pointer; offset, length: QWORD; flags: DWORD): HSTREAM; stdcall;
var BASS_StreamCreateURL      :function(url: Pointer; offset: DWORD; flags: DWORD; proc: DOWNLOADPROC; user: Pointer):HSTREAM; stdcall;
var BASS_StreamCreateFileUser :function(system, flags: DWORD; var procs: BASS_FILEPROCS; user: Pointer): HSTREAM; stdcall;
var BASS_StreamFree           :function(handle: HSTREAM): BOOL; stdcall;
var BASS_StreamGetFilePosition:function(handle: HSTREAM; mode: DWORD): QWORD; stdcall;
var BASS_StreamPutData        :function(handle: HSTREAM; buffer: Pointer; length: DWORD): DWORD; stdcall;
var BASS_StreamPutFileData    :function(handle: HSTREAM; buffer: Pointer; length: DWORD): DWORD; stdcall;

var BASS_RecordGetDeviceInfo:function(device: DWORD; var info: BASS_DEVICEINFO): BOOL; stdcall;
var BASS_RecordInit         :function(device: Integer): BOOL; stdcall;
var BASS_RecordSetDevice    :function(device: DWORD): BOOL; stdcall;
var BASS_RecordGetDevice    :function: DWORD; stdcall;
var BASS_RecordFree         :function: BOOL; stdcall;
var BASS_RecordGetInfo      :function(var info: BASS_RECORDINFO): BOOL; stdcall;
var BASS_RecordGetInputName :function(input: Integer): PAnsiChar; stdcall;
var BASS_RecordSetInput     :function(input: Integer; flags: DWORD; volume: Single): BOOL; stdcall;
var BASS_RecordGetInput     :function(input: Integer; var volume: Single): DWORD; stdcall;
var BASS_RecordStart        :function(freq, chans, flags: DWORD; proc: RECORDPROC; user: Pointer): HRECORD; stdcall;

var BASS_ChannelBytes2Seconds  :function(handle: DWORD; pos: QWORD): Double; stdcall;
var BASS_ChannelSeconds2Bytes  :function(handle: DWORD; pos: Double): QWORD; stdcall;
var BASS_ChannelGetDevice      :function(handle: DWORD): DWORD; stdcall;
var BASS_ChannelSetDevice      :function(handle, device: DWORD): BOOL; stdcall;
var BASS_ChannelIsActive       :function(handle: DWORD): DWORD; stdcall;
var BASS_ChannelGetInfo        :function(handle: DWORD; var info: BASS_CHANNELINFO):BOOL;stdcall;
var BASS_ChannelGetTags        :function(handle: HSTREAM; tags: DWORD): PAnsiChar; stdcall;
var BASS_ChannelFlags          :function(handle, flags, mask: DWORD): DWORD; stdcall;
var BASS_ChannelUpdate         :function(handle, length: DWORD): BOOL; stdcall;
var BASS_ChannelLock           :function(handle: DWORD; lock: BOOL): BOOL; stdcall;
var BASS_ChannelPlay           :function(handle: DWORD; restart: BOOL): BOOL; stdcall;
var BASS_ChannelStop           :function(handle: DWORD): BOOL; stdcall;
var BASS_ChannelPause          :function(handle: DWORD): BOOL; stdcall;
var BASS_ChannelSetAttribute   :function(handle, attrib: DWORD; value: Single): BOOL; stdcall;
var BASS_ChannelGetAttribute   :function(handle, attrib: DWORD; var value: Single): BOOL; stdcall;
var BASS_ChannelSlideAttribute :function(handle, attrib: DWORD; value: Single; time: DWORD): BOOL; stdcall;
var BASS_ChannelIsSliding      :function(handle, attrib: DWORD): BOOL; stdcall;
var BASS_ChannelSetAttributeEx :function(handle, attrib: DWORD; value: Pointer; size: DWORD): BOOL; stdcall;
var BASS_ChannelGetAttributeEx :function(handle, attrib: DWORD; value: Pointer; size: DWORD): DWORD; stdcall;

var BASS_ChannelSet3DAttributes:function(handle: DWORD; mode: Integer; min, max: Single; iangle, oangle, outvol: Integer): BOOL; stdcall;
var BASS_ChannelGet3DAttributes:function(handle: DWORD; var mode: DWORD; var min, max: Single; var iangle, oangle, outvol: DWORD): BOOL; stdcall;
var BASS_ChannelSet3DPosition  :function(handle: DWORD; var pos, orient, vel: BASS_3DVECTOR): BOOL; stdcall;
var BASS_ChannelGet3DPosition  :function(handle: DWORD; var pos, orient, vel: BASS_3DVECTOR): BOOL; stdcall;
var BASS_ChannelGetLength      :function(handle, mode: DWORD): QWORD; stdcall;
var BASS_ChannelSetPosition    :function(handle: DWORD; pos: QWORD; mode: DWORD): BOOL; stdcall;
var BASS_ChannelGetPosition    :function(handle, mode: DWORD): QWORD; stdcall;
var BASS_ChannelGetLevel       :function(handle: DWORD): DWORD; stdcall;
var BASS_ChannelGetLevelEx     :function(handle: DWORD; levels: PSingle; length: Single; flags: DWORD): BOOL; stdcall;
var BASS_ChannelGetData        :function(handle: DWORD; buffer: Pointer; length: DWORD): DWORD; stdcall;
var BASS_ChannelSetSync        :function(handle: DWORD; type_: DWORD; param: QWORD; proc: SYNCPROC; user: Pointer): HSYNC; stdcall;
var BASS_ChannelRemoveSync     :function(handle: DWORD; sync: HSYNC): BOOL; stdcall;
var BASS_ChannelSetDSP         :function(handle: DWORD; proc: DSPPROC; user: Pointer; priority: Integer): HDSP; stdcall;
var BASS_ChannelRemoveDSP      :function(handle: DWORD; dsp: HDSP): BOOL; stdcall;
var BASS_ChannelSetLink        :function(handle, chan: DWORD): BOOL; stdcall;
var BASS_ChannelRemoveLink     :function(handle, chan: DWORD): BOOL; stdcall;
var BASS_ChannelSetFX          :function(handle, type_: DWORD; priority: Integer): HFX; stdcall;
var BASS_ChannelRemoveFX       :function(handle: DWORD; fx: HFX): BOOL; stdcall;

var BASS_FXSetParameters:function(handle: HFX; par: Pointer): BOOL; stdcall;
var BASS_FXGetParameters:function(handle: HFX; par: Pointer): BOOL; stdcall;
var BASS_FXReset        :function(handle: HFX): BOOL; stdcall;

{ok, now we need something that loads our DLL and gets rid of it as well...}

var BASS_Handle:Thandle=0; // this will hold our handle for the dll; it functions nicely as a mutli-dll prevention unit as well...

Function Load_BASSDLL(dllfilename:PAnsiChar):boolean; overload;
Function Load_BASSDLL(dllfilename:PWideChar):boolean; overload;

Procedure Unload_BASSDLL; // another mystery function ???
{
  This function frees the dynamically linked-in functions from memory...don't forget to call it once you're done !
  Best place to put this is probably the OnDestroy of your Main-Form;
  suggested use in OnDestroy :
  - Call BASS_Free to get rid of everything that's eating memory (automatically called, but just to be on the safe-side !),
  - Then call this function.
}


function BASS_SPEAKER_N(n: DWORD): DWORD;
function BASS_SetEAXPreset(env: Integer): BOOL;
{
  This function is defined in the implementation part of this unit.
  It is not part of BASS.DLL but an extra function which makes it easier
  to set the predefined EAX environments.
  env    : a EAX_ENVIRONMENT_xxx constant
}

implementation

var BASS_PluginLoad_   :function(f: PAnsiChar; flags: DWORD): HPLUGIN; stdcall;
var BASS_PluginFree_   :function(handle: HPLUGIN): BOOL; stdcall;
var BASS_PluginGetInfo_:function(handle: HPLUGIN): PBASS_PLUGININFO; stdcall;

Function CheckBASSHandle:boolean;
{$IFDEF CHECK_PROC}label L_Exit;{$ENDIF}
begin
  if BASS_Handle<>0 then
  begin {now we tie the functions to the VARs from above}

    @BASS_SetConfig      :=GetProcAddress(BASS_Handle,PAnsiChar('BASS_SetConfig'));
    {$IFDEF CHECK_PROC}if @BASS_SetConfig=nil then goto L_Exit;{$ENDIF}
    @BASS_GetConfig      :=GetProcAddress(BASS_Handle,PAnsiChar('BASS_GetConfig'));
    {$IFDEF CHECK_PROC}if @BASS_GetConfig=nil then goto L_Exit;{$ENDIF}
    @BASS_SetConfigPtr   :=GetProcAddress(BASS_Handle,PAnsiChar('BASS_SetConfigPtr'));
    {$IFDEF CHECK_PROC}if @BASS_SetConfigPtr=nil then goto L_Exit;{$ENDIF}
    @BASS_GetConfigPtr   :=GetProcAddress(BASS_Handle,PAnsiChar('BASS_GetConfigPtr'));
    {$IFDEF CHECK_PROC}if @BASS_GetConfigPtr=nil then goto L_Exit;{$ENDIF}
    @BASS_GetVersion     :=GetProcAddress(BASS_Handle,PAnsiChar('BASS_GetVersion'));
    {$IFDEF CHECK_PROC}if @BASS_GetVersion=nil then goto L_Exit;{$ENDIF}
    @BASS_ErrorGetCode   :=GetProcAddress(BASS_Handle,PAnsiChar('BASS_ErrorGetCode'));
    {$IFDEF CHECK_PROC}if @BASS_ErrorGetCode=nil then goto L_Exit;{$ENDIF}
    @BASS_GetDeviceInfo  :=GetProcAddress(BASS_Handle,PAnsiChar('BASS_GetDeviceInfo'));
    {$IFDEF CHECK_PROC}if @BASS_GetDeviceInfo=nil then goto L_Exit;{$ENDIF}
    @BASS_Init           :=GetProcAddress(BASS_Handle,PAnsiChar('BASS_Init'));
    {$IFDEF CHECK_PROC}if @BASS_Init=nil then goto L_Exit;{$ENDIF}
    @BASS_SetDevice      :=GetProcAddress(BASS_Handle,PAnsiChar('BASS_SetDevice'));
    {$IFDEF CHECK_PROC}if @BASS_SetDevice=nil then goto L_Exit;{$ENDIF}
    @BASS_GetDevice      :=GetProcAddress(BASS_Handle,PAnsiChar('BASS_GetDevice'));
    {$IFDEF CHECK_PROC}if @BASS_GetDevice=nil then goto L_Exit;{$ENDIF}
    @BASS_Free           :=GetProcAddress(BASS_Handle,PAnsiChar('BASS_Free'));
    {$IFDEF CHECK_PROC}if @BASS_Free=nil then goto L_Exit;{$ENDIF}
    @BASS_GetDSoundObject:=GetProcAddress(BASS_Handle,PAnsiChar('BASS_GetDSoundObject'));
    {$IFDEF CHECK_PROC}if @BASS_GetDSoundObject=nil then goto L_Exit;{$ENDIF}
    @BASS_GetInfo        :=GetProcAddress(BASS_Handle,PAnsiChar('BASS_GetInfo'));
    {$IFDEF CHECK_PROC}if @BASS_GetInfo=nil then goto L_Exit;{$ENDIF}
    @BASS_Update         :=GetProcAddress(BASS_Handle,PAnsiChar('BASS_Update'));
    {$IFDEF CHECK_PROC}if @BASS_Update=nil then goto L_Exit;{$ENDIF}
    @BASS_GetCPU         :=GetProcAddress(BASS_Handle,PAnsiChar('BASS_GetCPU'));
    {$IFDEF CHECK_PROC}if @BASS_GetCPU=nil then goto L_Exit;{$ENDIF}
    @BASS_Start          :=GetProcAddress(BASS_Handle,PAnsiChar('BASS_Start'));
    {$IFDEF CHECK_PROC}if @BASS_Start=nil then goto L_Exit;{$ENDIF}
    @BASS_Stop           :=GetProcAddress(BASS_Handle,PAnsiChar('BASS_Stop'));
    {$IFDEF CHECK_PROC}if @BASS_Stop=nil then goto L_Exit;{$ENDIF}
    @BASS_Pause          :=GetProcAddress(BASS_Handle,PAnsiChar('BASS_Pause'));
    {$IFDEF CHECK_PROC}if @BASS_Pause=nil then goto L_Exit;{$ENDIF}
    @BASS_SetVolume      :=GetProcAddress(BASS_Handle,PAnsiChar('BASS_SetVolume'));
    {$IFDEF CHECK_PROC}if @BASS_SetVolume=nil then goto L_Exit;{$ENDIF}
    @BASS_GetVolume      :=GetProcAddress(BASS_Handle,PAnsiChar('BASS_GetVolume'));
    {$IFDEF CHECK_PROC}if @BASS_GetVolume=nil then goto L_Exit;{$ENDIF}

    @BASS_PluginLoad_   :=GetProcAddress(BASS_Handle,PAnsiChar('BASS_PluginLoad'));
    {$IFDEF CHECK_PROC}if @BASS_PluginLoad_=nil then goto L_Exit;{$ENDIF}
    @BASS_PluginFree_   :=GetProcAddress(BASS_Handle,PAnsiChar('BASS_PluginFree'));
    {$IFDEF CHECK_PROC}if @BASS_PluginFree_=nil then goto L_Exit;{$ENDIF}
    @BASS_PluginGetInfo_:=GetProcAddress(BASS_Handle,PAnsiChar('BASS_PluginGetInfo'));
    {$IFDEF CHECK_PROC}if @BASS_PluginGetInfo_=nil then goto L_Exit;{$ENDIF}

    @BASS_Set3DFactors    :=GetProcAddress(BASS_Handle,PAnsiChar('BASS_Set3DFactors'));
    {$IFDEF CHECK_PROC}if @BASS_Set3DFactors=nil then goto L_Exit;{$ENDIF}
    @BASS_Get3DFactors    :=GetProcAddress(BASS_Handle,PAnsiChar('BASS_Get3DFactors'));
    {$IFDEF CHECK_PROC}if @BASS_Get3DFactors=nil then goto L_Exit;{$ENDIF}
    @BASS_Set3DPosition   :=GetProcAddress(BASS_Handle,PAnsiChar('BASS_Set3DPosition'));
    {$IFDEF CHECK_PROC}if @BASS_Set3DPosition=nil then goto L_Exit;{$ENDIF}
    @BASS_Get3DPosition   :=GetProcAddress(BASS_Handle,PAnsiChar('BASS_Get3DPosition'));
    {$IFDEF CHECK_PROC}if @BASS_Get3DPosition=nil then goto L_Exit;{$ENDIF}
    @BASS_Apply3D         :=GetProcAddress(BASS_Handle,PAnsiChar('BASS_Apply3D'));
    {$IFDEF CHECK_PROC}if @BASS_Apply3D=nil then goto L_Exit;{$ENDIF}
    @BASS_SetEAXParameters:=GetProcAddress(BASS_Handle,PAnsiChar('BASS_SetEAXParameters'));
    {$IFDEF CHECK_PROC}if @BASS_SetEAXParameters=nil then goto L_Exit;{$ENDIF}
    @BASS_GetEAXParameters:=GetProcAddress(BASS_Handle,PAnsiChar('BASS_GetEAXParameters'));
    {$IFDEF CHECK_PROC}if @BASS_GetEAXParameters=nil then goto L_Exit;{$ENDIF}

    @BASS_MusicLoad:=GetProcAddress(BASS_Handle,PAnsiChar('BASS_MusicLoad'));
    {$IFDEF CHECK_PROC}if @BASS_MusicLoad=nil then goto L_Exit;{$ENDIF}
    @BASS_MusicFree:=GetProcAddress(BASS_Handle,PAnsiChar('BASS_MusicFree'));
    {$IFDEF CHECK_PROC}if @BASS_MusicFree=nil then goto L_Exit;{$ENDIF}

    @BASS_SampleLoad       :=GetProcAddress(BASS_Handle,PAnsiChar('BASS_SampleLoad'));
    {$IFDEF CHECK_PROC}if @BASS_SampleLoad=nil then goto L_Exit;{$ENDIF}
    @BASS_SampleCreate     :=GetProcAddress(BASS_Handle,PAnsiChar('BASS_SampleCreate'));
    {$IFDEF CHECK_PROC}if @BASS_SampleCreate=nil then goto L_Exit;{$ENDIF}
    @BASS_SampleFree       :=GetProcAddress(BASS_Handle,PAnsiChar('BASS_SampleFree'));
    {$IFDEF CHECK_PROC}if @BASS_SampleFree=nil then goto L_Exit;{$ENDIF}
    @BASS_SampleSetData    :=GetProcAddress(BASS_Handle,PAnsiChar('BASS_SampleSetData'));
    {$IFDEF CHECK_PROC}if @BASS_SampleSetData=nil then goto L_Exit;{$ENDIF}
    @BASS_SampleGetData    :=GetProcAddress(BASS_Handle,PAnsiChar('BASS_SampleGetData'));
    {$IFDEF CHECK_PROC}if @BASS_SampleGetData=nil then goto L_Exit;{$ENDIF}
    @BASS_SampleGetInfo    :=GetProcAddress(BASS_Handle,PAnsiChar('BASS_SampleGetInfo'));
    {$IFDEF CHECK_PROC}if @BASS_SampleGetInfo=nil then goto L_Exit;{$ENDIF}
    @BASS_SampleSetInfo    :=GetProcAddress(BASS_Handle,PAnsiChar('BASS_SampleSetInfo'));
    {$IFDEF CHECK_PROC}if @BASS_SampleSetInfo=nil then goto L_Exit;{$ENDIF}
    @BASS_SampleGetChannel :=GetProcAddress(BASS_Handle,PAnsiChar('BASS_SampleGetChannel'));
    {$IFDEF CHECK_PROC}if @BASS_SampleGetChannel=nil then goto L_Exit;{$ENDIF}
    @BASS_SampleGetChannels:=GetProcAddress(BASS_Handle,PAnsiChar('BASS_SampleGetChannels'));
    {$IFDEF CHECK_PROC}if @BASS_SampleGetChannels=nil then goto L_Exit;{$ENDIF}
    @BASS_SampleStop       :=GetProcAddress(BASS_Handle,PAnsiChar('BASS_SampleStop'));
    {$IFDEF CHECK_PROC}if @BASS_SampleStop=nil then goto L_Exit;{$ENDIF}

    @BASS_StreamCreate         :=GetProcAddress(BASS_Handle,PAnsiChar('BASS_StreamCreate'));
    {$IFDEF CHECK_PROC}if @BASS_StreamCreate=nil then goto L_Exit;{$ENDIF}
    @BASS_StreamCreateFile     :=GetProcAddress(BASS_Handle,PAnsiChar('BASS_StreamCreateFile'));
    {$IFDEF CHECK_PROC}if @BASS_StreamCreateFile=nil then goto L_Exit;{$ENDIF}
    @BASS_StreamCreateURL      :=GetProcAddress(BASS_Handle,PAnsiChar('BASS_StreamCreateURL'));
    {$IFDEF CHECK_PROC}if @BASS_StreamCreateURL=nil then goto L_Exit;{$ENDIF}
    @BASS_StreamCreateFileUser :=GetProcAddress(BASS_Handle,PAnsiChar('BASS_StreamCreateFileUser'));
    {$IFDEF CHECK_PROC}if @BASS_StreamCreateFileUser=nil then goto L_Exit;{$ENDIF}
    @BASS_StreamFree           :=GetProcAddress(BASS_Handle,PAnsiChar('BASS_StreamFree'));
    {$IFDEF CHECK_PROC}if @BASS_StreamFree=nil then goto L_Exit;{$ENDIF}
    @BASS_StreamGetFilePosition:=GetProcAddress(BASS_Handle,PAnsiChar('BASS_StreamGetFilePosition'));
    {$IFDEF CHECK_PROC}if @BASS_StreamGetFilePosition=nil then goto L_Exit;{$ENDIF}
    @BASS_StreamPutData        :=GetProcAddress(BASS_Handle,PAnsiChar('BASS_StreamPutData'));
    {$IFDEF CHECK_PROC}if @BASS_StreamPutData=nil then goto L_Exit;{$ENDIF}
    @BASS_StreamPutFileData    :=GetProcAddress(BASS_Handle,PAnsiChar('BASS_StreamPutFileData'));
    {$IFDEF CHECK_PROC}if @BASS_StreamPutFileData=nil then goto L_Exit;{$ENDIF}

    @BASS_RecordGetDeviceInfo:=GetProcAddress(BASS_Handle,PAnsiChar('BASS_RecordGetDeviceInfo'));
    {$IFDEF CHECK_PROC}if @BASS_RecordGetDeviceInfo=nil then goto L_Exit;{$ENDIF}
    @BASS_RecordInit         :=GetProcAddress(BASS_Handle,PAnsiChar('BASS_RecordInit'));
    {$IFDEF CHECK_PROC}if @BASS_RecordInit=nil then goto L_Exit;{$ENDIF}
    @BASS_RecordSetDevice    :=GetProcAddress(BASS_Handle,PAnsiChar('BASS_RecordSetDevice'));
    {$IFDEF CHECK_PROC}if @BASS_RecordSetDevice=nil then goto L_Exit;{$ENDIF}
    @BASS_RecordGetDevice    :=GetProcAddress(BASS_Handle,PAnsiChar('BASS_RecordGetDevice'));
    {$IFDEF CHECK_PROC}if @BASS_RecordGetDevice=nil then goto L_Exit;{$ENDIF}
    @BASS_RecordFree         :=GetProcAddress(BASS_Handle,PAnsiChar('BASS_RecordFree'));
    {$IFDEF CHECK_PROC}if @BASS_RecordFree=nil then goto L_Exit;{$ENDIF}
    @BASS_RecordGetInfo      :=GetProcAddress(BASS_Handle,PAnsiChar('BASS_RecordGetInfo'));
    {$IFDEF CHECK_PROC}if @BASS_RecordGetInfo=nil then goto L_Exit;{$ENDIF}
    @BASS_RecordGetInputName :=GetProcAddress(BASS_Handle,PAnsiChar('BASS_RecordGetInputName'));
    {$IFDEF CHECK_PROC}if @BASS_RecordGetInputName=nil then goto L_Exit;{$ENDIF}
    @BASS_RecordSetInput     :=GetProcAddress(BASS_Handle,PAnsiChar('BASS_RecordSetInput'));
    {$IFDEF CHECK_PROC}if @BASS_RecordSetInput=nil then goto L_Exit;{$ENDIF}
    @BASS_RecordGetInput     :=GetProcAddress(BASS_Handle,PAnsiChar('BASS_RecordGetInput'));
    {$IFDEF CHECK_PROC}if @BASS_RecordGetInput=nil then goto L_Exit;{$ENDIF}
    @BASS_RecordStart        :=GetProcAddress(BASS_Handle,PAnsiChar('BASS_RecordStart'));
    {$IFDEF CHECK_PROC}if @BASS_RecordStart=nil then goto L_Exit;{$ENDIF}

    @BASS_ChannelBytes2Seconds  :=GetProcAddress(BASS_Handle,PAnsiChar('BASS_ChannelBytes2Seconds'));
    {$IFDEF CHECK_PROC}if @BASS_ChannelBytes2Seconds=nil then goto L_Exit;{$ENDIF}
    @BASS_ChannelSeconds2Bytes  :=GetProcAddress(BASS_Handle,PAnsiChar('BASS_ChannelSeconds2Bytes'));
    {$IFDEF CHECK_PROC}if @BASS_ChannelSeconds2Bytes=nil then goto L_Exit;{$ENDIF}
    @BASS_ChannelGetDevice      :=GetProcAddress(BASS_Handle,PAnsiChar('BASS_ChannelGetDevice'));
    {$IFDEF CHECK_PROC}if @BASS_ChannelGetDevice=nil then goto L_Exit;{$ENDIF}
    @BASS_ChannelSetDevice      :=GetProcAddress(BASS_Handle,PAnsiChar('BASS_ChannelSetDevice'));
    {$IFDEF CHECK_PROC}if @BASS_ChannelSetDevice=nil then goto L_Exit;{$ENDIF}
    @BASS_ChannelIsActive       :=GetProcAddress(BASS_Handle,PAnsiChar('BASS_ChannelIsActive'));
    {$IFDEF CHECK_PROC}if @BASS_ChannelIsActive=nil then goto L_Exit;{$ENDIF}
    @BASS_ChannelGetInfo        :=GetProcAddress(BASS_Handle,PAnsiChar('BASS_ChannelGetInfo'));
    {$IFDEF CHECK_PROC}if @BASS_ChannelGetInfo=nil then goto L_Exit;{$ENDIF}
    @BASS_ChannelGetTags        :=GetProcAddress(BASS_Handle,PAnsiChar('BASS_ChannelGetTags'));
    {$IFDEF CHECK_PROC}if @BASS_ChannelGetTags=nil then goto L_Exit;{$ENDIF}
    @BASS_ChannelFlags          :=GetProcAddress(BASS_Handle,PAnsiChar('BASS_ChannelFlags'));
    {$IFDEF CHECK_PROC}if @BASS_ChannelFlags=nil then goto L_Exit;{$ENDIF}
    @BASS_ChannelUpdate         :=GetProcAddress(BASS_Handle,PAnsiChar('BASS_ChannelUpdate'));
    {$IFDEF CHECK_PROC}if @BASS_ChannelUpdate=nil then goto L_Exit;{$ENDIF}
    @BASS_ChannelLock           :=GetProcAddress(BASS_Handle,PAnsiChar('BASS_ChannelLock'));
    {$IFDEF CHECK_PROC}if @BASS_ChannelLock=nil then goto L_Exit;{$ENDIF}
    @BASS_ChannelPlay           :=GetProcAddress(BASS_Handle,PAnsiChar('BASS_ChannelPlay'));
    {$IFDEF CHECK_PROC}if @BASS_ChannelPlay=nil then goto L_Exit;{$ENDIF}
    @BASS_ChannelStop           :=GetProcAddress(BASS_Handle,PAnsiChar('BASS_ChannelStop'));
    {$IFDEF CHECK_PROC}if @BASS_ChannelStop=nil then goto L_Exit;{$ENDIF}
    @BASS_ChannelPause          :=GetProcAddress(BASS_Handle,PAnsiChar('BASS_ChannelPause'));
    {$IFDEF CHECK_PROC}if @BASS_ChannelPause=nil then goto L_Exit;{$ENDIF}
    @BASS_ChannelSetAttribute   :=GetProcAddress(BASS_Handle,PAnsiChar('BASS_ChannelSetAttribute'));
    {$IFDEF CHECK_PROC}if @BASS_ChannelSetAttribute=nil then goto L_Exit;{$ENDIF}
    @BASS_ChannelGetAttribute   :=GetProcAddress(BASS_Handle,PAnsiChar('BASS_ChannelGetAttribute'));
    {$IFDEF CHECK_PROC}if @BASS_ChannelGetAttribute=nil then goto L_Exit;{$ENDIF}
    @BASS_ChannelSlideAttribute :=GetProcAddress(BASS_Handle,PAnsiChar('BASS_ChannelSlideAttribute'));
    {$IFDEF CHECK_PROC}if @BASS_ChannelSlideAttribute=nil then goto L_Exit;{$ENDIF}
    @BASS_ChannelIsSliding      :=GetProcAddress(BASS_Handle,PAnsiChar('BASS_ChannelIsSliding'));
    {$IFDEF CHECK_PROC}if @BASS_ChannelIsSliding=nil then goto L_Exit;{$ENDIF}
    @BASS_ChannelSetAttributeEx :=GetProcAddress(BASS_Handle,PAnsiChar('BASS_ChannelSetAttributeEx'));
    {$IFDEF CHECK_PROC}if @BASS_ChannelSetAttributeEx=nil then goto L_Exit;{$ENDIF}
    @BASS_ChannelGetAttributeEx :=GetProcAddress(BASS_Handle,PAnsiChar('BASS_ChannelGetAttributeEx'));
    {$IFDEF CHECK_PROC}if @BASS_ChannelGetAttributeEx=nil then goto L_Exit;{$ENDIF}
    @BASS_ChannelSet3DAttributes:=GetProcAddress(BASS_Handle,PAnsiChar('BASS_ChannelSet3DAttributes'));
    {$IFDEF CHECK_PROC}if @BASS_ChannelSet3DAttributes=nil then goto L_Exit;{$ENDIF}
    @BASS_ChannelGet3DAttributes:=GetProcAddress(BASS_Handle,PAnsiChar('BASS_ChannelGet3DAttributes'));
    {$IFDEF CHECK_PROC}if @BASS_ChannelGet3DAttributes=nil then goto L_Exit;{$ENDIF}
    @BASS_ChannelSet3DPosition  :=GetProcAddress(BASS_Handle,PAnsiChar('BASS_ChannelSet3DPosition'));
    {$IFDEF CHECK_PROC}if @BASS_ChannelSet3DPosition=nil then goto L_Exit;{$ENDIF}
    @BASS_ChannelGet3DPosition  :=GetProcAddress(BASS_Handle,PAnsiChar('BASS_ChannelGet3DPosition'));
    {$IFDEF CHECK_PROC}if @BASS_ChannelGet3DPosition=nil then goto L_Exit;{$ENDIF}
    @BASS_ChannelGetLength      :=GetProcAddress(BASS_Handle,PAnsiChar('BASS_ChannelGetLength'));
    {$IFDEF CHECK_PROC}if @BASS_ChannelGetLength=nil then goto L_Exit;{$ENDIF}
    @BASS_ChannelSetPosition    :=GetProcAddress(BASS_Handle,PAnsiChar('BASS_ChannelSetPosition'));
    {$IFDEF CHECK_PROC}if @BASS_ChannelSetPosition=nil then goto L_Exit;{$ENDIF}
    @BASS_ChannelGetPosition    :=GetProcAddress(BASS_Handle,PAnsiChar('BASS_ChannelGetPosition'));
    {$IFDEF CHECK_PROC}if @BASS_ChannelGetPosition=nil then goto L_Exit;{$ENDIF}
    @BASS_ChannelGetLevel       :=GetProcAddress(BASS_Handle,PAnsiChar('BASS_ChannelGetLevel'));
    {$IFDEF CHECK_PROC}if @BASS_ChannelGetLevel=nil then goto L_Exit;{$ENDIF}
    @BASS_ChannelGetLevelEx       :=GetProcAddress(BASS_Handle,PAnsiChar('BASS_ChannelGetLevelEx'));
    {$IFDEF CHECK_PROC}if @BASS_ChannelGetLevelEx=nil then goto L_Exit;{$ENDIF}
    @BASS_ChannelGetData        :=GetProcAddress(BASS_Handle,PAnsiChar('BASS_ChannelGetData'));
    {$IFDEF CHECK_PROC}if @BASS_ChannelGetData=nil then goto L_Exit;{$ENDIF}
    @BASS_ChannelSetSync        :=GetProcAddress(BASS_Handle,PAnsiChar('BASS_ChannelSetSync'));
    {$IFDEF CHECK_PROC}if @BASS_ChannelSetSync=nil then goto L_Exit;{$ENDIF}
    @BASS_ChannelRemoveSync     :=GetProcAddress(BASS_Handle,PAnsiChar('BASS_ChannelRemoveSync'));
    {$IFDEF CHECK_PROC}if @BASS_ChannelRemoveSync=nil then goto L_Exit;{$ENDIF}
    @BASS_ChannelSetDSP         :=GetProcAddress(BASS_Handle,PAnsiChar('BASS_ChannelSetDSP'));
    {$IFDEF CHECK_PROC}if @BASS_ChannelSetDSP=nil then goto L_Exit;{$ENDIF}
    @BASS_ChannelRemoveDSP      :=GetProcAddress(BASS_Handle,PAnsiChar('BASS_ChannelRemoveDSP'));
    {$IFDEF CHECK_PROC}if @BASS_ChannelRemoveDSP=nil then goto L_Exit;{$ENDIF}
    @BASS_ChannelSetLink        :=GetProcAddress(BASS_Handle,PAnsiChar('BASS_ChannelSetLink'));
    {$IFDEF CHECK_PROC}if @BASS_ChannelSetLink=nil then goto L_Exit;{$ENDIF}
    @BASS_ChannelRemoveLink     :=GetProcAddress(BASS_Handle,PAnsiChar('BASS_ChannelRemoveLink'));
    {$IFDEF CHECK_PROC}if @BASS_ChannelRemoveLink=nil then goto L_Exit;{$ENDIF}
    @BASS_ChannelSetFX          :=GetProcAddress(BASS_Handle,PAnsiChar('BASS_ChannelSetFX'));
    {$IFDEF CHECK_PROC}if @BASS_ChannelSetFX=nil then goto L_Exit;{$ENDIF}
    @BASS_ChannelRemoveFX       :=GetProcAddress(BASS_Handle,PAnsiChar('BASS_ChannelRemoveFX'));
    {$IFDEF CHECK_PROC}if @BASS_ChannelRemoveFX=nil then goto L_Exit;{$ENDIF}

    @BASS_FXSetParameters:=GetProcAddress(BASS_Handle,PAnsiChar('BASS_FXSetParameters'));
    {$IFDEF CHECK_PROC}if @BASS_FXSetParameters=nil then goto L_Exit;{$ENDIF}
    @BASS_FXGetParameters:=GetProcAddress(BASS_Handle,PAnsiChar('BASS_FXGetParameters'));
    {$IFDEF CHECK_PROC}if @BASS_FXGetParameters=nil then goto L_Exit;{$ENDIF}
    @BASS_FXReset        :=GetProcAddress(BASS_Handle,PAnsiChar('BASS_FXReset'));
    {$IFDEF CHECK_PROC}if @BASS_FXReset=nil then goto L_Exit;{$ENDIF}

    result:=true;
    exit;
{$IFDEF CHECK_PROC}
L_Exit:
    FreeLibrary(BASS_Handle);
    BASS_Handle:=0;
{$ENDIF}
  end;
  result:=false;
end;

Function Load_BASSDLL(dllfilename:PAnsiChar):boolean;
var
  oldmode:integer;
begin
  if BASS_Handle<>0 then result:=true
  else
  begin
    oldmode:=SetErrorMode($8001);
    BASS_Handle:=LoadLibraryA(dllfilename);
    SetErrorMode(oldmode);
    result:=CheckBASSHandle;
  end;
end;

Function Load_BASSDLL(dllfilename:PWideChar):boolean;
var
  oldmode:integer;
begin
  if BASS_Handle<>0 then result:=true
  else
  begin
    oldmode:=SetErrorMode($8001);
    BASS_Handle:=LoadLibraryW(dllfilename);
    SetErrorMode(oldmode);
    result:=CheckBASSHandle;
  end;
end;

Procedure Unload_BASSDLL;
begin
  if BASS_Handle<>0 then
  begin
    BASS_Free; // make sure we release everything
    FreeLibrary(BASS_Handle);
  end;
  BASS_Handle:=0;
end;

function BASS_SPEAKER_N(n: DWORD): DWORD;
begin
  Result := n shl 24;
end;

type
  tEAXrec = record
    vol  ,
    decay,
    damp : Single;
  end;

const
  EAXTable : array [0..EAX_ENVIRONMENT_COUNT-1] of tEAXRec = (
    (vol:0.5  ; decay: 1.493; damp:0.5  ),
    (vol:0.25 ; decay: 0.1  ; damp:0    ),
    (vol:0.417; decay: 0.4  ; damp:0.666),
    (vol:0.653; decay: 1.499; damp:0.166),
    (vol:0.208; decay: 0.478; damp:0    ),
    (vol:0.5  ; decay: 2.309; damp:0.888),
    (vol:0.403; decay: 4.279; damp:0.5  ),
    (vol:0.5  ; decay: 3.961; damp:0.5  ),
    (vol:0.5  ; decay: 2.886; damp:1.304),
    (vol:0.361; decay: 7.284; damp:0.332),
    (vol:0.5  ; decay:10.0  ; damp:0.3  ),
    (vol:0.153; decay: 0.259; damp:2.0  ),
    (vol:0.361; decay: 1.493; damp:0    ),
    (vol:0.444; decay: 2.697; damp:0.638),
    (vol:0.25 ; decay: 1.752; damp:0.776),
    (vol:0.111; decay: 3.145; damp:0.472),
    (vol:0.111; decay: 2.767; damp:0.224),
    (vol:0.194; decay: 7.841; damp:0.472),
    (vol:1    ; decay: 1.499; damp:0.5  ),
    (vol:0.097; decay: 2.767; damp:0.224),
    (vol:0.208; decay: 1.652; damp:1.5  ),
    (vol:0.652; decay: 2.886; damp:0.25 ),
    (vol:1    ; decay: 1.499; damp:0    ),
    (vol:0.875; decay: 8.392; damp:1.388),
    (vol:0.139; decay:17.234; damp:0.666),
    (vol:0.486; decay: 7.563; damp:0.806));

function BASS_SetEAXPreset(env: Integer): BOOL;
begin
  if env<EAX_ENVIRONMENT_COUNT then
  begin
    with EAXTable[env] do
      result:=BASS_SetEAXParameters(env,vol,decay,damp);
  end
  else
    result:=false;
end;

const
  PluginsMax = 50;
const
  NumPlugin:cardinal=0;
var
  arrPlugins:array [0..PluginsMax] of HPLUGIN;

// Plugin functions
function BASS_PluginLoad(filename:PAnsiChar;flags:DWORD):HPLUGIN; stdcall;
var
  i:cardinal;
begin
  result:=BASS_PluginLoad_(filename,flags);
  if result<>0 then // 0 - not plugin?
  begin
    i:=0;
    while i<NumPlugin do
    begin
      if arrPlugins[i]=result then // if present already
        exit;
      inc(i);
    end;
    // Remember for future
    arrPlugins[i]:=result;
    inc(NumPlugin);
    arrPlugins[NumPlugin]:=0; //last - zero
  end;
end;

function BASS_PluginFree(handle:HPLUGIN):BOOL; stdcall;
var
  i:cardinal;
begin
  result:=BASS_PluginFree_(handle);
  if result then
  begin
    if handle=0 then
      NumPlugin:=0
    else
    begin
      i:=0;
      while i<NumPlugin do
      begin
        if arrPlugins[i]=handle then
        begin
          move(arrPlugins[i+1],arrPlugins[i],SizeOf(HPLUGIN)*(NumPlugin-i));
          dec(NumPlugin);
          exit;
        end;
        inc(i);
      end;
    end;
  end;
end;

function BASS_PluginGetInfo(handle:HPLUGIN):PBASS_PLUGININFO; stdcall;
begin
  if handle=0 then
  begin
    if NumPlugin=0 then
      result:=nil
    else
      result:=@arrPlugins;
  end
  else
    result:=BASS_PluginGetInfo_(handle);
end;

end.
