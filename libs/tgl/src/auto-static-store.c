static int cur_token_len;
static char *cur_token;
static int cur_token_real_len;
static int cur_token_quoted;

#define expect_token(token,len) \
  if (len != cur_token_len || memcmp (cur_token, token, cur_token_len)) { return -1; } \
  local_next_token ();

#define expect_token_ptr(token,len) \
  if (len != cur_token_len || memcmp (cur_token, token, cur_token_len)) { return 0; } \
  local_next_token ();

#define expect_token_autocomplete(token,len) \
  if (cur_token_len == -3 && len >= cur_token_real_len && !memcmp (cur_token, token, cur_token_real_len)) { set_autocomplete_string (token); return -1; }\
  if (len != cur_token_len || memcmp (cur_token, token, cur_token_len)) { return -1; } \
  local_next_token ();

#define expect_token_ptr_autocomplete(token,len) \
  if (cur_token_len == -3 && len >= cur_token_real_len && !memcmp (cur_token, token, cur_token_real_len)) { set_autocomplete_string (token); return 0; }\
  if (len != cur_token_len || memcmp (cur_token, token, cur_token_len)) { return 0; } \
  local_next_token ();

static int is_int (void) {
  if (cur_token_len <= 0) { return 0; }
  char c = cur_token[cur_token_len];
  cur_token[cur_token_len] = 0;
  char *p = 0;

  if (strtoll (cur_token, &p, 10)) {}
  cur_token[cur_token_len] = c;

  return p == cur_token + cur_token_len;
}

static long long get_int (void) {
  if (cur_token_len <= 0) { return 0; }
  char c = cur_token[cur_token_len];
  cur_token[cur_token_len] = 0;
  char *p = 0;

  long long val = strtoll (cur_token, &p, 0);
  cur_token[cur_token_len] = c;

  return val;
}

static int is_double (void) {
  if (cur_token_len <= 0) { return 0; }
  char c = cur_token[cur_token_len];
  cur_token[cur_token_len] = 0;
  char *p = 0;

  if (strtod (cur_token, &p)) {}
  cur_token[cur_token_len] = c;

  return p == cur_token + cur_token_len;
}

#ifndef IN_AUTOCOMPLETE_H
static double get_double (void) {
  if (cur_token_len <= 0) { return 0; }
  char c = cur_token[cur_token_len];
  cur_token[cur_token_len] = 0;
  char *p = 0;

  double val = strtod (cur_token, &p);
  cur_token[cur_token_len] = c;

  return val;
}
#endif

static char *buffer_pos, *buffer_end;

static int is_wspc (char c) {
  return c <= 32 && c > 0;
}

static void skip_wspc (void) {
  while (buffer_pos < buffer_end && is_wspc (*buffer_pos)) {
    buffer_pos ++;
  }
}

static int is_letter (char c) {
  return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9') || c == '.' || c == '_' || c == '-';
}


static char exp_buffer[1 << 25];;
static int exp_buffer_pos;

static inline int is_hex (char c) {
  return (c >= '0' && c <= '9') || (c >= 'a' && c <= 'f');
}

static inline int hex2dec (char c) {
  if (c >= '0' && c <= '9') { return c - '0'; }
  else { return c - 'a' + 10; }
}

static void expand_backslashed (char *s, int len) {
  int backslashed = 0;
  exp_buffer_pos = 0;
  int i = 0;
  while (i < len) {
    assert (i + 3 <= (1 << 25));
    if (backslashed) {
      backslashed = 0;
      switch (s[i ++]) {
      case 'n':
        exp_buffer[exp_buffer_pos ++] = '\n';
        break;
      case 'r':
        exp_buffer[exp_buffer_pos ++] = '\r';
        break;
      case 't':
        exp_buffer[exp_buffer_pos ++] = '\t';
        break;
      case 'b':
        exp_buffer[exp_buffer_pos ++] = '\b';
        break;
      case 'a':
        exp_buffer[exp_buffer_pos ++] = '\a';
        break;
      case '\\':
        exp_buffer[exp_buffer_pos ++] = '\\';
        break;
      case 'x':
        if (i + 2 > len || !is_hex (s[i]) || !is_hex (s[i + 1])) {
          exp_buffer_pos = -1;
          return;
        }
        exp_buffer[exp_buffer_pos ++] = hex2dec (s[i]) * 16 + hex2dec (s[i + 1]);
        i += 2;
        break;
      default:
        break;
      }
    } else {
      if (s[i] == '\\') { 
        backslashed = 1; 
        i ++;
      } else {
        exp_buffer[exp_buffer_pos ++] = s[i ++];
      }
    }
  }
}

