
/*
  Meanwhile - Unofficial Lotus Sametime Community Client Library
  Copyright (C) 2004  Christopher (siege) O'Brien
  
  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Library General Public
  License as published by the Free Software Foundation; either
  version 2 of the License, or (at your option) any later version.
  
  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Library General Public License for more details.
  
  You should have received a copy of the GNU Library General Public
  License along with this library; if not, write to the Free
  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#ifndef _MW_UTIL_H
#define _MW_UTIL_H


#include <glib.h>
#include <glib/ghash.h>
#include <glib/glist.h>


#define map_guint_new() \
  g_hash_table_new(g_direct_hash, g_direct_equal)


#define map_guint_new_full(valfree) \
  g_hash_table_new_full(g_direct_hash, g_direct_equal, NULL, (valfree))


#define map_guint_insert(ht, key, val) \
  g_hash_table_insert((ht), GUINT_TO_POINTER((guint)(key)), (val))


#define map_guint_replace(ht, key, val) \
  g_hash_table_replace((ht), GUINT_TO_POINTER((guint)(key)), (val))


#define map_guint_lookup(ht, key) \
  g_hash_table_lookup((ht), GUINT_TO_POINTER((guint)(key)))


#define map_guint_remove(ht, key) \
  g_hash_table_remove((ht), GUINT_TO_POINTER((guint)(key)))


#define map_guint_steal(ht, key) \
  g_hash_table_steal((ht), GUINT_TO_POINTER((guint)(key)))


GList *map_collect_keys(GHashTable *ht);


GList *map_collect_values(GHashTable *ht);


struct mw_datum {
  gpointer data;
  GDestroyNotify clear;
};


struct mw_datum *mw_datum_new(gpointer data, GDestroyNotify clear);


void mw_datum_set(struct mw_datum *d, gpointer data, GDestroyNotify clear);


gpointer mw_datum_get(struct mw_datum *d);


void mw_datum_clear(struct mw_datum *d);


void mw_datum_free(struct mw_datum *d);


#endif
