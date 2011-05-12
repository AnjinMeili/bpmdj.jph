/****
 BpmDj: Free Dj Tools
 Copyright (C) 2001-2007 Werner Van Belle

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
#ifndef __BPMDJ___USER_NOTIFICATION_H__
#define __BPMDJ___USER_NOTIFICATION_H__
using namespace std;
#line 1 "user-notification.h++"
#include <qstring.h>
#include "bpmdj-event.h"
#include "songselector.logic.h"

/**
 * A user notification can contain information for dialog boxes
 * and status messages. It can also contain a flag
 * that updates the statusbar if needed.
 */
class UserNotification: public BpmDjEvent
{
  bool update_view;
  bool main_changed;
public:
  QString status;
  int status_time;
  QString title;
  QString error;
  bool update_item_list;
  UserNotification();
  void updateProcessView(bool mc);
  void send();
  virtual void run(SongSelectorLogic * window);
};
#endif