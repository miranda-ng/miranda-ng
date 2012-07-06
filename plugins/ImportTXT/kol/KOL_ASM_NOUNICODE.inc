//------------------------------------------------------------------------------
// KOL_ASM_NOUNICODE.inc (to inlude in KOL.pas)
// v 3.141592

// this part of code is for case when ASM_VERSION is enabled and the symbol
// UNICODE_CTRLS is NOT defined (functions, procedures and methods which work
// with AnsiStrings only)

const comctl32_const: PKOLChar = 'comctl32';
      InitCommonControlsEx_const: PKOLChar = 'InitCommonControlsEx';
procedure DoInitCommonControls( dwICC: DWORD );
asm
    PUSH  EAX                             // dwICC
    CALL  InitCommonControls
    MOV   EAX, [ComCtl32_Module]
    TEST  EAX, EAX
    JNZ   @@1
    PUSH  [comctl32_const]
    CALL  LoadLibrary
    MOV   [ComCtl32_Module], EAX
@@1:PUSH  [InitCommonControlsEx_const]
    PUSH  EAX
    CALL  GetProcAddress
    XCHG  ECX, EAX
    {$IFDEF SAFE_CODE}
    POP   EDX
    JECXZ @@fin
    PUSH  EDX
    {$ENDIF}
    PUSH  8                               // dwSize
    PUSH  ESP                             // @ ICC
    CALL  ECX                             // Proc( @ ICC )
    POP   ECX
    POP   ECX
@@fin:
end;

function ShowMsg( const S: KOLString; Flags: DWORD ): DWORD;
asm
  push edx // Flags
  mov  ecx, [Applet]
  {$IFDEF SNAPMOUSE2DFLTBTN}
  {$IFDEF SAFE_CODE}
  jecxz @@0
  {$ENDIF}
  pushad
  xchg eax, ecx
  mov  edx, offset[WndProcSnapMouse2DfltBtn]
  call TControl.AttachProc
  popad
@@0:
  {$ENDIF}
  mov  edx, 0
  {$IFDEF SAFE_CODE}
  jecxz @@1
  {$ENDIF}
  mov  edx, [ecx].TControl.fHandle
  mov  ecx, [ecx].TControl.fCaption
@@1: push ecx // Title
  push eax    // S
  push edx    // Wnd
  call MessageBox
  {$IFDEF SNAPMOUSE2DFLTBTN}
  mov  ecx, [Applet]
  {$IFDEF SAFE_CODE}
  jecxz @@2
  {$ENDIF}
  pushad
  xchg eax, ecx
  mov  edx, offset[WndProcSnapMouse2DfltBtn]
  call TControl.DetachProc
  popad
@@2:
  {$ENDIF}
end;

procedure TGraphicTool.SetFontName(const Value: KOLString);
asm
          PUSH  EAX
          LEA   EAX, [EAX].fData.Font.Name
          XOR   ECX, ECX
          MOV   CL, 32
          PUSH  EAX
          PUSH  ECX
          PUSH  EDX
          CALL  StrLComp
          //TEST  EAX, EAX
          POP   EDX
          POP   ECX
          POP   EAX
          JZ    @@exit
          CALL  StrLCopy
          POP   EAX
          PUSH  EAX
          CALL  Changed
@@exit:   POP   EAX
end;

{$IFDEF TEXT_EXTENT_OLD}
function TCanvas.TextExtent(const Text: KOLString): TSize;
asm
        PUSH     EBX
        PUSH     ESI
        MOV      EBX, EAX
        PUSH     ECX
        PUSH     ECX               // prepare @Result
        MOV      EAX, EDX
        CALL     System.@LStrLen
        PUSH     EAX               // prepare Length(Text)
        CALL     EDX2PChar
        PUSH     EDX               // prepare PChar(Text)
        {$IFDEF  SAFE_CODE}
        MOV      EAX, EBX
        CALL     RefInc
        {$ENDIF}
        PUSH     HandleValid or FontValid
        PUSH     EBX
        CALL     RequiredState
        XCHG     ESI, EAX
        TEST     ESI, ESI          // ESI = fHandle before
        JNZ      @@1
        PUSH     ESI
        CALL     CreateCompatibleDC
        MOV      EDX, EBX
        XCHG     EAX, EDX // EAX := @Self; EDX := DC
        CALL     SetHandle
//****************************************************** // Added By M.Gerasimov
        CMP      WORD PTR [EBX].TCanvas.fIsPaintDC, 0
        JNZ      @@2
        XOR      ESI,ESI
@@2:
//******************************************************
@@1:
        PUSH     HandleValid or FontValid
        PUSH     EBX
        CALL     RequiredState
        PUSH     EAX               // prepare DC
        CALL     Windows.GetTextExtentPoint32A  // KOL_ANSI
        POP      EDX // @ Result
        {$IFDEF FIX_ITALIC_TEXT_WIDTH}
        MOV      ECX, [EBX].fFont
        //JECXZ    @@0
        CMP      [ECX].TGraphicTool.fData.Font.Italic, 0
        JZ       @@0
        MOV      EAX, [EDX].TSize.cy
        SHR      EAX, 2
        ADD      DWORD PTR [EDX], EAX
@@0:    {$ENDIF}
        TEST     ESI, ESI
        JNZ      @@exit
        XOR      EDX, EDX
        XCHG     EAX, EBX
        CALL     SetHandle
@@exit:
        {$IFDEF  SAFE_CODE}
        PUSH     EAX
        XCHG     EAX, EBX
        CALL     RefDec
        POP      EAX
        {$ENDIF}
        POP      ESI
        POP      EBX
end;
{$ELSE  TEXT_EXTENT_NEW}
function TCanvas.TextExtent(const Text: KOLString): TSize;
asm
         PUSH  ESI
         {$IFDEF FIX_ITALIC_TEXT_WIDTH}
         PUSH  EBX
         MOV   EBX, ECX
         {$ENDIF}
         XCHG  ESI, EAX // ESI = @Self: PCanvas
         CALL  EDX2PChar
         PUSH  ECX
         PUSH  EDX

         XCHG  EAX, EDX
         CALL  StrLen
         XCHG  [ESP], EAX
         PUSH  EAX

         PUSH  HandleValid or FontValid
         PUSH  ESI
         CALL  TCanvas.RequiredState
         PUSH  [ESI].TCanvas.fHandle
         CALL  GetTextExtentPoint32
         {$IFDEF FIX_ITALIC_TEXT_WIDTH}
         CMP   [ESI].TGraphicTool.fData.Font.Italic, 0
         JZ    @@1
         MOV   EAX, [EBX].TSize.cy
         SHR   EAX, 2
         ADD   DWORD PTR [EBX].TSize, EAX
@@1:     POP   EBX
         {$ENDIF}
         POP   ESI
end;
{$ENDIF  TEXT_EXTENT_NEW}

procedure TCanvas.TextOut(X, Y: Integer; const Text: KOLString); stdcall;
asm
        PUSH     EBX
        MOV      EBX, [EBP+8]

        MOV      EAX, [Text]
        PUSH     EAX
        CALL     System.@LStrLen
        XCHG     EAX, [ESP]             // prepare Length(Text)

        //CALL     System.@LStrToPChar  // string does not need to be null-terminated !
        PUSH     EAX                    // prepare PChar(Text)
        PUSH     [Y]                    // prepare Y
        PUSH     [X]                    // prepare X

        PUSH     HandleValid or FontValid or BrushValid or ChangingCanvas
        PUSH     EBX
        CALL     RequiredState
        PUSH     EAX                    // prepare fHandle
        CALL     Windows.TextOutA // KOL_ANSI

        POP      EBX
end;

function Int2Hex( Value : DWord; Digits : Integer ) : KOLString;
asm     // EAX = Value
        // EDX = Digits
        // ECX = @Result
        PUSH      0
        ADD       ESP, -0Ch
      PUSH      EDI
      PUSH      ECX
        LEA       EDI, [ESP+8+0Fh]  // EBX := @Buf[ 15 ]
        {$IFDEF SMALLEST_CODE}
        {$ELSE}
        AND       EDX, $F
        {$ENDIF}
@@loop: DEC       EDI
        DEC       EDX
          PUSH      EAX
          {$IFDEF PARANOIA} DB $24, $0F {$ELSE} AND AL, 0Fh {$ENDIF}
          AAM
          DB $D5, $11 //AAD
          ADD      AL, $30
        STOSB
        DEC       EDI
        POP       EAX
        SHR       EAX, 4
        JNZ       @@loop
        TEST      EDX, EDX
        JG        @@loop
          POP       EAX      // EAX = @Result
          MOV       EDX, EDI // EDX = @resulting string
          {$IFDEF _D2009orHigher}
          XOR      ECX, ECX
          {$ENDIF}
          CALL      System.@LStrFromPChar
          POP       EDI
          ADD       ESP, 10h
end;

function Hex2Int( const Value : AnsiString) : Integer;
asm
        CALL     EAX2PChar
        PUSH     ESI
        XCHG     ESI, EAX
        XOR      EDX, EDX
        TEST     ESI, ESI
        JE       @@exit
        LODSB
        {$IFDEF PARANOIA} DB $3C, '$' {$ELSE} CMP AL, '$' {$ENDIF}
        JNE      @@1
@@0:    LODSB
@@1:    TEST     AL, AL
        JE       @@exit
        {$IFDEF PARANOIA} DB $2C, '0' {$ELSE} SUB AL, '0' {$ENDIF}
        {$IFDEF PARANOIA} DB $3C, 9 {$ELSE} CMP AL, '9' - '0' {$ENDIF}
        JBE      @@3

        {$IFDEF PARANOIA} DB $2C, $11 {$ELSE} SUB AL, 'A' - '0' {$ENDIF}
        {$IFDEF PARANOIA} DB $3C, 5 {$ELSE} CMP AL, 'F' - 'A' {$ENDIF}
        JBE      @@2

        {$IFDEF PARANOIA} DB $2C, 32 {$ELSE} SUB AL, 32 {$ENDIF}
        {$IFDEF PARANOIA} DB $3C, 5 {$ELSE} CMP AL, 'F' - 'A' {$ENDIF}
        JA       @@exit
@@2:
        {$IFDEF PARANOIA} DB $04, 0Ah {$ELSE} ADD AL, 0Ah {$ENDIF}
@@3:
        SHL      EDX, 4
        ADD      DL, AL
        JMP      @@0

@@exit: XCHG     EAX, EDX
        POP      ESI
end;

function Int2Str( Value : Integer ) : KOLString;
asm
        XOR       ECX, ECX
        PUSH      ECX
        ADD       ESP, -0Ch

        PUSH      EBX
        LEA       EBX, [ESP + 15 + 4]
        PUSH      EDX
        CMP       EAX, ECX
        PUSHFD
        JGE       @@1
        NEG       EAX
@@1:
        MOV       CL, 10

@@2:
        DEC       EBX
        XOR       EDX, EDX
        DIV       ECX
        ADD       DL, 30h
        MOV       [EBX], DL
        TEST      EAX, EAX
        JNZ       @@2

        POPFD
        JGE       @@3

        DEC       EBX
        MOV       byte ptr [EBX], '-'
@@3:
        POP       EAX
        MOV       EDX, EBX
        {$IFDEF _D2009orHigher}
        XOR      ECX, ECX // TODO: safe to destory twice?
        {$ENDIF}
        CALL      System.@LStrFromPChar

        POP       EBX
        ADD       ESP, 10h
end;

function Int2Ths( I : Integer ) : AnsiString;
asm
        PUSH     EBP
        MOV      EBP, ESP
        PUSH     EAX
        PUSH     EDX
        CALL     Int2Str
        POP      EDX
        POP      EAX
        TEST     EAX, EAX
        JGE      @@0
        NEG      EAX
@@0:
        CMP      EAX, 1000
        JL       @@Exit
        PUSH     EDX
        MOV      EAX, [EDX]
        PUSH     EAX
        CALL     System.@LStrLen         // EAX = Length(Result)
        POP      EDX
        PUSH     EDX                     // EDX = @Result[ 1 ]
        XOR      ECX, ECX

@@1:
        ROL      ECX, 8
        DEC      EAX
        MOV      CL, [EDX+EAX]
        JZ       @@fin
        CMP      ECX, 300000h
        JL       @@1

        PUSH     ECX
        XOR      ECX, ECX
        MOV      CL, [ThsSeparator]
        JMP      @@1

@@fin:  CMP      CL, '-'
        JNE      @@fin1
        CMP      CH, [ThsSeparator]
        JNE      @@fin1
        MOV      CH, 0                   // this corrects -,ddd,...
@@fin1: CMP      ECX, 01000000h
        JGE      @@fin2
        INC      EAX
        ROL      ECX, 8
        JMP      @@fin1
@@fin2: PUSH     ECX

        LEA      EDX, [ESP+EAX]
        MOV      EAX, [EBP-4]
        {$IFDEF _D2009orHigher}
        XOR      ECX, ECX // TODO: safe to change ecx?
        {$ENDIF}
        CALL     System.@LStrFromPChar
@@Exit:
        MOV      ESP, EBP
        POP      EBP
end;

function Int2Digs( Value, Digits : Integer ) : KOLString;
asm
        PUSH     EBP
        MOV      EBP, ESP
        PUSH     EDX             // [EBP-4] = Digits
        PUSH     ECX
        MOV      EDX, ECX
        CALL     Int2Str
        POP      ECX
        PUSH     ECX             // [EBP-8] = @Result
        MOV      EAX, [ECX]
        PUSH     EAX
        CALL     System.@LStrLen
        POP      EDX             // EDX = @Result[1]
        MOV      ECX, EAX        // ECX = Length( Result )
        ADD      EAX, EAX
        SUB      ESP, EAX
        MOV      EAX, ESP
        PUSHAD
        CALL     StrCopy
        POPAD
        MOV      EDX, EAX
        ADD      ESP, -100
        CMP      byte ptr [EDX], '-'
        PUSHFD
        JNE      @@1
        INC      EDX
@@1:
        MOV      EAX, [EBP-4]    // EAX = Digits
        CMP      ECX, EAX
        JGE      @@2
        DEC      EDX
        MOV      byte ptr [EDX], '0'
        INC      ECX
        JMP      @@1
@@2:
        POPFD
        JNE      @@3
        DEC      EDX
        MOV      byte ptr [EDX], '-'
@@3:
        MOV      EAX, [EBP-8]
        {$IFDEF _D2009orHigher}
        XOR      ECX, ECX // TODO: eax or ecx affect result?
        {$ENDIF}
        CALL     System.@LStrFromPChar
        MOV      ESP, EBP
        POP      EBP
end;

function Num2Bytes( Value : Double ) : KOLString;
asm     PUSH     EBX
        PUSH     ESI
        PUSH     EDI
        MOV      EBX, ESP
        MOV      ESI, EAX
        MOV      ECX, 4
        MOV      EDX, 'TGMk'
@@1:    FLD      [Value]
@@10:   FICOM    dword ptr [@@1024]
        FSTSW    AX
        SAHF
        JB       @@2
        FIDIV    dword ptr [@@1024]
        FST      [Value]
        WAIT
        TEST     DL, 20h
        JE       @@ror
        AND      DL, not 20h
        JMP      @@nxt
@@1024: DD       1024
@@100:  DD       100
@@ror:  ROR      EDX, 8
@@nxt:  LOOP     @@10
@@2:    TEST     DL, 20h
        JZ       @@3
        MOV      DL, 0
@@3:    MOV      DH, 0
        PUSH     DX
        MOV      EDI, ESP
        FLD      ST(0)
        CALL     System.@TRUNC
        {$IFDEF _D2orD3}
        PUSH     0
        {$ELSE}
        PUSH     EDX
        {$ENDIF}
        PUSH     EAX
        FILD     qword ptr [ESP]
        POP      EDX
        POP      EDX
        MOV      EDX, ESI
        CALL     Int2Str
        FSUBP    ST(1), ST
        FIMUL    dword ptr [@@100]
        CALL     System.@TRUNC
        TEST     EAX, EAX
        JZ       @@4
        XOR      ECX, ECX
        MOV      CL, 0Ah
        CDQ
        IDIV     ECX
        TEST     EDX, EDX
        JZ       @@5
        MOV      AH, DL
        SHL      EAX, 16
        ADD      EAX, '00. '
        PUSH     EAX
        MOV      EDI, ESP
        INC      EDI
        JMP      @@4
@@5:    SHL      EAX, 8
        ADD      AX, '0.'
        PUSH     AX
        MOV      EDI, ESP
@@4:    MOV      EAX, [ESI]
        CALL     System.@LStrLen
        ADD      ESP, -100
        SUB      EDI, EAX
        PUSH     ESI
        PUSH     EDI
        MOV      ESI, [ESI]
        MOV      ECX, EAX
        REP      MOVSB
        POP      EDX
        POP      EAX
        {$IFDEF _D2009orHigher}
        XOR      ECX, ECX // TODO: IDIV
        {$ENDIF}
        CALL     System.@LStrFromPChar
        MOV      ESP, EBX
        POP      EDI
        POP      ESI
        POP      EBX
end;

function S2Int( S: PKOLChar ): Integer;
asm
        XCHG     EDX, EAX
        XOR      EAX, EAX
        TEST     EDX, EDX
        JZ       @@exit

        XOR      ECX, ECX
        MOV      CL, [EDX]
        INC      EDX
        CMP      CL, '-'
        PUSHFD
        JE       @@0
@@1:    CMP      CL, '+'
        JNE      @@2
@@0:    MOV      CL, [EDX]
        INC      EDX
@@2:    SUB      CL, '0'
        CMP      CL, '9'-'0'
        JA       @@fin
        LEA      EAX, [EAX+EAX*4] //
        LEA      EAX, [ECX+EAX*2] //
        JMP      @@0
@@fin:  POPFD
        JNE      @@exit
        NEG      EAX
@@exit:
end;

function Str2Int(const Value : KOLString) : Integer;
asm
        CALL     EAX2PChar
        CALL     S2Int
end;

function TrimLeft(const S: Ansistring): Ansistring;
asm
        XCHG     EAX, EDX
        CALL     EDX2PChar
        DEC      EDX
@@1:    INC      EDX
        MOVZX    ECX, byte ptr [EDX]
        JECXZ    @@fin
        CMP      CL, ' '
        JBE      @@1
