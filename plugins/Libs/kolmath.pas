{=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

        KKKKK    KKKKK    OOOOOOOOO    LLLLL
        KKKKK    KKKKK  OOOOOOOOOOOOO  LLLLL
        KKKKK    KKKKK  OOOOO   OOOOO  LLLLL
        KKKKK  KKKKK    OOOOO   OOOOO  LLLLL
        KKKKKKKKKK      OOOOO   OOOOO  LLLLL
        KKKKK  KKKKK    OOOOO   OOOOO  LLLLL
        KKKKK    KKKKK  OOOOO   OOOOO  LLLLL
        KKKKK    KKKKK  OOOOOOOOOOOOO  LLLLLLLLLLLLL
        KKKKK    KKKKK    OOOOOOOOO    LLLLLLLLLLLLL

  Key Objects Library (C) 2000 by Kladov Vladimir.

  mailto: vk@kolmck.net
  Home: http://kolmck.net

 =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-}
{
  This code is grabbed from standard math.pas unit,
  provided by Borland Delphi. This unit is for working with
  engineering (mathematical) functions. The main difference
  is that err unit specially designed to handle exceptions
  for KOL is used instead of SysUtils. This allows to make
  size of the executable smaller for about 5K. though this
  value is insignificant for project made with VCL, it can
  be more than 15% of executable file size made with KOL.  
}

{*******************************************************}
{                                                       }
{       Borland Delphi Runtime Library                  }
{       Math Unit                                       }
{                                                       }
{       Copyright (C) 1996,99 Inprise Corporation       }
{                                                       }
{*******************************************************}

unit kolmath;

{ This unit contains high-performance arithmetic, trigonometric, logorithmic,
  statistical and financial calculation routines which supplement the math
  routines that are part of the Delphi language or System unit. }

{$N+,S-}

{$I KOLDEF.INC}

interface

uses {$IFNDEF MATH_NOERR} err, {$ENDIF} kol;

const   { Ranges of the IEEE floating point types, including denormals }
  MinSingle        =     1.5e-45;
  MaxSingle        =     3.4e+38;
  MinDouble        =     5.0e-324;
  MaxDouble        =     1.7e+308;
  MinExtended      =     3.4e-4932;
  MaxExtended      =     1.1e+4932;
  MinComp          =     -9.223372036854775807e+18;
  MaxComp          =     9.223372036854775807e+18;

{-----------------------------------------------------------------------
References:

1) P.J. Plauger, "The Standard C Library", Prentice-Hall, 1992, Ch. 7.
2) W.J. Cody, Jr., and W. Waite, "Software Manual For the Elementary
   Functions", Prentice-Hall, 1980.
3) Namir Shammas, "C/C++ Mathematical Algorithms for Scientists and Engineers",
   McGraw-Hill, 1995, Ch 8.
4) H.T. Lau, "A Numerical Library in C for Scientists and Engineers",
   CRC Press, 1994, Ch. 6.
5) "Pentium(tm) Processor User's Manual, Volume 3: Architecture
   and Programming Manual", Intel, 1994
+6)Уоррен Младший, "Арифметические трюки для программистов", исправленное изд.,
   2004

All angle parameters and results of trig functions are in radians.

Most of the following trig and log routines map directly to Intel 80387 FPU
floating point machine instructions.  Input domains, output ranges, and
error handling are determined largely by the FPU hardware.
Routines coded in assembler favor the Pentium FPU pipeline architecture.
-----------------------------------------------------------------------}

function EAbs( D: Double ): Double;
function EMax( const Values: array of Double ): Double;
function EMin( const Values: array of Double ): Double;
function ESign( X: Extended ): Integer;
function iMax( const Values: array of Integer ): Integer;
function iMin( const Values: array of Integer ): Integer;
function iSign( i: Integer ): Integer;

{ Trigonometric functions }
function ArcCos(X: Extended): Extended;  { IN: |X| <= 1  OUT: [0..PI] radians }
function ArcSin(X: Extended): Extended;  { IN: |X| <= 1  OUT: [-PI/2..PI/2] radians }

{ ArcTan2 calculates ArcTan(Y/X), and returns an angle in the correct quadrant.
  IN: |Y| < 2^64, |X| < 2^64, X <> 0   OUT: [-PI..PI] radians }
function ArcTan2(Y, X: Extended): Extended;

{ SinCos is 2x faster than calling Sin and Cos separately for the same angle }
procedure SinCos(Theta: Extended; var Sin, Cos: Extended) register;
function Tan(X: Extended): Extended;
function Cotan(X: Extended): Extended;           { 1 / tan(X), X <> 0 }
function Hypot(X, Y: Extended): Extended;        { Sqrt(X**2 + Y**2) }

{ Angle unit conversion routines }
function DegToRad(Degrees: Extended): Extended;  { Radians := Degrees * PI / 180}
function RadToDeg(Radians: Extended): Extended;  { Degrees := Radians * 180 / PI }
function GradToRad(Grads: Extended): Extended;   { Radians := Grads * PI / 200 }
function RadToGrad(Radians: Extended): Extended; { Grads := Radians * 200 / PI }
function CycleToRad(Cycles: Extended): Extended; { Radians := Cycles * 2PI }
function RadToCycle(Radians: Extended): Extended;{ Cycles := Radians / 2PI }

{ Hyperbolic functions and inverses }
function Cosh(X: Extended): Extended;
function Sinh(X: Extended): Extended;
function Tanh(X: Extended): Extended;
function ArcCosh(X: Extended): Extended;   { IN: X >= 1 }
function ArcSinh(X: Extended): Extended;
function ArcTanh(X: Extended): Extended;   { IN: |X| <= 1 }

{ Logorithmic functions }
function LnXP1(X: Extended): Extended;   { Ln(X + 1), accurate for X near zero }
function Log10(X: Extended): Extended;                     { Log base 10 of X}
function Log2(X: Extended): Extended;                      { Log base 2 of X }
function LogN(Base, X: Extended): Extended;                { Log base N of X }

{ Exponential functions }

{ IntPower: Raise base to an integral power.  Fast. }
//function IntPower(Base: Extended; Exponent: Integer): Extended register;
// -- already defined in kol.pas

{ Power: Raise base to any power.
  For fractional exponents, or |exponents| > MaxInt, base must be > 0. }
function Power(Base, Exponent: Extended): Extended;
{$IFNDEF _D6orHigher}
function Trunc( X: Extended ): Int64;
{$ENDIF}

{ Miscellaneous Routines }

{ Frexp:  Separates the mantissa and exponent of X. }
procedure Frexp(X: Extended; var Mantissa: Extended; var Exponent: Integer) register;

{ Ldexp: returns X*2**P }
function Ldexp(X: Extended; P: Integer): Extended register;

{ Ceil: Smallest integer >= X, |X| < MaxInt }
function Ceil(X: Extended):Integer;

{ Floor: Largest integer <= X,  |X| < MaxInt }
function Floor(X: Extended): Integer;

{ Poly: Evaluates a uniform polynomial of one variable at value X.
    The coefficients are ordered in increasing powers of X:
    Coefficients[0] + Coefficients[1]*X + ... + Coefficients[N]*(X**N) }
function Poly(X: Extended; const Coefficients: array of Double): Extended;

{-----------------------------------------------------------------------
Statistical functions.

Common commercial spreadsheet macro names for these statistical and
financial functions are given in the comments preceding each function.
-----------------------------------------------------------------------}

{ Mean:  Arithmetic average of values.  (AVG):  SUM / N }
function Mean(const Data: array of Double): Extended;

