/*
    This file is part of tgl-libary/generate

    Tgl-library/generate is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 2 of the License, or
    (at your option) any later version.

    Tgl-library/generate is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this tgl-library/generate.  If not, see <http://www.gnu.org/licenses/>.

    Copyright Vitaly Valtman 2014-2015

    It is derivative work of VK/KittenPHP-DB-Engine (https://github.com/vk-com/kphp-kdb/)
    Copyright 2012-2013 Vkontakte Ltd
              2012-2013 Vitaliy Valtman
*/

#define _GNU_SOURCE

#include "config.h"
#include <stdio.h>
#include <signal.h>
#ifdef HAVE_EXECINFO_H
#include <execinfo.h>
#endif
#if defined(WIN32) || defined(_WIN32)
#include <io.h>
#include <stdint.h>
#include <string.h>
#ifndef __MINGW32__
#include "wingetopt.h"
#endif
#else
#include <unistd.h>
#endif
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <assert.h>
#include <string.h>

#include "tl-parser/tl-tl.h"
#include "generate.h"

#include "tree.h"

int header;

#if defined(WIN32) || defined(_WIN32)
char* strndup(const char* str, size_t n) {
  size_t len = n;
  const char *end = memchr(str, '\0', n);
  if (end)
    len = end - str;

  char* dupstr = malloc(len + 1);
  if (dupstr != NULL) {
    memcpy(dupstr, str, len);
    dupstr[len] = '\0';
  }
  return dupstr;
}

#define lrand48() rand()
#define _PRINTF_INT64_ "I64"
#else
#define _PRINTF_INT64_ "ll"
#endif

#define tl_type_name_cmp(a,b) (a->name > b->name ? 1 : a->name < b->name ? -1 : 0)

DEFINE_TREE (tl_type, struct tl_type *, tl_type_name_cmp, 0)
DEFINE_TREE (tl_combinator, struct tl_combinator *, tl_type_name_cmp, 0)

struct tree_tl_type *type_tree;
struct tree_tl_combinator *function_tree;

void tl_function_insert_by_name (struct tl_combinator *c) {
  function_tree = tree_insert_tl_combinator (function_tree, c, lrand48 ());
}

struct tl_type *tl_type_get_by_name (int name) {
  static struct tl_type t;
  t.name = name;

  return tree_lookup_tl_type (type_tree, &t);
}

void tl_type_insert_by_name (struct tl_type *t) {
  type_tree = tree_insert_tl_type (type_tree, t, lrand48 ());
}

int is_empty (struct tl_type *t) {
  if (t->name == NAME_INT  || t->name == NAME_LONG || t->name == NAME_DOUBLE || t->name == NAME_STRING) { return 1; }
  if (t->constructors_num != 1) { return 0; }
  int count = 0;
  int i;
  struct tl_combinator *c = t->constructors[0];
  for (i = 0; i < c->args_num; i++) {
    if (!(c->args[i]->flags & FLAG_OPT_VAR)) { count ++; }
  }
  return count == 1;
}


static char buf[1 << 20];
int buf_size;
int *buf_ptr = (int *)buf;
int *buf_end;
#ifndef DISABLE_EXTF
int skip_only = 0;
#else
int skip_only = 1;
#endif

int verbosity;

int get_int (void) {
  assert (buf_ptr < buf_end);
  return *(buf_ptr ++);
}

long long get_long (void) {
  assert (buf_ptr + 1 < buf_end);
  long long r = *(long long *)buf_ptr;
  buf_ptr += 2;
  return r;
}

static void *malloc0 (int size) {
  void *r = malloc (size);
  assert (r);
  memset (r, 0, size);
  return r;
}

char *get_string (void) {
  int l = *(unsigned char *)buf_ptr;
  assert (l != 0xff);
  
  char *res;
  int tlen = 0;
  if (l == 0xfe) {
    l = ((unsigned)get_int ()) >> 8;
    res = (char *)buf_ptr;
    tlen = l;
  } else {
    res = ((char *)buf_ptr) + 1;
    tlen = 1 + l;
  }

  int len = l;
  
  tlen += ((-tlen) & 3);
  assert (!(tlen & 3));

  buf_ptr += tlen / 4;
  assert (buf_ptr <= buf_end);
  
  char *r = strndup (res, len);
  assert (r);
  return r;
}


int tn, fn, cn;
struct tl_type **tps;
struct tl_combinator **fns;

struct tl_tree *read_tree (int *var_num);
struct tl_tree *read_nat_expr (int *var_num);
struct tl_tree *read_type_expr (int *var_num);
int read_args_list (struct arg **args, int args_num, int *var_num);

#define use_var_nat_full_form(x) 0

void *int_to_var_nat_const_init (long long x) {
  if (use_var_nat_full_form (x)) {
    struct tl_tree_nat_const *T = malloc (sizeof (*T));
    assert (T);
    T->self.flags = 0;
    T->self.methods = &tl_pnat_const_full_methods;
    T->value = x;
    return T;
  } else {
    return (void *)(long)(x * 2 - 0x80000001l);
  }
}

long long var_nat_const_to_int (void *x) {
  if (((long)x) & 1) {
    return (((long)x) + 0x80000001l) / 2;
  } else {
    return ((struct tl_tree_nat_const *)x)->value;
  }
}

int tl_tree_type_type (struct tl_tree *x) {
  return NODE_TYPE_TYPE;
}

int tl_tree_type_array (struct tl_tree *x) {
  return NODE_TYPE_ARRAY;
}

int tl_tree_type_nat_const (struct tl_tree *x) {
  return NODE_TYPE_NAT_CONST;
}

int tl_tree_type_var_num (struct tl_tree *x) {
  return NODE_TYPE_VAR_NUM;
}

int tl_tree_type_var_type (struct tl_tree *x) {
  return NODE_TYPE_VAR_TYPE;
}

struct tl_tree_methods tl_var_num_methods = {
  .type = tl_tree_type_var_num
};

struct tl_tree_methods tl_var_type_methods = {
  .type = tl_tree_type_var_type
};

struct tl_tree_methods tl_type_methods = {
  .type = tl_tree_type_type
};

struct tl_tree_methods tl_nat_const_methods = {
  .type = tl_tree_type_nat_const
};

struct tl_tree_methods tl_array_methods = {
  .type = tl_tree_type_array
};

struct tl_tree_methods tl_ptype_methods = {
  .type = tl_tree_type_type
};

struct tl_tree_methods tl_parray_methods = {
  .type = tl_tree_type_array
};

struct tl_tree_methods tl_pvar_num_methods = {
  .type = tl_tree_type_var_num
};

struct tl_tree_methods tl_pvar_type_methods = {
  .type = tl_tree_type_var_type
};

struct tl_tree_methods tl_nat_const_full_methods = {
  .type = tl_tree_type_nat_const
};

struct tl_tree_methods tl_pnat_const_full_methods = {
  .type = tl_tree_type_nat_const
};

struct tl_tree *read_num_const (int *var_num) {
  return (void *)int_to_var_nat_const_init (get_int ());
}


void print_c_type_name (struct tl_tree *t, char *offset, int in) {
  int x = TL_TREE_METHODS(t)->type (t);

  if (x == NODE_TYPE_VAR_TYPE) {
    printf ("void *");
    return;
  }

  if (x == NODE_TYPE_ARRAY) {
    struct tl_tree_array *a = (void *)t;
    assert (a->args_num == 1);
    print_c_type_name (a->args[0]->type, offset, in);
    printf ("*");
    return;
  }

  struct tl_type *T = ((struct tl_tree_type *)t)->type;
  if (!strcmp (T->id, "Vector") && in) {
    printf ("struct {\n");
    printf ("%s  int *cnt;\n", offset);
    printf ("%s  ", offset);
    print_c_type_name (((struct tl_tree_type *)t)->children[0], offset, in);
    printf ("*data;\n");
    printf ("%s} *", offset);
    return;
  } 
  if (!strcmp (T->id, "Long")) {
    printf ("long long *");
    return;
  } 
  if (!strcmp (T->id, "#") || !strcmp (T->id, "Int")) {
    printf ("int *");
    return;
  } 
  if (!strcmp (T->id, "Double")) {
    printf ("double *");
    return;
  } 
  if (!strcmp (T->id, "String") || !strcmp (T->id, "Bytes")) {
    /*printf ("struct {\n");
    printf ("%s  int len;\n", offset);
    printf ("%s  char *data;\n", offset);
    printf ("%s} *", offset);
    return;*/
    
    printf ("struct tl_ds_string *");
    return;
  }
  printf ("struct tl_ds_%s *", T->print_id);
}

int gen_uni_skip (struct tl_tree *t, char *cur_name, int *vars, int first, int fun) {
  assert (t);
  int x = TL_TREE_METHODS (t)->type (t);
  int l = 0;
  int i;
  int j;
  struct tl_tree_type *t1;
  struct tl_tree_array *t2;
  int y;
  int L = strlen (cur_name);
  char *fail = fun == 1 ? "return 0;" : fun == -1 ? "return;" : "return -1;";
  switch (x) {
  case NODE_TYPE_TYPE:
    t1 = (void *)t;
    if (!first) {
      printf ("  if (ODDP(%s) || %s->type->name != 0x%08x) { %s }\n", cur_name, cur_name, t1->type->name, fail);
    } else {
      printf ("  if (ODDP(%s) || (%s->type->name != 0x%08x && %s->type->name != 0x%08x)) { %s }\n", cur_name, cur_name, t1->type->name, cur_name, ~t1->type->name, fail);
    }
    for (i = 0; i < t1->children_num; i++) {
#if defined(_MSC_VER)
#pragma warning(disable : 4996)
#endif
      sprintf (cur_name + L, "->params[%d]", i);
#if defined(_MSC_VER)
#pragma warning(default : 4996)
#endif
      gen_uni_skip (t1->children[i], cur_name, vars, 0, fun);
      cur_name[L] = 0;
    }
    return 0;
  case NODE_TYPE_NAT_CONST:
    printf ("  if (EVENP(%s) || ((long)%s) != %"_PRINTF_INT64_"d) { %s }\n", cur_name, cur_name, var_nat_const_to_int (t) * 2 + 1, fail);
    return 0;
  case NODE_TYPE_ARRAY:
    printf ("  if (ODDP(%s) || %s->type->name != TL_TYPE_ARRAY) { %s }\n", cur_name, cur_name, fail);
    t2 = (void *)t;
    
#if defined(_MSC_VER)
#pragma warning(disable : 4996)
#endif
    sprintf (cur_name + L, "->params[0]");
#if defined(_MSC_VER)
#pragma warning(default : 4996)
#endif
    y = gen_uni_skip (t2->multiplicity, cur_name, vars, 0, fun);    
    cur_name[L] = 0;

#if defined(_MSC_VER)
#pragma warning(disable : 4996)
#endif
    sprintf (cur_name + L, "->params[1]");
#if defined(_MSC_VER)
#pragma warning(default : 4996)
#endif
    y += gen_uni_skip (t2->args[0]->type, cur_name, vars, 0, fun);
    cur_name[L] = 0;
    return 0;
  case NODE_TYPE_VAR_TYPE:
    printf ("  if (ODDP(%s)) { %s }\n", cur_name, fail);
    i = ((struct tl_tree_var_type *)t)->var_num;
    if (!vars[i]) {
      printf ("  struct paramed_type *var%d = %s; assert (var%d);\n", i, cur_name, i);      
      vars[i] = 1;
    } else if (vars[i] == 1) {
      printf (" if (compare_types (var%d, %s) < 0) { %s }\n", i, cur_name, fail);
    } else {
      assert (0);
      return -1;
    }
    return l;
  case NODE_TYPE_VAR_NUM:
    printf ("  if (EVENP(%s)) { %s }\n", cur_name, fail);
    i = ((struct tl_tree_var_num *)t)->var_num;
    j = ((struct tl_tree_var_num *)t)->dif;
    if (!vars[i]) {
      printf ("  struct paramed_type *var%d = ((void *)%s) + %d; assert (var%d);\n", i, cur_name, 2 * j, i);
      vars[i] = 2;
    } else if (vars[i] == 2) {
      printf ("  if (var%d != ((void *)%s) + %d) { %s }\n", i, cur_name, 2 * j, fail);
    } else {
      assert (0);
      return -1;
    }
    return 0;
  default:
    assert (0);
    return -1;
  }
}

void print_offset (int len) {
  int i;
  for (i = 0; i < len; i++) { printf (" "); }
}

int gen_create (struct tl_tree *t, int *vars, int offset) {
  int x = TL_TREE_METHODS (t)->type (t);
  int i;
  struct tl_tree_type *t1;
  struct tl_tree_array *t2;
  switch (x) {
  case NODE_TYPE_TYPE: 
    print_offset (offset); 
    printf ("&(struct paramed_type){\n");
    print_offset (offset + 2);
    t1 = (void *)t;
    if (t1->self.flags & FLAG_BARE) {
      printf (".type = &(struct tl_type_descr) {.name = 0x%08x, .id = \"Bare_%s\", .params_num = %d, .params_types = %"_PRINTF_INT64_"d},\n", ~t1->type->name, t1->type->id, t1->type->arity, t1->type->params_types);
    } else {
      printf (".type = &(struct tl_type_descr) {.name = 0x%08x, .id = \"%s\", .params_num = %d, .params_types = %"_PRINTF_INT64_"d},\n", t1->type->name, t1->type->id, t1->type->arity, t1->type->params_types);
    }
    if (t1->children_num) {
      print_offset (offset + 2);
      printf (".params = (struct paramed_type *[]){\n");
      for (i = 0; i < t1->children_num; i++) {
        assert (gen_create (t1->children[i], vars, offset + 4) >= 0);
        printf (",\n");
      }
      print_offset (offset + 2);
      printf ("}\n");
    } else {
      print_offset (offset + 2);
      printf (".params = 0,\n");
    }
    print_offset (offset);
    printf ("}");
    return 0;
  case NODE_TYPE_NAT_CONST:
    print_offset (offset); 
    printf ("INT2PTR (%d)", (int)var_nat_const_to_int (t));
    return 0;
  case NODE_TYPE_ARRAY:
    print_offset (offset); 
    printf ("&(struct paramed_type){\n");
    print_offset (offset + 2);
    t2 = (void *)t;
    printf (".type = &(struct tl_type_descr) {.name = NAME_ARRAY, .id = \"array\", .params_num = 2, .params_types = 1},\n");
    print_offset (offset + 2);
    printf (".params = (struct paramed_type **){\n");
    gen_create (t2->multiplicity, vars, offset + 4);
    printf (",\n");
    gen_create (t2->args[0]->type, vars, offset + 4);
    printf (",\n");
    print_offset (offset + 2);
    printf ("}\n");
    print_offset (offset);
    printf ("}");
    return 0;
  case NODE_TYPE_VAR_TYPE:
    print_offset (offset);
    printf ("var%d", ((struct tl_tree_var_type *)t)->var_num);
    return 0;
  case NODE_TYPE_VAR_NUM:
    print_offset (offset);
    printf ("((void *)var%d) + %d", ((struct tl_tree_var_type *)t)->var_num, 2 * ((struct tl_tree_var_num *)t)->dif);
    return 0;
  default:
    assert (0);
    return -1;
  }
}

int gen_field_skip (struct arg *arg, int *vars, int num) {
  assert (arg);
  char *offset = "  ";
  int o = 0;
  if (arg->exist_var_num >= 0) {
    printf ("  if (PTR2INT (var%d) & (1 << %d)) {\n", arg->exist_var_num, arg->exist_var_bit);
    offset = "    ";
    o = 2;
  }
  if (arg->var_num >= 0) {
    assert (TL_TREE_METHODS (arg->type)->type (arg->type) == NODE_TYPE_TYPE);
    int t = ((struct tl_tree_type *)arg->type)->type->name;
    if (t == NAME_VAR_TYPE) {
      fprintf (stderr, "Not supported yet\n");
      assert (0);
    } else {
      assert (t == NAME_VAR_NUM);
      if (vars[arg->var_num] == 0) {
        printf ("%sif (in_remaining () < 4) { return -1;}\n", offset);
        printf ("%sstruct paramed_type *var%d = INT2PTR (fetch_int ());\n", offset, arg->var_num);
        vars[arg->var_num] = 2;
      } else if (vars[arg->var_num] == 2) {
        printf ("%sif (in_remaining () < 4) { return -1;}\n", offset);
        printf ("%sif (vars%d != INT2PTR (fetch_int ())) { return -1; }\n", offset, arg->var_num);
      } else {
        assert (0);
        return -1;
      }
    }
  } else {
    int t = TL_TREE_METHODS (arg->type)->type (arg->type);
    if (t == NODE_TYPE_TYPE || t == NODE_TYPE_VAR_TYPE) {    
      printf ("%sstruct paramed_type *field%d = \n", offset, num);
      assert (gen_create (arg->type, vars, 2 + o) >= 0);
      printf (";\n");
      int bare = arg->flags & FLAG_BARE;
      if (!bare && t == NODE_TYPE_TYPE) {
        bare = ((struct tl_tree_type *)arg->type)->self.flags & FLAG_BARE;
      }
      if (!bare) {
        printf ("%sif (skip_type_%s (field%d) < 0) { return -1;}\n", offset, t == NODE_TYPE_VAR_TYPE ? "any" : ((struct tl_tree_type *)arg->type)->type->print_id, num);
      } else {
        printf ("%sif (skip_type_bare_%s (field%d) < 0) { return -1;}\n", offset, t == NODE_TYPE_VAR_TYPE ? "any" : ((struct tl_tree_type *)arg->type)->type->print_id, num);
      }
    } else {
      assert (t == NODE_TYPE_ARRAY);
      printf ("%sint multiplicity%d = PTR2INT (\n", offset, num);
      assert (gen_create (((struct tl_tree_array *)arg->type)->multiplicity, vars, 2 + o) >= 0);
      printf ("%s);\n", offset);
      printf ("%sstruct paramed_type *field%d = \n", offset, num);
      assert (gen_create (((struct tl_tree_array *)arg->type)->args[0]->type, vars, 2 + o) >= 0);
      printf (";\n");
      printf ("%swhile (multiplicity%d -- > 0) {\n", offset, num);
      printf ("%s  if (skip_type_%s (field%d) < 0) { return -1;}\n", offset, "any", num);
      printf ("%s}\n", offset);
    }
  }
  if (arg->exist_var_num >= 0) {
    printf ("  }\n");
  }
  return 0;
}

