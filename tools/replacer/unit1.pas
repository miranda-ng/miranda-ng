unit Unit1;



interface

uses
  Classes, SysUtils, FileUtil, Forms, Controls, Graphics, Dialogs, StdCtrls,
  CheckLst;
{  IMAGE_FILE_LINE_NUMS_STRIPPED or IMAGE_FILE_LOCAL_SYMS_STRIPPED
  $WEAKLINKRTTI ON $RTTI EXPLICIT METHODS([]) PROPERTIES([]) FIELDS([]) }
type

  { TForm1 }

  TForm1 = class(TForm)
    Button1: TButton;
    Button2: TButton;
    CheckListBox1: TCheckListBox;
    Edit1: TEdit;
    Edit2: TEdit;
    procedure Button1Click(Sender: TObject);
    procedure Button2Click(Sender: TObject);

  private
    { private declarations }
  public
    { public declarations }
  end;

var

   Form1: TForm1;
  d:tsearchrec;
  a:TStringList;
  langs:TStringList;
  i,j:integer;
  s:string;
  r,t:integer;

implementation

{$R *.lfm}

{ TForm1 }

procedure TForm1.Button1Click(Sender: TObject);
begin
    CheckListBox1.Items.clear;
    langs:=TstringList.Create;
    r:=findfirst(extractfilePath(application.exename)+'*',faanyfile,d);
    while r = 0 do
      begin
        if ((d.attr and fadirectory)=fadirectory)
        and ((d.name='.')or(d.name='..')) then
          begin r:=findnext(d); continue; end;
        if ((d.attr and fadirectory)=fadirectory)
        and (fileexists(extractfilepath(application.exename)+
        '/'+d.name+'/=HEAD=.txt')) then
      begin
       langs.Add(d.name);
      end;
        r:=FindNext(d);
      end;
FindClose(d);


for i := 0 to langs.Count-1 do
begin
  a:=TstringList.Create;
  if fileexists(extractfilepath(application.exename)+langs[i]+'/'+'=CORE=.txt') then
    begin
      a.LoadFromFile(extractfilepath(application.exename)+langs[i]+'/'+'=CORE=.txt');
      for j := 0 to a.count-1 do
      if a[j]=edit1.text then
        begin
          CheckListBox1.Items.Add(langs[i]+'/'+'=CORE=.txt');
          break;
        end;

     end;
 a.Free;

 r:=FindFirst(ExtractFilePath(Application.ExeName)+'/'
+langs[i]+'/plugins/*.txt', faAnyFile, d);
 while r = 0 do
    begin
      a:=TstringList.Create;
      a.LoadFromFile(extractfilepath(application.exename)
      +'/'+langs[i]+'/'+'plugins/'+d.name);
      for j := 0 to a.count-1 do
        if a[j]=edit1.text then
          begin
            CheckListBox1.Items.Add(langs[i]+'/'+'plugins/'+d.name);
            break;
          end;
      a.free;
       r:=FindNext(d);
    end;
     FindClose(d);

r:=FindFirst(ExtractFilePath(Application.ExeName)+'/'
+langs[i]+'/weather/*.txt', faAnyFile, d);
 while r = 0 do
    begin
      a:=TstringList.Create;
      a.LoadFromFile(extractfilepath(application.exename)
      +'/'+langs[i]+'/'+'weather/'+d.name);
      for j := 0 to a.count-1 do
        if a[j]=edit1.text then
          begin
            CheckListBox1.Items.Add(langs[i]+'/'+'plugins/'+d.name);
            break;
          end;
      a.free;
       r:=FindNext(d);
    end;
     FindClose(d);

end;

for i := 0 to CheckListBox1.Count-1 do
 CheckListBox1.Checked[i]:=true;

end;

procedure TForm1.Button2Click(Sender: TObject);
begin
 for i := 0 to CheckListBox1.Count-1 do
 if CheckListBox1.Checked[i]=true then
 begin
    a:=TstringList.Create;
    a.LoadFromFile(extractfilepath(application.exename)
      +'/'+CheckListBox1.Items[i]);
    for j := 1 to a.Count-1 do
      if a[j]=edit1.text then  a[j]:=edit2.text;
    a.SaveToFile(extractfilepath(application.exename)
      +'/'+CheckListBox1.Items[i]);
 end;
end;

end.

