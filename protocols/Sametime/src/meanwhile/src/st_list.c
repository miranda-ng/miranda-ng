
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

#include <stdio.h>
#include <string.h>
#include <glib/gstring.h>

#include "mw_debug.h"
#include "mw_util.h"
#include "mw_st_list.h"


struct mwSametimeList {
  guint ver_major;
  guint ver_minor;
  guint ver_micro;

  GList *groups;
};


struct mwSametimeGroup {
  struct mwSametimeList *list;

  enum mwSametimeGroupType type;
  char *name;
  char *alias;
  gboolean open;

  GList *users;
};


struct mwSametimeUser {
  struct mwSametimeGroup *group;

  enum mwSametimeUserType type;
  struct mwIdBlock id;
  char *name;
  char *alias;
};


static void user_free(struct mwSametimeUser *u) {
  struct mwSametimeGroup *g;

  g = u->group;
  g->users = g_list_remove(g->users, u);

  mwIdBlock_clear(&u->id);
  g_free(u->name);
  g_free(u->alias);
  g_free(u);
}


static void group_free(struct mwSametimeGroup *g) {
  struct mwSametimeList *l;

  l = g->list;
  l->groups = g_list_remove(l->groups, g);

  while(g->users)
    mwSametimeUser_free(g->users->data);

  g_free(g->name);
  g_free(g->alias);
  g_free(g);
}


static void list_free(struct mwSametimeList *l) {
  while(l->groups)
    mwSametimeGroup_free(l->groups->data);

  g_free(l);
}


struct mwSametimeList *
mwSametimeList_new() {

  struct mwSametimeList *stl;

  stl = g_new0(struct mwSametimeList, 1);
  stl->ver_major = ST_LIST_MAJOR;
  stl->ver_minor = ST_LIST_MINOR;
  stl->ver_micro = ST_LIST_MICRO;

  return stl;
}


void mwSametimeList_setMajor(struct mwSametimeList *l, guint v) {
  g_return_if_fail(l != NULL);
  l->ver_major = v;
}


guint mwSametimeList_getMajor(struct mwSametimeList *l) {
  g_return_val_if_fail(l != NULL, 0);
  return l->ver_major;
}


void mwSametimeList_setMinor(struct mwSametimeList *l, guint v) {
  g_return_if_fail(l != NULL);
  l->ver_minor = v;
}


guint mwSametimeList_getMinor(struct mwSametimeList *l) {
  g_return_val_if_fail(l != NULL, 0);
  return l->ver_minor;
}


void mwSametimeList_setMicro(struct mwSametimeList *l, guint v) {
  g_return_if_fail(l != NULL);
  l->ver_micro = v;
}


guint mwSametimeList_getMicro(struct mwSametimeList *l) {
  g_return_val_if_fail(l != NULL, 0);
  return l->ver_micro;
}


GList *mwSametimeList_getGroups(struct mwSametimeList *l) {
  g_return_val_if_fail(l != NULL, NULL);
  return g_list_copy(l->groups);
}


struct mwSametimeGroup *
mwSametimeList_findGroup(struct mwSametimeList *l,
			 const char *name) {
  GList *s;

  g_return_val_if_fail(l != NULL, NULL);
  g_return_val_if_fail(name != NULL, NULL);
  g_return_val_if_fail(*name != '\0', NULL);

  for(s = l->groups; s; s = s->next) {
    struct mwSametimeGroup *g = s->data;
    if(! strcmp(g->name, name)) return g;
  }

  return NULL;
}


void mwSametimeList_free(struct mwSametimeList *l) {
  g_return_if_fail(l != NULL);
  list_free(l);
}


struct mwSametimeGroup *
mwSametimeGroup_new(struct mwSametimeList *list,
		    enum mwSametimeGroupType type,
		    const char *name) {
  
  struct mwSametimeGroup *stg;

  g_return_val_if_fail(list != NULL, NULL);
  g_return_val_if_fail(name != NULL, NULL);
  g_return_val_if_fail(*name != '\0', NULL);

  stg = g_new0(struct mwSametimeGroup, 1);
  stg->list = list;
  stg->type = type;
  stg->name = g_strdup(name);

  list->groups = g_list_append(list->groups, stg);

  return stg;
}


