//{$DEFINE USE_EXCEPTIONS}
{*****************************************************************************
*  unit based on                                                             *
*  ZLibEx.pas (zlib 1.2.1)                                                   *
*  Edition 2003.12.18                                                        *
*                                                                            *
*  copyright (c) 2002-2003 Roberto Della Pasqua (www.dellapasqua.com)        *
*  copyright (c) 2000-2002 base2 technologies (www.base2ti.com)              *
*  copyright (c) 1997 Borland International (www.borland.com)                *
*                                                                            *
*  and                                                                       *
*                                                                            *
*  BZip2 unit by Edison Mera (www.geocities.com/efmera/)                     *
*  Version 1.02                                                              *
*  Edition 21-11-2002                                                        *
*                                                                            *
*  Changes made by GMax:                                                     *
*                                                                            *
*  units joined. gzip support functions added.                               *
*  compression/decompression streams classes excluded,                       *
*  compression/decompression stream2stream functions added                   *
*                                                                            *
*  procedures converted to functions to add "no exceptions" functionality    *
*  return values actual ONLY in this case (no exceptions)                    *
*  error occured while value<0                                               *
*                                                                            *
*  function names for Z(De)Compress changed to Z(De)CompressBuf              *
*                                                                            *
*  (C) GMax 2004. email: gmax@loving.ru                                      *
*****************************************************************************}

unit KolZLibBzip;

interface

uses
  KOL{$IFDEF USE_EXCEPTIONS}, ERR{$ENDIF};

const
  ZLIB_VERSION      = '1.2.1';
  BZIP_VERSION      = '1.0.2';

type
  TAlloc = function(opaque: Pointer; Items, Size: Integer): Pointer; cdecl;
  TFree = procedure(opaque, Block: Pointer); cdecl;

  TZCompressionLevel = (zcNone, zcFastest, zcDefault, zcMax);
  TZCompressionStrategy = (zcsDefault, zcsFiltered, zcsHuffmanOnly);
  {** TZStreamRec ***********************************************************}

  TZStreamRec = packed record
    next_in: PChar;       // next input byte
    avail_in: Longint;    // number of bytes available at next_in
    total_in: Longint;    // total nb of input bytes read so far

    next_out: PChar;      // next output byte should be put here
    avail_out: Longint;   // remaining free space at next_out
    total_out: Longint;   // total nb of bytes output so far

    msg: PChar;           // last error message, NULL if no error
    state: Pointer;       // not visible by applications

    zalloc: TAlloc;       // used to allocate the internal state
    zfree: TFree;         // used to free the internal state
    opaque: Pointer;      // private data object passed to zalloc and zfree

    data_type: Integer;   // best guess about the data type: ascii or binary
    adler: Longint;       // adler32 value of the uncompressed data
    reserved: Longint;    // reserved for future use
  end;

  {** zlib public routines ****************************************************}

  {*****************************************************************************
  *  ZCompressBuf                                                              *
  *                                                                            *
  *  pre-conditions                                                            *
  *    inBuffer  = pointer to uncompressed data                                *
  *    inSize    = size of inBuffer (bytes)                                    *
  *    outBuffer = pointer (unallocated)                                       *
  *    level     = compression level                                           *
  *                                                                            *
  *  post-conditions                                                           *
  *    outBuffer = pointer to compressed data (allocated)                      *
  *    outSize   = size of outBuffer (bytes)                                   *
  *****************************************************************************}

function ZCompressBuf(const inBuffer: Pointer; inSize: Integer;
  out outBuffer: Pointer; out outSize: Integer;
  level: TZCompressionLevel = zcDefault): Integer;

{*****************************************************************************
*  ZDecompressBuf                                                            *
*                                                                            *
*  pre-conditions                                                            *
*    inBuffer    = pointer to compressed data                                *
*    inSize      = size of inBuffer (bytes)                                  *
*    outBuffer   = pointer (unallocated)                                     *
*    outEstimate = estimated size of uncompressed data (bytes)               *
*                                                                            *
*  post-conditions                                                           *
*    outBuffer = pointer to decompressed data (allocated)                    *
*    outSize   = size of outBuffer (bytes)                                   *
*****************************************************************************}

function ZDecompressBuf(const inBuffer: Pointer; inSize: Integer;
  out outBuffer: Pointer; out outSize: Integer; outEstimate: Integer = 0): Integer;

{** string routines *********************************************************}

function ZCompressStr(const s: string; level: TZCompressionLevel = zcDefault): string;

function ZDecompressStr(const s: string): string;

{** stream routines *********************************************************}

function ZCompressStream(inStream, outStream: PStream;
  level: TZCompressionLevel = zcDefault): Integer;

function ZDecompressStream(inStream, outStream: PStream): Integer;

{** utility routines ********************************************************}

function adler32(adler: LongInt; const buf: PChar; len: Integer): LongInt;
function CRC32(CRC: Cardinal; const Data: PChar; cbData: Cardinal): Cardinal;
function compressBound(sourceLen: LongInt): LongInt;

{****************************************************************************}

procedure MoveI32(const Source; var Dest; Count: Integer);
procedure ZFastCompressString(var s: string; level: TZCompressionLevel);
procedure ZFastDecompressString(var s: string);
procedure ZSendToBrowser(var s: string);

type
  TgzipHeader = packed record
    FileName: string;
    Comment: string;
    FileTime: TDateTime;
    Extra: string;
  end;

function gZipCompressStream(inStream, outStream: PStream; var gzHdr: TgzipHeader; level: TZCompressionLevel = zcDefault; strategy: TZCompressionStrategy = zcsDefault): Integer; overload;
function gZipCompressStream(inStream, outStream: PStream; level: TZCompressionLevel = zcDefault; strategy: TZCompressionStrategy = zcsDefault): Integer; overload;
function gZipDecompressStreamHeader(inStream: PStream; var gzHdr: TgzipHeader): Integer;
function gZipDecompressStreamBody(inStream, outStream: PStream): Integer;
function gZipDecompressStream(inStream, outStream: PStream; var gzHdr: TgzipHeader): Integer;
function gZipDecompressString(const S: String): String;

{*******************************************************}
{                                                       }
{     BZIP2 Data Compression Interface Unit             }
{                                                       }
{*******************************************************}
type
  // Internal structure.
  TBZStreamRec = packed record
    next_in: PChar;       // next input byte
    avail_in: Integer;    // number of bytes available at next_in
    total_in_lo32: Integer; // total nb of input bytes read so far
    total_in_hi32: Integer;

    next_out: PChar;      // next output byte should be put here
    avail_out: Integer;   // remaining free space at next_out
    total_out_lo32: Integer; // total nb of bytes output so far
    total_out_hi32: Integer;

    state: Pointer;

    bzalloc: TAlloc;      // used to allocate the internal state
    bzfree: TFree;        // used to free the internal state
    opaque: Pointer;
  end;
  TBlockSize100k = 1..9;
  { CompressBuf compresses data, buffer to buffer, in one call.
     In: InBuf = ptr to compressed data
         InBytes = number of bytes in InBuf
    Out: OutBuf = ptr to newly allocated buffer containing decompressed data
         OutBytes = number of bytes in OutBuf   }
function BZCompressBuf(const InBuf: Pointer; InBytes: Integer;
  out OutBuf: Pointer; out OutBytes: Integer): Integer;

{ DecompressBuf decompresses data, buffer to buffer, in one call.
   In: InBuf = ptr to compressed data
       InBytes = number of bytes in InBuf
       OutEstimate = zero, or est. size of the decompressed data
  Out: OutBuf = ptr to newly allocated buffer containing decompressed data
       OutBytes = number of bytes in OutBuf   }
function BZDecompressBuf(const InBuf: Pointer; InBytes: Integer;
  OutEstimate: Integer; out OutBuf: Pointer; out OutBytes: Integer): Integer;

function BZCompressStream(inStream, outStream: PStream; BlockSize100k: TBlockSize100k = 5): Integer;
function BZDecompressStream(inStream, outStream: PStream): Integer;


{** deflate routines ********************************************************}

function deflateInit_(var strm: TZStreamRec; level: Integer; version: PChar;
  recsize: Integer): Integer; external;

function DeflateInit2_(var strm: TZStreamRec; level: integer; method: integer; windowBits: integer;
  memLevel: integer; strategy: integer; version: PChar; recsize: integer): integer; external;

function deflate(var strm: TZStreamRec; flush: Integer): Integer;
  external;

function deflateEnd(var strm: TZStreamRec): Integer; external;

{** inflate routines ********************************************************}

function inflateInit_(var strm: TZStreamRec; version: PChar;
  recsize: Integer): Integer; external;

function inflateInit2_(var strm: TZStreamRec; windowBits: integer;
  version: PChar; recsize: integer): integer; external;

function inflate(var strm: TZStreamRec; flush: Integer): Integer;
  external;

function inflateEnd(var strm: TZStreamRec): Integer; external;

function inflateReset(var strm: TZStreamRec): Integer; external;

const
  gzBufferSize      = 16384;
  gz_magic          : array[0..1] of Byte = ($1F, $8B);
  { gzip flag byte }

  GZF_ASCII_FLAG    = $01; { bit 0 set: file probably ascii text }
  GZF_HEAD_CRC      = $02; { bit 1 set: header CRC present }
  GZF_EXTRA_FIELD   = $04; { bit 2 set: extra field present }
  GZF_ORIG_NAME     = $08; { bit 3 set: original file name present }
  GZF_COMMENT       = $10; { bit 4 set: file comment present }
  GZF_RESERVED      = $E0; { bits 5..7: reserved }
  Z_EOF             = -1;

const
  { ** Maximum value for windowBits in deflateInit2 and inflateInit2 }
  MAX_WBITS         = 15;
  { ** Maximum value for memLevel in deflateInit2 }
