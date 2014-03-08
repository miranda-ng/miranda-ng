{$IFDEF VER90}
              {$DEFINE _D2}
              {$DEFINE _D2orD3}
              {$DEFINE _D2orD3orD4}
{$ENDIF}

{$IFDEF VER100}
              {$DEFINE _D3}
              {$DEFINE _D3orHigher}
              {$DEFINE _D2orD3}
              {$DEFINE _D2orD3orD4}
              {$DEFINE _D3orD4}
{$ENDIF}

{$IFDEF VER120}
              {$DEFINE _D3orHigher}
              {$DEFINE _D3orD4}
              {$DEFINE _D4}
              {$DEFINE _D4orHigher}
              {$DEFINE _D2orD3orD4}
                  {$IFnDEF KOL_MCK}
                           {$DEFINE INPACKAGE}
                  {$ENDIF}
{$ENDIF}

{$IFDEF VER130}
              {$DEFINE _D3orHigher}
              {$DEFINE _D4orHigher}
              {$DEFINE _D5}
              {$DEFINE _D5orHigher}
{$ENDIF}

{$IFDEF VER140}
              {$DEFINE _D3orHigher}
              {$DEFINE _D4orHigher}
              {$DEFINE _D5orHigher}
              {$DEFINE _D6}
              {$DEFINE _D6orHigher}
{$ENDIF}

{$IFDEF VER150}
              {$DEFINE _D3orHigher}
              {$DEFINE _D4orHigher}
              {$DEFINE _D5orHigher}
              {$DEFINE _D6orHigher}
              {$DEFINE _D7}
              {$DEFINE _D7orHigher}
{$WARN UNIT_DEPRECATED OFF}
{$WARN SYMBOL_PLATFORM OFF}
{$WARN UNSAFE_TYPE OFF}
{$WARN UNSAFE_CAST OFF}
{$WARN UNSAFE_CODE OFF}
{$ENDIF}

{$IFDEF VER160} // Delphi 8
Delphi version 8 not supported! (delphi 8 is .net only)
{$ENDIF}

{$IFDEF VER170} // Delphi 2005
             {$DEFINE _D3orHigher}
             {$DEFINE _D4orHigher}
             {$DEFINE _D5orHigher}
             {$DEFINE _D6orHigher}
             {$DEFINE _D7orHigher}
             {$DEFINE _D8}
             {$DEFINE _D8orHigher}
             {$DEFINE _D2005}
             {$DEFINE _D2005orHigher}
{$WARN UNIT_DEPRECATED OFF}
{$WARN SYMBOL_PLATFORM OFF}
{$WARN UNSAFE_TYPE OFF}
{$WARN UNSAFE_CAST OFF}
{$WARN UNSAFE_CODE OFF}
{$ENDIF}

{$IFDEF VER180} // Delphi 2006
             {$DEFINE _D3orHigher}
             {$DEFINE _D4orHigher}
             {$DEFINE _D5orHigher}
             {$DEFINE _D6orHigher}
             {$DEFINE _D7orHigher}
             {$DEFINE _D8orHigher}
             {$DEFINE _D2005}
             {$DEFINE _D2005orHigher}
             {$DEFINE _D2006orHigher}
{$WARN UNIT_DEPRECATED OFF}
{$WARN SYMBOL_PLATFORM OFF}
{$WARN UNSAFE_TYPE OFF}
{$WARN UNSAFE_CAST OFF}
{$WARN UNSAFE_CODE OFF}
{$ENDIF}

{$IFDEF _D2005orHigher}

  // by Thaddy de Koning:
  {$IFDEF VER185} // Delphi 2007 ( and Highlander )
               {$DEFINE _D3orHigher}
               {$DEFINE _D4orHigher}
               {$DEFINE _D5orHigher}
               {$DEFINE _D6orHigher}
               {$DEFINE _D7orHigher}
               {$DEFINE _D2005orHigher}
               {$DEFINE _D2006orHigher}
               {$DEFINE _D2007}
               {$DEFINE _D2007orHigher}
  {$WARN UNIT_DEPRECATED OFF}
  {$WARN SYMBOL_PLATFORM OFF}
  {$WARN UNSAFE_TYPE OFF}
  {$WARN UNSAFE_CAST OFF}
  {$WARN UNSAFE_CODE OFF}
  {$ENDIF}

