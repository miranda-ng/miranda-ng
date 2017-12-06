
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

#include "mw_util.h"


static void collect_keys(gpointer key, gpointer val, gpointer data) {
  GList **list = data;
  *list = g_list_append(*list, key);
}


GList *map_collect_keys(GHashTable *ht) {
  GList *ret = NULL;
  g_hash_table_foreach(ht, collect_keys, &ret);
  return ret;
}


static void collect_values(gpointer key, gpointer val, gpointer data) {
  GList **list = data;
  *list = g_list_append(*list, val);
}


GList *map_collect_values(GHashTable *ht) {
  GList *ret = NULL;
  g_hash_table_foreach(ht, collect_values, &ret);
  return ret;
}


struct mw_datum *mw_datum_new(gpointer data, GDestroyNotify clear) {
  struct mw_datum *d = g_new(struct mw_datum, 1);
  mw_datum_set(d, data, clear);
  return d;
}


void mw_datum_set(struct mw_datum *d, gpointer data, GDestroyNotify clear) {
  d->data = data;
  d->clear = clear;
}


gpointer mw_datum_get(struct mw_datum *d) {
  return d->data;
}


void mw_datum_clear(struct mw_datum *d) {
  if(d->clear) {
    d->clear(d->data);
    d->clear = NULL;
  }
  d->data = NULL;
}


void mw_datum_free(struct mw_datum *d) {
  mw_datum_clear(d);
  g_free(d);
}