int gen_field_fetch (struct arg *arg, int *vars, int num, int empty) {
  assert (arg);
  char *offset = "  ";
  int o = 0;
  if (arg->exist_var_num >= 0) {
    printf ("  if (PTR2INT (var%d) & (1 << %d)) {\n", arg->exist_var_num, arg->exist_var_bit);
    offset = "    ";
    o = 2;
  }
  if (!empty) {
    printf("%sif (multiline_output >= 2) { print_offset (); }\n", offset);
  }
  if (arg->id && strlen (arg->id) && !empty) {
    printf ("%sif (!disable_field_names) { eprintf (\" %s :\"); }\n", offset, arg->id);
  }
  if (arg->var_num >= 0) {
    assert (TL_TREE_METHODS (arg->type)->type (arg->type) == NODE_TYPE_TYPE);
    int t = ((struct tl_tree_type *)arg->type)->type->name;
    if (t == NAME_VAR_TYPE) {
      fprintf (stderr, "Not supported yet\n");
      assert (0);
    } else {
      assert (t == NAME_VAR_NUM);
      if (vars[arg->var_num] == 0) {
        printf ("%sif (in_remaining () < 4) { return -1;}\n", offset);
        printf ("%seprintf (\" %%d\", prefetch_int ());\n", offset);
        printf ("%sstruct paramed_type *var%d = INT2PTR (fetch_int ());\n", offset, arg->var_num);
        vars[arg->var_num] = 2;
      } else if (vars[arg->var_num] == 2) {
        printf ("%sif (in_remaining () < 4) { return -1;}\n", offset);
        printf ("%seprintf (\" %%d\", prefetch_int ());\n", offset);
        printf ("%sif (vars%d != INT2PTR (fetch_int ())) { return -1; }\n", offset, arg->var_num);
      } else {
        assert (0);
        return -1;
      }
    }
  } else {
    int t = TL_TREE_METHODS (arg->type)->type (arg->type);
    if (t == NODE_TYPE_TYPE || t == NODE_TYPE_VAR_TYPE) {    
      printf ("%sstruct paramed_type *field%d = \n", offset, num);
      assert (gen_create (arg->type, vars, 2 + o) >= 0);
      printf (";\n");
      int bare = arg->flags & FLAG_BARE;
      if (!bare && t == NODE_TYPE_TYPE) {
        bare = ((struct tl_tree_type *)arg->type)->self.flags & FLAG_BARE;
      }
      if (!bare) {
        printf ("%sif (fetch_type_%s (field%d) < 0) { return -1;}\n", offset, t == NODE_TYPE_VAR_TYPE ? "any" : ((struct tl_tree_type *)arg->type)->type->print_id, num);
      } else {
        printf ("%sif (fetch_type_bare_%s (field%d) < 0) { return -1;}\n", offset, t == NODE_TYPE_VAR_TYPE ? "any" : ((struct tl_tree_type *)arg->type)->type->print_id, num);
      }
    } else {
      assert (t == NODE_TYPE_ARRAY);
      printf ("%sint multiplicity%d = PTR2INT (\n", offset, num);
      assert (gen_create (((struct tl_tree_array *)arg->type)->multiplicity, vars, 2 + o) >= 0);
      printf ("%s);\n", offset);
      printf ("%sstruct paramed_type *field%d = \n", offset, num);
      assert (gen_create (((struct tl_tree_array *)arg->type)->args[0]->type, vars, 2 + o) >= 0);
      printf (";\n");
      printf ("%seprintf (\" [\");\n", offset);
      printf ("%sif (multiline_output >= 1) { eprintf (\"\\n\"); }\n", offset);
      printf ("%sif (multiline_output >= 1) { multiline_offset += multiline_offset_size;}\n", offset);
      printf ("%swhile (multiplicity%d -- > 0) {\n", offset, num);
      printf ("%s  if (multiline_output >= 1) { print_offset (); }\n", offset);
      printf ("%s  if (fetch_type_%s (field%d) < 0) { return -1;}\n", offset, "any", num);
      printf ("%s  if (multiline_output >= 1) { eprintf (\"\\n\"); }\n", offset);
      printf ("%s}\n", offset);
      printf ("%sif (multiline_output >= 1) { multiline_offset -= multiline_offset_size; print_offset ();}\n", offset);
      printf ("%seprintf (\" ]\");\n", offset);
    }
  }
  if (!empty) {
    printf("%sif (multiline_output >= 2) { eprintf (\"\\n\"); }\n", offset);
  }
  if (arg->exist_var_num >= 0) {
    printf ("  }\n");
  }
  return 0;
}

int gen_field_store (struct arg *arg, int *vars, int num, int from_func, int empty) {
  assert (arg);
  char *offset = "  ";
  int o = 0;
  if (arg->exist_var_num >= 0) {
    printf ("  if (PTR2INT (var%d) & (1 << %d)) {\n", arg->exist_var_num, arg->exist_var_bit);
    offset = "    ";
    o = 2;
  }
  char *fail = from_func ? "0" : "-1";
  char *expect = from_func ? "expect_token_ptr" : "expect_token";
  if (arg->id && strlen (arg->id) > 0 && !empty) {
    printf ("%sif (cur_token_len >= 0 && cur_token_len == %d && !cur_token_quoted && !memcmp (cur_token, \"%s\", cur_token_len)) {\n", offset, (int)(strlen (arg->id)), arg->id);
    printf ("%s  local_next_token ();\n", offset);
    printf ("%s  %s (\":\", 1);\n", offset, expect);
    printf ("%s}\n", offset);
  }
  if (arg->var_num >= 0) {
    printf ("%sif (cur_token_len < 0) { return %s; }\n", offset, fail);
    assert (TL_TREE_METHODS (arg->type)->type (arg->type) == NODE_TYPE_TYPE);
    int t = ((struct tl_tree_type *)arg->type)->type->name;
    if (t == NAME_VAR_TYPE) {
      fprintf (stderr, "Not supported yet\n");
      assert (0);
    } else {
      assert (t == NAME_VAR_NUM);
      if (vars[arg->var_num] == 0) {
        printf ("%sif (!is_int ()) { return %s;}\n", offset, fail);
        printf ("%sstruct paramed_type *var%d = INT2PTR (get_int ());\n", offset, arg->var_num);
        printf ("%sout_int (get_int ());\n", offset);
        printf ("%sassert (var%d);\n", offset, arg->var_num);
        printf ("%slocal_next_token ();\n", offset);
        vars[arg->var_num] = 2;
      } else if (vars[arg->var_num] == 2) {
        printf ("%sif (!is_int ()) { return %s;}\n", offset, fail);
        printf ("%sif (vars%d != INT2PTR (get_int ())) { return %s; }\n", offset, arg->var_num, fail);
        printf ("%sout_int (get_int ());\n", offset);
        printf ("%slocal_next_token ();\n", offset);
      } else {
        assert (0);
        return -1;
      }
    }
  } else {
    int t = TL_TREE_METHODS (arg->type)->type (arg->type);
    if (t == NODE_TYPE_TYPE || t == NODE_TYPE_VAR_TYPE) {    
      printf ("%sstruct paramed_type *field%d = \n", offset, num);
      assert (gen_create (arg->type, vars, 2 + o) >= 0);
      printf (";\n");
      int bare = arg->flags & FLAG_BARE;
      if (!bare && t == NODE_TYPE_TYPE) {
        bare = ((struct tl_tree_type *)arg->type)->self.flags & FLAG_BARE;
      }
      if (!bare) {
        printf ("%sif (store_type_%s (field%d) < 0) { return %s;}\n", offset, t == NODE_TYPE_VAR_TYPE ? "any" : ((struct tl_tree_type *)arg->type)->type->print_id, num, fail);
      } else {
        printf ("%sif (store_type_bare_%s (field%d) < 0) { return %s;}\n", offset, t == NODE_TYPE_VAR_TYPE ? "any" : ((struct tl_tree_type *)arg->type)->type->print_id, num, fail);
      }
    } else {
      printf ("%s%s (\"[\", 1);\n", offset, expect);
      
      assert (t == NODE_TYPE_ARRAY);
      printf ("%sint multiplicity%d = PTR2INT (\n", offset, num);
      assert (gen_create (((struct tl_tree_array *)arg->type)->multiplicity, vars, 2 + o) >= 0);
      printf ("%s);\n", offset);
      printf ("%sstruct paramed_type *field%d = \n", offset, num);
      assert (gen_create (((struct tl_tree_array *)arg->type)->args[0]->type, vars, 2 + o) >= 0);
      printf (";\n");
      printf ("%swhile (multiplicity%d -- > 0) {\n", offset, num);
      printf ("%s  if (store_type_%s (field%d) < 0) { return %s;}\n", offset, "any", num, fail);
      printf ("%s}\n", offset);
      
      printf ("%s%s (\"]\", 1);\n", offset, expect);
    }
  }
  if (arg->exist_var_num >= 0) {
    printf ("  }\n");
  }
  return 0;
}

int gen_field_autocomplete (struct arg *arg, int *vars, int num, int from_func, int empty) {
  assert (arg);
  char *offset = "  ";
  int o = 0;
  if (arg->exist_var_num >= 0) {
    printf ("  if (PTR2INT (var%d) & (1 << %d)) {\n", arg->exist_var_num, arg->exist_var_bit);
    offset = "    ";
    o = 2;
  }
  char *fail = from_func ? "0" : "-1";
  char *expect = from_func ? "expect_token_ptr_autocomplete" : "expect_token_autocomplete";
  if (arg->id && strlen (arg->id) > 0 && !empty) {
    printf ("%sif (cur_token_len == -3 && cur_token_real_len <= %d && !cur_token_quoted && !memcmp (cur_token, \"%s\", cur_token_real_len)) {\n", offset, (int)(strlen (arg->id)), arg->id);
    printf ("%s  set_autocomplete_string (\"%s\");\n", offset, arg->id);
    printf ("%s  return %s;\n", offset, fail);
    printf ("%s}\n", offset);

    printf ("%sif (cur_token_len >= 0 && cur_token_len == %d && !memcmp (cur_token, \"%s\", cur_token_len)) {\n", offset, (int)(strlen (arg->id)), arg->id);
    printf ("%s  local_next_token ();\n", offset);
    printf ("%s  %s (\":\", 1);\n", offset, expect);
    printf ("%s}\n", offset);
  }
  if (arg->var_num >= 0) {
    printf ("%sif (cur_token_len < 0) { return %s; }\n", offset, fail);
    assert (TL_TREE_METHODS (arg->type)->type (arg->type) == NODE_TYPE_TYPE);
    int t = ((struct tl_tree_type *)arg->type)->type->name;
    if (t == NAME_VAR_TYPE) {
      fprintf (stderr, "Not supported yet\n");
      assert (0);
    } else {
      assert (t == NAME_VAR_NUM);
      if (vars[arg->var_num] == 0) {
        printf ("%sif (!is_int ()) { return %s;}\n", offset, fail);
        printf ("%sstruct paramed_type *var%d = INT2PTR (get_int ());\n", offset, arg->var_num);
        printf ("%sassert (var%d);\n", offset, arg->var_num);
        printf ("%slocal_next_token ();\n", offset);
        vars[arg->var_num] = 2;
      } else if (vars[arg->var_num] == 2) {
        printf ("%sif (!is_int ()) { return %s;}\n", offset, fail);
        printf ("%sif (vars%d != INT2PTR (get_int ())) { return %s; }\n", offset, arg->var_num, fail);
        printf ("%slocal_next_token ();\n", offset);
      } else {
        assert (0);
        return -1;
      }
    }
  } else {
    int t = TL_TREE_METHODS (arg->type)->type (arg->type);
    if (t == NODE_TYPE_TYPE || t == NODE_TYPE_VAR_TYPE) {    
      printf ("%sstruct paramed_type *field%d = \n", offset, num);
      assert (gen_create (arg->type, vars, 2 + o) >= 0);
      printf (";\n");
      int bare = arg->flags & FLAG_BARE;
      if (!bare && t == NODE_TYPE_TYPE) {
        bare = ((struct tl_tree_type *)arg->type)->self.flags & FLAG_BARE;
      }
      if (!bare) {
        printf ("%sif (autocomplete_type_%s (field%d) < 0) { return %s;}\n", offset, t == NODE_TYPE_VAR_TYPE ? "any" : ((struct tl_tree_type *)arg->type)->type->print_id, num, fail);
      } else {
        printf ("%sif (autocomplete_type_bare_%s (field%d) < 0) { return %s;}\n", offset, t == NODE_TYPE_VAR_TYPE ? "any" : ((struct tl_tree_type *)arg->type)->type->print_id, num, fail);
      }
    } else {
      printf ("%s%s (\"[\", 1);\n", offset, expect);
      
      assert (t == NODE_TYPE_ARRAY);
      printf ("%sint multiplicity%d = PTR2INT (\n", offset, num);
      assert (gen_create (((struct tl_tree_array *)arg->type)->multiplicity, vars, 2 + o) >= 0);
      printf ("%s);\n", offset);
      printf ("%sstruct paramed_type *field%d = \n", offset, num);
      assert (gen_create (((struct tl_tree_array *)arg->type)->args[0]->type, vars, 2 + o) >= 0);
      printf (";\n");
      printf ("%swhile (multiplicity%d -- > 0) {\n", offset, num);
      printf ("%s  if (autocomplete_type_%s (field%d) < 0) { return %s;}\n", offset, "any", num, fail);
      printf ("%s}\n", offset);
      
      printf ("%s%s (\"]\", 1);\n", offset, expect);
    }
  }
  if (arg->exist_var_num >= 0) {
    printf ("  }\n");
  }
  return 0;
}

