/*
Copyright (C) 2012-21 Miranda NG team (https://miranda-ng.org)

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation version 2
of the License.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program. If not, see <http://www.gnu.org/licenses/>.
*/

#pragma once

#include "stdafx.h"

enum ClistAlign : BYTE
{
    left,
    right
};

enum ClistState : BYTE
{
    hidden,
    minimized,
    normal
};

struct ClistOptions
{
    CMOption<BYTE> isDocked;
    CMOption<BYTE> state;

    CMOption<DWORD> x;
    CMOption<DWORD> y;
    CMOption<DWORD> width;
    CMOption<DWORD> height;

    ClistOptions() :
        isDocked(CLIST_MODULE_NAME, "Docked", 0),
        state(CLIST_MODULE_NAME, "State", ClistState::normal),
        x(CLIST_MODULE_NAME, "x", 0),
        y(CLIST_MODULE_NAME, "y", 0),
        width(CLIST_MODULE_NAME, "Width", 150),
        height(CLIST_MODULE_NAME, "Height", 350)
    { }
};

struct StartPositionOptions
{
    CMOption<BYTE> setTopPosition;
    CMOption<BYTE> setBottomPosition;
    CMOption<BYTE> setSidePosition;
    CMOption<BYTE> clistAlign;
    CMOption<BYTE> setClistWidth;
    CMOption<BYTE> setClistStartState;
    CMOption<BYTE> clistState;

    CMOption<DWORD> pixelsFromTop;
    CMOption<DWORD> pixelsFromBottom;
    CMOption<DWORD> pixelsFromSide;
    CMOption<DWORD> clistWidth;

    StartPositionOptions();
};

class COptionsDlg : public CDlgBase
{
    CCtrlCheck chkPositionTop, chkPositionBottom, chkPositionSide, chkFromLeft, chkFromRight, chkWidth;
    CCtrlEdit edtPositionTop, edtPositionBottom, edtPositionSide, edtWidth;
    CCtrlCheck chkStartState, chkStartHidden, chkStartNormal;

public:
    COptionsDlg();

    bool OnInitDialog() override;
    bool OnApply() override;

private:
    void removeOldSettings();

    void onCheck_PositionTop(CCtrlCheck*);
    void onCheck_PositionBottom(CCtrlCheck*);
    void onCheck_PositionSide(CCtrlCheck*);
    void onCheck_Width(CCtrlCheck*);
    void onCheck_StartState(CCtrlCheck*);
};
