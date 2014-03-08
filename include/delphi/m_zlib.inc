{
Miranda NG: the free IM client for Microsoft* Windows*

Copyright 2012 Miranda NG project,
all portions of this codebase are copyrighted to the people
listed in contributors.txt.

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
}

{$IFNDEF M_ZLIB}
{$DEFINE M_ZLIB}

const
  ZlibDLL = 'zlib.dll';

  Z_NO_FLUSH      = 0;
  Z_PARTIAL_FLUSH = 1;
  Z_SYNC_FLUSH    = 2;
  Z_FULL_FLUSH    = 3;
  Z_FINISH        = 4;
  Z_BLOCK         = 5;
  Z_TREES         = 6;

  Z_OK            =  0;
  Z_STREAM_END    =  1;
  Z_NEED_DICT     =  2;
  Z_ERRNO         = -1;
  Z_STREAM_ERROR  = -2;
  Z_DATA_ERROR    = -3;
  Z_MEM_ERROR     = -4;
  Z_BUF_ERROR     = -5;
  Z_VERSION_ERROR = -6;

  Z_NO_COMPRESSION       =  0;
  Z_BEST_SPEED           =  1;
  Z_BEST_COMPRESSION     =  9;
  Z_DEFAULT_COMPRESSION  = -1;

  Z_FILTERED            = 1;
  Z_HUFFMAN_ONLY        = 2;
  Z_RLE                 = 3;
  Z_FIXED               = 4;
  Z_DEFAULT_STRATEGY    = 0;

  Z_BINARY   = 0;
  Z_TEXT     = 1;
  Z_ASCII    = 1;
  Z_UNKNOWN  = 2;

  Z_DEFLATED = 8;

type
  alloc_func = function(opaque: pointer; items, size: integer): pointer;
                 cdecl;
  free_func  = procedure(opaque, address: pointer);
                 cdecl;

  in_func    = function(opaque: pointer; var buf: PByte): integer;
                 cdecl;
  out_func   = function(opaque: pointer; buf: PByte; size: integer): integer;
                 cdecl;

  z_streamp = ^z_stream;
  z_stream = packed record
    next_in: PChar;       (* next input byte *)
    avail_in: integer;    (* number of bytes available at next_in *)
    total_in: LongInt;    (* total nb of input bytes read so far *)

    next_out: PChar;      (* next output byte should be put there *)
    avail_out: integer;   (* remaining free space at next_out *)
    total_out: LongInt;   (* total nb of bytes output so far *)

    msg: PChar;           (* last error message, NULL if no error *)
    state: pointer;       (* not visible by applications *)

    zalloc: alloc_func;   (* used to allocate the internal state *)
    zfree: free_func;     (* used to free the internal state *)
    opaque: pointer;      (* private data object passed to zalloc and zfree *)

    data_type: integer;   (* best guess about the data type: ascii or binary *)
    adler: LongInt;       (* adler32 value of the uncompressed data *)
    reserved: LongInt;    (* reserved for future use *)
  end;

function deflateInit(var strm: z_stream; level: integer): integer; cdecl;
                 external ZlibDLL name 'deflateInit_';

function deflate(var strm: z_stream; flush: integer): integer; cdecl;
                 external ZlibDLL name 'deflate';

function deflateEnd(var strm: z_stream): integer; cdecl;
                 external ZlibDLL name 'deflateEnd';

function inflateInit(var strm: z_stream): integer; cdecl;
                 external ZlibDLL name 'inflateInit_';

function inflate(var strm: z_stream; flush: integer): integer; cdecl;
                 external ZlibDLL name 'inflate';

function inflateEnd(var strm: z_stream): integer; cdecl;
                 external ZlibDLL name 'inflateEnd';

{$ENDIF}
