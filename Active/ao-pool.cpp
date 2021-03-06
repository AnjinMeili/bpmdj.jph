/****
 Active Objects v4.3
 Copyright (C) 2006-2012 Werner Van Belle
 http://active.yellowcouch.org/

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
#ifndef __loaded__ao_pool_cpp__
#define __loaded__ao_pool_cpp__
using namespace std;
#include <cstdio>
#include <iostream>
#include <unistd.h>
#include "ao-pool.h"
#include "ao-tracker.h"
#include "ao-scheduler.h"

/**
 * This pool relies on one Thread spawner and will serialize
 * the incoming requests by sending them to an active 
 * object. The scheduler requested for the self active object
 * (called "ao-pool") will be itself, however, when the sun
 * sets for the ao-pool will the application terminate.
 * The sun will set for ao-pool when it finds no active requests
 * pending.
 * This pool offers the following advantages
 * - no duplicate active object names can occur
 * - shared scheduler
 * - no double deletion of active objects
 * - automatic exit when no threads left to play
 */ 
class StdDawnTilDusk: public DawnTilDusk
{
  SpawnWhenActivated scheduler;
  AoTracker *tracker;
  volatile bool running;
public:
  StdDawnTilDusk(): scheduler()
  {
    // printf("StdDawnTillDusk gefixed\n");
    tracker=NULL;
    aoPool=this;
    tracker=new AoTracker();
    running=true;
  }
  ~StdDawnTilDusk()
  {
    assert(!running);
    delete tracker;
  }
  /**
   * Called by the constructor of each active object to obtain 
   * a scheduler for itself
   */
  virtual Scheduler * sunrise(string who)
  {
    if (tracker) tracker->sunrise(who);
    return &scheduler;
  };
  /**
   * Called by the scheduler whenever an active object
   * dies
   */
  virtual void sunset(string who)
  {
    assert(tracker);
    if (who=="AoTracker") running=false;
    else tracker->sunset(who);
  };
  virtual void wait_for_finish()
  {
    while(running) 
      {
	sleep(1);
	if (running)
	  {
	    cout<<"Waiting for objects to finish\n"; 
	    print_active_object_list();
	    cout.flush();
	  }
      }
  };
  /**
   * can be called to print the list of active objects
   */
  virtual void print_active_object_list()
  {
    assert(tracker);
    tracker->print();
  }
};

DawnTilDusk * aoPool = new StdDawnTilDusk();
#endif // __loaded__ao_pool_cpp__
