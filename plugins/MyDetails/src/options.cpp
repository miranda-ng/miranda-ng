/*
Copyright (C) 2005 Ricardo Pescuma Domenecci

This is free software; you can redistribute it and/or
modify it under the terms of the GNU Library General Public
License as published by the Free Software Foundation; either
version 2 of the License, or (at your option) any later version.

This is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Library General Public License for more details.

You should have received a copy of the GNU Library General Public
License along with this file; see the file license.txt.  If
not, write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
Boston, MA 02111-1307, USA.
*/


#include "stdafx.h"

// Prototypes /////////////////////////////////////////////////////////////////////////////////////

Options opts;

// Functions //////////////////////////////////////////////////////////////////////////////////////

static OptPageControl pageControls[] = {
	{ &opts.cycle_through_protocols, CONTROL_CHECKBOX, IDC_CYCLE_THROUGH_PROTOS, "CicleThroughtProtocols", (uint8_t)1 },
	{ &opts.seconds_to_show_protocol, CONTROL_SPIN, IDC_CYCLE_TIME, "CicleTime", (uint16_t)5, IDC_CYCLE_TIME_SPIN, (uint16_t)1, (uint16_t)255 },
	{ &opts.draw_show_protocol_name, CONTROL_CHECKBOX, IDC_SHOW_PROTO_NAME, "ShowProtocolName", (uint8_t)1 },
	{ &opts.show_protocol_cycle_button, CONTROL_CHECKBOX, IDC_SHOW_CYCLE_PROTO_BUTTON, "ShowProtocolCycleButton", (uint8_t)0 },
	{ &opts.draw_text_rtl, CONTROL_CHECKBOX, IDC_TEXT_RTL, "TextRTL", (uint8_t)0 },
	{ &opts.draw_text_align_right, CONTROL_CHECKBOX, IDC_TEXT_ALIGN_RIGHT, "TextAlignRight", (uint8_t)0 },
	{ &opts.replace_smileys, CONTROL_CHECKBOX, IDC_REPLACE_SMILEYS, "ReplaceSmileys", (uint8_t)1 },
	{ &opts.resize_smileys, CONTROL_CHECKBOX, IDC_RESIZE_SMILEYS, "ResizeSmileys", (uint8_t)0 },
	{ &opts.use_contact_list_smileys, CONTROL_CHECKBOX, IDC_USE_CONTACT_LIST_SMILEYS, "UseContactListSmileys", (uint8_t)0 },
	{ &opts.global_on_avatar, CONTROL_CHECKBOX, IDC_GLOBAL_ON_AVATAR, "GlobalOnAvatar", (uint8_t)0 },
	{ &opts.global_on_nickname, CONTROL_CHECKBOX, IDC_GLOBAL_ON_NICKNAME, "GlobalOnNickname", (uint8_t)0 },
	{ &opts.global_on_status, CONTROL_CHECKBOX, IDC_GLOBAL_ON_STATUS, "GlobalOnStatus", (uint8_t)0 },
	{ &opts.global_on_status_message, CONTROL_CHECKBOX, IDC_GLOBAL_ON_STATUS_MESSAGE, "GlobalOnStatusMessage", (uint8_t)0 },
	{ &opts.draw_avatar_allow_to_grow, CONTROL_CHECKBOX, IDC_AVATAR_ALLOW_TO_GROW, "AvatarAllowToGrow", (uint8_t)0 },
	{ &opts.draw_avatar_custom_size, CONTROL_CHECKBOX, IDC_AVATAR_CUSTOM_SIZE_CHK, "AvatarCustomSize", (uint8_t)0 },
	{ &opts.draw_avatar_custom_size_pixels, CONTROL_SPIN, IDC_AVATAR_CUSTOM_SIZE, "AvatarCustomSizePixels", (uint16_t)30, IDC_AVATAR_CUSTOM_SIZE_SPIN, (uint16_t)1, (uint16_t)255 },
	{ &opts.draw_avatar_border, CONTROL_CHECKBOX, IDC_AVATAR_DRAW_BORDER, "AvatarDrawBorders", (uint8_t)0 },
	{ &opts.draw_avatar_round_corner, CONTROL_CHECKBOX, IDC_AVATAR_ROUND_CORNERS, "AvatarRoundCorners", (uint8_t)1 },
	{ &opts.draw_avatar_use_custom_corner_size, CONTROL_CHECKBOX, IDC_AVATAR_CUSTOM_CORNER_SIZE_CHECK, "AvatarUseCustomCornerSize", (uint8_t)0 },
	{ &opts.draw_avatar_custom_corner_size, CONTROL_SPIN, IDC_AVATAR_CUSTOM_CORNER_SIZE, "AvatarCustomCornerSize", (uint16_t)4, IDC_AVATAR_CUSTOM_CORNER_SIZE_SPIN, (uint16_t)1, (uint16_t)255 },
	{ &opts.use_avatar_space_to_draw_text, CONTROL_CHECKBOX, IDC_AVATAR_USE_FREE_SPACE, "AvatarUseFreeSpaceToDrawText", (uint8_t)1 },
	{ &opts.resize_frame, CONTROL_CHECKBOX, IDC_RESIZE_FRAME, "ResizeFrame", (uint8_t)0 },
	{ &opts.use_skinning, CONTROL_CHECKBOX, IDC_USE_SKINNING, "UseSkinning", (uint8_t)0 },
	{ &opts.borders[RIGHT], CONTROL_SPIN, IDC_BORDER_RIGHT, "BorderRight", (uint16_t)8, IDC_BORDER_RIGHT_SPIN, (uint16_t)0, (uint16_t)255 },
	{ &opts.borders[LEFT], CONTROL_SPIN, IDC_BORDER_LEFT, "BorderLeft", (uint16_t)8, IDC_BORDER_LEFT_SPIN, (uint16_t)0, (uint16_t)255 },
	{ &opts.borders[TOP], CONTROL_SPIN, IDC_BORDER_TOP, "BorderTop", (uint16_t)8, IDC_BORDER_TOP_SPIN, (uint16_t)0, (uint16_t)255 },
	{ &opts.borders[BOTTOM], CONTROL_SPIN, IDC_BORDER_BOTTOM, "BorderBottom", (uint16_t)8, IDC_BORDER_BOTTOM_SPIN, (uint16_t)0, (uint16_t)255 },
};


