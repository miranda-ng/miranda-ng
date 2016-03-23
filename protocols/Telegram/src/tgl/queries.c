/*
    This file is part of tgl-library

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA

    Copyright Vitaly Valtman 2013-2015
*/

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#define _FILE_OFFSET_BITS 64
#include <string.h>
#include <memory.h>
#include <stdlib.h>
#if defined(WIN32) || defined(_WIN32)
#include <io.h>
#include <stdint.h>
#include <string.h>
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#undef WIN32_LEAN_AND_MEAN
#else
#include <unistd.h>
#include <sys/utsname.h>
#endif
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <share.h>

#include "mtproto-client.h"
#include "queries.h"
#include "tree.h"
#include "mtproto-common.h"
//#include "telegram.h"
#include "tgl-structures.h"
//#include "interface.h"
//#include "net.h"
#include <openssl/bn.h>
#include <openssl/rand.h>
#include <openssl/aes.h>
#include <openssl/sha.h>
#include <openssl/md5.h>

#include "no-preview.h"
#include "tgl-binlog.h"
#include "updates.h"
#include "auto.h"
#include "auto/auto-types.h"
#include "auto/auto-skip.h"
#include "auto/auto-free-ds.h"
#include "auto/auto-fetch-ds.h"
#include "auto/auto-print-ds.h"
#include "tgl.h"
#include "tg-mime-types.h"
#include "mtproto-utils.h"
#include "tgl-methods-in.h"

#define sha1 SHA1

#ifndef PATH_MAX
#define PATH_MAX 4096
#endif


static void out_peer_id (struct tgl_state *TLS, tgl_peer_id_t id);
static struct query_methods send_msgs_methods;

struct messages_send_extra {
  int multi;
  long long id;
  int count;
  long long *list;
};
#define QUERY_TIMEOUT 6.0

struct send_file {
  int fd;
  long long size;
  long long offset;
  int part_num;
  int part_size;
  long long id;
  long long thumb_id;
  tgl_peer_id_t to_id;
  int flags;
  char *file_name;
  int encr;
  int avatar;
  int reply;
  unsigned char *iv;
  unsigned char *init_iv;
  unsigned char *key;
  int w;
  int h;
  int duration;
  char *caption;
};

#define memcmp8(a,b) memcmp ((a), (b), 8)
DEFINE_TREE (query, struct query *, memcmp8, 0) ;

static int mystreq1 (const char *a, const char *b, int l) {
  if ((int)strlen (a) != l) { return 1; }
  return memcmp (a, b, l);
}

/* {{{ COMMON */

struct query *tglq_query_get (struct tgl_state *TLS, long long id) {
  return tree_lookup_query (TLS->queries_tree, (void *)&id);
}

static int alarm_query (struct tgl_state *TLS, struct query *q) {
  assert (q);
  vlogprintf (E_DEBUG - 2, "Alarm query %"_PRINTF_INT64_"d\n", q->msg_id);

  TLS->timer_methods->insert (q->ev, QUERY_TIMEOUT);

  if (q->session && q->session_id && q->DC && q->DC->sessions[0] == q->session && q->session->session_id == q->session_id) {
    clear_packet ();
    out_int (CODE_msg_container);
    out_int (1);
    out_long (q->msg_id);
    out_int (q->seq_no);
    out_int (4 * q->data_len);
    out_ints (q->data, q->data_len);

    tglmp_encrypt_send_message (TLS, q->session->c, packet_buffer, packet_ptr - packet_buffer, q->flags & QUERY_FORCE_SEND);
  } else {
    q->flags &= ~QUERY_ACK_RECEIVED;
    if (tree_lookup_query (TLS->queries_tree, q)) {
      TLS->queries_tree = tree_delete_query (TLS->queries_tree, q);
    }
    q->session = q->DC->sessions[0];
    q->msg_id = tglmp_encrypt_send_message (TLS, q->session->c, q->data, q->data_len, (q->flags & QUERY_FORCE_SEND) | 1);
    TLS->queries_tree = tree_insert_query (TLS->queries_tree, q, lrand48 ());
    q->session_id = q->session->session_id;
    if (!(q->session->dc->flags & 4) && !(q->flags & QUERY_FORCE_SEND)) {
      q->session_id = 0;
    }
  }
  return 0;
}

void tglq_regen_query (struct tgl_state *TLS, long long id) {
  struct query *q = tglq_query_get (TLS, id);
  if (!q) { return; }
  q->flags &= ~QUERY_ACK_RECEIVED;

  if (!(q->session && q->session_id && q->DC && q->DC->sessions[0] == q->session && q->session->session_id == q->session_id)) {
    q->session_id = 0;
  } else {
    if (!(q->session->dc->flags & 4) && !(q->flags & QUERY_FORCE_SEND)) {
      q->session_id = 0;
    }
  }
  vlogprintf (E_NOTICE, "regen query %"_PRINTF_INT64_"d\n", id);
  TLS->timer_methods->insert (q->ev, 0.001);
}

void tglq_query_restart (struct tgl_state *TLS, long long id) {
  struct query *q = tglq_query_get (TLS, id);
  if (q) {
    vlogprintf (E_NOTICE, "restarting query %"_PRINTF_INT64_"d\n", id);
    TLS->timer_methods->remove (q->ev);
    alarm_query (TLS, q);
  }
}

static void alarm_query_gateway (struct tgl_state *TLS, void *arg) {
  alarm_query (TLS, arg);
}


struct query *tglq_send_query_ex (struct tgl_state *TLS, struct tgl_dc *DC, int ints, void *data, struct query_methods *methods, void *extra, void *callback, void *callback_extra, int flags) {
  assert (DC);
  assert (DC->auth_key_id);
  if (!DC->sessions[0]) {
    tglmp_dc_create_session (TLS, DC);
  }
  vlogprintf (E_DEBUG, "Sending query of size %d to DC %d\n", 4 * ints, DC->id);
  struct query *q = talloc0 (sizeof (*q));
  q->data_len = ints;
  q->data = talloc (4 * ints);
  memcpy (q->data, data, 4 * ints);
  q->msg_id = tglmp_encrypt_send_message (TLS, DC->sessions[0]->c, data, ints, 1 | (flags & QUERY_FORCE_SEND));
  q->session = DC->sessions[0];
  q->seq_no = q->session->seq_no - 1;
  q->session_id = q->session->session_id;
  if (!(DC->flags & 4) && !(flags & QUERY_FORCE_SEND)) {
    q->session_id = 0;
  }
  vlogprintf (E_DEBUG, "Msg_id is %"_PRINTF_INT64_"d %p\n", q->msg_id, q);
  q->methods = methods;
  q->type = methods->type;
  q->DC = DC;
  q->flags = flags & QUERY_FORCE_SEND;
  if (TLS->queries_tree) {
    vlogprintf (E_DEBUG + 2, "%"_PRINTF_INT64_"d %"_PRINTF_INT64_"d\n", q->msg_id, TLS->queries_tree->x->msg_id);
  }
  TLS->queries_tree = tree_insert_query (TLS->queries_tree, q, lrand48 ());

  q->ev = TLS->timer_methods->alloc (TLS, alarm_query_gateway, q);
  TLS->timer_methods->insert (q->ev, QUERY_TIMEOUT);

  q->extra = extra;
  q->callback = callback;
  q->callback_extra = callback_extra;
  TLS->active_queries ++;
  return q;
}

struct query *tglq_send_query (struct tgl_state *TLS, struct tgl_dc *DC, int ints, void *data, struct query_methods *methods, void *extra, void *callback, void *callback_extra) {
  return tglq_send_query_ex (TLS, DC, ints, data, methods, extra, callback, callback_extra, 0);
}

static int fail_on_error (struct tgl_state *TLS, struct query *q, int error_code, int l, const char *error) {
  fprintf (stderr, "error #%d: %.*s\n", error_code, l, error);
  assert (0);
  return 0;
}

void tglq_query_ack (struct tgl_state *TLS, long long id) {
  struct query *q = tglq_query_get (TLS, id);
  if (q && !(q->flags & QUERY_ACK_RECEIVED)) {
    assert (q->msg_id == id);
    q->flags |= QUERY_ACK_RECEIVED;
    TLS->timer_methods->remove (q->ev);
  }
}

void tglq_query_delete (struct tgl_state *TLS, long long id) {
  struct query *q = tglq_query_get (TLS, id);
  if (!q) {
    return;
  }
  if (!(q->flags & QUERY_ACK_RECEIVED)) {
    TLS->timer_methods->remove (q->ev);
  }
  TLS->queries_tree = tree_delete_query (TLS->queries_tree, q);
  tfree (q->data, q->data_len * 4);
  TLS->timer_methods->free (q->ev);
  TLS->active_queries --;
}

static void resend_query_cb (struct tgl_state *TLS, void *_q, int success);

void tglq_free_query (struct query *q, void *extra) {
  struct tgl_state *TLS = extra;
  if (!(q->flags & QUERY_ACK_RECEIVED)) {
    TLS->timer_methods->remove (q->ev);
  }
  tfree (q->data, q->data_len * 4);
  TLS->timer_methods->free (q->ev);
}

void tglq_query_free_all (struct tgl_state *TLS) {
  tree_act_ex_query (TLS->queries_tree, tglq_free_query, TLS);
  TLS->queries_tree = tree_clear_query (TLS->queries_tree);
}

int tglq_query_error (struct tgl_state *TLS, long long id) {
  assert (fetch_int () == CODE_rpc_error);
  int error_code = fetch_int ();
  int error_len = prefetch_strlen ();
  char *error = fetch_str (error_len);
  struct query *q = tglq_query_get (TLS, id);
  if (!q) {
    vlogprintf (E_WARNING, "error for query #%"_PRINTF_INT64_"d: #%d :%.*s\n", id, error_code, error_len, error);
    vlogprintf (E_WARNING, "No such query\n");
  } else {
    if (!(q->flags & QUERY_ACK_RECEIVED)) {
      TLS->timer_methods->remove (q->ev);
    }
    TLS->queries_tree = tree_delete_query (TLS->queries_tree, q);
    int res = 0;

    int error_handled = 0;

    switch (error_code) {
    case 303:
      // migrate
      {
        int offset = -1;
        if (error_len >= 15 && !memcmp (error, "PHONE_MIGRATE_", 14)) {
          offset = 14;
        }
        if (error_len >= 17 && !memcmp (error, "NETWORK_MIGRATE_", 16)) {
          offset = 16;
        }
        if (error_len >= 14 && !memcmp (error, "USER_MIGRATE_", 13)) {
          offset = 13;
        }
        if (offset >= 0) {
          int i = 0;
          while (offset < error_len && error[offset] >= '0' && error[offset] <= '9') {
            i = i * 10 + error[offset] - '0';
            offset ++;
          }
          if (i > 0 && i < TGL_MAX_DC_NUM) {
            bl_do_set_working_dc (TLS, i);
            q->flags &= ~QUERY_ACK_RECEIVED;
            //q->session_id = 0;
            //struct tgl_dc *DC = q->DC;
            //if (!(DC->flags & 4) && !(q->flags & QUERY_FORCE_SEND)) {
            q->session_id = 0;
            //}
            q->DC = TLS->DC_working;
            TLS->timer_methods->insert (q->ev, 0);
            error_handled = 1;
            res = 1;
          }
        }
      }
      break;
    case 400:
      // nothing to handle
      // bad user input probably
      break;
    case 401:
      if (!mystreq1 ("SESSION_PASSWORD_NEEDED", error, error_len)) {
        if (!(TLS->locks & TGL_LOCK_PASSWORD)) {
          TLS->locks |= TGL_LOCK_PASSWORD;
          tgl_do_check_password (TLS, resend_query_cb, q);
        }
        res = 1;
        error_handled = 1;
      }
      break;
    case 403:
      // privacy violation
      break;
    case 404:
      // not found
      break;
    case 420:
      // flood
    case 500:
      // internal error
    default:
      // anything else. Treated as internal error
      {
        int wait;
        if (strncmp (error, "FLOOD_WAIT_", 11)) {
          if (error_code == 420) {
            vlogprintf (E_ERROR, "error = '%s'\n", error);
          }
          wait = 10;
        } else {
          wait = (int)atoll (error + 11);
        }
        q->flags &= ~QUERY_ACK_RECEIVED;
        TLS->timer_methods->insert (q->ev, wait);
        struct tgl_dc *DC = q->DC;
        if (!(DC->flags & 4) && !(q->flags & QUERY_FORCE_SEND)) {
          q->session_id = 0;
        }
        error_handled = 1;
      }
      break;
    }

    if (error_handled) {
      vlogprintf (E_DEBUG - 2, "error for query #%"_PRINTF_INT64_"d: #%d %.*s (HANDLED)\n", id, error_code, error_len, error);
    } else {
      vlogprintf (E_WARNING, "error for query #%"_PRINTF_INT64_"d: #%d %.*s\n", id, error_code, error_len, error);
      if (q->methods && q->methods->on_error) {
        res = q->methods->on_error (TLS, q, error_code, error_len, error);
      }
    }

    if (res <= 0) {
      tfree (q->data, q->data_len * 4);
      TLS->timer_methods->free (q->ev);
    }

    if (res == -11) {
      TLS->active_queries --;
      return -1;

    }
  }
  TLS->active_queries --;
  return 0;
}

#define MAX_PACKED_SIZE (1 << 24)
static int packed_buffer[MAX_PACKED_SIZE / 4];

int tglq_query_result (struct tgl_state *TLS, long long id) {
  vlogprintf (E_DEBUG, "result for query #%"_PRINTF_INT64_"d. Size %ld bytes\n", id, (long)4 * (in_end - in_ptr));
  int op = prefetch_int ();
  int *end = 0;
  int *eend = 0;
  if (op == CODE_gzip_packed) {
    fetch_int ();
    int l = prefetch_strlen ();
    char *s = fetch_str (l);
    int total_out = tgl_inflate (s, l, packed_buffer, MAX_PACKED_SIZE);
    vlogprintf (E_DEBUG, "inflated %d bytes\n", total_out);
    end = in_ptr;
    eend = in_end;
    in_ptr = packed_buffer;
    in_end = in_ptr + total_out / 4;
  }
  struct query *q = tglq_query_get (TLS, id);
  if (!q) {
    vlogprintf (E_WARNING, "No such query\n");
    in_ptr = in_end;
  } else {
    if (!(q->flags & QUERY_ACK_RECEIVED)) {
      TLS->timer_methods->remove (q->ev);
    }
    TLS->queries_tree = tree_delete_query (TLS->queries_tree, q);
    if (q->methods && q->methods->on_answer) {
      assert (q->type);
      int *save = in_ptr;
      vlogprintf (E_DEBUG, "in_ptr = %p, end_ptr = %p\n", in_ptr, in_end);
      if (skip_type_any (q->type) < 0) {
        vlogprintf (E_ERROR, "Skipped %ld int out of %ld (type %s)\n", (long)(in_ptr - save), (long)(in_end - save), q->type->type->id);
        vlogprintf (E_ERROR, "0x%08x 0x%08x\n", *(in_ptr - 1), *in_ptr);
        assert (0);
      }

      assert (in_ptr == in_end);
      in_ptr = save;

      void *DS = fetch_ds_type_any (q->type);
      assert (DS);

      q->methods->on_answer (TLS, q, DS);
      free_ds_type_any (DS, q->type);

      assert (in_ptr == in_end);
    }
    tfree (q->data, 4 * q->data_len);
    TLS->timer_methods->free (q->ev);
    tfree (q, sizeof (*q));

  }
  if (end) {
    in_ptr = end;
    in_end = eend;
  }
  TLS->active_queries --;
  return 0;
}

static void out_random (int n) {
  assert (n <= 32);
  static char buf[32];
  tglt_secure_random (buf, n);
  out_cstring (buf, n);
}

