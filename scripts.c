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

#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <libgen.h>
#include <unistd.h>
#include "cbpm-index.h"
#include "scripts.h"
#include "stdarg.h"

char *getRawFilename(const char * n)
{
  char d[2048];
  sprintf(d,"%s.raw",basename((char*)n));
  return (char*)strdup(d);
}

FILE * openRawFile()
{
  FILE * raw;
  char * name = getRawFilename(index_file);
  assert(name);
  raw = fopen(name,"rb");
  if (!raw)
    {
      printf("Error: Unable to open %s\n",name);
      exit(3);
    }
  free(name);
  return raw;
}

FILE * openRawFileForWriting()
{
  FILE * raw;
  char * name = getRawFilename(index_file);
  assert(name);
  raw = fopen(name,"r+b");
  if (!raw)
    {
      printf("Error: Unable to open %s\n",name);
      exit(3);
    }
  free(name);
  return raw;
}

void removeRaw()
{
  char * name = getRawFilename(index_file);
  remove(name);
  free(name);
}

void removeAllRaw()
{
  execute(RM"*"RAW_EXT);
}

void dumpAudio(const char* fname, unsigned4 *buffer, long length)
{
  FILE * tmp = fopen(fname,"wb");
  writesamples(buffer,length,tmp);
  fclose(tmp);
}


void spawn(const char* script)
{
  if (!fork())
    {
      execute(script);
      exit(100);
    }
}

int execute(const char* script)
{
  if (system(script)<=256) return 1;
  printf("Error: couldn't execute %s\n",script);
  return 0;
}

int vexecute(const char* script, ...)
{
  char toexecute[1024];
  va_list ap;
  va_start(ap,script);
  vsprintf(toexecute,script,ap);
  va_end(ap);
  return execute(toexecute);
}

FILE* openScriptFile(const char* name)
{
  FILE * script = fopen(name,"wb");
  assert(script);
  fprintf(script,SHELL_HEADER);
  return script;
}

char * getMd5(const char* argument)
{ 
  FILE * kloink;
  if (!vexecute("md5sum \"%s\" | awk '{printf $1}' >sum.tmp\n",argument))
    exit(101);
  kloink=fopen("sum.tmp","r");
  char s[40];
  int i=0;
  while(i<32)
    {
      int c = getc(kloink);
      s[i]=c;
      i++;
    }
  s[32]=0;
  fclose(kloink);
  return strdup(s);
}