// Initializations needed by options
void LoadOptions()
{
	LoadOpts(pageControls, _countof(pageControls), MODULENAME);

	// This is created here to assert that this key always exists
	opts.refresh_status_message_timer = db_get_w(0, "MyDetails", "RefreshStatusMessageTimer", 12);
	db_set_w(0, "MyDetails", "RefreshStatusMessageTimer", opts.refresh_status_message_timer);

	SetCycleTime();
	RefreshFrameAndCalcRects();
}

class COptDialog : public CDlgBase
{
	CCtrlCheck m_chkTextRTL, m_chkTextAlignRight, m_chkResizeFrame, m_chkUseCLSmilies, m_chkReplaceSmilies, m_chkResizeSmilies, m_chkGlobalAvatar, m_chkGlobalNickname, m_chkGlobalStatus,
		m_chkGlobalStatusMessage, m_chkShowProtoName, m_chkShowCycleProtoBtn, m_chkCycleProtos, m_chkAvatarCustomSize, m_chkAvatarGrow, m_chkAvatarBorder, m_chkAvatarRound, m_chkAvatarCustomCorn,
		m_chkAvatarFreeSpace;
	CCtrlEdit m_edtCycleTime, m_edtAvatarCustomSize, m_edtAvatarCustomCorn, m_edtBorderTop, m_edtBorderBottom, m_edtBorderLeft, m_edtBorderRight;
	CCtrlSpin m_spnCycleTime, m_spnAvatarCustomSize, m_spnAvatarCustomCorn, m_spnBorderTop, m_spnBorderBottom, m_spnBorderLeft, m_spnBorderRight;
public:
	COptDialog() :
		CDlgBase(g_plugin, IDD_OPTS),
		m_chkTextRTL(this, IDC_TEXT_RTL),
		m_chkTextAlignRight(this, IDC_TEXT_ALIGN_RIGHT),
		m_chkResizeFrame(this, IDC_RESIZE_FRAME),
		m_chkUseCLSmilies(this, IDC_USE_CONTACT_LIST_SMILEYS),
		m_chkReplaceSmilies(this, IDC_REPLACE_SMILEYS),
		m_chkResizeSmilies(this, IDC_RESIZE_SMILEYS),
		m_chkGlobalAvatar(this, IDC_GLOBAL_ON_AVATAR),
		m_chkGlobalNickname(this, IDC_GLOBAL_ON_NICKNAME),
		m_chkGlobalStatus(this, IDC_GLOBAL_ON_STATUS),
		m_chkGlobalStatusMessage(this, IDC_GLOBAL_ON_STATUS_MESSAGE),
		m_chkShowProtoName(this, IDC_SHOW_PROTO_NAME),
		m_chkShowCycleProtoBtn(this, IDC_SHOW_CYCLE_PROTO_BUTTON),
		m_chkCycleProtos(this, IDC_CYCLE_THROUGH_PROTOS),
		m_edtCycleTime(this, IDC_CYCLE_TIME),
		m_spnCycleTime(this, IDC_CYCLE_TIME_SPIN),
		m_chkAvatarCustomSize(this, IDC_AVATAR_CUSTOM_SIZE_CHK),
		m_edtAvatarCustomSize(this, IDC_AVATAR_CUSTOM_SIZE),
		m_spnAvatarCustomSize(this, IDC_AVATAR_CUSTOM_SIZE_SPIN),
		m_chkAvatarGrow(this, IDC_AVATAR_ALLOW_TO_GROW),
		m_chkAvatarBorder(this, IDC_AVATAR_DRAW_BORDER),
		m_chkAvatarRound(this, IDC_AVATAR_ROUND_CORNERS),
		m_chkAvatarCustomCorn(this, IDC_AVATAR_CUSTOM_CORNER_SIZE_CHECK),
		m_edtAvatarCustomCorn(this, IDC_AVATAR_CUSTOM_CORNER_SIZE),
		m_spnAvatarCustomCorn(this, IDC_AVATAR_CUSTOM_CORNER_SIZE_SPIN),
		m_chkAvatarFreeSpace(this, IDC_AVATAR_USE_FREE_SPACE),
		m_edtBorderTop(this, IDC_BORDER_TOP),
		m_spnBorderTop(this, IDC_BORDER_TOP_SPIN),
		m_edtBorderBottom(this, IDC_BORDER_BOTTOM),
		m_spnBorderBottom(this, IDC_BORDER_BOTTOM_SPIN),
		m_edtBorderLeft(this, IDC_BORDER_LEFT),
		m_spnBorderLeft(this, IDC_BORDER_LEFT_SPIN),
		m_edtBorderRight(this, IDC_BORDER_RIGHT),
		m_spnBorderRight(this, IDC_BORDER_RIGHT_SPIN)
	{
		m_chkAvatarRound.OnChange = Callback(this, &COptDialog::OnAvatarRoundChange);
		m_chkAvatarCustomCorn.OnChange = Callback(this, &COptDialog::OnAvatarCustomCornChange);
		m_chkShowProtoName.OnChange = Callback(this, &COptDialog::OnShowProtoNameChange);
		m_chkAvatarCustomSize.OnChange = Callback(this, &COptDialog::OnAvatarCustomSizeChange);
		m_chkCycleProtos.OnChange = Callback(this, &COptDialog::OnCycleProtosChange);
		m_chkReplaceSmilies.OnChange = Callback(this, &COptDialog::OnReplaceSmiliesChange);
	}