{ Sum: Sum of values.  (SUM) }
function Sum(const Data: array of Double): Extended register;
function SumInt(const Data: array of Integer): Integer register;
function SumOfSquares(const Data: array of Double): Extended;
procedure SumsAndSquares(const Data: array of Double;
  var Sum, SumOfSquares: Extended) register;

{ MinValue: Returns the smallest signed value in the data array (MIN) }
function MinValue(const Data: array of Double): Double;
function MinIntValue(const Data: array of Integer): Integer;

function Min(A,B: Integer): Integer;
{$IFDEF _D4orHigher}
overload;
function Min(A,B: I64): I64; overload;
function Min(A,B: Int64): Int64; overload;
function Min(A,B: Single): Single; overload;
function Min(A,B: Double): Double; overload;
function Min(A,B: Extended): Extended; overload;
{$ENDIF}

{ MaxValue: Returns the largest signed value in the data array (MAX) }
function MaxValue(const Data: array of Double): Double;
function MaxIntValue(const Data: array of Integer): Integer;

function Max(A,B: Integer): Integer;
{$IFDEF _D4orHigher}
overload;
function Max(A,B: I64): I64; overload;
function Max(A,B: Single): Single; overload;
function Max(A,B: Double): Double; overload;
function Max(A,B: Extended): Extended; overload;
{$ENDIF}

{ Standard Deviation (STD): Sqrt(Variance). aka Sample Standard Deviation }
function StdDev(const Data: array of Double): Extended;

{ MeanAndStdDev calculates Mean and StdDev in one call. }
procedure MeanAndStdDev(const Data: array of Double; var Mean, StdDev: Extended);

{ Population Standard Deviation (STDP): Sqrt(PopnVariance).
  Used in some business and financial calculations. }
function PopnStdDev(const Data: array of Double): Extended;

{ Variance (VARS): TotalVariance / (N-1). aka Sample Variance }
function Variance(const Data: array of Double): Extended;

{ Population Variance (VAR or VARP): TotalVariance/ N }
function PopnVariance(const Data: array of Double): Extended;

{ Total Variance: SUM(i=1,N)[(X(i) - Mean)**2] }
function TotalVariance(const Data: array of Double): Extended;

{ Norm:  The Euclidean L2-norm.  Sqrt(SumOfSquares) }
function Norm(const Data: array of Double): Extended;

{ MomentSkewKurtosis: Calculates the core factors of statistical analysis:
  the first four moments plus the coefficients of skewness and kurtosis.
  M1 is the Mean.  M2 is the Variance.
  Skew reflects symmetry of distribution: M3 / (M2**(3/2))
  Kurtosis reflects flatness of distribution: M4 / Sqr(M2) }
procedure MomentSkewKurtosis(const Data: array of Double;
  var M1, M2, M3, M4, Skew, Kurtosis: Extended);

{ RandG produces random numbers with Gaussian distribution about the mean.
  Useful for simulating data with sampling errors. }
function RandG(Mean, StdDev: Extended): Extended;

{-----------------------------------------------------------------------
Financial functions.  Standard set from Quattro Pro.

Parameter conventions:

From the point of view of A, amounts received by A are positive and
amounts disbursed by A are negative (e.g. a borrower's loan repayments
are regarded by the borrower as negative).

Interest rates are per payment period.  11% annual percentage rate on a
loan with 12 payments per year would be (11 / 100) / 12 = 0.00916667

-----------------------------------------------------------------------}

type
  TPaymentTime = (ptEndOfPeriod, ptStartOfPeriod);

{ Double Declining Balance (DDB) }
function DoubleDecliningBalance(Cost, Salvage: Extended;
  Life, Period: Integer): Extended;

{ Future Value (FVAL) }
function FutureValue(Rate: Extended; NPeriods: Integer; Payment, PresentValue:
  Extended; PaymentTime: TPaymentTime): Extended;

{ Interest Payment (IPAYMT)  }
function InterestPayment(Rate: Extended; Period, NPeriods: Integer; PresentValue,
  FutureValue: Extended; PaymentTime: TPaymentTime): Extended;

{ Interest Rate (IRATE) }
function InterestRate(NPeriods: Integer;
  Payment, PresentValue, FutureValue: Extended; PaymentTime: TPaymentTime): Extended;

{ Internal Rate of Return. (IRR) Needs array of cash flows. }
function InternalRateOfReturn(Guess: Extended;
  const CashFlows: array of Double): Extended;

{ Number of Periods (NPER) }
function NumberOfPeriods(Rate, Payment, PresentValue, FutureValue: Extended;
  PaymentTime: TPaymentTime): Extended;

{ Net Present Value. (NPV) Needs array of cash flows. }
function NetPresentValue(Rate: Extended; const CashFlows: array of Double;
  PaymentTime: TPaymentTime): Extended;

{ Payment (PAYMT) }
function Payment(Rate: Extended; NPeriods: Integer;
  PresentValue, FutureValue: Extended; PaymentTime: TPaymentTime): Extended;

{ Period Payment (PPAYMT) }
function PeriodPayment(Rate: Extended; Period, NPeriods: Integer;
  PresentValue, FutureValue: Extended; PaymentTime: TPaymentTime): Extended;

{ Present Value (PVAL) }
function PresentValue(Rate: Extended; NPeriods: Integer;
  Payment, FutureValue: Extended; PaymentTime: TPaymentTime): Extended;

{ Straight Line depreciation (SLN) }
function SLNDepreciation(Cost, Salvage: Extended; Life: Integer): Extended;

{ Sum-of-Years-Digits depreciation (SYD) }
function SYDDepreciation(Cost, Salvage: Extended; Life, Period: Integer): Extended;

{type
  EInvalidArgument = class(EMathError) end;}

{------------------------------------------------------------------------------}
{ Integer and logical functions }
function IsPowerOf2( i: Integer ): Boolean;
{* TRUE, если число является степенью числа 2 }

function Low1( i: Integer ): Integer;
{* Выделяет младший бит 1 из числа i. }

function Low0( i: Integer ): Integer;
{* Выделяет младший справа бит 0 из числа i, например, 1100011 -> 100 }

function count_1_bits_in_byte( x: Byte ): Byte;
{* Подсчитывает число единичных битов в байте }

function count_1_bits_in_dword( x: Integer ): Integer;
{* Подсчитывает число единичных битов в 32-битном }


implementation

{$IFNDEF _D2orD3}
uses SysConst;
{$ENDIF}

function EAbs( D: Double ): Double;
begin
  Result := D;
  if Result < 0.0 then
    Result := -Result;
end;

function EMax( const Values: array of Double ): Double;
var I: Integer;
begin
  Result := Values[ 0 ];
  for I := 1 to High( Values ) do
    if Result < Values[ I ] then Result := Values[ I ];
end;

function EMin( const Values: array of Double ): Double;
var I: Integer;
begin
  Result := Values[ 0 ];
  for I := 1 to High( Values ) do
    if Result > Values[ I ] then Result := Values[ I ];
end;

function ESign( X: Extended ): Integer;
begin
  if X < 0 then Result := -1
  else if X > 0 then Result := 1
  else Result := 1;
end;

function iMax( const Values: array of Integer ): Integer;
var I: Integer;
begin
  Result := Values[ 0 ];
  for I := 1 to High( Values ) do
    if Result < Values[ I ] then Result := Values[ I ];
end;

function iMin( const Values: array of Integer ): Integer;
var I: Integer;
begin
  Result := Values[ 0 ];
  for I := 1 to High( Values ) do
    if Result > Values[ I ] then Result := Values[ I ];
end;

