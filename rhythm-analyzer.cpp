/****
 BpmDj v4.1: Free Dj Tools
 Copyright (C) 2001-2010 Werner Van Belle

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
#ifndef __loaded__rhythm_analyzer_cpp__
#define __loaded__rhythm_analyzer_cpp__
using namespace std;
#line 1 "rhythm-analyzer.c++"
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
#include <fftw3.h>
#include <qmessagebox.h>
#include <sys/ioctl.h>
#include <qspinbox.h>
#include <unistd.h>
#include <libgen.h>
#include <qslider.h>
#include <time.h>
#include <sys/times.h>
#include <math.h>
#include <assert.h>
#include <math.h>
#include "memory.h"
#include "rhythm-analyzer.h"
#include "rhythm-property.h"
#include "bpmplay.h"
#include "version.h"
#include "scripts.h"
#include "pca.h"
#include "clock-drivers.h"

void fft_to_bark(float8 * in_r, int window_size, spectrum_type &out)
{
  for(int b = 0 ; b < barksize ; b ++)
    {
      int a = (int)barkbounds[b];
      int c = (int)barkbounds[b+1];
      a *= window_size;
      c *= window_size;
      a /= WAVRATE;
      c /= WAVRATE;
      if (c==a)
	{
	  printf("a = %d; b=%d, c=%d\n",a,b,c);
	  assert(c!=a);
	}
      out.set_bark(b,0);
      float8 r = 0;
      for(int i = a ; i < c; i++)
	r+=fabs(in_r[i]);
      out.set_bark(b,r/(c-a));
    } 
};

RhythmAnalyzer::RhythmAnalyzer(QWidget*parent) : QWidget(parent)
{
  setupUi(this);
  QPixmap *pm = new QPixmap(10,10);
  QPainter p;
  p.begin(pm);
  QRect r(QRect(0,0,pm->width(),pm->height()));
  p.fillRect(r,Qt::white);
  p.end();
  rhythm->setPixmap(*pm);
}

static float8 tovol(float8 a)
{
  float8 r = 10.0*log(a)/log(10.0);
  if (isnan(r)) return 0;
  if (r<-30) return 0;
  return r+30;
}

float8 xy_dist(float8 x1, float8 y1, float8 x2, float8 y2)
{
  float8 a = x1 - x2;
  float8 b = y1 - y2;
  a *= a;
  b *= b;
  return sqrt(a+b);
}

void wavelet_subtraction_test(long slice_size, unsigned4* phases, 
			      int maximum_slice, const char* target)
{
  int window_size = higher_power_of_two(slice_size);
  maximum_slice--;
  Signal<signed2,2> slice(window_size);
  Signal<float8,2>  pattern(window_size);
  pattern.clear();
  
  // read file, rotate and place in pattern array
  SignalIO<signed2,2> i(openCoreRawFile(),false);
  for(int x = 0 ; x < maximum_slice ; x++)
    {
      i.readSamples(slice,x*slice_size);
      Shift<signed2,2> shifted(slice,phases[x]);
      Signal<float8,2> waved(window_size);
      Haar<signed2,float8,2> h(shifted,waved,true);
      h.execute();
      waved.absolute();
      pattern.add(waved);
    }
  
  pattern.divide(maximum_slice);
  //  pattern.normalize_abs_max();
  //  pattern.multiply(32768);
  
  Signal<float8,2> wave_pattern(window_size);
  Haar<float8,float8,2> forward1(pattern,wave_pattern,true);
  forward1.execute();
  
  // read file, rotate and place in pattern array
  SignalIO<signed2,2> o(target,"wb");
  for(int x = 0 ; x < maximum_slice ; x++)
    {
      printf("%d/%d\n",x,maximum_slice);
      i.readSamples(slice,x*slice_size);
      Shift<signed2,2> shifted(slice,phases[x]);
      Signal<float8,2> new_wave(window_size);
      Haar<signed2,float8,2> forward(shifted,new_wave,true);
      forward.execute();
      for(int k = 0 ; k < window_size ; k++)
	for(int l = 0 ; l < 2 ; l++)
	  {
	    float8 T = new_wave.get(k,l);
	    float8 W = fabs(wave_pattern.get(k,l));
	    if (T > 0)
	      new_wave.set(k,l,T-W > 0 ? T-W : 0);
	    else
	      new_wave.set(k,l,T+W < 0 ? T+W : 0);
	  }
      //      new_wave-=wave_pattern;
      Haar<float8,float8,2> backward(new_wave,new_wave,false);
      backward.execute();
      new_wave.normalize_abs_max();
      new_wave.multiply(32768);
      Shift<float8,2> S(new_wave,-phases[x]);
      o.writeSamples(S,slice_size);
    }
}

void subtraction_test(long slice_size, unsigned4* phases, int maximum_slice, 
const char* target)
{
  int window_size = higher_power_of_two(slice_size);
  maximum_slice--;
  Signal<signed2,2> slice(window_size);
  Signal<float8,2>  pattern(window_size);
  pattern.clear();
  
  // read file, rotate and place in pattern array
  SignalIO<signed2,2> f(openCoreRawFile(),false);
  for(int x = 0 ; x < maximum_slice ; x++)
    {
      f.readSamples(slice,x*slice_size);
      Shift<signed2,2> shifted(slice,phases[x]);
      pattern.add(shifted);
    }
  
  pattern.normalize_abs_max();
  pattern.multiply(32768);
  
  // read file, rotate and place in pattern array
  SignalIO<signed2,2> out(target,"wb");
  for(int x = 0 ; x < maximum_slice ; x++)
    {
      f.readSamples(slice,x*slice_size);
      Signal<float8,2> slice2(slice);
      Shift<float8,2> shifted(pattern,-phases[x]);
      slice2-=shifted;
      out.writeSamples(slice2,slice_size);
    }
}

/**
 * Calculates the raw overlay pattern and writes it out to disk after shaping
 * the spectrum to fit the perfect spectrum.
 */