{$INLINE OFF}
{$ENDIF}

{$IFDEF VER200} // Delphi 2009
             {$DEFINE _D3orHigher}
             {$DEFINE _D4orHigher}
             {$DEFINE _D5orHigher}
             {$DEFINE _D6orHigher}
             {$DEFINE _D7orHigher}
             {$DEFINE _D2005orHigher}
             {$DEFINE _D2006orHigher}
             {$DEFINE _D2007orHigher}
             {$DEFINE _D2009}
             {$DEFINE _D2009orHigher}
{$WARN UNIT_DEPRECATED OFF}
{$WARN SYMBOL_PLATFORM OFF}
{$WARN UNSAFE_TYPE OFF}
{$WARN UNSAFE_CAST OFF}
{$WARN UNSAFE_CODE OFF}
{$ENDIF}

{$IFDEF VER210} // Delphi 2010
	{$DEFINE _D3orHigher}
	{$DEFINE _D4orHigher}
	{$DEFINE _D5orHigher}
	{$DEFINE _D6orHigher}
	{$DEFINE _D7orHigher}
	{$DEFINE _D2005orHigher}
	{$DEFINE _D2006orHigher}
	{$DEFINE _D2007orHigher}
	{$DEFINE _D2009orHigher}
	{$DEFINE _D2010}
	{$DEFINE _D2010orHigher}
	{$WARN UNIT_DEPRECATED OFF}
	{$WARN SYMBOL_PLATFORM OFF}
	{$WARN UNSAFE_TYPE OFF}
	{$WARN UNSAFE_CAST OFF}
	{$WARN UNSAFE_CODE OFF}
{$ENDIF}

{$IFDEF VER220} // Delphi XE
	{$DEFINE _D3orHigher}
	{$DEFINE _D4orHigher}
	{$DEFINE _D5orHigher}
	{$DEFINE _D6orHigher}
	{$DEFINE _D7orHigher}
	{$DEFINE _D2005orHigher}
	{$DEFINE _D2006orHigher}
	{$DEFINE _D2007orHigher}
	{$DEFINE _D2009orHigher}
	{$DEFINE _D2010orHigher}
	{$DEFINE _DXE}
	{$DEFINE _DXEorHigher}
    {$WARN UNIT_DEPRECATED OFF}
	{$WARN SYMBOL_PLATFORM OFF}
	{$WARN UNSAFE_TYPE OFF}
	{$WARN UNSAFE_CAST OFF}
	{$WARN UNSAFE_CODE OFF}
{$ENDIF}

{$IFDEF VER230} // Delphi XE2
	{$DEFINE _D3orHigher}
	{$DEFINE _D4orHigher}
	{$DEFINE _D5orHigher}
	{$DEFINE _D6orHigher}
	{$DEFINE _D7orHigher}
	{$DEFINE _D2005orHigher}
	{$DEFINE _D2006orHigher}
	{$DEFINE _D2007orHigher}
	{$DEFINE _D2009orHigher}
	{$DEFINE _D2010orHigher}
	{$DEFINE _DXE}
	{$DEFINE _DXEorHigher}
	{$DEFINE _DXE2}
	{$DEFINE _DXE2orHigher}
	{$DEFINE PAS_VERSION}
  {$WARN UNIT_DEPRECATED OFF}
	{$WARN SYMBOL_PLATFORM OFF}
	{$WARN UNSAFE_TYPE OFF}
	{$WARN UNSAFE_CAST OFF}
	{$WARN UNSAFE_CODE OFF}
{$ENDIF}

