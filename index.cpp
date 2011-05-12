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
#line 1 "index.c++"
/*-------------------------------------------
 *         Headers
 *-------------------------------------------*/
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <termios.h>
#include <fcntl.h>
#include <libgen.h>
#include <ctype.h>
#include <signal.h>
#include <assert.h>
#include <qlabel.h>
#include <qlineedit.h>
#include <time.h>
#include <string.h>
#include <qstring.h>
#include "common.h"
#include "index.h"
#include "version.h"
#include "song-information.h"
#include "memory.h"
#include "spectrum-type.h"
#include "kbpm-dj.h"
#include "smallhistogram-type.h"
#include "histogram-property.h"

using namespace std;


/*-------------------------------------------
 *         Symbols used
 *-------------------------------------------*/
static Symbol key_bpmdj_version("version");
static Symbol key_file("file");
static Symbol key_author("author");
static Symbol key_title("title");
static Symbol key_remix("remix");
static Symbol key_tags("tag");
static Symbol key_version("number");
static Symbol key_cue("cue");
static Symbol key_cuez("cuez");
static Symbol key_cuex("cuex");
static Symbol key_cuec("cuec");
static Symbol key_cuev("cuev");
static Symbol key_min("min");
static Symbol key_max("max");
static Symbol key_mean("mean");
static Symbol key_power("power");
static Symbol key_albums("albums");
static Symbol key_echo("histogram");
static Symbol key_rythm("rythm");
static Symbol key_composition("composition");
static Symbol key_spectrum("spectrum");
static Symbol key_md5sum("md5sum");
static Symbol key_time("time");
static Symbol key_count("count");
static Symbol key_info("info");
static Symbol key_name("name");
static Symbol key_period("period");
static Symbol key_nr("nr");
static Symbol key_prev("prev");
static Symbol key_next("next");
       Symbol key_content("content");
       Symbol key_scale("scale");
static Symbol key_disabled_capacities("disabled_capacities");

static String VersionString(QString("BpmDj v"VERSION));

/*-------------------------------------------
 *         Index operations
 *-------------------------------------------*/
void Index::init()
{
  index_period = -1;
  index_file = "";
  index_tags = "";
  index_md5sum = "";
  meta_changed = 0;
  index_time = "";
  index_cue_z = 0;
  index_cue_x = 0;
  index_cue_c = 0;
  index_cue_v = 0;
  index_cue = 0;
  index_spectrum = NULL;
  index_histogram.clear();
  index_rythm.clear();
  index_composition.clear();
  title = "";
  author = "";
  remix = "";
  version = "";
  prev = bpmdj_allocate(1, HistoryField*);
  prev[0]=NULL;
  next = bpmdj_allocate(1,HistoryField*);
  next[0]=NULL;
  albums = bpmdj_allocate(1, AlbumField*);
  albums[0]=NULL;
  clear_energy();
  index_disabled_capacities = no_disabled_capacities;
}

void Index::free()
{
  meta_filename="";
  if (index_spectrum) 
    delete index_spectrum;
  assert(prev && next);
  while (*prev) 
    delete(*prev++);
  prev = NULL;
  
  while (*next) 
    delete(*next++);
  next = NULL;

  while (*albums) 
    delete(*albums++);
  albums = NULL;
}

Data Index::get_data(const period_type &p)
{
  return Signed4(p.period);
}

Data Index::get_data(HistoryField ** history)
{
  // first count them;
  int c=0;
  HistoryField* *tmp = history;
  while(*tmp) {c++; tmp++;};
  // allocate array
  Array<1,Data> history_data(c);
  c=0;
  tmp = history;
  while(*tmp) 
    {
      history_data[c]=(*tmp)->get_data(0);
      c++; 
      tmp++;
    };
  return history_data;
}

HistoryField::HistoryField(QString f)
{
  file = f;
  count = 0;
  comment = "";
}

Data HistoryField::get_data(int version)
{
  Token result;
  result[key_file]  = String(file);
  result[key_count] = Signed4(count);
  result[key_info]  = String(comment);
  return result;
}