/*static float8 perfectspectrum[barksize] = 
  {
    7.48973,   8.60855,   7.30108,   6.08735,
    5.05958,   4.22398,   3.55564,   2.84894,
    1.99757,   1.37338,   0.814002,  0.479557,
   -0.125073, -0.522579, -0.939505, -1.68769,
   -2.2336,   -3.15377,  -4.26618,  -5.23179,
   -6.09273,  -7.06462,  -8.35773,  -10.1641
  };
*/

static float8 perfectspectrum[barksize] = 
  {
    9.9,   8.60855,   7.30108,   6.08735,
    5.05958,   4.22398,   3.55564,   2.84894,
    1.99757,   1.37338,   0.814002,  0.479557,
    -0.125073, -0.522579, -0.939505, -1.68769,
    -2.2336,   -3.15377,  -4.26618,  -5.23179,
    -6.09273,  -7.06462,  -8.35773,  -10.1641
  };

void shape(Signal<float8,2> &in)
{
  in.normalize_abs_max();
  HalfComplex<float8,2> *freq = new HalfComplex<float8,2>(in.length/2,
							  in.length/2);
  Fft<2> forward(in,*freq);
  forward.execute();
  Signal<float8,2> energy(in.length/2);
  Signal<float8,2> angle(in.length/2);
  freq->toPolar(energy,angle);
  Sample<float8,2> basis = 0;
  for(int b = 0 ; b < barksize ; b ++)
    {
      float8 a = barkbounds[b];
      float8 c = barkbounds[b+1];
      int l = (int)(a * in.length / 44100.0);
      int r = (int)(c * in.length / 44100.0);
      Sample<float8,2> e = 0;
      for(int i = l ; i < r ; i ++)
	{
	  // this strange construction is due to the tovol function
	  e[0]+=tovol(fabs(energy[i][0]*cos(angle[i][0])));
	  e[1]+=tovol(fabs(energy[i][1]*cos(angle[i][1])));
	}
      e/=r-l;
      if (b==0) basis=e;
      e-=basis;
      float8 f = perfectspectrum[b];
      printf("%g\t%g\n",e[0],f);
      e[0]=exp10((f-e[0])/10);
      e[1]=exp10((f-e[1])/10);
      printf("factor for %d is %g\n",b,e[0]);
      for(int i = l ; i < r ; i ++)
	energy.set(i,energy[i]*e);
    }
  
  float8 a = barkbounds[barksize];
  float8 c = 22050;
  int l = (int)(a * in.length / 44100.0);
  int r = (int)(c * in.length / 44100.0);
  for(int i = l ; i < r ; i ++)
    energy.set(i,0);
  
  freq->fromPolar(energy,angle);
  Fft<2> backward(*freq,in);
  backward.execute();
  delete(freq);
}

void pattern_shaped_test(long slice_size, unsigned4* phases, int maximum_slice,
const char* target)
{
  int window_size = higher_power_of_two(slice_size);
  maximum_slice--;
  Signal<signed2,2> slice(slice_size);
  Signal<float8,2>  pattern(window_size);
  pattern.clear();
  
  // read file, rotate and place in pattern array
  SignalIO<signed2,2> f(openCoreRawFile(),false);
  for(int x = 0 ; x < maximum_slice ; x++)
    {
      f.readSamples(slice,x*slice_size);
      Shift<signed2,2> shift(slice,phases[x]);
      pattern.add(shift);
    }
  
  shape(pattern);
  pattern.normalize_abs_max();
  
  // write out the pattern
  SignalIO<signed2,2> out(target,"wb");
  pattern.multiply(32767);
  out.writeSamples(pattern,slice_size);
  out.writeSamples(pattern,slice_size);
  out.writeSamples(pattern,slice_size);
  out.writeSamples(pattern,slice_size);
}  

