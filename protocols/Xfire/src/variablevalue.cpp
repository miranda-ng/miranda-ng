/*
 *  xfirelib - C++ Library for the xfire protocol.
 *  Copyright (C) 2006 by
 *          Beat Wolf <asraniel@fryx.ch> / http://gfire.sf.net
 *          Herbert Poul <herbert.poul@gmail.com> / http://goim.us
 *    http://xfirelib.sphene.net
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301 USA
 */

#include "stdafx.h"

#include "variablevalue.h"
#include "xdebug.h"

#include "math.h" // for pow(..)

using namespace std;

namespace xfirelib
{
	VariableValue::VariableValue()
	{
		m_value = 0;
	}

	VariableValue::~VariableValue()
	{
		delete[] m_value;
	}

	void VariableValue::setName(std::string name)
	{
		m_name = name;
	}

	void VariableValue::setValueLength(int valueLength)
	{
		m_valueLength = valueLength;
	}

	void VariableValue::setValue(char * value)
	{
		if (m_value) delete[] m_value;
		m_value = value;
	}

	void VariableValue::setValue(std::string value)
	{
		const char *val = value.c_str();
		setValue(val, value.length());
	}

	void VariableValue::setValue(const char *value, int valueLength)
	{
		m_value = new char[valueLength];
		memcpy(m_value, value, valueLength);
		m_valueLength = valueLength;
	}

	void VariableValue::setValueFromLong(long value, int bytes)
	{
		m_valueLength = bytes;
		m_value = new char[bytes];
		for (int i = 0; i < bytes; i++) {
			m_value[i] = value % 256;
			value = value / 256;
		}
	}

	long VariableValue::getValueAsLong()
	{
		long intVal = 0;
		for (int i = 0; i < m_valueLength; i++) {
			intVal += ((unsigned char)m_value[i]) * myPow(256, i);
		}
		return intVal;
	}

	long VariableValue::myPow(int x, int y)
	{
		long r = 1;
		for (int i = 0; i < y; i++) r *= x;
		return r;
	}

	int VariableValue::readName(char *packet, int index)
	{
		int read = 0;
		int nameLength = packet[index];
		read++;
		char* namestr = new char[nameLength + 1];
		namestr[nameLength] = 0;
		memcpy(namestr, packet + index + read, nameLength);
		m_name = string(namestr);
		read += nameLength;
		delete[] namestr;
		return read;
	}

	int VariableValue::readValue(char *packet, int index, int length, int ignoreZeroAfterLength)
	{
		int read = 0;
		m_valueLength = length;
		if (m_valueLength < 0) {
			m_valueLength = (unsigned char)packet[index + read]; read++;
			if (ignoreZeroAfterLength)
				read++;
		}

		if (m_value)
			delete[] m_value;
		m_value = new char[m_valueLength];
		memcpy(m_value, packet + index + read, m_valueLength);
		read += m_valueLength;
		return read;
	}

	int VariableValue::readVariableValue(char *packet, int index, int)
	{
		VariableValue *value = this;
		int nameLength = packet[index];

		int attLengthLength = 0;
		int attLength = 0;
		string name;

		int i = 1;
		for (; i <= nameLength; i++)
			name += packet[index + i];

		value->setName(name);

		index += i;
		attLengthLength = packet[index];
		index++;

		for (i = 0; i < attLengthLength; i++)
			attLength += (unsigned char)packet[index + i];/*todo: make it work if length is longer than 1 byte*/

		value->setValueLength(attLength);

		char *att = new char[attLength];
		index += i + 1;
		for (i = 0; i < attLength; i++)
			att[i] = packet[index + i];

		value->setValue(att);
		index += i;
		return index;
	}

	int VariableValue::writeName(char *buf, int index)
	{
		int len = m_name.length();
		buf[index] = len;
		memcpy(buf + index + 1, m_name.c_str(), len);
		return len + 1;
	}
	int VariableValue::writeValue(char *buf, int index)
	{
		memcpy(buf + index, m_value, m_valueLength);
		return m_valueLength;
	}
};
