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
#ifndef __loaded__md5_analyzer_h__
#define __loaded__md5_analyzer_h__
using namespace std;
#include "analyzer-runner.h"
class Md5Analyzer: public AnalyzerRunner
{
 public:
  Md5Analyzer(): AnalyzerRunner() {};
  virtual void analyze();
};
#endif // __loaded__md5_analyzer_h__
