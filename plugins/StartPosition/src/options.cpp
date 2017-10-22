#include "stdafx.h"


StartPositionOptions::StartPositionOptions() :
    setTopPosition(MODULE_NAME, "CLEnableTop", 1),
    setBottomPosition(MODULE_NAME, "CLEnableBottom", 0),
    setSidePosition(MODULE_NAME, "CLEnableSide", 1),
    clistAlign(MODULE_NAME, "CLAlign", ClistAlign::right),
    setClistWidth(MODULE_NAME, "CLEnableWidth", 0),
    setClistStartState(MODULE_NAME, "CLEnableState", 0),
    clistState(MODULE_NAME, "CLState", ClistState::normal),
    pixelsFromTop(MODULE_NAME, "CLpixelsTop", 3),
    pixelsFromBottom(MODULE_NAME, "CLpixelsBottom", 3),
    pixelsFromSide(MODULE_NAME, "CLpixelsSide", 3),
    clistWidth(MODULE_NAME, "CLWidth", 180)
{}

extern StartPositionOptions spOptions;

COptionsDlg::COptionsDlg() :
    CPluginDlgBase(g_hInst, IDD_OPTIONS, MODULE_NAME),
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
    chkStartHidden(this, IDC_CLSTATETRAY),
    chkStartNormal(this, IDC_CLSTATEOPENED)
{
    CreateLink(chkPositionTop, spOptions.setTopPosition);
    CreateLink(chkPositionBottom, spOptions.setBottomPosition);
    CreateLink(chkPositionSide, spOptions.setSidePosition);
    CreateLink(chkWidth, spOptions.setClistWidth);
    CreateLink(chkStartState, spOptions.setClistStartState);

    CreateLink(edtPositionTop, spOptions.pixelsFromTop);
    CreateLink(edtPositionBottom, spOptions.pixelsFromBottom);
    CreateLink(edtPositionSide, spOptions.pixelsFromSide);
    CreateLink(edtWidth, spOptions.clistWidth);

    chkPositionTop.OnChange = Callback(this, &COptionsDlg::onCheck_PositionTop);
    chkPositionBottom.OnChange = Callback(this, &COptionsDlg::onCheck_PositionBottom);
    chkPositionSide.OnChange = Callback(this, &COptionsDlg::onCheck_PositionSide);
    chkWidth.OnChange = Callback(this, &COptionsDlg::onCheck_Width);
    chkStartState.OnChange = Callback(this, &COptionsDlg::onCheck_StartState);
}

void COptionsDlg::OnInitDialog()
{
    if (spOptions.clistState == ClistState::normal)
        chkStartNormal.SetState(true);
    else
        chkStartHidden.SetState(true);

    chkStartHidden.Enable(chkStartState.GetState());
    chkStartNormal.Enable(chkStartState.GetState());

    if (spOptions.clistAlign == ClistAlign::right)
        chkFromRight.SetState(true);
    else
        chkFromLeft.SetState(true);
        
    chkFromLeft.Enable(chkPositionSide.GetState());
    chkFromRight.Enable(chkPositionSide.GetState());

    edtPositionTop.Enable(chkPositionTop.GetState());
    edtPositionBottom.Enable(chkPositionBottom.GetState());
    edtPositionSide.Enable(chkPositionSide.GetState());
    edtWidth.Enable(chkWidth.GetState());
}

void COptionsDlg::OnApply()
{
    removeOldSettings();

    if (chkStartNormal.GetState())
        spOptions.clistState = ClistState::normal;
    else
        spOptions.clistState = ClistState::hidden;

    if (chkFromRight.GetState())
        spOptions.clistAlign = ClistAlign::right;
    else
        spOptions.clistAlign = ClistAlign::left;
}

void COptionsDlg::removeOldSettings()
{
    if (db_get_b(0, MODULE_NAME, "CLEnableTop", dbERROR) == dbERROR)
    {
        db_unset(0, MODULE_NAME, "CLEnable");
        db_unset(0, MODULE_NAME, "CLuseLastWidth");
    }
}

void COptionsDlg::onCheck_PositionTop(CCtrlCheck*)
{
    edtPositionTop.Enable(chkPositionTop.GetState());
}

void COptionsDlg::onCheck_PositionBottom(CCtrlCheck*)
{
    edtPositionBottom.Enable(chkPositionBottom.GetState());
}

void COptionsDlg::onCheck_PositionSide(CCtrlCheck*)
{
    edtPositionSide.Enable(chkPositionSide.GetState());
    chkFromLeft.Enable(chkPositionSide.GetState());
    chkFromRight.Enable(chkPositionSide.GetState());
}

void COptionsDlg::onCheck_Width(CCtrlCheck*)
{
    edtWidth.Enable(chkWidth.GetState());
}

void COptionsDlg::onCheck_StartState(CCtrlCheck*)
{
    chkStartHidden.Enable(chkStartState.GetState());
    chkStartNormal.Enable(chkStartState.GetState());
}

int OptInitialise(WPARAM wParam, LPARAM)
{
    OPTIONSDIALOGPAGE odp = {};
    odp.hInstance = g_hInst;
    odp.szGroup.a = LPGEN("Contact list");
    odp.szTitle.a = LPGEN("Start position");
    odp.pDialog = new COptionsDlg;
    odp.flags = ODPF_BOLDGROUPS;
    Options_AddPage(wParam, &odp);
    return 0;
}
