#if !defined(AFX_MRA_ICONS_H__F58D13FF_F6F2_476C_B8F0_7B9E9357CF48__INCLUDED_)
#define AFX_MRA_ICONS_H__F58D13FF_F6F2_476C_B8F0_7B9E9357CF48__INCLUDED_


#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


HICON	IconLibGetIcon(HANDLE hIcon);
HICON	IconLibGetIconEx(HANDLE hIcon,DWORD dwFlags);

void	IconsLoad();
void	IconsUnLoad();

void	InitXStatusIcons();
void	DestroyXStatusIcons();


#endif // !defined(AFX_MRA_ICONS_H__F58D13FF_F6F2_476C_B8F0_7B9E9357CF48__INCLUDED_)