enum mwSametimeGroupType mwSametimeGroup_getType(struct mwSametimeGroup *g) {
  g_return_val_if_fail(g != NULL, mwSametimeGroup_UNKNOWN);
  return g->type;
}


const char *mwSametimeGroup_getName(struct mwSametimeGroup *g) {
  g_return_val_if_fail(g != NULL, NULL);
  return g->name;
}


void mwSametimeGroup_setAlias(struct mwSametimeGroup *g,
			      const char *alias) {
  g_return_if_fail(g != NULL);

  g_free(g->alias);
  g->alias = g_strdup(alias);
}


const char *mwSametimeGroup_getAlias(struct mwSametimeGroup *g) {
  g_return_val_if_fail(g != NULL, NULL);
  return g->alias;
}


void mwSametimeGroup_setOpen(struct mwSametimeGroup *g, gboolean open) {
  g_return_if_fail(g != NULL);
  g->open = open;
}


gboolean mwSametimeGroup_isOpen(struct mwSametimeGroup *g) {
  g_return_val_if_fail(g != NULL, FALSE);
  return g->open;
}


struct mwSametimeList *mwSametimeGroup_getList(struct mwSametimeGroup *g) {
  g_return_val_if_fail(g != NULL, NULL);
  return g->list;
}


GList *mwSametimeGroup_getUsers(struct mwSametimeGroup *g) {
  g_return_val_if_fail(g != NULL, NULL);
  return g_list_copy(g->users);
}


struct mwSametimeUser *
mwSametimeGroup_findUser(struct mwSametimeGroup *g,
			 struct mwIdBlock *user) {
  GList *s;

  g_return_val_if_fail(g != NULL, NULL);
  g_return_val_if_fail(user != NULL, NULL);

  for(s = g->users; s; s = s->next) {
    struct mwSametimeUser *u = s->data;
    if(mwIdBlock_equal(user, &u->id)) return u;
  }

  return NULL;
}


void mwSametimeGroup_free(struct mwSametimeGroup *g) {
  g_return_if_fail(g != NULL);
  g_return_if_fail(g->list != NULL);
  group_free(g);
}


struct mwSametimeUser *
mwSametimeUser_new(struct mwSametimeGroup *group,
		   enum mwSametimeUserType type,
		   struct mwIdBlock *id) {

  struct mwSametimeUser *stu;

  g_return_val_if_fail(group != NULL, NULL);
  g_return_val_if_fail(id != NULL, NULL);
  
  stu = g_new0(struct mwSametimeUser, 1);
  stu->group = group;
  stu->type = type;
  mwIdBlock_clone(&stu->id, id);

  group->users = g_list_append(group->users, stu);
  
  return stu;
}


struct mwSametimeGroup *mwSametimeUser_getGroup(struct mwSametimeUser *u) {
  g_return_val_if_fail(u != NULL, NULL);
  return u->group;
}


enum mwSametimeUserType mwSametimeUser_getType(struct mwSametimeUser *u) {
  g_return_val_if_fail(u != NULL, mwSametimeUser_UNKNOWN);
  return u->type;
}


const char *mwSametimeUser_getUser(struct mwSametimeUser *u) {
  g_return_val_if_fail(u != NULL, NULL);
  return u->id.user;
}


const char *mwSametimeUser_getCommunity(struct mwSametimeUser *u) {
  g_return_val_if_fail(u != NULL, NULL);
  return u->id.community;
}


void mwSametimeUser_setShortName(struct mwSametimeUser *u, const char *name) {
  g_return_if_fail(u != NULL);
  g_free(u->name);
  u->name = g_strdup(name);
}


const char *mwSametimeUser_getShortName(struct mwSametimeUser *u) {
  g_return_val_if_fail(u != NULL, NULL);
  return u->name;
}


void mwSametimeUser_setAlias(struct mwSametimeUser *u, const char *alias) {
  g_return_if_fail(u != NULL);
  g_free(u->alias);
  u->alias = g_strdup(alias);
}