	bool OnInitDialog() override
	{
		if (!m_chkAvatarRound.IsChecked())
			m_chkAvatarCustomCorn.Disable();

		if (!m_chkAvatarRound.IsChecked() || !m_chkAvatarCustomCorn.IsChecked()) {
			m_edtAvatarCustomCorn.Disable();
			m_spnAvatarCustomCorn.Disable();
		}

		if (!m_chkShowProtoName.IsChecked())
			m_chkShowCycleProtoBtn.Disable();

		if (!m_chkAvatarCustomSize.IsChecked()) {
			m_edtAvatarCustomSize.Disable();
			m_spnAvatarCustomSize.Disable();
		}

		if (!m_chkCycleProtos.IsChecked()) {
			m_edtCycleTime.Disable();
			m_spnCycleTime.Disable();
		}

		if (!m_chkReplaceSmilies.IsChecked()) {
			m_chkUseCLSmilies.Disable();
			m_chkResizeSmilies.Disable();
		}

		if (!ServiceExists(MS_SMILEYADD_BATCHPARSE)) {
			m_chkReplaceSmilies.Disable();
			m_chkUseCLSmilies.Disable();
			m_chkResizeSmilies.Disable();
		}

		if (!ServiceExists(MS_CLIST_FRAMES_SETFRAMEOPTIONS))
			m_chkResizeFrame.Disable();

		return true;
	}

