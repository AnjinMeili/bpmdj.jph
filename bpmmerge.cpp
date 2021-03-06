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
#ifndef __loaded__bpmmerge_cpp__
#define __loaded__bpmmerge_cpp__
using namespace std;
#include <stdlib.h>
#include <cstdio>
#include <iostream>
#include <iomanip>
#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>
#include <qdialog.h>
#include <ctype.h>
#include "index.h"
#include "unistd.h"
#include "common.h"
#include "memory.h"
#include "scripts.h"
#include "version.h"
#include "player-config.h"
#include "histogram-property.h"
#include "smallhistogram-type.h"
#include "signals.h"

PlayerConfig * config;

/*-------------------------------------------
 *         BpmDj Merger
 *-------------------------------------------*/
void options_failure(const char* err)
{
  cout << "BpmDj Merger v"VERSION", Copyright (c) 2000-2012 Werner Van Belle\n";
  cout << 
"This software is distributed under the GPL2 license. See copyright.txt\n\n"
"Usage:  bpmmerge <options> [old-song] new-song\n\n"
"  --mix         nbr  how many measures should be used to create the mix\n" 
"  --slide       nbr  how many measures should be used to find the best match\n"
"  --temposwitch nbr  how many measures should be used to switch tempos\n" 
"  --verbose          print useles information\n"
"  --rescantempo      do a fine scan of the tempo, suited for the mix\n"
"  --accountvolume    do a volume accounting when sliding\n"
"  --startpct    nbr  start the new song at %% of its time\n"
"  --stoppct     nbr  stop each song at %% of its time\n"
"  --startatcue       start the new song at the last cue\n"
"    --beforecue      and make sure the cue appears at the end of the mix\n"
"  --split            splits bpm-mixed.raw to separate files\n"
"    --beforemix      splits before a mix begins\n"
"    --middlemix      splits in the middle of the mix\n"
"    --aftermix       splits when the mix is done\n" 
"    --aftertempo     split after tempo change\n"
"    --number         number every part of the split process\n"
"  old-song           the index file of the last song\n"
"  new-song           the index file of the new song\n\n" 
  << err << endl << endl;
  _exit(1);
}

static char * old_index = NULL;
static char * new_index = NULL;
static int    mix_measures = 8;
static int    slide_measures = -1;
static int    tempo_measures = 16;
static int    start_pct = 25;
static int    stop_pct = 75;
static bool   verbose = false;
static bool   use_cue = false;
static bool   before_cue = false;
static bool   normalize = false;
static bool   rescan_tempo = false;
static bool   account_volume = false;
static FILE*  automix_log;

void openlog()
{
  automix_log = fopen("automix.log","a+b");
  assert(automix_log);
}

void log_entry(unsigned8 position, const char* type, const char* text, 
	       const char* text2 = NULL)
{
  position/=4;
  char * tmp = strdup(text);
  tmp=basename(tmp);
  tmp[strlen(tmp)-4]=0;
  if (text2)
    {
      char * tmp2 = strdup(text2);
      tmp2=basename(tmp2);
      tmp2[strlen(tmp2)-4]=0;
      fprintf(automix_log,"%d %s %s-%s\n",(unsigned4)position,type, tmp, tmp2);
    } 
  else
    fprintf(automix_log,"%d %s %s\n",(unsigned4)position,type, tmp);
}

void closeLog()
{
  fclose(automix_log);
}

static bool split = false;
static bool split_beforemix = false;
static bool split_middlemix = false;
static bool split_aftermix = false;
static bool split_aftertempo = false;
static bool split_number = false;

void cut_at(signed8 position, char* target)
{
  position*=4;
  FILE* f1 = fopen(BPMMIXED_NAME,"rb");
  FILE* f2 = fopen(target,"wb");
  assert(f1);
  assert(f2);
  fseek(f1,0,SEEK_END);
  signed8 length = ftell(f1);
  fseek(f1,position,SEEK_SET);
  signed8 togo = length - position;
  cout << "Writing "<< target << endl;
  // copy file
  unsigned4 buffer[64*1024];
  while(togo>0)
    {
      signed4 read = readsamples(buffer,64*1024,f1);
      signed4 write = writesamples(buffer,read,f2);
      assert(write==read);
      togo-=read*4;
    }
  fclose(f1);
  fclose(f2);
  // truncate the large file
  truncate(BPMMIXED_NAME,position);
}