int allow_send_os_version;
void tgl_do_insert_header (struct tgl_state *TLS) {
  out_int (CODE_invoke_with_layer);
  out_int (TGL_SCHEME_LAYER);
  out_int (CODE_init_connection);
  out_int (TLS->app_id);
  if (allow_send_os_version) {
    static char buf[4096];
#if defined(WIN32) || defined(_WIN32)
    SYSTEM_INFO sysInfo;
    char release[256] = { 0 };
    char version[256] = { 0 };
    OSVERSIONINFO versionInfo = { sizeof(OSVERSIONINFO), 0, 0, 0, 0,{ '\0' } };
    char* szSystemName = "Windows";
#if (WINVER <= _WIN32_WINNT_WIN7)
    GetVersionEx(&versionInfo);
#else
    // GetModuleHandle is not supported in WinRT and linking to it at load time
    #if WINAPI_FAMILY == WINAPI_FAMILY_PHONE_APP || WINAPI_FAMILY==WINAPI_FAMILY_APP
      szSystemName = "Windows RT"
      MEMORY_BASIC_INFORMATION mbi = { 0, 0, 0, 0, 0, 0, 0 };
      VirtualQuery((LPCVOID)VirtualQuery, &mbi, sizeof(mbi);
      HMODULE kernelModule = (HMODULE)mbi.AllocationBase;

      typedef HMODULE(WINAPI *GetModuleHandleFunction)(LPCWSTR);
      GetModuleHandleFunction pGetModuleHandle = (GetModuleHandleFunction) GetProcAddressA (kernelModule, "GetModuleHandleW");
    #else
      #define pGetModuleHandle GetModuleHandleW
    #endif

    #if !defined(WINCE) || !defined(_WIN32_WCE)
      #define GetProcAddressA GetProcAddress
    #endif

    HMODULE ntdll = pGetModuleHandle (L"ntdll.dll");
    typedef LONG NTSTATUS;
    typedef NTSTATUS(NTAPI *RtlGetVersionFunction)(LPOSVERSIONINFO);

    RtlGetVersionFunction pRtlGetVersion = (RtlGetVersionFunction)	GetProcAddressA (ntdll, "RtlGetVersion");
    if(pRtlGetVersion)
    pRtlGetVersion(&versionInfo); // always returns STATUS_SUCCESS
#endif
    if(versionInfo.dwBuildNumber > 0)
      _itoa_s(versionInfo.dwBuildNumber, release, sizeof(char) * 256, 10);
    if (versionInfo.dwMajorVersion > 0)
      sprintf_s(version, sizeof(char) * 256, "%i.%i", versionInfo.dwMajorVersion, versionInfo.dwMinorVersion);
    else
      strcpy_s(version, sizeof(char) * 256, "Unknown");

    // Get hardware info
    ZeroMemory(&sysInfo, sizeof(SYSTEM_INFO));
    GetSystemInfo(&sysInfo);

    switch (sysInfo.wProcessorArchitecture) {
    case PROCESSOR_ARCHITECTURE_AMD64:
      out_string("x86_64");
      break;
    case PROCESSOR_ARCHITECTURE_IA64:
      out_string("ia64");
      break;
    case PROCESSOR_ARCHITECTURE_INTEL:
      out_string("x86");
      break;
    case PROCESSOR_ARCHITECTURE_ARM:
      out_string("ARM");
      break;
    case PROCESSOR_ARCHITECTURE_PPC:
      out_string("PowerPC");
      break;
    case PROCESSOR_ARCHITECTURE_MIPS:
      out_string("MIPS");
      break;
    case PROCESSOR_ARCHITECTURE_MSIL:
      out_string("MSIL");
      break;
    case PROCESSOR_ARCHITECTURE_SHX:
      out_string("ShX");
      break;
    case PROCESSOR_ARCHITECTURE_UNKNOWN:
    default:
       out_string("unknown");
    }
    tsnprintf(buf, sizeof(buf) - 1, "%.999s %.999s %.999s", szSystemName, version, release);
#else
    struct utsname st;
    uname (&st);
    out_string (st.machine);
    tsnprintf (buf, sizeof (buf) - 1, "%.999s %.999s %.999s", st.sysname, st.release, st.version);
#endif
    out_string (buf);
    tsnprintf (buf, sizeof (buf) - 1, "%s (TGL %s)", TLS->app_version, TGL_VERSION);
    out_string (buf);
    out_string ("En");
  } else {
#if defined(WIN32) || defined(_WIN32)
    out_string("x86");
    out_string("Windows");
#else
    out_string ("x86");
    out_string ("Linux");
#endif
    static char buf[4096];
    tsnprintf (buf, sizeof (buf) - 1, "%s (TGL %s)", TLS->app_version, TGL_VERSION);
    out_string (buf);
    out_string ("en");
  }
}

void tgl_set_query_error (struct tgl_state *TLS, int error_code, const char *format, ...) __attribute__ ((format (printf, 3, 4)));
void tgl_set_query_error (struct tgl_state *TLS, int error_code, const char *format, ...) {
  static char s[1001];

  va_list ap;
  va_start (ap, format);
  vsnprintf (s, 1000, format, ap);
  va_end (ap);

  if (TLS->error) {
    tfree_str (TLS->error);
  }
  TLS->error = tstrdup (s);
  TLS->error_code = error_code;
}
/* }}} */

/* {{{ Default on error */

static int q_void_on_error (struct tgl_state *TLS, struct query *q, int error_code, int error_len, const char *error) {
  tgl_set_query_error (TLS, EPROTO, "RPC_CALL_FAIL %d: %.*s", error_code, error_len, error);
  if (q->callback) {
    ((void (*)(struct tgl_state *,void *, int))(q->callback))(TLS, q->callback_extra, 0);
  }
  return 0;
}

static int q_ptr_on_error (struct tgl_state *TLS, struct query *q, int error_code, int error_len, const char *error) {
  tgl_set_query_error (TLS, EPROTO, "RPC_CALL_FAIL %d: %.*s", error_code, error_len, error);
  if (q->callback) {
    ((void (*)(struct tgl_state *,void *, int, void *))(q->callback))(TLS, q->callback_extra, 0, NULL);
  }
  return 0;
}

static int q_list_on_error (struct tgl_state *TLS, struct query *q, int error_code, int error_len, const char *error) {
  tgl_set_query_error (TLS, EPROTO, "RPC_CALL_FAIL %d: %.*s", error_code, error_len, error);
  if (q->callback) {
    ((void (*)(struct tgl_state *,void *, int, int, void *))(q->callback))(TLS, q->callback_extra, 0, 0, NULL);
  }
  return 0;
}
/* }}} */

#include "queries-encrypted.c"

/* {{{ Get config */

static void fetch_dc_option (struct tgl_state *TLS, struct tl_ds_dc_option *DS_DO) {
  bl_do_dc_option_new (TLS, DS_LVAL (DS_DO->flags), DS_LVAL (DS_DO->id), DS_STR (DS_DO->hostname), DS_STR (DS_DO->ip_address), DS_LVAL (DS_DO->port));
}

static int help_get_config_on_answer (struct tgl_state *TLS, struct query *q, void *DS) {
  struct tl_ds_config *DS_C = DS;

  int i;
  for (i = 0; i < DS_LVAL (DS_C->dc_options->cnt); i++) {
    fetch_dc_option (TLS, DS_C->dc_options->data[i]);
  }

  int max_chat_size = DS_LVAL (DS_C->chat_size_max);
  int max_bcast_size = DS_LVAL (DS_C->broadcast_size_max);
  vlogprintf (E_DEBUG, "chat_size = %d, bcast_size = %d\n", max_chat_size, max_bcast_size);

  if (q->callback) {
    ((void (*)(struct tgl_state *,void *, int))(q->callback))(TLS, q->callback_extra, 1);
  }
  return 0;
}

static struct query_methods help_get_config_methods  = {
  .on_answer = help_get_config_on_answer,
  .on_error = q_void_on_error,
  .type = TYPE_TO_PARAM(config)
};

void tgl_do_help_get_config (struct tgl_state *TLS, void (*callback)(struct tgl_state *,void *, int), void *callback_extra) {
  clear_packet ();
  tgl_do_insert_header (TLS);
  out_int (CODE_help_get_config);
  tglq_send_query (TLS, TLS->DC_working, packet_ptr - packet_buffer, packet_buffer, &help_get_config_methods, 0, callback, callback_extra);
}

void tgl_do_help_get_config_dc (struct tgl_state *TLS, struct tgl_dc *D, void (*callback)(struct tgl_state *, void *, int), void *callback_extra) {
  clear_packet ();
  tgl_do_insert_header (TLS);
  out_int (CODE_help_get_config);
  tglq_send_query_ex (TLS, D, packet_ptr - packet_buffer, packet_buffer, &help_get_config_methods, 0, callback, callback_extra, 2);
}
/* }}} */

/* {{{ Send code */
static int send_code_on_answer (struct tgl_state *TLS, struct query *q, void *D) {
  struct tl_ds_auth_sent_code *DS_ASC = D;

  char *phone_code_hash = DS_STR_DUP (DS_ASC->phone_code_hash);
  int registered = DS_BVAL (DS_ASC->phone_registered);;

  if (q->callback) {
    ((void (*)(struct tgl_state *, void *, int, int, const char *))(q->callback)) (TLS, q->callback_extra, 1, registered, phone_code_hash);
  }
  tfree_str (phone_code_hash);
  return 0;
}

static struct query_methods send_code_methods  = {
  .on_answer = send_code_on_answer,
  .on_error = q_list_on_error,
  .type = TYPE_TO_PARAM(auth_sent_code)
};

void tgl_do_send_code (struct tgl_state *TLS, const char *phone, int phone_len, void (*callback)(struct tgl_state *TLS, void *callback_extra, int success, int registered, const char *hash), void *callback_extra) {
  vlogprintf (E_DEBUG, "sending code to dc %d\n", TLS->dc_working_num);

  clear_packet ();
  tgl_do_insert_header (TLS);
  out_int (CODE_auth_send_code);
  out_cstring (phone, phone_len);
  out_int (0);
  out_int (TLS->app_id);
  out_string (TLS->app_hash);
  out_string ("en");

  tglq_send_query (TLS, TLS->DC_working, packet_ptr - packet_buffer, packet_buffer, &send_code_methods, NULL, callback, callback_extra);
}


static int phone_call_on_answer (struct tgl_state *TLS, struct query *q, void *D) {
  if (q->callback) {
    ((void (*)(struct tgl_state *TLS, void *, int))(q->callback))(TLS, q->callback_extra, 1);
  }
  return 0;
}

static struct query_methods phone_call_methods  = {
  .on_answer = phone_call_on_answer,
  .on_error = q_void_on_error,
  .type = TYPE_TO_PARAM(bool)
};

void tgl_do_phone_call (struct tgl_state *TLS, const char *phone, int phone_len, const char *hash, int hash_len, void (*callback)(struct tgl_state *TLS, void *callback_extra, int success), void *callback_extra) {
  vlogprintf (E_DEBUG, "calling user\n");

  clear_packet ();
  tgl_do_insert_header (TLS);
  out_int (CODE_auth_send_call);
  out_cstring (phone, phone_len);
  out_cstring (hash, hash_len);

  tglq_send_query (TLS, TLS->DC_working, packet_ptr - packet_buffer, packet_buffer, &phone_call_methods, NULL, callback, callback_extra);
}
/* }}} */

/* {{{ Sign in / Sign up */
static int sign_in_on_answer (struct tgl_state *TLS, struct query *q, void *D) {
  struct tl_ds_auth_authorization *DS_AA = D;
  //vlogprintf (E_DEBUG, "Expires in %d\n", DS_LVAL (DS_AA->expires));

  struct tgl_user *U = tglf_fetch_alloc_user_new (TLS, DS_AA->user);

  bl_do_dc_signed (TLS, TLS->DC_working->id);

  if (q->callback) {
    ((void (*)(struct tgl_state *TLS, void *, int, struct tgl_user *))q->callback) (TLS, q->callback_extra, 1, U);
  }

  return 0;
}

static int sign_in_on_error (struct tgl_state *TLS, struct query *q, int error_code, int error_len, const char *error) {
  tgl_set_query_error (TLS, EPROTO, "RPC_CALL_FAIL %d: %.*s", error_code, error_len, error);
  if (q->callback) {
    ((void (*)(struct tgl_state *, void *, int, struct tgl_user *))q->callback) (TLS, q->callback_extra, 0, NULL);
  }
  return 0;
}

static struct query_methods sign_in_methods  = {
  .on_answer = sign_in_on_answer,
  .on_error = sign_in_on_error,
  .type = TYPE_TO_PARAM(auth_authorization)
};

int tgl_do_send_code_result (struct tgl_state *TLS, const char *phone, int phone_len, const char *hash, int hash_len, const char *code, int code_len, void (*callback)(struct tgl_state *TLS, void *callback_extra, int success, struct tgl_user *Self), void *callback_extra) {
  clear_packet ();
  out_int (CODE_auth_sign_in);
  out_cstring (phone, phone_len);
  out_cstring (hash, hash_len);
  out_cstring (code, code_len);
  tglq_send_query (TLS, TLS->DC_working, packet_ptr - packet_buffer, packet_buffer, &sign_in_methods, 0, callback, callback_extra);
  return 0;
}

int tgl_do_send_code_result_auth (struct tgl_state *TLS, const char *phone, int phone_len, const char *hash, int hash_len, const char *code, int code_len, const char *first_name, int first_name_len, const char *last_name, int last_name_len, void (*callback)(struct tgl_state *TLS, void *callback_extra, int success, struct tgl_user *Self), void *callback_extra) {
  clear_packet ();
  out_int (CODE_auth_sign_up);
  out_cstring (phone, phone_len);
  out_cstring (hash, hash_len);
  out_cstring (code, code_len);
  out_cstring (first_name, first_name_len);
  out_cstring (last_name, last_name_len);
  tglq_send_query (TLS, TLS->DC_working, packet_ptr - packet_buffer, packet_buffer, &sign_in_methods, 0, callback, callback_extra);
  return 0;
}

int tgl_do_send_bot_auth (struct tgl_state *TLS, const char *code, int code_len, void (*callback)(struct tgl_state *TLS, void *callback_extra, int success, struct tgl_user *Self), void *callback_extra) {
  clear_packet ();
  out_int (CODE_auth_import_bot_authorization);
  out_int (0);
  out_int (TLS->app_id);
  out_string (TLS->app_hash);
  out_cstring (code, code_len);
  tglq_send_query (TLS, TLS->DC_working, packet_ptr - packet_buffer, packet_buffer, &sign_in_methods, 0, callback, callback_extra);
  return 0;
}
/* }}} */

/* {{{ Get contacts */
static int get_contacts_on_answer (struct tgl_state *TLS, struct query *q, void *D) {
  struct tl_ds_contacts_contacts *DS_CC = D;

  int n = DS_CC->users ? DS_LVAL (DS_CC->users->cnt) : 0;

  int i;
  struct tgl_user **list = talloc (sizeof (void *) * n);
  for (i = 0; i < n; i++) {
    list[i] = tglf_fetch_alloc_user_new (TLS, DS_CC->users->data[i]);
  }
  if (q->callback) {
    ((void (*)(struct tgl_state *TLS, void *, int, int, struct tgl_user **))q->callback) (TLS, q->callback_extra, 1, n, list);
  }
  tfree (list, sizeof (void *) * n);
  return 0;
}

static struct query_methods get_contacts_methods = {
  .on_answer = get_contacts_on_answer,
  .on_error = q_list_on_error,
  .type = TYPE_TO_PARAM(contacts_contacts)
};


void tgl_do_update_contact_list (struct tgl_state *TLS, void (*callback) (struct tgl_state *TLS, void *callback_extra, int success, int size, struct tgl_user *contacts[]), void *callback_extra) {
  clear_packet ();
  out_int (CODE_contacts_get_contacts);
  out_string ("");
  tglq_send_query (TLS, TLS->DC_working, packet_ptr - packet_buffer, packet_buffer, &get_contacts_methods, NULL, callback, callback_extra);
}
/* }}} */

/* {{{ Send msg (plain text) */
static int msg_send_on_answer (struct tgl_state *TLS, struct query *q, void *D) {
  struct tl_ds_messages_sent_message *DS_MSM = D;

  long long y = *(long long *)q->extra;
  tfree (q->extra, 8);

  struct tgl_message *M = tgl_message_get (TLS, y);
  vlogprintf (E_DEBUG, "y = %"_PRINTF_INT64_"d\n", y);

  if (M && M->id != DS_LVAL (DS_MSM->id)) {
    assert (M->flags & TGLMF_PENDING);
    bl_do_create_message_new (TLS, M->id, NULL, NULL, NULL, NULL, NULL,
      DS_MSM->date, NULL, 0, DS_MSM->media, NULL, NULL, NULL, M->flags & 0xffff);
  }

  struct tl_ds_update *UPD = talloc0 (sizeof (*UPD));
  UPD->magic = CODE_update_message_i_d;
  UPD->id = talloc (4);
  *UPD->id = DS_LVAL (DS_MSM->id);
  UPD->random_id = talloc (8);
  *UPD->random_id = y;
  UPD->pts_count = talloc (4);
  *UPD->pts_count = DS_LVAL (DS_MSM->pts_count);
  UPD->pts = talloc (4);
  *UPD->pts = DS_LVAL (DS_MSM->pts);

  tglu_work_update_new (TLS, 1, UPD);
  tglu_work_update_new (TLS, 0, UPD);

  *UPD->pts_count = 0;
  tfree (UPD->random_id, 8);
  UPD->magic = CODE_update_msg_update;

  tglu_work_update_new (TLS, 1, UPD);
  tglu_work_update_new (TLS, 0, UPD);

  free_ds_type_update (UPD, TYPE_TO_PARAM (update));

  y = tgls_get_local_by_random (TLS, y);
  M = tgl_message_get (TLS, y);

  if (q->callback) {
    ((void (*)(struct tgl_state *,void *, int, struct tgl_message *))q->callback) (TLS, q->callback_extra, 1, M);
  }
  return 0;
}

static int msg_send_on_error (struct tgl_state *TLS, struct query *q, int error_code, int error_len, const char *error) {
  tgl_set_query_error (TLS, EPROTO, "RPC_CALL_FAIL %d: %.*s", error_code, error_len, error);
  long long x = *(long long *)q->extra;
  tfree (q->extra, 8);
  struct tgl_message *M = tgl_message_get (TLS, x);
  if (q->callback) {
    ((void (*)(struct tgl_state *,void *, int, struct tgl_message *))q->callback) (TLS, q->callback_extra, 0, M);
  }
  if (M) {
    bl_do_message_delete (TLS, M);
  }
  return 0;
}

static struct query_methods msg_send_methods = {
  .on_answer = msg_send_on_answer,
  .on_error = msg_send_on_error,
  .type = TYPE_TO_PARAM(messages_sent_message)
};

void tgl_do_send_msg (struct tgl_state *TLS, struct tgl_message *M, void (*callback)(struct tgl_state *TLS, void *callback_extra, int success, struct tgl_message *M), void *callback_extra) {
  if (tgl_get_peer_type (M->to_id) == TGL_PEER_ENCR_CHAT) {
    tgl_do_send_encr_msg (TLS, M, callback, callback_extra);
    return;
  }
  clear_packet ();
  out_int (CODE_messages_send_message);

  out_int (((M->flags & TGLMF_DISABLE_PREVIEW) ? 2 : 0) | (M->reply_id ? 1 : 0) | (M->reply_markup ? 4 : 0));
  out_peer_id (TLS, M->to_id);
  if (M->reply_id) {
    out_int (M->reply_id);
  }
  out_cstring (M->message, M->message_len);
  out_long (M->id);
  long long *x = talloc (8);
  *x = M->id;

  if (M->reply_markup) {
    if (M->reply_markup->rows) {
      out_int (CODE_reply_keyboard_markup);
      out_int (M->reply_markup->flags);
      out_int (CODE_vector);
      out_int (M->reply_markup->rows);
      int i;
      for (i = 0; i < M->reply_markup->rows; i++) {
        out_int (CODE_keyboard_button_row);
        out_int (CODE_vector);
        out_int (M->reply_markup->row_start[i + 1] - M->reply_markup->row_start[i]);
        int j;
        for (j = 0; j < M->reply_markup->row_start[i + 1] - M->reply_markup->row_start[i]; j++) {
          out_int (CODE_keyboard_button);
          out_string (M->reply_markup->buttons[j + M->reply_markup->row_start[i]]);
        }
      }
    } else {
      out_int (CODE_reply_keyboard_hide);
    }
  }

  tglq_send_query (TLS, TLS->DC_working, packet_ptr - packet_buffer, packet_buffer, &msg_send_methods, x, callback, callback_extra);
}

void tgl_do_send_message (struct tgl_state *TLS, tgl_peer_id_t id, const char *text, int text_len, unsigned long long flags, struct tl_ds_reply_markup *reply_markup, void (*callback)(struct tgl_state *TLS, void *callback_extra, int success, struct tgl_message *M), void *callback_extra) {
  if (tgl_get_peer_type (id) == TGL_PEER_ENCR_CHAT) {
    tgl_peer_t *P = tgl_peer_get (TLS, id);
    if (!P) {
      tgl_set_query_error (TLS, EINVAL, "unknown secret chat");
      if (callback) {
        callback (TLS, callback_extra, 0, 0);
      }
      return;
    }
    if (P->encr_chat.state != sc_ok) {
      tgl_set_query_error (TLS, EINVAL, "secret chat not in ok state");
      if (callback) {
        callback (TLS, callback_extra, 0, 0);
      }
      return;
    }
  }
  long long t;
  tglt_secure_random (&t, 8);

  int peer_type = tgl_get_peer_type (id);
  int peer_id = tgl_get_peer_id (id);
  int date = (int)time (0);

  if (tgl_get_peer_type (id) != TGL_PEER_ENCR_CHAT) {
    int reply = (flags >> 32);
    int disable_preview = flags & TGL_SEND_MSG_FLAG_DISABLE_PREVIEW;
    if (!(flags & TGL_SEND_MSG_FLAG_ENABLE_PREVIEW) && TLS->disable_link_preview) {
      disable_preview = 1;
    }
    if (disable_preview) {
      disable_preview = TGLMF_DISABLE_PREVIEW;
    }
    struct tl_ds_message_media TDSM;
    TDSM.magic = CODE_message_media_empty;

    bl_do_create_message_new (TLS, t, &TLS->our_id, &peer_type, &peer_id, NULL, NULL, &date, text, text_len, &TDSM, NULL, reply ? &reply : NULL, reply_markup, TGLMF_UNREAD | TGLMF_OUT | TGLMF_PENDING | TGLMF_CREATE | TGLMF_CREATED | TGLMF_SESSION_OUTBOUND | disable_preview);
  } else {
    struct tl_ds_decrypted_message_media TDSM;
    TDSM.magic = CODE_decrypted_message_media_empty;

    bl_do_create_message_encr_new (TLS, t, &TLS->our_id, &peer_type, &peer_id, &date, text, text_len, &TDSM, NULL, NULL, TGLMF_UNREAD | TGLMF_OUT | TGLMF_PENDING | TGLMF_CREATE | TGLMF_CREATED | TGLMF_SESSION_OUTBOUND | TGLMF_ENCRYPTED);
  }

  struct tgl_message *M = tgl_message_get (TLS, t);
  assert (M);
  tgl_do_send_msg (TLS, M, callback, callback_extra);
}

void tgl_do_reply_message (struct tgl_state *TLS, int reply_id, const char *text, int text_len, unsigned long long flags, void (*callback)(struct tgl_state *TLS, void *callback_extra, int success, struct tgl_message *M), void *callback_extra) {
  struct tgl_message *M = tgl_message_get (TLS, reply_id);
  if (!M || !(M->flags & TGLMF_CREATED) || (M->flags & TGLMF_ENCRYPTED)) {
    if (!M || !(M->flags & TGLMF_CREATED)) {
      tgl_set_query_error (TLS, EINVAL, "unknown message");
    } else {
      tgl_set_query_error (TLS, EINVAL, "can not reply on message from secret chat");
    }
    if (callback) {
      callback (TLS, callback_extra, 0, 0);
    }
    return;
  }

  tgl_peer_id_t id = M->to_id;
  if (tgl_get_peer_type (id) == TGL_PEER_USER && tgl_get_peer_id (id) == TLS->our_id) {
    id = M->from_id;
  }

  tgl_do_send_message (TLS, id, text, text_len, flags | TGL_SEND_MSG_FLAG_REPLY (reply_id), NULL, callback, callback_extra);
}
/* }}} */

/* {{{ Send text file */
void tgl_do_send_text (struct tgl_state *TLS, tgl_peer_id_t id, const char *file_name, unsigned long long flags, void (*callback)(struct tgl_state *TLS, void *callback_extra, int success, struct tgl_message *M), void *callback_extra) {
#if defined(_MSC_VER) && _MSC_VER >= 1400
  int fd = 0;
  errno_t err = _sopen_s(&fd, file_name, _O_RDONLY | _O_BINARY, _SH_DENYNO, _S_IREAD | _S_IWRITE);
  if(err != 0) {
	tgl_set_query_error(TLS, EBADF, "Can not open file: %s", GetErrnoStr (errno));
#elif defined(WIN32) || defined(_WIN32)
  int fd = open(file_name, O_RDONLY | O_BINARY);
  if (fd < 0) {
	tgl_set_query_error (TLS, EBADF, "Can not open file: %s", GetErrnoStr (errno));
#else
  int fd = open (file_name, O_RDONLY);
  if (fd < 0) {
    tgl_set_query_error (TLS, EBADF, "Can not open file: %m");
#endif
    if (callback) {
      callback (TLS, callback_extra, 0, NULL);
    }
    return;
  }
  static char buf[(1 << 20) + 1];
  int x = read (fd, buf, (1 << 20) + 1);
  if (x < 0) {
#if defined(WIN32) || defined(_WIN32)
    tgl_set_query_error (TLS, EBADF, "Can not read from file: %s", GetErrnoStr (errno));
#else
    tgl_set_query_error (TLS, EBADF, "Can not read from file: %m");
#endif
	close (fd);

    if (callback) {
      callback (TLS, callback_extra, 0, NULL);
    }
    return;
  }

  assert (x >= 0);
  close (fd);
  if (x == (1 << 20) + 1) {
    tgl_set_query_error (TLS, E2BIG, "text file is too big");
    if (callback) {
      callback (TLS, callback_extra, 0, NULL);
    }
  } else {
    tgl_do_send_message (TLS, id, buf, x, flags, NULL, callback, callback_extra);
  }
}

void tgl_do_reply_text (struct tgl_state *TLS, int reply_id, const char *file_name, unsigned long long flags, void (*callback)(struct tgl_state *TLS, void *callback_extra, int success, struct tgl_message *M), void *callback_extra) {
  struct tgl_message *M = tgl_message_get (TLS, reply_id);
  if (!M || !(M->flags & TGLMF_CREATED) || (M->flags & TGLMF_ENCRYPTED)) {
    if (!M || !(M->flags & TGLMF_CREATED)) {
      tgl_set_query_error (TLS, EINVAL, "unknown message");
    } else {
      tgl_set_query_error (TLS, EINVAL, "can not reply on message from secret chat");
    }
    if (callback) {
      callback (TLS, callback_extra, 0, 0);
    }
    return;
  }

  tgl_peer_id_t id = M->to_id;
  if (tgl_get_peer_type (id) == TGL_PEER_USER && tgl_get_peer_id (id) == TLS->our_id) {
    id = M->from_id;
  }

  tgl_do_send_text (TLS, id, file_name, flags | TGL_SEND_MSG_FLAG_REPLY (reply_id), callback, callback_extra);
}
/* }}} */

/* {{{ Mark read */

struct mark_read_extra {
  tgl_peer_id_t id;
  int max_id;
};

void tgl_do_messages_mark_read (struct tgl_state *TLS, tgl_peer_id_t id, int max_id, int offset, void (*callback)(struct tgl_state *TLS, void *callback_extra, int), void *callback_extra);

static int mark_read_on_receive (struct tgl_state *TLS, struct query *q, void *D) {
  struct tl_ds_messages_affected_history *DS_MAH = D;

  int r = tgl_check_pts_diff (TLS, DS_LVAL (DS_MAH->pts), DS_LVAL (DS_MAH->pts_count));

  if (r > 0) {
    bl_do_set_pts (TLS, DS_LVAL (DS_MAH->pts));
  }

  int offset = DS_LVAL (DS_MAH->offset);

  struct mark_read_extra *E = q->extra;
  if (offset > 0) {
    tgl_do_messages_mark_read (TLS, E->id, E->max_id, offset, q->callback, q->callback_extra);
  } else {
    if (tgl_get_peer_type (E->id) == TGL_PEER_USER) {
      bl_do_user_new (TLS, tgl_get_peer_id (E->id), NULL, NULL, 0, NULL, 0, NULL, 0, NULL, 0, NULL, NULL, 0, NULL, 0, NULL, &E->max_id, NULL, NULL, TGL_FLAGS_UNCHANGED);
    } else {
      assert (tgl_get_peer_type (E->id) == TGL_PEER_CHAT);
      bl_do_chat_new (TLS, tgl_get_peer_id (E->id), NULL, 0, NULL, NULL, NULL, NULL, NULL, NULL, NULL, &E->max_id, NULL, TGL_FLAGS_UNCHANGED);
    }
    if (q->callback) {
      ((void (*)(struct tgl_state *, void *, int))q->callback)(TLS, q->callback_extra, 1);
    }
  }
  tfree (E, sizeof (*E));
  return 0;
}

static int mark_read_on_error (struct tgl_state *TLS, struct query *q, int error_code, int error_len, const char *error) {
  tgl_set_query_error (TLS, EPROTO, "RPC_CALL_FAIL %d: %.*s", error_code, error_len, error);

  struct mark_read_extra *E = q->extra;
  tfree (E, sizeof (*E));
  if (q->callback) {
    ((void (*)(struct tgl_state *, void *, int))q->callback)(TLS, q->callback_extra, 0);
  }
  return 0;
}

static struct query_methods mark_read_methods = {
  .on_answer = mark_read_on_receive,
  .on_error = mark_read_on_error,
  .type = TYPE_TO_PARAM(messages_affected_history)
};

void tgl_do_messages_mark_read (struct tgl_state *TLS, tgl_peer_id_t id, int max_id, int offset, void (*callback)(struct tgl_state *TLS, void *callback_extra, int), void *callback_extra) {
  if (TLS->is_bot) { return; }
  clear_packet ();
  out_int (CODE_messages_read_history);
  out_peer_id (TLS, id);
  out_int (max_id);
  out_int (offset);

  struct mark_read_extra *E = talloc (sizeof (*E));
  E->id = id;
  E->max_id = max_id;

  tglq_send_query (TLS, TLS->DC_working, packet_ptr - packet_buffer, packet_buffer, &mark_read_methods, E, callback, callback_extra);
}

void tgl_do_mark_read (struct tgl_state *TLS, tgl_peer_id_t id, void (*callback)(struct tgl_state *TLS, void *callback_extra, int success), void *callback_extra) {
  if (tgl_get_peer_type (id) == TGL_PEER_USER || tgl_get_peer_type (id) == TGL_PEER_CHAT) {
    tgl_do_messages_mark_read (TLS, id, 0, 0, callback, callback_extra);
    return;
  }
  tgl_peer_t *P = tgl_peer_get (TLS, id);
  if (!P) {
    tgl_set_query_error (TLS, EINVAL, "unknown secret chat");
    if (callback) {
      callback (TLS, callback_extra, 0);
    }
    return;
  }
  assert (tgl_get_peer_type (id) == TGL_PEER_ENCR_CHAT);
  if (P->last) {
    tgl_do_messages_mark_read_encr (TLS, id, P->encr_chat.access_hash, P->last->date, callback, callback_extra);
  } else {
    tgl_do_messages_mark_read_encr (TLS, id, P->encr_chat.access_hash, (int)time (0) - 10, callback, callback_extra);
  }
}
/* }}} */

/* {{{ Get history */
struct get_history_extra {
  struct tgl_message **ML;
  int list_offset;
  int list_size;
  tgl_peer_id_t id;
  int limit;
  int offset;
  int max_id;
};

static void _tgl_do_get_history (struct tgl_state *TLS, struct get_history_extra *E, void (*callback)(struct tgl_state *TLS,void *callback_extra, int success, int size, struct tgl_message *list[]), void *callback_extra);


static int get_history_on_answer (struct tgl_state *TLS, struct query *q, void *D) {
  struct tl_ds_messages_messages *DS_MM = D;

  struct get_history_extra *E = q->extra;

  int n = DS_LVAL (DS_MM->messages->cnt);

  if (E->list_size - E->list_offset < n) {
    int new_list_size = 2 * E->list_size;
    if (new_list_size - E->list_offset < n) {
      new_list_size = n + E->list_offset;
    }
    E->ML = trealloc (E->ML, E->list_size * sizeof (void *), new_list_size * sizeof (void *));
    assert (E->ML);
    E->list_size = new_list_size;
  }

  int i;
  for (i = 0; i < n; i++) {
    E->ML[i + E->list_offset] = tglf_fetch_alloc_message_new (TLS, DS_MM->messages->data[i]);
  }
  E->list_offset += n;
  E->offset += n;
  E->limit -= n;

  int count = DS_LVAL (DS_MM->count);
  if (count >= 0 && E->limit + E->offset >= count) {
    E->limit = count - E->offset;
    if (E->limit < 0) { E->limit = 0; }
  }
  assert (E->limit >= 0);

  for (i = 0; i < DS_LVAL (DS_MM->chats->cnt); i++) {
    tglf_fetch_alloc_chat_new (TLS, DS_MM->chats->data[i]);
  }

  for (i = 0; i < DS_LVAL (DS_MM->users->cnt); i++) {
    tglf_fetch_alloc_user_new (TLS, DS_MM->users->data[i]);
  }


  if (E->limit <= 0 || DS_MM->magic == CODE_messages_messages) {
    if (q->callback) {
      ((void (*)(struct tgl_state *TLS, void *, int, int, struct tgl_message **))q->callback) (TLS, q->callback_extra, 1, E->list_offset, E->ML);
    }
    if (E->list_offset > 0) {
      tgl_do_messages_mark_read (TLS, E->id, E->ML[0]->id, 0, 0, 0);
    }

    tfree (E->ML, sizeof (void *) * E->list_size);
    tfree (E, sizeof (*E));
  } else {
    E->offset = 0;
    E->max_id = E->ML[E->list_offset - 1]->id;
    _tgl_do_get_history (TLS, E, q->callback, q->callback_extra);
  }
  return 0;
}

static int get_history_on_error (struct tgl_state *TLS, struct query *q, int error_code, int error_len, const char *error) {
  tgl_set_query_error (TLS, EPROTO, "RPC_CALL_FAIL %d: %.*s", error_code, error_len, error);

  struct get_history_extra *E = q->extra;
  tfree (E->ML, sizeof (void *) * E->list_size);
  tfree (E, sizeof (*E));

  if (q->callback) {
    ((void (*)(struct tgl_state *TLS, void *, int, int, struct tgl_message **))q->callback) (TLS, q->callback_extra, 0, 0, NULL);
  }
  return 0;
}

static struct query_methods get_history_methods = {
  .on_answer = get_history_on_answer,
  .on_error = get_history_on_error,
  .type = TYPE_TO_PARAM(messages_messages)
};

void tgl_do_get_local_history (struct tgl_state *TLS, tgl_peer_id_t id, int offset, int limit, void (*callback)(struct tgl_state *TLS,void *callback_extra, int success, int size, struct tgl_message *list[]), void *callback_extra) {
  tgl_peer_t *P = tgl_peer_get (TLS, id);
  if (!P || !P->last) {
    tgl_set_query_error (TLS, EINVAL, "unknown peer");
    if (callback) {
      callback (TLS, callback_extra, 0, 0, 0);
    }
    return;
  }
  struct tgl_message *M = P->last;
  int count = 1;
  assert (!M->prev);
  while (count < limit + offset && M->next) {
    M = M->next;
    count ++;
  }
  if (count <= offset) {
    if (callback) {
      callback (TLS, callback_extra, 1, 0, 0);
    }
    return;
  }
  struct tgl_message **ML = talloc (sizeof (void *) * (count - offset));
  M = P->last;
  ML[0] = M;
  count = 1;
  while (count < limit && M->next) {
    M = M->next;
    if (count >= offset) {
      ML[count - offset] = M;
    }
    count ++;
  }

  if (callback) {
    callback (TLS, callback_extra, 1, count - offset, ML);
  }
  tfree (ML, sizeof (void *) * (count) - offset);
}

static void _tgl_do_get_history (struct tgl_state *TLS, struct get_history_extra *E, void (*callback)(struct tgl_state *TLS,void *callback_extra, int success, int size, struct tgl_message *list[]), void *callback_extra) {
  clear_packet ();
  out_int (CODE_messages_get_history);
  out_peer_id (TLS, E->id);
  out_int (E->offset);
  out_int (E->max_id);
  out_int (E->limit);
  tglq_send_query (TLS, TLS->DC_working, packet_ptr - packet_buffer, packet_buffer, &get_history_methods, E, callback, callback_extra);
}

void tgl_do_get_history (struct tgl_state *TLS, tgl_peer_id_t id, int offset, int limit, int offline_mode, void (*callback)(struct tgl_state *TLS, void *callback_extra, int success, int size, struct tgl_message *list[]), void *callback_extra) {
  if (tgl_get_peer_type (id) == TGL_PEER_ENCR_CHAT || offline_mode) {
    tgl_do_get_local_history (TLS, id, offset, limit, callback, callback_extra);
    tgl_do_mark_read (TLS, id, 0, 0);
    return;
  }
  struct get_history_extra *E = talloc0 (sizeof (*E));
  E->id = id;
  E->limit = limit;
  E->offset = offset;
  _tgl_do_get_history (TLS, E, callback, callback_extra);
}
/* }}} */

/* {{{ Get dialogs */
struct get_dialogs_extra {
  tgl_peer_id_t *PL;
  int *UC;
  int *LM;
  int *LRM;

  int list_offset;
  int list_size;
  int limit;
  int offset;
  int max_id;
};

static void _tgl_do_get_dialog_list (struct tgl_state *TLS, struct get_dialogs_extra *E, void (*callback)(struct tgl_state *TLS, void *callback_extra, int success, int size, tgl_peer_id_t peers[], int last_msg_id[], int unread_count[]), void *callback_extra);

static int get_dialogs_on_answer (struct tgl_state *TLS, struct query *q, void *D) {
  struct tl_ds_messages_dialogs *DS_MD = D;

  struct get_dialogs_extra *E = q->extra;

  int dl_size = DS_LVAL (DS_MD->dialogs->cnt);

  if (E->list_offset + dl_size > E->list_size) {
    int new_list_size = E->list_size * 2;
    if (new_list_size < E->list_offset + dl_size) {
      new_list_size = E->list_offset + dl_size;
    }

    E->PL = trealloc (E->PL, E->list_size * sizeof (tgl_peer_id_t), new_list_size * sizeof (tgl_peer_id_t));
    assert (E->PL);
    E->UC = trealloc (E->UC, E->list_size * sizeof (int), new_list_size * sizeof (int));
    assert (E->UC);
    E->LM = trealloc (E->LM, E->list_size * sizeof (int), new_list_size * sizeof (int));
    assert (E->LM);
    E->LRM = trealloc (E->LRM, E->list_size * sizeof (int), new_list_size * sizeof (int));
    assert (E->LRM);

    E->list_size = new_list_size;
  }

  int i;
  for (i = 0; i < dl_size; i++) {
    struct tl_ds_dialog *DS_D = DS_MD->dialogs->data[i];
    E->PL[E->list_offset + i] = tglf_fetch_peer_id_new (TLS, DS_D->peer);
    E->LM[E->list_offset + i] = DS_LVAL (DS_D->top_message);
    E->UC[E->list_offset + i] = DS_LVAL (DS_D->unread_count);
    E->LRM[E->list_offset + i] = DS_LVAL (DS_D->read_inbox_max_id);
  }
  E->list_offset += dl_size;

  for (i = 0; i < DS_LVAL (DS_MD->messages->cnt); i++) {
    tglf_fetch_alloc_message_new (TLS, DS_MD->messages->data[i]);
  }

  for (i = 0; i < DS_LVAL (DS_MD->chats->cnt); i++) {
    tglf_fetch_alloc_chat_new (TLS, DS_MD->chats->data[i]);
  }

  for (i = 0; i < DS_LVAL (DS_MD->users->cnt); i++) {
    tglf_fetch_alloc_user_new (TLS, DS_MD->users->data[i]);
  }

  vlogprintf (E_DEBUG, "dl_size = %d, total = %d\n", dl_size, E->list_offset);
  if (dl_size && E->list_offset < E->limit && DS_MD->magic == CODE_messages_dialogs_slice && E->list_offset < DS_LVAL (DS_MD->count)) {
    E->offset += dl_size;
    _tgl_do_get_dialog_list (TLS, E, q->callback, q->callback_extra);
  } else {
    if (q->callback) {
      ((void (*)(struct tgl_state *TLS, void *, int, int, tgl_peer_id_t *, int *, int *))q->callback) (TLS, q->callback_extra, 1, E->list_offset, E->PL, E->LM, E->UC);
    }
    tfree (E->PL, sizeof (tgl_peer_id_t) * E->list_size);
    tfree (E->UC, 4 * E->list_size);
    tfree (E->LM, 4 * E->list_size);
    tfree (E->LRM, 4 * E->list_size);
    tfree (E, sizeof (*E));
  }

  return 0;
}

static int get_dialogs_on_error (struct tgl_state *TLS, struct query *q, int error_code, int error_len, const char *error) {
  tgl_set_query_error (TLS, EPROTO, "RPC_CALL_FAIL %d: %.*s", error_code, error_len, error);

  struct get_dialogs_extra *E = q->extra;
  tfree (E->PL, sizeof (tgl_peer_id_t) * E->list_size);
  tfree (E->UC, 4 * E->list_size);
  tfree (E->LM, 4 * E->list_size);
  tfree (E->LRM, 4 * E->list_size);
  tfree (E, sizeof (*E));
  if (q->callback) {
    ((void (*)(struct tgl_state *TLS, void *, int, int, tgl_peer_id_t *, int *, int *))q->callback) (TLS, q->callback_extra, 0, 0, NULL, NULL, NULL);
  }
  return 0;
}

static struct query_methods get_dialogs_methods = {
  .on_answer = get_dialogs_on_answer,
  .on_error = get_dialogs_on_error,
  .type = TYPE_TO_PARAM(messages_dialogs)
};

static void _tgl_do_get_dialog_list (struct tgl_state *TLS, struct get_dialogs_extra *E,  void (*callback)(struct tgl_state *TLS, void *callback_extra, int success, int size, tgl_peer_id_t peers[], int last_msg_id[], int unread_count[]), void *callback_extra) {
  clear_packet ();
  out_int (CODE_messages_get_dialogs);
  out_int (E->offset);
  out_int (0);
  out_int (E->limit - E->list_offset);

  tglq_send_query (TLS, TLS->DC_working, packet_ptr - packet_buffer, packet_buffer, &get_dialogs_methods, E, callback, callback_extra);
}

void tgl_do_get_dialog_list (struct tgl_state *TLS, int limit, int offset, void (*callback)(struct tgl_state *TLS, void *callback_extra, int success, int size, tgl_peer_id_t peers[], int last_msg_id[], int unread_count[]), void *callback_extra) {
  struct get_dialogs_extra *E = talloc0 (sizeof (*E));
  E->limit = limit;
  E->offset = offset;
  _tgl_do_get_dialog_list (TLS, E, callback, callback_extra);
}
/* }}} */

int allow_send_os_version = 1;

/* {{{ Send document file */

static void out_peer_id (struct tgl_state *TLS, tgl_peer_id_t id) {
  tgl_peer_t *U;
  switch (tgl_get_peer_type (id)) {
  case TGL_PEER_CHAT:
    out_int (CODE_input_peer_chat);
    out_int (tgl_get_peer_id (id));
    break;
  case TGL_PEER_USER:
    U = tgl_peer_get (TLS, id);
    if (U && U->user.access_hash) {
      out_int (CODE_input_peer_foreign);
      out_int (tgl_get_peer_id (id));
      out_long (U->user.access_hash);
    } else {
      out_int (CODE_input_peer_contact);
      out_int (tgl_get_peer_id (id));
    }
    break;
  default:
    assert (0);
  }
}

static void send_part (struct tgl_state *TLS, struct send_file *f, void *callback, void *callback_extra);
static int send_file_part_on_answer (struct tgl_state *TLS, struct query *q, void *D) {
  send_part (TLS, q->extra, q->callback, q->callback_extra);
  return 0;
}

static int set_photo_on_answer (struct tgl_state *TLS, struct query *q, void *D) {
  if (q->callback) {
    ((void (*)(struct tgl_state *, void *, int))q->callback)(TLS, q->callback_extra, 1);
  }
  return 0;
}

static int send_file_part_on_error (struct tgl_state *TLS, struct query *q, int error_code, int error_len, const char *error) {
  tgl_set_query_error (TLS, EPROTO, "RPC_CALL_FAIL %d: %.*s", error_code, error_len, error);

  struct send_file *f = q->extra;
  tfree_str (f->file_name);
  tfree_str (f->caption);
  if (!f->avatar) {
    if (q->callback) {
      ((void (*)(struct tgl_state *, void *, int, struct tgl_message *))q->callback) (TLS, q->callback_extra, 0, 0);
    }
  } else {
    if (q->callback) {
      ((void (*)(struct tgl_state *, void *, int))q->callback) (TLS, q->callback_extra, 0);
    }
  }
  tfree (f, sizeof (*f));
  return 0;
}

static struct query_methods send_file_part_methods = {
  .on_answer = send_file_part_on_answer,
  .on_error = send_file_part_on_error,
  .type = TYPE_TO_PARAM(bool)
};

static struct query_methods set_photo_methods = {
  .on_answer = set_photo_on_answer,
  .on_error = q_void_on_error,
  .type = TYPE_TO_PARAM(photos_photo)
};

static void send_avatar_end (struct tgl_state *TLS, struct send_file *f, void *callback, void *callback_extra) {
  if (f->avatar > 0) {
    out_int (CODE_messages_edit_chat_photo);
    out_int (f->avatar);
    out_int (CODE_input_chat_uploaded_photo);
    if (f->size < (16 << 20)) {
      out_int (CODE_input_file);
    } else {
      out_int (CODE_input_file_big);
    }
    out_long (f->id);
    out_int (f->part_num);
    out_string ("");
    if (f->size < (16 << 20)) {
      out_string ("");
    }
    out_int (CODE_input_photo_crop_auto);
    tglq_send_query (TLS, TLS->DC_working, packet_ptr - packet_buffer, packet_buffer, &send_msgs_methods, NULL, callback, callback_extra);
  } else {
    out_int (CODE_photos_upload_profile_photo);
    if (f->size < (16 << 20)) {
      out_int (CODE_input_file);
    } else {
      out_int (CODE_input_file_big);
    }
    out_long (f->id);
    out_int (f->part_num);
    char *s = f->file_name + strlen (f->file_name);
    while (s >= f->file_name && *s != '/') { s --;}
    out_string (s + 1);
    if (f->size < (16 << 20)) {
      out_string ("");
    }
    out_string ("profile photo");
    out_int (CODE_input_geo_point_empty);
    out_int (CODE_input_photo_crop_auto);
    tglq_send_query (TLS, TLS->DC_working, packet_ptr - packet_buffer, packet_buffer, &set_photo_methods, 0, callback, callback_extra);
  }
}


static void send_file_unencrypted_end (struct tgl_state *TLS, struct send_file *f, void *callback, void *callback_extra) {
  out_int (CODE_messages_send_media);
  out_int ((f->reply ? 1 : 0));
  out_peer_id (TLS, f->to_id);
  if (f->reply) {
    out_int (f->reply);
  }
  if (f->flags & TGL_SEND_MSG_FLAG_DOCUMENT_PHOTO) {
    out_int (CODE_input_media_uploaded_photo);
  } else {
    if (f->thumb_id > 0) {
      out_int (CODE_input_media_uploaded_thumb_document);
    } else {
      out_int (CODE_input_media_uploaded_document);
    }
  }

  if (f->size < (16 << 20)) {
    out_int (CODE_input_file);
  } else {
    out_int (CODE_input_file_big);
  }

  out_long (f->id);
  out_int (f->part_num);
  char *s = f->file_name + strlen (f->file_name);
  while (s >= f->file_name && *s != '/') { s --;}
  out_string (s + 1);
  if (f->size < (16 << 20)) {
    out_string ("");
  }

  if (!(f->flags & TGL_SEND_MSG_FLAG_DOCUMENT_PHOTO)) {
    out_string (tg_mime_by_filename (f->file_name));

    out_int (CODE_vector);
    if (f->flags & TGLDF_IMAGE) {
      if (f->flags & TGLDF_ANIMATED) {
        out_int (2);
        out_int (CODE_document_attribute_image_size);
        out_int (f->w);
        out_int (f->h);
        out_int (CODE_document_attribute_animated);
      } else {
        out_int (1);
        out_int (CODE_document_attribute_image_size);
        out_int (f->w);
        out_int (f->h);
      }
    } else if (f->flags & TGLDF_AUDIO) {
      out_int (2);
      out_int (CODE_document_attribute_audio);
      out_int (f->duration);
      out_int (CODE_document_attribute_filename);
      out_string (s + 1);
    } else if (f->flags & TGLDF_VIDEO) {
      out_int (2);
      out_int (CODE_document_attribute_video);
      out_int (f->duration);
      out_int (f->w);
      out_int (f->h);
      out_int (CODE_document_attribute_filename);
      out_string (s + 1);
    } else if (f->flags & TGLDF_STICKER) {
      out_int (1);
      out_int (CODE_document_attribute_sticker);
    } else {
      out_int (1);
      out_int (CODE_document_attribute_filename);
      out_string (s + 1);
    }

    if (f->thumb_id > 0) {
      out_int (CODE_input_file);
      out_long (f->thumb_id);
      out_int (1);
      out_string ("thumb.jpg");
      out_string ("");
    }
  } else {
    out_string (f->caption ? f->caption : "");
  }


  struct messages_send_extra *E = talloc0 (sizeof (*E));
  tglt_secure_random (&E->id, 8);
  out_long (E->id);

  tglq_send_query (TLS, TLS->DC_working, packet_ptr - packet_buffer, packet_buffer, &send_msgs_methods, E, callback, callback_extra);
  tfree_str (f->file_name);
  tfree_str (f->caption);
  tfree (f, sizeof (*f));
}

static void send_file_end (struct tgl_state *TLS, struct send_file *f, void *callback, void *callback_extra) {
  TLS->cur_uploaded_bytes -= f->size;
  TLS->cur_uploading_bytes -= f->size;
  clear_packet ();

  if (f->avatar) {
    send_avatar_end (TLS, f, callback, callback_extra);
    return;
  }
  if (!f->encr) {
    send_file_unencrypted_end (TLS, f, callback, callback_extra);
    return;
  }
  send_file_encrypted_end (TLS, f, callback, callback_extra);
  return;
}

static void send_part (struct tgl_state *TLS, struct send_file *f, void *callback, void *callback_extra) {
  if (f->fd >= 0) {
    if (!f->part_num) {
      TLS->cur_uploading_bytes += f->size;
    }
    clear_packet ();
    if (f->size < (16 << 20)) {
      out_int (CODE_upload_save_file_part);
      out_long (f->id);
      out_int (f->part_num ++);
    } else {
      out_int (CODE_upload_save_big_file_part);
      out_long (f->id);
      out_int (f->part_num ++);
      out_int ((f->size + f->part_size - 1) / f->part_size);
    }
    static char buf[512 << 10];
#ifdef _WIN32
	int x = _read(f->fd, buf, f->part_size);
#else
    int x = read (f->fd, buf, f->part_size);
#endif
    assert (x > 0);
    f->offset += x;
    TLS->cur_uploaded_bytes += x;

    if (f->encr) {
      if (x & 15) {
        assert (f->offset == f->size);
        tglt_secure_random (buf + x, (-x) & 15);
        x = (x + 15) & ~15;
      }

      AES_KEY aes_key;
      AES_set_encrypt_key (f->key, 256, &aes_key);
      AES_ige_encrypt ((void *)buf, (void *)buf, x, &aes_key, f->iv, 1);
      memset (&aes_key, 0, sizeof (aes_key));
    }
    out_cstring (buf, x);
    vlogprintf (E_DEBUG, "offset=%"_PRINTF_INT64_"d size=%"_PRINTF_INT64_"d\n", f->offset, f->size);
    if (f->offset == f->size) {
      close (f->fd);
      f->fd = -1;
    } else {
      assert (f->part_size == x);
    }
    //update_prompt ();
    tglq_send_query (TLS, TLS->DC_working, packet_ptr - packet_buffer, packet_buffer, &send_file_part_methods, f, callback, callback_extra);
  } else {
    send_file_end (TLS, f, callback, callback_extra);
  }
}

static void send_file_thumb (struct tgl_state *TLS, struct send_file *f, const void *thumb_data, int thumb_len, void *callback, void *callback_extra) {
  clear_packet ();
  f->thumb_id = lrand48 () * (1ll << 32) + lrand48 ();
  out_int (CODE_upload_save_file_part);
  out_long (f->thumb_id);
  out_int (0);
  out_cstring ((void *)thumb_data, thumb_len);
  tglq_send_query (TLS, TLS->DC_working, packet_ptr - packet_buffer, packet_buffer, &send_file_part_methods, f, callback, callback_extra);
}


static void _tgl_do_send_photo (struct tgl_state *TLS, tgl_peer_id_t to_id, const char *file_name, int avatar, int w, int h, int duration, const void *thumb_data, int thumb_len, const char *caption, int caption_len, unsigned long long flags, void (*callback)(struct tgl_state *TLS, void *callback_extra, int success, struct tgl_message *M), void *callback_extra) {
#if defined(_MSC_VER) && _MSC_VER >= 1400
  int fd = 0;
  errno_t err = _sopen_s(&fd, file_name, _O_RDONLY | _O_BINARY, _SH_DENYNO, _S_IREAD | _S_IWRITE);
  if(err != 0) {
    tgl_set_query_error (TLS, EBADF, "Can not open file: %s", GetErrnoStr (errno));
#elif defined(WIN32) || defined(_WIN32)
  int fd = open(file_name, O_RDONLY | O_BINARY);
  if (fd < 0) {
    tgl_set_query_error (TLS, EBADF, "Can not open file: %s", GetErrnoStr (errno));
#else
  int fd = open (file_name, O_RDONLY);
  if (fd < 0) {
    tgl_set_query_error (TLS, EBADF, "Can not open file: %m");
#endif
    if (!avatar) {
      if (callback) {
        callback (TLS, callback_extra, 0, 0);
      }
    } else {
      if (callback) {
        ((void (*)(struct tgl_state *, void *, int))callback) (TLS, callback_extra, 0);
      }
    }
    return;
  }
  struct stat buf;
  fstat (fd, &buf);
  long long size = buf.st_size;
  if (size <= 0) {
    tgl_set_query_error (TLS, EBADF, "File is empty");
    close (fd);
    if (!avatar) {
      if (callback) {
        callback (TLS, callback_extra, 0, 0);
      }
    } else {
      if (callback) {
        ((void (*)(struct tgl_state *, void *, int))callback) (TLS, callback_extra, 0);
      }
    }
    return;
  }
  struct send_file *f = talloc0 (sizeof (*f));
  f->fd = fd;
  f->size = size;
  f->offset = 0;
  f->part_num = 0;
  f->avatar = avatar;
  f->reply = flags >> 32;
  int tmp = ((size + 2999) / 3000);
  f->part_size = (1 << 14);
  while (f->part_size < tmp) {
    f->part_size *= 2;
  }
  f->flags = flags;

  if (f->part_size > (512 << 10)) {
    close (fd);
    tgl_set_query_error (TLS, E2BIG, "File is too big");
    tfree (f, sizeof (*f));
    if (!avatar) {
      if (callback) {
        callback (TLS, callback_extra, 0, 0);
      }
    } else {
      if (callback) {
        ((void (*)(struct tgl_state *, void *, int))callback) (TLS, callback_extra, 0);
      }
    }
    return;
  }

  tglt_secure_random (&f->id, 8);
  f->to_id = to_id;
  f->flags = flags;
  f->file_name = tstrdup (file_name);
  f->w = w;
  f->h = h;
  f->duration = duration;
  f->caption = caption ? tstrdup (caption) : tstrdup ("");

  if (tgl_get_peer_type (f->to_id) == TGL_PEER_ENCR_CHAT) {
    f->encr = 1;
    f->iv = talloc (32);
    tglt_secure_random (f->iv, 32);
    f->init_iv = talloc (32);
    memcpy (f->init_iv, f->iv, 32);
    f->key = talloc (32);
    tglt_secure_random (f->key, 32);
  }

  if (!f->encr && f->flags != -1 && thumb_len > 0) {
    send_file_thumb (TLS, f, thumb_data, thumb_len, callback, callback_extra);
  } else {
    send_part (TLS, f, callback, callback_extra);
  }
}

void tgl_do_send_document (struct tgl_state *TLS, tgl_peer_id_t to_id, const char *file_name, const char *caption, int caption_len, unsigned long long flags, void (*callback)(struct tgl_state *TLS, void *callback_extra, int success, struct tgl_message *M), void *callback_extra) {
  if (flags & TGL_SEND_MSG_FLAG_DOCUMENT_AUTO) {
    char *mime_type = tg_mime_by_filename (file_name);
    if (strcmp (mime_type, "image/gif") == 0) {
      flags |= TGL_SEND_MSG_FLAG_DOCUMENT_ANIMATED;
    } else if (!memcmp (mime_type, "image/", 6)) {
      flags |= TGL_SEND_MSG_FLAG_DOCUMENT_PHOTO;
    } else if (!memcmp (mime_type, "video/", 6)) {
      flags |= TGLDF_VIDEO;
    } else if (!memcmp (mime_type, "audio/", 6)) {
      flags |= TGLDF_AUDIO;
    }
  }
  _tgl_do_send_photo (TLS, to_id, file_name, 0, 100, 100, 100, 0, 0, caption, caption_len, flags, callback, callback_extra);
}

void tgl_do_reply_document (struct tgl_state *TLS, int reply_id, const char *file_name, const char *caption, int caption_len, unsigned long long flags, void (*callback)(struct tgl_state *TLS, void *callback_extra, int success, struct tgl_message *M), void *callback_extra) {
  struct tgl_message *M = tgl_message_get (TLS, reply_id);
  if (!M || !(M->flags & TGLMF_CREATED) || (M->flags & TGLMF_ENCRYPTED)) {
    if (!M || !(M->flags & TGLMF_CREATED)) {
      tgl_set_query_error (TLS, EINVAL, "unknown message");
    } else {
      tgl_set_query_error (TLS, EINVAL, "can not reply on message from secret chat");
    }
    if (callback) {
      callback (TLS, callback_extra, 0, 0);
    }
    return;
  }

  tgl_peer_id_t id = M->to_id;
  if (tgl_get_peer_type (id) == TGL_PEER_USER && tgl_get_peer_id (id) == TLS->our_id) {
    id = M->from_id;
  }

  tgl_do_send_document (TLS, id, file_name, caption, caption_len, flags | TGL_SEND_MSG_FLAG_REPLY (reply_id), callback, callback_extra);
}

void tgl_do_set_chat_photo (struct tgl_state *TLS, tgl_peer_id_t chat_id, const char *file_name, void (*callback)(struct tgl_state *TLS,void *callback_extra, int success), void *callback_extra) {
  assert (tgl_get_peer_type (chat_id) == TGL_PEER_CHAT);
  _tgl_do_send_photo (TLS, chat_id, file_name, tgl_get_peer_id (chat_id), 0, 0, 0, 0, 0, NULL, 0, TGL_SEND_MSG_FLAG_DOCUMENT_PHOTO, (void *)callback, callback_extra);
}

void tgl_do_set_profile_photo (struct tgl_state *TLS, const char *file_name, void (*callback)(struct tgl_state *TLS, void *callback_extra, int success), void *callback_extra) {
  _tgl_do_send_photo (TLS, TGL_MK_USER(TLS->our_id), file_name, -1, 0, 0, 0, 0, 0, NULL, 0, TGL_SEND_MSG_FLAG_DOCUMENT_PHOTO, (void *)callback, callback_extra);
}
/* }}} */

/* {{{ Profile name */

int set_profile_name_on_answer (struct tgl_state *TLS, struct query *q, void *D) {
  struct tl_ds_user *DS_U = D;
  struct tgl_user *U = tglf_fetch_alloc_user_new (TLS, DS_U);
  if (q->callback) {
    ((void (*)(struct tgl_state *, void *, int, struct tgl_user  *))q->callback) (TLS, q->callback_extra, 1, U);
  }
  return 0;
}

static struct query_methods set_profile_name_methods = {
  .on_answer = set_profile_name_on_answer,
  .on_error = q_ptr_on_error,
  .type = TYPE_TO_PARAM(user)
};

void tgl_do_set_profile_name (struct tgl_state *TLS, const char *first_name, int first_name_len, const char *last_name, int last_name_len, void (*callback)(struct tgl_state *TLS, void *callback_extra, int success, struct tgl_user *U), void *callback_extra) {
  clear_packet ();
  out_int (CODE_account_update_profile);
  out_cstring (first_name, first_name_len);
  out_cstring (last_name, last_name_len);

  tglq_send_query (TLS, TLS->DC_working, packet_ptr - packet_buffer, packet_buffer, &set_profile_name_methods, 0, callback, callback_extra);
}

void tgl_do_set_username (struct tgl_state *TLS, const char *username, int username_len, void (*callback)(struct tgl_state *TLS, void *callback_extra, int success, struct tgl_user *U), void *callback_extra) {
  clear_packet ();
  out_int (CODE_account_update_username);
  out_cstring (username, username_len);

  tglq_send_query (TLS, TLS->DC_working, packet_ptr - packet_buffer, packet_buffer, &set_profile_name_methods, 0, callback, callback_extra);
}
/* }}} */

/* {{{ Contacts search */

int contact_search_on_answer (struct tgl_state *TLS, struct query *q, void *D) {
  struct tl_ds_user *DS_U = D;

  struct tgl_user *U = tglf_fetch_alloc_user_new (TLS, DS_U);

  if (q->callback) {
    ((void (*)(struct tgl_state *,void *, int, struct tgl_user  *))q->callback) (TLS, q->callback_extra, 1, U);
  }

  return 0;
}

static struct query_methods contact_search_methods = {
  .on_answer = contact_search_on_answer,
  .on_error = q_list_on_error,
  .type = TYPE_TO_PARAM(user)
};

void tgl_do_contact_search (struct tgl_state *TLS, const char *name, int name_len, void (*callback)(struct tgl_state *TLS, void *callback_extra, int success, struct tgl_user *U), void *callback_extra) {
  clear_packet ();
  out_int (CODE_contacts_resolve_username);
  out_cstring (name, name_len);

  tglq_send_query (TLS, TLS->DC_working, packet_ptr - packet_buffer, packet_buffer, &contact_search_methods, 0, callback, callback_extra);
}
/* }}} */

/* {{{ Forward */

static int send_msgs_on_answer (struct tgl_state *TLS, struct query *q, void *D) {
  tglu_work_any_updates_new (TLS, 1, D);
  tglu_work_any_updates_new (TLS, 0, D);

  struct messages_send_extra *E = q->extra;

  if (!E) {
    if (q->callback) {
      ((void (*)(struct tgl_state *, void *, int))q->callback) (TLS, q->callback_extra, 1);
    }
  } else if (E->multi) {
    struct tgl_message **ML = talloc (sizeof (void *) * E->count);
    int count = E->count;
    int i;
    for (i = 0; i < count; i++) {
      int y = tgls_get_local_by_random (TLS, E->list[i]);
      ML[i] = tgl_message_get (TLS, y);
    }
    tfree (E->list, sizeof (long long) * count);
    tfree (E, sizeof (*E));
    if (q->callback) {
      ((void (*)(struct tgl_state *, void *, int, int, struct tgl_message **))q->callback) (TLS, q->callback_extra, 1, count, ML);
    }
    tfree (ML, sizeof (void *) * count);
  } else {
    int y = tgls_get_local_by_random (TLS, E->id);
    struct tgl_message *M = tgl_message_get (TLS, y);
    tfree (E, sizeof (*E));
    if (q->callback) {
      ((void (*)(struct tgl_state *, void *, int, struct tgl_message *))q->callback) (TLS, q->callback_extra, 1, M);
    }
  }
  return 0;
}

static int send_msgs_on_error (struct tgl_state *TLS, struct query *q, int error_code, int error_len, const char *error) {
  tgl_set_query_error (TLS, EPROTO, "RPC_CALL_FAIL %d: %.*s", error_code, error_len, error);
  struct messages_send_extra *E = q->extra;

  if (!E) {
    if (q->callback) {
      ((void (*)(struct tgl_state *, void *, int))q->callback) (TLS, q->callback_extra, 0);
    }
  } else if (E->multi) {
    int count = E->count;
    tfree (E->list, sizeof (long long) * count);
    tfree (E, sizeof (*E));
    if (q->callback) {
      ((void (*)(struct tgl_state *, void *, int, int, struct tgl_message **))q->callback) (TLS, q->callback_extra, 0, 0, NULL);
    }
  } else {
    tfree (E, sizeof (*E));
    if (q->callback) {
      ((void (*)(struct tgl_state *, void *, int, struct tgl_message *))q->callback) (TLS, q->callback_extra, 0, NULL);
    }
  }
  return 0;
}

static struct query_methods send_msgs_methods = {
  .on_answer = send_msgs_on_answer,
  .on_error = send_msgs_on_error,
  .type = TYPE_TO_PARAM(updates)
};

void tgl_do_forward_messages (struct tgl_state *TLS, tgl_peer_id_t id, int n, const int ids[], unsigned long long flags, void (*callback)(struct tgl_state *TLS, void *callback_extra, int success, int count, struct tgl_message *ML[]), void *callback_extra) {
  if (tgl_get_peer_type (id) == TGL_PEER_ENCR_CHAT) {
    tgl_set_query_error (TLS, EINVAL, "can not forward messages to secret chats");
    if (callback) {
      callback (TLS, callback_extra, 0, 0, 0);
    }
    return;
  }
  clear_packet ();
  out_int (CODE_messages_forward_messages);
  out_peer_id (TLS, id);
  out_int (CODE_vector);
  out_int (n);
  int i;
  for (i = 0; i < n; i++) {
    out_int (ids[i]);
  }

  struct messages_send_extra *E = talloc0 (sizeof (*E));
  E->multi = 1;
  E->count = n;
  E->list = talloc (sizeof (long long) * n);
  out_int (CODE_vector);
  out_int (n);
  for (i = 0; i < n; i++) {
    tglt_secure_random (&E->list[i], 8);
    out_long (E->list[i]);
  }

  tglq_send_query (TLS, TLS->DC_working, packet_ptr - packet_buffer, packet_buffer, &send_msgs_methods, E, callback, callback_extra);
}

void tgl_do_forward_message (struct tgl_state *TLS, tgl_peer_id_t id, int msg_id, unsigned long long flags, void (*callback)(struct tgl_state *TLS, void *callback_extra, int success, struct tgl_message *M), void *callback_extra) {
  if (tgl_get_peer_type (id) == TGL_PEER_ENCR_CHAT) {
    tgl_set_query_error (TLS, EINVAL, "can not forward messages to secret chats");
    if (callback) {
      callback (TLS, callback_extra, 0, 0);
    }
    return;
  }
  clear_packet ();
  out_int (CODE_messages_forward_message);
  out_peer_id (TLS, id);
  out_int (msg_id);

  struct messages_send_extra *E = talloc0 (sizeof (*E));
  tglt_secure_random (&E->id, 8);
  out_long (E->id);
  tglq_send_query (TLS, TLS->DC_working, packet_ptr - packet_buffer, packet_buffer, &send_msgs_methods, E, callback, callback_extra);
}

void tgl_do_send_contact (struct tgl_state *TLS, tgl_peer_id_t id, const char *phone, int phone_len, const char *first_name, int first_name_len, const char *last_name, int last_name_len, unsigned long long flags, void (*callback)(struct tgl_state *TLS, void *callback_extra, int success, struct tgl_message *M), void *callback_extra) {
  if (tgl_get_peer_type (id) == TGL_PEER_ENCR_CHAT) {
    tgl_set_query_error (TLS, EINVAL, "can not send contact to secret chat");
    if (callback) {
      callback (TLS, callback_extra, 0, 0);
    }
    return;
  }

  int reply_id = flags >> 32;

  clear_packet ();
  out_int (CODE_messages_send_media);
  out_int (reply_id ? 1 : 0);
  if (reply_id) { out_int (reply_id); }
  out_peer_id (TLS, id);
  out_int (CODE_input_media_contact);
  out_cstring (phone, phone_len);
  out_cstring (first_name, first_name_len);
  out_cstring (last_name, last_name_len);

  struct messages_send_extra *E = talloc0 (sizeof (*E));
  tglt_secure_random (&E->id, 8);
  out_long (E->id);

  tglq_send_query (TLS, TLS->DC_working, packet_ptr - packet_buffer, packet_buffer, &send_msgs_methods, E, callback, callback_extra);
}


void tgl_do_reply_contact (struct tgl_state *TLS, int reply_id, const char *phone, int phone_len, const char *first_name, int first_name_len, const char *last_name, int last_name_len, unsigned long long flags, void (*callback)(struct tgl_state *TLS, void *callback_extra, int success, struct tgl_message *M), void *callback_extra) {
  struct tgl_message *M = tgl_message_get (TLS, reply_id);
  if (!M || !(M->flags & TGLMF_CREATED) || (M->flags & TGLMF_ENCRYPTED)) {
    if (!M || !(M->flags & TGLMF_CREATED)) {
      tgl_set_query_error (TLS, EINVAL, "unknown message");
    } else {
      tgl_set_query_error (TLS, EINVAL, "can not reply on message from secret chat");
    }
    if (callback) {
      callback (TLS, callback_extra, 0, 0);
    }
    return;
  }

  tgl_peer_id_t id = M->to_id;
  if (tgl_get_peer_type (id) == TGL_PEER_USER && tgl_get_peer_id (id) == TLS->our_id) {
    id = M->from_id;
  }

  tgl_do_send_contact (TLS, id, phone, phone_len, first_name, first_name_len, last_name, last_name_len, flags | TGL_SEND_MSG_FLAG_REPLY (reply_id), callback, callback_extra);
}

void tgl_do_forward_media (struct tgl_state *TLS, tgl_peer_id_t id, int n, unsigned long long flags, void (*callback)(struct tgl_state *TLS, void *callback_extra, int success, struct tgl_message *M), void *callback_extra) {
  if (tgl_get_peer_type (id) == TGL_PEER_ENCR_CHAT) {
    tgl_set_query_error (TLS, EINVAL, "can not forward messages to secret chats");
    if (callback) {
      callback (TLS, callback_extra, 0, 0);
    }
    return;
  }
  struct tgl_message *M = tgl_message_get (TLS, n);
  if (!M || !(M->flags & TGLMF_CREATED) || (M->flags & TGLMF_ENCRYPTED)) {
    if (!M || !(M->flags & TGLMF_CREATED)) {
      tgl_set_query_error (TLS, EINVAL, "unknown message");
    } else {
      tgl_set_query_error (TLS, EINVAL, "can not forward message from secret chat");
    }
    if (callback) {
      callback (TLS, callback_extra, 0, 0);
    }
    return;
  }
  if (M->media.type != tgl_message_media_photo && M->media.type != tgl_message_media_document) {
    tgl_set_query_error (TLS, EINVAL, "can only forward photo/document");
    if (callback) {
      callback (TLS, callback_extra, 0, 0);
    }
    return;
  }
  clear_packet ();
  out_int (CODE_messages_send_media);
  out_peer_id (TLS, id);
  switch (M->media.type) {
  case tgl_message_media_photo:
    assert (M->media.photo);
    out_int (CODE_input_media_photo);
    out_int (CODE_input_photo);
    out_long (M->media.photo->id);
    out_long (M->media.photo->access_hash);
    break;
  case tgl_message_media_document:
    assert (M->media.document);
    out_int (CODE_input_media_document);
    out_int (CODE_input_document);
    out_long (M->media.document->id);
    out_long (M->media.document->access_hash);
    break;
  default:
    assert (0);
  }

  struct messages_send_extra *E = talloc0 (sizeof (*E));
  tglt_secure_random (&E->id, 8);
  out_long (E->id);

  tglq_send_query (TLS, TLS->DC_working, packet_ptr - packet_buffer, packet_buffer, &send_msgs_methods, E, callback, callback_extra);
}
/* }}} */

/* {{{ Send location */

void tgl_do_send_location (struct tgl_state *TLS, tgl_peer_id_t id, double latitude, double longitude, unsigned long long flags, void (*callback)(struct tgl_state *TLS, void *callback_extra, int success, struct tgl_message *M), void *callback_extra) {
  if (tgl_get_peer_type (id) == TGL_PEER_ENCR_CHAT) {
    tgl_do_send_location_encr (TLS, id, latitude, longitude, flags, callback, callback_extra);
  } else {
    int reply_id = flags >> 32;
    clear_packet ();
    out_int (CODE_messages_send_media);
    out_int (reply_id ? 1 : 0);
    if (reply_id) { out_int (reply_id); }
    out_peer_id (TLS, id);
    out_int (CODE_input_media_geo_point);
    out_int (CODE_input_geo_point);
    out_double (latitude);
    out_double (longitude);

    struct messages_send_extra *E = talloc0 (sizeof (*E));
    tglt_secure_random (&E->id, 8);
    out_long (E->id);

    tglq_send_query (TLS, TLS->DC_working, packet_ptr - packet_buffer, packet_buffer, &send_msgs_methods, E, callback, callback_extra);
  }
}

void tgl_do_reply_location (struct tgl_state *TLS, int reply_id, double latitude, double longitude, unsigned long long flags, void (*callback)(struct tgl_state *TLS, void *callback_extra, int success, struct tgl_message *M), void *callback_extra) {
  struct tgl_message *M = tgl_message_get (TLS, reply_id);
  if (!M || !(M->flags & TGLMF_CREATED) || (M->flags & TGLMF_ENCRYPTED)) {
    if (!M || !(M->flags & TGLMF_CREATED)) {
      tgl_set_query_error (TLS, EINVAL, "unknown message");
    } else {
      tgl_set_query_error (TLS, EINVAL, "can not forward message from secret chat");
    }
    if (callback) {
      callback (TLS, callback_extra, 0, 0);
    }
    return;
  }

  tgl_peer_id_t id = M->to_id;
  if (tgl_get_peer_type (id) == TGL_PEER_USER && tgl_get_peer_id (id) == TLS->our_id) {
    id = M->from_id;
  }

  tgl_do_send_location (TLS, id, latitude, longitude, flags | TGL_SEND_MSG_FLAG_REPLY (reply_id), callback, callback_extra);
}
/* }}} */

/* {{{ Rename chat */

void tgl_do_rename_chat (struct tgl_state *TLS, tgl_peer_id_t id, const char *name, int name_len, void (*callback)(struct tgl_state *TLS, void *callback_extra, int success), void *callback_extra) {
  clear_packet ();
  out_int (CODE_messages_edit_chat_title);
  assert (tgl_get_peer_type (id) == TGL_PEER_CHAT);
  out_int (tgl_get_peer_id (id));
  out_cstring (name, name_len);
  tglq_send_query (TLS, TLS->DC_working, packet_ptr - packet_buffer, packet_buffer, &send_msgs_methods, 0, callback, callback_extra);
}
/* }}} */

/* {{{ Chat info */

static int chat_info_on_answer (struct tgl_state *TLS, struct query *q, void *D) {
  struct tgl_chat *C = tglf_fetch_alloc_chat_full_new (TLS, D);
  //print_chat_info (C);
  if (q->callback) {
    ((void (*)(struct tgl_state *, void *, int, struct tgl_chat *))q->callback) (TLS, q->callback_extra, 1, C);
  }
  return 0;
}

static struct query_methods chat_info_methods = {
  .on_answer = chat_info_on_answer,
  .on_error = q_ptr_on_error,
  .type = TYPE_TO_PARAM(messages_chat_full)
};

void tgl_do_get_chat_info (struct tgl_state *TLS, tgl_peer_id_t id, int offline_mode, void (*callback)(struct tgl_state *TLS, void *callback_extra, int success, struct tgl_chat *C), void *callback_extra) {
  if (offline_mode) {
    tgl_peer_t *C = tgl_peer_get (TLS, id);
    if (!C) {
      tgl_set_query_error (TLS, EINVAL, "unknown chat id");
      if (callback) {
        callback (TLS, callback_extra, 0, 0);
      }
    } else {
      //print_chat_info (&C->chat);
      if (callback) {
        callback (TLS, callback_extra, 1, &C->chat);
      }
    }
    return;
  }
  clear_packet ();
  out_int (CODE_messages_get_full_chat);
  assert (tgl_get_peer_type (id) == TGL_PEER_CHAT);
  out_int (tgl_get_peer_id (id));
  tglq_send_query (TLS, TLS->DC_working, packet_ptr - packet_buffer, packet_buffer, &chat_info_methods, 0, callback, callback_extra);
}
/* }}} */

/* {{{ User info */

static int user_info_on_answer (struct tgl_state *TLS, struct query *q, void *D) {
  struct tgl_user *U = tglf_fetch_alloc_user_full_new (TLS, D);
  if (q->callback) {
    ((void (*)(struct tgl_state *, void *, int, struct tgl_user *))q->callback) (TLS, q->callback_extra, 1, U);
  }
  return 0;
}

static struct query_methods user_info_methods = {
  .on_answer = user_info_on_answer,
  .on_error = q_ptr_on_error,
  .type = TYPE_TO_PARAM(user_full)
};

void tgl_do_get_user_info (struct tgl_state *TLS, tgl_peer_id_t id, int offline_mode, void (*callback)(struct tgl_state *TLS, void *callback_extra, int success, struct tgl_user *U), void *callback_extra) {
  if (tgl_get_peer_type (id) != TGL_PEER_USER) {
    tgl_set_query_error (TLS, EINVAL, "id should be user id");
    if (callback) {
      callback (TLS, callback_extra, 0, NULL);
    }
    return;
  }
  if (offline_mode) {
    tgl_peer_t *C = tgl_peer_get (TLS, id);
    if (!C) {
      tgl_set_query_error (TLS, EINVAL, "unknown user id");
      if (callback) {
        callback (TLS, callback_extra, 0, 0);
      }
    } else {
      if (callback) {
        callback (TLS, callback_extra, 1, &C->user);
      }
    }
    return;
  }
  clear_packet ();
  out_int (CODE_users_get_full_user);
  assert (tgl_get_peer_type (id) == TGL_PEER_USER);
  tgl_peer_t *U = tgl_peer_get (TLS, id);
  if (U && U->user.access_hash) {
    out_int (CODE_input_user_foreign);
    out_int (tgl_get_peer_id (id));
    out_long (U->user.access_hash);
  } else {
    out_int (CODE_input_user_contact);
    out_int (tgl_get_peer_id (id));
  }
  tglq_send_query (TLS, TLS->DC_working, packet_ptr - packet_buffer, packet_buffer, &user_info_methods, 0, callback, callback_extra);
}

static void resend_query_cb (struct tgl_state *TLS, void *_q, int success) {
  assert (success);

  bl_do_dc_signed (TLS, TLS->DC_working->id);

  struct query *q = _q;

  clear_packet ();
  out_int (CODE_users_get_full_user);
  out_int (CODE_input_user_self);
  tglq_send_query (TLS, q->DC, packet_ptr - packet_buffer, packet_buffer, &user_info_methods, 0, q->callback, q->callback_extra);

  tfree (q->data, 4 * q->data_len);
  TLS->timer_methods->free (q->ev);
  tfree (q, sizeof (*q));
}
/* }}} */

/* {{{ Load photo/video */
struct download {
  int offset;
  int size;
  long long volume;
  long long secret;
  long long access_hash;
  int local_id;
  int dc;
  int next;
  int fd;
  char *name;
  char *ext;
  long long id;
  unsigned char *iv;
  unsigned char *key;
  int type;
  int refcnt;
};


static void end_load (struct tgl_state *TLS, struct download *D, void *callback, void *callback_extra) {
  TLS->cur_downloading_bytes -= D->size;
  TLS->cur_downloaded_bytes -= D->size;

  if (D->fd >= 0) {
    close (D->fd);
  }

  if (callback) {
    ((void (*)(struct tgl_state *, void *, int, char *))callback) (TLS, callback_extra, 1, D->name);
  }

  if (D->iv) {
    tfree_secure (D->iv, 32);
  }
  tfree_str (D->name);
  tfree (D, sizeof (*D));
}

static void load_next_part (struct tgl_state *TLS, struct download *D, void *callback, void *callback_extra);
static int download_on_answer (struct tgl_state *TLS, struct query *q, void *DD) {
  struct tl_ds_upload_file *DS_UF = DD;

  struct download *D = q->extra;
  if (D->fd == -1) {
#if defined(_MSC_VER) && _MSC_VER >= 1400
    errno_t err = _sopen_s(&D->fd, D->name, O_CREAT | O_WRONLY | _O_BINARY, _SH_DENYNO, _S_IREAD | _S_IWRITE);
    if (err != 0) {
      tgl_set_query_error (TLS, EBADF, "Can not open file for writing: %s", GetErrnoStr (errno));
#elif defined(WIN32) || defined(_WIN32)
    D->fd = open(D->name, O_CREAT | O_WRONLY | O_BINARY, 0640);
    if (D->fd < 0) {
      tgl_set_query_error (TLS, EBADF, "Can not open file for writing: %s", GetErrnoStr (errno));
#else
    D->fd = open (D->name, O_CREAT | O_WRONLY, 0640);
    if (D->fd < 0) {
      tgl_set_query_error (TLS, EBADF, "Can not open file for writing: %m");
#endif
      if (q->callback) {
        ((void (*)(struct tgl_state *, void *, int, char *))q->callback) (TLS, q->callback_extra, 0, NULL);
      }

      if (D->iv) {
        tfree_secure (D->iv, 32);
      }
      tfree_str (D->name);
      if (D->ext) {
        tfree_str (D->ext);
      }
      tfree (D, sizeof (*D));
      return 0;
    }
  }

  int len = DS_UF->bytes->len;
  TLS->cur_downloaded_bytes += len;
  //update_prompt ();

  if (D->iv) {
    assert (!(len & 15));
    void *ptr = DS_UF->bytes->data;

    AES_KEY aes_key;
    AES_set_decrypt_key (D->key, 256, &aes_key);
    AES_ige_encrypt (ptr, ptr, len, &aes_key, D->iv, 0);
    memset (&aes_key, 0, sizeof (aes_key));
    if (len > D->size - D->offset) {
      len = D->size - D->offset;
    }
    assert (write (D->fd, ptr, len) == len);
  } else {
    assert (write (D->fd, DS_UF->bytes->data, len) == len);
  }

  D->offset += len;
  D->refcnt --;
  if (D->offset < D->size) {
    load_next_part (TLS, D, q->callback, q->callback_extra);
    return 0;
  } else {
    if (!D->refcnt) {
      end_load (TLS, D, q->callback, q->callback_extra);
    }
    return 0;
  }
}

static int download_on_error (struct tgl_state *TLS, struct query *q, int error_code, int error_len, const char *error) {
  tgl_set_query_error (TLS, EPROTO, "RPC_CALL_FAIL %d: %.*s", error_code, error_len, error);

  struct download *D = q->extra;
  if (D->fd >= 0) {
    close (D->fd);
  }

  if (q->callback) {
    ((void (*)(struct tgl_state *, void *, int, char *))q->callback) (TLS, q->callback_extra, 0, NULL);
  }

  if (D->iv) {
    tfree_secure (D->iv, 32);
  }
  tfree_str (D->name);
  if (D->ext) {
    tfree_str (D->ext);
  }
  tfree (D, sizeof (*D));
  return 0;
}

static struct query_methods download_methods = {
  .on_answer = download_on_answer,
  .on_error = download_on_error,
  .type = TYPE_TO_PARAM(upload_file)
};

static void load_next_part (struct tgl_state *TLS, struct download *D, void *callback, void *callback_extra) {
  if (!D->offset) {
    static char buf[PATH_MAX];
    int l;
    if (!D->id) {
      l = tsnprintf (buf, sizeof (buf), "%s/download_%"_PRINTF_INT64_"d_%d.jpg", TLS->downloads_directory, D->volume, D->local_id);
    } else {
      if (D->ext) {
        l = tsnprintf (buf, sizeof (buf), "%s/download_%"_PRINTF_INT64_"d.%s", TLS->downloads_directory, D->id, D->ext);
      } else {
        l = tsnprintf (buf, sizeof (buf), "%s/download_%"_PRINTF_INT64_"d", TLS->downloads_directory, D->id);
      }
    }
    if (l >= (int) sizeof (buf)) {
      vlogprintf (E_ERROR, "Download filename is too long");
      assert (0);
    }
    D->name = tstrdup (buf);
    struct stat st;
    if (stat (buf, &st) >= 0) {
      D->offset = st.st_size;
      if (D->offset >= D->size) {
        TLS->cur_downloading_bytes += D->size;
        TLS->cur_downloaded_bytes += D->offset;
        vlogprintf (E_NOTICE, "Already downloaded\n");
        end_load (TLS, D, callback, callback_extra);
        return;
      }
    }

    TLS->cur_downloading_bytes += D->size;
    TLS->cur_downloaded_bytes += D->offset;
    //update_prompt ();
  }
  D->refcnt ++;
  clear_packet ();
  out_int (CODE_upload_get_file);
  if (!D->id) {
    out_int (CODE_input_file_location);
    out_long (D->volume);
    out_int (D->local_id);
    out_long (D->secret);
  } else {
    if (D->iv) {
      out_int (CODE_input_encrypted_file_location);
    } else {
      out_int (D->type);
    }
    out_long (D->id);
    out_long (D->access_hash);
  }
  out_int (D->offset);
  out_int (D->size ? (1 << 14) : (1 << 19));
  tglq_send_query (TLS, TLS->DC_list[D->dc], packet_ptr - packet_buffer, packet_buffer, &download_methods, D, callback, callback_extra);
  //tglq_send_query (TLS, TLS->DC_working, packet_ptr - packet_buffer, packet_buffer, &download_methods, D);
}

void tgl_do_load_photo_size (struct tgl_state *TLS, struct tgl_photo_size *P, void (*callback)(struct tgl_state *TLS, void *callback_extra, int success, const char *filename), void *callback_extra) {
  if (!P->loc.dc) {
    vlogprintf (E_WARNING, "Bad video thumb\n");
    if (callback) {
      callback (TLS, callback_extra, 0, 0);
    }
    return;
  }

  assert (P);
  struct download *D = talloc0 (sizeof (*D));
  D->id = 0;
  D->offset = 0;
  D->size = P->size;
  D->volume = P->loc.volume;
  D->dc = P->loc.dc;
  D->local_id = P->loc.local_id;
  D->secret = P->loc.secret;
  D->name = 0;
  D->fd = -1;
  load_next_part (TLS, D, callback, callback_extra);
}

void tgl_do_load_file_location (struct tgl_state *TLS, struct tgl_file_location *P, void (*callback)(struct tgl_state *TLS, void *callback_extra, int success, const char *filename), void *callback_extra) {
  if (!P->dc) {
    tgl_set_query_error (TLS, EINVAL, "Bad file location");
    if (callback) {
      callback (TLS, callback_extra, 0, 0);
    }
    return;
  }

  assert (P);
  struct download *D = talloc0 (sizeof (*D));
  D->id = 0;
  D->offset = 0;
  D->size = 0;
  D->volume = P->volume;
  D->dc = P->dc;
  D->local_id = P->local_id;
  D->secret = P->secret;
  D->name = 0;
  D->fd = -1;
  load_next_part (TLS, D, callback, callback_extra);
}

void tgl_do_load_photo (struct tgl_state *TLS, struct tgl_photo *photo, void (*callback)(struct tgl_state *TLS, void *callback_extra, int success, const char *filename), void *callback_extra) {
  if (!photo->sizes_num) {
    tgl_set_query_error (TLS, EINVAL, "Bad photo (no photo sizes");
    if (callback) {
      callback (TLS, callback_extra, 0, 0);
    }
    return;
  }
  int max = -1;
  int maxi = 0;
  int i;
  for (i = 0; i < photo->sizes_num; i++) {
    if (photo->sizes[i].w + photo->sizes[i].h > max) {
      max = photo->sizes[i].w + photo->sizes[i].h;
      maxi = i;
    }
  }
  tgl_do_load_photo_size (TLS, &photo->sizes[maxi], callback, callback_extra);
}

void tgl_do_load_document_thumb (struct tgl_state *TLS, struct tgl_document *video, void (*callback)(struct tgl_state *TLS, void *callback_extra, int success, const char *filename), void *callback_extra) {
  tgl_do_load_photo_size (TLS, &video->thumb, callback, callback_extra);
}

static void _tgl_do_load_document (struct tgl_state *TLS, struct tgl_document *V, struct download *D, void (*callback)(struct tgl_state *TLS, void *callback_extra, int success, const char *filename), void *callback_extra) {
  assert (V);
  D->offset = 0;
  D->size = V->size;
  D->id = V->id;
  D->access_hash = V->access_hash;
  D->dc = V->dc_id;
  D->name = 0;
  D->fd = -1;
  
  if (V->mime_type) {
    char *r = tg_extension_by_mime (V->mime_type);
    if (r) {
      D->ext = tstrdup (r);
    }
  }
  load_next_part (TLS, D, callback, callback_extra);
}

void tgl_do_load_document (struct tgl_state *TLS, struct tgl_document *V, void (*callback)(struct tgl_state *TLS, void *callback_extra, int success, const char *filename), void *callback_extra) {
  
  struct download *D = talloc0 (sizeof (*D));
  D->type = CODE_input_document_file_location;
  
  _tgl_do_load_document (TLS, V, D, callback, callback_extra);
}

void tgl_do_load_video (struct tgl_state *TLS, struct tgl_document *V, void (*callback)(struct tgl_state *TLS, void *callback_extra, int success, const char *filename), void *callback_extra) {

  struct download *D = talloc0 (sizeof (*D));
  D->type = CODE_input_video_file_location;
  
  _tgl_do_load_document (TLS, V, D, callback, callback_extra);
}

void tgl_do_load_audio (struct tgl_state *TLS, struct tgl_document *V, void (*callback)(struct tgl_state *TLS, void *callback_extra, int success, const char *filename), void *callback_extra) {
  
  struct download *D = talloc0 (sizeof (*D));
  D->type = CODE_input_audio_file_location;

  _tgl_do_load_document (TLS, V, D, callback, callback_extra);
}

void tgl_do_load_encr_document (struct tgl_state *TLS, struct tgl_encr_document *V, void (*callback)(struct tgl_state *TLS, void *callback_extra, int success, const char *filename), void *callback_extra) {
  assert (V);
  struct download *D = talloc0 (sizeof (*D));
  D->offset = 0;
  D->size = V->size;
  D->id = V->id;
  D->access_hash = V->access_hash;
  D->dc = V->dc_id;
  D->name = 0;
  D->fd = -1;
  D->key = V->key;
  D->iv = talloc (32);
  memcpy (D->iv, V->iv, 32);
  if (V->mime_type) {
    char *r = tg_extension_by_mime (V->mime_type);
    if (r) {
      D->ext = tstrdup (r);
    }
  }
  load_next_part (TLS, D, callback, callback_extra);

  unsigned char md5[16];
  unsigned char str[64];
  memcpy (str, V->key, 32);
  memcpy (str + 32, V->iv, 32);
  MD5 (str, 64, md5);
  assert (V->key_fingerprint == ((*(int *)md5) ^ (*(int *)(md5 + 4))));
}
/* }}} */

/* {{{ Export auth */

static int import_auth_on_answer (struct tgl_state *TLS, struct query *q, void *D) {
  struct tl_ds_auth_authorization *DS_U = D;
  tglf_fetch_alloc_user_new (TLS, DS_U->user);

  bl_do_dc_signed (TLS, ((struct tgl_dc *)q->extra)->id);

  if (q->callback) {
    ((void (*)(struct tgl_state *, void *, int))q->callback) (TLS, q->callback_extra, 1);
  }
  return 0;
}

static struct query_methods import_auth_methods = {
  .on_answer = import_auth_on_answer,
  .on_error = fail_on_error,
  .type = TYPE_TO_PARAM(auth_authorization)
};

static int export_auth_on_answer (struct tgl_state *TLS, struct query *q, void *D) {
  struct tl_ds_auth_exported_authorization *DS_EA = D;

  bl_do_set_our_id (TLS, DS_LVAL (DS_EA->id));


  clear_packet ();
  tgl_do_insert_header (TLS);
  out_int (CODE_auth_import_authorization);
  out_int (TLS->our_id);
  out_cstring (DS_STR (DS_EA->bytes));
  tglq_send_query (TLS, q->extra, packet_ptr - packet_buffer, packet_buffer, &import_auth_methods, q->extra, q->callback, q->callback_extra);
  return 0;
}

static struct query_methods export_auth_methods = {
  .on_answer = export_auth_on_answer,
  .on_error = fail_on_error,
  .type = TYPE_TO_PARAM(auth_exported_authorization)
};

void tgl_do_export_auth (struct tgl_state *TLS, int num, void (*callback) (struct tgl_state *TLS, void *callback_extra, int success), void *callback_extra) {
  clear_packet ();
  out_int (CODE_auth_export_authorization);
  out_int (num);
  tglq_send_query (TLS, TLS->DC_working, packet_ptr - packet_buffer, packet_buffer, &export_auth_methods, TLS->DC_list[num], callback, callback_extra);
}
/* }}} */

/* {{{ Add contact */
static int add_contact_on_answer (struct tgl_state *TLS, struct query *q, void *D) {
  struct tl_ds_contacts_imported_contacts *DS_CIC = D;

  if (DS_LVAL (DS_CIC->imported->cnt) > 0) {
    vlogprintf (E_DEBUG, "Added successfully");
  } else {
    vlogprintf (E_DEBUG, "Not added");
  }

  int n = DS_LVAL (DS_CIC->users->cnt);

  struct tgl_user **UL = talloc (n * sizeof (void *));
  int i;
  for (i = 0; i < n; i++) {
    UL[i] = tglf_fetch_alloc_user_new (TLS, DS_CIC->users->data[i]);
  }

  if (q->callback) {
    ((void (*)(struct tgl_state *, void *, int, int, struct tgl_user **))q->callback) (TLS, q->callback_extra, 1, n, UL);
  }
  tfree (UL, n * sizeof (void *));
  return 0;
}

static struct query_methods add_contact_methods = {
  .on_answer = add_contact_on_answer,
  .on_error = q_list_on_error,
  .type = TYPE_TO_PARAM(contacts_imported_contacts)
};

void tgl_do_add_contact (struct tgl_state *TLS, const char *phone, int phone_len, const char *first_name, int first_name_len, const char *last_name, int last_name_len, int force, void (*callback)(struct tgl_state *TLS,void *callback_extra, int success, int size, struct tgl_user *users[]), void *callback_extra) {
  clear_packet ();
  out_int (CODE_contacts_import_contacts);
  out_int (CODE_vector);
  out_int (1);
  out_int (CODE_input_phone_contact);
  long long r;
  tglt_secure_random (&r, 8);
  out_long (r);
  out_cstring (phone, phone_len);
  out_cstring (first_name, first_name_len);
  out_cstring (last_name, last_name_len);
  out_int (force ? CODE_bool_true : CODE_bool_false);
  tglq_send_query (TLS, TLS->DC_working, packet_ptr - packet_buffer, packet_buffer, &add_contact_methods, 0, callback, callback_extra);
}
/* }}} */

/* {{{ Del contact */
static int del_contact_on_answer (struct tgl_state *TLS, struct query *q, void *D) {
  if (q->callback) {
    ((void (*)(struct tgl_state *, void *, int))q->callback) (TLS, q->callback_extra, 1);
  }
  return 0;
}

static struct query_methods del_contact_methods = {
  .on_answer = del_contact_on_answer,
  .on_error = q_void_on_error,
  .type = TYPE_TO_PARAM(contacts_link)
};

void tgl_do_del_contact (struct tgl_state *TLS, tgl_peer_id_t id, void (*callback)(struct tgl_state *TLS, void *callback_extra, int success), void *callback_extra) {
  if (tgl_get_peer_type (id) != TGL_PEER_USER) {
    tgl_set_query_error (TLS, EINVAL, "id should be user id");
    if (callback) {
      callback (TLS, callback_extra, 0);
    }
    return;
  }
  clear_packet ();
  out_int (CODE_contacts_delete_contact);

  tgl_peer_t *U = tgl_peer_get (TLS, id);
  if (U && U->user.access_hash) {
    out_int (CODE_input_user_foreign);
    out_int (tgl_get_peer_id (id));
    out_long (U->user.access_hash);
  } else {
    out_int (CODE_input_user_contact);
    out_int (tgl_get_peer_id (id));
  }
  tglq_send_query (TLS, TLS->DC_working, packet_ptr - packet_buffer, packet_buffer, &del_contact_methods, 0, callback, callback_extra);
}
/* }}} */

/* {{{ Msg search */

struct msg_search_extra {
  struct tgl_message **ML;
  int list_offset;
  int list_size;
  tgl_peer_id_t id;
  int limit;
  int offset;
  int from;
  int to;
  int max_id;
  char *query;
};

static void _tgl_do_msg_search (struct tgl_state *TLS, struct msg_search_extra *E, void (*callback)(struct tgl_state *TLS, void *callback_extra, int success, int size, struct tgl_message *list[]), void *callback_extra);

static int msg_search_on_answer (struct tgl_state *TLS, struct query *q, void *D) {
  struct tl_ds_messages_messages *DS_MM = D;

  struct msg_search_extra *E = q->extra;

  int n = DS_LVAL (DS_MM->messages->cnt);

  if (E->list_size - E->list_offset < n) {
    int new_list_size = 2 * E->list_size;
    if (new_list_size - E->list_offset < n) {
      new_list_size = n + E->list_offset;
    }
    E->ML = trealloc (E->ML, E->list_size * sizeof (void *), new_list_size * sizeof (void *));
    assert (E->ML);
    E->list_size = new_list_size;
  }

  int i;
  for (i = 0; i < n; i++) {
    E->ML[i + E->list_offset] = tglf_fetch_alloc_message_new (TLS, DS_MM->messages->data[i]);
  }
  E->list_offset += n;
  E->offset += n;
  E->limit -= n;
  if (E->limit + E->offset >= DS_LVAL (DS_MM->count)) {
    E->limit = DS_LVAL (DS_MM->count) - E->offset;
    if (E->limit < 0) { E->limit = 0; }
  }
  assert (E->limit >= 0);

  for (i = 0; i < DS_LVAL (DS_MM->chats->cnt); i++) {
    tglf_fetch_alloc_chat_new (TLS, DS_MM->chats->data[i]);
  }
  for (i = 0; i < DS_LVAL (DS_MM->users->cnt); i++) {
    tglf_fetch_alloc_user_new (TLS, DS_MM->users->data[i]);
  }

  if (E->limit <= 0 || DS_MM->magic == CODE_messages_messages) {
    if (q->callback) {
      ((void (*)(struct tgl_state *, void *, int, int, struct tgl_message **))q->callback) (TLS, q->callback_extra, 1, E->list_offset, E->ML);
    }

    tfree_str (E->query);
    tfree (E->ML, sizeof (void *) * E->list_size);
    tfree (E, sizeof (*E));
  } else {
    E->max_id = E->ML[E->list_offset - 1]->id;
    E->offset = 0;
   _tgl_do_msg_search (TLS, E, q->callback, q->callback_extra);
  }
  return 0;
}

static int msg_search_on_error (struct tgl_state *TLS, struct query *q, int error_code, int error_len, const char *error) {
  tgl_set_query_error (TLS, EPROTO, "RPC_CALL_FAIL %d: %.*s", error_code, error_len, error);

  struct msg_search_extra *E = q->extra;
  tfree_str (E->query);
  tfree (E->ML, sizeof (void *) * E->list_size);
  tfree (E, sizeof (*E));
  if (q->callback) {
    ((void (*)(struct tgl_state *, void *, int, int, struct tgl_message **))q->callback) (TLS, q->callback_extra, 0, 0, NULL);
  }
  return 0;
}

static struct query_methods msg_search_methods = {
  .on_answer = msg_search_on_answer,
  .on_error = msg_search_on_error,
  .type = TYPE_TO_PARAM(messages_messages)
};

static void _tgl_do_msg_search (struct tgl_state *TLS, struct msg_search_extra *E, void (*callback)(struct tgl_state *TLS,void *callback_extra, int success, int size, struct tgl_message *list[]), void *callback_extra) {
  clear_packet ();
  out_int (CODE_messages_search);
  if (tgl_get_peer_type (E->id) == TGL_PEER_UNKNOWN) {
    out_int (CODE_input_peer_empty);
  } else {
    out_peer_id (TLS, E->id);
  }

  out_string (E->query);
  out_int (CODE_input_messages_filter_empty);
  out_int (E->from);
  out_int (E->to);
  out_int (E->offset); // offset
  out_int (E->max_id); // max_id
  out_int (E->limit);
  tglq_send_query (TLS, TLS->DC_working, packet_ptr - packet_buffer, packet_buffer, &msg_search_methods, E, callback, callback_extra);
}

void tgl_do_msg_search (struct tgl_state *TLS, tgl_peer_id_t id, int from, int to, int limit, int offset, const char *pattern, int pattern_len, void (*callback)(struct tgl_state *TLS, void *callback_extra, int success, int size, struct tgl_message *list[]), void *callback_extra) {
  if (tgl_get_peer_type (id) == TGL_PEER_ENCR_CHAT) {
    tgl_set_query_error (TLS, EINVAL, "can not search in secret chats");
    if (callback) {
      callback (TLS, callback_extra, 0, 0, 0);
    }
    return;
  }
  struct msg_search_extra *E = talloc0 (sizeof (*E));
  E->id = id;
  E->from = from;
  E->to = to;
  E->limit = limit;
  E->offset = offset;
  E->query = tstrndup (pattern, pattern_len);

  _tgl_do_msg_search (TLS, E, callback, callback_extra);
}
/* }}} */

/* {{{ Get difference */

static int get_state_on_answer (struct tgl_state *TLS, struct query *q, void *D) {
  struct tl_ds_updates_state *DS_US = D;

  assert (TLS->locks & TGL_LOCK_DIFF);
  TLS->locks ^= TGL_LOCK_DIFF;

  bl_do_set_pts (TLS, DS_LVAL (DS_US->pts));
  bl_do_set_qts (TLS, DS_LVAL (DS_US->qts));
  bl_do_set_date (TLS, DS_LVAL (DS_US->date));
  bl_do_set_seq (TLS, DS_LVAL (DS_US->seq));

  if (q->callback) {
    ((void (*)(struct tgl_state *, void *, int))q->callback) (TLS, q->callback_extra, 1);
  }
  return 0;
}

static int lookup_state_on_answer (struct tgl_state *TLS, struct query *q, void *D) {
  struct tl_ds_updates_state *DS_US = D;
  int pts = DS_LVAL (DS_US->pts);
  int qts = DS_LVAL (DS_US->qts);
  int seq = DS_LVAL (DS_US->seq);

  if (pts > TLS->pts || qts > TLS->qts || seq > TLS->seq) {
    tgl_do_get_difference (TLS, 0, 0, 0);
  }
  return 0;
}


//int get_difference_active;
static int get_difference_on_answer (struct tgl_state *TLS, struct query *q, void *D) {
  struct tl_ds_updates_difference *DS_UD = D;

  assert (TLS->locks & TGL_LOCK_DIFF);
  TLS->locks ^= TGL_LOCK_DIFF;

  if (DS_UD->magic == CODE_updates_difference_empty) {
    bl_do_set_date (TLS, DS_LVAL (DS_UD->date));
    bl_do_set_seq (TLS, DS_LVAL (DS_UD->seq));

    vlogprintf (E_DEBUG, "Empty difference. Seq = %d\n", TLS->seq);
    if (q->callback) {
      ((void (*)(struct tgl_state *, void *, int))q->callback) (TLS, q->callback_extra, 1);
    }
  } else {
    int i;

    for (i = 0; i < DS_LVAL (DS_UD->users->cnt); i++) {
      tglf_fetch_alloc_user_new (TLS, DS_UD->users->data[i]);
    }
    for (i = 0; i < DS_LVAL (DS_UD->chats->cnt); i++) {
      tglf_fetch_alloc_chat_new (TLS, DS_UD->chats->data[i]);
    }

    int ml_pos = DS_LVAL (DS_UD->new_messages->cnt);
    struct tgl_message **ML = talloc (ml_pos * sizeof (void *));
    for (i = 0; i < ml_pos; i++) {
      ML[i] = tglf_fetch_alloc_message_new (TLS, DS_UD->new_messages->data[i]);
    }

    int el_pos = DS_LVAL (DS_UD->new_encrypted_messages->cnt);
    struct tgl_message **EL = talloc (el_pos * sizeof (void *));
    for (i = 0; i < el_pos; i++) {
      EL[i] = tglf_fetch_alloc_encrypted_message_new (TLS, DS_UD->new_encrypted_messages->data[i]);
    }

    for (i = 0; i < DS_LVAL (DS_UD->other_updates->cnt); i++) {
      tglu_work_update_new (TLS, 1, DS_UD->other_updates->data[i]);
    }

    for (i = 0; i < DS_LVAL (DS_UD->other_updates->cnt); i++) {
      tglu_work_update_new (TLS, -1, DS_UD->other_updates->data[i]);
    }

    for (i = 0; i < ml_pos; i++) {
      bl_do_msg_update (TLS, ML[i]->id);
    }
    for (i = 0; i < el_pos; i++) {
      bl_do_msg_update (TLS, EL[i]->id);
    }

    tfree (ML, ml_pos * sizeof (void *));
    tfree (EL, el_pos * sizeof (void *));

    if (DS_UD->state) {
      bl_do_set_pts (TLS, DS_LVAL (DS_UD->state->pts));
      bl_do_set_qts (TLS, DS_LVAL (DS_UD->state->qts));
      bl_do_set_date (TLS, DS_LVAL (DS_UD->state->date));
      bl_do_set_seq (TLS, DS_LVAL (DS_UD->state->seq));

      if (q->callback) {
        ((void (*)(struct tgl_state *, void *, int))q->callback) (TLS, q->callback_extra, 1);
      }
    } else {
      bl_do_set_pts (TLS, DS_LVAL (DS_UD->intermediate_state->pts));
      bl_do_set_qts (TLS, DS_LVAL (DS_UD->intermediate_state->qts));
      bl_do_set_date (TLS, DS_LVAL (DS_UD->intermediate_state->date));

      tgl_do_get_difference (TLS, 0, q->callback, q->callback_extra);
    }
  }
  return 0;
}

static struct query_methods lookup_state_methods = {
  .on_answer = lookup_state_on_answer,
  .on_error = q_void_on_error,
  .type = TYPE_TO_PARAM(updates_state)
};

static struct query_methods get_state_methods = {
  .on_answer = get_state_on_answer,
  .on_error = q_void_on_error,
  .type = TYPE_TO_PARAM(updates_state)
};

static struct query_methods get_difference_methods = {
  .on_answer = get_difference_on_answer,
  .on_error = q_void_on_error,
  .type = TYPE_TO_PARAM(updates_difference)
};

void tgl_do_lookup_state (struct tgl_state *TLS) {
  if (TLS->locks & TGL_LOCK_DIFF) {
    return;
  }
  clear_packet ();
  tgl_do_insert_header (TLS);
  out_int (CODE_updates_get_state);
  tglq_send_query (TLS, TLS->DC_working, packet_ptr - packet_buffer, packet_buffer, &lookup_state_methods, 0, 0, 0);
}

void tgl_do_get_difference (struct tgl_state *TLS, int sync_from_start, void (*callback)(struct tgl_state *TLS, void *callback_extra, int success), void *callback_extra) {
  //get_difference_active = 1;
  //difference_got = 0;
  if (TLS->locks & TGL_LOCK_DIFF) {
    if (callback) {
      callback (TLS, callback_extra, 0);
    }
    return;
  }
  TLS->locks |= TGL_LOCK_DIFF;
  clear_packet ();
  tgl_do_insert_header (TLS);
  if (TLS->pts > 0 || sync_from_start) {
    if (TLS->pts == 0) { TLS->pts = 1; }
    //if (TLS->qts == 0) { TLS->qts = 1; }
    if (TLS->date == 0) { TLS->date = 1; }
    out_int (CODE_updates_get_difference);
    out_int (TLS->pts);
    out_int (TLS->date);
    out_int (TLS->qts);
    tglq_send_query (TLS, TLS->DC_working, packet_ptr - packet_buffer, packet_buffer, &get_difference_methods, 0, callback, callback_extra);
  } else {
    out_int (CODE_updates_get_state);
    tglq_send_query (TLS, TLS->DC_working, packet_ptr - packet_buffer, packet_buffer, &get_state_methods, 0, callback, callback_extra);
  }
}
/* }}} */

/* {{{ Visualize key */

int tgl_do_visualize_key (struct tgl_state *TLS, tgl_peer_id_t id, unsigned char buf[16]) {
  assert (tgl_get_peer_type (id) == TGL_PEER_ENCR_CHAT);
  tgl_peer_t *P = tgl_peer_get (TLS, id);
  assert (P);
  if (P->encr_chat.state != sc_ok) {
    vlogprintf (E_WARNING, "Chat is not initialized yet\n");
    return -1;
  }
  memcpy (buf, P->encr_chat.first_key_sha, 16);
  return 0;
}
/* }}} */

/* {{{ Add user to chat */

void tgl_do_add_user_to_chat (struct tgl_state *TLS, tgl_peer_id_t chat_id, tgl_peer_id_t id, int limit, void (*callback)(struct tgl_state *TLS, void *callback_extra, int success), void *callback_extra) {
  clear_packet ();
  out_int (CODE_messages_add_chat_user);
  out_int (tgl_get_peer_id (chat_id));

  assert (tgl_get_peer_type (id) == TGL_PEER_USER);
  tgl_peer_t *U = tgl_peer_get (TLS, id);
  if (U && U->user.access_hash) {
    out_int (CODE_input_user_foreign);
    out_int (tgl_get_peer_id (id));
    out_long (U->user.access_hash);
  } else {
    out_int (CODE_input_user_contact);
    out_int (tgl_get_peer_id (id));
  }
  out_int (limit);

  tglq_send_query (TLS, TLS->DC_working, packet_ptr - packet_buffer, packet_buffer, &send_msgs_methods, 0, callback, callback_extra);
}

void tgl_do_del_user_from_chat (struct tgl_state *TLS, tgl_peer_id_t chat_id, tgl_peer_id_t id, void (*callback)(struct tgl_state *TLS, void *callback_extra, int success), void *callback_extra) {
  clear_packet ();
  out_int (CODE_messages_delete_chat_user);
  out_int (tgl_get_peer_id (chat_id));

  assert (tgl_get_peer_type (id) == TGL_PEER_USER);
  tgl_peer_t *U = tgl_peer_get (TLS, id);
  if (U && U->user.access_hash) {
    out_int (CODE_input_user_foreign);
    out_int (tgl_get_peer_id (id));
    out_long (U->user.access_hash);
  } else {
    out_int (CODE_input_user_contact);
    out_int (tgl_get_peer_id (id));
  }
  tglq_send_query (TLS, TLS->DC_working, packet_ptr - packet_buffer, packet_buffer, &send_msgs_methods, 0, callback, callback_extra);
}

/* }}} */

/* {{{ Create secret chat */

void tgl_do_create_secret_chat (struct tgl_state *TLS, tgl_peer_id_t id, void (*callback)(struct tgl_state *TLS, void *callback_extra, int success, struct tgl_secret_chat *E), void *callback_extra) {
  assert (tgl_get_peer_type (id) == TGL_PEER_USER);
  tgl_peer_t *U = tgl_peer_get (TLS, id);
  if (!U) {
    tgl_set_query_error (TLS, EINVAL, "Can not create secret chat with unknown user");
    if (callback) {
      callback (TLS, callback_extra, 0, NULL);
    }
    return;
  }

  tgl_do_create_encr_chat_request (TLS, tgl_get_peer_id (id), callback, callback_extra);
}
/* }}} */

/* {{{ Create group chat */

void tgl_do_create_group_chat (struct tgl_state *TLS, int users_num, tgl_peer_id_t ids[], const char *chat_topic, int chat_topic_len, void (*callback)(struct tgl_state *TLS, void *callback_extra, int success), void *callback_extra) {
  clear_packet ();
  out_int (CODE_messages_create_chat);
  out_int (CODE_vector);
  out_int (users_num); // Number of users, currently we support only 1 user.
  int i;
  for (i = 0; i < users_num; i++) {
    tgl_peer_id_t id = ids[i];
    tgl_peer_t *U = tgl_peer_get (TLS, id);
    if (!U || tgl_get_peer_type (id) != TGL_PEER_USER) {
      tgl_set_query_error (TLS, EINVAL, "Can not create chat with unknown user");
      if (callback) {
        callback (TLS, callback_extra, 0);
      }
      return;
    }
    if (U && U->user.access_hash) {
      out_int (CODE_input_user_foreign);
      out_int (tgl_get_peer_id (id));
      out_long (U->user.access_hash);
    } else {
      out_int (CODE_input_user_contact);
      out_int (tgl_get_peer_id (id));
    }
  }
  out_cstring (chat_topic, chat_topic_len);
  tglq_send_query (TLS, TLS->DC_working, packet_ptr - packet_buffer, packet_buffer, &send_msgs_methods, 0, callback, callback_extra);
}
/* }}} */

/* {{{ Delete msg */

static int delete_msg_on_answer (struct tgl_state *TLS, struct query *q, void *D) {
  struct tl_ds_messages_affected_messages *DS_MAM = D;

  struct tgl_message *M = tgl_message_get (TLS, (long)q->extra);
  if (M) {
    bl_do_message_delete (TLS, M);
  }

  int r = tgl_check_pts_diff (TLS, DS_LVAL (DS_MAM->pts), DS_LVAL (DS_MAM->pts_count));

  if (r > 0) {
    bl_do_set_pts (TLS, DS_LVAL (DS_MAM->pts));
  }

  if (q->callback) {
    ((void (*)(struct tgl_state *, void *, int))q->callback) (TLS, q->callback_extra, 1);
  }
  return 0;
}

static struct query_methods delete_msg_methods = {
  .on_answer = delete_msg_on_answer,
  .on_error = q_void_on_error,
  .type = TYPE_TO_PARAM(messages_affected_messages)
};

void tgl_do_delete_msg (struct tgl_state *TLS, long long id, void (*callback)(struct tgl_state *TLS, void *callback_extra, int success), void *callback_extra) {
  clear_packet ();
  out_int (CODE_messages_delete_messages);
  out_int (CODE_vector);
  out_int (1);
  out_int (id);
  tglq_send_query (TLS, TLS->DC_working, packet_ptr - packet_buffer, packet_buffer, &delete_msg_methods, (void *)(long)id, callback, callback_extra);
}
/* }}} */

/* {{{ Export card */

static int export_card_on_answer (struct tgl_state *TLS, struct query *q, void *D) {
  struct tl_ds_vector *DS_V = D;

  int n = DS_LVAL (DS_V->f1);

  int *r = talloc (4 * n);
  int i;
  for (i = 0; i < n; i++) {
    r[i] = *(int *)DS_V->f2[i];
  }

  if (q->callback) {
    ((void (*)(struct tgl_state *, void *, int, int, int *))q->callback) (TLS, q->callback_extra, 1, n, r);
  }
  tfree (r, 4 * n);
  return 0;
}

static struct query_methods export_card_methods = {
  .on_answer = export_card_on_answer,
  .on_error = q_list_on_error,
  .type = TYPE_TO_PARAM_1(vector, TYPE_TO_PARAM (bare_int))
};

void tgl_do_export_card (struct tgl_state *TLS, void (*callback)(struct tgl_state *TLS, void *callback_extra, int success, int size, int *card), void *callback_extra) {
  clear_packet ();
  out_int (CODE_contacts_export_card);
  tglq_send_query (TLS, TLS->DC_working, packet_ptr - packet_buffer, packet_buffer, &export_card_methods, 0, callback, callback_extra);
}
/* }}} */

/* {{{ Import card */

static int import_card_on_answer (struct tgl_state *TLS, struct query *q, void *D) {
  struct tgl_user *U = tglf_fetch_alloc_user_new (TLS, D);

  if (q->callback) {
    ((void (*)(struct tgl_state *, void *, int, struct tgl_user *))q->callback) (TLS, q->callback_extra, 1, U);
  }
  return 0;
}

static struct query_methods import_card_methods = {
  .on_answer = import_card_on_answer,
  .on_error = q_ptr_on_error,
  .type = TYPE_TO_PARAM (user)
};

void tgl_do_import_card (struct tgl_state *TLS, int size, int *card, void (*callback)(struct tgl_state *TLS, void *callback_extra, int success, struct tgl_user *U), void *callback_extra) {
  clear_packet ();
  out_int (CODE_contacts_import_card);
  out_int (CODE_vector);
  out_int (size);
  out_ints (card, size);
  tglq_send_query (TLS, TLS->DC_working, packet_ptr - packet_buffer, packet_buffer, &import_card_methods, 0, callback, callback_extra);
}
/* }}} */

void tgl_do_start_bot (struct tgl_state *TLS, tgl_peer_id_t bot, tgl_peer_id_t chat, const char *str, int str_len, void (*callback)(struct tgl_state *TLS, void *callback_extra, int success), void *callback_extra) {
  clear_packet ();
  out_int (CODE_messages_start_bot);
  struct tgl_user *U = (void *)tgl_peer_get (TLS, bot);
  if (U && U->access_hash) {
    out_int (CODE_input_user_foreign);
    out_int (tgl_get_peer_id (bot));
    out_long (U->access_hash);
  } else {
    out_int (CODE_input_user_contact);
    out_int (tgl_get_peer_id (bot));
  }
  out_int (tgl_get_peer_id (chat));
  long long m;
  tglt_secure_random (&m, 8);
  out_long (m);
  out_cstring (str, str_len);
  tglq_send_query (TLS, TLS->DC_working, packet_ptr - packet_buffer, packet_buffer, &send_msgs_methods, 0, callback, callback_extra);
}

/* {{{ Send typing */
static int send_typing_on_answer (struct tgl_state *TLS, struct query *q, void *D) {
  if (q->callback) {
    ((void (*)(struct tgl_state *, void *, int))q->callback)(TLS, q->callback_extra, 1);
  }
  return 0;
}

static struct query_methods send_typing_methods = {
  .on_answer = send_typing_on_answer,
  .on_error = q_void_on_error,
  .type = TYPE_TO_PARAM(bool)
};

void tgl_do_send_typing (struct tgl_state *TLS, tgl_peer_id_t id, enum tgl_typing_status status, void (*callback)(struct tgl_state *TLS, void *callback_extra, int success), void *callback_extra) {
  if (tgl_get_peer_type (id) != TGL_PEER_ENCR_CHAT) {
    clear_packet ();
    out_int (CODE_messages_set_typing);
    out_peer_id (TLS, id);
    switch (status) {
    case tgl_typing_none:
    case tgl_typing_typing:
      out_int (CODE_send_message_typing_action);
      break;
    case tgl_typing_cancel:
      out_int (CODE_send_message_cancel_action);
      break;
    case tgl_typing_record_video:
      out_int (CODE_send_message_record_video_action);
      break;
    case tgl_typing_upload_video:
      out_int (CODE_send_message_upload_video_action);
      break;
    case tgl_typing_record_audio:
      out_int (CODE_send_message_record_audio_action);
      break;
    case tgl_typing_upload_audio:
      out_int (CODE_send_message_upload_audio_action);
      break;
    case tgl_typing_upload_photo:
      out_int (CODE_send_message_upload_photo_action);
      break;
    case tgl_typing_upload_document:
      out_int (CODE_send_message_upload_document_action);
      break;
    case tgl_typing_geo:
      out_int (CODE_send_message_geo_location_action);
      break;
    case tgl_typing_choose_contact:
      out_int (CODE_send_message_choose_contact_action);
      break;
    }
    tglq_send_query (TLS, TLS->DC_working, packet_ptr - packet_buffer, packet_buffer, &send_typing_methods, 0, callback, callback_extra);
  } else {
    if (callback) {
      callback (TLS, callback_extra, 0);
    }
  }
}
/* }}} */

/* {{{ Extd query */
#ifndef DISABLE_EXTF


char *tglf_extf_print_ds (struct tgl_state *TLS, void *DS, struct paramed_type *T);

static int ext_query_on_answer (struct tgl_state *TLS, struct query *q, void *D) {
  if (q->callback) {
    char *buf = tglf_extf_print_ds (TLS, D, q->type);
    ((void (*)(struct tgl_state *, void *, int, char *))q->callback) (TLS, q->callback_extra, 1, buf);
  }
  tgl_paramed_type_free (q->type);
  return 0;
}

static struct query_methods ext_query_methods = {
  .on_answer = ext_query_on_answer,
  .on_error = q_list_on_error
};

void tgl_do_send_extf (struct tgl_state *TLS, const char *data, int data_len, void (*callback)(struct tgl_state *TLS, void *callback_extra, int success, const char *buf), void *callback_extra) {
  clear_packet ();

  ext_query_methods.type = tglf_extf_store (TLS, data, data_len);

  if (ext_query_methods.type) {
    tglq_send_query (TLS, TLS->DC_working, packet_ptr - packet_buffer, packet_buffer, &ext_query_methods, 0, callback, callback_extra);
  }
}
#else
void tgl_do_send_extf (struct tgl_state *TLS, char *data, int data_len, void (*callback)(struct tgl_state *TLS, void *callback_extra, int success, char *buf), void *callback_extra) {
  if (callback) {
    callback (TLS, callback_extra, 0, 0);
  }
}
#endif
/* }}} */

/* {{{ get messages */

static int get_messages_on_answer (struct tgl_state *TLS, struct query *q, void *D) {
  struct tl_ds_messages_messages *DS_MM = D;

  int i;
  for (i = 0; i < DS_LVAL (DS_MM->users->cnt); i++) {
    tglf_fetch_alloc_user_new (TLS, DS_MM->users->data[i]);
  }
  for (i = 0; i < DS_LVAL (DS_MM->chats->cnt); i++) {
    tglf_fetch_alloc_chat_new (TLS, DS_MM->chats->data[i]);
  }

  struct tgl_message **ML;
  if (q->extra) {
    ML = talloc0 (sizeof (void *) * DS_LVAL (DS_MM->messages->cnt));
  } else {
    static struct tgl_message *M;
    M = NULL;
    ML = &M;
    assert (DS_LVAL (DS_MM->messages->cnt) <= 1);
  }
  for (i = 0; i < DS_LVAL (DS_MM->messages->cnt); i++) {
    ML[i] = tglf_fetch_alloc_message_new (TLS, DS_MM->messages->data[i]);
  }
  if (q->callback) {
    if (q->extra) {
      ((void (*)(struct tgl_state *, void *, int, int, struct tgl_message **))q->callback)(TLS, q->callback_extra, 1, DS_LVAL (DS_MM->messages->cnt), ML);
    } else {
      if (DS_LVAL (DS_MM->messages->cnt) > 0) {
        ((void (*)(struct tgl_state *, void *, int, struct tgl_message *))q->callback)(TLS, q->callback_extra, 1, *ML);
      } else {
        tgl_set_query_error (TLS, ENOENT, "no such message");
        ((void (*)(struct tgl_state *, void *, int, struct tgl_message *))q->callback)(TLS, q->callback_extra, 0, NULL);
      }
    }
  }
  if (q->extra) {
    tfree (ML, sizeof (void *) * DS_LVAL (DS_MM->messages->cnt));
  }
  return 0;
}

static struct query_methods get_messages_methods = {
  .on_answer = get_messages_on_answer,
  .on_error = q_ptr_on_error,
  .type = TYPE_TO_PARAM (messages_messages)
};

void tgl_do_get_message (struct tgl_state *TLS, long long id, void (*callback)(struct tgl_state *TLS, void *callback_extra, int success, struct tgl_message *M), void *callback_extra) {
  struct tgl_message *M = tgl_message_get (TLS, id);
  if (M) {
    if (callback) {
      callback (TLS, callback_extra, 1, M);
    }
    return;
  }

  clear_packet ();

  out_int (CODE_messages_get_messages);
  out_int (CODE_vector);
  out_int (1);
  out_int (id);


  tglq_send_query (TLS, TLS->DC_working, packet_ptr - packet_buffer, packet_buffer, &get_messages_methods, 0, callback, callback_extra);
}
/* }}} */

/* {{{ Export/import chat link */
static int export_chat_link_on_answer (struct tgl_state *TLS, struct query *q, void *D) {
  struct tl_ds_exported_chat_invite *DS_ECI = D;

  char *s = DS_STR_DUP (DS_ECI->link);

  if (q->callback) {
    ((void (*)(struct tgl_state *, void *, int, const char *))q->callback)(TLS, q->callback_extra, s ? 1 : 0, s);
  }
  tfree_str (s);
  return 0;
}


static struct query_methods export_chat_link_methods = {
  .on_answer = export_chat_link_on_answer,
  .on_error = q_ptr_on_error,
  .type = TYPE_TO_PARAM(exported_chat_invite)
};

void tgl_do_export_chat_link (struct tgl_state *TLS, tgl_peer_id_t id, void (*callback)(struct tgl_state *TLS, void *callback_extra, int success, const char *link), void *callback_extra) {
  if (tgl_get_peer_type (id) != TGL_PEER_CHAT) {
    tgl_set_query_error (TLS, EINVAL, "Can only export chat link for chat");
    if (callback) {
      callback (TLS, callback_extra, 0, NULL);
    }
    return;
  }

  clear_packet ();
  out_int (CODE_messages_export_chat_invite);
  out_int (tgl_get_peer_id (id));

  tglq_send_query (TLS, TLS->DC_working, packet_ptr - packet_buffer, packet_buffer, &export_chat_link_methods, 0, callback, callback_extra);
}

void tgl_do_import_chat_link (struct tgl_state *TLS, const char *link, int len, void (*callback)(struct tgl_state *TLS, void *callback_extra, int success), void *callback_extra) {
  const char *l = link + len - 1;
  while (l >= link && *l != '/') {
    l --;
  }
  l ++;

  clear_packet ();
  out_int (CODE_messages_import_chat_invite);
  out_cstring (l, len - (l - link));

  tglq_send_query (TLS, TLS->DC_working, packet_ptr - packet_buffer, packet_buffer, &send_msgs_methods, 0, callback, callback_extra);
}

/* }}} */

/* {{{ set password */
static int set_password_on_answer (struct tgl_state *TLS, struct query *q, void *D) {
  if (q->callback) {
    ((void (*)(struct tgl_state *, void *, int))q->callback)(TLS, q->callback_extra, 1);
  }
  return 0;
}

static int set_password_on_error (struct tgl_state *TLS, struct query *q, int error_code, int error_len, const char *error) {
  if (error_code == 400) {
    if (!strcmp (error, "PASSWORD_HASH_INVALID")) {
      vlogprintf (E_WARNING, "Bad old password\n");
      if (q->callback) {
        ((void (*)(struct tgl_state *, void *, int))q->callback)(TLS, q->callback_extra, 0);
      }
      return 0;
    }
    if (!strcmp (error, "NEW_PASSWORD_BAD")) {
      vlogprintf (E_WARNING, "Bad new password (unchanged or equals hint)\n");
      if (q->callback) {
        ((void (*)(struct tgl_state *, void *, int))q->callback)(TLS, q->callback_extra, 0);
      }
      return 0;
    }
    if (!strcmp (error, "NEW_SALT_INVALID")) {
      vlogprintf (E_WARNING, "Bad new salt\n");
      if (q->callback) {
        ((void (*)(struct tgl_state *, void *, int))q->callback)(TLS, q->callback_extra, 0);
      }
      return 0;
    }
  }
  tgl_set_query_error (TLS, EPROTO, "RPC_CALL_FAIL %d: %.*s", error_code, error_len, error);
  if (q->callback) {
    ((void (*)(struct tgl_state *, void *, int))q->callback)(TLS, q->callback_extra, 0);
  }
  return 0;
}

static struct query_methods set_password_methods = {
  .on_answer = set_password_on_answer,
  .on_error = set_password_on_error,
  .type = TYPE_TO_PARAM(bool)
};

static void tgl_do_act_set_password (struct tgl_state *TLS, const char *current_password, int current_password_len, const char *new_password, int new_password_len, const char *current_salt, int current_salt_len, const char *new_salt, int new_salt_len, const char *hint, int hint_len, void (*callback)(struct tgl_state *TLS, void *callback_extra, int success), void *callback_extra) {
  clear_packet ();
  static char s[512];
  static unsigned char shab[32];

  assert (current_salt_len <= 128);
  assert (current_password_len <= 128);
  assert (new_salt_len <= 128);
  assert (new_password_len <= 128);

  out_int (CODE_account_update_password_settings);

  if (current_password_len && current_salt_len) {
    memcpy (s, current_salt, current_salt_len);
    memcpy (s + current_salt_len, current_password, current_password_len);
    memcpy (s + current_salt_len + current_password_len, current_salt, current_salt_len);

    SHA256 ((void *)s, 2 * current_salt_len + current_password_len, shab);
    out_cstring ((void *)shab, 32);
  } else {
    out_string ("");
  }

  out_int (CODE_account_password_input_settings);
  if (new_password_len) {
    out_int (1);

    static char d[256];
    memcpy (d, new_salt, new_salt_len);

    int l = new_salt_len;
    tglt_secure_random (d + l, 16);
    l += 16;
    memcpy (s, d, l);

    memcpy (s + l, new_password, new_password_len);
    memcpy (s + l + new_password_len, d, l);

    SHA256 ((void *)s, 2 * l + new_password_len, shab);

    out_cstring (d, l);
    out_cstring ((void *)shab, 32);
    out_cstring (hint, hint_len);
  } else {
    out_int (0);
  }


  tglq_send_query (TLS, TLS->DC_working, packet_ptr - packet_buffer, packet_buffer, &set_password_methods, 0, callback, callback_extra);
}

struct change_password_extra {
  char *current_password;
  char *new_password;
  char *current_salt;
  char *new_salt;
  char *hint;
  int current_password_len;
  int new_password_len;
  int current_salt_len;
  int new_salt_len;
  int hint_len;
  void (*callback)(struct tgl_state *, void *, int);
  void *callback_extra;
};

void tgl_on_new_pwd (struct tgl_state *TLS, const char *pwd[], void *_T);
void tgl_on_new2_pwd (struct tgl_state *TLS, const char *pwd, void *_T) {
  struct change_password_extra *E = _T;
  if (strlen (pwd) != (size_t)E->new_password_len || memcmp (E->new_password, pwd, E->new_password_len)) {
    tfree (E->new_password, E->new_password_len);
    E->new_password = NULL;
    E->new_password_len = 0;
    vlogprintf (E_ERROR, "passwords do not match\n");
    TLS->callback.get_values (TLS, tgl_new_password, "new password: ", 2, tgl_on_new_pwd, E);
    return;
  }
  tgl_do_act_set_password (TLS, E->current_password, E->current_password_len,
                                E->new_password, E->new_password_len,
                                E->current_salt, E->current_salt_len,
                                E->new_salt, E->new_salt_len,
                                E->hint, E->hint_len,
                                E->callback, E->callback_extra);

  tfree (E->current_password, E->current_password_len);
  tfree (E->new_password, E->new_password_len);
  tfree (E->current_salt, E->current_salt_len);
  tfree (E->new_salt, E->new_salt_len);
  tfree_str (E->hint);
  tfree (E, sizeof (*E));
}

void tgl_on_new_pwd (struct tgl_state *TLS, const char *pwd[], void *_T) {
  struct change_password_extra *E = _T;
  E->new_password_len = strlen (pwd[0]);
  E->new_password = tmemdup (pwd[0], E->new_password_len);
  tgl_on_new2_pwd(TLS, pwd[1], E);
}

void tgl_on_old_pwd (struct tgl_state *TLS, const char *pwd[], void *_T) {
  struct change_password_extra *E = _T;
  E->current_password_len = strlen (pwd[0]);
  E->current_password = tmemdup (pwd[0], E->current_password_len);
  tgl_on_new_pwd(TLS, pwd + 1, E);
}

static int set_get_password_on_answer (struct tgl_state *TLS, struct query *q, void *D) {
  struct tl_ds_account_password *DS_AP = D;

  char *new_hint = q->extra;

  struct change_password_extra *E = talloc0 (sizeof (*E));

  if (DS_AP->current_salt) {
    E->current_salt_len = DS_AP->current_salt->len;
    E->current_salt = tmemdup (DS_AP->current_salt->data, E->current_salt_len);
  }
  if (DS_AP->new_salt) {
    E->new_salt_len = DS_AP->new_salt->len;
    E->new_salt = tmemdup (DS_AP->new_salt->data, E->new_salt_len);
  }

  if (new_hint) {
    E->hint_len = strlen (new_hint);
    E->hint = new_hint;
  }

  E->callback = q->callback;
  E->callback_extra = q->callback_extra;

  if (DS_AP->magic == CODE_account_no_password) {
    TLS->callback.get_values (TLS, tgl_new_password, "new password: ", 2, tgl_on_new_pwd, E);
  } else {
    static char s[512];
    _snprintf (s, 511, "old password (hint %.*s): ", DS_RSTR (DS_AP->hint));
    TLS->callback.get_values (TLS, tgl_cur_and_new_password, s, 3, tgl_on_old_pwd, E);
  }
  return 0;
}

static struct query_methods set_get_password_methods = {
  .on_answer = set_get_password_on_answer,
  .on_error = q_void_on_error,
  .type = TYPE_TO_PARAM(account_password)
};

void tgl_do_set_password (struct tgl_state *TLS, const char *hint, int hint_len, void (*callback)(struct tgl_state *TLS, void *extra, int success), void *callback_extra) {
  clear_packet ();
  out_int (CODE_account_get_password);
  tglq_send_query (TLS, TLS->DC_working, packet_ptr - packet_buffer, packet_buffer, &set_get_password_methods, hint ? tstrndup (hint, hint_len) : NULL, callback, callback_extra);
}

/* }}} */

/* {{{ check password */
static int check_password_on_error (struct tgl_state *TLS, struct query *q, int error_code, int error_len, const char *error) {
  if (error_code == 400) {
    vlogprintf (E_ERROR, "bad password\n");
    tgl_do_check_password (TLS, q->callback, q->callback_extra);
    return 0;
  }
  TLS->locks ^= TGL_LOCK_PASSWORD;
  tgl_set_query_error (TLS, EPROTO, "RPC_CALL_FAIL %d: %.*s", error_code, error_len, error);
  if (q->callback) {
    ((void (*)(struct tgl_state *, void *, int))q->callback)(TLS, q->callback_extra, 0);
  }
  return 0;
}

static int check_password_on_answer (struct tgl_state *TLS, struct query *q, void *D) {
  TLS->locks ^= TGL_LOCK_PASSWORD;
  if (q->callback) {
    ((void (*)(struct tgl_state *, void *, int))q->callback)(TLS, q->callback_extra, 1);
  }
  return 0;
}

static struct query_methods check_password_methods = {
  .on_answer = check_password_on_answer,
  .on_error = check_password_on_error,
  .type = TYPE_TO_PARAM(auth_authorization)
};


struct check_password_extra {
  char *current_salt;
  int current_salt_len;
  void (*callback)(struct tgl_state *, void *, int);
  void *callback_extra;
};

static void tgl_pwd_got (struct tgl_state *TLS, const char *pwd[], void *_T) {
  struct check_password_extra *E = _T;

  clear_packet ();
  static char s[512];
  static unsigned char shab[32];

  assert (E->current_salt_len <= 128);
  assert (strlen (pwd[0]) <= 128);

  out_int (CODE_auth_check_password);

  if (pwd[0] && E->current_salt_len) {
    int l = E->current_salt_len;
    memcpy (s, E->current_salt, l);

    int r = strlen (pwd[0]);
#if defined(_MSC_VER) && _MSC_VER >= 1400
    strcpy_s (s + l, 512 -l, pwd[0]);
#else
    strcpy (s + l, pwd[0]);
#endif

    memcpy (s + l + r, E->current_salt, l);

    SHA256 ((void *)s, 2 * l + r, shab);
    out_cstring ((void *)shab, 32);
  } else {
    out_string ("");
  }

  tglq_send_query (TLS, TLS->DC_working, packet_ptr - packet_buffer, packet_buffer, &check_password_methods, 0, E->callback, E->callback_extra);

  tfree (E->current_salt, E->current_salt_len);
  tfree (E, sizeof (*E));
}

static int check_get_password_on_error (struct tgl_state *TLS, struct query *q, int error_code, int error_len, const char *error) {
  TLS->locks ^= TGL_LOCK_PASSWORD;
  tgl_set_query_error (TLS, EPROTO, "RPC_CALL_FAIL %d: %.*s", error_code, error_len, error);
  if (q->callback) {
    ((void (*)(struct tgl_state *, void *, int))q->callback) (TLS, q->callback_extra, 0);
  }
  return 0;
}

static int check_get_password_on_answer (struct tgl_state *TLS, struct query *q, void *D) {
  struct tl_ds_account_password *DS_AP = D;

  if (DS_AP->magic == CODE_account_no_password) {
    TLS->locks ^= TGL_LOCK_PASSWORD;
    return 0;
  }
  static char s[512];
  _snprintf (s, 511, "type password (hint %.*s): ", DS_RSTR (DS_AP->hint));

  struct check_password_extra *E = talloc0 (sizeof (*E));

  if (DS_AP->current_salt) {
    E->current_salt_len = DS_AP->current_salt->len;
    E->current_salt = tmemdup (DS_AP->current_salt->data, E->current_salt_len);
  }

  E->callback = q->callback;
  E->callback_extra = q->callback_extra;

  TLS->callback.get_values (TLS, tgl_cur_password, s, 1, tgl_pwd_got, E);
  return 0;
}

static struct query_methods check_get_password_methods = {
  .on_answer = check_get_password_on_answer,
  .on_error = check_get_password_on_error,
  .type = TYPE_TO_PARAM(account_password)
};

void tgl_do_check_password (struct tgl_state *TLS, void (*callback)(struct tgl_state *TLS, void *extra, int success), void *callback_extra) {
  clear_packet ();
  out_int (CODE_account_get_password);
  tglq_send_query (TLS, TLS->DC_working, packet_ptr - packet_buffer, packet_buffer, &check_get_password_methods, NULL, callback, callback_extra);
}

/* }}} */

/* {{{ send broadcast */
void tgl_do_send_broadcast (struct tgl_state *TLS, int num, tgl_peer_id_t id[], const char *text, int text_len, unsigned long long flags, void (*callback)(struct tgl_state *TLS, void *extra, int success, int num, struct tgl_message *ML[]), void *callback_extra) {

  assert (num <= 1000);

  struct messages_send_extra *E = talloc0 (sizeof (*E));
  E->multi = 1;
  E->count = num;
  E->list = talloc (sizeof (long long) * num);

  int date = (int)time (0);
  struct tl_ds_message_media TDSM;
  TDSM.magic = CODE_message_media_empty;

  int i;
  for (i = 0; i < num; i++) {
    tglt_secure_random (&E->list[i], 8);

    int peer_type = tgl_get_peer_type (id[i]);
    int peer_id = tgl_get_peer_id (id[i]);
    assert (tgl_get_peer_type (id[i]) == TGL_PEER_USER);

    int disable_preview = flags & TGL_SEND_MSG_FLAG_DISABLE_PREVIEW;
    if (!(flags & TGL_SEND_MSG_FLAG_ENABLE_PREVIEW) && TLS->disable_link_preview) {
      disable_preview = 1;
    }
    if (disable_preview) {
      disable_preview = TGLMF_DISABLE_PREVIEW;
    }

    bl_do_create_message_new (TLS, E->list[i], &TLS->our_id, &peer_type, &peer_id, NULL, NULL, &date, text, text_len, &TDSM, NULL, NULL, NULL, TGLMF_UNREAD | TGLMF_OUT | TGLMF_PENDING | TGLMF_CREATE | TGLMF_CREATED | disable_preview);
  }

  clear_packet ();
  out_int (CODE_messages_send_broadcast);
  out_int (CODE_vector);
  out_int (num);
  for (i = 0; i < num; i++) {
    assert (tgl_get_peer_type (id[i]) == TGL_PEER_USER);

    struct tgl_user *U = (void *)tgl_peer_get (TLS, id[i]);
    if (U && U->access_hash) {
      out_int (CODE_input_user_foreign);
      out_int (tgl_get_peer_id (id[i]));
      out_long (U->access_hash);
    } else {
      out_int (CODE_input_user_contact);
      out_int (tgl_get_peer_id (id[i]));
    }
  }

  out_int (CODE_vector);
  out_int (num);
  for (i = 0; i < num; i++) {
    out_long (E->list[i]);
  }
  out_cstring (text, text_len);

  out_int (CODE_message_media_empty);

  tglq_send_query (TLS, TLS->DC_working, packet_ptr - packet_buffer, packet_buffer, &send_msgs_methods, E, callback, callback_extra);
}
/* }}} */

/* {{{ block user */
static int block_user_on_answer (struct tgl_state *TLS, struct query *q, void *D) {
  if (q->callback) {
    ((void (*)(struct tgl_state *, void *, int))q->callback)(TLS, q->callback_extra, 1);
  }
  return 0;
}

static struct query_methods block_user_methods = {
  .on_answer = block_user_on_answer,
  .on_error = q_void_on_error,
  .type = TYPE_TO_PARAM (bool)
};

void tgl_do_block_user (struct tgl_state *TLS, tgl_peer_id_t id, void (*callback)(struct tgl_state *TLS, void *callback_extra, int success), void *callback_extra) {
  if (tgl_get_peer_type (id) != TGL_PEER_USER) {
    tgl_set_query_error (TLS, EINVAL, "id should be user id");
    if (callback) {
      callback (TLS, callback_extra, 0);
    }
    return;
  }

  clear_packet ();

  out_int (CODE_contacts_block);
  tgl_peer_t *U = tgl_peer_get (TLS, id);
  if (U && U->user.access_hash) {
    out_int (CODE_input_user_foreign);
    out_int (tgl_get_peer_id (id));
    out_long (U->user.access_hash);
  } else {
    out_int (CODE_input_user_contact);
    out_int (tgl_get_peer_id (id));
  }
  tglq_send_query (TLS, TLS->DC_working, packet_ptr - packet_buffer, packet_buffer, &block_user_methods, 0, callback, callback_extra);
}

void tgl_do_unblock_user (struct tgl_state *TLS, tgl_peer_id_t id, void (*callback)(struct tgl_state *TLS, void *callback_extra, int success), void *callback_extra) {
  if (tgl_get_peer_type (id) != TGL_PEER_USER) {
    tgl_set_query_error (TLS, EINVAL, "id should be user id");
    if (callback) {
      callback (TLS, callback_extra, 0);
    }
    return;
  }

  clear_packet ();

  out_int (CODE_contacts_unblock);
  tgl_peer_t *U = tgl_peer_get (TLS, id);
  if (U && U->user.access_hash) {
    out_int (CODE_input_user_foreign);
    out_int (tgl_get_peer_id (id));
    out_long (U->user.access_hash);
  } else {
    out_int (CODE_input_user_contact);
    out_int (tgl_get_peer_id (id));
  }
  tglq_send_query (TLS, TLS->DC_working, packet_ptr - packet_buffer, packet_buffer, &block_user_methods, 0, callback, callback_extra);
}
/* }}} */



static void set_flag_4 (struct tgl_state *TLS, void *_D, int success) {
  struct tgl_dc *D = _D;
  assert (success);
  D->flags |= 4;

  TLS->timer_methods->insert (D->ev, TLS->temp_key_expire_time * 0.9);
}

static int send_bind_temp_on_answer (struct tgl_state *TLS, struct query *q, void *D) {
  struct tgl_dc *DC = q->extra;
  DC->flags |= 2;
  tgl_do_help_get_config_dc (TLS, DC, set_flag_4, DC);
  vlogprintf (E_DEBUG, "Bind successful in dc %d\n", DC->id);
  return 0;
}

static int send_bind_on_error (struct tgl_state *TLS, struct query *q, int error_code, int l, const char *error) {
  vlogprintf (E_WARNING, "bind: error %d: %.*s\n", error_code, l, error);
  if (error_code == 400) {
    return -11;
  }
  return 0;
}

static struct query_methods send_bind_temp_methods = {
  .on_answer = send_bind_temp_on_answer,
  .on_error = send_bind_on_error,
  .type = TYPE_TO_PARAM (bool)
};

void tgl_do_send_bind_temp_key (struct tgl_state *TLS, struct tgl_dc *D, long long nonce, int expires_at, void *data, int len, long long msg_id) {
  clear_packet ();
  out_int (CODE_auth_bind_temp_auth_key);
  out_long (D->auth_key_id);
  out_long (nonce);
  out_int (expires_at);
  out_cstring (data, len);
  struct query *q = tglq_send_query_ex (TLS, D, packet_ptr - packet_buffer, packet_buffer, &send_bind_temp_methods, D, 0, 0, 2);
  assert (q->msg_id == msg_id);
}

static int update_status_on_answer (struct tgl_state *TLS, struct query *q, void *D) {
  if (q->callback) {
    ((void (*)(struct tgl_state *, void *, int))q->callback) (TLS, q->callback_extra, 1);
  }
  return 0;
}

static struct query_methods update_status_methods = {
  .on_answer = update_status_on_answer,
  .on_error = q_void_on_error,
  .type = TYPE_TO_PARAM(bool)
};

void tgl_do_update_status (struct tgl_state *TLS, int online, void (*callback)(struct tgl_state *TLS, void *callback_extra, int success), void *callback_extra) {
  clear_packet ();
  out_int (CODE_account_update_status);
  out_int (online ? CODE_bool_false : CODE_bool_true);
  tglq_send_query (TLS, TLS->DC_working, packet_ptr - packet_buffer, packet_buffer, &update_status_methods, 0, callback, callback_extra);
}


void tgl_do_request_exchange (struct tgl_state *TLS, struct tgl_secret_chat *E) {
  /*static unsigned char s[256];
  tglt_secure_random (s, 256);

  long long id;
  tglt_secure_random (&id, 8);

  //bl_do_encr_chat_exchange_request (TLS, E, id, s);
  int rst =  tgl_sce_requested;
  bl_do_encr_chat_exchange_new (TLS, E, &id, NULL, &rst);

  BIGNUM *a = BN_bin2bn (s, 256, 0);
  ensure_ptr (a);
  BIGNUM *p = BN_bin2bn (TLS->encr_prime, 256, 0);
  ensure_ptr (p);

  BIGNUM *g = BN_new ();
  ensure_ptr (g);

  ensure (BN_set_word (g, TLS->encr_root));

  BIGNUM *r = BN_new ();
  ensure_ptr (r);

  ensure (BN_mod_exp (r, g, a, p, TLS->BN_ctx));

  static unsigned char kk[256];
  memset (kk, 0, sizeof (kk));
  BN_bn2bin (r, kk + (256 - BN_num_bytes (r)));

  BN_clear_free (a);
  BN_clear_free (g);
  BN_clear_free (p);
  BN_clear_free (r);

  static int action[70];
  action[0] = CODE_decrypted_message_action_request_key;
  *(long long *)(action + 1) = E->exchange_id;
  action[3] = 0x100fe;
  memcpy (action + 4, kk, 256);

  long long t;
  tglt_secure_random (&t, 8);

  bl_do_send_message_action_encr (TLS, t, TLS->our_id, tgl_get_peer_type (E->id), tgl_get_peer_id (E->id), time (0), 68, action);

  struct tgl_message *M = tgl_message_get (TLS, t);
  assert (M);
  assert (M->action.type == tgl_message_action_request_key);
  tgl_do_send_msg (TLS, M, 0, 0);*/
}

void tgl_do_accept_exchange (struct tgl_state *TLS, struct tgl_secret_chat *E, long long exchange_id, unsigned char ga[]) {
  /*static unsigned char s[256];
  tglt_secure_random (s, 256);

  BIGNUM *b = BN_bin2bn (s, 256, 0);
  ensure_ptr (b);
  BIGNUM *g_a = BN_bin2bn (ga, 256, 0);
  ensure_ptr (g_a);

  assert (tglmp_check_g_a (TLS, TLS->encr_prime_bn, g_a) >= 0);
  //if (!ctx) {
  //  ctx = BN_CTX_new ();
  //  ensure_ptr (ctx);
  //}
  BIGNUM *p = TLS->encr_prime_bn;
  ensure_ptr (p);
  BIGNUM *r = BN_new ();
  ensure_ptr (r);
  ensure (BN_mod_exp (r, g_a, b, p, TLS->BN_ctx));

  static unsigned char kk[256];
  memset (kk, 0, sizeof (kk));
  BN_bn2bin (r, kk + (256 - BN_num_bytes (r)));

  bl_do_encr_chat_exchange_accept (TLS, E, exchange_id, kk);

  ensure (BN_set_word (g_a, TLS->encr_root));
  ensure (BN_mod_exp (r, g_a, b, p, TLS->BN_ctx));

  static unsigned char buf[256];
  memset (buf, 0, sizeof (buf));
  BN_bn2bin (r, buf + (256 - BN_num_bytes (r)));

  static int action[70];
  action[0] = CODE_decrypted_message_action_accept_key;
  *(long long *)(action + 1) = E->exchange_id;
  action[3] = 0x100fe;
  memcpy (action + 4, buf, 256);
  *(long long *)(action + 68) = E->exchange_key_fingerprint;

  long long t;
  tglt_secure_random (&t, 8);

  bl_do_send_message_action_encr (TLS, t, TLS->our_id, tgl_get_peer_type (E->id), tgl_get_peer_id (E->id), time (0), 70, action);

  BN_clear_free (b);
  BN_clear_free (g_a);
  BN_clear_free (r);

  struct tgl_message *M = tgl_message_get (TLS, t);
  assert (M);
  assert (M->action.type == tgl_message_action_accept_key);
  tgl_do_send_msg (TLS, M, 0, 0);*/
}

void tgl_do_confirm_exchange (struct tgl_state *TLS, struct tgl_secret_chat *E, int sen_nop) {
  /*bl_do_encr_chat_exchange_confirm (TLS, E);
  if (sen_nop) {
    int action = CODE_decrypted_message_action_noop;

    long long t;
    tglt_secure_random (&t, 8);

    bl_do_send_message_action_encr (TLS, t, TLS->our_id, tgl_get_peer_type (E->id), tgl_get_peer_id (E->id), time (0), 1, &action);

    struct tgl_message *M = tgl_message_get (TLS, t);
    assert (M);
    assert (M->action.type == tgl_message_action_noop);
    tgl_do_send_msg (TLS, M, 0, 0);
  }*/
}

void tgl_do_commit_exchange (struct tgl_state *TLS, struct tgl_secret_chat *E, unsigned char gb[]) {
  /*assert (TLS->encr_prime);

  BIGNUM *g_b = BN_bin2bn (gb, 256, 0);
  ensure_ptr (g_b);
  assert (tglmp_check_g_a (TLS, TLS->encr_prime_bn, g_b) >= 0);

  BIGNUM *p = TLS->encr_prime_bn;
  ensure_ptr (p);
  BIGNUM *r = BN_new ();
  ensure_ptr (r);
  BIGNUM *a = BN_bin2bn ((void *)E->exchange_key, 256, 0);
  ensure_ptr (a);
  ensure (BN_mod_exp (r, g_b, a, p, TLS->BN_ctx));

  static unsigned char s[256];
  memset (s, 0, 256);

  BN_bn2bin (r, s + (256 - BN_num_bytes (r)));

  BN_clear_free (g_b);
  BN_clear_free (r);
  BN_clear_free (a);

  static unsigned char sh[20];
  SHA1 (s, 256, sh);

  int action[4];
  action[0] = CODE_decrypted_message_action_commit_key;
  *(long long *)(action + 1) = E->exchange_id;
  *(long long *)(action + 3) = *(long long *)(sh + 12);

  long long t;
  tglt_secure_random (&t, 8);

  bl_do_send_message_action_encr (TLS, t, TLS->our_id, tgl_get_peer_type (E->id), tgl_get_peer_id (E->id), time (0), 5, action);

  struct tgl_message *M = tgl_message_get (TLS, t);
  assert (M);
  assert (M->action.type == tgl_message_action_commit_key);
  tgl_do_send_msg (TLS, M, 0, 0);

  bl_do_encr_chat_exchange_commit (TLS, E, s);*/
}

void tgl_do_abort_exchange (struct tgl_state *TLS, struct tgl_secret_chat *E) {
  //bl_do_encr_chat_exchange_abort (TLS, E);
}

void tgl_started_cb (struct tgl_state *TLS, void *arg, int success) {
  assert (success);
  TLS->started = 1;
  if (TLS->callback.started) {
    TLS->callback.started (TLS);
  }
}

void tgl_export_auth_callback (struct tgl_state *TLS, void *arg, int success) {
  assert (success);
  int i;
  for (i = 0; i <= TLS->max_dc_num; i++) if (TLS->DC_list[i] && !tgl_signed_dc (TLS, TLS->DC_list[i])) {
    return;
  }
  if (TLS->callback.logged_in) {
    TLS->callback.logged_in (TLS);
  }

  tglm_send_all_unsent (TLS);
  tgl_do_get_difference (TLS, 0, tgl_started_cb, 0);
}

void tgl_export_all_auth (struct tgl_state *TLS) {
  int i;
  int ok = 1;
  for (i = 0; i <= TLS->max_dc_num; i++) if (TLS->DC_list[i] && !tgl_signed_dc (TLS, TLS->DC_list[i])) {
    tgl_do_export_auth (TLS, i, tgl_export_auth_callback, TLS->DC_list[i]);
    ok = 0;
  }
  if (ok) {
    if (TLS->callback.logged_in) {
      TLS->callback.logged_in (TLS);
    }

    tglm_send_all_unsent (TLS);
    tgl_do_get_difference (TLS, 0, tgl_started_cb, 0);
  }
}

struct sign_up_extra {
  char *phone;
  char *hash;
  char *first_name;
  char *last_name;
  int phone_len;
  int hash_len;
  int first_name_len;
  int last_name_len;
};

void tgl_sign_in_code (struct tgl_state *TLS, const char *code[], void *_T);
void tgl_sign_in_result (struct tgl_state *TLS, void *_T, int success, struct tgl_user *U) {
  struct sign_up_extra *E = _T;
  if (success) {
    tfree (E->phone, E->phone_len);
    tfree (E->hash, E->hash_len);
    tfree (E, sizeof (*E));
  } else {
    vlogprintf (E_ERROR, "incorrect code\n");
    TLS->callback.get_values (TLS, tgl_code, "code ('call' for phone call):", 1, tgl_sign_in_code, E);
    return;
  }
  tgl_export_all_auth (TLS);
}

void tgl_sign_in_code (struct tgl_state *TLS, const char *code[], void *_T) {
  struct sign_up_extra *E = _T;
  if (!strcmp (code[0], "call")) {
    tgl_do_phone_call (TLS, E->phone, E->phone_len, E->hash, E->hash_len, 0, 0);
    TLS->callback.get_values (TLS, tgl_code, "code ('call' for phone call):", 1, tgl_sign_in_code, E);
    return;
  }

  tgl_do_send_code_result (TLS, E->phone, E->phone_len, E->hash, E->hash_len, code[0], strlen (code[0]), tgl_sign_in_result, E);
}

void tgl_sign_up_code (struct tgl_state *TLS, const char *code[], void *_T);
void tgl_sign_up_result (struct tgl_state *TLS, void *_T, int success, struct tgl_user *U) {
  struct sign_up_extra *E = _T;
  if (success) {
    tfree (E->phone, E->phone_len);
    tfree (E->hash, E->hash_len);
    tfree (E->first_name, E->first_name_len);
    tfree (E->last_name, E->last_name_len);
    tfree (E, sizeof (*E));
  } else {
    vlogprintf (E_ERROR, "incorrect code\n");
    TLS->callback.get_values (TLS, tgl_code, "code ('call' for phone call):", 1, tgl_sign_up_code, E);
    return;
  }
  tgl_export_all_auth (TLS);
}

void tgl_sign_up_code (struct tgl_state *TLS, const char *code[], void *_T) {
  struct sign_up_extra *E = _T;
  if (!strcmp (code[0], "call")) {
    tgl_do_phone_call (TLS, E->phone, E->phone_len, E->hash, E->hash_len, 0, 0);
    TLS->callback.get_values (TLS, tgl_code, "code ('call' for phone call):", 1, tgl_sign_up_code, E);
    return;
  }

  tgl_do_send_code_result_auth (TLS, E->phone, E->phone_len, E->hash, E->hash_len, code[0], strlen (code[0]), E->first_name, E->first_name_len, E->last_name, E->last_name_len, tgl_sign_up_result, E);
}


void tgl_set_last_name (struct tgl_state *TLS, const char *last_name, struct sign_up_extra *E) {
  E->last_name_len = strlen (last_name);
  E->last_name = tmemdup (last_name, E->last_name_len);
}

int tgl_set_first_name (struct tgl_state *TLS, const char *first_name, struct sign_up_extra *E) {
  if (strlen (first_name) < 1) {
    return -1;
  }

  E->first_name_len = strlen (first_name);
  E->first_name = tmemdup (first_name, E->first_name_len);
  return 0;
}

void tgl_register_cb (struct tgl_state *TLS, const char *rinfo[], void *_T) {
  struct sign_up_extra *E = _T;
  const char *yn = rinfo[0];
  if (strlen (yn) > 1) {
    TLS->callback.get_values (TLS, tgl_register_info, "registration info:", 3, tgl_register_cb, E);
  } else if (strlen (yn) == 0 || *yn == 'y' || *yn == 'Y') {
    if (!tgl_set_first_name(TLS, rinfo[1], E)) {
      tgl_set_last_name(TLS, rinfo[2], E);
      TLS->callback.get_values (TLS, tgl_code, "code ('call' for phone call):", 1, tgl_sign_up_code, E);
    }
    else {
      TLS->callback.get_values (TLS, tgl_register_info, "registration info:", 3, tgl_register_cb, E);
    }
  } else if (*yn == 'n' || *yn == 'N') {
    vlogprintf (E_ERROR, "stopping registration");
    tfree (E->phone, E->phone_len);
    tfree (E->hash, E->hash_len);
    tfree (E, sizeof (*E));
    tgl_login (TLS);
  } else {
    TLS->callback.get_values (TLS, tgl_register_info, "registration info:", 3, tgl_register_cb, E);
  }
}

void tgl_sign_in_phone (struct tgl_state *TLS, const char *phone[], void *arg);
void tgl_sign_in_phone_cb (struct tgl_state *TLS, void *extra, int success, int registered, const char *mhash) {
  struct sign_up_extra *E = extra;
  if (!success) {
    vlogprintf (E_ERROR, "Incorrect phone number\n");

    tfree (E->phone, E->phone_len);
    tfree (E, sizeof (*E));
    TLS->callback.get_values (TLS, tgl_phone_number, "phone number:", 1, tgl_sign_in_phone, NULL);
    return;
  }

  E->hash_len = strlen (mhash);
  E->hash = tmemdup (mhash, E->hash_len);

  if (registered) {
    TLS->callback.get_values (TLS, tgl_code, "code ('call' for phone call):", 1, tgl_sign_in_code, E);
  } else {
    TLS->callback.get_values (TLS, tgl_register_info, "registration info:", 3, tgl_register_cb, E);
  }
}

void tgl_sign_in_phone (struct tgl_state *TLS, const char *phone[], void *arg) {
  struct sign_up_extra *E = talloc0 (sizeof (*E));
  E->phone_len = strlen (phone[0]);
  E->phone = tmemdup (phone[0], E->phone_len);

  tgl_do_send_code (TLS, E->phone, E->phone_len, tgl_sign_in_phone_cb, E);
}

void tgl_bot_hash_cb (struct tgl_state *TLS, const char *code[], void *arg);

void tgl_sign_in_bot_cb (struct tgl_state *TLS, void *_T, int success, struct tgl_user *U) {
  if (!success) {
    vlogprintf (E_ERROR, "incorrect bot hash\n");
    TLS->callback.get_values (TLS, tgl_bot_hash, "bot hash:", 1, tgl_bot_hash_cb, NULL);
    return;
  }
  tgl_export_all_auth (TLS);
}

void tgl_bot_hash_cb (struct tgl_state *TLS, const char *code[], void *arg) {
  tgl_do_send_bot_auth (TLS, code[0], strlen (code[0]), tgl_sign_in_bot_cb, NULL);
}

void tgl_sign_in (struct tgl_state *TLS) {
  if (!tgl_signed_dc (TLS, TLS->DC_working)) {
    if (TLS->is_bot) {
      TLS->callback.get_values (TLS, tgl_bot_hash, "bot hash:", 1, tgl_bot_hash_cb, NULL);
    } else {
      TLS->callback.get_values (TLS, tgl_phone_number, "phone number:", 1, tgl_sign_in_phone, NULL);
    }
  } else {
    tgl_export_all_auth (TLS);
  }
}

static void check_authorized (struct tgl_state *TLS, void *arg) {
  int i;
  int ok = 1;
  for (i = 0; i <= TLS->max_dc_num; i++) {
    if (TLS->DC_list[i] && !tgl_signed_dc (TLS, TLS->DC_list[i]) && !tgl_authorized_dc (TLS, TLS->DC_list[i])) {
      ok = 0;
      break;
    }
  }

  if (ok) {
    TLS->timer_methods->free (TLS->ev_login);
    TLS->ev_login = NULL;
    tgl_sign_in (TLS);
  } else {
    TLS->timer_methods->insert (TLS->ev_login, 0.1);
  }
}

void tgl_login (struct tgl_state *TLS) {
  int i;
  int ok = 1;
  for (i = 0; i <= TLS->max_dc_num; i++) {
    if (TLS->DC_list[i] && !tgl_signed_dc (TLS, TLS->DC_list[i]) && !tgl_authorized_dc (TLS, TLS->DC_list[i])) {
      ok = 0;
      break;
    }
  }

  if (!ok) {
    TLS->ev_login = TLS->timer_methods->alloc (TLS, check_authorized, NULL);
    TLS->timer_methods->insert (TLS->ev_login, 0.1);
  } else {
    tgl_sign_in (TLS);
  }
}
