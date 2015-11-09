/****
 BpmDj v4.2-pl4: Free Dj Tools
 Copyright (C) 2001-2012 Werner Van Belle

 http://bpmdj.yellowcouch.org/

 This program is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation; either version 2 of the License, or
 (at your option) any later version.
 
 This program is distributed in the hope that it will be useful,
 but without any warranty; without even the implied warranty of
 merchantability or fitness for a particular purpose.  See the
 GNU General Public License for more details.

 See the authors.txt for a full list of people involved.
****/
#ifndef __loaded__qstring_factory_h__
#define __loaded__qstring_factory_h__
using namespace std;
#include <qstring.h>
#include <set>
#include "Data/types.h"

QString tonumber(const float4 f);
QString tonumber(const unsigned4 b);

class QStringFactory
{
private:
  static bool killed;
  static set<QString> tree;
public:
  static QString create(QString str);
  static void kill();
};
#endif // __loaded__qstring_factory_h__