{$IFDEF PAS_VERSION}
function iSign( i: Integer ): Integer;
begin
  if i < 0 then Result := -1
  else if i > 0 then Result := 1
  else Result := 0;
end;
{$ELSE}
function iSign( i: Integer ): Integer;
asm
  XOR  EDX, EDX
  TEST EAX, EAX
  JZ   @@exit
  MOV  DL, 1
  JG   @@exit
  OR   EDX, -1
@@exit:
  XCHG EAX, EDX
end;
{$ENDIF}

function Annuity2(R: Extended; N: Integer; PaymentTime: TPaymentTime;
  var CompoundRN: Extended): Extended; Forward;
function Compound(R: Extended; N: Integer): Extended; Forward;
function RelSmall(X, Y: Extended): Boolean; Forward;

type
  TPoly = record
    Neg, Pos, DNeg, DPos: Extended
  end;

const
  MaxIterations = 15;

{$IFNDEF MATH_NOERR}
procedure ArgError(const Msg: string);
begin
  raise Exception.Create(e_Math_InvalidArgument, Msg);
end;
{$ENDIF}

function DegToRad(Degrees: Extended): Extended;  { Radians := Degrees * PI / 180 }
begin
  Result := Degrees * (PI / 180);
end;

function RadToDeg(Radians: Extended): Extended;  { Degrees := Radians * 180 / PI }
begin
  Result := Radians * (180 / PI);
end;

function GradToRad(Grads: Extended): Extended;   { Radians := Grads * PI / 200 }
begin
  Result := Grads * (PI / 200);
end;

function RadToGrad(Radians: Extended): Extended; { Grads := Radians * 200 / PI}
begin
  Result := Radians * (200 / PI);
end;

function CycleToRad(Cycles: Extended): Extended; { Radians := Cycles * 2PI }
begin
  Result := Cycles * (2 * PI);
end;

function RadToCycle(Radians: Extended): Extended;{ Cycles := Radians / 2PI }
begin
  Result := Radians / (2 * PI);
end;

function LnXP1(X: Extended): Extended;
{ Return ln(1 + X).  Accurate for X near 0. }
asm
        FLDLN2
        MOV     AX,WORD PTR X+8               { exponent }
        FLD     X
        CMP     AX,$3FFD                      { .4225 }
        JB      @@1
        FLD1
        FADD
        FYL2X
        JMP     @@2
@@1:
        FYL2XP1
@@2:
        FWAIT
end;

{ Invariant: Y >= 0 & Result*X**Y = X**I.  Init Y = I and Result = 1. }
{function IntPower(X: Extended; I: Integer): Extended;
var
  Y: Integer;
begin
  Y := Abs(I);
  Result := 1.0;
  while Y > 0 do begin
    while not Odd(Y) do
    begin
      Y := Y shr 1;
      X := X * X
    end;
    Dec(Y);
    Result := Result * X
  end;
  if I < 0 then Result := 1.0 / Result
end;
}
(* -- already defined in kol.pas
function IntPower(Base: Extended; Exponent: Integer): Extended;
asm
        mov     ecx, eax
        cdq
        fld1                      { Result := 1 }
        xor     eax, edx
        sub     eax, edx          { eax := Abs(Exponent) }
        jz      @@3
        fld     Base
        jmp     @@2
@@1:    fmul    ST, ST            { X := Base * Base }
@@2:    shr     eax,1
        jnc     @@1
        fmul    ST(1),ST          { Result := Result * X }
        jnz     @@1
        fstp    st                { pop X from FPU stack }
        cmp     ecx, 0
        jge     @@3
        fld1
        fdivrp                    { Result := 1 / Result }
@@3:
        fwait
end;
*)

function Compound(R: Extended; N: Integer): Extended;
{ Return (1 + R)**N. }
begin
  Result := IntPower(1.0 + R, N)
end;

function Annuity2(R: Extended; N: Integer; PaymentTime: TPaymentTime;
  var CompoundRN: Extended): Extended;
{ Set CompoundRN to Compound(R, N),
	return (1+Rate*PaymentTime)*(Compound(R,N)-1)/R;
}
begin
  if R = 0.0 then
  begin
    CompoundRN := 1.0;
    Result := N;
  end
  else
  begin
    { 6.1E-5 approx= 2**-14 }
    if EAbs(R) < 6.1E-5 then
    begin
      CompoundRN := Exp(N * LnXP1(R));
      Result := N*(1+(N-1)*R/2);
    end
    else
    begin
      CompoundRN := Compound(R, N);
      Result := (CompoundRN-1) / R
    end;
    if PaymentTime = ptStartOfPeriod then
      Result := Result * (1 + R);
  end;
end; {Annuity2}


procedure PolyX(const A: array of Double; X: Extended; var Poly: TPoly);
{ Compute A[0] + A[1]*X + ... + A[N]*X**N and X * its derivative.
  Accumulate positive and negative terms separately. }
var
  I: Integer;
  Neg, Pos, DNeg, DPos: Extended;
begin
  Neg := 0.0;
  Pos := 0.0;
  DNeg := 0.0;
  DPos := 0.0;
	for I := High(A) downto  Low(A) do
  begin
    DNeg := X * DNeg + Neg;
    Neg := Neg * X;
    DPos := X * DPos + Pos;
    Pos := Pos * X;
    if A[I] >= 0.0 then
      Pos := Pos + A[I]
    else
      Neg := Neg + A[I]
  end;
  Poly.Neg := Neg;
  Poly.Pos := Pos;
  Poly.DNeg := DNeg * X;
  Poly.DPos := DPos * X;
end; {PolyX}


function RelSmall(X, Y: Extended): Boolean;
{ Returns True if X is small relative to Y }
const
  C1: Double = 1E-15;
  C2: Double = 1E-12;
begin
  Result := EAbs(X) < (C1 + C2 * EAbs(Y))
end;

{ Math functions. }

function ArcCos(X: Extended): Extended;
begin
  if X > 0.999999999999999 then
    Result := 0 {иначе -NAN !}
  else
  if X < -0.999999999999999 then
    Result := PI
  else
  Result := ArcTan2(Sqrt(1 - X*X), X);
end;

function ArcSin(X: Extended): Extended;
begin
  Result := ArcTan2(X, Sqrt(1 - X*X))
end;

function ArcTan2(Y, X: Extended): Extended;
asm
        FLD     Y
        FLD     X
        FPATAN
        FWAIT
end;

function Tan(X: Extended): Extended;
{  Tan := Sin(X) / Cos(X) }
asm
        FLD    X
        FPTAN
        FSTP   ST(0)      { FPTAN pushes 1.0 after result }
        FWAIT
end;

function CoTan(X: Extended): Extended;
{ CoTan := Cos(X) / Sin(X) = 1 / Tan(X) }
asm
        FLD   X
        FPTAN
        FDIVRP
        FWAIT
end;

function Hypot(X, Y: Extended): Extended;
{ formula: Sqrt(X*X + Y*Y)
  implemented as:  |Y|*Sqrt(1+Sqr(X/Y)), |X| < |Y| for greater precision
var
  Temp: Extended;
begin
  X := Abs(X);
  Y := Abs(Y);
  if X > Y then
  begin
    Temp := X;
    X := Y;
    Y := Temp;
  end;
  if X = 0 then
    Result := Y
  else         // Y > X, X <> 0, so Y > 0
    Result := Y * Sqrt(1 + Sqr(X/Y));
end;
}
asm
        FLD     Y
        FABS
        FLD     X
        FABS
        FCOM
        FNSTSW  AX
        TEST    AH,$45
        JNZ      @@1        // if ST > ST(1) then swap
        FXCH    ST(1)      // put larger number in ST(1)