void HistoryField::set_data(Data &data)
{
  Token token=data;
  file=(String)token[key_file];
  count=(Signed4)token[key_count];
  comment=(String)token[key_info];
}

Data AlbumField::get_data(int version) const
{
  Token result;
  result[key_name]=String(name);
  result[key_nr]=Signed4(nr);
  return result;
}

void AlbumField::set_data(Data &data)
{
  Token cast = data;
  name=(String)cast[key_name];
  nr=(Signed4)cast[key_nr];
}

void Index::write_idx()
{
   if (!meta_filename)
     {
       printf("Error: no name given to write index file to\n");
       return;
     }

   Token token;
   token[key_bpmdj_version]=VersionString;
   token[key_file]=String(index_file);
   token[key_title]=String(title);
   token[key_author]=String(author);
   token[key_remix]=String(remix);
   token[key_version]=String(version);
   token[key_period]=get_data(index_period);
   token[key_tags]=String(index_tags);
   token[key_cuez]=(Signed8)index_cue_z;
   token[key_cuex]=(Signed8)index_cue_x;
   token[key_cuec]=(Signed8)index_cue_c;
   token[key_cuev]=(Signed8)index_cue_v;
   token[key_cue]=(Signed8)index_cue;
   token[key_md5sum]=String(index_md5sum);
   token[key_time]=String(index_time);
   if (index_spectrum!=no_spectrum)
     token[key_spectrum]=index_spectrum->get_data();
   token[key_disabled_capacities]=Unsigned2(index_disabled_capacities);
   token[key_prev]=get_data(prev);
   token[key_next]=get_data(next);
   
   const int versionnr = -1;
   token[key_echo]=index_histogram.get_data(versionnr);
   token[key_rythm]=index_rythm.get_data(versionnr);
   token[key_composition]=index_composition.get_data(versionnr);
   token[key_min]=index_min.get_data(versionnr);
   token[key_max]=index_max.get_data(versionnr);
   token[key_mean]=index_mean.get_data(versionnr);
   token[key_power]=index_power.get_data(versionnr);

   // count the albums
   AlbumField **amp = albums;
   int c = 0;
   while (*amp)
     {
       c++;
       amp++;
     }
   Array<1,Data> album_data(c);
   amp = albums;
   c=0;
   while (*amp)
     {
       album_data[c++]=(*amp)->get_data(versionnr);
       amp++;
     }
   token[key_albums]=album_data;
   DataBinner::write(token,(const char*)meta_filename);
}

AlbumField::AlbumField(int r, QString n)
{
  name = n;
  nr = r;
}

int strcompare(const void *a, const void * b)
{
  return strcmp(*(const char**)a,*(const char**)b);
}

bool Index::fix_tagline()
{
  // hoe kunnen we de tagline fixed ?
  // we creeeren een array die refereert naar de gevraagde words,
  int length;
  int nextword = 0;
  char* lastword;
  char* words[100];
  char* runner;
  char* temp;
  char * new_tags;
  int i = 0;
  if (index_tags==NULL)
    return false;
  // find all tags
  temp=strdup(index_tags);
  runner=temp;
  length = strlen(runner);
  while(1)
    {
      words[nextword++]=runner;
      while(*runner!=' ' && *runner!=0) runner++;
      if (!*runner) 
	break;
      *runner=0;
      runner++;
    }
  // now sort them 
  qsort(words,nextword,sizeof(char*),strcompare);
  // remove duplicates and create result
  new_tags = bpmdj_allocate(length+1,char);
  runner = new_tags;
  lastword = "";
  for(i = 0 ; i <nextword ; i ++)
    {
      if (strcmp(words[i],lastword)!=0)
	{
	  strcpy(runner,lastword=words[i]);
	  runner+=strlen(words[i]);
	  *runner=' ';
	  runner++;
	}
    }
  if (runner>new_tags)
    *(runner-1)=0;
  else
    *(runner)=0;
  if (strcmp(index_tags,new_tags)==0)
    {
      bpmdj_deallocate(new_tags);
      bpmdj_deallocate(temp);
      return false;
    }
  else
    {
      // printf("Index: tags '%s' to '%s'\n",index_tags,new_tags);
      bpmdj_deallocate(temp);
      index_tags=new_tags;
      return true;
    }
}

