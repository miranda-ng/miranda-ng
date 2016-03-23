static int multiline_output = 1;
static int multiline_offset;
static int multiline_offset_size = 2;

static int disable_field_names;

#define OUT_BUF_SIZE (1 << 25)
static char out_buf[OUT_BUF_SIZE];
static int out_buf_pos;

#define eprintf(...) \
  do { \
    out_buf_pos += _snprintf (out_buf + out_buf_pos, OUT_BUF_SIZE - out_buf_pos, __VA_ARGS__);\
    assert (out_buf_pos < OUT_BUF_SIZE);\
  } while (0)\

static int valid_utf8_char (const char *str) {
  unsigned char c = (unsigned char) *str;
  int n = 0;
 
  if ((c & 0x80) == 0x00) {
    n = 0;
  } else if ((c & 0xe0) == 0xc0) {
    n = 1;
  } else if ((c & 0xf0) == 0xe0) {
    n = 2;
  } else if ((c & 0xf8) == 0xf0) {
    n = 3;
  } else if ((c & 0xfc) == 0xf8) {
    n = 4;
  } else if ((c & 0xfe) == 0xfc) {
    n = 5;
  } else {
    return -1;
  }

  int i;
  for (i = 0; i < n; i ++) {
    if ((((unsigned char)(str[i + 1])) & 0xc0) != 0x80) {
      return -1;
    }
  }
  return n + 1;
}

static void print_escaped_string (const char *str, int len) {
  eprintf ("\"");
  const char *end = str + len;
  while (str < end) {
    int n = valid_utf8_char (str);
    if (n < 0) {
      eprintf ("\\x%02x", (int)(unsigned char)*str);
      str ++;
    } else if (n >= 2) {
      int i;
      for (i = 0; i < n; i++) {
        eprintf ("%c", *(str ++));
      }
    } else if (((unsigned char)*str) >= ' ' && *str != '"' && *str != '\\') {
      eprintf ("%c", *str);      
      str ++;
    } else {
      switch (*str) {
      case '\n':
        eprintf("\\n");
        break;
      case '\r':
        eprintf("\\r");
        break;
      case '\t':
        eprintf("\\t");
        break;
      case '\b':
        eprintf("\\b");
        break;
      case '\a':
        eprintf("\\a");
        break;
      case '\\':
        eprintf ("\\\\");
        break;
      case '"':
        eprintf ("\\\"");
        break;
      default:
        eprintf ("\\x%02x", (int)(unsigned char)*str);
        break;
      }
      str ++;
    }
  }
  eprintf ("\"");
}

static void print_offset (void) {
  int i;
  for (i = 0; i < multiline_offset; i++) {
    eprintf (" ");
  }
}

char *tglf_extf_print_ds (struct tgl_state *TLS, void *DS, struct paramed_type *T) {
  out_buf_pos = 0;
  if (print_ds_type_any (DS, T) < 0) { return 0; }
  return out_buf;
}
