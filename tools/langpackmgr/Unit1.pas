unit unit1;

interface

uses
  classes, sysutils, fileutil, forms, controls, graphics, dialogs, stdctrls,
  extctrls, Windows;

type

  { tform1 }

  tform1 = class(tform)
    button1: tbutton;
    button2: tbutton;
    button3: tbutton;
    button4: tbutton;
    Label1: TLabel;
    memo1: tmemo;
    memo2: tmemo;
    listbox: tlistbox;
    combobox1: tcombobox;
    combobox2: tcombobox;
    radiogroup1: tradiogroup;
    procedure Button1Click(Sender: TObject);
    procedure Button2Click(Sender: TObject);
    procedure Button3Click(Sender: TObject);
    procedure Button4Click(Sender: TObject);
    procedure combobox1change(sender: tobject);
    procedure ComboBox2Change(Sender: TObject);
    procedure formcreate(sender: tobject);
    procedure listbox1click(sender: tobject);
    procedure listbox2click(sender: tobject);
    procedure stringview;
    procedure savestring;
    procedure stringlist;

  private
    { private declarations }
  public
    { public declarations }
  end;

var
  form1: tform1;

  view,locale: string;
  trlang,trline: TStringList;
  z,j,i,l:integer;
  sr:tsearchrec;
  res:integer;
  s:array[0..1,1..6000] of string;
  u:array[0..6000] of integer;
  first:string;
  filename:string;
  m:integer;
  sse:string;
implementation

{$r *.lfm}

{ tform1 }

procedure tform1.formcreate(sender: tobject);
  var sr:tsearchrec; res:integer;
begin
view:='english';
trlang:=TstringList.Create;
trline:=TstringList.Create;
 if (paramstr(1)='-r') then
    begin
         form1.caption:='Miranda NG Langpack Tools: Replacer';
    end
    else
    begin
         res:=findfirst(extractfilepath(application.exename)+'*',faanyfile,sr);
         while res = 0 do
               begin
                    if ((sr.attr and fadirectory)=fadirectory)
                    and ((sr.name='.')or(sr.name='..')) then
                    begin res:=findnext(sr); continue; end;
                    if ((sr.attr and fadirectory)=fadirectory)
                    and (fileexists(extractfilepath(application.exename)+
                    '/'+sr.name+'/=head=.txt')) and (sr.name<>'english') then
                    begin
                    ListBox.items.add(sr.name);
                    trlang.add(sr.name);
                    end;
                    res:=findnext(sr);
               end;
         SysUtils.FindClose(sr);

    end;
end;

procedure tform1.stringlist;
 var llist:TStringList;
  begin
  for i := 1 to 6000 do
         begin s[0,i]:='';s[1,i]:='';end;
       j:=0;
   lList:=TstringList.Create;
       lList.LoadFromFile(extractfilePath(application.exename)+'/english/'
       +combobox1.items[combobox1.itemindex]+'.txt');
       first:=lList[0];
       for i := 1 to lList.Count-1 do
           begin
             j:=j+1;
             s[0,j]:=lList[i];
           end;
          lList.LoadFromFile(extractfilePath(application.exename)+'/'
     +locale+'/'+combobox1.items[combobox1.itemindex]+'.txt');
       for z:= 1 to j do
         for i := 1 to lList.count-2 do
           if (copy(s[0,z],1,1)='[')and(s[0,z]=lList[i])
           and (copy(lList[i+1],1,1)<>'[')then
             s[1,z]:=lList[i+1];
       lList.free;
       form1.Caption:='Miranda NG Langpack Tools: Editor -'
       +locale+'\'+combobox1.items[combobox1.itemindex];
       filename:=combobox1.items[combobox1.itemindex];
       listbox.items.Clear;
       z:=0;
       for i := 1 to j do
         if ((copy(s[0,i],1,1)='[')and(s[1,i]='') and (radiogroup1.ItemIndex=0))
         or ((copy(s[0,i],1,1)='[')and (radiogroup1.ItemIndex=1))
         then
           begin
             u[z]:=i;
             z:=z+1;
             listbox.items.add(copy(s[0,i],2,length(s[0,i])-2));
         end;
  end;