char * split_raw(int nr, signed8 lastpos)
{
  char entry[1024];
  int c, idx=0;
  // read entry
  while((c=fgetc(automix_log))!=-1)
    {
      if (c=='\n')
	break;
      entry[idx++]=c;
    }
  entry[idx]=0;
  if (entry[0]==0)
    return NULL;
  // parse entry as well as possible
  signed8 position = 0;
  idx=0;
  while(isdigit(entry[idx]))
    {
      position*=10;
      position+=entry[idx++]-'0';
    }
  idx++;
  char T = entry[idx];
  char *name = entry+idx+2;
  char *futuresong;
  // continue reading the file
  if (position<lastpos)
    {
      cout<<"Just read "<<entry<<endl;
      cout.flush();
    }
  assert(position>=lastpos);
  if (T == 'M' && split_beforemix)
    futuresong = split_raw(nr+1,position);
  else if (T == 'C' && split_middlemix)
     futuresong = split_raw(nr+1,position);
  else if (T == 'T' && split_aftermix)
     futuresong = split_raw(nr+1,position);
  else if (T == 'S' && (split_aftertempo || position == 0))
    futuresong = split_raw(nr+1,position);
  else
    futuresong = split_raw(nr,position);
  // now the end has been chopped of at the appropriate places
  bool docut = false;
  if (T == 'M' && split_beforemix)
    docut = true;
  else if (T == 'C' && split_middlemix)
    docut = true;
  else if (T == 'T' && split_aftermix)
    docut = true;
  else if (T == 'S')
    {
      docut = (split_aftertempo || position == 0);
      futuresong = strdup(name);
    }
  // create the target filename
  if (docut)
    {
      char cutter [1024];
      int nrofsplits = (split_beforemix ? 1 : 0)
	+ (split_middlemix ? 1 : 0)
	+ (split_aftermix ? 1 : 0)
	+ (split_aftertempo ? 1 : 0);
      if (nrofsplits==1 && futuresong)
	name = strdup(futuresong);
      if (split_number)
	{
	  if (nr<10)
	    sprintf(cutter,"0%d.%s"RAW_EXT,nr,name);
	  else
	    sprintf(cutter,"%d.%s"RAW_EXT,nr,name);
	}
      else
	sprintf(cutter,"%s"RAW_EXT,name);
      cut_at(position,cutter);
    }
  return futuresong;
}

void split_raw()
{
  fseek(automix_log,0,SEEK_SET);
  split_raw(1,-1);
}