const
  MAX_MEM_LEVEL     = 9;
  DEF_MEM_LEVEL     = 8;

  {** link zlib 1.2.1 *********************************************************}
  {** bcc32 flags: -c -6 -O2 -Ve -X- -pr -a8 -b -d -k- -vi -tWM -r -RT-        }

{$L zlib\adler32.obj}
{$L zlib\compress.obj}
  {.$L zlib\crc32.obj}
{$L zlib\deflate.obj}
{$L zlib\infback.obj}
{$L zlib\inffast.obj}
{$L zlib\inflate.obj}
{$L zlib\inftrees.obj}
{$L zlib\trees.obj}
{$L zlib\uncompr.obj}

  {*****************************************************************************
  *  note: do not reorder the above -- doing so will result in external        *
  *  functions being undefined                                                 *
  *****************************************************************************}

const
  {** flush constants *******************************************************}

  Z_NO_FLUSH        = 0;
  Z_PARTIAL_FLUSH   = 1;
  Z_SYNC_FLUSH      = 2;
  Z_FULL_FLUSH      = 3;
  Z_FINISH          = 4;

  {** return codes **********************************************************}

  Z_OK              = 0;
  Z_STREAM_END      = 1;
  Z_NEED_DICT       = 2;
  Z_ERRNO           = (-1);
  Z_STREAM_ERROR    = (-2);
  Z_DATA_ERROR      = (-3);
  Z_MEM_ERROR       = (-4);
  Z_BUF_ERROR       = (-5);
  Z_VERSION_ERROR   = (-6);
  Z_WRITE_ERROR     = (-10);
  Z_CRC_ERROR       = (-11);
  Z_SIZE_ERROR      = (-12);

  {** compression levels ****************************************************}

  Z_NO_COMPRESSION  = 0;
  Z_BEST_SPEED      = 1;
  Z_BEST_COMPRESSION = 9;
  Z_DEFAULT_COMPRESSION = (-1);

  {** compression strategies ************************************************}

  Z_FILTERED        = 1;
  Z_HUFFMAN_ONLY    = 2;
  Z_DEFAULT_STRATEGY = 0;

  {** data types ************************************************************}

  Z_BINARY          = 0;
  Z_ASCII           = 1;
  Z_UNKNOWN         = 2;

  {** compression methods ***************************************************}

  Z_DEFLATED        = 8;

  Z_NULL            = nil; { for initializing zalloc, zfree, opaque }

  {** return code messages **************************************************}

  _z_errmsg         : array[0..14] of PChar = (
    'need dictionary',    // Z_NEED_DICT      (2)
    'stream end',         // Z_STREAM_END     (1)
    '',                   // Z_OK             (0)
    'file error',         // Z_ERRNO          (-1)
    'stream error',       // Z_STREAM_ERROR   (-2)
    'data error',         // Z_DATA_ERROR     (-3)
    'insufficient memory', // Z_MEM_ERROR      (-4)
    'buffer error',       // Z_BUF_ERROR      (-5)
    'incompatible version', // Z_VERSION_ERROR  (-6)
    '', '', '',
    'stream write error', // Z_WRITE_ERROR   = (-10);
    'crc error',          // Z_CRC_ERROR     = (-11);
    'size mismarch'       // Z_SIZE_ERROR    = (-12);
    );

  ZLevels           : array[TZCompressionLevel] of Shortint = (
    Z_NO_COMPRESSION,
    Z_BEST_SPEED,
    Z_DEFAULT_COMPRESSION,
    Z_BEST_COMPRESSION
    );
  ZStrategy         : array[TZCompressionStrategy] of Shortint = (
    Z_DEFAULT_STRATEGY,
    Z_FILTERED,
    Z_HUFFMAN_ONLY
    );

  {************** BZip constants **********************************************}
{$L bz2\blocksort.obj}
{$L bz2\huffman.obj}
{$L bz2\compress.obj}
{$L bz2\decompress.obj}
{$L bz2\bzlib.obj}
  { $L bz2\crctable.obj}
  { $L bz2\randtable.obj}

procedure _BZ2_hbMakeCodeLengths; external;
procedure _BZ2_blockSort; external;
procedure _BZ2_hbCreateDecodeTables; external;
procedure _BZ2_hbAssignCodes; external;
procedure _BZ2_compressBlock; external;
procedure _BZ2_decompress; external;

const
  bzBufferSize      = 32768;

  BZ_RUN            = 0;
  BZ_FLUSH          = 1;
  BZ_FINISH         = 2;
  BZ_OK             = 0;
  BZ_RUN_OK         = 1;
  BZ_FLUSH_OK       = 2;
  BZ_FINISH_OK      = 3;
  BZ_STREAM_END     = 4;
  BZ_SEQUENCE_ERROR = (-1);
  BZ_PARAM_ERROR    = (-2);
  BZ_MEM_ERROR      = (-3);
  BZ_DATA_ERROR     = (-4);
  BZ_DATA_ERROR_MAGIC = (-5);
  BZ_IO_ERROR       = (-6);
  BZ_UNEXPECTED_EOF = (-7);
  BZ_OUTBUFF_FULL   = (-8);

  BZ_Error_Msg      : array[1..8] of PChar = (
    'BZ_SEQUENCE_ERROR',
    'BZ_PARAM_ERROR',
    'BZ_MEM_ERROR',
    'BZ_DATA_ERROR',
    'BZ_DATA_ERROR_MAGIC',
    'BZ_IO_ERROR',
    'BZ_UNEXPECTED_EOF',
    'BZ_OUTBUFF_FULL'
    );

  BZ_BLOCK_SIZE_100K = 9;

  _BZ2_rNums        : array[0..511] of Longint = (
    619, 720, 127, 481, 931, 816, 813, 233, 566, 247,
    985, 724, 205, 454, 863, 491, 741, 242, 949, 214,
    733, 859, 335, 708, 621, 574, 73, 654, 730, 472,
    419, 436, 278, 496, 867, 210, 399, 680, 480, 51,
    878, 465, 811, 169, 869, 675, 611, 697, 867, 561,
    862, 687, 507, 283, 482, 129, 807, 591, 733, 623,
    150, 238, 59, 379, 684, 877, 625, 169, 643, 105,
    170, 607, 520, 932, 727, 476, 693, 425, 174, 647,
    73, 122, 335, 530, 442, 853, 695, 249, 445, 515,
    909, 545, 703, 919, 874, 474, 882, 500, 594, 612,
    641, 801, 220, 162, 819, 984, 589, 513, 495, 799,
    161, 604, 958, 533, 221, 400, 386, 867, 600, 782,
    382, 596, 414, 171, 516, 375, 682, 485, 911, 276,
    98, 553, 163, 354, 666, 933, 424, 341, 533, 870,
    227, 730, 475, 186, 263, 647, 537, 686, 600, 224,
    469, 68, 770, 919, 190, 373, 294, 822, 808, 206,
    184, 943, 795, 384, 383, 461, 404, 758, 839, 887,
    715, 67, 618, 276, 204, 918, 873, 777, 604, 560,
    951, 160, 578, 722, 79, 804, 96, 409, 713, 940,
    652, 934, 970, 447, 318, 353, 859, 672, 112, 785,
    645, 863, 803, 350, 139, 93, 354, 99, 820, 908,
    609, 772, 154, 274, 580, 184, 79, 626, 630, 742,
    653, 282, 762, 623, 680, 81, 927, 626, 789, 125,
    411, 521, 938, 300, 821, 78, 343, 175, 128, 250,
    170, 774, 972, 275, 999, 639, 495, 78, 352, 126,
    857, 956, 358, 619, 580, 124, 737, 594, 701, 612,
    669, 112, 134, 694, 363, 992, 809, 743, 168, 974,
    944, 375, 748, 52, 600, 747, 642, 182, 862, 81,
    344, 805, 988, 739, 511, 655, 814, 334, 249, 515,
    897, 955, 664, 981, 649, 113, 974, 459, 893, 228,
    433, 837, 553, 268, 926, 240, 102, 654, 459, 51,
    686, 754, 806, 760, 493, 403, 415, 394, 687, 700,
    946, 670, 656, 610, 738, 392, 760, 799, 887, 653,
    978, 321, 576, 617, 626, 502, 894, 679, 243, 440,
    680, 879, 194, 572, 640, 724, 926, 56, 204, 700,
    707, 151, 457, 449, 797, 195, 791, 558, 945, 679,
    297, 59, 87, 824, 713, 663, 412, 693, 342, 606,
    134, 108, 571, 364, 631, 212, 174, 643, 304, 329,
    343, 97, 430, 751, 497, 314, 983, 374, 822, 928,
    140, 206, 73, 263, 980, 736, 876, 478, 430, 305,
    170, 514, 364, 692, 829, 82, 855, 953, 676, 246,
    369, 970, 294, 750, 807, 827, 150, 790, 288, 923,
    804, 378, 215, 828, 592, 281, 565, 555, 710, 82,
    896, 831, 547, 261, 524, 462, 293, 465, 502, 56,
    661, 821, 976, 991, 658, 869, 905, 758, 745, 193,
    768, 550, 608, 933, 378, 286, 215, 979, 792, 961,
    61, 688, 793, 644, 986, 403, 106, 366, 905, 644,
    372, 567, 466, 434, 645, 210, 389, 550, 919, 135,
    780, 773, 635, 389, 707, 100, 626, 958, 165, 504,
    920, 176, 193, 713, 857, 265, 203, 50, 668, 108,
    645, 990, 626, 197, 510, 357, 358, 850, 858, 364,
    936, 638
    );

  _BZ2_crc32Table   : array[0..255] of Longint = (
    $00000000, $04C11DB7, $09823B6E, $0D4326D9,
    $130476DC, $17C56B6B, $1A864DB2, $1E475005,
    $2608EDB8, $22C9F00F, $2F8AD6D6, $2B4BCB61,
    $350C9B64, $31CD86D3, $3C8EA00A, $384FBDBD,
    $4C11DB70, $48D0C6C7, $4593E01E, $4152FDA9,
    $5F15ADAC, $5BD4B01B, $569796C2, $52568B75,
    $6A1936C8, $6ED82B7F, $639B0DA6, $675A1011,
    $791D4014, $7DDC5DA3, $709F7B7A, $745E66CD,
    -$67DC4920, -$631D54A9, -$6E5E7272, -$6A9F6FC7,
    -$74D83FC4, -$70192275, -$7D5A04AE, -$799B191B,
    -$41D4A4A8, -$4515B911, -$48569FCA, -$4C97827F,
    -$52D0D27C, -$5611CFCD, -$5B52E916, -$5F93F4A3,
    -$2BCD9270, -$2F0C8FD9, -$224FA902, -$268EB4B7,
    -$38C9E4B4, -$3C08F905, -$314BDFDE, -$358AC26B,
    -$0DC57FD8, -$09046261, -$044744BA, -$0086590F,
    -$1EC1090C, -$1A0014BD, -$17433266, -$13822FD3,
    $34867077, $30476DC0, $3D044B19, $39C556AE,
    $278206AB, $23431B1C, $2E003DC5, $2AC12072,
    $128E9DCF, $164F8078, $1B0CA6A1, $1FCDBB16,
    $018AEB13, $054BF6A4, $0808D07D, $0CC9CDCA,
    $7897AB07, $7C56B6B0, $71159069, $75D48DDE,
    $6B93DDDB, $6F52C06C, $6211E6B5, $66D0FB02,
    $5E9F46BF, $5A5E5B08, $571D7DD1, $53DC6066,
    $4D9B3063, $495A2DD4, $44190B0D, $40D816BA,
    -$535A3969, -$579B24E0, -$5AD80207, -$5E191FB2,
    -$405E4FB5, -$449F5204, -$49DC74DB, -$4D1D696E,
    -$7552D4D1, -$7193C968, -$7CD0EFBF, -$7811F20A,
    -$6656A20D, -$6297BFBC, -$6FD49963, -$6B1584D6,
    -$1F4BE219, -$1B8AFFB0, -$16C9D977, -$1208C4C2,
    -$0C4F94C5, -$088E8974, -$05CDAFAB, -$010CB21E,
    -$39430FA1, -$3D821218, -$30C134CF, -$3400297A,
    -$2A47797D, -$2E8664CC, -$23C54213, -$27045FA6,
    $690CE0EE, $6DCDFD59, $608EDB80, $644FC637,
    $7A089632, $7EC98B85, $738AAD5C, $774BB0EB,
    $4F040D56, $4BC510E1, $46863638, $42472B8F,
    $5C007B8A, $58C1663D, $558240E4, $51435D53,
    $251D3B9E, $21DC2629, $2C9F00F0, $285E1D47,
    $36194D42, $32D850F5, $3F9B762C, $3B5A6B9B,
    $0315D626, $07D4CB91, $0A97ED48, $0E56F0FF,
    $1011A0FA, $14D0BD4D, $19939B94, $1D528623,
    -$0ED0A9F2, -$0A11B447, -$075292A0, -$03938F29,
    -$1DD4DF2E, -$1915C29B, -$1456E444, -$1097F9F5,
    -$28D8444A, -$2C1959FF, -$215A7F28, -$259B6291,
    -$3BDC3296, -$3F1D2F23, -$325E09FC, -$369F144D,
    -$42C17282, -$46006F37, -$4B4349F0, -$4F825459,
    -$51C5045E, -$550419EB, -$58473F34, -$5C862285,
    -$64C99F3A, -$6008828F, -$6D4BA458, -$698AB9E1,
    -$77CDE9E6, -$730CF453, -$7E4FD28C, -$7A8ECF3D,
    $5D8A9099, $594B8D2E, $5408ABF7, $50C9B640,
    $4E8EE645, $4A4FFBF2, $470CDD2B, $43CDC09C,
    $7B827D21, $7F436096, $7200464F, $76C15BF8,
    $68860BFD, $6C47164A, $61043093, $65C52D24,
    $119B4BE9, $155A565E, $18197087, $1CD86D30,
    $029F3D35, $065E2082, $0B1D065B, $0FDC1BEC,
    $3793A651, $3352BBE6, $3E119D3F, $3AD08088,
    $2497D08D, $2056CD3A, $2D15EBE3, $29D4F654,
    -$3A56D987, -$3E97C432, -$33D4E2E9, -$3715FF60,
    -$2952AF5B, -$2D93B2EE, -$20D09435, -$24118984,
    -$1C5E343F, -$189F298A, -$15DC0F51, -$111D12E8,
    -$0F5A42E3, -$0B9B5F56, -$06D8798D, -$0219643C,
    -$764702F7, -$72861F42, -$7FC53999, -$7B042430,
    -$6543742B, -$6182699E, -$6CC14F45, -$680052F4,
    -$504FEF4F, -$548EF2FA, -$59CDD421, -$5D0CC998,
    -$434B9993, -$478A8426, -$4AC9A2FD, -$4E08BF4C
    );