procedure tform1.stringview;
  begin
   for i:=0 to combobox2.items.count-1 do
      if view=combobox2.Items[i] then
        combobox2.itemindex:=i;
    sse:=trline[combobox2.ItemIndex];
    m:=1;
    for i := 1 to length(sse)-1 do
        begin
          if copy(sse,i,2)='\n' then
          begin
            memo1.Lines.Add(copy(sse,m,i-m));
            m:=i+2;
          end;
        end;
    memo2.Lines.Clear;
    l:=listbox.itemindex;
    if m=1 then  memo1.Lines.Add(sse)
    else memo1.Lines.Add(copy(sse,m,length(sse)-m+2));
   if s[1,u[listbox.itemindex]]<>'' then
    begin
      m:=1;
      for i := 1 to length(s[1,u[listbox.itemindex]])-1 do
        begin
          if copy(s[1,u[listbox.itemindex]],i,2)='\n' then
          begin
            memo2.Lines.Add(copy(s[1,u[listbox.itemindex]],m,i-m));
            m:=i+2;
          end;
        end;
    if m=1 then  memo2.Lines.Add(s[1,u[listbox.itemindex]])
    else memo2.Lines.Add(copy(s[1,u[listbox.itemindex]],m,
    length(s[1,u[listbox.itemindex]])-m+2));
    end;
  end;

procedure tform1.savestring;
var sa:integer;
    begin
         s[1,u[l]]:='';
         for sa:=0 to memo2.Lines.Count-1 do
             begin
                  s[1,u[l]]:=s[1,u[l]]+memo2.Lines[sa];
                  if sa<>memo2.Lines.Count-1 then  s[1,u[l]]:=s[1,u[l]]+'\n';
             end;
    end;

procedure tform1.combobox1change(sender: tobject);
  var f:textfile;
begin
  if not FileExists(extractfilePath(application.exename)
      +'/'+locale+'/'+combobox1.items[combobox1.itemindex]+'.txt')
  then
      begin
           assignfile(f,extractfilePath(application.exename)
           +'/'+locale+'/'+combobox1.items[combobox1.itemindex]+'.txt');
           rewrite(f);
           closefile(f);
      end;
  stringlist;
  memo1.lines.clear;
  memo2.lines.clear;
end;

procedure tform1.ComboBox2Change(Sender: TObject);
begin
   view:=combobox2.Text;
  if (listbox.items.count>0) and (combobox1.items.count>0) then
       begin
     savestring;
     memo1.lines.Clear;
     memo2.lines.Clear;
     stringview;
       end;
end;

procedure tform1.Button1Click(Sender: TObject);
  var sList:TStringList;
begin
     savestring;
     memo1.lines.Clear;
     memo2.lines.Clear;
     sList:=TstringList.Create;
     sList.add(first);
     for i := 0 to j do
         begin
              if copy(s[0,i],1,1)=';' then sList.add(s[0,i]);
              if s[1,i]<>'' then
         begin
            sList.add(s[0,i]);
            sList.add(s[1,i]);
          end
        else
        if (copy(s[0,i],1,1)='[')and
        (fileexists(extractfilePath(application.exename)+'clearstring.ini'))then
          begin
          sList.add(s[0,i]);
          sList.add('');
          end;
      end;
  sList.SavetoFile(extractfilePath(application.exename)
  +'/'+locale+'/'+filename+'.txt');
  sList.free;
  stringlist;
end;

procedure tform1.Button2Click(Sender: TObject);
  var search:TStringList; q:integer;
begin
     search:=TstringList.Create;
     search.LoadFromFile(extractfilePath(application.exename)+
     '/'+locale+'/'+'Langpack_'+locale+'.txt');
     for q := 0 to listbox.items.Count-1 do
         for i := 1 to search.count-1 do
             if (s[0,u[q]]=search[i])and(copy(search[i+1],1,1)<>'[')then
                begin
                     s[1,u[q]]:=search[i+1];
                     break;
                end;
     search.free;
end;

procedure tform1.Button3Click(Sender: TObject);
begin
     memo2.lines:=memo1.lines;
end;