void process_options(int argc, char* argv[])
{
  int i=1;
  for(i = 1 ; i < argc ; i ++)
    {
      if (argv[i][0]=='-')
	{
	  char* arg=NULL;
	  // check long opt ?
	  if (argv[i][1]=='-')
	    arg=argv[i]+2;
	  else if (argv[i][1]==0 || argv[i][2]!=0)
	    options_failure("option neither short nor long");
	  else arg=argv[i]+1;
	  // check value
	  if (strcmp(arg,"split")==0)
	    split=true;
	  else if (strcmp(arg,"beforemix")==0)
	    {
	      split_beforemix=true;
	      if (!split)
		options_failure("must specify --split before specifying where "
				"to split");
	    }
	  else if (strcmp(arg,"middlemix")==0)
	    {
	      split_middlemix=true;
	      if (!split)
		options_failure("must specify --split before specifying where "
				"to split");
	    }
	  else if (strcmp(arg,"aftermix")==0)
	    {
	      split_aftermix=true;
	      if (!split)
		options_failure("must specify --split before specifying where "
				"to split");
	    }
	  else if (strcmp(arg,"aftertempo")==0)
	    {
	      split_aftertempo=true;
	      if (!split)
		options_failure("must specify --split before specifying where "
				"to split");
	    }
	  else if (strcmp(arg,"number")==0)
	    {
	      split_number=true;
	      if (!split)
		options_failure("must specify --split before specifying where "
				"to split");
	    }
	  else if (strcmp(arg,"verbose")==0)
	    verbose = true;
	  else if (strcmp(arg,"accountvolume")==0)
	    account_volume = true;
	  else if (strcmp(arg,"normalize")==0)
	    normalize = true;
	  else if (strcmp(arg,"rescantempo")==0)
	    rescan_tempo = true;
	  else if (strcmp(arg,"startatcue")==0)
	    use_cue = true;
	  else if (strcmp(arg,"beforecue")==0)
	    before_cue = true;
	  else if (strcmp(arg,"startpct")==0)
	    {
	      if (++i>=argc) options_failure("mix argument scanning error");
	      start_pct = atoi(argv[i]);
	    }
	  else if (strcmp(arg,"stoppct")==0)
	    {
	      if (++i>=argc) options_failure("mix argument scanning error");
	      stop_pct = atoi(argv[i]);
	    }
	  else if (strcmp(arg,"mix")==0)
	    {
	      if (++i>=argc) 
		options_failure("mix argument scanning error");
	      mix_measures = atoi(argv[i]);
	    }
	  else if (strcmp(arg,"slide")==0)
	    {
	      if (++i>=argc) 
		options_failure("slide argument scanning error");
	      slide_measures = atoi(argv[i]);
	      if (slide_measures<mix_measures)
		options_failure("slide measures must be larger than mix "
				"measures");
	    }
	  else if (strcmp(arg,"temposwitch")==0)
	    {
	      if (++i>=argc) 
		options_failure("tempo switch argument scanning error");
	      tempo_measures = atoi(argv[i]);
	    }
	}
      else
	{
	  if (old_index==NULL)
	    old_index = argv[i];
	  else if (new_index == NULL)
	    new_index = argv[i];
	  else
	    options_failure("can only specify two songs");
	}
    }
  if (slide_measures==-1)
    slide_measures=mix_measures*2;
  if (split)
    {
      if (split_middlemix || split_beforemix ||
	  split_aftermix || split_aftertempo)
	return;
      options_failure("split requires at least one position where to split");
    }
  if (old_index==NULL)
    options_failure("requires at least one song to start with");
  if (new_index==NULL)
    new_index=old_index;
}

static signed4 period_a;
static signed4 period_b;
static signed4 cue_b;
static char * filename_b;
static FILE * file_a;
static FILE * file_b;

void normalize_file()
{
#define BUFSIZE 32*1024
  stereo_sample2 buffer[BUFSIZE];
  signed4 read;
  signed8 position;
  cout << "Normalizing new song\n";
  // find maximum..
  fseek(file_b,0,SEEK_SET);
  signed8 leftmax = 0;
  signed8 rightmax = 0;
  do
    {
      read = readsamples(buffer,BUFSIZE,file_b);
      for(int i = 0 ; i < read ; i++)
	{
	  int m = abs(buffer[i].left);
	  if (m > leftmax) leftmax = m;
	  m = abs(buffer[i].right);
	  if (m > rightmax) rightmax = m;
	}
    }
  while(read>0);
  // rescale everything..
  fseek(file_b,0,SEEK_SET);
  do
    {
      position=ftell(file_b);
      read = readsamples(buffer,BUFSIZE,file_b);
      int i = 0;
      while(i < read)
	{
	  buffer[i].left = 
	    ((signed8)(buffer[i].left))*
	    ((signed8)32767)/leftmax;
	  buffer[i].right = 
	    ((signed8)(buffer[i].right))*
	    ((signed8)32767)/rightmax;
	  i++;
	}
      fseek(file_b,position,SEEK_SET);
      writesamples(buffer,read,file_b);
    }
  while(read>0);
  fseek(file_b,0,SEEK_SET);
}

