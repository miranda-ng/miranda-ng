//------------------------------------------------------------------------------
// KOL_ASM.inc (to inlude in KOL.pas)
// v 3.17

function MsgBox( const S: KOLString; Flags: DWORD ): DWORD;
asm
        PUSH      EDX
        PUSH      EAX

        MOV       ECX, [Applet]
        XOR       EAX, EAX
        {$IFDEF   SAFE_CODE}
        JECXZ     @@1
        {$ENDIF}
  {$IFDEF SNAPMOUSE2DFLTBTN}
        PUSHAD
        XCHG      EAX, ECX
        XOR       EDX, EDX
        PUSH      EDX
        PUSH      EDX
        PUSH      EDX
        PUSH      EAX
        MOV       EDX, offset[WndProcSnapMouse2DfltBtn]
        CALL      TControl.AttachProc
        CALL      TControl.Postmsg
        POPAD
  {$ENDIF}

        MOV       EAX, [ECX].TControl.fCaption
  {$IFDEF SNAPMOUSE2DFLTBTN}
        MOV       ECX, [ECX].TControl.fHandle
  {$ENDIF}
@@1:
        XCHG      EAX, [ESP]
        PUSH      EAX
        PUSH      0
        {$IFDEF UNICODE_CTRLS}
        CALL      MessageBoxW
        {$ELSE}
        CALL      MessageBox
        {$ENDIF}
  {$IFDEF SNAPMOUSE2DFLTBTN}
        MOV       ECX, [Applet]
        {$IFDEF   SAFE_CODE}
        JECXZ     @@2
        {$ENDIF}
        PUSH      EAX
        XCHG      EAX, ECX
        MOV       EDX, offset[WndProcSnapMouse2DfltBtn]
        CALL      TControl.DetachProc
        POP       EAX
@@2:
  {$ENDIF}
end;

function MakeRect( Left, Top, Right, Bottom: Integer ): TRect; stdcall;
asm
        PUSH       ESI
        PUSH       EDI

        MOV        EDI, @Result
        LEA        ESI, [Left]

        MOVSD
        MOVSD
        MOVSD
        MOVSD

        POP        EDI
        POP        ESI
end;

function RectsEqual( const R1, R2: TRect ): Boolean;
asm
        //LEA       EAX, [R1]
        //LEA       EDX, [R2]
        MOV       ECX, size_TRect
        CALL      CompareMem
end;

function PointInRect( const P: TPoint; const R: TRect ): Boolean;
asm
        PUSH      ESI
        MOV       ECX, EAX
        MOV       ESI, EDX
        LODSD
        CMP       EAX, [ECX]
        JG        @@fail
        LODSD
        CMP       EAX, [ECX+4]
        JG        @@fail
        LODSD
        CMP       [ECX], EAX
        JG        @@fail
        LODSD
        CMP       [ECX+4], EAX
@@fail: SETLE     AL
        POP       ESI
end;

function OffsetPoint( const T: TPoint; dX, dY: Integer ): TPoint;
asm
  ADD  EDX, [EAX].TPoint.X
  ADD  ECX, [EAX].TPoint.Y
  MOV  EAX, [Result]
  MOV  [EAX].TPoint.X, EDX
  MOV  [EAX].TPoint.Y, ECX
end;

function OffsetSmallPoint( const T: TSmallPoint; dX, dY: SmallInt ): TSmallPoint;
asm
  SHL  EDX, 16
  SHLD ECX, EDX, 16
  CALL @@1
@@1:
  ROL  EAX, 16
  ROL  ECX, 16
  ADD  AX, CX
end;

function Point2SmallPoint( const T: TPoint ): TSmallPoint;
asm
  XCHG EDX, EAX
  MOV  EAX, [EDX].TPoint.Y-2
  MOV  AX,  word ptr [EDX].TPoint.X
end;

function SmallPoint2Point( const T: TSmallPoint ): TPoint;
asm
  MOVSX ECX, AX
  MOV   [EDX].TPoint.X, ECX
  SAR   EAX, 16
  MOV   [EDX].TPoint.Y, EAX
end;

function MakePoint( X, Y: Integer ): TPoint;
asm
        MOV      ECX, @Result
        MOV      [ECX].TPoint.x, EAX
        MOV      [ECX].TPoint.y, EDX
end;

function MakeSmallPoint( X, Y: Integer ): TSmallPoint;
asm
  SHL EAX, 16
  SHRD EAX, EDX, 16
end;

function MakeFlags( FlgSet: PDWORD; FlgArray: array of Integer): Integer;
asm
        PUSH     EBX
        PUSH     ESI
        MOV      EBX, [EAX]
        MOV      ESI, EDX
        XOR      EDX, EDX
        INC      ECX
        JZ       @@exit
@@loo:
        LODSD
        TEST     EAX, EAX
        JGE      @@ge
        NOT      EAX
        TEST     BL, 1
        JZ       @@or
        DEC      EBX
@@ge:
        TEST     BL, 1
        JZ       @@nx
@@or:
        OR       EDX, EAX
@@nx:
        SHR      EBX, 1
        LOOP     @@loo

@@exit:
        XCHG     EAX, EDX
        POP      ESI
        POP      EBX
end;

constructor TObj.Create;
asm
        //CALL      System.@ObjSetup - Generated always by compiler
        //JZ        @@exit

        PUSH      EAX
        MOV       EDX, [EAX]
        CALL      dword ptr [EDX]
        POP       EAX

@@exit:
end;

{$IFDEF OLD_REFCOUNT}
procedure TObj.DoDestroy;
asm
        MOV       EDX, [EAX].fRefCount
        SAR       EDX, 1
        JZ        @@1
        JC        @@exit
        DEC       [EAX].fRefCount
        STC

@@1:    JC        @@exit
        MOV       EDX, [EAX]
        CALL      dword ptr [EDX + 4]
@@exit:
end;
{$ENDIF OLD_REFCOUNT}

function TObj.RefDec: Integer;
asm
  TEST  EAX, EAX
  JZ    @@exit

        SUB      [EAX].fRefCount, 2
        JGE      @@exit
  {$IFDEF OLD_REFCOUNT}
        TEST     [EAX].fRefCount, 1
        JZ       @@exit
        MOV      EDX, [EAX]
  {$ENDIF}
        MOV      EDX, [EAX]
        PUSH     dword ptr [EDX+4]
@@exit:
end;

{$IFDEF OLD_FREE}
procedure TObj.Free;
asm
   //TEST    EAX,EAX
   JMP     RefDec
end;
{$ENDIF OLD_FREE}

{$IFNDEF CRASH_DEBUG}
destructor TObj.Destroy;
asm
        PUSH      EAX
        CALL      Final
        POP       EAX
        {$IFDEF USE_NAMES}
        PUSH      EAX
        XOR   EDX, EDX
        XOR   ECX, ECX
        CALL  SetName
        POP       EAX
        PUSH    EAX
        XOR             ECX, ECX
        XCHG    ECX, [EAX].fNamedObjList
        XCHG    EAX, ECX
        CALL    TObj.RefDec
        POP             EAX
        {$ENDIF}
        XOR       EDX, EDX
        CALL      System.@FreeMem
        //CALL      System.@Dispose
end;
{$ENDIF}

procedure TObj.Add2AutoFree(Obj: PObj);
asm     //cmd    //opd
        PUSH     EBX
        PUSH     EDX
        XCHG     EBX, EAX
        MOV      EAX, [EBX].fAutoFree
        TEST     EAX, EAX
        JNZ      @@1
        CALL     NewList
        MOV      [EBX].fAutoFree, EAX
@@1:    MOV      EBX, EAX
        XOR      EDX, EDX
        POP      ECX
        CALL     TList.Insert
        XCHG     EAX, EBX
        XOR      EDX, EDX
        MOV      ECX, offset TObj.RefDec
        //XOR      ECX, ECX
        CALL     TList.Insert
        POP      EBX
end;

procedure TObj.Add2AutoFreeEx( Proc: TObjectMethod );
asm     //cmd    //opd
        PUSH     EBX
        XCHG     EAX, EBX
        MOV      EAX, [EBX].fAutoFree
        TEST     EAX, EAX
        JNZ      @@1
        CALL     NewList
        MOV      [EBX].fAutoFree, EAX
@@1:    XOR      EDX, EDX
        MOV      ECX, [EBP+12] // Data
        MOV      EBX, EAX
        CALL     TList.Insert
        XCHG     EAX, EBX
        XOR      EDX, EDX
        MOV      ECX, [EBP+8] // Code
        CALL     TList.Insert
        POP      EBX
end;

procedure TObj.RemoveFromAutoFree(Obj: PObj);
asm
  PUSH  EBX
  XCHG  EBX, EAX
  MOV   ECX, [EBX].fAutoFree
  JECXZ @@exit
  XCHG  EAX, ECX
  PUSH  EAX
    CALL  TList.IndexOf
    TEST  EAX, EAX
  POP   EDX
  XCHG  EDX, EAX
  JL    @@exit
  PUSH  EAX
    AND   EDX, not 1
    XOR   ECX, ECX
    MOV   CL, 2
    CALL  TList.DeleteRange
  POP   EAX
  MOV   ECX, [EAX].TList.fCount
  INC   ECX
  LOOP  @@exit
  LEA   EAX, [EBX].fAutoFree
  CALL  Free_And_Nil
@@exit:
  POP   EBX
end;

destructor TList.Destroy;
asm
        PUSH      EAX
        CALL      TList.Clear
        POP       EAX
        CALL      TObj.Destroy
end;

procedure TList.SetCapacity( Value: Integer );
asm
  {$IFDEF TLIST_FAST}
  CMP  [EAX].fUseBlocks, 0
  JZ   @@old
  CMP  [EAX].fBlockList, 0
  JZ   @@old

  XOR  ECX, ECX
  MOV  CH,  1
  CMP  EDX, ECX
  JLE  @@256
  MOV  EDX, ECX
@@256:

@@just_set:
  MOV  [EAX].fCapacity, EDX
  RET
@@old:
  {$ENDIF}
        CMP       EDX, [EAX].fCount
        {$IFDEF USE_CMOV}
        CMOVL     EDX, [EAX].fCount
        {$ELSE}
        JGE       @@1
        MOV       EDX, [EAX].fCount
@@1:    {$ENDIF}
        CMP       EDX, [EAX].fCapacity
        JE        @@exit

        MOV       [EAX].fCapacity, EDX
        SAL       EDX, 2
        LEA       EAX, [EAX].fItems
        CALL      System.@ReallocMem
@@exit:
end;

procedure TList.Clear;
asm
  {$IFDEF TLIST_FAST}
  PUSH EAX
  MOV  ECX, [EAX].fBlockList
  JECXZ @@1
  MOV  EDX, [ECX].fItems
  MOV  ECX, [ECX].fCount
  SHR  ECX, 1
  JZ   @@1
@@0:
  MOV  EAX, [EDX]
  ADD  EDX, 8
  PUSH EDX
  PUSH ECX
  CALL System.@FreeMem
  POP  ECX
  POP  EDX
  LOOP @@0
@@1:
  POP  EAX
  PUSH EAX
  XOR  EDX, EDX
  MOV  [EAX].fLastKnownBlockIdx, EDX
  LEA  EAX, [EAX].fBlockList
  CALL Free_And_Nil
  POP  EAX
  {$ENDIF}
        PUSH      [EAX].fItems
        XOR       EDX, EDX
        MOV       [EAX].fItems, EDX
        MOV       [EAX].fCount, EDX
        MOV       [EAX].fCapacity, EDX
        POP       EAX
        CALL      System.@FreeMem
end;

{$IFDEF ASM_NO_VERSION}
procedure TList.Add( Value: Pointer );
asm
        PUSH      EDX
        {$IFDEF TLIST_FAST}
        //if fUseBlocks and ((fCount >= 256) or Assigned( fBlockList )) then
        CMP       [EAX].fUseBlocks, 0
        JZ        @@old
        MOV       ECX, [EAX].fBlockList
        CMP       [EAX].fCount, 256
        JGE       @@1
        JECXZ     @@old
@@1:
        PUSH      EBX
        PUSH      ESI
        XCHG      EBX, EAX               // EBX == @Self
        MOV       ESI, ECX
        //if fBlockList = nil then
        INC       ECX
        LOOP      @@2
        CALL      NewList
        XCHG      ESI, EAX               // ESI == fBlockList
        MOV       [EBX].fBlockList, ESI  //fBlockList := NewList;
        MOV       [ESI].fUseBlocks, 0    //fBlockList.fUseBlocks := FALSE;
        XOR       EDX, EDX
        XCHG      EDX, [EBX].fItems      //fItems := nil;
        MOV       EAX, ESI
        CALL      TList.Add              //fBlockList.Add( fItems );
        MOV       EDX, [EBX].fCount
        MOV       EAX, ESI
        CALL      TList.Add              //fBlockList.Add( Pointer( fCount ) );
@@2:
        //if fBlockList.fCount = 0 then
        MOV       ECX, [ESI].fCount
        JECXZ     @@2A
        //LastBlockCount := Integer( fBlockList.fItems[ fBlockList.fCount-1 ] );
        MOV       EDX, [ESI].fItems
        MOV       EAX, [EDX+ECX*4-4]
        //if LastBlockCount >= 256 then
        CMP       EAX, 256
        JL        @@3
@@2A:
        MOV       EAX, ESI
        XOR       EDX, EDX
        CALL      TList.Add             //fBlockList.Add( nil );
        MOV       EAX, ESI
        XOR       EDX, EDX
        CALL      TList.Add             //fBlockList.Add( nil );
        XOR       EAX, EAX              //LastBlockCount := 0;
@@3:
        PUSH      EAX
        //LastBlockStart := fBlockList.Items[ fBlockList.fCount-2 ];
        MOV       ECX, [ESI].fCount
        MOV       EDX, [ESI].fItems
        LEA       EDX, [EDX+ECX*4-8]
        MOV       EAX, [EDX]
        //if LastBlockStart = nil then
        TEST      EAX, EAX
        JNZ       @@4
        //GetMem( LastBlockStart, 256 * Sizeof( Pointer ) );
        PUSH      EDX
          //MOV       EAX, 1024
          XOR       EAX, EAX
          MOV       AH,  4
          CALL      System.@GetMem
        POP       EDX
        //fBlockList.Items[ fBlockList.fCount-2 ] := LastBlockStart;
        MOV       [EDX], EAX
@@4:
        //fBlockList.Items[ fBlockList.fCount-1 ] := Pointer( LastBlockCount+1 );
        INC       dword ptr[EDX+4]
        POP       ECX      // ECX == LastBlockCount

        //inc( fCount );
        INC       [EBX].fCount
        //PDWORD( Integer(LastBlockStart) + Sizeof(Pointer)*LastBlockCount )^ :=
        //        DWORD( Value );

        POP       ESI
        POP       EBX
        POP       EDX      // EDX == Value
        MOV       [EAX+ECX*4], EDX
        RET
@@old:
        {$ENDIF TLIST_FAST}
        LEA       ECX, [EAX].fCount
        MOV       EDX, [ECX]
        INC       dword ptr [ECX]
          PUSH      EDX
          CMP       EDX, [EAX].fCapacity
            PUSH      EAX
            JL        @@ok

            MOV       ECX, [EAX].fAddBy
            TEST      ECX, ECX
            JNZ       @@add
            MOV       ECX, EDX
            SHR       ECX, 2
            INC       ECX
          @@add:
            ADD       EDX, ECX
            CALL      TList.SetCapacity
@@ok:
            POP       ECX  // ECX = Self
          POP       EAX    // EAX = fCount -> Result (for TList.Insert)
        POP       EDX      // EDX = Value

        MOV       ECX, [ECX].fItems
        MOV       [ECX + EAX*4], EDX
end;
{$ENDIF}

{$IFDEF MoveItem_ASM}
procedure TList.MoveItem(OldIdx, NewIdx: Integer);
asm
        CMP       EDX, ECX
        JE        @@exit

        CMP       ECX, [EAX].fCount
        JGE       @@exit

        PUSH      EDI

        MOV       EDI, [EAX].fItems
        PUSH      dword ptr [EDI + EDX*4]
          PUSH      ECX
          PUSH      EAX
          CALL      TList.Delete
          POP       EAX
          POP       EDX
        POP       ECX

        POP       EDI
        CALL      TList.Insert
@@exit:
end;
{$ENDIF}

procedure TList.Put( Idx: Integer; Value: Pointer );
asm
  TEST   EDX, EDX
  JL     @@exit
  CMP    EDX, [EAX].fCount
  JGE    @@exit
  PUSH   ESI
  MOV    ESI, ECX
  {$IFDEF TLIST_FAST}
  CMP    [EAX].fUseBlocks, 0
  JZ     @@old
  MOV    ECX, [EAX].fBlockList
  JECXZ  @@old
  PUSH   EBX
  PUSH   ESI
  PUSH   EDI
  PUSH   EBP
  XCHG   EBX, EAX // EBX == @Self
  XOR    ECX, ECX // CountBefore := 0;
  XOR    EAX, EAX // i := 0;
  CMP    [EBX].fLastKnownBlockIdx, 0
  JLE    @@1
  CMP    EDX, [EBX].fLastKnownCountBefore
  JL     @@1
  MOV    ECX, [EBX].fLastKnownCountBefore
  MOV    EAX, [EBX].fLastKnownBlockIdx
@@1:
  MOV    ESI, [EBX].fBlockList
  MOV    ESI, [ESI].fItems
  MOV    EDI, [ESI+EAX*8]   // EDI = BlockStart
  MOV    ESI, [ESI+EAX*8+4] // ESI = CountCurrent
  CMP    ECX, EDX
  JG     @@next
  LEA    EBP, [ECX+ESI]
  CMP    EDX, EBP
  JGE    @@next
  MOV    [EBX].fLastKnownBlockIdx, EAX
  MOV    [EBX].fLastKnownCountBefore, ECX
  SUB    EDX, ECX
  LEA    EAX, [EDI+EDX*4]
  POP    EBP
  POP    EDI
  POP    ESI
  POP    EBX
  MOV    [EAX], ESI
  POP    ESI
  RET
@@next:
  ADD    ECX, ESI
  INC    EAX
  JMP    @@1
@@old:
  {$ENDIF}
  MOV    EAX, [EAX].fItems
  MOV    [EAX+EDX*4], ESI
  POP    ESI
@@exit:
end;

function TList.Get( Idx: Integer ): Pointer;
asm
  TEST   EDX, EDX
  JL     @@ret_nil
  CMP    EDX, [EAX].fCount
  JGE    @@ret_nil
  {$IFDEF TLIST_FAST}
  CMP    [EAX].fUseBlocks, 0
  JZ     @@old
  CMP    [EAX].fNotOptimized, 0
  JNZ    @@slow

  MOV    ECX, [EAX].fBlockList
  JECXZ  @@old
  MOV    ECX, [ECX].fItems
  MOV    EAX, EDX
  SHR    EAX, 8
  MOV    ECX, dword ptr [ECX+EAX*8]
  MOVZX  EAX, DL
  MOV    EAX, dword ptr [ECX+EAX*4]
  RET

@@slow:
  MOV    ECX, [EAX].fBlockList
  JECXZ  @@old
  PUSH   EBX
  PUSH   ESI
  PUSH   EDI
  PUSH   EBP
  XCHG   EBX, EAX // EBX == @Self
  XOR    ECX, ECX // CountBefore := 0;
  XOR    EAX, EAX // i := 0;
  CMP    [EBX].fLastKnownBlockIdx, 0
  JLE    @@1
  CMP    EDX, [EBX].fLastKnownCountBefore
  JL     @@1
  MOV    ECX, [EBX].fLastKnownCountBefore
  MOV    EAX, [EBX].fLastKnownBlockIdx
@@1:
  MOV    ESI, [EBX].fBlockList
  MOV    ESI, [ESI].fItems
  MOV    EDI, [ESI+EAX*8]   // EDI = BlockStart
  MOV    ESI, [ESI+EAX*8+4] // ESI = CountCurrent
  CMP    ECX, EDX
  JG     @@next
  LEA    EBP, [ECX+ESI]
  CMP    EDX, EBP
  JGE    @@next
  MOV    [EBX].fLastKnownBlockIdx, EAX
  MOV    [EBX].fLastKnownCountBefore, ECX
  SUB    EDX, ECX
  MOV    EAX, [EDI+EDX*4]
  POP    EBP
  POP    EDI
  POP    ESI
  POP    EBX
  RET
@@next:
  ADD    ECX, ESI
  INC    EAX
  JMP    @@1
@@old:
  {$ENDIF}
  MOV    EAX, [EAX].fItems
  MOV    EAX, [EAX+EDX*4]
  RET
@@ret_nil:
  XOR    EAX, EAX
end;

procedure TerminateExecution( var AppletCtl: PControl );
asm
          PUSH EBX
          PUSH ESI
          MOV  BX, $0100
          XCHG BX, word ptr [AppletRunning]
          XOR  ECX, ECX
          XCHG ECX, [Applet]
          JECXZ @@exit

          PUSH EAX

          XCHG EAX, ECX
          MOV  ESI, EAX
          CALL TObj.RefInc

          TEST BH, BH
          JNZ  @@closed

          MOV  EAX, ESI
          CALL TControl.ProcessMessages
          PUSH 0
          PUSH 0
          PUSH WM_CLOSE
          PUSH ESI
          CALL TControl.Perform
@@closed:
          POP  EAX
          XOR  ECX, ECX
          MOV  dword ptr [EAX], ECX
          MOV  EAX, ESI
          CALL TObj.RefDec
          XCHG EAX, ESI
          CALL TObj.RefDec
@@exit:
          POP  ESI
          POP  EBX
end;

procedure Run( var AppletCtl: PControl );
asm
        CMP       EAX, 0
        JZ        @@exit
        PUSH      EBX
        XCHG      EBX, EAX
        INC       [AppletRunning]
        MOV       EAX, [EBX]
        MOV       [Applet], EAX
        CALL      CallTControlCreateWindow
        JMP       @@2
@@1:
        CALL      WaitMessage
        MOV       EAX, [EBX]
        CALL      TControl.ProcessMessages
        {$IFDEF   USE_OnIdle}
        MOV       EAX, [EBX]
        CALL      [ProcessIdle]
        {$ENDIF}
@@2:
        MOVZX     ECX, [AppletTerminated]
        JECXZ     @@1

        MOV       ECX, [EBX]
        XCHG      EAX, EBX
        POP       EBX
        JECXZ     @@exit
        CALL      TerminateExecution
@@exit:
end;

function SimpleGetCtlBrushHandle( Sender: PControl ): HBrush;
asm     //        //
        {$IFDEF SMALLEST_CODE}
        PUSH      COLOR_BTNFACE
        CALL      GetSysColorBrush
        {$ELSE}
@@1:    MOV       ECX, [EAX].TControl.fParent
        JECXZ     @@2
        MOV       EDX, [EAX].TControl.fColor
        CMP       EDX, [ECX].TControl.fColor
        XCHG      EAX, ECX
        JE        @@1
        XCHG      EAX, ECX
@@2:    {$IFDEF   STORE_fTmpBrushColorRGB}
        PUSH      EBX
        XCHG      EBX, EAX
        MOV       ECX, [EBX].TControl.fTmpBrush
        JECXZ     @@3
        MOV       EAX, [EBX].TControl.fColor
        CALL      Color2RGB
        CMP       EAX, [EBX].TControl.fTmpBrushColorRGB
        JE        @@3
        XOR       EAX, EAX
        XCHG      [EBX].TControl.fTmpBrush, EAX
        PUSH      EAX
        CALL      DeleteObject
@@3:    MOV       EAX, [EBX].TControl.fTmpBrush
        TEST      EAX, EAX
        JNE       @@4
        MOV       EAX, [EBX].TControl.fColor
        CALL      Color2RGB
        MOV       [EBX].TControl.fTmpBrushColorRGB, EAX
        PUSH      EAX
        CALL      CreateSolidBrush
        MOV       [EBX].TControl.fTmpBrush, EAX
@@4:    POP       EBX
        {$ELSE}
        XCHG      ECX, EAX
        MOV       EAX, [ECX].TControl.fTmpBrush
        TEST      EAX, EAX
        JNZ       @@ret_EAX
        PUSH      ECX
        MOV       EAX, [ECX].TControl.fColor
        CALL      Color2RGB
        PUSH      EAX
        CALL      CreateSolidBrush
        POP       ECX
        MOV       [ECX].TControl.fTmpBrush, EAX
@@ret_EAX:
        {$ENDIF   not STORE_fTmpBrushColorRGB}
        {$ENDIF   not SMALLEST_CODE}
end;

function NormalGetCtlBrushHandle( Sender: PControl ): HBrush;
asm
        PUSH  ESI
        PUSH  [EAX].TControl.fParent
        CALL  TControl.GetBrush
        XCHG  ESI, EAX // ESI = Sender.Brush
        POP   ECX
        JECXZ @@retHandle
        XCHG  EAX, ECX
        CALL  TControl.GetBrush
        MOV   [ESI].TGraphicTool.fParentGDITool, EAX
@@retHandle:
        XCHG  EAX, ESI
        CALL  TGraphicTool.GetHandle
        POP   ESI
end;

function NewBrush: PGraphicTool;
asm
        MOV      [Global_GetCtlBrushHandle], offset NormalGetCtlBrushHandle
        CALL     _NewGraphicTool
        MOV      [EAX].TGraphicTool.fNewProc, offset[NewBrush]
        MOV      [EAX].TGraphicTool.fType, gttBrush
        MOV      [EAX].TGraphicTool.fMakeHandleProc, offset[MakeBrushHandle]
        MOV      [EAX].TGraphicTool.fData.Color, clBtnFace
end;

function NewFont: PGraphicTool;
const FontDtSz = sizeof( TGDIFont );
asm
  MOV EAX, offset[DoApplyFont2Wnd]
  MOV [ApplyFont2Wnd_Proc], EAX
        CALL     _NewGraphicTool
        MOV      [EAX].TGraphicTool.fNewProc, offset[NewFont]
        MOV      [EAX].TGraphicTool.fType, gttFont
        MOV      [EAX].TGraphicTool.fMakeHandleProc, offset[MakeFontHandle]
        MOV      EDX, [DefFontColor]
        MOV      [EAX].TGraphicTool.fData.Color, EDX

        PUSH     EAX
        LEA      EDX, [EAX].TGraphicTool.fData.Font
        MOV      EAX, offset[ DefFont ]
        XOR      ECX, ECX
        MOV      CL, FontDtSz
        CALL     System.Move
        POP      EAX
end;

function NewPen: PGraphicTool;
asm
        CALL     _NewGraphicTool
        MOV      [EAX].TGraphicTool.fNewProc, offset[NewPen]
        MOV      [EAX].TGraphicTool.fType, gttPen
        MOV      [EAX].TGraphicTool.fMakeHandleProc, offset[MakePenHandle]
        MOV      [EAX].TGraphicTool.fData.Pen.Mode, pmCopy
end;

function Color2RGB( Color: TColor ): TColor;
asm
         BTR  EAX, 31
         JNC  @@exit
         AND  EAX , $7F    // <- a Fix Hallif
         PUSH      EAX
         CALL      GetSysColor
@@exit:
end;

function Color2RGBQuad( Color: TColor ): TRGBQuad;
asm
        CALL     Color2RGB
        // code by bart:
        xchg    ah,al                   // xxRRGGBB
        ror     eax,16                  // BBGGxxRR
        xchg    ah,al                   // BBGGRRxx
        shr     eax,8                   // 00BBGGRR
end;

function Color2Color16( Color: TColor ): WORD;
asm
  MOV  EDX, EAX
  SHR  EDX, 19
  AND  EDX, $1F
  MOV  ECX, EAX
  SHR  ECX, 5
  AND  ECX, $7E0;
  MOV  AH, AL
  AND  EAX, $F800
  OR   EAX, EDX
  OR   EAX, ECX
end;

function TGraphicTool.Assign(Value: PGraphicTool): PGraphicTool;
const SzfData = sizeof( fData );
asm     //        //
        TEST      EDX, EDX
        JNZ       @@1
        {$IFDEF OLD_REFCOUNT}
        TEST      EAX, EAX
        JZ        @@0
        CALL      TObj.DoDestroy
        {$ELSE}
        CALL      TObj.RefDec
        {$ENDIF}
        XOR       EAX, EAX
@@0:    RET
@@1:    PUSH      EDI
        MOV       EDI, EDX
        TEST      EAX, EAX
        JNZ       @@2
        XCHG      EAX, EDX
        CALL      dword ptr[EAX].TGraphicTool.fNewProc
@@2:    CMP       EAX, EDI
        JE        @@exit
        PUSH      EBX
        XCHG      EBX, EAX

        MOV       ECX, [EBX].TGraphicTool.fHandle
        JECXZ     @@3
        CMP       ECX, [EDI].TGraphicTool.fHandle
        JE        @@exit1
@@3:
        MOV       EAX, EBX
        CALL      TGraphicTool.Changed
        LEA       EDX, [EBX].TGraphicTool.fData
        LEA       EAX, [EDI].TGraphicTool.fData
        MOV       ECX, SzfData
        CALL      System.Move
        MOV       EAX, EBX
        CALL      TGraphicTool.Changed

@@exit1:
        XCHG      EAX, EBX
        POP       EBX
@@exit: POP       EDI
end;

procedure TGraphicTool.Changed;
asm
        XOR      ECX, ECX
        XCHG     ECX, [EAX].fHandle
        JECXZ    @@exit
        PUSH     EAX
        PUSH     ECX

        CALL     @@CallOnChange

        CALL     DeleteObject
        POP      EAX
@@exit:

@@CallOnChange:
        MOV      ECX, [EAX].fOnGTChange.TMethod.Code
        JECXZ    @@no_onChange
        PUSH     EAX
        XCHG     EDX, EAX
        MOV      EAX, [EDX].fOnGTChange.TMethod.Data
        CALL     ECX
        POP      EAX
@@no_onChange:
end;

destructor TGraphicTool.Destroy;
asm
          PUSH      EAX
          CMP       [EAX].fType, gttFont
          JE        @@0
          MOV       ECX, [EAX].fData.Brush.Bitmap
          JECXZ     @@0
          PUSH      ECX
          CALL      DeleteObject
          POP       EAX
          PUSH      EAX
@@0:
        MOV       ECX, [EAX].fHandle
        JECXZ     @@1
        PUSH      ECX
        CALL      DeleteObject
@@1:
          POP       EAX
          CALL      TObj.Destroy
end;

function TGraphicTool.ReleaseHandle: THANDLE;
asm     //        //
        PUSH      EAX
        CALL      Changed
        POP       EDX
        XOR       EAX, EAX
        XCHG      [EDX].fHandle, EAX
end;

procedure TGraphicTool.SetInt( const Index: Integer; Value: Integer );
asm
        LEA    EDX, [EDX+EAX].fData
        CMP    [EDX], ECX
        JE     @@exit
        MOV    [EDX], ECX
        CALL   Changed
@@exit:
end;

function TGraphicTool.IsFontTrueType: Boolean;
asm
        CALL     GetHandle
        TEST     EAX, EAX
        JZ       @@exit

        PUSH     EBX

        PUSH     EAX                  // fHandle

        PUSH     0
        CALL     GetDC

        PUSH     EAX                  // DC
        MOV      EBX, EAX
        CALL     SelectObject
        PUSH     EAX

        XOR      ECX, ECX
        PUSH     ECX
        PUSH     ECX
        PUSH     ECX
        PUSH     ECX
        PUSH     EBX
        CALL     GetFontData

        XCHG     EAX, [ESP]

        PUSH     EAX
        PUSH     EBX
        CALL     SelectObject

        PUSH     EBX
        PUSH     0
        CALL     ReleaseDC

        POP      EAX
        INC      EAX
        SETNZ    AL

        POP      EBX
@@exit:
end;

procedure TextAreaEx( Sender: PCanvas; var Sz : TSize; var Pt : TPoint );
asm
        PUSH     EBX
        PUSH     ESI
        PUSH     EDI
        PUSH     EBP
        MOV      EBP, ESP
        PUSH     EDX // [EBP-4] = @Sz
        PUSH     ECX // [EBP-8] = @Pt
        MOV      EBX, EAX
        CALL     TCanvas.GetFont
        MOV      ESI, [EAX].TGraphicTool.fData.Font.Orientation
        CALL     TGraphicTool.IsFontTrueType
        TEST     AL, AL
        JZ       @@exit

        MOV      EDI, [EBP-8]
        XOR      EAX, EAX
        STOSD
        STOSD
        TEST     ESI, ESI
        JZ       @@exit

        PUSH     EAX // Pts[1].x
        PUSH     EAX // Pts[1].y

        PUSH     ESI
        FILD     dword ptr [ESP]
        POP      EDX

        FILD     word ptr [@@1800]
        FDIV
        //FWAIT
        FLDPI
        FMUL
        //FWAIT

        FLD      ST(0)
        FSINCOS
        FWAIT

        MOV      ESI, [EBP-4]
        LODSD         // Sz.cx
        PUSH     EAX
        FILD     dword ptr [ESP]
        FMUL
        FISTP    dword ptr [ESP] // Pts[2].x
        FWAIT
        NEG      EAX
        PUSH     EAX
        FILD     dword ptr [ESP]
        FMUL
        FISTP    dword ptr [ESP] // Pts[2].y
        FWAIT

        FLDPI
        FLD1
        FLD1
        FADD
        FDIV
        FADD
        FSINCOS
        FWAIT

        LODSD
        NEG      EAX
        PUSH     EAX
        FILD     dword ptr [ESP]
        FMUL
        FISTP    dword ptr [ESP] // Pts[4].x
        FWAIT
        NEG      EAX
        PUSH     EAX
        FILD     dword ptr [ESP]
        FMUL
        FISTP    dword ptr [ESP] // Pts[4].y
        FWAIT

        POP      ECX
        POP      EDX
        PUSH     EDX
        PUSH     ECX
        ADD      EDX, [ESP+12]
        ADD      ECX, [ESP+8]
        PUSH     EDX
        PUSH     ECX

        MOV      ESI, ESP
        XOR      EDX, EDX // MinX
        XOR      EDI, EDI // MinY
        XOR      ECX, ECX
        MOV      CL, 3

@@loo1: LODSD
        CMP      EAX, EDI
        JGE      @@1
        XCHG     EDI, EAX
@@1:    LODSD
        CMP      EAX, EDX
        JGE      @@2
        XCHG     EDX, EAX
@@2:    LOOP     @@loo1

        MOV      ESI, [EBP-4]
        MOV      [ESI], ECX
        MOV      [ESI+4], ECX
        MOV      CL, 4
@@loo2:
        POP      EBX
        SUB      EBX, EDI
        CMP      EBX, [ESI+4]
        JLE      @@3
        MOV      [ESI+4], EBX
@@3:
        POP      EAX
        SUB      EAX, EDX
        CMP      EAX, [ESI]
        JLE      @@4
        MOV      [ESI], EAX
@@4:
        LOOP     @@loo2

        MOV      EDI, [EBP-8]
        STOSD
        XCHG     EAX, EBX
        STOSD
        JMP      @@exit

@@1800: DW  1800

@@exit:
        MOV      ESP, EBP
        POP      EBP
        POP      EDI
        POP      ESI
        POP      EBX
end;

procedure TGraphicTool.SetFontOrientation(Value: Integer);
asm
        MOV      byte ptr [GlobalGraphics_UseFontOrient], 1
        MOV      [GlobalCanvas_OnTextArea], offset[TextAreaEx]

        PUSH     EAX
        XCHG     EAX, EDX
        MOV      ECX, 3600
        CDQ
        IDIV     ECX     // EDX = Value mod 3600
        POP      EAX

        MOV      [EAX].fData.Font.Escapement, EDX
        MOV      ECX, EDX
        XOR      EDX, EDX
        MOV      DL, go_FontOrientation
        CALL     SetInt
end;

function TGraphicTool.GetFontStyle: TFontStyle;
asm
       MOV   EDX, dword ptr [EAX].fData.Font.Italic
       AND   EDX, $010101
       MOV   EAX, [EAX].fData.Font.Weight
       CMP   EAX, 700
       SETGE AL       //AL:1 = fsBold
       ADD   EDX, EDX
       OR    EAX, EDX //AL:2 = fsItalic
       SHR   EDX, 7
       OR    EAX, EDX //AL:3 = fsUnderline
       SHR   EDX, 7
       OR    EAX, EDX //AL:4 = fsStrikeOut
end;

procedure TGraphicTool.SetFontStyle(const Value: TFontStyle);
asm
        PUSH     EDI
        MOV      EDI, EAX
        PUSH     EDX
        CALL     GetFontStyle
        POP      EDX
        CMP      AL, DL
        JE       @@exit
        PUSH     EDI

        LEA      EDI, [EDI].fData.Font.Weight
        MOV      ECX, [EDI]
        SHR      EDX, 1
        JNC      @@1
        CMP      ECX, 700
        JGE      @@2
        MOV      ECX, 700
        JMP      @@2
@@1:    CMP      ECX, 700
        JL       @@2
        XOR      ECX, ECX
@@2:    XCHG     EAX, ECX
        STOSD    // change Weight
        SHR      EDX, 1
        SETC     AL
        STOSB    // change Italic
        SHR      EDX, 1
        SETC     AL
        STOSB    // change Underline
        SHR      EDX, 1
        SETC     AL
        STOSB    // change StrikeOut
        POP      EAX
        CALL     Changed
@@exit: POP      EDI
end;

function TGraphicTool.GetHandle: THandle;
const DataSz = sizeof( TGDIToolData );
asm
        PUSH      EBX
@@start:
        XCHG      EBX, EAX
        MOV       ECX, [EBX].fHandle
        JECXZ     @@1

        MOV       EAX, [EBX].fData.Color
        CALL      Color2RGB
        CMP       EAX, [EBX].fColorRGB
        JE        @@1

        MOV       EAX, EBX
        CALL      ReleaseHandle
        PUSH      EAX
        CALL      DeleteObject

@@1:    MOV       ECX, [EBX].fHandle
        INC       ECX
        LOOP      @@exit

        MOV       ECX, [EBX].fParentGDITool
        JECXZ     @@2
        LEA       EDX, [ECX].fData
        LEA       EAX, [EBX].fData
        MOV       ECX, DataSz
        CALL      CompareMem
        TEST      AL, AL
        MOV       EAX, [EBX].fParentGDITool
        JNZ       @@start

@@2:    MOV       EAX, [EBX].fData.Color
        CALL      Color2RGB
        MOV       [EBX].fColorRGB, EAX
        XCHG      EAX, EBX
        CALL      dword ptr [EAX].fMakeHandleProc
        XCHG      ECX, EAX

@@exit: XCHG      EAX, ECX
        POP       EBX
end;

function MakeBrushHandle( Self_: PGraphicTool ): THandle;
asm
        PUSH     EBX
        XCHG     EBX, EAX
        MOV      EAX, [EBX].TGraphicTool.fHandle
        TEST     EAX, EAX
        JNZ      @@exit

        MOV      EAX, [EBX].TGraphicTool.fData.Color
        CALL     Color2RGB   // EAX = ColorRef

        XOR      EDX, EDX

        MOV      ECX, [EBX].TGraphicTool.fData.Brush.Bitmap
        PUSH     ECX
        JECXZ    @@1

        MOV      DL, BS_PATTERN
        JMP      @@2

@@1:
        MOV      CL, [EBX].TGraphicTool.fData.Brush.Style
        MOV      DL, CL
        SUB      CL, 2
        JL       @@2

        XCHG     ECX, [ESP]
        MOV      EAX, [EBX].TGraphicTool.fData.Brush.LineColor
        CALL     Color2RGB
        XOR      EDX, EDX
        MOV      DL, BS_HATCHED

@@2:    PUSH     EAX
        PUSH     EDX

        PUSH     ESP
        CALL     CreateBrushIndirect
        MOV      [EBX].TGraphicTool.fHandle, EAX

        ADD      ESP, 12

@@exit:
        POP      EBX
end;

function MakePenHandle( Self_: PGraphicTool ): THandle;
asm
        PUSH     EBX
        MOV      EBX, EAX

        MOV      EAX, [EBX].TGraphicTool.fHandle
        TEST     EAX, EAX
        JNZ      @@exit

        MOV      EAX, [EBX].TGraphicTool.fData.Color
        CALL     Color2RGB
        PUSH     EAX
        PUSH     EAX
        PUSH     [EBX].TGraphicTool.fData.Pen.Width
        MOVZX    EAX, [EBX].TGraphicTool.fData.Pen.Style
        PUSH     EAX
        PUSH     ESP
        CALL     CreatePenIndirect
        MOV      [EBX].TGraphicTool.fHandle, EAX
        ADD      ESP, 16
@@exit:
        POP      EBX
end;

function MakeGeometricPenHandle( Self_: PGraphicTool ): THandle;
asm
        MOV      ECX, [EAX].TGraphicTool.fHandle
        INC      ECX
        LOOP     @@exit

        PUSH     EBX
        XCHG     EBX, EAX
        MOV      EAX, [EBX].TGraphicTool.fData.Color
        CALL     Color2RGB // EAX = Color2RGB( fColor )
        CDQ                // EDX = lbHatch (0)
        MOV      ECX, [EBX].TGraphicTool.fData.Pen.BrushBitmap
        JECXZ    @@no_brush_bitmap

        XCHG     EDX, ECX // lbHatch = fPenBrushBitmap
        MOV      CL, BS_PATTERN // = 3
        JMP      @@create_pen

@@no_brush_bitmap:
        MOVZX    ECX, [EBX].TGraphicTool.fData.Pen.BrushStyle
        CMP      CL, 1
        JLE      @@create_pen
        MOV      EDX, ECX
        MOV      CL, 2
        SUB      EDX, ECX

@@create_pen:
        PUSH     EDX
        PUSH     EAX
        PUSH     ECX
        MOV      ECX, ESP

        CDQ
        PUSH     EDX
        PUSH     EDX
        PUSH     ECX
        PUSH     [EBX].TGraphicTool.fData.Pen.Width
        MOVZX    ECX, [EBX].TGraphicTool.fData.Pen.Join
        SHL      ECX, 12
        MOVZX    EDX, [EBX].TGraphicTool.fData.Pen.EndCap
        SHL      EDX, 8
        OR       EDX, ECX
        OR       DL, byte ptr [EBX].TGraphicTool.fData.Pen.Style
        OR       EDX, PS_GEOMETRIC
        PUSH     EDX
        CALL     ExtCreatePen

        POP      ECX
        POP      ECX
        POP      ECX

        MOV      [EBX].TGraphicTool.fHandle, EAX
        POP      EBX
        RET
@@exit:
        XCHG     EAX, ECX
end;

function TCanvas.Assign(SrcCanvas: PCanvas): Boolean;
asm
        PUSH     EBX
        PUSH     ESI
        XCHG     EBX, EAX
        MOV      ESI, EDX

        MOV      EAX, [EBX].fFont
        MOV      EDX, [ESI].fFont
        CALL     TGraphicTool.Assign
        MOV      [EBX].fFont, EAX

        MOV      EAX, [EBX].fBrush
        MOV      EDX, [ESI].fBrush
        CALL     TGraphicTool.Assign
        MOV      [EBX].fBrush, EAX

        MOV      EAX, [EBX].fPen
        MOV      EDX, [ESI].fPen
        CALL     TGraphicTool.Assign
        MOV      [EBX].fPen, EAX

        CALL     AssignChangeEvents

        MOV      ECX, [EBX].fFont
        OR       ECX, [EBX].fBrush
        OR       ECX, [EBX].fPen
        SETNZ    AL

        MOV      EDX, [ESI].fPenPos.x
        MOV      ECX, [ESI].fPenPos.y
        CMP      EDX, [EBX].fPenPos.x
        JNE      @@chg_penpos
        CMP      ECX, [EBX].fPenPos.y
        JE       @@1
@@chg_penpos:
        MOV      AL, 1
        MOV      [EBX].fPenPos.x, EDX
        MOV      [EBX].fPenPos.y, ECX
@@1:
        MOV       EDX, [ESI].fCopyMode
        CMP       EDX, [EBX].fCopyMode
        JE        @@2
        MOV       [EBX].fCopyMode, EDX
        MOV       AL, 1
@@2:
        POP       ESI
        POP       EBX
end;

procedure TCanvas.CreateBrush;
asm
        PUSH     EBX
        MOV      EBX, EAX

        MOV      ECX, [EAX].fBrush
        JECXZ    @@chk_owner

        MOV      EAX, ECX
        CALL     TGraphicTool.GetHandle
        PUSH     EAX

        MOV      EAX, EBX
        CALL     AssignChangeEvents

        MOV      EAX, EBX
        CALL     TCanvas.GetHandle
        PUSH     EAX

        CALL     SelectObject

        MOV      EDX, [EBX].TCanvas.fBrush
        CMP      [EDX].TGraphicTool.fData.Brush.Style, bsSolid

        MOV      EAX, [EDX].TGraphicTool.fData.Color
@@0:
        MOV      EBX, [EBX].TCanvas.fHandle
        MOV      ECX, offset[Color2RGB]
        JNZ      @@1

        PUSH     OPAQUE
        PUSH     EBX

        CALL     ECX //Color2RGB
        PUSH     EAX
        PUSH     EBX
        JMP      @@2
@@1:
        PUSH     TRANSPARENT
        PUSH     EBX

        CALL     ECX //Color2RGB
        NOT      EAX
        PUSH     EAX
        PUSH     EBX
@@2:
        CALL     SetBkColor
        CALL     SetBkMode
@@exit:
        POP      EBX
        RET

@@chk_owner:
        MOV      ECX, [EBX].fOwnerControl
        JECXZ    @@exit

        MOV      EAX, [ECX].TControl.fColor
        XOR      ECX, ECX
        JMP      @@0
end;

procedure TCanvas.CreateFont;
asm
        PUSH     EBX
        MOV      EBX, EAX

        MOV      ECX, [EAX].TCanvas.fFont
        JECXZ    @@chk_owner

        MOV      EAX, [ECX].TGraphicTool.fData.Color
        PUSH     ECX
        CALL     Color2RGB
        XCHG     EAX, [ESP]

        CALL     TGraphicTool.GetHandle
        PUSH     EAX

        MOV      EAX, EBX
        CALL     AssignChangeEvents;

        MOV      EAX, EBX
        CALL     TCanvas.GetHandle
        PUSH     EAX
        MOV      EBX, EAX

        CALL     SelectObject

@@set_txcolor:
        PUSH     EBX
        CALL     SetTextColor

@@exit:
        POP      EBX
        RET

@@chk_owner:
        MOV      ECX, [EBX].fOwnerControl
        JECXZ    @@exit

        MOV      EBX, [EBX].fHandle
        MOV      EAX, [ECX].TControl.fTextColor
        CALL     Color2RGB
        PUSH     EAX
        JMP      @@set_txcolor
end;

procedure TCanvas.CreatePen;
asm
        MOV      ECX, [EAX].TCanvas.fPen
        JECXZ    @@exit

        PUSH     EBX
        MOV      EBX, EAX

        MOV      DL, [ECX].TGraphicTool.fData.Pen.Mode
        MOVZX    EDX, DL
        INC      EDX
        PUSH     EDX

        MOV      EAX, ECX
        CALL     TGraphicTool.GetHandle
        PUSH     EAX

        MOV      EAX, EBX
        CALL     AssignChangeEvents

        MOV      EAX, EBX
        CALL     TCanvas.GetHandle
        PUSH     EAX
        MOV      EBX, EAX

        CALL     SelectObject
        PUSH     EBX
        CALL     SetROP2

        POP      EBX
@@exit:
end;

procedure TCanvas.DeselectHandles;
asm
        PUSH     EBX
        PUSH     ESI
        PUSH     EDI
        LEA      EBX, [EAX].TCanvas.fState
        //CALL     TCanvas.GetHandle
        MOV      EAX, [EAX].TCanvas.fHandle
        TEST     EAX, EAX
        JZ       @@exit

        MOVZX    EDX, byte ptr[EBX]
        AND      DL, PenValid or BrushValid or FontValid
        JZ       @@exit

        PUSH     EAX
        LEA      EDI, [Stock]

        MOV      ECX, [EDI]
        INC      ECX
        LOOP     @@1

        MOV      ESI, offset[ GetStockObject ]

        PUSH     BLACK_PEN
        CALL     ESI
        STOSD

        PUSH     HOLLOW_BRUSH
        CALL     ESI
        STOSD

        PUSH     SYSTEM_FONT
        CALL     ESI
        STOSD

@@1:
        LEA      ESI, [Stock]
        POP      EDX

        LODSD
        PUSH     EAX
        PUSH     EDX

        LODSD
        PUSH     EAX
        PUSH     EDX

        LODSD
        PUSH     EAX
        PUSH     EDX

        MOV      ESI, offset[ SelectObject ]
        CALL     ESI
        CALL     ESI
        CALL     ESI

        AND      byte ptr [EBX], not( PenValid or BrushValid or FontValid )
@@exit:
        POP      EDI
        POP      ESI
        POP      EBX
end;

function TCanvas.RequiredState(ReqState: DWORD): HDC; stdcall;
asm
        PUSH     EBX
        PUSH     ESI
        MOV      EBX, ReqState
        MOV      ESI, [EBP+8] //Self
        MOV      EAX, ESI
        TEST     BL, ChangingCanvas
        JZ       @@1
        CALL     Changing
@@1:    AND      BL, 0Fh

        TEST     BL, HandleValid
        JZ       @@2
        CALL     TCanvas.GetHandle
        TEST     EAX, EAX
        JZ       @@ret_0
@@2:
        MOV      AL, [ESI].TCanvas.fState
        NOT      EAX
        AND      BL, AL
        JZ       @@ret_handle

        TEST     BL, FontValid
        JZ       @@3
        MOV      EAX, ESI
        CALL     CreateFont
@@3:    TEST     BL, PenValid
        JZ       @@5
        MOV      EAX, ESI
        CALL     CreatePen
        MOV      ECX, [ESI].TCanvas.fPen
        JCXZ     @@5
        MOV      AL, [ECX].TGraphicTool.fData.Pen.Style
        DEC      AL
        {$IFDEF PARANOIA} DB $2C, 3 {$ELSE} SUB AL, 3 {$ENDIF}
        JB       @@6
@@5:    TEST     BL, BrushValid
        JZ       @@7
@@6:    MOV      EAX, ESI
        CALL     CreateBrush
@@7:    OR       [ESI].TCanvas.fState, BL
@@ret_handle:
        MOV      EAX, [ESI].TCanvas.fHandle
@@ret_0:
        POP      ESI
        POP      EBX
end;

procedure TCanvas.SetHandle(Value: HDC);
asm
        PUSH     EBX
        PUSH     ESI
        MOV      ESI, EDX             // ESI = Value
        MOV      EBX, EAX             // EAX = @ Self
        MOV      ECX, [EBX].fHandle   // ECX = fHandle (before)
        CMP      ECX, ESI             // compare with new Value in EDX
        JZ       @@exit               // equal? -> nothing to do
        JECXZ    @@chk_val            // fHandle = 0? -> check new value in EDX

        PUSH     ECX  // fHandle
          CALL     DeselectHandles
        POP      EDX  // fHandle

        MOV      ECX, [EBX].fOwnerControl
        JECXZ    @@chk_Release
        CMP      [ECX].TControl.fPaintDC, EDX
        JE       @@clr_Handle

@@chk_Release:
        CMP      [EBX].fOnGetHandle.TMethod.Code, offset[TControl.DC2Canvas]
        JNE      @@deldc
        PUSH     EDX  // fHandle
        PUSH     [ECX].TControl.fHandle
        CALL     ReleaseDC
        JMP      @@clr_Handle
@@deldc:
        CMP      WORD PTR [EBX].fIsPaintDC, 0
        JNZ      @@clr_Handle
        PUSH     EDX  // fHandle
        CALL     DeleteDC

@@clr_Handle:
        XOR      ECX, ECX
        MOV      [EBX].TCanvas.fHandle, ECX
        MOV      [EBX].TCanvas.fIsPaintDC, CL
        AND      [EBX].TCanvas.fState, not HandleValid

@@chk_val:
        TEST     ESI, ESI
        JZ       @@exit

        OR       [EBX].TCanvas.fState, HandleValid
        MOV      [EBX].TCanvas.fHandle, ESI
        LEA      EDX, [EBX].TCanvas.fPenPos
        MOV      EAX, EBX
        CALL     SetPenPos

@@exit: POP      ESI
        POP      EBX
end;

procedure TCanvas.SetPenPos(const Value: TPoint);
asm
          MOV     ECX, [EDX].TPoint.y
          MOV     EDX, [EDX].TPoint.x
          MOV     [EAX].fPenPos.x, EDX
          MOV     [EAX].fPenPos.y, ECX
          CALL    MoveTo
end;

procedure TCanvas.Changing;
asm
        PUSHAD
        MOV      ECX, [EAX].fOnChangeCanvas.TMethod.Code
        JECXZ    @@exit
        XCHG     EDX, EAX
        MOV      EAX, [EDX].fOnChangeCanvas.TMethod.Data
        CALL     ECX
@@exit:
        POPAD
end;

procedure TCanvas.Arc(X1, Y1, X2, Y2, X3, Y3, X4, Y4: Integer); stdcall;
asm
        PUSH     ESI

        PUSH     HandleValid or PenValid or ChangingCanvas
        PUSH     dword ptr [EBP+8]
        CALL     RequiredState

        MOV      EDX, EAX

        LEA      ESI, [Y4]
        STD

        XOR      ECX, ECX
        MOV      CL, 8
@@1:
        LODSD
        PUSH     EAX

        LOOP     @@1

        CLD
        PUSH     EDX  //Canvas.fHandle
        CALL     Windows.Arc
        POP      ESI
end;

procedure TCanvas.Chord(X1, Y1, X2, Y2, X3, Y3, X4, Y4: Integer); stdcall;
asm
        PUSH     HandleValid or PenValid or BrushValid or ChangingCanvas
        PUSH     dword ptr [EBP + 8]
        CALL     RequiredState

        MOV      EDX, EAX

        PUSH     ESI
        LEA      ESI, [Y4]
        STD

        XOR      ECX, ECX
        MOV      CL, 8
@@1:
        LODSD
        PUSH     EAX

        LOOP     @@1

        CLD
        PUSH     EDX  //Canvas.fHandle
        CALL     Chord
        POP      ESI
end;

procedure TCanvas.CopyRect(const DstRect: TRect; SrcCanvas: PCanvas;
  const SrcRect: TRect);
asm
        PUSH     ESI
        PUSH     EDI

        PUSH     [EAX].fCopyMode

        PUSH     EDX

          PUSH     HandleValid or BrushValid
          PUSH     ECX

          PUSH     HandleValid or FontValid or BrushValid or ChangingCanvas
          PUSH     EAX
          MOV      ESI, offset[ RequiredState ]
          CALL     ESI
          MOV      EDI, EAX     // EDI = @Self.fHandle

          CALL     ESI
          MOV      EDX, EAX     // EDX = SrcCanvas.fHandle

        POP      ECX          // ECX = @DstRect

        MOV      ESI, [SrcRect]

        MOV      EAX, [ESI].TRect.Bottom
        SUB      EAX, [ESI].TRect.Top
        PUSH     EAX

        MOV      EAX, [ESI].TRect.Right
        SUB      EAX, [ESI].TRect.Left
        PUSH     EAX

        PUSH     [ESI].TRect.Top

        LODSD
        PUSH     EAX

        PUSH     EDX

        MOV      EAX, [ECX].TRect.Bottom
        MOV      EDX, [ECX].TRect.Top
        SUB      EAX, EDX
        PUSH     EAX

        MOV      EAX, [ECX].TRect.Right
        MOV      ESI, [ECX].TRect.Left
        SUB      EAX, ESI
        PUSH     EAX

        PUSH     EDX

        PUSH     ESI

        PUSH     EDI

        CALL     StretchBlt

        POP      EDI
        POP      ESI
end;

procedure TCanvas.DrawFocusRect({$IFNDEF FPC}const{$ENDIF} Rect: TRect);
asm
        PUSH     EDX

        PUSH     HandleValid or BrushValid or FontValid or ChangingCanvas
        PUSH     EAX
        CALL     RequiredState

        PUSH     EAX
        CALL     Windows.DrawFocusRect
end;

procedure TCanvas.Ellipse(X1, Y1, X2, Y2: Integer);
asm
        PUSH     [Y2]
        PUSH     [X2]
        PUSH     ECX
        PUSH     EDX

        PUSH     HandleValid or PenValid or BrushValid or ChangingCanvas
        PUSH     EAX
        CALL     RequiredState

        PUSH     EAX
        CALL     Windows.Ellipse
end;

procedure TCanvas.FillRect({$IFNDEF FPC}const{$ENDIF} Rect: TRect);
asm
        PUSH     EBX
        XCHG     EBX, EAX
        PUSH     EDX
        PUSH     HandleValid or BrushValid or ChangingCanvas
        PUSH     EBX
        CALL     RequiredState
        MOV      ECX, [EBX].fBrush
        JECXZ    @@chk_ctl

@@fill_with_Brush:
        XCHG     EAX, ECX
        CALL     TGraphicTool.GetHandle
        POP      EDX
        PUSH     EAX
        JMP      @@fin
@@chk_ctl:
        MOV      ECX, [EBX].fOwnerControl
        JECXZ    @@dflt_fill
        XCHG     EAX, ECX
        MOV      ECX, [EAX].TControl.fBrush
        INC      ECX
        LOOP     @@fill_with_Brush
        MOV      EAX, [EAX].TControl.fColor
        CALL     Color2RGB
        PUSH     EAX
        CALL     CreateSolidBrush
        POP      EDX
        PUSH     EAX
        PUSH     EAX
        PUSH     EDX
        PUSH     [EBX].fHandle
        CALL     Windows.FillRect
        CALL     DeleteObject
        POP      EBX
        RET
@@dflt_fill:
        POP      EDX
        PUSH     COLOR_WINDOW + 1
@@fin:
        PUSH     EDX
        PUSH     [EBX].fHandle
        CALL     Windows.FillRect
        POP      EBX
end;

procedure TCanvas.FillRgn(const Rgn: HRgn);
asm
        PUSH     EBX
        XCHG     EBX, EAX
        PUSH     EDX

        PUSH     HandleValid or BrushValid or ChangingCanvas
        PUSH     EBX
        CALL     RequiredState

        MOV      ECX, [EBX].TCanvas.fBrush
        JECXZ    @@1

@@fill_rgn_using_Brush:
        XCHG     EAX, ECX
        CALL     TGraphicTool.GetHandle
        POP      EDX
        PUSH     EAX
        PUSH     EDX
        PUSH     [EBX].fHandle
        CALL     Windows.FillRgn
        JMP      @@fin

@@1:    MOV      ECX, [EBX].TCanvas.fOwnerControl
        MOV      EAX, -1 // clWhite
        JECXZ    @@2

        XCHG     EAX, ECX
        MOV      ECX, [EAX].TControl.fBrush
        INC      ECX
        LOOP     @@fill_rgn_using_Brush

        MOV      EAX, [EAX].TControl.fColor
@@2:
        CALL     Color2RGB
        PUSH     EAX
        CALL     CreateSolidBrush // EAX = Br

        POP      EDX // Rgn

        PUSH     EAX //-------------------//
        PUSH     EAX           // Br
        PUSH     EDX           // Rgn
        PUSH     [EBX].FHandle // fHandle
        CALL     Windows.FillRgn

        CALL     DeleteObject

@@fin:
        POP      EBX
end;

procedure TCanvas.FloodFill(X, Y: Integer; Color: TColor;
  FillStyle: TFillStyle);
asm
        PUSH     EBX
        MOV      EBX, EAX

        MOVZX    EAX, [FillStyle]
        TEST     EAX, EAX
        MOV      EAX, FLOODFILLSURFACE   // = 1
        JZ       @@1
        //MOV      EAX, FLOODFILLBORDER  // = 0
        DEC      EAX
@@1:
        PUSH     EAX
        PUSH     [Color]
        PUSH     ECX
        PUSH     EDX

        PUSH     HandleValid or BrushValid or ChangingCanvas
        PUSH     EBX
        CALL     RequiredState
        PUSH     EAX
        CALL     Windows.ExtFloodFill

        POP      EBX
end;

procedure TCanvas.FrameRect({$IFNDEF FPC}const{$ENDIF} Rect: TRect);
asm
        PUSH     EBX
        XCHG     EBX, EAX
        PUSH     EDX

        MOV      ECX, [EBX].TCanvas.fBrush
        JECXZ    @@1

        PUSH     [ECX].TGraphicTool.fData.Color
        JMP      @@cr_br

@@1:    MOV      ECX, [EBX].TCanvas.fOwnerControl
        JECXZ    @@2

        PUSH     [ECX].TControl.fColor
        JMP      @@cr_br

@@2:    PUSH     clWhite
@@cr_br:POP      EAX                  // @Rect
        CALL     Color2RGB
        PUSH     EAX
        CALL     CreateSolidBrush
        POP      EDX
          PUSH     EAX
        PUSH     EAX
        PUSH     EDX

        PUSH     HandleValid or ChangingCanvas
        PUSH     EBX
        CALL     RequiredState

        PUSH     EAX
        CALL     Windows.FrameRect

        CALL     DeleteObject

        POP      EBX
end;

procedure TCanvas.LineTo(X, Y: Integer);
asm
        PUSH     ECX
        PUSH     EDX
        PUSH     HandleValid or PenValid or BrushValid or ChangingCanvas
        PUSH     EAX
        CALL     RequiredState
        PUSH     EAX  //Canvas.fHandle
        CALL     Windows.LineTo
end;

procedure TCanvas.MoveTo(X, Y: Integer);
asm
        PUSH     0
        PUSH     ECX
        PUSH     EDX
        PUSH     HandleValid
        PUSH     EAX
        CALL     RequiredState
        PUSH     EAX  //Canvas.fHandle
        CALL     Windows.MoveToEx
end;

procedure TCanvas.Pie(X1, Y1, X2, Y2, X3, Y3, X4, Y4: Integer); stdcall;
asm
        PUSH     HandleValid or PenValid or BrushValid or ChangingCanvas
        PUSH     dword ptr [EBP + 8]
        CALL     RequiredState

        MOV      EDX, EAX

        PUSH     ESI
        LEA      ESI, [Y4]
        STD

        XOR      ECX, ECX
        MOV      CL, 8
@@1:
        LODSD
        PUSH     EAX

        LOOP     @@1

        CLD
        PUSH     EDX  //Canvas.fHandle
        CALL     Windows.Pie
        POP      ESI
end;

procedure TCanvas.Polygon(const Points: array of TPoint);
asm
        INC      ECX
        PUSH     ECX
        PUSH     EDX

        PUSH     HandleValid or PenValid or BrushValid or ChangingCanvas
        PUSH     EAX
        CALL     RequiredState

        PUSH     EAX
        CALL     Windows.Polygon
end;

procedure TCanvas.Polyline(const Points: array of TPoint);
asm
        INC      ECX
        PUSH     ECX
        PUSH     EDX

        PUSH     HandleValid or PenValid or BrushValid or ChangingCanvas
        PUSH     EAX
        CALL     RequiredState

        PUSH     EAX
        CALL     Windows.Polyline
end;

procedure TCanvas.Rectangle(X1, Y1, X2, Y2: Integer);
asm
        PUSH     [Y2]
        PUSH     [X2]
        PUSH     ECX
        PUSH     EDX

        PUSH     HandleValid or BrushValid or PenValid or ChangingCanvas
        PUSH     EAX
        CALL     RequiredState

        PUSH     EAX
        CALL     Windows.Rectangle
end;

procedure TCanvas.RoundRect(X1, Y1, X2, Y2, X3, Y3: Integer);
asm
        PUSH     [Y3]
        PUSH     [X3]
        PUSH     [Y2]
        PUSH     [X2]
        PUSH     ECX
        PUSH     EDX

        PUSH     HandleValid or BrushValid or PenValid or ChangingCanvas
        PUSH     EAX
        CALL     RequiredState

        PUSH     EAX
        CALL     Windows.RoundRect
end;

procedure TCanvas.TextArea(const Text: KOLString; var Sz: TSize;
  var P0: TPoint);
asm
        PUSH     EBX
        MOV      EBX, EAX

        PUSH     ECX
        CALL     TextExtent
        POP      EDX

        MOV      ECX, [P0]
        XOR      EAX, EAX
        MOV      [ECX].TPoint.x, EAX
        MOV      [ECX].TPoint.y, EAX

        CMP      [GlobalCanvas_OnTextArea], EAX
        JZ       @@exit
        MOV      EAX, EBX
        CALL     [GlobalCanvas_OnTextArea]

@@exit:
        POP      EBX
end;

procedure TCanvas.TextRect(const Rect: TRect; X, Y: Integer; const Text: Ansistring);
asm
        PUSH     EBX
        XCHG     EBX, EAX

        PUSH     0              // prepare 0

        PUSH     EDX
        PUSH     ECX

        MOV      EAX, [Text]
        PUSH     EAX
        CALL     System.@LStrLen

        POP      ECX            // ECX = @Text[1]
        POP      EDX            // EDX = X
        XCHG     EAX, [ESP]     // prepare Length(Text), EAX = @Rect
        PUSH     ECX            // prepare PChar(Text)
        PUSH     EAX            // prepare @Rect

        XOR      EAX, EAX
        MOV      AL, ETO_CLIPPED // = 4
        MOV      ECX, [EBX].fBrush
        JECXZ    @@opaque

        CMP      [ECX].TGraphicTool.fData.Brush.Style, bsClear
        JZ       @@txtout

@@opaque:
        DB $0C, ETO_OPAQUE //OR       AL, ETO_OPAQUE
@@txtout:
        PUSH     EAX            // prepare Options
        PUSH     [Y]            // prepare Y
        PUSH     EDX            // prepare X

        PUSH     HandleValid or FontValid or BrushValid or ChangingCanvas
        PUSH     EBX
        CALL     RequiredState  // EAX = fHandle
        PUSH     EAX            // prepare fHandle

        CALL     Windows.ExtTextOutA  // KOL_ANSI

        POP      EBX
end;

procedure TCanvas.DrawText(Text: AnsiString; var Rect:TRect; Flags:DWord);
asm
          PUSH  [Flags]
          PUSH  ECX
          PUSH  -1
          CALL  EDX2PChar
          PUSH  EDX

          PUSH  HandleValid or FontValid or BrushValid or ChangingCanvas
          PUSH  EAX
          CALL  RequiredState
          PUSH  EAX
          CALL  Windows.DrawTextA
end;

function TCanvas.GetBrush: PGraphicTool;
asm
        MOV      ECX, [EAX].fBrush
        INC      ECX
        LOOP     @@exit

        PUSH     EAX
        CALL     NewBrush
        POP      EDX
        PUSH     EAX

        MOV      [EDX].fBrush, EAX

        MOV      [EAX].TGraphicTool.fOnGTChange.TMethod.Code, Offset[TCanvas.ObjectChanged]
        MOV      [EAX].TGraphicTool.fOnGTChange.TMethod.Data, EDX
        MOV      ECX, [EDX].fOwnerControl
        JECXZ    @@1

        PUSH     [ECX].TControl.fBrush
        MOV      ECX, [ECX].TControl.fColor
        MOV      [EAX].TGraphicTool.fData.Color, ECX
        POP      EDX
        TEST     EDX, EDX
        JZ       @@1

        CALL     TGraphicTool.Assign

@@1:    POP      ECX

@@exit: XCHG     EAX, ECX
end;

function TCanvas.GetFont: PGraphicTool;
asm
        MOV      ECX, [EAX].TCanvas.fFont
        INC      ECX
        LOOP     @@exit

        PUSH     EAX
        CALL     NewFont
        POP      EDX
        PUSH     EAX

        MOV      [EDX].TCanvas.fFont, EAX
        MOV      [EAX].TGraphicTool.fOnGTChange.TMethod.Code, Offset[TCanvas.ObjectChanged]
        MOV      [EAX].TGraphicTool.fOnGTChange.TMethod.Data, EDX

        MOV      ECX, [EDX].fOwnerControl
        JECXZ    @@1

        PUSH     [ECX].TControl.fFont
        MOV      ECX, [ECX].TControl.fTextColor
        MOV      [EAX].TGraphicTool.fData.Color, ECX
        POP      EDX
        TEST     EDX, EDX
        JZ       @@1

        CALL     TGraphicTool.Assign

@@1:    POP      ECX

@@exit: MOV      EAX, ECX
end;

function TCanvas.GetPen: PGraphicTool;
asm
        MOV      ECX, [EAX].TCanvas.fPen
        INC      ECX
        LOOP     @@exit

        PUSH     EAX
        CALL     NewPen
        POP      EDX
        MOV      [EDX].fPen, EAX
        PUSH     EAX
        MOV      EAX, EDX
        CALL     AssignChangeEvents
        POP      ECX

@@exit: MOV      EAX, ECX
end;

function TCanvas.GetHandle: HDC;
asm
        CMP      word ptr[EAX].fOnGetHandle.TMethod.Code+2, 0
        MOV      EDX, EAX
        MOV      EAX, [EDX].fHandle
        JZ       @@exit
        MOV      EAX, [EDX].fOnGetHandle.TMethod.Data
        PUSH     EDX
        CALL     [EDX].fOnGetHandle.TMethod.Code
        XCHG     EAX, [ESP]
        POP      EDX
        PUSH     EDX
        CALL     SetHandle
        POP      EAX
@@exit:
end;

procedure TCanvas.AssignChangeEvents;
asm
        PUSH     ESI
        LEA      ESI, [EAX].fBrush
        MOV      CL, 3
        MOV      EDX, EAX
@@1:    LODSD
        TEST     EAX, EAX
        JZ       @@nxt
        MOV      [EAX].TGraphicTool.fOnGTChange.TMethod.Data, EDX
        MOV      [EAX].TGraphicTool.fOnGTChange.TMethod.Code, offset[ ObjectChanged ]
@@nxt:  DEC      CL
        JNZ      @@1
        POP      ESI
end;

function Mul64i( const X: I64; Mul: Integer ): I64;
asm     //cmd    //opd
        TEST     EDX, EDX
        PUSHFD
        JGE      @@1
        NEG      EDX
@@1:    PUSH     ECX
        CALL     Mul64EDX
        POP      EAX
        POPFD
        JGE      @@2
        MOV      EDX, EAX
        CALL     Neg64
@@2:
end;

function Div64i( const X: I64; D: Integer ): I64;
asm     //cmd    //opd
        PUSH     EBX
        XOR      EBX, EBX
        PUSH     ESI
        XCHG     ESI, EAX
        LODSD
        MOV      [ECX], EAX
        LODSD
        MOV      [ECX+4], EAX
        MOV      ESI, ECX
        PUSH     EDX
        XCHG     EAX, ECX
        CALL     Sgn64
        TEST     EAX, EAX
        JGE      @@1
        INC      EBX
        MOV      EAX, ESI
        MOV      EDX, ESI
        CALL     Neg64
@@1:    POP      EDX
        TEST     EDX, EDX
        JGE      @@2
        XOR      EBX, 1
        NEG      EDX
@@2:    MOV      EAX, ESI
        MOV      ECX, ESI
        CALL     Div64EDX
        DEC      EBX
        JNZ      @@3
        MOV      EDX, ESI
        XCHG     EAX, ESI
        CALL     Neg64
@@3:    POP      ESI
        POP      EBX
end;

function cHex2Int( const Value : KOLString) : Integer;
asm
     TEST  EAX, EAX
     JZ    @@exit
     CMP   word ptr [EAX], '0x'
     JZ    @@skip_2_chars
     CMP   word ptr [EAX], '0X'
     JNZ   @@2Hex2Int
@@skip_2_chars:
     INC   EAX
     INC   EAX
@@2Hex2Int:
     JMP   Hex2Int
@@exit:
end;

function Trim( const S : KOLString): KOLString;
asm
        PUSH     EDX
        CALL     TrimRight
        POP      EDX
        MOV      EAX, [EDX]
        CALL     TrimLeft
end;

function LowerCase(const S: Ansistring): Ansistring;
asm
        PUSH     ESI
        XCHG     EAX, EDX
        PUSH     EAX
        CALL     System.@LStrAsg
        POP      EAX

        CALL     UniqueString

        PUSH     EAX
        CALL     System.@LStrLen
        POP      ESI

        XCHG     ECX, EAX

        JECXZ    @@exit

@@go:
        LODSB
        {$IFDEF PARANOIA} DB $2C, 'A' {$ELSE} SUB AL, 'A' {$ENDIF}
        {$IFDEF PARANOIA} DB $3C, 26 {$ELSE} CMP AL, 'Z'-'A'+1 {$ENDIF}
        JNB      @@1

        ADD      byte ptr [ESI - 1], 20h
@@1:
        LOOP     @@go
@@exit:
        POP      ESI
end;

function UpperCase(const S: Ansistring): Ansistring;
asm
        PUSH     ESI
        XCHG     EAX, EDX
        PUSH     EAX
        CALL     System.@LStrAsg
        POP      EAX

        CALL     UniqueString

        PUSH     EAX
        CALL     System.@LStrLen
        POP      ESI

        XCHG     ECX, EAX

        JECXZ    @@exit

@@go:
        LODSB
        {$IFDEF PARANOIA} DB $2C, 'a' {$ELSE} SUB AL, 'a' {$ENDIF}
        {$IFDEF PARANOIA} DB $3C, $1A {$ELSE} CMP AL, 'z'-'a'+1 {$ENDIF}
        JNB      @@1

        SUB      byte ptr [ESI - 1], 20h
@@1:
        LOOP     @@go
@@exit:
        POP      ESI
end;

function AllocMem( Size : Integer ) : Pointer;
asm     //cmd    //opd
        TEST     EAX, EAX
        JZ       @@exit
        PUSH     EAX
        CALL     System.@GetMem
        POP      EDX
        PUSH     EAX
        //MOV      CL, 0
        CALL     ZeroMemory
        POP      EAX
@@exit:
end;

function _WStrComp(S1, S2: PWideChar): Integer;
asm
    PUSH  ESI
    XCHG  ESI, EAX
    XOR   EAX, EAX
@@1:
    LODSW
    MOV   ECX, EAX
    SUB   AX, word ptr [EDX]
    JNZ   @@exit
    JECXZ @@exit
    INC   EDX
    INC   EDX
    JMP   @@1
@@exit:
    MOVSX EAX, AX
    POP   ESI
end;

function _AnsiCompareStrA_Fast2(S1, S2: PAnsiChar): Integer;
asm
        CALL     EAX2PChar
        CALL     EDX2PChar
        PUSH     ESI
        XCHG     ESI, EAX
        XOR      EAX, EAX
@@1:
        LODSB
        MOV      CX, word ptr [EAX*2 + SortAnsiOrder]
        MOV      AL, [EDX]
        SUB      CX, word ptr [EAX*2 + SortAnsiOrder]
        JNZ      @@retCL
        INC      EDX
        TEST     AL, AL
        JNZ      @@1
@@retCL:
        MOVSX    EAX, CX
        POP      ESI
end;

function _AnsiCompareStrNoCaseA_Fast2(S1, S2: PAnsiChar): Integer;
asm
        CALL     EAX2PChar
        CALL     EDX2PChar
        PUSH     ESI
        XCHG     ESI, EAX
        XOR      EAX, EAX
@@1:
        LODSB
        MOV      CX, word ptr [EAX*2 + SortAnsiOrderNoCase]
        MOV      AL, [EDX]
        SUB      CX, word ptr [EAX*2 + SortAnsiOrderNoCase]
        JNZ      @@retCL
        INC      EDX
        TEST     AL, AL
        JNZ      @@1
@@retCL:
        MOVSX    EAX, CX
        POP      ESI
end;

function StrPCopy(Dest: PAnsiChar; const Source: Ansistring): PAnsiChar;
asm
        PUSH     EAX
        MOV      EAX, EDX
        CALL     System.@LStrLen
        MOV      ECX, EAX
        POP      EAX
        CALL     EDX2PChar
        CALL     StrLCopy
end;

function StrEq( const S1, S2 : AnsiString ) : Boolean;
asm
        TEST     EDX, EDX
        JNZ      @@1
@@0:    CMP      EAX, EDX
        JMP      @@exit
@@1:    TEST     EAX, EAX
        JZ       @@0
        MOV      ECX, [EAX-4]
        CMP      ECX, [EDX-4]
        JNE      @@exit
        PUSH     EAX
        PUSH     EDX
        PUSH     0
        MOV      EDX, ESP
        CALL     LowerCase
        PUSH     0
        MOV      EAX, [ESP + 8]
        MOV      EDX, ESP
        CALL     LowerCase
        POP      EAX
        POP      EDX
        PUSH     EDX
        PUSH     EAX
        CALL     System.@LStrCmp
        MOV      EAX, ESP
        PUSHFD
        XOR      EDX, EDX
        MOV      DL, 2
        CALL     System.@LStrArrayClr
        POPFD
        POP      EDX
        POP      EDX
        POP      EDX
        POP      EDX
@@exit:
        SETZ     AL
end;

function AnsiEq( const S1, S2 : KOLString ) : Boolean;
asm
        CALL     AnsiCompareStrNoCase
        TEST     EAX, EAX
        SETZ     AL
end;

function StrIn(const S: AnsiString; const A: array of AnsiString): Boolean;
asm
@@1:
        TEST     ECX, ECX
        JL       @@ret_0

        PUSH     EDX
        MOV      EDX, [EDX+ECX*4]
        DEC      ECX

        PUSH     ECX
        PUSH     EAX
        CALL     StrEq
        DEC      AL
        POP      EAX
        POP      ECX

        POP      EDX
        JNZ      @@1

        MOV      AL, 1
        RET

@@ret_0:XOR      EAX, EAX
end;

{$IFDEF ASM_no}
procedure NormalizeUnixText( var S: AnsiString );
asm     //cmd    //opd
        CMP      dword ptr [EAX], 0
        JZ       @@exit
        PUSH     EBX
        PUSH     EDI
        MOV      EBX, EAX
        CALL     UniqueString
        MOV      EDI, [EBX]
@@1:    MOV      EAX, EDI
        CALL     System.@LStrLen
        XCHG     ECX, EAX
        MOV      AX, $0D0A

        CMP      byte ptr [EDI], AL
        JNE      @@loo
        MOV      byte ptr [EDI], AH
@@loo:
        TEST     ECX, ECX
        JZ       @@fin
@@loo1:
        REPNZ SCASB
        JNZ      @@fin
        CMP      byte ptr [EDI-2], AH
        JE       @@loo
        MOV      byte ptr [EDI-1], AH
        JNE      @@loo1
@@fin:  POP      EDI
        POP      EBX
@@exit:
end;
{$ENDIF}

function FileCreate( const FileName: KOLString; OpenFlags: DWord): THandle;
asm
        XOR      ECX, ECX
        PUSH     ECX
        MOV      ECX, EDX
        SHR      ECX, 16
        AND      CX, $1FFF
        JNZ      @@1
        MOV      CL, FILE_ATTRIBUTE_NORMAL
@@1:    PUSH     ECX
        MOV      CL, DH
        PUSH     ECX                  // CreationMode
        PUSH     0
        MOV      CL, DL
        PUSH     ECX                  // ShareMode
        MOV      DX, 0
        PUSH     EDX                  // AccessMode
        //CALL     System.@LStrToPChar // FileName must not be ''
        PUSH     EAX
        CALL     CreateFile
end;

function FileClose( Handle: THandle): Boolean;
asm
        PUSH     EAX
        CALL     CloseHandle
        TEST     EAX, EAX
        SETNZ    AL
end;

function FileRead( Handle: THandle; var Buffer; Count: DWord): DWord;
asm
        PUSH     EBP
        PUSH     0
        MOV      EBP, ESP
        PUSH     0
        PUSH     EBP
        PUSH     ECX
        PUSH     EDX
        PUSH     EAX
        CALL     ReadFile
        TEST     EAX, EAX
        POP      EAX
        JNZ      @@exit
        XOR      EAX, EAX
@@exit:
        POP      EBP
end;

function File2Str( Handle: THandle): AnsiString;
asm
        PUSH     EDX
        TEST     EAX, EAX
        JZ       @@exit // return ''

        PUSH     EBX
        MOV      EBX, EAX // EBX = Handle
        XOR      EDX, EDX
        XOR      ECX, ECX
        INC      ECX
        CALL     FileSeek
        PUSH     EAX // Pos
        PUSH     0
        PUSH     EBX
        CALL     GetFileSize
        POP      EDX
        SUB      EAX, EDX // EAX = Size - Pos
        JZ       @@exitEBX

        PUSH     EAX
        CALL     System.@GetMem
        XCHG     EAX, EBX
        MOV      EDX, EBX
        POP      ECX
        PUSH     ECX
        CALL     FileRead
        POP      ECX
        MOV      EDX, EBX
        POP      EBX
        POP      EAX
        PUSH     EDX
        {$IFDEF _D2}
        CALL     _LStrFromPCharLen
        {$ELSE}
          {$IFDEF _D2009orHigher}
          PUSH     ECX // TODO: check to remove
          XOR      ECX, ECX
          {$ENDIF}
        CALL     System.@LStrFromPCharLen
          {$IFDEF _D2009orHigher}
          POP      ECX
          {$ENDIF}

        {$ENDIF}
        JMP      @@freebuf

@@exitEBX:
        POP      EBX
@@exit:
        XCHG     EDX, EAX
        POP      EAX // @Result
        PUSH     EDX
        {$IFDEF _D2009orHigher}
        XOR      ECX, ECX // TODO: confirm not need push
        {$ENDIF}
        CALL     System.@LStrFromPChar
@@freebuf:
        POP      EAX
        TEST     EAX, EAX
        JZ       @@fin
        CALL     System.@FreeMem
@@fin:
end;

function FileWrite( Handle: THandle; const Buffer; Count: DWord): DWord;
asm
        PUSH     EBP
        PUSH     EBP
        MOV      EBP, ESP
        PUSH     0
        PUSH     EBP
        PUSH     ECX
        PUSH     EDX
        PUSH     EAX
        CALL     WriteFile
        TEST     EAX, EAX
        POP      EAX
        JNZ      @@exit
        XOR      EAX, EAX
@@exit:
        POP      EBP
end;

function FileEOF( Handle: THandle ) : Boolean;
asm
        PUSH     EAX

        PUSH     0
        PUSH     EAX
        CALL     GetFileSize

        XCHG     EAX, [ESP]

        MOV      CL, spCurrent
        XOR      EDX, EDX
        CALL     FileSeek

        POP      EDX
        CMP      EAX, EDX
        SETGE    AL
end;

procedure FileTime( const Path: KOLString;
  CreateTime, LastAccessTime, LastModifyTime: PFileTime ); stdcall;
const Size_TFindFileData = (sizeof(TFindFileData) + 3) and not 3;
asm
          PUSH  ESI
          PUSH  EDI
          SUB   ESP, Size_TFindFileData
          MOV   EDX, ESP
          MOV   EAX, [Path]
          CALL  Find_First
          TEST  AL, AL
          JZ    @@exit
          MOV   EAX, ESP
          CALL  Find_Close
          XOR   ECX, ECX
          MOV   CL, 3
@@loop:   LEA   ESI, [ESP+ECX*8-8].TFindFileData.ftCreationTime
          MOV   EDI, [ECX*4+EBP+8]
          TEST  EDI, EDI
          JZ    @@e_loop
          MOVSD
          MOVSD
@@e_loop: LOOP  @@loop
@@exit:   ADD   ESP, Size_TFindFileData
          POP   EDI
          POP   ESI
end;

function CompareSystemTime( const D1, D2 : TSystemTime) : Integer; assembler;
asm
        PUSH     ESI
        PUSH     EBX
        MOV      ESI, EAX
        XOR      EAX, EAX
        XOR      ECX, ECX
        MOV      CL, 8  // 8 words: wYear, wMonth,..., wMilliseconds
@@loo:
        LODSW
        MOV      BX, [EDX]
        INC      EDX
        INC      EDX

        CMP      CL, 6
        JE       @@cont  // skip compare DayOfWeek

        SUB      AX, BX
        JNE      @@calc

@@cont:
        LOOP     @@loo
        JMP      @@exit

@@calc:
        SBB      EAX, EAX
        {$IFDEF PARANOIA} DB $0C, 1 {$ELSE} OR AL, 1 {$ENDIF}

@@exit:
        POP      EBX
        POP      ESI
end;

function DirectoryExists( const Name: KOLString): Boolean;
asm
   PUSH EBX
        //CALL     System.@LStrToPChar // Name must not be ''
        PUSH     EAX
   PUSH SEM_NOOPENFILEERRORBOX or SEM_FAILCRITICALERRORS
   CALL SetErrorMode
   XCHG EBX, EAX
        CALL     GetFileAttributes
        INC      EAX
        JZ       @@exit
        DEC      EAX
        {$IFDEF PARANOIA} DB $24, FILE_ATTRIBUTE_DIRECTORY {$ELSE} AND AL, FILE_ATTRIBUTE_DIRECTORY {$ENDIF}
        SETNZ    AL
@@exit:
   XCHG EAX, EBX
   PUSH EAX
   CALL SetErrorMode
   XCHG EAX, EBX
   POP  EBX
end;

procedure TDirList.Clear;
asm
        LEA      EDX, [EAX].FListPositions
        CALL     @@clear
        ADD      EDX, 4 // fStoreFiles -- order of fields is important!!!
@@clear:
        PUSHAD
        XOR      EAX, EAX
        XCHG     EAX, dword ptr [EDX]
        CALL     TObj.RefDec
        POPAD
@@exit:
end;

destructor TDirList.Destroy;
asm
        PUSH     EBX
        MOV      EBX, EAX
        CALL     Clear
        LEA      EAX, [EBX].FPath
        {$IFDEF UNICODE_CTRLS}
        CALL     System.@WStrClr
        {$ELSE}
        CALL     System.@LStrClr
        {$ENDIF}
        XCHG     EAX, EBX
        CALL     TObj.Destroy
        POP      EBX
end;

function TDirList.GetCount: Integer;
asm
        {CMP      EAX, 0
        JNZ      @@1
        NOP
@@1:    }
        MOV      ECX, [EAX].FListPositions
        JECXZ    @@retECX
        MOV      ECX, [ECX].TList.fCount
@@retECX:
        XCHG     EAX, ECX
end;

procedure SwapDirItems( Data : PSortDirData; const e1, e2 : DWORD );
asm
        MOV      EAX, [EAX].TSortDirData.Dir
        MOV      EAX, [EAX].TDirList.FListPositions
        {$IFDEF  xxSPEED_FASTER} //|||||||||||||||||||||||||||||||||||||||||||||
        MOV      EAX, [EAX].TList.fItems
        LEA      EDX, [EAX+EDX*4]
        LEA      ECX, [EAX+ECX*4]
        MOV      EAX, [EDX]
        XCHG     EAX, [ECX]
        MOV      [EDX], EAX
        {$ELSE}
        CALL     TList.Swap
        {$ENDIF}
end;

destructor TThread.Destroy;
asm
        PUSH     EBX
        MOV      EBX, EAX
      CALL RefInc
        MOV      EAX, EBX
        CMP      [EBX].FTerminated, 0
        JNZ      @@1
        CALL     Terminate
        MOV      EAX, EBX
        CALL     WaitFor
@@1:    MOV      ECX, [EBX].FHandle
        JECXZ    @@2
        PUSH     ECX
        CALL     CloseHandle
@@2:    POP      EAX
        XCHG     EBX, EAX
        JMP      TObj.Destroy
end;

destructor TStream.Destroy;
asm
        PUSH     EAX
        PUSH     [EAX].fData.fThread
        CALL     [EAX].fMethods.fClose
        POP      EAX
        CALL     TObj.RefDec
        POP      EAX
        CALL     TObj.Destroy
end;

procedure CloseMemStream( Strm: PStream );
asm
        XOR      ECX, ECX
        XCHG     ECX, [EAX].TStream.fMemory
        JECXZ    @@exit
        XCHG     EAX, ECX
        CALL     System.@FreeMem
@@exit:
end;

function NewReadFileStream( const FileName: KOLString ): PStream;
asm
        PUSH     EBX
        XCHG     EBX, EAX
        MOV      EAX, offset[BaseFileMethods]
        CALL     _NewStream
        MOV      EDX, [ReadFileStreamProc]
        MOV      [EAX].TStream.fMethods.fRead, EDX
        XCHG     EBX, EAX
        MOV      EDX, ofOpenRead or ofOpenExisting or ofShareDenyWrite
        CALL     FileCreate
        MOV      [EBX].TStream.fData.fHandle, EAX
        XCHG     EAX, EBX
        POP      EBX
end;

function NewWriteFileStream( const FileName: KOLString ): PStream;
asm
        PUSH     EBX
        XCHG     EBX, EAX
        MOV      EAX, offset[BaseFileMethods]
        CALL     _NewStream
        MOV      [EAX].TStream.fMethods.fWrite, offset[WriteFileStreamEOF]
        MOV      [EAX].TStream.fMethods.fSetSiz, offset[SetSizeFileStream]
        XCHG     EBX, EAX
        MOV      EDX, ofOpenWrite or ofCreateAlways or ofShareDenyWrite
        CALL     FileCreate
        MOV      [EBX].TStream.fData.fHandle, EAX
        XCHG     EAX, EBX
        POP      EBX
end;

destructor TIniFile.Destroy;
asm     //cmd    //opd
        PUSH     EAX
        LEA      EDX, [EAX].fFileName
        PUSH     EDX
        LEA      EAX, [EAX].fSection
        {$IFDEF UNICODE_CTRLS}
        CALL     System.@WStrClr
        {$ELSE}
        CALL     System.@LStrClr
        {$ENDIF}
        POP      EAX
        {$IFDEF UNICODE_CTRLS}
        CALL     System.@WStrClr
        {$ELSE}
        CALL     System.@LStrClr
        {$ENDIF}
        POP      EAX
        CALL     TObj.Destroy
end;

function MakeAccelerator( fVirt: Byte; Key: Word ): TMenuAccelerator;
asm
        MOVZX    EAX, AL
        PUSH     EAX
        MOV      [ESP+1], DX
        POP      EAX
end;

function NewCommandActionsObj_Packed( fromPack: PAnsiChar ): PCommandActionsObj;
asm
    PUSH ESI
    PUSH EDI
    PUSH EAX
    CALL NewCommandActionsObj
    POP  ESI
    CMP  ESI, 120
    MOV  [EAX].TCommandActionsObj.fIndexInActions, ESI
    JB   @@exit
    PUSH EAX
    LEA  EDI, [EAX].TCommandActionsObj.aClick
    XOR  EAX, EAX
    LODSB
    MOV  dword ptr [EDI + 76], EAX // Result.fIndexInActions := fromPack[0]
    XOR  ECX, ECX
    MOV  CL, 38
@@loop:
    CMP  byte ptr[ESI], 200
    JB   @@copy_word
    JA   @@clear_words
    INC  ESI
@@copy_word:
    MOVSW
    LOOP @@loop
    JMP  @@fin
@@clear_words:
    LODSB
    SUB  AL, 200
    SUB  CL, AL
    PUSH ECX
    MOVZX ECX, AL
    XOR   EAX, EAX
    REP   STOSW
    POP   ECX
    INC   ECX
    LOOP  @@loop
@@fin:
    POP  EAX
@@exit:
    POP  EDI
    POP  ESI
end;

function _NewTControl( AParent: PControl ): PControl;
begin
  New( Result, CreateParented( AParent ) );
end;

function _NewWindowed( AParent: PControl; ControlClassName: PKOLChar;
         Ctl3D: Boolean; ACommandActions: TCommandActionsParam ): PControl;
const   Sz_TCommandActions = Sizeof(TCommandActions);
asm
        PUSH     EBX
        PUSH     ESI
        PUSH     EDI
        MOV      EDI, ACommandActions
        MOV      [ACommandActions], ECX // Ctl3D -> ACommandActions

        PUSH     EDX // ControlClassName

        MOV      ESI, EAX // ESI = AParent
        CALL     _NewTControl
        XCHG     EBX, EAX // EBX = Result
        POP      [EBX].TControl.fControlClassName
        //INC      [EBX].TControl.fWindowed // set in TControl.Init

        {$IFDEF  COMMANDACTIONS_OBJ}
        MOV      EAX, EDI
        CMP      EAX, 120
        JB       @@IdxActions_Loaded
        MOVZX    EAX, byte ptr[EDI]
@@IdxActions_Loaded:
        PUSH     EAX
        MOV      ECX, dword ptr [AllActions_Objs + EAX*4]
        JECXZ    @@create_new_action
        XCHG     EAX, ECX
        PUSH     EAX
            CALL     TObj.RefInc
        POP      EAX
        JMP      @@action_assign

@@create_new_action:
        {$IFDEF  PACK_COMMANDACTIONS}
                MOV      EAX, EDI
                CALL     NewCommandActionsObj_Packed
        {$ELSE   not PACK_COMMANDACTIONS}
                CALL     NewCommandActionsObj

                TEST     EDI, EDI
                JZ       @@no_actions

                PUSH     EAX
                LEA      EDX, [EAX].TCommandActionsObj.aClear
                XCHG     EAX, EDI
                XOR      ECX, ECX
                MOV      CL, Sz_TCommandActions
                CALL     Move
                POP      EAX
                JMP      @@action_assign
        @@no_actions:
        {$ENDIF  not PACK_COMMANDACTIONS}
                MOV      [EAX].TCommandActionsObj.aClear, offset[ClearText]

@@action_assign:
        POP      EDX
        MOV      dword ptr [AllActions_Objs + EDX*4], EAX

        MOV      [EBX].TControl.fCommandActions, EAX
        XCHG     EDX, EAX
        MOV      EAX, EBX
        CALL     TControl.Add2AutoFree

        {$ELSE}
        TEST     EDI, EDI
        JZ       @@no_actions2
        PUSH     ESI
        MOV      ESI, EDI
        LEA      EDI, [EBX].TControl.fCommandActions
        XOR      ECX, ECX
        MOV      CL, Sz_TCommandActions
        REP      MOVSB
        POP      ESI
        JMP      @@actions_created
@@no_actions2:
        MOV      [EBX].TControl.fCommandActions.TCommandActions.aClear, offset[ClearText]
        {$ENDIF}
@@actions_created:

        TEST     ESI, ESI
        JZ       @@no_parent

        MOV      EAX, [ESI].TControl.PP.fGotoControl
        MOV      [EBX].TControl.PP.fGotoControl, EAX

        LEA      ESI, [ESI].TControl.fTextColor
        LEA      EDI, [EBX].TControl.fTextColor
        MOVSD    // fTextColor
        MOVSD    // fColor

        {$IFDEF SMALLEST_CODE}
            {$IFDEF SMALLEST_CODE_PARENTFONT}
                LODSD
                XCHG     EDX, EAX
                XOR      EAX, EAX
                CALL     TGraphicTool.Assign
                STOSD    // fFont
            {$ELSE}
                LODSD
                XOR     EAX, EAX
                STOSD   // fFont = nil
            {$ENDIF}
        {$ELSE}
            LODSD
            XCHG     EDX, EAX
            XOR      EAX, EAX
            PUSH     EDX
            CALL     TGraphicTool.Assign
            STOSD    // fFont
            POP      EDX
            XCHG     ECX, EAX
            JECXZ    @@no_font
            MOV      [ECX].TGraphicTool.fParentGDITool, EDX
            MOV      [ECX].TGraphicTool.fOnGTChange.TMethod.Code, offset[TControl.FontChanged]
            MOV      [ECX].TGraphicTool.fOnGTChange.TMethod.Data, EBX
            MOV      EAX, EBX
            MOV      EDX, ECX
            CALL     TControl.FontChanged
            {$IFDEF USE_AUTOFREE4CONTROLS}
                MOV      EAX, EBX
                MOV      EDX, [EBX].TControl.fFont
                CALL     TControl.Add2AutoFree
            {$ENDIF}
@@no_font:
        {$ENDIF}

        {$IFDEF SMALLEST_CODE}
            LODSD
            XOR      EAX, EAX
            STOSD
        {$ELSE}
            LODSD
            XCHG     EDX, EAX
            XOR      EAX, EAX
            PUSH     EDX
            CALL     TGraphicTool.Assign
            STOSD    // fBrush
            POP      EDX
            XCHG     ECX, EAX
            JECXZ    @@no_brush
            MOV      [ECX].TGraphicTool.fParentGDITool, EDX
            MOV      [ECX].TGraphicTool.fOnGTChange.TMethod.Code, offset[TControl.BrushChanged]
            MOV      [ECX].TGraphicTool.fOnGTChange.TMethod.Data, EBX
            MOV      EAX, EBX
            MOV      EDX, ECX
            CALL     TControl.BrushChanged
            {$IFDEF USE_AUTOFREE4CONTROLS}
                MOV      EAX, EBX
                MOV      EDX, [EBX].TControl.fBrush
                CALL     TControl.Add2AutoFree
            {$ENDIF}
@@no_brush:
        {$ENDIF}

        MOVSB    // fMargin
        LODSD           // skip fClientXXXXX
        ADD      EDI, 4

        LODSB    // fCtl3D_child
        TEST     AL, 2
        JZ       @@passed3D
        MOV      EDX, [ACommandActions] // DL <- Ctl3D !!!
        AND      AL, not 1
        AND      DL, 1
        OR       EAX, EDX
@@passed3D:
        STOSB    // fCtl3D_child

@@no_parent:
        XCHG     EAX, EBX
        POP      EDI
        POP      ESI
        POP      EBX
        {$IFDEF  DUMP_WINDOWED}
        CALL     DumpWindowed
        {$ENDIF}
end;

function NewForm( AParent: PControl; const Caption: KOLString ): PControl;
const FormClass: array[ 0..4 ] of KOLChar = ( 'F', 'o', 'r', 'm', #0 );
asm
        PUSH     EBX
        PUSH     EDX
        MOV      EDX, offset[FormClass]
        MOV      CL, 1
        {$IFDEF  COMMANDACTIONS_OBJ}
        PUSH     OTHER_ACTIONS
        {$ELSE}
        PUSH     0
        {$ENDIF}
        CALL     _NewWindowed
        MOV      EBX, EAX
        OR       byte ptr [EBX].TControl.fClsStyle, CS_DBLCLKS
        MOV      EDX, offset[WndProcForm]
        CALL     TControl.AttachProc
        MOV      EDX, offset[WndProcDoEraseBkgnd]
        MOV      EAX, EBX
        CALL     TControl.AttachProc
        POP      EDX
        MOV      EAX, EBX
        CALL     TControl.SetCaption
        {$IFDEF  USE_FLAGS}
        OR       [EBX].TControl.fFlagsG3, (1 shl G3_IsForm) or (1 shl G3_SizeGrip)
        {$ELSE}
        INC      [EBX].TControl.fSizeGrip
        DEC      WORD PTR [EBX].TControl.fIsForm // why word?
        {$ENDIF}
        XCHG     EAX, EBX
        POP      EBX
end;

function NewButton( AParent: PControl; const Caption: KOLString ): PControl;
const szActions = sizeof(TCommandActions);
asm
   PUSH EBX
        PUSH     EDX

        PUSH     0
        {$IFDEF  PACK_COMMANDACTIONS}
        PUSH     [ButtonActions_Packed]
        {$ELSE}
        PUSH     offset[ButtonActions]
        {$ENDIF}
        MOV      EDX, offset[ButtonClass]
        MOV      ECX, WS_VISIBLE or WS_CHILD or BS_PUSHLIKE or WS_TABSTOP or BS_NOTIFY
        CALL     _NewControl
        XCHG     EBX, EAX
        //MOV      Byte Ptr[EBX].TControl.aAutoSzX, 14
        //MOV      Byte Ptr[EBX].TControl.aAutoSzY, 6
        MOV      word ptr [EBX].TControl.aAutoSzX, 6 shl 8 + 14
        MOV      EDX, [EBX].TControl.fBoundsRect.Top
        ADD      EDX, 22
        MOV      [EBX].TControl.fBoundsRect.Bottom, EDX
        MOV      [EBX].TControl.fTextAlign, taCenter
        {$IFDEF  USE_FLAGS}
        OR       [EBX].TControl.fFlagsG5, (1 shl G5_IsButton) or (1 shl G5_IgnoreDefault)
        {$ELSE}
        INC      [EBX].TControl.fIsButton
        INC      [EBX].TControl.fIgnoreDefault
        {$ENDIF}
        POP      EDX
        MOV      EAX, EBX
        CALL     TControl.SetCaption
        {$IFNDEF SMALLEST_CODE}
        {$IFNDEF BUTTON_DBLCLICK}
        MOV      EAX, EBX
        MOV      EDX, offset[WndProcBtnDblClkAsClk]
        CALL     TControl.AttachProc
        {$ENDIF}
        {$ENDIF SMALLEST_CODE}
        {$IFDEF ALL_BUTTONS_RESPOND_TO_ENTER}
        MOV      EAX, EBX
        MOV      EDX, offset[WndProcBtnReturnClick]
        CALL     TControl.AttachProc
        {$ENDIF}
   XCHG EAX, EBX
   POP  EBX
   
{$IFDEF GRAPHCTL_XPSTYLES}
        PUSH     EAX
        MOV      EDX, offset[XP_Themes_For_BitBtn]
        CALL     Attach_WM_THEMECHANGED
        POP      EAX
{$ENDIF}
end;

function WndProc_DrawItem( Sender: PControl; var Msg: TMsg; var Rslt: Integer )
                          : Boolean;
asm     //cmd    //opd
       CMP       word ptr [EDX].TMsg.message, WM_DRAWITEM
       JNZ       @@ret_false
       MOV       EAX, [EDX].TMsg.lParam
       MOV       ECX, [EAX].TDrawItemStruct.hwndItem
       JECXZ     @@ret_false
       PUSH      EDX
       {$IFDEF USE_PROP}
       PUSH      offset[ID_SELF]
       PUSH      ECX
       CALL      GetProp
       {$ELSE}
       PUSH      GWL_USERDATA
       PUSH      ECX
       CALL      GetWindowLong
       {$ENDIF}
       POP       EDX
       TEST      EAX, EAX
       JZ        @@ret_false
       PUSH      [EDX].TMsg.lParam
       PUSH      [EDX].TMsg.wParam
       PUSH      CN_DRAWITEM
       PUSH      EAX
       CALL      TControl.Perform
       MOV       AL, 1
       RET
@@ret_false:
       XOR       EAX, EAX
end;

{$IFDEF BITBTN_ASM}
function NewBitBtn( AParent: PControl; const Caption: KOLString;
         Options: TBitBtnOptions; Layout: TGlyphLayout; GlyphBitmap: HBitmap; GlyphCount: Integer ): PControl;
const szBitmapInfo = sizeof(TBitmapInfo);
asm
        PUSH     EBX
        PUSH     EDX
        PUSH     ECX

        PUSH     0
        {$IFDEF  PACK_COMMANDACTIONS}
        PUSH     [ButtonActions_Packed]
        {$ELSE}
        PUSH     offset[ButtonActions]
        {$ENDIF}
        MOV      EDX, offset[ButtonClass]
        MOV      ECX, WS_VISIBLE or WS_CHILD or WS_TABSTOP or BS_OWNERDRAW or BS_NOTIFY
        CALL     _NewControl
        XCHG     EBX, EAX
        {$IFDEF  USE_FLAGS}
        OR       [EBX].TControl.fFlagsG5, (1 shl G5_IgnoreDefault)or(1 shl G5_IsButton)or(1 shl G5_IsBitBtn)
        {$ELSE}
        INC      [EBX].TControl.fIgnoreDefault
        INC      [EBX].TControl.fIsButton
        INC      [EBX].TControl.fIsBitBtn
        {$ENDIF}
        //MOV      byte ptr [EBX].TControl.fCommandActions.aAutoSzX, 8
        //MOV      byte ptr [EBX].TControl.fCommandActions.aAutoSzY, 8
        MOV      word ptr [EBX].TControl.fCommandActions.aAutoSzY, $808
        POP      EAX
        MOV      [EBX].TControl.fBitBtnOptions, AL
        MOVZX    EDX, Layout
        MOV      [EBX].TControl.fGlyphLayout, DL
        MOV      ECX, GlyphBitmap
        MOV      [EBX].TControl.fGlyphBitmap, ECX
        MOV      EDX, [EBX].TControl.fBoundsRect.Top
        ADD      EDX, 22
        MOV      [EBX].TControl.fBoundsRect.Bottom, EDX
        TEST     ECX, ECX
        JZ       @@noGlyphWH
        {$IFDEF PARANOIA} DB $A8, 01 {$ELSE} TEST AL, bboImageList {$ENDIF}
        JZ       @@getBmpWH
        PUSH     EAX
        MOV      EAX, ESP
        PUSH     EAX
        MOV      EDX, ESP
        PUSH     EAX
        PUSH     EDX
        PUSH     ECX
        CALL     ImageList_GetIconSize
        POP      EAX
        POP      EDX
        MOV      ECX, GlyphCount
        JMP      @@WHready
@@getBmpWH:
        ADD      ESP, -szBitmapInfo
        PUSH     ESP
        PUSH     szBitmapInfo
        PUSH     ECX
        CALL     GetObject
        XCHG     ECX, EAX
        POP      EAX
        POP      EAX
        POP      EDX
        ADD      ESP, szBitmapInfo-12
        TEST     ECX, ECX
        JZ       @@noGlyphWH
        MOV      ECX, GlyphCount
        INC      ECX
        LOOP     @@GlyphCountOK
        PUSH     EAX
        PUSH     EDX
        XCHG     EDX, ECX
        DIV      ECX
        XCHG     ECX, EAX
        POP      EDX
        POP      EAX
@@GlyphCountOK:
        CMP      ECX, 1
        JLE      @@WHReady
        PUSH     EDX
        CDQ
        IDIV     ECX
        POP      EDX
@@WHReady:
        MOV      [EBX].TControl.fGlyphWidth, EAX
        MOV      [EBX].TControl.fGlyphHeight, EDX
        MOV      [EBX].TControl.fGlyphCount, ECX
        POP      ECX     // ECX = @ Caption[ 1 ]
        PUSH     ECX
        PUSH     EDX
        PUSH     EAX
        TEST     EAX, EAX
        JLE      @@noWidthResize
        JECXZ    @@addWLeft
        CMP      [Layout], glyphOver
        JE       @@addWLeft
        MOVZX    ECX, byte ptr[ECX]
        JECXZ    @@addWLeft
        // else
        CMP      [Layout], glyphLeft
        JZ       @@addWRight
        CMP      [Layout], glyphRight
        JNZ      @@noWidthResize
@@addWRight:
        ADD      [EBX].TControl.fBoundsRect.Right, EAX
        ADD      byte ptr [EBX].TControl.aAutoSzX, AL
        JMP      @@noWidthResize
@@addWLeft:
        // then
        ADD      EAX, [EBX].TControl.fBoundsRect.Left
        MOV      [EBX].TControl.fBoundsRect.Right, EAX
        MOV      byte ptr [EBX].TControl.aAutoSzX, 0
@@noWidthResize:
        TEST     EDX, EDX
        JLE      @@noHeightResize
        CMP      [Layout], glyphTop
        JE       @@addHBottom
        CMP      [Layout], glyphBottom
        JNE      @@addHTop
@@addHBottom:
        ADD      [EBX].TControl.fBoundsRect.Bottom, EDX
        ADD      byte ptr [EBX].TControl.aAutoSzY, DL
        JMP      @@noHeightResize
@@addHTop:
        ADD      EDX, [EBX].TControl.fBoundsRect.Top
        MOV      [EBX].TControl.fBoundsRect.Bottom, EDX
        MOV      byte ptr [EBX].TControl.aAutoSzY, 0
@@noHeightResize:
        POP      ECX
        POP      EAX
        CDQ
        MOV      DL, 4
        TEST     [EBX].TControl.fBitBtnOptions, 2 //1 shl bboNoBorder
        JNZ      @@noBorderResize
        JECXZ    @@noBorderWinc
        ADD      [EBX].TControl.fBoundsRect.Right, EDX
        CMP      [EBX].TControl.aAutoSzX, 0
        JZ       @@noBorderWinc
        ADD      [EBX].TControl.aAutoSzX, DL
@@noBorderWinc:
        TEST     EAX, EAX
        JLE      @@noBorderResize
        ADD      [EBX].TControl.fBoundsRect.Bottom, EDX
        CMP      [EBX].TControl.aAutoSzY, 0
        JZ       @@noBorderResize
        ADD      [EBX].TControl.aAutoSzY, DL
@@noBorderResize:
@@noGlyphWH:
        MOV      ECX, [EBX].TControl.fParent
        JECXZ    @@notAttach2Parent
        XCHG     EAX, ECX
        MOV      EDX, offset[WndProc_DrawItem]
        CALL     TControl.AttachProc
@@notAttach2Parent:
        MOV      EAX, EBX
        MOV      EDX, offset[WndProcBitBtn]
        CALL     TControl.AttachProc
        MOV      EAX, EBX
        POP      EDX
        CALL     TControl.SetCaption
        MOV      [EBX].TControl.fTextAlign, taCenter
        {$IFDEF ALL_BUTTONS_RESPOND_TO_ENTER}
        MOV      EAX, EBX
        MOV      EDX, offset[WndProcBtnReturnClick]
        CALL     TControl.AttachProc
        {$ENDIF}
        XCHG     EAX, EBX
        POP      EBX
        
{$IFDEF GRAPHCTL_XPSTYLES}
        PUSH     EAX
        MOV      EDX, offset[XP_Themes_For_BitBtn]
        CALL     Attach_WM_THEMECHANGED
        POP      EAX
 {$ENDIF}
end;
{$ENDIF BITBTN_ASM}

function NewCheckbox( AParent: PControl; const Caption: KOLString ): PControl;
asm
        CALL     NewButton
        MOV      EDX, [EAX].TControl.fBoundsRect.Left
        ADD      EDX, 72
        MOV      [EAX].TControl.fBoundsRect.Right, EDX
        MOV      [EAX].TControl.fStyle, WS_VISIBLE or WS_CHILD or BS_AUTOCHECKBOX or WS_TABSTOP or BS_NOTIFY
        MOV      [EAX].TControl.aAutoSzX, 24
        
{$IFDEF GRAPHCTL_XPSTYLES}
        PUSH     EAX
        MOV      EDX, offset[XP_Themes_For_CheckBox]
        CALL     Attach_WM_THEMECHANGED
        POP      EAX
{$ENDIF}
end;

procedure ClickRadio( Sender:PObj );
asm
        PUSH     EBX
        MOV      EBX, [EAX].TControl.fParent
        TEST     EBX, EBX
        JZ       @@exit
        {$IFDEF  USE_FLAGS}
        PUSH     ESI
        PUSH     EDI
        XCHG     ESI, EAX
        OR       EDI, -1
@@cont_loop:
        INC      EDI
        MOV      EAX, [EBX].TControl.fChildren
        CMP      EDI, [EAX].TList.fCount
        JGE      @@e_loop
        MOV      EDX, EDI
        CALL     TList.Get
        TEST     [EAX].TControl.fFlagsG5, 1 shl G5_IsButton
        JZ       @@cont_loop
        TEST     [EAX].TControl.fStyle.f0_Style, BS_RADIOBUTTON
        JZ       @@cont_loop
        CMP      EAX, ESI
        PUSH     EAX
            SETZ     DL
            PUSH     EDX
                CALL     TControl.GetChecked
            POP      EDX
            CMP      DL, AL
        POP      EAX
        JZ       @@cont_loop
        CALL     TControl.SetChecked
        JMP      @@cont_loop
@@e_loop:
        POP      EDI
        POP      ESI
        {$ELSE   not USE_FLAGS}
        PUSH     [EAX].TControl.fMenu
        MOV      EAX, EBX
        MOV      EDX, offset[RADIO_LAST]
        CALL     TControl.Get_Prop_Int
        PUSH     EAX
        MOV      EAX, EBX
        MOV      EDX, offset[RADIO_1ST]
        CALL     TControl.Get_Prop_Int
        PUSH     EAX
        PUSH     [EBX].TControl.fHandle
        CALL     CheckRadioButton
        {$ENDIF  USE_FLAGS}
@@exit:
        POP      EBX
end;

function NewRadiobox( AParent: PControl; const Caption: KOLString ): PControl;
const
  RadioboxStyles = WS_VISIBLE or WS_CHILD or BS_RADIOBUTTON or
                   WS_TABSTOP or WS_GROUP or BS_NOTIFY;
asm
        PUSH     EBX
        PUSH     ESI
        MOV      ESI, EAX
        CALL     NewCheckbox
        XCHG     EBX, EAX
        MOV      [EBX].TControl.fStyle, RadioboxStyles
        MOV      [EBX].TControl.PP.fControlClick, offset[ClickRadio]
        TEST     ESI, ESI
        JZ       @@exit
        {$IFDEF  USE_FLAGS}
        BTS      DWORD PTR [ESI].TControl.fFlagsG1, 1 shl G1_HasRadio
        JNZ      @@exit
        MOV      EAX, EBX
        CALL     TControl.SetRadioChecked
        {$ELSE}
        MOV      ECX, [EBX].TControl.fMenu
        PUSH     ECX
        MOV      EDX, offset[RADIO_LAST]
        MOV      EAX, ESI
        CALL     TControl.Set_Prop_Int
        MOV      EDX, offset[RADIO_1ST]
        PUSH     EDX
        MOV      EAX, ESI
        CALL     TControl.Get_Prop_Int
        TEST     EAX, EAX
        POP      EDX
        POP      ECX
        JNZ      @@exit
        MOV      EAX, ESI
        CALL     TControl.Set_Prop_Int
        MOV      EAX, EBX
        CALL     TControl.SetRadioChecked
        {$ENDIF}
@@exit: XCHG     EAX, EBX
        POP      ESI
        POP      EBX

{$IFDEF GRAPHCTL_XPSTYLES}
        PUSH     EAX
        MOV      EDX, offset[XP_Themes_For_RadioBox]
        CALL     Attach_WM_THEMECHANGED
        POP      EAX
{$ENDIF}
end;

function NewWordWrapLabel( AParent: PControl; const Caption: KOLString ): PControl;
asm
        CALL     NewLabel
        MOV      EDX, [EAX].TControl.fBoundsRect.Top
        ADD      EDX, 44
        MOV      [EAX].TControl.fBoundsRect.Bottom, EDX
        {$IFDEF  USE_FLAGS}
        OR       [EAX].TControl.fFlagsG1, (1 shl G1_WordWrap)
        {$ELSE}
        INC      [EAX].TControl.fWordWrap
        {$ENDIF}
        AND      byte ptr [EAX].TControl.fStyle, not SS_LEFTNOWORDWRAP
end;

function NewLabelEffect( AParent: PControl; const Caption: KOLString; ShadowDeep: Integer ): PControl;
asm
        PUSH     EBX

        PUSH     ECX
        PUSH     EDX
        XOR      EDX, EDX
        CALL     NewLabel
        MOV      EBX, EAX
        {$IFDEF  USE_FLAGS}
        AND      [EBX].TControl.fFlagsG1, not(1 shl G1_IsStaticControl)
        {$ELSE}
        DEC      [EBX].TControl.fIsStaticControl // снова 0 !
        {$ENDIF  USE_FLAGS}
        MOV      EDX, offset[WndProcLabelEffect]
        CALL     TControl.AttachProc

        POP      EDX
        MOV      EAX, EBX
        CALL     TControl.SetCaption

        MOV      EDX, offset[WndProcDoEraseBkgnd]
        MOV      EAX,EBX
        CALL     TControl.AttachProc
        MOV      [EBX].TControl.fTextAlign, taCenter
        MOV      [EBX].TControl.fTextColor, clWindowText
        POP      [EBX].TControl.DF.fShadowDeep
        {$IFDEF  USE_FLAGS}
        OR       [EBX].TControl.fFlagsG1, (1 shl G1_IgnoreWndCaption)
        {$ELSE}
        INC      [EBX].TControl.fIgnoreWndCaption
        {$ENDIF  USE_FLAGS}
        ADD      [EBX].TControl.fBoundsRect.Bottom, 40 - 22
        MOV      [EBX].TControl.DF.fColor2, clNone

        XCHG     EAX, EBX
        POP      EBX
end;

function WndProcDoEraseBkgnd( Self_: PControl; var Msg: TMsg; var Rslt: Integer ): Boolean;
asm     //        //
        CMP       word ptr [EDX].TMsg.message, WM_ERASEBKGND
        JNE       @@ret_false
        MOV       byte ptr [ECX], 1
        PUSH      EBX
        PUSH      EDI
        MOV       EBX, EAX
        MOV       EDI, [EDX].TMsg.wParam

        {$IFDEF SMALLEST_CODE}
        {$ELSE}
        CALL      TControl.CreateChildWindows
        {$IFDEF   USE_FLAGS}
        TEST      [EBX].TControl.fFlagsG2, (1 shl G2_Transparent)
        {$ELSE}
        CMP       [EBX].TControl.fTransparent, 0
        {$ENDIF   USE_FLAGS}
        JNE       @@exit
        {$ENDIF}

        {$IFDEF SMALLEST_CODE}
        {$ELSE}
        PUSH      OPAQUE
        PUSH      EDI
        CALL      SetBkMode
        MOV       EAX, [EBX].TControl.fColor
        CALL      Color2RGB
        PUSH      EAX
        PUSH      EDI
        CALL      SetBkColor
        XOR       EAX, EAX
        PUSH      EAX
        PUSH      EAX
        PUSH      EAX
        PUSH      EDI
        CALL      SetBrushOrgEx
        {$ENDIF}
        SUB       ESP, 16
        PUSH      ESP
        PUSH      [EBX].TControl.fHandle
        CALL      GetClientRect
        MOV       EAX, EBX
        CALL      dword ptr[Global_GetCtlBrushHandle]
        MOV       EDX, ESP
        PUSH      EAX
        PUSH      EDX
        PUSH      EDI
        CALL      Windows.FillRect
        ADD       ESP, 16
@@exit: POP       EDI
        POP       EBX
@@ret_false:
        XOR       EAX, EAX
end;

function WndProcSplitter( Self_: PControl; var Msg: TMsg; var Rslt: Integer ): Boolean;
asm
        CMP      word ptr [EDX].TMsg.message, WM_NCHITTEST
        JNE      @@noWM_NCHITTEST
        PUSH     ECX
        PUSH     [EDX].TMsg.lParam
        PUSH     [EDX].TMsg.wParam
        PUSH     [EDX].TMsg.message
        PUSH     [EAX].TControl.fHandle
        CALL     DefWindowProc
        TEST     EAX, EAX
        JLE      @@htReady
        XOR      EAX, EAX
        INC      EAX
@@htReady:
        POP      ECX
        MOV      [ECX], EAX
        MOV      AL, 1
        RET

@@noWM_NCHITTEST:
        PUSH     EBX
        XCHG     EBX, EAX
        CMP      word ptr [EDX].TMsg.message, WM_MOUSEMOVE
        JNE      @@noWM_MOUSEMOVE

        PUSH     [EBX].TControl.fCursor
        CALL     Windows.SetCursor

        XOR      EDX, EDX

        {$IFDEF USE_ASM_DODRAG}
        CALL     @@DoDrag
        {$ELSE}
        MOV      EAX, EBX
        CALL     DoDrag
        {$ENDIF}

        POP      EBX
        RET

{$IFDEF USE_ASM_DODRAG}
@@DoDrag:
        PUSHAD
        MOVZX    EDI, DL // EDI = 1 if Cancel, 0 otherwise
        CMP      [EBX].TControl.fDragging, 0
        JZ       @@e_DoDrag
        MOV      EAX, [EBX].TControl.fParent
        MOV      EAX, [EAX].TControl.fChildren
        PUSH     EAX
        MOV      EDX, EBX
        CALL     TList.IndexOf
        POP      EDX // EDX = Self_.fParent.fChildren:PList
        MOV      EBP, EBX  // Prev := Self_;
        TEST     EAX, EAX
        JLE      @@noPrev
        MOV      EDX, [EDX].TList.fItems
        MOV      EBP, [EDX+EAX*4-4] // Prev = Self_.fParent.fChildren.fItems[I-1]
        PUSH     EBP  // push Prev
@@noPrev:
        PUSH     EDX
        PUSH     EDX
        PUSH     ESP
        CALL     GetCursorPos
        DEC      EDI
        JNZ      @@noCancel
        POP      EDX
        POP      EDX
        PUSH     [EBX].TControl.fSplitStartPos.y
        PUSH     [EBX].TControl.fSplitStartPos.x
@@noCancel:
        OR       EDI, -1
        MOV      CL, [EBX].TControl.fAlign
        MOV      AL, 1
        SHL      EAX, CL
        {$IFDEF PARANOIA} DB $A8, chkRight or chkBott {$ELSE} TEST AL, chkRight or chkBott {$ENDIF} //fAlign in [ caRight, caBottom ] ?
        JNZ      @@mReady
        INC      EDI
        INC      EDI
@@mReady:
        MOV      EDX, [EBX].TControl.fParent
        MOVSX    EBP, [EDX].TControl.fMargin
        NEG      EBP
        {$IFDEF PARANOIA} DB $A8, chkTop or chkBott {$ELSE} TEST AL, chkTop or chkBott {$ENDIF} // fAlign in [ caTop, caBottom ] ?
        XCHG     EAX, EDX
        JZ       @@noTopBottom

        CALL     TControl.GetClientHeight
        XCHG     EDX, EAX

        POP      EAX
        POP      ESI // MousePos.y
        MOV      EAX, ESI
        PUSH     EDX // Self_.fParent.ClientHeight
        SUB      EAX, [EBX].TControl.fSplitStartPos.y
        IMUL     EAX, EDI
        ADD      EAX, [EBX].TControl.fSplitStartSize // EAX = NewSize1

        POP      EDX
        SUB      EDX, EAX
        SUB      EDX, [EBX].TControl.fBoundsRect.Bottom
        ADD      EDX, [EBX].TControl.fBoundsRect.Top
        LEA      EDX, [EDX+EBP*4]

        MOV      ECX, [EBX].TControl.fSecondControl
        JECXZ    @@noSecondControl
        MOV      EDX, [ECX].TControl.fBoundsRect.Bottom
        SUB      EDX, [ECX].TControl.fBoundsRect.Top
        CMP      [ECX].TControl.fAlign, caClient
        JNZ      @@noSecondControl

        PUSH     EAX
        MOV      EAX, [EBX].TControl.fSplitStartPos.y
        SUB      EAX, ESI
        IMUL     EAX, EDI
        ADD      EAX, [EBX].TControl.fSplitStartPos2.y
        LEA      EDX, [EAX+EBP*4]
        POP      EAX

@@noSecondControl:
        JMP      @@newSizesReady

@@noTopBottom:
        CALL     TControl.GetClientWidth
        XCHG     EDX, EAX

        POP      ESI // MousePos.x
        POP      ECX
        MOV      EAX, ESI
        PUSH     EDX // Self_.fParent.ClientWidth
        SUB      EAX, [EBX].TControl.fSplitStartPos.x
        IMUL     EAX, EDI
        ADD      EAX, [EBX].TControl.fSplitStartSize // EAX = NewSize1

        POP      EDX
        SUB      EDX, EAX
        SUB      EDX, [EBX].TControl.fBoundsRect.Right
        ADD      EDX, [EBX].TControl.fBoundsRect.Left
        LEA      EDX, [EDX+EBP*4]

        MOV      ECX, [EBX].TControl.fSecondControl
        JECXZ    @@newSizesReady
        MOV      EDX, [ECX].TControl.fBoundsRect.Right
        SUB      EDX, [ECX].TControl.fBoundsRect.Left
        CMP      [ECX].TControl.fAlign, caClient
        JNZ      @@noSecondControl

        PUSH     EAX
        MOV      EAX, [EBX].TControl.fSplitStartPos.x
        SUB      EAX, ESI
        IMUL     EAX, EDI
        ADD      EAX, [EBX].TControl.fSplitStartPos2.x
        LEA      EDX, [EAX+EBP*4]
        POP      EAX

@@newSizesReady:
        MOV      ECX, [EBX].TControl.fSplitMinSize1
        SUB      ECX, EAX
        JLE      @@noCheckMinSize1
        SUB      EDX, ECX
        ADD      EAX, ECX

@@noCheckMinSize1:
        MOV      ECX, [EBX].TControl.fSplitMinSize2
        SUB      ECX, EDX
        JLE      @@noCheckMinSize2
        SUB      EAX, ECX
        ADD      EDX, ECX

@@noCheckMinSize2:
        MOV      ECX, [EBX].TControl.fOnSplit.TMethod.Code
        JECXZ    @@noOnSplit
        PUSHAD
        PUSH     EDX
        MOV      ESI, ECX
        XCHG     ECX, EAX
        MOV      EDX, EBX
        MOV      EAX, [EBX].TControl.fOnSplit.TMethod.Data
        CALL     ESI
        TEST     AL, AL
        POPAD
        JZ       @@e_DoDrag

@@noOnSplit:
        XCHG     ESI, EAX // NewSize1 -> ESI
        POP      EBP
        ADD      ESP, -16
        MOV      EAX, EBP
        MOV      EDX, ESP
        CALL     TControl.GetBoundsRect
        MOVZX    ECX, [EBX].TControl.fAlign
        LOOP     @@noPrev_caLeft
        ADD      ESI, [ESP].TRect.Left
        MOV      [ESP].TRect.Right, ESI
@@noPrev_caLeft:
        LOOP     @@noPrev_caTop
        ADD      ESI, [ESP].TRect.Top
        MOV      [ESP].TRect.Bottom, ESI
@@noPrev_caTop:
        LOOP     @@noPrev_caRight
        MOV      EAX, [ESP].TRect.Right
        SUB      EAX, ESI
        MOV      [ESP].TRect.Left, EAX
@@noPrev_caRight:
        LOOP     @@noPrev_caBottom
        MOV      EAX, [ESP].TRect.Bottom
        SUB      EAX, ESI
        MOV      [ESP].TRect.Top, EAX
@@noPrev_caBottom:
        MOV       EAX, EBP
        MOV       EDX, ESP
        CALL      TControl.SetBoundsRect
        ADD       ESP, 16
    {$IFDEF OLD_ALIGN}
        MOV       EAX, [EBX].TControl.fParent
    {$ELSE NEW_ALIGN}
        MOV       EAX, EBX
    {$ENDIF}
        CALL      dword ptr[Global_Align]

@@e_DoDrag:
        POPAD
        RET
{$ENDIF USE_ASM_DODRAG}

@@noWM_MOUSEMOVE:
        CMP      word ptr [EDX].TMsg.message, WM_LBUTTONDOWN
        JNE      @@noWM_LBUTTONDOWN
        MOV      ECX, [EBX].TControl.fParent
        TEST     ECX, ECX
        JZ       @@noWM_LBUTTONDOWN

        MOV      EAX, [ECX].TControl.fChildren
        PUSH     EAX
        MOV      EDX, EBX
        CALL     TList.IndexOf
        POP      ECX
        MOV      EDX, EBX
        TEST     EAX, EAX
        JLE      @@noParent1
        MOV      ECX, [ECX].TList.fItems
        MOV      EDX, [ECX+EAX*4-4]
@@noParent1:

        MOV      CL, [EBX].TControl.fAlign
        MOV      AL, 1
        SHL      EAX, CL
        {$IFDEF PARANOIA} DB $A8, chkTop or chkBott {$ELSE} TEST AL, chkTop or chkBott {$ENDIF} // fAlign in [caTop,caBottom] ?
        XCHG     EAX, EDX
        JZ       @@no_caTop_caBottom
        CALL     TControl.GetHeight
        JMP      @@caTop_caBottom
@@no_caTop_caBottom:
        CALL     TControl.GetWidth
@@caTop_caBottom:
        MOV      [EBX].TControl.DF.fSplitStartSize, EAX
        MOV      ECX, [EBX].TControl.DF.fSecondControl
        JECXZ    @@noSecondControl1
        XCHG     EAX, ECX
        PUSH     EAX
        CALL     TControl.GetWidth
        MOV      [EBX].TControl.DF.fSplitStartPos2.x, EAX
        POP      EAX
        CALL     TControl.GetHeight
        MOV      [EBX].TControl.DF.fSplitStartPos2.y, EAX
@@noSecondControl1:
        PUSH     [EBX].TControl.fHandle
        CALL     SetCapture
        {$IFDEF  USE_FLAGS}
        OR       [EBX].TControl.fFlagsG6, 1 shl G6_Dragging
        {$ELSE}
        OR       [EBX].TControl.fDragging, 1
        {$ENDIF}
        PUSH     0
        PUSH     100
        PUSH     $7B
        PUSH     [EBX].TControl.fHandle
        CALL     SetTimer
        LEA      EAX, [EBX].TControl.DF.fSplitStartPos
        PUSH     EAX
        CALL     GetCursorPos
        JMP      @@exit

@@noWM_LBUTTONDOWN:
        CMP      word ptr [EDX].TMsg.message, WM_LBUTTONUP
        JNE      @@noWM_LBUTTONUP
        XOR      EDX, EDX

        {$IFDEF USE_ASM_DODRAG}
        CALL     @@DoDrag
        {$ELSE}
        MOV      EAX, EBX
        CALL     DoDrag
        {$ENDIF}

        JMP      @@killtimer

@@noWM_LBUTTONUP:
        CMP      word ptr[EDX].TMsg.message, WM_TIMER
        JNE      @@exit
        {$IFDEF  USE_FLAGS}
        TEST     [EBX].TControl.fFlagsG6, 1 shl G6_Dragging
        {$ELSE}
        CMP      [EBX].TControl.fDragging, 0
        {$ENDIF}
        JE       @@exit
        PUSH     VK_ESCAPE
        CALL     GetAsyncKeyState
        TEST     EAX, EAX
        JGE      @@exit

        MOV      DL, 1
        {$IFDEF USE_ASM_DODRAG}
        CALL     @@DoDrag
        {$ELSE}
        MOV      EAX, EBX
        CALL     DoDrag
        {$ENDIF}

@@killtimer:
        {$IFDEF  USE_FLAGS}
        AND      [EBX].TControl.fFlagsG6, $7F //not(1 shl G6_Dragging)
        {$ELSE}
        MOV      [EBX].TControl.fDragging, 0
        {$ENDIF}
        PUSH     $7B
        PUSH     [EBX].TControl.fHandle
        CALL     KillTimer
        CALL     ReleaseCapture

@@exit:
        POP      EBX
        XOR      EAX, EAX
end;

function NewSplitterEx( AParent: PControl; MinSizePrev, MinSizeNext: Integer;
         EdgeStyle: TEdgeStyle ): PControl;
const int_IDC_SIZEWE = integer( IDC_SIZEWE );
asm
        PUSH     EBX
        PUSH     EAX  // AParent
        PUSH     ECX  // MinSizePrev
        PUSH     EDX  // MinSizeNext
        MOV      DL, EdgeStyle
        CALL     NewPanel
        XCHG     EBX, EAX
        POP      [EBX].TControl.DF.fSplitMinSize1
        POP      [EBX].TControl.DF.fSplitMinSize2
        {$IFDEF  USE_FLAGS}
        MOV      [EBX].TControl.fFlagsG5, 1 shl G5_IsSplitter
        {$ELSE}
        INC      [EBX].TControl.fIsSplitter
        {$ENDIF}
        XOR      EDX, EDX
        MOV      DL, 4
        MOV      EAX, [EBX].TControl.fBoundsRect.Left
        ADD      EAX, EDX
        MOV      [EBX].TControl.fBoundsRect.Right, EAX
        ADD      EDX, [EBX].TControl.fBoundsRect.Top
        MOV      [EBX].TControl.fBoundsRect.Bottom, EDX

        POP      ECX  // ECX = AParent
        JECXZ    @@noParent2
        MOV      EAX, [ECX].TControl.fChildren
        MOV      ECX, [EAX].TList.fCount
        CMP      ECX, 1
        JLE      @@noParent2

        MOV      EAX, [EAX].TList.fItems
        MOV      EAX, [EAX+ECX*4-8]
        MOV      CL, [EAX].TControl.fAlign
        PUSH     ECX
        MOV      AL, 1
        SHL      EAX, CL
        {$IFDEF PARANOIA} DB $A8, chkTop or chkBott {$ELSE} TEST AL, chkTop or chkBott {$ENDIF}
        MOV      EAX, int_IDC_SIZEWE
        JZ       @@TopBottom
        INC      EAX
@@TopBottom:
        PUSH     EAX
        PUSH     0
        CALL     LoadCursor
        MOV      [EBX].TControl.fCursor, EAX
        POP      EDX
        MOV      EAX, EBX
        CALL     TControl.SetAlign

@@noParent2:
        MOV      EAX, EBX
        MOV      EDX, offset[WndProcSplitter]
        CALL     TControl.AttachProc
        XCHG     EAX, EBX
        POP      EBX
        
{$IFDEF GRAPHCTL_XPSTYLES}
        PUSH     EAX
        MOV      EDX, offset[XP_Themes_For_Splitter]
        CALL     Attach_WM_THEMECHANGED
        POP      EAX
{$ENDIF}
end;

function NewGradientPanel( AParent: PControl; Color1, Color2: TColor ): PControl;
asm
        PUSH     ECX
        PUSH     EDX
        XOR      EDX, EDX
        CALL     NewLabel
        PUSH     EAX
        MOV      EDX, offset[WndProcGradient]
        CALL     TControl.AttachProc
        POP      EAX
        POP      [EAX].TControl.DF.fColor1
        POP      [EAX].TControl.DF.fColor2
        ADD      [EAX].TControl.fBoundsRect.Right, 40-64
        ADD      [EAX].TControl.fBoundsRect.Bottom, 40 - 22
end;

function NewGradientPanelEx( AParent: PControl; Color1, Color2: TColor;
                             Style: TGradientStyle; Layout: TGradientLayout ): PControl;
asm
        PUSH     ECX
        PUSH     EDX
        XOR      EDX, EDX
        CALL     NewLabel
        PUSH     EAX
        MOV      EDX, offset[WndProcGradientEx]
        CALL     TControl.AttachProc
        POP      EAX
        POP      [EAX].TControl.DF.fColor1
        POP      [EAX].TControl.DF.fColor2
        ADD      [EAX].TControl.fBoundsRect.Right, 40-100
        ADD      [EAX].TControl.fBoundsRect.Bottom, 40 - 22
        MOV      DL, Style
        MOV      [EAX].TControl.DF.fGradientStyle, DL
        MOV      DL, Layout
        MOV      [EAX].TControl.DF.fGradientLayout, DL
end;

const EditClass: array[0..4] of KOLChar = ( 'E','D','I','T',#0 );
function NewEditbox( AParent: PControl; Options: TEditOptions ) : PControl;
const int_IDC_IBEAM = integer( IDC_IBEAM );
const WS_flags = integer( WS_VISIBLE or WS_CHILD or WS_TABSTOP or WS_BORDER );
const WS_clear = integer( not(WS_VSCROLL or WS_HSCROLL) );
asm
        PUSH     EBX
        XCHG     EBX, EAX // EBX=AParent
        PUSH     EDX
        MOV      EAX, ESP
        XOR      ECX, ECX
        MOV      CL, 11
        MOV      EDX, offset [EditFlags]
        CALL     MakeFlags
        XCHG     ECX, EAX // ECX = Flags
        POP      EAX  // Options
        PUSH     EAX
        {$IFDEF PARANOIA} DB $A8, 8 {$ELSE} TEST AL, 8 {$ENDIF}
        JNZ      @@1
        AND      ECX, WS_clear
@@1:    OR       ECX, WS_flags
        PUSH     1
        {$IFDEF  PACK_COMMANDACTIONS}
        PUSH     [EditActions_Packed]
        {$ELSE}
        PUSH     offset[EditActions]
        {$ENDIF}
        MOV      EDX, offset[EditClass]
        XCHG     EAX, EBX
        CALL     _NewControl
        XCHG     EBX, EAX
        MOV      Byte Ptr [EBX].TControl.aAutoSzY, 6
        LEA      ECX, [EBX].TControl.fBoundsRect
        MOV      EDX, [ECX].TRect.Left
        ADD      EDX, 100
        MOV      [ECX].TRect.Right, EDX
        MOV      EDX, [ECX].TRect.Top
        ADD      EDX, 22
        MOV      [ECX].TRect.Bottom, EDX
        POP      EAX // Options
        {$IFDEF PARANOIA} DB $A8, 8 {$ELSE} TEST AL, 8 {$ENDIF}
        MOV      DL, $0D
        JZ       @@2
        ADD      [ECX].TRect.Right, 100
        ADD      [ECX].TRect.Bottom, 200 - 22
        MOV      DL, 1
        {$IFDEF  USE_FLAGS}
        OR       [EBX].TControl.fFlagsG5, 1 shl G5_IgnoreDefault
        {$ELSE}
        INC      [EBX].TControl.fIgnoreDefault
        {$ENDIF}
@@2:
        TEST     AH, 4
        JZ       @@3
        AND      DL, $FE
@@3:    MOV      [EBX].TControl.fLookTabKeys, DL
{$IFDEF UNICODE_CTRLS}
        MOV      EAX, EBX
        MOV      EDX, offset[WndProcUnicodeChars]
        CALL     TControl.AttachProc
{$ENDIF}
        XCHG     EAX, EBX
        POP      EBX
end;

{$IFNDEF USE_DROPDOWNCOUNT}
procedure ComboboxDropDown( Sender: PObj );
asm
        PUSH     EBX
        PUSH     ESI
        MOV      EBX, EAX
        CALL     TControl.GetItemsCount
        CMP      EAX, 1
        JGE      @@1
        MOV      AL, 1
@@1:    CMP      EAX, 8
        JLE      @@2
        XOR      EAX, EAX
        MOV      AL, 8
@@2:    XOR      ESI, ESI
        PUSH     SWP_NOMOVE or SWP_NOSIZE or SWP_NOZORDER or SWP_NOACTIVATE or SWP_NOREDRAW or SWP_SHOWWINDOW
        PUSH     ESI
        PUSH     ESI
        PUSH     SWP_NOMOVE or SWP_NOZORDER or SWP_NOACTIVATE or SWP_NOREDRAW or SWP_HIDEWINDOW
        PUSH     EAX
        MOV      EAX, EBX
        CALL     TControl.GetHeight
        POP      ECX
        INC      ECX
        IMUL     ECX
        INC      EAX
        INC      EAX
        PUSH     EAX
        MOV      EAX, EBX
        CALL     TControl.GetWidth
        PUSH     EAX
        INC      ESI
@@3:    XOR      EDX, EDX
        PUSH     EDX
        PUSH     EDX
        PUSH     EDX
        PUSH     [EBX].TControl.fHandle
        CALL     SetWindowPos
        DEC      ESI
        JZ       @@3
        {$IFDEF  EVENTS_DYNAMIC}
        MOV      EAX, [EBX].TControl.EV
        MOV      ECX, [EAX].TEvents.fOnDropDown.TMethod.Code
        {$ELSE}
        MOV      ECX, [EBX].TControl.EV.fOnDropDown.TMethod.Code
        {$ENDIF}
        {$IFDEF  NIL_EVENTS}
        JECXZ    @@exit
        {$ENDIF}
        {$IFDEF  EVENTS_DYNAMIC}
        MOV      EAX, [EAX].TEvents.fOnDropDown.TMethod.Data
        {$ELSE}
        MOV      EAX, [EBX].TControl.EV.fOnDropDown.TMethod.Data
        {$ENDIF}
        MOV      EDX, EBX
        CALL     ECX
@@exit: POP      ESI
        POP      EBX
end;
{$ENDIF}

const ComboboxClass: array[0..8] of KOLChar = ('C','O','M','B','O','B','O','X',#0 );
function NewCombobox( AParent: PControl; Options: TComboOptions ): PControl;
asm
  {$IFDEF GRAPHCTL_XPSTYLES}
  {$IFDEF UNICODE_CTRLS}
  PUSHAD
  CALL  InitCommonControls;
  POPAD
  {$ENDIF}
  {$ENDIF}
        PUSH     EDX
        PUSH     EAX
        PUSH     EDX
        MOV      EAX, ESP
        MOV      EDX, offset[ComboFlags]
        XOR      ECX, ECX
        MOV      CL, 10
        CALL     MakeFlags
        POP      EDX
        XCHG     ECX, EAX
        POP      EAX
        PUSH     1
        {$IFDEF  PACK_COMMANDACTIONS}
        PUSH     [ComboActions_Packed]
        {$ELSE}
        PUSH     offset[ComboActions]
        {$ENDIF}
        MOV      EDX, offset[ComboboxClass]
        OR       ECX, WS_VISIBLE or WS_CHILD or WS_VSCROLL or CBS_HASSTRINGS or WS_TABSTOP
        TEST     ECX, CBS_SIMPLE
        JNZ      @@O
        OR       ECX, CBS_DROPDOWN
@@O:
        CALL     _NewControl
        {$IFDEF  PACK_COMMANDACTIONS}
        MOV      EDX, [EAX].TControl.fCommandActions
        MOV      [EDX].TCommandActionsObj.aClear, offset[ClearCombobox]
        {$ENDIF}
        MOV      Byte Ptr [EAX].TControl.aAutoSzY, 6
        MOV      [EAX].TControl.PP.fCreateWndExt, offset[CreateComboboxWnd]
        OR       byte ptr [EAX].TControl.fClsStyle, CS_DBLCLKS
        ADD      [EAX].TControl.fBoundsRect.Right, 100-64
        ADD      [EAX].TControl.fBoundsRect.Bottom, 22-64
        MOV      CL, 1
        POP      EDX
        TEST     DL, 1
        JZ       @@exit
        MOV      CL, 3
@@exit:
        MOV      [EAX].TControl.fLookTabKeys, CL
        PUSH     EAX
        MOV      EDX, offset[ WndProcCombo ]
        CALL     TControl.AttachProc
        POP      EAX
end;

function WndProcParentResize( Self_: PControl; var Msg: TMsg; var Rslt: Integer ): Boolean;
asm
        CMP      word ptr [EDX].TMsg.message, CM_SIZE
        JNZ      @@exit
        PUSH     EAX
        PUSH     0
        PUSH     0
        PUSH     WM_SIZE
        PUSH     EAX
        CALL     TControl.Perform
        POP      EAX
        CALL     TControl.Invalidate
@@exit: XOR      EAX, EAX
end;

procedure InitCommonControlCommonNotify( Ctrl: PControl );
asm
    {$IFDEF   USE_FLAGS}
    OR        [EAX].TControl.fFlagsG5, 1 shl G5_IsCommonCtl
    {$ELSE}
    MOV       [EAX].TControl.fIsCommonControl, 1
    {$ENDIF}
    MOV       ECX, [EAX].TControl.fParent
    JECXZ     @@fin
    PUSH      ECX
    MOV       EDX, offset[WndProcCommonNotify]
    CALL      TControl.AttachProc
    POP       EAX
    MOV       EDX, offset[WndProcNotify]
    CALL      TControl.AttachProc
@@fin:
end;

function NewProgressbar( AParent: PControl ): PControl;
asm
        PUSH     1
        {$IFDEF  COMMANDACTIONS_OBJ}
        PUSH     PROGRESS_ACTIONS
        {$ELSE}
        PUSH     0
        {$ENDIF}
        MOV      EDX, offset[Progress_class]
        MOV      ECX, WS_CHILD or WS_VISIBLE
        CALL     _NewCommonControl
        LEA      EDX, [EAX].TControl.fBoundsRect
        MOV      ECX, [EDX].TRect.Left
        ADD      ECX, 300
        MOV      [EDX].TRect.Right, ECX
        MOV      ECX, [EDX].TRect.Top
        ADD      ECX, 20
        MOV      [EDX].TRect.Bottom, ECX
        XOR      EDX, EDX
        MOV      [EAX].TControl.fMenu, EDX
        MOV      [EAX].TControl.fTextColor, clHighlight
        {$IFDEF  COMMANDACTIONS_OBJ} //todo: should be used separate Actions record
        MOV      ECX, [EAX].TControl.fCommandActions
        MOV      [ECX].TCommandActionsObj.aSetBkColor, PBM_SETBKCOLOR
        {$ELSE}
        MOV      [EAX].TControl.fCommandActions.aSetBkColor, PBM_SETBKCOLOR
        {$ENDIF}
end;

function NewProgressbarEx( AParent: PControl; Options: TProgressbarOptions ): PControl;
asm
        PUSH     EDX
        CALL     NewProgressbar
        POP      ECX
        XOR      EDX, EDX
        SHR      ECX, 1
        JNC      @@notVert
        MOV      DL, 4
@@notVert:
        SHR      ECX, 1
        JNC      @@notSmooth
        INC      EDX
@@notSmooth:
        OR       [EAX].TControl.fStyle, EDX
end;

// by Galkov, Jun-2009
function WndProcNotify( Self_: PControl; var Msg: TMsg; var Rslt: Integer ): Boolean;
asm
       CMP      word ptr [EDX].TMsg.message, WM_NOTIFY
       JNE      @@ret_false
        PUSH     ECX
        PUSH     EDX
        push     eax
       MOV      ECX, [EDX].TMsg.lParam
       {$IFDEF USE_PROP}
       PUSH     offset[ID_SELF]
       PUSH     [ECX].TNMHdr.hwndFrom
       CALL     GetProp
       {$ELSE}
       PUSH     GWL_USERDATA
       PUSH     [ECX].TNMHdr.hwndFrom
       CALL     GetWindowLong
       {$ENDIF}
        pop      ecx
        POP      EDX
        TEST     EAX, EAX
        JZ       @@ret_false_ECX
        cmp      eax, ecx
        jz       @@ret_false_ECX
        MOV      ECX, [EAX].TControl.fHandle
        MOV      [EDX].TMsg.hwnd, ECX
        POP      ECX
       JMP      TControl.EnumDynHandlers
@@ret_false_ECX:
        POP      ECX
@@ret_false:
       XOR      EAX, EAX
end;

function WndProcCommonNotify( Self_: PControl; var Msg: TMsg; var Rslt: Integer ): Boolean;
asm
        CMP      word ptr [EDX].TMsg.message, WM_NOTIFY
        JNE      @@ret_false
        PUSH     EBX
        MOV      EBX, [EDX].TMsg.lParam
        MOV      EDX, [EBX].TNMHdr.code

@@chk_nm_click:
        XOR      ECX, ECX
        CMP      EDX, NM_CLICK
        JZ       @@click
        CMP      EDX, NM_RCLICK
        JNE      @@chk_killfocus
        {$IFDEF  USE_FLAGS}
        MOV      CL, 1 shl G6_RightClick
        {$ELSE}
        INC      ECX
        {$ENDIF}
@@click:
        {$IFDEF  USE_FLAGS}
        AND      [EAX].TControl.fFlagsG6, not(1 shl G6_RightClick)
        OR       [EAX].TControl.fFlagsG6, CL
        {$ELSE}
        MOV      [EAX].TControl.fRightClick, CL
        {$ENDIF}

        {$IFDEF  EVENTS_DYNAMIC}
        MOV      ECX, [EAX].TControl.EV
        MOV      ECX, [ECX].TEvents.fOnClick.TMethod.Code
        {$ELSE}
        MOV      ECX, [EAX].TControl.EV.fOnClick.TMethod.Code
        {$ENDIF}
        {$IFDEF  NIL_EVENTS}
        JECXZ    @@fin_false
        {$ENDIF}
        {$IFDEF  EVENTS_DYNAMIC}
        MOV      EDX, [EAX].TControl.EV
        MOV      EDX, [EDX].TEvents.fOnClick.TMethod.Data
        {$ELSE}
        MOV      EDX, [EAX].TControl.EV.fOnClick.TMethod.Data
        {$ENDIF}
        JMP      @@fin_event

{$IFDEF NIL_EVENTS}
@@fin_false:
        POP      EBX
@@ret_false:
        XOR      EAX, EAX
        RET
{$ENDIF}

@@chk_killfocus:
        CMP      EDX, NM_KILLFOCUS
        JNE      @@chk_setfocus
        {$IFDEF  EVENTS_DYNAMIC}
        MOV      EAX, [EAX].TControl.EV
        MOV      ECX, [EAX].TEvents.fOnLeave.TMethod.Code
        {$ELSE}
        MOV      ECX, [EAX].TControl.EV.fOnLeave.TMethod.Code
        {$ENDIF}
        {$IFDEF  NIL_EVENTS}
        JECXZ    @@fin_false
        {$ENDIF}
        {$IFDEF  EVENTS_DYNAMIC}
        MOV      EDX, [EAX].TEvents.fOnLeave.TMethod.Data
        {$ELSE}
        MOV      EDX, [EAX].TControl.EV.fOnLeave.TMethod.Data
        {$ENDIF}
        JMP      @@fin_event
@@chk_setfocus:
        CMP      EDX, NM_RETURN
        JE       @@set_focus
        CMP      EDX, NM_SETFOCUS
        JNE      @@fin_false

@@set_focus:
        {$IFDEF  EVENTS_DYNAMIC}
        MOV      EAX, [EAX].TControl.EV
        MOV      ECX, [EAX].TEvents.fOnEnter.TMethod.Code
        {$ELSE}
        MOV      ECX, [EAX].TControl.EV.fOnEnter.TMethod.Code
        {$ENDIF}
        {$IFDEF  NIL_EVENTS}
        JECXZ    @@fin_false
        {$ENDIF}
        {$IFDEF  EVENTS_DYNAMIC}
        MOV      EDX, [EAX].TEvents.fOnEnter.TMethod.Data
        {$ELSE}
        MOV      EDX, [EAX].TControl.EV.fOnEnter.TMethod.Data
        {$ENDIF}

@@fin_event:
        XCHG     EAX, EDX
        CALL     ECX
{$IFnDEF NIL_EVENTS}
@@fin_false:
{$ENDIF}
        POP      EBX
{$IFnDEF NIL_EVENTS}
@@ret_false:
{$ENDIF}
        //MOV      AL, 1
        XOR      EAX, EAX
end;

procedure ApplyImageLists2Control( Sender: PControl );
asm
        PUSHAD
        XCHG     ESI, EAX
        {$IFDEF  COMMANDACTIONS_OBJ}
        MOV      ECX, [ESI].TControl.fCommandActions
        MOVZX    ECX, [ECX].TCommandActionsObj.aSetImgList      
        {$ELSE}
        MOVZX    ECX, [ESI].TControl.fCommandActions.aSetImgList
        {$ENDIF}
        JECXZ    @@fin
        MOV      EBP, ECX
        XOR      EBX, EBX
        MOV      BL, 32
        XOR      EDI, EDI
@@loo:
        MOV      EAX, ESI
        MOV      EDX, EBX
        CALL     TControl.GetImgListIdx
        TEST     EAX, EAX
        JZ       @@nx
        CALL     TImageList.GetHandle
        PUSH     EAX
        PUSH     EDI
        PUSH     EBP
        PUSH     ESI
        CALL     TControl.Perform
@@nx:
        INC      EDI
        SHR      EBX, 1
        JZ       @@fin
        CMP      BL, 16
        JGE      @@loo
        XOR      EBX, EBX
        JMP      @@loo
@@fin:
        POPAD
end;

procedure ApplyImageLists2ListView( Sender: PControl );
asm
        PUSHAD

        XCHG     ESI, EAX
        PUSH     dword ptr [ESI].TControl.DF.fLVOptions
        MOV      EAX, ESP
        MOV      EDX, offset[ListViewFlags]
        XOR      ECX, ECX
        MOV      CL, 25
        CALL     MakeFlags
        POP      ECX
        PUSH     ECX

        MOV      EDX, [ESI].TControl.fStyle
        //AND      DH, 3
        AND      DX, not $403F
        OR       EDX, EAX

        MOVZX    EAX, [ESI].TControl.DF.fLVStyle
        OR       EDX, [EAX*4 + offset ListViewStyles]

        MOV      EAX, ESI
        CALL     TControl.SetStyle

        MOV      EAX, ESP
        MOV      EDX, offset[ListViewExFlags]
        XOR      ECX, ECX
        MOV      CL, 23
        CALL     MakeFlags
        POP      EDX
        PUSH     EAX
        PUSH     $3FFF
        PUSH     LVM_SETEXTENDEDLISTVIEWSTYLE
        PUSH     ESI
        CALL     TControl.Perform

        POPAD
        CALL     ApplyImageLists2Control
end;

function NewListView( AParent: PControl; Style: TListViewStyle; Options: TListViewOptions;
                      ImageListSmall, ImageListNormal, ImageListState: PImageList ): PControl;
asm
        PUSH     EDX
        PUSH     ECX
        MOVZX    EDX, DL
        MOV      ECX, [EDX*4 + offset ListViewStyles]
        OR       ECX, LVS_SHAREIMAGELISTS or WS_CHILD or WS_VISIBLE or WS_TABSTOP
        MOV      EDX, offset[WC_LISTVIEW]
        PUSH     1
        {$IFDEF  PACK_COMMANDACTIONS}
        PUSH     [ListViewActions_Packed]
        {$ELSE}
        PUSH     offset[ListViewActions]
        {$ENDIF}
        CALL     _NewCommonControl

        {$IFDEF  PACK_COMMANDACTIONS}
        MOV      EDX, [EAX].TControl.fCommandActions
        MOV      [EDX].TCommandActionsObj.aClear, offset[ClearListView]
        {$ENDIF}

        MOV      EDX, ESP
        PUSH     EAX
        XCHG     EAX, EDX
        MOV      EDX, offset ListViewFlags
        XOR      ECX, ECX
        MOV      CL, 25
        CALL     MakeFlags
        XCHG     EDX, EAX
        POP      EAX
        MOV      ECX, [EAX].TControl.fStyle
        AND      ECX, not LVS_TYPESTYLEMASK
        OR       EDX, ECX
        MOV      [EAX].TControl.fStyle, EDX

        POP      [EAX].TControl.DF.fLVOptions
        POP      EDX
        MOV      [EAX].TControl.DF.fLVStyle, DL
        MOV      [EAX].TControl.PP.fCreateWndExt, offset[ApplyImageLists2ListView]
        ADD      [EAX].TControl.fBoundsRect.Right, 200-64
        ADD      [EAX].TControl.fBoundsRect.Bottom, 150-64
        MOV      ECX, [ImageListState]
        XOR      EDX, EDX
        PUSHAD
        CALL     TControl.SetImgListIdx
        POPAD
        MOV      ECX, [ImageListSmall]
        MOV      DL, 16
        PUSHAD
        CALL     TControl.SetImgListIdx
        POPAD
        MOV      ECX, [ImageListNormal]
        ADD      EDX, EDX
        PUSH     EAX
        CALL     TControl.SetImgListIdx
        POP      EAX
        MOV      [EAX].TControl.DF.fLVTextBkColor, clWindow
        XOR      EDX, EDX
        INC      EDX
        MOV      [EAX].TControl.fLookTabKeys, DL
end;

function NewTreeView( AParent: PControl; Options: TTreeViewOptions;
                      ImgListNormal, ImgListState: PImageList ): PControl;
asm     //cmd    //opd
        PUSH     EBX
        PUSH     ECX
        PUSH     EAX
        PUSH     EDX
        MOV      EAX, ESP
        MOV      EDX, offset[TreeViewFlags]
        XOR      ECX, ECX
        MOV      CL, 13
        CALL     MakeFlags
        POP      EDX
        OR       EAX, WS_VISIBLE or WS_CHILD or WS_TABSTOP
        XCHG     ECX, EAX
        POP      EAX
        MOV      EDX, offset[WC_TREEVIEW]
        PUSH     1
        {$IFDEF  PACK_COMMANDACTIONS}
        PUSH     [TreeViewActions_Packed]
        {$ELSE}
        PUSH     offset[TreeViewActions]
        {$ENDIF}
        CALL     _NewCommonControl
        MOV      EBX, EAX
        {$IFDEF  PACK_COMMANDACTIONS}
        MOV      EDX, [EBX].TControl.fCommandActions
        MOV      [EDX].TCommandActionsObj.aClear, offset[ClearTreeView]
        {$ENDIF}
        MOV      [EBX].TControl.PP.fCreateWndExt, offset[ApplyImageLists2Control]
        MOV      [EBX].TControl.fColor, clWindow
        MOV      EDX, offset[WndProcTreeView]
        CALL     TControl.AttachProc
        ADD      [EBX].TControl.fBoundsRect.Right, 150-64
        ADD      [EBX].TControl.fBoundsRect.Bottom, 200-64
        MOV      EAX, EBX
        XOR      EDX, EDX
        MOV      DL, 32
        POP      ECX // ImageListNormal
        CALL     TControl.SetImgListIdx
        MOV      EAX, EBX
        XOR      EDX, EDX
        MOV      ECX, [ImgListState]
        CALL     TControl.SetImgListIdx
        MOV      byte ptr [EBX].TControl.fLookTabKeys, 1
        XCHG     EAX, EBX
        POP      EBX
end;

function WndProcTabControl( Self_: PControl; var Msg: TMsg; var Rslt: Integer ): Boolean;
asm     //cmd    //opd
{$IFDEF OLD_ALIGN}
        PUSH     EBP
        PUSH     EBX
        PUSH     ESI
        PUSH     EDI
        MOV      EBX, EAX
        CMP      word ptr [EDX].TMsg.message, WM_NOTIFY
        JNZ      @@chk_WM_SIZE
        MOV      EDX, [EDX].TMsg.lParam
//!!!
        CMP      word ptr [EDX].TNMHdr.code, TCN_SELCHANGING
        JNZ      @@chk_TCN_SELCHANGE
        CALL     TControl.GetCurIndex
        MOV      [EBX].TControl.fCurIndex, EAX
        JMP      @@ret_false
@@chk_TCN_SELCHANGE:
        CMP      word ptr [EDX].TNMHdr.code, TCN_SELCHANGE
        JNZ      @@ret_false

        CALL     TControl.GetCurIndex
        XCHG     EDI, EAX
        CMP      EDI, [EBX].TControl.fCurIndex
        PUSHFD   // WasActive = ZF

        MOV      [EBX].TControl.FCurIndex, EDI

        MOV      EAX, EBX
        CALL     TControl.GetItemsCount
        XCHG     ESI, EAX // ESI := Self_.Count

@@loo:  DEC      ESI
        JS       @@e_loo
        MOV      EDX, ESI
        MOV      EAX, EBX
        CALL     TControl.GetPages

        CMP      ESI, EDI
        PUSH     EAX
        SETZ     DL
        CALL     TControl.SetVisible
        POP      EAX
        CMP      ESI, EDI
        JNE      @@nx_loo
        CALL     TControl.BringToFront
@@nx_loo:
        JMP      @@loo
@@e_loo:
        POPFD
        JZ       @@ret_false

        {$IFDEF  EVENTS_DYNAMIC}
        MOV      EAX, [EBX].TControl.EV
        MOV      ECX, [EAX].TEvents.fOnSelChange.TMethod.Code
        {$ELSE}
        MOV      ECX, [EBX].TControl.EV.fOnSelChange.TMethod.Code
        {$ENDIF}
        JECXZ    @@ret_false
        MOV      EDX, EBX
        {$IFDEF  EVENTS_DYNAMIC}
        MOV      EAX, [EAX].TEvents.fOnSelChange.TMethod.Data
        {$ELSE}
        MOV      EAX, [EBX].TControl.EV.fOnSelChange.TMethod.Data
        {$ENDIF}
        CALL     ECX
        JMP      @@ret_false
@@chk_WM_SIZE:
        CMP      word ptr [EDX].TMsg.message, WM_SIZE
        JNE      @@ret_false
        ADD      ESP, -16
        PUSH     ESP
        PUSH     [EBX].TControl.fHandle
        CALL     Windows.GetClientRect
        PUSH     ESP
        PUSH     0
        PUSH     TCM_ADJUSTRECT
        PUSH     EBX
        CALL     TControl.Perform
        MOV      EAX, EBX
        CALL     TControl.GetItemsCount
        XCHG     ESI, EAX
@@loo2:
        DEC      ESI
        JS       @@e_loo2
        MOV      EDX, ESI
        MOV      EAX, EBX
        CALL     TControl.GetPages
        MOV      EDX, ESP
        CALL     TControl.SetBoundsRect
        JMP      @@loo2
@@e_loo2:
        ADD      ESP, 16
@@ret_false:
        XOR      EAX, EAX
        POP      EDI
        POP      ESI
        POP      EBX
        POP      EBP
{$ELSE NEW_ALIGN}
        PUSH     EBX
        MOV      EBX, EAX
        CMP      word ptr [EDX].TMsg.message, WM_NOTIFY
        JNZ      @@chk_WM_SIZE
        MOV      EDX, [EDX].TMsg.lParam

        CMP      word ptr [EDX].TNMHdr.code, TCN_SELCHANGING
        JNZ      @@chk_TCN_SELCHANGE
        CALL     TControl.GetCurIndex
        MOV      [EBX].TControl.fCurIndex, EAX
        JMP      @@ret_false
@@chk_TCN_SELCHANGE:
        CMP      word ptr [EDX].TNMHdr.code, TCN_SELCHANGE
        JNZ      @@ret_false
        CALL     TControl.GetCurIndex
        MOV      EDX, [EBX].TControl.fCurIndex
        MOV      [EBX].TControl.fCurIndex, EAX
        CMP      EAX, EDX
        PUSHFD   // WasActive = ZF
        BT       EDX,31
        JBE      @@00
        MOV      EAX, EBX
        CALL     TControl.GetPages
        XOR      EDX,EDX
        CALL     TControl.SetVisible
@@00:
        MOV      EDX, [EBX].TControl.fCurIndex
        MOV      EAX, EBX
        CALL     TControl.GetPages
        MOV      DL,1
        PUSH     EAX
        CALL     TControl.SetVisible
        POP      EAX
        CALL     TControl.BringToFront
        POPFD
        JZ       @@ret_false
        {$IFDEF  EVENTS_DYNAMIC}
        MOV      EAX, [EBX].TControl.EV
        MOV      ECX, [EAX].TEvents.fOnSelChange.TMethod.Code
        {$ELSE}
        MOV      ECX, [EBX].TControl.EV.fOnSelChange.TMethod.Code
        {$ENDIF}
        {$IFDEF  NIL_EVENTS}
        JECXZ    @@ret_false
        {$ENDIF}
        MOV      EDX, EBX
        {$IFDEF  EVENTS_DYNAMIC}
        MOV      EAX, [EAX].TEvents.fOnSelChange.TMethod.Data
        {$ELSE}
        MOV      EAX, [EBX].TControl.EV.fOnSelChange.TMethod.Data
        {$ENDIF}
        CALL     ECX
        JMP      @@ret_false
@@chk_WM_SIZE:
        CMP      word ptr [EDX].TMsg.message, WM_SIZE
        JNE      @@ret_false
        SUB      ESP, 10h
        PUSH     ESP
        PUSH     [EBX].TControl.fHandle
        CALL     Windows.GetClientRect
        MOV      EAX,[ESP].TRect.Right
        MOV      [EBX].TControl.fClientRight, AL
        MOV      EAX,[ESP].TRect.Bottom
        MOV      [EBX].TControl.fClientBottom, AL
        PUSH     ESP
        PUSH     0
        PUSH     TCM_ADJUSTRECT
        PUSH     EBX
        CALL     TControl.Perform
        POP      EAX
        MOV      [EBX].TControl.fClientLeft, AL
        POP      EAX
        MOV      [EBX].TControl.fClientTop, AL
        POP      EAX
        SUB      [EBX].TControl.fClientRight, AL
        POP      EAX
        SUB      [EBX].TControl.fClientBottom, AL
@@ret_false:
        XOR      EAX, EAX
        POP      EBX
{$ENDIF}
end;

{$IFNDEF OLD_ALIGN}
function NewTabEmpty( AParent: PControl; Options: TTabControlOptions;
         ImgList: PImageList ): PControl;
const lenf=high(TabControlFlags); //+++
asm     //cmd    //opd
        PUSH     EBX
        MOV      EBX, EAX
        PUSH     ECX
        PUSH     EDX
        MOV      EAX, ESP
        MOV      EDX, offset[TabControlFlags]
        XOR      ECX, ECX
        MOV      CL, lenf
        CALL     MakeFlags
        TEST     byte ptr [ESP], 4
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
        POP      ECX //Options
        TEST     ECX, 2 shl (tcoBorder - 1)
        JNZ      @@borderfixed
        AND      [EBX].TControl.fExStyle, not WS_EX_CLIENTEDGE
@@borderfixed:
        MOV      EDX, offset[WndProcTabControl]
        CALL     TControl.AttachProc
        ADD      [EBX].TControl.fBoundsRect.Right, 100-64
        ADD      [EBX].TControl.fBoundsRect.Bottom, 100-64
        POP      ECX //ImgList
        JECXZ    @@2
        XCHG     EAX, ECX
        CALL     TImageList.GetHandle
        PUSH     EAX
        PUSH     0
        PUSH     TCM_SETIMAGELIST
        PUSH     EBX
        CALL     TControl.Perform
@@2:
        MOV      byte ptr [EBX].TControl.fLookTabKeys, 1
        XCHG     EAX, EBX
        POP      EBX
end;
{$ENDIF}

{$IFNDEF NOT_USE_RICHEDIT}

const RichEdit50W: array[0..11] of AnsiChar = ('R','i','c','h','E','d','i','t','5','0','W',#0 );
function NewRichEdit( AParent: PControl; Options: TEditOptions ): PControl;
const deltaChr = 24; // sizeof( TCharFormat2 ) - sizeof( RichEdit.TCharFormat );
      deltaPar = sizeof( TParaFormat2 ) - sizeof( RichEdit.TParaFormat );
asm
        PUSHAD
        CALL     OleInit
        TEST     EAX, EAX
        POPAD
        JZ       @@new1
        MOV      [RichEditIdx], 0
        CALL     NewRichEdit1
        MOV      byte ptr [EAX].TControl.DF.fCharFmtDeltaSz, deltaChr
        MOV      byte ptr [EAX].TControl.DF.fParaFmtDeltaSz, deltaPar
        RET
@@new1: CALL     NewRichEdit1
end;

(*
function WndProc_RE_LinkNotify( Self_: PControl; var Msg: TMsg; var Rslt: Integer ): Boolean;
asm
        CMP      word ptr [EDX].TMsg.message, WM_NOTIFY
        JNE      @@ret_false
        MOV      EDX, [EDX].TMsg.lParam
        CMP      [EDX].TNMHdr.code, EN_LINK
        JNE      @@ret_false
        PUSH     EBX
        PUSH     EDX
        XCHG     EBX, EAX
        XOR      EAX, EAX
        MOV      [ECX], EAX
        {$IFDEF UNICODE_CTRLS}
        ADD      ESP, -2040
        {$ELSE}
        ADD      ESP, -1020
        {$ENDIF}
        PUSH     EAX
        PUSH     ESP
        PUSH     [EDX].TENLink.chrg.cpMax
        PUSH     [EDX].TENLink.chrg.cpMin
        PUSH     ESP
        PUSH     0
        PUSH     EM_GETTEXTRANGE
        PUSH     EBX
        CALL     TControl.Perform
        LEA      EAX, [EBX].TControl.fREUrl

        POP      EDX
        POP      ECX
        DEC      EDX
        CMP      ECX, EDX
        POP      ECX
        MOV      EDX, ESP
        JLE      @@1
        CMP      byte ptr [EDX+1], 0
        JNZ      @@1
        // система вернула текст как unicode
        {$IFDEF UNICODE_CTRLS}
        CALL     System.@WStrFromPWChar // TODO: not need ecx
        {$ELSE not UNICODE_CTRLS}
        {$IFDEF  _D2}
        CALL     LStrFromPWChar
        {$ELSE}
          {$IFDEF _D2009orHigher}
          XOR      ECX, ECX // TODO: fixme
          {$ENDIF}
        CALL     System.@LStrFromPWChar
        {$ENDIF}
        {$ENDIF UNICODE_CTRLS}
        JMP      @@2
@@1:
        // система вернула текст как обычную строку
        {$IFDEF UNICODE_CTRLS}
        CALL     System.@WStrFromPChar
        {$ELSE not UNICODE_CTRLS}
        {$IFDEF _D2009orHigher}
        XOR      ECX, ECX // TODO: fixme
        {$ENDIF}
        CALL     System.@LStrFromPChar
        {$ENDIF UNICODE_CTRLS}
@@2:
        {$IFDEF UNICODE_CTRLS}
        ADD      ESP, 2044
        {$ELSE not UNICODE_CTRLS}
        ADD      ESP, 1024
        {$ENDIF UNICODE_CTRLS}
        POP      EDX
        MOV      ECX, [EDX].TENLink.msg
        {$IFDEF  EVENTS_DYNAMIC}
        MOV      EAX, [EBX].TControl.EV
        LEA      EAX, [EAX].TEvents.fOnREOverURL
        {$ELSE}
        LEA      EAX, [EBX].TControl.EV.fOnREOverURL
        {$ENDIF}
        CMP      ECX, WM_MOUSEMOVE
        JE       @@Url_event
        //LEA      EAX, [EBX].TControl.EV.fOnREUrlClick
        ADD      EAX, 8
        CMP      ECX, WM_LBUTTONDOWN
        JE       @@Url_Event
        CMP      ECX, WM_RBUTTONDOWN
        JNE      @@after_Url_event
@@Url_event:
        MOV      ECX, [EAX].TMethod.Code
        {$IFDEF  NIL_EVENTS}
        JECXZ    @@after_Url_event
        {$ENDIF}
        MOV      EDX, EBX
        MOV      EAX, [EAX].TMethod.Data
        CALL     ECX
@@after_Url_event:
        POP      EBX
        MOV      AL, 1
        RET
@@ret_false:
        XOR      EAX, EAX
end;
*)
{$ENDIF NOT_USE_RICHEDIT}

function OleInit: Boolean;
asm
        MOV      ECX, [OleInitCount]
        INC      ECX
        LOOP     @@init1
        PUSH     ECX
        CALL     OleInitialize
        TEST     EAX, EAX
        MOV      AL, 0
        JNZ      @@exit
@@init1:
        INC      [OleInitCount]
        MOV      AL, 1
@@exit:
end;

procedure OleUnInit;
asm
        MOV      ECX, [OleInitCount]
        JECXZ    @@exit
        DEC      [OleInitCount]
        JNZ      @@exit
        CALL     OleUninitialize
@@exit:
end;

procedure TControl.Init;
const
  IniStyle = WS_VISIBLE or WS_CLIPCHILDREN or WS_CLIPSIBLINGS or
            WS_CAPTION or WS_SYSMENU or WS_MINIMIZEBOX or WS_MAXIMIZEBOX or
            WS_BORDER or WS_THICKFRAME;
asm     //cmd    //opd
        PUSH     EBX
        PUSH     EDI
        MOV      EBX, EAX
        {$IFDEF  CALL_INHERITED}
        CALL     TObj.Init // for now, TObj.Init do nothing for Delphi 4 and higher
        {$ENDIF}
        {$IFDEF USE_GRAPHCTLS}
        MOV      [EBX].PP.fDoInvalidate, offset[InvalidateWindowed]
        {$ENDIF}

        {$IFDEF  OLD_EVENTS_MODEL}
            MOV      EAX, offset WndProcDummy
            LEA      EDI, [EBX].PP.fPass2DefProc
            STOSD    // fPass2DefProc := WndProcDummy
            STOSD    // fOnDynHandlers := WndProcDummy
            STOSD    // fWndProcKeybd := WndProcDummy
            STOSD    // fControlClick := WndProcDummy - similar to DefWindowProc
            STOSD    // fAutoSize := WndProcDummy - similar to DefWindowProc
            LEA      EDI, [EBX].PP.fWndProcResizeFlicks
            STOSD

            MOV      [EBX].PP.fWndFunc, offset WndFunc
        {$ELSE  NEW_EVENTS_MODEL}
            {$IFDEF  EVENTS_DYNAMIC}
                XOR      ECX, ECX
                CMP      DWORD PTR[EmptyEvents].TEvents.fOnMessage.TMethod.Code, ECX
                JNZ      @@a2
                MOV      CL, idx_LastEvent+1
        @@a1:   MOVZX    EDX, byte ptr [ECX+InitEventsTable-1]
                AND      DL, $0F
                MOV      EDX, dword ptr [EDX*4 + DummyProcTable]
                MOV      dword ptr [EmptyEvents+ECX*8-8], EDX
                LOOP     @@a1
        @@a2:
                MOV      EDX, offset[EmptyEvents]
                MOV      [EBX].EV, EDX
                MOV      CL, idx_LastProc - idx_LastEvent
        @@a3:
                MOVZX    EDX, byte ptr [ECX+InitEventsTable-1]
                SHR      EDX, 4
                MOV      EDX, dword ptr [EDX*4 + DummyProcTable]
                MOV      dword ptr [EBX+ECX*4-4].PP, EDX
                LOOP     @@a3
            {$ELSE}
                XOR      ECX, ECX
                MOV      CL,  idx_LastEvent+1
        @@1:
                MOVZX    EDX, byte ptr [ECX+InitEventsTable-1]
                PUSH     EDX
                AND      DL, $0F
                MOV      EDX, [EDX*4 + DummyProcTable]
                MOV      dword ptr [EBX+ECX*8-8].EV, EDX
                POP      EDX
                SHR      EDX, 4
                CMP      ECX, idx_LastProc - idx_LastEvent + 1
                JGE      @@2

                MOV      EDX, [EDX*4 + DummyProcTable]
                MOV      dword ptr [EBX+ECX*4-4].PP, EDX
        @@2:
                LOOP     @@1
            {$ENDIF}
        {$ENDIF  NEW_EVENTS_MODEL}

        {$IFDEF  COMMANDACTIONS_OBJ} //--- moved to _NewWindowed
        //----   MOV      EDX, [EBX].fCommandActions
        //----   MOV      [EDX].TCommandActionsObj.aClear, offset[ClearText]
        {$ELSE}
        //----   MOV      [EBX].fCommandActions.aClear, offset[ClearText]
        {$ENDIF}
        {$IFDEF  USE_FLAGS}
        {$ELSE}
        INC      [EBX].fWindowed
        {$ENDIF}
        MOV      [EBX].fColor, clBtnFace
        {$IFDEF SYSTEMCOLORS_DELPHI}
        MOV      [EBX].fTextColor, clWindowText and $FF
        {$ELSE}
        MOV      [EBX].fTextColor, clWindowText
        {$ENDIF}

        MOV      byte ptr [EBX].fMargin, 2
        OR       dword ptr [EBX].fCtl3D_child, 3

        {$IFDEF SMALLEST_CODE}
        {$ELSE}
        DEC      byte ptr [EBX].fAlphaBlend    // has no effect until AlphaBlend changed
        {$ENDIF}
        MOV      byte ptr[EBX].fClsStyle, CS_OWNDC
        MOV      [EBX].fStyle, IniStyle
        INC      dword ptr[EBX].fExStyle+2
        {$IFDEF  USE_FLAGS}
        //AND      [EBX].fStyle.f3_Style, not(1 shl F3_Disabled)
        OR       [EBX].fStyle.f3_Style, (1 shl F3_Visible)
        {$ELSE}
        DEC      WORD PTR [EBX].fEnabled
        {$ENDIF}

        LEA      EDI, [EBX].fDynHandlers
        MOV      EBX, offset[NewList]
        CALL     EBX
        STOSD
        CALL     EBX
        STOSD

        POP      EDI
        POP      EBX
end;

procedure CallTControlInit( Ctl: PControl );
begin
  Ctl.Init;
end;

procedure TControl.InitParented( AParent: PControl );
const IStyle = WS_VISIBLE or WS_CLIPCHILDREN or WS_CLIPSIBLINGS or
            WS_CAPTION or WS_SYSMENU or WS_MINIMIZEBOX or WS_MAXIMIZEBOX or
            WS_BORDER or WS_THICKFRAME;
      IExStyle = WS_EX_CONTROLPARENT;
      IClsStyle = CS_OWNDC;
      int_IDC_ARROW = integer( IDC_ARROW );
asm
        PUSH      EAX
        PUSH      EDX
        //CALL      CallTControlInit
        mov EDX, [EAX]
        call dword ptr [EDX]

        POP       EDX
        POP       EAX
        TEST      EDX, EDX
        JZ        @@0
        MOV       ECX, [EDX].fColor
        MOV       [EAX].fColor, ECX
@@0:
        CALL      SetParent
end;

destructor TControl.Destroy;
asm
        PUSH     EBX
        PUSH     ESI
        MOV      EBX, EAX
        CALL     TControl.ParentForm
        XCHG     ECX, EAX
        JECXZ    @@cur_ctl_removed
        MOV      EDX, EBX
        XOR      EDX, [ECX].TControl.DF.fCurrentControl
        JNE      @@cur_ctl_removed
        MOV      [ECX].TControl.DF.fCurrentControl, EDX
@@cur_ctl_removed:

        MOV      ECX, [EBX].fHandle
        JECXZ    @@wndhidden
        PUSH     SW_HIDE
        PUSH     ECX
        CALL     ShowWindow
@@wndhidden:

        MOV      EAX, EBX
        CALL     Final
        {$IFDEF USE_AUTOFREE4CHILDREN}
        {$ELSE}
        MOV      EAX, EBX
        CALL     DestroyChildren
        {$ENDIF}

        {$IFDEF  USE_FLAGS}
        BTS      DWORD PTR [EBX].fFlagsG2, G2_Destroying
        JC       @@destroyed
        {$ELSE}
        XOR      ECX, ECX
        CMP      [EBX].fDestroying, CL
        JNZ      @@destroyed
        INC      [EBX].fDestroying
        {$ENDIF  USE_FLAGS}

        {$IFDEF USE_AUTOFREE4CONTROLS}
        XOR      EAX, EAX
        XCHG     EAX, [EBX].fCanvas
        CALL     TObj.RefDec
        {$ELSE}
        PUSH     EBX
        LEA      ESI, [EBX].fFont
        MOV      BL, 3
@@free_font_brush_canvas:
        XOR      ECX, ECX
        XCHG     ECX, [ESI]
        LODSD
        XCHG     EAX, ECX
        CALL     TObj.RefDec
        DEC      BL
        JNZ      @@free_font_brush_canvas
        POP      EBX
        {$ENDIF}

        MOV      EAX, [EBX].fCustomObj
        CALL     TObj.RefDec

        MOV      EAX, [EBX].fHandle
        TEST     EAX, EAX
        JZ       @@free_fields

        {$IFNDEF USE_AUTOFREE4CONTROLS}
        {$IFNDEF NEW_MENU_ACCELL}
        XOR      ECX, ECX
        XCHG     ECX, [EBX].fAccelTable
        JECXZ    @@accelTable_destroyed
        PUSH     ECX
        CALL     DestroyAcceleratorTable
@@accelTable_destroyed:
        {$ENDIF}
        MOV      EAX, [EBX].fMenuObj
        CALL     TObj.RefDec
@@destroy_img_list:
        XOR      EAX, EAX
        XCHG     EAX, [EBX].fImageList
        TEST     EAX, EAX
        JZ       @@img_list_destroyed
        CALL     TObj.RefDec
        JMP      @@destroy_img_list
@@img_list_destroyed:
        {$ENDIF}

        MOV      ECX, [EBX].DF.fIcon
        JECXZ    @@icoremoved
        INC      ECX
        JZ       @@icoremoved
        {$IFDEF  USE_FLAGS}
        TEST     [EBX].fFlagsG1, (1 shl G1_IconShared)
        JNZ      @@icoremoved
        {$ELSE}
        CMP      [EBX].fIconShared, 0
        JNZ      @@icoremoved
        {$ENDIF  USE_FLAGS}
        DEC      ECX
        PUSH     ECX
        CALL     DestroyIcon
@@icoremoved:

        PUSH     [EBX].fHandle
        CALL     IsWindow
        TEST     EAX, EAX
        JZ       @@destroy2
        (* -- moved to WM_NCDESTROY handler - VK + Alexey Kirov
        {$IFDEF USE_PROP}
        PUSH     offset[ID_SELF] //* Remarked By M.Gerasimov
        PUSH     [EBX].fHandle   //* unremarked to prevent problems with progress bar
        CALL     RemoveProp
        {$ELSE}
        PUSH     0
        PUSH     GWL_USERDATA
        PUSH     [EBX].fHandle
        CALL     SetWindowLong
        {$ENDIF}
        *)
        {$IFDEF  USE_fNCDestroyed}
        CMP      [EBX].fNCDestroyed, 0
        JNZ      @@destroy2
        {$ENDIF  USE_fNCDestroyed}
        PUSH     [EBX].fHandle
        CALL     DestroyWindow
@@destroy2:
        XOR      EAX, EAX
        MOV      [EBX].fHandle, EAX

@@free_fields:
        PUSH     0
        {$IFDEF  USE_FLAGS}
        TEST     [EBX].fFlagsG6, 1 shl G6_CtlClassNameChg
        JZ       @@notFreeCtlClsName
        {$ELSE}
        MOVZX    ECX, [EBX].fCtlClsNameChg
        JECXZ    @@notFreeCtlClsName
        {$ENDIF}
        PUSH     [EBX].fControlClassName
@@notFreeCtlClsName:
        MOV      ECX, [EBX].fCustomData
        JECXZ    @@notFreeCustomData
        PUSH     ECX
@@notFreeCustomData:
@@FreeFieldsLoop:
        POP      ECX
        JECXZ    @@endFreeFieldsLoop
        XCHG     EAX, ECX
        CALL     System.@FreeMem
        JMP      @@FreeFieldsLoop
@@endFreeFieldsLoop:

        XOR      ECX, ECX
        XCHG     ECX, [EBX].fTmpBrush
        JECXZ    @@tmpBrush_deleted
        PUSH     ECX
        CALL     DeleteObject
@@tmpBrush_deleted:

        MOV      ECX, [EBX].fParent
        JECXZ    @@removed_from_parent
        CMP      [ECX].DF.fCurrentControl, EBX
        JNE      @@removefromParent
        XOR      EAX, EAX
        MOV      [ECX].DF.fCurrentControl, EAX
@@removefromParent:
        {$IFDEF USE_AUTOFREE4CHILDREN}
        PUSH     ECX
        {$ENDIF}
        MOV      EAX, [ECX].fChildren
        MOV      EDX, EBX
        CALL     TList.Remove
        {$IFDEF USE_AUTOFREE4CHILDREN}
        POP      EAX
        MOV      EDX, EBX
        CALL     TControl.RemoveFromAutoFree
        {$ENDIF}
@@removed_from_parent:

        {$IFDEF USE_AUTOFREE4CONTROLS}
        LEA      ESI, [EBX].fDynHandlers
        LODSD
        CALL     TObj.RefDec
        LODSD         // fChildren
        CALL     TObj.RefDec
        {$ELSE}
        PUSH     EBX
        LEA      ESI, [EBX].fDynHandlers
        MOV      BL, 5
@@freeloo:
        LODSD
        CALL     TObj.RefDec
        DEC      BL
        JNZ      @@freeloo
        POP      EBX
        {$ENDIF}

        LEA      EAX, [EBX].fCaption
        {$IFDEF UNICODE_CTRLS}
        CALL     System.@WStrClr
        {$ELSE}
        CALL     System.@LStrClr
        {$ENDIF}
        XCHG     EAX, EBX
        CALL     TObj.Destroy
@@destroyed:
        POP      ESI
        POP      EBX
end;

procedure TControl.SetEnabled( Value: Boolean );
asm
        PUSH     EBX
        MOV      EBX, EAX
        MOVZX    EDX, DL
        PUSH     EDX
        CALL     GetEnabled
        POP      EDX
        CMP      AL, DL
        JZ       @@exit
        {$IFDEF  USE_FLAGS}
        {$ELSE}
        MOV      [EBX].fEnabled, DL
        {$ENDIF  USE_FLAGS}
        TEST     EDX, EDX
        JNZ      @@andnot
        OR       [EBX].fStyle.f3_Style, (1 shl F3_Disabled)
        JMP      @@1
@@andnot:
        AND      [EBX].fStyle.f3_Style, not(1 shl F3_Disabled)
@@1:
        MOV      ECX, [EBX].fHandle
        JECXZ    @@2

        PUSH     EDX
        PUSH     ECX
        CALL     EnableWindow

@@2:
        XCHG     EAX, EBX
        CALL     Invalidate

@@exit:
        POP      EBX
end;

{function TControl.GetParentWindow: HWnd;
asm
        MOV       ECX, [EAX].fHandle
        JECXZ     @@1
        PUSH      EAX
          PUSH      GW_OWNER
          PUSH      EAX
          CALL      GetWindow
        POP       ECX
        TEST      EAX, EAX
        JZ        @@0
        RET
@@0:    XCHG      EAX, ECX
@@1:
        MOV       EAX, [EAX].fParent
        TEST      EAX, EAX
        JNZ       TControl.GetWindowHandle
end;}

function WndProcMouse(Self_: PControl; var Msg: TMsg; var Rslt: Integer): Boolean;
asm
         PUSH      EBX
         PUSH      ESI
         XCHG      EBX, EAX

         XOR       ECX, ECX // Rslt not used. ECX <= Result = 0
         MOV       EAX, [EDX].TMsg.message
         SUB       AH, WM_MOUSEFIRST shr 8
         CMP       EAX, $20A - WM_MOUSEFIRST //WM_MOUSELAST - WM_MOUSEFIRST
         JA        @@exit

         PUSH      dword ptr [EDX].TMsg.lParam // prepare X, Y

         PUSHAD
           PUSH      VK_MENU
           CALL      GetKeyState
           ADD       EAX, EAX
         POPAD

         XCHG        EAX, EDX
           MOV       EAX, [EAX].TMsg.wParam

           JNC       @@noset_MKALT
           {$IFDEF PARANOIA} DB $0C, MK_ALT {$ELSE} OR AL, MK_ALT {$ENDIF}
@@noset_MKALT:

         PUSH      EAX             // prepare Shift

         {$IFDEF   EVENTS_DYNAMIC}
         MOV       EAX, [EBX].TControl.EV
         LEA       ESI, [EAX].TEvents.fOnMouseDown
         {$ELSE}
         LEA       ESI, [EBX].TControl.EV.fOnMouseDown
         {$ENDIF}
         CALL      dword ptr [EDX*4 + @@jump_table]

@@call_evnt:

         PUSH      ECX             // prepare Button, StopHandling
         MOV       ECX, ESP        // ECX = @MouseData

         {$IFDEF   NIL_EVENTS}
         CMP       word ptr [ESI].TMethod.Code+2, 0
         JZ        @@after_call
         {$ENDIF}

         MOV       EDX, EBX        // EDX = Self_
         MOV       EAX, [ESI].TMethod.Data      // EAX = Target_
         CALL      dword ptr [ESI].TMethod.Code

@@after_call:
         POP       ECX
         POP       EDX
         POP       EDX
         MOV       CL, CH           // Result := StopHandling

@@exit:
         XCHG      EAX, ECX
         POP       ESI
         POP       EBX
         RET

@@jump_table:
         DD Offset[@@MMove],Offset[@@LDown],Offset[@@LUp],Offset[@@LDblClk]
         DD Offset[@@RDown],Offset[@@RUp],Offset[@@RDblClk]
         DD Offset[@@MDown],Offset[@@MUp],Offset[@@MDblClk],Offset[@@MWheel]

@@MDown: INC       ECX
@@RDown: INC       ECX
@@LDown: INC       ECX
         RET

@@MUp:   INC       ECX
@@RUp:   INC       ECX
@@LUp:   INC       ECX
         LODSD
         LODSD
         RET

@@MMove: ADD       ESI, 16
         RET

@@MDblClk: INC     ECX
@@RDblClk: INC     ECX
@@LDblClk: INC     ECX
         ADD       ESI, 24
         RET

@@MWheel:ADD       ESI, 32
end;

{$IFnDEF USE_GRAPHCTLS}
{$IFnDEF NEW_MODAL}
{$IFnDEF USE_MDI}
function TControl.WndProc( var Msg: TMsg ): Integer;
asm     //cmd    //opd
        PUSH     EBX
        PUSH     ESI
        PUSH     EDI
        PUSH     EBP
        //MOV      ESI, EAX
        XCHG     ESI, EAX
        MOV      EDI, EDX
        //CALL     TControl.RefInc
        MOV      EBP, [ESI].TControl.PP.fPass2DefProc

        XOR      EAX, EAX
        CMP      EAX, [EDI].TMsg.hWnd
        JE       @@1
        CMP      EAX, [ESI].TControl.fHandle
        JNE      @@1
        {$IFDEF USE_GRAPHCTLS}
        {$IFDEF USE_FLAGS}
        TEST     [ESI].TControl.fFlagsG6, 1 shl G6_GraphicCtl
        {$ELSE}
        CMP      [ESI].TControl.fWindowed, AL
        {$ENDIF}
        JNE      @@1
        {$ENDIF}
        MOV      EAX, [EDI].TMsg.hWnd
        MOV      [ESI].TControl.fHandle, EAX
@@1:
        XOR      eax, eax

        CMP      [AppletRunning], AL
        JZ       @@dyn2
        MOV      ECX, [Applet]
        JECXZ    @@dyn2
        CMP      ECX, ESI
        JE       @@dyn2

        CALL     @@onmess

@@dyn2: MOV      ECX, ESI
        CALL     @@onmess

        MOV      EBX, [ESI].TControl.PP.fOnDynHandlers
        MOV      EAX, ESI
        CALL     @@callonmes

//**********************************************************
        MOVZX    EAX, word ptr [EDI].TMsg.message
        CMP      AX, WM_CLOSE
        JNZ      @@chk_WM_DESTROY

        CMP      ESI, [Applet]
        JZ       @@postquit
        MOV      EAX, ESI
        CALL     IsMainWindow
        TEST     AL, AL
        JZ       @@calldef
@@postquit:
        PUSH     0
        CALL     PostQuitMessage
        MOV      byte ptr [AppletTerminated], 1
        JMP      @@calldef
//********************************************************** Added By M.Gerasimov
@@chk_WM_DESTROY:
        {$IFnDEF SMALLER_CODE}
        MOV      EDX, [EDI].TMsg.hWnd
        {$ENDIF  SMALLER_CODE}
        CMP      AX, WM_DESTROY
        JNE      @@chk_WM_NCDESTROY

        {$IFnDEF SMALLER_CODE}
        CMP      EDX, [ESI].TControl.fHandle
        JNE      @@chk_WM_NCDESTROY
        {$ENDIF  SMALLER_CODE}

        {$IFDEF  USE_FLAGS}
        OR       [ESI].TControl.fFlagsG2, (1 shl G2_BeginDestroying)
        {$ELSE}
        MOV      [ESI].TControl.fBeginDestroying, AL
        {$ENDIF}
        JMP      @@calldef
//**********************************************************
@@chk_WM_NCDESTROY:
        CMP      AX, WM_NCDESTROY
        JNE      @@chk_WM_SIZE // @@chk_CM_RELEASE
//********************************************************** Added By M.Gerasimov
        {$IFnDEF SMALLER_CODE}
        CMP      EDX, [ESI].TControl.fHandle
        JNE      @@chk_WM_SIZE
        {$ENDIF  SMALLER_CODE}

        {$IFDEF USE_PROP}
        PUSH     offset[ID_SELF]
        PUSH     [ESI].fHandle
        CALL     RemoveProp
        {$ELSE}
        PUSH     0
        PUSH     GWL_USERDATA
        PUSH     [ESI].fHandle
        CALL     SetWindowLong
        {$ENDIF}
        JMP      @@calldef
//**********************************************************
@@return0:
        XOR      EAX, EAX
        JMP      @@exit // WM_NCDESTROY and CM_RELEASE
                        // is not a subject to pass it
                        // to fPass2DefProc
@@onmess:
        {$IFDEF  EVENTS_DYNAMIC}
        MOV      EAX, [ECX].TControl.EV
        MOV      EBX, [EAX].TEvents.fOnMessage.TMethod.Code
        MOV      EAX, [EAX].TEvents.fOnMessage.TMethod.Data
        {$ELSE}
        MOV      EAX, [ECX].TControl.EV.fOnMessage.TMethod.Data
        MOV      EBX, [ECX].TControl.EV.fOnMessage.TMethod.Code
        {$ENDIF}
@@callonmes:
        {$IFDEF  NIL_EVENTS}
        TEST     EBX, EBX
        JZ       @@ret
        {$ENDIF}
@@onmess1:
        PUSH     0

        MOV      EDX, EDI
        MOV      ECX, ESP
        CALL     EBX
        TEST     AL, AL

        POP      EAX
        JZ       @@ret
        POP      EDX // pop retaddr
        JMP      @@pass2defproc

//**************************************************************
@@chk_WM_SIZE:
        CMP      AX, WM_SIZE
        JNE      @@chk_WM_SYSCOMMAND //@@chk_WM_SHOWWINDOW

        MOV      EDX, EDI
        MOV      EAX, ESI
        CALL     TControl.CallDefWndProc
        PUSH     EAX

    {$IFDEF OLD_ALIGN}
        {$IFDEF  USE_FLAGS}
        TEST     [ESI].TControl.fFlagsG3, (1 shl G3_IsForm)
        {$ELSE}
        CMP      [ESI].TControl.fIsForm, 0
        {$ENDIF}
        JNZ      @@doGlobalAlignSelf
        MOV      EAX, [ESI].TControl.fParent
        CALL     dword ptr [Global_Align]
@@doGlobalAlignSelf:
    {$ENDIF}
        MOV      EAX, ESI
        CALL     dword ptr [Global_Align]
        JMP      @@popeax_exit  // fPass2DefProc not needed, CallDefWndProc already called

//**************************************************************
@@chk_WM_SYSCOMMAND:
        CMP      AX, WM_SYSCOMMAND
        JNE      @@chk_WM_SETFOCUS

        MOV      EAX, [EDI].TMsg.wParam
        {$IFDEF PARANOIA} DB $24, $F0 {$ELSE} AND AL, $F0 {$ENDIF}
        CMP      AX, SC_MINIMIZE
        JNE      @@calldef

        MOV      EAX, ESI
        CALL     TControl.IsMainWindow
        TEST     AL, AL
        JZ       @@calldef

        CMP      ESI, [Applet]
        JE       @@calldef

        PUSH     0
        PUSH     SC_MINIMIZE
        PUSH     WM_SYSCOMMAND
        MOV      EAX, [Applet]
        PUSH     [EAX].TControl.fHandle
        CALL     PostMessage
@@ret_0:
        JMP      @@0pass2defproc

//***************************************************************
@@chk_WM_SETFOCUS:
        CMP      AX, WM_SETFOCUS
        JNE      @@chk_WM_CTLCOLOR //@@chk_WM_SETCURSOR

        MOV      EAX, ESI
        CALL     TControl.DoSetFocus
        TEST     AL, AL
        JZ       @@0pass2defproc

        INC      [ESI].TControl.fClickDisabled

        MOV      EAX, ESI
        MOV      EDX, EDI
        CALL     TControl.CallDefWndProc

        DEC      [ESI].TControl.fClickDisabled
        JMP      @@exit

//**************************************************************
@@chk_WM_CTLCOLOR:
        MOV      EDX, EAX
        SUB      DX, WM_CTLCOLORMSGBOX
        CMP      DX, WM_CTLCOLORSTATIC-WM_CTLCOLORMSGBOX
        JA       @@chk_WM_COMMAND

        PUSH     [EDI].TMsg.lParam
        PUSH     [EDI].TMsg.wParam
        ADD      AX, CN_BASE //+WM_CTLCOLORMSGBOX
        PUSH     EAX
        PUSH     [EDI].TMsg.lParam
        CALL     SendMessage
        JMP      @@pass2defproc

//**************************************************************
@@chk_WM_COMMAND:
        CMP      AX, WM_COMMAND
        JNE      @@chk_WM_KEY

        {$IFDEF USE_PROP}
        PUSH     offset[ID_SELF]
        PUSH     [EDI].TMsg.lParam
        CALL     GetProp
        {$ELSE}
        PUSH     GWL_USERDATA
        PUSH     [EDI].TMsg.lParam
        CALL     GetWindowLong
        {$ENDIF}
        TEST     EAX, EAX
        JZ       @@calldef

        PUSH     [EDI].TMsg.lParam
        PUSH     [EDI].TMsg.wParam
        PUSH     CM_COMMAND
        PUSH     [EDI].TMsg.lParam
        CALL     SendMessage
        JMP      @@pass2defproc

//**************************************************************
@@chk_WM_KEY:
        MOV      EDX, EAX
        SUB      DX, WM_KEYFIRST
        CMP      DX, WM_KEYLAST-WM_KEYFIRST
        JA       @@calldef //@@chk_CM_EXECPROC
        {$IFDEF KEY_PREVIEW}
        {$IFDEF  USE_FLAGS}
        TEST     [ESI].TControl.fFlagsG4, 1 shl G4_Pushed
        {$ELSE}
        CMP      [ESI].TControl.fKeyPreviewing, 0
        {$ENDIF}
        JNE      @@in_focus
        {$ENDIF KEY_PREVIEW}

        CALL     GetFocus
        //--- CMP      EAX, [ESI].TControl.fFocusHandle
        //--- JE       @@in_focus
        CMP      EAX, [ESI].TControl.fHandle
        {$IFDEF USE_GRAPHCTLS}
        JE       @@in_focus
        CMP      [ESI].fWindowed, 0
        {$ENDIF}
        JNE      @@0pass2defproc

@@in_focus:
        {$IFDEF KEY_PREVIEW}
            {$IFDEF  USE_FLAGS}
            AND      [ESI].TControl.fFlagsG4, not(1 shl G4_Pushed)
            {$ELSE}
            MOV      [ESI].TControl.fKeyPreviewing, 0
            {$ENDIF}
        {$ENDIF KEY_PREVIEW}
        PUSH     EAX

        MOV      ECX, ESP
        MOV      EDX, EDI
        MOV      EAX, ESI
        CALL     dword ptr [fGlobalProcKeybd]
        TEST     AL, AL
        JNZ      @@to_exit

        MOV      ECX, ESP
        MOV      EDX, EDI
        MOV      EAX, ESI
        CALL     [ESI].PP.fWndProcKeybd
        TEST     AL, AL
@@to_exit:
        POP      EAX
        JNZ      @@pass2defproc

        PUSH     VK_CONTROL
        CALL     GetKeyState
        XCHG     EBX, EAX
        PUSH     VK_MENU
        CALL     GetKeyState
        OR       EAX, EBX
        JS       @@calldef

        CMP      word ptr [EDI].TMsg.message, WM_CHAR
        JNE      @@to_fGotoControl

        CMP      byte ptr [EDI].TMsg.wParam, 9
        JE       @@clear_wParam
        JMP      @@calldef

@@to_fGotoControl:
        MOV      EAX, ESI
        CALL     TControl.ParentForm
        TEST     EAX, EAX
        JZ       @@calldef

        MOV      ECX, [EAX].PP.fGotoControl
        {$IFDEF  NIL_EVENTS}
        JECXZ    @@calldef
        {$ENDIF}

        MOV      EBX, ECX
        CMP      [EDI].TMsg.message, WM_KEYDOWN
        SETNE    CL
        CMP      [EDI].TMsg.message, WM_SYSKEYDOWN
        SETNE    CH
        AND      CL, CH
        MOV      EDX, [EDI].TMsg.wParam
        MOV      EAX, ESI
        CALL     EBX
        TEST     AL, AL
        JZ       @@calldef

@@clear_wParam:
        XOR      EAX, EAX
        MOV      [EDI].TMsg.wParam, EAX
        JMP      @@pass2defproc

@@calldef:
        MOV      EAX, ESI
        MOV      EDX, EDI
        CALL     TControl.CallDefWndProc
        JMP      @@exit

@@0pass2defproc:
        XOR      EAX, EAX
@@pass2defproc:
        PUSH     EAX
@@1pass2defproc:
        CMP      [AppletTerminated], 0 //
        JNZ      @@popeax_exit         // uncommented 25-Oct-2003
        {$IFDEF  USE_fNCDestroyed}
        CMP      [ESI].fNCDestroyed, 0 //
        JNZ      @@popeax_exit         //
        {$ENDIF  USE_fNCDestroyed}

        MOV      ECX, ESP
        MOV      EAX, ESI
        MOV      EDX, EDI
        CALL     EBP
@@popeax_exit:
        POP      EAX

@@exit:
        {XCHG     ESI, EAX
        CALL     TControl.RefDec
        XCHG     EAX, ESI}

        POP      EBP
        POP      EDI
        POP      ESI
        POP      EBX
@@ret:
end;
{$ENDIF no_USE_MDI}
{$ENDIF no NEW_MODAL}
{$ENDIF no USE_GRAPHCTLS}

procedure TControl.SetClsStyle( Value: DWord );
asm     //cmd    //opd
        CMP      EDX, [EAX].TControl.fClsStyle
        JE       @@exit
        MOV      [EAX].TControl.fClsStyle, EDX
        MOV      ECX, [EAX].TControl.fHandle
        JECXZ    @@exit
        PUSH     EDX
        PUSH     GCL_STYLE
        PUSH     ECX
        CALL     SetClassLong
@@exit:
end;

procedure TControl.SetStyle( Value: DWord );
const SWP_FLAGS = SWP_NOACTIVATE or SWP_NOMOVE or SWP_NOSIZE or
                 SWP_NOZORDER or SWP_FRAMECHANGED;
asm
        CMP      EDX, [EAX].fStyle
        JZ       @@exit
        MOV      [EAX].fStyle, EDX
        MOV      ECX, [EAX].fHandle
        JECXZ    @@exit

        PUSH     EAX

        PUSH     SWP_FLAGS
        XOR      EAX, EAX
        PUSH     EAX
        PUSH     EAX
        PUSH     EAX
        PUSH     EAX
        PUSH     EAX
        PUSH     ECX

        PUSH     EDX
        PUSH     GWL_STYLE
        PUSH     ECX
        CALL     SetWindowLong

        CALL     SetWindowPos

        POP      EAX
        CALL     Invalidate
@@exit:
end;

procedure TControl.SetExStyle( Value: DWord );
const SWP_FLAGS = SWP_NOACTIVATE or SWP_NOMOVE or SWP_NOSIZE or
                 SWP_NOZORDER or SWP_FRAMECHANGED;
asm
        CMP      EDX, [EAX].fExStyle
        JZ       @@exit
        MOV      [EAX].fExStyle, EDX
        MOV      ECX, [EAX].fHandle
        JECXZ    @@exit

        PUSH     EAX

        PUSH     SWP_FLAGS
        XOR      EAX, EAX
        PUSH     EAX
        PUSH     EAX
        PUSH     EAX
        PUSH     EAX
        PUSH     EAX
        PUSH     ECX

        PUSH     EDX
        PUSH     GWL_EXSTYLE
        PUSH     ECX
        CALL     SetWindowLong

        CALL     SetWindowPos

        POP      EAX
        CALL     Invalidate
@@exit:
end;

procedure TControl.SetCursor( Value: HCursor );
asm     //cmd    //opd
        PUSH     EBX
        MOV      EBX, EAX
        PUSH     EDX
        LEA      EDX, WndProcSetCursor
        CALL     TControl.AttachProc
        POP      EDX

        CMP      EDX, [EBX].TControl.fCursor
        JE       @@exit
        MOV      [EBX].TControl.fCursor, EDX
        MOV      ECX, [EBX].TControl.fHandle
        JECXZ    @@exit
        TEST     EDX, EDX                      //YS
        JE       @@exit                        //YS
        MOV      ECX, [ScreenCursor]
        INC      ECX
        LOOP     @@exit

        PUSH     EDX
        PUSH     EAX
        PUSH     EAX
        PUSH     ESP
        CALL     GetCursorPos
        MOV      EDX, ESP
        MOV      ECX, EDX
        MOV      EAX, EBX
        CALL     Screen2Client
        ADD      ESP, -16
        MOV      EDX, ESP
        MOV      EAX, EBX
        CALL     TControl.ClientRect
        MOV      EDX, ESP
        LEA      EAX, [ESP+16]
        CALL     PointInRect
        ADD      ESP, 24
        TEST     AL, AL
        JZ       @@fin
        CALL     Windows.SetCursor
        PUSH     EAX
@@fin:  POP      EAX
@@exit:
        POP      EBX
end;

procedure TControl.SetIcon( Value: HIcon );
asm     //cmd    //opd
        CMP      EDX, [EAX].TControl.DF.fIcon
        JE       @@exit
        MOV      [EAX].TControl.DF.fIcon, EDX
        INC      EDX
        JZ       @@1
        DEC      EDX
@@1:
        PUSH     EDX
        PUSH     1 //ICON_BIG
        PUSH     WM_SETICON
        PUSH     EAX
        CALL     Perform
        TEST     EAX, EAX
        JZ       @@exit
        PUSH     EAX
        CALL     DestroyIcon
@@exit:
end;

procedure TControl.SetMenu( Value: HMenu );
asm
        PUSH     EBX
        XCHG     EBX, EAX
        CMP      [EBX].fMenu, EDX
        JZ       @@exit
        PUSH     EDX
        MOV      ECX, [EBX].fMenuObj
        JECXZ    @@no_free_menuctl
        {$IFDEF USE_AUTOFREE4CONTROLS}
        PUSH     EDX
        MOV      EAX, EBX
        CALL     TControl.RemoveFromAutoFree
        POP      EAX
        {$ELSE}
        XCHG     EAX, EDX
        {$ENDIF}
        CALL     TObj.RefDec
@@no_free_menuctl:
        MOV      ECX, [EBX].fMenu
        JECXZ    @@no_destroy
        PUSH     ECX
        CALL     DestroyMenu
@@no_destroy:
        POP      EDX
        MOV      [EBX].fMenu, EDX
        MOV      ECX, [EBX].fHandle
        JECXZ    @@exit
        PUSH     EDX
        PUSH     ECX
        CALL     Windows.SetMenu
@@exit:
        POP      EBX
end;

procedure TControl.DoAutoSize;
asm
  {$IFDEF   NIL_EVENTS}
  MOV  ECX, [EAX].PP.fAutoSize
  JECXZ     @@exit
  PUSH      ECX
  {$ELSE    not NIL_EVENTS}
  PUSH      [EAX].PP.fAutoSize
  {$ENDIF}
@@exit:
end;

procedure TControl.SetCaption( const Value: KOLString );
asm
        PUSH    EBX
        XCHG    EBX, EAX
        LEA     EAX, [EBX].fCaption
        {$IFDEF UNICODE_CTRLS}
        CALL    System.@WStrAsg
        {$ELSE}
        CALL    System.@LStrAsg
        {$ENDIF}

        MOV     ECX, [EBX].fHandle
        JECXZ   @@0
        PUSH    [EBX].TControl.fCaption
        PUSH    0
        PUSH    WM_SETTEXT
        PUSH    ECX
        {$IFDEF UNICODE_CTRLS}
        CALL    SendMessageW
        {$ELSE}
        CALL    SendMessage
        {$ENDIF}
@@0:
        {$IFDEF USE_FLAGS}
        TEST    [EBX].fFlagsG1, (1 shl G1_IsStaticControl)
        JNZ     @@1
        {$ELSE}
        MOVZX   ECX, byte ptr [EBX].fIsStaticControl
        INC     ECX
        LOOP    @@1
        {$ENDIF}
        MOV     EAX, EBX
        CALL    Invalidate
@@1:
        XCHG    EAX, EBX
@@exit: POP     EBX
        PUSH    [EAX].PP.fAutoSize
@@exit_2:
end;

function TControl.GetVisible: Boolean;
asm
        //CALL    UpdateWndStyles
        {MOV     ECX, [EAX].fHandle
        JECXZ   @@check_fStyle
          PUSH  EAX
        PUSH    ECX
        CALL    IsWindowVisible
        TEST    EAX, EAX
          POP   EAX
        JMP     @@checked // Z if not visible
        }
@@check_fStyle:
        TEST    byte ptr [EAX].fStyle.f3_Style, 1 shl F3_Visible // WS_VISIBLE shr 3
@@checked:
        {$IFDEF USE_FLAGS}
        SETNZ   AL
        {$ELSE}
        SETNZ   DL
        MOV     [EAX].fVisible, DL
        XCHG    EAX, EDX
        {$ENDIF}
end;

function TControl.Get_Visible: Boolean;
asm     //      //
        {$IFDEF USE_FLAGS}
        CALL    GetVisible
        {$ELSE}
        MOV     ECX, [EAX].fHandle
        JECXZ   @@ret_fVisible
        {$IFDEF USE_FLAGS}
        TEST    [EAX].fFlagsG3, 1 shl G3_IsControl
        {$ELSE}
        CMP     [EAX].fIsControl, 0
        {$ENDIF}
        JNZ     @@ret_fVisible
        PUSH    EAX
        PUSH    ECX
        CALL    IsWindowVisible
        XCHG    EDX, EAX
        POP     EAX
        {$IFDEF USE_FLAGS}
        SHL     DL, F3_Visible
        AND     [EAX].TControl.fStyle.f3_Style, not(1 shl F3_Visible)
        OR      [EAX].TControl.fStyle.f3_Style, DL
        {$ELSE}
        MOV     [EAX].fVisible, DL
        {$ENDIF}
@@ret_fVisible:
        {$IFDEF USE_FLAGS}
        TEST    [EAX].fStyle.f3_Style, (1 shl F3_Visible)
        SETNZ   AL
        {$ELSE}
        MOVZX   EAX, [EAX].fVisible
        {$ENDIF}
        {$ENDIF USE_FLAGS}
end;

procedure TControl.Set_Visible( Value: Boolean );
const wsVisible = $10;
asm
    {$IFDEF OLD_ALIGN}
        PUSH     EBX
        PUSH     ESI
        //MOV      ESI, EAX
        XCHG     ESI, EAX
        MOVZX    EBX, DL
        {CALL     Get_Visible
        CMP      AL, BL
        JE       @@reset_fCreateHidden}

        MOV      AL, byte ptr [ESI].fStyle + 3
        TEST     EBX, EBX
        JZ       @@reset_WS_VISIBLE
        {$IFDEF  USE_FLAGS}
        OR       AL, 1 shl F3_Visible
        {$ELSE}
        OR       AL, wsVisible
        {$ENDIF}
        PUSH     SW_SHOW
        JMP      @@store_Visible
@@reset_WS_VISIBLE:
        {$IFDEF  USE_FLAGS}
        AND      AL, not(1 shl F3_Visible)
        {$ELSE}
        AND      AL, not wsVisible
        {$ENDIF}
        PUSH     SW_HIDE

@@store_Visible:
        MOV      byte ptr [ESI].fStyle + 3, AL
        {$IFDEF  USE_FLAGS}
        {$ELSE}
        MOV      [ESI].fVisible, BL
        {$ENDIF}
        MOV      ECX, [ESI].fHandle
        JECXZ    @@after_showwindow

        PUSH     ECX
        CALL     ShowWindow
        PUSH     ECX
@@after_showwindow:
        POP      ECX

        MOV      EAX, [ESI].fParent
        CALL     dword ptr [Global_Align]

@@chk_align_Self:
        TEST     EBX, EBX
        JZ       @@reset_fCreateHidden
        MOV      EAX, ESI
        CALL     dword ptr [Global_Align]


@@reset_fCreateHidden:
        MOV      ECX, [ESI].fHandle
        JECXZ    @@exit
        TEST     BL, BL
        JNZ      @@exit
        {$IFDEF  USE_FLAGS}
        AND      [ESI], not(1 shl G4_CreateHidden)
        {$ELSE}
        MOV      [ESI].fCreateHidden, BL { +++ }
        {$ENDIF}
@@exit:
        POP      ESI
        POP      EBX
    {$ELSE NEW_ALIGN}
        AND      byte ptr [EAX].fStyle.f3_Style, not(1 shl F3_Visible)
        TEST     DL,DL
        JZ       @@0
        OR       byte ptr [EAX].fStyle.f3_Style, (1 shl F3_Visible)
@@0:
        {$IFDEF  USE_FLAGS}
        {$ELSE}
        MOV      [EAX].fVisible, DL
        {$ENDIF  USE_FLAGS}
        MOV      ECX, [EAX].fHandle
        JECXZ    @@exit
        PUSH     EAX
        JZ       @@1
        CALL     dword ptr [Global_Align]
        POP      EAX
        PUSH     SW_SHOW
        PUSH     [EAX].fHandle
        CALL     ShowWindow
@@exit:
        RET
@@1:
        {$IFDEF  USE_FLAGS}
        AND      [EAX].fFlagsG4, not(1 shl G4_CreateHidden)
        {$ELSE}
        MOV      [EAX].fCreateHidden, DL  // = 0
        {$ENDIF}
        PUSH     SW_HIDE
        PUSH     ECX
        CALL     ShowWindow
        POP      EAX
        CALL     dword ptr [Global_Align]
    {$ENDIF}
end;

procedure TControl.SetVisible( Value: Boolean );
asm
    {$IFDEF  USE_FLAGS}
    OR       [EAX].TControl.fFlagsG4, 1 shl G4_CreateVisible
    {$ELSE}
    MOV      [EAX].TControl.fCreateVisible, 1
    {$ENDIF}
    CALL     TControl.Set_Visible
end;

function TControl.GetBoundsRect: TRect;
asm
        PUSH      ESI
        PUSH      EDI
        LEA       ESI, [EAX].fBoundsRect
        MOV       EDI, EDX

        PUSH      EDX

        MOVSD
        MOVSD
        MOVSD
        MOVSD

        POP       EDI

        XCHG      ESI, EAX
        MOV       ECX, [ESI].fHandle
        JECXZ     @@exit

        PUSH      EDI
        PUSH      ECX
        CALL      GetWindowRect

        {$IFDEF   USE_FLAGS}
        TEST      [ESI].fFlagsG3, (1 shl G3_IsControl) or (1 shl G3_IsMDIChild)
        {$ELSE}
        MOV       AL, [ESI].fIsControl
        OR        AL, [ESI].fIsMDIChild
        {$ENDIF}
        JZ        @@storeBounds

@@chk_Parent:
        MOV       EAX, ESI
        CALL      TControl.GetParentWindow

        TEST      EAX, EAX
        JZ        @@exit

        XOR       EDX, EDX
        PUSH      EDX
        PUSH      EDX
        PUSH      ESP
        PUSH      EAX
        CALL      Windows.ClientToScreen
        
        POP       EAX
        NEG       EAX
        POP       ECX
        NEG       ECX
        PUSH      ECX
        PUSH      EAX
        PUSH      EDI
        CALL      OffsetRect

@@storeBounds:
        XCHG      ESI, EDI
        LEA       EDI, [EDI].fBoundsRect
        MOVSD
        MOVSD
        MOVSD
        MOVSD

@@exit:
        POP       EDI
        POP       ESI
end;

procedure HelpGetBoundsRect;
asm
        POP       ECX
        ADD       ESP, - size_TRect
        MOV       EDX, ESP
        PUSH      ECX
        PUSH      EAX
        CALL      TControl.GetBoundsRect
        POP       EAX
end;

procedure TControl.SetBoundsRect( const Value: TRect );
const swp_flags = SWP_NOZORDER or SWP_NOACTIVATE;
asm
        PUSH      EDI
        MOV       EDI, EAX

        PUSH      ESI
        MOV       ESI, EDX

        CALL      HelpGetBoundsRect

        MOV       EAX, ESI
        MOV       EDX, ESP
        CALL      RectsEqual
        TEST      AL, AL
        JNZ       @@exit

        POP       EDX   // left
        POP       ECX   // top
        POP       EAX   // right
        PUSH      EAX
        PUSH      ECX
        PUSH      EDX

        SUB       EAX, EDX  // EAX = width
        CMP       EDX, [ESI].TRect.Left
        {$IFDEF   USE_FLAGS}
        {$ELSE}
        MOV       DL, 0
        {$ENDIF}
        JNE       @@11
@@1:    CMP       ECX, [ESI].TRect.Top
        JE        @@2
@@11:
        {$IFDEF   USE_FLAGS}
        OR        [EDI].fFlagsG2, (1 shl G2_ChangedPos)
        {$ELSE}
        OR        DL, 2
        OR        [EDI].fChangedPosSz, DL
        {$ENDIF}
@@2:
        PUSH      EAX      // W saved

        MOV       EAX, [EDI].fBoundsRect.Bottom
        SUB       EAX, ECX
        PUSH      EAX      // H saved

        PUSH      EDI      // @Self saved
        {$IFDEF USE_GRAPHCTLS}
        {$IFDEF   USE_FLAGS}
        TEST      [EDI].fFlagsG6, 1 shl G6_GraphicCtl
        JZ        @@invalid1
        {$ELSE}
        CMP       [EDI].fWindowed, 0
        JNZ       @@invalid1
        {$ENDIF}
        MOV       EAX, EDI
        CALL      TControl.InvalidateNonWindowed
@@invalid1:
        {$ENDIF}

        LEA       EDI, [EDI].fBoundsRect
        MOVSD
        MOVSD
        MOVSD
        MOVSD

        MOV       ESI, EDI
        POP       EDI     // @ Self restored

        MOV       ECX, [EDI].fHandle
        JECXZ     @@fin

        STD
        PUSH      swp_flags

        LODSD
        LODSD
        XCHG      EDX, EAX // EDX = bottom
        LODSD
        XCHG      ECX, EAX // ECX = right
        LODSD
        SUB       EDX, EAX // EAX = bottom - top
        PUSH      EDX       // push HEIGHT
        XCHG      EDX, EAX  // EDX = top
        LODSD     // EAX = left
        CLD

        SUB       ECX, EAX
        PUSH      ECX       // push WIDTH

        PUSH      EDX       // push TOP
        PUSH      EAX       // push LEFT
        PUSH      0

        PUSH      [EDI].fHandle
        CALL      SetWindowPos

@@fin:
        POP       EDX       // H restored
        POP       EAX       // W restored

        {$IFDEF   USE_FLAGS}
        TEST      [EDI].fFlagsG1, (1 shl G1_SizeRedraw)
        {$ELSE}
        CMP       [EDI].fSizeRedraw, 0
        {$ENDIF   USE_FLAGS}
        JE        @@exit
@@invalid2:
        XCHG      EAX, EDI
        CALL      Invalidate

@@exit:
        ADD       ESP, size_TRect
        POP       ESI
        POP       EDI
end;

procedure TControl.SetWindowState( Value: TWindowState );
asm     //cmd    //opd
        PUSH     EAX
        PUSH     EDX
        CALL     TControl.GetWindowState
        POP      EDX
        CMP      AL, DL
        POP      EAX
        JE       @@exit
        MOV      [EAX].TControl.DF.fWindowState, DL
        MOV      ECX, [EAX].TControl.fHandle
        JECXZ    @@exit
        XCHG     EAX, EDX
        CBW
        CWDE
        MOV      AL, byte ptr [WindowStateShowCommands+EAX]
        PUSH     EAX
        PUSH     ECX
        CALL     ShowWindow
@@exit:
end;

procedure TControl.Show;
asm
        PUSH     EBX
        MOV      EBX, EAX
        CALL     CreateWindow
        MOV      DL, 1
        MOV      EAX, EBX
        CALL     SetVisible
        PUSH     [EBX].fHandle
        CALL     SetForegroundWindow
        XCHG     EAX, EBX
        CALL     DoSetFocus
        POP      EBX
end;

function TControl.Client2Screen( const P: TPoint ): TPoint;
asm
        PUSH      ESI
        PUSH      EDI

        MOV       ESI, EDX
        MOV       EDI, ECX

        MOVSD
        MOVSD

        PUSH      ECX
        MOV       ECX, [EAX].fHandle
        JECXZ     @@exit

        PUSH      ECX
        CALL      ClientToScreen
        PUSH      ECX

@@exit: POP       ECX
        POP       EDI
        POP       ESI
end;

function TControl.Screen2Client( const P: TPoint ): TPoint;
asm
        PUSH      ESI
        PUSH      EDI

        MOV       ESI, EDX
        MOV       EDI, ECX

        MOVSD
        MOVSD

        PUSH      ECX
        MOV       ECX, [EAX].fHandle
        JECXZ     @@exit

        PUSH      ECX
        CALL      ScreenToClient
        PUSH      ECX

@@exit: POP       ECX
        POP       EDI
        POP       ESI
end;

function TControl.ClientRect: TRect;
asm
        PUSH      ESI
        XCHG      ESI, EAX
        PUSH      EDX
        PUSH      EDX      // prepare 'dest' for GetClientRect

        LEA       EAX, [ESI].fBoundsRect
        XOR       ECX, ECX
        MOV       CL, size_TRect

        CALL      System.Move

        MOV       EAX, ESI
        CALL      TControl.GetWindowHandle

        XCHG      ECX, EAX
        JECXZ     @@exit

        PUSH      ECX    // prepare 'handle' for GetClientRect
        CALL      GetClientRect

        PUSH      EDX

@@exit: POP       EDX
        POP       EDX  // EDX = @Result
        LEA       ESI, [ESI].fClientTop
        LODSB
        MOVSX     EAX, AL
        ADD       [EDX].TRect.Top, EAX
        LODSB
        MOVSX     EAX, AL
        SUB       [EDX].TRect.Bottom, EAX
        LODSB
        MOVSX     EAX, AL
        ADD       [EDX].TRect.Left, EAX
        LODSB
        MOVSX     EAX, AL
        SUB       [EDX].TRect.Right, EAX
        POP       ESI
end;

procedure TControl.Invalidate;
asm
  {$IFDEF USE_GRAPHCTLS}
  PUSH dword ptr [EAX].TControl.PP.fDoInvalidate
  {$ELSE}
  MOV ECX, [EAX].fHandle
  JECXZ @@exit
  PUSH  $FF
  PUSH  0
  PUSH  ECX
  CALL  Windows.InvalidateRect
@@exit:
  {$ENDIF}
end;

{$IFDEF USE_GRAPHCTLS}
procedure InvalidateWindowed( Sender: PObj );
asm
  MOV ECX, [EAX].TControl.fHandle
  JECXZ @@exit
  PUSH  $FF
  PUSH  0
  PUSH  ECX
  CALL  Windows.InvalidateRect
@@exit:
end;
{$ENDIF USE_GRAPHCTLS}

function TControl.GetIcon: HIcon;
asm
        PUSH      EBX
        XCHG      EBX, EAX
        MOV       EAX, [EBX].DF.fIcon
        INC       EAX
        JZ        @@exit
        DEC       EAX
        JNZ       @@exit

        MOV       ECX, [Applet]
        JECXZ     @@load
        CMP       ECX, EBX
        JZ        @@load

        XCHG      EAX, ECX
        CALL      TControl.GetIcon
        TEST      EAX, EAX
        JZ        @@exit

        XOR       EDX, EDX
        PUSH      EDX
        PUSH      EDX
        PUSH      EDX
        INC       EDX  // IMAGE_ICON = 1
        PUSH      EDX
        PUSH      EAX
        CALL      CopyImage
        JMP       @@store_fIcon

@@main_icon:
        {$IFDEF NUMERIC_APPICON} {$DEFINE CUSTOM_APPICON} {$ENDIF}
        {$IFDEF CUSTOM_APPICON}
        {$I CustomAppIconRsrcName_ASM.inc} // create such file with DB 'your icon rsrc name' / DD youriconnumber
        {$ELSE}
        {$IFDEF UNICODE_CTRLS}
        DB 'M',0,'A',0,'I',0,'N',0,'I',0,'C',0,'O',0,'N',0,0
        {$ELSE}
        DB 'MAINICON'
        {$ENDIF}
        {$ENDIF}
        DB 0

@@load:
        {$IFDEF NUMERIC_APPICON}
        PUSH    DWORD  [@@main_icon]
        {$ELSE}
        PUSH      offset @@main_icon
        {$ENDIF}
        PUSH      [hInstance]
        CALL      LoadIcon
@@store_fIcon:
        MOV       [EBX].DF.fIcon, EAX
@@exit:
        POP       EBX
end;

function TControl.CallDefWndProc(var Msg: TMsg): Integer;
asm
        PUSH     [EDX].TMsg.lParam
        PUSH     [EDX].TMsg.wParam
        PUSH     [EDX].TMsg.message

        MOV      ECX, [EAX].fDefWndProc
        JECXZ    @@defwindowproc

        PUSH     [EAX].fHandle
        PUSH     ECX
        CALL     CallWindowProc
        RET

@@defwindowproc:
        PUSH     [EDX].TMsg.hwnd
        {$IFDEF UNICODE_CTRLS}
        CALL     DefWindowProcW
        {$ELSE}
        CALL     DefWindowProc
        {$ENDIF}
end;

function TControl.GetWindowState: TWindowState;
asm     //cmd    //opd
        PUSH     EBX
        PUSH     ESI
        XCHG     ESI, EAX
        MOVZX    EBX, [ESI].TControl.DF.fWindowState
        MOV      ECX, [ESI].TControl.fHandle
        JECXZ    @@ret_EBX
        MOV      BL, 2
        MOV      ESI, ECX
        PUSH     ESI
        CALL     IsZoomed
        TEST     EAX, EAX
        JNZ      @@ret_EBX
        DEC      EBX
        PUSH     ESI
        CALL     IsIconic
        TEST     EAX, EAX
        JNZ      @@ret_EBX
        DEC      EBX
@@ret_EBX:
        XCHG     EAX, EBX
        POP      ESI
        POP      EBX
end;

function TControl.DoSetFocus: Boolean;
asm
        PUSH      ESI
        MOV       ESI, EAX

        CALL      GetEnabled
        (*
        {$IFDEF   USE_FLAGS}
        MOV       DL, byte ptr [ESI].TControl.fStyle.f2_Style
        // F2_Tabstop = 0 !
        {$ELSE}
        MOV       DL, byte ptr [ESI+2].TControl.fStyle
        OR        DL, [ESI].TControl.fTabstop
        {$ENDIF   USE_FLAGS}
        AND       AL, DL
        *)
        TEST      AL, AL
        JZ        @@exit

        INC       [ESI].TControl.fClickDisabled
        PUSH      [ESI].TControl.fHandle
        CALL      SetFocus
        DEC       [ESI].TControl.fClickDisabled
        MOV       AL, 1
@@exit:
        POP       ESI
end;

function TControl.GetEnabled: Boolean;
asm
        MOV       ECX, [EAX].fHandle
        JECXZ     @@get_field

        PUSH      ECX
        CALL      IsWindowEnabled
        RET

@@get_field:
        TEST      byte ptr [EAX].fStyle + 3, 8 //WS_DISABLED shr 3
        SETZ      AL
end;

function TControl.IsMainWindow: Boolean;
asm     XCHG ECX, EAX
        XOR  EDX, EDX
        MOV  EAX, [Applet]
        TEST EAX, EAX
        JNZ  @@0
        {$IFDEF USE_FLAGS}
        TEST [ECX].fFlagsG3, 1 shl G3_IsControl
        {$ELSE}
        CMP  [ECX].fIsControl, AL
        {$ENDIF}
        JMP  @@3
@@0:    CMP  [appbuttonUsed], DL
        JZ   @@2
@@1:    PUSH ECX
        CALL TControl.GetMembers
        POP  ECX
@@2:    CMP  ECX, EAX
@@3:    SETZ AL
end;

procedure TControl.SetParent( Value: PControl );
asm
        PUSH     EBX
        PUSH     EDI
        XCHG     EBX, EAX
        MOV      EDI, EDX
        MOV      ECX, [EBX].fParent
        CMP      EDI, ECX
        JE       @@exit

        JECXZ    @@1
        {$IFDEF USE_GRAPHCTLS}
        PUSH     ECX
        MOV      EAX, EBX
        CALL     TControl.Invalidate
        POP      ECX
        {$ENDIF}
        PUSH     ECX

        MOV      EAX, [ECX].fChildren
        MOV      EDX, EBX
        CALL     TList.Remove

        POP      EAX
        {$IFNDEF USE_AUTOFREE4CONTROL}
        PUSH     EAX
        MOV      EDX, EBX
        CALL     TObj.RemoveFromAutoFree
        POP      EAX
        {$ENDIF}

        {$IFNDEF SMALLEST_CODE}
        MOV      ECX, [EAX].PP.fNotifyChild
        {$IFDEF  NIL_EVENTS}
        JECXZ    @@1
        {$ENDIF}
        XOR      EDX, EDX
        CALL     ECX
        {$ENDIF}
@@1:
        MOV      [EBX].fParent, EDI
        TEST     EDI, EDI
        JZ       @@exit

        MOV      EAX, [EDI].fChildren
        MOV      EDX, EBX
        CALL     TList.Add

        {$IFDEF USE_AUTOFREE4CHILDREN}
        MOV      EAX, EDI
        MOV      EDX, EBX
        CALL     TControl.Add2AutoFree
        {$ENDIF}

        {$IFNDEF INPACKAGE}
        MOV      ECX, [EBX].fHandle
        JECXZ    @@2
        MOV      EAX, EDI
        CALL     TControl.GetWindowHandle
        PUSH     EAX
        PUSH     [EBX].fHandle
        CALL     Windows.SetParent
@@2:
        {$ENDIF}

        {$IFNDEF SMALLEST_CODE}
        MOV      ECX, [EDI].PP.fNotifyChild
        {$IFDEF  NIL_EVENTS}
        JECXZ    @@3
        {$ENDIF}
        MOV      EAX, EDI
        MOV      EDX, EBX
        CALL     ECX
@@3:
        MOV      ECX, [EBX].PP.fNotifyChild
        {$IFDEF  NIL_EVENTS}
        JECXZ    @@4
        {$ENDIF}
        MOV      EAX, EDI
        MOV      EDX, EBX
        CALL     ECX
@@4:    {$ENDIF}

        {$IFNDEF USE_GRAPHCTLS}
        XCHG     EAX, EBX
        CALL     TControl.Invalidate
        {$ENDIF}
@@exit:
        POP      EDI
        POP      EBX
end;

constructor TControl.CreateParented(AParent: PControl);
asm     //cmd    //opd
        PUSH     EAX
        MOV      EDX, ECX
        MOV      ECX, [EAX]
        CALL     dword ptr [ECX+8]
        POP      EAX
end;

function TControl.GetLeft: Integer;
asm
        CALL      HelpGetBoundsRect
        POP       EAX

        POP       ECX
        POP       ECX
        POP       ECX
end;

procedure TControl.SetLeft( Value: Integer );
asm
        PUSH      EDI

        PUSH      EDX
        CALL      HelpGetBoundsRect
        POP       EDX           // EDX = Left
        POP       ECX           // ECX = Top
        POP       EDI           // EDI = Right

        SUB       EDI, EDX      // EDI = width
        MOV       EDX, [ESP+4]  // EDX = Left'
        ADD       EDI, EDX      // EDI = Right'

        PUSH      EDI
        PUSH      ECX
        PUSH      EDX
        MOV       EDX, ESP

        CALL      SetBoundsRect
        ADD       ESP, size_TRect + 4

        POP       EDI

end;

function TControl.GetTop: Integer;
asm
        CALL      HelpGetBoundsRect
        POP       EDX
          POP       EAX
        POP       EDX
        POP       EDX
end;

procedure TControl.SetTop( Value: Integer );
asm
        PUSH      ESI
        PUSH      EDI

          PUSH      EDX
        CALL      HelpGetBoundsRect
        POP       EDX           // EDX = Left
        POP       ECX           // ECX = Top
        POP       EDI           // EDI = Right
        POP       ESI           // ESI = Bottom

        SUB       ESI, ECX      // ESI = Height'
          POP       ECX         // ECX = Top'
        ADD       ESI, ECX      // ESI = Bottom'

        PUSH      ESI
        PUSH      EDI
        PUSH      ECX
        PUSH      EDX
        MOV       EDX, ESP

        CALL      SetBoundsRect
        ADD       ESP, size_TRect

        POP       EDI
        POP       ESI
end;

function TControl.GetWidth: Integer;
asm
        CALL      HelpGetBoundsRect
        POP       EDX
          POP       ECX
        POP       EAX
        SUB       EAX, EDX
          POP       ECX
end;

procedure TControl.SetWidth( Value: Integer );
asm
        PUSH      EDX

        CALL      HelpGetBoundsRect
        POP       EDX
        PUSH      EDX
        ADD       EDX, [ESP].size_TRect
        MOV       [ESP].TRect.Right, EDX

        MOV       EDX, ESP
        CALL      SetBoundsRect

        ADD       ESP, size_TRect + 4
end;

function TControl.GetHeight: Integer;
asm
        CALL      HelpGetBoundsRect
        POP       ECX
        POP       EDX          // EDX = top
        POP       ECX
        POP       EAX          // EAX = bottom
        SUB       EAX, EDX     // result = height
end;

procedure TControl.SetHeight( Value: Integer );
asm
        PUSH      EDX

        CALL      HelpGetBoundsRect
        MOV       EDX, [ESP].TRect.Top
        ADD       EDX, [ESP].size_TRect
        MOV       [ESP].TRect.Bottom, EDX

        MOV       EDX, ESP
        CALL      SetBoundsRect

        ADD       ESP, size_TRect + 4
end;

function TControl.GetPosition: TPoint;
asm
        PUSH      EDX
        CALL      HelpGetBoundsRect
        POP       EAX         // EAX = left
        POP       ECX         // ECX = top
        POP       EDX
        POP       EDX
        POP       EDX         // EDX = @Result
        MOV       [EDX], EAX
        MOV       [EDX+4], ECX
end;

procedure TControl.Set_Position( Value: TPoint );
asm
        PUSH      ESI
        PUSH      EDI

        PUSH      EAX
        PUSH      EDX
        CALL      HelpGetBoundsRect
        POP       EDX           // left
        POP       EAX           // top
        POP       ECX           // right
        SUB       ECX, EDX      // ECX = width
        POP       EDX           // bottom
        SUB       EDX, EAX      // EDX = height
        POP       EAX           // EAX = @Value
        POP       ESI           // ESI = @Self

        MOV       EDI, [EAX+4]  // top'
        ADD       EDX, EDI
        PUSH      EDX           // bottom'

        MOV       EAX, [EAX]    // left'
        ADD       ECX, EAX
        PUSH      ECX           // right'

        PUSH      EDI           // top'
        PUSH      EAX           // left'

        MOV       EAX, ESI
        MOV       EDX, ESP
        CALL      SetBoundsRect

        ADD       ESP, size_TRect

        POP       EDI
        POP       ESI
end;

procedure DefaultPaintBackground( Sender: PControl; DC: HDC; Rect: PRect );
asm
        PUSH      EDI

        PUSH      EDI
        MOV       EDI, ESP

        PUSH      ECX
        PUSH      EDX

        MOV       EAX, [EAX].TControl.fColor
        CALL      Color2RGB
        PUSH      EAX
        CALL      CreateSolidBrush
        STOSD
        MOV       EDI, EAX
        CALL      windows.FillRect
        PUSH      EDI
        CALL      DeleteObject
        POP       EDI
end;

procedure TControl.SetCtlColor( Value: TColor );
asm
        PUSH     EBX
        MOV      EBX, EAX

        {$IFNDEF INPACKAGE}
        PUSH     EDX

        CALL     GetWindowHandle
        XCHG     ECX, EAX

        POP      EDX
        {$ELSE}
        MOV      ECX, [EBX].fHandle
        {$ENDIF}

        JECXZ    @@1

        {$IFDEF  COMMANDACTIONS_OBJ}
        MOV      ECX, [EBX].fCommandActions
        MOVZX    ECX, [ECX].TCommandActionsObj.aSetBkColor
        {$ELSE}
        MOVZX    ECX, [EBX].fCommandActions.aSetBkColor
        {$ENDIF}
        JECXZ    @@1

        PUSH     EDX

        XCHG     EAX, EDX
        PUSH     ECX
        CALL     Color2RGB
        POP      ECX

        PUSH     EAX        // Color2RGB( Value )
        PUSH     0          // 0
        PUSH     ECX        // fCommandActions.aSetBkColor
        PUSH     EBX        // @ Self
        CALL     TControl.Perform

        POP      EDX

@@1:
        CMP      EDX, [EBX].fColor
        JZ       @@exit

        MOV      [EBX].fColor, EDX

        XOR      ECX, ECX
        XCHG     ECX, [EBX].fTmpBrush
        JECXZ    @@setbrushcolor

        PUSH     EDX
        PUSH     ECX
        CALL     DeleteObject
        POP      EDX

@@setbrushcolor:
        MOV      ECX, [EBX].fBrush
        JECXZ    @@invldte

        XCHG     EAX, ECX
        MOV      ECX, EDX
        //MOV      EDX, go_Color
        XOR      EDX, EDX
        CALL     TGraphicTool.SetInt

@@invldte:
        XCHG     EAX, EBX
        CALL     TControl.Invalidate
@@exit:
        POP      EBX
end;

function TControl.GetParentWnd( NeedHandle: Boolean ): HWnd;
asm
        XCHG      EDX, EAX
        TEST      AL, AL
        MOV       EAX, [EDX].fParentWnd
        MOV       ECX, [EDX].fParent
        JECXZ     @@exit

        PUSH      ECX
        JZ        @@load_handle

        XCHG      EAX, ECX
        CALL      GetWindowHandle

@@load_handle:
        POP       EAX
        MOV       EAX, [EAX].fHandle
@@exit:
end;

function TControl.ProcessMessage: Boolean;
const size_TMsg = sizeof( TMsg );
asm
        PUSH      EBX
        XCHG      EBX, EAX

        ADD       ESP, -size_TMsg-4

        MOV       EDX, ESP
        PUSH      1
        XOR       ECX, ECX
        PUSH      ECX
        PUSH      ECX
        PUSH      ECX
        PUSH      EDX
        CALL      PeekMessage

        TEST      EAX, EAX
        JZ        @@exit

        CMP       WORD PTR [ESP].TMsg.message, WM_QUIT
        JNE       @@tran_disp
        OR        [AppletTerminated], DL
        {$IFDEF   PROVIDE_EXITCODE}
        MOV       EDX, [ESP].TMsg.wParam
        MOV       [ExitCode], EDX
        {$ENDIF   PROVIDE_EXITCODE}
        JMP       @@fin

@@tran_disp:
        MOV       ECX, [EBX].PP.fExMsgProc
        {$IFDEF   NIL_EVENTS}
        JECXZ     @@do_tran_disp
        {$ENDIF}
        XCHG      EAX, EBX
        MOV       EDX, ESP
        CALL      ECX
        TEST      AL, AL
        JNZ       @@fin

@@do_tran_disp:
        MOV       EAX, ESP
        PUSH      EAX
        PUSH      EAX
        CALL      TranslateMessage
        CALL      DispatchMessage

@@fin:
        CMP       word ptr [ESP].TMsg.message, 0
        SETNZ     AL

@@exit: ADD       ESP, size_TMsg+4
        POP       EBX
end;

procedure TControl.ProcessMessages;
asm
@@loo:  PUSH     EAX
        CALL     ProcessMessage
        DEC      AL
        POP      EAX
        JZ       @@loo
end;

function WndProcForm(Self_: PControl; var Msg: TMsg; var Rslt: Integer ): Boolean;
const szPaintStruct = sizeof(TPaintStruct);
asm     //cmd    //opd
        {$IFDEF ENDSESSION_HALT}
        CMP      word ptr [EDX].TMsg.message, WM_ENDSESSION
        JNE      @@chk_WM_SETFOCUS

        CMP      [EDX].TMsg.wParam, 0
        JZ       @@ret_false

        CALL     TObj.RefDec
        XOR      EAX, EAX
        MOV      [AppletRunning], AL
        XCHG     EAX, [Applet]
        INC      [AppletTerminated]

        CALL     TObj.RefDec
        CALL     System.@Halt0
        {$ENDIF ENDSESSION_HALT}

@@chk_WM_SETFOCUS:
        CMP      word ptr [EDX].TMsg.message, WM_SETFOCUS
        JNE      @@ret_false

        PUSH     EBX
        PUSH     ESI
        XOR      EBX, EBX
        INC      EBX
        XCHG     ESI, EAX
      {$IFDEF NEW_MODAL}
        MOV      ECX, [ESI].TControl.DF.fModalForm
        JECXZ    @@no_fix_modal_setfocus
        PUSH     [ECX].TControl.fHandle
        CALL     SetFocus
@@no_fix_modal_setfocus:
        MOV      ECX, [ESI].TControl.DF.FCurrentControl
        JECXZ    @@setFocuswhenCreateWindow
        {$IFDEF  USE_FLAGS}
        TEST     [ECX].TControl.fFlagsG3, (1 shl G3_IsForm)
        SETNZ    DL
        TEST     [ESI].TControl.fFlagsG3, (1 shl G3_IsApplet)
        SETNZ    DH
        XOR      DL, DH
        JNZ      @@1
        {$ELSE}
        MOV      DL,  [ECX].TControl.fIsForm
        XOR      DL,  [ESI].TControl.FIsApplet
        JNZ      @@1
        {$ENDIF}
      {$ELSE not NEW_MODAL}
        MOV      ECX, [ESI].TControl.DF.fCurrentControl
        JECXZ    @@0
      {$ENDIF}
@@setFocuswhenCreateWindow:
        JECXZ    @@1  //+++++++++++++++
        //INC      EBX
        XCHG     EAX, ECX

        // or CreateForm?
        PUSH     EAX
        CALL     CallTControlCreateWindow
        TEST     AL, AL
        POP      EAX
        JZ       @@1

        PUSH     [EAX].TControl.fHandle
        CALL     SetFocus
        INC      EBX
@@0:    DEC      EBX
@@1:    MOV      ECX, [Applet]
        JECXZ    @@ret_EBX
        CMP      ECX, ESI
        JE       @@ret_EBX
        MOV      [ECX].TControl.DF.FCurrentControl, ESI
@@ret_EBX:
        XCHG     EAX, EBX
        POP      ESI
        POP      EBX
        RET

@@ret_false:
        XOR      EAX, EAX
end;

function GetPrevCtrlBoundsRect( P: PControl; var R: TRect ): Boolean;
asm
        MOV       EDX, EBX
        MOV       EAX, [EBX].TControl.fParent
        TEST      EAX, EAX
        JZ        @@exit
          PUSH      EAX
        CALL      TControl.ChildIndex
        TEST      EAX, EAX
        XCHG      EDX, EAX
          POP       EAX
        JZ        @@exit
        DEC       EDX
        CALL      TControl.GetMembers

        POP       ECX  // retaddr
        ADD       ESP, -size_TRect
        MOV       EDX, ESP
        PUSH      ECX
        CALL      TControl.GetBoundsRect
        STC       // return CARRY
@@exit:
end;

function TControl.PlaceUnder: PControl;
asm
        PUSH      EBX
        XCHG      EBX, EAX
        CALL      GetPrevCtrlBoundsRect
        JNC       @@exit
        POP       EDX  // EDX = Left
        MOV       EAX, EBX
        CALL      TControl.SetLeft

        POP       EDX
        POP       EDX
        POP       EDX  // EDX = Bottom

        MOV       EAX, [EBX].fParent
        MOVSX     ECX, [EAX].fMargin
        ADD       EDX, ECX

        MOV       EAX, EBX
        CALL      TControl.SetTop
@@exit:
        XCHG      EAX, EBX
        POP       EBX
end;

function TControl.PlaceDown: PControl;
asm
        PUSH      EBX
        XCHG      EBX, EAX
        CALL      GetPrevCtrlBoundsRect
        JNC       @@exit
        POP       EDX
        POP       EDX
        POP       EDX
        POP       EDX  // EDX = Bottom

        MOV       EAX, [EBX].fParent
        MOVSX     ECX, [EAX].fMargin
        ADD       EDX, ECX

        MOV       EAX, EBX
        CALL      TControl.SetTop
@@exit:
        XCHG       EAX, EBX
        POP       EBX
end;

function TControl.PlaceRight: PControl;
asm
        PUSH      EBX
        XCHG      EBX, EAX
        CALL      GetPrevCtrlBoundsRect
        JNC       @@exit
        POP       EDX
        POP       EDX  // EDX = Top
        MOV       EAX, EBX
        CALL      TControl.SetTop
        POP       EDX  // EDX = Right

        MOV       EAX, [EBX].fParent
        MOVSX     ECX, [EAX].fMargin
        ADD       EDX, ECX

        POP       ECX
        MOV       EAX, EBX
        CALL      TControl.SetLeft
@@exit:
        XCHG      EAX, EBX
        POP       EBX
end;

function TControl.SetSize(W, H: Integer): PControl;
asm
        PUSH      EBX
        XCHG      EBX, EAX
        SUB  ESP, 16
        XCHG      EAX, EDX
        MOV  EDX, ESP
        PUSH      ECX // save H
        PUSH      EAX // save W
        MOV  EAX, EBX
        CALL GetBoundsRect
        POP       ECX // pop W
        JECXZ     @@nochg_W
        ADD       ECX, [ESP+4].TRect.Left
        MOV       [ESP+4].TRect.Right, ECX
@@nochg_W:
        POP       ECX // pop H
        JECXZ     @@nochg_H
        ADD       ECX, [ESP].TRect.Top
        MOV       [ESP].TRect.Bottom, ECX
@@nochg_H:
        MOV       EAX, EBX
        MOV       EDX, ESP
        CALL      TControl.SetBoundsRect
        ADD  ESP, 16
        XCHG      EAX, EBX
        POP       EBX
end;

function TControl.AlignLeft(P: PControl): PControl;
asm
        PUSH     EAX
        MOV      EAX, EDX
        CALL     TControl.GetLeft
        MOV      EDX, EAX
        POP      EAX
        PUSH     EAX
        CALL     TControl.SetLeft
        POP      EAX
end;

function TControl.AlignTop(P: PControl): PControl;
asm
        PUSH     EAX
        MOV      EAX, EDX
        CALL     TControl.GetTop
        MOV      EDX, EAX
        POP      EAX
        PUSH     EAX
        CALL     TControl.SetTop
        POP      EAX
end;

function WndProcCtrl( Self_: PControl; var Msg: TMsg; var Rslt: Integer): Boolean;
asm     //cmd    //opd
        PUSH     EBX
        XCHG     EBX, EAX
        PUSH     ESI
        PUSH     EDI
        MOV      EDI, EDX
        MOV      EDX, [EDI].TMsg.message

        SUB      DX, CN_CTLCOLORMSGBOX
        CMP      DX, CN_CTLCOLORSTATIC-CN_CTLCOLORMSGBOX
        JA       @@chk_CM_COMMAND
@@2:
        PUSH     ECX
        MOV      EAX, [EBX].TControl.fTextColor
        CALL     Color2RGB
        XCHG     ESI, EAX
        PUSH     ESI
        PUSH     [EDI].TMsg.wParam
        CALL     SetTextColor
        {$IFDEF  USE_FLAGS}
        TEST     [EBX].TControl.fFlagsG2, (1 shl G2_Transparent)
        {$ELSE}
        CMP      [EBX].TControl.fTransparent, 0
        {$ENDIF}
        JZ       @@opaque

        PUSH     Windows.TRANSPARENT
        PUSH     [EDI].TMsg.wParam
        CALL     SetBkMode
        PUSH     NULL_BRUSH
        CALL     GetStockObject
        JMP      @@ret_rslt

@@opaque:
        MOV      EAX, [EBX].TControl.fColor
        CALL     Color2RGB
        XCHG     ESI, EAX
        PUSH     OPAQUE
        PUSH     [EDI].TMsg.wParam
        CALL     SetBkMode
        PUSH     ESI
        PUSH     [EDI].TMsg.wParam
        CALL     SetBkColor

        MOV      EAX, EBX
        CALL     Global_GetCtlBrushHandle
@@ret_rslt:
        XCHG     ECX, EAX
@@tmpbrushready:
        POP      EAX
        MOV      [EAX], ECX
@@ret_true:
        MOV      AL, 1

        JMP      @@ret_EAX

@@chk_CM_COMMAND:
        CMP      word ptr [EDI].TMsg.message, CM_COMMAND
        JNE      @@chk_WM_SETFOCUS

        PUSH     ECX

        MOVZX    ECX, word ptr [EDI].TMsg.wParam+2
        {$IFDEF  COMMANDACTIONS_OBJ}
        MOV      ESI, [EBX].TControl.fCommandActions
        CMP      CX, [ESI].TCommandActionsObj.aClick
        {$ELSE}
        CMP      CX, [EBX].TControl.fCommandActions.aClick
        {$ENDIF}
        JNE      @@chk_aEnter

        CMP      [EBX].TControl.fClickDisabled, 0
        JG       @@calldef
        MOV      EAX, EBX
        MOV      DL, 1
        CALL     TControl.SetFocused
        MOV      EAX, EBX
        CALL     TControl.DoClick
        JMP      @@calldef

@@chk_aEnter:
        {$IFDEF  COMMANDACTIONS_OBJ}
        MOV      EAX, [EBX].TControl.fCommandActions
        CMP      CX, [EAX].TCommandActionsObj.aEnter
        {$ELSE}
        CMP      CX, [EBX].TControl.fCommandActions.aEnter
        {$ENDIF}
        {$IFDEF  EVENTS_DYNAMIC}
        MOV      EAX, [EBX].TControl.EV
        LEA      EAX, [EAX].TEvents.fOnEnter
        {$ELSE}
        LEA      EAX, [EBX].TControl.EV.fOnEnter
        {$ENDIF}
        JE       @@goEvent
        //LEA      EAX, [EBX].TControl.EV.fOnLeave
        ADD      EAX, 8
        {$IFDEF  COMMANDACTIONS_OBJ}
        CMP      CX, [ESI].TCommandActionsObj.aLeave
        {$ELSE}
        CMP      CX, [EBX].TControl.fCommandActions.aLeave
        {$ENDIF}
        JE       @@goEvent
        //LEA      EAX, [EBX].TControl.EV.fOnChangeCtl
        SUB      EAX, 16
        {$IFDEF  COMMANDACTIONS_OBJ}
        CMP      CX, [ESI].TCommandActionsObj.aChange
        {$ELSE}
        CMP      CX, [EBX].TControl.fCommandActions.aChange
        {$ENDIF}
        JNE      @@chk_aSelChange
@@goEvent:
        MOV      ECX, [EAX].TMethod.Code
        {$IFDEF  NIL_EVENTS}
        JECXZ    @@2calldef
        {$ENDIF}
        MOV      EAX, [EAX].TMethod.Data
        MOV      EDX, EBX
        CALL     ECX
@@2calldef:
        JMP      @@calldef

@@chk_aSelChange:
        {$IFDEF  COMMANDACTIONS_OBJ}
        CMP      CX, [ESI].TCommandActionsObj.aSelChange
        {$ELSE}
        CMP      CX, [EBX].TControl.fCommandActions.aSelChange
        {$ENDIF}
        JNE      @@chk_WM_SETFOCUS_1
        MOV      EAX, EBX
        CALL     TControl.DoSelChange

@@calldef:
        XCHG     EAX, EBX
        MOV      EDX, EDI
        CALL     TControl.CallDefWndProc
        JMP      @@ret_rslt

@@chk_WM_SETFOCUS_1:
        POP      ECX
@@chk_WM_SETFOCUS:
        XOR      EAX, EAX
        CMP      word ptr [EDI].TMsg.message, WM_SETFOCUS
        JNE      @@chk_WM_KEYDOWN

        MOV      [ECX], EAX
        MOV      EAX, EBX
        CALL     TControl.ParentForm
        TEST     EAX, EAX
        JZ       @@ret_true

        PUSH     EAX
        MOV      ECX, [EAX].TControl.DF.FCurrentControl
        JECXZ    @@a1
        CMP      ECX, EBX
        JZ       @@a1
        XCHG     EAX, ECX
        {$IFDEF  EVENTS_DYNAMIC}
        MOV      EAX, [EAX].TControl.EV
        MOV      ECX, [EAX].TEvents.fLeave.TMethod.Code
        {$ELSE}
        MOV      ECX, [EAX].TControl.EV.fLeave.TMethod.Code
        {$ENDIF}
        {$IFDEF  NIL_EVENTS}
        JECXZ    @@a1
        {$ENDIF}
        XCHG     EDX, EAX
        {$IFDEF  EVENTS_DYNAMIC}
        MOV      EAX, [EDX].TEvents.fLeave.TMethod.Data
        {$ELSE}
        MOV      EAX, [EDX].TControl.EV.fLeave.TMethod.Data
        {$ENDIF}
        CALL     ECX
@@a1:   POP      EAX

        MOV      [EAX].TControl.DF.FCurrentControl, EBX
        XOR      EAX, EAX

        PUSH     EDX
@@2ret_EAX:
        POP      EDX

@@chk_WM_KEYDOWN:
        {$IFDEF KEY_PREVIEW_OR_ESC_CLOSE_DIALOGS}
        CMP      word ptr [EDI].TMsg.message, WM_KEYDOWN
        {$IFDEF KEY_PREVIEW}
        JNE      @@chk_other_KEYMSGS
        {$ELSE}
        JNE      @@ret0
        {$ENDIF}

        {$IFDEF KEY_PREVIEW}
                MOV      EAX, EBX
                CALL     TControl.ParentForm
                CMP      EAX, EBX
                JE       @@kp_end

                {$IFDEF  USE_FLAGS}
                TEST     [EAX].TControl.fFlagsG6, 1 shl G6_KeyPreview
                {$ELSE}
                CMP      [EAX].TControl.fKeyPreview, 0
                {$ENDIF}
                JZ       @@kp_end

                {$IFDEF  USE_FLAGS}
                OR       [EAX].TControl.fFlagsG4, 1 shl G4_Pushed
                {$ELSE}
                MOV      [EAX].TControl.fKeyPreviewing, 1
                {$ENDIF}
                INC      [EAX].TControl.DF.fKeyPreviewCount
                PUSH     EAX

                PUSH     [EDI].TMsg.lParam
                PUSH     [EDI].TMsg.wParam
                PUSH     WM_KEYDOWN
                PUSH     EAX
                CALL     TControl.Perform
                POP      EAX
                DEC      [EAX].TControl.DF.fKeyPreviewCount
@@kp_end:
        {$ENDIF}

        {$IFDEF ESC_CLOSE_DIALOGS}
        MOV      EAX, EBX
        CALL     TControl.ParentForm
        TEST     [EAX].TControl.fExStyle, WS_EX_DLGMODALFRAME
        JZ       @@ecd_end
        CMP      [EDI].TMsg.wParam, 27
        JNE      @@ecd_end
        PUSH     0
        PUSH     0
        PUSH     WM_CLOSE
        PUSH     EAX
        CALL     TControl.Perform
@@ecd_end:
        {$ENDIF}

@@ret0:
        XOR      EAX, EAX
        {$IFDEF KEY_PREVIEW}
                JMP      @@ret_EAX
@@chk_other_KEYMSGS:
                MOVZX    EAX, word ptr [EDI].TMsg.message
                SUB      AX, WM_KEYDOWN
                JB       @@ret0
                CMP      AX, 6
                JA       @@ret0
                // all WM_KEYUP=$101, WM_CHAR=$102, WM_DEADCHAR=$103, WM_SYSKEYDOWN=$104,
                //  WM_SYSKEYUP=$105, WM_SYSCHAR=$106, WM_SYSDEADCHAR=$107
                MOV      EAX, EBX
                CALL     TControl.ParentForm
                CMP      EAX, EBX
                JE       @@ret0

                {$IFDEF  USE_FLAGS}
                TEST     [EAX].TControl.fFlagsG6, 1 shl G6_KeyPreview
                {$ELSE}
                CMP      [EAX].fKeyPreview, 0
                {$ENDIF}
                JZ       @@ret0

                {$IFDEF  USE_FLAGS}
                OR       [EAX].TControl.fFlagsG4, 1 shl G4_Pushed
                {$ELSE}
                MOV      [EAX].TControl.fKeyPreviewing, 1
                {$ENDIF}
                INC      [EAX].TControl.DF.fKeyPreviewCount
                PUSH     EAX
                PUSH     [EDI].TMsg.lParam
                PUSH     [EDI].TMsg.wParam
                PUSH     [EDI].TMsg.message
                PUSH     EAX
                CALL     TControl.Perform
                POP      EAX
                DEC      [EAX].TControl.DF.fKeyPreviewCount
                XOR      EAX, EAX
        {$ENDIF KEY_PREVIEW}
        {$ENDIF KEY_PREVIEW_OR_ESC_CLOSE_DIALOGS}

@@ret_EAX:
        POP      EDI
        POP      ESI
        POP      EBX
end;

procedure TControl.DoClick;
asm
        PUSH     EAX
        CALL     [EAX].PP.fControlClick
        POP      EDX
        {$IFDEF  EVENTS_DYNAMIC}
        MOV      EAX, [EDX].TControl.EV
        MOV      ECX, [EAX].TEvents.fOnClick.TMethod.Code
        {$ELSE}
        MOV      ECX, [EDX].EV.fOnClick.TMethod.Code
        {$ENDIF}
        {$IFDEF  NIL_EVENTS}
        JECXZ    @@exit
        {$ENDIF}
        {$IFDEF  EVENTS_DYNAMIC}
        MOV      EAX, [EAX].TEvents.fOnClick.TMethod.Data
        {$ELSE}
        MOV      EAX, [EDX].EV.fOnClick.TMethod.Data
        {$ENDIF}
        CALL     ECX
@@exit:
end;

function TControl.ParentForm: PControl;
asm
@@1:    {$IFDEF  USE_FLAGS}
        TEST     [EAX].fFlagsG3, 1 shl G3_IsControl
        {$ELSE}
        CMP      [EAX].fIsControl, 0
        {$ENDIF}
        JZ       @@exit
        MOV      EAX, [EAX].fParent
        TEST     EAX, EAX
        JNZ      @@1
@@exit:
end;

procedure TControl.SetProgressColor(const Value: TColor);
asm
        PUSH     EDX
        PUSH     EAX
        MOV      EAX, EDX
        CALL     Color2RGB
        POP      EDX
        PUSH     EDX
        PUSH     EAX
        PUSH     0
        PUSH     PBM_SETBARCOLOR
        PUSH     EDX
        CALL     Perform
        TEST     EAX, EAX
        POP      EAX
        POP      EDX
        JZ       @@exit
        MOV      [EAX].fTextColor, EDX
@@exit:
end;

function TControl.GetFont: PGraphicTool;
asm
        MOV      ECX, [EAX].FFont
        INC      ECX
        LOOP     @@exit
        PUSH     EAX
        CALL     NewFont
        {$IFDEF USE_AUTOFREE4CONTROLS}
        POP      EDX
        PUSH     EDX
        PUSH     EAX
        XCHG     eax, edx
        CALL     TObj.Add2AutoFree
        POP      EAX
        {$ENDIF}
        POP      EDX
        MOV      [EDX].FFont, EAX
        MOV      ECX, [EDX].fTextColor
        MOV      [EAX].TGraphicTool.fData.Color, ECX
        MOV      [EAX].TGraphicTool.fOnGTChange.TMethod.Code, offset[FontChanged]
        MOV      [EAX].TGraphicTool.fOnGTChange.TMethod.Data, EDX
        RET
@@exit: XCHG     EAX, ECX
end;

function TControl.GetBrush: PGraphicTool;
asm
        MOV      ECX, [EAX].FBrush
        INC      ECX
        LOOP     @@exit
        PUSH     EAX
        CALL     NewBrush
        POP      EDX   // @ Self
        MOV      [EDX].FBrush, EAX
        MOV      ECX, [EDX].fColor
        MOV      [EAX].TGraphicTool.fData.Color, ECX
        MOV      [EAX].TGraphicTool.fOnGTChange.TMethod.Code, offset[BrushChanged]
        MOV      [EAX].TGraphicTool.fOnGTChange.TMethod.Data, EDX
        {$IFDEF USE_AUTOFREE4CONTROLS}
        PUSH     EAX
        XCHG     EAX, EDX
        CALL     TControl.Add2AutoFree
        POP      ECX
        {$ENDIF}
@@exit: XCHG     EAX, ECX
end;

procedure TControl.FontChanged(Sender: PGraphicTool);
asm
        MOV      ECX, [EDX].TGraphicTool.fData.Color
        MOV      [EAX].fTextColor, ECX
        PUSH     EAX
        CALL     [ApplyFont2Wnd_Proc]
        POP      EAX
        CALL     Invalidate
end;

procedure TControl.BrushChanged(Sender: PGraphicTool);
asm
        MOV      ECX, [EDX].TGraphicTool.fData.Color
        MOV      [EAX].fColor, ECX
        XOR      ECX, ECX
        XCHG     ECX, [EAX].fTmpBrush
        JECXZ    @@inv
        PUSH     EAX
        PUSH     ECX
        CALL     DeleteObject
        POP      EAX
@@inv:  CALL     Invalidate
end;

procedure DoApplyFont2Wnd( _Self: PControl );
asm
        PUSH       EBX
        XCHG       EBX, EAX

        MOV        ECX, [EBX].TControl.fFont
        JECXZ      @@exit
        XCHG       EAX, ECX

        MOV        ECX, [EBX].TControl.fHandle
        JECXZ      @@0

        MOV        EDX, [EAX].TGraphicTool.fData.Color
        MOV        [EBX].TControl.fTextColor, EDX

        PUSH       $FFFF
        CALL       TGraphicTool.GetHandle
        PUSH       EAX
        PUSH       WM_SETFONT
        PUSH       EBX
        CALL       TControl.Perform

@@0:
        XOR        ECX, ECX
        XCHG       ECX, [EBX].TControl.fCanvas
        JECXZ      @@1

        XCHG       EAX, ECX
        CALL       TObj.RefDec
@@1:
        XCHG       EAX, EBX
        CALL       TControl.DoAutoSize
@@exit:
        POP        EBX
end;

function TControl.ResizeParent: PControl;
asm
        LEA       EDX, [TControl.ResizeParentRight]
        PUSH      EDX
        CALL      EDX
        CALL      TControl.ResizeParentBottom
end;

function TControl.ResizeParentBottom: PControl;
asm
        PUSH      EAX
        PUSH      EBX
        MOV       EBX, [EAX].fParent
        TEST      EBX, EBX
        JZ        @@exit

        MOV       EDX, [EAX].fBoundsRect.Bottom
        MOVSX     ECX, [EBX].fMargin
        ADD       EDX, ECX

        {$IFDEF   USE_FLAGS}
        TEST      [EBX].fFlagsG2, (1 shl G2_ChangedH)
        JZ        @@1
        {$ELSE}
        TEST      [EBX].fChangedPosSz, 20h
        JZ        @@1
        {$ENDIF}

        PUSH      EDX
        MOV       EAX, EBX
        CALL      GetClientHeight
        POP       EDX

        CMP       EDX, EAX
        JE        @@exit
@@1:
        MOV       EAX, EBX
        CALL      TControl.SetClientHeight
        {$IFDEF   USE_FLAGS}
        OR        [EBX].fFlagsG2, (1 shl G2_ChangedH)
        {$ELSE}
        OR        [EBX].fChangedPosSz, 20h
        {$ENDIF}
@@exit:
        POP       EBX
        POP       EAX
end;

function TControl.ResizeParentRight: PControl;
asm
        PUSH      EAX
        PUSH      EBX
        MOV       EBX, [EAX].fParent
        TEST      EBX, EBX
        JZ        @@exit

        MOV       EDX, [EAX].fBoundsRect.Right
        MOVSX     ECX, [EBX].fMargin
        ADD       EDX, ECX

        {$IFDEF   USE_FLAGS}
        TEST      [EBX].fFlagsG2, (1 shl G2_ChangedW)
        {$ELSE}
        TEST      [EBX].fChangedPosSz, 10h
        {$ENDIF}
        JZ        @@1

        PUSH      EDX
        MOV       EAX, EBX
        CALL      GetClientWidth
        POP       EDX

        CMP       EDX, EAX
        JLE       @@exit
@@1:
        MOV       EAX, EBX
        CALL      TControl.SetClientWidth
        {$IFDEF   USE_FLAGS}
        OR        [EBX].fFlagsG2, (1 shl G2_ChangedW)
        {$ELSE}
        OR        [EBX].fChangedPosSz, 10h
        {$ENDIF}
@@exit:
        POP       EBX
        POP       EAX
end;

function TControl.GetClientHeight: Integer;
asm
        ADD       ESP, -size_TRect
        MOV       EDX, ESP
        CALL      TControl.ClientRect
        POP       EDX
        POP       ECX            // Top
        POP       EDX
        POP       EAX            // Bottom
        SUB       EAX, ECX       // Result = Bottom - Top
end;

function TControl.GetClientWidth: Integer;
asm
        ADD       ESP, -size_TRect
        MOV       EDX, ESP
        CALL      TControl.ClientRect
        POP       ECX            // Left
        POP       EDX
        POP       EAX            // Right
        SUB       EAX, ECX       // Result = Right - Left
        POP       EDX
end;

procedure TControl.SetClientHeight(const Value: Integer);
asm
        PUSH      EBX
         PUSH      EDX

        MOV       EBX, EAX
        CALL      TControl.GetClientHeight
          PUSH      EAX
        MOV       EAX, EBX
        CALL      TControl.GetHeight // EAX = Height

          POP       EDX              // EDX = ClientHeight
        SUB       EAX, EDX           // EAX = Delta
         POP       EDX               // EDX = Value
        ADD       EDX, EAX           // EDX = Value + Delta
        XCHG      EAX, EBX           // EAX = @Self
        CALL      TControl.SetHeight
        POP       EBX
end;

procedure TControl.SetClientWidth(const Value: Integer);
asm
        PUSH      EBX
         PUSH      EDX

        MOV       EBX, EAX
        CALL      TControl.GetClientWidth
          PUSH      EAX
        MOV       EAX, EBX
        CALL      TControl.GetWidth  // EAX = Width

          POP       EDX              // EDX = ClientWidth
        SUB       EAX, EDX           // EAX = Width - ClientWidth
         POP       EDX               // EDX = Value
        ADD       EDX, EAX           // EDX = Value + Delta
        XCHG      EAX, EBX           // EAX = @Self
        CALL      TControl.SetWidth
        POP       EBX
end;

function TControl.CenterOnParent: PControl;
asm
        PUSHAD

        XCHG     ESI, EAX
        MOV      ECX, [ESI].fParent
        JECXZ    @@1
        {$IFDEF  USE_FLAGS}
        TEST     [ESI].fFlagsG3, 1 shl G3_IsControl
        {$ELSE}
        CMP      [ESI].fIsControl, 0
        {$ENDIF}
        JNZ      @@2

@@1:
        PUSH     SM_CYSCREEN
        CALL     GetSystemMetrics
        PUSH     EAX

        PUSH     SM_CXSCREEN
        CALL     GetSystemMetrics
        PUSH     EAX

        PUSH     0
        PUSH     0               // ESP -> Rect( 0, 0, CX, CY )

        JMP      @@3

@@2:    ADD      ESP, -size_TRect
        MOV      EDX, ESP
        XCHG     EAX, ECX
        CALL     TControl.ClientRect
                                 // ESP -> ClientRect
@@3:    MOV      EAX, ESI
        CALL     GetWindowHandle

        MOV      EAX, ESI
        CALL     GetWidth

        POP      EDX       // left
        ADD      EAX, EDX          // + width

        POP      EDI       // top
        POP      EDX       // right

        SUB      EDX, EAX
        SAR      EDX, 1

        MOV      EAX, ESI
        CALL     SetLeft

        MOV      EAX, ESI
        CALL     GetHeight

        ADD      EAX, EDI  // height + top

        POP      EDX       // bottom
        SUB      EDX, EAX
        SAR      EDX, 1

        XCHG     EAX, ESI
        CALL     SetTop

        POPAD
end;

function TControl.GetHasBorder: Boolean;
const style_mask = WS_BORDER or WS_THICKFRAME or WS_DLGFRAME;
asm
        CALL     UpdateWndStyles
        MOV      EDX, [EAX].fStyle
        AND      EDX, style_mask
        SETNZ    DL
        MOV      EAX, [EAX].fExStyle
        AND      EAX, WS_EX_CLIENTEDGE
        SETNZ    AL
        OR       AL, DL
end;

function TControl.GetHasCaption: Boolean;
const style_mask1 = (WS_POPUP or WS_DLGFRAME) shr 16;
      style_mask2 = WS_CAPTION shr 16;
asm
        CALL     UpdateWndStyles
        MOV      ECX, [EAX].fStyle + 2
        MOV      EDX, ECX
        MOV      AL, 1
        AND      DX, style_mask1
        JZ       @@1
        AND      CX, style_mask2
        JNZ      @@1
        XOR      EAX, EAX
@@1:
end;

procedure TControl.SetHasCaption(const Value: Boolean);
const style_mask = not (WS_BORDER or WS_THICKFRAME or WS_DLGFRAME or WS_CAPTION
                             or WS_MINIMIZEBOX or WS_MAXIMIZEBOX or WS_SYSMENU);
      exstyle_mask = not (WS_EX_CONTROLPARENT or WS_EX_DLGMODALFRAME
                                or WS_EX_WINDOWEDGE or WS_EX_CLIENTEDGE);
asm
        PUSH     EAX
          PUSH     EDX

            CALL     GetHasCaption
          POP      ECX
          CMP      AL, CL

        POP      EAX
        JZ       @@exit   // Value = HasCaption

        MOV      EDX, [EAX].fStyle
        DEC      CL
        JNZ      @@1      // if not Value -> @@1

        AND      EDX, not WS_POPUP
        OR       EDX, WS_CAPTION
        JMP      @@set_style

@@1:
        {$IFDEF  USE_FLAGS}
        TEST     [EAX].fFlagsG3, 1 shl G3_IsControl
        {$ELSE}
        CMP      [EAX].fIsControl, 0
        {$ENDIF}
        JNZ      @@2               // if fIsControl -> @@2

        AND      EDX, not (WS_CAPTION or WS_SYSMENU)
        OR       EDX, WS_POPUP
        JMP      @@3

@@2:
        AND      EDX, not WS_CAPTION
        OR       EDX, WS_DLGFRAME

@@3:
        PUSH     EDX

        MOV      EDX, [EAX].fExStyle
        OR       EDX, WS_EX_DLGMODALFRAME

        PUSH     EAX
        CALL     SetExStyle
        POP      EAX

        POP      EDX
@@set_style:
        CALL     SetStyle
@@exit:
end;

function TControl.GetCanResize: Boolean;
asm
    {$IFDEF USE_FLAGS}
        TEST    [EAX].fFlagsG1, (1 shl G1_PreventResize)
        SETZ    AL
    {$ELSE}
        MOV      AL, [EAX].fPreventResize
        {$IFDEF PARANOIA} DB $34,$01 {$ELSE} XOR AL, 1 {$ENDIF}
    {$ENDIF USE_FLAGS}
end;

procedure TControl.SetCanResize( const Value: Boolean );
asm
        PUSH     EBX
        MOV      EBX, EAX

            CALL     GetCanResize
        CMP      AL, DL

        JZ       @@exit   // Value = CanResize
        {$IFDEF  USE_FLAGS}
        // AL:bit0 = can resize
        SHL      AL, G1_PreventResize
        AND      [EBX].fFlagsG1, not (1 shl G1_PreventResize)
        OR       [EBX].fFlagsG1, AL
        {$ELSE}
        MOV      [EBX].fPreventResize, AL
        {$ENDIF  USE_FLAGS}
        {$IFDEF CANRESIZE_THICKFRAME}
        TEST     DL, DL

        MOV      EDX, [EBX].fStyle
        JZ       @@set_thick

        OR       EDX, WS_THICKFRAME
        JMP      @@set_style

@@set_thick:
        AND      EDX, not WS_THICKFRAME

@@set_style:
        MOV      EAX, EBX
        CALL     SetStyle
        {$ENDIF CANRESIZE_THICKFRAME}

        {$IFDEF  FIX_WIDTH_HEIGHT}
        MOV      EAX, EBX
        CALL     GetWindowHandle

        MOV      EAX, EBX
        CALL     GetWidth
        MOV      [EBX].FFixWidth, EAX

        MOV      EAX, EBX
        CALL     GetHeight
        MOV      [EBX].FFixHeight, EAX
        {$ENDIF  FIX_WIDTH_HEIGHT}

        XCHG     EAX, EBX
        MOV      EDX, offset[WndProcCanResize]
        CALL     TControl.AttachProc
@@exit:
        POP      EBX
end;

function TControl.GetStayOnTop: Boolean;
asm
        CALL     UpdateWndStyles
        TEST     byte ptr [EAX].fExStyle, WS_EX_TOPMOST
        SETNZ    AL
end;

procedure TControl.SetStayOnTop(const Value: Boolean);
asm
        PUSH     EAX
          PUSH     EDX

            CALL     GetStayOnTop
          POP      ECX
          MOVZX    ECX, CL
          CMP      AL, CL

        POP      EAX
        JZ       @@exit   // Value = StayOnTop

        MOV      EDX, [EAX].fHandle
        TEST     EDX, EDX
        JZ       @@1

        PUSH     SWP_NOMOVE or SWP_NOSIZE or SWP_NOACTIVATE
        XOR      EAX, EAX
        PUSH     EAX
        PUSH     EAX
        PUSH     EAX
        PUSH     EAX
        DEC      ECX
        DEC      ECX
        PUSH     ECX

        PUSH     EDX
        CALL     SetWindowPos
        RET

@@1:
        JECXZ    @@1and

        OR       byte ptr [EAX].fExStyle, WS_EX_TOPMOST
        RET

@@1and: AND      byte ptr [EAX].fExStyle, not WS_EX_TOPMOST

@@exit:
end;

function TControl.UpdateWndStyles: PControl;
asm
        MOV      ECX, [EAX].fHandle
        JECXZ    @@exit

        PUSH     EBX

        XCHG     EBX, EAX
          PUSH     GCL_STYLE
          PUSH     ECX

          PUSH     GWL_EXSTYLE
          PUSH     ECX

          PUSH     GWL_STYLE
          PUSH     ECX

          CALL     GetWindowLong
          MOV      [EBX].fStyle, EAX

          CALL     GetWindowLong
          MOV      [EBX].fExStyle, EAX

          CALL     GetClassLong
          MOV      [EBX].fClsStyle, EAX
        XCHG     EAX, EBX
        POP      EBX
@@exit:
end;

function TControl.GetChecked: Boolean;
asm
        TEST     [EAX].DF.fBitBtnOptions, 8 //1 shl Ord(bboFixed)
        JZ       @@1
        {$IFDEF  USE_FLAGS}
        TEST     [EAX].fFlagsG4, 1 shl G4_Checked
        SETNZ    AL
        {$ELSE}
        MOV      AL, [EAX].fChecked
        {$ENDIF}
        RET
@@1:
        PUSH     0
        PUSH     0
        PUSH     BM_GETCHECK
        PUSH     EAX
        CALL     Perform
@@exit:
end;

procedure TControl.Set_Checked(const Value: Boolean);
asm
        TEST     [EAX].DF.fBitBtnOptions, 8 //1 shl Ord(bboFixed)
        JZ       @@1
        {$IFDEF  USE_FLAGS}
        SHL      DL, G4_Checked
        AND      [EAX].fFlagsG4, not(1 shl G4_Checked)
        OR       [EAX].fFlagsG4, DL
        {$ELSE}
        MOV      [EAX].fChecked, DL
        {$ENDIF}
        JMP      Invalidate
@@1:
        PUSH     0
        MOVZX    EDX, DL
        PUSH     EDX
        PUSH     BM_SETCHECK
        PUSH     EAX
        Call     Perform
end;

function TControl.SetRadioChecked: PControl;
asm
  {$IFDEF USE_FLAGS}
  PUSH  DWORD PTR[EAX].fStyle
  PUSH  EAX
  AND   [EAX].fStyle.f2_Style, not(1 shl F2_Tabstop)
  CALL  DoClick
  POP   EAX
  POP   DWORD PTR[EAX].fStyle
  {$ELSE}
  PUSH  EAX
  PUSH  DWORD PTR[EAX].fTabStop
  MOV   [EAX].fTabStop, 0
@@1:
  CALL  DoClick
  POP   EDX
  POP   EAX
  MOV   [EAX].fTabStop, DL
  {$ENDIF USE_FLAGS}
end;

function TControl.GetSelStart: Integer;
asm
         {$IFDEF  COMMANDACTIONS_OBJ}
         MOV      ECX, [EAX].fCommandActions
         MOVZX    ECX, [ECX].TCommandActionsObj.aGetSelRange
         {$ELSE}
         MOVZX    ECX, [EAX].fCommandActions.aGetSelRange
         {$ENDIF}
         JECXZ    @@exit
         XOR      EDX, EDX
         PUSH     EDX // space for Result
         PUSH     EDX // 0
         LEA      EDX, [ESP+4]
         PUSH     EDX // @ Result
         PUSH     ECX // EM_GETSEL
         PUSH     EAX
         CALL     Perform
         POP      ECX // Result
@@exit:
         XCHG     EAX, ECX
end;

function TControl.GetSelLength: Integer;
asm
        XOR       EDX, EDX
        {$IFDEF   COMMANDACTIONS_OBJ}
        MOV       ECX, [EAX].fCommandActions
        MOVZX     ECX, word ptr[ECX].TCommandActionsObj.aGetSelCount
        {$ELSE}
        MOVZX     ECX, word ptr[EAX].fCommandActions.aGetSelCount
        {$ENDIF}
        JECXZ     @@ret_ecx

        CMP       CX, EM_GETSEL
        JNZ       @@1
        PUSH      EDX
        PUSH      EDX
        MOV       EDX, ESP
        PUSH      EDX
        ADD       EDX, 4
        PUSH      EDX
        PUSH      ECX
        PUSH      EAX
        CALL      Perform
        POP       ECX
        POP       EDX
        SUB       ECX, EDX
@@ret_ecx:
        XCHG      EAX, ECX
        RET

@@1:    // LB_GETSELCOUNT, LVM_GETSELECTEDCOUNT
        PUSH      EDX // 0
        PUSH      EDX // 0
        PUSH      ECX // aGetSelCount
        PUSH      EAX // Handle
        CALL      Perform
@@fin_EAX:
end;

procedure TControl.SetSelLength(const Value: Integer);
asm
        PUSH     EBP
        MOV      EBP, ESP
        PUSH     EAX
        PUSH     EDX
        CALL     GetSelStart
        POP      ECX
        POP      EDX
        ADD      ECX, EAX
        PUSH     ECX
        {$IFDEF  COMMANDACTIONS_OBJ}
        MOV      ECX, [EDX].fCommandActions
        MOVZX    ECX, [ECX].TCommandActionsObj.aSetSelRange
        {$ELSE}
        MOVZX    ECX, [EDX].fCommandActions.aSetSelRange
        {$ENDIF}
        JECXZ    @@check_ex
        PUSH     EAX
        JMP      @@perform

@@check_ex:
        {$IFDEF  COMMANDACTIONS_OBJ}
        MOV      ECX, [EDX].fCommandActions
        MOVZX    ECX, [ECX].TCommandActionsObj.aExSetSelRange
        {$ELSE}
        MOVZX    ECX, [EDX].fCommandActions.aExSetSelRange
        {$ENDIF}
        JECXZ    @@exit
        PUSH     EAX
        PUSH     ESP
        PUSH     0
@@perform:
        PUSH     ECX
        PUSH     EDX
        CALL     Perform
@@exit: MOV      ESP, EBP
        POP      EBP
end;

function TControl.GetItemsCount: Integer;
asm
        PUSH     0
        {$IFDEF  COMMANDACTIONS_OBJ}
        MOV      ECX, [EAX].fCommandActions
        MOVZX    ECX, [ECX].TCommandActionsObj.aGetCount
        {$ELSE}
        MOVZX    ECX, [EAX].fCommandActions.aGetCount
        {$ENDIF}
        JECXZ    @@ret_0
        PUSH     0
        PUSH     ECX
        PUSH     EAX
        CALL     Perform
        PUSH     EAX

@@ret_0:
        POP      EAX
end;

procedure HelpConvertItem2Pos;
asm
        JECXZ     @@exit
        PUSH      0
        PUSH      EDX
        PUSH      ECX
        PUSH      EAX
        CALL      TControl.Perform
        {XOR       EDX, EDX
        TEST      EAX, EAX
        JL        @@exit
        RET}
        XCHG      EDX, EAX
@@exit:
        XCHG      EAX, EDX
end;

function TControl.Item2Pos(ItemIdx: Integer): DWORD;
asm
        {$IFDEF   COMMANDACTIONS_OBJ}
        MOV       ECX, [EAX].fCommandActions
        MOVZX     ECX, [ECX].TCommandActionsObj.bItem2Pos
        {$ELSE}
        MOVZX     ECX, BYTE PTR [EAX].fCommandActions.bItem2Pos
        {$ENDIF}
        JMP       HelpConvertItem2Pos
end;

function TControl.Pos2Item(Pos: Integer): DWORD;
asm
        {$IFDEF   COMMANDACTIONS_OBJ}
        MOV       ECX, [EAX].fCommandActions
        MOVZX     ECX, [ECX].TCommandActionsObj.bPos2Item
        {$ELSE}
        MOVZX     ECX, BYTE PTR [EAX].fCommandActions.bPos2Item
        {$ENDIF}
        JMP       HelpConvertItem2Pos
end;

procedure TControl.Delete(Idx: Integer);
asm
        {$IFDEF  COMMANDACTIONS_OBJ}
        MOV      ECX, [EAX].fCommandActions
        MOVZX    ECX, [ECX].TCommandActionsObj.aDeleteItem
        {$ELSE}
        MOVZX    ECX, [EAX].fCommandActions.aDeleteItem
        {$ENDIF}
        JECXZ    @@exit

        PUSH     0
        PUSH     EDX
        PUSH     ECX
        PUSH     EAX
        CALL     Perform
@@exit:
end;

function TControl.GetItemSelected(ItemIdx: Integer): Boolean;
asm
        {$IFDEF  COMMANDACTIONS_OBJ}
        MOV      ECX, [EAX].fCommandActions
        MOVZX    ECX, [ECX].TCommandActionsObj.aGetSelected
        {$ELSE}
        MOVZX    ECX, [EAX].fCommandActions.aGetSelected
        {$ENDIF}
        JECXZ    @@check_range

        PUSH     1
        CMP      CL, CB_GETCURSEL and $FF
        JNZ      @@1
        MOV      [ESP], EDX
@@1:
        PUSH     LVIS_SELECTED // 2
        PUSH     EDX
        PUSH     ECX
        PUSH     EAX
        CALL     Perform
        POP      EDX
        CMP      EAX, EDX
        SETZ     AL
        RET

@@check_range:
        PUSH     EBX
        PUSH     ESI
        XCHG     ESI, EDX
        MOV      EBX, EAX

        CALL     GetSelStart
        XCHG     EBX, EAX
        CALL     GetSelLength

        SUB      ESI, EBX
        JL       @@ret_false

        CMP      EAX, ESI
@@ret_false:
        SETGE    AL
        POP      ESI
        POP      EBX
end;

procedure TControl.SetItemSelected(ItemIdx: Integer; const Value: Boolean);
asm
        PUSH     EDX
        PUSH     ECX
        {$IFDEF  COMMANDACTIONS_OBJ}
        MOV      ECX, [EAX].fCommandActions
        MOVZX    ECX, [ECX].TCommandActionsObj.aSetSelected
        {$ELSE}
        MOVZX    ECX, [EAX].fCommandActions.aSetSelected
        {$ENDIF}
        JECXZ    @@chk_aSetCurrent

@@0:
        PUSH     ECX
        PUSH     EAX
        CALL     Perform
        RET

@@chk_aSetCurrent:
        POP      ECX
        {$IFDEF  COMMANDACTIONS_OBJ}
        MOV      ECX, [EAX].fCommandActions
        MOVZX    ECX, [ECX].TCommandActionsObj.aSetCurrent
        {$ELSE}
        MOVZX    ECX, [EAX].fCommandActions.aSetCurrent
        {$ENDIF}
        JECXZ    @@chk_aSetSelRange

        POP      EDX
        PUSH     0
        JMP      @@3

@@chk_aSetSelRange:
        {$IFDEF  COMMANDACTIONS_OBJ}
        MOV      ECX, [EAX].fCommandActions
        MOVZX    ECX, [ECX].TCommandActionsObj.aSetSelRange
        {$ELSE}
        MOVZX    ECX, [EAX].fCommandActions.aSetSelRange
        {$ENDIF}
        JECXZ    @@chk_aExSetSelRange
@@3:
        PUSH     EDX
        JMP      @@0

@@else: MOV      [EAX].FCurIndex, EDX
        CALL     TControl.Invalidate
        JMP      @@exit

@@chk_aExSetSelRange:
        {$IFDEF  COMMANDACTIONS_OBJ}
        MOV      ECX, [EAX].fCommandActions
        MOVZX    ECX, [ECX].TCommandActionsObj.aExSetSelRange
        {$ELSE}
        MOVZX    ECX, [EAX].fCommandActions.aExSetSelRange
        {$ENDIF}
        JECXZ    @@else

        PUSH     EDX
        PUSH     ESP
        PUSH     0
        PUSH     ECX
        PUSH     EAX
        CALL     Perform
        POP      ECX

@@exit:
        POP      ECX
end;

procedure TControl.SetCtl3D(const Value: Boolean);
asm
        AND      [EAX].fCtl3D_child, not 1
        OR       [EAX].fCtl3D_child, DL

        PUSHAD
        CALL     UpdateWndStyles
        POPAD

        MOV      ECX, [EAX].fExStyle
        DEC      DL
        MOV      EDX, [EAX].fStyle
        JNZ      @@1
        AND      EDX, not WS_BORDER
        OR       CH, WS_EX_CLIENTEDGE shr 8
        JMP      @@2
@@1:
        OR       EDX, WS_BORDER
        AND      CH, not(WS_EX_CLIENTEDGE shr 8)
@@2:
        PUSH     ECX
        PUSH     EAX
        CALL     SetStyle
        POP      EAX
        POP      EDX
        JMP      SetExStyle
@@exit:
end;

function TControl.Shift(dX, dY: Integer): PControl;
asm
        PUSHAD
        ADD      EDX, [EAX].fBoundsRect.TRect.Left
        CALL     SetLeft
        POPAD
        PUSH     EAX
        MOV      EDX, [EAX].fBoundsRect.TRect.Top
        ADD      EDX, ECX
        CALL     SetTop
        POP      EAX
end;

function Tabulate2Control( Self_: PControl; Key: DWORD; checkOnly: Boolean ): Boolean;
const tk_Tab = 1;
      tk_LR  = 2;
      tk_UD  = 4;
      tk_PuPd= 8;
asm
        PUSH     EBX
        XCHG     EBX, EAX
        PUSH     ESI
        MOV      ESI, offset[@@data]
        XOR      EAX, EAX
@@loop:
        LODSW
        TEST     EAX, EAX
        JZ       @@exit_false

        CMP      AL, DL
        JNZ      @@loop

        TEST     [EBX].TControl.fLookTabKeys, AH
        JZ       @@exit_false

        TEST     CL, CL
        JNZ      @@exit_true

        MOV      DH, AH
        PUSH     EDX
            XCHG     EAX, EBX
            CALL     TControl.ParentForm
            XCHG     ESI, EAX
        POP      EAX

        CMP      AL, 9
        JNZ      @@test_flag

        PUSH     EAX
            PUSH     VK_SHIFT
            CALL     GetKeyState
        POP      EDX

        AND      AH, $80
        OR       AH, DH
@@test_flag:
        {XOR      EDX, EDX
        INC      EDX
        ADD      AH, AH
        JNC      @@tabul_1
        NEG      EDX
@@tabul_1:}                           //AH<80  //AH>=80
        ADD      AH, AH               //       //
        SBB      EDX, EDX             //EDX=0  //EDX=-1
        ADD      EDX, EDX             //    0  //    -2
        INC      EDX                  //    1  //    -1

        XCHG     EAX, ESI
        CALL     Tabulate2Next
@@exit_true:
        MOV      AL, 1
        POP      ESI
        POP      EBX
        RET

@@data:
        DB       VK_TAB, tk_Tab, VK_LEFT, tk_LR or $80, VK_RIGHT, tk_LR
        DB       VK_UP, tk_UD or $80, VK_DOWN, tk_UD
        DB       VK_PRIOR, tk_PuPd or $80, VK_NEXT, tk_PuPd, 0, 0

@@exit_false:
        XOR      EAX, EAX
        POP      ESI
        POP      EBX
        RET
end;

function TControl.Tabulate: PControl;
asm
        PUSH     EAX
        CALL     ParentForm
        TEST     EAX, EAX
        JZ       @@exit
        MOV      [EAX].PP.fGotoControl, offset[Tabulate2Control]
@@exit: POP      EAX
end;

function TControl.TabulateEx: PControl;
asm
        PUSH     EAX
        CALL     ParentForm
        TEST     EAX, EAX
        JZ       @@exit
        MOV      [EAX].PP.fGotoControl, offset[Tabulate2ControlEx]
@@exit: POP      EAX
end;

function TControl.GetCurIndex: Integer;
asm
        PUSH     EBX
        XCHG     EBX, EAX
        MOV      EAX, [EBX].fCurIndex
        {$IFDEF  COMMANDACTIONS_OBJ}
        MOV      ECX, [EBX].fCommandActions
        MOVZX    ECX, [ECX].TCommandActionsObj.aGetCurrent
        {$ELSE}
        MOVZX    ECX, [EBX].fCommandActions.aGetCurrent
        {$ENDIF}
        JECXZ    @@exit
        XOR      EAX, EAX
        CDQ
        CMP      CX, LVM_GETNEXTITEM
        JNE      @@0
        INC      EAX
        INC      EAX
        JMP      @@1
@@0:
        CMP      CL, EM_LINEINDEX and $FF
        JNZ      @@2
@@1:
        DEC      EDX
@@2:
        PUSH     EAX
        PUSH     EDX
        PUSH     ECX
        PUSH     EBX
        CALL     Perform

@@exit: POP      EBX
end;

{procedure TControl.SetCurIndex(const Value: Integer);
asm
        MOVZX    ECX, [EAX].fCommandActions.aSetCurrent
        JECXZ    @@set_item_sel
        PUSHAD
        PUSH     0
        PUSH     EDX
        PUSH     ECX
        PUSH     EAX
        CALL     Perform
        POPAD
        CMP      CX, TCM_SETCURSEL
        JNE      @@exit
        PUSH     TCN_SELCHANGE
        PUSH     EAX // idfrom doesn't matter
        PUSH     [EAX].fHandle
        PUSH     ESP
        PUSH     0
        PUSH     WM_NOTIFY
        PUSH     EAX
        CALL     Perform
        POP      ECX
        POP      ECX
        POP      ECX
@@exit:
        RET
@@set_item_sel:
        INC      ECX
        CALL     SetItemSelected
end;}

procedure TControl.SetCurIndex(const Value: Integer);                           // fix av
asm
       {$IFDEF  COMMANDACTIONS_OBJ}
       MOV      ECX, [EAX].fCommandActions
       MOVZX    ECX, [ECX].TCommandActionsObj.aSetCurrent
       {$ELSE}
       MOVZX    ECX, [EAX].fCommandActions.aSetCurrent
       {$ENDIF}
       JECXZ    @@set_item_sel
       PUSH     ECX            //+aSetCurrent
       PUSH     EAX            //+self
       PUSH     0
       PUSH     EDX
       PUSH     ECX
       PUSH     EAX
       CALL     Perform
       POP      EDX            //+self
       POP      ECX            //+aSetCurrent
       CMP      CX, TCM_SETCURSEL
       JNE      @@exit
       MOV      [EDX].fCurIndex,EAX
       PUSH     TCN_SELCHANGE  // NMHdr.code
       PUSH     EDX            // NMHdr.idfrom - doesn't matter
       PUSH     [EDX].fHandle  // NMHdr.hwndFrom
       PUSH     ESP
       PUSH     0
       PUSH     WM_NOTIFY
       PUSH     EDX
       CALL     Perform
       ADD      ESP,12         //NMHdr destroy
@@exit:
       RET
@@set_item_sel:
       INC      ECX
       CALL     SetItemSelected
end;

function TControl.GetTextAlign: TTextAlign;
asm
        PUSH     EAX
        CALL     UpdateWndStyles
        MOV      ECX, [EAX].fStyle
        {$IFDEF  COMMANDACTIONS_OBJ}
        MOV      EDX, [EAX].fCommandActions
        MOV      EDX, dword ptr [EDX].TCommandActionsObj.aTextAlignRight
        {$ELSE}
        MOV      EDX, dword ptr [EAX].fCommandActions.aTextAlignRight
        {$ENDIF}
        XOR      EAX, EAX
        AND      DX, CX
        JNZ      @@ret_1
        SHR      EDX, 16
        AND      ECX, EDX
        JNZ      @@ret_2
        POP      EAX
        MOVZX    EAX, [EAX].fTextAlign
        RET

@@ret_2:INC      EAX
@@ret_1:INC      EAX
@@ret_0:POP      ECX
end;

procedure TControl.SetTextAlign(const Value: TTextAlign);
asm
        {$IFDEF  COMMANDACTIONS_OBJ}
        PUSH     EBX
        {$ENDIF}
        MOV      [EAX].fTextAlign, DL
        XOR      ECX, ECX
        {$IFDEF  COMMANDACTIONS_OBJ}
        MOV      EBX, [EAX].fCommandActions
        MOV      CX, [EBX].TCommandActionsObj.aTextAlignLeft
        OR       CX, [EBX].TCommandActionsObj.aTextAlignCenter
        OR       CX, [EBX].TCommandActionsObj.aTextAlignRight
        {$ELSE}
        MOV      CX, [EAX].fCommandActions.aTextAlignLeft
        OR       CX, [EAX].fCommandActions.aTextAlignCenter
        OR       CX, [EAX].fCommandActions.aTextAlignRight
        {$ENDIF}
        NOT      ECX
        AND      ECX, [EAX].fStyle

        AND      EDX, 3
        {$IFDEF  COMMANDACTIONS_OBJ}
        OR       CX, [EBX + EDX * 2].TCommandActionsObj.aTextAlignLeft
        MOV      DL, BYTE PTR [EBX].TCommandActionsObj.bTextAlignMask
        {$ELSE}
        OR       CX, [EAX + EDX * 2].fCommandActions.aTextAlignLeft
        MOV      DL, BYTE PTR [EAX].fCommandActions.bTextAlignMask
        {$ENDIF}

        NOT      EDX
        AND      EDX, ECX
        CALL     SetStyle
        {$IFDEF  COMMANDACTIONS_OBJ}
        POP      EBX
        {$ENDIF}
end;

function TControl.GetVerticalAlign: TVerticalAlign;
asm
        PUSH     EAX
        CALL     UpdateWndStyles
        {$IFDEF  COMMANDACTIONS_OBJ}
        MOV      EDX, [EAX].fCommandActions
        MOV      EDX, dword ptr [EDX].TCommandActionsObj.bVertAlignCenter
        {$ELSE}
        MOV      EDX, dword ptr [EAX].fCommandActions.bVertAlignCenter
        {$ENDIF}
        MOV      ECX, [EAX].fStyle
        XOR      EAX, EAX
        MOV      DH, DL
        AND      DL, CH
        JZ       @@1
        CMP      DL, DH
        JE       @@ret_0
@@1:    SHR      EDX, 16
        MOV      DH, DL
        AND      DL, CH
        JZ       @@2
        CMP      DL, DH
        JE       @@ret_2
@@2:    POP      EAX
        MOVZX    EAX, [EAX].fVerticalAlign
        RET
@@ret_2:INC      EAX
@@ret_1:INC      EAX
@@ret_0:POP      ECX
end;

procedure TControl.SetVerticalAlign(const Value: TVerticalAlign);
asm
        MOVZX    EDX, DL
        MOV      [EAX].fVerticalAlign, DL

        {$IFDEF  COMMANDACTIONS_OBJ}
        MOV      ECX, [EAX].fCommandActions
        MOVZX    ECX, byte ptr [ECX+EDX].TCommandActionsObj.bVertAlignTop
        {$ELSE}
        MOVZX    ECX, byte ptr [EAX+EDX].fCommandActions.bVertAlignTop
        {$ENDIF}
        SHL      ECX, 8

        MOV      EDX, [EAX].fStyle
        AND      DH, $F3
        OR       EDX, ECX

        CALL     SetStyle
end;

function TControl.Dc2Canvas( Sender: PCanvas ): HDC;
asm
        MOV      ECX, [EAX].fPaintDC
        JECXZ    @@chk_fHandle

        PUSH     ECX
        XCHG     EAX, EDX // EAX <= Sender
        MOV      EDX, ECX // EDX <= fPaintDC
        PUSH     EAX
        CALL     TCanvas.SetHandle
        POP      EAX
        MOV      [EAX].TCanvas.fIsPaintDC, 1
        POP      ECX
@@ret_ECX:
        XCHG     EAX, ECX
        RET

@@chk_fHandle:
        MOV      ECX, [EDX].TCanvas.fHandle
        INC      ECX
        LOOP     @@ret_ECX

        CALL     GetWindowHandle
        PUSH     EAX
        CALL     GetDC
end;

function TControl.GetCanvas: PCanvas;
asm
        PUSH     EBX
        PUSH     ESI
        {$IFDEF  SAFE_CODE}
        MOV      EBX, EAX
        CALL     CreateWindow
        {$ELSE}
        XCHG     EBX, EAX
        {$ENDIF}

        MOV      ESI, [EBX].fCanvas
        TEST     ESI, ESI
        JNZ      @@exit

        XOR      EAX, EAX
        CALL     NewCanvas
        MOV      [EBX].fCanvas, EAX
        MOV      [EAX].TCanvas.fOwnerControl, EBX
        MOV      [EAX].TCanvas.fOnGetHandle.TMethod.Code, offset[ DC2Canvas ]
        MOV      [EAX].TCanvas.fOnGetHandle.TMethod.Data, EBX
        XCHG     ESI, EAX

        MOV      ECX, [EBX].fFont
        JECXZ    @@exit

        MOV      EAX, [ESI].TCanvas.fFont
        MOV      EDX, ECX
        CALL     TGraphicTool.Assign
        MOV      [ESI].TCanvas.fFont, EAX

        MOV      ECX, [EBX].fBrush
        JECXZ    @@exit

        MOV      EAX, [ESI].TCanvas.fBrush
        MOV      EDX, ECX
        CALL     TGraphicTool.Assign
        MOV      [ESI].TCanvas.fBrush, EAX

@@exit: XCHG     EAX, ESI
        POP      ESI
        POP      EBX
end;

procedure TControl.SetDoubleBuffered(const Value: Boolean);
asm
    {$IFDEF  USE_FLAGS}
    TEST  [EAX].fFlagsG1, 1 shl G1_CanNotDoubleBuf
    JNZ   @@exit
    {$ELSE}
    CMP   [EAX].fCannotDoubleBuf, 0
    JNZ   @@exit
    {$ENDIF}
    {$IFDEF USE_FLAGS}
    SHL   DL, G2_DoubleBuffered
    AND   [EAX].fFlagsG2, not(1 shl G2_DoubleBuffered)
    OR    [EAX].fFlagsG2, DL
    {$ELSE}
    MOV   [EAX].fDoubleBuffered, DL
    {$ENDIF}
    MOV   EDX, offset[WndProcTransparent]
    CALL  TControl.AttachProc
    {$IFnDEF SMALLEST_CODE}
    LEA   EAX, [TransparentAttachProcExtension]
    MOV   [Global_AttachProcExtension], EAX
    {$ENDIF}
@@exit:
end;

procedure TControl.SetTransparent(const Value: Boolean);
asm
        MOV      ECX, [EAX].fParent
        JECXZ    @@exit
        {$IFDEF  USE_FLAGS}
        AND      [EAX].fFlagsG2, not(1 shl G2_Transparent)
        TEST     DL, DL
        JZ       @@exit
        OR       [EAX].fFlagsG2, 1 shl G2_Transparent
        {$ELSE}
        MOV      [EAX].fTransparent, DL
        TEST     DL, DL
        JZ       @@exit
        {$ENDIF}

{$IFDEF GRAPHCTL_XPSTYLES}
        CMP      AppTheming, FALSE
        JNE      @@not_th
        {$IFDEF  USE_FLAGS}
        OR       [EAX].fFlagsG3, G3_ClassicTransparent
        {$ELSE}
        MOV      [EAX].fClassicTransparent, DL
        {$ENDIF  USE_FLAGS}
@@not_th:
{$ENDIF}

        PUSH     EAX
        XCHG     EAX, ECX
        CALL     SetDoubleBuffered
        POP      EAX
        MOV      EDX, offset[WndProcTransparent]
        CALL     AttachProc
@@exit:
end;

function _NewTrayIcon: PTrayIcon;
begin
  New(Result,Create);
end;
function NewTrayIcon( Wnd: PControl; Icon: HIcon ): PTrayIcon;
asm
        PUSH     EBX
        PUSH     EDX // push Icon
        PUSH     EAX // push Wnd
        CALL     _NewTrayIcon
        XCHG     EBX, EAX

        MOV      EAX, [FTrayItems]
        TEST     EAX, EAX
        JNZ      @@1
        CALL     NewList
        MOV      [FTrayItems], EAX
@@1:
        MOV      EDX, EBX
        CALL     TList.Add

        POP      EAX //Wnd
        MOV      [EBX].TTrayIcon.fControl, EAX
        POP      [EBX].TTrayIcon.fIcon //Icon

        MOV      EDX, offset[WndProcTray]
        TEST     EAX, EAX
        JZ       @@2
        CALL     TControl.AttachProc
@@2:
        MOV      DL, 1
        MOV      EAX, EBX
        CALL     TTrayIcon.SetActive
        XCHG     EAX, EBX
        POP      EBX
end;

function WndProcRecreateTrayIcons( Sender: PControl; var Msg: TMsg; var Rslt: Integer ): Boolean;
asm     //cmd    //opd
        MOV      ECX, [fRecreateMsg]
        CMP      word ptr [EDX].TMsg.message, CX
        JNE      @@ret_false
        PUSH     ESI
        MOV      ESI, [FTrayItems]
        MOV      ECX, [ESI].TList.fCount
        MOV      ESI, [ESI].TList.fItems
@@loo:  PUSH     ECX
        LODSD
        MOV      DL, [EAX].TTrayIcon.fAutoRecreate
        AND      DL, [EAX].TTrayIcon.fActive
        JZ       @@nx
        DEC      [EAX].TTrayIcon.fActive
        CALL     TTrayIcon.SetActive
@@nx:   POP      ECX
        LOOP     @@loo
@@e_loo:POP      ESI
@@ret_false:
        XOR      EAX, EAX
end;

procedure TTrayIcon.SetAutoRecreate(const Value: Boolean);
asm     //cmd    //opd
        MOV      [EAX].fAutoRecreate, DL
        MOV      EAX, [EAX].FControl
        CALL     TControl.ParentForm
        MOV      EDX, offset[WndProcRecreateTrayIcons]
        CALL     TControl.AttachProc
        PUSH     offset[TaskbarCreatedMsg]
        CALL     RegisterWindowMessage
        MOV      [fRecreateMsg], EAX
end;

destructor TTrayIcon.Destroy;
asm
        PUSH     EBX
        PUSH     ESI
        MOV      EBX, EAX
        XOR      EDX, EDX
        CALL     SetActive

        MOV      ECX, [EBX].fIcon
        JECXZ    @@icon_destroyed
        PUSH     ECX
        CALL     DestroyIcon
@@icon_destroyed:

        MOV      EDX, EBX
        MOV      ESI, [FTrayItems]
        MOV      EAX, ESI
        CALL     TList.IndexOf
        TEST     EAX, EAX
        JL       @@fin
          XCHG     EDX, EAX
          MOV      EAX, ESI
          CALL     TList.Delete
          MOV      EAX, [ESI].TList.fCount
          TEST     EAX, EAX
          JNZ      @@fin
          XCHG     EAX, [FTrayItems]
          CALL     TObj.RefDec
@@fin:  LEA      EAX, [EBX].FTooltip
        {$IFDEF UNICODE_CTRLS}
        CALL     System.@WStrClr
        {$ELSE}
        CALL     System.@LStrClr
        {$ENDIF}
        XCHG     EAX, EBX
        CALL     TObj.Destroy
        POP      ESI
        POP      EBX
end;

procedure TTrayIcon.SetActive(const Value: Boolean);
asm
        CMP      [EAX].fActive, DL
        JE       @@exit
        MOV      ECX, [EAX].fIcon
        JECXZ    @@exit
        PUSH     EDX
        PUSH     EAX
          MOV      ECX, [EAX].FWnd
          INC      ECX
          LOOP     @@1
          MOV      ECX, [EAX].fControl
          XOR      EAX, EAX
          JECXZ    @@1
          XCHG     EAX, ECX
          CALL     TControl.GetWindowHandle
@@1:
        POP      ECX
        POP      EDX
        XCHG     EAX, ECX
        JECXZ    @@exit
        MOV      [EAX].fActive, DL
        MOVZX    EDX, DL
        XOR      DL, 1
        ADD      EDX, EDX
        CALL     SetTrayIcon
@@exit:
end;

procedure TTrayIcon.SetIcon(const Value: HIcon);
asm
        MOV      ECX, [EAX].fIcon
        CMP      ECX, EDX
        JE       @@exit
        MOV      [EAX].fIcon, EDX
        XOR      EDX, EDX
        JECXZ    @@nim_add
        INC      EDX      // NIM_MODIFY = 1
@@nim_add:
        MOVZX    ECX, [EAX].fActive
        JECXZ    @@exit
        CALL     SetTrayIcon
@@exit:
end;

function WndProcJustOne( Control: PControl; var Msg: TMsg; var Rslt: Integer ) : Boolean;
asm
        MOV      ECX, [EDX].TMsg.message
        SUB      ECX, WM_CLOSE
        JE       @@1
        SUB      ECX, WM_NCDESTROY - WM_CLOSE
        JNE      @@exit
@@1:
        MOV      ECX, [EDX].TMsg.hwnd
        SUB      ECX, [EAX].TControl.fHandle
        JNE      @@exit

        XCHG     ECX, [JustOneMutex]
        JECXZ    @@exit

        PUSH     ECX
        CALL     CloseHandle

@@exit:
        XOR      EAX, EAX
end;

destructor TStrList.Destroy;
asm
        PUSH     EAX
        CALL     Clear
        POP      EAX
        CALL     TObj.Destroy
end;

function TStrList.Add(const S: Ansistring): integer;
asm
        MOV      ECX, EDX
        MOV      EDX, [EAX].fCount
        PUSH     EDX
        CALL     Insert
        POP      EAX
end;

procedure TStrList.AddStrings(Strings: PStrList);
asm
        PUSH     EAX
        XCHG     EAX, EDX
        PUSH     0
        MOV      EDX, ESP
        CALL     GetTextStr
        POP      EDX
        POP      EAX
        MOV      CL, 1
        PUSH     EDX
        CALL     SetText
        CALL     RemoveStr
end;

procedure TStrList.Assign(Strings: PStrList);
asm
        PUSHAD
        CALL     Clear
        POPAD
        JMP      AddStrings
end;

procedure TStrList.Clear;
asm
        PUSH     EBX
        XCHG     EBX, EAX
        MOV      EDX, [EBX].fCount
@@loo:  DEC      EDX
        JL       @@eloo
        PUSH     EDX
        MOV      EAX, EBX
        CALL     Delete
        POP      EDX
        JMP      @@loo
@@eloo:
        XOR      EAX, EAX
        MOV      [EBX].fTextSiz, EAX
        XCHG     EAX, [EBX].fTextBuf
        TEST     EAX, EAX
        JZ       @@1
        CALL     System.@FreeMem
        {$IFNDEF _D2orD3} //???//
        XOR      EAX, EAX // not needed for Delphi4 and Higher: if OK, EAX = 0
        {$ENDIF}
@@1:    XCHG     EAX, [EBX].fList
        CALL     TObj.RefDec
        POP      EBX
end;

{$IFDEF TStrList_Delete_ASM}
procedure TStrList.Delete(Idx: integer);
asm
        DEC      [EAX].fCount
        PUSH     EAX
        MOV      EAX, [EAX].fList
        MOV      ECX, [EAX].TList.fItems
        PUSH     dword ptr [ECX+EDX*4]
        CALL     TList.Delete
        POP      EAX
        POP      EDX
        MOV      ECX, [EDX].fTextSiz
        JECXZ    @@fremem
        CMP      EAX, [EDX].fTextBuf
        JB       @@fremem
        ADD      ECX, [EDX].fTextBuf
        CMP      EAX, ECX
        JB       @@exit
@@fremem:
        CALL     System.@FreeMem
@@exit:
end;
{$ENDIF}

function TStrList.Get(Idx: integer): Ansistring;
asm
        PUSH     ECX
        MOV      EAX, [EAX].fList
        TEST     EAX, EAX
        JZ       @@1
        CALL     TList.Get
@@1:    XCHG     EDX, EAX
        POP      EAX
        {$IFDEF _D2009orHigher}
        XOR      ECX, ECX // TODO: safe?
        {$ENDIF}
        JMP      System.@LStrFromPChar
end;

procedure TStrList.Insert(Idx: integer; const S: Ansistring);
asm
        PUSH     EBX
        PUSH     EDX
        PUSH     ECX
        XCHG     EBX, EAX
        MOV      EAX, [EBX].fList
        TEST     EAX, EAX
        JNZ      @@1
        CALL     NewList
        MOV      [EBX].fList, EAX
@@1:
        POP      EAX
        PUSH     EAX          // push S
        CALL     System.@LStrLen
        INC      EAX
        PUSH     EAX          // push L
        CALL     System.@GetMem
        MOV      byte ptr[EAX], 0
        XCHG     EDX, EAX
        POP      ECX
        POP      EAX
        PUSH     EDX          // push Mem
        TEST     EAX, EAX
        JE       @@2
        CALL     System.Move
@@2:    POP      ECX
        POP      EDX
        MOV      EAX, [EBX].fList
        CALL     TList.Insert
        INC      [EBX].fCount
        POP      EBX
end;

procedure TStrList.Put(Idx: integer; const Value: Ansistring);
asm
        PUSH     EAX
        PUSH     EDX
        CALL     Insert
        POP      EDX
        POP      EAX
        INC      EDX
        JMP      Delete
end;

procedure LowerCaseStrFromPCharEDX;
asm
          { <- EDX = PChar string
            -> [ESP] = LowerCase( PChar( EDX ) ),
               EAX, EDX, ECX - ?
          }
        POP      EAX
        PUSH     0
        PUSH     EAX
        LEA      EAX, [ESP+4]
        PUSH     EAX
        {$IFDEF _D2009orHigher}
        XOR      ECX, ECX // TODO: fixme
        {$ENDIF}
        CALL     System.@LStrFromPChar
        POP      EDX
        MOV      EAX, [EDX]
        JMP      LowerCase
end;

procedure TStrList.Sort(CaseSensitive: Boolean);
asm
        MOV      [EAX].fCaseSensitiveSort, DL
        MOV      [EAX].fAnsiSort, 0
  {$IFDEF SPEED_FASTER}
          {$DEFINE SORT_STRLIST_ARRAY}
  {$ENDIF}
  {$IFDEF TLIST_FAST}
          {$UNDEF SORT_STRLIST_ARRAY}
  {$ENDIF}
        {$IFDEF  SORT_STRLIST_ARRAY}
        MOV      ECX, offset[StrComp]
        CMP      DL, 0
        JNZ      @@01
        {$IFDEF  SMALLER_CODE}
        MOV      ECX, offset[StrComp_NoCase]
        {$ELSE}
        MOV      ECX, [StrComp_NoCase]
        {$ENDIF}
@@01:
        MOV      EAX, [EAX].fList
        TEST     EAX, EAX
        JZ       @@exit
        MOV      EDX, [EAX].TList.fCount
        CMP      EDX, 1
        JLE      @@02
        MOV      EAX, [EAX].TList.fItems
        CALL     SortArray
@@02:
        {$ELSE}
        PUSH     Offset[TStrList.Swap]
        MOV      ECX, Offset[CompareStrListItems_Case]
        CMP      DL, 0
        JNZ      @1
        MOV      ECX, Offset[CompareStrListItems_NoCase]
@1:     MOV      EDX, [EAX].fCount
        CALL     SortData
        {$ENDIF}
@@exit:
end;

procedure TStrList.MergeFromFile(const FileName: KOLString);
asm
        PUSH     EAX
        XCHG     EAX, EDX
        CALL     NewReadFileStream
        XCHG     EDX, EAX
        POP      EAX
        MOV      CL, 1
        PUSH     EDX
        CALL     LoadFromStream
        POP      EAX
        JMP      TObj.RefDec
end;

procedure TStrList.SaveToStream(Stream: PStream);
asm
        PUSH     EDX
        PUSH     0
        MOV      EDX, ESP
        CALL     GetTextStr
        POP      EAX
        PUSH     EAX
        CALL     System.@LStrLen
        XCHG     ECX, EAX
        POP      EDX
        POP      EAX
        PUSH     EDX
        JECXZ    @@1
        CALL     TStream.Write
@@1:
        CALL     RemoveStr
end;

procedure SortData( const Data: Pointer; const uNElem: Dword;
                    const CompareFun: TCompareEvent;
                    const SwapProc: TSwapEvent );
asm
        CMP      EDX, 2
        JL       @@exit

        PUSH     EAX      // [EBP-4] = Data
        PUSH     ECX      // [EBP-8] = CompareFun
        PUSH     EBX      // EBX = pivotP
        XOR      EBX, EBX
        INC      EBX      // EBX = 1 to pass to qSortHelp as PivotP
        MOV      EAX, EDX // EAX = nElem
        CALL     @@qSortHelp
        POP      EBX
        POP      ECX
        POP      ECX
@@exit:
        POP      EBP
        RET      4

@@qSortHelp:
        PUSH     EBX      // EBX (in) = PivotP
        PUSH     ESI      // ESI      = leftP
        PUSH     EDI      // EDI      = rightP

@@TailRecursion:
        CMP      EAX, 2
        JG       @@2
        JNE      @@exit_qSortHelp
        LEA      ECX, [EBX+1]
        MOV      EDX, EBX
        CALL     @@Compare
        JLE      @@exit_qSortHelp
@@swp_exit:
        CALL     @@Swap
@@exit_qSortHelp:
        POP      EDI
        POP      ESI
        POP      EBX
        RET

        // ESI = leftP
        // EDI = rightP
@@2:    LEA      EDI, [EAX+EBX-1]
        MOV      ESI, EAX
        SHR      ESI, 1
        ADD      ESI, EBX
        MOV      ECX, ESI
        MOV      EDX, EDI
        CALL     @@CompareLeSwap
        MOV      EDX, EBX
        CALL     @@Compare

        JG       @@4
        CALL     @@Swap
        JMP      @@5
@@4:    MOV      ECX, EBX
        MOV      EDX, EDI
        CALL     @@CompareLeSwap
@@5:
        CMP      EAX, 3
        JNE      @@6
        MOV      EDX, EBX
        MOV      ECX, ESI
        JMP      @@swp_exit
@@6:    // classic Horae algorithm

        PUSH     EAX     // EAX = pivotEnd
        LEA      EAX, [EBX+1]
        MOV      ESI, EAX
@@repeat:
        MOV      EDX, ESI
        MOV      ECX, EBX
        CALL     @@Compare
        JG       @@while2
@@while1:
        JNE      @@7
        MOV      EDX, ESI
        MOV      ECX, EAX
        CALL     @@Swap
        INC      EAX
@@7:
        CMP      ESI, EDI
        JGE      @@qBreak
        INC      ESI
        JMP      @@repeat
@@while2:
        CMP      ESI, EDI
        JGE      @@until
        MOV      EDX, EBX
        MOV      ECX, EDI
        CALL     @@Compare
        JGE      @@8
        DEC      EDI
        JMP      @@while2
@@8:
        MOV      EDX, ESI
        MOV      ECX, EDI
        PUSHFD
        CALL     @@Swap
        POPFD
        JE       @@until
        INC      ESI
        DEC      EDI
@@until:
        CMP      ESI, EDI
        JL       @@repeat
@@qBreak:
        MOV      EDX, ESI
        MOV      ECX, EBX
        CALL     @@Compare
        JG       @@9
        INC      ESI
@@9:
        PUSH     EBX      // EBX = PivotTemp
        PUSH     ESI      // ESI = leftTemp
        DEC      ESI
@@while3:
        CMP      EBX, EAX
        JGE      @@while3_break
        CMP      ESI, EAX
        JL       @@while3_break
        MOV      EDX, EBX
        MOV      ECX, ESI
        CALL     @@Swap
        INC      EBX
        DEC      ESI
        JMP      @@while3
@@while3_break:
        POP      ESI
        POP      EBX

        MOV      EDX, EAX
        POP      EAX     // EAX = nElem
        PUSH     EDI     // EDI = lNum
        MOV      EDI, ESI
        SUB      EDI, EDX
        ADD      EAX, EBX
        SUB      EAX, ESI

        PUSH     EBX
        PUSH     EAX
        CMP      EAX, EDI
        JGE      @@10

        MOV      EBX, ESI
        CALL     @@qSortHelp
        POP      EAX
        MOV      EAX, EDI
        POP      EBX
        JMP      @@11

@@10:   MOV      EAX, EDI
        CALL     @@qSortHelp
        POP      EAX
        POP      EBX
        MOV      EBX, ESI
@@11:
        POP      EDI
        JMP      @@TailRecursion

@@Compare:
        PUSH     EAX
        PUSH     EDX
        PUSH     ECX
        MOV      EAX, [EBP-4]
        DEC      EDX
        DEC      ECX
        CALL     dword ptr [EBP-8]
        POP      ECX
        POP      EDX
        TEST     EAX, EAX
        POP      EAX
        RET

@@CompareLeSwap:
        CALL     @@Compare
        JG       @@ret

@@Swap: PUSH     EAX
        PUSH     EDX
        PUSH     ECX
        MOV      EAX, [EBP-4]
        DEC      EDX
        DEC      ECX
        CALL     dword ptr [SwapProc]
        POP      ECX
        POP      EDX
        TEST     EAX, EAX
        POP      EAX
@@ret:
        RET

end;

procedure SortArray( const Data: Pointer; const uNElem: Dword;
                    const CompareFun: TCompareArrayEvent );
asm
        PUSH     EBP
        MOV      EBP, ESP
        CMP      EDX, 2
        JL       @@exit

        SUB      EAX, 4
        PUSH     EAX      // [EBP-4] = Data
        PUSH     ECX      // [EBP-8] = CompareFun
        PUSH     EBX      // EBX = pivotP
        XOR      EBX, EBX
        INC      EBX      // EBX = 1 to pass to qSortHelp as PivotP
        MOV      EAX, EDX // EAX = nElem
        CALL     @@qSortHelp
        POP      EBX
        POP      ECX
        POP      ECX
@@exit:
        POP      EBP
        RET      

@@qSortHelp:
        PUSH     EBX      // EBX (in) = PivotP
        PUSH     ESI      // ESI      = leftP
        PUSH     EDI      // EDI      = rightP

@@TailRecursion:
        CMP      EAX, 2
        JG       @@2
        JNE      @@exit_qSortHelp
        LEA      ECX, [EBX+1]
        MOV      EDX, EBX
        //CALL     @@Compare
        PUSH     EAX
        PUSH     EDX
        PUSH     ECX
        MOV      EAX, [EBP-4]
        MOV      EAX, [EAX + EDX*4]
        MOV      EDX, [EBP-4]
        MOV      EDX, [EDX + ECX*4]
        CALL     dword ptr [EBP-8]
        POP      ECX
        POP      EDX
        TEST     EAX, EAX
        POP      EAX

        JLE      @@exit_qSortHelp
@@swp_exit:
        //CALL     @@Swap
        PUSH     EAX
        PUSH     ESI
        MOV      ESI, [EBP-4]
        MOV      EAX, [ESI+EDX*4]
        XCHG     EAX, [ESI+ECX*4]
        MOV      [ESI+EDX*4], EAX
        POP      ESI
        POP      EAX

@@exit_qSortHelp:
        POP      EDI
        POP      ESI
        POP      EBX
        RET

        // ESI = leftP
        // EDI = rightP
@@2:    LEA      EDI, [EAX+EBX-1]
        MOV      ESI, EAX
        SHR      ESI, 1
        ADD      ESI, EBX
        MOV      ECX, ESI
        MOV      EDX, EDI
        CALL     @@CompareLeSwap
        MOV      EDX, EBX
        //CALL     @@Compare
        PUSH     EAX
        PUSH     EDX
        PUSH     ECX
        MOV      EAX, [EBP-4]
        MOV      EAX, [EAX + EDX*4]
        MOV      EDX, [EBP-4]
        MOV      EDX, [EDX + ECX*4]
        CALL     dword ptr [EBP-8]
        POP      ECX
        POP      EDX
        TEST     EAX, EAX
        POP      EAX

        JG       @@4
        //CALL     @@Swap
        PUSH     EAX
        PUSH     ESI
        MOV      ESI, [EBP-4]
        MOV      EAX, [ESI+EDX*4]
        XCHG     EAX, [ESI+ECX*4]
        MOV      [ESI+EDX*4], EAX
        POP      ESI
        POP      EAX

        JMP      @@5
@@4:    MOV      ECX, EBX
        MOV      EDX, EDI
        CALL     @@CompareLeSwap
@@5:
        CMP      EAX, 3
        JNE      @@6
        MOV      EDX, EBX
        MOV      ECX, ESI
        JMP      @@swp_exit
@@6:    // classic Horae algorithm

        PUSH     EAX     // EAX = pivotEnd
        LEA      EAX, [EBX+1]
        MOV      ESI, EAX
@@repeat:
        MOV      EDX, ESI
        MOV      ECX, EBX
        //CALL     @@Compare
        PUSH     EAX
        PUSH     EDX
        PUSH     ECX
        MOV      EAX, [EBP-4]
        MOV      EAX, [EAX + EDX*4]
        MOV      EDX, [EBP-4]
        MOV      EDX, [EDX + ECX*4]
        CALL     dword ptr [EBP-8]
        POP      ECX
        POP      EDX
        TEST     EAX, EAX
        POP      EAX

        JG       @@while2
@@while1:
        JNE      @@7
        MOV      EDX, ESI
        MOV      ECX, EAX
        //CALL     @@Swap
        PUSH     EAX
        PUSH     ESI
        MOV      ESI, [EBP-4]
        MOV      EAX, [ESI+EDX*4]
        XCHG     EAX, [ESI+ECX*4]
        MOV      [ESI+EDX*4], EAX
        POP      ESI
        POP      EAX

        INC      EAX
@@7:
        CMP      ESI, EDI
        JGE      @@qBreak
        INC      ESI
        JMP      @@repeat
@@while2:
        CMP      ESI, EDI
        JGE      @@until
        MOV      EDX, EBX
        MOV      ECX, EDI
        //CALL     @@Compare
        PUSH     EAX
        PUSH     EDX
        PUSH     ECX
        MOV      EAX, [EBP-4]
        MOV      EAX, [EAX + EDX*4]
        MOV      EDX, [EBP-4]
        MOV      EDX, [EDX + ECX*4]
        CALL     dword ptr [EBP-8]
        POP      ECX
        POP      EDX
        TEST     EAX, EAX
        POP      EAX

        JGE      @@8
        DEC      EDI
        JMP      @@while2
@@8:
        MOV      EDX, ESI
        MOV      ECX, EDI
        //PUSHFD
        //CALL     @@Swap
        PUSH     EAX
        PUSH     ESI
        MOV      ESI, [EBP-4]
        MOV      EAX, [ESI+EDX*4]
        XCHG     EAX, [ESI+ECX*4]
        MOV      [ESI+EDX*4], EAX
        POP      ESI
        POP      EAX

        //POPFD
        JE       @@until
        INC      ESI
        DEC      EDI
@@until:
        CMP      ESI, EDI
        JL       @@repeat
@@qBreak:
        MOV      EDX, ESI
        MOV      ECX, EBX
        //CALL     @@Compare
        PUSH     EAX
        PUSH     EDX
        PUSH     ECX
        MOV      EAX, [EBP-4]
        MOV      EAX, [EAX + EDX*4]
        MOV      EDX, [EBP-4]
        MOV      EDX, [EDX + ECX*4]
        CALL     dword ptr [EBP-8]
        POP      ECX
        POP      EDX
        TEST     EAX, EAX
        POP      EAX

        JG       @@9
        INC      ESI
@@9:
        PUSH     EBX      // EBX = PivotTemp
        PUSH     ESI      // ESI = leftTemp
        DEC      ESI
@@while3:
        CMP      EBX, EAX
        JGE      @@while3_break
        CMP      ESI, EAX
        JL       @@while3_break
        MOV      EDX, EBX
        MOV      ECX, ESI
        //CALL     @@Swap
        PUSH     EAX
        PUSH     ESI
        MOV      ESI, [EBP-4]
        MOV      EAX, [ESI+EDX*4]
        XCHG     EAX, [ESI+ECX*4]
        MOV      [ESI+EDX*4], EAX
        POP      ESI
        POP      EAX

        INC      EBX
        DEC      ESI
        JMP      @@while3
@@while3_break:
        POP      ESI
        POP      EBX

        MOV      EDX, EAX
        POP      EAX     // EAX = nElem
        PUSH     EDI     // EDI = lNum
        MOV      EDI, ESI
        SUB      EDI, EDX
        ADD      EAX, EBX
        SUB      EAX, ESI

        PUSH     EBX
        PUSH     EAX
        CMP      EAX, EDI
        JGE      @@10

        MOV      EBX, ESI
        CALL     @@qSortHelp
        POP      EAX
        MOV      EAX, EDI
        POP      EBX
        JMP      @@11

@@10:   MOV      EAX, EDI
        CALL     @@qSortHelp
        POP      EAX
        POP      EBX
        MOV      EBX, ESI
@@11:
        POP      EDI
        JMP      @@TailRecursion

{@@Compare:
        PUSH     EAX
        PUSH     EDX
        PUSH     ECX
        MOV      EAX, [EBP-4]
        MOV      EAX, [EAX + EDX*4]
        MOV      EDX, [EBP-4]
        MOV      EDX, [EDX + ECX*4]
        CALL     dword ptr [EBP-8]
        POP      ECX
        POP      EDX
        TEST     EAX, EAX
        POP      EAX
        RET}

@@CompareLeSwap:
        //CALL     @@Compare
        PUSH     EAX
        PUSH     EDX
        PUSH     ECX
        MOV      EAX, [EBP-4]
        MOV      EAX, [EAX + EDX*4]
        MOV      EDX, [EBP-4]
        MOV      EDX, [EDX + ECX*4]
        CALL     dword ptr [EBP-8]
        POP      ECX
        POP      EDX
        TEST     EAX, EAX
        POP      EAX

        JG       @@ret

@@Swap: PUSH     EAX
        PUSH     ESI
        MOV      ESI, [EBP-4]
        MOV      EAX, [ESI+EDX*4]
        XCHG     EAX, [ESI+ECX*4]
        MOV      [ESI+EDX*4], EAX
        POP      ESI
        //TEST     EAX, EAX
        POP      EAX
@@ret:
        RET

end;


function CompareIntegers( const Sender : Pointer; const e1, e2 : DWORD ) : Integer;
asm
        MOV      EDX, [EAX+EDX*4]
        SUB      EDX, [EAX+ECX*4]
        XCHG     EAX, EDX
end;

function CompareDwords( const Sender : Pointer; const e1, e2 : DWORD ) : Integer;
asm
        MOV      EDX, [EAX+EDX*4]
        SUB      EDX, [EAX+ECX*4]
        XCHG     EAX, EDX
        JNB      @@1
        SBB      EAX, EAX
@@1:
end;

function Compare2Dwords( e1, e2 : DWORD ) : Integer;
asm
    SUB  EAX, EDX
    JZ   @@exit
    MOV  EAX, 0
    JB   @@neg
    INC  EAX
    INC  EAX
@@neg:
    DEC  EAX
@@exit:
end;

procedure SwapIntegers( const Sender : Pointer; const e1, e2 : DWORD );
asm
        LEA      EDX, [EAX+EDX*4]
        LEA      ECX, [EAX+ECX*4]
        MOV      EAX, [EDX]
        XCHG     EAX, [ECX]
        MOV      [EDX], EAX
end;

function _NewStatusbar( AParent: PControl ): PControl;
const STAT_CLS_NAM: PKOLChar = STATUSCLASSNAME;
asm
        PUSH     0
        {$IFDEF  COMMANDACTIONS_OBJ}
        PUSH     OTHER_ACTIONS
        {$ELSE}
        PUSH     0
        {$ENDIF}
        {$IFDEF  USE_FLAGS}
        TEST     [EAX].TControl.fFlagsG3, (1 shl G3_SizeGrip)
        {$ELSE}
        CMP      [EAX].TControl.fSizeGrip, 0
        {$ENDIF}
        MOV      ECX, WS_CHILD or WS_CLIPSIBLINGS or WS_CLIPCHILDREN or 3 or WS_VISIBLE
        JZ       @@1
        INC      CH
        AND      CL, not 3
@@1:
        MOV      EDX, [STAT_CLS_NAM]
        CALL     _NewCommonControl
        PUSH     EBX
        XCHG     EBX, EAX
        PUSH     EDI
        LEA      EDI, [EBX].TControl.fBoundsRect
        XOR      EAX, EAX
        STOSD
        STOSD
        STOSD
        STOSD
        MOV      [EBX].TControl.fAlign, caBottom
        {$IFDEF  USE_FLAGS}
        OR       [EBX].TControl.fFlagsG4, 1 shl G4_NotUseAlign
        {$ELSE}
        INC      [EBX].TControl.fNotUseAlign
        {$ENDIF}
        POP      EDI
        MOV      EAX, EBX
        CALL     InitCommonControlSizeNotify
        XCHG     EAX, EBX
        POP      EBX
end;

procedure TControl.RemoveStatus;
asm
        MOV      ECX, [EAX].fStatusCtl
        JECXZ    @@exit
        PUSH     EBX
        MOV      EBX, EAX
        CALL     GetClientHeight
        PUSH     EAX
        XOR      EAX, EAX
        XCHG     [EBX].fStatusCtl, EAX
        CALL     TObj.RefDec
        POP      EAX
        CDQ
        MOV      [EBX].fClientBottom, DL
        XCHG     EDX, EAX
        XCHG     EAX, EBX
        POP      EBX
        CALL     SetClientHeight
@@exit:
end;

function TControl.StatusPanelCount: Integer;
asm
        MOV      ECX, [EAX].fStatusCtl
        JECXZ    @@exit
        PUSH     0
        PUSH     0
        PUSH     SB_GETPARTS
        PUSH     ECX
        CALL     Perform
@@exit:
end;

function TControl.GetStatusPanelX(Idx: Integer): Integer;
asm
        MOV      ECX, [EAX].fStatusCtl
        JECXZ    @@exit
        PUSH     EBX
        MOV      EBX, EDX
        ADD      ESP, -1024
        PUSH     ESP
        XOR      EDX, EDX
        DEC      DL
        PUSH     EDX
        MOV      DX, SB_GETPARTS
        PUSH     EDX
        PUSH     ECX
        CALL     Perform
        CMP      EAX, EBX
        MOV      ECX, [ESP+EBX*4]
        JG       @@1
        XOR      ECX, ECX
@@1:    ADD      ESP, 1024
        POP      EBX
@@exit:
        XCHG     EAX, ECX
end;

procedure TControl.SetStatusPanelX(Idx: Integer; const Value: Integer);
asm
        ADD      ESP, -1024
        MOV      EAX, [EAX].fStatusCtl
        TEST     EAX, EAX
        JZ       @@exit

        PUSH     ESP
        PUSH     EDX
        PUSH     SB_SETPARTS
        PUSH     EAX

        PUSH     EDX
        PUSH     ECX

        LEA      EDX, [ESP+24]
        PUSH     EDX
        PUSH     255
        PUSH     SB_GETPARTS
        PUSH     EAX
        CALL     Perform

        POP      ECX
        POP      EDX
        CMP      EAX, EDX
        JG       @@1
        ADD      ESP, 16
        JMP      @@exit

@@1:    MOV      [ESP+8], EAX
        MOV      [ESP+16+EDX*4], ECX
        CALL     Perform

@@exit: ADD      ESP, 1024
end;

destructor TImageList.Destroy;
asm
        PUSH     EAX
        XOR      EDX, EDX
        CALL     SetHandle
        POP      EAX
        MOV      EDX, [EAX].fNext
        MOV      ECX, [EAX].fPrev
        TEST     EDX, EDX
        JZ       @@nonext
        MOV      [EDX].fPrev, ECX
@@nonext:
        JECXZ    @@noprev
        MOV      [ECX].fNext, EDX
@@noprev:
        MOV      ECX, [EAX].fControl
        JECXZ    @@fin
        CMP      [ECX].TControl.fImageList, EAX
        JNZ      @@fin
        MOV      [ECX].TControl.fImageList, EDX
        {$IFDEF USE_AUTOFREE4CONTROLS}
        PUSH     EAX
        XCHG     EAX, ECX
        MOV      EDX, ECX
        CALL     TControl.RemoveFromAutoFree
        POP      EAX
        {$ENDIF}
@@fin:  CALL     TObj.Destroy
end;

function TImageList.GetHandle: THandle;
asm
        PUSH     EAX
        CALL     HandleNeeded
        POP      EAX
        MOV      EAX, [EAX].FHandle
end;

procedure TImageList.SetHandle(const Value: THandle);
asm
        PUSH     EBX
        XCHG     EBX, EAX
        MOV      ECX, [EBX].FHandle
        CMP      ECX, EDX
        JZ       @@exit
        JECXZ    @@set_handle
        CMP      [EBX].fShareImages, 0
        JNZ      @@set_handle
        PUSH     EDX
        PUSH     ECX
        CALL     ImageList_Destroy
        POP      EDX

@@set_handle:
        MOV      [EBX].FHandle, EDX
        TEST     EDX, EDX
        JZ       @@set_sz0
        LEA      EAX, [EBX].FImgHeight
        PUSH     EAX
        LEA      EAX, [EBX].FImgWidth
        PUSH     EAX
        PUSH     EDX
        CALL     ImageList_GetIconSize
        JMP      @@exit

@@set_sz0:
        MOV      [EBX].fImgWidth, EDX
        MOV      [EBX].fImgHeight, EDX

@@exit:
        POP      EBX
end;

function TControl.Perform(msgcode: DWORD; wParam, lParam: Integer): Integer; stdcall;
asm
        PUSH     [lParam]
        PUSH     [wParam]
        PUSH     [msgcode]
        MOV      EAX, [EBP+8]
        CALL     TControl.GetWindowHandle
        PUSH     EAX
        {$IFDEF UNICODE_CTRLS}
        CALL     Windows.SendMessageW
        {$ELSE}
        CALL     Windows.SendMessageA
        {$ENDIF}
end;

function TControl.Postmsg(msgcode: DWORD; wParam, lParam: Integer): Boolean; stdcall;
asm
        PUSH     [lParam]
        PUSH     [wParam]
        PUSH     [msgcode]
        MOV      EAX, [EBP+8]
        CALL     TControl.GetWindowHandle
        PUSH     EAX
        CALL     Windows.PostMessageA
end;

function TControl.GetChildCount: Integer;
asm
        MOV      EAX, [EAX].fChildren
        MOV      EAX, [EAX].TList.fCount
end;

procedure TControl.SetItemVal(Item: Integer; const Index: Integer; const Value: Integer);
asm
        PUSH     EAX
        PUSH     [Value]
        PUSH     EDX
        MOV      EDX, ECX
        SHR      EDX, 16
        JNZ      @@1
        MOV      EDX, ECX
        INC      EDX
@@1:
        MOV      EBP, EDX
        AND      EDX, 7FFFh
        PUSH     EDX
        PUSH     EAX
        CALL     Perform
        MOV      EAX, EBP
        ADD      AX, AX
        POP      EAX
        JNB      @@2
        CALL     Invalidate
@@2:
end;

destructor TOpenSaveDialog.Destroy;
asm     //cmd    //opd
        PUSH     EAX
        PUSH     0
        LEA      EDX, [EAX].FFilter
        PUSH     EDX
        LEA      EDX, [EAX].FInitialDir
        PUSH     EDX
        LEA      EDX, [EAX].FDefExtension
        PUSH     EDX
        LEA      EDX, [EAX].FFileName
        PUSH     EDX
        LEA      EAX, [EAX].FTitle
@@loo:
        {$IFDEF UNICODE_CTRLS}
        CALL     System.@WStrClr
        {$ELSE}
        CALL     System.@LStrClr
        {$ENDIF}
        POP      EAX
        TEST     EAX, EAX
        JNZ      @@loo
        POP      EAX
        CALL     TObj.Destroy
end;

destructor TOpenDirDialog.Destroy;
asm     //cmd    //opd
        PUSH     EAX
        PUSH     0
        LEA      EDX, [EAX].FTitle
        PUSH     EDX
        LEA      EDX, [EAX].FInitialPath
        PUSH     EDX
        LEA      EAX, [EAX].FStatusText
@@loo:
        {$IFDEF UNICODE_CTRLS}
        CALL     System.@WStrClr
        {$ELSE}
        CALL     System.@LStrClr
        {$ENDIF}
        POP      EAX
        TEST     EAX, EAX
        JNZ      @@loo
        POP      EAX
        CALL     TObj.Destroy
end;

{$IFNDEF NEW_OPEN_DIR_STYLE_EX}
function OpenDirCallBack( Wnd: HWnd; Msg: DWORD; lParam, lpData: LParam ): Integer;
         stdcall;
asm
        MOV      EAX, [Wnd]
        MOV      EDX, [lpData]

        MOV      [EDX].TOpenDirDialog.FDialogWnd, EAX

        MOV      ECX, [Msg]
        LOOP     @@chk_sel_chg
        // Msg = 1 -> BFFM_Initialized

        MOV      ECX, [EDX].TOpenDirDialog.FCenterProc
        JECXZ    @@1
        PUSH     EDX
        CALL     ECX
        POP      EDX
@@1:    MOV      ECX, [EDX].TOpenDirDialog.FInitialPath
        JECXZ    @@exit
        PUSH     ECX
        PUSH     1
        {$IFDEF UNICODE_CTRLS}
        PUSH     BFFM_SETSELECTIONW
        {$ELSE}
        PUSH     BFFM_SETSELECTION
        {$ENDIF}
        PUSH     [Wnd]
        CALL     SendMessage
        JMP      @@exit

@@chk_sel_chg:
        LOOP     @@exit
        // Msg = 2 -> BFFM_SelChanged

        MOV      ECX, [EDX].TOpenDirDialog.FDoSelChanged
        JECXZ    @@exit
        POP      EBP
        JMP      ECX

@@exit: XOR      EAX, EAX
end;
{$ENDIF}

procedure OpenDirDlgCenter( Wnd: HWnd );
asm
        PUSH     EBX
        MOV      EBX, EAX

        ADD      ESP, -16
        PUSH     ESP
        PUSH     EAX
        CALL     GetWindowRect
        POP      EDX          // EDX = Left
        POP      ECX          // ECX = Top
        POP      EAX          // EAX = Right
        SUB      EAX, EDX     // EAX = W
        POP      EDX          // EDX = Bottom
        SUB      EDX, ECX     // EDX = H
        XOR      ECX, ECX
        INC      ECX
        PUSH     ECX  // prepare True
        PUSH     EDX  // prepare H
        PUSH     EAX  // prepare W

        INC      ECX
@@1:
        PUSH     ECX

        DEC      ECX
        PUSH     ECX
        CALL     GetSystemMetrics

        POP      ECX
        SUB      EAX, [ESP+4]
        SAR      EAX, 1
        PUSH     EAX

        LOOP     @@1

        PUSH     EBX
        CALL     MoveWindow
        POP      EBX
end;

procedure TOpenDirDialog.SetCenterOnScreen(const Value: Boolean);
asm
        MOV      [EAX].FCenterOnScreen, DL
        MOVZX    ECX, DL
        JECXZ    @@1
        MOV      ECX, Offset[OpenDirDlgCenter]
@@1:    MOV      [EAX].FCenterProc, ECX
end;

function TControl.TBAddButtons(const Buttons: array of PKOLChar;
         const BtnImgIdxArray: array of Integer): Integer;
asm
        PUSH     dword ptr [EBP+8]
        PUSH     dword ptr [EBP+12]
        PUSH     ECX
        PUSH     EDX
        PUSH     -1
        PUSH     EAX
        CALL     TBAddInsButtons
end;

function TControl.TBGetBtnStt(BtnID: Integer; const Index: Integer): Boolean;
asm
        PUSH     0
        PUSH     ECX
        PUSH     EAX
        CALL     GetTBBtnGoodID
        POP      EDX
        POP      ECX
        PUSH     EAX
        ADD      ECX, 8
        PUSH     ECX
        PUSH     EDX
        CALL     Perform
        TEST     EAX, EAX
        SETNZ    AL
end;

function TControl.TBIndex2Item(Idx: Integer): Integer;
const                                                 //
  _sizeof_TTBButton = sizeof( TTBButton );            //
asm
        ADD      ESP, -_sizeof_TTBButton              //
        PUSH     ESP
        PUSH     EDX
        PUSH     TB_GETBUTTON
        PUSH     EAX
        CALL     Perform
        TEST     EAX, EAX
        MOV      EAX, [ESP].TTBButton.idCommand
        JNZ      @@1
        OR       EAX, -1
@@1:    ADD      ESP, _sizeof_TTBButton               //
end;

// TODO: testcase
//{$IFDEF ASM_UNICODE}
procedure TControl.TBSetTooltips(BtnID1st: Integer;
  const Tooltips: array of PKOLChar);
asm
        PUSH     EBX
        PUSH     ESI
        MOV      ESI, ECX
        MOV      EBX, EAX
        PUSHAD
        MOV      ECX, [EBX].DF.fTBttCmd
        INC      ECX
        LOOP     @@1
        CALL     NewList
        MOV      [EBX].DF.fTBttCmd, EAX
        {$IFDEF USE_AUTOFREE4CONTROLS}
        XCHG     EDX, EAX
        MOV      EAX, EBX
        CALL     TControl.Add2AutoFree
        {$ENDIF}
        {$IFDEF UNICODE_CTRLS}
        CALL     NewWStrList
        {$ELSE}
        CALL     NewStrList
        {$ENDIF}
        MOV      [EBX].DF.fTBttTxt, EAX
        {$IFDEF USE_AUTOFREE4CONTROLS}
        XCHG     EDX, EAX
        MOV      EAX, EBX
        CALL     TControl.Add2AutoFree
        {$ENDIF}
@@1:    POPAD
        MOV      ECX, [EBP+8]
        INC      ECX
        JZ       @@exit
@@loop:
        PUSH     ECX
        PUSH     EDX
        PUSH     0
        LODSD
        MOV      EDX, EAX
        MOV      EAX, ESP
        {$IFDEF UNICODE_CTRLS}
        CALL     System.@WStrFromPWChar
        {$ELSE}
        {$IFDEF _D2009orHigher}
        XOR      ECX, ECX // TODO: safe?
        {$ENDIF}
        CALL     System.@LStrFromPChar
        {$ENDIF}

        MOV      EDX, [ESP+4]
        MOV      EAX, [EBX].DF.fTBttCmd
        CALL     TList.IndexOf
        TEST     EAX, EAX
        JGE      @@2

        MOV      EDX, [ESP+4]
        MOV      EAX, [EBX].DF.fTBttCmd
        CALL     TList.Add
        POP      EDX
        PUSH     EDX
        MOV      EAX, [EBX].DF.fTBttTxt
        {$IFDEF UNICODE_CTRLS}
        CALL     TWStrList.Add
        {$ELSE}
        CALL     TStrList.Add
        {$ENDIF}
        JMP      @@3

@@2:
        MOV      EDX, EAX
        POP      ECX
        PUSH     ECX
        MOV      EAX, [EBX].DF.fTBttTxt
        {$IFDEF UNICODE_CTRLS}
        CALL     TWStrList.Put
        {$ELSE}
        CALL     TStrList.Put
        {$ENDIF}
@@3:
        {$IFDEF UNICODE_CTRLS}
        CALL     RemoveWStr
        {$ELSE}
        CALL     RemoveStr
        {$ENDIF}

        POP      EDX
        POP      ECX
        INC      EDX
        LOOP     @@loop
@@exit:
        POP      ESI
        POP      EBX
end;
//{$ENDIF}

function TControl.TBButtonAtPos(X, Y: Integer): Integer;
asm
        PUSH     EAX
        CALL     TBBtnIdxAtPos
        TEST     EAX, EAX
        MOV      EDX, EAX
        POP      EAX
        JGE      TBIndex2Item
        MOV      EAX, EDX
end;

function TControl.TBBtnIdxAtPos(X, Y: Integer): Integer;
asm
        PUSH     EBX
        PUSH     ECX
        PUSH     EDX
        MOV      EBX, EAX
        CALL     GetItemsCount
        MOV      ECX, EAX
        JECXZ    @@fin
@@1:    PUSH     ECX
        ADD      ESP, -16
        PUSH     ESP
        DEC      ECX
        PUSH     ECX
        PUSH     TB_GETITEMRECT
        PUSH     EBX
        CALL     Perform
        MOV      EDX, ESP
        LEA      EAX, [ESP+20]
        CALL     PointInRect
        ADD      ESP, 16
        POP      ECX
        TEST     AL, AL
        {$IFDEF USE_CMOV}
        CMOVNZ   EAX, ECX
        {$ELSE}
        JZ       @@2
        MOV      EAX, ECX
        JMP      @@fin
@@2:    {$ENDIF}
        JNZ      @@fin

        LOOP     @@1
@@fin:  DEC      EAX
        POP      EDX
        POP      EDX
        POP      EBX
end;

procedure TControl.TBSetButtonText(BtnID: Integer; const Value: KOLString);
asm
        PUSH     0
        PUSH     ECX
        PUSH     EAX
        CALL     GetTBBtnGoodID
        POP      EDX

        ADD      ESP, -16
        PUSH     TBIF_TEXT
        PUSH     32 //Sizeof( TTBButtonInfo )
        PUSH     ESP
        PUSH     EAX
        PUSH     TB_SETBUTTONINFO
        PUSH     EDX
        CALL     Perform
        ADD      ESP, 32 //sizeof( TTBButtonInfo )
end;

function TControl.TBGetBtnWidth(BtnID: Integer): Integer;
asm
        ADD      ESP, -16
        MOV      ECX, ESP
        CALL     TBGetButtonRect
        POP      EDX
        POP      ECX
        POP      EAX
        SUB      EAX, EDX
        POP      EDX
end;

procedure TControl.TBSetBtnWidth(BtnID: Integer; const Value: Integer);
asm
        PUSH     EBX
        MOV      EBX, ECX

        PUSH     EAX
        CALL     GetTBBtnGoodID
        POP      EDX

        ADD      ESP, -24
        PUSH     TBIF_SIZE or TBIF_STYLE
        PUSH     32
        MOV      ECX, ESP

        PUSH     ECX
        PUSH     EAX
        PUSH     TB_SETBUTTONINFO
        PUSH     EDX

        PUSH     ECX
        PUSH     EAX
        PUSH     TB_GETBUTTONINFO
        PUSH     EDX
        CALL     Perform

        MOV      [ESP+16+18], BX
        AND      byte ptr [ESP+16].TTBButtonInfo.fsStyle, not TBSTYLE_AUTOSIZE
        CALL     Perform
        ADD      ESP, 32
        POP      EBX
end;

procedure TControl.AddDirList(const Filemask: KOLString; Attrs: DWORD);
asm
        CALL     EDX2PChar
        PUSH     EDX
        PUSH     ECX
        {$IFDEF  COMMANDACTIONS_OBJ}
        MOV      ECX, [EAX].fCommandActions
        MOVZX    ECX, [ECX].TCommandActionsObj.aDir
        {$ELSE}
        MOVZX    ECX, [EAX].fCommandActions.aDir
        {$ENDIF}
        JECXZ    @@exit
        PUSH     ECX
        PUSH     EAX
        CALL     Perform
        RET
@@exit:
        POP      ECX
        POP      ECX
end;

{$IFDEF noASM_VERSION}
function WndProcShowModal( Self_: PControl; var Msg: TMsg; var Rslt: Integer ): Boolean;
asm
        CMP      word ptr [EDX].TMsg.message, WM_CLOSE
        JNZ      @@ret_false

        XCHG     EDX, EAX
        XOR      EAX, EAX
        CMP      [EDX].TControl.fModalResult, EAX
        JNZ      @@1
        OR       [EDX].TControl.fModalResult, -1
@@1:
        MOV      [ECX], EAX
        INC      EAX
        RET
@@ret_false:
        XOR      EAX, EAX

end;
{$ENDIF}

function TimerProc( Wnd : HWnd; Msg : Integer; T : PTimer; CurrentTime : DWord ): Integer;
          stdcall;
asm     //cmd    //opd
        {$IFDEF STOPTIMER_AFTER_APPLETTERMINATED}
        CMP      [AppletTerminated], 0
        JNZ      @@exit
        {$ENDIF}
        MOV      EDX, T
        MOV      ECX, [EDX].TTimer.fOnTimer.TMethod.Code
        JECXZ    @@exit
        MOV      EAX, [EDX].TTimer.fOnTimer.TMethod.Data
        CALL     ECX
@@exit: XOR      EAX, EAX
end;

destructor TTimer.Destroy;
asm
        PUSH     EAX
        XOR      EDX, EDX
        CALL     TTimer.SetEnabled
        POP      EAX
        CALL     TObj.Destroy
        DEC      [TimerCount]
        JNZ      @@exit
        XOR      EAX, EAX
        XCHG     EAX, [TimerOwnerWnd]
        CALL     TObj.RefDec
@@exit:
end;

procedure TTimer.SetEnabled(const Value: Boolean);
asm
        PUSH     EBX
        XCHG     EBX, EAX

        CMP      [EBX].fEnabled, DL
        JZ       @@exit

        {$IFDEF TIMER_APPLETWND}

        MOV      ECX, [Applet]
        JECXZ    @@exit

        MOV      [EBX].fEnabled, DL
        TEST     DL, DL
        JZ       @@disable

        {$ELSE}

        MOV      [EBX].fEnabled, DL
        TEST     DL, DL
        JZ       @@disable

        MOV      ECX, [TimerOwnerWnd]
        INC      ECX
        LOOP     @@owner_ready

        INC      ECX
        MOV      EDX, offset[EmptyString]
        XOR      EAX, EAX
        PUSH     EAX
        CALL     _NewWindowed
        MOV      [TimerOwnerWnd], EAX
        MOV      [EAX].TControl.fStyle, 0
        {$IFDEF  USE_FLAGS}
        OR       [EAX].TControl.fFlagsG3, 1 shl G3_IsControl
        {$ELSE}
        INC      [EAX].TControl.fIsControl
        {$ENDIF}
        XCHG     ECX, EAX

        {$ENDIF}

@@owner_ready:

        PUSH     offset[TimerProc]
        PUSH     [EBX].fInterval
        PUSH     EBX
        XCHG     EAX, ECX
        CALL     TControl.GetWindowHandle
        PUSH     EAX
        CALL     SetTimer
        MOV      [EBX].fHandle, EAX

        JMP      @@exit

@@disable:
        XOR      ECX, ECX
        XCHG     ECX, [EBX].TTimer.fHandle
        JECXZ    @@exit

        PUSH     ECX
        {$IFDEF TIMER_APPLETWND}
        MOV      EAX, [Applet]
        {$ELSE}
        MOV      EAX, [TimerOwnerWnd]
        {$ENDIF}
        PUSH     [EAX].TControl.fHandle
        CALL     KillTimer

@@exit:
        POP      EBX
end;

function PrepareBitmapHeader( W, H, BitsPerPixel: Integer ): PBitmapInfo;
const szIH = sizeof(TBitmapInfoHeader);
      szHd = szIH + 256 * Sizeof(TRGBQuad);
asm
        PUSH     EDI

          PUSH     ECX  // BitsPerPixel
        PUSH     EDX    // H
        PUSH     EAX    // W

        MOV      EAX, szHd
        CALL     AllocMem

        MOV      EDI, EAX
        XCHG     ECX, EAX

        XOR      EAX, EAX
        MOV      AL, szIH
        STOSD           // biSize = Sizeof( TBitmapInfoHeader )
        POP      EAX    // ^ W
        STOSD           // -> biWidth
        POP      EAX    // ^ H
        STOSD           // -> biHeight
        XOR      EAX, EAX
        INC      EAX
        STOSW           // 1 -> biPlanes
          POP      EAX  // ^ BitsPerPixel
        STOSW           // -> biBitCount

        XCHG     EAX, ECX // EAX = Result
        POP      EDI
end;

function Bits2PixelFormat( BitsPerPixel: Integer ): TPixelFormat;
asm
        PUSH     ESI
        MOV      ESI, offset[ BitsPerPixel_By_PixelFormat + 1 ]
        XOR      ECX, ECX
        XCHG     EDX, EAX
@@loo:  INC      ECX
        LODSB
        CMP      AL, DL
        JZ       @@exit
        TEST     AL, AL
        JNZ      @@loo
@@exit: XCHG     EAX, ECX
        POP      ESI
end;

function _NewBitmap( W, H: Integer ): PBitmap;
begin
  New( Result, Create );
  Result.fDetachCanvas := DummyDetachCanvas;
  Result.fWidth := W;
  Result.fHeight := H;
end;
function NewBitmap( W, H: Integer ): PBitmap;
asm
        PUSH     EAX
        PUSH     EDX
        CALL     _NewBitmap
        POP      EDX
        POP      ECX
        PUSH     EAX
        INC      [EAX].TBitmap.fHandleType
        JECXZ    @@exit
        TEST     EDX, EDX
        JZ       @@exit
        PUSH     EBX
        PUSH     EAX
        PUSH     EDX
        PUSH     ECX
        PUSH     0
        CALL     GetDC
        PUSH     EAX
        XCHG     EBX, EAX
        CALL     CreateCompatibleBitmap
        POP      EDX
        MOV      [EDX].TBitmap.fHandle, EAX
        PUSH     EBX
        PUSH     0
        CALL     ReleaseDC
        POP      EBX
@@exit: POP      EAX
end;

procedure PreparePF16bit( DIBHeader: PBitmapInfo );
const szBIH = sizeof(TBitmapInfoHeader);
asm
        MOV      byte ptr [EAX].TBitmapInfoHeader.biCompression, BI_BITFIELDS
        ADD      EAX, szBIH
        XCHG     EDX, EAX
        MOV      EAX, offset[InitColors]
        XOR      ECX, ECX
        MOV      CL, 19*4
        CALL     System.Move
end;

function NewDIBBitmap( W, H: Integer; PixelFormat: TPixelFormat ): PBitmap;
asm
        PUSH     EBX

        PUSH     ECX
        PUSH     EDX
        PUSH     EAX
        CALL     _NewBitmap
        XCHG     EBX, EAX
        POP      EAX //W
        POP      EDX //H
        POP      ECX //PixelFormat

        TEST     EAX, EAX
        JZ       @@exit
        TEST     EDX, EDX
        JZ       @@exit

        PUSH     EAX
        MOVZX    EAX, CL
        JMP      @@loadBitsPixel
@@loadDefault:
        MOVZX    EAX, [DefaultPixelFormat]
@@loadBitsPixel:
        MOVZX    ECX, byte ptr [ BitsPerPixel_By_PixelFormat + EAX ]
        JECXZ    @@loadDefault
        MOV      [EBX].TBitmap.fNewPixelFormat, AL
        {$IFDEF PARANOIA} DB $3C, pf16bit {$ELSE} CMP AL, pf16bit {$ENDIF}
        POP      EAX

        PUSHFD
        CALL     PrepareBitmapHeader
        MOV      [EBX].TBitmap.fDIBHeader, EAX
        POPFD
        JNZ      @@2

        CALL     PreparePF16bit

@@2:
        MOV      EAX, EBX
        CALL     TBitmap.GetScanLineSize
        MOV      EDX, [EBX].TBitmap.fHeight
        MUL      EDX
        MOV      [EBX].TBitmap.fDIBSize, EAX
        ADD      EAX, 16
        PUSH     EAX
        PUSH     GMEM_FIXED or GMEM_ZEROINIT
        CALL     GlobalAlloc
        MOV      [EBX].TBitmap.fDIBBits, EAX
@@exit:
        XCHG     EAX, EBX
        POP      EBX
end;

procedure TBitmap.ClearData;
asm
        PUSH     EBX
        MOV      EBX, EAX
        CALL     [EBX].fDetachCanvas
        XOR      ECX, ECX
        XCHG     ECX, [EBX].fHandle
        JECXZ    @@1
        PUSH     ECX
        CALL     DeleteObject
        XOR      ECX, ECX
        MOV      [EBX].fDIBBits, ECX
@@1:    XCHG     ECX, [EBX].fDIBBits
        JECXZ    @@2
        CMP      [EBX].fDIBAutoFree, 0
        JNZ      @@2
        PUSH     ECX
        CALL     GlobalFree
@@2:    XOR      ECX, ECX
        XCHG     ECX, [EBX].fDIBHeader
        JECXZ    @@3
        XCHG     EAX, ECX
        CALL     System.@FreeMem
@@3:    XOR      EAX, EAX
        MOV      [EBX].fScanLineSize, EAX
        MOV      [EBX].fGetDIBPixels, EAX
        MOV      [EBX].fSetDIBPixels, EAX
        XCHG     EAX, EBX
        POP      EBX
        CALL     ClearTransImage
end;

procedure TBitmap.Clear;
asm
        PUSH     EAX
        CALL     RemoveCanvas
        POP      EAX
        PUSH     EAX
        CALL     ClearData
        POP      EAX
        XOR      EDX, EDX
        MOV      [EAX].fWidth, EDX
        MOV      [EAX].fHeight, EDX
        MOV      [EAX].fDIBAutoFree, DL
end;

destructor TBitmap.Destroy;
asm
        PUSH     EAX
        CALL     Clear
        POP      EAX
        CALL     TObj.Destroy
end;

procedure TBitmap.Draw(DC: HDC; X, Y: Integer);
const szBitmap = sizeof( tagBitmap );
asm                       // [EBP+8] = Y
        PUSH     EDX      // [EBP-4] = DC
        PUSH     ECX      // [EBP-8] = X
        PUSH     EBX
        PUSH     ESI
@@try_again:
        MOV      EBX, EAX
        CALL     GetEmpty // GetEmpty must be assembler version !
        JZ       @@exit

        MOV      ECX, [EBX].fHandle
        JECXZ    @@2
        //MOV      EAX, EBX
        //CALL     [EBX].fDetachCanvas // detached in StartDC
        ADD      ESP, -szBitmap
        PUSH     ESP
        PUSH     szBitmap
        PUSH     [EBX].fHandle
        CALL     GetObject
        TEST     EAX, EAX
        MOV      ESI, [ESP].tagBitmap.bmHeight
        {$IFDEF USE_CMOV}
        CMOVZ    ESI, [EBX].fHeight
        {$ELSE}
        JNZ      @@1
        MOV      ESI, [EBX].fHeight
@@1:    {$ENDIF}

        ADD      ESP, szBitmap
        CALL     StartDC

        PUSH     SRCCOPY
        PUSH     0
        PUSH     0
        PUSH     EAX
        CALL     @@prepare
        CALL     BitBlt
        CALL     FinishDC
        JMP      @@exit

@@prepare:
        XCHG     ESI, [ESP]
        PUSH     [EBX].fWidth
        PUSH     Y
        PUSH     dword ptr [EBP-8]
        PUSH     dword ptr [EBP-4]
        JMP      ESI

@@2:
        MOV      ECX, [EBX].fDIBHeader
        JECXZ    @@exit

        MOV      ESI, [ECX].TBitmapInfoHeader.biHeight
        TEST     ESI, ESI
        JGE      @@20
        NEG      ESI
@@20:
        PUSH     SRCCOPY
        PUSH     DIB_RGB_COLORS
        PUSH     ECX
        PUSH     [EBX].fDIBBits
        PUSH     ESI
        PUSH     [EBX].fWidth
        PUSH     0
        PUSH     0
        CALL     @@prepare
        CALL     StretchDIBits
        TEST     EAX, EAX
        JNZ      @@exit
        MOV      EAX, EBX
        CALL     GetHandle
        TEST     EAX, EAX
        XCHG     EAX, EBX
        JNZ      @@try_again
@@exit:
        POP      ESI
        POP      EBX
        MOV      ESP, EBP
end;

procedure TBitmap.StretchDraw(DC: HDC; const Rect: TRect);
asm
        PUSH     EBX
        PUSH     EDI
        PUSH     EBP
        MOV      EBP, ESP
        PUSH     EDX
        PUSH     ECX
        MOV      EBX, EAX
        CALL     GetEmpty
        JZ       @@exit

        MOV      ECX, [EBX].fHandle
        JECXZ    @@2

@@0:
        CALL     StartDC
        PUSH     SRCCOPY
        PUSH     [EBX].fHeight
        PUSH     [EBX].fWidth
        PUSH     0
        PUSH     0
        PUSH     EAX

        CALL     @@prepare
        CALL     StretchBlt
        CALL     FinishDC
        JMP      @@exit

@@prepare:
        POP      EDI
        MOV      EAX, [EBP-8]
        MOV      EDX, [EAX].TRect.Bottom
        MOV      ECX, [EAX].TRect.Top
        SUB      EDX, ECX
        PUSH     EDX
        MOV      EDX, [EAX].TRect.Right
        MOV      EAX, [EAX].TRect.Left
        SUB      EDX, EAX
        PUSH     EDX
        PUSH     ECX
        PUSH     EAX
        PUSH     dword ptr [EBP-4]
        JMP      EDI


@@2:    MOV      ECX, [EBX].fDIBHeader
        JECXZ    @@exit

        PUSH     SRCCOPY
        PUSH     DIB_RGB_COLORS
        PUSH     ECX
        PUSH     [EBX].fDIBBits
        PUSH     [EBX].fHeight
        PUSH     [EBX].fWidth
        PUSH     0
        PUSH     0
        CALL     @@prepare
        CALL     StretchDIBits
        TEST     EAX, EAX
        JG       @@exit

        MOV      EAX, EBX
        CALL     GetHandle
        MOV      ECX, [EBX].fHandle
        JECXZ    @@exit
        JMP      @@0

@@exit: MOV      ESP, EBP
        POP      EBP
        POP      EDI
        POP      EBX
end;

procedure TBitmap.DrawTransparent(DC: HDC; X, Y: Integer; TranspColor: TColor);
asm
        PUSH     ECX
        MOV      ECX, TranspColor
        INC      ECX
        MOV      ECX, [Y]
        JNZ      @@2
        XCHG     ECX, [ESP]
        CALL     Draw
        JMP      @@exit
@@2:
        ADD      ECX, [EAX].fHeight
        PUSH     ECX
        MOV      ECX, [EBP-4]
        ADD      ECX, [EAX].fWidth
        PUSH     ECX
        PUSH     [Y]
        PUSH     dword ptr [EBP-4]
        MOV      ECX, ESP
        PUSH     [TranspColor]
        CALL     StretchDrawTransparent
@@exit:
        MOV      ESP, EBP
end;

procedure TBitmap.StretchDrawTransparent(DC: HDC; const Rect: TRect; TranspColor: TColor);
asm
        PUSH     EBX
        XCHG     EBX, EAX
        MOV      EAX, [TranspColor]
        INC      EAX
        MOV      EAX, EBX
        JNZ      @@2
        CALL     StretchDraw
        JMP      @@exit
@@2:
        PUSH     EDX
        PUSH     ECX
        CALL     GetHandle
        TEST     EAX, EAX
        JZ       @@exit2

        MOV      EAX, [TranspColor]
        CALL     Color2RGB
        MOV      ECX, [EBX].fTransMaskBmp
        JECXZ    @@makemask0
        CMP      EAX, [EBX].fTransColor
        JE       @@3
@@makemask0:
        MOV      [EBX].fTransColor, EAX
        INC      ECX
        LOOP     @@20
        XOR      EAX, EAX // pass height = 0
        // absolutely no matter what to pass as width
        CALL     NewBitmap
        MOV      [EBX].fTransMaskBmp, EAX
@@20:
        MOV      EAX, [EBX].fTransMaskBmp
        PUSH     EAX
        MOV      EDX, EBX
        CALL     Assign
        POP      EAX
        MOV      EDX, [EBX].fTransColor
        CALL     Convert2Mask
@@3:
        MOV      EAX, [EBX].fTransMaskBmp
        CALL     GetHandle
        POP      ECX
        POP      EDX
        PUSH     EAX
        XCHG     EAX, EBX
        CALL     StretchDrawMasked
        JMP      @@exit
@@exit2:
        POP      ECX
        POP      EDX
@@exit:
        POP      EBX
end;

procedure TBitmap.StretchDrawMasked(DC: HDC; const Rect: TRect; Mask: HBitmap);
asm
        PUSH     EDX                    // [EBP-4] = DC
        PUSH     ECX                    // [EBP-8] = Rect
        PUSH     EBX                    // save EBX
        MOV      EBX, EAX               // EBX = @ Self
        PUSH     ESI                    // save ESI
  {$IFDEF FIX_TRANSPBMPPALETTE}
  CALL GetPixelFormat
  CMP  AL, pf4bit
  JZ   @@draw_fixed
  CMP  AL, pf8bit
  JNZ  @@draw_normal
  @@draw_fixed:
    XOR  EAX, EAX
    XOR  EDX, EDX
    CALL NewBitmap
    MOV  ESI, EAX
    MOV  EDX, EBX
    CALL Assign
    MOV  EAX, ESI
    XOR  EDX, EDX
    MOV  DL, pf32bit
    CALL SetPixelFormat
    MOV  EAX, ESI
    MOV  EDX, [EBP-4]
    MOV  ECX, [EBP-8]
    PUSH [Mask]
    CALL StretchDrawMasked
    XCHG EAX, ESI
    CALL TObj.RefDec
    JMP  @@exit
  @@draw_normal:
        MOV      EAX, EBX
  {$ENDIF FIX_TRANSPBMPPALETTE}
        CALL     GetHandle
        TEST     EAX, EAX
        JZ       @@to_exit

        PUSH     0
        CALL     CreateCompatibleDC
        PUSH     EAX                    // [EBP-20] = MaskDC

        PUSH     [Mask]
        PUSH     EAX
        CALL     SelectObject
        PUSH     EAX                    // [EBP-24] = Save4Mask

        CALL     StartDC                // [EBP-28] = DCfrom; [EBP-32] = Save4From

        PUSH     [EBX].fHeight
        PUSH     [EBX].fWidth
        PUSH     EAX
        CALL     CreateCompatibleBitmap
        PUSH     EAX                    // [EBP-36] = MemBmp

        PUSH     0
        CALL     CreateCompatibleDC
        PUSH     EAX                    // [EBP-40] = MemDC

        PUSH     dword ptr [EBP-36] //MemBmp
        PUSH     EAX
        CALL     SelectObject
        PUSH     EAX                    // [EBP-44] = Save4Mem

        PUSH     SRCCOPY
        MOV      EAX, [EBP-20] //MaskDC
        CALL     @@stretch1

        PUSH     SRCERASE
        MOV      EAX, [EBP-28] //DCfrom
        CALL     @@stretch1

        PUSH     0
        PUSH     dword ptr [EBP-4] //DC
        CALL     SetTextColor
        PUSH     EAX                    // [EBP-48] = crText

        PUSH     $FFFFFF
        PUSH     dword ptr [EBP-4] //DC
        CALL     Windows.SetBkColor
        PUSH     EAX                    // [EBP-52] = crBack

        PUSH     SRCAND
        MOV      EAX, [EBP-20] //MaskDC
        CALL     @@stretch2

        PUSH     SRCINVERT
        MOV      EAX, [EBP-40] //MemDC
        CALL     @@stretch2

        PUSH     dword ptr [EBP-4] //DC
        CALL     Windows.SetBkColor

        PUSH     dword ptr [EBP-4] //DC
        CALL     SetTextColor

        MOV      ESI, offset[FinishDC]
        CALL     ESI
        CALL     DeleteObject   // DeleteObject( MemBmp )
        CALL     ESI
        CALL     ESI
@@to_exit:
        STC
        JC       @@exit

@@stretch1:
        POP      ESI
        PUSH     [EBX].fHeight
        PUSH     [EBX].fWidth
        XOR      EDX, EDX
        PUSH     EDX
        PUSH     EDX
        PUSH     EAX
        PUSH     [EBX].fHeight
        PUSH     [EBX].fWidth
        PUSH     EDX
        PUSH     EDX
        PUSH     dword ptr [EBP-40] //MemDC
        JMP      @@stretch3

@@stretch2:
        POP      ESI
        PUSH     [EBX].fHeight
        PUSH     [EBX].fWidth
        PUSH     0
        PUSH     0
        PUSH     EAX
        MOV      EAX, [EBP-8] //Rect
        MOV      EDX, [EAX].TRect.Bottom
        MOV      ECX, [EAX].TRect.Top
        SUB      EDX, ECX
        PUSH     EDX
        MOV      EDX, [EAX].TRect.Right
        MOV      EAX, [EAX].TRect.Left
        SUB      EDX, EAX
        PUSH     EDX
        PUSH     ECX
        PUSH     EAX
        PUSH     dword ptr [EBP-4] //DC
@@stretch3:
        CALL     StretchBlt
        JMP      ESI

@@exit:
        POP      ESI
        POP      EBX
        MOV      ESP, EBP
end;

procedure DetachBitmapFromCanvas( Sender: PBitmap );
asm
        XOR      ECX, ECX
        XCHG     ECX, [EAX].TBitmap.fCanvasAttached
        JECXZ    @@exit
        PUSH     ECX
        MOV      EAX, [EAX].TBitmap.fCanvas
        PUSH     [EAX].TCanvas.fHandle
        CALL     SelectObject
@@exit:
end;

function TBitmap.GetCanvas: PCanvas;
asm
        PUSH     EBX
        MOV      EBX, EAX
        CALL     GetEmpty
        JZ       @@exit
        MOV      EAX, EBX
        CALL     GetHandle
        TEST     EAX, EAX
        JZ       @@exit
        MOV      ECX, [EBX].fCanvas
        INC      ECX
        LOOP     @@ret_Canvas

        MOV      [EBX].fApplyBkColor2Canvas, offset[ApplyBitmapBkColor2Canvas]
        //CALL     CreateCompatibleDC
        XOR      EAX, EAX
        //PUSH     EAX
        CALL     NewCanvas
        MOV      [EBX].fCanvas, EAX
        //MOV      [EAX].TCanvas.fIsAlienDC, 0
        MOV      [EAX].TCanvas.fOnChangeCanvas.TMethod.Code, offset[CanvasChanged]
        MOV      [EAX].TCanvas.fOnChangeCanvas.TMethod.Data, EBX
        CALL     TCanvas.GetBrush
        XOR      EDX, EDX
        MOV      ECX, [EBX].fBkColor
        JECXZ    @@ret_Canvas
        CALL     TGraphicTool.SetInt

@@ret_Canvas:
        MOV      EAX, [EBX].fCanvas
        MOV      ECX, [EAX].TCanvas.fHandle
        INC      ECX
        LOOP     @@attach_Canvas
        PUSH     EAX
        MOV      [EBX].fCanvasAttached, ECX
        PUSH     ECX
        CALL     CreateCompatibleDC
        XCHG     EDX, EAX
        POP      EAX
        CALL     TCanvas.SetHandle

@@attach_Canvas:
        MOV      ECX, [EBX].fCanvasAttached
        INC      ECX
        LOOP     @@2
        PUSH     [EBX].fHandle
        MOV      EAX, [EBX].fCanvas
        CALL     TCanvas.GetHandle
        PUSH     EAX
        CALL     SelectObject
        MOV      [EBX].fCanvasAttached, EAX

@@2:    MOV      [EBX].fDetachCanvas, offset[DetachBitmapFromCanvas]
        MOV      EAX, [EBX].fCanvas
@@exit: POP      EBX
end;

function TBitmap.GetEmpty: Boolean;
asm
        PUSH     ECX
        MOV      ECX, [EAX].fWidth
        JECXZ    @@1
        MOV      ECX, [EAX].fHeight
@@1:    TEST     ECX, ECX
        POP      ECX
        SETZ     AL
end;

procedure TBitmap.LoadFromFile(const Filename: KOLString);
asm
        PUSH     EAX
        XCHG     EAX, EDX
        CALL     NewReadFileStream
        XCHG     EDX, EAX
        POP      EAX
        PUSH     EDX
        CALL     LoadFromStream
        POP      EAX
        CALL     TObj.RefDec
end;

function TBitmap.ReleaseHandle: HBitmap;
asm
        PUSH     EBX
        MOV      EBX, EAX
        XOR      EDX, EDX
        CALL     SetHandleType
        MOV      EAX, EBX
        CALL     GetHandle
        TEST     EAX, EAX
        JZ       @@exit

        CMP      [EBX].fDIBAutoFree, 0
        JZ       @@1
        MOV      EAX, [EBX].fDIBSize
        PUSH     EAX
        PUSH     EAX
        PUSH     GMEM_FIXED {or GMEM_ZEROINIT}
        CALL     GlobalAlloc
        MOV      EDX, EAX
        XCHG     EAX, [EBX].fDIBBits
        POP      ECX
        CALL     System.Move
@@1:
        XOR      EAX, EAX
        MOV      [EBX].fDIBAutoFree, AL
        XCHG     EAX, [EBX].fHandle

@@exit: POP      EBX
end;

procedure TBitmap.SaveToFile(const Filename: KOLString);
asm
        PUSH     EAX
        PUSH     EDX
        CALL     GetEmpty
        POP      EAX
        JZ       @@exit
        CALL     NewWriteFileStream
        XCHG     EDX, EAX
        POP      EAX
        PUSH     EDX
        CALL     SaveToStream
        POP      EAX
        CALL     TObj.RefDec
        PUSH     EAX
@@exit: POP      EAX
end;

procedure TBitmap.SetHandle(const Value: HBitmap);
const szB = sizeof( tagBitmap );
      szDIB = sizeof( TDIBSection );
      szBIH = sizeof( TBitmapInfoHeader ); // = 40
asm
        PUSH     EBX
        MOV      EBX, EAX
        PUSH     EDX
        CALL     Clear
        POP      ECX
        TEST     ECX, ECX
        JZ       @@exit
        PUSH     ECX
        ADD      ESP, -szDIB

        CALL     WinVer
        CMP      AL, wvNT
        JB       @@ddb

        PUSH     ESP
        PUSH     szDIB
        PUSH     ECX
        CALL     GetObject
        CMP      EAX, szDIB
        JNZ      @@ddb

        MOV      [EBX].fHandleType, 0
        MOV      EAX, [ESP].TDIBSection.dsBm.bmWidth
        MOV      [EBX].fWidth, EAX
        MOV      EDX, [ESP].TDIBSection.dsBm.bmHeight
        MOV      [EBX].fHeight, EDX
        MOVZX    ECX, [ESP].TDIBSection.dsBm.bmBitsPixel
        CALL     PrepareBitmapHeader
        MOV      [EBX].fDIBHeader, EAX
        LEA      EDX, [EAX].TBitmapInfo.bmiColors
        LEA      EAX, [ESP].TDIBSection.dsBitfields
        XOR      ECX, ECX
        MOV      CL, 12
        CALL     System.Move

        MOV      EDX, [ESP].TDIBSection.dsBm.bmBits
        MOV      [EBX].fDIBBits, EDX
        MOV      EDX, [ESP].TDIBSection.dsBmih.biSizeImage
        MOV      [EBX].fDIBSize, EDX
        MOV      [EBX].fDIBAutoFree, 1
        ADD      ESP, szDIB
        POP      [EBX].fHandle
        JMP      @@exit

@@ddb:
        MOV      ECX, [ESP+szDIB]
        PUSH     ESP
        PUSH     szB
        PUSH     ECX
        CALL     GetObject
        POP      EDX
        POP      EDX         // bmWidth
        POP      ECX         // bmHeight
        ADD      ESP, szDIB-12
        TEST     EAX, EAX
        JZ       @@exit
        MOV      [EBX].fWidth, EDX
        MOV      [EBX].fHeight, ECX
        POP      dword ptr [EBX].fHandle
        MOV      [EBX].fHandleType, 1
@@exit: POP      EBX
end;

procedure TBitmap.SetHeight(const Value: Integer);
var
 pf : TPixelFormat;
asm
        CMP      EDX, [EAX].fHeight
        JE       @@exit

        PUSHAD
        CALL     GetPixelFormat
        MOV      pf, AL
        POPAD

        PUSHAD
        XOR      EDX, EDX
        INC      EDX
        CALL     SetHandleType
        POPAD
        MOV      [EAX].fHeight, EDX
        CALL     FormatChanged

        PUSHAD
        MOV      DL, pf
        CALL     SetPixelFormat
        POPAD
@@exit:
end;

procedure TBitmap.SetPixelFormat(Value: TPixelFormat);
asm
        PUSH     EBX
        MOV      EBX, EAX
        CALL     GetEmpty   //   if Empty then Exit;
        JZ       @@exit     //
        MOV      EAX, EBX   //
        PUSH     EDX
        CALL     GetPixelFormat
        POP      EDX
        CMP      EAX, EDX
        JE       @@exit
        TEST     EDX, EDX
        MOV      EAX, EBX
        JNE      @@2
        POP      EBX
        INC      EDX // EDX = bmDDB
        JMP      SetHandleType
@@2:
        MOV      [EBX].fNewPixelFormat, DL
@@3:
        XOR      EDX, EDX
        CALL     SetHandleType
        XCHG     EAX, EBX
        CMP      EAX, 0
@@exit:
        POP      EBX
        JNE      FormatChanged
end;

function CalcScanLineSize( Header: PBitmapInfoHeader ): Integer;
asm
        MOVZX    EDX, [EAX].TBitmapInfoHeader.biBitCount
        MOV      EAX, [EAX].TBitmapInfoHeader.biWidth
        MUL      EDX
        ADD      EAX, 31
        SHR      EAX, 3
        AND      EAX, -4
end;

procedure FillBmpWithBkColor( Bmp: PBitmap; DC2: HDC; oldWidth, oldHeight: Integer );
asm
        PUSH     EBX
        PUSH     ESI
        XCHG     EAX, EBX
        PUSH     EDX // [EBP-12] = DC2
        PUSH     ECX // [EBP-16] = oldWidth
        MOV      EAX, [EBX].TBitmap.fBkColor
        CALL     Color2RGB
        TEST     EAX, EAX
        JZ       @@exit
        XCHG     ESI, EAX // ESI = Color2RGB( Bmp.fBkColor )
        MOV      EAX, EBX
        CALL     TBitmap.GetHandle
        TEST     EAX, EAX
        JZ       @@exit
        PUSH     EAX //fHandle
        PUSH     dword ptr [EBP-12] //DC2
        CALL     SelectObject
        PUSH     EAX // [EBP-20] = oldBmp
        PUSH     ESI
        CALL     CreateSolidBrush
        XCHG     ESI, EAX // ESI = Br
        PUSH     [EBX].TBitmap.fHeight
        PUSH     [EBX].TBitmap.fWidth
        MOV      EAX, [oldHeight]
        MOV      EDX, [EBP-16] //oldWidth
        CMP      EAX, [EBX].TBitmap.fHeight
        JL       @@fill
        CMP      EDX, [EBX].TBitmap.fWidth
        JGE      @@nofill
@@fill: CMP      EAX, [EBX].TBitmap.fHeight
        JNE      @@1
        XOR      EAX, EAX
@@1:
        CMP      EDX, [EBX].TBitmap.fWidth
        JNZ      @@2
        CDQ
@@2:    PUSH     EAX
        PUSH     EDX

        MOV      EDX, ESP
        PUSH     ESI
        PUSH     EDX
        PUSH     dword ptr [EBP-12] //DC2
        CALL     Windows.FillRect
        POP      ECX
        POP      ECX
@@nofill:
        POP      ECX
        POP      ECX
        PUSH     ESI //Br
        CALL     DeleteObject
        PUSH     dword ptr [EBP-12] //DC2
        CALL     SelectObject
@@exit:
        POP      ECX
        POP      EDX
        POP      ESI
        POP      EBX
end;

procedure TBitmap.FormatChanged;
type  tBIH = TBitmapInfoHeader;
      tBmp = tagBitmap;
const szBIH = Sizeof( tBIH );
      szBmp = Sizeof( tBmp );
asm
        PUSH     EAX
        CALL     GetEmpty
        POP      EAX
        JZ       @@exit
        PUSHAD
        MOV      EBX, EAX
        CALL     [EBX].fDetachCanvas
        XOR      EAX, EAX
        MOV      [EBX].fScanLineSize, EAX
        MOV      [EBX].fGetDIBPixels, EAX
        MOV      [EBX].fSetDIBPixels, EAX
        MOV      ESI, [EBX].fWidth    // ESI := oldWidth
        MOV      EDI, [EBX].fHeight   // EDI := oldHeight
        MOV      ECX, [EBX].fDIBBits
        JECXZ    @@noDIBBits
        MOV      EAX, [EBX].fDIBHeader
        MOV      ESI, [EAX].TBitmapInfo.bmiHeader.biWidth
        MOV      EDI, [EAX].TBitmapInfo.bmiHeader.biHeight
        TEST     EDI, EDI
        JGE      @@1
        NEG      EDI
@@1:    JMP      @@createDC2
@@noDIBBits:
        MOV      ECX, [EBX].fHandle
        JECXZ    @@createDC2
        ADD      ESP, -24 // -szBmp
        PUSH     ESP
        PUSH     24 //szBmp
        PUSH     ECX
        CALL     GetObject
        XCHG     ECX, EAX
        JECXZ    @@2
        MOV      ESI, [ESP].tBmp.bmWidth
        MOV      EDI, [ESP].tBmp.bmHeight
@@2:    ADD      ESP, 24 //szBmp
@@createDC2:
        PUSH     0
        CALL     CreateCompatibleDC
        PUSH     EAX                         // > DC2
        CMP      [EBX].fHandleType, bmDDB
        JNE      @@DIB_handle_type
        PUSH     0
        CALL     GetDC
        PUSH     EAX                         // > DC0
        PUSH     [EBX].fHeight
        PUSH     [EBX].fWidth
        PUSH     EAX
        CALL     CreateCompatibleBitmap
        XCHG     EBP, EAX        // EBP := NewHandle
        PUSH     0
        CALL     ReleaseDC                   // <
        POP      EDX
        PUSH     EDX             // EDX := DC2
        PUSH     EBP
        PUSH     EDX
        CALL     SelectObject
        PUSH     EAX                         // > OldBmp
        PUSH     [EBX].fHeight   // prepare Rect(0,0,fWidth,fHeight)
        PUSH     [EBX].fWidth
        PUSH     0
        PUSH     0
        MOV      EAX, [EBX].fBkColor
        CALL     Color2RGB
        PUSH     EAX
        CALL     CreateSolidBrush
        MOV      EDX, ESP
        PUSH     EAX                         // > Br
        PUSH     EAX
        PUSH     EDX
        PUSH     dword ptr [ESP+32] // (DC2)
        CALL     Windows.FillRect
        CALL     DeleteObject                // <
        ADD      ESP, 16            // remove Rect
        MOV      ECX, [EBX].fDIBBits
        JECXZ    @@draw
        PUSH     dword ptr [ESP+4] // (DC2)
        CALL     SelectObject                // < (OldBmp)
        PUSH     DIB_RGB_COLORS    // : DIB_RGB_COLORS
        PUSH     [EBX].fDIBHeader  // : fDIBHeader
        PUSH     [EBX].fDIBBits    // : fDIBBits
        PUSH     [EBX].fHeight     // : fHeight
        PUSH     0                 // : 0
        PUSH     EBP               // : NewHandle
        PUSH     dword ptr [ESP+24] // (DC2)
        CALL     SetDIBits
        JMP      @@clearData
@@draw:
        MOV      EDX, [ESP+4]
        PUSH     EDX           // prepare DC2 for SelectObject
        MOV      EAX, EBX
        XOR      ECX, ECX
        PUSH     ECX
        CALL     Draw
        CALL     SelectObject
@@clearData:
        MOV      EAX, EBX
        CALL     ClearData
        MOV      [EBX].fHandle, EBP

        JMP      @@fillBkColor

@@DIB_handle_type:    // [ESP] = DC2
        MOVZX    EAX, [EBX].fNewPixelFormat
@@getBitsPixel:
        XCHG     ECX, EAX
        MOV      CL, [ECX] + offset BitCounts
        MOVZX    EAX, [DefaultPixelFormat]
        JECXZ    @@getBitsPixel
        XOR      EBP, EBP            // NewHandle := 0
        MOV      EAX, [EBX].fWidth   // EAX := fWidth
        MOV      EDX, [EBX].fHeight  // EDX := fHeight
        CALL     PrepareBitmapHeader
        PUSH     EAX                            // > NewHeader
        CMP      [EBX].fNewPixelFormat, pf16bit
        JNE      @@newHeaderReady
        CALL     PreparePF16bit
@@newHeaderReady:
        POP      EAX
        PUSH     EAX
        CALL     CalcScanLineSize
        MOV      EDX, [EBX].fHeight
        MUL      EDX
        PUSH     EAX                           // > sizeBits

        PUSH     EAX
        PUSH     GMEM_FIXED
        CALL     GlobalAlloc

        PUSH     EAX                           // > NewBits
        PUSH     DIB_RGB_COLORS
        PUSH     dword ptr [ESP+12] // (NewHeader)
        PUSH     EAX
        MOV      EAX, [EBX].fHeight
        CMP      EAX, EDI
        {$IFDEF USE_CMOV}
        CMOVG    EAX, EDI
        {$ELSE}
        JLE      @@3
        MOV      EAX, EDI
@@3:    {$ENDIF}

        PUSH     EAX
        PUSH     0
        MOV      EAX, EBX
        CALL     GetHandle
        PUSH     EAX
        PUSH     dword ptr [ESP+36] // (DC2)
        CALL     GetDIBits

        MOV      EDX, [EBX].fHeight
        CMP      EDX, EDI
        {$IFDEF USE_CMOV}
        CMOVG    EDX, EDI
        {$ELSE}
        JLE      @@30
        MOV      EDX, EDI
@@30:   {$ENDIF}

        CMP      EAX, EDX
        JE       @@2clearData

        CALL     GlobalFree

        XOR      EAX, EAX
        PUSH     EAX

        MOV      EDX, ESP        // EDX = @NewBits
        MOV      ECX, [ESP+8]    // ECX = @NewHeader
        PUSH     EAX             // -> 0
        PUSH     EAX             // -> 0
        PUSH     EDX             // -> @NewBits
        PUSH     DIB_RGB_COLORS  // -> DIB_RGB_COLORS
        PUSH     ECX             // -> @NewHeader
        PUSH     dword ptr [ESP+32] // -> DC2
        CALL     CreateDIBSection

        XOR      ESI, -1 // use OldWidth to store NewDIBAutoFree flag

        XCHG     EBP, EAX        // EBP := NewHandle
        PUSH     EBP
        PUSH     dword ptr [ESP+16] // -> DC2
        CALL     SelectObject
        PUSH     EAX           // save oldBmp
        MOV      EDX, [ESP+16] // DC2 -> EDX (DC)
        XOR      ECX, ECX      // 0   -> ECX (X)
        PUSH     ECX           // 0   -> stack (Y)
        MOV      EAX, EBX
        CALL     TBitmap.Draw
        PUSH     dword ptr [ESP+16] // -> DC2
        CALL     SelectObject

@@2clearData:
        MOV      EAX, EBX
        CALL     ClearData

        POP      [EBX].fDIBBits
        POP      [EBX].fDIBSize
        POP      [EBX].fDIBHeader
        MOV      [EBX].fHandle, EBP

        TEST     ESI, ESI
        MOV      [EBX].fDIBAutoFree, 0
        JGE      @@noDIBautoFree
        INC      [EBX].fDIBAutoFree
@@noDIBautoFree:

@@fillBkColor:
        MOV      ECX, [EBX].fFillWithBkColor
        JECXZ    @@deleteDC2
        POP      EDX // (DC2)
        PUSH     EDX
        PUSH     EDI
        XCHG     ECX, ESI
        XCHG     EAX, EBX
        CALL     ESI
@@deleteDC2:
        CALL     DeleteDC
        POPAD
@@exit:
end;

function TBitmap.GetScanLine(Y: Integer): Pointer;
asm
        MOV      ECX, [EAX].fDIBHeader
        JECXZ    @@exit
        MOV      ECX, [ECX].TBitmapInfoHeader.biHeight
        TEST     ECX, ECX
        JL       @@1

        SUB      ECX, EDX
        DEC      ECX
        MOV      EDX, ECX

@@1:    MOV      ECX, [EAX].fScanLineSize
        INC      ECX
        PUSH     [EAX].fDIBBits
        LOOP     @@2

        PUSH     EDX
        CALL     GetScanLineSize
        POP      EDX
        XCHG     ECX, EAX

@@2:    XCHG     EAX, ECX
        MUL      EDX
        POP      ECX
        ADD      ECX, EAX

@@exit: XCHG     EAX, ECX
end;

function TBitmap.GetScanLineSize: Integer;
asm
        MOV      ECX, [EAX].fDIBHeader
        JECXZ    @@exit

        PUSH     EAX
        XCHG     EAX, ECX
        CALL     CalcScanLineSize
        XCHG     ECX, EAX
        POP      EAX
        MOV      [EAX].fScanLineSize, ECX

@@exit: XCHG     EAX, ECX
end;

procedure TBitmap.CanvasChanged( Sender : PObj );
asm
        PUSH     EAX

          XCHG     EAX, EDX
          CALL     TCanvas.GetBrush
          MOV      EDX, [EAX].TGraphicTool.fData.Color

        POP      EAX
        MOV      [EAX].fBkColor, EAX
        CALL     ClearTransImage
end;

procedure TBitmap.Dormant;
asm
        PUSH     EAX
        CALL     RemoveCanvas
        POP      EAX
        MOV      ECX, [EAX].fHandle
        JECXZ    @@exit
        CALL     ReleaseHandle
        PUSH     EAX
        CALL     DeleteObject
@@exit:
end;

procedure TBitmap.SetBkColor(const Value: TColor);
asm
        CMP      [EAX].fBkColor, EDX
        JE       @@exit
        MOV      [EAX].fBkColor, EDX
        MOV      [EAX].fFillWithBkColor, offset[FillBmpWithBkColor]
        MOV      ECX, [EAX].fApplyBkColor2Canvas
        JECXZ    @@exit
        CALL     ECX
@@exit:
end;

function TBitmap.Assign(SrcBmp: PBitmap): Boolean;
const szBIH = sizeof(TBitmapInfoHeader);
asm
        PUSHAD
        XCHG     EBX, EAX
@@clear:
        MOV      ESI, EDX
        MOV      EAX, EBX
        CALL     Clear
        MOV      EAX, ESI
        OR       EAX, EAX
        JZ       @@exit
        CALL     GetEmpty
        JZ       @@exit
        MOV      EAX, [ESI].fWidth
        MOV      [EBX].fWidth, EAX
        MOV      EAX, [ESI].fHeight
        MOV      [EBX].fHeight, EAX
        MOVZX    ECX, [ESI].fHandleType
        MOV      [EBX].fHandleType, CL
          JECXZ    @@fmtDIB

        DEC      ECX  // ECX = 0
        PUSH     ECX
        PUSH     ECX
        PUSH     ECX
        PUSH     ECX //IMAGE_BITMAP=0
        PUSH     [ESI].fHandle
        CALL     CopyImage
        MOV      [EBX].fHandle, EAX
        TEST     EAX, EAX
        XCHG     EDX, EAX
        JZ       @@clear
        JMP      @@exit

@@fmtDIB:
        XCHG     EAX, ECX
        MOV      AX, szBIH+1024
        PUSH     EAX
        CALL     System.@GetMem
        MOV      [EBX].fDIBHeader, EAX
        XCHG     EDX, EAX
        POP      ECX
        MOV      EAX, [ESI].fDIBHeader
        CALL     System.Move
        MOV      EAX, [ESI].fDIBSize
        MOV      [EBX].fDIBSize, EAX
        PUSH     EAX
        PUSH     EAX
        PUSH     GMEM_FIXED
        CALL     GlobalAlloc
        MOV      [EBX].fDIBBits, EAX
        XCHG     EDX, EAX
        POP      ECX
        MOV      EAX, [ESI].fDIBBits
        CALL     System.Move

        INC      EBX // reset "ZF"

@@exit:
        POPAD
        SETNZ    AL
end;

procedure TBitmap.RemoveCanvas;
asm
        PUSH     EAX
        CALL     [EAX].fDetachCanvas
        POP      EDX
        XOR      EAX, EAX
        XCHG     EAX, [EDX].fCanvas
        CALL     TObj.RefDec
end;

function TBitmap.DIBPalNearestEntry(Color: TColor): Integer;
const szBIH = sizeof(TBitmapInfoHeader);
asm
        PUSH     EBX
        PUSH     ESI
        PUSH     EDI
        XCHG     ESI, EAX
        XCHG     EAX, EDX
        CALL     Color2RGBQuad
        XCHG     EDI, EAX
        MOV      EAX, ESI
        CALL     GetDIBPalEntryCount
        XCHG     ECX, EAX
        XOR      EAX, EAX
        JECXZ    @@exit

        MOV      ESI, [ESI].fDIBHeader
        ADD      ESI, szBIH
        XOR      EDX, EDX
        PUSH     EDX
        DEC      DX

@@loo:  LODSD
        XOR      EAX, EDI
        MOV      EBX, EAX
        SHR      EBX, 16
        MOV      BH, 0
        ADD      AL, AH
        MOV      AH, 0
        ADC      AX, BX
        CMP      AX, DX
        JAE      @@1
        MOV      DX, AX
        POP      EBX
        PUSH     EDX // save better index (in high order word)
@@1:    ADD      EDX, $10000 // increment index
        LOOP     @@loo

        XCHG     EAX, ECX
        POP      AX
        POP      AX
@@exit:
        POP      EDI
        POP      ESI
        POP      EBX
end;

function TBitmap.GetDIBPalEntries(Idx: Integer): TColor;
const szBIH = sizeof(TBitmapInfoHeader);
asm
        MOV      ECX, [EAX].fDIBHeader
        JECXZ    @@exit

        MOV      ECX, [ECX+szBIH+EDX*4]
        INC      ECX

@@exit: DEC      ECX
        XCHG     EAX, ECX
end;

function TBitmap.GetDIBPalEntryCount: Integer;
asm
        PUSH     EAX
        CALL     GetEmpty
        POP      EAX
        JZ       @@ret0
        CALL     GetPixelFormat
        MOVZX    ECX, AL
        MOV      EAX, ECX
        LOOP     @@1
        // pf1bit:
        INC      EAX
        RET
@@1:
        LOOP     @@2
        // pf4bit:
        MOV      AL, 16
        RET
@@2:
        LOOP     @@ret0
        // pf8bit:
        XOR      EAX, EAX
        INC      AH
        RET
@@ret0:
        XOR      EAX, EAX
end;

procedure TBitmap.ClearTransImage;
asm
        OR       [EAX].fTransColor, -1
        XOR      EDX, EDX
        XCHG     [EAX].fTransMaskBmp, EDX
        XCHG     EAX, EDX
        CALL     TObj.RefDec
end;

{$IFDEF USE_OLDCONVERT2MASK}
procedure TBitmap.Convert2Mask(TranspColor: TColor);
asm
        PUSH     EBX
        PUSH     ESI
        MOV      EBX, EAX
        MOV      ESI, EDX
        CALL     GetHandle
        TEST     EAX, EAX
        JZ       @@exit

        PUSH     0
        PUSH     1
        PUSH     1
        PUSH     [EBX].fHeight
        PUSH     [EBX].fWidth
        CALL     CreateBitmap
        PUSH     EAX                // MonoHandle
        PUSH     0
        CALL     CreateCompatibleDC
        POP      EDX
        PUSH     EDX
        PUSH     EAX                // MonoDC

        PUSH     EDX
        PUSH     EAX
        CALL     SelectObject
        PUSH     EAX                // SaveMono

        CALL     StartDC            // DCfrom, SaveFrom
        XCHG     EAX, ESI
        CALL     Color2RGB
        PUSH     EAX // Color2RGB(TranspColor)
        PUSH     dword ptr [ESP+8] //DCfrom
        CALL     Windows.SetBkColor
        PUSH     EAX                // SaveBkColor

        PUSH     SRCCOPY
        PUSH     0
        PUSH     0
        PUSH     dword ptr [ESP+12+4+4] //DCfrom
        PUSH     [EBX].fHeight
        PUSH     [EBX].fWidth
        PUSH     0
        PUSH     0
        PUSH     dword ptr [ESP+32+16] //MonoDC
        CALL     BitBlt

        PUSH     dword ptr [ESP+8] //DCfrom
        CALL     Windows.SetBkColor // ESP-> SaveFrom
        CALL     FinishDC           // ESP-> SaveMono
        CALL     FinishDC           // ESP-> MonoHandle

        MOV      EAX, EBX
        CALL     ClearData
        POP      [EBX].fHandle
        MOV      [EBX].fHandleType, bmDDB
@@exit:
        POP      ESI
        POP      EBX
end;
{$ELSE USE_OLDCONVERT2MASK} //Pascal
procedure TBitmap.Convert2Mask(TranspColor: TColor);
asm
        PUSH EBX
        PUSH ESI
        PUSH EBP
        PUSH EDI
        XCHG EBP, EAX          // EBP = @ Self
        XCHG EAX, EDX          // EAX = TranspColor
        CALL Color2RGB
        XCHG EBX, EAX          // EBX := Color2RGB( TranspColor );
        MOV  EAX, EBP          // EAX := @ Self;
        CALL GetPixelFormat
        CMP  AL, pf15bit
        JB   @@SwapRB
        CMP  AL, pf24bit
        JB   @@noSwapRB
@@SwapRB:
        BSWAP EBX
        SHR   EBX, 8
@@noSwapRB:
        MOV  DL, pf4bit
        CMP  AL, DL
        JB   @@setpixelformat
@@1:    MOV  DL, pf32bit
        CMP  AL, DL
        JBE  @@translate
@@setpixelformat:
        MOV  EAX, EBP
        CALL SetPixelFormat
@@translate:
        MOV  EAX, [EBP].fWidth
        MOV  EDX, [EBP].fHeight
        MOV  CL, pf1bit
        CALL NewDibBitmap
        PUSH EAX
        XOR  EDX, EDX
        INC  EDX
        MOV  ECX, $FFFFFF
        CALL SetDIBPalEntries
        XOR  EDX, EDX
@@Yloop:CMP  EDX, [EBP].fHeight
        JGE  @@exit
        PUSH EDX
        MOV  EAX, EBP
        CALL GetScanLine
        XCHG ESI, EAX
        MOV  EAX, [ESP+4]
        POP  EDX
        PUSH EDX
        CALL GetScanLine
        XCHG EDI, EAX
        MOV  EAX, EBP
        CALL GetPixelFormat
        MOVZX ECX, AL
        SUB  ECX, pf4bit
        MOV  DL, 8
        JNE  @@chk_pf8bit
        //-------- pf4bit:
        CMP  dword ptr [ESP], 0
        JNZ  @@4_0
        XOR  EDX, EDX
@@4_searchentry:
        PUSH EDX
        MOV  EAX, EBP //[ESP+8]
        CALL GetDIBPalEntries
        CMP  EAX, EBX
        POP  EDX
        JZ   @@4_foundentry
        INC  EDX
        CMP  EDX, 16
        JB   @@4_searchentry
@@4_foundentry:
        XCHG EBX, EDX
        MOV  DL, 8
@@4_0:  MOV  ECX, [EBP].fWidth
        INC  ECX
        SHR  ECX, 1
@@Xloop_pf4bit:
        MOV  AH, [ESI]
        SHR  AH, 4
        CMP  AH, BL
        SETZ AH
        SHL  AL, 1
        OR   AL, AH
        MOV  AH, [ESI]
        AND  AH, $0F
        CMP  AH, BL
        SETZ AH
        SHL  AL, 1
        OR   AL, AH
        DEC  DL
        DEC  DL
        JNZ  @@4_1
        STOSB
        MOV  DL, 8
@@4_1: INC  ESI
        LOOP @@Xloop_pf4bit
        JMP  @@nextline
@@chk_pf8bit:
        LOOP @@chk_pf15bit
        //-------- pf4bit:
        CMP  dword ptr [ESP], 0
        JNZ  @@8_0
        XOR  EDX, EDX
@@8_searchentry:
        PUSH EDX
        MOV  EAX, EBP //[ESP+8]
        CALL GetDIBPalEntries
        CMP  EAX, EBX
        POP  EDX
        JZ   @@8_foundentry
        INC  DL
        JNZ  @@8_searchentry
@@8_foundentry:
        XCHG EBX, EDX
        MOV  DL, 8
@@8_0:  MOV  ECX, [EBP].fWidth
        INC  ECX
@@Xloop_pf8bit:
        CMP  BL, [ESI]
        SETZ AH
        SHL  AL, 1
        OR   AL, AH
        DEC  DL
        JNZ  @@8_1
        STOSB
        MOV  DL, 8
@@8_1:  INC  ESI
        LOOP @@Xloop_pf8bit
        JMP  @@nextline
@@chk_pf15bit:
        LOOP @@chk_pf16bit
        //-------- pf15bit:
        CMP  dword ptr [ESP], 0
        JNZ  @@15_0
        XCHG EAX, EBX
        PUSH EDX
        CALL Color2Color15
        POP  EDX
        XCHG EBX, EAX
@@15_0: MOV  ECX, [EBP].fWidth
@@Xloop_pf15bit:
        CMP  word ptr [ESI], BX
        SETZ AH
        SHL  AL, 1
        OR   AL, AH
        DEC  DL
        JNZ  @@15_1
        STOSB
        MOV  DL, 8
@@15_1: ADD  ESI, 2
        LOOP @@Xloop_pf15bit
        JMP  @@nextline
@@chk_pf16bit:
        LOOP @@chk_pf24bit
        //-------- pf16bit:
        CMP  dword ptr [ESP], 0
        JNZ  @@16_0
        XCHG EAX, EBX
        PUSH EDX
        CALL Color2Color16
        POP  EDX
        XCHG EBX, EAX
@@16_0: MOV  ECX, [EBP].fWidth
@@Xloop_pf16bit:
        CMP  word ptr [ESI], BX
        SETZ AH
        SHL  AL, 1
        OR   AL, AH
        DEC  DL
        JNZ  @@16_1
        STOSB
        MOV  DL, 8
@@16_1: ADD  ESI, 2
        LOOP @@Xloop_pf16bit
        JMP  @@nextline
@@chk_pf24bit:
        LOOP @@chk_pf32bit
        //-------- pf24bit:
        MOV  ECX, [EBP].fWidth
        PUSH EBP
        //AND  EBX, $FFFFFF
@@Xloop_pf24bit:
        MOV  EBP, dword ptr [ESI]
        AND  EBP, $FFFFFF
        CMP  EBP, EBX
        SETZ AH
        SHL  AL, 1
        OR   AL, AH
        DEC  DL
        JNZ  @@24_1
        STOSB
        MOV  DL, 8
@@24_1: ADD  ESI, 3
        LOOP @@Xloop_pf24bit
        POP  EBP
        JMP  @@nextline
@@chk_pf32bit:
        //-------- pf32bit:
        MOV  ECX, [EBP].fWidth
@@Xloop_pf32bit:
        and  dword ptr [ESI], $FFFFFF
        CMP  EBX, dword ptr [ESI]
        SETZ AH
        SHL  AL, 1
        OR   AL, AH
        DEC  DL
        JNZ  @@32_1
        STOSB
        MOV  DL, 8
@@32_1: ADD  ESI, 4
        LOOP @@Xloop_pf32bit
@@nextline:
        TEST DL, DL
        JZ   @@nx1
        CMP  DL, 8
        JE   @@nx1
@@finloop1:
        SHL  AL, 1
        DEC  DL
        JNZ  @@finloop1
        STOSB
@@nx1:
        POP  EDX
        INC  EDX
        JMP  @@Yloop
@@exit:
        POP  EDX
        PUSH EDX
        XCHG EAX, EBP
        CALL Assign
        POP  EAX
        CALL TObj.RefDec
        POP  EDI
        POP  EBP
        POP  ESI
        POP  EBX
end;
{$ENDIF USE_OLDCONVERT2MASK} //Pascal

procedure _PrepareBmp2Rotate;
const szBIH = sizeof(TBitmapInfoHeader);
asm
        { <- BL = increment to height }
        XCHG     EDI, EAX
        MOV      ESI, EDX // ESI = SrcBmp

        XCHG     EAX, EDX
        CALL     TBitmap.GetPixelFormat
        MOVZX    ECX, AL
        PUSH     ECX

        MOV      EDX, [ESI].TBitmap.fWidth
        MOVZX    EBX, BL
        ADD      EDX, EBX

        MOV      EAX, [ESI].TBitmap.fHeight
        CALL     NewDIBBitmap
        STOSD
        XCHG     EDI, EAX

        MOV      EAX, [ESI].TBitmap.fDIBHeader
        ADD      EAX, szBIH
        MOV      EDX, [EDI].TBitmap.fDIBHeader
        ADD      EDX, szBIH
        XOR      ECX, ECX
        MOV      CH, 4
        CALL     System.Move

        MOV      EAX, EDI
        XOR      EDX, EDX
        CALL     TBitmap.GetScanLine
        MOV      EBX, [EDI].TBitmap.fWidth
        DEC      EBX // EBX = DstBmp.fWidth - 1
        XCHG     EDI, EAX // EDI = DstBmp.ScanLine[ 0 ]

        XOR      EDX, EDX
        INC      EDX
        CALL     TBitmap.GetScanLine
        XCHG     EDX, EAX
        SUB      EDX, EDI // EDX = BytesPerDstLine

        MOV      EBP, [ESI].TBitmap.fWidth
        DEC      EBP // EBP = SrcBmp.fWidth - 1

        POP      ECX // ECX = PixelFormat
end;
procedure _RotateBitmapMono( var DstBmp: PBitmap; SrcBmp: PBitmap );
const szBIH = sizeof(TBitmapInfoHeader);
asm
        PUSHAD
        MOV      BL, 7
        CALL     _PrepareBmp2Rotate

        SHR      EBP, 3
        SHL      EBP, 8 // EBP = (WBytes-1) * 256

        MOV      ECX, EBX // ECX and 7 = Shf
        SHR      EBX, 3
        ADD      EDI, EBX // EDI = Dst

        XOR      EBX, EBX // EBX = temp mask
        XOR      EAX, EAX // Y = 0
@@looY:
        PUSH     EAX
        PUSH     EDI // Dst1 = Dst (Dst1 in EDI, Dst saved)
        PUSH     ESI // SrcBmp

        PUSH     EDX //BytesPerDstLine
        PUSH     ECX //Shf

        XCHG     EDX, EAX
        XCHG     EAX, ESI
        CALL     TBitmap.GetScanLine
        XCHG     ESI, EAX // ESI = Src

        POP      ECX // CL = Shf
        AND      ECX, 7 // ECX = Shf
        OR       ECX, EBP // ECX = (Wbytes-1)*8 + Shf
        POP      EDX // EDX = BytesPerDstLine

        MOV      BH, $80
        SHR      EBX, CL // BH = mask, BL = mask & Tmp
@@looX:
        XOR      EAX, EAX

        LODSB

        MOV      AH, AL
        SHR      EAX, CL
        OR       EAX,$01000000

@@looBits:
        MOV      BL, AH
        AND      BL, BH
        OR       [EDI], BL
        ADD      EDI, EDX
        ADD      EAX, EAX
        JNC      @@looBits

        SUB      ECX, 256
        JGE      @@looX

        POP      ESI // ESI = SrcBmp
        POP      EDI // EDI = Dst
        POP      EAX // EAX = Y

        ADD      ECX, 256-1
        JGE      @@1
        DEC      EDI
@@1:
        INC      EAX
        CMP      EAX, [ESI].TBitmap.fHeight
        JL       @@looY

        POPAD
end;

procedure _RotateBitmap4bit( var DstBmp: PBitmap; SrcBmp: PBitmap );
const szBIH = sizeof(TBitmapInfoHeader);
asm
        PUSHAD
        MOV      BL, 1
        CALL     _PrepareBmp2Rotate

        SHR      EBP, 1 // EBP = WBytes - 1
        SHL      EBP, 8 // EBP = (WBytes - 1) * 256

        // EBX = DstBmp.fWidth - 1
        MOV      ECX, EBX
        SHL      ECX, 2 // ECX and 7 = Shf (0 or 4)
        SHR      EBX, 1
        ADD      EDI, EBX // EDI = Dst

        XOR      EAX, EAX // Y = 0
        XOR      EBX, EBX

@@looY:
        PUSH     EAX // save Y
        PUSH     EDI // Dst1 = Dst (Dst1 in EDI, Dst saved)
        PUSH     ESI // SrcBmp

        PUSH     EDX // BytesPerDstLine
        PUSH     ECX // Shf

        XCHG     EDX, EAX
        XCHG     EAX, ESI
        CALL     TBitmap.GetScanLine
        XCHG     ESI, EAX // ESI = Src

        POP      ECX
        AND      ECX, 7 // CL = Shf
        OR       ECX, EBP // ECX = (WBytes-1)*256 + Shf
        POP      EDX // EDX = BytesPerDstLine

        MOV      BH, $F0
        SHR      EBX, CL // shift mask right 4 or 0

@@looX:
        XOR      EAX, EAX
        LODSB
        MOV      AH, AL
        SHR      EAX, CL

        MOV      BL, AH
        AND      BL, BH
        OR       [EDI], BL
        ADD      EDI, EDX

        SHL      EAX, 4
        AND      AH, BH
        OR       [EDI], AH
        ADD      EDI, EDX

        SUB      ECX, 256
        JGE      @@looX

        POP      ESI // ESI = SrcBmp
        POP      EDI // EDI = Dst
        POP      EAX // EAX = Y

        ADD      ECX, 256 - 4
        JGE      @@1

        DEC      EDI
@@1:
        INC      EAX
        CMP      EAX, [ESI].TBitmap.fHeight
        JL       @@looY

        POPAD
end;

procedure _RotateBitmap8bit( var DstBmp: PBitmap; SrcBmp: PBitmap );
const szBIH = sizeof(TBitmapInfoHeader);
asm
        PUSHAD
        XOR      EBX, EBX
        CALL     _PrepareBmp2Rotate

        ADD      EDI, EBX // EDI = Dst

        MOV      EBX, EDX // EBX = BytesPerDstLine
        DEC      EBX
        MOV      EBP, ESI // EBP = SrcBmp

        XOR      EDX, EDX // Y = 0

@@looY:
        PUSH     EDX
        PUSH     EDI

        MOV      EAX, EBP
        CALL     TBitmap.GetScanLine
        XCHG     ESI, EAX
        MOV      ECX, [EBP].TBitmap.fWidth

@@looX:
        MOVSB
        ADD      EDI, EBX
        LOOP     @@looX

        POP      EDI
        POP      EDX

        DEC      EDI
        INC      EDX
        CMP      EDX, [EBP].TBitmap.fHeight
        JL       @@looY

        POPAD
end;

procedure _RotateBitmap16bit( var DstBmp: PBitmap; SrcBmp: PBitmap );
asm
        PUSHAD
        XOR      EBX, EBX
        CALL     _PrepareBmp2Rotate

        ADD      EBX, EBX
        ADD      EDI, EBX // EDI = Dst
        MOV      EBX, EDX // EBX = BytesPerDstLine
        DEC      EBX
        DEC      EBX
        MOV      EBP, ESI // EBP = SrcBmp

        XOR      EDX, EDX // Y = 0

@@looY:
        PUSH     EDX
        PUSH     EDI

        MOV      EAX, EBP
        CALL     TBitmap.GetScanLine
        XCHG     ESI, EAX
        MOV      ECX, [EBP].TBitmap.fWidth

@@looX:
        MOVSW
        ADD      EDI, EBX
        LOOP     @@looX

        POP      EDI
        POP      EDX

        DEC      EDI
        DEC      EDI
        INC      EDX
        CMP      EDX, [EBP].TBitmap.fHeight
        JL       @@looY

        POPAD
end;

procedure _RotateBitmap2432bit( var DstBmp: PBitmap; SrcBmp: PBitmap );
asm
        PUSHAD
        XOR      EBX, EBX
        CALL     _PrepareBmp2Rotate

        SUB      ECX, pf24bit
        JNZ      @@10
        LEA      EBX, [EBX+EBX*2]
        JMP      @@11
@@10:
        LEA      EBX, [EBX*4]
@@11:   ADD      EDI, EBX // EDI = Dst

        MOV      EBX, EDX // EBX = BytesPerDstLine
        DEC      EBX
        DEC      EBX
        DEC      EBX

        MOV      EBP, ESI // EBP = SrcBmp

        XOR      EDX, EDX // Y = 0

@@looY:
        PUSH     EDX
        PUSH     EDI
        PUSH     ECX // ECX = 0 if pf24bit (1 if pf32bit)

        MOV      EAX, EBP
        CALL     TBitmap.GetScanLine
        XCHG     ESI, EAX
        MOV      ECX, [EBP].TBitmap.fWidth
        POP      EAX
        PUSH     EAX

@@looX:
        MOVSW
        MOVSB
        ADD      ESI, EAX
        ADD      EDI, EBX
        LOOP     @@looX

        POP      ECX
        POP      EDI
        POP      EDX

        DEC      EDI
        DEC      EDI
        DEC      EDI
        SUB      EDI, ECX
        INC      EDX
        CMP      EDX, [EBP].TBitmap.fHeight
        JL       @@looY

        POPAD
end;

procedure _RotateBitmapRight( SrcBmp: PBitmap );
asm
        PUSH     EBX
        PUSH     EDI
        MOV      EBX, EAX
        CMP      [EBX].TBitmap.fHandleType, bmDIB
        JNZ      @@exit

        CALL     TBitmap.GetPixelFormat
        MOVZX    ECX, AL
        LOOP     @@not1bit
        MOV      EAX, [RotateProcs.proc_RotateBitmapMono]
@@not1bit:
        LOOP     @@not4bit
        MOV      EAX, [RotateProcs.proc_RotateBitmap4bit]
@@not4bit:
        LOOP     @@not8bit
        MOV      EAX, [RotateProcs.proc_RotateBitmap8bit]
@@not8bit:
        LOOP     @@not15bit
        INC      ECX
@@not15bit:
        LOOP     @@not16bit
        MOV      EAX, [RotateProcs.proc_RotateBitmap16bit]
@@not16bit:
        LOOP     @@not24bit
        INC      ECX
@@not24bit:
        LOOP     @@not32bit
        MOV      EAX, [RotateProcs.proc_RotateBitmap2432bit]
@@not32bit:
        TEST     EAX, EAX
        JZ       @@exit

        PUSH     ECX
        XCHG     ECX, EAX
        MOV      EAX, ESP
        MOV      EDX, EBX
        CALL     ECX

        POP      EDI
        MOV      EAX, [EBX].TBitmap.fWidth
        CMP      EAX, [EDI].TBitmap.fHeight
        JGE      @@noCutHeight

        MOV      EDX, [EDI].TBitmap.fScanLineSize
        MUL      EDX
        MOV      [EDI].TBitmap.fDIBSize, EAX

        MOV      EDX, [EDI].TBitmap.fDIBHeader
        MOV      EDX, [EDX].TBitmapInfoHeader.biHeight
        TEST     EDX, EDX
        JL       @@noCorrectImg

        PUSH     EAX

        MOV      EDX, [EDI].TBitmap.fHeight
        DEC      EDX
        MOV      EAX, EDI
        CALL     TBitmap.GetScanLine
        PUSH     EAX

        MOV      EDX, [EBX].TBitmap.fWidth
        DEC      EDX
        MOV      EAX, EDI
        CALL     TBitmap.GetScanLine
        POP      EDX

        POP      ECX
        CALL     System.Move

@@noCorrectImg:
        MOV      EAX, [EBX].TBitmap.fWidth
        MOV      [EDI].TBitmap.fHeight, EAX
        MOV      EDX, [EDI].TBitmap.fDIBHeader
        MOV      [EDX].TBitmapInfoHeader.biHeight, EAX

@@noCutHeight:
        MOV      EAX, EBX
        CALL     TBitmap.ClearData

        XOR      EAX, EAX
        XCHG     EAX, [EDI].TBitmap.fDIBHeader
        XCHG     [EBX].TBitmap.fDIBHeader, EAX

        XCHG     EAX, [EDI].TBitmap.fDIBBits
        XCHG     [EBX].TBitmap.fDIBBits, EAX

        MOV      AL, [EDI].TBitmap.fDIBAutoFree
        MOV      [EBX].TBitmap.fDIBAutoFree, AL

        MOV      EAX, [EDI].TBitmap.fDIBSize
        MOV      [EBX].TBitmap.fDIBSize, EAX

        MOV      EAX, [EDI].TBitmap.fWidth
        MOV      [EBX].TBitmap.fWidth, EAX

        MOV      EAX, [EDI].TBitmap.fHeight
        MOV      [EBX].TBitmap.fHeight, EAX

        XCHG     EAX, EDI
        CALL     TObj.RefDec
@@exit:
        POP      EDI
        POP      EBX
end;

function TBitmap.GetPixels(X, Y: Integer): TColor;
asm
        PUSH     EBX
        MOV      EBX, EAX
        PUSH     ECX
        PUSH     EDX
        CALL     GetEmpty
        PUSHFD
        OR       EAX, -1
        POPFD
        JZ       @@exit

        CALL     StartDC
        PUSH     dword ptr [ESP+12]
        PUSH     dword ptr [ESP+12]
        PUSH     EAX
        CALL     Windows.GetPixel
        XCHG     EBX, EAX
        CALL     FinishDC
        XCHG     EAX, EBX
@@exit:
        POP      EDX
        POP      EDX
        POP      EBX
end;

procedure TBitmap.SetPixels(X, Y: Integer; const Value: TColor);
asm
        PUSH     EBX
        MOV      EBX, EAX
        PUSH     ECX
        PUSH     EDX
        CALL     GetEmpty
        JZ       @@exit

        CALL     StartDC
        MOV      EAX, Value
        CALL     Color2RGB
        PUSH     EAX
        PUSH     dword ptr [ESP+16]
        PUSH     dword ptr [ESP+16]
        PUSH     dword ptr [ESP+16]
        CALL     Windows.SetPixel
        CALL     FinishDC
@@exit:
        POP      EDX
        POP      ECX
        POP      EBX
end;

function _GetDIBPixelsPalIdx( Bmp: PBitmap; X, Y: Integer ): TColor;
const szBIH = Sizeof(TBitmapInfoHeader);
asm
        PUSH     EBX
        PUSH     EDI
        PUSH     EDX
        XCHG     EBX, EAX

        XCHG     EAX, EDX
        MOV      EDI, [EBX].TBitmap.fPixelsPerByteMask
        INC      EDI
        CDQ
        DIV      EDI
        DEC      EDI
        XCHG     ECX, EAX // EAX = Y, ECX = X div (Bmp.fPixeldPerByteMask+1)

        MOV      EDX, [EBX].TBitmap.fScanLineDelta
        IMUL     EDX

        ADD      EAX, [EBX].TBitmap.fScanLine0
        MOVZX    EAX, byte ptr[EAX+ECX]

        POP      EDX
        MOV      ECX, [EBX].TBitmap.fPixelsPerByteMask
        AND      EDX, ECX
        SUB      ECX, EDX

        PUSH     EAX
        MOV      EDI, [EBX].TBitmap.fDIBHeader
        MOVZX    EAX, [EDI].TBitmapInfoHeader.biBitCount
        MUL      ECX
        XCHG     ECX, EAX
        POP      EAX
        SHR      EAX, CL
        AND      EAX, [EBX].TBitmap.fPixelMask

        MOV      EAX, [EDI+szBIH+EAX*4]
        CALL     Color2RGBQuad

        POP      EDI
        POP      EBX
end;

function _GetDIBPixels16bit( Bmp: PBitmap; X, Y: Integer ): TColor;
asm
        PUSH     [EAX].TBitmap.fPixelMask
        PUSH     EDX // X
        PUSH     EAX
        MOV      EAX, [EAX].TBitmap.fScanLineDelta
        IMUL     ECX
        POP      EDX
        ADD      EAX, [EDX].TBitmap.fScanLine0
        POP      ECX
        MOVZX    EAX, word ptr [EAX+ECX*2]
        POP      EDX
        CMP      DL, 15
        JNE      @@16bit

        MOV      EDX, EAX
        SHR      EDX, 7
        SHL      EAX, 6
        MOV      DH, AH
        AND      DH, $F8
        SHL      EAX, 13
        JMP      @@1516bit

@@16bit:
        MOV      DL, AH
        SHL      EAX, 5
        MOV      DH, AH
        SHL      EAX, 14
@@1516bit:
        AND      EAX, $F80000
        OR       EAX, EDX
        AND      AX, $FCF8
end;

function _GetDIBPixelsTrueColor( Bmp: PBitmap; X, Y: Integer ): TColor;
asm
        PUSH     EBX
        XCHG     EBX, EAX
        PUSH     EDX
        MOV      EAX, [EBX].TBitmap.fScanLineDelta
        IMUL     ECX
        XCHG     ECX, EAX
        POP      EDX
        MOV      EAX, [EBX].TBitmap.fBytesPerPixel
        MUL      EDX
        ADD      EAX, [EBX].TBitmap.fScanLine0
        MOV      EAX, [EAX+ECX]
        AND      EAX, $FFFFFF
        CALL     Color2RGBQuad
        POP      EBX
end;

function _GetDIBPixelsTrueColorAlpha( Bmp: PBitmap; X, Y: Integer ): TColor;
asm
        PUSH     EBX
        XCHG     EBX, EAX
        PUSH     EDX
        MOV      EAX, [EBX].TBitmap.fScanLineDelta
        IMUL     ECX
        XCHG     ECX, EAX
        POP      EDX
        MOV      EAX, [EBX].TBitmap.fBytesPerPixel
        MUL      EDX
        ADD      EAX, [EBX].TBitmap.fScanLine0
        MOV      EAX, [EAX+ECX]
        MOV      EDX, EAX
        AND      EDX, $FF00FF
        AND      EAX, $FF00FF00
        ROL      EDX, 16
        OR       EAX, EDX
        POP      EBX
end;

function TBitmap.GetDIBPixels(X, Y: Integer): TColor;
asm
        CMP      word ptr [EAX].fGetDIBPixels+2, 0
        JNZ      @@assigned

        // if not assigned, this preparing will be performed for first call:
        CMP      [EAX].fHandleType, bmDDB
        JZ       @@GetPixels

        PUSHAD
        MOV      EBX, EAX
        XOR      EDX, EDX
        CALL     GetScanLine
        MOV      [EBX].fScanLine0, EAX
        XOR      EDX, EDX
        INC      EDX
        MOV      EAX, EBX
        CALL     GetScanLine
        SUB      EAX, [EBX].fScanLine0
        MOV      [EBX].fScanLineDelta, EAX
        MOV      EAX, EBX
        CALL     GetPixelFormat
        MOVZX    ECX, AL
        MOV      DX, $0F00
        MOV      byte ptr [EBX].fBytesPerPixel, 4
        XOR      EAX, EAX
        LOOP     @@if4bit
        MOV      DX, $0107
        JMP      @@1bit4bit8bit
@@if4bit:
        LOOP     @@if8bit
        INC      EDX // MOV      DX, $0F01
        JMP      @@1bit4bit8bit
@@if8bit:
        LOOP     @@if15bit
        MOV      DH, $FF //MOV      DX, $FF00
@@1bit4bit8bit:
        MOV      EAX, offset[_GetDIBPixelsPalIdx]
@@if15bit:
        LOOP     @@if16bit
        //MOV      DH, $0F
        DEC      DH
        INC      ECX
@@if16bit:
        LOOP     @@if24bit
        INC      DH
        MOV      EAX, offset[_GetDIBPixels16bit]
@@if24bit:
        LOOP     @@if32bit
        DEC      [EBX].fBytesPerPixel
        INC      ECX
        DEC      EDX
@@if32bit:
        LOOP     @@iffin
        INC      EDX
        {$IFDEF DIBPixels32bitWithAlpha}
        MOV      EAX, offset[_GetDIBPixelsTrueColorAlpha]
        {$ELSE}
        MOV      EAX, offset[_GetDIBPixelsTrueColor]
        {$ENDIF}
@@iffin:
        MOV      byte ptr [EBX].fPixelMask, DH
        MOV      byte ptr [EBX].fPixelsPerByteMask, DL
        MOV      [EBX].fGetDIBPixels, EAX
        TEST     EAX, EAX
        POPAD
@@GetPixels:
        JZ       GetPixels

@@assigned:
        JMP      [EAX].fGetDIBPixels
end;

procedure _SetDIBPixels1bit( Bmp: PBitmap; X, Y: Integer; Value: TColor );
asm
        PUSH     EDX
        PUSH     [EAX].TBitmap.fScanLine0
        PUSH     ECX
        PUSH     [EAX].TBitmap.fScanLineDelta
        MOV      EAX, Value
        CALL     Color2RGB
        MOV      EDX, EAX
        SHR      EAX, 16
        ADD      AL, DL
        ADC      AL, DH
        CMP      EAX, 170
        SETGE    CL
        AND      ECX, 1
        SHL      ECX, 7
        POP      EAX
        POP      EDX
        IMUL     EDX
        POP      EDX
        ADD      EAX, EDX
        POP      EDX
        PUSH     ECX
        MOV      ECX, EDX
        SHR      EDX, 3
        ADD      EAX, EDX
        AND      ECX, 7
        MOV      DX, $FF7F
        SHR      EDX, CL
        AND      byte ptr [EAX], DL
        POP      EDX
        SHR      EDX, CL
        OR       byte ptr [EAX], DL
end;

procedure _SetDIBPixelsPalIdx( Bmp: PBitmap; X, Y: Integer; Value: TColor );
asm
        XCHG     EAX, EBP
        PUSH     EDX // -> X
        PUSH     ECX // -> Y
        MOV      ECX, [EBP].TBitmap.fPixelsPerByteMask
        INC      ECX
        XCHG     EAX, EDX
        CDQ
        DIV      ECX
        XCHG     ECX, EAX // ECX = X div (fPixelsPerByteMask+1)
        POP      EAX // <- Y
        MOV      EDX, [EBP].TBitmap.fScanLineDelta
        IMUL     EDX
        ADD      ECX, EAX
        ADD      ECX, [EBP].TBitmap.fScanLine0 // ECX = Pos
        PUSH     ECX // -> Pos

        MOV      EDX, [ESP+16] // Value
        MOV      EAX, EBP
        CALL     TBitmap.DIBPalNearestEntry // EAX = Pixel

        POP      ECX // <- Pos
        POP      EDX // <- X

        PUSH     EAX // -> Pixel

        MOV      EAX, [EBP].TBitmap.fPixelsPerByteMask
        AND      EDX, EAX
        SUB      EAX, EDX
        MOV      EDX, [EBP].TBitmap.fDIBHeader
        MOVZX    EDX, [EDX].TBitmapInfoHeader.biBitCount
        MUL      EDX // EAX = Shf

        XCHG     ECX, EAX // ECX = Shf, EAX = Pos
        MOV      EDX, [EBP].TBitmap.fPixelMask
        SHL      EDX, CL
        NOT      EDX
        AND      byte ptr [EAX], DL

        POP      EDX // <- Pixel
        SHL      EDX, CL
        OR       byte ptr [EAX], DL
end;

procedure _SetDIBPixels16bit( Bmp: PBitmap; X, Y: Integer; Value: TColor );
asm
        ADD      EDX, EDX
        ADD      EDX, [EAX].TBitmap.fScanLine0
        PUSH     EDX // -> X*2 + Bmp.fScanLine0
        PUSH     [EAX].TBitmap.fPixelMask
        MOV      EAX, [EAX].TBitmap.fScanLineDelta
        IMUL     ECX
        PUSH     EAX  // -> Y* Bmp.fScanLineDelta
        MOV      EAX, Value
        CALL     Color2RGB
        POP      EBP  // <- Y* Bmp.fScanLineDelta
        POP      EDX
        XOR      ECX, ECX
        SUB      DL, 16
        JZ       @@16bit

        MOV      CH, AL
        SHR      CH, 1
        SHR      EAX, 6
        MOV      EDX, EAX
        AND      DX, $3E0
        SHR      EAX, 13
        JMP      @@1516

@@16bit:
        {$IFDEF PARANOIA} DB $24, $F8 {$ELSE} AND AL, $F8 {$ENDIF}
        MOV      CH, AL
        SHR      EAX, 5
        MOV      EDX, EAX
        AND      DX, $7E0
        SHR      EAX, 14

@@1516:
        MOV      AH, CH
        AND      AX, $FC1F
        OR       AX, DX

        POP      EDX
        MOV      [EBP+EDX], AX
end;

procedure _SetDIBPixelsTrueColor( Bmp: PBitmap; X, Y: Integer; Value: TColor );
asm
        PUSH     [EAX].TBitmap.fScanLineDelta
        PUSH     [EAX].TBitmap.fScanLine0
        MOV      EAX, [EAX].TBitmap.fBytesPerPixel
        MUL      EDX
        POP      EDX
        ADD      EDX, EAX
        POP      EAX
        PUSH     EDX
        IMUL     ECX
        POP      EDX
        ADD      EDX, EAX
        PUSH     EDX
        MOV      EAX, Value
        CALL     Color2RGBQuad
        POP      EDX
        AND      dword ptr [EDX], $FF000000
        OR       [EDX], EAX
end;

procedure _SetDIBPixelsTrueColorAlpha( Bmp: PBitmap; X, Y: Integer; Value: TColor );
asm
        PUSH     [EAX].TBitmap.fScanLineDelta
        PUSH     [EAX].TBitmap.fScanLine0
        MOV      EAX, [EAX].TBitmap.fBytesPerPixel
        MUL      EDX
        POP      EDX
        ADD      EDX, EAX
        POP      EAX
        PUSH     EDX
        IMUL     ECX
        POP      EDX
        ADD      EDX, EAX
        MOV      EAX, Value
        MOV      ECX, EAX
        AND      ECX, $FF00FF
        AND      EAX, $FF00FF00
        ROL      ECX, 16
        OR       EAX, ECX
        MOV      [EDX], EAX
end;

procedure TBitmap.SetDIBPixels(X, Y: Integer; const Value: TColor);
asm
        CMP      word ptr [EAX].fSetDIBPixels+2, 0
        JNZ      @@assigned
        PUSHAD
        MOV      EBX, EAX
        XOR      EDX, EDX
        CMP      [EBX].fHandleType, DL // bmDIB = 0
        JNE      @@ddb
        CALL     GetScanLine
        MOV      [EBX].fScanLine0, EAX
        XOR      EDX, EDX
        INC      EDX
        MOV      EAX, EBX
        CALL     GetScanLine
        SUB      EAX, [EBX].fScanLine0
        MOV      [EBX].fScanLineDelta, EAX
        MOV      EAX, EBX
        CALL     GetPixelFormat
        MOVZX    ECX, AL
        MOV      DX, $0F01
        MOV      EAX, offset[_SetDIBPixelsPalIdx]
        MOV      byte ptr [EBX].fBytesPerPixel, 4
        LOOP     @@if4bit
        MOV      EAX, offset[_SetDIBPixels1bit]
@@if4bit:
        LOOP     @@if8bit
@@if8bit:
        LOOP     @@if15bit
        DEC      DL
        MOV      DH, $FF
@@if15bit:
        LOOP     @@if16bit
        DEC      DH
        INC      ECX
@@if16bit:
        LOOP     @@if24bit
        INC      DH
        MOV      EAX, offset[_SetDIBPixels16bit]
@@if24bit:
        LOOP     @@if32bit
        DEC      EDX
        DEC      [EBX].fBytesPerPixel
        INC      ECX
@@if32bit:
        LOOP     @@ifend
        INC      EDX
        {$IFDEF DIBPixels32bitWithAlpha}
        MOV      EAX, offset[_SetDIBPixelsTrueColor]
        {$ELSE}
        MOV      EAX, offset[_SetDIBPixelsTrueColor]
        {$ENDIF}
@@ifend:
        MOV      byte ptr [EBX].fPixelMask, DH
        MOV      byte ptr [EBX].fPixelsPerByteMask, DL
        MOV      [EBX].fSetDIBPixels, EAX
        TEST     EAX, EAX
@@ddb:
        POPAD
        JNZ      @@assigned
        PUSH     Value
        CALL     SetPixels
        JMP      @@exit
@@assigned:
        PUSH     Value
        CALL     [EAX].fSetDIBPixels
@@exit:
end;

procedure TBitmap.FlipVertical;
asm
        PUSH     EBX
        MOV      EBX, EAX
        MOV      ECX, [EBX].fHandle
        JECXZ    @@noHandle

        CALL     StartDC
        PUSH     SrcCopy
        MOV      EDX, [EBX].fHeight
        PUSH     EDX
        MOV      ECX, [EBX].fWidth
        PUSH     ECX
        PUSH     0
        PUSH     0
        PUSH     EAX
        NEG      EDX
        PUSH     EDX
        PUSH     ECX
        NEG      EDX
        DEC      EDX
        PUSH     EDX
        PUSH     0
        PUSH     EAX
        CALL     StretchBlt
        CALL     FinishDC
        POP      EBX
        RET

@@noHandle:
        MOV      ECX, [EBX].fDIBBits
        JECXZ    @@exit

        PUSHAD   //----------------------------------------\
        XOR      EBP, EBP // Y = 0
        //+++++++++++++++++++++++++++ provide fScanLineSize
        MOV      EAX, EBX
        MOV      EDX, EBP
        CALL     GetScanLine //
        SUB      ESP, [EBX].fScanLineSize

@@loo:  LEA      EAX, [EBP*2]
        CMP      EAX, [EBX].fHeight
        JGE      @@finloo

        MOV      EAX, EBX
        MOV      EDX, EBP
        CALL     GetScanLine
        MOV      ESI, EAX // ESI = ScanLine[ Y ]
        MOV      EDX, ESP
        MOV      ECX, [EBX].fScanLineSize
        PUSH     ECX
        CALL     System.Move

        MOV      EAX, EBX
        MOV      EDX, [EBX].fHeight
        SUB      EDX, EBP
        DEC      EDX
        CALL     GetScanLine
        MOV      EDI, EAX
        MOV      EDX, ESI
        POP      ECX
        PUSH     ECX
        CALL     System.Move

        POP      ECX
        MOV      EAX, ESP
        MOV      EDX, EDI
        CALL     System.Move

        INC      EBP
        JMP      @@loo

@@finloo:
        ADD      ESP, [EBX].fScanLineSize
        POPAD
@@exit:
        POP      EBX
end;

procedure TBitmap.FlipHorizontal;
asm
        PUSH     EBX
        MOV      EBX, EAX
        CALL     GetHandle
        TEST     EAX, EAX
        JZ       @@exit

        CALL     StartDC
        PUSH     SrcCopy
        MOV      EDX, [EBX].fHeight
        PUSH     EDX
        MOV      ECX, [EBX].fWidth
        PUSH     ECX
        PUSH     0
        PUSH     0
        PUSH     EAX
        PUSH     EDX
        NEG      ECX
        PUSH     ECX
        PUSH     0
        NEG      ECX
        DEC      ECX
        PUSH     ECX
        PUSH     EAX
        CALL     StretchBlt
        CALL     FinishDC
@@exit:
        POP      EBX
end;

procedure TBitmap.CopyRect(const DstRect: TRect; SrcBmp: PBitmap;
  const SrcRect: TRect);
asm
        PUSHAD
        MOV      EBX, EAX
        MOV      ESI, ECX
        MOV      EDI, EDX
        CALL     GetHandle
        TEST     EAX, EAX
        JZ       @@exit
        MOV      EAX, ESI
        CALL     GetHandle
        TEST     EAX, EAX
        JZ       @@exit
        CALL     StartDC
        XCHG     EBX, ESI
        CMP      EBX, ESI
        JNZ      @@diff1
        PUSH     EAX
        PUSH     0
        JMP      @@nodiff1
@@diff1:
        CALL     StartDC
@@nodiff1:
        PUSH     SrcCopy                  // ->
        MOV      EBP, [SrcRect]
        MOV      EAX, [EBP].TRect.Bottom
        MOV      EDX, [EBP].TRect.Top
        SUB      EAX, EDX
        PUSH     EAX                      // ->
        MOV      EAX, [EBP].TRect.Right
        MOV      ECX, [EBP].TRect.Left
        SUB      EAX, ECX
        PUSH     EAX                      // ->
        PUSH     EDX                      // ->
        PUSH     ECX                      // ->
        PUSH     dword ptr [ESP+24]       // -> DCsrc
        MOV      EAX, [EDI].TRect.Bottom
        MOV      EDX, [EDI].TRect.Top
        SUB      EAX, EDX
        PUSH     EAX                      // ->
        MOV      EAX, [EDI].TRect.Right
        MOV      ECX, [EDI].TRect.Left
        SUB      EAX, ECX
        PUSH     EAX                      // ->
        PUSH     EDX                      // ->
        PUSH     ECX                      // ->
        PUSH     dword ptr [ESP+13*4]     // -> DCdst
        CALL     StretchBlt
        CMP      EBX, ESI
        JNE      @@diff2
        POP      ECX
        POP      ECX
        JMP      @@nodiff2
@@diff2:
        CALL     FinishDC
@@nodiff2:
        CALL     FinishDC
@@exit:
        POPAD
end;

procedure asmIconEmpty( Icon: PIcon );
asm
        CMP      [EAX].TIcon.fHandle, 0
end;

procedure TIcon.Clear;
asm     //cmd    //opd
        XOR      ECX, ECX
        XCHG     ECX, [EAX].fHandle
        JECXZ    @@1
        CMP      [EAX].fShareIcon, 0
        JNZ      @@1
        PUSH     EAX
        PUSH     ECX
        CALL     DestroyIcon
        POP      EAX
@@1:    MOV      [EAX].fShareIcon, 0
end;

{$IFNDEF ICON_DIFF_WH}
function TIcon.Convert2Bitmap(TranColor: TColor): HBitmap;
asm     //cmd    //opd
        PUSH     EBX
        PUSH     ESI
        PUSH     EDI
        PUSH     EBP
        MOV      EBX, EAX
        MOV      EBP, EDX
        XOR      EDX, EDX
        CALL     asmIconEmpty
        JZ       @@ret_0
        PUSH     0
        CALL     GetDC
        PUSH     EAX //> DC0
        PUSH     EAX
        CALL     CreateCompatibleDC
        XCHG     EDI, EAX
        MOV      EDX, [EBX].fSize

        POP      EAX
        PUSH     EAX
        PUSH     EDX //>Bottom
        PUSH     EDX //>Right
        PUSH     0   //>Top
        PUSH     0   //>Left

        PUSH     EDX
        PUSH     EDX
        PUSH     EAX
        CALL     CreateCompatibleBitmap
        XCHG     EBP, EAX

        CALL     Color2RGB
        PUSH     EAX

        PUSH     EBP
        PUSH     EDI
        CALL     SelectObject
        XCHG     ESI, EAX

        CALL     CreateSolidBrush

        MOV      EDX, ESP
        PUSH     EAX
        PUSH     EAX
        PUSH     EDX
        PUSH     EDI
        CALL     Windows.FillRect
        CALL     DeleteObject

        XCHG     EAX, EBX
        MOV      EDX, EDI
        XOR      ECX, ECX
        PUSH     ECX
        CALL     Draw

        PUSH     EDI
        PUSH     ESI
        CALL     FinishDC

        ADD      ESP, 16
        PUSH     0
        CALL     ReleaseDC
        MOV      EDX, EBP

@@ret_0:
        XCHG     EAX, EDX
        POP      EBP
        POP      EDI
        POP      ESI
        POP      EBX
end;
{$ENDIF}

destructor TIcon.Destroy;
asm     //cmd    //opd
        PUSH     EAX
        CALL     Clear
        POP      EAX
        CALL     TObj.Destroy
end;

procedure TIcon.Draw(DC: HDC; X, Y: Integer);
asm     //cmd    //opd
        CALL     asmIconEmpty
        JZ       @@exit
        PUSH     DI_NORMAL
        PUSH     0
        PUSH     0
        {$IFDEF ICON_DIFF_WH}
        PUSH     [EAX].fHeight
        PUSH     [EAX].fWidth
        {$ELSE}
        PUSH     [EAX].fSize
        PUSH     [EAX].fSize
        {$ENDIF}
        PUSH     [EAX].fHandle
        PUSH     Y
        PUSH     ECX
        PUSH     EDX
        CALL     DrawIconEx
@@exit:
end;

procedure TIcon.StretchDraw(DC: HDC; Dest: TRect);
asm     //cmd    //opd
        CALL     asmIconEmpty
        JZ       @@exit
        PUSH     DI_NORMAL
        PUSH     0
        PUSH     0
        PUSH     ECX
        PUSH     ECX
        PUSH     [EAX].fHandle
        PUSH     [ECX].TRect.Top
        PUSH     [ECX].TRect.Left
        PUSH     EDX
        MOV      EAX, [ECX].TRect.Bottom
        SUB      EAX, [ECX].TRect.Top
        MOV      [ESP+20], EAX
        MOV      EAX, [ECX].TRect.Right
        SUB      EAX, [ECX].TRect.Left
        MOV      [ESP+16], EAX
        CALL     DrawIconEx
@@exit:
end;

procedure TIcon.SaveToFile(const FileName: KOLString);
asm     //cmd    //opd
        PUSH     EAX
        MOV      EAX, ESP
        MOV      ECX, EDX
        XOR      EDX, EDX
        CALL     SaveIcons2File
        POP      EAX
end;

procedure TIcon.SaveToStream(Strm: PStream);
asm     //cmd    //opd
        PUSH     EAX
        MOV      EAX, ESP
        MOV      ECX, EDX
        XOR      EDX, EDX
        CALL     SaveIcons2Stream
        POP      EAX
end;

function ColorBits( ColorsCount : Integer ) : Integer;
asm     //cmd    //opd
        PUSH     EBX
        MOV      EDX, offset[PossibleColorBits]
@@loop: MOVZX    ECX, byte ptr [EDX]
        JECXZ    @@e_loop
        INC      EDX
        XOR      EBX, EBX
        INC      EBX
        SHL      EBX, CL
        CMP      EBX, EAX
        JL       @@loop
@@e_loop:
        XCHG     EAX, ECX
        POP      EBX
end;

function WndProcUpdate( Sender: PControl; var Msg: TMsg; var Rslt: Integer ): Boolean;
asm     //cmd    //opd
        PUSH     EBX
        XCHG     EBX, EAX
        MOVZX    EAX, [EBX].TControl.fUpdateCount
        TEST     EAX, EAX
        JZ       @@exit

        XOR      EAX, EAX
        MOV      EDX, [EDX].TMsg.message
        CMP      DX, WM_PAINT
        JNE      @@chk_erasebkgnd

        MOV      [ECX], EAX
        PUSH     EAX
        PUSH     [EBX].TControl.fHandle
        CALL     ValidateRect
        JMP      @@rslt_1
@@chk_erasebkgnd:
        CMP      DX, WM_ERASEBKGND
        JNE      @@exit
        INC      EAX
        MOV      [ECX], EAX
@@rslt_1:
        MOV      AL, 1
@@exit:
        POP      EBX
end;

procedure TControl.SetFocused(const Value: Boolean);
asm
          PUSH  ESI
          MOV   ESI, EAX
          TEST  DL, DL
          JZ    @@1
          {$IFDEF USE_FLAGS}
          TEST  [ESI].fStyle.f2_Style, 1 shl F2_Tabstop
          {$ELSE}
          CMP   [ESI].fTabstop, 0
          {$ENDIF}
          JZ    @@exit
@@1:      {$IFDEF USE_FLAGS}
          TEST  [ESI].fFlagsG3, 1 shl G3_IsControl
          {$ELSE}
          CMP   [ESI].fIsControl, 0
          {$ENDIF}
          JZ    @@SetForegroundWindow
          CALL  TControl.ParentForm
          PUSH  EAX
          MOV   ECX, [EAX].DF.fCurrentControl
          JECXZ @@PF_setCurCtl
          CMP   ECX, ESI
          JZ    @@PF_setCurCtl
          MOV   EAX, [EAX].DF.fCurrentControl
          {$IFDEF EVENTS_DYNAMIC}
          MOV   ECX, [EAX].EV
          MOV   EDX, [ECX].TEvents.fLeave.TMethod.Data
          MOV   ECX, [ECX].TEvents.fLeave.TMethod.Code
          {$ELSE}
          MOV   ECX, [EAX].EV.fLeave.TMethod.Code
          MOV   EDX, [EAX].EV.fLeave.TMethod.Data
          {$ENDIF}
          JECXZ @@SetFocus0
          XCHG  EAX, EDX
          CALL  ECX
          JMP   @@PF_setCurCtl
@@setFocus0:
          PUSH  0
          CALL  Windows.SetFocus
@@PF_setCurCtl:
          POP   EAX
          MOV   [EAX].DF.fCurrentControl, ESI
          {$IFDEF USE_GRAPHCTLS}
          MOV   ECX, [ESI].fSetFocus.TMethod.Code
          MOV   EAX, [ESI].fSetFocus.TMethod.Data
          JECXZ @@SetFocus_GetwindowHandle
          MOV   EDX, ESI
          CALL  ECX
          {$ENDIF}
@@SetFocus_GetwindowHandle:
          XCHG  EAX, ESI
          CALL  TControl.GetWindowHandle
          PUSH  EAX
          CALL  Windows.SetFocus
          JMP   @@exit
@@SetForegroundWindow:
          XCHG  EAX, ESI
          CALL  TControl.GetWindowHandle
          PUSH  EAX
          CALL  SetForegroundWindow
@@exit:   POP   ESI 
end;

procedure TControl.AttachProcEx( Proc: TWindowFunc; ExecuteAfterAppletTerminated: Boolean );
asm     PUSH     EBX
        PUSH     EDI
        PUSH     ECX
        XCHG     EBX, EAX
        MOV      EDI, EDX
        MOV      [EBX].PP.fOnDynHandlers, offset[EnumDynHandlers]
        MOV      EAX, [EBX].fDynHandlers
        MOV      EDX, EDI
        CALL     TList.IndexOf
        TEST     EAX, EAX
        JGE      @@exit

        MOV      EAX, [EBX].fDynHandlers
        PUSH     EAX
        MOV      EDX, EDI
        CALL     TList.Add
        POP      EAX
        POP      EDX
        PUSH     EDX
        CALL     TList.Add
@@exit: {$IFNDEF SMALLEST_CODE}
        MOV      EAX, [EBX].fDynHandlers
        CALL     [Global_AttachProcExtension]
        {$ENDIF}
        POP      ECX
        POP      EDI
        POP      EBX
end;

function TControl.IsProcAttached(Proc: TWindowFunc): Boolean;
asm     //cmd    //opd
        MOV      EAX, [EAX].TControl.fDynHandlers
        CALL     TList.IndexOf
        TEST     EAX, EAX
        SETGE    AL
end;

{$IFDEF nASM_VERSION}
function WndProcAutoPopupMenu( Control: PControl; var Msg: TMsg; var MsgRslt: Integer ): Boolean;
asm
    CMP  WORD PTR[EDX].TMsg.message, WM_CONTEXTMENU
    JNZ  @@ret_0
    CMP  DWORD PTR[EAX].TControl.fAutoPopupMenu, 0
    JZ   @@ret_0
    PUSH ESI
    PUSH EDI
    PUSH EBX
    XCHG ESI, EAX // ESI = Control
    MOV  EDI, EDX

    MOVSX EAX, WORD PTR[EDX].TMsg.lParam+2
    PUSH  EAX  // P.Y
    MOVSX EAX, WORD PTR[EDX].TMsg.lParam
    PUSH  EAX  // P.X

    CMP   DWORD PTR[EDX].TMsg.lParam, -1
    JNZ   @@auto_popup

    MOV   EAX, ESI
    CALL  TControl.GetCurIndex
    CMP   EAX, 0
    JL    @@coords_2screen
    // EAX = I

    MOVZX EBX, WORD PTR[ESI].TControl.fCommandActions.aItem2XY
    CMP   EBX, 0
    JZ    @@coords_2screen

    CMP   BX, EM_POSFROMCHAR
    JNZ   @@chk_LB_LV_TC

    PUSH  1
    MOV   EAX, ESI
    CALL  TControl.GetSelStart
    PUSH  EAX
    MOV   EAX, ESI
    CALL  TControl.GetSelLength
    ADD   DWORD PTR[ESP], EAX
    PUSH  EBX
    PUSH  ESI
    CALL  TControl.Perform
    MOVSX EBX, AX
    SHR   EAX, 16
    MOVSX EAX, AX
    POP   ECX
    POP   ECX
    PUSH  EAX
    PUSH  EBX
    JMP   @@check_bounds

@@chk_LB_LV_TC:
    CMP   BX, LB_GETITEMRECT
    JZ    @@LB_LV_TC
    CMP   BX, LVM_GETITEMRECT
    JZ    @@LB_LV_TC
    CMP   BX, TCM_GETITEMRECT
    JNZ   @@chk_TVM
@@LB_LV_TC: // EAX = I
    PUSH  ECX
    PUSH  LVIR_BOUNDS
    PUSH  ESP // @R
    PUSH  EAX // I
    JMP   @@get_2

@@chk_TVM:
    CMP   BX, TVM_GETITEMRECT
    JNZ   @@check_bounds

    MOV   EDX, TVGN_CARET
    MOV   EAX, ESI
    CALL  TControl.TVGetItemIdx
    PUSH  ECX
    PUSH  EAX
    PUSH  ESP // @R
    PUSH  1   // 1
@@get_2:
    PUSH  EBX // M
    PUSH  ESI // Control
    CALL  TControl.Perform
    POP   EAX
    POP   ECX
    POP   ECX
    PUSH  EAX

@@check_bounds:
    POP   EBX // P.X
    POP   EDI // P.Y
    SUB   ESP, 16
    MOV   EDX, ESP
    MOV   EAX, ESI
    CALL  TControl.ClientRect

    POP   EAX // R.Left == 0
    POP   EAX // R.Top  == 0
    POP   EAX // R.Right
    CMP   EBX, EAX
    JLE   @@1
    XCHG  EBX, EAX
@@1:POP   EAX // R.Bottom
    CMP   EDI, EAX
    JLE   @@2
    XCHG  EDI, EAX
@@2:PUSH  EDI // P.Y
    PUSH  EBX // P.X

@@coords_2screen:
    MOV  EDX, ESP
    MOV  EAX, ESI
    MOV  ECX, EDX
    CALL TControl.Client2Screen

@@auto_popup:
    POP  EDX  // P.X
    POP  ECX  // P.Y
    MOV  EAX, [ESI].TControl.fAutoPopupMenu
    CALL TMenu.Popup

    POP  EBX
    POP  EDI
    POP  ESI
    OR   EAX, -1
    RET
@@ret_0:
    XOR  EAX, EAX
end;
{$ENDIF nASM_VERSION}

function WndProcMouseEnterLeave( Self_: PControl; var Msg: TMsg; var Rslt: Integer ): Boolean;
asm
    PUSH ESI
    XCHG ESI, EAX

    MOV  AX, word ptr [EDX].TMsg.message
    CMP  AX, WM_MOUSELEAVE
    JE   @@MOUSELEAVE
    SUB  AX, WM_MOUSEFIRST
    CMP  AX, WM_MOUSELEAVE-WM_MOUSEFIRST
    JA   @@retFalse

    {$IFDEF USE_FLAGS}
    TEST [ESI].TControl.fFlagsG3, 1 shl G3_MouseInCtl
    SETNZ AL
    {$ELSE}
    MOV  AL, [ESI].TControl.fMouseInControl
    {$ENDIF}
    PUSH EAX
    {$IFDEF EVENTS_DYNAMIC}
    MOV  EAX, [ESI].TControl.EV
    MOV  ECX, [EAX].TEvents.fOnTestMouseOver.TMethod.Code
    {$ELSE}
    MOV  ECX, [ESI].TControl.EV.fOnTestMouseOver.TMethod.Code
    {$ENDIF}
    JECXZ     @@1
    {$IFDEF  EVENTS_DYNAMIC}
    MOV  EAX, [EAX].TEvents.fOnTestMouseOver.TMethod.Data
    {$ELSE}
    MOV  EAX, [ESI].TControl.EV.fOnTestMouseOver.TMethod.Data
    {$ENDIF}
    MOV  EDX, ESI
    CALL ECX
    JMP  @@2
@@1:
    PUSH ECX
    PUSH ECX
    PUSH ESP
    CALL GetCursorPos
    MOV  EAX, ESI
    MOV  EDX, ESP
    MOV  ECX, EDX
    CALL TControl.Screen2Client
    MOV  ECX, ESP  // @P
    SUB  ESP, 16
    MOV  EDX, ESP  // @ClientRect
    MOV  EAX, ESI

    PUSH EDX
    PUSH ECX
    CALL TControl.ClientRect
    POP  EAX
    POP  EDX
    CALL PointInRect
    ADD  ESP, 16+8

@@2:
    POP  EDX
    CMP  AL, DL
    JE   @@retFalse

    //MouseWasInControl <> Yes
    PUSH EAX
    MOV  EAX, ESI
    CALL TControl.Invalidate
    POP  EAX

    TEST AL, AL
    JZ   @@3

    {$IFDEF USE_FLAGS}
    OR   [ESI].TControl.fFlagsG3, 1 shl G3_MouseInCtl
    {$ELSE}
    MOV  [ESI].TControl.fMouseInControl, 1
    {$ENDIF}
    {$IFDEF EVENTS_DYNAMIC}
    MOV  EAX, [ESI].TControl.EV
    MOV  ECX, [EAX].TEvents.fOnMouseEnter.TMethod.Code
    {$ELSE}
    MOV  ECX, [ESI].TControl.EV.fOnMouseEnter.TMethod.Code
    {$ENDIF}
    JECXZ @@2_1
    {$IFDEF EVENTS_DYNAMIC}
    MOV  EAX, [EAX].TEvents.fOnMouseEnter.TMethod.Data
    {$ELSE}
    MOV  EAX, [ESI].TControl.EV.fOnMouseEnter.TMethod.Data
    {$ENDIF}
    MOV  EDX, ESI
    CALL ECX
@@2_1:
    PUSH ECX
    PUSH [ESI].TControl.fHandle
    PUSH TME_LEAVE
    PUSH 16
    MOV  EAX, ESP
    CALL DoTrackMouseEvent
    JMP  @@4

@@3:
    {$IFDEF USE_FLAGS}
    AND  byte ptr [ESI].TControl.fFlagsG3, $7F // not(1 shl G3_MouseInCtl)
    {$ELSE}
    MOV  [ESI].TControl.fMouseInControl, 0
    {$ENDIF}
    PUSH ECX
    PUSH [ESI].TControl.fHandle
    PUSH TME_LEAVE or TME_CANCEL
    PUSH 16
    MOV  EAX, ESP
    CALL DoTrackMouseEvent

@@3_X:
    {$IFDEF EVENTS_DYNAMIC}
    MOV  EAX, [ESI].TControl.EV
    MOV  ECX, [EAX].TEvents.fOnMouseLeave.TMethod.Code
    {$ELSE}
    MOV  ECX, [ESI].TControl.EV.fOnMouseLeave.TMethod.Code
    {$ENDIF}
    JECXZ @@3_1
    {$IFDEF  EVENTS_DYNAMIC}
    MOV  EAX, [EAX].TEvents.fOnMouseLeave.TMethod.Data
    {$ELSE}
    MOV  EAX, [ESI].TControl.EV.fOnMouseLeave.TMethod.Data
    {$ENDIF}
    MOV  EDX, ESI
    CALL ECX
@@3_1:

@@4:
    ADD  ESP, 16
@@4_1:
    MOV  EAX, ESI
    CALL TControl.Invalidate
    JMP  @@retFalse

@@MOUSELEAVE:
    {$IFDEF USE_FLAGS}
    BTR  dword ptr [ESI].TControl.fFlagsG3, G3_MouseInCtl
    JNC  @@retFalse
    {$ELSE}
    BTR  DWORD PTR [ESI].TControl.fMouseInControl, 0
    JNC  @@retFalse
    {$ENDIF}

    {$IFDEF GRAPHCTL_HOTTRACK}
        {$IFDEF EVENTS_DYNAMIC}
        MOV  EAX, [ESI].TControl.EV
        MOV  ECX, [EAX].TEvents.fMouseLeaveProc.TMethod.Code
        {$ELSE}
        MOV  ECX, [ESI].TControl.EV.fMouseLeaveProc.TMethod.Code
        {$ENDIF}
        {$IFDEF NIL_EVENTS}
        JECXZ @@4_1
        {$ENDIF}
        {$IFDEF EVENTS_DYNAMIC}
        MOV  EAX, [EAX].TEvents.fMouseLeaveProc.TMethod.Data
        {$ELSE}
        MOV  EAX, [ESI].TControl.EV.fMouseLeaveProc.TMethod.Data
        {$ENDIF}
        CALL ECX
    {$ENDIF}

    SUB  ESP, 16
    JMP  @@3_X

@@retFalse:
    XOR  EAX, EAX
    POP  ESI
end;

function TControl.GetToBeVisible: Boolean;
asm
    {$IFDEF USE_FLAGS}
        TEST    [EAX].TControl.fStyle.f3_Style, 1 shl F3_Visible
        SETNZ   DH
        TEST    [EAX].TControl.fFlagsG4, (1 shl G4_CreateHidden) or (1 shl G4_VisibleWOParent)
        SETNZ   DL
        OR      DL, DH
        TEST    [EAX].TControl.fFlagsG3, 1 shl G3_IsControl
        JZ      @@retDL
        MOV     ECX, [EAX].TControl.fParent
        JECXZ   @@retDL

        {$IFDEF OLD_ALIGN}
        TEST    [EAX].TControl.fFlagsG4, 1 shl G4_VisibleWOParent
        JZ      @@1
        MOV     DL, DH
        JMP     @@retDL
        {$ENDIF}

    {$ELSE not USE_FLAGS}
        MOV     DH, [EAX].TControl.fVisible
        MOV     DL, [EAX].TControl.fCreateHidden
        OR      DL, DH
        OR      DL, [EAX].TControl.fVisibleWoParent
        CMP     [EAX].TControl.fIsControl, 0
        JZ      @@retDL
        MOV     ECX, [EAX].TControl.fParent
        JECXZ   @@retDL

        {$IFDEF OLD_ALIGN}
        CMP     [EAX].TControl.fVisibleWoParent, 0
        JZ      @@1
        MOV     DL, DH
        JMP     @@retDL
        {$ENDIF}

    {$ENDIF}

@@1:
    TEST    DL, DL
    JZ      @@retDL
    XCHG    EAX, ECX
    PUSH    EAX
    CALL    TControl.Get_Visible
    POP     EAX
    CALL    TControl.GetToBeVisible
    XCHG    EDX, EAX
@@retDL:
    XCHG    EAX, EDX
end;

// by MTsv DN - v2.90  --  chg by VK
function WinVer : TWindowsVersion;
asm
 MOVSX EAX, byte ptr [SaveWinVer]
 INC   AH     // если <> 0 после инкремента, то AL содержит вычисленную версию
 JNZ   @@exit
 CALL  GetVersion // EAX < 0 для платформы 9х, иначе NT; AL=MajorVersion; AH=MinorVersion
 XCHG  EDX, EAX
 XOR   EAX, EAX
 TEST  EDX, EDX
 XCHG  DL, DH    // DH=MajorVersion; DL=MinorVersion

 JL    @@platform_9x
 MOV   AL, wvNT
 CMP   DX, $0400
 JZ    @@save_exit

 INC   AL // wvY2K
 SUB   DX, $0500
 JZ    @@save_exit

 INC   AL // wvXP
 //CMP   DX, $0501
 DEC   DX
 JZ    @@save_exit

 INC   AL // wvWin2003Server
 //CMP   DX, $0502
 DEC   DX
 JZ    @@save_exit

 INC   AL // wvVista
 CMP   DX, $0600 - $0502
 JZ    @@save_exit

 INC   AL // wvSeven
 //CMP   DX, $0601
 //DEC   DX
 JMP   @@save_exit
@@platform_9x:
 CMP   DH, 4
 JB    @@save_exit // wv31
 INC   AL // wv95
 CMP   DX, $040A
 JB    @@save_exit
 INC   AL // wv98
 CMP   DX, $045A
 JB    @@save_exit
 INC   AL // wvME
@@save_exit:
 MOV   byte ptr [SaveWinVer], AL
@@exit:
end;

{$IFDEF USE_CONSTRUCTORS}
constructor TControl.CreateGradientPanel(AParent: PControl; AColor1,            //
  AColor2: TColor);                                                             //
asm     //cmd    //opd                                                          //
        XOR      EDX, EDX                                                       //
        PUSH     EDX                                                            //
        CALL     CreateLabel                                                    //
        MOV      ECX, AColor1                                                   //
        MOV      [EAX].fColor1, ECX                                             //
        MOV      ECX, AColor2                                                   //
        MOV      [EAX].fColor2, ECX                                             //
        MOV      EDX, [EAX].fBoundsRect.Left                                    //
        ADD      EDX, 40                                                        //
        MOV      [EAX].fBoundsRect.Right, EDX                                   //
        MOV      EDX, [EAX].fBoundsRect.Top                                     //
        ADD      EDX, 40                                                        //
        MOV      [EAX].fBoundsRect.Bottom, EDX                                  //
        PUSH     EAX                                                            //
        MOV      EDX, offset[ WndProcGradient ]                                 //
        CALL     AttachProc                                                     //
        POP      EAX                                                            //
end;                                                                            //
{$ENDIF USE_CONSTRUCTORS}

function TControl.MakeWordWrap: PControl;
asm
    {$IFDEF USE_FLAGS}
    OR   [EAX].TControl.fFlagsG1, (1 shl G1_WordWrap)
    {$ELSE}
    MOV  [EAX].TControl.fWordWrap, 1
    {$ENDIF}

    MOV  EDX, [EAX].TControl.fStyle
    {$IFDEF USE_FLAGS}
    TEST [EAX].TControl.fFlagsG5, 1 shl G5_IsButton
    {$ELSE}
    CMP  [EAX].TControl.fIsButton, 0
    {$ENDIF}
    JNZ   @@1
    AND  DL, not SS_LEFTNOWORDWRAP
@@1:
    OR   DH, $20 or SS_LEFTNOWORDWRAP // BS_MULTILINE >> 8
@@2:
    PUSH EAX
    CALL TControl.SetStyle
    POP  EAX
end;

function TControl.FormGetIntParam: Integer;
asm
    PUSH ESI
    PUSH EDI
    MOV  EDI, EAX // EDX = @ Self

    XOR  EDX, EDX
@@loop:

    LEA  ECX, [EDI].DF.FormParams
    MOV  ESI, DWORD PTR[ECX]
    LODSB
    MOV  DWORD PTR[ECX], ESI

    SHR  AL, 1
    JNC  @@nocont

    SHL  EDX, 7
    OR   DL, AL
    JMP  @@loop

@@nocont:

    SHR  AL, 1
    PUSHF
    XCHG EDX, EAX
    SHL  EAX, 6
    OR   AL, DL
    POPF
    JNC  @@noneg

    NEG  EAX
@@noneg:
    POP  EDI
    POP  ESI
end;

function TControl.FormGetColorParam: Integer;
asm
    CALL FormGetIntParam
    ROR  EAX, 1
end;

procedure TControl.FormGetStrParam;
asm
    PUSH EDI
        MOV  EDI, EAX
        CALL FormGetIntParam
        XCHG ECX, EAX
        LEA  EAX, [EDI].FormString
        PUSH ECX
            MOV  EDX, DWORD PTR[EDI].DF.FormParams
            {$IFDEF _D2}
            CALL System.@LStrFromLenStr
            {$ELSE}
            CALL System.@LStrFromPCharLen
            {$ENDIF}
        POP  ECX
        ADD  DWORD PTR[EDI].DF.FormParams, ECX
    POP  EDI
end;

procedure TControl.FormExecuteCommands(AForm: PControl; ControlPtrOffsets: PSmallIntArray);
asm
    PUSH  EBX
    PUSH  ESI
    PUSH  EDI
    XCHG  EDI, EAX // EDI = @ Self
    MOV   EBX, EDX // EBX = AForm
    MOV   ESI, ECX // ECX = @ ControlPtrOffsets[0]
@@while_do:
    MOV   EAX, EDI
    CALL  FormGetIntParam
    TEST  EAX, EAX
    JZ    @@ewhile
    JG    @@not_create_ctrl

    NEG   EAX
    MOV   ECX, [EDI].DF.FormAlphabet
    MOV   ECX, [ECX+EAX*4-4]

    MOV   EAX, EDI

    CALL  ECX
    XCHG  ECX, EAX

    XOR   EAX, EAX
    LODSW
    MOV   DWORD PTR[EBX+EAX*4], ECX
    MOV   [EDI].DF.FormLastCreatedChild, ECX
    JMP   @@while_do

@@not_create_ctrl:
    MOV   ECX, [EDI].DF.FormAlphabet
    MOV   ECX, [ECX+EAX*4-4]
    MOV   EAX, [EDI].DF.FormLastCreatedChild

    XOR   EDX, EDX
    INC   EDX

    CALL  ECX
    JMP   @@while_do

@@ewhile:
    LEA   EAX, [EDI].FormString
    CALL  System.@LStrClr

    POP   EDI
    POP   ESI
    POP   EBX
end;

function FormNewLabel( Form: PControl ): PControl;
asm
    CALL FormPrepareStrParamCreateCtrl
    CALL NewLabel
end;

function FormNewWordWrapLabel( Form: PControl ): PControl;
asm
    CALL FormPrepareStrParamCreateCtrl
    CALL NewWordWrapLabel
end;

function FormNewLabelEffect( Form: PControl ): PControl;
asm
    PUSH EAX
    CALL TControl.FormGetStrParam
    POP  EAX
    PUSH EAX
    CALL TControl.FormGetIntParam
    POP  ECX
    PUSH EAX
    MOV  EAX, [ECX].TControl.DF.FormCurrentParent
    MOV  EDX, [ECX].TControl.FormString
    POP  ECX
    CALL NewLabelEffect
end;

function FormNewButton( Form: PControl ): PControl;
asm
    CALL FormPrepareStrParamCreateCtrl
    CALL NewButton
end;

function FormNewPanel( Form: PControl ): PControl;
asm
    CALL FormPrepareIntParamCreateCtrl
    CALL NewPanel
end;

function FormNewGroupbox( Form: PControl ): PControl;
asm
    CALL FormPrepareStrParamCreateCtrl
    CALL NewGroupbox
end;

function FormNewEditBox( Form: PControl ): PControl;
asm
    CALL FormPrepareIntParamCreateCtrl
    CALL NewEditBox
end;

{$IFDEF USE_RICHEDIT}
function FormNewRichEdit( Form: PControl ): PControl;
asm CALL FormPrepareIntParamCreateCtrl
    CALL NewRichEdit
end;
{$ENDIF USE_RICHEDIT}

function FormNewComboBox( Form: PControl ): PControl;
asm
    CALL FormPrepareIntParamCreateCtrl
    CALL NewCombobox
end;

function FormNewCheckbox( Form: PControl ): PControl;
asm
    CALL FormPrepareStrParamCreateCtrl
    CALL NewCheckbox
end;

function FormNewRadiobox( Form: PControl ): PControl;
asm
    CALL FormPrepareStrParamCreateCtrl
    CALL NewRadiobox
end;

function FormNewListbox( Form: PControl ): PControl;
asm
    CALL FormPrepareIntParamCreateCtrl
    CALL NewListbox
end;

//!!! asm version returns in EAX Control,
//    and integer parameter in EDX and ECX (EDX=ECX) !!!
//--- this is enough to call method of Control with a single int param ---
function ParentForm_IntParamAsm(Control: PControl): Integer;
asm PUSH EAX
    CALL TControl.FormParentForm
    CALL TControl.FormGetIntParam
    XCHG EDX, EAX
    MOV  ECX, EDX
    POP  EAX
end;
function ParentForm_ColorParamAsm(Control: PControl): Integer;
asm CALL ParentForm_IntParamAsm
    ROR  EDX, 1
end;

procedure FormSetSize( Form: PControl );
asm
    CALL ParentForm_IntParamAsm
    PUSH EDX
    CALL ParentForm_IntParamAsm
    //XCHG ECX, EDX
    POP  EDX
    CALL TControl.SetSize
end;

function ParentForm_PCharParamAsm(Control: PControl): PChar;
asm PUSH EAX
    CALL ParentForm_PCharParam
    XCHG EDX, EAX
    MOV  ECX, EDX
    POP  EAX
end;

procedure FormSetPosition( Form: PControl );
asm
    CALL ParentForm_IntParamAsm
    PUSH EDX
    CALL ParentForm_IntParamAsm
    POP  EDX
    CALL TControl.SetPosition
end;

procedure FormSetClientSize( Form: PControl );
asm
    CALL ParentForm_IntParamAsm
    PUSH EDX
    CALL ParentForm_IntParamAsm
    //XCHG ECX, EDX
    POP  EDX
    CALL TControl.SetClientSize
end;

procedure FormSetAlign( Form: PControl );
asm
    CALL ParentForm_IntParamAsm
    CALL TControl.SetAlign
end;

procedure FormSetCanResizeFalse( Form: PControl );
asm
    XOR  EDX, EDX
    CALL TControl.SetCanResize
end;

procedure FormInitMenu( Form: PControl );
asm
    PUSH 0
    PUSH 0
    PUSH WM_INITMENU
    PUSH EAX
    CALL TControl.Perform
end;

procedure FormSetExStyle( Form: PControl );
asm
    CALL ParentForm_IntParamAsm
    OR   EDX, [EAX].TControl.fExStyle
    CALL TControl.SetExStyle
end;

procedure FormSetVisibleFalse( Form: PControl );
asm
    XOR  EDX, EDX
    CALL TControl.SetVisible
end;

procedure FormSetEnabledFalse( Form: PControl );
asm
    XOR  EDX, EDX
    CALL TControl.SetEnabled
end;

procedure FormResetStyles( Form: PControl );
asm
    CALL ParentForm_IntParamAsm
    NOT  EDX
    AND  EDX, [EAX].TControl.fStyle
    CALL TControl.SetStyle
end;

procedure FormSetStyle( Form: PControl );
asm
    CALL ParentForm_IntParamAsm
    OR   EDX, [EAX].TControl.fStyle
    CALL TControl.SetStyle
end;

procedure FormSetAlphaBlend( Form: PControl );
asm
    CALL ParentForm_IntParamAsm
    CALL TControl.SetAlphaBlend
end;

procedure FormSetHasBorderFalse( Form: PControl );
asm
    XOR  EDX, EDX
    CALL TControl.SetHasBorder
end;

procedure FormSetHasCaptionFalse( Form: PControl );
asm
    XOR  EDX, EDX
    CALL TControl.SetHasCaption
end;

procedure FormResetCtl3D( Form: PControl );
asm
    XOR  EDX, EDX
    CALL TControl.SetCtl3D
end;

procedure FormIconLoad_hInstance( Form: PControl );
asm
    CALL ParentForm_IntParamAsm
    MOV  EDX, [hInstance]
    CALL TControl.IconLoad
end;

procedure FormIconLoadCursor_0( Form: PControl );
asm
    CALL ParentForm_IntParamAsm
    XOR  EDX, EDX
    CALL TControl.IconLoadCursor
end;

procedure FormSetIconNeg1( Form: PControl );
asm
    OR   EDX, -1
    CALL TControl.SetIcon
end;

procedure FormSetWindowState( Form: PControl );
asm
    CALL ParentForm_IntParamAsm
    CALL TControl.SetWindowState
end;

procedure FormCursorLoad_0( Form: PControl );
asm
    CALL ParentForm_IntParamAsm
    XOR  EDX, EDX
    CALL TControl.CursorLoad
end;

procedure FormSetColor( Form: PControl );
asm
    CALL ParentForm_ColorParamAsm
    CALL TControl.SetCtlColor
end;

procedure FormSetBrushStyle( Form: PControl );
asm
    CALL ParentForm_IntParamAsm
    PUSH EDX
    CALL TControl.GetBrush
    POP  EDX
    CALL TGraphicTool.SetBrushStyle
end;

procedure FormSetBrushBitmap( Form: PControl );
asm
    PUSH EDI
    MOV  EDI, EAX
    CALL TControl.FormParentForm

    PUSH EAX
    CALL ParentForm_PCharParam
    XCHG EDX, EAX
    MOV  EAX, [hInstance]
    POP  ECX

    CALL LoadBmp

    PUSH EAX
    MOV  EAX, EDI
    CALL TControl.GetBrush
    POP  EDX

    CALL TGraphicTool.SetBrushBitmap
    POP  EDI
end;

procedure FormSetFontColor( Form: PControl );
asm
    CALL ParentForm_ColorParamAsm
    PUSH EDX
    CALL TControl.GetFont
    POP  EDX
    CALL TGraphicTool.SetColor
end;

procedure FormSetFontStyles( Form: PControl );
asm
    CALL ParentForm_IntParamAsm
    PUSH EDX
    CALL TControl.GetFont
    POP  EDX
    CALL TGraphicTool.SetFontStyle
end;

procedure FormSetFontHeight( Form: PControl );
asm
    CALL ParentForm_IntParamAsm
    PUSH EDX
    CALL TControl.GetFont
    XOR  EDX, EDX
    MOV  DL, 4
    POP  ECX
    CALL TGraphicTool.SetInt
end;

procedure FormSetFontWidth( Form: PControl );
asm
    CALL ParentForm_IntParamAsm
    PUSH EDX
    CALL TControl.GetFont
    XOR  EDX, EDX
    MOV  DL, 8
    POP  ECX
    CALL TGraphicTool.SetInt
end;

procedure FormSetFontOrientation( Form: PControl );
asm
    CALL ParentForm_IntParamAsm
    PUSH EDX
    CALL TControl.GetFont
    POP  EDX
    CALL TGraphicTool.SetFontOrientation
end;

procedure FormSetFontCharset( Form: PControl );
asm
    CALL ParentForm_IntParamAsm
    PUSH EDX
    CALL TControl.GetFont
    POP  EDX
    CALL TGraphicTool.SetFontCharset
end;

procedure FormSetFontPitch( Form: PControl );
asm
    CALL ParentForm_IntParamAsm
    PUSH EDX
    CALL TControl.GetFont
    POP  EDX
    CALL TGraphicTool.SetFontPitch
end;

procedure FormSetBorder( Form: PControl );
asm
    CALL ParentForm_IntParamAsm
    MOV  [EAX].TControl.fMargin, DL
end;

procedure FormSetMarginTop( Form: PControl );
asm
    CALL ParentForm_IntParamAsm
    XOR  EDX, EDX
    INC  EDX
    CALL TControl.SetClientMargin
end;

procedure FormSetMarginBottom( Form: PControl );
asm
    CALL ParentForm_IntParamAsm
    XOR  EDX, EDX
    MOV  DL, 2
    CALL TControl.SetClientMargin
end;

procedure FormSetMarginLeft( Form: PControl );
asm
    CALL ParentForm_IntParamAsm
    XOR  EDX, EDX
    MOV  DL, 3
    CALL TControl.SetClientMargin
end;

procedure FormSetMarginRight( Form: PControl );
asm
    CALL ParentForm_IntParamAsm
    XOR  EDX, EDX
    MOV  DL, 4
    CALL TControl.SetClientMargin
end;

procedure FormSetSimpleStatusText( Form: PControl );
asm
    CALL ParentForm_PCharParamAsm
    XOR  EDX, EDX
    MOV  DL, 255
    CALL TControl.SetStatusText
end;

procedure FormSetStatusText( Form: PControl );
asm
    CALL ParentForm_IntParamAsm
    PUSH EDX
    CALL ParentForm_PCharParamAsm
    POP  EDX
    CALL TControl.SetStatusText
end;

procedure FormRemoveCloseIcon( Form: PControl );
asm
    PUSH MF_BYCOMMAND
    PUSH SC_CLOSE
    CALL TControl.GetWindowHandle
    PUSH 0
    PUSH EAX
    CALL GetSystemMenu
    PUSH EAX
    CALL DeleteMenu
end;

procedure FormSetConstraint;
asm
    MOVZX EDX, DL
    PUSH  EDX
    CALL ParentForm_IntParamAsm
    POP   EDX
    CALL TControl.SetConstraint
end;

procedure FormSetMinWidth( Form: PControl );
asm
    XOR  EDX, EDX
    CALL FormSetConstraint
end;

procedure FormSetMaxWidth( Form: PControl );
asm
    MOV  DL, 2
    CALL FormSetConstraint
end;

procedure FormSetMinHeight( Form: PControl );
asm
    MOV  DL, 1
    CALL FormSetConstraint
end;

procedure FormSetMaxHeight( Form: PControl );
asm
    MOV  DL, 3
    CALL FormSetConstraint
end;

procedure FormSetTextShiftX( Form: PControl );
asm
    CALL ParentForm_IntParamAsm
    MOV  [EAX].TControl.DF.fTextShiftX, EDX
end;

procedure FormSetTextShiftY( Form: PControl );
asm
    CALL ParentForm_IntParamAsm
    MOV  [EAX].TControl.DF.fTextShiftY, EDX
end;

procedure FormSetColor2( Form: PControl );
asm
    CALL ParentForm_ColorParamAsm
    CALL TControl.SetColor2
end;

procedure FormSetTextAlign( Form: PControl );
asm
    CALL ParentForm_IntParamAsm
    CALL TControl.SetTextAlign
end;

procedure FormSetTextVAlign( Form: PControl );
asm
    CALL ParentForm_IntParamAsm
    CALL TControl.SetVerticalAlign
end;

procedure FormSetIgnoreDefault( Form: PControl );
asm
    CALL ParentForm_IntParamAsm
    {$IFDEF USE_FLAGS}
    SHL  EDX, G5_IgnoreDefault
    AND  [EAX].TControl.fFlagsG5, $7F //not(1 shl G5_IgnoreDefault)
    OR   [EAX].TControl.fFlagsG5, DL
    {$ELSE}
    MOV  [EAX].TControl.FIgnoreDefault, DL
    {$ENDIF}
end;

procedure FormSetCaption( Form: PControl );
asm
    PUSH EAX
    CALL TControl.FormParentForm
    PUSH EAX
    CALL TControl.FormGetStrParam
    POP  EAX
    MOV  EDX, [EAX].TControl.FormString
    POP  EAX
    CALL TControl.SetCaption
end;

procedure FormSetGradienStyle( Form: PControl );
asm
    CALL ParentForm_IntParamAsm
    CALL TControl.SetGradientStyle
end;

{$IFDEF USE_RICHEDIT}
procedure FormSetRE_AutoFontFalse( Form: PControl );
asm
    XOR  EDX, EDX
    MOV  DL, 4
    XOR  ECX, ECX
    CALL TControl.RESetLangOptions
end;

procedure FormSetRE_AutoFontSizeAdjustFalse( Form: PControl );
asm
    XOR  EDX, EDX
    MOV  DL, 16
    XOR  ECX, ECX
    CALL TControl.RESetLangOptions
end;

procedure FormSetRE_DualFontTrue( Form: PControl );
asm
    XOR  EDX, EDX
    MOV  DL, 128
    MOV  CL, 1
    CALL TControl.RESetLangOptions
end;

procedure FormSetRE_UIFontsTrue( Form: PControl );
asm
    XOR  EDX, EDX
    MOV  DL, 32
    MOV  CL, 1
    CALL TControl.RESetLangOptions
end;

procedure FormSetRE_IMECancelCompleteTrue( Form: PControl );
asm
    XOR  EDX, EDX
    MOV  DL, 4
    MOV  CL, 1
    CALL TControl.RESetLangOptions
end;

procedure FormSetRE_IMEAlwaysSendNotifyTrue( Form: PControl );
asm
    XOR  EDX, EDX
    MOV  DL, 8
    MOV  CL, 1
    CALL TControl.RESetLangOptions
end;

procedure FormSetMaxTextSize( Form: PControl );
asm
    CALL ParentForm_IntParamAsm
    CALL TControl.SetMaxTextSize
end;

procedure FormSetRE_AutoKeyboardTrue( Form: PControl );
asm
    XOR  EDX, EDX
    MOV  DL, 1
    MOV  CL, 1
    CALL TControl.RESetLangOptions
end;

procedure FormSetRE_Zoom( Form: PControl );
asm
    CALL ParentForm_IntParamAsm
    PUSH EDX
    CALL ParentForm_IntParamAsm
    POP  EDX
    SHL  ECX, 16
    OR   EDX, ECX
    CALL TControl.ReSetZoom
end;
{$ENDIF USE_RICHEDIT}

procedure FormSetCount( Form: PControl );
asm
    CALL ParentForm_IntParamAsm
    CALL TControl.SetItemsCount
end;

procedure FormSetDroppedWidth( Form: PControl );
asm
    CALL ParentForm_IntParamAsm
    CALL TControl.SetDroppedWidth
end;

procedure FormSetButtonImage( Form: PControl );
asm
    PUSH EDI
    MOV  EDI, EAX
    CALL ParentForm_IntParamAsm
    PUSH ECX
    CALL ParentForm_IntParamAsm
    POP  ECX
    PUSH $8000 // LR_SHARED
    PUSH ECX
    PUSH EDX
    PUSH IMAGE_ICON
    CALL ParentForm_PCharParam
    PUSH EAX
    PUSH [hInstance]
    CALL LoadImage
    XCHG EDX, EAX
    XCHG EAX, EDI
    CALL TControl.SetButtonIcon
    POP  EDI
end;

procedure FormSetButtonBitmap( Form: PControl );
asm
    PUSH EAX
    CALL ParentForm_PCharParam
    PUSH EAX
    PUSH [hInstance]
    CALL LoadBitmap
    XCHG EDX, EAX
    POP  EAX
    CALL TControl.SetButtonBitmap
end;

procedure FormSetMaxProgress( Form: PControl );
asm
    CALL ParentForm_IntParamAsm
    MOV  EDX, (PBM_SETRANGE32 or $8000) shl 16
    CALL TControl.SetMaxProgress
end;

procedure FormSetProgress( Form: PControl );
asm
    CALL ParentForm_IntParamAsm
    MOV  EDX, (PBM_SETPOS or $8000) shl 16
    CALL TControl.SetIntVal
end;

procedure FormLVColumsAdd( Form: PControl );
asm
    PUSH EDI
    MOV  EDI, EAX
    CALL ParentForm_IntParamAsm
    JECXZ @@fin
@@1:
    PUSH ECX
    MOV  EAX, EDI
    CALL ParentForm_IntParamAsm
    PUSH ECX
    CALL ParentForm_StrParam
    MOV  EAX, EDI
    CALL TControl.FormParentForm
    MOV  EDX, [EAX].TControl.FormString
    XOR  ECX, ECX
    MOV  CL, taLeft
    MOV  EAX, EDI
    CALL TControl.LVColAdd
    POP  ECX
    LOOP @@1
@@fin:
    POP  EDI
end;

procedure FormSetLVColOrder( Form: PControl );
asm
    CALL ParentForm_IntParamAsm
    PUSH EDX
    CALL ParentForm_IntParamAsm
    POP  EDX
    PUSH ECX
    MOV  ECX, LVCF_ORDER or (28 shl 16)
    CALL TControl.SetLVColEx
end;

procedure FormSetLVColImage( Form: PControl );
asm
    CALL ParentForm_IntParamAsm
    PUSH EDX
    CALL ParentForm_IntParamAsm
    POP  EDX
    PUSH ECX
    MOV  ECX, LVCF_IMAGE or (24 shl 16)
    CALL TControl.SetLVColEx
end;

procedure FormSetTVIndent( Form: PControl );
asm
    CALL ParentForm_IntParamAsm
    MOV  EDX, TVM_GETINDENT
    CALL TControl.SetIntVal
end;

procedure FormSetDateTimeFormat( Form: PControl );
asm
    PUSH EAX
    CALL TControl.FormParentForm
    PUSH EAX
    CALL TControl.FormGetStrParam
    POP  EAX
    MOV  EDX, [EAX].TControl.FormString
    POP  EAX
    CALL TControl.SetDateTimeFormat
end;

procedure FormSetCurrentTab( Form: PControl );
asm
    CALL ParentForm_IntParamAsm
    PUSHAD
    CALL TControl.SetCurIndex
    POPAD
    CALL TControl.GetPages
    CALL TControl.BringToFront
end;

procedure FormSetCurIdx( Form: PControl );
asm
    CALL ParentForm_IntParamAsm
    CALL TControl.SetCurIndex
end;

procedure FormSetSBMin( Form: PControl );
asm
    CALL ParentForm_IntParamAsm
    CALL TControl.SetSBMin
end;

procedure FormSetSBMax( Form: PControl );
asm
    CALL ParentForm_IntParamAsm
    CALL TControl.SetSBMax
end;

procedure FormSetSBPosition( Form: PControl );
asm
    CALL ParentForm_IntParamAsm
    CALL TControl.SetSBPosition
end;

procedure FormSetSBPageSize( Form: PControl );
asm
    CALL ParentForm_IntParamAsm
    CALL TControl.SetSBPageSize
end;

procedure FormLastCreatedChildAsNewCurrentParent( Form: PControl );
asm
    PUSH EAX
    CALL TControl.FormParentForm
    POP  [EAX].TControl.DF.FormCurrentParent
end;

procedure FormSetTabpageAsParent( Form: PControl );
asm
    PUSH EAX
    CALL TControl.FormParentForm
    CALL ParentForm_IntParamAsm
    POP  ECX
    PUSH EAX
    XCHG EAX, ECX
    CALL TControl.GetPages
    POP  EDX
    MOV  [EDX].TControl.DF.FormCurrentParent, EAX
    MOV  [EDX].TControl.DF.FormLastCreatedChild, EAX
end;

procedure FormSetCurCtl( Form: PControl );
asm
    CALL TControl.FormParentForm
    CALL ParentForm_IntParamAsm
    MOV  ECX, [EAX].TControl.DF.FormAddress
    MOV  ECX, [ECX + EDX*4]

    TEST ECX, ECX
    JNZ  @@1
    MOV  ECX, EAX

@@1:
    MOV  [EAX].TControl.DF.FormLastCreatedChild, ECX
end;

procedure FormSetEvent( Form: PControl );
asm
    PUSH  EDI
    MOV   EDI, EAX
    PUSH  ESI
    CALL  TControl.FormParentForm
    MOV   ESI, EAX
    PUSH  [ESI].TControl.DF.FormObj
    CALL  ParentForm_IntParamAsm
    MOV   ESI, [EAX].TControl.DF.FormAlphabet
    PUSH  dword ptr [ESI+EDX*4]
    CALL  ParentForm_IntParamAsm
    XCHG  EAX, EDI
    CALL  dword ptr [ESI+EDX*4]
    POP   ESI
    POP   EDI
end;

procedure FormSetIndexedEvent( Form: PControl );
asm
    PUSH  EDI
    MOV   EDI, EAX
    PUSH  ESI
    CALL  TControl.FormParentForm
    MOV   ESI, EAX
    PUSH  [ESI].TControl.DF.FormObj
    CALL  ParentForm_IntParamAsm
    MOV   ESI, [EAX].TControl.DF.FormAlphabet
    PUSH  dword ptr [ESI+EDX*4]

    CALL  ParentForm_IntParamAsm // idx
    PUSH  EDX

    CALL  ParentForm_IntParamAsm
    XCHG  EAX, EDI
    MOV   ECX, dword ptr [ESI+EDX*4]

    POP   EDX
    CALL  ECX
    POP   ESI
    POP   EDI
end;

{$ENDIF}

//======================================== THE END OF FILE KOL_ASM.inc
