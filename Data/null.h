/****
 Hierarchical Data Objects
 Copyright (C) 2005-2012 Werner Van Belle
 http://flow.yellowcouch.org/data/

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
#ifndef __loaded__null_h__
#define __loaded__null_h__
using namespace std;
#line 1 "null.h++"
#include "data.h"

//---------------------------------------------------------------
//                            Null
//---------------------------------------------------------------
class Null: public DataClass
{
 private:
  void crash() const;
 protected:
  virtual QString type_name() const 
    {
      return "Null"; 
    };
  virtual DataClass* shallow_copy() const 
    {
      return new Null();
    };
  virtual void visit(DataVisitor&) 
    {
      crash(); 
    };
 public:
  Null() 
    {
    };
};
#endif // __loaded__null_h__
