/****
 BpmDj: Free Dj Tools
 Copyright (C) 2001 Werner Van Belle
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

#include <unistd.h>
#include <qinputdialog.h>
#include <math.h>
#include <qmessagebox.h>
#include <qlcdnumber.h>
#include <ctype.h>
#include <qlabel.h>
#include <qlistview.h>
#include <qgroupbox.h>
#include <qdir.h>
#include <qfiledialog.h>
#include <qmultilineedit.h>
#include <qlineedit.h>
#include <qcheckbox.h>
#include <qcursor.h>
#include <qtabwidget.h>
#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <errno.h>
#include <sys/mount.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <libgen.h>
#include "importscanner.h"
#include "renamer.logic.h"
#include "about.h"
#include "tagbox.h"
#include "qsong.h"
#include "kbpm-played.h"
#include "askinput.h"
#include "version.h"
#include "kbpm-dj.h"
#include "bpmbounds.h"
#include "process-manager.h"
#include "similarscanner.h"

extern "C"
{
#include "cbpm-index.h"
#include "edit-distance.h"
}

int  SongSelectorLogic::itemcount=0;

SongSelectorLogic::SongSelectorLogic(QWidget * parent, const QString name) :
  SongSelector(parent,name)
{
  // link with the processmanager
  processManager = new ProcessManager(this);
  // initialise fields
  mainTicks=0;
  timer=new QTimer(this);
  connect(timer,SIGNAL(timeout()), SLOT(timerTick()));
  timer->start(1000);

  nextTagLine=0;
  itemcount=0;
  
  // create menu structure
  QMenuBar *main = new QMenuBar(this);
  QPopupMenu *help = new QPopupMenu(this);
  QPopupMenu *before = new QPopupMenu(this);
  QPopupMenu *after = new QPopupMenu(this);
  QPopupMenu *tools = new QPopupMenu(this);
  QPopupMenu *file = new QPopupMenu(this);
  selection = new QPopupMenu(this);
  view = new QPopupMenu(this);

  // file menu
  file->insertItem("&Quit",this,SLOT(quitButton()));
  // help menu
  help->insertItem("&About",this,SLOT(doAbout()));
  help->insertItem("&Read Manual",this,SLOT(doHelp()));
  help->insertItem("Online &Manual",this,SLOT(doOnlineHelp()));
  help->insertItem("&Legende",this,SLOT(doLegende()));
  help->insertItem("L&icense",this,SLOT(doLicense()));
  
  // before menu
  before->insertItem("&Import mp3's",this,SLOT(importMp3s())); // retrieves from ./music all mp3's and creates index files for them
  before->insertItem("Find similar song names... (experimental)",this,SLOT(findsimilarnames()));
  before->insertItem("Find all similar song names... (experimental)",this,SLOT(findallsimilarnames()));
  before->insertItem("Find wrong mp3 file names...",this,SLOT(findWrongMp3Names()));
  
  // after menu
  after->insertItem("&Check/import cdrom content",this,SLOT(checkDisc())); // checks wether the songs on a disk can be found somewhere in the indices
  after->insertItem("Mark duplicates (experimental)",this,SLOT(doMarkDups())); // searches the current list and marks all MD5sum duplicates
  after->insertItem("Find wrong index file names...",this,SLOT(findWrongIdxNames()));
  
  // tools menu
  tools->insertItem("&Preferences",this,SLOT(doPreferences()));

  // view color menu
  view->insertSeparator();
  coloralreadyplayed_item = view->insertItem("Color already played titles",this,SLOT(toggle_coloralreadyplayed()));
  colorauthorplayed_item  = view->insertItem("Color already played authors",this,SLOT(toggle_colorauthorplayed()));
  colorinrange_item       = view->insertItem("Color songs within tempo range",this,SLOT(toggle_colorinrange()));
  colorsongondisk_item    = view->insertItem("Color songs not on disk",this,SLOT(toggle_colorsongsnotondisk()));
  colorcues_item          = view ->insertItem("color songs without cues",this,SLOT(toggle_colorsongswithoutcues()));
  colordcolor_item        = view -> insertItem("color spectrum distance",this, SLOT(toggle_colordcolor()));
  colorspectrum_item      = view -> insertItem("color spectrum",this,SLOT(toggle_colorspectrum()));
  view->setItemChecked(coloralreadyplayed_item, Config::color_played);
  view->setItemChecked(colorinrange_item      , Config::color_range);
  view->setItemChecked(colorauthorplayed_item , Config::color_authorplayed);
  view->setItemChecked(colorsongondisk_item   , Config::color_ondisk);
  view->setItemChecked(colorcues_item         , Config::color_cues);
  view->setItemChecked(colordcolor_item       , Config::color_dcolor);
  view->setItemChecked(colorspectrum_item     , Config::color_spectrum);

  // view display part
  view->insertSeparator();
  temporange_item         = view->insertItem("Limit selection to &Tempo range",this,SLOT(toggle_temporange()));
  onlyuptemporange_item   = view->insertItem("&Up Tempo Range (0 < BPM < 0.06)",this,SLOT(toggle_onlyuptemporange()));
  onlydowntemporange_item = view->insertItem("&Down Tempo Range (-0.06 < BPM < 0)",this,SLOT(toggle_onlydowntemporange()));
  notyetplayed_item       = view->insertItem("Limit selection to &Non-played songs",this,SLOT(toggle_notyetplayed()));
  onlyondisk_item         = view->insertItem("Limit selection to &Files on disk",this,SLOT(toggle_onlyondisk()));
  onlyindistance_item     = view->insertItem("Limit selection to &dColor range",this,SLOT(toggle_onlyindistance()));
  onlyinspectrum_item     = view->insertItem("Limit selection to &Spectrum range",this,SLOT(toggle_onlyspectrum()));
  view->setCheckable(true);
  view->setItemChecked(onlyondisk_item,         Config::limit_ondisk);
  view->setItemChecked(notyetplayed_item,       Config::limit_nonplayed);
  view->setItemChecked(onlyuptemporange_item,   Config::limit_uprange);
  view->setItemChecked(onlydowntemporange_item, Config::limit_downrange);
  view->setItemChecked(onlyindistance_item,     Config::limit_indistance);
  view->setItemChecked(onlyinspectrum_item,     Config::limit_inspectrum);

  // selection menu
  selection->insertItem("&Add tag...",this,SLOT(selectionAddTags()));
  selection->insertItem("&Delete tag...",this,SLOT(selectionDelTags()));
  selection->insertItem("&Measure Bpm ...",this,SLOT(measureBpms()));
  selection->insertItem("Measure Sound Color ...",this,SLOT(measureSpectra()));
  selection->insertSeparator();
  selection->insertItem("Play in 3th player",this,SLOT(selectionPlayIn3th()));
  selection->insertItem("This _is_ the main song",this,SLOT(selectionSetMainSong()));
  selection->insertSeparator();
  selection->insertItem("&Fetch selection from cdrom...",this,SLOT(fetchSelection()));
  selection->insertItem("E&xport xmms playlist",this,SLOT(exportPlayList()));
  selection->insertSeparator();
  selection->insertItem("Select all b&ut tagged",this,SLOT(selectAllButTagged()));
  
  // main menu
  main->insertItem("&File",file);
  main->insertItem("&View",view);
  main->insertItem("&Selection",selection);
  main->insertItem("&Before",before);
  main->insertItem("&After",after);
  main->insertItem("&Tools",tools);
  main->insertItem("&Help",help);

  // nog een aantal items toevoegen
  //   create remote/local script pair
  //   R- copy index files to other side
  //   R- position of binaries
  //   R- start player
  //   R- position to start player
  //   R- copy files back
  //   L- give shell to start here
  // mounten van music directory
};

void SongSelectorLogic::toggle_notyetplayed()
{
  toggleItem(notyetplayed_item);
}

void SongSelectorLogic::toggle_coloralreadyplayed()
{
  toggleItem(coloralreadyplayed_item);
}

void SongSelectorLogic::toggle_colorauthorplayed()
{
  toggleItem(colorauthorplayed_item);
}

void SongSelectorLogic::toggle_colorsongsnotondisk()
{
  toggleItem(colorsongondisk_item);
}

void SongSelectorLogic::toggle_colorsongswithoutcues()
{
  toggleItem(colorcues_item);
}

void SongSelectorLogic::toggle_colordcolor()
{
  toggleItem(colordcolor_item);
}

void SongSelectorLogic::toggle_colorspectrum()
{
  toggleItem(colorspectrum_item);
}

void SongSelectorLogic::toggle_onlyuptemporange()
{
  toggleItem(onlyuptemporange_item);
}

void SongSelectorLogic::toggle_onlydowntemporange()
{
  toggleItem(onlydowntemporange_item);
}

void SongSelectorLogic::toggle_temporange()
{
  toggleItem(onlyuptemporange_item);
  toggleItem(onlydowntemporange_item);
}

void SongSelectorLogic::toggle_colorinrange()
{
  toggleItem(colorinrange_item);
}

void SongSelectorLogic::toggle_onlyondisk()
{
  toggleItem(onlyondisk_item);
}

void SongSelectorLogic::toggle_onlyindistance()
{
  toggleItem(onlyindistance_item);
}

void SongSelectorLogic::toggle_onlyspectrum()
{
  toggleItem(onlyinspectrum_item);
}

void SongSelectorLogic::toggleItem(int which)
{
  view->setItemChecked(which,!view->isItemChecked(which));
  doFilterChanged();
}

void SongSelectorLogic::doHelp()
{
  if (!fork())
    {
      system("gv /usr/share/doc/bpmdj/beatmixing.ps");
      exit(0);
    }
}

void SongSelectorLogic::doOnlineHelp()
{
  QMessageBox::warning(this,"Online","Please browse to http://bpmdj.sourceforge.net/");
}

void SongSelectorLogic::findAllTags()
{
  // add all tags
  addTag("");
  QListViewItemIterator it(songList);
  for(;it.current();++it)
    {
      QSong * song = (QSong*)it.current();
      song->getColors();
      parseTags(song->song_tags);
    }
  // enable the first everything tag
  tagInclude[0] -> setChecked(true);
  // update the item list
  updateItemList();
}

void SongSelectorLogic::parseTags(QString tagz)
{
  char * original = strdup(tagz);
  char * tags = original;
  if (!tags) return;
  char * runner=tags;
  while(*runner)
    {
      while(*runner && *runner!=' ') runner++;
      if (*runner) 
	{
	  *runner=0;
	  addTag(tags);
	  *runner=' ';
	  tags=++runner;
	}
      else
	{
	  addTag(tags);
	}
    }
  free(original);
}

bool SongSelectorLogic::doFilter(QSong* song)
{
  if (filter(song))
    {
      itemcount++;
      return true;
    }
  else return false;
}

static bool alreadygavefilterwarning = false;

void SongSelectorLogic::updateItemList()
{
  itemcount=0;
  QListViewItemIterator it(songList);
  for(;it.current();++it)
    {
      QSong *svi=(QSong*)it.current();
      bool vis = doFilter(svi);
      svi->setVisible(vis);
      if (vis)
	svi->repaint();
    }
  songList->repaint();
  countLcd->display(itemcount);
  if (itemcount==0 && !alreadygavefilterwarning)
    {
      QMessageBox::warning(this,"Empty Selection","With the current selection of filters,\n"
			   "there are no songs to be displayed.\n"
			   "If you want to modify this, go to the View menu.");
      alreadygavefilterwarning = true;
    }
}

bool SongSelectorLogic::lookfor(const QString w) 
{ 
  QListViewItemIterator it(songList);
  for(;it.current();++it)
    {
      QSong *song = (QSong*)it.current();
      if (song->song_file == w)
	return true;
    }
  return false;
};

bool SongSelectorLogic::filter(QSong *item)
{
  if (ProcessManager::playingInMain!=NULL && ProcessManager::playingInMain->song_author==item->song_author)
    {
      item->played_author_at_time = Played::songs_played;
    }
  // we must obtain the distance first because this value is updated on the fly
  bool indistance = item->getDistance();
  if (view->isItemChecked(onlyindistance_item) && !indistance)
    return false;
  // song on disk ?
  if (view->isItemChecked(onlyondisk_item) && !item->song_ondisk)
    return false;
  // in spectrum range ?
  if (view->isItemChecked(onlyinspectrum_item))
    {
      if (ProcessManager::playingInMain && !ProcessManager::playingInMain->song_spectrum.isNull())
	{
	  QSong *main= ProcessManager::playingInMain;
	  float distance  = fabs(main->hue-item->hue);
	  if (360-distance < distance)
	    distance = 360-distance;
	  if (distance>=60)
	    return false;
	}
    }
  // song played ?
  if (view->isItemChecked(notyetplayed_item) && item->song_played)
    return false;
  // check tag matching
  bool matched=false;
  for (int i=0; i<nextTagLine; i++)
    if (tagInclude[i]->isChecked())
      if (item->containsTag(tagLines[i]->text()))
	matched=true;
  if (!matched) return false;
  for (int i=0; i<nextTagLine; i++)
    if (tagExclude[i]->isChecked())
      if (item->containsTag(tagLines[i]->text()))
	matched=false;
  if (!matched) return false;
  // tempo 
  if (item->song_tempo && (view->isItemChecked(onlyuptemporange_item) ||
			   view->isItemChecked(onlydowntemporange_item)))
    {
      double t=atof(item->song_tempo);
      if (!view->isItemChecked(onlydowntemporange_item))
	if (t<processManager->mainTempo) return false;
      if (t<processManager->mainTempo*0.94) return false;
      if (!view->isItemChecked(onlyuptemporange_item))
	if (t>processManager->mainTempo) return false;
      if (t>processManager->mainTempo*1.06) return false;
    }
  // search
  const QString lookingfor = searchLine -> text();
  if (strcmp(lookingfor,"")!=0)
    {
      QString title=item->song_title.upper();
      QString author=item->song_author.upper();
      if (title.contains(lookingfor)+author.contains(lookingfor)==0)
	return false;
    }
  return true;
}

void SongSelectorLogic::timerTick()
{
   mainLCD->display(++mainTicks);
   if (mainTicks==1)
     setColor(QColor(0,255,0));
   else if (mainTicks==Config::yellowTime)
     setColor(QColor(255,255,0));
   else if (mainTicks==Config::orangeTime)
     setColor(QColor(255,127,0));
   else if (mainTicks==Config::redTime)
     setColor(QColor(255,0,0));
   processManager->checkSignals();
}

void SongSelectorLogic::setColor(QColor color)
{
   countLcd->setBackgroundColor(color);
   mainLCD->setBackgroundColor(color);
}

void SongSelectorLogic::setPlayerColor(QGroupBox *player ,QColor color)
{
  player->setPaletteBackgroundColor(color);
}

static QColor getColorFor(QCheckBox *onoff, QCheckBox *main, QLabel *title)
{
  QColor unavailable(127+32,127+32,127+32);
  QColor available(192+32,192+32,192+32);
  QColor playing(192,255,192);
  QColor mainc(127,255,127);
  
  if (onoff->isChecked())
    {
      if (main->isChecked())
	return mainc;
      else if (title->text()==QString::null)
	return available;
      else 
	return playing;
    }
  else
    return unavailable;
}

void SongSelectorLogic::playersChanged()
{
  // here we currently only look at the enabled or disabled property of the players...
  //  setPlayerColor(player1, getColorFor(onoff1, ismain1, title1));
  // setPlayerColor(player2, getColorFor(onoff2, ismain2, title2));
  //setPlayerColor(player3, getColorFor(onoff3, ismain3, title3));
  //setPlayerColor(player4, getColorFor(onoff4, ismain4, title4));
}

void SongSelectorLogic::resetCounter()
{
  mainTicks=-1;
  timerTick();
}

void SongSelectorLogic::switchMonitorToMain()
{
  // forced switch
  processManager->switchMonitorToMain();
}

void SongSelectorLogic::updateProcessView()
{
  // set main song
  QSong * song = processManager->playingInMain;
  QColor color;
  color.setHsv(0,0,128);
  if (song)
    {
      title1->setText(song->song_title);
      author1->setText(song->song_author);
      color.setHsv(song->hue,song->sat,255);
    }
  else
    {
      title1->setText(QString::null);
      author1->setText(QString::null);
    }
  setPlayerColor(player1,color);
  // set monitor song
  song = processManager->playingInMonitor;
  color.setHsv(0,0,128);
  if (song)
    {
      title2->setText(song->song_title);
      author2->setText(song->song_author);
      color.setHsv(song->hue,song->sat,255);
    }
  else
    {
      title2->setText("");
      author2->setText("");
    }
  setPlayerColor(player2,color);
  // set main tempo 
  char tmp[500];
  sprintf(tmp,"%g",processManager->mainTempo);
  mainTempoText->setText(tmp);
  // update item list
  updateItemList();
}

int songFileCompare(const void * a, const void* b)
{
  QSong **pa=(QSong**)a;
  QSong **pb=(QSong**)b;
  QSong *A=*pa;
  QSong *B=*pb;
  return strcmp(A->song_file,B->song_file);
}

void SongSelectorLogic::checkDisc()
{
  // 0 - which CD is this ?
  AskWhichCd whichCd(NULL,NULL,TRUE);
  if (whichCd.exec()==whichCd.Rejected) return;
  // 1 - mount cdrom
  system("mount /cdrom");
  // 2 - Scan directory
  ImportScanner scanner(this);
  scanner.scan("/cdrom",whichCd.LineEdit->text());
  updateItemList();
  // 3 - umount cdrom
  system("umount /cdrom");
  system("eject /cdrom");
}

void SongSelectorLogic::exportPlayList()
{
   /* count nr of files to get */
   int selectionCount=0;
   QListViewItemIterator it1(songList);
   for(;it1.current();++it1)
     {
	QSong *svi=(QSong*)it1.current();
	if (svi->isSelected() && svi->isVisible()) selectionCount++;
     }
   /* allocate enough space to store the songs */
   QSong * * toFetch;
   int next=0;
   toFetch=(QSong**)calloc(selectionCount,sizeof(QSong*));
   assert(toFetch);
   next=0;
   /* read all the songs */
   QListViewItemIterator it2(songList);
   for(;it2.current();++it2)
     {
	QSong *svi=(QSong*)it2.current();
	if (svi->isSelected() && svi->isVisible())
	  toFetch[next++]=svi;
     }
   /* write script to disk */
   char* homedir = getenv("HOME");
   char scriptname[1024];
   sprintf(scriptname,"%s/.xmms/xmms.m3u",homedir);
   char * current_directory=getcwd(NULL,0);
   FILE* script=fopen(scriptname,"wb");
   assert(script);
   int i = 0;
   while(i<next)
     {
       QSong* song=toFetch[i++];
       const char* filename=song->song_file;
       fprintf(script,"%s/./music/%s\n",current_directory,filename);
     }
   fclose(script);
   free(current_directory);
}

