
#include <glib.h>

typedef gchar              mw_mp_sign;
typedef guint16            mw_mp_digit;  /* 2 byte type */
typedef guint32            mw_mp_word;   /* 4 byte type */
typedef gsize              mw_mp_size;
typedef gint               mw_mp_err;

#define MP_DIGIT_BIT       16
#define MP_DIGIT_MAX       G_MAXUINT16
#define MP_WORD_BIT        32
#define MP_WORD_MAX        G_MAXUINT32

#define RADIX              (MP_DIGIT_MAX+1)

#define MP_DIGIT_SIZE      2
#define DIGIT_FMT          "%04X"
