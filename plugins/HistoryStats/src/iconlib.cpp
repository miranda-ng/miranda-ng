#include "_globals.h"
#include "iconlib.h"

#include "main.h"
#include "resource.h"

bool IconLib::m_bIcoLibAvailable = false;
HANDLE IconLib::m_hHookSkin2IconsChanged = NULL;

IconLib::IconInfo IconLib::m_IconInfo[] = {
	{ IDI_HISTORYSTATS  , muA("main_menu")     , NULL                     , I18N(muT("Create statistics (main menu)"))     },
	{ IDI_HISTORYSTATS  , muA("menu_show")     , NULL                     , I18N(muT("Show statistics (main menu)"))       },
	{ IDI_HISTORYSTATS  , muA("menu_config")   , NULL                     , I18N(muT("Configure... (main menu)"))          },
	{ IDI_HISTORYSTATS  , muA("contact_menu")  , NULL                     , I18N(muT("Contact menu"))                      },
	{ IDI_EXCLUDE_NO    , muA("exclude_no")    , NULL                     , I18N(muT("Unexcluded contacts"))               },
	{ IDI_EXCLUDE_YES   , muA("exclude_yes")   , NULL                     , I18N(muT("Excluded contacts"))                 },
	{ IDI_TREE_CHECK1   , muA("tree_check1")   , I18N(muT("Options tree")), I18N(muT("Checkbox"))                          },
	{ IDI_TREE_CHECK2   , muA("tree_check2")   , I18N(muT("Options tree")), I18N(muT("Checkbox (checked)"))                },
	{ IDI_TREE_CHECK3   , muA("tree_check3")   , I18N(muT("Options tree")), I18N(muT("Checkbox (disabled)"))               },
	{ IDI_TREE_CHECK4   , muA("tree_check4")   , I18N(muT("Options tree")), I18N(muT("Checkbox (checked & disabled)"))     },
	{ IDI_TREE_RADIO1   , muA("tree_radio1")   , I18N(muT("Options tree")), I18N(muT("Radio button"))                      },
	{ IDI_TREE_RADIO2   , muA("tree_radio2")   , I18N(muT("Options tree")), I18N(muT("Radio button (checked)"))            },
	{ IDI_TREE_RADIO3   , muA("tree_radio3")   , I18N(muT("Options tree")), I18N(muT("Radio button (disabled)"))           },
	{ IDI_TREE_RADIO4   , muA("tree_radio4")   , I18N(muT("Options tree")), I18N(muT("Radio button (checked & disabled)")) },
	{ IDI_TREE_EDIT1    , muA("tree_edit1")    , I18N(muT("Options tree")), I18N(muT("Edit control"))                      },
	{ IDI_TREE_EDIT2    , muA("tree_edit2")    , I18N(muT("Options tree")), I18N(muT("Edit control (disabled)"))           },
	{ IDI_TREE_COMBO1   , muA("tree_combo1")   , I18N(muT("Options tree")), I18N(muT("Combo box"))                         },
	{ IDI_TREE_COMBO2   , muA("tree_combo2")   , I18N(muT("Options tree")), I18N(muT("Combo box (disabled)"))              },
	{ IDI_TREE_FOLDER1  , muA("tree_folder1")  , I18N(muT("Options tree")), I18N(muT("Folder"))                            },
	{ IDI_TREE_FOLDER2  , muA("tree_folder2")  , I18N(muT("Options tree")), I18N(muT("Folder (disabled)"))                 },
	{ IDI_TREE_BUTTON1  , muA("tree_button1")  , I18N(muT("Options tree")), I18N(muT("Button"))                            },
	{ IDI_TREE_BUTTON2  , muA("tree_button2")  , I18N(muT("Options tree")), I18N(muT("Button (disabled)"))                 },
	{ IDI_TREE_DATETIME1, muA("tree_datetime1"), I18N(muT("Options tree")), I18N(muT("Date/time picker"))                  },
	{ IDI_TREE_DATETIME2, muA("tree_datetime2"), I18N(muT("Options tree")), I18N(muT("Date/time picker (disabled)"))       },
};

ext::string IconLib::m_Section;
ext::a::string IconLib::m_IconName;
IconLib::CallbackSet IconLib::m_Callbacks;

int IconLib::handleCallbacks(WPARAM wParam, LPARAM lParam)
{
	citer_each_(CallbackSet, i, m_Callbacks)
	{
		(*i->first)(i->second);
	}

	return 0;
}

void IconLib::init()
{
	array_each_(i, m_IconInfo)
	{
		m_IconInfo[i].hIcon = NULL;
	}

	if (m_bIcoLibAvailable = mu::icolib::_available())
	{
		bool bIcoLibTested = false;

		mu_ansi szModule[MAX_PATH];

		GetModuleFileNameA(g_hInst, szModule, MAX_PATH);

		m_Section = muT("HistoryStats");
		m_IconName = muA("historystats_");

		array_each_(i, m_IconInfo)
		{
			ext::string strSection = m_Section;

			if (m_IconInfo[i].szSection)
			{
				strSection += muT("/");
				strSection += i18n(m_IconInfo[i].szSection);
			}

			mu::icolib::addIcon(
				strSection.c_str(),
				i18n(m_IconInfo[i].szDescription),
				(m_IconName + m_IconInfo[i].szIconName).c_str(),
				szModule,
				-m_IconInfo[i].wID);

			if (!bIcoLibTested)
			{
				bIcoLibTested = true;

				if (!getIcon(static_cast<IconIndex>(i)))
				{
					m_bIcoLibAvailable = false;

					break;
				}
			}
		}

		m_hHookSkin2IconsChanged = HookEvent(ME_SKIN2_ICONSCHANGED, handleCallbacks);
	}

	if (!m_bIcoLibAvailable)
	{
		array_each_(i, m_IconInfo)
		{
			m_IconInfo[i].hIcon = reinterpret_cast<HICON>(LoadImage(
				g_hInst,
				MAKEINTRESOURCE(m_IconInfo[i].wID),
				IMAGE_ICON,
				OS::smIconCX(),
				OS::smIconCY(),
				0));
		}
	}
}

void IconLib::registerCallback(CallbackProc callback, LPARAM lParam)
{
	m_Callbacks.insert(std::make_pair(callback, lParam));
}

void IconLib::unregisterCallback(CallbackProc callback, LPARAM lParam)
{
	m_Callbacks.erase(std::make_pair(callback, lParam));
}

void IconLib::uninit()
{
	array_each_(i, m_IconInfo)
	{
		if (m_IconInfo[i].hIcon)
		{
			DestroyIcon(m_IconInfo[i].hIcon);
			m_IconInfo[i].hIcon = NULL;
		}
	}
}

HICON IconLib::getIcon(IconIndex index)
{
	if (m_bIcoLibAvailable)
	{
		return mu::icolib::getIcon((m_IconName + m_IconInfo[index].szIconName).c_str());
	}
	else
	{
		return m_IconInfo[index].hIcon;
	}
}