void SongSelectorLogic::fetchSelection()
{
  /* count nr of files to get */
  int selectionCount = 0;
  QListViewItemIterator it1(songList);
  for(;it1.current();++it1)
    {
      QSong *svi=(QSong*)it1.current();
      if (svi->isSelected() && svi->isVisible()) selectionCount++;
    }
  /* allocate enough space to store the songs */
  QSong * * toFetch;
  int next=0;
  toFetch=(QSong**)calloc(selectionCount,sizeof(QSong*));
  assert(toFetch);
  next=0;
  /* read all the songs */
  QListViewItemIterator it2(songList);
  for(;it2.current();++it2)
    {
      QSong *svi=(QSong*)it2.current();
      if (svi->isSelected()  && svi->isVisible())
	toFetch[next++]=svi;
    }
  /* sort the list */
  qsort(toFetch,next,sizeof(QSong*),songFileCompare);
  /* write script to disk */
  FILE* script=fopen("fetchfiles.sh","wb");
  assert(script);
  fprintf(script,"#!/bin/bash\n");
  fprintf(script,"#writing %d files\n",next);
  fprintf(script,"umount /cdrom\n");
  fprintf(script,"eject /cdrom\n");
  int i = 0;
  char* currentcd=0;
  while(i<next)
    {
      QSong* song=toFetch[i++];
      const char* filename=song->song_file;
      char* newcd=strdup(filename);
      char temp[500];
      char* breakat=index(newcd,'/');
      assert(breakat);
      *breakat=0;
      if (currentcd==0 || strcmp(currentcd,newcd)!=0)
	{
	  if (currentcd)
	    {
	      free(currentcd);
	      fprintf(script,"umount /cdrom\n");
	      fprintf(script,"eject /cdrom\n");
	    }
	  currentcd=newcd;
	  fprintf(script,"dialog --msgbox \"Please insert %s\" 10 40\n",newcd);
	  fprintf(script,"mount /cdrom\n");
	}
      else 
	free(newcd);
      char* dirc=strdup(filename);
      char* filenc=strdup(filename);
      char* dir=dirname(dirc);
      char* filen=basename(filenc);
      sprintf(temp,"\"./music/%s\"",dir);
      char* target=strdup(temp);
      sprintf(temp,"mkdir -p %s",target);
      fprintf(script,"%s\n",temp);
      sprintf(temp,"\"/cdrom%s\"",index(filename,'/'));
      char* source=strdup(temp);
      sprintf(temp,"cp %s %s\n",source,target);
      fprintf(script,temp);
      free(source);
      free(target);
      free(dirc);
      free(filenc);
    }
  fprintf(script,"umount /cdrom\n");
  fclose(script);
  
  /* umount any left over cdrom */
  system("umount /cdrom");
  system("eject /cdrom");
  /* fetch the files one by one */
  i = 0;
  currentcd = 0;
  while(i<next)
    {
      QSong* song=toFetch[i++];
      const char* filename=song->song_file;
      /* read the cd-entry */
      char* newcd=strdup(filename);
      char temp[500];
      char* breakat=index(newcd,'/');
      assert(breakat);
      *breakat=0;
      if (currentcd==0 || strcmp(currentcd,newcd)!=0)
	{
	  /* umount if necessarry */
	  if (currentcd)
	    {
	      free(currentcd);
	      if (system("umount /cdrom")<0)
		QMessageBox::warning(this,"CD","Cannot umount /cdrom");
	      system("eject /cdrom");
	    }
	  /* ask user to input new cd */
	  currentcd=newcd;
	  sprintf(temp,"Please insert %s",newcd);
	  QMessageBox::information(this,"CD",temp);
	  /* mount the cdrom */
	  if (system("mount /cdrom")==-1)
	    {
	      sprintf(temp,"Cannout mount /cdrom (errno=%d)",errno);
	      QMessageBox::warning(this,"CD",temp);
	    }
	}
      else 
	free(newcd);
      /* create the target directory */
      char* dirc=strdup(filename);
      char* filenc=strdup(filename);
      char* dir=dirname(dirc);
      char* filen=basename(filenc);
      sprintf(temp,"\"./music/%s\"",dir);
      char* target=strdup(temp);
      sprintf(temp,"mkdir -p %s",target);
      system(temp);
      /* read the given file from the cdrom */
      sprintf(temp,"\"/cdrom%s\"",index(filename,'/'));
      char* source=strdup(temp);
      sprintf(temp,"cp %s %s\n",source,target);
      /* copy it */
      system(temp);
      /* mark it on disk */
      song->song_ondisk=true;
      /* free source & target */
      free(source);
      free(target);
      free(dirc);
      free(filenc);
    }
  /* finally umount the last cdrom */
  system("umount /cdrom");
}

