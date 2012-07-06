unit Mmx;
{* MMX support unit. By Vladimir Kladov, 2003. }

interface

{$I KOLDEF.INC}

uses
  Windows, Kol;

type
  TCpuId = ( cpuNew486, cpuMMX, cpuMMX_Plus, cpu3DNow, cpu3DNow_Plus,
             cpuSSE, cpuSSE2 );
  {* Enumeration type to represent CPU type.
     cpuOld486: Old 486 Processor and earlier
     cpuNew486: New 486 Processor to Pentium1 without MMX
     cpuMMX   : MMX supported (but not SSE or SSE2)
     cpuSSE   : MMX and SSE supported (but not SSE2)
     cpuSSE2  : MMX, SSE and SSE2 supported
  }

  TCpuCaps = set of TCpuId;

function GetCPUType: TCpuCaps;
{* Checks CPU (Intel PC x86 Architecture) for MMX support.
|<p><p>

Use following constants in shuffle commands (like "pshufw") as third operand
to instruct to which locations (0,1,2,3) source parts should be placed:  }
const
  SH0000 = $00;
  SH0001 = $01;
  SH0002 = $02;
  SH0003 = $03;
  SH0010 = $04;
  SH0011 = $05;
  SH0012 = $06;
  SH0013 = $07;
  SH0020 = $08;
  SH0021 = $09;
  SH0022 = $0A;
  SH0023 = $0B;
  SH0030 = $0C;
  SH0031 = $0D;
  SH0032 = $0E;
  SH0033 = $0F;
  SH0100 = $10;
  SH0101 = $11;
  SH0102 = $12;
  SH0103 = $13;
  SH0110 = $14;
  SH0111 = $15;
  SH0112 = $16;
  SH0113 = $17;
  SH0120 = $18;
  SH0121 = $19;
  SH0122 = $1A;
  SH0123 = $1B;
  SH0130 = $1C;
  SH0131 = $1D;
  SH0132 = $1E;
  SH0133 = $1F;
  SH0200 = $20;
  SH0201 = $21;
  SH0202 = $22;
  SH0203 = $23;
  SH0210 = $24;
  SH0211 = $25;
  SH0212 = $26;
  SH0213 = $27;
  SH0220 = $28;
  SH0221 = $29;
  SH0222 = $2A;
  SH0223 = $2B;
  SH0230 = $2C;
  SH0231 = $2D;
  SH0232 = $2E;
  SH0233 = $2F;
  SH0300 = $30;
  SH0301 = $31;
  SH0302 = $32;
  SH0303 = $33;
  SH0310 = $34;
  SH0311 = $35;
  SH0312 = $36;
  SH0313 = $37;
  SH0320 = $38;
  SH0321 = $39;
  SH0322 = $3A;
  SH0323 = $3B;
  SH0330 = $3C;
  SH0331 = $3D;
  SH0332 = $3E;
  SH0333 = $3F;
  SH1000 = $40;
  SH1001 = $41;
  SH1002 = $42;
  SH1003 = $43;
  SH1010 = $44;
  SH1011 = $45;
  SH1012 = $46;
  SH1013 = $47;
  SH1020 = $48;
  SH1021 = $49;
  SH1022 = $4A;
  SH1023 = $4B;
  SH1030 = $4C;
  SH1031 = $4D;
  SH1032 = $4E;
  SH1033 = $4F;
  SH1100 = $50;
  SH1101 = $51;
  SH1102 = $52;
  SH1103 = $53;
  SH1110 = $54;
  SH1111 = $55;
  SH1112 = $56;
  SH1113 = $57;
  SH1120 = $58;
  SH1121 = $59;
  SH1122 = $5A;
  SH1123 = $5B;
  SH1130 = $5C;
  SH1131 = $5D;
  SH1132 = $5E;
  SH1133 = $5F;
  SH1200 = $60;
  SH1201 = $61;
  SH1202 = $62;
  SH1203 = $63;
  SH1210 = $64;
  SH1211 = $65;
  SH1212 = $66;
  SH1213 = $67;
  SH1220 = $68;
  SH1221 = $69;
  SH1222 = $6A;
  SH1223 = $6B;
  SH1230 = $6C;
  SH1231 = $6D;
  SH1232 = $6E;
  SH1233 = $6F;
  SH1300 = $70;
  SH1301 = $71;
  SH1302 = $72;
  SH1303 = $73;
  SH1310 = $74;
  SH1311 = $75;
  SH1312 = $76;
  SH1313 = $77;
  SH1320 = $78;
  SH1321 = $79;
  SH1322 = $7A;
  SH1323 = $7B;
  SH1330 = $7C;
  SH1331 = $7D;
  SH1332 = $7E;
  SH1333 = $7F;
  SH2000 = $80;
  SH2001 = $81;
  SH2002 = $82;
  SH2003 = $83;
  SH2010 = $84;
  SH2011 = $85;
  SH2012 = $86;
  SH2013 = $87;
  SH2020 = $88;
  SH2021 = $89;
  SH2022 = $8A;
  SH2023 = $8B;
  SH2030 = $8C;
  SH2031 = $8D;
  SH2032 = $8E;
  SH2033 = $8F;
  SH2100 = $90;
  SH2101 = $91;
  SH2102 = $92;
  SH2103 = $93;
  SH2110 = $94;
  SH2111 = $95;
  SH2112 = $96;
  SH2113 = $97;
  SH2120 = $98;
  SH2121 = $99;
  SH2122 = $9A;
  SH2123 = $9B;
  SH2130 = $9C;
  SH2131 = $9D;
  SH2132 = $9E;
  SH2133 = $9F;
  SH2200 = $A0;
  SH2201 = $A1;
  SH2202 = $A2;
  SH2203 = $A3;
  SH2210 = $A4;
  SH2211 = $A5;
  SH2212 = $A6;
  SH2213 = $A7;
  SH2220 = $A8;
  SH2221 = $A9;
  SH2222 = $AA;
  SH2223 = $AB;
  SH2230 = $AC;
  SH2231 = $AD;
  SH2232 = $AE;
  SH2233 = $AF;
  SH2300 = $B0;
  SH2301 = $B1;
  SH2302 = $B2;
  SH2303 = $B3;
  SH2310 = $B4;
  SH2311 = $B5;
  SH2312 = $B6;
  SH2313 = $B7;
  SH2320 = $B8;
  SH2321 = $B9;
  SH2322 = $BA;
  SH2323 = $BB;
  SH2330 = $BC;
  SH2331 = $BD;
  SH2332 = $BE;
  SH2333 = $BF;
  SH3000 = $C0;
  SH3001 = $C1;
  SH3002 = $C2;
  SH3003 = $C3;
  SH3010 = $C4;
  SH3011 = $C5;
  SH3012 = $C6;
  SH3013 = $C7;
  SH3020 = $C8;
  SH3021 = $C9;
  SH3022 = $CA;
  SH3023 = $CB;
  SH3030 = $CC;
  SH3031 = $CD;
  SH3032 = $CE;
  SH3033 = $CF;
  SH3100 = $D0;
  SH3101 = $D1;
  SH3102 = $D2;
  SH3103 = $D3;
  SH3110 = $D4;
  SH3111 = $D5;
  SH3112 = $D6;
  SH3113 = $D7;
  SH3120 = $D8;
  SH3121 = $D9;
  SH3122 = $DA;
  SH3123 = $DB;
  SH3130 = $DC;
  SH3131 = $DD;
  SH3132 = $DE;
  SH3133 = $DF;
  SH3200 = $E0;
  SH3201 = $E1;
  SH3202 = $E2;
  SH3203 = $E3;
  SH3210 = $E4;
  SH3211 = $E5;
  SH3212 = $E6;
  SH3213 = $E7;
  SH3220 = $E8;
  SH3221 = $E9;
  SH3222 = $EA;
  SH3223 = $EB;
  SH3230 = $EC;
  SH3231 = $ED;
  SH3232 = $EE;
  SH3233 = $EF;
  SH3300 = $F0;
  SH3301 = $F1;
  SH3302 = $F2;
  SH3303 = $F3;
  SH3310 = $F4;
  SH3311 = $F5;
  SH3312 = $F6;
  SH3313 = $F7;
  SH3320 = $F8;
  SH3321 = $F9;
  SH3322 = $FA;
  SH3323 = $FB;
  SH3330 = $FC;
  SH3331 = $FD;
  SH3332 = $FE;
  SH3333 = $FF;

