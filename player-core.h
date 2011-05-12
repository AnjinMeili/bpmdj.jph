/****
 BpmDj: Free Dj Tools
 Copyright (C) 2001-2004 Werner Van Belle
 See 'BeatMixing.ps' for more information

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

#ifndef PLAYER_CORE_H
#define PLAYER_CORE_H
#include "index.h"
#include "dsp-drivers.h"

/*-------------------------------------------
 *         Wavrate conversion routines
 *-------------------------------------------*/ 
#define  WAVRATE  (44100)
#define  samples2ms(samples) (int)(1000*(samples)/WAVRATE)
#define  bytes2ms(bytes) (int)samples2ms((bytes)/4)
#define  ms2samples(ms) (int)((ms)*WAVRATE/1000)
#define  ms2bytes(ms) (int)(4*ms2samples(ms))
#define  samples2s(samples) (int)(samples)/WAVRATE
#define  mperiod2bpm(period) (4.0*(double)WAVRATE*60.0/(double)(period))

/*-------------------------------------------
 *         Other prullaria
 *-------------------------------------------*/ 
#define  wave_bufsize (32L*1024L)
// one period is the length of 1 measure
extern  signed8 targetperiod;
extern  signed8 currentperiod;
extern  signed8 normalperiod;
// x is the data position in the raw file (thus at normal tempo)
// y is the position in the playing file (thus at target tempo)
extern  signed8 y,x;
volatile extern  int stop;
volatile extern  int finished;
volatile extern  int paused;
typedef unsigned8 cue_info;
extern  cue_info cue;
extern  cue_info cues[4];
extern  char* wave_name;
extern  Index* playing;
unsigned4 wave_max();
unsigned8 x_normalise(unsigned8 y);
unsigned8 y_normalise(unsigned8 x);
void      cue_set();
void      cue_store(char*, int);
void      cue_retrieve(char*, int);
void      jumpto(signed8, int);
void      changetempo(signed8);
void      cue_shift(char*, signed8);
void      doubleperiod();
void      halveperiod();

extern dsp_driver *dsp;

#define   err_none    0
// initializes the index fields
// and start reading the audio
#define   err_needidx 1
#define   err_noraw   2
#define   err_nospawn 3
int       core_init(int synchronous);
// opens the playing device
// #define   err_mixer   4
#define   err_dsp     5
int       core_open();
// plays until asked to stop
void      core_play();
// closes the playing device
void      core_close();
// closes the wave and writes any changes to the index 
void      core_done();

unsigned4 lfo_no(unsigned4 x);
unsigned4 lfo_saw(unsigned4 x);
unsigned4 lfo_pan(unsigned4 x);
unsigned4 lfo_break(unsigned4 x);
unsigned4 lfo_difference(unsigned4 x);
unsigned4 lfo_revsaw(unsigned4 x);
unsigned4 lfo_metronome(unsigned4 x);
typedef unsigned4 (*_lfo_)(unsigned4 x);
void  lfo_set(char* name, _lfo_ l, unsigned8 freq, unsigned8 phase);
_lfo_ lfo_get();

typedef struct t_segment {
  signed2 take_from;
  signed2 speed_mult;
  signed2 speed_div;
  signed2 volume; // per 100
} map_segment;
typedef map_segment* map_data;

#define map_exit_restart -1
#define map_exit_continue -2
#define map_exit_stop -1000
// the exit is expressed at the normal speed of playing
void map_set(signed2 map_size, map_data m, unsigned8 size, signed8 exit, bool loop);
void map_loop_set(bool l);
void map_stop();

extern int   opt_quiet;
extern int   opt_match;
extern char* arg_match;
extern int   opt_dspverbose;
extern int   opt_latency;
extern char* arg_latency;
//extern char* arg_mixer;
extern char* arg_rawpath;
extern char* argument;

void line();

extern int app_init(int argc, char *argv[]);
extern void process_options(int argc,char *argv[]);
extern void msg_slowdown(int c);
extern void msg_speedup(int c);
#endif
