/****
 BpmDj v4.0: Free Dj Tools
 Copyright (C) 2001-2009 Werner Van Belle

 http://bpmdj.yellowcouch.org/

 This program is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation; either version 2 of the License, or
 (at your option) any later version.
 
 This program is distributed in the hope that it will be useful,
 but without any warranty; without even the implied warranty of
 merchantability or fitness for a particular purpose.  See the
 GNU General Public License for more details.
****/
#ifndef __loaded__history_h__
#define __loaded__history_h__
using namespace std;
#line 1 "history.h++"
#include <stdlib.h>
#include <stdio.h>
#include <qstring.h>
#include <qlistview.h>
#include "song.h"
#include "database.h"
#include "data.h"

/**
 * A singleton class to represent the song already played
 */
class History
{
private:
  static Song * t_2;  // T - 2
  static Song * t_1;  // T - 1
  static Song * t_0;  // T
  static FILE* file;
  static QByteArray filename;
  singleton_accessors(int,songs_played);
  static Q3ListView * log_ui;
  static void mark_as_played(DataBase * db, QString s);
  static void mark_as_played(Song *song);
public:
  static void mark_all_played_songs(DataBase * db);
  static void init(const QString filename, Q3ListView * putin);
  void init(const QString filename, DataBase * db, Q3ListView * putin);
  static void this_is_playing(Song * main_now);
  static void save_history();
  static void clear_history(DataBase * db);
};
#endif // __loaded__history_h__