/**
 * Calculates the raw overlay pattern and writes it out to disk 
 */
void pattern_test(long slice_size, unsigned4* phases, int maximum_slice, 
const char* target)
{
  int window_size = higher_power_of_two(slice_size);
  maximum_slice--;
  Signal<signed2,2> slice(window_size);
  Signal<float8,2>  pattern(window_size);
  pattern.clear();
  
  // read file, rotate and place in pattern array
  SignalIO<signed2,2> f(openCoreRawFile(),false);
  for(int x = 0 ; x < maximum_slice ; x++)
    {
      f.readSamples(slice,x*slice_size);
      Shift<signed2,2> shift(slice,phases[x]);
      pattern.add(shift);
    }
  
  pattern.normalize_abs_max();

  // multiply rotating pattern with the signal

  // write out the pattern
  SignalIO<signed2,2> out(target,"wb");
  pattern.multiply(32767);
  out.writeSamples(pattern,slice_size);
  out.writeSamples(pattern,slice_size);
  out.writeSamples(pattern,slice_size);
  out.writeSamples(pattern,slice_size);
}  


#ifdef TEST_RHYTHM_PROJECTIONS
void write_out_projection(long slice_size, unsigned4* phases, int maximum_slice,
const char* target)
{
  int frame_size = higher_power_of_two(2 * WAVRATE / 40);
  Signal<float8,2> curr_frame(frame_size);
  Signal<float8,2> prev_frame(frame_size);
  HalfComplex<float8,2> curr_spectr(frame_size/2,frame_size/2);
  HalfComplex<float8,2> prev_spectr(frame_size/2,frame_size/2);
  Fft<2> curr_forward(curr_frame,curr_spectr);
  //  Signal<float8,2> curr_energy(frame_size/2);
  //  Signal<float8,2> curr_angle(frame_size/2);
  //  Signal<float8,2> prev_energy(frame_size/2);
  SignalIO<signed2,2> f(openCoreRawFile());
  unsigned4 l = f.samples();
  curr_frame.clear();
  // we allocate the matrix
  int count = l/frame_size;
  float4 ** target_m = matrix(count,frame_size);
  for(unsigned4 x = 0 ; x + frame_size < l ; x+=frame_size)
    {
      // read current frame
      f.readSamples(curr_frame,x);
      curr_forward.execute();
      printf("%d/%d\n",(int)x/frame_size,count);
      fflush(stdout);
      for(int i = 0 ; i < frame_size ; i ++)
	target_m[(x/frame_size)+1][i+1]=curr_spectr.get(i)[0];
    }
  const char *ignore = NULL;
  printf("starting PCA\n");
  fflush(stdout);
  do_pca(count,frame_size,target_m,ignore);
  if (ignore) printf("%s\n",ignore);
  else
    for(unsigned4 x = 0 ; x + frame_size < l ; x+=frame_size)
      printf("%g %g\n",target_m[(x/frame_size)+1][1],
	     target_m[(x/frame_size)+1][2]);
  fflush(stdout);
  _exit(0);
}  

/**
 * In this routine we go through the song window size by window size and 
 * we subtract the energy of x windows ago, we keep the phase as it was. 
 */
