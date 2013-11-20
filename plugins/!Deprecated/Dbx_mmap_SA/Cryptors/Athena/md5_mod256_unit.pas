unit md5_mod256_unit;
interface

Uses
   MD5_Unit, sysutils, classes, windows;
Const
   MSize = 256;
Type
   tiAr = array [0..255] of integer;

Function MD5_Mod(s: String): String;
Function MD5_Matrix(s: string): tArr;
Function str_back(s: String): String;

implementation
//==============================================================================
Function str_back(s: String): String;
Var
   i: integer;
Begin
   result := '';
   for i := Length(s) downto 1 do result := result + s[i];
end;
//==============================================================================
Function MD5_Mod_back(s: String): String;
Var
   s1, s2 : String;
   k      : word;
Begin
   s1 := str_back(s)+s;
   s2 := md5(str_back(s)+str_back(md5(s))+
             str_back(md5(s+s))+
             str_back(md5(s+s+s))+
             str_back(md5(s+s+s+s))+
             str_back(md5(s+s+s+'asddsa'+s)));
   For k:=1 to trunc(sqrt(MSize))-1 do
   Begin
      s1 := md5(s1 + s2 + md5(s1+s1+s2+s) + md5(s2+s2+s1+s) + s);
      s2 := str_back(s2 + str_back(md5(s1+s2+md5(s))));
   End;
   result:=s2;
end;
//==============================================================================
Function MD5_Mod(s: String): String;
Var
   s1, s2 : String;
   k      : word;
Begin
   s1 := s + AnsiUpperCase(s) + AnsiLowerCase(s);
   s2 := md5(s+md5(s)+md5(s+s)+md5(s+AnsiUpperCase(s)+s)+md5(s+AnsiLowerCase(s+s)+s)+md5(s+s+str_back(s+s+'qweewq')));
   For k:=1 to trunc(sqrt(MSize))-1 do
   Begin
      s1 := md5(s1 + str_back(s2) + md5(s1+s1+s2+s) + str_back(md5(s2+s2+s1+s)) + s);
      s2 := str_back(s2 + md5(s1+AnsiUpperCase(s2+md5(s))));
   End;
   result:=s2;
end;
//==============================================================================

//==============================================================================
end.