// deflate compresses data

function BZ2_bzCompressInit(var strm: TBZStreamRec; blockSize100k: Integer;
  verbosity: Integer; workFactor: Integer): Integer; stdcall; external;

function BZ2_bzCompress(var strm: TBZStreamRec; action: Integer): Integer; stdcall; external;

function BZ2_bzCompressEnd(var strm: TBZStreamRec): Integer; stdcall; external;

function BZ2_bzBuffToBuffCompress(dest: Pointer; var destLen: Integer; source: Pointer;
  sourceLen, blockSize100k, verbosity, workFactor: Integer): Integer; stdcall; external;

// inflate decompresses data

function BZ2_bzDecompressInit(var strm: TBZStreamRec; verbosity: Integer;
  small: Integer): Integer; stdcall; external;

function BZ2_bzDecompress(var strm: TBZStreamRec): Integer; stdcall; external;

function BZ2_bzDecompressEnd(var strm: TBZStreamRec): Integer; stdcall; external;

function BZ2_bzBuffToBuffDecompress(dest: Pointer; var destLen: Integer; source: Pointer;
  sourceLen, small, verbosity: Integer): Integer; stdcall; external;

{** utility routines  *******************************************************}

function adler32(adler: LongInt; const buf: PChar; len: Integer): LongInt; external;
function compressBound(sourceLen: LongInt): LongInt; external;

//
function InflateInit(var stream: TZStreamRec): Integer;

implementation

procedure _bz_internal_error(errcode: Integer); cdecl;
begin
{$IFDEF USE_EXCEPTIONS}
  //raise EBZip2Error.CreateFmt('Compression Error %d', [errcode]);
  raise Exception.CreateFMT(e_Convert, 'Compression Error %d', [errcode]);
  // I don't know, what make in {$ELSE} :(
{$ENDIF}
end;

function _malloc(size: Integer): Pointer; cdecl;
begin
  GetMem(Result, Size);
end;

procedure _free(block: Pointer); cdecl;
begin
  FreeMem(block);
end;

function bzip2AllocMem(AppData: Pointer; Items, Size: Integer): Pointer; cdecl;
begin
  GetMem(Result, Items * Size);
end;

procedure bzip2FreeMem(AppData, Block: Pointer); cdecl;
begin
  FreeMem(Block);
end;

{*********************** Peter Morris not aligned move **********************}

procedure MoveI32(const Source; var Dest; Count: Integer); register;
asm
        cmp   ECX,0
        Je    @JustQuit
        push  ESI
        push  EDI
        mov   ESI, EAX
        mov   EDI, EDX
    @Loop:
	Mov   AL, [ESI]
        Inc   ESI
        mov   [EDI], AL
        Inc   EDI
        Dec   ECX
        Jnz   @Loop
        pop   EDI
        pop   ESI
    @JustQuit:
end;
{****************************************************************************}

{** utility routines  *******************************************************}

//function crc32; external;
function CRC32(CRC: Cardinal; const Data: PChar; cbData: Cardinal): Cardinal; assembler;
asm
      or     edx, edx
      je     @@exi
      jecxz  @@exi
      xor    eax,0FFFFFFFFh
      push   ebx
@@upd:
      movzx  ebx, al
      xor    bl, [ edx ]
      shr    eax, 8
      and    eax, 00FFFFFFh
      xor    eax, cs:[ebx*4 + offset @@c32tt ]//OFFSET @@C32TT ]
      inc    edx
      loop   @@upd
      pop    ebx
      xor    eax,0FFFFFFFFh
@@exi:
      ret

@@C32TT:

DD 000000000h, 077073096h, 0ee0e612ch, 0990951bah
DD 0076dc419h, 0706af48fh, 0e963a535h, 09e6495a3h
DD 00edb8832h, 079dcb8a4h, 0e0d5e91eh, 097d2d988h
DD 009b64c2bh, 07eb17cbdh, 0e7b82d07h, 090bf1d91h
DD 01db71064h, 06ab020f2h, 0f3b97148h, 084be41deh
DD 01adad47dh, 06ddde4ebh, 0f4d4b551h, 083d385c7h
DD 0136c9856h, 0646ba8c0h, 0fd62f97ah, 08a65c9ech
DD 014015c4fh, 063066cd9h, 0fa0f3d63h, 08d080df5h
DD 03b6e20c8h, 04c69105eh, 0d56041e4h, 0a2677172h
DD 03c03e4d1h, 04b04d447h, 0d20d85fdh, 0a50ab56bh
DD 035b5a8fah, 042b2986ch, 0dbbbc9d6h, 0acbcf940h
DD 032d86ce3h, 045df5c75h, 0dcd60dcfh, 0abd13d59h
DD 026d930ach, 051de003ah, 0c8d75180h, 0bfd06116h
DD 021b4f4b5h, 056b3c423h, 0cfba9599h, 0b8bda50fh
DD 02802b89eh, 05f058808h, 0c60cd9b2h, 0b10be924h
DD 02f6f7c87h, 058684c11h, 0c1611dabh, 0b6662d3dh
DD 076dc4190h, 001db7106h, 098d220bch, 0efd5102ah
DD 071b18589h, 006b6b51fh, 09fbfe4a5h, 0e8b8d433h
DD 07807c9a2h, 00f00f934h, 09609a88eh, 0e10e9818h
DD 07f6a0dbbh, 0086d3d2dh, 091646c97h, 0e6635c01h
DD 06b6b51f4h, 01c6c6162h, 0856530d8h, 0f262004eh
DD 06c0695edh, 01b01a57bh, 08208f4c1h, 0f50fc457h
DD 065b0d9c6h, 012b7e950h, 08bbeb8eah, 0fcb9887ch
DD 062dd1ddfh, 015da2d49h, 08cd37cf3h, 0fbd44c65h
DD 04db26158h, 03ab551ceh, 0a3bc0074h, 0d4bb30e2h
DD 04adfa541h, 03dd895d7h, 0a4d1c46dh, 0d3d6f4fbh
DD 04369e96ah, 0346ed9fch, 0ad678846h, 0da60b8d0h
DD 044042d73h, 033031de5h, 0aa0a4c5fh, 0dd0d7cc9h
DD 05005713ch, 0270241aah, 0be0b1010h, 0c90c2086h
DD 05768b525h, 0206f85b3h, 0b966d409h, 0ce61e49fh
DD 05edef90eh, 029d9c998h, 0b0d09822h, 0c7d7a8b4h
DD 059b33d17h, 02eb40d81h, 0b7bd5c3bh, 0c0ba6cadh
DD 0edb88320h, 09abfb3b6h, 003b6e20ch, 074b1d29ah
DD 0ead54739h, 09dd277afh, 004db2615h, 073dc1683h
DD 0e3630b12h, 094643b84h, 00d6d6a3eh, 07a6a5aa8h
DD 0e40ecf0bh, 09309ff9dh, 00a00ae27h, 07d079eb1h
DD 0f00f9344h, 08708a3d2h, 01e01f268h, 06906c2feh
DD 0f762575dh, 0806567cbh, 0196c3671h, 06e6b06e7h
DD 0fed41b76h, 089d32be0h, 010da7a5ah, 067dd4acch
DD 0f9b9df6fh, 08ebeeff9h, 017b7be43h, 060b08ed5h
DD 0d6d6a3e8h, 0a1d1937eh, 038d8c2c4h, 04fdff252h
DD 0d1bb67f1h, 0a6bc5767h, 03fb506ddh, 048b2364bh
DD 0d80d2bdah, 0af0a1b4ch, 036034af6h, 041047a60h
DD 0df60efc3h, 0a867df55h, 0316e8eefh, 04669be79h
DD 0cb61b38ch, 0bc66831ah, 0256fd2a0h, 05268e236h
DD 0cc0c7795h, 0bb0b4703h, 0220216b9h, 05505262fh
DD 0c5ba3bbeh, 0b2bd0b28h, 02bb45a92h, 05cb36a04h
DD 0c2d7ffa7h, 0b5d0cf31h, 02cd99e8bh, 05bdeae1dh
DD 09b64c2b0h, 0ec63f226h, 0756aa39ch, 0026d930ah
DD 09c0906a9h, 0eb0e363fh, 072076785h, 005005713h
DD 095bf4a82h, 0e2b87a14h, 07bb12baeh, 00cb61b38h
DD 092d28e9bh, 0e5d5be0dh, 07cdcefb7h, 00bdbdf21h
DD 086d3d2d4h, 0f1d4e242h, 068ddb3f8h, 01fda836eh
DD 081be16cdh, 0f6b9265bh, 06fb077e1h, 018b74777h
DD 088085ae6h, 0ff0f6a70h, 066063bcah, 011010b5ch
DD 08f659effh, 0f862ae69h, 0616bffd3h, 0166ccf45h
DD 0a00ae278h, 0d70dd2eeh, 04e048354h, 03903b3c2h
DD 0a7672661h, 0d06016f7h, 04969474dh, 03e6e77dbh
DD 0aed16a4ah, 0d9d65adch, 040df0b66h, 037d83bf0h
DD 0a9bcae53h, 0debb9ec5h, 047b2cf7fh, 030b5ffe9h
DD 0bdbdf21ch, 0cabac28ah, 053b39330h, 024b4a3a6h
DD 0bad03605h, 0cdd70693h, 054de5729h, 023d967bfh
DD 0b3667a2eh, 0c4614ab8h, 05d681b02h, 02a6f2b94h
DD 0b40bbe37h, 0c30c8ea1h, 05a05df1bh, 02d02ef8dh

end;

{** zlib function implementations *******************************************}

function zcalloc(opaque: Pointer; items, size: Integer): Pointer;
begin
  GetMem(result, items * size);
end;

procedure zcfree(opaque, block: Pointer);
begin
  FreeMem(block);
end;

{** c function implementations **********************************************}

procedure _memset(p: Pointer; b: Byte; count: Integer); cdecl;
begin
  FillChar(p^, count, b);
end;

procedure _memcpy(dest, source: Pointer; count: Integer); cdecl;
begin
  Move(source^, dest^, count);
end;

{** custom zlib routines ****************************************************}

function DeflateInit(var stream: TZStreamRec; level: Integer): Integer;
begin
  result := DeflateInit_(stream, level, ZLIB_VERSION, SizeOf(TZStreamRec));
end;

function DeflateInit2(var stream: TZStreamRec; level, method, windowBits, memLevel, strategy: Integer): Integer;
begin
  result := DeflateInit2_(stream, level, method, windowBits, memLevel,
    strategy, ZLIB_VERSION, SizeOf(TZStreamRec));
end;

function InflateInit(var stream: TZStreamRec): Integer;
begin
  result := InflateInit_(stream, ZLIB_VERSION, SizeOf(TZStreamRec));
end;

function InflateInit2(var stream: TZStreamRec; windowBits: Integer): Integer;
begin
  result := InflateInit2_(stream, windowBits, ZLIB_VERSION, SizeOf(TZStreamRec));
end;

{****************************************************************************}
{$IFDEF USE_EXCEPTIONS}

function ZCompressCheck(code: Integer): Integer;
begin
  result := code;

  if code < 0 then begin
    raise Exception.CreateFMT(e_Convert, 'Compression Error %d - %s', [code, _z_errmsg[2 - code]]);
  end;
end;

function ZDecompressCheck(code: Integer): Integer;
begin
  Result := code;

  if code < 0 then begin
    raise Exception.CreateFMT(e_Convert, 'Decompression Error %d - %s', [code, _z_errmsg[2 - code]]);
  end;
end;
{$ENDIF}

{****************************************************************************}
{****************************************************************************}
{****************************************************************************}
{**** implementation itself *************************************************}
{****************************************************************************}
{****************************************************************************}
{****************************************************************************}

function ZCompressBuf(const inBuffer: Pointer; inSize: Integer; out outBuffer: Pointer; out outSize: Integer; level: TZCompressionLevel): Integer;
const
  delta             = 256;
var
  zstream           : TZStreamRec;
begin
  FillChar(zstream, SizeOf(TZStreamRec), 0);
  Result := Z_OK;
  outSize := ((inSize + (inSize div 10) + 12) + 255) and not 255;
  outBuffer := nil;
  GetMem(outBuffer, outSize);
  try
    zstream.next_in := inBuffer;
    zstream.avail_in := inSize;
    zstream.next_out := outBuffer;
    zstream.avail_out := outSize;
{$IFDEF USE_EXCEPTIONS}
    ZCompressCheck(DeflateInit(zstream, ZLevels[level]));
{$ELSE}
    Result := DeflateInit(zstream, ZLevels[level]);
    if Result < 0 then Exit;
{$ENDIF}
    try
{$IFDEF USE_EXCEPTIONS}
      Result := ZCompressCheck(deflate(zstream, Z_FINISH));
{$ELSE}
      Result := deflate(zstream, Z_FINISH);
      if Result < 0 then Exit;
{$ENDIF}
      while Result <> Z_STREAM_END do begin
        Inc(outSize, delta);
        ReallocMem(outBuffer, outSize);

        zstream.next_out := PChar(Integer(outBuffer) + zstream.total_out);
        zstream.avail_out := delta;
{$IFDEF USE_EXCEPTIONS}
        Result := ZCompressCheck(deflate(zstream, Z_FINISH));
{$ELSE}
        Result := deflate(zstream, Z_FINISH);
        if Result < 0 then Exit;
{$ENDIF}
      end;                // while
    finally
{$IFDEF USE_EXCEPTIONS}
      ZCompressCheck(deflateEnd(zstream));
{$ELSE}
      deflateEnd(zstream);
{$ENDIF}
    end;

    ReallocMem(outBuffer, zstream.total_out);
    outSize := zstream.total_out;
{$IFDEF USE_EXCEPTIONS}
  except
    FreeMem(outBuffer);
    raise;
{$ELSE}
  finally
    if Result < 0 then FreeMem(outBuffer);
{$ENDIF}
  end;
end;

function ZCompressBuf2(const inBuffer: Pointer; inSize: Integer; out outBuffer: Pointer; out outSize: Integer): Integer;
const
  delta             = 256;
var
  zstream           : TZStreamRec;
