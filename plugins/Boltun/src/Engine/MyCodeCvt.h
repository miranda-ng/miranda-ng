//***********************************************************
//	Copyright © 2008 Valentin Pavlyuchenko
//
//	This file is part of Boltun.
//
//    Boltun is free software: you can redistribute it and/or modify
//    it under the terms of the GNU General Public License as published by
//    the Free Software Foundation, either version 2 of the License, or
//    (at your option) any later version.
//
//    Boltun is distributed in the hope that it will be useful,
//    but WITHOUT ANY WARRANTY; without even the implied warranty of
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//    GNU General Public License for more details.
//
//    You should have received a copy of the GNU General Public License
//	  along with Boltun. If not, see <http://www.gnu.org/licenses/>.
//
//***********************************************************

#ifndef MYCODECVT_H
#define MYCODECVT_H

typedef std::codecvt<wchar_t, char, std::mbstate_t> MyCodeCvtBase;

class MyCodeCvt
	: public MyCodeCvtBase
{
public:
	typedef wchar_t _E;
	typedef char _To;
	typedef std::mbstate_t _St;
	explicit MyCodeCvt(size_t _R = 0);
protected:
	virtual result do_in(_St& _State, const _To* _F1, const _To* _L1, const _To*& _Mid1,
		_E* F2, _E* _L2, _E*& _Mid2) const;
#ifdef MSVC
	virtual result do_out(_St& _State, const _E* _F1, const _E* _L1, const _E*& _Mid1,
		_To* F2, _E* _L2, _To*& _Mid2) const;
#else
	virtual result do_out(_St& _State, const _E* _F1, const _E* _L1, const _E*& _Mid1,
		_To* F2, _To* _L2, _To*& _Mid2) const;
#endif
	virtual result do_unshift(_St& _State, _To* _F2, _To* _L2, _To*& _Mid2) const;
#ifdef MSVC
	virtual int do_length(_St& _State, const _To* _F1, const _To* _L1, size_t _N2) const _THROW0();
#else
	virtual int do_length(const _St& _State, const _To* _F1, const _To* _L1, size_t _N2) const _THROW0();
#endif
	virtual bool do_always_noconv() const _THROW0();
	virtual int do_max_length() const _THROW0();
	virtual int do_encoding() const _THROW0();
};

#endif /* MYCODECVT_H */