void Index::add_history(HistoryField **& history, HistoryField * field)
{
  // we assume that the historyfield itself is not yet available !
  int c=0;
  HistoryField* *tmp = history;
  while(*tmp) {c++; tmp++;};
  c++;
  history=bpmdj_reallocate(history,c+1,HistoryField*);
  history[c-1]=field;
  history[c]=NULL;
}

void Index::add_album(AlbumField * a)
{
  int c=1;
  AlbumField* *tmp = albums;
  while(*tmp) {c++; tmp++;};
  albums=bpmdj_reallocate(albums,c+1,AlbumField*);
  albums[c-1]=a;
  albums[c]=NULL;
}

AlbumField ** Index::copy_albums()
{
  AlbumField ** tmp = albums;
  int c=1;
  while ( *tmp ) 
    { 
      c++; 
      tmp++; 
    } ;
  AlbumField ** result = bpmdj_allocate(c,AlbumField*);
  result[ --c ] = NULL ;
  while( --c >= 0 ) 
    result[c] = new AlbumField(albums[c]->nr, albums[c]->name);
  return result;
}

AlbumField * Index::find_album(QString n)
{
  AlbumField* *tmp = albums;
  while(*tmp && n!=(*tmp)->name) 
    tmp++;
  return *tmp;
}

void Index::delete_album(QString n)
{
  AlbumField* *tmp = albums;
  while(*tmp && n!=(*tmp)->name)
    tmp++;
  while(tmp[0])
    {
      tmp[0]=tmp[1];
      tmp++;
    }
}

void Index::add_prev_history(HistoryField * h)
{
  add_history(prev,h);
}

void Index::add_next_history(HistoryField * h)
{
  add_history(next,h);
}

bool Index::fix_tempo_fields()
{
  if (index_period.valid())
    {
      tempo_type T=get_tempo();
      if (!T.in_range())
	index_period = period_type();
    }
  else if (!(index_period.none()))
    {
      index_period = period_type();
      return true;
    }
  return false;
}

void Index::read_idx(QString indexn)
{
  // open file
  meta_filename = indexn;
  // clear all fields
  init();
  // we assume we have a text file format
  Token token = (Token)DataBinner::read_file((const char*)meta_filename);

  // we retrieve all fields that we are interested in
  String meta_version = (String)token[key_bpmdj_version];
  index_period = period_type((Signed4)token[key_period]);
  index_file   = (String)token[key_file];
  index_time   = (String)token[key_time];
  index_cue    = (Signed8)token[key_cue];
  index_cue_z  = (Signed8)token[key_cuez];
  index_cue_x  = (Signed8)token[key_cuex];
  index_cue_c  = (Signed8)token[key_cuec];
  index_cue_v  = (Signed8)token[key_cuev];
  index_md5sum = (String)token[key_md5sum];
  index_min.set_data(token[key_min]);
  index_max.set_data(token[key_max]);
  index_mean.set_data(token[key_mean]);
  index_power.set_data(token[key_power]);
  Data d = token[key_spectrum];
  if (!d.isNull())
    index_spectrum=new spectrum_type(d);
  index_histogram.set_data(token[key_echo]);
  index_rythm.set_data(token[key_rythm]);
  index_composition.set_data(token[key_composition]);
  index_disabled_capacities=(Unsigned2)token[key_disabled_capacities];
  // the previous and next fields are somewhat more interesting...
  Array<1,Data>::values iterator;
  Array<1,Data> prevfields = (Array<1,Data>)token[key_prev];
  for(iterator.reset(prevfields) ; iterator.more() ; ++iterator)
    add_prev_history(new HistoryField(*iterator));
  Array<1,Data> nextfields = (Array<1,Data>)token[key_next];
  for(iterator.reset(nextfields) ; iterator.more() ; ++iterator)
    add_next_history(new HistoryField(*iterator));
  // albums
  Array<1,Data> albums = (Array<1,Data>)token[key_albums];
  iterator.reset(albums);
  while(iterator.more())
    {
      add_album(new AlbumField(*iterator));
      ++iterator;
    }
  title=(String)token[key_title];
  author=(String)token[key_author];
  remix=(String)token[key_remix];
  version=(String)token[key_version];
  index_tags=(String)token[key_tags];

  if (fix_tempo_fields())
    meta_changed=true;
  if (fix_tagline())
    meta_changed=true;
}