void write_out_projection_hald_working(unsigned4 slice_size, unsigned4* phases,
int maximum_slice, const char* target)
{
  int frame_size = higher_power_of_two(2 * WAVRATE / 40);
  Signal<float8,2> curr_frame(frame_size);
  Signal<float8,2> prev_frame(frame_size);
  HalfComplex<float8,2> curr_spectr(frame_size/2,frame_size/2);
  HalfComplex<float8,2> prev_spectr(frame_size/2,frame_size/2);
  Fft<2> curr_forward(curr_frame,curr_spectr);
  Fft<2> prev_forward(prev_frame,prev_spectr);
  Fft<2> curr_backward(curr_spectr,curr_frame);
  Signal<float8,2> curr_energy(frame_size/2);
  Signal<float8,2> curr_angle(frame_size/2);
  Signal<float8,2> prev_energy(frame_size/2);
  SignalIO<signed2,2> f(openCoreRawFile());
  SignalIO<signed2,2> out(target,"wb");
  unsigned4 l = f.samples();
  curr_frame.clear();
  out.writeSamples(curr_frame,frame_size);
  for(unsigned4 x = 0 ; x + frame_size < l ; x+=frame_size/8)
    {
      printf("%d/%d\n",(int)x/frame_size,(int)l/frame_size);
      // read current frame
      f.readSamples(curr_frame,x);
      // read old frame
      if (x>slice_size)
	f.readSamples(prev_frame,x-slice_size);
      // convert both to their spectrum
      curr_forward.execute();
      prev_forward.execute();
      // measure the difference in energy and store it into 
      curr_spectr.toPolar(curr_energy,curr_angle);
      prev_spectr.toPolar(prev_energy);
      //      prev_energy.smooth(40);
      //      prev_energy.multiply(1.75);
      curr_energy-=prev_energy;
      curr_energy>=0;
      curr_energy.absolute();
      // change the energy of the new spectrum
      curr_spectr.fromPolar(curr_energy,curr_angle);
      // do backward transform
      curr_frame.clear();
      curr_backward.execute();
      curr_frame.divide(frame_size);
      // and write it out
      out.writeSamples(curr_frame,frame_size/8);
    }
  _exit(0);
}  

/**
 * Deconvolution really messes it up because we need to describe all the
 * remaining as superpositions of the entire wave... we don't want that. We 
 * want to filter out those frequencies. I still do have a problem with the 
 * theory: a single pulse convolved with the full measure = measure however, if
 * we divide this in the spectral domain by itself we get white noise, in which
 * all frequencies are present. this is the good thing, we really get white 
 * like noise. if we then remove any frequency smaller < 1  what we are looking
 * for is (* = convolution) a = p*s+r >==> A=P.S+R  we want s to be as sparse 
 * as possible and a the remainder we also want p to be the mean measure, so 
 * we now have again no sentence end :-)
 */