void SongSelectorLogic::measureBpms()
{
  /* ask user the given bpm's */
  BpmBounds *bounds = new BpmBounds(this,0,true);
  int res = bounds->exec();
  if (res==QDialog::Rejected)
    return;
  char frombound[500];
  char tobound[500];
  if (bounds->From->text().isEmpty())
    sprintf(frombound,"");
  else 
    sprintf(frombound,"--low %s",(const char*)(bounds->From->text()));
  if (bounds->To->text().isEmpty())
    sprintf(tobound,"");
  else
    sprintf(tobound,"--high %s",(const char*)(bounds->To->text()));
  /* write out executable batch processing for every line */
  FILE* script=fopen("process_bpm.sh","wb");
  fprintf(script,"#!/bin/sh\n");
  assert(script);
  QListViewItemIterator it1(songList);
  for(;it1.current();++it1)
    {
      QSong *svi=(QSong*)it1.current();
      if (svi->isSelected()  && svi->isVisible()) 
	fprintf(script,"kbpm-play --batch %s %s \"%s\"\n",frombound,tobound,(const char*)svi->song_index);
    }
  fclose(script);
  chmod("process_bpm.sh",S_IRUSR | S_IWUSR | S_IXUSR);
  /* now start the sucker */
  if (!fork())
    {
      if (system("./process_bpm.sh")<=256) exit(100);
      printf("Error: couldn't execute process_bpm.sh\n");
      exit(100);
    }
}

