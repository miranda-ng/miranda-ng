#ifdef _MSC_VER
#include "../auto.h"
#include "auto-store-ds.h"
#include "auto-skip.h"
#include "auto-types.h"
#include "../mtproto-common.h"
#else
#include "auto.h"
#include "auto/auto-store-ds.h"
#include "auto/auto-skip.h"
#include "auto/auto-types.h"
#include "auto-static-store-ds.c"
#include "mtproto-common.h"
#endif
#include <assert.h>
void store_ds_constructor_account_days_t_t_l (struct tl_ds_account_days_t_t_l *D, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0xb8d0afdf && T->type->name != 0x472f5020)) { return; }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_int (D->days, field1);
}
void store_ds_constructor_audio_empty (struct tl_ds_audio *D, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x9fc5ec4e && T->type->name != 0x603a13b1)) { return; }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xddf89345, .id = "Bare_Long", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_long (D->id, field1);
}
void store_ds_constructor_audio (struct tl_ds_audio *D, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x9fc5ec4e && T->type->name != 0x603a13b1)) { return; }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xddf89345, .id = "Bare_Long", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_long (D->id, field1);
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xddf89345, .id = "Bare_Long", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_long (D->access_hash, field2);
  struct paramed_type *field3 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_int (D->user_id, field3);
  struct paramed_type *field4 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_int (D->date, field4);
  struct paramed_type *field5 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_int (D->duration, field5);
  struct paramed_type *field6 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_string (D->mime_type, field6);
  struct paramed_type *field7 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_int (D->size, field7);
  struct paramed_type *field8 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_int (D->dc_id, field8);
}
void store_ds_constructor_authorization (struct tl_ds_authorization *D, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x7bf2e6f6 && T->type->name != 0x840d1909)) { return; }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xddf89345, .id = "Bare_Long", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_long (D->hash, field1);
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_int (D->flags, field2);
  struct paramed_type *field3 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_string (D->device_model, field3);
  struct paramed_type *field4 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_string (D->platform, field4);
  struct paramed_type *field5 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_string (D->system_version, field5);
  struct paramed_type *field6 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_int (D->api_id, field6);
  struct paramed_type *field7 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_string (D->app_name, field7);
  struct paramed_type *field8 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_string (D->app_version, field8);
  struct paramed_type *field9 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_int (D->date_created, field9);
  struct paramed_type *field10 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_int (D->date_active, field10);
  struct paramed_type *field11 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_string (D->ip, field11);
  struct paramed_type *field12 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_string (D->country, field12);
  struct paramed_type *field13 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_string (D->region, field13);
}
void store_ds_constructor_bool_false (struct tl_ds_bool *D, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x250be282 && T->type->name != 0xdaf41d7d)) { return; }
}
void store_ds_constructor_bool_true (struct tl_ds_bool *D, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x250be282 && T->type->name != 0xdaf41d7d)) { return; }
}
void store_ds_constructor_bot_command (struct tl_ds_bot_command *D, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x75e7ea6c && T->type->name != 0x8a181593)) { return; }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_string (D->command, field1);
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_string (D->description, field2);
}
void store_ds_constructor_bot_command_old (struct tl_ds_bot_command *D, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x75e7ea6c && T->type->name != 0x8a181593)) { return; }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_string (D->command, field1);
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_string (D->params, field2);
  struct paramed_type *field3 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_string (D->description, field3);
}
void store_ds_constructor_bot_info_empty (struct tl_ds_bot_info *D, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0xb2e16f93 && T->type->name != 0x4d1e906c)) { return; }
}
void store_ds_constructor_bot_info (struct tl_ds_bot_info *D, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0xb2e16f93 && T->type->name != 0x4d1e906c)) { return; }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_int (D->user_id, field1);
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_int (D->version, field2);
  struct paramed_type *field3 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_string (D->share_text, field3);
  struct paramed_type *field4 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_string (D->description, field4);
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
  store_ds_type_vector ((void *)D->commands, field5);
}
void store_ds_constructor_bytes (struct tl_ds_string *D, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x0ee1379f && T->type->name != 0xf11ec860)) { return; }
  out_cstring (D->data, D->len);
}
void store_ds_constructor_chat_empty (struct tl_ds_chat *D, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x7bd865dc && T->type->name != 0x84279a23)) { return; }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_int (D->id, field1);
}
void store_ds_constructor_chat (struct tl_ds_chat *D, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x7bd865dc && T->type->name != 0x84279a23)) { return; }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_int (D->id, field1);
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_string (D->title, field2);
  struct paramed_type *field3 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x56922676, .id = "ChatPhoto", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_chat_photo (D->photo, field3);
  struct paramed_type *field4 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_int (D->participants_count, field4);
  struct paramed_type *field5 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_int (D->date, field5);
  struct paramed_type *field6 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x250be282, .id = "Bool", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bool (D->left, field6);
  struct paramed_type *field7 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_int (D->version, field7);
}
void store_ds_constructor_chat_forbidden (struct tl_ds_chat *D, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x7bd865dc && T->type->name != 0x84279a23)) { return; }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_int (D->id, field1);
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_string (D->title, field2);
  struct paramed_type *field3 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_int (D->date, field3);
}
void store_ds_constructor_geo_chat (struct tl_ds_chat *D, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x7bd865dc && T->type->name != 0x84279a23)) { return; }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_int (D->id, field1);
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xddf89345, .id = "Bare_Long", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_long (D->access_hash, field2);
  struct paramed_type *field3 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_string (D->title, field3);
  struct paramed_type *field4 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_string (D->address, field4);
  struct paramed_type *field5 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_string (D->venue, field5);
  struct paramed_type *field6 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x315e0a53, .id = "GeoPoint", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_geo_point (D->geo, field6);
  struct paramed_type *field7 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x56922676, .id = "ChatPhoto", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_chat_photo (D->photo, field7);
  struct paramed_type *field8 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_int (D->participants_count, field8);
  struct paramed_type *field9 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_int (D->date, field9);
  struct paramed_type *field10 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x250be282, .id = "Bool", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bool (D->checked_in, field10);
  struct paramed_type *field11 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_int (D->version, field11);
}
void store_ds_constructor_chat_full (struct tl_ds_chat_full *D, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x2e02a614 && T->type->name != 0xd1fd59eb)) { return; }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_int (D->id, field1);
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x77930f9f, .id = "ChatParticipants", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_chat_participants (D->participants, field2);
  struct paramed_type *field3 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xc207550a, .id = "Photo", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_photo (D->chat_photo, field3);
  struct paramed_type *field4 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xfdf894fc, .id = "PeerNotifySettings", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_peer_notify_settings (D->notify_settings, field4);
  struct paramed_type *field5 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x95f132d5, .id = "ExportedChatInvite", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_exported_chat_invite (D->exported_invite, field5);
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
  store_ds_type_vector ((void *)D->bot_info, field6);
}
void store_ds_constructor_chat_invite_already (struct tl_ds_chat_invite *D, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x94f910b1 && T->type->name != 0x6b06ef4e)) { return; }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x7bd865dc, .id = "Chat", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_chat (D->chat, field1);
}
void store_ds_constructor_chat_invite (struct tl_ds_chat_invite *D, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x94f910b1 && T->type->name != 0x6b06ef4e)) { return; }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_string (D->title, field1);
}
void store_ds_constructor_chat_located (struct tl_ds_chat_located *D, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x3631cf4c && T->type->name != 0xc9ce30b3)) { return; }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_int (D->chat_id, field1);
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_int (D->distance, field2);
}
void store_ds_constructor_chat_participant (struct tl_ds_chat_participant *D, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0xc8d7493e && T->type->name != 0x3728b6c1)) { return; }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_int (D->user_id, field1);
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_int (D->inviter_id, field2);
  struct paramed_type *field3 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_int (D->date, field3);
}
void store_ds_constructor_chat_participants_forbidden (struct tl_ds_chat_participants *D, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x77930f9f && T->type->name != 0x886cf060)) { return; }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_int (D->chat_id, field1);
}
void store_ds_constructor_chat_participants (struct tl_ds_chat_participants *D, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x77930f9f && T->type->name != 0x886cf060)) { return; }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_int (D->chat_id, field1);
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_int (D->admin_id, field2);
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
  store_ds_type_vector ((void *)D->participants, field3);
  struct paramed_type *field4 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_int (D->version, field4);
}
void store_ds_constructor_chat_photo_empty (struct tl_ds_chat_photo *D, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x56922676 && T->type->name != 0xa96dd989)) { return; }
}
void store_ds_constructor_chat_photo (struct tl_ds_chat_photo *D, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x56922676 && T->type->name != 0xa96dd989)) { return; }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x2f8ffb30, .id = "FileLocation", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_file_location (D->photo_small, field1);
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x2f8ffb30, .id = "FileLocation", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_file_location (D->photo_big, field2);
}
void store_ds_constructor_client_d_h_inner_data (struct tl_ds_client_d_h_inner_data *D, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x6643b654 && T->type->name != 0x99bc49ab)) { return; }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x82c93bc6, .id = "Bare_Int128", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_int128 (D->nonce, field1);
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x82c93bc6, .id = "Bare_Int128", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_int128 (D->server_nonce, field2);
  struct paramed_type *field3 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xddf89345, .id = "Bare_Long", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_long (D->retry_id, field3);
  struct paramed_type *field4 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_string (D->g_b, field4);
}
void store_ds_constructor_config (struct tl_ds_config *D, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x4e32b894 && T->type->name != 0xb1cd476b)) { return; }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_int (D->date, field1);
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_int (D->expires, field2);
  struct paramed_type *field3 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x250be282, .id = "Bool", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bool (D->test_mode, field3);
  struct paramed_type *field4 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_int (D->this_dc, field4);
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
  store_ds_type_vector ((void *)D->dc_options, field5);
  struct paramed_type *field6 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_int (D->chat_size_max, field6);
  struct paramed_type *field7 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_int (D->broadcast_size_max, field7);
  struct paramed_type *field8 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_int (D->forwarded_count_max, field8);
  struct paramed_type *field9 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_int (D->online_update_period_ms, field9);
  struct paramed_type *field10 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_int (D->offline_blur_timeout_ms, field10);
  struct paramed_type *field11 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_int (D->offline_idle_timeout_ms, field11);
  struct paramed_type *field12 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_int (D->online_cloud_timeout_ms, field12);
  struct paramed_type *field13 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_int (D->notify_cloud_delay_ms, field13);
  struct paramed_type *field14 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_int (D->notify_default_delay_ms, field14);
  struct paramed_type *field15 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_int (D->chat_big_size, field15);
  struct paramed_type *field16 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_int (D->push_chat_period_ms, field16);
  struct paramed_type *field17 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_int (D->push_chat_limit, field17);
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
  store_ds_type_vector ((void *)D->disabled_features, field18);
}
void store_ds_constructor_contact (struct tl_ds_contact *D, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0xf911c994 && T->type->name != 0x06ee366b)) { return; }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_int (D->user_id, field1);
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x250be282, .id = "Bool", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bool (D->mutual, field2);
}
void store_ds_constructor_contact_blocked (struct tl_ds_contact_blocked *D, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x561bc879 && T->type->name != 0xa9e43786)) { return; }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_int (D->user_id, field1);
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_int (D->date, field2);
}
void store_ds_constructor_contact_found (struct tl_ds_contact_found *D, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0xea879f95 && T->type->name != 0x1578606a)) { return; }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_int (D->user_id, field1);
}
void store_ds_constructor_contact_link_unknown (struct tl_ds_contact_link *D, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x522fbc63 && T->type->name != 0xadd0439c)) { return; }
}
void store_ds_constructor_contact_link_none (struct tl_ds_contact_link *D, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x522fbc63 && T->type->name != 0xadd0439c)) { return; }
}
void store_ds_constructor_contact_link_has_phone (struct tl_ds_contact_link *D, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x522fbc63 && T->type->name != 0xadd0439c)) { return; }
}
void store_ds_constructor_contact_link_contact (struct tl_ds_contact_link *D, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x522fbc63 && T->type->name != 0xadd0439c)) { return; }
}
void store_ds_constructor_contact_status (struct tl_ds_contact_status *D, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0xd3680c61 && T->type->name != 0x2c97f39e)) { return; }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_int (D->user_id, field1);
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x76de9570, .id = "UserStatus", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_user_status (D->status, field2);
}
void store_ds_constructor_contact_suggested (struct tl_ds_contact_suggested *D, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x3de191a1 && T->type->name != 0xc21e6e5e)) { return; }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_int (D->user_id, field1);
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_int (D->mutual_contacts, field2);
}
void store_ds_constructor_dc_option_l28 (struct tl_ds_dc_option *D, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x2b1a62f0 && T->type->name != 0xd4e59d0f)) { return; }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_int (D->id, field1);
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_string (D->hostname, field2);
  struct paramed_type *field3 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_string (D->ip_address, field3);
  struct paramed_type *field4 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_int (D->port, field4);
}
void store_ds_constructor_dc_option (struct tl_ds_dc_option *D, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x2b1a62f0 && T->type->name != 0xd4e59d0f)) { return; }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_int (D->flags, field1);
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_int (D->id, field2);
  struct paramed_type *field3 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_string (D->ip_address, field3);
  struct paramed_type *field4 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_int (D->port, field4);
}
void store_ds_constructor_decrypted_message (struct tl_ds_decrypted_message *D, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x535b7918 && T->type->name != 0xaca486e7)) { return; }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xddf89345, .id = "Bare_Long", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_long (D->random_id, field1);
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_int (D->ttl, field2);
  struct paramed_type *field3 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_string (D->message, field3);
  struct paramed_type *field4 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x34e0d674, .id = "DecryptedMessageMedia", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_decrypted_message_media (D->media, field4);
}
void store_ds_constructor_decrypted_message_service (struct tl_ds_decrypted_message *D, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x535b7918 && T->type->name != 0xaca486e7)) { return; }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xddf89345, .id = "Bare_Long", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_long (D->random_id, field1);
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x4e0eefde, .id = "DecryptedMessageAction", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_decrypted_message_action (D->action, field2);
}
void store_ds_constructor_decrypted_message_action_set_message_t_t_l (struct tl_ds_decrypted_message_action *D, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x4e0eefde && T->type->name != 0xb1f11021)) { return; }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_int (D->ttl_seconds, field1);
}
void store_ds_constructor_decrypted_message_action_read_messages (struct tl_ds_decrypted_message_action *D, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x4e0eefde && T->type->name != 0xb1f11021)) { return; }
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
  store_ds_type_vector ((void *)D->random_ids, field1);
}
void store_ds_constructor_decrypted_message_action_delete_messages (struct tl_ds_decrypted_message_action *D, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x4e0eefde && T->type->name != 0xb1f11021)) { return; }
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
  store_ds_type_vector ((void *)D->random_ids, field1);
}
void store_ds_constructor_decrypted_message_action_screenshot_messages (struct tl_ds_decrypted_message_action *D, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x4e0eefde && T->type->name != 0xb1f11021)) { return; }
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
  store_ds_type_vector ((void *)D->random_ids, field1);
}
void store_ds_constructor_decrypted_message_action_flush_history (struct tl_ds_decrypted_message_action *D, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x4e0eefde && T->type->name != 0xb1f11021)) { return; }
}
void store_ds_constructor_decrypted_message_action_resend (struct tl_ds_decrypted_message_action *D, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x4e0eefde && T->type->name != 0xb1f11021)) { return; }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_int (D->start_seq_no, field1);
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_int (D->end_seq_no, field2);
}
void store_ds_constructor_decrypted_message_action_notify_layer (struct tl_ds_decrypted_message_action *D, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x4e0eefde && T->type->name != 0xb1f11021)) { return; }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_int (D->layer, field1);
}
void store_ds_constructor_decrypted_message_action_typing (struct tl_ds_decrypted_message_action *D, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x4e0eefde && T->type->name != 0xb1f11021)) { return; }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x70541c7e, .id = "SendMessageAction", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_send_message_action (D->action, field1);
}
void store_ds_constructor_decrypted_message_action_request_key (struct tl_ds_decrypted_message_action *D, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x4e0eefde && T->type->name != 0xb1f11021)) { return; }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xddf89345, .id = "Bare_Long", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_long (D->exchange_id, field1);
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xf11ec860, .id = "Bare_Bytes", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_bytes (D->g_a, field2);
}
void store_ds_constructor_decrypted_message_action_accept_key (struct tl_ds_decrypted_message_action *D, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x4e0eefde && T->type->name != 0xb1f11021)) { return; }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xddf89345, .id = "Bare_Long", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_long (D->exchange_id, field1);
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xf11ec860, .id = "Bare_Bytes", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_bytes (D->g_b, field2);
  struct paramed_type *field3 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xddf89345, .id = "Bare_Long", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_long (D->key_fingerprint, field3);
}
void store_ds_constructor_decrypted_message_action_abort_key (struct tl_ds_decrypted_message_action *D, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x4e0eefde && T->type->name != 0xb1f11021)) { return; }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xddf89345, .id = "Bare_Long", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_long (D->exchange_id, field1);
}
void store_ds_constructor_decrypted_message_action_commit_key (struct tl_ds_decrypted_message_action *D, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x4e0eefde && T->type->name != 0xb1f11021)) { return; }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xddf89345, .id = "Bare_Long", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_long (D->exchange_id, field1);
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xddf89345, .id = "Bare_Long", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_long (D->key_fingerprint, field2);
}
void store_ds_constructor_decrypted_message_action_noop (struct tl_ds_decrypted_message_action *D, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x4e0eefde && T->type->name != 0xb1f11021)) { return; }
}
void store_ds_constructor_decrypted_message_layer (struct tl_ds_decrypted_message_layer *D, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x1be31789 && T->type->name != 0xe41ce876)) { return; }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xf11ec860, .id = "Bare_Bytes", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_bytes (D->random_bytes, field1);
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_int (D->layer, field2);
  struct paramed_type *field3 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_int (D->in_seq_no, field3);
  struct paramed_type *field4 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_int (D->out_seq_no, field4);
  struct paramed_type *field5 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x535b7918, .id = "DecryptedMessage", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_decrypted_message (D->message, field5);
}
void store_ds_constructor_decrypted_message_media_empty (struct tl_ds_decrypted_message_media *D, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x34e0d674 && T->type->name != 0xcb1f298b)) { return; }
}
void store_ds_constructor_decrypted_message_media_photo (struct tl_ds_decrypted_message_media *D, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x34e0d674 && T->type->name != 0xcb1f298b)) { return; }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xf11ec860, .id = "Bare_Bytes", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_bytes (D->str_thumb, field1);
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_int (D->thumb_w, field2);
  struct paramed_type *field3 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_int (D->thumb_h, field3);
  struct paramed_type *field4 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_int (D->w, field4);
  struct paramed_type *field5 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_int (D->h, field5);
  struct paramed_type *field6 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_int (D->size, field6);
  struct paramed_type *field7 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xf11ec860, .id = "Bare_Bytes", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_bytes (D->key, field7);
  struct paramed_type *field8 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xf11ec860, .id = "Bare_Bytes", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_bytes (D->iv, field8);
}
void store_ds_constructor_decrypted_message_media_geo_point (struct tl_ds_decrypted_message_media *D, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x34e0d674 && T->type->name != 0xcb1f298b)) { return; }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xddef3eab, .id = "Bare_Double", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_double (D->latitude, field1);
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xddef3eab, .id = "Bare_Double", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_double (D->longitude, field2);
}
void store_ds_constructor_decrypted_message_media_contact (struct tl_ds_decrypted_message_media *D, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x34e0d674 && T->type->name != 0xcb1f298b)) { return; }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_string (D->phone_number, field1);
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_string (D->first_name, field2);
  struct paramed_type *field3 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_string (D->last_name, field3);
  struct paramed_type *field4 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_int (D->user_id, field4);
}
void store_ds_constructor_decrypted_message_media_document (struct tl_ds_decrypted_message_media *D, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x34e0d674 && T->type->name != 0xcb1f298b)) { return; }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xf11ec860, .id = "Bare_Bytes", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_bytes (D->str_thumb, field1);
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_int (D->thumb_w, field2);
  struct paramed_type *field3 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_int (D->thumb_h, field3);
  struct paramed_type *field4 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_string (D->file_name, field4);
  struct paramed_type *field5 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_string (D->mime_type, field5);
  struct paramed_type *field6 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_int (D->size, field6);
  struct paramed_type *field7 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xf11ec860, .id = "Bare_Bytes", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_bytes (D->key, field7);
  struct paramed_type *field8 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xf11ec860, .id = "Bare_Bytes", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_bytes (D->iv, field8);
}
void store_ds_constructor_decrypted_message_media_video (struct tl_ds_decrypted_message_media *D, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x34e0d674 && T->type->name != 0xcb1f298b)) { return; }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xf11ec860, .id = "Bare_Bytes", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_bytes (D->str_thumb, field1);
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_int (D->thumb_w, field2);
  struct paramed_type *field3 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_int (D->thumb_h, field3);
  struct paramed_type *field4 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_int (D->duration, field4);
  struct paramed_type *field5 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_string (D->mime_type, field5);
  struct paramed_type *field6 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_int (D->w, field6);
  struct paramed_type *field7 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_int (D->h, field7);
  struct paramed_type *field8 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_int (D->size, field8);
  struct paramed_type *field9 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xf11ec860, .id = "Bare_Bytes", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_bytes (D->key, field9);
  struct paramed_type *field10 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xf11ec860, .id = "Bare_Bytes", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_bytes (D->iv, field10);
}
void store_ds_constructor_decrypted_message_media_audio (struct tl_ds_decrypted_message_media *D, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x34e0d674 && T->type->name != 0xcb1f298b)) { return; }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_int (D->duration, field1);
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_string (D->mime_type, field2);
  struct paramed_type *field3 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_int (D->size, field3);
  struct paramed_type *field4 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xf11ec860, .id = "Bare_Bytes", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_bytes (D->key, field4);
  struct paramed_type *field5 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xf11ec860, .id = "Bare_Bytes", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_bytes (D->iv, field5);
}
void store_ds_constructor_decrypted_message_media_external_document (struct tl_ds_decrypted_message_media *D, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x34e0d674 && T->type->name != 0xcb1f298b)) { return; }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xddf89345, .id = "Bare_Long", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_long (D->id, field1);
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xddf89345, .id = "Bare_Long", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_long (D->access_hash, field2);
  struct paramed_type *field3 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_int (D->date, field3);
  struct paramed_type *field4 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_string (D->mime_type, field4);
  struct paramed_type *field5 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_int (D->size, field5);
  struct paramed_type *field6 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x900f60dd, .id = "PhotoSize", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_photo_size (D->thumb, field6);
  struct paramed_type *field7 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_int (D->dc_id, field7);
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
  store_ds_type_vector ((void *)D->attributes, field8);
}
void store_ds_constructor_decrypted_message_media_video_l12 (struct tl_ds_decrypted_message_media *D, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x34e0d674 && T->type->name != 0xcb1f298b)) { return; }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xf11ec860, .id = "Bare_Bytes", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_bytes (D->str_thumb, field1);
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_int (D->thumb_w, field2);
  struct paramed_type *field3 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_int (D->thumb_h, field3);
  struct paramed_type *field4 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_int (D->duration, field4);
  struct paramed_type *field5 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_int (D->w, field5);
  struct paramed_type *field6 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_int (D->h, field6);
  struct paramed_type *field7 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_int (D->size, field7);
  struct paramed_type *field8 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xf11ec860, .id = "Bare_Bytes", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_bytes (D->key, field8);
  struct paramed_type *field9 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xf11ec860, .id = "Bare_Bytes", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_bytes (D->iv, field9);
}
void store_ds_constructor_decrypted_message_media_audio_l12 (struct tl_ds_decrypted_message_media *D, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x34e0d674 && T->type->name != 0xcb1f298b)) { return; }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_int (D->duration, field1);
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_int (D->size, field2);
  struct paramed_type *field3 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xf11ec860, .id = "Bare_Bytes", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_bytes (D->key, field3);
  struct paramed_type *field4 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xf11ec860, .id = "Bare_Bytes", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_bytes (D->iv, field4);
}
void store_ds_constructor_dialog (struct tl_ds_dialog *D, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0xc1dd804a && T->type->name != 0x3e227fb5)) { return; }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x276159d6, .id = "Peer", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_peer (D->peer, field1);
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_int (D->top_message, field2);
  struct paramed_type *field3 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_int (D->read_inbox_max_id, field3);
  struct paramed_type *field4 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_int (D->unread_count, field4);
  struct paramed_type *field5 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xfdf894fc, .id = "PeerNotifySettings", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_peer_notify_settings (D->notify_settings, field5);
}
void store_ds_constructor_disabled_feature (struct tl_ds_disabled_feature *D, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0xae636f24 && T->type->name != 0x519c90db)) { return; }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_string (D->feature, field1);
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_string (D->description, field2);
}
void store_ds_constructor_document_empty (struct tl_ds_document *D, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x51a73418 && T->type->name != 0xae58cbe7)) { return; }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xddf89345, .id = "Bare_Long", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_long (D->id, field1);
}
void store_ds_constructor_document (struct tl_ds_document *D, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x51a73418 && T->type->name != 0xae58cbe7)) { return; }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xddf89345, .id = "Bare_Long", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_long (D->id, field1);
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xddf89345, .id = "Bare_Long", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_long (D->access_hash, field2);
  struct paramed_type *field3 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_int (D->date, field3);
  struct paramed_type *field4 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_string (D->mime_type, field4);
  struct paramed_type *field5 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_int (D->size, field5);
  struct paramed_type *field6 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x900f60dd, .id = "PhotoSize", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_photo_size (D->thumb, field6);
  struct paramed_type *field7 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_int (D->dc_id, field7);
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
  store_ds_type_vector ((void *)D->attributes, field8);
}
void store_ds_constructor_document_l19 (struct tl_ds_document *D, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x51a73418 && T->type->name != 0xae58cbe7)) { return; }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xddf89345, .id = "Bare_Long", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_long (D->id, field1);
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xddf89345, .id = "Bare_Long", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_long (D->access_hash, field2);
  struct paramed_type *field3 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_int (D->user_id, field3);
  struct paramed_type *field4 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_int (D->date, field4);
  struct paramed_type *field5 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_string (D->file_name, field5);
  struct paramed_type *field6 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_string (D->mime_type, field6);
  struct paramed_type *field7 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_int (D->size, field7);
  struct paramed_type *field8 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x900f60dd, .id = "PhotoSize", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_photo_size (D->thumb, field8);
  struct paramed_type *field9 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_int (D->dc_id, field9);
}
void store_ds_constructor_document_attribute_image_size (struct tl_ds_document_attribute *D, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x97c637a3 && T->type->name != 0x6839c85c)) { return; }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_int (D->w, field1);
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_int (D->h, field2);
}
void store_ds_constructor_document_attribute_animated (struct tl_ds_document_attribute *D, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x97c637a3 && T->type->name != 0x6839c85c)) { return; }
}
void store_ds_constructor_document_attribute_sticker_l28 (struct tl_ds_document_attribute *D, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x97c637a3 && T->type->name != 0x6839c85c)) { return; }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_string (D->alt, field1);
}
void store_ds_constructor_document_attribute_sticker (struct tl_ds_document_attribute *D, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x97c637a3 && T->type->name != 0x6839c85c)) { return; }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_string (D->alt, field1);
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xe44d415c, .id = "InputStickerSet", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_input_sticker_set (D->stickerset, field2);
}
void store_ds_constructor_document_attribute_video (struct tl_ds_document_attribute *D, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x97c637a3 && T->type->name != 0x6839c85c)) { return; }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_int (D->duration, field1);
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_int (D->w, field2);
  struct paramed_type *field3 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_int (D->h, field3);
}
void store_ds_constructor_document_attribute_audio (struct tl_ds_document_attribute *D, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x97c637a3 && T->type->name != 0x6839c85c)) { return; }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_int (D->duration, field1);
}
void store_ds_constructor_document_attribute_filename (struct tl_ds_document_attribute *D, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x97c637a3 && T->type->name != 0x6839c85c)) { return; }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_string (D->file_name, field1);
}
void store_ds_constructor_double (double *D, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x2210c154 && T->type->name != 0xddef3eab)) { return; }
  out_double (*D);
}
void store_ds_constructor_encrypted_chat_empty (struct tl_ds_encrypted_chat *D, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0xb1718213 && T->type->name != 0x4e8e7dec)) { return; }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_int (D->id, field1);
}
void store_ds_constructor_encrypted_chat_waiting (struct tl_ds_encrypted_chat *D, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0xb1718213 && T->type->name != 0x4e8e7dec)) { return; }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_int (D->id, field1);
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xddf89345, .id = "Bare_Long", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_long (D->access_hash, field2);
  struct paramed_type *field3 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_int (D->date, field3);
  struct paramed_type *field4 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_int (D->admin_id, field4);
  struct paramed_type *field5 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_int (D->participant_id, field5);
}
void store_ds_constructor_encrypted_chat_requested (struct tl_ds_encrypted_chat *D, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0xb1718213 && T->type->name != 0x4e8e7dec)) { return; }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_int (D->id, field1);
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xddf89345, .id = "Bare_Long", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_long (D->access_hash, field2);
  struct paramed_type *field3 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_int (D->date, field3);
  struct paramed_type *field4 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_int (D->admin_id, field4);
  struct paramed_type *field5 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_int (D->participant_id, field5);
  struct paramed_type *field6 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xf11ec860, .id = "Bare_Bytes", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_bytes (D->g_a, field6);
}
void store_ds_constructor_encrypted_chat (struct tl_ds_encrypted_chat *D, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0xb1718213 && T->type->name != 0x4e8e7dec)) { return; }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_int (D->id, field1);
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xddf89345, .id = "Bare_Long", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_long (D->access_hash, field2);
  struct paramed_type *field3 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_int (D->date, field3);
  struct paramed_type *field4 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_int (D->admin_id, field4);
  struct paramed_type *field5 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_int (D->participant_id, field5);
  struct paramed_type *field6 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xf11ec860, .id = "Bare_Bytes", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_bytes (D->g_a_or_b, field6);
  struct paramed_type *field7 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xddf89345, .id = "Bare_Long", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_long (D->key_fingerprint, field7);
}
void store_ds_constructor_encrypted_chat_discarded (struct tl_ds_encrypted_chat *D, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0xb1718213 && T->type->name != 0x4e8e7dec)) { return; }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_int (D->id, field1);
}
void store_ds_constructor_encrypted_file_empty (struct tl_ds_encrypted_file *D, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x886fd032 && T->type->name != 0x77902fcd)) { return; }
}
void store_ds_constructor_encrypted_file (struct tl_ds_encrypted_file *D, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x886fd032 && T->type->name != 0x77902fcd)) { return; }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xddf89345, .id = "Bare_Long", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_long (D->id, field1);
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xddf89345, .id = "Bare_Long", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_long (D->access_hash, field2);
  struct paramed_type *field3 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_int (D->size, field3);
  struct paramed_type *field4 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_int (D->dc_id, field4);
  struct paramed_type *field5 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_int (D->key_fingerprint, field5);
}
void store_ds_constructor_encrypted_message (struct tl_ds_encrypted_message *D, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0xce6b8a1e && T->type->name != 0x319475e1)) { return; }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xddf89345, .id = "Bare_Long", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_long (D->random_id, field1);
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_int (D->chat_id, field2);
  struct paramed_type *field3 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_int (D->date, field3);
  struct paramed_type *field4 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xf11ec860, .id = "Bare_Bytes", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_bytes (D->bytes, field4);
  struct paramed_type *field5 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x886fd032, .id = "EncryptedFile", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_encrypted_file (D->file, field5);
}
void store_ds_constructor_encrypted_message_service (struct tl_ds_encrypted_message *D, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0xce6b8a1e && T->type->name != 0x319475e1)) { return; }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xddf89345, .id = "Bare_Long", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_long (D->random_id, field1);
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_int (D->chat_id, field2);
  struct paramed_type *field3 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_int (D->date, field3);
  struct paramed_type *field4 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xf11ec860, .id = "Bare_Bytes", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_bytes (D->bytes, field4);
}
void store_ds_constructor_error (struct tl_ds_error *D, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0xc4b9f9bb && T->type->name != 0x3b460644)) { return; }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_int (D->code, field1);
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_string (D->text, field2);
}
void store_ds_constructor_chat_invite_empty (struct tl_ds_exported_chat_invite *D, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x95f132d5 && T->type->name != 0x6a0ecd2a)) { return; }
}
void store_ds_constructor_chat_invite_exported (struct tl_ds_exported_chat_invite *D, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x95f132d5 && T->type->name != 0x6a0ecd2a)) { return; }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_string (D->link, field1);
}
void store_ds_constructor_file_location_unavailable (struct tl_ds_file_location *D, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x2f8ffb30 && T->type->name != 0xd07004cf)) { return; }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xddf89345, .id = "Bare_Long", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_long (D->volume_id, field1);
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_int (D->local_id, field2);
  struct paramed_type *field3 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xddf89345, .id = "Bare_Long", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_long (D->secret, field3);
}
void store_ds_constructor_file_location (struct tl_ds_file_location *D, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x2f8ffb30 && T->type->name != 0xd07004cf)) { return; }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_int (D->dc_id, field1);
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xddf89345, .id = "Bare_Long", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_long (D->volume_id, field2);
  struct paramed_type *field3 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_int (D->local_id, field3);
  struct paramed_type *field4 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xddf89345, .id = "Bare_Long", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_long (D->secret, field4);
}
void store_ds_constructor_geo_chat_message_empty (struct tl_ds_geo_chat_message *D, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0xf67b4034 && T->type->name != 0x0984bfcb)) { return; }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_int (D->chat_id, field1);
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_int (D->id, field2);
}
void store_ds_constructor_geo_chat_message (struct tl_ds_geo_chat_message *D, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0xf67b4034 && T->type->name != 0x0984bfcb)) { return; }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_int (D->chat_id, field1);
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_int (D->id, field2);
  struct paramed_type *field3 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_int (D->from_id, field3);
  struct paramed_type *field4 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_int (D->date, field4);
  struct paramed_type *field5 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_string (D->message, field5);
  struct paramed_type *field6 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x95f2471a, .id = "MessageMedia", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_message_media (D->media, field6);
}
void store_ds_constructor_geo_chat_message_service (struct tl_ds_geo_chat_message *D, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0xf67b4034 && T->type->name != 0x0984bfcb)) { return; }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_int (D->chat_id, field1);
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_int (D->id, field2);
  struct paramed_type *field3 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_int (D->from_id, field3);
  struct paramed_type *field4 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_int (D->date, field4);
  struct paramed_type *field5 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x383415fa, .id = "MessageAction", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_message_action (D->action, field5);
}
void store_ds_constructor_geo_point_empty (struct tl_ds_geo_point *D, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x315e0a53 && T->type->name != 0xcea1f5ac)) { return; }
}
void store_ds_constructor_geo_point (struct tl_ds_geo_point *D, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x315e0a53 && T->type->name != 0xcea1f5ac)) { return; }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xddef3eab, .id = "Bare_Double", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_double (D->longitude, field1);
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xddef3eab, .id = "Bare_Double", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_double (D->latitude, field2);
}
void store_ds_constructor_imported_contact (struct tl_ds_imported_contact *D, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0xd0028438 && T->type->name != 0x2ffd7bc7)) { return; }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_int (D->user_id, field1);
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xddf89345, .id = "Bare_Long", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_long (D->client_id, field2);
}
void store_ds_constructor_input_app_event (struct tl_ds_input_app_event *D, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x770656a8 && T->type->name != 0x88f9a957)) { return; }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xddef3eab, .id = "Bare_Double", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_double (D->time, field1);
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_string (D->type, field2);
  struct paramed_type *field3 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xddf89345, .id = "Bare_Long", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_long (D->peer, field3);
  struct paramed_type *field4 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_string (D->data, field4);
}
void store_ds_constructor_input_audio_empty (struct tl_ds_input_audio *D, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0xae8e9c7b && T->type->name != 0x51716384)) { return; }
}
void store_ds_constructor_input_audio (struct tl_ds_input_audio *D, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0xae8e9c7b && T->type->name != 0x51716384)) { return; }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xddf89345, .id = "Bare_Long", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_long (D->id, field1);
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xddf89345, .id = "Bare_Long", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_long (D->access_hash, field2);
}
void store_ds_constructor_input_chat_photo_empty (struct tl_ds_input_chat_photo *D, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x3a60776d && T->type->name != 0xc59f8892)) { return; }
}
void store_ds_constructor_input_chat_uploaded_photo (struct tl_ds_input_chat_photo *D, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x3a60776d && T->type->name != 0xc59f8892)) { return; }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x0f60f9ca, .id = "InputFile", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_input_file (D->file, field1);
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x7477e321, .id = "InputPhotoCrop", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_input_photo_crop (D->crop, field2);
}
void store_ds_constructor_input_chat_photo (struct tl_ds_input_chat_photo *D, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x3a60776d && T->type->name != 0xc59f8892)) { return; }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xe74279c9, .id = "InputPhoto", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_input_photo (D->id, field1);
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x7477e321, .id = "InputPhotoCrop", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_input_photo_crop (D->crop, field2);
}
void store_ds_constructor_input_phone_contact (struct tl_ds_input_contact *D, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0xf392b7f4 && T->type->name != 0x0c6d480b)) { return; }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xddf89345, .id = "Bare_Long", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_long (D->client_id, field1);
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_string (D->phone, field2);
  struct paramed_type *field3 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_string (D->first_name, field3);
  struct paramed_type *field4 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_string (D->last_name, field4);
}
void store_ds_constructor_input_document_empty (struct tl_ds_input_document *D, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x6a8963fc && T->type->name != 0x95769c03)) { return; }
}
void store_ds_constructor_input_document (struct tl_ds_input_document *D, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x6a8963fc && T->type->name != 0x95769c03)) { return; }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xddf89345, .id = "Bare_Long", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_long (D->id, field1);
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xddf89345, .id = "Bare_Long", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_long (D->access_hash, field2);
}
void store_ds_constructor_input_encrypted_chat (struct tl_ds_input_encrypted_chat *D, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0xf141b5e1 && T->type->name != 0x0ebe4a1e)) { return; }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_int (D->chat_id, field1);
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xddf89345, .id = "Bare_Long", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_long (D->access_hash, field2);
}
void store_ds_constructor_input_encrypted_file_empty (struct tl_ds_input_encrypted_file *D, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x0b5c064f && T->type->name != 0xf4a3f9b0)) { return; }
}
void store_ds_constructor_input_encrypted_file_uploaded (struct tl_ds_input_encrypted_file *D, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x0b5c064f && T->type->name != 0xf4a3f9b0)) { return; }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xddf89345, .id = "Bare_Long", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_long (D->id, field1);
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_int (D->parts, field2);
  struct paramed_type *field3 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_string (D->md5_checksum, field3);
  struct paramed_type *field4 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_int (D->key_fingerprint, field4);
}
void store_ds_constructor_input_encrypted_file (struct tl_ds_input_encrypted_file *D, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x0b5c064f && T->type->name != 0xf4a3f9b0)) { return; }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xddf89345, .id = "Bare_Long", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_long (D->id, field1);
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xddf89345, .id = "Bare_Long", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_long (D->access_hash, field2);
}
void store_ds_constructor_input_encrypted_file_big_uploaded (struct tl_ds_input_encrypted_file *D, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x0b5c064f && T->type->name != 0xf4a3f9b0)) { return; }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xddf89345, .id = "Bare_Long", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_long (D->id, field1);
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_int (D->parts, field2);
  struct paramed_type *field3 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_int (D->key_fingerprint, field3);
}
void store_ds_constructor_input_file (struct tl_ds_input_file *D, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x0f60f9ca && T->type->name != 0xf09f0635)) { return; }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xddf89345, .id = "Bare_Long", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_long (D->id, field1);
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_int (D->parts, field2);
  struct paramed_type *field3 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_string (D->name, field3);
  struct paramed_type *field4 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_string (D->md5_checksum, field4);
}
void store_ds_constructor_input_file_big (struct tl_ds_input_file *D, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x0f60f9ca && T->type->name != 0xf09f0635)) { return; }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xddf89345, .id = "Bare_Long", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_long (D->id, field1);
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_int (D->parts, field2);
  struct paramed_type *field3 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_string (D->name, field3);
}
void store_ds_constructor_input_file_location (struct tl_ds_input_file_location *D, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0xe6daa38b && T->type->name != 0x19255c74)) { return; }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xddf89345, .id = "Bare_Long", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_long (D->volume_id, field1);
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_int (D->local_id, field2);
  struct paramed_type *field3 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xddf89345, .id = "Bare_Long", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_long (D->secret, field3);
}
void store_ds_constructor_input_video_file_location (struct tl_ds_input_file_location *D, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0xe6daa38b && T->type->name != 0x19255c74)) { return; }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xddf89345, .id = "Bare_Long", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_long (D->id, field1);
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xddf89345, .id = "Bare_Long", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_long (D->access_hash, field2);
}
void store_ds_constructor_input_encrypted_file_location (struct tl_ds_input_file_location *D, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0xe6daa38b && T->type->name != 0x19255c74)) { return; }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xddf89345, .id = "Bare_Long", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_long (D->id, field1);
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xddf89345, .id = "Bare_Long", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_long (D->access_hash, field2);
}
void store_ds_constructor_input_audio_file_location (struct tl_ds_input_file_location *D, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0xe6daa38b && T->type->name != 0x19255c74)) { return; }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xddf89345, .id = "Bare_Long", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_long (D->id, field1);
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xddf89345, .id = "Bare_Long", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_long (D->access_hash, field2);
}
void store_ds_constructor_input_document_file_location (struct tl_ds_input_file_location *D, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0xe6daa38b && T->type->name != 0x19255c74)) { return; }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xddf89345, .id = "Bare_Long", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_long (D->id, field1);
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xddf89345, .id = "Bare_Long", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_long (D->access_hash, field2);
}
void store_ds_constructor_input_geo_chat (struct tl_ds_input_geo_chat *D, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x74d456fa && T->type->name != 0x8b2ba905)) { return; }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_int (D->chat_id, field1);
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xddf89345, .id = "Bare_Long", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_long (D->access_hash, field2);
}
void store_ds_constructor_input_geo_point_empty (struct tl_ds_input_geo_point *D, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x17768f1f && T->type->name != 0xe88970e0)) { return; }
}
void store_ds_constructor_input_geo_point (struct tl_ds_input_geo_point *D, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x17768f1f && T->type->name != 0xe88970e0)) { return; }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xddef3eab, .id = "Bare_Double", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_double (D->latitude, field1);
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xddef3eab, .id = "Bare_Double", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_double (D->longitude, field2);
}
void store_ds_constructor_input_media_empty (struct tl_ds_input_media *D, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0xb32c91dd && T->type->name != 0x4cd36e22)) { return; }
}
void store_ds_constructor_input_media_uploaded_photo (struct tl_ds_input_media *D, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0xb32c91dd && T->type->name != 0x4cd36e22)) { return; }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x0f60f9ca, .id = "InputFile", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_input_file (D->file, field1);
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_string (D->caption, field2);
}
void store_ds_constructor_input_media_photo (struct tl_ds_input_media *D, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0xb32c91dd && T->type->name != 0x4cd36e22)) { return; }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xe74279c9, .id = "InputPhoto", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_input_photo (D->id, field1);
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_string (D->caption, field2);
}
void store_ds_constructor_input_media_geo_point (struct tl_ds_input_media *D, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0xb32c91dd && T->type->name != 0x4cd36e22)) { return; }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x17768f1f, .id = "InputGeoPoint", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_input_geo_point (D->geo_point, field1);
}
void store_ds_constructor_input_media_contact (struct tl_ds_input_media *D, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0xb32c91dd && T->type->name != 0x4cd36e22)) { return; }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_string (D->phone_number, field1);
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_string (D->first_name, field2);
  struct paramed_type *field3 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_string (D->last_name, field3);
}
void store_ds_constructor_input_media_uploaded_video (struct tl_ds_input_media *D, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0xb32c91dd && T->type->name != 0x4cd36e22)) { return; }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x0f60f9ca, .id = "InputFile", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_input_file (D->file, field1);
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_int (D->duration, field2);
  struct paramed_type *field3 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_int (D->w, field3);
  struct paramed_type *field4 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_int (D->h, field4);
  struct paramed_type *field5 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_string (D->caption, field5);
}
void store_ds_constructor_input_media_uploaded_thumb_video (struct tl_ds_input_media *D, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0xb32c91dd && T->type->name != 0x4cd36e22)) { return; }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x0f60f9ca, .id = "InputFile", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_input_file (D->file, field1);
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x0f60f9ca, .id = "InputFile", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_input_file (D->thumb, field2);
  struct paramed_type *field3 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_int (D->duration, field3);
  struct paramed_type *field4 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_int (D->w, field4);
  struct paramed_type *field5 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_int (D->h, field5);
  struct paramed_type *field6 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_string (D->caption, field6);
}
void store_ds_constructor_input_media_video (struct tl_ds_input_media *D, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0xb32c91dd && T->type->name != 0x4cd36e22)) { return; }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xbb5f7a27, .id = "InputVideo", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_input_video (D->video_id, field1);
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_string (D->caption, field2);
}
void store_ds_constructor_input_media_uploaded_audio (struct tl_ds_input_media *D, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0xb32c91dd && T->type->name != 0x4cd36e22)) { return; }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x0f60f9ca, .id = "InputFile", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_input_file (D->file, field1);
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_int (D->duration, field2);
  struct paramed_type *field3 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_string (D->mime_type, field3);
}
void store_ds_constructor_input_media_audio (struct tl_ds_input_media *D, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0xb32c91dd && T->type->name != 0x4cd36e22)) { return; }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xae8e9c7b, .id = "InputAudio", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_input_audio (D->audio_id, field1);
}
void store_ds_constructor_input_media_uploaded_document (struct tl_ds_input_media *D, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0xb32c91dd && T->type->name != 0x4cd36e22)) { return; }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x0f60f9ca, .id = "InputFile", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_input_file (D->file, field1);
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_string (D->mime_type, field2);
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
  store_ds_type_vector ((void *)D->attributes, field3);
}
void store_ds_constructor_input_media_uploaded_thumb_document (struct tl_ds_input_media *D, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0xb32c91dd && T->type->name != 0x4cd36e22)) { return; }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x0f60f9ca, .id = "InputFile", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_input_file (D->file, field1);
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x0f60f9ca, .id = "InputFile", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_input_file (D->thumb, field2);
  struct paramed_type *field3 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_string (D->mime_type, field3);
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
  store_ds_type_vector ((void *)D->attributes, field4);
}
void store_ds_constructor_input_media_document (struct tl_ds_input_media *D, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0xb32c91dd && T->type->name != 0x4cd36e22)) { return; }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x6a8963fc, .id = "InputDocument", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_input_document (D->document_id, field1);
}
void store_ds_constructor_input_media_venue (struct tl_ds_input_media *D, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0xb32c91dd && T->type->name != 0x4cd36e22)) { return; }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x17768f1f, .id = "InputGeoPoint", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_input_geo_point (D->geo_point, field1);
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_string (D->title, field2);
  struct paramed_type *field3 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_string (D->address, field3);
  struct paramed_type *field4 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_string (D->provider, field4);
  struct paramed_type *field5 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_string (D->venue_id, field5);
}
void store_ds_constructor_input_notify_peer (struct tl_ds_input_notify_peer *D, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x02b6911b && T->type->name != 0xfd496ee4)) { return; }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x9e67268d, .id = "InputPeer", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_input_peer (D->peer, field1);
}
void store_ds_constructor_input_notify_users (struct tl_ds_input_notify_peer *D, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x02b6911b && T->type->name != 0xfd496ee4)) { return; }
}
void store_ds_constructor_input_notify_chats (struct tl_ds_input_notify_peer *D, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x02b6911b && T->type->name != 0xfd496ee4)) { return; }
}
void store_ds_constructor_input_notify_all (struct tl_ds_input_notify_peer *D, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x02b6911b && T->type->name != 0xfd496ee4)) { return; }
}
void store_ds_constructor_input_notify_geo_chat_peer (struct tl_ds_input_notify_peer *D, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x02b6911b && T->type->name != 0xfd496ee4)) { return; }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x74d456fa, .id = "InputGeoChat", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_input_geo_chat (D->geo_peer, field1);
}
void store_ds_constructor_input_peer_empty (struct tl_ds_input_peer *D, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x9e67268d && T->type->name != 0x6198d972)) { return; }
}
void store_ds_constructor_input_peer_self (struct tl_ds_input_peer *D, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x9e67268d && T->type->name != 0x6198d972)) { return; }
}
void store_ds_constructor_input_peer_contact (struct tl_ds_input_peer *D, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x9e67268d && T->type->name != 0x6198d972)) { return; }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_int (D->user_id, field1);
}
void store_ds_constructor_input_peer_foreign (struct tl_ds_input_peer *D, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x9e67268d && T->type->name != 0x6198d972)) { return; }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_int (D->user_id, field1);
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xddf89345, .id = "Bare_Long", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_long (D->access_hash, field2);
}
void store_ds_constructor_input_peer_chat (struct tl_ds_input_peer *D, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x9e67268d && T->type->name != 0x6198d972)) { return; }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_int (D->chat_id, field1);
}
void store_ds_constructor_input_peer_notify_events_empty (struct tl_ds_input_peer_notify_events *D, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x185a48ac && T->type->name != 0xe7a5b753)) { return; }
}
void store_ds_constructor_input_peer_notify_events_all (struct tl_ds_input_peer_notify_events *D, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x185a48ac && T->type->name != 0xe7a5b753)) { return; }
}
void store_ds_constructor_input_peer_notify_settings (struct tl_ds_input_peer_notify_settings *D, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x46a2ce98 && T->type->name != 0xb95d3167)) { return; }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_int (D->mute_until, field1);
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_string (D->sound, field2);
  struct paramed_type *field3 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x250be282, .id = "Bool", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bool (D->show_previews, field3);
  struct paramed_type *field4 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_int (D->events_mask, field4);
}
void store_ds_constructor_input_photo_empty (struct tl_ds_input_photo *D, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0xe74279c9 && T->type->name != 0x18bd8636)) { return; }
}
void store_ds_constructor_input_photo (struct tl_ds_input_photo *D, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0xe74279c9 && T->type->name != 0x18bd8636)) { return; }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xddf89345, .id = "Bare_Long", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_long (D->id, field1);
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xddf89345, .id = "Bare_Long", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_long (D->access_hash, field2);
}
void store_ds_constructor_input_photo_crop_auto (struct tl_ds_input_photo_crop *D, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x7477e321 && T->type->name != 0x8b881cde)) { return; }
}
void store_ds_constructor_input_photo_crop (struct tl_ds_input_photo_crop *D, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x7477e321 && T->type->name != 0x8b881cde)) { return; }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xddef3eab, .id = "Bare_Double", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_double (D->crop_left, field1);
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xddef3eab, .id = "Bare_Double", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_double (D->crop_top, field2);
  struct paramed_type *field3 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xddef3eab, .id = "Bare_Double", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_double (D->crop_width, field3);
}
void store_ds_constructor_input_privacy_key_status_timestamp (struct tl_ds_input_privacy_key *D, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x4f96cb18 && T->type->name != 0xb06934e7)) { return; }
}
void store_ds_constructor_input_privacy_value_allow_contacts (struct tl_ds_input_privacy_rule *D, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x4b815163 && T->type->name != 0xb47eae9c)) { return; }
}
void store_ds_constructor_input_privacy_value_allow_all (struct tl_ds_input_privacy_rule *D, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x4b815163 && T->type->name != 0xb47eae9c)) { return; }
}
void store_ds_constructor_input_privacy_value_allow_users (struct tl_ds_input_privacy_rule *D, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x4b815163 && T->type->name != 0xb47eae9c)) { return; }
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
  store_ds_type_vector ((void *)D->users, field1);
}
void store_ds_constructor_input_privacy_value_disallow_contacts (struct tl_ds_input_privacy_rule *D, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x4b815163 && T->type->name != 0xb47eae9c)) { return; }
}
void store_ds_constructor_input_privacy_value_disallow_all (struct tl_ds_input_privacy_rule *D, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x4b815163 && T->type->name != 0xb47eae9c)) { return; }
}
void store_ds_constructor_input_privacy_value_disallow_users (struct tl_ds_input_privacy_rule *D, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x4b815163 && T->type->name != 0xb47eae9c)) { return; }
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
  store_ds_type_vector ((void *)D->users, field1);
}
void store_ds_constructor_input_sticker_set_empty (struct tl_ds_input_sticker_set *D, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0xe44d415c && T->type->name != 0x1bb2bea3)) { return; }
}
void store_ds_constructor_input_sticker_set_i_d (struct tl_ds_input_sticker_set *D, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0xe44d415c && T->type->name != 0x1bb2bea3)) { return; }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xddf89345, .id = "Bare_Long", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_long (D->id, field1);
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xddf89345, .id = "Bare_Long", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_long (D->access_hash, field2);
}
void store_ds_constructor_input_sticker_set_short_name (struct tl_ds_input_sticker_set *D, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0xe44d415c && T->type->name != 0x1bb2bea3)) { return; }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_string (D->short_name, field1);
}
void store_ds_constructor_input_user_empty (struct tl_ds_input_user *D, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0xadfe0c6a && T->type->name != 0x5201f395)) { return; }
}
void store_ds_constructor_input_user_self (struct tl_ds_input_user *D, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0xadfe0c6a && T->type->name != 0x5201f395)) { return; }
}
void store_ds_constructor_input_user_contact (struct tl_ds_input_user *D, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0xadfe0c6a && T->type->name != 0x5201f395)) { return; }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_int (D->user_id, field1);
}
void store_ds_constructor_input_user_foreign (struct tl_ds_input_user *D, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0xadfe0c6a && T->type->name != 0x5201f395)) { return; }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_int (D->user_id, field1);
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xddf89345, .id = "Bare_Long", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_long (D->access_hash, field2);
}
void store_ds_constructor_input_video_empty (struct tl_ds_input_video *D, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0xbb5f7a27 && T->type->name != 0x44a085d8)) { return; }
}
void store_ds_constructor_input_video (struct tl_ds_input_video *D, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0xbb5f7a27 && T->type->name != 0x44a085d8)) { return; }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xddf89345, .id = "Bare_Long", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_long (D->id, field1);
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xddf89345, .id = "Bare_Long", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_long (D->access_hash, field2);
}
void store_ds_constructor_int (int *D, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0xa8509bda && T->type->name != 0x57af6425)) { return; }
  out_int (*D);
}
void store_ds_constructor_int128 (struct tl_ds_int128 *D, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x7d36c439 && T->type->name != 0x82c93bc6)) { return; }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xddf89345, .id = "Bare_Long", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_long (D->f0, field1);
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xddf89345, .id = "Bare_Long", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_long (D->f1, field2);
}
void store_ds_constructor_int256 (struct tl_ds_int256 *D, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0xf2c798b3 && T->type->name != 0x0d38674c)) { return; }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xddf89345, .id = "Bare_Long", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_long (D->f0, field1);
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xddf89345, .id = "Bare_Long", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_long (D->f1, field2);
  struct paramed_type *field3 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xddf89345, .id = "Bare_Long", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_long (D->f2, field3);
  struct paramed_type *field4 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xddf89345, .id = "Bare_Long", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_long (D->f3, field4);
}
void store_ds_constructor_keyboard_button (struct tl_ds_keyboard_button *D, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0xa2fa4880 && T->type->name != 0x5d05b77f)) { return; }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_string (D->text, field1);
}
void store_ds_constructor_keyboard_button_row (struct tl_ds_keyboard_button_row *D, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x77608b83 && T->type->name != 0x889f747c)) { return; }
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
  store_ds_type_vector ((void *)D->buttons, field1);
}
void store_ds_constructor_long (long long *D, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x22076cba && T->type->name != 0xddf89345)) { return; }
  out_long (*D);
}
void store_ds_constructor_message_empty (struct tl_ds_message *D, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x5d652a7f && T->type->name != 0xa29ad580)) { return; }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_int (D->id, field1);
}
void store_ds_constructor_message (struct tl_ds_message *D, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x5d652a7f && T->type->name != 0xa29ad580)) { return; }
  struct paramed_type *var0 = INT2PTR (*D->flags);
  out_int (PTR2INT (var0));
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_int (D->id, field2);
  struct paramed_type *field3 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_int (D->from_id, field3);
  struct paramed_type *field4 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x276159d6, .id = "Peer", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_peer (D->to_id, field4);
  if (PTR2INT (var0) & (1 << 2)) {
    struct paramed_type *field5 = 
    &(struct paramed_type){
      .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
      .params = 0,
    };
    store_ds_type_bare_int (D->fwd_from_id, field5);
  }
  if (PTR2INT (var0) & (1 << 2)) {
    struct paramed_type *field6 = 
    &(struct paramed_type){
      .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
      .params = 0,
    };
    store_ds_type_bare_int (D->fwd_date, field6);
  }
  if (PTR2INT (var0) & (1 << 3)) {
    struct paramed_type *field7 = 
    &(struct paramed_type){
      .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
      .params = 0,
    };
    store_ds_type_bare_int (D->reply_to_msg_id, field7);
  }
  struct paramed_type *field8 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_int (D->date, field8);
  struct paramed_type *field9 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_string (D->message, field9);
  struct paramed_type *field10 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x95f2471a, .id = "MessageMedia", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_message_media (D->media, field10);
  if (PTR2INT (var0) & (1 << 6)) {
    struct paramed_type *field11 = 
    &(struct paramed_type){
      .type = &(struct tl_type_descr) {.name = 0x612ca4a9, .id = "ReplyMarkup", .params_num = 0, .params_types = 0},
      .params = 0,
    };
    store_ds_type_reply_markup (D->reply_markup, field11);
  }
}
void store_ds_constructor_message_service (struct tl_ds_message *D, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x5d652a7f && T->type->name != 0xa29ad580)) { return; }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_int (D->flags, field1);
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_int (D->id, field2);
  struct paramed_type *field3 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_int (D->from_id, field3);
  struct paramed_type *field4 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x276159d6, .id = "Peer", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_peer (D->to_id, field4);
  struct paramed_type *field5 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_int (D->date, field5);
  struct paramed_type *field6 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x383415fa, .id = "MessageAction", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_message_action (D->action, field6);
}
void store_ds_constructor_message_action_empty (struct tl_ds_message_action *D, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x383415fa && T->type->name != 0xc7cbea05)) { return; }
}
void store_ds_constructor_message_action_chat_create (struct tl_ds_message_action *D, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x383415fa && T->type->name != 0xc7cbea05)) { return; }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_string (D->title, field1);
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
  store_ds_type_vector ((void *)D->users, field2);
}
void store_ds_constructor_message_action_chat_edit_title (struct tl_ds_message_action *D, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x383415fa && T->type->name != 0xc7cbea05)) { return; }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_string (D->title, field1);
}
void store_ds_constructor_message_action_chat_edit_photo (struct tl_ds_message_action *D, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x383415fa && T->type->name != 0xc7cbea05)) { return; }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xc207550a, .id = "Photo", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_photo (D->photo, field1);
}
void store_ds_constructor_message_action_chat_delete_photo (struct tl_ds_message_action *D, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x383415fa && T->type->name != 0xc7cbea05)) { return; }
}
void store_ds_constructor_message_action_chat_add_user (struct tl_ds_message_action *D, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x383415fa && T->type->name != 0xc7cbea05)) { return; }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_int (D->user_id, field1);
}
void store_ds_constructor_message_action_chat_delete_user (struct tl_ds_message_action *D, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x383415fa && T->type->name != 0xc7cbea05)) { return; }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_int (D->user_id, field1);
}
void store_ds_constructor_message_action_geo_chat_create (struct tl_ds_message_action *D, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x383415fa && T->type->name != 0xc7cbea05)) { return; }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_string (D->title, field1);
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_string (D->address, field2);
}
void store_ds_constructor_message_action_geo_chat_checkin (struct tl_ds_message_action *D, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x383415fa && T->type->name != 0xc7cbea05)) { return; }
}
void store_ds_constructor_message_action_chat_joined_by_link (struct tl_ds_message_action *D, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x383415fa && T->type->name != 0xc7cbea05)) { return; }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_int (D->inviter_id, field1);
}
void store_ds_constructor_message_media_empty (struct tl_ds_message_media *D, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x95f2471a && T->type->name != 0x6a0db8e5)) { return; }
}
void store_ds_constructor_message_media_photo (struct tl_ds_message_media *D, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x95f2471a && T->type->name != 0x6a0db8e5)) { return; }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xc207550a, .id = "Photo", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_photo (D->photo, field1);
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_string (D->caption, field2);
}
void store_ds_constructor_message_media_video (struct tl_ds_message_media *D, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x95f2471a && T->type->name != 0x6a0db8e5)) { return; }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x1716b174, .id = "Video", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_video (D->video, field1);
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_string (D->caption, field2);
}
void store_ds_constructor_message_media_geo (struct tl_ds_message_media *D, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x95f2471a && T->type->name != 0x6a0db8e5)) { return; }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x315e0a53, .id = "GeoPoint", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_geo_point (D->geo, field1);
}
void store_ds_constructor_message_media_contact (struct tl_ds_message_media *D, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x95f2471a && T->type->name != 0x6a0db8e5)) { return; }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_string (D->phone_number, field1);
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_string (D->first_name, field2);
  struct paramed_type *field3 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_string (D->last_name, field3);
  struct paramed_type *field4 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_int (D->user_id, field4);
}
void store_ds_constructor_message_media_unsupported (struct tl_ds_message_media *D, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x95f2471a && T->type->name != 0x6a0db8e5)) { return; }
}
void store_ds_constructor_message_media_document (struct tl_ds_message_media *D, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x95f2471a && T->type->name != 0x6a0db8e5)) { return; }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x51a73418, .id = "Document", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_document (D->document, field1);
}
void store_ds_constructor_message_media_audio (struct tl_ds_message_media *D, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x95f2471a && T->type->name != 0x6a0db8e5)) { return; }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x9fc5ec4e, .id = "Audio", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_audio (D->audio, field1);
}
void store_ds_constructor_message_media_web_page (struct tl_ds_message_media *D, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x95f2471a && T->type->name != 0x6a0db8e5)) { return; }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x8d8c0d41, .id = "WebPage", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_web_page (D->webpage, field1);
}
void store_ds_constructor_message_media_venue (struct tl_ds_message_media *D, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x95f2471a && T->type->name != 0x6a0db8e5)) { return; }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x315e0a53, .id = "GeoPoint", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_geo_point (D->geo, field1);
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_string (D->title, field2);
  struct paramed_type *field3 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_string (D->address, field3);
  struct paramed_type *field4 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_string (D->provider, field4);
  struct paramed_type *field5 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_string (D->venue_id, field5);
}
void store_ds_constructor_message_media_photo_l27 (struct tl_ds_message_media *D, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x95f2471a && T->type->name != 0x6a0db8e5)) { return; }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xc207550a, .id = "Photo", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_photo (D->photo, field1);
}
void store_ds_constructor_message_media_video_l27 (struct tl_ds_message_media *D, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x95f2471a && T->type->name != 0x6a0db8e5)) { return; }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x1716b174, .id = "Video", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_video (D->video, field1);
}
void store_ds_constructor_input_messages_filter_empty (struct tl_ds_messages_filter *D, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x80895ae0 && T->type->name != 0x7f76a51f)) { return; }
}
void store_ds_constructor_input_messages_filter_photos (struct tl_ds_messages_filter *D, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x80895ae0 && T->type->name != 0x7f76a51f)) { return; }
}
void store_ds_constructor_input_messages_filter_video (struct tl_ds_messages_filter *D, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x80895ae0 && T->type->name != 0x7f76a51f)) { return; }
}
void store_ds_constructor_input_messages_filter_photo_video (struct tl_ds_messages_filter *D, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x80895ae0 && T->type->name != 0x7f76a51f)) { return; }
}
void store_ds_constructor_input_messages_filter_photo_video_documents (struct tl_ds_messages_filter *D, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x80895ae0 && T->type->name != 0x7f76a51f)) { return; }
}
void store_ds_constructor_input_messages_filter_document (struct tl_ds_messages_filter *D, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x80895ae0 && T->type->name != 0x7f76a51f)) { return; }
}
void store_ds_constructor_input_messages_filter_audio (struct tl_ds_messages_filter *D, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x80895ae0 && T->type->name != 0x7f76a51f)) { return; }
}
void store_ds_constructor_nearest_dc (struct tl_ds_nearest_dc *D, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x8e1a1775 && T->type->name != 0x71e5e88a)) { return; }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_string (D->country, field1);
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_int (D->this_dc, field2);
  struct paramed_type *field3 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_int (D->nearest_dc, field3);
}
void store_ds_constructor_notify_peer (struct tl_ds_notify_peer *D, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x9fcb8237 && T->type->name != 0x60347dc8)) { return; }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x276159d6, .id = "Peer", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_peer (D->peer, field1);
}
void store_ds_constructor_notify_users (struct tl_ds_notify_peer *D, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x9fcb8237 && T->type->name != 0x60347dc8)) { return; }
}
void store_ds_constructor_notify_chats (struct tl_ds_notify_peer *D, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x9fcb8237 && T->type->name != 0x60347dc8)) { return; }
}
void store_ds_constructor_notify_all (struct tl_ds_notify_peer *D, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x9fcb8237 && T->type->name != 0x60347dc8)) { return; }
}
void store_ds_constructor_null (struct tl_ds_null *D, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x56730bcc && T->type->name != 0xa98cf433)) { return; }
}
void store_ds_constructor_p_q_inner_data (struct tl_ds_p_q_inner_data *D, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0xbfa3de38 && T->type->name != 0x405c21c7)) { return; }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_string (D->pq, field1);
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_string (D->p, field2);
  struct paramed_type *field3 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_string (D->q, field3);
  struct paramed_type *field4 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x82c93bc6, .id = "Bare_Int128", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_int128 (D->nonce, field4);
  struct paramed_type *field5 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x82c93bc6, .id = "Bare_Int128", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_int128 (D->server_nonce, field5);
  struct paramed_type *field6 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x0d38674c, .id = "Bare_Int256", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_int256 (D->new_nonce, field6);
}
void store_ds_constructor_p_q_inner_data_temp (struct tl_ds_p_q_inner_data *D, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0xbfa3de38 && T->type->name != 0x405c21c7)) { return; }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_string (D->pq, field1);
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_string (D->p, field2);
  struct paramed_type *field3 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_string (D->q, field3);
  struct paramed_type *field4 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x82c93bc6, .id = "Bare_Int128", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_int128 (D->nonce, field4);
  struct paramed_type *field5 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x82c93bc6, .id = "Bare_Int128", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_int128 (D->server_nonce, field5);
  struct paramed_type *field6 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x0d38674c, .id = "Bare_Int256", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_int256 (D->new_nonce, field6);
  struct paramed_type *field7 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_int (D->expires_in, field7);
}
void store_ds_constructor_peer_user (struct tl_ds_peer *D, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x276159d6 && T->type->name != 0xd89ea629)) { return; }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_int (D->user_id, field1);
}
void store_ds_constructor_peer_chat (struct tl_ds_peer *D, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x276159d6 && T->type->name != 0xd89ea629)) { return; }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_int (D->chat_id, field1);
}
void store_ds_constructor_peer_notify_events_empty (struct tl_ds_peer_notify_events *D, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0xc0c8d13b && T->type->name != 0x3f372ec4)) { return; }
}
void store_ds_constructor_peer_notify_events_all (struct tl_ds_peer_notify_events *D, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0xc0c8d13b && T->type->name != 0x3f372ec4)) { return; }
}
void store_ds_constructor_peer_notify_settings_empty (struct tl_ds_peer_notify_settings *D, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0xfdf894fc && T->type->name != 0x02076b03)) { return; }
}
void store_ds_constructor_peer_notify_settings (struct tl_ds_peer_notify_settings *D, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0xfdf894fc && T->type->name != 0x02076b03)) { return; }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_int (D->mute_until, field1);
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_string (D->sound, field2);
  struct paramed_type *field3 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x250be282, .id = "Bool", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bool (D->show_previews, field3);
  struct paramed_type *field4 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_int (D->events_mask, field4);
}
void store_ds_constructor_photo_empty (struct tl_ds_photo *D, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0xc207550a && T->type->name != 0x3df8aaf5)) { return; }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xddf89345, .id = "Bare_Long", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_long (D->id, field1);
}
void store_ds_constructor_photo (struct tl_ds_photo *D, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0xc207550a && T->type->name != 0x3df8aaf5)) { return; }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xddf89345, .id = "Bare_Long", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_long (D->id, field1);
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xddf89345, .id = "Bare_Long", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_long (D->access_hash, field2);
  struct paramed_type *field3 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_int (D->user_id, field3);
  struct paramed_type *field4 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_int (D->date, field4);
  struct paramed_type *field5 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x315e0a53, .id = "GeoPoint", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_geo_point (D->geo, field5);
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
  store_ds_type_vector ((void *)D->sizes, field6);
}
void store_ds_constructor_photo_l27 (struct tl_ds_photo *D, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0xc207550a && T->type->name != 0x3df8aaf5)) { return; }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xddf89345, .id = "Bare_Long", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_long (D->id, field1);
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xddf89345, .id = "Bare_Long", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_long (D->access_hash, field2);
  struct paramed_type *field3 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_int (D->user_id, field3);
  struct paramed_type *field4 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_int (D->date, field4);
  struct paramed_type *field5 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_string (D->caption, field5);
  struct paramed_type *field6 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x315e0a53, .id = "GeoPoint", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_geo_point (D->geo, field6);
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
  store_ds_type_vector ((void *)D->sizes, field7);
}
void store_ds_constructor_photo_size_empty (struct tl_ds_photo_size *D, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x900f60dd && T->type->name != 0x6ff09f22)) { return; }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_string (D->type, field1);
}
void store_ds_constructor_photo_size (struct tl_ds_photo_size *D, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x900f60dd && T->type->name != 0x6ff09f22)) { return; }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_string (D->type, field1);
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x2f8ffb30, .id = "FileLocation", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_file_location (D->location, field2);
  struct paramed_type *field3 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_int (D->w, field3);
  struct paramed_type *field4 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_int (D->h, field4);
  struct paramed_type *field5 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_int (D->size, field5);
}
void store_ds_constructor_photo_cached_size (struct tl_ds_photo_size *D, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x900f60dd && T->type->name != 0x6ff09f22)) { return; }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_string (D->type, field1);
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x2f8ffb30, .id = "FileLocation", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_file_location (D->location, field2);
  struct paramed_type *field3 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_int (D->w, field3);
  struct paramed_type *field4 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_int (D->h, field4);
  struct paramed_type *field5 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xf11ec860, .id = "Bare_Bytes", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_bytes (D->bytes, field5);
}
void store_ds_constructor_privacy_key_status_timestamp (struct tl_ds_privacy_key *D, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0xbc2eab30 && T->type->name != 0x43d154cf)) { return; }
}
void store_ds_constructor_privacy_value_allow_contacts (struct tl_ds_privacy_rule *D, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0xa8638aec && T->type->name != 0x579c7513)) { return; }
}
void store_ds_constructor_privacy_value_allow_all (struct tl_ds_privacy_rule *D, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0xa8638aec && T->type->name != 0x579c7513)) { return; }
}
void store_ds_constructor_privacy_value_allow_users (struct tl_ds_privacy_rule *D, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0xa8638aec && T->type->name != 0x579c7513)) { return; }
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
  store_ds_type_vector ((void *)D->users, field1);
}
void store_ds_constructor_privacy_value_disallow_contacts (struct tl_ds_privacy_rule *D, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0xa8638aec && T->type->name != 0x579c7513)) { return; }
}
void store_ds_constructor_privacy_value_disallow_all (struct tl_ds_privacy_rule *D, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0xa8638aec && T->type->name != 0x579c7513)) { return; }
}
void store_ds_constructor_privacy_value_disallow_users (struct tl_ds_privacy_rule *D, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0xa8638aec && T->type->name != 0x579c7513)) { return; }
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
  store_ds_type_vector ((void *)D->users, field1);
}
void store_ds_constructor_received_notify_message (struct tl_ds_received_notify_message *D, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0xa384b779 && T->type->name != 0x5c7b4886)) { return; }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_int (D->id, field1);
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_int (D->flags, field2);
}
void store_ds_constructor_reply_keyboard_hide (struct tl_ds_reply_markup *D, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x612ca4a9 && T->type->name != 0x9ed35b56)) { return; }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_int (D->flags, field1);
}
void store_ds_constructor_reply_keyboard_force_reply (struct tl_ds_reply_markup *D, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x612ca4a9 && T->type->name != 0x9ed35b56)) { return; }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_int (D->flags, field1);
}
void store_ds_constructor_reply_keyboard_markup (struct tl_ds_reply_markup *D, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x612ca4a9 && T->type->name != 0x9ed35b56)) { return; }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_int (D->flags, field1);
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
  store_ds_type_vector ((void *)D->rows, field2);
}
void store_ds_constructor_res_p_q (struct tl_ds_res_p_q *D, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x05162463 && T->type->name != 0xfae9db9c)) { return; }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x82c93bc6, .id = "Bare_Int128", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_int128 (D->nonce, field1);
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x82c93bc6, .id = "Bare_Int128", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_int128 (D->server_nonce, field2);
  struct paramed_type *field3 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_string (D->pq, field3);
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
  store_ds_type_vector ((void *)D->server_public_key_fingerprints, field4);
}
void store_ds_constructor_send_message_typing_action (struct tl_ds_send_message_action *D, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x70541c7e && T->type->name != 0x8fabe381)) { return; }
}
void store_ds_constructor_send_message_cancel_action (struct tl_ds_send_message_action *D, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x70541c7e && T->type->name != 0x8fabe381)) { return; }
}
void store_ds_constructor_send_message_record_video_action (struct tl_ds_send_message_action *D, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x70541c7e && T->type->name != 0x8fabe381)) { return; }
}
void store_ds_constructor_send_message_upload_video_action_l27 (struct tl_ds_send_message_action *D, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x70541c7e && T->type->name != 0x8fabe381)) { return; }
}
void store_ds_constructor_send_message_upload_video_action (struct tl_ds_send_message_action *D, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x70541c7e && T->type->name != 0x8fabe381)) { return; }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_int (D->progress, field1);
}
void store_ds_constructor_send_message_record_audio_action (struct tl_ds_send_message_action *D, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x70541c7e && T->type->name != 0x8fabe381)) { return; }
}
void store_ds_constructor_send_message_upload_audio_action_l27 (struct tl_ds_send_message_action *D, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x70541c7e && T->type->name != 0x8fabe381)) { return; }
}
void store_ds_constructor_send_message_upload_audio_action (struct tl_ds_send_message_action *D, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x70541c7e && T->type->name != 0x8fabe381)) { return; }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_int (D->progress, field1);
}
void store_ds_constructor_send_message_upload_photo_action (struct tl_ds_send_message_action *D, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x70541c7e && T->type->name != 0x8fabe381)) { return; }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_int (D->progress, field1);
}
void store_ds_constructor_send_message_upload_document_action_l27 (struct tl_ds_send_message_action *D, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x70541c7e && T->type->name != 0x8fabe381)) { return; }
}
void store_ds_constructor_send_message_upload_document_action (struct tl_ds_send_message_action *D, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x70541c7e && T->type->name != 0x8fabe381)) { return; }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_int (D->progress, field1);
}
void store_ds_constructor_send_message_geo_location_action (struct tl_ds_send_message_action *D, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x70541c7e && T->type->name != 0x8fabe381)) { return; }
}
void store_ds_constructor_send_message_choose_contact_action (struct tl_ds_send_message_action *D, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x70541c7e && T->type->name != 0x8fabe381)) { return; }
}
void store_ds_constructor_server_d_h_params_fail (struct tl_ds_server_d_h_params *D, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0xa9230301 && T->type->name != 0x56dcfcfe)) { return; }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x82c93bc6, .id = "Bare_Int128", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_int128 (D->nonce, field1);
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x82c93bc6, .id = "Bare_Int128", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_int128 (D->server_nonce, field2);
  struct paramed_type *field3 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x82c93bc6, .id = "Bare_Int128", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_int128 (D->new_nonce_hash, field3);
}
void store_ds_constructor_server_d_h_params_ok (struct tl_ds_server_d_h_params *D, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0xa9230301 && T->type->name != 0x56dcfcfe)) { return; }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x82c93bc6, .id = "Bare_Int128", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_int128 (D->nonce, field1);
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x82c93bc6, .id = "Bare_Int128", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_int128 (D->server_nonce, field2);
  struct paramed_type *field3 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_string (D->encrypted_answer, field3);
}
void store_ds_constructor_server_d_h_inner_data (struct tl_ds_server_d_h_inner_data *D, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0xb5890dba && T->type->name != 0x4a76f245)) { return; }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x82c93bc6, .id = "Bare_Int128", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_int128 (D->nonce, field1);
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x82c93bc6, .id = "Bare_Int128", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_int128 (D->server_nonce, field2);
  struct paramed_type *field3 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_int (D->g, field3);
  struct paramed_type *field4 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_string (D->dh_prime, field4);
  struct paramed_type *field5 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_string (D->g_a, field5);
  struct paramed_type *field6 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_int (D->server_time, field6);
}
void store_ds_constructor_dh_gen_ok (struct tl_ds_set_client_d_h_params_answer *D, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0xdb8a468f && T->type->name != 0x2475b970)) { return; }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x82c93bc6, .id = "Bare_Int128", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_int128 (D->nonce, field1);
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x82c93bc6, .id = "Bare_Int128", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_int128 (D->server_nonce, field2);
  struct paramed_type *field3 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x82c93bc6, .id = "Bare_Int128", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_int128 (D->new_nonce_hash1, field3);
}
void store_ds_constructor_dh_gen_retry (struct tl_ds_set_client_d_h_params_answer *D, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0xdb8a468f && T->type->name != 0x2475b970)) { return; }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x82c93bc6, .id = "Bare_Int128", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_int128 (D->nonce, field1);
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x82c93bc6, .id = "Bare_Int128", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_int128 (D->server_nonce, field2);
  struct paramed_type *field3 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x82c93bc6, .id = "Bare_Int128", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_int128 (D->new_nonce_hash2, field3);
}
void store_ds_constructor_dh_gen_fail (struct tl_ds_set_client_d_h_params_answer *D, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0xdb8a468f && T->type->name != 0x2475b970)) { return; }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x82c93bc6, .id = "Bare_Int128", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_int128 (D->nonce, field1);
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x82c93bc6, .id = "Bare_Int128", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_int128 (D->server_nonce, field2);
  struct paramed_type *field3 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x82c93bc6, .id = "Bare_Int128", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_int128 (D->new_nonce_hash3, field3);
}
void store_ds_constructor_sticker_pack (struct tl_ds_sticker_pack *D, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x12b299d4 && T->type->name != 0xed4d662b)) { return; }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_string (D->emoticon, field1);
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
  store_ds_type_vector ((void *)D->documents, field2);
}
void store_ds_constructor_sticker_set (struct tl_ds_sticker_set *D, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0xa7a43b17 && T->type->name != 0x585bc4e8)) { return; }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xddf89345, .id = "Bare_Long", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_long (D->id, field1);
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xddf89345, .id = "Bare_Long", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_long (D->access_hash, field2);
  struct paramed_type *field3 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_string (D->title, field3);
  struct paramed_type *field4 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_string (D->short_name, field4);
}
void store_ds_constructor_string (struct tl_ds_string *D, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0xb5286e24 && T->type->name != 0x4ad791db)) { return; }
  out_cstring (D->data, D->len);
}
void store_ds_constructor_update_new_message (struct tl_ds_update *D, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x2e02b27e && T->type->name != 0xd1fd4d81)) { return; }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x5d652a7f, .id = "Message", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_message (D->message, field1);
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_int (D->pts, field2);
  struct paramed_type *field3 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_int (D->pts_count, field3);
}
void store_ds_constructor_update_message_i_d (struct tl_ds_update *D, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x2e02b27e && T->type->name != 0xd1fd4d81)) { return; }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_int (D->id, field1);
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xddf89345, .id = "Bare_Long", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_long (D->random_id, field2);
}
void store_ds_constructor_update_delete_messages (struct tl_ds_update *D, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x2e02b27e && T->type->name != 0xd1fd4d81)) { return; }
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
  store_ds_type_vector ((void *)D->messages, field1);
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_int (D->pts, field2);
  struct paramed_type *field3 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_int (D->pts_count, field3);
}
void store_ds_constructor_update_user_typing (struct tl_ds_update *D, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x2e02b27e && T->type->name != 0xd1fd4d81)) { return; }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_int (D->user_id, field1);
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x70541c7e, .id = "SendMessageAction", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_send_message_action (D->action, field2);
}
void store_ds_constructor_update_chat_user_typing (struct tl_ds_update *D, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x2e02b27e && T->type->name != 0xd1fd4d81)) { return; }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_int (D->chat_id, field1);
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_int (D->user_id, field2);
  struct paramed_type *field3 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x70541c7e, .id = "SendMessageAction", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_send_message_action (D->action, field3);
}
void store_ds_constructor_update_chat_participants (struct tl_ds_update *D, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x2e02b27e && T->type->name != 0xd1fd4d81)) { return; }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x77930f9f, .id = "ChatParticipants", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_chat_participants (D->participants, field1);
}
void store_ds_constructor_update_user_status (struct tl_ds_update *D, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x2e02b27e && T->type->name != 0xd1fd4d81)) { return; }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_int (D->user_id, field1);
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x76de9570, .id = "UserStatus", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_user_status (D->status, field2);
}
void store_ds_constructor_update_user_name (struct tl_ds_update *D, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x2e02b27e && T->type->name != 0xd1fd4d81)) { return; }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_int (D->user_id, field1);
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_string (D->first_name, field2);
  struct paramed_type *field3 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_string (D->last_name, field3);
  struct paramed_type *field4 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_string (D->username, field4);
}
void store_ds_constructor_update_user_photo (struct tl_ds_update *D, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x2e02b27e && T->type->name != 0xd1fd4d81)) { return; }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_int (D->user_id, field1);
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_int (D->date, field2);
  struct paramed_type *field3 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x9a486229, .id = "UserProfilePhoto", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_user_profile_photo (D->photo, field3);
  struct paramed_type *field4 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x250be282, .id = "Bool", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bool (D->previous, field4);
}
void store_ds_constructor_update_contact_registered (struct tl_ds_update *D, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x2e02b27e && T->type->name != 0xd1fd4d81)) { return; }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_int (D->user_id, field1);
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_int (D->date, field2);
}
void store_ds_constructor_update_contact_link (struct tl_ds_update *D, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x2e02b27e && T->type->name != 0xd1fd4d81)) { return; }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_int (D->user_id, field1);
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x522fbc63, .id = "ContactLink", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_contact_link (D->my_link, field2);
  struct paramed_type *field3 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x522fbc63, .id = "ContactLink", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_contact_link (D->foreign_link, field3);
}
void store_ds_constructor_update_new_authorization (struct tl_ds_update *D, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x2e02b27e && T->type->name != 0xd1fd4d81)) { return; }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xddf89345, .id = "Bare_Long", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_long (D->auth_key_id, field1);
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_int (D->date, field2);
  struct paramed_type *field3 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_string (D->device, field3);
  struct paramed_type *field4 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_string (D->location, field4);
}
void store_ds_constructor_update_new_geo_chat_message (struct tl_ds_update *D, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x2e02b27e && T->type->name != 0xd1fd4d81)) { return; }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xf67b4034, .id = "GeoChatMessage", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_geo_chat_message (D->geo_message, field1);
}
void store_ds_constructor_update_new_encrypted_message (struct tl_ds_update *D, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x2e02b27e && T->type->name != 0xd1fd4d81)) { return; }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xce6b8a1e, .id = "EncryptedMessage", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_encrypted_message (D->encr_message, field1);
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_int (D->qts, field2);
}
void store_ds_constructor_update_encrypted_chat_typing (struct tl_ds_update *D, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x2e02b27e && T->type->name != 0xd1fd4d81)) { return; }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_int (D->chat_id, field1);
}
void store_ds_constructor_update_encryption (struct tl_ds_update *D, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x2e02b27e && T->type->name != 0xd1fd4d81)) { return; }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xb1718213, .id = "EncryptedChat", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_encrypted_chat (D->encr_chat, field1);
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_int (D->date, field2);
}
void store_ds_constructor_update_encrypted_messages_read (struct tl_ds_update *D, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x2e02b27e && T->type->name != 0xd1fd4d81)) { return; }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_int (D->chat_id, field1);
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_int (D->max_date, field2);
  struct paramed_type *field3 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_int (D->date, field3);
}
void store_ds_constructor_update_chat_participant_add (struct tl_ds_update *D, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x2e02b27e && T->type->name != 0xd1fd4d81)) { return; }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_int (D->chat_id, field1);
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_int (D->user_id, field2);
  struct paramed_type *field3 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_int (D->inviter_id, field3);
  struct paramed_type *field4 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_int (D->version, field4);
}
void store_ds_constructor_update_chat_participant_delete (struct tl_ds_update *D, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x2e02b27e && T->type->name != 0xd1fd4d81)) { return; }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_int (D->chat_id, field1);
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_int (D->user_id, field2);
  struct paramed_type *field3 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_int (D->version, field3);
}
void store_ds_constructor_update_dc_options (struct tl_ds_update *D, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x2e02b27e && T->type->name != 0xd1fd4d81)) { return; }
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
  store_ds_type_vector ((void *)D->dc_options, field1);
}
void store_ds_constructor_update_user_blocked (struct tl_ds_update *D, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x2e02b27e && T->type->name != 0xd1fd4d81)) { return; }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_int (D->user_id, field1);
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x250be282, .id = "Bool", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bool (D->blocked, field2);
}
void store_ds_constructor_update_notify_settings (struct tl_ds_update *D, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x2e02b27e && T->type->name != 0xd1fd4d81)) { return; }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x9fcb8237, .id = "NotifyPeer", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_notify_peer (D->notify_peer, field1);
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xfdf894fc, .id = "PeerNotifySettings", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_peer_notify_settings (D->notify_settings, field2);
}
void store_ds_constructor_update_service_notification (struct tl_ds_update *D, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x2e02b27e && T->type->name != 0xd1fd4d81)) { return; }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_string (D->type, field1);
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_string (D->message_text, field2);
  struct paramed_type *field3 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x95f2471a, .id = "MessageMedia", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_message_media (D->media, field3);
  struct paramed_type *field4 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x250be282, .id = "Bool", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bool (D->popup, field4);
}
void store_ds_constructor_update_privacy (struct tl_ds_update *D, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x2e02b27e && T->type->name != 0xd1fd4d81)) { return; }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xbc2eab30, .id = "PrivacyKey", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_privacy_key (D->key, field1);
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
  store_ds_type_vector ((void *)D->rules, field2);
}
void store_ds_constructor_update_user_phone (struct tl_ds_update *D, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x2e02b27e && T->type->name != 0xd1fd4d81)) { return; }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_int (D->user_id, field1);
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_string (D->phone, field2);
}
void store_ds_constructor_update_read_history_inbox (struct tl_ds_update *D, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x2e02b27e && T->type->name != 0xd1fd4d81)) { return; }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x276159d6, .id = "Peer", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_peer (D->peer, field1);
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_int (D->max_id, field2);
  struct paramed_type *field3 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_int (D->pts, field3);
  struct paramed_type *field4 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_int (D->pts_count, field4);
}
void store_ds_constructor_update_read_history_outbox (struct tl_ds_update *D, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x2e02b27e && T->type->name != 0xd1fd4d81)) { return; }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x276159d6, .id = "Peer", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_peer (D->peer, field1);
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_int (D->max_id, field2);
  struct paramed_type *field3 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_int (D->pts, field3);
  struct paramed_type *field4 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_int (D->pts_count, field4);
}
void store_ds_constructor_update_web_page (struct tl_ds_update *D, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x2e02b27e && T->type->name != 0xd1fd4d81)) { return; }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x8d8c0d41, .id = "WebPage", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_web_page (D->webpage, field1);
}
void store_ds_constructor_update_read_messages_contents (struct tl_ds_update *D, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x2e02b27e && T->type->name != 0xd1fd4d81)) { return; }
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
  store_ds_type_vector ((void *)D->messages, field1);
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_int (D->pts, field2);
  struct paramed_type *field3 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_int (D->pts_count, field3);
}
void store_ds_constructor_update_msg_update (struct tl_ds_update *D, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x2e02b27e && T->type->name != 0xd1fd4d81)) { return; }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_int (D->id, field1);
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_int (D->pts, field2);
  struct paramed_type *field3 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_int (D->pts_count, field3);
}
void store_ds_constructor_updates_too_long (struct tl_ds_updates *D, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x22499d27 && T->type->name != 0xddb662d8)) { return; }
}
void store_ds_constructor_update_short_message (struct tl_ds_updates *D, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x22499d27 && T->type->name != 0xddb662d8)) { return; }
  struct paramed_type *var0 = INT2PTR (*D->flags);
  out_int (PTR2INT (var0));
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_int (D->id, field2);
  struct paramed_type *field3 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_int (D->user_id, field3);
  struct paramed_type *field4 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_string (D->message, field4);
  struct paramed_type *field5 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_int (D->pts, field5);
  struct paramed_type *field6 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_int (D->pts_count, field6);
  struct paramed_type *field7 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_int (D->date, field7);
  if (PTR2INT (var0) & (1 << 2)) {
    struct paramed_type *field8 = 
    &(struct paramed_type){
      .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
      .params = 0,
    };
    store_ds_type_bare_int (D->fwd_from_id, field8);
  }
  if (PTR2INT (var0) & (1 << 2)) {
    struct paramed_type *field9 = 
    &(struct paramed_type){
      .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
      .params = 0,
    };
    store_ds_type_bare_int (D->fwd_date, field9);
  }
  if (PTR2INT (var0) & (1 << 3)) {
    struct paramed_type *field10 = 
    &(struct paramed_type){
      .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
      .params = 0,
    };
    store_ds_type_bare_int (D->reply_to_msg_id, field10);
  }
}
void store_ds_constructor_update_short_chat_message (struct tl_ds_updates *D, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x22499d27 && T->type->name != 0xddb662d8)) { return; }
  struct paramed_type *var0 = INT2PTR (*D->flags);
  out_int (PTR2INT (var0));
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_int (D->id, field2);
  struct paramed_type *field3 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_int (D->from_id, field3);
  struct paramed_type *field4 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_int (D->chat_id, field4);
  struct paramed_type *field5 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_string (D->message, field5);
  struct paramed_type *field6 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_int (D->pts, field6);
  struct paramed_type *field7 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_int (D->pts_count, field7);
  struct paramed_type *field8 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_int (D->date, field8);
  if (PTR2INT (var0) & (1 << 2)) {
    struct paramed_type *field9 = 
    &(struct paramed_type){
      .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
      .params = 0,
    };
    store_ds_type_bare_int (D->fwd_from_id, field9);
  }
  if (PTR2INT (var0) & (1 << 2)) {
    struct paramed_type *field10 = 
    &(struct paramed_type){
      .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
      .params = 0,
    };
    store_ds_type_bare_int (D->fwd_date, field10);
  }
  if (PTR2INT (var0) & (1 << 3)) {
    struct paramed_type *field11 = 
    &(struct paramed_type){
      .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
      .params = 0,
    };
    store_ds_type_bare_int (D->reply_to_msg_id, field11);
  }
}
void store_ds_constructor_update_short (struct tl_ds_updates *D, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x22499d27 && T->type->name != 0xddb662d8)) { return; }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x2e02b27e, .id = "Update", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_update (D->update, field1);
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_int (D->date, field2);
}
void store_ds_constructor_updates_combined (struct tl_ds_updates *D, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x22499d27 && T->type->name != 0xddb662d8)) { return; }
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
  store_ds_type_vector ((void *)D->updates, field1);
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
  store_ds_type_vector ((void *)D->users, field2);
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
  store_ds_type_vector ((void *)D->chats, field3);
  struct paramed_type *field4 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_int (D->date, field4);
  struct paramed_type *field5 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_int (D->seq_start, field5);
  struct paramed_type *field6 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_int (D->seq, field6);
}
void store_ds_constructor_updates (struct tl_ds_updates *D, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x22499d27 && T->type->name != 0xddb662d8)) { return; }
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
  store_ds_type_vector ((void *)D->updates, field1);
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
  store_ds_type_vector ((void *)D->users, field2);
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
  store_ds_type_vector ((void *)D->chats, field3);
  struct paramed_type *field4 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_int (D->date, field4);
  struct paramed_type *field5 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_int (D->seq, field5);
}
void store_ds_constructor_user_empty (struct tl_ds_user *D, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x02e6c0c8 && T->type->name != 0xfd193f37)) { return; }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_int (D->id, field1);
}
void store_ds_constructor_user (struct tl_ds_user *D, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x02e6c0c8 && T->type->name != 0xfd193f37)) { return; }
  struct paramed_type *var0 = INT2PTR (*D->flags);
  out_int (PTR2INT (var0));
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_int (D->id, field2);
  if (PTR2INT (var0) & (1 << 0)) {
    struct paramed_type *field3 = 
    &(struct paramed_type){
      .type = &(struct tl_type_descr) {.name = 0xddf89345, .id = "Bare_Long", .params_num = 0, .params_types = 0},
      .params = 0,
    };
    store_ds_type_bare_long (D->access_hash, field3);
  }
  if (PTR2INT (var0) & (1 << 1)) {
    struct paramed_type *field4 = 
    &(struct paramed_type){
      .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
      .params = 0,
    };
    store_ds_type_bare_string (D->first_name, field4);
  }
  if (PTR2INT (var0) & (1 << 2)) {
    struct paramed_type *field5 = 
    &(struct paramed_type){
      .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
      .params = 0,
    };
    store_ds_type_bare_string (D->last_name, field5);
  }
  if (PTR2INT (var0) & (1 << 3)) {
    struct paramed_type *field6 = 
    &(struct paramed_type){
      .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
      .params = 0,
    };
    store_ds_type_bare_string (D->username, field6);
  }
  if (PTR2INT (var0) & (1 << 4)) {
    struct paramed_type *field7 = 
    &(struct paramed_type){
      .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
      .params = 0,
    };
    store_ds_type_bare_string (D->phone, field7);
  }
  if (PTR2INT (var0) & (1 << 5)) {
    struct paramed_type *field8 = 
    &(struct paramed_type){
      .type = &(struct tl_type_descr) {.name = 0x9a486229, .id = "UserProfilePhoto", .params_num = 0, .params_types = 0},
      .params = 0,
    };
    store_ds_type_user_profile_photo (D->photo, field8);
  }
  if (PTR2INT (var0) & (1 << 6)) {
    struct paramed_type *field9 = 
    &(struct paramed_type){
      .type = &(struct tl_type_descr) {.name = 0x76de9570, .id = "UserStatus", .params_num = 0, .params_types = 0},
      .params = 0,
    };
    store_ds_type_user_status (D->status, field9);
  }
  if (PTR2INT (var0) & (1 << 14)) {
    struct paramed_type *field10 = 
    &(struct paramed_type){
      .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
      .params = 0,
    };
    store_ds_type_bare_int (D->bot_info_version, field10);
  }
}
void store_ds_constructor_user_full (struct tl_ds_user_full *D, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x5a89ac5b && T->type->name != 0xa57653a4)) { return; }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x02e6c0c8, .id = "User", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_user (D->user, field1);
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x3ace484c, .id = "contacts.Link", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_contacts_link (D->link, field2);
  struct paramed_type *field3 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xc207550a, .id = "Photo", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_photo (D->profile_photo, field3);
  struct paramed_type *field4 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xfdf894fc, .id = "PeerNotifySettings", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_peer_notify_settings (D->notify_settings, field4);
  struct paramed_type *field5 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x250be282, .id = "Bool", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bool (D->blocked, field5);
  struct paramed_type *field6 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xb2e16f93, .id = "BotInfo", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bot_info (D->bot_info, field6);
}
void store_ds_constructor_user_profile_photo_empty (struct tl_ds_user_profile_photo *D, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x9a486229 && T->type->name != 0x65b79dd6)) { return; }
}
void store_ds_constructor_user_profile_photo (struct tl_ds_user_profile_photo *D, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x9a486229 && T->type->name != 0x65b79dd6)) { return; }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xddf89345, .id = "Bare_Long", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_long (D->photo_id, field1);
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x2f8ffb30, .id = "FileLocation", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_file_location (D->photo_small, field2);
  struct paramed_type *field3 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x2f8ffb30, .id = "FileLocation", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_file_location (D->photo_big, field3);
}
void store_ds_constructor_user_status_empty (struct tl_ds_user_status *D, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x76de9570 && T->type->name != 0x89216a8f)) { return; }
}
void store_ds_constructor_user_status_online (struct tl_ds_user_status *D, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x76de9570 && T->type->name != 0x89216a8f)) { return; }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_int (D->expires, field1);
}
void store_ds_constructor_user_status_offline (struct tl_ds_user_status *D, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x76de9570 && T->type->name != 0x89216a8f)) { return; }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_int (D->was_online, field1);
}
void store_ds_constructor_user_status_recently (struct tl_ds_user_status *D, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x76de9570 && T->type->name != 0x89216a8f)) { return; }
}
void store_ds_constructor_user_status_last_week (struct tl_ds_user_status *D, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x76de9570 && T->type->name != 0x89216a8f)) { return; }
}
void store_ds_constructor_user_status_last_month (struct tl_ds_user_status *D, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x76de9570 && T->type->name != 0x89216a8f)) { return; }
}
void store_ds_constructor_vector (struct tl_ds_vector *D, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x1cb5c415 && T->type->name != 0xe34a3bea)) { return; }
  if (ODDP(T->params[0])) { return; }
  struct paramed_type *var0 = T->params[0]; assert (var0);
  struct paramed_type *var1 = INT2PTR (*D->f1);
  out_int (PTR2INT (var1));
  int multiplicity3 = PTR2INT (
  ((void *)var1) + 0  );
  struct paramed_type *field3 = 
  var0;
  {
    int i = 0;
    while (i < multiplicity3) {
      store_ds_type_any (D->f2[i ++], field3);
    }
  }
}
void store_ds_constructor_video_empty (struct tl_ds_video *D, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x1716b174 && T->type->name != 0xe8e94e8b)) { return; }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xddf89345, .id = "Bare_Long", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_long (D->id, field1);
}
void store_ds_constructor_video (struct tl_ds_video *D, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x1716b174 && T->type->name != 0xe8e94e8b)) { return; }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xddf89345, .id = "Bare_Long", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_long (D->id, field1);
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xddf89345, .id = "Bare_Long", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_long (D->access_hash, field2);
  struct paramed_type *field3 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_int (D->user_id, field3);
  struct paramed_type *field4 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_int (D->date, field4);
  struct paramed_type *field5 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_int (D->duration, field5);
  struct paramed_type *field6 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_int (D->size, field6);
  struct paramed_type *field7 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x900f60dd, .id = "PhotoSize", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_photo_size (D->thumb, field7);
  struct paramed_type *field8 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_int (D->dc_id, field8);
  struct paramed_type *field9 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_int (D->w, field9);
  struct paramed_type *field10 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_int (D->h, field10);
}
void store_ds_constructor_video_l27 (struct tl_ds_video *D, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x1716b174 && T->type->name != 0xe8e94e8b)) { return; }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xddf89345, .id = "Bare_Long", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_long (D->id, field1);
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xddf89345, .id = "Bare_Long", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_long (D->access_hash, field2);
  struct paramed_type *field3 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_int (D->user_id, field3);
  struct paramed_type *field4 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_int (D->date, field4);
  struct paramed_type *field5 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_string (D->caption, field5);
  struct paramed_type *field6 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_int (D->duration, field6);
  struct paramed_type *field7 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_string (D->mime_type, field7);
  struct paramed_type *field8 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_int (D->size, field8);
  struct paramed_type *field9 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x900f60dd, .id = "PhotoSize", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_photo_size (D->thumb, field9);
  struct paramed_type *field10 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_int (D->dc_id, field10);
  struct paramed_type *field11 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_int (D->w, field11);
  struct paramed_type *field12 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_int (D->h, field12);
}
void store_ds_constructor_wall_paper (struct tl_ds_wall_paper *D, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0xafa14973 && T->type->name != 0x505eb68c)) { return; }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_int (D->id, field1);
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_string (D->title, field2);
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
  store_ds_type_vector ((void *)D->sizes, field3);
  struct paramed_type *field4 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_int (D->color, field4);
}
void store_ds_constructor_wall_paper_solid (struct tl_ds_wall_paper *D, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0xafa14973 && T->type->name != 0x505eb68c)) { return; }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_int (D->id, field1);
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_string (D->title, field2);
  struct paramed_type *field3 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_int (D->bg_color, field3);
  struct paramed_type *field4 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_int (D->color, field4);
}
void store_ds_constructor_web_page_empty (struct tl_ds_web_page *D, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x8d8c0d41 && T->type->name != 0x7273f2be)) { return; }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xddf89345, .id = "Bare_Long", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_long (D->id, field1);
}
void store_ds_constructor_web_page_pending (struct tl_ds_web_page *D, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x8d8c0d41 && T->type->name != 0x7273f2be)) { return; }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xddf89345, .id = "Bare_Long", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_long (D->id, field1);
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_int (D->date, field2);
}
void store_ds_constructor_web_page (struct tl_ds_web_page *D, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x8d8c0d41 && T->type->name != 0x7273f2be)) { return; }
  struct paramed_type *var0 = INT2PTR (*D->flags);
  out_int (PTR2INT (var0));
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xddf89345, .id = "Bare_Long", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_long (D->id, field2);
  struct paramed_type *field3 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_string (D->url, field3);
  struct paramed_type *field4 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_string (D->display_url, field4);
  if (PTR2INT (var0) & (1 << 0)) {
    struct paramed_type *field5 = 
    &(struct paramed_type){
      .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
      .params = 0,
    };
    store_ds_type_bare_string (D->type, field5);
  }
  if (PTR2INT (var0) & (1 << 1)) {
    struct paramed_type *field6 = 
    &(struct paramed_type){
      .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
      .params = 0,
    };
    store_ds_type_bare_string (D->site_name, field6);
  }
  if (PTR2INT (var0) & (1 << 2)) {
    struct paramed_type *field7 = 
    &(struct paramed_type){
      .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
      .params = 0,
    };
    store_ds_type_bare_string (D->title, field7);
  }
  if (PTR2INT (var0) & (1 << 3)) {
    struct paramed_type *field8 = 
    &(struct paramed_type){
      .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
      .params = 0,
    };
    store_ds_type_bare_string (D->description, field8);
  }
  if (PTR2INT (var0) & (1 << 4)) {
    struct paramed_type *field9 = 
    &(struct paramed_type){
      .type = &(struct tl_type_descr) {.name = 0xc207550a, .id = "Photo", .params_num = 0, .params_types = 0},
      .params = 0,
    };
    store_ds_type_photo (D->photo, field9);
  }
  if (PTR2INT (var0) & (1 << 5)) {
    struct paramed_type *field10 = 
    &(struct paramed_type){
      .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
      .params = 0,
    };
    store_ds_type_bare_string (D->embed_url, field10);
  }
  if (PTR2INT (var0) & (1 << 5)) {
    struct paramed_type *field11 = 
    &(struct paramed_type){
      .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
      .params = 0,
    };
    store_ds_type_bare_string (D->embed_type, field11);
  }
  if (PTR2INT (var0) & (1 << 6)) {
    struct paramed_type *field12 = 
    &(struct paramed_type){
      .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
      .params = 0,
    };
    store_ds_type_bare_int (D->embed_width, field12);
  }
  if (PTR2INT (var0) & (1 << 6)) {
    struct paramed_type *field13 = 
    &(struct paramed_type){
      .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
      .params = 0,
    };
    store_ds_type_bare_int (D->embed_height, field13);
  }
  if (PTR2INT (var0) & (1 << 7)) {
    struct paramed_type *field14 = 
    &(struct paramed_type){
      .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
      .params = 0,
    };
    store_ds_type_bare_int (D->duration, field14);
  }
  if (PTR2INT (var0) & (1 << 8)) {
    struct paramed_type *field15 = 
    &(struct paramed_type){
      .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
      .params = 0,
    };
    store_ds_type_bare_string (D->author, field15);
  }
}
void store_ds_constructor_account_authorizations (struct tl_ds_account_authorizations *D, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x1250abde && T->type->name != 0xedaf5421)) { return; }
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
  store_ds_type_vector ((void *)D->authorizations, field1);
}
void store_ds_constructor_account_no_password (struct tl_ds_account_password *D, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0xeac2a804 && T->type->name != 0x153d57fb)) { return; }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xf11ec860, .id = "Bare_Bytes", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_bytes (D->new_salt, field1);
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_string (D->email_unconfirmed_pattern, field2);
}
void store_ds_constructor_account_password (struct tl_ds_account_password *D, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0xeac2a804 && T->type->name != 0x153d57fb)) { return; }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xf11ec860, .id = "Bare_Bytes", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_bytes (D->current_salt, field1);
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xf11ec860, .id = "Bare_Bytes", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_bytes (D->new_salt, field2);
  struct paramed_type *field3 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_string (D->hint, field3);
  struct paramed_type *field4 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x250be282, .id = "Bool", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bool (D->has_recovery, field4);
  struct paramed_type *field5 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_string (D->email_unconfirmed_pattern, field5);
}
void store_ds_constructor_account_password_input_settings (struct tl_ds_account_password_input_settings *D, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0xbcfc532c && T->type->name != 0x4303acd3)) { return; }
  struct paramed_type *var0 = INT2PTR (*D->flags);
  out_int (PTR2INT (var0));
  if (PTR2INT (var0) & (1 << 0)) {
    struct paramed_type *field2 = 
    &(struct paramed_type){
      .type = &(struct tl_type_descr) {.name = 0xf11ec860, .id = "Bare_Bytes", .params_num = 0, .params_types = 0},
      .params = 0,
    };
    store_ds_type_bare_bytes (D->new_salt, field2);
  }
  if (PTR2INT (var0) & (1 << 0)) {
    struct paramed_type *field3 = 
    &(struct paramed_type){
      .type = &(struct tl_type_descr) {.name = 0xf11ec860, .id = "Bare_Bytes", .params_num = 0, .params_types = 0},
      .params = 0,
    };
    store_ds_type_bare_bytes (D->new_password_hash, field3);
  }
  if (PTR2INT (var0) & (1 << 0)) {
    struct paramed_type *field4 = 
    &(struct paramed_type){
      .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
      .params = 0,
    };
    store_ds_type_bare_string (D->hint, field4);
  }
  if (PTR2INT (var0) & (1 << 1)) {
    struct paramed_type *field5 = 
    &(struct paramed_type){
      .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
      .params = 0,
    };
    store_ds_type_bare_string (D->email, field5);
  }
}
void store_ds_constructor_account_password_settings (struct tl_ds_account_password_settings *D, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0xb7b72ab3 && T->type->name != 0x4848d54c)) { return; }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_string (D->email, field1);
}
void store_ds_constructor_account_privacy_rules (struct tl_ds_account_privacy_rules *D, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x554abb6f && T->type->name != 0xaab54490)) { return; }
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
  store_ds_type_vector ((void *)D->rules, field1);
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
  store_ds_type_vector ((void *)D->users, field2);
}
void store_ds_constructor_account_sent_change_phone_code (struct tl_ds_account_sent_change_phone_code *D, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0xa4f58c4c && T->type->name != 0x5b0a73b3)) { return; }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_string (D->phone_code_hash, field1);
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_int (D->send_call_timeout, field2);
}
void store_ds_constructor_auth_authorization (struct tl_ds_auth_authorization *D, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0xff036af1 && T->type->name != 0x00fc950e)) { return; }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x02e6c0c8, .id = "User", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_user (D->user, field1);
}
void store_ds_constructor_auth_checked_phone (struct tl_ds_auth_checked_phone *D, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x811ea28e && T->type->name != 0x7ee15d71)) { return; }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x250be282, .id = "Bool", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bool (D->phone_registered, field1);
}
void store_ds_constructor_auth_exported_authorization (struct tl_ds_auth_exported_authorization *D, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0xdf969c2d && T->type->name != 0x206963d2)) { return; }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_int (D->id, field1);
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xf11ec860, .id = "Bare_Bytes", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_bytes (D->bytes, field2);
}
void store_ds_constructor_auth_password_recovery (struct tl_ds_auth_password_recovery *D, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x137948a5 && T->type->name != 0xec86b75a)) { return; }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_string (D->email_pattern, field1);
}
void store_ds_constructor_auth_sent_code (struct tl_ds_auth_sent_code *D, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x0cc8bc16 && T->type->name != 0xf33743e9)) { return; }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x250be282, .id = "Bool", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bool (D->phone_registered, field1);
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_string (D->phone_code_hash, field2);
  struct paramed_type *field3 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_int (D->send_call_timeout, field3);
  struct paramed_type *field4 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x250be282, .id = "Bool", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bool (D->is_password, field4);
}
void store_ds_constructor_auth_sent_app_code (struct tl_ds_auth_sent_code *D, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x0cc8bc16 && T->type->name != 0xf33743e9)) { return; }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x250be282, .id = "Bool", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bool (D->phone_registered, field1);
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_string (D->phone_code_hash, field2);
  struct paramed_type *field3 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_int (D->send_call_timeout, field3);
  struct paramed_type *field4 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x250be282, .id = "Bool", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bool (D->is_password, field4);
}
void store_ds_constructor_binlog_encr_key (struct tl_ds_binlog_encr_key *D, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x0377168f && T->type->name != 0xfc88e970)) { return; }
  int multiplicity1 = PTR2INT (
  INT2PTR (64)  );
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  {
    int i = 0;
    while (i < multiplicity1) {
      store_ds_type_any (D->key[i ++], field1);
    }
  }
}
void store_ds_constructor_binlog_start (struct tl_ds_binlog_update *D, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x66ff03a6 && T->type->name != 0x9900fc59)) { return; }
}
void store_ds_constructor_binlog_dc_option (struct tl_ds_binlog_update *D, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x66ff03a6 && T->type->name != 0x9900fc59)) { return; }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_int (D->dc, field1);
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_string (D->name, field2);
  struct paramed_type *field3 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_string (D->ip, field3);
  struct paramed_type *field4 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_int (D->port, field4);
}
void store_ds_constructor_binlog_dc_option_new (struct tl_ds_binlog_update *D, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x66ff03a6 && T->type->name != 0x9900fc59)) { return; }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_int (D->flags, field1);
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_int (D->dc, field2);
  struct paramed_type *field3 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_string (D->name, field3);
  struct paramed_type *field4 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_string (D->ip, field4);
  struct paramed_type *field5 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_int (D->port, field5);
}
void store_ds_constructor_binlog_auth_key (struct tl_ds_binlog_update *D, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x66ff03a6 && T->type->name != 0x9900fc59)) { return; }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_int (D->dc, field1);
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xfc88e970, .id = "Bare_binlog.EncrKey", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_binlog_encr_key (D->key, field2);
}
void store_ds_constructor_binlog_default_dc (struct tl_ds_binlog_update *D, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x66ff03a6 && T->type->name != 0x9900fc59)) { return; }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_int (D->dc, field1);
}
void store_ds_constructor_binlog_dc_signed (struct tl_ds_binlog_update *D, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x66ff03a6 && T->type->name != 0x9900fc59)) { return; }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_int (D->dc, field1);
}
void store_ds_constructor_binlog_our_id (struct tl_ds_binlog_update *D, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x66ff03a6 && T->type->name != 0x9900fc59)) { return; }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_int (D->id, field1);
}
void store_ds_constructor_binlog_set_dh_params (struct tl_ds_binlog_update *D, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x66ff03a6 && T->type->name != 0x9900fc59)) { return; }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_int (D->root, field1);
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xfc88e970, .id = "Bare_binlog.EncrKey", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_binlog_encr_key (D->prime, field2);
  struct paramed_type *field3 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_int (D->version, field3);
}
void store_ds_constructor_binlog_set_pts (struct tl_ds_binlog_update *D, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x66ff03a6 && T->type->name != 0x9900fc59)) { return; }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_int (D->pts, field1);
}
void store_ds_constructor_binlog_set_qts (struct tl_ds_binlog_update *D, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x66ff03a6 && T->type->name != 0x9900fc59)) { return; }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_int (D->qts, field1);
}
void store_ds_constructor_binlog_set_date (struct tl_ds_binlog_update *D, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x66ff03a6 && T->type->name != 0x9900fc59)) { return; }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_int (D->date, field1);
}
void store_ds_constructor_binlog_set_seq (struct tl_ds_binlog_update *D, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x66ff03a6 && T->type->name != 0x9900fc59)) { return; }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_int (D->seq, field1);
}
void store_ds_constructor_binlog_encr_chat_delete (struct tl_ds_binlog_update *D, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x66ff03a6 && T->type->name != 0x9900fc59)) { return; }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_int (D->id, field1);
}
void store_ds_constructor_binlog_encr_chat_new (struct tl_ds_binlog_update *D, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x66ff03a6 && T->type->name != 0x9900fc59)) { return; }
  struct paramed_type *var0 = INT2PTR (*D->flags);
  out_int (PTR2INT (var0));
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_int (D->id, field2);
  if (PTR2INT (var0) & (1 << 17)) {
    struct paramed_type *field3 = 
    &(struct paramed_type){
      .type = &(struct tl_type_descr) {.name = 0xddf89345, .id = "Bare_Long", .params_num = 0, .params_types = 0},
      .params = 0,
    };
    store_ds_type_bare_long (D->access_hash, field3);
  }
  if (PTR2INT (var0) & (1 << 18)) {
    struct paramed_type *field4 = 
    &(struct paramed_type){
      .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
      .params = 0,
    };
    store_ds_type_bare_int (D->date, field4);
  }
  if (PTR2INT (var0) & (1 << 19)) {
    struct paramed_type *field5 = 
    &(struct paramed_type){
      .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
      .params = 0,
    };
    store_ds_type_bare_int (D->admin, field5);
  }
  if (PTR2INT (var0) & (1 << 20)) {
    struct paramed_type *field6 = 
    &(struct paramed_type){
      .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
      .params = 0,
    };
    store_ds_type_bare_int (D->user_id, field6);
  }
  if (PTR2INT (var0) & (1 << 21)) {
    struct paramed_type *field7 = 
    &(struct paramed_type){
      .type = &(struct tl_type_descr) {.name = 0xfc88e970, .id = "Bare_binlog.EncrKey", .params_num = 0, .params_types = 0},
      .params = 0,
    };
    store_ds_type_bare_binlog_encr_key (D->key, field7);
  }
  if (PTR2INT (var0) & (1 << 22)) {
    struct paramed_type *field8 = 
    &(struct paramed_type){
      .type = &(struct tl_type_descr) {.name = 0xfc88e970, .id = "Bare_binlog.EncrKey", .params_num = 0, .params_types = 0},
      .params = 0,
    };
    store_ds_type_bare_binlog_encr_key (D->g_key, field8);
  }
  if (PTR2INT (var0) & (1 << 23)) {
    struct paramed_type *field9 = 
    &(struct paramed_type){
      .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
      .params = 0,
    };
    store_ds_type_bare_int (D->state, field9);
  }
  if (PTR2INT (var0) & (1 << 24)) {
    struct paramed_type *field10 = 
    &(struct paramed_type){
      .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
      .params = 0,
    };
    store_ds_type_bare_int (D->ttl, field10);
  }
  if (PTR2INT (var0) & (1 << 25)) {
    struct paramed_type *field11 = 
    &(struct paramed_type){
      .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
      .params = 0,
    };
    store_ds_type_bare_int (D->layer, field11);
  }
  if (PTR2INT (var0) & (1 << 26)) {
    struct paramed_type *field12 = 
    &(struct paramed_type){
      .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
      .params = 0,
    };
    store_ds_type_bare_int (D->in_seq_no, field12);
  }
  if (PTR2INT (var0) & (1 << 26)) {
    struct paramed_type *field13 = 
    &(struct paramed_type){
      .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
      .params = 0,
    };
    store_ds_type_bare_int (D->last_in_seq_no, field13);
  }
  if (PTR2INT (var0) & (1 << 26)) {
    struct paramed_type *field14 = 
    &(struct paramed_type){
      .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
      .params = 0,
    };
    store_ds_type_bare_int (D->out_seq_no, field14);
  }
  if (PTR2INT (var0) & (1 << 27)) {
    struct paramed_type *field15 = 
    &(struct paramed_type){
      .type = &(struct tl_type_descr) {.name = 0xddf89345, .id = "Bare_Long", .params_num = 0, .params_types = 0},
      .params = 0,
    };
    store_ds_type_bare_long (D->key_fingerprint, field15);
  }
}
void store_ds_constructor_binlog_encr_chat_exchange_new (struct tl_ds_binlog_update *D, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x66ff03a6 && T->type->name != 0x9900fc59)) { return; }
  struct paramed_type *var0 = INT2PTR (*D->flags);
  out_int (PTR2INT (var0));
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_int (D->id, field2);
  if (PTR2INT (var0) & (1 << 17)) {
    struct paramed_type *field3 = 
    &(struct paramed_type){
      .type = &(struct tl_type_descr) {.name = 0xddf89345, .id = "Bare_Long", .params_num = 0, .params_types = 0},
      .params = 0,
    };
    store_ds_type_bare_long (D->exchange_id, field3);
  }
  if (PTR2INT (var0) & (1 << 18)) {
    struct paramed_type *field4 = 
    &(struct paramed_type){
      .type = &(struct tl_type_descr) {.name = 0xfc88e970, .id = "Bare_binlog.EncrKey", .params_num = 0, .params_types = 0},
      .params = 0,
    };
    store_ds_type_bare_binlog_encr_key (D->key, field4);
  }
  if (PTR2INT (var0) & (1 << 19)) {
    struct paramed_type *field5 = 
    &(struct paramed_type){
      .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
      .params = 0,
    };
    store_ds_type_bare_int (D->state, field5);
  }
}
void store_ds_constructor_binlog_user_delete (struct tl_ds_binlog_update *D, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x66ff03a6 && T->type->name != 0x9900fc59)) { return; }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_int (D->id, field1);
}
void store_ds_constructor_binlog_user_new (struct tl_ds_binlog_update *D, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x66ff03a6 && T->type->name != 0x9900fc59)) { return; }
  struct paramed_type *var0 = INT2PTR (*D->flags);
  out_int (PTR2INT (var0));
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_int (D->id, field2);
  if (PTR2INT (var0) & (1 << 17)) {
    struct paramed_type *field3 = 
    &(struct paramed_type){
      .type = &(struct tl_type_descr) {.name = 0xddf89345, .id = "Bare_Long", .params_num = 0, .params_types = 0},
      .params = 0,
    };
    store_ds_type_bare_long (D->access_hash, field3);
  }
  if (PTR2INT (var0) & (1 << 18)) {
    struct paramed_type *field4 = 
    &(struct paramed_type){
      .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
      .params = 0,
    };
    store_ds_type_bare_string (D->first_name, field4);
  }
  if (PTR2INT (var0) & (1 << 18)) {
    struct paramed_type *field5 = 
    &(struct paramed_type){
      .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
      .params = 0,
    };
    store_ds_type_bare_string (D->last_name, field5);
  }
  if (PTR2INT (var0) & (1 << 19)) {
    struct paramed_type *field6 = 
    &(struct paramed_type){
      .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
      .params = 0,
    };
    store_ds_type_bare_string (D->phone, field6);
  }
  if (PTR2INT (var0) & (1 << 20)) {
    struct paramed_type *field7 = 
    &(struct paramed_type){
      .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
      .params = 0,
    };
    store_ds_type_bare_string (D->username, field7);
  }
  if (PTR2INT (var0) & (1 << 21)) {
    struct paramed_type *field8 = 
    &(struct paramed_type){
      .type = &(struct tl_type_descr) {.name = 0xc207550a, .id = "Photo", .params_num = 0, .params_types = 0},
      .params = 0,
    };
    store_ds_type_photo (D->photo, field8);
  }
  if (PTR2INT (var0) & (1 << 22)) {
    struct paramed_type *field9 = 
    &(struct paramed_type){
      .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
      .params = 0,
    };
    store_ds_type_bare_string (D->real_first_name, field9);
  }
  if (PTR2INT (var0) & (1 << 22)) {
    struct paramed_type *field10 = 
    &(struct paramed_type){
      .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
      .params = 0,
    };
    store_ds_type_bare_string (D->real_last_name, field10);
  }
  if (PTR2INT (var0) & (1 << 23)) {
    struct paramed_type *field11 = 
    &(struct paramed_type){
      .type = &(struct tl_type_descr) {.name = 0x9a486229, .id = "UserProfilePhoto", .params_num = 0, .params_types = 0},
      .params = 0,
    };
    store_ds_type_user_profile_photo (D->user_photo, field11);
  }
  if (PTR2INT (var0) & (1 << 24)) {
    struct paramed_type *field12 = 
    &(struct paramed_type){
      .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
      .params = 0,
    };
    store_ds_type_bare_int (D->last_read_in, field12);
  }
  if (PTR2INT (var0) & (1 << 25)) {
    struct paramed_type *field13 = 
    &(struct paramed_type){
      .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
      .params = 0,
    };
    store_ds_type_bare_int (D->last_read_out, field13);
  }
  if (PTR2INT (var0) & (1 << 26)) {
    struct paramed_type *field14 = 
    &(struct paramed_type){
      .type = &(struct tl_type_descr) {.name = 0xb2e16f93, .id = "BotInfo", .params_num = 0, .params_types = 0},
      .params = 0,
    };
    store_ds_type_bot_info (D->bot_info, field14);
  }
}
void store_ds_constructor_binlog_chat_new (struct tl_ds_binlog_update *D, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x66ff03a6 && T->type->name != 0x9900fc59)) { return; }
  struct paramed_type *var0 = INT2PTR (*D->flags);
  out_int (PTR2INT (var0));
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_int (D->id, field2);
  if (PTR2INT (var0) & (1 << 17)) {
    struct paramed_type *field3 = 
    &(struct paramed_type){
      .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
      .params = 0,
    };
    store_ds_type_bare_string (D->title, field3);
  }
  if (PTR2INT (var0) & (1 << 18)) {
    struct paramed_type *field4 = 
    &(struct paramed_type){
      .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
      .params = 0,
    };
    store_ds_type_bare_int (D->user_num, field4);
  }
  if (PTR2INT (var0) & (1 << 19)) {
    struct paramed_type *field5 = 
    &(struct paramed_type){
      .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
      .params = 0,
    };
    store_ds_type_bare_int (D->date, field5);
  }
  if (PTR2INT (var0) & (1 << 20)) {
    struct paramed_type *field6 = 
    &(struct paramed_type){
      .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
      .params = 0,
    };
    store_ds_type_bare_int (D->version, field6);
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
    store_ds_type_vector ((void *)D->participants, field7);
  }
  if (PTR2INT (var0) & (1 << 21)) {
    struct paramed_type *field8 = 
    &(struct paramed_type){
      .type = &(struct tl_type_descr) {.name = 0x56922676, .id = "ChatPhoto", .params_num = 0, .params_types = 0},
      .params = 0,
    };
    store_ds_type_chat_photo (D->chat_photo, field8);
  }
  if (PTR2INT (var0) & (1 << 22)) {
    struct paramed_type *field9 = 
    &(struct paramed_type){
      .type = &(struct tl_type_descr) {.name = 0xc207550a, .id = "Photo", .params_num = 0, .params_types = 0},
      .params = 0,
    };
    store_ds_type_photo (D->photo, field9);
  }
  if (PTR2INT (var0) & (1 << 23)) {
    struct paramed_type *field10 = 
    &(struct paramed_type){
      .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
      .params = 0,
    };
    store_ds_type_bare_int (D->admin, field10);
  }
  if (PTR2INT (var0) & (1 << 24)) {
    struct paramed_type *field11 = 
    &(struct paramed_type){
      .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
      .params = 0,
    };
    store_ds_type_bare_int (D->last_read_in, field11);
  }
  if (PTR2INT (var0) & (1 << 25)) {
    struct paramed_type *field12 = 
    &(struct paramed_type){
      .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
      .params = 0,
    };
    store_ds_type_bare_int (D->last_read_out, field12);
  }
}
void store_ds_constructor_binlog_chat_add_participant (struct tl_ds_binlog_update *D, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x66ff03a6 && T->type->name != 0x9900fc59)) { return; }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_int (D->id, field1);
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_int (D->version, field2);
  struct paramed_type *field3 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_int (D->user_id, field3);
  struct paramed_type *field4 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_int (D->inviter_id, field4);
  struct paramed_type *field5 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_int (D->date, field5);
}
void store_ds_constructor_binlog_chat_del_participant (struct tl_ds_binlog_update *D, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x66ff03a6 && T->type->name != 0x9900fc59)) { return; }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_int (D->id, field1);
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_int (D->version, field2);
  struct paramed_type *field3 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_int (D->user_id, field3);
}
void store_ds_constructor_binlog_set_msg_id (struct tl_ds_binlog_update *D, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x66ff03a6 && T->type->name != 0x9900fc59)) { return; }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xddf89345, .id = "Bare_Long", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_long (D->old_id, field1);
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_int (D->new_id, field2);
}
void store_ds_constructor_binlog_message_delete (struct tl_ds_binlog_update *D, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x66ff03a6 && T->type->name != 0x9900fc59)) { return; }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xddf89345, .id = "Bare_Long", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_long (D->lid, field1);
}
void store_ds_constructor_binlog_message_new (struct tl_ds_binlog_update *D, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x66ff03a6 && T->type->name != 0x9900fc59)) { return; }
  struct paramed_type *var0 = INT2PTR (*D->flags);
  out_int (PTR2INT (var0));
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xddf89345, .id = "Bare_Long", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_long (D->lid, field2);
  if (PTR2INT (var0) & (1 << 17)) {
    struct paramed_type *field3 = 
    &(struct paramed_type){
      .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
      .params = 0,
    };
    store_ds_type_bare_int (D->from_id, field3);
  }
  if (PTR2INT (var0) & (1 << 17)) {
    struct paramed_type *field4 = 
    &(struct paramed_type){
      .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
      .params = 0,
    };
    store_ds_type_bare_int (D->to_type, field4);
  }
  if (PTR2INT (var0) & (1 << 17)) {
    struct paramed_type *field5 = 
    &(struct paramed_type){
      .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
      .params = 0,
    };
    store_ds_type_bare_int (D->to_id, field5);
  }
  if (PTR2INT (var0) & (1 << 18)) {
    struct paramed_type *field6 = 
    &(struct paramed_type){
      .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
      .params = 0,
    };
    store_ds_type_bare_int (D->fwd_from_id, field6);
  }
  if (PTR2INT (var0) & (1 << 18)) {
    struct paramed_type *field7 = 
    &(struct paramed_type){
      .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
      .params = 0,
    };
    store_ds_type_bare_int (D->fwd_date, field7);
  }
  if (PTR2INT (var0) & (1 << 19)) {
    struct paramed_type *field8 = 
    &(struct paramed_type){
      .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
      .params = 0,
    };
    store_ds_type_bare_int (D->date, field8);
  }
  if (PTR2INT (var0) & (1 << 20)) {
    struct paramed_type *field9 = 
    &(struct paramed_type){
      .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
      .params = 0,
    };
    store_ds_type_bare_string (D->message, field9);
  }
  if (PTR2INT (var0) & (1 << 21)) {
    struct paramed_type *field10 = 
    &(struct paramed_type){
      .type = &(struct tl_type_descr) {.name = 0x95f2471a, .id = "MessageMedia", .params_num = 0, .params_types = 0},
      .params = 0,
    };
    store_ds_type_message_media (D->media, field10);
  }
  if (PTR2INT (var0) & (1 << 22)) {
    struct paramed_type *field11 = 
    &(struct paramed_type){
      .type = &(struct tl_type_descr) {.name = 0x383415fa, .id = "MessageAction", .params_num = 0, .params_types = 0},
      .params = 0,
    };
    store_ds_type_message_action (D->action, field11);
  }
  if (PTR2INT (var0) & (1 << 23)) {
    struct paramed_type *field12 = 
    &(struct paramed_type){
      .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
      .params = 0,
    };
    store_ds_type_bare_int (D->reply_id, field12);
  }
  if (PTR2INT (var0) & (1 << 24)) {
    struct paramed_type *field13 = 
    &(struct paramed_type){
      .type = &(struct tl_type_descr) {.name = 0x612ca4a9, .id = "ReplyMarkup", .params_num = 0, .params_types = 0},
      .params = 0,
    };
    store_ds_type_reply_markup (D->reply_markup, field13);
  }
}
void store_ds_constructor_binlog_message_encr_new (struct tl_ds_binlog_update *D, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x66ff03a6 && T->type->name != 0x9900fc59)) { return; }
  struct paramed_type *var0 = INT2PTR (*D->flags);
  out_int (PTR2INT (var0));
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xddf89345, .id = "Bare_Long", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_long (D->lid, field2);
  if (PTR2INT (var0) & (1 << 17)) {
    struct paramed_type *field3 = 
    &(struct paramed_type){
      .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
      .params = 0,
    };
    store_ds_type_bare_int (D->from_id, field3);
  }
  if (PTR2INT (var0) & (1 << 17)) {
    struct paramed_type *field4 = 
    &(struct paramed_type){
      .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
      .params = 0,
    };
    store_ds_type_bare_int (D->to_type, field4);
  }
  if (PTR2INT (var0) & (1 << 17)) {
    struct paramed_type *field5 = 
    &(struct paramed_type){
      .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
      .params = 0,
    };
    store_ds_type_bare_int (D->to_id, field5);
  }
  if (PTR2INT (var0) & (1 << 19)) {
    struct paramed_type *field6 = 
    &(struct paramed_type){
      .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
      .params = 0,
    };
    store_ds_type_bare_int (D->date, field6);
  }
  if (PTR2INT (var0) & (1 << 20)) {
    struct paramed_type *field7 = 
    &(struct paramed_type){
      .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
      .params = 0,
    };
    store_ds_type_bare_string (D->message, field7);
  }
  if (PTR2INT (var0) & (1 << 21)) {
    struct paramed_type *field8 = 
    &(struct paramed_type){
      .type = &(struct tl_type_descr) {.name = 0x34e0d674, .id = "DecryptedMessageMedia", .params_num = 0, .params_types = 0},
      .params = 0,
    };
    store_ds_type_decrypted_message_media (D->encr_media, field8);
  }
  if (PTR2INT (var0) & (1 << 22)) {
    struct paramed_type *field9 = 
    &(struct paramed_type){
      .type = &(struct tl_type_descr) {.name = 0x4e0eefde, .id = "DecryptedMessageAction", .params_num = 0, .params_types = 0},
      .params = 0,
    };
    store_ds_type_decrypted_message_action (D->encr_action, field9);
  }
  if (PTR2INT (var0) & (1 << 23)) {
    struct paramed_type *field10 = 
    &(struct paramed_type){
      .type = &(struct tl_type_descr) {.name = 0x886fd032, .id = "EncryptedFile", .params_num = 0, .params_types = 0},
      .params = 0,
    };
    store_ds_type_encrypted_file (D->file, field10);
  }
}
void store_ds_constructor_binlog_msg_update (struct tl_ds_binlog_update *D, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x66ff03a6 && T->type->name != 0x9900fc59)) { return; }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xddf89345, .id = "Bare_Long", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_long (D->lid, field1);
}
void store_ds_constructor_binlog_reset_authorization (struct tl_ds_binlog_update *D, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x66ff03a6 && T->type->name != 0x9900fc59)) { return; }
}
void store_ds_constructor_contacts_blocked (struct tl_ds_contacts_blocked *D, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x8c1b8fb4 && T->type->name != 0x73e4704b)) { return; }
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
  store_ds_type_vector ((void *)D->blocked, field1);
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
  store_ds_type_vector ((void *)D->users, field2);
}
void store_ds_constructor_contacts_blocked_slice (struct tl_ds_contacts_blocked *D, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x8c1b8fb4 && T->type->name != 0x73e4704b)) { return; }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_int (D->count, field1);
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
  store_ds_type_vector ((void *)D->blocked, field2);
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
  store_ds_type_vector ((void *)D->users, field3);
}
void store_ds_constructor_contacts_contacts_not_modified (struct tl_ds_contacts_contacts *D, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0xd8c02560 && T->type->name != 0x273fda9f)) { return; }
}
void store_ds_constructor_contacts_contacts (struct tl_ds_contacts_contacts *D, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0xd8c02560 && T->type->name != 0x273fda9f)) { return; }
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
  store_ds_type_vector ((void *)D->contacts, field1);
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
  store_ds_type_vector ((void *)D->users, field2);
}
void store_ds_constructor_contacts_found (struct tl_ds_contacts_found *D, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x0566000e && T->type->name != 0xfa99fff1)) { return; }
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
  store_ds_type_vector ((void *)D->results, field1);
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
  store_ds_type_vector ((void *)D->users, field2);
}
void store_ds_constructor_contacts_imported_contacts (struct tl_ds_contacts_imported_contacts *D, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0xad524315 && T->type->name != 0x52adbcea)) { return; }
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
  store_ds_type_vector ((void *)D->imported, field1);
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
  store_ds_type_vector ((void *)D->retry_contacts, field2);
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
  store_ds_type_vector ((void *)D->users, field3);
}
void store_ds_constructor_contacts_link (struct tl_ds_contacts_link *D, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x3ace484c && T->type->name != 0xc531b7b3)) { return; }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x522fbc63, .id = "ContactLink", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_contact_link (D->my_link, field1);
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x522fbc63, .id = "ContactLink", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_contact_link (D->foreign_link, field2);
  struct paramed_type *field3 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x02e6c0c8, .id = "User", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_user (D->user, field3);
}
void store_ds_constructor_contacts_suggested (struct tl_ds_contacts_suggested *D, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x5649dcc5 && T->type->name != 0xa9b6233a)) { return; }
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
  store_ds_type_vector ((void *)D->results, field1);
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
  store_ds_type_vector ((void *)D->users, field2);
}
void store_ds_constructor_geochats_located (struct tl_ds_geochats_located *D, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x48feb267 && T->type->name != 0xb7014d98)) { return; }
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
  store_ds_type_vector ((void *)D->results, field1);
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
  store_ds_type_vector ((void *)D->messages, field2);
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
  store_ds_type_vector ((void *)D->chats, field3);
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
  store_ds_type_vector ((void *)D->users, field4);
}
void store_ds_constructor_geochats_messages (struct tl_ds_geochats_messages *D, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x6d0a0e59 && T->type->name != 0x92f5f1a6)) { return; }
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
  store_ds_type_vector ((void *)D->messages, field1);
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
  store_ds_type_vector ((void *)D->chats, field2);
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
  store_ds_type_vector ((void *)D->users, field3);
}
void store_ds_constructor_geochats_messages_slice (struct tl_ds_geochats_messages *D, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x6d0a0e59 && T->type->name != 0x92f5f1a6)) { return; }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_int (D->count, field1);
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
  store_ds_type_vector ((void *)D->messages, field2);
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
  store_ds_type_vector ((void *)D->chats, field3);
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
  store_ds_type_vector ((void *)D->users, field4);
}
void store_ds_constructor_geochats_stated_message (struct tl_ds_geochats_stated_message *D, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x17b1578b && T->type->name != 0xe84ea874)) { return; }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xf67b4034, .id = "GeoChatMessage", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_geo_chat_message (D->message, field1);
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
  store_ds_type_vector ((void *)D->chats, field2);
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
  store_ds_type_vector ((void *)D->users, field3);
  struct paramed_type *field4 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_int (D->seq, field4);
}
void store_ds_constructor_help_app_update (struct tl_ds_help_app_update *D, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x4ddd9627 && T->type->name != 0xb22269d8)) { return; }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_int (D->id, field1);
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x250be282, .id = "Bool", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bool (D->critical, field2);
  struct paramed_type *field3 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_string (D->url, field3);
  struct paramed_type *field4 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_string (D->text, field4);
}
void store_ds_constructor_help_no_app_update (struct tl_ds_help_app_update *D, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x4ddd9627 && T->type->name != 0xb22269d8)) { return; }
}
void store_ds_constructor_help_invite_text (struct tl_ds_help_invite_text *D, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x18cb9f78 && T->type->name != 0xe7346087)) { return; }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_string (D->message, field1);
}
void store_ds_constructor_help_support (struct tl_ds_help_support *D, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x17c6b5f6 && T->type->name != 0xe8394a09)) { return; }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_string (D->phone_number, field1);
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x02e6c0c8, .id = "User", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_user (D->user, field2);
}
void store_ds_constructor_messages_affected_history (struct tl_ds_messages_affected_history *D, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0xb45c69d1 && T->type->name != 0x4ba3962e)) { return; }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_int (D->pts, field1);
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_int (D->pts_count, field2);
  struct paramed_type *field3 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_int (D->offset, field3);
}
void store_ds_constructor_messages_affected_messages (struct tl_ds_messages_affected_messages *D, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x84d19185 && T->type->name != 0x7b2e6e7a)) { return; }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_int (D->pts, field1);
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_int (D->pts_count, field2);
}
void store_ds_constructor_messages_all_stickers_not_modified (struct tl_ds_messages_all_stickers *D, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0xb485502f && T->type->name != 0x4b7aafd0)) { return; }
}
void store_ds_constructor_messages_all_stickers (struct tl_ds_messages_all_stickers *D, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0xb485502f && T->type->name != 0x4b7aafd0)) { return; }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_string (D->hash, field1);
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
  store_ds_type_vector ((void *)D->packs, field2);
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
  store_ds_type_vector ((void *)D->sets, field3);
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
  store_ds_type_vector ((void *)D->documents, field4);
}
void store_ds_constructor_messages_chat_full (struct tl_ds_messages_chat_full *D, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0xe5d7d19c && T->type->name != 0x1a282e63)) { return; }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x2e02a614, .id = "ChatFull", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_chat_full (D->full_chat, field1);
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
  store_ds_type_vector ((void *)D->chats, field2);
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
  store_ds_type_vector ((void *)D->users, field3);
}
void store_ds_constructor_messages_chats (struct tl_ds_messages_chats *D, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x64ff9fd5 && T->type->name != 0x9b00602a)) { return; }
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
  store_ds_type_vector ((void *)D->chats, field1);
}
void store_ds_constructor_messages_dh_config_not_modified (struct tl_ds_messages_dh_config *D, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0xecc058e8 && T->type->name != 0x133fa717)) { return; }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xf11ec860, .id = "Bare_Bytes", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_bytes (D->random, field1);
}
void store_ds_constructor_messages_dh_config (struct tl_ds_messages_dh_config *D, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0xecc058e8 && T->type->name != 0x133fa717)) { return; }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_int (D->g, field1);
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xf11ec860, .id = "Bare_Bytes", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_bytes (D->p, field2);
  struct paramed_type *field3 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_int (D->version, field3);
  struct paramed_type *field4 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xf11ec860, .id = "Bare_Bytes", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_bytes (D->random, field4);
}
void store_ds_constructor_messages_dialogs (struct tl_ds_messages_dialogs *D, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x645af8b3 && T->type->name != 0x9ba5074c)) { return; }
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
  store_ds_type_vector ((void *)D->dialogs, field1);
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
  store_ds_type_vector ((void *)D->messages, field2);
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
  store_ds_type_vector ((void *)D->chats, field3);
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
  store_ds_type_vector ((void *)D->users, field4);
}
void store_ds_constructor_messages_dialogs_slice (struct tl_ds_messages_dialogs *D, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x645af8b3 && T->type->name != 0x9ba5074c)) { return; }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_int (D->count, field1);
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
  store_ds_type_vector ((void *)D->dialogs, field2);
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
  store_ds_type_vector ((void *)D->messages, field3);
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
  store_ds_type_vector ((void *)D->chats, field4);
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
  store_ds_type_vector ((void *)D->users, field5);
}
void store_ds_constructor_messages_message_empty (struct tl_ds_messages_message *D, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x3f4e0648 && T->type->name != 0xc0b1f9b7)) { return; }
}
void store_ds_constructor_messages_messages (struct tl_ds_messages_messages *D, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x8735e464 && T->type->name != 0x78ca1b9b)) { return; }
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
  store_ds_type_vector ((void *)D->messages, field1);
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
  store_ds_type_vector ((void *)D->chats, field2);
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
  store_ds_type_vector ((void *)D->users, field3);
}
void store_ds_constructor_messages_messages_slice (struct tl_ds_messages_messages *D, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x8735e464 && T->type->name != 0x78ca1b9b)) { return; }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_int (D->count, field1);
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
  store_ds_type_vector ((void *)D->messages, field2);
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
  store_ds_type_vector ((void *)D->chats, field3);
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
  store_ds_type_vector ((void *)D->users, field4);
}
void store_ds_constructor_messages_sent_encrypted_message (struct tl_ds_messages_sent_encrypted_message *D, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0xc29c7607 && T->type->name != 0x3d6389f8)) { return; }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_int (D->date, field1);
}
void store_ds_constructor_messages_sent_encrypted_file (struct tl_ds_messages_sent_encrypted_message *D, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0xc29c7607 && T->type->name != 0x3d6389f8)) { return; }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_int (D->date, field1);
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x886fd032, .id = "EncryptedFile", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_encrypted_file (D->file, field2);
}
void store_ds_constructor_messages_sent_message (struct tl_ds_messages_sent_message *D, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x799ce190 && T->type->name != 0x86631e6f)) { return; }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_int (D->id, field1);
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_int (D->date, field2);
  struct paramed_type *field3 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x95f2471a, .id = "MessageMedia", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_message_media (D->media, field3);
  struct paramed_type *field4 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_int (D->pts, field4);
  struct paramed_type *field5 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_int (D->pts_count, field5);
}
void store_ds_constructor_messages_sent_message_link (struct tl_ds_messages_sent_message *D, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x799ce190 && T->type->name != 0x86631e6f)) { return; }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_int (D->id, field1);
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_int (D->date, field2);
  struct paramed_type *field3 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x95f2471a, .id = "MessageMedia", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_message_media (D->media, field3);
  struct paramed_type *field4 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_int (D->pts, field4);
  struct paramed_type *field5 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_int (D->pts_count, field5);
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
  store_ds_type_vector ((void *)D->links, field6);
  struct paramed_type *field7 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_int (D->seq, field7);
}
void store_ds_constructor_messages_sticker_set (struct tl_ds_messages_sticker_set *D, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0xb60a24a6 && T->type->name != 0x49f5db59)) { return; }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xa7a43b17, .id = "StickerSet", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_sticker_set (D->set, field1);
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
  store_ds_type_vector ((void *)D->packs, field2);
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
  store_ds_type_vector ((void *)D->documents, field3);
}
void store_ds_constructor_messages_stickers_not_modified (struct tl_ds_messages_stickers *D, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x7bfa5710 && T->type->name != 0x8405a8ef)) { return; }
}
void store_ds_constructor_messages_stickers (struct tl_ds_messages_stickers *D, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x7bfa5710 && T->type->name != 0x8405a8ef)) { return; }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_string (D->hash, field1);
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
  store_ds_type_vector ((void *)D->stickers, field2);
}
void store_ds_constructor_photos_photo (struct tl_ds_photos_photo *D, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x20212ca8 && T->type->name != 0xdfded357)) { return; }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xc207550a, .id = "Photo", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_photo (D->photo, field1);
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
  store_ds_type_vector ((void *)D->users, field2);
}
void store_ds_constructor_photos_photos (struct tl_ds_photos_photos *D, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x98cf75f1 && T->type->name != 0x67308a0e)) { return; }
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
  store_ds_type_vector ((void *)D->photos, field1);
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
  store_ds_type_vector ((void *)D->users, field2);
}
void store_ds_constructor_photos_photos_slice (struct tl_ds_photos_photos *D, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x98cf75f1 && T->type->name != 0x67308a0e)) { return; }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_int (D->count, field1);
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
  store_ds_type_vector ((void *)D->photos, field2);
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
  store_ds_type_vector ((void *)D->users, field3);
}
void store_ds_constructor_storage_file_unknown (struct tl_ds_storage_file_type *D, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x3e2838a8 && T->type->name != 0xc1d7c757)) { return; }
}
void store_ds_constructor_storage_file_jpeg (struct tl_ds_storage_file_type *D, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x3e2838a8 && T->type->name != 0xc1d7c757)) { return; }
}
void store_ds_constructor_storage_file_gif (struct tl_ds_storage_file_type *D, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x3e2838a8 && T->type->name != 0xc1d7c757)) { return; }
}
void store_ds_constructor_storage_file_png (struct tl_ds_storage_file_type *D, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x3e2838a8 && T->type->name != 0xc1d7c757)) { return; }
}
void store_ds_constructor_storage_file_pdf (struct tl_ds_storage_file_type *D, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x3e2838a8 && T->type->name != 0xc1d7c757)) { return; }
}
void store_ds_constructor_storage_file_mp3 (struct tl_ds_storage_file_type *D, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x3e2838a8 && T->type->name != 0xc1d7c757)) { return; }
}
void store_ds_constructor_storage_file_mov (struct tl_ds_storage_file_type *D, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x3e2838a8 && T->type->name != 0xc1d7c757)) { return; }
}
void store_ds_constructor_storage_file_partial (struct tl_ds_storage_file_type *D, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x3e2838a8 && T->type->name != 0xc1d7c757)) { return; }
}
void store_ds_constructor_storage_file_mp4 (struct tl_ds_storage_file_type *D, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x3e2838a8 && T->type->name != 0xc1d7c757)) { return; }
}
void store_ds_constructor_storage_file_webp (struct tl_ds_storage_file_type *D, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x3e2838a8 && T->type->name != 0xc1d7c757)) { return; }
}
void store_ds_constructor_updates_difference_empty (struct tl_ds_updates_difference *D, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0xf57a2419 && T->type->name != 0x0a85dbe6)) { return; }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_int (D->date, field1);
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_int (D->seq, field2);
}
void store_ds_constructor_updates_difference (struct tl_ds_updates_difference *D, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0xf57a2419 && T->type->name != 0x0a85dbe6)) { return; }
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
  store_ds_type_vector ((void *)D->new_messages, field1);
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
  store_ds_type_vector ((void *)D->new_encrypted_messages, field2);
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
  store_ds_type_vector ((void *)D->other_updates, field3);
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
  store_ds_type_vector ((void *)D->chats, field4);
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
  store_ds_type_vector ((void *)D->users, field5);
  struct paramed_type *field6 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xa56c2a3e, .id = "updates.State", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_updates_state (D->state, field6);
}
void store_ds_constructor_updates_difference_slice (struct tl_ds_updates_difference *D, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0xf57a2419 && T->type->name != 0x0a85dbe6)) { return; }
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
  store_ds_type_vector ((void *)D->new_messages, field1);
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
  store_ds_type_vector ((void *)D->new_encrypted_messages, field2);
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
  store_ds_type_vector ((void *)D->other_updates, field3);
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
  store_ds_type_vector ((void *)D->chats, field4);
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
  store_ds_type_vector ((void *)D->users, field5);
  struct paramed_type *field6 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xa56c2a3e, .id = "updates.State", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_updates_state (D->intermediate_state, field6);
}
void store_ds_constructor_updates_state (struct tl_ds_updates_state *D, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0xa56c2a3e && T->type->name != 0x5a93d5c1)) { return; }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_int (D->pts, field1);
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_int (D->qts, field2);
  struct paramed_type *field3 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_int (D->date, field3);
  struct paramed_type *field4 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_int (D->seq, field4);
  struct paramed_type *field5 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_int (D->unread_count, field5);
}
void store_ds_constructor_upload_file (struct tl_ds_upload_file *D, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x096a18d5 && T->type->name != 0xf695e72a)) { return; }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x3e2838a8, .id = "storage.FileType", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_storage_file_type (D->type, field1);
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_int (D->mtime, field2);
  struct paramed_type *field3 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xf11ec860, .id = "Bare_Bytes", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  store_ds_type_bare_bytes (D->bytes, field3);
}
void store_ds_type_account_days_t_t_l (struct tl_ds_account_days_t_t_l *D, struct paramed_type *T) {
  out_int (0xb8d0afdf);
  store_ds_constructor_account_days_t_t_l (D, T); return; 
}
void store_ds_type_bare_account_days_t_t_l (struct tl_ds_account_days_t_t_l *D, struct paramed_type *T) {
  store_ds_constructor_account_days_t_t_l (D, T); return; 
}
void store_ds_type_audio (struct tl_ds_audio *D, struct paramed_type *T) {
  out_int (D->magic);
  switch (D->magic) {
  case 0x586988d8: store_ds_constructor_audio_empty (D, T); return; 
  case 0xc7ac6496: store_ds_constructor_audio (D, T); return; 
  default: assert (0);
  }
}
void store_ds_type_bare_audio (struct tl_ds_audio *D, struct paramed_type *T) {
  switch (D->magic) {
  case 0x586988d8: store_ds_constructor_audio_empty (D, T); return; 
  case 0xc7ac6496: store_ds_constructor_audio (D, T); return; 
  default: assert (0);
  }
}
void store_ds_type_authorization (struct tl_ds_authorization *D, struct paramed_type *T) {
  out_int (0x7bf2e6f6);
  store_ds_constructor_authorization (D, T); return; 
}
void store_ds_type_bare_authorization (struct tl_ds_authorization *D, struct paramed_type *T) {
  store_ds_constructor_authorization (D, T); return; 
}
void store_ds_type_bool (struct tl_ds_bool *D, struct paramed_type *T) {
  out_int (D->magic);
  switch (D->magic) {
  case 0xbc799737: store_ds_constructor_bool_false (D, T); return; 
  case 0x997275b5: store_ds_constructor_bool_true (D, T); return; 
  default: assert (0);
  }
}
void store_ds_type_bare_bool (struct tl_ds_bool *D, struct paramed_type *T) {
  switch (D->magic) {
  case 0xbc799737: store_ds_constructor_bool_false (D, T); return; 
  case 0x997275b5: store_ds_constructor_bool_true (D, T); return; 
  default: assert (0);
  }
}
void store_ds_type_bot_command (struct tl_ds_bot_command *D, struct paramed_type *T) {
  out_int (D->magic);
  switch (D->magic) {
  case 0xc27ac8c7: store_ds_constructor_bot_command (D, T); return; 
  case 0xb79d22ab: store_ds_constructor_bot_command_old (D, T); return; 
  default: assert (0);
  }
}
void store_ds_type_bare_bot_command (struct tl_ds_bot_command *D, struct paramed_type *T) {
  switch (D->magic) {
  case 0xc27ac8c7: store_ds_constructor_bot_command (D, T); return; 
  case 0xb79d22ab: store_ds_constructor_bot_command_old (D, T); return; 
  default: assert (0);
  }
}
void store_ds_type_bot_info (struct tl_ds_bot_info *D, struct paramed_type *T) {
  out_int (D->magic);
  switch (D->magic) {
  case 0xbb2e37ce: store_ds_constructor_bot_info_empty (D, T); return; 
  case 0x09cf585d: store_ds_constructor_bot_info (D, T); return; 
  default: assert (0);
  }
}
void store_ds_type_bare_bot_info (struct tl_ds_bot_info *D, struct paramed_type *T) {
  switch (D->magic) {
  case 0xbb2e37ce: store_ds_constructor_bot_info_empty (D, T); return; 
  case 0x09cf585d: store_ds_constructor_bot_info (D, T); return; 
  default: assert (0);
  }
}
void store_ds_type_bytes (struct tl_ds_string *D, struct paramed_type *T) {
  out_int (0x0ee1379f);
  store_ds_constructor_bytes (D, T); return; 
}
void store_ds_type_bare_bytes (struct tl_ds_string *D, struct paramed_type *T) {
  store_ds_constructor_bytes (D, T); return; 
}
void store_ds_type_chat (struct tl_ds_chat *D, struct paramed_type *T) {
  out_int (D->magic);
  switch (D->magic) {
  case 0x9ba2d800: store_ds_constructor_chat_empty (D, T); return; 
  case 0x6e9c9bc7: store_ds_constructor_chat (D, T); return; 
  case 0xfb0ccc41: store_ds_constructor_chat_forbidden (D, T); return; 
  case 0x75eaea5a: store_ds_constructor_geo_chat (D, T); return; 
  default: assert (0);
  }
}
void store_ds_type_bare_chat (struct tl_ds_chat *D, struct paramed_type *T) {
  switch (D->magic) {
  case 0x9ba2d800: store_ds_constructor_chat_empty (D, T); return; 
  case 0x6e9c9bc7: store_ds_constructor_chat (D, T); return; 
  case 0xfb0ccc41: store_ds_constructor_chat_forbidden (D, T); return; 
  case 0x75eaea5a: store_ds_constructor_geo_chat (D, T); return; 
  default: assert (0);
  }
}
void store_ds_type_chat_full (struct tl_ds_chat_full *D, struct paramed_type *T) {
  out_int (0x2e02a614);
  store_ds_constructor_chat_full (D, T); return; 
}
void store_ds_type_bare_chat_full (struct tl_ds_chat_full *D, struct paramed_type *T) {
  store_ds_constructor_chat_full (D, T); return; 
}
void store_ds_type_chat_invite (struct tl_ds_chat_invite *D, struct paramed_type *T) {
  out_int (D->magic);
  switch (D->magic) {
  case 0x5a686d7c: store_ds_constructor_chat_invite_already (D, T); return; 
  case 0xce917dcd: store_ds_constructor_chat_invite (D, T); return; 
  default: assert (0);
  }
}
void store_ds_type_bare_chat_invite (struct tl_ds_chat_invite *D, struct paramed_type *T) {
  switch (D->magic) {
  case 0x5a686d7c: store_ds_constructor_chat_invite_already (D, T); return; 
  case 0xce917dcd: store_ds_constructor_chat_invite (D, T); return; 
  default: assert (0);
  }
}
void store_ds_type_chat_located (struct tl_ds_chat_located *D, struct paramed_type *T) {
  out_int (0x3631cf4c);
  store_ds_constructor_chat_located (D, T); return; 
}
void store_ds_type_bare_chat_located (struct tl_ds_chat_located *D, struct paramed_type *T) {
  store_ds_constructor_chat_located (D, T); return; 
}
void store_ds_type_chat_participant (struct tl_ds_chat_participant *D, struct paramed_type *T) {
  out_int (0xc8d7493e);
  store_ds_constructor_chat_participant (D, T); return; 
}
void store_ds_type_bare_chat_participant (struct tl_ds_chat_participant *D, struct paramed_type *T) {
  store_ds_constructor_chat_participant (D, T); return; 
}
void store_ds_type_chat_participants (struct tl_ds_chat_participants *D, struct paramed_type *T) {
  out_int (D->magic);
  switch (D->magic) {
  case 0x0fd2bb8a: store_ds_constructor_chat_participants_forbidden (D, T); return; 
  case 0x7841b415: store_ds_constructor_chat_participants (D, T); return; 
  default: assert (0);
  }
}
void store_ds_type_bare_chat_participants (struct tl_ds_chat_participants *D, struct paramed_type *T) {
  switch (D->magic) {
  case 0x0fd2bb8a: store_ds_constructor_chat_participants_forbidden (D, T); return; 
  case 0x7841b415: store_ds_constructor_chat_participants (D, T); return; 
  default: assert (0);
  }
}
void store_ds_type_chat_photo (struct tl_ds_chat_photo *D, struct paramed_type *T) {
  out_int (D->magic);
  switch (D->magic) {
  case 0x37c1011c: store_ds_constructor_chat_photo_empty (D, T); return; 
  case 0x6153276a: store_ds_constructor_chat_photo (D, T); return; 
  default: assert (0);
  }
}
void store_ds_type_bare_chat_photo (struct tl_ds_chat_photo *D, struct paramed_type *T) {
  switch (D->magic) {
  case 0x37c1011c: store_ds_constructor_chat_photo_empty (D, T); return; 
  case 0x6153276a: store_ds_constructor_chat_photo (D, T); return; 
  default: assert (0);
  }
}
void store_ds_type_client_d_h_inner_data (struct tl_ds_client_d_h_inner_data *D, struct paramed_type *T) {
  out_int (0x6643b654);
  store_ds_constructor_client_d_h_inner_data (D, T); return; 
}
void store_ds_type_bare_client_d_h_inner_data (struct tl_ds_client_d_h_inner_data *D, struct paramed_type *T) {
  store_ds_constructor_client_d_h_inner_data (D, T); return; 
}
void store_ds_type_config (struct tl_ds_config *D, struct paramed_type *T) {
  out_int (0x4e32b894);
  store_ds_constructor_config (D, T); return; 
}
void store_ds_type_bare_config (struct tl_ds_config *D, struct paramed_type *T) {
  store_ds_constructor_config (D, T); return; 
}
void store_ds_type_contact (struct tl_ds_contact *D, struct paramed_type *T) {
  out_int (0xf911c994);
  store_ds_constructor_contact (D, T); return; 
}
void store_ds_type_bare_contact (struct tl_ds_contact *D, struct paramed_type *T) {
  store_ds_constructor_contact (D, T); return; 
}
void store_ds_type_contact_blocked (struct tl_ds_contact_blocked *D, struct paramed_type *T) {
  out_int (0x561bc879);
  store_ds_constructor_contact_blocked (D, T); return; 
}
void store_ds_type_bare_contact_blocked (struct tl_ds_contact_blocked *D, struct paramed_type *T) {
  store_ds_constructor_contact_blocked (D, T); return; 
}
void store_ds_type_contact_found (struct tl_ds_contact_found *D, struct paramed_type *T) {
  out_int (0xea879f95);
  store_ds_constructor_contact_found (D, T); return; 
}
void store_ds_type_bare_contact_found (struct tl_ds_contact_found *D, struct paramed_type *T) {
  store_ds_constructor_contact_found (D, T); return; 
}
void store_ds_type_contact_link (struct tl_ds_contact_link *D, struct paramed_type *T) {
  out_int (D->magic);
  switch (D->magic) {
  case 0x5f4f9247: store_ds_constructor_contact_link_unknown (D, T); return; 
  case 0xfeedd3ad: store_ds_constructor_contact_link_none (D, T); return; 
  case 0x268f3f59: store_ds_constructor_contact_link_has_phone (D, T); return; 
  case 0xd502c2d0: store_ds_constructor_contact_link_contact (D, T); return; 
  default: assert (0);
  }
}
void store_ds_type_bare_contact_link (struct tl_ds_contact_link *D, struct paramed_type *T) {
  switch (D->magic) {
  case 0x5f4f9247: store_ds_constructor_contact_link_unknown (D, T); return; 
  case 0xfeedd3ad: store_ds_constructor_contact_link_none (D, T); return; 
  case 0x268f3f59: store_ds_constructor_contact_link_has_phone (D, T); return; 
  case 0xd502c2d0: store_ds_constructor_contact_link_contact (D, T); return; 
  default: assert (0);
  }
}
void store_ds_type_contact_status (struct tl_ds_contact_status *D, struct paramed_type *T) {
  out_int (0xd3680c61);
  store_ds_constructor_contact_status (D, T); return; 
}
void store_ds_type_bare_contact_status (struct tl_ds_contact_status *D, struct paramed_type *T) {
  store_ds_constructor_contact_status (D, T); return; 
}
void store_ds_type_contact_suggested (struct tl_ds_contact_suggested *D, struct paramed_type *T) {
  out_int (0x3de191a1);
  store_ds_constructor_contact_suggested (D, T); return; 
}
void store_ds_type_bare_contact_suggested (struct tl_ds_contact_suggested *D, struct paramed_type *T) {
  store_ds_constructor_contact_suggested (D, T); return; 
}
void store_ds_type_dc_option (struct tl_ds_dc_option *D, struct paramed_type *T) {
  out_int (D->magic);
  switch (D->magic) {
  case 0x2ec2a43c: store_ds_constructor_dc_option_l28 (D, T); return; 
  case 0x05d8c6cc: store_ds_constructor_dc_option (D, T); return; 
  default: assert (0);
  }
}
void store_ds_type_bare_dc_option (struct tl_ds_dc_option *D, struct paramed_type *T) {
  switch (D->magic) {
  case 0x2ec2a43c: store_ds_constructor_dc_option_l28 (D, T); return; 
  case 0x05d8c6cc: store_ds_constructor_dc_option (D, T); return; 
  default: assert (0);
  }
}
void store_ds_type_decrypted_message (struct tl_ds_decrypted_message *D, struct paramed_type *T) {
  out_int (D->magic);
  switch (D->magic) {
  case 0x204d3878: store_ds_constructor_decrypted_message (D, T); return; 
  case 0x73164160: store_ds_constructor_decrypted_message_service (D, T); return; 
  default: assert (0);
  }
}
void store_ds_type_bare_decrypted_message (struct tl_ds_decrypted_message *D, struct paramed_type *T) {
  switch (D->magic) {
  case 0x204d3878: store_ds_constructor_decrypted_message (D, T); return; 
  case 0x73164160: store_ds_constructor_decrypted_message_service (D, T); return; 
  default: assert (0);
  }
}
void store_ds_type_decrypted_message_action (struct tl_ds_decrypted_message_action *D, struct paramed_type *T) {
  out_int (D->magic);
  switch (D->magic) {
  case 0xa1733aec: store_ds_constructor_decrypted_message_action_set_message_t_t_l (D, T); return; 
  case 0x0c4f40be: store_ds_constructor_decrypted_message_action_read_messages (D, T); return; 
  case 0x65614304: store_ds_constructor_decrypted_message_action_delete_messages (D, T); return; 
  case 0x8ac1f475: store_ds_constructor_decrypted_message_action_screenshot_messages (D, T); return; 
  case 0x6719e45c: store_ds_constructor_decrypted_message_action_flush_history (D, T); return; 
  case 0x511110b0: store_ds_constructor_decrypted_message_action_resend (D, T); return; 
  case 0xf3048883: store_ds_constructor_decrypted_message_action_notify_layer (D, T); return; 
  case 0xccb27641: store_ds_constructor_decrypted_message_action_typing (D, T); return; 
  case 0xf3c9611b: store_ds_constructor_decrypted_message_action_request_key (D, T); return; 
  case 0x6fe1735b: store_ds_constructor_decrypted_message_action_accept_key (D, T); return; 
  case 0xdd05ec6b: store_ds_constructor_decrypted_message_action_abort_key (D, T); return; 
  case 0xec2e0b9b: store_ds_constructor_decrypted_message_action_commit_key (D, T); return; 
  case 0xa82fdd63: store_ds_constructor_decrypted_message_action_noop (D, T); return; 
  default: assert (0);
  }
}
void store_ds_type_bare_decrypted_message_action (struct tl_ds_decrypted_message_action *D, struct paramed_type *T) {
  switch (D->magic) {
  case 0xa1733aec: store_ds_constructor_decrypted_message_action_set_message_t_t_l (D, T); return; 
  case 0x0c4f40be: store_ds_constructor_decrypted_message_action_read_messages (D, T); return; 
  case 0x65614304: store_ds_constructor_decrypted_message_action_delete_messages (D, T); return; 
  case 0x8ac1f475: store_ds_constructor_decrypted_message_action_screenshot_messages (D, T); return; 
  case 0x6719e45c: store_ds_constructor_decrypted_message_action_flush_history (D, T); return; 
  case 0x511110b0: store_ds_constructor_decrypted_message_action_resend (D, T); return; 
  case 0xf3048883: store_ds_constructor_decrypted_message_action_notify_layer (D, T); return; 
  case 0xccb27641: store_ds_constructor_decrypted_message_action_typing (D, T); return; 
  case 0xf3c9611b: store_ds_constructor_decrypted_message_action_request_key (D, T); return; 
  case 0x6fe1735b: store_ds_constructor_decrypted_message_action_accept_key (D, T); return; 
  case 0xdd05ec6b: store_ds_constructor_decrypted_message_action_abort_key (D, T); return; 
  case 0xec2e0b9b: store_ds_constructor_decrypted_message_action_commit_key (D, T); return; 
  case 0xa82fdd63: store_ds_constructor_decrypted_message_action_noop (D, T); return; 
  default: assert (0);
  }
}
void store_ds_type_decrypted_message_layer (struct tl_ds_decrypted_message_layer *D, struct paramed_type *T) {
  out_int (0x1be31789);
  store_ds_constructor_decrypted_message_layer (D, T); return; 
}
void store_ds_type_bare_decrypted_message_layer (struct tl_ds_decrypted_message_layer *D, struct paramed_type *T) {
  store_ds_constructor_decrypted_message_layer (D, T); return; 
}
void store_ds_type_decrypted_message_media (struct tl_ds_decrypted_message_media *D, struct paramed_type *T) {
  out_int (D->magic);
  switch (D->magic) {
  case 0x089f5c4a: store_ds_constructor_decrypted_message_media_empty (D, T); return; 
  case 0x32798a8c: store_ds_constructor_decrypted_message_media_photo (D, T); return; 
  case 0x35480a59: store_ds_constructor_decrypted_message_media_geo_point (D, T); return; 
  case 0x588a0a97: store_ds_constructor_decrypted_message_media_contact (D, T); return; 
  case 0xb095434b: store_ds_constructor_decrypted_message_media_document (D, T); return; 
  case 0x524a415d: store_ds_constructor_decrypted_message_media_video (D, T); return; 
  case 0x57e0a9cb: store_ds_constructor_decrypted_message_media_audio (D, T); return; 
  case 0xfa95b0dd: store_ds_constructor_decrypted_message_media_external_document (D, T); return; 
  case 0x4cee6ef3: store_ds_constructor_decrypted_message_media_video_l12 (D, T); return; 
  case 0x6080758f: store_ds_constructor_decrypted_message_media_audio_l12 (D, T); return; 
  default: assert (0);
  }
}
void store_ds_type_bare_decrypted_message_media (struct tl_ds_decrypted_message_media *D, struct paramed_type *T) {
  switch (D->magic) {
  case 0x089f5c4a: store_ds_constructor_decrypted_message_media_empty (D, T); return; 
  case 0x32798a8c: store_ds_constructor_decrypted_message_media_photo (D, T); return; 
  case 0x35480a59: store_ds_constructor_decrypted_message_media_geo_point (D, T); return; 
  case 0x588a0a97: store_ds_constructor_decrypted_message_media_contact (D, T); return; 
  case 0xb095434b: store_ds_constructor_decrypted_message_media_document (D, T); return; 
  case 0x524a415d: store_ds_constructor_decrypted_message_media_video (D, T); return; 
  case 0x57e0a9cb: store_ds_constructor_decrypted_message_media_audio (D, T); return; 
  case 0xfa95b0dd: store_ds_constructor_decrypted_message_media_external_document (D, T); return; 
  case 0x4cee6ef3: store_ds_constructor_decrypted_message_media_video_l12 (D, T); return; 
  case 0x6080758f: store_ds_constructor_decrypted_message_media_audio_l12 (D, T); return; 
  default: assert (0);
  }
}
void store_ds_type_dialog (struct tl_ds_dialog *D, struct paramed_type *T) {
  out_int (0xc1dd804a);
  store_ds_constructor_dialog (D, T); return; 
}
void store_ds_type_bare_dialog (struct tl_ds_dialog *D, struct paramed_type *T) {
  store_ds_constructor_dialog (D, T); return; 
}
void store_ds_type_disabled_feature (struct tl_ds_disabled_feature *D, struct paramed_type *T) {
  out_int (0xae636f24);
  store_ds_constructor_disabled_feature (D, T); return; 
}
void store_ds_type_bare_disabled_feature (struct tl_ds_disabled_feature *D, struct paramed_type *T) {
  store_ds_constructor_disabled_feature (D, T); return; 
}
void store_ds_type_document (struct tl_ds_document *D, struct paramed_type *T) {
  out_int (D->magic);
  switch (D->magic) {
  case 0x36f8c871: store_ds_constructor_document_empty (D, T); return; 
  case 0xf9a39f4f: store_ds_constructor_document (D, T); return; 
  case 0x9efc6326: store_ds_constructor_document_l19 (D, T); return; 
  default: assert (0);
  }
}
void store_ds_type_bare_document (struct tl_ds_document *D, struct paramed_type *T) {
  switch (D->magic) {
  case 0x36f8c871: store_ds_constructor_document_empty (D, T); return; 
  case 0xf9a39f4f: store_ds_constructor_document (D, T); return; 
  case 0x9efc6326: store_ds_constructor_document_l19 (D, T); return; 
  default: assert (0);
  }
}
void store_ds_type_document_attribute (struct tl_ds_document_attribute *D, struct paramed_type *T) {
  out_int (D->magic);
  switch (D->magic) {
  case 0x6c37c15c: store_ds_constructor_document_attribute_image_size (D, T); return; 
  case 0x11b58939: store_ds_constructor_document_attribute_animated (D, T); return; 
  case 0x994c9882: store_ds_constructor_document_attribute_sticker_l28 (D, T); return; 
  case 0x3a556302: store_ds_constructor_document_attribute_sticker (D, T); return; 
  case 0x5910cccb: store_ds_constructor_document_attribute_video (D, T); return; 
  case 0x051448e5: store_ds_constructor_document_attribute_audio (D, T); return; 
  case 0x15590068: store_ds_constructor_document_attribute_filename (D, T); return; 
  default: assert (0);
  }
}
void store_ds_type_bare_document_attribute (struct tl_ds_document_attribute *D, struct paramed_type *T) {
  switch (D->magic) {
  case 0x6c37c15c: store_ds_constructor_document_attribute_image_size (D, T); return; 
  case 0x11b58939: store_ds_constructor_document_attribute_animated (D, T); return; 
  case 0x994c9882: store_ds_constructor_document_attribute_sticker_l28 (D, T); return; 
  case 0x3a556302: store_ds_constructor_document_attribute_sticker (D, T); return; 
  case 0x5910cccb: store_ds_constructor_document_attribute_video (D, T); return; 
  case 0x051448e5: store_ds_constructor_document_attribute_audio (D, T); return; 
  case 0x15590068: store_ds_constructor_document_attribute_filename (D, T); return; 
  default: assert (0);
  }
}
void store_ds_type_double (double *D, struct paramed_type *T) {
  out_int (0x2210c154);
  store_ds_constructor_double (D, T); return; 
}
void store_ds_type_bare_double (double *D, struct paramed_type *T) {
  store_ds_constructor_double (D, T); return; 
}
void store_ds_type_encrypted_chat (struct tl_ds_encrypted_chat *D, struct paramed_type *T) {
  out_int (D->magic);
  switch (D->magic) {
  case 0xab7ec0a0: store_ds_constructor_encrypted_chat_empty (D, T); return; 
  case 0x3bf703dc: store_ds_constructor_encrypted_chat_waiting (D, T); return; 
  case 0xc878527e: store_ds_constructor_encrypted_chat_requested (D, T); return; 
  case 0xfa56ce36: store_ds_constructor_encrypted_chat (D, T); return; 
  case 0x13d6dd27: store_ds_constructor_encrypted_chat_discarded (D, T); return; 
  default: assert (0);
  }
}
void store_ds_type_bare_encrypted_chat (struct tl_ds_encrypted_chat *D, struct paramed_type *T) {
  switch (D->magic) {
  case 0xab7ec0a0: store_ds_constructor_encrypted_chat_empty (D, T); return; 
  case 0x3bf703dc: store_ds_constructor_encrypted_chat_waiting (D, T); return; 
  case 0xc878527e: store_ds_constructor_encrypted_chat_requested (D, T); return; 
  case 0xfa56ce36: store_ds_constructor_encrypted_chat (D, T); return; 
  case 0x13d6dd27: store_ds_constructor_encrypted_chat_discarded (D, T); return; 
  default: assert (0);
  }
}
void store_ds_type_encrypted_file (struct tl_ds_encrypted_file *D, struct paramed_type *T) {
  out_int (D->magic);
  switch (D->magic) {
  case 0xc21f497e: store_ds_constructor_encrypted_file_empty (D, T); return; 
  case 0x4a70994c: store_ds_constructor_encrypted_file (D, T); return; 
  default: assert (0);
  }
}
void store_ds_type_bare_encrypted_file (struct tl_ds_encrypted_file *D, struct paramed_type *T) {
  switch (D->magic) {
  case 0xc21f497e: store_ds_constructor_encrypted_file_empty (D, T); return; 
  case 0x4a70994c: store_ds_constructor_encrypted_file (D, T); return; 
  default: assert (0);
  }
}
void store_ds_type_encrypted_message (struct tl_ds_encrypted_message *D, struct paramed_type *T) {
  out_int (D->magic);
  switch (D->magic) {
  case 0xed18c118: store_ds_constructor_encrypted_message (D, T); return; 
  case 0x23734b06: store_ds_constructor_encrypted_message_service (D, T); return; 
  default: assert (0);
  }
}
void store_ds_type_bare_encrypted_message (struct tl_ds_encrypted_message *D, struct paramed_type *T) {
  switch (D->magic) {
  case 0xed18c118: store_ds_constructor_encrypted_message (D, T); return; 
  case 0x23734b06: store_ds_constructor_encrypted_message_service (D, T); return; 
  default: assert (0);
  }
}
void store_ds_type_error (struct tl_ds_error *D, struct paramed_type *T) {
  out_int (0xc4b9f9bb);
  store_ds_constructor_error (D, T); return; 
}
void store_ds_type_bare_error (struct tl_ds_error *D, struct paramed_type *T) {
  store_ds_constructor_error (D, T); return; 
}
void store_ds_type_exported_chat_invite (struct tl_ds_exported_chat_invite *D, struct paramed_type *T) {
  out_int (D->magic);
  switch (D->magic) {
  case 0x69df3769: store_ds_constructor_chat_invite_empty (D, T); return; 
  case 0xfc2e05bc: store_ds_constructor_chat_invite_exported (D, T); return; 
  default: assert (0);
  }
}
void store_ds_type_bare_exported_chat_invite (struct tl_ds_exported_chat_invite *D, struct paramed_type *T) {
  switch (D->magic) {
  case 0x69df3769: store_ds_constructor_chat_invite_empty (D, T); return; 
  case 0xfc2e05bc: store_ds_constructor_chat_invite_exported (D, T); return; 
  default: assert (0);
  }
}
void store_ds_type_file_location (struct tl_ds_file_location *D, struct paramed_type *T) {
  out_int (D->magic);
  switch (D->magic) {
  case 0x7c596b46: store_ds_constructor_file_location_unavailable (D, T); return; 
  case 0x53d69076: store_ds_constructor_file_location (D, T); return; 
  default: assert (0);
  }
}
void store_ds_type_bare_file_location (struct tl_ds_file_location *D, struct paramed_type *T) {
  switch (D->magic) {
  case 0x7c596b46: store_ds_constructor_file_location_unavailable (D, T); return; 
  case 0x53d69076: store_ds_constructor_file_location (D, T); return; 
  default: assert (0);
  }
}
void store_ds_type_geo_chat_message (struct tl_ds_geo_chat_message *D, struct paramed_type *T) {
  out_int (D->magic);
  switch (D->magic) {
  case 0x60311a9b: store_ds_constructor_geo_chat_message_empty (D, T); return; 
  case 0x4505f8e1: store_ds_constructor_geo_chat_message (D, T); return; 
  case 0xd34fa24e: store_ds_constructor_geo_chat_message_service (D, T); return; 
  default: assert (0);
  }
}
void store_ds_type_bare_geo_chat_message (struct tl_ds_geo_chat_message *D, struct paramed_type *T) {
  switch (D->magic) {
  case 0x60311a9b: store_ds_constructor_geo_chat_message_empty (D, T); return; 
  case 0x4505f8e1: store_ds_constructor_geo_chat_message (D, T); return; 
  case 0xd34fa24e: store_ds_constructor_geo_chat_message_service (D, T); return; 
  default: assert (0);
  }
}
void store_ds_type_geo_point (struct tl_ds_geo_point *D, struct paramed_type *T) {
  out_int (D->magic);
  switch (D->magic) {
  case 0x1117dd5f: store_ds_constructor_geo_point_empty (D, T); return; 
  case 0x2049d70c: store_ds_constructor_geo_point (D, T); return; 
  default: assert (0);
  }
}
void store_ds_type_bare_geo_point (struct tl_ds_geo_point *D, struct paramed_type *T) {
  switch (D->magic) {
  case 0x1117dd5f: store_ds_constructor_geo_point_empty (D, T); return; 
  case 0x2049d70c: store_ds_constructor_geo_point (D, T); return; 
  default: assert (0);
  }
}
void store_ds_type_imported_contact (struct tl_ds_imported_contact *D, struct paramed_type *T) {
  out_int (0xd0028438);
  store_ds_constructor_imported_contact (D, T); return; 
}
void store_ds_type_bare_imported_contact (struct tl_ds_imported_contact *D, struct paramed_type *T) {
  store_ds_constructor_imported_contact (D, T); return; 
}
void store_ds_type_input_app_event (struct tl_ds_input_app_event *D, struct paramed_type *T) {
  out_int (0x770656a8);
  store_ds_constructor_input_app_event (D, T); return; 
}
void store_ds_type_bare_input_app_event (struct tl_ds_input_app_event *D, struct paramed_type *T) {
  store_ds_constructor_input_app_event (D, T); return; 
}
void store_ds_type_input_audio (struct tl_ds_input_audio *D, struct paramed_type *T) {
  out_int (D->magic);
  switch (D->magic) {
  case 0xd95adc84: store_ds_constructor_input_audio_empty (D, T); return; 
  case 0x77d440ff: store_ds_constructor_input_audio (D, T); return; 
  default: assert (0);
  }
}
void store_ds_type_bare_input_audio (struct tl_ds_input_audio *D, struct paramed_type *T) {
  switch (D->magic) {
  case 0xd95adc84: store_ds_constructor_input_audio_empty (D, T); return; 
  case 0x77d440ff: store_ds_constructor_input_audio (D, T); return; 
  default: assert (0);
  }
}
void store_ds_type_input_chat_photo (struct tl_ds_input_chat_photo *D, struct paramed_type *T) {
  out_int (D->magic);
  switch (D->magic) {
  case 0x1ca48f57: store_ds_constructor_input_chat_photo_empty (D, T); return; 
  case 0x94254732: store_ds_constructor_input_chat_uploaded_photo (D, T); return; 
  case 0xb2e1bf08: store_ds_constructor_input_chat_photo (D, T); return; 
  default: assert (0);
  }
}
void store_ds_type_bare_input_chat_photo (struct tl_ds_input_chat_photo *D, struct paramed_type *T) {
  switch (D->magic) {
  case 0x1ca48f57: store_ds_constructor_input_chat_photo_empty (D, T); return; 
  case 0x94254732: store_ds_constructor_input_chat_uploaded_photo (D, T); return; 
  case 0xb2e1bf08: store_ds_constructor_input_chat_photo (D, T); return; 
  default: assert (0);
  }
}
void store_ds_type_input_contact (struct tl_ds_input_contact *D, struct paramed_type *T) {
  out_int (0xf392b7f4);
  store_ds_constructor_input_phone_contact (D, T); return; 
}
void store_ds_type_bare_input_contact (struct tl_ds_input_contact *D, struct paramed_type *T) {
  store_ds_constructor_input_phone_contact (D, T); return; 
}
void store_ds_type_input_document (struct tl_ds_input_document *D, struct paramed_type *T) {
  out_int (D->magic);
  switch (D->magic) {
  case 0x72f0eaae: store_ds_constructor_input_document_empty (D, T); return; 
  case 0x18798952: store_ds_constructor_input_document (D, T); return; 
  default: assert (0);
  }
}
void store_ds_type_bare_input_document (struct tl_ds_input_document *D, struct paramed_type *T) {
  switch (D->magic) {
  case 0x72f0eaae: store_ds_constructor_input_document_empty (D, T); return; 
  case 0x18798952: store_ds_constructor_input_document (D, T); return; 
  default: assert (0);
  }
}
void store_ds_type_input_encrypted_chat (struct tl_ds_input_encrypted_chat *D, struct paramed_type *T) {
  out_int (0xf141b5e1);
  store_ds_constructor_input_encrypted_chat (D, T); return; 
}
void store_ds_type_bare_input_encrypted_chat (struct tl_ds_input_encrypted_chat *D, struct paramed_type *T) {
  store_ds_constructor_input_encrypted_chat (D, T); return; 
}
void store_ds_type_input_encrypted_file (struct tl_ds_input_encrypted_file *D, struct paramed_type *T) {
  out_int (D->magic);
  switch (D->magic) {
  case 0x1837c364: store_ds_constructor_input_encrypted_file_empty (D, T); return; 
  case 0x64bd0306: store_ds_constructor_input_encrypted_file_uploaded (D, T); return; 
  case 0x5a17b5e5: store_ds_constructor_input_encrypted_file (D, T); return; 
  case 0x2dc173c8: store_ds_constructor_input_encrypted_file_big_uploaded (D, T); return; 
  default: assert (0);
  }
}
void store_ds_type_bare_input_encrypted_file (struct tl_ds_input_encrypted_file *D, struct paramed_type *T) {
  switch (D->magic) {
  case 0x1837c364: store_ds_constructor_input_encrypted_file_empty (D, T); return; 
  case 0x64bd0306: store_ds_constructor_input_encrypted_file_uploaded (D, T); return; 
  case 0x5a17b5e5: store_ds_constructor_input_encrypted_file (D, T); return; 
  case 0x2dc173c8: store_ds_constructor_input_encrypted_file_big_uploaded (D, T); return; 
  default: assert (0);
  }
}
void store_ds_type_input_file (struct tl_ds_input_file *D, struct paramed_type *T) {
  out_int (D->magic);
  switch (D->magic) {
  case 0xf52ff27f: store_ds_constructor_input_file (D, T); return; 
  case 0xfa4f0bb5: store_ds_constructor_input_file_big (D, T); return; 
  default: assert (0);
  }
}
void store_ds_type_bare_input_file (struct tl_ds_input_file *D, struct paramed_type *T) {
  switch (D->magic) {
  case 0xf52ff27f: store_ds_constructor_input_file (D, T); return; 
  case 0xfa4f0bb5: store_ds_constructor_input_file_big (D, T); return; 
  default: assert (0);
  }
}
void store_ds_type_input_file_location (struct tl_ds_input_file_location *D, struct paramed_type *T) {
  out_int (D->magic);
  switch (D->magic) {
  case 0x14637196: store_ds_constructor_input_file_location (D, T); return; 
  case 0x3d0364ec: store_ds_constructor_input_video_file_location (D, T); return; 
  case 0xf5235d55: store_ds_constructor_input_encrypted_file_location (D, T); return; 
  case 0x74dc404d: store_ds_constructor_input_audio_file_location (D, T); return; 
  case 0x4e45abe9: store_ds_constructor_input_document_file_location (D, T); return; 
  default: assert (0);
  }
}
void store_ds_type_bare_input_file_location (struct tl_ds_input_file_location *D, struct paramed_type *T) {
  switch (D->magic) {
  case 0x14637196: store_ds_constructor_input_file_location (D, T); return; 
  case 0x3d0364ec: store_ds_constructor_input_video_file_location (D, T); return; 
  case 0xf5235d55: store_ds_constructor_input_encrypted_file_location (D, T); return; 
  case 0x74dc404d: store_ds_constructor_input_audio_file_location (D, T); return; 
  case 0x4e45abe9: store_ds_constructor_input_document_file_location (D, T); return; 
  default: assert (0);
  }
}
void store_ds_type_input_geo_chat (struct tl_ds_input_geo_chat *D, struct paramed_type *T) {
  out_int (0x74d456fa);
  store_ds_constructor_input_geo_chat (D, T); return; 
}
void store_ds_type_bare_input_geo_chat (struct tl_ds_input_geo_chat *D, struct paramed_type *T) {
  store_ds_constructor_input_geo_chat (D, T); return; 
}
void store_ds_type_input_geo_point (struct tl_ds_input_geo_point *D, struct paramed_type *T) {
  out_int (D->magic);
  switch (D->magic) {
  case 0xe4c123d6: store_ds_constructor_input_geo_point_empty (D, T); return; 
  case 0xf3b7acc9: store_ds_constructor_input_geo_point (D, T); return; 
  default: assert (0);
  }
}
void store_ds_type_bare_input_geo_point (struct tl_ds_input_geo_point *D, struct paramed_type *T) {
  switch (D->magic) {
  case 0xe4c123d6: store_ds_constructor_input_geo_point_empty (D, T); return; 
  case 0xf3b7acc9: store_ds_constructor_input_geo_point (D, T); return; 
  default: assert (0);
  }
}
void store_ds_type_input_media (struct tl_ds_input_media *D, struct paramed_type *T) {
  out_int (D->magic);
  switch (D->magic) {
  case 0x9664f57f: store_ds_constructor_input_media_empty (D, T); return; 
  case 0xf7aff1c0: store_ds_constructor_input_media_uploaded_photo (D, T); return; 
  case 0xe9bfb4f3: store_ds_constructor_input_media_photo (D, T); return; 
  case 0xf9c44144: store_ds_constructor_input_media_geo_point (D, T); return; 
  case 0xa6e45987: store_ds_constructor_input_media_contact (D, T); return; 
  case 0xe13fd4bc: store_ds_constructor_input_media_uploaded_video (D, T); return; 
  case 0x96fb97dc: store_ds_constructor_input_media_uploaded_thumb_video (D, T); return; 
  case 0x936a4ebd: store_ds_constructor_input_media_video (D, T); return; 
  case 0x4e498cab: store_ds_constructor_input_media_uploaded_audio (D, T); return; 
  case 0x89938781: store_ds_constructor_input_media_audio (D, T); return; 
  case 0xffe76b78: store_ds_constructor_input_media_uploaded_document (D, T); return; 
  case 0x41481486: store_ds_constructor_input_media_uploaded_thumb_document (D, T); return; 
  case 0xd184e841: store_ds_constructor_input_media_document (D, T); return; 
  case 0x2827a81a: store_ds_constructor_input_media_venue (D, T); return; 
  default: assert (0);
  }
}
void store_ds_type_bare_input_media (struct tl_ds_input_media *D, struct paramed_type *T) {
  switch (D->magic) {
  case 0x9664f57f: store_ds_constructor_input_media_empty (D, T); return; 
  case 0xf7aff1c0: store_ds_constructor_input_media_uploaded_photo (D, T); return; 
  case 0xe9bfb4f3: store_ds_constructor_input_media_photo (D, T); return; 
  case 0xf9c44144: store_ds_constructor_input_media_geo_point (D, T); return; 
  case 0xa6e45987: store_ds_constructor_input_media_contact (D, T); return; 
  case 0xe13fd4bc: store_ds_constructor_input_media_uploaded_video (D, T); return; 
  case 0x96fb97dc: store_ds_constructor_input_media_uploaded_thumb_video (D, T); return; 
  case 0x936a4ebd: store_ds_constructor_input_media_video (D, T); return; 
  case 0x4e498cab: store_ds_constructor_input_media_uploaded_audio (D, T); return; 
  case 0x89938781: store_ds_constructor_input_media_audio (D, T); return; 
  case 0xffe76b78: store_ds_constructor_input_media_uploaded_document (D, T); return; 
  case 0x41481486: store_ds_constructor_input_media_uploaded_thumb_document (D, T); return; 
  case 0xd184e841: store_ds_constructor_input_media_document (D, T); return; 
  case 0x2827a81a: store_ds_constructor_input_media_venue (D, T); return; 
  default: assert (0);
  }
}
void store_ds_type_input_notify_peer (struct tl_ds_input_notify_peer *D, struct paramed_type *T) {
  out_int (D->magic);
  switch (D->magic) {
  case 0xb8bc5b0c: store_ds_constructor_input_notify_peer (D, T); return; 
  case 0x193b4417: store_ds_constructor_input_notify_users (D, T); return; 
  case 0x4a95e84e: store_ds_constructor_input_notify_chats (D, T); return; 
  case 0xa429b886: store_ds_constructor_input_notify_all (D, T); return; 
  case 0x4d8ddec8: store_ds_constructor_input_notify_geo_chat_peer (D, T); return; 
  default: assert (0);
  }
}
void store_ds_type_bare_input_notify_peer (struct tl_ds_input_notify_peer *D, struct paramed_type *T) {
  switch (D->magic) {
  case 0xb8bc5b0c: store_ds_constructor_input_notify_peer (D, T); return; 
  case 0x193b4417: store_ds_constructor_input_notify_users (D, T); return; 
  case 0x4a95e84e: store_ds_constructor_input_notify_chats (D, T); return; 
  case 0xa429b886: store_ds_constructor_input_notify_all (D, T); return; 
  case 0x4d8ddec8: store_ds_constructor_input_notify_geo_chat_peer (D, T); return; 
  default: assert (0);
  }
}
void store_ds_type_input_peer (struct tl_ds_input_peer *D, struct paramed_type *T) {
  out_int (D->magic);
  switch (D->magic) {
  case 0x7f3b18ea: store_ds_constructor_input_peer_empty (D, T); return; 
  case 0x7da07ec9: store_ds_constructor_input_peer_self (D, T); return; 
  case 0x1023dbe8: store_ds_constructor_input_peer_contact (D, T); return; 
  case 0x9b447325: store_ds_constructor_input_peer_foreign (D, T); return; 
  case 0x179be863: store_ds_constructor_input_peer_chat (D, T); return; 
  default: assert (0);
  }
}
void store_ds_type_bare_input_peer (struct tl_ds_input_peer *D, struct paramed_type *T) {
  switch (D->magic) {
  case 0x7f3b18ea: store_ds_constructor_input_peer_empty (D, T); return; 
  case 0x7da07ec9: store_ds_constructor_input_peer_self (D, T); return; 
  case 0x1023dbe8: store_ds_constructor_input_peer_contact (D, T); return; 
  case 0x9b447325: store_ds_constructor_input_peer_foreign (D, T); return; 
  case 0x179be863: store_ds_constructor_input_peer_chat (D, T); return; 
  default: assert (0);
  }
}
void store_ds_type_input_peer_notify_events (struct tl_ds_input_peer_notify_events *D, struct paramed_type *T) {
  out_int (D->magic);
  switch (D->magic) {
  case 0xf03064d8: store_ds_constructor_input_peer_notify_events_empty (D, T); return; 
  case 0xe86a2c74: store_ds_constructor_input_peer_notify_events_all (D, T); return; 
  default: assert (0);
  }
}
void store_ds_type_bare_input_peer_notify_events (struct tl_ds_input_peer_notify_events *D, struct paramed_type *T) {
  switch (D->magic) {
  case 0xf03064d8: store_ds_constructor_input_peer_notify_events_empty (D, T); return; 
  case 0xe86a2c74: store_ds_constructor_input_peer_notify_events_all (D, T); return; 
  default: assert (0);
  }
}
void store_ds_type_input_peer_notify_settings (struct tl_ds_input_peer_notify_settings *D, struct paramed_type *T) {
  out_int (0x46a2ce98);
  store_ds_constructor_input_peer_notify_settings (D, T); return; 
}
void store_ds_type_bare_input_peer_notify_settings (struct tl_ds_input_peer_notify_settings *D, struct paramed_type *T) {
  store_ds_constructor_input_peer_notify_settings (D, T); return; 
}
void store_ds_type_input_photo (struct tl_ds_input_photo *D, struct paramed_type *T) {
  out_int (D->magic);
  switch (D->magic) {
  case 0x1cd7bf0d: store_ds_constructor_input_photo_empty (D, T); return; 
  case 0xfb95c6c4: store_ds_constructor_input_photo (D, T); return; 
  default: assert (0);
  }
}
void store_ds_type_bare_input_photo (struct tl_ds_input_photo *D, struct paramed_type *T) {
  switch (D->magic) {
  case 0x1cd7bf0d: store_ds_constructor_input_photo_empty (D, T); return; 
  case 0xfb95c6c4: store_ds_constructor_input_photo (D, T); return; 
  default: assert (0);
  }
}
void store_ds_type_input_photo_crop (struct tl_ds_input_photo_crop *D, struct paramed_type *T) {
  out_int (D->magic);
  switch (D->magic) {
  case 0xade6b004: store_ds_constructor_input_photo_crop_auto (D, T); return; 
  case 0xd9915325: store_ds_constructor_input_photo_crop (D, T); return; 
  default: assert (0);
  }
}
void store_ds_type_bare_input_photo_crop (struct tl_ds_input_photo_crop *D, struct paramed_type *T) {
  switch (D->magic) {
  case 0xade6b004: store_ds_constructor_input_photo_crop_auto (D, T); return; 
  case 0xd9915325: store_ds_constructor_input_photo_crop (D, T); return; 
  default: assert (0);
  }
}
void store_ds_type_input_privacy_key (struct tl_ds_input_privacy_key *D, struct paramed_type *T) {
  out_int (0x4f96cb18);
  store_ds_constructor_input_privacy_key_status_timestamp (D, T); return; 
}
void store_ds_type_bare_input_privacy_key (struct tl_ds_input_privacy_key *D, struct paramed_type *T) {
  store_ds_constructor_input_privacy_key_status_timestamp (D, T); return; 
}
void store_ds_type_input_privacy_rule (struct tl_ds_input_privacy_rule *D, struct paramed_type *T) {
  out_int (D->magic);
  switch (D->magic) {
  case 0x0d09e07b: store_ds_constructor_input_privacy_value_allow_contacts (D, T); return; 
  case 0x184b35ce: store_ds_constructor_input_privacy_value_allow_all (D, T); return; 
  case 0x131cc67f: store_ds_constructor_input_privacy_value_allow_users (D, T); return; 
  case 0x0ba52007: store_ds_constructor_input_privacy_value_disallow_contacts (D, T); return; 
  case 0xd66b66c9: store_ds_constructor_input_privacy_value_disallow_all (D, T); return; 
  case 0x90110467: store_ds_constructor_input_privacy_value_disallow_users (D, T); return; 
  default: assert (0);
  }
}
void store_ds_type_bare_input_privacy_rule (struct tl_ds_input_privacy_rule *D, struct paramed_type *T) {
  switch (D->magic) {
  case 0x0d09e07b: store_ds_constructor_input_privacy_value_allow_contacts (D, T); return; 
  case 0x184b35ce: store_ds_constructor_input_privacy_value_allow_all (D, T); return; 
  case 0x131cc67f: store_ds_constructor_input_privacy_value_allow_users (D, T); return; 
  case 0x0ba52007: store_ds_constructor_input_privacy_value_disallow_contacts (D, T); return; 
  case 0xd66b66c9: store_ds_constructor_input_privacy_value_disallow_all (D, T); return; 
  case 0x90110467: store_ds_constructor_input_privacy_value_disallow_users (D, T); return; 
  default: assert (0);
  }
}
void store_ds_type_input_sticker_set (struct tl_ds_input_sticker_set *D, struct paramed_type *T) {
  out_int (D->magic);
  switch (D->magic) {
  case 0xffb62b95: store_ds_constructor_input_sticker_set_empty (D, T); return; 
  case 0x9de7a269: store_ds_constructor_input_sticker_set_i_d (D, T); return; 
  case 0x861cc8a0: store_ds_constructor_input_sticker_set_short_name (D, T); return; 
  default: assert (0);
  }
}
void store_ds_type_bare_input_sticker_set (struct tl_ds_input_sticker_set *D, struct paramed_type *T) {
  switch (D->magic) {
  case 0xffb62b95: store_ds_constructor_input_sticker_set_empty (D, T); return; 
  case 0x9de7a269: store_ds_constructor_input_sticker_set_i_d (D, T); return; 
  case 0x861cc8a0: store_ds_constructor_input_sticker_set_short_name (D, T); return; 
  default: assert (0);
  }
}
void store_ds_type_input_user (struct tl_ds_input_user *D, struct paramed_type *T) {
  out_int (D->magic);
  switch (D->magic) {
  case 0xb98886cf: store_ds_constructor_input_user_empty (D, T); return; 
  case 0xf7c1b13f: store_ds_constructor_input_user_self (D, T); return; 
  case 0x86e94f65: store_ds_constructor_input_user_contact (D, T); return; 
  case 0x655e74ff: store_ds_constructor_input_user_foreign (D, T); return; 
  default: assert (0);
  }
}
void store_ds_type_bare_input_user (struct tl_ds_input_user *D, struct paramed_type *T) {
  switch (D->magic) {
  case 0xb98886cf: store_ds_constructor_input_user_empty (D, T); return; 
  case 0xf7c1b13f: store_ds_constructor_input_user_self (D, T); return; 
  case 0x86e94f65: store_ds_constructor_input_user_contact (D, T); return; 
  case 0x655e74ff: store_ds_constructor_input_user_foreign (D, T); return; 
  default: assert (0);
  }
}
void store_ds_type_input_video (struct tl_ds_input_video *D, struct paramed_type *T) {
  out_int (D->magic);
  switch (D->magic) {
  case 0x5508ec75: store_ds_constructor_input_video_empty (D, T); return; 
  case 0xee579652: store_ds_constructor_input_video (D, T); return; 
  default: assert (0);
  }
}
void store_ds_type_bare_input_video (struct tl_ds_input_video *D, struct paramed_type *T) {
  switch (D->magic) {
  case 0x5508ec75: store_ds_constructor_input_video_empty (D, T); return; 
  case 0xee579652: store_ds_constructor_input_video (D, T); return; 
  default: assert (0);
  }
}
void store_ds_type_int (int *D, struct paramed_type *T) {
  out_int (0xa8509bda);
  store_ds_constructor_int (D, T); return; 
}
void store_ds_type_bare_int (int *D, struct paramed_type *T) {
  store_ds_constructor_int (D, T); return; 
}
void store_ds_type_int128 (struct tl_ds_int128 *D, struct paramed_type *T) {
  out_int (0x7d36c439);
  store_ds_constructor_int128 (D, T); return; 
}
void store_ds_type_bare_int128 (struct tl_ds_int128 *D, struct paramed_type *T) {
  store_ds_constructor_int128 (D, T); return; 
}
void store_ds_type_int256 (struct tl_ds_int256 *D, struct paramed_type *T) {
  out_int (0xf2c798b3);
  store_ds_constructor_int256 (D, T); return; 
}
void store_ds_type_bare_int256 (struct tl_ds_int256 *D, struct paramed_type *T) {
  store_ds_constructor_int256 (D, T); return; 
}
void store_ds_type_keyboard_button (struct tl_ds_keyboard_button *D, struct paramed_type *T) {
  out_int (0xa2fa4880);
  store_ds_constructor_keyboard_button (D, T); return; 
}
void store_ds_type_bare_keyboard_button (struct tl_ds_keyboard_button *D, struct paramed_type *T) {
  store_ds_constructor_keyboard_button (D, T); return; 
}
void store_ds_type_keyboard_button_row (struct tl_ds_keyboard_button_row *D, struct paramed_type *T) {
  out_int (0x77608b83);
  store_ds_constructor_keyboard_button_row (D, T); return; 
}
void store_ds_type_bare_keyboard_button_row (struct tl_ds_keyboard_button_row *D, struct paramed_type *T) {
  store_ds_constructor_keyboard_button_row (D, T); return; 
}
void store_ds_type_long (long long *D, struct paramed_type *T) {
  out_int (0x22076cba);
  store_ds_constructor_long (D, T); return; 
}
void store_ds_type_bare_long (long long *D, struct paramed_type *T) {
  store_ds_constructor_long (D, T); return; 
}
void store_ds_type_message (struct tl_ds_message *D, struct paramed_type *T) {
  out_int (D->magic);
  switch (D->magic) {
  case 0x83e5de54: store_ds_constructor_message_empty (D, T); return; 
  case 0xc3060325: store_ds_constructor_message (D, T); return; 
  case 0x1d86f70e: store_ds_constructor_message_service (D, T); return; 
  default: assert (0);
  }
}
void store_ds_type_bare_message (struct tl_ds_message *D, struct paramed_type *T) {
  switch (D->magic) {
  case 0x83e5de54: store_ds_constructor_message_empty (D, T); return; 
  case 0xc3060325: store_ds_constructor_message (D, T); return; 
  case 0x1d86f70e: store_ds_constructor_message_service (D, T); return; 
  default: assert (0);
  }
}
void store_ds_type_message_action (struct tl_ds_message_action *D, struct paramed_type *T) {
  out_int (D->magic);
  switch (D->magic) {
  case 0xb6aef7b0: store_ds_constructor_message_action_empty (D, T); return; 
  case 0xa6638b9a: store_ds_constructor_message_action_chat_create (D, T); return; 
  case 0xb5a1ce5a: store_ds_constructor_message_action_chat_edit_title (D, T); return; 
  case 0x7fcb13a8: store_ds_constructor_message_action_chat_edit_photo (D, T); return; 
  case 0x95e3fbef: store_ds_constructor_message_action_chat_delete_photo (D, T); return; 
  case 0x5e3cfc4b: store_ds_constructor_message_action_chat_add_user (D, T); return; 
  case 0xb2ae9b0c: store_ds_constructor_message_action_chat_delete_user (D, T); return; 
  case 0x6f038ebc: store_ds_constructor_message_action_geo_chat_create (D, T); return; 
  case 0x0c7d53de: store_ds_constructor_message_action_geo_chat_checkin (D, T); return; 
  case 0xf89cf5e8: store_ds_constructor_message_action_chat_joined_by_link (D, T); return; 
  default: assert (0);
  }
}
void store_ds_type_bare_message_action (struct tl_ds_message_action *D, struct paramed_type *T) {
  switch (D->magic) {
  case 0xb6aef7b0: store_ds_constructor_message_action_empty (D, T); return; 
  case 0xa6638b9a: store_ds_constructor_message_action_chat_create (D, T); return; 
  case 0xb5a1ce5a: store_ds_constructor_message_action_chat_edit_title (D, T); return; 
  case 0x7fcb13a8: store_ds_constructor_message_action_chat_edit_photo (D, T); return; 
  case 0x95e3fbef: store_ds_constructor_message_action_chat_delete_photo (D, T); return; 
  case 0x5e3cfc4b: store_ds_constructor_message_action_chat_add_user (D, T); return; 
  case 0xb2ae9b0c: store_ds_constructor_message_action_chat_delete_user (D, T); return; 
  case 0x6f038ebc: store_ds_constructor_message_action_geo_chat_create (D, T); return; 
  case 0x0c7d53de: store_ds_constructor_message_action_geo_chat_checkin (D, T); return; 
  case 0xf89cf5e8: store_ds_constructor_message_action_chat_joined_by_link (D, T); return; 
  default: assert (0);
  }
}
void store_ds_type_message_media (struct tl_ds_message_media *D, struct paramed_type *T) {
  out_int (D->magic);
  switch (D->magic) {
  case 0x3ded6320: store_ds_constructor_message_media_empty (D, T); return; 
  case 0x3d8ce53d: store_ds_constructor_message_media_photo (D, T); return; 
  case 0x5bcf1675: store_ds_constructor_message_media_video (D, T); return; 
  case 0x56e0d474: store_ds_constructor_message_media_geo (D, T); return; 
  case 0x5e7d2f39: store_ds_constructor_message_media_contact (D, T); return; 
  case 0x9f84f49e: store_ds_constructor_message_media_unsupported (D, T); return; 
  case 0x2fda2204: store_ds_constructor_message_media_document (D, T); return; 
  case 0xc6b68300: store_ds_constructor_message_media_audio (D, T); return; 
  case 0xa32dd600: store_ds_constructor_message_media_web_page (D, T); return; 
  case 0x7912b71f: store_ds_constructor_message_media_venue (D, T); return; 
  case 0xc8c45a2a: store_ds_constructor_message_media_photo_l27 (D, T); return; 
  case 0xa2d24290: store_ds_constructor_message_media_video_l27 (D, T); return; 
  default: assert (0);
  }
}
void store_ds_type_bare_message_media (struct tl_ds_message_media *D, struct paramed_type *T) {
  switch (D->magic) {
  case 0x3ded6320: store_ds_constructor_message_media_empty (D, T); return; 
  case 0x3d8ce53d: store_ds_constructor_message_media_photo (D, T); return; 
  case 0x5bcf1675: store_ds_constructor_message_media_video (D, T); return; 
  case 0x56e0d474: store_ds_constructor_message_media_geo (D, T); return; 
  case 0x5e7d2f39: store_ds_constructor_message_media_contact (D, T); return; 
  case 0x9f84f49e: store_ds_constructor_message_media_unsupported (D, T); return; 
  case 0x2fda2204: store_ds_constructor_message_media_document (D, T); return; 
  case 0xc6b68300: store_ds_constructor_message_media_audio (D, T); return; 
  case 0xa32dd600: store_ds_constructor_message_media_web_page (D, T); return; 
  case 0x7912b71f: store_ds_constructor_message_media_venue (D, T); return; 
  case 0xc8c45a2a: store_ds_constructor_message_media_photo_l27 (D, T); return; 
  case 0xa2d24290: store_ds_constructor_message_media_video_l27 (D, T); return; 
  default: assert (0);
  }
}
void store_ds_type_messages_filter (struct tl_ds_messages_filter *D, struct paramed_type *T) {
  out_int (D->magic);
  switch (D->magic) {
  case 0x57e2f66c: store_ds_constructor_input_messages_filter_empty (D, T); return; 
  case 0x9609a51c: store_ds_constructor_input_messages_filter_photos (D, T); return; 
  case 0x9fc00e65: store_ds_constructor_input_messages_filter_video (D, T); return; 
  case 0x56e9f0e4: store_ds_constructor_input_messages_filter_photo_video (D, T); return; 
  case 0xd95e73bb: store_ds_constructor_input_messages_filter_photo_video_documents (D, T); return; 
  case 0x9eddf188: store_ds_constructor_input_messages_filter_document (D, T); return; 
  case 0xcfc87522: store_ds_constructor_input_messages_filter_audio (D, T); return; 
  default: assert (0);
  }
}
void store_ds_type_bare_messages_filter (struct tl_ds_messages_filter *D, struct paramed_type *T) {
  switch (D->magic) {
  case 0x57e2f66c: store_ds_constructor_input_messages_filter_empty (D, T); return; 
  case 0x9609a51c: store_ds_constructor_input_messages_filter_photos (D, T); return; 
  case 0x9fc00e65: store_ds_constructor_input_messages_filter_video (D, T); return; 
  case 0x56e9f0e4: store_ds_constructor_input_messages_filter_photo_video (D, T); return; 
  case 0xd95e73bb: store_ds_constructor_input_messages_filter_photo_video_documents (D, T); return; 
  case 0x9eddf188: store_ds_constructor_input_messages_filter_document (D, T); return; 
  case 0xcfc87522: store_ds_constructor_input_messages_filter_audio (D, T); return; 
  default: assert (0);
  }
}
void store_ds_type_nearest_dc (struct tl_ds_nearest_dc *D, struct paramed_type *T) {
  out_int (0x8e1a1775);
  store_ds_constructor_nearest_dc (D, T); return; 
}
void store_ds_type_bare_nearest_dc (struct tl_ds_nearest_dc *D, struct paramed_type *T) {
  store_ds_constructor_nearest_dc (D, T); return; 
}
void store_ds_type_notify_peer (struct tl_ds_notify_peer *D, struct paramed_type *T) {
  out_int (D->magic);
  switch (D->magic) {
  case 0x9fd40bd8: store_ds_constructor_notify_peer (D, T); return; 
  case 0xb4c83b4c: store_ds_constructor_notify_users (D, T); return; 
  case 0xc007cec3: store_ds_constructor_notify_chats (D, T); return; 
  case 0x74d07c60: store_ds_constructor_notify_all (D, T); return; 
  default: assert (0);
  }
}
void store_ds_type_bare_notify_peer (struct tl_ds_notify_peer *D, struct paramed_type *T) {
  switch (D->magic) {
  case 0x9fd40bd8: store_ds_constructor_notify_peer (D, T); return; 
  case 0xb4c83b4c: store_ds_constructor_notify_users (D, T); return; 
  case 0xc007cec3: store_ds_constructor_notify_chats (D, T); return; 
  case 0x74d07c60: store_ds_constructor_notify_all (D, T); return; 
  default: assert (0);
  }
}
void store_ds_type_null (struct tl_ds_null *D, struct paramed_type *T) {
  out_int (0x56730bcc);
  store_ds_constructor_null (D, T); return; 
}
void store_ds_type_bare_null (struct tl_ds_null *D, struct paramed_type *T) {
  store_ds_constructor_null (D, T); return; 
}
void store_ds_type_p_q_inner_data (struct tl_ds_p_q_inner_data *D, struct paramed_type *T) {
  out_int (D->magic);
  switch (D->magic) {
  case 0x83c95aec: store_ds_constructor_p_q_inner_data (D, T); return; 
  case 0x3c6a84d4: store_ds_constructor_p_q_inner_data_temp (D, T); return; 
  default: assert (0);
  }
}
void store_ds_type_bare_p_q_inner_data (struct tl_ds_p_q_inner_data *D, struct paramed_type *T) {
  switch (D->magic) {
  case 0x83c95aec: store_ds_constructor_p_q_inner_data (D, T); return; 
  case 0x3c6a84d4: store_ds_constructor_p_q_inner_data_temp (D, T); return; 
  default: assert (0);
  }
}
void store_ds_type_peer (struct tl_ds_peer *D, struct paramed_type *T) {
  out_int (D->magic);
  switch (D->magic) {
  case 0x9db1bc6d: store_ds_constructor_peer_user (D, T); return; 
  case 0xbad0e5bb: store_ds_constructor_peer_chat (D, T); return; 
  default: assert (0);
  }
}
void store_ds_type_bare_peer (struct tl_ds_peer *D, struct paramed_type *T) {
  switch (D->magic) {
  case 0x9db1bc6d: store_ds_constructor_peer_user (D, T); return; 
  case 0xbad0e5bb: store_ds_constructor_peer_chat (D, T); return; 
  default: assert (0);
  }
}
void store_ds_type_peer_notify_events (struct tl_ds_peer_notify_events *D, struct paramed_type *T) {
  out_int (D->magic);
  switch (D->magic) {
  case 0xadd53cb3: store_ds_constructor_peer_notify_events_empty (D, T); return; 
  case 0x6d1ded88: store_ds_constructor_peer_notify_events_all (D, T); return; 
  default: assert (0);
  }
}
void store_ds_type_bare_peer_notify_events (struct tl_ds_peer_notify_events *D, struct paramed_type *T) {
  switch (D->magic) {
  case 0xadd53cb3: store_ds_constructor_peer_notify_events_empty (D, T); return; 
  case 0x6d1ded88: store_ds_constructor_peer_notify_events_all (D, T); return; 
  default: assert (0);
  }
}
void store_ds_type_peer_notify_settings (struct tl_ds_peer_notify_settings *D, struct paramed_type *T) {
  out_int (D->magic);
  switch (D->magic) {
  case 0x70a68512: store_ds_constructor_peer_notify_settings_empty (D, T); return; 
  case 0x8d5e11ee: store_ds_constructor_peer_notify_settings (D, T); return; 
  default: assert (0);
  }
}
void store_ds_type_bare_peer_notify_settings (struct tl_ds_peer_notify_settings *D, struct paramed_type *T) {
  switch (D->magic) {
  case 0x70a68512: store_ds_constructor_peer_notify_settings_empty (D, T); return; 
  case 0x8d5e11ee: store_ds_constructor_peer_notify_settings (D, T); return; 
  default: assert (0);
  }
}
void store_ds_type_photo (struct tl_ds_photo *D, struct paramed_type *T) {
  out_int (D->magic);
  switch (D->magic) {
  case 0x2331b22d: store_ds_constructor_photo_empty (D, T); return; 
  case 0xc3838076: store_ds_constructor_photo (D, T); return; 
  case 0x22b56751: store_ds_constructor_photo_l27 (D, T); return; 
  default: assert (0);
  }
}
void store_ds_type_bare_photo (struct tl_ds_photo *D, struct paramed_type *T) {
  switch (D->magic) {
  case 0x2331b22d: store_ds_constructor_photo_empty (D, T); return; 
  case 0xc3838076: store_ds_constructor_photo (D, T); return; 
  case 0x22b56751: store_ds_constructor_photo_l27 (D, T); return; 
  default: assert (0);
  }
}
void store_ds_type_photo_size (struct tl_ds_photo_size *D, struct paramed_type *T) {
  out_int (D->magic);
  switch (D->magic) {
  case 0x0e17e23c: store_ds_constructor_photo_size_empty (D, T); return; 
  case 0x77bfb61b: store_ds_constructor_photo_size (D, T); return; 
  case 0xe9a734fa: store_ds_constructor_photo_cached_size (D, T); return; 
  default: assert (0);
  }
}
void store_ds_type_bare_photo_size (struct tl_ds_photo_size *D, struct paramed_type *T) {
  switch (D->magic) {
  case 0x0e17e23c: store_ds_constructor_photo_size_empty (D, T); return; 
  case 0x77bfb61b: store_ds_constructor_photo_size (D, T); return; 
  case 0xe9a734fa: store_ds_constructor_photo_cached_size (D, T); return; 
  default: assert (0);
  }
}
void store_ds_type_privacy_key (struct tl_ds_privacy_key *D, struct paramed_type *T) {
  out_int (0xbc2eab30);
  store_ds_constructor_privacy_key_status_timestamp (D, T); return; 
}
void store_ds_type_bare_privacy_key (struct tl_ds_privacy_key *D, struct paramed_type *T) {
  store_ds_constructor_privacy_key_status_timestamp (D, T); return; 
}
void store_ds_type_privacy_rule (struct tl_ds_privacy_rule *D, struct paramed_type *T) {
  out_int (D->magic);
  switch (D->magic) {
  case 0xfffe1bac: store_ds_constructor_privacy_value_allow_contacts (D, T); return; 
  case 0x65427b82: store_ds_constructor_privacy_value_allow_all (D, T); return; 
  case 0x4d5bbe0c: store_ds_constructor_privacy_value_allow_users (D, T); return; 
  case 0xf888fa1a: store_ds_constructor_privacy_value_disallow_contacts (D, T); return; 
  case 0x8b73e763: store_ds_constructor_privacy_value_disallow_all (D, T); return; 
  case 0x0c7f49b7: store_ds_constructor_privacy_value_disallow_users (D, T); return; 
  default: assert (0);
  }
}
void store_ds_type_bare_privacy_rule (struct tl_ds_privacy_rule *D, struct paramed_type *T) {
  switch (D->magic) {
  case 0xfffe1bac: store_ds_constructor_privacy_value_allow_contacts (D, T); return; 
  case 0x65427b82: store_ds_constructor_privacy_value_allow_all (D, T); return; 
  case 0x4d5bbe0c: store_ds_constructor_privacy_value_allow_users (D, T); return; 
  case 0xf888fa1a: store_ds_constructor_privacy_value_disallow_contacts (D, T); return; 
  case 0x8b73e763: store_ds_constructor_privacy_value_disallow_all (D, T); return; 
  case 0x0c7f49b7: store_ds_constructor_privacy_value_disallow_users (D, T); return; 
  default: assert (0);
  }
}
void store_ds_type_received_notify_message (struct tl_ds_received_notify_message *D, struct paramed_type *T) {
  out_int (0xa384b779);
  store_ds_constructor_received_notify_message (D, T); return; 
}
void store_ds_type_bare_received_notify_message (struct tl_ds_received_notify_message *D, struct paramed_type *T) {
  store_ds_constructor_received_notify_message (D, T); return; 
}
void store_ds_type_reply_markup (struct tl_ds_reply_markup *D, struct paramed_type *T) {
  out_int (D->magic);
  switch (D->magic) {
  case 0xa03e5b85: store_ds_constructor_reply_keyboard_hide (D, T); return; 
  case 0xf4108aa0: store_ds_constructor_reply_keyboard_force_reply (D, T); return; 
  case 0x3502758c: store_ds_constructor_reply_keyboard_markup (D, T); return; 
  default: assert (0);
  }
}
void store_ds_type_bare_reply_markup (struct tl_ds_reply_markup *D, struct paramed_type *T) {
  switch (D->magic) {
  case 0xa03e5b85: store_ds_constructor_reply_keyboard_hide (D, T); return; 
  case 0xf4108aa0: store_ds_constructor_reply_keyboard_force_reply (D, T); return; 
  case 0x3502758c: store_ds_constructor_reply_keyboard_markup (D, T); return; 
  default: assert (0);
  }
}
void store_ds_type_res_p_q (struct tl_ds_res_p_q *D, struct paramed_type *T) {
  out_int (0x05162463);
  store_ds_constructor_res_p_q (D, T); return; 
}
void store_ds_type_bare_res_p_q (struct tl_ds_res_p_q *D, struct paramed_type *T) {
  store_ds_constructor_res_p_q (D, T); return; 
}
void store_ds_type_send_message_action (struct tl_ds_send_message_action *D, struct paramed_type *T) {
  out_int (D->magic);
  switch (D->magic) {
  case 0x16bf744e: store_ds_constructor_send_message_typing_action (D, T); return; 
  case 0xfd5ec8f5: store_ds_constructor_send_message_cancel_action (D, T); return; 
  case 0xa187d66f: store_ds_constructor_send_message_record_video_action (D, T); return; 
  case 0x92042ff7: store_ds_constructor_send_message_upload_video_action_l27 (D, T); return; 
  case 0xe9763aec: store_ds_constructor_send_message_upload_video_action (D, T); return; 
  case 0xd52f73f7: store_ds_constructor_send_message_record_audio_action (D, T); return; 
  case 0xe6ac8a6f: store_ds_constructor_send_message_upload_audio_action_l27 (D, T); return; 
  case 0xf351d7ab: store_ds_constructor_send_message_upload_audio_action (D, T); return; 
  case 0xd1d34a26: store_ds_constructor_send_message_upload_photo_action (D, T); return; 
  case 0x8faee98e: store_ds_constructor_send_message_upload_document_action_l27 (D, T); return; 
  case 0xaa0cd9e4: store_ds_constructor_send_message_upload_document_action (D, T); return; 
  case 0x176f8ba1: store_ds_constructor_send_message_geo_location_action (D, T); return; 
  case 0x628cbc6f: store_ds_constructor_send_message_choose_contact_action (D, T); return; 
  default: assert (0);
  }
}
void store_ds_type_bare_send_message_action (struct tl_ds_send_message_action *D, struct paramed_type *T) {
  switch (D->magic) {
  case 0x16bf744e: store_ds_constructor_send_message_typing_action (D, T); return; 
  case 0xfd5ec8f5: store_ds_constructor_send_message_cancel_action (D, T); return; 
  case 0xa187d66f: store_ds_constructor_send_message_record_video_action (D, T); return; 
  case 0x92042ff7: store_ds_constructor_send_message_upload_video_action_l27 (D, T); return; 
  case 0xe9763aec: store_ds_constructor_send_message_upload_video_action (D, T); return; 
  case 0xd52f73f7: store_ds_constructor_send_message_record_audio_action (D, T); return; 
  case 0xe6ac8a6f: store_ds_constructor_send_message_upload_audio_action_l27 (D, T); return; 
  case 0xf351d7ab: store_ds_constructor_send_message_upload_audio_action (D, T); return; 
  case 0xd1d34a26: store_ds_constructor_send_message_upload_photo_action (D, T); return; 
  case 0x8faee98e: store_ds_constructor_send_message_upload_document_action_l27 (D, T); return; 
  case 0xaa0cd9e4: store_ds_constructor_send_message_upload_document_action (D, T); return; 
  case 0x176f8ba1: store_ds_constructor_send_message_geo_location_action (D, T); return; 
  case 0x628cbc6f: store_ds_constructor_send_message_choose_contact_action (D, T); return; 
  default: assert (0);
  }
}
void store_ds_type_server_d_h_params (struct tl_ds_server_d_h_params *D, struct paramed_type *T) {
  out_int (D->magic);
  switch (D->magic) {
  case 0x79cb045d: store_ds_constructor_server_d_h_params_fail (D, T); return; 
  case 0xd0e8075c: store_ds_constructor_server_d_h_params_ok (D, T); return; 
  default: assert (0);
  }
}
void store_ds_type_bare_server_d_h_params (struct tl_ds_server_d_h_params *D, struct paramed_type *T) {
  switch (D->magic) {
  case 0x79cb045d: store_ds_constructor_server_d_h_params_fail (D, T); return; 
  case 0xd0e8075c: store_ds_constructor_server_d_h_params_ok (D, T); return; 
  default: assert (0);
  }
}
void store_ds_type_server_d_h_inner_data (struct tl_ds_server_d_h_inner_data *D, struct paramed_type *T) {
  out_int (0xb5890dba);
  store_ds_constructor_server_d_h_inner_data (D, T); return; 
}
void store_ds_type_bare_server_d_h_inner_data (struct tl_ds_server_d_h_inner_data *D, struct paramed_type *T) {
  store_ds_constructor_server_d_h_inner_data (D, T); return; 
}
void store_ds_type_set_client_d_h_params_answer (struct tl_ds_set_client_d_h_params_answer *D, struct paramed_type *T) {
  out_int (D->magic);
  switch (D->magic) {
  case 0x3bcbf734: store_ds_constructor_dh_gen_ok (D, T); return; 
  case 0x46dc1fb9: store_ds_constructor_dh_gen_retry (D, T); return; 
  case 0xa69dae02: store_ds_constructor_dh_gen_fail (D, T); return; 
  default: assert (0);
  }
}
void store_ds_type_bare_set_client_d_h_params_answer (struct tl_ds_set_client_d_h_params_answer *D, struct paramed_type *T) {
  switch (D->magic) {
  case 0x3bcbf734: store_ds_constructor_dh_gen_ok (D, T); return; 
  case 0x46dc1fb9: store_ds_constructor_dh_gen_retry (D, T); return; 
  case 0xa69dae02: store_ds_constructor_dh_gen_fail (D, T); return; 
  default: assert (0);
  }
}
void store_ds_type_sticker_pack (struct tl_ds_sticker_pack *D, struct paramed_type *T) {
  out_int (0x12b299d4);
  store_ds_constructor_sticker_pack (D, T); return; 
}
void store_ds_type_bare_sticker_pack (struct tl_ds_sticker_pack *D, struct paramed_type *T) {
  store_ds_constructor_sticker_pack (D, T); return; 
}
void store_ds_type_sticker_set (struct tl_ds_sticker_set *D, struct paramed_type *T) {
  out_int (0xa7a43b17);
  store_ds_constructor_sticker_set (D, T); return; 
}
void store_ds_type_bare_sticker_set (struct tl_ds_sticker_set *D, struct paramed_type *T) {
  store_ds_constructor_sticker_set (D, T); return; 
}
void store_ds_type_string (struct tl_ds_string *D, struct paramed_type *T) {
  out_int (0xb5286e24);
  store_ds_constructor_string (D, T); return; 
}
void store_ds_type_bare_string (struct tl_ds_string *D, struct paramed_type *T) {
  store_ds_constructor_string (D, T); return; 
}
void store_ds_type_update (struct tl_ds_update *D, struct paramed_type *T) {
  out_int (D->magic);
  switch (D->magic) {
  case 0x1f2b0afd: store_ds_constructor_update_new_message (D, T); return; 
  case 0x4e90bfd6: store_ds_constructor_update_message_i_d (D, T); return; 
  case 0xa20db0e5: store_ds_constructor_update_delete_messages (D, T); return; 
  case 0x5c486927: store_ds_constructor_update_user_typing (D, T); return; 
  case 0x9a65ea1f: store_ds_constructor_update_chat_user_typing (D, T); return; 
  case 0x07761198: store_ds_constructor_update_chat_participants (D, T); return; 
  case 0x1bfbd823: store_ds_constructor_update_user_status (D, T); return; 
  case 0xa7332b73: store_ds_constructor_update_user_name (D, T); return; 
  case 0x95313b0c: store_ds_constructor_update_user_photo (D, T); return; 
  case 0x2575bbb9: store_ds_constructor_update_contact_registered (D, T); return; 
  case 0x9d2e67c5: store_ds_constructor_update_contact_link (D, T); return; 
  case 0x8f06529a: store_ds_constructor_update_new_authorization (D, T); return; 
  case 0x5a68e3f7: store_ds_constructor_update_new_geo_chat_message (D, T); return; 
  case 0x12bcbd9a: store_ds_constructor_update_new_encrypted_message (D, T); return; 
  case 0x1710f156: store_ds_constructor_update_encrypted_chat_typing (D, T); return; 
  case 0xb4a2e88d: store_ds_constructor_update_encryption (D, T); return; 
  case 0x38fe25b7: store_ds_constructor_update_encrypted_messages_read (D, T); return; 
  case 0x3a0eeb22: store_ds_constructor_update_chat_participant_add (D, T); return; 
  case 0x6e5f8c22: store_ds_constructor_update_chat_participant_delete (D, T); return; 
  case 0x8e5e9873: store_ds_constructor_update_dc_options (D, T); return; 
  case 0x80ece81a: store_ds_constructor_update_user_blocked (D, T); return; 
  case 0xbec268ef: store_ds_constructor_update_notify_settings (D, T); return; 
  case 0x382dd3e4: store_ds_constructor_update_service_notification (D, T); return; 
  case 0xee3b272a: store_ds_constructor_update_privacy (D, T); return; 
  case 0x12b9417b: store_ds_constructor_update_user_phone (D, T); return; 
  case 0x9961fd5c: store_ds_constructor_update_read_history_inbox (D, T); return; 
  case 0x2f2f21bf: store_ds_constructor_update_read_history_outbox (D, T); return; 
  case 0x2cc36971: store_ds_constructor_update_web_page (D, T); return; 
  case 0x68c13933: store_ds_constructor_update_read_messages_contents (D, T); return; 
  case 0x03114739: store_ds_constructor_update_msg_update (D, T); return; 
  default: assert (0);
  }
}
void store_ds_type_bare_update (struct tl_ds_update *D, struct paramed_type *T) {
  switch (D->magic) {
  case 0x1f2b0afd: store_ds_constructor_update_new_message (D, T); return; 
  case 0x4e90bfd6: store_ds_constructor_update_message_i_d (D, T); return; 
  case 0xa20db0e5: store_ds_constructor_update_delete_messages (D, T); return; 
  case 0x5c486927: store_ds_constructor_update_user_typing (D, T); return; 
  case 0x9a65ea1f: store_ds_constructor_update_chat_user_typing (D, T); return; 
  case 0x07761198: store_ds_constructor_update_chat_participants (D, T); return; 
  case 0x1bfbd823: store_ds_constructor_update_user_status (D, T); return; 
  case 0xa7332b73: store_ds_constructor_update_user_name (D, T); return; 
  case 0x95313b0c: store_ds_constructor_update_user_photo (D, T); return; 
  case 0x2575bbb9: store_ds_constructor_update_contact_registered (D, T); return; 
  case 0x9d2e67c5: store_ds_constructor_update_contact_link (D, T); return; 
  case 0x8f06529a: store_ds_constructor_update_new_authorization (D, T); return; 
  case 0x5a68e3f7: store_ds_constructor_update_new_geo_chat_message (D, T); return; 
  case 0x12bcbd9a: store_ds_constructor_update_new_encrypted_message (D, T); return; 
  case 0x1710f156: store_ds_constructor_update_encrypted_chat_typing (D, T); return; 
  case 0xb4a2e88d: store_ds_constructor_update_encryption (D, T); return; 
  case 0x38fe25b7: store_ds_constructor_update_encrypted_messages_read (D, T); return; 
  case 0x3a0eeb22: store_ds_constructor_update_chat_participant_add (D, T); return; 
  case 0x6e5f8c22: store_ds_constructor_update_chat_participant_delete (D, T); return; 
  case 0x8e5e9873: store_ds_constructor_update_dc_options (D, T); return; 
  case 0x80ece81a: store_ds_constructor_update_user_blocked (D, T); return; 
  case 0xbec268ef: store_ds_constructor_update_notify_settings (D, T); return; 
  case 0x382dd3e4: store_ds_constructor_update_service_notification (D, T); return; 
  case 0xee3b272a: store_ds_constructor_update_privacy (D, T); return; 
  case 0x12b9417b: store_ds_constructor_update_user_phone (D, T); return; 
  case 0x9961fd5c: store_ds_constructor_update_read_history_inbox (D, T); return; 
  case 0x2f2f21bf: store_ds_constructor_update_read_history_outbox (D, T); return; 
  case 0x2cc36971: store_ds_constructor_update_web_page (D, T); return; 
  case 0x68c13933: store_ds_constructor_update_read_messages_contents (D, T); return; 
  case 0x03114739: store_ds_constructor_update_msg_update (D, T); return; 
  default: assert (0);
  }
}
void store_ds_type_updates (struct tl_ds_updates *D, struct paramed_type *T) {
  out_int (D->magic);
  switch (D->magic) {
  case 0xe317af7e: store_ds_constructor_updates_too_long (D, T); return; 
  case 0xed5c2127: store_ds_constructor_update_short_message (D, T); return; 
  case 0x52238b3c: store_ds_constructor_update_short_chat_message (D, T); return; 
  case 0x78d4dec1: store_ds_constructor_update_short (D, T); return; 
  case 0x725b04c3: store_ds_constructor_updates_combined (D, T); return; 
  case 0x74ae4240: store_ds_constructor_updates (D, T); return; 
  default: assert (0);
  }
}
void store_ds_type_bare_updates (struct tl_ds_updates *D, struct paramed_type *T) {
  switch (D->magic) {
  case 0xe317af7e: store_ds_constructor_updates_too_long (D, T); return; 
  case 0xed5c2127: store_ds_constructor_update_short_message (D, T); return; 
  case 0x52238b3c: store_ds_constructor_update_short_chat_message (D, T); return; 
  case 0x78d4dec1: store_ds_constructor_update_short (D, T); return; 
  case 0x725b04c3: store_ds_constructor_updates_combined (D, T); return; 
  case 0x74ae4240: store_ds_constructor_updates (D, T); return; 
  default: assert (0);
  }
}
void store_ds_type_user (struct tl_ds_user *D, struct paramed_type *T) {
  out_int (D->magic);
  switch (D->magic) {
  case 0x200250ba: store_ds_constructor_user_empty (D, T); return; 
  case 0x22e49072: store_ds_constructor_user (D, T); return; 
  default: assert (0);
  }
}
void store_ds_type_bare_user (struct tl_ds_user *D, struct paramed_type *T) {
  switch (D->magic) {
  case 0x200250ba: store_ds_constructor_user_empty (D, T); return; 
  case 0x22e49072: store_ds_constructor_user (D, T); return; 
  default: assert (0);
  }
}
void store_ds_type_user_full (struct tl_ds_user_full *D, struct paramed_type *T) {
  out_int (0x5a89ac5b);
  store_ds_constructor_user_full (D, T); return; 
}
void store_ds_type_bare_user_full (struct tl_ds_user_full *D, struct paramed_type *T) {
  store_ds_constructor_user_full (D, T); return; 
}
void store_ds_type_user_profile_photo (struct tl_ds_user_profile_photo *D, struct paramed_type *T) {
  out_int (D->magic);
  switch (D->magic) {
  case 0x4f11bae1: store_ds_constructor_user_profile_photo_empty (D, T); return; 
  case 0xd559d8c8: store_ds_constructor_user_profile_photo (D, T); return; 
  default: assert (0);
  }
}
void store_ds_type_bare_user_profile_photo (struct tl_ds_user_profile_photo *D, struct paramed_type *T) {
  switch (D->magic) {
  case 0x4f11bae1: store_ds_constructor_user_profile_photo_empty (D, T); return; 
  case 0xd559d8c8: store_ds_constructor_user_profile_photo (D, T); return; 
  default: assert (0);
  }
}
void store_ds_type_user_status (struct tl_ds_user_status *D, struct paramed_type *T) {
  out_int (D->magic);
  switch (D->magic) {
  case 0x09d05049: store_ds_constructor_user_status_empty (D, T); return; 
  case 0xedb93949: store_ds_constructor_user_status_online (D, T); return; 
  case 0x008c703f: store_ds_constructor_user_status_offline (D, T); return; 
  case 0xe26f42f1: store_ds_constructor_user_status_recently (D, T); return; 
  case 0x07bf09fc: store_ds_constructor_user_status_last_week (D, T); return; 
  case 0x77ebc742: store_ds_constructor_user_status_last_month (D, T); return; 
  default: assert (0);
  }
}
void store_ds_type_bare_user_status (struct tl_ds_user_status *D, struct paramed_type *T) {
  switch (D->magic) {
  case 0x09d05049: store_ds_constructor_user_status_empty (D, T); return; 
  case 0xedb93949: store_ds_constructor_user_status_online (D, T); return; 
  case 0x008c703f: store_ds_constructor_user_status_offline (D, T); return; 
  case 0xe26f42f1: store_ds_constructor_user_status_recently (D, T); return; 
  case 0x07bf09fc: store_ds_constructor_user_status_last_week (D, T); return; 
  case 0x77ebc742: store_ds_constructor_user_status_last_month (D, T); return; 
  default: assert (0);
  }
}
void store_ds_type_vector (struct tl_ds_vector *D, struct paramed_type *T) {
  out_int (0x1cb5c415);
  store_ds_constructor_vector (D, T); return; 
}
void store_ds_type_bare_vector (struct tl_ds_vector *D, struct paramed_type *T) {
  store_ds_constructor_vector (D, T); return; 
}
void store_ds_type_video (struct tl_ds_video *D, struct paramed_type *T) {
  out_int (D->magic);
  switch (D->magic) {
  case 0xc10658a8: store_ds_constructor_video_empty (D, T); return; 
  case 0xee9f4a4d: store_ds_constructor_video (D, T); return; 
  case 0x388fa391: store_ds_constructor_video_l27 (D, T); return; 
  default: assert (0);
  }
}
void store_ds_type_bare_video (struct tl_ds_video *D, struct paramed_type *T) {
  switch (D->magic) {
  case 0xc10658a8: store_ds_constructor_video_empty (D, T); return; 
  case 0xee9f4a4d: store_ds_constructor_video (D, T); return; 
  case 0x388fa391: store_ds_constructor_video_l27 (D, T); return; 
  default: assert (0);
  }
}
void store_ds_type_wall_paper (struct tl_ds_wall_paper *D, struct paramed_type *T) {
  out_int (D->magic);
  switch (D->magic) {
  case 0xccb03657: store_ds_constructor_wall_paper (D, T); return; 
  case 0x63117f24: store_ds_constructor_wall_paper_solid (D, T); return; 
  default: assert (0);
  }
}
void store_ds_type_bare_wall_paper (struct tl_ds_wall_paper *D, struct paramed_type *T) {
  switch (D->magic) {
  case 0xccb03657: store_ds_constructor_wall_paper (D, T); return; 
  case 0x63117f24: store_ds_constructor_wall_paper_solid (D, T); return; 
  default: assert (0);
  }
}
void store_ds_type_web_page (struct tl_ds_web_page *D, struct paramed_type *T) {
  out_int (D->magic);
  switch (D->magic) {
  case 0xeb1477e8: store_ds_constructor_web_page_empty (D, T); return; 
  case 0xc586da1c: store_ds_constructor_web_page_pending (D, T); return; 
  case 0xa31ea0b5: store_ds_constructor_web_page (D, T); return; 
  default: assert (0);
  }
}
void store_ds_type_bare_web_page (struct tl_ds_web_page *D, struct paramed_type *T) {
  switch (D->magic) {
  case 0xeb1477e8: store_ds_constructor_web_page_empty (D, T); return; 
  case 0xc586da1c: store_ds_constructor_web_page_pending (D, T); return; 
  case 0xa31ea0b5: store_ds_constructor_web_page (D, T); return; 
  default: assert (0);
  }
}
void store_ds_type_account_authorizations (struct tl_ds_account_authorizations *D, struct paramed_type *T) {
  out_int (0x1250abde);
  store_ds_constructor_account_authorizations (D, T); return; 
}
void store_ds_type_bare_account_authorizations (struct tl_ds_account_authorizations *D, struct paramed_type *T) {
  store_ds_constructor_account_authorizations (D, T); return; 
}
void store_ds_type_account_password (struct tl_ds_account_password *D, struct paramed_type *T) {
  out_int (D->magic);
  switch (D->magic) {
  case 0x96dabc18: store_ds_constructor_account_no_password (D, T); return; 
  case 0x7c18141c: store_ds_constructor_account_password (D, T); return; 
  default: assert (0);
  }
}
void store_ds_type_bare_account_password (struct tl_ds_account_password *D, struct paramed_type *T) {
  switch (D->magic) {
  case 0x96dabc18: store_ds_constructor_account_no_password (D, T); return; 
  case 0x7c18141c: store_ds_constructor_account_password (D, T); return; 
  default: assert (0);
  }
}
void store_ds_type_account_password_input_settings (struct tl_ds_account_password_input_settings *D, struct paramed_type *T) {
  out_int (0xbcfc532c);
  store_ds_constructor_account_password_input_settings (D, T); return; 
}
void store_ds_type_bare_account_password_input_settings (struct tl_ds_account_password_input_settings *D, struct paramed_type *T) {
  store_ds_constructor_account_password_input_settings (D, T); return; 
}
void store_ds_type_account_password_settings (struct tl_ds_account_password_settings *D, struct paramed_type *T) {
  out_int (0xb7b72ab3);
  store_ds_constructor_account_password_settings (D, T); return; 
}
void store_ds_type_bare_account_password_settings (struct tl_ds_account_password_settings *D, struct paramed_type *T) {
  store_ds_constructor_account_password_settings (D, T); return; 
}
void store_ds_type_account_privacy_rules (struct tl_ds_account_privacy_rules *D, struct paramed_type *T) {
  out_int (0x554abb6f);
  store_ds_constructor_account_privacy_rules (D, T); return; 
}
void store_ds_type_bare_account_privacy_rules (struct tl_ds_account_privacy_rules *D, struct paramed_type *T) {
  store_ds_constructor_account_privacy_rules (D, T); return; 
}
void store_ds_type_account_sent_change_phone_code (struct tl_ds_account_sent_change_phone_code *D, struct paramed_type *T) {
  out_int (0xa4f58c4c);
  store_ds_constructor_account_sent_change_phone_code (D, T); return; 
}
void store_ds_type_bare_account_sent_change_phone_code (struct tl_ds_account_sent_change_phone_code *D, struct paramed_type *T) {
  store_ds_constructor_account_sent_change_phone_code (D, T); return; 
}
void store_ds_type_auth_authorization (struct tl_ds_auth_authorization *D, struct paramed_type *T) {
  out_int (0xff036af1);
  store_ds_constructor_auth_authorization (D, T); return; 
}
void store_ds_type_bare_auth_authorization (struct tl_ds_auth_authorization *D, struct paramed_type *T) {
  store_ds_constructor_auth_authorization (D, T); return; 
}
void store_ds_type_auth_checked_phone (struct tl_ds_auth_checked_phone *D, struct paramed_type *T) {
  out_int (0x811ea28e);
  store_ds_constructor_auth_checked_phone (D, T); return; 
}
void store_ds_type_bare_auth_checked_phone (struct tl_ds_auth_checked_phone *D, struct paramed_type *T) {
  store_ds_constructor_auth_checked_phone (D, T); return; 
}
void store_ds_type_auth_exported_authorization (struct tl_ds_auth_exported_authorization *D, struct paramed_type *T) {
  out_int (0xdf969c2d);
  store_ds_constructor_auth_exported_authorization (D, T); return; 
}
void store_ds_type_bare_auth_exported_authorization (struct tl_ds_auth_exported_authorization *D, struct paramed_type *T) {
  store_ds_constructor_auth_exported_authorization (D, T); return; 
}
void store_ds_type_auth_password_recovery (struct tl_ds_auth_password_recovery *D, struct paramed_type *T) {
  out_int (0x137948a5);
  store_ds_constructor_auth_password_recovery (D, T); return; 
}
void store_ds_type_bare_auth_password_recovery (struct tl_ds_auth_password_recovery *D, struct paramed_type *T) {
  store_ds_constructor_auth_password_recovery (D, T); return; 
}
void store_ds_type_auth_sent_code (struct tl_ds_auth_sent_code *D, struct paramed_type *T) {
  out_int (D->magic);
  switch (D->magic) {
  case 0xefed51d9: store_ds_constructor_auth_sent_code (D, T); return; 
  case 0xe325edcf: store_ds_constructor_auth_sent_app_code (D, T); return; 
  default: assert (0);
  }
}
void store_ds_type_bare_auth_sent_code (struct tl_ds_auth_sent_code *D, struct paramed_type *T) {
  switch (D->magic) {
  case 0xefed51d9: store_ds_constructor_auth_sent_code (D, T); return; 
  case 0xe325edcf: store_ds_constructor_auth_sent_app_code (D, T); return; 
  default: assert (0);
  }
}
void store_ds_type_binlog_encr_key (struct tl_ds_binlog_encr_key *D, struct paramed_type *T) {
  out_int (0x0377168f);
  store_ds_constructor_binlog_encr_key (D, T); return; 
}
void store_ds_type_bare_binlog_encr_key (struct tl_ds_binlog_encr_key *D, struct paramed_type *T) {
  store_ds_constructor_binlog_encr_key (D, T); return; 
}
void store_ds_type_binlog_update (struct tl_ds_binlog_update *D, struct paramed_type *T) {
  out_int (D->magic);
  switch (D->magic) {
  case 0x3b06de69: store_ds_constructor_binlog_start (D, T); return; 
  case 0xf96feb32: store_ds_constructor_binlog_dc_option (D, T); return; 
  case 0x7c0d22d8: store_ds_constructor_binlog_dc_option_new (D, T); return; 
  case 0x71e8c156: store_ds_constructor_binlog_auth_key (D, T); return; 
  case 0x9e83dbdc: store_ds_constructor_binlog_default_dc (D, T); return; 
  case 0x26451bb5: store_ds_constructor_binlog_dc_signed (D, T); return; 
  case 0x68a870e8: store_ds_constructor_binlog_our_id (D, T); return; 
  case 0xeaeb7826: store_ds_constructor_binlog_set_dh_params (D, T); return; 
  case 0x2ca8c939: store_ds_constructor_binlog_set_pts (D, T); return; 
  case 0xd95738ac: store_ds_constructor_binlog_set_qts (D, T); return; 
  case 0x1d0f4b52: store_ds_constructor_binlog_set_date (D, T); return; 
  case 0x6eeb2989: store_ds_constructor_binlog_set_seq (D, T); return; 
  case 0xee1b38e8: store_ds_constructor_binlog_encr_chat_delete (D, T); return; 
  case 0x84977251: store_ds_constructor_binlog_encr_chat_new (D, T); return; 
  case 0x9d49488d: store_ds_constructor_binlog_encr_chat_exchange_new (D, T); return; 
  case 0xac55d447: store_ds_constructor_binlog_user_delete (D, T); return; 
  case 0x127cf2f9: store_ds_constructor_binlog_user_new (D, T); return; 
  case 0x0a10aa92: store_ds_constructor_binlog_chat_new (D, T); return; 
  case 0x535475ea: store_ds_constructor_binlog_chat_add_participant (D, T); return; 
  case 0x7dd1a1a2: store_ds_constructor_binlog_chat_del_participant (D, T); return; 
  case 0x3c873416: store_ds_constructor_binlog_set_msg_id (D, T); return; 
  case 0x847e77b1: store_ds_constructor_binlog_message_delete (D, T); return; 
  case 0x427cfcdb: store_ds_constructor_binlog_message_new (D, T); return; 
  case 0x6cf7cabc: store_ds_constructor_binlog_message_encr_new (D, T); return; 
  case 0x6dd4d85f: store_ds_constructor_binlog_msg_update (D, T); return; 
  case 0x83327955: store_ds_constructor_binlog_reset_authorization (D, T); return; 
  default: assert (0);
  }
}
void store_ds_type_bare_binlog_update (struct tl_ds_binlog_update *D, struct paramed_type *T) {
  switch (D->magic) {
  case 0x3b06de69: store_ds_constructor_binlog_start (D, T); return; 
  case 0xf96feb32: store_ds_constructor_binlog_dc_option (D, T); return; 
  case 0x7c0d22d8: store_ds_constructor_binlog_dc_option_new (D, T); return; 
  case 0x71e8c156: store_ds_constructor_binlog_auth_key (D, T); return; 
  case 0x9e83dbdc: store_ds_constructor_binlog_default_dc (D, T); return; 
  case 0x26451bb5: store_ds_constructor_binlog_dc_signed (D, T); return; 
  case 0x68a870e8: store_ds_constructor_binlog_our_id (D, T); return; 
  case 0xeaeb7826: store_ds_constructor_binlog_set_dh_params (D, T); return; 
  case 0x2ca8c939: store_ds_constructor_binlog_set_pts (D, T); return; 
  case 0xd95738ac: store_ds_constructor_binlog_set_qts (D, T); return; 
  case 0x1d0f4b52: store_ds_constructor_binlog_set_date (D, T); return; 
  case 0x6eeb2989: store_ds_constructor_binlog_set_seq (D, T); return; 
  case 0xee1b38e8: store_ds_constructor_binlog_encr_chat_delete (D, T); return; 
  case 0x84977251: store_ds_constructor_binlog_encr_chat_new (D, T); return; 
  case 0x9d49488d: store_ds_constructor_binlog_encr_chat_exchange_new (D, T); return; 
  case 0xac55d447: store_ds_constructor_binlog_user_delete (D, T); return; 
  case 0x127cf2f9: store_ds_constructor_binlog_user_new (D, T); return; 
  case 0x0a10aa92: store_ds_constructor_binlog_chat_new (D, T); return; 
  case 0x535475ea: store_ds_constructor_binlog_chat_add_participant (D, T); return; 
  case 0x7dd1a1a2: store_ds_constructor_binlog_chat_del_participant (D, T); return; 
  case 0x3c873416: store_ds_constructor_binlog_set_msg_id (D, T); return; 
  case 0x847e77b1: store_ds_constructor_binlog_message_delete (D, T); return; 
  case 0x427cfcdb: store_ds_constructor_binlog_message_new (D, T); return; 
  case 0x6cf7cabc: store_ds_constructor_binlog_message_encr_new (D, T); return; 
  case 0x6dd4d85f: store_ds_constructor_binlog_msg_update (D, T); return; 
  case 0x83327955: store_ds_constructor_binlog_reset_authorization (D, T); return; 
  default: assert (0);
  }
}
void store_ds_type_contacts_blocked (struct tl_ds_contacts_blocked *D, struct paramed_type *T) {
  out_int (D->magic);
  switch (D->magic) {
  case 0x1c138d15: store_ds_constructor_contacts_blocked (D, T); return; 
  case 0x900802a1: store_ds_constructor_contacts_blocked_slice (D, T); return; 
  default: assert (0);
  }
}
void store_ds_type_bare_contacts_blocked (struct tl_ds_contacts_blocked *D, struct paramed_type *T) {
  switch (D->magic) {
  case 0x1c138d15: store_ds_constructor_contacts_blocked (D, T); return; 
  case 0x900802a1: store_ds_constructor_contacts_blocked_slice (D, T); return; 
  default: assert (0);
  }
}
void store_ds_type_contacts_contacts (struct tl_ds_contacts_contacts *D, struct paramed_type *T) {
  out_int (D->magic);
  switch (D->magic) {
  case 0xb74ba9d2: store_ds_constructor_contacts_contacts_not_modified (D, T); return; 
  case 0x6f8b8cb2: store_ds_constructor_contacts_contacts (D, T); return; 
  default: assert (0);
  }
}
void store_ds_type_bare_contacts_contacts (struct tl_ds_contacts_contacts *D, struct paramed_type *T) {
  switch (D->magic) {
  case 0xb74ba9d2: store_ds_constructor_contacts_contacts_not_modified (D, T); return; 
  case 0x6f8b8cb2: store_ds_constructor_contacts_contacts (D, T); return; 
  default: assert (0);
  }
}
void store_ds_type_contacts_found (struct tl_ds_contacts_found *D, struct paramed_type *T) {
  out_int (0x0566000e);
  store_ds_constructor_contacts_found (D, T); return; 
}
void store_ds_type_bare_contacts_found (struct tl_ds_contacts_found *D, struct paramed_type *T) {
  store_ds_constructor_contacts_found (D, T); return; 
}
void store_ds_type_contacts_imported_contacts (struct tl_ds_contacts_imported_contacts *D, struct paramed_type *T) {
  out_int (0xad524315);
  store_ds_constructor_contacts_imported_contacts (D, T); return; 
}
void store_ds_type_bare_contacts_imported_contacts (struct tl_ds_contacts_imported_contacts *D, struct paramed_type *T) {
  store_ds_constructor_contacts_imported_contacts (D, T); return; 
}
void store_ds_type_contacts_link (struct tl_ds_contacts_link *D, struct paramed_type *T) {
  out_int (0x3ace484c);
  store_ds_constructor_contacts_link (D, T); return; 
}
void store_ds_type_bare_contacts_link (struct tl_ds_contacts_link *D, struct paramed_type *T) {
  store_ds_constructor_contacts_link (D, T); return; 
}
void store_ds_type_contacts_suggested (struct tl_ds_contacts_suggested *D, struct paramed_type *T) {
  out_int (0x5649dcc5);
  store_ds_constructor_contacts_suggested (D, T); return; 
}
void store_ds_type_bare_contacts_suggested (struct tl_ds_contacts_suggested *D, struct paramed_type *T) {
  store_ds_constructor_contacts_suggested (D, T); return; 
}
void store_ds_type_geochats_located (struct tl_ds_geochats_located *D, struct paramed_type *T) {
  out_int (0x48feb267);
  store_ds_constructor_geochats_located (D, T); return; 
}
void store_ds_type_bare_geochats_located (struct tl_ds_geochats_located *D, struct paramed_type *T) {
  store_ds_constructor_geochats_located (D, T); return; 
}
void store_ds_type_geochats_messages (struct tl_ds_geochats_messages *D, struct paramed_type *T) {
  out_int (D->magic);
  switch (D->magic) {
  case 0xd1526db1: store_ds_constructor_geochats_messages (D, T); return; 
  case 0xbc5863e8: store_ds_constructor_geochats_messages_slice (D, T); return; 
  default: assert (0);
  }
}
void store_ds_type_bare_geochats_messages (struct tl_ds_geochats_messages *D, struct paramed_type *T) {
  switch (D->magic) {
  case 0xd1526db1: store_ds_constructor_geochats_messages (D, T); return; 
  case 0xbc5863e8: store_ds_constructor_geochats_messages_slice (D, T); return; 
  default: assert (0);
  }
}
void store_ds_type_geochats_stated_message (struct tl_ds_geochats_stated_message *D, struct paramed_type *T) {
  out_int (0x17b1578b);
  store_ds_constructor_geochats_stated_message (D, T); return; 
}
void store_ds_type_bare_geochats_stated_message (struct tl_ds_geochats_stated_message *D, struct paramed_type *T) {
  store_ds_constructor_geochats_stated_message (D, T); return; 
}
void store_ds_type_help_app_update (struct tl_ds_help_app_update *D, struct paramed_type *T) {
  out_int (D->magic);
  switch (D->magic) {
  case 0x8987f311: store_ds_constructor_help_app_update (D, T); return; 
  case 0xc45a6536: store_ds_constructor_help_no_app_update (D, T); return; 
  default: assert (0);
  }
}
void store_ds_type_bare_help_app_update (struct tl_ds_help_app_update *D, struct paramed_type *T) {
  switch (D->magic) {
  case 0x8987f311: store_ds_constructor_help_app_update (D, T); return; 
  case 0xc45a6536: store_ds_constructor_help_no_app_update (D, T); return; 
  default: assert (0);
  }
}
void store_ds_type_help_invite_text (struct tl_ds_help_invite_text *D, struct paramed_type *T) {
  out_int (0x18cb9f78);
  store_ds_constructor_help_invite_text (D, T); return; 
}
void store_ds_type_bare_help_invite_text (struct tl_ds_help_invite_text *D, struct paramed_type *T) {
  store_ds_constructor_help_invite_text (D, T); return; 
}
void store_ds_type_help_support (struct tl_ds_help_support *D, struct paramed_type *T) {
  out_int (0x17c6b5f6);
  store_ds_constructor_help_support (D, T); return; 
}
void store_ds_type_bare_help_support (struct tl_ds_help_support *D, struct paramed_type *T) {
  store_ds_constructor_help_support (D, T); return; 
}
void store_ds_type_messages_affected_history (struct tl_ds_messages_affected_history *D, struct paramed_type *T) {
  out_int (0xb45c69d1);
  store_ds_constructor_messages_affected_history (D, T); return; 
}
void store_ds_type_bare_messages_affected_history (struct tl_ds_messages_affected_history *D, struct paramed_type *T) {
  store_ds_constructor_messages_affected_history (D, T); return; 
}
void store_ds_type_messages_affected_messages (struct tl_ds_messages_affected_messages *D, struct paramed_type *T) {
  out_int (0x84d19185);
  store_ds_constructor_messages_affected_messages (D, T); return; 
}
void store_ds_type_bare_messages_affected_messages (struct tl_ds_messages_affected_messages *D, struct paramed_type *T) {
  store_ds_constructor_messages_affected_messages (D, T); return; 
}
void store_ds_type_messages_all_stickers (struct tl_ds_messages_all_stickers *D, struct paramed_type *T) {
  out_int (D->magic);
  switch (D->magic) {
  case 0xe86602c3: store_ds_constructor_messages_all_stickers_not_modified (D, T); return; 
  case 0x5ce352ec: store_ds_constructor_messages_all_stickers (D, T); return; 
  default: assert (0);
  }
}
void store_ds_type_bare_messages_all_stickers (struct tl_ds_messages_all_stickers *D, struct paramed_type *T) {
  switch (D->magic) {
  case 0xe86602c3: store_ds_constructor_messages_all_stickers_not_modified (D, T); return; 
  case 0x5ce352ec: store_ds_constructor_messages_all_stickers (D, T); return; 
  default: assert (0);
  }
}
void store_ds_type_messages_chat_full (struct tl_ds_messages_chat_full *D, struct paramed_type *T) {
  out_int (0xe5d7d19c);
  store_ds_constructor_messages_chat_full (D, T); return; 
}
void store_ds_type_bare_messages_chat_full (struct tl_ds_messages_chat_full *D, struct paramed_type *T) {
  store_ds_constructor_messages_chat_full (D, T); return; 
}
void store_ds_type_messages_chats (struct tl_ds_messages_chats *D, struct paramed_type *T) {
  out_int (0x64ff9fd5);
  store_ds_constructor_messages_chats (D, T); return; 
}
void store_ds_type_bare_messages_chats (struct tl_ds_messages_chats *D, struct paramed_type *T) {
  store_ds_constructor_messages_chats (D, T); return; 
}
void store_ds_type_messages_dh_config (struct tl_ds_messages_dh_config *D, struct paramed_type *T) {
  out_int (D->magic);
  switch (D->magic) {
  case 0xc0e24635: store_ds_constructor_messages_dh_config_not_modified (D, T); return; 
  case 0x2c221edd: store_ds_constructor_messages_dh_config (D, T); return; 
  default: assert (0);
  }
}
void store_ds_type_bare_messages_dh_config (struct tl_ds_messages_dh_config *D, struct paramed_type *T) {
  switch (D->magic) {
  case 0xc0e24635: store_ds_constructor_messages_dh_config_not_modified (D, T); return; 
  case 0x2c221edd: store_ds_constructor_messages_dh_config (D, T); return; 
  default: assert (0);
  }
}
void store_ds_type_messages_dialogs (struct tl_ds_messages_dialogs *D, struct paramed_type *T) {
  out_int (D->magic);
  switch (D->magic) {
  case 0x15ba6c40: store_ds_constructor_messages_dialogs (D, T); return; 
  case 0x71e094f3: store_ds_constructor_messages_dialogs_slice (D, T); return; 
  default: assert (0);
  }
}
void store_ds_type_bare_messages_dialogs (struct tl_ds_messages_dialogs *D, struct paramed_type *T) {
  switch (D->magic) {
  case 0x15ba6c40: store_ds_constructor_messages_dialogs (D, T); return; 
  case 0x71e094f3: store_ds_constructor_messages_dialogs_slice (D, T); return; 
  default: assert (0);
  }
}
void store_ds_type_messages_message (struct tl_ds_messages_message *D, struct paramed_type *T) {
  out_int (0x3f4e0648);
  store_ds_constructor_messages_message_empty (D, T); return; 
}
void store_ds_type_bare_messages_message (struct tl_ds_messages_message *D, struct paramed_type *T) {
  store_ds_constructor_messages_message_empty (D, T); return; 
}
void store_ds_type_messages_messages (struct tl_ds_messages_messages *D, struct paramed_type *T) {
  out_int (D->magic);
  switch (D->magic) {
  case 0x8c718e87: store_ds_constructor_messages_messages (D, T); return; 
  case 0x0b446ae3: store_ds_constructor_messages_messages_slice (D, T); return; 
  default: assert (0);
  }
}
void store_ds_type_bare_messages_messages (struct tl_ds_messages_messages *D, struct paramed_type *T) {
  switch (D->magic) {
  case 0x8c718e87: store_ds_constructor_messages_messages (D, T); return; 
  case 0x0b446ae3: store_ds_constructor_messages_messages_slice (D, T); return; 
  default: assert (0);
  }
}
void store_ds_type_messages_sent_encrypted_message (struct tl_ds_messages_sent_encrypted_message *D, struct paramed_type *T) {
  out_int (D->magic);
  switch (D->magic) {
  case 0x560f8935: store_ds_constructor_messages_sent_encrypted_message (D, T); return; 
  case 0x9493ff32: store_ds_constructor_messages_sent_encrypted_file (D, T); return; 
  default: assert (0);
  }
}
void store_ds_type_bare_messages_sent_encrypted_message (struct tl_ds_messages_sent_encrypted_message *D, struct paramed_type *T) {
  switch (D->magic) {
  case 0x560f8935: store_ds_constructor_messages_sent_encrypted_message (D, T); return; 
  case 0x9493ff32: store_ds_constructor_messages_sent_encrypted_file (D, T); return; 
  default: assert (0);
  }
}
void store_ds_type_messages_sent_message (struct tl_ds_messages_sent_message *D, struct paramed_type *T) {
  out_int (D->magic);
  switch (D->magic) {
  case 0x4c3d47f3: store_ds_constructor_messages_sent_message (D, T); return; 
  case 0x35a1a663: store_ds_constructor_messages_sent_message_link (D, T); return; 
  default: assert (0);
  }
}
void store_ds_type_bare_messages_sent_message (struct tl_ds_messages_sent_message *D, struct paramed_type *T) {
  switch (D->magic) {
  case 0x4c3d47f3: store_ds_constructor_messages_sent_message (D, T); return; 
  case 0x35a1a663: store_ds_constructor_messages_sent_message_link (D, T); return; 
  default: assert (0);
  }
}
void store_ds_type_messages_sticker_set (struct tl_ds_messages_sticker_set *D, struct paramed_type *T) {
  out_int (0xb60a24a6);
  store_ds_constructor_messages_sticker_set (D, T); return; 
}
void store_ds_type_bare_messages_sticker_set (struct tl_ds_messages_sticker_set *D, struct paramed_type *T) {
  store_ds_constructor_messages_sticker_set (D, T); return; 
}
void store_ds_type_messages_stickers (struct tl_ds_messages_stickers *D, struct paramed_type *T) {
  out_int (D->magic);
  switch (D->magic) {
  case 0xf1749a22: store_ds_constructor_messages_stickers_not_modified (D, T); return; 
  case 0x8a8ecd32: store_ds_constructor_messages_stickers (D, T); return; 
  default: assert (0);
  }
}
void store_ds_type_bare_messages_stickers (struct tl_ds_messages_stickers *D, struct paramed_type *T) {
  switch (D->magic) {
  case 0xf1749a22: store_ds_constructor_messages_stickers_not_modified (D, T); return; 
  case 0x8a8ecd32: store_ds_constructor_messages_stickers (D, T); return; 
  default: assert (0);
  }
}
void store_ds_type_photos_photo (struct tl_ds_photos_photo *D, struct paramed_type *T) {
  out_int (0x20212ca8);
  store_ds_constructor_photos_photo (D, T); return; 
}
void store_ds_type_bare_photos_photo (struct tl_ds_photos_photo *D, struct paramed_type *T) {
  store_ds_constructor_photos_photo (D, T); return; 
}
void store_ds_type_photos_photos (struct tl_ds_photos_photos *D, struct paramed_type *T) {
  out_int (D->magic);
  switch (D->magic) {
  case 0x8dca6aa5: store_ds_constructor_photos_photos (D, T); return; 
  case 0x15051f54: store_ds_constructor_photos_photos_slice (D, T); return; 
  default: assert (0);
  }
}
void store_ds_type_bare_photos_photos (struct tl_ds_photos_photos *D, struct paramed_type *T) {
  switch (D->magic) {
  case 0x8dca6aa5: store_ds_constructor_photos_photos (D, T); return; 
  case 0x15051f54: store_ds_constructor_photos_photos_slice (D, T); return; 
  default: assert (0);
  }
}
void store_ds_type_storage_file_type (struct tl_ds_storage_file_type *D, struct paramed_type *T) {
  out_int (D->magic);
  switch (D->magic) {
  case 0xaa963b05: store_ds_constructor_storage_file_unknown (D, T); return; 
  case 0x007efe0e: store_ds_constructor_storage_file_jpeg (D, T); return; 
  case 0xcae1aadf: store_ds_constructor_storage_file_gif (D, T); return; 
  case 0x0a4f63c0: store_ds_constructor_storage_file_png (D, T); return; 
  case 0xae1e508d: store_ds_constructor_storage_file_pdf (D, T); return; 
  case 0x528a0677: store_ds_constructor_storage_file_mp3 (D, T); return; 
  case 0x4b09ebbc: store_ds_constructor_storage_file_mov (D, T); return; 
  case 0x40bc6f52: store_ds_constructor_storage_file_partial (D, T); return; 
  case 0xb3cea0e4: store_ds_constructor_storage_file_mp4 (D, T); return; 
  case 0x1081464c: store_ds_constructor_storage_file_webp (D, T); return; 
  default: assert (0);
  }
}
void store_ds_type_bare_storage_file_type (struct tl_ds_storage_file_type *D, struct paramed_type *T) {
  switch (D->magic) {
  case 0xaa963b05: store_ds_constructor_storage_file_unknown (D, T); return; 
  case 0x007efe0e: store_ds_constructor_storage_file_jpeg (D, T); return; 
  case 0xcae1aadf: store_ds_constructor_storage_file_gif (D, T); return; 
  case 0x0a4f63c0: store_ds_constructor_storage_file_png (D, T); return; 
  case 0xae1e508d: store_ds_constructor_storage_file_pdf (D, T); return; 
  case 0x528a0677: store_ds_constructor_storage_file_mp3 (D, T); return; 
  case 0x4b09ebbc: store_ds_constructor_storage_file_mov (D, T); return; 
  case 0x40bc6f52: store_ds_constructor_storage_file_partial (D, T); return; 
  case 0xb3cea0e4: store_ds_constructor_storage_file_mp4 (D, T); return; 
  case 0x1081464c: store_ds_constructor_storage_file_webp (D, T); return; 
  default: assert (0);
  }
}
void store_ds_type_updates_difference (struct tl_ds_updates_difference *D, struct paramed_type *T) {
  out_int (D->magic);
  switch (D->magic) {
  case 0x5d75a138: store_ds_constructor_updates_difference_empty (D, T); return; 
  case 0x00f49ca0: store_ds_constructor_updates_difference (D, T); return; 
  case 0xa8fb1981: store_ds_constructor_updates_difference_slice (D, T); return; 
  default: assert (0);
  }
}
void store_ds_type_bare_updates_difference (struct tl_ds_updates_difference *D, struct paramed_type *T) {
  switch (D->magic) {
  case 0x5d75a138: store_ds_constructor_updates_difference_empty (D, T); return; 
  case 0x00f49ca0: store_ds_constructor_updates_difference (D, T); return; 
  case 0xa8fb1981: store_ds_constructor_updates_difference_slice (D, T); return; 
  default: assert (0);
  }
}
void store_ds_type_updates_state (struct tl_ds_updates_state *D, struct paramed_type *T) {
  out_int (0xa56c2a3e);
  store_ds_constructor_updates_state (D, T); return; 
}
void store_ds_type_bare_updates_state (struct tl_ds_updates_state *D, struct paramed_type *T) {
  store_ds_constructor_updates_state (D, T); return; 
}
void store_ds_type_upload_file (struct tl_ds_upload_file *D, struct paramed_type *T) {
  out_int (0x096a18d5);
  store_ds_constructor_upload_file (D, T); return; 
}
void store_ds_type_bare_upload_file (struct tl_ds_upload_file *D, struct paramed_type *T) {
  store_ds_constructor_upload_file (D, T); return; 
}
void store_ds_type_any (void *D, struct paramed_type *T) {
  switch (T->type->name) {
  case 0xb8d0afdf: store_ds_type_account_days_t_t_l (D, T); return;
  case 0x472f5020: store_ds_type_bare_account_days_t_t_l (D, T); return;
  case 0x9fc5ec4e: store_ds_type_audio (D, T); return;
  case 0x603a13b1: store_ds_type_bare_audio (D, T); return;
  case 0x7bf2e6f6: store_ds_type_authorization (D, T); return;
  case 0x840d1909: store_ds_type_bare_authorization (D, T); return;
  case 0x250be282: store_ds_type_bool (D, T); return;
  case 0xdaf41d7d: store_ds_type_bare_bool (D, T); return;
  case 0x75e7ea6c: store_ds_type_bot_command (D, T); return;
  case 0x8a181593: store_ds_type_bare_bot_command (D, T); return;
  case 0xb2e16f93: store_ds_type_bot_info (D, T); return;
  case 0x4d1e906c: store_ds_type_bare_bot_info (D, T); return;
  case 0x0ee1379f: store_ds_type_bytes (D, T); return;
  case 0xf11ec860: store_ds_type_bare_bytes (D, T); return;
  case 0x7bd865dc: store_ds_type_chat (D, T); return;
  case 0x84279a23: store_ds_type_bare_chat (D, T); return;
  case 0x2e02a614: store_ds_type_chat_full (D, T); return;
  case 0xd1fd59eb: store_ds_type_bare_chat_full (D, T); return;
  case 0x94f910b1: store_ds_type_chat_invite (D, T); return;
  case 0x6b06ef4e: store_ds_type_bare_chat_invite (D, T); return;
  case 0x3631cf4c: store_ds_type_chat_located (D, T); return;
  case 0xc9ce30b3: store_ds_type_bare_chat_located (D, T); return;
  case 0xc8d7493e: store_ds_type_chat_participant (D, T); return;
  case 0x3728b6c1: store_ds_type_bare_chat_participant (D, T); return;
  case 0x77930f9f: store_ds_type_chat_participants (D, T); return;
  case 0x886cf060: store_ds_type_bare_chat_participants (D, T); return;
  case 0x56922676: store_ds_type_chat_photo (D, T); return;
  case 0xa96dd989: store_ds_type_bare_chat_photo (D, T); return;
  case 0x6643b654: store_ds_type_client_d_h_inner_data (D, T); return;
  case 0x99bc49ab: store_ds_type_bare_client_d_h_inner_data (D, T); return;
  case 0x4e32b894: store_ds_type_config (D, T); return;
  case 0xb1cd476b: store_ds_type_bare_config (D, T); return;
  case 0xf911c994: store_ds_type_contact (D, T); return;
  case 0x06ee366b: store_ds_type_bare_contact (D, T); return;
  case 0x561bc879: store_ds_type_contact_blocked (D, T); return;
  case 0xa9e43786: store_ds_type_bare_contact_blocked (D, T); return;
  case 0xea879f95: store_ds_type_contact_found (D, T); return;
  case 0x1578606a: store_ds_type_bare_contact_found (D, T); return;
  case 0x522fbc63: store_ds_type_contact_link (D, T); return;
  case 0xadd0439c: store_ds_type_bare_contact_link (D, T); return;
  case 0xd3680c61: store_ds_type_contact_status (D, T); return;
  case 0x2c97f39e: store_ds_type_bare_contact_status (D, T); return;
  case 0x3de191a1: store_ds_type_contact_suggested (D, T); return;
  case 0xc21e6e5e: store_ds_type_bare_contact_suggested (D, T); return;
  case 0x2b1a62f0: store_ds_type_dc_option (D, T); return;
  case 0xd4e59d0f: store_ds_type_bare_dc_option (D, T); return;
  case 0x535b7918: store_ds_type_decrypted_message (D, T); return;
  case 0xaca486e7: store_ds_type_bare_decrypted_message (D, T); return;
  case 0x4e0eefde: store_ds_type_decrypted_message_action (D, T); return;
  case 0xb1f11021: store_ds_type_bare_decrypted_message_action (D, T); return;
  case 0x1be31789: store_ds_type_decrypted_message_layer (D, T); return;
  case 0xe41ce876: store_ds_type_bare_decrypted_message_layer (D, T); return;
  case 0x34e0d674: store_ds_type_decrypted_message_media (D, T); return;
  case 0xcb1f298b: store_ds_type_bare_decrypted_message_media (D, T); return;
  case 0xc1dd804a: store_ds_type_dialog (D, T); return;
  case 0x3e227fb5: store_ds_type_bare_dialog (D, T); return;
  case 0xae636f24: store_ds_type_disabled_feature (D, T); return;
  case 0x519c90db: store_ds_type_bare_disabled_feature (D, T); return;
  case 0x51a73418: store_ds_type_document (D, T); return;
  case 0xae58cbe7: store_ds_type_bare_document (D, T); return;
  case 0x97c637a3: store_ds_type_document_attribute (D, T); return;
  case 0x6839c85c: store_ds_type_bare_document_attribute (D, T); return;
  case 0x2210c154: store_ds_type_double (D, T); return;
  case 0xddef3eab: store_ds_type_bare_double (D, T); return;
  case 0xb1718213: store_ds_type_encrypted_chat (D, T); return;
  case 0x4e8e7dec: store_ds_type_bare_encrypted_chat (D, T); return;
  case 0x886fd032: store_ds_type_encrypted_file (D, T); return;
  case 0x77902fcd: store_ds_type_bare_encrypted_file (D, T); return;
  case 0xce6b8a1e: store_ds_type_encrypted_message (D, T); return;
  case 0x319475e1: store_ds_type_bare_encrypted_message (D, T); return;
  case 0xc4b9f9bb: store_ds_type_error (D, T); return;
  case 0x3b460644: store_ds_type_bare_error (D, T); return;
  case 0x95f132d5: store_ds_type_exported_chat_invite (D, T); return;
  case 0x6a0ecd2a: store_ds_type_bare_exported_chat_invite (D, T); return;
  case 0x2f8ffb30: store_ds_type_file_location (D, T); return;
  case 0xd07004cf: store_ds_type_bare_file_location (D, T); return;
  case 0xf67b4034: store_ds_type_geo_chat_message (D, T); return;
  case 0x0984bfcb: store_ds_type_bare_geo_chat_message (D, T); return;
  case 0x315e0a53: store_ds_type_geo_point (D, T); return;
  case 0xcea1f5ac: store_ds_type_bare_geo_point (D, T); return;
  case 0xd0028438: store_ds_type_imported_contact (D, T); return;
  case 0x2ffd7bc7: store_ds_type_bare_imported_contact (D, T); return;
  case 0x770656a8: store_ds_type_input_app_event (D, T); return;
  case 0x88f9a957: store_ds_type_bare_input_app_event (D, T); return;
  case 0xae8e9c7b: store_ds_type_input_audio (D, T); return;
  case 0x51716384: store_ds_type_bare_input_audio (D, T); return;
  case 0x3a60776d: store_ds_type_input_chat_photo (D, T); return;
  case 0xc59f8892: store_ds_type_bare_input_chat_photo (D, T); return;
  case 0xf392b7f4: store_ds_type_input_contact (D, T); return;
  case 0x0c6d480b: store_ds_type_bare_input_contact (D, T); return;
  case 0x6a8963fc: store_ds_type_input_document (D, T); return;
  case 0x95769c03: store_ds_type_bare_input_document (D, T); return;
  case 0xf141b5e1: store_ds_type_input_encrypted_chat (D, T); return;
  case 0x0ebe4a1e: store_ds_type_bare_input_encrypted_chat (D, T); return;
  case 0x0b5c064f: store_ds_type_input_encrypted_file (D, T); return;
  case 0xf4a3f9b0: store_ds_type_bare_input_encrypted_file (D, T); return;
  case 0x0f60f9ca: store_ds_type_input_file (D, T); return;
  case 0xf09f0635: store_ds_type_bare_input_file (D, T); return;
  case 0xe6daa38b: store_ds_type_input_file_location (D, T); return;
  case 0x19255c74: store_ds_type_bare_input_file_location (D, T); return;
  case 0x74d456fa: store_ds_type_input_geo_chat (D, T); return;
  case 0x8b2ba905: store_ds_type_bare_input_geo_chat (D, T); return;
  case 0x17768f1f: store_ds_type_input_geo_point (D, T); return;
  case 0xe88970e0: store_ds_type_bare_input_geo_point (D, T); return;
  case 0xb32c91dd: store_ds_type_input_media (D, T); return;
  case 0x4cd36e22: store_ds_type_bare_input_media (D, T); return;
  case 0x02b6911b: store_ds_type_input_notify_peer (D, T); return;
  case 0xfd496ee4: store_ds_type_bare_input_notify_peer (D, T); return;
  case 0x9e67268d: store_ds_type_input_peer (D, T); return;
  case 0x6198d972: store_ds_type_bare_input_peer (D, T); return;
  case 0x185a48ac: store_ds_type_input_peer_notify_events (D, T); return;
  case 0xe7a5b753: store_ds_type_bare_input_peer_notify_events (D, T); return;
  case 0x46a2ce98: store_ds_type_input_peer_notify_settings (D, T); return;
  case 0xb95d3167: store_ds_type_bare_input_peer_notify_settings (D, T); return;
  case 0xe74279c9: store_ds_type_input_photo (D, T); return;
  case 0x18bd8636: store_ds_type_bare_input_photo (D, T); return;
  case 0x7477e321: store_ds_type_input_photo_crop (D, T); return;
  case 0x8b881cde: store_ds_type_bare_input_photo_crop (D, T); return;
  case 0x4f96cb18: store_ds_type_input_privacy_key (D, T); return;
  case 0xb06934e7: store_ds_type_bare_input_privacy_key (D, T); return;
  case 0x4b815163: store_ds_type_input_privacy_rule (D, T); return;
  case 0xb47eae9c: store_ds_type_bare_input_privacy_rule (D, T); return;
  case 0xe44d415c: store_ds_type_input_sticker_set (D, T); return;
  case 0x1bb2bea3: store_ds_type_bare_input_sticker_set (D, T); return;
  case 0xadfe0c6a: store_ds_type_input_user (D, T); return;
  case 0x5201f395: store_ds_type_bare_input_user (D, T); return;
  case 0xbb5f7a27: store_ds_type_input_video (D, T); return;
  case 0x44a085d8: store_ds_type_bare_input_video (D, T); return;
  case 0xa8509bda: store_ds_type_int (D, T); return;
  case 0x57af6425: store_ds_type_bare_int (D, T); return;
  case 0x7d36c439: store_ds_type_int128 (D, T); return;
  case 0x82c93bc6: store_ds_type_bare_int128 (D, T); return;
  case 0xf2c798b3: store_ds_type_int256 (D, T); return;
  case 0x0d38674c: store_ds_type_bare_int256 (D, T); return;
  case 0xa2fa4880: store_ds_type_keyboard_button (D, T); return;
  case 0x5d05b77f: store_ds_type_bare_keyboard_button (D, T); return;
  case 0x77608b83: store_ds_type_keyboard_button_row (D, T); return;
  case 0x889f747c: store_ds_type_bare_keyboard_button_row (D, T); return;
  case 0x22076cba: store_ds_type_long (D, T); return;
  case 0xddf89345: store_ds_type_bare_long (D, T); return;
  case 0x5d652a7f: store_ds_type_message (D, T); return;
  case 0xa29ad580: store_ds_type_bare_message (D, T); return;
  case 0x383415fa: store_ds_type_message_action (D, T); return;
  case 0xc7cbea05: store_ds_type_bare_message_action (D, T); return;
  case 0x95f2471a: store_ds_type_message_media (D, T); return;
  case 0x6a0db8e5: store_ds_type_bare_message_media (D, T); return;
  case 0x80895ae0: store_ds_type_messages_filter (D, T); return;
  case 0x7f76a51f: store_ds_type_bare_messages_filter (D, T); return;
  case 0x8e1a1775: store_ds_type_nearest_dc (D, T); return;
  case 0x71e5e88a: store_ds_type_bare_nearest_dc (D, T); return;
  case 0x9fcb8237: store_ds_type_notify_peer (D, T); return;
  case 0x60347dc8: store_ds_type_bare_notify_peer (D, T); return;
  case 0x56730bcc: store_ds_type_null (D, T); return;
  case 0xa98cf433: store_ds_type_bare_null (D, T); return;
  case 0xbfa3de38: store_ds_type_p_q_inner_data (D, T); return;
  case 0x405c21c7: store_ds_type_bare_p_q_inner_data (D, T); return;
  case 0x276159d6: store_ds_type_peer (D, T); return;
  case 0xd89ea629: store_ds_type_bare_peer (D, T); return;
  case 0xc0c8d13b: store_ds_type_peer_notify_events (D, T); return;
  case 0x3f372ec4: store_ds_type_bare_peer_notify_events (D, T); return;
  case 0xfdf894fc: store_ds_type_peer_notify_settings (D, T); return;
  case 0x02076b03: store_ds_type_bare_peer_notify_settings (D, T); return;
  case 0xc207550a: store_ds_type_photo (D, T); return;
  case 0x3df8aaf5: store_ds_type_bare_photo (D, T); return;
  case 0x900f60dd: store_ds_type_photo_size (D, T); return;
  case 0x6ff09f22: store_ds_type_bare_photo_size (D, T); return;
  case 0xbc2eab30: store_ds_type_privacy_key (D, T); return;
  case 0x43d154cf: store_ds_type_bare_privacy_key (D, T); return;
  case 0xa8638aec: store_ds_type_privacy_rule (D, T); return;
  case 0x579c7513: store_ds_type_bare_privacy_rule (D, T); return;
  case 0xa384b779: store_ds_type_received_notify_message (D, T); return;
  case 0x5c7b4886: store_ds_type_bare_received_notify_message (D, T); return;
  case 0x612ca4a9: store_ds_type_reply_markup (D, T); return;
  case 0x9ed35b56: store_ds_type_bare_reply_markup (D, T); return;
  case 0x05162463: store_ds_type_res_p_q (D, T); return;
  case 0xfae9db9c: store_ds_type_bare_res_p_q (D, T); return;
  case 0x70541c7e: store_ds_type_send_message_action (D, T); return;
  case 0x8fabe381: store_ds_type_bare_send_message_action (D, T); return;
  case 0xa9230301: store_ds_type_server_d_h_params (D, T); return;
  case 0x56dcfcfe: store_ds_type_bare_server_d_h_params (D, T); return;
  case 0xb5890dba: store_ds_type_server_d_h_inner_data (D, T); return;
  case 0x4a76f245: store_ds_type_bare_server_d_h_inner_data (D, T); return;
  case 0xdb8a468f: store_ds_type_set_client_d_h_params_answer (D, T); return;
  case 0x2475b970: store_ds_type_bare_set_client_d_h_params_answer (D, T); return;
  case 0x12b299d4: store_ds_type_sticker_pack (D, T); return;
  case 0xed4d662b: store_ds_type_bare_sticker_pack (D, T); return;
  case 0xa7a43b17: store_ds_type_sticker_set (D, T); return;
  case 0x585bc4e8: store_ds_type_bare_sticker_set (D, T); return;
  case 0xb5286e24: store_ds_type_string (D, T); return;
  case 0x4ad791db: store_ds_type_bare_string (D, T); return;
  case 0x2e02b27e: store_ds_type_update (D, T); return;
  case 0xd1fd4d81: store_ds_type_bare_update (D, T); return;
  case 0x22499d27: store_ds_type_updates (D, T); return;
  case 0xddb662d8: store_ds_type_bare_updates (D, T); return;
  case 0x02e6c0c8: store_ds_type_user (D, T); return;
  case 0xfd193f37: store_ds_type_bare_user (D, T); return;
  case 0x5a89ac5b: store_ds_type_user_full (D, T); return;
  case 0xa57653a4: store_ds_type_bare_user_full (D, T); return;
  case 0x9a486229: store_ds_type_user_profile_photo (D, T); return;
  case 0x65b79dd6: store_ds_type_bare_user_profile_photo (D, T); return;
  case 0x76de9570: store_ds_type_user_status (D, T); return;
  case 0x89216a8f: store_ds_type_bare_user_status (D, T); return;
  case 0x1cb5c415: store_ds_type_vector (D, T); return;
  case 0xe34a3bea: store_ds_type_bare_vector (D, T); return;
  case 0x1716b174: store_ds_type_video (D, T); return;
  case 0xe8e94e8b: store_ds_type_bare_video (D, T); return;
  case 0xafa14973: store_ds_type_wall_paper (D, T); return;
  case 0x505eb68c: store_ds_type_bare_wall_paper (D, T); return;
  case 0x8d8c0d41: store_ds_type_web_page (D, T); return;
  case 0x7273f2be: store_ds_type_bare_web_page (D, T); return;
  case 0x1250abde: store_ds_type_account_authorizations (D, T); return;
  case 0xedaf5421: store_ds_type_bare_account_authorizations (D, T); return;
  case 0xeac2a804: store_ds_type_account_password (D, T); return;
  case 0x153d57fb: store_ds_type_bare_account_password (D, T); return;
  case 0xbcfc532c: store_ds_type_account_password_input_settings (D, T); return;
  case 0x4303acd3: store_ds_type_bare_account_password_input_settings (D, T); return;
  case 0xb7b72ab3: store_ds_type_account_password_settings (D, T); return;
  case 0x4848d54c: store_ds_type_bare_account_password_settings (D, T); return;
  case 0x554abb6f: store_ds_type_account_privacy_rules (D, T); return;
  case 0xaab54490: store_ds_type_bare_account_privacy_rules (D, T); return;
  case 0xa4f58c4c: store_ds_type_account_sent_change_phone_code (D, T); return;
  case 0x5b0a73b3: store_ds_type_bare_account_sent_change_phone_code (D, T); return;
  case 0xff036af1: store_ds_type_auth_authorization (D, T); return;
  case 0x00fc950e: store_ds_type_bare_auth_authorization (D, T); return;
  case 0x811ea28e: store_ds_type_auth_checked_phone (D, T); return;
  case 0x7ee15d71: store_ds_type_bare_auth_checked_phone (D, T); return;
  case 0xdf969c2d: store_ds_type_auth_exported_authorization (D, T); return;
  case 0x206963d2: store_ds_type_bare_auth_exported_authorization (D, T); return;
  case 0x137948a5: store_ds_type_auth_password_recovery (D, T); return;
  case 0xec86b75a: store_ds_type_bare_auth_password_recovery (D, T); return;
  case 0x0cc8bc16: store_ds_type_auth_sent_code (D, T); return;
  case 0xf33743e9: store_ds_type_bare_auth_sent_code (D, T); return;
  case 0x0377168f: store_ds_type_binlog_encr_key (D, T); return;
  case 0xfc88e970: store_ds_type_bare_binlog_encr_key (D, T); return;
  case 0x66ff03a6: store_ds_type_binlog_update (D, T); return;
  case 0x9900fc59: store_ds_type_bare_binlog_update (D, T); return;
  case 0x8c1b8fb4: store_ds_type_contacts_blocked (D, T); return;
  case 0x73e4704b: store_ds_type_bare_contacts_blocked (D, T); return;
  case 0xd8c02560: store_ds_type_contacts_contacts (D, T); return;
  case 0x273fda9f: store_ds_type_bare_contacts_contacts (D, T); return;
  case 0x0566000e: store_ds_type_contacts_found (D, T); return;
  case 0xfa99fff1: store_ds_type_bare_contacts_found (D, T); return;
  case 0xad524315: store_ds_type_contacts_imported_contacts (D, T); return;
  case 0x52adbcea: store_ds_type_bare_contacts_imported_contacts (D, T); return;
  case 0x3ace484c: store_ds_type_contacts_link (D, T); return;
  case 0xc531b7b3: store_ds_type_bare_contacts_link (D, T); return;
  case 0x5649dcc5: store_ds_type_contacts_suggested (D, T); return;
  case 0xa9b6233a: store_ds_type_bare_contacts_suggested (D, T); return;
  case 0x48feb267: store_ds_type_geochats_located (D, T); return;
  case 0xb7014d98: store_ds_type_bare_geochats_located (D, T); return;
  case 0x6d0a0e59: store_ds_type_geochats_messages (D, T); return;
  case 0x92f5f1a6: store_ds_type_bare_geochats_messages (D, T); return;
  case 0x17b1578b: store_ds_type_geochats_stated_message (D, T); return;
  case 0xe84ea874: store_ds_type_bare_geochats_stated_message (D, T); return;
  case 0x4ddd9627: store_ds_type_help_app_update (D, T); return;
  case 0xb22269d8: store_ds_type_bare_help_app_update (D, T); return;
  case 0x18cb9f78: store_ds_type_help_invite_text (D, T); return;
  case 0xe7346087: store_ds_type_bare_help_invite_text (D, T); return;
  case 0x17c6b5f6: store_ds_type_help_support (D, T); return;
  case 0xe8394a09: store_ds_type_bare_help_support (D, T); return;
  case 0xb45c69d1: store_ds_type_messages_affected_history (D, T); return;
  case 0x4ba3962e: store_ds_type_bare_messages_affected_history (D, T); return;
  case 0x84d19185: store_ds_type_messages_affected_messages (D, T); return;
  case 0x7b2e6e7a: store_ds_type_bare_messages_affected_messages (D, T); return;
  case 0xb485502f: store_ds_type_messages_all_stickers (D, T); return;
  case 0x4b7aafd0: store_ds_type_bare_messages_all_stickers (D, T); return;
  case 0xe5d7d19c: store_ds_type_messages_chat_full (D, T); return;
  case 0x1a282e63: store_ds_type_bare_messages_chat_full (D, T); return;
  case 0x64ff9fd5: store_ds_type_messages_chats (D, T); return;
  case 0x9b00602a: store_ds_type_bare_messages_chats (D, T); return;
  case 0xecc058e8: store_ds_type_messages_dh_config (D, T); return;
  case 0x133fa717: store_ds_type_bare_messages_dh_config (D, T); return;
  case 0x645af8b3: store_ds_type_messages_dialogs (D, T); return;
  case 0x9ba5074c: store_ds_type_bare_messages_dialogs (D, T); return;
  case 0x3f4e0648: store_ds_type_messages_message (D, T); return;
  case 0xc0b1f9b7: store_ds_type_bare_messages_message (D, T); return;
  case 0x8735e464: store_ds_type_messages_messages (D, T); return;
  case 0x78ca1b9b: store_ds_type_bare_messages_messages (D, T); return;
  case 0xc29c7607: store_ds_type_messages_sent_encrypted_message (D, T); return;
  case 0x3d6389f8: store_ds_type_bare_messages_sent_encrypted_message (D, T); return;
  case 0x799ce190: store_ds_type_messages_sent_message (D, T); return;
  case 0x86631e6f: store_ds_type_bare_messages_sent_message (D, T); return;
  case 0xb60a24a6: store_ds_type_messages_sticker_set (D, T); return;
  case 0x49f5db59: store_ds_type_bare_messages_sticker_set (D, T); return;
  case 0x7bfa5710: store_ds_type_messages_stickers (D, T); return;
  case 0x8405a8ef: store_ds_type_bare_messages_stickers (D, T); return;
  case 0x20212ca8: store_ds_type_photos_photo (D, T); return;
  case 0xdfded357: store_ds_type_bare_photos_photo (D, T); return;
  case 0x98cf75f1: store_ds_type_photos_photos (D, T); return;
  case 0x67308a0e: store_ds_type_bare_photos_photos (D, T); return;
  case 0x3e2838a8: store_ds_type_storage_file_type (D, T); return;
  case 0xc1d7c757: store_ds_type_bare_storage_file_type (D, T); return;
  case 0xf57a2419: store_ds_type_updates_difference (D, T); return;
  case 0x0a85dbe6: store_ds_type_bare_updates_difference (D, T); return;
  case 0xa56c2a3e: store_ds_type_updates_state (D, T); return;
  case 0x5a93d5c1: store_ds_type_bare_updates_state (D, T); return;
  case 0x096a18d5: store_ds_type_upload_file (D, T); return;
  case 0xf695e72a: store_ds_type_bare_upload_file (D, T); return;
  default: return; }
}
