#include "stdafx.h"

CMPlugin::CMPlugin() :
	PLUGIN<CMPlugin>(MODULENAME, pluginInfoEx)
{}

void CMPlugin::Init()
{
	HookPluginEvent(ME_OPT_INITIALISE, &CMPlugin::OnOptionsInit);
}

void CMPlugin::positionClist()
{
	ClistOptions clOptions;

	if (spOptions.setClistStartState)
		clOptions.state = static_cast<uint8_t>(spOptions.clistState);

	if (spOptions.setClistWidth && spOptions.clistWidth > 0)
		clOptions.width = static_cast<uint32_t>(spOptions.clistWidth);
	else
		spOptions.clistWidth = static_cast<uint32_t>(clOptions.width);

	if (spOptions.setTopPosition || spOptions.setBottomPosition || spOptions.setSidePosition)
		clOptions.isDocked = false;

	if (spOptions.setTopPosition)
		clOptions.y = static_cast<uint32_t>(spOptions.pixelsFromTop);

	RECT WorkArea;
	SystemParametersInfo(SPI_GETWORKAREA, 0, &WorkArea, 0);

	if (spOptions.setBottomPosition) {
		if (spOptions.setTopPosition)
			clOptions.height = WorkArea.bottom - WorkArea.top - spOptions.pixelsFromTop - spOptions.pixelsFromBottom;
		else
			clOptions.y = WorkArea.bottom - spOptions.pixelsFromBottom - clOptions.height;
	}

	if (spOptions.setSidePosition) {
		if (spOptions.clistAlign == ClistAlign::right)
			clOptions.x = WorkArea.right - spOptions.clistWidth - spOptions.pixelsFromSide;
		else
			clOptions.x = WorkArea.left + spOptions.pixelsFromSide;
	}
}

int CMPlugin::OnOptionsInit(WPARAM wParam, LPARAM)
{
	OPTIONSDIALOGPAGE odp = {};
	odp.szGroup.a = LPGEN("Contact list");
	odp.szTitle.a = LPGEN("Start position");
	odp.pDialog = new COptionsDlg();
	odp.flags = ODPF_BOLDGROUPS;
	g_plugin.addOptions(wParam, &odp);

	return 0;
}