@@fin:
        {$IFDEF _D2009orHigher}
        XOR      ECX, ECX
        {$ENDIF}
        CALL     System.@LStrFromPChar
end;

function TrimRight(const S: Ansistring): Ansistring;
asm
        PUSH     EDX
        PUSH     EAX

        PUSH     EAX
        CALL     System.@LStrLen
        XCHG     EAX, [ESP]
        CALL     EAX2PChar
        POP      ECX
        INC      ECX
@@1:    DEC      ECX
        MOV      DL, [EAX+ECX]
        JL       @@fin
        CMP      DL, ' '
        JBE      @@1
@@fin:
        INC      ECX
        POP      EAX
        XOR      EDX, EDX
        INC      EDX
        CALL     System.@LStrCopy
end;

function CopyEnd( const S : AnsiString; Idx : Integer ) : AnsiString;
asm
        PUSH     ECX
        PUSH     EAX
        PUSH     EDX

        CALL     System.@LStrLen

        POP      EDX
        TEST     EDX, EDX
        JG       @@1
        XOR      EDX, EDX
        INC      EDX
@@1:
        SUB      EAX, EDX
        MOV      ECX, EAX

        POP      EAX
        JGE      @@ret_end

        POP      EAX
        JL       System.@LStrClr

@@ret_end:
        INC      ECX
        CALL     System.@LStrCopy
end;

function CopyTail( const S : AnsiString; Len : Integer ) : AnsiString;
asm
        PUSH     ECX
        PUSH     EAX
          PUSH     EDX
            CALL     System.@LStrLen
          POP      ECX
          CMP      ECX, EAX
          {$IFDEF USE_CMOV}
          CMOVG    ECX, EAX
          {$ELSE}
          JLE      @@1
          MOV      ECX, EAX
@@1:      {$ENDIF}

        MOV      EDX, EAX
        SUB      EDX, ECX
        INC      EDX
        POP      EAX
        CALL     System.@LStrCopy
end;

procedure DeleteTail( var S : AnsiString; Len : Integer );
asm
        PUSH     EAX
        PUSH     EDX
        MOV      EAX, [EAX]
        CALL     System.@LStrLen
        POP      ECX
        CMP      ECX, EAX
        {$IFDEF USE_CMOV}
        CMOVG    ECX, EAX
        {$ELSE}
        JLE      @@1
        MOV      ECX, EAX
@@1:    {$ENDIF}

        MOV      EDX, EAX
        SUB      EDX, ECX
        INC      EDX
        POP      EAX
        CALL     System.@LStrDelete
end;

{$IFnDEF TEST_INDEXOFCHARS_COMPAT}
function IndexOfChar( const S : KOLString; Chr : KOLChar ) : Integer;
asm
        CALL     EAX2PChar
        PUSH     EAX
        MOV      ECX, [EAX-4]
        CALL     StrScanLen
        POP      EDX
        JZ       @@1
        LEA      EDX, [EAX+1]
@@1:    SUB      EAX, EDX
end;
{$ENDIF}

function IndexOfCharsMin( const S, Chars : AnsiString ) : Integer;
asm     PUSH     ESI
        PUSH     EBX
        PUSH     EAX
        CALL     EDX2PChar
        MOV      ESI, EDX

        OR       EBX, -1
        MOV      ECX, [EDX-4]
        JECXZ    @@EXIT

@@1:    LODSB

        XCHG     EDX, EAX
        POP      EAX
        PUSH     EAX

        PUSH     ECX
        CALL     IndexOfChar
        POP      ECX
        TEST     EAX, EAX
        JLE      @@NEXT

        TEST     EBX, EBX
        JLE      @@ASGN
        CMP      EAX, EBX
        JGE      @@NEXT
@@ASGN:
        XCHG     EAX, EBX
@@NEXT: LOOP     @@1

@@EXIT: XCHG     EAX, EBX
        POP      ECX
        POP      EBX
        POP      ESI
end;

function Parse( var S : AnsiString; const Separators : AnsiString ) : AnsiString;
asm
         PUSH    EBX
         PUSH    ESI
         PUSH    EDI
         MOV     EDI, ECX
         XCHG    ESI, EAX
         MOV     EAX, [ESI]
         CALL    IndexOfCharsMin
         XCHG    EBX, EAX
         TEST    EBX, EBX
         JG      @@1
         MOV     EAX, [ESI]
         CALL    System.@LStrLen
         XCHG    EBX, EAX
         INC     EBX
@@1:
         XOR     EDX, EDX
         INC     EDX
         PUSH    EDX

         PUSH    EDI
         MOV     ECX, EBX
         DEC     ECX
         MOV     EAX, [ESI]
         CALL    System.@LStrCopy
         XCHG    EAX, ESI
         MOV     ECX, EBX
         POP     EDX
         CALL    System.@LStrDelete
         POP     EDI
         POP     ESI
         POP     EBX
end;

function _StrSatisfy( S, Mask : PKOLChar ) : Boolean;
asm
    TEST EAX, EAX
    JZ   @@exit
        XCHG     ECX, EAX
        //       EDX <- Mask
        //       ECX <- S
        XOR      EAX, EAX
        MOV      AL, '*'
@@rest_satisfy:
        PUSH     ECX
        PUSH     EDX

@@nx_char:
        MOV      AH, [EDX]
        OR       AH, [ECX]
        JZ       @@fin //@@ret_true

        MOV      AH, 0

        CMP      word ptr [EDX], AX //'*'
        JE       @@fin //@@ret_true

        CMP      byte ptr [ECX], AH
        JNE      @@10

        DEC      EDX
@@1:
        INC      EDX
        CMP      byte ptr [EDX], AL //'*'
        JE       @@1

        CMP      byte ptr [EDX], AH
        SETZ     AL
        JMP      @@fin

@@10:   CMP      byte ptr [EDX], AH
        JE       @@ret_false

        CMP      byte ptr [EDX], '?'
        JNE      @@11

@@go_nx_char:
        INC      ECX
        INC      EDX
        JMP      @@nx_char

@@11:
        CMP      byte ptr [EDX], AL //'*'
        JNE      @@20

        INC      EDX
@@12:   CMP      byte ptr [ECX], AH
        JE       @@ret_false

        CALL     @@rest_satisfy
        TEST     AL, AL
        JNE      @@fin
        MOV      AL, '*'

        INC      ECX
        JMP      @@12

@@20:   MOV      AH, [EDX]
        XOR      AH, [ECX]

        JE       @@go_nx_char
@@ret_false:
        XOR      EAX, EAX

@@fin:
        POP      EDX
        POP      ECX
@@exit:
end;

function StrSatisfy( const S, Mask: AnsiString ): Boolean;
asm
        PUSH     ESI
        TEST     EAX, EAX
        JZ       @@exit

        XCHG     ESI, EAX

        XCHG     EAX, EDX
        TEST     EAX, EAX
        JZ       @@exit

        CALL     EAX2PChar

        PUSH     0
        MOV      EDX, ESP
        CALL     AnsiLowerCase

        XCHG     EAX, ESI
        CALL     EAX2PChar

        PUSH     0
        MOV      EDX, ESP
        CALL     AnsiLowerCase

        POP      EAX
        POP      EDX
        PUSH     EDX
        PUSH     EAX
        CALL     _StrSatisfy

        XCHG     ESI, EAX

        CALL     RemoveStr
        CALL     RemoveStr
        XCHG     EAX, ESI

@@exit:
        POP      ESI
end;

function _2StrSatisfy( S, Mask: PAnsiChar ): Boolean;
asm     //     //
        PUSH   EBX
        PUSH   ECX
        XCHG   EBX, EAX
        PUSH   0
        MOV    EAX, ESP
        {$IFDEF _D2009orHigher}
        XOR      ECX, ECX
        {$ENDIF}
        CALL   System.@LStrFromPChar
        PUSH   0
        MOV    EAX, ESP
        MOV    EDX, EBX
        {$IFDEF _D2009orHigher}
        XOR      ECX, ECX
        {$ENDIF}
        CALL   System.@LStrFromPChar
        POP    EAX
        POP    EDX
        PUSH   EDX
        PUSH   EAX
        CALL   StrSatisfy
        XCHG   EBX, EAX
        CALL   RemoveStr
        CALL   RemoveStr
        XCHG   EAX, EBX
        POP    ECX
        POP    EBX
end;

function SkipSpaces( P: PKOLChar ): PKOLChar;
asm
         DEC   EAX
@@loop:  INC   EAX
         CMP   byte ptr [EAX], 0
         JE    @@exit
         CMP   byte ptr [EAX], ' '
         JBE   @@loop
@@exit:
end;

function SkipParam(P: PKOLChar): PKOLChar;
asm
         CALL  SkipSpaces
@@while: CMP   byte ptr [EAX], ' '
         JBE   @@exit
         CMP   byte ptr [EAX], '"'
         JNE   @@incP_goLoop
@@untilQuot:
         INC   EAX
         CMP   byte ptr [EAX], 0
         JE    @@exit
         CMP   byte ptr [EAX], '"'
         JNE   @@untilQuot
@@incP_goLoop:
         INC   EAX
         JMP   @@while
@@exit:
end;

function ParamCount: Integer;
asm
         CALL GetCommandLine
         OR   EDX, -1
@@while: INC  EDX
         CALL SkipParam
         CALL SkipSpaces
         CMP  byte ptr [EAX], 0
         JNE  @@while
         XCHG EAX, EDX
end;

function __DelimiterLast( Str: PAnsiChar; Delimiters: PAnsiChar ): PAnsiChar;
asm
        PUSH     ESI

        CALL     EAX2PChar

        MOV      ESI, EDX
        MOV      EDX, EAX

@@tolast:
        CMP      byte ptr [EAX], 0
        JZ       @@next1
        INC      EAX
        JMP      @@tolast

@@next1:
        PUSH     EAX

@@next:
        LODSB
        TEST     AL, AL
        JZ       @@exit

        PUSH     EDX
        XCHG     EDX, EAX
        CALL     StrRScan
        POP      EDX

        TEST     EAX, EAX
        JZ       @@next

        POP      ECX
        CMP      byte ptr [ECX], 0
        JZ       @@next1

        CMP      EAX, ECX
        JG       @@next1

        PUSH     ECX
        JLE      @@next

@@exit: POP      EAX
        POP      ESI
end;

function DelimiterLast( const Str, Delimiters: KOLString ): Integer;
asm
        CALL     EAX2PChar
        CALL     EDX2PChar
        PUSH     EAX
        CALL     __DelimiterLast
        POP      EDX
        SUB      EAX, EDX
        INC      EAX
end;

function StrIsStartingFrom( Str, Pattern: PKOLChar ): Boolean;
asm
  {$IFDEF F_P}
        MOV     EAX, [Str]
        MOV     EDX, [Pattern]
  {$ENDIF F_P}
        XOR     ECX, ECX
      @@1:
        MOV     CL, [EDX]   // pattern[ i ]
        INC     EDX
        MOV     CH, [EAX]   // str[ i ]
        INC     EAX
        JECXZ   @@2         // str = pattern; CL = #0, CH = #0
        CMP     CL, CH
        JE      @@1
      @@2:
        TEST    CL, CL
        SETZ    AL
end {$IFDEF F_P} [ 'EAX', 'EDX', 'ECX' ] {$ENDIF};

function Format( const fmt: KOLString; params: array of const ): AnsiString;
asm
        PUSH    ESI
        PUSH    EDI
        PUSH    EBX
        MOV     EBX, ESP
        {$IFDEF UNICODE_CTRLS}
        ADD     ESP, -2048
        {$ELSE}
        ADD     ESP, -1024
        {$ENDIF}
        MOV     ESI, ESP

        INC     ECX
        JZ      @@2
@@1:
        MOV     EDI, [EDX + ECX*8 - 8]
        PUSH    EDI
        LOOP    @@1
@@2:
        PUSH    ESP
        PUSH    EAX
        PUSH    ESI

        CALL    wvsprintf

        MOV     EDX, ESI
        MOV     EAX, @Result
        {$IFDEF _D2009orHigher}
        PUSH     ECX
        XOR      ECX, ECX
        {$ENDIF}
        CALL    System.@LStrFromPChar
        {$IFDEF _D2009orHigher}
        POP      ECX
        {$ENDIF}

        MOV     ESP, EBX
        POP     EBX
        POP     EDI
        POP     ESI
end;

function FileExists( const FileName : KOLString ) : Boolean;
const size_TWin32FindData = sizeof( {$IFDEF UNICODE_CTRLS} TWin32FindDataW {$ELSE} TWin32FindDataA {$ENDIF} );
      Size_TFindFileData = (sizeof(TFindFileData) + 3) and not 3;
asm
{$IFDEF FILE_EXISTS_EX}
        PUSH     EBX
        MOV      BL, 0
        PUSH     EAX
        PUSH     SEM_NOOPENFILEERRORBOX or SEM_FAILCRITICALERRORS
        CALL     SetErrorMode
        XCHG     EAX, [ESP]
        SUB      ESP, Size_TFindFileData
        MOV      EDX, ESP
        CALL     Find_First
        TEST     AL, AL
        JZ       @@fin
        MOV      EAX, ESP
        CALL     Find_Close
        TEST     byte ptr [ESP].TFindFileData.dwFileAttributes, FILE_ATTRIBUTE_DIRECTORY
        JNZ      @@fin
        PUSH     ESP
        LEA      EAX, [ESP+4].TFindFileData.ftLastWriteTime
        PUSH     EAX
        CALL     FileTimeToLocalFileTime
        LEA      EAX, [ESP+8]
        PUSH     EAX
        INC      EAX
        INC      EAX
        PUSH     EAX
        SUB      EAX, 10
        PUSH     EAX
        CALL     FileTimeToDOSDateTime
        TEST     EAX, EAX
        SETNZ    BL
@@fin:  ADD      ESP, Size_TFindFileData
        CALL     SetErrorMode
        XCHG     EAX, EBX
        POP      EBX
{$ELSE}
        CALL     EAX2PChar
        PUSH     EAX
        CALL     GetFileAttributes
        INC      EAX
        JZ       @@exit
        DEC      EAX
        {$IFDEF PARANOIA} DB $24, FILE_ATTRIBUTE_DIRECTORY {$ELSE} AND AL, FILE_ATTRIBUTE_DIRECTORY {$ENDIF}
        SETZ     AL
@@exit:
{$ENDIF}
end;

function DiskPresent( const DrivePath: KOLString ): Boolean;
asm
         PUSH  EBX
         MOV   BH, 0
         TEST  EAX, EAX
         JZ    @@dirExists
         CMP   byte ptr [EAX], '\'
         JZ    @@dirExists
         PUSH  EAX
         PUSH  EAX
         CALL  GetDriveType
         CMP   AL, DRIVE_REMOVABLE
         JE    @@setErrMode
         CMP   AL, DRIVE_CDROM
         JE    @@setErrMode
         CMP   AL, DRIVE_RAMDISK
         JNE   @@popPath_dirExists
@@setErrMode:
         INC   BH
         PUSH  SEM_NOOPENFILEERRORBOX or SEM_FAILCRITICALERRORS
         CALL  SetErrorMode
         XCHG  [ESP], EAX
         PUSH  EAX
@@popPath_dirExists:
         POP   EAX
@@dirExists:
         CALL  DirectoryExists
         MOV   BL, AL
         TEST  BH, BH
         JZ    @@exit
         CALL  SetErrorMode
@@exit:  XCHG  EAX, EBX
         POP   EBX
end;

function GetStartDir : AnsiString;
asm
        PUSH     EBX
        MOV      EBX, EAX

        XOR      EAX, EAX
        MOV      AH, 2
        SUB      ESP, EAX
        MOV      EDX, ESP
        PUSH     EAX
        PUSH     EDX
        PUSH     0
        CALL     GetModuleFileName // in KOL_ANSI

        LEA      EDX, [ESP + EAX]
@@1:    DEC      EDX
        CMP      byte ptr [EDX], '\'
        JNZ      @@1

        INC      EDX
        MOV      byte ptr [EDX], 0

        MOV      EAX, EBX
        MOV      EDX, ESP
        {$IFDEF _D2009orHigher}
        XOR      ECX, ECX
        {$ENDIF}
        CALL     System.@LStrFromPChar // AnsiSafe!

        ADD      ESP, 200h
        POP      EBX
end;

function IncludeTrailingChar( const S: KOLString; C: KOLChar ): KOLString;
asm
  push edx
  push ecx
  xchg ecx, eax
  xchg edx, ecx
  call System.@LStrAsg
  pop  eax
  pop  edx
  mov  ecx, [eax]
  jecxz @@1
  add  ecx, [ecx-4]
  dec  ecx
  cmp  byte ptr [ecx], dl
  jz   @@exit
@@1:
  push eax
  push 0
  mov  eax, esp
  {$IFDEF _D2009orHigher}
  //push ecx
  xor ecx, ecx
  {$ENDIF}
  call System.@LStrFromChar
  {$IFDEF _D2009orHigher}
  //pop  ecx
  {$ENDIF}
  mov  edx, [esp]
  mov  eax, [esp+4]
  call System.@LStrCat 
  call RemoveStr
  pop  eax
@@exit:
end;

const
  DirDelimiters: PAnsiChar = ':\/';
function ExtractFileName( const Path : AnsiString ) : AnsiString;
asm
        PUSH     EDX
        PUSH     EAX
        MOV      EDX, [DirDelimiters]
        CALL     __DelimiterLast
        POP      EDX
        CMP      byte ptr [EAX], 0
        JZ       @@1
        XCHG     EDX, EAX
        INC      EDX
@@1:    POP      EAX
        {$IFDEF _D2009orHigher}
        XOR      ECX, ECX
        {$ENDIF}
        CALL     System.@LStrFromPChar // Safe!
end;

function ExtractFileNameWOext( const Path : KOLString ) : KOLString;
asm
  push ebx

  push edx
  push eax
  call ExtractFileName
  pop  edx // Path - не нужен больше
  mov  eax, [esp] // eax = Result = ExtractFileName(Path)
  mov  eax, [eax]
  push 0
  mov  edx, esp
  call ExtractFileExt
  mov  eax, [esp]
  call System.@LStrLen
  xchg ebx, eax   // ebx = Length(ExtractFileExt(Result))
  call RemoveStr  // ExtractFileExt - больше не нужен
  mov  eax, [esp]
  mov  eax, [eax]
  call System.@LStrLen // eax = Length(Result)
  sub  eax, ebx
  xchg ecx, eax
  xor  edx, edx
  inc  edx
  mov  eax, [esp]
  mov  eax, [eax]
  call System.@LStrCopy

  pop  ebx