begin
  FillChar(zstream, SizeOf(TZStreamRec), 0);

  outSize := ((inSize + (inSize div 10) + 12) + 255) and not 255;
  GetMem(outBuffer, outSize);
  Result := Z_OK;
  try
    zstream.next_in := inBuffer;
    zstream.avail_in := inSize;
    zstream.next_out := outBuffer;
    zstream.avail_out := outSize;
{$IFDEF USE_EXCEPTIONS}
    ZCompressCheck(DeflateInit2(zstream, 1, 8, -15, 9, 0));
{$ELSE}
    Result := DeflateInit2(zstream, 1, 8, -15, 9, 0);
    if Result < 0 then Exit;
{$ENDIF}

    try
{$IFDEF USE_EXCEPTIONS}
      Result := ZCompressCheck(deflate(zstream, Z_FINISH));
{$ELSE}
      Result := deflate(zstream, Z_FINISH);
      if Result < 0 then Exit;
{$ENDIF}
      while Result <> Z_STREAM_END do begin
        Inc(outSize, delta);
        ReallocMem(outBuffer, outSize);

        zstream.next_out := PChar(Integer(outBuffer) + zstream.total_out);
        zstream.avail_out := delta;
{$IFDEF USE_EXCEPTIONS}
        Result := ZCompressCheck(deflate(zstream, Z_FINISH));
{$ELSE}
        Result := deflate(zstream, Z_FINISH);
        if Result < 0 then Exit;
{$ENDIF}
      end;                // while
    finally
{$IFDEF USE_EXCEPTIONS}
      ZCompressCheck(deflateEnd(zstream));
{$ELSE}
      deflateEnd(zstream);
{$ENDIF}
    end;

    ReallocMem(outBuffer, zstream.total_out);
    outSize := zstream.total_out;
{$IFDEF USE_EXCEPTIONS}
  except
    FreeMem(outBuffer);
    raise;
{$ELSE}
  finally
    if Result < 0 then FreeMem(outBuffer);
{$ENDIF}
  end;
end;

function ZDecompressBuf(const inBuffer: Pointer; inSize: Integer; out outBuffer: Pointer; out outSize: Integer; outEstimate: Integer): Integer;
var
  zstream           : TZStreamRec;
  delta             : Integer;
begin
  FillChar(zstream, SizeOf(TZStreamRec), 0);

  delta := (inSize + 255) and not 255;

  if outEstimate = 0 then outSize := delta
  else outSize := outEstimate;
  Result := Z_OK;
  GetMem(outBuffer, outSize);
  try
    zstream.next_in := inBuffer;
    zstream.avail_in := inSize;
    zstream.next_out := outBuffer;
    zstream.avail_out := outSize;

{$IFDEF USE_EXCEPTIONS}
    ZDecompressCheck(InflateInit(zstream));
{$ELSE}
    Result := InflateInit(zstream);
    if Result < 0 then Exit;
{$ENDIF}

    try
{$IFDEF USE_EXCEPTIONS}
      Result := ZDecompressCheck(inflate(zstream, Z_NO_FLUSH));
{$ELSE}
      Result := inflate(zstream, Z_NO_FLUSH);
      if Result < 0 then Exit;
{$ENDIF}
      while (Result <> Z_STREAM_END) do begin
        Inc(outSize, delta);
        ReallocMem(outBuffer, outSize);

        zstream.next_out := PChar(Integer(outBuffer) + zstream.total_out);
        zstream.avail_out := delta;
{$IFDEF USE_EXCEPTIONS}
        Result := ZDecompressCheck(inflate(zstream, Z_NO_FLUSH));
{$ELSE}
        Result := inflate(zstream, Z_NO_FLUSH);
        if Result < 0 then Exit;
{$ENDIF}
      end;
    finally
{$IFDEF USE_EXCEPTIONS}
      ZDecompressCheck(inflateEnd(zstream));
{$ELSE}
      inflateEnd(zstream);
{$ENDIF}
    end;

    ReallocMem(outBuffer, zstream.total_out);
    outSize := zstream.total_out;

{$IFDEF USE_EXCEPTIONS}
  except
    FreeMem(outBuffer);
    raise;
{$ELSE}
  finally
    if Result < 0 then FreeMem(outBuffer);
{$ENDIF}
  end;
end;

{** string routines *********************************************************}

function ZCompressStr(const s: string; level: TZCompressionLevel): string;
var
  buffer            : Pointer;
  size              : Integer;
begin
  ZCompressBuf(PChar(s), Length(s), buffer, size, level);
  SetLength(result, size);
  Move(buffer^, pointer(result)^, size);
  FreeMem(buffer);
end;

procedure ZFastCompressString(var s: string; level: TZCompressionLevel);
var
  outBuf            : Pointer;
  outBytes          : Integer;
begin
  ZCompressBuf(pointer(s), length(s), outBuf, outBytes, level);
  SetLength(s, outBytes);
  MoveI32(pointer(outBuf)^, pointer(s)^, outBytes);
  FreeMem(outBuf);
end;

procedure ZFastDecompressString(var s: string);
var
  outBuf            : Pointer;
  outBytes          : Integer;
begin
  ZDecompressBuf(pointer(s), Length(s), outBuf, outBytes);
  SetLength(s, outBytes);
  MoveI32(pointer(outBuf)^, pointer(s)^, outBytes);
  FreeMem(outBuf);
end;

procedure ZSendToBrowser(var s: string);
var
  outBuf            : Pointer;
  outBytes          : Integer;
begin
  ZCompressBuf2(pointer(s), length(s), outBuf, outBytes);
  SetLength(s, outBytes);
  Move(pointer(outBuf)^, pointer(s)^, outBytes);
  FreeMem(outBuf);
end;

function ZDecompressStr(const s: string): string;
var
  buffer            : Pointer;
  size              : Integer;
begin
  ZDecompressBuf(PChar(s), Length(s), buffer, size);
  SetLength(result, size);
  Move(buffer^, pointer(result)^, size);
  FreeMem(buffer);
end;

{** stream routines *********************************************************}

function ZCompressStream(inStream, outStream: PStream; level: TZCompressionLevel): Integer;
const
  bufferSize        = 32768;
var
  zstream           : TZStreamRec;
  inBuffer          : array[0..bufferSize - 1] of Char;
  outBuffer         : array[0..bufferSize - 1] of Char;
  inSize            : Integer;
  outSize           : Integer;
begin
  FillChar(zstream, SizeOf(TZStreamRec), 0);
{$IFDEF USE_EXCEPTIONS}
  Result := Z_OK;
  ZCompressCheck(DeflateInit(zstream, ZLevels[level]));
{$ELSE}
  Result := DeflateInit(zstream, ZLevels[level]);
  if Result < 0 then Exit;
{$ENDIF}
  try
    inSize := inStream.Read(inBuffer, bufferSize);

    while inSize > 0 do begin
      zstream.next_in := inBuffer;
      zstream.avail_in := inSize;

      repeat
        zstream.next_out := outBuffer;
        zstream.avail_out := bufferSize;

{$IFDEF USE_EXCEPTIONS}
        ZCompressCheck(deflate(zstream, Z_NO_FLUSH));
{$ELSE}
        Result := deflate(zstream, Z_NO_FLUSH);
        if Result < 0 then Exit;
{$ENDIF}

        // outSize := zstream.next_out - outBuffer;
        outSize := bufferSize - zstream.avail_out;

        outStream.Write(outBuffer, outSize);
      until (zstream.avail_in = 0) and (zstream.avail_out > 0);

      inSize := inStream.Read(inBuffer, bufferSize);
    end;

    repeat
      zstream.next_out := outBuffer;
      zstream.avail_out := bufferSize;

{$IFDEF USE_EXCEPTIONS}
      Result := ZCompressCheck(deflate(zstream, Z_FINISH));
{$ELSE}
      Result := deflate(zstream, Z_FINISH);
      if Result < 0 then Break;
{$ENDIF}

      // outSize := zstream.next_out - outBuffer;
      outSize := bufferSize - zstream.avail_out;

      outStream.Write(outBuffer, outSize);
    until (Result = Z_STREAM_END) and (zstream.avail_out > 0);
  finally
{$IFDEF USE_EXCEPTIONS}
    ZCompressCheck(deflateEnd(zstream));
{$ELSE}
    deflateEnd(zstream);
{$ENDIF}
  end;
end;

function ZDecompressStream(inStream, outStream: PStream): Integer;
const
  bufferSize        = 32768;
var
  zstream           : TZStreamRec;
  inBuffer          : array[0..bufferSize - 1] of Char;
  outBuffer         : array[0..bufferSize - 1] of Char;
  inSize            : Integer;
  outSize           : Integer;
begin
  FillChar(zstream, SizeOf(TZStreamRec), 0);

{$IFDEF USE_EXCEPTIONS}
  Result := ZCompressCheck(InflateInit(zstream));
{$ELSE}
  Result := InflateInit(zstream);
  if Result < 0 then Exit;
{$ENDIF}
  try
    inSize := inStream.Read(inBuffer, bufferSize);

    while inSize > 0 do begin
      zstream.next_in := inBuffer;
      zstream.avail_in := inSize;

      repeat
        zstream.next_out := outBuffer;
        zstream.avail_out := bufferSize;

{$IFDEF USE_EXCEPTIONS}
        ZCompressCheck(inflate(zstream, Z_NO_FLUSH));
{$ELSE}
        Result := inflate(zstream, Z_NO_FLUSH);
        if Result < 0 then Exit;
{$ENDIF}

        // outSize := zstream.next_out - outBuffer;
        outSize := bufferSize - zstream.avail_out;

        outStream.Write(outBuffer, outSize);
      until (zstream.avail_in = 0) and (zstream.avail_out > 0);

      inSize := inStream.Read(inBuffer, bufferSize);
    end;

    repeat
      zstream.next_out := outBuffer;
      zstream.avail_out := bufferSize;

{$IFDEF USE_EXCEPTIONS}
      Result := ZCompressCheck(inflate(zstream, Z_FINISH));
{$ELSE}
      Result := inflate(zstream, Z_FINISH);
      if Result < 0 then Break;
{$ENDIF}

      // outSize := zstream.next_out - outBuffer;
      outSize := bufferSize - zstream.avail_out;

      outStream.Write(outBuffer, outSize);
    until (Result = Z_STREAM_END) and (zstream.avail_out > 0);
  finally
{$IFDEF USE_EXCEPTIONS}
    ZCompressCheck(inflateEnd(zstream));
{$ELSE}
    inflateEnd(zstream);
{$ENDIF}
  end;
