unit CplxMath;
{* This unit contains functins for working with complex numbers. To use with
   KOL library and its kolmath.pas unit instead of standard math.pas, define
   synmbol KOL in project options, or uncomment its definition below. }

interface

//{$DEFINE KOL}

{$IFNDEF KOL}
  {$IFDEF KOL_MCK}
    {$DEFINE KOL}
  {$ENDIF}
{$ENDIF}

uses {$IFDEF KOL} kolmath, kol {$ELSE} math, sysutils {$ENDIF};

type
  {$IFDEF CPLX_EXTENDED}
    Double = Extended;
  {$ENDIF}

  Complex = record Re, Im: double end;
  {* }

  function CfromReIm( Re, Im: Double ): Complex;
  {* Re + i * Im }

  function Cadd( const X, Y: Complex ): Complex;
  {* X + Y }

  function Cneg( const X: Complex ): Complex;
  {* -X }

  function Csub( const X, Y: Complex ): Complex;
  {* X - Y }

  function Cmul( const X, Y: Complex ): Complex;
  {* X * Y }

  function CmulD( const X: Complex; D: Double ): Complex;
  {* X * D }

  function CmulI( const X: Complex ): Complex;
  {* i * X }

  function Cdiv( const X, Y: Complex ): Complex;
  {* X / Y }

  function Cmod( const X: Complex ): Double;
  {* Q( X.Re^2 + X.Im^2 ) }

  function Carg( const X: Complex ): Double;
  {* arctg( X.Im / X.Re ) }

  function CfromModArg( R, Arg: Double ): Complex;
  {* R * ( cos Arg + i * sin Arg ) }

  function Cpow( const X: Complex; Pow: Double ): Complex;
  {* X ^ Pow }

  function Cpower( const X, Pow: Complex ): Complex;
  {* X ^ Pow }

  function CIntPower( const X: Complex; Pow: Integer ): Complex;
  {* X ^ Pow}

  function Csqrt( const X: Complex ): Complex;
  {* Q( X ) }

  function Cexp( const X: Complex ): Complex;
  {* exp( X ) }

  function Cln( const X: Complex ): Complex;
  {* ln( X ) }

  function Ccos( const X: Complex ): Complex;
  {* cos( X ) }

  function Csin( const X: Complex ): Complex;
  {* sin( X ) }

  function C2Str( const X: Complex ): String;
  {* }

  function C2StrEx( const X: Complex ): String;
  {* experimental }

implementation

  function CfromReIm( Re, Im: Double ): Complex;
  begin
    Result.Re := Re;
    Result.Im := Im;
  end;

  function Cadd( const X, Y: Complex ): Complex;
  begin
    Result.Re := X.Re + Y.Re;
    Result.Im := X.Im + Y.Im;
  end;

  function Cneg( const X: Complex ): Complex;
  begin
    Result.Re := -X.Re;
    Result.Im := -X.Im;
  end;

  function Csub( const X, Y: Complex ): Complex;
  begin
    Result := Cadd( X, Cneg( Y ) );
  end;

  function Cmul( const X, Y: Complex ): Complex;
  begin
    Result.Re := X.Re * Y.Re - X.Im * Y.Im;
    Result.Im := X.Re * Y.Im + X.Im * Y.Re;
  end;

  function CmulD( const X: Complex; D: Double ): Complex;
  begin
    Result.Re := X.Re * D;
    Result.Im := X.Im * D;
  end;

  function CmulI( const X: Complex ): Complex;
  begin
    Result.Re := -X.Im;
    Result.Im := X.Re;
  end;

  function Cdiv( const X, Y: Complex ): Complex;
  var Z: Double;
  begin
    Z := 1.0 / ( Y.Re * Y.Re + Y.Im * Y.Im );
    Result.Re := (X.Re * Y.Re + X.Im * Y.Im ) * Z;
    Result.Im := (X.Im * Y.Re - X.Re * Y.Im ) * Z;
  end;

  function Cmod( const X: Complex ): Double;
  begin
    Result := sqrt( X.Re * X.Re + X.Im * X.Im );
  end;

  function Carg( const X: Complex ): Double;
  begin
    Result := ArcTan2( X.Im, X.Re );
  end;

  function CfromModArg( R, Arg: Double ): Complex;
  begin
    Result.Re := R * cos( Arg );
    Result.Im := R * sin( Arg );
  end;

  function Cpow( const X: Complex; Pow: Double ): Complex;
  var R, A: Double;
  begin
    R := power( Cmod( X ), Pow );
    A := Pow * Carg( X );
    Result := CfromModArg( R, A );
  end;

  function Cpower( const X, Pow: Complex ): Complex;
  begin
    Result := Cexp( Cmul( X, Cln( Pow ) ) );
  end;

  function CIntPower( const X: Complex; Pow: Integer ): Complex;
  begin
    if (Pow < 0) or (Pow > 100) then Result := Cpow( X, Pow )
    else if Pow = 0 then
    begin
      Result.Re := 1;
      Result.Im := 0;
    end
      else
    begin
      Result := X;
      while Pow > 1 do
      begin
        Result := Cmul( Result, X );
        dec( Pow );
      end;
    end;
  end;

  function Csqrt( const X: Complex ): Complex;
  begin
    Result := Cpow( X, 0.5 );
  end;

  function Cexp( const X: Complex ): Complex;
  var Z: Double;
  begin
    Z := exp( X.Re );
    Result.Re := Z * cos( X.Im );
    Result.Im := Z * sin( X.Im );
  end;

  function Cln( const X: Complex ): Complex;
  begin
    Result := CfromModArg( ln( Cmod( X ) ), Carg( X ) );
  end;

  function Ccos( const X: Complex ): Complex;
  begin
    Result := CmulI( X );
    Result := CmulD( Cadd( Cexp( Result ), Cexp( Cneg( Result ) ) ),
                     0.5 );
  end;

  function Csin( const X: Complex ): Complex;
  begin
    Result := CmulI( X );
    Result := CmulD( Csub( Cexp(Result), Cexp( Cneg(Result) ) ),
                     0.5 );
  end;

  {$IFDEF KOL}
  function Abs( X: Double ): Double;
  begin
    Result := EAbs( X );
  end;
  {$ENDIF}

  {$IFNDEF KOL}
  function Double2Str( D: Double ): String;
  begin
    Result := DoubleToStr( D );
  end;
  {$ENDIF}

  function C2Str( const X: Complex ): String;
  begin
    if Abs( X.Im ) < 1e-307 then
    begin
      Result := Double2Str( X.Re );
    end
      else
    begin
      Result := '';
      if Abs( X.Re ) > 1e-307 then
      begin
        Result := Double2Str( X.Re );
        if X.Im > 0.0 then
          Result := Result + ' + ';
      end;
      if X.Im < 0.0 then
        Result := Result + '- i * ' + Double2Str( -X.Im )
      else
        Result := Result + 'i * ' + Double2Str( X.Im );
    end;
  end;

  function C2StrEx( const X: Complex ): String;
  begin
    if Abs( X.Im ) < 1e-307 then
    begin
      Result := Double2StrEx( X.Re );
    end
      else
    begin
      Result := '';
      if Abs( X.Re ) > 1e-307 then
      begin
        Result := Double2StrEx( X.Re );
        if X.Im > 0.0 then
          Result := Result + ' + ';
      end;
      if X.Im < 0.0 then
        Result := Result + '- i * ' + Double2StrEx( -X.Im )
      else
        Result := Result + 'i * ' + Double2StrEx( X.Im );
    end;
  end;

end.