end;

const
  ExtDelimeters: PAnsiChar = '.';

function ExtractFileExt( const Path : KOLString ) : KOLString;
asm
        PUSH     EDX
        MOV      EDX, [ExtDelimeters]
        CALL     EAX2PChar
        CALL     __DelimiterLast
@@1:    XCHG     EDX, EAX
        POP      EAX
        {$IFDEF _D2009orHigher}
        PUSH     ECX
        XOR      ECX, ECX
        {$ENDIF}
        CALL     System.@LStrFromPChar
        {$IFDEF _D2009orHigher}
        POP      ECX // this routine hasn't touch ECX
        {$ENDIF}
end;

function ReplaceExt( const Path, NewExt: KOLString ): KOLString;
asm
  push ecx // result
  push edx // NewExt
  push eax // Path

  push 0
  mov  edx, esp
  call ExtractFilePath
  pop  eax
  xchg [esp], eax // eax=Path, Path in stack replaced with ExtractFilePath(Path)

  push 0
  mov  edx, esp
  call ExtractFileNameWOext
  // now stack conatins: result,NewExt,ExtractFilePath(Path),ExtractFileNameWOext(Path)<-ESP

  mov  eax, [esp+12]
  mov  edx, esp
  push dword ptr [edx+4] // ExtractFilePath(Path)
  push dword ptr [edx]   // ExtractFileNameWOext(Path)
  push dword ptr [edx+8] // NewExt
  mov  edx,  3
  call System.@LStrCatN
  call RemoveStr
  call RemoveStr
  pop  ecx
  pop  ecx
end;

function GetSystemDir: KOLString;
asm
         PUSH  EBX
         XCHG  EBX, EAX
         SUB   ESP, MAX_PATH
         MOV   EAX, ESP
         PUSH  MAX_PATH
         PUSH  EAX
         CALL  GetSystemDirectory
         MOV   EAX, EBX
         MOV   EDX, ESP
         CALL  System.@LStrFromPChar
         MOV   EDX, EBX
         MOV   EAX, [EDX]
         CALL  IncludeTrailingPathDelimiter
         ADD   ESP, MAX_PATH
         POP   EBX
end;

function GetWindowsDir : KOLString;
asm
         PUSH  EBX
         XCHG  EBX, EAX
         SUB   ESP, MAX_PATH
         MOV   EAX, ESP
         PUSH  MAX_PATH
         PUSH  EAX
         CALL  GetWindowsDirectory
         MOV   EAX, EBX
         MOV   EDX, ESP
         CALL  System.@LStrFromPChar
         MOV   EDX, EBX
         MOV   EAX, [EDX]
         CALL  IncludeTrailingPathDelimiter
         ADD   ESP, MAX_PATH
         POP   EBX
end;

function GetWorkDir : KOLString;
asm
         PUSH  EBX
         XCHG  EBX, EAX
         SUB   ESP, MAX_PATH
         PUSH  ESP
         PUSH  MAX_PATH
         CALL  GetCurrentDirectory
         MOV   EAX, EBX
         MOV   EDX, ESP
         CALL  System.@LStrFromPChar
         MOV   EDX, EBX
         MOV   EAX, [EDX]
         CALL  IncludeTrailingPathDelimiter
         ADD   ESP, MAX_PATH
         POP   EBX
end;

function GetTempDir : KOLString;
asm
  push eax
  sub esp, 264
  push esp
  push 261
  call GetTempPath
  mov  edx, esp
  mov  eax, [esp+264]
  {$IFDEF _D2009orHigher}
  xor  ecx, ecx
  {$ENDIF}
  call System.@LStrFromPChar
  add esp, 264
  pop edx
  mov eax, [edx]
  call IncludeTrailingPathDelimiter
end;

function CreateTempFile( const DirPath, Prefix: KOLString ): KOLString;
asm
  push ecx
  call EAX2PCHAR
  call EDX2PCHAR
  sub  esp, 264
  push esp
  push 0
  push edx
  push eax
  call GetTempFileName
  mov  eax, [esp+264]
  mov  edx, esp
  {$IFDEF _D2009orHigher}
  xor ecx, ecx // ecx is argument
  {$ENDIF}
  call System.@LStrFromPChar
  add  esp, 268
end;

function FindFilter( const Filter: AnsiString): AnsiString;
asm
        XCHG     EAX, EDX
        PUSH     EAX
        CALL     System.@LStrAsg
        POP      EAX
        CMP      dword ptr [EAX], 0
        JNE      @@exit
        LEA      EDX, @@mask_all
        {$IFDEF _D2009orHigher}
        XOR      ECX, ECX
        {$ENDIF}
        JE       System.@LStrFromPChar
@@mask_all:  DB  '*.*',0
@@exit:
end;

procedure TDirList.ScanDirectoryEx(const DirPath, Filters: AnsiString;
  Attr: DWord);
asm
        PUSH     EBX
        MOV      EBX, EAX

        PUSHAD
        LEA      EAX, [EBX].fFilters
        CALL     Free_And_Nil

        CALL     NewStrList
        MOV      [EBX].fFilters, EAX
        POPAD

        PUSHAD
        PUSH     0
        MOV      EAX, ESP
        MOV      EDX, ECX
        CALL     System.@LStrLAsg
@@1:    MOV      ECX, [ESP]
        JECXZ    @@2
        MOV      EAX, ESP
        MOV      EDX, offset[@@semicolon]
        PUSH     0
        MOV      ECX, ESP
        CALL     Parse
        MOV      EAX, [ESP]
        MOV      EDX, ESP
        CALL     Trim
        POP      EDX
        PUSH     EDX
        TEST     EDX, EDX
        JZ       @@filt_added
        MOV      EAX, [EBX].fFilters
        CALL     TStrList.Add
@@filt_added:
        CALL     RemoveStr
        JMP      @@1

        //       ';' string literal
        {$IFDEF _D2009orHigher}
        DW       0, 1 
        {$ENDIF}
        DD       -1, 1
@@semicolon:
        DB       ';',0

@@2:    POP      ECX
        POPAD
        XOR      ECX, ECX
        PUSH     [Attr]
        CALL     ScanDirectory
        POP      EBX
@@exit:
end;

procedure _FillStrList;    // Эта часть кода общая для двух следующих процедур
asm
///////////////////////////////
        OR      EAX,0
        JE      @@EXIT                  //ERROR
//        LEA     EAX,[EAX-IniBufferSize]
//        JE      @@EXIT
//      возможна нехватка Буфера... в принципе не ошибка :)
//      возвращаем что влезло...
@@LOOP:
        LEA     EAX,[ESI+4]
        CALL    StrLen
        MOV     [ESI],EAX
        LEA     EDX,[ESI+4]
        INC     EAX
        ADD     ESI,EAX

        MOV     EAX,EDI

        CALL    TStrList.ADD

        CMP     byte ptr [ESI+4],0
        JNE     @@LOOP

@@EXIT:
        POP     EAX
        CALL    System.@FreeMem


        POP     ECX
        POP     EBX
        POP     EDI
        POP     ESI
end;

procedure TIniFile.GetSectionNames(Names: PStrList);
asm
        PUSH    ESI
        PUSH    EDI
        PUSH    EBX
        PUSH    ECX

        MOV     EBX,EAX
        MOV     EAX, IniBufferStrSize
        MOV     EDI,EDX

        CALL    System.@GetMem
        MOV     ESI,EAX
        PUSH    EAX

        PUSH    [EBX].fFileName
        MOV     EAX,IniBufferSize
        PUSH    EAX

        LEA     EAX,[ESI+4]
        PUSH    EAX

        CALL    GetPrivateProfileSectionNames
        JMP     _FillStrList
end;

procedure TIniFile.SectionData(Names: PStrList);
asm
        PUSH    ESI
        PUSH    EDI
        PUSH    EBX
        PUSH    ECX

        MOV     EBX,EAX
        MOV     EAX, IniBufferStrSize
        MOV     EDI,EDX

        CALL    System.@GetMem
        MOV     ESI,EAX
        PUSH    EAX

        OR     [EBX].fMode,0
        JNE     @@DOWrite

        PUSH    [EBX].fFileName
        MOV     EAX,IniBufferSize
        PUSH    EAX

        LEA     EAX,[ESI+4]
        PUSH    EAX
        PUSH    [EBX].fSection

        CALL    GetPrivateProfileSection
        JMP     _FillStrList

@@DOWrite:

        PUSH    EBX
        PUSH    ESI
        PUSH    EDX
        PUSH    EBP

        MOV     EDX,0
        MOV     EBP,[EDI].TStrList.fCount
        MOV     EBX,IniBufferSize-2 // оставим место для #0#0

{ECM+++>} OR      EBP,EBP  // otherwise GetPChars when StrList.Count = 0 crashed

@@LOOP:
        JE      @@ENDLOOP

        OR      EBX,EBX
        JE      @@ENDLOOP

        PUSH    EDX
        MOV     EAX,EDI
        CALL    TStrList.GetPChars

        PUSH    EAX
        CALL    StrLen
        POP     EAX

        XOR     ECX,-1
        MOV     EDX,ESI

        SUB     EBX,ECX
        JA      @@L1
        ADD     ECX,EBX
        XOR     EBX,EBX
@@L1:

        ADD     ESI,ECX

        CALL    MOVE
@@L2:
        POP     EDX
        INC     EDX
        DEC     EBP
        JMP     @@LOOP
@@ENDLOOP:
        MOV     WORD PTR [ESI],0

        POP     EBP
        POP     EDX
        POP     ESI
        POP     EBX
        MOV     EAX,EBX
        CALL    ClearSection

        PUSH    [EBX].fFileName
        PUSH    ESI
        PUSH    [EBX].fSection

        CALL    WritePrivateProfileSection

        POP     EAX
        CALL    System.@FreeMem

        POP     ECX
        POP     EBX
        POP     EDI
        POP     ESI

end;

function _NewControl( AParent: PControl; ControlClassName: PKOLChar;
                      Style: DWORD; Ctl3D: Boolean;
                      Actions: TCommandActionsParam ): PControl;
const szActions = sizeof(TCommandActions);
asm
        PUSH     EBX
        PUSH     EAX  // push AParent
        PUSH     ECX  // push Style
            MOVZX    ECX, [Ctl3D]
            PUSH     [Actions]
            CALL     _NewWindowed
            XCHG     EBX, EAX
            {$IFDEF  USE_FLAGS}
            OR       [EBX].TControl.fFlagsG3, (1 shl G3_IsControl)
            {$ELSE}
            INC      [EBX].TControl.fIsControl
            {$ENDIF}
        POP      EDX  // pop Style
        OR       EDX, WS_CLIPSIBLINGS or WS_CLIPCHILDREN
        //INC      [EBX].TControl.fVerticalAlign
        MOV      byte ptr [EBX].TControl.fLookTabKeys, $0F
        TEST     [EBX].TControl.fCtl3D_child, 1
        JZ       @@noCtl3D
        AND      EDX, not WS_BORDER
        OR       byte ptr [EBX].TControl.fExStyle + 1, WS_EX_CLIENTEDGE shr 8
@@noCtl3D:
        MOV      [EBX].TControl.fStyle, EDX
        {$IFDEF  USE_FLAGS}
        {$ELSE}
            TEST     EDX, WS_VISIBLE
            SETNZ    AL
            MOV      [EBX].TControl.fVisible, AL
            TEST     EDX, WS_TABSTOP
            SETNZ    AL
            MOV      [EBX].TControl.fTabstop, AL
        {$ENDIF  USE_FLAGS}
        POP      ECX // pop AParent
        JECXZ    @@noParent

        PUSH     ESI
        PUSH     EDI
        PUSH     ECX
            LEA      ESI, [ECX].TControl.fMargin
            LEA      EDI, [EBX].TControl.fBoundsRect
            LODSB
            MOVSX    EAX, AL
            {$IFNDEF SMALLEST_CODE}
            PUSH     EAX
            MOVSX    ECX, byte ptr [ESI+2]
            ADD      EAX, ECX // AParent.fClientLeft
            {$ENDIF}
            STOSD    // fBoundsRect.Left
            {$IFNDEF SMALLEST_CODE}
            POP      EAX
            PUSH     EAX
            MOVSX    ECX, byte ptr [ESI+0]
            ADD      EAX, ECX // AParent.fClientTop
            {$ENDIF}
            STOSD    // fBoundsRect.Top
            {$IFNDEF SMALLEST_CODE}
            XCHG     EDX, EAX
            POP      EAX
            {$ENDIF}
            ADD      EAX, 64
            STOSD    // fBoundsRect.Right
            {$IFNDEF SMALLEST_CODE}
            XCHG     EAX, EDX
            ADD      EAX, 64
            {$ENDIF}
            STOSD    // fBoundsRect.Bottom}
        POP      ECX
            MOV      EAX, [ECX].TControl.fCursor
            STOSD
        POP      EDI
        POP      ESI

        XCHG     EAX, ECX
        CALL     TControl.ParentForm
        XCHG     ECX, EAX
        JECXZ    @@noParentForm
        INC      [ECX].TControl.fTabOrder
        MOV      DX, WORD PTR [ECX].TControl.fTabOrder
        MOV      WORD PTR [EBX].TControl.fTabOrder, DX
        TEST     [EBX].TControl.fStyle, WS_TABSTOP
        JZ       @@CurrentControl_set
        CMP      [ECX].TControl.DF.fCurrentControl, 0
        JNZ      @@CurrentControl_set
        MOV      [ECX].TControl.DF.fCurrentControl, EBX
@@CurrentControl_set:
@@noParentForm:
@@noParent:
        MOVZX    EDX, [CtlIdCount]
        INC      [CtlIdCount]
        MOV      [EBX].TControl.fMenu, EDX
        MOV      EDX, offset[WndProcCtrl]
        MOV      EAX, EBX
        CALL     TControl.AttachProc
        XCHG     EAX, EBX
        POP      EBX
        {$IFDEF  DEBUG_ALTSPC}
        PUSH     EAX
        CALL     DumpWindowed
        POP      EAX
        {$ENDIF}
end;