	bool OnApply() override
	{
		LoadOptions();
		return true;
	}

	void OnAvatarRoundChange(CCtrlBase*)
	{
		bool enabled = m_chkAvatarRound.IsChecked();
		m_chkAvatarCustomCorn.Enable(enabled);
		enabled = enabled && m_chkAvatarCustomCorn.IsChecked();
		m_edtAvatarCustomCorn.Enable(enabled);
		m_spnAvatarCustomCorn.Enable(enabled);
	}

	void OnAvatarCustomCornChange(CCtrlBase*)
	{
		bool enabled = m_chkAvatarRound.IsChecked();
		m_chkAvatarCustomCorn.Enable(enabled);
		enabled = enabled && m_chkAvatarCustomCorn.IsChecked();
		m_edtAvatarCustomCorn.Enable(enabled);
		m_spnAvatarCustomCorn.Enable(enabled);
	}

	void OnShowProtoNameChange(CCtrlBase*)
	{
		bool enabled = m_chkShowProtoName.IsChecked();
		m_chkShowCycleProtoBtn.Enable(enabled);
	}

	void OnAvatarCustomSizeChange(CCtrlBase*)
	{
		bool enabled = m_chkAvatarCustomSize.IsChecked();
		m_edtAvatarCustomSize.Enable(enabled);
		m_spnAvatarCustomSize.Enable(enabled);
	}

	void OnCycleProtosChange(CCtrlBase*)
	{
		bool enabled = m_chkCycleProtos.IsChecked();
		m_edtCycleTime.Enable(enabled);
		m_spnCycleTime.Enable(enabled);
	}

	void OnReplaceSmiliesChange(CCtrlBase*)
	{
		bool enabled = m_chkReplaceSmilies.IsChecked();
		m_chkUseCLSmilies.Enable(enabled);
		m_chkResizeSmilies.Enable(enabled);
	}

	INT_PTR DlgProc(UINT msg, WPARAM wParam, LPARAM lParam) override
	{
		INT_PTR ret = SaveOptsDlgProc(pageControls, _countof(pageControls), MODULENAME, m_hwnd, msg, wParam, lParam);
		if (ret)
			return ret;
		
		return CDlgBase::DlgProc(msg, wParam, lParam);
	}
};

int InitOptionsCallback(WPARAM wParam, LPARAM)
{
	OPTIONSDIALOGPAGE odp = {};
	odp.pDialog = new COptDialog;
	odp.szGroup.w = LPGENW("Contact list");
	odp.szTitle.w = LPGENW("My details");
	odp.flags = ODPF_BOLDGROUPS | ODPF_UNICODE;
	g_plugin.addOptions(wParam, &odp);
	return 0;
}
