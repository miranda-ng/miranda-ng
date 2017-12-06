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

COptionsDlg::COptionsDlg(StartPositionPlugin* instance) :
    CPluginDlgBase(g_hInst, IDD_OPTIONS, MODULE_NAME),
    m_plugin(instance),
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
    CreateLink(chkPositionTop, m_plugin->spOptions.setTopPosition);
    CreateLink(chkPositionBottom, m_plugin->spOptions.setBottomPosition);
    CreateLink(chkPositionSide, m_plugin->spOptions.setSidePosition);
    CreateLink(chkWidth, m_plugin->spOptions.setClistWidth);
    CreateLink(chkStartState, m_plugin->spOptions.setClistStartState);

    CreateLink(edtPositionTop, m_plugin->spOptions.pixelsFromTop);
    CreateLink(edtPositionBottom, m_plugin->spOptions.pixelsFromBottom);
    CreateLink(edtPositionSide, m_plugin->spOptions.pixelsFromSide);
    CreateLink(edtWidth, m_plugin->spOptions.clistWidth);

    chkPositionTop.OnChange = Callback(this, &COptionsDlg::onCheck_PositionTop);
    chkPositionBottom.OnChange = Callback(this, &COptionsDlg::onCheck_PositionBottom);
    chkPositionSide.OnChange = Callback(this, &COptionsDlg::onCheck_PositionSide);
    chkWidth.OnChange = Callback(this, &COptionsDlg::onCheck_Width);
    chkStartState.OnChange = Callback(this, &COptionsDlg::onCheck_StartState);
}

void COptionsDlg::OnInitDialog()
{
    if (m_plugin->spOptions.clistState == ClistState::normal)
        chkStartNormal.SetState(true);
    else
        chkStartHidden.SetState(true);

    chkStartHidden.Enable(chkStartState.GetState());
    chkStartNormal.Enable(chkStartState.GetState());

    if (m_plugin->spOptions.clistAlign == ClistAlign::right)
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
        m_plugin->spOptions.clistState = ClistState::normal;
    else
        m_plugin->spOptions.clistState = ClistState::hidden;

    if (chkFromRight.GetState())
        m_plugin->spOptions.clistAlign = ClistAlign::right;
    else
        m_plugin->spOptions.clistAlign = ClistAlign::left;
}

void COptionsDlg::removeOldSettings()
{
    m_plugin->delSetting("CLEnable");
    m_plugin->delSetting("CLuseLastWidth");
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
