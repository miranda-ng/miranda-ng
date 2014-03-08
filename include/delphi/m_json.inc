{
Miranda IM: the free IM client for Microsoft* Windows*

Copyright 2000-2008 Miranda ICQ/IM project,
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

{$IFNDEF MIM_LIBJSON}
{$DEFINE MIM_LIBJSON}

type
  PJSONNODE = ^TJSONNODE;
  TJSONNODE = pointer;

procedure json_free(str:pointer); stdcall;
          external CoreDLL name 'json_free';
procedure json_delete(node:PJSONNODE); stdcall;
          external CoreDLL name 'json_delete';

function json_parse(json:PAnsiChar):PJSONNODE; stdcall;
          external CoreDLL name 'json_parse';
function json_strip_white_space(json:PAnsiChar):PWideChar; stdcall;
          external CoreDLL name 'json_strip_white_space';

function json_validate(json:PAnsiChar):PJSONNODE; stdcall;
          external CoreDLL name 'json_validate';

{
	stuff that's in class TJSONNode
}
//ctors
function json_new_a(name:PAnsiChar; value:PAnsiChar):PJSONNODE; stdcall;
          external CoreDLL name 'json_new_a';
function json_new_i(name:PAnsiChar; value:long):PJSONNODE; stdcall;
          external CoreDLL name 'json_new_i';
function json_new_f(name:PAnsiChar; value:double):PJSONNODE; stdcall;
          external CoreDLL name 'json_new_f';
function json_new_b(name:PAnsiChar; value:int):PJSONNODE; stdcall;
          external CoreDLL name 'json_new_b';
function json_new(typ:AnsiChar):PJSONNODE; stdcall;
          external CoreDLL name 'json_new';
function json_copy(const orig:PJSONNODE):PJSONNODE; stdcall;
          external CoreDLL name 'json_copy';
function json_duplicate(const orig:PJSONNODE):PJSONNODE; stdcall;
          external CoreDLL name 'json_duplicate';

//assignment
procedure json_set_a(node:PJSONNODE; value:PAnsiChar); stdcall;
          external CoreDLL name 'json_set_a';
procedure json_set_i(node:PJSONNODE; value:long); stdcall;
          external CoreDLL name 'json_set_i';
procedure json_set_f(node:PJSONNODE; value:double); stdcall;
          external CoreDLL name 'json_set_f';
procedure json_set_b(node:PJSONNODE; value:int); stdcall;
          external CoreDLL name 'json_set_b';
procedure json_set_n(node:PJSONNODE; const orig: PJSONNODE); stdcall;
          external CoreDLL name 'json_set_n';

//inspectors
function json_type(const node:PJSONNODE):AnsiChar; stdcall;
          external CoreDLL name 'json_type';
function json_size(const node:PJSONNODE):size_t; stdcall;
          external CoreDLL name 'json_size';
function json_empty(const node:PJSONNODE):int; stdcall;
          external CoreDLL name 'json_empty';
function json_name(const node:PJSONNODE):PAnsiChar; stdcall;
          external CoreDLL name 'json_name';

function json_get_comment(const node:PJSONNODE):PAnsiChar; stdcall;
          external CoreDLL name 'json_get_comment';

function json_as_string(const node:PJSONNODE):PWideChar; stdcall;
          external CoreDLL name 'json_as_string';
function json_as_int(const node:PJSONNODE):long; stdcall;
          external CoreDLL name 'json_as_int';
function json_as_float(const node:PJSONNODE):double; stdcall;
          external CoreDLL name 'json_as_float';
function json_as_bool(const node:PJSONNODE):int; stdcall;
          external CoreDLL name 'json_as_bool';
function json_as_node(const node:PJSONNODE):PJSONNODE; stdcall;
          external CoreDLL name 'json_as_node';
function json_as_array(const node:PJSONNODE):PJSONNODE; stdcall;
          external CoreDLL name 'json_as_array';

function json_write(const node:PJSONNODE):PWideChar; stdcall;
          external CoreDLL name 'json_write';
function json_write_formatted(const node:PJSONNODE):PWideChar; stdcall;
          external CoreDLL name 'json_write_formatted';

//modifiers
procedure json_set_name(node:PJSONNODE; name:PAnsiChar); stdcall;
          external CoreDLL name 'json_set_name';

procedure json_set_comment(node:PJSONNODE; comment:PAnsiChar); stdcall;
          external CoreDLL name 'json_set_comment';

procedure json_clear(node:PJSONNODE); stdcall;
          external CoreDLL name 'json_clear';
procedure json_nullify(node:PJSONNODE); stdcall;
          external CoreDLL name 'json_nullify';
procedure json_swap(node:PJSONNODE; node2:PJSONNODE); stdcall;
          external CoreDLL name 'json_swap';
procedure json_merge(node:PJSONNODE; node2:PJSONNODE); stdcall;
          external CoreDLL name 'json_merge';

procedure json_preparse(node:PJSONNODE); stdcall;
          external CoreDLL name 'json_preparse';

procedure json_set_binary(node:PJSONNODE; data:pointer; length:ulong); stdcall;
          external CoreDLL name 'json_set_binary';

procedure json_cast(node:PJSONNODE; typ:AnsiChar); stdcall;
          external CoreDLL name 'json_cast';

//children access
procedure json_reserve(node:PJSONNODE; siz:size_t); stdcall;
          external CoreDLL name 'json_reserve';
function  json_at(node:PJSONNODE; pos:size_t):PJSONNODE; stdcall;
          external CoreDLL name 'json_at';
function  json_get(node:PJSONNODE; name:PAnsiChar):PJSONNODE; stdcall;
          external CoreDLL name 'json_get';

function json_get_nocase(node:PJSONNODE; name:PAnsiChar):PJSONNODE; stdcall;
          external CoreDLL name 'json_get_nocase';
function json_pop_back_nocase(node:PJSONNODE; name:PAnsiChar):PJSONNODE; stdcall;
          external CoreDLL name 'json_pop_back_nocase';

procedure json_push_back(node:PJSONNODE; node2:PJSONNODE); stdcall;
          external CoreDLL name 'json_push_back';
function  json_pop_back_at(node:PJSONNODE; pos:size_t):PJSONNODE; stdcall;
          external CoreDLL name 'json_pop_back_at';
function  json_pop_back(node:PJSONNODE; name:PAnsiChar):PJSONNODE; stdcall;
          external CoreDLL name 'json_pop_back';

//comparsion
function json_equal(node:PJSONNODE; node2:PJSONNODE):int; stdcall;
          external CoreDLL name 'json_equal';

{$ENDIF}