void write_out_projection_old(long slice_size, unsigned4* phases, 
int maximum_slice, const char* target)
{
  int     window_size = higher_power_of_two(slice_size);
  maximum_slice--;
  Signal<signed2,2> slice(window_size);
  Signal<float8,2> pattern(window_size);
  pattern.clear();
  
  // read file, rotate and place in pattern array
  printf("First run\n");
  SignalIO<signed2,2> f(openCoreRawFile());
  for(int x = 0 ; x < maximum_slice ; x++)
    {
      f.readSamples(slice,x*slice_size);
      Shift<signed2,2> shift(slice,phases[x]);
      pattern.add(shift);
    }

  pattern.normalize_abs_max();

  // convert to frequency domain
  HalfComplex<float8,2> spectrum(window_size/2,window_size/2);
  Fft<2> forward(pattern,spectrum);
  forward.execute();
  
  // we traverse the entire file
  int max_frames = f.samples();
  max_frames /=window_size;
  Signal<float8,2> frame(window_size);
  HalfComplex<float8,2> spec(window_size/2,window_size/2);
  Fft<2> warp(frame,spec);
  Fft<2> back(spec,frame);
  SignalIO<signed2,2> out(target,"wb");
  for(int x = 0 ; x < max_frames ; x++)
    {
      printf("%d/%d\n",x,max_frames);
      f.readSamples(frame,x*window_size);
      frame.normalize_abs_max();
      warp.execute();
      spec/=spectrum;
      back.execute();
      frame.normalize_abs_max();
      frame.multiply(32767);
      out.writeSamples(frame,window_size);
    }
  _exit(0);
  
  // when we have the pattern array, 
  // we convert it to its frequency domain, 
  // which forms the target phases
  /*  fftw_execute(plan_l);
  for(int i = 0 ; i < window_size ; i++)
    mean_l[i]=spectrum[i];
  fftw_execute(plan_r);
  for(int i = 0 ; i < window_size ; i++)
    mean_r[i]=spectrum[i];
  for(int i = 1 ; i < window_size/2 ; i++)
    {
      float8 lx = mean_l[i];
      float8 ly = mean_l[window_size-i];
      float8 rx = mean_r[i];
      float8 ry = mean_r[window_size-i];
      angle_l[i]=atan2(ly,lx);
      angle_r[i]=atan2(ry,rx);
    }
  
  // secondly, we go through the entire file again,
  // calculate the energies and obtain the angle variance
  // this time. This variance will afterward be used to alter
  // the strength of the output signal
  printf("Second run\n");
  f = openCoreRawFile();
  for(int x = 0 ; x < maximum_slice ; x++)
    {
      readsamples(slice,slice_size,f);
      for(int y = 0 ; y < slice_size ; y++)
	{
	  int z = ( y + phases[x] ) % slice_size;
	  pattern_l[y]=slice[z].left;
	  pattern_r[y]=slice[z].right;
	}
      printf("%d/%d\n",x,maximum_slice);
      // when we have it in the pattern array for the
      // first time, we convert it to its frequency domain
      // of which we then calculate the mean phase
      fftw_execute(plan_l);
      for(int i = 1 ; i < window_size / 2 ; i++)
	{
	  float8 x1 = spectrum[i];
	  float8 y1 = spectrum[window_size-i];
	  float8 angle = atan2(y1,x1);
	  angle-=angle_l[i];
	  angle=fabs(angle);
	  if (angle>M_PI) angle-=M_PI;
	  var_l[i]+=angle;
	}
      fftw_execute(plan_r);
      for(int i = 1 ; i < window_size / 2 ; i++)
	{
	  float8 x1 = spectrum[i];
	  float8 y1 = spectrum[window_size-i];
	  float8 angle = atan2(y1,x1);
	  angle-=angle_r[i];
	  angle=fabs(angle);
	  if (angle>M_PI) angle-=M_PI;
	  var_r[i]+=angle;
	}
    }
  fclose(f);

  // the final step is the normalization of the allowable
  // phase differences. We do this by finding the mean phase
  // then the deviation and scale everything from m-d to m+d
  float8 ml=0,mr=0;
  for(int i = 0 ; i < window_size/2 ; i++)
    {
      var_l[i]/=maximum_slice; 
      var_r[i]/=maximum_slice; 
      var_l[i]/=M_PI;
      var_r[i]/=M_PI;
      var_l[i]=1-var_l[i];
      var_r[i]=1-var_r[i];
    }
  normalize_mean(var_l,window_size/2);
  normalize_mean(var_r,window_size/2);

  // good this experiment doesn't work...
  // the variance is a bad measure...
  // we modify the strength of a specific frequency depending on its
  // variance. If it varies a lot then we remove it slightly
  for(int i = 1 ; i < window_size/2 ; i++)
    {
      float8 vl = var_l[i];
      float8 vr = var_r[i];
      vl*=vl;
      vr*=vr;

      vl*=vl;
      vr*=vr;

      vl*=vl;
      vr*=vr;

      vl*=vl;
      vr*=vr;
      mean_l[i]*=vl;
      mean_r[i]*=vr;
      mean_l[window_size-i]*=vl;
      mean_r[window_size-i]*=vr;
    }
  // now we can write out the inverse transform of the mean spectra
  // by storing everything back into the pattern and normalizing it
  // afterward
  printf("Creating mean stuff\n");
  fftw_execute(plan_lr);
  fftw_execute(plan_rr);
  normalize_abs_max(pattern_l,slice_size);
  normalize_abs_max(pattern_r,slice_size);

  bpmdj_deallocate(slice);
  
  out = fopen(target,"wb");
  for(int y = 0 ; y < slice_size ; y++)
    {
      signed2 t = (signed2)(pattern_l[y]*32767.0);
      fwrite(&t,2,1,out);
      t = (signed2)(pattern_r[y]*32767.0);
      fwrite(&t,2,1,out);
    }
  for(int y = 0 ; y < slice_size ; y++)
    {
      signed2 t = (signed2)(pattern_l[y]*32767.0);
      fwrite(&t,2,1,out);
      t = (signed2)(pattern_r[y]*32767.0);
      fwrite(&t,2,1,out);
    }
  for(int y = 0 ; y < slice_size ; y++)
    {
      signed2 t = (signed2)(pattern_l[y]*32767.0);
      fwrite(&t,2,1,out);
      t = (signed2)(pattern_r[y]*32767.0);
      fwrite(&t,2,1,out);
    }
  for(int y = 0 ; y < slice_size ; y++)
    {
      signed2 t = (signed2)(pattern_l[y]*32767.0);
      fwrite(&t,2,1,out);
      t = (signed2)(pattern_r[y]*32767.0);
      fwrite(&t,2,1,out);
    }
    fclose(out);*/
}
#endif

static fftw_plan plan;
static float8 *fft_out = NULL;
static float8 *fft_in = NULL;
static float8 *init_bark_fft2(int window_size)
{
  fft_in  = bpmdj_allocate(window_size,float8);
  fft_out = bpmdj_allocate(window_size,float8);
  plan = fftw_plan_r2r_1d(window_size,fft_in,fft_out,FFTW_R2HC,FFTW_MEASURE);
  return fft_in;
}

static void bark_fft2(int window_size, spectrum_type &bark_energy)
{
  fftw_execute(plan);
  fft_to_bark(fft_out,window_size,bark_energy);
}