int gen_field_fetch_ds (struct arg *arg, int *vars, int num, int empty) {
  assert (arg);
  char *offset = "  ";
  int o = 0;
  if (arg->exist_var_num >= 0) {
    printf ("  if (PTR2INT (var%d) & (1 << %d)) {\n", arg->exist_var_num, arg->exist_var_bit);
    offset = "    ";
    o = 2;
  }
  if (arg->var_num >= 0) {
    assert (TL_TREE_METHODS (arg->type)->type (arg->type) == NODE_TYPE_TYPE);
    int t = ((struct tl_tree_type *)arg->type)->type->name;
    if (t == NAME_VAR_TYPE) {
      fprintf (stderr, "Not supported yet\n");
      assert (0);
    } else {
      assert (t == NAME_VAR_NUM);
      printf ("%sassert (in_remaining () >= 4);\n", offset);
      if (arg->id && strlen (arg->id)) {
        printf ("%sresult->%s = talloc (4);", offset, arg->id);
        printf ("%s*result->%s = prefetch_int ();", offset, arg->id);
      } else {
        printf ("%sresult->f%d = talloc (4);", offset, num - 1);
        printf ("%s*result->f%d = prefetch_int ();", offset, num - 1);
      }
      if (vars[arg->var_num] == 0) {
        printf ("%sstruct paramed_type *var%d = INT2PTR (fetch_int ());\n", offset, arg->var_num);
        vars[arg->var_num] = 2;
      } else if (vars[arg->var_num] == 2) {
        printf ("%sassert (vars%d == INT2PTR (fetch_int ()));\n", offset, arg->var_num);
      } else {
        assert (0);
        return -1;
      }
    }
  } else {
    int t = TL_TREE_METHODS (arg->type)->type (arg->type);
    if (t == NODE_TYPE_TYPE || t == NODE_TYPE_VAR_TYPE) {    
      printf ("%sstruct paramed_type *field%d = \n", offset, num);
      assert (gen_create (arg->type, vars, 2 + o) >= 0);
      printf (";\n");
      int bare = arg->flags & FLAG_BARE;
      if (!bare && t == NODE_TYPE_TYPE) {
        bare = ((struct tl_tree_type *)arg->type)->self.flags & FLAG_BARE;
      }
      if (arg->id && strlen (arg->id)) {
        printf ("%sresult->%s = ", offset, arg->id);
      } else {
        printf ("%sresult->f%d = ", offset, num - 1);
      }
      if (t == NODE_TYPE_TYPE && !strcmp (((struct tl_tree_type *)arg->type)->type->id, "Vector")) {
        printf ("(void *)");
      }
      if (!bare) {
        printf ("fetch_ds_type_%s (field%d);\n", t == NODE_TYPE_VAR_TYPE ? "any" : ((struct tl_tree_type *)arg->type)->type->print_id, num);
      } else {
        printf ("fetch_ds_type_bare_%s (field%d);\n", t == NODE_TYPE_VAR_TYPE ? "any" : ((struct tl_tree_type *)arg->type)->type->print_id, num);
      }
    } else {
      assert (t == NODE_TYPE_ARRAY);
      printf ("%sint multiplicity%d = PTR2INT (\n", offset, num);
      assert (gen_create (((struct tl_tree_array *)arg->type)->multiplicity, vars, 2 + o) >= 0);
      printf ("%s);\n", offset);
      printf ("%sstruct paramed_type *field%d = \n", offset, num);
      assert (gen_create (((struct tl_tree_array *)arg->type)->args[0]->type, vars, 2 + o) >= 0);
      printf (";\n");
      if (arg->id && strlen (arg->id)) {
        printf ("%sresult->%s = ", offset, arg->id);
      } else {
        printf ("%sresult->f%d = ", offset, num - 1);
      }
      printf ("talloc0 (multiplicity%d * sizeof (void *));\n", num);
      printf ("%s{\n", offset);
      printf ("%s  int i = 0;\n", offset);
      printf ("%s  while (i < multiplicity%d) {\n", offset, num);
      if (arg->id && strlen (arg->id)) {
        printf ("%s    result->%s[i ++] =", offset, arg->id);
      } else {
        printf ("%s    result->f%d[i ++] = ", offset, num - 1);
      }
      printf ("fetch_ds_type_%s (field%d);\n", "any", num);
      printf ("%s  }\n", offset);
      printf ("%s}\n", offset);
    }
  }
  if (arg->exist_var_num >= 0) {
    printf ("  }\n");
  }
  return 0;
}

int gen_field_free_ds (struct arg *arg, int *vars, int num, int empty) {
  assert (arg);
  char *offset = "  ";
  int o = 0;
  if (arg->exist_var_num >= 0) {
    printf ("  if (PTR2INT (var%d) & (1 << %d)) {\n", arg->exist_var_num, arg->exist_var_bit);
    offset = "    ";
    o = 2;
  }
  if (arg->var_num >= 0) {
    assert (TL_TREE_METHODS (arg->type)->type (arg->type) == NODE_TYPE_TYPE);
    int t = ((struct tl_tree_type *)arg->type)->type->name;
    if (t == NAME_VAR_TYPE) {
      fprintf (stderr, "Not supported yet\n");
      assert (0);
    } else {
      assert (t == NAME_VAR_NUM);
      if (arg->id && strlen (arg->id)) {
        if (vars[arg->var_num] == 0) {
          printf ("%sstruct paramed_type *var%d = INT2PTR (*D->%s);\n", offset, arg->var_num, arg->id);
          vars[arg->var_num] = 2;
        } else if (vars[arg->var_num] == 2) {
          printf ("%sassert (vars%d == INT2PTR (*D->%s));\n", offset, arg->var_num, arg->id);
        }
        printf ("%stfree (D->%s, sizeof (*D->%s));\n", offset, arg->id, arg->id);
      } else {
        if (vars[arg->var_num] == 0) {
          printf ("%sstruct paramed_type *var%d = INT2PTR (*D->f%d);\n", offset, arg->var_num, num - 1);
          vars[arg->var_num] = 2;
        } else if (vars[arg->var_num] == 2) {
          printf ("%sassert (vars%d == *D->f%d);\n", offset, arg->var_num, num - 1);
        }
        printf ("%stfree (D->f%d, sizeof (*D->f%d));\n", offset, num - 1, num - 1);
      }
    }
  } else {
    int t = TL_TREE_METHODS (arg->type)->type (arg->type);
    if (t == NODE_TYPE_TYPE || t == NODE_TYPE_VAR_TYPE) {    
      printf ("%sstruct paramed_type *field%d = \n", offset, num);
      assert (gen_create (arg->type, vars, 2 + o) >= 0);
      printf (";\n");
      int any = (t == NODE_TYPE_VAR_TYPE) || ((struct tl_tree_type *)arg->type)->type->name == NAME_VECTOR;
      if (arg->id && strlen (arg->id)) {
        printf ("%sfree_ds_type_%s (D->%s, field%d);\n", offset, any ? "any" : ((struct tl_tree_type *)arg->type)->type->print_id, arg->id, num);      
      } else {
        printf ("%sfree_ds_type_%s (D->f%d, field%d);\n", offset, any ? "any" : ((struct tl_tree_type *)arg->type)->type->print_id, num - 1, num);      
      }
    } else {
      assert (t == NODE_TYPE_ARRAY);
      printf ("%sint multiplicity%d = PTR2INT (\n", offset, num);
      assert (gen_create (((struct tl_tree_array *)arg->type)->multiplicity, vars, 2 + o) >= 0);
      printf ("%s);\n", offset);
      printf ("%sstruct paramed_type *field%d = \n", offset, num);
      assert (gen_create (((struct tl_tree_array *)arg->type)->args[0]->type, vars, 2 + o) >= 0);
      printf (";\n");
      printf ("%s{\n", offset);
      printf ("%s  int i = 0;\n", offset);
      printf ("%s  while (i < multiplicity%d) {\n", offset, num);
      if (arg->id && strlen (arg->id)) {
        printf ("%s    free_ds_type_%s (D->%s[i ++], field%d);\n", offset, "any", arg->id, num);
      } else {
        printf ("%s    free_ds_type_%s (D->f%d[i ++], field%d);\n", offset, "any", num - 1, num);
      }
      printf ("%s  }\n", offset);
      printf ("%s}\n", offset);
      if (arg->id && strlen (arg->id)) {
        printf ("%stfree (D->%s, sizeof (void *) * multiplicity%d);\n", offset, arg->id, num);
      } else {
        printf ("%stfree (D->f%d, sizeof (void *) * multiplicity%d);\n", offset, num - 1, num);
      }
    }
  }
  if (arg->exist_var_num >= 0) {
    printf ("  }\n");
  }
  return 0;
}

int gen_field_store_ds (struct arg *arg, int *vars, int num, int empty) {
  assert (arg);
  char *offset = "  ";
  int o = 0;
  if (arg->exist_var_num >= 0) {
    printf ("  if (PTR2INT (var%d) & (1 << %d)) {\n", arg->exist_var_num, arg->exist_var_bit);
    offset = "    ";
    o = 2;
  }
  if (arg->var_num >= 0) {
    assert (TL_TREE_METHODS (arg->type)->type (arg->type) == NODE_TYPE_TYPE);
    int t = ((struct tl_tree_type *)arg->type)->type->name;
    if (t == NAME_VAR_TYPE) {
      fprintf (stderr, "Not supported yet\n");
      assert (0);
    } else {
      assert (t == NAME_VAR_NUM);
      if (arg->id && strlen (arg->id)) {
        if (vars[arg->var_num] == 0) {
          printf ("%sstruct paramed_type *var%d = INT2PTR (*D->%s);\n", offset, arg->var_num, arg->id);
          vars[arg->var_num] = 2;
        } else if (vars[arg->var_num] == 2) {
          printf ("%sassert (vars%d == INT2PTR (*D->%s));\n", offset, arg->var_num, arg->id);
        }
        printf ("%sout_int (PTR2INT (var%d));\n", offset, arg->var_num);
      } else {
        if (vars[arg->var_num] == 0) {
          printf ("%sstruct paramed_type *var%d = INT2PTR (*D->f%d);\n", offset, arg->var_num, num - 1);
          vars[arg->var_num] = 2;
        } else if (vars[arg->var_num] == 2) {
          printf ("%sassert (vars%d == *D->f%d);\n", offset, arg->var_num, num - 1);
        }
        printf ("%sout_int (PTR2INT (var%d));\n", offset, arg->var_num);
      }
    }
  } else {
    int t = TL_TREE_METHODS (arg->type)->type (arg->type);
    if (t == NODE_TYPE_TYPE || t == NODE_TYPE_VAR_TYPE) {    
      int bare = arg->flags & FLAG_BARE;
      if (!bare && t == NODE_TYPE_TYPE) {
        bare = ((struct tl_tree_type *)arg->type)->self.flags & FLAG_BARE;
      }
      printf ("%sstruct paramed_type *field%d = \n", offset, num);
      assert (gen_create (arg->type, vars, 2 + o) >= 0);
      printf (";\n");
      int any = (t == NODE_TYPE_VAR_TYPE);
      int vec = ((struct tl_tree_type *)arg->type)->type->name == NAME_VECTOR;
      if (arg->id && strlen (arg->id)) {
        printf ("%sstore_ds_type_%s%s (%sD->%s, field%d);\n", offset, bare ? "bare_" : "", any ? "any" : ((struct tl_tree_type *)arg->type)->type->print_id, vec ? "(void *)" : "", arg->id, num);      
      } else {
        printf ("%sstore_ds_type_%s%s (%sD->f%d, field%d);\n", offset, bare ? "bare_" : "", any ? "any" : ((struct tl_tree_type *)arg->type)->type->print_id, vec ? "(void *)" : "", num - 1, num);      
      }
    } else {
      assert (t == NODE_TYPE_ARRAY);
      printf ("%sint multiplicity%d = PTR2INT (\n", offset, num);
      assert (gen_create (((struct tl_tree_array *)arg->type)->multiplicity, vars, 2 + o) >= 0);
      printf ("%s);\n", offset);
      printf ("%sstruct paramed_type *field%d = \n", offset, num);
      assert (gen_create (((struct tl_tree_array *)arg->type)->args[0]->type, vars, 2 + o) >= 0);
      printf (";\n");
      printf ("%s{\n", offset);
      printf ("%s  int i = 0;\n", offset);
      printf ("%s  while (i < multiplicity%d) {\n", offset, num);
      if (arg->id && strlen (arg->id)) {
        printf ("%s    store_ds_type_%s (D->%s[i ++], field%d);\n", offset, "any", arg->id, num);
      } else {
        printf ("%s    store_ds_type_%s (D->f%d[i ++], field%d);\n", offset, "any", num - 1, num);
      }
      printf ("%s  }\n", offset);
      printf ("%s}\n", offset);
    }
  }
  if (arg->exist_var_num >= 0) {
    printf ("  }\n");
  }
  return 0;
}

int gen_field_print_ds (struct arg *arg, int *vars, int num, int empty) {
  assert (arg);
  char *offset = "  ";
  int o = 0;
  if (arg->exist_var_num >= 0) {
    printf ("  if (PTR2INT (var%d) & (1 << %d)) {\n", arg->exist_var_num, arg->exist_var_bit);
    offset = "    ";
    o = 2;
  }
  if (!empty) {
    printf("%sif (multiline_output >= 2) { print_offset (); }\n", offset);
  }
  if (arg->id && strlen (arg->id) && !empty) {
    printf ("%sif (!disable_field_names) { eprintf (\" %s :\"); }\n", offset, arg->id);
  }
  if (arg->var_num >= 0) {
    assert (TL_TREE_METHODS (arg->type)->type (arg->type) == NODE_TYPE_TYPE);
    int t = ((struct tl_tree_type *)arg->type)->type->name;
    if (t == NAME_VAR_TYPE) {
      fprintf (stderr, "Not supported yet\n");
      assert (0);
    } else {
      if (arg->id && strlen (arg->id)) {
        if (vars[arg->var_num] == 0) {
          printf ("%sstruct paramed_type *var%d = INT2PTR (*DS->%s);\n", offset, arg->var_num, arg->id);
          vars[arg->var_num] = 2;
        } else if (vars[arg->var_num] == 2) {
          printf ("%sassert (vars%d == INT2PTR (*DS->%s));\n", offset, arg->var_num, arg->id);
        }
      } else {
        if (vars[arg->var_num] == 0) {
          printf ("%sstruct paramed_type *var%d = INT2PTR (*DS->f%d);\n", offset, arg->var_num, num - 1);
          vars[arg->var_num] = 2;
        } else if (vars[arg->var_num] == 2) {
          printf ("%sassert (vars%d == *DS->f%d);\n", offset, arg->var_num, num - 1);
        }
      }
      printf ("%seprintf (\" %%d\", (int)PTR2INT (var%d));\n", offset, arg->var_num);
    }
  } else {
    int t = TL_TREE_METHODS (arg->type)->type (arg->type);
    if (t == NODE_TYPE_TYPE || t == NODE_TYPE_VAR_TYPE) {    
      printf ("%sstruct paramed_type *field%d = \n", offset, num);
      assert (gen_create (arg->type, vars, 2 + o) >= 0);
      printf (";\n");
      int bare = arg->flags & FLAG_BARE;
      if (!bare && t == NODE_TYPE_TYPE) {
        bare = ((struct tl_tree_type *)arg->type)->self.flags & FLAG_BARE;
      }
      int any = (t == NODE_TYPE_VAR_TYPE);
      int vec = ((struct tl_tree_type *)arg->type)->type->name == NAME_VECTOR;
      if (arg->id && strlen (arg->id)) {
        printf ("%sprint_ds_type_%s%s (%sDS->%s, field%d);\n", offset, bare ? "bare_" : "", any ? "any" : ((struct tl_tree_type *)arg->type)->type->print_id, vec ? "(void *)" : "", arg->id, num);      
      } else {
        printf ("%sprint_ds_type_%s%s (%sDS->f%d, field%d);\n", offset, bare ? "bare_" : "", any ? "any" : ((struct tl_tree_type *)arg->type)->type->print_id, vec ? "(void *)" : "", num - 1, num);      
      }
    } else {
      assert (t == NODE_TYPE_ARRAY);
      printf ("%sint multiplicity%d = PTR2INT (\n", offset, num);
      assert (gen_create (((struct tl_tree_array *)arg->type)->multiplicity, vars, 2 + o) >= 0);
      printf ("%s);\n", offset);
      printf ("%sstruct paramed_type *field%d = \n", offset, num);
      assert (gen_create (((struct tl_tree_array *)arg->type)->args[0]->type, vars, 2 + o) >= 0);
      printf (";\n");
      printf ("%seprintf (\" [\");\n", offset);
      printf ("%sif (multiline_output >= 1) { eprintf (\"\\n\"); }\n", offset);
      printf ("%sif (multiline_output >= 1) { multiline_offset += multiline_offset_size;}\n", offset);
      printf ("%s{\n", offset);
      printf ("%s  int i = 0;\n", offset);
      printf ("%s  while (i < multiplicity%d) {\n", offset, num);
      printf ("%s    if (multiline_output >= 1) { print_offset (); }\n", offset);
      if (arg->id && strlen (arg->id)) {
        printf ("%s    print_ds_type_%s (DS->%s[i ++], field%d);\n", offset, "any", arg->id, num);
      } else {
        printf ("%s    print_ds_type_%s (DS->f%d[i ++], field%d);\n", offset, "any", num - 1, num);
      }
      printf ("%s    if (multiline_output >= 1) { eprintf (\"\\n\"); }\n", offset);
      printf ("%s  }\n", offset);
      printf ("%s}\n", offset);
      printf ("%sif (multiline_output >= 1) { multiline_offset -= multiline_offset_size; print_offset ();}\n", offset);
      printf ("%seprintf (\" ]\");\n", offset);
    }
  }
  if (!empty) {
    printf("%sif (multiline_output >= 2) { eprintf (\"\\n\"); }\n", offset);
  }
  if (arg->exist_var_num >= 0) {
    printf ("  }\n");
  }
  return 0;
}

