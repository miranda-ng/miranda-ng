#include "Common.h"

SoundNotifyData::SoundNotifyData() : _contact(0), _soundChanged(false)
{

}

SoundNotifyData::SoundNotifyData(HANDLE contact, const ModuleString & module, const xsn_string & sound) :
	_contact(contact), _soundPath(sound), _soundChanged(false)
{
}

void SoundNotifyData::setSound(const xsn_string & sound)
{
	_soundChanged = true;
	_soundPath = sound;
}

HANDLE SoundNotifyData::contact() const
{
	return _contact;
}

const xsn_string & SoundNotifyData::soundPath() const
{
	return _soundPath;
}

const ModuleString & SoundNotifyData::module() const
{
	return _module;
}

bool SoundNotifyData::isSoundChanged() const
{
	return _soundChanged;
}
