/****
 BpmDj: Free Dj Tools
 Copyright (C) 2001-2006 Werner Van Belle

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

#ifndef EXISTENCE_SCANNER_H
#define EXISTENCE_SCANNER_H
#include "growing-array.h"
#include "analyzer.h"
#include "bpmdj-event.h"
class Song;

class ExistenceScanner : public ThreadedAnalyzer
{
  GrowingArray<Song*> * all;
 public:
  ExistenceScanner(const GrowingArray<Song*> &i) : all(i.deepCopy())
    {
    };
  virtual void run();
  virtual void stoppedAnalyzing();
  virtual ~ExistenceScanner();
};


class ExistenceScannerFinished: public BpmDjEvent
{
  ExistenceScanner * thread;
 public:
  ExistenceScannerFinished(ExistenceScanner *c) : thread(c)
    {
    }
  virtual void run(SongSelectorLogic * song_selector_window);
};

#endif

