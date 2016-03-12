#ifdef _MSC_VER
#include "..\auto.h"
#include "auto-fetch-ds.h"
#include "auto-skip.h"
#include "auto-types.h"
#include "..\mtproto-common.h"
#else
#include "auto.h"
#include "auto/auto-fetch-ds.h"
#include "auto/auto-skip.h"
#include "auto/auto-types.h"
#include "auto-static-fetch-ds.c"
#include "mtproto-common.h"
#endif
#include <assert.h>
struct tl_ds_account_days_t_t_l *fetch_ds_constructor_account_days_t_t_l (struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0xb8d0afdf && T->type->name != 0x472f5020)) { return 0; }
  struct tl_ds_account_days_t_t_l *  result = talloc0 (sizeof (*result));
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->days = fetch_ds_type_bare_int (field1);
  return result;
}
struct tl_ds_audio *fetch_ds_constructor_audio_empty (struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x9fc5ec4e && T->type->name != 0x603a13b1)) { return 0; }
  struct tl_ds_audio *  result = talloc0 (sizeof (*result));
  result->magic = 0x586988d8;
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xddf89345, .id = "Bare_Long", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->id = fetch_ds_type_bare_long (field1);
  return result;
}
struct tl_ds_audio *fetch_ds_constructor_audio (struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x9fc5ec4e && T->type->name != 0x603a13b1)) { return 0; }
  struct tl_ds_audio *  result = talloc0 (sizeof (*result));
  result->magic = 0xc7ac6496;
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xddf89345, .id = "Bare_Long", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->id = fetch_ds_type_bare_long (field1);
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xddf89345, .id = "Bare_Long", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->access_hash = fetch_ds_type_bare_long (field2);
  struct paramed_type *field3 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->user_id = fetch_ds_type_bare_int (field3);
  struct paramed_type *field4 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->date = fetch_ds_type_bare_int (field4);
  struct paramed_type *field5 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->duration = fetch_ds_type_bare_int (field5);
  struct paramed_type *field6 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->mime_type = fetch_ds_type_bare_string (field6);
  struct paramed_type *field7 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->size = fetch_ds_type_bare_int (field7);
  struct paramed_type *field8 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->dc_id = fetch_ds_type_bare_int (field8);
  return result;
}
struct tl_ds_authorization *fetch_ds_constructor_authorization (struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x7bf2e6f6 && T->type->name != 0x840d1909)) { return 0; }
  struct tl_ds_authorization *  result = talloc0 (sizeof (*result));
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xddf89345, .id = "Bare_Long", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->hash = fetch_ds_type_bare_long (field1);
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->flags = fetch_ds_type_bare_int (field2);
  struct paramed_type *field3 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->device_model = fetch_ds_type_bare_string (field3);
  struct paramed_type *field4 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->platform = fetch_ds_type_bare_string (field4);
  struct paramed_type *field5 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->system_version = fetch_ds_type_bare_string (field5);
  struct paramed_type *field6 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->api_id = fetch_ds_type_bare_int (field6);
  struct paramed_type *field7 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->app_name = fetch_ds_type_bare_string (field7);
  struct paramed_type *field8 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->app_version = fetch_ds_type_bare_string (field8);
  struct paramed_type *field9 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->date_created = fetch_ds_type_bare_int (field9);
  struct paramed_type *field10 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->date_active = fetch_ds_type_bare_int (field10);
  struct paramed_type *field11 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->ip = fetch_ds_type_bare_string (field11);
  struct paramed_type *field12 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->country = fetch_ds_type_bare_string (field12);
  struct paramed_type *field13 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->region = fetch_ds_type_bare_string (field13);
  return result;
}
struct tl_ds_bool *fetch_ds_constructor_bool_false (struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x250be282 && T->type->name != 0xdaf41d7d)) { return 0; }
  struct tl_ds_bool *  result = talloc0 (sizeof (*result));
  result->magic = 0xbc799737;
  return result;
}
struct tl_ds_bool *fetch_ds_constructor_bool_true (struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x250be282 && T->type->name != 0xdaf41d7d)) { return 0; }
  struct tl_ds_bool *  result = talloc0 (sizeof (*result));
  result->magic = 0x997275b5;
  return result;
}
struct tl_ds_bot_command *fetch_ds_constructor_bot_command (struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x75e7ea6c && T->type->name != 0x8a181593)) { return 0; }
  struct tl_ds_bot_command *  result = talloc0 (sizeof (*result));
  result->magic = 0xc27ac8c7;
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->command = fetch_ds_type_bare_string (field1);
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->description = fetch_ds_type_bare_string (field2);
  return result;
}
struct tl_ds_bot_command *fetch_ds_constructor_bot_command_old (struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x75e7ea6c && T->type->name != 0x8a181593)) { return 0; }
  struct tl_ds_bot_command *  result = talloc0 (sizeof (*result));
  result->magic = 0xb79d22ab;
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->command = fetch_ds_type_bare_string (field1);
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->params = fetch_ds_type_bare_string (field2);
  struct paramed_type *field3 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->description = fetch_ds_type_bare_string (field3);
  return result;
}
struct tl_ds_bot_info *fetch_ds_constructor_bot_info_empty (struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0xb2e16f93 && T->type->name != 0x4d1e906c)) { return 0; }
  struct tl_ds_bot_info *  result = talloc0 (sizeof (*result));
  result->magic = 0xbb2e37ce;
  return result;
}
struct tl_ds_bot_info *fetch_ds_constructor_bot_info (struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0xb2e16f93 && T->type->name != 0x4d1e906c)) { return 0; }
  struct tl_ds_bot_info *  result = talloc0 (sizeof (*result));
  result->magic = 0x09cf585d;
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->user_id = fetch_ds_type_bare_int (field1);
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->version = fetch_ds_type_bare_int (field2);
  struct paramed_type *field3 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->share_text = fetch_ds_type_bare_string (field3);
  struct paramed_type *field4 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->description = fetch_ds_type_bare_string (field4);
  struct paramed_type *field5 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x1cb5c415, .id = "Vector", .params_num = 1, .params_types = 0},
    .params = (struct paramed_type *[]){
      &(struct paramed_type){
        .type = &(struct tl_type_descr) {.name = 0x75e7ea6c, .id = "BotCommand", .params_num = 0, .params_types = 0},
        .params = 0,
      },
    }
  };
  result->commands = (void *)fetch_ds_type_vector (field5);
  return result;
}
struct tl_ds_string *fetch_ds_constructor_bytes (struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x0ee1379f && T->type->name != 0xf11ec860)) { return 0; }
  struct tl_ds_string *  result = talloc0 (sizeof (*result));
  assert (in_remaining () >= 4);
  int l = prefetch_strlen ();
  assert (l >= 0);
  result->len = l;
  result->data = talloc (l + 1);
  result->data[l] = 0;
  memcpy (result->data, fetch_str (l), l);
  return result;
}
struct tl_ds_chat *fetch_ds_constructor_chat_empty (struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x7bd865dc && T->type->name != 0x84279a23)) { return 0; }
  struct tl_ds_chat *  result = talloc0 (sizeof (*result));
  result->magic = 0x9ba2d800;
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->id = fetch_ds_type_bare_int (field1);
  return result;
}
struct tl_ds_chat *fetch_ds_constructor_chat (struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x7bd865dc && T->type->name != 0x84279a23)) { return 0; }
  struct tl_ds_chat *  result = talloc0 (sizeof (*result));
  result->magic = 0x6e9c9bc7;
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->id = fetch_ds_type_bare_int (field1);
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->title = fetch_ds_type_bare_string (field2);
  struct paramed_type *field3 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x56922676, .id = "ChatPhoto", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->photo = fetch_ds_type_chat_photo (field3);
  struct paramed_type *field4 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->participants_count = fetch_ds_type_bare_int (field4);
  struct paramed_type *field5 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->date = fetch_ds_type_bare_int (field5);
  struct paramed_type *field6 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x250be282, .id = "Bool", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->left = fetch_ds_type_bool (field6);
  struct paramed_type *field7 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->version = fetch_ds_type_bare_int (field7);
  return result;
}
struct tl_ds_chat *fetch_ds_constructor_chat_forbidden (struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x7bd865dc && T->type->name != 0x84279a23)) { return 0; }
  struct tl_ds_chat *  result = talloc0 (sizeof (*result));
  result->magic = 0xfb0ccc41;
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->id = fetch_ds_type_bare_int (field1);
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->title = fetch_ds_type_bare_string (field2);
  struct paramed_type *field3 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->date = fetch_ds_type_bare_int (field3);
  return result;
}
struct tl_ds_chat *fetch_ds_constructor_geo_chat (struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x7bd865dc && T->type->name != 0x84279a23)) { return 0; }
  struct tl_ds_chat *  result = talloc0 (sizeof (*result));
  result->magic = 0x75eaea5a;
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->id = fetch_ds_type_bare_int (field1);
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xddf89345, .id = "Bare_Long", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->access_hash = fetch_ds_type_bare_long (field2);
  struct paramed_type *field3 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->title = fetch_ds_type_bare_string (field3);
  struct paramed_type *field4 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->address = fetch_ds_type_bare_string (field4);
  struct paramed_type *field5 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->venue = fetch_ds_type_bare_string (field5);
  struct paramed_type *field6 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x315e0a53, .id = "GeoPoint", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->geo = fetch_ds_type_geo_point (field6);
  struct paramed_type *field7 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x56922676, .id = "ChatPhoto", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->photo = fetch_ds_type_chat_photo (field7);
  struct paramed_type *field8 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->participants_count = fetch_ds_type_bare_int (field8);
  struct paramed_type *field9 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->date = fetch_ds_type_bare_int (field9);
  struct paramed_type *field10 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x250be282, .id = "Bool", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->checked_in = fetch_ds_type_bool (field10);
  struct paramed_type *field11 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->version = fetch_ds_type_bare_int (field11);
  return result;
}
struct tl_ds_chat_full *fetch_ds_constructor_chat_full (struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x2e02a614 && T->type->name != 0xd1fd59eb)) { return 0; }
  struct tl_ds_chat_full *  result = talloc0 (sizeof (*result));
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->id = fetch_ds_type_bare_int (field1);
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x77930f9f, .id = "ChatParticipants", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->participants = fetch_ds_type_chat_participants (field2);
  struct paramed_type *field3 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xc207550a, .id = "Photo", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->chat_photo = fetch_ds_type_photo (field3);
  struct paramed_type *field4 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xfdf894fc, .id = "PeerNotifySettings", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->notify_settings = fetch_ds_type_peer_notify_settings (field4);
  struct paramed_type *field5 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x95f132d5, .id = "ExportedChatInvite", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->exported_invite = fetch_ds_type_exported_chat_invite (field5);
  struct paramed_type *field6 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x1cb5c415, .id = "Vector", .params_num = 1, .params_types = 0},
    .params = (struct paramed_type *[]){
      &(struct paramed_type){
        .type = &(struct tl_type_descr) {.name = 0xb2e16f93, .id = "BotInfo", .params_num = 0, .params_types = 0},
        .params = 0,
      },
    }
  };
  result->bot_info = (void *)fetch_ds_type_vector (field6);
  return result;
}
struct tl_ds_chat_invite *fetch_ds_constructor_chat_invite_already (struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x94f910b1 && T->type->name != 0x6b06ef4e)) { return 0; }
  struct tl_ds_chat_invite *  result = talloc0 (sizeof (*result));
  result->magic = 0x5a686d7c;
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x7bd865dc, .id = "Chat", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->chat = fetch_ds_type_chat (field1);
  return result;
}
struct tl_ds_chat_invite *fetch_ds_constructor_chat_invite (struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x94f910b1 && T->type->name != 0x6b06ef4e)) { return 0; }
  struct tl_ds_chat_invite *  result = talloc0 (sizeof (*result));
  result->magic = 0xce917dcd;
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->title = fetch_ds_type_bare_string (field1);
  return result;
}
struct tl_ds_chat_located *fetch_ds_constructor_chat_located (struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x3631cf4c && T->type->name != 0xc9ce30b3)) { return 0; }
  struct tl_ds_chat_located *  result = talloc0 (sizeof (*result));
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->chat_id = fetch_ds_type_bare_int (field1);
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->distance = fetch_ds_type_bare_int (field2);
  return result;
}
struct tl_ds_chat_participant *fetch_ds_constructor_chat_participant (struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0xc8d7493e && T->type->name != 0x3728b6c1)) { return 0; }
  struct tl_ds_chat_participant *  result = talloc0 (sizeof (*result));
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->user_id = fetch_ds_type_bare_int (field1);
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->inviter_id = fetch_ds_type_bare_int (field2);
  struct paramed_type *field3 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->date = fetch_ds_type_bare_int (field3);
  return result;
}
struct tl_ds_chat_participants *fetch_ds_constructor_chat_participants_forbidden (struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x77930f9f && T->type->name != 0x886cf060)) { return 0; }
  struct tl_ds_chat_participants *  result = talloc0 (sizeof (*result));
  result->magic = 0x0fd2bb8a;
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->chat_id = fetch_ds_type_bare_int (field1);
  return result;
}
struct tl_ds_chat_participants *fetch_ds_constructor_chat_participants (struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x77930f9f && T->type->name != 0x886cf060)) { return 0; }
  struct tl_ds_chat_participants *  result = talloc0 (sizeof (*result));
  result->magic = 0x7841b415;
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->chat_id = fetch_ds_type_bare_int (field1);
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->admin_id = fetch_ds_type_bare_int (field2);
  struct paramed_type *field3 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x1cb5c415, .id = "Vector", .params_num = 1, .params_types = 0},
    .params = (struct paramed_type *[]){
      &(struct paramed_type){
        .type = &(struct tl_type_descr) {.name = 0xc8d7493e, .id = "ChatParticipant", .params_num = 0, .params_types = 0},
        .params = 0,
      },
    }
  };
  result->participants = (void *)fetch_ds_type_vector (field3);
  struct paramed_type *field4 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->version = fetch_ds_type_bare_int (field4);
  return result;
}
struct tl_ds_chat_photo *fetch_ds_constructor_chat_photo_empty (struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x56922676 && T->type->name != 0xa96dd989)) { return 0; }
  struct tl_ds_chat_photo *  result = talloc0 (sizeof (*result));
  result->magic = 0x37c1011c;
  return result;
}
struct tl_ds_chat_photo *fetch_ds_constructor_chat_photo (struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x56922676 && T->type->name != 0xa96dd989)) { return 0; }
  struct tl_ds_chat_photo *  result = talloc0 (sizeof (*result));
  result->magic = 0x6153276a;
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x2f8ffb30, .id = "FileLocation", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->photo_small = fetch_ds_type_file_location (field1);
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x2f8ffb30, .id = "FileLocation", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->photo_big = fetch_ds_type_file_location (field2);
  return result;
}
struct tl_ds_client_d_h_inner_data *fetch_ds_constructor_client_d_h_inner_data (struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x6643b654 && T->type->name != 0x99bc49ab)) { return 0; }
  struct tl_ds_client_d_h_inner_data *  result = talloc0 (sizeof (*result));
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x82c93bc6, .id = "Bare_Int128", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->nonce = fetch_ds_type_bare_int128 (field1);
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x82c93bc6, .id = "Bare_Int128", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->server_nonce = fetch_ds_type_bare_int128 (field2);
  struct paramed_type *field3 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xddf89345, .id = "Bare_Long", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->retry_id = fetch_ds_type_bare_long (field3);
  struct paramed_type *field4 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->g_b = fetch_ds_type_bare_string (field4);
  return result;
}
struct tl_ds_config *fetch_ds_constructor_config (struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x4e32b894 && T->type->name != 0xb1cd476b)) { return 0; }
  struct tl_ds_config *  result = talloc0 (sizeof (*result));
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->date = fetch_ds_type_bare_int (field1);
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->expires = fetch_ds_type_bare_int (field2);
  struct paramed_type *field3 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x250be282, .id = "Bool", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->test_mode = fetch_ds_type_bool (field3);
  struct paramed_type *field4 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->this_dc = fetch_ds_type_bare_int (field4);
  struct paramed_type *field5 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x1cb5c415, .id = "Vector", .params_num = 1, .params_types = 0},
    .params = (struct paramed_type *[]){
      &(struct paramed_type){
        .type = &(struct tl_type_descr) {.name = 0x2b1a62f0, .id = "DcOption", .params_num = 0, .params_types = 0},
        .params = 0,
      },
    }
  };
  result->dc_options = (void *)fetch_ds_type_vector (field5);
  struct paramed_type *field6 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->chat_size_max = fetch_ds_type_bare_int (field6);
  struct paramed_type *field7 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->broadcast_size_max = fetch_ds_type_bare_int (field7);
  struct paramed_type *field8 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->forwarded_count_max = fetch_ds_type_bare_int (field8);
  struct paramed_type *field9 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->online_update_period_ms = fetch_ds_type_bare_int (field9);
  struct paramed_type *field10 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->offline_blur_timeout_ms = fetch_ds_type_bare_int (field10);
  struct paramed_type *field11 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->offline_idle_timeout_ms = fetch_ds_type_bare_int (field11);
  struct paramed_type *field12 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->online_cloud_timeout_ms = fetch_ds_type_bare_int (field12);
  struct paramed_type *field13 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->notify_cloud_delay_ms = fetch_ds_type_bare_int (field13);
  struct paramed_type *field14 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->notify_default_delay_ms = fetch_ds_type_bare_int (field14);
  struct paramed_type *field15 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->chat_big_size = fetch_ds_type_bare_int (field15);
  struct paramed_type *field16 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->push_chat_period_ms = fetch_ds_type_bare_int (field16);
  struct paramed_type *field17 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->push_chat_limit = fetch_ds_type_bare_int (field17);
  struct paramed_type *field18 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x1cb5c415, .id = "Vector", .params_num = 1, .params_types = 0},
    .params = (struct paramed_type *[]){
      &(struct paramed_type){
        .type = &(struct tl_type_descr) {.name = 0xae636f24, .id = "DisabledFeature", .params_num = 0, .params_types = 0},
        .params = 0,
      },
    }
  };
  result->disabled_features = (void *)fetch_ds_type_vector (field18);
  return result;
}
struct tl_ds_contact *fetch_ds_constructor_contact (struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0xf911c994 && T->type->name != 0x06ee366b)) { return 0; }
  struct tl_ds_contact *  result = talloc0 (sizeof (*result));
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->user_id = fetch_ds_type_bare_int (field1);
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x250be282, .id = "Bool", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->mutual = fetch_ds_type_bool (field2);
  return result;
}
struct tl_ds_contact_blocked *fetch_ds_constructor_contact_blocked (struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x561bc879 && T->type->name != 0xa9e43786)) { return 0; }
  struct tl_ds_contact_blocked *  result = talloc0 (sizeof (*result));
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->user_id = fetch_ds_type_bare_int (field1);
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->date = fetch_ds_type_bare_int (field2);
  return result;
}
struct tl_ds_contact_found *fetch_ds_constructor_contact_found (struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0xea879f95 && T->type->name != 0x1578606a)) { return 0; }
  struct tl_ds_contact_found *  result = talloc0 (sizeof (*result));
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->user_id = fetch_ds_type_bare_int (field1);
  return result;
}
struct tl_ds_contact_link *fetch_ds_constructor_contact_link_unknown (struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x522fbc63 && T->type->name != 0xadd0439c)) { return 0; }
  struct tl_ds_contact_link *  result = talloc0 (sizeof (*result));
  result->magic = 0x5f4f9247;
  return result;
}
struct tl_ds_contact_link *fetch_ds_constructor_contact_link_none (struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x522fbc63 && T->type->name != 0xadd0439c)) { return 0; }
  struct tl_ds_contact_link *  result = talloc0 (sizeof (*result));
  result->magic = 0xfeedd3ad;
  return result;
}
struct tl_ds_contact_link *fetch_ds_constructor_contact_link_has_phone (struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x522fbc63 && T->type->name != 0xadd0439c)) { return 0; }
  struct tl_ds_contact_link *  result = talloc0 (sizeof (*result));
  result->magic = 0x268f3f59;
  return result;
}
struct tl_ds_contact_link *fetch_ds_constructor_contact_link_contact (struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x522fbc63 && T->type->name != 0xadd0439c)) { return 0; }
  struct tl_ds_contact_link *  result = talloc0 (sizeof (*result));
  result->magic = 0xd502c2d0;
  return result;
}
struct tl_ds_contact_status *fetch_ds_constructor_contact_status (struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0xd3680c61 && T->type->name != 0x2c97f39e)) { return 0; }
  struct tl_ds_contact_status *  result = talloc0 (sizeof (*result));
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->user_id = fetch_ds_type_bare_int (field1);
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x76de9570, .id = "UserStatus", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->status = fetch_ds_type_user_status (field2);
  return result;
}
struct tl_ds_contact_suggested *fetch_ds_constructor_contact_suggested (struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x3de191a1 && T->type->name != 0xc21e6e5e)) { return 0; }
  struct tl_ds_contact_suggested *  result = talloc0 (sizeof (*result));
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->user_id = fetch_ds_type_bare_int (field1);
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->mutual_contacts = fetch_ds_type_bare_int (field2);
  return result;
}
struct tl_ds_dc_option *fetch_ds_constructor_dc_option_l28 (struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x2b1a62f0 && T->type->name != 0xd4e59d0f)) { return 0; }
  struct tl_ds_dc_option *  result = talloc0 (sizeof (*result));
  result->magic = 0x2ec2a43c;
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->id = fetch_ds_type_bare_int (field1);
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->hostname = fetch_ds_type_bare_string (field2);
  struct paramed_type *field3 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->ip_address = fetch_ds_type_bare_string (field3);
  struct paramed_type *field4 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->port = fetch_ds_type_bare_int (field4);
  return result;
}
struct tl_ds_dc_option *fetch_ds_constructor_dc_option (struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x2b1a62f0 && T->type->name != 0xd4e59d0f)) { return 0; }
  struct tl_ds_dc_option *  result = talloc0 (sizeof (*result));
  result->magic = 0x05d8c6cc;
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->flags = fetch_ds_type_bare_int (field1);
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->id = fetch_ds_type_bare_int (field2);
  struct paramed_type *field3 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->ip_address = fetch_ds_type_bare_string (field3);
  struct paramed_type *field4 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->port = fetch_ds_type_bare_int (field4);
  return result;
}
struct tl_ds_decrypted_message *fetch_ds_constructor_decrypted_message (struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x535b7918 && T->type->name != 0xaca486e7)) { return 0; }
  struct tl_ds_decrypted_message *  result = talloc0 (sizeof (*result));
  result->magic = 0x204d3878;
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xddf89345, .id = "Bare_Long", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->random_id = fetch_ds_type_bare_long (field1);
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->ttl = fetch_ds_type_bare_int (field2);
  struct paramed_type *field3 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->message = fetch_ds_type_bare_string (field3);
  struct paramed_type *field4 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x34e0d674, .id = "DecryptedMessageMedia", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->media = fetch_ds_type_decrypted_message_media (field4);
  return result;
}
struct tl_ds_decrypted_message *fetch_ds_constructor_decrypted_message_service (struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x535b7918 && T->type->name != 0xaca486e7)) { return 0; }
  struct tl_ds_decrypted_message *  result = talloc0 (sizeof (*result));
  result->magic = 0x73164160;
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xddf89345, .id = "Bare_Long", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->random_id = fetch_ds_type_bare_long (field1);
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x4e0eefde, .id = "DecryptedMessageAction", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->action = fetch_ds_type_decrypted_message_action (field2);
  return result;
}
struct tl_ds_decrypted_message_action *fetch_ds_constructor_decrypted_message_action_set_message_t_t_l (struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x4e0eefde && T->type->name != 0xb1f11021)) { return 0; }
  struct tl_ds_decrypted_message_action *  result = talloc0 (sizeof (*result));
  result->magic = 0xa1733aec;
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->ttl_seconds = fetch_ds_type_bare_int (field1);
  return result;
}
struct tl_ds_decrypted_message_action *fetch_ds_constructor_decrypted_message_action_read_messages (struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x4e0eefde && T->type->name != 0xb1f11021)) { return 0; }
  struct tl_ds_decrypted_message_action *  result = talloc0 (sizeof (*result));
  result->magic = 0x0c4f40be;
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x1cb5c415, .id = "Vector", .params_num = 1, .params_types = 0},
    .params = (struct paramed_type *[]){
      &(struct paramed_type){
        .type = &(struct tl_type_descr) {.name = 0xddf89345, .id = "Bare_Long", .params_num = 0, .params_types = 0},
        .params = 0,
      },
    }
  };
  result->random_ids = (void *)fetch_ds_type_vector (field1);
  return result;
}
struct tl_ds_decrypted_message_action *fetch_ds_constructor_decrypted_message_action_delete_messages (struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x4e0eefde && T->type->name != 0xb1f11021)) { return 0; }
  struct tl_ds_decrypted_message_action *  result = talloc0 (sizeof (*result));
  result->magic = 0x65614304;
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x1cb5c415, .id = "Vector", .params_num = 1, .params_types = 0},
    .params = (struct paramed_type *[]){
      &(struct paramed_type){
        .type = &(struct tl_type_descr) {.name = 0xddf89345, .id = "Bare_Long", .params_num = 0, .params_types = 0},
        .params = 0,
      },
    }
  };
  result->random_ids = (void *)fetch_ds_type_vector (field1);
  return result;
}
struct tl_ds_decrypted_message_action *fetch_ds_constructor_decrypted_message_action_screenshot_messages (struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x4e0eefde && T->type->name != 0xb1f11021)) { return 0; }
  struct tl_ds_decrypted_message_action *  result = talloc0 (sizeof (*result));
  result->magic = 0x8ac1f475;
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x1cb5c415, .id = "Vector", .params_num = 1, .params_types = 0},
    .params = (struct paramed_type *[]){
      &(struct paramed_type){
        .type = &(struct tl_type_descr) {.name = 0xddf89345, .id = "Bare_Long", .params_num = 0, .params_types = 0},
        .params = 0,
      },
    }
  };
  result->random_ids = (void *)fetch_ds_type_vector (field1);
  return result;
}
struct tl_ds_decrypted_message_action *fetch_ds_constructor_decrypted_message_action_flush_history (struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x4e0eefde && T->type->name != 0xb1f11021)) { return 0; }
  struct tl_ds_decrypted_message_action *  result = talloc0 (sizeof (*result));
  result->magic = 0x6719e45c;
  return result;
}
struct tl_ds_decrypted_message_action *fetch_ds_constructor_decrypted_message_action_resend (struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x4e0eefde && T->type->name != 0xb1f11021)) { return 0; }
  struct tl_ds_decrypted_message_action *  result = talloc0 (sizeof (*result));
  result->magic = 0x511110b0;
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->start_seq_no = fetch_ds_type_bare_int (field1);
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->end_seq_no = fetch_ds_type_bare_int (field2);
  return result;
}
struct tl_ds_decrypted_message_action *fetch_ds_constructor_decrypted_message_action_notify_layer (struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x4e0eefde && T->type->name != 0xb1f11021)) { return 0; }
  struct tl_ds_decrypted_message_action *  result = talloc0 (sizeof (*result));
  result->magic = 0xf3048883;
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->layer = fetch_ds_type_bare_int (field1);
  return result;
}
struct tl_ds_decrypted_message_action *fetch_ds_constructor_decrypted_message_action_typing (struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x4e0eefde && T->type->name != 0xb1f11021)) { return 0; }
  struct tl_ds_decrypted_message_action *  result = talloc0 (sizeof (*result));
  result->magic = 0xccb27641;
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x70541c7e, .id = "SendMessageAction", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->action = fetch_ds_type_send_message_action (field1);
  return result;
}
struct tl_ds_decrypted_message_action *fetch_ds_constructor_decrypted_message_action_request_key (struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x4e0eefde && T->type->name != 0xb1f11021)) { return 0; }
  struct tl_ds_decrypted_message_action *  result = talloc0 (sizeof (*result));
  result->magic = 0xf3c9611b;
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xddf89345, .id = "Bare_Long", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->exchange_id = fetch_ds_type_bare_long (field1);
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xf11ec860, .id = "Bare_Bytes", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->g_a = fetch_ds_type_bare_bytes (field2);
  return result;
}
struct tl_ds_decrypted_message_action *fetch_ds_constructor_decrypted_message_action_accept_key (struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x4e0eefde && T->type->name != 0xb1f11021)) { return 0; }
  struct tl_ds_decrypted_message_action *  result = talloc0 (sizeof (*result));
  result->magic = 0x6fe1735b;
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xddf89345, .id = "Bare_Long", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->exchange_id = fetch_ds_type_bare_long (field1);
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xf11ec860, .id = "Bare_Bytes", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->g_b = fetch_ds_type_bare_bytes (field2);
  struct paramed_type *field3 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xddf89345, .id = "Bare_Long", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->key_fingerprint = fetch_ds_type_bare_long (field3);
  return result;
}
struct tl_ds_decrypted_message_action *fetch_ds_constructor_decrypted_message_action_abort_key (struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x4e0eefde && T->type->name != 0xb1f11021)) { return 0; }
  struct tl_ds_decrypted_message_action *  result = talloc0 (sizeof (*result));
  result->magic = 0xdd05ec6b;
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xddf89345, .id = "Bare_Long", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->exchange_id = fetch_ds_type_bare_long (field1);
  return result;
}
struct tl_ds_decrypted_message_action *fetch_ds_constructor_decrypted_message_action_commit_key (struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x4e0eefde && T->type->name != 0xb1f11021)) { return 0; }
  struct tl_ds_decrypted_message_action *  result = talloc0 (sizeof (*result));
  result->magic = 0xec2e0b9b;
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xddf89345, .id = "Bare_Long", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->exchange_id = fetch_ds_type_bare_long (field1);
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xddf89345, .id = "Bare_Long", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->key_fingerprint = fetch_ds_type_bare_long (field2);
  return result;
}
struct tl_ds_decrypted_message_action *fetch_ds_constructor_decrypted_message_action_noop (struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x4e0eefde && T->type->name != 0xb1f11021)) { return 0; }
  struct tl_ds_decrypted_message_action *  result = talloc0 (sizeof (*result));
  result->magic = 0xa82fdd63;
  return result;
}
struct tl_ds_decrypted_message_layer *fetch_ds_constructor_decrypted_message_layer (struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x1be31789 && T->type->name != 0xe41ce876)) { return 0; }
  struct tl_ds_decrypted_message_layer *  result = talloc0 (sizeof (*result));
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xf11ec860, .id = "Bare_Bytes", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->random_bytes = fetch_ds_type_bare_bytes (field1);
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->layer = fetch_ds_type_bare_int (field2);
  struct paramed_type *field3 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->in_seq_no = fetch_ds_type_bare_int (field3);
  struct paramed_type *field4 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->out_seq_no = fetch_ds_type_bare_int (field4);
  struct paramed_type *field5 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x535b7918, .id = "DecryptedMessage", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->message = fetch_ds_type_decrypted_message (field5);
  return result;
}
struct tl_ds_decrypted_message_media *fetch_ds_constructor_decrypted_message_media_empty (struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x34e0d674 && T->type->name != 0xcb1f298b)) { return 0; }
  struct tl_ds_decrypted_message_media *  result = talloc0 (sizeof (*result));
  result->magic = 0x089f5c4a;
  return result;
}
struct tl_ds_decrypted_message_media *fetch_ds_constructor_decrypted_message_media_photo (struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x34e0d674 && T->type->name != 0xcb1f298b)) { return 0; }
  struct tl_ds_decrypted_message_media *  result = talloc0 (sizeof (*result));
  result->magic = 0x32798a8c;
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xf11ec860, .id = "Bare_Bytes", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->str_thumb = fetch_ds_type_bare_bytes (field1);
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->thumb_w = fetch_ds_type_bare_int (field2);
  struct paramed_type *field3 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->thumb_h = fetch_ds_type_bare_int (field3);
  struct paramed_type *field4 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->w = fetch_ds_type_bare_int (field4);
  struct paramed_type *field5 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->h = fetch_ds_type_bare_int (field5);
  struct paramed_type *field6 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->size = fetch_ds_type_bare_int (field6);
  struct paramed_type *field7 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xf11ec860, .id = "Bare_Bytes", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->key = fetch_ds_type_bare_bytes (field7);
  struct paramed_type *field8 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xf11ec860, .id = "Bare_Bytes", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->iv = fetch_ds_type_bare_bytes (field8);
  return result;
}
struct tl_ds_decrypted_message_media *fetch_ds_constructor_decrypted_message_media_geo_point (struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x34e0d674 && T->type->name != 0xcb1f298b)) { return 0; }
  struct tl_ds_decrypted_message_media *  result = talloc0 (sizeof (*result));
  result->magic = 0x35480a59;
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xddef3eab, .id = "Bare_Double", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->latitude = fetch_ds_type_bare_double (field1);
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xddef3eab, .id = "Bare_Double", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->longitude = fetch_ds_type_bare_double (field2);
  return result;
}
struct tl_ds_decrypted_message_media *fetch_ds_constructor_decrypted_message_media_contact (struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x34e0d674 && T->type->name != 0xcb1f298b)) { return 0; }
  struct tl_ds_decrypted_message_media *  result = talloc0 (sizeof (*result));
  result->magic = 0x588a0a97;
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->phone_number = fetch_ds_type_bare_string (field1);
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->first_name = fetch_ds_type_bare_string (field2);
  struct paramed_type *field3 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->last_name = fetch_ds_type_bare_string (field3);
  struct paramed_type *field4 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->user_id = fetch_ds_type_bare_int (field4);
  return result;
}
struct tl_ds_decrypted_message_media *fetch_ds_constructor_decrypted_message_media_document (struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x34e0d674 && T->type->name != 0xcb1f298b)) { return 0; }
  struct tl_ds_decrypted_message_media *  result = talloc0 (sizeof (*result));
  result->magic = 0xb095434b;
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xf11ec860, .id = "Bare_Bytes", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->str_thumb = fetch_ds_type_bare_bytes (field1);
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->thumb_w = fetch_ds_type_bare_int (field2);
  struct paramed_type *field3 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->thumb_h = fetch_ds_type_bare_int (field3);
  struct paramed_type *field4 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->file_name = fetch_ds_type_bare_string (field4);
  struct paramed_type *field5 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->mime_type = fetch_ds_type_bare_string (field5);
  struct paramed_type *field6 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->size = fetch_ds_type_bare_int (field6);
  struct paramed_type *field7 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xf11ec860, .id = "Bare_Bytes", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->key = fetch_ds_type_bare_bytes (field7);
  struct paramed_type *field8 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xf11ec860, .id = "Bare_Bytes", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->iv = fetch_ds_type_bare_bytes (field8);
  return result;
}
struct tl_ds_decrypted_message_media *fetch_ds_constructor_decrypted_message_media_video (struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x34e0d674 && T->type->name != 0xcb1f298b)) { return 0; }
  struct tl_ds_decrypted_message_media *  result = talloc0 (sizeof (*result));
  result->magic = 0x524a415d;
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xf11ec860, .id = "Bare_Bytes", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->str_thumb = fetch_ds_type_bare_bytes (field1);
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->thumb_w = fetch_ds_type_bare_int (field2);
  struct paramed_type *field3 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->thumb_h = fetch_ds_type_bare_int (field3);
  struct paramed_type *field4 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->duration = fetch_ds_type_bare_int (field4);
  struct paramed_type *field5 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->mime_type = fetch_ds_type_bare_string (field5);
  struct paramed_type *field6 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->w = fetch_ds_type_bare_int (field6);
  struct paramed_type *field7 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->h = fetch_ds_type_bare_int (field7);
  struct paramed_type *field8 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->size = fetch_ds_type_bare_int (field8);
  struct paramed_type *field9 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xf11ec860, .id = "Bare_Bytes", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->key = fetch_ds_type_bare_bytes (field9);
  struct paramed_type *field10 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xf11ec860, .id = "Bare_Bytes", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->iv = fetch_ds_type_bare_bytes (field10);
  return result;
}
struct tl_ds_decrypted_message_media *fetch_ds_constructor_decrypted_message_media_audio (struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x34e0d674 && T->type->name != 0xcb1f298b)) { return 0; }
  struct tl_ds_decrypted_message_media *  result = talloc0 (sizeof (*result));
  result->magic = 0x57e0a9cb;
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->duration = fetch_ds_type_bare_int (field1);
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->mime_type = fetch_ds_type_bare_string (field2);
  struct paramed_type *field3 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->size = fetch_ds_type_bare_int (field3);
  struct paramed_type *field4 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xf11ec860, .id = "Bare_Bytes", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->key = fetch_ds_type_bare_bytes (field4);
  struct paramed_type *field5 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xf11ec860, .id = "Bare_Bytes", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->iv = fetch_ds_type_bare_bytes (field5);
  return result;
}
struct tl_ds_decrypted_message_media *fetch_ds_constructor_decrypted_message_media_external_document (struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x34e0d674 && T->type->name != 0xcb1f298b)) { return 0; }
  struct tl_ds_decrypted_message_media *  result = talloc0 (sizeof (*result));
  result->magic = 0xfa95b0dd;
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xddf89345, .id = "Bare_Long", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->id = fetch_ds_type_bare_long (field1);
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xddf89345, .id = "Bare_Long", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->access_hash = fetch_ds_type_bare_long (field2);
  struct paramed_type *field3 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->date = fetch_ds_type_bare_int (field3);
  struct paramed_type *field4 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->mime_type = fetch_ds_type_bare_string (field4);
  struct paramed_type *field5 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->size = fetch_ds_type_bare_int (field5);
  struct paramed_type *field6 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x900f60dd, .id = "PhotoSize", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->thumb = fetch_ds_type_photo_size (field6);
  struct paramed_type *field7 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->dc_id = fetch_ds_type_bare_int (field7);
  struct paramed_type *field8 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x1cb5c415, .id = "Vector", .params_num = 1, .params_types = 0},
    .params = (struct paramed_type *[]){
      &(struct paramed_type){
        .type = &(struct tl_type_descr) {.name = 0x97c637a3, .id = "DocumentAttribute", .params_num = 0, .params_types = 0},
        .params = 0,
      },
    }
  };
  result->attributes = (void *)fetch_ds_type_vector (field8);
  return result;
}
struct tl_ds_decrypted_message_media *fetch_ds_constructor_decrypted_message_media_video_l12 (struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x34e0d674 && T->type->name != 0xcb1f298b)) { return 0; }
  struct tl_ds_decrypted_message_media *  result = talloc0 (sizeof (*result));
  result->magic = 0x4cee6ef3;
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xf11ec860, .id = "Bare_Bytes", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->str_thumb = fetch_ds_type_bare_bytes (field1);
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->thumb_w = fetch_ds_type_bare_int (field2);
  struct paramed_type *field3 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->thumb_h = fetch_ds_type_bare_int (field3);
  struct paramed_type *field4 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->duration = fetch_ds_type_bare_int (field4);
  struct paramed_type *field5 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->w = fetch_ds_type_bare_int (field5);
  struct paramed_type *field6 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->h = fetch_ds_type_bare_int (field6);
  struct paramed_type *field7 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->size = fetch_ds_type_bare_int (field7);
  struct paramed_type *field8 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xf11ec860, .id = "Bare_Bytes", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->key = fetch_ds_type_bare_bytes (field8);
  struct paramed_type *field9 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xf11ec860, .id = "Bare_Bytes", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->iv = fetch_ds_type_bare_bytes (field9);
  return result;
}
struct tl_ds_decrypted_message_media *fetch_ds_constructor_decrypted_message_media_audio_l12 (struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x34e0d674 && T->type->name != 0xcb1f298b)) { return 0; }
  struct tl_ds_decrypted_message_media *  result = talloc0 (sizeof (*result));
  result->magic = 0x6080758f;
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->duration = fetch_ds_type_bare_int (field1);
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->size = fetch_ds_type_bare_int (field2);
  struct paramed_type *field3 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xf11ec860, .id = "Bare_Bytes", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->key = fetch_ds_type_bare_bytes (field3);
  struct paramed_type *field4 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xf11ec860, .id = "Bare_Bytes", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->iv = fetch_ds_type_bare_bytes (field4);
  return result;
}
struct tl_ds_dialog *fetch_ds_constructor_dialog (struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0xc1dd804a && T->type->name != 0x3e227fb5)) { return 0; }
  struct tl_ds_dialog *  result = talloc0 (sizeof (*result));
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x276159d6, .id = "Peer", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->peer = fetch_ds_type_peer (field1);
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->top_message = fetch_ds_type_bare_int (field2);
  struct paramed_type *field3 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->read_inbox_max_id = fetch_ds_type_bare_int (field3);
  struct paramed_type *field4 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->unread_count = fetch_ds_type_bare_int (field4);
  struct paramed_type *field5 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xfdf894fc, .id = "PeerNotifySettings", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->notify_settings = fetch_ds_type_peer_notify_settings (field5);
  return result;
}
struct tl_ds_disabled_feature *fetch_ds_constructor_disabled_feature (struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0xae636f24 && T->type->name != 0x519c90db)) { return 0; }
  struct tl_ds_disabled_feature *  result = talloc0 (sizeof (*result));
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->feature = fetch_ds_type_bare_string (field1);
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->description = fetch_ds_type_bare_string (field2);
  return result;
}
struct tl_ds_document *fetch_ds_constructor_document_empty (struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x51a73418 && T->type->name != 0xae58cbe7)) { return 0; }
  struct tl_ds_document *  result = talloc0 (sizeof (*result));
  result->magic = 0x36f8c871;
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xddf89345, .id = "Bare_Long", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->id = fetch_ds_type_bare_long (field1);
  return result;
}
struct tl_ds_document *fetch_ds_constructor_document (struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x51a73418 && T->type->name != 0xae58cbe7)) { return 0; }
  struct tl_ds_document *  result = talloc0 (sizeof (*result));
  result->magic = 0xf9a39f4f;
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xddf89345, .id = "Bare_Long", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->id = fetch_ds_type_bare_long (field1);
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xddf89345, .id = "Bare_Long", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->access_hash = fetch_ds_type_bare_long (field2);
  struct paramed_type *field3 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->date = fetch_ds_type_bare_int (field3);
  struct paramed_type *field4 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->mime_type = fetch_ds_type_bare_string (field4);
  struct paramed_type *field5 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->size = fetch_ds_type_bare_int (field5);
  struct paramed_type *field6 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x900f60dd, .id = "PhotoSize", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->thumb = fetch_ds_type_photo_size (field6);
  struct paramed_type *field7 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->dc_id = fetch_ds_type_bare_int (field7);
  struct paramed_type *field8 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x1cb5c415, .id = "Vector", .params_num = 1, .params_types = 0},
    .params = (struct paramed_type *[]){
      &(struct paramed_type){
        .type = &(struct tl_type_descr) {.name = 0x97c637a3, .id = "DocumentAttribute", .params_num = 0, .params_types = 0},
        .params = 0,
      },
    }
  };
  result->attributes = (void *)fetch_ds_type_vector (field8);
  return result;
}
struct tl_ds_document *fetch_ds_constructor_document_l19 (struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x51a73418 && T->type->name != 0xae58cbe7)) { return 0; }
  struct tl_ds_document *  result = talloc0 (sizeof (*result));
  result->magic = 0x9efc6326;
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xddf89345, .id = "Bare_Long", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->id = fetch_ds_type_bare_long (field1);
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xddf89345, .id = "Bare_Long", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->access_hash = fetch_ds_type_bare_long (field2);
  struct paramed_type *field3 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->user_id = fetch_ds_type_bare_int (field3);
  struct paramed_type *field4 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->date = fetch_ds_type_bare_int (field4);
  struct paramed_type *field5 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->file_name = fetch_ds_type_bare_string (field5);
  struct paramed_type *field6 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->mime_type = fetch_ds_type_bare_string (field6);
  struct paramed_type *field7 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->size = fetch_ds_type_bare_int (field7);
  struct paramed_type *field8 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x900f60dd, .id = "PhotoSize", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->thumb = fetch_ds_type_photo_size (field8);
  struct paramed_type *field9 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->dc_id = fetch_ds_type_bare_int (field9);
  return result;
}
struct tl_ds_document_attribute *fetch_ds_constructor_document_attribute_image_size (struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x97c637a3 && T->type->name != 0x6839c85c)) { return 0; }
  struct tl_ds_document_attribute *  result = talloc0 (sizeof (*result));
  result->magic = 0x6c37c15c;
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->w = fetch_ds_type_bare_int (field1);
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->h = fetch_ds_type_bare_int (field2);
  return result;
}
struct tl_ds_document_attribute *fetch_ds_constructor_document_attribute_animated (struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x97c637a3 && T->type->name != 0x6839c85c)) { return 0; }
  struct tl_ds_document_attribute *  result = talloc0 (sizeof (*result));
  result->magic = 0x11b58939;
  return result;
}
struct tl_ds_document_attribute *fetch_ds_constructor_document_attribute_sticker_l28 (struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x97c637a3 && T->type->name != 0x6839c85c)) { return 0; }
  struct tl_ds_document_attribute *  result = talloc0 (sizeof (*result));
  result->magic = 0x994c9882;
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->alt = fetch_ds_type_bare_string (field1);
  return result;
}
struct tl_ds_document_attribute *fetch_ds_constructor_document_attribute_sticker (struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x97c637a3 && T->type->name != 0x6839c85c)) { return 0; }
  struct tl_ds_document_attribute *  result = talloc0 (sizeof (*result));
  result->magic = 0x3a556302;
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->alt = fetch_ds_type_bare_string (field1);
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xe44d415c, .id = "InputStickerSet", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->stickerset = fetch_ds_type_input_sticker_set (field2);
  return result;
}
struct tl_ds_document_attribute *fetch_ds_constructor_document_attribute_video (struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x97c637a3 && T->type->name != 0x6839c85c)) { return 0; }
  struct tl_ds_document_attribute *  result = talloc0 (sizeof (*result));
  result->magic = 0x5910cccb;
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->duration = fetch_ds_type_bare_int (field1);
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->w = fetch_ds_type_bare_int (field2);
  struct paramed_type *field3 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->h = fetch_ds_type_bare_int (field3);
  return result;
}
struct tl_ds_document_attribute *fetch_ds_constructor_document_attribute_audio (struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x97c637a3 && T->type->name != 0x6839c85c)) { return 0; }
  struct tl_ds_document_attribute *  result = talloc0 (sizeof (*result));
  result->magic = 0x051448e5;
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->duration = fetch_ds_type_bare_int (field1);
  return result;
}
struct tl_ds_document_attribute *fetch_ds_constructor_document_attribute_filename (struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x97c637a3 && T->type->name != 0x6839c85c)) { return 0; }
  struct tl_ds_document_attribute *  result = talloc0 (sizeof (*result));
  result->magic = 0x15590068;
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->file_name = fetch_ds_type_bare_string (field1);
  return result;
}
double *fetch_ds_constructor_double (struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x2210c154 && T->type->name != 0xddef3eab)) { return 0; }
  double *  result = talloc0 (sizeof (*result));
  assert (in_remaining () >= 8);
  *result = fetch_double ();
  return result;
}
struct tl_ds_encrypted_chat *fetch_ds_constructor_encrypted_chat_empty (struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0xb1718213 && T->type->name != 0x4e8e7dec)) { return 0; }
  struct tl_ds_encrypted_chat *  result = talloc0 (sizeof (*result));
  result->magic = 0xab7ec0a0;
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->id = fetch_ds_type_bare_int (field1);
  return result;
}
struct tl_ds_encrypted_chat *fetch_ds_constructor_encrypted_chat_waiting (struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0xb1718213 && T->type->name != 0x4e8e7dec)) { return 0; }
  struct tl_ds_encrypted_chat *  result = talloc0 (sizeof (*result));
  result->magic = 0x3bf703dc;
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->id = fetch_ds_type_bare_int (field1);
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xddf89345, .id = "Bare_Long", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->access_hash = fetch_ds_type_bare_long (field2);
  struct paramed_type *field3 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->date = fetch_ds_type_bare_int (field3);
  struct paramed_type *field4 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->admin_id = fetch_ds_type_bare_int (field4);
  struct paramed_type *field5 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->participant_id = fetch_ds_type_bare_int (field5);
  return result;
}
struct tl_ds_encrypted_chat *fetch_ds_constructor_encrypted_chat_requested (struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0xb1718213 && T->type->name != 0x4e8e7dec)) { return 0; }
  struct tl_ds_encrypted_chat *  result = talloc0 (sizeof (*result));
  result->magic = 0xc878527e;
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->id = fetch_ds_type_bare_int (field1);
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xddf89345, .id = "Bare_Long", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->access_hash = fetch_ds_type_bare_long (field2);
  struct paramed_type *field3 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->date = fetch_ds_type_bare_int (field3);
  struct paramed_type *field4 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->admin_id = fetch_ds_type_bare_int (field4);
  struct paramed_type *field5 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->participant_id = fetch_ds_type_bare_int (field5);
  struct paramed_type *field6 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xf11ec860, .id = "Bare_Bytes", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->g_a = fetch_ds_type_bare_bytes (field6);
  return result;
}
struct tl_ds_encrypted_chat *fetch_ds_constructor_encrypted_chat (struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0xb1718213 && T->type->name != 0x4e8e7dec)) { return 0; }
  struct tl_ds_encrypted_chat *  result = talloc0 (sizeof (*result));
  result->magic = 0xfa56ce36;
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->id = fetch_ds_type_bare_int (field1);
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xddf89345, .id = "Bare_Long", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->access_hash = fetch_ds_type_bare_long (field2);
  struct paramed_type *field3 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->date = fetch_ds_type_bare_int (field3);
  struct paramed_type *field4 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->admin_id = fetch_ds_type_bare_int (field4);
  struct paramed_type *field5 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->participant_id = fetch_ds_type_bare_int (field5);
  struct paramed_type *field6 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xf11ec860, .id = "Bare_Bytes", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->g_a_or_b = fetch_ds_type_bare_bytes (field6);
  struct paramed_type *field7 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xddf89345, .id = "Bare_Long", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->key_fingerprint = fetch_ds_type_bare_long (field7);
  return result;
}
struct tl_ds_encrypted_chat *fetch_ds_constructor_encrypted_chat_discarded (struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0xb1718213 && T->type->name != 0x4e8e7dec)) { return 0; }
  struct tl_ds_encrypted_chat *  result = talloc0 (sizeof (*result));
  result->magic = 0x13d6dd27;
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->id = fetch_ds_type_bare_int (field1);
  return result;
}
struct tl_ds_encrypted_file *fetch_ds_constructor_encrypted_file_empty (struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x886fd032 && T->type->name != 0x77902fcd)) { return 0; }
  struct tl_ds_encrypted_file *  result = talloc0 (sizeof (*result));
  result->magic = 0xc21f497e;
  return result;
}
struct tl_ds_encrypted_file *fetch_ds_constructor_encrypted_file (struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x886fd032 && T->type->name != 0x77902fcd)) { return 0; }
  struct tl_ds_encrypted_file *  result = talloc0 (sizeof (*result));
  result->magic = 0x4a70994c;
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xddf89345, .id = "Bare_Long", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->id = fetch_ds_type_bare_long (field1);
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xddf89345, .id = "Bare_Long", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->access_hash = fetch_ds_type_bare_long (field2);
  struct paramed_type *field3 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->size = fetch_ds_type_bare_int (field3);
  struct paramed_type *field4 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->dc_id = fetch_ds_type_bare_int (field4);
  struct paramed_type *field5 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->key_fingerprint = fetch_ds_type_bare_int (field5);
  return result;
}
struct tl_ds_encrypted_message *fetch_ds_constructor_encrypted_message (struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0xce6b8a1e && T->type->name != 0x319475e1)) { return 0; }
  struct tl_ds_encrypted_message *  result = talloc0 (sizeof (*result));
  result->magic = 0xed18c118;
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xddf89345, .id = "Bare_Long", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->random_id = fetch_ds_type_bare_long (field1);
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->chat_id = fetch_ds_type_bare_int (field2);
  struct paramed_type *field3 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->date = fetch_ds_type_bare_int (field3);
  struct paramed_type *field4 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xf11ec860, .id = "Bare_Bytes", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->bytes = fetch_ds_type_bare_bytes (field4);
  struct paramed_type *field5 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x886fd032, .id = "EncryptedFile", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->file = fetch_ds_type_encrypted_file (field5);
  return result;
}
struct tl_ds_encrypted_message *fetch_ds_constructor_encrypted_message_service (struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0xce6b8a1e && T->type->name != 0x319475e1)) { return 0; }
  struct tl_ds_encrypted_message *  result = talloc0 (sizeof (*result));
  result->magic = 0x23734b06;
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xddf89345, .id = "Bare_Long", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->random_id = fetch_ds_type_bare_long (field1);
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->chat_id = fetch_ds_type_bare_int (field2);
  struct paramed_type *field3 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->date = fetch_ds_type_bare_int (field3);
  struct paramed_type *field4 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xf11ec860, .id = "Bare_Bytes", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->bytes = fetch_ds_type_bare_bytes (field4);
  return result;
}
struct tl_ds_error *fetch_ds_constructor_error (struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0xc4b9f9bb && T->type->name != 0x3b460644)) { return 0; }
  struct tl_ds_error *  result = talloc0 (sizeof (*result));
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->code = fetch_ds_type_bare_int (field1);
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->text = fetch_ds_type_bare_string (field2);
  return result;
}
struct tl_ds_exported_chat_invite *fetch_ds_constructor_chat_invite_empty (struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x95f132d5 && T->type->name != 0x6a0ecd2a)) { return 0; }
  struct tl_ds_exported_chat_invite *  result = talloc0 (sizeof (*result));
  result->magic = 0x69df3769;
  return result;
}
struct tl_ds_exported_chat_invite *fetch_ds_constructor_chat_invite_exported (struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x95f132d5 && T->type->name != 0x6a0ecd2a)) { return 0; }
  struct tl_ds_exported_chat_invite *  result = talloc0 (sizeof (*result));
  result->magic = 0xfc2e05bc;
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->link = fetch_ds_type_bare_string (field1);
  return result;
}
struct tl_ds_file_location *fetch_ds_constructor_file_location_unavailable (struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x2f8ffb30 && T->type->name != 0xd07004cf)) { return 0; }
  struct tl_ds_file_location *  result = talloc0 (sizeof (*result));
  result->magic = 0x7c596b46;
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xddf89345, .id = "Bare_Long", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->volume_id = fetch_ds_type_bare_long (field1);
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->local_id = fetch_ds_type_bare_int (field2);
  struct paramed_type *field3 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xddf89345, .id = "Bare_Long", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->secret = fetch_ds_type_bare_long (field3);
  return result;
}
struct tl_ds_file_location *fetch_ds_constructor_file_location (struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x2f8ffb30 && T->type->name != 0xd07004cf)) { return 0; }
  struct tl_ds_file_location *  result = talloc0 (sizeof (*result));
  result->magic = 0x53d69076;
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->dc_id = fetch_ds_type_bare_int (field1);
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xddf89345, .id = "Bare_Long", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->volume_id = fetch_ds_type_bare_long (field2);
  struct paramed_type *field3 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->local_id = fetch_ds_type_bare_int (field3);
  struct paramed_type *field4 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xddf89345, .id = "Bare_Long", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->secret = fetch_ds_type_bare_long (field4);
  return result;
}
struct tl_ds_geo_chat_message *fetch_ds_constructor_geo_chat_message_empty (struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0xf67b4034 && T->type->name != 0x0984bfcb)) { return 0; }
  struct tl_ds_geo_chat_message *  result = talloc0 (sizeof (*result));
  result->magic = 0x60311a9b;
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->chat_id = fetch_ds_type_bare_int (field1);
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->id = fetch_ds_type_bare_int (field2);
  return result;
}
struct tl_ds_geo_chat_message *fetch_ds_constructor_geo_chat_message (struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0xf67b4034 && T->type->name != 0x0984bfcb)) { return 0; }
  struct tl_ds_geo_chat_message *  result = talloc0 (sizeof (*result));
  result->magic = 0x4505f8e1;
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->chat_id = fetch_ds_type_bare_int (field1);
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->id = fetch_ds_type_bare_int (field2);
  struct paramed_type *field3 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->from_id = fetch_ds_type_bare_int (field3);
  struct paramed_type *field4 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->date = fetch_ds_type_bare_int (field4);
  struct paramed_type *field5 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->message = fetch_ds_type_bare_string (field5);
  struct paramed_type *field6 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x95f2471a, .id = "MessageMedia", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->media = fetch_ds_type_message_media (field6);
  return result;
}
struct tl_ds_geo_chat_message *fetch_ds_constructor_geo_chat_message_service (struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0xf67b4034 && T->type->name != 0x0984bfcb)) { return 0; }
  struct tl_ds_geo_chat_message *  result = talloc0 (sizeof (*result));
  result->magic = 0xd34fa24e;
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->chat_id = fetch_ds_type_bare_int (field1);
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->id = fetch_ds_type_bare_int (field2);
  struct paramed_type *field3 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->from_id = fetch_ds_type_bare_int (field3);
  struct paramed_type *field4 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->date = fetch_ds_type_bare_int (field4);
  struct paramed_type *field5 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x383415fa, .id = "MessageAction", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->action = fetch_ds_type_message_action (field5);
  return result;
}
struct tl_ds_geo_point *fetch_ds_constructor_geo_point_empty (struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x315e0a53 && T->type->name != 0xcea1f5ac)) { return 0; }
  struct tl_ds_geo_point *  result = talloc0 (sizeof (*result));
  result->magic = 0x1117dd5f;
  return result;
}
struct tl_ds_geo_point *fetch_ds_constructor_geo_point (struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x315e0a53 && T->type->name != 0xcea1f5ac)) { return 0; }
  struct tl_ds_geo_point *  result = talloc0 (sizeof (*result));
  result->magic = 0x2049d70c;
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xddef3eab, .id = "Bare_Double", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->longitude = fetch_ds_type_bare_double (field1);
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xddef3eab, .id = "Bare_Double", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->latitude = fetch_ds_type_bare_double (field2);
  return result;
}
struct tl_ds_imported_contact *fetch_ds_constructor_imported_contact (struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0xd0028438 && T->type->name != 0x2ffd7bc7)) { return 0; }
  struct tl_ds_imported_contact *  result = talloc0 (sizeof (*result));
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->user_id = fetch_ds_type_bare_int (field1);
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xddf89345, .id = "Bare_Long", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->client_id = fetch_ds_type_bare_long (field2);
  return result;
}
struct tl_ds_input_app_event *fetch_ds_constructor_input_app_event (struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x770656a8 && T->type->name != 0x88f9a957)) { return 0; }
  struct tl_ds_input_app_event *  result = talloc0 (sizeof (*result));
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xddef3eab, .id = "Bare_Double", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->time = fetch_ds_type_bare_double (field1);
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->type = fetch_ds_type_bare_string (field2);
  struct paramed_type *field3 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xddf89345, .id = "Bare_Long", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->peer = fetch_ds_type_bare_long (field3);
  struct paramed_type *field4 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->data = fetch_ds_type_bare_string (field4);
  return result;
}
struct tl_ds_input_audio *fetch_ds_constructor_input_audio_empty (struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0xae8e9c7b && T->type->name != 0x51716384)) { return 0; }
  struct tl_ds_input_audio *  result = talloc0 (sizeof (*result));
  result->magic = 0xd95adc84;
  return result;
}
struct tl_ds_input_audio *fetch_ds_constructor_input_audio (struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0xae8e9c7b && T->type->name != 0x51716384)) { return 0; }
  struct tl_ds_input_audio *  result = talloc0 (sizeof (*result));
  result->magic = 0x77d440ff;
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xddf89345, .id = "Bare_Long", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->id = fetch_ds_type_bare_long (field1);
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xddf89345, .id = "Bare_Long", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->access_hash = fetch_ds_type_bare_long (field2);
  return result;
}
struct tl_ds_input_chat_photo *fetch_ds_constructor_input_chat_photo_empty (struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x3a60776d && T->type->name != 0xc59f8892)) { return 0; }
  struct tl_ds_input_chat_photo *  result = talloc0 (sizeof (*result));
  result->magic = 0x1ca48f57;
  return result;
}
struct tl_ds_input_chat_photo *fetch_ds_constructor_input_chat_uploaded_photo (struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x3a60776d && T->type->name != 0xc59f8892)) { return 0; }
  struct tl_ds_input_chat_photo *  result = talloc0 (sizeof (*result));
  result->magic = 0x94254732;
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x0f60f9ca, .id = "InputFile", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->file = fetch_ds_type_input_file (field1);
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x7477e321, .id = "InputPhotoCrop", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->crop = fetch_ds_type_input_photo_crop (field2);
  return result;
}
struct tl_ds_input_chat_photo *fetch_ds_constructor_input_chat_photo (struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x3a60776d && T->type->name != 0xc59f8892)) { return 0; }
  struct tl_ds_input_chat_photo *  result = talloc0 (sizeof (*result));
  result->magic = 0xb2e1bf08;
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xe74279c9, .id = "InputPhoto", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->id = fetch_ds_type_input_photo (field1);
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x7477e321, .id = "InputPhotoCrop", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->crop = fetch_ds_type_input_photo_crop (field2);
  return result;
}
struct tl_ds_input_contact *fetch_ds_constructor_input_phone_contact (struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0xf392b7f4 && T->type->name != 0x0c6d480b)) { return 0; }
  struct tl_ds_input_contact *  result = talloc0 (sizeof (*result));
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xddf89345, .id = "Bare_Long", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->client_id = fetch_ds_type_bare_long (field1);
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->phone = fetch_ds_type_bare_string (field2);
  struct paramed_type *field3 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->first_name = fetch_ds_type_bare_string (field3);
  struct paramed_type *field4 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->last_name = fetch_ds_type_bare_string (field4);
  return result;
}
struct tl_ds_input_document *fetch_ds_constructor_input_document_empty (struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x6a8963fc && T->type->name != 0x95769c03)) { return 0; }
  struct tl_ds_input_document *  result = talloc0 (sizeof (*result));
  result->magic = 0x72f0eaae;
  return result;
}
struct tl_ds_input_document *fetch_ds_constructor_input_document (struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x6a8963fc && T->type->name != 0x95769c03)) { return 0; }
  struct tl_ds_input_document *  result = talloc0 (sizeof (*result));
  result->magic = 0x18798952;
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xddf89345, .id = "Bare_Long", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->id = fetch_ds_type_bare_long (field1);
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xddf89345, .id = "Bare_Long", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->access_hash = fetch_ds_type_bare_long (field2);
  return result;
}
struct tl_ds_input_encrypted_chat *fetch_ds_constructor_input_encrypted_chat (struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0xf141b5e1 && T->type->name != 0x0ebe4a1e)) { return 0; }
  struct tl_ds_input_encrypted_chat *  result = talloc0 (sizeof (*result));
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->chat_id = fetch_ds_type_bare_int (field1);
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xddf89345, .id = "Bare_Long", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->access_hash = fetch_ds_type_bare_long (field2);
  return result;
}
struct tl_ds_input_encrypted_file *fetch_ds_constructor_input_encrypted_file_empty (struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x0b5c064f && T->type->name != 0xf4a3f9b0)) { return 0; }
  struct tl_ds_input_encrypted_file *  result = talloc0 (sizeof (*result));
  result->magic = 0x1837c364;
  return result;
}
struct tl_ds_input_encrypted_file *fetch_ds_constructor_input_encrypted_file_uploaded (struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x0b5c064f && T->type->name != 0xf4a3f9b0)) { return 0; }
  struct tl_ds_input_encrypted_file *  result = talloc0 (sizeof (*result));
  result->magic = 0x64bd0306;
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xddf89345, .id = "Bare_Long", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->id = fetch_ds_type_bare_long (field1);
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->parts = fetch_ds_type_bare_int (field2);
  struct paramed_type *field3 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->md5_checksum = fetch_ds_type_bare_string (field3);
  struct paramed_type *field4 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->key_fingerprint = fetch_ds_type_bare_int (field4);
  return result;
}
struct tl_ds_input_encrypted_file *fetch_ds_constructor_input_encrypted_file (struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x0b5c064f && T->type->name != 0xf4a3f9b0)) { return 0; }
  struct tl_ds_input_encrypted_file *  result = talloc0 (sizeof (*result));
  result->magic = 0x5a17b5e5;
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xddf89345, .id = "Bare_Long", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->id = fetch_ds_type_bare_long (field1);
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xddf89345, .id = "Bare_Long", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->access_hash = fetch_ds_type_bare_long (field2);
  return result;
}
struct tl_ds_input_encrypted_file *fetch_ds_constructor_input_encrypted_file_big_uploaded (struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x0b5c064f && T->type->name != 0xf4a3f9b0)) { return 0; }
  struct tl_ds_input_encrypted_file *  result = talloc0 (sizeof (*result));
  result->magic = 0x2dc173c8;
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xddf89345, .id = "Bare_Long", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->id = fetch_ds_type_bare_long (field1);
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->parts = fetch_ds_type_bare_int (field2);
  struct paramed_type *field3 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->key_fingerprint = fetch_ds_type_bare_int (field3);
  return result;
}
struct tl_ds_input_file *fetch_ds_constructor_input_file (struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x0f60f9ca && T->type->name != 0xf09f0635)) { return 0; }
  struct tl_ds_input_file *  result = talloc0 (sizeof (*result));
  result->magic = 0xf52ff27f;
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xddf89345, .id = "Bare_Long", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->id = fetch_ds_type_bare_long (field1);
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->parts = fetch_ds_type_bare_int (field2);
  struct paramed_type *field3 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->name = fetch_ds_type_bare_string (field3);
  struct paramed_type *field4 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->md5_checksum = fetch_ds_type_bare_string (field4);
  return result;
}
struct tl_ds_input_file *fetch_ds_constructor_input_file_big (struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x0f60f9ca && T->type->name != 0xf09f0635)) { return 0; }
  struct tl_ds_input_file *  result = talloc0 (sizeof (*result));
  result->magic = 0xfa4f0bb5;
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xddf89345, .id = "Bare_Long", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->id = fetch_ds_type_bare_long (field1);
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->parts = fetch_ds_type_bare_int (field2);
  struct paramed_type *field3 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->name = fetch_ds_type_bare_string (field3);
  return result;
}
struct tl_ds_input_file_location *fetch_ds_constructor_input_file_location (struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0xe6daa38b && T->type->name != 0x19255c74)) { return 0; }
  struct tl_ds_input_file_location *  result = talloc0 (sizeof (*result));
  result->magic = 0x14637196;
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xddf89345, .id = "Bare_Long", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->volume_id = fetch_ds_type_bare_long (field1);
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->local_id = fetch_ds_type_bare_int (field2);
  struct paramed_type *field3 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xddf89345, .id = "Bare_Long", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->secret = fetch_ds_type_bare_long (field3);
  return result;
}
struct tl_ds_input_file_location *fetch_ds_constructor_input_video_file_location (struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0xe6daa38b && T->type->name != 0x19255c74)) { return 0; }
  struct tl_ds_input_file_location *  result = talloc0 (sizeof (*result));
  result->magic = 0x3d0364ec;
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xddf89345, .id = "Bare_Long", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->id = fetch_ds_type_bare_long (field1);
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xddf89345, .id = "Bare_Long", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->access_hash = fetch_ds_type_bare_long (field2);
  return result;
}
struct tl_ds_input_file_location *fetch_ds_constructor_input_encrypted_file_location (struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0xe6daa38b && T->type->name != 0x19255c74)) { return 0; }
  struct tl_ds_input_file_location *  result = talloc0 (sizeof (*result));
  result->magic = 0xf5235d55;
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xddf89345, .id = "Bare_Long", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->id = fetch_ds_type_bare_long (field1);
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xddf89345, .id = "Bare_Long", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->access_hash = fetch_ds_type_bare_long (field2);
  return result;
}
struct tl_ds_input_file_location *fetch_ds_constructor_input_audio_file_location (struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0xe6daa38b && T->type->name != 0x19255c74)) { return 0; }
  struct tl_ds_input_file_location *  result = talloc0 (sizeof (*result));
  result->magic = 0x74dc404d;
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xddf89345, .id = "Bare_Long", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->id = fetch_ds_type_bare_long (field1);
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xddf89345, .id = "Bare_Long", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->access_hash = fetch_ds_type_bare_long (field2);
  return result;
}
struct tl_ds_input_file_location *fetch_ds_constructor_input_document_file_location (struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0xe6daa38b && T->type->name != 0x19255c74)) { return 0; }
  struct tl_ds_input_file_location *  result = talloc0 (sizeof (*result));
  result->magic = 0x4e45abe9;
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xddf89345, .id = "Bare_Long", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->id = fetch_ds_type_bare_long (field1);
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xddf89345, .id = "Bare_Long", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->access_hash = fetch_ds_type_bare_long (field2);
  return result;
}
struct tl_ds_input_geo_chat *fetch_ds_constructor_input_geo_chat (struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x74d456fa && T->type->name != 0x8b2ba905)) { return 0; }
  struct tl_ds_input_geo_chat *  result = talloc0 (sizeof (*result));
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->chat_id = fetch_ds_type_bare_int (field1);
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xddf89345, .id = "Bare_Long", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->access_hash = fetch_ds_type_bare_long (field2);
  return result;
}
struct tl_ds_input_geo_point *fetch_ds_constructor_input_geo_point_empty (struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x17768f1f && T->type->name != 0xe88970e0)) { return 0; }
  struct tl_ds_input_geo_point *  result = talloc0 (sizeof (*result));
  result->magic = 0xe4c123d6;
  return result;
}
struct tl_ds_input_geo_point *fetch_ds_constructor_input_geo_point (struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x17768f1f && T->type->name != 0xe88970e0)) { return 0; }
  struct tl_ds_input_geo_point *  result = talloc0 (sizeof (*result));
  result->magic = 0xf3b7acc9;
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xddef3eab, .id = "Bare_Double", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->latitude = fetch_ds_type_bare_double (field1);
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xddef3eab, .id = "Bare_Double", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->longitude = fetch_ds_type_bare_double (field2);
  return result;
}
struct tl_ds_input_media *fetch_ds_constructor_input_media_empty (struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0xb32c91dd && T->type->name != 0x4cd36e22)) { return 0; }
  struct tl_ds_input_media *  result = talloc0 (sizeof (*result));
  result->magic = 0x9664f57f;
  return result;
}
struct tl_ds_input_media *fetch_ds_constructor_input_media_uploaded_photo (struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0xb32c91dd && T->type->name != 0x4cd36e22)) { return 0; }
  struct tl_ds_input_media *  result = talloc0 (sizeof (*result));
  result->magic = 0xf7aff1c0;
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x0f60f9ca, .id = "InputFile", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->file = fetch_ds_type_input_file (field1);
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->caption = fetch_ds_type_bare_string (field2);
  return result;
}
struct tl_ds_input_media *fetch_ds_constructor_input_media_photo (struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0xb32c91dd && T->type->name != 0x4cd36e22)) { return 0; }
  struct tl_ds_input_media *  result = talloc0 (sizeof (*result));
  result->magic = 0xe9bfb4f3;
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xe74279c9, .id = "InputPhoto", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->id = fetch_ds_type_input_photo (field1);
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->caption = fetch_ds_type_bare_string (field2);
  return result;
}
struct tl_ds_input_media *fetch_ds_constructor_input_media_geo_point (struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0xb32c91dd && T->type->name != 0x4cd36e22)) { return 0; }
  struct tl_ds_input_media *  result = talloc0 (sizeof (*result));
  result->magic = 0xf9c44144;
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x17768f1f, .id = "InputGeoPoint", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->geo_point = fetch_ds_type_input_geo_point (field1);
  return result;
}
struct tl_ds_input_media *fetch_ds_constructor_input_media_contact (struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0xb32c91dd && T->type->name != 0x4cd36e22)) { return 0; }
  struct tl_ds_input_media *  result = talloc0 (sizeof (*result));
  result->magic = 0xa6e45987;
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->phone_number = fetch_ds_type_bare_string (field1);
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->first_name = fetch_ds_type_bare_string (field2);
  struct paramed_type *field3 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->last_name = fetch_ds_type_bare_string (field3);
  return result;
}
struct tl_ds_input_media *fetch_ds_constructor_input_media_uploaded_video (struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0xb32c91dd && T->type->name != 0x4cd36e22)) { return 0; }
  struct tl_ds_input_media *  result = talloc0 (sizeof (*result));
  result->magic = 0xe13fd4bc;
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x0f60f9ca, .id = "InputFile", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->file = fetch_ds_type_input_file (field1);
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->duration = fetch_ds_type_bare_int (field2);
  struct paramed_type *field3 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->w = fetch_ds_type_bare_int (field3);
  struct paramed_type *field4 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->h = fetch_ds_type_bare_int (field4);
  struct paramed_type *field5 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->caption = fetch_ds_type_bare_string (field5);
  return result;
}
struct tl_ds_input_media *fetch_ds_constructor_input_media_uploaded_thumb_video (struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0xb32c91dd && T->type->name != 0x4cd36e22)) { return 0; }
  struct tl_ds_input_media *  result = talloc0 (sizeof (*result));
  result->magic = 0x96fb97dc;
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x0f60f9ca, .id = "InputFile", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->file = fetch_ds_type_input_file (field1);
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x0f60f9ca, .id = "InputFile", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->thumb = fetch_ds_type_input_file (field2);
  struct paramed_type *field3 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->duration = fetch_ds_type_bare_int (field3);
  struct paramed_type *field4 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->w = fetch_ds_type_bare_int (field4);
  struct paramed_type *field5 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->h = fetch_ds_type_bare_int (field5);
  struct paramed_type *field6 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->caption = fetch_ds_type_bare_string (field6);
  return result;
}
struct tl_ds_input_media *fetch_ds_constructor_input_media_video (struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0xb32c91dd && T->type->name != 0x4cd36e22)) { return 0; }
  struct tl_ds_input_media *  result = talloc0 (sizeof (*result));
  result->magic = 0x936a4ebd;
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xbb5f7a27, .id = "InputVideo", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->video_id = fetch_ds_type_input_video (field1);
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->caption = fetch_ds_type_bare_string (field2);
  return result;
}
struct tl_ds_input_media *fetch_ds_constructor_input_media_uploaded_audio (struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0xb32c91dd && T->type->name != 0x4cd36e22)) { return 0; }
  struct tl_ds_input_media *  result = talloc0 (sizeof (*result));
  result->magic = 0x4e498cab;
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x0f60f9ca, .id = "InputFile", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->file = fetch_ds_type_input_file (field1);
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->duration = fetch_ds_type_bare_int (field2);
  struct paramed_type *field3 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->mime_type = fetch_ds_type_bare_string (field3);
  return result;
}
struct tl_ds_input_media *fetch_ds_constructor_input_media_audio (struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0xb32c91dd && T->type->name != 0x4cd36e22)) { return 0; }
  struct tl_ds_input_media *  result = talloc0 (sizeof (*result));
  result->magic = 0x89938781;
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xae8e9c7b, .id = "InputAudio", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->audio_id = fetch_ds_type_input_audio (field1);
  return result;
}
struct tl_ds_input_media *fetch_ds_constructor_input_media_uploaded_document (struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0xb32c91dd && T->type->name != 0x4cd36e22)) { return 0; }
  struct tl_ds_input_media *  result = talloc0 (sizeof (*result));
  result->magic = 0xffe76b78;
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x0f60f9ca, .id = "InputFile", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->file = fetch_ds_type_input_file (field1);
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->mime_type = fetch_ds_type_bare_string (field2);
  struct paramed_type *field3 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x1cb5c415, .id = "Vector", .params_num = 1, .params_types = 0},
    .params = (struct paramed_type *[]){
      &(struct paramed_type){
        .type = &(struct tl_type_descr) {.name = 0x97c637a3, .id = "DocumentAttribute", .params_num = 0, .params_types = 0},
        .params = 0,
      },
    }
  };
  result->attributes = (void *)fetch_ds_type_vector (field3);
  return result;
}
struct tl_ds_input_media *fetch_ds_constructor_input_media_uploaded_thumb_document (struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0xb32c91dd && T->type->name != 0x4cd36e22)) { return 0; }
  struct tl_ds_input_media *  result = talloc0 (sizeof (*result));
  result->magic = 0x41481486;
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x0f60f9ca, .id = "InputFile", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->file = fetch_ds_type_input_file (field1);
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x0f60f9ca, .id = "InputFile", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->thumb = fetch_ds_type_input_file (field2);
  struct paramed_type *field3 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->mime_type = fetch_ds_type_bare_string (field3);
  struct paramed_type *field4 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x1cb5c415, .id = "Vector", .params_num = 1, .params_types = 0},
    .params = (struct paramed_type *[]){
      &(struct paramed_type){
        .type = &(struct tl_type_descr) {.name = 0x97c637a3, .id = "DocumentAttribute", .params_num = 0, .params_types = 0},
        .params = 0,
      },
    }
  };
  result->attributes = (void *)fetch_ds_type_vector (field4);
  return result;
}
struct tl_ds_input_media *fetch_ds_constructor_input_media_document (struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0xb32c91dd && T->type->name != 0x4cd36e22)) { return 0; }
  struct tl_ds_input_media *  result = talloc0 (sizeof (*result));
  result->magic = 0xd184e841;
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x6a8963fc, .id = "InputDocument", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->document_id = fetch_ds_type_input_document (field1);
  return result;
}
struct tl_ds_input_media *fetch_ds_constructor_input_media_venue (struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0xb32c91dd && T->type->name != 0x4cd36e22)) { return 0; }
  struct tl_ds_input_media *  result = talloc0 (sizeof (*result));
  result->magic = 0x2827a81a;
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x17768f1f, .id = "InputGeoPoint", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->geo_point = fetch_ds_type_input_geo_point (field1);
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->title = fetch_ds_type_bare_string (field2);
  struct paramed_type *field3 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->address = fetch_ds_type_bare_string (field3);
  struct paramed_type *field4 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->provider = fetch_ds_type_bare_string (field4);
  struct paramed_type *field5 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->venue_id = fetch_ds_type_bare_string (field5);
  return result;
}
struct tl_ds_input_notify_peer *fetch_ds_constructor_input_notify_peer (struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x02b6911b && T->type->name != 0xfd496ee4)) { return 0; }
  struct tl_ds_input_notify_peer *  result = talloc0 (sizeof (*result));
  result->magic = 0xb8bc5b0c;
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x9e67268d, .id = "InputPeer", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->peer = fetch_ds_type_input_peer (field1);
  return result;
}
struct tl_ds_input_notify_peer *fetch_ds_constructor_input_notify_users (struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x02b6911b && T->type->name != 0xfd496ee4)) { return 0; }
  struct tl_ds_input_notify_peer *  result = talloc0 (sizeof (*result));
  result->magic = 0x193b4417;
  return result;
}
struct tl_ds_input_notify_peer *fetch_ds_constructor_input_notify_chats (struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x02b6911b && T->type->name != 0xfd496ee4)) { return 0; }
  struct tl_ds_input_notify_peer *  result = talloc0 (sizeof (*result));
  result->magic = 0x4a95e84e;
  return result;
}
struct tl_ds_input_notify_peer *fetch_ds_constructor_input_notify_all (struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x02b6911b && T->type->name != 0xfd496ee4)) { return 0; }
  struct tl_ds_input_notify_peer *  result = talloc0 (sizeof (*result));
  result->magic = 0xa429b886;
  return result;
}
struct tl_ds_input_notify_peer *fetch_ds_constructor_input_notify_geo_chat_peer (struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x02b6911b && T->type->name != 0xfd496ee4)) { return 0; }
  struct tl_ds_input_notify_peer *  result = talloc0 (sizeof (*result));
  result->magic = 0x4d8ddec8;
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x74d456fa, .id = "InputGeoChat", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->geo_peer = fetch_ds_type_input_geo_chat (field1);
  return result;
}
struct tl_ds_input_peer *fetch_ds_constructor_input_peer_empty (struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x9e67268d && T->type->name != 0x6198d972)) { return 0; }
  struct tl_ds_input_peer *  result = talloc0 (sizeof (*result));
  result->magic = 0x7f3b18ea;
  return result;
}
struct tl_ds_input_peer *fetch_ds_constructor_input_peer_self (struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x9e67268d && T->type->name != 0x6198d972)) { return 0; }
  struct tl_ds_input_peer *  result = talloc0 (sizeof (*result));
  result->magic = 0x7da07ec9;
  return result;
}
struct tl_ds_input_peer *fetch_ds_constructor_input_peer_contact (struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x9e67268d && T->type->name != 0x6198d972)) { return 0; }
  struct tl_ds_input_peer *  result = talloc0 (sizeof (*result));
  result->magic = 0x1023dbe8;
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->user_id = fetch_ds_type_bare_int (field1);
  return result;
}
struct tl_ds_input_peer *fetch_ds_constructor_input_peer_foreign (struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x9e67268d && T->type->name != 0x6198d972)) { return 0; }
  struct tl_ds_input_peer *  result = talloc0 (sizeof (*result));
  result->magic = 0x9b447325;
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->user_id = fetch_ds_type_bare_int (field1);
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xddf89345, .id = "Bare_Long", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->access_hash = fetch_ds_type_bare_long (field2);
  return result;
}
struct tl_ds_input_peer *fetch_ds_constructor_input_peer_chat (struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x9e67268d && T->type->name != 0x6198d972)) { return 0; }
  struct tl_ds_input_peer *  result = talloc0 (sizeof (*result));
  result->magic = 0x179be863;
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->chat_id = fetch_ds_type_bare_int (field1);
  return result;
}
struct tl_ds_input_peer_notify_events *fetch_ds_constructor_input_peer_notify_events_empty (struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x185a48ac && T->type->name != 0xe7a5b753)) { return 0; }
  struct tl_ds_input_peer_notify_events *  result = talloc0 (sizeof (*result));
  result->magic = 0xf03064d8;
  return result;
}
struct tl_ds_input_peer_notify_events *fetch_ds_constructor_input_peer_notify_events_all (struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x185a48ac && T->type->name != 0xe7a5b753)) { return 0; }
  struct tl_ds_input_peer_notify_events *  result = talloc0 (sizeof (*result));
  result->magic = 0xe86a2c74;
  return result;
}
struct tl_ds_input_peer_notify_settings *fetch_ds_constructor_input_peer_notify_settings (struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x46a2ce98 && T->type->name != 0xb95d3167)) { return 0; }
  struct tl_ds_input_peer_notify_settings *  result = talloc0 (sizeof (*result));
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->mute_until = fetch_ds_type_bare_int (field1);
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->sound = fetch_ds_type_bare_string (field2);
  struct paramed_type *field3 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x250be282, .id = "Bool", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->show_previews = fetch_ds_type_bool (field3);
  struct paramed_type *field4 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->events_mask = fetch_ds_type_bare_int (field4);
  return result;
}
struct tl_ds_input_photo *fetch_ds_constructor_input_photo_empty (struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0xe74279c9 && T->type->name != 0x18bd8636)) { return 0; }
  struct tl_ds_input_photo *  result = talloc0 (sizeof (*result));
  result->magic = 0x1cd7bf0d;
  return result;
}
struct tl_ds_input_photo *fetch_ds_constructor_input_photo (struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0xe74279c9 && T->type->name != 0x18bd8636)) { return 0; }
  struct tl_ds_input_photo *  result = talloc0 (sizeof (*result));
  result->magic = 0xfb95c6c4;
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xddf89345, .id = "Bare_Long", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->id = fetch_ds_type_bare_long (field1);
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xddf89345, .id = "Bare_Long", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->access_hash = fetch_ds_type_bare_long (field2);
  return result;
}
struct tl_ds_input_photo_crop *fetch_ds_constructor_input_photo_crop_auto (struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x7477e321 && T->type->name != 0x8b881cde)) { return 0; }
  struct tl_ds_input_photo_crop *  result = talloc0 (sizeof (*result));
  result->magic = 0xade6b004;
  return result;
}
struct tl_ds_input_photo_crop *fetch_ds_constructor_input_photo_crop (struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x7477e321 && T->type->name != 0x8b881cde)) { return 0; }
  struct tl_ds_input_photo_crop *  result = talloc0 (sizeof (*result));
  result->magic = 0xd9915325;
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xddef3eab, .id = "Bare_Double", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->crop_left = fetch_ds_type_bare_double (field1);
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xddef3eab, .id = "Bare_Double", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->crop_top = fetch_ds_type_bare_double (field2);
  struct paramed_type *field3 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xddef3eab, .id = "Bare_Double", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->crop_width = fetch_ds_type_bare_double (field3);
  return result;
}
struct tl_ds_input_privacy_key *fetch_ds_constructor_input_privacy_key_status_timestamp (struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x4f96cb18 && T->type->name != 0xb06934e7)) { return 0; }
  struct tl_ds_input_privacy_key *  result = talloc0 (sizeof (*result));
  return result;
}
struct tl_ds_input_privacy_rule *fetch_ds_constructor_input_privacy_value_allow_contacts (struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x4b815163 && T->type->name != 0xb47eae9c)) { return 0; }
  struct tl_ds_input_privacy_rule *  result = talloc0 (sizeof (*result));
  result->magic = 0x0d09e07b;
  return result;
}
struct tl_ds_input_privacy_rule *fetch_ds_constructor_input_privacy_value_allow_all (struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x4b815163 && T->type->name != 0xb47eae9c)) { return 0; }
  struct tl_ds_input_privacy_rule *  result = talloc0 (sizeof (*result));
  result->magic = 0x184b35ce;
  return result;
}
struct tl_ds_input_privacy_rule *fetch_ds_constructor_input_privacy_value_allow_users (struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x4b815163 && T->type->name != 0xb47eae9c)) { return 0; }
  struct tl_ds_input_privacy_rule *  result = talloc0 (sizeof (*result));
  result->magic = 0x131cc67f;
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x1cb5c415, .id = "Vector", .params_num = 1, .params_types = 0},
    .params = (struct paramed_type *[]){
      &(struct paramed_type){
        .type = &(struct tl_type_descr) {.name = 0xadfe0c6a, .id = "InputUser", .params_num = 0, .params_types = 0},
        .params = 0,
      },
    }
  };
  result->users = (void *)fetch_ds_type_vector (field1);
  return result;
}
struct tl_ds_input_privacy_rule *fetch_ds_constructor_input_privacy_value_disallow_contacts (struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x4b815163 && T->type->name != 0xb47eae9c)) { return 0; }
  struct tl_ds_input_privacy_rule *  result = talloc0 (sizeof (*result));
  result->magic = 0x0ba52007;
  return result;
}
struct tl_ds_input_privacy_rule *fetch_ds_constructor_input_privacy_value_disallow_all (struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x4b815163 && T->type->name != 0xb47eae9c)) { return 0; }
  struct tl_ds_input_privacy_rule *  result = talloc0 (sizeof (*result));
  result->magic = 0xd66b66c9;
  return result;
}
struct tl_ds_input_privacy_rule *fetch_ds_constructor_input_privacy_value_disallow_users (struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x4b815163 && T->type->name != 0xb47eae9c)) { return 0; }
  struct tl_ds_input_privacy_rule *  result = talloc0 (sizeof (*result));
  result->magic = 0x90110467;
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x1cb5c415, .id = "Vector", .params_num = 1, .params_types = 0},
    .params = (struct paramed_type *[]){
      &(struct paramed_type){
        .type = &(struct tl_type_descr) {.name = 0xadfe0c6a, .id = "InputUser", .params_num = 0, .params_types = 0},
        .params = 0,
      },
    }
  };
  result->users = (void *)fetch_ds_type_vector (field1);
  return result;
}
struct tl_ds_input_sticker_set *fetch_ds_constructor_input_sticker_set_empty (struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0xe44d415c && T->type->name != 0x1bb2bea3)) { return 0; }
  struct tl_ds_input_sticker_set *  result = talloc0 (sizeof (*result));
  result->magic = 0xffb62b95;
  return result;
}
struct tl_ds_input_sticker_set *fetch_ds_constructor_input_sticker_set_i_d (struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0xe44d415c && T->type->name != 0x1bb2bea3)) { return 0; }
  struct tl_ds_input_sticker_set *  result = talloc0 (sizeof (*result));
  result->magic = 0x9de7a269;
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xddf89345, .id = "Bare_Long", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->id = fetch_ds_type_bare_long (field1);
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xddf89345, .id = "Bare_Long", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->access_hash = fetch_ds_type_bare_long (field2);
  return result;
}
struct tl_ds_input_sticker_set *fetch_ds_constructor_input_sticker_set_short_name (struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0xe44d415c && T->type->name != 0x1bb2bea3)) { return 0; }
  struct tl_ds_input_sticker_set *  result = talloc0 (sizeof (*result));
  result->magic = 0x861cc8a0;
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->short_name = fetch_ds_type_bare_string (field1);
  return result;
}
struct tl_ds_input_user *fetch_ds_constructor_input_user_empty (struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0xadfe0c6a && T->type->name != 0x5201f395)) { return 0; }
  struct tl_ds_input_user *  result = talloc0 (sizeof (*result));
  result->magic = 0xb98886cf;
  return result;
}
struct tl_ds_input_user *fetch_ds_constructor_input_user_self (struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0xadfe0c6a && T->type->name != 0x5201f395)) { return 0; }
  struct tl_ds_input_user *  result = talloc0 (sizeof (*result));
  result->magic = 0xf7c1b13f;
  return result;
}
struct tl_ds_input_user *fetch_ds_constructor_input_user_contact (struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0xadfe0c6a && T->type->name != 0x5201f395)) { return 0; }
  struct tl_ds_input_user *  result = talloc0 (sizeof (*result));
  result->magic = 0x86e94f65;
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->user_id = fetch_ds_type_bare_int (field1);
  return result;
}
struct tl_ds_input_user *fetch_ds_constructor_input_user_foreign (struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0xadfe0c6a && T->type->name != 0x5201f395)) { return 0; }
  struct tl_ds_input_user *  result = talloc0 (sizeof (*result));
  result->magic = 0x655e74ff;
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->user_id = fetch_ds_type_bare_int (field1);
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xddf89345, .id = "Bare_Long", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->access_hash = fetch_ds_type_bare_long (field2);
  return result;
}
struct tl_ds_input_video *fetch_ds_constructor_input_video_empty (struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0xbb5f7a27 && T->type->name != 0x44a085d8)) { return 0; }
  struct tl_ds_input_video *  result = talloc0 (sizeof (*result));
  result->magic = 0x5508ec75;
  return result;
}
struct tl_ds_input_video *fetch_ds_constructor_input_video (struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0xbb5f7a27 && T->type->name != 0x44a085d8)) { return 0; }
  struct tl_ds_input_video *  result = talloc0 (sizeof (*result));
  result->magic = 0xee579652;
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xddf89345, .id = "Bare_Long", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->id = fetch_ds_type_bare_long (field1);
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xddf89345, .id = "Bare_Long", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->access_hash = fetch_ds_type_bare_long (field2);
  return result;
}
int *fetch_ds_constructor_int (struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0xa8509bda && T->type->name != 0x57af6425)) { return 0; }
  int *  result = talloc0 (sizeof (*result));
  assert (in_remaining () >= 4);
  *result = fetch_int ();
  return result;
}
struct tl_ds_int128 *fetch_ds_constructor_int128 (struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x7d36c439 && T->type->name != 0x82c93bc6)) { return 0; }
  struct tl_ds_int128 *  result = talloc0 (sizeof (*result));
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xddf89345, .id = "Bare_Long", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->f0 = fetch_ds_type_bare_long (field1);
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xddf89345, .id = "Bare_Long", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->f1 = fetch_ds_type_bare_long (field2);
  return result;
}
struct tl_ds_int256 *fetch_ds_constructor_int256 (struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0xf2c798b3 && T->type->name != 0x0d38674c)) { return 0; }
  struct tl_ds_int256 *  result = talloc0 (sizeof (*result));
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xddf89345, .id = "Bare_Long", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->f0 = fetch_ds_type_bare_long (field1);
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xddf89345, .id = "Bare_Long", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->f1 = fetch_ds_type_bare_long (field2);
  struct paramed_type *field3 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xddf89345, .id = "Bare_Long", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->f2 = fetch_ds_type_bare_long (field3);
  struct paramed_type *field4 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xddf89345, .id = "Bare_Long", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->f3 = fetch_ds_type_bare_long (field4);
  return result;
}
struct tl_ds_keyboard_button *fetch_ds_constructor_keyboard_button (struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0xa2fa4880 && T->type->name != 0x5d05b77f)) { return 0; }
  struct tl_ds_keyboard_button *  result = talloc0 (sizeof (*result));
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->text = fetch_ds_type_bare_string (field1);
  return result;
}
struct tl_ds_keyboard_button_row *fetch_ds_constructor_keyboard_button_row (struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x77608b83 && T->type->name != 0x889f747c)) { return 0; }
  struct tl_ds_keyboard_button_row *  result = talloc0 (sizeof (*result));
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x1cb5c415, .id = "Vector", .params_num = 1, .params_types = 0},
    .params = (struct paramed_type *[]){
      &(struct paramed_type){
        .type = &(struct tl_type_descr) {.name = 0xa2fa4880, .id = "KeyboardButton", .params_num = 0, .params_types = 0},
        .params = 0,
      },
    }
  };
  result->buttons = (void *)fetch_ds_type_vector (field1);
  return result;
}
long long *fetch_ds_constructor_long (struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x22076cba && T->type->name != 0xddf89345)) { return 0; }
  long long *  result = talloc0 (sizeof (*result));
  assert (in_remaining () >= 8);
  *result = fetch_long ();
  return result;
}
struct tl_ds_message *fetch_ds_constructor_message_empty (struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x5d652a7f && T->type->name != 0xa29ad580)) { return 0; }
  struct tl_ds_message *  result = talloc0 (sizeof (*result));
  result->magic = 0x83e5de54;
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->id = fetch_ds_type_bare_int (field1);
  return result;
}
struct tl_ds_message *fetch_ds_constructor_message (struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x5d652a7f && T->type->name != 0xa29ad580)) { return 0; }
  struct tl_ds_message *  result = talloc0 (sizeof (*result));
  result->magic = 0xc3060325;
  assert (in_remaining () >= 4);
  result->flags = talloc (4);  *result->flags = prefetch_int ();  struct paramed_type *var0 = INT2PTR (fetch_int ());
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->id = fetch_ds_type_bare_int (field2);
  struct paramed_type *field3 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->from_id = fetch_ds_type_bare_int (field3);
  struct paramed_type *field4 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x276159d6, .id = "Peer", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->to_id = fetch_ds_type_peer (field4);
  if (PTR2INT (var0) & (1 << 2)) {
    struct paramed_type *field5 = 
    &(struct paramed_type){
      .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
      .params = 0,
    };
    result->fwd_from_id = fetch_ds_type_bare_int (field5);
  }
  if (PTR2INT (var0) & (1 << 2)) {
    struct paramed_type *field6 = 
    &(struct paramed_type){
      .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
      .params = 0,
    };
    result->fwd_date = fetch_ds_type_bare_int (field6);
  }
  if (PTR2INT (var0) & (1 << 3)) {
    struct paramed_type *field7 = 
    &(struct paramed_type){
      .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
      .params = 0,
    };
    result->reply_to_msg_id = fetch_ds_type_bare_int (field7);
  }
  struct paramed_type *field8 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->date = fetch_ds_type_bare_int (field8);
  struct paramed_type *field9 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->message = fetch_ds_type_bare_string (field9);
  struct paramed_type *field10 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x95f2471a, .id = "MessageMedia", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->media = fetch_ds_type_message_media (field10);
  if (PTR2INT (var0) & (1 << 6)) {
    struct paramed_type *field11 = 
    &(struct paramed_type){
      .type = &(struct tl_type_descr) {.name = 0x612ca4a9, .id = "ReplyMarkup", .params_num = 0, .params_types = 0},
      .params = 0,
    };
    result->reply_markup = fetch_ds_type_reply_markup (field11);
  }
  return result;
}
struct tl_ds_message *fetch_ds_constructor_message_service (struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x5d652a7f && T->type->name != 0xa29ad580)) { return 0; }
  struct tl_ds_message *  result = talloc0 (sizeof (*result));
  result->magic = 0x1d86f70e;
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->flags = fetch_ds_type_bare_int (field1);
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->id = fetch_ds_type_bare_int (field2);
  struct paramed_type *field3 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->from_id = fetch_ds_type_bare_int (field3);
  struct paramed_type *field4 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x276159d6, .id = "Peer", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->to_id = fetch_ds_type_peer (field4);
  struct paramed_type *field5 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->date = fetch_ds_type_bare_int (field5);
  struct paramed_type *field6 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x383415fa, .id = "MessageAction", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->action = fetch_ds_type_message_action (field6);
  return result;
}
struct tl_ds_message_action *fetch_ds_constructor_message_action_empty (struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x383415fa && T->type->name != 0xc7cbea05)) { return 0; }
  struct tl_ds_message_action *  result = talloc0 (sizeof (*result));
  result->magic = 0xb6aef7b0;
  return result;
}
struct tl_ds_message_action *fetch_ds_constructor_message_action_chat_create (struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x383415fa && T->type->name != 0xc7cbea05)) { return 0; }
  struct tl_ds_message_action *  result = talloc0 (sizeof (*result));
  result->magic = 0xa6638b9a;
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->title = fetch_ds_type_bare_string (field1);
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x1cb5c415, .id = "Vector", .params_num = 1, .params_types = 0},
    .params = (struct paramed_type *[]){
      &(struct paramed_type){
        .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
        .params = 0,
      },
    }
  };
  result->users = (void *)fetch_ds_type_vector (field2);
  return result;
}
struct tl_ds_message_action *fetch_ds_constructor_message_action_chat_edit_title (struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x383415fa && T->type->name != 0xc7cbea05)) { return 0; }
  struct tl_ds_message_action *  result = talloc0 (sizeof (*result));
  result->magic = 0xb5a1ce5a;
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->title = fetch_ds_type_bare_string (field1);
  return result;
}
struct tl_ds_message_action *fetch_ds_constructor_message_action_chat_edit_photo (struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x383415fa && T->type->name != 0xc7cbea05)) { return 0; }
  struct tl_ds_message_action *  result = talloc0 (sizeof (*result));
  result->magic = 0x7fcb13a8;
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xc207550a, .id = "Photo", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->photo = fetch_ds_type_photo (field1);
  return result;
}
struct tl_ds_message_action *fetch_ds_constructor_message_action_chat_delete_photo (struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x383415fa && T->type->name != 0xc7cbea05)) { return 0; }
  struct tl_ds_message_action *  result = talloc0 (sizeof (*result));
  result->magic = 0x95e3fbef;
  return result;
}
struct tl_ds_message_action *fetch_ds_constructor_message_action_chat_add_user (struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x383415fa && T->type->name != 0xc7cbea05)) { return 0; }
  struct tl_ds_message_action *  result = talloc0 (sizeof (*result));
  result->magic = 0x5e3cfc4b;
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->user_id = fetch_ds_type_bare_int (field1);
  return result;
}
struct tl_ds_message_action *fetch_ds_constructor_message_action_chat_delete_user (struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x383415fa && T->type->name != 0xc7cbea05)) { return 0; }
  struct tl_ds_message_action *  result = talloc0 (sizeof (*result));
  result->magic = 0xb2ae9b0c;
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->user_id = fetch_ds_type_bare_int (field1);
  return result;
}
struct tl_ds_message_action *fetch_ds_constructor_message_action_geo_chat_create (struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x383415fa && T->type->name != 0xc7cbea05)) { return 0; }
  struct tl_ds_message_action *  result = talloc0 (sizeof (*result));
  result->magic = 0x6f038ebc;
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->title = fetch_ds_type_bare_string (field1);
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->address = fetch_ds_type_bare_string (field2);
  return result;
}
struct tl_ds_message_action *fetch_ds_constructor_message_action_geo_chat_checkin (struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x383415fa && T->type->name != 0xc7cbea05)) { return 0; }
  struct tl_ds_message_action *  result = talloc0 (sizeof (*result));
  result->magic = 0x0c7d53de;
  return result;
}
struct tl_ds_message_action *fetch_ds_constructor_message_action_chat_joined_by_link (struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x383415fa && T->type->name != 0xc7cbea05)) { return 0; }
  struct tl_ds_message_action *  result = talloc0 (sizeof (*result));
  result->magic = 0xf89cf5e8;
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->inviter_id = fetch_ds_type_bare_int (field1);
  return result;
}
struct tl_ds_message_media *fetch_ds_constructor_message_media_empty (struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x95f2471a && T->type->name != 0x6a0db8e5)) { return 0; }
  struct tl_ds_message_media *  result = talloc0 (sizeof (*result));
  result->magic = 0x3ded6320;
  return result;
}
struct tl_ds_message_media *fetch_ds_constructor_message_media_photo (struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x95f2471a && T->type->name != 0x6a0db8e5)) { return 0; }
  struct tl_ds_message_media *  result = talloc0 (sizeof (*result));
  result->magic = 0x3d8ce53d;
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xc207550a, .id = "Photo", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->photo = fetch_ds_type_photo (field1);
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->caption = fetch_ds_type_bare_string (field2);
  return result;
}
struct tl_ds_message_media *fetch_ds_constructor_message_media_video (struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x95f2471a && T->type->name != 0x6a0db8e5)) { return 0; }
  struct tl_ds_message_media *  result = talloc0 (sizeof (*result));
  result->magic = 0x5bcf1675;
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x1716b174, .id = "Video", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->video = fetch_ds_type_video (field1);
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->caption = fetch_ds_type_bare_string (field2);
  return result;
}
struct tl_ds_message_media *fetch_ds_constructor_message_media_geo (struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x95f2471a && T->type->name != 0x6a0db8e5)) { return 0; }
  struct tl_ds_message_media *  result = talloc0 (sizeof (*result));
  result->magic = 0x56e0d474;
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x315e0a53, .id = "GeoPoint", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->geo = fetch_ds_type_geo_point (field1);
  return result;
}
struct tl_ds_message_media *fetch_ds_constructor_message_media_contact (struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x95f2471a && T->type->name != 0x6a0db8e5)) { return 0; }
  struct tl_ds_message_media *  result = talloc0 (sizeof (*result));
  result->magic = 0x5e7d2f39;
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->phone_number = fetch_ds_type_bare_string (field1);
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->first_name = fetch_ds_type_bare_string (field2);
  struct paramed_type *field3 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->last_name = fetch_ds_type_bare_string (field3);
  struct paramed_type *field4 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->user_id = fetch_ds_type_bare_int (field4);
  return result;
}
struct tl_ds_message_media *fetch_ds_constructor_message_media_unsupported (struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x95f2471a && T->type->name != 0x6a0db8e5)) { return 0; }
  struct tl_ds_message_media *  result = talloc0 (sizeof (*result));
  result->magic = 0x9f84f49e;
  return result;
}
struct tl_ds_message_media *fetch_ds_constructor_message_media_document (struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x95f2471a && T->type->name != 0x6a0db8e5)) { return 0; }
  struct tl_ds_message_media *  result = talloc0 (sizeof (*result));
  result->magic = 0x2fda2204;
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x51a73418, .id = "Document", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->document = fetch_ds_type_document (field1);
  return result;
}
struct tl_ds_message_media *fetch_ds_constructor_message_media_audio (struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x95f2471a && T->type->name != 0x6a0db8e5)) { return 0; }
  struct tl_ds_message_media *  result = talloc0 (sizeof (*result));
  result->magic = 0xc6b68300;
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x9fc5ec4e, .id = "Audio", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->audio = fetch_ds_type_audio (field1);
  return result;
}
struct tl_ds_message_media *fetch_ds_constructor_message_media_web_page (struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x95f2471a && T->type->name != 0x6a0db8e5)) { return 0; }
  struct tl_ds_message_media *  result = talloc0 (sizeof (*result));
  result->magic = 0xa32dd600;
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x8d8c0d41, .id = "WebPage", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->webpage = fetch_ds_type_web_page (field1);
  return result;
}
struct tl_ds_message_media *fetch_ds_constructor_message_media_venue (struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x95f2471a && T->type->name != 0x6a0db8e5)) { return 0; }
  struct tl_ds_message_media *  result = talloc0 (sizeof (*result));
  result->magic = 0x7912b71f;
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x315e0a53, .id = "GeoPoint", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->geo = fetch_ds_type_geo_point (field1);
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->title = fetch_ds_type_bare_string (field2);
  struct paramed_type *field3 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->address = fetch_ds_type_bare_string (field3);
  struct paramed_type *field4 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->provider = fetch_ds_type_bare_string (field4);
  struct paramed_type *field5 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->venue_id = fetch_ds_type_bare_string (field5);
  return result;
}
struct tl_ds_message_media *fetch_ds_constructor_message_media_photo_l27 (struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x95f2471a && T->type->name != 0x6a0db8e5)) { return 0; }
  struct tl_ds_message_media *  result = talloc0 (sizeof (*result));
  result->magic = 0xc8c45a2a;
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xc207550a, .id = "Photo", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->photo = fetch_ds_type_photo (field1);
  return result;
}
struct tl_ds_message_media *fetch_ds_constructor_message_media_video_l27 (struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x95f2471a && T->type->name != 0x6a0db8e5)) { return 0; }
  struct tl_ds_message_media *  result = talloc0 (sizeof (*result));
  result->magic = 0xa2d24290;
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x1716b174, .id = "Video", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->video = fetch_ds_type_video (field1);
  return result;
}
struct tl_ds_messages_filter *fetch_ds_constructor_input_messages_filter_empty (struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x80895ae0 && T->type->name != 0x7f76a51f)) { return 0; }
  struct tl_ds_messages_filter *  result = talloc0 (sizeof (*result));
  result->magic = 0x57e2f66c;
  return result;
}
struct tl_ds_messages_filter *fetch_ds_constructor_input_messages_filter_photos (struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x80895ae0 && T->type->name != 0x7f76a51f)) { return 0; }
  struct tl_ds_messages_filter *  result = talloc0 (sizeof (*result));
  result->magic = 0x9609a51c;
  return result;
}
struct tl_ds_messages_filter *fetch_ds_constructor_input_messages_filter_video (struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x80895ae0 && T->type->name != 0x7f76a51f)) { return 0; }
  struct tl_ds_messages_filter *  result = talloc0 (sizeof (*result));
  result->magic = 0x9fc00e65;
  return result;
}
struct tl_ds_messages_filter *fetch_ds_constructor_input_messages_filter_photo_video (struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x80895ae0 && T->type->name != 0x7f76a51f)) { return 0; }
  struct tl_ds_messages_filter *  result = talloc0 (sizeof (*result));
  result->magic = 0x56e9f0e4;
  return result;
}
struct tl_ds_messages_filter *fetch_ds_constructor_input_messages_filter_photo_video_documents (struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x80895ae0 && T->type->name != 0x7f76a51f)) { return 0; }
  struct tl_ds_messages_filter *  result = talloc0 (sizeof (*result));
  result->magic = 0xd95e73bb;
  return result;
}
struct tl_ds_messages_filter *fetch_ds_constructor_input_messages_filter_document (struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x80895ae0 && T->type->name != 0x7f76a51f)) { return 0; }
  struct tl_ds_messages_filter *  result = talloc0 (sizeof (*result));
  result->magic = 0x9eddf188;
  return result;
}
struct tl_ds_messages_filter *fetch_ds_constructor_input_messages_filter_audio (struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x80895ae0 && T->type->name != 0x7f76a51f)) { return 0; }
  struct tl_ds_messages_filter *  result = talloc0 (sizeof (*result));
  result->magic = 0xcfc87522;
  return result;
}
struct tl_ds_nearest_dc *fetch_ds_constructor_nearest_dc (struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x8e1a1775 && T->type->name != 0x71e5e88a)) { return 0; }
  struct tl_ds_nearest_dc *  result = talloc0 (sizeof (*result));
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->country = fetch_ds_type_bare_string (field1);
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->this_dc = fetch_ds_type_bare_int (field2);
  struct paramed_type *field3 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->nearest_dc = fetch_ds_type_bare_int (field3);
  return result;
}
struct tl_ds_notify_peer *fetch_ds_constructor_notify_peer (struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x9fcb8237 && T->type->name != 0x60347dc8)) { return 0; }
  struct tl_ds_notify_peer *  result = talloc0 (sizeof (*result));
  result->magic = 0x9fd40bd8;
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x276159d6, .id = "Peer", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->peer = fetch_ds_type_peer (field1);
  return result;
}
struct tl_ds_notify_peer *fetch_ds_constructor_notify_users (struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x9fcb8237 && T->type->name != 0x60347dc8)) { return 0; }
  struct tl_ds_notify_peer *  result = talloc0 (sizeof (*result));
  result->magic = 0xb4c83b4c;
  return result;
}
struct tl_ds_notify_peer *fetch_ds_constructor_notify_chats (struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x9fcb8237 && T->type->name != 0x60347dc8)) { return 0; }
  struct tl_ds_notify_peer *  result = talloc0 (sizeof (*result));
  result->magic = 0xc007cec3;
  return result;
}
struct tl_ds_notify_peer *fetch_ds_constructor_notify_all (struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x9fcb8237 && T->type->name != 0x60347dc8)) { return 0; }
  struct tl_ds_notify_peer *  result = talloc0 (sizeof (*result));
  result->magic = 0x74d07c60;
  return result;
}
struct tl_ds_null *fetch_ds_constructor_null (struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x56730bcc && T->type->name != 0xa98cf433)) { return 0; }
  struct tl_ds_null *  result = talloc0 (sizeof (*result));
  return result;
}
struct tl_ds_p_q_inner_data *fetch_ds_constructor_p_q_inner_data (struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0xbfa3de38 && T->type->name != 0x405c21c7)) { return 0; }
  struct tl_ds_p_q_inner_data *  result = talloc0 (sizeof (*result));
  result->magic = 0x83c95aec;
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->pq = fetch_ds_type_bare_string (field1);
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->p = fetch_ds_type_bare_string (field2);
  struct paramed_type *field3 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->q = fetch_ds_type_bare_string (field3);
  struct paramed_type *field4 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x82c93bc6, .id = "Bare_Int128", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->nonce = fetch_ds_type_bare_int128 (field4);
  struct paramed_type *field5 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x82c93bc6, .id = "Bare_Int128", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->server_nonce = fetch_ds_type_bare_int128 (field5);
  struct paramed_type *field6 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x0d38674c, .id = "Bare_Int256", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->new_nonce = fetch_ds_type_bare_int256 (field6);
  return result;
}
struct tl_ds_p_q_inner_data *fetch_ds_constructor_p_q_inner_data_temp (struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0xbfa3de38 && T->type->name != 0x405c21c7)) { return 0; }
  struct tl_ds_p_q_inner_data *  result = talloc0 (sizeof (*result));
  result->magic = 0x3c6a84d4;
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->pq = fetch_ds_type_bare_string (field1);
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->p = fetch_ds_type_bare_string (field2);
  struct paramed_type *field3 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->q = fetch_ds_type_bare_string (field3);
  struct paramed_type *field4 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x82c93bc6, .id = "Bare_Int128", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->nonce = fetch_ds_type_bare_int128 (field4);
  struct paramed_type *field5 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x82c93bc6, .id = "Bare_Int128", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->server_nonce = fetch_ds_type_bare_int128 (field5);
  struct paramed_type *field6 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x0d38674c, .id = "Bare_Int256", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->new_nonce = fetch_ds_type_bare_int256 (field6);
  struct paramed_type *field7 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->expires_in = fetch_ds_type_bare_int (field7);
  return result;
}
struct tl_ds_peer *fetch_ds_constructor_peer_user (struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x276159d6 && T->type->name != 0xd89ea629)) { return 0; }
  struct tl_ds_peer *  result = talloc0 (sizeof (*result));
  result->magic = 0x9db1bc6d;
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->user_id = fetch_ds_type_bare_int (field1);
  return result;
}
struct tl_ds_peer *fetch_ds_constructor_peer_chat (struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x276159d6 && T->type->name != 0xd89ea629)) { return 0; }
  struct tl_ds_peer *  result = talloc0 (sizeof (*result));
  result->magic = 0xbad0e5bb;
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->chat_id = fetch_ds_type_bare_int (field1);
  return result;
}
struct tl_ds_peer_notify_events *fetch_ds_constructor_peer_notify_events_empty (struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0xc0c8d13b && T->type->name != 0x3f372ec4)) { return 0; }
  struct tl_ds_peer_notify_events *  result = talloc0 (sizeof (*result));
  result->magic = 0xadd53cb3;
  return result;
}
struct tl_ds_peer_notify_events *fetch_ds_constructor_peer_notify_events_all (struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0xc0c8d13b && T->type->name != 0x3f372ec4)) { return 0; }
  struct tl_ds_peer_notify_events *  result = talloc0 (sizeof (*result));
  result->magic = 0x6d1ded88;
  return result;
}
struct tl_ds_peer_notify_settings *fetch_ds_constructor_peer_notify_settings_empty (struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0xfdf894fc && T->type->name != 0x02076b03)) { return 0; }
  struct tl_ds_peer_notify_settings *  result = talloc0 (sizeof (*result));
  result->magic = 0x70a68512;
  return result;
}
struct tl_ds_peer_notify_settings *fetch_ds_constructor_peer_notify_settings (struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0xfdf894fc && T->type->name != 0x02076b03)) { return 0; }
  struct tl_ds_peer_notify_settings *  result = talloc0 (sizeof (*result));
  result->magic = 0x8d5e11ee;
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->mute_until = fetch_ds_type_bare_int (field1);
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->sound = fetch_ds_type_bare_string (field2);
  struct paramed_type *field3 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x250be282, .id = "Bool", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->show_previews = fetch_ds_type_bool (field3);
  struct paramed_type *field4 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->events_mask = fetch_ds_type_bare_int (field4);
  return result;
}
struct tl_ds_photo *fetch_ds_constructor_photo_empty (struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0xc207550a && T->type->name != 0x3df8aaf5)) { return 0; }
  struct tl_ds_photo *  result = talloc0 (sizeof (*result));
  result->magic = 0x2331b22d;
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xddf89345, .id = "Bare_Long", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->id = fetch_ds_type_bare_long (field1);
  return result;
}
struct tl_ds_photo *fetch_ds_constructor_photo (struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0xc207550a && T->type->name != 0x3df8aaf5)) { return 0; }
  struct tl_ds_photo *  result = talloc0 (sizeof (*result));
  result->magic = 0xc3838076;
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xddf89345, .id = "Bare_Long", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->id = fetch_ds_type_bare_long (field1);
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xddf89345, .id = "Bare_Long", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->access_hash = fetch_ds_type_bare_long (field2);
  struct paramed_type *field3 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->user_id = fetch_ds_type_bare_int (field3);
  struct paramed_type *field4 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->date = fetch_ds_type_bare_int (field4);
  struct paramed_type *field5 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x315e0a53, .id = "GeoPoint", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->geo = fetch_ds_type_geo_point (field5);
  struct paramed_type *field6 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x1cb5c415, .id = "Vector", .params_num = 1, .params_types = 0},
    .params = (struct paramed_type *[]){
      &(struct paramed_type){
        .type = &(struct tl_type_descr) {.name = 0x900f60dd, .id = "PhotoSize", .params_num = 0, .params_types = 0},
        .params = 0,
      },
    }
  };
  result->sizes = (void *)fetch_ds_type_vector (field6);
  return result;
}
struct tl_ds_photo *fetch_ds_constructor_photo_l27 (struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0xc207550a && T->type->name != 0x3df8aaf5)) { return 0; }
  struct tl_ds_photo *  result = talloc0 (sizeof (*result));
  result->magic = 0x22b56751;
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xddf89345, .id = "Bare_Long", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->id = fetch_ds_type_bare_long (field1);
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xddf89345, .id = "Bare_Long", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->access_hash = fetch_ds_type_bare_long (field2);
  struct paramed_type *field3 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->user_id = fetch_ds_type_bare_int (field3);
  struct paramed_type *field4 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->date = fetch_ds_type_bare_int (field4);
  struct paramed_type *field5 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->caption = fetch_ds_type_bare_string (field5);
  struct paramed_type *field6 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x315e0a53, .id = "GeoPoint", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->geo = fetch_ds_type_geo_point (field6);
  struct paramed_type *field7 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x1cb5c415, .id = "Vector", .params_num = 1, .params_types = 0},
    .params = (struct paramed_type *[]){
      &(struct paramed_type){
        .type = &(struct tl_type_descr) {.name = 0x900f60dd, .id = "PhotoSize", .params_num = 0, .params_types = 0},
        .params = 0,
      },
    }
  };
  result->sizes = (void *)fetch_ds_type_vector (field7);
  return result;
}
struct tl_ds_photo_size *fetch_ds_constructor_photo_size_empty (struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x900f60dd && T->type->name != 0x6ff09f22)) { return 0; }
  struct tl_ds_photo_size *  result = talloc0 (sizeof (*result));
  result->magic = 0x0e17e23c;
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->type = fetch_ds_type_bare_string (field1);
  return result;
}
struct tl_ds_photo_size *fetch_ds_constructor_photo_size (struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x900f60dd && T->type->name != 0x6ff09f22)) { return 0; }
  struct tl_ds_photo_size *  result = talloc0 (sizeof (*result));
  result->magic = 0x77bfb61b;
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->type = fetch_ds_type_bare_string (field1);
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x2f8ffb30, .id = "FileLocation", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->location = fetch_ds_type_file_location (field2);
  struct paramed_type *field3 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->w = fetch_ds_type_bare_int (field3);
  struct paramed_type *field4 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->h = fetch_ds_type_bare_int (field4);
  struct paramed_type *field5 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->size = fetch_ds_type_bare_int (field5);
  return result;
}
struct tl_ds_photo_size *fetch_ds_constructor_photo_cached_size (struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x900f60dd && T->type->name != 0x6ff09f22)) { return 0; }
  struct tl_ds_photo_size *  result = talloc0 (sizeof (*result));
  result->magic = 0xe9a734fa;
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->type = fetch_ds_type_bare_string (field1);
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x2f8ffb30, .id = "FileLocation", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->location = fetch_ds_type_file_location (field2);
  struct paramed_type *field3 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->w = fetch_ds_type_bare_int (field3);
  struct paramed_type *field4 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->h = fetch_ds_type_bare_int (field4);
  struct paramed_type *field5 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xf11ec860, .id = "Bare_Bytes", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->bytes = fetch_ds_type_bare_bytes (field5);
  return result;
}
struct tl_ds_privacy_key *fetch_ds_constructor_privacy_key_status_timestamp (struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0xbc2eab30 && T->type->name != 0x43d154cf)) { return 0; }
  struct tl_ds_privacy_key *  result = talloc0 (sizeof (*result));
  return result;
}
struct tl_ds_privacy_rule *fetch_ds_constructor_privacy_value_allow_contacts (struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0xa8638aec && T->type->name != 0x579c7513)) { return 0; }
  struct tl_ds_privacy_rule *  result = talloc0 (sizeof (*result));
  result->magic = 0xfffe1bac;
  return result;
}
struct tl_ds_privacy_rule *fetch_ds_constructor_privacy_value_allow_all (struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0xa8638aec && T->type->name != 0x579c7513)) { return 0; }
  struct tl_ds_privacy_rule *  result = talloc0 (sizeof (*result));
  result->magic = 0x65427b82;
  return result;
}
struct tl_ds_privacy_rule *fetch_ds_constructor_privacy_value_allow_users (struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0xa8638aec && T->type->name != 0x579c7513)) { return 0; }
  struct tl_ds_privacy_rule *  result = talloc0 (sizeof (*result));
  result->magic = 0x4d5bbe0c;
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x1cb5c415, .id = "Vector", .params_num = 1, .params_types = 0},
    .params = (struct paramed_type *[]){
      &(struct paramed_type){
        .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
        .params = 0,
      },
    }
  };
  result->users = (void *)fetch_ds_type_vector (field1);
  return result;
}
struct tl_ds_privacy_rule *fetch_ds_constructor_privacy_value_disallow_contacts (struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0xa8638aec && T->type->name != 0x579c7513)) { return 0; }
  struct tl_ds_privacy_rule *  result = talloc0 (sizeof (*result));
  result->magic = 0xf888fa1a;
  return result;
}
struct tl_ds_privacy_rule *fetch_ds_constructor_privacy_value_disallow_all (struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0xa8638aec && T->type->name != 0x579c7513)) { return 0; }
  struct tl_ds_privacy_rule *  result = talloc0 (sizeof (*result));
  result->magic = 0x8b73e763;
  return result;
}
struct tl_ds_privacy_rule *fetch_ds_constructor_privacy_value_disallow_users (struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0xa8638aec && T->type->name != 0x579c7513)) { return 0; }
  struct tl_ds_privacy_rule *  result = talloc0 (sizeof (*result));
  result->magic = 0x0c7f49b7;
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x1cb5c415, .id = "Vector", .params_num = 1, .params_types = 0},
    .params = (struct paramed_type *[]){
      &(struct paramed_type){
        .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
        .params = 0,
      },
    }
  };
  result->users = (void *)fetch_ds_type_vector (field1);
  return result;
}
struct tl_ds_received_notify_message *fetch_ds_constructor_received_notify_message (struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0xa384b779 && T->type->name != 0x5c7b4886)) { return 0; }
  struct tl_ds_received_notify_message *  result = talloc0 (sizeof (*result));
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->id = fetch_ds_type_bare_int (field1);
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->flags = fetch_ds_type_bare_int (field2);
  return result;
}
struct tl_ds_reply_markup *fetch_ds_constructor_reply_keyboard_hide (struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x612ca4a9 && T->type->name != 0x9ed35b56)) { return 0; }
  struct tl_ds_reply_markup *  result = talloc0 (sizeof (*result));
  result->magic = 0xa03e5b85;
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->flags = fetch_ds_type_bare_int (field1);
  return result;
}
struct tl_ds_reply_markup *fetch_ds_constructor_reply_keyboard_force_reply (struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x612ca4a9 && T->type->name != 0x9ed35b56)) { return 0; }
  struct tl_ds_reply_markup *  result = talloc0 (sizeof (*result));
  result->magic = 0xf4108aa0;
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->flags = fetch_ds_type_bare_int (field1);
  return result;
}
struct tl_ds_reply_markup *fetch_ds_constructor_reply_keyboard_markup (struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x612ca4a9 && T->type->name != 0x9ed35b56)) { return 0; }
  struct tl_ds_reply_markup *  result = talloc0 (sizeof (*result));
  result->magic = 0x3502758c;
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->flags = fetch_ds_type_bare_int (field1);
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x1cb5c415, .id = "Vector", .params_num = 1, .params_types = 0},
    .params = (struct paramed_type *[]){
      &(struct paramed_type){
        .type = &(struct tl_type_descr) {.name = 0x77608b83, .id = "KeyboardButtonRow", .params_num = 0, .params_types = 0},
        .params = 0,
      },
    }
  };
  result->rows = (void *)fetch_ds_type_vector (field2);
  return result;
}
struct tl_ds_res_p_q *fetch_ds_constructor_res_p_q (struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x05162463 && T->type->name != 0xfae9db9c)) { return 0; }
  struct tl_ds_res_p_q *  result = talloc0 (sizeof (*result));
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x82c93bc6, .id = "Bare_Int128", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->nonce = fetch_ds_type_bare_int128 (field1);
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x82c93bc6, .id = "Bare_Int128", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->server_nonce = fetch_ds_type_bare_int128 (field2);
  struct paramed_type *field3 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->pq = fetch_ds_type_bare_string (field3);
  struct paramed_type *field4 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x1cb5c415, .id = "Vector", .params_num = 1, .params_types = 0},
    .params = (struct paramed_type *[]){
      &(struct paramed_type){
        .type = &(struct tl_type_descr) {.name = 0xddf89345, .id = "Bare_Long", .params_num = 0, .params_types = 0},
        .params = 0,
      },
    }
  };
  result->server_public_key_fingerprints = (void *)fetch_ds_type_vector (field4);
  return result;
}
struct tl_ds_send_message_action *fetch_ds_constructor_send_message_typing_action (struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x70541c7e && T->type->name != 0x8fabe381)) { return 0; }
  struct tl_ds_send_message_action *  result = talloc0 (sizeof (*result));
  result->magic = 0x16bf744e;
  return result;
}
struct tl_ds_send_message_action *fetch_ds_constructor_send_message_cancel_action (struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x70541c7e && T->type->name != 0x8fabe381)) { return 0; }
  struct tl_ds_send_message_action *  result = talloc0 (sizeof (*result));
  result->magic = 0xfd5ec8f5;
  return result;
}
struct tl_ds_send_message_action *fetch_ds_constructor_send_message_record_video_action (struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x70541c7e && T->type->name != 0x8fabe381)) { return 0; }
  struct tl_ds_send_message_action *  result = talloc0 (sizeof (*result));
  result->magic = 0xa187d66f;
  return result;
}
struct tl_ds_send_message_action *fetch_ds_constructor_send_message_upload_video_action_l27 (struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x70541c7e && T->type->name != 0x8fabe381)) { return 0; }
  struct tl_ds_send_message_action *  result = talloc0 (sizeof (*result));
  result->magic = 0x92042ff7;
  return result;
}
struct tl_ds_send_message_action *fetch_ds_constructor_send_message_upload_video_action (struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x70541c7e && T->type->name != 0x8fabe381)) { return 0; }
  struct tl_ds_send_message_action *  result = talloc0 (sizeof (*result));
  result->magic = 0xe9763aec;
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->progress = fetch_ds_type_bare_int (field1);
  return result;
}
struct tl_ds_send_message_action *fetch_ds_constructor_send_message_record_audio_action (struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x70541c7e && T->type->name != 0x8fabe381)) { return 0; }
  struct tl_ds_send_message_action *  result = talloc0 (sizeof (*result));
  result->magic = 0xd52f73f7;
  return result;
}
struct tl_ds_send_message_action *fetch_ds_constructor_send_message_upload_audio_action_l27 (struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x70541c7e && T->type->name != 0x8fabe381)) { return 0; }
  struct tl_ds_send_message_action *  result = talloc0 (sizeof (*result));
  result->magic = 0xe6ac8a6f;
  return result;
}
struct tl_ds_send_message_action *fetch_ds_constructor_send_message_upload_audio_action (struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x70541c7e && T->type->name != 0x8fabe381)) { return 0; }
  struct tl_ds_send_message_action *  result = talloc0 (sizeof (*result));
  result->magic = 0xf351d7ab;
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->progress = fetch_ds_type_bare_int (field1);
  return result;
}
struct tl_ds_send_message_action *fetch_ds_constructor_send_message_upload_photo_action (struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x70541c7e && T->type->name != 0x8fabe381)) { return 0; }
  struct tl_ds_send_message_action *  result = talloc0 (sizeof (*result));
  result->magic = 0xd1d34a26;
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->progress = fetch_ds_type_bare_int (field1);
  return result;
}
struct tl_ds_send_message_action *fetch_ds_constructor_send_message_upload_document_action_l27 (struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x70541c7e && T->type->name != 0x8fabe381)) { return 0; }
  struct tl_ds_send_message_action *  result = talloc0 (sizeof (*result));
  result->magic = 0x8faee98e;
  return result;
}
struct tl_ds_send_message_action *fetch_ds_constructor_send_message_upload_document_action (struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x70541c7e && T->type->name != 0x8fabe381)) { return 0; }
  struct tl_ds_send_message_action *  result = talloc0 (sizeof (*result));
  result->magic = 0xaa0cd9e4;
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->progress = fetch_ds_type_bare_int (field1);
  return result;
}
struct tl_ds_send_message_action *fetch_ds_constructor_send_message_geo_location_action (struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x70541c7e && T->type->name != 0x8fabe381)) { return 0; }
  struct tl_ds_send_message_action *  result = talloc0 (sizeof (*result));
  result->magic = 0x176f8ba1;
  return result;
}
struct tl_ds_send_message_action *fetch_ds_constructor_send_message_choose_contact_action (struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x70541c7e && T->type->name != 0x8fabe381)) { return 0; }
  struct tl_ds_send_message_action *  result = talloc0 (sizeof (*result));
  result->magic = 0x628cbc6f;
  return result;
}
struct tl_ds_server_d_h_params *fetch_ds_constructor_server_d_h_params_fail (struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0xa9230301 && T->type->name != 0x56dcfcfe)) { return 0; }
  struct tl_ds_server_d_h_params *  result = talloc0 (sizeof (*result));
  result->magic = 0x79cb045d;
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x82c93bc6, .id = "Bare_Int128", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->nonce = fetch_ds_type_bare_int128 (field1);
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x82c93bc6, .id = "Bare_Int128", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->server_nonce = fetch_ds_type_bare_int128 (field2);
  struct paramed_type *field3 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x82c93bc6, .id = "Bare_Int128", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->new_nonce_hash = fetch_ds_type_bare_int128 (field3);
  return result;
}
struct tl_ds_server_d_h_params *fetch_ds_constructor_server_d_h_params_ok (struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0xa9230301 && T->type->name != 0x56dcfcfe)) { return 0; }
  struct tl_ds_server_d_h_params *  result = talloc0 (sizeof (*result));
  result->magic = 0xd0e8075c;
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x82c93bc6, .id = "Bare_Int128", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->nonce = fetch_ds_type_bare_int128 (field1);
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x82c93bc6, .id = "Bare_Int128", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->server_nonce = fetch_ds_type_bare_int128 (field2);
  struct paramed_type *field3 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->encrypted_answer = fetch_ds_type_bare_string (field3);
  return result;
}
struct tl_ds_server_d_h_inner_data *fetch_ds_constructor_server_d_h_inner_data (struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0xb5890dba && T->type->name != 0x4a76f245)) { return 0; }
  struct tl_ds_server_d_h_inner_data *  result = talloc0 (sizeof (*result));
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x82c93bc6, .id = "Bare_Int128", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->nonce = fetch_ds_type_bare_int128 (field1);
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x82c93bc6, .id = "Bare_Int128", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->server_nonce = fetch_ds_type_bare_int128 (field2);
  struct paramed_type *field3 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->g = fetch_ds_type_bare_int (field3);
  struct paramed_type *field4 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->dh_prime = fetch_ds_type_bare_string (field4);
  struct paramed_type *field5 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->g_a = fetch_ds_type_bare_string (field5);
  struct paramed_type *field6 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->server_time = fetch_ds_type_bare_int (field6);
  return result;
}
struct tl_ds_set_client_d_h_params_answer *fetch_ds_constructor_dh_gen_ok (struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0xdb8a468f && T->type->name != 0x2475b970)) { return 0; }
  struct tl_ds_set_client_d_h_params_answer *  result = talloc0 (sizeof (*result));
  result->magic = 0x3bcbf734;
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x82c93bc6, .id = "Bare_Int128", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->nonce = fetch_ds_type_bare_int128 (field1);
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x82c93bc6, .id = "Bare_Int128", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->server_nonce = fetch_ds_type_bare_int128 (field2);
  struct paramed_type *field3 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x82c93bc6, .id = "Bare_Int128", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->new_nonce_hash1 = fetch_ds_type_bare_int128 (field3);
  return result;
}
struct tl_ds_set_client_d_h_params_answer *fetch_ds_constructor_dh_gen_retry (struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0xdb8a468f && T->type->name != 0x2475b970)) { return 0; }
  struct tl_ds_set_client_d_h_params_answer *  result = talloc0 (sizeof (*result));
  result->magic = 0x46dc1fb9;
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x82c93bc6, .id = "Bare_Int128", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->nonce = fetch_ds_type_bare_int128 (field1);
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x82c93bc6, .id = "Bare_Int128", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->server_nonce = fetch_ds_type_bare_int128 (field2);
  struct paramed_type *field3 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x82c93bc6, .id = "Bare_Int128", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->new_nonce_hash2 = fetch_ds_type_bare_int128 (field3);
  return result;
}
struct tl_ds_set_client_d_h_params_answer *fetch_ds_constructor_dh_gen_fail (struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0xdb8a468f && T->type->name != 0x2475b970)) { return 0; }
  struct tl_ds_set_client_d_h_params_answer *  result = talloc0 (sizeof (*result));
  result->magic = 0xa69dae02;
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x82c93bc6, .id = "Bare_Int128", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->nonce = fetch_ds_type_bare_int128 (field1);
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x82c93bc6, .id = "Bare_Int128", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->server_nonce = fetch_ds_type_bare_int128 (field2);
  struct paramed_type *field3 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x82c93bc6, .id = "Bare_Int128", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->new_nonce_hash3 = fetch_ds_type_bare_int128 (field3);
  return result;
}
struct tl_ds_sticker_pack *fetch_ds_constructor_sticker_pack (struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x12b299d4 && T->type->name != 0xed4d662b)) { return 0; }
  struct tl_ds_sticker_pack *  result = talloc0 (sizeof (*result));
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->emoticon = fetch_ds_type_bare_string (field1);
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x1cb5c415, .id = "Vector", .params_num = 1, .params_types = 0},
    .params = (struct paramed_type *[]){
      &(struct paramed_type){
        .type = &(struct tl_type_descr) {.name = 0xddf89345, .id = "Bare_Long", .params_num = 0, .params_types = 0},
        .params = 0,
      },
    }
  };
  result->documents = (void *)fetch_ds_type_vector (field2);
  return result;
}
struct tl_ds_sticker_set *fetch_ds_constructor_sticker_set (struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0xa7a43b17 && T->type->name != 0x585bc4e8)) { return 0; }
  struct tl_ds_sticker_set *  result = talloc0 (sizeof (*result));
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xddf89345, .id = "Bare_Long", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->id = fetch_ds_type_bare_long (field1);
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xddf89345, .id = "Bare_Long", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->access_hash = fetch_ds_type_bare_long (field2);
  struct paramed_type *field3 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->title = fetch_ds_type_bare_string (field3);
  struct paramed_type *field4 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->short_name = fetch_ds_type_bare_string (field4);
  return result;
}
struct tl_ds_string *fetch_ds_constructor_string (struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0xb5286e24 && T->type->name != 0x4ad791db)) { return 0; }
  struct tl_ds_string *  result = talloc0 (sizeof (*result));
  assert (in_remaining () >= 4);
  int l = prefetch_strlen ();
  assert (l >= 0);
  result->len = l;
  result->data = talloc (l + 1);
  result->data[l] = 0;
  memcpy (result->data, fetch_str (l), l);
  return result;
}
struct tl_ds_update *fetch_ds_constructor_update_new_message (struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x2e02b27e && T->type->name != 0xd1fd4d81)) { return 0; }
  struct tl_ds_update *  result = talloc0 (sizeof (*result));
  result->magic = 0x1f2b0afd;
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x5d652a7f, .id = "Message", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->message = fetch_ds_type_message (field1);
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->pts = fetch_ds_type_bare_int (field2);
  struct paramed_type *field3 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->pts_count = fetch_ds_type_bare_int (field3);
  return result;
}
struct tl_ds_update *fetch_ds_constructor_update_message_i_d (struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x2e02b27e && T->type->name != 0xd1fd4d81)) { return 0; }
  struct tl_ds_update *  result = talloc0 (sizeof (*result));
  result->magic = 0x4e90bfd6;
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->id = fetch_ds_type_bare_int (field1);
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xddf89345, .id = "Bare_Long", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->random_id = fetch_ds_type_bare_long (field2);
  return result;
}
struct tl_ds_update *fetch_ds_constructor_update_delete_messages (struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x2e02b27e && T->type->name != 0xd1fd4d81)) { return 0; }
  struct tl_ds_update *  result = talloc0 (sizeof (*result));
  result->magic = 0xa20db0e5;
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x1cb5c415, .id = "Vector", .params_num = 1, .params_types = 0},
    .params = (struct paramed_type *[]){
      &(struct paramed_type){
        .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
        .params = 0,
      },
    }
  };
  result->messages = (void *)fetch_ds_type_vector (field1);
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->pts = fetch_ds_type_bare_int (field2);
  struct paramed_type *field3 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->pts_count = fetch_ds_type_bare_int (field3);
  return result;
}
struct tl_ds_update *fetch_ds_constructor_update_user_typing (struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x2e02b27e && T->type->name != 0xd1fd4d81)) { return 0; }
  struct tl_ds_update *  result = talloc0 (sizeof (*result));
  result->magic = 0x5c486927;
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->user_id = fetch_ds_type_bare_int (field1);
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x70541c7e, .id = "SendMessageAction", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->action = fetch_ds_type_send_message_action (field2);
  return result;
}
struct tl_ds_update *fetch_ds_constructor_update_chat_user_typing (struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x2e02b27e && T->type->name != 0xd1fd4d81)) { return 0; }
  struct tl_ds_update *  result = talloc0 (sizeof (*result));
  result->magic = 0x9a65ea1f;
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->chat_id = fetch_ds_type_bare_int (field1);
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->user_id = fetch_ds_type_bare_int (field2);
  struct paramed_type *field3 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x70541c7e, .id = "SendMessageAction", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->action = fetch_ds_type_send_message_action (field3);
  return result;
}
struct tl_ds_update *fetch_ds_constructor_update_chat_participants (struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x2e02b27e && T->type->name != 0xd1fd4d81)) { return 0; }
  struct tl_ds_update *  result = talloc0 (sizeof (*result));
  result->magic = 0x07761198;
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x77930f9f, .id = "ChatParticipants", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->participants = fetch_ds_type_chat_participants (field1);
  return result;
}
struct tl_ds_update *fetch_ds_constructor_update_user_status (struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x2e02b27e && T->type->name != 0xd1fd4d81)) { return 0; }
  struct tl_ds_update *  result = talloc0 (sizeof (*result));
  result->magic = 0x1bfbd823;
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->user_id = fetch_ds_type_bare_int (field1);
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x76de9570, .id = "UserStatus", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->status = fetch_ds_type_user_status (field2);
  return result;
}
struct tl_ds_update *fetch_ds_constructor_update_user_name (struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x2e02b27e && T->type->name != 0xd1fd4d81)) { return 0; }
  struct tl_ds_update *  result = talloc0 (sizeof (*result));
  result->magic = 0xa7332b73;
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->user_id = fetch_ds_type_bare_int (field1);
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->first_name = fetch_ds_type_bare_string (field2);
  struct paramed_type *field3 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->last_name = fetch_ds_type_bare_string (field3);
  struct paramed_type *field4 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->username = fetch_ds_type_bare_string (field4);
  return result;
}
struct tl_ds_update *fetch_ds_constructor_update_user_photo (struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x2e02b27e && T->type->name != 0xd1fd4d81)) { return 0; }
  struct tl_ds_update *  result = talloc0 (sizeof (*result));
  result->magic = 0x95313b0c;
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->user_id = fetch_ds_type_bare_int (field1);
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->date = fetch_ds_type_bare_int (field2);
  struct paramed_type *field3 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x9a486229, .id = "UserProfilePhoto", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->photo = fetch_ds_type_user_profile_photo (field3);
  struct paramed_type *field4 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x250be282, .id = "Bool", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->previous = fetch_ds_type_bool (field4);
  return result;
}
struct tl_ds_update *fetch_ds_constructor_update_contact_registered (struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x2e02b27e && T->type->name != 0xd1fd4d81)) { return 0; }
  struct tl_ds_update *  result = talloc0 (sizeof (*result));
  result->magic = 0x2575bbb9;
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->user_id = fetch_ds_type_bare_int (field1);
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->date = fetch_ds_type_bare_int (field2);
  return result;
}
struct tl_ds_update *fetch_ds_constructor_update_contact_link (struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x2e02b27e && T->type->name != 0xd1fd4d81)) { return 0; }
  struct tl_ds_update *  result = talloc0 (sizeof (*result));
  result->magic = 0x9d2e67c5;
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->user_id = fetch_ds_type_bare_int (field1);
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x522fbc63, .id = "ContactLink", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->my_link = fetch_ds_type_contact_link (field2);
  struct paramed_type *field3 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x522fbc63, .id = "ContactLink", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->foreign_link = fetch_ds_type_contact_link (field3);
  return result;
}
struct tl_ds_update *fetch_ds_constructor_update_new_authorization (struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x2e02b27e && T->type->name != 0xd1fd4d81)) { return 0; }
  struct tl_ds_update *  result = talloc0 (sizeof (*result));
  result->magic = 0x8f06529a;
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xddf89345, .id = "Bare_Long", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->auth_key_id = fetch_ds_type_bare_long (field1);
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->date = fetch_ds_type_bare_int (field2);
  struct paramed_type *field3 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->device = fetch_ds_type_bare_string (field3);
  struct paramed_type *field4 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->location = fetch_ds_type_bare_string (field4);
  return result;
}
struct tl_ds_update *fetch_ds_constructor_update_new_geo_chat_message (struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x2e02b27e && T->type->name != 0xd1fd4d81)) { return 0; }
  struct tl_ds_update *  result = talloc0 (sizeof (*result));
  result->magic = 0x5a68e3f7;
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xf67b4034, .id = "GeoChatMessage", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->geo_message = fetch_ds_type_geo_chat_message (field1);
  return result;
}
struct tl_ds_update *fetch_ds_constructor_update_new_encrypted_message (struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x2e02b27e && T->type->name != 0xd1fd4d81)) { return 0; }
  struct tl_ds_update *  result = talloc0 (sizeof (*result));
  result->magic = 0x12bcbd9a;
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xce6b8a1e, .id = "EncryptedMessage", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->encr_message = fetch_ds_type_encrypted_message (field1);
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->qts = fetch_ds_type_bare_int (field2);
  return result;
}
struct tl_ds_update *fetch_ds_constructor_update_encrypted_chat_typing (struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x2e02b27e && T->type->name != 0xd1fd4d81)) { return 0; }
  struct tl_ds_update *  result = talloc0 (sizeof (*result));
  result->magic = 0x1710f156;
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->chat_id = fetch_ds_type_bare_int (field1);
  return result;
}
struct tl_ds_update *fetch_ds_constructor_update_encryption (struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x2e02b27e && T->type->name != 0xd1fd4d81)) { return 0; }
  struct tl_ds_update *  result = talloc0 (sizeof (*result));
  result->magic = 0xb4a2e88d;
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xb1718213, .id = "EncryptedChat", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->encr_chat = fetch_ds_type_encrypted_chat (field1);
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->date = fetch_ds_type_bare_int (field2);
  return result;
}
struct tl_ds_update *fetch_ds_constructor_update_encrypted_messages_read (struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x2e02b27e && T->type->name != 0xd1fd4d81)) { return 0; }
  struct tl_ds_update *  result = talloc0 (sizeof (*result));
  result->magic = 0x38fe25b7;
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->chat_id = fetch_ds_type_bare_int (field1);
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->max_date = fetch_ds_type_bare_int (field2);
  struct paramed_type *field3 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->date = fetch_ds_type_bare_int (field3);
  return result;
}
struct tl_ds_update *fetch_ds_constructor_update_chat_participant_add (struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x2e02b27e && T->type->name != 0xd1fd4d81)) { return 0; }
  struct tl_ds_update *  result = talloc0 (sizeof (*result));
  result->magic = 0x3a0eeb22;
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->chat_id = fetch_ds_type_bare_int (field1);
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->user_id = fetch_ds_type_bare_int (field2);
  struct paramed_type *field3 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->inviter_id = fetch_ds_type_bare_int (field3);
  struct paramed_type *field4 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->version = fetch_ds_type_bare_int (field4);
  return result;
}
struct tl_ds_update *fetch_ds_constructor_update_chat_participant_delete (struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x2e02b27e && T->type->name != 0xd1fd4d81)) { return 0; }
  struct tl_ds_update *  result = talloc0 (sizeof (*result));
  result->magic = 0x6e5f8c22;
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->chat_id = fetch_ds_type_bare_int (field1);
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->user_id = fetch_ds_type_bare_int (field2);
  struct paramed_type *field3 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->version = fetch_ds_type_bare_int (field3);
  return result;
}
struct tl_ds_update *fetch_ds_constructor_update_dc_options (struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x2e02b27e && T->type->name != 0xd1fd4d81)) { return 0; }
  struct tl_ds_update *  result = talloc0 (sizeof (*result));
  result->magic = 0x8e5e9873;
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x1cb5c415, .id = "Vector", .params_num = 1, .params_types = 0},
    .params = (struct paramed_type *[]){
      &(struct paramed_type){
        .type = &(struct tl_type_descr) {.name = 0x2b1a62f0, .id = "DcOption", .params_num = 0, .params_types = 0},
        .params = 0,
      },
    }
  };
  result->dc_options = (void *)fetch_ds_type_vector (field1);
  return result;
}
struct tl_ds_update *fetch_ds_constructor_update_user_blocked (struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x2e02b27e && T->type->name != 0xd1fd4d81)) { return 0; }
  struct tl_ds_update *  result = talloc0 (sizeof (*result));
  result->magic = 0x80ece81a;
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->user_id = fetch_ds_type_bare_int (field1);
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x250be282, .id = "Bool", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->blocked = fetch_ds_type_bool (field2);
  return result;
}
struct tl_ds_update *fetch_ds_constructor_update_notify_settings (struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x2e02b27e && T->type->name != 0xd1fd4d81)) { return 0; }
  struct tl_ds_update *  result = talloc0 (sizeof (*result));
  result->magic = 0xbec268ef;
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x9fcb8237, .id = "NotifyPeer", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->notify_peer = fetch_ds_type_notify_peer (field1);
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xfdf894fc, .id = "PeerNotifySettings", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->notify_settings = fetch_ds_type_peer_notify_settings (field2);
  return result;
}
struct tl_ds_update *fetch_ds_constructor_update_service_notification (struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x2e02b27e && T->type->name != 0xd1fd4d81)) { return 0; }
  struct tl_ds_update *  result = talloc0 (sizeof (*result));
  result->magic = 0x382dd3e4;
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->type = fetch_ds_type_bare_string (field1);
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->message_text = fetch_ds_type_bare_string (field2);
  struct paramed_type *field3 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x95f2471a, .id = "MessageMedia", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->media = fetch_ds_type_message_media (field3);
  struct paramed_type *field4 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x250be282, .id = "Bool", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->popup = fetch_ds_type_bool (field4);
  return result;
}
struct tl_ds_update *fetch_ds_constructor_update_privacy (struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x2e02b27e && T->type->name != 0xd1fd4d81)) { return 0; }
  struct tl_ds_update *  result = talloc0 (sizeof (*result));
  result->magic = 0xee3b272a;
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xbc2eab30, .id = "PrivacyKey", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->key = fetch_ds_type_privacy_key (field1);
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x1cb5c415, .id = "Vector", .params_num = 1, .params_types = 0},
    .params = (struct paramed_type *[]){
      &(struct paramed_type){
        .type = &(struct tl_type_descr) {.name = 0xa8638aec, .id = "PrivacyRule", .params_num = 0, .params_types = 0},
        .params = 0,
      },
    }
  };
  result->rules = (void *)fetch_ds_type_vector (field2);
  return result;
}
struct tl_ds_update *fetch_ds_constructor_update_user_phone (struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x2e02b27e && T->type->name != 0xd1fd4d81)) { return 0; }
  struct tl_ds_update *  result = talloc0 (sizeof (*result));
  result->magic = 0x12b9417b;
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->user_id = fetch_ds_type_bare_int (field1);
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->phone = fetch_ds_type_bare_string (field2);
  return result;
}
struct tl_ds_update *fetch_ds_constructor_update_read_history_inbox (struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x2e02b27e && T->type->name != 0xd1fd4d81)) { return 0; }
  struct tl_ds_update *  result = talloc0 (sizeof (*result));
  result->magic = 0x9961fd5c;
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x276159d6, .id = "Peer", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->peer = fetch_ds_type_peer (field1);
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->max_id = fetch_ds_type_bare_int (field2);
  struct paramed_type *field3 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->pts = fetch_ds_type_bare_int (field3);
  struct paramed_type *field4 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->pts_count = fetch_ds_type_bare_int (field4);
  return result;
}
struct tl_ds_update *fetch_ds_constructor_update_read_history_outbox (struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x2e02b27e && T->type->name != 0xd1fd4d81)) { return 0; }
  struct tl_ds_update *  result = talloc0 (sizeof (*result));
  result->magic = 0x2f2f21bf;
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x276159d6, .id = "Peer", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->peer = fetch_ds_type_peer (field1);
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->max_id = fetch_ds_type_bare_int (field2);
  struct paramed_type *field3 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->pts = fetch_ds_type_bare_int (field3);
  struct paramed_type *field4 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->pts_count = fetch_ds_type_bare_int (field4);
  return result;
}
struct tl_ds_update *fetch_ds_constructor_update_web_page (struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x2e02b27e && T->type->name != 0xd1fd4d81)) { return 0; }
  struct tl_ds_update *  result = talloc0 (sizeof (*result));
  result->magic = 0x2cc36971;
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x8d8c0d41, .id = "WebPage", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->webpage = fetch_ds_type_web_page (field1);
  return result;
}
struct tl_ds_update *fetch_ds_constructor_update_read_messages_contents (struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x2e02b27e && T->type->name != 0xd1fd4d81)) { return 0; }
  struct tl_ds_update *  result = talloc0 (sizeof (*result));
  result->magic = 0x68c13933;
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x1cb5c415, .id = "Vector", .params_num = 1, .params_types = 0},
    .params = (struct paramed_type *[]){
      &(struct paramed_type){
        .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
        .params = 0,
      },
    }
  };
  result->messages = (void *)fetch_ds_type_vector (field1);
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->pts = fetch_ds_type_bare_int (field2);
  struct paramed_type *field3 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->pts_count = fetch_ds_type_bare_int (field3);
  return result;
}
struct tl_ds_update *fetch_ds_constructor_update_msg_update (struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x2e02b27e && T->type->name != 0xd1fd4d81)) { return 0; }
  struct tl_ds_update *  result = talloc0 (sizeof (*result));
  result->magic = 0x03114739;
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->id = fetch_ds_type_bare_int (field1);
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->pts = fetch_ds_type_bare_int (field2);
  struct paramed_type *field3 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->pts_count = fetch_ds_type_bare_int (field3);
  return result;
}
struct tl_ds_updates *fetch_ds_constructor_updates_too_long (struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x22499d27 && T->type->name != 0xddb662d8)) { return 0; }
  struct tl_ds_updates *  result = talloc0 (sizeof (*result));
  result->magic = 0xe317af7e;
  return result;
}
struct tl_ds_updates *fetch_ds_constructor_update_short_message (struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x22499d27 && T->type->name != 0xddb662d8)) { return 0; }
  struct tl_ds_updates *  result = talloc0 (sizeof (*result));
  result->magic = 0xed5c2127;
  assert (in_remaining () >= 4);
  result->flags = talloc (4);  *result->flags = prefetch_int ();  struct paramed_type *var0 = INT2PTR (fetch_int ());
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->id = fetch_ds_type_bare_int (field2);
  struct paramed_type *field3 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->user_id = fetch_ds_type_bare_int (field3);
  struct paramed_type *field4 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->message = fetch_ds_type_bare_string (field4);
  struct paramed_type *field5 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->pts = fetch_ds_type_bare_int (field5);
  struct paramed_type *field6 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->pts_count = fetch_ds_type_bare_int (field6);
  struct paramed_type *field7 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->date = fetch_ds_type_bare_int (field7);
  if (PTR2INT (var0) & (1 << 2)) {
    struct paramed_type *field8 = 
    &(struct paramed_type){
      .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
      .params = 0,
    };
    result->fwd_from_id = fetch_ds_type_bare_int (field8);
  }
  if (PTR2INT (var0) & (1 << 2)) {
    struct paramed_type *field9 = 
    &(struct paramed_type){
      .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
      .params = 0,
    };
    result->fwd_date = fetch_ds_type_bare_int (field9);
  }
  if (PTR2INT (var0) & (1 << 3)) {
    struct paramed_type *field10 = 
    &(struct paramed_type){
      .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
      .params = 0,
    };
    result->reply_to_msg_id = fetch_ds_type_bare_int (field10);
  }
  return result;
}
struct tl_ds_updates *fetch_ds_constructor_update_short_chat_message (struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x22499d27 && T->type->name != 0xddb662d8)) { return 0; }
  struct tl_ds_updates *  result = talloc0 (sizeof (*result));
  result->magic = 0x52238b3c;
  assert (in_remaining () >= 4);
  result->flags = talloc (4);  *result->flags = prefetch_int ();  struct paramed_type *var0 = INT2PTR (fetch_int ());
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->id = fetch_ds_type_bare_int (field2);
  struct paramed_type *field3 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->from_id = fetch_ds_type_bare_int (field3);
  struct paramed_type *field4 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->chat_id = fetch_ds_type_bare_int (field4);
  struct paramed_type *field5 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->message = fetch_ds_type_bare_string (field5);
  struct paramed_type *field6 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->pts = fetch_ds_type_bare_int (field6);
  struct paramed_type *field7 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->pts_count = fetch_ds_type_bare_int (field7);
  struct paramed_type *field8 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->date = fetch_ds_type_bare_int (field8);
  if (PTR2INT (var0) & (1 << 2)) {
    struct paramed_type *field9 = 
    &(struct paramed_type){
      .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
      .params = 0,
    };
    result->fwd_from_id = fetch_ds_type_bare_int (field9);
  }
  if (PTR2INT (var0) & (1 << 2)) {
    struct paramed_type *field10 = 
    &(struct paramed_type){
      .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
      .params = 0,
    };
    result->fwd_date = fetch_ds_type_bare_int (field10);
  }
  if (PTR2INT (var0) & (1 << 3)) {
    struct paramed_type *field11 = 
    &(struct paramed_type){
      .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
      .params = 0,
    };
    result->reply_to_msg_id = fetch_ds_type_bare_int (field11);
  }
  return result;
}
struct tl_ds_updates *fetch_ds_constructor_update_short (struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x22499d27 && T->type->name != 0xddb662d8)) { return 0; }
  struct tl_ds_updates *  result = talloc0 (sizeof (*result));
  result->magic = 0x78d4dec1;
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x2e02b27e, .id = "Update", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->update = fetch_ds_type_update (field1);
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->date = fetch_ds_type_bare_int (field2);
  return result;
}
struct tl_ds_updates *fetch_ds_constructor_updates_combined (struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x22499d27 && T->type->name != 0xddb662d8)) { return 0; }
  struct tl_ds_updates *  result = talloc0 (sizeof (*result));
  result->magic = 0x725b04c3;
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x1cb5c415, .id = "Vector", .params_num = 1, .params_types = 0},
    .params = (struct paramed_type *[]){
      &(struct paramed_type){
        .type = &(struct tl_type_descr) {.name = 0x2e02b27e, .id = "Update", .params_num = 0, .params_types = 0},
        .params = 0,
      },
    }
  };
  result->updates = (void *)fetch_ds_type_vector (field1);
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x1cb5c415, .id = "Vector", .params_num = 1, .params_types = 0},
    .params = (struct paramed_type *[]){
      &(struct paramed_type){
        .type = &(struct tl_type_descr) {.name = 0x02e6c0c8, .id = "User", .params_num = 0, .params_types = 0},
        .params = 0,
      },
    }
  };
  result->users = (void *)fetch_ds_type_vector (field2);
  struct paramed_type *field3 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x1cb5c415, .id = "Vector", .params_num = 1, .params_types = 0},
    .params = (struct paramed_type *[]){
      &(struct paramed_type){
        .type = &(struct tl_type_descr) {.name = 0x7bd865dc, .id = "Chat", .params_num = 0, .params_types = 0},
        .params = 0,
      },
    }
  };
  result->chats = (void *)fetch_ds_type_vector (field3);
  struct paramed_type *field4 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->date = fetch_ds_type_bare_int (field4);
  struct paramed_type *field5 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->seq_start = fetch_ds_type_bare_int (field5);
  struct paramed_type *field6 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->seq = fetch_ds_type_bare_int (field6);
  return result;
}
struct tl_ds_updates *fetch_ds_constructor_updates (struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x22499d27 && T->type->name != 0xddb662d8)) { return 0; }
  struct tl_ds_updates *  result = talloc0 (sizeof (*result));
  result->magic = 0x74ae4240;
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x1cb5c415, .id = "Vector", .params_num = 1, .params_types = 0},
    .params = (struct paramed_type *[]){
      &(struct paramed_type){
        .type = &(struct tl_type_descr) {.name = 0x2e02b27e, .id = "Update", .params_num = 0, .params_types = 0},
        .params = 0,
      },
    }
  };
  result->updates = (void *)fetch_ds_type_vector (field1);
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x1cb5c415, .id = "Vector", .params_num = 1, .params_types = 0},
    .params = (struct paramed_type *[]){
      &(struct paramed_type){
        .type = &(struct tl_type_descr) {.name = 0x02e6c0c8, .id = "User", .params_num = 0, .params_types = 0},
        .params = 0,
      },
    }
  };
  result->users = (void *)fetch_ds_type_vector (field2);
  struct paramed_type *field3 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x1cb5c415, .id = "Vector", .params_num = 1, .params_types = 0},
    .params = (struct paramed_type *[]){
      &(struct paramed_type){
        .type = &(struct tl_type_descr) {.name = 0x7bd865dc, .id = "Chat", .params_num = 0, .params_types = 0},
        .params = 0,
      },
    }
  };
  result->chats = (void *)fetch_ds_type_vector (field3);
  struct paramed_type *field4 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->date = fetch_ds_type_bare_int (field4);
  struct paramed_type *field5 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->seq = fetch_ds_type_bare_int (field5);
  return result;
}
struct tl_ds_user *fetch_ds_constructor_user_empty (struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x02e6c0c8 && T->type->name != 0xfd193f37)) { return 0; }
  struct tl_ds_user *  result = talloc0 (sizeof (*result));
  result->magic = 0x200250ba;
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->id = fetch_ds_type_bare_int (field1);
  return result;
}
struct tl_ds_user *fetch_ds_constructor_user (struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x02e6c0c8 && T->type->name != 0xfd193f37)) { return 0; }
  struct tl_ds_user *  result = talloc0 (sizeof (*result));
  result->magic = 0x22e49072;
  assert (in_remaining () >= 4);
  result->flags = talloc (4);  *result->flags = prefetch_int ();  struct paramed_type *var0 = INT2PTR (fetch_int ());
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->id = fetch_ds_type_bare_int (field2);
  if (PTR2INT (var0) & (1 << 0)) {
    struct paramed_type *field3 = 
    &(struct paramed_type){
      .type = &(struct tl_type_descr) {.name = 0xddf89345, .id = "Bare_Long", .params_num = 0, .params_types = 0},
      .params = 0,
    };
    result->access_hash = fetch_ds_type_bare_long (field3);
  }
  if (PTR2INT (var0) & (1 << 1)) {
    struct paramed_type *field4 = 
    &(struct paramed_type){
      .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
      .params = 0,
    };
    result->first_name = fetch_ds_type_bare_string (field4);
  }
  if (PTR2INT (var0) & (1 << 2)) {
    struct paramed_type *field5 = 
    &(struct paramed_type){
      .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
      .params = 0,
    };
    result->last_name = fetch_ds_type_bare_string (field5);
  }
  if (PTR2INT (var0) & (1 << 3)) {
    struct paramed_type *field6 = 
    &(struct paramed_type){
      .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
      .params = 0,
    };
    result->username = fetch_ds_type_bare_string (field6);
  }
  if (PTR2INT (var0) & (1 << 4)) {
    struct paramed_type *field7 = 
    &(struct paramed_type){
      .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
      .params = 0,
    };
    result->phone = fetch_ds_type_bare_string (field7);
  }
  if (PTR2INT (var0) & (1 << 5)) {
    struct paramed_type *field8 = 
    &(struct paramed_type){
      .type = &(struct tl_type_descr) {.name = 0x9a486229, .id = "UserProfilePhoto", .params_num = 0, .params_types = 0},
      .params = 0,
    };
    result->photo = fetch_ds_type_user_profile_photo (field8);
  }
  if (PTR2INT (var0) & (1 << 6)) {
    struct paramed_type *field9 = 
    &(struct paramed_type){
      .type = &(struct tl_type_descr) {.name = 0x76de9570, .id = "UserStatus", .params_num = 0, .params_types = 0},
      .params = 0,
    };
    result->status = fetch_ds_type_user_status (field9);
  }
  if (PTR2INT (var0) & (1 << 14)) {
    struct paramed_type *field10 = 
    &(struct paramed_type){
      .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
      .params = 0,
    };
    result->bot_info_version = fetch_ds_type_bare_int (field10);
  }
  return result;
}
struct tl_ds_user_full *fetch_ds_constructor_user_full (struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x5a89ac5b && T->type->name != 0xa57653a4)) { return 0; }
  struct tl_ds_user_full *  result = talloc0 (sizeof (*result));
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x02e6c0c8, .id = "User", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->user = fetch_ds_type_user (field1);
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x3ace484c, .id = "contacts.Link", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->link = fetch_ds_type_contacts_link (field2);
  struct paramed_type *field3 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xc207550a, .id = "Photo", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->profile_photo = fetch_ds_type_photo (field3);
  struct paramed_type *field4 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xfdf894fc, .id = "PeerNotifySettings", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->notify_settings = fetch_ds_type_peer_notify_settings (field4);
  struct paramed_type *field5 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x250be282, .id = "Bool", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->blocked = fetch_ds_type_bool (field5);
  struct paramed_type *field6 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xb2e16f93, .id = "BotInfo", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->bot_info = fetch_ds_type_bot_info (field6);
  return result;
}
struct tl_ds_user_profile_photo *fetch_ds_constructor_user_profile_photo_empty (struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x9a486229 && T->type->name != 0x65b79dd6)) { return 0; }
  struct tl_ds_user_profile_photo *  result = talloc0 (sizeof (*result));
  result->magic = 0x4f11bae1;
  return result;
}
struct tl_ds_user_profile_photo *fetch_ds_constructor_user_profile_photo (struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x9a486229 && T->type->name != 0x65b79dd6)) { return 0; }
  struct tl_ds_user_profile_photo *  result = talloc0 (sizeof (*result));
  result->magic = 0xd559d8c8;
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xddf89345, .id = "Bare_Long", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->photo_id = fetch_ds_type_bare_long (field1);
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x2f8ffb30, .id = "FileLocation", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->photo_small = fetch_ds_type_file_location (field2);
  struct paramed_type *field3 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x2f8ffb30, .id = "FileLocation", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->photo_big = fetch_ds_type_file_location (field3);
  return result;
}
struct tl_ds_user_status *fetch_ds_constructor_user_status_empty (struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x76de9570 && T->type->name != 0x89216a8f)) { return 0; }
  struct tl_ds_user_status *  result = talloc0 (sizeof (*result));
  result->magic = 0x09d05049;
  return result;
}
struct tl_ds_user_status *fetch_ds_constructor_user_status_online (struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x76de9570 && T->type->name != 0x89216a8f)) { return 0; }
  struct tl_ds_user_status *  result = talloc0 (sizeof (*result));
  result->magic = 0xedb93949;
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->expires = fetch_ds_type_bare_int (field1);
  return result;
}
struct tl_ds_user_status *fetch_ds_constructor_user_status_offline (struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x76de9570 && T->type->name != 0x89216a8f)) { return 0; }
  struct tl_ds_user_status *  result = talloc0 (sizeof (*result));
  result->magic = 0x008c703f;
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->was_online = fetch_ds_type_bare_int (field1);
  return result;
}
struct tl_ds_user_status *fetch_ds_constructor_user_status_recently (struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x76de9570 && T->type->name != 0x89216a8f)) { return 0; }
  struct tl_ds_user_status *  result = talloc0 (sizeof (*result));
  result->magic = 0xe26f42f1;
  return result;
}
struct tl_ds_user_status *fetch_ds_constructor_user_status_last_week (struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x76de9570 && T->type->name != 0x89216a8f)) { return 0; }
  struct tl_ds_user_status *  result = talloc0 (sizeof (*result));
  result->magic = 0x07bf09fc;
  return result;
}
struct tl_ds_user_status *fetch_ds_constructor_user_status_last_month (struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x76de9570 && T->type->name != 0x89216a8f)) { return 0; }
  struct tl_ds_user_status *  result = talloc0 (sizeof (*result));
  result->magic = 0x77ebc742;
  return result;
}
struct tl_ds_vector *fetch_ds_constructor_vector (struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x1cb5c415 && T->type->name != 0xe34a3bea)) { return 0; }
  if (ODDP(T->params[0])) { return 0; }
  struct paramed_type *var0 = T->params[0]; assert (var0);
  struct tl_ds_vector *  result = talloc0 (sizeof (*result));
  assert (in_remaining () >= 4);
  result->f1 = talloc (4);  *result->f1 = prefetch_int ();  struct paramed_type *var1 = INT2PTR (fetch_int ());
  int multiplicity3 = PTR2INT (
  ((void *)var1) + 0  );
  struct paramed_type *field3 = 
  var0;
  result->f2 = talloc0 (multiplicity3 * sizeof (void *));
  {
    int i = 0;
    while (i < multiplicity3) {
      result->f2[i ++] = fetch_ds_type_any (field3);
    }
  }
  return result;
}
struct tl_ds_video *fetch_ds_constructor_video_empty (struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x1716b174 && T->type->name != 0xe8e94e8b)) { return 0; }
  struct tl_ds_video *  result = talloc0 (sizeof (*result));
  result->magic = 0xc10658a8;
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xddf89345, .id = "Bare_Long", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->id = fetch_ds_type_bare_long (field1);
  return result;
}
struct tl_ds_video *fetch_ds_constructor_video (struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x1716b174 && T->type->name != 0xe8e94e8b)) { return 0; }
  struct tl_ds_video *  result = talloc0 (sizeof (*result));
  result->magic = 0xee9f4a4d;
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xddf89345, .id = "Bare_Long", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->id = fetch_ds_type_bare_long (field1);
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xddf89345, .id = "Bare_Long", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->access_hash = fetch_ds_type_bare_long (field2);
  struct paramed_type *field3 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->user_id = fetch_ds_type_bare_int (field3);
  struct paramed_type *field4 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->date = fetch_ds_type_bare_int (field4);
  struct paramed_type *field5 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->duration = fetch_ds_type_bare_int (field5);
  struct paramed_type *field6 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->size = fetch_ds_type_bare_int (field6);
  struct paramed_type *field7 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x900f60dd, .id = "PhotoSize", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->thumb = fetch_ds_type_photo_size (field7);
  struct paramed_type *field8 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->dc_id = fetch_ds_type_bare_int (field8);
  struct paramed_type *field9 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->w = fetch_ds_type_bare_int (field9);
  struct paramed_type *field10 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->h = fetch_ds_type_bare_int (field10);
  return result;
}
struct tl_ds_video *fetch_ds_constructor_video_l27 (struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x1716b174 && T->type->name != 0xe8e94e8b)) { return 0; }
  struct tl_ds_video *  result = talloc0 (sizeof (*result));
  result->magic = 0x388fa391;
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xddf89345, .id = "Bare_Long", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->id = fetch_ds_type_bare_long (field1);
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xddf89345, .id = "Bare_Long", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->access_hash = fetch_ds_type_bare_long (field2);
  struct paramed_type *field3 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->user_id = fetch_ds_type_bare_int (field3);
  struct paramed_type *field4 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->date = fetch_ds_type_bare_int (field4);
  struct paramed_type *field5 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->caption = fetch_ds_type_bare_string (field5);
  struct paramed_type *field6 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->duration = fetch_ds_type_bare_int (field6);
  struct paramed_type *field7 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->mime_type = fetch_ds_type_bare_string (field7);
  struct paramed_type *field8 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->size = fetch_ds_type_bare_int (field8);
  struct paramed_type *field9 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x900f60dd, .id = "PhotoSize", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->thumb = fetch_ds_type_photo_size (field9);
  struct paramed_type *field10 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->dc_id = fetch_ds_type_bare_int (field10);
  struct paramed_type *field11 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->w = fetch_ds_type_bare_int (field11);
  struct paramed_type *field12 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->h = fetch_ds_type_bare_int (field12);
  return result;
}
struct tl_ds_wall_paper *fetch_ds_constructor_wall_paper (struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0xafa14973 && T->type->name != 0x505eb68c)) { return 0; }
  struct tl_ds_wall_paper *  result = talloc0 (sizeof (*result));
  result->magic = 0xccb03657;
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->id = fetch_ds_type_bare_int (field1);
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->title = fetch_ds_type_bare_string (field2);
  struct paramed_type *field3 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x1cb5c415, .id = "Vector", .params_num = 1, .params_types = 0},
    .params = (struct paramed_type *[]){
      &(struct paramed_type){
        .type = &(struct tl_type_descr) {.name = 0x900f60dd, .id = "PhotoSize", .params_num = 0, .params_types = 0},
        .params = 0,
      },
    }
  };
  result->sizes = (void *)fetch_ds_type_vector (field3);
  struct paramed_type *field4 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->color = fetch_ds_type_bare_int (field4);
  return result;
}
struct tl_ds_wall_paper *fetch_ds_constructor_wall_paper_solid (struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0xafa14973 && T->type->name != 0x505eb68c)) { return 0; }
  struct tl_ds_wall_paper *  result = talloc0 (sizeof (*result));
  result->magic = 0x63117f24;
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->id = fetch_ds_type_bare_int (field1);
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->title = fetch_ds_type_bare_string (field2);
  struct paramed_type *field3 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->bg_color = fetch_ds_type_bare_int (field3);
  struct paramed_type *field4 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->color = fetch_ds_type_bare_int (field4);
  return result;
}
struct tl_ds_web_page *fetch_ds_constructor_web_page_empty (struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x8d8c0d41 && T->type->name != 0x7273f2be)) { return 0; }
  struct tl_ds_web_page *  result = talloc0 (sizeof (*result));
  result->magic = 0xeb1477e8;
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xddf89345, .id = "Bare_Long", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->id = fetch_ds_type_bare_long (field1);
  return result;
}
struct tl_ds_web_page *fetch_ds_constructor_web_page_pending (struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x8d8c0d41 && T->type->name != 0x7273f2be)) { return 0; }
  struct tl_ds_web_page *  result = talloc0 (sizeof (*result));
  result->magic = 0xc586da1c;
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xddf89345, .id = "Bare_Long", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->id = fetch_ds_type_bare_long (field1);
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->date = fetch_ds_type_bare_int (field2);
  return result;
}
struct tl_ds_web_page *fetch_ds_constructor_web_page (struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x8d8c0d41 && T->type->name != 0x7273f2be)) { return 0; }
  struct tl_ds_web_page *  result = talloc0 (sizeof (*result));
  result->magic = 0xa31ea0b5;
  assert (in_remaining () >= 4);
  result->flags = talloc (4);  *result->flags = prefetch_int ();  struct paramed_type *var0 = INT2PTR (fetch_int ());
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xddf89345, .id = "Bare_Long", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->id = fetch_ds_type_bare_long (field2);
  struct paramed_type *field3 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->url = fetch_ds_type_bare_string (field3);
  struct paramed_type *field4 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->display_url = fetch_ds_type_bare_string (field4);
  if (PTR2INT (var0) & (1 << 0)) {
    struct paramed_type *field5 = 
    &(struct paramed_type){
      .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
      .params = 0,
    };
    result->type = fetch_ds_type_bare_string (field5);
  }
  if (PTR2INT (var0) & (1 << 1)) {
    struct paramed_type *field6 = 
    &(struct paramed_type){
      .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
      .params = 0,
    };
    result->site_name = fetch_ds_type_bare_string (field6);
  }
  if (PTR2INT (var0) & (1 << 2)) {
    struct paramed_type *field7 = 
    &(struct paramed_type){
      .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
      .params = 0,
    };
    result->title = fetch_ds_type_bare_string (field7);
  }
  if (PTR2INT (var0) & (1 << 3)) {
    struct paramed_type *field8 = 
    &(struct paramed_type){
      .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
      .params = 0,
    };
    result->description = fetch_ds_type_bare_string (field8);
  }
  if (PTR2INT (var0) & (1 << 4)) {
    struct paramed_type *field9 = 
    &(struct paramed_type){
      .type = &(struct tl_type_descr) {.name = 0xc207550a, .id = "Photo", .params_num = 0, .params_types = 0},
      .params = 0,
    };
    result->photo = fetch_ds_type_photo (field9);
  }
  if (PTR2INT (var0) & (1 << 5)) {
    struct paramed_type *field10 = 
    &(struct paramed_type){
      .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
      .params = 0,
    };
    result->embed_url = fetch_ds_type_bare_string (field10);
  }
  if (PTR2INT (var0) & (1 << 5)) {
    struct paramed_type *field11 = 
    &(struct paramed_type){
      .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
      .params = 0,
    };
    result->embed_type = fetch_ds_type_bare_string (field11);
  }
  if (PTR2INT (var0) & (1 << 6)) {
    struct paramed_type *field12 = 
    &(struct paramed_type){
      .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
      .params = 0,
    };
    result->embed_width = fetch_ds_type_bare_int (field12);
  }
  if (PTR2INT (var0) & (1 << 6)) {
    struct paramed_type *field13 = 
    &(struct paramed_type){
      .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
      .params = 0,
    };
    result->embed_height = fetch_ds_type_bare_int (field13);
  }
  if (PTR2INT (var0) & (1 << 7)) {
    struct paramed_type *field14 = 
    &(struct paramed_type){
      .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
      .params = 0,
    };
    result->duration = fetch_ds_type_bare_int (field14);
  }
  if (PTR2INT (var0) & (1 << 8)) {
    struct paramed_type *field15 = 
    &(struct paramed_type){
      .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
      .params = 0,
    };
    result->author = fetch_ds_type_bare_string (field15);
  }
  return result;
}
struct tl_ds_account_authorizations *fetch_ds_constructor_account_authorizations (struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x1250abde && T->type->name != 0xedaf5421)) { return 0; }
  struct tl_ds_account_authorizations *  result = talloc0 (sizeof (*result));
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x1cb5c415, .id = "Vector", .params_num = 1, .params_types = 0},
    .params = (struct paramed_type *[]){
      &(struct paramed_type){
        .type = &(struct tl_type_descr) {.name = 0x7bf2e6f6, .id = "Authorization", .params_num = 0, .params_types = 0},
        .params = 0,
      },
    }
  };
  result->authorizations = (void *)fetch_ds_type_vector (field1);
  return result;
}
struct tl_ds_account_password *fetch_ds_constructor_account_no_password (struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0xeac2a804 && T->type->name != 0x153d57fb)) { return 0; }
  struct tl_ds_account_password *  result = talloc0 (sizeof (*result));
  result->magic = 0x96dabc18;
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xf11ec860, .id = "Bare_Bytes", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->new_salt = fetch_ds_type_bare_bytes (field1);
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->email_unconfirmed_pattern = fetch_ds_type_bare_string (field2);
  return result;
}
struct tl_ds_account_password *fetch_ds_constructor_account_password (struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0xeac2a804 && T->type->name != 0x153d57fb)) { return 0; }
  struct tl_ds_account_password *  result = talloc0 (sizeof (*result));
  result->magic = 0x7c18141c;
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xf11ec860, .id = "Bare_Bytes", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->current_salt = fetch_ds_type_bare_bytes (field1);
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xf11ec860, .id = "Bare_Bytes", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->new_salt = fetch_ds_type_bare_bytes (field2);
  struct paramed_type *field3 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->hint = fetch_ds_type_bare_string (field3);
  struct paramed_type *field4 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x250be282, .id = "Bool", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->has_recovery = fetch_ds_type_bool (field4);
  struct paramed_type *field5 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->email_unconfirmed_pattern = fetch_ds_type_bare_string (field5);
  return result;
}
struct tl_ds_account_password_input_settings *fetch_ds_constructor_account_password_input_settings (struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0xbcfc532c && T->type->name != 0x4303acd3)) { return 0; }
  struct tl_ds_account_password_input_settings *  result = talloc0 (sizeof (*result));
  assert (in_remaining () >= 4);
  result->flags = talloc (4);  *result->flags = prefetch_int ();  struct paramed_type *var0 = INT2PTR (fetch_int ());
  if (PTR2INT (var0) & (1 << 0)) {
    struct paramed_type *field2 = 
    &(struct paramed_type){
      .type = &(struct tl_type_descr) {.name = 0xf11ec860, .id = "Bare_Bytes", .params_num = 0, .params_types = 0},
      .params = 0,
    };
    result->new_salt = fetch_ds_type_bare_bytes (field2);
  }
  if (PTR2INT (var0) & (1 << 0)) {
    struct paramed_type *field3 = 
    &(struct paramed_type){
      .type = &(struct tl_type_descr) {.name = 0xf11ec860, .id = "Bare_Bytes", .params_num = 0, .params_types = 0},
      .params = 0,
    };
    result->new_password_hash = fetch_ds_type_bare_bytes (field3);
  }
  if (PTR2INT (var0) & (1 << 0)) {
    struct paramed_type *field4 = 
    &(struct paramed_type){
      .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
      .params = 0,
    };
    result->hint = fetch_ds_type_bare_string (field4);
  }
  if (PTR2INT (var0) & (1 << 1)) {
    struct paramed_type *field5 = 
    &(struct paramed_type){
      .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
      .params = 0,
    };
    result->email = fetch_ds_type_bare_string (field5);
  }
  return result;
}
struct tl_ds_account_password_settings *fetch_ds_constructor_account_password_settings (struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0xb7b72ab3 && T->type->name != 0x4848d54c)) { return 0; }
  struct tl_ds_account_password_settings *  result = talloc0 (sizeof (*result));
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->email = fetch_ds_type_bare_string (field1);
  return result;
}
struct tl_ds_account_privacy_rules *fetch_ds_constructor_account_privacy_rules (struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x554abb6f && T->type->name != 0xaab54490)) { return 0; }
  struct tl_ds_account_privacy_rules *  result = talloc0 (sizeof (*result));
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x1cb5c415, .id = "Vector", .params_num = 1, .params_types = 0},
    .params = (struct paramed_type *[]){
      &(struct paramed_type){
        .type = &(struct tl_type_descr) {.name = 0xa8638aec, .id = "PrivacyRule", .params_num = 0, .params_types = 0},
        .params = 0,
      },
    }
  };
  result->rules = (void *)fetch_ds_type_vector (field1);
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x1cb5c415, .id = "Vector", .params_num = 1, .params_types = 0},
    .params = (struct paramed_type *[]){
      &(struct paramed_type){
        .type = &(struct tl_type_descr) {.name = 0x02e6c0c8, .id = "User", .params_num = 0, .params_types = 0},
        .params = 0,
      },
    }
  };
  result->users = (void *)fetch_ds_type_vector (field2);
  return result;
}
struct tl_ds_account_sent_change_phone_code *fetch_ds_constructor_account_sent_change_phone_code (struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0xa4f58c4c && T->type->name != 0x5b0a73b3)) { return 0; }
  struct tl_ds_account_sent_change_phone_code *  result = talloc0 (sizeof (*result));
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->phone_code_hash = fetch_ds_type_bare_string (field1);
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->send_call_timeout = fetch_ds_type_bare_int (field2);
  return result;
}
struct tl_ds_auth_authorization *fetch_ds_constructor_auth_authorization (struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0xff036af1 && T->type->name != 0x00fc950e)) { return 0; }
  struct tl_ds_auth_authorization *  result = talloc0 (sizeof (*result));
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x02e6c0c8, .id = "User", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->user = fetch_ds_type_user (field1);
  return result;
}
struct tl_ds_auth_checked_phone *fetch_ds_constructor_auth_checked_phone (struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x811ea28e && T->type->name != 0x7ee15d71)) { return 0; }
  struct tl_ds_auth_checked_phone *  result = talloc0 (sizeof (*result));
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x250be282, .id = "Bool", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->phone_registered = fetch_ds_type_bool (field1);
  return result;
}
struct tl_ds_auth_exported_authorization *fetch_ds_constructor_auth_exported_authorization (struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0xdf969c2d && T->type->name != 0x206963d2)) { return 0; }
  struct tl_ds_auth_exported_authorization *  result = talloc0 (sizeof (*result));
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->id = fetch_ds_type_bare_int (field1);
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xf11ec860, .id = "Bare_Bytes", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->bytes = fetch_ds_type_bare_bytes (field2);
  return result;
}
struct tl_ds_auth_password_recovery *fetch_ds_constructor_auth_password_recovery (struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x137948a5 && T->type->name != 0xec86b75a)) { return 0; }
  struct tl_ds_auth_password_recovery *  result = talloc0 (sizeof (*result));
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->email_pattern = fetch_ds_type_bare_string (field1);
  return result;
}
struct tl_ds_auth_sent_code *fetch_ds_constructor_auth_sent_code (struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x0cc8bc16 && T->type->name != 0xf33743e9)) { return 0; }
  struct tl_ds_auth_sent_code *  result = talloc0 (sizeof (*result));
  result->magic = 0xefed51d9;
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x250be282, .id = "Bool", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->phone_registered = fetch_ds_type_bool (field1);
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->phone_code_hash = fetch_ds_type_bare_string (field2);
  struct paramed_type *field3 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->send_call_timeout = fetch_ds_type_bare_int (field3);
  struct paramed_type *field4 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x250be282, .id = "Bool", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->is_password = fetch_ds_type_bool (field4);
  return result;
}
struct tl_ds_auth_sent_code *fetch_ds_constructor_auth_sent_app_code (struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x0cc8bc16 && T->type->name != 0xf33743e9)) { return 0; }
  struct tl_ds_auth_sent_code *  result = talloc0 (sizeof (*result));
  result->magic = 0xe325edcf;
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x250be282, .id = "Bool", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->phone_registered = fetch_ds_type_bool (field1);
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->phone_code_hash = fetch_ds_type_bare_string (field2);
  struct paramed_type *field3 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->send_call_timeout = fetch_ds_type_bare_int (field3);
  struct paramed_type *field4 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x250be282, .id = "Bool", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->is_password = fetch_ds_type_bool (field4);
  return result;
}
struct tl_ds_binlog_encr_key *fetch_ds_constructor_binlog_encr_key (struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x0377168f && T->type->name != 0xfc88e970)) { return 0; }
  struct tl_ds_binlog_encr_key *  result = talloc0 (sizeof (*result));
  int multiplicity1 = PTR2INT (
  INT2PTR (64)  );
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->key = talloc0 (multiplicity1 * sizeof (void *));
  {
    int i = 0;
    while (i < multiplicity1) {
      result->key[i ++] =fetch_ds_type_any (field1);
    }
  }
  return result;
}
struct tl_ds_binlog_update *fetch_ds_constructor_binlog_start (struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x66ff03a6 && T->type->name != 0x9900fc59)) { return 0; }
  struct tl_ds_binlog_update *  result = talloc0 (sizeof (*result));
  result->magic = 0x3b06de69;
  return result;
}
struct tl_ds_binlog_update *fetch_ds_constructor_binlog_dc_option (struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x66ff03a6 && T->type->name != 0x9900fc59)) { return 0; }
  struct tl_ds_binlog_update *  result = talloc0 (sizeof (*result));
  result->magic = 0xf96feb32;
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->dc = fetch_ds_type_bare_int (field1);
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->name = fetch_ds_type_bare_string (field2);
  struct paramed_type *field3 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->ip = fetch_ds_type_bare_string (field3);
  struct paramed_type *field4 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->port = fetch_ds_type_bare_int (field4);
  return result;
}
struct tl_ds_binlog_update *fetch_ds_constructor_binlog_dc_option_new (struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x66ff03a6 && T->type->name != 0x9900fc59)) { return 0; }
  struct tl_ds_binlog_update *  result = talloc0 (sizeof (*result));
  result->magic = 0x7c0d22d8;
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->flags = fetch_ds_type_bare_int (field1);
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->dc = fetch_ds_type_bare_int (field2);
  struct paramed_type *field3 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->name = fetch_ds_type_bare_string (field3);
  struct paramed_type *field4 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->ip = fetch_ds_type_bare_string (field4);
  struct paramed_type *field5 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->port = fetch_ds_type_bare_int (field5);
  return result;
}
struct tl_ds_binlog_update *fetch_ds_constructor_binlog_auth_key (struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x66ff03a6 && T->type->name != 0x9900fc59)) { return 0; }
  struct tl_ds_binlog_update *  result = talloc0 (sizeof (*result));
  result->magic = 0x71e8c156;
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->dc = fetch_ds_type_bare_int (field1);
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xfc88e970, .id = "Bare_binlog.EncrKey", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->key = fetch_ds_type_bare_binlog_encr_key (field2);
  return result;
}
struct tl_ds_binlog_update *fetch_ds_constructor_binlog_default_dc (struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x66ff03a6 && T->type->name != 0x9900fc59)) { return 0; }
  struct tl_ds_binlog_update *  result = talloc0 (sizeof (*result));
  result->magic = 0x9e83dbdc;
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->dc = fetch_ds_type_bare_int (field1);
  return result;
}
struct tl_ds_binlog_update *fetch_ds_constructor_binlog_dc_signed (struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x66ff03a6 && T->type->name != 0x9900fc59)) { return 0; }
  struct tl_ds_binlog_update *  result = talloc0 (sizeof (*result));
  result->magic = 0x26451bb5;
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->dc = fetch_ds_type_bare_int (field1);
  return result;
}
struct tl_ds_binlog_update *fetch_ds_constructor_binlog_our_id (struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x66ff03a6 && T->type->name != 0x9900fc59)) { return 0; }
  struct tl_ds_binlog_update *  result = talloc0 (sizeof (*result));
  result->magic = 0x68a870e8;
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->id = fetch_ds_type_bare_int (field1);
  return result;
}
struct tl_ds_binlog_update *fetch_ds_constructor_binlog_set_dh_params (struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x66ff03a6 && T->type->name != 0x9900fc59)) { return 0; }
  struct tl_ds_binlog_update *  result = talloc0 (sizeof (*result));
  result->magic = 0xeaeb7826;
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->root = fetch_ds_type_bare_int (field1);
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xfc88e970, .id = "Bare_binlog.EncrKey", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->prime = fetch_ds_type_bare_binlog_encr_key (field2);
  struct paramed_type *field3 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->version = fetch_ds_type_bare_int (field3);
  return result;
}
struct tl_ds_binlog_update *fetch_ds_constructor_binlog_set_pts (struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x66ff03a6 && T->type->name != 0x9900fc59)) { return 0; }
  struct tl_ds_binlog_update *  result = talloc0 (sizeof (*result));
  result->magic = 0x2ca8c939;
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->pts = fetch_ds_type_bare_int (field1);
  return result;
}
struct tl_ds_binlog_update *fetch_ds_constructor_binlog_set_qts (struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x66ff03a6 && T->type->name != 0x9900fc59)) { return 0; }
  struct tl_ds_binlog_update *  result = talloc0 (sizeof (*result));
  result->magic = 0xd95738ac;
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->qts = fetch_ds_type_bare_int (field1);
  return result;
}
struct tl_ds_binlog_update *fetch_ds_constructor_binlog_set_date (struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x66ff03a6 && T->type->name != 0x9900fc59)) { return 0; }
  struct tl_ds_binlog_update *  result = talloc0 (sizeof (*result));
  result->magic = 0x1d0f4b52;
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->date = fetch_ds_type_bare_int (field1);
  return result;
}
struct tl_ds_binlog_update *fetch_ds_constructor_binlog_set_seq (struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x66ff03a6 && T->type->name != 0x9900fc59)) { return 0; }
  struct tl_ds_binlog_update *  result = talloc0 (sizeof (*result));
  result->magic = 0x6eeb2989;
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->seq = fetch_ds_type_bare_int (field1);
  return result;
}
struct tl_ds_binlog_update *fetch_ds_constructor_binlog_encr_chat_delete (struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x66ff03a6 && T->type->name != 0x9900fc59)) { return 0; }
  struct tl_ds_binlog_update *  result = talloc0 (sizeof (*result));
  result->magic = 0xee1b38e8;
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->id = fetch_ds_type_bare_int (field1);
  return result;
}
struct tl_ds_binlog_update *fetch_ds_constructor_binlog_encr_chat_new (struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x66ff03a6 && T->type->name != 0x9900fc59)) { return 0; }
  struct tl_ds_binlog_update *  result = talloc0 (sizeof (*result));
  result->magic = 0x84977251;
  assert (in_remaining () >= 4);
  result->flags = talloc (4);  *result->flags = prefetch_int ();  struct paramed_type *var0 = INT2PTR (fetch_int ());
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->id = fetch_ds_type_bare_int (field2);
  if (PTR2INT (var0) & (1 << 17)) {
    struct paramed_type *field3 = 
    &(struct paramed_type){
      .type = &(struct tl_type_descr) {.name = 0xddf89345, .id = "Bare_Long", .params_num = 0, .params_types = 0},
      .params = 0,
    };
    result->access_hash = fetch_ds_type_bare_long (field3);
  }
  if (PTR2INT (var0) & (1 << 18)) {
    struct paramed_type *field4 = 
    &(struct paramed_type){
      .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
      .params = 0,
    };
    result->date = fetch_ds_type_bare_int (field4);
  }
  if (PTR2INT (var0) & (1 << 19)) {
    struct paramed_type *field5 = 
    &(struct paramed_type){
      .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
      .params = 0,
    };
    result->admin = fetch_ds_type_bare_int (field5);
  }
  if (PTR2INT (var0) & (1 << 20)) {
    struct paramed_type *field6 = 
    &(struct paramed_type){
      .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
      .params = 0,
    };
    result->user_id = fetch_ds_type_bare_int (field6);
  }
  if (PTR2INT (var0) & (1 << 21)) {
    struct paramed_type *field7 = 
    &(struct paramed_type){
      .type = &(struct tl_type_descr) {.name = 0xfc88e970, .id = "Bare_binlog.EncrKey", .params_num = 0, .params_types = 0},
      .params = 0,
    };
    result->key = fetch_ds_type_bare_binlog_encr_key (field7);
  }
  if (PTR2INT (var0) & (1 << 22)) {
    struct paramed_type *field8 = 
    &(struct paramed_type){
      .type = &(struct tl_type_descr) {.name = 0xfc88e970, .id = "Bare_binlog.EncrKey", .params_num = 0, .params_types = 0},
      .params = 0,
    };
    result->g_key = fetch_ds_type_bare_binlog_encr_key (field8);
  }
  if (PTR2INT (var0) & (1 << 23)) {
    struct paramed_type *field9 = 
    &(struct paramed_type){
      .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
      .params = 0,
    };
    result->state = fetch_ds_type_bare_int (field9);
  }
  if (PTR2INT (var0) & (1 << 24)) {
    struct paramed_type *field10 = 
    &(struct paramed_type){
      .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
      .params = 0,
    };
    result->ttl = fetch_ds_type_bare_int (field10);
  }
  if (PTR2INT (var0) & (1 << 25)) {
    struct paramed_type *field11 = 
    &(struct paramed_type){
      .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
      .params = 0,
    };
    result->layer = fetch_ds_type_bare_int (field11);
  }
  if (PTR2INT (var0) & (1 << 26)) {
    struct paramed_type *field12 = 
    &(struct paramed_type){
      .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
      .params = 0,
    };
    result->in_seq_no = fetch_ds_type_bare_int (field12);
  }
  if (PTR2INT (var0) & (1 << 26)) {
    struct paramed_type *field13 = 
    &(struct paramed_type){
      .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
      .params = 0,
    };
    result->last_in_seq_no = fetch_ds_type_bare_int (field13);
  }
  if (PTR2INT (var0) & (1 << 26)) {
    struct paramed_type *field14 = 
    &(struct paramed_type){
      .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
      .params = 0,
    };
    result->out_seq_no = fetch_ds_type_bare_int (field14);
  }
  if (PTR2INT (var0) & (1 << 27)) {
    struct paramed_type *field15 = 
    &(struct paramed_type){
      .type = &(struct tl_type_descr) {.name = 0xddf89345, .id = "Bare_Long", .params_num = 0, .params_types = 0},
      .params = 0,
    };
    result->key_fingerprint = fetch_ds_type_bare_long (field15);
  }
  return result;
}
struct tl_ds_binlog_update *fetch_ds_constructor_binlog_encr_chat_exchange_new (struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x66ff03a6 && T->type->name != 0x9900fc59)) { return 0; }
  struct tl_ds_binlog_update *  result = talloc0 (sizeof (*result));
  result->magic = 0x9d49488d;
  assert (in_remaining () >= 4);
  result->flags = talloc (4);  *result->flags = prefetch_int ();  struct paramed_type *var0 = INT2PTR (fetch_int ());
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->id = fetch_ds_type_bare_int (field2);
  if (PTR2INT (var0) & (1 << 17)) {
    struct paramed_type *field3 = 
    &(struct paramed_type){
      .type = &(struct tl_type_descr) {.name = 0xddf89345, .id = "Bare_Long", .params_num = 0, .params_types = 0},
      .params = 0,
    };
    result->exchange_id = fetch_ds_type_bare_long (field3);
  }
  if (PTR2INT (var0) & (1 << 18)) {
    struct paramed_type *field4 = 
    &(struct paramed_type){
      .type = &(struct tl_type_descr) {.name = 0xfc88e970, .id = "Bare_binlog.EncrKey", .params_num = 0, .params_types = 0},
      .params = 0,
    };
    result->key = fetch_ds_type_bare_binlog_encr_key (field4);
  }
  if (PTR2INT (var0) & (1 << 19)) {
    struct paramed_type *field5 = 
    &(struct paramed_type){
      .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
      .params = 0,
    };
    result->state = fetch_ds_type_bare_int (field5);
  }
  return result;
}
struct tl_ds_binlog_update *fetch_ds_constructor_binlog_user_delete (struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x66ff03a6 && T->type->name != 0x9900fc59)) { return 0; }
  struct tl_ds_binlog_update *  result = talloc0 (sizeof (*result));
  result->magic = 0xac55d447;
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->id = fetch_ds_type_bare_int (field1);
  return result;
}
struct tl_ds_binlog_update *fetch_ds_constructor_binlog_user_new (struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x66ff03a6 && T->type->name != 0x9900fc59)) { return 0; }
  struct tl_ds_binlog_update *  result = talloc0 (sizeof (*result));
  result->magic = 0x127cf2f9;
  assert (in_remaining () >= 4);
  result->flags = talloc (4);  *result->flags = prefetch_int ();  struct paramed_type *var0 = INT2PTR (fetch_int ());
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->id = fetch_ds_type_bare_int (field2);
  if (PTR2INT (var0) & (1 << 17)) {
    struct paramed_type *field3 = 
    &(struct paramed_type){
      .type = &(struct tl_type_descr) {.name = 0xddf89345, .id = "Bare_Long", .params_num = 0, .params_types = 0},
      .params = 0,
    };
    result->access_hash = fetch_ds_type_bare_long (field3);
  }
  if (PTR2INT (var0) & (1 << 18)) {
    struct paramed_type *field4 = 
    &(struct paramed_type){
      .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
      .params = 0,
    };
    result->first_name = fetch_ds_type_bare_string (field4);
  }
  if (PTR2INT (var0) & (1 << 18)) {
    struct paramed_type *field5 = 
    &(struct paramed_type){
      .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
      .params = 0,
    };
    result->last_name = fetch_ds_type_bare_string (field5);
  }
  if (PTR2INT (var0) & (1 << 19)) {
    struct paramed_type *field6 = 
    &(struct paramed_type){
      .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
      .params = 0,
    };
    result->phone = fetch_ds_type_bare_string (field6);
  }
  if (PTR2INT (var0) & (1 << 20)) {
    struct paramed_type *field7 = 
    &(struct paramed_type){
      .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
      .params = 0,
    };
    result->username = fetch_ds_type_bare_string (field7);
  }
  if (PTR2INT (var0) & (1 << 21)) {
    struct paramed_type *field8 = 
    &(struct paramed_type){
      .type = &(struct tl_type_descr) {.name = 0xc207550a, .id = "Photo", .params_num = 0, .params_types = 0},
      .params = 0,
    };
    result->photo = fetch_ds_type_photo (field8);
  }
  if (PTR2INT (var0) & (1 << 22)) {
    struct paramed_type *field9 = 
    &(struct paramed_type){
      .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
      .params = 0,
    };
    result->real_first_name = fetch_ds_type_bare_string (field9);
  }
  if (PTR2INT (var0) & (1 << 22)) {
    struct paramed_type *field10 = 
    &(struct paramed_type){
      .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
      .params = 0,
    };
    result->real_last_name = fetch_ds_type_bare_string (field10);
  }
  if (PTR2INT (var0) & (1 << 23)) {
    struct paramed_type *field11 = 
    &(struct paramed_type){
      .type = &(struct tl_type_descr) {.name = 0x9a486229, .id = "UserProfilePhoto", .params_num = 0, .params_types = 0},
      .params = 0,
    };
    result->user_photo = fetch_ds_type_user_profile_photo (field11);
  }
  if (PTR2INT (var0) & (1 << 24)) {
    struct paramed_type *field12 = 
    &(struct paramed_type){
      .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
      .params = 0,
    };
    result->last_read_in = fetch_ds_type_bare_int (field12);
  }
  if (PTR2INT (var0) & (1 << 25)) {
    struct paramed_type *field13 = 
    &(struct paramed_type){
      .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
      .params = 0,
    };
    result->last_read_out = fetch_ds_type_bare_int (field13);
  }
  if (PTR2INT (var0) & (1 << 26)) {
    struct paramed_type *field14 = 
    &(struct paramed_type){
      .type = &(struct tl_type_descr) {.name = 0xb2e16f93, .id = "BotInfo", .params_num = 0, .params_types = 0},
      .params = 0,
    };
    result->bot_info = fetch_ds_type_bot_info (field14);
  }
  return result;
}
struct tl_ds_binlog_update *fetch_ds_constructor_binlog_chat_new (struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x66ff03a6 && T->type->name != 0x9900fc59)) { return 0; }
  struct tl_ds_binlog_update *  result = talloc0 (sizeof (*result));
  result->magic = 0x0a10aa92;
  assert (in_remaining () >= 4);
  result->flags = talloc (4);  *result->flags = prefetch_int ();  struct paramed_type *var0 = INT2PTR (fetch_int ());
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->id = fetch_ds_type_bare_int (field2);
  if (PTR2INT (var0) & (1 << 17)) {
    struct paramed_type *field3 = 
    &(struct paramed_type){
      .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
      .params = 0,
    };
    result->title = fetch_ds_type_bare_string (field3);
  }
  if (PTR2INT (var0) & (1 << 18)) {
    struct paramed_type *field4 = 
    &(struct paramed_type){
      .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
      .params = 0,
    };
    result->user_num = fetch_ds_type_bare_int (field4);
  }
  if (PTR2INT (var0) & (1 << 19)) {
    struct paramed_type *field5 = 
    &(struct paramed_type){
      .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
      .params = 0,
    };
    result->date = fetch_ds_type_bare_int (field5);
  }
  if (PTR2INT (var0) & (1 << 20)) {
    struct paramed_type *field6 = 
    &(struct paramed_type){
      .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
      .params = 0,
    };
    result->version = fetch_ds_type_bare_int (field6);
  }
  if (PTR2INT (var0) & (1 << 20)) {
    struct paramed_type *field7 = 
    &(struct paramed_type){
      .type = &(struct tl_type_descr) {.name = 0x1cb5c415, .id = "Vector", .params_num = 1, .params_types = 0},
      .params = (struct paramed_type *[]){
        &(struct paramed_type){
          .type = &(struct tl_type_descr) {.name = 0xc8d7493e, .id = "ChatParticipant", .params_num = 0, .params_types = 0},
          .params = 0,
        },
      }
    };
    result->participants = (void *)fetch_ds_type_vector (field7);
  }
  if (PTR2INT (var0) & (1 << 21)) {
    struct paramed_type *field8 = 
    &(struct paramed_type){
      .type = &(struct tl_type_descr) {.name = 0x56922676, .id = "ChatPhoto", .params_num = 0, .params_types = 0},
      .params = 0,
    };
    result->chat_photo = fetch_ds_type_chat_photo (field8);
  }
  if (PTR2INT (var0) & (1 << 22)) {
    struct paramed_type *field9 = 
    &(struct paramed_type){
      .type = &(struct tl_type_descr) {.name = 0xc207550a, .id = "Photo", .params_num = 0, .params_types = 0},
      .params = 0,
    };
    result->photo = fetch_ds_type_photo (field9);
  }
  if (PTR2INT (var0) & (1 << 23)) {
    struct paramed_type *field10 = 
    &(struct paramed_type){
      .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
      .params = 0,
    };
    result->admin = fetch_ds_type_bare_int (field10);
  }
  if (PTR2INT (var0) & (1 << 24)) {
    struct paramed_type *field11 = 
    &(struct paramed_type){
      .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
      .params = 0,
    };
    result->last_read_in = fetch_ds_type_bare_int (field11);
  }
  if (PTR2INT (var0) & (1 << 25)) {
    struct paramed_type *field12 = 
    &(struct paramed_type){
      .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
      .params = 0,
    };
    result->last_read_out = fetch_ds_type_bare_int (field12);
  }
  return result;
}
struct tl_ds_binlog_update *fetch_ds_constructor_binlog_chat_add_participant (struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x66ff03a6 && T->type->name != 0x9900fc59)) { return 0; }
  struct tl_ds_binlog_update *  result = talloc0 (sizeof (*result));
  result->magic = 0x535475ea;
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->id = fetch_ds_type_bare_int (field1);
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->version = fetch_ds_type_bare_int (field2);
  struct paramed_type *field3 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->user_id = fetch_ds_type_bare_int (field3);
  struct paramed_type *field4 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->inviter_id = fetch_ds_type_bare_int (field4);
  struct paramed_type *field5 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->date = fetch_ds_type_bare_int (field5);
  return result;
}
struct tl_ds_binlog_update *fetch_ds_constructor_binlog_chat_del_participant (struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x66ff03a6 && T->type->name != 0x9900fc59)) { return 0; }
  struct tl_ds_binlog_update *  result = talloc0 (sizeof (*result));
  result->magic = 0x7dd1a1a2;
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->id = fetch_ds_type_bare_int (field1);
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->version = fetch_ds_type_bare_int (field2);
  struct paramed_type *field3 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->user_id = fetch_ds_type_bare_int (field3);
  return result;
}
struct tl_ds_binlog_update *fetch_ds_constructor_binlog_set_msg_id (struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x66ff03a6 && T->type->name != 0x9900fc59)) { return 0; }
  struct tl_ds_binlog_update *  result = talloc0 (sizeof (*result));
  result->magic = 0x3c873416;
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xddf89345, .id = "Bare_Long", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->old_id = fetch_ds_type_bare_long (field1);
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->new_id = fetch_ds_type_bare_int (field2);
  return result;
}
struct tl_ds_binlog_update *fetch_ds_constructor_binlog_message_delete (struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x66ff03a6 && T->type->name != 0x9900fc59)) { return 0; }
  struct tl_ds_binlog_update *  result = talloc0 (sizeof (*result));
  result->magic = 0x847e77b1;
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xddf89345, .id = "Bare_Long", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->lid = fetch_ds_type_bare_long (field1);
  return result;
}
struct tl_ds_binlog_update *fetch_ds_constructor_binlog_message_new (struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x66ff03a6 && T->type->name != 0x9900fc59)) { return 0; }
  struct tl_ds_binlog_update *  result = talloc0 (sizeof (*result));
  result->magic = 0x427cfcdb;
  assert (in_remaining () >= 4);
  result->flags = talloc (4);  *result->flags = prefetch_int ();  struct paramed_type *var0 = INT2PTR (fetch_int ());
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xddf89345, .id = "Bare_Long", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->lid = fetch_ds_type_bare_long (field2);
  if (PTR2INT (var0) & (1 << 17)) {
    struct paramed_type *field3 = 
    &(struct paramed_type){
      .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
      .params = 0,
    };
    result->from_id = fetch_ds_type_bare_int (field3);
  }
  if (PTR2INT (var0) & (1 << 17)) {
    struct paramed_type *field4 = 
    &(struct paramed_type){
      .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
      .params = 0,
    };
    result->to_type = fetch_ds_type_bare_int (field4);
  }
  if (PTR2INT (var0) & (1 << 17)) {
    struct paramed_type *field5 = 
    &(struct paramed_type){
      .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
      .params = 0,
    };
    result->to_id = fetch_ds_type_bare_int (field5);
  }
  if (PTR2INT (var0) & (1 << 18)) {
    struct paramed_type *field6 = 
    &(struct paramed_type){
      .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
      .params = 0,
    };
    result->fwd_from_id = fetch_ds_type_bare_int (field6);
  }
  if (PTR2INT (var0) & (1 << 18)) {
    struct paramed_type *field7 = 
    &(struct paramed_type){
      .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
      .params = 0,
    };
    result->fwd_date = fetch_ds_type_bare_int (field7);
  }
  if (PTR2INT (var0) & (1 << 19)) {
    struct paramed_type *field8 = 
    &(struct paramed_type){
      .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
      .params = 0,
    };
    result->date = fetch_ds_type_bare_int (field8);
  }
  if (PTR2INT (var0) & (1 << 20)) {
    struct paramed_type *field9 = 
    &(struct paramed_type){
      .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
      .params = 0,
    };
    result->message = fetch_ds_type_bare_string (field9);
  }
  if (PTR2INT (var0) & (1 << 21)) {
    struct paramed_type *field10 = 
    &(struct paramed_type){
      .type = &(struct tl_type_descr) {.name = 0x95f2471a, .id = "MessageMedia", .params_num = 0, .params_types = 0},
      .params = 0,
    };
    result->media = fetch_ds_type_message_media (field10);
  }
  if (PTR2INT (var0) & (1 << 22)) {
    struct paramed_type *field11 = 
    &(struct paramed_type){
      .type = &(struct tl_type_descr) {.name = 0x383415fa, .id = "MessageAction", .params_num = 0, .params_types = 0},
      .params = 0,
    };
    result->action = fetch_ds_type_message_action (field11);
  }
  if (PTR2INT (var0) & (1 << 23)) {
    struct paramed_type *field12 = 
    &(struct paramed_type){
      .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
      .params = 0,
    };
    result->reply_id = fetch_ds_type_bare_int (field12);
  }
  if (PTR2INT (var0) & (1 << 24)) {
    struct paramed_type *field13 = 
    &(struct paramed_type){
      .type = &(struct tl_type_descr) {.name = 0x612ca4a9, .id = "ReplyMarkup", .params_num = 0, .params_types = 0},
      .params = 0,
    };
    result->reply_markup = fetch_ds_type_reply_markup (field13);
  }
  return result;
}
struct tl_ds_binlog_update *fetch_ds_constructor_binlog_message_encr_new (struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x66ff03a6 && T->type->name != 0x9900fc59)) { return 0; }
  struct tl_ds_binlog_update *  result = talloc0 (sizeof (*result));
  result->magic = 0x6cf7cabc;
  assert (in_remaining () >= 4);
  result->flags = talloc (4);  *result->flags = prefetch_int ();  struct paramed_type *var0 = INT2PTR (fetch_int ());
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xddf89345, .id = "Bare_Long", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->lid = fetch_ds_type_bare_long (field2);
  if (PTR2INT (var0) & (1 << 17)) {
    struct paramed_type *field3 = 
    &(struct paramed_type){
      .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
      .params = 0,
    };
    result->from_id = fetch_ds_type_bare_int (field3);
  }
  if (PTR2INT (var0) & (1 << 17)) {
    struct paramed_type *field4 = 
    &(struct paramed_type){
      .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
      .params = 0,
    };
    result->to_type = fetch_ds_type_bare_int (field4);
  }
  if (PTR2INT (var0) & (1 << 17)) {
    struct paramed_type *field5 = 
    &(struct paramed_type){
      .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
      .params = 0,
    };
    result->to_id = fetch_ds_type_bare_int (field5);
  }
  if (PTR2INT (var0) & (1 << 19)) {
    struct paramed_type *field6 = 
    &(struct paramed_type){
      .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
      .params = 0,
    };
    result->date = fetch_ds_type_bare_int (field6);
  }
  if (PTR2INT (var0) & (1 << 20)) {
    struct paramed_type *field7 = 
    &(struct paramed_type){
      .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
      .params = 0,
    };
    result->message = fetch_ds_type_bare_string (field7);
  }
  if (PTR2INT (var0) & (1 << 21)) {
    struct paramed_type *field8 = 
    &(struct paramed_type){
      .type = &(struct tl_type_descr) {.name = 0x34e0d674, .id = "DecryptedMessageMedia", .params_num = 0, .params_types = 0},
      .params = 0,
    };
    result->encr_media = fetch_ds_type_decrypted_message_media (field8);
  }
  if (PTR2INT (var0) & (1 << 22)) {
    struct paramed_type *field9 = 
    &(struct paramed_type){
      .type = &(struct tl_type_descr) {.name = 0x4e0eefde, .id = "DecryptedMessageAction", .params_num = 0, .params_types = 0},
      .params = 0,
    };
    result->encr_action = fetch_ds_type_decrypted_message_action (field9);
  }
  if (PTR2INT (var0) & (1 << 23)) {
    struct paramed_type *field10 = 
    &(struct paramed_type){
      .type = &(struct tl_type_descr) {.name = 0x886fd032, .id = "EncryptedFile", .params_num = 0, .params_types = 0},
      .params = 0,
    };
    result->file = fetch_ds_type_encrypted_file (field10);
  }
  return result;
}
struct tl_ds_binlog_update *fetch_ds_constructor_binlog_msg_update (struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x66ff03a6 && T->type->name != 0x9900fc59)) { return 0; }
  struct tl_ds_binlog_update *  result = talloc0 (sizeof (*result));
  result->magic = 0x6dd4d85f;
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xddf89345, .id = "Bare_Long", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->lid = fetch_ds_type_bare_long (field1);
  return result;
}
struct tl_ds_binlog_update *fetch_ds_constructor_binlog_reset_authorization (struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x66ff03a6 && T->type->name != 0x9900fc59)) { return 0; }
  struct tl_ds_binlog_update *  result = talloc0 (sizeof (*result));
  result->magic = 0x83327955;
  return result;
}
struct tl_ds_contacts_blocked *fetch_ds_constructor_contacts_blocked (struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x8c1b8fb4 && T->type->name != 0x73e4704b)) { return 0; }
  struct tl_ds_contacts_blocked *  result = talloc0 (sizeof (*result));
  result->magic = 0x1c138d15;
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x1cb5c415, .id = "Vector", .params_num = 1, .params_types = 0},
    .params = (struct paramed_type *[]){
      &(struct paramed_type){
        .type = &(struct tl_type_descr) {.name = 0x561bc879, .id = "ContactBlocked", .params_num = 0, .params_types = 0},
        .params = 0,
      },
    }
  };
  result->blocked = (void *)fetch_ds_type_vector (field1);
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x1cb5c415, .id = "Vector", .params_num = 1, .params_types = 0},
    .params = (struct paramed_type *[]){
      &(struct paramed_type){
        .type = &(struct tl_type_descr) {.name = 0x02e6c0c8, .id = "User", .params_num = 0, .params_types = 0},
        .params = 0,
      },
    }
  };
  result->users = (void *)fetch_ds_type_vector (field2);
  return result;
}
struct tl_ds_contacts_blocked *fetch_ds_constructor_contacts_blocked_slice (struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x8c1b8fb4 && T->type->name != 0x73e4704b)) { return 0; }
  struct tl_ds_contacts_blocked *  result = talloc0 (sizeof (*result));
  result->magic = 0x900802a1;
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->count = fetch_ds_type_bare_int (field1);
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x1cb5c415, .id = "Vector", .params_num = 1, .params_types = 0},
    .params = (struct paramed_type *[]){
      &(struct paramed_type){
        .type = &(struct tl_type_descr) {.name = 0x561bc879, .id = "ContactBlocked", .params_num = 0, .params_types = 0},
        .params = 0,
      },
    }
  };
  result->blocked = (void *)fetch_ds_type_vector (field2);
  struct paramed_type *field3 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x1cb5c415, .id = "Vector", .params_num = 1, .params_types = 0},
    .params = (struct paramed_type *[]){
      &(struct paramed_type){
        .type = &(struct tl_type_descr) {.name = 0x02e6c0c8, .id = "User", .params_num = 0, .params_types = 0},
        .params = 0,
      },
    }
  };
  result->users = (void *)fetch_ds_type_vector (field3);
  return result;
}
struct tl_ds_contacts_contacts *fetch_ds_constructor_contacts_contacts_not_modified (struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0xd8c02560 && T->type->name != 0x273fda9f)) { return 0; }
  struct tl_ds_contacts_contacts *  result = talloc0 (sizeof (*result));
  result->magic = 0xb74ba9d2;
  return result;
}
struct tl_ds_contacts_contacts *fetch_ds_constructor_contacts_contacts (struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0xd8c02560 && T->type->name != 0x273fda9f)) { return 0; }
  struct tl_ds_contacts_contacts *  result = talloc0 (sizeof (*result));
  result->magic = 0x6f8b8cb2;
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x1cb5c415, .id = "Vector", .params_num = 1, .params_types = 0},
    .params = (struct paramed_type *[]){
      &(struct paramed_type){
        .type = &(struct tl_type_descr) {.name = 0xf911c994, .id = "Contact", .params_num = 0, .params_types = 0},
        .params = 0,
      },
    }
  };
  result->contacts = (void *)fetch_ds_type_vector (field1);
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x1cb5c415, .id = "Vector", .params_num = 1, .params_types = 0},
    .params = (struct paramed_type *[]){
      &(struct paramed_type){
        .type = &(struct tl_type_descr) {.name = 0x02e6c0c8, .id = "User", .params_num = 0, .params_types = 0},
        .params = 0,
      },
    }
  };
  result->users = (void *)fetch_ds_type_vector (field2);
  return result;
}
struct tl_ds_contacts_found *fetch_ds_constructor_contacts_found (struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x0566000e && T->type->name != 0xfa99fff1)) { return 0; }
  struct tl_ds_contacts_found *  result = talloc0 (sizeof (*result));
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x1cb5c415, .id = "Vector", .params_num = 1, .params_types = 0},
    .params = (struct paramed_type *[]){
      &(struct paramed_type){
        .type = &(struct tl_type_descr) {.name = 0xea879f95, .id = "ContactFound", .params_num = 0, .params_types = 0},
        .params = 0,
      },
    }
  };
  result->results = (void *)fetch_ds_type_vector (field1);
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x1cb5c415, .id = "Vector", .params_num = 1, .params_types = 0},
    .params = (struct paramed_type *[]){
      &(struct paramed_type){
        .type = &(struct tl_type_descr) {.name = 0x02e6c0c8, .id = "User", .params_num = 0, .params_types = 0},
        .params = 0,
      },
    }
  };
  result->users = (void *)fetch_ds_type_vector (field2);
  return result;
}
struct tl_ds_contacts_imported_contacts *fetch_ds_constructor_contacts_imported_contacts (struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0xad524315 && T->type->name != 0x52adbcea)) { return 0; }
  struct tl_ds_contacts_imported_contacts *  result = talloc0 (sizeof (*result));
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x1cb5c415, .id = "Vector", .params_num = 1, .params_types = 0},
    .params = (struct paramed_type *[]){
      &(struct paramed_type){
        .type = &(struct tl_type_descr) {.name = 0xd0028438, .id = "ImportedContact", .params_num = 0, .params_types = 0},
        .params = 0,
      },
    }
  };
  result->imported = (void *)fetch_ds_type_vector (field1);
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x1cb5c415, .id = "Vector", .params_num = 1, .params_types = 0},
    .params = (struct paramed_type *[]){
      &(struct paramed_type){
        .type = &(struct tl_type_descr) {.name = 0xddf89345, .id = "Bare_Long", .params_num = 0, .params_types = 0},
        .params = 0,
      },
    }
  };
  result->retry_contacts = (void *)fetch_ds_type_vector (field2);
  struct paramed_type *field3 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x1cb5c415, .id = "Vector", .params_num = 1, .params_types = 0},
    .params = (struct paramed_type *[]){
      &(struct paramed_type){
        .type = &(struct tl_type_descr) {.name = 0x02e6c0c8, .id = "User", .params_num = 0, .params_types = 0},
        .params = 0,
      },
    }
  };
  result->users = (void *)fetch_ds_type_vector (field3);
  return result;
}
struct tl_ds_contacts_link *fetch_ds_constructor_contacts_link (struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x3ace484c && T->type->name != 0xc531b7b3)) { return 0; }
  struct tl_ds_contacts_link *  result = talloc0 (sizeof (*result));
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x522fbc63, .id = "ContactLink", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->my_link = fetch_ds_type_contact_link (field1);
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x522fbc63, .id = "ContactLink", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->foreign_link = fetch_ds_type_contact_link (field2);
  struct paramed_type *field3 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x02e6c0c8, .id = "User", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->user = fetch_ds_type_user (field3);
  return result;
}
struct tl_ds_contacts_suggested *fetch_ds_constructor_contacts_suggested (struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x5649dcc5 && T->type->name != 0xa9b6233a)) { return 0; }
  struct tl_ds_contacts_suggested *  result = talloc0 (sizeof (*result));
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x1cb5c415, .id = "Vector", .params_num = 1, .params_types = 0},
    .params = (struct paramed_type *[]){
      &(struct paramed_type){
        .type = &(struct tl_type_descr) {.name = 0x3de191a1, .id = "ContactSuggested", .params_num = 0, .params_types = 0},
        .params = 0,
      },
    }
  };
  result->results = (void *)fetch_ds_type_vector (field1);
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x1cb5c415, .id = "Vector", .params_num = 1, .params_types = 0},
    .params = (struct paramed_type *[]){
      &(struct paramed_type){
        .type = &(struct tl_type_descr) {.name = 0x02e6c0c8, .id = "User", .params_num = 0, .params_types = 0},
        .params = 0,
      },
    }
  };
  result->users = (void *)fetch_ds_type_vector (field2);
  return result;
}
struct tl_ds_geochats_located *fetch_ds_constructor_geochats_located (struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x48feb267 && T->type->name != 0xb7014d98)) { return 0; }
  struct tl_ds_geochats_located *  result = talloc0 (sizeof (*result));
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x1cb5c415, .id = "Vector", .params_num = 1, .params_types = 0},
    .params = (struct paramed_type *[]){
      &(struct paramed_type){
        .type = &(struct tl_type_descr) {.name = 0x3631cf4c, .id = "ChatLocated", .params_num = 0, .params_types = 0},
        .params = 0,
      },
    }
  };
  result->results = (void *)fetch_ds_type_vector (field1);
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x1cb5c415, .id = "Vector", .params_num = 1, .params_types = 0},
    .params = (struct paramed_type *[]){
      &(struct paramed_type){
        .type = &(struct tl_type_descr) {.name = 0xf67b4034, .id = "GeoChatMessage", .params_num = 0, .params_types = 0},
        .params = 0,
      },
    }
  };
  result->messages = (void *)fetch_ds_type_vector (field2);
  struct paramed_type *field3 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x1cb5c415, .id = "Vector", .params_num = 1, .params_types = 0},
    .params = (struct paramed_type *[]){
      &(struct paramed_type){
        .type = &(struct tl_type_descr) {.name = 0x7bd865dc, .id = "Chat", .params_num = 0, .params_types = 0},
        .params = 0,
      },
    }
  };
  result->chats = (void *)fetch_ds_type_vector (field3);
  struct paramed_type *field4 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x1cb5c415, .id = "Vector", .params_num = 1, .params_types = 0},
    .params = (struct paramed_type *[]){
      &(struct paramed_type){
        .type = &(struct tl_type_descr) {.name = 0x02e6c0c8, .id = "User", .params_num = 0, .params_types = 0},
        .params = 0,
      },
    }
  };
  result->users = (void *)fetch_ds_type_vector (field4);
  return result;
}
struct tl_ds_geochats_messages *fetch_ds_constructor_geochats_messages (struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x6d0a0e59 && T->type->name != 0x92f5f1a6)) { return 0; }
  struct tl_ds_geochats_messages *  result = talloc0 (sizeof (*result));
  result->magic = 0xd1526db1;
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x1cb5c415, .id = "Vector", .params_num = 1, .params_types = 0},
    .params = (struct paramed_type *[]){
      &(struct paramed_type){
        .type = &(struct tl_type_descr) {.name = 0xf67b4034, .id = "GeoChatMessage", .params_num = 0, .params_types = 0},
        .params = 0,
      },
    }
  };
  result->messages = (void *)fetch_ds_type_vector (field1);
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x1cb5c415, .id = "Vector", .params_num = 1, .params_types = 0},
    .params = (struct paramed_type *[]){
      &(struct paramed_type){
        .type = &(struct tl_type_descr) {.name = 0x7bd865dc, .id = "Chat", .params_num = 0, .params_types = 0},
        .params = 0,
      },
    }
  };
  result->chats = (void *)fetch_ds_type_vector (field2);
  struct paramed_type *field3 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x1cb5c415, .id = "Vector", .params_num = 1, .params_types = 0},
    .params = (struct paramed_type *[]){
      &(struct paramed_type){
        .type = &(struct tl_type_descr) {.name = 0x02e6c0c8, .id = "User", .params_num = 0, .params_types = 0},
        .params = 0,
      },
    }
  };
  result->users = (void *)fetch_ds_type_vector (field3);
  return result;
}
struct tl_ds_geochats_messages *fetch_ds_constructor_geochats_messages_slice (struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x6d0a0e59 && T->type->name != 0x92f5f1a6)) { return 0; }
  struct tl_ds_geochats_messages *  result = talloc0 (sizeof (*result));
  result->magic = 0xbc5863e8;
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->count = fetch_ds_type_bare_int (field1);
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x1cb5c415, .id = "Vector", .params_num = 1, .params_types = 0},
    .params = (struct paramed_type *[]){
      &(struct paramed_type){
        .type = &(struct tl_type_descr) {.name = 0xf67b4034, .id = "GeoChatMessage", .params_num = 0, .params_types = 0},
        .params = 0,
      },
    }
  };
  result->messages = (void *)fetch_ds_type_vector (field2);
  struct paramed_type *field3 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x1cb5c415, .id = "Vector", .params_num = 1, .params_types = 0},
    .params = (struct paramed_type *[]){
      &(struct paramed_type){
        .type = &(struct tl_type_descr) {.name = 0x7bd865dc, .id = "Chat", .params_num = 0, .params_types = 0},
        .params = 0,
      },
    }
  };
  result->chats = (void *)fetch_ds_type_vector (field3);
  struct paramed_type *field4 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x1cb5c415, .id = "Vector", .params_num = 1, .params_types = 0},
    .params = (struct paramed_type *[]){
      &(struct paramed_type){
        .type = &(struct tl_type_descr) {.name = 0x02e6c0c8, .id = "User", .params_num = 0, .params_types = 0},
        .params = 0,
      },
    }
  };
  result->users = (void *)fetch_ds_type_vector (field4);
  return result;
}
struct tl_ds_geochats_stated_message *fetch_ds_constructor_geochats_stated_message (struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x17b1578b && T->type->name != 0xe84ea874)) { return 0; }
  struct tl_ds_geochats_stated_message *  result = talloc0 (sizeof (*result));
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xf67b4034, .id = "GeoChatMessage", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->message = fetch_ds_type_geo_chat_message (field1);
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x1cb5c415, .id = "Vector", .params_num = 1, .params_types = 0},
    .params = (struct paramed_type *[]){
      &(struct paramed_type){
        .type = &(struct tl_type_descr) {.name = 0x7bd865dc, .id = "Chat", .params_num = 0, .params_types = 0},
        .params = 0,
      },
    }
  };
  result->chats = (void *)fetch_ds_type_vector (field2);
  struct paramed_type *field3 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x1cb5c415, .id = "Vector", .params_num = 1, .params_types = 0},
    .params = (struct paramed_type *[]){
      &(struct paramed_type){
        .type = &(struct tl_type_descr) {.name = 0x02e6c0c8, .id = "User", .params_num = 0, .params_types = 0},
        .params = 0,
      },
    }
  };
  result->users = (void *)fetch_ds_type_vector (field3);
  struct paramed_type *field4 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->seq = fetch_ds_type_bare_int (field4);
  return result;
}
struct tl_ds_help_app_update *fetch_ds_constructor_help_app_update (struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x4ddd9627 && T->type->name != 0xb22269d8)) { return 0; }
  struct tl_ds_help_app_update *  result = talloc0 (sizeof (*result));
  result->magic = 0x8987f311;
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->id = fetch_ds_type_bare_int (field1);
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x250be282, .id = "Bool", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->critical = fetch_ds_type_bool (field2);
  struct paramed_type *field3 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->url = fetch_ds_type_bare_string (field3);
  struct paramed_type *field4 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->text = fetch_ds_type_bare_string (field4);
  return result;
}
struct tl_ds_help_app_update *fetch_ds_constructor_help_no_app_update (struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x4ddd9627 && T->type->name != 0xb22269d8)) { return 0; }
  struct tl_ds_help_app_update *  result = talloc0 (sizeof (*result));
  result->magic = 0xc45a6536;
  return result;
}
struct tl_ds_help_invite_text *fetch_ds_constructor_help_invite_text (struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x18cb9f78 && T->type->name != 0xe7346087)) { return 0; }
  struct tl_ds_help_invite_text *  result = talloc0 (sizeof (*result));
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->message = fetch_ds_type_bare_string (field1);
  return result;
}
struct tl_ds_help_support *fetch_ds_constructor_help_support (struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x17c6b5f6 && T->type->name != 0xe8394a09)) { return 0; }
  struct tl_ds_help_support *  result = talloc0 (sizeof (*result));
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->phone_number = fetch_ds_type_bare_string (field1);
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x02e6c0c8, .id = "User", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->user = fetch_ds_type_user (field2);
  return result;
}
struct tl_ds_messages_affected_history *fetch_ds_constructor_messages_affected_history (struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0xb45c69d1 && T->type->name != 0x4ba3962e)) { return 0; }
  struct tl_ds_messages_affected_history *  result = talloc0 (sizeof (*result));
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->pts = fetch_ds_type_bare_int (field1);
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->pts_count = fetch_ds_type_bare_int (field2);
  struct paramed_type *field3 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->offset = fetch_ds_type_bare_int (field3);
  return result;
}
struct tl_ds_messages_affected_messages *fetch_ds_constructor_messages_affected_messages (struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x84d19185 && T->type->name != 0x7b2e6e7a)) { return 0; }
  struct tl_ds_messages_affected_messages *  result = talloc0 (sizeof (*result));
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->pts = fetch_ds_type_bare_int (field1);
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->pts_count = fetch_ds_type_bare_int (field2);
  return result;
}
struct tl_ds_messages_all_stickers *fetch_ds_constructor_messages_all_stickers_not_modified (struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0xb485502f && T->type->name != 0x4b7aafd0)) { return 0; }
  struct tl_ds_messages_all_stickers *  result = talloc0 (sizeof (*result));
  result->magic = 0xe86602c3;
  return result;
}
struct tl_ds_messages_all_stickers *fetch_ds_constructor_messages_all_stickers (struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0xb485502f && T->type->name != 0x4b7aafd0)) { return 0; }
  struct tl_ds_messages_all_stickers *  result = talloc0 (sizeof (*result));
  result->magic = 0x5ce352ec;
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->hash = fetch_ds_type_bare_string (field1);
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x1cb5c415, .id = "Vector", .params_num = 1, .params_types = 0},
    .params = (struct paramed_type *[]){
      &(struct paramed_type){
        .type = &(struct tl_type_descr) {.name = 0x12b299d4, .id = "StickerPack", .params_num = 0, .params_types = 0},
        .params = 0,
      },
    }
  };
  result->packs = (void *)fetch_ds_type_vector (field2);
  struct paramed_type *field3 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x1cb5c415, .id = "Vector", .params_num = 1, .params_types = 0},
    .params = (struct paramed_type *[]){
      &(struct paramed_type){
        .type = &(struct tl_type_descr) {.name = 0xa7a43b17, .id = "StickerSet", .params_num = 0, .params_types = 0},
        .params = 0,
      },
    }
  };
  result->sets = (void *)fetch_ds_type_vector (field3);
  struct paramed_type *field4 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x1cb5c415, .id = "Vector", .params_num = 1, .params_types = 0},
    .params = (struct paramed_type *[]){
      &(struct paramed_type){
        .type = &(struct tl_type_descr) {.name = 0x51a73418, .id = "Document", .params_num = 0, .params_types = 0},
        .params = 0,
      },
    }
  };
  result->documents = (void *)fetch_ds_type_vector (field4);
  return result;
}
struct tl_ds_messages_chat_full *fetch_ds_constructor_messages_chat_full (struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0xe5d7d19c && T->type->name != 0x1a282e63)) { return 0; }
  struct tl_ds_messages_chat_full *  result = talloc0 (sizeof (*result));
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x2e02a614, .id = "ChatFull", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->full_chat = fetch_ds_type_chat_full (field1);
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x1cb5c415, .id = "Vector", .params_num = 1, .params_types = 0},
    .params = (struct paramed_type *[]){
      &(struct paramed_type){
        .type = &(struct tl_type_descr) {.name = 0x7bd865dc, .id = "Chat", .params_num = 0, .params_types = 0},
        .params = 0,
      },
    }
  };
  result->chats = (void *)fetch_ds_type_vector (field2);
  struct paramed_type *field3 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x1cb5c415, .id = "Vector", .params_num = 1, .params_types = 0},
    .params = (struct paramed_type *[]){
      &(struct paramed_type){
        .type = &(struct tl_type_descr) {.name = 0x02e6c0c8, .id = "User", .params_num = 0, .params_types = 0},
        .params = 0,
      },
    }
  };
  result->users = (void *)fetch_ds_type_vector (field3);
  return result;
}
struct tl_ds_messages_chats *fetch_ds_constructor_messages_chats (struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x64ff9fd5 && T->type->name != 0x9b00602a)) { return 0; }
  struct tl_ds_messages_chats *  result = talloc0 (sizeof (*result));
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x1cb5c415, .id = "Vector", .params_num = 1, .params_types = 0},
    .params = (struct paramed_type *[]){
      &(struct paramed_type){
        .type = &(struct tl_type_descr) {.name = 0x7bd865dc, .id = "Chat", .params_num = 0, .params_types = 0},
        .params = 0,
      },
    }
  };
  result->chats = (void *)fetch_ds_type_vector (field1);
  return result;
}
struct tl_ds_messages_dh_config *fetch_ds_constructor_messages_dh_config_not_modified (struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0xecc058e8 && T->type->name != 0x133fa717)) { return 0; }
  struct tl_ds_messages_dh_config *  result = talloc0 (sizeof (*result));
  result->magic = 0xc0e24635;
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xf11ec860, .id = "Bare_Bytes", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->random = fetch_ds_type_bare_bytes (field1);
  return result;
}
struct tl_ds_messages_dh_config *fetch_ds_constructor_messages_dh_config (struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0xecc058e8 && T->type->name != 0x133fa717)) { return 0; }
  struct tl_ds_messages_dh_config *  result = talloc0 (sizeof (*result));
  result->magic = 0x2c221edd;
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->g = fetch_ds_type_bare_int (field1);
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xf11ec860, .id = "Bare_Bytes", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->p = fetch_ds_type_bare_bytes (field2);
  struct paramed_type *field3 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->version = fetch_ds_type_bare_int (field3);
  struct paramed_type *field4 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xf11ec860, .id = "Bare_Bytes", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->random = fetch_ds_type_bare_bytes (field4);
  return result;
}
struct tl_ds_messages_dialogs *fetch_ds_constructor_messages_dialogs (struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x645af8b3 && T->type->name != 0x9ba5074c)) { return 0; }
  struct tl_ds_messages_dialogs *  result = talloc0 (sizeof (*result));
  result->magic = 0x15ba6c40;
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x1cb5c415, .id = "Vector", .params_num = 1, .params_types = 0},
    .params = (struct paramed_type *[]){
      &(struct paramed_type){
        .type = &(struct tl_type_descr) {.name = 0xc1dd804a, .id = "Dialog", .params_num = 0, .params_types = 0},
        .params = 0,
      },
    }
  };
  result->dialogs = (void *)fetch_ds_type_vector (field1);
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x1cb5c415, .id = "Vector", .params_num = 1, .params_types = 0},
    .params = (struct paramed_type *[]){
      &(struct paramed_type){
        .type = &(struct tl_type_descr) {.name = 0x5d652a7f, .id = "Message", .params_num = 0, .params_types = 0},
        .params = 0,
      },
    }
  };
  result->messages = (void *)fetch_ds_type_vector (field2);
  struct paramed_type *field3 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x1cb5c415, .id = "Vector", .params_num = 1, .params_types = 0},
    .params = (struct paramed_type *[]){
      &(struct paramed_type){
        .type = &(struct tl_type_descr) {.name = 0x7bd865dc, .id = "Chat", .params_num = 0, .params_types = 0},
        .params = 0,
      },
    }
  };
  result->chats = (void *)fetch_ds_type_vector (field3);
  struct paramed_type *field4 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x1cb5c415, .id = "Vector", .params_num = 1, .params_types = 0},
    .params = (struct paramed_type *[]){
      &(struct paramed_type){
        .type = &(struct tl_type_descr) {.name = 0x02e6c0c8, .id = "User", .params_num = 0, .params_types = 0},
        .params = 0,
      },
    }
  };
  result->users = (void *)fetch_ds_type_vector (field4);
  return result;
}
struct tl_ds_messages_dialogs *fetch_ds_constructor_messages_dialogs_slice (struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x645af8b3 && T->type->name != 0x9ba5074c)) { return 0; }
  struct tl_ds_messages_dialogs *  result = talloc0 (sizeof (*result));
  result->magic = 0x71e094f3;
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->count = fetch_ds_type_bare_int (field1);
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x1cb5c415, .id = "Vector", .params_num = 1, .params_types = 0},
    .params = (struct paramed_type *[]){
      &(struct paramed_type){
        .type = &(struct tl_type_descr) {.name = 0xc1dd804a, .id = "Dialog", .params_num = 0, .params_types = 0},
        .params = 0,
      },
    }
  };
  result->dialogs = (void *)fetch_ds_type_vector (field2);
  struct paramed_type *field3 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x1cb5c415, .id = "Vector", .params_num = 1, .params_types = 0},
    .params = (struct paramed_type *[]){
      &(struct paramed_type){
        .type = &(struct tl_type_descr) {.name = 0x5d652a7f, .id = "Message", .params_num = 0, .params_types = 0},
        .params = 0,
      },
    }
  };
  result->messages = (void *)fetch_ds_type_vector (field3);
  struct paramed_type *field4 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x1cb5c415, .id = "Vector", .params_num = 1, .params_types = 0},
    .params = (struct paramed_type *[]){
      &(struct paramed_type){
        .type = &(struct tl_type_descr) {.name = 0x7bd865dc, .id = "Chat", .params_num = 0, .params_types = 0},
        .params = 0,
      },
    }
  };
  result->chats = (void *)fetch_ds_type_vector (field4);
  struct paramed_type *field5 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x1cb5c415, .id = "Vector", .params_num = 1, .params_types = 0},
    .params = (struct paramed_type *[]){
      &(struct paramed_type){
        .type = &(struct tl_type_descr) {.name = 0x02e6c0c8, .id = "User", .params_num = 0, .params_types = 0},
        .params = 0,
      },
    }
  };
  result->users = (void *)fetch_ds_type_vector (field5);
  return result;
}
struct tl_ds_messages_message *fetch_ds_constructor_messages_message_empty (struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x3f4e0648 && T->type->name != 0xc0b1f9b7)) { return 0; }
  struct tl_ds_messages_message *  result = talloc0 (sizeof (*result));
  return result;
}
struct tl_ds_messages_messages *fetch_ds_constructor_messages_messages (struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x8735e464 && T->type->name != 0x78ca1b9b)) { return 0; }
  struct tl_ds_messages_messages *  result = talloc0 (sizeof (*result));
  result->magic = 0x8c718e87;
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x1cb5c415, .id = "Vector", .params_num = 1, .params_types = 0},
    .params = (struct paramed_type *[]){
      &(struct paramed_type){
        .type = &(struct tl_type_descr) {.name = 0x5d652a7f, .id = "Message", .params_num = 0, .params_types = 0},
        .params = 0,
      },
    }
  };
  result->messages = (void *)fetch_ds_type_vector (field1);
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x1cb5c415, .id = "Vector", .params_num = 1, .params_types = 0},
    .params = (struct paramed_type *[]){
      &(struct paramed_type){
        .type = &(struct tl_type_descr) {.name = 0x7bd865dc, .id = "Chat", .params_num = 0, .params_types = 0},
        .params = 0,
      },
    }
  };
  result->chats = (void *)fetch_ds_type_vector (field2);
  struct paramed_type *field3 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x1cb5c415, .id = "Vector", .params_num = 1, .params_types = 0},
    .params = (struct paramed_type *[]){
      &(struct paramed_type){
        .type = &(struct tl_type_descr) {.name = 0x02e6c0c8, .id = "User", .params_num = 0, .params_types = 0},
        .params = 0,
      },
    }
  };
  result->users = (void *)fetch_ds_type_vector (field3);
  return result;
}
struct tl_ds_messages_messages *fetch_ds_constructor_messages_messages_slice (struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x8735e464 && T->type->name != 0x78ca1b9b)) { return 0; }
  struct tl_ds_messages_messages *  result = talloc0 (sizeof (*result));
  result->magic = 0x0b446ae3;
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->count = fetch_ds_type_bare_int (field1);
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x1cb5c415, .id = "Vector", .params_num = 1, .params_types = 0},
    .params = (struct paramed_type *[]){
      &(struct paramed_type){
        .type = &(struct tl_type_descr) {.name = 0x5d652a7f, .id = "Message", .params_num = 0, .params_types = 0},
        .params = 0,
      },
    }
  };
  result->messages = (void *)fetch_ds_type_vector (field2);
  struct paramed_type *field3 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x1cb5c415, .id = "Vector", .params_num = 1, .params_types = 0},
    .params = (struct paramed_type *[]){
      &(struct paramed_type){
        .type = &(struct tl_type_descr) {.name = 0x7bd865dc, .id = "Chat", .params_num = 0, .params_types = 0},
        .params = 0,
      },
    }
  };
  result->chats = (void *)fetch_ds_type_vector (field3);
  struct paramed_type *field4 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x1cb5c415, .id = "Vector", .params_num = 1, .params_types = 0},
    .params = (struct paramed_type *[]){
      &(struct paramed_type){
        .type = &(struct tl_type_descr) {.name = 0x02e6c0c8, .id = "User", .params_num = 0, .params_types = 0},
        .params = 0,
      },
    }
  };
  result->users = (void *)fetch_ds_type_vector (field4);
  return result;
}
struct tl_ds_messages_sent_encrypted_message *fetch_ds_constructor_messages_sent_encrypted_message (struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0xc29c7607 && T->type->name != 0x3d6389f8)) { return 0; }
  struct tl_ds_messages_sent_encrypted_message *  result = talloc0 (sizeof (*result));
  result->magic = 0x560f8935;
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->date = fetch_ds_type_bare_int (field1);
  return result;
}
struct tl_ds_messages_sent_encrypted_message *fetch_ds_constructor_messages_sent_encrypted_file (struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0xc29c7607 && T->type->name != 0x3d6389f8)) { return 0; }
  struct tl_ds_messages_sent_encrypted_message *  result = talloc0 (sizeof (*result));
  result->magic = 0x9493ff32;
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->date = fetch_ds_type_bare_int (field1);
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x886fd032, .id = "EncryptedFile", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->file = fetch_ds_type_encrypted_file (field2);
  return result;
}
struct tl_ds_messages_sent_message *fetch_ds_constructor_messages_sent_message (struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x799ce190 && T->type->name != 0x86631e6f)) { return 0; }
  struct tl_ds_messages_sent_message *  result = talloc0 (sizeof (*result));
  result->magic = 0x4c3d47f3;
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->id = fetch_ds_type_bare_int (field1);
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->date = fetch_ds_type_bare_int (field2);
  struct paramed_type *field3 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x95f2471a, .id = "MessageMedia", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->media = fetch_ds_type_message_media (field3);
  struct paramed_type *field4 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->pts = fetch_ds_type_bare_int (field4);
  struct paramed_type *field5 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->pts_count = fetch_ds_type_bare_int (field5);
  return result;
}
struct tl_ds_messages_sent_message *fetch_ds_constructor_messages_sent_message_link (struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x799ce190 && T->type->name != 0x86631e6f)) { return 0; }
  struct tl_ds_messages_sent_message *  result = talloc0 (sizeof (*result));
  result->magic = 0x35a1a663;
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->id = fetch_ds_type_bare_int (field1);
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->date = fetch_ds_type_bare_int (field2);
  struct paramed_type *field3 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x95f2471a, .id = "MessageMedia", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->media = fetch_ds_type_message_media (field3);
  struct paramed_type *field4 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->pts = fetch_ds_type_bare_int (field4);
  struct paramed_type *field5 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->pts_count = fetch_ds_type_bare_int (field5);
  struct paramed_type *field6 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x1cb5c415, .id = "Vector", .params_num = 1, .params_types = 0},
    .params = (struct paramed_type *[]){
      &(struct paramed_type){
        .type = &(struct tl_type_descr) {.name = 0x3ace484c, .id = "contacts.Link", .params_num = 0, .params_types = 0},
        .params = 0,
      },
    }
  };
  result->links = (void *)fetch_ds_type_vector (field6);
  struct paramed_type *field7 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->seq = fetch_ds_type_bare_int (field7);
  return result;
}
struct tl_ds_messages_sticker_set *fetch_ds_constructor_messages_sticker_set (struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0xb60a24a6 && T->type->name != 0x49f5db59)) { return 0; }
  struct tl_ds_messages_sticker_set *  result = talloc0 (sizeof (*result));
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xa7a43b17, .id = "StickerSet", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->set = fetch_ds_type_sticker_set (field1);
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x1cb5c415, .id = "Vector", .params_num = 1, .params_types = 0},
    .params = (struct paramed_type *[]){
      &(struct paramed_type){
        .type = &(struct tl_type_descr) {.name = 0x12b299d4, .id = "StickerPack", .params_num = 0, .params_types = 0},
        .params = 0,
      },
    }
  };
  result->packs = (void *)fetch_ds_type_vector (field2);
  struct paramed_type *field3 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x1cb5c415, .id = "Vector", .params_num = 1, .params_types = 0},
    .params = (struct paramed_type *[]){
      &(struct paramed_type){
        .type = &(struct tl_type_descr) {.name = 0x51a73418, .id = "Document", .params_num = 0, .params_types = 0},
        .params = 0,
      },
    }
  };
  result->documents = (void *)fetch_ds_type_vector (field3);
  return result;
}
struct tl_ds_messages_stickers *fetch_ds_constructor_messages_stickers_not_modified (struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x7bfa5710 && T->type->name != 0x8405a8ef)) { return 0; }
  struct tl_ds_messages_stickers *  result = talloc0 (sizeof (*result));
  result->magic = 0xf1749a22;
  return result;
}
struct tl_ds_messages_stickers *fetch_ds_constructor_messages_stickers (struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x7bfa5710 && T->type->name != 0x8405a8ef)) { return 0; }
  struct tl_ds_messages_stickers *  result = talloc0 (sizeof (*result));
  result->magic = 0x8a8ecd32;
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->hash = fetch_ds_type_bare_string (field1);
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x1cb5c415, .id = "Vector", .params_num = 1, .params_types = 0},
    .params = (struct paramed_type *[]){
      &(struct paramed_type){
        .type = &(struct tl_type_descr) {.name = 0x51a73418, .id = "Document", .params_num = 0, .params_types = 0},
        .params = 0,
      },
    }
  };
  result->stickers = (void *)fetch_ds_type_vector (field2);
  return result;
}
struct tl_ds_photos_photo *fetch_ds_constructor_photos_photo (struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x20212ca8 && T->type->name != 0xdfded357)) { return 0; }
  struct tl_ds_photos_photo *  result = talloc0 (sizeof (*result));
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xc207550a, .id = "Photo", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->photo = fetch_ds_type_photo (field1);
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x1cb5c415, .id = "Vector", .params_num = 1, .params_types = 0},
    .params = (struct paramed_type *[]){
      &(struct paramed_type){
        .type = &(struct tl_type_descr) {.name = 0x02e6c0c8, .id = "User", .params_num = 0, .params_types = 0},
        .params = 0,
      },
    }
  };
  result->users = (void *)fetch_ds_type_vector (field2);
  return result;
}
struct tl_ds_photos_photos *fetch_ds_constructor_photos_photos (struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x98cf75f1 && T->type->name != 0x67308a0e)) { return 0; }
  struct tl_ds_photos_photos *  result = talloc0 (sizeof (*result));
  result->magic = 0x8dca6aa5;
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x1cb5c415, .id = "Vector", .params_num = 1, .params_types = 0},
    .params = (struct paramed_type *[]){
      &(struct paramed_type){
        .type = &(struct tl_type_descr) {.name = 0xc207550a, .id = "Photo", .params_num = 0, .params_types = 0},
        .params = 0,
      },
    }
  };
  result->photos = (void *)fetch_ds_type_vector (field1);
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x1cb5c415, .id = "Vector", .params_num = 1, .params_types = 0},
    .params = (struct paramed_type *[]){
      &(struct paramed_type){
        .type = &(struct tl_type_descr) {.name = 0x02e6c0c8, .id = "User", .params_num = 0, .params_types = 0},
        .params = 0,
      },
    }
  };
  result->users = (void *)fetch_ds_type_vector (field2);
  return result;
}
struct tl_ds_photos_photos *fetch_ds_constructor_photos_photos_slice (struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x98cf75f1 && T->type->name != 0x67308a0e)) { return 0; }
  struct tl_ds_photos_photos *  result = talloc0 (sizeof (*result));
  result->magic = 0x15051f54;
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->count = fetch_ds_type_bare_int (field1);
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x1cb5c415, .id = "Vector", .params_num = 1, .params_types = 0},
    .params = (struct paramed_type *[]){
      &(struct paramed_type){
        .type = &(struct tl_type_descr) {.name = 0xc207550a, .id = "Photo", .params_num = 0, .params_types = 0},
        .params = 0,
      },
    }
  };
  result->photos = (void *)fetch_ds_type_vector (field2);
  struct paramed_type *field3 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x1cb5c415, .id = "Vector", .params_num = 1, .params_types = 0},
    .params = (struct paramed_type *[]){
      &(struct paramed_type){
        .type = &(struct tl_type_descr) {.name = 0x02e6c0c8, .id = "User", .params_num = 0, .params_types = 0},
        .params = 0,
      },
    }
  };
  result->users = (void *)fetch_ds_type_vector (field3);
  return result;
}
struct tl_ds_storage_file_type *fetch_ds_constructor_storage_file_unknown (struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x3e2838a8 && T->type->name != 0xc1d7c757)) { return 0; }
  struct tl_ds_storage_file_type *  result = talloc0 (sizeof (*result));
  result->magic = 0xaa963b05;
  return result;
}
struct tl_ds_storage_file_type *fetch_ds_constructor_storage_file_jpeg (struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x3e2838a8 && T->type->name != 0xc1d7c757)) { return 0; }
  struct tl_ds_storage_file_type *  result = talloc0 (sizeof (*result));
  result->magic = 0x007efe0e;
  return result;
}
struct tl_ds_storage_file_type *fetch_ds_constructor_storage_file_gif (struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x3e2838a8 && T->type->name != 0xc1d7c757)) { return 0; }
  struct tl_ds_storage_file_type *  result = talloc0 (sizeof (*result));
  result->magic = 0xcae1aadf;
  return result;
}
struct tl_ds_storage_file_type *fetch_ds_constructor_storage_file_png (struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x3e2838a8 && T->type->name != 0xc1d7c757)) { return 0; }
  struct tl_ds_storage_file_type *  result = talloc0 (sizeof (*result));
  result->magic = 0x0a4f63c0;
  return result;
}
struct tl_ds_storage_file_type *fetch_ds_constructor_storage_file_pdf (struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x3e2838a8 && T->type->name != 0xc1d7c757)) { return 0; }
  struct tl_ds_storage_file_type *  result = talloc0 (sizeof (*result));
  result->magic = 0xae1e508d;
  return result;
}
struct tl_ds_storage_file_type *fetch_ds_constructor_storage_file_mp3 (struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x3e2838a8 && T->type->name != 0xc1d7c757)) { return 0; }
  struct tl_ds_storage_file_type *  result = talloc0 (sizeof (*result));
  result->magic = 0x528a0677;
  return result;
}
struct tl_ds_storage_file_type *fetch_ds_constructor_storage_file_mov (struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x3e2838a8 && T->type->name != 0xc1d7c757)) { return 0; }
  struct tl_ds_storage_file_type *  result = talloc0 (sizeof (*result));
  result->magic = 0x4b09ebbc;
  return result;
}
struct tl_ds_storage_file_type *fetch_ds_constructor_storage_file_partial (struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x3e2838a8 && T->type->name != 0xc1d7c757)) { return 0; }
  struct tl_ds_storage_file_type *  result = talloc0 (sizeof (*result));
  result->magic = 0x40bc6f52;
  return result;
}
struct tl_ds_storage_file_type *fetch_ds_constructor_storage_file_mp4 (struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x3e2838a8 && T->type->name != 0xc1d7c757)) { return 0; }
  struct tl_ds_storage_file_type *  result = talloc0 (sizeof (*result));
  result->magic = 0xb3cea0e4;
  return result;
}
struct tl_ds_storage_file_type *fetch_ds_constructor_storage_file_webp (struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x3e2838a8 && T->type->name != 0xc1d7c757)) { return 0; }
  struct tl_ds_storage_file_type *  result = talloc0 (sizeof (*result));
  result->magic = 0x1081464c;
  return result;
}
struct tl_ds_updates_difference *fetch_ds_constructor_updates_difference_empty (struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0xf57a2419 && T->type->name != 0x0a85dbe6)) { return 0; }
  struct tl_ds_updates_difference *  result = talloc0 (sizeof (*result));
  result->magic = 0x5d75a138;
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->date = fetch_ds_type_bare_int (field1);
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->seq = fetch_ds_type_bare_int (field2);
  return result;
}
struct tl_ds_updates_difference *fetch_ds_constructor_updates_difference (struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0xf57a2419 && T->type->name != 0x0a85dbe6)) { return 0; }
  struct tl_ds_updates_difference *  result = talloc0 (sizeof (*result));
  result->magic = 0x00f49ca0;
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x1cb5c415, .id = "Vector", .params_num = 1, .params_types = 0},
    .params = (struct paramed_type *[]){
      &(struct paramed_type){
        .type = &(struct tl_type_descr) {.name = 0x5d652a7f, .id = "Message", .params_num = 0, .params_types = 0},
        .params = 0,
      },
    }
  };
  result->new_messages = (void *)fetch_ds_type_vector (field1);
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x1cb5c415, .id = "Vector", .params_num = 1, .params_types = 0},
    .params = (struct paramed_type *[]){
      &(struct paramed_type){
        .type = &(struct tl_type_descr) {.name = 0xce6b8a1e, .id = "EncryptedMessage", .params_num = 0, .params_types = 0},
        .params = 0,
      },
    }
  };
  result->new_encrypted_messages = (void *)fetch_ds_type_vector (field2);
  struct paramed_type *field3 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x1cb5c415, .id = "Vector", .params_num = 1, .params_types = 0},
    .params = (struct paramed_type *[]){
      &(struct paramed_type){
        .type = &(struct tl_type_descr) {.name = 0x2e02b27e, .id = "Update", .params_num = 0, .params_types = 0},
        .params = 0,
      },
    }
  };
  result->other_updates = (void *)fetch_ds_type_vector (field3);
  struct paramed_type *field4 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x1cb5c415, .id = "Vector", .params_num = 1, .params_types = 0},
    .params = (struct paramed_type *[]){
      &(struct paramed_type){
        .type = &(struct tl_type_descr) {.name = 0x7bd865dc, .id = "Chat", .params_num = 0, .params_types = 0},
        .params = 0,
      },
    }
  };
  result->chats = (void *)fetch_ds_type_vector (field4);
  struct paramed_type *field5 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x1cb5c415, .id = "Vector", .params_num = 1, .params_types = 0},
    .params = (struct paramed_type *[]){
      &(struct paramed_type){
        .type = &(struct tl_type_descr) {.name = 0x02e6c0c8, .id = "User", .params_num = 0, .params_types = 0},
        .params = 0,
      },
    }
  };
  result->users = (void *)fetch_ds_type_vector (field5);
  struct paramed_type *field6 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xa56c2a3e, .id = "updates.State", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->state = fetch_ds_type_updates_state (field6);
  return result;
}
struct tl_ds_updates_difference *fetch_ds_constructor_updates_difference_slice (struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0xf57a2419 && T->type->name != 0x0a85dbe6)) { return 0; }
  struct tl_ds_updates_difference *  result = talloc0 (sizeof (*result));
  result->magic = 0xa8fb1981;
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x1cb5c415, .id = "Vector", .params_num = 1, .params_types = 0},
    .params = (struct paramed_type *[]){
      &(struct paramed_type){
        .type = &(struct tl_type_descr) {.name = 0x5d652a7f, .id = "Message", .params_num = 0, .params_types = 0},
        .params = 0,
      },
    }
  };
  result->new_messages = (void *)fetch_ds_type_vector (field1);
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x1cb5c415, .id = "Vector", .params_num = 1, .params_types = 0},
    .params = (struct paramed_type *[]){
      &(struct paramed_type){
        .type = &(struct tl_type_descr) {.name = 0xce6b8a1e, .id = "EncryptedMessage", .params_num = 0, .params_types = 0},
        .params = 0,
      },
    }
  };
  result->new_encrypted_messages = (void *)fetch_ds_type_vector (field2);
  struct paramed_type *field3 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x1cb5c415, .id = "Vector", .params_num = 1, .params_types = 0},
    .params = (struct paramed_type *[]){
      &(struct paramed_type){
        .type = &(struct tl_type_descr) {.name = 0x2e02b27e, .id = "Update", .params_num = 0, .params_types = 0},
        .params = 0,
      },
    }
  };
  result->other_updates = (void *)fetch_ds_type_vector (field3);
  struct paramed_type *field4 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x1cb5c415, .id = "Vector", .params_num = 1, .params_types = 0},
    .params = (struct paramed_type *[]){
      &(struct paramed_type){
        .type = &(struct tl_type_descr) {.name = 0x7bd865dc, .id = "Chat", .params_num = 0, .params_types = 0},
        .params = 0,
      },
    }
  };
  result->chats = (void *)fetch_ds_type_vector (field4);
  struct paramed_type *field5 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x1cb5c415, .id = "Vector", .params_num = 1, .params_types = 0},
    .params = (struct paramed_type *[]){
      &(struct paramed_type){
        .type = &(struct tl_type_descr) {.name = 0x02e6c0c8, .id = "User", .params_num = 0, .params_types = 0},
        .params = 0,
      },
    }
  };
  result->users = (void *)fetch_ds_type_vector (field5);
  struct paramed_type *field6 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xa56c2a3e, .id = "updates.State", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->intermediate_state = fetch_ds_type_updates_state (field6);
  return result;
}
struct tl_ds_updates_state *fetch_ds_constructor_updates_state (struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0xa56c2a3e && T->type->name != 0x5a93d5c1)) { return 0; }
  struct tl_ds_updates_state *  result = talloc0 (sizeof (*result));
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->pts = fetch_ds_type_bare_int (field1);
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->qts = fetch_ds_type_bare_int (field2);
  struct paramed_type *field3 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->date = fetch_ds_type_bare_int (field3);
  struct paramed_type *field4 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->seq = fetch_ds_type_bare_int (field4);
  struct paramed_type *field5 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->unread_count = fetch_ds_type_bare_int (field5);
  return result;
}
struct tl_ds_upload_file *fetch_ds_constructor_upload_file (struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x096a18d5 && T->type->name != 0xf695e72a)) { return 0; }
  struct tl_ds_upload_file *  result = talloc0 (sizeof (*result));
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x3e2838a8, .id = "storage.FileType", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->type = fetch_ds_type_storage_file_type (field1);
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->mtime = fetch_ds_type_bare_int (field2);
  struct paramed_type *field3 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xf11ec860, .id = "Bare_Bytes", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  result->bytes = fetch_ds_type_bare_bytes (field3);
  return result;
}
struct tl_ds_account_days_t_t_l *fetch_ds_type_account_days_t_t_l (struct paramed_type *T) {
  assert (in_remaining () >= 4);
  int magic = fetch_int ();
  switch (magic) {
  case 0xb8d0afdf: return fetch_ds_constructor_account_days_t_t_l (T); break;
  default: assert (0); return NULL;
  }
}
struct tl_ds_account_days_t_t_l *fetch_ds_type_bare_account_days_t_t_l (struct paramed_type *T) {
  return fetch_ds_constructor_account_days_t_t_l (T);
  assert (0);
  return NULL;
}
struct tl_ds_audio *fetch_ds_type_audio (struct paramed_type *T) {
  assert (in_remaining () >= 4);
  int magic = fetch_int ();
  switch (magic) {
  case 0x586988d8: return fetch_ds_constructor_audio_empty (T); break;
  case 0xc7ac6496: return fetch_ds_constructor_audio (T); break;
  default: assert (0); return NULL;
  }
}
struct tl_ds_audio *fetch_ds_type_bare_audio (struct paramed_type *T) {
  int *save_in_ptr = in_ptr;
  if (skip_constructor_audio_empty (T) >= 0) { in_ptr = save_in_ptr; return fetch_ds_constructor_audio_empty (T); }
  if (skip_constructor_audio (T) >= 0) { in_ptr = save_in_ptr; return fetch_ds_constructor_audio (T); }
  assert (0);
  return NULL;
}
struct tl_ds_authorization *fetch_ds_type_authorization (struct paramed_type *T) {
  assert (in_remaining () >= 4);
  int magic = fetch_int ();
  switch (magic) {
  case 0x7bf2e6f6: return fetch_ds_constructor_authorization (T); break;
  default: assert (0); return NULL;
  }
}
struct tl_ds_authorization *fetch_ds_type_bare_authorization (struct paramed_type *T) {
  return fetch_ds_constructor_authorization (T);
  assert (0);
  return NULL;
}
struct tl_ds_bool *fetch_ds_type_bool (struct paramed_type *T) {
  assert (in_remaining () >= 4);
  int magic = fetch_int ();
  switch (magic) {
  case 0xbc799737: return fetch_ds_constructor_bool_false (T); break;
  case 0x997275b5: return fetch_ds_constructor_bool_true (T); break;
  default: assert (0); return NULL;
  }
}
struct tl_ds_bool *fetch_ds_type_bare_bool (struct paramed_type *T) {
  int *save_in_ptr = in_ptr;
  if (skip_constructor_bool_false (T) >= 0) { in_ptr = save_in_ptr; return fetch_ds_constructor_bool_false (T); }
  if (skip_constructor_bool_true (T) >= 0) { in_ptr = save_in_ptr; return fetch_ds_constructor_bool_true (T); }
  assert (0);
  return NULL;
}
struct tl_ds_bot_command *fetch_ds_type_bot_command (struct paramed_type *T) {
  assert (in_remaining () >= 4);
  int magic = fetch_int ();
  switch (magic) {
  case 0xc27ac8c7: return fetch_ds_constructor_bot_command (T); break;
  case 0xb79d22ab: return fetch_ds_constructor_bot_command_old (T); break;
  default: assert (0); return NULL;
  }
}
struct tl_ds_bot_command *fetch_ds_type_bare_bot_command (struct paramed_type *T) {
  int *save_in_ptr = in_ptr;
  if (skip_constructor_bot_command (T) >= 0) { in_ptr = save_in_ptr; return fetch_ds_constructor_bot_command (T); }
  if (skip_constructor_bot_command_old (T) >= 0) { in_ptr = save_in_ptr; return fetch_ds_constructor_bot_command_old (T); }
  assert (0);
  return NULL;
}
struct tl_ds_bot_info *fetch_ds_type_bot_info (struct paramed_type *T) {
  assert (in_remaining () >= 4);
  int magic = fetch_int ();
  switch (magic) {
  case 0xbb2e37ce: return fetch_ds_constructor_bot_info_empty (T); break;
  case 0x09cf585d: return fetch_ds_constructor_bot_info (T); break;
  default: assert (0); return NULL;
  }
}
struct tl_ds_bot_info *fetch_ds_type_bare_bot_info (struct paramed_type *T) {
  int *save_in_ptr = in_ptr;
  if (skip_constructor_bot_info_empty (T) >= 0) { in_ptr = save_in_ptr; return fetch_ds_constructor_bot_info_empty (T); }
  if (skip_constructor_bot_info (T) >= 0) { in_ptr = save_in_ptr; return fetch_ds_constructor_bot_info (T); }
  assert (0);
  return NULL;
}
struct tl_ds_string *fetch_ds_type_bytes (struct paramed_type *T) {
  assert (in_remaining () >= 4);
  int magic = fetch_int ();
  switch (magic) {
  case 0x0ee1379f: return fetch_ds_constructor_bytes (T); break;
  default: assert (0); return NULL;
  }
}
struct tl_ds_string *fetch_ds_type_bare_bytes (struct paramed_type *T) {
  return fetch_ds_constructor_bytes (T);
  assert (0);
  return NULL;
}
struct tl_ds_chat *fetch_ds_type_chat (struct paramed_type *T) {
  assert (in_remaining () >= 4);
  int magic = fetch_int ();
  switch (magic) {
  case 0x9ba2d800: return fetch_ds_constructor_chat_empty (T); break;
  case 0x6e9c9bc7: return fetch_ds_constructor_chat (T); break;
  case 0xfb0ccc41: return fetch_ds_constructor_chat_forbidden (T); break;
  case 0x75eaea5a: return fetch_ds_constructor_geo_chat (T); break;
  default: assert (0); return NULL;
  }
}
struct tl_ds_chat *fetch_ds_type_bare_chat (struct paramed_type *T) {
  int *save_in_ptr = in_ptr;
  if (skip_constructor_chat_empty (T) >= 0) { in_ptr = save_in_ptr; return fetch_ds_constructor_chat_empty (T); }
  if (skip_constructor_chat (T) >= 0) { in_ptr = save_in_ptr; return fetch_ds_constructor_chat (T); }
  if (skip_constructor_chat_forbidden (T) >= 0) { in_ptr = save_in_ptr; return fetch_ds_constructor_chat_forbidden (T); }
  if (skip_constructor_geo_chat (T) >= 0) { in_ptr = save_in_ptr; return fetch_ds_constructor_geo_chat (T); }
  assert (0);
  return NULL;
}
struct tl_ds_chat_full *fetch_ds_type_chat_full (struct paramed_type *T) {
  assert (in_remaining () >= 4);
  int magic = fetch_int ();
  switch (magic) {
  case 0x2e02a614: return fetch_ds_constructor_chat_full (T); break;
  default: assert (0); return NULL;
  }
}
struct tl_ds_chat_full *fetch_ds_type_bare_chat_full (struct paramed_type *T) {
  return fetch_ds_constructor_chat_full (T);
  assert (0);
  return NULL;
}
struct tl_ds_chat_invite *fetch_ds_type_chat_invite (struct paramed_type *T) {
  assert (in_remaining () >= 4);
  int magic = fetch_int ();
  switch (magic) {
  case 0x5a686d7c: return fetch_ds_constructor_chat_invite_already (T); break;
  case 0xce917dcd: return fetch_ds_constructor_chat_invite (T); break;
  default: assert (0); return NULL;
  }
}
struct tl_ds_chat_invite *fetch_ds_type_bare_chat_invite (struct paramed_type *T) {
  int *save_in_ptr = in_ptr;
  if (skip_constructor_chat_invite_already (T) >= 0) { in_ptr = save_in_ptr; return fetch_ds_constructor_chat_invite_already (T); }
  if (skip_constructor_chat_invite (T) >= 0) { in_ptr = save_in_ptr; return fetch_ds_constructor_chat_invite (T); }
  assert (0);
  return NULL;
}
struct tl_ds_chat_located *fetch_ds_type_chat_located (struct paramed_type *T) {
  assert (in_remaining () >= 4);
  int magic = fetch_int ();
  switch (magic) {
  case 0x3631cf4c: return fetch_ds_constructor_chat_located (T); break;
  default: assert (0); return NULL;
  }
}
struct tl_ds_chat_located *fetch_ds_type_bare_chat_located (struct paramed_type *T) {
  return fetch_ds_constructor_chat_located (T);
  assert (0);
  return NULL;
}
struct tl_ds_chat_participant *fetch_ds_type_chat_participant (struct paramed_type *T) {
  assert (in_remaining () >= 4);
  int magic = fetch_int ();
  switch (magic) {
  case 0xc8d7493e: return fetch_ds_constructor_chat_participant (T); break;
  default: assert (0); return NULL;
  }
}
struct tl_ds_chat_participant *fetch_ds_type_bare_chat_participant (struct paramed_type *T) {
  return fetch_ds_constructor_chat_participant (T);
  assert (0);
  return NULL;
}
struct tl_ds_chat_participants *fetch_ds_type_chat_participants (struct paramed_type *T) {
  assert (in_remaining () >= 4);
  int magic = fetch_int ();
  switch (magic) {
  case 0x0fd2bb8a: return fetch_ds_constructor_chat_participants_forbidden (T); break;
  case 0x7841b415: return fetch_ds_constructor_chat_participants (T); break;
  default: assert (0); return NULL;
  }
}
struct tl_ds_chat_participants *fetch_ds_type_bare_chat_participants (struct paramed_type *T) {
  int *save_in_ptr = in_ptr;
  if (skip_constructor_chat_participants_forbidden (T) >= 0) { in_ptr = save_in_ptr; return fetch_ds_constructor_chat_participants_forbidden (T); }
  if (skip_constructor_chat_participants (T) >= 0) { in_ptr = save_in_ptr; return fetch_ds_constructor_chat_participants (T); }
  assert (0);
  return NULL;
}
struct tl_ds_chat_photo *fetch_ds_type_chat_photo (struct paramed_type *T) {
  assert (in_remaining () >= 4);
  int magic = fetch_int ();
  switch (magic) {
  case 0x37c1011c: return fetch_ds_constructor_chat_photo_empty (T); break;
  case 0x6153276a: return fetch_ds_constructor_chat_photo (T); break;
  default: assert (0); return NULL;
  }
}
struct tl_ds_chat_photo *fetch_ds_type_bare_chat_photo (struct paramed_type *T) {
  int *save_in_ptr = in_ptr;
  if (skip_constructor_chat_photo_empty (T) >= 0) { in_ptr = save_in_ptr; return fetch_ds_constructor_chat_photo_empty (T); }
  if (skip_constructor_chat_photo (T) >= 0) { in_ptr = save_in_ptr; return fetch_ds_constructor_chat_photo (T); }
  assert (0);
  return NULL;
}
struct tl_ds_client_d_h_inner_data *fetch_ds_type_client_d_h_inner_data (struct paramed_type *T) {
  assert (in_remaining () >= 4);
  int magic = fetch_int ();
  switch (magic) {
  case 0x6643b654: return fetch_ds_constructor_client_d_h_inner_data (T); break;
  default: assert (0); return NULL;
  }
}
struct tl_ds_client_d_h_inner_data *fetch_ds_type_bare_client_d_h_inner_data (struct paramed_type *T) {
  return fetch_ds_constructor_client_d_h_inner_data (T);
  assert (0);
  return NULL;
}
struct tl_ds_config *fetch_ds_type_config (struct paramed_type *T) {
  assert (in_remaining () >= 4);
  int magic = fetch_int ();
  switch (magic) {
  case 0x4e32b894: return fetch_ds_constructor_config (T); break;
  default: assert (0); return NULL;
  }
}
struct tl_ds_config *fetch_ds_type_bare_config (struct paramed_type *T) {
  return fetch_ds_constructor_config (T);
  assert (0);
  return NULL;
}
struct tl_ds_contact *fetch_ds_type_contact (struct paramed_type *T) {
  assert (in_remaining () >= 4);
  int magic = fetch_int ();
  switch (magic) {
  case 0xf911c994: return fetch_ds_constructor_contact (T); break;
  default: assert (0); return NULL;
  }
}
struct tl_ds_contact *fetch_ds_type_bare_contact (struct paramed_type *T) {
  return fetch_ds_constructor_contact (T);
  assert (0);
  return NULL;
}
struct tl_ds_contact_blocked *fetch_ds_type_contact_blocked (struct paramed_type *T) {
  assert (in_remaining () >= 4);
  int magic = fetch_int ();
  switch (magic) {
  case 0x561bc879: return fetch_ds_constructor_contact_blocked (T); break;
  default: assert (0); return NULL;
  }
}
struct tl_ds_contact_blocked *fetch_ds_type_bare_contact_blocked (struct paramed_type *T) {
  return fetch_ds_constructor_contact_blocked (T);
  assert (0);
  return NULL;
}
struct tl_ds_contact_found *fetch_ds_type_contact_found (struct paramed_type *T) {
  assert (in_remaining () >= 4);
  int magic = fetch_int ();
  switch (magic) {
  case 0xea879f95: return fetch_ds_constructor_contact_found (T); break;
  default: assert (0); return NULL;
  }
}
struct tl_ds_contact_found *fetch_ds_type_bare_contact_found (struct paramed_type *T) {
  return fetch_ds_constructor_contact_found (T);
  assert (0);
  return NULL;
}
struct tl_ds_contact_link *fetch_ds_type_contact_link (struct paramed_type *T) {
  assert (in_remaining () >= 4);
  int magic = fetch_int ();
  switch (magic) {
  case 0x5f4f9247: return fetch_ds_constructor_contact_link_unknown (T); break;
  case 0xfeedd3ad: return fetch_ds_constructor_contact_link_none (T); break;
  case 0x268f3f59: return fetch_ds_constructor_contact_link_has_phone (T); break;
  case 0xd502c2d0: return fetch_ds_constructor_contact_link_contact (T); break;
  default: assert (0); return NULL;
  }
}
struct tl_ds_contact_link *fetch_ds_type_bare_contact_link (struct paramed_type *T) {
  int *save_in_ptr = in_ptr;
  if (skip_constructor_contact_link_unknown (T) >= 0) { in_ptr = save_in_ptr; return fetch_ds_constructor_contact_link_unknown (T); }
  if (skip_constructor_contact_link_none (T) >= 0) { in_ptr = save_in_ptr; return fetch_ds_constructor_contact_link_none (T); }
  if (skip_constructor_contact_link_has_phone (T) >= 0) { in_ptr = save_in_ptr; return fetch_ds_constructor_contact_link_has_phone (T); }
  if (skip_constructor_contact_link_contact (T) >= 0) { in_ptr = save_in_ptr; return fetch_ds_constructor_contact_link_contact (T); }
  assert (0);
  return NULL;
}
struct tl_ds_contact_status *fetch_ds_type_contact_status (struct paramed_type *T) {
  assert (in_remaining () >= 4);
  int magic = fetch_int ();
  switch (magic) {
  case 0xd3680c61: return fetch_ds_constructor_contact_status (T); break;
  default: assert (0); return NULL;
  }
}
struct tl_ds_contact_status *fetch_ds_type_bare_contact_status (struct paramed_type *T) {
  return fetch_ds_constructor_contact_status (T);
  assert (0);
  return NULL;
}
struct tl_ds_contact_suggested *fetch_ds_type_contact_suggested (struct paramed_type *T) {
  assert (in_remaining () >= 4);
  int magic = fetch_int ();
  switch (magic) {
  case 0x3de191a1: return fetch_ds_constructor_contact_suggested (T); break;
  default: assert (0); return NULL;
  }
}
struct tl_ds_contact_suggested *fetch_ds_type_bare_contact_suggested (struct paramed_type *T) {
  return fetch_ds_constructor_contact_suggested (T);
  assert (0);
  return NULL;
}
struct tl_ds_dc_option *fetch_ds_type_dc_option (struct paramed_type *T) {
  assert (in_remaining () >= 4);
  int magic = fetch_int ();
  switch (magic) {
  case 0x2ec2a43c: return fetch_ds_constructor_dc_option_l28 (T); break;
  case 0x05d8c6cc: return fetch_ds_constructor_dc_option (T); break;
  default: assert (0); return NULL;
  }
}
struct tl_ds_dc_option *fetch_ds_type_bare_dc_option (struct paramed_type *T) {
  int *save_in_ptr = in_ptr;
  if (skip_constructor_dc_option_l28 (T) >= 0) { in_ptr = save_in_ptr; return fetch_ds_constructor_dc_option_l28 (T); }
  if (skip_constructor_dc_option (T) >= 0) { in_ptr = save_in_ptr; return fetch_ds_constructor_dc_option (T); }
  assert (0);
  return NULL;
}
struct tl_ds_decrypted_message *fetch_ds_type_decrypted_message (struct paramed_type *T) {
  assert (in_remaining () >= 4);
  int magic = fetch_int ();
  switch (magic) {
  case 0x204d3878: return fetch_ds_constructor_decrypted_message (T); break;
  case 0x73164160: return fetch_ds_constructor_decrypted_message_service (T); break;
  default: assert (0); return NULL;
  }
}
struct tl_ds_decrypted_message *fetch_ds_type_bare_decrypted_message (struct paramed_type *T) {
  int *save_in_ptr = in_ptr;
  if (skip_constructor_decrypted_message (T) >= 0) { in_ptr = save_in_ptr; return fetch_ds_constructor_decrypted_message (T); }
  if (skip_constructor_decrypted_message_service (T) >= 0) { in_ptr = save_in_ptr; return fetch_ds_constructor_decrypted_message_service (T); }
  assert (0);
  return NULL;
}
struct tl_ds_decrypted_message_action *fetch_ds_type_decrypted_message_action (struct paramed_type *T) {
  assert (in_remaining () >= 4);
  int magic = fetch_int ();
  switch (magic) {
  case 0xa1733aec: return fetch_ds_constructor_decrypted_message_action_set_message_t_t_l (T); break;
  case 0x0c4f40be: return fetch_ds_constructor_decrypted_message_action_read_messages (T); break;
  case 0x65614304: return fetch_ds_constructor_decrypted_message_action_delete_messages (T); break;
  case 0x8ac1f475: return fetch_ds_constructor_decrypted_message_action_screenshot_messages (T); break;
  case 0x6719e45c: return fetch_ds_constructor_decrypted_message_action_flush_history (T); break;
  case 0x511110b0: return fetch_ds_constructor_decrypted_message_action_resend (T); break;
  case 0xf3048883: return fetch_ds_constructor_decrypted_message_action_notify_layer (T); break;
  case 0xccb27641: return fetch_ds_constructor_decrypted_message_action_typing (T); break;
  case 0xf3c9611b: return fetch_ds_constructor_decrypted_message_action_request_key (T); break;
  case 0x6fe1735b: return fetch_ds_constructor_decrypted_message_action_accept_key (T); break;
  case 0xdd05ec6b: return fetch_ds_constructor_decrypted_message_action_abort_key (T); break;
  case 0xec2e0b9b: return fetch_ds_constructor_decrypted_message_action_commit_key (T); break;
  case 0xa82fdd63: return fetch_ds_constructor_decrypted_message_action_noop (T); break;
  default: assert (0); return NULL;
  }
}
struct tl_ds_decrypted_message_action *fetch_ds_type_bare_decrypted_message_action (struct paramed_type *T) {
  int *save_in_ptr = in_ptr;
  if (skip_constructor_decrypted_message_action_set_message_t_t_l (T) >= 0) { in_ptr = save_in_ptr; return fetch_ds_constructor_decrypted_message_action_set_message_t_t_l (T); }
  if (skip_constructor_decrypted_message_action_read_messages (T) >= 0) { in_ptr = save_in_ptr; return fetch_ds_constructor_decrypted_message_action_read_messages (T); }
  if (skip_constructor_decrypted_message_action_delete_messages (T) >= 0) { in_ptr = save_in_ptr; return fetch_ds_constructor_decrypted_message_action_delete_messages (T); }
  if (skip_constructor_decrypted_message_action_screenshot_messages (T) >= 0) { in_ptr = save_in_ptr; return fetch_ds_constructor_decrypted_message_action_screenshot_messages (T); }
  if (skip_constructor_decrypted_message_action_flush_history (T) >= 0) { in_ptr = save_in_ptr; return fetch_ds_constructor_decrypted_message_action_flush_history (T); }
  if (skip_constructor_decrypted_message_action_resend (T) >= 0) { in_ptr = save_in_ptr; return fetch_ds_constructor_decrypted_message_action_resend (T); }
  if (skip_constructor_decrypted_message_action_notify_layer (T) >= 0) { in_ptr = save_in_ptr; return fetch_ds_constructor_decrypted_message_action_notify_layer (T); }
  if (skip_constructor_decrypted_message_action_typing (T) >= 0) { in_ptr = save_in_ptr; return fetch_ds_constructor_decrypted_message_action_typing (T); }
  if (skip_constructor_decrypted_message_action_request_key (T) >= 0) { in_ptr = save_in_ptr; return fetch_ds_constructor_decrypted_message_action_request_key (T); }
  if (skip_constructor_decrypted_message_action_accept_key (T) >= 0) { in_ptr = save_in_ptr; return fetch_ds_constructor_decrypted_message_action_accept_key (T); }
  if (skip_constructor_decrypted_message_action_abort_key (T) >= 0) { in_ptr = save_in_ptr; return fetch_ds_constructor_decrypted_message_action_abort_key (T); }
  if (skip_constructor_decrypted_message_action_commit_key (T) >= 0) { in_ptr = save_in_ptr; return fetch_ds_constructor_decrypted_message_action_commit_key (T); }
  if (skip_constructor_decrypted_message_action_noop (T) >= 0) { in_ptr = save_in_ptr; return fetch_ds_constructor_decrypted_message_action_noop (T); }
  assert (0);
  return NULL;
}
struct tl_ds_decrypted_message_layer *fetch_ds_type_decrypted_message_layer (struct paramed_type *T) {
  assert (in_remaining () >= 4);
  int magic = fetch_int ();
  switch (magic) {
  case 0x1be31789: return fetch_ds_constructor_decrypted_message_layer (T); break;
  default: assert (0); return NULL;
  }
}
struct tl_ds_decrypted_message_layer *fetch_ds_type_bare_decrypted_message_layer (struct paramed_type *T) {
  return fetch_ds_constructor_decrypted_message_layer (T);
  assert (0);
  return NULL;
}
struct tl_ds_decrypted_message_media *fetch_ds_type_decrypted_message_media (struct paramed_type *T) {
  assert (in_remaining () >= 4);
  int magic = fetch_int ();
  switch (magic) {
  case 0x089f5c4a: return fetch_ds_constructor_decrypted_message_media_empty (T); break;
  case 0x32798a8c: return fetch_ds_constructor_decrypted_message_media_photo (T); break;
  case 0x35480a59: return fetch_ds_constructor_decrypted_message_media_geo_point (T); break;
  case 0x588a0a97: return fetch_ds_constructor_decrypted_message_media_contact (T); break;
  case 0xb095434b: return fetch_ds_constructor_decrypted_message_media_document (T); break;
  case 0x524a415d: return fetch_ds_constructor_decrypted_message_media_video (T); break;
  case 0x57e0a9cb: return fetch_ds_constructor_decrypted_message_media_audio (T); break;
  case 0xfa95b0dd: return fetch_ds_constructor_decrypted_message_media_external_document (T); break;
  case 0x4cee6ef3: return fetch_ds_constructor_decrypted_message_media_video_l12 (T); break;
  case 0x6080758f: return fetch_ds_constructor_decrypted_message_media_audio_l12 (T); break;
  default: assert (0); return NULL;
  }
}
struct tl_ds_decrypted_message_media *fetch_ds_type_bare_decrypted_message_media (struct paramed_type *T) {
  int *save_in_ptr = in_ptr;
  if (skip_constructor_decrypted_message_media_empty (T) >= 0) { in_ptr = save_in_ptr; return fetch_ds_constructor_decrypted_message_media_empty (T); }
  if (skip_constructor_decrypted_message_media_photo (T) >= 0) { in_ptr = save_in_ptr; return fetch_ds_constructor_decrypted_message_media_photo (T); }
  if (skip_constructor_decrypted_message_media_geo_point (T) >= 0) { in_ptr = save_in_ptr; return fetch_ds_constructor_decrypted_message_media_geo_point (T); }
  if (skip_constructor_decrypted_message_media_contact (T) >= 0) { in_ptr = save_in_ptr; return fetch_ds_constructor_decrypted_message_media_contact (T); }
  if (skip_constructor_decrypted_message_media_document (T) >= 0) { in_ptr = save_in_ptr; return fetch_ds_constructor_decrypted_message_media_document (T); }
  if (skip_constructor_decrypted_message_media_video (T) >= 0) { in_ptr = save_in_ptr; return fetch_ds_constructor_decrypted_message_media_video (T); }
  if (skip_constructor_decrypted_message_media_audio (T) >= 0) { in_ptr = save_in_ptr; return fetch_ds_constructor_decrypted_message_media_audio (T); }
  if (skip_constructor_decrypted_message_media_external_document (T) >= 0) { in_ptr = save_in_ptr; return fetch_ds_constructor_decrypted_message_media_external_document (T); }
  if (skip_constructor_decrypted_message_media_video_l12 (T) >= 0) { in_ptr = save_in_ptr; return fetch_ds_constructor_decrypted_message_media_video_l12 (T); }
  if (skip_constructor_decrypted_message_media_audio_l12 (T) >= 0) { in_ptr = save_in_ptr; return fetch_ds_constructor_decrypted_message_media_audio_l12 (T); }
  assert (0);
  return NULL;
}
struct tl_ds_dialog *fetch_ds_type_dialog (struct paramed_type *T) {
  assert (in_remaining () >= 4);
  int magic = fetch_int ();
  switch (magic) {
  case 0xc1dd804a: return fetch_ds_constructor_dialog (T); break;
  default: assert (0); return NULL;
  }
}
struct tl_ds_dialog *fetch_ds_type_bare_dialog (struct paramed_type *T) {
  return fetch_ds_constructor_dialog (T);
  assert (0);
  return NULL;
}
struct tl_ds_disabled_feature *fetch_ds_type_disabled_feature (struct paramed_type *T) {
  assert (in_remaining () >= 4);
  int magic = fetch_int ();
  switch (magic) {
  case 0xae636f24: return fetch_ds_constructor_disabled_feature (T); break;
  default: assert (0); return NULL;
  }
}
struct tl_ds_disabled_feature *fetch_ds_type_bare_disabled_feature (struct paramed_type *T) {
  return fetch_ds_constructor_disabled_feature (T);
  assert (0);
  return NULL;
}
struct tl_ds_document *fetch_ds_type_document (struct paramed_type *T) {
  assert (in_remaining () >= 4);
  int magic = fetch_int ();
  switch (magic) {
  case 0x36f8c871: return fetch_ds_constructor_document_empty (T); break;
  case 0xf9a39f4f: return fetch_ds_constructor_document (T); break;
  case 0x9efc6326: return fetch_ds_constructor_document_l19 (T); break;
  default: assert (0); return NULL;
  }
}
struct tl_ds_document *fetch_ds_type_bare_document (struct paramed_type *T) {
  int *save_in_ptr = in_ptr;
  if (skip_constructor_document_empty (T) >= 0) { in_ptr = save_in_ptr; return fetch_ds_constructor_document_empty (T); }
  if (skip_constructor_document (T) >= 0) { in_ptr = save_in_ptr; return fetch_ds_constructor_document (T); }
  if (skip_constructor_document_l19 (T) >= 0) { in_ptr = save_in_ptr; return fetch_ds_constructor_document_l19 (T); }
  assert (0);
  return NULL;
}
struct tl_ds_document_attribute *fetch_ds_type_document_attribute (struct paramed_type *T) {
  assert (in_remaining () >= 4);
  int magic = fetch_int ();
  switch (magic) {
  case 0x6c37c15c: return fetch_ds_constructor_document_attribute_image_size (T); break;
  case 0x11b58939: return fetch_ds_constructor_document_attribute_animated (T); break;
  case 0x994c9882: return fetch_ds_constructor_document_attribute_sticker_l28 (T); break;
  case 0x3a556302: return fetch_ds_constructor_document_attribute_sticker (T); break;
  case 0x5910cccb: return fetch_ds_constructor_document_attribute_video (T); break;
  case 0x051448e5: return fetch_ds_constructor_document_attribute_audio (T); break;
  case 0x15590068: return fetch_ds_constructor_document_attribute_filename (T); break;
  default: assert (0); return NULL;
  }
}
struct tl_ds_document_attribute *fetch_ds_type_bare_document_attribute (struct paramed_type *T) {
  int *save_in_ptr = in_ptr;
  if (skip_constructor_document_attribute_image_size (T) >= 0) { in_ptr = save_in_ptr; return fetch_ds_constructor_document_attribute_image_size (T); }
  if (skip_constructor_document_attribute_animated (T) >= 0) { in_ptr = save_in_ptr; return fetch_ds_constructor_document_attribute_animated (T); }
  if (skip_constructor_document_attribute_sticker_l28 (T) >= 0) { in_ptr = save_in_ptr; return fetch_ds_constructor_document_attribute_sticker_l28 (T); }
  if (skip_constructor_document_attribute_sticker (T) >= 0) { in_ptr = save_in_ptr; return fetch_ds_constructor_document_attribute_sticker (T); }
  if (skip_constructor_document_attribute_video (T) >= 0) { in_ptr = save_in_ptr; return fetch_ds_constructor_document_attribute_video (T); }
  if (skip_constructor_document_attribute_audio (T) >= 0) { in_ptr = save_in_ptr; return fetch_ds_constructor_document_attribute_audio (T); }
  if (skip_constructor_document_attribute_filename (T) >= 0) { in_ptr = save_in_ptr; return fetch_ds_constructor_document_attribute_filename (T); }
  assert (0);
  return NULL;
}
double *fetch_ds_type_double (struct paramed_type *T) {
  assert (in_remaining () >= 4);
  int magic = fetch_int ();
  switch (magic) {
  case 0x2210c154: return fetch_ds_constructor_double (T); break;
  default: assert (0); return NULL;
  }
}
double *fetch_ds_type_bare_double (struct paramed_type *T) {
  return fetch_ds_constructor_double (T);
  assert (0);
  return NULL;
}
struct tl_ds_encrypted_chat *fetch_ds_type_encrypted_chat (struct paramed_type *T) {
  assert (in_remaining () >= 4);
  int magic = fetch_int ();
  switch (magic) {
  case 0xab7ec0a0: return fetch_ds_constructor_encrypted_chat_empty (T); break;
  case 0x3bf703dc: return fetch_ds_constructor_encrypted_chat_waiting (T); break;
  case 0xc878527e: return fetch_ds_constructor_encrypted_chat_requested (T); break;
  case 0xfa56ce36: return fetch_ds_constructor_encrypted_chat (T); break;
  case 0x13d6dd27: return fetch_ds_constructor_encrypted_chat_discarded (T); break;
  default: assert (0); return NULL;
  }
}
struct tl_ds_encrypted_chat *fetch_ds_type_bare_encrypted_chat (struct paramed_type *T) {
  int *save_in_ptr = in_ptr;
  if (skip_constructor_encrypted_chat_empty (T) >= 0) { in_ptr = save_in_ptr; return fetch_ds_constructor_encrypted_chat_empty (T); }
  if (skip_constructor_encrypted_chat_waiting (T) >= 0) { in_ptr = save_in_ptr; return fetch_ds_constructor_encrypted_chat_waiting (T); }
  if (skip_constructor_encrypted_chat_requested (T) >= 0) { in_ptr = save_in_ptr; return fetch_ds_constructor_encrypted_chat_requested (T); }
  if (skip_constructor_encrypted_chat (T) >= 0) { in_ptr = save_in_ptr; return fetch_ds_constructor_encrypted_chat (T); }
  if (skip_constructor_encrypted_chat_discarded (T) >= 0) { in_ptr = save_in_ptr; return fetch_ds_constructor_encrypted_chat_discarded (T); }
  assert (0);
  return NULL;
}
struct tl_ds_encrypted_file *fetch_ds_type_encrypted_file (struct paramed_type *T) {
  assert (in_remaining () >= 4);
  int magic = fetch_int ();
  switch (magic) {
  case 0xc21f497e: return fetch_ds_constructor_encrypted_file_empty (T); break;
  case 0x4a70994c: return fetch_ds_constructor_encrypted_file (T); break;
  default: assert (0); return NULL;
  }
}
struct tl_ds_encrypted_file *fetch_ds_type_bare_encrypted_file (struct paramed_type *T) {
  int *save_in_ptr = in_ptr;
  if (skip_constructor_encrypted_file_empty (T) >= 0) { in_ptr = save_in_ptr; return fetch_ds_constructor_encrypted_file_empty (T); }
  if (skip_constructor_encrypted_file (T) >= 0) { in_ptr = save_in_ptr; return fetch_ds_constructor_encrypted_file (T); }
  assert (0);
  return NULL;
}
struct tl_ds_encrypted_message *fetch_ds_type_encrypted_message (struct paramed_type *T) {
  assert (in_remaining () >= 4);
  int magic = fetch_int ();
  switch (magic) {
  case 0xed18c118: return fetch_ds_constructor_encrypted_message (T); break;
  case 0x23734b06: return fetch_ds_constructor_encrypted_message_service (T); break;
  default: assert (0); return NULL;
  }
}
struct tl_ds_encrypted_message *fetch_ds_type_bare_encrypted_message (struct paramed_type *T) {
  int *save_in_ptr = in_ptr;
  if (skip_constructor_encrypted_message (T) >= 0) { in_ptr = save_in_ptr; return fetch_ds_constructor_encrypted_message (T); }
  if (skip_constructor_encrypted_message_service (T) >= 0) { in_ptr = save_in_ptr; return fetch_ds_constructor_encrypted_message_service (T); }
  assert (0);
  return NULL;
}
struct tl_ds_error *fetch_ds_type_error (struct paramed_type *T) {
  assert (in_remaining () >= 4);
  int magic = fetch_int ();
  switch (magic) {
  case 0xc4b9f9bb: return fetch_ds_constructor_error (T); break;
  default: assert (0); return NULL;
  }
}
struct tl_ds_error *fetch_ds_type_bare_error (struct paramed_type *T) {
  return fetch_ds_constructor_error (T);
  assert (0);
  return NULL;
}
struct tl_ds_exported_chat_invite *fetch_ds_type_exported_chat_invite (struct paramed_type *T) {
  assert (in_remaining () >= 4);
  int magic = fetch_int ();
  switch (magic) {
  case 0x69df3769: return fetch_ds_constructor_chat_invite_empty (T); break;
  case 0xfc2e05bc: return fetch_ds_constructor_chat_invite_exported (T); break;
  default: assert (0); return NULL;
  }
}
struct tl_ds_exported_chat_invite *fetch_ds_type_bare_exported_chat_invite (struct paramed_type *T) {
  int *save_in_ptr = in_ptr;
  if (skip_constructor_chat_invite_empty (T) >= 0) { in_ptr = save_in_ptr; return fetch_ds_constructor_chat_invite_empty (T); }
  if (skip_constructor_chat_invite_exported (T) >= 0) { in_ptr = save_in_ptr; return fetch_ds_constructor_chat_invite_exported (T); }
  assert (0);
  return NULL;
}
struct tl_ds_file_location *fetch_ds_type_file_location (struct paramed_type *T) {
  assert (in_remaining () >= 4);
  int magic = fetch_int ();
  switch (magic) {
  case 0x7c596b46: return fetch_ds_constructor_file_location_unavailable (T); break;
  case 0x53d69076: return fetch_ds_constructor_file_location (T); break;
  default: assert (0); return NULL;
  }
}
struct tl_ds_file_location *fetch_ds_type_bare_file_location (struct paramed_type *T) {
  int *save_in_ptr = in_ptr;
  if (skip_constructor_file_location_unavailable (T) >= 0) { in_ptr = save_in_ptr; return fetch_ds_constructor_file_location_unavailable (T); }
  if (skip_constructor_file_location (T) >= 0) { in_ptr = save_in_ptr; return fetch_ds_constructor_file_location (T); }
  assert (0);
  return NULL;
}
struct tl_ds_geo_chat_message *fetch_ds_type_geo_chat_message (struct paramed_type *T) {
  assert (in_remaining () >= 4);
  int magic = fetch_int ();
  switch (magic) {
  case 0x60311a9b: return fetch_ds_constructor_geo_chat_message_empty (T); break;
  case 0x4505f8e1: return fetch_ds_constructor_geo_chat_message (T); break;
  case 0xd34fa24e: return fetch_ds_constructor_geo_chat_message_service (T); break;
  default: assert (0); return NULL;
  }
}
struct tl_ds_geo_chat_message *fetch_ds_type_bare_geo_chat_message (struct paramed_type *T) {
  int *save_in_ptr = in_ptr;
  if (skip_constructor_geo_chat_message_empty (T) >= 0) { in_ptr = save_in_ptr; return fetch_ds_constructor_geo_chat_message_empty (T); }
  if (skip_constructor_geo_chat_message (T) >= 0) { in_ptr = save_in_ptr; return fetch_ds_constructor_geo_chat_message (T); }
  if (skip_constructor_geo_chat_message_service (T) >= 0) { in_ptr = save_in_ptr; return fetch_ds_constructor_geo_chat_message_service (T); }
  assert (0);
  return NULL;
}
struct tl_ds_geo_point *fetch_ds_type_geo_point (struct paramed_type *T) {
  assert (in_remaining () >= 4);
  int magic = fetch_int ();
  switch (magic) {
  case 0x1117dd5f: return fetch_ds_constructor_geo_point_empty (T); break;
  case 0x2049d70c: return fetch_ds_constructor_geo_point (T); break;
  default: assert (0); return NULL;
  }
}
struct tl_ds_geo_point *fetch_ds_type_bare_geo_point (struct paramed_type *T) {
  int *save_in_ptr = in_ptr;
  if (skip_constructor_geo_point_empty (T) >= 0) { in_ptr = save_in_ptr; return fetch_ds_constructor_geo_point_empty (T); }
  if (skip_constructor_geo_point (T) >= 0) { in_ptr = save_in_ptr; return fetch_ds_constructor_geo_point (T); }
  assert (0);
  return NULL;
}
struct tl_ds_imported_contact *fetch_ds_type_imported_contact (struct paramed_type *T) {
  assert (in_remaining () >= 4);
  int magic = fetch_int ();
  switch (magic) {
  case 0xd0028438: return fetch_ds_constructor_imported_contact (T); break;
  default: assert (0); return NULL;
  }
}
struct tl_ds_imported_contact *fetch_ds_type_bare_imported_contact (struct paramed_type *T) {
  return fetch_ds_constructor_imported_contact (T);
  assert (0);
  return NULL;
}
struct tl_ds_input_app_event *fetch_ds_type_input_app_event (struct paramed_type *T) {
  assert (in_remaining () >= 4);
  int magic = fetch_int ();
  switch (magic) {
  case 0x770656a8: return fetch_ds_constructor_input_app_event (T); break;
  default: assert (0); return NULL;
  }
}
struct tl_ds_input_app_event *fetch_ds_type_bare_input_app_event (struct paramed_type *T) {
  return fetch_ds_constructor_input_app_event (T);
  assert (0);
  return NULL;
}
struct tl_ds_input_audio *fetch_ds_type_input_audio (struct paramed_type *T) {
  assert (in_remaining () >= 4);
  int magic = fetch_int ();
  switch (magic) {
  case 0xd95adc84: return fetch_ds_constructor_input_audio_empty (T); break;
  case 0x77d440ff: return fetch_ds_constructor_input_audio (T); break;
  default: assert (0); return NULL;
  }
}
struct tl_ds_input_audio *fetch_ds_type_bare_input_audio (struct paramed_type *T) {
  int *save_in_ptr = in_ptr;
  if (skip_constructor_input_audio_empty (T) >= 0) { in_ptr = save_in_ptr; return fetch_ds_constructor_input_audio_empty (T); }
  if (skip_constructor_input_audio (T) >= 0) { in_ptr = save_in_ptr; return fetch_ds_constructor_input_audio (T); }
  assert (0);
  return NULL;
}
struct tl_ds_input_chat_photo *fetch_ds_type_input_chat_photo (struct paramed_type *T) {
  assert (in_remaining () >= 4);
  int magic = fetch_int ();
  switch (magic) {
  case 0x1ca48f57: return fetch_ds_constructor_input_chat_photo_empty (T); break;
  case 0x94254732: return fetch_ds_constructor_input_chat_uploaded_photo (T); break;
  case 0xb2e1bf08: return fetch_ds_constructor_input_chat_photo (T); break;
  default: assert (0); return NULL;
  }
}
struct tl_ds_input_chat_photo *fetch_ds_type_bare_input_chat_photo (struct paramed_type *T) {
  int *save_in_ptr = in_ptr;
  if (skip_constructor_input_chat_photo_empty (T) >= 0) { in_ptr = save_in_ptr; return fetch_ds_constructor_input_chat_photo_empty (T); }
  if (skip_constructor_input_chat_uploaded_photo (T) >= 0) { in_ptr = save_in_ptr; return fetch_ds_constructor_input_chat_uploaded_photo (T); }
  if (skip_constructor_input_chat_photo (T) >= 0) { in_ptr = save_in_ptr; return fetch_ds_constructor_input_chat_photo (T); }
  assert (0);
  return NULL;
}
struct tl_ds_input_contact *fetch_ds_type_input_contact (struct paramed_type *T) {
  assert (in_remaining () >= 4);
  int magic = fetch_int ();
  switch (magic) {
  case 0xf392b7f4: return fetch_ds_constructor_input_phone_contact (T); break;
  default: assert (0); return NULL;
  }
}
struct tl_ds_input_contact *fetch_ds_type_bare_input_contact (struct paramed_type *T) {
  return fetch_ds_constructor_input_phone_contact (T);
  assert (0);
  return NULL;
}
struct tl_ds_input_document *fetch_ds_type_input_document (struct paramed_type *T) {
  assert (in_remaining () >= 4);
  int magic = fetch_int ();
  switch (magic) {
  case 0x72f0eaae: return fetch_ds_constructor_input_document_empty (T); break;
  case 0x18798952: return fetch_ds_constructor_input_document (T); break;
  default: assert (0); return NULL;
  }
}
struct tl_ds_input_document *fetch_ds_type_bare_input_document (struct paramed_type *T) {
  int *save_in_ptr = in_ptr;
  if (skip_constructor_input_document_empty (T) >= 0) { in_ptr = save_in_ptr; return fetch_ds_constructor_input_document_empty (T); }
  if (skip_constructor_input_document (T) >= 0) { in_ptr = save_in_ptr; return fetch_ds_constructor_input_document (T); }
  assert (0);
  return NULL;
}
struct tl_ds_input_encrypted_chat *fetch_ds_type_input_encrypted_chat (struct paramed_type *T) {
  assert (in_remaining () >= 4);
  int magic = fetch_int ();
  switch (magic) {
  case 0xf141b5e1: return fetch_ds_constructor_input_encrypted_chat (T); break;
  default: assert (0); return NULL;
  }
}
struct tl_ds_input_encrypted_chat *fetch_ds_type_bare_input_encrypted_chat (struct paramed_type *T) {
  return fetch_ds_constructor_input_encrypted_chat (T);
  assert (0);
  return NULL;
}
struct tl_ds_input_encrypted_file *fetch_ds_type_input_encrypted_file (struct paramed_type *T) {
  assert (in_remaining () >= 4);
  int magic = fetch_int ();
  switch (magic) {
  case 0x1837c364: return fetch_ds_constructor_input_encrypted_file_empty (T); break;
  case 0x64bd0306: return fetch_ds_constructor_input_encrypted_file_uploaded (T); break;
  case 0x5a17b5e5: return fetch_ds_constructor_input_encrypted_file (T); break;
  case 0x2dc173c8: return fetch_ds_constructor_input_encrypted_file_big_uploaded (T); break;
  default: assert (0); return NULL;
  }
}
struct tl_ds_input_encrypted_file *fetch_ds_type_bare_input_encrypted_file (struct paramed_type *T) {
  int *save_in_ptr = in_ptr;
  if (skip_constructor_input_encrypted_file_empty (T) >= 0) { in_ptr = save_in_ptr; return fetch_ds_constructor_input_encrypted_file_empty (T); }
  if (skip_constructor_input_encrypted_file_uploaded (T) >= 0) { in_ptr = save_in_ptr; return fetch_ds_constructor_input_encrypted_file_uploaded (T); }
  if (skip_constructor_input_encrypted_file (T) >= 0) { in_ptr = save_in_ptr; return fetch_ds_constructor_input_encrypted_file (T); }
  if (skip_constructor_input_encrypted_file_big_uploaded (T) >= 0) { in_ptr = save_in_ptr; return fetch_ds_constructor_input_encrypted_file_big_uploaded (T); }
  assert (0);
  return NULL;
}
struct tl_ds_input_file *fetch_ds_type_input_file (struct paramed_type *T) {
  assert (in_remaining () >= 4);
  int magic = fetch_int ();
  switch (magic) {
  case 0xf52ff27f: return fetch_ds_constructor_input_file (T); break;
  case 0xfa4f0bb5: return fetch_ds_constructor_input_file_big (T); break;
  default: assert (0); return NULL;
  }
}
struct tl_ds_input_file *fetch_ds_type_bare_input_file (struct paramed_type *T) {
  int *save_in_ptr = in_ptr;
  if (skip_constructor_input_file (T) >= 0) { in_ptr = save_in_ptr; return fetch_ds_constructor_input_file (T); }
  if (skip_constructor_input_file_big (T) >= 0) { in_ptr = save_in_ptr; return fetch_ds_constructor_input_file_big (T); }
  assert (0);
  return NULL;
}
struct tl_ds_input_file_location *fetch_ds_type_input_file_location (struct paramed_type *T) {
  assert (in_remaining () >= 4);
  int magic = fetch_int ();
  switch (magic) {
  case 0x14637196: return fetch_ds_constructor_input_file_location (T); break;
  case 0x3d0364ec: return fetch_ds_constructor_input_video_file_location (T); break;
  case 0xf5235d55: return fetch_ds_constructor_input_encrypted_file_location (T); break;
  case 0x74dc404d: return fetch_ds_constructor_input_audio_file_location (T); break;
  case 0x4e45abe9: return fetch_ds_constructor_input_document_file_location (T); break;
  default: assert (0); return NULL;
  }
}
struct tl_ds_input_file_location *fetch_ds_type_bare_input_file_location (struct paramed_type *T) {
  int *save_in_ptr = in_ptr;
  if (skip_constructor_input_file_location (T) >= 0) { in_ptr = save_in_ptr; return fetch_ds_constructor_input_file_location (T); }
  if (skip_constructor_input_video_file_location (T) >= 0) { in_ptr = save_in_ptr; return fetch_ds_constructor_input_video_file_location (T); }
  if (skip_constructor_input_encrypted_file_location (T) >= 0) { in_ptr = save_in_ptr; return fetch_ds_constructor_input_encrypted_file_location (T); }
  if (skip_constructor_input_audio_file_location (T) >= 0) { in_ptr = save_in_ptr; return fetch_ds_constructor_input_audio_file_location (T); }
  if (skip_constructor_input_document_file_location (T) >= 0) { in_ptr = save_in_ptr; return fetch_ds_constructor_input_document_file_location (T); }
  assert (0);
  return NULL;
}
struct tl_ds_input_geo_chat *fetch_ds_type_input_geo_chat (struct paramed_type *T) {
  assert (in_remaining () >= 4);
  int magic = fetch_int ();
  switch (magic) {
  case 0x74d456fa: return fetch_ds_constructor_input_geo_chat (T); break;
  default: assert (0); return NULL;
  }
}
struct tl_ds_input_geo_chat *fetch_ds_type_bare_input_geo_chat (struct paramed_type *T) {
  return fetch_ds_constructor_input_geo_chat (T);
  assert (0);
  return NULL;
}
struct tl_ds_input_geo_point *fetch_ds_type_input_geo_point (struct paramed_type *T) {
  assert (in_remaining () >= 4);
  int magic = fetch_int ();
  switch (magic) {
  case 0xe4c123d6: return fetch_ds_constructor_input_geo_point_empty (T); break;
  case 0xf3b7acc9: return fetch_ds_constructor_input_geo_point (T); break;
  default: assert (0); return NULL;
  }
}
struct tl_ds_input_geo_point *fetch_ds_type_bare_input_geo_point (struct paramed_type *T) {
  int *save_in_ptr = in_ptr;
  if (skip_constructor_input_geo_point_empty (T) >= 0) { in_ptr = save_in_ptr; return fetch_ds_constructor_input_geo_point_empty (T); }
  if (skip_constructor_input_geo_point (T) >= 0) { in_ptr = save_in_ptr; return fetch_ds_constructor_input_geo_point (T); }
  assert (0);
  return NULL;
}
struct tl_ds_input_media *fetch_ds_type_input_media (struct paramed_type *T) {
  assert (in_remaining () >= 4);
  int magic = fetch_int ();
  switch (magic) {
  case 0x9664f57f: return fetch_ds_constructor_input_media_empty (T); break;
  case 0xf7aff1c0: return fetch_ds_constructor_input_media_uploaded_photo (T); break;
  case 0xe9bfb4f3: return fetch_ds_constructor_input_media_photo (T); break;
  case 0xf9c44144: return fetch_ds_constructor_input_media_geo_point (T); break;
  case 0xa6e45987: return fetch_ds_constructor_input_media_contact (T); break;
  case 0xe13fd4bc: return fetch_ds_constructor_input_media_uploaded_video (T); break;
  case 0x96fb97dc: return fetch_ds_constructor_input_media_uploaded_thumb_video (T); break;
  case 0x936a4ebd: return fetch_ds_constructor_input_media_video (T); break;
  case 0x4e498cab: return fetch_ds_constructor_input_media_uploaded_audio (T); break;
  case 0x89938781: return fetch_ds_constructor_input_media_audio (T); break;
  case 0xffe76b78: return fetch_ds_constructor_input_media_uploaded_document (T); break;
  case 0x41481486: return fetch_ds_constructor_input_media_uploaded_thumb_document (T); break;
  case 0xd184e841: return fetch_ds_constructor_input_media_document (T); break;
  case 0x2827a81a: return fetch_ds_constructor_input_media_venue (T); break;
  default: assert (0); return NULL;
  }
}
struct tl_ds_input_media *fetch_ds_type_bare_input_media (struct paramed_type *T) {
  int *save_in_ptr = in_ptr;
  if (skip_constructor_input_media_empty (T) >= 0) { in_ptr = save_in_ptr; return fetch_ds_constructor_input_media_empty (T); }
  if (skip_constructor_input_media_uploaded_photo (T) >= 0) { in_ptr = save_in_ptr; return fetch_ds_constructor_input_media_uploaded_photo (T); }
  if (skip_constructor_input_media_photo (T) >= 0) { in_ptr = save_in_ptr; return fetch_ds_constructor_input_media_photo (T); }
  if (skip_constructor_input_media_geo_point (T) >= 0) { in_ptr = save_in_ptr; return fetch_ds_constructor_input_media_geo_point (T); }
  if (skip_constructor_input_media_contact (T) >= 0) { in_ptr = save_in_ptr; return fetch_ds_constructor_input_media_contact (T); }
  if (skip_constructor_input_media_uploaded_video (T) >= 0) { in_ptr = save_in_ptr; return fetch_ds_constructor_input_media_uploaded_video (T); }
  if (skip_constructor_input_media_uploaded_thumb_video (T) >= 0) { in_ptr = save_in_ptr; return fetch_ds_constructor_input_media_uploaded_thumb_video (T); }
  if (skip_constructor_input_media_video (T) >= 0) { in_ptr = save_in_ptr; return fetch_ds_constructor_input_media_video (T); }
  if (skip_constructor_input_media_uploaded_audio (T) >= 0) { in_ptr = save_in_ptr; return fetch_ds_constructor_input_media_uploaded_audio (T); }
  if (skip_constructor_input_media_audio (T) >= 0) { in_ptr = save_in_ptr; return fetch_ds_constructor_input_media_audio (T); }
  if (skip_constructor_input_media_uploaded_document (T) >= 0) { in_ptr = save_in_ptr; return fetch_ds_constructor_input_media_uploaded_document (T); }
  if (skip_constructor_input_media_uploaded_thumb_document (T) >= 0) { in_ptr = save_in_ptr; return fetch_ds_constructor_input_media_uploaded_thumb_document (T); }
  if (skip_constructor_input_media_document (T) >= 0) { in_ptr = save_in_ptr; return fetch_ds_constructor_input_media_document (T); }
  if (skip_constructor_input_media_venue (T) >= 0) { in_ptr = save_in_ptr; return fetch_ds_constructor_input_media_venue (T); }
  assert (0);
  return NULL;
}
struct tl_ds_input_notify_peer *fetch_ds_type_input_notify_peer (struct paramed_type *T) {
  assert (in_remaining () >= 4);
  int magic = fetch_int ();
  switch (magic) {
  case 0xb8bc5b0c: return fetch_ds_constructor_input_notify_peer (T); break;
  case 0x193b4417: return fetch_ds_constructor_input_notify_users (T); break;
  case 0x4a95e84e: return fetch_ds_constructor_input_notify_chats (T); break;
  case 0xa429b886: return fetch_ds_constructor_input_notify_all (T); break;
  case 0x4d8ddec8: return fetch_ds_constructor_input_notify_geo_chat_peer (T); break;
  default: assert (0); return NULL;
  }
}
struct tl_ds_input_notify_peer *fetch_ds_type_bare_input_notify_peer (struct paramed_type *T) {
  int *save_in_ptr = in_ptr;
  if (skip_constructor_input_notify_peer (T) >= 0) { in_ptr = save_in_ptr; return fetch_ds_constructor_input_notify_peer (T); }
  if (skip_constructor_input_notify_users (T) >= 0) { in_ptr = save_in_ptr; return fetch_ds_constructor_input_notify_users (T); }
  if (skip_constructor_input_notify_chats (T) >= 0) { in_ptr = save_in_ptr; return fetch_ds_constructor_input_notify_chats (T); }
  if (skip_constructor_input_notify_all (T) >= 0) { in_ptr = save_in_ptr; return fetch_ds_constructor_input_notify_all (T); }
  if (skip_constructor_input_notify_geo_chat_peer (T) >= 0) { in_ptr = save_in_ptr; return fetch_ds_constructor_input_notify_geo_chat_peer (T); }
  assert (0);
  return NULL;
}
struct tl_ds_input_peer *fetch_ds_type_input_peer (struct paramed_type *T) {
  assert (in_remaining () >= 4);
  int magic = fetch_int ();
  switch (magic) {
  case 0x7f3b18ea: return fetch_ds_constructor_input_peer_empty (T); break;
  case 0x7da07ec9: return fetch_ds_constructor_input_peer_self (T); break;
  case 0x1023dbe8: return fetch_ds_constructor_input_peer_contact (T); break;
  case 0x9b447325: return fetch_ds_constructor_input_peer_foreign (T); break;
  case 0x179be863: return fetch_ds_constructor_input_peer_chat (T); break;
  default: assert (0); return NULL;
  }
}
struct tl_ds_input_peer *fetch_ds_type_bare_input_peer (struct paramed_type *T) {
  int *save_in_ptr = in_ptr;
  if (skip_constructor_input_peer_empty (T) >= 0) { in_ptr = save_in_ptr; return fetch_ds_constructor_input_peer_empty (T); }
  if (skip_constructor_input_peer_self (T) >= 0) { in_ptr = save_in_ptr; return fetch_ds_constructor_input_peer_self (T); }
  if (skip_constructor_input_peer_contact (T) >= 0) { in_ptr = save_in_ptr; return fetch_ds_constructor_input_peer_contact (T); }
  if (skip_constructor_input_peer_foreign (T) >= 0) { in_ptr = save_in_ptr; return fetch_ds_constructor_input_peer_foreign (T); }
  if (skip_constructor_input_peer_chat (T) >= 0) { in_ptr = save_in_ptr; return fetch_ds_constructor_input_peer_chat (T); }
  assert (0);
  return NULL;
}
struct tl_ds_input_peer_notify_events *fetch_ds_type_input_peer_notify_events (struct paramed_type *T) {
  assert (in_remaining () >= 4);
  int magic = fetch_int ();
  switch (magic) {
  case 0xf03064d8: return fetch_ds_constructor_input_peer_notify_events_empty (T); break;
  case 0xe86a2c74: return fetch_ds_constructor_input_peer_notify_events_all (T); break;
  default: assert (0); return NULL;
  }
}
struct tl_ds_input_peer_notify_events *fetch_ds_type_bare_input_peer_notify_events (struct paramed_type *T) {
  int *save_in_ptr = in_ptr;
  if (skip_constructor_input_peer_notify_events_empty (T) >= 0) { in_ptr = save_in_ptr; return fetch_ds_constructor_input_peer_notify_events_empty (T); }
  if (skip_constructor_input_peer_notify_events_all (T) >= 0) { in_ptr = save_in_ptr; return fetch_ds_constructor_input_peer_notify_events_all (T); }
  assert (0);
  return NULL;
}
struct tl_ds_input_peer_notify_settings *fetch_ds_type_input_peer_notify_settings (struct paramed_type *T) {
  assert (in_remaining () >= 4);
  int magic = fetch_int ();
  switch (magic) {
  case 0x46a2ce98: return fetch_ds_constructor_input_peer_notify_settings (T); break;
  default: assert (0); return NULL;
  }
}
struct tl_ds_input_peer_notify_settings *fetch_ds_type_bare_input_peer_notify_settings (struct paramed_type *T) {
  return fetch_ds_constructor_input_peer_notify_settings (T);
  assert (0);
  return NULL;
}
struct tl_ds_input_photo *fetch_ds_type_input_photo (struct paramed_type *T) {
  assert (in_remaining () >= 4);
  int magic = fetch_int ();
  switch (magic) {
  case 0x1cd7bf0d: return fetch_ds_constructor_input_photo_empty (T); break;
  case 0xfb95c6c4: return fetch_ds_constructor_input_photo (T); break;
  default: assert (0); return NULL;
  }
}
struct tl_ds_input_photo *fetch_ds_type_bare_input_photo (struct paramed_type *T) {
  int *save_in_ptr = in_ptr;
  if (skip_constructor_input_photo_empty (T) >= 0) { in_ptr = save_in_ptr; return fetch_ds_constructor_input_photo_empty (T); }
  if (skip_constructor_input_photo (T) >= 0) { in_ptr = save_in_ptr; return fetch_ds_constructor_input_photo (T); }
  assert (0);
  return NULL;
}
struct tl_ds_input_photo_crop *fetch_ds_type_input_photo_crop (struct paramed_type *T) {
  assert (in_remaining () >= 4);
  int magic = fetch_int ();
  switch (magic) {
  case 0xade6b004: return fetch_ds_constructor_input_photo_crop_auto (T); break;
  case 0xd9915325: return fetch_ds_constructor_input_photo_crop (T); break;
  default: assert (0); return NULL;
  }
}
struct tl_ds_input_photo_crop *fetch_ds_type_bare_input_photo_crop (struct paramed_type *T) {
  int *save_in_ptr = in_ptr;
  if (skip_constructor_input_photo_crop_auto (T) >= 0) { in_ptr = save_in_ptr; return fetch_ds_constructor_input_photo_crop_auto (T); }
  if (skip_constructor_input_photo_crop (T) >= 0) { in_ptr = save_in_ptr; return fetch_ds_constructor_input_photo_crop (T); }
  assert (0);
  return NULL;
}
struct tl_ds_input_privacy_key *fetch_ds_type_input_privacy_key (struct paramed_type *T) {
  assert (in_remaining () >= 4);
  int magic = fetch_int ();
  switch (magic) {
  case 0x4f96cb18: return fetch_ds_constructor_input_privacy_key_status_timestamp (T); break;
  default: assert (0); return NULL;
  }
}
struct tl_ds_input_privacy_key *fetch_ds_type_bare_input_privacy_key (struct paramed_type *T) {
  return fetch_ds_constructor_input_privacy_key_status_timestamp (T);
  assert (0);
  return NULL;
}
struct tl_ds_input_privacy_rule *fetch_ds_type_input_privacy_rule (struct paramed_type *T) {
  assert (in_remaining () >= 4);
  int magic = fetch_int ();
  switch (magic) {
  case 0x0d09e07b: return fetch_ds_constructor_input_privacy_value_allow_contacts (T); break;
  case 0x184b35ce: return fetch_ds_constructor_input_privacy_value_allow_all (T); break;
  case 0x131cc67f: return fetch_ds_constructor_input_privacy_value_allow_users (T); break;
  case 0x0ba52007: return fetch_ds_constructor_input_privacy_value_disallow_contacts (T); break;
  case 0xd66b66c9: return fetch_ds_constructor_input_privacy_value_disallow_all (T); break;
  case 0x90110467: return fetch_ds_constructor_input_privacy_value_disallow_users (T); break;
  default: assert (0); return NULL;
  }
}
struct tl_ds_input_privacy_rule *fetch_ds_type_bare_input_privacy_rule (struct paramed_type *T) {
  int *save_in_ptr = in_ptr;
  if (skip_constructor_input_privacy_value_allow_contacts (T) >= 0) { in_ptr = save_in_ptr; return fetch_ds_constructor_input_privacy_value_allow_contacts (T); }
  if (skip_constructor_input_privacy_value_allow_all (T) >= 0) { in_ptr = save_in_ptr; return fetch_ds_constructor_input_privacy_value_allow_all (T); }
  if (skip_constructor_input_privacy_value_allow_users (T) >= 0) { in_ptr = save_in_ptr; return fetch_ds_constructor_input_privacy_value_allow_users (T); }
  if (skip_constructor_input_privacy_value_disallow_contacts (T) >= 0) { in_ptr = save_in_ptr; return fetch_ds_constructor_input_privacy_value_disallow_contacts (T); }
  if (skip_constructor_input_privacy_value_disallow_all (T) >= 0) { in_ptr = save_in_ptr; return fetch_ds_constructor_input_privacy_value_disallow_all (T); }
  if (skip_constructor_input_privacy_value_disallow_users (T) >= 0) { in_ptr = save_in_ptr; return fetch_ds_constructor_input_privacy_value_disallow_users (T); }
  assert (0);
  return NULL;
}
struct tl_ds_input_sticker_set *fetch_ds_type_input_sticker_set (struct paramed_type *T) {
  assert (in_remaining () >= 4);
  int magic = fetch_int ();
  switch (magic) {
  case 0xffb62b95: return fetch_ds_constructor_input_sticker_set_empty (T); break;
  case 0x9de7a269: return fetch_ds_constructor_input_sticker_set_i_d (T); break;
  case 0x861cc8a0: return fetch_ds_constructor_input_sticker_set_short_name (T); break;
  default: assert (0); return NULL;
  }
}
struct tl_ds_input_sticker_set *fetch_ds_type_bare_input_sticker_set (struct paramed_type *T) {
  int *save_in_ptr = in_ptr;
  if (skip_constructor_input_sticker_set_empty (T) >= 0) { in_ptr = save_in_ptr; return fetch_ds_constructor_input_sticker_set_empty (T); }
  if (skip_constructor_input_sticker_set_i_d (T) >= 0) { in_ptr = save_in_ptr; return fetch_ds_constructor_input_sticker_set_i_d (T); }
  if (skip_constructor_input_sticker_set_short_name (T) >= 0) { in_ptr = save_in_ptr; return fetch_ds_constructor_input_sticker_set_short_name (T); }
  assert (0);
  return NULL;
}
struct tl_ds_input_user *fetch_ds_type_input_user (struct paramed_type *T) {
  assert (in_remaining () >= 4);
  int magic = fetch_int ();
  switch (magic) {
  case 0xb98886cf: return fetch_ds_constructor_input_user_empty (T); break;
  case 0xf7c1b13f: return fetch_ds_constructor_input_user_self (T); break;
  case 0x86e94f65: return fetch_ds_constructor_input_user_contact (T); break;
  case 0x655e74ff: return fetch_ds_constructor_input_user_foreign (T); break;
  default: assert (0); return NULL;
  }
}
struct tl_ds_input_user *fetch_ds_type_bare_input_user (struct paramed_type *T) {
  int *save_in_ptr = in_ptr;
  if (skip_constructor_input_user_empty (T) >= 0) { in_ptr = save_in_ptr; return fetch_ds_constructor_input_user_empty (T); }
  if (skip_constructor_input_user_self (T) >= 0) { in_ptr = save_in_ptr; return fetch_ds_constructor_input_user_self (T); }
  if (skip_constructor_input_user_contact (T) >= 0) { in_ptr = save_in_ptr; return fetch_ds_constructor_input_user_contact (T); }
  if (skip_constructor_input_user_foreign (T) >= 0) { in_ptr = save_in_ptr; return fetch_ds_constructor_input_user_foreign (T); }
  assert (0);
  return NULL;
}
struct tl_ds_input_video *fetch_ds_type_input_video (struct paramed_type *T) {
  assert (in_remaining () >= 4);
  int magic = fetch_int ();
  switch (magic) {
  case 0x5508ec75: return fetch_ds_constructor_input_video_empty (T); break;
  case 0xee579652: return fetch_ds_constructor_input_video (T); break;
  default: assert (0); return NULL;
  }
}
struct tl_ds_input_video *fetch_ds_type_bare_input_video (struct paramed_type *T) {
  int *save_in_ptr = in_ptr;
  if (skip_constructor_input_video_empty (T) >= 0) { in_ptr = save_in_ptr; return fetch_ds_constructor_input_video_empty (T); }
  if (skip_constructor_input_video (T) >= 0) { in_ptr = save_in_ptr; return fetch_ds_constructor_input_video (T); }
  assert (0);
  return NULL;
}
int *fetch_ds_type_int (struct paramed_type *T) {
  assert (in_remaining () >= 4);
  int magic = fetch_int ();
  switch (magic) {
  case 0xa8509bda: return fetch_ds_constructor_int (T); break;
  default: assert (0); return NULL;
  }
}
int *fetch_ds_type_bare_int (struct paramed_type *T) {
  return fetch_ds_constructor_int (T);
  assert (0);
  return NULL;
}
struct tl_ds_int128 *fetch_ds_type_int128 (struct paramed_type *T) {
  assert (in_remaining () >= 4);
  int magic = fetch_int ();
  switch (magic) {
  case 0x7d36c439: return fetch_ds_constructor_int128 (T); break;
  default: assert (0); return NULL;
  }
}
struct tl_ds_int128 *fetch_ds_type_bare_int128 (struct paramed_type *T) {
  return fetch_ds_constructor_int128 (T);
  assert (0);
  return NULL;
}
struct tl_ds_int256 *fetch_ds_type_int256 (struct paramed_type *T) {
  assert (in_remaining () >= 4);
  int magic = fetch_int ();
  switch (magic) {
  case 0xf2c798b3: return fetch_ds_constructor_int256 (T); break;
  default: assert (0); return NULL;
  }
}
struct tl_ds_int256 *fetch_ds_type_bare_int256 (struct paramed_type *T) {
  return fetch_ds_constructor_int256 (T);
  assert (0);
  return NULL;
}
struct tl_ds_keyboard_button *fetch_ds_type_keyboard_button (struct paramed_type *T) {
  assert (in_remaining () >= 4);
  int magic = fetch_int ();
  switch (magic) {
  case 0xa2fa4880: return fetch_ds_constructor_keyboard_button (T); break;
  default: assert (0); return NULL;
  }
}
struct tl_ds_keyboard_button *fetch_ds_type_bare_keyboard_button (struct paramed_type *T) {
  return fetch_ds_constructor_keyboard_button (T);
  assert (0);
  return NULL;
}
struct tl_ds_keyboard_button_row *fetch_ds_type_keyboard_button_row (struct paramed_type *T) {
  assert (in_remaining () >= 4);
  int magic = fetch_int ();
  switch (magic) {
  case 0x77608b83: return fetch_ds_constructor_keyboard_button_row (T); break;
  default: assert (0); return NULL;
  }
}
struct tl_ds_keyboard_button_row *fetch_ds_type_bare_keyboard_button_row (struct paramed_type *T) {
  return fetch_ds_constructor_keyboard_button_row (T);
  assert (0);
  return NULL;
}
long long *fetch_ds_type_long (struct paramed_type *T) {
  assert (in_remaining () >= 4);
  int magic = fetch_int ();
  switch (magic) {
  case 0x22076cba: return fetch_ds_constructor_long (T); break;
  default: assert (0); return NULL;
  }
}
long long *fetch_ds_type_bare_long (struct paramed_type *T) {
  return fetch_ds_constructor_long (T);
  assert (0);
  return NULL;
}
struct tl_ds_message *fetch_ds_type_message (struct paramed_type *T) {
  assert (in_remaining () >= 4);
  int magic = fetch_int ();
  switch (magic) {
  case 0x83e5de54: return fetch_ds_constructor_message_empty (T); break;
  case 0xc3060325: return fetch_ds_constructor_message (T); break;
  case 0x1d86f70e: return fetch_ds_constructor_message_service (T); break;
  default: assert (0); return NULL;
  }
}
struct tl_ds_message *fetch_ds_type_bare_message (struct paramed_type *T) {
  int *save_in_ptr = in_ptr;
  if (skip_constructor_message_empty (T) >= 0) { in_ptr = save_in_ptr; return fetch_ds_constructor_message_empty (T); }
  if (skip_constructor_message (T) >= 0) { in_ptr = save_in_ptr; return fetch_ds_constructor_message (T); }
  if (skip_constructor_message_service (T) >= 0) { in_ptr = save_in_ptr; return fetch_ds_constructor_message_service (T); }
  assert (0);
  return NULL;
}
struct tl_ds_message_action *fetch_ds_type_message_action (struct paramed_type *T) {
  assert (in_remaining () >= 4);
  int magic = fetch_int ();
  switch (magic) {
  case 0xb6aef7b0: return fetch_ds_constructor_message_action_empty (T); break;
  case 0xa6638b9a: return fetch_ds_constructor_message_action_chat_create (T); break;
  case 0xb5a1ce5a: return fetch_ds_constructor_message_action_chat_edit_title (T); break;
  case 0x7fcb13a8: return fetch_ds_constructor_message_action_chat_edit_photo (T); break;
  case 0x95e3fbef: return fetch_ds_constructor_message_action_chat_delete_photo (T); break;
  case 0x5e3cfc4b: return fetch_ds_constructor_message_action_chat_add_user (T); break;
  case 0xb2ae9b0c: return fetch_ds_constructor_message_action_chat_delete_user (T); break;
  case 0x6f038ebc: return fetch_ds_constructor_message_action_geo_chat_create (T); break;
  case 0x0c7d53de: return fetch_ds_constructor_message_action_geo_chat_checkin (T); break;
  case 0xf89cf5e8: return fetch_ds_constructor_message_action_chat_joined_by_link (T); break;
  default: assert (0); return NULL;
  }
}
struct tl_ds_message_action *fetch_ds_type_bare_message_action (struct paramed_type *T) {
  int *save_in_ptr = in_ptr;
  if (skip_constructor_message_action_empty (T) >= 0) { in_ptr = save_in_ptr; return fetch_ds_constructor_message_action_empty (T); }
  if (skip_constructor_message_action_chat_create (T) >= 0) { in_ptr = save_in_ptr; return fetch_ds_constructor_message_action_chat_create (T); }
  if (skip_constructor_message_action_chat_edit_title (T) >= 0) { in_ptr = save_in_ptr; return fetch_ds_constructor_message_action_chat_edit_title (T); }
  if (skip_constructor_message_action_chat_edit_photo (T) >= 0) { in_ptr = save_in_ptr; return fetch_ds_constructor_message_action_chat_edit_photo (T); }
  if (skip_constructor_message_action_chat_delete_photo (T) >= 0) { in_ptr = save_in_ptr; return fetch_ds_constructor_message_action_chat_delete_photo (T); }
  if (skip_constructor_message_action_chat_add_user (T) >= 0) { in_ptr = save_in_ptr; return fetch_ds_constructor_message_action_chat_add_user (T); }
  if (skip_constructor_message_action_chat_delete_user (T) >= 0) { in_ptr = save_in_ptr; return fetch_ds_constructor_message_action_chat_delete_user (T); }
  if (skip_constructor_message_action_geo_chat_create (T) >= 0) { in_ptr = save_in_ptr; return fetch_ds_constructor_message_action_geo_chat_create (T); }
  if (skip_constructor_message_action_geo_chat_checkin (T) >= 0) { in_ptr = save_in_ptr; return fetch_ds_constructor_message_action_geo_chat_checkin (T); }
  if (skip_constructor_message_action_chat_joined_by_link (T) >= 0) { in_ptr = save_in_ptr; return fetch_ds_constructor_message_action_chat_joined_by_link (T); }
  assert (0);
  return NULL;
}
struct tl_ds_message_media *fetch_ds_type_message_media (struct paramed_type *T) {
  assert (in_remaining () >= 4);
  int magic = fetch_int ();
  switch (magic) {
  case 0x3ded6320: return fetch_ds_constructor_message_media_empty (T); break;
  case 0x3d8ce53d: return fetch_ds_constructor_message_media_photo (T); break;
  case 0x5bcf1675: return fetch_ds_constructor_message_media_video (T); break;
  case 0x56e0d474: return fetch_ds_constructor_message_media_geo (T); break;
  case 0x5e7d2f39: return fetch_ds_constructor_message_media_contact (T); break;
  case 0x9f84f49e: return fetch_ds_constructor_message_media_unsupported (T); break;
  case 0x2fda2204: return fetch_ds_constructor_message_media_document (T); break;
  case 0xc6b68300: return fetch_ds_constructor_message_media_audio (T); break;
  case 0xa32dd600: return fetch_ds_constructor_message_media_web_page (T); break;
  case 0x7912b71f: return fetch_ds_constructor_message_media_venue (T); break;
  case 0xc8c45a2a: return fetch_ds_constructor_message_media_photo_l27 (T); break;
  case 0xa2d24290: return fetch_ds_constructor_message_media_video_l27 (T); break;
  default: assert (0); return NULL;
  }
}
struct tl_ds_message_media *fetch_ds_type_bare_message_media (struct paramed_type *T) {
  int *save_in_ptr = in_ptr;
  if (skip_constructor_message_media_empty (T) >= 0) { in_ptr = save_in_ptr; return fetch_ds_constructor_message_media_empty (T); }
  if (skip_constructor_message_media_photo (T) >= 0) { in_ptr = save_in_ptr; return fetch_ds_constructor_message_media_photo (T); }
  if (skip_constructor_message_media_video (T) >= 0) { in_ptr = save_in_ptr; return fetch_ds_constructor_message_media_video (T); }
  if (skip_constructor_message_media_geo (T) >= 0) { in_ptr = save_in_ptr; return fetch_ds_constructor_message_media_geo (T); }
  if (skip_constructor_message_media_contact (T) >= 0) { in_ptr = save_in_ptr; return fetch_ds_constructor_message_media_contact (T); }
  if (skip_constructor_message_media_unsupported (T) >= 0) { in_ptr = save_in_ptr; return fetch_ds_constructor_message_media_unsupported (T); }
  if (skip_constructor_message_media_document (T) >= 0) { in_ptr = save_in_ptr; return fetch_ds_constructor_message_media_document (T); }
  if (skip_constructor_message_media_audio (T) >= 0) { in_ptr = save_in_ptr; return fetch_ds_constructor_message_media_audio (T); }
  if (skip_constructor_message_media_web_page (T) >= 0) { in_ptr = save_in_ptr; return fetch_ds_constructor_message_media_web_page (T); }
  if (skip_constructor_message_media_venue (T) >= 0) { in_ptr = save_in_ptr; return fetch_ds_constructor_message_media_venue (T); }
  if (skip_constructor_message_media_photo_l27 (T) >= 0) { in_ptr = save_in_ptr; return fetch_ds_constructor_message_media_photo_l27 (T); }
  if (skip_constructor_message_media_video_l27 (T) >= 0) { in_ptr = save_in_ptr; return fetch_ds_constructor_message_media_video_l27 (T); }
  assert (0);
  return NULL;
}
struct tl_ds_messages_filter *fetch_ds_type_messages_filter (struct paramed_type *T) {
  assert (in_remaining () >= 4);
  int magic = fetch_int ();
  switch (magic) {
  case 0x57e2f66c: return fetch_ds_constructor_input_messages_filter_empty (T); break;
  case 0x9609a51c: return fetch_ds_constructor_input_messages_filter_photos (T); break;
  case 0x9fc00e65: return fetch_ds_constructor_input_messages_filter_video (T); break;
  case 0x56e9f0e4: return fetch_ds_constructor_input_messages_filter_photo_video (T); break;
  case 0xd95e73bb: return fetch_ds_constructor_input_messages_filter_photo_video_documents (T); break;
  case 0x9eddf188: return fetch_ds_constructor_input_messages_filter_document (T); break;
  case 0xcfc87522: return fetch_ds_constructor_input_messages_filter_audio (T); break;
  default: assert (0); return NULL;
  }
}
struct tl_ds_messages_filter *fetch_ds_type_bare_messages_filter (struct paramed_type *T) {
  int *save_in_ptr = in_ptr;
  if (skip_constructor_input_messages_filter_empty (T) >= 0) { in_ptr = save_in_ptr; return fetch_ds_constructor_input_messages_filter_empty (T); }
  if (skip_constructor_input_messages_filter_photos (T) >= 0) { in_ptr = save_in_ptr; return fetch_ds_constructor_input_messages_filter_photos (T); }
  if (skip_constructor_input_messages_filter_video (T) >= 0) { in_ptr = save_in_ptr; return fetch_ds_constructor_input_messages_filter_video (T); }
  if (skip_constructor_input_messages_filter_photo_video (T) >= 0) { in_ptr = save_in_ptr; return fetch_ds_constructor_input_messages_filter_photo_video (T); }
  if (skip_constructor_input_messages_filter_photo_video_documents (T) >= 0) { in_ptr = save_in_ptr; return fetch_ds_constructor_input_messages_filter_photo_video_documents (T); }
  if (skip_constructor_input_messages_filter_document (T) >= 0) { in_ptr = save_in_ptr; return fetch_ds_constructor_input_messages_filter_document (T); }
  if (skip_constructor_input_messages_filter_audio (T) >= 0) { in_ptr = save_in_ptr; return fetch_ds_constructor_input_messages_filter_audio (T); }
  assert (0);
  return NULL;
}
struct tl_ds_nearest_dc *fetch_ds_type_nearest_dc (struct paramed_type *T) {
  assert (in_remaining () >= 4);
  int magic = fetch_int ();
  switch (magic) {
  case 0x8e1a1775: return fetch_ds_constructor_nearest_dc (T); break;
  default: assert (0); return NULL;
  }
}
struct tl_ds_nearest_dc *fetch_ds_type_bare_nearest_dc (struct paramed_type *T) {
  return fetch_ds_constructor_nearest_dc (T);
  assert (0);
  return NULL;
}
struct tl_ds_notify_peer *fetch_ds_type_notify_peer (struct paramed_type *T) {
  assert (in_remaining () >= 4);
  int magic = fetch_int ();
  switch (magic) {
  case 0x9fd40bd8: return fetch_ds_constructor_notify_peer (T); break;
  case 0xb4c83b4c: return fetch_ds_constructor_notify_users (T); break;
  case 0xc007cec3: return fetch_ds_constructor_notify_chats (T); break;
  case 0x74d07c60: return fetch_ds_constructor_notify_all (T); break;
  default: assert (0); return NULL;
  }
}
struct tl_ds_notify_peer *fetch_ds_type_bare_notify_peer (struct paramed_type *T) {
  int *save_in_ptr = in_ptr;
  if (skip_constructor_notify_peer (T) >= 0) { in_ptr = save_in_ptr; return fetch_ds_constructor_notify_peer (T); }
  if (skip_constructor_notify_users (T) >= 0) { in_ptr = save_in_ptr; return fetch_ds_constructor_notify_users (T); }
  if (skip_constructor_notify_chats (T) >= 0) { in_ptr = save_in_ptr; return fetch_ds_constructor_notify_chats (T); }
  if (skip_constructor_notify_all (T) >= 0) { in_ptr = save_in_ptr; return fetch_ds_constructor_notify_all (T); }
  assert (0);
  return NULL;
}
struct tl_ds_null *fetch_ds_type_null (struct paramed_type *T) {
  assert (in_remaining () >= 4);
  int magic = fetch_int ();
  switch (magic) {
  case 0x56730bcc: return fetch_ds_constructor_null (T); break;
  default: assert (0); return NULL;
  }
}
struct tl_ds_null *fetch_ds_type_bare_null (struct paramed_type *T) {
  return fetch_ds_constructor_null (T);
  assert (0);
  return NULL;
}
struct tl_ds_p_q_inner_data *fetch_ds_type_p_q_inner_data (struct paramed_type *T) {
  assert (in_remaining () >= 4);
  int magic = fetch_int ();
  switch (magic) {
  case 0x83c95aec: return fetch_ds_constructor_p_q_inner_data (T); break;
  case 0x3c6a84d4: return fetch_ds_constructor_p_q_inner_data_temp (T); break;
  default: assert (0); return NULL;
  }
}
struct tl_ds_p_q_inner_data *fetch_ds_type_bare_p_q_inner_data (struct paramed_type *T) {
  int *save_in_ptr = in_ptr;
  if (skip_constructor_p_q_inner_data (T) >= 0) { in_ptr = save_in_ptr; return fetch_ds_constructor_p_q_inner_data (T); }
  if (skip_constructor_p_q_inner_data_temp (T) >= 0) { in_ptr = save_in_ptr; return fetch_ds_constructor_p_q_inner_data_temp (T); }
  assert (0);
  return NULL;
}
struct tl_ds_peer *fetch_ds_type_peer (struct paramed_type *T) {
  assert (in_remaining () >= 4);
  int magic = fetch_int ();
  switch (magic) {
  case 0x9db1bc6d: return fetch_ds_constructor_peer_user (T); break;
  case 0xbad0e5bb: return fetch_ds_constructor_peer_chat (T); break;
  default: assert (0); return NULL;
  }
}
struct tl_ds_peer *fetch_ds_type_bare_peer (struct paramed_type *T) {
  int *save_in_ptr = in_ptr;
  if (skip_constructor_peer_user (T) >= 0) { in_ptr = save_in_ptr; return fetch_ds_constructor_peer_user (T); }
  if (skip_constructor_peer_chat (T) >= 0) { in_ptr = save_in_ptr; return fetch_ds_constructor_peer_chat (T); }
  assert (0);
  return NULL;
}
struct tl_ds_peer_notify_events *fetch_ds_type_peer_notify_events (struct paramed_type *T) {
  assert (in_remaining () >= 4);
  int magic = fetch_int ();
  switch (magic) {
  case 0xadd53cb3: return fetch_ds_constructor_peer_notify_events_empty (T); break;
  case 0x6d1ded88: return fetch_ds_constructor_peer_notify_events_all (T); break;
  default: assert (0); return NULL;
  }
}
struct tl_ds_peer_notify_events *fetch_ds_type_bare_peer_notify_events (struct paramed_type *T) {
  int *save_in_ptr = in_ptr;
  if (skip_constructor_peer_notify_events_empty (T) >= 0) { in_ptr = save_in_ptr; return fetch_ds_constructor_peer_notify_events_empty (T); }
  if (skip_constructor_peer_notify_events_all (T) >= 0) { in_ptr = save_in_ptr; return fetch_ds_constructor_peer_notify_events_all (T); }
  assert (0);
  return NULL;
}
struct tl_ds_peer_notify_settings *fetch_ds_type_peer_notify_settings (struct paramed_type *T) {
  assert (in_remaining () >= 4);
  int magic = fetch_int ();
  switch (magic) {
  case 0x70a68512: return fetch_ds_constructor_peer_notify_settings_empty (T); break;
  case 0x8d5e11ee: return fetch_ds_constructor_peer_notify_settings (T); break;
  default: assert (0); return NULL;
  }
}
struct tl_ds_peer_notify_settings *fetch_ds_type_bare_peer_notify_settings (struct paramed_type *T) {
  int *save_in_ptr = in_ptr;
  if (skip_constructor_peer_notify_settings_empty (T) >= 0) { in_ptr = save_in_ptr; return fetch_ds_constructor_peer_notify_settings_empty (T); }
  if (skip_constructor_peer_notify_settings (T) >= 0) { in_ptr = save_in_ptr; return fetch_ds_constructor_peer_notify_settings (T); }
  assert (0);
  return NULL;
}
struct tl_ds_photo *fetch_ds_type_photo (struct paramed_type *T) {
  assert (in_remaining () >= 4);
  int magic = fetch_int ();
  switch (magic) {
  case 0x2331b22d: return fetch_ds_constructor_photo_empty (T); break;
  case 0xc3838076: return fetch_ds_constructor_photo (T); break;
  case 0x22b56751: return fetch_ds_constructor_photo_l27 (T); break;
  default: assert (0); return NULL;
  }
}
struct tl_ds_photo *fetch_ds_type_bare_photo (struct paramed_type *T) {
  int *save_in_ptr = in_ptr;
  if (skip_constructor_photo_empty (T) >= 0) { in_ptr = save_in_ptr; return fetch_ds_constructor_photo_empty (T); }
  if (skip_constructor_photo (T) >= 0) { in_ptr = save_in_ptr; return fetch_ds_constructor_photo (T); }
  if (skip_constructor_photo_l27 (T) >= 0) { in_ptr = save_in_ptr; return fetch_ds_constructor_photo_l27 (T); }
  assert (0);
  return NULL;
}
struct tl_ds_photo_size *fetch_ds_type_photo_size (struct paramed_type *T) {
  assert (in_remaining () >= 4);
  int magic = fetch_int ();
  switch (magic) {
  case 0x0e17e23c: return fetch_ds_constructor_photo_size_empty (T); break;
  case 0x77bfb61b: return fetch_ds_constructor_photo_size (T); break;
  case 0xe9a734fa: return fetch_ds_constructor_photo_cached_size (T); break;
  default: assert (0); return NULL;
  }
}
struct tl_ds_photo_size *fetch_ds_type_bare_photo_size (struct paramed_type *T) {
  int *save_in_ptr = in_ptr;
  if (skip_constructor_photo_size_empty (T) >= 0) { in_ptr = save_in_ptr; return fetch_ds_constructor_photo_size_empty (T); }
  if (skip_constructor_photo_size (T) >= 0) { in_ptr = save_in_ptr; return fetch_ds_constructor_photo_size (T); }
  if (skip_constructor_photo_cached_size (T) >= 0) { in_ptr = save_in_ptr; return fetch_ds_constructor_photo_cached_size (T); }
  assert (0);
  return NULL;
}
struct tl_ds_privacy_key *fetch_ds_type_privacy_key (struct paramed_type *T) {
  assert (in_remaining () >= 4);
  int magic = fetch_int ();
  switch (magic) {
  case 0xbc2eab30: return fetch_ds_constructor_privacy_key_status_timestamp (T); break;
  default: assert (0); return NULL;
  }
}
struct tl_ds_privacy_key *fetch_ds_type_bare_privacy_key (struct paramed_type *T) {
  return fetch_ds_constructor_privacy_key_status_timestamp (T);
  assert (0);
  return NULL;
}
struct tl_ds_privacy_rule *fetch_ds_type_privacy_rule (struct paramed_type *T) {
  assert (in_remaining () >= 4);
  int magic = fetch_int ();
  switch (magic) {
  case 0xfffe1bac: return fetch_ds_constructor_privacy_value_allow_contacts (T); break;
  case 0x65427b82: return fetch_ds_constructor_privacy_value_allow_all (T); break;
  case 0x4d5bbe0c: return fetch_ds_constructor_privacy_value_allow_users (T); break;
  case 0xf888fa1a: return fetch_ds_constructor_privacy_value_disallow_contacts (T); break;
  case 0x8b73e763: return fetch_ds_constructor_privacy_value_disallow_all (T); break;
  case 0x0c7f49b7: return fetch_ds_constructor_privacy_value_disallow_users (T); break;
  default: assert (0); return NULL;
  }
}
struct tl_ds_privacy_rule *fetch_ds_type_bare_privacy_rule (struct paramed_type *T) {
  int *save_in_ptr = in_ptr;
  if (skip_constructor_privacy_value_allow_contacts (T) >= 0) { in_ptr = save_in_ptr; return fetch_ds_constructor_privacy_value_allow_contacts (T); }
  if (skip_constructor_privacy_value_allow_all (T) >= 0) { in_ptr = save_in_ptr; return fetch_ds_constructor_privacy_value_allow_all (T); }
  if (skip_constructor_privacy_value_allow_users (T) >= 0) { in_ptr = save_in_ptr; return fetch_ds_constructor_privacy_value_allow_users (T); }
  if (skip_constructor_privacy_value_disallow_contacts (T) >= 0) { in_ptr = save_in_ptr; return fetch_ds_constructor_privacy_value_disallow_contacts (T); }
  if (skip_constructor_privacy_value_disallow_all (T) >= 0) { in_ptr = save_in_ptr; return fetch_ds_constructor_privacy_value_disallow_all (T); }
  if (skip_constructor_privacy_value_disallow_users (T) >= 0) { in_ptr = save_in_ptr; return fetch_ds_constructor_privacy_value_disallow_users (T); }
  assert (0);
  return NULL;
}
struct tl_ds_received_notify_message *fetch_ds_type_received_notify_message (struct paramed_type *T) {
  assert (in_remaining () >= 4);
  int magic = fetch_int ();
  switch (magic) {
  case 0xa384b779: return fetch_ds_constructor_received_notify_message (T); break;
  default: assert (0); return NULL;
  }
}
struct tl_ds_received_notify_message *fetch_ds_type_bare_received_notify_message (struct paramed_type *T) {
  return fetch_ds_constructor_received_notify_message (T);
  assert (0);
  return NULL;
}
struct tl_ds_reply_markup *fetch_ds_type_reply_markup (struct paramed_type *T) {
  assert (in_remaining () >= 4);
  int magic = fetch_int ();
  switch (magic) {
  case 0xa03e5b85: return fetch_ds_constructor_reply_keyboard_hide (T); break;
  case 0xf4108aa0: return fetch_ds_constructor_reply_keyboard_force_reply (T); break;
  case 0x3502758c: return fetch_ds_constructor_reply_keyboard_markup (T); break;
  default: assert (0); return NULL;
  }
}
struct tl_ds_reply_markup *fetch_ds_type_bare_reply_markup (struct paramed_type *T) {
  int *save_in_ptr = in_ptr;
  if (skip_constructor_reply_keyboard_hide (T) >= 0) { in_ptr = save_in_ptr; return fetch_ds_constructor_reply_keyboard_hide (T); }
  if (skip_constructor_reply_keyboard_force_reply (T) >= 0) { in_ptr = save_in_ptr; return fetch_ds_constructor_reply_keyboard_force_reply (T); }
  if (skip_constructor_reply_keyboard_markup (T) >= 0) { in_ptr = save_in_ptr; return fetch_ds_constructor_reply_keyboard_markup (T); }
  assert (0);
  return NULL;
}
struct tl_ds_res_p_q *fetch_ds_type_res_p_q (struct paramed_type *T) {
  assert (in_remaining () >= 4);
  int magic = fetch_int ();
  switch (magic) {
  case 0x05162463: return fetch_ds_constructor_res_p_q (T); break;
  default: assert (0); return NULL;
  }
}
struct tl_ds_res_p_q *fetch_ds_type_bare_res_p_q (struct paramed_type *T) {
  return fetch_ds_constructor_res_p_q (T);
  assert (0);
  return NULL;
}
struct tl_ds_send_message_action *fetch_ds_type_send_message_action (struct paramed_type *T) {
  assert (in_remaining () >= 4);
  int magic = fetch_int ();
  switch (magic) {
  case 0x16bf744e: return fetch_ds_constructor_send_message_typing_action (T); break;
  case 0xfd5ec8f5: return fetch_ds_constructor_send_message_cancel_action (T); break;
  case 0xa187d66f: return fetch_ds_constructor_send_message_record_video_action (T); break;
  case 0x92042ff7: return fetch_ds_constructor_send_message_upload_video_action_l27 (T); break;
  case 0xe9763aec: return fetch_ds_constructor_send_message_upload_video_action (T); break;
  case 0xd52f73f7: return fetch_ds_constructor_send_message_record_audio_action (T); break;
  case 0xe6ac8a6f: return fetch_ds_constructor_send_message_upload_audio_action_l27 (T); break;
  case 0xf351d7ab: return fetch_ds_constructor_send_message_upload_audio_action (T); break;
  case 0xd1d34a26: return fetch_ds_constructor_send_message_upload_photo_action (T); break;
  case 0x8faee98e: return fetch_ds_constructor_send_message_upload_document_action_l27 (T); break;
  case 0xaa0cd9e4: return fetch_ds_constructor_send_message_upload_document_action (T); break;
  case 0x176f8ba1: return fetch_ds_constructor_send_message_geo_location_action (T); break;
  case 0x628cbc6f: return fetch_ds_constructor_send_message_choose_contact_action (T); break;
  default: assert (0); return NULL;
  }
}
struct tl_ds_send_message_action *fetch_ds_type_bare_send_message_action (struct paramed_type *T) {
  int *save_in_ptr = in_ptr;
  if (skip_constructor_send_message_typing_action (T) >= 0) { in_ptr = save_in_ptr; return fetch_ds_constructor_send_message_typing_action (T); }
  if (skip_constructor_send_message_cancel_action (T) >= 0) { in_ptr = save_in_ptr; return fetch_ds_constructor_send_message_cancel_action (T); }
  if (skip_constructor_send_message_record_video_action (T) >= 0) { in_ptr = save_in_ptr; return fetch_ds_constructor_send_message_record_video_action (T); }
  if (skip_constructor_send_message_upload_video_action_l27 (T) >= 0) { in_ptr = save_in_ptr; return fetch_ds_constructor_send_message_upload_video_action_l27 (T); }
  if (skip_constructor_send_message_upload_video_action (T) >= 0) { in_ptr = save_in_ptr; return fetch_ds_constructor_send_message_upload_video_action (T); }
  if (skip_constructor_send_message_record_audio_action (T) >= 0) { in_ptr = save_in_ptr; return fetch_ds_constructor_send_message_record_audio_action (T); }
  if (skip_constructor_send_message_upload_audio_action_l27 (T) >= 0) { in_ptr = save_in_ptr; return fetch_ds_constructor_send_message_upload_audio_action_l27 (T); }
  if (skip_constructor_send_message_upload_audio_action (T) >= 0) { in_ptr = save_in_ptr; return fetch_ds_constructor_send_message_upload_audio_action (T); }
  if (skip_constructor_send_message_upload_photo_action (T) >= 0) { in_ptr = save_in_ptr; return fetch_ds_constructor_send_message_upload_photo_action (T); }
  if (skip_constructor_send_message_upload_document_action_l27 (T) >= 0) { in_ptr = save_in_ptr; return fetch_ds_constructor_send_message_upload_document_action_l27 (T); }
  if (skip_constructor_send_message_upload_document_action (T) >= 0) { in_ptr = save_in_ptr; return fetch_ds_constructor_send_message_upload_document_action (T); }
  if (skip_constructor_send_message_geo_location_action (T) >= 0) { in_ptr = save_in_ptr; return fetch_ds_constructor_send_message_geo_location_action (T); }
  if (skip_constructor_send_message_choose_contact_action (T) >= 0) { in_ptr = save_in_ptr; return fetch_ds_constructor_send_message_choose_contact_action (T); }
  assert (0);
  return NULL;
}
struct tl_ds_server_d_h_params *fetch_ds_type_server_d_h_params (struct paramed_type *T) {
  assert (in_remaining () >= 4);
  int magic = fetch_int ();
  switch (magic) {
  case 0x79cb045d: return fetch_ds_constructor_server_d_h_params_fail (T); break;
  case 0xd0e8075c: return fetch_ds_constructor_server_d_h_params_ok (T); break;
  default: assert (0); return NULL;
  }
}
struct tl_ds_server_d_h_params *fetch_ds_type_bare_server_d_h_params (struct paramed_type *T) {
  int *save_in_ptr = in_ptr;
  if (skip_constructor_server_d_h_params_fail (T) >= 0) { in_ptr = save_in_ptr; return fetch_ds_constructor_server_d_h_params_fail (T); }
  if (skip_constructor_server_d_h_params_ok (T) >= 0) { in_ptr = save_in_ptr; return fetch_ds_constructor_server_d_h_params_ok (T); }
  assert (0);
  return NULL;
}
struct tl_ds_server_d_h_inner_data *fetch_ds_type_server_d_h_inner_data (struct paramed_type *T) {
  assert (in_remaining () >= 4);
  int magic = fetch_int ();
  switch (magic) {
  case 0xb5890dba: return fetch_ds_constructor_server_d_h_inner_data (T); break;
  default: assert (0); return NULL;
  }
}
struct tl_ds_server_d_h_inner_data *fetch_ds_type_bare_server_d_h_inner_data (struct paramed_type *T) {
  return fetch_ds_constructor_server_d_h_inner_data (T);
  assert (0);
  return NULL;
}
struct tl_ds_set_client_d_h_params_answer *fetch_ds_type_set_client_d_h_params_answer (struct paramed_type *T) {
  assert (in_remaining () >= 4);
  int magic = fetch_int ();
  switch (magic) {
  case 0x3bcbf734: return fetch_ds_constructor_dh_gen_ok (T); break;
  case 0x46dc1fb9: return fetch_ds_constructor_dh_gen_retry (T); break;
  case 0xa69dae02: return fetch_ds_constructor_dh_gen_fail (T); break;
  default: assert (0); return NULL;
  }
}
struct tl_ds_set_client_d_h_params_answer *fetch_ds_type_bare_set_client_d_h_params_answer (struct paramed_type *T) {
  int *save_in_ptr = in_ptr;
  if (skip_constructor_dh_gen_ok (T) >= 0) { in_ptr = save_in_ptr; return fetch_ds_constructor_dh_gen_ok (T); }
  if (skip_constructor_dh_gen_retry (T) >= 0) { in_ptr = save_in_ptr; return fetch_ds_constructor_dh_gen_retry (T); }
  if (skip_constructor_dh_gen_fail (T) >= 0) { in_ptr = save_in_ptr; return fetch_ds_constructor_dh_gen_fail (T); }
  assert (0);
  return NULL;
}
struct tl_ds_sticker_pack *fetch_ds_type_sticker_pack (struct paramed_type *T) {
  assert (in_remaining () >= 4);
  int magic = fetch_int ();
  switch (magic) {
  case 0x12b299d4: return fetch_ds_constructor_sticker_pack (T); break;
  default: assert (0); return NULL;
  }
}
struct tl_ds_sticker_pack *fetch_ds_type_bare_sticker_pack (struct paramed_type *T) {
  return fetch_ds_constructor_sticker_pack (T);
  assert (0);
  return NULL;
}
struct tl_ds_sticker_set *fetch_ds_type_sticker_set (struct paramed_type *T) {
  assert (in_remaining () >= 4);
  int magic = fetch_int ();
  switch (magic) {
  case 0xa7a43b17: return fetch_ds_constructor_sticker_set (T); break;
  default: assert (0); return NULL;
  }
}
struct tl_ds_sticker_set *fetch_ds_type_bare_sticker_set (struct paramed_type *T) {
  return fetch_ds_constructor_sticker_set (T);
  assert (0);
  return NULL;
}
struct tl_ds_string *fetch_ds_type_string (struct paramed_type *T) {
  assert (in_remaining () >= 4);
  int magic = fetch_int ();
  switch (magic) {
  case 0xb5286e24: return fetch_ds_constructor_string (T); break;
  default: assert (0); return NULL;
  }
}
struct tl_ds_string *fetch_ds_type_bare_string (struct paramed_type *T) {
  return fetch_ds_constructor_string (T);
  assert (0);
  return NULL;
}
struct tl_ds_update *fetch_ds_type_update (struct paramed_type *T) {
  assert (in_remaining () >= 4);
  int magic = fetch_int ();
  switch (magic) {
  case 0x1f2b0afd: return fetch_ds_constructor_update_new_message (T); break;
  case 0x4e90bfd6: return fetch_ds_constructor_update_message_i_d (T); break;
  case 0xa20db0e5: return fetch_ds_constructor_update_delete_messages (T); break;
  case 0x5c486927: return fetch_ds_constructor_update_user_typing (T); break;
  case 0x9a65ea1f: return fetch_ds_constructor_update_chat_user_typing (T); break;
  case 0x07761198: return fetch_ds_constructor_update_chat_participants (T); break;
  case 0x1bfbd823: return fetch_ds_constructor_update_user_status (T); break;
  case 0xa7332b73: return fetch_ds_constructor_update_user_name (T); break;
  case 0x95313b0c: return fetch_ds_constructor_update_user_photo (T); break;
  case 0x2575bbb9: return fetch_ds_constructor_update_contact_registered (T); break;
  case 0x9d2e67c5: return fetch_ds_constructor_update_contact_link (T); break;
  case 0x8f06529a: return fetch_ds_constructor_update_new_authorization (T); break;
  case 0x5a68e3f7: return fetch_ds_constructor_update_new_geo_chat_message (T); break;
  case 0x12bcbd9a: return fetch_ds_constructor_update_new_encrypted_message (T); break;
  case 0x1710f156: return fetch_ds_constructor_update_encrypted_chat_typing (T); break;
  case 0xb4a2e88d: return fetch_ds_constructor_update_encryption (T); break;
  case 0x38fe25b7: return fetch_ds_constructor_update_encrypted_messages_read (T); break;
  case 0x3a0eeb22: return fetch_ds_constructor_update_chat_participant_add (T); break;
  case 0x6e5f8c22: return fetch_ds_constructor_update_chat_participant_delete (T); break;
  case 0x8e5e9873: return fetch_ds_constructor_update_dc_options (T); break;
  case 0x80ece81a: return fetch_ds_constructor_update_user_blocked (T); break;
  case 0xbec268ef: return fetch_ds_constructor_update_notify_settings (T); break;
  case 0x382dd3e4: return fetch_ds_constructor_update_service_notification (T); break;
  case 0xee3b272a: return fetch_ds_constructor_update_privacy (T); break;
  case 0x12b9417b: return fetch_ds_constructor_update_user_phone (T); break;
  case 0x9961fd5c: return fetch_ds_constructor_update_read_history_inbox (T); break;
  case 0x2f2f21bf: return fetch_ds_constructor_update_read_history_outbox (T); break;
  case 0x2cc36971: return fetch_ds_constructor_update_web_page (T); break;
  case 0x68c13933: return fetch_ds_constructor_update_read_messages_contents (T); break;
  case 0x03114739: return fetch_ds_constructor_update_msg_update (T); break;
  default: assert (0); return NULL;
  }
}
struct tl_ds_update *fetch_ds_type_bare_update (struct paramed_type *T) {
  int *save_in_ptr = in_ptr;
  if (skip_constructor_update_new_message (T) >= 0) { in_ptr = save_in_ptr; return fetch_ds_constructor_update_new_message (T); }
  if (skip_constructor_update_message_i_d (T) >= 0) { in_ptr = save_in_ptr; return fetch_ds_constructor_update_message_i_d (T); }
  if (skip_constructor_update_delete_messages (T) >= 0) { in_ptr = save_in_ptr; return fetch_ds_constructor_update_delete_messages (T); }
  if (skip_constructor_update_user_typing (T) >= 0) { in_ptr = save_in_ptr; return fetch_ds_constructor_update_user_typing (T); }
  if (skip_constructor_update_chat_user_typing (T) >= 0) { in_ptr = save_in_ptr; return fetch_ds_constructor_update_chat_user_typing (T); }
  if (skip_constructor_update_chat_participants (T) >= 0) { in_ptr = save_in_ptr; return fetch_ds_constructor_update_chat_participants (T); }
  if (skip_constructor_update_user_status (T) >= 0) { in_ptr = save_in_ptr; return fetch_ds_constructor_update_user_status (T); }
  if (skip_constructor_update_user_name (T) >= 0) { in_ptr = save_in_ptr; return fetch_ds_constructor_update_user_name (T); }
  if (skip_constructor_update_user_photo (T) >= 0) { in_ptr = save_in_ptr; return fetch_ds_constructor_update_user_photo (T); }
  if (skip_constructor_update_contact_registered (T) >= 0) { in_ptr = save_in_ptr; return fetch_ds_constructor_update_contact_registered (T); }
  if (skip_constructor_update_contact_link (T) >= 0) { in_ptr = save_in_ptr; return fetch_ds_constructor_update_contact_link (T); }
  if (skip_constructor_update_new_authorization (T) >= 0) { in_ptr = save_in_ptr; return fetch_ds_constructor_update_new_authorization (T); }
  if (skip_constructor_update_new_geo_chat_message (T) >= 0) { in_ptr = save_in_ptr; return fetch_ds_constructor_update_new_geo_chat_message (T); }
  if (skip_constructor_update_new_encrypted_message (T) >= 0) { in_ptr = save_in_ptr; return fetch_ds_constructor_update_new_encrypted_message (T); }
  if (skip_constructor_update_encrypted_chat_typing (T) >= 0) { in_ptr = save_in_ptr; return fetch_ds_constructor_update_encrypted_chat_typing (T); }
  if (skip_constructor_update_encryption (T) >= 0) { in_ptr = save_in_ptr; return fetch_ds_constructor_update_encryption (T); }
  if (skip_constructor_update_encrypted_messages_read (T) >= 0) { in_ptr = save_in_ptr; return fetch_ds_constructor_update_encrypted_messages_read (T); }
  if (skip_constructor_update_chat_participant_add (T) >= 0) { in_ptr = save_in_ptr; return fetch_ds_constructor_update_chat_participant_add (T); }
  if (skip_constructor_update_chat_participant_delete (T) >= 0) { in_ptr = save_in_ptr; return fetch_ds_constructor_update_chat_participant_delete (T); }
  if (skip_constructor_update_dc_options (T) >= 0) { in_ptr = save_in_ptr; return fetch_ds_constructor_update_dc_options (T); }
  if (skip_constructor_update_user_blocked (T) >= 0) { in_ptr = save_in_ptr; return fetch_ds_constructor_update_user_blocked (T); }
  if (skip_constructor_update_notify_settings (T) >= 0) { in_ptr = save_in_ptr; return fetch_ds_constructor_update_notify_settings (T); }
  if (skip_constructor_update_service_notification (T) >= 0) { in_ptr = save_in_ptr; return fetch_ds_constructor_update_service_notification (T); }
  if (skip_constructor_update_privacy (T) >= 0) { in_ptr = save_in_ptr; return fetch_ds_constructor_update_privacy (T); }
  if (skip_constructor_update_user_phone (T) >= 0) { in_ptr = save_in_ptr; return fetch_ds_constructor_update_user_phone (T); }
  if (skip_constructor_update_read_history_inbox (T) >= 0) { in_ptr = save_in_ptr; return fetch_ds_constructor_update_read_history_inbox (T); }
  if (skip_constructor_update_read_history_outbox (T) >= 0) { in_ptr = save_in_ptr; return fetch_ds_constructor_update_read_history_outbox (T); }
  if (skip_constructor_update_web_page (T) >= 0) { in_ptr = save_in_ptr; return fetch_ds_constructor_update_web_page (T); }
  if (skip_constructor_update_read_messages_contents (T) >= 0) { in_ptr = save_in_ptr; return fetch_ds_constructor_update_read_messages_contents (T); }
  if (skip_constructor_update_msg_update (T) >= 0) { in_ptr = save_in_ptr; return fetch_ds_constructor_update_msg_update (T); }
  assert (0);
  return NULL;
}
struct tl_ds_updates *fetch_ds_type_updates (struct paramed_type *T) {
  assert (in_remaining () >= 4);
  int magic = fetch_int ();
  switch (magic) {
  case 0xe317af7e: return fetch_ds_constructor_updates_too_long (T); break;
  case 0xed5c2127: return fetch_ds_constructor_update_short_message (T); break;
  case 0x52238b3c: return fetch_ds_constructor_update_short_chat_message (T); break;
  case 0x78d4dec1: return fetch_ds_constructor_update_short (T); break;
  case 0x725b04c3: return fetch_ds_constructor_updates_combined (T); break;
  case 0x74ae4240: return fetch_ds_constructor_updates (T); break;
  default: assert (0); return NULL;
  }
}
struct tl_ds_updates *fetch_ds_type_bare_updates (struct paramed_type *T) {
  int *save_in_ptr = in_ptr;
  if (skip_constructor_updates_too_long (T) >= 0) { in_ptr = save_in_ptr; return fetch_ds_constructor_updates_too_long (T); }
  if (skip_constructor_update_short_message (T) >= 0) { in_ptr = save_in_ptr; return fetch_ds_constructor_update_short_message (T); }
  if (skip_constructor_update_short_chat_message (T) >= 0) { in_ptr = save_in_ptr; return fetch_ds_constructor_update_short_chat_message (T); }
  if (skip_constructor_update_short (T) >= 0) { in_ptr = save_in_ptr; return fetch_ds_constructor_update_short (T); }
  if (skip_constructor_updates_combined (T) >= 0) { in_ptr = save_in_ptr; return fetch_ds_constructor_updates_combined (T); }
  if (skip_constructor_updates (T) >= 0) { in_ptr = save_in_ptr; return fetch_ds_constructor_updates (T); }
  assert (0);
  return NULL;
}
struct tl_ds_user *fetch_ds_type_user (struct paramed_type *T) {
  assert (in_remaining () >= 4);
  int magic = fetch_int ();
  switch (magic) {
  case 0x200250ba: return fetch_ds_constructor_user_empty (T); break;
  case 0x22e49072: return fetch_ds_constructor_user (T); break;
  default: assert (0); return NULL;
  }
}
struct tl_ds_user *fetch_ds_type_bare_user (struct paramed_type *T) {
  int *save_in_ptr = in_ptr;
  if (skip_constructor_user_empty (T) >= 0) { in_ptr = save_in_ptr; return fetch_ds_constructor_user_empty (T); }
  if (skip_constructor_user (T) >= 0) { in_ptr = save_in_ptr; return fetch_ds_constructor_user (T); }
  assert (0);
  return NULL;
}
struct tl_ds_user_full *fetch_ds_type_user_full (struct paramed_type *T) {
  assert (in_remaining () >= 4);
  int magic = fetch_int ();
  switch (magic) {
  case 0x5a89ac5b: return fetch_ds_constructor_user_full (T); break;
  default: assert (0); return NULL;
  }
}
struct tl_ds_user_full *fetch_ds_type_bare_user_full (struct paramed_type *T) {
  return fetch_ds_constructor_user_full (T);
  assert (0);
  return NULL;
}
struct tl_ds_user_profile_photo *fetch_ds_type_user_profile_photo (struct paramed_type *T) {
  assert (in_remaining () >= 4);
  int magic = fetch_int ();
  switch (magic) {
  case 0x4f11bae1: return fetch_ds_constructor_user_profile_photo_empty (T); break;
  case 0xd559d8c8: return fetch_ds_constructor_user_profile_photo (T); break;
  default: assert (0); return NULL;
  }
}
struct tl_ds_user_profile_photo *fetch_ds_type_bare_user_profile_photo (struct paramed_type *T) {
  int *save_in_ptr = in_ptr;
  if (skip_constructor_user_profile_photo_empty (T) >= 0) { in_ptr = save_in_ptr; return fetch_ds_constructor_user_profile_photo_empty (T); }
  if (skip_constructor_user_profile_photo (T) >= 0) { in_ptr = save_in_ptr; return fetch_ds_constructor_user_profile_photo (T); }
  assert (0);
  return NULL;
}
struct tl_ds_user_status *fetch_ds_type_user_status (struct paramed_type *T) {
  assert (in_remaining () >= 4);
  int magic = fetch_int ();
  switch (magic) {
  case 0x09d05049: return fetch_ds_constructor_user_status_empty (T); break;
  case 0xedb93949: return fetch_ds_constructor_user_status_online (T); break;
  case 0x008c703f: return fetch_ds_constructor_user_status_offline (T); break;
  case 0xe26f42f1: return fetch_ds_constructor_user_status_recently (T); break;
  case 0x07bf09fc: return fetch_ds_constructor_user_status_last_week (T); break;
  case 0x77ebc742: return fetch_ds_constructor_user_status_last_month (T); break;
  default: assert (0); return NULL;
  }
}
struct tl_ds_user_status *fetch_ds_type_bare_user_status (struct paramed_type *T) {
  int *save_in_ptr = in_ptr;
  if (skip_constructor_user_status_empty (T) >= 0) { in_ptr = save_in_ptr; return fetch_ds_constructor_user_status_empty (T); }
  if (skip_constructor_user_status_online (T) >= 0) { in_ptr = save_in_ptr; return fetch_ds_constructor_user_status_online (T); }
  if (skip_constructor_user_status_offline (T) >= 0) { in_ptr = save_in_ptr; return fetch_ds_constructor_user_status_offline (T); }
  if (skip_constructor_user_status_recently (T) >= 0) { in_ptr = save_in_ptr; return fetch_ds_constructor_user_status_recently (T); }
  if (skip_constructor_user_status_last_week (T) >= 0) { in_ptr = save_in_ptr; return fetch_ds_constructor_user_status_last_week (T); }
  if (skip_constructor_user_status_last_month (T) >= 0) { in_ptr = save_in_ptr; return fetch_ds_constructor_user_status_last_month (T); }
  assert (0);
  return NULL;
}
struct tl_ds_vector *fetch_ds_type_vector (struct paramed_type *T) {
  assert (in_remaining () >= 4);
  int magic = fetch_int ();
  switch (magic) {
  case 0x1cb5c415: return fetch_ds_constructor_vector (T); break;
  default: assert (0); return NULL;
  }
}
struct tl_ds_vector *fetch_ds_type_bare_vector (struct paramed_type *T) {
  return fetch_ds_constructor_vector (T);
  assert (0);
  return NULL;
}
struct tl_ds_video *fetch_ds_type_video (struct paramed_type *T) {
  assert (in_remaining () >= 4);
  int magic = fetch_int ();
  switch (magic) {
  case 0xc10658a8: return fetch_ds_constructor_video_empty (T); break;
  case 0xee9f4a4d: return fetch_ds_constructor_video (T); break;
  case 0x388fa391: return fetch_ds_constructor_video_l27 (T); break;
  default: assert (0); return NULL;
  }
}
struct tl_ds_video *fetch_ds_type_bare_video (struct paramed_type *T) {
  int *save_in_ptr = in_ptr;
  if (skip_constructor_video_empty (T) >= 0) { in_ptr = save_in_ptr; return fetch_ds_constructor_video_empty (T); }
  if (skip_constructor_video (T) >= 0) { in_ptr = save_in_ptr; return fetch_ds_constructor_video (T); }
  if (skip_constructor_video_l27 (T) >= 0) { in_ptr = save_in_ptr; return fetch_ds_constructor_video_l27 (T); }
  assert (0);
  return NULL;
}
struct tl_ds_wall_paper *fetch_ds_type_wall_paper (struct paramed_type *T) {
  assert (in_remaining () >= 4);
  int magic = fetch_int ();
  switch (magic) {
  case 0xccb03657: return fetch_ds_constructor_wall_paper (T); break;
  case 0x63117f24: return fetch_ds_constructor_wall_paper_solid (T); break;
  default: assert (0); return NULL;
  }
}
struct tl_ds_wall_paper *fetch_ds_type_bare_wall_paper (struct paramed_type *T) {
  int *save_in_ptr = in_ptr;
  if (skip_constructor_wall_paper (T) >= 0) { in_ptr = save_in_ptr; return fetch_ds_constructor_wall_paper (T); }
  if (skip_constructor_wall_paper_solid (T) >= 0) { in_ptr = save_in_ptr; return fetch_ds_constructor_wall_paper_solid (T); }
  assert (0);
  return NULL;
}
struct tl_ds_web_page *fetch_ds_type_web_page (struct paramed_type *T) {
  assert (in_remaining () >= 4);
  int magic = fetch_int ();
  switch (magic) {
  case 0xeb1477e8: return fetch_ds_constructor_web_page_empty (T); break;
  case 0xc586da1c: return fetch_ds_constructor_web_page_pending (T); break;
  case 0xa31ea0b5: return fetch_ds_constructor_web_page (T); break;
  default: assert (0); return NULL;
  }
}
struct tl_ds_web_page *fetch_ds_type_bare_web_page (struct paramed_type *T) {
  int *save_in_ptr = in_ptr;
  if (skip_constructor_web_page_empty (T) >= 0) { in_ptr = save_in_ptr; return fetch_ds_constructor_web_page_empty (T); }
  if (skip_constructor_web_page_pending (T) >= 0) { in_ptr = save_in_ptr; return fetch_ds_constructor_web_page_pending (T); }
  if (skip_constructor_web_page (T) >= 0) { in_ptr = save_in_ptr; return fetch_ds_constructor_web_page (T); }
  assert (0);
  return NULL;
}
struct tl_ds_account_authorizations *fetch_ds_type_account_authorizations (struct paramed_type *T) {
  assert (in_remaining () >= 4);
  int magic = fetch_int ();
  switch (magic) {
  case 0x1250abde: return fetch_ds_constructor_account_authorizations (T); break;
  default: assert (0); return NULL;
  }
}
struct tl_ds_account_authorizations *fetch_ds_type_bare_account_authorizations (struct paramed_type *T) {
  return fetch_ds_constructor_account_authorizations (T);
  assert (0);
  return NULL;
}
struct tl_ds_account_password *fetch_ds_type_account_password (struct paramed_type *T) {
  assert (in_remaining () >= 4);
  int magic = fetch_int ();
  switch (magic) {
  case 0x96dabc18: return fetch_ds_constructor_account_no_password (T); break;
  case 0x7c18141c: return fetch_ds_constructor_account_password (T); break;
  default: assert (0); return NULL;
  }
}
struct tl_ds_account_password *fetch_ds_type_bare_account_password (struct paramed_type *T) {
  int *save_in_ptr = in_ptr;
  if (skip_constructor_account_no_password (T) >= 0) { in_ptr = save_in_ptr; return fetch_ds_constructor_account_no_password (T); }
  if (skip_constructor_account_password (T) >= 0) { in_ptr = save_in_ptr; return fetch_ds_constructor_account_password (T); }
  assert (0);
  return NULL;
}
struct tl_ds_account_password_input_settings *fetch_ds_type_account_password_input_settings (struct paramed_type *T) {
  assert (in_remaining () >= 4);
  int magic = fetch_int ();
  switch (magic) {
  case 0xbcfc532c: return fetch_ds_constructor_account_password_input_settings (T); break;
  default: assert (0); return NULL;
  }
}
struct tl_ds_account_password_input_settings *fetch_ds_type_bare_account_password_input_settings (struct paramed_type *T) {
  return fetch_ds_constructor_account_password_input_settings (T);
  assert (0);
  return NULL;
}
struct tl_ds_account_password_settings *fetch_ds_type_account_password_settings (struct paramed_type *T) {
  assert (in_remaining () >= 4);
  int magic = fetch_int ();
  switch (magic) {
  case 0xb7b72ab3: return fetch_ds_constructor_account_password_settings (T); break;
  default: assert (0); return NULL;
  }
}
struct tl_ds_account_password_settings *fetch_ds_type_bare_account_password_settings (struct paramed_type *T) {
  return fetch_ds_constructor_account_password_settings (T);
  assert (0);
  return NULL;
}
struct tl_ds_account_privacy_rules *fetch_ds_type_account_privacy_rules (struct paramed_type *T) {
  assert (in_remaining () >= 4);
  int magic = fetch_int ();
  switch (magic) {
  case 0x554abb6f: return fetch_ds_constructor_account_privacy_rules (T); break;
  default: assert (0); return NULL;
  }
}
struct tl_ds_account_privacy_rules *fetch_ds_type_bare_account_privacy_rules (struct paramed_type *T) {
  return fetch_ds_constructor_account_privacy_rules (T);
  assert (0);
  return NULL;
}
struct tl_ds_account_sent_change_phone_code *fetch_ds_type_account_sent_change_phone_code (struct paramed_type *T) {
  assert (in_remaining () >= 4);
  int magic = fetch_int ();
  switch (magic) {
  case 0xa4f58c4c: return fetch_ds_constructor_account_sent_change_phone_code (T); break;
  default: assert (0); return NULL;
  }
}
struct tl_ds_account_sent_change_phone_code *fetch_ds_type_bare_account_sent_change_phone_code (struct paramed_type *T) {
  return fetch_ds_constructor_account_sent_change_phone_code (T);
  assert (0);
  return NULL;
}
struct tl_ds_auth_authorization *fetch_ds_type_auth_authorization (struct paramed_type *T) {
  assert (in_remaining () >= 4);
  int magic = fetch_int ();
  switch (magic) {
  case 0xff036af1: return fetch_ds_constructor_auth_authorization (T); break;
  default: assert (0); return NULL;
  }
}
struct tl_ds_auth_authorization *fetch_ds_type_bare_auth_authorization (struct paramed_type *T) {
  return fetch_ds_constructor_auth_authorization (T);
  assert (0);
  return NULL;
}
struct tl_ds_auth_checked_phone *fetch_ds_type_auth_checked_phone (struct paramed_type *T) {
  assert (in_remaining () >= 4);
  int magic = fetch_int ();
  switch (magic) {
  case 0x811ea28e: return fetch_ds_constructor_auth_checked_phone (T); break;
  default: assert (0); return NULL;
  }
}
struct tl_ds_auth_checked_phone *fetch_ds_type_bare_auth_checked_phone (struct paramed_type *T) {
  return fetch_ds_constructor_auth_checked_phone (T);
  assert (0);
  return NULL;
}
struct tl_ds_auth_exported_authorization *fetch_ds_type_auth_exported_authorization (struct paramed_type *T) {
  assert (in_remaining () >= 4);
  int magic = fetch_int ();
  switch (magic) {
  case 0xdf969c2d: return fetch_ds_constructor_auth_exported_authorization (T); break;
  default: assert (0); return NULL;
  }
}
struct tl_ds_auth_exported_authorization *fetch_ds_type_bare_auth_exported_authorization (struct paramed_type *T) {
  return fetch_ds_constructor_auth_exported_authorization (T);
  assert (0);
  return NULL;
}
struct tl_ds_auth_password_recovery *fetch_ds_type_auth_password_recovery (struct paramed_type *T) {
  assert (in_remaining () >= 4);
  int magic = fetch_int ();
  switch (magic) {
  case 0x137948a5: return fetch_ds_constructor_auth_password_recovery (T); break;
  default: assert (0); return NULL;
  }
}
struct tl_ds_auth_password_recovery *fetch_ds_type_bare_auth_password_recovery (struct paramed_type *T) {
  return fetch_ds_constructor_auth_password_recovery (T);
  assert (0);
  return NULL;
}
struct tl_ds_auth_sent_code *fetch_ds_type_auth_sent_code (struct paramed_type *T) {
  assert (in_remaining () >= 4);
  int magic = fetch_int ();
  switch (magic) {
  case 0xefed51d9: return fetch_ds_constructor_auth_sent_code (T); break;
  case 0xe325edcf: return fetch_ds_constructor_auth_sent_app_code (T); break;
  default: assert (0); return NULL;
  }
}
struct tl_ds_auth_sent_code *fetch_ds_type_bare_auth_sent_code (struct paramed_type *T) {
  int *save_in_ptr = in_ptr;
  if (skip_constructor_auth_sent_code (T) >= 0) { in_ptr = save_in_ptr; return fetch_ds_constructor_auth_sent_code (T); }
  if (skip_constructor_auth_sent_app_code (T) >= 0) { in_ptr = save_in_ptr; return fetch_ds_constructor_auth_sent_app_code (T); }
  assert (0);
  return NULL;
}
struct tl_ds_binlog_encr_key *fetch_ds_type_binlog_encr_key (struct paramed_type *T) {
  assert (in_remaining () >= 4);
  int magic = fetch_int ();
  switch (magic) {
  case 0x0377168f: return fetch_ds_constructor_binlog_encr_key (T); break;
  default: assert (0); return NULL;
  }
}
struct tl_ds_binlog_encr_key *fetch_ds_type_bare_binlog_encr_key (struct paramed_type *T) {
  return fetch_ds_constructor_binlog_encr_key (T);
  assert (0);
  return NULL;
}
struct tl_ds_binlog_update *fetch_ds_type_binlog_update (struct paramed_type *T) {
  assert (in_remaining () >= 4);
  int magic = fetch_int ();
  switch (magic) {
  case 0x3b06de69: return fetch_ds_constructor_binlog_start (T); break;
  case 0xf96feb32: return fetch_ds_constructor_binlog_dc_option (T); break;
  case 0x7c0d22d8: return fetch_ds_constructor_binlog_dc_option_new (T); break;
  case 0x71e8c156: return fetch_ds_constructor_binlog_auth_key (T); break;
  case 0x9e83dbdc: return fetch_ds_constructor_binlog_default_dc (T); break;
  case 0x26451bb5: return fetch_ds_constructor_binlog_dc_signed (T); break;
  case 0x68a870e8: return fetch_ds_constructor_binlog_our_id (T); break;
  case 0xeaeb7826: return fetch_ds_constructor_binlog_set_dh_params (T); break;
  case 0x2ca8c939: return fetch_ds_constructor_binlog_set_pts (T); break;
  case 0xd95738ac: return fetch_ds_constructor_binlog_set_qts (T); break;
  case 0x1d0f4b52: return fetch_ds_constructor_binlog_set_date (T); break;
  case 0x6eeb2989: return fetch_ds_constructor_binlog_set_seq (T); break;
  case 0xee1b38e8: return fetch_ds_constructor_binlog_encr_chat_delete (T); break;
  case 0x84977251: return fetch_ds_constructor_binlog_encr_chat_new (T); break;
  case 0x9d49488d: return fetch_ds_constructor_binlog_encr_chat_exchange_new (T); break;
  case 0xac55d447: return fetch_ds_constructor_binlog_user_delete (T); break;
  case 0x127cf2f9: return fetch_ds_constructor_binlog_user_new (T); break;
  case 0x0a10aa92: return fetch_ds_constructor_binlog_chat_new (T); break;
  case 0x535475ea: return fetch_ds_constructor_binlog_chat_add_participant (T); break;
  case 0x7dd1a1a2: return fetch_ds_constructor_binlog_chat_del_participant (T); break;
  case 0x3c873416: return fetch_ds_constructor_binlog_set_msg_id (T); break;
  case 0x847e77b1: return fetch_ds_constructor_binlog_message_delete (T); break;
  case 0x427cfcdb: return fetch_ds_constructor_binlog_message_new (T); break;
  case 0x6cf7cabc: return fetch_ds_constructor_binlog_message_encr_new (T); break;
  case 0x6dd4d85f: return fetch_ds_constructor_binlog_msg_update (T); break;
  case 0x83327955: return fetch_ds_constructor_binlog_reset_authorization (T); break;
  default: assert (0); return NULL;
  }
}
struct tl_ds_binlog_update *fetch_ds_type_bare_binlog_update (struct paramed_type *T) {
  int *save_in_ptr = in_ptr;
  if (skip_constructor_binlog_start (T) >= 0) { in_ptr = save_in_ptr; return fetch_ds_constructor_binlog_start (T); }
  if (skip_constructor_binlog_dc_option (T) >= 0) { in_ptr = save_in_ptr; return fetch_ds_constructor_binlog_dc_option (T); }
  if (skip_constructor_binlog_dc_option_new (T) >= 0) { in_ptr = save_in_ptr; return fetch_ds_constructor_binlog_dc_option_new (T); }
  if (skip_constructor_binlog_auth_key (T) >= 0) { in_ptr = save_in_ptr; return fetch_ds_constructor_binlog_auth_key (T); }
  if (skip_constructor_binlog_default_dc (T) >= 0) { in_ptr = save_in_ptr; return fetch_ds_constructor_binlog_default_dc (T); }
  if (skip_constructor_binlog_dc_signed (T) >= 0) { in_ptr = save_in_ptr; return fetch_ds_constructor_binlog_dc_signed (T); }
  if (skip_constructor_binlog_our_id (T) >= 0) { in_ptr = save_in_ptr; return fetch_ds_constructor_binlog_our_id (T); }
  if (skip_constructor_binlog_set_dh_params (T) >= 0) { in_ptr = save_in_ptr; return fetch_ds_constructor_binlog_set_dh_params (T); }
  if (skip_constructor_binlog_set_pts (T) >= 0) { in_ptr = save_in_ptr; return fetch_ds_constructor_binlog_set_pts (T); }
  if (skip_constructor_binlog_set_qts (T) >= 0) { in_ptr = save_in_ptr; return fetch_ds_constructor_binlog_set_qts (T); }
  if (skip_constructor_binlog_set_date (T) >= 0) { in_ptr = save_in_ptr; return fetch_ds_constructor_binlog_set_date (T); }
  if (skip_constructor_binlog_set_seq (T) >= 0) { in_ptr = save_in_ptr; return fetch_ds_constructor_binlog_set_seq (T); }
  if (skip_constructor_binlog_encr_chat_delete (T) >= 0) { in_ptr = save_in_ptr; return fetch_ds_constructor_binlog_encr_chat_delete (T); }
  if (skip_constructor_binlog_encr_chat_new (T) >= 0) { in_ptr = save_in_ptr; return fetch_ds_constructor_binlog_encr_chat_new (T); }
  if (skip_constructor_binlog_encr_chat_exchange_new (T) >= 0) { in_ptr = save_in_ptr; return fetch_ds_constructor_binlog_encr_chat_exchange_new (T); }
  if (skip_constructor_binlog_user_delete (T) >= 0) { in_ptr = save_in_ptr; return fetch_ds_constructor_binlog_user_delete (T); }
  if (skip_constructor_binlog_user_new (T) >= 0) { in_ptr = save_in_ptr; return fetch_ds_constructor_binlog_user_new (T); }
  if (skip_constructor_binlog_chat_new (T) >= 0) { in_ptr = save_in_ptr; return fetch_ds_constructor_binlog_chat_new (T); }
  if (skip_constructor_binlog_chat_add_participant (T) >= 0) { in_ptr = save_in_ptr; return fetch_ds_constructor_binlog_chat_add_participant (T); }
  if (skip_constructor_binlog_chat_del_participant (T) >= 0) { in_ptr = save_in_ptr; return fetch_ds_constructor_binlog_chat_del_participant (T); }
  if (skip_constructor_binlog_set_msg_id (T) >= 0) { in_ptr = save_in_ptr; return fetch_ds_constructor_binlog_set_msg_id (T); }
  if (skip_constructor_binlog_message_delete (T) >= 0) { in_ptr = save_in_ptr; return fetch_ds_constructor_binlog_message_delete (T); }
  if (skip_constructor_binlog_message_new (T) >= 0) { in_ptr = save_in_ptr; return fetch_ds_constructor_binlog_message_new (T); }
  if (skip_constructor_binlog_message_encr_new (T) >= 0) { in_ptr = save_in_ptr; return fetch_ds_constructor_binlog_message_encr_new (T); }
  if (skip_constructor_binlog_msg_update (T) >= 0) { in_ptr = save_in_ptr; return fetch_ds_constructor_binlog_msg_update (T); }
  if (skip_constructor_binlog_reset_authorization (T) >= 0) { in_ptr = save_in_ptr; return fetch_ds_constructor_binlog_reset_authorization (T); }
  assert (0);
  return NULL;
}
struct tl_ds_contacts_blocked *fetch_ds_type_contacts_blocked (struct paramed_type *T) {
  assert (in_remaining () >= 4);
  int magic = fetch_int ();
  switch (magic) {
  case 0x1c138d15: return fetch_ds_constructor_contacts_blocked (T); break;
  case 0x900802a1: return fetch_ds_constructor_contacts_blocked_slice (T); break;
  default: assert (0); return NULL;
  }
}
struct tl_ds_contacts_blocked *fetch_ds_type_bare_contacts_blocked (struct paramed_type *T) {
  int *save_in_ptr = in_ptr;
  if (skip_constructor_contacts_blocked (T) >= 0) { in_ptr = save_in_ptr; return fetch_ds_constructor_contacts_blocked (T); }
  if (skip_constructor_contacts_blocked_slice (T) >= 0) { in_ptr = save_in_ptr; return fetch_ds_constructor_contacts_blocked_slice (T); }
  assert (0);
  return NULL;
}
struct tl_ds_contacts_contacts *fetch_ds_type_contacts_contacts (struct paramed_type *T) {
  assert (in_remaining () >= 4);
  int magic = fetch_int ();
  switch (magic) {
  case 0xb74ba9d2: return fetch_ds_constructor_contacts_contacts_not_modified (T); break;
  case 0x6f8b8cb2: return fetch_ds_constructor_contacts_contacts (T); break;
  default: assert (0); return NULL;
  }
}
struct tl_ds_contacts_contacts *fetch_ds_type_bare_contacts_contacts (struct paramed_type *T) {
  int *save_in_ptr = in_ptr;
  if (skip_constructor_contacts_contacts_not_modified (T) >= 0) { in_ptr = save_in_ptr; return fetch_ds_constructor_contacts_contacts_not_modified (T); }
  if (skip_constructor_contacts_contacts (T) >= 0) { in_ptr = save_in_ptr; return fetch_ds_constructor_contacts_contacts (T); }
  assert (0);
  return NULL;
}
struct tl_ds_contacts_found *fetch_ds_type_contacts_found (struct paramed_type *T) {
  assert (in_remaining () >= 4);
  int magic = fetch_int ();
  switch (magic) {
  case 0x0566000e: return fetch_ds_constructor_contacts_found (T); break;
  default: assert (0); return NULL;
  }
}
struct tl_ds_contacts_found *fetch_ds_type_bare_contacts_found (struct paramed_type *T) {
  return fetch_ds_constructor_contacts_found (T);
  assert (0);
  return NULL;
}
struct tl_ds_contacts_imported_contacts *fetch_ds_type_contacts_imported_contacts (struct paramed_type *T) {
  assert (in_remaining () >= 4);
  int magic = fetch_int ();
  switch (magic) {
  case 0xad524315: return fetch_ds_constructor_contacts_imported_contacts (T); break;
  default: assert (0); return NULL;
  }
}
struct tl_ds_contacts_imported_contacts *fetch_ds_type_bare_contacts_imported_contacts (struct paramed_type *T) {
  return fetch_ds_constructor_contacts_imported_contacts (T);
  assert (0);
  return NULL;
}
struct tl_ds_contacts_link *fetch_ds_type_contacts_link (struct paramed_type *T) {
  assert (in_remaining () >= 4);
  int magic = fetch_int ();
  switch (magic) {
  case 0x3ace484c: return fetch_ds_constructor_contacts_link (T); break;
  default: assert (0); return NULL;
  }
}
struct tl_ds_contacts_link *fetch_ds_type_bare_contacts_link (struct paramed_type *T) {
  return fetch_ds_constructor_contacts_link (T);
  assert (0);
  return NULL;
}
struct tl_ds_contacts_suggested *fetch_ds_type_contacts_suggested (struct paramed_type *T) {
  assert (in_remaining () >= 4);
  int magic = fetch_int ();
  switch (magic) {
  case 0x5649dcc5: return fetch_ds_constructor_contacts_suggested (T); break;
  default: assert (0); return NULL;
  }
}
struct tl_ds_contacts_suggested *fetch_ds_type_bare_contacts_suggested (struct paramed_type *T) {
  return fetch_ds_constructor_contacts_suggested (T);
  assert (0);
  return NULL;
}
struct tl_ds_geochats_located *fetch_ds_type_geochats_located (struct paramed_type *T) {
  assert (in_remaining () >= 4);
  int magic = fetch_int ();
  switch (magic) {
  case 0x48feb267: return fetch_ds_constructor_geochats_located (T); break;
  default: assert (0); return NULL;
  }
}
struct tl_ds_geochats_located *fetch_ds_type_bare_geochats_located (struct paramed_type *T) {
  return fetch_ds_constructor_geochats_located (T);
  assert (0);
  return NULL;
}
struct tl_ds_geochats_messages *fetch_ds_type_geochats_messages (struct paramed_type *T) {
  assert (in_remaining () >= 4);
  int magic = fetch_int ();
  switch (magic) {
  case 0xd1526db1: return fetch_ds_constructor_geochats_messages (T); break;
  case 0xbc5863e8: return fetch_ds_constructor_geochats_messages_slice (T); break;
  default: assert (0); return NULL;
  }
}
struct tl_ds_geochats_messages *fetch_ds_type_bare_geochats_messages (struct paramed_type *T) {
  int *save_in_ptr = in_ptr;
  if (skip_constructor_geochats_messages (T) >= 0) { in_ptr = save_in_ptr; return fetch_ds_constructor_geochats_messages (T); }
  if (skip_constructor_geochats_messages_slice (T) >= 0) { in_ptr = save_in_ptr; return fetch_ds_constructor_geochats_messages_slice (T); }
  assert (0);
  return NULL;
}
struct tl_ds_geochats_stated_message *fetch_ds_type_geochats_stated_message (struct paramed_type *T) {
  assert (in_remaining () >= 4);
  int magic = fetch_int ();
  switch (magic) {
  case 0x17b1578b: return fetch_ds_constructor_geochats_stated_message (T); break;
  default: assert (0); return NULL;
  }
}
struct tl_ds_geochats_stated_message *fetch_ds_type_bare_geochats_stated_message (struct paramed_type *T) {
  return fetch_ds_constructor_geochats_stated_message (T);
  assert (0);
  return NULL;
}
struct tl_ds_help_app_update *fetch_ds_type_help_app_update (struct paramed_type *T) {
  assert (in_remaining () >= 4);
  int magic = fetch_int ();
  switch (magic) {
  case 0x8987f311: return fetch_ds_constructor_help_app_update (T); break;
  case 0xc45a6536: return fetch_ds_constructor_help_no_app_update (T); break;
  default: assert (0); return NULL;
  }
}
struct tl_ds_help_app_update *fetch_ds_type_bare_help_app_update (struct paramed_type *T) {
  int *save_in_ptr = in_ptr;
  if (skip_constructor_help_app_update (T) >= 0) { in_ptr = save_in_ptr; return fetch_ds_constructor_help_app_update (T); }
  if (skip_constructor_help_no_app_update (T) >= 0) { in_ptr = save_in_ptr; return fetch_ds_constructor_help_no_app_update (T); }
  assert (0);
  return NULL;
}
struct tl_ds_help_invite_text *fetch_ds_type_help_invite_text (struct paramed_type *T) {
  assert (in_remaining () >= 4);
  int magic = fetch_int ();
  switch (magic) {
  case 0x18cb9f78: return fetch_ds_constructor_help_invite_text (T); break;
  default: assert (0); return NULL;
  }
}
struct tl_ds_help_invite_text *fetch_ds_type_bare_help_invite_text (struct paramed_type *T) {
  return fetch_ds_constructor_help_invite_text (T);
  assert (0);
  return NULL;
}
struct tl_ds_help_support *fetch_ds_type_help_support (struct paramed_type *T) {
  assert (in_remaining () >= 4);
  int magic = fetch_int ();
  switch (magic) {
  case 0x17c6b5f6: return fetch_ds_constructor_help_support (T); break;
  default: assert (0); return NULL;
  }
}
struct tl_ds_help_support *fetch_ds_type_bare_help_support (struct paramed_type *T) {
  return fetch_ds_constructor_help_support (T);
  assert (0);
  return NULL;
}
struct tl_ds_messages_affected_history *fetch_ds_type_messages_affected_history (struct paramed_type *T) {
  assert (in_remaining () >= 4);
  int magic = fetch_int ();
  switch (magic) {
  case 0xb45c69d1: return fetch_ds_constructor_messages_affected_history (T); break;
  default: assert (0); return NULL;
  }
}
struct tl_ds_messages_affected_history *fetch_ds_type_bare_messages_affected_history (struct paramed_type *T) {
  return fetch_ds_constructor_messages_affected_history (T);
  assert (0);
  return NULL;
}
struct tl_ds_messages_affected_messages *fetch_ds_type_messages_affected_messages (struct paramed_type *T) {
  assert (in_remaining () >= 4);
  int magic = fetch_int ();
  switch (magic) {
  case 0x84d19185: return fetch_ds_constructor_messages_affected_messages (T); break;
  default: assert (0); return NULL;
  }
}
struct tl_ds_messages_affected_messages *fetch_ds_type_bare_messages_affected_messages (struct paramed_type *T) {
  return fetch_ds_constructor_messages_affected_messages (T);
  assert (0);
  return NULL;
}
struct tl_ds_messages_all_stickers *fetch_ds_type_messages_all_stickers (struct paramed_type *T) {
  assert (in_remaining () >= 4);
  int magic = fetch_int ();
  switch (magic) {
  case 0xe86602c3: return fetch_ds_constructor_messages_all_stickers_not_modified (T); break;
  case 0x5ce352ec: return fetch_ds_constructor_messages_all_stickers (T); break;
  default: assert (0); return NULL;
  }
}
struct tl_ds_messages_all_stickers *fetch_ds_type_bare_messages_all_stickers (struct paramed_type *T) {
  int *save_in_ptr = in_ptr;
  if (skip_constructor_messages_all_stickers_not_modified (T) >= 0) { in_ptr = save_in_ptr; return fetch_ds_constructor_messages_all_stickers_not_modified (T); }
  if (skip_constructor_messages_all_stickers (T) >= 0) { in_ptr = save_in_ptr; return fetch_ds_constructor_messages_all_stickers (T); }
  assert (0);
  return NULL;
}
struct tl_ds_messages_chat_full *fetch_ds_type_messages_chat_full (struct paramed_type *T) {
  assert (in_remaining () >= 4);
  int magic = fetch_int ();
  switch (magic) {
  case 0xe5d7d19c: return fetch_ds_constructor_messages_chat_full (T); break;
  default: assert (0); return NULL;
  }
}
struct tl_ds_messages_chat_full *fetch_ds_type_bare_messages_chat_full (struct paramed_type *T) {
  return fetch_ds_constructor_messages_chat_full (T);
  assert (0);
  return NULL;
}
struct tl_ds_messages_chats *fetch_ds_type_messages_chats (struct paramed_type *T) {
  assert (in_remaining () >= 4);
  int magic = fetch_int ();
  switch (magic) {
  case 0x64ff9fd5: return fetch_ds_constructor_messages_chats (T); break;
  default: assert (0); return NULL;
  }
}
struct tl_ds_messages_chats *fetch_ds_type_bare_messages_chats (struct paramed_type *T) {
  return fetch_ds_constructor_messages_chats (T);
  assert (0);
  return NULL;
}
struct tl_ds_messages_dh_config *fetch_ds_type_messages_dh_config (struct paramed_type *T) {
  assert (in_remaining () >= 4);
  int magic = fetch_int ();
  switch (magic) {
  case 0xc0e24635: return fetch_ds_constructor_messages_dh_config_not_modified (T); break;
  case 0x2c221edd: return fetch_ds_constructor_messages_dh_config (T); break;
  default: assert (0); return NULL;
  }
}
struct tl_ds_messages_dh_config *fetch_ds_type_bare_messages_dh_config (struct paramed_type *T) {
  int *save_in_ptr = in_ptr;
  if (skip_constructor_messages_dh_config_not_modified (T) >= 0) { in_ptr = save_in_ptr; return fetch_ds_constructor_messages_dh_config_not_modified (T); }
  if (skip_constructor_messages_dh_config (T) >= 0) { in_ptr = save_in_ptr; return fetch_ds_constructor_messages_dh_config (T); }
  assert (0);
  return NULL;
}
struct tl_ds_messages_dialogs *fetch_ds_type_messages_dialogs (struct paramed_type *T) {
  assert (in_remaining () >= 4);
  int magic = fetch_int ();
  switch (magic) {
  case 0x15ba6c40: return fetch_ds_constructor_messages_dialogs (T); break;
  case 0x71e094f3: return fetch_ds_constructor_messages_dialogs_slice (T); break;
  default: assert (0); return NULL;
  }
}
struct tl_ds_messages_dialogs *fetch_ds_type_bare_messages_dialogs (struct paramed_type *T) {
  int *save_in_ptr = in_ptr;
  if (skip_constructor_messages_dialogs (T) >= 0) { in_ptr = save_in_ptr; return fetch_ds_constructor_messages_dialogs (T); }
  if (skip_constructor_messages_dialogs_slice (T) >= 0) { in_ptr = save_in_ptr; return fetch_ds_constructor_messages_dialogs_slice (T); }
  assert (0);
  return NULL;
}
struct tl_ds_messages_message *fetch_ds_type_messages_message (struct paramed_type *T) {
  assert (in_remaining () >= 4);
  int magic = fetch_int ();
  switch (magic) {
  case 0x3f4e0648: return fetch_ds_constructor_messages_message_empty (T); break;
  default: assert (0); return NULL;
  }
}
struct tl_ds_messages_message *fetch_ds_type_bare_messages_message (struct paramed_type *T) {
  return fetch_ds_constructor_messages_message_empty (T);
  assert (0);
  return NULL;
}
struct tl_ds_messages_messages *fetch_ds_type_messages_messages (struct paramed_type *T) {
  assert (in_remaining () >= 4);
  int magic = fetch_int ();
  switch (magic) {
  case 0x8c718e87: return fetch_ds_constructor_messages_messages (T); break;
  case 0x0b446ae3: return fetch_ds_constructor_messages_messages_slice (T); break;
  default: assert (0); return NULL;
  }
}
struct tl_ds_messages_messages *fetch_ds_type_bare_messages_messages (struct paramed_type *T) {
  int *save_in_ptr = in_ptr;
  if (skip_constructor_messages_messages (T) >= 0) { in_ptr = save_in_ptr; return fetch_ds_constructor_messages_messages (T); }
  if (skip_constructor_messages_messages_slice (T) >= 0) { in_ptr = save_in_ptr; return fetch_ds_constructor_messages_messages_slice (T); }
  assert (0);
  return NULL;
}
struct tl_ds_messages_sent_encrypted_message *fetch_ds_type_messages_sent_encrypted_message (struct paramed_type *T) {
  assert (in_remaining () >= 4);
  int magic = fetch_int ();
  switch (magic) {
  case 0x560f8935: return fetch_ds_constructor_messages_sent_encrypted_message (T); break;
  case 0x9493ff32: return fetch_ds_constructor_messages_sent_encrypted_file (T); break;
  default: assert (0); return NULL;
  }
}
struct tl_ds_messages_sent_encrypted_message *fetch_ds_type_bare_messages_sent_encrypted_message (struct paramed_type *T) {
  int *save_in_ptr = in_ptr;
  if (skip_constructor_messages_sent_encrypted_message (T) >= 0) { in_ptr = save_in_ptr; return fetch_ds_constructor_messages_sent_encrypted_message (T); }
  if (skip_constructor_messages_sent_encrypted_file (T) >= 0) { in_ptr = save_in_ptr; return fetch_ds_constructor_messages_sent_encrypted_file (T); }
  assert (0);
  return NULL;
}
struct tl_ds_messages_sent_message *fetch_ds_type_messages_sent_message (struct paramed_type *T) {
  assert (in_remaining () >= 4);
  int magic = fetch_int ();
  switch (magic) {
  case 0x4c3d47f3: return fetch_ds_constructor_messages_sent_message (T); break;
  case 0x35a1a663: return fetch_ds_constructor_messages_sent_message_link (T); break;
  default: assert (0); return NULL;
  }
}
struct tl_ds_messages_sent_message *fetch_ds_type_bare_messages_sent_message (struct paramed_type *T) {
  int *save_in_ptr = in_ptr;
  if (skip_constructor_messages_sent_message (T) >= 0) { in_ptr = save_in_ptr; return fetch_ds_constructor_messages_sent_message (T); }
  if (skip_constructor_messages_sent_message_link (T) >= 0) { in_ptr = save_in_ptr; return fetch_ds_constructor_messages_sent_message_link (T); }
  assert (0);
  return NULL;
}
struct tl_ds_messages_sticker_set *fetch_ds_type_messages_sticker_set (struct paramed_type *T) {
  assert (in_remaining () >= 4);
  int magic = fetch_int ();
  switch (magic) {
  case 0xb60a24a6: return fetch_ds_constructor_messages_sticker_set (T); break;
  default: assert (0); return NULL;
  }
}
struct tl_ds_messages_sticker_set *fetch_ds_type_bare_messages_sticker_set (struct paramed_type *T) {
  return fetch_ds_constructor_messages_sticker_set (T);
  assert (0);
  return NULL;
}
struct tl_ds_messages_stickers *fetch_ds_type_messages_stickers (struct paramed_type *T) {
  assert (in_remaining () >= 4);
  int magic = fetch_int ();
  switch (magic) {
  case 0xf1749a22: return fetch_ds_constructor_messages_stickers_not_modified (T); break;
  case 0x8a8ecd32: return fetch_ds_constructor_messages_stickers (T); break;
  default: assert (0); return NULL;
  }
}
struct tl_ds_messages_stickers *fetch_ds_type_bare_messages_stickers (struct paramed_type *T) {
  int *save_in_ptr = in_ptr;
  if (skip_constructor_messages_stickers_not_modified (T) >= 0) { in_ptr = save_in_ptr; return fetch_ds_constructor_messages_stickers_not_modified (T); }
  if (skip_constructor_messages_stickers (T) >= 0) { in_ptr = save_in_ptr; return fetch_ds_constructor_messages_stickers (T); }
  assert (0);
  return NULL;
}
struct tl_ds_photos_photo *fetch_ds_type_photos_photo (struct paramed_type *T) {
  assert (in_remaining () >= 4);
  int magic = fetch_int ();
  switch (magic) {
  case 0x20212ca8: return fetch_ds_constructor_photos_photo (T); break;
  default: assert (0); return NULL;
  }
}
struct tl_ds_photos_photo *fetch_ds_type_bare_photos_photo (struct paramed_type *T) {
  return fetch_ds_constructor_photos_photo (T);
  assert (0);
  return NULL;
}
struct tl_ds_photos_photos *fetch_ds_type_photos_photos (struct paramed_type *T) {
  assert (in_remaining () >= 4);
  int magic = fetch_int ();
  switch (magic) {
  case 0x8dca6aa5: return fetch_ds_constructor_photos_photos (T); break;
  case 0x15051f54: return fetch_ds_constructor_photos_photos_slice (T); break;
  default: assert (0); return NULL;
  }
}
struct tl_ds_photos_photos *fetch_ds_type_bare_photos_photos (struct paramed_type *T) {
  int *save_in_ptr = in_ptr;
  if (skip_constructor_photos_photos (T) >= 0) { in_ptr = save_in_ptr; return fetch_ds_constructor_photos_photos (T); }
  if (skip_constructor_photos_photos_slice (T) >= 0) { in_ptr = save_in_ptr; return fetch_ds_constructor_photos_photos_slice (T); }
  assert (0);
  return NULL;
}
struct tl_ds_storage_file_type *fetch_ds_type_storage_file_type (struct paramed_type *T) {
  assert (in_remaining () >= 4);
  int magic = fetch_int ();
  switch (magic) {
  case 0xaa963b05: return fetch_ds_constructor_storage_file_unknown (T); break;
  case 0x007efe0e: return fetch_ds_constructor_storage_file_jpeg (T); break;
  case 0xcae1aadf: return fetch_ds_constructor_storage_file_gif (T); break;
  case 0x0a4f63c0: return fetch_ds_constructor_storage_file_png (T); break;
  case 0xae1e508d: return fetch_ds_constructor_storage_file_pdf (T); break;
  case 0x528a0677: return fetch_ds_constructor_storage_file_mp3 (T); break;
  case 0x4b09ebbc: return fetch_ds_constructor_storage_file_mov (T); break;
  case 0x40bc6f52: return fetch_ds_constructor_storage_file_partial (T); break;
  case 0xb3cea0e4: return fetch_ds_constructor_storage_file_mp4 (T); break;
  case 0x1081464c: return fetch_ds_constructor_storage_file_webp (T); break;
  default: assert (0); return NULL;
  }
}
struct tl_ds_storage_file_type *fetch_ds_type_bare_storage_file_type (struct paramed_type *T) {
  int *save_in_ptr = in_ptr;
  if (skip_constructor_storage_file_unknown (T) >= 0) { in_ptr = save_in_ptr; return fetch_ds_constructor_storage_file_unknown (T); }
  if (skip_constructor_storage_file_jpeg (T) >= 0) { in_ptr = save_in_ptr; return fetch_ds_constructor_storage_file_jpeg (T); }
  if (skip_constructor_storage_file_gif (T) >= 0) { in_ptr = save_in_ptr; return fetch_ds_constructor_storage_file_gif (T); }
  if (skip_constructor_storage_file_png (T) >= 0) { in_ptr = save_in_ptr; return fetch_ds_constructor_storage_file_png (T); }
  if (skip_constructor_storage_file_pdf (T) >= 0) { in_ptr = save_in_ptr; return fetch_ds_constructor_storage_file_pdf (T); }
  if (skip_constructor_storage_file_mp3 (T) >= 0) { in_ptr = save_in_ptr; return fetch_ds_constructor_storage_file_mp3 (T); }
  if (skip_constructor_storage_file_mov (T) >= 0) { in_ptr = save_in_ptr; return fetch_ds_constructor_storage_file_mov (T); }
  if (skip_constructor_storage_file_partial (T) >= 0) { in_ptr = save_in_ptr; return fetch_ds_constructor_storage_file_partial (T); }
  if (skip_constructor_storage_file_mp4 (T) >= 0) { in_ptr = save_in_ptr; return fetch_ds_constructor_storage_file_mp4 (T); }
  if (skip_constructor_storage_file_webp (T) >= 0) { in_ptr = save_in_ptr; return fetch_ds_constructor_storage_file_webp (T); }
  assert (0);
  return NULL;
}
struct tl_ds_updates_difference *fetch_ds_type_updates_difference (struct paramed_type *T) {
  assert (in_remaining () >= 4);
  int magic = fetch_int ();
  switch (magic) {
  case 0x5d75a138: return fetch_ds_constructor_updates_difference_empty (T); break;
  case 0x00f49ca0: return fetch_ds_constructor_updates_difference (T); break;
  case 0xa8fb1981: return fetch_ds_constructor_updates_difference_slice (T); break;
  default: assert (0); return NULL;
  }
}
struct tl_ds_updates_difference *fetch_ds_type_bare_updates_difference (struct paramed_type *T) {
  int *save_in_ptr = in_ptr;
  if (skip_constructor_updates_difference_empty (T) >= 0) { in_ptr = save_in_ptr; return fetch_ds_constructor_updates_difference_empty (T); }
  if (skip_constructor_updates_difference (T) >= 0) { in_ptr = save_in_ptr; return fetch_ds_constructor_updates_difference (T); }
  if (skip_constructor_updates_difference_slice (T) >= 0) { in_ptr = save_in_ptr; return fetch_ds_constructor_updates_difference_slice (T); }
  assert (0);
  return NULL;
}
struct tl_ds_updates_state *fetch_ds_type_updates_state (struct paramed_type *T) {
  assert (in_remaining () >= 4);
  int magic = fetch_int ();
  switch (magic) {
  case 0xa56c2a3e: return fetch_ds_constructor_updates_state (T); break;
  default: assert (0); return NULL;
  }
}
struct tl_ds_updates_state *fetch_ds_type_bare_updates_state (struct paramed_type *T) {
  return fetch_ds_constructor_updates_state (T);
  assert (0);
  return NULL;
}
struct tl_ds_upload_file *fetch_ds_type_upload_file (struct paramed_type *T) {
  assert (in_remaining () >= 4);
  int magic = fetch_int ();
  switch (magic) {
  case 0x096a18d5: return fetch_ds_constructor_upload_file (T); break;
  default: assert (0); return NULL;
  }
}
struct tl_ds_upload_file *fetch_ds_type_bare_upload_file (struct paramed_type *T) {
  return fetch_ds_constructor_upload_file (T);
  assert (0);
  return NULL;
}
void *fetch_ds_type_any (struct paramed_type *T) {
  switch (T->type->name) {
  case 0xb8d0afdf: return fetch_ds_type_account_days_t_t_l (T);
  case 0x472f5020: return fetch_ds_type_bare_account_days_t_t_l (T);
  case 0x9fc5ec4e: return fetch_ds_type_audio (T);
  case 0x603a13b1: return fetch_ds_type_bare_audio (T);
  case 0x7bf2e6f6: return fetch_ds_type_authorization (T);
  case 0x840d1909: return fetch_ds_type_bare_authorization (T);
  case 0x250be282: return fetch_ds_type_bool (T);
  case 0xdaf41d7d: return fetch_ds_type_bare_bool (T);
  case 0x75e7ea6c: return fetch_ds_type_bot_command (T);
  case 0x8a181593: return fetch_ds_type_bare_bot_command (T);
  case 0xb2e16f93: return fetch_ds_type_bot_info (T);
  case 0x4d1e906c: return fetch_ds_type_bare_bot_info (T);
  case 0x0ee1379f: return fetch_ds_type_bytes (T);
  case 0xf11ec860: return fetch_ds_type_bare_bytes (T);
  case 0x7bd865dc: return fetch_ds_type_chat (T);
  case 0x84279a23: return fetch_ds_type_bare_chat (T);
  case 0x2e02a614: return fetch_ds_type_chat_full (T);
  case 0xd1fd59eb: return fetch_ds_type_bare_chat_full (T);
  case 0x94f910b1: return fetch_ds_type_chat_invite (T);
  case 0x6b06ef4e: return fetch_ds_type_bare_chat_invite (T);
  case 0x3631cf4c: return fetch_ds_type_chat_located (T);
  case 0xc9ce30b3: return fetch_ds_type_bare_chat_located (T);
  case 0xc8d7493e: return fetch_ds_type_chat_participant (T);
  case 0x3728b6c1: return fetch_ds_type_bare_chat_participant (T);
  case 0x77930f9f: return fetch_ds_type_chat_participants (T);
  case 0x886cf060: return fetch_ds_type_bare_chat_participants (T);
  case 0x56922676: return fetch_ds_type_chat_photo (T);
  case 0xa96dd989: return fetch_ds_type_bare_chat_photo (T);
  case 0x6643b654: return fetch_ds_type_client_d_h_inner_data (T);
  case 0x99bc49ab: return fetch_ds_type_bare_client_d_h_inner_data (T);
  case 0x4e32b894: return fetch_ds_type_config (T);
  case 0xb1cd476b: return fetch_ds_type_bare_config (T);
  case 0xf911c994: return fetch_ds_type_contact (T);
  case 0x06ee366b: return fetch_ds_type_bare_contact (T);
  case 0x561bc879: return fetch_ds_type_contact_blocked (T);
  case 0xa9e43786: return fetch_ds_type_bare_contact_blocked (T);
  case 0xea879f95: return fetch_ds_type_contact_found (T);
  case 0x1578606a: return fetch_ds_type_bare_contact_found (T);
  case 0x522fbc63: return fetch_ds_type_contact_link (T);
  case 0xadd0439c: return fetch_ds_type_bare_contact_link (T);
  case 0xd3680c61: return fetch_ds_type_contact_status (T);
  case 0x2c97f39e: return fetch_ds_type_bare_contact_status (T);
  case 0x3de191a1: return fetch_ds_type_contact_suggested (T);
  case 0xc21e6e5e: return fetch_ds_type_bare_contact_suggested (T);
  case 0x2b1a62f0: return fetch_ds_type_dc_option (T);
  case 0xd4e59d0f: return fetch_ds_type_bare_dc_option (T);
  case 0x535b7918: return fetch_ds_type_decrypted_message (T);
  case 0xaca486e7: return fetch_ds_type_bare_decrypted_message (T);
  case 0x4e0eefde: return fetch_ds_type_decrypted_message_action (T);
  case 0xb1f11021: return fetch_ds_type_bare_decrypted_message_action (T);
  case 0x1be31789: return fetch_ds_type_decrypted_message_layer (T);
  case 0xe41ce876: return fetch_ds_type_bare_decrypted_message_layer (T);
  case 0x34e0d674: return fetch_ds_type_decrypted_message_media (T);
  case 0xcb1f298b: return fetch_ds_type_bare_decrypted_message_media (T);
  case 0xc1dd804a: return fetch_ds_type_dialog (T);
  case 0x3e227fb5: return fetch_ds_type_bare_dialog (T);
  case 0xae636f24: return fetch_ds_type_disabled_feature (T);
  case 0x519c90db: return fetch_ds_type_bare_disabled_feature (T);
  case 0x51a73418: return fetch_ds_type_document (T);
  case 0xae58cbe7: return fetch_ds_type_bare_document (T);
  case 0x97c637a3: return fetch_ds_type_document_attribute (T);
  case 0x6839c85c: return fetch_ds_type_bare_document_attribute (T);
  case 0x2210c154: return fetch_ds_type_double (T);
  case 0xddef3eab: return fetch_ds_type_bare_double (T);
  case 0xb1718213: return fetch_ds_type_encrypted_chat (T);
  case 0x4e8e7dec: return fetch_ds_type_bare_encrypted_chat (T);
  case 0x886fd032: return fetch_ds_type_encrypted_file (T);
  case 0x77902fcd: return fetch_ds_type_bare_encrypted_file (T);
  case 0xce6b8a1e: return fetch_ds_type_encrypted_message (T);
  case 0x319475e1: return fetch_ds_type_bare_encrypted_message (T);
  case 0xc4b9f9bb: return fetch_ds_type_error (T);
  case 0x3b460644: return fetch_ds_type_bare_error (T);
  case 0x95f132d5: return fetch_ds_type_exported_chat_invite (T);
  case 0x6a0ecd2a: return fetch_ds_type_bare_exported_chat_invite (T);
  case 0x2f8ffb30: return fetch_ds_type_file_location (T);
  case 0xd07004cf: return fetch_ds_type_bare_file_location (T);
  case 0xf67b4034: return fetch_ds_type_geo_chat_message (T);
  case 0x0984bfcb: return fetch_ds_type_bare_geo_chat_message (T);
  case 0x315e0a53: return fetch_ds_type_geo_point (T);
  case 0xcea1f5ac: return fetch_ds_type_bare_geo_point (T);
  case 0xd0028438: return fetch_ds_type_imported_contact (T);
  case 0x2ffd7bc7: return fetch_ds_type_bare_imported_contact (T);
  case 0x770656a8: return fetch_ds_type_input_app_event (T);
  case 0x88f9a957: return fetch_ds_type_bare_input_app_event (T);
  case 0xae8e9c7b: return fetch_ds_type_input_audio (T);
  case 0x51716384: return fetch_ds_type_bare_input_audio (T);
  case 0x3a60776d: return fetch_ds_type_input_chat_photo (T);
  case 0xc59f8892: return fetch_ds_type_bare_input_chat_photo (T);
  case 0xf392b7f4: return fetch_ds_type_input_contact (T);
  case 0x0c6d480b: return fetch_ds_type_bare_input_contact (T);
  case 0x6a8963fc: return fetch_ds_type_input_document (T);
  case 0x95769c03: return fetch_ds_type_bare_input_document (T);
  case 0xf141b5e1: return fetch_ds_type_input_encrypted_chat (T);
  case 0x0ebe4a1e: return fetch_ds_type_bare_input_encrypted_chat (T);
  case 0x0b5c064f: return fetch_ds_type_input_encrypted_file (T);
  case 0xf4a3f9b0: return fetch_ds_type_bare_input_encrypted_file (T);
  case 0x0f60f9ca: return fetch_ds_type_input_file (T);
  case 0xf09f0635: return fetch_ds_type_bare_input_file (T);
  case 0xe6daa38b: return fetch_ds_type_input_file_location (T);
  case 0x19255c74: return fetch_ds_type_bare_input_file_location (T);
  case 0x74d456fa: return fetch_ds_type_input_geo_chat (T);
  case 0x8b2ba905: return fetch_ds_type_bare_input_geo_chat (T);
  case 0x17768f1f: return fetch_ds_type_input_geo_point (T);
  case 0xe88970e0: return fetch_ds_type_bare_input_geo_point (T);
  case 0xb32c91dd: return fetch_ds_type_input_media (T);
  case 0x4cd36e22: return fetch_ds_type_bare_input_media (T);
  case 0x02b6911b: return fetch_ds_type_input_notify_peer (T);
  case 0xfd496ee4: return fetch_ds_type_bare_input_notify_peer (T);
  case 0x9e67268d: return fetch_ds_type_input_peer (T);
  case 0x6198d972: return fetch_ds_type_bare_input_peer (T);
  case 0x185a48ac: return fetch_ds_type_input_peer_notify_events (T);
  case 0xe7a5b753: return fetch_ds_type_bare_input_peer_notify_events (T);
  case 0x46a2ce98: return fetch_ds_type_input_peer_notify_settings (T);
  case 0xb95d3167: return fetch_ds_type_bare_input_peer_notify_settings (T);
  case 0xe74279c9: return fetch_ds_type_input_photo (T);
  case 0x18bd8636: return fetch_ds_type_bare_input_photo (T);
  case 0x7477e321: return fetch_ds_type_input_photo_crop (T);
  case 0x8b881cde: return fetch_ds_type_bare_input_photo_crop (T);
  case 0x4f96cb18: return fetch_ds_type_input_privacy_key (T);
  case 0xb06934e7: return fetch_ds_type_bare_input_privacy_key (T);
  case 0x4b815163: return fetch_ds_type_input_privacy_rule (T);
  case 0xb47eae9c: return fetch_ds_type_bare_input_privacy_rule (T);
  case 0xe44d415c: return fetch_ds_type_input_sticker_set (T);
  case 0x1bb2bea3: return fetch_ds_type_bare_input_sticker_set (T);
  case 0xadfe0c6a: return fetch_ds_type_input_user (T);
  case 0x5201f395: return fetch_ds_type_bare_input_user (T);
  case 0xbb5f7a27: return fetch_ds_type_input_video (T);
  case 0x44a085d8: return fetch_ds_type_bare_input_video (T);
  case 0xa8509bda: return fetch_ds_type_int (T);
  case 0x57af6425: return fetch_ds_type_bare_int (T);
  case 0x7d36c439: return fetch_ds_type_int128 (T);
  case 0x82c93bc6: return fetch_ds_type_bare_int128 (T);
  case 0xf2c798b3: return fetch_ds_type_int256 (T);
  case 0x0d38674c: return fetch_ds_type_bare_int256 (T);
  case 0xa2fa4880: return fetch_ds_type_keyboard_button (T);
  case 0x5d05b77f: return fetch_ds_type_bare_keyboard_button (T);
  case 0x77608b83: return fetch_ds_type_keyboard_button_row (T);
  case 0x889f747c: return fetch_ds_type_bare_keyboard_button_row (T);
  case 0x22076cba: return fetch_ds_type_long (T);
  case 0xddf89345: return fetch_ds_type_bare_long (T);
  case 0x5d652a7f: return fetch_ds_type_message (T);
  case 0xa29ad580: return fetch_ds_type_bare_message (T);
  case 0x383415fa: return fetch_ds_type_message_action (T);
  case 0xc7cbea05: return fetch_ds_type_bare_message_action (T);
  case 0x95f2471a: return fetch_ds_type_message_media (T);
  case 0x6a0db8e5: return fetch_ds_type_bare_message_media (T);
  case 0x80895ae0: return fetch_ds_type_messages_filter (T);
  case 0x7f76a51f: return fetch_ds_type_bare_messages_filter (T);
  case 0x8e1a1775: return fetch_ds_type_nearest_dc (T);
  case 0x71e5e88a: return fetch_ds_type_bare_nearest_dc (T);
  case 0x9fcb8237: return fetch_ds_type_notify_peer (T);
  case 0x60347dc8: return fetch_ds_type_bare_notify_peer (T);
  case 0x56730bcc: return fetch_ds_type_null (T);
  case 0xa98cf433: return fetch_ds_type_bare_null (T);
  case 0xbfa3de38: return fetch_ds_type_p_q_inner_data (T);
  case 0x405c21c7: return fetch_ds_type_bare_p_q_inner_data (T);
  case 0x276159d6: return fetch_ds_type_peer (T);
  case 0xd89ea629: return fetch_ds_type_bare_peer (T);
  case 0xc0c8d13b: return fetch_ds_type_peer_notify_events (T);
  case 0x3f372ec4: return fetch_ds_type_bare_peer_notify_events (T);
  case 0xfdf894fc: return fetch_ds_type_peer_notify_settings (T);
  case 0x02076b03: return fetch_ds_type_bare_peer_notify_settings (T);
  case 0xc207550a: return fetch_ds_type_photo (T);
  case 0x3df8aaf5: return fetch_ds_type_bare_photo (T);
  case 0x900f60dd: return fetch_ds_type_photo_size (T);
  case 0x6ff09f22: return fetch_ds_type_bare_photo_size (T);
  case 0xbc2eab30: return fetch_ds_type_privacy_key (T);
  case 0x43d154cf: return fetch_ds_type_bare_privacy_key (T);
  case 0xa8638aec: return fetch_ds_type_privacy_rule (T);
  case 0x579c7513: return fetch_ds_type_bare_privacy_rule (T);
  case 0xa384b779: return fetch_ds_type_received_notify_message (T);
  case 0x5c7b4886: return fetch_ds_type_bare_received_notify_message (T);
  case 0x612ca4a9: return fetch_ds_type_reply_markup (T);
  case 0x9ed35b56: return fetch_ds_type_bare_reply_markup (T);
  case 0x05162463: return fetch_ds_type_res_p_q (T);
  case 0xfae9db9c: return fetch_ds_type_bare_res_p_q (T);
  case 0x70541c7e: return fetch_ds_type_send_message_action (T);
  case 0x8fabe381: return fetch_ds_type_bare_send_message_action (T);
  case 0xa9230301: return fetch_ds_type_server_d_h_params (T);
  case 0x56dcfcfe: return fetch_ds_type_bare_server_d_h_params (T);
  case 0xb5890dba: return fetch_ds_type_server_d_h_inner_data (T);
  case 0x4a76f245: return fetch_ds_type_bare_server_d_h_inner_data (T);
  case 0xdb8a468f: return fetch_ds_type_set_client_d_h_params_answer (T);
  case 0x2475b970: return fetch_ds_type_bare_set_client_d_h_params_answer (T);
  case 0x12b299d4: return fetch_ds_type_sticker_pack (T);
  case 0xed4d662b: return fetch_ds_type_bare_sticker_pack (T);
  case 0xa7a43b17: return fetch_ds_type_sticker_set (T);
  case 0x585bc4e8: return fetch_ds_type_bare_sticker_set (T);
  case 0xb5286e24: return fetch_ds_type_string (T);
  case 0x4ad791db: return fetch_ds_type_bare_string (T);
  case 0x2e02b27e: return fetch_ds_type_update (T);
  case 0xd1fd4d81: return fetch_ds_type_bare_update (T);
  case 0x22499d27: return fetch_ds_type_updates (T);
  case 0xddb662d8: return fetch_ds_type_bare_updates (T);
  case 0x02e6c0c8: return fetch_ds_type_user (T);
  case 0xfd193f37: return fetch_ds_type_bare_user (T);
  case 0x5a89ac5b: return fetch_ds_type_user_full (T);
  case 0xa57653a4: return fetch_ds_type_bare_user_full (T);
  case 0x9a486229: return fetch_ds_type_user_profile_photo (T);
  case 0x65b79dd6: return fetch_ds_type_bare_user_profile_photo (T);
  case 0x76de9570: return fetch_ds_type_user_status (T);
  case 0x89216a8f: return fetch_ds_type_bare_user_status (T);
  case 0x1cb5c415: return fetch_ds_type_vector (T);
  case 0xe34a3bea: return fetch_ds_type_bare_vector (T);
  case 0x1716b174: return fetch_ds_type_video (T);
  case 0xe8e94e8b: return fetch_ds_type_bare_video (T);
  case 0xafa14973: return fetch_ds_type_wall_paper (T);
  case 0x505eb68c: return fetch_ds_type_bare_wall_paper (T);
  case 0x8d8c0d41: return fetch_ds_type_web_page (T);
  case 0x7273f2be: return fetch_ds_type_bare_web_page (T);
  case 0x1250abde: return fetch_ds_type_account_authorizations (T);
  case 0xedaf5421: return fetch_ds_type_bare_account_authorizations (T);
  case 0xeac2a804: return fetch_ds_type_account_password (T);
  case 0x153d57fb: return fetch_ds_type_bare_account_password (T);
  case 0xbcfc532c: return fetch_ds_type_account_password_input_settings (T);
  case 0x4303acd3: return fetch_ds_type_bare_account_password_input_settings (T);
  case 0xb7b72ab3: return fetch_ds_type_account_password_settings (T);
  case 0x4848d54c: return fetch_ds_type_bare_account_password_settings (T);
  case 0x554abb6f: return fetch_ds_type_account_privacy_rules (T);
  case 0xaab54490: return fetch_ds_type_bare_account_privacy_rules (T);
  case 0xa4f58c4c: return fetch_ds_type_account_sent_change_phone_code (T);
  case 0x5b0a73b3: return fetch_ds_type_bare_account_sent_change_phone_code (T);
  case 0xff036af1: return fetch_ds_type_auth_authorization (T);
  case 0x00fc950e: return fetch_ds_type_bare_auth_authorization (T);
  case 0x811ea28e: return fetch_ds_type_auth_checked_phone (T);
  case 0x7ee15d71: return fetch_ds_type_bare_auth_checked_phone (T);
  case 0xdf969c2d: return fetch_ds_type_auth_exported_authorization (T);
  case 0x206963d2: return fetch_ds_type_bare_auth_exported_authorization (T);
  case 0x137948a5: return fetch_ds_type_auth_password_recovery (T);
  case 0xec86b75a: return fetch_ds_type_bare_auth_password_recovery (T);
  case 0x0cc8bc16: return fetch_ds_type_auth_sent_code (T);
  case 0xf33743e9: return fetch_ds_type_bare_auth_sent_code (T);
  case 0x0377168f: return fetch_ds_type_binlog_encr_key (T);
  case 0xfc88e970: return fetch_ds_type_bare_binlog_encr_key (T);
  case 0x66ff03a6: return fetch_ds_type_binlog_update (T);
  case 0x9900fc59: return fetch_ds_type_bare_binlog_update (T);
  case 0x8c1b8fb4: return fetch_ds_type_contacts_blocked (T);
  case 0x73e4704b: return fetch_ds_type_bare_contacts_blocked (T);
  case 0xd8c02560: return fetch_ds_type_contacts_contacts (T);
  case 0x273fda9f: return fetch_ds_type_bare_contacts_contacts (T);
  case 0x0566000e: return fetch_ds_type_contacts_found (T);
  case 0xfa99fff1: return fetch_ds_type_bare_contacts_found (T);
  case 0xad524315: return fetch_ds_type_contacts_imported_contacts (T);
  case 0x52adbcea: return fetch_ds_type_bare_contacts_imported_contacts (T);
  case 0x3ace484c: return fetch_ds_type_contacts_link (T);
  case 0xc531b7b3: return fetch_ds_type_bare_contacts_link (T);
  case 0x5649dcc5: return fetch_ds_type_contacts_suggested (T);
  case 0xa9b6233a: return fetch_ds_type_bare_contacts_suggested (T);
  case 0x48feb267: return fetch_ds_type_geochats_located (T);
  case 0xb7014d98: return fetch_ds_type_bare_geochats_located (T);
  case 0x6d0a0e59: return fetch_ds_type_geochats_messages (T);
  case 0x92f5f1a6: return fetch_ds_type_bare_geochats_messages (T);
  case 0x17b1578b: return fetch_ds_type_geochats_stated_message (T);
  case 0xe84ea874: return fetch_ds_type_bare_geochats_stated_message (T);
  case 0x4ddd9627: return fetch_ds_type_help_app_update (T);
  case 0xb22269d8: return fetch_ds_type_bare_help_app_update (T);
  case 0x18cb9f78: return fetch_ds_type_help_invite_text (T);
  case 0xe7346087: return fetch_ds_type_bare_help_invite_text (T);
  case 0x17c6b5f6: return fetch_ds_type_help_support (T);
  case 0xe8394a09: return fetch_ds_type_bare_help_support (T);
  case 0xb45c69d1: return fetch_ds_type_messages_affected_history (T);
  case 0x4ba3962e: return fetch_ds_type_bare_messages_affected_history (T);
  case 0x84d19185: return fetch_ds_type_messages_affected_messages (T);
  case 0x7b2e6e7a: return fetch_ds_type_bare_messages_affected_messages (T);
  case 0xb485502f: return fetch_ds_type_messages_all_stickers (T);
  case 0x4b7aafd0: return fetch_ds_type_bare_messages_all_stickers (T);
  case 0xe5d7d19c: return fetch_ds_type_messages_chat_full (T);
  case 0x1a282e63: return fetch_ds_type_bare_messages_chat_full (T);
  case 0x64ff9fd5: return fetch_ds_type_messages_chats (T);
  case 0x9b00602a: return fetch_ds_type_bare_messages_chats (T);
  case 0xecc058e8: return fetch_ds_type_messages_dh_config (T);
  case 0x133fa717: return fetch_ds_type_bare_messages_dh_config (T);
  case 0x645af8b3: return fetch_ds_type_messages_dialogs (T);
  case 0x9ba5074c: return fetch_ds_type_bare_messages_dialogs (T);
  case 0x3f4e0648: return fetch_ds_type_messages_message (T);
  case 0xc0b1f9b7: return fetch_ds_type_bare_messages_message (T);
  case 0x8735e464: return fetch_ds_type_messages_messages (T);
  case 0x78ca1b9b: return fetch_ds_type_bare_messages_messages (T);
  case 0xc29c7607: return fetch_ds_type_messages_sent_encrypted_message (T);
  case 0x3d6389f8: return fetch_ds_type_bare_messages_sent_encrypted_message (T);
  case 0x799ce190: return fetch_ds_type_messages_sent_message (T);
  case 0x86631e6f: return fetch_ds_type_bare_messages_sent_message (T);
  case 0xb60a24a6: return fetch_ds_type_messages_sticker_set (T);
  case 0x49f5db59: return fetch_ds_type_bare_messages_sticker_set (T);
  case 0x7bfa5710: return fetch_ds_type_messages_stickers (T);
  case 0x8405a8ef: return fetch_ds_type_bare_messages_stickers (T);
  case 0x20212ca8: return fetch_ds_type_photos_photo (T);
  case 0xdfded357: return fetch_ds_type_bare_photos_photo (T);
  case 0x98cf75f1: return fetch_ds_type_photos_photos (T);
  case 0x67308a0e: return fetch_ds_type_bare_photos_photos (T);
  case 0x3e2838a8: return fetch_ds_type_storage_file_type (T);
  case 0xc1d7c757: return fetch_ds_type_bare_storage_file_type (T);
  case 0xf57a2419: return fetch_ds_type_updates_difference (T);
  case 0x0a85dbe6: return fetch_ds_type_bare_updates_difference (T);
  case 0xa56c2a3e: return fetch_ds_type_updates_state (T);
  case 0x5a93d5c1: return fetch_ds_type_bare_updates_state (T);
  case 0x096a18d5: return fetch_ds_type_upload_file (T);
  case 0xf695e72a: return fetch_ds_type_bare_upload_file (T);
  default: return NULL; }
}