const char *mwSametimeUser_getAlias(struct mwSametimeUser *u) {
  g_return_val_if_fail(u != NULL, NULL);
  return u->alias;
}


void mwSametimeUser_free(struct mwSametimeUser *u) {
  g_return_if_fail(u != NULL);
  g_return_if_fail(u->group != NULL);
  user_free(u);
}


static void str_replace(char *str, char from, char to) {
  if(! str) return;
  for(; *str; str++) if(*str == from) *str = to;
}


static char user_type_to_char(enum mwSametimeUserType type) {
  switch(type) {
  case mwSametimeUser_NORMAL:    return '1';
  case mwSametimeUser_EXTERNAL:  return '2';
  case mwSametimeUser_UNKNOWN:
  default:                       return '9';
  }
}


static enum mwSametimeUserType user_char_to_type(char type) {
  switch(type) {
  case '1':  return mwSametimeUser_NORMAL;
  case '2':  return mwSametimeUser_EXTERNAL;
  default:   return mwSametimeUser_UNKNOWN;
  }
}


static void user_put(GString *str, struct mwSametimeUser *u) {
  char *id, *name, *alias;
  char type;
  
  id = g_strdup(u->id.user);
  name = g_strdup(u->name);
  alias = g_strdup(u->alias);
  type = user_type_to_char(u->type);

  if(id) str_replace(id, ' ', ';');
  if(name) str_replace(name, ' ', ';');
  if(alias) str_replace(alias, ' ', ';');

  if(!name && alias) {
    name = alias;
    alias = NULL;
  }

  g_string_append_printf(str, "U %s%c:: %s,%s\r\n",
			 id, type, (name? name: ""), (alias? alias: ""));

  g_free(id);
  g_free(name);
  g_free(alias);  
}


static char group_type_to_char(enum mwSametimeGroupType type) {
  switch(type) {
  case mwSametimeGroup_NORMAL:   return '2';
  case mwSametimeGroup_DYNAMIC:  return '3';
  case mwSametimeGroup_UNKNOWN:
  default:                       return '9';
  }
}


static enum mwSametimeGroupType group_char_to_type(char type) {
  switch(type) {
  case '2':  return mwSametimeGroup_NORMAL;
  case '3':  return mwSametimeGroup_DYNAMIC;
  default:   return mwSametimeGroup_UNKNOWN;
  }
}


static void group_put(GString *str, struct mwSametimeGroup *g) {
  char *name, *alias;
  char type;
  GList *gl;

  name = g_strdup(g->name);
  alias = g_strdup((g->alias)? g->alias: name);
  type = group_type_to_char(g->type);

  str_replace(name, ' ', ';');
  str_replace(alias, ' ', ';');

  g_string_append_printf(str, "G %s%c %s %c\r\n",
			 name, type, alias, (g->open? 'O':'C'));

  for(gl = g->users; gl; gl = gl->next) {
    user_put(str, gl->data);
  }

  g_free(name);
  g_free(alias);
}


/** composes a GString with the written contents of a sametime list */
static GString *list_store(struct mwSametimeList *l) {
  GString *str;
  GList *gl;

  g_return_val_if_fail(l != NULL, NULL);

  str = g_string_new(NULL);
  g_string_append_printf(str, "Version=%u.%u.%u\r\n",
			 l->ver_major, l->ver_minor, l->ver_micro);

  for(gl = l->groups; gl; gl = gl->next) {
    group_put(str, gl->data);
  }

  return str;
}


char *mwSametimeList_store(struct mwSametimeList *l) {
  GString *str;
  char *s;

  g_return_val_if_fail(l != NULL, NULL);

  str = list_store(l);
  s = str->str;
  g_string_free(str, FALSE);
  return s;
}


void mwSametimeList_put(struct mwPutBuffer *b, struct mwSametimeList *l) {
  GString *str;
  guint16 len;

  g_return_if_fail(l != NULL);
  g_return_if_fail(b != NULL);

  str = list_store(l);
  len = (guint16) str->len;
  guint16_put(b, len);
  mwPutBuffer_write(b, str->str, len);

  g_string_free(str, TRUE);
}


