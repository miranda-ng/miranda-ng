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

#include <string>
#include "variablevalue.h"
#include "xdebug.h"

#include "math.h" // for pow(..)

namespace xfirelib {

using namespace std;

  VariableValue::VariableValue() {
    value = 0;
  }
  VariableValue::~VariableValue() {
    if (value)
      delete[] value;
  }

  void VariableValue::setName(std::string name){
    this->name = name; 
  }

  void VariableValue::setValueLength(int valueLength){
    this->valueLength = valueLength;
  }

  void VariableValue::setValue( char * value) {
    if (this->value) delete[] this->value;
    this->value = value;
  }
  void VariableValue::setValue( std::string value ) {
    const char *val = value.c_str();
    setValue( val, value.length() );
  }
  void VariableValue::setValue( const char *value, int valueLength ) {
    this->value = new char[valueLength];
    memcpy( this->value, value, valueLength );
    this->valueLength = valueLength;
  }
  void VariableValue::setValueFromLong( long value, int bytes ) {
    this->valueLength = bytes;
    this->value = new char[bytes];
    for(int i = 0 ; i < bytes ; i++) {
      this->value[i] = value % 256;
      value = value / 256;
    }
  }

  std::string VariableValue::getName(){
    return name;
  }

  int VariableValue::getValueLength(){
    return valueLength;
  }

  char* VariableValue::getValue(){
    return value;
  }

  long VariableValue::getValueAsLong() {
    long intVal = 0;
    for(int i = 0 ; i < valueLength ; i++) {
      intVal += ((unsigned char)value[i]) * myPow(256,i);
    }
    return intVal;
  }

  long VariableValue::myPow(int x, int y) {
    long r = 1;
    for(int i = 0 ; i < y ; i++) r *= x;
    return r;
  }

  int VariableValue::readName(char *packet, int index) {
    int read = 0;
    int nameLength = packet[index];
    read++;
    char* namestr=new char[nameLength+1];
	namestr[nameLength]=0;
    memcpy(namestr,packet+index+read,nameLength);
    name = string(namestr);
    read+=nameLength;
	delete[] namestr;
    return read;
  }

  int VariableValue::readValue(char *packet, int index, int length,int ignoreZeroAfterLength) {
    int read = 0;
    valueLength = length;
    if (valueLength < 0) {
      valueLength = (unsigned char)packet[index+read];read++;
      if (ignoreZeroAfterLength) read++;
    }

    if (value) delete[] value;
    value = new char[valueLength];
    memcpy(value,packet+index+read,valueLength);
    read+=valueLength;

    return read;
  }

/*TODO: this cant work, attLength is always 0, needs to be fixed*/
/* int VariableValue::readFixValue(char *packet, int index, int packetLength, int valueLength) {
    VariableValue *value = this;
    int nameLength = packet[index];
    
    int i = 1;
    int attLengthLength = 0;
    int attLength = 0;
    string name;
    
    for(; i <= nameLength;i++){
      name += packet[index+i];
    }
    value->setName(name);
    index += i;
    index++; //ignore next value

    value->setValueLength(valueLength);
    
    char *att = new char[attLength];
    index += i+1;
    for(i = 0; i < attLength;i++){
      att[i] = packet[index+i];
    }
    index += i;
    value->setValue(att);
    return index;
 }*/


  int VariableValue::readVariableValue(char *packet, int index, int packetLength){
    VariableValue *value = this;
    int nameLength = packet[index];
    
    int i = 1;
    int attLengthLength = 0;
    int attLength = 0;
    string name;
    
    for(; i <= nameLength;i++){
      name += packet[index+i];
    }
    value->setName(name);
    
    index += i;
    attLengthLength = packet[index];
    index++;
    
    for(i = 0; i < attLengthLength;i++){
      attLength += (unsigned char)packet[index+i];/*todo: make it work if length is longer than 1 byte*/
    }
    value->setValueLength(attLength);
    
    char *att = new char[attLength];
    index += i+1;
    for(i = 0; i < attLength;i++){
      att[i] = packet[index+i];
    }
    value->setValue(att);
    index += i;
    return index;
  }




  int VariableValue::writeName(char *buf, int index) {
    int len = name.length();
    buf[index] = len;
    memcpy(buf+index+1,name.c_str(),len);
    return len+1;
  }
  int VariableValue::writeValue(char *buf, int index) {
    memcpy(buf+index,value,valueLength);
    return valueLength;
  }

};