void SongSelectorLogic::measureSpectra()
{
  QMessageBox::warning(this,"Measuring Song Colors",
		       "I will now start a backgroundprocess that measures the color of all the selected songs.\n"
		       "This can take a while. The color of a song is measured at the last used cue-position.\n"
		       "The new colors will only be imported after you restart the application");
  FILE* script=fopen("process_spectrum.sh","wb");
  fprintf(script,"#!/bin/sh\n");
  assert(script);
  QListViewItemIterator it1(songList);
  for(;it1.current();++it1)
    {
      QSong *svi=(QSong*)it1.current();
      if (svi->isSelected()  && svi->isVisible()) 
	fprintf(script,"kbpm-play --spectrum \"%s\"\n",(const char*)svi->song_index);
    }
  fclose(script);
  chmod("process_spectrum.sh",S_IRUSR | S_IWUSR | S_IXUSR);
  /* now start the sucker */
  if (!fork())
    {
      if (system("./process_spectrum.sh")<=256) exit(100);
      printf("Error: couldn't execute process_bpm.sh\n");
      exit(100);
    }
}


void SongSelectorLogic::selectSong(QListViewItem *song)
{
  int result;
  QString player;
  // if no song selected (clicked outside songable area)
  if (!song) return;
  // inform process manager
  processManager->startSong(((QSong*)song));
}