@@1:    FLDZ
        FCOMP
        FNSTSW  AX
        TEST    AH,$40     // if ST = 0, return ST(1)
        JZ      @@2
        FSTP    ST         // eat ST(0)
        JMP     @@3
@@2:    FDIV    ST,ST(1)   // ST := ST / ST(1)
        FMUL    ST,ST      // ST := ST * ST
        FLD1
        FADD               // ST := ST + 1
        FSQRT              // ST := Sqrt(ST)
        FMUL               // ST(1) := ST * ST(1); Pop ST
@@3:    FWAIT
end;


procedure SinCos(Theta: Extended; var Sin, Cos: Extended);
asm
        FLD     Theta
        FSINCOS
        FSTP    tbyte ptr [edx]    // Cos
        FSTP    tbyte ptr [eax]    // Sin
        FWAIT
end;

{ Extract exponent and mantissa from X }
procedure Frexp(X: Extended; var Mantissa: Extended; var Exponent: Integer);
{ Mantissa ptr in EAX, Exponent ptr in EDX }
asm
        FLD     X
        PUSH    EAX
        MOV     dword ptr [edx], 0    { if X = 0, return 0 }

        FTST
        FSTSW   AX
        FWAIT
        SAHF
        JZ      @@Done

        FXTRACT                 // ST(1) = exponent, (pushed) ST = fraction
        FXCH

// The FXTRACT instruction normalizes the fraction 1 bit higher than
// wanted for the definition of frexp() so we need to tweak the result
// by scaling the fraction down and incrementing the exponent.

        FISTP   dword ptr [edx]
        FLD1
        FCHS
        FXCH
        FSCALE                  // scale fraction
        INC     dword ptr [edx] // exponent biased to match
        FSTP ST(1)              // discard -1, leave fraction as TOS

@@Done:
        POP     EAX
        FSTP    tbyte ptr [eax]
        FWAIT
end;

function Ldexp(X: Extended; P: Integer): Extended;
  { Result := X * (2^P) }
asm
        PUSH    EAX
        FILD    dword ptr [ESP]
        FLD     X
        FSCALE
        POP     EAX
        FSTP    ST(1)
        FWAIT
end;

function Ceil(X: Extended): Integer;
begin
  Result := Integer(Trunc(X));
  if Frac(X) > 0 then
    Inc(Result);
end;

function Floor(X: Extended): Integer;
begin
  Result := Integer(Trunc(X));
  if Frac(X) < 0 then
    Dec(Result);
end;

{ Conversion of bases:  Log.b(X) = Log.a(X) / Log.a(b)  }

function Log10(X: Extended): Extended;
  { Log.10(X) := Log.2(X) * Log.10(2) }
asm
        FLDLG2     { Log base ten of 2 }
        FLD     X
        FYL2X
        FWAIT
end;

function Log2(X: Extended): Extended;
asm
        FLD1
        FLD     X
        FYL2X
        FWAIT
end;

function LogN(Base, X: Extended): Extended;
{ Log.N(X) := Log.2(X) / Log.2(N) }
asm
        FLD1
        FLD     X
        FYL2X
        FLD1
        FLD     Base
        FYL2X
        FDIV
        FWAIT
end;

