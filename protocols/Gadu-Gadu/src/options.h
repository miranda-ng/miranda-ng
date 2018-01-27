#pragma once

#include "gg.h"

struct GaduOptions
{
	CMOption<BYTE> autoRecconect;
	CMOption<BYTE> keepConnectionAlive;
	CMOption<BYTE> showConnectionErrors;
	CMOption<BYTE> useDirectConnections;
	CMOption<BYTE> useForwarding;
	CMOption<BYTE> useManualHosts;
	CMOption<BYTE> useMsgDeliveryAcknowledge;
	CMOption<BYTE> useSslConnection;

	CMOption<WORD> directConnectionPort;
	CMOption<WORD> forwardPort;

	CMOption<wchar_t*> forwardHost;
	CMOption<wchar_t*> serverHosts;

	GaduOptions(PROTO_INTERFACE *proto);
};

using GaduDlgBase = CProtoDlgBase<GaduProto>;

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

	void OnInitDialog() override;

private:
	void onCheck_ManualHosts(CCtrlCheck*);
	void onCheck_DirectConnections(CCtrlCheck*);
	void onCheck_Forwarding(CCtrlCheck*);

	void showRecconectRequired(CCtrlBase* = nullptr);
};
