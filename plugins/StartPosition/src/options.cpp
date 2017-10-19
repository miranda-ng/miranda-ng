#include "stdafx.h"


class COptionsDlg : public CDlgBase
{
    CCtrlCheck chkPositionTop, chkPositionBottom, chkPositionSide, chkFromLeft, chkFromRight, chkWidth;
    CCtrlEdit edtPositionTop, edtPositionBottom, edtPositionSide, edtWidth;
    CCtrlCheck chkStartState, chkStartMinimized, chkStartOpened;

public:
    COptionsDlg() :
        CDlgBase(g_hInst, IDD_OPTIONS),
        chkPositionTop(this, IDC_CLTOPENABLE),
        edtPositionTop(this, IDC_CLTOP),
        chkPositionBottom(this, IDC_CLBOTTOMENABLE),
        edtPositionBottom(this, IDC_CLBOTTOM),
        chkPositionSide(this, IDC_CLSIDEENABLE),
        edtPositionSide(this, IDC_CLSIDE),
        chkFromLeft(this, IDC_CLALIGNLEFT),
        chkFromRight(this, IDC_CLALIGNRIGHT),
        chkWidth(this, IDC_CLWIDTHENABLE),
        edtWidth(this, IDC_CLWIDTH),
        chkStartState(this, IDC_CLSTATEENABLE),
        chkStartMinimized(this, IDC_CLSTATETRAY),
        chkStartOpened(this, IDC_CLSTATEOPENED)
    {
        chkPositionTop.OnChange = Callback(this, &COptionsDlg::onCheck_PositionTop);
        chkPositionBottom.OnChange = Callback(this, &COptionsDlg::onCheck_PositionBottom);
        chkPositionSide.OnChange = Callback(this, &COptionsDlg::onCheck_PositionSide);
        chkWidth.OnChange = Callback(this, &COptionsDlg::onCheck_Width);
        chkStartState.OnChange = Callback(this, &COptionsDlg::onCheck_StartState);
    }

    virtual void OnInitDialog() override
    {
        if (db_get_b(0, MODULE_NAME, "CLState", 2))
            chkStartOpened.SetState(true);
        else
            chkStartMinimized.SetState(true);

        chkStartState.SetState(db_get_b(0, MODULE_NAME, "CLEnableState", 0));
        chkStartMinimized.Enable(chkStartState.GetState());
        chkStartOpened.Enable(chkStartState.GetState());

        if (db_get_b(0, MODULE_NAME, "CLAlign", RIGHT))
            chkFromRight.SetState(true);
        else
            chkFromLeft.SetState(true);

        chkPositionSide.SetState(true);
        chkFromLeft.Enable(chkPositionSide.GetState());
        chkFromRight.Enable(chkPositionSide.GetState());

        chkPositionTop.SetState(db_get_b(0, MODULE_NAME, "CLEnableTop", 1));
        chkPositionBottom.SetState(db_get_b(0, MODULE_NAME, "CLEnableBottom", 0));
        chkWidth.SetState(db_get_b(0, MODULE_NAME, "CLEnableWidth", 0));

        edtPositionTop.SetInt(db_get_dw(0, MODULE_NAME, "CLpixelsTop", 3));
        edtPositionTop.Enable(chkPositionTop.GetState());

        edtPositionBottom.SetInt(db_get_dw(0, MODULE_NAME, "CLpixelsBottom", 3));
        edtPositionBottom.Enable(chkPositionBottom.GetState());

        edtPositionSide.SetInt(db_get_dw(0, MODULE_NAME, "CLpixelsSide", 3));
        edtPositionSide.Enable(chkPositionSide.GetState());

        edtWidth.SetInt(db_get_dw(0, MODULE_NAME, "CLWidth", 180));
        edtWidth.Enable(chkWidth.GetState());
    }

    virtual void OnApply() override
    {
        removeOldSettings();

        if (chkStartOpened.GetState())
            db_set_b(0, MODULE_NAME, "CLState", 2);
        else
            db_set_b(0, MODULE_NAME, "CLState", 0);

        if (chkFromLeft.GetState())
            db_set_b(0, MODULE_NAME, "CLAlign", LEFT);
        else
            db_set_b(0, MODULE_NAME, "CLAlign", RIGHT);

        db_set_b(0, MODULE_NAME, "CLEnableState", static_cast<BYTE>(chkStartState.GetState()));

        db_set_b(0, MODULE_NAME, "CLEnableTop", static_cast<BYTE>(chkPositionTop.GetState()));
        db_set_b(0, MODULE_NAME, "CLEnableBottom", static_cast<BYTE>(chkPositionBottom.GetState()));
        db_set_b(0, MODULE_NAME, "CLEnableSide", static_cast<BYTE>(chkPositionSide.GetState()));
        db_set_b(0, MODULE_NAME, "CLEnableWidth", static_cast<BYTE>(chkWidth.GetState()));
        db_set_dw(0, MODULE_NAME, "CLWidth", edtWidth.GetInt());
        db_set_dw(0, MODULE_NAME, "CLpixelsTop", edtPositionTop.GetInt());
        db_set_dw(0, MODULE_NAME, "CLpixelsBottom", edtPositionBottom.GetInt());
        db_set_dw(0, MODULE_NAME, "CLpixelsSide", edtPositionSide.GetInt());
    }

private:
    void removeOldSettings()
    {
        if (db_get_b(0, MODULE_NAME, "CLEnableTop", dbERROR) == dbERROR)
        {
            db_unset(0, MODULE_NAME, "CLEnable");
            db_unset(0, MODULE_NAME, "CLuseLastWidth");
        }
    }

    void onCheck_PositionTop(CCtrlCheck*)
    {
        edtPositionTop.Enable(chkPositionTop.GetState());
    }

    void onCheck_PositionBottom(CCtrlCheck*)
    {
        edtPositionBottom.Enable(chkPositionBottom.GetState());
    }

    void onCheck_PositionSide(CCtrlCheck*)
    {
        edtPositionSide.Enable(chkPositionSide.GetState());
        chkFromLeft.Enable(chkPositionSide.GetState());
        chkFromRight.Enable(chkPositionSide.GetState());
    }

    void onCheck_Width(CCtrlCheck*)
    {
        edtWidth.Enable(chkWidth.GetState());
    }

    void onCheck_StartState(CCtrlCheck*)
    {
        chkStartMinimized.Enable(chkStartState.GetState());
        chkStartOpened.Enable(chkStartState.GetState());
    }

};

int OptInitialise(WPARAM wParam, LPARAM)
{
    OPTIONSDIALOGPAGE odp = { };
    odp.hInstance = g_hInst;
    odp.szGroup.a = LPGEN("Contact list");
    odp.szTitle.a = LPGEN("Start position");
    odp.pDialog = new COptionsDlg;
    odp.flags = ODPF_BOLDGROUPS;
    Options_AddPage(wParam, &odp);
    return 0;
}