void SongSelectorLogic::doPreferences()
{
  Config::openUi();
}

void SongSelectorLogic::songAddTag(QListViewItem * S, const QString & tag)
{
  QSong * song = (QSong*)S;
  QString index = song->song_index;
  // read the index file
  index_read(index);
  // modify taglines
  QString newtags = song->song_tags+" "+tag;
  index_tags = strdup(newtags);
  // write the indfex file
  index_write();
  // free the bastard
  index_free();
  // now update the local songdata
  ((QSong*)song)->reread();
}

void SongSelectorLogic::songDelTag(QListViewItem *S, const QString & tag)
{
  QSong * song = (QSong*)S;
  char* newtags;
  // read the index file
  index_read(song->song_index);
  newtags=strdup(index_tags);
  // modify taglines
  char* pos = strstr(index_tags,(const char*)tag);
  if (pos)
    {
      // simply clear the sucker
      newtags[pos-index_tags]=' ';
      strcpy(newtags+(pos-index_tags)+1, pos+strlen(tag));
      index_tags = strdup(newtags);
    }
  // write the index file
  index_write();
  // free the bastard
  index_free();
  // now update the local songdata
  ((QSong*)song)->reread();
}

void SongSelectorLogic::selectionAddTags()
{
   // ask user the tag to add
   TagBox songdata(NULL,NULL,TRUE);
   songdata.newTags->setText("");
   if (songdata.exec()==QDialog::Accepted)
     {
	QListViewItemIterator it1(songList);
	for(;it1.current();++it1)
	  {
	    QSong *svi=(QSong*)it1.current();
	     if (svi->isSelected() && svi->isVisible()) 
	       songAddTag(svi,songdata.newTags->text());
	  }
	parseTags(songdata.newTags->text());
     }
}