// constructors and destructors
Index::Index()
{
  init();
}

Index::Index(QString from)
{
  read_idx(from);
}

Index::~Index()
{
  free();
}

// accessors
QString Index::readable_description()
{
  QString r,a;
  if (!remix.isEmpty())
    r=QString("{")+remix+"}";
  if (!author.isEmpty())
    a=QString("[")+author+"]";
  return title+r+a+version;
}

QString Index::get_display_title()
{
  if (title.isEmpty())
    return meta_filename;
  else if (!remix.isEmpty())
    return title+"{"+remix+"}";
  return title;
}

void Index::set_period(period_type t, bool update_on_disk)
{
  // if the period is different from the old period then we remove the rythm information
  // and compostion information
  if (t.period!=index_period.period)
    {
      index_rythm.clear();
      index_composition.clear();
    }
  index_period = t;
  meta_changed = 1;
  if (update_on_disk)
    write_idx();
}; 

// dialog boxes to update the state of an index file
#define field2this(namea,nameb) if (info.namea##Edit->text()!=nameb) \
  { nameb = info.namea##Edit->text(); \
    meta_changed = true; }
void Index::executeInfoDialog()
{
  SongInformation info(NULL,NULL,TRUE);
  info.idxLabel->setText(meta_filename);
  info.md5Label->setText(index_md5sum);
  info.titleEdit->setText(title);
  info.authorEdit->setText(author);
  info.versionEdit->setText(version);
  info.remixEdit->setText(remix);
  info.tagEdit->setText(index_tags);
  capacity_type old_disabled_capacity = index_disabled_capacities;
  init_capacity_widget(info.capacity,old_disabled_capacity);
  if (info.exec()==QDialog::Accepted)
    {
      field2this(title, title);
      field2this(author, author);
      field2this(remix, remix);
      field2this(version, version);
      field2this(tag, index_tags);
      index_disabled_capacities = get_capacity(info.capacity);
      meta_changed |= old_disabled_capacity!=index_disabled_capacities;
    }
}

HistoryField *Index::add_prev_song(QString mp3)
{
  HistoryField * f = find_field(prev,mp3);
  f->count++;
  meta_changed=true;
  return f;
}

HistoryField *Index::add_next_song(QString mp3)
{
  HistoryField * f = find_field(next,mp3);
  f->count++;
  meta_changed=true;
  return f;
}

HistoryField* Index::find_field(HistoryField **&ar, QString mp3)
{
  HistoryField ** cur = ar;
  while(*cur)
    {
      if ((*cur)->file==mp3)
	return *cur;
      cur++;
    }
  HistoryField *g = new HistoryField(mp3);
  add_history(ar,g);
  return g;
}

void Index::set_time(QString str)
{
  if (meta_changed || index_time!=str)
    {
      index_time = str;
      meta_changed = 1;
    }
};

spectrum_type *Index::get_spectrum()
{
  if (!index_spectrum) return no_spectrum;
  return new spectrum_type(index_spectrum);
}

bool Index::fully_defined_energy()
{
  return index_min.fully_defined()
    && index_max.fully_defined()
    && index_mean.fully_defined()
    && index_power.fully_defined();
}

void Index::clear_energy()
{
  index_min=sample4_type();
  index_max=sample4_type();
  index_mean=sample4_type();
  index_power=power_type();
}

int Index::get_time_in_seconds()
{
  const char * T = get_time();
  if (!T) return -1;
  int minutes = atoi(T);
  while(*T && isdigit(*T)) T++;
  if (*T!=':') return -1;
  int seconds = atoi(T);
  return minutes*60+seconds;
}

