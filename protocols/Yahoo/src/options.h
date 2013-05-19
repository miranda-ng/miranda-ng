/*
 * $Id: options.h 8425 2008-10-15 16:02:48Z gena01 $
 *
 * myYahoo Miranda Plugin 
 *
 * Authors: Gennady Feldman (aka Gena01) 
 *          Laurent Marechal (aka Peorth)
 *
 * This code is under GPL and is based on AIM, MSN and Miranda source code.
 * I want to thank Robert Rainwater and George Hazan for their code and support
 * and for answering some of my questions during development of this plugin.
 */
#ifndef _YAHOO_SEARCH_H_
#define _YAHOO_SEARCH_H_

BOOL CALLBACK DlgProcYahooOpts(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam);
BOOL CALLBACK DlgProcYahooOptsConn(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam);
BOOL CALLBACK DlgProcYahooOptsIgnore(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam);
//BOOL CALLBACK DlgProcYahooPopupOpts(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam);
int YahooOptInit(WPARAM wParam,LPARAM lParam);

#endif
