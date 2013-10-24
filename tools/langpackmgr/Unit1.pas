unit Unit1;

interface

uses
  Winapi.Windows, Winapi.Messages, System.SysUtils, System.Variants,
  System.Classes, Vcl.Graphics, Vcl.Controls, Vcl.Forms, Vcl.Dialogs,
  Vcl.StdCtrls, Vcl.ExtCtrls, Vcl.Grids, ShellApi;

type
  TForm1 = class(TForm)
    Button1: TButton;
    Button2: TButton;
    Button3: TButton;
    Label1: TLabel;
    Label2: TLabel;
    Label3: TLabel;
    Memo1: TMemo;
    Memo2: TMemo;
    RadioGroup1: TRadioGroup;
    ComboBox1: TComboBox;
    StringGrid1: TStringGrid;

    procedure FormCreate(Sender: TObject);
    procedure Click(Sender: TObject);
    procedure Button1Click(Sender: TObject);
    procedure Button2Click(Sender: TObject);
    procedure Button3Click(Sender: TObject);
    procedure viewline;
    procedure RadioGroup1Click(Sender: TObject);
    procedure ComboBox1Change(Sender: TObject);
    procedure Draw(Sender: TObject; ACol, ARow: Integer; Rect: TRect;
      State: TGridDrawState);


  private
    { Private declarations }
  public
    { Public declarations }
  end;

var
  Form1: TForm1;
  openfile:textfile;
  lang,stmp: string;
  str: array [0..40,0..6000] of string;
  ffile:array [0..6000] of string;
  core: array [0..30] of string;
  l:array [0..6000] of integer;
  ns:array [0..6000] of boolean;
  ii,i,n:integer;


implementation

{$R *.dfm}

procedure TForm1.FormCreate(Sender: TObject);
var
  SR:TSearchRec;
  Res:Integer;
begin
  combobox1.Items.Clear;
  combobox1.Items.Add('=CORE=');
  Res:=FindFirst(ExtractFilePath(Application.ExeName)
  +'\english\plugins\*.txt', faAnyFile, SR);
  while Res = 0 do
        begin
        Form1.comboBox1.Items.Add('p|'+copy(extractfilename(SR.Name),
        1,length(extractfilename(SR.Name))-4));
        Res:=FindNext(SR);
        end;
  FindClose(SR);

  Res:=FindFirst(ExtractFilePath(Application.ExeName)
  +'\english\weather\*.txt', faAnyFile, SR);
  while Res = 0 do
        begin
        Form1.comboBox1.Items.Add('w|'+copy(extractfilename(SR.Name),
        1,length(extractfilename(SR.Name))-4));
        Res:=FindNext(SR);
        end;
  FindClose(SR);
  end;



procedure TForm1.Button2Click(Sender: TObject);

var str:string; i:integer;
 begin
 str:='http://translate.google.com/?hl=&ie=yhgg#auto/';
 if  form1.Caption='belarusian' then str:=str+'be/';
 if  form1.Caption='bulgarian' then str:=str+'bg/';
 if  form1.Caption='chinese' then str:=str+'zh-CN/';
 if  form1.Caption='czech' then str:=str+'cs/';
 if  form1.Caption='duth' then str:=str+'da/';
 if  form1.Caption='estonian' then str:=str+'et/';
 if  form1.Caption='french' then str:=str+'fr/';
 if  form1.Caption='german' then str:=str+'de/';
 if  form1.Caption='herbrew' then str:=str+'iw/';
 if  form1.Caption='hungarian' then str:=str+'hu/';
 if  form1.Caption='italian' then str:=str+'it/';
 if  form1.Caption='japanese' then str:=str+'ja/';
 if  form1.Caption='korean' then str:=str+'ko/';
 if  form1.Caption='norwegian' then str:=str+'no/';
 if  form1.Caption='polish' then str:=str+'pl/';
 if  form1.Caption='portuguese_br' then str:=str+'pt/';
 if  form1.Caption='russian' then str:=str+'ru/';
 if  form1.Caption='slovak' then str:=str+'sk/';
 if  form1.Caption='spanish' then str:=str+'es/';
 if  form1.Caption='turkish' then str:=str+'tr/';
 if  form1.Caption='ukrainian' then str:=str+'uk/';
 for i:=0 to memo1.Lines.Count-1 do
    begin
    str:=str+memo1.Lines[i];
    if i<memo1.Lines.Count-1 then
    str:=str+'+%0A+';
    end;
 ShellExecute(0, 'open',PChar(str), nil, nil, SW_SHOW);
end;

procedure TForm1.Button3Click(Sender: TObject);
  var fi:integer;
