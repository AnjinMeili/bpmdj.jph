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
#line 1 "wavelet-analyzer.logic.c++"
#include <qapplication.h>
#include <qlineedit.h>
#include <qpixmap.h>
#include <qpainter.h>
#include <qpushbutton.h>
#include <qlcdnumber.h>
#include <qcheckbox.h>
#include <qlabel.h>
#include <qprogressbar.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <libgen.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/ioctl.h>
#include <qspinbox.h>
#include <unistd.h>
#include <libgen.h>
#include <qslider.h>
#include <fftw3.h>
#include <time.h>
#include <sys/times.h>
#include <math.h>
#include <assert.h>
#include <math.h>
#include "memory.h"
#include "spectrum-analyzer.logic.h"
#include "kbpm-play.h"
#include "version.h"
#include "scripts.h"
#include "signals.h"
#include "data.h"

static float lineabsmax[1024];

void normalize_draw_decompose(Array<1,float4>& block, QPainter &p, int y, int sx, int depth)
{
   //  normalize(block);
   int w = block.size(0);
   QColor col;
   float divisor = lineabsmax[y];
   for(int x = 0 ; x < sx ; x ++)
     {
	int a = x*w/sx;
	float d = block[a];
	float ad = fabs(d);
	if (ad>divisor)
	  lineabsmax[y]=divisor=ad;
	d/=divisor;
	ad/=divisor;
	int h = (int)(240.0-240*ad);
	//      int v = (int)(255.0*fabs(block[a]));
	int v = (int)(255*ad*2);
	if (v<0) v = 0;
	if (v>255) v = 255;
	col.setHsv(h,255,v);
	p.setPen(col);
	p.drawPoint(x,y);
     }
   if(depth==0) return;
   // decompose the sucker in two pieces
   Array<1,float4> lo(w/2),hi(w/2);
   float prev  = 0.0;
   hi[0]=lo[0]=0;
   int target = 0;
   Array<1,float4>::values it(block);
   float twoago = 0;
   while(it.more())
     {
	float here;
	prev = *it;
	it++;
	here = *it;
	hi[target]=here-twoago;
	lo[target]=twoago+2*prev+here;
	it++;
	twoago= prev;
	target ++;
     }
   int height = (1<<depth)-1;
   normalize_draw_decompose(hi, p, y+1,        sx, depth-1);
   normalize_draw_decompose(lo, p, y+1+height, sx, depth-1);
}

void SpectrumDialogLogic::fetchSpectrum_wavelet()
{
  // read the file in memory
  FILE * raw = openCoreRawFile();
  long int fs = fsize(raw)/4;
  int DIV=8;
  int window_size = 16384*DIV;
  int depth=8;
  int height = (1<<(depth+1))-1;
  Array<1,float4> window(window_size);
  Array<1,float4> windowtje(window_size/DIV);
  // a painter to draw our wavelet
  QPixmap *pm = new QPixmap(1024, height);
  QPainter p;
  for(int i = 0 ; i < 1024 ; i ++)
    lineabsmax[i]=0;
  // read window by window
  for(long int pos = 44100*60 ; pos+window_size < fs ; pos += window_size)
    {
      if (pos+window_size>fs) break;
      fseek(raw,pos*sizeof(stereo_sample2),SEEK_SET);
      readsamples(window,raw);
      Array<1,float4>::positions it(windowtje);
      for(;it.more();it++)
	it = window[it.position[0]*DIV];
      
      p.begin(pm);
      normalize_draw_decompose(windowtje,p,0,1024,depth);
      p.end(); 
      histo->setPixmap(*pm);
      app->processEvents();
      ::y=pos;
      //      sleep(5);
    }
}