<?php

include_once 'const.php';

function mbot_load()
{
  mb_SchReg("ex/1min","* * *",'every_1min',1,1);
}

function every_1min()
{
  $songinfo = 0;
  if(mb_SysCallService(MS_WAT_GETMUSICINFO, WAT_INF_ANSI,
     mb_SysGetPointer($songinfo))!=WAT_PLS_NORMAL)
    return 0;

  $artist_pos = mb_SysGetNumber($songinfo+wato_artist, 4);
  $artist = mb_SysGetString($artist_pos);

  $title_pos = mb_SysGetNumber($songinfo+wato_title, 4);
  $title = mb_SysGetString($title_pos);

  $newtext = "mp3: $artist - $title";

  mb_MsgBox($newtext, "MBot", 4);
  return 0;
}
?>