begin

  rewrite(openfile);
  writeln(openfile, str[0,0]);
  for fi:=1 to n do
    begin
    if copy(str[0,fi],1,1)=';' then
    writeln(openfile, str[0,fi]);
    if (copy(str[0,fi],1,1)='[') and (str[1,fi]<>'') then
       begin
        writeln(openfile, str[0,fi]);
        writeln(openfile, str[1,fi]);
       end;
    end;
  CloseFile(openfile);
end;

procedure TForm1.Button1Click(Sender: TObject);
begin
  memo2.Lines:=memo1.Lines;
end;

procedure TForm1.Click(Sender: TObject);
var sa:integer; da:string;
begin
  da:=str[1,l[ii]];
  str[1,l[ii]]:='';
 for sa:=0 to memo2.Lines.Count-1 do
begin
  str[1,l[ii]]:=str[1,l[ii]]+memo2.Lines[sa];
  if sa<>memo2.Lines.Count-1 then  str[1,l[ii]]:=str[1,l[ii]]+'\n';
end;
  if str[1,l[ii]]<>da then ns[ii]:=true;
memo1.Lines.clear;
memo2.Lines.clear;
viewline;
ii:=stringgrid1.Row;
stringgrid1.rowcount:=stringgrid1.rowcount+1;
stringgrid1.rowcount:=stringgrid1.rowcount-1;
end;


procedure tform1.viewline;
var im,m:integer;
untranslate,changes: integer;
sse:string;
begin
  m:=1;
  if RadioGroup1.itemindex=0
  then
  sse:=copy(str[0,l[stringgrid1.row]],2, length(str[0,l[stringgrid1.row]])-2)
  else
  sse:=str[2,l[stringgrid1.row]];
      for im := 1 to length(sse)-1 do
        begin
          if copy(sse,im,2)='\n' then
          begin
            memo1.Lines.Add(copy(sse,m,im-m));
            m:=im+2;
          end;
        end;
    if m=1 then  memo1.Lines.Add(sse)
    else memo1.Lines.Add(copy(sse,m,length(sse)-m+2));
   if str[1,l[stringgrid1.row]]<>'' then
    begin
      m:=1;
      for im := 1 to length(str[1,l[stringgrid1.row]])-1 do
        begin
          if copy(str[1,l[stringgrid1.row]],im,2)='\n' then
          begin
            memo2.Lines.Add(copy(str[1,l[stringgrid1.row]],m,im-m));
            m:=im+2;
          end;
        end;
    if m=1 then  memo2.Lines.Add(str[1,l[stringgrid1.row]])
    else memo2.Lines.Add(copy(str[1,l[stringgrid1.row]],m,
    length(str[1,l[stringgrid1.row]])-m+2));
    end;

 changes:=0;
 for  im:= 0 to stringgrid1.rowcount-1 do
 if ns[im]=true then changes:=changes+1;
 label2.Caption:='Changes:'+inttostr(changes);
 untranslate:=0;
 for  im:= 0 to stringgrid1.rowcount-1 do
 if str[1,l[im]]='' then untranslate:=untranslate+1;
 label3.Caption:='Untranslate:'+inttostr(untranslate);

end;


procedure TForm1.ComboBox1Change(Sender: TObject);

