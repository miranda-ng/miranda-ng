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

#include <string>

#ifndef __VARIABVLEVALUE_H
#define __VARIABVLEVALUE_H

namespace xfirelib {

class VariableValue {
 public:
    VariableValue();
    ~VariableValue();

    void setName(std::string name);
    void setValueLength(int valueLength);
    void setValue( char * value );
    void setValue( std::string value );
    void setValue( const char *value, int valueLength );
    void setValueFromLong( long value, int bytes );
    std::string getName();
    int getValueLength();
    char* getValue();

    int readName(char *packet, int index);
    int readValue(char *packet, int index, int length = -1, int ignoreZeroAfterLength = 0);
    int readVariableValue(char *packet, int index, int packetLength);
    /*TODO: disabled because of a bug in this method*/
    //int VariableValue::readFixValue(char *packet, int index, int packetLength, int valueLength);

    int writeName(char *buf, int index);
    int writeValue(char *buf, int index);

    long getValueAsLong();
  private:
    long myPow(int x, int y);

    std::string name;
    char *value;
    int valueLength;
};

};

#endif