(*
// TODO: check DLL project
{$IFNDEF NO_STRIP_RELOC}
  // by Thaddy de Koning:
  {$IFDEF _D2006orHigher}
  // strips relocs, like stripreloc.exe does
  {$SetPEFlags 1}
//	{$SETPEFlAGS IMAGE_FILE_RELOCS_STRIPPED or IMAGE_FILE_DEBUG_STRIPPED or IMAGE_FILE_LINE_NUMS_STRIPPED or IMAGE_FILE_LOCAL_SYMS_STRIPPED or IMAGE_FILE_REMOVABLE_RUN_FROM_SWAP or IMAGE_FILE_NET_RUN_FROM_SWAP}
  {$ENDIF}
{$ENDIF}
*)
{$IFDEF FPC}
{------------------------------------
by Thaddy de Koning:

FPC version 2.1.1 is very compatible with Delphi and kol now.
You can simply use the $(DELPHI)\source\rtl\win\*.pas files from Delphi 4/5 instead of the prepared files that were needed for 
FPC1.X
 
That is all to have full compatibility.
------------------------------------}
  {$DEFINE PAS_VERSION}
  {$IFDEF VER2}
   {$DEFINE _D3orHigher}
   {$DEFINE _D4orHigher}
   {$DEFINE _D5orHigher}
   {$DEFINE _D6orHigher}
   {$DEFINE _D7}
   {$DEFINE _D7orHigher}
  {$ENDIF}
{$ENDIF FPC}

{$IFNDEF _NOT_KOLCtrlWrapper_}
 {$DEFINE _KOLCtrlWrapper_}
{$ENDIF}

{$IFNDEF _NOT_KOLCtrlWrapper_}
  {$DEFINE _KOLCtrlWrapper_}
{$ENDIF}

//// from delphidef.inc ////

{$IFDEF WIN64}
  {$DEFINE x64}
  {$DEFINE PAS_VERSION}
{$ENDIF}

//{$DEFINE _FPC}
{$DEFINE ASM_VERSION} // Comment this line to produce Pascal code.
                      // Or, just add PAS_VERSION to conditionals
                      // of your project (must be rebuilt).

{$IFDEF ASM_VERSION}
  {$IFDEF PAS_VERSION}
    {$UNDEF ASM_VERSION}
    // To compile a project with ASM_VERSION option turned off,
    // define a symbol PAS_VERSION in project options.
  {$ENDIF}
{$ENDIF}

//{$DEFINE USE_CONSTRUCTORS}
// Comment this line to produce smaller code if constructors are not used.
// When uncommented, this definition allows to create descendant controls
// and objects overriding constructors, which are actually members of objects.
// Otherwise, global functions (usually named New<ObjectName>) are used to
// create and initialize object instances. This gives smaller code, but
// prevents from using OOP inheritance.
// Note: creating descendant objects derived from TObj does not require using
// of this option. It is actually needed only for deriving new controls on
// base of TControl. See also option USE_CUSTOMEXTENSIONS below.

//{$DEFINE USE_CUSTOMEXTENSIONS}
// Uncomment this option or add it to your project conditional defines,
// if You wish to extend existing TControl object from
// the inner of those. When this option is turned on, include directive at the
// tail of TControl declaration is enabled, causing a compiler to include your
// portion of source directly into the TControl body. See comments near this
// directive there. (Search the word: USE_CUSTOMEXTENSIONS).
// Please note, that this option is not fully supported now.

//{$IFNDEF NOT_UNLOAD_RICHEDITLIB}
//  {$DEFINE UNLOAD_RICHEDITLIB}
//{$ENDIF}
// You can freely comment this directive. 1st, if the application does not
// use richedit control. 2nd, even if it does, freeing the library handle
// actually is not needed.
// Another way to turn this option off is to define symbol NOT_UNLOAD_RICHEDITLIB
// in your project options.

//{$DEFINE TEST_VERSION}
{$IFNDEF _D6orHigher}
 {$DEFINE PARANOIA} //seems not needed from D6 !!! Inprise fixed this, finally...
{$ENDIF}


{$IFNDEF USE_OLD_FLAGS}
         {$DEFINE USE_FLAGS}
{$ELSE}  {$UNDEF  USE_FLAGS}
{$ENDIF}

{$IFnDEF EVENTS_STATIC}
         {$DEFINE EVENTS_DYNAMIC}
{$ENDIF}
{$IFnDEF CMDACTIONS_RECORD}
         {$DEFINE COMMANDACTIONS_OBJ}
         {$DEFINE PACK_COMMANDACTIONS}
         {$IFDEF  NOT_PACK_COMMANDACTIONS}
                  {$UNDEF PACK_COMMANDACTIONS}
         {$ENDIF}
{$ENDIF}

{$DEFINE KOL3XX}
{$DEFINE DIBPixels32bitWithAlpha}