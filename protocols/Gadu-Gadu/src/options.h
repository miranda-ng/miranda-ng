#pragma once

#include "gg.h"

struct GaduOptions
{
	CMOption<uint8_t> autoRecconect;
	CMOption<uint8_t> keepConnectionAlive;
	CMOption<uint8_t> showConnectionErrors;
	CMOption<uint8_t> useDirectConnections;
	CMOption<uint8_t> useForwarding;
	CMOption<uint8_t> useManualHosts;
	CMOption<uint8_t> useMsgDeliveryAcknowledge;
	CMOption<uint8_t> useSslConnection;

	CMOption<uint16_t> directConnectionPort;
	CMOption<uint16_t> forwardPort;

	CMOption<wchar_t*> forwardHost;
	CMOption<wchar_t*> serverHosts;

	GaduOptions(PROTO_INTERFACE *proto);
};

using GaduDlgBase = CProtoDlgBase<GaduProto>;

class GaduOptionsDlgConference : public GaduDlgBase
{
private:
	CCtrlCombo cmbPolicyForAllChatParticipants;
	CCtrlSpin edtNumberOfAllChatParticipants;

	CCtrlCombo cmbPolicyForUnknownChatParticipants;
	CCtrlSpin edtNumberOfUnknownChatParticipants;

	CCtrlCombo cmbDefaultChatPolicy;

public:
	GaduOptionsDlgConference(GaduProto *proto);

	bool OnInitDialog() override;
	bool OnApply() override;
};

class GaduOptionsDlgAdvanced : public GaduDlgBase
{
private:
	CCtrlCheck chkAutoReconnect;
	CCtrlCheck chkKeepConnectionAlive;
	CCtrlCheck chkMsgAcknowledge;
	CCtrlCheck chkShowConnectionErrors;
	CCtrlCheck chkSslConnection;
	
	CCtrlCheck chkManualHosts;
	CCtrlEdit edtServerHosts;
	CCtrlBase txtServerHostsLabel;

	CCtrlCheck chkDirectConnections;
	CCtrlEdit edtDirectPort;
	CCtrlBase txtDirectPortLabel;

	CCtrlCheck chkForwarding;
	CCtrlEdit edtForwardHost;
	CCtrlBase txtForwardHostLabel;
	CCtrlEdit edtForwardPort;
	CCtrlBase txtForwardPortLabel;

	CCtrlBase txtReconnectRequired;

public:
	GaduOptionsDlgAdvanced(GaduProto *proto);

	bool OnInitDialog() override;

private:
	void onCheck_ManualHosts(CCtrlCheck*);
	void onCheck_DirectConnections(CCtrlCheck*);
	void onCheck_Forwarding(CCtrlCheck*);

	void showRecconectRequired(CCtrlBase* = nullptr);
};