int gen_field_autocomplete_excl (struct arg *arg, int *vars, int num, int from_func) {
  assert (arg);
  assert (arg->var_num < 0);
  char *offset = "  ";
  if (arg->exist_var_num >= 0) {
    printf ("  if (PTR2INT (var%d) & (1 << %d)) {\n", arg->exist_var_num, arg->exist_var_bit);
    offset = "    ";
  }
  char *fail = from_func ? "0" : "-1";
  char *expect = from_func ? "expect_token_ptr_autocomplete" : "expect_token_autocomplete";
  if (arg->id && strlen (arg->id) > 0) {
    printf ("%sif (cur_token_len == -3 && cur_token_real_len <= %d && !cur_token_quoted && !memcmp (cur_token, \"%s\", cur_token_real_len)) {\n", offset, (int)(strlen (arg->id)), arg->id);
    printf ("%s  set_autocomplete_string (\"%s\");\n", offset, arg->id);
    printf ("%s  return %s;\n", offset, fail);
    printf ("%s}\n", offset);

    printf ("%sif (cur_token_len >= 0 && cur_token_len == %d && !memcmp (cur_token, \"%s\", cur_token_len)) {\n", offset, (int)(strlen (arg->id)), arg->id);
    printf ("%s  local_next_token ();\n", offset);
    printf ("%s  %s (\":\", 1);\n", offset, expect);
    printf ("%s}\n", offset);
  }
  int t = TL_TREE_METHODS (arg->type)->type (arg->type);
  assert (t == NODE_TYPE_TYPE || t == NODE_TYPE_VAR_TYPE);
  printf ("%sstruct paramed_type *field%d = autocomplete_function_any ();\n", offset, num);
  printf ("%sif (!field%d) { return 0; }\n", offset, num);
  printf ("%sadd_var_to_be_freed (field%d);\n", offset, num);
  static char s[20];
#if defined(_MSC_VER) && _MSC_VER >= 1400
  sprintf_s (s, 20, "field%d", num);
#else
  sprintf (s, "field%d", num);
#endif
  gen_uni_skip (arg->type, s, vars, 1, 1);
  if (arg->exist_var_num >= 0) {
    printf ("  }\n");
  }
  return 0;
}

int gen_field_store_excl (struct arg *arg, int *vars, int num, int from_func) {
  assert (arg);
  assert (arg->var_num < 0);
  char *offset = "  ";
  if (arg->exist_var_num >= 0) {
    printf ("  if (PTR2INT (var%d) & (1 << %d)) {\n", arg->exist_var_num, arg->exist_var_bit);
    offset = "    ";
  }
  char *expect = from_func ? "expect_token_ptr" : "expect_token";
  if (arg->id && strlen (arg->id) > 0) {
    printf ("%sif (cur_token_len >= 0 && cur_token_len == %d && !cur_token_quoted && !memcmp (cur_token, \"%s\", cur_token_len)) {\n", offset, (int)(strlen (arg->id)), arg->id);
    printf ("%s  local_next_token ();\n", offset);
    printf ("%s  %s (\":\", 1);\n", offset, expect);
    printf ("%s}\n", offset);
  }
  int t = TL_TREE_METHODS (arg->type)->type (arg->type);
  assert (t == NODE_TYPE_TYPE || t == NODE_TYPE_VAR_TYPE);
  printf ("%sstruct paramed_type *field%d = store_function_any ();\n", offset, num);
  printf ("%sif (!field%d) { return 0; }\n", offset, num);
  static char s[20];
#if defined(_MSC_VER) && _MSC_VER >= 1400
  sprintf_s (s, 20, "field%d", num);
#else
  sprintf (s, "field%d", num);
#endif
  gen_uni_skip (arg->type, s, vars, 1, 1);
  if (arg->exist_var_num >= 0) {
    printf ("  }\n");
  }
  return 0;
}

void gen_constructor_skip (struct tl_combinator *c) {
  printf ("int skip_constructor_%s (struct paramed_type *T) {\n", c->print_id);
  int i;
  for (i = 0; i < c->args_num; i++) if (c->args[i]->flags & FLAG_EXCL) {
    printf (" return -1;\n");
    printf ("}\n");
    return;
  }
  static char s[10000];
#if defined(_MSC_VER) && _MSC_VER >= 1400
  sprintf_s (s, 10000, "T");
#else
  sprintf (s, "T");
#endif
  
  int *vars = malloc0 (c->var_num * 4);;
  gen_uni_skip (c->result, s, vars, 1, 0);
  
  if (c->name == NAME_INT) {
    printf ("  if (in_remaining () < 4) { return -1;}\n");
    printf ("  fetch_int ();\n");
    printf ("  return 0;\n");
    printf ("}\n");
    return;
  } else if (c->name == NAME_LONG) {
    printf ("  if (in_remaining () < 8) { return -1;}\n");
    printf ("  fetch_long ();\n");
    printf ("  return 0;\n");
    printf ("}\n");
    return;
  } else if (c->name == NAME_STRING) {
    printf ("  int l = prefetch_strlen ();\n");
    printf ("  if (l < 0) { return -1;}\n");
    printf ("  fetch_str (l);\n");
    printf ("  return 0;\n");
    printf ("}\n");
    return;
  } else if (c->name == NAME_DOUBLE) {
    printf ("  if (in_remaining () < 8) { return -1;}\n");
    printf ("  fetch_double ();\n");
    printf ("  return 0;\n");
    printf ("}\n");
    return;
  }

  for (i = 0; i < c->args_num; i++) if (!(c->args[i]->flags & FLAG_OPT_VAR)) {
    assert (gen_field_skip (c->args[i], vars, i + 1) >= 0);
  }
  free (vars);
  printf ("  return 0;\n");
  printf ("}\n"); 
}

void gen_constructor_fetch (struct tl_combinator *c) {
  printf ("int fetch_constructor_%s (struct paramed_type *T) {\n", c->print_id);
  int i;
  for (i = 0; i < c->args_num; i++) if (c->args[i]->flags & FLAG_EXCL) {
    printf (" return -1;\n");
    printf ("}\n");
    return;
  }
  static char s[10000];
#if defined(_MSC_VER) && _MSC_VER >= 1400
  sprintf_s (s, 10000, "T");
#else
  sprintf (s, "T");
#endif
  
  int *vars = malloc0 (c->var_num * 4);;
  gen_uni_skip (c->result, s, vars, 1, 0);

  if (c->name == NAME_INT) {
    printf ("  if (in_remaining () < 4) { return -1;}\n");
    printf ("  eprintf (\" %%d\", fetch_int ());\n");
    printf ("  return 0;\n");
    printf ("}\n");
    return;
  } else if (c->name == NAME_LONG) {
    printf ("  if (in_remaining () < 8) { return -1;}\n");
    printf ("  eprintf (\" %%"_PRINTF_INT64_"d\", fetch_long ());\n");
    printf ("  return 0;\n");
    printf ("}\n");
    return;
  } else if (c->name == NAME_STRING) {
    printf ("  static char buf[1 << 22];\n");
    printf ("  int l = prefetch_strlen ();\n");
    printf ("  if (l < 0 || (l >= (1 << 22) - 2)) { return -1; }\n");
    printf ("  memcpy (buf, fetch_str (l), l);\n");
    printf ("  buf[l] = 0;\n");
    printf ("  print_escaped_string (buf, l);\n");
    printf ("  return 0;\n");
    printf ("}\n");
    return;
  } else if (c->name == NAME_DOUBLE) {
    printf ("  if (in_remaining () < 8) { return -1;}\n");
    printf ("  eprintf (\" %%lf\", fetch_double ());\n");
    printf ("  return 0;\n");
    printf ("}\n");
    return;
  }
 
  assert (c->result->methods->type (c->result) == NODE_TYPE_TYPE);
  int empty = is_empty (((struct tl_tree_type *)c->result)->type);
  if (!empty) {
    printf ("  eprintf (\" %s\");\n", c->id);
    printf ("  if (multiline_output >= 2) { eprintf (\"\\n\"); }\n");
  }
  

  for (i = 0; i < c->args_num; i++) if (!(c->args[i]->flags & FLAG_OPT_VAR)) {
    assert (gen_field_fetch (c->args[i], vars, i + 1, empty) >= 0);
  }
  free (vars);
  printf ("  return 0;\n");
  printf ("}\n"); 
}

void gen_constructor_store (struct tl_combinator *c) {
  printf ("int store_constructor_%s (struct paramed_type *T) {\n", c->print_id);
  int i;
  for (i = 0; i < c->args_num; i++) if (c->args[i]->flags & FLAG_EXCL) {
    printf (" return -1;\n");
    printf ("}\n");
    return;
  }
  static char s[10000];
#if defined(_MSC_VER) && _MSC_VER >= 1400
  sprintf_s (s, 10000, "T");
#else
  sprintf (s, "T");
#endif
  
  int *vars = malloc0 (c->var_num * 4);;
  assert (c->var_num <= 10);
  gen_uni_skip (c->result, s, vars, 1, 0);

  if (c->name == NAME_INT) {
    printf ("  if (is_int ()) {\n");
    printf ("    out_int (get_int ());\n");
    printf ("    local_next_token ();\n");
    printf ("    return 0;\n");
    printf ("  } else {\n");
    printf ("    return -1;\n");
    printf ("  }\n");
    printf ("}\n");
    return;
  } else if (c->name == NAME_LONG) {
    printf ("  if (is_int ()) {\n");
    printf ("    out_long (get_int ());\n");
    printf ("    local_next_token ();\n");
    printf ("    return 0;\n");
    printf ("  } else {\n");
    printf ("    return -1;\n");
    printf ("  }\n");
    printf ("}\n");
    return;
  } else if (c->name == NAME_STRING) {
    printf ("  if (cur_token_len >= 0) {\n");
    printf ("    out_cstring (cur_token, cur_token_len);\n");
    printf ("    local_next_token ();\n");
    printf ("    return 0;\n");
    printf ("  } else {\n");
    printf ("    return -1;\n");
    printf ("  }\n");
    printf ("}\n");
    return;
  } else if (c->name == NAME_DOUBLE) {
    printf ("  if (is_double ()) {\n");
    printf ("    out_double (get_double());\n");
    printf ("    local_next_token ();\n");
    printf ("    return 0;\n");
    printf ("  } else {\n");
    printf ("    return -1;\n");
    printf ("  }\n");
    printf ("}\n");
    return;
  }

  int empty = is_empty (((struct tl_tree_type *)c->result)->type);
  for (i = 0; i < c->args_num; i++) if (!(c->args[i]->flags & FLAG_OPT_VAR)) {
    assert (gen_field_store (c->args[i], vars, i + 1, 0, empty) >= 0);
  }

  free (vars);
  printf ("  return 0;\n");
  printf ("}\n"); 
}

void gen_constructor_autocomplete (struct tl_combinator *c) {
  printf ("int autocomplete_constructor_%s (struct paramed_type *T) {\n", c->print_id);
  int i;
  for (i = 0; i < c->args_num; i++) if (c->args[i]->flags & FLAG_EXCL) {
    printf (" return -1;\n");
    printf ("}\n");
    return;
  }
  static char s[10000];
#if defined(_MSC_VER) && _MSC_VER >= 1400
  sprintf_s (s, 10000, "T");
#else
  sprintf (s, "T");
#endif
  
  int *vars = malloc0 (c->var_num * 4);;
  assert (c->var_num <= 10);
  gen_uni_skip (c->result, s, vars, 1, 0);

  if (c->name == NAME_INT) {
    printf ("  if (is_int ()) {\n");
    printf ("    local_next_token ();\n");
    printf ("    return 0;\n");
    printf ("  } else {\n");
    printf ("    return -1;\n");
    printf ("  }\n");
    printf ("}\n");
    return;
  } else if (c->name == NAME_LONG) {
    printf ("  if (is_int ()) {\n");
    printf ("    local_next_token ();\n");
    printf ("    return 0;\n");
    printf ("  } else {\n");
    printf ("    return -1;\n");
    printf ("  }\n");
    printf ("}\n");
    return;
  } else if (c->name == NAME_STRING) {
    printf ("  if (cur_token_len >= 0) {\n");
    printf ("    local_next_token ();\n");
    printf ("    return 0;\n");
    printf ("  } else {\n");
    printf ("    return -1;\n");
    printf ("  }\n");
    printf ("}\n");
    return;
  } else if (c->name == NAME_DOUBLE) {
    printf ("  if (is_double ()) {\n");
    printf ("    local_next_token ();\n");
    printf ("    return 0;\n");
    printf ("  } else {\n");
    printf ("    return -1;\n");
    printf ("  }\n");
    printf ("}\n");
    return;
  }

  int empty = is_empty (((struct tl_tree_type *)c->result)->type);
  for (i = 0; i < c->args_num; i++) if (!(c->args[i]->flags & FLAG_OPT_VAR)) {
    assert (gen_field_autocomplete (c->args[i], vars, i + 1, 0, empty) >= 0);
  }

  free (vars);
  printf ("  return 0;\n");
  printf ("}\n"); 
}

void gen_constructor_fetch_ds (struct tl_combinator *c) {
  print_c_type_name (c->result, "", 0);
  printf ("fetch_ds_constructor_%s (struct paramed_type *T) {\n", c->print_id);
  int i;
  for (i = 0; i < c->args_num; i++) if (c->args[i]->flags & FLAG_EXCL) {
    printf (" assert (0);\n");
    printf ("}\n");
    return;
  }
  static char s[10000];
#if defined(_MSC_VER) && _MSC_VER >= 1400
  sprintf_s (s, 10000, "T");
#else
  sprintf (s, "T");
#endif
  
  int *vars = malloc0 (c->var_num * 4);;
  gen_uni_skip (c->result, s, vars, 1, 1);

  printf ("  ");
  print_c_type_name (c->result, "  ", 0);
  printf ("  result = talloc0 (sizeof (*result));\n");

  struct tl_type *T = ((struct tl_tree_type *)c->result)->type;
  if (T->constructors_num > 1) {
    printf ("  result->magic = 0x%08x;\n", c->name);
  }

  if (c->name == NAME_INT) {
    printf ("  assert (in_remaining () >= 4);\n");
    printf ("  *result = fetch_int ();\n");
    printf ("  return result;\n");
    printf ("}\n");
    return;
  } else if (c->name == NAME_LONG) {
    printf ("  assert (in_remaining () >= 8);\n");
    printf ("  *result = fetch_long ();\n");
    printf ("  return result;\n");
    printf ("}\n");
    return;
  } else if (c->name == NAME_STRING || c->name == NAME_BYTES) {
    printf ("  assert (in_remaining () >= 4);\n");
    printf ("  int l = prefetch_strlen ();\n");
    printf ("  assert (l >= 0);\n");
    printf ("  result->len = l;\n");
    printf ("  result->data = talloc (l + 1);\n");
    printf ("  result->data[l] = 0;\n");
    printf ("  memcpy (result->data, fetch_str (l), l);\n");
    printf ("  return result;\n");
    printf ("}\n");
    return;
  } else if (c->name == NAME_DOUBLE) {
    printf ("  assert (in_remaining () >= 8);\n");
    printf ("  *result = fetch_double ();\n");
    printf ("  return result;\n");
    printf ("}\n");
    return;
  }
 
  assert (c->result->methods->type (c->result) == NODE_TYPE_TYPE);
  int empty = is_empty (((struct tl_tree_type *)c->result)->type);

  for (i = 0; i < c->args_num; i++) if (!(c->args[i]->flags & FLAG_OPT_VAR)) {
    assert (gen_field_fetch_ds (c->args[i], vars, i + 1, empty) >= 0);
  }
  free (vars);
  printf ("  return result;\n");
  printf ("}\n"); 
}

void gen_constructor_free_ds (struct tl_combinator *c) {
  printf ("void free_ds_constructor_%s (", c->print_id);
  print_c_type_name (c->result, "", 0);
  printf ("D, struct paramed_type *T) {\n");
  int i;
  for (i = 0; i < c->args_num; i++) if (c->args[i]->flags & FLAG_EXCL) {
    printf (" assert (0);\n");
    printf ("}\n");
    return;
  }

  static char s[10000];
#if defined(_MSC_VER) && _MSC_VER >= 1400
  sprintf_s (s, 10000, "T");
#else
  sprintf (s, "T");
#endif
  
  int *vars = malloc0 (c->var_num * 4);;
  gen_uni_skip (c->result, s, vars, 1, -1);

  //printf ("  ");
  //print_c_type_name (c->result, "  ", 0);
  //printf ("  result = talloc0 (sizeof (*result));\n");

  //struct tl_type *T = ((struct tl_tree_type *)c->result)->type;

  if (c->name == NAME_INT) {
    printf ("  tfree (D, sizeof (*D));\n");
    printf ("}\n");
    return;
  } else if (c->name == NAME_LONG) {
    printf ("  tfree (D, sizeof (*D));\n");
    printf ("}\n");
    return;
  } else if (c->name == NAME_STRING || c->name == NAME_BYTES) {
    printf ("  tfree (D->data, D->len + 1);\n");
    printf ("  tfree (D, sizeof (*D));\n");
    printf ("}\n");
    return;
  } else if (c->name == NAME_DOUBLE) {
    printf ("  tfree (D, sizeof (*D));\n");
    printf ("}\n");
    return;
  }
 
  assert (c->result->methods->type (c->result) == NODE_TYPE_TYPE);
  int empty = is_empty (((struct tl_tree_type *)c->result)->type);

  for (i = 0; i < c->args_num; i++) if (!(c->args[i]->flags & FLAG_OPT_VAR)) {
    assert (gen_field_free_ds (c->args[i], vars, i + 1, empty) >= 0);
  }
  free (vars);
  printf ("}\n"); 
}