static void free_bark_fft2()
{
  fftw_free(plan);
}

void RhythmAnalyzer::calculateRhythmPattern2()
{
  // check premises
  if (!normalperiod)
    { 
      QMessageBox::critical(NULL,
	    "Rhythm analyzer",
	    QString("Need tempo before rhythm can be analyzed"),
	    QMessageBox::Ok,0,0);
      return;
    }
  spectrum_type * spectrum = playing->get_spectrum();
  if (!spectrum)
    {
      QMessageBox::critical(NULL,
	    "Rhythm analyzer",
	    QString("Need spectrum before rhythm can be analyzed"),
	    QMessageBox::Ok,0,0);
      return;
    }
  // constants and some variables
  const int window_size = 2048;
  const int view_xs = 640;
  // how many frames will go into the period of the song
  int slice_samples = normalperiod * beats->value() / 4;
  int slice_frames = 192;
  // allocate arrays
  stereo_sample2 * slice_audio = bpmdj_allocate(slice_samples,stereo_sample2);
  spectrum_type * slice_freq = bpmdj_allocate(slice_frames,spectrum_type);
  spectrum_type * slice_prot = bpmdj_allocate(slice_frames,spectrum_type);
  spectrum_type * last_measure = bpmdj_allocate(slice_frames,spectrum_type);
  for(int i = 0 ; i < slice_frames ; i ++)
    for(int b = 0 ; b < 24 ; b++)
      {
	slice_prot[i].set_bark(b,0);
	last_measure[i].set_bark(b,0);
      }
  float8 * buffer_fft = init_bark_fft2(window_size);
  // open song
  FILE * raw = openCoreRawFile();
  long int audio_size = fsize(raw)/4;
  int max_slices = audio_size / slice_samples;
  spectrum_type * changes = bpmdj_allocate(max_slices,spectrum_type);
  for(int i = 0 ; i < max_slices ; i ++)
    for(int b = 0 ; b < 24 ; b ++)
      changes[i].set_bark(b,0);
  unsigned4 * phases = bpmdj_allocate(max_slices,unsigned4);
  // go through song slice by slice
  status_bar->setText("Calibrating");
  for(int slice = 0 ; slice < max_slices ; slice++)
    {
      Progress->setMaximum(max_slices);
      Progress->setValue(slice);

      if (slice==1) status_bar->setText("Processing");
      app->processEvents();
      // read data
      long int pos = slice * slice_samples;
      fseek(raw,pos*sizeof(stereo_sample2),SEEK_SET);
      assert(pos+slice_samples <= audio_size);
      int read = readsamples(slice_audio,slice_samples,raw);
      assert(read>0);
      for(int a = 0 ; a < slice_frames ; a ++)
	{
	  // take the appropriate data
	  int s = a * ( slice_samples - window_size )/ slice_frames;
	  assert(s + window_size < slice_samples);
	  for(int i = 0 ; i < window_size ; i++)
	    buffer_fft[i]=slice_audio[i+s].left+slice_audio[i+s].right;
	  // convert to spectrum and normalize
	  bark_fft2(window_size,slice_freq[a]);
	  for(int i = 0 ; i < 24 ; i ++)
	    {
	      float8 v;
	      v = slice_prot[a].get_bark(i);
	      assert(!isinf(v));
	      v = tovol(slice_freq[a].get_bark(i));
	      assert(!isinf(v));
	      slice_freq[a].set_bark(i,v);
	    }
	}
      // find best rotational fit towards prototype
      float8 m = HUGE;
      int a = 0;
      for(int p = 0 ; p < slice_frames ; p++)
	{
	  float8 d = 0;
	  for(int y = 0 ; y < slice_frames ; y++)
	    {
	      int z = (y + p)%slice_frames;
	      for(int b = 0 ; b < 24 && d < m; b ++)
		{
		  float8 v = slice_prot[y].get_bark(b) / slice;
		  float8 w = slice_freq[z].get_bark(b);
		  d += (v-w) * (v-w);
		}
	    }
	  if (d<m)
	    {
	      m = d;
	      a = p;
	    }
	}
      phases[slice]=a * ( slice_samples - window_size )/ slice_frames;
      // add the rotation to the prototype
      for(int y = 0 ; y < slice_frames; y++)
	{
	  int z = (y + a)%slice_frames;
	  for(int b = 0 ; b < 24 ; b++)
	    slice_prot[y].set_bark(b,slice_prot[y].get_bark(b)
				   +slice_freq[z].get_bark(b));
	}
      // find best rotational fit tov last measure
      if (slice>0)
	{
	  // store last measure and remember change level
	  for(int b = 0 ; b < 24 ; b++)
	    {
	      float8 d = 0;
	      for(int y = 0 ; y < slice_frames; y++)
		{
		  // float8 v = last_measure[y].get_bark(b);
		  float8 w = slice_freq[y].get_bark(b);
		  d += /* v - */ w;
		}
	      // 	      d = log(d);
	      if (d<0) d = 0;
	      changes[slice].set_bark(b,d);
	    }
	}
      for(int p = 0 ; p < slice_frames ; p++)
	for(int q = 0 ; q < 24 ; q ++)
	  last_measure[p].set_bark(q,slice_freq[p].get_bark(q));
    }
  
  Progress->setMaximum(1); 
  Progress->setValue(1);
  
  fclose(raw);
  
  if (!outfile->text().isEmpty())
    {
      status_bar->setText("Writing out projection");
      app->processEvents();
      pattern_shaped_test(slice_samples,phases,max_slices,
			  outfile->text().toAscii());
    }
  
  status_bar->setText("Normalizing");
  app->processEvents();

  // normalize the spectrum
  for(int f = 0 ; f < slice_frames ; f++)
    for(int b = 0 ; b < 24 ; b ++)
      slice_prot[f].set_bark(b,slice_prot[f].get_bark(b)/max_slices);
  spectrum_type maxima;
  spectrum_type mean;
  spectrum_type scale;
  for(int f = 0 ; f < slice_frames ; f++)
    for(int b = 0 ; b < 24 ; b ++)
      {
	float8 v = slice_prot[f].get_bark(b);
	mean.set_bark(b,mean.get_bark(b)+v);
	if (fabs(v) > maxima.get_bark(b))
	  maxima.set_bark(b,fabs(v));
      }
  for(int f = 0 ; f < slice_frames ; f++)
    for(int b = 0 ; b < 24 ; b ++)
      {
	float8 v = slice_prot[f].get_bark(b);
	float8 m = mean.get_bark(b) / slice_frames;
	v -=m;
	v /= maxima.get_bark(b) - m;
	scale.set_bark(b,maxima.get_bark(b) - m);
	slice_prot[f].set_bark(b,v);
      }

  // now we must find a good start on the music.
  // the start is the place after which most energy is packed 
  // as well now as in the longer future. 
  // so we take 1/16the surface and add it with 1/8the and 1/4th and 1/2
  int startpos = 0;
  float8 startenergy = 0;
  status_bar->setText("Locating start");
  app->processEvents();
  for(int a = 0 ; a < slice_frames ; a++)
    {
      float8 total = 0;
      for(int l = slice_frames/16 ; l < slice_frames ; l*=2)
	{
	  for(int f = 0 ; f < l ; f++)
	    {
	      int z = (a+f)%slice_frames;
	      for(int b = 0 ; b < 24 ; b ++)
		{
		  float8 v = slice_prot[z].get_bark(b);
		  total+=v;
		}
	    }
	}
      if (total>startenergy)
	{
	  startpos=a;
	  startenergy=total;
	}
    }

  // fix the rhythm information for this file
  rhythm_property R;
  R.init();
  for(int y = 0 ; y < 24 ; y++)
    {
      R.set_scale(y,scale.get_bark(y));
      for(int x = 0 ; x < rhythm_prop_sx ; x ++)
	{
	  int z = x * slice_frames / rhythm_prop_sx; 
	  z += startpos;
	  z %= slice_frames;
	  float8 dB = slice_prot[z].get_bark(y);
	  dB*=127;
	  dB+=127;
	  if (dB<0) dB=0;
	  if (dB>255) dB=255;
	  R.set_energy(x,y,(unsigned1)dB);
	}
    }
  playing->set_rhythm(R);

  // now draw the different distribution on the different axis
  QPixmap *pm = new QPixmap(view_xs, 24);
  QPixmap *pr = new QPixmap(view_xs, 1);
  QPainter p, q;
  p.begin(pm);
  q.begin(pr);
  for(int x = 0 ; x < view_xs ; x++)
    {
      int sum = 0;
      for(int y = 0 ; y < 24 ; y ++)
	{
	  int z = x * (slice_frames - 1) / ( view_xs - 1 );
	  z+=startpos;
	  z%=slice_frames;
	  float8 dB = slice_prot[z].get_bark(y);
	  QColor col;
	  /* This is one way of coloring */
	  dB*=192;
	  dB+=63;
	  if (dB<0) dB=0;
	  col.setHsv(y*240/24,255,(int)dB);
	  /* This is another 
	     dB*=255;
	     if (dB>255) dB=255;
	     if (dB<-255) dB=-255;
	     if (dB>=0)
	     col.setRgb((int)dB,0,0);
	     else if (dB<0)
	     col.setRgb(0,0,(int)-dB);
	  */
	  sum+=(int)dB;
	  
	  p.setPen(col);
	  p.drawPoint(x,23-y);
	}
      sum /= 24;
      q.setPen(QColor(sum,sum,sum));
      q.drawPoint(x,0);
    }
  p.end(); 
  q.end();
  rhythm->setPixmap(*pm);
  projection->setPixmap(*pr);
  
  // normalize the composition bar & draw it
  {
    for(int b = 0 ; b < 24 ; b++)
      changes[max_slices-1].set_bark(b,0);
    spectrum_type maxima;
    spectrum_type mean;
    // spectrum_type scale;
    for(int f = 0 ; f < max_slices ; f++)
      for(int b = 0 ; b < 24 ; b ++)
	{
	  float8 v = changes[f].get_bark(b);
	  mean.set_bark(b,mean.get_bark(b)+v);
	  if (fabs(v) > maxima.get_bark(b))
	    maxima.set_bark(b,fabs(v));
	}
    for(int f = 0 ; f < max_slices ; f++)
      for(int b = 0 ; b < 24 ; b ++)
	{
	  float8 v = changes[f].get_bark(b);
	  float8 m = mean.get_bark(b) / max_slices;
	  v -=m;
	  v /= maxima.get_bark(b) - m;
	  // scale.set_bark(b,maxima.get_bark(b) - m);
	  changes[f].set_bark(b,v);
	}
  }

  // draw the composition bar
  pm = new QPixmap(max_slices, 24);
  p.begin(pm);
  p.fillRect(QRect(0,0,max_slices,24),Qt::white);
  p.setPen(Qt::black);
  for(int x = 0 ; x < max_slices ; x++)
    {
      for(int y = 0 ; y < 24 ; y ++)
	{
	  float8 dB = changes[x].get_bark(y);
	  QColor col;
	  dB*=128;
	  dB+=127;
	  if (dB<0) dB=0;
	  col.setHsv(y*240/24,255,(int)dB);
	  p.setPen(col);
	  p.drawPoint(x,23-y);
	}
    }
  p.end(); 
  composition->setPixmap(*pm);

  // calculate the frequency content of the composition
  // we take 33 measures because the first will always be zero
  const int ps = 33; 
  int ws = higher_power_of_two(max_slices);
  if (ps>ws) 
    ws = higher_power_of_two(ps);
  float8 * ain = bpmdj_allocate(ws,float8);
  float8 * aou = bpmdj_allocate(ws,float8);
  float8 * aio = bpmdj_allocate(ws,float8);
  float8 * periods = bpmdj_allocate(ps,float8);
  // int *counts = bpmdj_allocate(ps,int);
  pm = new QPixmap(ps, 24);
  p.begin(pm);
  composition_property cp;
  cp.init();
  for(int y = 0 ; y < 24 ; y ++)
    {
      for(int x = 0 ; x < max_slices ; x++)
	ain[x]=changes[x].get_bark(y);
      for(int x = max_slices - 1; x >=1 ; x--)
	ain[x]-=ain[x-1];
      ain[0]=0;
      for(int x = max_slices - 1; x < ws ; x++)
	ain[x]=0;
      unbiased_autocorrelation(ain,ws);
      for(int pml = 0 ; pml < ps ; pml++)
	periods[pml]=ain[pml];
      periods[0]=0;
      float8 maxed = normalize_abs_max(periods,ps);
      // printf("%g\n",maxed);
      // assign the autocorrelation to the composition property
      cp.set_scale(y,maxed);
      for(int x = 1 ; x < ps ; x ++)
	{
	  float8 val = periods[x];
	  val*=127;
	  val+=127;
	  if (val<0) val = 0;
	  if (val>255) val = 255;
	  cp.set_energy(x-1,y,(unsigned1)val);
	}

      for(int x = 0 ; x < ps ; x++)
	{
	  float8 dB = periods[x];
	  QColor col;
	  if (dB<0) dB=-dB;
	  dB*=255;
	  col.setHsv(y*240/24,255,(int)dB);
	  p.setPen(col);
	  p.drawPoint(x,23-y);
	}
    }
  // the data which we will write out in the index file are the non 
  // absolute periods
  bpmdj_deallocate(aio);
  bpmdj_deallocate(aou);
  bpmdj_deallocate(ain);
  p.end(); 
  composition_freq->setPixmap(*pm);
  playing->set_composition(cp);
  
  status_bar->setText("Done");
  free_bark_fft2();
}
#endif // __loaded__rhythm_analyzer_cpp__
