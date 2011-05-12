/****
 BpmDj: Free Dj Tools
 Copyright (C) 2001-2004 Werner Van Belle
 See 'BeatMixing.ps' for more information

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

#include <stdlib.h>
#include <stdio.h>
#include "qstring-factory.h"
#include "songtree.h"

const QString TRUE_TEXT("Yes");
const QString FALSE_TEXT("No");
const QString TAG_TRUE("Yes"); 
const QString TAG_FALSE("___");
const QString zero("0");
const QString zeroo("00");
const QString slash("/");
const QString EMPTY("");

QString tonumber(const int b)
{
  return ( b < 10 ?
	   zeroo+QString::number(b) :
	   ( b < 100 ?
	     zero+QString::number(b) :
	     QString::number(b)));
}

QString tonumber(const float f)
{
  return QString::number(f);
}

AvlTree<QString> QStringFactory::tree;
bool QStringFactory::killed = false;

// unsigned long winst = 0;
QString QStringFactory::create(char*  str)
{
  if (str==NULL) return QString::null;
  if (killed) return QString(str);
  QString newly_allocated(str);
  QStringNode * found = (QStringNode*) tree.search(newly_allocated);
  if (found)
    {
      // winst+=found->content.length()*2+1;
      return found->content;
      // the newly allocated string will be destroyed at the moment
      // this function returns. The data stored in the found content
      // will be refered an extra time.
    }
  else
    {
      tree.add(new QStringNode(newly_allocated));
      // we add this newly allocated string, the object located in 
      // this method will be removed, however, the content will be passed 
      // on to the result and in the tree.
      return newly_allocated;
    }
}

void QStringFactory::kill()
{
  // printf("Advantage by using QString factory = %d\n",winst);
  tree.purge();
  killed=true;
}