begin
   for i := 0 to 6000 do
     begin
     for n:=0 to 40 do str[n,i]:='';
     l[i]:=0;
     ns[i]:=false;
     memo1.Lines.Clear;
     memo2.Lines.Clear;
     end;
  if  ComboBox1.Text='=CORE=' then
   assignfile(openfile,ExtractFilePath(Application.ExeName)+
  '\english\=CORE=.txt',CP_UTF8);
  if copy(ComboBox1.Text,1,2)='w|' then
   assignfile(openfile,ExtractFilePath(Application.ExeName)+
  '\english\weather\'+copy(ComboBox1.Text,3,length(ComboBox1.Text)-2)+'.txt',CP_UTF8);
    if copy(ComboBox1.Text,1,2)='p|' then
   assignfile(openfile,ExtractFilePath(Application.ExeName)+
  '\english\plugins\'+copy(ComboBox1.Text,3,length(ComboBox1.Text)-2)+'.txt',CP_UTF8);
  reset(openfile);
  readln(openfile,stmp);
  str[0,0]:=stmp;
  stringgrid1.Rowcount:=0;
  n:=0;
while not eof(openfile) do
begin
  n:=n+1;
  readln(openfile,stmp);
  str[0,n]:=stmp;
  if copy(stmp,1,1)='[' then
  begin
  l[stringgrid1.Rowcount-1]:=n;
  stringgrid1.Cells[0,stringgrid1.Rowcount-1]:=stmp;
  stringgrid1.Rowcount:=stringgrid1.Rowcount+1;
  end;
end;
//////////////////////////////////////////////////////////

if RadioGroup1.Items.Count=2 then
begin
   if  ComboBox1.Text='=CORE=' then
   assignfile(openfile,ExtractFilePath(Application.ExeName)+
  '\'+RadioGroup1.Items[1]+'\=CORE=.txt',CP_UTF8);
  if copy(ComboBox1.Text,1,2)='w|' then
   assignfile(openfile,ExtractFilePath(Application.ExeName)+'\'
   +RadioGroup1.Items[1]+
   '\weather\'+copy(ComboBox1.Text,3,length(ComboBox1.Text)-2)+'.txt',CP_UTF8);
    if copy(ComboBox1.Text,1,2)='p|' then
   assignfile(openfile,ExtractFilePath(Application.ExeName)+'\'
   +RadioGroup1.Items[1]+
  '\plugins\'+copy(ComboBox1.Text,3,length(ComboBox1.Text)-2)+'.txt',CP_UTF8);
  reset(openfile);
    readln(openfile);
     n:=0;
  while not eof(openfile) do
begin
   readln(openfile,ffile[n]);
   n:=n+1;
end;
for i := 0 to stringgrid1.Rowcount do
for ii := 0 to n-1 do
if ffile[ii]=stringgrid1.cells[0,i] then
str[2,l[i]]:=ffile[ii+1];
end;
///////////////////////////////////////
//////////////////////////////////////////////////////////
  if  ComboBox1.Text='=CORE=' then
   assignfile(openfile,ExtractFilePath(Application.ExeName)+
  '\'+form1.Caption+'\=CORE=.txt',CP_UTF8);
  if copy(ComboBox1.Text,1,2)='w|' then
   assignfile(openfile,ExtractFilePath(Application.ExeName)+'\'+form1.Caption+
   '\weather\'+copy(ComboBox1.Text,3,length(ComboBox1.Text)-2)+'.txt',CP_UTF8);
    if copy(ComboBox1.Text,1,2)='p|' then
   assignfile(openfile,ExtractFilePath(Application.ExeName)+'\'+form1.Caption+
  '\plugins\'+copy(ComboBox1.Text,3,length(ComboBox1.Text)-2)+'.txt',CP_UTF8);
  reset(openfile);
    readln(openfile);
 n:=0;
  while not eof(openfile) do
begin
   readln(openfile,ffile[n]);
   n:=n+1;
end;
for i := 0 to stringgrid1.Rowcount do
for ii := 0 to n-1 do
if ffile[ii]=stringgrid1.cells[0,i] then
str[1,l[i]]:=ffile[ii+1];
////////////////////////////////////////////////////////////////


stringgrid1.Rowcount:=stringgrid1.Rowcount-1;
label1.Caption:='Strings:'+inttostr(stringgrid1.Rowcount);

ii:=0;
viewline;
end;

procedure TForm1.Draw(Sender: TObject; ACol, ARow: Integer; Rect: TRect;
  State: TGridDrawState);
Var
  StringGrid: TStringGrid;
  Can: TCanvas;
begin
  StringGrid := Sender as TStringGrid;
  Can := StringGrid.Canvas; Can.Font := StringGrid.Font;
  if (ARow >= StringGrid.FixedRows) and (ACol >= StringGrid.FixedCols)
  then Can.Brush.Color := StringGrid.Color
  else Can.Brush.Color := StringGrid.FixedColor;
  if str[1,l[ARow]]='' then Can.Brush.Color:=clRed;
  if ns[arow]=true then Can.Brush.Color:=clGreen;
  Can.FillRect(Rect);
  Can.TextOut(Rect.Left+2,Rect.Top+2, StringGrid.Cells[ACol, ARow]);
end;

procedure TForm1.RadioGroup1Click(Sender: TObject);
var m,im:integer;
sse:string;
begin
memo1.Lines.Clear;
  m:=1;
  if RadioGroup1.itemindex=0
  then
  sse:=copy(str[0,l[stringgrid1.row]],2, length(str[0,l[stringgrid1.row]])-2)
  else
  sse:=str[2,l[stringgrid1.row]];
   for im := 1 to length(sse)-1 do
        begin
          if copy(sse,im,2)='\n' then
          begin
            memo1.Lines.Add(copy(sse,m,im-m));
            m:=im+2;
          end;
        end;
    if m=1 then  memo1.Lines.Add(sse)
    else memo1.Lines.Add(copy(sse,m,length(sse)-m+2));
end;

end.
