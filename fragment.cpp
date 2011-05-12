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
using namespace std;
#line 1 "fragment.c++"
#include "files.h"
#include "fragment.h"

FragmentObj::FragmentObj(QString filename)
{
  FILE * f = fopen(filename.ascii(),"rb");
  if (!f)
    {
      printf("Could not read fragment \"%s\"\n",filename.ascii());
      size = 0;
      samples = NULL;
      return;
    }
  fseek(f,0,SEEK_END);
  size = ftell(f)/4;
  fseek(f,0,SEEK_SET);
  samples = bpmdj_allocate(size,stereo_sample2);
  size = readsamples(samples,size,f);
  fclose(f);
};