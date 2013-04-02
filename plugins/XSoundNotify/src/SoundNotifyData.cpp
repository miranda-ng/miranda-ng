#include "Common.h"

SoundNotifyData::SoundNotifyData() : _contact(0), _soundChanged(false)
{

}

SoundNotifyData::SoundNotifyData(HANDLE contact, const ModuleString & module, const std::tstring &sound) :
	_contact(contact), _soundPath(sound), _soundChanged(false)
{
}

void SoundNotifyData::setSound(const std::tstring &sound)
{
	_soundChanged = true;
	_soundPath = sound;
}

HANDLE SoundNotifyData::contact() const
{
	return _contact;
}

const std::tstring & SoundNotifyData::soundPath() const
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
