////////////////////////////////////////////////////////////////////////////////
// All code below is exclusively owned by author of Chess4Net - Pavel Perminov
// (packpaul@mail.ru, packpaul1@gmail.com).
// Any changes, modifications, borrowing and adaptation are a subject for
// explicit permition from the owner.

unit GlobalsLocalUnit;

// Модуль для глобальных переменных и констант версии для Миранды

interface

uses
  Graphics;

const
  CHESS4NET = 'Chess4Net';
  CHESS4NET_VERSION = 201100; // 2011.0
  CHESS4NET_TITLE = 'Chess4Net 2011.0 (http://chess4net.ru)';
  MSG_INVITATION = 'Wellcome to Chess4Net. If you don''t have it, please download it from http://chess4net.ru';
  PROMPT_HEAD = 'Ch4N';
  MSG_DATA_SEPARATOR = '&&';

  PLUGIN_NAME = CHESS4NET;
  PLUGIN_VERSION = CHESS4NET_VERSION;
  PLUGIN_PLAYING_OVER = 'Plugin for playing chess over Miranda';
  PLUGIN_INFO_NAME = 'Chess4Net 2011.0.0';
  PLUGIN_URL = 'http://chess4net.ru';
  PLUGIN_EMAIL = 'packpaul@mail.ru';

var
  Chess4NetIcon, pluginIcon: TIcon;

implementation

end.