const StaticClass: Array[0..6] of AnsiChar=('S','T','A','T','I','C',#0);
function NewLabel( AParent: PControl; const Caption: KOLString ): PControl;
asm
        PUSH     EDX

        PUSH     0
        {$IFDEF  PACK_COMMANDACTIONS}
        PUSH     [LabelActions_Packed]
        {$ELSE}
        PUSH     offset[LabelActions]
        {$ENDIF}
        MOV      ECX, WS_VISIBLE or WS_CHILD or SS_LEFTNOWORDWRAP or SS_NOPREFIX or SS_NOTIFY
        MOV      EDX, offset[StaticClass]
        CALL     _NewControl
        MOV      word ptr [EAX].TControl.aAutoSzX, $101
        {$IFDEF  USE_FLAGS}
        OR       [EAX].TControl.fFlagsG1, (1 shl G1_SizeRedraw) or (1 shl G1_IsStaticControl)
        {$ELSE}
        INC      [EAX].TControl.fIsStaticControl
        INC      [EAX].TControl.fSizeRedraw
        {$ENDIF}
        MOV      EDX, [EAX].TControl.fBoundsRect.Top
        ADD      EDX, 22
        MOV      [EAX].TControl.fBoundsRect.Bottom, EDX
        POP      EDX
        PUSH     EAX
        CALL     TControl.SetCaption
        POP      EAX

{$IFDEF GRAPHCTL_XPSTYLES}
        PUSH     EAX
        MOV      EDX, offset[XP_Themes_For_Label]
        CALL     Attach_WM_THEMECHANGED
        POP      EAX
{$ENDIF}
end;

function NewGroupbox( AParent: PControl; const Caption: KOLString ): PControl;
asm
        PUSH     EDX
        PUSH     0
        {$IFDEF  PACK_COMMANDACTIONS}
        PUSH     [ButtonActions_Packed]
        {$ELSE}
        PUSH     offset[ButtonActions]
        {$ENDIF}
        MOV      EDX, offset[ButtonClass]
        MOV      ECX, WS_VISIBLE or WS_CHILD or BS_GROUPBOX or WS_CLIPCHILDREN or WS_CLIPSIBLINGS
        CALL     _NewControl
        OR       [EAX].TControl.fExStyle, WS_EX_CONTROLPARENT
        MOV      EDX, [EAX].TControl.fBoundsRect.Left
        ADD      EDX, 100
        MOV      [EAX].TControl.fBoundsRect.Right, EDX
        MOV      EDX, [EAX].TControl.fBoundsRect.Top
        ADD      EDX, 100
        MOV      [EAX].TControl.fBoundsRect.Bottom, EDX
        MOV      byte ptr [EAX].TControl.fClientTop, 22
        XOR      EDX, EDX
        {$IFDEF  USE_FLAGS}
        AND      [EAX].TControl.fStyle.f2_Style, not(1 shl F2_Tabstop)
        {$ELSE}
        MOV      [EAX].TControl.fTabstop, DL
        {$ENDIF  USE_FLAGS}
        MOV      DL, 2
        ADD      [EAX].TControl.fClientBottom, DL
        ADD      [EAX].TControl.fClientLeft, DL
        ADD      [EAX].TControl.fClientRight, DL
        POP      EDX
        PUSH     EAX
        CALL     TControl.SetCaption
        POP      EAX
        PUSH     EAX
        {$IFDEF  USE_FLAGS}
        OR       [EAX].TControl.fFlagsG5, 1 shl G5_IsGroupbox
        {$ELSE}
        INC      [EAX].TControl.fIsGroupBox
        {$ENDIF}
        MOV      EDX, offset[WndProcDoEraseBkgnd]
        CALL     TControl.AttachProc
        POP      EAX

{$IFDEF GRAPHCTL_XPSTYLES}
        PUSH     EAX
        MOV      EDX, offset[XP_Themes_For_GroupBox]
        CALL     Attach_WM_THEMECHANGED
        POP      EAX
{$ENDIF}
end;

function NewPanel( AParent: PControl; EdgeStyle: TEdgeStyle ): PControl;
const CreateStyle = WS_VISIBLE or WS_CHILD or SS_LEFTNOWORDWRAP or
                    SS_NOPREFIX or SS_NOTIFY;
asm
{$IFDEF GRAPHCTL_XPSTYLES}
        MOVZX    EDX, EdgeStyle
        PUSH     EDX
{$ENDIF}

        PUSH     EDX
        MOV      EDX, offset[StaticClass]
        MOV      ECX, CreateStyle
        PUSH     0
        {$IFDEF  PACK_COMMANDACTIONS}
        PUSH     [LabelActions_Packed]
        {$ELSE}
        PUSH     offset[LabelActions]
        {$ENDIF}
        CALL     _NewControl
        //INC      byte ptr [EAX].TControl.aAutoSzX
        //INC      byte ptr [EAX].TControl.aAutoSzY
        MOV      word ptr [EAX].TControl.aAutoSzX, $101
        ADD      [EAX].TControl.fBoundsRect.Right, 100-64
        ADD      [EAX].TControl.fBoundsRect.Bottom, 100-64
        OR       byte ptr [EAX].TControl.fExStyle+2, 1
        POP      ECX
        CMP      CL, 1
        JG       @@exit
        JE       @@sunken
        OR       byte ptr [EAX].TControl.fStyle+2, $40
{$IFDEF GRAPHCTL_XPSTYLES}
        JMP      @@visual
{$ELSE}
        RET
{$ENDIF}
@@sunken:
        OR       byte ptr [EAX].TControl.fStyle+1, $10
@@exit:

{$IFDEF GRAPHCTL_XPSTYLES}
@@visual:
        CMP      AppTheming, TRUE
        JNE      @@es_none_
        CMP      CL, 1
        JG       @@es_none_
        JE       @@not_sunken
        AND      byte ptr [EAX].TControl.fStyle+2, $00
        JNE      @@es_none_
@@not_sunken:
        AND      byte ptr [EAX].TControl.fStyle+1, $00
@@es_none_:
        POP      EDX
        PUSH     EAX
        CALL     TControl.SetEdgeStyle
        POP      EAX
        PUSH     EAX
        MOV      EDX, offset[XP_Themes_For_Panel]
        CALL     Attach_WM_THEMECHANGED
        POP      EAX
{$ENDIF}
end;

const ListBoxClass : Array[ 0..7 ] of AnsiChar = ( 'L','I','S','T','B','O','X',#0 );
function NewListbox( AParent: PControl; Options: TListOptions ): PControl;
asm
        PUSH     EAX
        PUSH     EDX
        MOV      EAX, ESP
        MOV      EDX, offset[ListFlags]
        XOR      ECX, ECX
        MOV      CL, 11
        CALL     MakeFlags
        POP      EDX
        OR       EAX, WS_VISIBLE or WS_CHILD or WS_TABSTOP or WS_BORDER or WS_VSCROLL or LBS_NOTIFY
        XCHG     ECX, EAX
        POP      EAX
        PUSH     1
        {$IFDEF  PACK_COMMANDACTIONS}
        PUSH     [ListActions_Packed]
        {$ELSE}
        PUSH     offset[ListActions]
        {$ENDIF}
        MOV      EDX, offset[ListBoxClass]
        CALL     _NewControl
        {$IFDEF  PACK_COMMANDACTIONS}
        MOV      EDX, [EAX].TControl.fCommandActions
        MOV      [EDX].TCommandActionsObj.aClear, offset[ClearListbox]
        {$ENDIF}
        ADD      [EAX].TControl.fBoundsRect.Right, 100
        ADD      [EAX].TControl.fBoundsRect.Bottom, 200-64
        MOV      [EAX].TControl.fColor, clWindow
        MOV      [EAX].TControl.fLookTabKeys, 3
end;

procedure CreateComboboxWnd( Combo: PControl );
//const PrevProcStr: PAnsiChar = 'PREV_PROC'; //************ Remarked By M.Gerasimov
asm
        PUSH     EDI
        PUSH     EBX
        XCHG     EBX, EAX
        PUSH     GW_CHILD
        PUSH     [EBX].TControl.fHandle
@@getwindow:
        CALL     GetWindow
        TEST     EAX, EAX
        JZ       @@fin
        PUSH     offset[WndFuncCombo]
        PUSH     GWL_WNDPROC
        PUSH     EAX
        XCHG     EDI, EAX
        CALL     SetWindowLong
        PUSH     EAX
        PUSH     offset [ID_PREVPROC]                       //
        PUSH     EDI
        CALL     SetProp
@@2getnext:
        PUSH     GW_HWNDNEXT
        PUSH     EDI
        JMP      @@getwindow
@@fin:  POP      EBX
        POP      EDI
end;

{$IFDEF WNDPROCTREEVIEW_OLDASMVERSION}
function WndProcTreeView( Self_: PControl; var Msg: TMsg; var Rslt: Integer ): Boolean;
asm     //cmd    //opd
        CMP      word ptr [EDX].TMsg.message, WM_NOTIFY
        JNZ      @@ret_false
        PUSH     EBX
        XCHG     EBX, EAX
        MOV      EDX, [EDX].TMsg.lParam
        {$IFDEF  EVENTS_DYNAMIC}
        MOV      EAX, [EBX].TControl.EV
        LEA      EAX, [EAX].TEvents.fOnTVBeginDrag
        {$ELSE}
        LEA      EAX, [EBX].TControl.EV.fOnTVBeginDrag
        {$ENDIF}
        CMP      word ptr [EDX].TNMTreeView.hdr.code, NM_RCLICK
        JNE      @@chk_TVN_BEGINDRAG
        PUSH     ECX
        PUSH     ECX
        PUSH     ESP
        CALL     GetCursorPos
        MOV      EAX, EBX
        MOV      EDX, ESP
        MOV      ECX, EDX
        CALL     TControl.Screen2Client
        POP      EAX
        AND      EAX, $FFFF
        POP      EDX
        SHL      EDX, 16
        OR       EAX, EDX
        PUSH     EAX
        CALL     GetShiftState
        PUSH     EAX
        PUSH     WM_RBUTTONUP
        PUSH     [EBX].TControl.fHandle
        CALL     PostMessage
        JMP      @@2fin_false1

@@chk_TVN_BEGINDRAG:
        {$IFDEF UNICODE_CTRLS}
        CMP      word ptr [EDX].TNMTreeView.hdr.code, TVN_BEGINDRAGW
        JZ       @@event_drag
        CMP      word ptr [EDX].TNMTreeView.hdr.code, TVN_BEGINRDRAGW
        JZ       @@event_drag
        {$ENDIF UNICODE_CTRLS}
        CMP      word ptr [EDX].TNMTreeView.hdr.code, TVN_BEGINDRAG
        JZ       @@event_drag
        CMP      word ptr [EDX].TNMTreeView.hdr.code, TVN_BEGINRDRAG
        JNZ      @@chk_BEGINLABELEDIT
@@event_drag:
        MOV      EDX, [EDX].TNMTreeView.itemNew.hItem
@@event_call:
        MOV      ECX, [EAX].TMethod.Code
        JECXZ    @@2fin_false1
        MOV      EAX, [EAX].TMethod.Data
        XCHG     EBX, ECX
        XCHG     EDX, ECX
        CALL     EBX
@@2fin_false1:   JMP      @@fin_false
@@chk_BEGINLABELEDIT:
        {$IFDEF  EVENTS_DYNAMIC}
        MOV      EAX, [EBX].TControl.EV
        LEA      EAX, [EAX].TEvents.FOnTVBeginEdit
        {$ELSE}
        LEA      EAX, [EBX].TControl.EV.fOnTVBeginEdit
        {$ENDIF}
        {$IFDEF UNICODE_CTRLS}
        CMP      word ptr [EDX].TNMTreeView.hdr.code, TVN_BEGINLABELEDITW
        JZ       @@beginlabeledit
        {$ENDIF UNICODE_CTRLS}
        CMP      word ptr [EDX].TNMTreeView.hdr.code, TVN_BEGINLABELEDIT
        JNZ      @@chk_ITEMEXPANDED //@@chk_DELETEITEM
@@beginlabeledit:
        {$IFDEF  USE_FLAGS}
        TEST     [EBX].TControl.fFlagsG6, 1 shl G6_Dragging
        {$ELSE}
        CMP      [EBX].TControl.fDragging, 0
        {$ENDIF}
        JZ       @@allow_LABELEDIT
        XOR      EAX, EAX
        INC      EAX
        MOV      [ECX], EAX
        JMP      @@ret_true

@@allow_LABELEDIT:
        PUSH     ECX // @Rslt

        MOV      ECX, [EAX].TMethod.Code
        JECXZ    @@2fin_false1
        PUSH     EBX
        XCHG     EBX, ECX
        MOV      EDX, [EDX].TTVDispInfo.item.hItem
        XCHG     EDX, ECX
        MOV      EAX, [EAX].TMethod.Data
        CALL     EBX
        TEST     AL, AL
        SETZ     AL        // Rslt := not event result;
        POP      EBX
        JMP      @@ret_EAX

@@call_EBX:
        CALL     EBX
@@2fin_false:
        JMP      @@fin_false
@@chk_ITEMEXPANDED:
        {$IFDEF  EVENTS_DYNAMIC}
        MOV      EAX, [EBX].TControl.EV
        LEA      EAX, [EAX].TEvents.fOnTVExpanded
        {$ELSE}
        LEA      EAX, [EBX].TControl.EV.fOnTVExpanded
        {$ENDIF}
        {$IFDEF UNICODE_CTRLS}
        CMP      word ptr [EDX].TNMTreeView.hdr.code, TVN_ITEMEXPANDEDW
        JZ       @@itemexpanded
        {$ENDIF UNICODE_CTRLS}
        CMP      word ptr [EDX].TNMTreeView.hdr.code, TVN_ITEMEXPANDED
        JNZ      @@chk_SELCHANGING
@@itemexpanded:
        MOV      ECX, [EAX].TMethod.Code
        JECXZ    @@2fin_false
        CMP      [EDX].TNMTreeView.action, TVE_EXPAND
        PUSH     ECX
        SETZ     CL
        XCHG     ECX, [ESP]
        JMP      @@event_drag
@@chk_SELCHANGING:
        CMP      word ptr [EDX].TNMTreeView.hdr.code, TVN_SELCHANGING
        JNE      @@chk_ITEMEXPANDING
        XCHG     EAX, ECX
        {$IFDEF  EVENTS_DYNAMIC}
        MOV      ECX, [EBX].TControl.EV
        MOV      ECX, [ECX].TEvents.fOnTVSelChanging.TMethod.Code
        {$ELSE}
        MOV      ECX, [EBX].TControl.EV.fOnTVSelChanging.TMethod.Code
        {$ENDIF}
@@2fin_false2:
        JECXZ    @@2fin_false
        PUSH     EAX  //@Rslt
        PUSH     [EDX].TNMTreeView.itemNew.hItem
        XCHG     ECX, EBX //EBX=OnTVSelChanging.Code ECX=Sender
        XCHG     ECX, EDX //EDX=Sender ECX=Msg
        MOV      ECX, [ECX].TNMTreeView.itemOld.hItem
        {$IFDEF  EVENTS_DYNAMIC}
        MOV      EAX, [EDX].TControl.EV
        MOV      EAX, [EAX].TEvents.fOnTVSelChanging.TMethod.Data
        {$ELSE}
        MOV      EAX, [EDX].TControl.EV.fOnTVSelChanging.TMethod.Data
        {$ENDIF}
        CALL     EBX
        XOR      AL, 1
        MOVZX    EAX, AL
        JMP      @@ret_EAX

@@chk_ITEMEXPANDING:
        {$IFDEF UNICODE_CTRLS}
        CMP      word ptr [EDX].TNMTreeView.hdr.code, TVN_ITEMEXPANDINGW
        JZ       @@itemexpanding
        {$ENDIF UNICODE_CTRLS}
        CMP      word ptr [EDX].TNMTreeView.hdr.code, TVN_ITEMEXPANDING
        JNE      @@chk_ENDLABELEDIT
@@itemexpanding:
        XCHG     EAX, ECX
        {$IFDEF  EVENTS_DYNAMIC}
        MOV      ECX, [EBX].TControl.EV
        MOV      ECX, [ECX].TEvents.fOnTVExpanding.TMethod.Code
        {$ELSE}
        MOV      ECX, [EBX].TControl.EV.fOnTVExpanding.TMethod.Code
        {$ENDIF}
        JECXZ    @@2fin_false2
        PUSH     EAX // @Rslt
        CMP      [EDX].TNMTreeView.action, TVE_EXPAND
        PUSH     ECX
        SETZ     CL
        XCHG     ECX, [ESP]
        XCHG     ECX, EBX  //EBX=OnTVExpanding.Code ECX=Seneder
        XCHG     EDX, ECX  //ECX=Msg EDX=Sender
        MOV      ECX, [ECX].TNMTreeView.itemNew.hItem //ECX=Item
        {$IFDEF  EVENTS_DYNAMIC}
        MOV      EAX, [EDX].TControl.EV
        MOV      EAX, [EAX].TEvents.fOnTVExpanding.TMethod.Data
        {$ELSE}
        MOV      EAX, [EDX].TControl.EV.fOnTVExpanding.TMethod.Data //EAX=object
        {$ENDIF}
@@111:
        CALL     EBX
@@ret_EAX:
        POP      EDX //EDX=@Rslt
        MOVZX    EAX, AL
        NEG      EAX
        MOV      [EDX], EAX
@@ret_true:
        MOV      AL, 1
        POP      EBX
        RET
@@chk_ENDLABELEDIT:
        {$IFDEF UNICODE_CTRLS}
        CMP      word ptr [EDX].TNMTreeView.hdr.code, TVN_ENDLABELEDITW
        JZ       @@endlabeledit
        {$ENDIF UNICODE_CTRLS}
        CMP      word ptr [EDX].TNMTreeView.hdr.code, TVN_ENDLABELEDIT
        JNZ      @@chk_SELCHANGED
@@endlabeledit:
        XCHG     EAX, ECX
        {$IFDEF  EVENTS_DYNAMIC}
        MOV      ECX, [EBX].TControl.EV
        MOV      ECX, [ECX].TEvents.fOnTVEndEdit.TMethod.Code
        {$ELSE}
        MOV      ECX, [EBX].TControl.EV.fOnTVEndEdit.TMethod.Code
        {$ENDIF}
        JECXZ    @@ret_1
        PUSH     EAX
        PUSH     EBX
        PUSH     0

        XCHG     EDX, EBX
        MOV      EAX, [EBX].TTVDispInfo.item.pszText
        PUSH     EDX
        PUSH     ECX
        XCHG     EAX, EDX
        {$IFDEF UNICODE_CTRLS}
        CMP      [EBX].TNMTreeView.hdr.code, TVN_ENDLABELEDITW
        JNZ      @@endlabeleditA
        CALL     TControl.TVGetItemTextW
        JMP      @@NewTxt_ready
@@endlabeleditA:
        {$ENDIF UNICODE_CTRLS}
        TEST     EDX, EDX
        JNZ      @@prepare_NewTxt
        // NewTxt := [EDX].TControl.TVItemText[ hItem ]
        LEA      ECX, [ESP + 8]
        MOV      EDX, [EBX].TTVDispInfo.item.hItem
        CALL     TControl.TVGetItemText
        JMP      @@NewTxt_ready
@@prepare_NewTxt:
        LEA      EAX, [ESP+8]
        {$IFDEF _D2009orHigher}
        PUSH     ECX
        XOR      ECX, ECX
        {$ENDIF}
        CALL     System.@LStrFromPChar
        {$IFDEF _D2009orHigher}
        POP      ECX
        {$ENDIF}
@@NewTxt_ready:
        POP      ECX
        POP      EDX
        POP      EAX
        PUSH     EAX
        PUSH     EAX
        {$IFDEF  EVENTS_DYNAMIC}
        MOV      EAX, [EDX].TControl.EV
        MOV      EAX, [EAX].TEvents.fOnTVEndEdit.TMethod.Data
        {$ELSE}
        MOV      EAX, [EDX].TControl.EV.fOnTVEndEdit.TMethod.Data
        {$ENDIF}
        MOV      EBX, [EBX].TTVDispInfo.item.hItem
        XCHG     ECX, EBX
        CALL     EBX
        XCHG     EBX, EAX
        CALL     RemoveStr
        XCHG     EAX, EBX
        POP      EBX
        JMP      @@ret_EAX
@@ret_1:
        INC      ECX
        MOV      [EAX], ECX
        JMP      @@ret_true

@@chk_SELCHANGED:
        {$IFDEF UNICODE_CTRLS}
        CMP      word ptr [EDX].TNMTreeView.hdr.code, TVN_SELCHANGEDW
        JZ       @@selchanged
        {$ENDIF UNICODE_CTRLS}
        CMP      word ptr [EDX].TNMTreeView.hdr.code, TVN_SELCHANGED
        JNZ      @@fin_false
@@selchanged:
        XCHG     EAX, EBX
        CALL     TControl.DoSelChange

@@fin_false:
        POP      EBX
@@ret_false:
        XOR      EAX, EAX
end;
{$ELSE NEW VERSION OF WndProcTreeView}
function WndProcTreeView( Self_: PControl; var Msg: TMsg; var Rslt: Integer ): Boolean;
asm
         PUSH  ESI
         PUSH  EDI
         MOV   EDI, ECX // EDI -> Rslt
         XOR   ECX, ECX
         CMP   WORD PTR [EDX].TMsg.message, WM_NOTIFY
         JNZ   @@ret_false1
         XCHG  ESI, EAX
         MOV   EDX, [EDX].TMsg.lParam
         CMP   WORD PTR [EDX].TNMTreeView.hdr.code, NM_RCLICK
         JNE   @@chk_TVN_BEGINDRAG
         PUSH  ECX
         PUSH  ECX
         PUSH  ESP
         CALL  GetCursorPos
         MOV   EAX, ESI
         MOV   EDX, ESP
         MOV   ECX, EDX
         CALL  TControl.Screen2Client
         POP   EDX
         POP   EAX
         SHLD  EAX, EDX, 16
         PUSH  EAX
         CALL  GetShiftState
         PUSH  EAX
         PUSH  WM_RBUTTONUP
         PUSH  ESI
         CALL  TControl.PostMsg
         JMP   @@ret_false1
@@prepareCallEvent:
         STC
         MOV   EDX, ESI
         {$IFDEF EVENTS_DYNAMIC}
         MOV   ESI, [ESI].TControl.EV
         LEA   ECX, [ESI+ECX*8].TEvents.fOnTVBeginDrag
         {$ELSE}
         LEA   ECX, [ESI+ECX*8].TControl.EV.fOnTVBeginDrag
         {$ENDIF}
         MOV   EAX, [ECX].TMethod.Data
         MOV   ECX, [ECX].TMethod.Code
         JECXZ @@noEvent
         MOV   ESI, ECX
         AND   EAX, EAX
@@noEvent:
         RET
@@chk_TVN_BEGINDRAG: ///////////////////////////////////////////////////////////
         CMP   WORD PTR [EDX].TNMTreeView.hdr.code, TVN_BEGINDRAG
         JE    @@beginDrag
         CMP   WORD PTR [EDX].TNMTreeView.hdr.code, TVN_BEGINRDRAG
         JNE   @@chk_TVNBEGINLABELEDIT
@@beginDrag:
         PUSH  [EDX].TNMTreeView.itemNew.hItem
         CALL  @@prepareCallEvent
         POP   ECX
         JC    @@ret_false1
@@justEventCall:
         CALL  ESI
@@RsltEAX_ResultFalse:
         MOV   [EDI], EAX
         XOR   EAX, EAX
         POP   EDI
         POP   ESI
         RET
@@chk_TVNBEGINLABELEDIT: ///////////////////////////////////////////////////////
         INC   ECX  // -> FOnTVBeginEdit
         CMP   WORD PTR [EDX].TNMTreeView.hdr.code, TVN_BEGINLABELEDIT
         JNE   @@chk_ENDLABELEDIT
         ///////////////////////////////////////////////////////////////////////
         XOR   EAX, EAX
         INC   EAX
         {$IFDEF USE_FLAGS}
         TEST  [ESI].TControl.fFlagsG6, 1 shl G6_Dragging
         {$ELSE}
         CMP   [ESI].TControl.fDragging, 0
         {$ENDIF}
         JNZ   @@rsltEAX_ResultTrue
         PUSH  [EDX].TTVDispInfo.item.hItem
         CALL  @@prepareCallEvent
         POP   ECX
         JC    @@ret_false1
         CALL  ESI
         XOR   AL, 1   //+Dufa
@@rsltEAX_ResultTrue:
         MOV   [EDI], AL //+VK
@@ResultTrue:
         MOV   AL, 1
         POP   EDI
         POP   ESI
         RET
@@chk_ENDLABELEDIT:
         INC   ECX //  -> fOnTVEndEdit
         CMP   WORD PTR [EDX].TNMTreeView.hdr.code, TVN_ENDLABELEDIT
         JNE   @@chk_ITEMEXPANDING
         MOV   EAX, [EDX].TTVDispInfo.item.pszText
         TEST  EAX, EAX
         JZ    @@ResultTrue
         PUSH  EAX
         PUSH  [EDX].TTVDispInfo.item.hItem
         CALL  @@prepareCallEvent
         POP   ECX
         //JNC   @@justEventCall ---//dufa
         JC    @@ret_false1         //dufa
         CALL  ESI                  //dufa
         JMP   @@rsltEAX_ResultTrue //dufa
@@Rslt1_ResultTrue:
         XOR   EAX, EAX
         INC   EAX
         JMP   @@RsltEAX_ResultFalse
@@chk_ITEMEXPANDING: ///////////////////////////////////////////////////////////
         INC   ECX //  -> FOnTVExpanding
         CMP   WORD PTR [EDX].TNMTreeView.hdr.code, TVN_ITEMEXPANDING
         JNE   @@chk_ITEMEXPANDED
@@expanding_expanded:
         CMP   [EDX].TNMTreeView.action, TVE_EXPAND
         SETZ  AL
         PUSH  EAX
         PUSH  [EDX].TNMTreeView.itemNew.hItem
@@event3:
         CALL  @@prepareCallEvent
         POP   ECX
         JNC   @@justEventCall
         POP   EAX
         JMP   @@ret_false1
@@chk_ITEMEXPANDED: ////////////////////////////////////////////////////////////
         INC   ECX //  -> FOnTVExpanded
         CMP   [EDX].TNMTreeView.hdr.code, TVN_ITEMEXPANDED
         JE    @@expanding_expanded
         ///////////////////////////////////////////////////////////////////////
         INC   ECX //  -> FOnTVSelChanging
         CMP   [EDX].TNMTreeView.hdr.code, TVN_SELCHANGING
         JNE   @@chk_TVN_SELCHANGED
         PUSH  [EDX].TNMTreeView.itemNew.hItem
         PUSH  [EDX].TNMTreeView.itemOld.hItem
         JMP   @@event3
@@chk_TVN_SELCHANGED:
         CMP   [EDX].TNMTreeView.hdr.code, TVN_SELCHANGED
         JNE   @@ret_false1
         XCHG  EAX, ESI
         CALL  TControl.DoSelChange
@@ret_false1:
         XOR   EAX, EAX
         POP   EDI
         POP   ESI
end;
{$ENDIF}

function NewTabControl( AParent: PControl; const Tabs: array of PKOLChar; Options: TTabControlOptions;
         ImgList: PImageList; ImgList1stIdx: Integer ): PControl;
const lenf=high(TabControlFlags); //+++
asm     //cmd    //opd
        PUSH     EBX
        PUSH     ESI
        PUSH     EDI
        XCHG     EBX, EAX
        PUSH     EDX
        PUSH     ECX
        LEA      EAX, [Options]
        MOV      EDX, offset[TabControlFlags]
        XOR      ECX, ECX
        MOV      CL, lenf
        CALL     MakeFlags
        TEST     byte ptr [Options], 4
        JZ       @@0
        OR       EAX, WS_TABSTOP or TCS_FOCUSONBUTTONDOWN
@@0:    OR       EAX, WS_CHILD or WS_CLIPSIBLINGS or WS_CLIPCHILDREN or WS_VISIBLE
        XCHG     ECX, EAX
        XCHG     EAX, EBX
        MOV      EDX, offset[WC_TABCONTROL]
        PUSH     1
        {$IFDEF  PACK_COMMANDACTIONS}
        PUSH     [TabControlActions_Packed]
        {$ELSE}
        PUSH     offset[TabControlActions]
        {$ENDIF}
        CALL     _NewCommonControl
        MOV      EBX, EAX
        TEST     [Options], 2 shl (tcoBorder - 1)
        JNZ      @@borderfixed
        AND      [EBX].TControl.fExStyle, not WS_EX_CLIENTEDGE
@@borderfixed:
        MOV      EDX, offset[WndProcTabControl]
        CALL     TControl.AttachProc
        ADD      [EBX].TControl.fBoundsRect.Right, 100-64
        ADD      [EBX].TControl.fBoundsRect.Bottom, 100-64
        MOV      ECX, [ImgList]
        JECXZ    @@2
        XCHG     EAX, ECX
        CALL     TImageList.GetHandle
        PUSH     EAX
        PUSH     0
        PUSH     TCM_SETIMAGELIST
        PUSH     EBX
        CALL     TControl.Perform
@@2:
        POP      EDI // EDI = High(Tabs)
        POP      ESI // ESI = Tabs
        XOR      EDX, EDX // EDX := 0 (=I)
        MOV      EAX, [ImgList1stIdx] //(=II)
@@loop:
        CMP      EDX, EDI
        JG       @@e_loop
        PUSH     EAX
        PUSH     EDX
        PUSH     EAX
        LODSD
        XCHG     ECX, EAX
        MOV      EAX, EBX
        CALL     TControl.TC_Insert
        POP      EDX
        POP      EAX
        INC      EAX
        INC      EDX
        JMP      @@loop
@@e_loop:
        MOV      byte ptr [EBX].TControl.fLookTabKeys, 1
        XCHG     EAX, EBX
        POP      EDI
        POP      ESI
        POP      EBX
end;

function NewToolbar( AParent: PControl; Align: TControlAlign; Options: TToolbarOptions;
                     Bitmap: HBitmap; const Buttons: array of PKOLChar;
                     const BtnImgIdxArray: array of Integer ) : PControl;
const szTBButton = Sizeof( TTBButton );
      Option3DBorder = 1 shl Ord( tbo3DBorder );
asm     //cmd    //opd
   PUSH EDI
        MOVZX    EDX, DL
        PUSH     EDX // Align
        PUSH     EAX // AParent

        XOR      EAX, EAX
        TEST     CL, Option3DBorder
        SETNZ    AL
        PUSH     EAX

        PUSH     ECX // Options

        MOV      AL, ICC_BAR_CLASSES
        CALL     DoInitCommonControls

        MOV      EAX, ESP
        MOV      EDX, offset[ToolbarOptions]
        XOR      ECX, ECX
        MOV      CL, 6
        CALL     MakeFlags
        POP      EDX

        {$IFDEF  COMMANDACTIONS_OBJ}
        PUSH     TOOLBAR_ACTIONS
        {$ELSE}
        PUSH     0   //: actions : = nil
        {$ENDIF}
        XCHG     ECX, EAX // ECX = MakeFlags(...)
   MOV  EDI, ECX
        MOV      EAX, [ESP+8] // EAX = AParent
        MOV      EDX, [ESP+12] // EDX = Align
        OR       ECX, [EDX*4+offset ToolbarAligns]
        OR       ECX, WS_CHILD or WS_VISIBLE or TBSTYLE_TOOLTIPS
        MOV      EDX, offset[ TOOLBARCLASSNAME ]
        CALL     _NewCommonControl
        {$IFDEF  COMMANDACTIONS_OBJ}
        MOV      EDX, [EAX].TControl.fCommandActions
        MOV      [EDX].TCommandActionsObj.aClear, offset[ClearToolbar]
        MOV      [EDX].TCommandActionsObj.aGetCount, TB_BUTTONCOUNT
        {$ELSE}
        MOV      [EAX].TControl.fCommandActions.aClear, offset[ClearToolbar]
        MOV      [EAX].TControl.fCommandActions.aGetCount, TB_BUTTONCOUNT
        {$ENDIF}
        {$IFDEF  USE_FLAGS}
        OR       [EAX].TControl.fFlagsG5, 1 shl G5_IsButton
        {$ELSE}
        INC      [EAX].TControl.fIsButton
        {$ENDIF}
        POP      EDX // pop AParent
        POP      EDX // EDX = Align
        PUSH     EDX
        TEST     EDX, EDX
        JE       @@zero_bounds
        ADD      [EAX].TControl.fBoundsRect.Bottom, 26-64
        ADD      [EAX].TControl.fBoundsRect.Right, 1000-64
        JMP      @@bounds_ready
@@zero_bounds:
        MOV      [EAX].TControl.fBoundsRect.Left, EDX
        MOV      [EAX].TControl.fBoundsRect.Top, EDX
        MOV      [EAX].TControl.fBoundsRect.Right, EDX
        MOV      [EAX].TControl.fBoundsRect.Bottom, EDX
@@bounds_ready:
        PUSH     EBX
        PUSH     ESI
            XCHG     EBX, EAX
            MOV      ESI, offset[TControl.Perform]
            PUSH     0
            PUSH     0
            PUSH     TB_GETEXTENDEDSTYLE
            PUSH     EBX
            CALL     ESI
            OR       EAX, TBSTYLE_EX_DRAWDDARROWS
            PUSH     EAX
            PUSH     0
            PUSH     TB_SETEXTENDEDSTYLE
            PUSH     EBX
            CALL     ESI
            MOV      EDX, offset[WndProcToolbarCtrl]
            MOV      EAX, EBX
            CALL     TControl.AttachProc
            MOV      EDX, offset[WndProcDoEraseBkgnd]
            MOV      EAX, EBX
            CALL     TControl.AttachProc
            PUSH     0
            PUSH     szTBButton
            PUSH     TB_BUTTONSTRUCTSIZE
            PUSH     EBX
            CALL     ESI
            PUSH     0
            MOVSX    EAX, [EBX].TControl.fMargin
            PUSH     EAX
            PUSH     TB_SETINDENT
            PUSH     EBX
            CALL     ESI
            MOV      EAX, [ESP+8] // Align
            {$IFDEF PARANOIA} DB $2C, 1 {$ELSE} SUB AL, 1 {$ENDIF}
            JL       @@bounds_correct
            JE       @@corr_right
            {$IFDEF PARANOIA} DB $2C, 2 {$ELSE} SUB AL, 2 {$ENDIF}
            JNE      @@corr_bottom
    @@corr_right:
            MOV      EDX, [EBX].TControl.fBoundsRect.Left
            ADD      EDX, 24
            MOV      [EBX].TControl.fBoundsRect.Right, EDX
            JMP      @@bounds_correct
    @@corr_bottom:
            MOV      EDX, [EBX].TControl.fBoundsRect.Top
            ADD      EDX, 22
            MOV      [EBX].TControl.fBoundsrect.Bottom, EDX
    @@bounds_correct:
            {$IFnDEF TBBUTTONS_DFLT_NOAUTOSIZE}
            MOV      byte ptr [EBX].TControl.DF.fDefaultTBBtnStyle, TBSTYLE_AUTOSIZE
            {$ENDIF}
            MOV      EDX, [Bitmap]
            TEST     EDX, EDX
            JZ       @@bitmap_added
            MOV      EAX, EBX
            CALL     TControl.TBAddBitmap
    @@bitmap_added:

            PUSH     dword ptr [BtnImgIdxArray]
            PUSH     dword ptr [BtnImgIdxArray-4]
            MOV      ECX, [Buttons-4]
            MOV      EDX, [Buttons]
            MOV      EAX, EBX
            CALL     TControl.TBAddButtons

            PUSH     0
            PUSH     0
            PUSH     WM_SIZE
            PUSH     EBX
            CALL     ESI
    // ---
    {+|ecm|}
    // ---
            MOV      EDX,EDI
            OR       EDX,[EBX].TControl.FStyle
            MOV      EAX,EBX
            CALL     TControl.SetStyle
    //  ---
    {/+|ecm|}
    //  ---
            XCHG     EAX, EBX
        POP      ESI
        POP      EBX
        POP      EDX
   POP  EDI
end;

function WndProcDateTimePickerNotify( Self_: PControl; var Msg: TMsg; var Rslt: Integer ): Boolean;
type
    TStrStr = record
        param_Date: TDateTime;
        param_PtrToAccept: PInteger;
        Accept: Integer;
        UserString: String;
    end;
const Size_TStrStr = sizeof( TStrStr );
asm
         PUSH  ESI
         PUSH  EDI
         MOV   EDI, EDX
         CMP   WORD PTR [EDI].TMsg.message, WM_NOTIFY
         JNZ   @@ret_false
         {$IFDEF EVENTS_DYNAMIC}
         MOV   ESI, [EAX].TControl.EV
         {$ENDIF}
         MOV   ECX, [EDI].TMsg.lParam
         MOV   EDX, [ECX].TNMHdr.code
         CMP   EDX, DTN_DROPDOWN
         JNZ   @@chk_DTN_CLOSEUP
         {$IFDEF EVENTS_DYNAMIC}
         LEA   ECX, [ESI].TEvents.fOnDropDown.TMethod.Code
         {$ELSE}
         LEA   ECX, [EAX].TControl.EV.fOnDropDown.TMethod.Code
         {$ENDIF}
@@event1:
         MOV   EDX, [ECX].TMethod.Data
         MOV   ECX, [ECX].TMethod.Code
         {$IFDEF NIL_EVENTS}
         JECXZ @@ret_false
         {$ENDIF}
         XCHG  EAX, EDX
         CALL  ECX
         JMP   @@ret_false
@@chk_DTN_CLOSEUP: /////////////////////////////////////////////////////////////
         {$IFDEF EVENTS_DYNAMIC}
         LEA   ECX, [ESI].TEvents.fOnCloseUp.TMethod.Code
         {$ELSE}
         LEA   ECX, [EAX].TControl.EV.fOnCloseUp.TMethod.Code
         {$ENDIF}
         CMP   EDX, DTN_CLOSEUP
         JE    @@event1
////////////////////////////////////////////////////////////////////////////////
         {$IFDEF EVENTS_DYNAMIC}
         LEA   ECX, [ESI].TEvents.fOnChangeCtl.TMethod.Code
         {$ELSE}
         LEA   ECX, [EAX].TControl.EV.fOnChangeCtl.TMethod.Code
         {$ENDIF}
         CMP   EDX, DTN_DATETIMECHANGE
         JE    @@event1
         CMP   EDX, DTN_USERSTRING
         JNE   @@ret_false
////////////////////////////////////////////////////////////////////////////////
         {$IFDEF EVENTS_DYNAMIC}
         MOV   ECX, [ESI].TEvents.fOnDTPUserString.TMethod.Code
         MOV   EDX, [ESI].TEvents.fOnDTPUserString.TMethod.Data
         {$ELSE}
         MOV   ECX, [EAX].TControl.EV.fOnDTPUserString.TMethod.Code
         MOV   EDX, [EAX].TControl.EV.fOnDTPUserString.TMethod.Data
         {$ENDIF}
         {$IFDEF NIL_EVENTS}
         JECXZ @@ret_false
         {$ENDIF}
         SUB   ESP, Size_TStrStr
         MOV   ESI, ESP
         PUSHAD
         CALL  TControl.GetDateTime
         FSTP  QWORD PTR [ESI].TStrStr.param_Date
         WAIT
         //POPAD
         //PUSHAD
         LEA   EAX, [ESI].TStrStr.UserString
         AND   dword ptr [EAX], 0
         MOV   EDI, [EDI].TMsg.lParam
         MOV   EDX, [EDI].TNMDateTimeString.pszUserString
         CALL  System.@LStrFromPChar
         LEA   EAX, [ESI].TStrStr.Accept
         MOV   byte ptr [EAX], 1
         MOV   [ESI].TStrStr.param_PtrToAccept, EAX
         POPAD
         MOV   ESI, ECX
         MOV   ECX, [ESI].TStrStr.UserString
         XCHG  EAX, EDX
         CALL  ESI
         MOV   EAX, [ESP].TStrStr.Accept
         AND   EAX, 1
         MOV   [EDI].TNMDateTimeString.dwFlags, EAX
         LEA   EAX, [ESI].TStrStr.UserString
         CALL  System.@LStrClr
         ADD   ESP, Size_TStrStr
@@ret_false:
         XOR   EAX, EAX
         POP   EDI
         POP   ESI
end;

function TControl.GetWindowHandle: HWnd;
asm
         MOV      ECX, [EAX].fHandle
         JECXZ    @@1
         XCHG     EAX, ECX
         RET
@@1:
         PUSH     EBX
         MOV      EBX, EAX
         {$IFDEF  USE_FLAGS}
         TEST     [EBX].fFlagsG4, 1 shl G4_CreateVisible
         {$ELSE}
         CMP      [EBX].fCreateVisible, 0
         {$ENDIF}
         JNZ      @@2

         XOR      EDX, EDX
         CALL     TControl.Set_Visible

         MOV      EAX, EBX
         CALL     CallTControlCreateWindow
         { This is a call to Pascal piece of code, which
           calls virtual method TControl.CreateWindow }

         {$IFDEF  USE_FLAGS}
         OR       [EBX].fFlagsG4, 1 shl G4_CreateHidden
         {$ELSE}
         INC      [EBX].fCreateHidden
         {$ENDIF}
         JMP      @@0

@@2:     CALL     CallTControlCreateWindow
@@0:     MOV      EAX, [EBX].fHandle
         POP      EBX
end;

function TControl.CreateWindow: Boolean;
type PCreateWndParams = ^TCreateWndParams;
const
  CS_OFF = CS_OWNDC or CS_CLASSDC or CS_PARENTDC or CS_GLOBALCLASS;
  CS_ON = 0; //CS_VREDRAW or CS_HREDRAW;
  szWndClass = sizeof( TWndClass );
  int_IDC_ARROW = integer( IDC_ARROW );
asm
        PUSH     EBX
        XCHG     EBX, EAX
        {$IFDEF DEBUG_CREATEWINDOW}
        MOV     EAX, EBX
        CALL    Debug_CreateWindow1
        {$ENDIF}
        MOV      ECX, [EBX].fParent
        JECXZ    @@chk_handle
        XCHG     EAX, ECX
        CALL     GetWindowHandle
        TEST     EAX, EAX
        JZ       @@ret_0
@@chk_handle:
        MOV      ECX, [EBX].fHandle
        JECXZ    @@prepare_Params
        MOV      EAX, EBX
        {$IFDEF  USE_FLAGS}
        TEST     [EBX].fFlagsG4, 1 shl G4_CreateHidden
        {$ELSE}
        CMP      [EBX].fCreateHidden, 0
        {$ENDIF}
        JZ       @@create_children
        CALL     CreateChildWindows
        MOV      EAX, EBX
        MOV      DL, 1
        CALL     Set_Visible
        {$IFDEF  USE_FLAGS}
        AND      [EBX].fFlagsG4, not(1 shl G4_CreateHidden)
        {$ELSE}
        MOV      [EBX].fCreateHidden, 0
        {$ENDIF}
        JMP      @@ret_true
@@create_children:
        CALL     CreateChildWindows
@@ret_true:
        MOV      AL, 1
@@ret_0:
        POP      EBX
        RET
@@prepare_params:
        {$IFDEF USE_GRAPHCTLS}
            {$IFDEF  USE_FLAGS}
            TEST    [EBX].fFlagsG6, 1 shl G6_GraphicCtl
            SETNZ   AL
            JNZ     @@ret_0
            {$ELSE}
            MOV      AL, [EBX].fWindowed
            CMP      AL, 0
            JZ       @@ret_0
            {$ENDIF}
        {$ENDIF}
        PUSH     EBP
        MOV      EBP, ESP

        PUSH     ECX        // Params.WindowClass.lpszClassName := nil
        PUSH     ECX        // Params.WindowClass.lpszMenuName := nil
        PUSH     ECX        // Params.WindowClass.hbrBackground := 0
        PUSH     int_IDC_ARROW
        PUSH     ECX
        CALL     LoadCursor
        PUSH     EAX        // Params.WindowClass.hCursor := LoadCursor( 0, IDC_ARROW )
        XOR      ECX, ECX
        PUSH     ECX        // Params.WindowClass.hIcon := 0
        PUSH     [hInstance]// Params.WindowClass.hInstance := hInstance
        PUSH     ECX        // Params.WindowClass.cbWndExtra := 0
        PUSH     ECX        // Params.WindowClass.cbClsExtra := 0
        {$IFDEF  SAFE_CODE}
        PUSH     [EBX].fDefWndProc     // Params.WindowClass.lpfnWndProc := fDefWndProc
        {$ELSE}
        PUSH     0
        {$ENDIF}
        PUSH     [EBX].fClsStyle       // Params.WindowClass.style := fStyle
        ADD      ESP, -64
        PUSH     ECX
        MOV      EAX, EBX
        MOV      EDX, ESP
        CALL     get_ClassName
        POP      EDX
        MOV      EAX, ESP
        PUSH     EDX
        //CALL     StrPCopy    // StrPCopy( Params.WinClsNamBuf, ClassName )
        CALL     StrCopy
        CALL     RemoveStr
        PUSH     0           // Params.Param := nil
        PUSH     [hInstance] // Params.Inst := hInstance
        PUSH     [EBX].fMenu // Params.Menu := fMenu
        MOV      DL, 1
        MOV      EAX, EBX
        CALL     GetParentWnd
        PUSH     EAX         // Params.WndParent := GetParentWnd( True )

        MOV      ECX, CW_USEDEFAULT
        MOV      EAX, [EBX].fBoundsRect.Bottom
        MOV      EDX, [EBX].fBoundsRect.Top
        SUB      EAX, EDX
        JNZ      @@1
        MOV      EAX, ECX
@@1:    PUSH     EAX         // Params.Height := Height | CW_UseDefault
        MOV      EAX, [EBX].fBoundsRect.Right
        SUB      EAX, [EBX].fBoundsRect.Left
        {$IFDEF USE_CMOV}
        CMOVZ    EAX, ECX
        {$ELSE}
        JNZ      @@2
        MOV      EAX, ECX
@@2:    {$ENDIF}

        PUSH     EAX         // Params.Width := Width | CW_UseDefault
        MOV      EAX, [EBX].fBoundsRect.Left
        {$IFDEF  USE_FLAGS}
        TEST     [EBX].fFlagsG3, 1 shl G3_IsControl
        {$ELSE}
        CMP      [EBX].fIsControl, CL
        {$ENDIF}
        JNZ      @@3
        {$IFDEF  USE_FLAGS}
        TEST     [EBX].fFlagsG2, (1 shl G2_ChangedPos)
        {$ELSE}
        TEST     byte ptr [EBX].fChangedPosSz, 3
        {$ENDIF  USE_FLAGS}
        JNZ      @@3
        MOV      EDX, ECX
        XCHG     EAX, ECX
@@3:    PUSH     EDX         // Params.Y := Top | CW_UseDefault
        PUSH     EAX         // Params.X := Left | CW_UseDefault
        PUSH     [EBX].fStyle    // Params.Style := fStyle
        PUSH     [EBX].fCaption  // Params.Caption := fCaption
        LEA      EAX, [ESP+40]
        PUSH     EAX         // Params.WinClassName := @Params.WinClsNamBuf
        PUSH     [EBX].fExStyle  // Params.ExStyle := fExStyle

        MOV      ECX, [EBX].fControlClassName
        JECXZ    @@registerClass
        LEA      EAX, [ESP].TCreateWndParams.WindowClass
        PUSH     EAX           // @Params.WindowClass
        PUSH     ECX           // fControlClassName
        PUSH     [hInstance]   // hInstance
        CALL     GetClassInfo
        MOV      EAX, [ESP].TCreateWndParams.Inst
        MOV      [ESP].TCreateWndParams.WindowClass.hInstance, EAX
        AND      [ESP].TCreateWndParams.WindowClass.style, not CS_OFF
@@registerClass:
        CMP      [EBX].fDefWndProc, 0
        JNE      @@fDefWndProc_ready
        MOV      EAX, [ESP].TCreateWndParams.WindowClass.lpfnWndProc
        MOV      [EBX].fDefWndProc, EAX
@@fDefWndProc_ready:
        MOV      ECX, [ESP].TCreateWndParams.WndParent
        TEST     ECX, ECX
        JNZ      @@registerClass1
        TEST     byte ptr [ESP].TCreateWndParams.Style+3, $40
        XCHG     EAX, ECX
        JNZ      @@fin
@@registerClass1:
        MOV      EAX, [ESP].TCreateWndParams.WinClassName
        MOV      EDX, [ESP].TCreateWndParams.WindowClass.hInstance
        ADD      ESP, -szWndClass
        PUSH     ESP
        PUSH     EAX
        PUSH     EDX
        CALL     GetClassInfo
        ADD      ESP, szWndClass
        TEST     EAX, EAX
        JNZ      @@registered
        MOV      EAX, [ESP].TCreateWndParams.WinClassName
        MOV      [ESP].TCreateWndParams.WindowClass.lpszClassName, EAX
        MOV      [ESP].TCreateWndParams.WindowClass.lpfnWndProc, offset WndFunc
        LEA      EAX, [ESP].TCreateWndParams.WindowClass
        PUSH     EAX
        CALL     RegisterClass
        TEST     EAX, EAX
        JZ       @@fin
@@registered:
        MOV      [CreatingWindow], EBX
        {$IFDEF DEBUG_CREATEWINDOW}
        MOV     EAX, EBX
        MOV     EDX, ESP
        CALL    Debug_CreateWindow2
        {$ENDIF}
        CALL     CreateWindowEx
        MOV      [EBX].fHandle, EAX
        TEST     EAX, EAX
        JZ       @@fin
        PUSH     EAX
        {$IFDEF USE_PROP}
        PUSH     offset ID_SELF
        {$ELSE}
        PUSH     GWL_USERDATA
        {$ENDIF}
        PUSH     EAX

        PUSH     0
        PUSH     $10002 //UIS_CLEAR or (UISF_HIDEFOCUS shl 16)
        PUSH     $0128  //WM_UPDATEUISTATE
        PUSH     EAX
        CALL     SendMessage

        {$IFDEF USE_PROP}
        CALL     GetProp
        {$ELSE}
        CALL     GetWindowLong
        {$ENDIF}
        XCHG     ECX, EAX
        POP      EAX
        INC      ECX
        LOOP     @@propSet
        MOV      [CreatingWindow], ECX
        PUSH     EBX
        {$IFDEF USE_PROP}
        PUSH     offset ID_SELF
        PUSH     EAX
        CALL     SetProp
        {$ELSE}
        PUSH     GWL_USERDATA
        PUSH     EAX
        CALL     SetWindowLong
        {$ENDIF}
@@propSet:
        {$IFDEF SMALLEST_CODE}
        {$ELSE}
        {$IFDEF  USE_FLAGS}
        TEST     [EBX].fFlagsG3, 1 shl G3_IsControl
        {$ELSE}
        CMP      [EBX].fIsControl, 0
        {$ENDIF}
        JNZ      @@iconSet
        MOV      EAX, EBX
        CALL     GetIcon
        PUSH     EAX
        PUSH     1
        PUSH     WM_SETICON
        PUSH     EBX
        CALL     Perform
@@iconSet:
        {$ENDIF}
        MOV      ECX, [EBX].PP.fCreateWndExt
        {$IFDEF  NIL_EVENTS}
        JECXZ    @@dblbufcreate
        {$ENDIF}
        MOV      EAX, EBX
        CALL     ECX
@@dblbufcreate:
@@applyfont:
        MOV      EAX, EBX
        CALL     [ApplyFont2Wnd_Proc]
        MOV      EAX, EBX
        CALL     [ApplyFont2Wnd_Proc]
@@createchildren:
        XCHG     EAX, EBX
        CALL     CreateChildWindows
        MOV      AL, 1
@@fin:
        MOV      ESP, EBP
        POP      EBP
@@ret_false:
        POP      EBX
end;


function WndProcKeybd( Self_: PControl; var Msg: TMsg; var Rslt: Integer): Boolean;
asm
        PUSH     EBX
        MOV      ECX, [EDX].TMsg.message
        SUB      CX, $100
        CMP      ECX, 5
        JA       @@fin_false
        XCHG     EBX, EAX   // EBX = @Self
        XCHG     EAX, ECX   // EAX = message - WM_KEYFIRST
        {$IFDEF  EVENTS_DYNAMIC}
        MOV      ECX, [EBX].TControl.EV
        LEA      ECX, [ECX].TEvents.fOnKeyUp
        {$ELSE}
        LEA      ECX, [EBX].TControl.EV.fOnKeyUp
        {$ENDIF}
        JZ       @@event
        {$IFDEF PARANOIA} DB $34, 1 {$ELSE} XOR AL, 1 {$ENDIF}
        JZ       @@event
        //LEA      ECX, [EBX].TControl.EV.fOnKeyDown
        ADD      ECX, 8
        {$IFDEF PARANOIA} DB $34, 1 {$ELSE} XOR AL, 1 {$ENDIF}
        JZ       @@event
        {$IFDEF PARANOIA} DB $34, 4 {$ELSE} XOR AL, 4 {$ENDIF}
        JZ       @@event
        //LEA      ECX, [EBX].TControl.EV.fOnChar
        SUB      ECX, 24
        {$IFDEF PARANOIA} DB $34, 6 {$ELSE} XOR AL, 2 xor 4 {$ENDIF}
        JZ       @@event
        {$IFDEF PARANOIA} DB $34, 4 {$ELSE} XOR AL, 6 xor 2 {$ENDIF}
        JNZ      @@fin_false
@@event:
        {$IFDEF  NIL_EVENTS}
        CMP      word ptr [ECX].TMethod.Code+2, 0
        JZ       @@fin_false
        {$ENDIF}
        PUSH     EDX
        PUSH     ECX
        LEA      ECX, [EDX].TMsg.wParam
        PUSH     ECX
        CALL     GetShiftState
        POP      ECX         // @wParam
        XCHG     EAX, [ESP]  // ShiftState; EAX=@event
        MOV      EDX, EBX    // @Self
        MOV      EBX, [EAX].TMethod.Code
        MOV      EAX, [EAX].TMethod.Data
        CALL     EBX

        POP      EDX
        MOV      ECX, [EDX].TMsg.wParam
        JECXZ    @@fin_true

@@fin_false:
        XOR      EAX, EAX
        POP      EBX
        RET

@@fin_true:
        MOV      AL, 1
        POP      EBX
end;

function TControl.GetCaption: KOLString;
asm
        PUSH      EBX
        PUSH      EDI
        XCHG      EBX, EAX
        MOV       EDI, EDX
        {$IFDEF   USE_FLAGS}
        TEST      [EBX].fFlagsG1, (1 shl G1_IgnoreWndCaption)
        {$ELSE}
        CMP       [EBX].fIgnoreWndCaption, 0
        {$ENDIF   USE_FLAGS}
        JNZ       @@getFCaption
        MOV       ECX, [EBX].fHandle
        JECXZ     @@getFCaption
@@getWndCaption:
        PUSH      ECX
        CALL      GetWindowTextLength
        PUSH      EAX
          XCHG      EDX, EAX
          LEA       EAX, [EBX].fCaption
          CALL      System.@LStrSetLength
        POP       ECX
        JECXZ     @@getFCaption
        INC       ECX
        PUSH      ECX
        PUSH      [EBX].fCaption
        PUSH      [EBX].fHandle
        CALL      GetWindowText
@@getFCaption:
        MOV       EDX, [EBX].fCaption
        XCHG      EAX, EDI
        {$IFNDEF UNICODE_CTRLS}
        CALL      System.@LStrAsg
        {$ELSE}
        CALL      System.@WStrFromPChar
        {$ENDIF}
@@exit:
        POP       EDI
        POP       EBX
end;

function TControl.get_ClassName: AnsiString;
asm
        PUSH      EBX
        XCHG      EBX, EAX
        XCHG      EAX, EDX
        MOV       EDX, [EBX].fControlClassName
        PUSH      EAX
        {$IFDEF _D2009orHigher}
        XOR       ECX, ECX
        {$ENDIF}
        CALL      System.@LStrFromPChar // EAX^ := String(EDX)
        POP       EAX
        {$IFDEF   USE_FLAGS}
        TEST      [EBX].fFlagsG6, 1 shl G6_CtlClassNameChg
        {$ELSE}
        CMP       [EBX].fCtlClsNameChg, 0
        {$ENDIF}
        JNZ       @@exit
        MOV       ECX, [EAX]
        MOV       EDX, offset[ @@obj ]
        CALL      System.@LStrCat3 // EAX^ := EDX + ECX
        JMP       @@exit

        {$IFDEF _D2009orHigher}
        DW        1252, 1  // CP_ANSI_LATIN1, Byte // TODO: CP_ACP
        {$ENDIF}
        DD        -1, 4    // FFFFFFFF 04000000 obj_, 0
@@obj:  DB        'obj_', 0
@@exit:
        POP       EBX
end;

function TControl.GetItems(Idx: Integer): AnsiString;
asm
        PUSH     ESI
        PUSH     EDI
        PUSH     EBX
        PUSH     EBP
        MOV      EBP, ESP

        MOV      EBX, EAX       // @Self
        MOV      ESI, EDX       // Idx
        MOV      EDI, ECX       // @Result

        CALL     Item2Pos
        PUSH     0              // push  0
        PUSH     EAX            // store Pos

        XCHG     EDX, EAX
        MOV      EAX, EBX
        CALL     Pos2Item       // EAX = Idx'
        XCHG     ESI, EAX       // ESI = Idx'

        XOR      EAX, EAX
        {$IFDEF  COMMANDACTIONS_OBJ}
        MOV      ECX, [EBX].fCommandActions
        MOVZX    ECX, [ECX].TCommandActionsObj.aGetItemLength
        {$ELSE}
        MOVZX    ECX, [EBX].fCommandActions.aGetItemLength
        {$ENDIF}
        JECXZ    @@ret_empty

        PUSH     ECX            // push aGetItemLength

        PUSH     EBX
        CALL     Perform

        TEST     EAX, EAX
        JZ       @@ret_empty

        PUSH     EAX              // save L
        ADD      EAX, 4

        CALL     System.@GetMem   // GetMem( L+4 )
        POP      EDX              // restore L
        LEA      ECX, [EDX+1]
        MOV      dword ptr [EAX], ECX
        {$IFDEF  COMMANDACTIONS_OBJ}
        MOV      ECX, [EBX].fCommandActions
        MOVZX    ECX, [ECX].TCommandActionsObj.aGetItemText
        {$ELSE}
        MOVZX    ECX, [EBX].fCommandActions.aGetItemText
        {$ENDIF}
        JECXZ    @@ret_buf

        PUSH     EDX              // save L

        PUSH     EAX
          PUSH     EAX            // push Buf
          PUSH     ESI            // push Idx

          PUSH     ECX            // push aGetItemText
          PUSH     EBX
          CALL     Perform
        POP      EAX

        POP      EDX
@@ret_buf:
        MOV      byte ptr [EAX + EDX], 0   // Buf[ L ] := #0

@@ret_empty:     // EAX = 0
        XCHG     EDX, EAX
        MOV      EAX, EDI
        PUSH     EDX
        {$IFDEF _D2009orHigher}
        XOR      ECX, ECX
        {$ENDIF}
        CALL     System.@LStrFromPChar
        POP      ECX
        JECXZ    @@exit
        XCHG     EAX, ECX
        CALL     System.@FreeMem
@@exit:
        MOV      ESP, EBP
        POP      EBP
        POP      EBX
        POP      EDI
        POP      ESI
end;

procedure TControl.SetItems(Idx: Integer; const Value: AnsiString);
asm
        PUSH     EDI
        PUSH     EBX
        XCHG     EBX, EAX
        XCHG     EDI, EDX       // EDI = Idx
        CALL     ECX2PChar
        PUSH     ECX       // @Value[1]

        {$IFDEF  COMMANDACTIONS_OBJ}
        MOV      ECX, [EBX].fCommandActions
        MOVZX    ECX, [ECX].TCommandActionsObj.aSetItemText
        {$ELSE}
        MOVZX    ECX, [EBX].fCommandActions.aSetItemText
        {$ENDIF}
        JECXZ    @@1

        PUSH     0
        PUSH     ECX

        MOV      EDX, EDI
        MOV      EAX, EBX
        CALL     Item2Pos
        PUSH     EAX            // store Strt

        MOV      EDX, EDI
        INC      EDX
        MOV      EAX, EBX
        CALL     Item2Pos
        POP      EDX            // EDX = Strt

        SUB      EAX, EDX
        PUSH     EAX            // store L

        MOV      EAX, EBX
        CALL     SetSelStart

        POP      EDX            // EDX = L
        PUSH     EBX            // prepare @Self for Perform
        XCHG     EAX, EBX
        CALL     SetSelLength

        // @Value[1] already in stack,
        // 0 already in stack
        // aSetItemText already in stack
        // @Self already in stack

        CALL     Perform
        JMP      @@exit

@@1:    // @Value[1] in stack already
        POP      EDX
        {$IFDEF  COMMANDACTIONS_OBJ}
        MOV      ECX, [EBX].fCommandActions
        MOVZX    ECX, [ECX].TCommandActionsObj.aDeleteItem
        {$ELSE}
        MOVZX    ECX, [EBX].fCommandActions.aDeleteItem
        {$ENDIF}
        JECXZ    @@exit

        {$IFNDEF  NOT_FIX_CURINDEX}
        PUSH     ESI
        PUSH     EBP

        PUSH     EDX

        MOV      EAX, EBX               // +AK
        CALL     GetCurIndex            // +AK
        XCHG     ESI, EAX    // ESI = TmpCurIdx

        MOV      EAX, EBX
        MOV      EDX, EDI
        CALL     GetItemData
        XCHG     EBP, EAX   // EBP = TmpData

        MOV      EDX, EDI
        MOV      EAX, EBX
        CALL     Delete

        MOV      EAX, EBX               // *AK
        MOV      EDX, EDI
        POP      ECX
        CALL     Insert

        MOV      ECX, EBP // ECX = TmpData
        MOV      EDX, EDI
        MOV      EAX, EBX
        CALL     SetItemData

        XCHG     EAX, EBX               // +AK
        MOV      EDX, ESI               // +AK
        CALL     SetCurIndex            // +AK

        POP      EBP
        POP      ESI
        {$ELSE NOT_FIX_CURINDEX}
        PUSH     EDX

        MOV      EDX, EDI
        MOV      EAX, EBX
        CALL     Delete

        XCHG     EAX, EBX
        XCHG     EDX, EDI

        POP      ECX
        CALL     Insert
        {$ENDIF NOT_FIX_CURINDEX}

@@exit:
        POP      EBX
        POP      EDI
end;

function TControl.Add(const S: KOLString): Integer;
asm
        PUSH     EBX
        MOV      EBX, EAX               // EBX = @Self

        {$IFDEF  COMMANDACTIONS_OBJ}
        MOV      ECX, [EBX].fCommandActions
        MOVZX    ECX, [ECX].TCommandActionsObj.aAddItem
        {$ELSE}
        MOVZX    ECX, [EBX].fCommandActions.aAddItem   // ECX = aAddItem
        {$ENDIF}
        JECXZ    @@chk_addtext

        CALL     EDX2PChar
        PUSH     EDX
        PUSH     0
        PUSH     ECX
        PUSH     EBX
        CALL     Perform
        PUSH     EAX

        MOV      EAX, EBX
        CALL     TControl.GetItemsCount
        XCHG     EAX, ECX
        LOOP     @@ret_EAX

        XCHG     EAX, EBX
        INC      ECX
        XOR      EDX, EDX
        CALL     TControl.SetItemSelected
@@ret_EAX:
        POP      EAX
        JMP      @@exit

@@chk_addtext:
        {$IFDEF  COMMANDACTIONS_OBJ}
        MOV      ECX, [EBX].fCommandActions
        MOV      ECX, [ECX].TCommandActionsObj.aAddText
        {$ELSE}
        MOV      ECX, [EBX].fCommandActions.aAddText
        {$ENDIF}
        JECXZ    @@add_text_simple

        CALL     ECX
        JMP      @@exit_0

@@add_text_simple:
        LEA      EAX, [EBX].fCaption
        CALL     System.@LStrCat
        MOV      EDX, [EBX].fCaption
        MOV      EAX, EBX
        CALL     SetCaption

@@exit_0:
        XOR      EAX, EAX
@@exit:
        POP      EBX
end;

function TControl.Insert(Idx: Integer; const S: AnsiString): Integer;
asm
         CALL    ECX2PChar
         PUSH    ECX
         {$IFDEF COMMANDACTIONS_OBJ}
         MOV     ECX, [EAX].fCommandActions
         MOVZX   ECX, [ECX].TCommandActionsObj.aInsertItem
         {$ELSE}
         MOVZX   ECX, [EAX].fCommandActions.aInsertItem
         {$ENDIF}
         JECXZ   @@exit_1

         PUSH    EDX
         PUSH    ECX
         PUSH    EAX
         CALL    Perform
         RET

@@exit_1:OR      EAX, -1
         POP     ECX
end;

procedure TTrayIcon.SetTooltip(const Value: AnsiString);
asm
        PUSH     EBX
        XCHG     EBX, EAX
        MOV      EAX, [EBX].fTooltip
        PUSH     EDX
        CALL     System.@LStrCmp
        POP      EDX
        JE       @@exit
        LEA      EAX, [EBX].fTooltip
        CALL     System.@LStrAsg
        CMP      [EBX].fActive, 0
        JE       @@exit
        XOR      EDX, EDX
        INC      EDX     // EDX = NIM_MODIFY
        XCHG     EAX, EBX
        CALL     SetTrayIcon
@@exit:
        POP      EBX
end;

procedure TTrayIcon.SetTrayIcon(const Value: DWORD);
const sz_tid = sizeof( TNotifyIconData );
asm
        CMP      [AppletTerminated], 0
        JE       @@1
        MOV      DL, NIM_DELETE
@@1:
        PUSH     EBX
        PUSH     ESI
        MOV      ESI, EAX
        MOV      EBX, EDX

        XOR      ECX, ECX
        PUSH     ECX
        ADD      ESP, -60
        MOV      EDX, [ESI].fToolTip
        CALL     EDX2PChar
        MOV      EAX, ESP
        MOV      CL, 63
        CALL     StrLCopy

        PUSH     [ESI].fIcon
        PUSH     CM_TRAYICON
        XOR      EDX, EDX
        CMP      BL, NIM_DELETE
        JE       @@2
        MOV      DL, NIF_ICON or NIF_MESSAGE or NIF_TIP
@@2:    PUSH     EDX
        PUSH     ESI
        MOV      EAX, [ESI].FWnd
        TEST     EAX, EAX
        JNZ      @@3
        MOV      EAX, [ESI].fControl
        MOV      EAX, [EAX].TControl.fHandle
@@3:
        PUSH     EAX
        PUSH     sz_tid

        PUSH     ESP
        PUSH     EBX
        CALL     Shell_NotifyIcon

        ADD      ESP, sz_tid
        POP      ESI
        POP      EBX
@@exit:
end;

function WndProcJustOneNotify( Control: PControl; var Msg: TMsg; var Rslt: Integer ) : Boolean;
asm
        PUSH     EBP
        MOV      EBP, ESP
        PUSHAD
        CALL     WndProcJustOne
        POPAD
        XOR      EAX, EAX
        PUSH     ECX
          MOV      ECX, [EDX].TMsg.message
          SUB      ECX, [JustOneMsg]
        POP      ECX
        JNE      @@exit
        MOV      [ECX], EAX
        CMP      [OnAnotherInstance].TMethod.Code, EAX
        JE       @@exit_1

        //MOV      EAX, (MAX_PATH + 3) and 0FFFFCh
        MOV      AH, 2
        SUB      ESP, EAX

        MOV      ECX, ESP
        PUSH     EAX
        PUSH     ECX
        PUSH     [EDX].TMsg.lParam
        CALL     GetWindowText

        MOV      EDX, ESP
        PUSH     0
        MOV      EAX, ESP
        {$IFDEF _D2009orHigher}
        XOR      ECX, ECX
        {$ENDIF}
        CALL     System.@LStrFromPChar

        MOV      EDX, [ESP]
        MOV      EAX, [OnAnotherInstance].TMethod.Data
        CALL     [OnAnotherInstance].TMethod.Code

        MOV      EAX, ESP
        CALL     System.@LStrClr
@@exit_1:
        MOV      AL, 1
@@exit:
        MOV      ESP, EBP
        POP      EBP
end;

function JustOneNotify( Wnd: PControl; const Identifier : AnsiString;
                        const aOnAnotherInstance: TOnAnotherInstance ) : Boolean;
asm
        PUSHAD
        MOV      EBP, ESP

        XCHG     EAX, EDX
        PUSH     EAX
        CALL     System.@LStrLen
        POP      EDX
        ADD      EAX, EAX
        SUB      ESP, EAX
        MOV      EAX, ESP
        CALL     StrPCopy
        PUSH     '.ega'
        PUSH     'sseM'
        PUSH     ESP
        CALL     RegisterWindowMessage
        MOV      [JustOneMsg], EAX
        TEST     EAX, EAX
          MOV      ESP, EBP
          POPAD
          JE       @@exit_f
        PUSHAD
        CALL     JustOne
        DEC      AL
        POPAD
        JZ       @@exit_t
          PUSH     EBX
          XCHG     EBX, EAX
          XOR      EDX, EDX
          XCHG     [EBX].TControl.fCaption, EDX
          PUSH     EDX
        CALL     GetCommandLine
        XCHG     EDX, EAX
        LEA      EAX, [EBX].TControl.fCaption
        {$IFDEF _D2009orHigher}
        PUSH     ECX
        XOR      ECX, ECX
        {$ENDIF}
        CALL     System.@LStrFromPChar
        {$IFDEF _D2009orHigher}
        POP      ECX
        {$ENDIF}
        MOV      EAX, EBX
        MOV      EDX, [EBX].TControl.fCaption
        CALL     TControl.SetCaption
        MOV      EAX, EBX
        CALL     TControl.GetWindowHandle
        TEST     EAX, EAX
        JZ       @@rest_cap
          PUSH     BSM_APPLICATIONS
          MOV      EDX, ESP
        PUSH     EAX
        PUSH     0
        PUSH     [JustOneMsg]
        PUSH     EDX
        PUSH     BSF_QUERY or BSF_IGNORECURRENTTASK
        CALL     BroadcastSystemMessage
          POP      EDX
@@rest_cap:
        LEA      EAX, [EBX].TControl.fCaption
        CALL     System.@LStrClr
        POP      EDX
        MOV      [EBX].TControl.fCaption, EDX
        MOV      EAX, EBX
        CALL     TControl.SetCaption
        POP      EBX
@@exit_f:
        XOR      EAX, EAX
        JMP      @@exit
@@exit_t:
        PUSHAD
        LEA      ESI, [aOnAnotherInstance]
        LEA      EDI, [OnAnotherInstance]
        MOVSD
        MOVSD
        MOV      EDX, offset[WndProcJustOneNotify]
        CALL     TControl.AttachProc
        POPAD
        MOV      AL, 1
@@exit:
end;

function TStrList.AppendToFile(const FileName: Ansistring): Boolean;
asm
        PUSH     EBX
        MOV      EBX, EDX
        PUSH     0
        MOV      EDX, ESP
        CALL     GetTextStr
        XCHG     EAX, EBX
        MOV      EDX, ofOpenWrite or ofOpenAlways
        CALL     FileCreate
        MOV      EBX, EAX
        INC      EAX
        JZ       @@exit
        DEC      EAX
        XOR      EDX, EDX
        XOR      ECX, ECX
        MOV      CL, spEnd
        CALL     FileSeek
        POP      EAX
        PUSH     EAX
        CALL     System.@LStrLen
        XCHG     ECX, EAX
        MOV      EAX, EBX
        POP      EDX
        PUSH     EDX
        CALL     FileWrite
        XCHG     EAX, EBX
        CALL     FileClose
@@exit:
        CALL     RemoveStr
        POP      EBX
end;

function TStrList.LoadFromFile(const FileName: AnsiString): Boolean;
asm
      PUSH     EAX
        XCHG     EAX, EDX
        MOV      EDX, ofOpenRead or ofShareDenyWrite or ofOpenExisting
        CALL     FileCreate
        INC      EAX
        JZ       @@exit
        DEC      EAX
        PUSH     EBX
        XCHG     EBX, EAX
        PUSH     0
        PUSH     EBX
        CALL     GetFileSize
        XOR      EDX, EDX
        PUSH     EDX
        XCHG     ECX, EAX
        MOV      EAX, ESP
        PUSH     ECX
        {$IFDEF _D2}
        CALL     _LStrFromPCharLen
        {$ELSE}
        {$IFDEF _D2009orHigher}
        PUSH EDX // ushort 0, CodePage?
        {$ENDIF}
        CALL     System.@LStrFromPCharLen
        {$ENDIF}
        POP      ECX
        MOV      EAX, EBX
        POP      EDX
        PUSH     EDX
        CALL     FileRead
        XCHG     EAX, EBX
        CALL     FileClose
        POP      EDX
        POP      EBX
      POP      EAX
        PUSH     EDX
        XOR      ECX, ECX
        CALL     SetText
        CALL     RemoveStr
        PUSH     EDX
        MOV      AL, 1
@@exit: POP      EDX
end;

function TStrList.SaveToFile(const FileName: Ansistring): Boolean;
asm
        PUSH     EBX
        PUSH     EAX
        XCHG     EAX, EDX
        MOV      EDX, ofOpenWrite or ofCreateAlways
        CALL     FileCreate
        INC      EAX
        JZ       @@exit
        DEC      EAX
        XCHG     EBX, EAX
        POP      EAX
        PUSH     0
        MOV      EDX, ESP
        CALL     GetTextStr
        POP      EAX
        PUSH     EAX
        CALL     System.@LStrLen
        XCHG     ECX, EAX
        POP      EDX
        PUSH     EDX
        MOV      EAX, EBX
        CALL     FileWrite
        PUSH     EBX
        CALL     SetEndOfFile
        XCHG     EAX, EBX
        CALL     FileClose
        CALL     RemoveStr
        PUSH     EDX
        INC      EAX
@@exit:
        POP      EDX
        POP      EBX
end;

procedure TControl.SetStatusText(Index: Integer; const Value: KOLString);
asm
        PUSHAD
        MOV      EBX, EDX // EBX = Index
        MOV      ESI, EAX // ESI = @Self
        PUSH     Value    // prepare value for call at the end of procedure
        PUSH     EBX      // prepare Index for call at the end of procedure
        MOV      ECX, [ESI].fStatusCtl
        MOV      EBP, ECX
        INC      ECX
        LOOP     @@status_created
        CALL     GetClientHeight
        PUSH     EAX      // ch = old client height
              MOV      EAX, ESI
              CALL     _NewStatusBar
              MOV      [ESI].fStatusCtl, EAX
              XCHG     EBP, EAX
              XOR      EDX, EDX
                       PUSH     EDX
                       INC      DH
                       DEC      EDX
                       CMP      EBX, EDX
                       SETZ     DL
                       NEG      EDX
                       PUSH     EDX
                       PUSH     SB_SIMPLE
                       PUSH     EBP
                       CALL     TControl.Perform
                  ADD      ESP, -16
                  PUSH     ESP
                  PUSH     [EBP].fHandle
                  CALL     GetWindowRect
                  POP      EAX
                  POP      EDX
                  POP      EAX
                  POP      EAX
              SUB      EAX, EDX
              MOV      [ESI].fClientBottom,  AL
        POP      EDX // ch
                 PUSH     0
                 PUSH     0
                 PUSH     WM_SIZE
                 PUSH     EBP
        MOV      EAX, ESI
        CALL     TControl.SetClientHeight
                 CALL     TControl.Perform
@@status_created:
        CMP      EBX, 255
        JGE      @@not_simple
                 PUSH     0
                 PUSH     0
                 PUSH     SB_GETPARTS
                 PUSH     EBP
                 CALL     Perform
        CMP      EAX, EBX
        JG       @@reset_simple
        MOV      EAX, ESI
        CALL     GetWidth
        CDQ
        MOV      ECX, EBX
        INC      ECX
        IDIV     ECX
        MOV      EDX, EAX
        ADD      ESP, -1024
        ///////////////////
        MOV      ECX, EBX
        MOV      EDI, ESP
        JECXZ    @@2
@@store_loo:
        STOSD
        ADD      EAX, EDX
        LOOP     @@store_loo
@@2:
        OR       dword ptr [ESP+EBX*4], -1
                 PUSH     ESP
        INC      EBX
                 PUSH     EBX
                 PUSH     SB_SETPARTS
                 PUSH     EBP
                 CALL     Perform
        ////////////////////
        ADD      ESP, 1024
@@reset_simple:
                 PUSH     0
                 PUSH     0
                 PUSH     SB_SIMPLE
                 PUSH     EBP
                 CALL     Perform
@@not_simple:
        PUSH     SB_SETTEXT
        PUSH     EBP
        CALL     Perform
        POPAD
end;

function TOpenSaveDialog.Execute: Boolean;
asm
        PUSH     EBX
        XCHG     EBX, EAX

        XOR      ECX, ECX
        {$IFDEF OpenSaveDialog_Extended}
        MOVZX    EAX, [EBX].NoPlaceBar
        PUSH     EAX
        PUSH     ECX
        PUSH     ECX
        PUSH     [EBX].TemplateName
        PUSH     [EBX].HookProc
        {$ELSE}
        PUSH     ECX                      // prepare lpTemplateName = nil
        PUSH     ECX                      // prepare lpfnHook = nil
        {$ENDIF}
        PUSH     EBX                      // prepare lCustData = @Self
        MOV      EDX, [EBX].FDefExtension
        CALL     EDX2PChar
        PUSH     EDX                      // prepare lpstrDefExt = FDefExtension
        PUSH     ECX                      // prepare nFileExtension, nFileOffset: Word = 0, 0
        // prepare flags:
        LEA      EAX, [EBX].FOptions
        MOV      EDX, Offset[@@OpenSaveFlags]
        {$IFDEF OpenSaveDialog_Extended}
        MOV      CL, 14
        {$ELSE}
        MOV      CL, 12
        {$ENDIF}
        CALL     MakeFlags
        XOR      ECX, ECX
        OR       EAX, OFN_EXPLORER or OFN_LONGNAMES or OFN_ENABLESIZING
        PUSH     EAX                       // push Flags
        PUSH     [EBX].FTitle              // prepare lpstrTitle
        PUSH     [EBX].FInitialDir         // prepare lpstrInitialDir
        PUSH     ECX                       // prepare nMaxFileTitle = 0
        PUSH     ECX                       // prepare lpstrFileTitle = nil
        TEST     AH, 2      // MultiSelect?
        MOV      EAX, 65520
        JNZ      @@1
        MOV      AX, MAX_PATH+2
@@1:    PUSH     EAX                       // prepare nMaxFile
        CALL     System.@GetMem
        POP      ECX
        PUSH     ECX
        PUSH     EAX                       // prepare lpStrFile
        XOR      EDX, EDX

@@2:    MOV      EDX, [EBX].fFileName // no, fill it initilly by FileName
        CALL     EDX2PChar
        DEC      ECX // added 5 october 2003 to prevent possible error if FileName too big
        CALL     StrLCopy
        XOR      EDX, EDX

        PUSH     [EBX].FFilterIndex        // prepare nFilterIndex
        PUSH     EDX                       // prepare nMaxCustFilter
        PUSH     EDX                       // prepare lpstrCustomFilter
        PUSH     EDX                       // prepare lpstrFilter = nil
        MOV      EAX, ESP
        OR       EDX, [EBX].FFilter
        JZ       @@5

        MOV      ECX, offset[@@0]
        CALL     System.@LStrCat3          // prepare lpStrFilter = FFilter + #0
        POP      EAX
        PUSH     EAX
        XOR      EDX, EDX
@@3:    INC      EAX   // filter is not starting from ';' or '|'...
        CMP      [EAX], DL
        JZ       @@5
        CMP      byte ptr [EAX], '|'
        JNZ      @@3
@@4:    MOV      [EAX], DL
        JMP      @@3
@@OpenSaveFlags:
        DD OFN_CREATEPROMPT, OFN_EXTENSIONDIFFERENT, OFN_FILEMUSTEXIST
        DD OFN_HIDEREADONLY, OFN_NOCHANGEDIR, OFN_NODEREFERENCELINKS
        DD OFN_ALLOWMULTISELECT, OFN_NONETWORKBUTTON, OFN_NOREADONLYRETURN
        DD OFN_OVERWRITEPROMPT, OFN_PATHMUSTEXIST, OFN_READONLY, OFN_NOVALIDATE
        {$IFDEF OpenSaveDialog_Extended}
        DD OFN_ENABLETEMPLATE, OFN_ENABLEHOOK
        {$ENDIF}

        {$IFDEF _D2009orHigher}
        DW       0, 1
        {$ENDIF}
        DD       -1, 1
@@0:    DB       0


@@5:
        PUSH     [hInstance]               // prepare hInstance

        MOV      ECX, [EBX].TControl.fWnd
        INC      ECX
        LOOP     @@6
        MOV      ECX, [Applet]
        JECXZ    @@6
        MOV      ECX, [ECX].TControl.fHandle
@@6:    PUSH     ECX                       // prepare hWndOwner
        {$IFDEF OpenSaveDialog_Extended}
        CALL     WinVer
        CMP      AL, wvNT
        MOV      DL, 76+12
        JA       @@6a
        CMP      AL, wvME
        JE       @@6a
        MOV      DL, 76
@@6a:   MOVZX    EAX, DL
        PUSH     EAX
        {$ELSE}
        PUSH     76                        // prepare lStructSize
        {$ENDIF}

        PUSH     ESP
        CMP      [EBX].TControl.FOpenDialog, 0
        JZ       @@7
        CALL     GetOpenFileName
        JMP      @@8
@@7:    CALL     GetSaveFileName
@@8:
        PUSH     EAX
        XOR      EDX, EDX
        TEST     EAX, EAX
        JZ       @@10

        MOV      EAX, [ESP+4].TOpenFileName.nFilterIndex
        MOV      [EBX].FFilterIndex, EAX

        TEST     BYTE PTR [ESP+4].TOpenFileName.Flags, OFN_READONLY
        SETNZ    AL
        MOV      [EBX].fOpenReadOnly, AL

        MOV      EAX, [ESP+4].TOpenFileName.lpstrFile
        MOV      EDX, EAX
        XOR      ECX, ECX

        TEST     [EBX].FOptions, 1 shl OSAllowMultiSelect
        JZ       @@10

        DEC      EAX
@@9:    INC      EAX
        CMP      byte ptr [EAX], CL
        JNZ      @@9
        CMP      byte ptr [EAX+1], CL
        JZ       @@10
        MOV      byte ptr [EAX], 13
        JMP      @@9

@@10:
        LEA      EAX, [EBX].FFileName
        {$IFDEF _D2009orHigher}
        XOR      ECX, ECX
        {$ENDIF}
        CALL     System.@LStrFromPChar
        MOV      EAX, [ESP+4].TOpenFileName.lpstrFile
        CALL     System.@FreeMem // v1.86 +AK

        LEA      EAX, [ESP+4].TOpenFileName.lpstrFilter
        CALL     System.@LStrClr

        POP      EAX
        {$IFDEF OpenSaveDialog_Extended}
        ADD      ESP, 76+12
        {$ELSE}
        ADD      ESP, 76
        {$ENDIF}
        POP      EBX
end;

function TOpenDirDialog.Execute: Boolean;
asm
          PUSH     EBX
          XCHG     EBX, EAX
        XOR      ECX, ECX
        PUSH     ECX             // prepare iImage = 0
        PUSH     EBX             // prepare lParam = @Self
        PUSH     [EBX].FCallBack // prepare lpfn = FCallBack
        LEA      EAX, [EBX].FOptions
        MOV      EDX, Offset[@@FlagsArray]
        MOV      CL, 8
        CALL     MakeFlags
        PUSH     EAX             // prepare ulFlags = Options
        PUSH     [EBX].FTitle    // prepare lpszTitle
        LEA      EAX, [EBX].FBuf
        PUSH     EAX             // prepare pszDisplayName
        PUSH     0               // prepare pidlRoot
        MOV      ECX, [EBX].fWnd
        INC      ECX
        LOOP     @@1
        MOV      ECX, Applet
        JECXZ    @@1
        MOV      ECX, [ECX].TControl.fHandle
@@1:    PUSH     ECX             // prepare hwndOwner
            PUSH     ESP
            CALL     SHBrowseForFolderA
            ADD      ESP, 32
            TEST     EAX, EAX
            JZ       @@exit
          PUSH     EAX
        LEA      EDX, [EBX].FBuf
        PUSH     EDX
        PUSH     EAX
        CALL     SHGetPathFromIDListA
            CALL     CoTaskMemFree
        MOV      AL, 1
        JMP      @@fin
@@FlagsArray:
        DD BIF_BROWSEFORCOMPUTER, BIF_BROWSEFORPRINTER, BIF_DONTGOBELOWDOMAIN
        DD BIF_RETURNFSANCESTORS, BIF_RETURNONLYFSDIRS, BIF_STATUSTEXT
        DD BIF_BROWSEINCLUDEFILES, BIF_EDITBOX, BIF_NEWDIALOGSTYLE
@@exit: XOR      EAX, EAX
@@fin:
        POP      EBX
end;

function OpenDirSelChangeCallBack( Wnd: HWnd; Msg: DWORD; lParam, lpData: LParam ):
         Integer; stdcall;
asm
        MOV      EAX, [lpData]
        MOV      ECX, [EAX].TOpenDirDialog.FOnSelChanged.TMethod.Code
        JECXZ    @@exit
          LEA      EDX, [EAX].TOpenDirDialog.FBuf
          PUSH     EDX
          PUSH     [lParam]
          CALL     SHGetPathFromIDListA
        MOV      EDX, [lpData]
        LEA      ECX, [EDX].TOpenDirDialog.FBuf
        PUSH     0
        PUSH     ESP
        LEA      EAX, [EDX].TOpenDirDialog.FStatusText
        PUSH     EAX
        MOV      EAX, [EDX].TOpenDirDialog.FOnSelChanged.TMethod.Data
        CALL     dword ptr [EDX].TOpenDirDialog.FOnSelChanged.TMethod.Code
        PUSH     0
        PUSH     BFFM_ENABLEOK
        PUSH     [Wnd]
        CALL     SendMessage
@@1:      MOV      EDX, [lpData]
          MOV      ECX, [EDX].TOpenDirDialog.FStatusText
          JECXZ    @@exit
        PUSH     ECX
        PUSH     0
        PUSH     BFFM_SETSTATUSTEXT
        PUSH     [Wnd]
        CALL     SendMessage
@@exit: XOR      EAX, EAX
end;

function TControl.TBAddInsButtons(Idx: Integer; const Buttons: array of PKOLChar;
  const BtnImgIdxArray: array of Integer): Integer; stdcall;
asm     { [EBP+$8] = @Self
          [EBP+$C] = Idx
          [EBP+$10] = Buttons
          [EBP+$14] = High(Butons)
          [EBP+$18] = BtnImgIdxArray
          [EBP+$1C] = High(BtnImgIdxArray) }
        PUSH     EBX
        PUSH     ESI
        PUSH     EDI
        OR       EBX, -1
        MOV      EAX, 20
        MOV      ECX, [EBP+$14]
        CMP      ECX, EBX
        JLE      @@fin
        INC      ECX
        MUL      ECX
        CALL     System.@GetMem
        PUSH     EAX           // save AB to FreeMem after
        MOV      EDX, EBX
        DEC      EDX           // nBmp := -2
          MOV      ECX, [EBP+$14]
          INC      ECX
          JZ       @@exit
        MOV      ECX, [EBP+$1C]
        INC      ECX
        JZ       @@1
        MOV      ECX, [BtnImgIdxArray]
        MOV      EDX, [ECX]
        DEC      EDX           // nBmp := BtnImgIdxArray[ 0 ] - 1
@@1:    MOV      ECX, [EBP+$14]
        INC      ECX
        MOV      ESI, [Buttons]
        MOV      EDI, EAX      // EDI = PAB
        PUSH     0             // N:=0 in [EBP-$14]
@@loop:
        LODSD
        TEST     EAX, EAX
        JZ       @@break
        PUSH     ECX
          CMP      word ptr [EAX], '-'
          JNE      @@2
        OR       EAX, -1
        STOSD
        MOV      EAX, [ToolbarsIDcmd]
        TEST     EBX, EBX
        {$IFDEF USE_CMOV}
        CMOVL    EBX, EAX
        {$ELSE}
        JGE      @@b0
        MOV      EBX, EAX
@@b0:   {$ENDIF}
          STOSD
          XOR      EAX, EAX
          INC      AH  // TBSTYLE_SEP = 1
          STOSD
          DEC      AH
          STOSD
          DEC      EAX
          JMP      @@3
        {$IFDEF _D2009orHigher}
        DW       0, 1
        {$ENDIF}
        DD       -1, 1
@@0:    DB       0
@@2:
          INC      EDX  // Inc( nBmp )
          PUSH     EAX
        MOV      EAX, [EBP+$1C]
        MOV      ECX, [EBP-$14]
        CMP      EAX, ECX
        MOV      EAX, EDX
        JL       @@21
        MOV      EAX, [BtnImgIdxArray]
        MOV      EAX, [EAX+ECX*4]
@@21:   STOSD
          TEST     EDX, EDX
          JGE      @@2a
          DEC      EDX
@@2a:
        MOV      EAX, [ToolbarsIDcmd]
        STOSD
        TEST     EBX, EBX
        {$IFDEF USE_CMOV}
        CMOVL    EBX, EAX
        {$ELSE}
        JGE      @@210
        MOV      EBX, EAX
@@210:  {$ENDIF}
          MOV      ECX, [EBP+8]
          MOV      AH,  BYTE PTR [ECX].TControl.DF.fDefaultTBBtnStyle
        POP      ECX
        MOV      AL, 4 // AL=fsState=_ENABLED, AH=fsStyle=_AUTOSIZE if fDefaultTBBtnStyle contains
        CMP      byte ptr [ECX], '^'
        JNE      @@22
        OR       AH, TBSTYLE_DROPDOWN
        INC      ECX
@@22:   CMP      byte ptr [ECX], '-'
        JZ       @@23
        CMP      byte ptr [ECX], '+'
        JNZ      @@24
        MOV      AL, TBSTATE_ENABLED or TBSTATE_CHECKED
@@23:   INC      ECX
        OR       AH, TBSTYLE_CHECK
        CMP      byte ptr [ECX], '!'
        JNZ      @@24
        OR       AH, TBSTYLE_GROUP
        INC      ECX
@@24:   {$IFDEF TOOLBAR_DOT_NOAUTOSIZE_BUTTON}
        CMP      byte ptr [ECX], '.'
        JNZ      @@25
        AND      AH, not TBSTYLE_AUTOSIZE
        INC      ECX
@@25:
        {$ENDIF TOOLBAR_DOT_NOAUTOSIZE_BUTTON}
          STOSD
          MOV      EAX, [EBP+8]
          STOSD
          OR       EAX, -1
          CMP      word ptr [ECX], ' '
          JZ       @@3
          CMP      byte ptr [ECX], 0
          JZ       @@3
        PUSH     EDX
        PUSH     0
        MOV      EDX, ECX
        MOV      EAX, ESP
        {$IFDEF _D2009orHigher}
        PUSH     ECX
        XOR      ECX, ECX
        {$ENDIF}
        CALL     System.@LStrFromPChar
        {$IFDEF _D2009orHigher}
        POP      ECX
        {$ENDIF}
        MOV      EAX, ESP
        MOV      EDX, offset[@@0]
        CALL     System.@LStrCat
        PUSH     dword ptr [ESP]
        PUSH     0
        PUSH     TB_ADDSTRING
        PUSH     dword ptr [EBP+8]
        CALL     Perform
        STOSD
          CALL     RemoveStr
          POP      EDX
          JMP      @@30
@@3:    STOSD
@@30:   INC      dword ptr [EBP-$14]
        INC      [ToolbarsIDcmd]
        POP      ECX
        DEC      ECX
        JNZ      @@loop
@@break:
          POP      ECX
          JECXZ    @@exit
          PUSH     dword ptr [ESP]
          MOV      EAX, [Idx]
          TEST     EAX, EAX
          JGE      @@31
        PUSH     ECX
        PUSH     TB_ADDBUTTONS
        JMP      @@32
@@31:
        PUSH     EAX
        PUSH     TB_INSERTBUTTON
@@32:
        PUSH     dword ptr [EBP+8]
        CALL     Perform
@@exit:
        POP      EAX
        CALL     System.@FreeMem
@@fin:
        POP      EDI
        POP      ESI
        XCHG     EAX, EBX
        POP      EBX
end;

function TControl.TBGetButtonText( BtnID: Integer ): AnsiString;
asm
        PUSH     ECX
        ADD      ESP, -1024
        PUSH     ESP
        PUSH     EAX
        CALL     GetTBBtnGoodID
        POP      EDX
        PUSH     EAX
        PUSH     TB_GETBUTTONTEXT
        PUSH     EDX
        CALL     Perform
        TEST     EAX, EAX
        JLE      @@2
        MOV      EDX, ESP
        JMP      @@1
@@2:    XOR      EDX, EDX
@@1:    MOV      EAX, [ESP+1024]
        {$IFDEF _D2009orHigher}
        XOR      ECX, ECX
        {$ENDIF}
        CALL     System.@LStrFromPChar
        ADD      ESP, 1028
end;

procedure TBitmap.LoadFromResourceName(Inst: DWORD; ResName: PAnsiChar);
asm
        PUSH     EBX
        MOV      EBX, EAX
        PUSHAD
        CALL     Clear
        POPAD
        XOR      EAX, EAX
        PUSH     ECX
        MOVZX    ECX, [EBX].fHandleType
        INC      ECX
        LOOP     @@1
        MOV      AH, LR_CREATEDIBSECTION shr 8 // = $2000
@@1:    MOV      AL, LR_DEFAULTSIZE // = $40
        POP      ECX
        PUSH     EAX
        PUSH     0
        PUSH     0
        PUSH     IMAGE_BITMAP
        PUSH     ECX
        PUSH     EDX
        CALL     LoadImage
        TEST     EAX, EAX
        JZ       @@exit
        XCHG     EDX, EAX
        XCHG     EAX, EBX
        CALL     SetHandle
@@exit: POP      EBX
end;
{$ENDIF}