end;

{** gzip Stream routines ******************************************************}
const
  UnixDateDelta     = 25569;

function DateTimeToUnix(ConvDate: TDateTime): Longint;
begin
  //example: DateTimeToUnix(now);
  Result := Round((ConvDate - UnixDateDelta) * 86400);
end;

function UnixToDateTime(USec: Longint): TDateTime;
begin
  //Example: UnixToDateTime(1003187418);
  Result := (Usec / 86400) + UnixDateDelta;
end;

function gZipCompressStream(inStream, outStream: PStream; var gzHdr: TgzipHeader; level: TZCompressionLevel = zcDefault; strategy: TZCompressionStrategy = zcsDefault): Integer;
var
  rSize,
    wSize,
    zResult         : LongInt;
  done              : Boolean;
  iBuffer,
    oBuffer         : PChar; //Array [0..gzBufferSize-1] of Char;
  fCrc              : Cardinal;
  zStream           : TZStreamRec;
  stamp             : Integer;

begin
  iBuffer := nil;
  oBuffer := nil;
  Result := Z_MEM_ERROR;
  try
    GetMem(iBuffer, gzBufferSize);
    GetMem(oBuffer, gzBufferSize);

    fCrc := 0;
    FillChar(zStream, SizeOf(zStream), 0);

{$IFDEF USE_EXCEPTIONS}
    ZCompressCheck(DeflateInit2(zStream, ZLevels[level], Z_DEFLATED, -MAX_WBITS,
      DEF_MEM_LEVEL, ZStrategy[strategy]));
{$ELSE}
    Result := DeflateInit2(zStream, ZLevels[level], Z_DEFLATED, -MAX_WBITS,
      DEF_MEM_LEVEL, ZStrategy[strategy]);
    if Result < 0 then Exit;
{$ENDIF}
    { windowBits is passed < 0 to suppress zlib header }
    oBuffer[0] := Char(gz_magic[0]);
    oBuffer[1] := Char(gz_magic[1]); // gz Magic
    oBuffer[2] := #08;    // gz Compression method
    oBuffer[3] := #0;
    // set mtime
    {
    Inc(gzHdr.TimeStamp,gzTimeStampCorrection);
    oBuffer[4]:=Lo(gzHdr.TimeStamp and $FFFF); oBuffer[5]:=Hi(gzHdr.TimeStamp and $FFFF);
    oBuffer[6]:=Lo(gzHdr.TimeStamp shr 16); oBuffer[7]:=Hi(gzHdr.TimeStamp shr 16);
    Dec(gzHdr.TimeStamp,gzTimeStampCorrection);
    }
    stamp := DateTimeToUnix(gzHdr.FileTime);
    oBuffer[4] := Char(Lo(stamp and $FFFF));
    oBuffer[5] := Char(Hi(stamp and $FFFF));
    oBuffer[6] := Char(Lo(stamp shr 16));
    oBuffer[7] := Char(Hi(stamp shr 16));

    // xfl, os code sets to 0
    oBuffer[8] := #0;
    oBuffer[9] := #0;

    if gzHdr.FileName <> '' then begin
      oBuffer[3] := Char(Byte(oBuffer[3]) or GZF_ORIG_NAME);
    end;
    if gzHdr.Comment <> '' then begin
      oBuffer[3] := Char(Byte(oBuffer[3]) or GZF_COMMENT);
    end;
    if gzHdr.Extra <> '' then begin
      oBuffer[3] := Char(Byte(oBuffer[3]) or GZF_EXTRA_FIELD);
    end;
    wSize := outStream.Write(oBuffer^, 10);
{$IFDEF USE_EXCEPTIONS}
    if wSize <> 10 then ZCompressCheck(Z_WRITE_ERROR);
{$ELSE}
    if wSize <> 10 then begin
      Result := Z_WRITE_ERROR;
      Exit;
    end;
{$ENDIF}

    // extra
    if (byte(oBuffer[3]) and GZF_EXTRA_FIELD) <> 0 then begin
      rSize := Length(gzHdr.Extra);
      Move(gzHdr.Extra[1], iBuffer^, rSize);
      iBuffer[rSize] := #0;
      Inc(rSize);
      wSize := outStream.Write(iBuffer^, rSize);
{$IFDEF USE_EXCEPTIONS}
      if wSize <> rSize then ZCompressCheck(Z_WRITE_ERROR);
{$ELSE}
      if wSize <> rSize then begin
        Result := Z_WRITE_ERROR;
        Exit;
      end;
{$ENDIF}
    end;
    // filename
    if (byte(oBuffer[3]) and GZF_ORIG_NAME) <> 0 then begin
      rSize := Length(gzHdr.FileName);
      Move(gzHdr.FileName[1], iBuffer^, rSize);
      iBuffer[rSize] := #0;
      Inc(rSize);
      wSize := outStream.Write(iBuffer^, rSize);
{$IFDEF USE_EXCEPTIONS}
      if wSize <> rSize then ZCompressCheck(Z_WRITE_ERROR);
{$ELSE}
      if wSize <> rSize then begin
        Result := Z_WRITE_ERROR;
        Exit;
      end;
{$ENDIF}
    end;
    // comment
    if (byte(oBuffer[3]) and GZF_COMMENT) <> 0 then begin
      rSize := Length(gzHdr.Comment);
      Move(gzHdr.Comment[1], iBuffer^, rSize);
      iBuffer[rSize] := #0;
      Inc(rSize);
      wSize := outStream.Write(iBuffer^, rSize);
{$IFDEF USE_EXCEPTIONS}
      if wSize <> rSize then ZCompressCheck(Z_WRITE_ERROR);
{$ELSE}
      if wSize <> rSize then begin
        Result := Z_WRITE_ERROR;
        Exit;
      end;
{$ENDIF}
    end;
    // hcrc

    rSize := inStream.Read(iBuffer^, gzBufferSize);
    zStream.next_out := PChar(oBuffer);
    zStream.avail_out := gzBufferSize;
    repeat
      //DoProgressEvent;
      zStream.next_in := PChar(iBuffer);
      zStream.avail_in := rSize;
      while (zStream.avail_in <> 0) do begin
        if (zStream.avail_out = 0) then begin
          zStream.next_out := PChar(oBuffer);
          wSize := outStream.Write(oBuffer^, gzBufferSize);
          if (wSize <> gzBufferSize) then begin
{$IFDEF USE_EXCEPTIONS}
            ZCompressCheck(Z_WRITE_ERROR);
{$ELSE}
            Result := Z_WRITE_ERROR;
            Exit;
{$ENDIF}
          end;
          zStream.avail_out := gzBufferSize;
        end;
{$IFDEF USE_EXCEPTIONS}
        ZCompressCheck(deflate(zStream, Z_NO_FLUSH));
{$ELSE}
        Result := deflate(zStream, Z_NO_FLUSH);
        if Result < 0 then Exit;
{$ENDIF}
      end;                // while
      fCrc := Crc32(fCrc, PChar(iBuffer), rSize);
      rSize := inStream.Read(iBuffer^, gzBufferSize);
    until rSize = 0;
    { flush buffers }
    zStream.avail_in := 0; { should be zero already anyway }
    done := False;

    repeat
      rSize := gzBufferSize - zStream.avail_out;
      if (rSize <> 0) then begin
        wSize := outStream.Write(oBuffer^, rSize);
{$IFDEF USE_EXCEPTIONS}
        if (wSize <> rSize) then ZCompressCheck(Z_WRITE_ERROR);
{$ELSE}
        if (wSize <> rSize) then begin
          Result := Z_WRITE_ERROR;
          Exit;
        end;
{$ENDIF}
        zStream.next_out := PChar(oBuffer);
        zStream.avail_out := gzBufferSize;
      end;
      if done then Break;
      zResult := deflate(zStream, Z_FINISH);
      if (rSize = 0) and (zResult = Z_BUF_ERROR) then
{$IFDEF USE_EXCEPTIONS}
      else ZCompressCheck(zResult);
{$ELSE}
      else begin
        Result := zResult;
        if Result < 0 then Exit;
      end;
{$ENDIF}
      { deflate has finished flushing only when it hasn't used up
        all the available space in the output buffer: }
      done := (zStream.avail_out <> 0) or (zResult = Z_STREAM_END);
    until False;
    wSize := outStream.Write(fCrc, 4);
{$IFDEF USE_EXCEPTIONS}
    if wSize <> 4 then ZCompressCheck(Z_WRITE_ERROR);
{$ELSE}
    if wSize <> 4 then begin
      Result := Z_WRITE_ERROR;
      Exit;
    end;
{$ENDIF}
    rSize := inStream.Size;
    wSize := outStream.Write(rSize, 4);
{$IFDEF USE_EXCEPTIONS}
    if wSize <> 4 then ZCompressCheck(Z_WRITE_ERROR);
{$ELSE}
    if wSize <> 4 then begin
      Result := Z_WRITE_ERROR;
      Exit;
    end;
{$ENDIF}
    Result := Z_OK;
  finally
    deflateEnd(zStream);
    if Assigned(iBuffer) then FreeMem(iBuffer);
    if Assigned(oBuffer) then FreeMem(oBuffer);
  end;
end;

function gZipCompressStream(inStream, outStream: PStream; level: TZCompressionLevel = zcDefault; strategy: TZCompressionStrategy = zcsDefault): Integer; overload;
var
  gzHdr             : TgzipHeader;
begin
  FillChar(gzHdr, SizeOf(gzHdr), 0);
  gzHdr.FileTime := Date;
  Result := gZipCompressStream(inStream, outStream, gzHdr, level, strategy);
end;