void gen_constructor_store_ds (struct tl_combinator *c) {
  printf ("void store_ds_constructor_%s (", c->print_id);
  print_c_type_name (c->result, "", 0);
  printf ("D, struct paramed_type *T) {\n");
  int i;
  for (i = 0; i < c->args_num; i++) if (c->args[i]->flags & FLAG_EXCL) {
    printf (" assert (0);\n");
    printf ("}\n");
    return;
  }

  static char s[10000];
#if defined(_MSC_VER) && _MSC_VER >= 1400
  sprintf_s (s, 10000, "T");
#else
  sprintf (s, "T");
#endif
  
  int *vars = malloc0 (c->var_num * 4);;
  gen_uni_skip (c->result, s, vars, 1, -1);

  //printf ("  ");
  //print_c_type_name (c->result, "  ", 0);
  //printf ("  result = talloc0 (sizeof (*result));\n");

  //struct tl_type *T = ((struct tl_tree_type *)c->result)->type;

  if (c->name == NAME_INT) {
    printf ("  out_int (*D);\n");
    printf ("}\n");
    return;
  } else if (c->name == NAME_LONG) {
    printf ("  out_long (*D);\n");
    printf ("}\n");
    return;
  } else if (c->name == NAME_STRING || c->name == NAME_BYTES) {
    printf ("  out_cstring (D->data, D->len);\n");
    printf ("}\n");
    return;
  } else if (c->name == NAME_DOUBLE) {
    printf ("  out_double (*D);\n");
    printf ("}\n");
    return;
  }
 
  assert (c->result->methods->type (c->result) == NODE_TYPE_TYPE);
  int empty = is_empty (((struct tl_tree_type *)c->result)->type);

  for (i = 0; i < c->args_num; i++) if (!(c->args[i]->flags & FLAG_OPT_VAR)) {
    assert (gen_field_store_ds (c->args[i], vars, i + 1, empty) >= 0);
  }
  free (vars);
  printf ("}\n"); 
}

void gen_constructor_print_ds (struct tl_combinator *c) {
  printf ("int print_ds_constructor_%s (", c->print_id);
  print_c_type_name (c->result, "", 0);
  printf ("DS, struct paramed_type *T) {\n");
  int i;
  for (i = 0; i < c->args_num; i++) if (c->args[i]->flags & FLAG_EXCL) {
    printf (" return -1;\n");
    printf ("}\n");
    return;
  }
  static char s[10000];
#if defined(_MSC_VER) && _MSC_VER >= 1400
  sprintf_s (s, 10000, "T");
#else
  sprintf (s, "T");
#endif
  
  int *vars = malloc0 (c->var_num * 4);;
  gen_uni_skip (c->result, s, vars, 1, 0);

  if (c->name == NAME_INT) {
    printf ("  eprintf (\" %%d\", *DS);\n");
    printf ("  return 0;\n");
    printf ("}\n");
    return;
  } else if (c->name == NAME_LONG) {
    printf ("  eprintf (\" %%"_PRINTF_INT64_"d\", *DS);\n");
    printf ("  return 0;\n");
    printf ("}\n");
    return;
  } else if (c->name == NAME_STRING || c->name == NAME_BYTES) {
    printf ("  print_escaped_string (DS->data, DS->len);\n");
    printf ("  return 0;\n");
    printf ("}\n");
    return;
  } else if (c->name == NAME_DOUBLE) {
    printf ("  eprintf (\" %%lf\", *DS);\n");
    printf ("  return 0;\n");
    printf ("}\n");
    return;
  }
 
  assert (c->result->methods->type (c->result) == NODE_TYPE_TYPE);
  int empty = is_empty (((struct tl_tree_type *)c->result)->type);
  if (!empty) {
    printf ("  eprintf (\" %s\");\n", c->id);
    printf ("  if (multiline_output >= 2) { eprintf (\"\\n\"); }\n");
  }
  

  for (i = 0; i < c->args_num; i++) if (!(c->args[i]->flags & FLAG_OPT_VAR)) {
    assert (gen_field_print_ds (c->args[i], vars, i + 1, empty) >= 0);
  }
  free (vars);
  printf ("  return 0;\n");
  printf ("}\n"); 
}

void gen_type_skip (struct tl_type *t) {
  printf ("int skip_type_%s (struct paramed_type *T) {\n", t->print_id);
  printf ("  if (in_remaining () < 4) { return -1;}\n");
  printf ("  int magic = fetch_int ();\n");
  printf ("  switch (magic) {\n");
  int i;
  for (i = 0; i < t->constructors_num; i++) {
     printf ("  case 0x%08x: return skip_constructor_%s (T);\n", t->constructors[i]->name, t->constructors[i]->print_id);
  }
  printf ("  default: return -1;\n");
  printf ("  }\n");
  printf ("}\n");
  printf ("int skip_type_bare_%s (struct paramed_type *T) {\n", t->print_id);
  if (t->constructors_num > 1) {
    printf ("  int *save_in_ptr = in_ptr;\n");
    for (i = 0; i < t->constructors_num; i++) {
      printf ("  if (skip_constructor_%s (T) >= 0) { return 0; }\n", t->constructors[i]->print_id);
      printf ("  in_ptr = save_in_ptr;\n");
    }
  } else {
    for (i = 0; i < t->constructors_num; i++) {
      printf ("  if (skip_constructor_%s (T) >= 0) { return 0; }\n", t->constructors[i]->print_id);
    }
  }
  printf ("  return -1;\n");
  printf ("}\n");
}

void gen_type_fetch (struct tl_type *t) {
  int empty = is_empty (t);;
  printf ("int fetch_type_%s (struct paramed_type *T) {\n", t->print_id);
  printf ("  if (in_remaining () < 4) { return -1;}\n");
  if (!empty) {
    printf ("  if (multiline_output >= 2) { multiline_offset += multiline_offset_size; }\n");
    printf ("  eprintf (\" (\");\n");
  }
  printf ("  int magic = fetch_int ();\n");
  printf ("  int res = -1;\n");
  printf ("  switch (magic) {\n");
  int i;
  for (i = 0; i < t->constructors_num; i++) {
     printf ("  case 0x%08x: res = fetch_constructor_%s (T); break;\n", t->constructors[i]->name, t->constructors[i]->print_id);
  }
  printf ("  default: return -1;\n");
  printf ("  }\n");
  if (!empty) {
    printf ("  if (res >= 0) {\n");
    printf ("    if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); }\n");
    printf ("    eprintf (\" )\");\n");
    //printf ("    if (multiline_output >= 2) { printf (\"\\n\"); }\n");
    printf ("  }\n");
  }
  printf ("  return res;\n");
  printf ("}\n");
  printf ("int fetch_type_bare_%s (struct paramed_type *T) {\n", t->print_id);
  if (t->constructors_num > 1) {
    printf ("  int *save_in_ptr = in_ptr;\n");
  
    if (!empty) {
      printf ("  if (multiline_output >= 2) { multiline_offset += multiline_offset_size; }\n");
    }
    for (i = 0; i < t->constructors_num; i++) {
      printf ("  if (skip_constructor_%s (T) >= 0) { in_ptr = save_in_ptr; %sassert (!fetch_constructor_%s (T)); %sreturn 0; }\n", t->constructors[i]->print_id, empty ? "" : "eprintf (\" (\"); ", t->constructors[i]->print_id , empty ? "" : "if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); } eprintf (\" )\");");
      printf ("  in_ptr = save_in_ptr;\n");
    }
  } else {
    for (i = 0; i < t->constructors_num; i++) {
      if (!empty) {
        printf ("  if (multiline_output >= 2) { multiline_offset += multiline_offset_size; }\n");
        printf ("  eprintf (\" (\");\n");
      }
      printf ("  if (fetch_constructor_%s (T) >= 0) { %sreturn 0; }\n", t->constructors[i]->print_id, empty ? "" : "if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); } eprintf (\" )\");" );
    }
  }
  printf ("  return -1;\n");
  printf ("}\n");
}

void gen_type_store (struct tl_type *t) {
  int empty = is_empty (t);;
  int k = 0;
  for (k = 0; k < 2; k++) {
    printf ("int store_type_%s%s (struct paramed_type *T) {\n", k == 0 ? "" : "bare_", t->print_id);
    if (empty) {
      if (!k) {
        printf ("    out_int (0x%08x);\n", t->constructors[0]->name);
      }
      printf ("  if (store_constructor_%s (T) < 0) { return -1; }\n", t->constructors[0]->print_id);
      printf ("  return 0;\n");
      printf ("}\n");
    } else {
      printf ("  expect_token (\"(\", 1);\n");
      printf ("  if (cur_token_len < 0) { return -1; }\n");
      printf ("  if (cur_token_len < 0) { return -1; }\n");      
      int i;
      for (i = 0; i < t->constructors_num; i++) {
        printf ("  if (cur_token_len == %d && !memcmp (cur_token, \"%s\", cur_token_len)) {\n", (int)strlen (t->constructors[i]->id), t->constructors[i]->id);
        if (!k) {
          printf ("    out_int (0x%08x);\n", t->constructors[i]->name);
        }
        printf ("    local_next_token ();\n");
        printf ("    if (store_constructor_%s (T) < 0) { return -1; }\n", t->constructors[i]->print_id);
        printf ("    expect_token (\")\", 1);\n");
        printf ("    return 0;\n");
        printf ("  }\n");
      }
      /*if (t->constructors_num == 1) {
        printf ("  if (!force) {\n");
        if (!k) {
          printf ("    out_int (0x%08x);\n", t->constructors[0]->name);
        }
        printf ("    if (store_constructor_%s (T) < 0) { return -1; }\n", t->constructors[0]->print_id);
        printf ("    expect_token (\")\", 1);\n");
        printf ("    return 0;\n");
        printf ("  }\n");
      }*/
      printf ("  return -1;\n");
      printf ("}\n");
    }
  }
}

void gen_type_autocomplete (struct tl_type *t) {
  int empty = is_empty (t);;
  int k = 0;
  for (k = 0; k < 2; k++) {
    printf ("int autocomplete_type_%s%s (struct paramed_type *T) {\n", k == 0 ? "" : "bare_", t->print_id);
    if (empty) {
      printf ("  if (autocomplete_constructor_%s (T) < 0) { return -1; }\n", t->constructors[0]->print_id);
      printf ("  return 0;\n");
      printf ("}\n");
    } else {
      printf ("  expect_token_autocomplete (\"(\", 1);\n");
      printf ("  if (cur_token_len == -3) { set_autocomplete_type (do_autocomplete_type_%s); return -1; }\n", t->print_id);
      printf ("  if (cur_token_len < 0) { return -1; }\n");      
      int i;
      for (i = 0; i < t->constructors_num; i++) {
        printf ("  if (cur_token_len == %d && !memcmp (cur_token, \"%s\", cur_token_len)) {\n", (int)strlen (t->constructors[i]->id), t->constructors[i]->id);
        printf ("    local_next_token ();\n");
        printf ("    if (autocomplete_constructor_%s (T) < 0) { return -1; }\n", t->constructors[i]->print_id);
        printf ("    expect_token_autocomplete (\")\", 1);\n");
        printf ("    return 0;\n");
        printf ("  }\n");
      }
      /*if (t->constructors_num == 1) {
        printf ("  if (!force) {\n");
        printf ("    if (autocomplete_constructor_%s (T) < 0) { return -1; }\n", t->constructors[0]->print_id);
        printf ("    expect_token_autocomplete (\")\", 1);\n");
        printf ("    return 0;\n");
        printf ("  }\n");
      }*/
      printf ("  return -1;\n");
      printf ("}\n");
    }
  }
}

void gen_type_fetch_ds (struct tl_type *t) {
  //int empty = is_empty (t);;  
  print_c_type_name (t->constructors[0]->result, "", 0);

  printf ("fetch_ds_type_%s (struct paramed_type *T) {\n", t->print_id);
  printf ("  assert (in_remaining () >= 4);\n");
  printf ("  int magic = fetch_int ();\n");
  printf ("  switch (magic) {\n");
  int i;
  for (i = 0; i < t->constructors_num; i++) {
     printf ("  case 0x%08x: return fetch_ds_constructor_%s (T); break;\n", t->constructors[i]->name, t->constructors[i]->print_id);
  }
  printf ("  default: assert (0); return NULL;\n");
  printf ("  }\n");
  printf ("}\n");
  print_c_type_name (t->constructors[0]->result, "", 0);
  printf ("fetch_ds_type_bare_%s (struct paramed_type *T) {\n", t->print_id);
  if (t->constructors_num > 1) {
    printf ("  int *save_in_ptr = in_ptr;\n");
  
    for (i = 0; i < t->constructors_num; i++) {
      printf ("  if (skip_constructor_%s (T) >= 0) { in_ptr = save_in_ptr; return fetch_ds_constructor_%s (T); }\n", t->constructors[i]->print_id, t->constructors[i]->print_id);
    }
  } else {
    printf ("  return fetch_ds_constructor_%s (T);\n", t->constructors[0]->print_id);
  }
  printf ("  assert (0);\n");
  printf ("  return NULL;\n");
  printf ("}\n");
}

void gen_type_free_ds (struct tl_type *t) {
  printf ("void free_ds_type_%s (", t->print_id);
  print_c_type_name (t->constructors[0]->result, "", 0);
  printf ("D, struct paramed_type *T) {\n");
  
  if (t->constructors_num > 1) {
    printf ("  switch (D->magic) {\n");
    int i;
    for (i = 0; i < t->constructors_num; i++) {
      printf ("  case 0x%08x: free_ds_constructor_%s (D, T); return; \n", t->constructors[i]->name, t->constructors[i]->print_id);
    }
    printf ("  default: assert (0);\n");
    printf ("  }\n");
  } else {
    printf ("  free_ds_constructor_%s (D, T); return; \n", t->constructors[0]->print_id);
  }
  printf ("}\n");
}

void gen_type_store_ds (struct tl_type *t) {
  int k;
  for (k = 0; k < 2; k++) {
    if (k == 0) {
      printf ("void store_ds_type_%s (", t->print_id);
    } else {
      printf ("void store_ds_type_bare_%s (", t->print_id);
    }
    print_c_type_name (t->constructors[0]->result, "", 0);
    printf ("D, struct paramed_type *T) {\n");

    if (t->constructors_num > 1) {
      if (k == 0) {
        printf ("  out_int (D->magic);\n");
      }
      printf ("  switch (D->magic) {\n");
      int i;
      for (i = 0; i < t->constructors_num; i++) {
        printf ("  case 0x%08x: store_ds_constructor_%s (D, T); return; \n", t->constructors[i]->name, t->constructors[i]->print_id);
      }
      printf ("  default: assert (0);\n");
      printf ("  }\n");
    } else {
      if (k == 0) {
        printf ("  out_int (0x%08x);\n", t->constructors[0]->name);
      }
      printf ("  store_ds_constructor_%s (D, T); return; \n", t->constructors[0]->print_id);
    }
    printf ("}\n");
  }
}

void gen_type_print_ds (struct tl_type *t) {
  int empty = is_empty (t);;
  int k;
  for (k = 0; k < 2; k++) {
    printf ("int print_ds_type_%s%s (", k ? "bare_" : "", t->print_id);
    print_c_type_name (t->constructors[0]->result, "", 0);
    printf ("DS, struct paramed_type *T) {\n");
    printf ("  int res;\n");
    if (!empty) {
      printf ("  if (multiline_output >= 2) { multiline_offset += multiline_offset_size; }\n");
      printf ("  eprintf (\" (\");\n");
    }
    if (t->constructors_num > 1) {
      printf ("  switch (DS->magic) {\n");
      int i;
      for (i = 0; i < t->constructors_num; i++) {
         printf ("  case 0x%08x: res = print_ds_constructor_%s (DS, T); break;\n", t->constructors[i]->name, t->constructors[i]->print_id);
      }
      printf ("  default: return -1;\n");
      printf ("  }\n");
    } else {
      printf ("  res = print_ds_constructor_%s (DS, T);\n", t->constructors[0]->print_id);
    }
    if (!empty) {
      printf ("  if (res >= 0) {\n");
      printf ("    if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); }\n");
      printf ("    eprintf (\" )\");\n");
      //printf ("    if (multiline_output >= 2) { printf (\"\\n\"); }\n");
      printf ("  }\n");
    }
    printf ("  return res;\n");
    printf ("}\n");
  }
}