procedure tform1.Button4Click(Sender: TObject);
  var str:widestring; i:integer;
   begin
   str:='http://translate.google.com/?hl=&ie=yhgg#auto/';
   if  locale='belarusian' then str:=str+'be/';
   if  locale='bulgarian' then str:=str+'bg/';
   if  locale='chinese' then str:=str+'zh-CN/';
   if  locale='czech' then str:=str+'cs/';
   if  locale='dutch' then str:=str+'da/';
   if  locale='estonian' then str:=str+'et/';
   if  locale='french' then str:=str+'fr/';
   if  locale='german' then str:=str+'de/';
   if  locale='hebrew' then str:=str+'iw/';
   if  locale='hungarian' then str:=str+'hu/';
   if  locale='italian' then str:=str+'it/';
   if  locale='japanese' then str:=str+'ja/';
   if  locale='korean' then str:=str+'ko/';
   if  locale='norwegian' then str:=str+'no/';
   if  locale='polish' then str:=str+'pl/';
   if  locale='portuguese_br' then str:=str+'pt/';
   if  locale='russian' then str:=str+'ru/';
   if  locale='slovak' then str:=str+'sk/';
   if  locale='spanish' then str:=str+'es/';
   if  locale='turkish' then str:=str+'tr/';
   if  locale='ukrainian' then str:=str+'uk/';
   for i:=0 to memo1.Lines.Count-1 do
      begin
           str:=str+memo1.Lines[i];
           if i<memo1.Lines.Count-1 then
              str:=str+'+%0A+';
      end;
   ShellExecute(0, 'open',PChar(str), nil, nil, SW_SHOW);
end;

procedure tform1.ListBox1Click(sender: tobject);
var search:TStringList; r:integer;
begin
   if (form1.caption<>'Miranda NG Langpack Tools: Editor')
   then
      begin
        savestring;
               search:=tstringlist.Create;
               combobox2.items.clear;
               trline.Clear;
               combobox2.items.add('english');
               trline.add(copy(s[0,u[listbox.itemindex]],2,
               length(s[0,u[listbox.itemindex]])-2));
               for i:=0 to trlang.count-1 do
               begin
               if (fileexists(extractfilepath(application.exename)+
                    '/'+trlang[i]+'/'+filename+'.txt'))
               and(trlang[i]<>locale) then
                  begin
                    search.LoadFromFile(extractfilepath(application.exename)+
                    '/'+trlang[i]+'/'+filename+'.txt');
                   for r:=1 to search.Count-2 do
                    if (search[r]=s[0,u[listbox.itemindex]])
                    and (search[r+1]<>'')
                    and (search[r+1]<>'[')
                    and (search[r+1]<>';')
                    then begin
                    combobox2.items.add(trlang[i]);
                    trline.add(search[r+1]);
                    break;
                    end;
                  end;
               end;
               search.free;
               combobox2.itemindex:=0;
        memo1.lines.clear;
        memo2.lines.clear;
        stringview;
      end;
end;

procedure tform1.listbox2click(sender: tobject);
  var sr:tsearchrec; res:integer;
begin
  if form1.caption='Miranda NG Langpack Tools: Editor'
  then
    begin
      locale:=ListBox.items[listBox.itemindex];
      form1.caption:=form1.caption+': '+locale;
      ListBox.items.clear;
      combobox1.items.add('=CORE=');
      res:=findfirst(extractfilepath(application.exename)
      +'\english\plugins\*.txt', faanyfile, sr);
      while res = 0 do
            begin
                 combobox1.items.add('plugins\'+copy(extractfilename(sr.name),1,
                 length(extractfilename(sr.name))-4));
                 res:=findnext(sr);
            end;
      SysUtils.FindClose(sr);
      res:=findfirst(extractfilepath(application.exename)
      +'\english\weather\*.txt', faanyfile, sr);
      while res = 0 do
            begin
                 combobox1.items.add('weather\'+copy(extractfilename(sr.name),1,
                 length(extractfilename(sr.name))-4));
                 res:=findnext(sr);
            end;
      SysUtils.FindClose(sr);
      combobox1.itemindex:=0;
      stringlist;
      if (fileexists(extractfilePath(application.exename)+'/'+locale+'/'
      +'Langpack_'+locale+'.txt'))  then button2.Enabled:=true;
    end;

end;

end.

