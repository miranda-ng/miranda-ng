/*
 *  xfiregateway - Jabber Gateway for XFire.
 *  Copyright (C) 2006 by
 *          Herbert Poul <herbert.poul@gmail.com> / http://goim.us
 *    http://xfirelib.sphene.net/
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


#ifndef __MONITOREDOBJ_H
#define __MONITOREDOBJ_H

#include <typeinfo>
#include <string>
#include "xdebug.h"
#include <map>

namespace xfirelib {
  class MonitoredObj {
  public:
    MonitoredObj();
    ~MonitoredObj();
    int getTotalObjectCount();
    
    std::string classname;
    int myid;
    static std::map<std::string,int> instances;
    static int id;
  };


};



#endif
