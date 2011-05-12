/****
 BpmDj v3.6: Free Dj Tools
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
#ifndef __loaded__embedded_files_h__
#define __loaded__embedded_files_h__
using namespace std;
#line 1 "embedded-files.h++"
/**
 * the pointers refers to files that are embedded as object files
 * before they are usefull init_embedded_files should be called
 */
void init_embedded_files();

#define EMBEDDED_FILES \
FILE(logo_png)  \
FILE(bpmdj_ogg) \
FILE(bpmdj_mp3)

#define FILE(name) \
  extern         int name## _size; \
  extern const char* name;
EMBEDDED_FILES
#undef FILE
#endif // __loaded__embedded_files_h__
