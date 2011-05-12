/****
 BpmDj: Free Dj Tools
 Copyright (C) 2001-2005 Werner Van Belle

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

#include <assert.h>
#include <math.h>
#include <stdlib.h>
#include "histogram-type.h"
#include "common.h"
#include "memory.h"
#include "signals.h"
#include "files.h"

histogram_type::histogram_type()
{
  left = 0;
  right = 0;
  sum = 0;
  scale = 0;
  total = 0;
  median = 0;
  bins = NULL;
  count = 0;
}

histogram_type::histogram_type(double l, double r, int c)
{
  init(l,r,c);
}

void histogram_type::init(double l, double r, int c)
{
  left = l;
  right = r;
  count = c;
  assert(right>left);
  bins = allocate(c,signed4);
  for(int i = 0 ; i < count; i++)
    bins[i]=0;
  sum = 0;
  total = 0;
}

int histogram_type::bin(double val)
{
  double x = (val - left) / (right - left);
  x *= count;
  int b = (int)x;
  if (b>=0 && b <count)
    return b;
  return -1;
}

void histogram_type::hit(double val)
{
  int b = bin(val);
  if (b>0)
    {
      sum+=val;
      total++;
      bins[b]++;
    }
}

void histogram_type::normalize(int val)
{
  int p = 0;
  int m = bins[0];
  for(int i = count-1 ; i >=0 ; i --)
    {
      if (bins[i]>m)
	{
	  p = i;
	  m = bins[i];
	}
    }
  median = p;
  vector_mul_div(bins,count,val,m);
}

void histogram_type::strip()
{
  int a = 0;
  int b = count - 1;
  while(bins[a]==0) a++;
  while(bins[b]==0) b--;
  double new_left = left + a * (right-left) / count;
  double new_right = left + (b+1) * (right-left) / count;
  int    new_count = b-a+1;
  signed4 * new_bins = allocate(new_count,signed4);
  for(int i = a ; i <= b ; i++)
    new_bins[i-a]=bins[i];
  // printf("Stripped histogram from %d to %d\n",count,new_count);
  deallocate(bins);
  bins = new_bins;
  left = new_left;
  right = new_right;
  count = new_count;
}

double histogram_type::dev()
{
  double m = mean();
  double dev = 0;
  unsigned8 hits = 0;
  for(int i = 0; i < count ; i++)
    {
      hits+=bins[i];
      double d = (i*(right-left)/count) - m;
      dev+=d*d*bins[i];
    }
  dev/=hits;
  dev=sqrt(dev);
  return dev;
}

int histogram_type::valat(double v)
{
  int b = bin(v);
  if (b>=0)
    return bins[b];
  return 0;
}

/*double histogram_type::median_scaled(double mean)
{
  double answer = left + (right-left)*((double)median+0)/(double)count;
  return mean;
}*/

double histogram_type::mean()
{
  return sum/total;
}


void histogram_type::normalize_autocorrelation_diff(int val)
{
  // convert it to doubles
  int ns = higher_power_of_two(count);
  double *in = allocate(ns,double);
  for(int i = 0; i < count ;i++)
    in[i]=bins[i];
  for(int i = count ; i < ns; i++)
    in[i]=0;
  unbiased_autocorrelation(in,ns);
  normalize_abs_max(in,ns);
  for(int i = 0 ; i < ns ; i++)
    in[i]=sqrt(fabs(in[i]));
  differentiate(in,ns);
  scale = find_abs_max(in,ns);
  normalize_abs_max(in,ns);
  for(int i = 0 ; i < count ; i++)
    bins[i]=(int)(in[i]*(double)val);
  deallocate(in);
}

double histogram_type::best_dist(histogram_type *a)
{
  int m = count > a->count ? count : a->count;
  double *A = allocate(m,double);
  double *B = allocate(m,double);
  for(int i = 0 ; i < m; i++)
    B[i]=A[i]=0;
  for(int i = 0; i < m ;i++)
    {
      if (i < count)
	A[i]=bins[i];
      if (i < a->count)
	B[i]=a->bins[i];
    }
  return biased_best_abs_circular_distance(A,B,m);
}

double histogram_type::cor_dist(histogram_type *a)
{

  // convert it to doubles
  int m = count > a->count ? count : a->count;
  int ns = higher_power_of_two(m);
  double *A = allocate(ns,double);
  double *B = allocate(ns,double);
  for(int i = 0 ; i < ns; i++)
    B[i]=A[i]=0;
  for(int i = 0; i < m ;i++)
    {
      if (i < count)
	A[i]=bins[i];
      if (i < a->count)
	B[i]=a->bins[i];
    }
  unbiased_autocorrelation(A,ns);
  unbiased_autocorrelation(B,ns);
  
  normalize_abs_max(A,ns);
  normalize_abs_max(B,ns);
  for(int i = 0 ; i < ns ; i ++)
    {
      A[i]=sqrt(fabs(A[i]));
      B[i]=sqrt(fabs(B[i]));
    }
  
  differentiate(A,ns);
  differentiate(B,ns);

  double d = unbiased_abs_distance(A,B,ns);
  return d/2.0;
}

void histogram_type::halve()
{
  count/=2;
  for(int i = 1 ; i < count ; i++)
    bins[i]=(bins[2*i]+bins[2*i+1])/2;
}

// --------------------------------------------------------------------
void smallhistogram_type::init(histogram_type * other)
{
  scale = other->scale;
  bin = allocate(count=96,unsigned1);
  for(int i = 0 ; i < 96; i++)
    if (i<other->count)
      bin[i]=abs(other->bins[i]);
    else
      bin[i]=0;
}

void smallhistogram_type::init()
{
  bin = allocate(count=smallhistogram_size,unsigned1);
  for(int i = 0 ; i < count; i++)
    bin[i]=0;
}

smallhistogram_type::smallhistogram_type()
{
}

const void smallhistogram_type::write_idx(FILE*f)
{
  int c = count-1;
  while(bin[c]==0) c--;
  fprintf(f,"%g %d ",scale,c+1);
  for(int i = 0 ; i <= c ; i++)
    fprintf(f,"%2x ",bin[i]);
  fprintf(f,"\n");
}

//static long winst = 0;
void smallhistogram_type::read_idx(const char* str)
{
  char * cur;
  char * nxt;
  scale = strtof(str,&cur);
  count = strtol(cur,&nxt,10);
  cur=nxt;
  assert(count<=smallhistogram_size);
  if (count<0)
    count = 0;
  //winst+=smallhistogram_size-count;
  //printf("%d\n",winst);
  bin = allocate(count,unsigned1);
  for(int i = 0 ; i < count ; i++)
    {
      bin[i]=strtol(cur,&nxt,16);
      cur = nxt;
    }
}

const void smallhistogram_type::write_bib_v272(FILE * index)
{
  file_float4(scale,index);
  file_unsigned1(count,index);
  file_sequence(bin,count,index);
}

void smallhistogram_type::read_bib_v271()
{
  scale = buffer_float4();
  bin = allocate(count=smallhistogram_size,unsigned1);
  buffer_sequence(bin,count);
}

void smallhistogram_type::read_bib_v272()
{
  scale = buffer_float4();
  count = buffer_unsigned1();
  assert(count>=0);
  bin = allocate(count,unsigned1);
  buffer_sequence(bin,count);
}