function Poly(X: Extended; const Coefficients: array of Double): Extended;
{ Horner's method }
var
  I: Integer;
begin
  Result := Coefficients[High(Coefficients)];
  for I := High(Coefficients)-1 downto Low(Coefficients) do
    Result := Result * X + Coefficients[I];
end;

function Power(Base, Exponent: Extended): Extended;
begin
  if Exponent = 0.0 then
    Result := 1.0               { n**0 = 1 }
  else if (Base = 0.0) and (Exponent > 0.0) then
    Result := 0.0               { 0**n = 0, n > 0 }
  else if (Frac(Exponent) = 0.0) and (EAbs(Exponent) <= MaxInt) then
    Result := IntPower(Base, Integer(Trunc(Exponent)))
  else
    Result := Exp(Exponent * Ln(Base))
end;

{$IFNDEF _D6orHigher}
(*function Trunc1( X: Extended ): Int64;
begin
  Result := System.Trunc( X );
end;
asm
       FLD     qword ptr [ESP+4]
       { ->    FST(0)   Extended argument       }
       { <-    EDX:EAX  Result                  }


        SUB     ESP,12
        FNSTCW  [ESP].Word          // save
        FNSTCW  [ESP+2].Word        // scratch
        FWAIT
        OR      [ESP+2].Word, $0F00  // trunc toward zero, full precision
        FLDCW   [ESP+2].Word
        FISTP   qword ptr [ESP+4]
        FWAIT
        FLDCW   [ESP].Word
        POP     ECX
        POP     EAX
        POP     EDX
end;*)

function Trunc( X: Extended ): Int64;
begin
  if Abs( X ) < 1 then Result := 0 else
  if X < 0 then Result := -System.Trunc( -X )
  else Result := System.Trunc( X );
end;
{$ENDIF}


{ Hyperbolic functions }

function CoshSinh(X: Extended; Factor: Double): Extended;
begin
  Result := Exp(X) / 2;
  Result := Result + Factor / Result;
end;

function Cosh(X: Extended): Extended;
begin
  Result := CoshSinh(X, 0.25)
end;

function Sinh(X: Extended): Extended;
begin
  Result := CoshSinh(X, -0.25)
end;

const
  MaxTanhDomain = 5678.22249441322; // Ln(MaxExtended)/2

function Tanh(X: Extended): Extended;
begin
  if X > MaxTanhDomain then
    Result := 1.0
  else if X < -MaxTanhDomain then
    Result := -1.0
  else
  begin
    Result := Exp(X);
    Result := Result * Result;
    Result := (Result - 1.0) / (Result + 1.0)
  end;
end;

function ArcCosh(X: Extended): Extended;
begin
  if X <= 1.0 then
    Result := 0.0
  else if X > 1.0e10 then
    Result := Ln(2) + Ln(X)
  else
    Result := Ln(X + Sqrt((X - 1.0) * (X + 1.0)));
end;

function ArcSinh(X: Extended): Extended;
var
  Neg: Boolean;
begin
  if X = 0 then
    Result := 0
  else
  begin
    Neg := (X < 0);
    X := EAbs(X);
    if X > 1.0e10 then
      Result := Ln(2) + Ln(X)
    else
    begin
      Result := X*X;
      Result := LnXP1(X + Result / (1 + Sqrt(1 + Result)));
    end;
    if Neg then Result := -Result;
  end;
end;

function ArcTanh(X: Extended): Extended;
var
  Neg: Boolean;
begin
  if X = 0 then
    Result := 0
  else
  begin
    Neg := (X < 0);
    X := EAbs(X);
    if X >= 1 then
      Result := MaxExtended
    else
      Result := 0.5 * LnXP1((2.0 * X) / (1.0 - X));
    if Neg then Result := -Result;
  end;
end;

{ Statistical functions }

function Mean(const Data: array of Double): Extended;
begin
  Result := SUM(Data) / (High(Data) - Low(Data) + 1)
end;

function MinValue(const Data: array of Double): Double;
var
  I: Integer;
begin
  Result := Data[Low(Data)];
  for I := Low(Data) + 1 to High(Data) do
    if Result > Data[I] then
      Result := Data[I];
end;

function MinIntValue(const Data: array of Integer): Integer;
var
  I: Integer;
begin
  Result := Data[Low(Data)];
  for I := Low(Data) + 1 to High(Data) do
    if Result > Data[I] then
      Result := Data[I];
end;

{$IFDEF ASM_VERSION}
function Min(A,B: Integer): Integer;
asm
  CMP EAX, EDX
  JL  @@1
  XCHG EAX, EDX
@@1:
end;
{$ELSE}
function Min(A,B: Integer): Integer;
begin
  if A < B then
    Result := A
  else
    Result := B;
end;
{$ENDIF}

{$IFDEF _D4orHigher}
function Min(A,B: I64): I64;
begin
  if Cmp64( A, B ) < 0 then
    Result := A
  else
    Result := B;
end;

function Min(A,B: Int64): Int64;
begin
  if A < B then
    Result := A
  else
    Result := B;
end;

function Min(A,B: Single): Single;
begin
  if A < B then
    Result := A
  else
    Result := B;
end;

function Min(A,B: Double): Double;
begin
  if A < B then
    Result := A
  else
    Result := B;
end;

function Min(A,B: Extended): Extended;
begin
  if A < B then
    Result := A
  else
    Result := B;
end;
{$ENDIF}

function MaxValue(const Data: array of Double): Double;
var
  I: Integer;
begin
  Result := Data[Low(Data)];
  for I := Low(Data) + 1 to High(Data) do
    if Result < Data[I] then
      Result := Data[I];
end;

function MaxIntValue(const Data: array of Integer): Integer;
var
  I: Integer;
begin
  Result := Data[Low(Data)];
  for I := Low(Data) + 1 to High(Data) do
    if Result < Data[I] then
      Result := Data[I];
end;

{$IFDEF ASM_VERSION}
function Max(A,B: Integer): Integer;
asm
  CMP EAX, EDX
  JG  @@1
  XCHG EAX, EDX
@@1:
end;
{$ELSE}
function Max(A,B: Integer): Integer;
begin
  if A > B then
    Result := A
  else
    Result := B;
end;
{$ENDIF}

{$IFDEF _D4orHigher}
function Max(A,B: I64): I64;
begin
  if Cmp64( A, B ) > 0 then
    Result := A
  else
    Result := B;
end;

function Max(A,B: Single): Single;
begin
  if A > B then
    Result := A
  else
    Result := B;
end;

function Max(A,B: Double): Double;
begin
  if A > B then
    Result := A
  else
    Result := B;
end;

function Max(A,B: Extended): Extended;
begin
  if A > B then
    Result := A
  else
    Result := B;
end;
{$ENDIF}

procedure MeanAndStdDev(const Data: array of Double; var Mean, StdDev: Extended);
var
  S: Extended;
  N,I: Integer;
begin
  N := High(Data)- Low(Data) + 1;
  if N = 1 then
  begin
    Mean := Data[0];
    StdDev := Data[0];
    Exit;
  end;
  Mean := Sum(Data) / N;
  S := 0;               // sum differences from the mean, for greater accuracy
  for I := Low(Data) to High(Data) do
    S := S + Sqr(Mean - Data[I]);
  StdDev := Sqrt(S / (N - 1));
end;

procedure MomentSkewKurtosis(const Data: array of Double;
  var M1, M2, M3, M4, Skew, Kurtosis: Extended);
var
  Sum, SumSquares, SumCubes, SumQuads, OverN, Accum, M1Sqr, S2N, S3N: Extended;
  I: Integer;
begin
  OverN := 1 / (High(Data) - Low(Data) + 1);
  Sum := 0;
  SumSquares := 0;
  SumCubes := 0;
  SumQuads := 0;
  for I := Low(Data) to High(Data) do
  begin
    Sum := Sum + Data[I];
    Accum := Sqr(Data[I]);
    SumSquares := SumSquares + Accum;
    Accum := Accum*Data[I];
    SumCubes := SumCubes + Accum;
    SumQuads := SumQuads + Accum*Data[I];
  end;
  M1 := Sum * OverN;
  M1Sqr := Sqr(M1);
  S2N := SumSquares * OverN;
  S3N := SumCubes * OverN;
  M2 := S2N - M1Sqr;
  M3 := S3N - (M1 * 3 * S2N) + 2*M1Sqr*M1;
  M4 := (SumQuads * OverN) - (M1 * 4 * S3N) + (M1Sqr*6*S2N - 3*Sqr(M1Sqr));
  Skew := M3 * Power(M2, -3/2);   // = M3 / Power(M2, 3/2)
  Kurtosis := M4 / Sqr(M2);
end;

function Norm(const Data: array of Double): Extended;
begin
  Result := Sqrt(SumOfSquares(Data));
end;

function PopnStdDev(const Data: array of Double): Extended;
begin
  Result := Sqrt(PopnVariance(Data))
end;

function PopnVariance(const Data: array of Double): Extended;
begin
  Result := TotalVariance(Data) / (High(Data) - Low(Data) + 1)
end;

function RandG(Mean, StdDev: Extended): Extended;
{ Marsaglia-Bray algorithm }
var
  U1, S2: Extended;
begin
  repeat
    U1 := 2*Random - 1;
    S2 := Sqr(U1) + Sqr(2*Random-1);
  until S2 < 1;
  Result := Sqrt(-2*Ln(S2)/S2) * U1 * StdDev + Mean;
end;

function StdDev(const Data: array of Double): Extended;
begin
  Result := Sqrt(Variance(Data))
end;

procedure RaiseOverflowError; forward;

function SumInt(const Data: array of Integer): Integer;
{var
  I: Integer;
begin
  Result := 0;
  for I := Low(Data) to High(Data) do
    Result := Result + Data[I]
end; }
asm  // IN: EAX = ptr to Data, EDX = High(Data) = Count - 1
     // loop unrolled 4 times, 5 clocks per loop, 1.2 clocks per datum
      PUSH EBX
      MOV  ECX, EAX         // ecx = ptr to data
      MOV  EBX, EDX
      XOR  EAX, EAX
      AND  EDX, not 3
      AND  EBX, 3
      SHL  EDX, 2
      JMP  @Vector.Pointer[EBX*4]
@Vector:
      DD @@1
      DD @@2
      DD @@3
      DD @@4
@@4:
      ADD  EAX, [ECX+12+EDX]
      JO   @@RaiseOverflowError
@@3:
      ADD  EAX, [ECX+8+EDX]
      JO   @@RaiseOverflowError
@@2:
      ADD  EAX, [ECX+4+EDX]
      JO   @@RaiseOverflowError
@@1:
      ADD  EAX, [ECX+EDX]
      JO   @@RaiseOverflowError
      SUB  EDX,16
      JNS  @@4
      POP  EBX
      RET
@@RaiseOverflowError:
      POP EBX
      POP ECX
      JMP RaiseOverflowError
end;

procedure RaiseOverflowError;
begin
  {$IFNDEF MATH_NOERR}
  raise Exception.Create(e_IntOverflow, SIntOverflow);
  {$ENDIF}
end;

function SUM(const Data: array of Double): Extended;
{var
  I: Integer;
begin
  Result := 0.0;
  for I := Low(Data) to High(Data) do
    Result := Result + Data[I]
end; }
asm  // IN: EAX = ptr to Data, EDX = High(Data) = Count - 1
     // Uses 4 accumulators to minimize read-after-write delays and loop overhead
     // 5 clocks per loop, 4 items per loop = 1.2 clocks per item
       FLDZ
       MOV      ECX, EDX
       FLD      ST(0)
       AND      EDX, not 3
       FLD      ST(0)
       AND      ECX, 3
       FLD      ST(0)
       SHL      EDX, 3      // count * sizeof(Double) = count * 8
       JMP      @Vector.Pointer[ECX*4]
@Vector:
       DD @@1
       DD @@2
       DD @@3
       DD @@4
@@4:   FADD     qword ptr [EAX+EDX+24]    // 1
       FXCH     ST(3)                     // 0
@@3:   FADD     qword ptr [EAX+EDX+16]    // 1
       FXCH     ST(2)                     // 0
@@2:   FADD     qword ptr [EAX+EDX+8]     // 1
       FXCH     ST(1)                     // 0
@@1:   FADD     qword ptr [EAX+EDX]       // 1
       FXCH     ST(2)                     // 0
       SUB      EDX, 32
       JNS      @@4
       FADDP    ST(3),ST                  // ST(3) := ST + ST(3); Pop ST
       FADD                               // ST(1) := ST + ST(1); Pop ST
       FADD                               // ST(1) := ST + ST(1); Pop ST
       FWAIT
end;

function SumOfSquares(const Data: array of Double): Extended;
var
  I: Integer;
begin
  Result := 0.0;
  for I := Low(Data) to High(Data) do
    Result := Result + Sqr(Data[I]);
end;

procedure SumsAndSquares(const Data: array of Double; var Sum, SumOfSquares: Extended);
{var
  I: Integer;
begin
  Sum := 0;
  SumOfSquares := 0;
  for I := Low(Data) to High(Data) do
  begin
    Sum := Sum + Data[I];
    SumOfSquares := SumOfSquares + Data[I]*Data[I];
  end;
end;  }
asm  // IN:  EAX = ptr to Data
     //      EDX = High(Data) = Count - 1
     //      ECX = ptr to Sum
     // Est. 17 clocks per loop, 4 items per loop = 4.5 clocks per data item
       FLDZ                 // init Sum accumulator
       PUSH     ECX
       MOV      ECX, EDX
       FLD      ST(0)       // init Sqr1 accum.
       AND      EDX, not 3
       FLD      ST(0)       // init Sqr2 accum.
       AND      ECX, 3
       FLD      ST(0)       // init/simulate last data item left in ST
       SHL      EDX, 3      // count * sizeof(Double) = count * 8
       JMP      @Vector.Pointer[ECX*4]
@Vector:
       DD @@1
       DD @@2
       DD @@3
       DD @@4
@@4:   FADD                            // Sqr2 := Sqr2 + Sqr(Data4); Pop Data4
       FLD     qword ptr [EAX+EDX+24]  // Load Data1
       FADD    ST(3),ST                // Sum := Sum + Data1
       FMUL    ST,ST                   // Data1 := Sqr(Data1)
@@3:   FLD     qword ptr [EAX+EDX+16]  // Load Data2
       FADD    ST(4),ST                // Sum := Sum + Data2
       FMUL    ST,ST                   // Data2 := Sqr(Data2)
       FXCH                            // Move Sqr(Data1) into ST(0)
       FADDP   ST(3),ST                // Sqr1 := Sqr1 + Sqr(Data1); Pop Data1
@@2:   FLD     qword ptr [EAX+EDX+8]   // Load Data3
       FADD    ST(4),ST                // Sum := Sum + Data3
       FMUL    ST,ST                   // Data3 := Sqr(Data3)
       FXCH                            // Move Sqr(Data2) into ST(0)
       FADDP   ST(3),ST                // Sqr1 := Sqr1 + Sqr(Data2); Pop Data2
@@1:   FLD     qword ptr [EAX+EDX]     // Load Data4
       FADD    ST(4),ST                // Sum := Sum + Data4
       FMUL    ST,ST                   // Sqr(Data4)
       FXCH                            // Move Sqr(Data3) into ST(0)
       FADDP   ST(3),ST                // Sqr1 := Sqr1 + Sqr(Data3); Pop Data3
       SUB     EDX,32
       JNS     @@4
       FADD                         // Sqr2 := Sqr2 + Sqr(Data4); Pop Data4
       POP     ECX
       FADD                         // Sqr1 := Sqr2 + Sqr1; Pop Sqr2
       FXCH                         // Move Sum1 into ST(0)
       MOV     EAX, SumOfSquares
       FSTP    tbyte ptr [ECX]      // Sum := Sum1; Pop Sum1
       FSTP    tbyte ptr [EAX]      // SumOfSquares := Sum1; Pop Sum1
       FWAIT
end;

function TotalVariance(const Data: array of Double): Extended;
var
  Sum, SumSquares: Extended;
begin
  SumsAndSquares(Data, Sum, SumSquares);
  Result := SumSquares - Sqr(Sum)/(High(Data) - Low(Data) + 1);
end;

function Variance(const Data: array of Double): Extended;
begin
  Result := TotalVariance(Data) / (High(Data) - Low(Data))
end;


{ Depreciation functions. }

function DoubleDecliningBalance(Cost, Salvage: Extended; Life, Period: Integer): Extended;
{ dv := cost * (1 - 2/life)**(period - 1)
  DDB = (2/life) * dv
  if DDB > dv - salvage then DDB := dv - salvage
  if DDB < 0 then DDB := 0
}
var
  DepreciatedVal, Factor: Extended;
begin
  Result := 0;
	if (Period < 1) or (Life < Period) or (Life < 1) or (Cost <= Salvage) then
    Exit;

	{depreciate everything in period 1 if life is only one or two periods}
	if ( Life <= 2 ) then
  begin
		if ( Period = 1 ) then
      DoubleDecliningBalance:=Cost-Salvage
		else
			DoubleDecliningBalance:=0; {all depreciation occurred in first period}
		exit;
  end;
  Factor := 2.0 / Life;

  DepreciatedVal := Cost * IntPower((1.0 - Factor), Period - 1);
	   {DepreciatedVal is Cost-(sum of previous depreciation results)}

  Result := Factor * DepreciatedVal;
	   {Nominal computed depreciation for this period.  The rest of the
			function applies limits to this nominal value. }

	{Only depreciate until total depreciation equals cost-salvage.}
  if Result > DepreciatedVal - Salvage then
    Result := DepreciatedVal - Salvage;

	{No more depreciation after salvage value is reached.  This is mostly a nit.
	 If Result is negative at this point, it's very close to zero.}
  if Result < 0.0 then
    Result := 0.0;
end;

function SLNDepreciation(Cost, Salvage: Extended; Life: Integer): Extended;
{ Spreads depreciation linearly over life. }
begin
  {$IFNDEF MATH_NOERR}
  if Life < 1 then ArgError('SLNDepreciation');
  {$ENDIF}
  Result := (Cost - Salvage) / Life
end;

function SYDDepreciation(Cost, Salvage: Extended; Life, Period: Integer): Extended;
{ SYD = (cost - salvage) * (life - period + 1) / (life*(life + 1)/2) }
{ Note: life*(life+1)/2 = 1+2+3+...+life "sum of years"
				The depreciation factor varies from life/sum_of_years in first period = 1
																			 downto  1/sum_of_years in last period = life.
				Total depreciation over life is cost-salvage.}
var
  X1, X2: Extended;
begin
  Result := 0;
	if (Period < 1) or (Life < Period) or (Cost <= Salvage) then Exit;
  X1 := 2 * (Life - Period + 1);
  X2 := Life * (Life + 1);
  Result := (Cost - Salvage) * X1 / X2
end;

{ Discounted cash flow functions. }

function InternalRateOfReturn(Guess: Extended; const CashFlows: array of Double): Extended;
{
Use Newton's method to solve NPV = 0, where NPV is a polynomial in
x = 1/(1+rate).  Split the coefficients into negative and postive sets:
  neg + pos = 0, so pos = -neg, so  -neg/pos = 1
Then solve:
  log(-neg/pos) = 0

  Let  t = log(1/(1+r) = -LnXP1(r)
  then r = exp(-t) - 1
Iterate on t, then use the last equation to compute r.
}
var
  T, Y: Extended;
  Poly: TPoly;
  K, Count: Integer;

  function ConditionP(const CashFlows: array of Double): Integer;
  { Guarantees existence and uniqueness of root.  The sign of payments
    must change exactly once, the net payout must be always > 0 for
    first portion, then each payment must be >= 0.
    Returns: 0 if condition not satisfied, > 0 if condition satisfied
    and this is the index of the first value considered a payback. }
  var
    X: Double;
    I, K: Integer;
  begin
    K := High(CashFlows);
    while (K >= 0) and (CashFlows[K] >= 0.0) do Dec(K);
    Inc(K);
    if K > 0 then
    begin
      X := 0.0;
      I := 0;
      while I < K do begin
	      X := X + CashFlows[I];
        if X >= 0.0 then
        begin
     	    K := 0;
      	  Break
    	  end;
      	Inc(I)
      end
    end;
    ConditionP := K
  end;

begin
  InternalRateOfReturn := 0;
  K := ConditionP(CashFlows);
  {$IFNDEF MATH_NOERR}
  if K < 0 then ArgError('InternalRateOfReturn');
  {$ENDIF}
  if K = 0 then
  begin
    {$IFNDEF MATH_NOERR}
    if Guess <= -1.0 then ArgError('InternalRateOfReturn');
    {$ENDIF}
    T := -LnXP1(Guess)
  end else
    T := 0.0;
  for Count := 1 to MaxIterations do
  begin
    PolyX(CashFlows, Exp(T), Poly);
    {$IFNDEF MATH_NOERR}
    if Poly.Pos <= Poly.Neg then ArgError('InternalRateOfReturn');
    {$ENDIF}
    if (Poly.Neg >= 0.0) or (Poly.Pos <= 0.0) then
    begin
      InternalRateOfReturn := -1.0;
      Exit;
    end;
    with Poly do
      Y := Ln(-Neg / Pos) / (DNeg / Neg - DPos / Pos);
    T := T - Y;
    if RelSmall(Y, T) then
    begin
      InternalRateOfReturn := Exp(-T) - 1.0;
      Exit;
    end
  end;
  {$IFNDEF MATH_NOERR}
  ArgError('InternalRateOfReturn');
  {$ENDIF}
end;

function NetPresentValue(Rate: Extended; const CashFlows: array of Double;
  PaymentTime: TPaymentTime): Extended;
{ Caution: The sign of NPV is reversed from what would be expected for standard
   cash flows!}
var
  rr: Extended;
  I: Integer;
begin
  {$IFNDEF MATH_NOERR}
  if Rate <= -1.0 then ArgError('NetPresentValue');
  {$ENDIF}
  rr := 1/(1+Rate);
  result := 0;
  for I := High(CashFlows) downto Low(CashFlows) do
    result := rr * result + CashFlows[I];
  if PaymentTime = ptEndOfPeriod then result := rr * result;
end;

{ Annuity functions. }

{---------------
From the point of view of A, amounts received by A are positive and
amounts disbursed by A are negative (e.g. a borrower's loan repayments
are regarded by the borrower as negative).

Given interest rate r, number of periods n:
  compound(r, n) = (1 + r)**n               "Compounding growth factor"
  annuity(r, n) = (compound(r, n)-1) / r   "Annuity growth factor"

Given future value fv, periodic payment pmt, present value pv and type
of payment (start, 1 , or end of period, 0) pmtTime, financial variables satisfy:

  fv = -pmt*(1 + r*pmtTime)*annuity(r, n) - pv*compound(r, n)

For fv, pv, pmt:

  C := compound(r, n)
  A := (1 + r*pmtTime)*annuity(r, n)
  Compute both at once in Annuity2.

  if C > 1E16 then A = C/r, so:
    fv := meaningless
    pv := -pmt*(pmtTime+1/r)
    pmt := -pv*r/(1 + r*pmtTime)
  else
    fv := -pmt(1+r*pmtTime)*A - pv*C
    pv := (-pmt(1+r*pmtTime)*A - fv)/C
    pmt := (-pv*C-fv)/((1+r*pmtTime)*A)
---------------}

function PaymentParts(Period, NPeriods: Integer; Rate, PresentValue,
  FutureValue: Extended; PaymentTime: TPaymentTime; var IntPmt: Extended):
  Extended;
var
		Crn:extended; { =Compound(Rate,NPeriods) }
		Crp:extended; { =Compound(Rate,Period-1) }
		Arn:extended; { =AnnuityF(Rate,NPeriods) }

begin
  {$IFNDEF MATH_NOERR}
  if Rate <= -1.0 then ArgError('PaymentParts');
  {$ENDIF}
  Crp:=Compound(Rate,Period-1);
  Arn:=Annuity2(Rate,NPeriods,PaymentTime,Crn);
  IntPmt:=(FutureValue*(Crp-1)-PresentValue*(Crn-Crp))/Arn;
  PaymentParts:=(-FutureValue-PresentValue)*Crp/Arn;
end;

function FutureValue(Rate: Extended; NPeriods: Integer; Payment, PresentValue:
  Extended; PaymentTime: TPaymentTime): Extended;
var
  Annuity, CompoundRN: Extended;
begin
  {$IFNDEF MATH_NOERR}
  if Rate <= -1.0 then ArgError('FutureValue');
  {$ENDIF}
  Annuity := Annuity2(Rate, NPeriods, PaymentTime, CompoundRN);
  {$IFNDEF MATH_NOERR}
  if CompoundRN > 1.0E16 then ArgError('FutureValue');
  {$ENDIF}
  FutureValue := -Payment * Annuity - PresentValue * CompoundRN
end;

function InterestPayment(Rate: Extended; Period, NPeriods: Integer; PresentValue,
  FutureValue: Extended; PaymentTime: TPaymentTime): Extended;
var
  Crp:extended; { compound(rate,period-1)}
  Crn:extended; { compound(rate,nperiods)}
  Arn:extended; { annuityf(rate,nperiods)}
begin
  {$IFNDEF MATH_NOERR}
  if (Rate <= -1.0)
    or (Period < 1) or (Period > NPeriods) then ArgError('InterestPayment');
  {$ENDIF}
  Crp:=Compound(Rate,Period-1);
  Arn:=Annuity2(Rate,Nperiods,PaymentTime,Crn);
  InterestPayment:=(FutureValue*(Crp-1)-PresentValue*(Crn-Crp))/Arn;
end;

function InterestRate(NPeriods: Integer;
  Payment, PresentValue, FutureValue: Extended; PaymentTime: TPaymentTime): Extended;
{
Given:
  First and last payments are non-zero and of opposite signs.
  Number of periods N >= 2.
Convert data into cash flow of first, N-1 payments, last with
first < 0, payment > 0, last > 0.
Compute the IRR of this cash flow:
  0 = first + pmt*x + pmt*x**2 + ... + pmt*x**(N-1) + last*x**N
where x = 1/(1 + rate).
Substitute x = exp(t) and apply Newton's method to
  f(t) = log(pmt*x + ... + last*x**N) / -first
which has a unique root given the above hypotheses.
}
var
  X, Y, Z, First, Pmt, Last, T, ET, EnT, ET1: Extended;
  Count: Integer;
  Reverse: Boolean;

  function LostPrecision(X: Extended): Boolean;
  asm
        XOR     EAX, EAX
        MOV     BX,WORD PTR X+8
        INC     EAX
        AND     EBX, $7FF0
        JZ      @@1
        CMP     EBX, $7FF0
        JE      @@1
        XOR     EAX,EAX
  @@1:
  end;

begin
  Result := 0;
  {$IFNDEF MATH_NOERR}
  if NPeriods <= 0 then ArgError('InterestRate');
  {$ENDIF}
  Pmt := Payment;
  if PaymentTime = ptEndOfPeriod then
  begin
    X := PresentValue;
    Y := FutureValue + Payment
  end
  else
  begin
    X := PresentValue + Payment;
    Y := FutureValue
  end;
  First := X;
  Last := Y;
  Reverse := False;
  if First * Payment > 0.0 then
  begin
    Reverse := True;
    T := First;
    First := Last;
    Last := T
  end;
  if first > 0.0 then
  begin
    First := -First;
    Pmt := -Pmt;
    Last := -Last
  end;
  {$IFNDEF MATH_NOERR}
  if (First = 0.0) or (Last < 0.0) then ArgError('InterestRate');
  {$ENDIF}
  T := 0.0;                     { Guess at solution }
  for Count := 1 to MaxIterations do
  begin
    EnT := Exp(NPeriods * T);
    if {LostPrecision(EnT)} ent=(ent+1) then
    begin
      Result := -Pmt / First;
      if Reverse then
        Result := Exp(-LnXP1(Result)) - 1.0;
      Exit;
    end;
    ET := Exp(T);
    ET1 := ET - 1.0;
    if ET1 = 0.0 then
    begin
      X := NPeriods;
      Y := X * (X - 1.0) / 2.0
    end
    else
    begin
      X := ET * (Exp((NPeriods - 1) * T)-1.0) / ET1;
      Y := (NPeriods * EnT - ET - X * ET) / ET1
    end;
    Z := Pmt * X + Last * EnT;
    Y := Ln(Z / -First) / ((Pmt * Y + Last * NPeriods *EnT) / Z);
    T := T - Y;
    if RelSmall(Y, T) then
    begin
      if not Reverse then T := -T;
      InterestRate := Exp(T)-1.0;
      Exit;
    end
  end;
  {$IFNDEF MATH_NOERR}
  ArgError('InterestRate');
  {$ENDIF}
end;

function NumberOfPeriods(Rate, Payment, PresentValue, FutureValue: Extended;
  PaymentTime: TPaymentTime): Extended;

{ If Rate = 0 then nper := -(pv + fv) / pmt
  else cf := pv + pmt * (1 + rate*pmtTime) / rate
       nper := LnXP1(-(pv + fv) / cf) / LnXP1(rate) }

var
  PVRPP: Extended; { =PV*Rate+Payment } {"initial cash flow"}
  T:     Extended;

begin
  {$IFNDEF MATH_NOERR}
  if Rate <= -1.0 then ArgError('NumberOfPeriods');
  {$ENDIF}

{whenever both Payment and PaymentTime are given together, the PaymentTime has the effect
 of modifying the effective Payment by the interest accrued on the Payment}

  if ( PaymentTime=ptStartOfPeriod ) then
    Payment:=Payment*(1+Rate);

{if the payment exactly matches the interest accrued periodically on the
 presentvalue, then an infinite number of payments are going to be
 required to effect a change from presentvalue to futurevalue.  The
 following catches that specific error where payment is exactly equal,
 but opposite in sign to the interest on the present value.  If PVRPP
 ("initial cash flow") is simply close to zero, the computation will
 be numerically unstable, but not as likely to cause an error.}

  PVRPP:=PresentValue*Rate+Payment;
  {$IFNDEF MATH_NOERR}
  if PVRPP=0 then ArgError('NumberOfPeriods');
  {$ENDIF}

  { 6.1E-5 approx= 2**-14 }
  if ( EAbs(Rate)<6.1E-5 ) then
    Result:=-(PresentValue+FutureValue)/PVRPP
  else
  begin

{starting with the initial cash flow, each compounding period cash flow
 should result in the current value approaching the final value.  The
 following test combines a number of simultaneous conditions to ensure
 reasonableness of the cashflow before computing the NPER.}

    T:= -(PresentValue+FutureValue)*Rate/PVRPP;
    {$IFNDEF MATH_NOERR}
    if  T<=-1.0  then ArgError('NumberOfPeriods');
    {$ENDIF}
    Result := LnXP1(T) / LnXP1(Rate)
  end;
  NumberOfPeriods:=Result;
end;

function Payment(Rate: Extended; NPeriods: Integer; PresentValue, FutureValue:
  Extended; PaymentTime: TPaymentTime): Extended;
var
  Annuity, CompoundRN: Extended;
begin
  {$IFNDEF MATH_NOERR}
  if Rate <= -1.0 then ArgError('Payment');
  {$ENDIF}
  Annuity := Annuity2(Rate, NPeriods, PaymentTime, CompoundRN);
  if CompoundRN > 1.0E16 then
    Payment := -PresentValue * Rate / (1 + Integer(PaymentTime) * Rate)
  else
    Payment := (-PresentValue * CompoundRN - FutureValue) / Annuity
end;

function PeriodPayment(Rate: Extended; Period, NPeriods: Integer;
  PresentValue, FutureValue: Extended; PaymentTime: TPaymentTime): Extended;
var
  Junk: Extended;
begin
  {$IFNDEF MATH_NOERR}
  if (Rate <= -1.0) or (Period < 1) or (Period > NPeriods) then ArgError('PeriodPayment');
  {$ENDIF}
  PeriodPayment := PaymentParts(Period, NPeriods, Rate, PresentValue,
       FutureValue, PaymentTime, Junk);
end;

function PresentValue(Rate: Extended; NPeriods: Integer; Payment, FutureValue:
  Extended; PaymentTime: TPaymentTime): Extended;
var
  Annuity, CompoundRN: Extended;
begin
  {$IFNDEF MATH_NOERR}
  if Rate <= -1.0 then ArgError('PresentValue');
  {$ENDIF}
  Annuity := Annuity2(Rate, NPeriods, PaymentTime, CompoundRN);
  if CompoundRN > 1.0E16 then
    PresentValue := -(Payment / Rate * Integer(PaymentTime) * Payment)
  else
    PresentValue := (-Payment * Annuity - FutureValue) / CompoundRN
end;

{------------------------------------------------------------------------------}

function IsPowerOf2( i: Integer ): Boolean; { Result = (i <> 0) and (i and (i-1) = 0); }
asm
  OR EAX,EAX
  JZ @@exit // 0 не является степенью числа 2
  LEA EDX, [EAX-1]
  OR  EAX,EDX
  SETZ AL   // число является степенью 2, если (i & (i-1)) = 0, т.е. если после
            // обнуления младшей 1 в числе больше не осталось битов 1.
@@exit:
end;

function Low1( i: Integer ): Integer; { Result := i and (-i); }
asm
  MOV EDX, EAX
  NEG EAX
  AND EAX, EDX 
end;

function Low0( i: Integer ): Integer; { Result := -i and (i+1); }
asm
  LEA EDX, [EAX+1]
  NEG EAX
  AND EAX, EDX
end;

function count_1_bits_in_byte( x: Byte ): Byte;
  asm
    MOV   CL, AL
@@loop:
    SHR   CL, 1
    JZ    @@exit
    SUB   AL, CL
    JMP   @@loop
@@exit:
  end;

function count_1_bits_in_dword( x: Integer ): Integer;
  asm
    MOV   ECX, EAX
    JMP   @@go
@@loop:
    SUB   EAX, ECX
@@go:
    SHR   ECX, 1
    JNZ   @@loop
  end;

end.