bool createFiles(char* a, char* b)
{
  Index idx_b(b);
  filename_b = strdup(idx_b.get_filename().toAscii().data());
  period_b   = period_to_quad(idx_b.get_period());
  cue_b      = period_to_quad(idx_b.get_cue());
  set_decoder_environment(config,&idx_b);
  cout << "Decoding " << b << endl;
  if (!bpmdjraw(true,filename_b,"./")) _exit(100);
  file_b=openRawFileForWriting(&idx_b, "./");
  if (normalize) normalize_file();
  
  Index idx_a(a);
  period_a = period_to_quad(idx_a.get_period());
  
  file_a=fopen(BPMMIXED_NAME,"r+b");
  if (file_a)
    {
      if (verbose)
	cout << "Found target file\n";
      return true;
    }
  cout << "This is the first song of the mix\n";
  file_a=fopen(BPMMIXED_NAME,"ab");
  assert(file_a);
  return false;
}

static unsigned4 filelength_a;
static unsigned4 position_a = 0;
static unsigned4 length_a;
static stereo_sample2 *buffer_a; // the full data at the end of the old file

void readTail(signed8 measures)
{
  length_a = (unsigned4)(measures * period_a);
  buffer_a = bpmdj_allocate(length_a,stereo_sample2);
  filelength_a = fsize(file_a);
  if (length_a * 4 > filelength_a)
    {
      cout << "Error: not enough data in file...\n";
      cout.flush();
      _exit(50);
    }
  position_a = filelength_a-(length_a*4);
  position_a /= 4;
  position_a *= 4;

  fseek(file_a,position_a,SEEK_SET);
  cout << "Reading tail (" << (int)length_a
       << " samples at position " << (int)position_a 
       << " of length " << (int)filelength_a
       <<")\n";
  readsamples(buffer_a,length_a,file_a);
}

static unsigned4 filelength_b;
static unsigned4 position_b;
static unsigned4 length_b;
// full new data at the beginning of the new file
static stereo_sample2 *buffer_b; 

void readHead(signed8 percent, signed8 measures)
{
  filelength_b = fsize(file_b);
  length_b = (unsigned4)(measures * period_b);

  position_b=0;
  if (use_cue)
    {
      position_b=cue_b;
      if (!cue_b)
	cout<<"Cannot use cue as head since cue does not exist\n";
      if (before_cue)
	{
	  if (length_b>position_b) position_b=0;
	  else position_b-=length_b;
	}
    }
  else
    {
      position_b = filelength_b*percent/100;
    }

  position_b /= 4;
  position_b *= 4;
  buffer_b = bpmdj_allocate(length_b,stereo_sample2);
  fseek(file_b,position_b,SEEK_SET);
  cout <<"Reading head ("<<length_b
       <<" samples at position "<<position_b
       <<" of length "<<filelength_b
       <<")\n";	 
  readsamples(buffer_b,length_b,file_b);
}

void copySong(signed8 percent)
{
  length_b = (unsigned4)64*1024;
  buffer_b = bpmdj_allocate(length_b,stereo_sample2);
  position_b = ftell(file_b);
  filelength_b = fsize(file_b);
  unsigned4 togo=(filelength_b*percent/100)-position_b;
  togo/=4;
  assert(togo>=0);
  cout << "Copying non-mixed part of song (" <<(int)togo << " samples)\n";
  log_entry(ftell(file_a),"S",new_index);
  while(togo>0)
    {
      unsigned4 toread = togo;
      if (toread>length_b) toread=length_b;
      signed4 read = readsamples(buffer_b,toread,file_b);
      signed4 write = writesamples(buffer_b,read,file_a);
      assert(write>0);
      togo-=toread;
    }
} 