void SongSelectorLogic::selectionPlayIn3th()
{
  QListViewItemIterator it1(songList);
  for(;it1.current();++it1)
    {
      QSong *svi=(QSong*)it1.current();
      if (svi->isSelected() && svi->isVisible()) 
	{
	  processManager->startSongPlayer3(svi);
	  return;
	}
    }
}

void SongSelectorLogic::selectionSetMainSong()
{
  QListViewItemIterator it1(songList);
  for(;it1.current();++it1)
    {
      QSong *svi=(QSong*)it1.current();
      if (svi->isSelected() && svi->isVisible()) 
	{
	  processManager->setMainSong(svi);
	  return;
	}
    }
}

void SongSelectorLogic::selectionDelTags()
{
   // ask user the tag to add
   TagBox songdata(NULL,NULL,TRUE);
   songdata.newTags->setText("");
   if (songdata.exec()==QDialog::Accepted)
     {
	QListViewItemIterator it1(songList);
	for(;it1.current();++it1)
	  {
	     QSong *svi=(QSong*)it1.current();
	     if (svi->isSelected() && svi->isVisible()) 
	       songDelTag(svi,songdata.newTags->text());
	  }
	parseTags(songdata.newTags->text());
     }
}

void SongSelectorLogic::doMarkDups()
{
   int nr=0;
   QListViewItemIterator it1(songList);
   for(;it1.current();++it1)
     {
	QSong *svi=(QSong*)it1.current();
	if (svi->isSelected() && svi->isVisible())
	  {
	    QListViewItemIterator it2(songList);
	    QString txt=svi->song_md5sum;
	    printf("%d -- ",nr++);
	    bool found=false;
	    for(;it2.current();++it2)
	      {
		QSong *svi2=(QSong*)it2.current();
		if (svi!=svi2 
		    && txt==svi2->song_md5sum)
		  {
		    found=true;
		    svi2->setSelected(true);
		    break;
		  }
	      }
	    if (!found)
	      svi->setSelected(false);
	  }
     }
}