static void get_version(const char *line, struct mwSametimeList *l) {
  guint major = 0, minor = 0, micro = 0;
  int ret;

  ret = sscanf(line, "Version=%u.%u.%u\n", &major, &minor, &micro);
  if(ret != 3) {
    g_warning("strange sametime list version line:\n%s", line);
  }

  l->ver_major = major;
  l->ver_minor = minor;
  l->ver_micro = micro;
}


static struct mwSametimeGroup *get_group(const char *line,
					 struct mwSametimeList *l) {
  struct mwSametimeGroup *group;
  char *name, *alias;
  char type = '2', open = 'O';
  int ret;

  ret = strlen(line);
  name = g_malloc0(ret);
  alias = g_malloc0(ret);

  ret = sscanf(line, "G %s %s %c\n",
	       name, alias, &open);

  if(ret < 3) {
    g_warning("strange sametime list group line:\n%s", line);
  }
  
  str_replace(name, ';', ' ');
  str_replace(alias, ';', ' ');

  if(name && *name) {
    int l = strlen(name)-1;
    type = name[l];
    name[l] = '\0';
  }

  group = g_new0(struct mwSametimeGroup, 1);
  group->list = l;
  group->name = name;
  group->type = group_char_to_type(type);
  group->alias = alias;
  group->open = (open == 'O');

  l->groups = g_list_append(l->groups, group);

  return group;
}


static void get_user(const char *line, struct mwSametimeGroup *g) {
  struct mwSametimeUser *user;
  struct mwIdBlock idb = { 0, 0 };
  char *name, *alias = NULL;
  char type = '1';
  int ret;
  
  ret = strlen(line);
  idb.user = g_malloc0(ret);
  name = g_malloc0(ret);

  ret = sscanf(line, "U %s %s",
	       idb.user, name);

  if(ret < 2) {
    g_warning("strange sametime list user line:\n%s", line);
  }

  str_replace(idb.user, ';', ' ');
  str_replace(name, ';', ' ');

  if(idb.user && *idb.user) {
    char *tmp = strstr(idb.user, "::");
    if(tmp--) {
      type = *(tmp);
      *tmp = '\0';
    }
  }

  if(name && *name) {
    char *tmp = strrchr(name, ',');
    if(tmp) {
      *tmp++ = '\0';
      if(*tmp) alias = tmp;
    }
  }

  user = g_new0(struct mwSametimeUser, 1);
  user->group = g;
  user->id.user = idb.user;
  user->type = user_char_to_type(type);
  user->name = name;
  user->alias = g_strdup(alias);
  
  g->users = g_list_append(g->users, user);
}


/** returns a line from str, and advances str */
static char *fetch_line(char **str) {
  char *start = *str;
  char *end;

  /* move to first non-whitespace character */
  while(*start && g_ascii_isspace(*start)) start++;
  if(! *start) return NULL;

  for(end = start + 1; *end; end++) {
    if(*end == '\n' || *end == '\r') {
      *(end++) = '\0';
      break;
    }
  }

  *str = end;
  return start;
}


static void list_get(const char *lines, struct mwSametimeList *l) {
  char *s = (char *) lines;
  char *line;

  struct mwSametimeGroup *g = NULL;
  
  while( (line = fetch_line(&s)) ) {
    switch(*line) {
    case 'V':
      get_version(line, l);
      break;

    case 'G':
      g = get_group(line, l);
      break;

    case 'U':
      get_user(line, g);
      break;

    default:
      g_warning("unknown sametime list data line:\n%s", line);
    }
  }  
}


struct mwSametimeList *mwSametimeList_load(const char *data) {
  struct mwSametimeList *l;

  g_return_val_if_fail(data != NULL, NULL);
  
  l = mwSametimeList_new();
  list_get(data, l);

  return l;
}


void mwSametimeList_get(struct mwGetBuffer *b, struct mwSametimeList *l) {
  char *str = NULL;

  g_return_if_fail(l != NULL);
  g_return_if_fail(b != NULL);

  mwString_get(b, &str);
  list_get(str, l);
  g_free(str);
}

