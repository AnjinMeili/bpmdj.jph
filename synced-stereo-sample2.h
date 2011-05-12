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
#ifndef __loaded__synced_stereo_sample2_h__
#define __loaded__synced_stereo_sample2_h__
using namespace std;
#line 1 "synced-stereo-sample2.h++"
#include "stereo-sample2.h"

#define sync_max 8192
#define sync_halve (8192/2)

class synced_stereo_sample2
{
 public:
  stereo_sample2 value;
  signed4 sync;
};
#endif // __loaded__synced_stereo_sample2_h__
