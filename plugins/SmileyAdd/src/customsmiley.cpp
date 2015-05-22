/*
Miranda SmileyAdd Plugin
Copyright (C) 2008 - 2011 Boris Krasnovskiy All Rights Reserved

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation version 2
of the License.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "general.h"

SmileyPackCListType g_SmileyPackCStore;


bool SmileyPackCListType::AddSmileyPack(MCONTACT hContact, TCHAR* dir)
{
	bool res = true;
	if (GetSmileyPack(hContact) == NULL) {
		SmileyPackCType *smileyPack = new SmileyPackCType;

		res = smileyPack->LoadSmileyDir(dir);
		if (res) {
			smileyPack->SetId(hContact);
			m_SmileyPacks.insert(smileyPack);
		}
		else delete smileyPack;
	}
	return res;
}


bool SmileyPackCListType::AddSmiley(MCONTACT hContact, TCHAR* path)
{
	SmileyPackCType *smpack = GetSmileyPack(hContact);
	if (smpack == NULL) {  
		smpack = new SmileyPackCType;

		smpack->SetId(hContact);
		m_SmileyPacks.insert(smpack);
	}
	return smpack->LoadSmiley(path);
}


SmileyPackCType* SmileyPackCListType::GetSmileyPack(MCONTACT id)
{
	for (int i = 0; i < m_SmileyPacks.getCount(); i++)
		if (m_SmileyPacks[i].GetId() == id)
			return &m_SmileyPacks[i];

	return NULL;
}


SmileyCType::SmileyCType(const CMString& fullpath, const TCHAR* filepath)
{
	LoadFromResource(fullpath, 0); 
	CreateTriggerText(T2A_SM(filepath));
}

bool SmileyCType::CreateTriggerText(char* text)
{
	UrlDecode(text);

	int len = (int)mir_strlen(text);
	if (len == 0) return false;

	unsigned reslen;
	char* res = (char*)mir_base64_decode(text, &reslen);
	if (res == NULL)
		return false;

	char save = res[reslen]; res[reslen] = 0; // safe because of mir_alloc
	TCHAR *txt = mir_utf8decodeT(res);
	res[reslen] = save;

	if (txt == NULL)
		return false;

	m_TriggerText = txt;
	mir_free(txt);
	mir_free(res);
	return true;
}


//
// SmileyPackCType
//

bool SmileyPackCType::LoadSmileyDir(TCHAR* dir)
{
	CMString dirs = dir;
	dirs += _T("\\*.*");

	_tfinddata_t c_file;
	INT_PTR hFile = _tfindfirst((TCHAR*)dirs.c_str(), &c_file);
	if (hFile > -1L) {
		do {
			if (c_file.name[0] != '.') {
				CMString fullpath = dir;
				fullpath = fullpath + _T("\\") + c_file.name;
				TCHAR* div = _tcsrchr(c_file.name, '.');
				if (div) {
					*div = 0;
					SmileyCType *smlc = new SmileyCType(fullpath, c_file.name);
					if (smlc->GetTriggerText().IsEmpty())
						delete smlc;
					else
						m_SmileyList.insert(smlc);
				}
			}
		}
			while( _tfindnext( hFile, &c_file ) == 0 );
		_findclose( hFile );
		AddTriggersToSmileyLookup();
		return true;
	}
	return false;
}


bool SmileyPackCType::LoadSmiley(TCHAR* path)
{
	CMString dirs = path;
	int slash = dirs.ReverseFind('\\');
	int dot = dirs.ReverseFind('.');

	CMString name = dirs.Mid(slash+1, dot - slash - 1); 

	for (int i=0; i < m_SmileyList.getCount(); i++)
		if (m_SmileyList[i].GetTriggerText() == name) {
			m_SmileyList[i].LoadFromResource(dirs, 0);
			return true; 
		}

	m_SmileyList.insert(new SmileyCType(dirs, (TCHAR*)name.c_str()));

	CMString empty;
	m_SmileyLookup.insert(new SmileyLookup(
		m_SmileyList[m_SmileyList.getCount()-1].GetTriggerText(), false, m_SmileyList.getCount()-1, empty));

	return true;
}


void SmileyPackCType::AddTriggersToSmileyLookup(void)
{
	CMString empty;
	for (int dist=0; dist<m_SmileyList.getCount(); dist++) {
		SmileyLookup *dats = new SmileyLookup(m_SmileyList[dist].GetTriggerText(), false, dist, empty); 
		m_SmileyLookup.insert(dats);
	}
}
