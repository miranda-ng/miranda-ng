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

#include "..\stdafx.h"

using namespace std;

MyCodeCvt::MyCodeCvt(size_t _R)
	: MyCodeCvtBase(_R)
{
}

MyCodeCvt::result MyCodeCvt::do_in(_St&,
	const _To*, const _To*, const _To*&,
	_E*, _E*, _E*&) const
{
	return noconv;
}

#ifdef MSVC
MyCodeCvt::result MyCodeCvt::do_out(_St& _State,
	const _E* _F1, const _E* _L1, const _E*& _Mid1,
	_To* F2, _E* _L2, _To*& _Mid2) const
#else
MyCodeCvt::result MyCodeCvt::do_out(_St&,
	const _E*, const _E*, const _E*&,
	_To*, _To*, _To*&) const
#endif
{
	return noconv;
}

MyCodeCvt::result MyCodeCvt::do_unshift(_St&,
	_To*, _To*, _To*&) const
{
	return noconv;
}

#ifdef MSVC
int MyCodeCvt::do_length(_St& _State, const _To* _F1,
	const _To* _L1, size_t _N2) const _THROW0()
#else
int MyCodeCvt::do_length(const _St&, const _To* _F1,
	const _To* _L1, size_t _N2) const _THROW0()
#endif

{
	return (_N2 < (size_t)(_L1 - _F1)) ? (int)_N2 : (int)(_L1 - _F1);
}

bool MyCodeCvt::do_always_noconv() const _THROW0()
{
	return true;
}

int MyCodeCvt::do_max_length() const _THROW0()
{
	return 2;
}

int MyCodeCvt::do_encoding() const _THROW0()
{
	return 2;
}