function gZipDecompressStreamHeader(inStream: PStream; var gzHdr: TgzipHeader): Integer;
var
  i, c, flg         : LongInt;
  fEOF              : Boolean;

  function gz_getbyte: Integer;
  var
    b, c            : Integer;
  begin
    b := 0;
    c := inStream.Read(b, 1);
    if c = 0 then begin
      fEOF := True;
      Result := Z_EOF;
    end
    else Result := b;
  end;

  function gz_getlong: Integer;
  var
    b, c            : Integer;
  begin
    b := 0;
    c := inStream.Read(b, 4);
    if c < 4 then begin
      fEOF := True;
      Result := Z_EOF;
    end
    else Result := b;
  end;
begin
  //  fTransparent := False;
  Result := Z_OK;
  fEOF := False;
  gzHdr.FileName := '';
  gzHdr.Comment := '';
  gzHdr.Extra := '';
  try
    for i := 0 to 1 do begin
      flg := gz_getbyte;
      if (flg <> gz_magic[i]) then begin
        fEOF := True;
        exit;
      end;
    end;
    c := gz_getbyte;      // method
    flg := gz_getbyte;    // flags
    if (c <> Z_DEFLATED) or ((flg and GZF_RESERVED) <> 0) then begin
      fEOF := True;
      exit;
    end;

    gzHdr.FileTime := UnixToDateTime(gz_getLong);
    gz_getbyte;
    gz_getbyte;           { skip xflags and OS code }

    if (flg and GZF_EXTRA_FIELD) <> 0 then begin // skip extra fields
      i := gz_getbyte + (gz_getbyte shl 8); // length of extra
      SetLength(gzHdr.Extra, i);
      c := inStream.Read(gzHdr.Extra, i);
      if c <> i then begin
        fEOF := True;
        Exit;
      end;
    end;
    if (flg and GZF_ORIG_NAME) <> 0 then begin // extract File Name
      repeat
        c := gz_getbyte;
        if (c <> 0) and (c <> Z_EOF) then gzHdr.FileName := gzHdr.FileName + char(c);
      until (c = 0) or (c = Z_EOF);
    end;
    if (flg and GZF_COMMENT) <> 0 then begin // extract Comment
      repeat
        c := gz_getbyte;
        if (c <> 0) and (c <> Z_EOF) then gzHdr.Comment := gzHdr.Comment + char(c);
      until (c = 0) or (c = Z_EOF);
    end;
    if (flg and GZF_HEAD_CRC) <> 0 then begin // skip head crc
      gz_getbyte;
      gz_getbyte;
    end;
  finally
    if fEOF then Result := Z_DATA_ERROR
    else Result := Z_OK;
  end;
end;

function gZipDecompressStreamBody(inStream, outStream: PStream): Integer;
var
  iBuffer,
    oBuffer         : PChar; //Array [0..gzBufferSize-1] of Char;
  fCrc              : Cardinal;
  zStream           : TZStreamRec;
  rSize,
    wSize           : LongInt;
  startCRC          : PChar;
  fileCRC,
    fileSize        : Cardinal;
  fEOF              : Boolean;

  function gz_getbyte: Integer;
  begin
    //  if (eof) then result:=-1;
    if (zStream.avail_in = 0) then begin
      zStream.avail_in := inStream.Read(iBuffer^, gzBufferSize);
      if (zStream.avail_in = 0) then begin
        Result := Z_EOF;
        fEOF := True;
        exit;
      end
      else zStream.next_in := PChar(iBuffer);
    end;
    Dec(zStream.avail_in);
    Result := Byte(zStream.next_in[0]);
    Inc(zStream.next_in);
  end;

  function gz_getLong: Cardinal;
  var
    c               : Integer;
  begin
    c := gz_getbyte;
    c := c + gz_getbyte shl 8;
    c := c + gz_getbyte shl 16;
    c := c + gz_getbyte shl 24;
    Result := Cardinal(c);
  end;
begin
  iBuffer := nil;
  oBuffer := nil;
  Result := Z_MEM_ERROR;
  try
    GetMem(iBuffer, gzBufferSize);
    GetMem(oBuffer, gzBufferSize);
    fEOF := False;
    {Check the gzip header of a gz_stream opened for reading.
     Set the stream mode to transparent if the gzip magic header is not present.}

    FillChar(zStream, SizeOf(zStream), 0);
    zStream.next_in := pChar(iBuffer);
    fCRC := 0;
    { windowBits is passed < 0 to tell that there is no zlib header }
{$IFDEF USE_EXCEPTIONS}
    ZDecompressCheck(InflateInit2(zStream, -MAX_WBITS));
{$ELSE}
    Result := InflateInit2(zStream, -MAX_WBITS);
    if Result < 0 then Exit;
{$ENDIF}
    while not fEOF do begin
      // gzread()
  //    DoProgressEvent;
      startCRC := PChar(oBuffer);
      zStream.next_out := PChar(oBuffer);
      zStream.avail_out := gzBufferSize;
//      rSize := 0;
      Result := Z_OK;
      while zStream.avail_out <> 0 do begin
        // not transparent
        if (zStream.avail_in = 0) and (not fEOF) then begin
          zStream.avail_in := inStream.Read(iBuffer^, gzBufferSize);
          if (zStream.avail_in = 0) then fEOF := True;
          zStream.next_in := PChar(iBuffer);
        end;
        Result := inflate(zStream, Z_NO_FLUSH);
        if (Result = Z_STREAM_END) then begin
          { Check CRC and original size }
          fCrc := crc32(fCrc, PChar(StartCRC), (zStream.next_out - startCRC));
          startCRC := zStream.next_out;

          fileCRC := gz_getLong;
          fileSize := gz_getLong;
          if (fCrc <> fileCRC) then
{$IFDEF USE_EXCEPTIONS}
            ZDecompressCheck(Z_CRC_ERROR)
{$ELSE}
            Result := Z_CRC_ERROR
{$ENDIF}
          else
            if (Cardinal(zStream.total_out) <> fileSize) then
{$IFDEF USE_EXCEPTIONS}
              ZDecompressCheck(Z_SIZE_ERROR)
{$ELSE}
              Result := Z_SIZE_ERROR
{$ENDIF}
            else begin
              if zStream.avail_in > 0 then inStream.Seek(-zStream.avail_in, spCurrent);
              fEOF := True;
            end;
        end;
        if (Result <> Z_OK) or (fEOF) then break;
      end;                // while zStream.avail_out<>0
      // end of gzread()

{$IFDEF USE_EXCEPTIONS}
      ZDecompressCheck(Result);
{$ELSE}
      if Result < 0 then Exit;
{$ENDIF}
      fCrc := crc32(fCrc, PChar(oBuffer), (zStream.next_out - startCRC));
      rSize := gzBufferSize - zStream.avail_out;

{$IFDEF USE_EXCEPTIONS}
      if rSize < 0 then ZDecompressCheck(rSize);
{$ELSE}
      if rSize <= 0 then break;
{$ENDIF}
      wSize := outStream.Write(oBuffer^, rSize);
{$IFDEF USE_EXCEPTIONS}
      if (rSize <> wSize) then ZDecompressCheck(Z_WRITE_ERROR);
{$ELSE}
      if (rSize <> wSize) then begin
        Result := Z_WRITE_ERROR;
        Exit;
      end;
{$ENDIF}
    end;
    if Result = Z_STREAM_END then Result := Z_OK;
  finally
    inflateEnd(zStream);
    if Assigned(iBuffer) then FreeMem(iBuffer);
    if Assigned(oBuffer) then FreeMem(oBuffer);
  end;
end;

function gZipDecompressStream(inStream, outStream: PStream; var gzHdr: TgzipHeader): Integer;
begin
  Result := gZipDecompressStreamHeader(inStream, gzHdr);
  if (Result >= 0) then
    Result := gZipDecompressStreamBody(inStream, outStream);
end;

function gZipDecompressString(const S: String): String;
var
  Rslt:      Integer;
  gzHdr:     TgzipHeader;
  inStream:  PStream;
  outStream: PStream;
begin
  Result := '';
  inStream := NewExMemoryStream(@S[1], Length(S));
  Rslt := gZipDecompressStreamHeader(inStream, gzHdr);
  if (Rslt >= 0) then begin
    outStream := NewMemoryStream;
    Rslt := gZipDecompressStreamBody(inStream, outStream);
    if (Rslt >= 0) then begin
      outStream.Position := 0;
      Result := outStream.ReadStrLen(outStream.Size);
    end;
    outStream.Free;
  end;
  inStream.Free;
end;

{****************************************************************************}
{** BZip implementation *****************************************************}
{****************************************************************************}
{$IFDEF USE_EXCEPTIONS}

function CCheck(code: Integer): Integer;
begin
  Result := code;
  if code < 0 then
    raise Exception.CreateFMT(e_Convert, 'Compression Error %d - %s', [code, BZ_Error_Msg[-code]]);
end;

function DCheck(code: Integer): Integer;
begin
  Result := code;
  if code < 0 then
    raise Exception.CreateFMT(e_Convert, 'Decompression Error %d - %s', [code, BZ_Error_Msg[-code]]);
end;
{$ENDIF}

function BZCompressBuf(const InBuf: Pointer; InBytes: Integer; out OutBuf: Pointer; out OutBytes: Integer): Integer;
var
  strm              : TBZStreamRec;
  P                 : Pointer;
