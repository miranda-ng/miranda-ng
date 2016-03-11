#ifdef _MSC_VER
#include "..\auto.h"
#include "auto-print-ds.h"
#include "auto-skip.h"
#include "..\auto-static-print-ds.c"
#include "..\mtproto-common.h"
#else
#include "auto.h"
#include "auto/auto-print-ds.h"
#include "auto/auto-skip.h"
#include "auto-static-print-ds.c"
#include "mtproto-common.h"
#endif
#include <assert.h>
int print_ds_constructor_account_days_t_t_l (struct tl_ds_account_days_t_t_l *DS, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0xb8d0afdf && T->type->name != 0x472f5020)) { return -1; }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_int (DS->days, field1);
  return 0;
}
int print_ds_constructor_audio_empty (struct tl_ds_audio *DS, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x9fc5ec4e && T->type->name != 0x603a13b1)) { return -1; }
  eprintf (" audioEmpty");
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" id :"); }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xddf89345, .id = "Bare_Long", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_long (DS->id, field1);
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int print_ds_constructor_audio (struct tl_ds_audio *DS, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x9fc5ec4e && T->type->name != 0x603a13b1)) { return -1; }
  eprintf (" audio");
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" id :"); }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xddf89345, .id = "Bare_Long", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_long (DS->id, field1);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" access_hash :"); }
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xddf89345, .id = "Bare_Long", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_long (DS->access_hash, field2);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" user_id :"); }
  struct paramed_type *field3 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_int (DS->user_id, field3);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" date :"); }
  struct paramed_type *field4 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_int (DS->date, field4);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" duration :"); }
  struct paramed_type *field5 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_int (DS->duration, field5);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" mime_type :"); }
  struct paramed_type *field6 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_string (DS->mime_type, field6);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" size :"); }
  struct paramed_type *field7 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_int (DS->size, field7);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" dc_id :"); }
  struct paramed_type *field8 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_int (DS->dc_id, field8);
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int print_ds_constructor_authorization (struct tl_ds_authorization *DS, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x7bf2e6f6 && T->type->name != 0x840d1909)) { return -1; }
  eprintf (" authorization");
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" hash :"); }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xddf89345, .id = "Bare_Long", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_long (DS->hash, field1);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" flags :"); }
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_int (DS->flags, field2);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" device_model :"); }
  struct paramed_type *field3 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_string (DS->device_model, field3);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" platform :"); }
  struct paramed_type *field4 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_string (DS->platform, field4);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" system_version :"); }
  struct paramed_type *field5 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_string (DS->system_version, field5);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" api_id :"); }
  struct paramed_type *field6 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_int (DS->api_id, field6);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" app_name :"); }
  struct paramed_type *field7 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_string (DS->app_name, field7);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" app_version :"); }
  struct paramed_type *field8 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_string (DS->app_version, field8);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" date_created :"); }
  struct paramed_type *field9 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_int (DS->date_created, field9);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" date_active :"); }
  struct paramed_type *field10 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_int (DS->date_active, field10);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" ip :"); }
  struct paramed_type *field11 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_string (DS->ip, field11);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" country :"); }
  struct paramed_type *field12 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_string (DS->country, field12);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" region :"); }
  struct paramed_type *field13 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_string (DS->region, field13);
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int print_ds_constructor_bool_false (struct tl_ds_bool *DS, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x250be282 && T->type->name != 0xdaf41d7d)) { return -1; }
  eprintf (" boolFalse");
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int print_ds_constructor_bool_true (struct tl_ds_bool *DS, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x250be282 && T->type->name != 0xdaf41d7d)) { return -1; }
  eprintf (" boolTrue");
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int print_ds_constructor_bot_command (struct tl_ds_bot_command *DS, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x75e7ea6c && T->type->name != 0x8a181593)) { return -1; }
  eprintf (" botCommand");
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" command :"); }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_string (DS->command, field1);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" description :"); }
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_string (DS->description, field2);
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int print_ds_constructor_bot_command_old (struct tl_ds_bot_command *DS, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x75e7ea6c && T->type->name != 0x8a181593)) { return -1; }
  eprintf (" botCommandOld");
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" command :"); }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_string (DS->command, field1);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" params :"); }
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_string (DS->params, field2);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" description :"); }
  struct paramed_type *field3 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_string (DS->description, field3);
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int print_ds_constructor_bot_info_empty (struct tl_ds_bot_info *DS, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0xb2e16f93 && T->type->name != 0x4d1e906c)) { return -1; }
  eprintf (" botInfoEmpty");
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int print_ds_constructor_bot_info (struct tl_ds_bot_info *DS, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0xb2e16f93 && T->type->name != 0x4d1e906c)) { return -1; }
  eprintf (" botInfo");
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" user_id :"); }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_int (DS->user_id, field1);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" version :"); }
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_int (DS->version, field2);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" share_text :"); }
  struct paramed_type *field3 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_string (DS->share_text, field3);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" description :"); }
  struct paramed_type *field4 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_string (DS->description, field4);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" commands :"); }
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
  print_ds_type_vector ((void *)DS->commands, field5);
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int print_ds_constructor_bytes (struct tl_ds_string *DS, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x0ee1379f && T->type->name != 0xf11ec860)) { return -1; }
  print_escaped_string (DS->data, DS->len);
  return 0;
}
int print_ds_constructor_chat_empty (struct tl_ds_chat *DS, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x7bd865dc && T->type->name != 0x84279a23)) { return -1; }
  eprintf (" chatEmpty");
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" id :"); }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_int (DS->id, field1);
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int print_ds_constructor_chat (struct tl_ds_chat *DS, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x7bd865dc && T->type->name != 0x84279a23)) { return -1; }
  eprintf (" chat");
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" id :"); }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_int (DS->id, field1);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" title :"); }
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_string (DS->title, field2);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" photo :"); }
  struct paramed_type *field3 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x56922676, .id = "ChatPhoto", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_chat_photo (DS->photo, field3);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" participants_count :"); }
  struct paramed_type *field4 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_int (DS->participants_count, field4);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" date :"); }
  struct paramed_type *field5 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_int (DS->date, field5);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" left :"); }
  struct paramed_type *field6 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x250be282, .id = "Bool", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bool (DS->left, field6);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" version :"); }
  struct paramed_type *field7 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_int (DS->version, field7);
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int print_ds_constructor_chat_forbidden (struct tl_ds_chat *DS, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x7bd865dc && T->type->name != 0x84279a23)) { return -1; }
  eprintf (" chatForbidden");
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" id :"); }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_int (DS->id, field1);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" title :"); }
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_string (DS->title, field2);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" date :"); }
  struct paramed_type *field3 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_int (DS->date, field3);
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int print_ds_constructor_geo_chat (struct tl_ds_chat *DS, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x7bd865dc && T->type->name != 0x84279a23)) { return -1; }
  eprintf (" geoChat");
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" id :"); }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_int (DS->id, field1);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" access_hash :"); }
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xddf89345, .id = "Bare_Long", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_long (DS->access_hash, field2);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" title :"); }
  struct paramed_type *field3 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_string (DS->title, field3);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" address :"); }
  struct paramed_type *field4 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_string (DS->address, field4);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" venue :"); }
  struct paramed_type *field5 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_string (DS->venue, field5);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" geo :"); }
  struct paramed_type *field6 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x315e0a53, .id = "GeoPoint", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_geo_point (DS->geo, field6);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" photo :"); }
  struct paramed_type *field7 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x56922676, .id = "ChatPhoto", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_chat_photo (DS->photo, field7);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" participants_count :"); }
  struct paramed_type *field8 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_int (DS->participants_count, field8);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" date :"); }
  struct paramed_type *field9 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_int (DS->date, field9);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" checked_in :"); }
  struct paramed_type *field10 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x250be282, .id = "Bool", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bool (DS->checked_in, field10);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" version :"); }
  struct paramed_type *field11 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_int (DS->version, field11);
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int print_ds_constructor_chat_full (struct tl_ds_chat_full *DS, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x2e02a614 && T->type->name != 0xd1fd59eb)) { return -1; }
  eprintf (" chatFull");
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" id :"); }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_int (DS->id, field1);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" participants :"); }
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x77930f9f, .id = "ChatParticipants", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_chat_participants (DS->participants, field2);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" chat_photo :"); }
  struct paramed_type *field3 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xc207550a, .id = "Photo", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_photo (DS->chat_photo, field3);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" notify_settings :"); }
  struct paramed_type *field4 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xfdf894fc, .id = "PeerNotifySettings", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_peer_notify_settings (DS->notify_settings, field4);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" exported_invite :"); }
  struct paramed_type *field5 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x95f132d5, .id = "ExportedChatInvite", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_exported_chat_invite (DS->exported_invite, field5);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" bot_info :"); }
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
  print_ds_type_vector ((void *)DS->bot_info, field6);
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int print_ds_constructor_chat_invite_already (struct tl_ds_chat_invite *DS, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x94f910b1 && T->type->name != 0x6b06ef4e)) { return -1; }
  eprintf (" chatInviteAlready");
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" chat :"); }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x7bd865dc, .id = "Chat", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_chat (DS->chat, field1);
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int print_ds_constructor_chat_invite (struct tl_ds_chat_invite *DS, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x94f910b1 && T->type->name != 0x6b06ef4e)) { return -1; }
  eprintf (" chatInvite");
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" title :"); }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_string (DS->title, field1);
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int print_ds_constructor_chat_located (struct tl_ds_chat_located *DS, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x3631cf4c && T->type->name != 0xc9ce30b3)) { return -1; }
  eprintf (" chatLocated");
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" chat_id :"); }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_int (DS->chat_id, field1);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" distance :"); }
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_int (DS->distance, field2);
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int print_ds_constructor_chat_participant (struct tl_ds_chat_participant *DS, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0xc8d7493e && T->type->name != 0x3728b6c1)) { return -1; }
  eprintf (" chatParticipant");
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" user_id :"); }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_int (DS->user_id, field1);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" inviter_id :"); }
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_int (DS->inviter_id, field2);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" date :"); }
  struct paramed_type *field3 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_int (DS->date, field3);
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int print_ds_constructor_chat_participants_forbidden (struct tl_ds_chat_participants *DS, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x77930f9f && T->type->name != 0x886cf060)) { return -1; }
  eprintf (" chatParticipantsForbidden");
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" chat_id :"); }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_int (DS->chat_id, field1);
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int print_ds_constructor_chat_participants (struct tl_ds_chat_participants *DS, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x77930f9f && T->type->name != 0x886cf060)) { return -1; }
  eprintf (" chatParticipants");
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" chat_id :"); }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_int (DS->chat_id, field1);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" admin_id :"); }
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_int (DS->admin_id, field2);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" participants :"); }
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
  print_ds_type_vector ((void *)DS->participants, field3);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" version :"); }
  struct paramed_type *field4 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_int (DS->version, field4);
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int print_ds_constructor_chat_photo_empty (struct tl_ds_chat_photo *DS, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x56922676 && T->type->name != 0xa96dd989)) { return -1; }
  eprintf (" chatPhotoEmpty");
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int print_ds_constructor_chat_photo (struct tl_ds_chat_photo *DS, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x56922676 && T->type->name != 0xa96dd989)) { return -1; }
  eprintf (" chatPhoto");
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" photo_small :"); }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x2f8ffb30, .id = "FileLocation", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_file_location (DS->photo_small, field1);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" photo_big :"); }
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x2f8ffb30, .id = "FileLocation", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_file_location (DS->photo_big, field2);
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int print_ds_constructor_client_d_h_inner_data (struct tl_ds_client_d_h_inner_data *DS, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x6643b654 && T->type->name != 0x99bc49ab)) { return -1; }
  eprintf (" client_DH_inner_data");
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" nonce :"); }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x82c93bc6, .id = "Bare_Int128", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_int128 (DS->nonce, field1);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" server_nonce :"); }
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x82c93bc6, .id = "Bare_Int128", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_int128 (DS->server_nonce, field2);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" retry_id :"); }
  struct paramed_type *field3 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xddf89345, .id = "Bare_Long", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_long (DS->retry_id, field3);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" g_b :"); }
  struct paramed_type *field4 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_string (DS->g_b, field4);
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int print_ds_constructor_config (struct tl_ds_config *DS, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x4e32b894 && T->type->name != 0xb1cd476b)) { return -1; }
  eprintf (" config");
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" date :"); }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_int (DS->date, field1);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" expires :"); }
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_int (DS->expires, field2);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" test_mode :"); }
  struct paramed_type *field3 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x250be282, .id = "Bool", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bool (DS->test_mode, field3);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" this_dc :"); }
  struct paramed_type *field4 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_int (DS->this_dc, field4);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" dc_options :"); }
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
  print_ds_type_vector ((void *)DS->dc_options, field5);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" chat_size_max :"); }
  struct paramed_type *field6 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_int (DS->chat_size_max, field6);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" broadcast_size_max :"); }
  struct paramed_type *field7 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_int (DS->broadcast_size_max, field7);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" forwarded_count_max :"); }
  struct paramed_type *field8 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_int (DS->forwarded_count_max, field8);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" online_update_period_ms :"); }
  struct paramed_type *field9 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_int (DS->online_update_period_ms, field9);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" offline_blur_timeout_ms :"); }
  struct paramed_type *field10 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_int (DS->offline_blur_timeout_ms, field10);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" offline_idle_timeout_ms :"); }
  struct paramed_type *field11 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_int (DS->offline_idle_timeout_ms, field11);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" online_cloud_timeout_ms :"); }
  struct paramed_type *field12 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_int (DS->online_cloud_timeout_ms, field12);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" notify_cloud_delay_ms :"); }
  struct paramed_type *field13 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_int (DS->notify_cloud_delay_ms, field13);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" notify_default_delay_ms :"); }
  struct paramed_type *field14 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_int (DS->notify_default_delay_ms, field14);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" chat_big_size :"); }
  struct paramed_type *field15 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_int (DS->chat_big_size, field15);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" push_chat_period_ms :"); }
  struct paramed_type *field16 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_int (DS->push_chat_period_ms, field16);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" push_chat_limit :"); }
  struct paramed_type *field17 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_int (DS->push_chat_limit, field17);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" disabled_features :"); }
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
  print_ds_type_vector ((void *)DS->disabled_features, field18);
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int print_ds_constructor_contact (struct tl_ds_contact *DS, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0xf911c994 && T->type->name != 0x06ee366b)) { return -1; }
  eprintf (" contact");
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" user_id :"); }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_int (DS->user_id, field1);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" mutual :"); }
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x250be282, .id = "Bool", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bool (DS->mutual, field2);
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int print_ds_constructor_contact_blocked (struct tl_ds_contact_blocked *DS, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x561bc879 && T->type->name != 0xa9e43786)) { return -1; }
  eprintf (" contactBlocked");
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" user_id :"); }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_int (DS->user_id, field1);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" date :"); }
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_int (DS->date, field2);
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int print_ds_constructor_contact_found (struct tl_ds_contact_found *DS, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0xea879f95 && T->type->name != 0x1578606a)) { return -1; }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_int (DS->user_id, field1);
  return 0;
}
int print_ds_constructor_contact_link_unknown (struct tl_ds_contact_link *DS, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x522fbc63 && T->type->name != 0xadd0439c)) { return -1; }
  eprintf (" contactLinkUnknown");
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int print_ds_constructor_contact_link_none (struct tl_ds_contact_link *DS, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x522fbc63 && T->type->name != 0xadd0439c)) { return -1; }
  eprintf (" contactLinkNone");
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int print_ds_constructor_contact_link_has_phone (struct tl_ds_contact_link *DS, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x522fbc63 && T->type->name != 0xadd0439c)) { return -1; }
  eprintf (" contactLinkHasPhone");
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int print_ds_constructor_contact_link_contact (struct tl_ds_contact_link *DS, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x522fbc63 && T->type->name != 0xadd0439c)) { return -1; }
  eprintf (" contactLinkContact");
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int print_ds_constructor_contact_status (struct tl_ds_contact_status *DS, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0xd3680c61 && T->type->name != 0x2c97f39e)) { return -1; }
  eprintf (" contactStatus");
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" user_id :"); }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_int (DS->user_id, field1);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" status :"); }
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x76de9570, .id = "UserStatus", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_user_status (DS->status, field2);
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int print_ds_constructor_contact_suggested (struct tl_ds_contact_suggested *DS, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x3de191a1 && T->type->name != 0xc21e6e5e)) { return -1; }
  eprintf (" contactSuggested");
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" user_id :"); }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_int (DS->user_id, field1);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" mutual_contacts :"); }
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_int (DS->mutual_contacts, field2);
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int print_ds_constructor_dc_option_l28 (struct tl_ds_dc_option *DS, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x2b1a62f0 && T->type->name != 0xd4e59d0f)) { return -1; }
  eprintf (" dcOptionL28");
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" id :"); }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_int (DS->id, field1);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" hostname :"); }
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_string (DS->hostname, field2);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" ip_address :"); }
  struct paramed_type *field3 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_string (DS->ip_address, field3);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" port :"); }
  struct paramed_type *field4 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_int (DS->port, field4);
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int print_ds_constructor_dc_option (struct tl_ds_dc_option *DS, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x2b1a62f0 && T->type->name != 0xd4e59d0f)) { return -1; }
  eprintf (" dcOption");
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" flags :"); }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_int (DS->flags, field1);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" id :"); }
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_int (DS->id, field2);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" ip_address :"); }
  struct paramed_type *field3 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_string (DS->ip_address, field3);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" port :"); }
  struct paramed_type *field4 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_int (DS->port, field4);
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int print_ds_constructor_decrypted_message (struct tl_ds_decrypted_message *DS, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x535b7918 && T->type->name != 0xaca486e7)) { return -1; }
  eprintf (" decryptedMessage");
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" random_id :"); }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xddf89345, .id = "Bare_Long", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_long (DS->random_id, field1);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" ttl :"); }
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_int (DS->ttl, field2);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" message :"); }
  struct paramed_type *field3 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_string (DS->message, field3);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" media :"); }
  struct paramed_type *field4 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x34e0d674, .id = "DecryptedMessageMedia", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_decrypted_message_media (DS->media, field4);
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int print_ds_constructor_decrypted_message_service (struct tl_ds_decrypted_message *DS, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x535b7918 && T->type->name != 0xaca486e7)) { return -1; }
  eprintf (" decryptedMessageService");
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" random_id :"); }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xddf89345, .id = "Bare_Long", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_long (DS->random_id, field1);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" action :"); }
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x4e0eefde, .id = "DecryptedMessageAction", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_decrypted_message_action (DS->action, field2);
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int print_ds_constructor_decrypted_message_action_set_message_t_t_l (struct tl_ds_decrypted_message_action *DS, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x4e0eefde && T->type->name != 0xb1f11021)) { return -1; }
  eprintf (" decryptedMessageActionSetMessageTTL");
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" ttl_seconds :"); }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_int (DS->ttl_seconds, field1);
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int print_ds_constructor_decrypted_message_action_read_messages (struct tl_ds_decrypted_message_action *DS, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x4e0eefde && T->type->name != 0xb1f11021)) { return -1; }
  eprintf (" decryptedMessageActionReadMessages");
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" random_ids :"); }
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
  print_ds_type_vector ((void *)DS->random_ids, field1);
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int print_ds_constructor_decrypted_message_action_delete_messages (struct tl_ds_decrypted_message_action *DS, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x4e0eefde && T->type->name != 0xb1f11021)) { return -1; }
  eprintf (" decryptedMessageActionDeleteMessages");
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" random_ids :"); }
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
  print_ds_type_vector ((void *)DS->random_ids, field1);
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int print_ds_constructor_decrypted_message_action_screenshot_messages (struct tl_ds_decrypted_message_action *DS, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x4e0eefde && T->type->name != 0xb1f11021)) { return -1; }
  eprintf (" decryptedMessageActionScreenshotMessages");
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" random_ids :"); }
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
  print_ds_type_vector ((void *)DS->random_ids, field1);
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int print_ds_constructor_decrypted_message_action_flush_history (struct tl_ds_decrypted_message_action *DS, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x4e0eefde && T->type->name != 0xb1f11021)) { return -1; }
  eprintf (" decryptedMessageActionFlushHistory");
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int print_ds_constructor_decrypted_message_action_resend (struct tl_ds_decrypted_message_action *DS, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x4e0eefde && T->type->name != 0xb1f11021)) { return -1; }
  eprintf (" decryptedMessageActionResend");
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" start_seq_no :"); }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_int (DS->start_seq_no, field1);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" end_seq_no :"); }
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_int (DS->end_seq_no, field2);
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int print_ds_constructor_decrypted_message_action_notify_layer (struct tl_ds_decrypted_message_action *DS, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x4e0eefde && T->type->name != 0xb1f11021)) { return -1; }
  eprintf (" decryptedMessageActionNotifyLayer");
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" layer :"); }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_int (DS->layer, field1);
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int print_ds_constructor_decrypted_message_action_typing (struct tl_ds_decrypted_message_action *DS, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x4e0eefde && T->type->name != 0xb1f11021)) { return -1; }
  eprintf (" decryptedMessageActionTyping");
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" action :"); }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x70541c7e, .id = "SendMessageAction", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_send_message_action (DS->action, field1);
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int print_ds_constructor_decrypted_message_action_request_key (struct tl_ds_decrypted_message_action *DS, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x4e0eefde && T->type->name != 0xb1f11021)) { return -1; }
  eprintf (" decryptedMessageActionRequestKey");
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" exchange_id :"); }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xddf89345, .id = "Bare_Long", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_long (DS->exchange_id, field1);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" g_a :"); }
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xf11ec860, .id = "Bare_Bytes", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_bytes (DS->g_a, field2);
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int print_ds_constructor_decrypted_message_action_accept_key (struct tl_ds_decrypted_message_action *DS, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x4e0eefde && T->type->name != 0xb1f11021)) { return -1; }
  eprintf (" decryptedMessageActionAcceptKey");
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" exchange_id :"); }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xddf89345, .id = "Bare_Long", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_long (DS->exchange_id, field1);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" g_b :"); }
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xf11ec860, .id = "Bare_Bytes", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_bytes (DS->g_b, field2);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" key_fingerprint :"); }
  struct paramed_type *field3 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xddf89345, .id = "Bare_Long", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_long (DS->key_fingerprint, field3);
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int print_ds_constructor_decrypted_message_action_abort_key (struct tl_ds_decrypted_message_action *DS, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x4e0eefde && T->type->name != 0xb1f11021)) { return -1; }
  eprintf (" decryptedMessageActionAbortKey");
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" exchange_id :"); }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xddf89345, .id = "Bare_Long", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_long (DS->exchange_id, field1);
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int print_ds_constructor_decrypted_message_action_commit_key (struct tl_ds_decrypted_message_action *DS, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x4e0eefde && T->type->name != 0xb1f11021)) { return -1; }
  eprintf (" decryptedMessageActionCommitKey");
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" exchange_id :"); }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xddf89345, .id = "Bare_Long", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_long (DS->exchange_id, field1);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" key_fingerprint :"); }
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xddf89345, .id = "Bare_Long", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_long (DS->key_fingerprint, field2);
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int print_ds_constructor_decrypted_message_action_noop (struct tl_ds_decrypted_message_action *DS, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x4e0eefde && T->type->name != 0xb1f11021)) { return -1; }
  eprintf (" decryptedMessageActionNoop");
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int print_ds_constructor_decrypted_message_layer (struct tl_ds_decrypted_message_layer *DS, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x1be31789 && T->type->name != 0xe41ce876)) { return -1; }
  eprintf (" decryptedMessageLayer");
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" random_bytes :"); }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xf11ec860, .id = "Bare_Bytes", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_bytes (DS->random_bytes, field1);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" layer :"); }
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_int (DS->layer, field2);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" in_seq_no :"); }
  struct paramed_type *field3 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_int (DS->in_seq_no, field3);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" out_seq_no :"); }
  struct paramed_type *field4 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_int (DS->out_seq_no, field4);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" message :"); }
  struct paramed_type *field5 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x535b7918, .id = "DecryptedMessage", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_decrypted_message (DS->message, field5);
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int print_ds_constructor_decrypted_message_media_empty (struct tl_ds_decrypted_message_media *DS, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x34e0d674 && T->type->name != 0xcb1f298b)) { return -1; }
  eprintf (" decryptedMessageMediaEmpty");
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int print_ds_constructor_decrypted_message_media_photo (struct tl_ds_decrypted_message_media *DS, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x34e0d674 && T->type->name != 0xcb1f298b)) { return -1; }
  eprintf (" decryptedMessageMediaPhoto");
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" str_thumb :"); }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xf11ec860, .id = "Bare_Bytes", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_bytes (DS->str_thumb, field1);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" thumb_w :"); }
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_int (DS->thumb_w, field2);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" thumb_h :"); }
  struct paramed_type *field3 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_int (DS->thumb_h, field3);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" w :"); }
  struct paramed_type *field4 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_int (DS->w, field4);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" h :"); }
  struct paramed_type *field5 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_int (DS->h, field5);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" size :"); }
  struct paramed_type *field6 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_int (DS->size, field6);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" key :"); }
  struct paramed_type *field7 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xf11ec860, .id = "Bare_Bytes", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_bytes (DS->key, field7);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" iv :"); }
  struct paramed_type *field8 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xf11ec860, .id = "Bare_Bytes", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_bytes (DS->iv, field8);
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int print_ds_constructor_decrypted_message_media_geo_point (struct tl_ds_decrypted_message_media *DS, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x34e0d674 && T->type->name != 0xcb1f298b)) { return -1; }
  eprintf (" decryptedMessageMediaGeoPoint");
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" latitude :"); }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xddef3eab, .id = "Bare_Double", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_double (DS->latitude, field1);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" longitude :"); }
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xddef3eab, .id = "Bare_Double", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_double (DS->longitude, field2);
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int print_ds_constructor_decrypted_message_media_contact (struct tl_ds_decrypted_message_media *DS, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x34e0d674 && T->type->name != 0xcb1f298b)) { return -1; }
  eprintf (" decryptedMessageMediaContact");
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" phone_number :"); }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_string (DS->phone_number, field1);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" first_name :"); }
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_string (DS->first_name, field2);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" last_name :"); }
  struct paramed_type *field3 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_string (DS->last_name, field3);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" user_id :"); }
  struct paramed_type *field4 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_int (DS->user_id, field4);
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int print_ds_constructor_decrypted_message_media_document (struct tl_ds_decrypted_message_media *DS, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x34e0d674 && T->type->name != 0xcb1f298b)) { return -1; }
  eprintf (" decryptedMessageMediaDocument");
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" str_thumb :"); }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xf11ec860, .id = "Bare_Bytes", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_bytes (DS->str_thumb, field1);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" thumb_w :"); }
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_int (DS->thumb_w, field2);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" thumb_h :"); }
  struct paramed_type *field3 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_int (DS->thumb_h, field3);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" file_name :"); }
  struct paramed_type *field4 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_string (DS->file_name, field4);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" mime_type :"); }
  struct paramed_type *field5 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_string (DS->mime_type, field5);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" size :"); }
  struct paramed_type *field6 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_int (DS->size, field6);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" key :"); }
  struct paramed_type *field7 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xf11ec860, .id = "Bare_Bytes", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_bytes (DS->key, field7);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" iv :"); }
  struct paramed_type *field8 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xf11ec860, .id = "Bare_Bytes", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_bytes (DS->iv, field8);
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int print_ds_constructor_decrypted_message_media_video (struct tl_ds_decrypted_message_media *DS, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x34e0d674 && T->type->name != 0xcb1f298b)) { return -1; }
  eprintf (" decryptedMessageMediaVideo");
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" str_thumb :"); }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xf11ec860, .id = "Bare_Bytes", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_bytes (DS->str_thumb, field1);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" thumb_w :"); }
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_int (DS->thumb_w, field2);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" thumb_h :"); }
  struct paramed_type *field3 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_int (DS->thumb_h, field3);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" duration :"); }
  struct paramed_type *field4 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_int (DS->duration, field4);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" mime_type :"); }
  struct paramed_type *field5 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_string (DS->mime_type, field5);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" w :"); }
  struct paramed_type *field6 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_int (DS->w, field6);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" h :"); }
  struct paramed_type *field7 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_int (DS->h, field7);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" size :"); }
  struct paramed_type *field8 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_int (DS->size, field8);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" key :"); }
  struct paramed_type *field9 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xf11ec860, .id = "Bare_Bytes", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_bytes (DS->key, field9);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" iv :"); }
  struct paramed_type *field10 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xf11ec860, .id = "Bare_Bytes", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_bytes (DS->iv, field10);
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int print_ds_constructor_decrypted_message_media_audio (struct tl_ds_decrypted_message_media *DS, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x34e0d674 && T->type->name != 0xcb1f298b)) { return -1; }
  eprintf (" decryptedMessageMediaAudio");
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" duration :"); }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_int (DS->duration, field1);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" mime_type :"); }
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_string (DS->mime_type, field2);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" size :"); }
  struct paramed_type *field3 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_int (DS->size, field3);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" key :"); }
  struct paramed_type *field4 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xf11ec860, .id = "Bare_Bytes", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_bytes (DS->key, field4);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" iv :"); }
  struct paramed_type *field5 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xf11ec860, .id = "Bare_Bytes", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_bytes (DS->iv, field5);
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int print_ds_constructor_decrypted_message_media_external_document (struct tl_ds_decrypted_message_media *DS, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x34e0d674 && T->type->name != 0xcb1f298b)) { return -1; }
  eprintf (" decryptedMessageMediaExternalDocument");
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" id :"); }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xddf89345, .id = "Bare_Long", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_long (DS->id, field1);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" access_hash :"); }
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xddf89345, .id = "Bare_Long", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_long (DS->access_hash, field2);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" date :"); }
  struct paramed_type *field3 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_int (DS->date, field3);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" mime_type :"); }
  struct paramed_type *field4 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_string (DS->mime_type, field4);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" size :"); }
  struct paramed_type *field5 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_int (DS->size, field5);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" thumb :"); }
  struct paramed_type *field6 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x900f60dd, .id = "PhotoSize", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_photo_size (DS->thumb, field6);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" dc_id :"); }
  struct paramed_type *field7 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_int (DS->dc_id, field7);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" attributes :"); }
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
  print_ds_type_vector ((void *)DS->attributes, field8);
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int print_ds_constructor_decrypted_message_media_video_l12 (struct tl_ds_decrypted_message_media *DS, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x34e0d674 && T->type->name != 0xcb1f298b)) { return -1; }
  eprintf (" decryptedMessageMediaVideoL12");
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" str_thumb :"); }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xf11ec860, .id = "Bare_Bytes", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_bytes (DS->str_thumb, field1);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" thumb_w :"); }
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_int (DS->thumb_w, field2);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" thumb_h :"); }
  struct paramed_type *field3 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_int (DS->thumb_h, field3);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" duration :"); }
  struct paramed_type *field4 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_int (DS->duration, field4);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" w :"); }
  struct paramed_type *field5 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_int (DS->w, field5);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" h :"); }
  struct paramed_type *field6 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_int (DS->h, field6);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" size :"); }
  struct paramed_type *field7 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_int (DS->size, field7);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" key :"); }
  struct paramed_type *field8 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xf11ec860, .id = "Bare_Bytes", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_bytes (DS->key, field8);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" iv :"); }
  struct paramed_type *field9 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xf11ec860, .id = "Bare_Bytes", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_bytes (DS->iv, field9);
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int print_ds_constructor_decrypted_message_media_audio_l12 (struct tl_ds_decrypted_message_media *DS, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x34e0d674 && T->type->name != 0xcb1f298b)) { return -1; }
  eprintf (" decryptedMessageMediaAudioL12");
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" duration :"); }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_int (DS->duration, field1);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" size :"); }
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_int (DS->size, field2);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" key :"); }
  struct paramed_type *field3 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xf11ec860, .id = "Bare_Bytes", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_bytes (DS->key, field3);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" iv :"); }
  struct paramed_type *field4 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xf11ec860, .id = "Bare_Bytes", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_bytes (DS->iv, field4);
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int print_ds_constructor_dialog (struct tl_ds_dialog *DS, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0xc1dd804a && T->type->name != 0x3e227fb5)) { return -1; }
  eprintf (" dialog");
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" peer :"); }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x276159d6, .id = "Peer", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_peer (DS->peer, field1);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" top_message :"); }
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_int (DS->top_message, field2);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" read_inbox_max_id :"); }
  struct paramed_type *field3 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_int (DS->read_inbox_max_id, field3);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" unread_count :"); }
  struct paramed_type *field4 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_int (DS->unread_count, field4);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" notify_settings :"); }
  struct paramed_type *field5 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xfdf894fc, .id = "PeerNotifySettings", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_peer_notify_settings (DS->notify_settings, field5);
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int print_ds_constructor_disabled_feature (struct tl_ds_disabled_feature *DS, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0xae636f24 && T->type->name != 0x519c90db)) { return -1; }
  eprintf (" disabledFeature");
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" feature :"); }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_string (DS->feature, field1);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" description :"); }
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_string (DS->description, field2);
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int print_ds_constructor_document_empty (struct tl_ds_document *DS, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x51a73418 && T->type->name != 0xae58cbe7)) { return -1; }
  eprintf (" documentEmpty");
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" id :"); }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xddf89345, .id = "Bare_Long", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_long (DS->id, field1);
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int print_ds_constructor_document (struct tl_ds_document *DS, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x51a73418 && T->type->name != 0xae58cbe7)) { return -1; }
  eprintf (" document");
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" id :"); }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xddf89345, .id = "Bare_Long", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_long (DS->id, field1);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" access_hash :"); }
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xddf89345, .id = "Bare_Long", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_long (DS->access_hash, field2);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" date :"); }
  struct paramed_type *field3 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_int (DS->date, field3);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" mime_type :"); }
  struct paramed_type *field4 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_string (DS->mime_type, field4);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" size :"); }
  struct paramed_type *field5 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_int (DS->size, field5);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" thumb :"); }
  struct paramed_type *field6 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x900f60dd, .id = "PhotoSize", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_photo_size (DS->thumb, field6);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" dc_id :"); }
  struct paramed_type *field7 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_int (DS->dc_id, field7);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" attributes :"); }
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
  print_ds_type_vector ((void *)DS->attributes, field8);
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int print_ds_constructor_document_l19 (struct tl_ds_document *DS, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x51a73418 && T->type->name != 0xae58cbe7)) { return -1; }
  eprintf (" document_l19");
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" id :"); }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xddf89345, .id = "Bare_Long", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_long (DS->id, field1);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" access_hash :"); }
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xddf89345, .id = "Bare_Long", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_long (DS->access_hash, field2);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" user_id :"); }
  struct paramed_type *field3 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_int (DS->user_id, field3);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" date :"); }
  struct paramed_type *field4 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_int (DS->date, field4);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" file_name :"); }
  struct paramed_type *field5 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_string (DS->file_name, field5);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" mime_type :"); }
  struct paramed_type *field6 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_string (DS->mime_type, field6);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" size :"); }
  struct paramed_type *field7 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_int (DS->size, field7);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" thumb :"); }
  struct paramed_type *field8 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x900f60dd, .id = "PhotoSize", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_photo_size (DS->thumb, field8);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" dc_id :"); }
  struct paramed_type *field9 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_int (DS->dc_id, field9);
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int print_ds_constructor_document_attribute_image_size (struct tl_ds_document_attribute *DS, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x97c637a3 && T->type->name != 0x6839c85c)) { return -1; }
  eprintf (" documentAttributeImageSize");
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" w :"); }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_int (DS->w, field1);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" h :"); }
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_int (DS->h, field2);
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int print_ds_constructor_document_attribute_animated (struct tl_ds_document_attribute *DS, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x97c637a3 && T->type->name != 0x6839c85c)) { return -1; }
  eprintf (" documentAttributeAnimated");
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int print_ds_constructor_document_attribute_sticker_l28 (struct tl_ds_document_attribute *DS, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x97c637a3 && T->type->name != 0x6839c85c)) { return -1; }
  eprintf (" documentAttributeStickerL28");
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" alt :"); }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_string (DS->alt, field1);
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int print_ds_constructor_document_attribute_sticker (struct tl_ds_document_attribute *DS, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x97c637a3 && T->type->name != 0x6839c85c)) { return -1; }
  eprintf (" documentAttributeSticker");
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" alt :"); }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_string (DS->alt, field1);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" stickerset :"); }
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xe44d415c, .id = "InputStickerSet", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_input_sticker_set (DS->stickerset, field2);
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int print_ds_constructor_document_attribute_video (struct tl_ds_document_attribute *DS, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x97c637a3 && T->type->name != 0x6839c85c)) { return -1; }
  eprintf (" documentAttributeVideo");
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" duration :"); }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_int (DS->duration, field1);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" w :"); }
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_int (DS->w, field2);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" h :"); }
  struct paramed_type *field3 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_int (DS->h, field3);
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int print_ds_constructor_document_attribute_audio (struct tl_ds_document_attribute *DS, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x97c637a3 && T->type->name != 0x6839c85c)) { return -1; }
  eprintf (" documentAttributeAudio");
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" duration :"); }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_int (DS->duration, field1);
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int print_ds_constructor_document_attribute_filename (struct tl_ds_document_attribute *DS, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x97c637a3 && T->type->name != 0x6839c85c)) { return -1; }
  eprintf (" documentAttributeFilename");
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" file_name :"); }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_string (DS->file_name, field1);
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int print_ds_constructor_double (double *DS, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x2210c154 && T->type->name != 0xddef3eab)) { return -1; }
  eprintf (" %lf", *DS);
  return 0;
}
int print_ds_constructor_encrypted_chat_empty (struct tl_ds_encrypted_chat *DS, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0xb1718213 && T->type->name != 0x4e8e7dec)) { return -1; }
  eprintf (" encryptedChatEmpty");
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" id :"); }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_int (DS->id, field1);
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int print_ds_constructor_encrypted_chat_waiting (struct tl_ds_encrypted_chat *DS, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0xb1718213 && T->type->name != 0x4e8e7dec)) { return -1; }
  eprintf (" encryptedChatWaiting");
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" id :"); }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_int (DS->id, field1);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" access_hash :"); }
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xddf89345, .id = "Bare_Long", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_long (DS->access_hash, field2);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" date :"); }
  struct paramed_type *field3 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_int (DS->date, field3);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" admin_id :"); }
  struct paramed_type *field4 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_int (DS->admin_id, field4);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" participant_id :"); }
  struct paramed_type *field5 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_int (DS->participant_id, field5);
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int print_ds_constructor_encrypted_chat_requested (struct tl_ds_encrypted_chat *DS, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0xb1718213 && T->type->name != 0x4e8e7dec)) { return -1; }
  eprintf (" encryptedChatRequested");
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" id :"); }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_int (DS->id, field1);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" access_hash :"); }
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xddf89345, .id = "Bare_Long", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_long (DS->access_hash, field2);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" date :"); }
  struct paramed_type *field3 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_int (DS->date, field3);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" admin_id :"); }
  struct paramed_type *field4 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_int (DS->admin_id, field4);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" participant_id :"); }
  struct paramed_type *field5 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_int (DS->participant_id, field5);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" g_a :"); }
  struct paramed_type *field6 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xf11ec860, .id = "Bare_Bytes", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_bytes (DS->g_a, field6);
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int print_ds_constructor_encrypted_chat (struct tl_ds_encrypted_chat *DS, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0xb1718213 && T->type->name != 0x4e8e7dec)) { return -1; }
  eprintf (" encryptedChat");
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" id :"); }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_int (DS->id, field1);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" access_hash :"); }
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xddf89345, .id = "Bare_Long", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_long (DS->access_hash, field2);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" date :"); }
  struct paramed_type *field3 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_int (DS->date, field3);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" admin_id :"); }
  struct paramed_type *field4 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_int (DS->admin_id, field4);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" participant_id :"); }
  struct paramed_type *field5 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_int (DS->participant_id, field5);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" g_a_or_b :"); }
  struct paramed_type *field6 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xf11ec860, .id = "Bare_Bytes", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_bytes (DS->g_a_or_b, field6);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" key_fingerprint :"); }
  struct paramed_type *field7 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xddf89345, .id = "Bare_Long", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_long (DS->key_fingerprint, field7);
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int print_ds_constructor_encrypted_chat_discarded (struct tl_ds_encrypted_chat *DS, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0xb1718213 && T->type->name != 0x4e8e7dec)) { return -1; }
  eprintf (" encryptedChatDiscarded");
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" id :"); }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_int (DS->id, field1);
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int print_ds_constructor_encrypted_file_empty (struct tl_ds_encrypted_file *DS, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x886fd032 && T->type->name != 0x77902fcd)) { return -1; }
  eprintf (" encryptedFileEmpty");
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int print_ds_constructor_encrypted_file (struct tl_ds_encrypted_file *DS, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x886fd032 && T->type->name != 0x77902fcd)) { return -1; }
  eprintf (" encryptedFile");
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" id :"); }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xddf89345, .id = "Bare_Long", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_long (DS->id, field1);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" access_hash :"); }
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xddf89345, .id = "Bare_Long", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_long (DS->access_hash, field2);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" size :"); }
  struct paramed_type *field3 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_int (DS->size, field3);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" dc_id :"); }
  struct paramed_type *field4 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_int (DS->dc_id, field4);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" key_fingerprint :"); }
  struct paramed_type *field5 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_int (DS->key_fingerprint, field5);
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int print_ds_constructor_encrypted_message (struct tl_ds_encrypted_message *DS, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0xce6b8a1e && T->type->name != 0x319475e1)) { return -1; }
  eprintf (" encryptedMessage");
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" random_id :"); }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xddf89345, .id = "Bare_Long", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_long (DS->random_id, field1);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" chat_id :"); }
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_int (DS->chat_id, field2);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" date :"); }
  struct paramed_type *field3 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_int (DS->date, field3);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" bytes :"); }
  struct paramed_type *field4 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xf11ec860, .id = "Bare_Bytes", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_bytes (DS->bytes, field4);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" file :"); }
  struct paramed_type *field5 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x886fd032, .id = "EncryptedFile", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_encrypted_file (DS->file, field5);
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int print_ds_constructor_encrypted_message_service (struct tl_ds_encrypted_message *DS, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0xce6b8a1e && T->type->name != 0x319475e1)) { return -1; }
  eprintf (" encryptedMessageService");
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" random_id :"); }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xddf89345, .id = "Bare_Long", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_long (DS->random_id, field1);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" chat_id :"); }
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_int (DS->chat_id, field2);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" date :"); }
  struct paramed_type *field3 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_int (DS->date, field3);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" bytes :"); }
  struct paramed_type *field4 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xf11ec860, .id = "Bare_Bytes", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_bytes (DS->bytes, field4);
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int print_ds_constructor_error (struct tl_ds_error *DS, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0xc4b9f9bb && T->type->name != 0x3b460644)) { return -1; }
  eprintf (" error");
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" code :"); }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_int (DS->code, field1);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" text :"); }
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_string (DS->text, field2);
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int print_ds_constructor_chat_invite_empty (struct tl_ds_exported_chat_invite *DS, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x95f132d5 && T->type->name != 0x6a0ecd2a)) { return -1; }
  eprintf (" chatInviteEmpty");
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int print_ds_constructor_chat_invite_exported (struct tl_ds_exported_chat_invite *DS, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x95f132d5 && T->type->name != 0x6a0ecd2a)) { return -1; }
  eprintf (" chatInviteExported");
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" link :"); }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_string (DS->link, field1);
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int print_ds_constructor_file_location_unavailable (struct tl_ds_file_location *DS, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x2f8ffb30 && T->type->name != 0xd07004cf)) { return -1; }
  eprintf (" fileLocationUnavailable");
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" volume_id :"); }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xddf89345, .id = "Bare_Long", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_long (DS->volume_id, field1);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" local_id :"); }
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_int (DS->local_id, field2);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" secret :"); }
  struct paramed_type *field3 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xddf89345, .id = "Bare_Long", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_long (DS->secret, field3);
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int print_ds_constructor_file_location (struct tl_ds_file_location *DS, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x2f8ffb30 && T->type->name != 0xd07004cf)) { return -1; }
  eprintf (" fileLocation");
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" dc_id :"); }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_int (DS->dc_id, field1);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" volume_id :"); }
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xddf89345, .id = "Bare_Long", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_long (DS->volume_id, field2);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" local_id :"); }
  struct paramed_type *field3 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_int (DS->local_id, field3);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" secret :"); }
  struct paramed_type *field4 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xddf89345, .id = "Bare_Long", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_long (DS->secret, field4);
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int print_ds_constructor_geo_chat_message_empty (struct tl_ds_geo_chat_message *DS, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0xf67b4034 && T->type->name != 0x0984bfcb)) { return -1; }
  eprintf (" geoChatMessageEmpty");
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" chat_id :"); }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_int (DS->chat_id, field1);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" id :"); }
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_int (DS->id, field2);
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int print_ds_constructor_geo_chat_message (struct tl_ds_geo_chat_message *DS, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0xf67b4034 && T->type->name != 0x0984bfcb)) { return -1; }
  eprintf (" geoChatMessage");
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" chat_id :"); }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_int (DS->chat_id, field1);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" id :"); }
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_int (DS->id, field2);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" from_id :"); }
  struct paramed_type *field3 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_int (DS->from_id, field3);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" date :"); }
  struct paramed_type *field4 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_int (DS->date, field4);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" message :"); }
  struct paramed_type *field5 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_string (DS->message, field5);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" media :"); }
  struct paramed_type *field6 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x95f2471a, .id = "MessageMedia", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_message_media (DS->media, field6);
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int print_ds_constructor_geo_chat_message_service (struct tl_ds_geo_chat_message *DS, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0xf67b4034 && T->type->name != 0x0984bfcb)) { return -1; }
  eprintf (" geoChatMessageService");
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" chat_id :"); }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_int (DS->chat_id, field1);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" id :"); }
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_int (DS->id, field2);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" from_id :"); }
  struct paramed_type *field3 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_int (DS->from_id, field3);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" date :"); }
  struct paramed_type *field4 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_int (DS->date, field4);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" action :"); }
  struct paramed_type *field5 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x383415fa, .id = "MessageAction", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_message_action (DS->action, field5);
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int print_ds_constructor_geo_point_empty (struct tl_ds_geo_point *DS, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x315e0a53 && T->type->name != 0xcea1f5ac)) { return -1; }
  eprintf (" geoPointEmpty");
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int print_ds_constructor_geo_point (struct tl_ds_geo_point *DS, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x315e0a53 && T->type->name != 0xcea1f5ac)) { return -1; }
  eprintf (" geoPoint");
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" longitude :"); }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xddef3eab, .id = "Bare_Double", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_double (DS->longitude, field1);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" latitude :"); }
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xddef3eab, .id = "Bare_Double", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_double (DS->latitude, field2);
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int print_ds_constructor_imported_contact (struct tl_ds_imported_contact *DS, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0xd0028438 && T->type->name != 0x2ffd7bc7)) { return -1; }
  eprintf (" importedContact");
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" user_id :"); }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_int (DS->user_id, field1);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" client_id :"); }
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xddf89345, .id = "Bare_Long", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_long (DS->client_id, field2);
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int print_ds_constructor_input_app_event (struct tl_ds_input_app_event *DS, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x770656a8 && T->type->name != 0x88f9a957)) { return -1; }
  eprintf (" inputAppEvent");
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" time :"); }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xddef3eab, .id = "Bare_Double", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_double (DS->time, field1);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" type :"); }
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_string (DS->type, field2);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" peer :"); }
  struct paramed_type *field3 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xddf89345, .id = "Bare_Long", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_long (DS->peer, field3);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" data :"); }
  struct paramed_type *field4 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_string (DS->data, field4);
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int print_ds_constructor_input_audio_empty (struct tl_ds_input_audio *DS, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0xae8e9c7b && T->type->name != 0x51716384)) { return -1; }
  eprintf (" inputAudioEmpty");
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int print_ds_constructor_input_audio (struct tl_ds_input_audio *DS, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0xae8e9c7b && T->type->name != 0x51716384)) { return -1; }
  eprintf (" inputAudio");
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" id :"); }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xddf89345, .id = "Bare_Long", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_long (DS->id, field1);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" access_hash :"); }
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xddf89345, .id = "Bare_Long", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_long (DS->access_hash, field2);
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int print_ds_constructor_input_chat_photo_empty (struct tl_ds_input_chat_photo *DS, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x3a60776d && T->type->name != 0xc59f8892)) { return -1; }
  eprintf (" inputChatPhotoEmpty");
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int print_ds_constructor_input_chat_uploaded_photo (struct tl_ds_input_chat_photo *DS, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x3a60776d && T->type->name != 0xc59f8892)) { return -1; }
  eprintf (" inputChatUploadedPhoto");
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" file :"); }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x0f60f9ca, .id = "InputFile", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_input_file (DS->file, field1);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" crop :"); }
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x7477e321, .id = "InputPhotoCrop", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_input_photo_crop (DS->crop, field2);
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int print_ds_constructor_input_chat_photo (struct tl_ds_input_chat_photo *DS, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x3a60776d && T->type->name != 0xc59f8892)) { return -1; }
  eprintf (" inputChatPhoto");
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" id :"); }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xe74279c9, .id = "InputPhoto", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_input_photo (DS->id, field1);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" crop :"); }
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x7477e321, .id = "InputPhotoCrop", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_input_photo_crop (DS->crop, field2);
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int print_ds_constructor_input_phone_contact (struct tl_ds_input_contact *DS, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0xf392b7f4 && T->type->name != 0x0c6d480b)) { return -1; }
  eprintf (" inputPhoneContact");
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" client_id :"); }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xddf89345, .id = "Bare_Long", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_long (DS->client_id, field1);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" phone :"); }
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_string (DS->phone, field2);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" first_name :"); }
  struct paramed_type *field3 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_string (DS->first_name, field3);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" last_name :"); }
  struct paramed_type *field4 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_string (DS->last_name, field4);
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int print_ds_constructor_input_document_empty (struct tl_ds_input_document *DS, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x6a8963fc && T->type->name != 0x95769c03)) { return -1; }
  eprintf (" inputDocumentEmpty");
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int print_ds_constructor_input_document (struct tl_ds_input_document *DS, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x6a8963fc && T->type->name != 0x95769c03)) { return -1; }
  eprintf (" inputDocument");
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" id :"); }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xddf89345, .id = "Bare_Long", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_long (DS->id, field1);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" access_hash :"); }
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xddf89345, .id = "Bare_Long", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_long (DS->access_hash, field2);
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int print_ds_constructor_input_encrypted_chat (struct tl_ds_input_encrypted_chat *DS, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0xf141b5e1 && T->type->name != 0x0ebe4a1e)) { return -1; }
  eprintf (" inputEncryptedChat");
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" chat_id :"); }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_int (DS->chat_id, field1);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" access_hash :"); }
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xddf89345, .id = "Bare_Long", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_long (DS->access_hash, field2);
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int print_ds_constructor_input_encrypted_file_empty (struct tl_ds_input_encrypted_file *DS, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x0b5c064f && T->type->name != 0xf4a3f9b0)) { return -1; }
  eprintf (" inputEncryptedFileEmpty");
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int print_ds_constructor_input_encrypted_file_uploaded (struct tl_ds_input_encrypted_file *DS, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x0b5c064f && T->type->name != 0xf4a3f9b0)) { return -1; }
  eprintf (" inputEncryptedFileUploaded");
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" id :"); }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xddf89345, .id = "Bare_Long", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_long (DS->id, field1);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" parts :"); }
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_int (DS->parts, field2);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" md5_checksum :"); }
  struct paramed_type *field3 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_string (DS->md5_checksum, field3);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" key_fingerprint :"); }
  struct paramed_type *field4 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_int (DS->key_fingerprint, field4);
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int print_ds_constructor_input_encrypted_file (struct tl_ds_input_encrypted_file *DS, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x0b5c064f && T->type->name != 0xf4a3f9b0)) { return -1; }
  eprintf (" inputEncryptedFile");
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" id :"); }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xddf89345, .id = "Bare_Long", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_long (DS->id, field1);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" access_hash :"); }
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xddf89345, .id = "Bare_Long", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_long (DS->access_hash, field2);
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int print_ds_constructor_input_encrypted_file_big_uploaded (struct tl_ds_input_encrypted_file *DS, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x0b5c064f && T->type->name != 0xf4a3f9b0)) { return -1; }
  eprintf (" inputEncryptedFileBigUploaded");
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" id :"); }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xddf89345, .id = "Bare_Long", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_long (DS->id, field1);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" parts :"); }
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_int (DS->parts, field2);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" key_fingerprint :"); }
  struct paramed_type *field3 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_int (DS->key_fingerprint, field3);
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int print_ds_constructor_input_file (struct tl_ds_input_file *DS, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x0f60f9ca && T->type->name != 0xf09f0635)) { return -1; }
  eprintf (" inputFile");
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" id :"); }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xddf89345, .id = "Bare_Long", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_long (DS->id, field1);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" parts :"); }
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_int (DS->parts, field2);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" name :"); }
  struct paramed_type *field3 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_string (DS->name, field3);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" md5_checksum :"); }
  struct paramed_type *field4 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_string (DS->md5_checksum, field4);
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int print_ds_constructor_input_file_big (struct tl_ds_input_file *DS, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x0f60f9ca && T->type->name != 0xf09f0635)) { return -1; }
  eprintf (" inputFileBig");
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" id :"); }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xddf89345, .id = "Bare_Long", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_long (DS->id, field1);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" parts :"); }
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_int (DS->parts, field2);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" name :"); }
  struct paramed_type *field3 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_string (DS->name, field3);
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int print_ds_constructor_input_file_location (struct tl_ds_input_file_location *DS, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0xe6daa38b && T->type->name != 0x19255c74)) { return -1; }
  eprintf (" inputFileLocation");
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" volume_id :"); }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xddf89345, .id = "Bare_Long", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_long (DS->volume_id, field1);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" local_id :"); }
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_int (DS->local_id, field2);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" secret :"); }
  struct paramed_type *field3 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xddf89345, .id = "Bare_Long", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_long (DS->secret, field3);
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int print_ds_constructor_input_video_file_location (struct tl_ds_input_file_location *DS, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0xe6daa38b && T->type->name != 0x19255c74)) { return -1; }
  eprintf (" inputVideoFileLocation");
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" id :"); }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xddf89345, .id = "Bare_Long", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_long (DS->id, field1);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" access_hash :"); }
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xddf89345, .id = "Bare_Long", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_long (DS->access_hash, field2);
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int print_ds_constructor_input_encrypted_file_location (struct tl_ds_input_file_location *DS, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0xe6daa38b && T->type->name != 0x19255c74)) { return -1; }
  eprintf (" inputEncryptedFileLocation");
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" id :"); }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xddf89345, .id = "Bare_Long", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_long (DS->id, field1);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" access_hash :"); }
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xddf89345, .id = "Bare_Long", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_long (DS->access_hash, field2);
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int print_ds_constructor_input_audio_file_location (struct tl_ds_input_file_location *DS, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0xe6daa38b && T->type->name != 0x19255c74)) { return -1; }
  eprintf (" inputAudioFileLocation");
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" id :"); }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xddf89345, .id = "Bare_Long", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_long (DS->id, field1);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" access_hash :"); }
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xddf89345, .id = "Bare_Long", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_long (DS->access_hash, field2);
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int print_ds_constructor_input_document_file_location (struct tl_ds_input_file_location *DS, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0xe6daa38b && T->type->name != 0x19255c74)) { return -1; }
  eprintf (" inputDocumentFileLocation");
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" id :"); }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xddf89345, .id = "Bare_Long", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_long (DS->id, field1);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" access_hash :"); }
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xddf89345, .id = "Bare_Long", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_long (DS->access_hash, field2);
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int print_ds_constructor_input_geo_chat (struct tl_ds_input_geo_chat *DS, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x74d456fa && T->type->name != 0x8b2ba905)) { return -1; }
  eprintf (" inputGeoChat");
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" chat_id :"); }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_int (DS->chat_id, field1);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" access_hash :"); }
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xddf89345, .id = "Bare_Long", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_long (DS->access_hash, field2);
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int print_ds_constructor_input_geo_point_empty (struct tl_ds_input_geo_point *DS, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x17768f1f && T->type->name != 0xe88970e0)) { return -1; }
  eprintf (" inputGeoPointEmpty");
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int print_ds_constructor_input_geo_point (struct tl_ds_input_geo_point *DS, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x17768f1f && T->type->name != 0xe88970e0)) { return -1; }
  eprintf (" inputGeoPoint");
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" latitude :"); }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xddef3eab, .id = "Bare_Double", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_double (DS->latitude, field1);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" longitude :"); }
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xddef3eab, .id = "Bare_Double", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_double (DS->longitude, field2);
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int print_ds_constructor_input_media_empty (struct tl_ds_input_media *DS, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0xb32c91dd && T->type->name != 0x4cd36e22)) { return -1; }
  eprintf (" inputMediaEmpty");
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int print_ds_constructor_input_media_uploaded_photo (struct tl_ds_input_media *DS, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0xb32c91dd && T->type->name != 0x4cd36e22)) { return -1; }
  eprintf (" inputMediaUploadedPhoto");
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" file :"); }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x0f60f9ca, .id = "InputFile", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_input_file (DS->file, field1);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" caption :"); }
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_string (DS->caption, field2);
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int print_ds_constructor_input_media_photo (struct tl_ds_input_media *DS, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0xb32c91dd && T->type->name != 0x4cd36e22)) { return -1; }
  eprintf (" inputMediaPhoto");
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" id :"); }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xe74279c9, .id = "InputPhoto", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_input_photo (DS->id, field1);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" caption :"); }
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_string (DS->caption, field2);
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int print_ds_constructor_input_media_geo_point (struct tl_ds_input_media *DS, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0xb32c91dd && T->type->name != 0x4cd36e22)) { return -1; }
  eprintf (" inputMediaGeoPoint");
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" geo_point :"); }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x17768f1f, .id = "InputGeoPoint", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_input_geo_point (DS->geo_point, field1);
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int print_ds_constructor_input_media_contact (struct tl_ds_input_media *DS, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0xb32c91dd && T->type->name != 0x4cd36e22)) { return -1; }
  eprintf (" inputMediaContact");
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" phone_number :"); }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_string (DS->phone_number, field1);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" first_name :"); }
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_string (DS->first_name, field2);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" last_name :"); }
  struct paramed_type *field3 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_string (DS->last_name, field3);
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int print_ds_constructor_input_media_uploaded_video (struct tl_ds_input_media *DS, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0xb32c91dd && T->type->name != 0x4cd36e22)) { return -1; }
  eprintf (" inputMediaUploadedVideo");
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" file :"); }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x0f60f9ca, .id = "InputFile", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_input_file (DS->file, field1);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" duration :"); }
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_int (DS->duration, field2);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" w :"); }
  struct paramed_type *field3 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_int (DS->w, field3);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" h :"); }
  struct paramed_type *field4 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_int (DS->h, field4);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" caption :"); }
  struct paramed_type *field5 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_string (DS->caption, field5);
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int print_ds_constructor_input_media_uploaded_thumb_video (struct tl_ds_input_media *DS, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0xb32c91dd && T->type->name != 0x4cd36e22)) { return -1; }
  eprintf (" inputMediaUploadedThumbVideo");
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" file :"); }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x0f60f9ca, .id = "InputFile", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_input_file (DS->file, field1);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" thumb :"); }
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x0f60f9ca, .id = "InputFile", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_input_file (DS->thumb, field2);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" duration :"); }
  struct paramed_type *field3 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_int (DS->duration, field3);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" w :"); }
  struct paramed_type *field4 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_int (DS->w, field4);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" h :"); }
  struct paramed_type *field5 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_int (DS->h, field5);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" caption :"); }
  struct paramed_type *field6 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_string (DS->caption, field6);
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int print_ds_constructor_input_media_video (struct tl_ds_input_media *DS, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0xb32c91dd && T->type->name != 0x4cd36e22)) { return -1; }
  eprintf (" inputMediaVideo");
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" video_id :"); }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xbb5f7a27, .id = "InputVideo", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_input_video (DS->video_id, field1);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" caption :"); }
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_string (DS->caption, field2);
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int print_ds_constructor_input_media_uploaded_audio (struct tl_ds_input_media *DS, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0xb32c91dd && T->type->name != 0x4cd36e22)) { return -1; }
  eprintf (" inputMediaUploadedAudio");
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" file :"); }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x0f60f9ca, .id = "InputFile", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_input_file (DS->file, field1);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" duration :"); }
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_int (DS->duration, field2);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" mime_type :"); }
  struct paramed_type *field3 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_string (DS->mime_type, field3);
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int print_ds_constructor_input_media_audio (struct tl_ds_input_media *DS, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0xb32c91dd && T->type->name != 0x4cd36e22)) { return -1; }
  eprintf (" inputMediaAudio");
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" audio_id :"); }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xae8e9c7b, .id = "InputAudio", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_input_audio (DS->audio_id, field1);
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int print_ds_constructor_input_media_uploaded_document (struct tl_ds_input_media *DS, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0xb32c91dd && T->type->name != 0x4cd36e22)) { return -1; }
  eprintf (" inputMediaUploadedDocument");
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" file :"); }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x0f60f9ca, .id = "InputFile", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_input_file (DS->file, field1);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" mime_type :"); }
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_string (DS->mime_type, field2);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" attributes :"); }
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
  print_ds_type_vector ((void *)DS->attributes, field3);
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int print_ds_constructor_input_media_uploaded_thumb_document (struct tl_ds_input_media *DS, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0xb32c91dd && T->type->name != 0x4cd36e22)) { return -1; }
  eprintf (" inputMediaUploadedThumbDocument");
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" file :"); }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x0f60f9ca, .id = "InputFile", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_input_file (DS->file, field1);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" thumb :"); }
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x0f60f9ca, .id = "InputFile", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_input_file (DS->thumb, field2);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" mime_type :"); }
  struct paramed_type *field3 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_string (DS->mime_type, field3);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" attributes :"); }
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
  print_ds_type_vector ((void *)DS->attributes, field4);
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int print_ds_constructor_input_media_document (struct tl_ds_input_media *DS, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0xb32c91dd && T->type->name != 0x4cd36e22)) { return -1; }
  eprintf (" inputMediaDocument");
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" document_id :"); }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x6a8963fc, .id = "InputDocument", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_input_document (DS->document_id, field1);
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int print_ds_constructor_input_media_venue (struct tl_ds_input_media *DS, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0xb32c91dd && T->type->name != 0x4cd36e22)) { return -1; }
  eprintf (" inputMediaVenue");
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" geo_point :"); }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x17768f1f, .id = "InputGeoPoint", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_input_geo_point (DS->geo_point, field1);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" title :"); }
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_string (DS->title, field2);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" address :"); }
  struct paramed_type *field3 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_string (DS->address, field3);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" provider :"); }
  struct paramed_type *field4 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_string (DS->provider, field4);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" venue_id :"); }
  struct paramed_type *field5 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_string (DS->venue_id, field5);
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int print_ds_constructor_input_notify_peer (struct tl_ds_input_notify_peer *DS, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x02b6911b && T->type->name != 0xfd496ee4)) { return -1; }
  eprintf (" inputNotifyPeer");
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" peer :"); }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x9e67268d, .id = "InputPeer", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_input_peer (DS->peer, field1);
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int print_ds_constructor_input_notify_users (struct tl_ds_input_notify_peer *DS, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x02b6911b && T->type->name != 0xfd496ee4)) { return -1; }
  eprintf (" inputNotifyUsers");
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int print_ds_constructor_input_notify_chats (struct tl_ds_input_notify_peer *DS, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x02b6911b && T->type->name != 0xfd496ee4)) { return -1; }
  eprintf (" inputNotifyChats");
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int print_ds_constructor_input_notify_all (struct tl_ds_input_notify_peer *DS, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x02b6911b && T->type->name != 0xfd496ee4)) { return -1; }
  eprintf (" inputNotifyAll");
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int print_ds_constructor_input_notify_geo_chat_peer (struct tl_ds_input_notify_peer *DS, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x02b6911b && T->type->name != 0xfd496ee4)) { return -1; }
  eprintf (" inputNotifyGeoChatPeer");
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" geo_peer :"); }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x74d456fa, .id = "InputGeoChat", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_input_geo_chat (DS->geo_peer, field1);
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int print_ds_constructor_input_peer_empty (struct tl_ds_input_peer *DS, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x9e67268d && T->type->name != 0x6198d972)) { return -1; }
  eprintf (" inputPeerEmpty");
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int print_ds_constructor_input_peer_self (struct tl_ds_input_peer *DS, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x9e67268d && T->type->name != 0x6198d972)) { return -1; }
  eprintf (" inputPeerSelf");
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int print_ds_constructor_input_peer_contact (struct tl_ds_input_peer *DS, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x9e67268d && T->type->name != 0x6198d972)) { return -1; }
  eprintf (" inputPeerContact");
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" user_id :"); }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_int (DS->user_id, field1);
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int print_ds_constructor_input_peer_foreign (struct tl_ds_input_peer *DS, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x9e67268d && T->type->name != 0x6198d972)) { return -1; }
  eprintf (" inputPeerForeign");
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" user_id :"); }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_int (DS->user_id, field1);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" access_hash :"); }
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xddf89345, .id = "Bare_Long", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_long (DS->access_hash, field2);
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int print_ds_constructor_input_peer_chat (struct tl_ds_input_peer *DS, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x9e67268d && T->type->name != 0x6198d972)) { return -1; }
  eprintf (" inputPeerChat");
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" chat_id :"); }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_int (DS->chat_id, field1);
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int print_ds_constructor_input_peer_notify_events_empty (struct tl_ds_input_peer_notify_events *DS, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x185a48ac && T->type->name != 0xe7a5b753)) { return -1; }
  eprintf (" inputPeerNotifyEventsEmpty");
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int print_ds_constructor_input_peer_notify_events_all (struct tl_ds_input_peer_notify_events *DS, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x185a48ac && T->type->name != 0xe7a5b753)) { return -1; }
  eprintf (" inputPeerNotifyEventsAll");
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int print_ds_constructor_input_peer_notify_settings (struct tl_ds_input_peer_notify_settings *DS, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x46a2ce98 && T->type->name != 0xb95d3167)) { return -1; }
  eprintf (" inputPeerNotifySettings");
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" mute_until :"); }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_int (DS->mute_until, field1);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" sound :"); }
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_string (DS->sound, field2);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" show_previews :"); }
  struct paramed_type *field3 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x250be282, .id = "Bool", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bool (DS->show_previews, field3);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" events_mask :"); }
  struct paramed_type *field4 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_int (DS->events_mask, field4);
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int print_ds_constructor_input_photo_empty (struct tl_ds_input_photo *DS, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0xe74279c9 && T->type->name != 0x18bd8636)) { return -1; }
  eprintf (" inputPhotoEmpty");
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int print_ds_constructor_input_photo (struct tl_ds_input_photo *DS, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0xe74279c9 && T->type->name != 0x18bd8636)) { return -1; }
  eprintf (" inputPhoto");
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" id :"); }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xddf89345, .id = "Bare_Long", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_long (DS->id, field1);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" access_hash :"); }
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xddf89345, .id = "Bare_Long", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_long (DS->access_hash, field2);
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int print_ds_constructor_input_photo_crop_auto (struct tl_ds_input_photo_crop *DS, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x7477e321 && T->type->name != 0x8b881cde)) { return -1; }
  eprintf (" inputPhotoCropAuto");
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int print_ds_constructor_input_photo_crop (struct tl_ds_input_photo_crop *DS, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x7477e321 && T->type->name != 0x8b881cde)) { return -1; }
  eprintf (" inputPhotoCrop");
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" crop_left :"); }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xddef3eab, .id = "Bare_Double", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_double (DS->crop_left, field1);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" crop_top :"); }
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xddef3eab, .id = "Bare_Double", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_double (DS->crop_top, field2);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" crop_width :"); }
  struct paramed_type *field3 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xddef3eab, .id = "Bare_Double", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_double (DS->crop_width, field3);
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int print_ds_constructor_input_privacy_key_status_timestamp (struct tl_ds_input_privacy_key *DS, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x4f96cb18 && T->type->name != 0xb06934e7)) { return -1; }
  eprintf (" inputPrivacyKeyStatusTimestamp");
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int print_ds_constructor_input_privacy_value_allow_contacts (struct tl_ds_input_privacy_rule *DS, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x4b815163 && T->type->name != 0xb47eae9c)) { return -1; }
  eprintf (" inputPrivacyValueAllowContacts");
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int print_ds_constructor_input_privacy_value_allow_all (struct tl_ds_input_privacy_rule *DS, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x4b815163 && T->type->name != 0xb47eae9c)) { return -1; }
  eprintf (" inputPrivacyValueAllowAll");
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int print_ds_constructor_input_privacy_value_allow_users (struct tl_ds_input_privacy_rule *DS, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x4b815163 && T->type->name != 0xb47eae9c)) { return -1; }
  eprintf (" inputPrivacyValueAllowUsers");
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" users :"); }
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
  print_ds_type_vector ((void *)DS->users, field1);
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int print_ds_constructor_input_privacy_value_disallow_contacts (struct tl_ds_input_privacy_rule *DS, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x4b815163 && T->type->name != 0xb47eae9c)) { return -1; }
  eprintf (" inputPrivacyValueDisallowContacts");
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int print_ds_constructor_input_privacy_value_disallow_all (struct tl_ds_input_privacy_rule *DS, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x4b815163 && T->type->name != 0xb47eae9c)) { return -1; }
  eprintf (" inputPrivacyValueDisallowAll");
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int print_ds_constructor_input_privacy_value_disallow_users (struct tl_ds_input_privacy_rule *DS, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x4b815163 && T->type->name != 0xb47eae9c)) { return -1; }
  eprintf (" inputPrivacyValueDisallowUsers");
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" users :"); }
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
  print_ds_type_vector ((void *)DS->users, field1);
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int print_ds_constructor_input_sticker_set_empty (struct tl_ds_input_sticker_set *DS, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0xe44d415c && T->type->name != 0x1bb2bea3)) { return -1; }
  eprintf (" inputStickerSetEmpty");
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int print_ds_constructor_input_sticker_set_i_d (struct tl_ds_input_sticker_set *DS, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0xe44d415c && T->type->name != 0x1bb2bea3)) { return -1; }
  eprintf (" inputStickerSetID");
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" id :"); }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xddf89345, .id = "Bare_Long", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_long (DS->id, field1);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" access_hash :"); }
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xddf89345, .id = "Bare_Long", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_long (DS->access_hash, field2);
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int print_ds_constructor_input_sticker_set_short_name (struct tl_ds_input_sticker_set *DS, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0xe44d415c && T->type->name != 0x1bb2bea3)) { return -1; }
  eprintf (" inputStickerSetShortName");
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" short_name :"); }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_string (DS->short_name, field1);
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int print_ds_constructor_input_user_empty (struct tl_ds_input_user *DS, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0xadfe0c6a && T->type->name != 0x5201f395)) { return -1; }
  eprintf (" inputUserEmpty");
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int print_ds_constructor_input_user_self (struct tl_ds_input_user *DS, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0xadfe0c6a && T->type->name != 0x5201f395)) { return -1; }
  eprintf (" inputUserSelf");
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int print_ds_constructor_input_user_contact (struct tl_ds_input_user *DS, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0xadfe0c6a && T->type->name != 0x5201f395)) { return -1; }
  eprintf (" inputUserContact");
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" user_id :"); }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_int (DS->user_id, field1);
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int print_ds_constructor_input_user_foreign (struct tl_ds_input_user *DS, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0xadfe0c6a && T->type->name != 0x5201f395)) { return -1; }
  eprintf (" inputUserForeign");
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" user_id :"); }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_int (DS->user_id, field1);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" access_hash :"); }
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xddf89345, .id = "Bare_Long", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_long (DS->access_hash, field2);
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int print_ds_constructor_input_video_empty (struct tl_ds_input_video *DS, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0xbb5f7a27 && T->type->name != 0x44a085d8)) { return -1; }
  eprintf (" inputVideoEmpty");
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int print_ds_constructor_input_video (struct tl_ds_input_video *DS, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0xbb5f7a27 && T->type->name != 0x44a085d8)) { return -1; }
  eprintf (" inputVideo");
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" id :"); }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xddf89345, .id = "Bare_Long", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_long (DS->id, field1);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" access_hash :"); }
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xddf89345, .id = "Bare_Long", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_long (DS->access_hash, field2);
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int print_ds_constructor_int (int *DS, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0xa8509bda && T->type->name != 0x57af6425)) { return -1; }
  eprintf (" %d", *DS);
  return 0;
}
int print_ds_constructor_int128 (struct tl_ds_int128 *DS, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x7d36c439 && T->type->name != 0x82c93bc6)) { return -1; }
  eprintf (" int128");
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xddf89345, .id = "Bare_Long", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_long (DS->f0, field1);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xddf89345, .id = "Bare_Long", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_long (DS->f1, field2);
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int print_ds_constructor_int256 (struct tl_ds_int256 *DS, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0xf2c798b3 && T->type->name != 0x0d38674c)) { return -1; }
  eprintf (" int256");
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xddf89345, .id = "Bare_Long", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_long (DS->f0, field1);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xddf89345, .id = "Bare_Long", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_long (DS->f1, field2);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  struct paramed_type *field3 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xddf89345, .id = "Bare_Long", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_long (DS->f2, field3);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  struct paramed_type *field4 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xddf89345, .id = "Bare_Long", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_long (DS->f3, field4);
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int print_ds_constructor_keyboard_button (struct tl_ds_keyboard_button *DS, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0xa2fa4880 && T->type->name != 0x5d05b77f)) { return -1; }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_string (DS->text, field1);
  return 0;
}
int print_ds_constructor_keyboard_button_row (struct tl_ds_keyboard_button_row *DS, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x77608b83 && T->type->name != 0x889f747c)) { return -1; }
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
  print_ds_type_vector ((void *)DS->buttons, field1);
  return 0;
}
int print_ds_constructor_long (long long *DS, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x22076cba && T->type->name != 0xddf89345)) { return -1; }
  eprintf (" %I64d", *DS);
  return 0;
}
int print_ds_constructor_message_empty (struct tl_ds_message *DS, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x5d652a7f && T->type->name != 0xa29ad580)) { return -1; }
  eprintf (" messageEmpty");
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" id :"); }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_int (DS->id, field1);
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int print_ds_constructor_message (struct tl_ds_message *DS, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x5d652a7f && T->type->name != 0xa29ad580)) { return -1; }
  eprintf (" message");
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" flags :"); }
  struct paramed_type *var0 = INT2PTR (*DS->flags);
  eprintf (" %d", (int)PTR2INT (var0));
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" id :"); }
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_int (DS->id, field2);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" from_id :"); }
  struct paramed_type *field3 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_int (DS->from_id, field3);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" to_id :"); }
  struct paramed_type *field4 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x276159d6, .id = "Peer", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_peer (DS->to_id, field4);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (PTR2INT (var0) & (1 << 2)) {
    if (multiline_output >= 2) { print_offset (); }
    if (!disable_field_names) { eprintf (" fwd_from_id :"); }
    struct paramed_type *field5 = 
    &(struct paramed_type){
      .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
      .params = 0,
    };
    print_ds_type_bare_int (DS->fwd_from_id, field5);
    if (multiline_output >= 2) { eprintf ("\n"); }
  }
  if (PTR2INT (var0) & (1 << 2)) {
    if (multiline_output >= 2) { print_offset (); }
    if (!disable_field_names) { eprintf (" fwd_date :"); }
    struct paramed_type *field6 = 
    &(struct paramed_type){
      .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
      .params = 0,
    };
    print_ds_type_bare_int (DS->fwd_date, field6);
    if (multiline_output >= 2) { eprintf ("\n"); }
  }
  if (PTR2INT (var0) & (1 << 3)) {
    if (multiline_output >= 2) { print_offset (); }
    if (!disable_field_names) { eprintf (" reply_to_msg_id :"); }
    struct paramed_type *field7 = 
    &(struct paramed_type){
      .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
      .params = 0,
    };
    print_ds_type_bare_int (DS->reply_to_msg_id, field7);
    if (multiline_output >= 2) { eprintf ("\n"); }
  }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" date :"); }
  struct paramed_type *field8 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_int (DS->date, field8);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" message :"); }
  struct paramed_type *field9 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_string (DS->message, field9);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" media :"); }
  struct paramed_type *field10 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x95f2471a, .id = "MessageMedia", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_message_media (DS->media, field10);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (PTR2INT (var0) & (1 << 6)) {
    if (multiline_output >= 2) { print_offset (); }
    if (!disable_field_names) { eprintf (" reply_markup :"); }
    struct paramed_type *field11 = 
    &(struct paramed_type){
      .type = &(struct tl_type_descr) {.name = 0x612ca4a9, .id = "ReplyMarkup", .params_num = 0, .params_types = 0},
      .params = 0,
    };
    print_ds_type_reply_markup (DS->reply_markup, field11);
    if (multiline_output >= 2) { eprintf ("\n"); }
  }
  return 0;
}
int print_ds_constructor_message_service (struct tl_ds_message *DS, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x5d652a7f && T->type->name != 0xa29ad580)) { return -1; }
  eprintf (" messageService");
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" flags :"); }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_int (DS->flags, field1);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" id :"); }
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_int (DS->id, field2);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" from_id :"); }
  struct paramed_type *field3 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_int (DS->from_id, field3);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" to_id :"); }
  struct paramed_type *field4 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x276159d6, .id = "Peer", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_peer (DS->to_id, field4);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" date :"); }
  struct paramed_type *field5 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_int (DS->date, field5);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" action :"); }
  struct paramed_type *field6 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x383415fa, .id = "MessageAction", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_message_action (DS->action, field6);
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int print_ds_constructor_message_action_empty (struct tl_ds_message_action *DS, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x383415fa && T->type->name != 0xc7cbea05)) { return -1; }
  eprintf (" messageActionEmpty");
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int print_ds_constructor_message_action_chat_create (struct tl_ds_message_action *DS, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x383415fa && T->type->name != 0xc7cbea05)) { return -1; }
  eprintf (" messageActionChatCreate");
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" title :"); }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_string (DS->title, field1);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" users :"); }
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
  print_ds_type_vector ((void *)DS->users, field2);
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int print_ds_constructor_message_action_chat_edit_title (struct tl_ds_message_action *DS, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x383415fa && T->type->name != 0xc7cbea05)) { return -1; }
  eprintf (" messageActionChatEditTitle");
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" title :"); }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_string (DS->title, field1);
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int print_ds_constructor_message_action_chat_edit_photo (struct tl_ds_message_action *DS, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x383415fa && T->type->name != 0xc7cbea05)) { return -1; }
  eprintf (" messageActionChatEditPhoto");
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" photo :"); }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xc207550a, .id = "Photo", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_photo (DS->photo, field1);
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int print_ds_constructor_message_action_chat_delete_photo (struct tl_ds_message_action *DS, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x383415fa && T->type->name != 0xc7cbea05)) { return -1; }
  eprintf (" messageActionChatDeletePhoto");
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int print_ds_constructor_message_action_chat_add_user (struct tl_ds_message_action *DS, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x383415fa && T->type->name != 0xc7cbea05)) { return -1; }
  eprintf (" messageActionChatAddUser");
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" user_id :"); }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_int (DS->user_id, field1);
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int print_ds_constructor_message_action_chat_delete_user (struct tl_ds_message_action *DS, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x383415fa && T->type->name != 0xc7cbea05)) { return -1; }
  eprintf (" messageActionChatDeleteUser");
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" user_id :"); }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_int (DS->user_id, field1);
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int print_ds_constructor_message_action_geo_chat_create (struct tl_ds_message_action *DS, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x383415fa && T->type->name != 0xc7cbea05)) { return -1; }
  eprintf (" messageActionGeoChatCreate");
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" title :"); }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_string (DS->title, field1);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" address :"); }
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_string (DS->address, field2);
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int print_ds_constructor_message_action_geo_chat_checkin (struct tl_ds_message_action *DS, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x383415fa && T->type->name != 0xc7cbea05)) { return -1; }
  eprintf (" messageActionGeoChatCheckin");
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int print_ds_constructor_message_action_chat_joined_by_link (struct tl_ds_message_action *DS, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x383415fa && T->type->name != 0xc7cbea05)) { return -1; }
  eprintf (" messageActionChatJoinedByLink");
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" inviter_id :"); }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_int (DS->inviter_id, field1);
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int print_ds_constructor_message_media_empty (struct tl_ds_message_media *DS, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x95f2471a && T->type->name != 0x6a0db8e5)) { return -1; }
  eprintf (" messageMediaEmpty");
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int print_ds_constructor_message_media_photo (struct tl_ds_message_media *DS, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x95f2471a && T->type->name != 0x6a0db8e5)) { return -1; }
  eprintf (" messageMediaPhoto");
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" photo :"); }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xc207550a, .id = "Photo", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_photo (DS->photo, field1);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" caption :"); }
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_string (DS->caption, field2);
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int print_ds_constructor_message_media_video (struct tl_ds_message_media *DS, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x95f2471a && T->type->name != 0x6a0db8e5)) { return -1; }
  eprintf (" messageMediaVideo");
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" video :"); }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x1716b174, .id = "Video", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_video (DS->video, field1);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" caption :"); }
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_string (DS->caption, field2);
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int print_ds_constructor_message_media_geo (struct tl_ds_message_media *DS, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x95f2471a && T->type->name != 0x6a0db8e5)) { return -1; }
  eprintf (" messageMediaGeo");
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" geo :"); }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x315e0a53, .id = "GeoPoint", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_geo_point (DS->geo, field1);
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int print_ds_constructor_message_media_contact (struct tl_ds_message_media *DS, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x95f2471a && T->type->name != 0x6a0db8e5)) { return -1; }
  eprintf (" messageMediaContact");
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" phone_number :"); }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_string (DS->phone_number, field1);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" first_name :"); }
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_string (DS->first_name, field2);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" last_name :"); }
  struct paramed_type *field3 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_string (DS->last_name, field3);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" user_id :"); }
  struct paramed_type *field4 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_int (DS->user_id, field4);
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int print_ds_constructor_message_media_unsupported (struct tl_ds_message_media *DS, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x95f2471a && T->type->name != 0x6a0db8e5)) { return -1; }
  eprintf (" messageMediaUnsupported");
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int print_ds_constructor_message_media_document (struct tl_ds_message_media *DS, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x95f2471a && T->type->name != 0x6a0db8e5)) { return -1; }
  eprintf (" messageMediaDocument");
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" document :"); }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x51a73418, .id = "Document", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_document (DS->document, field1);
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int print_ds_constructor_message_media_audio (struct tl_ds_message_media *DS, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x95f2471a && T->type->name != 0x6a0db8e5)) { return -1; }
  eprintf (" messageMediaAudio");
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" audio :"); }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x9fc5ec4e, .id = "Audio", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_audio (DS->audio, field1);
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int print_ds_constructor_message_media_web_page (struct tl_ds_message_media *DS, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x95f2471a && T->type->name != 0x6a0db8e5)) { return -1; }
  eprintf (" messageMediaWebPage");
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" webpage :"); }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x8d8c0d41, .id = "WebPage", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_web_page (DS->webpage, field1);
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int print_ds_constructor_message_media_venue (struct tl_ds_message_media *DS, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x95f2471a && T->type->name != 0x6a0db8e5)) { return -1; }
  eprintf (" messageMediaVenue");
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" geo :"); }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x315e0a53, .id = "GeoPoint", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_geo_point (DS->geo, field1);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" title :"); }
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_string (DS->title, field2);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" address :"); }
  struct paramed_type *field3 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_string (DS->address, field3);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" provider :"); }
  struct paramed_type *field4 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_string (DS->provider, field4);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" venue_id :"); }
  struct paramed_type *field5 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_string (DS->venue_id, field5);
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int print_ds_constructor_message_media_photo_l27 (struct tl_ds_message_media *DS, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x95f2471a && T->type->name != 0x6a0db8e5)) { return -1; }
  eprintf (" messageMediaPhotoL27");
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" photo :"); }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xc207550a, .id = "Photo", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_photo (DS->photo, field1);
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int print_ds_constructor_message_media_video_l27 (struct tl_ds_message_media *DS, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x95f2471a && T->type->name != 0x6a0db8e5)) { return -1; }
  eprintf (" messageMediaVideoL27");
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" video :"); }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x1716b174, .id = "Video", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_video (DS->video, field1);
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int print_ds_constructor_input_messages_filter_empty (struct tl_ds_messages_filter *DS, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x80895ae0 && T->type->name != 0x7f76a51f)) { return -1; }
  eprintf (" inputMessagesFilterEmpty");
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int print_ds_constructor_input_messages_filter_photos (struct tl_ds_messages_filter *DS, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x80895ae0 && T->type->name != 0x7f76a51f)) { return -1; }
  eprintf (" inputMessagesFilterPhotos");
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int print_ds_constructor_input_messages_filter_video (struct tl_ds_messages_filter *DS, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x80895ae0 && T->type->name != 0x7f76a51f)) { return -1; }
  eprintf (" inputMessagesFilterVideo");
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int print_ds_constructor_input_messages_filter_photo_video (struct tl_ds_messages_filter *DS, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x80895ae0 && T->type->name != 0x7f76a51f)) { return -1; }
  eprintf (" inputMessagesFilterPhotoVideo");
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int print_ds_constructor_input_messages_filter_photo_video_documents (struct tl_ds_messages_filter *DS, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x80895ae0 && T->type->name != 0x7f76a51f)) { return -1; }
  eprintf (" inputMessagesFilterPhotoVideoDocuments");
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int print_ds_constructor_input_messages_filter_document (struct tl_ds_messages_filter *DS, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x80895ae0 && T->type->name != 0x7f76a51f)) { return -1; }
  eprintf (" inputMessagesFilterDocument");
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int print_ds_constructor_input_messages_filter_audio (struct tl_ds_messages_filter *DS, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x80895ae0 && T->type->name != 0x7f76a51f)) { return -1; }
  eprintf (" inputMessagesFilterAudio");
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int print_ds_constructor_nearest_dc (struct tl_ds_nearest_dc *DS, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x8e1a1775 && T->type->name != 0x71e5e88a)) { return -1; }
  eprintf (" nearestDc");
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" country :"); }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_string (DS->country, field1);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" this_dc :"); }
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_int (DS->this_dc, field2);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" nearest_dc :"); }
  struct paramed_type *field3 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_int (DS->nearest_dc, field3);
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int print_ds_constructor_notify_peer (struct tl_ds_notify_peer *DS, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x9fcb8237 && T->type->name != 0x60347dc8)) { return -1; }
  eprintf (" notifyPeer");
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" peer :"); }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x276159d6, .id = "Peer", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_peer (DS->peer, field1);
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int print_ds_constructor_notify_users (struct tl_ds_notify_peer *DS, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x9fcb8237 && T->type->name != 0x60347dc8)) { return -1; }
  eprintf (" notifyUsers");
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int print_ds_constructor_notify_chats (struct tl_ds_notify_peer *DS, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x9fcb8237 && T->type->name != 0x60347dc8)) { return -1; }
  eprintf (" notifyChats");
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int print_ds_constructor_notify_all (struct tl_ds_notify_peer *DS, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x9fcb8237 && T->type->name != 0x60347dc8)) { return -1; }
  eprintf (" notifyAll");
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int print_ds_constructor_null (struct tl_ds_null *DS, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x56730bcc && T->type->name != 0xa98cf433)) { return -1; }
  eprintf (" null");
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int print_ds_constructor_p_q_inner_data (struct tl_ds_p_q_inner_data *DS, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0xbfa3de38 && T->type->name != 0x405c21c7)) { return -1; }
  eprintf (" p_q_inner_data");
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" pq :"); }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_string (DS->pq, field1);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" p :"); }
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_string (DS->p, field2);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" q :"); }
  struct paramed_type *field3 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_string (DS->q, field3);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" nonce :"); }
  struct paramed_type *field4 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x82c93bc6, .id = "Bare_Int128", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_int128 (DS->nonce, field4);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" server_nonce :"); }
  struct paramed_type *field5 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x82c93bc6, .id = "Bare_Int128", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_int128 (DS->server_nonce, field5);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" new_nonce :"); }
  struct paramed_type *field6 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x0d38674c, .id = "Bare_Int256", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_int256 (DS->new_nonce, field6);
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int print_ds_constructor_p_q_inner_data_temp (struct tl_ds_p_q_inner_data *DS, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0xbfa3de38 && T->type->name != 0x405c21c7)) { return -1; }
  eprintf (" p_q_inner_data_temp");
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" pq :"); }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_string (DS->pq, field1);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" p :"); }
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_string (DS->p, field2);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" q :"); }
  struct paramed_type *field3 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_string (DS->q, field3);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" nonce :"); }
  struct paramed_type *field4 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x82c93bc6, .id = "Bare_Int128", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_int128 (DS->nonce, field4);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" server_nonce :"); }
  struct paramed_type *field5 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x82c93bc6, .id = "Bare_Int128", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_int128 (DS->server_nonce, field5);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" new_nonce :"); }
  struct paramed_type *field6 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x0d38674c, .id = "Bare_Int256", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_int256 (DS->new_nonce, field6);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" expires_in :"); }
  struct paramed_type *field7 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_int (DS->expires_in, field7);
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int print_ds_constructor_peer_user (struct tl_ds_peer *DS, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x276159d6 && T->type->name != 0xd89ea629)) { return -1; }
  eprintf (" peerUser");
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" user_id :"); }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_int (DS->user_id, field1);
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int print_ds_constructor_peer_chat (struct tl_ds_peer *DS, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x276159d6 && T->type->name != 0xd89ea629)) { return -1; }
  eprintf (" peerChat");
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" chat_id :"); }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_int (DS->chat_id, field1);
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int print_ds_constructor_peer_notify_events_empty (struct tl_ds_peer_notify_events *DS, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0xc0c8d13b && T->type->name != 0x3f372ec4)) { return -1; }
  eprintf (" peerNotifyEventsEmpty");
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int print_ds_constructor_peer_notify_events_all (struct tl_ds_peer_notify_events *DS, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0xc0c8d13b && T->type->name != 0x3f372ec4)) { return -1; }
  eprintf (" peerNotifyEventsAll");
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int print_ds_constructor_peer_notify_settings_empty (struct tl_ds_peer_notify_settings *DS, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0xfdf894fc && T->type->name != 0x02076b03)) { return -1; }
  eprintf (" peerNotifySettingsEmpty");
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int print_ds_constructor_peer_notify_settings (struct tl_ds_peer_notify_settings *DS, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0xfdf894fc && T->type->name != 0x02076b03)) { return -1; }
  eprintf (" peerNotifySettings");
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" mute_until :"); }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_int (DS->mute_until, field1);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" sound :"); }
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_string (DS->sound, field2);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" show_previews :"); }
  struct paramed_type *field3 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x250be282, .id = "Bool", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bool (DS->show_previews, field3);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" events_mask :"); }
  struct paramed_type *field4 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_int (DS->events_mask, field4);
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int print_ds_constructor_photo_empty (struct tl_ds_photo *DS, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0xc207550a && T->type->name != 0x3df8aaf5)) { return -1; }
  eprintf (" photoEmpty");
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" id :"); }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xddf89345, .id = "Bare_Long", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_long (DS->id, field1);
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int print_ds_constructor_photo (struct tl_ds_photo *DS, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0xc207550a && T->type->name != 0x3df8aaf5)) { return -1; }
  eprintf (" photo");
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" id :"); }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xddf89345, .id = "Bare_Long", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_long (DS->id, field1);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" access_hash :"); }
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xddf89345, .id = "Bare_Long", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_long (DS->access_hash, field2);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" user_id :"); }
  struct paramed_type *field3 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_int (DS->user_id, field3);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" date :"); }
  struct paramed_type *field4 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_int (DS->date, field4);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" geo :"); }
  struct paramed_type *field5 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x315e0a53, .id = "GeoPoint", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_geo_point (DS->geo, field5);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" sizes :"); }
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
  print_ds_type_vector ((void *)DS->sizes, field6);
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int print_ds_constructor_photo_l27 (struct tl_ds_photo *DS, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0xc207550a && T->type->name != 0x3df8aaf5)) { return -1; }
  eprintf (" photoL27");
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" id :"); }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xddf89345, .id = "Bare_Long", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_long (DS->id, field1);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" access_hash :"); }
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xddf89345, .id = "Bare_Long", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_long (DS->access_hash, field2);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" user_id :"); }
  struct paramed_type *field3 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_int (DS->user_id, field3);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" date :"); }
  struct paramed_type *field4 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_int (DS->date, field4);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" caption :"); }
  struct paramed_type *field5 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_string (DS->caption, field5);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" geo :"); }
  struct paramed_type *field6 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x315e0a53, .id = "GeoPoint", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_geo_point (DS->geo, field6);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" sizes :"); }
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
  print_ds_type_vector ((void *)DS->sizes, field7);
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int print_ds_constructor_photo_size_empty (struct tl_ds_photo_size *DS, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x900f60dd && T->type->name != 0x6ff09f22)) { return -1; }
  eprintf (" photoSizeEmpty");
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" type :"); }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_string (DS->type, field1);
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int print_ds_constructor_photo_size (struct tl_ds_photo_size *DS, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x900f60dd && T->type->name != 0x6ff09f22)) { return -1; }
  eprintf (" photoSize");
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" type :"); }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_string (DS->type, field1);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" location :"); }
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x2f8ffb30, .id = "FileLocation", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_file_location (DS->location, field2);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" w :"); }
  struct paramed_type *field3 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_int (DS->w, field3);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" h :"); }
  struct paramed_type *field4 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_int (DS->h, field4);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" size :"); }
  struct paramed_type *field5 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_int (DS->size, field5);
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int print_ds_constructor_photo_cached_size (struct tl_ds_photo_size *DS, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x900f60dd && T->type->name != 0x6ff09f22)) { return -1; }
  eprintf (" photoCachedSize");
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" type :"); }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_string (DS->type, field1);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" location :"); }
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x2f8ffb30, .id = "FileLocation", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_file_location (DS->location, field2);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" w :"); }
  struct paramed_type *field3 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_int (DS->w, field3);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" h :"); }
  struct paramed_type *field4 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_int (DS->h, field4);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" bytes :"); }
  struct paramed_type *field5 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xf11ec860, .id = "Bare_Bytes", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_bytes (DS->bytes, field5);
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int print_ds_constructor_privacy_key_status_timestamp (struct tl_ds_privacy_key *DS, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0xbc2eab30 && T->type->name != 0x43d154cf)) { return -1; }
  eprintf (" privacyKeyStatusTimestamp");
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int print_ds_constructor_privacy_value_allow_contacts (struct tl_ds_privacy_rule *DS, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0xa8638aec && T->type->name != 0x579c7513)) { return -1; }
  eprintf (" privacyValueAllowContacts");
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int print_ds_constructor_privacy_value_allow_all (struct tl_ds_privacy_rule *DS, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0xa8638aec && T->type->name != 0x579c7513)) { return -1; }
  eprintf (" privacyValueAllowAll");
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int print_ds_constructor_privacy_value_allow_users (struct tl_ds_privacy_rule *DS, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0xa8638aec && T->type->name != 0x579c7513)) { return -1; }
  eprintf (" privacyValueAllowUsers");
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" users :"); }
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
  print_ds_type_vector ((void *)DS->users, field1);
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int print_ds_constructor_privacy_value_disallow_contacts (struct tl_ds_privacy_rule *DS, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0xa8638aec && T->type->name != 0x579c7513)) { return -1; }
  eprintf (" privacyValueDisallowContacts");
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int print_ds_constructor_privacy_value_disallow_all (struct tl_ds_privacy_rule *DS, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0xa8638aec && T->type->name != 0x579c7513)) { return -1; }
  eprintf (" privacyValueDisallowAll");
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int print_ds_constructor_privacy_value_disallow_users (struct tl_ds_privacy_rule *DS, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0xa8638aec && T->type->name != 0x579c7513)) { return -1; }
  eprintf (" privacyValueDisallowUsers");
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" users :"); }
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
  print_ds_type_vector ((void *)DS->users, field1);
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int print_ds_constructor_received_notify_message (struct tl_ds_received_notify_message *DS, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0xa384b779 && T->type->name != 0x5c7b4886)) { return -1; }
  eprintf (" receivedNotifyMessage");
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" id :"); }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_int (DS->id, field1);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" flags :"); }
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_int (DS->flags, field2);
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int print_ds_constructor_reply_keyboard_hide (struct tl_ds_reply_markup *DS, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x612ca4a9 && T->type->name != 0x9ed35b56)) { return -1; }
  eprintf (" replyKeyboardHide");
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" flags :"); }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_int (DS->flags, field1);
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int print_ds_constructor_reply_keyboard_force_reply (struct tl_ds_reply_markup *DS, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x612ca4a9 && T->type->name != 0x9ed35b56)) { return -1; }
  eprintf (" replyKeyboardForceReply");
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" flags :"); }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_int (DS->flags, field1);
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int print_ds_constructor_reply_keyboard_markup (struct tl_ds_reply_markup *DS, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x612ca4a9 && T->type->name != 0x9ed35b56)) { return -1; }
  eprintf (" replyKeyboardMarkup");
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" flags :"); }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_int (DS->flags, field1);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" rows :"); }
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
  print_ds_type_vector ((void *)DS->rows, field2);
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int print_ds_constructor_res_p_q (struct tl_ds_res_p_q *DS, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x05162463 && T->type->name != 0xfae9db9c)) { return -1; }
  eprintf (" resPQ");
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" nonce :"); }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x82c93bc6, .id = "Bare_Int128", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_int128 (DS->nonce, field1);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" server_nonce :"); }
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x82c93bc6, .id = "Bare_Int128", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_int128 (DS->server_nonce, field2);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" pq :"); }
  struct paramed_type *field3 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_string (DS->pq, field3);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" server_public_key_fingerprints :"); }
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
  print_ds_type_vector ((void *)DS->server_public_key_fingerprints, field4);
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int print_ds_constructor_send_message_typing_action (struct tl_ds_send_message_action *DS, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x70541c7e && T->type->name != 0x8fabe381)) { return -1; }
  eprintf (" sendMessageTypingAction");
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int print_ds_constructor_send_message_cancel_action (struct tl_ds_send_message_action *DS, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x70541c7e && T->type->name != 0x8fabe381)) { return -1; }
  eprintf (" sendMessageCancelAction");
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int print_ds_constructor_send_message_record_video_action (struct tl_ds_send_message_action *DS, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x70541c7e && T->type->name != 0x8fabe381)) { return -1; }
  eprintf (" sendMessageRecordVideoAction");
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int print_ds_constructor_send_message_upload_video_action_l27 (struct tl_ds_send_message_action *DS, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x70541c7e && T->type->name != 0x8fabe381)) { return -1; }
  eprintf (" sendMessageUploadVideoActionL27");
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int print_ds_constructor_send_message_upload_video_action (struct tl_ds_send_message_action *DS, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x70541c7e && T->type->name != 0x8fabe381)) { return -1; }
  eprintf (" sendMessageUploadVideoAction");
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" progress :"); }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_int (DS->progress, field1);
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int print_ds_constructor_send_message_record_audio_action (struct tl_ds_send_message_action *DS, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x70541c7e && T->type->name != 0x8fabe381)) { return -1; }
  eprintf (" sendMessageRecordAudioAction");
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int print_ds_constructor_send_message_upload_audio_action_l27 (struct tl_ds_send_message_action *DS, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x70541c7e && T->type->name != 0x8fabe381)) { return -1; }
  eprintf (" sendMessageUploadAudioActionL27");
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int print_ds_constructor_send_message_upload_audio_action (struct tl_ds_send_message_action *DS, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x70541c7e && T->type->name != 0x8fabe381)) { return -1; }
  eprintf (" sendMessageUploadAudioAction");
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" progress :"); }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_int (DS->progress, field1);
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int print_ds_constructor_send_message_upload_photo_action (struct tl_ds_send_message_action *DS, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x70541c7e && T->type->name != 0x8fabe381)) { return -1; }
  eprintf (" sendMessageUploadPhotoAction");
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" progress :"); }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_int (DS->progress, field1);
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int print_ds_constructor_send_message_upload_document_action_l27 (struct tl_ds_send_message_action *DS, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x70541c7e && T->type->name != 0x8fabe381)) { return -1; }
  eprintf (" sendMessageUploadDocumentActionL27");
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int print_ds_constructor_send_message_upload_document_action (struct tl_ds_send_message_action *DS, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x70541c7e && T->type->name != 0x8fabe381)) { return -1; }
  eprintf (" sendMessageUploadDocumentAction");
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" progress :"); }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_int (DS->progress, field1);
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int print_ds_constructor_send_message_geo_location_action (struct tl_ds_send_message_action *DS, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x70541c7e && T->type->name != 0x8fabe381)) { return -1; }
  eprintf (" sendMessageGeoLocationAction");
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int print_ds_constructor_send_message_choose_contact_action (struct tl_ds_send_message_action *DS, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x70541c7e && T->type->name != 0x8fabe381)) { return -1; }
  eprintf (" sendMessageChooseContactAction");
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int print_ds_constructor_server_d_h_params_fail (struct tl_ds_server_d_h_params *DS, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0xa9230301 && T->type->name != 0x56dcfcfe)) { return -1; }
  eprintf (" server_DH_params_fail");
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" nonce :"); }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x82c93bc6, .id = "Bare_Int128", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_int128 (DS->nonce, field1);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" server_nonce :"); }
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x82c93bc6, .id = "Bare_Int128", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_int128 (DS->server_nonce, field2);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" new_nonce_hash :"); }
  struct paramed_type *field3 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x82c93bc6, .id = "Bare_Int128", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_int128 (DS->new_nonce_hash, field3);
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int print_ds_constructor_server_d_h_params_ok (struct tl_ds_server_d_h_params *DS, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0xa9230301 && T->type->name != 0x56dcfcfe)) { return -1; }
  eprintf (" server_DH_params_ok");
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" nonce :"); }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x82c93bc6, .id = "Bare_Int128", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_int128 (DS->nonce, field1);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" server_nonce :"); }
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x82c93bc6, .id = "Bare_Int128", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_int128 (DS->server_nonce, field2);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" encrypted_answer :"); }
  struct paramed_type *field3 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_string (DS->encrypted_answer, field3);
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int print_ds_constructor_server_d_h_inner_data (struct tl_ds_server_d_h_inner_data *DS, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0xb5890dba && T->type->name != 0x4a76f245)) { return -1; }
  eprintf (" server_DH_inner_data");
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" nonce :"); }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x82c93bc6, .id = "Bare_Int128", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_int128 (DS->nonce, field1);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" server_nonce :"); }
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x82c93bc6, .id = "Bare_Int128", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_int128 (DS->server_nonce, field2);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" g :"); }
  struct paramed_type *field3 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_int (DS->g, field3);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" dh_prime :"); }
  struct paramed_type *field4 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_string (DS->dh_prime, field4);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" g_a :"); }
  struct paramed_type *field5 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_string (DS->g_a, field5);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" server_time :"); }
  struct paramed_type *field6 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_int (DS->server_time, field6);
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int print_ds_constructor_dh_gen_ok (struct tl_ds_set_client_d_h_params_answer *DS, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0xdb8a468f && T->type->name != 0x2475b970)) { return -1; }
  eprintf (" dh_gen_ok");
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" nonce :"); }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x82c93bc6, .id = "Bare_Int128", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_int128 (DS->nonce, field1);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" server_nonce :"); }
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x82c93bc6, .id = "Bare_Int128", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_int128 (DS->server_nonce, field2);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" new_nonce_hash1 :"); }
  struct paramed_type *field3 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x82c93bc6, .id = "Bare_Int128", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_int128 (DS->new_nonce_hash1, field3);
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int print_ds_constructor_dh_gen_retry (struct tl_ds_set_client_d_h_params_answer *DS, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0xdb8a468f && T->type->name != 0x2475b970)) { return -1; }
  eprintf (" dh_gen_retry");
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" nonce :"); }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x82c93bc6, .id = "Bare_Int128", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_int128 (DS->nonce, field1);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" server_nonce :"); }
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x82c93bc6, .id = "Bare_Int128", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_int128 (DS->server_nonce, field2);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" new_nonce_hash2 :"); }
  struct paramed_type *field3 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x82c93bc6, .id = "Bare_Int128", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_int128 (DS->new_nonce_hash2, field3);
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int print_ds_constructor_dh_gen_fail (struct tl_ds_set_client_d_h_params_answer *DS, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0xdb8a468f && T->type->name != 0x2475b970)) { return -1; }
  eprintf (" dh_gen_fail");
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" nonce :"); }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x82c93bc6, .id = "Bare_Int128", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_int128 (DS->nonce, field1);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" server_nonce :"); }
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x82c93bc6, .id = "Bare_Int128", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_int128 (DS->server_nonce, field2);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" new_nonce_hash3 :"); }
  struct paramed_type *field3 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x82c93bc6, .id = "Bare_Int128", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_int128 (DS->new_nonce_hash3, field3);
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int print_ds_constructor_sticker_pack (struct tl_ds_sticker_pack *DS, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x12b299d4 && T->type->name != 0xed4d662b)) { return -1; }
  eprintf (" stickerPack");
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" emoticon :"); }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_string (DS->emoticon, field1);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" documents :"); }
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
  print_ds_type_vector ((void *)DS->documents, field2);
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int print_ds_constructor_sticker_set (struct tl_ds_sticker_set *DS, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0xa7a43b17 && T->type->name != 0x585bc4e8)) { return -1; }
  eprintf (" stickerSet");
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" id :"); }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xddf89345, .id = "Bare_Long", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_long (DS->id, field1);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" access_hash :"); }
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xddf89345, .id = "Bare_Long", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_long (DS->access_hash, field2);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" title :"); }
  struct paramed_type *field3 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_string (DS->title, field3);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" short_name :"); }
  struct paramed_type *field4 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_string (DS->short_name, field4);
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int print_ds_constructor_string (struct tl_ds_string *DS, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0xb5286e24 && T->type->name != 0x4ad791db)) { return -1; }
  print_escaped_string (DS->data, DS->len);
  return 0;
}
int print_ds_constructor_update_new_message (struct tl_ds_update *DS, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x2e02b27e && T->type->name != 0xd1fd4d81)) { return -1; }
  eprintf (" updateNewMessage");
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" message :"); }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x5d652a7f, .id = "Message", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_message (DS->message, field1);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" pts :"); }
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_int (DS->pts, field2);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" pts_count :"); }
  struct paramed_type *field3 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_int (DS->pts_count, field3);
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int print_ds_constructor_update_message_i_d (struct tl_ds_update *DS, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x2e02b27e && T->type->name != 0xd1fd4d81)) { return -1; }
  eprintf (" updateMessageID");
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" id :"); }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_int (DS->id, field1);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" random_id :"); }
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xddf89345, .id = "Bare_Long", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_long (DS->random_id, field2);
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int print_ds_constructor_update_delete_messages (struct tl_ds_update *DS, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x2e02b27e && T->type->name != 0xd1fd4d81)) { return -1; }
  eprintf (" updateDeleteMessages");
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" messages :"); }
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
  print_ds_type_vector ((void *)DS->messages, field1);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" pts :"); }
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_int (DS->pts, field2);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" pts_count :"); }
  struct paramed_type *field3 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_int (DS->pts_count, field3);
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int print_ds_constructor_update_user_typing (struct tl_ds_update *DS, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x2e02b27e && T->type->name != 0xd1fd4d81)) { return -1; }
  eprintf (" updateUserTyping");
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" user_id :"); }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_int (DS->user_id, field1);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" action :"); }
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x70541c7e, .id = "SendMessageAction", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_send_message_action (DS->action, field2);
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int print_ds_constructor_update_chat_user_typing (struct tl_ds_update *DS, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x2e02b27e && T->type->name != 0xd1fd4d81)) { return -1; }
  eprintf (" updateChatUserTyping");
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" chat_id :"); }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_int (DS->chat_id, field1);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" user_id :"); }
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_int (DS->user_id, field2);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" action :"); }
  struct paramed_type *field3 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x70541c7e, .id = "SendMessageAction", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_send_message_action (DS->action, field3);
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int print_ds_constructor_update_chat_participants (struct tl_ds_update *DS, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x2e02b27e && T->type->name != 0xd1fd4d81)) { return -1; }
  eprintf (" updateChatParticipants");
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" participants :"); }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x77930f9f, .id = "ChatParticipants", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_chat_participants (DS->participants, field1);
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int print_ds_constructor_update_user_status (struct tl_ds_update *DS, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x2e02b27e && T->type->name != 0xd1fd4d81)) { return -1; }
  eprintf (" updateUserStatus");
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" user_id :"); }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_int (DS->user_id, field1);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" status :"); }
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x76de9570, .id = "UserStatus", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_user_status (DS->status, field2);
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int print_ds_constructor_update_user_name (struct tl_ds_update *DS, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x2e02b27e && T->type->name != 0xd1fd4d81)) { return -1; }
  eprintf (" updateUserName");
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" user_id :"); }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_int (DS->user_id, field1);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" first_name :"); }
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_string (DS->first_name, field2);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" last_name :"); }
  struct paramed_type *field3 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_string (DS->last_name, field3);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" username :"); }
  struct paramed_type *field4 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_string (DS->username, field4);
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int print_ds_constructor_update_user_photo (struct tl_ds_update *DS, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x2e02b27e && T->type->name != 0xd1fd4d81)) { return -1; }
  eprintf (" updateUserPhoto");
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" user_id :"); }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_int (DS->user_id, field1);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" date :"); }
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_int (DS->date, field2);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" photo :"); }
  struct paramed_type *field3 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x9a486229, .id = "UserProfilePhoto", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_user_profile_photo (DS->photo, field3);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" previous :"); }
  struct paramed_type *field4 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x250be282, .id = "Bool", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bool (DS->previous, field4);
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int print_ds_constructor_update_contact_registered (struct tl_ds_update *DS, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x2e02b27e && T->type->name != 0xd1fd4d81)) { return -1; }
  eprintf (" updateContactRegistered");
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" user_id :"); }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_int (DS->user_id, field1);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" date :"); }
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_int (DS->date, field2);
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int print_ds_constructor_update_contact_link (struct tl_ds_update *DS, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x2e02b27e && T->type->name != 0xd1fd4d81)) { return -1; }
  eprintf (" updateContactLink");
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" user_id :"); }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_int (DS->user_id, field1);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" my_link :"); }
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x522fbc63, .id = "ContactLink", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_contact_link (DS->my_link, field2);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" foreign_link :"); }
  struct paramed_type *field3 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x522fbc63, .id = "ContactLink", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_contact_link (DS->foreign_link, field3);
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int print_ds_constructor_update_new_authorization (struct tl_ds_update *DS, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x2e02b27e && T->type->name != 0xd1fd4d81)) { return -1; }
  eprintf (" updateNewAuthorization");
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" auth_key_id :"); }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xddf89345, .id = "Bare_Long", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_long (DS->auth_key_id, field1);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" date :"); }
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_int (DS->date, field2);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" device :"); }
  struct paramed_type *field3 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_string (DS->device, field3);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" location :"); }
  struct paramed_type *field4 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_string (DS->location, field4);
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int print_ds_constructor_update_new_geo_chat_message (struct tl_ds_update *DS, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x2e02b27e && T->type->name != 0xd1fd4d81)) { return -1; }
  eprintf (" updateNewGeoChatMessage");
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" geo_message :"); }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xf67b4034, .id = "GeoChatMessage", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_geo_chat_message (DS->geo_message, field1);
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int print_ds_constructor_update_new_encrypted_message (struct tl_ds_update *DS, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x2e02b27e && T->type->name != 0xd1fd4d81)) { return -1; }
  eprintf (" updateNewEncryptedMessage");
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" encr_message :"); }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xce6b8a1e, .id = "EncryptedMessage", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_encrypted_message (DS->encr_message, field1);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" qts :"); }
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_int (DS->qts, field2);
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int print_ds_constructor_update_encrypted_chat_typing (struct tl_ds_update *DS, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x2e02b27e && T->type->name != 0xd1fd4d81)) { return -1; }
  eprintf (" updateEncryptedChatTyping");
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" chat_id :"); }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_int (DS->chat_id, field1);
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int print_ds_constructor_update_encryption (struct tl_ds_update *DS, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x2e02b27e && T->type->name != 0xd1fd4d81)) { return -1; }
  eprintf (" updateEncryption");
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" encr_chat :"); }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xb1718213, .id = "EncryptedChat", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_encrypted_chat (DS->encr_chat, field1);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" date :"); }
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_int (DS->date, field2);
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int print_ds_constructor_update_encrypted_messages_read (struct tl_ds_update *DS, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x2e02b27e && T->type->name != 0xd1fd4d81)) { return -1; }
  eprintf (" updateEncryptedMessagesRead");
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" chat_id :"); }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_int (DS->chat_id, field1);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" max_date :"); }
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_int (DS->max_date, field2);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" date :"); }
  struct paramed_type *field3 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_int (DS->date, field3);
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int print_ds_constructor_update_chat_participant_add (struct tl_ds_update *DS, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x2e02b27e && T->type->name != 0xd1fd4d81)) { return -1; }
  eprintf (" updateChatParticipantAdd");
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" chat_id :"); }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_int (DS->chat_id, field1);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" user_id :"); }
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_int (DS->user_id, field2);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" inviter_id :"); }
  struct paramed_type *field3 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_int (DS->inviter_id, field3);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" version :"); }
  struct paramed_type *field4 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_int (DS->version, field4);
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int print_ds_constructor_update_chat_participant_delete (struct tl_ds_update *DS, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x2e02b27e && T->type->name != 0xd1fd4d81)) { return -1; }
  eprintf (" updateChatParticipantDelete");
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" chat_id :"); }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_int (DS->chat_id, field1);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" user_id :"); }
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_int (DS->user_id, field2);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" version :"); }
  struct paramed_type *field3 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_int (DS->version, field3);
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int print_ds_constructor_update_dc_options (struct tl_ds_update *DS, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x2e02b27e && T->type->name != 0xd1fd4d81)) { return -1; }
  eprintf (" updateDcOptions");
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" dc_options :"); }
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
  print_ds_type_vector ((void *)DS->dc_options, field1);
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int print_ds_constructor_update_user_blocked (struct tl_ds_update *DS, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x2e02b27e && T->type->name != 0xd1fd4d81)) { return -1; }
  eprintf (" updateUserBlocked");
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" user_id :"); }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_int (DS->user_id, field1);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" blocked :"); }
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x250be282, .id = "Bool", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bool (DS->blocked, field2);
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int print_ds_constructor_update_notify_settings (struct tl_ds_update *DS, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x2e02b27e && T->type->name != 0xd1fd4d81)) { return -1; }
  eprintf (" updateNotifySettings");
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" notify_peer :"); }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x9fcb8237, .id = "NotifyPeer", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_notify_peer (DS->notify_peer, field1);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" notify_settings :"); }
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xfdf894fc, .id = "PeerNotifySettings", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_peer_notify_settings (DS->notify_settings, field2);
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int print_ds_constructor_update_service_notification (struct tl_ds_update *DS, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x2e02b27e && T->type->name != 0xd1fd4d81)) { return -1; }
  eprintf (" updateServiceNotification");
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" type :"); }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_string (DS->type, field1);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" message_text :"); }
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_string (DS->message_text, field2);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" media :"); }
  struct paramed_type *field3 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x95f2471a, .id = "MessageMedia", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_message_media (DS->media, field3);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" popup :"); }
  struct paramed_type *field4 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x250be282, .id = "Bool", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bool (DS->popup, field4);
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int print_ds_constructor_update_privacy (struct tl_ds_update *DS, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x2e02b27e && T->type->name != 0xd1fd4d81)) { return -1; }
  eprintf (" updatePrivacy");
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" key :"); }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xbc2eab30, .id = "PrivacyKey", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_privacy_key (DS->key, field1);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" rules :"); }
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
  print_ds_type_vector ((void *)DS->rules, field2);
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int print_ds_constructor_update_user_phone (struct tl_ds_update *DS, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x2e02b27e && T->type->name != 0xd1fd4d81)) { return -1; }
  eprintf (" updateUserPhone");
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" user_id :"); }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_int (DS->user_id, field1);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" phone :"); }
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_string (DS->phone, field2);
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int print_ds_constructor_update_read_history_inbox (struct tl_ds_update *DS, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x2e02b27e && T->type->name != 0xd1fd4d81)) { return -1; }
  eprintf (" updateReadHistoryInbox");
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" peer :"); }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x276159d6, .id = "Peer", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_peer (DS->peer, field1);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" max_id :"); }
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_int (DS->max_id, field2);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" pts :"); }
  struct paramed_type *field3 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_int (DS->pts, field3);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" pts_count :"); }
  struct paramed_type *field4 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_int (DS->pts_count, field4);
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int print_ds_constructor_update_read_history_outbox (struct tl_ds_update *DS, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x2e02b27e && T->type->name != 0xd1fd4d81)) { return -1; }
  eprintf (" updateReadHistoryOutbox");
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" peer :"); }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x276159d6, .id = "Peer", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_peer (DS->peer, field1);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" max_id :"); }
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_int (DS->max_id, field2);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" pts :"); }
  struct paramed_type *field3 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_int (DS->pts, field3);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" pts_count :"); }
  struct paramed_type *field4 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_int (DS->pts_count, field4);
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int print_ds_constructor_update_web_page (struct tl_ds_update *DS, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x2e02b27e && T->type->name != 0xd1fd4d81)) { return -1; }
  eprintf (" updateWebPage");
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" webpage :"); }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x8d8c0d41, .id = "WebPage", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_web_page (DS->webpage, field1);
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int print_ds_constructor_update_read_messages_contents (struct tl_ds_update *DS, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x2e02b27e && T->type->name != 0xd1fd4d81)) { return -1; }
  eprintf (" updateReadMessagesContents");
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" messages :"); }
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
  print_ds_type_vector ((void *)DS->messages, field1);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" pts :"); }
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_int (DS->pts, field2);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" pts_count :"); }
  struct paramed_type *field3 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_int (DS->pts_count, field3);
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int print_ds_constructor_update_msg_update (struct tl_ds_update *DS, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x2e02b27e && T->type->name != 0xd1fd4d81)) { return -1; }
  eprintf (" updateMsgUpdate");
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" id :"); }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_int (DS->id, field1);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" pts :"); }
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_int (DS->pts, field2);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" pts_count :"); }
  struct paramed_type *field3 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_int (DS->pts_count, field3);
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int print_ds_constructor_updates_too_long (struct tl_ds_updates *DS, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x22499d27 && T->type->name != 0xddb662d8)) { return -1; }
  eprintf (" updatesTooLong");
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int print_ds_constructor_update_short_message (struct tl_ds_updates *DS, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x22499d27 && T->type->name != 0xddb662d8)) { return -1; }
  eprintf (" updateShortMessage");
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" flags :"); }
  struct paramed_type *var0 = INT2PTR (*DS->flags);
  eprintf (" %d", (int)PTR2INT (var0));
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" id :"); }
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_int (DS->id, field2);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" user_id :"); }
  struct paramed_type *field3 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_int (DS->user_id, field3);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" message :"); }
  struct paramed_type *field4 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_string (DS->message, field4);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" pts :"); }
  struct paramed_type *field5 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_int (DS->pts, field5);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" pts_count :"); }
  struct paramed_type *field6 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_int (DS->pts_count, field6);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" date :"); }
  struct paramed_type *field7 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_int (DS->date, field7);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (PTR2INT (var0) & (1 << 2)) {
    if (multiline_output >= 2) { print_offset (); }
    if (!disable_field_names) { eprintf (" fwd_from_id :"); }
    struct paramed_type *field8 = 
    &(struct paramed_type){
      .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
      .params = 0,
    };
    print_ds_type_bare_int (DS->fwd_from_id, field8);
    if (multiline_output >= 2) { eprintf ("\n"); }
  }
  if (PTR2INT (var0) & (1 << 2)) {
    if (multiline_output >= 2) { print_offset (); }
    if (!disable_field_names) { eprintf (" fwd_date :"); }
    struct paramed_type *field9 = 
    &(struct paramed_type){
      .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
      .params = 0,
    };
    print_ds_type_bare_int (DS->fwd_date, field9);
    if (multiline_output >= 2) { eprintf ("\n"); }
  }
  if (PTR2INT (var0) & (1 << 3)) {
    if (multiline_output >= 2) { print_offset (); }
    if (!disable_field_names) { eprintf (" reply_to_msg_id :"); }
    struct paramed_type *field10 = 
    &(struct paramed_type){
      .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
      .params = 0,
    };
    print_ds_type_bare_int (DS->reply_to_msg_id, field10);
    if (multiline_output >= 2) { eprintf ("\n"); }
  }
  return 0;
}
int print_ds_constructor_update_short_chat_message (struct tl_ds_updates *DS, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x22499d27 && T->type->name != 0xddb662d8)) { return -1; }
  eprintf (" updateShortChatMessage");
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" flags :"); }
  struct paramed_type *var0 = INT2PTR (*DS->flags);
  eprintf (" %d", (int)PTR2INT (var0));
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" id :"); }
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_int (DS->id, field2);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" from_id :"); }
  struct paramed_type *field3 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_int (DS->from_id, field3);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" chat_id :"); }
  struct paramed_type *field4 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_int (DS->chat_id, field4);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" message :"); }
  struct paramed_type *field5 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_string (DS->message, field5);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" pts :"); }
  struct paramed_type *field6 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_int (DS->pts, field6);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" pts_count :"); }
  struct paramed_type *field7 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_int (DS->pts_count, field7);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" date :"); }
  struct paramed_type *field8 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_int (DS->date, field8);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (PTR2INT (var0) & (1 << 2)) {
    if (multiline_output >= 2) { print_offset (); }
    if (!disable_field_names) { eprintf (" fwd_from_id :"); }
    struct paramed_type *field9 = 
    &(struct paramed_type){
      .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
      .params = 0,
    };
    print_ds_type_bare_int (DS->fwd_from_id, field9);
    if (multiline_output >= 2) { eprintf ("\n"); }
  }
  if (PTR2INT (var0) & (1 << 2)) {
    if (multiline_output >= 2) { print_offset (); }
    if (!disable_field_names) { eprintf (" fwd_date :"); }
    struct paramed_type *field10 = 
    &(struct paramed_type){
      .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
      .params = 0,
    };
    print_ds_type_bare_int (DS->fwd_date, field10);
    if (multiline_output >= 2) { eprintf ("\n"); }
  }
  if (PTR2INT (var0) & (1 << 3)) {
    if (multiline_output >= 2) { print_offset (); }
    if (!disable_field_names) { eprintf (" reply_to_msg_id :"); }
    struct paramed_type *field11 = 
    &(struct paramed_type){
      .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
      .params = 0,
    };
    print_ds_type_bare_int (DS->reply_to_msg_id, field11);
    if (multiline_output >= 2) { eprintf ("\n"); }
  }
  return 0;
}
int print_ds_constructor_update_short (struct tl_ds_updates *DS, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x22499d27 && T->type->name != 0xddb662d8)) { return -1; }
  eprintf (" updateShort");
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" update :"); }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x2e02b27e, .id = "Update", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_update (DS->update, field1);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" date :"); }
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_int (DS->date, field2);
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int print_ds_constructor_updates_combined (struct tl_ds_updates *DS, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x22499d27 && T->type->name != 0xddb662d8)) { return -1; }
  eprintf (" updatesCombined");
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" updates :"); }
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
  print_ds_type_vector ((void *)DS->updates, field1);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" users :"); }
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
  print_ds_type_vector ((void *)DS->users, field2);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" chats :"); }
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
  print_ds_type_vector ((void *)DS->chats, field3);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" date :"); }
  struct paramed_type *field4 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_int (DS->date, field4);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" seq_start :"); }
  struct paramed_type *field5 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_int (DS->seq_start, field5);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" seq :"); }
  struct paramed_type *field6 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_int (DS->seq, field6);
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int print_ds_constructor_updates (struct tl_ds_updates *DS, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x22499d27 && T->type->name != 0xddb662d8)) { return -1; }
  eprintf (" updates");
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" updates :"); }
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
  print_ds_type_vector ((void *)DS->updates, field1);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" users :"); }
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
  print_ds_type_vector ((void *)DS->users, field2);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" chats :"); }
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
  print_ds_type_vector ((void *)DS->chats, field3);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" date :"); }
  struct paramed_type *field4 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_int (DS->date, field4);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" seq :"); }
  struct paramed_type *field5 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_int (DS->seq, field5);
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int print_ds_constructor_user_empty (struct tl_ds_user *DS, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x02e6c0c8 && T->type->name != 0xfd193f37)) { return -1; }
  eprintf (" userEmpty");
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" id :"); }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_int (DS->id, field1);
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int print_ds_constructor_user (struct tl_ds_user *DS, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x02e6c0c8 && T->type->name != 0xfd193f37)) { return -1; }
  eprintf (" user");
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" flags :"); }
  struct paramed_type *var0 = INT2PTR (*DS->flags);
  eprintf (" %d", (int)PTR2INT (var0));
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" id :"); }
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_int (DS->id, field2);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (PTR2INT (var0) & (1 << 0)) {
    if (multiline_output >= 2) { print_offset (); }
    if (!disable_field_names) { eprintf (" access_hash :"); }
    struct paramed_type *field3 = 
    &(struct paramed_type){
      .type = &(struct tl_type_descr) {.name = 0xddf89345, .id = "Bare_Long", .params_num = 0, .params_types = 0},
      .params = 0,
    };
    print_ds_type_bare_long (DS->access_hash, field3);
    if (multiline_output >= 2) { eprintf ("\n"); }
  }
  if (PTR2INT (var0) & (1 << 1)) {
    if (multiline_output >= 2) { print_offset (); }
    if (!disable_field_names) { eprintf (" first_name :"); }
    struct paramed_type *field4 = 
    &(struct paramed_type){
      .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
      .params = 0,
    };
    print_ds_type_bare_string (DS->first_name, field4);
    if (multiline_output >= 2) { eprintf ("\n"); }
  }
  if (PTR2INT (var0) & (1 << 2)) {
    if (multiline_output >= 2) { print_offset (); }
    if (!disable_field_names) { eprintf (" last_name :"); }
    struct paramed_type *field5 = 
    &(struct paramed_type){
      .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
      .params = 0,
    };
    print_ds_type_bare_string (DS->last_name, field5);
    if (multiline_output >= 2) { eprintf ("\n"); }
  }
  if (PTR2INT (var0) & (1 << 3)) {
    if (multiline_output >= 2) { print_offset (); }
    if (!disable_field_names) { eprintf (" username :"); }
    struct paramed_type *field6 = 
    &(struct paramed_type){
      .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
      .params = 0,
    };
    print_ds_type_bare_string (DS->username, field6);
    if (multiline_output >= 2) { eprintf ("\n"); }
  }
  if (PTR2INT (var0) & (1 << 4)) {
    if (multiline_output >= 2) { print_offset (); }
    if (!disable_field_names) { eprintf (" phone :"); }
    struct paramed_type *field7 = 
    &(struct paramed_type){
      .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
      .params = 0,
    };
    print_ds_type_bare_string (DS->phone, field7);
    if (multiline_output >= 2) { eprintf ("\n"); }
  }
  if (PTR2INT (var0) & (1 << 5)) {
    if (multiline_output >= 2) { print_offset (); }
    if (!disable_field_names) { eprintf (" photo :"); }
    struct paramed_type *field8 = 
    &(struct paramed_type){
      .type = &(struct tl_type_descr) {.name = 0x9a486229, .id = "UserProfilePhoto", .params_num = 0, .params_types = 0},
      .params = 0,
    };
    print_ds_type_user_profile_photo (DS->photo, field8);
    if (multiline_output >= 2) { eprintf ("\n"); }
  }
  if (PTR2INT (var0) & (1 << 6)) {
    if (multiline_output >= 2) { print_offset (); }
    if (!disable_field_names) { eprintf (" status :"); }
    struct paramed_type *field9 = 
    &(struct paramed_type){
      .type = &(struct tl_type_descr) {.name = 0x76de9570, .id = "UserStatus", .params_num = 0, .params_types = 0},
      .params = 0,
    };
    print_ds_type_user_status (DS->status, field9);
    if (multiline_output >= 2) { eprintf ("\n"); }
  }
  if (PTR2INT (var0) & (1 << 14)) {
    if (multiline_output >= 2) { print_offset (); }
    if (!disable_field_names) { eprintf (" bot_info_version :"); }
    struct paramed_type *field10 = 
    &(struct paramed_type){
      .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
      .params = 0,
    };
    print_ds_type_bare_int (DS->bot_info_version, field10);
    if (multiline_output >= 2) { eprintf ("\n"); }
  }
  return 0;
}
int print_ds_constructor_user_full (struct tl_ds_user_full *DS, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x5a89ac5b && T->type->name != 0xa57653a4)) { return -1; }
  eprintf (" userFull");
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" user :"); }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x02e6c0c8, .id = "User", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_user (DS->user, field1);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" link :"); }
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x3ace484c, .id = "contacts.Link", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_contacts_link (DS->link, field2);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" profile_photo :"); }
  struct paramed_type *field3 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xc207550a, .id = "Photo", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_photo (DS->profile_photo, field3);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" notify_settings :"); }
  struct paramed_type *field4 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xfdf894fc, .id = "PeerNotifySettings", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_peer_notify_settings (DS->notify_settings, field4);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" blocked :"); }
  struct paramed_type *field5 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x250be282, .id = "Bool", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bool (DS->blocked, field5);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" bot_info :"); }
  struct paramed_type *field6 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xb2e16f93, .id = "BotInfo", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bot_info (DS->bot_info, field6);
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int print_ds_constructor_user_profile_photo_empty (struct tl_ds_user_profile_photo *DS, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x9a486229 && T->type->name != 0x65b79dd6)) { return -1; }
  eprintf (" userProfilePhotoEmpty");
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int print_ds_constructor_user_profile_photo (struct tl_ds_user_profile_photo *DS, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x9a486229 && T->type->name != 0x65b79dd6)) { return -1; }
  eprintf (" userProfilePhoto");
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" photo_id :"); }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xddf89345, .id = "Bare_Long", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_long (DS->photo_id, field1);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" photo_small :"); }
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x2f8ffb30, .id = "FileLocation", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_file_location (DS->photo_small, field2);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" photo_big :"); }
  struct paramed_type *field3 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x2f8ffb30, .id = "FileLocation", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_file_location (DS->photo_big, field3);
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int print_ds_constructor_user_status_empty (struct tl_ds_user_status *DS, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x76de9570 && T->type->name != 0x89216a8f)) { return -1; }
  eprintf (" userStatusEmpty");
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int print_ds_constructor_user_status_online (struct tl_ds_user_status *DS, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x76de9570 && T->type->name != 0x89216a8f)) { return -1; }
  eprintf (" userStatusOnline");
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" expires :"); }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_int (DS->expires, field1);
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int print_ds_constructor_user_status_offline (struct tl_ds_user_status *DS, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x76de9570 && T->type->name != 0x89216a8f)) { return -1; }
  eprintf (" userStatusOffline");
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" was_online :"); }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_int (DS->was_online, field1);
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int print_ds_constructor_user_status_recently (struct tl_ds_user_status *DS, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x76de9570 && T->type->name != 0x89216a8f)) { return -1; }
  eprintf (" userStatusRecently");
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int print_ds_constructor_user_status_last_week (struct tl_ds_user_status *DS, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x76de9570 && T->type->name != 0x89216a8f)) { return -1; }
  eprintf (" userStatusLastWeek");
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int print_ds_constructor_user_status_last_month (struct tl_ds_user_status *DS, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x76de9570 && T->type->name != 0x89216a8f)) { return -1; }
  eprintf (" userStatusLastMonth");
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int print_ds_constructor_vector (struct tl_ds_vector *DS, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x1cb5c415 && T->type->name != 0xe34a3bea)) { return -1; }
  if (ODDP(T->params[0])) { return -1; }
  struct paramed_type *var0 = T->params[0]; assert (var0);
  eprintf (" vector");
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  struct paramed_type *var1 = INT2PTR (*DS->f1);
  eprintf (" %d", (int)PTR2INT (var1));
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  int multiplicity3 = PTR2INT (
  ((void *)var1) + 0  );
  struct paramed_type *field3 = 
  var0;
  eprintf (" [");
  if (multiline_output >= 1) { eprintf ("\n"); }
  if (multiline_output >= 1) { multiline_offset += multiline_offset_size;}
  {
    int i = 0;
    while (i < multiplicity3) {
      if (multiline_output >= 1) { print_offset (); }
      print_ds_type_any (DS->f2[i ++], field3);
      if (multiline_output >= 1) { eprintf ("\n"); }
    }
  }
  if (multiline_output >= 1) { multiline_offset -= multiline_offset_size; print_offset ();}
  eprintf (" ]");
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int print_ds_constructor_video_empty (struct tl_ds_video *DS, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x1716b174 && T->type->name != 0xe8e94e8b)) { return -1; }
  eprintf (" videoEmpty");
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" id :"); }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xddf89345, .id = "Bare_Long", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_long (DS->id, field1);
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int print_ds_constructor_video (struct tl_ds_video *DS, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x1716b174 && T->type->name != 0xe8e94e8b)) { return -1; }
  eprintf (" video");
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" id :"); }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xddf89345, .id = "Bare_Long", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_long (DS->id, field1);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" access_hash :"); }
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xddf89345, .id = "Bare_Long", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_long (DS->access_hash, field2);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" user_id :"); }
  struct paramed_type *field3 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_int (DS->user_id, field3);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" date :"); }
  struct paramed_type *field4 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_int (DS->date, field4);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" duration :"); }
  struct paramed_type *field5 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_int (DS->duration, field5);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" size :"); }
  struct paramed_type *field6 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_int (DS->size, field6);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" thumb :"); }
  struct paramed_type *field7 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x900f60dd, .id = "PhotoSize", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_photo_size (DS->thumb, field7);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" dc_id :"); }
  struct paramed_type *field8 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_int (DS->dc_id, field8);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" w :"); }
  struct paramed_type *field9 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_int (DS->w, field9);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" h :"); }
  struct paramed_type *field10 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_int (DS->h, field10);
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int print_ds_constructor_video_l27 (struct tl_ds_video *DS, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x1716b174 && T->type->name != 0xe8e94e8b)) { return -1; }
  eprintf (" videoL27");
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" id :"); }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xddf89345, .id = "Bare_Long", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_long (DS->id, field1);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" access_hash :"); }
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xddf89345, .id = "Bare_Long", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_long (DS->access_hash, field2);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" user_id :"); }
  struct paramed_type *field3 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_int (DS->user_id, field3);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" date :"); }
  struct paramed_type *field4 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_int (DS->date, field4);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" caption :"); }
  struct paramed_type *field5 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_string (DS->caption, field5);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" duration :"); }
  struct paramed_type *field6 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_int (DS->duration, field6);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" mime_type :"); }
  struct paramed_type *field7 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_string (DS->mime_type, field7);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" size :"); }
  struct paramed_type *field8 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_int (DS->size, field8);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" thumb :"); }
  struct paramed_type *field9 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x900f60dd, .id = "PhotoSize", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_photo_size (DS->thumb, field9);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" dc_id :"); }
  struct paramed_type *field10 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_int (DS->dc_id, field10);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" w :"); }
  struct paramed_type *field11 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_int (DS->w, field11);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" h :"); }
  struct paramed_type *field12 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_int (DS->h, field12);
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int print_ds_constructor_wall_paper (struct tl_ds_wall_paper *DS, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0xafa14973 && T->type->name != 0x505eb68c)) { return -1; }
  eprintf (" wallPaper");
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" id :"); }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_int (DS->id, field1);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" title :"); }
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_string (DS->title, field2);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" sizes :"); }
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
  print_ds_type_vector ((void *)DS->sizes, field3);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" color :"); }
  struct paramed_type *field4 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_int (DS->color, field4);
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int print_ds_constructor_wall_paper_solid (struct tl_ds_wall_paper *DS, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0xafa14973 && T->type->name != 0x505eb68c)) { return -1; }
  eprintf (" wallPaperSolid");
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" id :"); }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_int (DS->id, field1);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" title :"); }
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_string (DS->title, field2);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" bg_color :"); }
  struct paramed_type *field3 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_int (DS->bg_color, field3);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" color :"); }
  struct paramed_type *field4 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_int (DS->color, field4);
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int print_ds_constructor_web_page_empty (struct tl_ds_web_page *DS, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x8d8c0d41 && T->type->name != 0x7273f2be)) { return -1; }
  eprintf (" webPageEmpty");
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" id :"); }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xddf89345, .id = "Bare_Long", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_long (DS->id, field1);
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int print_ds_constructor_web_page_pending (struct tl_ds_web_page *DS, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x8d8c0d41 && T->type->name != 0x7273f2be)) { return -1; }
  eprintf (" webPagePending");
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" id :"); }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xddf89345, .id = "Bare_Long", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_long (DS->id, field1);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" date :"); }
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_int (DS->date, field2);
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int print_ds_constructor_web_page (struct tl_ds_web_page *DS, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x8d8c0d41 && T->type->name != 0x7273f2be)) { return -1; }
  eprintf (" webPage");
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" flags :"); }
  struct paramed_type *var0 = INT2PTR (*DS->flags);
  eprintf (" %d", (int)PTR2INT (var0));
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" id :"); }
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xddf89345, .id = "Bare_Long", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_long (DS->id, field2);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" url :"); }
  struct paramed_type *field3 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_string (DS->url, field3);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" display_url :"); }
  struct paramed_type *field4 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_string (DS->display_url, field4);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (PTR2INT (var0) & (1 << 0)) {
    if (multiline_output >= 2) { print_offset (); }
    if (!disable_field_names) { eprintf (" type :"); }
    struct paramed_type *field5 = 
    &(struct paramed_type){
      .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
      .params = 0,
    };
    print_ds_type_bare_string (DS->type, field5);
    if (multiline_output >= 2) { eprintf ("\n"); }
  }
  if (PTR2INT (var0) & (1 << 1)) {
    if (multiline_output >= 2) { print_offset (); }
    if (!disable_field_names) { eprintf (" site_name :"); }
    struct paramed_type *field6 = 
    &(struct paramed_type){
      .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
      .params = 0,
    };
    print_ds_type_bare_string (DS->site_name, field6);
    if (multiline_output >= 2) { eprintf ("\n"); }
  }
  if (PTR2INT (var0) & (1 << 2)) {
    if (multiline_output >= 2) { print_offset (); }
    if (!disable_field_names) { eprintf (" title :"); }
    struct paramed_type *field7 = 
    &(struct paramed_type){
      .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
      .params = 0,
    };
    print_ds_type_bare_string (DS->title, field7);
    if (multiline_output >= 2) { eprintf ("\n"); }
  }
  if (PTR2INT (var0) & (1 << 3)) {
    if (multiline_output >= 2) { print_offset (); }
    if (!disable_field_names) { eprintf (" description :"); }
    struct paramed_type *field8 = 
    &(struct paramed_type){
      .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
      .params = 0,
    };
    print_ds_type_bare_string (DS->description, field8);
    if (multiline_output >= 2) { eprintf ("\n"); }
  }
  if (PTR2INT (var0) & (1 << 4)) {
    if (multiline_output >= 2) { print_offset (); }
    if (!disable_field_names) { eprintf (" photo :"); }
    struct paramed_type *field9 = 
    &(struct paramed_type){
      .type = &(struct tl_type_descr) {.name = 0xc207550a, .id = "Photo", .params_num = 0, .params_types = 0},
      .params = 0,
    };
    print_ds_type_photo (DS->photo, field9);
    if (multiline_output >= 2) { eprintf ("\n"); }
  }
  if (PTR2INT (var0) & (1 << 5)) {
    if (multiline_output >= 2) { print_offset (); }
    if (!disable_field_names) { eprintf (" embed_url :"); }
    struct paramed_type *field10 = 
    &(struct paramed_type){
      .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
      .params = 0,
    };
    print_ds_type_bare_string (DS->embed_url, field10);
    if (multiline_output >= 2) { eprintf ("\n"); }
  }
  if (PTR2INT (var0) & (1 << 5)) {
    if (multiline_output >= 2) { print_offset (); }
    if (!disable_field_names) { eprintf (" embed_type :"); }
    struct paramed_type *field11 = 
    &(struct paramed_type){
      .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
      .params = 0,
    };
    print_ds_type_bare_string (DS->embed_type, field11);
    if (multiline_output >= 2) { eprintf ("\n"); }
  }
  if (PTR2INT (var0) & (1 << 6)) {
    if (multiline_output >= 2) { print_offset (); }
    if (!disable_field_names) { eprintf (" embed_width :"); }
    struct paramed_type *field12 = 
    &(struct paramed_type){
      .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
      .params = 0,
    };
    print_ds_type_bare_int (DS->embed_width, field12);
    if (multiline_output >= 2) { eprintf ("\n"); }
  }
  if (PTR2INT (var0) & (1 << 6)) {
    if (multiline_output >= 2) { print_offset (); }
    if (!disable_field_names) { eprintf (" embed_height :"); }
    struct paramed_type *field13 = 
    &(struct paramed_type){
      .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
      .params = 0,
    };
    print_ds_type_bare_int (DS->embed_height, field13);
    if (multiline_output >= 2) { eprintf ("\n"); }
  }
  if (PTR2INT (var0) & (1 << 7)) {
    if (multiline_output >= 2) { print_offset (); }
    if (!disable_field_names) { eprintf (" duration :"); }
    struct paramed_type *field14 = 
    &(struct paramed_type){
      .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
      .params = 0,
    };
    print_ds_type_bare_int (DS->duration, field14);
    if (multiline_output >= 2) { eprintf ("\n"); }
  }
  if (PTR2INT (var0) & (1 << 8)) {
    if (multiline_output >= 2) { print_offset (); }
    if (!disable_field_names) { eprintf (" author :"); }
    struct paramed_type *field15 = 
    &(struct paramed_type){
      .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
      .params = 0,
    };
    print_ds_type_bare_string (DS->author, field15);
    if (multiline_output >= 2) { eprintf ("\n"); }
  }
  return 0;
}
int print_ds_constructor_account_authorizations (struct tl_ds_account_authorizations *DS, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x1250abde && T->type->name != 0xedaf5421)) { return -1; }
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
  print_ds_type_vector ((void *)DS->authorizations, field1);
  return 0;
}
int print_ds_constructor_account_no_password (struct tl_ds_account_password *DS, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0xeac2a804 && T->type->name != 0x153d57fb)) { return -1; }
  eprintf (" account.noPassword");
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" new_salt :"); }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xf11ec860, .id = "Bare_Bytes", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_bytes (DS->new_salt, field1);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" email_unconfirmed_pattern :"); }
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_string (DS->email_unconfirmed_pattern, field2);
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int print_ds_constructor_account_password (struct tl_ds_account_password *DS, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0xeac2a804 && T->type->name != 0x153d57fb)) { return -1; }
  eprintf (" account.password");
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" current_salt :"); }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xf11ec860, .id = "Bare_Bytes", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_bytes (DS->current_salt, field1);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" new_salt :"); }
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xf11ec860, .id = "Bare_Bytes", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_bytes (DS->new_salt, field2);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" hint :"); }
  struct paramed_type *field3 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_string (DS->hint, field3);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" has_recovery :"); }
  struct paramed_type *field4 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x250be282, .id = "Bool", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bool (DS->has_recovery, field4);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" email_unconfirmed_pattern :"); }
  struct paramed_type *field5 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_string (DS->email_unconfirmed_pattern, field5);
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int print_ds_constructor_account_password_input_settings (struct tl_ds_account_password_input_settings *DS, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0xbcfc532c && T->type->name != 0x4303acd3)) { return -1; }
  eprintf (" account.passwordInputSettings");
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" flags :"); }
  struct paramed_type *var0 = INT2PTR (*DS->flags);
  eprintf (" %d", (int)PTR2INT (var0));
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (PTR2INT (var0) & (1 << 0)) {
    if (multiline_output >= 2) { print_offset (); }
    if (!disable_field_names) { eprintf (" new_salt :"); }
    struct paramed_type *field2 = 
    &(struct paramed_type){
      .type = &(struct tl_type_descr) {.name = 0xf11ec860, .id = "Bare_Bytes", .params_num = 0, .params_types = 0},
      .params = 0,
    };
    print_ds_type_bare_bytes (DS->new_salt, field2);
    if (multiline_output >= 2) { eprintf ("\n"); }
  }
  if (PTR2INT (var0) & (1 << 0)) {
    if (multiline_output >= 2) { print_offset (); }
    if (!disable_field_names) { eprintf (" new_password_hash :"); }
    struct paramed_type *field3 = 
    &(struct paramed_type){
      .type = &(struct tl_type_descr) {.name = 0xf11ec860, .id = "Bare_Bytes", .params_num = 0, .params_types = 0},
      .params = 0,
    };
    print_ds_type_bare_bytes (DS->new_password_hash, field3);
    if (multiline_output >= 2) { eprintf ("\n"); }
  }
  if (PTR2INT (var0) & (1 << 0)) {
    if (multiline_output >= 2) { print_offset (); }
    if (!disable_field_names) { eprintf (" hint :"); }
    struct paramed_type *field4 = 
    &(struct paramed_type){
      .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
      .params = 0,
    };
    print_ds_type_bare_string (DS->hint, field4);
    if (multiline_output >= 2) { eprintf ("\n"); }
  }
  if (PTR2INT (var0) & (1 << 1)) {
    if (multiline_output >= 2) { print_offset (); }
    if (!disable_field_names) { eprintf (" email :"); }
    struct paramed_type *field5 = 
    &(struct paramed_type){
      .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
      .params = 0,
    };
    print_ds_type_bare_string (DS->email, field5);
    if (multiline_output >= 2) { eprintf ("\n"); }
  }
  return 0;
}
int print_ds_constructor_account_password_settings (struct tl_ds_account_password_settings *DS, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0xb7b72ab3 && T->type->name != 0x4848d54c)) { return -1; }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_string (DS->email, field1);
  return 0;
}
int print_ds_constructor_account_privacy_rules (struct tl_ds_account_privacy_rules *DS, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x554abb6f && T->type->name != 0xaab54490)) { return -1; }
  eprintf (" account.privacyRules");
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" rules :"); }
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
  print_ds_type_vector ((void *)DS->rules, field1);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" users :"); }
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
  print_ds_type_vector ((void *)DS->users, field2);
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int print_ds_constructor_account_sent_change_phone_code (struct tl_ds_account_sent_change_phone_code *DS, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0xa4f58c4c && T->type->name != 0x5b0a73b3)) { return -1; }
  eprintf (" account.sentChangePhoneCode");
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" phone_code_hash :"); }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_string (DS->phone_code_hash, field1);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" send_call_timeout :"); }
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_int (DS->send_call_timeout, field2);
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int print_ds_constructor_auth_authorization (struct tl_ds_auth_authorization *DS, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0xff036af1 && T->type->name != 0x00fc950e)) { return -1; }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x02e6c0c8, .id = "User", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_user (DS->user, field1);
  return 0;
}
int print_ds_constructor_auth_checked_phone (struct tl_ds_auth_checked_phone *DS, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x811ea28e && T->type->name != 0x7ee15d71)) { return -1; }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x250be282, .id = "Bool", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bool (DS->phone_registered, field1);
  return 0;
}
int print_ds_constructor_auth_exported_authorization (struct tl_ds_auth_exported_authorization *DS, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0xdf969c2d && T->type->name != 0x206963d2)) { return -1; }
  eprintf (" auth.exportedAuthorization");
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" id :"); }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_int (DS->id, field1);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" bytes :"); }
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xf11ec860, .id = "Bare_Bytes", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_bytes (DS->bytes, field2);
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int print_ds_constructor_auth_password_recovery (struct tl_ds_auth_password_recovery *DS, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x137948a5 && T->type->name != 0xec86b75a)) { return -1; }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_string (DS->email_pattern, field1);
  return 0;
}
int print_ds_constructor_auth_sent_code (struct tl_ds_auth_sent_code *DS, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x0cc8bc16 && T->type->name != 0xf33743e9)) { return -1; }
  eprintf (" auth.sentCode");
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" phone_registered :"); }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x250be282, .id = "Bool", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bool (DS->phone_registered, field1);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" phone_code_hash :"); }
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_string (DS->phone_code_hash, field2);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" send_call_timeout :"); }
  struct paramed_type *field3 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_int (DS->send_call_timeout, field3);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" is_password :"); }
  struct paramed_type *field4 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x250be282, .id = "Bool", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bool (DS->is_password, field4);
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int print_ds_constructor_auth_sent_app_code (struct tl_ds_auth_sent_code *DS, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x0cc8bc16 && T->type->name != 0xf33743e9)) { return -1; }
  eprintf (" auth.sentAppCode");
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" phone_registered :"); }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x250be282, .id = "Bool", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bool (DS->phone_registered, field1);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" phone_code_hash :"); }
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_string (DS->phone_code_hash, field2);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" send_call_timeout :"); }
  struct paramed_type *field3 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_int (DS->send_call_timeout, field3);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" is_password :"); }
  struct paramed_type *field4 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x250be282, .id = "Bool", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bool (DS->is_password, field4);
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int print_ds_constructor_binlog_encr_key (struct tl_ds_binlog_encr_key *DS, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x0377168f && T->type->name != 0xfc88e970)) { return -1; }
  int multiplicity1 = PTR2INT (
  INT2PTR (64)  );
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  eprintf (" [");
  if (multiline_output >= 1) { eprintf ("\n"); }
  if (multiline_output >= 1) { multiline_offset += multiline_offset_size;}
  {
    int i = 0;
    while (i < multiplicity1) {
      if (multiline_output >= 1) { print_offset (); }
      print_ds_type_any (DS->key[i ++], field1);
      if (multiline_output >= 1) { eprintf ("\n"); }
    }
  }
  if (multiline_output >= 1) { multiline_offset -= multiline_offset_size; print_offset ();}
  eprintf (" ]");
  return 0;
}
int print_ds_constructor_binlog_start (struct tl_ds_binlog_update *DS, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x66ff03a6 && T->type->name != 0x9900fc59)) { return -1; }
  eprintf (" binlog.start");
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int print_ds_constructor_binlog_dc_option (struct tl_ds_binlog_update *DS, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x66ff03a6 && T->type->name != 0x9900fc59)) { return -1; }
  eprintf (" binlog.dcOption");
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" dc :"); }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_int (DS->dc, field1);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" name :"); }
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_string (DS->name, field2);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" ip :"); }
  struct paramed_type *field3 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_string (DS->ip, field3);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" port :"); }
  struct paramed_type *field4 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_int (DS->port, field4);
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int print_ds_constructor_binlog_dc_option_new (struct tl_ds_binlog_update *DS, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x66ff03a6 && T->type->name != 0x9900fc59)) { return -1; }
  eprintf (" binlog.dcOptionNew");
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" flags :"); }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_int (DS->flags, field1);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" dc :"); }
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_int (DS->dc, field2);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" name :"); }
  struct paramed_type *field3 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_string (DS->name, field3);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" ip :"); }
  struct paramed_type *field4 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_string (DS->ip, field4);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" port :"); }
  struct paramed_type *field5 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_int (DS->port, field5);
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int print_ds_constructor_binlog_auth_key (struct tl_ds_binlog_update *DS, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x66ff03a6 && T->type->name != 0x9900fc59)) { return -1; }
  eprintf (" binlog.authKey");
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" dc :"); }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_int (DS->dc, field1);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" key :"); }
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xfc88e970, .id = "Bare_binlog.EncrKey", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_binlog_encr_key (DS->key, field2);
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int print_ds_constructor_binlog_default_dc (struct tl_ds_binlog_update *DS, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x66ff03a6 && T->type->name != 0x9900fc59)) { return -1; }
  eprintf (" binlog.defaultDc");
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" dc :"); }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_int (DS->dc, field1);
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int print_ds_constructor_binlog_dc_signed (struct tl_ds_binlog_update *DS, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x66ff03a6 && T->type->name != 0x9900fc59)) { return -1; }
  eprintf (" binlog.dcSigned");
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" dc :"); }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_int (DS->dc, field1);
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int print_ds_constructor_binlog_our_id (struct tl_ds_binlog_update *DS, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x66ff03a6 && T->type->name != 0x9900fc59)) { return -1; }
  eprintf (" binlog.ourId");
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" id :"); }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_int (DS->id, field1);
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int print_ds_constructor_binlog_set_dh_params (struct tl_ds_binlog_update *DS, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x66ff03a6 && T->type->name != 0x9900fc59)) { return -1; }
  eprintf (" binlog.setDhParams");
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" root :"); }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_int (DS->root, field1);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" prime :"); }
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xfc88e970, .id = "Bare_binlog.EncrKey", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_binlog_encr_key (DS->prime, field2);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" version :"); }
  struct paramed_type *field3 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_int (DS->version, field3);
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int print_ds_constructor_binlog_set_pts (struct tl_ds_binlog_update *DS, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x66ff03a6 && T->type->name != 0x9900fc59)) { return -1; }
  eprintf (" binlog.setPts");
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" pts :"); }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_int (DS->pts, field1);
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int print_ds_constructor_binlog_set_qts (struct tl_ds_binlog_update *DS, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x66ff03a6 && T->type->name != 0x9900fc59)) { return -1; }
  eprintf (" binlog.setQts");
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" qts :"); }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_int (DS->qts, field1);
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int print_ds_constructor_binlog_set_date (struct tl_ds_binlog_update *DS, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x66ff03a6 && T->type->name != 0x9900fc59)) { return -1; }
  eprintf (" binlog.setDate");
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" date :"); }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_int (DS->date, field1);
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int print_ds_constructor_binlog_set_seq (struct tl_ds_binlog_update *DS, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x66ff03a6 && T->type->name != 0x9900fc59)) { return -1; }
  eprintf (" binlog.setSeq");
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" seq :"); }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_int (DS->seq, field1);
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int print_ds_constructor_binlog_encr_chat_delete (struct tl_ds_binlog_update *DS, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x66ff03a6 && T->type->name != 0x9900fc59)) { return -1; }
  eprintf (" binlog.encrChatDelete");
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" id :"); }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_int (DS->id, field1);
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int print_ds_constructor_binlog_encr_chat_new (struct tl_ds_binlog_update *DS, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x66ff03a6 && T->type->name != 0x9900fc59)) { return -1; }
  eprintf (" binlog.encrChatNew");
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" flags :"); }
  struct paramed_type *var0 = INT2PTR (*DS->flags);
  eprintf (" %d", (int)PTR2INT (var0));
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" id :"); }
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_int (DS->id, field2);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (PTR2INT (var0) & (1 << 17)) {
    if (multiline_output >= 2) { print_offset (); }
    if (!disable_field_names) { eprintf (" access_hash :"); }
    struct paramed_type *field3 = 
    &(struct paramed_type){
      .type = &(struct tl_type_descr) {.name = 0xddf89345, .id = "Bare_Long", .params_num = 0, .params_types = 0},
      .params = 0,
    };
    print_ds_type_bare_long (DS->access_hash, field3);
    if (multiline_output >= 2) { eprintf ("\n"); }
  }
  if (PTR2INT (var0) & (1 << 18)) {
    if (multiline_output >= 2) { print_offset (); }
    if (!disable_field_names) { eprintf (" date :"); }
    struct paramed_type *field4 = 
    &(struct paramed_type){
      .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
      .params = 0,
    };
    print_ds_type_bare_int (DS->date, field4);
    if (multiline_output >= 2) { eprintf ("\n"); }
  }
  if (PTR2INT (var0) & (1 << 19)) {
    if (multiline_output >= 2) { print_offset (); }
    if (!disable_field_names) { eprintf (" admin :"); }
    struct paramed_type *field5 = 
    &(struct paramed_type){
      .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
      .params = 0,
    };
    print_ds_type_bare_int (DS->admin, field5);
    if (multiline_output >= 2) { eprintf ("\n"); }
  }
  if (PTR2INT (var0) & (1 << 20)) {
    if (multiline_output >= 2) { print_offset (); }
    if (!disable_field_names) { eprintf (" user_id :"); }
    struct paramed_type *field6 = 
    &(struct paramed_type){
      .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
      .params = 0,
    };
    print_ds_type_bare_int (DS->user_id, field6);
    if (multiline_output >= 2) { eprintf ("\n"); }
  }
  if (PTR2INT (var0) & (1 << 21)) {
    if (multiline_output >= 2) { print_offset (); }
    if (!disable_field_names) { eprintf (" key :"); }
    struct paramed_type *field7 = 
    &(struct paramed_type){
      .type = &(struct tl_type_descr) {.name = 0xfc88e970, .id = "Bare_binlog.EncrKey", .params_num = 0, .params_types = 0},
      .params = 0,
    };
    print_ds_type_bare_binlog_encr_key (DS->key, field7);
    if (multiline_output >= 2) { eprintf ("\n"); }
  }
  if (PTR2INT (var0) & (1 << 22)) {
    if (multiline_output >= 2) { print_offset (); }
    if (!disable_field_names) { eprintf (" g_key :"); }
    struct paramed_type *field8 = 
    &(struct paramed_type){
      .type = &(struct tl_type_descr) {.name = 0xfc88e970, .id = "Bare_binlog.EncrKey", .params_num = 0, .params_types = 0},
      .params = 0,
    };
    print_ds_type_bare_binlog_encr_key (DS->g_key, field8);
    if (multiline_output >= 2) { eprintf ("\n"); }
  }
  if (PTR2INT (var0) & (1 << 23)) {
    if (multiline_output >= 2) { print_offset (); }
    if (!disable_field_names) { eprintf (" state :"); }
    struct paramed_type *field9 = 
    &(struct paramed_type){
      .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
      .params = 0,
    };
    print_ds_type_bare_int (DS->state, field9);
    if (multiline_output >= 2) { eprintf ("\n"); }
  }
  if (PTR2INT (var0) & (1 << 24)) {
    if (multiline_output >= 2) { print_offset (); }
    if (!disable_field_names) { eprintf (" ttl :"); }
    struct paramed_type *field10 = 
    &(struct paramed_type){
      .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
      .params = 0,
    };
    print_ds_type_bare_int (DS->ttl, field10);
    if (multiline_output >= 2) { eprintf ("\n"); }
  }
  if (PTR2INT (var0) & (1 << 25)) {
    if (multiline_output >= 2) { print_offset (); }
    if (!disable_field_names) { eprintf (" layer :"); }
    struct paramed_type *field11 = 
    &(struct paramed_type){
      .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
      .params = 0,
    };
    print_ds_type_bare_int (DS->layer, field11);
    if (multiline_output >= 2) { eprintf ("\n"); }
  }
  if (PTR2INT (var0) & (1 << 26)) {
    if (multiline_output >= 2) { print_offset (); }
    if (!disable_field_names) { eprintf (" in_seq_no :"); }
    struct paramed_type *field12 = 
    &(struct paramed_type){
      .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
      .params = 0,
    };
    print_ds_type_bare_int (DS->in_seq_no, field12);
    if (multiline_output >= 2) { eprintf ("\n"); }
  }
  if (PTR2INT (var0) & (1 << 26)) {
    if (multiline_output >= 2) { print_offset (); }
    if (!disable_field_names) { eprintf (" last_in_seq_no :"); }
    struct paramed_type *field13 = 
    &(struct paramed_type){
      .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
      .params = 0,
    };
    print_ds_type_bare_int (DS->last_in_seq_no, field13);
    if (multiline_output >= 2) { eprintf ("\n"); }
  }
  if (PTR2INT (var0) & (1 << 26)) {
    if (multiline_output >= 2) { print_offset (); }
    if (!disable_field_names) { eprintf (" out_seq_no :"); }
    struct paramed_type *field14 = 
    &(struct paramed_type){
      .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
      .params = 0,
    };
    print_ds_type_bare_int (DS->out_seq_no, field14);
    if (multiline_output >= 2) { eprintf ("\n"); }
  }
  if (PTR2INT (var0) & (1 << 27)) {
    if (multiline_output >= 2) { print_offset (); }
    if (!disable_field_names) { eprintf (" key_fingerprint :"); }
    struct paramed_type *field15 = 
    &(struct paramed_type){
      .type = &(struct tl_type_descr) {.name = 0xddf89345, .id = "Bare_Long", .params_num = 0, .params_types = 0},
      .params = 0,
    };
    print_ds_type_bare_long (DS->key_fingerprint, field15);
    if (multiline_output >= 2) { eprintf ("\n"); }
  }
  return 0;
}
int print_ds_constructor_binlog_encr_chat_exchange_new (struct tl_ds_binlog_update *DS, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x66ff03a6 && T->type->name != 0x9900fc59)) { return -1; }
  eprintf (" binlog.encrChatExchangeNew");
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" flags :"); }
  struct paramed_type *var0 = INT2PTR (*DS->flags);
  eprintf (" %d", (int)PTR2INT (var0));
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" id :"); }
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_int (DS->id, field2);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (PTR2INT (var0) & (1 << 17)) {
    if (multiline_output >= 2) { print_offset (); }
    if (!disable_field_names) { eprintf (" exchange_id :"); }
    struct paramed_type *field3 = 
    &(struct paramed_type){
      .type = &(struct tl_type_descr) {.name = 0xddf89345, .id = "Bare_Long", .params_num = 0, .params_types = 0},
      .params = 0,
    };
    print_ds_type_bare_long (DS->exchange_id, field3);
    if (multiline_output >= 2) { eprintf ("\n"); }
  }
  if (PTR2INT (var0) & (1 << 18)) {
    if (multiline_output >= 2) { print_offset (); }
    if (!disable_field_names) { eprintf (" key :"); }
    struct paramed_type *field4 = 
    &(struct paramed_type){
      .type = &(struct tl_type_descr) {.name = 0xfc88e970, .id = "Bare_binlog.EncrKey", .params_num = 0, .params_types = 0},
      .params = 0,
    };
    print_ds_type_bare_binlog_encr_key (DS->key, field4);
    if (multiline_output >= 2) { eprintf ("\n"); }
  }
  if (PTR2INT (var0) & (1 << 19)) {
    if (multiline_output >= 2) { print_offset (); }
    if (!disable_field_names) { eprintf (" state :"); }
    struct paramed_type *field5 = 
    &(struct paramed_type){
      .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
      .params = 0,
    };
    print_ds_type_bare_int (DS->state, field5);
    if (multiline_output >= 2) { eprintf ("\n"); }
  }
  return 0;
}
int print_ds_constructor_binlog_user_delete (struct tl_ds_binlog_update *DS, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x66ff03a6 && T->type->name != 0x9900fc59)) { return -1; }
  eprintf (" binlog.userDelete");
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" id :"); }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_int (DS->id, field1);
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int print_ds_constructor_binlog_user_new (struct tl_ds_binlog_update *DS, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x66ff03a6 && T->type->name != 0x9900fc59)) { return -1; }
  eprintf (" binlog.userNew");
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" flags :"); }
  struct paramed_type *var0 = INT2PTR (*DS->flags);
  eprintf (" %d", (int)PTR2INT (var0));
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" id :"); }
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_int (DS->id, field2);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (PTR2INT (var0) & (1 << 17)) {
    if (multiline_output >= 2) { print_offset (); }
    if (!disable_field_names) { eprintf (" access_hash :"); }
    struct paramed_type *field3 = 
    &(struct paramed_type){
      .type = &(struct tl_type_descr) {.name = 0xddf89345, .id = "Bare_Long", .params_num = 0, .params_types = 0},
      .params = 0,
    };
    print_ds_type_bare_long (DS->access_hash, field3);
    if (multiline_output >= 2) { eprintf ("\n"); }
  }
  if (PTR2INT (var0) & (1 << 18)) {
    if (multiline_output >= 2) { print_offset (); }
    if (!disable_field_names) { eprintf (" first_name :"); }
    struct paramed_type *field4 = 
    &(struct paramed_type){
      .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
      .params = 0,
    };
    print_ds_type_bare_string (DS->first_name, field4);
    if (multiline_output >= 2) { eprintf ("\n"); }
  }
  if (PTR2INT (var0) & (1 << 18)) {
    if (multiline_output >= 2) { print_offset (); }
    if (!disable_field_names) { eprintf (" last_name :"); }
    struct paramed_type *field5 = 
    &(struct paramed_type){
      .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
      .params = 0,
    };
    print_ds_type_bare_string (DS->last_name, field5);
    if (multiline_output >= 2) { eprintf ("\n"); }
  }
  if (PTR2INT (var0) & (1 << 19)) {
    if (multiline_output >= 2) { print_offset (); }
    if (!disable_field_names) { eprintf (" phone :"); }
    struct paramed_type *field6 = 
    &(struct paramed_type){
      .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
      .params = 0,
    };
    print_ds_type_bare_string (DS->phone, field6);
    if (multiline_output >= 2) { eprintf ("\n"); }
  }
  if (PTR2INT (var0) & (1 << 20)) {
    if (multiline_output >= 2) { print_offset (); }
    if (!disable_field_names) { eprintf (" username :"); }
    struct paramed_type *field7 = 
    &(struct paramed_type){
      .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
      .params = 0,
    };
    print_ds_type_bare_string (DS->username, field7);
    if (multiline_output >= 2) { eprintf ("\n"); }
  }
  if (PTR2INT (var0) & (1 << 21)) {
    if (multiline_output >= 2) { print_offset (); }
    if (!disable_field_names) { eprintf (" photo :"); }
    struct paramed_type *field8 = 
    &(struct paramed_type){
      .type = &(struct tl_type_descr) {.name = 0xc207550a, .id = "Photo", .params_num = 0, .params_types = 0},
      .params = 0,
    };
    print_ds_type_photo (DS->photo, field8);
    if (multiline_output >= 2) { eprintf ("\n"); }
  }
  if (PTR2INT (var0) & (1 << 22)) {
    if (multiline_output >= 2) { print_offset (); }
    if (!disable_field_names) { eprintf (" real_first_name :"); }
    struct paramed_type *field9 = 
    &(struct paramed_type){
      .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
      .params = 0,
    };
    print_ds_type_bare_string (DS->real_first_name, field9);
    if (multiline_output >= 2) { eprintf ("\n"); }
  }
  if (PTR2INT (var0) & (1 << 22)) {
    if (multiline_output >= 2) { print_offset (); }
    if (!disable_field_names) { eprintf (" real_last_name :"); }
    struct paramed_type *field10 = 
    &(struct paramed_type){
      .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
      .params = 0,
    };
    print_ds_type_bare_string (DS->real_last_name, field10);
    if (multiline_output >= 2) { eprintf ("\n"); }
  }
  if (PTR2INT (var0) & (1 << 23)) {
    if (multiline_output >= 2) { print_offset (); }
    if (!disable_field_names) { eprintf (" user_photo :"); }
    struct paramed_type *field11 = 
    &(struct paramed_type){
      .type = &(struct tl_type_descr) {.name = 0x9a486229, .id = "UserProfilePhoto", .params_num = 0, .params_types = 0},
      .params = 0,
    };
    print_ds_type_user_profile_photo (DS->user_photo, field11);
    if (multiline_output >= 2) { eprintf ("\n"); }
  }
  if (PTR2INT (var0) & (1 << 24)) {
    if (multiline_output >= 2) { print_offset (); }
    if (!disable_field_names) { eprintf (" last_read_in :"); }
    struct paramed_type *field12 = 
    &(struct paramed_type){
      .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
      .params = 0,
    };
    print_ds_type_bare_int (DS->last_read_in, field12);
    if (multiline_output >= 2) { eprintf ("\n"); }
  }
  if (PTR2INT (var0) & (1 << 25)) {
    if (multiline_output >= 2) { print_offset (); }
    if (!disable_field_names) { eprintf (" last_read_out :"); }
    struct paramed_type *field13 = 
    &(struct paramed_type){
      .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
      .params = 0,
    };
    print_ds_type_bare_int (DS->last_read_out, field13);
    if (multiline_output >= 2) { eprintf ("\n"); }
  }
  if (PTR2INT (var0) & (1 << 26)) {
    if (multiline_output >= 2) { print_offset (); }
    if (!disable_field_names) { eprintf (" bot_info :"); }
    struct paramed_type *field14 = 
    &(struct paramed_type){
      .type = &(struct tl_type_descr) {.name = 0xb2e16f93, .id = "BotInfo", .params_num = 0, .params_types = 0},
      .params = 0,
    };
    print_ds_type_bot_info (DS->bot_info, field14);
    if (multiline_output >= 2) { eprintf ("\n"); }
  }
  return 0;
}
int print_ds_constructor_binlog_chat_new (struct tl_ds_binlog_update *DS, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x66ff03a6 && T->type->name != 0x9900fc59)) { return -1; }
  eprintf (" binlog.chatNew");
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" flags :"); }
  struct paramed_type *var0 = INT2PTR (*DS->flags);
  eprintf (" %d", (int)PTR2INT (var0));
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" id :"); }
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_int (DS->id, field2);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (PTR2INT (var0) & (1 << 17)) {
    if (multiline_output >= 2) { print_offset (); }
    if (!disable_field_names) { eprintf (" title :"); }
    struct paramed_type *field3 = 
    &(struct paramed_type){
      .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
      .params = 0,
    };
    print_ds_type_bare_string (DS->title, field3);
    if (multiline_output >= 2) { eprintf ("\n"); }
  }
  if (PTR2INT (var0) & (1 << 18)) {
    if (multiline_output >= 2) { print_offset (); }
    if (!disable_field_names) { eprintf (" user_num :"); }
    struct paramed_type *field4 = 
    &(struct paramed_type){
      .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
      .params = 0,
    };
    print_ds_type_bare_int (DS->user_num, field4);
    if (multiline_output >= 2) { eprintf ("\n"); }
  }
  if (PTR2INT (var0) & (1 << 19)) {
    if (multiline_output >= 2) { print_offset (); }
    if (!disable_field_names) { eprintf (" date :"); }
    struct paramed_type *field5 = 
    &(struct paramed_type){
      .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
      .params = 0,
    };
    print_ds_type_bare_int (DS->date, field5);
    if (multiline_output >= 2) { eprintf ("\n"); }
  }
  if (PTR2INT (var0) & (1 << 20)) {
    if (multiline_output >= 2) { print_offset (); }
    if (!disable_field_names) { eprintf (" version :"); }
    struct paramed_type *field6 = 
    &(struct paramed_type){
      .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
      .params = 0,
    };
    print_ds_type_bare_int (DS->version, field6);
    if (multiline_output >= 2) { eprintf ("\n"); }
  }
  if (PTR2INT (var0) & (1 << 20)) {
    if (multiline_output >= 2) { print_offset (); }
    if (!disable_field_names) { eprintf (" participants :"); }
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
    print_ds_type_vector ((void *)DS->participants, field7);
    if (multiline_output >= 2) { eprintf ("\n"); }
  }
  if (PTR2INT (var0) & (1 << 21)) {
    if (multiline_output >= 2) { print_offset (); }
    if (!disable_field_names) { eprintf (" chat_photo :"); }
    struct paramed_type *field8 = 
    &(struct paramed_type){
      .type = &(struct tl_type_descr) {.name = 0x56922676, .id = "ChatPhoto", .params_num = 0, .params_types = 0},
      .params = 0,
    };
    print_ds_type_chat_photo (DS->chat_photo, field8);
    if (multiline_output >= 2) { eprintf ("\n"); }
  }
  if (PTR2INT (var0) & (1 << 22)) {
    if (multiline_output >= 2) { print_offset (); }
    if (!disable_field_names) { eprintf (" photo :"); }
    struct paramed_type *field9 = 
    &(struct paramed_type){
      .type = &(struct tl_type_descr) {.name = 0xc207550a, .id = "Photo", .params_num = 0, .params_types = 0},
      .params = 0,
    };
    print_ds_type_photo (DS->photo, field9);
    if (multiline_output >= 2) { eprintf ("\n"); }
  }
  if (PTR2INT (var0) & (1 << 23)) {
    if (multiline_output >= 2) { print_offset (); }
    if (!disable_field_names) { eprintf (" admin :"); }
    struct paramed_type *field10 = 
    &(struct paramed_type){
      .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
      .params = 0,
    };
    print_ds_type_bare_int (DS->admin, field10);
    if (multiline_output >= 2) { eprintf ("\n"); }
  }
  if (PTR2INT (var0) & (1 << 24)) {
    if (multiline_output >= 2) { print_offset (); }
    if (!disable_field_names) { eprintf (" last_read_in :"); }
    struct paramed_type *field11 = 
    &(struct paramed_type){
      .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
      .params = 0,
    };
    print_ds_type_bare_int (DS->last_read_in, field11);
    if (multiline_output >= 2) { eprintf ("\n"); }
  }
  if (PTR2INT (var0) & (1 << 25)) {
    if (multiline_output >= 2) { print_offset (); }
    if (!disable_field_names) { eprintf (" last_read_out :"); }
    struct paramed_type *field12 = 
    &(struct paramed_type){
      .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
      .params = 0,
    };
    print_ds_type_bare_int (DS->last_read_out, field12);
    if (multiline_output >= 2) { eprintf ("\n"); }
  }
  return 0;
}
int print_ds_constructor_binlog_chat_add_participant (struct tl_ds_binlog_update *DS, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x66ff03a6 && T->type->name != 0x9900fc59)) { return -1; }
  eprintf (" binlog.chatAddParticipant");
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" id :"); }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_int (DS->id, field1);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" version :"); }
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_int (DS->version, field2);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" user_id :"); }
  struct paramed_type *field3 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_int (DS->user_id, field3);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" inviter_id :"); }
  struct paramed_type *field4 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_int (DS->inviter_id, field4);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" date :"); }
  struct paramed_type *field5 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_int (DS->date, field5);
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int print_ds_constructor_binlog_chat_del_participant (struct tl_ds_binlog_update *DS, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x66ff03a6 && T->type->name != 0x9900fc59)) { return -1; }
  eprintf (" binlog.chatDelParticipant");
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" id :"); }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_int (DS->id, field1);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" version :"); }
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_int (DS->version, field2);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" user_id :"); }
  struct paramed_type *field3 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_int (DS->user_id, field3);
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int print_ds_constructor_binlog_set_msg_id (struct tl_ds_binlog_update *DS, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x66ff03a6 && T->type->name != 0x9900fc59)) { return -1; }
  eprintf (" binlog.setMsgId");
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" old_id :"); }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xddf89345, .id = "Bare_Long", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_long (DS->old_id, field1);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" new_id :"); }
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_int (DS->new_id, field2);
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int print_ds_constructor_binlog_message_delete (struct tl_ds_binlog_update *DS, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x66ff03a6 && T->type->name != 0x9900fc59)) { return -1; }
  eprintf (" binlog.messageDelete");
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" lid :"); }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xddf89345, .id = "Bare_Long", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_long (DS->lid, field1);
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int print_ds_constructor_binlog_message_new (struct tl_ds_binlog_update *DS, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x66ff03a6 && T->type->name != 0x9900fc59)) { return -1; }
  eprintf (" binlog.messageNew");
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" flags :"); }
  struct paramed_type *var0 = INT2PTR (*DS->flags);
  eprintf (" %d", (int)PTR2INT (var0));
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" lid :"); }
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xddf89345, .id = "Bare_Long", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_long (DS->lid, field2);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (PTR2INT (var0) & (1 << 17)) {
    if (multiline_output >= 2) { print_offset (); }
    if (!disable_field_names) { eprintf (" from_id :"); }
    struct paramed_type *field3 = 
    &(struct paramed_type){
      .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
      .params = 0,
    };
    print_ds_type_bare_int (DS->from_id, field3);
    if (multiline_output >= 2) { eprintf ("\n"); }
  }
  if (PTR2INT (var0) & (1 << 17)) {
    if (multiline_output >= 2) { print_offset (); }
    if (!disable_field_names) { eprintf (" to_type :"); }
    struct paramed_type *field4 = 
    &(struct paramed_type){
      .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
      .params = 0,
    };
    print_ds_type_bare_int (DS->to_type, field4);
    if (multiline_output >= 2) { eprintf ("\n"); }
  }
  if (PTR2INT (var0) & (1 << 17)) {
    if (multiline_output >= 2) { print_offset (); }
    if (!disable_field_names) { eprintf (" to_id :"); }
    struct paramed_type *field5 = 
    &(struct paramed_type){
      .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
      .params = 0,
    };
    print_ds_type_bare_int (DS->to_id, field5);
    if (multiline_output >= 2) { eprintf ("\n"); }
  }
  if (PTR2INT (var0) & (1 << 18)) {
    if (multiline_output >= 2) { print_offset (); }
    if (!disable_field_names) { eprintf (" fwd_from_id :"); }
    struct paramed_type *field6 = 
    &(struct paramed_type){
      .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
      .params = 0,
    };
    print_ds_type_bare_int (DS->fwd_from_id, field6);
    if (multiline_output >= 2) { eprintf ("\n"); }
  }
  if (PTR2INT (var0) & (1 << 18)) {
    if (multiline_output >= 2) { print_offset (); }
    if (!disable_field_names) { eprintf (" fwd_date :"); }
    struct paramed_type *field7 = 
    &(struct paramed_type){
      .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
      .params = 0,
    };
    print_ds_type_bare_int (DS->fwd_date, field7);
    if (multiline_output >= 2) { eprintf ("\n"); }
  }
  if (PTR2INT (var0) & (1 << 19)) {
    if (multiline_output >= 2) { print_offset (); }
    if (!disable_field_names) { eprintf (" date :"); }
    struct paramed_type *field8 = 
    &(struct paramed_type){
      .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
      .params = 0,
    };
    print_ds_type_bare_int (DS->date, field8);
    if (multiline_output >= 2) { eprintf ("\n"); }
  }
  if (PTR2INT (var0) & (1 << 20)) {
    if (multiline_output >= 2) { print_offset (); }
    if (!disable_field_names) { eprintf (" message :"); }
    struct paramed_type *field9 = 
    &(struct paramed_type){
      .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
      .params = 0,
    };
    print_ds_type_bare_string (DS->message, field9);
    if (multiline_output >= 2) { eprintf ("\n"); }
  }
  if (PTR2INT (var0) & (1 << 21)) {
    if (multiline_output >= 2) { print_offset (); }
    if (!disable_field_names) { eprintf (" media :"); }
    struct paramed_type *field10 = 
    &(struct paramed_type){
      .type = &(struct tl_type_descr) {.name = 0x95f2471a, .id = "MessageMedia", .params_num = 0, .params_types = 0},
      .params = 0,
    };
    print_ds_type_message_media (DS->media, field10);
    if (multiline_output >= 2) { eprintf ("\n"); }
  }
  if (PTR2INT (var0) & (1 << 22)) {
    if (multiline_output >= 2) { print_offset (); }
    if (!disable_field_names) { eprintf (" action :"); }
    struct paramed_type *field11 = 
    &(struct paramed_type){
      .type = &(struct tl_type_descr) {.name = 0x383415fa, .id = "MessageAction", .params_num = 0, .params_types = 0},
      .params = 0,
    };
    print_ds_type_message_action (DS->action, field11);
    if (multiline_output >= 2) { eprintf ("\n"); }
  }
  if (PTR2INT (var0) & (1 << 23)) {
    if (multiline_output >= 2) { print_offset (); }
    if (!disable_field_names) { eprintf (" reply_id :"); }
    struct paramed_type *field12 = 
    &(struct paramed_type){
      .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
      .params = 0,
    };
    print_ds_type_bare_int (DS->reply_id, field12);
    if (multiline_output >= 2) { eprintf ("\n"); }
  }
  if (PTR2INT (var0) & (1 << 24)) {
    if (multiline_output >= 2) { print_offset (); }
    if (!disable_field_names) { eprintf (" reply_markup :"); }
    struct paramed_type *field13 = 
    &(struct paramed_type){
      .type = &(struct tl_type_descr) {.name = 0x612ca4a9, .id = "ReplyMarkup", .params_num = 0, .params_types = 0},
      .params = 0,
    };
    print_ds_type_reply_markup (DS->reply_markup, field13);
    if (multiline_output >= 2) { eprintf ("\n"); }
  }
  return 0;
}
int print_ds_constructor_binlog_message_encr_new (struct tl_ds_binlog_update *DS, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x66ff03a6 && T->type->name != 0x9900fc59)) { return -1; }
  eprintf (" binlog.messageEncrNew");
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" flags :"); }
  struct paramed_type *var0 = INT2PTR (*DS->flags);
  eprintf (" %d", (int)PTR2INT (var0));
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" lid :"); }
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xddf89345, .id = "Bare_Long", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_long (DS->lid, field2);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (PTR2INT (var0) & (1 << 17)) {
    if (multiline_output >= 2) { print_offset (); }
    if (!disable_field_names) { eprintf (" from_id :"); }
    struct paramed_type *field3 = 
    &(struct paramed_type){
      .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
      .params = 0,
    };
    print_ds_type_bare_int (DS->from_id, field3);
    if (multiline_output >= 2) { eprintf ("\n"); }
  }
  if (PTR2INT (var0) & (1 << 17)) {
    if (multiline_output >= 2) { print_offset (); }
    if (!disable_field_names) { eprintf (" to_type :"); }
    struct paramed_type *field4 = 
    &(struct paramed_type){
      .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
      .params = 0,
    };
    print_ds_type_bare_int (DS->to_type, field4);
    if (multiline_output >= 2) { eprintf ("\n"); }
  }
  if (PTR2INT (var0) & (1 << 17)) {
    if (multiline_output >= 2) { print_offset (); }
    if (!disable_field_names) { eprintf (" to_id :"); }
    struct paramed_type *field5 = 
    &(struct paramed_type){
      .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
      .params = 0,
    };
    print_ds_type_bare_int (DS->to_id, field5);
    if (multiline_output >= 2) { eprintf ("\n"); }
  }
  if (PTR2INT (var0) & (1 << 19)) {
    if (multiline_output >= 2) { print_offset (); }
    if (!disable_field_names) { eprintf (" date :"); }
    struct paramed_type *field6 = 
    &(struct paramed_type){
      .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
      .params = 0,
    };
    print_ds_type_bare_int (DS->date, field6);
    if (multiline_output >= 2) { eprintf ("\n"); }
  }
  if (PTR2INT (var0) & (1 << 20)) {
    if (multiline_output >= 2) { print_offset (); }
    if (!disable_field_names) { eprintf (" message :"); }
    struct paramed_type *field7 = 
    &(struct paramed_type){
      .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
      .params = 0,
    };
    print_ds_type_bare_string (DS->message, field7);
    if (multiline_output >= 2) { eprintf ("\n"); }
  }
  if (PTR2INT (var0) & (1 << 21)) {
    if (multiline_output >= 2) { print_offset (); }
    if (!disable_field_names) { eprintf (" encr_media :"); }
    struct paramed_type *field8 = 
    &(struct paramed_type){
      .type = &(struct tl_type_descr) {.name = 0x34e0d674, .id = "DecryptedMessageMedia", .params_num = 0, .params_types = 0},
      .params = 0,
    };
    print_ds_type_decrypted_message_media (DS->encr_media, field8);
    if (multiline_output >= 2) { eprintf ("\n"); }
  }
  if (PTR2INT (var0) & (1 << 22)) {
    if (multiline_output >= 2) { print_offset (); }
    if (!disable_field_names) { eprintf (" encr_action :"); }
    struct paramed_type *field9 = 
    &(struct paramed_type){
      .type = &(struct tl_type_descr) {.name = 0x4e0eefde, .id = "DecryptedMessageAction", .params_num = 0, .params_types = 0},
      .params = 0,
    };
    print_ds_type_decrypted_message_action (DS->encr_action, field9);
    if (multiline_output >= 2) { eprintf ("\n"); }
  }
  if (PTR2INT (var0) & (1 << 23)) {
    if (multiline_output >= 2) { print_offset (); }
    if (!disable_field_names) { eprintf (" file :"); }
    struct paramed_type *field10 = 
    &(struct paramed_type){
      .type = &(struct tl_type_descr) {.name = 0x886fd032, .id = "EncryptedFile", .params_num = 0, .params_types = 0},
      .params = 0,
    };
    print_ds_type_encrypted_file (DS->file, field10);
    if (multiline_output >= 2) { eprintf ("\n"); }
  }
  return 0;
}
int print_ds_constructor_binlog_msg_update (struct tl_ds_binlog_update *DS, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x66ff03a6 && T->type->name != 0x9900fc59)) { return -1; }
  eprintf (" binlog.msgUpdate");
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" lid :"); }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xddf89345, .id = "Bare_Long", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_long (DS->lid, field1);
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int print_ds_constructor_binlog_reset_authorization (struct tl_ds_binlog_update *DS, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x66ff03a6 && T->type->name != 0x9900fc59)) { return -1; }
  eprintf (" binlog.resetAuthorization");
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int print_ds_constructor_contacts_blocked (struct tl_ds_contacts_blocked *DS, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x8c1b8fb4 && T->type->name != 0x73e4704b)) { return -1; }
  eprintf (" contacts.blocked");
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" blocked :"); }
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
  print_ds_type_vector ((void *)DS->blocked, field1);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" users :"); }
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
  print_ds_type_vector ((void *)DS->users, field2);
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int print_ds_constructor_contacts_blocked_slice (struct tl_ds_contacts_blocked *DS, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x8c1b8fb4 && T->type->name != 0x73e4704b)) { return -1; }
  eprintf (" contacts.blockedSlice");
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" count :"); }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_int (DS->count, field1);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" blocked :"); }
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
  print_ds_type_vector ((void *)DS->blocked, field2);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" users :"); }
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
  print_ds_type_vector ((void *)DS->users, field3);
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int print_ds_constructor_contacts_contacts_not_modified (struct tl_ds_contacts_contacts *DS, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0xd8c02560 && T->type->name != 0x273fda9f)) { return -1; }
  eprintf (" contacts.contactsNotModified");
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int print_ds_constructor_contacts_contacts (struct tl_ds_contacts_contacts *DS, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0xd8c02560 && T->type->name != 0x273fda9f)) { return -1; }
  eprintf (" contacts.contacts");
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" contacts :"); }
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
  print_ds_type_vector ((void *)DS->contacts, field1);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" users :"); }
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
  print_ds_type_vector ((void *)DS->users, field2);
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int print_ds_constructor_contacts_found (struct tl_ds_contacts_found *DS, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x0566000e && T->type->name != 0xfa99fff1)) { return -1; }
  eprintf (" contacts.found");
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" results :"); }
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
  print_ds_type_vector ((void *)DS->results, field1);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" users :"); }
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
  print_ds_type_vector ((void *)DS->users, field2);
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int print_ds_constructor_contacts_imported_contacts (struct tl_ds_contacts_imported_contacts *DS, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0xad524315 && T->type->name != 0x52adbcea)) { return -1; }
  eprintf (" contacts.importedContacts");
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" imported :"); }
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
  print_ds_type_vector ((void *)DS->imported, field1);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" retry_contacts :"); }
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
  print_ds_type_vector ((void *)DS->retry_contacts, field2);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" users :"); }
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
  print_ds_type_vector ((void *)DS->users, field3);
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int print_ds_constructor_contacts_link (struct tl_ds_contacts_link *DS, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x3ace484c && T->type->name != 0xc531b7b3)) { return -1; }
  eprintf (" contacts.link");
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" my_link :"); }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x522fbc63, .id = "ContactLink", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_contact_link (DS->my_link, field1);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" foreign_link :"); }
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x522fbc63, .id = "ContactLink", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_contact_link (DS->foreign_link, field2);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" user :"); }
  struct paramed_type *field3 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x02e6c0c8, .id = "User", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_user (DS->user, field3);
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int print_ds_constructor_contacts_suggested (struct tl_ds_contacts_suggested *DS, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x5649dcc5 && T->type->name != 0xa9b6233a)) { return -1; }
  eprintf (" contacts.suggested");
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" results :"); }
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
  print_ds_type_vector ((void *)DS->results, field1);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" users :"); }
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
  print_ds_type_vector ((void *)DS->users, field2);
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int print_ds_constructor_geochats_located (struct tl_ds_geochats_located *DS, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x48feb267 && T->type->name != 0xb7014d98)) { return -1; }
  eprintf (" geochats.located");
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" results :"); }
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
  print_ds_type_vector ((void *)DS->results, field1);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" messages :"); }
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
  print_ds_type_vector ((void *)DS->messages, field2);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" chats :"); }
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
  print_ds_type_vector ((void *)DS->chats, field3);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" users :"); }
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
  print_ds_type_vector ((void *)DS->users, field4);
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int print_ds_constructor_geochats_messages (struct tl_ds_geochats_messages *DS, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x6d0a0e59 && T->type->name != 0x92f5f1a6)) { return -1; }
  eprintf (" geochats.messages");
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" messages :"); }
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
  print_ds_type_vector ((void *)DS->messages, field1);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" chats :"); }
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
  print_ds_type_vector ((void *)DS->chats, field2);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" users :"); }
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
  print_ds_type_vector ((void *)DS->users, field3);
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int print_ds_constructor_geochats_messages_slice (struct tl_ds_geochats_messages *DS, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x6d0a0e59 && T->type->name != 0x92f5f1a6)) { return -1; }
  eprintf (" geochats.messagesSlice");
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" count :"); }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_int (DS->count, field1);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" messages :"); }
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
  print_ds_type_vector ((void *)DS->messages, field2);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" chats :"); }
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
  print_ds_type_vector ((void *)DS->chats, field3);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" users :"); }
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
  print_ds_type_vector ((void *)DS->users, field4);
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int print_ds_constructor_geochats_stated_message (struct tl_ds_geochats_stated_message *DS, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x17b1578b && T->type->name != 0xe84ea874)) { return -1; }
  eprintf (" geochats.statedMessage");
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" message :"); }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xf67b4034, .id = "GeoChatMessage", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_geo_chat_message (DS->message, field1);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" chats :"); }
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
  print_ds_type_vector ((void *)DS->chats, field2);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" users :"); }
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
  print_ds_type_vector ((void *)DS->users, field3);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" seq :"); }
  struct paramed_type *field4 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_int (DS->seq, field4);
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int print_ds_constructor_help_app_update (struct tl_ds_help_app_update *DS, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x4ddd9627 && T->type->name != 0xb22269d8)) { return -1; }
  eprintf (" help.appUpdate");
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" id :"); }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_int (DS->id, field1);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" critical :"); }
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x250be282, .id = "Bool", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bool (DS->critical, field2);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" url :"); }
  struct paramed_type *field3 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_string (DS->url, field3);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" text :"); }
  struct paramed_type *field4 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_string (DS->text, field4);
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int print_ds_constructor_help_no_app_update (struct tl_ds_help_app_update *DS, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x4ddd9627 && T->type->name != 0xb22269d8)) { return -1; }
  eprintf (" help.noAppUpdate");
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int print_ds_constructor_help_invite_text (struct tl_ds_help_invite_text *DS, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x18cb9f78 && T->type->name != 0xe7346087)) { return -1; }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_string (DS->message, field1);
  return 0;
}
int print_ds_constructor_help_support (struct tl_ds_help_support *DS, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x17c6b5f6 && T->type->name != 0xe8394a09)) { return -1; }
  eprintf (" help.support");
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" phone_number :"); }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_string (DS->phone_number, field1);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" user :"); }
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x02e6c0c8, .id = "User", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_user (DS->user, field2);
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int print_ds_constructor_messages_affected_history (struct tl_ds_messages_affected_history *DS, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0xb45c69d1 && T->type->name != 0x4ba3962e)) { return -1; }
  eprintf (" messages.affectedHistory");
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" pts :"); }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_int (DS->pts, field1);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" pts_count :"); }
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_int (DS->pts_count, field2);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" offset :"); }
  struct paramed_type *field3 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_int (DS->offset, field3);
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int print_ds_constructor_messages_affected_messages (struct tl_ds_messages_affected_messages *DS, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x84d19185 && T->type->name != 0x7b2e6e7a)) { return -1; }
  eprintf (" messages.affectedMessages");
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" pts :"); }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_int (DS->pts, field1);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" pts_count :"); }
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_int (DS->pts_count, field2);
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int print_ds_constructor_messages_all_stickers_not_modified (struct tl_ds_messages_all_stickers *DS, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0xb485502f && T->type->name != 0x4b7aafd0)) { return -1; }
  eprintf (" messages.allStickersNotModified");
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int print_ds_constructor_messages_all_stickers (struct tl_ds_messages_all_stickers *DS, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0xb485502f && T->type->name != 0x4b7aafd0)) { return -1; }
  eprintf (" messages.allStickers");
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" hash :"); }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_string (DS->hash, field1);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" packs :"); }
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
  print_ds_type_vector ((void *)DS->packs, field2);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" sets :"); }
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
  print_ds_type_vector ((void *)DS->sets, field3);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" documents :"); }
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
  print_ds_type_vector ((void *)DS->documents, field4);
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int print_ds_constructor_messages_chat_full (struct tl_ds_messages_chat_full *DS, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0xe5d7d19c && T->type->name != 0x1a282e63)) { return -1; }
  eprintf (" messages.chatFull");
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" full_chat :"); }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x2e02a614, .id = "ChatFull", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_chat_full (DS->full_chat, field1);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" chats :"); }
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
  print_ds_type_vector ((void *)DS->chats, field2);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" users :"); }
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
  print_ds_type_vector ((void *)DS->users, field3);
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int print_ds_constructor_messages_chats (struct tl_ds_messages_chats *DS, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x64ff9fd5 && T->type->name != 0x9b00602a)) { return -1; }
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
  print_ds_type_vector ((void *)DS->chats, field1);
  return 0;
}
int print_ds_constructor_messages_dh_config_not_modified (struct tl_ds_messages_dh_config *DS, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0xecc058e8 && T->type->name != 0x133fa717)) { return -1; }
  eprintf (" messages.dhConfigNotModified");
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" random :"); }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xf11ec860, .id = "Bare_Bytes", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_bytes (DS->random, field1);
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int print_ds_constructor_messages_dh_config (struct tl_ds_messages_dh_config *DS, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0xecc058e8 && T->type->name != 0x133fa717)) { return -1; }
  eprintf (" messages.dhConfig");
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" g :"); }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_int (DS->g, field1);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" p :"); }
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xf11ec860, .id = "Bare_Bytes", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_bytes (DS->p, field2);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" version :"); }
  struct paramed_type *field3 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_int (DS->version, field3);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" random :"); }
  struct paramed_type *field4 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xf11ec860, .id = "Bare_Bytes", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_bytes (DS->random, field4);
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int print_ds_constructor_messages_dialogs (struct tl_ds_messages_dialogs *DS, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x645af8b3 && T->type->name != 0x9ba5074c)) { return -1; }
  eprintf (" messages.dialogs");
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" dialogs :"); }
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
  print_ds_type_vector ((void *)DS->dialogs, field1);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" messages :"); }
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
  print_ds_type_vector ((void *)DS->messages, field2);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" chats :"); }
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
  print_ds_type_vector ((void *)DS->chats, field3);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" users :"); }
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
  print_ds_type_vector ((void *)DS->users, field4);
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int print_ds_constructor_messages_dialogs_slice (struct tl_ds_messages_dialogs *DS, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x645af8b3 && T->type->name != 0x9ba5074c)) { return -1; }
  eprintf (" messages.dialogsSlice");
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" count :"); }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_int (DS->count, field1);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" dialogs :"); }
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
  print_ds_type_vector ((void *)DS->dialogs, field2);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" messages :"); }
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
  print_ds_type_vector ((void *)DS->messages, field3);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" chats :"); }
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
  print_ds_type_vector ((void *)DS->chats, field4);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" users :"); }
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
  print_ds_type_vector ((void *)DS->users, field5);
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int print_ds_constructor_messages_message_empty (struct tl_ds_messages_message *DS, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x3f4e0648 && T->type->name != 0xc0b1f9b7)) { return -1; }
  eprintf (" messages.messageEmpty");
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int print_ds_constructor_messages_messages (struct tl_ds_messages_messages *DS, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x8735e464 && T->type->name != 0x78ca1b9b)) { return -1; }
  eprintf (" messages.messages");
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" messages :"); }
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
  print_ds_type_vector ((void *)DS->messages, field1);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" chats :"); }
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
  print_ds_type_vector ((void *)DS->chats, field2);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" users :"); }
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
  print_ds_type_vector ((void *)DS->users, field3);
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int print_ds_constructor_messages_messages_slice (struct tl_ds_messages_messages *DS, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x8735e464 && T->type->name != 0x78ca1b9b)) { return -1; }
  eprintf (" messages.messagesSlice");
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" count :"); }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_int (DS->count, field1);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" messages :"); }
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
  print_ds_type_vector ((void *)DS->messages, field2);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" chats :"); }
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
  print_ds_type_vector ((void *)DS->chats, field3);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" users :"); }
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
  print_ds_type_vector ((void *)DS->users, field4);
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int print_ds_constructor_messages_sent_encrypted_message (struct tl_ds_messages_sent_encrypted_message *DS, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0xc29c7607 && T->type->name != 0x3d6389f8)) { return -1; }
  eprintf (" messages.sentEncryptedMessage");
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" date :"); }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_int (DS->date, field1);
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int print_ds_constructor_messages_sent_encrypted_file (struct tl_ds_messages_sent_encrypted_message *DS, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0xc29c7607 && T->type->name != 0x3d6389f8)) { return -1; }
  eprintf (" messages.sentEncryptedFile");
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" date :"); }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_int (DS->date, field1);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" file :"); }
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x886fd032, .id = "EncryptedFile", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_encrypted_file (DS->file, field2);
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int print_ds_constructor_messages_sent_message (struct tl_ds_messages_sent_message *DS, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x799ce190 && T->type->name != 0x86631e6f)) { return -1; }
  eprintf (" messages.sentMessage");
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" id :"); }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_int (DS->id, field1);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" date :"); }
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_int (DS->date, field2);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" media :"); }
  struct paramed_type *field3 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x95f2471a, .id = "MessageMedia", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_message_media (DS->media, field3);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" pts :"); }
  struct paramed_type *field4 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_int (DS->pts, field4);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" pts_count :"); }
  struct paramed_type *field5 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_int (DS->pts_count, field5);
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int print_ds_constructor_messages_sent_message_link (struct tl_ds_messages_sent_message *DS, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x799ce190 && T->type->name != 0x86631e6f)) { return -1; }
  eprintf (" messages.sentMessageLink");
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" id :"); }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_int (DS->id, field1);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" date :"); }
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_int (DS->date, field2);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" media :"); }
  struct paramed_type *field3 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x95f2471a, .id = "MessageMedia", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_message_media (DS->media, field3);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" pts :"); }
  struct paramed_type *field4 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_int (DS->pts, field4);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" pts_count :"); }
  struct paramed_type *field5 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_int (DS->pts_count, field5);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" links :"); }
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
  print_ds_type_vector ((void *)DS->links, field6);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" seq :"); }
  struct paramed_type *field7 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_int (DS->seq, field7);
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int print_ds_constructor_messages_sticker_set (struct tl_ds_messages_sticker_set *DS, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0xb60a24a6 && T->type->name != 0x49f5db59)) { return -1; }
  eprintf (" messages.stickerSet");
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" set :"); }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xa7a43b17, .id = "StickerSet", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_sticker_set (DS->set, field1);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" packs :"); }
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
  print_ds_type_vector ((void *)DS->packs, field2);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" documents :"); }
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
  print_ds_type_vector ((void *)DS->documents, field3);
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int print_ds_constructor_messages_stickers_not_modified (struct tl_ds_messages_stickers *DS, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x7bfa5710 && T->type->name != 0x8405a8ef)) { return -1; }
  eprintf (" messages.stickersNotModified");
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int print_ds_constructor_messages_stickers (struct tl_ds_messages_stickers *DS, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x7bfa5710 && T->type->name != 0x8405a8ef)) { return -1; }
  eprintf (" messages.stickers");
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" hash :"); }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_string (DS->hash, field1);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" stickers :"); }
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
  print_ds_type_vector ((void *)DS->stickers, field2);
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int print_ds_constructor_photos_photo (struct tl_ds_photos_photo *DS, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x20212ca8 && T->type->name != 0xdfded357)) { return -1; }
  eprintf (" photos.photo");
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" photo :"); }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xc207550a, .id = "Photo", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_photo (DS->photo, field1);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" users :"); }
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
  print_ds_type_vector ((void *)DS->users, field2);
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int print_ds_constructor_photos_photos (struct tl_ds_photos_photos *DS, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x98cf75f1 && T->type->name != 0x67308a0e)) { return -1; }
  eprintf (" photos.photos");
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" photos :"); }
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
  print_ds_type_vector ((void *)DS->photos, field1);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" users :"); }
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
  print_ds_type_vector ((void *)DS->users, field2);
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int print_ds_constructor_photos_photos_slice (struct tl_ds_photos_photos *DS, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x98cf75f1 && T->type->name != 0x67308a0e)) { return -1; }
  eprintf (" photos.photosSlice");
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" count :"); }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_int (DS->count, field1);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" photos :"); }
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
  print_ds_type_vector ((void *)DS->photos, field2);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" users :"); }
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
  print_ds_type_vector ((void *)DS->users, field3);
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int print_ds_constructor_storage_file_unknown (struct tl_ds_storage_file_type *DS, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x3e2838a8 && T->type->name != 0xc1d7c757)) { return -1; }
  eprintf (" storage.fileUnknown");
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int print_ds_constructor_storage_file_jpeg (struct tl_ds_storage_file_type *DS, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x3e2838a8 && T->type->name != 0xc1d7c757)) { return -1; }
  eprintf (" storage.fileJpeg");
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int print_ds_constructor_storage_file_gif (struct tl_ds_storage_file_type *DS, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x3e2838a8 && T->type->name != 0xc1d7c757)) { return -1; }
  eprintf (" storage.fileGif");
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int print_ds_constructor_storage_file_png (struct tl_ds_storage_file_type *DS, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x3e2838a8 && T->type->name != 0xc1d7c757)) { return -1; }
  eprintf (" storage.filePng");
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int print_ds_constructor_storage_file_pdf (struct tl_ds_storage_file_type *DS, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x3e2838a8 && T->type->name != 0xc1d7c757)) { return -1; }
  eprintf (" storage.filePdf");
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int print_ds_constructor_storage_file_mp3 (struct tl_ds_storage_file_type *DS, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x3e2838a8 && T->type->name != 0xc1d7c757)) { return -1; }
  eprintf (" storage.fileMp3");
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int print_ds_constructor_storage_file_mov (struct tl_ds_storage_file_type *DS, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x3e2838a8 && T->type->name != 0xc1d7c757)) { return -1; }
  eprintf (" storage.fileMov");
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int print_ds_constructor_storage_file_partial (struct tl_ds_storage_file_type *DS, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x3e2838a8 && T->type->name != 0xc1d7c757)) { return -1; }
  eprintf (" storage.filePartial");
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int print_ds_constructor_storage_file_mp4 (struct tl_ds_storage_file_type *DS, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x3e2838a8 && T->type->name != 0xc1d7c757)) { return -1; }
  eprintf (" storage.fileMp4");
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int print_ds_constructor_storage_file_webp (struct tl_ds_storage_file_type *DS, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x3e2838a8 && T->type->name != 0xc1d7c757)) { return -1; }
  eprintf (" storage.fileWebp");
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int print_ds_constructor_updates_difference_empty (struct tl_ds_updates_difference *DS, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0xf57a2419 && T->type->name != 0x0a85dbe6)) { return -1; }
  eprintf (" updates.differenceEmpty");
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" date :"); }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_int (DS->date, field1);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" seq :"); }
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_int (DS->seq, field2);
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int print_ds_constructor_updates_difference (struct tl_ds_updates_difference *DS, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0xf57a2419 && T->type->name != 0x0a85dbe6)) { return -1; }
  eprintf (" updates.difference");
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" new_messages :"); }
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
  print_ds_type_vector ((void *)DS->new_messages, field1);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" new_encrypted_messages :"); }
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
  print_ds_type_vector ((void *)DS->new_encrypted_messages, field2);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" other_updates :"); }
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
  print_ds_type_vector ((void *)DS->other_updates, field3);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" chats :"); }
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
  print_ds_type_vector ((void *)DS->chats, field4);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" users :"); }
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
  print_ds_type_vector ((void *)DS->users, field5);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" state :"); }
  struct paramed_type *field6 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xa56c2a3e, .id = "updates.State", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_updates_state (DS->state, field6);
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int print_ds_constructor_updates_difference_slice (struct tl_ds_updates_difference *DS, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0xf57a2419 && T->type->name != 0x0a85dbe6)) { return -1; }
  eprintf (" updates.differenceSlice");
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" new_messages :"); }
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
  print_ds_type_vector ((void *)DS->new_messages, field1);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" new_encrypted_messages :"); }
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
  print_ds_type_vector ((void *)DS->new_encrypted_messages, field2);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" other_updates :"); }
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
  print_ds_type_vector ((void *)DS->other_updates, field3);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" chats :"); }
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
  print_ds_type_vector ((void *)DS->chats, field4);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" users :"); }
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
  print_ds_type_vector ((void *)DS->users, field5);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" intermediate_state :"); }
  struct paramed_type *field6 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xa56c2a3e, .id = "updates.State", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_updates_state (DS->intermediate_state, field6);
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int print_ds_constructor_updates_state (struct tl_ds_updates_state *DS, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0xa56c2a3e && T->type->name != 0x5a93d5c1)) { return -1; }
  eprintf (" updates.state");
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" pts :"); }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_int (DS->pts, field1);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" qts :"); }
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_int (DS->qts, field2);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" date :"); }
  struct paramed_type *field3 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_int (DS->date, field3);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" seq :"); }
  struct paramed_type *field4 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_int (DS->seq, field4);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" unread_count :"); }
  struct paramed_type *field5 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_int (DS->unread_count, field5);
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int print_ds_constructor_upload_file (struct tl_ds_upload_file *DS, struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x096a18d5 && T->type->name != 0xf695e72a)) { return -1; }
  eprintf (" upload.file");
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" type :"); }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x3e2838a8, .id = "storage.FileType", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_storage_file_type (DS->type, field1);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" mtime :"); }
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_int (DS->mtime, field2);
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" bytes :"); }
  struct paramed_type *field3 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xf11ec860, .id = "Bare_Bytes", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  print_ds_type_bare_bytes (DS->bytes, field3);
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int print_ds_type_account_days_t_t_l (struct tl_ds_account_days_t_t_l *DS, struct paramed_type *T) {
  int res;
  res = print_ds_constructor_account_days_t_t_l (DS, T);
  return res;
}
int print_ds_type_bare_account_days_t_t_l (struct tl_ds_account_days_t_t_l *DS, struct paramed_type *T) {
  int res;
  res = print_ds_constructor_account_days_t_t_l (DS, T);
  return res;
}
int print_ds_type_audio (struct tl_ds_audio *DS, struct paramed_type *T) {
  int res;
  if (multiline_output >= 2) { multiline_offset += multiline_offset_size; }
  eprintf (" (");
  switch (DS->magic) {
  case 0x586988d8: res = print_ds_constructor_audio_empty (DS, T); break;
  case 0xc7ac6496: res = print_ds_constructor_audio (DS, T); break;
  default: return -1;
  }
  if (res >= 0) {
    if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); }
    eprintf (" )");
  }
  return res;
}
int print_ds_type_bare_audio (struct tl_ds_audio *DS, struct paramed_type *T) {
  int res;
  if (multiline_output >= 2) { multiline_offset += multiline_offset_size; }
  eprintf (" (");
  switch (DS->magic) {
  case 0x586988d8: res = print_ds_constructor_audio_empty (DS, T); break;
  case 0xc7ac6496: res = print_ds_constructor_audio (DS, T); break;
  default: return -1;
  }
  if (res >= 0) {
    if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); }
    eprintf (" )");
  }
  return res;
}
int print_ds_type_authorization (struct tl_ds_authorization *DS, struct paramed_type *T) {
  int res;
  if (multiline_output >= 2) { multiline_offset += multiline_offset_size; }
  eprintf (" (");
  res = print_ds_constructor_authorization (DS, T);
  if (res >= 0) {
    if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); }
    eprintf (" )");
  }
  return res;
}
int print_ds_type_bare_authorization (struct tl_ds_authorization *DS, struct paramed_type *T) {
  int res;
  if (multiline_output >= 2) { multiline_offset += multiline_offset_size; }
  eprintf (" (");
  res = print_ds_constructor_authorization (DS, T);
  if (res >= 0) {
    if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); }
    eprintf (" )");
  }
  return res;
}
int print_ds_type_bool (struct tl_ds_bool *DS, struct paramed_type *T) {
  int res;
  if (multiline_output >= 2) { multiline_offset += multiline_offset_size; }
  eprintf (" (");
  switch (DS->magic) {
  case 0xbc799737: res = print_ds_constructor_bool_false (DS, T); break;
  case 0x997275b5: res = print_ds_constructor_bool_true (DS, T); break;
  default: return -1;
  }
  if (res >= 0) {
    if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); }
    eprintf (" )");
  }
  return res;
}
int print_ds_type_bare_bool (struct tl_ds_bool *DS, struct paramed_type *T) {
  int res;
  if (multiline_output >= 2) { multiline_offset += multiline_offset_size; }
  eprintf (" (");
  switch (DS->magic) {
  case 0xbc799737: res = print_ds_constructor_bool_false (DS, T); break;
  case 0x997275b5: res = print_ds_constructor_bool_true (DS, T); break;
  default: return -1;
  }
  if (res >= 0) {
    if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); }
    eprintf (" )");
  }
  return res;
}
int print_ds_type_bot_command (struct tl_ds_bot_command *DS, struct paramed_type *T) {
  int res;
  if (multiline_output >= 2) { multiline_offset += multiline_offset_size; }
  eprintf (" (");
  switch (DS->magic) {
  case 0xc27ac8c7: res = print_ds_constructor_bot_command (DS, T); break;
  case 0xb79d22ab: res = print_ds_constructor_bot_command_old (DS, T); break;
  default: return -1;
  }
  if (res >= 0) {
    if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); }
    eprintf (" )");
  }
  return res;
}
int print_ds_type_bare_bot_command (struct tl_ds_bot_command *DS, struct paramed_type *T) {
  int res;
  if (multiline_output >= 2) { multiline_offset += multiline_offset_size; }
  eprintf (" (");
  switch (DS->magic) {
  case 0xc27ac8c7: res = print_ds_constructor_bot_command (DS, T); break;
  case 0xb79d22ab: res = print_ds_constructor_bot_command_old (DS, T); break;
  default: return -1;
  }
  if (res >= 0) {
    if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); }
    eprintf (" )");
  }
  return res;
}
int print_ds_type_bot_info (struct tl_ds_bot_info *DS, struct paramed_type *T) {
  int res;
  if (multiline_output >= 2) { multiline_offset += multiline_offset_size; }
  eprintf (" (");
  switch (DS->magic) {
  case 0xbb2e37ce: res = print_ds_constructor_bot_info_empty (DS, T); break;
  case 0x09cf585d: res = print_ds_constructor_bot_info (DS, T); break;
  default: return -1;
  }
  if (res >= 0) {
    if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); }
    eprintf (" )");
  }
  return res;
}
int print_ds_type_bare_bot_info (struct tl_ds_bot_info *DS, struct paramed_type *T) {
  int res;
  if (multiline_output >= 2) { multiline_offset += multiline_offset_size; }
  eprintf (" (");
  switch (DS->magic) {
  case 0xbb2e37ce: res = print_ds_constructor_bot_info_empty (DS, T); break;
  case 0x09cf585d: res = print_ds_constructor_bot_info (DS, T); break;
  default: return -1;
  }
  if (res >= 0) {
    if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); }
    eprintf (" )");
  }
  return res;
}
int print_ds_type_bytes (struct tl_ds_string *DS, struct paramed_type *T) {
  int res;
  res = print_ds_constructor_bytes (DS, T);
  return res;
}
int print_ds_type_bare_bytes (struct tl_ds_string *DS, struct paramed_type *T) {
  int res;
  res = print_ds_constructor_bytes (DS, T);
  return res;
}
int print_ds_type_chat (struct tl_ds_chat *DS, struct paramed_type *T) {
  int res;
  if (multiline_output >= 2) { multiline_offset += multiline_offset_size; }
  eprintf (" (");
  switch (DS->magic) {
  case 0x9ba2d800: res = print_ds_constructor_chat_empty (DS, T); break;
  case 0x6e9c9bc7: res = print_ds_constructor_chat (DS, T); break;
  case 0xfb0ccc41: res = print_ds_constructor_chat_forbidden (DS, T); break;
  case 0x75eaea5a: res = print_ds_constructor_geo_chat (DS, T); break;
  default: return -1;
  }
  if (res >= 0) {
    if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); }
    eprintf (" )");
  }
  return res;
}
int print_ds_type_bare_chat (struct tl_ds_chat *DS, struct paramed_type *T) {
  int res;
  if (multiline_output >= 2) { multiline_offset += multiline_offset_size; }
  eprintf (" (");
  switch (DS->magic) {
  case 0x9ba2d800: res = print_ds_constructor_chat_empty (DS, T); break;
  case 0x6e9c9bc7: res = print_ds_constructor_chat (DS, T); break;
  case 0xfb0ccc41: res = print_ds_constructor_chat_forbidden (DS, T); break;
  case 0x75eaea5a: res = print_ds_constructor_geo_chat (DS, T); break;
  default: return -1;
  }
  if (res >= 0) {
    if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); }
    eprintf (" )");
  }
  return res;
}
int print_ds_type_chat_full (struct tl_ds_chat_full *DS, struct paramed_type *T) {
  int res;
  if (multiline_output >= 2) { multiline_offset += multiline_offset_size; }
  eprintf (" (");
  res = print_ds_constructor_chat_full (DS, T);
  if (res >= 0) {
    if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); }
    eprintf (" )");
  }
  return res;
}
int print_ds_type_bare_chat_full (struct tl_ds_chat_full *DS, struct paramed_type *T) {
  int res;
  if (multiline_output >= 2) { multiline_offset += multiline_offset_size; }
  eprintf (" (");
  res = print_ds_constructor_chat_full (DS, T);
  if (res >= 0) {
    if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); }
    eprintf (" )");
  }
  return res;
}
int print_ds_type_chat_invite (struct tl_ds_chat_invite *DS, struct paramed_type *T) {
  int res;
  if (multiline_output >= 2) { multiline_offset += multiline_offset_size; }
  eprintf (" (");
  switch (DS->magic) {
  case 0x5a686d7c: res = print_ds_constructor_chat_invite_already (DS, T); break;
  case 0xce917dcd: res = print_ds_constructor_chat_invite (DS, T); break;
  default: return -1;
  }
  if (res >= 0) {
    if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); }
    eprintf (" )");
  }
  return res;
}
int print_ds_type_bare_chat_invite (struct tl_ds_chat_invite *DS, struct paramed_type *T) {
  int res;
  if (multiline_output >= 2) { multiline_offset += multiline_offset_size; }
  eprintf (" (");
  switch (DS->magic) {
  case 0x5a686d7c: res = print_ds_constructor_chat_invite_already (DS, T); break;
  case 0xce917dcd: res = print_ds_constructor_chat_invite (DS, T); break;
  default: return -1;
  }
  if (res >= 0) {
    if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); }
    eprintf (" )");
  }
  return res;
}
int print_ds_type_chat_located (struct tl_ds_chat_located *DS, struct paramed_type *T) {
  int res;
  if (multiline_output >= 2) { multiline_offset += multiline_offset_size; }
  eprintf (" (");
  res = print_ds_constructor_chat_located (DS, T);
  if (res >= 0) {
    if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); }
    eprintf (" )");
  }
  return res;
}
int print_ds_type_bare_chat_located (struct tl_ds_chat_located *DS, struct paramed_type *T) {
  int res;
  if (multiline_output >= 2) { multiline_offset += multiline_offset_size; }
  eprintf (" (");
  res = print_ds_constructor_chat_located (DS, T);
  if (res >= 0) {
    if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); }
    eprintf (" )");
  }
  return res;
}
int print_ds_type_chat_participant (struct tl_ds_chat_participant *DS, struct paramed_type *T) {
  int res;
  if (multiline_output >= 2) { multiline_offset += multiline_offset_size; }
  eprintf (" (");
  res = print_ds_constructor_chat_participant (DS, T);
  if (res >= 0) {
    if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); }
    eprintf (" )");
  }
  return res;
}
int print_ds_type_bare_chat_participant (struct tl_ds_chat_participant *DS, struct paramed_type *T) {
  int res;
  if (multiline_output >= 2) { multiline_offset += multiline_offset_size; }
  eprintf (" (");
  res = print_ds_constructor_chat_participant (DS, T);
  if (res >= 0) {
    if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); }
    eprintf (" )");
  }
  return res;
}
int print_ds_type_chat_participants (struct tl_ds_chat_participants *DS, struct paramed_type *T) {
  int res;
  if (multiline_output >= 2) { multiline_offset += multiline_offset_size; }
  eprintf (" (");
  switch (DS->magic) {
  case 0x0fd2bb8a: res = print_ds_constructor_chat_participants_forbidden (DS, T); break;
  case 0x7841b415: res = print_ds_constructor_chat_participants (DS, T); break;
  default: return -1;
  }
  if (res >= 0) {
    if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); }
    eprintf (" )");
  }
  return res;
}
int print_ds_type_bare_chat_participants (struct tl_ds_chat_participants *DS, struct paramed_type *T) {
  int res;
  if (multiline_output >= 2) { multiline_offset += multiline_offset_size; }
  eprintf (" (");
  switch (DS->magic) {
  case 0x0fd2bb8a: res = print_ds_constructor_chat_participants_forbidden (DS, T); break;
  case 0x7841b415: res = print_ds_constructor_chat_participants (DS, T); break;
  default: return -1;
  }
  if (res >= 0) {
    if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); }
    eprintf (" )");
  }
  return res;
}
int print_ds_type_chat_photo (struct tl_ds_chat_photo *DS, struct paramed_type *T) {
  int res;
  if (multiline_output >= 2) { multiline_offset += multiline_offset_size; }
  eprintf (" (");
  switch (DS->magic) {
  case 0x37c1011c: res = print_ds_constructor_chat_photo_empty (DS, T); break;
  case 0x6153276a: res = print_ds_constructor_chat_photo (DS, T); break;
  default: return -1;
  }
  if (res >= 0) {
    if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); }
    eprintf (" )");
  }
  return res;
}
int print_ds_type_bare_chat_photo (struct tl_ds_chat_photo *DS, struct paramed_type *T) {
  int res;
  if (multiline_output >= 2) { multiline_offset += multiline_offset_size; }
  eprintf (" (");
  switch (DS->magic) {
  case 0x37c1011c: res = print_ds_constructor_chat_photo_empty (DS, T); break;
  case 0x6153276a: res = print_ds_constructor_chat_photo (DS, T); break;
  default: return -1;
  }
  if (res >= 0) {
    if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); }
    eprintf (" )");
  }
  return res;
}
int print_ds_type_client_d_h_inner_data (struct tl_ds_client_d_h_inner_data *DS, struct paramed_type *T) {
  int res;
  if (multiline_output >= 2) { multiline_offset += multiline_offset_size; }
  eprintf (" (");
  res = print_ds_constructor_client_d_h_inner_data (DS, T);
  if (res >= 0) {
    if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); }
    eprintf (" )");
  }
  return res;
}
int print_ds_type_bare_client_d_h_inner_data (struct tl_ds_client_d_h_inner_data *DS, struct paramed_type *T) {
  int res;
  if (multiline_output >= 2) { multiline_offset += multiline_offset_size; }
  eprintf (" (");
  res = print_ds_constructor_client_d_h_inner_data (DS, T);
  if (res >= 0) {
    if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); }
    eprintf (" )");
  }
  return res;
}
int print_ds_type_config (struct tl_ds_config *DS, struct paramed_type *T) {
  int res;
  if (multiline_output >= 2) { multiline_offset += multiline_offset_size; }
  eprintf (" (");
  res = print_ds_constructor_config (DS, T);
  if (res >= 0) {
    if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); }
    eprintf (" )");
  }
  return res;
}
int print_ds_type_bare_config (struct tl_ds_config *DS, struct paramed_type *T) {
  int res;
  if (multiline_output >= 2) { multiline_offset += multiline_offset_size; }
  eprintf (" (");
  res = print_ds_constructor_config (DS, T);
  if (res >= 0) {
    if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); }
    eprintf (" )");
  }
  return res;
}
int print_ds_type_contact (struct tl_ds_contact *DS, struct paramed_type *T) {
  int res;
  if (multiline_output >= 2) { multiline_offset += multiline_offset_size; }
  eprintf (" (");
  res = print_ds_constructor_contact (DS, T);
  if (res >= 0) {
    if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); }
    eprintf (" )");
  }
  return res;
}
int print_ds_type_bare_contact (struct tl_ds_contact *DS, struct paramed_type *T) {
  int res;
  if (multiline_output >= 2) { multiline_offset += multiline_offset_size; }
  eprintf (" (");
  res = print_ds_constructor_contact (DS, T);
  if (res >= 0) {
    if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); }
    eprintf (" )");
  }
  return res;
}
int print_ds_type_contact_blocked (struct tl_ds_contact_blocked *DS, struct paramed_type *T) {
  int res;
  if (multiline_output >= 2) { multiline_offset += multiline_offset_size; }
  eprintf (" (");
  res = print_ds_constructor_contact_blocked (DS, T);
  if (res >= 0) {
    if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); }
    eprintf (" )");
  }
  return res;
}
int print_ds_type_bare_contact_blocked (struct tl_ds_contact_blocked *DS, struct paramed_type *T) {
  int res;
  if (multiline_output >= 2) { multiline_offset += multiline_offset_size; }
  eprintf (" (");
  res = print_ds_constructor_contact_blocked (DS, T);
  if (res >= 0) {
    if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); }
    eprintf (" )");
  }
  return res;
}
int print_ds_type_contact_found (struct tl_ds_contact_found *DS, struct paramed_type *T) {
  int res;
  res = print_ds_constructor_contact_found (DS, T);
  return res;
}
int print_ds_type_bare_contact_found (struct tl_ds_contact_found *DS, struct paramed_type *T) {
  int res;
  res = print_ds_constructor_contact_found (DS, T);
  return res;
}
int print_ds_type_contact_link (struct tl_ds_contact_link *DS, struct paramed_type *T) {
  int res;
  if (multiline_output >= 2) { multiline_offset += multiline_offset_size; }
  eprintf (" (");
  switch (DS->magic) {
  case 0x5f4f9247: res = print_ds_constructor_contact_link_unknown (DS, T); break;
  case 0xfeedd3ad: res = print_ds_constructor_contact_link_none (DS, T); break;
  case 0x268f3f59: res = print_ds_constructor_contact_link_has_phone (DS, T); break;
  case 0xd502c2d0: res = print_ds_constructor_contact_link_contact (DS, T); break;
  default: return -1;
  }
  if (res >= 0) {
    if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); }
    eprintf (" )");
  }
  return res;
}
int print_ds_type_bare_contact_link (struct tl_ds_contact_link *DS, struct paramed_type *T) {
  int res;
  if (multiline_output >= 2) { multiline_offset += multiline_offset_size; }
  eprintf (" (");
  switch (DS->magic) {
  case 0x5f4f9247: res = print_ds_constructor_contact_link_unknown (DS, T); break;
  case 0xfeedd3ad: res = print_ds_constructor_contact_link_none (DS, T); break;
  case 0x268f3f59: res = print_ds_constructor_contact_link_has_phone (DS, T); break;
  case 0xd502c2d0: res = print_ds_constructor_contact_link_contact (DS, T); break;
  default: return -1;
  }
  if (res >= 0) {
    if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); }
    eprintf (" )");
  }
  return res;
}
int print_ds_type_contact_status (struct tl_ds_contact_status *DS, struct paramed_type *T) {
  int res;
  if (multiline_output >= 2) { multiline_offset += multiline_offset_size; }
  eprintf (" (");
  res = print_ds_constructor_contact_status (DS, T);
  if (res >= 0) {
    if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); }
    eprintf (" )");
  }
  return res;
}
int print_ds_type_bare_contact_status (struct tl_ds_contact_status *DS, struct paramed_type *T) {
  int res;
  if (multiline_output >= 2) { multiline_offset += multiline_offset_size; }
  eprintf (" (");
  res = print_ds_constructor_contact_status (DS, T);
  if (res >= 0) {
    if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); }
    eprintf (" )");
  }
  return res;
}
int print_ds_type_contact_suggested (struct tl_ds_contact_suggested *DS, struct paramed_type *T) {
  int res;
  if (multiline_output >= 2) { multiline_offset += multiline_offset_size; }
  eprintf (" (");
  res = print_ds_constructor_contact_suggested (DS, T);
  if (res >= 0) {
    if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); }
    eprintf (" )");
  }
  return res;
}
int print_ds_type_bare_contact_suggested (struct tl_ds_contact_suggested *DS, struct paramed_type *T) {
  int res;
  if (multiline_output >= 2) { multiline_offset += multiline_offset_size; }
  eprintf (" (");
  res = print_ds_constructor_contact_suggested (DS, T);
  if (res >= 0) {
    if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); }
    eprintf (" )");
  }
  return res;
}
int print_ds_type_dc_option (struct tl_ds_dc_option *DS, struct paramed_type *T) {
  int res;
  if (multiline_output >= 2) { multiline_offset += multiline_offset_size; }
  eprintf (" (");
  switch (DS->magic) {
  case 0x2ec2a43c: res = print_ds_constructor_dc_option_l28 (DS, T); break;
  case 0x05d8c6cc: res = print_ds_constructor_dc_option (DS, T); break;
  default: return -1;
  }
  if (res >= 0) {
    if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); }
    eprintf (" )");
  }
  return res;
}
int print_ds_type_bare_dc_option (struct tl_ds_dc_option *DS, struct paramed_type *T) {
  int res;
  if (multiline_output >= 2) { multiline_offset += multiline_offset_size; }
  eprintf (" (");
  switch (DS->magic) {
  case 0x2ec2a43c: res = print_ds_constructor_dc_option_l28 (DS, T); break;
  case 0x05d8c6cc: res = print_ds_constructor_dc_option (DS, T); break;
  default: return -1;
  }
  if (res >= 0) {
    if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); }
    eprintf (" )");
  }
  return res;
}
int print_ds_type_decrypted_message (struct tl_ds_decrypted_message *DS, struct paramed_type *T) {
  int res;
  if (multiline_output >= 2) { multiline_offset += multiline_offset_size; }
  eprintf (" (");
  switch (DS->magic) {
  case 0x204d3878: res = print_ds_constructor_decrypted_message (DS, T); break;
  case 0x73164160: res = print_ds_constructor_decrypted_message_service (DS, T); break;
  default: return -1;
  }
  if (res >= 0) {
    if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); }
    eprintf (" )");
  }
  return res;
}
int print_ds_type_bare_decrypted_message (struct tl_ds_decrypted_message *DS, struct paramed_type *T) {
  int res;
  if (multiline_output >= 2) { multiline_offset += multiline_offset_size; }
  eprintf (" (");
  switch (DS->magic) {
  case 0x204d3878: res = print_ds_constructor_decrypted_message (DS, T); break;
  case 0x73164160: res = print_ds_constructor_decrypted_message_service (DS, T); break;
  default: return -1;
  }
  if (res >= 0) {
    if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); }
    eprintf (" )");
  }
  return res;
}
int print_ds_type_decrypted_message_action (struct tl_ds_decrypted_message_action *DS, struct paramed_type *T) {
  int res;
  if (multiline_output >= 2) { multiline_offset += multiline_offset_size; }
  eprintf (" (");
  switch (DS->magic) {
  case 0xa1733aec: res = print_ds_constructor_decrypted_message_action_set_message_t_t_l (DS, T); break;
  case 0x0c4f40be: res = print_ds_constructor_decrypted_message_action_read_messages (DS, T); break;
  case 0x65614304: res = print_ds_constructor_decrypted_message_action_delete_messages (DS, T); break;
  case 0x8ac1f475: res = print_ds_constructor_decrypted_message_action_screenshot_messages (DS, T); break;
  case 0x6719e45c: res = print_ds_constructor_decrypted_message_action_flush_history (DS, T); break;
  case 0x511110b0: res = print_ds_constructor_decrypted_message_action_resend (DS, T); break;
  case 0xf3048883: res = print_ds_constructor_decrypted_message_action_notify_layer (DS, T); break;
  case 0xccb27641: res = print_ds_constructor_decrypted_message_action_typing (DS, T); break;
  case 0xf3c9611b: res = print_ds_constructor_decrypted_message_action_request_key (DS, T); break;
  case 0x6fe1735b: res = print_ds_constructor_decrypted_message_action_accept_key (DS, T); break;
  case 0xdd05ec6b: res = print_ds_constructor_decrypted_message_action_abort_key (DS, T); break;
  case 0xec2e0b9b: res = print_ds_constructor_decrypted_message_action_commit_key (DS, T); break;
  case 0xa82fdd63: res = print_ds_constructor_decrypted_message_action_noop (DS, T); break;
  default: return -1;
  }
  if (res >= 0) {
    if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); }
    eprintf (" )");
  }
  return res;
}
int print_ds_type_bare_decrypted_message_action (struct tl_ds_decrypted_message_action *DS, struct paramed_type *T) {
  int res;
  if (multiline_output >= 2) { multiline_offset += multiline_offset_size; }
  eprintf (" (");
  switch (DS->magic) {
  case 0xa1733aec: res = print_ds_constructor_decrypted_message_action_set_message_t_t_l (DS, T); break;
  case 0x0c4f40be: res = print_ds_constructor_decrypted_message_action_read_messages (DS, T); break;
  case 0x65614304: res = print_ds_constructor_decrypted_message_action_delete_messages (DS, T); break;
  case 0x8ac1f475: res = print_ds_constructor_decrypted_message_action_screenshot_messages (DS, T); break;
  case 0x6719e45c: res = print_ds_constructor_decrypted_message_action_flush_history (DS, T); break;
  case 0x511110b0: res = print_ds_constructor_decrypted_message_action_resend (DS, T); break;
  case 0xf3048883: res = print_ds_constructor_decrypted_message_action_notify_layer (DS, T); break;
  case 0xccb27641: res = print_ds_constructor_decrypted_message_action_typing (DS, T); break;
  case 0xf3c9611b: res = print_ds_constructor_decrypted_message_action_request_key (DS, T); break;
  case 0x6fe1735b: res = print_ds_constructor_decrypted_message_action_accept_key (DS, T); break;
  case 0xdd05ec6b: res = print_ds_constructor_decrypted_message_action_abort_key (DS, T); break;
  case 0xec2e0b9b: res = print_ds_constructor_decrypted_message_action_commit_key (DS, T); break;
  case 0xa82fdd63: res = print_ds_constructor_decrypted_message_action_noop (DS, T); break;
  default: return -1;
  }
  if (res >= 0) {
    if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); }
    eprintf (" )");
  }
  return res;
}
int print_ds_type_decrypted_message_layer (struct tl_ds_decrypted_message_layer *DS, struct paramed_type *T) {
  int res;
  if (multiline_output >= 2) { multiline_offset += multiline_offset_size; }
  eprintf (" (");
  res = print_ds_constructor_decrypted_message_layer (DS, T);
  if (res >= 0) {
    if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); }
    eprintf (" )");
  }
  return res;
}
int print_ds_type_bare_decrypted_message_layer (struct tl_ds_decrypted_message_layer *DS, struct paramed_type *T) {
  int res;
  if (multiline_output >= 2) { multiline_offset += multiline_offset_size; }
  eprintf (" (");
  res = print_ds_constructor_decrypted_message_layer (DS, T);
  if (res >= 0) {
    if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); }
    eprintf (" )");
  }
  return res;
}
int print_ds_type_decrypted_message_media (struct tl_ds_decrypted_message_media *DS, struct paramed_type *T) {
  int res;
  if (multiline_output >= 2) { multiline_offset += multiline_offset_size; }
  eprintf (" (");
  switch (DS->magic) {
  case 0x089f5c4a: res = print_ds_constructor_decrypted_message_media_empty (DS, T); break;
  case 0x32798a8c: res = print_ds_constructor_decrypted_message_media_photo (DS, T); break;
  case 0x35480a59: res = print_ds_constructor_decrypted_message_media_geo_point (DS, T); break;
  case 0x588a0a97: res = print_ds_constructor_decrypted_message_media_contact (DS, T); break;
  case 0xb095434b: res = print_ds_constructor_decrypted_message_media_document (DS, T); break;
  case 0x524a415d: res = print_ds_constructor_decrypted_message_media_video (DS, T); break;
  case 0x57e0a9cb: res = print_ds_constructor_decrypted_message_media_audio (DS, T); break;
  case 0xfa95b0dd: res = print_ds_constructor_decrypted_message_media_external_document (DS, T); break;
  case 0x4cee6ef3: res = print_ds_constructor_decrypted_message_media_video_l12 (DS, T); break;
  case 0x6080758f: res = print_ds_constructor_decrypted_message_media_audio_l12 (DS, T); break;
  default: return -1;
  }
  if (res >= 0) {
    if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); }
    eprintf (" )");
  }
  return res;
}
int print_ds_type_bare_decrypted_message_media (struct tl_ds_decrypted_message_media *DS, struct paramed_type *T) {
  int res;
  if (multiline_output >= 2) { multiline_offset += multiline_offset_size; }
  eprintf (" (");
  switch (DS->magic) {
  case 0x089f5c4a: res = print_ds_constructor_decrypted_message_media_empty (DS, T); break;
  case 0x32798a8c: res = print_ds_constructor_decrypted_message_media_photo (DS, T); break;
  case 0x35480a59: res = print_ds_constructor_decrypted_message_media_geo_point (DS, T); break;
  case 0x588a0a97: res = print_ds_constructor_decrypted_message_media_contact (DS, T); break;
  case 0xb095434b: res = print_ds_constructor_decrypted_message_media_document (DS, T); break;
  case 0x524a415d: res = print_ds_constructor_decrypted_message_media_video (DS, T); break;
  case 0x57e0a9cb: res = print_ds_constructor_decrypted_message_media_audio (DS, T); break;
  case 0xfa95b0dd: res = print_ds_constructor_decrypted_message_media_external_document (DS, T); break;
  case 0x4cee6ef3: res = print_ds_constructor_decrypted_message_media_video_l12 (DS, T); break;
  case 0x6080758f: res = print_ds_constructor_decrypted_message_media_audio_l12 (DS, T); break;
  default: return -1;
  }
  if (res >= 0) {
    if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); }
    eprintf (" )");
  }
  return res;
}
int print_ds_type_dialog (struct tl_ds_dialog *DS, struct paramed_type *T) {
  int res;
  if (multiline_output >= 2) { multiline_offset += multiline_offset_size; }
  eprintf (" (");
  res = print_ds_constructor_dialog (DS, T);
  if (res >= 0) {
    if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); }
    eprintf (" )");
  }
  return res;
}
int print_ds_type_bare_dialog (struct tl_ds_dialog *DS, struct paramed_type *T) {
  int res;
  if (multiline_output >= 2) { multiline_offset += multiline_offset_size; }
  eprintf (" (");
  res = print_ds_constructor_dialog (DS, T);
  if (res >= 0) {
    if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); }
    eprintf (" )");
  }
  return res;
}
int print_ds_type_disabled_feature (struct tl_ds_disabled_feature *DS, struct paramed_type *T) {
  int res;
  if (multiline_output >= 2) { multiline_offset += multiline_offset_size; }
  eprintf (" (");
  res = print_ds_constructor_disabled_feature (DS, T);
  if (res >= 0) {
    if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); }
    eprintf (" )");
  }
  return res;
}
int print_ds_type_bare_disabled_feature (struct tl_ds_disabled_feature *DS, struct paramed_type *T) {
  int res;
  if (multiline_output >= 2) { multiline_offset += multiline_offset_size; }
  eprintf (" (");
  res = print_ds_constructor_disabled_feature (DS, T);
  if (res >= 0) {
    if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); }
    eprintf (" )");
  }
  return res;
}
int print_ds_type_document (struct tl_ds_document *DS, struct paramed_type *T) {
  int res;
  if (multiline_output >= 2) { multiline_offset += multiline_offset_size; }
  eprintf (" (");
  switch (DS->magic) {
  case 0x36f8c871: res = print_ds_constructor_document_empty (DS, T); break;
  case 0xf9a39f4f: res = print_ds_constructor_document (DS, T); break;
  case 0x9efc6326: res = print_ds_constructor_document_l19 (DS, T); break;
  default: return -1;
  }
  if (res >= 0) {
    if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); }
    eprintf (" )");
  }
  return res;
}
int print_ds_type_bare_document (struct tl_ds_document *DS, struct paramed_type *T) {
  int res;
  if (multiline_output >= 2) { multiline_offset += multiline_offset_size; }
  eprintf (" (");
  switch (DS->magic) {
  case 0x36f8c871: res = print_ds_constructor_document_empty (DS, T); break;
  case 0xf9a39f4f: res = print_ds_constructor_document (DS, T); break;
  case 0x9efc6326: res = print_ds_constructor_document_l19 (DS, T); break;
  default: return -1;
  }
  if (res >= 0) {
    if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); }
    eprintf (" )");
  }
  return res;
}
int print_ds_type_document_attribute (struct tl_ds_document_attribute *DS, struct paramed_type *T) {
  int res;
  if (multiline_output >= 2) { multiline_offset += multiline_offset_size; }
  eprintf (" (");
  switch (DS->magic) {
  case 0x6c37c15c: res = print_ds_constructor_document_attribute_image_size (DS, T); break;
  case 0x11b58939: res = print_ds_constructor_document_attribute_animated (DS, T); break;
  case 0x994c9882: res = print_ds_constructor_document_attribute_sticker_l28 (DS, T); break;
  case 0x3a556302: res = print_ds_constructor_document_attribute_sticker (DS, T); break;
  case 0x5910cccb: res = print_ds_constructor_document_attribute_video (DS, T); break;
  case 0x051448e5: res = print_ds_constructor_document_attribute_audio (DS, T); break;
  case 0x15590068: res = print_ds_constructor_document_attribute_filename (DS, T); break;
  default: return -1;
  }
  if (res >= 0) {
    if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); }
    eprintf (" )");
  }
  return res;
}
int print_ds_type_bare_document_attribute (struct tl_ds_document_attribute *DS, struct paramed_type *T) {
  int res;
  if (multiline_output >= 2) { multiline_offset += multiline_offset_size; }
  eprintf (" (");
  switch (DS->magic) {
  case 0x6c37c15c: res = print_ds_constructor_document_attribute_image_size (DS, T); break;
  case 0x11b58939: res = print_ds_constructor_document_attribute_animated (DS, T); break;
  case 0x994c9882: res = print_ds_constructor_document_attribute_sticker_l28 (DS, T); break;
  case 0x3a556302: res = print_ds_constructor_document_attribute_sticker (DS, T); break;
  case 0x5910cccb: res = print_ds_constructor_document_attribute_video (DS, T); break;
  case 0x051448e5: res = print_ds_constructor_document_attribute_audio (DS, T); break;
  case 0x15590068: res = print_ds_constructor_document_attribute_filename (DS, T); break;
  default: return -1;
  }
  if (res >= 0) {
    if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); }
    eprintf (" )");
  }
  return res;
}
int print_ds_type_double (double *DS, struct paramed_type *T) {
  int res;
  res = print_ds_constructor_double (DS, T);
  return res;
}
int print_ds_type_bare_double (double *DS, struct paramed_type *T) {
  int res;
  res = print_ds_constructor_double (DS, T);
  return res;
}
int print_ds_type_encrypted_chat (struct tl_ds_encrypted_chat *DS, struct paramed_type *T) {
  int res;
  if (multiline_output >= 2) { multiline_offset += multiline_offset_size; }
  eprintf (" (");
  switch (DS->magic) {
  case 0xab7ec0a0: res = print_ds_constructor_encrypted_chat_empty (DS, T); break;
  case 0x3bf703dc: res = print_ds_constructor_encrypted_chat_waiting (DS, T); break;
  case 0xc878527e: res = print_ds_constructor_encrypted_chat_requested (DS, T); break;
  case 0xfa56ce36: res = print_ds_constructor_encrypted_chat (DS, T); break;
  case 0x13d6dd27: res = print_ds_constructor_encrypted_chat_discarded (DS, T); break;
  default: return -1;
  }
  if (res >= 0) {
    if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); }
    eprintf (" )");
  }
  return res;
}
int print_ds_type_bare_encrypted_chat (struct tl_ds_encrypted_chat *DS, struct paramed_type *T) {
  int res;
  if (multiline_output >= 2) { multiline_offset += multiline_offset_size; }
  eprintf (" (");
  switch (DS->magic) {
  case 0xab7ec0a0: res = print_ds_constructor_encrypted_chat_empty (DS, T); break;
  case 0x3bf703dc: res = print_ds_constructor_encrypted_chat_waiting (DS, T); break;
  case 0xc878527e: res = print_ds_constructor_encrypted_chat_requested (DS, T); break;
  case 0xfa56ce36: res = print_ds_constructor_encrypted_chat (DS, T); break;
  case 0x13d6dd27: res = print_ds_constructor_encrypted_chat_discarded (DS, T); break;
  default: return -1;
  }
  if (res >= 0) {
    if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); }
    eprintf (" )");
  }
  return res;
}
int print_ds_type_encrypted_file (struct tl_ds_encrypted_file *DS, struct paramed_type *T) {
  int res;
  if (multiline_output >= 2) { multiline_offset += multiline_offset_size; }
  eprintf (" (");
  switch (DS->magic) {
  case 0xc21f497e: res = print_ds_constructor_encrypted_file_empty (DS, T); break;
  case 0x4a70994c: res = print_ds_constructor_encrypted_file (DS, T); break;
  default: return -1;
  }
  if (res >= 0) {
    if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); }
    eprintf (" )");
  }
  return res;
}
int print_ds_type_bare_encrypted_file (struct tl_ds_encrypted_file *DS, struct paramed_type *T) {
  int res;
  if (multiline_output >= 2) { multiline_offset += multiline_offset_size; }
  eprintf (" (");
  switch (DS->magic) {
  case 0xc21f497e: res = print_ds_constructor_encrypted_file_empty (DS, T); break;
  case 0x4a70994c: res = print_ds_constructor_encrypted_file (DS, T); break;
  default: return -1;
  }
  if (res >= 0) {
    if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); }
    eprintf (" )");
  }
  return res;
}
int print_ds_type_encrypted_message (struct tl_ds_encrypted_message *DS, struct paramed_type *T) {
  int res;
  if (multiline_output >= 2) { multiline_offset += multiline_offset_size; }
  eprintf (" (");
  switch (DS->magic) {
  case 0xed18c118: res = print_ds_constructor_encrypted_message (DS, T); break;
  case 0x23734b06: res = print_ds_constructor_encrypted_message_service (DS, T); break;
  default: return -1;
  }
  if (res >= 0) {
    if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); }
    eprintf (" )");
  }
  return res;
}
int print_ds_type_bare_encrypted_message (struct tl_ds_encrypted_message *DS, struct paramed_type *T) {
  int res;
  if (multiline_output >= 2) { multiline_offset += multiline_offset_size; }
  eprintf (" (");
  switch (DS->magic) {
  case 0xed18c118: res = print_ds_constructor_encrypted_message (DS, T); break;
  case 0x23734b06: res = print_ds_constructor_encrypted_message_service (DS, T); break;
  default: return -1;
  }
  if (res >= 0) {
    if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); }
    eprintf (" )");
  }
  return res;
}
int print_ds_type_error (struct tl_ds_error *DS, struct paramed_type *T) {
  int res;
  if (multiline_output >= 2) { multiline_offset += multiline_offset_size; }
  eprintf (" (");
  res = print_ds_constructor_error (DS, T);
  if (res >= 0) {
    if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); }
    eprintf (" )");
  }
  return res;
}
int print_ds_type_bare_error (struct tl_ds_error *DS, struct paramed_type *T) {
  int res;
  if (multiline_output >= 2) { multiline_offset += multiline_offset_size; }
  eprintf (" (");
  res = print_ds_constructor_error (DS, T);
  if (res >= 0) {
    if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); }
    eprintf (" )");
  }
  return res;
}
int print_ds_type_exported_chat_invite (struct tl_ds_exported_chat_invite *DS, struct paramed_type *T) {
  int res;
  if (multiline_output >= 2) { multiline_offset += multiline_offset_size; }
  eprintf (" (");
  switch (DS->magic) {
  case 0x69df3769: res = print_ds_constructor_chat_invite_empty (DS, T); break;
  case 0xfc2e05bc: res = print_ds_constructor_chat_invite_exported (DS, T); break;
  default: return -1;
  }
  if (res >= 0) {
    if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); }
    eprintf (" )");
  }
  return res;
}
int print_ds_type_bare_exported_chat_invite (struct tl_ds_exported_chat_invite *DS, struct paramed_type *T) {
  int res;
  if (multiline_output >= 2) { multiline_offset += multiline_offset_size; }
  eprintf (" (");
  switch (DS->magic) {
  case 0x69df3769: res = print_ds_constructor_chat_invite_empty (DS, T); break;
  case 0xfc2e05bc: res = print_ds_constructor_chat_invite_exported (DS, T); break;
  default: return -1;
  }
  if (res >= 0) {
    if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); }
    eprintf (" )");
  }
  return res;
}
int print_ds_type_file_location (struct tl_ds_file_location *DS, struct paramed_type *T) {
  int res;
  if (multiline_output >= 2) { multiline_offset += multiline_offset_size; }
  eprintf (" (");
  switch (DS->magic) {
  case 0x7c596b46: res = print_ds_constructor_file_location_unavailable (DS, T); break;
  case 0x53d69076: res = print_ds_constructor_file_location (DS, T); break;
  default: return -1;
  }
  if (res >= 0) {
    if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); }
    eprintf (" )");
  }
  return res;
}
int print_ds_type_bare_file_location (struct tl_ds_file_location *DS, struct paramed_type *T) {
  int res;
  if (multiline_output >= 2) { multiline_offset += multiline_offset_size; }
  eprintf (" (");
  switch (DS->magic) {
  case 0x7c596b46: res = print_ds_constructor_file_location_unavailable (DS, T); break;
  case 0x53d69076: res = print_ds_constructor_file_location (DS, T); break;
  default: return -1;
  }
  if (res >= 0) {
    if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); }
    eprintf (" )");
  }
  return res;
}
int print_ds_type_geo_chat_message (struct tl_ds_geo_chat_message *DS, struct paramed_type *T) {
  int res;
  if (multiline_output >= 2) { multiline_offset += multiline_offset_size; }
  eprintf (" (");
  switch (DS->magic) {
  case 0x60311a9b: res = print_ds_constructor_geo_chat_message_empty (DS, T); break;
  case 0x4505f8e1: res = print_ds_constructor_geo_chat_message (DS, T); break;
  case 0xd34fa24e: res = print_ds_constructor_geo_chat_message_service (DS, T); break;
  default: return -1;
  }
  if (res >= 0) {
    if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); }
    eprintf (" )");
  }
  return res;
}
int print_ds_type_bare_geo_chat_message (struct tl_ds_geo_chat_message *DS, struct paramed_type *T) {
  int res;
  if (multiline_output >= 2) { multiline_offset += multiline_offset_size; }
  eprintf (" (");
  switch (DS->magic) {
  case 0x60311a9b: res = print_ds_constructor_geo_chat_message_empty (DS, T); break;
  case 0x4505f8e1: res = print_ds_constructor_geo_chat_message (DS, T); break;
  case 0xd34fa24e: res = print_ds_constructor_geo_chat_message_service (DS, T); break;
  default: return -1;
  }
  if (res >= 0) {
    if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); }
    eprintf (" )");
  }
  return res;
}
int print_ds_type_geo_point (struct tl_ds_geo_point *DS, struct paramed_type *T) {
  int res;
  if (multiline_output >= 2) { multiline_offset += multiline_offset_size; }
  eprintf (" (");
  switch (DS->magic) {
  case 0x1117dd5f: res = print_ds_constructor_geo_point_empty (DS, T); break;
  case 0x2049d70c: res = print_ds_constructor_geo_point (DS, T); break;
  default: return -1;
  }
  if (res >= 0) {
    if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); }
    eprintf (" )");
  }
  return res;
}
int print_ds_type_bare_geo_point (struct tl_ds_geo_point *DS, struct paramed_type *T) {
  int res;
  if (multiline_output >= 2) { multiline_offset += multiline_offset_size; }
  eprintf (" (");
  switch (DS->magic) {
  case 0x1117dd5f: res = print_ds_constructor_geo_point_empty (DS, T); break;
  case 0x2049d70c: res = print_ds_constructor_geo_point (DS, T); break;
  default: return -1;
  }
  if (res >= 0) {
    if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); }
    eprintf (" )");
  }
  return res;
}
int print_ds_type_imported_contact (struct tl_ds_imported_contact *DS, struct paramed_type *T) {
  int res;
  if (multiline_output >= 2) { multiline_offset += multiline_offset_size; }
  eprintf (" (");
  res = print_ds_constructor_imported_contact (DS, T);
  if (res >= 0) {
    if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); }
    eprintf (" )");
  }
  return res;
}
int print_ds_type_bare_imported_contact (struct tl_ds_imported_contact *DS, struct paramed_type *T) {
  int res;
  if (multiline_output >= 2) { multiline_offset += multiline_offset_size; }
  eprintf (" (");
  res = print_ds_constructor_imported_contact (DS, T);
  if (res >= 0) {
    if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); }
    eprintf (" )");
  }
  return res;
}
int print_ds_type_input_app_event (struct tl_ds_input_app_event *DS, struct paramed_type *T) {
  int res;
  if (multiline_output >= 2) { multiline_offset += multiline_offset_size; }
  eprintf (" (");
  res = print_ds_constructor_input_app_event (DS, T);
  if (res >= 0) {
    if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); }
    eprintf (" )");
  }
  return res;
}
int print_ds_type_bare_input_app_event (struct tl_ds_input_app_event *DS, struct paramed_type *T) {
  int res;
  if (multiline_output >= 2) { multiline_offset += multiline_offset_size; }
  eprintf (" (");
  res = print_ds_constructor_input_app_event (DS, T);
  if (res >= 0) {
    if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); }
    eprintf (" )");
  }
  return res;
}
int print_ds_type_input_audio (struct tl_ds_input_audio *DS, struct paramed_type *T) {
  int res;
  if (multiline_output >= 2) { multiline_offset += multiline_offset_size; }
  eprintf (" (");
  switch (DS->magic) {
  case 0xd95adc84: res = print_ds_constructor_input_audio_empty (DS, T); break;
  case 0x77d440ff: res = print_ds_constructor_input_audio (DS, T); break;
  default: return -1;
  }
  if (res >= 0) {
    if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); }
    eprintf (" )");
  }
  return res;
}
int print_ds_type_bare_input_audio (struct tl_ds_input_audio *DS, struct paramed_type *T) {
  int res;
  if (multiline_output >= 2) { multiline_offset += multiline_offset_size; }
  eprintf (" (");
  switch (DS->magic) {
  case 0xd95adc84: res = print_ds_constructor_input_audio_empty (DS, T); break;
  case 0x77d440ff: res = print_ds_constructor_input_audio (DS, T); break;
  default: return -1;
  }
  if (res >= 0) {
    if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); }
    eprintf (" )");
  }
  return res;
}
int print_ds_type_input_chat_photo (struct tl_ds_input_chat_photo *DS, struct paramed_type *T) {
  int res;
  if (multiline_output >= 2) { multiline_offset += multiline_offset_size; }
  eprintf (" (");
  switch (DS->magic) {
  case 0x1ca48f57: res = print_ds_constructor_input_chat_photo_empty (DS, T); break;
  case 0x94254732: res = print_ds_constructor_input_chat_uploaded_photo (DS, T); break;
  case 0xb2e1bf08: res = print_ds_constructor_input_chat_photo (DS, T); break;
  default: return -1;
  }
  if (res >= 0) {
    if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); }
    eprintf (" )");
  }
  return res;
}
int print_ds_type_bare_input_chat_photo (struct tl_ds_input_chat_photo *DS, struct paramed_type *T) {
  int res;
  if (multiline_output >= 2) { multiline_offset += multiline_offset_size; }
  eprintf (" (");
  switch (DS->magic) {
  case 0x1ca48f57: res = print_ds_constructor_input_chat_photo_empty (DS, T); break;
  case 0x94254732: res = print_ds_constructor_input_chat_uploaded_photo (DS, T); break;
  case 0xb2e1bf08: res = print_ds_constructor_input_chat_photo (DS, T); break;
  default: return -1;
  }
  if (res >= 0) {
    if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); }
    eprintf (" )");
  }
  return res;
}
int print_ds_type_input_contact (struct tl_ds_input_contact *DS, struct paramed_type *T) {
  int res;
  if (multiline_output >= 2) { multiline_offset += multiline_offset_size; }
  eprintf (" (");
  res = print_ds_constructor_input_phone_contact (DS, T);
  if (res >= 0) {
    if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); }
    eprintf (" )");
  }
  return res;
}
int print_ds_type_bare_input_contact (struct tl_ds_input_contact *DS, struct paramed_type *T) {
  int res;
  if (multiline_output >= 2) { multiline_offset += multiline_offset_size; }
  eprintf (" (");
  res = print_ds_constructor_input_phone_contact (DS, T);
  if (res >= 0) {
    if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); }
    eprintf (" )");
  }
  return res;
}
int print_ds_type_input_document (struct tl_ds_input_document *DS, struct paramed_type *T) {
  int res;
  if (multiline_output >= 2) { multiline_offset += multiline_offset_size; }
  eprintf (" (");
  switch (DS->magic) {
  case 0x72f0eaae: res = print_ds_constructor_input_document_empty (DS, T); break;
  case 0x18798952: res = print_ds_constructor_input_document (DS, T); break;
  default: return -1;
  }
  if (res >= 0) {
    if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); }
    eprintf (" )");
  }
  return res;
}
int print_ds_type_bare_input_document (struct tl_ds_input_document *DS, struct paramed_type *T) {
  int res;
  if (multiline_output >= 2) { multiline_offset += multiline_offset_size; }
  eprintf (" (");
  switch (DS->magic) {
  case 0x72f0eaae: res = print_ds_constructor_input_document_empty (DS, T); break;
  case 0x18798952: res = print_ds_constructor_input_document (DS, T); break;
  default: return -1;
  }
  if (res >= 0) {
    if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); }
    eprintf (" )");
  }
  return res;
}
int print_ds_type_input_encrypted_chat (struct tl_ds_input_encrypted_chat *DS, struct paramed_type *T) {
  int res;
  if (multiline_output >= 2) { multiline_offset += multiline_offset_size; }
  eprintf (" (");
  res = print_ds_constructor_input_encrypted_chat (DS, T);
  if (res >= 0) {
    if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); }
    eprintf (" )");
  }
  return res;
}
int print_ds_type_bare_input_encrypted_chat (struct tl_ds_input_encrypted_chat *DS, struct paramed_type *T) {
  int res;
  if (multiline_output >= 2) { multiline_offset += multiline_offset_size; }
  eprintf (" (");
  res = print_ds_constructor_input_encrypted_chat (DS, T);
  if (res >= 0) {
    if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); }
    eprintf (" )");
  }
  return res;
}
int print_ds_type_input_encrypted_file (struct tl_ds_input_encrypted_file *DS, struct paramed_type *T) {
  int res;
  if (multiline_output >= 2) { multiline_offset += multiline_offset_size; }
  eprintf (" (");
  switch (DS->magic) {
  case 0x1837c364: res = print_ds_constructor_input_encrypted_file_empty (DS, T); break;
  case 0x64bd0306: res = print_ds_constructor_input_encrypted_file_uploaded (DS, T); break;
  case 0x5a17b5e5: res = print_ds_constructor_input_encrypted_file (DS, T); break;
  case 0x2dc173c8: res = print_ds_constructor_input_encrypted_file_big_uploaded (DS, T); break;
  default: return -1;
  }
  if (res >= 0) {
    if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); }
    eprintf (" )");
  }
  return res;
}
int print_ds_type_bare_input_encrypted_file (struct tl_ds_input_encrypted_file *DS, struct paramed_type *T) {
  int res;
  if (multiline_output >= 2) { multiline_offset += multiline_offset_size; }
  eprintf (" (");
  switch (DS->magic) {
  case 0x1837c364: res = print_ds_constructor_input_encrypted_file_empty (DS, T); break;
  case 0x64bd0306: res = print_ds_constructor_input_encrypted_file_uploaded (DS, T); break;
  case 0x5a17b5e5: res = print_ds_constructor_input_encrypted_file (DS, T); break;
  case 0x2dc173c8: res = print_ds_constructor_input_encrypted_file_big_uploaded (DS, T); break;
  default: return -1;
  }
  if (res >= 0) {
    if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); }
    eprintf (" )");
  }
  return res;
}
int print_ds_type_input_file (struct tl_ds_input_file *DS, struct paramed_type *T) {
  int res;
  if (multiline_output >= 2) { multiline_offset += multiline_offset_size; }
  eprintf (" (");
  switch (DS->magic) {
  case 0xf52ff27f: res = print_ds_constructor_input_file (DS, T); break;
  case 0xfa4f0bb5: res = print_ds_constructor_input_file_big (DS, T); break;
  default: return -1;
  }
  if (res >= 0) {
    if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); }
    eprintf (" )");
  }
  return res;
}
int print_ds_type_bare_input_file (struct tl_ds_input_file *DS, struct paramed_type *T) {
  int res;
  if (multiline_output >= 2) { multiline_offset += multiline_offset_size; }
  eprintf (" (");
  switch (DS->magic) {
  case 0xf52ff27f: res = print_ds_constructor_input_file (DS, T); break;
  case 0xfa4f0bb5: res = print_ds_constructor_input_file_big (DS, T); break;
  default: return -1;
  }
  if (res >= 0) {
    if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); }
    eprintf (" )");
  }
  return res;
}
int print_ds_type_input_file_location (struct tl_ds_input_file_location *DS, struct paramed_type *T) {
  int res;
  if (multiline_output >= 2) { multiline_offset += multiline_offset_size; }
  eprintf (" (");
  switch (DS->magic) {
  case 0x14637196: res = print_ds_constructor_input_file_location (DS, T); break;
  case 0x3d0364ec: res = print_ds_constructor_input_video_file_location (DS, T); break;
  case 0xf5235d55: res = print_ds_constructor_input_encrypted_file_location (DS, T); break;
  case 0x74dc404d: res = print_ds_constructor_input_audio_file_location (DS, T); break;
  case 0x4e45abe9: res = print_ds_constructor_input_document_file_location (DS, T); break;
  default: return -1;
  }
  if (res >= 0) {
    if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); }
    eprintf (" )");
  }
  return res;
}
int print_ds_type_bare_input_file_location (struct tl_ds_input_file_location *DS, struct paramed_type *T) {
  int res;
  if (multiline_output >= 2) { multiline_offset += multiline_offset_size; }
  eprintf (" (");
  switch (DS->magic) {
  case 0x14637196: res = print_ds_constructor_input_file_location (DS, T); break;
  case 0x3d0364ec: res = print_ds_constructor_input_video_file_location (DS, T); break;
  case 0xf5235d55: res = print_ds_constructor_input_encrypted_file_location (DS, T); break;
  case 0x74dc404d: res = print_ds_constructor_input_audio_file_location (DS, T); break;
  case 0x4e45abe9: res = print_ds_constructor_input_document_file_location (DS, T); break;
  default: return -1;
  }
  if (res >= 0) {
    if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); }
    eprintf (" )");
  }
  return res;
}
int print_ds_type_input_geo_chat (struct tl_ds_input_geo_chat *DS, struct paramed_type *T) {
  int res;
  if (multiline_output >= 2) { multiline_offset += multiline_offset_size; }
  eprintf (" (");
  res = print_ds_constructor_input_geo_chat (DS, T);
  if (res >= 0) {
    if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); }
    eprintf (" )");
  }
  return res;
}
int print_ds_type_bare_input_geo_chat (struct tl_ds_input_geo_chat *DS, struct paramed_type *T) {
  int res;
  if (multiline_output >= 2) { multiline_offset += multiline_offset_size; }
  eprintf (" (");
  res = print_ds_constructor_input_geo_chat (DS, T);
  if (res >= 0) {
    if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); }
    eprintf (" )");
  }
  return res;
}
int print_ds_type_input_geo_point (struct tl_ds_input_geo_point *DS, struct paramed_type *T) {
  int res;
  if (multiline_output >= 2) { multiline_offset += multiline_offset_size; }
  eprintf (" (");
  switch (DS->magic) {
  case 0xe4c123d6: res = print_ds_constructor_input_geo_point_empty (DS, T); break;
  case 0xf3b7acc9: res = print_ds_constructor_input_geo_point (DS, T); break;
  default: return -1;
  }
  if (res >= 0) {
    if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); }
    eprintf (" )");
  }
  return res;
}
int print_ds_type_bare_input_geo_point (struct tl_ds_input_geo_point *DS, struct paramed_type *T) {
  int res;
  if (multiline_output >= 2) { multiline_offset += multiline_offset_size; }
  eprintf (" (");
  switch (DS->magic) {
  case 0xe4c123d6: res = print_ds_constructor_input_geo_point_empty (DS, T); break;
  case 0xf3b7acc9: res = print_ds_constructor_input_geo_point (DS, T); break;
  default: return -1;
  }
  if (res >= 0) {
    if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); }
    eprintf (" )");
  }
  return res;
}
int print_ds_type_input_media (struct tl_ds_input_media *DS, struct paramed_type *T) {
  int res;
  if (multiline_output >= 2) { multiline_offset += multiline_offset_size; }
  eprintf (" (");
  switch (DS->magic) {
  case 0x9664f57f: res = print_ds_constructor_input_media_empty (DS, T); break;
  case 0xf7aff1c0: res = print_ds_constructor_input_media_uploaded_photo (DS, T); break;
  case 0xe9bfb4f3: res = print_ds_constructor_input_media_photo (DS, T); break;
  case 0xf9c44144: res = print_ds_constructor_input_media_geo_point (DS, T); break;
  case 0xa6e45987: res = print_ds_constructor_input_media_contact (DS, T); break;
  case 0xe13fd4bc: res = print_ds_constructor_input_media_uploaded_video (DS, T); break;
  case 0x96fb97dc: res = print_ds_constructor_input_media_uploaded_thumb_video (DS, T); break;
  case 0x936a4ebd: res = print_ds_constructor_input_media_video (DS, T); break;
  case 0x4e498cab: res = print_ds_constructor_input_media_uploaded_audio (DS, T); break;
  case 0x89938781: res = print_ds_constructor_input_media_audio (DS, T); break;
  case 0xffe76b78: res = print_ds_constructor_input_media_uploaded_document (DS, T); break;
  case 0x41481486: res = print_ds_constructor_input_media_uploaded_thumb_document (DS, T); break;
  case 0xd184e841: res = print_ds_constructor_input_media_document (DS, T); break;
  case 0x2827a81a: res = print_ds_constructor_input_media_venue (DS, T); break;
  default: return -1;
  }
  if (res >= 0) {
    if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); }
    eprintf (" )");
  }
  return res;
}
int print_ds_type_bare_input_media (struct tl_ds_input_media *DS, struct paramed_type *T) {
  int res;
  if (multiline_output >= 2) { multiline_offset += multiline_offset_size; }
  eprintf (" (");
  switch (DS->magic) {
  case 0x9664f57f: res = print_ds_constructor_input_media_empty (DS, T); break;
  case 0xf7aff1c0: res = print_ds_constructor_input_media_uploaded_photo (DS, T); break;
  case 0xe9bfb4f3: res = print_ds_constructor_input_media_photo (DS, T); break;
  case 0xf9c44144: res = print_ds_constructor_input_media_geo_point (DS, T); break;
  case 0xa6e45987: res = print_ds_constructor_input_media_contact (DS, T); break;
  case 0xe13fd4bc: res = print_ds_constructor_input_media_uploaded_video (DS, T); break;
  case 0x96fb97dc: res = print_ds_constructor_input_media_uploaded_thumb_video (DS, T); break;
  case 0x936a4ebd: res = print_ds_constructor_input_media_video (DS, T); break;
  case 0x4e498cab: res = print_ds_constructor_input_media_uploaded_audio (DS, T); break;
  case 0x89938781: res = print_ds_constructor_input_media_audio (DS, T); break;
  case 0xffe76b78: res = print_ds_constructor_input_media_uploaded_document (DS, T); break;
  case 0x41481486: res = print_ds_constructor_input_media_uploaded_thumb_document (DS, T); break;
  case 0xd184e841: res = print_ds_constructor_input_media_document (DS, T); break;
  case 0x2827a81a: res = print_ds_constructor_input_media_venue (DS, T); break;
  default: return -1;
  }
  if (res >= 0) {
    if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); }
    eprintf (" )");
  }
  return res;
}
int print_ds_type_input_notify_peer (struct tl_ds_input_notify_peer *DS, struct paramed_type *T) {
  int res;
  if (multiline_output >= 2) { multiline_offset += multiline_offset_size; }
  eprintf (" (");
  switch (DS->magic) {
  case 0xb8bc5b0c: res = print_ds_constructor_input_notify_peer (DS, T); break;
  case 0x193b4417: res = print_ds_constructor_input_notify_users (DS, T); break;
  case 0x4a95e84e: res = print_ds_constructor_input_notify_chats (DS, T); break;
  case 0xa429b886: res = print_ds_constructor_input_notify_all (DS, T); break;
  case 0x4d8ddec8: res = print_ds_constructor_input_notify_geo_chat_peer (DS, T); break;
  default: return -1;
  }
  if (res >= 0) {
    if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); }
    eprintf (" )");
  }
  return res;
}
int print_ds_type_bare_input_notify_peer (struct tl_ds_input_notify_peer *DS, struct paramed_type *T) {
  int res;
  if (multiline_output >= 2) { multiline_offset += multiline_offset_size; }
  eprintf (" (");
  switch (DS->magic) {
  case 0xb8bc5b0c: res = print_ds_constructor_input_notify_peer (DS, T); break;
  case 0x193b4417: res = print_ds_constructor_input_notify_users (DS, T); break;
  case 0x4a95e84e: res = print_ds_constructor_input_notify_chats (DS, T); break;
  case 0xa429b886: res = print_ds_constructor_input_notify_all (DS, T); break;
  case 0x4d8ddec8: res = print_ds_constructor_input_notify_geo_chat_peer (DS, T); break;
  default: return -1;
  }
  if (res >= 0) {
    if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); }
    eprintf (" )");
  }
  return res;
}
int print_ds_type_input_peer (struct tl_ds_input_peer *DS, struct paramed_type *T) {
  int res;
  if (multiline_output >= 2) { multiline_offset += multiline_offset_size; }
  eprintf (" (");
  switch (DS->magic) {
  case 0x7f3b18ea: res = print_ds_constructor_input_peer_empty (DS, T); break;
  case 0x7da07ec9: res = print_ds_constructor_input_peer_self (DS, T); break;
  case 0x1023dbe8: res = print_ds_constructor_input_peer_contact (DS, T); break;
  case 0x9b447325: res = print_ds_constructor_input_peer_foreign (DS, T); break;
  case 0x179be863: res = print_ds_constructor_input_peer_chat (DS, T); break;
  default: return -1;
  }
  if (res >= 0) {
    if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); }
    eprintf (" )");
  }
  return res;
}
int print_ds_type_bare_input_peer (struct tl_ds_input_peer *DS, struct paramed_type *T) {
  int res;
  if (multiline_output >= 2) { multiline_offset += multiline_offset_size; }
  eprintf (" (");
  switch (DS->magic) {
  case 0x7f3b18ea: res = print_ds_constructor_input_peer_empty (DS, T); break;
  case 0x7da07ec9: res = print_ds_constructor_input_peer_self (DS, T); break;
  case 0x1023dbe8: res = print_ds_constructor_input_peer_contact (DS, T); break;
  case 0x9b447325: res = print_ds_constructor_input_peer_foreign (DS, T); break;
  case 0x179be863: res = print_ds_constructor_input_peer_chat (DS, T); break;
  default: return -1;
  }
  if (res >= 0) {
    if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); }
    eprintf (" )");
  }
  return res;
}
int print_ds_type_input_peer_notify_events (struct tl_ds_input_peer_notify_events *DS, struct paramed_type *T) {
  int res;
  if (multiline_output >= 2) { multiline_offset += multiline_offset_size; }
  eprintf (" (");
  switch (DS->magic) {
  case 0xf03064d8: res = print_ds_constructor_input_peer_notify_events_empty (DS, T); break;
  case 0xe86a2c74: res = print_ds_constructor_input_peer_notify_events_all (DS, T); break;
  default: return -1;
  }
  if (res >= 0) {
    if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); }
    eprintf (" )");
  }
  return res;
}
int print_ds_type_bare_input_peer_notify_events (struct tl_ds_input_peer_notify_events *DS, struct paramed_type *T) {
  int res;
  if (multiline_output >= 2) { multiline_offset += multiline_offset_size; }
  eprintf (" (");
  switch (DS->magic) {
  case 0xf03064d8: res = print_ds_constructor_input_peer_notify_events_empty (DS, T); break;
  case 0xe86a2c74: res = print_ds_constructor_input_peer_notify_events_all (DS, T); break;
  default: return -1;
  }
  if (res >= 0) {
    if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); }
    eprintf (" )");
  }
  return res;
}
int print_ds_type_input_peer_notify_settings (struct tl_ds_input_peer_notify_settings *DS, struct paramed_type *T) {
  int res;
  if (multiline_output >= 2) { multiline_offset += multiline_offset_size; }
  eprintf (" (");
  res = print_ds_constructor_input_peer_notify_settings (DS, T);
  if (res >= 0) {
    if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); }
    eprintf (" )");
  }
  return res;
}
int print_ds_type_bare_input_peer_notify_settings (struct tl_ds_input_peer_notify_settings *DS, struct paramed_type *T) {
  int res;
  if (multiline_output >= 2) { multiline_offset += multiline_offset_size; }
  eprintf (" (");
  res = print_ds_constructor_input_peer_notify_settings (DS, T);
  if (res >= 0) {
    if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); }
    eprintf (" )");
  }
  return res;
}
int print_ds_type_input_photo (struct tl_ds_input_photo *DS, struct paramed_type *T) {
  int res;
  if (multiline_output >= 2) { multiline_offset += multiline_offset_size; }
  eprintf (" (");
  switch (DS->magic) {
  case 0x1cd7bf0d: res = print_ds_constructor_input_photo_empty (DS, T); break;
  case 0xfb95c6c4: res = print_ds_constructor_input_photo (DS, T); break;
  default: return -1;
  }
  if (res >= 0) {
    if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); }
    eprintf (" )");
  }
  return res;
}
int print_ds_type_bare_input_photo (struct tl_ds_input_photo *DS, struct paramed_type *T) {
  int res;
  if (multiline_output >= 2) { multiline_offset += multiline_offset_size; }
  eprintf (" (");
  switch (DS->magic) {
  case 0x1cd7bf0d: res = print_ds_constructor_input_photo_empty (DS, T); break;
  case 0xfb95c6c4: res = print_ds_constructor_input_photo (DS, T); break;
  default: return -1;
  }
  if (res >= 0) {
    if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); }
    eprintf (" )");
  }
  return res;
}
int print_ds_type_input_photo_crop (struct tl_ds_input_photo_crop *DS, struct paramed_type *T) {
  int res;
  if (multiline_output >= 2) { multiline_offset += multiline_offset_size; }
  eprintf (" (");
  switch (DS->magic) {
  case 0xade6b004: res = print_ds_constructor_input_photo_crop_auto (DS, T); break;
  case 0xd9915325: res = print_ds_constructor_input_photo_crop (DS, T); break;
  default: return -1;
  }
  if (res >= 0) {
    if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); }
    eprintf (" )");
  }
  return res;
}
int print_ds_type_bare_input_photo_crop (struct tl_ds_input_photo_crop *DS, struct paramed_type *T) {
  int res;
  if (multiline_output >= 2) { multiline_offset += multiline_offset_size; }
  eprintf (" (");
  switch (DS->magic) {
  case 0xade6b004: res = print_ds_constructor_input_photo_crop_auto (DS, T); break;
  case 0xd9915325: res = print_ds_constructor_input_photo_crop (DS, T); break;
  default: return -1;
  }
  if (res >= 0) {
    if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); }
    eprintf (" )");
  }
  return res;
}
int print_ds_type_input_privacy_key (struct tl_ds_input_privacy_key *DS, struct paramed_type *T) {
  int res;
  if (multiline_output >= 2) { multiline_offset += multiline_offset_size; }
  eprintf (" (");
  res = print_ds_constructor_input_privacy_key_status_timestamp (DS, T);
  if (res >= 0) {
    if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); }
    eprintf (" )");
  }
  return res;
}
int print_ds_type_bare_input_privacy_key (struct tl_ds_input_privacy_key *DS, struct paramed_type *T) {
  int res;
  if (multiline_output >= 2) { multiline_offset += multiline_offset_size; }
  eprintf (" (");
  res = print_ds_constructor_input_privacy_key_status_timestamp (DS, T);
  if (res >= 0) {
    if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); }
    eprintf (" )");
  }
  return res;
}
int print_ds_type_input_privacy_rule (struct tl_ds_input_privacy_rule *DS, struct paramed_type *T) {
  int res;
  if (multiline_output >= 2) { multiline_offset += multiline_offset_size; }
  eprintf (" (");
  switch (DS->magic) {
  case 0x0d09e07b: res = print_ds_constructor_input_privacy_value_allow_contacts (DS, T); break;
  case 0x184b35ce: res = print_ds_constructor_input_privacy_value_allow_all (DS, T); break;
  case 0x131cc67f: res = print_ds_constructor_input_privacy_value_allow_users (DS, T); break;
  case 0x0ba52007: res = print_ds_constructor_input_privacy_value_disallow_contacts (DS, T); break;
  case 0xd66b66c9: res = print_ds_constructor_input_privacy_value_disallow_all (DS, T); break;
  case 0x90110467: res = print_ds_constructor_input_privacy_value_disallow_users (DS, T); break;
  default: return -1;
  }
  if (res >= 0) {
    if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); }
    eprintf (" )");
  }
  return res;
}
int print_ds_type_bare_input_privacy_rule (struct tl_ds_input_privacy_rule *DS, struct paramed_type *T) {
  int res;
  if (multiline_output >= 2) { multiline_offset += multiline_offset_size; }
  eprintf (" (");
  switch (DS->magic) {
  case 0x0d09e07b: res = print_ds_constructor_input_privacy_value_allow_contacts (DS, T); break;
  case 0x184b35ce: res = print_ds_constructor_input_privacy_value_allow_all (DS, T); break;
  case 0x131cc67f: res = print_ds_constructor_input_privacy_value_allow_users (DS, T); break;
  case 0x0ba52007: res = print_ds_constructor_input_privacy_value_disallow_contacts (DS, T); break;
  case 0xd66b66c9: res = print_ds_constructor_input_privacy_value_disallow_all (DS, T); break;
  case 0x90110467: res = print_ds_constructor_input_privacy_value_disallow_users (DS, T); break;
  default: return -1;
  }
  if (res >= 0) {
    if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); }
    eprintf (" )");
  }
  return res;
}
int print_ds_type_input_sticker_set (struct tl_ds_input_sticker_set *DS, struct paramed_type *T) {
  int res;
  if (multiline_output >= 2) { multiline_offset += multiline_offset_size; }
  eprintf (" (");
  switch (DS->magic) {
  case 0xffb62b95: res = print_ds_constructor_input_sticker_set_empty (DS, T); break;
  case 0x9de7a269: res = print_ds_constructor_input_sticker_set_i_d (DS, T); break;
  case 0x861cc8a0: res = print_ds_constructor_input_sticker_set_short_name (DS, T); break;
  default: return -1;
  }
  if (res >= 0) {
    if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); }
    eprintf (" )");
  }
  return res;
}
int print_ds_type_bare_input_sticker_set (struct tl_ds_input_sticker_set *DS, struct paramed_type *T) {
  int res;
  if (multiline_output >= 2) { multiline_offset += multiline_offset_size; }
  eprintf (" (");
  switch (DS->magic) {
  case 0xffb62b95: res = print_ds_constructor_input_sticker_set_empty (DS, T); break;
  case 0x9de7a269: res = print_ds_constructor_input_sticker_set_i_d (DS, T); break;
  case 0x861cc8a0: res = print_ds_constructor_input_sticker_set_short_name (DS, T); break;
  default: return -1;
  }
  if (res >= 0) {
    if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); }
    eprintf (" )");
  }
  return res;
}
int print_ds_type_input_user (struct tl_ds_input_user *DS, struct paramed_type *T) {
  int res;
  if (multiline_output >= 2) { multiline_offset += multiline_offset_size; }
  eprintf (" (");
  switch (DS->magic) {
  case 0xb98886cf: res = print_ds_constructor_input_user_empty (DS, T); break;
  case 0xf7c1b13f: res = print_ds_constructor_input_user_self (DS, T); break;
  case 0x86e94f65: res = print_ds_constructor_input_user_contact (DS, T); break;
  case 0x655e74ff: res = print_ds_constructor_input_user_foreign (DS, T); break;
  default: return -1;
  }
  if (res >= 0) {
    if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); }
    eprintf (" )");
  }
  return res;
}
int print_ds_type_bare_input_user (struct tl_ds_input_user *DS, struct paramed_type *T) {
  int res;
  if (multiline_output >= 2) { multiline_offset += multiline_offset_size; }
  eprintf (" (");
  switch (DS->magic) {
  case 0xb98886cf: res = print_ds_constructor_input_user_empty (DS, T); break;
  case 0xf7c1b13f: res = print_ds_constructor_input_user_self (DS, T); break;
  case 0x86e94f65: res = print_ds_constructor_input_user_contact (DS, T); break;
  case 0x655e74ff: res = print_ds_constructor_input_user_foreign (DS, T); break;
  default: return -1;
  }
  if (res >= 0) {
    if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); }
    eprintf (" )");
  }
  return res;
}
int print_ds_type_input_video (struct tl_ds_input_video *DS, struct paramed_type *T) {
  int res;
  if (multiline_output >= 2) { multiline_offset += multiline_offset_size; }
  eprintf (" (");
  switch (DS->magic) {
  case 0x5508ec75: res = print_ds_constructor_input_video_empty (DS, T); break;
  case 0xee579652: res = print_ds_constructor_input_video (DS, T); break;
  default: return -1;
  }
  if (res >= 0) {
    if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); }
    eprintf (" )");
  }
  return res;
}
int print_ds_type_bare_input_video (struct tl_ds_input_video *DS, struct paramed_type *T) {
  int res;
  if (multiline_output >= 2) { multiline_offset += multiline_offset_size; }
  eprintf (" (");
  switch (DS->magic) {
  case 0x5508ec75: res = print_ds_constructor_input_video_empty (DS, T); break;
  case 0xee579652: res = print_ds_constructor_input_video (DS, T); break;
  default: return -1;
  }
  if (res >= 0) {
    if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); }
    eprintf (" )");
  }
  return res;
}
int print_ds_type_int (int *DS, struct paramed_type *T) {
  int res;
  res = print_ds_constructor_int (DS, T);
  return res;
}
int print_ds_type_bare_int (int *DS, struct paramed_type *T) {
  int res;
  res = print_ds_constructor_int (DS, T);
  return res;
}
int print_ds_type_int128 (struct tl_ds_int128 *DS, struct paramed_type *T) {
  int res;
  if (multiline_output >= 2) { multiline_offset += multiline_offset_size; }
  eprintf (" (");
  res = print_ds_constructor_int128 (DS, T);
  if (res >= 0) {
    if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); }
    eprintf (" )");
  }
  return res;
}
int print_ds_type_bare_int128 (struct tl_ds_int128 *DS, struct paramed_type *T) {
  int res;
  if (multiline_output >= 2) { multiline_offset += multiline_offset_size; }
  eprintf (" (");
  res = print_ds_constructor_int128 (DS, T);
  if (res >= 0) {
    if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); }
    eprintf (" )");
  }
  return res;
}
int print_ds_type_int256 (struct tl_ds_int256 *DS, struct paramed_type *T) {
  int res;
  if (multiline_output >= 2) { multiline_offset += multiline_offset_size; }
  eprintf (" (");
  res = print_ds_constructor_int256 (DS, T);
  if (res >= 0) {
    if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); }
    eprintf (" )");
  }
  return res;
}
int print_ds_type_bare_int256 (struct tl_ds_int256 *DS, struct paramed_type *T) {
  int res;
  if (multiline_output >= 2) { multiline_offset += multiline_offset_size; }
  eprintf (" (");
  res = print_ds_constructor_int256 (DS, T);
  if (res >= 0) {
    if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); }
    eprintf (" )");
  }
  return res;
}
int print_ds_type_keyboard_button (struct tl_ds_keyboard_button *DS, struct paramed_type *T) {
  int res;
  res = print_ds_constructor_keyboard_button (DS, T);
  return res;
}
int print_ds_type_bare_keyboard_button (struct tl_ds_keyboard_button *DS, struct paramed_type *T) {
  int res;
  res = print_ds_constructor_keyboard_button (DS, T);
  return res;
}
int print_ds_type_keyboard_button_row (struct tl_ds_keyboard_button_row *DS, struct paramed_type *T) {
  int res;
  res = print_ds_constructor_keyboard_button_row (DS, T);
  return res;
}
int print_ds_type_bare_keyboard_button_row (struct tl_ds_keyboard_button_row *DS, struct paramed_type *T) {
  int res;
  res = print_ds_constructor_keyboard_button_row (DS, T);
  return res;
}
int print_ds_type_long (long long *DS, struct paramed_type *T) {
  int res;
  res = print_ds_constructor_long (DS, T);
  return res;
}
int print_ds_type_bare_long (long long *DS, struct paramed_type *T) {
  int res;
  res = print_ds_constructor_long (DS, T);
  return res;
}
int print_ds_type_message (struct tl_ds_message *DS, struct paramed_type *T) {
  int res;
  if (multiline_output >= 2) { multiline_offset += multiline_offset_size; }
  eprintf (" (");
  switch (DS->magic) {
  case 0x83e5de54: res = print_ds_constructor_message_empty (DS, T); break;
  case 0xc3060325: res = print_ds_constructor_message (DS, T); break;
  case 0x1d86f70e: res = print_ds_constructor_message_service (DS, T); break;
  default: return -1;
  }
  if (res >= 0) {
    if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); }
    eprintf (" )");
  }
  return res;
}
int print_ds_type_bare_message (struct tl_ds_message *DS, struct paramed_type *T) {
  int res;
  if (multiline_output >= 2) { multiline_offset += multiline_offset_size; }
  eprintf (" (");
  switch (DS->magic) {
  case 0x83e5de54: res = print_ds_constructor_message_empty (DS, T); break;
  case 0xc3060325: res = print_ds_constructor_message (DS, T); break;
  case 0x1d86f70e: res = print_ds_constructor_message_service (DS, T); break;
  default: return -1;
  }
  if (res >= 0) {
    if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); }
    eprintf (" )");
  }
  return res;
}
int print_ds_type_message_action (struct tl_ds_message_action *DS, struct paramed_type *T) {
  int res;
  if (multiline_output >= 2) { multiline_offset += multiline_offset_size; }
  eprintf (" (");
  switch (DS->magic) {
  case 0xb6aef7b0: res = print_ds_constructor_message_action_empty (DS, T); break;
  case 0xa6638b9a: res = print_ds_constructor_message_action_chat_create (DS, T); break;
  case 0xb5a1ce5a: res = print_ds_constructor_message_action_chat_edit_title (DS, T); break;
  case 0x7fcb13a8: res = print_ds_constructor_message_action_chat_edit_photo (DS, T); break;
  case 0x95e3fbef: res = print_ds_constructor_message_action_chat_delete_photo (DS, T); break;
  case 0x5e3cfc4b: res = print_ds_constructor_message_action_chat_add_user (DS, T); break;
  case 0xb2ae9b0c: res = print_ds_constructor_message_action_chat_delete_user (DS, T); break;
  case 0x6f038ebc: res = print_ds_constructor_message_action_geo_chat_create (DS, T); break;
  case 0x0c7d53de: res = print_ds_constructor_message_action_geo_chat_checkin (DS, T); break;
  case 0xf89cf5e8: res = print_ds_constructor_message_action_chat_joined_by_link (DS, T); break;
  default: return -1;
  }
  if (res >= 0) {
    if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); }
    eprintf (" )");
  }
  return res;
}
int print_ds_type_bare_message_action (struct tl_ds_message_action *DS, struct paramed_type *T) {
  int res;
  if (multiline_output >= 2) { multiline_offset += multiline_offset_size; }
  eprintf (" (");
  switch (DS->magic) {
  case 0xb6aef7b0: res = print_ds_constructor_message_action_empty (DS, T); break;
  case 0xa6638b9a: res = print_ds_constructor_message_action_chat_create (DS, T); break;
  case 0xb5a1ce5a: res = print_ds_constructor_message_action_chat_edit_title (DS, T); break;
  case 0x7fcb13a8: res = print_ds_constructor_message_action_chat_edit_photo (DS, T); break;
  case 0x95e3fbef: res = print_ds_constructor_message_action_chat_delete_photo (DS, T); break;
  case 0x5e3cfc4b: res = print_ds_constructor_message_action_chat_add_user (DS, T); break;
  case 0xb2ae9b0c: res = print_ds_constructor_message_action_chat_delete_user (DS, T); break;
  case 0x6f038ebc: res = print_ds_constructor_message_action_geo_chat_create (DS, T); break;
  case 0x0c7d53de: res = print_ds_constructor_message_action_geo_chat_checkin (DS, T); break;
  case 0xf89cf5e8: res = print_ds_constructor_message_action_chat_joined_by_link (DS, T); break;
  default: return -1;
  }
  if (res >= 0) {
    if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); }
    eprintf (" )");
  }
  return res;
}
int print_ds_type_message_media (struct tl_ds_message_media *DS, struct paramed_type *T) {
  int res;
  if (multiline_output >= 2) { multiline_offset += multiline_offset_size; }
  eprintf (" (");
  switch (DS->magic) {
  case 0x3ded6320: res = print_ds_constructor_message_media_empty (DS, T); break;
  case 0x3d8ce53d: res = print_ds_constructor_message_media_photo (DS, T); break;
  case 0x5bcf1675: res = print_ds_constructor_message_media_video (DS, T); break;
  case 0x56e0d474: res = print_ds_constructor_message_media_geo (DS, T); break;
  case 0x5e7d2f39: res = print_ds_constructor_message_media_contact (DS, T); break;
  case 0x9f84f49e: res = print_ds_constructor_message_media_unsupported (DS, T); break;
  case 0x2fda2204: res = print_ds_constructor_message_media_document (DS, T); break;
  case 0xc6b68300: res = print_ds_constructor_message_media_audio (DS, T); break;
  case 0xa32dd600: res = print_ds_constructor_message_media_web_page (DS, T); break;
  case 0x7912b71f: res = print_ds_constructor_message_media_venue (DS, T); break;
  case 0xc8c45a2a: res = print_ds_constructor_message_media_photo_l27 (DS, T); break;
  case 0xa2d24290: res = print_ds_constructor_message_media_video_l27 (DS, T); break;
  default: return -1;
  }
  if (res >= 0) {
    if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); }
    eprintf (" )");
  }
  return res;
}
int print_ds_type_bare_message_media (struct tl_ds_message_media *DS, struct paramed_type *T) {
  int res;
  if (multiline_output >= 2) { multiline_offset += multiline_offset_size; }
  eprintf (" (");
  switch (DS->magic) {
  case 0x3ded6320: res = print_ds_constructor_message_media_empty (DS, T); break;
  case 0x3d8ce53d: res = print_ds_constructor_message_media_photo (DS, T); break;
  case 0x5bcf1675: res = print_ds_constructor_message_media_video (DS, T); break;
  case 0x56e0d474: res = print_ds_constructor_message_media_geo (DS, T); break;
  case 0x5e7d2f39: res = print_ds_constructor_message_media_contact (DS, T); break;
  case 0x9f84f49e: res = print_ds_constructor_message_media_unsupported (DS, T); break;
  case 0x2fda2204: res = print_ds_constructor_message_media_document (DS, T); break;
  case 0xc6b68300: res = print_ds_constructor_message_media_audio (DS, T); break;
  case 0xa32dd600: res = print_ds_constructor_message_media_web_page (DS, T); break;
  case 0x7912b71f: res = print_ds_constructor_message_media_venue (DS, T); break;
  case 0xc8c45a2a: res = print_ds_constructor_message_media_photo_l27 (DS, T); break;
  case 0xa2d24290: res = print_ds_constructor_message_media_video_l27 (DS, T); break;
  default: return -1;
  }
  if (res >= 0) {
    if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); }
    eprintf (" )");
  }
  return res;
}
int print_ds_type_messages_filter (struct tl_ds_messages_filter *DS, struct paramed_type *T) {
  int res;
  if (multiline_output >= 2) { multiline_offset += multiline_offset_size; }
  eprintf (" (");
  switch (DS->magic) {
  case 0x57e2f66c: res = print_ds_constructor_input_messages_filter_empty (DS, T); break;
  case 0x9609a51c: res = print_ds_constructor_input_messages_filter_photos (DS, T); break;
  case 0x9fc00e65: res = print_ds_constructor_input_messages_filter_video (DS, T); break;
  case 0x56e9f0e4: res = print_ds_constructor_input_messages_filter_photo_video (DS, T); break;
  case 0xd95e73bb: res = print_ds_constructor_input_messages_filter_photo_video_documents (DS, T); break;
  case 0x9eddf188: res = print_ds_constructor_input_messages_filter_document (DS, T); break;
  case 0xcfc87522: res = print_ds_constructor_input_messages_filter_audio (DS, T); break;
  default: return -1;
  }
  if (res >= 0) {
    if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); }
    eprintf (" )");
  }
  return res;
}
int print_ds_type_bare_messages_filter (struct tl_ds_messages_filter *DS, struct paramed_type *T) {
  int res;
  if (multiline_output >= 2) { multiline_offset += multiline_offset_size; }
  eprintf (" (");
  switch (DS->magic) {
  case 0x57e2f66c: res = print_ds_constructor_input_messages_filter_empty (DS, T); break;
  case 0x9609a51c: res = print_ds_constructor_input_messages_filter_photos (DS, T); break;
  case 0x9fc00e65: res = print_ds_constructor_input_messages_filter_video (DS, T); break;
  case 0x56e9f0e4: res = print_ds_constructor_input_messages_filter_photo_video (DS, T); break;
  case 0xd95e73bb: res = print_ds_constructor_input_messages_filter_photo_video_documents (DS, T); break;
  case 0x9eddf188: res = print_ds_constructor_input_messages_filter_document (DS, T); break;
  case 0xcfc87522: res = print_ds_constructor_input_messages_filter_audio (DS, T); break;
  default: return -1;
  }
  if (res >= 0) {
    if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); }
    eprintf (" )");
  }
  return res;
}
int print_ds_type_nearest_dc (struct tl_ds_nearest_dc *DS, struct paramed_type *T) {
  int res;
  if (multiline_output >= 2) { multiline_offset += multiline_offset_size; }
  eprintf (" (");
  res = print_ds_constructor_nearest_dc (DS, T);
  if (res >= 0) {
    if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); }
    eprintf (" )");
  }
  return res;
}
int print_ds_type_bare_nearest_dc (struct tl_ds_nearest_dc *DS, struct paramed_type *T) {
  int res;
  if (multiline_output >= 2) { multiline_offset += multiline_offset_size; }
  eprintf (" (");
  res = print_ds_constructor_nearest_dc (DS, T);
  if (res >= 0) {
    if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); }
    eprintf (" )");
  }
  return res;
}
int print_ds_type_notify_peer (struct tl_ds_notify_peer *DS, struct paramed_type *T) {
  int res;
  if (multiline_output >= 2) { multiline_offset += multiline_offset_size; }
  eprintf (" (");
  switch (DS->magic) {
  case 0x9fd40bd8: res = print_ds_constructor_notify_peer (DS, T); break;
  case 0xb4c83b4c: res = print_ds_constructor_notify_users (DS, T); break;
  case 0xc007cec3: res = print_ds_constructor_notify_chats (DS, T); break;
  case 0x74d07c60: res = print_ds_constructor_notify_all (DS, T); break;
  default: return -1;
  }
  if (res >= 0) {
    if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); }
    eprintf (" )");
  }
  return res;
}
int print_ds_type_bare_notify_peer (struct tl_ds_notify_peer *DS, struct paramed_type *T) {
  int res;
  if (multiline_output >= 2) { multiline_offset += multiline_offset_size; }
  eprintf (" (");
  switch (DS->magic) {
  case 0x9fd40bd8: res = print_ds_constructor_notify_peer (DS, T); break;
  case 0xb4c83b4c: res = print_ds_constructor_notify_users (DS, T); break;
  case 0xc007cec3: res = print_ds_constructor_notify_chats (DS, T); break;
  case 0x74d07c60: res = print_ds_constructor_notify_all (DS, T); break;
  default: return -1;
  }
  if (res >= 0) {
    if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); }
    eprintf (" )");
  }
  return res;
}
int print_ds_type_null (struct tl_ds_null *DS, struct paramed_type *T) {
  int res;
  if (multiline_output >= 2) { multiline_offset += multiline_offset_size; }
  eprintf (" (");
  res = print_ds_constructor_null (DS, T);
  if (res >= 0) {
    if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); }
    eprintf (" )");
  }
  return res;
}
int print_ds_type_bare_null (struct tl_ds_null *DS, struct paramed_type *T) {
  int res;
  if (multiline_output >= 2) { multiline_offset += multiline_offset_size; }
  eprintf (" (");
  res = print_ds_constructor_null (DS, T);
  if (res >= 0) {
    if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); }
    eprintf (" )");
  }
  return res;
}
int print_ds_type_p_q_inner_data (struct tl_ds_p_q_inner_data *DS, struct paramed_type *T) {
  int res;
  if (multiline_output >= 2) { multiline_offset += multiline_offset_size; }
  eprintf (" (");
  switch (DS->magic) {
  case 0x83c95aec: res = print_ds_constructor_p_q_inner_data (DS, T); break;
  case 0x3c6a84d4: res = print_ds_constructor_p_q_inner_data_temp (DS, T); break;
  default: return -1;
  }
  if (res >= 0) {
    if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); }
    eprintf (" )");
  }
  return res;
}
int print_ds_type_bare_p_q_inner_data (struct tl_ds_p_q_inner_data *DS, struct paramed_type *T) {
  int res;
  if (multiline_output >= 2) { multiline_offset += multiline_offset_size; }
  eprintf (" (");
  switch (DS->magic) {
  case 0x83c95aec: res = print_ds_constructor_p_q_inner_data (DS, T); break;
  case 0x3c6a84d4: res = print_ds_constructor_p_q_inner_data_temp (DS, T); break;
  default: return -1;
  }
  if (res >= 0) {
    if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); }
    eprintf (" )");
  }
  return res;
}
int print_ds_type_peer (struct tl_ds_peer *DS, struct paramed_type *T) {
  int res;
  if (multiline_output >= 2) { multiline_offset += multiline_offset_size; }
  eprintf (" (");
  switch (DS->magic) {
  case 0x9db1bc6d: res = print_ds_constructor_peer_user (DS, T); break;
  case 0xbad0e5bb: res = print_ds_constructor_peer_chat (DS, T); break;
  default: return -1;
  }
  if (res >= 0) {
    if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); }
    eprintf (" )");
  }
  return res;
}
int print_ds_type_bare_peer (struct tl_ds_peer *DS, struct paramed_type *T) {
  int res;
  if (multiline_output >= 2) { multiline_offset += multiline_offset_size; }
  eprintf (" (");
  switch (DS->magic) {
  case 0x9db1bc6d: res = print_ds_constructor_peer_user (DS, T); break;
  case 0xbad0e5bb: res = print_ds_constructor_peer_chat (DS, T); break;
  default: return -1;
  }
  if (res >= 0) {
    if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); }
    eprintf (" )");
  }
  return res;
}
int print_ds_type_peer_notify_events (struct tl_ds_peer_notify_events *DS, struct paramed_type *T) {
  int res;
  if (multiline_output >= 2) { multiline_offset += multiline_offset_size; }
  eprintf (" (");
  switch (DS->magic) {
  case 0xadd53cb3: res = print_ds_constructor_peer_notify_events_empty (DS, T); break;
  case 0x6d1ded88: res = print_ds_constructor_peer_notify_events_all (DS, T); break;
  default: return -1;
  }
  if (res >= 0) {
    if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); }
    eprintf (" )");
  }
  return res;
}
int print_ds_type_bare_peer_notify_events (struct tl_ds_peer_notify_events *DS, struct paramed_type *T) {
  int res;
  if (multiline_output >= 2) { multiline_offset += multiline_offset_size; }
  eprintf (" (");
  switch (DS->magic) {
  case 0xadd53cb3: res = print_ds_constructor_peer_notify_events_empty (DS, T); break;
  case 0x6d1ded88: res = print_ds_constructor_peer_notify_events_all (DS, T); break;
  default: return -1;
  }
  if (res >= 0) {
    if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); }
    eprintf (" )");
  }
  return res;
}
int print_ds_type_peer_notify_settings (struct tl_ds_peer_notify_settings *DS, struct paramed_type *T) {
  int res;
  if (multiline_output >= 2) { multiline_offset += multiline_offset_size; }
  eprintf (" (");
  switch (DS->magic) {
  case 0x70a68512: res = print_ds_constructor_peer_notify_settings_empty (DS, T); break;
  case 0x8d5e11ee: res = print_ds_constructor_peer_notify_settings (DS, T); break;
  default: return -1;
  }
  if (res >= 0) {
    if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); }
    eprintf (" )");
  }
  return res;
}
int print_ds_type_bare_peer_notify_settings (struct tl_ds_peer_notify_settings *DS, struct paramed_type *T) {
  int res;
  if (multiline_output >= 2) { multiline_offset += multiline_offset_size; }
  eprintf (" (");
  switch (DS->magic) {
  case 0x70a68512: res = print_ds_constructor_peer_notify_settings_empty (DS, T); break;
  case 0x8d5e11ee: res = print_ds_constructor_peer_notify_settings (DS, T); break;
  default: return -1;
  }
  if (res >= 0) {
    if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); }
    eprintf (" )");
  }
  return res;
}
int print_ds_type_photo (struct tl_ds_photo *DS, struct paramed_type *T) {
  int res;
  if (multiline_output >= 2) { multiline_offset += multiline_offset_size; }
  eprintf (" (");
  switch (DS->magic) {
  case 0x2331b22d: res = print_ds_constructor_photo_empty (DS, T); break;
  case 0xc3838076: res = print_ds_constructor_photo (DS, T); break;
  case 0x22b56751: res = print_ds_constructor_photo_l27 (DS, T); break;
  default: return -1;
  }
  if (res >= 0) {
    if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); }
    eprintf (" )");
  }
  return res;
}
int print_ds_type_bare_photo (struct tl_ds_photo *DS, struct paramed_type *T) {
  int res;
  if (multiline_output >= 2) { multiline_offset += multiline_offset_size; }
  eprintf (" (");
  switch (DS->magic) {
  case 0x2331b22d: res = print_ds_constructor_photo_empty (DS, T); break;
  case 0xc3838076: res = print_ds_constructor_photo (DS, T); break;
  case 0x22b56751: res = print_ds_constructor_photo_l27 (DS, T); break;
  default: return -1;
  }
  if (res >= 0) {
    if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); }
    eprintf (" )");
  }
  return res;
}
int print_ds_type_photo_size (struct tl_ds_photo_size *DS, struct paramed_type *T) {
  int res;
  if (multiline_output >= 2) { multiline_offset += multiline_offset_size; }
  eprintf (" (");
  switch (DS->magic) {
  case 0x0e17e23c: res = print_ds_constructor_photo_size_empty (DS, T); break;
  case 0x77bfb61b: res = print_ds_constructor_photo_size (DS, T); break;
  case 0xe9a734fa: res = print_ds_constructor_photo_cached_size (DS, T); break;
  default: return -1;
  }
  if (res >= 0) {
    if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); }
    eprintf (" )");
  }
  return res;
}
int print_ds_type_bare_photo_size (struct tl_ds_photo_size *DS, struct paramed_type *T) {
  int res;
  if (multiline_output >= 2) { multiline_offset += multiline_offset_size; }
  eprintf (" (");
  switch (DS->magic) {
  case 0x0e17e23c: res = print_ds_constructor_photo_size_empty (DS, T); break;
  case 0x77bfb61b: res = print_ds_constructor_photo_size (DS, T); break;
  case 0xe9a734fa: res = print_ds_constructor_photo_cached_size (DS, T); break;
  default: return -1;
  }
  if (res >= 0) {
    if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); }
    eprintf (" )");
  }
  return res;
}
int print_ds_type_privacy_key (struct tl_ds_privacy_key *DS, struct paramed_type *T) {
  int res;
  if (multiline_output >= 2) { multiline_offset += multiline_offset_size; }
  eprintf (" (");
  res = print_ds_constructor_privacy_key_status_timestamp (DS, T);
  if (res >= 0) {
    if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); }
    eprintf (" )");
  }
  return res;
}
int print_ds_type_bare_privacy_key (struct tl_ds_privacy_key *DS, struct paramed_type *T) {
  int res;
  if (multiline_output >= 2) { multiline_offset += multiline_offset_size; }
  eprintf (" (");
  res = print_ds_constructor_privacy_key_status_timestamp (DS, T);
  if (res >= 0) {
    if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); }
    eprintf (" )");
  }
  return res;
}
int print_ds_type_privacy_rule (struct tl_ds_privacy_rule *DS, struct paramed_type *T) {
  int res;
  if (multiline_output >= 2) { multiline_offset += multiline_offset_size; }
  eprintf (" (");
  switch (DS->magic) {
  case 0xfffe1bac: res = print_ds_constructor_privacy_value_allow_contacts (DS, T); break;
  case 0x65427b82: res = print_ds_constructor_privacy_value_allow_all (DS, T); break;
  case 0x4d5bbe0c: res = print_ds_constructor_privacy_value_allow_users (DS, T); break;
  case 0xf888fa1a: res = print_ds_constructor_privacy_value_disallow_contacts (DS, T); break;
  case 0x8b73e763: res = print_ds_constructor_privacy_value_disallow_all (DS, T); break;
  case 0x0c7f49b7: res = print_ds_constructor_privacy_value_disallow_users (DS, T); break;
  default: return -1;
  }
  if (res >= 0) {
    if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); }
    eprintf (" )");
  }
  return res;
}
int print_ds_type_bare_privacy_rule (struct tl_ds_privacy_rule *DS, struct paramed_type *T) {
  int res;
  if (multiline_output >= 2) { multiline_offset += multiline_offset_size; }
  eprintf (" (");
  switch (DS->magic) {
  case 0xfffe1bac: res = print_ds_constructor_privacy_value_allow_contacts (DS, T); break;
  case 0x65427b82: res = print_ds_constructor_privacy_value_allow_all (DS, T); break;
  case 0x4d5bbe0c: res = print_ds_constructor_privacy_value_allow_users (DS, T); break;
  case 0xf888fa1a: res = print_ds_constructor_privacy_value_disallow_contacts (DS, T); break;
  case 0x8b73e763: res = print_ds_constructor_privacy_value_disallow_all (DS, T); break;
  case 0x0c7f49b7: res = print_ds_constructor_privacy_value_disallow_users (DS, T); break;
  default: return -1;
  }
  if (res >= 0) {
    if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); }
    eprintf (" )");
  }
  return res;
}
int print_ds_type_received_notify_message (struct tl_ds_received_notify_message *DS, struct paramed_type *T) {
  int res;
  if (multiline_output >= 2) { multiline_offset += multiline_offset_size; }
  eprintf (" (");
  res = print_ds_constructor_received_notify_message (DS, T);
  if (res >= 0) {
    if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); }
    eprintf (" )");
  }
  return res;
}
int print_ds_type_bare_received_notify_message (struct tl_ds_received_notify_message *DS, struct paramed_type *T) {
  int res;
  if (multiline_output >= 2) { multiline_offset += multiline_offset_size; }
  eprintf (" (");
  res = print_ds_constructor_received_notify_message (DS, T);
  if (res >= 0) {
    if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); }
    eprintf (" )");
  }
  return res;
}
int print_ds_type_reply_markup (struct tl_ds_reply_markup *DS, struct paramed_type *T) {
  int res;
  if (multiline_output >= 2) { multiline_offset += multiline_offset_size; }
  eprintf (" (");
  switch (DS->magic) {
  case 0xa03e5b85: res = print_ds_constructor_reply_keyboard_hide (DS, T); break;
  case 0xf4108aa0: res = print_ds_constructor_reply_keyboard_force_reply (DS, T); break;
  case 0x3502758c: res = print_ds_constructor_reply_keyboard_markup (DS, T); break;
  default: return -1;
  }
  if (res >= 0) {
    if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); }
    eprintf (" )");
  }
  return res;
}
int print_ds_type_bare_reply_markup (struct tl_ds_reply_markup *DS, struct paramed_type *T) {
  int res;
  if (multiline_output >= 2) { multiline_offset += multiline_offset_size; }
  eprintf (" (");
  switch (DS->magic) {
  case 0xa03e5b85: res = print_ds_constructor_reply_keyboard_hide (DS, T); break;
  case 0xf4108aa0: res = print_ds_constructor_reply_keyboard_force_reply (DS, T); break;
  case 0x3502758c: res = print_ds_constructor_reply_keyboard_markup (DS, T); break;
  default: return -1;
  }
  if (res >= 0) {
    if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); }
    eprintf (" )");
  }
  return res;
}
int print_ds_type_res_p_q (struct tl_ds_res_p_q *DS, struct paramed_type *T) {
  int res;
  if (multiline_output >= 2) { multiline_offset += multiline_offset_size; }
  eprintf (" (");
  res = print_ds_constructor_res_p_q (DS, T);
  if (res >= 0) {
    if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); }
    eprintf (" )");
  }
  return res;
}
int print_ds_type_bare_res_p_q (struct tl_ds_res_p_q *DS, struct paramed_type *T) {
  int res;
  if (multiline_output >= 2) { multiline_offset += multiline_offset_size; }
  eprintf (" (");
  res = print_ds_constructor_res_p_q (DS, T);
  if (res >= 0) {
    if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); }
    eprintf (" )");
  }
  return res;
}
int print_ds_type_send_message_action (struct tl_ds_send_message_action *DS, struct paramed_type *T) {
  int res;
  if (multiline_output >= 2) { multiline_offset += multiline_offset_size; }
  eprintf (" (");
  switch (DS->magic) {
  case 0x16bf744e: res = print_ds_constructor_send_message_typing_action (DS, T); break;
  case 0xfd5ec8f5: res = print_ds_constructor_send_message_cancel_action (DS, T); break;
  case 0xa187d66f: res = print_ds_constructor_send_message_record_video_action (DS, T); break;
  case 0x92042ff7: res = print_ds_constructor_send_message_upload_video_action_l27 (DS, T); break;
  case 0xe9763aec: res = print_ds_constructor_send_message_upload_video_action (DS, T); break;
  case 0xd52f73f7: res = print_ds_constructor_send_message_record_audio_action (DS, T); break;
  case 0xe6ac8a6f: res = print_ds_constructor_send_message_upload_audio_action_l27 (DS, T); break;
  case 0xf351d7ab: res = print_ds_constructor_send_message_upload_audio_action (DS, T); break;
  case 0xd1d34a26: res = print_ds_constructor_send_message_upload_photo_action (DS, T); break;
  case 0x8faee98e: res = print_ds_constructor_send_message_upload_document_action_l27 (DS, T); break;
  case 0xaa0cd9e4: res = print_ds_constructor_send_message_upload_document_action (DS, T); break;
  case 0x176f8ba1: res = print_ds_constructor_send_message_geo_location_action (DS, T); break;
  case 0x628cbc6f: res = print_ds_constructor_send_message_choose_contact_action (DS, T); break;
  default: return -1;
  }
  if (res >= 0) {
    if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); }
    eprintf (" )");
  }
  return res;
}
int print_ds_type_bare_send_message_action (struct tl_ds_send_message_action *DS, struct paramed_type *T) {
  int res;
  if (multiline_output >= 2) { multiline_offset += multiline_offset_size; }
  eprintf (" (");
  switch (DS->magic) {
  case 0x16bf744e: res = print_ds_constructor_send_message_typing_action (DS, T); break;
  case 0xfd5ec8f5: res = print_ds_constructor_send_message_cancel_action (DS, T); break;
  case 0xa187d66f: res = print_ds_constructor_send_message_record_video_action (DS, T); break;
  case 0x92042ff7: res = print_ds_constructor_send_message_upload_video_action_l27 (DS, T); break;
  case 0xe9763aec: res = print_ds_constructor_send_message_upload_video_action (DS, T); break;
  case 0xd52f73f7: res = print_ds_constructor_send_message_record_audio_action (DS, T); break;
  case 0xe6ac8a6f: res = print_ds_constructor_send_message_upload_audio_action_l27 (DS, T); break;
  case 0xf351d7ab: res = print_ds_constructor_send_message_upload_audio_action (DS, T); break;
  case 0xd1d34a26: res = print_ds_constructor_send_message_upload_photo_action (DS, T); break;
  case 0x8faee98e: res = print_ds_constructor_send_message_upload_document_action_l27 (DS, T); break;
  case 0xaa0cd9e4: res = print_ds_constructor_send_message_upload_document_action (DS, T); break;
  case 0x176f8ba1: res = print_ds_constructor_send_message_geo_location_action (DS, T); break;
  case 0x628cbc6f: res = print_ds_constructor_send_message_choose_contact_action (DS, T); break;
  default: return -1;
  }
  if (res >= 0) {
    if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); }
    eprintf (" )");
  }
  return res;
}
int print_ds_type_server_d_h_params (struct tl_ds_server_d_h_params *DS, struct paramed_type *T) {
  int res;
  if (multiline_output >= 2) { multiline_offset += multiline_offset_size; }
  eprintf (" (");
  switch (DS->magic) {
  case 0x79cb045d: res = print_ds_constructor_server_d_h_params_fail (DS, T); break;
  case 0xd0e8075c: res = print_ds_constructor_server_d_h_params_ok (DS, T); break;
  default: return -1;
  }
  if (res >= 0) {
    if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); }
    eprintf (" )");
  }
  return res;
}
int print_ds_type_bare_server_d_h_params (struct tl_ds_server_d_h_params *DS, struct paramed_type *T) {
  int res;
  if (multiline_output >= 2) { multiline_offset += multiline_offset_size; }
  eprintf (" (");
  switch (DS->magic) {
  case 0x79cb045d: res = print_ds_constructor_server_d_h_params_fail (DS, T); break;
  case 0xd0e8075c: res = print_ds_constructor_server_d_h_params_ok (DS, T); break;
  default: return -1;
  }
  if (res >= 0) {
    if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); }
    eprintf (" )");
  }
  return res;
}
int print_ds_type_server_d_h_inner_data (struct tl_ds_server_d_h_inner_data *DS, struct paramed_type *T) {
  int res;
  if (multiline_output >= 2) { multiline_offset += multiline_offset_size; }
  eprintf (" (");
  res = print_ds_constructor_server_d_h_inner_data (DS, T);
  if (res >= 0) {
    if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); }
    eprintf (" )");
  }
  return res;
}
int print_ds_type_bare_server_d_h_inner_data (struct tl_ds_server_d_h_inner_data *DS, struct paramed_type *T) {
  int res;
  if (multiline_output >= 2) { multiline_offset += multiline_offset_size; }
  eprintf (" (");
  res = print_ds_constructor_server_d_h_inner_data (DS, T);
  if (res >= 0) {
    if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); }
    eprintf (" )");
  }
  return res;
}
int print_ds_type_set_client_d_h_params_answer (struct tl_ds_set_client_d_h_params_answer *DS, struct paramed_type *T) {
  int res;
  if (multiline_output >= 2) { multiline_offset += multiline_offset_size; }
  eprintf (" (");
  switch (DS->magic) {
  case 0x3bcbf734: res = print_ds_constructor_dh_gen_ok (DS, T); break;
  case 0x46dc1fb9: res = print_ds_constructor_dh_gen_retry (DS, T); break;
  case 0xa69dae02: res = print_ds_constructor_dh_gen_fail (DS, T); break;
  default: return -1;
  }
  if (res >= 0) {
    if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); }
    eprintf (" )");
  }
  return res;
}
int print_ds_type_bare_set_client_d_h_params_answer (struct tl_ds_set_client_d_h_params_answer *DS, struct paramed_type *T) {
  int res;
  if (multiline_output >= 2) { multiline_offset += multiline_offset_size; }
  eprintf (" (");
  switch (DS->magic) {
  case 0x3bcbf734: res = print_ds_constructor_dh_gen_ok (DS, T); break;
  case 0x46dc1fb9: res = print_ds_constructor_dh_gen_retry (DS, T); break;
  case 0xa69dae02: res = print_ds_constructor_dh_gen_fail (DS, T); break;
  default: return -1;
  }
  if (res >= 0) {
    if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); }
    eprintf (" )");
  }
  return res;
}
int print_ds_type_sticker_pack (struct tl_ds_sticker_pack *DS, struct paramed_type *T) {
  int res;
  if (multiline_output >= 2) { multiline_offset += multiline_offset_size; }
  eprintf (" (");
  res = print_ds_constructor_sticker_pack (DS, T);
  if (res >= 0) {
    if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); }
    eprintf (" )");
  }
  return res;
}
int print_ds_type_bare_sticker_pack (struct tl_ds_sticker_pack *DS, struct paramed_type *T) {
  int res;
  if (multiline_output >= 2) { multiline_offset += multiline_offset_size; }
  eprintf (" (");
  res = print_ds_constructor_sticker_pack (DS, T);
  if (res >= 0) {
    if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); }
    eprintf (" )");
  }
  return res;
}
int print_ds_type_sticker_set (struct tl_ds_sticker_set *DS, struct paramed_type *T) {
  int res;
  if (multiline_output >= 2) { multiline_offset += multiline_offset_size; }
  eprintf (" (");
  res = print_ds_constructor_sticker_set (DS, T);
  if (res >= 0) {
    if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); }
    eprintf (" )");
  }
  return res;
}
int print_ds_type_bare_sticker_set (struct tl_ds_sticker_set *DS, struct paramed_type *T) {
  int res;
  if (multiline_output >= 2) { multiline_offset += multiline_offset_size; }
  eprintf (" (");
  res = print_ds_constructor_sticker_set (DS, T);
  if (res >= 0) {
    if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); }
    eprintf (" )");
  }
  return res;
}
int print_ds_type_string (struct tl_ds_string *DS, struct paramed_type *T) {
  int res;
  res = print_ds_constructor_string (DS, T);
  return res;
}
int print_ds_type_bare_string (struct tl_ds_string *DS, struct paramed_type *T) {
  int res;
  res = print_ds_constructor_string (DS, T);
  return res;
}
int print_ds_type_update (struct tl_ds_update *DS, struct paramed_type *T) {
  int res;
  if (multiline_output >= 2) { multiline_offset += multiline_offset_size; }
  eprintf (" (");
  switch (DS->magic) {
  case 0x1f2b0afd: res = print_ds_constructor_update_new_message (DS, T); break;
  case 0x4e90bfd6: res = print_ds_constructor_update_message_i_d (DS, T); break;
  case 0xa20db0e5: res = print_ds_constructor_update_delete_messages (DS, T); break;
  case 0x5c486927: res = print_ds_constructor_update_user_typing (DS, T); break;
  case 0x9a65ea1f: res = print_ds_constructor_update_chat_user_typing (DS, T); break;
  case 0x07761198: res = print_ds_constructor_update_chat_participants (DS, T); break;
  case 0x1bfbd823: res = print_ds_constructor_update_user_status (DS, T); break;
  case 0xa7332b73: res = print_ds_constructor_update_user_name (DS, T); break;
  case 0x95313b0c: res = print_ds_constructor_update_user_photo (DS, T); break;
  case 0x2575bbb9: res = print_ds_constructor_update_contact_registered (DS, T); break;
  case 0x9d2e67c5: res = print_ds_constructor_update_contact_link (DS, T); break;
  case 0x8f06529a: res = print_ds_constructor_update_new_authorization (DS, T); break;
  case 0x5a68e3f7: res = print_ds_constructor_update_new_geo_chat_message (DS, T); break;
  case 0x12bcbd9a: res = print_ds_constructor_update_new_encrypted_message (DS, T); break;
  case 0x1710f156: res = print_ds_constructor_update_encrypted_chat_typing (DS, T); break;
  case 0xb4a2e88d: res = print_ds_constructor_update_encryption (DS, T); break;
  case 0x38fe25b7: res = print_ds_constructor_update_encrypted_messages_read (DS, T); break;
  case 0x3a0eeb22: res = print_ds_constructor_update_chat_participant_add (DS, T); break;
  case 0x6e5f8c22: res = print_ds_constructor_update_chat_participant_delete (DS, T); break;
  case 0x8e5e9873: res = print_ds_constructor_update_dc_options (DS, T); break;
  case 0x80ece81a: res = print_ds_constructor_update_user_blocked (DS, T); break;
  case 0xbec268ef: res = print_ds_constructor_update_notify_settings (DS, T); break;
  case 0x382dd3e4: res = print_ds_constructor_update_service_notification (DS, T); break;
  case 0xee3b272a: res = print_ds_constructor_update_privacy (DS, T); break;
  case 0x12b9417b: res = print_ds_constructor_update_user_phone (DS, T); break;
  case 0x9961fd5c: res = print_ds_constructor_update_read_history_inbox (DS, T); break;
  case 0x2f2f21bf: res = print_ds_constructor_update_read_history_outbox (DS, T); break;
  case 0x2cc36971: res = print_ds_constructor_update_web_page (DS, T); break;
  case 0x68c13933: res = print_ds_constructor_update_read_messages_contents (DS, T); break;
  case 0x03114739: res = print_ds_constructor_update_msg_update (DS, T); break;
  default: return -1;
  }
  if (res >= 0) {
    if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); }
    eprintf (" )");
  }
  return res;
}
int print_ds_type_bare_update (struct tl_ds_update *DS, struct paramed_type *T) {
  int res;
  if (multiline_output >= 2) { multiline_offset += multiline_offset_size; }
  eprintf (" (");
  switch (DS->magic) {
  case 0x1f2b0afd: res = print_ds_constructor_update_new_message (DS, T); break;
  case 0x4e90bfd6: res = print_ds_constructor_update_message_i_d (DS, T); break;
  case 0xa20db0e5: res = print_ds_constructor_update_delete_messages (DS, T); break;
  case 0x5c486927: res = print_ds_constructor_update_user_typing (DS, T); break;
  case 0x9a65ea1f: res = print_ds_constructor_update_chat_user_typing (DS, T); break;
  case 0x07761198: res = print_ds_constructor_update_chat_participants (DS, T); break;
  case 0x1bfbd823: res = print_ds_constructor_update_user_status (DS, T); break;
  case 0xa7332b73: res = print_ds_constructor_update_user_name (DS, T); break;
  case 0x95313b0c: res = print_ds_constructor_update_user_photo (DS, T); break;
  case 0x2575bbb9: res = print_ds_constructor_update_contact_registered (DS, T); break;
  case 0x9d2e67c5: res = print_ds_constructor_update_contact_link (DS, T); break;
  case 0x8f06529a: res = print_ds_constructor_update_new_authorization (DS, T); break;
  case 0x5a68e3f7: res = print_ds_constructor_update_new_geo_chat_message (DS, T); break;
  case 0x12bcbd9a: res = print_ds_constructor_update_new_encrypted_message (DS, T); break;
  case 0x1710f156: res = print_ds_constructor_update_encrypted_chat_typing (DS, T); break;
  case 0xb4a2e88d: res = print_ds_constructor_update_encryption (DS, T); break;
  case 0x38fe25b7: res = print_ds_constructor_update_encrypted_messages_read (DS, T); break;
  case 0x3a0eeb22: res = print_ds_constructor_update_chat_participant_add (DS, T); break;
  case 0x6e5f8c22: res = print_ds_constructor_update_chat_participant_delete (DS, T); break;
  case 0x8e5e9873: res = print_ds_constructor_update_dc_options (DS, T); break;
  case 0x80ece81a: res = print_ds_constructor_update_user_blocked (DS, T); break;
  case 0xbec268ef: res = print_ds_constructor_update_notify_settings (DS, T); break;
  case 0x382dd3e4: res = print_ds_constructor_update_service_notification (DS, T); break;
  case 0xee3b272a: res = print_ds_constructor_update_privacy (DS, T); break;
  case 0x12b9417b: res = print_ds_constructor_update_user_phone (DS, T); break;
  case 0x9961fd5c: res = print_ds_constructor_update_read_history_inbox (DS, T); break;
  case 0x2f2f21bf: res = print_ds_constructor_update_read_history_outbox (DS, T); break;
  case 0x2cc36971: res = print_ds_constructor_update_web_page (DS, T); break;
  case 0x68c13933: res = print_ds_constructor_update_read_messages_contents (DS, T); break;
  case 0x03114739: res = print_ds_constructor_update_msg_update (DS, T); break;
  default: return -1;
  }
  if (res >= 0) {
    if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); }
    eprintf (" )");
  }
  return res;
}
int print_ds_type_updates (struct tl_ds_updates *DS, struct paramed_type *T) {
  int res;
  if (multiline_output >= 2) { multiline_offset += multiline_offset_size; }
  eprintf (" (");
  switch (DS->magic) {
  case 0xe317af7e: res = print_ds_constructor_updates_too_long (DS, T); break;
  case 0xed5c2127: res = print_ds_constructor_update_short_message (DS, T); break;
  case 0x52238b3c: res = print_ds_constructor_update_short_chat_message (DS, T); break;
  case 0x78d4dec1: res = print_ds_constructor_update_short (DS, T); break;
  case 0x725b04c3: res = print_ds_constructor_updates_combined (DS, T); break;
  case 0x74ae4240: res = print_ds_constructor_updates (DS, T); break;
  default: return -1;
  }
  if (res >= 0) {
    if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); }
    eprintf (" )");
  }
  return res;
}
int print_ds_type_bare_updates (struct tl_ds_updates *DS, struct paramed_type *T) {
  int res;
  if (multiline_output >= 2) { multiline_offset += multiline_offset_size; }
  eprintf (" (");
  switch (DS->magic) {
  case 0xe317af7e: res = print_ds_constructor_updates_too_long (DS, T); break;
  case 0xed5c2127: res = print_ds_constructor_update_short_message (DS, T); break;
  case 0x52238b3c: res = print_ds_constructor_update_short_chat_message (DS, T); break;
  case 0x78d4dec1: res = print_ds_constructor_update_short (DS, T); break;
  case 0x725b04c3: res = print_ds_constructor_updates_combined (DS, T); break;
  case 0x74ae4240: res = print_ds_constructor_updates (DS, T); break;
  default: return -1;
  }
  if (res >= 0) {
    if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); }
    eprintf (" )");
  }
  return res;
}
int print_ds_type_user (struct tl_ds_user *DS, struct paramed_type *T) {
  int res;
  if (multiline_output >= 2) { multiline_offset += multiline_offset_size; }
  eprintf (" (");
  switch (DS->magic) {
  case 0x200250ba: res = print_ds_constructor_user_empty (DS, T); break;
  case 0x22e49072: res = print_ds_constructor_user (DS, T); break;
  default: return -1;
  }
  if (res >= 0) {
    if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); }
    eprintf (" )");
  }
  return res;
}
int print_ds_type_bare_user (struct tl_ds_user *DS, struct paramed_type *T) {
  int res;
  if (multiline_output >= 2) { multiline_offset += multiline_offset_size; }
  eprintf (" (");
  switch (DS->magic) {
  case 0x200250ba: res = print_ds_constructor_user_empty (DS, T); break;
  case 0x22e49072: res = print_ds_constructor_user (DS, T); break;
  default: return -1;
  }
  if (res >= 0) {
    if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); }
    eprintf (" )");
  }
  return res;
}
int print_ds_type_user_full (struct tl_ds_user_full *DS, struct paramed_type *T) {
  int res;
  if (multiline_output >= 2) { multiline_offset += multiline_offset_size; }
  eprintf (" (");
  res = print_ds_constructor_user_full (DS, T);
  if (res >= 0) {
    if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); }
    eprintf (" )");
  }
  return res;
}
int print_ds_type_bare_user_full (struct tl_ds_user_full *DS, struct paramed_type *T) {
  int res;
  if (multiline_output >= 2) { multiline_offset += multiline_offset_size; }
  eprintf (" (");
  res = print_ds_constructor_user_full (DS, T);
  if (res >= 0) {
    if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); }
    eprintf (" )");
  }
  return res;
}
int print_ds_type_user_profile_photo (struct tl_ds_user_profile_photo *DS, struct paramed_type *T) {
  int res;
  if (multiline_output >= 2) { multiline_offset += multiline_offset_size; }
  eprintf (" (");
  switch (DS->magic) {
  case 0x4f11bae1: res = print_ds_constructor_user_profile_photo_empty (DS, T); break;
  case 0xd559d8c8: res = print_ds_constructor_user_profile_photo (DS, T); break;
  default: return -1;
  }
  if (res >= 0) {
    if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); }
    eprintf (" )");
  }
  return res;
}
int print_ds_type_bare_user_profile_photo (struct tl_ds_user_profile_photo *DS, struct paramed_type *T) {
  int res;
  if (multiline_output >= 2) { multiline_offset += multiline_offset_size; }
  eprintf (" (");
  switch (DS->magic) {
  case 0x4f11bae1: res = print_ds_constructor_user_profile_photo_empty (DS, T); break;
  case 0xd559d8c8: res = print_ds_constructor_user_profile_photo (DS, T); break;
  default: return -1;
  }
  if (res >= 0) {
    if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); }
    eprintf (" )");
  }
  return res;
}
int print_ds_type_user_status (struct tl_ds_user_status *DS, struct paramed_type *T) {
  int res;
  if (multiline_output >= 2) { multiline_offset += multiline_offset_size; }
  eprintf (" (");
  switch (DS->magic) {
  case 0x09d05049: res = print_ds_constructor_user_status_empty (DS, T); break;
  case 0xedb93949: res = print_ds_constructor_user_status_online (DS, T); break;
  case 0x008c703f: res = print_ds_constructor_user_status_offline (DS, T); break;
  case 0xe26f42f1: res = print_ds_constructor_user_status_recently (DS, T); break;
  case 0x07bf09fc: res = print_ds_constructor_user_status_last_week (DS, T); break;
  case 0x77ebc742: res = print_ds_constructor_user_status_last_month (DS, T); break;
  default: return -1;
  }
  if (res >= 0) {
    if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); }
    eprintf (" )");
  }
  return res;
}
int print_ds_type_bare_user_status (struct tl_ds_user_status *DS, struct paramed_type *T) {
  int res;
  if (multiline_output >= 2) { multiline_offset += multiline_offset_size; }
  eprintf (" (");
  switch (DS->magic) {
  case 0x09d05049: res = print_ds_constructor_user_status_empty (DS, T); break;
  case 0xedb93949: res = print_ds_constructor_user_status_online (DS, T); break;
  case 0x008c703f: res = print_ds_constructor_user_status_offline (DS, T); break;
  case 0xe26f42f1: res = print_ds_constructor_user_status_recently (DS, T); break;
  case 0x07bf09fc: res = print_ds_constructor_user_status_last_week (DS, T); break;
  case 0x77ebc742: res = print_ds_constructor_user_status_last_month (DS, T); break;
  default: return -1;
  }
  if (res >= 0) {
    if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); }
    eprintf (" )");
  }
  return res;
}
int print_ds_type_vector (struct tl_ds_vector *DS, struct paramed_type *T) {
  int res;
  if (multiline_output >= 2) { multiline_offset += multiline_offset_size; }
  eprintf (" (");
  res = print_ds_constructor_vector (DS, T);
  if (res >= 0) {
    if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); }
    eprintf (" )");
  }
  return res;
}
int print_ds_type_bare_vector (struct tl_ds_vector *DS, struct paramed_type *T) {
  int res;
  if (multiline_output >= 2) { multiline_offset += multiline_offset_size; }
  eprintf (" (");
  res = print_ds_constructor_vector (DS, T);
  if (res >= 0) {
    if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); }
    eprintf (" )");
  }
  return res;
}
int print_ds_type_video (struct tl_ds_video *DS, struct paramed_type *T) {
  int res;
  if (multiline_output >= 2) { multiline_offset += multiline_offset_size; }
  eprintf (" (");
  switch (DS->magic) {
  case 0xc10658a8: res = print_ds_constructor_video_empty (DS, T); break;
  case 0xee9f4a4d: res = print_ds_constructor_video (DS, T); break;
  case 0x388fa391: res = print_ds_constructor_video_l27 (DS, T); break;
  default: return -1;
  }
  if (res >= 0) {
    if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); }
    eprintf (" )");
  }
  return res;
}
int print_ds_type_bare_video (struct tl_ds_video *DS, struct paramed_type *T) {
  int res;
  if (multiline_output >= 2) { multiline_offset += multiline_offset_size; }
  eprintf (" (");
  switch (DS->magic) {
  case 0xc10658a8: res = print_ds_constructor_video_empty (DS, T); break;
  case 0xee9f4a4d: res = print_ds_constructor_video (DS, T); break;
  case 0x388fa391: res = print_ds_constructor_video_l27 (DS, T); break;
  default: return -1;
  }
  if (res >= 0) {
    if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); }
    eprintf (" )");
  }
  return res;
}
int print_ds_type_wall_paper (struct tl_ds_wall_paper *DS, struct paramed_type *T) {
  int res;
  if (multiline_output >= 2) { multiline_offset += multiline_offset_size; }
  eprintf (" (");
  switch (DS->magic) {
  case 0xccb03657: res = print_ds_constructor_wall_paper (DS, T); break;
  case 0x63117f24: res = print_ds_constructor_wall_paper_solid (DS, T); break;
  default: return -1;
  }
  if (res >= 0) {
    if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); }
    eprintf (" )");
  }
  return res;
}
int print_ds_type_bare_wall_paper (struct tl_ds_wall_paper *DS, struct paramed_type *T) {
  int res;
  if (multiline_output >= 2) { multiline_offset += multiline_offset_size; }
  eprintf (" (");
  switch (DS->magic) {
  case 0xccb03657: res = print_ds_constructor_wall_paper (DS, T); break;
  case 0x63117f24: res = print_ds_constructor_wall_paper_solid (DS, T); break;
  default: return -1;
  }
  if (res >= 0) {
    if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); }
    eprintf (" )");
  }
  return res;
}
int print_ds_type_web_page (struct tl_ds_web_page *DS, struct paramed_type *T) {
  int res;
  if (multiline_output >= 2) { multiline_offset += multiline_offset_size; }
  eprintf (" (");
  switch (DS->magic) {
  case 0xeb1477e8: res = print_ds_constructor_web_page_empty (DS, T); break;
  case 0xc586da1c: res = print_ds_constructor_web_page_pending (DS, T); break;
  case 0xa31ea0b5: res = print_ds_constructor_web_page (DS, T); break;
  default: return -1;
  }
  if (res >= 0) {
    if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); }
    eprintf (" )");
  }
  return res;
}
int print_ds_type_bare_web_page (struct tl_ds_web_page *DS, struct paramed_type *T) {
  int res;
  if (multiline_output >= 2) { multiline_offset += multiline_offset_size; }
  eprintf (" (");
  switch (DS->magic) {
  case 0xeb1477e8: res = print_ds_constructor_web_page_empty (DS, T); break;
  case 0xc586da1c: res = print_ds_constructor_web_page_pending (DS, T); break;
  case 0xa31ea0b5: res = print_ds_constructor_web_page (DS, T); break;
  default: return -1;
  }
  if (res >= 0) {
    if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); }
    eprintf (" )");
  }
  return res;
}
int print_ds_type_account_authorizations (struct tl_ds_account_authorizations *DS, struct paramed_type *T) {
  int res;
  res = print_ds_constructor_account_authorizations (DS, T);
  return res;
}
int print_ds_type_bare_account_authorizations (struct tl_ds_account_authorizations *DS, struct paramed_type *T) {
  int res;
  res = print_ds_constructor_account_authorizations (DS, T);
  return res;
}
int print_ds_type_account_password (struct tl_ds_account_password *DS, struct paramed_type *T) {
  int res;
  if (multiline_output >= 2) { multiline_offset += multiline_offset_size; }
  eprintf (" (");
  switch (DS->magic) {
  case 0x96dabc18: res = print_ds_constructor_account_no_password (DS, T); break;
  case 0x7c18141c: res = print_ds_constructor_account_password (DS, T); break;
  default: return -1;
  }
  if (res >= 0) {
    if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); }
    eprintf (" )");
  }
  return res;
}
int print_ds_type_bare_account_password (struct tl_ds_account_password *DS, struct paramed_type *T) {
  int res;
  if (multiline_output >= 2) { multiline_offset += multiline_offset_size; }
  eprintf (" (");
  switch (DS->magic) {
  case 0x96dabc18: res = print_ds_constructor_account_no_password (DS, T); break;
  case 0x7c18141c: res = print_ds_constructor_account_password (DS, T); break;
  default: return -1;
  }
  if (res >= 0) {
    if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); }
    eprintf (" )");
  }
  return res;
}
int print_ds_type_account_password_input_settings (struct tl_ds_account_password_input_settings *DS, struct paramed_type *T) {
  int res;
  if (multiline_output >= 2) { multiline_offset += multiline_offset_size; }
  eprintf (" (");
  res = print_ds_constructor_account_password_input_settings (DS, T);
  if (res >= 0) {
    if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); }
    eprintf (" )");
  }
  return res;
}
int print_ds_type_bare_account_password_input_settings (struct tl_ds_account_password_input_settings *DS, struct paramed_type *T) {
  int res;
  if (multiline_output >= 2) { multiline_offset += multiline_offset_size; }
  eprintf (" (");
  res = print_ds_constructor_account_password_input_settings (DS, T);
  if (res >= 0) {
    if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); }
    eprintf (" )");
  }
  return res;
}
int print_ds_type_account_password_settings (struct tl_ds_account_password_settings *DS, struct paramed_type *T) {
  int res;
  res = print_ds_constructor_account_password_settings (DS, T);
  return res;
}
int print_ds_type_bare_account_password_settings (struct tl_ds_account_password_settings *DS, struct paramed_type *T) {
  int res;
  res = print_ds_constructor_account_password_settings (DS, T);
  return res;
}
int print_ds_type_account_privacy_rules (struct tl_ds_account_privacy_rules *DS, struct paramed_type *T) {
  int res;
  if (multiline_output >= 2) { multiline_offset += multiline_offset_size; }
  eprintf (" (");
  res = print_ds_constructor_account_privacy_rules (DS, T);
  if (res >= 0) {
    if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); }
    eprintf (" )");
  }
  return res;
}
int print_ds_type_bare_account_privacy_rules (struct tl_ds_account_privacy_rules *DS, struct paramed_type *T) {
  int res;
  if (multiline_output >= 2) { multiline_offset += multiline_offset_size; }
  eprintf (" (");
  res = print_ds_constructor_account_privacy_rules (DS, T);
  if (res >= 0) {
    if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); }
    eprintf (" )");
  }
  return res;
}
int print_ds_type_account_sent_change_phone_code (struct tl_ds_account_sent_change_phone_code *DS, struct paramed_type *T) {
  int res;
  if (multiline_output >= 2) { multiline_offset += multiline_offset_size; }
  eprintf (" (");
  res = print_ds_constructor_account_sent_change_phone_code (DS, T);
  if (res >= 0) {
    if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); }
    eprintf (" )");
  }
  return res;
}
int print_ds_type_bare_account_sent_change_phone_code (struct tl_ds_account_sent_change_phone_code *DS, struct paramed_type *T) {
  int res;
  if (multiline_output >= 2) { multiline_offset += multiline_offset_size; }
  eprintf (" (");
  res = print_ds_constructor_account_sent_change_phone_code (DS, T);
  if (res >= 0) {
    if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); }
    eprintf (" )");
  }
  return res;
}
int print_ds_type_auth_authorization (struct tl_ds_auth_authorization *DS, struct paramed_type *T) {
  int res;
  res = print_ds_constructor_auth_authorization (DS, T);
  return res;
}
int print_ds_type_bare_auth_authorization (struct tl_ds_auth_authorization *DS, struct paramed_type *T) {
  int res;
  res = print_ds_constructor_auth_authorization (DS, T);
  return res;
}
int print_ds_type_auth_checked_phone (struct tl_ds_auth_checked_phone *DS, struct paramed_type *T) {
  int res;
  res = print_ds_constructor_auth_checked_phone (DS, T);
  return res;
}
int print_ds_type_bare_auth_checked_phone (struct tl_ds_auth_checked_phone *DS, struct paramed_type *T) {
  int res;
  res = print_ds_constructor_auth_checked_phone (DS, T);
  return res;
}
int print_ds_type_auth_exported_authorization (struct tl_ds_auth_exported_authorization *DS, struct paramed_type *T) {
  int res;
  if (multiline_output >= 2) { multiline_offset += multiline_offset_size; }
  eprintf (" (");
  res = print_ds_constructor_auth_exported_authorization (DS, T);
  if (res >= 0) {
    if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); }
    eprintf (" )");
  }
  return res;
}
int print_ds_type_bare_auth_exported_authorization (struct tl_ds_auth_exported_authorization *DS, struct paramed_type *T) {
  int res;
  if (multiline_output >= 2) { multiline_offset += multiline_offset_size; }
  eprintf (" (");
  res = print_ds_constructor_auth_exported_authorization (DS, T);
  if (res >= 0) {
    if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); }
    eprintf (" )");
  }
  return res;
}
int print_ds_type_auth_password_recovery (struct tl_ds_auth_password_recovery *DS, struct paramed_type *T) {
  int res;
  res = print_ds_constructor_auth_password_recovery (DS, T);
  return res;
}
int print_ds_type_bare_auth_password_recovery (struct tl_ds_auth_password_recovery *DS, struct paramed_type *T) {
  int res;
  res = print_ds_constructor_auth_password_recovery (DS, T);
  return res;
}
int print_ds_type_auth_sent_code (struct tl_ds_auth_sent_code *DS, struct paramed_type *T) {
  int res;
  if (multiline_output >= 2) { multiline_offset += multiline_offset_size; }
  eprintf (" (");
  switch (DS->magic) {
  case 0xefed51d9: res = print_ds_constructor_auth_sent_code (DS, T); break;
  case 0xe325edcf: res = print_ds_constructor_auth_sent_app_code (DS, T); break;
  default: return -1;
  }
  if (res >= 0) {
    if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); }
    eprintf (" )");
  }
  return res;
}
int print_ds_type_bare_auth_sent_code (struct tl_ds_auth_sent_code *DS, struct paramed_type *T) {
  int res;
  if (multiline_output >= 2) { multiline_offset += multiline_offset_size; }
  eprintf (" (");
  switch (DS->magic) {
  case 0xefed51d9: res = print_ds_constructor_auth_sent_code (DS, T); break;
  case 0xe325edcf: res = print_ds_constructor_auth_sent_app_code (DS, T); break;
  default: return -1;
  }
  if (res >= 0) {
    if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); }
    eprintf (" )");
  }
  return res;
}
int print_ds_type_binlog_encr_key (struct tl_ds_binlog_encr_key *DS, struct paramed_type *T) {
  int res;
  res = print_ds_constructor_binlog_encr_key (DS, T);
  return res;
}
int print_ds_type_bare_binlog_encr_key (struct tl_ds_binlog_encr_key *DS, struct paramed_type *T) {
  int res;
  res = print_ds_constructor_binlog_encr_key (DS, T);
  return res;
}
int print_ds_type_binlog_update (struct tl_ds_binlog_update *DS, struct paramed_type *T) {
  int res;
  if (multiline_output >= 2) { multiline_offset += multiline_offset_size; }
  eprintf (" (");
  switch (DS->magic) {
  case 0x3b06de69: res = print_ds_constructor_binlog_start (DS, T); break;
  case 0xf96feb32: res = print_ds_constructor_binlog_dc_option (DS, T); break;
  case 0x7c0d22d8: res = print_ds_constructor_binlog_dc_option_new (DS, T); break;
  case 0x71e8c156: res = print_ds_constructor_binlog_auth_key (DS, T); break;
  case 0x9e83dbdc: res = print_ds_constructor_binlog_default_dc (DS, T); break;
  case 0x26451bb5: res = print_ds_constructor_binlog_dc_signed (DS, T); break;
  case 0x68a870e8: res = print_ds_constructor_binlog_our_id (DS, T); break;
  case 0xeaeb7826: res = print_ds_constructor_binlog_set_dh_params (DS, T); break;
  case 0x2ca8c939: res = print_ds_constructor_binlog_set_pts (DS, T); break;
  case 0xd95738ac: res = print_ds_constructor_binlog_set_qts (DS, T); break;
  case 0x1d0f4b52: res = print_ds_constructor_binlog_set_date (DS, T); break;
  case 0x6eeb2989: res = print_ds_constructor_binlog_set_seq (DS, T); break;
  case 0xee1b38e8: res = print_ds_constructor_binlog_encr_chat_delete (DS, T); break;
  case 0x84977251: res = print_ds_constructor_binlog_encr_chat_new (DS, T); break;
  case 0x9d49488d: res = print_ds_constructor_binlog_encr_chat_exchange_new (DS, T); break;
  case 0xac55d447: res = print_ds_constructor_binlog_user_delete (DS, T); break;
  case 0x127cf2f9: res = print_ds_constructor_binlog_user_new (DS, T); break;
  case 0x0a10aa92: res = print_ds_constructor_binlog_chat_new (DS, T); break;
  case 0x535475ea: res = print_ds_constructor_binlog_chat_add_participant (DS, T); break;
  case 0x7dd1a1a2: res = print_ds_constructor_binlog_chat_del_participant (DS, T); break;
  case 0x3c873416: res = print_ds_constructor_binlog_set_msg_id (DS, T); break;
  case 0x847e77b1: res = print_ds_constructor_binlog_message_delete (DS, T); break;
  case 0x427cfcdb: res = print_ds_constructor_binlog_message_new (DS, T); break;
  case 0x6cf7cabc: res = print_ds_constructor_binlog_message_encr_new (DS, T); break;
  case 0x6dd4d85f: res = print_ds_constructor_binlog_msg_update (DS, T); break;
  case 0x83327955: res = print_ds_constructor_binlog_reset_authorization (DS, T); break;
  default: return -1;
  }
  if (res >= 0) {
    if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); }
    eprintf (" )");
  }
  return res;
}
int print_ds_type_bare_binlog_update (struct tl_ds_binlog_update *DS, struct paramed_type *T) {
  int res;
  if (multiline_output >= 2) { multiline_offset += multiline_offset_size; }
  eprintf (" (");
  switch (DS->magic) {
  case 0x3b06de69: res = print_ds_constructor_binlog_start (DS, T); break;
  case 0xf96feb32: res = print_ds_constructor_binlog_dc_option (DS, T); break;
  case 0x7c0d22d8: res = print_ds_constructor_binlog_dc_option_new (DS, T); break;
  case 0x71e8c156: res = print_ds_constructor_binlog_auth_key (DS, T); break;
  case 0x9e83dbdc: res = print_ds_constructor_binlog_default_dc (DS, T); break;
  case 0x26451bb5: res = print_ds_constructor_binlog_dc_signed (DS, T); break;
  case 0x68a870e8: res = print_ds_constructor_binlog_our_id (DS, T); break;
  case 0xeaeb7826: res = print_ds_constructor_binlog_set_dh_params (DS, T); break;
  case 0x2ca8c939: res = print_ds_constructor_binlog_set_pts (DS, T); break;
  case 0xd95738ac: res = print_ds_constructor_binlog_set_qts (DS, T); break;
  case 0x1d0f4b52: res = print_ds_constructor_binlog_set_date (DS, T); break;
  case 0x6eeb2989: res = print_ds_constructor_binlog_set_seq (DS, T); break;
  case 0xee1b38e8: res = print_ds_constructor_binlog_encr_chat_delete (DS, T); break;
  case 0x84977251: res = print_ds_constructor_binlog_encr_chat_new (DS, T); break;
  case 0x9d49488d: res = print_ds_constructor_binlog_encr_chat_exchange_new (DS, T); break;
  case 0xac55d447: res = print_ds_constructor_binlog_user_delete (DS, T); break;
  case 0x127cf2f9: res = print_ds_constructor_binlog_user_new (DS, T); break;
  case 0x0a10aa92: res = print_ds_constructor_binlog_chat_new (DS, T); break;
  case 0x535475ea: res = print_ds_constructor_binlog_chat_add_participant (DS, T); break;
  case 0x7dd1a1a2: res = print_ds_constructor_binlog_chat_del_participant (DS, T); break;
  case 0x3c873416: res = print_ds_constructor_binlog_set_msg_id (DS, T); break;
  case 0x847e77b1: res = print_ds_constructor_binlog_message_delete (DS, T); break;
  case 0x427cfcdb: res = print_ds_constructor_binlog_message_new (DS, T); break;
  case 0x6cf7cabc: res = print_ds_constructor_binlog_message_encr_new (DS, T); break;
  case 0x6dd4d85f: res = print_ds_constructor_binlog_msg_update (DS, T); break;
  case 0x83327955: res = print_ds_constructor_binlog_reset_authorization (DS, T); break;
  default: return -1;
  }
  if (res >= 0) {
    if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); }
    eprintf (" )");
  }
  return res;
}
int print_ds_type_contacts_blocked (struct tl_ds_contacts_blocked *DS, struct paramed_type *T) {
  int res;
  if (multiline_output >= 2) { multiline_offset += multiline_offset_size; }
  eprintf (" (");
  switch (DS->magic) {
  case 0x1c138d15: res = print_ds_constructor_contacts_blocked (DS, T); break;
  case 0x900802a1: res = print_ds_constructor_contacts_blocked_slice (DS, T); break;
  default: return -1;
  }
  if (res >= 0) {
    if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); }
    eprintf (" )");
  }
  return res;
}
int print_ds_type_bare_contacts_blocked (struct tl_ds_contacts_blocked *DS, struct paramed_type *T) {
  int res;
  if (multiline_output >= 2) { multiline_offset += multiline_offset_size; }
  eprintf (" (");
  switch (DS->magic) {
  case 0x1c138d15: res = print_ds_constructor_contacts_blocked (DS, T); break;
  case 0x900802a1: res = print_ds_constructor_contacts_blocked_slice (DS, T); break;
  default: return -1;
  }
  if (res >= 0) {
    if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); }
    eprintf (" )");
  }
  return res;
}
int print_ds_type_contacts_contacts (struct tl_ds_contacts_contacts *DS, struct paramed_type *T) {
  int res;
  if (multiline_output >= 2) { multiline_offset += multiline_offset_size; }
  eprintf (" (");
  switch (DS->magic) {
  case 0xb74ba9d2: res = print_ds_constructor_contacts_contacts_not_modified (DS, T); break;
  case 0x6f8b8cb2: res = print_ds_constructor_contacts_contacts (DS, T); break;
  default: return -1;
  }
  if (res >= 0) {
    if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); }
    eprintf (" )");
  }
  return res;
}
int print_ds_type_bare_contacts_contacts (struct tl_ds_contacts_contacts *DS, struct paramed_type *T) {
  int res;
  if (multiline_output >= 2) { multiline_offset += multiline_offset_size; }
  eprintf (" (");
  switch (DS->magic) {
  case 0xb74ba9d2: res = print_ds_constructor_contacts_contacts_not_modified (DS, T); break;
  case 0x6f8b8cb2: res = print_ds_constructor_contacts_contacts (DS, T); break;
  default: return -1;
  }
  if (res >= 0) {
    if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); }
    eprintf (" )");
  }
  return res;
}
int print_ds_type_contacts_found (struct tl_ds_contacts_found *DS, struct paramed_type *T) {
  int res;
  if (multiline_output >= 2) { multiline_offset += multiline_offset_size; }
  eprintf (" (");
  res = print_ds_constructor_contacts_found (DS, T);
  if (res >= 0) {
    if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); }
    eprintf (" )");
  }
  return res;
}
int print_ds_type_bare_contacts_found (struct tl_ds_contacts_found *DS, struct paramed_type *T) {
  int res;
  if (multiline_output >= 2) { multiline_offset += multiline_offset_size; }
  eprintf (" (");
  res = print_ds_constructor_contacts_found (DS, T);
  if (res >= 0) {
    if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); }
    eprintf (" )");
  }
  return res;
}
int print_ds_type_contacts_imported_contacts (struct tl_ds_contacts_imported_contacts *DS, struct paramed_type *T) {
  int res;
  if (multiline_output >= 2) { multiline_offset += multiline_offset_size; }
  eprintf (" (");
  res = print_ds_constructor_contacts_imported_contacts (DS, T);
  if (res >= 0) {
    if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); }
    eprintf (" )");
  }
  return res;
}
int print_ds_type_bare_contacts_imported_contacts (struct tl_ds_contacts_imported_contacts *DS, struct paramed_type *T) {
  int res;
  if (multiline_output >= 2) { multiline_offset += multiline_offset_size; }
  eprintf (" (");
  res = print_ds_constructor_contacts_imported_contacts (DS, T);
  if (res >= 0) {
    if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); }
    eprintf (" )");
  }
  return res;
}
int print_ds_type_contacts_link (struct tl_ds_contacts_link *DS, struct paramed_type *T) {
  int res;
  if (multiline_output >= 2) { multiline_offset += multiline_offset_size; }
  eprintf (" (");
  res = print_ds_constructor_contacts_link (DS, T);
  if (res >= 0) {
    if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); }
    eprintf (" )");
  }
  return res;
}
int print_ds_type_bare_contacts_link (struct tl_ds_contacts_link *DS, struct paramed_type *T) {
  int res;
  if (multiline_output >= 2) { multiline_offset += multiline_offset_size; }
  eprintf (" (");
  res = print_ds_constructor_contacts_link (DS, T);
  if (res >= 0) {
    if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); }
    eprintf (" )");
  }
  return res;
}
int print_ds_type_contacts_suggested (struct tl_ds_contacts_suggested *DS, struct paramed_type *T) {
  int res;
  if (multiline_output >= 2) { multiline_offset += multiline_offset_size; }
  eprintf (" (");
  res = print_ds_constructor_contacts_suggested (DS, T);
  if (res >= 0) {
    if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); }
    eprintf (" )");
  }
  return res;
}
int print_ds_type_bare_contacts_suggested (struct tl_ds_contacts_suggested *DS, struct paramed_type *T) {
  int res;
  if (multiline_output >= 2) { multiline_offset += multiline_offset_size; }
  eprintf (" (");
  res = print_ds_constructor_contacts_suggested (DS, T);
  if (res >= 0) {
    if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); }
    eprintf (" )");
  }
  return res;
}
int print_ds_type_geochats_located (struct tl_ds_geochats_located *DS, struct paramed_type *T) {
  int res;
  if (multiline_output >= 2) { multiline_offset += multiline_offset_size; }
  eprintf (" (");
  res = print_ds_constructor_geochats_located (DS, T);
  if (res >= 0) {
    if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); }
    eprintf (" )");
  }
  return res;
}
int print_ds_type_bare_geochats_located (struct tl_ds_geochats_located *DS, struct paramed_type *T) {
  int res;
  if (multiline_output >= 2) { multiline_offset += multiline_offset_size; }
  eprintf (" (");
  res = print_ds_constructor_geochats_located (DS, T);
  if (res >= 0) {
    if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); }
    eprintf (" )");
  }
  return res;
}
int print_ds_type_geochats_messages (struct tl_ds_geochats_messages *DS, struct paramed_type *T) {
  int res;
  if (multiline_output >= 2) { multiline_offset += multiline_offset_size; }
  eprintf (" (");
  switch (DS->magic) {
  case 0xd1526db1: res = print_ds_constructor_geochats_messages (DS, T); break;
  case 0xbc5863e8: res = print_ds_constructor_geochats_messages_slice (DS, T); break;
  default: return -1;
  }
  if (res >= 0) {
    if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); }
    eprintf (" )");
  }
  return res;
}
int print_ds_type_bare_geochats_messages (struct tl_ds_geochats_messages *DS, struct paramed_type *T) {
  int res;
  if (multiline_output >= 2) { multiline_offset += multiline_offset_size; }
  eprintf (" (");
  switch (DS->magic) {
  case 0xd1526db1: res = print_ds_constructor_geochats_messages (DS, T); break;
  case 0xbc5863e8: res = print_ds_constructor_geochats_messages_slice (DS, T); break;
  default: return -1;
  }
  if (res >= 0) {
    if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); }
    eprintf (" )");
  }
  return res;
}
int print_ds_type_geochats_stated_message (struct tl_ds_geochats_stated_message *DS, struct paramed_type *T) {
  int res;
  if (multiline_output >= 2) { multiline_offset += multiline_offset_size; }
  eprintf (" (");
  res = print_ds_constructor_geochats_stated_message (DS, T);
  if (res >= 0) {
    if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); }
    eprintf (" )");
  }
  return res;
}
int print_ds_type_bare_geochats_stated_message (struct tl_ds_geochats_stated_message *DS, struct paramed_type *T) {
  int res;
  if (multiline_output >= 2) { multiline_offset += multiline_offset_size; }
  eprintf (" (");
  res = print_ds_constructor_geochats_stated_message (DS, T);
  if (res >= 0) {
    if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); }
    eprintf (" )");
  }
  return res;
}
int print_ds_type_help_app_update (struct tl_ds_help_app_update *DS, struct paramed_type *T) {
  int res;
  if (multiline_output >= 2) { multiline_offset += multiline_offset_size; }
  eprintf (" (");
  switch (DS->magic) {
  case 0x8987f311: res = print_ds_constructor_help_app_update (DS, T); break;
  case 0xc45a6536: res = print_ds_constructor_help_no_app_update (DS, T); break;
  default: return -1;
  }
  if (res >= 0) {
    if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); }
    eprintf (" )");
  }
  return res;
}
int print_ds_type_bare_help_app_update (struct tl_ds_help_app_update *DS, struct paramed_type *T) {
  int res;
  if (multiline_output >= 2) { multiline_offset += multiline_offset_size; }
  eprintf (" (");
  switch (DS->magic) {
  case 0x8987f311: res = print_ds_constructor_help_app_update (DS, T); break;
  case 0xc45a6536: res = print_ds_constructor_help_no_app_update (DS, T); break;
  default: return -1;
  }
  if (res >= 0) {
    if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); }
    eprintf (" )");
  }
  return res;
}
int print_ds_type_help_invite_text (struct tl_ds_help_invite_text *DS, struct paramed_type *T) {
  int res;
  res = print_ds_constructor_help_invite_text (DS, T);
  return res;
}
int print_ds_type_bare_help_invite_text (struct tl_ds_help_invite_text *DS, struct paramed_type *T) {
  int res;
  res = print_ds_constructor_help_invite_text (DS, T);
  return res;
}
int print_ds_type_help_support (struct tl_ds_help_support *DS, struct paramed_type *T) {
  int res;
  if (multiline_output >= 2) { multiline_offset += multiline_offset_size; }
  eprintf (" (");
  res = print_ds_constructor_help_support (DS, T);
  if (res >= 0) {
    if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); }
    eprintf (" )");
  }
  return res;
}
int print_ds_type_bare_help_support (struct tl_ds_help_support *DS, struct paramed_type *T) {
  int res;
  if (multiline_output >= 2) { multiline_offset += multiline_offset_size; }
  eprintf (" (");
  res = print_ds_constructor_help_support (DS, T);
  if (res >= 0) {
    if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); }
    eprintf (" )");
  }
  return res;
}
int print_ds_type_messages_affected_history (struct tl_ds_messages_affected_history *DS, struct paramed_type *T) {
  int res;
  if (multiline_output >= 2) { multiline_offset += multiline_offset_size; }
  eprintf (" (");
  res = print_ds_constructor_messages_affected_history (DS, T);
  if (res >= 0) {
    if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); }
    eprintf (" )");
  }
  return res;
}
int print_ds_type_bare_messages_affected_history (struct tl_ds_messages_affected_history *DS, struct paramed_type *T) {
  int res;
  if (multiline_output >= 2) { multiline_offset += multiline_offset_size; }
  eprintf (" (");
  res = print_ds_constructor_messages_affected_history (DS, T);
  if (res >= 0) {
    if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); }
    eprintf (" )");
  }
  return res;
}
int print_ds_type_messages_affected_messages (struct tl_ds_messages_affected_messages *DS, struct paramed_type *T) {
  int res;
  if (multiline_output >= 2) { multiline_offset += multiline_offset_size; }
  eprintf (" (");
  res = print_ds_constructor_messages_affected_messages (DS, T);
  if (res >= 0) {
    if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); }
    eprintf (" )");
  }
  return res;
}
int print_ds_type_bare_messages_affected_messages (struct tl_ds_messages_affected_messages *DS, struct paramed_type *T) {
  int res;
  if (multiline_output >= 2) { multiline_offset += multiline_offset_size; }
  eprintf (" (");
  res = print_ds_constructor_messages_affected_messages (DS, T);
  if (res >= 0) {
    if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); }
    eprintf (" )");
  }
  return res;
}
int print_ds_type_messages_all_stickers (struct tl_ds_messages_all_stickers *DS, struct paramed_type *T) {
  int res;
  if (multiline_output >= 2) { multiline_offset += multiline_offset_size; }
  eprintf (" (");
  switch (DS->magic) {
  case 0xe86602c3: res = print_ds_constructor_messages_all_stickers_not_modified (DS, T); break;
  case 0x5ce352ec: res = print_ds_constructor_messages_all_stickers (DS, T); break;
  default: return -1;
  }
  if (res >= 0) {
    if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); }
    eprintf (" )");
  }
  return res;
}
int print_ds_type_bare_messages_all_stickers (struct tl_ds_messages_all_stickers *DS, struct paramed_type *T) {
  int res;
  if (multiline_output >= 2) { multiline_offset += multiline_offset_size; }
  eprintf (" (");
  switch (DS->magic) {
  case 0xe86602c3: res = print_ds_constructor_messages_all_stickers_not_modified (DS, T); break;
  case 0x5ce352ec: res = print_ds_constructor_messages_all_stickers (DS, T); break;
  default: return -1;
  }
  if (res >= 0) {
    if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); }
    eprintf (" )");
  }
  return res;
}
int print_ds_type_messages_chat_full (struct tl_ds_messages_chat_full *DS, struct paramed_type *T) {
  int res;
  if (multiline_output >= 2) { multiline_offset += multiline_offset_size; }
  eprintf (" (");
  res = print_ds_constructor_messages_chat_full (DS, T);
  if (res >= 0) {
    if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); }
    eprintf (" )");
  }
  return res;
}
int print_ds_type_bare_messages_chat_full (struct tl_ds_messages_chat_full *DS, struct paramed_type *T) {
  int res;
  if (multiline_output >= 2) { multiline_offset += multiline_offset_size; }
  eprintf (" (");
  res = print_ds_constructor_messages_chat_full (DS, T);
  if (res >= 0) {
    if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); }
    eprintf (" )");
  }
  return res;
}
int print_ds_type_messages_chats (struct tl_ds_messages_chats *DS, struct paramed_type *T) {
  int res;
  res = print_ds_constructor_messages_chats (DS, T);
  return res;
}
int print_ds_type_bare_messages_chats (struct tl_ds_messages_chats *DS, struct paramed_type *T) {
  int res;
  res = print_ds_constructor_messages_chats (DS, T);
  return res;
}
int print_ds_type_messages_dh_config (struct tl_ds_messages_dh_config *DS, struct paramed_type *T) {
  int res;
  if (multiline_output >= 2) { multiline_offset += multiline_offset_size; }
  eprintf (" (");
  switch (DS->magic) {
  case 0xc0e24635: res = print_ds_constructor_messages_dh_config_not_modified (DS, T); break;
  case 0x2c221edd: res = print_ds_constructor_messages_dh_config (DS, T); break;
  default: return -1;
  }
  if (res >= 0) {
    if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); }
    eprintf (" )");
  }
  return res;
}
int print_ds_type_bare_messages_dh_config (struct tl_ds_messages_dh_config *DS, struct paramed_type *T) {
  int res;
  if (multiline_output >= 2) { multiline_offset += multiline_offset_size; }
  eprintf (" (");
  switch (DS->magic) {
  case 0xc0e24635: res = print_ds_constructor_messages_dh_config_not_modified (DS, T); break;
  case 0x2c221edd: res = print_ds_constructor_messages_dh_config (DS, T); break;
  default: return -1;
  }
  if (res >= 0) {
    if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); }
    eprintf (" )");
  }
  return res;
}
int print_ds_type_messages_dialogs (struct tl_ds_messages_dialogs *DS, struct paramed_type *T) {
  int res;
  if (multiline_output >= 2) { multiline_offset += multiline_offset_size; }
  eprintf (" (");
  switch (DS->magic) {
  case 0x15ba6c40: res = print_ds_constructor_messages_dialogs (DS, T); break;
  case 0x71e094f3: res = print_ds_constructor_messages_dialogs_slice (DS, T); break;
  default: return -1;
  }
  if (res >= 0) {
    if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); }
    eprintf (" )");
  }
  return res;
}
int print_ds_type_bare_messages_dialogs (struct tl_ds_messages_dialogs *DS, struct paramed_type *T) {
  int res;
  if (multiline_output >= 2) { multiline_offset += multiline_offset_size; }
  eprintf (" (");
  switch (DS->magic) {
  case 0x15ba6c40: res = print_ds_constructor_messages_dialogs (DS, T); break;
  case 0x71e094f3: res = print_ds_constructor_messages_dialogs_slice (DS, T); break;
  default: return -1;
  }
  if (res >= 0) {
    if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); }
    eprintf (" )");
  }
  return res;
}
int print_ds_type_messages_message (struct tl_ds_messages_message *DS, struct paramed_type *T) {
  int res;
  if (multiline_output >= 2) { multiline_offset += multiline_offset_size; }
  eprintf (" (");
  res = print_ds_constructor_messages_message_empty (DS, T);
  if (res >= 0) {
    if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); }
    eprintf (" )");
  }
  return res;
}
int print_ds_type_bare_messages_message (struct tl_ds_messages_message *DS, struct paramed_type *T) {
  int res;
  if (multiline_output >= 2) { multiline_offset += multiline_offset_size; }
  eprintf (" (");
  res = print_ds_constructor_messages_message_empty (DS, T);
  if (res >= 0) {
    if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); }
    eprintf (" )");
  }
  return res;
}
int print_ds_type_messages_messages (struct tl_ds_messages_messages *DS, struct paramed_type *T) {
  int res;
  if (multiline_output >= 2) { multiline_offset += multiline_offset_size; }
  eprintf (" (");
  switch (DS->magic) {
  case 0x8c718e87: res = print_ds_constructor_messages_messages (DS, T); break;
  case 0x0b446ae3: res = print_ds_constructor_messages_messages_slice (DS, T); break;
  default: return -1;
  }
  if (res >= 0) {
    if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); }
    eprintf (" )");
  }
  return res;
}
int print_ds_type_bare_messages_messages (struct tl_ds_messages_messages *DS, struct paramed_type *T) {
  int res;
  if (multiline_output >= 2) { multiline_offset += multiline_offset_size; }
  eprintf (" (");
  switch (DS->magic) {
  case 0x8c718e87: res = print_ds_constructor_messages_messages (DS, T); break;
  case 0x0b446ae3: res = print_ds_constructor_messages_messages_slice (DS, T); break;
  default: return -1;
  }
  if (res >= 0) {
    if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); }
    eprintf (" )");
  }
  return res;
}
int print_ds_type_messages_sent_encrypted_message (struct tl_ds_messages_sent_encrypted_message *DS, struct paramed_type *T) {
  int res;
  if (multiline_output >= 2) { multiline_offset += multiline_offset_size; }
  eprintf (" (");
  switch (DS->magic) {
  case 0x560f8935: res = print_ds_constructor_messages_sent_encrypted_message (DS, T); break;
  case 0x9493ff32: res = print_ds_constructor_messages_sent_encrypted_file (DS, T); break;
  default: return -1;
  }
  if (res >= 0) {
    if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); }
    eprintf (" )");
  }
  return res;
}
int print_ds_type_bare_messages_sent_encrypted_message (struct tl_ds_messages_sent_encrypted_message *DS, struct paramed_type *T) {
  int res;
  if (multiline_output >= 2) { multiline_offset += multiline_offset_size; }
  eprintf (" (");
  switch (DS->magic) {
  case 0x560f8935: res = print_ds_constructor_messages_sent_encrypted_message (DS, T); break;
  case 0x9493ff32: res = print_ds_constructor_messages_sent_encrypted_file (DS, T); break;
  default: return -1;
  }
  if (res >= 0) {
    if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); }
    eprintf (" )");
  }
  return res;
}
int print_ds_type_messages_sent_message (struct tl_ds_messages_sent_message *DS, struct paramed_type *T) {
  int res;
  if (multiline_output >= 2) { multiline_offset += multiline_offset_size; }
  eprintf (" (");
  switch (DS->magic) {
  case 0x4c3d47f3: res = print_ds_constructor_messages_sent_message (DS, T); break;
  case 0x35a1a663: res = print_ds_constructor_messages_sent_message_link (DS, T); break;
  default: return -1;
  }
  if (res >= 0) {
    if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); }
    eprintf (" )");
  }
  return res;
}
int print_ds_type_bare_messages_sent_message (struct tl_ds_messages_sent_message *DS, struct paramed_type *T) {
  int res;
  if (multiline_output >= 2) { multiline_offset += multiline_offset_size; }
  eprintf (" (");
  switch (DS->magic) {
  case 0x4c3d47f3: res = print_ds_constructor_messages_sent_message (DS, T); break;
  case 0x35a1a663: res = print_ds_constructor_messages_sent_message_link (DS, T); break;
  default: return -1;
  }
  if (res >= 0) {
    if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); }
    eprintf (" )");
  }
  return res;
}
int print_ds_type_messages_sticker_set (struct tl_ds_messages_sticker_set *DS, struct paramed_type *T) {
  int res;
  if (multiline_output >= 2) { multiline_offset += multiline_offset_size; }
  eprintf (" (");
  res = print_ds_constructor_messages_sticker_set (DS, T);
  if (res >= 0) {
    if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); }
    eprintf (" )");
  }
  return res;
}
int print_ds_type_bare_messages_sticker_set (struct tl_ds_messages_sticker_set *DS, struct paramed_type *T) {
  int res;
  if (multiline_output >= 2) { multiline_offset += multiline_offset_size; }
  eprintf (" (");
  res = print_ds_constructor_messages_sticker_set (DS, T);
  if (res >= 0) {
    if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); }
    eprintf (" )");
  }
  return res;
}
int print_ds_type_messages_stickers (struct tl_ds_messages_stickers *DS, struct paramed_type *T) {
  int res;
  if (multiline_output >= 2) { multiline_offset += multiline_offset_size; }
  eprintf (" (");
  switch (DS->magic) {
  case 0xf1749a22: res = print_ds_constructor_messages_stickers_not_modified (DS, T); break;
  case 0x8a8ecd32: res = print_ds_constructor_messages_stickers (DS, T); break;
  default: return -1;
  }
  if (res >= 0) {
    if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); }
    eprintf (" )");
  }
  return res;
}
int print_ds_type_bare_messages_stickers (struct tl_ds_messages_stickers *DS, struct paramed_type *T) {
  int res;
  if (multiline_output >= 2) { multiline_offset += multiline_offset_size; }
  eprintf (" (");
  switch (DS->magic) {
  case 0xf1749a22: res = print_ds_constructor_messages_stickers_not_modified (DS, T); break;
  case 0x8a8ecd32: res = print_ds_constructor_messages_stickers (DS, T); break;
  default: return -1;
  }
  if (res >= 0) {
    if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); }
    eprintf (" )");
  }
  return res;
}
int print_ds_type_photos_photo (struct tl_ds_photos_photo *DS, struct paramed_type *T) {
  int res;
  if (multiline_output >= 2) { multiline_offset += multiline_offset_size; }
  eprintf (" (");
  res = print_ds_constructor_photos_photo (DS, T);
  if (res >= 0) {
    if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); }
    eprintf (" )");
  }
  return res;
}
int print_ds_type_bare_photos_photo (struct tl_ds_photos_photo *DS, struct paramed_type *T) {
  int res;
  if (multiline_output >= 2) { multiline_offset += multiline_offset_size; }
  eprintf (" (");
  res = print_ds_constructor_photos_photo (DS, T);
  if (res >= 0) {
    if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); }
    eprintf (" )");
  }
  return res;
}
int print_ds_type_photos_photos (struct tl_ds_photos_photos *DS, struct paramed_type *T) {
  int res;
  if (multiline_output >= 2) { multiline_offset += multiline_offset_size; }
  eprintf (" (");
  switch (DS->magic) {
  case 0x8dca6aa5: res = print_ds_constructor_photos_photos (DS, T); break;
  case 0x15051f54: res = print_ds_constructor_photos_photos_slice (DS, T); break;
  default: return -1;
  }
  if (res >= 0) {
    if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); }
    eprintf (" )");
  }
  return res;
}
int print_ds_type_bare_photos_photos (struct tl_ds_photos_photos *DS, struct paramed_type *T) {
  int res;
  if (multiline_output >= 2) { multiline_offset += multiline_offset_size; }
  eprintf (" (");
  switch (DS->magic) {
  case 0x8dca6aa5: res = print_ds_constructor_photos_photos (DS, T); break;
  case 0x15051f54: res = print_ds_constructor_photos_photos_slice (DS, T); break;
  default: return -1;
  }
  if (res >= 0) {
    if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); }
    eprintf (" )");
  }
  return res;
}
int print_ds_type_storage_file_type (struct tl_ds_storage_file_type *DS, struct paramed_type *T) {
  int res;
  if (multiline_output >= 2) { multiline_offset += multiline_offset_size; }
  eprintf (" (");
  switch (DS->magic) {
  case 0xaa963b05: res = print_ds_constructor_storage_file_unknown (DS, T); break;
  case 0x007efe0e: res = print_ds_constructor_storage_file_jpeg (DS, T); break;
  case 0xcae1aadf: res = print_ds_constructor_storage_file_gif (DS, T); break;
  case 0x0a4f63c0: res = print_ds_constructor_storage_file_png (DS, T); break;
  case 0xae1e508d: res = print_ds_constructor_storage_file_pdf (DS, T); break;
  case 0x528a0677: res = print_ds_constructor_storage_file_mp3 (DS, T); break;
  case 0x4b09ebbc: res = print_ds_constructor_storage_file_mov (DS, T); break;
  case 0x40bc6f52: res = print_ds_constructor_storage_file_partial (DS, T); break;
  case 0xb3cea0e4: res = print_ds_constructor_storage_file_mp4 (DS, T); break;
  case 0x1081464c: res = print_ds_constructor_storage_file_webp (DS, T); break;
  default: return -1;
  }
  if (res >= 0) {
    if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); }
    eprintf (" )");
  }
  return res;
}
int print_ds_type_bare_storage_file_type (struct tl_ds_storage_file_type *DS, struct paramed_type *T) {
  int res;
  if (multiline_output >= 2) { multiline_offset += multiline_offset_size; }
  eprintf (" (");
  switch (DS->magic) {
  case 0xaa963b05: res = print_ds_constructor_storage_file_unknown (DS, T); break;
  case 0x007efe0e: res = print_ds_constructor_storage_file_jpeg (DS, T); break;
  case 0xcae1aadf: res = print_ds_constructor_storage_file_gif (DS, T); break;
  case 0x0a4f63c0: res = print_ds_constructor_storage_file_png (DS, T); break;
  case 0xae1e508d: res = print_ds_constructor_storage_file_pdf (DS, T); break;
  case 0x528a0677: res = print_ds_constructor_storage_file_mp3 (DS, T); break;
  case 0x4b09ebbc: res = print_ds_constructor_storage_file_mov (DS, T); break;
  case 0x40bc6f52: res = print_ds_constructor_storage_file_partial (DS, T); break;
  case 0xb3cea0e4: res = print_ds_constructor_storage_file_mp4 (DS, T); break;
  case 0x1081464c: res = print_ds_constructor_storage_file_webp (DS, T); break;
  default: return -1;
  }
  if (res >= 0) {
    if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); }
    eprintf (" )");
  }
  return res;
}
int print_ds_type_updates_difference (struct tl_ds_updates_difference *DS, struct paramed_type *T) {
  int res;
  if (multiline_output >= 2) { multiline_offset += multiline_offset_size; }
  eprintf (" (");
  switch (DS->magic) {
  case 0x5d75a138: res = print_ds_constructor_updates_difference_empty (DS, T); break;
  case 0x00f49ca0: res = print_ds_constructor_updates_difference (DS, T); break;
  case 0xa8fb1981: res = print_ds_constructor_updates_difference_slice (DS, T); break;
  default: return -1;
  }
  if (res >= 0) {
    if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); }
    eprintf (" )");
  }
  return res;
}
int print_ds_type_bare_updates_difference (struct tl_ds_updates_difference *DS, struct paramed_type *T) {
  int res;
  if (multiline_output >= 2) { multiline_offset += multiline_offset_size; }
  eprintf (" (");
  switch (DS->magic) {
  case 0x5d75a138: res = print_ds_constructor_updates_difference_empty (DS, T); break;
  case 0x00f49ca0: res = print_ds_constructor_updates_difference (DS, T); break;
  case 0xa8fb1981: res = print_ds_constructor_updates_difference_slice (DS, T); break;
  default: return -1;
  }
  if (res >= 0) {
    if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); }
    eprintf (" )");
  }
  return res;
}
int print_ds_type_updates_state (struct tl_ds_updates_state *DS, struct paramed_type *T) {
  int res;
  if (multiline_output >= 2) { multiline_offset += multiline_offset_size; }
  eprintf (" (");
  res = print_ds_constructor_updates_state (DS, T);
  if (res >= 0) {
    if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); }
    eprintf (" )");
  }
  return res;
}
int print_ds_type_bare_updates_state (struct tl_ds_updates_state *DS, struct paramed_type *T) {
  int res;
  if (multiline_output >= 2) { multiline_offset += multiline_offset_size; }
  eprintf (" (");
  res = print_ds_constructor_updates_state (DS, T);
  if (res >= 0) {
    if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); }
    eprintf (" )");
  }
  return res;
}
int print_ds_type_upload_file (struct tl_ds_upload_file *DS, struct paramed_type *T) {
  int res;
  if (multiline_output >= 2) { multiline_offset += multiline_offset_size; }
  eprintf (" (");
  res = print_ds_constructor_upload_file (DS, T);
  if (res >= 0) {
    if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); }
    eprintf (" )");
  }
  return res;
}
int print_ds_type_bare_upload_file (struct tl_ds_upload_file *DS, struct paramed_type *T) {
  int res;
  if (multiline_output >= 2) { multiline_offset += multiline_offset_size; }
  eprintf (" (");
  res = print_ds_constructor_upload_file (DS, T);
  if (res >= 0) {
    if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); }
    eprintf (" )");
  }
  return res;
}
int print_ds_type_any (void *DS, struct paramed_type *T) {
  switch (T->type->name) {
  case 0xb8d0afdf: return print_ds_type_account_days_t_t_l (DS, T);
  case 0x472f5020: return print_ds_type_bare_account_days_t_t_l (DS, T);
  case 0x9fc5ec4e: return print_ds_type_audio (DS, T);
  case 0x603a13b1: return print_ds_type_bare_audio (DS, T);
  case 0x7bf2e6f6: return print_ds_type_authorization (DS, T);
  case 0x840d1909: return print_ds_type_bare_authorization (DS, T);
  case 0x250be282: return print_ds_type_bool (DS, T);
  case 0xdaf41d7d: return print_ds_type_bare_bool (DS, T);
  case 0x75e7ea6c: return print_ds_type_bot_command (DS, T);
  case 0x8a181593: return print_ds_type_bare_bot_command (DS, T);
  case 0xb2e16f93: return print_ds_type_bot_info (DS, T);
  case 0x4d1e906c: return print_ds_type_bare_bot_info (DS, T);
  case 0x0ee1379f: return print_ds_type_bytes (DS, T);
  case 0xf11ec860: return print_ds_type_bare_bytes (DS, T);
  case 0x7bd865dc: return print_ds_type_chat (DS, T);
  case 0x84279a23: return print_ds_type_bare_chat (DS, T);
  case 0x2e02a614: return print_ds_type_chat_full (DS, T);
  case 0xd1fd59eb: return print_ds_type_bare_chat_full (DS, T);
  case 0x94f910b1: return print_ds_type_chat_invite (DS, T);
  case 0x6b06ef4e: return print_ds_type_bare_chat_invite (DS, T);
  case 0x3631cf4c: return print_ds_type_chat_located (DS, T);
  case 0xc9ce30b3: return print_ds_type_bare_chat_located (DS, T);
  case 0xc8d7493e: return print_ds_type_chat_participant (DS, T);
  case 0x3728b6c1: return print_ds_type_bare_chat_participant (DS, T);
  case 0x77930f9f: return print_ds_type_chat_participants (DS, T);
  case 0x886cf060: return print_ds_type_bare_chat_participants (DS, T);
  case 0x56922676: return print_ds_type_chat_photo (DS, T);
  case 0xa96dd989: return print_ds_type_bare_chat_photo (DS, T);
  case 0x6643b654: return print_ds_type_client_d_h_inner_data (DS, T);
  case 0x99bc49ab: return print_ds_type_bare_client_d_h_inner_data (DS, T);
  case 0x4e32b894: return print_ds_type_config (DS, T);
  case 0xb1cd476b: return print_ds_type_bare_config (DS, T);
  case 0xf911c994: return print_ds_type_contact (DS, T);
  case 0x06ee366b: return print_ds_type_bare_contact (DS, T);
  case 0x561bc879: return print_ds_type_contact_blocked (DS, T);
  case 0xa9e43786: return print_ds_type_bare_contact_blocked (DS, T);
  case 0xea879f95: return print_ds_type_contact_found (DS, T);
  case 0x1578606a: return print_ds_type_bare_contact_found (DS, T);
  case 0x522fbc63: return print_ds_type_contact_link (DS, T);
  case 0xadd0439c: return print_ds_type_bare_contact_link (DS, T);
  case 0xd3680c61: return print_ds_type_contact_status (DS, T);
  case 0x2c97f39e: return print_ds_type_bare_contact_status (DS, T);
  case 0x3de191a1: return print_ds_type_contact_suggested (DS, T);
  case 0xc21e6e5e: return print_ds_type_bare_contact_suggested (DS, T);
  case 0x2b1a62f0: return print_ds_type_dc_option (DS, T);
  case 0xd4e59d0f: return print_ds_type_bare_dc_option (DS, T);
  case 0x535b7918: return print_ds_type_decrypted_message (DS, T);
  case 0xaca486e7: return print_ds_type_bare_decrypted_message (DS, T);
  case 0x4e0eefde: return print_ds_type_decrypted_message_action (DS, T);
  case 0xb1f11021: return print_ds_type_bare_decrypted_message_action (DS, T);
  case 0x1be31789: return print_ds_type_decrypted_message_layer (DS, T);
  case 0xe41ce876: return print_ds_type_bare_decrypted_message_layer (DS, T);
  case 0x34e0d674: return print_ds_type_decrypted_message_media (DS, T);
  case 0xcb1f298b: return print_ds_type_bare_decrypted_message_media (DS, T);
  case 0xc1dd804a: return print_ds_type_dialog (DS, T);
  case 0x3e227fb5: return print_ds_type_bare_dialog (DS, T);
  case 0xae636f24: return print_ds_type_disabled_feature (DS, T);
  case 0x519c90db: return print_ds_type_bare_disabled_feature (DS, T);
  case 0x51a73418: return print_ds_type_document (DS, T);
  case 0xae58cbe7: return print_ds_type_bare_document (DS, T);
  case 0x97c637a3: return print_ds_type_document_attribute (DS, T);
  case 0x6839c85c: return print_ds_type_bare_document_attribute (DS, T);
  case 0x2210c154: return print_ds_type_double (DS, T);
  case 0xddef3eab: return print_ds_type_bare_double (DS, T);
  case 0xb1718213: return print_ds_type_encrypted_chat (DS, T);
  case 0x4e8e7dec: return print_ds_type_bare_encrypted_chat (DS, T);
  case 0x886fd032: return print_ds_type_encrypted_file (DS, T);
  case 0x77902fcd: return print_ds_type_bare_encrypted_file (DS, T);
  case 0xce6b8a1e: return print_ds_type_encrypted_message (DS, T);
  case 0x319475e1: return print_ds_type_bare_encrypted_message (DS, T);
  case 0xc4b9f9bb: return print_ds_type_error (DS, T);
  case 0x3b460644: return print_ds_type_bare_error (DS, T);
  case 0x95f132d5: return print_ds_type_exported_chat_invite (DS, T);
  case 0x6a0ecd2a: return print_ds_type_bare_exported_chat_invite (DS, T);
  case 0x2f8ffb30: return print_ds_type_file_location (DS, T);
  case 0xd07004cf: return print_ds_type_bare_file_location (DS, T);
  case 0xf67b4034: return print_ds_type_geo_chat_message (DS, T);
  case 0x0984bfcb: return print_ds_type_bare_geo_chat_message (DS, T);
  case 0x315e0a53: return print_ds_type_geo_point (DS, T);
  case 0xcea1f5ac: return print_ds_type_bare_geo_point (DS, T);
  case 0xd0028438: return print_ds_type_imported_contact (DS, T);
  case 0x2ffd7bc7: return print_ds_type_bare_imported_contact (DS, T);
  case 0x770656a8: return print_ds_type_input_app_event (DS, T);
  case 0x88f9a957: return print_ds_type_bare_input_app_event (DS, T);
  case 0xae8e9c7b: return print_ds_type_input_audio (DS, T);
  case 0x51716384: return print_ds_type_bare_input_audio (DS, T);
  case 0x3a60776d: return print_ds_type_input_chat_photo (DS, T);
  case 0xc59f8892: return print_ds_type_bare_input_chat_photo (DS, T);
  case 0xf392b7f4: return print_ds_type_input_contact (DS, T);
  case 0x0c6d480b: return print_ds_type_bare_input_contact (DS, T);
  case 0x6a8963fc: return print_ds_type_input_document (DS, T);
  case 0x95769c03: return print_ds_type_bare_input_document (DS, T);
  case 0xf141b5e1: return print_ds_type_input_encrypted_chat (DS, T);
  case 0x0ebe4a1e: return print_ds_type_bare_input_encrypted_chat (DS, T);
  case 0x0b5c064f: return print_ds_type_input_encrypted_file (DS, T);
  case 0xf4a3f9b0: return print_ds_type_bare_input_encrypted_file (DS, T);
  case 0x0f60f9ca: return print_ds_type_input_file (DS, T);
  case 0xf09f0635: return print_ds_type_bare_input_file (DS, T);
  case 0xe6daa38b: return print_ds_type_input_file_location (DS, T);
  case 0x19255c74: return print_ds_type_bare_input_file_location (DS, T);
  case 0x74d456fa: return print_ds_type_input_geo_chat (DS, T);
  case 0x8b2ba905: return print_ds_type_bare_input_geo_chat (DS, T);
  case 0x17768f1f: return print_ds_type_input_geo_point (DS, T);
  case 0xe88970e0: return print_ds_type_bare_input_geo_point (DS, T);
  case 0xb32c91dd: return print_ds_type_input_media (DS, T);
  case 0x4cd36e22: return print_ds_type_bare_input_media (DS, T);
  case 0x02b6911b: return print_ds_type_input_notify_peer (DS, T);
  case 0xfd496ee4: return print_ds_type_bare_input_notify_peer (DS, T);
  case 0x9e67268d: return print_ds_type_input_peer (DS, T);
  case 0x6198d972: return print_ds_type_bare_input_peer (DS, T);
  case 0x185a48ac: return print_ds_type_input_peer_notify_events (DS, T);
  case 0xe7a5b753: return print_ds_type_bare_input_peer_notify_events (DS, T);
  case 0x46a2ce98: return print_ds_type_input_peer_notify_settings (DS, T);
  case 0xb95d3167: return print_ds_type_bare_input_peer_notify_settings (DS, T);
  case 0xe74279c9: return print_ds_type_input_photo (DS, T);
  case 0x18bd8636: return print_ds_type_bare_input_photo (DS, T);
  case 0x7477e321: return print_ds_type_input_photo_crop (DS, T);
  case 0x8b881cde: return print_ds_type_bare_input_photo_crop (DS, T);
  case 0x4f96cb18: return print_ds_type_input_privacy_key (DS, T);
  case 0xb06934e7: return print_ds_type_bare_input_privacy_key (DS, T);
  case 0x4b815163: return print_ds_type_input_privacy_rule (DS, T);
  case 0xb47eae9c: return print_ds_type_bare_input_privacy_rule (DS, T);
  case 0xe44d415c: return print_ds_type_input_sticker_set (DS, T);
  case 0x1bb2bea3: return print_ds_type_bare_input_sticker_set (DS, T);
  case 0xadfe0c6a: return print_ds_type_input_user (DS, T);
  case 0x5201f395: return print_ds_type_bare_input_user (DS, T);
  case 0xbb5f7a27: return print_ds_type_input_video (DS, T);
  case 0x44a085d8: return print_ds_type_bare_input_video (DS, T);
  case 0xa8509bda: return print_ds_type_int (DS, T);
  case 0x57af6425: return print_ds_type_bare_int (DS, T);
  case 0x7d36c439: return print_ds_type_int128 (DS, T);
  case 0x82c93bc6: return print_ds_type_bare_int128 (DS, T);
  case 0xf2c798b3: return print_ds_type_int256 (DS, T);
  case 0x0d38674c: return print_ds_type_bare_int256 (DS, T);
  case 0xa2fa4880: return print_ds_type_keyboard_button (DS, T);
  case 0x5d05b77f: return print_ds_type_bare_keyboard_button (DS, T);
  case 0x77608b83: return print_ds_type_keyboard_button_row (DS, T);
  case 0x889f747c: return print_ds_type_bare_keyboard_button_row (DS, T);
  case 0x22076cba: return print_ds_type_long (DS, T);
  case 0xddf89345: return print_ds_type_bare_long (DS, T);
  case 0x5d652a7f: return print_ds_type_message (DS, T);
  case 0xa29ad580: return print_ds_type_bare_message (DS, T);
  case 0x383415fa: return print_ds_type_message_action (DS, T);
  case 0xc7cbea05: return print_ds_type_bare_message_action (DS, T);
  case 0x95f2471a: return print_ds_type_message_media (DS, T);
  case 0x6a0db8e5: return print_ds_type_bare_message_media (DS, T);
  case 0x80895ae0: return print_ds_type_messages_filter (DS, T);
  case 0x7f76a51f: return print_ds_type_bare_messages_filter (DS, T);
  case 0x8e1a1775: return print_ds_type_nearest_dc (DS, T);
  case 0x71e5e88a: return print_ds_type_bare_nearest_dc (DS, T);
  case 0x9fcb8237: return print_ds_type_notify_peer (DS, T);
  case 0x60347dc8: return print_ds_type_bare_notify_peer (DS, T);
  case 0x56730bcc: return print_ds_type_null (DS, T);
  case 0xa98cf433: return print_ds_type_bare_null (DS, T);
  case 0xbfa3de38: return print_ds_type_p_q_inner_data (DS, T);
  case 0x405c21c7: return print_ds_type_bare_p_q_inner_data (DS, T);
  case 0x276159d6: return print_ds_type_peer (DS, T);
  case 0xd89ea629: return print_ds_type_bare_peer (DS, T);
  case 0xc0c8d13b: return print_ds_type_peer_notify_events (DS, T);
  case 0x3f372ec4: return print_ds_type_bare_peer_notify_events (DS, T);
  case 0xfdf894fc: return print_ds_type_peer_notify_settings (DS, T);
  case 0x02076b03: return print_ds_type_bare_peer_notify_settings (DS, T);
  case 0xc207550a: return print_ds_type_photo (DS, T);
  case 0x3df8aaf5: return print_ds_type_bare_photo (DS, T);
  case 0x900f60dd: return print_ds_type_photo_size (DS, T);
  case 0x6ff09f22: return print_ds_type_bare_photo_size (DS, T);
  case 0xbc2eab30: return print_ds_type_privacy_key (DS, T);
  case 0x43d154cf: return print_ds_type_bare_privacy_key (DS, T);
  case 0xa8638aec: return print_ds_type_privacy_rule (DS, T);
  case 0x579c7513: return print_ds_type_bare_privacy_rule (DS, T);
  case 0xa384b779: return print_ds_type_received_notify_message (DS, T);
  case 0x5c7b4886: return print_ds_type_bare_received_notify_message (DS, T);
  case 0x612ca4a9: return print_ds_type_reply_markup (DS, T);
  case 0x9ed35b56: return print_ds_type_bare_reply_markup (DS, T);
  case 0x05162463: return print_ds_type_res_p_q (DS, T);
  case 0xfae9db9c: return print_ds_type_bare_res_p_q (DS, T);
  case 0x70541c7e: return print_ds_type_send_message_action (DS, T);
  case 0x8fabe381: return print_ds_type_bare_send_message_action (DS, T);
  case 0xa9230301: return print_ds_type_server_d_h_params (DS, T);
  case 0x56dcfcfe: return print_ds_type_bare_server_d_h_params (DS, T);
  case 0xb5890dba: return print_ds_type_server_d_h_inner_data (DS, T);
  case 0x4a76f245: return print_ds_type_bare_server_d_h_inner_data (DS, T);
  case 0xdb8a468f: return print_ds_type_set_client_d_h_params_answer (DS, T);
  case 0x2475b970: return print_ds_type_bare_set_client_d_h_params_answer (DS, T);
  case 0x12b299d4: return print_ds_type_sticker_pack (DS, T);
  case 0xed4d662b: return print_ds_type_bare_sticker_pack (DS, T);
  case 0xa7a43b17: return print_ds_type_sticker_set (DS, T);
  case 0x585bc4e8: return print_ds_type_bare_sticker_set (DS, T);
  case 0xb5286e24: return print_ds_type_string (DS, T);
  case 0x4ad791db: return print_ds_type_bare_string (DS, T);
  case 0x2e02b27e: return print_ds_type_update (DS, T);
  case 0xd1fd4d81: return print_ds_type_bare_update (DS, T);
  case 0x22499d27: return print_ds_type_updates (DS, T);
  case 0xddb662d8: return print_ds_type_bare_updates (DS, T);
  case 0x02e6c0c8: return print_ds_type_user (DS, T);
  case 0xfd193f37: return print_ds_type_bare_user (DS, T);
  case 0x5a89ac5b: return print_ds_type_user_full (DS, T);
  case 0xa57653a4: return print_ds_type_bare_user_full (DS, T);
  case 0x9a486229: return print_ds_type_user_profile_photo (DS, T);
  case 0x65b79dd6: return print_ds_type_bare_user_profile_photo (DS, T);
  case 0x76de9570: return print_ds_type_user_status (DS, T);
  case 0x89216a8f: return print_ds_type_bare_user_status (DS, T);
  case 0x1cb5c415: return print_ds_type_vector (DS, T);
  case 0xe34a3bea: return print_ds_type_bare_vector (DS, T);
  case 0x1716b174: return print_ds_type_video (DS, T);
  case 0xe8e94e8b: return print_ds_type_bare_video (DS, T);
  case 0xafa14973: return print_ds_type_wall_paper (DS, T);
  case 0x505eb68c: return print_ds_type_bare_wall_paper (DS, T);
  case 0x8d8c0d41: return print_ds_type_web_page (DS, T);
  case 0x7273f2be: return print_ds_type_bare_web_page (DS, T);
  case 0x1250abde: return print_ds_type_account_authorizations (DS, T);
  case 0xedaf5421: return print_ds_type_bare_account_authorizations (DS, T);
  case 0xeac2a804: return print_ds_type_account_password (DS, T);
  case 0x153d57fb: return print_ds_type_bare_account_password (DS, T);
  case 0xbcfc532c: return print_ds_type_account_password_input_settings (DS, T);
  case 0x4303acd3: return print_ds_type_bare_account_password_input_settings (DS, T);
  case 0xb7b72ab3: return print_ds_type_account_password_settings (DS, T);
  case 0x4848d54c: return print_ds_type_bare_account_password_settings (DS, T);
  case 0x554abb6f: return print_ds_type_account_privacy_rules (DS, T);
  case 0xaab54490: return print_ds_type_bare_account_privacy_rules (DS, T);
  case 0xa4f58c4c: return print_ds_type_account_sent_change_phone_code (DS, T);
  case 0x5b0a73b3: return print_ds_type_bare_account_sent_change_phone_code (DS, T);
  case 0xff036af1: return print_ds_type_auth_authorization (DS, T);
  case 0x00fc950e: return print_ds_type_bare_auth_authorization (DS, T);
  case 0x811ea28e: return print_ds_type_auth_checked_phone (DS, T);
  case 0x7ee15d71: return print_ds_type_bare_auth_checked_phone (DS, T);
  case 0xdf969c2d: return print_ds_type_auth_exported_authorization (DS, T);
  case 0x206963d2: return print_ds_type_bare_auth_exported_authorization (DS, T);
  case 0x137948a5: return print_ds_type_auth_password_recovery (DS, T);
  case 0xec86b75a: return print_ds_type_bare_auth_password_recovery (DS, T);
  case 0x0cc8bc16: return print_ds_type_auth_sent_code (DS, T);
  case 0xf33743e9: return print_ds_type_bare_auth_sent_code (DS, T);
  case 0x0377168f: return print_ds_type_binlog_encr_key (DS, T);
  case 0xfc88e970: return print_ds_type_bare_binlog_encr_key (DS, T);
  case 0x66ff03a6: return print_ds_type_binlog_update (DS, T);
  case 0x9900fc59: return print_ds_type_bare_binlog_update (DS, T);
  case 0x8c1b8fb4: return print_ds_type_contacts_blocked (DS, T);
  case 0x73e4704b: return print_ds_type_bare_contacts_blocked (DS, T);
  case 0xd8c02560: return print_ds_type_contacts_contacts (DS, T);
  case 0x273fda9f: return print_ds_type_bare_contacts_contacts (DS, T);
  case 0x0566000e: return print_ds_type_contacts_found (DS, T);
  case 0xfa99fff1: return print_ds_type_bare_contacts_found (DS, T);
  case 0xad524315: return print_ds_type_contacts_imported_contacts (DS, T);
  case 0x52adbcea: return print_ds_type_bare_contacts_imported_contacts (DS, T);
  case 0x3ace484c: return print_ds_type_contacts_link (DS, T);
  case 0xc531b7b3: return print_ds_type_bare_contacts_link (DS, T);
  case 0x5649dcc5: return print_ds_type_contacts_suggested (DS, T);
  case 0xa9b6233a: return print_ds_type_bare_contacts_suggested (DS, T);
  case 0x48feb267: return print_ds_type_geochats_located (DS, T);
  case 0xb7014d98: return print_ds_type_bare_geochats_located (DS, T);
  case 0x6d0a0e59: return print_ds_type_geochats_messages (DS, T);
  case 0x92f5f1a6: return print_ds_type_bare_geochats_messages (DS, T);
  case 0x17b1578b: return print_ds_type_geochats_stated_message (DS, T);
  case 0xe84ea874: return print_ds_type_bare_geochats_stated_message (DS, T);
  case 0x4ddd9627: return print_ds_type_help_app_update (DS, T);
  case 0xb22269d8: return print_ds_type_bare_help_app_update (DS, T);
  case 0x18cb9f78: return print_ds_type_help_invite_text (DS, T);
  case 0xe7346087: return print_ds_type_bare_help_invite_text (DS, T);
  case 0x17c6b5f6: return print_ds_type_help_support (DS, T);
  case 0xe8394a09: return print_ds_type_bare_help_support (DS, T);
  case 0xb45c69d1: return print_ds_type_messages_affected_history (DS, T);
  case 0x4ba3962e: return print_ds_type_bare_messages_affected_history (DS, T);
  case 0x84d19185: return print_ds_type_messages_affected_messages (DS, T);
  case 0x7b2e6e7a: return print_ds_type_bare_messages_affected_messages (DS, T);
  case 0xb485502f: return print_ds_type_messages_all_stickers (DS, T);
  case 0x4b7aafd0: return print_ds_type_bare_messages_all_stickers (DS, T);
  case 0xe5d7d19c: return print_ds_type_messages_chat_full (DS, T);
  case 0x1a282e63: return print_ds_type_bare_messages_chat_full (DS, T);
  case 0x64ff9fd5: return print_ds_type_messages_chats (DS, T);
  case 0x9b00602a: return print_ds_type_bare_messages_chats (DS, T);
  case 0xecc058e8: return print_ds_type_messages_dh_config (DS, T);
  case 0x133fa717: return print_ds_type_bare_messages_dh_config (DS, T);
  case 0x645af8b3: return print_ds_type_messages_dialogs (DS, T);
  case 0x9ba5074c: return print_ds_type_bare_messages_dialogs (DS, T);
  case 0x3f4e0648: return print_ds_type_messages_message (DS, T);
  case 0xc0b1f9b7: return print_ds_type_bare_messages_message (DS, T);
  case 0x8735e464: return print_ds_type_messages_messages (DS, T);
  case 0x78ca1b9b: return print_ds_type_bare_messages_messages (DS, T);
  case 0xc29c7607: return print_ds_type_messages_sent_encrypted_message (DS, T);
  case 0x3d6389f8: return print_ds_type_bare_messages_sent_encrypted_message (DS, T);
  case 0x799ce190: return print_ds_type_messages_sent_message (DS, T);
  case 0x86631e6f: return print_ds_type_bare_messages_sent_message (DS, T);
  case 0xb60a24a6: return print_ds_type_messages_sticker_set (DS, T);
  case 0x49f5db59: return print_ds_type_bare_messages_sticker_set (DS, T);
  case 0x7bfa5710: return print_ds_type_messages_stickers (DS, T);
  case 0x8405a8ef: return print_ds_type_bare_messages_stickers (DS, T);
  case 0x20212ca8: return print_ds_type_photos_photo (DS, T);
  case 0xdfded357: return print_ds_type_bare_photos_photo (DS, T);
  case 0x98cf75f1: return print_ds_type_photos_photos (DS, T);
  case 0x67308a0e: return print_ds_type_bare_photos_photos (DS, T);
  case 0x3e2838a8: return print_ds_type_storage_file_type (DS, T);
  case 0xc1d7c757: return print_ds_type_bare_storage_file_type (DS, T);
  case 0xf57a2419: return print_ds_type_updates_difference (DS, T);
  case 0x0a85dbe6: return print_ds_type_bare_updates_difference (DS, T);
  case 0xa56c2a3e: return print_ds_type_updates_state (DS, T);
  case 0x5a93d5c1: return print_ds_type_bare_updates_state (DS, T);
  case 0x096a18d5: return print_ds_type_upload_file (DS, T);
  case 0xf695e72a: return print_ds_type_bare_upload_file (DS, T);
  default: return -1; }
}
