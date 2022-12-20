//
// Copyright Aliaksei Levin (levlam@telegram.org), Arseny Smirnov (arseny30@gmail.com) 2014-2022
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
#pragma once

#include "td/utils/common.h"

#ifdef TD_PORT_WINDOWS

namespace td {

inline BOOL CreateDirectoryFromAppW(_In_ LPCWSTR lpPathName, _In_opt_ LPSECURITY_ATTRIBUTES lpSecurityAttributes) {
  return CreateDirectoryW(lpPathName, lpSecurityAttributes);
}

inline BOOL RemoveDirectoryFromAppW(_In_ LPCWSTR lpPathName) {
  return RemoveDirectoryW(lpPathName);
}

inline BOOL DeleteFileFromAppW(_In_ LPCWSTR lpFileName) {
  return DeleteFileW(lpFileName);
}

inline BOOL MoveFileExFromAppW(_In_ LPCWSTR lpExistingFileName, _In_ LPCWSTR lpNewFileName, _In_ DWORD dwFlags) {
  return MoveFileEx(lpExistingFileName, lpNewFileName, dwFlags);
}

inline HANDLE FindFirstFileExFromAppW(_In_ LPCWSTR lpFileName, _In_ FINDEX_INFO_LEVELS fInfoLevelId,
                                      _Out_writes_bytes_(sizeof(WIN32_FIND_DATAW)) LPVOID lpFindFileData,
                                      _In_ FINDEX_SEARCH_OPS fSearchOp, _Reserved_ LPVOID lpSearchFilter,
                                      _In_ DWORD dwAdditionalFlags) {
  return FindFirstFileEx(lpFileName, fInfoLevelId, lpFindFileData, fSearchOp, lpSearchFilter, dwAdditionalFlags);
}

}  // namespace td

#endif
