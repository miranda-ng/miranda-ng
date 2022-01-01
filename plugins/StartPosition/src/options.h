/*
Copyright (C) 2012-22 Miranda NG team (https://miranda-ng.org)

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

enum ClistAlign : uint8_t
{
    left,
    right
};

enum ClistState : uint8_t
{
    hidden,
    minimized,
    normal
};

struct ClistOptions
{
    CMOption<uint8_t> isDocked;
    CMOption<uint8_t> state;

    CMOption<uint32_t> x;
    CMOption<uint32_t> y;
    CMOption<uint32_t> width;
    CMOption<uint32_t> height;

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
    CMOption<uint8_t> setTopPosition;
    CMOption<uint8_t> setBottomPosition;
    CMOption<uint8_t> setSidePosition;
    CMOption<uint8_t> clistAlign;
    CMOption<uint8_t> setClistWidth;
    CMOption<uint8_t> setClistStartState;
    CMOption<uint8_t> clistState;

    CMOption<uint32_t> pixelsFromTop;
    CMOption<uint32_t> pixelsFromBottom;
    CMOption<uint32_t> pixelsFromSide;
    CMOption<uint32_t> clistWidth;

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
