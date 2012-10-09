{
	RSSNews plugin
	by Angelo Luiz Tartari
}

{$IFNDEF M_RSSNEWS}
{$DEFINE M_RSSNEWS}

const

(*
  Retrieve news.
  wParam = (WPARAM)(HANDLE)hContact
  lParam = 0
  Returns: 0 on success, -1 on error.
*)
  MS_RSSNEWS_GETNEWS = 'RSSNews/GetNews';

(*
  Retrieve news (all feeds).
  wParam = 0
  lParam = 0
  Returns: 0 on success, -1 on error.
*)
  MS_RSSNEWS_GETALLNEWS = 'RSSNews/GetAllNews';

(*
  Brings up the add new feed dialog.
  wParam = 0
  lParam = 0
  Returns: 0 on success, -1 on error.
*)
  MS_RSSNEWS_ADDNEWFEED = 'RSSNews/AddNewFeed';

(*
  Brings up the import dialog.
  wParam = 0
  lParam = 0
  Returns: 0 on success, -1 on error.
*)
  MS_RSSNEWS_IMPORT = 'RSSNews/Import';

(*
  Brings up the export dialog.
  wParam = 0
  lParam = 0
  Returns: 0 on success, -1 on error.
*)
  MS_RSSNEWS_EXPORT = 'RSSNews/Export';

{$ENDIF}
