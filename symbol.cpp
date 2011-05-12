/****
 Om-Data
 Copyright (C) 2005-2006 Werner Van Belle

 This program is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation; either version 2 of the License, or
 (at your option) any later version.
 
 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program; if not, write to the Free Software
 Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
****/

#include "symbol.h"
#include "data-visitor.h"
#include <set>
using namespace std;

//---------------------------------------------------------------
//                            Symbol
//---------------------------------------------------------------
struct QStringLesser
{
  bool operator()(QString *s1, QString *s2) const {return (*s1)<(*s2);};
};

static set<QString*, QStringLesser> symbol_table;

bool Symbol::operator < (const Symbol & other)
{
  return text < other.text;
}

bool Symbol::operator == (const Symbol & other)
{
  return text == other.text;
}

Symbol::Symbol(QString s)
{
  init(s);
}

void Symbol::init(QString s)
{
  set<QString*,QStringLesser>::iterator pos = symbol_table.find(&s);
  if (pos==symbol_table.end())
    {
      text = new QString(s);
      symbol_table.insert(text);
    }
  else 
    text = *pos;
}

void Symbol::visit(DataVisitor& v)
{
  v.visit(*this);
};