void SongSelectorLogic::doLegende()
{
  doAbout(2);
}

void SongSelectorLogic::doLicense()
{
  doAbout(1);
}

void SongSelectorLogic::doAbout(int pg)
{
  char tmp[500];
  AboutDialog about(NULL,NULL,1);
  sprintf(tmp,"BpmDj v%s",VERSION);
  about.versionLabel->setText(tmp);
  about.pages->setCurrentPage(pg);
  about.exec();
}

void SongSelectorLogic::doAbout()
{
  doAbout(0);
}

void SongSelectorLogic::doFilterChanged()
{
  Config::color_range = view->isItemChecked(colorinrange_item);
  Config::color_played = view->isItemChecked(coloralreadyplayed_item);
  Config::color_authorplayed = view->isItemChecked(colorauthorplayed_item);
  Config::color_ondisk = view->isItemChecked(colorsongondisk_item);
  Config::color_cues = view->isItemChecked(colorcues_item);
  Config::color_dcolor = view->isItemChecked(colordcolor_item);
  Config::color_spectrum = view->isItemChecked(colorspectrum_item);
  Config::limit_ondisk = view->isItemChecked(onlyondisk_item);
  Config::limit_nonplayed = view->isItemChecked(notyetplayed_item); 
  Config::limit_uprange = view->isItemChecked(onlyuptemporange_item);
  Config::limit_downrange = view->isItemChecked(onlydowntemporange_item);
  Config::limit_indistance = view->isItemChecked(onlyindistance_item);
  Config::limit_inspectrum = view->isItemChecked(onlyinspectrum_item);     
  searchLine -> setText(searchLine->text().upper());
  updateItemList();
}

