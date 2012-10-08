unit tags;
{$include compilers.inc}
interface

uses wat_api,windows;

{$DEFINE Interface}
{$include tag_id3v2.inc}
{$include tag_id3v1.inc}
{$include tag_apev2.inc}

implementation

uses common,io,utils;

{$UNDEF Interface}
{$include tag_id3v2.inc}
{$include tag_id3v1.inc}
{$include tag_apev2.inc}

end.