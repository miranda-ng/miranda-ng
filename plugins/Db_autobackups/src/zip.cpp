#include "stdafx.h"
#include "../../libs/zlib/src/zip.h"

int CreateZipFile(const wchar_t *szDestPath, OBJLIST<ZipFile> &lstFiles, const std::function<bool(size_t)> &fnCallback)
{
	zipFile hZip = zipOpen2_64(szDestPath, APPEND_STATUS_CREATE, nullptr, nullptr);
	if (!hZip) return 1;

	zip_fileinfo fi = { 0 };

	int ret = 0;
	for (int i = 0; i < lstFiles.getCount(); i++) {
		ZipFile &zf = lstFiles[i];

		HANDLE hSrcFile = CreateFileW(zf.sPath.c_str(), GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
		if (hSrcFile != INVALID_HANDLE_VALUE) {
			int iOpenRes = zipOpenNewFileInZip(hZip, _T2A(zf.sZipPath.c_str()), &fi, nullptr, 0, nullptr, 0, "", Z_DEFLATED, Z_BEST_COMPRESSION);

			if (iOpenRes == ZIP_OK) {
				DWORD dwRead;
				BYTE buf[0x40000];

				while (ReadFile(hSrcFile, buf, sizeof(buf), &dwRead, nullptr) && dwRead && (zipWriteInFileInZip(hZip, buf, dwRead) == ZIP_OK));
				zipCloseFileInZip(hZip);
				CloseHandle(hSrcFile);

				if (!fnCallback(i)) {
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
