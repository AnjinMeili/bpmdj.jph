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
#ifndef __BPMDJ___RYTHM_ANALYZER_H__
#define __BPMDJ___RYTHM_ANALYZER_H__
using namespace std;
#line 1 "rythm-analyzer.h++"
#include "ui-rythm.h"
#include "analyzer.h"
#include "types.h"

class SongPlayer;
class RythmAnalyzer : 
  public QWidget, 
  public Ui::RythmAnalyzer, 
  public Analyzer
{
  Q_OBJECT
public:
  RythmAnalyzer(QWidget * parent=0);
public slots:
  virtual void calculateRythmPattern2();
  virtual void analyze() { calculateRythmPattern2();};
  virtual void on_calculate_clicked() { start(); };
};

void write_out_projection(long slice_size, unsigned4 *phases, int maximum_slice, const char * target);
#endif