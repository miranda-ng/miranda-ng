HistoryToDBUpdater
------------------

Утилита установки/обновления компонентов плагинов RnQHistoryToDB, QIPHistoryToDB, MirandaIMHistoryToDB, MirandaNGHistoryToDB.

Автор:		Михаил Григорьев
E-Mail: 	sleuthhound@gmail.com
ICQ: 		161867489
WWW:		http://www.im-history.ru
Лицензия:	GNU GPLv3

Системные требования:
---------------------
ОС:		Win2000/XP/2003/Vista/7
IM-клиент:	Любой
БД:		MySQL 4.0, 4.1, 5.0, 5.1
		PostgreSQL 7.1 - 8.3
		Oracle 8i - 11i
		SQLite 3
		Firebird 2.x

Необходимые компоненты для сборки плагина:
------------------------------------------

1. Embarcadero RAD Studio XE3

2. IM-History Downloader Component
   See also directory IMDownloaderComponent

3. DCPcrypt Cryptographic Component Library v2
   http://www.cityinthesky.co.uk/

Описание параметров запуска:
----------------------------

HistoryToDBUpdater.exe <1>

<1> - (Необязательный параметр) - Путь до файла настроек HistoryToDB.ini (Например: "C:\Program Files\QIP Infium\Profiles\username@qip.ru\Plugins\QIPHistoryToDB\")