void SongSelectorLogic::quitButton()
{
   close();
}
			
void SongSelectorLogic::selectAllButTagged()
{
   int i = 1;
   while(i<nextTagLine)
     tagExclude[i++] -> setChecked(true);
}

void SongSelectorLogic::addTag(const QString tag)
{
  int baseline=40;
  int col=0, row=0;
  int xbox=120;
  int i=0;
  /* does the tag already exist */
  for(i=0;i<nextTagLine;i++)
    if (tagLines[i]->text()==tag) 
      return;
  /* no, add it */
  if (nextTagLine>=MAXTAGS) return;
  assert(nextTagLine<MAXTAGS);
  col=nextTagLine/MAXTAGSPERCOL;
  row=nextTagLine-col*MAXTAGSPERCOL;
  tagLines[nextTagLine]   =  new QLabel( GroupBox2, "" );
  tagLines[nextTagLine]   -> setGeometry( QRect( 50+col*xbox, baseline+30*row, 110, 30 ) );
  tagLines[nextTagLine]   -> setText( tag );
  tagLines[nextTagLine]   -> show();
  tagInclude[nextTagLine] =  new QCheckBox( GroupBox2, "" );
  tagInclude[nextTagLine] -> setGeometry( QRect( 10+col*xbox, baseline+30*row, 20, 30 ) );
  tagInclude[nextTagLine] -> setText( tr( "" ) );
  tagInclude[nextTagLine] -> show();
  tagExclude[nextTagLine] =  new QCheckBox( GroupBox2, "" );
  tagExclude[nextTagLine] -> setGeometry( QRect( 30+col*xbox, baseline+30*row, 20, 30 ) );
  tagExclude[nextTagLine] -> setText( tr( "" ) );
  tagExclude[nextTagLine] -> show();
  connect( tagInclude[nextTagLine], SIGNAL( stateChanged(int) ), this, SLOT( doFilterChanged() ) );
  connect( tagExclude[nextTagLine], SIGNAL( stateChanged(int) ), this, SLOT( doFilterChanged() ) );
  nextTagLine++;
  /* and now sort it */
  i=0;
  while(i<nextTagLine-1)
    {
      if ( tagLines[i]->text() > tagLines[i+1]->text() )
	{
	  QString txt = tagLines[i]->text();
	  tagLines[i]->setText(tagLines[i+1]->text());
	  tagLines[i+1]->setText(txt);
	  i=0;
	}
      else
	i++;
    }
}

void SongSelectorLogic::findallsimilarnames()
{
  /** WVB -- todo
   *  we should store all similar names into a list
   *  afterwaqrds, we should be able to 
   *   - fetch the timings for the song
   *   - delete it from disk
   *   - forget the entry
   */
  QString text = QFileDialog::getExistingDirectory(NULL,this,NULL,"Specify directory to compare with database");
  // ask the name to start with
  if (!text.isEmpty())
    {
      SimilarScanner *scanner = new SimilarScanner(this);
      scanner->scan(text);
    }
}

void SongSelectorLogic::findsimilarnames(const QString & text, const QString & fullname)
{
  assert(0);
}

void SongSelectorLogic::findsimilarnames()
{
  bool ok;
  // ask the name to start with
  QString tmp1 = "Input name to compare with";
  QString tmp2 = "label";
  QString tmp3 = "insert name";
  QString text = QInputDialog::getText(tmp1,tmp2,QLineEdit::Normal,tmp3,&ok,0,0);
  if (ok && !text.isEmpty())
    {
      dist_init();
      findsimilarnames(text,"");
      dist_done();
    }
}

void SongSelectorLogic::importMp3s()
{
  ImportScanner scanner(this);
  scanner.scan(MusicDir);
  updateItemList();
}

void SongSelectorLogic::selectionMenu()
{
  selection->exec(QCursor::pos());
}

void SongSelectorLogic::findWrongMp3Names()
{
  QString text = QFileDialog::getExistingDirectory(NULL,this,NULL,"Specify directory to look for wrong mp3 names");
  if (!text.isEmpty())
    {
      if (text.right(1)=="/")
	text = text.left(text.length()-1);
      RenamerLogic *renamer = new RenamerLogic(this);
      renamer->scan(text,text);
      renamer->show();
    }
}

void SongSelectorLogic::findWrongIdxNames()
{
  RenamerLogic *renamer = new RenamerLogic(this);
  renamer->scan(IndexDir,IndexDir);
  renamer->show();
}
