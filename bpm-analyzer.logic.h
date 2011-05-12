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
#ifndef __BPMDJ___BPM_ANALYZER_LOGIC_H__
#define __BPMDJ___BPM_ANALYZER_LOGIC_H__
using namespace std;
#line 1 "bpm-analyzer.logic.h++"
#include <qstring.h>
#include "kbpm-play.h"
#include "analyzer.h"
#include "songplayer.h"
#include "bpmcounter.h"

typedef double fft_type;

class BpmAnalyzerDialog : public CountDialog, public Analyzer
{
  Q_OBJECT
private:
  // fine scanning
   unsigned char * audio;
     signed long   audiosize;
   unsigned long   audiorate;
   unsigned long   startbpm, stopbpm;
   unsigned long   startshift, stopshift;
   long     int    bufsiz;
	    // progress indicator fields
            int    reading_progress;
            int    processing_progress;
	    // tapping fields
            int    tapcount;
            int    starttime;
	    // fft fields
        fft_type * freq;
	fft_type * peak_bpm;
	fft_type * peak_energy;
	    int  * peak_fit;
 	    int    windowsize;
	    int    peaks;
   void          set_measured_period(QString technique, int period);
            // Weighted enveloppe and autocorrelation
   void          wec();
	    // Enveloppe fft's
   void          enveloppe_spectrum();
   void          autocorrelate_spectrum();
	    // FFT guidance routines
   void          fft();        // do a quick fft to obtain a set of 'hints'
   void          fft_draw(QPainter &p, int xs, int ys, int shifter, double bpm_divisor);
   void          autocorrelate_draw(QPainter &p, int xs, int ys, int shifter);
            // finding the error fit of a curve
   unsigned long phasefit(unsigned long i);
   unsigned long phasefit(unsigned long i, unsigned long clip);
   void          rayshoot_scan();
   void          peak_scan();                    // scan based on fft-peaks
   void          readAudio();                    // reads the file in memory
   void          readAudioBlock(int blocksize);  // reads the file in memory divided by blocks
 private:
   void status(QString text);
   void set_labels();
   void updateReadingProgress(int);
   void updateProcessingProgress(int);
   void updateInfo();
 public:
   void          setBpmBounds(long start, long stop);
   void          getMd5();     // retrieves MD5 sum
   void          writeAudio(); // writes audio to disk
   void          analyze();
   void          rangeCheck();
   void          removeRaw();
   SongPlayer *  player;
   BpmAnalyzerDialog(SongPlayer*parent=0, const char * name=0, bool modal=FALSE, WFlags f=0);
   virtual void started();
   virtual void stop();
   virtual void stopped();
 public slots:
   virtual void startStopButton();
   virtual void finish();
   virtual void tap();
   virtual void reset();
};
#endif