void gen_function_store (struct tl_combinator *f) {
  printf ("struct paramed_type *store_function_%s (void) {\n", f->print_id);
  int i;
  
  int *vars = malloc0 (f->var_num * 4);;
  assert (f->var_num <= 10);

  for (i = 0; i < f->args_num; i++) if (!(f->args[i]->flags & FLAG_OPT_VAR)) {
    if (f->args[i]->flags & FLAG_EXCL) {
      assert (gen_field_store_excl (f->args[i], vars, i + 1, 1) >= 0);
    } else {
      assert (gen_field_store (f->args[i], vars, i + 1, 1, 0) >= 0);
    }
  }


  printf ("  struct paramed_type *R = \n");
  assert (gen_create (f->result, vars, 2) >= 0);
  printf (";\n");

  free (vars);
  printf ("  return paramed_type_dup (R);\n");
  printf ("}\n"); 
}

void gen_function_autocomplete (struct tl_combinator *f) {
  printf ("struct paramed_type *autocomplete_function_%s (void) {\n", f->print_id);
  int i;
  
  int *vars = malloc0 (f->var_num * 4);;
  assert (f->var_num <= 10);

  for (i = 0; i < f->args_num; i++) if (!(f->args[i]->flags & FLAG_OPT_VAR)) {
    if (f->args[i]->flags & FLAG_EXCL) {
      assert (gen_field_autocomplete_excl (f->args[i], vars, i + 1, 1) >= 0);
    } else {
      assert (gen_field_autocomplete (f->args[i], vars, i + 1, 1, 0) >= 0);
    }
  }

  printf ("  struct paramed_type *R = \n");
  assert (gen_create (f->result, vars, 2) >= 0);
  printf (";\n");

  free (vars);
  printf ("  return paramed_type_dup (R);\n");
  printf ("}\n"); 
}

void gen_type_do_autocomplete (struct tl_type *t) {
  printf ("int do_autocomplete_type_%s (const char *text, int text_len, int index, char **R) {\n", t->print_id);
  printf ("  index ++;\n");  
  int i;
  for (i = 0; i < t->constructors_num; i++) {
    printf ("  if (index == %d) { if (!strncmp (text, \"%s\", text_len)) { *R = tstrdup (\"%s\"); return index; } else { index ++; }}\n", i, t->constructors[i]->id, t->constructors[i]->id);
  }
  printf ("  *R = 0;\n");
  printf ("  return 0;\n");
  printf ("}\n");
}

struct tl_tree *read_num_var (int *var_num) {
  struct tl_tree_var_num *T = malloc0 (sizeof (*T));
  T->self.flags = 0;
  T->self.methods = &tl_pvar_num_methods;; 
  T->dif = get_int ();
  T->var_num = get_int ();
  
  if (T->var_num >= *var_num) {
    *var_num = T->var_num + 1;
  }
  assert (!(T->self.flags & FLAG_NOVAR));
  return (void *)T;
}

struct tl_tree *read_type_var (int *var_num) {
  struct tl_tree_var_type *T = malloc0 (sizeof (*T));
  T->self.methods = &tl_pvar_type_methods;
  T->var_num = get_int ();
  T->self.flags = get_int ();
  if (T->var_num >= *var_num) {  
    *var_num = T->var_num + 1;
  }
  assert (!(T->self.flags & (FLAG_NOVAR | FLAG_BARE)));
  return (void *)T;
}

struct tl_tree *read_array (int *var_num) {
  struct tl_tree_array *T = malloc0 (sizeof (*T));
  T->self.methods = &tl_parray_methods;
  T->self.flags = 0;
  T->multiplicity = read_nat_expr (var_num);
  assert (T->multiplicity);

  T->args_num = get_int ();
  assert (T->args_num >= 0 && T->args_num <= 1000);
  T->args = malloc0 (sizeof (void *) * T->args_num);

  assert (read_args_list (T->args, T->args_num, var_num) >= 0);
  T->self.flags |= FLAG_NOVAR;
  int i;
  for (i = 0; i < T->args_num; i++) {
    if (!(T->args[i]->flags & FLAG_NOVAR)) {
      T->self.flags &= ~FLAG_NOVAR;
    }
  }
  return (void *)T;
}

struct tl_tree *read_type (int *var_num) {
  struct tl_tree_type *T = malloc0 (sizeof (*T));
  T->self.methods = &tl_ptype_methods;
 
  T->type = tl_type_get_by_name (get_int ());
  assert (T->type);
  T->self.flags = get_int ();
  T->children_num = get_int ();
  assert (T->type->arity == T->children_num);
  T->children = malloc0 (sizeof (void *) * T->children_num);
  int i;
  T->self.flags |= FLAG_NOVAR;
  for (i = 0; i < T->children_num; i++) {
    int t = get_int ();
    if (t == (int)TLS_EXPR_NAT) {
#ifdef _MSC_VER
      assert ((T->type->params_types & (1i64 << i)));
#else
      assert ((T->type->params_types & (1 << i)));
#endif
      T->children[i] = read_nat_expr (var_num);
    } else if (t == (int)TLS_EXPR_TYPE) {
#ifdef _MSC_VER
      assert (!(T->type->params_types & (1i64 << i)));
#else
      assert (!(T->type->params_types & (1 << i)));
#endif
      T->children[i] = read_type_expr (var_num);
    } else {
      assert (0);
    }
    if (!TL_IS_NAT_VAR (T->children[i]) && !(T->children[i]->flags & FLAG_NOVAR)) {
      T->self.flags &= ~FLAG_NOVAR;
    }
  }
  return (void *)T;
}

struct tl_tree *read_tree (int *var_num) {
  int x = get_int ();
  if (verbosity >= 2) {
    fprintf (stderr, "read_tree: constructor = 0x%08x\n", x);
  }
  switch (x) {
  case TLS_TREE_NAT_CONST:
    return read_num_const (var_num);
  case TLS_TREE_NAT_VAR:
    return read_num_var (var_num);
  case TLS_TREE_TYPE_VAR:
    return read_type_var (var_num);
  case TLS_TREE_TYPE:
    return read_type (var_num);
  case TLS_TREE_ARRAY:
    return read_array (var_num);
  default:
    if (verbosity) {
      fprintf (stderr, "x = %d\n", x);
    }
    assert (0);
    return 0;
  }    
}

struct tl_tree *read_type_expr (int *var_num) {
  int x = get_int ();
  if (verbosity >= 2) {
    fprintf (stderr, "read_type_expr: constructor = 0x%08x\n", x);
  }
  switch (x) {
  case TLS_TYPE_VAR:
    return read_type_var (var_num);
  case TLS_TYPE_EXPR:
    return read_type (var_num);
  case TLS_ARRAY:
    return read_array (var_num);
  default:
    if (verbosity) {
      fprintf (stderr, "x = %d\n", x);
    }
    assert (0);
    return 0;
  }     
}

struct tl_tree *read_nat_expr (int *var_num) {
  int x = get_int ();
  if (verbosity >= 2) {
    fprintf (stderr, "read_nat_expr: constructor = 0x%08x\n", x);
  }
  switch (x) {
  case TLS_NAT_CONST:
    return read_num_const (var_num);
  case TLS_NAT_VAR:
    return read_num_var (var_num);
  default:
    if (verbosity) {
      fprintf (stderr, "x = %d\n", x);
    }
    assert (0);
    return 0;
  }     
}

struct tl_tree *read_expr (int *var_num) {
  int x = get_int ();
  if (verbosity >= 2) {
    fprintf (stderr, "read_nat_expr: constructor = 0x%08x\n", x);
  }
  switch (x) {
  case TLS_EXPR_NAT:
    return read_nat_expr (var_num);
  case TLS_EXPR_TYPE:
    return read_type_expr (var_num);
  default:
    if (verbosity) {
      fprintf (stderr, "x = %d\n", x);
    }
    assert (0);
    return 0;
  }
}

int read_args_list (struct arg **args, int args_num, int *var_num) {
  int i;
  for (i = 0; i < args_num; i++) {
    args[i] = malloc0 (sizeof (struct arg));
    args[i]->exist_var_num = -1;
    args[i]->exist_var_bit = 0;
    assert (get_int () == TLS_ARG_V2);
    args[i]->id = get_string ();
    args[i]->flags = get_int ();

    if (args[i]->flags & 2) {
      args[i]->flags &= ~2;
      args[i]->flags |= (1 << 20);
    }
    if (args[i]->flags & 4) {
      args[i]->flags &= ~4;
      args[i]->var_num = get_int ();
    } else {
      args[i]->var_num = -1;
    }
    
    int x = args[i]->flags & 6;
    args[i]->flags &= ~6;
    if (x & 2) { args[i]->flags |= 4; }
    if (x & 4) { args[i]->flags |= 2; }
    
    if (args[i]->var_num >= *var_num) {
      *var_num = args[i]->var_num + 1;
    }
    if (args[i]->flags & FLAG_OPT_FIELD) {
      args[i]->exist_var_num = get_int ();
      args[i]->exist_var_bit = get_int ();
    }
    args[i]->type = read_type_expr (var_num);
    assert (args[i]->type);
    
    if (args[i]->var_num < 0 && args[i]->exist_var_num < 0 && (TL_IS_NAT_VAR(args[i]->type) || (args[i]->type->flags & FLAG_NOVAR))) {
      args[i]->flags |= FLAG_NOVAR;
    }
  }
  return 1;
}

int read_combinator_args_list (struct tl_combinator *c) {
  c->args_num = get_int ();
  if (verbosity >= 2) {
    fprintf (stderr, "c->id = %s, c->args_num = %d\n", c->id, c->args_num);
  }
  assert (c->args_num >= 0 && c->args_num <= 1000);
  c->args = malloc0 (sizeof (void *) * c->args_num);
  c->var_num = 0;
  return read_args_list (c->args, c->args_num, &c->var_num);
}

int read_combinator_right (struct tl_combinator *c) {
  assert (get_int () == TLS_COMBINATOR_RIGHT_V2);
  c->result = read_type_expr (&c->var_num);
  assert (c->result);
  return 1;
}

int read_combinator_left (struct tl_combinator *c) {
  int x = get_int ();

  if (x == (int)TLS_COMBINATOR_LEFT_BUILTIN) {
    c->args_num = 0;
    c->var_num = 0;
    c->args = 0;
    return 1;
  } else if (x == TLS_COMBINATOR_LEFT) {
    return read_combinator_args_list (c);
  } else {
    assert (0);
    return -1;
  }
}

char *gen_print_id (const char *id) {
  static char s[1000];
  char *ptr = s;
  int first = 1;
  while (*id) {
    if (*id == '.') { 
      *(ptr ++) = '_';
    } else if (*id >= 'A' && *id <= 'Z') {
      if (!first && *(ptr - 1) != '_') {
        *(ptr ++) = '_';
      }
      *(ptr ++) = *id - 'A' + 'a';
    } else {
      *(ptr ++) = *id;
    }
    id ++;
    first = 0;
  }
  *ptr = 0;
  return s;
}

struct tl_combinator *read_combinators (int v) {
  struct tl_combinator *c = malloc0 (sizeof (*c));
  c->name = get_int ();
  c->id = get_string ();
  c->print_id = strdup (gen_print_id (c->id));
  assert (c->print_id);
  //char *s = c->id;
  //while (*s) { if (*s == '.') { *s = '_'; } ; s ++;}
  int x = get_int ();
  struct tl_type *t = tl_type_get_by_name (x);
  assert (t || (!x && v == 3));

  if (v == 2) {
    assert (t->extra < t->constructors_num);
    t->constructors[t->extra ++] = c;
    c->is_fun = 0;
  } else {
    assert (v == 3);
    tl_function_insert_by_name (c);
    c->is_fun = 1;
  }
  assert (read_combinator_left (c) >= 0);
  assert (read_combinator_right (c) >= 0);
  return c;
}

struct tl_type *read_types (void) {
  struct tl_type *t = malloc0 (sizeof (*t));
  t->name = get_int ();
  t->id = get_string ();
  t->print_id = strdup (gen_print_id (t->id));
  assert (t->print_id);
  
  t->constructors_num = get_int ();
  assert (t->constructors_num >= 0 && t->constructors_num <= 1000);

  t->constructors = malloc0 (sizeof (void *) * t->constructors_num);
  t->flags = get_int ();
  t->arity = get_int ();
  t->params_types = get_long (); // params_types
  t->extra = 0;
  tl_type_insert_by_name (t);
  return t;
}



char *gen_what[1000];
int gen_what_cnt;


void gen_skip_header (void) {
  printf ("#ifdef _MSC_VER\n");
  printf ("#include \"..\\auto.h\"\n");
  printf ("#else\n");
  printf ("#include \"auto.h\"\n");
  printf ("#endif\n");
  printf ("#include <assert.h>\n");

  int i, j;
  for (i = 0; i < tn; i++) {
    for (j = 0; j < tps[i]->constructors_num; j ++) {
      printf ("int skip_constructor_%s (struct paramed_type *T);\n", tps[i]->constructors[j]->print_id);
    }
  }
  for (i = 0; i < tn; i++) if (tps[i]->id[0] != '#' && strcmp (tps[i]->id, "Type")) {
    printf ("int skip_type_%s (struct paramed_type *T);\n", tps[i]->print_id);
    printf ("int skip_type_bare_%s (struct paramed_type *T);\n", tps[i]->print_id);
  }
  printf ("int skip_type_any (struct paramed_type *T);\n");
}

void gen_skip_source (void) {
  printf ("#ifdef _MSC_VER\n");
  printf ("#include \"..\\auto.h\"\n");
  printf ("#include \"auto-skip.h\"\n");
  printf ("#include \"..\\auto-static-skip.c\"\n");
  printf ("#include \"..\\mtproto-common.h\"\n");
  printf ("#else\n");
  printf ("#include \"auto.h\"\n");

  printf ("#include \"auto/auto-skip.h\"\n");
  printf ("#include \"auto-static-skip.c\"\n");
  printf ("#include \"mtproto-common.h\"\n");
  printf ("#endif\n");
  printf ("#include <assert.h>\n");

  int i, j;
  for (i = 0; i < tn; i++) {
    for (j = 0; j < tps[i]->constructors_num; j ++) {
      gen_constructor_skip (tps[i]->constructors[j]);
    }
  }
  for (i = 0; i < tn; i++) if (tps[i]->id[0] != '#' && strcmp (tps[i]->id, "Type")) {
    gen_type_skip (tps[i]);
  }
  printf ("int skip_type_any (struct paramed_type *T) {\n");
  printf ("  switch (T->type->name) {\n");
  for (i = 0; i < tn; i++) if (tps[i]->id[0] != '#' && strcmp (tps[i]->id, "Type") && tps[i]->name) {
    printf ("  case 0x%08x: return skip_type_%s (T);\n", tps[i]->name, tps[i]->print_id);
    printf ("  case 0x%08x: return skip_type_bare_%s (T);\n", ~tps[i]->name, tps[i]->print_id);
  }
  printf ("  default: return -1; }\n");
  printf ("}\n");
}

void gen_fetch_header (void) {
  printf ("#ifdef _MSC_VER\n");
  printf ("#include \"..\\auto.h\"\n");
  printf ("#else\n");
  printf ("#include \"auto.h\"\n");
  printf ("#endif\n");
  printf ("#include <assert.h>\n");
  printf ("#include <stdio.h>\n");

  printf ("struct tgl_state;\n");
  printf ("char *tglf_extf_fetch (struct tgl_state *TLS, struct paramed_type *T);\n");

  int i, j;
  for (i = 0; i < tn; i++) {
    for (j = 0; j < tps[i]->constructors_num; j ++) {
      printf ("int fetch_constructor_%s (struct paramed_type *T);\n", tps[i]->constructors[j]->print_id);
    }
  }
  for (i = 0; i < tn; i++) if (tps[i]->id[0] != '#' && strcmp (tps[i]->id, "Type")) {
    printf ("int fetch_type_%s (struct paramed_type *T);\n", tps[i]->print_id);
    printf ("int fetch_type_bare_%s (struct paramed_type *T);\n", tps[i]->print_id);
  }
  printf ("int fetch_type_any (struct paramed_type *T);\n");
}

void gen_fetch_source (void) {
  printf ("#ifdef _MSC_VER \n");
  printf ("#include \"..\\auto.h\"\n");
  printf ("#include \"auto-fetch.h\"\n");
  printf ("#include \"auto-skip.h\"\n");
  printf ("#include \"..\\auto-static-fetch.c\"\n");
  printf ("#include \"..\\mtproto-common.h\"\n");
  printf ("#else\n");
  printf ("#include \"auto.h\"\n");

  printf ("#include \"auto/auto-fetch.h\"\n");
  printf ("#include \"auto/auto-skip.h\"\n");
  printf ("#include \"auto-static-fetch.c\"\n");
  printf ("#include \"mtproto-common.h\"\n");
  printf ("#endif\n");
  printf ("#include <assert.h>\n");
  int i, j;
  for (i = 0; i < tn; i++) {
    for (j = 0; j < tps[i]->constructors_num; j ++) {
      gen_constructor_fetch (tps[i]->constructors[j]);
    }
  }
  for (i = 0; i < tn; i++) if (tps[i]->id[0] != '#' && strcmp (tps[i]->id, "Type")) {
    gen_type_fetch (tps[i]);
  }
  printf ("int fetch_type_any (struct paramed_type *T) {\n");
  printf ("  switch (T->type->name) {\n");
  for (i = 0; i < tn; i++) if (tps[i]->id[0] != '#' && strcmp (tps[i]->id, "Type") && tps[i]->name) {
    printf ("  case 0x%08x: return fetch_type_%s (T);\n", tps[i]->name, tps[i]->print_id);
    printf ("  case 0x%08x: return fetch_type_bare_%s (T);\n", ~tps[i]->name, tps[i]->print_id);
  }
  printf ("  default: return -1; }\n");
  printf ("}\n");
}