static stereo_sample2 *buffer_c;  // stretched full data of head
static unsigned4 length_c;
void stretchHead(signed8 headmeasures)
{
  cout << "Stretching head to fit (period_a = " << period_a 
       <<", period_b = " << period_b << ")\n";
  length_c = period_a*headmeasures;
  buffer_c = bpmdj_allocate(length_c,stereo_sample2);
  for(unsigned4 i = 0 ; i < length_c ; i ++)
    buffer_c[i]=buffer_b[(unsigned8)i*(unsigned8)length_b/(unsigned8)length_c];
}

// WVB -- WARNING !!!
// als COLLAPSE aangepast wordt, moet eveneens de phasefit herbekeken worden !!!
#define COLLAPSE 4
typedef signed char compressed;
static compressed * buffer_d=NULL;  // collapsed data of tail
static compressed * buffer_e=NULL;  // collapsed data of stretched head
static compressed * buffer_f=NULL;  // collapsed data of non stretched head
static unsigned int length_d;
static unsigned int length_e;
static unsigned int length_f;

void collapseBuffers()
{
  length_d = length_a / COLLAPSE;
  length_f = length_b / COLLAPSE;
  buffer_d = bpmdj_allocate(length_d,compressed);
  buffer_f = bpmdj_allocate(length_f,compressed);
  // calculate absolute value
  for(unsigned4 pos = 0 ; pos < length_d ; pos++)
    buffer_d[pos]=
      (((signed4)abs(((stereo_sample2*)buffer_a)[pos * COLLAPSE].left)+
	(signed4)abs(((stereo_sample2*)buffer_a)[pos * COLLAPSE].right))/2)>>8;
  for(unsigned4 pos = 0 ; pos < length_f ; pos++)
    buffer_f[pos]=
      (((signed4)abs(((stereo_sample2*)buffer_b)[pos * COLLAPSE].left)+
	(signed4)abs(((stereo_sample2*)buffer_b)[pos * COLLAPSE].right))/2)>>8;
}

void collapseStretchedBuffer()
{
  length_e = length_c / COLLAPSE;
  buffer_e = bpmdj_allocate(length_e,compressed);
  for(unsigned4 pos = 0 ; pos < length_e ; pos++)
    buffer_e[pos]=
      (((signed4)abs(((stereo_sample2*)buffer_c)[pos * COLLAPSE].left)+
	(signed4)abs(((stereo_sample2*)buffer_c)[pos * COLLAPSE].right))/2)>>8;
}

signed8 phaseFit(int period, compressed* buffer, int length)
{
  long c,d;
  unsigned4 mismatch=0;
  assert(period<length);
  for(c=period;c<length;c++)
    {
      d=abs((long)buffer[c]-(long)buffer[c-period]);
      mismatch+=d;
    }
  return mismatch;
}

signed8 rescanTempo(signed8 hi, signed8 lo, compressed* buffer, int length)
{
  signed8 minimum = phaseFit(lo,buffer,length);
  signed8 minat = lo;
  signed8 period;
  for(period = lo; period < hi; period++)
    {
      signed8 res = phaseFit(period,buffer,length);
      if (res < minimum)
	{
	  minimum = res;
	  minat = period;
	}
    }
  return minat;
}

void rescanTempo()
{
  if (!rescan_tempo) 
    return;
  // 1 BPM lager gaan ????
  // hoe doen we dat ???
  // eerst terugrekenen naar
  float8 T=4.0*(float8)11025*60.0/(float8)(period_a/4);
  float8 S=T;
  signed8 T1=(signed8)(4.0*(float8)11025*60.0/(float8)(T-1));
  signed8 T2=(signed8)(4.0*(float8)11025*60.0/(float8)(T+1));
  cout<<"Rescanning frequency of tail ";
  period_a=rescanTempo(T1,T2,buffer_d,length_d)*4;
  T=4.0*(float8)11025*60.0/(float8)(period_a/4);
  cout<<"("<<T-S<<" adjustment)\n";
  
  S=T=4.0*(float8)11025*60.0/(float8)(period_b/4);
  T1=(signed8)(4.0*(float8)11025*60.0/(float8)(T-1));
  T2=(signed8)(4.0*(float8)11025*60.0/(float8)(T+1));
  cout << "Rescanning frequency of head ";
  period_b=rescanTempo(T1,T2,buffer_f,length_f)*4;
  T=4.0*(float8)11025*60.0/(float8)(period_b/4);
  cout <<"("<<T-S<<" adjustment)\n";
}

