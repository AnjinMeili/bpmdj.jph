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
#ifndef __loaded__existence_scanner_cpp__
#define __loaded__existence_scanner_cpp__
using namespace std;
#include "existence-scanner.h"
#include "bpmdj.h"
#include "vector-iterator.h"
#include "user-notification.h"
              
elementResult ActiveExistenceScanner::start(vector<Song*> *all)
{
  if (songs) 
    delete songs;
  songs=all;
  current=0;
  queue_thunk();
  return Done;
}

elementResult ActiveExistenceScanner::thunk()
{
  const static int chunksize = 100;
  if (!songs) return Done;
  unsigned int stopat = songs->size();
  if (stopat>current+chunksize) stopat=current+chunksize;
  for(unsigned int i = current ; i < stopat ;  i++)
    songs[0][i]->checkondisk();
  if (stopat==songs->size())
    {
      UserNotification * ui = new UserNotification();
      ui->status="Finished checking availability of songs";
      ui->update_item_list = true;
      ui->send();
      delete songs;
      songs=NULL;
      return Done;
    }
  current = stopat;
  return RevisitAfterIncoming;
}

elementResult ActiveExistenceScanner::terminate()
{
  if (songs) delete songs;
  deactivate();
  return Done;
}

ExistenceScanner* existenceScanner=NULL;
#endif // __loaded__existence_scanner_cpp__
