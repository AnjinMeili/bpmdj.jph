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
#ifndef __loaded__accessors_h__
#define __loaded__accessors_h__
using namespace std;
#define accessors(type,var,init) \
  private: type _##var; \
  public: inline type get_##var() const { return _##var; }; \
  inline void set_##var(type a) { _##var = a; }; \
  inline void init_##var() { _##var = init; };

#define container_accessors(type,var,accesstype,access) \
  private: type _##var; \
  public: inline type get_##var() const { return _##var; }; \
  inline void set_##var(type a) { _##var = a; }; \
  inline void init_##var() { _##var = init; };

#define singleton_accessors(type,var)  \
  private: static type _##var; \
  public: static inline type get_##var() { return _##var; }; \
  void static inline set_##var(type a) { _##var = a; };

#define init_singleton_var(klasse, var, type, init) type klasse::_##var = init
#endif // __loaded__accessors_h__