unsigned4 findMatchWithVolumeAccounting()
{
  unsigned int step = 10;
  unsigned4 mindiff=(unsigned4)-1;
  unsigned4 minpos=0;
  cout << "Finding approximate fit, taking into account volume ";
  // volume accounting on head
  for(unsigned4 pos = 0 ; pos < length_e ; pos ++)
    buffer_e[pos]=(unsigned4)buffer_e[pos]*pos/(unsigned4)length_e;
  // find global match
  for(unsigned4 pos = 0 ; pos < length_d-length_e; pos += step ) 
    {
      unsigned4 diff = 0;
      compressed * buffer_dprime = (compressed*)(buffer_d+pos);
      for(unsigned4 y = 0 ; y < length_e && diff < mindiff; y ++)
	diff+=(unsigned4)labs((signed4)buffer_dprime[y]*(length_e-y)/length_e
			      -(signed4)buffer_e[y]);
      if (diff<mindiff)
	{
	  mindiff = diff;
	  minpos = pos;
	  if (verbose) 
	    {
	      cout << minpos << " ";
	      cout.flush();
	    }
	}
    }
  step*=4;
  if (minpos < step)
    minpos = step;
  if (minpos + step > length_d - length_e)
    minpos = length_d - length_e - step;
  cout << "\nFinding volume accounted fine fit ";
  for(unsigned4 pos = minpos - step ; pos < minpos + step; pos ++) 
    {
      unsigned4 diff = 0;
      compressed * buffer_dprime = (compressed*)(buffer_d+pos);
      for(unsigned4 y = 0 ; y < length_e && diff < mindiff; y ++)
	{
	  diff+= (unsigned4)labs(
				 (signed4)buffer_dprime[y]*(length_e-y)/length_e
				 -(signed4)buffer_e[y]
				 );
	}
      if (diff<mindiff)
	{
	  mindiff = diff;
	  minpos = pos;
	  if (verbose) 
	    {
	      cout << " " << minpos;
	      cout.flush();
	    }
	}
    }
  cout << endl;
  return minpos*COLLAPSE;
}

unsigned4 findMatchWithoutVolumeAccounting()
{
  unsigned int step = 10;
  // find global match
  unsigned4 mindiff=(unsigned4)-1;
  unsigned4 minpos=0;
  cout<< "Finding approximate fit ";
  for(unsigned4 pos = 0 ; pos < length_d-length_e; pos += step ) 
    {
      unsigned4 diff = 0;
      compressed * at = (compressed*)(buffer_d+pos);
      compressed * bt = (compressed*)(buffer_e);
      compressed * st = (compressed*)(buffer_e+length_e);
      while(bt<st && diff < mindiff)
	{
	  diff+=(unsigned4)abs((signed4)*at-(signed4)*bt);
	  at++;
	  bt++;
	}
      if (diff<mindiff)
	{
	  mindiff = diff;
	  minpos = pos;
	  if (verbose) 
	    {
	      cout << minpos << " ";
	      cout.flush();
	    }
	}
    }
  step*=4;
  if (minpos < step)
    minpos = step;
  if (minpos + step > length_d - length_e)
    minpos = length_d - length_e - step;
  cout << "\nFinding fine fit ";
  for(unsigned4 pos = minpos - step ; pos < minpos + step; pos ++) 
    {
      unsigned4 diff = 0;
      compressed * at = (compressed*)(buffer_d+pos);
      compressed * bt = (compressed*)(buffer_e);
      compressed * st = (compressed*)(buffer_e+length_e);
      while(bt<st && diff < mindiff)
	{
	  diff+=(unsigned4)abs((signed4)*at-(signed4)*bt);
	  at++;
	  bt++;
	}
      if (diff<mindiff)
	{
	  mindiff = diff;
	  minpos = pos;
	  if (verbose)
	    {
	      cout << " " << minpos;
	      cout.flush();
	    }
	}
    }
  cout << endl;
  return minpos*COLLAPSE;
}

