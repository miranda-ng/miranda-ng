#include "stdafx.h"
#include "../../libs/zlib/src/zip.h"

int CreateZipFile(const wchar_t *szDestPath, OBJLIST<ZipFile> &lstFiles, const std::function<bool(size_t)> &fnCallback)
{
	zipFile hZip = zipOpen2_64(szDestPath, APPEND_STATUS_CREATE, nullptr, nullptr);
	if (!hZip) return 1;

	zip_fileinfo fi = { 0 };

	int ret = 0;
	for (auto &zf : lstFiles) {
		HANDLE hSrcFile = CreateFileW(zf->sPath.c_str(), GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
		if (hSrcFile != INVALID_HANDLE_VALUE) {
			FILETIME ft, ft2;
			GetFileTime(hSrcFile, 0, &ft, 0);
			FileTimeToLocalFileTime(&ft, &ft2);

			uint16_t dosDate, dosTime;
			FileTimeToDosDateTime(&ft2, &dosDate, &dosTime);
			fi.dosDate = MAKELONG(dosTime, dosDate);

			int iOpenRes = zipOpenNewFileInZip(hZip, _T2A(zf->sZipPath.c_str()), &fi, nullptr, 0, nullptr, 0, "", Z_DEFLATED, Z_BEST_COMPRESSION);
			if (iOpenRes == ZIP_OK) {
				DWORD dwRead;
				uint8_t buf[0x40000];

				while (ReadFile(hSrcFile, buf, sizeof(buf), &dwRead, nullptr) && dwRead)
					if (zipWriteInFileInZip(hZip, buf, dwRead) != ZIP_OK)
						break;

				zipCloseFileInZip(hZip);
				CloseHandle(hSrcFile);

				if (!fnCallback(lstFiles.indexOf(&zf))) {
					ret = 3;
					break;
				}
			}
			else CloseHandle(hSrcFile);
		}
	}

	zipClose(hZip, CMStringA(FORMAT, Translate("Miranda NG [%s] database backup"), g_szMirVer));
	return ret;
}