void gen_store_header (void) {
  printf ("#ifdef _MSC_VER\n");
  printf ("#include \"..\\auto.h\"\n");
  printf ("#else\n");
  printf ("#include \"auto.h\"\n");
  printf ("#endif\n");
  printf ("#include <assert.h>\n");


  printf ("struct paramed_type *tglf_extf_store (struct tgl_state *TLS, const char *data, int data_len);\n");
  printf ("int tglf_store_type (struct tgl_state *TLS, const char *work, int work_len, struct paramed_type *P);\n");

  int i, j;
  for (i = 0; i < tn; i++) {
    for (j = 0; j < tps[i]->constructors_num; j ++) {
      printf ("int store_constructor_%s (struct paramed_type *T);\n", tps[i]->constructors[j]->print_id);
    }
  }
  for (i = 0; i < tn; i++) if (tps[i]->id[0] != '#' && strcmp (tps[i]->id, "Type")) {
    printf ("int store_type_%s (struct paramed_type *T);\n", tps[i]->print_id);
    printf ("int store_type_bare_%s (struct paramed_type *T);\n", tps[i]->print_id);
  }
  for (i = 0; i < fn; i++) {
    printf ("struct paramed_type *store_function_%s (void);\n", fns[i]->print_id);
  }
  printf ("int store_type_any (struct paramed_type *T);\n");
  printf ("struct paramed_type *store_function_any (void);\n");
}

void gen_store_source (void ) {
  printf ("#ifdef _MSC_VER\n");
  printf ("#include \"..\\auto.h\"\n");
  printf ("#include \"..\\mtproto-common.h\"\n");
  printf ("#include \"auto-store.h\"\n");
  printf ("#include \"..\\auto-static-store.c\"\n");
  printf ("#else\n");
  printf ("#include \"auto.h\"\n");
  
  printf ("#include \"mtproto-common.h\"\n");
  printf ("#include \"auto/auto-store.h\"\n");
  printf ("#include \"auto-static-store.c\"\n");
  printf ("#endif\n");
  printf ("#include <assert.h>\n");

  int i, j;
  for (i = 0; i < tn; i++) {
    for (j = 0; j < tps[i]->constructors_num; j ++) {
      gen_constructor_store (tps[i]->constructors[j]);
    }
  }
  for (i = 0; i < tn; i++) if (tps[i]->id[0] != '#' && strcmp (tps[i]->id, "Type")) {
    gen_type_store (tps[i]);
  }
  for (i = 0; i < fn; i++) {
    gen_function_store (fns[i]);
  }
  printf ("int store_type_any (struct paramed_type *T) {\n");
  printf ("  switch (T->type->name) {\n");
  for (i = 0; i < tn; i++) if (tps[i]->id[0] != '#' && strcmp (tps[i]->id, "Type") && tps[i]->name) {
    printf ("  case 0x%08x: return store_type_%s (T);\n", tps[i]->name, tps[i]->print_id);
    printf ("  case 0x%08x: return store_type_bare_%s (T);\n", ~tps[i]->name, tps[i]->print_id);
  }
  printf ("  default: return -1; }\n");
  printf ("}\n");
  printf ("struct paramed_type *store_function_any (void) {\n");
  printf ("  if (cur_token_len != 1 || *cur_token != '(') { return 0; }\n");
  printf ("  local_next_token ();\n");
  printf ("  if (cur_token_len == 1 || *cur_token == '.') { \n");
  printf ("    local_next_token ();\n");
  printf ("    if (cur_token_len != 1 || *cur_token != '=') { return 0; }\n");
  printf ("    local_next_token ();\n");
  printf ("  };\n");
  printf ("  if (cur_token_len < 0) { return 0; }\n");
  for (i = 0; i < fn; i++) {
    printf ("  if (cur_token_len == %d && !memcmp (cur_token, \"%s\", cur_token_len)) {\n", (int)strlen (fns[i]->id), fns[i]->id);
    printf ("    out_int (0x%08x);\n", fns[i]->name);
    printf ("    local_next_token ();\n");
    printf ("    struct paramed_type *P = store_function_%s ();\n", fns[i]->print_id);
    printf ("    if (!P) { return 0; }\n");
    printf ("    if (cur_token_len != 1 || *cur_token != ')') { return 0; }\n");
    printf ("    local_next_token ();\n");
    printf ("    return P;\n");
    printf ("  }\n");
  }
  printf ("  return 0;\n");
  printf ("}\n");
}

void gen_autocomplete_header (void) {
  printf ("#ifdef _MSC_VER\n");
  printf ("#include \"..\\auto.h\"\n");
  printf ("#else\n");
  printf ("#include \"auto.h\"\n");
  printf ("#endif\n");
  printf ("#include <assert.h>\n");

  printf ("int tglf_extf_autocomplete (struct tgl_state *TLS, const char *text, int text_len, int index, char **R, char *data, int data_len);\n");

  int i, j;
  for (i = 0; i < tn; i++) {
    for (j = 0; j < tps[i]->constructors_num; j ++) {
      printf ("int autocomplete_constructor_%s (struct paramed_type *T);\n", tps[i]->constructors[j]->print_id);
    }
  }
  for (i = 0; i < tn; i++) if (tps[i]->id[0] != '#' && strcmp (tps[i]->id, "Type")) {
    printf ("int autocomplete_type_%s (struct paramed_type *T);\n", tps[i]->print_id);
    printf ("int do_autocomplete_type_%s (const char *text, int len, int index, char **R);\n", tps[i]->print_id);
    printf ("int autocomplete_type_bare_%s (struct paramed_type *T);\n", tps[i]->print_id);
  }
  printf ("int autocomplete_type_any (struct paramed_type *T);\n");
  printf ("struct paramed_type *autocomplete_function_any (void);\n");
}

void gen_autocomplete_source (void) {
  printf ("#ifdef _MSC_VER\n");
  printf ("#include \"..\\auto.h\"\n");
  printf ("#include \"..\\mtproto-common.h\"\n");
  printf ("#include \"auto-autocomplete.h\"\n");
  printf ("#include \"..\\auto-static-autocomplete.c\"\n");
  printf ("#else\n");
  printf ("#include \"auto.h\"\n");
  
  printf ("#include \"mtproto-common.h\"\n");
  printf ("#include \"auto/auto-autocomplete.h\"\n");
  printf ("#include \"auto-static-autocomplete.c\"\n");
  printf ("#endif\n");
  printf ("#include <assert.h>\n");

  int i, j;
  for (i = 0; i < tn; i++) {
    for (j = 0; j < tps[i]->constructors_num; j ++) {
      gen_constructor_autocomplete (tps[i]->constructors[j]);
    }
  }
  for (i = 0; i < tn; i++) if (tps[i]->id[0] != '#' && strcmp (tps[i]->id, "Type")) {
    gen_type_autocomplete (tps[i]);
    gen_type_do_autocomplete (tps[i]);
  }
  for (i = 0; i < fn; i++) {
    gen_function_autocomplete (fns[i]);
  }
  printf ("int autocomplete_type_any (struct paramed_type *T) {\n");
  printf ("  switch (T->type->name) {\n");
  for (i = 0; i < tn; i++) if (tps[i]->id[0] != '#' && strcmp (tps[i]->id, "Type") && tps[i]->name) {
    printf ("  case 0x%08x: return autocomplete_type_%s (T);\n", tps[i]->name, tps[i]->print_id);
    printf ("  case 0x%08x: return autocomplete_type_bare_%s (T);\n", ~tps[i]->name, tps[i]->print_id);
  }
  printf ("  default: return -1; }\n");
  printf ("}\n");
  printf ("int do_autocomplete_function (const char *text, int text_len, int index, char **R) {\n");
  printf ("  index ++;\n");  
  for (i = 0; i < fn; i++) {
    printf ("  if (index == %d) { if (!strncmp (text, \"%s\", text_len)) { *R = tstrdup (\"%s\"); return index; } else { index ++; }}\n", i, fns[i]->id, fns[i]->id);
  }
  printf ("  *R = 0;\n");
  printf ("  return 0;\n");
  printf ("}\n");
  printf ("struct paramed_type *autocomplete_function_any (void) {\n");
  printf ("  expect_token_ptr_autocomplete (\"(\", 1);\n");
  printf ("  if (cur_token_len == -3) { set_autocomplete_type (do_autocomplete_function); }\n");
  printf ("  if (cur_token_len < 0) { return 0; }\n");
  for (i = 0; i < fn; i++) {
    printf ("  if (cur_token_len == %d && !memcmp (cur_token, \"%s\", cur_token_len)) {\n", (int)strlen (fns[i]->id), fns[i]->id);
    printf ("    local_next_token ();\n");
    printf ("    struct paramed_type *P = autocomplete_function_%s ();\n", fns[i]->print_id);
    printf ("    if (!P) { return 0; }\n");
    printf ("    expect_token_ptr_autocomplete (\")\", 1);\n");
    printf ("    return P;\n");
    printf ("  }\n");
  }
  printf ("  return 0;\n");
  printf ("}\n");
}

void gen_types_header (void) {
  printf ("#ifndef __AUTO_TYPES_H__\n");
  printf ("#define __AUTO_TYPES_H__\n");
  printf ("#ifdef _MSC_VER\n");
  printf ("#include \"..\\auto.h\"\n");
  printf ("#else\n");
  printf ("#include \"auto.h\"\n");
  printf ("#endif\n");
  int i;
  for (i = 0; i < tn; i++) if (tps[i]->id[0] != '#' && strcmp (tps[i]->id, "Type")) {
    printf ("extern struct tl_type_descr tl_type_%s;\n", tps[i]->print_id);
    printf ("extern struct tl_type_descr tl_type_bare_%s;\n", tps[i]->print_id);
  }
  for (i = 0; i < tn; i++) if (tps[i]->id[0] != '#' && strcmp (tps[i]->id, "Type")) {
    printf ("struct tl_ds_%s {\n", tps[i]->print_id);
#if !defined(__STDC__) ||  __STDC_VERSION__ < 199901L
    if (tps[i]->constructors_num == 1 && tps[i]->constructors[0]->args_num == 0 &&
        !(!strcmp (tps[i]->id, "String") || !strcmp (tps[i]->id, "Bytes"))) {
      printf ("  int : 0;\n");
      printf ("};\n");
      continue;
    }
#endif

    if (!strcmp (tps[i]->id, "String") || !strcmp (tps[i]->id, "Bytes")) {
      printf ("  int len;\n");
      printf ("  char *data;\n");
      printf ("};\n");
      continue;
    }
    int j;
    if (tps[i]->constructors_num > 1) {
      printf ("  unsigned magic;\n");
    }
    for (j = 0; j < tps[i]->constructors_num; j++) {
      struct tl_combinator *c = tps[i]->constructors[j];
      int k;
      for (k = 0; k < c->args_num; k++) {
        if ((c->args[k]->flags & FLAG_OPT_VAR)) { continue; }
        if (c->args[k]->id && strlen (c->args[k]->id) && j > 0) {
          int l;
          int ok = 1;
          for (l = 0; l < j && ok; l++) {
            int m;
            struct tl_combinator *d = tps[i]->constructors[l];
            for (m = 0; m < d->args_num && ok; m++) {
              if (d->args[m]->id && !strcmp (d->args[m]->id, c->args[k]->id)) {
                ok = 0;
              }
            }
          }
          if (!ok) { continue; }
        }
    
        printf ("  ");
        print_c_type_name (c->args[k]->type, "  ", 1);

        if (!c->args[k]->id || !strlen (c->args[k]->id)) {          
          assert (!j);
          
          printf ("f%d;\n", k);
        } else {
          printf ("%s;\n", c->args[k]->id);
        }
      }
    }
    printf ("};\n");
  }
  printf ("#endif\n");
}

void gen_types_source (void) {
  printf ("#ifdef _MSC_VER\n");
  printf ("#include \"..\\auto.h\"\n");
  printf ("#else\n");
  printf ("#include \"auto.h\"\n");
  printf ("#endif\n");
  int i;
  for (i = 0; i < tn; i++) if (tps[i]->id[0] != '#' && strcmp (tps[i]->id, "Type")) {
    printf ("struct tl_type_descr tl_type_%s = {\n", tps[i]->print_id);
    printf ("  .name = 0x%08x,\n", tps[i]->name);
    printf ("  .id = \"%s\"\n,", tps[i]->id);
    printf ("  .params_num = %d,\n", tps[i]->arity);
    printf ("  .params_types = %"_PRINTF_INT64_"d\n", tps[i]->params_types);
    printf ("};\n");
    printf ("struct tl_type_descr tl_type_bare_%s = {\n", tps[i]->print_id);
    printf ("  .name = 0x%08x,\n", ~tps[i]->name);
    printf ("  .id = \"Bare_%s\",\n", tps[i]->id);
    printf ("  .params_num = %d,\n", tps[i]->arity);
    printf ("  .params_types = %"_PRINTF_INT64_"d\n", tps[i]->params_types);
    printf ("};\n");
  }
}

void gen_fetch_ds_source (void) {
  printf ("#ifdef _MSC_VER\n");
  printf ("#include \"..\\auto.h\"\n");
  printf ("#include \"auto-fetch-ds.h\"\n");
  printf ("#include \"auto-skip.h\"\n");
  printf ("#include \"auto-types.h\"\n");
  printf ("#include \"..\\auto-static-fetch-ds.c\"\n");
  printf ("#include \"..\\mtproto-common.h\"\n");
  printf ("#else\n");
  printf ("#include \"auto.h\"\n");

  printf ("#include \"auto/auto-fetch-ds.h\"\n");
  printf ("#include \"auto/auto-skip.h\"\n");
  printf ("#include \"auto/auto-types.h\"\n");
  printf ("#include \"auto-static-fetch-ds.c\"\n");
  printf ("#include \"mtproto-common.h\"\n");
  printf ("#endif\n");
  printf ("#include <assert.h>\n");
  int i, j;
  for (i = 0; i < tn; i++) {
    for (j = 0; j < tps[i]->constructors_num; j ++) {
      gen_constructor_fetch_ds (tps[i]->constructors[j]);
    }
  }
  for (i = 0; i < tn; i++) if (tps[i]->id[0] != '#' && strcmp (tps[i]->id, "Type")) {
    gen_type_fetch_ds (tps[i]);
  }
  printf ("void *fetch_ds_type_any (struct paramed_type *T) {\n");
  printf ("  switch (T->type->name) {\n");
  for (i = 0; i < tn; i++) if (tps[i]->id[0] != '#' && strcmp (tps[i]->id, "Type") && tps[i]->name) {
    printf ("  case 0x%08x: return fetch_ds_type_%s (T);\n", tps[i]->name, tps[i]->print_id);
    printf ("  case 0x%08x: return fetch_ds_type_bare_%s (T);\n", ~tps[i]->name, tps[i]->print_id);
  }
  printf ("  default: return NULL; }\n");
  printf ("}\n");
}

void gen_fetch_ds_header (void) {
  printf ("#ifdef _MSC_VER\n");
  printf ("#include \"..\\auto.h\"\n");
  printf ("#else\n");
  printf ("#include \"auto.h\"\n");
  printf ("#endif\n");
  printf ("#include <assert.h>\n");
  printf ("#include <stdio.h>\n");

  printf ("struct tgl_state;\n");
  //printf ("char *tglf_extf_fetch (struct tgl_state *TLS, struct paramed_type *T);\n");

  int i, j;
  for (i = 0; i < tn; i++) {
    for (j = 0; j < tps[i]->constructors_num; j ++) {
      print_c_type_name (tps[i]->constructors[j]->result, "", 0);
      printf ("fetch_ds_constructor_%s (struct paramed_type *T);\n", tps[i]->constructors[j]->print_id);
    }
  }
  for (i = 0; i < tn; i++) if (tps[i]->id[0] != '#' && strcmp (tps[i]->id, "Type")) {
    print_c_type_name (tps[i]->constructors[0]->result, "", 0);
    printf ("fetch_ds_type_%s (struct paramed_type *T);\n", tps[i]->print_id);
    print_c_type_name (tps[i]->constructors[0]->result, "", 0);
    printf ("fetch_ds_type_bare_%s (struct paramed_type *T);\n", tps[i]->print_id);
  }
  printf ("void *fetch_ds_type_any (struct paramed_type *T);\n");
}

