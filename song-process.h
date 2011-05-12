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
#ifndef __BPMDJ___SONG_PROCESS_H__
#define __BPMDJ___SONG_PROCESS_H__
using namespace std;
#line 1 "song-process.h++"
#include <qcolor.h>
#include <qstring.h>
#include <qcheckbox.h>
#include <qcombobox.h>
#include <qlineedit.h>
#include <qpushbutton.h>
#include <qhbox.h>
#include "index.h"
#include "basic-process-manager.h"

class Song;
class AnalyzersManager;

class SongProcess: public QObject
{
  Q_OBJECT ;
 public:
  enum command_type { empty, standard, xmms };
  enum enabled_type { disabled, enabling, ok };
  enum state_type   { unchecked, checking, usefull, useless };
  enum kind_type    { player, analyzer };
 private:
  kind_type kind;             // player or analyzer ?
  int     id;                 // the player or analyzer ID
  QString name;               // the name of this analyzer/player
  command_type cmd;           // the command prefix
  QString remote;             // the remote location of the player, if any
  enabled_type estate;        // true if the user wants to use this process
  QString text;               // thename of the playing / analyzing song
  Song  * song;               // the song being analyzed/played;
  int     running_time;       // the current running time
  long    total_running_time; // the cummulated running time
  int     songs_finished;     // the amount of stopped songs
  time_t  started_at;         // start_time when running
  state_type state;           // whether this one is useable, useless or not yet checked
 private:
  void init();
  void setText(QString t);
  void setEstate(enabled_type state);
  void setUsefullState();
  void setUselessState();
 public:
  SongProcess();
  // accessors
  enabled_type enabledState() const 
    {
      return estate;
    };
  void setEnabled(bool enabled = true);
  bool isEnabledOk() const 
    {
      return estate==ok;
    };
  QString getText() const 
    {
      return text;
    };
  bool canRun() const 
    {
      return estate==ok && song==NULL;
    };
  bool isAnalyzer() const 
    {
      return kind == analyzer; 
    };
 private: 
  QString getBasicCommand() const;
 public:
  command_type getCmd() const 
    {
      return cmd; 
    };
  QString getPlayCommand(Index & matchto, Index & song) const;
  QString getAnalCommand(bool tempo, int bpmtechnique, double from, double to, bool spectrum, bool energy, bool rythm, QString song) const;
  void    setAnalyzer() 
    {
      kind = analyzer;
    };
  void    setOldCommand(QString s);
  void    setCommand(command_type cmd);
  QString getRemote() const 
    {
      return remote;
    };
  void    setRemote(QString remote);
  bool    isEmpty() const 
    {
      return cmd == empty; 
    };
  bool    isBusy() const 
    {
      return song != NULL; 
    };
  void    setSong(Song * s);
  Song *  getSong() const 
    {
      return song;
    };
  void    reread();
  int     get_running_time() const 
    {
      return running_time; 
    };
  int     inc_running_time();
  float   songs_per_second() const;
  void    start(Song * song);
  void    stop();
  void    setNameId(QString name,int id);
  void    setName(QString name);
  QString getName() const 
    {
      return name;
    };
  int     getId()   const 
    {
      return id;
    };
  // checking of the quality of these process parameters
  void    startChecking();
  void    checkerDied();
  QString getLogName();
 public slots:
  void setup();
 signals:
  void stateChanged();
  void viewChanged();
  void timeChanged();
};

class CheckersManager: public BasicProcessManager
{
  private:
    SongProcess ** songprocesses;
    virtual void clearId(int id);
  public:
    CheckersManager();
    virtual ~CheckersManager() {};
    virtual void start(SongProcess *sp, QString command, QString log);
};

extern CheckersManager checkers;

class SongSelectorAnalView: public QCheckBox
{
   Q_OBJECT ;
   SongProcess * song_process;
   AnalyzersManager * anal_processes;
   float   report_time;             // the color die out time
   float   relative_running_time();
 public:
   SongSelectorAnalView(QWidget * parent, AnalyzersManager * a, SongProcess & s);
 private slots:
   void updateBacking();
 public slots:
   void processChange();
   void colorChange();
 signals:
   void songKilled(Song *);
};

class SongSelectorPlayView: public QCheckBox
{
   Q_OBJECT ;
   SongProcess * song_process;
 public:
   SongSelectorPlayView(QWidget * parent, SongProcess & s);
 private slots:
   void updateBacking();
 public slots:
   void processChange();
};

class SongProcPrefView: public QHBox
{
   Q_OBJECT;
 protected:
   QComboBox   * cmd_box;
   QLineEdit   * name_edit;
   QLineEdit   * remote_edit;
   SongProcess * song_process; 
 private:
   QPushButton * configure_button;
   void update_disable_enable();
 public:
   SongProcPrefView(QWidget * parent, SongProcess & s);
 public slots:
   void commandChanged();
};
#endif