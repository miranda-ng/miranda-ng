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

    Copyright Vitaly Valtman 2014-2015
*/

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "tgl.h"
#include "tools.h"
#include "mtproto-client.h"
#include "tgl-structures.h"
//#include "net.h"

#include <assert.h>

struct tgl_state tgl_state;

    
void tgl_set_binlog_mode (struct tgl_state *TLS, int mode) {
  TLS->binlog_enabled = mode;
}

void tgl_set_binlog_path (struct tgl_state *TLS, const char *path) {
  TLS->binlog_name = tstrdup (path);
}
    
void tgl_set_auth_file_path (struct tgl_state *TLS, const char *path) {
  TLS->auth_file = tstrdup (path);
}

void tgl_set_download_directory (struct tgl_state *TLS, const char *path) {
  TLS->downloads_directory = tstrdup (path);
}

void tgl_set_callback (struct tgl_state *TLS, struct tgl_update_callback *cb) {
  TLS->callback = *cb;
}

void tgl_set_rsa_key (struct tgl_state *TLS, const char *key) {
  assert (TLS->rsa_key_num < TGL_MAX_RSA_KEYS_NUM);
  TLS->rsa_key_list[TLS->rsa_key_num ++] = tstrdup (key);
}

void tgl_init (struct tgl_state *TLS) {
  assert (TLS->timer_methods);
  assert (TLS->net_methods);
  if (!TLS->callback.create_print_name) {
    TLS->callback.create_print_name = tgls_default_create_print_name;
  }
  if (!TLS->temp_key_expire_time) {
    TLS->temp_key_expire_time = 100000;
  }

  TLS->message_list.next_use = &TLS->message_list;
  TLS->message_list.prev_use = &TLS->message_list;

  tglmp_on_start (TLS);
  
  if (!TLS->app_id) {
    TLS->app_id = TG_APP_ID;
    TLS->app_hash = tstrdup (TG_APP_HASH);
  }
}

int tgl_authorized_dc (struct tgl_state *TLS, struct tgl_dc *DC) {
  assert (DC);
  return (DC->flags & 4) != 0;//DC->auth_key_id;
}

int tgl_signed_dc (struct tgl_state *TLS, struct tgl_dc *DC) {
  assert (DC);
  return (DC->flags & TGLDCF_LOGGED_IN) != 0;
}

void tgl_register_app_id (struct tgl_state *TLS, int app_id, const char *app_hash) {
  TLS->app_id = app_id;
  TLS->app_hash = tstrdup (app_hash);
}

struct tgl_state *tgl_state_alloc (void) {
  struct tgl_state *TLS = (struct tgl_state *)malloc (sizeof (*TLS));
  if (!TLS) { return NULL; }
  memset (TLS, 0, sizeof (*TLS));
  return TLS;
}

void tgl_incr_verbosity (struct tgl_state *TLS) {
  TLS->verbosity ++;
}

void tgl_set_verbosity (struct tgl_state *TLS, int val) {
  TLS->verbosity = val;
}

void tgl_enable_pfs (struct tgl_state *TLS) {
  TLS->enable_pfs = 1;
}

void tgl_set_test_mode (struct tgl_state *TLS) {
  TLS->test_mode ++;
}

void tgl_set_net_methods (struct tgl_state *TLS, struct tgl_net_methods *methods) {
  TLS->net_methods = methods;
}

void tgl_set_timer_methods (struct tgl_state *TLS, struct tgl_timer_methods *methods) {
  TLS->timer_methods = methods;
}

void tgl_set_ev_base (struct tgl_state *TLS, void *ev_base) {
  TLS->ev_base = ev_base;
}

void tgl_set_app_version (struct tgl_state *TLS, const char *app_version) {
  if (TLS->app_version) {
    tfree_str (TLS->app_version);
  }
  TLS->app_version = tstrdup (app_version);
}

void tgl_enable_ipv6 (struct tgl_state *TLS) {
  TLS->ipv6_enabled = 1;
}

void tgl_disable_link_preview (struct tgl_state *TLS) {
  TLS->disable_link_preview = 1;
}

void tgl_enable_bot (struct tgl_state *TLS) {
  TLS->is_bot = 1;
}