implementation

var cpu: TCpuCaps = [ ];

function GetCPUType: TCpuCaps;
var I, J: Integer;
    Vend1: array[ 0..3 ] of Char;
begin
  Result := cpu;               // old 486 and earlier
  if Result <> [] then Exit;
  I := 0;
  asm // check if bit 21 of EFLAGS can be set and reset
    PUSHFD
    POP     EAX
    OR      EAX, 1 shl 21
    PUSH    EAX
    POPFD
    PUSHFD
    POP     EAX
    TEST    EAX, 1 shl 21
    JZ      @@1
    AND     EAX, not( 1 shl 21 )
    PUSH    EAX
    POPFD
    PUSHFD
    POP     EAX
    TEST    EAX, 1 shl 21
    JNZ     @@1
    INC     [ I ]
  @@1:
  end;
  if I = 0 then Exit;                    // CPUID not supported
  Include( Result, cpuNew486 );          // at least cpuNew486
  asm // get CPU features flags using CPUID command
    PUSH    EBX
    MOV     EAX, 0
    DB $0F, $A2 //CPUID : EAX, EBX,  EDX and ECX are changed!!!
    MOV     [ Vend1 ], EBX

    MOV     EAX, 1
    DB $0F, $A2 //CPUID : EAX, EBX,  EDX and ECX are changed!!!
    MOV     [ I ], EDX  // I := features information
    POP     EBX
  end;
  if (I and (1 shl 23)) = 0 then Exit;   // MMX not supported at all
  Include( Result, cpuMMX );             // MMX supported.
  if Vend1 = 'Auth' then // AuthenticAMD ?
  begin
    asm
      PUSH EBX
      MOV  EAX, $80000001
      DB $0F, $A2 //CPUID : EAX, EBX,  EDX and ECX are changed!!!
      MOV  [ J ], EDX
      POP  EBX
    end;
    if (J and (1 shl 22)) <> 0 then
      Include( Result, cpuMMX_Plus );    // MMX+ supported.
    if (J and (1 shl 31)) <> 0 then
    begin
      Include( Result, cpu3DNow );       // 3DNow! supported.
      if (J and (1 shl 30)) <> 0 then
        Include( Result, cpu3DNow_Plus );// 3DNow!+ supported.
    end;
  end;
  if (I and (1 shl 25)) <> 0 then
  begin
    Include( Result, cpuSSE );           // SSE supported.
    if (I and (1 shl 26)) <> 0 then
      Include( Result, cpuSSE2 );        // SSE2 supported.
  end;
  cpu := Result;
end;

end.
