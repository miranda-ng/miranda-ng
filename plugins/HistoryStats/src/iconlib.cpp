#include "_globals.h"
#include "iconlib.h"

#include "main.h"
#include "resource.h"

bool IconLib::m_bIcoLibAvailable = false;
HANDLE IconLib::m_hHookSkin2IconsChanged = NULL;

IconLib::IconInfo IconLib::m_IconInfo[] = {
	{ IDI_HISTORYSTATS  , "main_menu"     , NULL,                   LPGENT("Create statistics (main menu)")     },
	{ IDI_HISTORYSTATS  , "menu_show"     , NULL,                   LPGENT("Show statistics (main menu)")       },
	{ IDI_HISTORYSTATS  , "menu_config"   , NULL,                   LPGENT("Configure... (main menu)")          },
	{ IDI_HISTORYSTATS  , "contact_menu"  , NULL,                   LPGENT("Contact menu")                      },
	{ IDI_EXCLUDE_NO    , "exclude_no"    , NULL,                   LPGENT("Unexcluded contacts")               },
	{ IDI_EXCLUDE_YES   , "exclude_yes"   , NULL,                   LPGENT("Excluded contacts")                 },
	{ IDI_TREE_CHECK1   , "tree_check1"   , LPGENT("Options tree"), LPGENT("Checkbox")                          },
	{ IDI_TREE_CHECK2   , "tree_check2"   , LPGENT("Options tree"), LPGENT("Checkbox (checked)")                },
	{ IDI_TREE_CHECK3   , "tree_check3"   , LPGENT("Options tree"), LPGENT("Checkbox (disabled)")               },
	{ IDI_TREE_CHECK4   , "tree_check4"   , LPGENT("Options tree"), LPGENT("Checkbox (checked & disabled)")     },
	{ IDI_TREE_RADIO1   , "tree_radio1"   , LPGENT("Options tree"), LPGENT("Radio button")                      },
	{ IDI_TREE_RADIO2   , "tree_radio2"   , LPGENT("Options tree"), LPGENT("Radio button (checked)")            },
	{ IDI_TREE_RADIO3   , "tree_radio3"   , LPGENT("Options tree"), LPGENT("Radio button (disabled)")           },
	{ IDI_TREE_RADIO4   , "tree_radio4"   , LPGENT("Options tree"), LPGENT("Radio button (checked & disabled)") },
	{ IDI_TREE_EDIT1    , "tree_edit1"    , LPGENT("Options tree"), LPGENT("Edit control")                      },
	{ IDI_TREE_EDIT2    , "tree_edit2"    , LPGENT("Options tree"), LPGENT("Edit control (disabled)")           },
	{ IDI_TREE_COMBO1   , "tree_combo1"   , LPGENT("Options tree"), LPGENT("Combo box")                         },
	{ IDI_TREE_COMBO2   , "tree_combo2"   , LPGENT("Options tree"), LPGENT("Combo box (disabled)")              },
	{ IDI_TREE_FOLDER1  , "tree_folder1"  , LPGENT("Options tree"), LPGENT("Folder")                            },
	{ IDI_TREE_FOLDER2  , "tree_folder2"  , LPGENT("Options tree"), LPGENT("Folder (disabled)")                 },
	{ IDI_TREE_BUTTON1  , "tree_button1"  , LPGENT("Options tree"), LPGENT("Button")                            },
	{ IDI_TREE_BUTTON2  , "tree_button2"  , LPGENT("Options tree"), LPGENT("Button (disabled)")                 },
	{ IDI_TREE_DATETIME1, "tree_datetime1", LPGENT("Options tree"), LPGENT("Date/time picker")                  },
	{ IDI_TREE_DATETIME2, "tree_datetime2", LPGENT("Options tree"), LPGENT("Date/time picker (disabled)")       },
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

	if (m_bIcoLibAvailable = mu::icolib::_available()) {
		bool bIcoLibTested = false;

		char szModule[MAX_PATH];

		GetModuleFileNameA(g_hInst, szModule, MAX_PATH);

		m_Section = _T("HistoryStats");
		m_IconName = "historystats_";

		array_each_(i, m_IconInfo)
		{
			ext::string strSection = m_Section;

			if (m_IconInfo[i].szSection) {
				strSection += _T("/");
				strSection += TranslateTS(m_IconInfo[i].szSection);
			}

			mu::icolib::addIcon(
				strSection.c_str(),
				TranslateTS(m_IconInfo[i].szDescription),
				(m_IconName + m_IconInfo[i].szIconName).c_str(),
				szModule,
				-m_IconInfo[i].wID);

			if (!bIcoLibTested) {
				bIcoLibTested = true;

				if (!getIcon(static_cast<IconIndex>(i))) {
					m_bIcoLibAvailable = false;
					break;
				}
			}
		}

		m_hHookSkin2IconsChanged = HookEvent(ME_SKIN2_ICONSCHANGED, handleCallbacks);
	}

	if (!m_bIcoLibAvailable) {
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
		if (m_IconInfo[i].hIcon) {
			DestroyIcon(m_IconInfo[i].hIcon);
			m_IconInfo[i].hIcon = NULL;
		}
	}
}

HICON IconLib::getIcon(IconIndex index)
{
	if (m_bIcoLibAvailable)
		return mu::icolib::getIcon((m_IconName + m_IconInfo[index].szIconName).c_str());

	return m_IconInfo[index].hIcon;
}
