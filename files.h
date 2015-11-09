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
#ifndef __loaded__files_h__
#define __loaded__files_h__
using namespace std;
#include <cstdio>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include "stereo-sample2.h"

long fsize(FILE * f);

template <class sample_type>
long readsamples(sample_type* target, int count, FILE* file)
{
  int result;
  assert(target);
  assert(file);
  assert(count>0);
  result = fread(target,sizeof(sample_type),count,file);
  if (result<=0)
    {
      int err = ferror(file);
      if (feof(file)) 
	return 0;
      printf("Could not read %d samples, errno = %d (%s)\n",
	     count,err,strerror(err));
      assert(0);
    }
  return result;
}

long writesamples(void* target, int count, FILE* file);
#endif // __loaded__files_h__