unsigned4 findMatch()
{
  if (account_volume)
    return findMatchWithVolumeAccounting();
  else
    return findMatchWithoutVolumeAccounting();
}

stereo_sample2 mix(stereo_sample2 a, stereo_sample2 b, signed8 vol, signed8 tot)
{
  stereo_sample2 c;
  c.left = (signed8)a.left * vol / tot
    + (signed8)b.left * (tot-vol) / tot;
  c.right = (signed8)a.right * vol / tot
    + (signed8)b.right * (tot-vol) / tot;
  return c;
}

void volumefade(signed8 pos)
{
  cout << "Creating the mix (pos = "<<pos<<")\n";
  signed8 start=(signed8)position_a+(signed8)sizeof(unsigned4)*(signed8)pos;
  signed8 end = start+sizeof(unsigned4)*length_c;
  signed8 centre = (start+end)/2;
  assert(centre>=start);
  log_entry(start,"M",old_index,new_index);
  log_entry(centre,"C",old_index,new_index);
  for(signed8 x = 0 ; x < length_c; x ++)
    buffer_a[x+pos]=mix(buffer_c[x],buffer_a[x+pos],x,length_c);
  fseek(file_a,start,SEEK_SET);
  writesamples(buffer_a+pos,length_c,file_a);
}

void tempocopy(unsigned4 target_period)
{
  buffer_b=bpmdj_allocate(period_b,stereo_sample2);
  buffer_c=bpmdj_allocate(target_period,stereo_sample2);
  readsamples(buffer_b,period_b,file_b);
  for(unsigned4 i = 0 ; i < target_period; i ++ )
    buffer_c[i]=buffer_b[(signed8)i*(signed8)period_b/(signed8)target_period];
  signed4 written = writesamples(buffer_c, target_period, file_a);
  assert(written == (signed)target_period);
  bpmdj_deallocate(buffer_b);
  bpmdj_deallocate(buffer_c);
}

void tempofade(int time)
{
  signed4 now=0;
  assert(time>=0);
  signed4 pos = ftell(file_b);
  signed4 maxmeasures = (filelength_b - pos)/4;
  maxmeasures/=period_b;
  if (time > maxmeasures)
    cout<<"Warning: tempo fade can only be done in "
	<< (time=maxmeasures)
	<< " measures\n";
  cout<< "Tempo fading during "<<time << " measures\n";
  log_entry(ftell(file_a),"T",new_index);
  while(now<time)
    {
      signed4 target_period = period_b-period_a;
      target_period*=now++;
      target_period/=time;
      target_period+=period_a;
      tempocopy(target_period);
    }
}

void closeFiles()
{
  fclose(file_a);
  fclose(file_b);
  char* rawname = getRawFilename("./",filename_b);
  if (verbose)
    cout<<"Removing "<<rawname <<endl;
  remove(rawname);
}

/**
 * Two separate behaviors are offered by this program
 * - the first is to merge 2 songs together
 * - the second is to split one large song in separate pieces
 */
int main(int argc, char* argv[])
{
  openlog();
  process_options(argc,argv);
  config = new PlayerConfig();
  assert(config);
  if (split)
    {
      split_raw();
      return 0;
    }
  if (createFiles(old_index,new_index))
    {
      readTail(slide_measures);
      readHead(start_pct,mix_measures);
      collapseBuffers();
      rescanTempo();
      stretchHead(mix_measures);
      collapseStretchedBuffer();
      signed8 position = findMatch();
      volumefade(position);
      tempofade(tempo_measures);
    }
  copySong(stop_pct);
  closeFiles();
  closeLog();
  return 0;
}
#endif // __loaded__bpmmerge_cpp__