begin
  FillChar(strm, sizeof(strm), 0);
  strm.bzalloc := bzip2AllocMem;
  strm.bzfree := bzip2FreeMem;
  OutBytes := ((InBytes + (InBytes div 10) + 12) + 255) and not 255;
  GetMem(OutBuf, OutBytes);
  Result := BZ_OK;
  try
    strm.next_in := InBuf;
    strm.avail_in := InBytes;
    strm.next_out := OutBuf;
    strm.avail_out := OutBytes;
{$IFDEF USE_EXCEPTIONS}
    CCheck(BZ2_bzCompressInit(strm, 9, 0, 0));
{$ELSE}
    Result := BZ2_bzCompressInit(strm, 9, 0, 0);
    if Result < 0 then Exit;
{$ENDIF}
    try
{$IFDEF USE_EXCEPTIONS}
      Result := CCheck(BZ2_bzCompress(strm, BZ_FINISH));
{$ELSE}
      Result := BZ2_bzCompress(strm, BZ_FINISH);
      if Result < 0 then Exit;
{$ENDIF}
      while Result <> BZ_STREAM_END do begin
        P := OutBuf;
        Inc(OutBytes, 256);
        ReallocMem(OutBuf, OutBytes);
        strm.next_out := PChar(Integer(OutBuf) + (Integer(strm.next_out) - Integer(P)));
        strm.avail_out := 256;
{$IFDEF USE_EXCEPTIONS}
        Result := CCheck(BZ2_bzCompress(strm, BZ_FINISH));
{$ELSE}
        Result := BZ2_bzCompress(strm, BZ_FINISH);
        if Result < 0 then Exit;
{$ENDIF}
      end;
    finally
{$IFDEF USE_EXCEPTIONS}
      CCheck(BZ2_bzCompressEnd(strm));
{$ELSE}
      BZ2_bzCompressEnd(strm);
{$ENDIF}
    end;
    ReallocMem(OutBuf, strm.total_out_lo32);
    OutBytes := strm.total_out_lo32;
{$IFDEF USE_EXCEPTIONS}
  except
    FreeMem(outBuf);
    raise;
{$ELSE}
  finally
    if Result < 0 then FreeMem(outBuf);
{$ENDIF}
  end;
end;

function BZDecompressBuf(const InBuf: Pointer; InBytes: Integer; OutEstimate: Integer; out OutBuf: Pointer; out OutBytes: Integer): Integer;
var
  strm              : TBZStreamRec;
  P                 : Pointer;
  BufInc            : Integer;
begin
  FillChar(strm, sizeof(strm), 0);
  strm.bzalloc := bzip2AllocMem;
  strm.bzfree := bzip2FreeMem;
  BufInc := (InBytes + 255) and not 255;
  Result := BZ_OK;
  if OutEstimate = 0 then
    OutBytes := BufInc
  else
    OutBytes := OutEstimate;
  GetMem(OutBuf, OutBytes);
  try
    strm.next_in := InBuf;
    strm.avail_in := InBytes;
    strm.next_out := OutBuf;
    strm.avail_out := OutBytes;
{$IFDEF USE_EXCEPTIONS}
    DCheck(BZ2_bzDecompressInit(strm, 0, 0));
{$ELSE}
    Result := BZ2_bzDecompressInit(strm, 0, 0);
    if Result < 0 then Exit;
{$ENDIF}
    try
{$IFDEF USE_EXCEPTIONS}
      Result := DCheck(BZ2_bzDecompress(strm));
{$ELSE}
      Result := BZ2_bzDecompress(strm);
      if Result < 0 then Exit;
{$ENDIF}
      while Result <> BZ_STREAM_END do begin
        P := OutBuf;
        Inc(OutBytes, BufInc);
        ReallocMem(OutBuf, OutBytes);
        strm.next_out := PChar(Integer(OutBuf) + (Integer(strm.next_out) - Integer(P)));
        strm.avail_out := BufInc;
{$IFDEF USE_EXCEPTIONS}
        Result := DCheck(BZ2_bzDecompress(strm));
{$ELSE}
        Result := BZ2_bzDecompress(strm);
        if Result < 0 then Exit;
{$ENDIF}
      end;
    finally
{$IFDEF USE_EXCEPTIONS}
      DCheck(BZ2_bzDecompressEnd(strm));
{$ELSE}
      BZ2_bzDecompressEnd(strm);
{$ENDIF}
    end;
    ReallocMem(OutBuf, strm.total_out_lo32);
    OutBytes := strm.total_out_lo32;
  except
    FreeMem(OutBuf);
    raise
  end;
end;

function BZCompressStream(inStream, outStream: PStream; BlockSize100k: TBlockSize100k = 5): Integer;
var
  FBZRec            : TBZStreamRec;
  iBuffer,
    oBuffer         : PChar;
  wSize,
    rSize           : Integer;
begin
  Result := BZ_MEM_ERROR;
  iBuffer := nil;
  oBuffer := nil;
  FillChar(FBZRec, SizeOf(FBZRec), 0);
  //  FBZRec.bzalloc := bzip2AllocMem;
  //  FBZRec.bzfree := bzip2FreeMem;
  try
    GetMem(iBuffer, bzBufferSize);
    GetMem(oBuffer, bzBufferSize);
{$IFDEF USE_EXCEPTIONS}
    CCheck(BZ2_bzCompressInit(FBZRec, BlockSize100k, 0, 0));
{$ELSE}
    Result := BZ2_bzCompressInit(FBZRec, BlockSize100k, 0, 0);
    if Result < 0 then Exit;
{$ENDIF}
    FBZRec.next_out := PChar(oBuffer);
    FBZRec.avail_out := bzBufferSize;
    rSize := inStream.Read(iBuffer^, bzBufferSize);
    repeat
      //DoProgressEvent;
      FBZRec.next_in := PChar(iBuffer);
      FBZRec.avail_in := rSize;
      while (FBZRec.avail_in > 0) do begin
        if (FBZRec.avail_out = 0) then begin
          wSize := outStream.Write(oBuffer^, bzBufferSize);
          if (wSize <> bzBufferSize) then begin
{$IFDEF USE_EXCEPTIONS}
            CCheck(BZ_IO_ERROR);
{$ELSE}
            Result := BZ_IO_ERROR;
            Exit;
{$ENDIF}
          end;
          FBZRec.next_out := PChar(oBuffer);
          FBZRec.avail_out := bzBufferSize;
        end;
{$IFDEF USE_EXCEPTIONS}
        CCheck(BZ2_bzCompress(FBZRec, BZ_RUN));
{$ELSE}
        Result := BZ2_bzCompress(FBZRec, BZ_RUN);
        if Result < 0 then Exit;
{$ENDIF}
      end;                // while
      rSize := inStream.Read(iBuffer^, bzBufferSize);
    until rSize = 0;
    { flush buffers }
    FBZRec.avail_in := 0; { should be zero already anyway }
    repeat
{$IFDEF USE_EXCEPTIONS}
      Result := CCheck(BZ2_bzCompress(FBZRec, BZ_FINISH));
{$ELSE}
      Result := BZ2_bzCompress(FBZRec, BZ_FINISH);
      if Result < 0 then Break;
{$ENDIF}
      rSize := bzBufferSize - FBZRec.avail_out;
      wSize := outStream.Write(oBuffer^, rSize);
{$IFDEF USE_EXCEPTIONS}
      if (wSize <> rSize) then CCheck(BZ_IO_ERROR);
{$ELSE}
      if (wSize <> rSize) then begin
        Result := BZ_IO_ERROR;
        break;
      end;
{$ENDIF}
      FBZRec.next_out := PChar(oBuffer);
      FBZRec.avail_out := bzBufferSize;
    until Result = BZ_STREAM_END;
  finally
    if Result = BZ_STREAM_END then Result := BZ_OK;
    BZ2_bzCompressEnd(FBZRec);
    if Assigned(iBuffer) then FreeMem(iBuffer);
    if Assigned(oBuffer) then FreeMem(oBuffer);
  end;

end;

function BZDecompressStream(inStream, outStream: PStream): Integer;
var
  FBZRec            : TBZStreamRec;
  iBuffer,
    oBuffer         : PChar;
  wSize,
    rSize           : Integer;
begin
  Result := BZ_MEM_ERROR;
  iBuffer := nil;
  oBuffer := nil;
  FillChar(FBZRec, SizeOf(FBZRec), 0);
  try
    GetMem(iBuffer, bzBufferSize);
    GetMem(oBuffer, bzBufferSize);
{$IFDEF USE_EXCEPTIONS}
    DCheck(BZ2_bzDecompressInit(FBZRec, 0, 0));
{$ELSE}
    Result := BZ2_bzDecompressInit(FBZRec, 0, 0);
    if Result < 0 then Exit;
{$ENDIF}
    rSize := inStream.Read(iBuffer^, bzBufferSize);
    FBZRec.next_in := PChar(iBuffer);
    FBZRec.avail_in := rSize;
    repeat
      FBZRec.next_out := PChar(oBuffer);
      FBZRec.avail_out := bzBufferSize;
      Result := 0;
      while (FBZRec.avail_out > 0) and (Result <> BZ_STREAM_END) do begin
{$IFDEF USE_EXCEPTIONS}
        Result := CCheck(BZ2_bzDecompress(FBZRec));
{$ELSE}
        Result := BZ2_bzDecompress(FBZRec);
        if Result < 0 then Break;
{$ENDIF}
        if FBZRec.avail_in = 0 then begin
          rSize := inStream.Read(iBuffer^, bzBufferSize);
          FBZRec.next_in := PChar(iBuffer);
          FBZRec.avail_in := rSize;
        end;
      end;
      FBZRec.avail_out := bzBufferSize - FBZRec.avail_out;
      wSize := outStream.Write(oBuffer^, FBZRec.avail_out);
{$IFDEF USE_EXCEPTIONS}
      if FBZRec.avail_out <> wSize then CCheck(BZ_IO_ERROR);
{$ELSE}
      if FBZRec.avail_out <> wSize then Result := BZ_IO_ERROR;
{$ENDIF}
    until (rSize = 0) or (Result < 0);
  finally
    if Result = BZ_STREAM_END then Result := BZ_OK;
    BZ2_bzDecompressEnd(FBZRec);
    if Assigned(iBuffer) then FreeMem(iBuffer);
    if Assigned(oBuffer) then FreeMem(oBuffer);
  end;
end;

end.
