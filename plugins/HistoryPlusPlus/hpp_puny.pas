// Punycode Kovertierung
//
// Punycode: A Bootstring encoding of Unicode for Internationalized Domain Names in Applications (IDNA)
// http://www.rfc-editor.org/rfc/rfc3492.txt
//
// Delphi-Unit von Daniel Mitte (2005)
// Original-Code von http://www.activevb.de
//
// Beispiel:
// pc := TPunyClass.Create;
// e := pc.Encode('müller'); // Verschlüsselt 'müller' zu 'mller-kva'
// d := pc.Decode(e); // Entschlüsselt 'mller-kva' zu 'müller'
// pc.Free;

unit hpp_puny;

interface

const
  BASE: Longint = 36;
  TMIN: Longint = 1;
  TMAX: Longint = 26;
  SKEW: Longint = 38;
  DAMP: Longint = 700;
  INITIAL_BIAS: Longint = 72;
  INITIAL_N: Longint = 128;
  Delimiter: String = '-';
  MAX_INT: Longint = 2147483647;

type
  TPunyClass = class
  private
    function GetMinCodePoint(const n: Longint; const data: String): Longint;
    function IsBasic(const c: String; const n: Longint): Boolean;
    function Adapt(const delta, numpoints: Longint; const firsttime: Boolean): Longint;
    function Digit2Codepoint(const d: Longint): Longint;
    function Codepoint2Digit(const c: Longint): Longint;
    function UInt(i: Longint): Longint;
    function Asc(const s: String): Longint;
    function AscW(const s: String): Longint;
    function PosRev(const sub, text: String): Longint;
  public
    function Encode(const input: String): String;
    function Decode(const input: String): String;
  end;

implementation

function TPunyClass.Encode(const input: String): String;
var
  n, delta, bias, b, l, h, q, m, k, t: Longint;
  text, output, c: String;
  first: Boolean;

begin
  text := input;
  output := '';

  try
    n := INITIAL_N;
    bias := INITIAL_BIAS;
    b := 0;
    
    for l := 1 to Length(text) do
    begin
      c := Copy(text, l, 1);

      if IsBasic(c, INITIAL_N) then
      begin
        output := output + c;
        b := b + 1;
      end;
    end;

    if Length(output) < Length(text) then
      if Length(output) > 0 then output := output + Delimiter;

    h := b;
    delta := 0;
    
    while h < Length(text) do
    begin
      m := GetMinCodePoint(n, text);
      delta := delta + UInt(m - n) * (h + 1);

      n := m;

      for l := 1 to Length(text) do
      begin
        c := Copy(text, l, 1);
        if IsBasic(c, n) then delta := delta + 1
        else if UInt(AscW(c)) = n then
        begin
          q := delta;

          k := BASE;
          while k <= MAX_INT do
          begin
            if k <= (bias + TMIN) then t := TMIN
            else if k >= (bias + TMAX) then t := TMAX
            else t := k - bias;

            if q < t then break;

            output := output + Chr(Digit2Codepoint(t + ((q - t) Mod (BASE - t))));
            q := (q - t) div (BASE - t);

            k := k + BASE;
          end;

          output := output + Chr(Digit2Codepoint(q));
          first := False;
          if h = b then first := True;
          bias := Adapt(delta, h + 1, first);
          delta := 0;
          h := h + 1;
        end;
      end;
      
      delta := delta + 1;
      n := n + 1;
    end;
  except
    output := input;
  end;

  Result := output;
end;

function TPunyClass.Decode(const input: String): String;
var
  n, i, bias, l, ps, oldi, w, k, t: Longint;
  digit: Byte;
  text, output, c: String;
  first: Boolean;