static void local_next_token (void) {
  skip_wspc ();
  cur_token_quoted = 0;
  if (buffer_pos >= buffer_end) {
    cur_token_len = -3;
    cur_token_real_len = 0;
    return;
  }
  char c = *buffer_pos;
  if (is_letter (c)) {
    cur_token = buffer_pos;
    while (buffer_pos < buffer_end && is_letter (*buffer_pos)) {
      buffer_pos ++;
    }
    if (buffer_pos < buffer_end) {
      cur_token_len = buffer_pos - cur_token;
    } else {
      cur_token_real_len = buffer_pos - cur_token;
      cur_token_len = -3;
    }
    return;
  } else if (c == '"') {
    cur_token_quoted = 1;
    cur_token = buffer_pos ++;
    int backslashed = 0;
    while (buffer_pos < buffer_end && (*buffer_pos != '"' || backslashed)) {
      if (*buffer_pos == '\\') {
        backslashed ^= 1;
      } else {
        backslashed = 0;
      }
      buffer_pos ++;
    }
    if (*buffer_pos == '"') {
      buffer_pos ++;
      expand_backslashed (cur_token + 1, buffer_pos - cur_token - 2);
      if (exp_buffer_pos < 0) {
        cur_token_len = -2;
      } else {
        cur_token_len = exp_buffer_pos;
        cur_token = exp_buffer;
      }
    } else {
      cur_token_len = -2;
    }
    return;
  } else {
    if (c) {
      cur_token = buffer_pos ++;
      cur_token_len = 1;
    } else {
      cur_token_len = -3;
      cur_token_real_len = 0;
    }
  }
}

static struct paramed_type *paramed_type_dup (struct paramed_type *P) {
  if (ODDP (P)) { return P; }
  struct paramed_type *R = malloc (sizeof (*R));
  assert (R);
  R->type = malloc (sizeof (*R->type));
  assert (R->type);
  memcpy (R->type, P->type, sizeof (*P->type)); 
  R->type->id = strdup (P->type->id);
  assert (R->type->id);

  if (P->type->params_num) {
    R->params = malloc (sizeof (void *) * P->type->params_num);
    assert (R->params);
    int i;
    for (i = 0; i < P->type->params_num; i++) {
      R->params[i] = paramed_type_dup (P->params[i]);
    }
  }
  return R;
}

#ifndef IN_AUTOCOMPLETE_H
void tgl_paramed_type_free (struct paramed_type *P) {
  if (ODDP (P)) { return; }
  if (P->type->params_num) {
    int i;
    for (i = 0; i < P->type->params_num; i++) {
      tgl_paramed_type_free (P->params[i]);
    }
    free (P->params);
  }
  free (P->type->id);
  free (P->type);
  free (P);
}
#else
void tgl_paramed_type_free (struct paramed_type *P);
#endif

#ifndef IN_AUTOCOMPLETE_H
struct paramed_type *tglf_extf_store (struct tgl_state *TLS, const char *data, int data_len) { 
  buffer_pos = (char *)data;
  buffer_end = (char *)(data + data_len);
  local_next_token ();
  return store_function_any ();
}

int tglf_store_type (struct tgl_state *TLS, const char *data, int data_len, struct paramed_type *P) {
  buffer_pos = (char *)data;
  buffer_end = (char *)(data + data_len);
  local_next_token ();
  return store_type_any (P);
}
#endif
