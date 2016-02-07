[LangOptions]
LanguageName=<0420><0443><0441><0441><043A><0438><0439>
LanguageID=$0419
LanguageCodePage=1251

[CustomMessages]
; *** "UpdateForm form"
cmWaiting=Ожидание...
cmFileNotFound=Архивы не найдены.
cmBinariesPath=Укажите путь к исполняемому файлу Miranda32/64.exe:
cmFilter=Файл Miranda??.exe (Miranda??.exe)|Miranda??.exe
cmSelectArchive=Выберите архив из списка:
cmSelectResources=Выберите ресурсы:
cmSelectResButtonCaption1a=Выбрать все
cmSelectResButtonCaption1b=Снять все
cmSelectResButtonCaption1c=Сбросить
cmPatchButtonCaption=Патчить
cmStatusLabelCaption1a=Обработка: %1
cmStatusLabelCaption1b=Готово

; *** "Logs"
cmLogFindResource7ZipFile1a=Поиск архивов с ресурсами начат
cmLogFindResource7ZipFile1b=Поиск архивов с ресурсами завершен
cmLogResource7ZipFile=Найден архив: %1
cmLogResource7ZipFileContent=Список файлов архива: %1
cmLogSelectedBinaryFile=Выбран исполняемый файл: %1
cmLogSelectedResource7ZipFile=Выбран файл с ресурсами: %1
cmLogUpdateResources1a=Обновление ресурсов начато
cmLogUpdateResources1b=Обновление ресурсов завершено
cmLogExtractResource7ZipFile=Распаковка ресурсов для файла %1.* из %2
cmLogUpdateFileResourcesStatus1a=Обновление ресурсов в файле %1: успех
cmLogUpdateFileResourcesStatus1b=Обновление ресурсов в файле %1: ошибка
cmLogUpdateFileResourcesStatus1c=Обновление ресурсов: пропуск. Ни один из файлов %1 не найден
cmLogResultCode=Код возврата: %1

; *** "Help Message"
cmHelpMsgCaption=Синтаксис командной строки %1
cmHelpMsg=Использование:%n  %1 [ /? | /bin="exepath" | /arc="arcname" | /res=reslist|* ]%n%nГде%n  /? - Отобразить это справочное сообщение%n  exepath - Путь к файлу Miranda32/64.exe%n  arcname - Имя 7z-архива c ресурсами без расширения%n  reslist - Список ресурсов (корневых папок), разделенных через запятую