int Index::get_playcount()
{
  int total = 0;
  if (prev)
    {
      int i = 0;
      while(prev[i])
	total+=prev[i++]->count;
    }
  if(next)
    {
      int i = 0;
      while(next[i])
	total+=next[i++]->count;
    }
  return total;
}

/**
 * This function tries to find title, author and remix in the
 * filename and set the specified fields in the index.
 *
 * This function probably still leaks memory but it is only
 * used when a new index is created. So at the moment I don't
 * care that much. 
 */
bool Index::set_title_author_remix(QString meta_filename)
{
  char * original = strdup(meta_filename);
  if(!original) return false;
  int l = strlen(original);
  // if the filename does not end on .idx then we skip
  if (l<4) return false;
  if (l>4)
    if (strcmp(original+l-4,".idx")!=0)
      return false;
  // ditch the last three characters (.idx)
  original[l-4]=0;
  char * fulltitle = basename(original);
  // obtain title and author
  char * tmp = fulltitle;
  int busy=0;
  
  char * _title = fulltitle;
  char * _remix = NULL;
  char * _author = NULL;
  char * _version = NULL;
  title = fulltitle;
  // busy = 0, begonnen aan title
  // busy = 1, begonnen aan remix
  // busy = 2, geeindigt met remix
  // busy = 3, begonnen met author
  // busy = 4, geeindigt met author, begonnen met version
  // busy >= 6, geeindigt met crash
  while(*tmp)
    {
      if (*tmp == '{')
	{
	  if (busy!=0) 
	    { 
	      busy=6; 
	      break; 
	    }
	  _remix=tmp+1;
	  *tmp=0;
	  busy=1;
	}
      if (*tmp == '}')
	{
	  if (busy!=1) 
	    { 
	      busy=7; 
	      break;
	    }
	  *tmp=0;
	  busy=2;
	  tmp++;
	  if (*tmp!='[') 
	    { 
	      busy=8; 
	      break; 
	    }
	}
      if (*tmp == '[')
	{
	  if (busy!=2 && busy!=0) 
	    {
	      busy=9; 
	      break; 
	    }
	  _author = tmp+1;
	  *tmp=0;
	  busy=3;
	}
      if (*tmp == ']')
	{
	  if (busy!=3) 
	    { 
	      busy=10; 
	      break; 
	    }
	  *tmp=0;
	  busy=4;
	  _version = tmp+1;
	}
      tmp++;
    }
  if (busy==4)
    {
      // printf("OldIndex: fixed %s { %s } [ %s ] %s\n",title,remix, author, version);
      if (_title) 
	title = _title;
      if (_version)
	version = _version;
      if (_author)
	author = _author;
      if (_remix)
	remix = _remix;
    }
  bpmdj_deallocate(original);
  return busy==4;
}

char* findUniqueName(const char * directory, const char* filename)
{
  char indexname[500];
  char *temp;
  char halfindexname[500];
  temp=strdup(basename(strdup(filename)));
  temp[strlen(temp)-4]=0;
  sprintf(halfindexname,"%s.idx",temp);
  sprintf(indexname,"%s%s.idx",(const char*)directory,temp);
  int nr=2;
  while(exists(indexname))
    {
      sprintf(halfindexname,"%s%d.idx",temp,nr);
      sprintf(indexname,"%s%s%d.idx",(const char*)directory,temp,nr++);
    }
  return strdup(indexname);
}

Index* createNewIndexFor(QString filename, QString directory)
{
  // find a unique index filename
  char * indexname = findUniqueName(directory,filename);
  printf("Creating index file %s\n",indexname);
  // create an index and set the file in which it is stored
  Index *index = new Index();
  index->set_storedin(indexname);
  // set the songname 
  index->set_filename(filename);
  index->set_tags("New");
  index->set_title_author_remix(indexname);
  // we set the period to unknown, which will also 
  // immediatelly write the index to disk
  index->set_period(-1);
  index->set_changed();
  index->write_idx();
  bpmdj_deallocate(indexname);
  return index;
}
