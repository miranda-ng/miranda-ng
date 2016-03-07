#define IN_AUTOCOMPLETE_H
#include "auto-static-store.c"
#undef IN_AUTOCOMPLETE_H

static int autocomplete_mode;
static char *autocomplete_string;
static int (*autocomplete_fun)(const char *, int, int, char **);

static void set_autocomplete_string (const char *s) {
  if (autocomplete_string) { free (autocomplete_string); }
  autocomplete_string = strdup (s);
  assert (autocomplete_string);
  autocomplete_mode = 1;
}

static void set_autocomplete_type (int (*f)(const char *, int, int, char **)) {
  autocomplete_fun = f;
  autocomplete_mode = 2;
}

#define MAX_FVARS 100
static struct paramed_type *fvars[MAX_FVARS];
static int fvars_pos;

static void add_var_to_be_freed (struct paramed_type *P) {
  assert (fvars_pos < MAX_FVARS);
  fvars[fvars_pos ++] = P;
}

static void free_vars_to_be_freed (void) {
  int i;
  for (i = 0; i < fvars_pos; i++) {
    tgl_paramed_type_free (fvars[i]);
  }
  fvars_pos = 0;
}

int tglf_extf_autocomplete (struct tgl_state *TLS, const char *text, int text_len, int index, char **R, char *data, int data_len) {
  if (index == -1) {
    buffer_pos = data;
    buffer_end = data + data_len;
    autocomplete_mode = 0;
    local_next_token ();
    struct paramed_type *P = autocomplete_function_any ();
    free_vars_to_be_freed ();
    if (P) { tgl_paramed_type_free (P); }
  }
  if (autocomplete_mode == 0) { return -1; }
  int len = strlen (text);
  if (autocomplete_mode == 1) {
    if (index >= 0) { return -1; }
    index = 0;
    if (!strncmp (text, autocomplete_string, len)) {
      *R = strdup (autocomplete_string);
      assert (*R);
      return index;
    } else {
      return -1;
    }
  } else {
    return autocomplete_fun (text, len, index, R);
  }
}