void gen_free_ds_source (void) {
  printf ("#ifdef _MSC_VER\n");
  printf ("#include \"..\\auto.h\"\n");
  printf ("#include \"auto-free-ds.h\"\n");
  printf ("#include \"auto-skip.h\"\n");
  printf ("#include \"auto-types.h\"\n");
  printf ("#include \"..\\auto-static-free-ds.c\"\n");
  printf ("#include \"..\\mtproto-common.h\"\n");
  printf ("#else\n");
  printf ("#include \"auto.h\"\n");

  printf ("#include \"auto/auto-free-ds.h\"\n");
  printf ("#include \"auto/auto-skip.h\"\n");
  printf ("#include \"auto/auto-types.h\"\n");
  printf ("#include \"auto-static-free-ds.c\"\n");
  printf ("#include \"mtproto-common.h\"\n");
  printf ("#endif\n");
  printf ("#include <assert.h>\n");
  int i, j;
  for (i = 0; i < tn; i++) {
    for (j = 0; j < tps[i]->constructors_num; j ++) {
      gen_constructor_free_ds (tps[i]->constructors[j]);
    }
  }
  for (i = 0; i < tn; i++) if (tps[i]->id[0] != '#' && strcmp (tps[i]->id, "Type")) {
    gen_type_free_ds (tps[i]);
  }
  printf ("void free_ds_type_any (void *D, struct paramed_type *T) {\n");
  printf ("  switch (T->type->name) {\n");
  for (i = 0; i < tn; i++) if (tps[i]->id[0] != '#' && strcmp (tps[i]->id, "Type") && tps[i]->name) {
    printf ("  case 0x%08x: free_ds_type_%s (D, T); return;\n", tps[i]->name, tps[i]->print_id);
    printf ("  case 0x%08x: free_ds_type_%s (D, T); return;\n", ~tps[i]->name, tps[i]->print_id);
  }
  printf ("  default: return; }\n");
  printf ("}\n");
}

void gen_free_ds_header (void) {
  printf ("#ifdef _MSC_VER\n");
  printf ("#include \"..\\auto.h\"\n");
  printf ("#include \"auto-types.h\"\n");
  printf ("#else\n");
  printf ("#include \"auto.h\"\n");
  printf ("#include \"auto/auto-types.h\"\n");
  printf ("#endif\n");
  printf ("#include <assert.h>\n");
  printf ("#include <stdio.h>\n");

  printf ("struct tgl_state;\n");
  //printf ("char *tglf_extf_fetch (struct tgl_state *TLS, struct paramed_type *T);\n");

  int i, j;
  for (i = 0; i < tn; i++) {
    for (j = 0; j < tps[i]->constructors_num; j ++) {
      printf ("void free_ds_constructor_%s (", tps[i]->constructors[j]->print_id); 
      print_c_type_name (tps[i]->constructors[j]->result, "", 0);
      printf ("D, struct paramed_type *T);\n");
    }
  }
  for (i = 0; i < tn; i++) if (tps[i]->id[0] != '#' && strcmp (tps[i]->id, "Type")) {
    printf ("void free_ds_type_%s (", tps[i]->print_id);
    print_c_type_name (tps[i]->constructors[0]->result, "", 0);
    printf ("D, struct paramed_type *T);\n");
  }
  printf ("void free_ds_type_any (void *D, struct paramed_type *T);\n");
}

void gen_store_ds_source (void) {
  printf ("#ifdef _MSC_VER\n");
  printf ("#include \"..\\auto.h\"\n");
  printf ("#include \"auto-store-ds.h\"\n");
  printf ("#include \"auto-skip.h\"\n");
  printf ("#include \"auto-types.h\"\n");
  printf ("#include \"..\\auto-static-store-ds.c\"\n");
  printf ("#include \"..\\mtproto-common.h\"\n");
  printf ("#else\n");
  printf ("#include \"auto.h\"\n");

  printf ("#include \"auto/auto-store-ds.h\"\n");
  printf ("#include \"auto/auto-skip.h\"\n");
  printf ("#include \"auto/auto-types.h\"\n");
  printf ("#include \"auto-static-store-ds.c\"\n");
  printf ("#include \"mtproto-common.h\"\n");
  printf ("#endif\n");
  printf ("#include <assert.h>\n");
  int i, j;
  for (i = 0; i < tn; i++) {
    for (j = 0; j < tps[i]->constructors_num; j ++) {
      gen_constructor_store_ds (tps[i]->constructors[j]);
    }
  }
  for (i = 0; i < tn; i++) if (tps[i]->id[0] != '#' && strcmp (tps[i]->id, "Type")) {
    gen_type_store_ds (tps[i]);
  }
  printf ("void store_ds_type_any (void *D, struct paramed_type *T) {\n");
  printf ("  switch (T->type->name) {\n");
  for (i = 0; i < tn; i++) if (tps[i]->id[0] != '#' && strcmp (tps[i]->id, "Type") && tps[i]->name) {
    printf ("  case 0x%08x: store_ds_type_%s (D, T); return;\n", tps[i]->name, tps[i]->print_id);
    printf ("  case 0x%08x: store_ds_type_bare_%s (D, T); return;\n", ~tps[i]->name, tps[i]->print_id);
  }
  printf ("  default: return; }\n");
  printf ("}\n");
}

void gen_store_ds_header (void) {
  printf ("#ifdef _MSC_VER\n");
  printf ("#include \"..\\auto.h\"\n");
  printf ("#include \"auto-types.h\"\n");
  printf ("#else\n");
  printf ("#include \"auto.h\"\n");
  printf ("#include \"auto/auto-types.h\"\n");
  printf ("#endif\n");
  printf ("#include <assert.h>\n");
  printf ("#include <stdio.h>\n");

  printf ("struct tgl_state;\n");
  //printf ("char *tglf_extf_fetch (struct tgl_state *TLS, struct paramed_type *T);\n");

  int i, j;
  for (i = 0; i < tn; i++) {
    for (j = 0; j < tps[i]->constructors_num; j ++) {
      printf ("void store_ds_constructor_%s (", tps[i]->constructors[j]->print_id); 
      print_c_type_name (tps[i]->constructors[j]->result, "", 0);
      printf ("D, struct paramed_type *T);\n");
    }
  }
  for (i = 0; i < tn; i++) if (tps[i]->id[0] != '#' && strcmp (tps[i]->id, "Type")) {
    printf ("void store_ds_type_%s (", tps[i]->print_id);
    print_c_type_name (tps[i]->constructors[0]->result, "", 0);
    printf ("D, struct paramed_type *T);\n");
    printf ("void store_ds_type_bare_%s (", tps[i]->print_id);
    print_c_type_name (tps[i]->constructors[0]->result, "", 0);
    printf ("D, struct paramed_type *T);\n");
  }
  printf ("void store_ds_type_any (void *D, struct paramed_type *T);\n");
}

void gen_print_ds_header (void) {
  printf ("#ifdef _MSC_VER\n");
  printf ("#include \"..\\auto.h\"\n");
  printf ("#include \"auto-types.h\"\n");
  printf ("#else\n");
  printf ("#include \"auto.h\"\n");
  printf ("#include \"auto-types.h\"\n");
  printf ("#endif\n");
  printf ("#include <assert.h>\n");
  printf ("#include <stdio.h>\n");

  printf ("struct tgl_state;\n");
  printf ("char *tglf_extf_print_ds (struct tgl_state *TLS, void *DS, struct paramed_type *T);\n");

  int i, j;
  for (i = 0; i < tn; i++) {
    for (j = 0; j < tps[i]->constructors_num; j ++) {
      printf ("int print_ds_constructor_%s (", tps[i]->constructors[j]->print_id);
      print_c_type_name (tps[i]->constructors[j]->result, "", 0);
      printf ("DS, struct paramed_type *T);\n");
    }
  }
  for (i = 0; i < tn; i++) if (tps[i]->id[0] != '#' && strcmp (tps[i]->id, "Type")) {
    printf ("int print_ds_type_%s (", tps[i]->print_id);
    print_c_type_name (tps[i]->constructors[0]->result, "", 0);
    printf ("DS, struct paramed_type *T);\n");
    printf ("int print_ds_type_bare_%s (", tps[i]->print_id);
    print_c_type_name (tps[i]->constructors[0]->result, "", 0);
    printf ("DS, struct paramed_type *T);\n");
  }
  printf ("int print_ds_type_any (void *DS, struct paramed_type *T);\n");
}

void gen_print_ds_source (void) {
  printf ("#ifdef _MSC_VER\n");
  printf ("#include \"..\\auto.h\"\n");
  printf ("#include \"auto-print-ds.h\"\n");
  printf ("#include \"auto-skip.h\"\n");
  printf ("#include \"..\\auto-static-print-ds.c\"\n");
  printf ("#include \"..\\mtproto-common.h\"\n");
  printf ("#else\n");
  printf ("#include \"auto.h\"\n");

  printf ("#include \"auto/auto-print-ds.h\"\n");
  printf ("#include \"auto/auto-skip.h\"\n");
  printf ("#include \"auto-static-print-ds.c\"\n");
  printf ("#include \"mtproto-common.h\"\n");
  printf ("#endif\n");
  printf ("#include <assert.h>\n");
  int i, j;
  for (i = 0; i < tn; i++) {
    for (j = 0; j < tps[i]->constructors_num; j ++) {
      gen_constructor_print_ds (tps[i]->constructors[j]);
    }
  }
  for (i = 0; i < tn; i++) if (tps[i]->id[0] != '#' && strcmp (tps[i]->id, "Type")) {
    gen_type_print_ds (tps[i]);
  }
  printf ("int print_ds_type_any (void *DS, struct paramed_type *T) {\n");
  printf ("  switch (T->type->name) {\n");
  for (i = 0; i < tn; i++) if (tps[i]->id[0] != '#' && strcmp (tps[i]->id, "Type") && tps[i]->name) {
    printf ("  case 0x%08x: return print_ds_type_%s (DS, T);\n", tps[i]->name, tps[i]->print_id);
    printf ("  case 0x%08x: return print_ds_type_bare_%s (DS, T);\n", ~tps[i]->name, tps[i]->print_id);
  }
  printf ("  default: return -1; }\n");
  printf ("}\n");
}

int parse_tlo_file (void) {
  buf_end = buf_ptr + (buf_size / 4);
  assert (get_int () == TLS_SCHEMA_V2);

  get_int (); // version
  get_int (); // date
  
  tn = 0;
  fn = 0;
  cn = 0;
  int i;

  tn = get_int ();
  assert (tn >= 0 && tn < 10000);
  tps = malloc0 (sizeof (void *) * tn);
  
  if (verbosity >= 2) {
    fprintf (stderr, "Found %d types\n", tn);
  }

  for (i = 0; i < tn; i++) {
    assert (get_int () == TLS_TYPE);
    tps[i] = read_types ();
    assert (tps[i]);
  }

  cn = get_int ();  
  assert (cn >= 0);

  if (verbosity >= 2) {
    fprintf (stderr, "Found %d constructors\n", cn);
  }

  for (i = 0; i < cn; i++) {
    assert (get_int () == TLS_COMBINATOR);
    assert (read_combinators (2));
  }
  
  fn = get_int ();
  assert (fn >= 0 && fn < 10000);
  
  fns = malloc0 (sizeof (void *) * fn);
  
  if (verbosity >= 2) {
    fprintf (stderr, "Found %d functions\n", fn);
  }

  for (i = 0; i < fn; i++) {
    assert (get_int () == TLS_COMBINATOR);
    fns[i] = read_combinators (3);
    assert (fns[i]);
  }

  assert (buf_ptr == buf_end);
 

  int j;
  for (i = 0; i < tn; i++) if (tps[i]->id[0] != '#' && strcmp (tps[i]->id, "Type")) {
    tps[i]->name = 0;
    for (j = 0; j < tps[i]->constructors_num; j ++) {
      tps[i]->name ^= tps[i]->constructors[j]->name;
    }
  }
 
  
  for (i = 0; i < gen_what_cnt; i++) {
    if (!strcmp (gen_what[i], "fetch")) {
      gen_fetch_source ();
    } else if (!strcmp (gen_what[i], "fetch-header")) {
      gen_fetch_header ();
    } else if (!strcmp (gen_what[i], "skip")) {
      gen_skip_source ();
    } else if (!strcmp (gen_what[i], "skip-header")) {
      gen_skip_header ();
    } else if (!strcmp (gen_what[i], "store")) {
      gen_store_source ();
    } else if (!strcmp (gen_what[i], "store-header")) {
      gen_store_header ();
    } else if (!strcmp (gen_what[i], "autocomplete")) {
      gen_autocomplete_source ();
    } else if (!strcmp (gen_what[i], "autocomplete-header")) {
      gen_autocomplete_header ();
    } else if (!strcmp (gen_what[i], "types")) {
      gen_types_source ();
    } else if (!strcmp (gen_what[i], "types-header")) {
      gen_types_header ();
    } else if (!strcmp (gen_what[i], "fetch-ds")) {
      gen_fetch_ds_source ();
    } else if (!strcmp (gen_what[i], "fetch-ds-header")) {
      gen_fetch_ds_header ();
    } else if (!strcmp (gen_what[i], "free-ds")) {
      gen_free_ds_source ();
    } else if (!strcmp (gen_what[i], "free-ds-header")) {
      gen_free_ds_header ();
    } else if (!strcmp (gen_what[i], "store-ds")) {
      gen_store_ds_source ();
    } else if (!strcmp (gen_what[i], "store-ds-header")) {
      gen_store_ds_header ();
    } else if (!strcmp (gen_what[i], "print-ds")) {
      gen_print_ds_source ();
    } else if (!strcmp (gen_what[i], "print-ds-header")) {
      gen_print_ds_header ();
    } else {
      assert (0);
    }
  }

  
  return 0;
}

void usage (void) {
  printf ("usage: generate [-v] [-h] <tlo-file>\n"
       );
  exit (2);
}

#ifndef _MSC_VER
void logprintf (const char *format, ...) __attribute__ ((format (printf, 1, 2)));
void logprintf (const char *format __attribute__ ((unused)), ...) {
}
#endif
/*
void hexdump (int *in_ptr, int *in_end) {
  int *ptr = in_ptr;
  while (ptr < in_end) { printf (" %08x", *(ptr ++)); }
  printf ("\n");
}*/

#ifdef HAVE_EXECINFO_H
void print_backtrace (void) {
  void *buffer[255];
  const int calls = backtrace (buffer, sizeof (buffer) / sizeof (void *));
  backtrace_symbols_fd (buffer, calls, 1);
}
#else
void print_backtrace (void) {
  if (write (1, "No libexec. Backtrace disabled\n", 32) < 0) {
    // Sad thing
  }
}
#endif

void sig_segv_handler (int signum __attribute__ ((unused))) {
  if (write (1, "SIGSEGV received\n", 18) < 0) { 
    // Sad thing
  }
  print_backtrace ();
  exit (EXIT_FAILURE);
}

void sig_abrt_handler (int signum __attribute__ ((unused))) {
  if (write (1, "SIGABRT received\n", 18) < 0) { 
    // Sad thing
  }
  print_backtrace ();
  exit (EXIT_FAILURE);
}

int main (int argc, char **argv) {
  signal (SIGSEGV, sig_segv_handler);
  signal (SIGABRT, sig_abrt_handler);
  int i;
  while ((i = getopt (argc, argv, "vhHg:")) != -1) {
    switch (i) {
    case 'h':
      usage ();
      return 2;
    case 'v':
      verbosity++;
      break;
    case 'H':
      header ++;
      break;
    case 'g':
      assert (gen_what_cnt < 1000);
      gen_what[gen_what_cnt ++] = optarg;
      break;
    }
  }

  if (argc != optind + 1) {
    usage ();
  }

#if defined(_MSC_VER) && _MSC_VER >= 1400
  int fd = 0;
  errno_t err = _sopen_s(&fd, argv[optind], _O_RDONLY | _O_BINARY, _SH_DENYNO, _S_IREAD | _S_IWRITE);
  if(err != 0) {
    char errnoStr[256] = { 0 };
    strerror_s (errnoStr, 256, err);
    fprintf (stderr, "Can not open file '%s'. Error %s\n", argv[optind], errnoStr);
#elif defined(WIN32) || defined(_WIN32)
  int fd = open (argv[optind], O_RDONLY | O_BINARY);
  if (fd < 0) {
    fprintf (stderr, "Can not open file '%s'. Error %s\n", argv[optind], strerror (errno));
#else
  int fd = open (argv[optind], O_RDONLY);
  if (fd < 0) {
    fprintf (stderr, "Can not open file '%s'. Error %m\n", argv[optind]);
#endif
    exit (1);
  }
  buf_size = read (fd, buf, (1 << 20));
  if (fd == (1 << 20)) {
    fprintf (stderr, "Too big tlo file\n");
    exit (2);
  }
  return parse_tlo_file ();
}