begin
  text := input;
  output := '';

  try
    n := INITIAL_N;
    bias := INITIAL_BIAS;
    i := 0;

    ps := PosRev(Delimiter, text);

    if ps > 0 then
    begin
      for l := 1 to (ps - 1) do
      begin
        c := Copy(text, l, 1);

        if IsBasic(c, INITIAL_N) then output := output + c
        else
        begin
          Result := '';
          Exit;
        end;
      end;
    end;

    ps := ps + 1;

    while ps <= Length(text) do
    begin
      oldi := i;
      w := 1;

      k := BASE;
      while ((k <= MAX_INT) and (ps <= Length(text))) do
      begin
        c := Copy(text, ps, 1);
        ps := ps + 1;

        digit := Codepoint2Digit(Asc(c));
        if ((digit >= BASE) or (digit > ((MAX_INT - i) / w))) then
        begin
          Result := '';
          Exit;
        end;

        i := i + digit * w;

        if k <= bias then t := TMIN
        else if k >= (bias + TMAX) then t := TMAX
        else t := k - bias;

        if digit < t then break;

        if w > (maxint / (BASE - t)) then
        begin
          Result := '';
          Exit;
        end;

        w := w * (BASE - t);

        k := k + BASE;
      end;

      first := False;
      if oldi = 0 then first := True;
      bias := Adapt(i - oldi, Length(output) + 1, first);

      if (i / (Length(output) + 1)) > (MAX_INT - n) then
      begin
        Result := '';
        Exit;
      end;

      n := n + i div (Length(output) + 1);
      i := i mod (Length(output) + 1);

      if IsBasic(Char(n), INITIAL_N) then
      begin
        Result := '';
        Exit;
      end;

      output := Copy(output, 1, i) + Char(n) + Copy(output, i + 1, Length(output) - (i + 1) + 1);
      i := i + 1;
    end;
  except
    output := input;
  end;

  Result := output;
end;

function TPunyClass.GetMinCodePoint(const n: Longint; const data: String): Longint;
var
  t, a, res: Longint;

begin
  res := 2147483647;

  for t := 1 to Length(data) do
  begin
    a := UInt(AscW(Copy(data, t, 1)));
    if ((a >= n) and (a < res)) then res := a;
  end;

  Result := res;
end;

function TPunyClass.IsBasic(const c: String; const n: Longint): Boolean;
begin
  Result := False;
  if UInt(AscW(c)) < n then Result := True;
end;

function TPunyClass.Adapt(const delta, numpoints: Longint; const firsttime: Boolean): Longint;
var
  k, dt: Longint;

begin
  dt := delta;

  if firsttime then dt := dt div DAMP
  else dt := dt div 2;

  dt := dt + (dt div numpoints);
  k := 0;

  while dt > (((BASE - TMIN) * TMAX) div 2) do
  begin
    dt := dt div (BASE - TMIN);
    k := k + BASE;
  end;

  Result := k + (((BASE - TMIN + 1) * dt) div (dt + SKEW));
end;

function TPunyClass.Digit2Codepoint(const d: Longint): Longint;
begin
  Result := 0;
  
  if d < 26 then Result := d + 97
  else if d < 36 then Result := d - 26 + 48;
end;

function TPunyClass.Codepoint2Digit(const c: Longint): Longint;
begin
  Result := BASE;

  if (c - 48) < 10 then Result := c - 22
  else if (c - 65) < 26 then Result := c - 65
  else if (c - 97) < 26 then Result := c - 97;
end;

function TPunyClass.UInt(i: Longint): Longint;
begin
  Result := i;
  if i < 0 then Result := 65536 + i;
end;

function TPunyClass.Asc(const s: String): Longint;
var
  c: Char;
begin
  Result := 0;

  if Length(s) > 0 then
  begin
    c := s[1];
    Result := Word(c);
  end;
end;

function TPunyClass.AscW(const s: String): Longint;
var
  c: Char;
begin
  Result := 0;

  if Length(s) > 0 then
  begin
    c := s[1];
    Result := Longint(c);
  end;
end;

function TPunyClass.PosRev(const sub, text: String): Longint;
var
  p: Longint;
  s: String;

begin
  Result := 0;

  s := '';
  for p := 1 to Length(text) do s := s + Copy(text, Length(text) - p + 1, 1);

  p := Pos(sub, s);
  if p > 0 then Result := Length(s) - p + 1;
end;

end.
