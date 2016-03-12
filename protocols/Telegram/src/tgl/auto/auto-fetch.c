#ifdef _MSC_VER 
#include "..\auto.h"
#include "auto-fetch.h"
#include "auto-skip.h"
#include "..\auto-static-fetch.c"
#include "..\mtproto-common.h"
#else
#include "auto.h"
#include "auto/auto-fetch.h"
#include "auto/auto-skip.h"
#include "auto-static-fetch.c"
#include "mtproto-common.h"
#endif
#include <assert.h>
int fetch_constructor_account_days_t_t_l (struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0xb8d0afdf && T->type->name != 0x472f5020)) { return -1; }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  if (fetch_type_bare_int (field1) < 0) { return -1;}
  return 0;
}
int fetch_constructor_audio_empty (struct paramed_type *T) {
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
  if (fetch_type_bare_long (field1) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int fetch_constructor_audio (struct paramed_type *T) {
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
  if (fetch_type_bare_long (field1) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" access_hash :"); }
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xddf89345, .id = "Bare_Long", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  if (fetch_type_bare_long (field2) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" user_id :"); }
  struct paramed_type *field3 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  if (fetch_type_bare_int (field3) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" date :"); }
  struct paramed_type *field4 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  if (fetch_type_bare_int (field4) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" duration :"); }
  struct paramed_type *field5 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  if (fetch_type_bare_int (field5) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" mime_type :"); }
  struct paramed_type *field6 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  if (fetch_type_bare_string (field6) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" size :"); }
  struct paramed_type *field7 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  if (fetch_type_bare_int (field7) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" dc_id :"); }
  struct paramed_type *field8 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  if (fetch_type_bare_int (field8) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int fetch_constructor_authorization (struct paramed_type *T) {
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
  if (fetch_type_bare_long (field1) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" flags :"); }
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  if (fetch_type_bare_int (field2) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" device_model :"); }
  struct paramed_type *field3 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  if (fetch_type_bare_string (field3) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" platform :"); }
  struct paramed_type *field4 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  if (fetch_type_bare_string (field4) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" system_version :"); }
  struct paramed_type *field5 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  if (fetch_type_bare_string (field5) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" api_id :"); }
  struct paramed_type *field6 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  if (fetch_type_bare_int (field6) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" app_name :"); }
  struct paramed_type *field7 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  if (fetch_type_bare_string (field7) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" app_version :"); }
  struct paramed_type *field8 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  if (fetch_type_bare_string (field8) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" date_created :"); }
  struct paramed_type *field9 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  if (fetch_type_bare_int (field9) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" date_active :"); }
  struct paramed_type *field10 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  if (fetch_type_bare_int (field10) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" ip :"); }
  struct paramed_type *field11 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  if (fetch_type_bare_string (field11) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" country :"); }
  struct paramed_type *field12 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  if (fetch_type_bare_string (field12) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" region :"); }
  struct paramed_type *field13 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  if (fetch_type_bare_string (field13) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int fetch_constructor_bool_false (struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x250be282 && T->type->name != 0xdaf41d7d)) { return -1; }
  eprintf (" boolFalse");
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int fetch_constructor_bool_true (struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x250be282 && T->type->name != 0xdaf41d7d)) { return -1; }
  eprintf (" boolTrue");
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int fetch_constructor_bot_command (struct paramed_type *T) {
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
  if (fetch_type_bare_string (field1) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" description :"); }
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  if (fetch_type_bare_string (field2) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int fetch_constructor_bot_command_old (struct paramed_type *T) {
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
  if (fetch_type_bare_string (field1) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" params :"); }
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  if (fetch_type_bare_string (field2) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" description :"); }
  struct paramed_type *field3 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  if (fetch_type_bare_string (field3) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int fetch_constructor_bot_info_empty (struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0xb2e16f93 && T->type->name != 0x4d1e906c)) { return -1; }
  eprintf (" botInfoEmpty");
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int fetch_constructor_bot_info (struct paramed_type *T) {
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
  if (fetch_type_bare_int (field1) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" version :"); }
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  if (fetch_type_bare_int (field2) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" share_text :"); }
  struct paramed_type *field3 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  if (fetch_type_bare_string (field3) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" description :"); }
  struct paramed_type *field4 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  if (fetch_type_bare_string (field4) < 0) { return -1;}
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
  if (fetch_type_vector (field5) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int fetch_constructor_bytes (struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x0ee1379f && T->type->name != 0xf11ec860)) { return -1; }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  if (fetch_type_bare_string (field1) < 0) { return -1;}
  return 0;
}
int fetch_constructor_chat_empty (struct paramed_type *T) {
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
  if (fetch_type_bare_int (field1) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int fetch_constructor_chat (struct paramed_type *T) {
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
  if (fetch_type_bare_int (field1) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" title :"); }
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  if (fetch_type_bare_string (field2) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" photo :"); }
  struct paramed_type *field3 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x56922676, .id = "ChatPhoto", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  if (fetch_type_chat_photo (field3) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" participants_count :"); }
  struct paramed_type *field4 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  if (fetch_type_bare_int (field4) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" date :"); }
  struct paramed_type *field5 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  if (fetch_type_bare_int (field5) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" left :"); }
  struct paramed_type *field6 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x250be282, .id = "Bool", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  if (fetch_type_bool (field6) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" version :"); }
  struct paramed_type *field7 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  if (fetch_type_bare_int (field7) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int fetch_constructor_chat_forbidden (struct paramed_type *T) {
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
  if (fetch_type_bare_int (field1) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" title :"); }
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  if (fetch_type_bare_string (field2) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" date :"); }
  struct paramed_type *field3 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  if (fetch_type_bare_int (field3) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int fetch_constructor_geo_chat (struct paramed_type *T) {
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
  if (fetch_type_bare_int (field1) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" access_hash :"); }
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xddf89345, .id = "Bare_Long", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  if (fetch_type_bare_long (field2) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" title :"); }
  struct paramed_type *field3 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  if (fetch_type_bare_string (field3) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" address :"); }
  struct paramed_type *field4 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  if (fetch_type_bare_string (field4) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" venue :"); }
  struct paramed_type *field5 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  if (fetch_type_bare_string (field5) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" geo :"); }
  struct paramed_type *field6 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x315e0a53, .id = "GeoPoint", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  if (fetch_type_geo_point (field6) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" photo :"); }
  struct paramed_type *field7 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x56922676, .id = "ChatPhoto", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  if (fetch_type_chat_photo (field7) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" participants_count :"); }
  struct paramed_type *field8 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  if (fetch_type_bare_int (field8) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" date :"); }
  struct paramed_type *field9 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  if (fetch_type_bare_int (field9) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" checked_in :"); }
  struct paramed_type *field10 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x250be282, .id = "Bool", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  if (fetch_type_bool (field10) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" version :"); }
  struct paramed_type *field11 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  if (fetch_type_bare_int (field11) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int fetch_constructor_chat_full (struct paramed_type *T) {
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
  if (fetch_type_bare_int (field1) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" participants :"); }
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x77930f9f, .id = "ChatParticipants", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  if (fetch_type_chat_participants (field2) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" chat_photo :"); }
  struct paramed_type *field3 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xc207550a, .id = "Photo", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  if (fetch_type_photo (field3) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" notify_settings :"); }
  struct paramed_type *field4 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xfdf894fc, .id = "PeerNotifySettings", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  if (fetch_type_peer_notify_settings (field4) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" exported_invite :"); }
  struct paramed_type *field5 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x95f132d5, .id = "ExportedChatInvite", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  if (fetch_type_exported_chat_invite (field5) < 0) { return -1;}
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
  if (fetch_type_vector (field6) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int fetch_constructor_chat_invite_already (struct paramed_type *T) {
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
  if (fetch_type_chat (field1) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int fetch_constructor_chat_invite (struct paramed_type *T) {
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
  if (fetch_type_bare_string (field1) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int fetch_constructor_chat_located (struct paramed_type *T) {
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
  if (fetch_type_bare_int (field1) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" distance :"); }
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  if (fetch_type_bare_int (field2) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int fetch_constructor_chat_participant (struct paramed_type *T) {
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
  if (fetch_type_bare_int (field1) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" inviter_id :"); }
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  if (fetch_type_bare_int (field2) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" date :"); }
  struct paramed_type *field3 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  if (fetch_type_bare_int (field3) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int fetch_constructor_chat_participants_forbidden (struct paramed_type *T) {
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
  if (fetch_type_bare_int (field1) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int fetch_constructor_chat_participants (struct paramed_type *T) {
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
  if (fetch_type_bare_int (field1) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" admin_id :"); }
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  if (fetch_type_bare_int (field2) < 0) { return -1;}
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
  if (fetch_type_vector (field3) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" version :"); }
  struct paramed_type *field4 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  if (fetch_type_bare_int (field4) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int fetch_constructor_chat_photo_empty (struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x56922676 && T->type->name != 0xa96dd989)) { return -1; }
  eprintf (" chatPhotoEmpty");
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int fetch_constructor_chat_photo (struct paramed_type *T) {
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
  if (fetch_type_file_location (field1) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" photo_big :"); }
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x2f8ffb30, .id = "FileLocation", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  if (fetch_type_file_location (field2) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int fetch_constructor_client_d_h_inner_data (struct paramed_type *T) {
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
  if (fetch_type_bare_int128 (field1) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" server_nonce :"); }
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x82c93bc6, .id = "Bare_Int128", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  if (fetch_type_bare_int128 (field2) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" retry_id :"); }
  struct paramed_type *field3 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xddf89345, .id = "Bare_Long", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  if (fetch_type_bare_long (field3) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" g_b :"); }
  struct paramed_type *field4 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  if (fetch_type_bare_string (field4) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int fetch_constructor_config (struct paramed_type *T) {
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
  if (fetch_type_bare_int (field1) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" expires :"); }
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  if (fetch_type_bare_int (field2) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" test_mode :"); }
  struct paramed_type *field3 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x250be282, .id = "Bool", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  if (fetch_type_bool (field3) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" this_dc :"); }
  struct paramed_type *field4 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  if (fetch_type_bare_int (field4) < 0) { return -1;}
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
  if (fetch_type_vector (field5) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" chat_size_max :"); }
  struct paramed_type *field6 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  if (fetch_type_bare_int (field6) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" broadcast_size_max :"); }
  struct paramed_type *field7 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  if (fetch_type_bare_int (field7) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" forwarded_count_max :"); }
  struct paramed_type *field8 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  if (fetch_type_bare_int (field8) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" online_update_period_ms :"); }
  struct paramed_type *field9 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  if (fetch_type_bare_int (field9) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" offline_blur_timeout_ms :"); }
  struct paramed_type *field10 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  if (fetch_type_bare_int (field10) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" offline_idle_timeout_ms :"); }
  struct paramed_type *field11 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  if (fetch_type_bare_int (field11) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" online_cloud_timeout_ms :"); }
  struct paramed_type *field12 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  if (fetch_type_bare_int (field12) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" notify_cloud_delay_ms :"); }
  struct paramed_type *field13 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  if (fetch_type_bare_int (field13) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" notify_default_delay_ms :"); }
  struct paramed_type *field14 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  if (fetch_type_bare_int (field14) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" chat_big_size :"); }
  struct paramed_type *field15 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  if (fetch_type_bare_int (field15) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" push_chat_period_ms :"); }
  struct paramed_type *field16 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  if (fetch_type_bare_int (field16) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" push_chat_limit :"); }
  struct paramed_type *field17 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  if (fetch_type_bare_int (field17) < 0) { return -1;}
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
  if (fetch_type_vector (field18) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int fetch_constructor_contact (struct paramed_type *T) {
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
  if (fetch_type_bare_int (field1) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" mutual :"); }
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x250be282, .id = "Bool", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  if (fetch_type_bool (field2) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int fetch_constructor_contact_blocked (struct paramed_type *T) {
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
  if (fetch_type_bare_int (field1) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" date :"); }
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  if (fetch_type_bare_int (field2) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int fetch_constructor_contact_found (struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0xea879f95 && T->type->name != 0x1578606a)) { return -1; }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  if (fetch_type_bare_int (field1) < 0) { return -1;}
  return 0;
}
int fetch_constructor_contact_link_unknown (struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x522fbc63 && T->type->name != 0xadd0439c)) { return -1; }
  eprintf (" contactLinkUnknown");
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int fetch_constructor_contact_link_none (struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x522fbc63 && T->type->name != 0xadd0439c)) { return -1; }
  eprintf (" contactLinkNone");
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int fetch_constructor_contact_link_has_phone (struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x522fbc63 && T->type->name != 0xadd0439c)) { return -1; }
  eprintf (" contactLinkHasPhone");
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int fetch_constructor_contact_link_contact (struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x522fbc63 && T->type->name != 0xadd0439c)) { return -1; }
  eprintf (" contactLinkContact");
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int fetch_constructor_contact_status (struct paramed_type *T) {
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
  if (fetch_type_bare_int (field1) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" status :"); }
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x76de9570, .id = "UserStatus", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  if (fetch_type_user_status (field2) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int fetch_constructor_contact_suggested (struct paramed_type *T) {
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
  if (fetch_type_bare_int (field1) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" mutual_contacts :"); }
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  if (fetch_type_bare_int (field2) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int fetch_constructor_dc_option_l28 (struct paramed_type *T) {
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
  if (fetch_type_bare_int (field1) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" hostname :"); }
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  if (fetch_type_bare_string (field2) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" ip_address :"); }
  struct paramed_type *field3 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  if (fetch_type_bare_string (field3) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" port :"); }
  struct paramed_type *field4 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  if (fetch_type_bare_int (field4) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int fetch_constructor_dc_option (struct paramed_type *T) {
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
  if (fetch_type_bare_int (field1) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" id :"); }
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  if (fetch_type_bare_int (field2) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" ip_address :"); }
  struct paramed_type *field3 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  if (fetch_type_bare_string (field3) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" port :"); }
  struct paramed_type *field4 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  if (fetch_type_bare_int (field4) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int fetch_constructor_decrypted_message (struct paramed_type *T) {
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
  if (fetch_type_bare_long (field1) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" ttl :"); }
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  if (fetch_type_bare_int (field2) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" message :"); }
  struct paramed_type *field3 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  if (fetch_type_bare_string (field3) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" media :"); }
  struct paramed_type *field4 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x34e0d674, .id = "DecryptedMessageMedia", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  if (fetch_type_decrypted_message_media (field4) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int fetch_constructor_decrypted_message_service (struct paramed_type *T) {
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
  if (fetch_type_bare_long (field1) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" action :"); }
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x4e0eefde, .id = "DecryptedMessageAction", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  if (fetch_type_decrypted_message_action (field2) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int fetch_constructor_decrypted_message_action_set_message_t_t_l (struct paramed_type *T) {
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
  if (fetch_type_bare_int (field1) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int fetch_constructor_decrypted_message_action_read_messages (struct paramed_type *T) {
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
  if (fetch_type_vector (field1) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int fetch_constructor_decrypted_message_action_delete_messages (struct paramed_type *T) {
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
  if (fetch_type_vector (field1) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int fetch_constructor_decrypted_message_action_screenshot_messages (struct paramed_type *T) {
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
  if (fetch_type_vector (field1) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int fetch_constructor_decrypted_message_action_flush_history (struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x4e0eefde && T->type->name != 0xb1f11021)) { return -1; }
  eprintf (" decryptedMessageActionFlushHistory");
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int fetch_constructor_decrypted_message_action_resend (struct paramed_type *T) {
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
  if (fetch_type_bare_int (field1) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" end_seq_no :"); }
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  if (fetch_type_bare_int (field2) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int fetch_constructor_decrypted_message_action_notify_layer (struct paramed_type *T) {
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
  if (fetch_type_bare_int (field1) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int fetch_constructor_decrypted_message_action_typing (struct paramed_type *T) {
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
  if (fetch_type_send_message_action (field1) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int fetch_constructor_decrypted_message_action_request_key (struct paramed_type *T) {
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
  if (fetch_type_bare_long (field1) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" g_a :"); }
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xf11ec860, .id = "Bare_Bytes", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  if (fetch_type_bare_bytes (field2) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int fetch_constructor_decrypted_message_action_accept_key (struct paramed_type *T) {
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
  if (fetch_type_bare_long (field1) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" g_b :"); }
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xf11ec860, .id = "Bare_Bytes", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  if (fetch_type_bare_bytes (field2) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" key_fingerprint :"); }
  struct paramed_type *field3 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xddf89345, .id = "Bare_Long", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  if (fetch_type_bare_long (field3) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int fetch_constructor_decrypted_message_action_abort_key (struct paramed_type *T) {
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
  if (fetch_type_bare_long (field1) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int fetch_constructor_decrypted_message_action_commit_key (struct paramed_type *T) {
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
  if (fetch_type_bare_long (field1) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" key_fingerprint :"); }
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xddf89345, .id = "Bare_Long", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  if (fetch_type_bare_long (field2) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int fetch_constructor_decrypted_message_action_noop (struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x4e0eefde && T->type->name != 0xb1f11021)) { return -1; }
  eprintf (" decryptedMessageActionNoop");
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int fetch_constructor_decrypted_message_layer (struct paramed_type *T) {
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
  if (fetch_type_bare_bytes (field1) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" layer :"); }
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  if (fetch_type_bare_int (field2) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" in_seq_no :"); }
  struct paramed_type *field3 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  if (fetch_type_bare_int (field3) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" out_seq_no :"); }
  struct paramed_type *field4 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  if (fetch_type_bare_int (field4) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" message :"); }
  struct paramed_type *field5 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x535b7918, .id = "DecryptedMessage", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  if (fetch_type_decrypted_message (field5) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int fetch_constructor_decrypted_message_media_empty (struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x34e0d674 && T->type->name != 0xcb1f298b)) { return -1; }
  eprintf (" decryptedMessageMediaEmpty");
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int fetch_constructor_decrypted_message_media_photo (struct paramed_type *T) {
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
  if (fetch_type_bare_bytes (field1) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" thumb_w :"); }
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  if (fetch_type_bare_int (field2) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" thumb_h :"); }
  struct paramed_type *field3 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  if (fetch_type_bare_int (field3) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" w :"); }
  struct paramed_type *field4 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  if (fetch_type_bare_int (field4) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" h :"); }
  struct paramed_type *field5 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  if (fetch_type_bare_int (field5) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" size :"); }
  struct paramed_type *field6 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  if (fetch_type_bare_int (field6) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" key :"); }
  struct paramed_type *field7 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xf11ec860, .id = "Bare_Bytes", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  if (fetch_type_bare_bytes (field7) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" iv :"); }
  struct paramed_type *field8 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xf11ec860, .id = "Bare_Bytes", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  if (fetch_type_bare_bytes (field8) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int fetch_constructor_decrypted_message_media_geo_point (struct paramed_type *T) {
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
  if (fetch_type_bare_double (field1) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" longitude :"); }
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xddef3eab, .id = "Bare_Double", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  if (fetch_type_bare_double (field2) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int fetch_constructor_decrypted_message_media_contact (struct paramed_type *T) {
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
  if (fetch_type_bare_string (field1) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" first_name :"); }
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  if (fetch_type_bare_string (field2) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" last_name :"); }
  struct paramed_type *field3 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  if (fetch_type_bare_string (field3) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" user_id :"); }
  struct paramed_type *field4 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  if (fetch_type_bare_int (field4) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int fetch_constructor_decrypted_message_media_document (struct paramed_type *T) {
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
  if (fetch_type_bare_bytes (field1) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" thumb_w :"); }
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  if (fetch_type_bare_int (field2) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" thumb_h :"); }
  struct paramed_type *field3 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  if (fetch_type_bare_int (field3) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" file_name :"); }
  struct paramed_type *field4 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  if (fetch_type_bare_string (field4) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" mime_type :"); }
  struct paramed_type *field5 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  if (fetch_type_bare_string (field5) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" size :"); }
  struct paramed_type *field6 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  if (fetch_type_bare_int (field6) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" key :"); }
  struct paramed_type *field7 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xf11ec860, .id = "Bare_Bytes", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  if (fetch_type_bare_bytes (field7) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" iv :"); }
  struct paramed_type *field8 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xf11ec860, .id = "Bare_Bytes", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  if (fetch_type_bare_bytes (field8) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int fetch_constructor_decrypted_message_media_video (struct paramed_type *T) {
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
  if (fetch_type_bare_bytes (field1) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" thumb_w :"); }
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  if (fetch_type_bare_int (field2) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" thumb_h :"); }
  struct paramed_type *field3 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  if (fetch_type_bare_int (field3) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" duration :"); }
  struct paramed_type *field4 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  if (fetch_type_bare_int (field4) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" mime_type :"); }
  struct paramed_type *field5 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  if (fetch_type_bare_string (field5) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" w :"); }
  struct paramed_type *field6 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  if (fetch_type_bare_int (field6) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" h :"); }
  struct paramed_type *field7 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  if (fetch_type_bare_int (field7) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" size :"); }
  struct paramed_type *field8 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  if (fetch_type_bare_int (field8) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" key :"); }
  struct paramed_type *field9 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xf11ec860, .id = "Bare_Bytes", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  if (fetch_type_bare_bytes (field9) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" iv :"); }
  struct paramed_type *field10 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xf11ec860, .id = "Bare_Bytes", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  if (fetch_type_bare_bytes (field10) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int fetch_constructor_decrypted_message_media_audio (struct paramed_type *T) {
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
  if (fetch_type_bare_int (field1) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" mime_type :"); }
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  if (fetch_type_bare_string (field2) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" size :"); }
  struct paramed_type *field3 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  if (fetch_type_bare_int (field3) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" key :"); }
  struct paramed_type *field4 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xf11ec860, .id = "Bare_Bytes", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  if (fetch_type_bare_bytes (field4) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" iv :"); }
  struct paramed_type *field5 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xf11ec860, .id = "Bare_Bytes", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  if (fetch_type_bare_bytes (field5) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int fetch_constructor_decrypted_message_media_external_document (struct paramed_type *T) {
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
  if (fetch_type_bare_long (field1) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" access_hash :"); }
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xddf89345, .id = "Bare_Long", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  if (fetch_type_bare_long (field2) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" date :"); }
  struct paramed_type *field3 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  if (fetch_type_bare_int (field3) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" mime_type :"); }
  struct paramed_type *field4 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  if (fetch_type_bare_string (field4) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" size :"); }
  struct paramed_type *field5 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  if (fetch_type_bare_int (field5) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" thumb :"); }
  struct paramed_type *field6 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x900f60dd, .id = "PhotoSize", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  if (fetch_type_photo_size (field6) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" dc_id :"); }
  struct paramed_type *field7 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  if (fetch_type_bare_int (field7) < 0) { return -1;}
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
  if (fetch_type_vector (field8) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int fetch_constructor_decrypted_message_media_video_l12 (struct paramed_type *T) {
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
  if (fetch_type_bare_bytes (field1) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" thumb_w :"); }
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  if (fetch_type_bare_int (field2) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" thumb_h :"); }
  struct paramed_type *field3 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  if (fetch_type_bare_int (field3) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" duration :"); }
  struct paramed_type *field4 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  if (fetch_type_bare_int (field4) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" w :"); }
  struct paramed_type *field5 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  if (fetch_type_bare_int (field5) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" h :"); }
  struct paramed_type *field6 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  if (fetch_type_bare_int (field6) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" size :"); }
  struct paramed_type *field7 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  if (fetch_type_bare_int (field7) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" key :"); }
  struct paramed_type *field8 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xf11ec860, .id = "Bare_Bytes", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  if (fetch_type_bare_bytes (field8) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" iv :"); }
  struct paramed_type *field9 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xf11ec860, .id = "Bare_Bytes", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  if (fetch_type_bare_bytes (field9) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int fetch_constructor_decrypted_message_media_audio_l12 (struct paramed_type *T) {
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
  if (fetch_type_bare_int (field1) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" size :"); }
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  if (fetch_type_bare_int (field2) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" key :"); }
  struct paramed_type *field3 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xf11ec860, .id = "Bare_Bytes", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  if (fetch_type_bare_bytes (field3) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" iv :"); }
  struct paramed_type *field4 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xf11ec860, .id = "Bare_Bytes", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  if (fetch_type_bare_bytes (field4) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int fetch_constructor_dialog (struct paramed_type *T) {
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
  if (fetch_type_peer (field1) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" top_message :"); }
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  if (fetch_type_bare_int (field2) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" read_inbox_max_id :"); }
  struct paramed_type *field3 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  if (fetch_type_bare_int (field3) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" unread_count :"); }
  struct paramed_type *field4 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  if (fetch_type_bare_int (field4) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" notify_settings :"); }
  struct paramed_type *field5 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xfdf894fc, .id = "PeerNotifySettings", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  if (fetch_type_peer_notify_settings (field5) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int fetch_constructor_disabled_feature (struct paramed_type *T) {
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
  if (fetch_type_bare_string (field1) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" description :"); }
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  if (fetch_type_bare_string (field2) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int fetch_constructor_document_empty (struct paramed_type *T) {
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
  if (fetch_type_bare_long (field1) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int fetch_constructor_document (struct paramed_type *T) {
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
  if (fetch_type_bare_long (field1) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" access_hash :"); }
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xddf89345, .id = "Bare_Long", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  if (fetch_type_bare_long (field2) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" date :"); }
  struct paramed_type *field3 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  if (fetch_type_bare_int (field3) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" mime_type :"); }
  struct paramed_type *field4 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  if (fetch_type_bare_string (field4) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" size :"); }
  struct paramed_type *field5 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  if (fetch_type_bare_int (field5) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" thumb :"); }
  struct paramed_type *field6 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x900f60dd, .id = "PhotoSize", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  if (fetch_type_photo_size (field6) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" dc_id :"); }
  struct paramed_type *field7 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  if (fetch_type_bare_int (field7) < 0) { return -1;}
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
  if (fetch_type_vector (field8) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int fetch_constructor_document_l19 (struct paramed_type *T) {
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
  if (fetch_type_bare_long (field1) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" access_hash :"); }
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xddf89345, .id = "Bare_Long", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  if (fetch_type_bare_long (field2) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" user_id :"); }
  struct paramed_type *field3 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  if (fetch_type_bare_int (field3) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" date :"); }
  struct paramed_type *field4 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  if (fetch_type_bare_int (field4) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" file_name :"); }
  struct paramed_type *field5 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  if (fetch_type_bare_string (field5) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" mime_type :"); }
  struct paramed_type *field6 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  if (fetch_type_bare_string (field6) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" size :"); }
  struct paramed_type *field7 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  if (fetch_type_bare_int (field7) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" thumb :"); }
  struct paramed_type *field8 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x900f60dd, .id = "PhotoSize", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  if (fetch_type_photo_size (field8) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" dc_id :"); }
  struct paramed_type *field9 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  if (fetch_type_bare_int (field9) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int fetch_constructor_document_attribute_image_size (struct paramed_type *T) {
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
  if (fetch_type_bare_int (field1) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" h :"); }
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  if (fetch_type_bare_int (field2) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int fetch_constructor_document_attribute_animated (struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x97c637a3 && T->type->name != 0x6839c85c)) { return -1; }
  eprintf (" documentAttributeAnimated");
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int fetch_constructor_document_attribute_sticker_l28 (struct paramed_type *T) {
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
  if (fetch_type_bare_string (field1) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int fetch_constructor_document_attribute_sticker (struct paramed_type *T) {
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
  if (fetch_type_bare_string (field1) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" stickerset :"); }
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xe44d415c, .id = "InputStickerSet", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  if (fetch_type_input_sticker_set (field2) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int fetch_constructor_document_attribute_video (struct paramed_type *T) {
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
  if (fetch_type_bare_int (field1) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" w :"); }
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  if (fetch_type_bare_int (field2) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" h :"); }
  struct paramed_type *field3 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  if (fetch_type_bare_int (field3) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int fetch_constructor_document_attribute_audio (struct paramed_type *T) {
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
  if (fetch_type_bare_int (field1) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int fetch_constructor_document_attribute_filename (struct paramed_type *T) {
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
  if (fetch_type_bare_string (field1) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int fetch_constructor_double (struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x2210c154 && T->type->name != 0xddef3eab)) { return -1; }
  if (in_remaining () < 8) { return -1;}
  eprintf (" %lf", fetch_double ());
  return 0;
}
int fetch_constructor_encrypted_chat_empty (struct paramed_type *T) {
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
  if (fetch_type_bare_int (field1) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int fetch_constructor_encrypted_chat_waiting (struct paramed_type *T) {
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
  if (fetch_type_bare_int (field1) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" access_hash :"); }
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xddf89345, .id = "Bare_Long", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  if (fetch_type_bare_long (field2) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" date :"); }
  struct paramed_type *field3 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  if (fetch_type_bare_int (field3) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" admin_id :"); }
  struct paramed_type *field4 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  if (fetch_type_bare_int (field4) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" participant_id :"); }
  struct paramed_type *field5 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  if (fetch_type_bare_int (field5) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int fetch_constructor_encrypted_chat_requested (struct paramed_type *T) {
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
  if (fetch_type_bare_int (field1) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" access_hash :"); }
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xddf89345, .id = "Bare_Long", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  if (fetch_type_bare_long (field2) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" date :"); }
  struct paramed_type *field3 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  if (fetch_type_bare_int (field3) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" admin_id :"); }
  struct paramed_type *field4 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  if (fetch_type_bare_int (field4) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" participant_id :"); }
  struct paramed_type *field5 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  if (fetch_type_bare_int (field5) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" g_a :"); }
  struct paramed_type *field6 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xf11ec860, .id = "Bare_Bytes", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  if (fetch_type_bare_bytes (field6) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int fetch_constructor_encrypted_chat (struct paramed_type *T) {
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
  if (fetch_type_bare_int (field1) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" access_hash :"); }
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xddf89345, .id = "Bare_Long", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  if (fetch_type_bare_long (field2) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" date :"); }
  struct paramed_type *field3 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  if (fetch_type_bare_int (field3) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" admin_id :"); }
  struct paramed_type *field4 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  if (fetch_type_bare_int (field4) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" participant_id :"); }
  struct paramed_type *field5 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  if (fetch_type_bare_int (field5) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" g_a_or_b :"); }
  struct paramed_type *field6 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xf11ec860, .id = "Bare_Bytes", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  if (fetch_type_bare_bytes (field6) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" key_fingerprint :"); }
  struct paramed_type *field7 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xddf89345, .id = "Bare_Long", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  if (fetch_type_bare_long (field7) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int fetch_constructor_encrypted_chat_discarded (struct paramed_type *T) {
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
  if (fetch_type_bare_int (field1) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int fetch_constructor_encrypted_file_empty (struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x886fd032 && T->type->name != 0x77902fcd)) { return -1; }
  eprintf (" encryptedFileEmpty");
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int fetch_constructor_encrypted_file (struct paramed_type *T) {
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
  if (fetch_type_bare_long (field1) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" access_hash :"); }
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xddf89345, .id = "Bare_Long", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  if (fetch_type_bare_long (field2) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" size :"); }
  struct paramed_type *field3 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  if (fetch_type_bare_int (field3) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" dc_id :"); }
  struct paramed_type *field4 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  if (fetch_type_bare_int (field4) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" key_fingerprint :"); }
  struct paramed_type *field5 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  if (fetch_type_bare_int (field5) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int fetch_constructor_encrypted_message (struct paramed_type *T) {
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
  if (fetch_type_bare_long (field1) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" chat_id :"); }
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  if (fetch_type_bare_int (field2) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" date :"); }
  struct paramed_type *field3 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  if (fetch_type_bare_int (field3) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" bytes :"); }
  struct paramed_type *field4 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xf11ec860, .id = "Bare_Bytes", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  if (fetch_type_bare_bytes (field4) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" file :"); }
  struct paramed_type *field5 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x886fd032, .id = "EncryptedFile", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  if (fetch_type_encrypted_file (field5) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int fetch_constructor_encrypted_message_service (struct paramed_type *T) {
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
  if (fetch_type_bare_long (field1) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" chat_id :"); }
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  if (fetch_type_bare_int (field2) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" date :"); }
  struct paramed_type *field3 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  if (fetch_type_bare_int (field3) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" bytes :"); }
  struct paramed_type *field4 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xf11ec860, .id = "Bare_Bytes", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  if (fetch_type_bare_bytes (field4) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int fetch_constructor_error (struct paramed_type *T) {
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
  if (fetch_type_bare_int (field1) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" text :"); }
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  if (fetch_type_bare_string (field2) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int fetch_constructor_chat_invite_empty (struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x95f132d5 && T->type->name != 0x6a0ecd2a)) { return -1; }
  eprintf (" chatInviteEmpty");
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int fetch_constructor_chat_invite_exported (struct paramed_type *T) {
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
  if (fetch_type_bare_string (field1) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int fetch_constructor_file_location_unavailable (struct paramed_type *T) {
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
  if (fetch_type_bare_long (field1) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" local_id :"); }
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  if (fetch_type_bare_int (field2) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" secret :"); }
  struct paramed_type *field3 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xddf89345, .id = "Bare_Long", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  if (fetch_type_bare_long (field3) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int fetch_constructor_file_location (struct paramed_type *T) {
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
  if (fetch_type_bare_int (field1) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" volume_id :"); }
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xddf89345, .id = "Bare_Long", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  if (fetch_type_bare_long (field2) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" local_id :"); }
  struct paramed_type *field3 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  if (fetch_type_bare_int (field3) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" secret :"); }
  struct paramed_type *field4 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xddf89345, .id = "Bare_Long", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  if (fetch_type_bare_long (field4) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int fetch_constructor_geo_chat_message_empty (struct paramed_type *T) {
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
  if (fetch_type_bare_int (field1) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" id :"); }
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  if (fetch_type_bare_int (field2) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int fetch_constructor_geo_chat_message (struct paramed_type *T) {
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
  if (fetch_type_bare_int (field1) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" id :"); }
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  if (fetch_type_bare_int (field2) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" from_id :"); }
  struct paramed_type *field3 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  if (fetch_type_bare_int (field3) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" date :"); }
  struct paramed_type *field4 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  if (fetch_type_bare_int (field4) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" message :"); }
  struct paramed_type *field5 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  if (fetch_type_bare_string (field5) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" media :"); }
  struct paramed_type *field6 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x95f2471a, .id = "MessageMedia", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  if (fetch_type_message_media (field6) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int fetch_constructor_geo_chat_message_service (struct paramed_type *T) {
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
  if (fetch_type_bare_int (field1) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" id :"); }
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  if (fetch_type_bare_int (field2) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" from_id :"); }
  struct paramed_type *field3 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  if (fetch_type_bare_int (field3) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" date :"); }
  struct paramed_type *field4 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  if (fetch_type_bare_int (field4) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" action :"); }
  struct paramed_type *field5 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x383415fa, .id = "MessageAction", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  if (fetch_type_message_action (field5) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int fetch_constructor_geo_point_empty (struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x315e0a53 && T->type->name != 0xcea1f5ac)) { return -1; }
  eprintf (" geoPointEmpty");
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int fetch_constructor_geo_point (struct paramed_type *T) {
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
  if (fetch_type_bare_double (field1) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" latitude :"); }
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xddef3eab, .id = "Bare_Double", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  if (fetch_type_bare_double (field2) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int fetch_constructor_imported_contact (struct paramed_type *T) {
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
  if (fetch_type_bare_int (field1) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" client_id :"); }
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xddf89345, .id = "Bare_Long", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  if (fetch_type_bare_long (field2) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int fetch_constructor_input_app_event (struct paramed_type *T) {
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
  if (fetch_type_bare_double (field1) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" type :"); }
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  if (fetch_type_bare_string (field2) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" peer :"); }
  struct paramed_type *field3 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xddf89345, .id = "Bare_Long", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  if (fetch_type_bare_long (field3) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" data :"); }
  struct paramed_type *field4 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  if (fetch_type_bare_string (field4) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int fetch_constructor_input_audio_empty (struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0xae8e9c7b && T->type->name != 0x51716384)) { return -1; }
  eprintf (" inputAudioEmpty");
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int fetch_constructor_input_audio (struct paramed_type *T) {
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
  if (fetch_type_bare_long (field1) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" access_hash :"); }
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xddf89345, .id = "Bare_Long", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  if (fetch_type_bare_long (field2) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int fetch_constructor_input_chat_photo_empty (struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x3a60776d && T->type->name != 0xc59f8892)) { return -1; }
  eprintf (" inputChatPhotoEmpty");
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int fetch_constructor_input_chat_uploaded_photo (struct paramed_type *T) {
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
  if (fetch_type_input_file (field1) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" crop :"); }
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x7477e321, .id = "InputPhotoCrop", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  if (fetch_type_input_photo_crop (field2) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int fetch_constructor_input_chat_photo (struct paramed_type *T) {
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
  if (fetch_type_input_photo (field1) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" crop :"); }
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x7477e321, .id = "InputPhotoCrop", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  if (fetch_type_input_photo_crop (field2) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int fetch_constructor_input_phone_contact (struct paramed_type *T) {
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
  if (fetch_type_bare_long (field1) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" phone :"); }
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  if (fetch_type_bare_string (field2) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" first_name :"); }
  struct paramed_type *field3 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  if (fetch_type_bare_string (field3) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" last_name :"); }
  struct paramed_type *field4 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  if (fetch_type_bare_string (field4) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int fetch_constructor_input_document_empty (struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x6a8963fc && T->type->name != 0x95769c03)) { return -1; }
  eprintf (" inputDocumentEmpty");
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int fetch_constructor_input_document (struct paramed_type *T) {
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
  if (fetch_type_bare_long (field1) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" access_hash :"); }
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xddf89345, .id = "Bare_Long", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  if (fetch_type_bare_long (field2) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int fetch_constructor_input_encrypted_chat (struct paramed_type *T) {
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
  if (fetch_type_bare_int (field1) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" access_hash :"); }
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xddf89345, .id = "Bare_Long", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  if (fetch_type_bare_long (field2) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int fetch_constructor_input_encrypted_file_empty (struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x0b5c064f && T->type->name != 0xf4a3f9b0)) { return -1; }
  eprintf (" inputEncryptedFileEmpty");
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int fetch_constructor_input_encrypted_file_uploaded (struct paramed_type *T) {
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
  if (fetch_type_bare_long (field1) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" parts :"); }
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  if (fetch_type_bare_int (field2) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" md5_checksum :"); }
  struct paramed_type *field3 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  if (fetch_type_bare_string (field3) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" key_fingerprint :"); }
  struct paramed_type *field4 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  if (fetch_type_bare_int (field4) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int fetch_constructor_input_encrypted_file (struct paramed_type *T) {
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
  if (fetch_type_bare_long (field1) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" access_hash :"); }
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xddf89345, .id = "Bare_Long", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  if (fetch_type_bare_long (field2) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int fetch_constructor_input_encrypted_file_big_uploaded (struct paramed_type *T) {
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
  if (fetch_type_bare_long (field1) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" parts :"); }
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  if (fetch_type_bare_int (field2) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" key_fingerprint :"); }
  struct paramed_type *field3 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  if (fetch_type_bare_int (field3) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int fetch_constructor_input_file (struct paramed_type *T) {
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
  if (fetch_type_bare_long (field1) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" parts :"); }
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  if (fetch_type_bare_int (field2) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" name :"); }
  struct paramed_type *field3 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  if (fetch_type_bare_string (field3) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" md5_checksum :"); }
  struct paramed_type *field4 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  if (fetch_type_bare_string (field4) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int fetch_constructor_input_file_big (struct paramed_type *T) {
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
  if (fetch_type_bare_long (field1) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" parts :"); }
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  if (fetch_type_bare_int (field2) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" name :"); }
  struct paramed_type *field3 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  if (fetch_type_bare_string (field3) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int fetch_constructor_input_file_location (struct paramed_type *T) {
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
  if (fetch_type_bare_long (field1) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" local_id :"); }
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  if (fetch_type_bare_int (field2) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" secret :"); }
  struct paramed_type *field3 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xddf89345, .id = "Bare_Long", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  if (fetch_type_bare_long (field3) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int fetch_constructor_input_video_file_location (struct paramed_type *T) {
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
  if (fetch_type_bare_long (field1) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" access_hash :"); }
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xddf89345, .id = "Bare_Long", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  if (fetch_type_bare_long (field2) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int fetch_constructor_input_encrypted_file_location (struct paramed_type *T) {
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
  if (fetch_type_bare_long (field1) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" access_hash :"); }
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xddf89345, .id = "Bare_Long", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  if (fetch_type_bare_long (field2) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int fetch_constructor_input_audio_file_location (struct paramed_type *T) {
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
  if (fetch_type_bare_long (field1) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" access_hash :"); }
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xddf89345, .id = "Bare_Long", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  if (fetch_type_bare_long (field2) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int fetch_constructor_input_document_file_location (struct paramed_type *T) {
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
  if (fetch_type_bare_long (field1) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" access_hash :"); }
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xddf89345, .id = "Bare_Long", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  if (fetch_type_bare_long (field2) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int fetch_constructor_input_geo_chat (struct paramed_type *T) {
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
  if (fetch_type_bare_int (field1) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" access_hash :"); }
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xddf89345, .id = "Bare_Long", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  if (fetch_type_bare_long (field2) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int fetch_constructor_input_geo_point_empty (struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x17768f1f && T->type->name != 0xe88970e0)) { return -1; }
  eprintf (" inputGeoPointEmpty");
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int fetch_constructor_input_geo_point (struct paramed_type *T) {
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
  if (fetch_type_bare_double (field1) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" longitude :"); }
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xddef3eab, .id = "Bare_Double", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  if (fetch_type_bare_double (field2) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int fetch_constructor_input_media_empty (struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0xb32c91dd && T->type->name != 0x4cd36e22)) { return -1; }
  eprintf (" inputMediaEmpty");
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int fetch_constructor_input_media_uploaded_photo (struct paramed_type *T) {
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
  if (fetch_type_input_file (field1) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" caption :"); }
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  if (fetch_type_bare_string (field2) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int fetch_constructor_input_media_photo (struct paramed_type *T) {
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
  if (fetch_type_input_photo (field1) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" caption :"); }
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  if (fetch_type_bare_string (field2) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int fetch_constructor_input_media_geo_point (struct paramed_type *T) {
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
  if (fetch_type_input_geo_point (field1) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int fetch_constructor_input_media_contact (struct paramed_type *T) {
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
  if (fetch_type_bare_string (field1) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" first_name :"); }
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  if (fetch_type_bare_string (field2) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" last_name :"); }
  struct paramed_type *field3 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  if (fetch_type_bare_string (field3) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int fetch_constructor_input_media_uploaded_video (struct paramed_type *T) {
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
  if (fetch_type_input_file (field1) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" duration :"); }
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  if (fetch_type_bare_int (field2) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" w :"); }
  struct paramed_type *field3 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  if (fetch_type_bare_int (field3) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" h :"); }
  struct paramed_type *field4 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  if (fetch_type_bare_int (field4) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" caption :"); }
  struct paramed_type *field5 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  if (fetch_type_bare_string (field5) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int fetch_constructor_input_media_uploaded_thumb_video (struct paramed_type *T) {
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
  if (fetch_type_input_file (field1) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" thumb :"); }
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x0f60f9ca, .id = "InputFile", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  if (fetch_type_input_file (field2) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" duration :"); }
  struct paramed_type *field3 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  if (fetch_type_bare_int (field3) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" w :"); }
  struct paramed_type *field4 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  if (fetch_type_bare_int (field4) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" h :"); }
  struct paramed_type *field5 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  if (fetch_type_bare_int (field5) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" caption :"); }
  struct paramed_type *field6 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  if (fetch_type_bare_string (field6) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int fetch_constructor_input_media_video (struct paramed_type *T) {
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
  if (fetch_type_input_video (field1) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" caption :"); }
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  if (fetch_type_bare_string (field2) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int fetch_constructor_input_media_uploaded_audio (struct paramed_type *T) {
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
  if (fetch_type_input_file (field1) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" duration :"); }
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  if (fetch_type_bare_int (field2) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" mime_type :"); }
  struct paramed_type *field3 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  if (fetch_type_bare_string (field3) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int fetch_constructor_input_media_audio (struct paramed_type *T) {
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
  if (fetch_type_input_audio (field1) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int fetch_constructor_input_media_uploaded_document (struct paramed_type *T) {
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
  if (fetch_type_input_file (field1) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" mime_type :"); }
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  if (fetch_type_bare_string (field2) < 0) { return -1;}
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
  if (fetch_type_vector (field3) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int fetch_constructor_input_media_uploaded_thumb_document (struct paramed_type *T) {
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
  if (fetch_type_input_file (field1) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" thumb :"); }
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x0f60f9ca, .id = "InputFile", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  if (fetch_type_input_file (field2) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" mime_type :"); }
  struct paramed_type *field3 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  if (fetch_type_bare_string (field3) < 0) { return -1;}
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
  if (fetch_type_vector (field4) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int fetch_constructor_input_media_document (struct paramed_type *T) {
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
  if (fetch_type_input_document (field1) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int fetch_constructor_input_media_venue (struct paramed_type *T) {
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
  if (fetch_type_input_geo_point (field1) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" title :"); }
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  if (fetch_type_bare_string (field2) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" address :"); }
  struct paramed_type *field3 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  if (fetch_type_bare_string (field3) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" provider :"); }
  struct paramed_type *field4 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  if (fetch_type_bare_string (field4) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" venue_id :"); }
  struct paramed_type *field5 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  if (fetch_type_bare_string (field5) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int fetch_constructor_input_notify_peer (struct paramed_type *T) {
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
  if (fetch_type_input_peer (field1) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int fetch_constructor_input_notify_users (struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x02b6911b && T->type->name != 0xfd496ee4)) { return -1; }
  eprintf (" inputNotifyUsers");
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int fetch_constructor_input_notify_chats (struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x02b6911b && T->type->name != 0xfd496ee4)) { return -1; }
  eprintf (" inputNotifyChats");
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int fetch_constructor_input_notify_all (struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x02b6911b && T->type->name != 0xfd496ee4)) { return -1; }
  eprintf (" inputNotifyAll");
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int fetch_constructor_input_notify_geo_chat_peer (struct paramed_type *T) {
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
  if (fetch_type_input_geo_chat (field1) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int fetch_constructor_input_peer_empty (struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x9e67268d && T->type->name != 0x6198d972)) { return -1; }
  eprintf (" inputPeerEmpty");
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int fetch_constructor_input_peer_self (struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x9e67268d && T->type->name != 0x6198d972)) { return -1; }
  eprintf (" inputPeerSelf");
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int fetch_constructor_input_peer_contact (struct paramed_type *T) {
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
  if (fetch_type_bare_int (field1) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int fetch_constructor_input_peer_foreign (struct paramed_type *T) {
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
  if (fetch_type_bare_int (field1) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" access_hash :"); }
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xddf89345, .id = "Bare_Long", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  if (fetch_type_bare_long (field2) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int fetch_constructor_input_peer_chat (struct paramed_type *T) {
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
  if (fetch_type_bare_int (field1) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int fetch_constructor_input_peer_notify_events_empty (struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x185a48ac && T->type->name != 0xe7a5b753)) { return -1; }
  eprintf (" inputPeerNotifyEventsEmpty");
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int fetch_constructor_input_peer_notify_events_all (struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x185a48ac && T->type->name != 0xe7a5b753)) { return -1; }
  eprintf (" inputPeerNotifyEventsAll");
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int fetch_constructor_input_peer_notify_settings (struct paramed_type *T) {
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
  if (fetch_type_bare_int (field1) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" sound :"); }
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  if (fetch_type_bare_string (field2) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" show_previews :"); }
  struct paramed_type *field3 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x250be282, .id = "Bool", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  if (fetch_type_bool (field3) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" events_mask :"); }
  struct paramed_type *field4 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  if (fetch_type_bare_int (field4) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int fetch_constructor_input_photo_empty (struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0xe74279c9 && T->type->name != 0x18bd8636)) { return -1; }
  eprintf (" inputPhotoEmpty");
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int fetch_constructor_input_photo (struct paramed_type *T) {
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
  if (fetch_type_bare_long (field1) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" access_hash :"); }
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xddf89345, .id = "Bare_Long", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  if (fetch_type_bare_long (field2) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int fetch_constructor_input_photo_crop_auto (struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x7477e321 && T->type->name != 0x8b881cde)) { return -1; }
  eprintf (" inputPhotoCropAuto");
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int fetch_constructor_input_photo_crop (struct paramed_type *T) {
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
  if (fetch_type_bare_double (field1) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" crop_top :"); }
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xddef3eab, .id = "Bare_Double", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  if (fetch_type_bare_double (field2) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" crop_width :"); }
  struct paramed_type *field3 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xddef3eab, .id = "Bare_Double", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  if (fetch_type_bare_double (field3) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int fetch_constructor_input_privacy_key_status_timestamp (struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x4f96cb18 && T->type->name != 0xb06934e7)) { return -1; }
  eprintf (" inputPrivacyKeyStatusTimestamp");
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int fetch_constructor_input_privacy_value_allow_contacts (struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x4b815163 && T->type->name != 0xb47eae9c)) { return -1; }
  eprintf (" inputPrivacyValueAllowContacts");
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int fetch_constructor_input_privacy_value_allow_all (struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x4b815163 && T->type->name != 0xb47eae9c)) { return -1; }
  eprintf (" inputPrivacyValueAllowAll");
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int fetch_constructor_input_privacy_value_allow_users (struct paramed_type *T) {
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
  if (fetch_type_vector (field1) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int fetch_constructor_input_privacy_value_disallow_contacts (struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x4b815163 && T->type->name != 0xb47eae9c)) { return -1; }
  eprintf (" inputPrivacyValueDisallowContacts");
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int fetch_constructor_input_privacy_value_disallow_all (struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x4b815163 && T->type->name != 0xb47eae9c)) { return -1; }
  eprintf (" inputPrivacyValueDisallowAll");
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int fetch_constructor_input_privacy_value_disallow_users (struct paramed_type *T) {
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
  if (fetch_type_vector (field1) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int fetch_constructor_input_sticker_set_empty (struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0xe44d415c && T->type->name != 0x1bb2bea3)) { return -1; }
  eprintf (" inputStickerSetEmpty");
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int fetch_constructor_input_sticker_set_i_d (struct paramed_type *T) {
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
  if (fetch_type_bare_long (field1) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" access_hash :"); }
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xddf89345, .id = "Bare_Long", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  if (fetch_type_bare_long (field2) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int fetch_constructor_input_sticker_set_short_name (struct paramed_type *T) {
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
  if (fetch_type_bare_string (field1) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int fetch_constructor_input_user_empty (struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0xadfe0c6a && T->type->name != 0x5201f395)) { return -1; }
  eprintf (" inputUserEmpty");
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int fetch_constructor_input_user_self (struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0xadfe0c6a && T->type->name != 0x5201f395)) { return -1; }
  eprintf (" inputUserSelf");
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int fetch_constructor_input_user_contact (struct paramed_type *T) {
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
  if (fetch_type_bare_int (field1) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int fetch_constructor_input_user_foreign (struct paramed_type *T) {
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
  if (fetch_type_bare_int (field1) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" access_hash :"); }
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xddf89345, .id = "Bare_Long", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  if (fetch_type_bare_long (field2) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int fetch_constructor_input_video_empty (struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0xbb5f7a27 && T->type->name != 0x44a085d8)) { return -1; }
  eprintf (" inputVideoEmpty");
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int fetch_constructor_input_video (struct paramed_type *T) {
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
  if (fetch_type_bare_long (field1) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" access_hash :"); }
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xddf89345, .id = "Bare_Long", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  if (fetch_type_bare_long (field2) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int fetch_constructor_int (struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0xa8509bda && T->type->name != 0x57af6425)) { return -1; }
  if (in_remaining () < 4) { return -1;}
  eprintf (" %d", fetch_int ());
  return 0;
}
int fetch_constructor_int128 (struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x7d36c439 && T->type->name != 0x82c93bc6)) { return -1; }
  eprintf (" int128");
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xddf89345, .id = "Bare_Long", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  if (fetch_type_bare_long (field1) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xddf89345, .id = "Bare_Long", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  if (fetch_type_bare_long (field2) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int fetch_constructor_int256 (struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0xf2c798b3 && T->type->name != 0x0d38674c)) { return -1; }
  eprintf (" int256");
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xddf89345, .id = "Bare_Long", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  if (fetch_type_bare_long (field1) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xddf89345, .id = "Bare_Long", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  if (fetch_type_bare_long (field2) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  struct paramed_type *field3 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xddf89345, .id = "Bare_Long", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  if (fetch_type_bare_long (field3) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  struct paramed_type *field4 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xddf89345, .id = "Bare_Long", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  if (fetch_type_bare_long (field4) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int fetch_constructor_keyboard_button (struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0xa2fa4880 && T->type->name != 0x5d05b77f)) { return -1; }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  if (fetch_type_bare_string (field1) < 0) { return -1;}
  return 0;
}
int fetch_constructor_keyboard_button_row (struct paramed_type *T) {
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
  if (fetch_type_vector (field1) < 0) { return -1;}
  return 0;
}
int fetch_constructor_long (struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x22076cba && T->type->name != 0xddf89345)) { return -1; }
  if (in_remaining () < 8) { return -1;}
  eprintf (" %I64d", fetch_long ());
  return 0;
}
int fetch_constructor_message_empty (struct paramed_type *T) {
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
  if (fetch_type_bare_int (field1) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int fetch_constructor_message (struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x5d652a7f && T->type->name != 0xa29ad580)) { return -1; }
  eprintf (" message");
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" flags :"); }
  if (in_remaining () < 4) { return -1;}
  eprintf (" %d", prefetch_int ());
  struct paramed_type *var0 = INT2PTR (fetch_int ());
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" id :"); }
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  if (fetch_type_bare_int (field2) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" from_id :"); }
  struct paramed_type *field3 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  if (fetch_type_bare_int (field3) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" to_id :"); }
  struct paramed_type *field4 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x276159d6, .id = "Peer", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  if (fetch_type_peer (field4) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (PTR2INT (var0) & (1 << 2)) {
    if (multiline_output >= 2) { print_offset (); }
    if (!disable_field_names) { eprintf (" fwd_from_id :"); }
    struct paramed_type *field5 = 
    &(struct paramed_type){
      .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
      .params = 0,
    };
    if (fetch_type_bare_int (field5) < 0) { return -1;}
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
    if (fetch_type_bare_int (field6) < 0) { return -1;}
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
    if (fetch_type_bare_int (field7) < 0) { return -1;}
    if (multiline_output >= 2) { eprintf ("\n"); }
  }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" date :"); }
  struct paramed_type *field8 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  if (fetch_type_bare_int (field8) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" message :"); }
  struct paramed_type *field9 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  if (fetch_type_bare_string (field9) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" media :"); }
  struct paramed_type *field10 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x95f2471a, .id = "MessageMedia", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  if (fetch_type_message_media (field10) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (PTR2INT (var0) & (1 << 6)) {
    if (multiline_output >= 2) { print_offset (); }
    if (!disable_field_names) { eprintf (" reply_markup :"); }
    struct paramed_type *field11 = 
    &(struct paramed_type){
      .type = &(struct tl_type_descr) {.name = 0x612ca4a9, .id = "ReplyMarkup", .params_num = 0, .params_types = 0},
      .params = 0,
    };
    if (fetch_type_reply_markup (field11) < 0) { return -1;}
    if (multiline_output >= 2) { eprintf ("\n"); }
  }
  return 0;
}
int fetch_constructor_message_service (struct paramed_type *T) {
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
  if (fetch_type_bare_int (field1) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" id :"); }
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  if (fetch_type_bare_int (field2) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" from_id :"); }
  struct paramed_type *field3 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  if (fetch_type_bare_int (field3) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" to_id :"); }
  struct paramed_type *field4 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x276159d6, .id = "Peer", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  if (fetch_type_peer (field4) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" date :"); }
  struct paramed_type *field5 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  if (fetch_type_bare_int (field5) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" action :"); }
  struct paramed_type *field6 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x383415fa, .id = "MessageAction", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  if (fetch_type_message_action (field6) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int fetch_constructor_message_action_empty (struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x383415fa && T->type->name != 0xc7cbea05)) { return -1; }
  eprintf (" messageActionEmpty");
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int fetch_constructor_message_action_chat_create (struct paramed_type *T) {
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
  if (fetch_type_bare_string (field1) < 0) { return -1;}
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
  if (fetch_type_vector (field2) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int fetch_constructor_message_action_chat_edit_title (struct paramed_type *T) {
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
  if (fetch_type_bare_string (field1) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int fetch_constructor_message_action_chat_edit_photo (struct paramed_type *T) {
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
  if (fetch_type_photo (field1) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int fetch_constructor_message_action_chat_delete_photo (struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x383415fa && T->type->name != 0xc7cbea05)) { return -1; }
  eprintf (" messageActionChatDeletePhoto");
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int fetch_constructor_message_action_chat_add_user (struct paramed_type *T) {
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
  if (fetch_type_bare_int (field1) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int fetch_constructor_message_action_chat_delete_user (struct paramed_type *T) {
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
  if (fetch_type_bare_int (field1) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int fetch_constructor_message_action_geo_chat_create (struct paramed_type *T) {
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
  if (fetch_type_bare_string (field1) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" address :"); }
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  if (fetch_type_bare_string (field2) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int fetch_constructor_message_action_geo_chat_checkin (struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x383415fa && T->type->name != 0xc7cbea05)) { return -1; }
  eprintf (" messageActionGeoChatCheckin");
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int fetch_constructor_message_action_chat_joined_by_link (struct paramed_type *T) {
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
  if (fetch_type_bare_int (field1) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int fetch_constructor_message_media_empty (struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x95f2471a && T->type->name != 0x6a0db8e5)) { return -1; }
  eprintf (" messageMediaEmpty");
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int fetch_constructor_message_media_photo (struct paramed_type *T) {
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
  if (fetch_type_photo (field1) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" caption :"); }
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  if (fetch_type_bare_string (field2) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int fetch_constructor_message_media_video (struct paramed_type *T) {
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
  if (fetch_type_video (field1) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" caption :"); }
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  if (fetch_type_bare_string (field2) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int fetch_constructor_message_media_geo (struct paramed_type *T) {
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
  if (fetch_type_geo_point (field1) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int fetch_constructor_message_media_contact (struct paramed_type *T) {
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
  if (fetch_type_bare_string (field1) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" first_name :"); }
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  if (fetch_type_bare_string (field2) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" last_name :"); }
  struct paramed_type *field3 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  if (fetch_type_bare_string (field3) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" user_id :"); }
  struct paramed_type *field4 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  if (fetch_type_bare_int (field4) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int fetch_constructor_message_media_unsupported (struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x95f2471a && T->type->name != 0x6a0db8e5)) { return -1; }
  eprintf (" messageMediaUnsupported");
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int fetch_constructor_message_media_document (struct paramed_type *T) {
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
  if (fetch_type_document (field1) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int fetch_constructor_message_media_audio (struct paramed_type *T) {
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
  if (fetch_type_audio (field1) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int fetch_constructor_message_media_web_page (struct paramed_type *T) {
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
  if (fetch_type_web_page (field1) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int fetch_constructor_message_media_venue (struct paramed_type *T) {
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
  if (fetch_type_geo_point (field1) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" title :"); }
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  if (fetch_type_bare_string (field2) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" address :"); }
  struct paramed_type *field3 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  if (fetch_type_bare_string (field3) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" provider :"); }
  struct paramed_type *field4 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  if (fetch_type_bare_string (field4) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" venue_id :"); }
  struct paramed_type *field5 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  if (fetch_type_bare_string (field5) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int fetch_constructor_message_media_photo_l27 (struct paramed_type *T) {
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
  if (fetch_type_photo (field1) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int fetch_constructor_message_media_video_l27 (struct paramed_type *T) {
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
  if (fetch_type_video (field1) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int fetch_constructor_input_messages_filter_empty (struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x80895ae0 && T->type->name != 0x7f76a51f)) { return -1; }
  eprintf (" inputMessagesFilterEmpty");
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int fetch_constructor_input_messages_filter_photos (struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x80895ae0 && T->type->name != 0x7f76a51f)) { return -1; }
  eprintf (" inputMessagesFilterPhotos");
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int fetch_constructor_input_messages_filter_video (struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x80895ae0 && T->type->name != 0x7f76a51f)) { return -1; }
  eprintf (" inputMessagesFilterVideo");
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int fetch_constructor_input_messages_filter_photo_video (struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x80895ae0 && T->type->name != 0x7f76a51f)) { return -1; }
  eprintf (" inputMessagesFilterPhotoVideo");
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int fetch_constructor_input_messages_filter_photo_video_documents (struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x80895ae0 && T->type->name != 0x7f76a51f)) { return -1; }
  eprintf (" inputMessagesFilterPhotoVideoDocuments");
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int fetch_constructor_input_messages_filter_document (struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x80895ae0 && T->type->name != 0x7f76a51f)) { return -1; }
  eprintf (" inputMessagesFilterDocument");
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int fetch_constructor_input_messages_filter_audio (struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x80895ae0 && T->type->name != 0x7f76a51f)) { return -1; }
  eprintf (" inputMessagesFilterAudio");
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int fetch_constructor_nearest_dc (struct paramed_type *T) {
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
  if (fetch_type_bare_string (field1) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" this_dc :"); }
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  if (fetch_type_bare_int (field2) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" nearest_dc :"); }
  struct paramed_type *field3 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  if (fetch_type_bare_int (field3) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int fetch_constructor_notify_peer (struct paramed_type *T) {
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
  if (fetch_type_peer (field1) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int fetch_constructor_notify_users (struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x9fcb8237 && T->type->name != 0x60347dc8)) { return -1; }
  eprintf (" notifyUsers");
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int fetch_constructor_notify_chats (struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x9fcb8237 && T->type->name != 0x60347dc8)) { return -1; }
  eprintf (" notifyChats");
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int fetch_constructor_notify_all (struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x9fcb8237 && T->type->name != 0x60347dc8)) { return -1; }
  eprintf (" notifyAll");
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int fetch_constructor_null (struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x56730bcc && T->type->name != 0xa98cf433)) { return -1; }
  eprintf (" null");
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int fetch_constructor_p_q_inner_data (struct paramed_type *T) {
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
  if (fetch_type_bare_string (field1) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" p :"); }
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  if (fetch_type_bare_string (field2) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" q :"); }
  struct paramed_type *field3 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  if (fetch_type_bare_string (field3) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" nonce :"); }
  struct paramed_type *field4 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x82c93bc6, .id = "Bare_Int128", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  if (fetch_type_bare_int128 (field4) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" server_nonce :"); }
  struct paramed_type *field5 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x82c93bc6, .id = "Bare_Int128", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  if (fetch_type_bare_int128 (field5) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" new_nonce :"); }
  struct paramed_type *field6 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x0d38674c, .id = "Bare_Int256", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  if (fetch_type_bare_int256 (field6) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int fetch_constructor_p_q_inner_data_temp (struct paramed_type *T) {
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
  if (fetch_type_bare_string (field1) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" p :"); }
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  if (fetch_type_bare_string (field2) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" q :"); }
  struct paramed_type *field3 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  if (fetch_type_bare_string (field3) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" nonce :"); }
  struct paramed_type *field4 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x82c93bc6, .id = "Bare_Int128", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  if (fetch_type_bare_int128 (field4) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" server_nonce :"); }
  struct paramed_type *field5 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x82c93bc6, .id = "Bare_Int128", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  if (fetch_type_bare_int128 (field5) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" new_nonce :"); }
  struct paramed_type *field6 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x0d38674c, .id = "Bare_Int256", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  if (fetch_type_bare_int256 (field6) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" expires_in :"); }
  struct paramed_type *field7 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  if (fetch_type_bare_int (field7) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int fetch_constructor_peer_user (struct paramed_type *T) {
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
  if (fetch_type_bare_int (field1) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int fetch_constructor_peer_chat (struct paramed_type *T) {
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
  if (fetch_type_bare_int (field1) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int fetch_constructor_peer_notify_events_empty (struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0xc0c8d13b && T->type->name != 0x3f372ec4)) { return -1; }
  eprintf (" peerNotifyEventsEmpty");
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int fetch_constructor_peer_notify_events_all (struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0xc0c8d13b && T->type->name != 0x3f372ec4)) { return -1; }
  eprintf (" peerNotifyEventsAll");
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int fetch_constructor_peer_notify_settings_empty (struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0xfdf894fc && T->type->name != 0x02076b03)) { return -1; }
  eprintf (" peerNotifySettingsEmpty");
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int fetch_constructor_peer_notify_settings (struct paramed_type *T) {
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
  if (fetch_type_bare_int (field1) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" sound :"); }
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  if (fetch_type_bare_string (field2) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" show_previews :"); }
  struct paramed_type *field3 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x250be282, .id = "Bool", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  if (fetch_type_bool (field3) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" events_mask :"); }
  struct paramed_type *field4 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  if (fetch_type_bare_int (field4) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int fetch_constructor_photo_empty (struct paramed_type *T) {
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
  if (fetch_type_bare_long (field1) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int fetch_constructor_photo (struct paramed_type *T) {
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
  if (fetch_type_bare_long (field1) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" access_hash :"); }
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xddf89345, .id = "Bare_Long", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  if (fetch_type_bare_long (field2) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" user_id :"); }
  struct paramed_type *field3 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  if (fetch_type_bare_int (field3) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" date :"); }
  struct paramed_type *field4 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  if (fetch_type_bare_int (field4) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" geo :"); }
  struct paramed_type *field5 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x315e0a53, .id = "GeoPoint", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  if (fetch_type_geo_point (field5) < 0) { return -1;}
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
  if (fetch_type_vector (field6) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int fetch_constructor_photo_l27 (struct paramed_type *T) {
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
  if (fetch_type_bare_long (field1) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" access_hash :"); }
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xddf89345, .id = "Bare_Long", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  if (fetch_type_bare_long (field2) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" user_id :"); }
  struct paramed_type *field3 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  if (fetch_type_bare_int (field3) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" date :"); }
  struct paramed_type *field4 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  if (fetch_type_bare_int (field4) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" caption :"); }
  struct paramed_type *field5 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  if (fetch_type_bare_string (field5) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" geo :"); }
  struct paramed_type *field6 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x315e0a53, .id = "GeoPoint", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  if (fetch_type_geo_point (field6) < 0) { return -1;}
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
  if (fetch_type_vector (field7) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int fetch_constructor_photo_size_empty (struct paramed_type *T) {
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
  if (fetch_type_bare_string (field1) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int fetch_constructor_photo_size (struct paramed_type *T) {
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
  if (fetch_type_bare_string (field1) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" location :"); }
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x2f8ffb30, .id = "FileLocation", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  if (fetch_type_file_location (field2) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" w :"); }
  struct paramed_type *field3 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  if (fetch_type_bare_int (field3) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" h :"); }
  struct paramed_type *field4 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  if (fetch_type_bare_int (field4) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" size :"); }
  struct paramed_type *field5 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  if (fetch_type_bare_int (field5) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int fetch_constructor_photo_cached_size (struct paramed_type *T) {
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
  if (fetch_type_bare_string (field1) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" location :"); }
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x2f8ffb30, .id = "FileLocation", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  if (fetch_type_file_location (field2) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" w :"); }
  struct paramed_type *field3 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  if (fetch_type_bare_int (field3) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" h :"); }
  struct paramed_type *field4 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  if (fetch_type_bare_int (field4) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" bytes :"); }
  struct paramed_type *field5 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xf11ec860, .id = "Bare_Bytes", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  if (fetch_type_bare_bytes (field5) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int fetch_constructor_privacy_key_status_timestamp (struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0xbc2eab30 && T->type->name != 0x43d154cf)) { return -1; }
  eprintf (" privacyKeyStatusTimestamp");
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int fetch_constructor_privacy_value_allow_contacts (struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0xa8638aec && T->type->name != 0x579c7513)) { return -1; }
  eprintf (" privacyValueAllowContacts");
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int fetch_constructor_privacy_value_allow_all (struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0xa8638aec && T->type->name != 0x579c7513)) { return -1; }
  eprintf (" privacyValueAllowAll");
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int fetch_constructor_privacy_value_allow_users (struct paramed_type *T) {
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
  if (fetch_type_vector (field1) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int fetch_constructor_privacy_value_disallow_contacts (struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0xa8638aec && T->type->name != 0x579c7513)) { return -1; }
  eprintf (" privacyValueDisallowContacts");
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int fetch_constructor_privacy_value_disallow_all (struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0xa8638aec && T->type->name != 0x579c7513)) { return -1; }
  eprintf (" privacyValueDisallowAll");
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int fetch_constructor_privacy_value_disallow_users (struct paramed_type *T) {
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
  if (fetch_type_vector (field1) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int fetch_constructor_received_notify_message (struct paramed_type *T) {
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
  if (fetch_type_bare_int (field1) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" flags :"); }
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  if (fetch_type_bare_int (field2) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int fetch_constructor_reply_keyboard_hide (struct paramed_type *T) {
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
  if (fetch_type_bare_int (field1) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int fetch_constructor_reply_keyboard_force_reply (struct paramed_type *T) {
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
  if (fetch_type_bare_int (field1) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int fetch_constructor_reply_keyboard_markup (struct paramed_type *T) {
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
  if (fetch_type_bare_int (field1) < 0) { return -1;}
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
  if (fetch_type_vector (field2) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int fetch_constructor_res_p_q (struct paramed_type *T) {
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
  if (fetch_type_bare_int128 (field1) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" server_nonce :"); }
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x82c93bc6, .id = "Bare_Int128", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  if (fetch_type_bare_int128 (field2) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" pq :"); }
  struct paramed_type *field3 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  if (fetch_type_bare_string (field3) < 0) { return -1;}
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
  if (fetch_type_vector (field4) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int fetch_constructor_send_message_typing_action (struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x70541c7e && T->type->name != 0x8fabe381)) { return -1; }
  eprintf (" sendMessageTypingAction");
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int fetch_constructor_send_message_cancel_action (struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x70541c7e && T->type->name != 0x8fabe381)) { return -1; }
  eprintf (" sendMessageCancelAction");
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int fetch_constructor_send_message_record_video_action (struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x70541c7e && T->type->name != 0x8fabe381)) { return -1; }
  eprintf (" sendMessageRecordVideoAction");
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int fetch_constructor_send_message_upload_video_action_l27 (struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x70541c7e && T->type->name != 0x8fabe381)) { return -1; }
  eprintf (" sendMessageUploadVideoActionL27");
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int fetch_constructor_send_message_upload_video_action (struct paramed_type *T) {
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
  if (fetch_type_bare_int (field1) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int fetch_constructor_send_message_record_audio_action (struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x70541c7e && T->type->name != 0x8fabe381)) { return -1; }
  eprintf (" sendMessageRecordAudioAction");
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int fetch_constructor_send_message_upload_audio_action_l27 (struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x70541c7e && T->type->name != 0x8fabe381)) { return -1; }
  eprintf (" sendMessageUploadAudioActionL27");
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int fetch_constructor_send_message_upload_audio_action (struct paramed_type *T) {
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
  if (fetch_type_bare_int (field1) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int fetch_constructor_send_message_upload_photo_action (struct paramed_type *T) {
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
  if (fetch_type_bare_int (field1) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int fetch_constructor_send_message_upload_document_action_l27 (struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x70541c7e && T->type->name != 0x8fabe381)) { return -1; }
  eprintf (" sendMessageUploadDocumentActionL27");
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int fetch_constructor_send_message_upload_document_action (struct paramed_type *T) {
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
  if (fetch_type_bare_int (field1) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int fetch_constructor_send_message_geo_location_action (struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x70541c7e && T->type->name != 0x8fabe381)) { return -1; }
  eprintf (" sendMessageGeoLocationAction");
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int fetch_constructor_send_message_choose_contact_action (struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x70541c7e && T->type->name != 0x8fabe381)) { return -1; }
  eprintf (" sendMessageChooseContactAction");
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int fetch_constructor_server_d_h_params_fail (struct paramed_type *T) {
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
  if (fetch_type_bare_int128 (field1) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" server_nonce :"); }
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x82c93bc6, .id = "Bare_Int128", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  if (fetch_type_bare_int128 (field2) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" new_nonce_hash :"); }
  struct paramed_type *field3 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x82c93bc6, .id = "Bare_Int128", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  if (fetch_type_bare_int128 (field3) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int fetch_constructor_server_d_h_params_ok (struct paramed_type *T) {
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
  if (fetch_type_bare_int128 (field1) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" server_nonce :"); }
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x82c93bc6, .id = "Bare_Int128", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  if (fetch_type_bare_int128 (field2) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" encrypted_answer :"); }
  struct paramed_type *field3 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  if (fetch_type_bare_string (field3) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int fetch_constructor_server_d_h_inner_data (struct paramed_type *T) {
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
  if (fetch_type_bare_int128 (field1) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" server_nonce :"); }
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x82c93bc6, .id = "Bare_Int128", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  if (fetch_type_bare_int128 (field2) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" g :"); }
  struct paramed_type *field3 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  if (fetch_type_bare_int (field3) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" dh_prime :"); }
  struct paramed_type *field4 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  if (fetch_type_bare_string (field4) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" g_a :"); }
  struct paramed_type *field5 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  if (fetch_type_bare_string (field5) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" server_time :"); }
  struct paramed_type *field6 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  if (fetch_type_bare_int (field6) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int fetch_constructor_dh_gen_ok (struct paramed_type *T) {
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
  if (fetch_type_bare_int128 (field1) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" server_nonce :"); }
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x82c93bc6, .id = "Bare_Int128", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  if (fetch_type_bare_int128 (field2) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" new_nonce_hash1 :"); }
  struct paramed_type *field3 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x82c93bc6, .id = "Bare_Int128", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  if (fetch_type_bare_int128 (field3) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int fetch_constructor_dh_gen_retry (struct paramed_type *T) {
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
  if (fetch_type_bare_int128 (field1) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" server_nonce :"); }
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x82c93bc6, .id = "Bare_Int128", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  if (fetch_type_bare_int128 (field2) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" new_nonce_hash2 :"); }
  struct paramed_type *field3 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x82c93bc6, .id = "Bare_Int128", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  if (fetch_type_bare_int128 (field3) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int fetch_constructor_dh_gen_fail (struct paramed_type *T) {
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
  if (fetch_type_bare_int128 (field1) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" server_nonce :"); }
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x82c93bc6, .id = "Bare_Int128", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  if (fetch_type_bare_int128 (field2) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" new_nonce_hash3 :"); }
  struct paramed_type *field3 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x82c93bc6, .id = "Bare_Int128", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  if (fetch_type_bare_int128 (field3) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int fetch_constructor_sticker_pack (struct paramed_type *T) {
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
  if (fetch_type_bare_string (field1) < 0) { return -1;}
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
  if (fetch_type_vector (field2) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int fetch_constructor_sticker_set (struct paramed_type *T) {
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
  if (fetch_type_bare_long (field1) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" access_hash :"); }
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xddf89345, .id = "Bare_Long", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  if (fetch_type_bare_long (field2) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" title :"); }
  struct paramed_type *field3 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  if (fetch_type_bare_string (field3) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" short_name :"); }
  struct paramed_type *field4 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  if (fetch_type_bare_string (field4) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int fetch_constructor_string (struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0xb5286e24 && T->type->name != 0x4ad791db)) { return -1; }
  static char buf[1 << 22];
  int l = prefetch_strlen ();
  if (l < 0 || (l >= (1 << 22) - 2)) { return -1; }
  memcpy (buf, fetch_str (l), l);
  buf[l] = 0;
  print_escaped_string (buf, l);
  return 0;
}
int fetch_constructor_update_new_message (struct paramed_type *T) {
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
  if (fetch_type_message (field1) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" pts :"); }
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  if (fetch_type_bare_int (field2) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" pts_count :"); }
  struct paramed_type *field3 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  if (fetch_type_bare_int (field3) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int fetch_constructor_update_message_i_d (struct paramed_type *T) {
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
  if (fetch_type_bare_int (field1) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" random_id :"); }
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xddf89345, .id = "Bare_Long", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  if (fetch_type_bare_long (field2) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int fetch_constructor_update_delete_messages (struct paramed_type *T) {
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
  if (fetch_type_vector (field1) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" pts :"); }
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  if (fetch_type_bare_int (field2) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" pts_count :"); }
  struct paramed_type *field3 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  if (fetch_type_bare_int (field3) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int fetch_constructor_update_user_typing (struct paramed_type *T) {
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
  if (fetch_type_bare_int (field1) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" action :"); }
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x70541c7e, .id = "SendMessageAction", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  if (fetch_type_send_message_action (field2) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int fetch_constructor_update_chat_user_typing (struct paramed_type *T) {
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
  if (fetch_type_bare_int (field1) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" user_id :"); }
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  if (fetch_type_bare_int (field2) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" action :"); }
  struct paramed_type *field3 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x70541c7e, .id = "SendMessageAction", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  if (fetch_type_send_message_action (field3) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int fetch_constructor_update_chat_participants (struct paramed_type *T) {
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
  if (fetch_type_chat_participants (field1) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int fetch_constructor_update_user_status (struct paramed_type *T) {
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
  if (fetch_type_bare_int (field1) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" status :"); }
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x76de9570, .id = "UserStatus", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  if (fetch_type_user_status (field2) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int fetch_constructor_update_user_name (struct paramed_type *T) {
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
  if (fetch_type_bare_int (field1) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" first_name :"); }
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  if (fetch_type_bare_string (field2) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" last_name :"); }
  struct paramed_type *field3 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  if (fetch_type_bare_string (field3) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" username :"); }
  struct paramed_type *field4 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  if (fetch_type_bare_string (field4) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int fetch_constructor_update_user_photo (struct paramed_type *T) {
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
  if (fetch_type_bare_int (field1) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" date :"); }
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  if (fetch_type_bare_int (field2) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" photo :"); }
  struct paramed_type *field3 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x9a486229, .id = "UserProfilePhoto", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  if (fetch_type_user_profile_photo (field3) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" previous :"); }
  struct paramed_type *field4 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x250be282, .id = "Bool", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  if (fetch_type_bool (field4) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int fetch_constructor_update_contact_registered (struct paramed_type *T) {
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
  if (fetch_type_bare_int (field1) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" date :"); }
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  if (fetch_type_bare_int (field2) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int fetch_constructor_update_contact_link (struct paramed_type *T) {
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
  if (fetch_type_bare_int (field1) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" my_link :"); }
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x522fbc63, .id = "ContactLink", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  if (fetch_type_contact_link (field2) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" foreign_link :"); }
  struct paramed_type *field3 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x522fbc63, .id = "ContactLink", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  if (fetch_type_contact_link (field3) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int fetch_constructor_update_new_authorization (struct paramed_type *T) {
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
  if (fetch_type_bare_long (field1) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" date :"); }
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  if (fetch_type_bare_int (field2) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" device :"); }
  struct paramed_type *field3 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  if (fetch_type_bare_string (field3) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" location :"); }
  struct paramed_type *field4 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  if (fetch_type_bare_string (field4) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int fetch_constructor_update_new_geo_chat_message (struct paramed_type *T) {
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
  if (fetch_type_geo_chat_message (field1) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int fetch_constructor_update_new_encrypted_message (struct paramed_type *T) {
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
  if (fetch_type_encrypted_message (field1) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" qts :"); }
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  if (fetch_type_bare_int (field2) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int fetch_constructor_update_encrypted_chat_typing (struct paramed_type *T) {
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
  if (fetch_type_bare_int (field1) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int fetch_constructor_update_encryption (struct paramed_type *T) {
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
  if (fetch_type_encrypted_chat (field1) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" date :"); }
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  if (fetch_type_bare_int (field2) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int fetch_constructor_update_encrypted_messages_read (struct paramed_type *T) {
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
  if (fetch_type_bare_int (field1) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" max_date :"); }
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  if (fetch_type_bare_int (field2) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" date :"); }
  struct paramed_type *field3 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  if (fetch_type_bare_int (field3) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int fetch_constructor_update_chat_participant_add (struct paramed_type *T) {
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
  if (fetch_type_bare_int (field1) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" user_id :"); }
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  if (fetch_type_bare_int (field2) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" inviter_id :"); }
  struct paramed_type *field3 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  if (fetch_type_bare_int (field3) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" version :"); }
  struct paramed_type *field4 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  if (fetch_type_bare_int (field4) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int fetch_constructor_update_chat_participant_delete (struct paramed_type *T) {
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
  if (fetch_type_bare_int (field1) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" user_id :"); }
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  if (fetch_type_bare_int (field2) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" version :"); }
  struct paramed_type *field3 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  if (fetch_type_bare_int (field3) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int fetch_constructor_update_dc_options (struct paramed_type *T) {
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
  if (fetch_type_vector (field1) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int fetch_constructor_update_user_blocked (struct paramed_type *T) {
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
  if (fetch_type_bare_int (field1) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" blocked :"); }
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x250be282, .id = "Bool", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  if (fetch_type_bool (field2) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int fetch_constructor_update_notify_settings (struct paramed_type *T) {
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
  if (fetch_type_notify_peer (field1) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" notify_settings :"); }
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xfdf894fc, .id = "PeerNotifySettings", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  if (fetch_type_peer_notify_settings (field2) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int fetch_constructor_update_service_notification (struct paramed_type *T) {
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
  if (fetch_type_bare_string (field1) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" message_text :"); }
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  if (fetch_type_bare_string (field2) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" media :"); }
  struct paramed_type *field3 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x95f2471a, .id = "MessageMedia", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  if (fetch_type_message_media (field3) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" popup :"); }
  struct paramed_type *field4 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x250be282, .id = "Bool", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  if (fetch_type_bool (field4) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int fetch_constructor_update_privacy (struct paramed_type *T) {
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
  if (fetch_type_privacy_key (field1) < 0) { return -1;}
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
  if (fetch_type_vector (field2) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int fetch_constructor_update_user_phone (struct paramed_type *T) {
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
  if (fetch_type_bare_int (field1) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" phone :"); }
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  if (fetch_type_bare_string (field2) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int fetch_constructor_update_read_history_inbox (struct paramed_type *T) {
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
  if (fetch_type_peer (field1) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" max_id :"); }
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  if (fetch_type_bare_int (field2) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" pts :"); }
  struct paramed_type *field3 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  if (fetch_type_bare_int (field3) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" pts_count :"); }
  struct paramed_type *field4 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  if (fetch_type_bare_int (field4) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int fetch_constructor_update_read_history_outbox (struct paramed_type *T) {
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
  if (fetch_type_peer (field1) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" max_id :"); }
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  if (fetch_type_bare_int (field2) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" pts :"); }
  struct paramed_type *field3 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  if (fetch_type_bare_int (field3) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" pts_count :"); }
  struct paramed_type *field4 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  if (fetch_type_bare_int (field4) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int fetch_constructor_update_web_page (struct paramed_type *T) {
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
  if (fetch_type_web_page (field1) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int fetch_constructor_update_read_messages_contents (struct paramed_type *T) {
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
  if (fetch_type_vector (field1) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" pts :"); }
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  if (fetch_type_bare_int (field2) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" pts_count :"); }
  struct paramed_type *field3 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  if (fetch_type_bare_int (field3) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int fetch_constructor_update_msg_update (struct paramed_type *T) {
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
  if (fetch_type_bare_int (field1) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" pts :"); }
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  if (fetch_type_bare_int (field2) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" pts_count :"); }
  struct paramed_type *field3 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  if (fetch_type_bare_int (field3) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int fetch_constructor_updates_too_long (struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x22499d27 && T->type->name != 0xddb662d8)) { return -1; }
  eprintf (" updatesTooLong");
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int fetch_constructor_update_short_message (struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x22499d27 && T->type->name != 0xddb662d8)) { return -1; }
  eprintf (" updateShortMessage");
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" flags :"); }
  if (in_remaining () < 4) { return -1;}
  eprintf (" %d", prefetch_int ());
  struct paramed_type *var0 = INT2PTR (fetch_int ());
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" id :"); }
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  if (fetch_type_bare_int (field2) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" user_id :"); }
  struct paramed_type *field3 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  if (fetch_type_bare_int (field3) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" message :"); }
  struct paramed_type *field4 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  if (fetch_type_bare_string (field4) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" pts :"); }
  struct paramed_type *field5 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  if (fetch_type_bare_int (field5) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" pts_count :"); }
  struct paramed_type *field6 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  if (fetch_type_bare_int (field6) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" date :"); }
  struct paramed_type *field7 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  if (fetch_type_bare_int (field7) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (PTR2INT (var0) & (1 << 2)) {
    if (multiline_output >= 2) { print_offset (); }
    if (!disable_field_names) { eprintf (" fwd_from_id :"); }
    struct paramed_type *field8 = 
    &(struct paramed_type){
      .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
      .params = 0,
    };
    if (fetch_type_bare_int (field8) < 0) { return -1;}
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
    if (fetch_type_bare_int (field9) < 0) { return -1;}
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
    if (fetch_type_bare_int (field10) < 0) { return -1;}
    if (multiline_output >= 2) { eprintf ("\n"); }
  }
  return 0;
}
int fetch_constructor_update_short_chat_message (struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x22499d27 && T->type->name != 0xddb662d8)) { return -1; }
  eprintf (" updateShortChatMessage");
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" flags :"); }
  if (in_remaining () < 4) { return -1;}
  eprintf (" %d", prefetch_int ());
  struct paramed_type *var0 = INT2PTR (fetch_int ());
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" id :"); }
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  if (fetch_type_bare_int (field2) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" from_id :"); }
  struct paramed_type *field3 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  if (fetch_type_bare_int (field3) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" chat_id :"); }
  struct paramed_type *field4 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  if (fetch_type_bare_int (field4) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" message :"); }
  struct paramed_type *field5 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  if (fetch_type_bare_string (field5) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" pts :"); }
  struct paramed_type *field6 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  if (fetch_type_bare_int (field6) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" pts_count :"); }
  struct paramed_type *field7 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  if (fetch_type_bare_int (field7) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" date :"); }
  struct paramed_type *field8 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  if (fetch_type_bare_int (field8) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (PTR2INT (var0) & (1 << 2)) {
    if (multiline_output >= 2) { print_offset (); }
    if (!disable_field_names) { eprintf (" fwd_from_id :"); }
    struct paramed_type *field9 = 
    &(struct paramed_type){
      .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
      .params = 0,
    };
    if (fetch_type_bare_int (field9) < 0) { return -1;}
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
    if (fetch_type_bare_int (field10) < 0) { return -1;}
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
    if (fetch_type_bare_int (field11) < 0) { return -1;}
    if (multiline_output >= 2) { eprintf ("\n"); }
  }
  return 0;
}
int fetch_constructor_update_short (struct paramed_type *T) {
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
  if (fetch_type_update (field1) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" date :"); }
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  if (fetch_type_bare_int (field2) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int fetch_constructor_updates_combined (struct paramed_type *T) {
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
  if (fetch_type_vector (field1) < 0) { return -1;}
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
  if (fetch_type_vector (field2) < 0) { return -1;}
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
  if (fetch_type_vector (field3) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" date :"); }
  struct paramed_type *field4 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  if (fetch_type_bare_int (field4) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" seq_start :"); }
  struct paramed_type *field5 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  if (fetch_type_bare_int (field5) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" seq :"); }
  struct paramed_type *field6 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  if (fetch_type_bare_int (field6) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int fetch_constructor_updates (struct paramed_type *T) {
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
  if (fetch_type_vector (field1) < 0) { return -1;}
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
  if (fetch_type_vector (field2) < 0) { return -1;}
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
  if (fetch_type_vector (field3) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" date :"); }
  struct paramed_type *field4 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  if (fetch_type_bare_int (field4) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" seq :"); }
  struct paramed_type *field5 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  if (fetch_type_bare_int (field5) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int fetch_constructor_user_empty (struct paramed_type *T) {
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
  if (fetch_type_bare_int (field1) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int fetch_constructor_user (struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x02e6c0c8 && T->type->name != 0xfd193f37)) { return -1; }
  eprintf (" user");
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" flags :"); }
  if (in_remaining () < 4) { return -1;}
  eprintf (" %d", prefetch_int ());
  struct paramed_type *var0 = INT2PTR (fetch_int ());
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" id :"); }
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  if (fetch_type_bare_int (field2) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (PTR2INT (var0) & (1 << 0)) {
    if (multiline_output >= 2) { print_offset (); }
    if (!disable_field_names) { eprintf (" access_hash :"); }
    struct paramed_type *field3 = 
    &(struct paramed_type){
      .type = &(struct tl_type_descr) {.name = 0xddf89345, .id = "Bare_Long", .params_num = 0, .params_types = 0},
      .params = 0,
    };
    if (fetch_type_bare_long (field3) < 0) { return -1;}
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
    if (fetch_type_bare_string (field4) < 0) { return -1;}
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
    if (fetch_type_bare_string (field5) < 0) { return -1;}
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
    if (fetch_type_bare_string (field6) < 0) { return -1;}
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
    if (fetch_type_bare_string (field7) < 0) { return -1;}
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
    if (fetch_type_user_profile_photo (field8) < 0) { return -1;}
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
    if (fetch_type_user_status (field9) < 0) { return -1;}
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
    if (fetch_type_bare_int (field10) < 0) { return -1;}
    if (multiline_output >= 2) { eprintf ("\n"); }
  }
  return 0;
}
int fetch_constructor_user_full (struct paramed_type *T) {
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
  if (fetch_type_user (field1) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" link :"); }
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x3ace484c, .id = "contacts.Link", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  if (fetch_type_contacts_link (field2) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" profile_photo :"); }
  struct paramed_type *field3 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xc207550a, .id = "Photo", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  if (fetch_type_photo (field3) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" notify_settings :"); }
  struct paramed_type *field4 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xfdf894fc, .id = "PeerNotifySettings", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  if (fetch_type_peer_notify_settings (field4) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" blocked :"); }
  struct paramed_type *field5 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x250be282, .id = "Bool", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  if (fetch_type_bool (field5) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" bot_info :"); }
  struct paramed_type *field6 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xb2e16f93, .id = "BotInfo", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  if (fetch_type_bot_info (field6) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int fetch_constructor_user_profile_photo_empty (struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x9a486229 && T->type->name != 0x65b79dd6)) { return -1; }
  eprintf (" userProfilePhotoEmpty");
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int fetch_constructor_user_profile_photo (struct paramed_type *T) {
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
  if (fetch_type_bare_long (field1) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" photo_small :"); }
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x2f8ffb30, .id = "FileLocation", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  if (fetch_type_file_location (field2) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" photo_big :"); }
  struct paramed_type *field3 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x2f8ffb30, .id = "FileLocation", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  if (fetch_type_file_location (field3) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int fetch_constructor_user_status_empty (struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x76de9570 && T->type->name != 0x89216a8f)) { return -1; }
  eprintf (" userStatusEmpty");
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int fetch_constructor_user_status_online (struct paramed_type *T) {
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
  if (fetch_type_bare_int (field1) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int fetch_constructor_user_status_offline (struct paramed_type *T) {
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
  if (fetch_type_bare_int (field1) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int fetch_constructor_user_status_recently (struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x76de9570 && T->type->name != 0x89216a8f)) { return -1; }
  eprintf (" userStatusRecently");
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int fetch_constructor_user_status_last_week (struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x76de9570 && T->type->name != 0x89216a8f)) { return -1; }
  eprintf (" userStatusLastWeek");
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int fetch_constructor_user_status_last_month (struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x76de9570 && T->type->name != 0x89216a8f)) { return -1; }
  eprintf (" userStatusLastMonth");
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int fetch_constructor_vector (struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x1cb5c415 && T->type->name != 0xe34a3bea)) { return -1; }
  if (ODDP(T->params[0])) { return -1; }
  struct paramed_type *var0 = T->params[0]; assert (var0);
  eprintf (" vector");
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (in_remaining () < 4) { return -1;}
  eprintf (" %d", prefetch_int ());
  struct paramed_type *var1 = INT2PTR (fetch_int ());
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  int multiplicity3 = PTR2INT (
  ((void *)var1) + 0  );
  struct paramed_type *field3 = 
  var0;
  eprintf (" [");
  if (multiline_output >= 1) { eprintf ("\n"); }
  if (multiline_output >= 1) { multiline_offset += multiline_offset_size;}
  while (multiplicity3 -- > 0) {
    if (multiline_output >= 1) { print_offset (); }
    if (fetch_type_any (field3) < 0) { return -1;}
    if (multiline_output >= 1) { eprintf ("\n"); }
  }
  if (multiline_output >= 1) { multiline_offset -= multiline_offset_size; print_offset ();}
  eprintf (" ]");
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int fetch_constructor_video_empty (struct paramed_type *T) {
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
  if (fetch_type_bare_long (field1) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int fetch_constructor_video (struct paramed_type *T) {
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
  if (fetch_type_bare_long (field1) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" access_hash :"); }
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xddf89345, .id = "Bare_Long", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  if (fetch_type_bare_long (field2) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" user_id :"); }
  struct paramed_type *field3 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  if (fetch_type_bare_int (field3) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" date :"); }
  struct paramed_type *field4 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  if (fetch_type_bare_int (field4) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" duration :"); }
  struct paramed_type *field5 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  if (fetch_type_bare_int (field5) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" size :"); }
  struct paramed_type *field6 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  if (fetch_type_bare_int (field6) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" thumb :"); }
  struct paramed_type *field7 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x900f60dd, .id = "PhotoSize", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  if (fetch_type_photo_size (field7) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" dc_id :"); }
  struct paramed_type *field8 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  if (fetch_type_bare_int (field8) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" w :"); }
  struct paramed_type *field9 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  if (fetch_type_bare_int (field9) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" h :"); }
  struct paramed_type *field10 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  if (fetch_type_bare_int (field10) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int fetch_constructor_video_l27 (struct paramed_type *T) {
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
  if (fetch_type_bare_long (field1) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" access_hash :"); }
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xddf89345, .id = "Bare_Long", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  if (fetch_type_bare_long (field2) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" user_id :"); }
  struct paramed_type *field3 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  if (fetch_type_bare_int (field3) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" date :"); }
  struct paramed_type *field4 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  if (fetch_type_bare_int (field4) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" caption :"); }
  struct paramed_type *field5 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  if (fetch_type_bare_string (field5) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" duration :"); }
  struct paramed_type *field6 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  if (fetch_type_bare_int (field6) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" mime_type :"); }
  struct paramed_type *field7 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  if (fetch_type_bare_string (field7) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" size :"); }
  struct paramed_type *field8 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  if (fetch_type_bare_int (field8) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" thumb :"); }
  struct paramed_type *field9 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x900f60dd, .id = "PhotoSize", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  if (fetch_type_photo_size (field9) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" dc_id :"); }
  struct paramed_type *field10 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  if (fetch_type_bare_int (field10) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" w :"); }
  struct paramed_type *field11 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  if (fetch_type_bare_int (field11) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" h :"); }
  struct paramed_type *field12 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  if (fetch_type_bare_int (field12) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int fetch_constructor_wall_paper (struct paramed_type *T) {
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
  if (fetch_type_bare_int (field1) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" title :"); }
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  if (fetch_type_bare_string (field2) < 0) { return -1;}
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
  if (fetch_type_vector (field3) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" color :"); }
  struct paramed_type *field4 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  if (fetch_type_bare_int (field4) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int fetch_constructor_wall_paper_solid (struct paramed_type *T) {
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
  if (fetch_type_bare_int (field1) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" title :"); }
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  if (fetch_type_bare_string (field2) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" bg_color :"); }
  struct paramed_type *field3 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  if (fetch_type_bare_int (field3) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" color :"); }
  struct paramed_type *field4 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  if (fetch_type_bare_int (field4) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int fetch_constructor_web_page_empty (struct paramed_type *T) {
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
  if (fetch_type_bare_long (field1) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int fetch_constructor_web_page_pending (struct paramed_type *T) {
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
  if (fetch_type_bare_long (field1) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" date :"); }
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  if (fetch_type_bare_int (field2) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int fetch_constructor_web_page (struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x8d8c0d41 && T->type->name != 0x7273f2be)) { return -1; }
  eprintf (" webPage");
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" flags :"); }
  if (in_remaining () < 4) { return -1;}
  eprintf (" %d", prefetch_int ());
  struct paramed_type *var0 = INT2PTR (fetch_int ());
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" id :"); }
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xddf89345, .id = "Bare_Long", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  if (fetch_type_bare_long (field2) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" url :"); }
  struct paramed_type *field3 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  if (fetch_type_bare_string (field3) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" display_url :"); }
  struct paramed_type *field4 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  if (fetch_type_bare_string (field4) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (PTR2INT (var0) & (1 << 0)) {
    if (multiline_output >= 2) { print_offset (); }
    if (!disable_field_names) { eprintf (" type :"); }
    struct paramed_type *field5 = 
    &(struct paramed_type){
      .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
      .params = 0,
    };
    if (fetch_type_bare_string (field5) < 0) { return -1;}
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
    if (fetch_type_bare_string (field6) < 0) { return -1;}
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
    if (fetch_type_bare_string (field7) < 0) { return -1;}
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
    if (fetch_type_bare_string (field8) < 0) { return -1;}
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
    if (fetch_type_photo (field9) < 0) { return -1;}
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
    if (fetch_type_bare_string (field10) < 0) { return -1;}
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
    if (fetch_type_bare_string (field11) < 0) { return -1;}
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
    if (fetch_type_bare_int (field12) < 0) { return -1;}
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
    if (fetch_type_bare_int (field13) < 0) { return -1;}
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
    if (fetch_type_bare_int (field14) < 0) { return -1;}
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
    if (fetch_type_bare_string (field15) < 0) { return -1;}
    if (multiline_output >= 2) { eprintf ("\n"); }
  }
  return 0;
}
int fetch_constructor_account_authorizations (struct paramed_type *T) {
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
  if (fetch_type_vector (field1) < 0) { return -1;}
  return 0;
}
int fetch_constructor_account_no_password (struct paramed_type *T) {
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
  if (fetch_type_bare_bytes (field1) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" email_unconfirmed_pattern :"); }
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  if (fetch_type_bare_string (field2) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int fetch_constructor_account_password (struct paramed_type *T) {
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
  if (fetch_type_bare_bytes (field1) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" new_salt :"); }
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xf11ec860, .id = "Bare_Bytes", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  if (fetch_type_bare_bytes (field2) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" hint :"); }
  struct paramed_type *field3 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  if (fetch_type_bare_string (field3) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" has_recovery :"); }
  struct paramed_type *field4 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x250be282, .id = "Bool", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  if (fetch_type_bool (field4) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" email_unconfirmed_pattern :"); }
  struct paramed_type *field5 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  if (fetch_type_bare_string (field5) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int fetch_constructor_account_password_input_settings (struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0xbcfc532c && T->type->name != 0x4303acd3)) { return -1; }
  eprintf (" account.passwordInputSettings");
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" flags :"); }
  if (in_remaining () < 4) { return -1;}
  eprintf (" %d", prefetch_int ());
  struct paramed_type *var0 = INT2PTR (fetch_int ());
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (PTR2INT (var0) & (1 << 0)) {
    if (multiline_output >= 2) { print_offset (); }
    if (!disable_field_names) { eprintf (" new_salt :"); }
    struct paramed_type *field2 = 
    &(struct paramed_type){
      .type = &(struct tl_type_descr) {.name = 0xf11ec860, .id = "Bare_Bytes", .params_num = 0, .params_types = 0},
      .params = 0,
    };
    if (fetch_type_bare_bytes (field2) < 0) { return -1;}
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
    if (fetch_type_bare_bytes (field3) < 0) { return -1;}
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
    if (fetch_type_bare_string (field4) < 0) { return -1;}
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
    if (fetch_type_bare_string (field5) < 0) { return -1;}
    if (multiline_output >= 2) { eprintf ("\n"); }
  }
  return 0;
}
int fetch_constructor_account_password_settings (struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0xb7b72ab3 && T->type->name != 0x4848d54c)) { return -1; }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  if (fetch_type_bare_string (field1) < 0) { return -1;}
  return 0;
}
int fetch_constructor_account_privacy_rules (struct paramed_type *T) {
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
  if (fetch_type_vector (field1) < 0) { return -1;}
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
  if (fetch_type_vector (field2) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int fetch_constructor_account_sent_change_phone_code (struct paramed_type *T) {
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
  if (fetch_type_bare_string (field1) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" send_call_timeout :"); }
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  if (fetch_type_bare_int (field2) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int fetch_constructor_auth_authorization (struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0xff036af1 && T->type->name != 0x00fc950e)) { return -1; }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x02e6c0c8, .id = "User", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  if (fetch_type_user (field1) < 0) { return -1;}
  return 0;
}
int fetch_constructor_auth_checked_phone (struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x811ea28e && T->type->name != 0x7ee15d71)) { return -1; }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x250be282, .id = "Bool", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  if (fetch_type_bool (field1) < 0) { return -1;}
  return 0;
}
int fetch_constructor_auth_exported_authorization (struct paramed_type *T) {
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
  if (fetch_type_bare_int (field1) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" bytes :"); }
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xf11ec860, .id = "Bare_Bytes", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  if (fetch_type_bare_bytes (field2) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int fetch_constructor_auth_password_recovery (struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x137948a5 && T->type->name != 0xec86b75a)) { return -1; }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  if (fetch_type_bare_string (field1) < 0) { return -1;}
  return 0;
}
int fetch_constructor_auth_sent_code (struct paramed_type *T) {
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
  if (fetch_type_bool (field1) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" phone_code_hash :"); }
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  if (fetch_type_bare_string (field2) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" send_call_timeout :"); }
  struct paramed_type *field3 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  if (fetch_type_bare_int (field3) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" is_password :"); }
  struct paramed_type *field4 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x250be282, .id = "Bool", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  if (fetch_type_bool (field4) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int fetch_constructor_auth_sent_app_code (struct paramed_type *T) {
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
  if (fetch_type_bool (field1) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" phone_code_hash :"); }
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  if (fetch_type_bare_string (field2) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" send_call_timeout :"); }
  struct paramed_type *field3 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  if (fetch_type_bare_int (field3) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" is_password :"); }
  struct paramed_type *field4 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x250be282, .id = "Bool", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  if (fetch_type_bool (field4) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int fetch_constructor_binlog_encr_key (struct paramed_type *T) {
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
  while (multiplicity1 -- > 0) {
    if (multiline_output >= 1) { print_offset (); }
    if (fetch_type_any (field1) < 0) { return -1;}
    if (multiline_output >= 1) { eprintf ("\n"); }
  }
  if (multiline_output >= 1) { multiline_offset -= multiline_offset_size; print_offset ();}
  eprintf (" ]");
  return 0;
}
int fetch_constructor_binlog_start (struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x66ff03a6 && T->type->name != 0x9900fc59)) { return -1; }
  eprintf (" binlog.start");
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int fetch_constructor_binlog_dc_option (struct paramed_type *T) {
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
  if (fetch_type_bare_int (field1) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" name :"); }
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  if (fetch_type_bare_string (field2) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" ip :"); }
  struct paramed_type *field3 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  if (fetch_type_bare_string (field3) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" port :"); }
  struct paramed_type *field4 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  if (fetch_type_bare_int (field4) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int fetch_constructor_binlog_dc_option_new (struct paramed_type *T) {
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
  if (fetch_type_bare_int (field1) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" dc :"); }
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  if (fetch_type_bare_int (field2) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" name :"); }
  struct paramed_type *field3 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  if (fetch_type_bare_string (field3) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" ip :"); }
  struct paramed_type *field4 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  if (fetch_type_bare_string (field4) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" port :"); }
  struct paramed_type *field5 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  if (fetch_type_bare_int (field5) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int fetch_constructor_binlog_auth_key (struct paramed_type *T) {
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
  if (fetch_type_bare_int (field1) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" key :"); }
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xfc88e970, .id = "Bare_binlog.EncrKey", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  if (fetch_type_bare_binlog_encr_key (field2) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int fetch_constructor_binlog_default_dc (struct paramed_type *T) {
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
  if (fetch_type_bare_int (field1) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int fetch_constructor_binlog_dc_signed (struct paramed_type *T) {
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
  if (fetch_type_bare_int (field1) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int fetch_constructor_binlog_our_id (struct paramed_type *T) {
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
  if (fetch_type_bare_int (field1) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int fetch_constructor_binlog_set_dh_params (struct paramed_type *T) {
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
  if (fetch_type_bare_int (field1) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" prime :"); }
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xfc88e970, .id = "Bare_binlog.EncrKey", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  if (fetch_type_bare_binlog_encr_key (field2) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" version :"); }
  struct paramed_type *field3 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  if (fetch_type_bare_int (field3) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int fetch_constructor_binlog_set_pts (struct paramed_type *T) {
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
  if (fetch_type_bare_int (field1) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int fetch_constructor_binlog_set_qts (struct paramed_type *T) {
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
  if (fetch_type_bare_int (field1) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int fetch_constructor_binlog_set_date (struct paramed_type *T) {
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
  if (fetch_type_bare_int (field1) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int fetch_constructor_binlog_set_seq (struct paramed_type *T) {
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
  if (fetch_type_bare_int (field1) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int fetch_constructor_binlog_encr_chat_delete (struct paramed_type *T) {
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
  if (fetch_type_bare_int (field1) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int fetch_constructor_binlog_encr_chat_new (struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x66ff03a6 && T->type->name != 0x9900fc59)) { return -1; }
  eprintf (" binlog.encrChatNew");
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" flags :"); }
  if (in_remaining () < 4) { return -1;}
  eprintf (" %d", prefetch_int ());
  struct paramed_type *var0 = INT2PTR (fetch_int ());
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" id :"); }
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  if (fetch_type_bare_int (field2) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (PTR2INT (var0) & (1 << 17)) {
    if (multiline_output >= 2) { print_offset (); }
    if (!disable_field_names) { eprintf (" access_hash :"); }
    struct paramed_type *field3 = 
    &(struct paramed_type){
      .type = &(struct tl_type_descr) {.name = 0xddf89345, .id = "Bare_Long", .params_num = 0, .params_types = 0},
      .params = 0,
    };
    if (fetch_type_bare_long (field3) < 0) { return -1;}
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
    if (fetch_type_bare_int (field4) < 0) { return -1;}
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
    if (fetch_type_bare_int (field5) < 0) { return -1;}
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
    if (fetch_type_bare_int (field6) < 0) { return -1;}
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
    if (fetch_type_bare_binlog_encr_key (field7) < 0) { return -1;}
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
    if (fetch_type_bare_binlog_encr_key (field8) < 0) { return -1;}
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
    if (fetch_type_bare_int (field9) < 0) { return -1;}
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
    if (fetch_type_bare_int (field10) < 0) { return -1;}
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
    if (fetch_type_bare_int (field11) < 0) { return -1;}
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
    if (fetch_type_bare_int (field12) < 0) { return -1;}
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
    if (fetch_type_bare_int (field13) < 0) { return -1;}
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
    if (fetch_type_bare_int (field14) < 0) { return -1;}
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
    if (fetch_type_bare_long (field15) < 0) { return -1;}
    if (multiline_output >= 2) { eprintf ("\n"); }
  }
  return 0;
}
int fetch_constructor_binlog_encr_chat_exchange_new (struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x66ff03a6 && T->type->name != 0x9900fc59)) { return -1; }
  eprintf (" binlog.encrChatExchangeNew");
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" flags :"); }
  if (in_remaining () < 4) { return -1;}
  eprintf (" %d", prefetch_int ());
  struct paramed_type *var0 = INT2PTR (fetch_int ());
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" id :"); }
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  if (fetch_type_bare_int (field2) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (PTR2INT (var0) & (1 << 17)) {
    if (multiline_output >= 2) { print_offset (); }
    if (!disable_field_names) { eprintf (" exchange_id :"); }
    struct paramed_type *field3 = 
    &(struct paramed_type){
      .type = &(struct tl_type_descr) {.name = 0xddf89345, .id = "Bare_Long", .params_num = 0, .params_types = 0},
      .params = 0,
    };
    if (fetch_type_bare_long (field3) < 0) { return -1;}
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
    if (fetch_type_bare_binlog_encr_key (field4) < 0) { return -1;}
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
    if (fetch_type_bare_int (field5) < 0) { return -1;}
    if (multiline_output >= 2) { eprintf ("\n"); }
  }
  return 0;
}
int fetch_constructor_binlog_user_delete (struct paramed_type *T) {
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
  if (fetch_type_bare_int (field1) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int fetch_constructor_binlog_user_new (struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x66ff03a6 && T->type->name != 0x9900fc59)) { return -1; }
  eprintf (" binlog.userNew");
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" flags :"); }
  if (in_remaining () < 4) { return -1;}
  eprintf (" %d", prefetch_int ());
  struct paramed_type *var0 = INT2PTR (fetch_int ());
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" id :"); }
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  if (fetch_type_bare_int (field2) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (PTR2INT (var0) & (1 << 17)) {
    if (multiline_output >= 2) { print_offset (); }
    if (!disable_field_names) { eprintf (" access_hash :"); }
    struct paramed_type *field3 = 
    &(struct paramed_type){
      .type = &(struct tl_type_descr) {.name = 0xddf89345, .id = "Bare_Long", .params_num = 0, .params_types = 0},
      .params = 0,
    };
    if (fetch_type_bare_long (field3) < 0) { return -1;}
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
    if (fetch_type_bare_string (field4) < 0) { return -1;}
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
    if (fetch_type_bare_string (field5) < 0) { return -1;}
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
    if (fetch_type_bare_string (field6) < 0) { return -1;}
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
    if (fetch_type_bare_string (field7) < 0) { return -1;}
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
    if (fetch_type_photo (field8) < 0) { return -1;}
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
    if (fetch_type_bare_string (field9) < 0) { return -1;}
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
    if (fetch_type_bare_string (field10) < 0) { return -1;}
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
    if (fetch_type_user_profile_photo (field11) < 0) { return -1;}
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
    if (fetch_type_bare_int (field12) < 0) { return -1;}
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
    if (fetch_type_bare_int (field13) < 0) { return -1;}
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
    if (fetch_type_bot_info (field14) < 0) { return -1;}
    if (multiline_output >= 2) { eprintf ("\n"); }
  }
  return 0;
}
int fetch_constructor_binlog_chat_new (struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x66ff03a6 && T->type->name != 0x9900fc59)) { return -1; }
  eprintf (" binlog.chatNew");
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" flags :"); }
  if (in_remaining () < 4) { return -1;}
  eprintf (" %d", prefetch_int ());
  struct paramed_type *var0 = INT2PTR (fetch_int ());
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" id :"); }
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  if (fetch_type_bare_int (field2) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (PTR2INT (var0) & (1 << 17)) {
    if (multiline_output >= 2) { print_offset (); }
    if (!disable_field_names) { eprintf (" title :"); }
    struct paramed_type *field3 = 
    &(struct paramed_type){
      .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
      .params = 0,
    };
    if (fetch_type_bare_string (field3) < 0) { return -1;}
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
    if (fetch_type_bare_int (field4) < 0) { return -1;}
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
    if (fetch_type_bare_int (field5) < 0) { return -1;}
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
    if (fetch_type_bare_int (field6) < 0) { return -1;}
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
    if (fetch_type_vector (field7) < 0) { return -1;}
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
    if (fetch_type_chat_photo (field8) < 0) { return -1;}
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
    if (fetch_type_photo (field9) < 0) { return -1;}
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
    if (fetch_type_bare_int (field10) < 0) { return -1;}
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
    if (fetch_type_bare_int (field11) < 0) { return -1;}
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
    if (fetch_type_bare_int (field12) < 0) { return -1;}
    if (multiline_output >= 2) { eprintf ("\n"); }
  }
  return 0;
}
int fetch_constructor_binlog_chat_add_participant (struct paramed_type *T) {
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
  if (fetch_type_bare_int (field1) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" version :"); }
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  if (fetch_type_bare_int (field2) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" user_id :"); }
  struct paramed_type *field3 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  if (fetch_type_bare_int (field3) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" inviter_id :"); }
  struct paramed_type *field4 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  if (fetch_type_bare_int (field4) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" date :"); }
  struct paramed_type *field5 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  if (fetch_type_bare_int (field5) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int fetch_constructor_binlog_chat_del_participant (struct paramed_type *T) {
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
  if (fetch_type_bare_int (field1) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" version :"); }
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  if (fetch_type_bare_int (field2) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" user_id :"); }
  struct paramed_type *field3 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  if (fetch_type_bare_int (field3) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int fetch_constructor_binlog_set_msg_id (struct paramed_type *T) {
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
  if (fetch_type_bare_long (field1) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" new_id :"); }
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  if (fetch_type_bare_int (field2) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int fetch_constructor_binlog_message_delete (struct paramed_type *T) {
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
  if (fetch_type_bare_long (field1) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int fetch_constructor_binlog_message_new (struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x66ff03a6 && T->type->name != 0x9900fc59)) { return -1; }
  eprintf (" binlog.messageNew");
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" flags :"); }
  if (in_remaining () < 4) { return -1;}
  eprintf (" %d", prefetch_int ());
  struct paramed_type *var0 = INT2PTR (fetch_int ());
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" lid :"); }
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xddf89345, .id = "Bare_Long", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  if (fetch_type_bare_long (field2) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (PTR2INT (var0) & (1 << 17)) {
    if (multiline_output >= 2) { print_offset (); }
    if (!disable_field_names) { eprintf (" from_id :"); }
    struct paramed_type *field3 = 
    &(struct paramed_type){
      .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
      .params = 0,
    };
    if (fetch_type_bare_int (field3) < 0) { return -1;}
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
    if (fetch_type_bare_int (field4) < 0) { return -1;}
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
    if (fetch_type_bare_int (field5) < 0) { return -1;}
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
    if (fetch_type_bare_int (field6) < 0) { return -1;}
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
    if (fetch_type_bare_int (field7) < 0) { return -1;}
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
    if (fetch_type_bare_int (field8) < 0) { return -1;}
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
    if (fetch_type_bare_string (field9) < 0) { return -1;}
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
    if (fetch_type_message_media (field10) < 0) { return -1;}
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
    if (fetch_type_message_action (field11) < 0) { return -1;}
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
    if (fetch_type_bare_int (field12) < 0) { return -1;}
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
    if (fetch_type_reply_markup (field13) < 0) { return -1;}
    if (multiline_output >= 2) { eprintf ("\n"); }
  }
  return 0;
}
int fetch_constructor_binlog_message_encr_new (struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x66ff03a6 && T->type->name != 0x9900fc59)) { return -1; }
  eprintf (" binlog.messageEncrNew");
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" flags :"); }
  if (in_remaining () < 4) { return -1;}
  eprintf (" %d", prefetch_int ());
  struct paramed_type *var0 = INT2PTR (fetch_int ());
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" lid :"); }
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xddf89345, .id = "Bare_Long", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  if (fetch_type_bare_long (field2) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (PTR2INT (var0) & (1 << 17)) {
    if (multiline_output >= 2) { print_offset (); }
    if (!disable_field_names) { eprintf (" from_id :"); }
    struct paramed_type *field3 = 
    &(struct paramed_type){
      .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
      .params = 0,
    };
    if (fetch_type_bare_int (field3) < 0) { return -1;}
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
    if (fetch_type_bare_int (field4) < 0) { return -1;}
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
    if (fetch_type_bare_int (field5) < 0) { return -1;}
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
    if (fetch_type_bare_int (field6) < 0) { return -1;}
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
    if (fetch_type_bare_string (field7) < 0) { return -1;}
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
    if (fetch_type_decrypted_message_media (field8) < 0) { return -1;}
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
    if (fetch_type_decrypted_message_action (field9) < 0) { return -1;}
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
    if (fetch_type_encrypted_file (field10) < 0) { return -1;}
    if (multiline_output >= 2) { eprintf ("\n"); }
  }
  return 0;
}
int fetch_constructor_binlog_msg_update (struct paramed_type *T) {
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
  if (fetch_type_bare_long (field1) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int fetch_constructor_binlog_reset_authorization (struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x66ff03a6 && T->type->name != 0x9900fc59)) { return -1; }
  eprintf (" binlog.resetAuthorization");
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int fetch_constructor_contacts_blocked (struct paramed_type *T) {
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
  if (fetch_type_vector (field1) < 0) { return -1;}
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
  if (fetch_type_vector (field2) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int fetch_constructor_contacts_blocked_slice (struct paramed_type *T) {
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
  if (fetch_type_bare_int (field1) < 0) { return -1;}
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
  if (fetch_type_vector (field2) < 0) { return -1;}
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
  if (fetch_type_vector (field3) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int fetch_constructor_contacts_contacts_not_modified (struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0xd8c02560 && T->type->name != 0x273fda9f)) { return -1; }
  eprintf (" contacts.contactsNotModified");
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int fetch_constructor_contacts_contacts (struct paramed_type *T) {
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
  if (fetch_type_vector (field1) < 0) { return -1;}
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
  if (fetch_type_vector (field2) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int fetch_constructor_contacts_found (struct paramed_type *T) {
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
  if (fetch_type_vector (field1) < 0) { return -1;}
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
  if (fetch_type_vector (field2) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int fetch_constructor_contacts_imported_contacts (struct paramed_type *T) {
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
  if (fetch_type_vector (field1) < 0) { return -1;}
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
  if (fetch_type_vector (field2) < 0) { return -1;}
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
  if (fetch_type_vector (field3) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int fetch_constructor_contacts_link (struct paramed_type *T) {
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
  if (fetch_type_contact_link (field1) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" foreign_link :"); }
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x522fbc63, .id = "ContactLink", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  if (fetch_type_contact_link (field2) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" user :"); }
  struct paramed_type *field3 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x02e6c0c8, .id = "User", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  if (fetch_type_user (field3) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int fetch_constructor_contacts_suggested (struct paramed_type *T) {
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
  if (fetch_type_vector (field1) < 0) { return -1;}
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
  if (fetch_type_vector (field2) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int fetch_constructor_geochats_located (struct paramed_type *T) {
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
  if (fetch_type_vector (field1) < 0) { return -1;}
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
  if (fetch_type_vector (field2) < 0) { return -1;}
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
  if (fetch_type_vector (field3) < 0) { return -1;}
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
  if (fetch_type_vector (field4) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int fetch_constructor_geochats_messages (struct paramed_type *T) {
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
  if (fetch_type_vector (field1) < 0) { return -1;}
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
  if (fetch_type_vector (field2) < 0) { return -1;}
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
  if (fetch_type_vector (field3) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int fetch_constructor_geochats_messages_slice (struct paramed_type *T) {
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
  if (fetch_type_bare_int (field1) < 0) { return -1;}
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
  if (fetch_type_vector (field2) < 0) { return -1;}
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
  if (fetch_type_vector (field3) < 0) { return -1;}
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
  if (fetch_type_vector (field4) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int fetch_constructor_geochats_stated_message (struct paramed_type *T) {
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
  if (fetch_type_geo_chat_message (field1) < 0) { return -1;}
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
  if (fetch_type_vector (field2) < 0) { return -1;}
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
  if (fetch_type_vector (field3) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" seq :"); }
  struct paramed_type *field4 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  if (fetch_type_bare_int (field4) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int fetch_constructor_help_app_update (struct paramed_type *T) {
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
  if (fetch_type_bare_int (field1) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" critical :"); }
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x250be282, .id = "Bool", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  if (fetch_type_bool (field2) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" url :"); }
  struct paramed_type *field3 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  if (fetch_type_bare_string (field3) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" text :"); }
  struct paramed_type *field4 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  if (fetch_type_bare_string (field4) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int fetch_constructor_help_no_app_update (struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x4ddd9627 && T->type->name != 0xb22269d8)) { return -1; }
  eprintf (" help.noAppUpdate");
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int fetch_constructor_help_invite_text (struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x18cb9f78 && T->type->name != 0xe7346087)) { return -1; }
  struct paramed_type *field1 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x4ad791db, .id = "Bare_String", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  if (fetch_type_bare_string (field1) < 0) { return -1;}
  return 0;
}
int fetch_constructor_help_support (struct paramed_type *T) {
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
  if (fetch_type_bare_string (field1) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" user :"); }
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x02e6c0c8, .id = "User", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  if (fetch_type_user (field2) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int fetch_constructor_messages_affected_history (struct paramed_type *T) {
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
  if (fetch_type_bare_int (field1) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" pts_count :"); }
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  if (fetch_type_bare_int (field2) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" offset :"); }
  struct paramed_type *field3 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  if (fetch_type_bare_int (field3) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int fetch_constructor_messages_affected_messages (struct paramed_type *T) {
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
  if (fetch_type_bare_int (field1) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" pts_count :"); }
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  if (fetch_type_bare_int (field2) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int fetch_constructor_messages_all_stickers_not_modified (struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0xb485502f && T->type->name != 0x4b7aafd0)) { return -1; }
  eprintf (" messages.allStickersNotModified");
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int fetch_constructor_messages_all_stickers (struct paramed_type *T) {
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
  if (fetch_type_bare_string (field1) < 0) { return -1;}
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
  if (fetch_type_vector (field2) < 0) { return -1;}
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
  if (fetch_type_vector (field3) < 0) { return -1;}
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
  if (fetch_type_vector (field4) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int fetch_constructor_messages_chat_full (struct paramed_type *T) {
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
  if (fetch_type_chat_full (field1) < 0) { return -1;}
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
  if (fetch_type_vector (field2) < 0) { return -1;}
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
  if (fetch_type_vector (field3) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int fetch_constructor_messages_chats (struct paramed_type *T) {
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
  if (fetch_type_vector (field1) < 0) { return -1;}
  return 0;
}
int fetch_constructor_messages_dh_config_not_modified (struct paramed_type *T) {
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
  if (fetch_type_bare_bytes (field1) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int fetch_constructor_messages_dh_config (struct paramed_type *T) {
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
  if (fetch_type_bare_int (field1) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" p :"); }
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xf11ec860, .id = "Bare_Bytes", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  if (fetch_type_bare_bytes (field2) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" version :"); }
  struct paramed_type *field3 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  if (fetch_type_bare_int (field3) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" random :"); }
  struct paramed_type *field4 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xf11ec860, .id = "Bare_Bytes", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  if (fetch_type_bare_bytes (field4) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int fetch_constructor_messages_dialogs (struct paramed_type *T) {
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
  if (fetch_type_vector (field1) < 0) { return -1;}
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
  if (fetch_type_vector (field2) < 0) { return -1;}
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
  if (fetch_type_vector (field3) < 0) { return -1;}
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
  if (fetch_type_vector (field4) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int fetch_constructor_messages_dialogs_slice (struct paramed_type *T) {
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
  if (fetch_type_bare_int (field1) < 0) { return -1;}
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
  if (fetch_type_vector (field2) < 0) { return -1;}
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
  if (fetch_type_vector (field3) < 0) { return -1;}
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
  if (fetch_type_vector (field4) < 0) { return -1;}
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
  if (fetch_type_vector (field5) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int fetch_constructor_messages_message_empty (struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x3f4e0648 && T->type->name != 0xc0b1f9b7)) { return -1; }
  eprintf (" messages.messageEmpty");
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int fetch_constructor_messages_messages (struct paramed_type *T) {
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
  if (fetch_type_vector (field1) < 0) { return -1;}
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
  if (fetch_type_vector (field2) < 0) { return -1;}
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
  if (fetch_type_vector (field3) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int fetch_constructor_messages_messages_slice (struct paramed_type *T) {
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
  if (fetch_type_bare_int (field1) < 0) { return -1;}
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
  if (fetch_type_vector (field2) < 0) { return -1;}
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
  if (fetch_type_vector (field3) < 0) { return -1;}
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
  if (fetch_type_vector (field4) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int fetch_constructor_messages_sent_encrypted_message (struct paramed_type *T) {
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
  if (fetch_type_bare_int (field1) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int fetch_constructor_messages_sent_encrypted_file (struct paramed_type *T) {
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
  if (fetch_type_bare_int (field1) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" file :"); }
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x886fd032, .id = "EncryptedFile", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  if (fetch_type_encrypted_file (field2) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int fetch_constructor_messages_sent_message (struct paramed_type *T) {
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
  if (fetch_type_bare_int (field1) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" date :"); }
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  if (fetch_type_bare_int (field2) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" media :"); }
  struct paramed_type *field3 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x95f2471a, .id = "MessageMedia", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  if (fetch_type_message_media (field3) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" pts :"); }
  struct paramed_type *field4 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  if (fetch_type_bare_int (field4) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" pts_count :"); }
  struct paramed_type *field5 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  if (fetch_type_bare_int (field5) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int fetch_constructor_messages_sent_message_link (struct paramed_type *T) {
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
  if (fetch_type_bare_int (field1) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" date :"); }
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  if (fetch_type_bare_int (field2) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" media :"); }
  struct paramed_type *field3 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x95f2471a, .id = "MessageMedia", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  if (fetch_type_message_media (field3) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" pts :"); }
  struct paramed_type *field4 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  if (fetch_type_bare_int (field4) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" pts_count :"); }
  struct paramed_type *field5 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  if (fetch_type_bare_int (field5) < 0) { return -1;}
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
  if (fetch_type_vector (field6) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" seq :"); }
  struct paramed_type *field7 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  if (fetch_type_bare_int (field7) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int fetch_constructor_messages_sticker_set (struct paramed_type *T) {
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
  if (fetch_type_sticker_set (field1) < 0) { return -1;}
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
  if (fetch_type_vector (field2) < 0) { return -1;}
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
  if (fetch_type_vector (field3) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int fetch_constructor_messages_stickers_not_modified (struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x7bfa5710 && T->type->name != 0x8405a8ef)) { return -1; }
  eprintf (" messages.stickersNotModified");
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int fetch_constructor_messages_stickers (struct paramed_type *T) {
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
  if (fetch_type_bare_string (field1) < 0) { return -1;}
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
  if (fetch_type_vector (field2) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int fetch_constructor_photos_photo (struct paramed_type *T) {
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
  if (fetch_type_photo (field1) < 0) { return -1;}
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
  if (fetch_type_vector (field2) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int fetch_constructor_photos_photos (struct paramed_type *T) {
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
  if (fetch_type_vector (field1) < 0) { return -1;}
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
  if (fetch_type_vector (field2) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int fetch_constructor_photos_photos_slice (struct paramed_type *T) {
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
  if (fetch_type_bare_int (field1) < 0) { return -1;}
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
  if (fetch_type_vector (field2) < 0) { return -1;}
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
  if (fetch_type_vector (field3) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int fetch_constructor_storage_file_unknown (struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x3e2838a8 && T->type->name != 0xc1d7c757)) { return -1; }
  eprintf (" storage.fileUnknown");
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int fetch_constructor_storage_file_jpeg (struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x3e2838a8 && T->type->name != 0xc1d7c757)) { return -1; }
  eprintf (" storage.fileJpeg");
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int fetch_constructor_storage_file_gif (struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x3e2838a8 && T->type->name != 0xc1d7c757)) { return -1; }
  eprintf (" storage.fileGif");
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int fetch_constructor_storage_file_png (struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x3e2838a8 && T->type->name != 0xc1d7c757)) { return -1; }
  eprintf (" storage.filePng");
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int fetch_constructor_storage_file_pdf (struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x3e2838a8 && T->type->name != 0xc1d7c757)) { return -1; }
  eprintf (" storage.filePdf");
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int fetch_constructor_storage_file_mp3 (struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x3e2838a8 && T->type->name != 0xc1d7c757)) { return -1; }
  eprintf (" storage.fileMp3");
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int fetch_constructor_storage_file_mov (struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x3e2838a8 && T->type->name != 0xc1d7c757)) { return -1; }
  eprintf (" storage.fileMov");
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int fetch_constructor_storage_file_partial (struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x3e2838a8 && T->type->name != 0xc1d7c757)) { return -1; }
  eprintf (" storage.filePartial");
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int fetch_constructor_storage_file_mp4 (struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x3e2838a8 && T->type->name != 0xc1d7c757)) { return -1; }
  eprintf (" storage.fileMp4");
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int fetch_constructor_storage_file_webp (struct paramed_type *T) {
  if (ODDP(T) || (T->type->name != 0x3e2838a8 && T->type->name != 0xc1d7c757)) { return -1; }
  eprintf (" storage.fileWebp");
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int fetch_constructor_updates_difference_empty (struct paramed_type *T) {
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
  if (fetch_type_bare_int (field1) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" seq :"); }
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  if (fetch_type_bare_int (field2) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int fetch_constructor_updates_difference (struct paramed_type *T) {
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
  if (fetch_type_vector (field1) < 0) { return -1;}
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
  if (fetch_type_vector (field2) < 0) { return -1;}
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
  if (fetch_type_vector (field3) < 0) { return -1;}
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
  if (fetch_type_vector (field4) < 0) { return -1;}
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
  if (fetch_type_vector (field5) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" state :"); }
  struct paramed_type *field6 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xa56c2a3e, .id = "updates.State", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  if (fetch_type_updates_state (field6) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int fetch_constructor_updates_difference_slice (struct paramed_type *T) {
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
  if (fetch_type_vector (field1) < 0) { return -1;}
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
  if (fetch_type_vector (field2) < 0) { return -1;}
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
  if (fetch_type_vector (field3) < 0) { return -1;}
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
  if (fetch_type_vector (field4) < 0) { return -1;}
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
  if (fetch_type_vector (field5) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" intermediate_state :"); }
  struct paramed_type *field6 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xa56c2a3e, .id = "updates.State", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  if (fetch_type_updates_state (field6) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int fetch_constructor_updates_state (struct paramed_type *T) {
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
  if (fetch_type_bare_int (field1) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" qts :"); }
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  if (fetch_type_bare_int (field2) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" date :"); }
  struct paramed_type *field3 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  if (fetch_type_bare_int (field3) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" seq :"); }
  struct paramed_type *field4 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  if (fetch_type_bare_int (field4) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" unread_count :"); }
  struct paramed_type *field5 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  if (fetch_type_bare_int (field5) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int fetch_constructor_upload_file (struct paramed_type *T) {
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
  if (fetch_type_storage_file_type (field1) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" mtime :"); }
  struct paramed_type *field2 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0x57af6425, .id = "Bare_Int", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  if (fetch_type_bare_int (field2) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  if (multiline_output >= 2) { print_offset (); }
  if (!disable_field_names) { eprintf (" bytes :"); }
  struct paramed_type *field3 = 
  &(struct paramed_type){
    .type = &(struct tl_type_descr) {.name = 0xf11ec860, .id = "Bare_Bytes", .params_num = 0, .params_types = 0},
    .params = 0,
  };
  if (fetch_type_bare_bytes (field3) < 0) { return -1;}
  if (multiline_output >= 2) { eprintf ("\n"); }
  return 0;
}
int fetch_type_account_days_t_t_l (struct paramed_type *T) {
  if (in_remaining () < 4) { return -1;}
  int magic = fetch_int ();
  int res = -1;
  switch (magic) {
  case 0xb8d0afdf: res = fetch_constructor_account_days_t_t_l (T); break;
  default: return -1;
  }
  return res;
}
int fetch_type_bare_account_days_t_t_l (struct paramed_type *T) {
  if (fetch_constructor_account_days_t_t_l (T) >= 0) { return 0; }
  return -1;
}
int fetch_type_audio (struct paramed_type *T) {
  if (in_remaining () < 4) { return -1;}
  if (multiline_output >= 2) { multiline_offset += multiline_offset_size; }
  eprintf (" (");
  int magic = fetch_int ();
  int res = -1;
  switch (magic) {
  case 0x586988d8: res = fetch_constructor_audio_empty (T); break;
  case 0xc7ac6496: res = fetch_constructor_audio (T); break;
  default: return -1;
  }
  if (res >= 0) {
    if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); }
    eprintf (" )");
  }
  return res;
}
int fetch_type_bare_audio (struct paramed_type *T) {
  int *save_in_ptr = in_ptr;
  if (multiline_output >= 2) { multiline_offset += multiline_offset_size; }
  if (skip_constructor_audio_empty (T) >= 0) { in_ptr = save_in_ptr; eprintf (" ("); assert (!fetch_constructor_audio_empty (T)); if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); } eprintf (" )");return 0; }
  in_ptr = save_in_ptr;
  if (skip_constructor_audio (T) >= 0) { in_ptr = save_in_ptr; eprintf (" ("); assert (!fetch_constructor_audio (T)); if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); } eprintf (" )");return 0; }
  in_ptr = save_in_ptr;
  return -1;
}
int fetch_type_authorization (struct paramed_type *T) {
  if (in_remaining () < 4) { return -1;}
  if (multiline_output >= 2) { multiline_offset += multiline_offset_size; }
  eprintf (" (");
  int magic = fetch_int ();
  int res = -1;
  switch (magic) {
  case 0x7bf2e6f6: res = fetch_constructor_authorization (T); break;
  default: return -1;
  }
  if (res >= 0) {
    if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); }
    eprintf (" )");
  }
  return res;
}
int fetch_type_bare_authorization (struct paramed_type *T) {
  if (multiline_output >= 2) { multiline_offset += multiline_offset_size; }
  eprintf (" (");
  if (fetch_constructor_authorization (T) >= 0) { if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); } eprintf (" )");return 0; }
  return -1;
}
int fetch_type_bool (struct paramed_type *T) {
  if (in_remaining () < 4) { return -1;}
  if (multiline_output >= 2) { multiline_offset += multiline_offset_size; }
  eprintf (" (");
  int magic = fetch_int ();
  int res = -1;
  switch (magic) {
  case 0xbc799737: res = fetch_constructor_bool_false (T); break;
  case 0x997275b5: res = fetch_constructor_bool_true (T); break;
  default: return -1;
  }
  if (res >= 0) {
    if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); }
    eprintf (" )");
  }
  return res;
}
int fetch_type_bare_bool (struct paramed_type *T) {
  int *save_in_ptr = in_ptr;
  if (multiline_output >= 2) { multiline_offset += multiline_offset_size; }
  if (skip_constructor_bool_false (T) >= 0) { in_ptr = save_in_ptr; eprintf (" ("); assert (!fetch_constructor_bool_false (T)); if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); } eprintf (" )");return 0; }
  in_ptr = save_in_ptr;
  if (skip_constructor_bool_true (T) >= 0) { in_ptr = save_in_ptr; eprintf (" ("); assert (!fetch_constructor_bool_true (T)); if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); } eprintf (" )");return 0; }
  in_ptr = save_in_ptr;
  return -1;
}
int fetch_type_bot_command (struct paramed_type *T) {
  if (in_remaining () < 4) { return -1;}
  if (multiline_output >= 2) { multiline_offset += multiline_offset_size; }
  eprintf (" (");
  int magic = fetch_int ();
  int res = -1;
  switch (magic) {
  case 0xc27ac8c7: res = fetch_constructor_bot_command (T); break;
  case 0xb79d22ab: res = fetch_constructor_bot_command_old (T); break;
  default: return -1;
  }
  if (res >= 0) {
    if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); }
    eprintf (" )");
  }
  return res;
}
int fetch_type_bare_bot_command (struct paramed_type *T) {
  int *save_in_ptr = in_ptr;
  if (multiline_output >= 2) { multiline_offset += multiline_offset_size; }
  if (skip_constructor_bot_command (T) >= 0) { in_ptr = save_in_ptr; eprintf (" ("); assert (!fetch_constructor_bot_command (T)); if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); } eprintf (" )");return 0; }
  in_ptr = save_in_ptr;
  if (skip_constructor_bot_command_old (T) >= 0) { in_ptr = save_in_ptr; eprintf (" ("); assert (!fetch_constructor_bot_command_old (T)); if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); } eprintf (" )");return 0; }
  in_ptr = save_in_ptr;
  return -1;
}
int fetch_type_bot_info (struct paramed_type *T) {
  if (in_remaining () < 4) { return -1;}
  if (multiline_output >= 2) { multiline_offset += multiline_offset_size; }
  eprintf (" (");
  int magic = fetch_int ();
  int res = -1;
  switch (magic) {
  case 0xbb2e37ce: res = fetch_constructor_bot_info_empty (T); break;
  case 0x09cf585d: res = fetch_constructor_bot_info (T); break;
  default: return -1;
  }
  if (res >= 0) {
    if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); }
    eprintf (" )");
  }
  return res;
}
int fetch_type_bare_bot_info (struct paramed_type *T) {
  int *save_in_ptr = in_ptr;
  if (multiline_output >= 2) { multiline_offset += multiline_offset_size; }
  if (skip_constructor_bot_info_empty (T) >= 0) { in_ptr = save_in_ptr; eprintf (" ("); assert (!fetch_constructor_bot_info_empty (T)); if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); } eprintf (" )");return 0; }
  in_ptr = save_in_ptr;
  if (skip_constructor_bot_info (T) >= 0) { in_ptr = save_in_ptr; eprintf (" ("); assert (!fetch_constructor_bot_info (T)); if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); } eprintf (" )");return 0; }
  in_ptr = save_in_ptr;
  return -1;
}
int fetch_type_bytes (struct paramed_type *T) {
  if (in_remaining () < 4) { return -1;}
  int magic = fetch_int ();
  int res = -1;
  switch (magic) {
  case 0x0ee1379f: res = fetch_constructor_bytes (T); break;
  default: return -1;
  }
  return res;
}
int fetch_type_bare_bytes (struct paramed_type *T) {
  if (fetch_constructor_bytes (T) >= 0) { return 0; }
  return -1;
}
int fetch_type_chat (struct paramed_type *T) {
  if (in_remaining () < 4) { return -1;}
  if (multiline_output >= 2) { multiline_offset += multiline_offset_size; }
  eprintf (" (");
  int magic = fetch_int ();
  int res = -1;
  switch (magic) {
  case 0x9ba2d800: res = fetch_constructor_chat_empty (T); break;
  case 0x6e9c9bc7: res = fetch_constructor_chat (T); break;
  case 0xfb0ccc41: res = fetch_constructor_chat_forbidden (T); break;
  case 0x75eaea5a: res = fetch_constructor_geo_chat (T); break;
  default: return -1;
  }
  if (res >= 0) {
    if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); }
    eprintf (" )");
  }
  return res;
}
int fetch_type_bare_chat (struct paramed_type *T) {
  int *save_in_ptr = in_ptr;
  if (multiline_output >= 2) { multiline_offset += multiline_offset_size; }
  if (skip_constructor_chat_empty (T) >= 0) { in_ptr = save_in_ptr; eprintf (" ("); assert (!fetch_constructor_chat_empty (T)); if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); } eprintf (" )");return 0; }
  in_ptr = save_in_ptr;
  if (skip_constructor_chat (T) >= 0) { in_ptr = save_in_ptr; eprintf (" ("); assert (!fetch_constructor_chat (T)); if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); } eprintf (" )");return 0; }
  in_ptr = save_in_ptr;
  if (skip_constructor_chat_forbidden (T) >= 0) { in_ptr = save_in_ptr; eprintf (" ("); assert (!fetch_constructor_chat_forbidden (T)); if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); } eprintf (" )");return 0; }
  in_ptr = save_in_ptr;
  if (skip_constructor_geo_chat (T) >= 0) { in_ptr = save_in_ptr; eprintf (" ("); assert (!fetch_constructor_geo_chat (T)); if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); } eprintf (" )");return 0; }
  in_ptr = save_in_ptr;
  return -1;
}
int fetch_type_chat_full (struct paramed_type *T) {
  if (in_remaining () < 4) { return -1;}
  if (multiline_output >= 2) { multiline_offset += multiline_offset_size; }
  eprintf (" (");
  int magic = fetch_int ();
  int res = -1;
  switch (magic) {
  case 0x2e02a614: res = fetch_constructor_chat_full (T); break;
  default: return -1;
  }
  if (res >= 0) {
    if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); }
    eprintf (" )");
  }
  return res;
}
int fetch_type_bare_chat_full (struct paramed_type *T) {
  if (multiline_output >= 2) { multiline_offset += multiline_offset_size; }
  eprintf (" (");
  if (fetch_constructor_chat_full (T) >= 0) { if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); } eprintf (" )");return 0; }
  return -1;
}
int fetch_type_chat_invite (struct paramed_type *T) {
  if (in_remaining () < 4) { return -1;}
  if (multiline_output >= 2) { multiline_offset += multiline_offset_size; }
  eprintf (" (");
  int magic = fetch_int ();
  int res = -1;
  switch (magic) {
  case 0x5a686d7c: res = fetch_constructor_chat_invite_already (T); break;
  case 0xce917dcd: res = fetch_constructor_chat_invite (T); break;
  default: return -1;
  }
  if (res >= 0) {
    if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); }
    eprintf (" )");
  }
  return res;
}
int fetch_type_bare_chat_invite (struct paramed_type *T) {
  int *save_in_ptr = in_ptr;
  if (multiline_output >= 2) { multiline_offset += multiline_offset_size; }
  if (skip_constructor_chat_invite_already (T) >= 0) { in_ptr = save_in_ptr; eprintf (" ("); assert (!fetch_constructor_chat_invite_already (T)); if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); } eprintf (" )");return 0; }
  in_ptr = save_in_ptr;
  if (skip_constructor_chat_invite (T) >= 0) { in_ptr = save_in_ptr; eprintf (" ("); assert (!fetch_constructor_chat_invite (T)); if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); } eprintf (" )");return 0; }
  in_ptr = save_in_ptr;
  return -1;
}
int fetch_type_chat_located (struct paramed_type *T) {
  if (in_remaining () < 4) { return -1;}
  if (multiline_output >= 2) { multiline_offset += multiline_offset_size; }
  eprintf (" (");
  int magic = fetch_int ();
  int res = -1;
  switch (magic) {
  case 0x3631cf4c: res = fetch_constructor_chat_located (T); break;
  default: return -1;
  }
  if (res >= 0) {
    if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); }
    eprintf (" )");
  }
  return res;
}
int fetch_type_bare_chat_located (struct paramed_type *T) {
  if (multiline_output >= 2) { multiline_offset += multiline_offset_size; }
  eprintf (" (");
  if (fetch_constructor_chat_located (T) >= 0) { if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); } eprintf (" )");return 0; }
  return -1;
}
int fetch_type_chat_participant (struct paramed_type *T) {
  if (in_remaining () < 4) { return -1;}
  if (multiline_output >= 2) { multiline_offset += multiline_offset_size; }
  eprintf (" (");
  int magic = fetch_int ();
  int res = -1;
  switch (magic) {
  case 0xc8d7493e: res = fetch_constructor_chat_participant (T); break;
  default: return -1;
  }
  if (res >= 0) {
    if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); }
    eprintf (" )");
  }
  return res;
}
int fetch_type_bare_chat_participant (struct paramed_type *T) {
  if (multiline_output >= 2) { multiline_offset += multiline_offset_size; }
  eprintf (" (");
  if (fetch_constructor_chat_participant (T) >= 0) { if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); } eprintf (" )");return 0; }
  return -1;
}
int fetch_type_chat_participants (struct paramed_type *T) {
  if (in_remaining () < 4) { return -1;}
  if (multiline_output >= 2) { multiline_offset += multiline_offset_size; }
  eprintf (" (");
  int magic = fetch_int ();
  int res = -1;
  switch (magic) {
  case 0x0fd2bb8a: res = fetch_constructor_chat_participants_forbidden (T); break;
  case 0x7841b415: res = fetch_constructor_chat_participants (T); break;
  default: return -1;
  }
  if (res >= 0) {
    if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); }
    eprintf (" )");
  }
  return res;
}
int fetch_type_bare_chat_participants (struct paramed_type *T) {
  int *save_in_ptr = in_ptr;
  if (multiline_output >= 2) { multiline_offset += multiline_offset_size; }
  if (skip_constructor_chat_participants_forbidden (T) >= 0) { in_ptr = save_in_ptr; eprintf (" ("); assert (!fetch_constructor_chat_participants_forbidden (T)); if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); } eprintf (" )");return 0; }
  in_ptr = save_in_ptr;
  if (skip_constructor_chat_participants (T) >= 0) { in_ptr = save_in_ptr; eprintf (" ("); assert (!fetch_constructor_chat_participants (T)); if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); } eprintf (" )");return 0; }
  in_ptr = save_in_ptr;
  return -1;
}
int fetch_type_chat_photo (struct paramed_type *T) {
  if (in_remaining () < 4) { return -1;}
  if (multiline_output >= 2) { multiline_offset += multiline_offset_size; }
  eprintf (" (");
  int magic = fetch_int ();
  int res = -1;
  switch (magic) {
  case 0x37c1011c: res = fetch_constructor_chat_photo_empty (T); break;
  case 0x6153276a: res = fetch_constructor_chat_photo (T); break;
  default: return -1;
  }
  if (res >= 0) {
    if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); }
    eprintf (" )");
  }
  return res;
}
int fetch_type_bare_chat_photo (struct paramed_type *T) {
  int *save_in_ptr = in_ptr;
  if (multiline_output >= 2) { multiline_offset += multiline_offset_size; }
  if (skip_constructor_chat_photo_empty (T) >= 0) { in_ptr = save_in_ptr; eprintf (" ("); assert (!fetch_constructor_chat_photo_empty (T)); if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); } eprintf (" )");return 0; }
  in_ptr = save_in_ptr;
  if (skip_constructor_chat_photo (T) >= 0) { in_ptr = save_in_ptr; eprintf (" ("); assert (!fetch_constructor_chat_photo (T)); if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); } eprintf (" )");return 0; }
  in_ptr = save_in_ptr;
  return -1;
}
int fetch_type_client_d_h_inner_data (struct paramed_type *T) {
  if (in_remaining () < 4) { return -1;}
  if (multiline_output >= 2) { multiline_offset += multiline_offset_size; }
  eprintf (" (");
  int magic = fetch_int ();
  int res = -1;
  switch (magic) {
  case 0x6643b654: res = fetch_constructor_client_d_h_inner_data (T); break;
  default: return -1;
  }
  if (res >= 0) {
    if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); }
    eprintf (" )");
  }
  return res;
}
int fetch_type_bare_client_d_h_inner_data (struct paramed_type *T) {
  if (multiline_output >= 2) { multiline_offset += multiline_offset_size; }
  eprintf (" (");
  if (fetch_constructor_client_d_h_inner_data (T) >= 0) { if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); } eprintf (" )");return 0; }
  return -1;
}
int fetch_type_config (struct paramed_type *T) {
  if (in_remaining () < 4) { return -1;}
  if (multiline_output >= 2) { multiline_offset += multiline_offset_size; }
  eprintf (" (");
  int magic = fetch_int ();
  int res = -1;
  switch (magic) {
  case 0x4e32b894: res = fetch_constructor_config (T); break;
  default: return -1;
  }
  if (res >= 0) {
    if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); }
    eprintf (" )");
  }
  return res;
}
int fetch_type_bare_config (struct paramed_type *T) {
  if (multiline_output >= 2) { multiline_offset += multiline_offset_size; }
  eprintf (" (");
  if (fetch_constructor_config (T) >= 0) { if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); } eprintf (" )");return 0; }
  return -1;
}
int fetch_type_contact (struct paramed_type *T) {
  if (in_remaining () < 4) { return -1;}
  if (multiline_output >= 2) { multiline_offset += multiline_offset_size; }
  eprintf (" (");
  int magic = fetch_int ();
  int res = -1;
  switch (magic) {
  case 0xf911c994: res = fetch_constructor_contact (T); break;
  default: return -1;
  }
  if (res >= 0) {
    if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); }
    eprintf (" )");
  }
  return res;
}
int fetch_type_bare_contact (struct paramed_type *T) {
  if (multiline_output >= 2) { multiline_offset += multiline_offset_size; }
  eprintf (" (");
  if (fetch_constructor_contact (T) >= 0) { if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); } eprintf (" )");return 0; }
  return -1;
}
int fetch_type_contact_blocked (struct paramed_type *T) {
  if (in_remaining () < 4) { return -1;}
  if (multiline_output >= 2) { multiline_offset += multiline_offset_size; }
  eprintf (" (");
  int magic = fetch_int ();
  int res = -1;
  switch (magic) {
  case 0x561bc879: res = fetch_constructor_contact_blocked (T); break;
  default: return -1;
  }
  if (res >= 0) {
    if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); }
    eprintf (" )");
  }
  return res;
}
int fetch_type_bare_contact_blocked (struct paramed_type *T) {
  if (multiline_output >= 2) { multiline_offset += multiline_offset_size; }
  eprintf (" (");
  if (fetch_constructor_contact_blocked (T) >= 0) { if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); } eprintf (" )");return 0; }
  return -1;
}
int fetch_type_contact_found (struct paramed_type *T) {
  if (in_remaining () < 4) { return -1;}
  int magic = fetch_int ();
  int res = -1;
  switch (magic) {
  case 0xea879f95: res = fetch_constructor_contact_found (T); break;
  default: return -1;
  }
  return res;
}
int fetch_type_bare_contact_found (struct paramed_type *T) {
  if (fetch_constructor_contact_found (T) >= 0) { return 0; }
  return -1;
}
int fetch_type_contact_link (struct paramed_type *T) {
  if (in_remaining () < 4) { return -1;}
  if (multiline_output >= 2) { multiline_offset += multiline_offset_size; }
  eprintf (" (");
  int magic = fetch_int ();
  int res = -1;
  switch (magic) {
  case 0x5f4f9247: res = fetch_constructor_contact_link_unknown (T); break;
  case 0xfeedd3ad: res = fetch_constructor_contact_link_none (T); break;
  case 0x268f3f59: res = fetch_constructor_contact_link_has_phone (T); break;
  case 0xd502c2d0: res = fetch_constructor_contact_link_contact (T); break;
  default: return -1;
  }
  if (res >= 0) {
    if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); }
    eprintf (" )");
  }
  return res;
}
int fetch_type_bare_contact_link (struct paramed_type *T) {
  int *save_in_ptr = in_ptr;
  if (multiline_output >= 2) { multiline_offset += multiline_offset_size; }
  if (skip_constructor_contact_link_unknown (T) >= 0) { in_ptr = save_in_ptr; eprintf (" ("); assert (!fetch_constructor_contact_link_unknown (T)); if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); } eprintf (" )");return 0; }
  in_ptr = save_in_ptr;
  if (skip_constructor_contact_link_none (T) >= 0) { in_ptr = save_in_ptr; eprintf (" ("); assert (!fetch_constructor_contact_link_none (T)); if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); } eprintf (" )");return 0; }
  in_ptr = save_in_ptr;
  if (skip_constructor_contact_link_has_phone (T) >= 0) { in_ptr = save_in_ptr; eprintf (" ("); assert (!fetch_constructor_contact_link_has_phone (T)); if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); } eprintf (" )");return 0; }
  in_ptr = save_in_ptr;
  if (skip_constructor_contact_link_contact (T) >= 0) { in_ptr = save_in_ptr; eprintf (" ("); assert (!fetch_constructor_contact_link_contact (T)); if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); } eprintf (" )");return 0; }
  in_ptr = save_in_ptr;
  return -1;
}
int fetch_type_contact_status (struct paramed_type *T) {
  if (in_remaining () < 4) { return -1;}
  if (multiline_output >= 2) { multiline_offset += multiline_offset_size; }
  eprintf (" (");
  int magic = fetch_int ();
  int res = -1;
  switch (magic) {
  case 0xd3680c61: res = fetch_constructor_contact_status (T); break;
  default: return -1;
  }
  if (res >= 0) {
    if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); }
    eprintf (" )");
  }
  return res;
}
int fetch_type_bare_contact_status (struct paramed_type *T) {
  if (multiline_output >= 2) { multiline_offset += multiline_offset_size; }
  eprintf (" (");
  if (fetch_constructor_contact_status (T) >= 0) { if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); } eprintf (" )");return 0; }
  return -1;
}
int fetch_type_contact_suggested (struct paramed_type *T) {
  if (in_remaining () < 4) { return -1;}
  if (multiline_output >= 2) { multiline_offset += multiline_offset_size; }
  eprintf (" (");
  int magic = fetch_int ();
  int res = -1;
  switch (magic) {
  case 0x3de191a1: res = fetch_constructor_contact_suggested (T); break;
  default: return -1;
  }
  if (res >= 0) {
    if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); }
    eprintf (" )");
  }
  return res;
}
int fetch_type_bare_contact_suggested (struct paramed_type *T) {
  if (multiline_output >= 2) { multiline_offset += multiline_offset_size; }
  eprintf (" (");
  if (fetch_constructor_contact_suggested (T) >= 0) { if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); } eprintf (" )");return 0; }
  return -1;
}
int fetch_type_dc_option (struct paramed_type *T) {
  if (in_remaining () < 4) { return -1;}
  if (multiline_output >= 2) { multiline_offset += multiline_offset_size; }
  eprintf (" (");
  int magic = fetch_int ();
  int res = -1;
  switch (magic) {
  case 0x2ec2a43c: res = fetch_constructor_dc_option_l28 (T); break;
  case 0x05d8c6cc: res = fetch_constructor_dc_option (T); break;
  default: return -1;
  }
  if (res >= 0) {
    if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); }
    eprintf (" )");
  }
  return res;
}
int fetch_type_bare_dc_option (struct paramed_type *T) {
  int *save_in_ptr = in_ptr;
  if (multiline_output >= 2) { multiline_offset += multiline_offset_size; }
  if (skip_constructor_dc_option_l28 (T) >= 0) { in_ptr = save_in_ptr; eprintf (" ("); assert (!fetch_constructor_dc_option_l28 (T)); if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); } eprintf (" )");return 0; }
  in_ptr = save_in_ptr;
  if (skip_constructor_dc_option (T) >= 0) { in_ptr = save_in_ptr; eprintf (" ("); assert (!fetch_constructor_dc_option (T)); if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); } eprintf (" )");return 0; }
  in_ptr = save_in_ptr;
  return -1;
}
int fetch_type_decrypted_message (struct paramed_type *T) {
  if (in_remaining () < 4) { return -1;}
  if (multiline_output >= 2) { multiline_offset += multiline_offset_size; }
  eprintf (" (");
  int magic = fetch_int ();
  int res = -1;
  switch (magic) {
  case 0x204d3878: res = fetch_constructor_decrypted_message (T); break;
  case 0x73164160: res = fetch_constructor_decrypted_message_service (T); break;
  default: return -1;
  }
  if (res >= 0) {
    if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); }
    eprintf (" )");
  }
  return res;
}
int fetch_type_bare_decrypted_message (struct paramed_type *T) {
  int *save_in_ptr = in_ptr;
  if (multiline_output >= 2) { multiline_offset += multiline_offset_size; }
  if (skip_constructor_decrypted_message (T) >= 0) { in_ptr = save_in_ptr; eprintf (" ("); assert (!fetch_constructor_decrypted_message (T)); if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); } eprintf (" )");return 0; }
  in_ptr = save_in_ptr;
  if (skip_constructor_decrypted_message_service (T) >= 0) { in_ptr = save_in_ptr; eprintf (" ("); assert (!fetch_constructor_decrypted_message_service (T)); if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); } eprintf (" )");return 0; }
  in_ptr = save_in_ptr;
  return -1;
}
int fetch_type_decrypted_message_action (struct paramed_type *T) {
  if (in_remaining () < 4) { return -1;}
  if (multiline_output >= 2) { multiline_offset += multiline_offset_size; }
  eprintf (" (");
  int magic = fetch_int ();
  int res = -1;
  switch (magic) {
  case 0xa1733aec: res = fetch_constructor_decrypted_message_action_set_message_t_t_l (T); break;
  case 0x0c4f40be: res = fetch_constructor_decrypted_message_action_read_messages (T); break;
  case 0x65614304: res = fetch_constructor_decrypted_message_action_delete_messages (T); break;
  case 0x8ac1f475: res = fetch_constructor_decrypted_message_action_screenshot_messages (T); break;
  case 0x6719e45c: res = fetch_constructor_decrypted_message_action_flush_history (T); break;
  case 0x511110b0: res = fetch_constructor_decrypted_message_action_resend (T); break;
  case 0xf3048883: res = fetch_constructor_decrypted_message_action_notify_layer (T); break;
  case 0xccb27641: res = fetch_constructor_decrypted_message_action_typing (T); break;
  case 0xf3c9611b: res = fetch_constructor_decrypted_message_action_request_key (T); break;
  case 0x6fe1735b: res = fetch_constructor_decrypted_message_action_accept_key (T); break;
  case 0xdd05ec6b: res = fetch_constructor_decrypted_message_action_abort_key (T); break;
  case 0xec2e0b9b: res = fetch_constructor_decrypted_message_action_commit_key (T); break;
  case 0xa82fdd63: res = fetch_constructor_decrypted_message_action_noop (T); break;
  default: return -1;
  }
  if (res >= 0) {
    if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); }
    eprintf (" )");
  }
  return res;
}
int fetch_type_bare_decrypted_message_action (struct paramed_type *T) {
  int *save_in_ptr = in_ptr;
  if (multiline_output >= 2) { multiline_offset += multiline_offset_size; }
  if (skip_constructor_decrypted_message_action_set_message_t_t_l (T) >= 0) { in_ptr = save_in_ptr; eprintf (" ("); assert (!fetch_constructor_decrypted_message_action_set_message_t_t_l (T)); if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); } eprintf (" )");return 0; }
  in_ptr = save_in_ptr;
  if (skip_constructor_decrypted_message_action_read_messages (T) >= 0) { in_ptr = save_in_ptr; eprintf (" ("); assert (!fetch_constructor_decrypted_message_action_read_messages (T)); if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); } eprintf (" )");return 0; }
  in_ptr = save_in_ptr;
  if (skip_constructor_decrypted_message_action_delete_messages (T) >= 0) { in_ptr = save_in_ptr; eprintf (" ("); assert (!fetch_constructor_decrypted_message_action_delete_messages (T)); if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); } eprintf (" )");return 0; }
  in_ptr = save_in_ptr;
  if (skip_constructor_decrypted_message_action_screenshot_messages (T) >= 0) { in_ptr = save_in_ptr; eprintf (" ("); assert (!fetch_constructor_decrypted_message_action_screenshot_messages (T)); if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); } eprintf (" )");return 0; }
  in_ptr = save_in_ptr;
  if (skip_constructor_decrypted_message_action_flush_history (T) >= 0) { in_ptr = save_in_ptr; eprintf (" ("); assert (!fetch_constructor_decrypted_message_action_flush_history (T)); if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); } eprintf (" )");return 0; }
  in_ptr = save_in_ptr;
  if (skip_constructor_decrypted_message_action_resend (T) >= 0) { in_ptr = save_in_ptr; eprintf (" ("); assert (!fetch_constructor_decrypted_message_action_resend (T)); if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); } eprintf (" )");return 0; }
  in_ptr = save_in_ptr;
  if (skip_constructor_decrypted_message_action_notify_layer (T) >= 0) { in_ptr = save_in_ptr; eprintf (" ("); assert (!fetch_constructor_decrypted_message_action_notify_layer (T)); if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); } eprintf (" )");return 0; }
  in_ptr = save_in_ptr;
  if (skip_constructor_decrypted_message_action_typing (T) >= 0) { in_ptr = save_in_ptr; eprintf (" ("); assert (!fetch_constructor_decrypted_message_action_typing (T)); if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); } eprintf (" )");return 0; }
  in_ptr = save_in_ptr;
  if (skip_constructor_decrypted_message_action_request_key (T) >= 0) { in_ptr = save_in_ptr; eprintf (" ("); assert (!fetch_constructor_decrypted_message_action_request_key (T)); if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); } eprintf (" )");return 0; }
  in_ptr = save_in_ptr;
  if (skip_constructor_decrypted_message_action_accept_key (T) >= 0) { in_ptr = save_in_ptr; eprintf (" ("); assert (!fetch_constructor_decrypted_message_action_accept_key (T)); if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); } eprintf (" )");return 0; }
  in_ptr = save_in_ptr;
  if (skip_constructor_decrypted_message_action_abort_key (T) >= 0) { in_ptr = save_in_ptr; eprintf (" ("); assert (!fetch_constructor_decrypted_message_action_abort_key (T)); if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); } eprintf (" )");return 0; }
  in_ptr = save_in_ptr;
  if (skip_constructor_decrypted_message_action_commit_key (T) >= 0) { in_ptr = save_in_ptr; eprintf (" ("); assert (!fetch_constructor_decrypted_message_action_commit_key (T)); if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); } eprintf (" )");return 0; }
  in_ptr = save_in_ptr;
  if (skip_constructor_decrypted_message_action_noop (T) >= 0) { in_ptr = save_in_ptr; eprintf (" ("); assert (!fetch_constructor_decrypted_message_action_noop (T)); if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); } eprintf (" )");return 0; }
  in_ptr = save_in_ptr;
  return -1;
}
int fetch_type_decrypted_message_layer (struct paramed_type *T) {
  if (in_remaining () < 4) { return -1;}
  if (multiline_output >= 2) { multiline_offset += multiline_offset_size; }
  eprintf (" (");
  int magic = fetch_int ();
  int res = -1;
  switch (magic) {
  case 0x1be31789: res = fetch_constructor_decrypted_message_layer (T); break;
  default: return -1;
  }
  if (res >= 0) {
    if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); }
    eprintf (" )");
  }
  return res;
}
int fetch_type_bare_decrypted_message_layer (struct paramed_type *T) {
  if (multiline_output >= 2) { multiline_offset += multiline_offset_size; }
  eprintf (" (");
  if (fetch_constructor_decrypted_message_layer (T) >= 0) { if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); } eprintf (" )");return 0; }
  return -1;
}
int fetch_type_decrypted_message_media (struct paramed_type *T) {
  if (in_remaining () < 4) { return -1;}
  if (multiline_output >= 2) { multiline_offset += multiline_offset_size; }
  eprintf (" (");
  int magic = fetch_int ();
  int res = -1;
  switch (magic) {
  case 0x089f5c4a: res = fetch_constructor_decrypted_message_media_empty (T); break;
  case 0x32798a8c: res = fetch_constructor_decrypted_message_media_photo (T); break;
  case 0x35480a59: res = fetch_constructor_decrypted_message_media_geo_point (T); break;
  case 0x588a0a97: res = fetch_constructor_decrypted_message_media_contact (T); break;
  case 0xb095434b: res = fetch_constructor_decrypted_message_media_document (T); break;
  case 0x524a415d: res = fetch_constructor_decrypted_message_media_video (T); break;
  case 0x57e0a9cb: res = fetch_constructor_decrypted_message_media_audio (T); break;
  case 0xfa95b0dd: res = fetch_constructor_decrypted_message_media_external_document (T); break;
  case 0x4cee6ef3: res = fetch_constructor_decrypted_message_media_video_l12 (T); break;
  case 0x6080758f: res = fetch_constructor_decrypted_message_media_audio_l12 (T); break;
  default: return -1;
  }
  if (res >= 0) {
    if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); }
    eprintf (" )");
  }
  return res;
}
int fetch_type_bare_decrypted_message_media (struct paramed_type *T) {
  int *save_in_ptr = in_ptr;
  if (multiline_output >= 2) { multiline_offset += multiline_offset_size; }
  if (skip_constructor_decrypted_message_media_empty (T) >= 0) { in_ptr = save_in_ptr; eprintf (" ("); assert (!fetch_constructor_decrypted_message_media_empty (T)); if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); } eprintf (" )");return 0; }
  in_ptr = save_in_ptr;
  if (skip_constructor_decrypted_message_media_photo (T) >= 0) { in_ptr = save_in_ptr; eprintf (" ("); assert (!fetch_constructor_decrypted_message_media_photo (T)); if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); } eprintf (" )");return 0; }
  in_ptr = save_in_ptr;
  if (skip_constructor_decrypted_message_media_geo_point (T) >= 0) { in_ptr = save_in_ptr; eprintf (" ("); assert (!fetch_constructor_decrypted_message_media_geo_point (T)); if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); } eprintf (" )");return 0; }
  in_ptr = save_in_ptr;
  if (skip_constructor_decrypted_message_media_contact (T) >= 0) { in_ptr = save_in_ptr; eprintf (" ("); assert (!fetch_constructor_decrypted_message_media_contact (T)); if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); } eprintf (" )");return 0; }
  in_ptr = save_in_ptr;
  if (skip_constructor_decrypted_message_media_document (T) >= 0) { in_ptr = save_in_ptr; eprintf (" ("); assert (!fetch_constructor_decrypted_message_media_document (T)); if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); } eprintf (" )");return 0; }
  in_ptr = save_in_ptr;
  if (skip_constructor_decrypted_message_media_video (T) >= 0) { in_ptr = save_in_ptr; eprintf (" ("); assert (!fetch_constructor_decrypted_message_media_video (T)); if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); } eprintf (" )");return 0; }
  in_ptr = save_in_ptr;
  if (skip_constructor_decrypted_message_media_audio (T) >= 0) { in_ptr = save_in_ptr; eprintf (" ("); assert (!fetch_constructor_decrypted_message_media_audio (T)); if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); } eprintf (" )");return 0; }
  in_ptr = save_in_ptr;
  if (skip_constructor_decrypted_message_media_external_document (T) >= 0) { in_ptr = save_in_ptr; eprintf (" ("); assert (!fetch_constructor_decrypted_message_media_external_document (T)); if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); } eprintf (" )");return 0; }
  in_ptr = save_in_ptr;
  if (skip_constructor_decrypted_message_media_video_l12 (T) >= 0) { in_ptr = save_in_ptr; eprintf (" ("); assert (!fetch_constructor_decrypted_message_media_video_l12 (T)); if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); } eprintf (" )");return 0; }
  in_ptr = save_in_ptr;
  if (skip_constructor_decrypted_message_media_audio_l12 (T) >= 0) { in_ptr = save_in_ptr; eprintf (" ("); assert (!fetch_constructor_decrypted_message_media_audio_l12 (T)); if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); } eprintf (" )");return 0; }
  in_ptr = save_in_ptr;
  return -1;
}
int fetch_type_dialog (struct paramed_type *T) {
  if (in_remaining () < 4) { return -1;}
  if (multiline_output >= 2) { multiline_offset += multiline_offset_size; }
  eprintf (" (");
  int magic = fetch_int ();
  int res = -1;
  switch (magic) {
  case 0xc1dd804a: res = fetch_constructor_dialog (T); break;
  default: return -1;
  }
  if (res >= 0) {
    if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); }
    eprintf (" )");
  }
  return res;
}
int fetch_type_bare_dialog (struct paramed_type *T) {
  if (multiline_output >= 2) { multiline_offset += multiline_offset_size; }
  eprintf (" (");
  if (fetch_constructor_dialog (T) >= 0) { if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); } eprintf (" )");return 0; }
  return -1;
}
int fetch_type_disabled_feature (struct paramed_type *T) {
  if (in_remaining () < 4) { return -1;}
  if (multiline_output >= 2) { multiline_offset += multiline_offset_size; }
  eprintf (" (");
  int magic = fetch_int ();
  int res = -1;
  switch (magic) {
  case 0xae636f24: res = fetch_constructor_disabled_feature (T); break;
  default: return -1;
  }
  if (res >= 0) {
    if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); }
    eprintf (" )");
  }
  return res;
}
int fetch_type_bare_disabled_feature (struct paramed_type *T) {
  if (multiline_output >= 2) { multiline_offset += multiline_offset_size; }
  eprintf (" (");
  if (fetch_constructor_disabled_feature (T) >= 0) { if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); } eprintf (" )");return 0; }
  return -1;
}
int fetch_type_document (struct paramed_type *T) {
  if (in_remaining () < 4) { return -1;}
  if (multiline_output >= 2) { multiline_offset += multiline_offset_size; }
  eprintf (" (");
  int magic = fetch_int ();
  int res = -1;
  switch (magic) {
  case 0x36f8c871: res = fetch_constructor_document_empty (T); break;
  case 0xf9a39f4f: res = fetch_constructor_document (T); break;
  case 0x9efc6326: res = fetch_constructor_document_l19 (T); break;
  default: return -1;
  }
  if (res >= 0) {
    if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); }
    eprintf (" )");
  }
  return res;
}
int fetch_type_bare_document (struct paramed_type *T) {
  int *save_in_ptr = in_ptr;
  if (multiline_output >= 2) { multiline_offset += multiline_offset_size; }
  if (skip_constructor_document_empty (T) >= 0) { in_ptr = save_in_ptr; eprintf (" ("); assert (!fetch_constructor_document_empty (T)); if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); } eprintf (" )");return 0; }
  in_ptr = save_in_ptr;
  if (skip_constructor_document (T) >= 0) { in_ptr = save_in_ptr; eprintf (" ("); assert (!fetch_constructor_document (T)); if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); } eprintf (" )");return 0; }
  in_ptr = save_in_ptr;
  if (skip_constructor_document_l19 (T) >= 0) { in_ptr = save_in_ptr; eprintf (" ("); assert (!fetch_constructor_document_l19 (T)); if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); } eprintf (" )");return 0; }
  in_ptr = save_in_ptr;
  return -1;
}
int fetch_type_document_attribute (struct paramed_type *T) {
  if (in_remaining () < 4) { return -1;}
  if (multiline_output >= 2) { multiline_offset += multiline_offset_size; }
  eprintf (" (");
  int magic = fetch_int ();
  int res = -1;
  switch (magic) {
  case 0x6c37c15c: res = fetch_constructor_document_attribute_image_size (T); break;
  case 0x11b58939: res = fetch_constructor_document_attribute_animated (T); break;
  case 0x994c9882: res = fetch_constructor_document_attribute_sticker_l28 (T); break;
  case 0x3a556302: res = fetch_constructor_document_attribute_sticker (T); break;
  case 0x5910cccb: res = fetch_constructor_document_attribute_video (T); break;
  case 0x051448e5: res = fetch_constructor_document_attribute_audio (T); break;
  case 0x15590068: res = fetch_constructor_document_attribute_filename (T); break;
  default: return -1;
  }
  if (res >= 0) {
    if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); }
    eprintf (" )");
  }
  return res;
}
int fetch_type_bare_document_attribute (struct paramed_type *T) {
  int *save_in_ptr = in_ptr;
  if (multiline_output >= 2) { multiline_offset += multiline_offset_size; }
  if (skip_constructor_document_attribute_image_size (T) >= 0) { in_ptr = save_in_ptr; eprintf (" ("); assert (!fetch_constructor_document_attribute_image_size (T)); if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); } eprintf (" )");return 0; }
  in_ptr = save_in_ptr;
  if (skip_constructor_document_attribute_animated (T) >= 0) { in_ptr = save_in_ptr; eprintf (" ("); assert (!fetch_constructor_document_attribute_animated (T)); if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); } eprintf (" )");return 0; }
  in_ptr = save_in_ptr;
  if (skip_constructor_document_attribute_sticker_l28 (T) >= 0) { in_ptr = save_in_ptr; eprintf (" ("); assert (!fetch_constructor_document_attribute_sticker_l28 (T)); if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); } eprintf (" )");return 0; }
  in_ptr = save_in_ptr;
  if (skip_constructor_document_attribute_sticker (T) >= 0) { in_ptr = save_in_ptr; eprintf (" ("); assert (!fetch_constructor_document_attribute_sticker (T)); if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); } eprintf (" )");return 0; }
  in_ptr = save_in_ptr;
  if (skip_constructor_document_attribute_video (T) >= 0) { in_ptr = save_in_ptr; eprintf (" ("); assert (!fetch_constructor_document_attribute_video (T)); if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); } eprintf (" )");return 0; }
  in_ptr = save_in_ptr;
  if (skip_constructor_document_attribute_audio (T) >= 0) { in_ptr = save_in_ptr; eprintf (" ("); assert (!fetch_constructor_document_attribute_audio (T)); if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); } eprintf (" )");return 0; }
  in_ptr = save_in_ptr;
  if (skip_constructor_document_attribute_filename (T) >= 0) { in_ptr = save_in_ptr; eprintf (" ("); assert (!fetch_constructor_document_attribute_filename (T)); if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); } eprintf (" )");return 0; }
  in_ptr = save_in_ptr;
  return -1;
}
int fetch_type_double (struct paramed_type *T) {
  if (in_remaining () < 4) { return -1;}
  int magic = fetch_int ();
  int res = -1;
  switch (magic) {
  case 0x2210c154: res = fetch_constructor_double (T); break;
  default: return -1;
  }
  return res;
}
int fetch_type_bare_double (struct paramed_type *T) {
  if (fetch_constructor_double (T) >= 0) { return 0; }
  return -1;
}
int fetch_type_encrypted_chat (struct paramed_type *T) {
  if (in_remaining () < 4) { return -1;}
  if (multiline_output >= 2) { multiline_offset += multiline_offset_size; }
  eprintf (" (");
  int magic = fetch_int ();
  int res = -1;
  switch (magic) {
  case 0xab7ec0a0: res = fetch_constructor_encrypted_chat_empty (T); break;
  case 0x3bf703dc: res = fetch_constructor_encrypted_chat_waiting (T); break;
  case 0xc878527e: res = fetch_constructor_encrypted_chat_requested (T); break;
  case 0xfa56ce36: res = fetch_constructor_encrypted_chat (T); break;
  case 0x13d6dd27: res = fetch_constructor_encrypted_chat_discarded (T); break;
  default: return -1;
  }
  if (res >= 0) {
    if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); }
    eprintf (" )");
  }
  return res;
}
int fetch_type_bare_encrypted_chat (struct paramed_type *T) {
  int *save_in_ptr = in_ptr;
  if (multiline_output >= 2) { multiline_offset += multiline_offset_size; }
  if (skip_constructor_encrypted_chat_empty (T) >= 0) { in_ptr = save_in_ptr; eprintf (" ("); assert (!fetch_constructor_encrypted_chat_empty (T)); if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); } eprintf (" )");return 0; }
  in_ptr = save_in_ptr;
  if (skip_constructor_encrypted_chat_waiting (T) >= 0) { in_ptr = save_in_ptr; eprintf (" ("); assert (!fetch_constructor_encrypted_chat_waiting (T)); if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); } eprintf (" )");return 0; }
  in_ptr = save_in_ptr;
  if (skip_constructor_encrypted_chat_requested (T) >= 0) { in_ptr = save_in_ptr; eprintf (" ("); assert (!fetch_constructor_encrypted_chat_requested (T)); if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); } eprintf (" )");return 0; }
  in_ptr = save_in_ptr;
  if (skip_constructor_encrypted_chat (T) >= 0) { in_ptr = save_in_ptr; eprintf (" ("); assert (!fetch_constructor_encrypted_chat (T)); if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); } eprintf (" )");return 0; }
  in_ptr = save_in_ptr;
  if (skip_constructor_encrypted_chat_discarded (T) >= 0) { in_ptr = save_in_ptr; eprintf (" ("); assert (!fetch_constructor_encrypted_chat_discarded (T)); if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); } eprintf (" )");return 0; }
  in_ptr = save_in_ptr;
  return -1;
}
int fetch_type_encrypted_file (struct paramed_type *T) {
  if (in_remaining () < 4) { return -1;}
  if (multiline_output >= 2) { multiline_offset += multiline_offset_size; }
  eprintf (" (");
  int magic = fetch_int ();
  int res = -1;
  switch (magic) {
  case 0xc21f497e: res = fetch_constructor_encrypted_file_empty (T); break;
  case 0x4a70994c: res = fetch_constructor_encrypted_file (T); break;
  default: return -1;
  }
  if (res >= 0) {
    if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); }
    eprintf (" )");
  }
  return res;
}
int fetch_type_bare_encrypted_file (struct paramed_type *T) {
  int *save_in_ptr = in_ptr;
  if (multiline_output >= 2) { multiline_offset += multiline_offset_size; }
  if (skip_constructor_encrypted_file_empty (T) >= 0) { in_ptr = save_in_ptr; eprintf (" ("); assert (!fetch_constructor_encrypted_file_empty (T)); if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); } eprintf (" )");return 0; }
  in_ptr = save_in_ptr;
  if (skip_constructor_encrypted_file (T) >= 0) { in_ptr = save_in_ptr; eprintf (" ("); assert (!fetch_constructor_encrypted_file (T)); if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); } eprintf (" )");return 0; }
  in_ptr = save_in_ptr;
  return -1;
}
int fetch_type_encrypted_message (struct paramed_type *T) {
  if (in_remaining () < 4) { return -1;}
  if (multiline_output >= 2) { multiline_offset += multiline_offset_size; }
  eprintf (" (");
  int magic = fetch_int ();
  int res = -1;
  switch (magic) {
  case 0xed18c118: res = fetch_constructor_encrypted_message (T); break;
  case 0x23734b06: res = fetch_constructor_encrypted_message_service (T); break;
  default: return -1;
  }
  if (res >= 0) {
    if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); }
    eprintf (" )");
  }
  return res;
}
int fetch_type_bare_encrypted_message (struct paramed_type *T) {
  int *save_in_ptr = in_ptr;
  if (multiline_output >= 2) { multiline_offset += multiline_offset_size; }
  if (skip_constructor_encrypted_message (T) >= 0) { in_ptr = save_in_ptr; eprintf (" ("); assert (!fetch_constructor_encrypted_message (T)); if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); } eprintf (" )");return 0; }
  in_ptr = save_in_ptr;
  if (skip_constructor_encrypted_message_service (T) >= 0) { in_ptr = save_in_ptr; eprintf (" ("); assert (!fetch_constructor_encrypted_message_service (T)); if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); } eprintf (" )");return 0; }
  in_ptr = save_in_ptr;
  return -1;
}
int fetch_type_error (struct paramed_type *T) {
  if (in_remaining () < 4) { return -1;}
  if (multiline_output >= 2) { multiline_offset += multiline_offset_size; }
  eprintf (" (");
  int magic = fetch_int ();
  int res = -1;
  switch (magic) {
  case 0xc4b9f9bb: res = fetch_constructor_error (T); break;
  default: return -1;
  }
  if (res >= 0) {
    if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); }
    eprintf (" )");
  }
  return res;
}
int fetch_type_bare_error (struct paramed_type *T) {
  if (multiline_output >= 2) { multiline_offset += multiline_offset_size; }
  eprintf (" (");
  if (fetch_constructor_error (T) >= 0) { if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); } eprintf (" )");return 0; }
  return -1;
}
int fetch_type_exported_chat_invite (struct paramed_type *T) {
  if (in_remaining () < 4) { return -1;}
  if (multiline_output >= 2) { multiline_offset += multiline_offset_size; }
  eprintf (" (");
  int magic = fetch_int ();
  int res = -1;
  switch (magic) {
  case 0x69df3769: res = fetch_constructor_chat_invite_empty (T); break;
  case 0xfc2e05bc: res = fetch_constructor_chat_invite_exported (T); break;
  default: return -1;
  }
  if (res >= 0) {
    if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); }
    eprintf (" )");
  }
  return res;
}
int fetch_type_bare_exported_chat_invite (struct paramed_type *T) {
  int *save_in_ptr = in_ptr;
  if (multiline_output >= 2) { multiline_offset += multiline_offset_size; }
  if (skip_constructor_chat_invite_empty (T) >= 0) { in_ptr = save_in_ptr; eprintf (" ("); assert (!fetch_constructor_chat_invite_empty (T)); if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); } eprintf (" )");return 0; }
  in_ptr = save_in_ptr;
  if (skip_constructor_chat_invite_exported (T) >= 0) { in_ptr = save_in_ptr; eprintf (" ("); assert (!fetch_constructor_chat_invite_exported (T)); if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); } eprintf (" )");return 0; }
  in_ptr = save_in_ptr;
  return -1;
}
int fetch_type_file_location (struct paramed_type *T) {
  if (in_remaining () < 4) { return -1;}
  if (multiline_output >= 2) { multiline_offset += multiline_offset_size; }
  eprintf (" (");
  int magic = fetch_int ();
  int res = -1;
  switch (magic) {
  case 0x7c596b46: res = fetch_constructor_file_location_unavailable (T); break;
  case 0x53d69076: res = fetch_constructor_file_location (T); break;
  default: return -1;
  }
  if (res >= 0) {
    if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); }
    eprintf (" )");
  }
  return res;
}
int fetch_type_bare_file_location (struct paramed_type *T) {
  int *save_in_ptr = in_ptr;
  if (multiline_output >= 2) { multiline_offset += multiline_offset_size; }
  if (skip_constructor_file_location_unavailable (T) >= 0) { in_ptr = save_in_ptr; eprintf (" ("); assert (!fetch_constructor_file_location_unavailable (T)); if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); } eprintf (" )");return 0; }
  in_ptr = save_in_ptr;
  if (skip_constructor_file_location (T) >= 0) { in_ptr = save_in_ptr; eprintf (" ("); assert (!fetch_constructor_file_location (T)); if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); } eprintf (" )");return 0; }
  in_ptr = save_in_ptr;
  return -1;
}
int fetch_type_geo_chat_message (struct paramed_type *T) {
  if (in_remaining () < 4) { return -1;}
  if (multiline_output >= 2) { multiline_offset += multiline_offset_size; }
  eprintf (" (");
  int magic = fetch_int ();
  int res = -1;
  switch (magic) {
  case 0x60311a9b: res = fetch_constructor_geo_chat_message_empty (T); break;
  case 0x4505f8e1: res = fetch_constructor_geo_chat_message (T); break;
  case 0xd34fa24e: res = fetch_constructor_geo_chat_message_service (T); break;
  default: return -1;
  }
  if (res >= 0) {
    if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); }
    eprintf (" )");
  }
  return res;
}
int fetch_type_bare_geo_chat_message (struct paramed_type *T) {
  int *save_in_ptr = in_ptr;
  if (multiline_output >= 2) { multiline_offset += multiline_offset_size; }
  if (skip_constructor_geo_chat_message_empty (T) >= 0) { in_ptr = save_in_ptr; eprintf (" ("); assert (!fetch_constructor_geo_chat_message_empty (T)); if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); } eprintf (" )");return 0; }
  in_ptr = save_in_ptr;
  if (skip_constructor_geo_chat_message (T) >= 0) { in_ptr = save_in_ptr; eprintf (" ("); assert (!fetch_constructor_geo_chat_message (T)); if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); } eprintf (" )");return 0; }
  in_ptr = save_in_ptr;
  if (skip_constructor_geo_chat_message_service (T) >= 0) { in_ptr = save_in_ptr; eprintf (" ("); assert (!fetch_constructor_geo_chat_message_service (T)); if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); } eprintf (" )");return 0; }
  in_ptr = save_in_ptr;
  return -1;
}
int fetch_type_geo_point (struct paramed_type *T) {
  if (in_remaining () < 4) { return -1;}
  if (multiline_output >= 2) { multiline_offset += multiline_offset_size; }
  eprintf (" (");
  int magic = fetch_int ();
  int res = -1;
  switch (magic) {
  case 0x1117dd5f: res = fetch_constructor_geo_point_empty (T); break;
  case 0x2049d70c: res = fetch_constructor_geo_point (T); break;
  default: return -1;
  }
  if (res >= 0) {
    if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); }
    eprintf (" )");
  }
  return res;
}
int fetch_type_bare_geo_point (struct paramed_type *T) {
  int *save_in_ptr = in_ptr;
  if (multiline_output >= 2) { multiline_offset += multiline_offset_size; }
  if (skip_constructor_geo_point_empty (T) >= 0) { in_ptr = save_in_ptr; eprintf (" ("); assert (!fetch_constructor_geo_point_empty (T)); if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); } eprintf (" )");return 0; }
  in_ptr = save_in_ptr;
  if (skip_constructor_geo_point (T) >= 0) { in_ptr = save_in_ptr; eprintf (" ("); assert (!fetch_constructor_geo_point (T)); if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); } eprintf (" )");return 0; }
  in_ptr = save_in_ptr;
  return -1;
}
int fetch_type_imported_contact (struct paramed_type *T) {
  if (in_remaining () < 4) { return -1;}
  if (multiline_output >= 2) { multiline_offset += multiline_offset_size; }
  eprintf (" (");
  int magic = fetch_int ();
  int res = -1;
  switch (magic) {
  case 0xd0028438: res = fetch_constructor_imported_contact (T); break;
  default: return -1;
  }
  if (res >= 0) {
    if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); }
    eprintf (" )");
  }
  return res;
}
int fetch_type_bare_imported_contact (struct paramed_type *T) {
  if (multiline_output >= 2) { multiline_offset += multiline_offset_size; }
  eprintf (" (");
  if (fetch_constructor_imported_contact (T) >= 0) { if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); } eprintf (" )");return 0; }
  return -1;
}
int fetch_type_input_app_event (struct paramed_type *T) {
  if (in_remaining () < 4) { return -1;}
  if (multiline_output >= 2) { multiline_offset += multiline_offset_size; }
  eprintf (" (");
  int magic = fetch_int ();
  int res = -1;
  switch (magic) {
  case 0x770656a8: res = fetch_constructor_input_app_event (T); break;
  default: return -1;
  }
  if (res >= 0) {
    if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); }
    eprintf (" )");
  }
  return res;
}
int fetch_type_bare_input_app_event (struct paramed_type *T) {
  if (multiline_output >= 2) { multiline_offset += multiline_offset_size; }
  eprintf (" (");
  if (fetch_constructor_input_app_event (T) >= 0) { if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); } eprintf (" )");return 0; }
  return -1;
}
int fetch_type_input_audio (struct paramed_type *T) {
  if (in_remaining () < 4) { return -1;}
  if (multiline_output >= 2) { multiline_offset += multiline_offset_size; }
  eprintf (" (");
  int magic = fetch_int ();
  int res = -1;
  switch (magic) {
  case 0xd95adc84: res = fetch_constructor_input_audio_empty (T); break;
  case 0x77d440ff: res = fetch_constructor_input_audio (T); break;
  default: return -1;
  }
  if (res >= 0) {
    if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); }
    eprintf (" )");
  }
  return res;
}
int fetch_type_bare_input_audio (struct paramed_type *T) {
  int *save_in_ptr = in_ptr;
  if (multiline_output >= 2) { multiline_offset += multiline_offset_size; }
  if (skip_constructor_input_audio_empty (T) >= 0) { in_ptr = save_in_ptr; eprintf (" ("); assert (!fetch_constructor_input_audio_empty (T)); if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); } eprintf (" )");return 0; }
  in_ptr = save_in_ptr;
  if (skip_constructor_input_audio (T) >= 0) { in_ptr = save_in_ptr; eprintf (" ("); assert (!fetch_constructor_input_audio (T)); if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); } eprintf (" )");return 0; }
  in_ptr = save_in_ptr;
  return -1;
}
int fetch_type_input_chat_photo (struct paramed_type *T) {
  if (in_remaining () < 4) { return -1;}
  if (multiline_output >= 2) { multiline_offset += multiline_offset_size; }
  eprintf (" (");
  int magic = fetch_int ();
  int res = -1;
  switch (magic) {
  case 0x1ca48f57: res = fetch_constructor_input_chat_photo_empty (T); break;
  case 0x94254732: res = fetch_constructor_input_chat_uploaded_photo (T); break;
  case 0xb2e1bf08: res = fetch_constructor_input_chat_photo (T); break;
  default: return -1;
  }
  if (res >= 0) {
    if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); }
    eprintf (" )");
  }
  return res;
}
int fetch_type_bare_input_chat_photo (struct paramed_type *T) {
  int *save_in_ptr = in_ptr;
  if (multiline_output >= 2) { multiline_offset += multiline_offset_size; }
  if (skip_constructor_input_chat_photo_empty (T) >= 0) { in_ptr = save_in_ptr; eprintf (" ("); assert (!fetch_constructor_input_chat_photo_empty (T)); if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); } eprintf (" )");return 0; }
  in_ptr = save_in_ptr;
  if (skip_constructor_input_chat_uploaded_photo (T) >= 0) { in_ptr = save_in_ptr; eprintf (" ("); assert (!fetch_constructor_input_chat_uploaded_photo (T)); if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); } eprintf (" )");return 0; }
  in_ptr = save_in_ptr;
  if (skip_constructor_input_chat_photo (T) >= 0) { in_ptr = save_in_ptr; eprintf (" ("); assert (!fetch_constructor_input_chat_photo (T)); if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); } eprintf (" )");return 0; }
  in_ptr = save_in_ptr;
  return -1;
}
int fetch_type_input_contact (struct paramed_type *T) {
  if (in_remaining () < 4) { return -1;}
  if (multiline_output >= 2) { multiline_offset += multiline_offset_size; }
  eprintf (" (");
  int magic = fetch_int ();
  int res = -1;
  switch (magic) {
  case 0xf392b7f4: res = fetch_constructor_input_phone_contact (T); break;
  default: return -1;
  }
  if (res >= 0) {
    if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); }
    eprintf (" )");
  }
  return res;
}
int fetch_type_bare_input_contact (struct paramed_type *T) {
  if (multiline_output >= 2) { multiline_offset += multiline_offset_size; }
  eprintf (" (");
  if (fetch_constructor_input_phone_contact (T) >= 0) { if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); } eprintf (" )");return 0; }
  return -1;
}
int fetch_type_input_document (struct paramed_type *T) {
  if (in_remaining () < 4) { return -1;}
  if (multiline_output >= 2) { multiline_offset += multiline_offset_size; }
  eprintf (" (");
  int magic = fetch_int ();
  int res = -1;
  switch (magic) {
  case 0x72f0eaae: res = fetch_constructor_input_document_empty (T); break;
  case 0x18798952: res = fetch_constructor_input_document (T); break;
  default: return -1;
  }
  if (res >= 0) {
    if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); }
    eprintf (" )");
  }
  return res;
}
int fetch_type_bare_input_document (struct paramed_type *T) {
  int *save_in_ptr = in_ptr;
  if (multiline_output >= 2) { multiline_offset += multiline_offset_size; }
  if (skip_constructor_input_document_empty (T) >= 0) { in_ptr = save_in_ptr; eprintf (" ("); assert (!fetch_constructor_input_document_empty (T)); if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); } eprintf (" )");return 0; }
  in_ptr = save_in_ptr;
  if (skip_constructor_input_document (T) >= 0) { in_ptr = save_in_ptr; eprintf (" ("); assert (!fetch_constructor_input_document (T)); if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); } eprintf (" )");return 0; }
  in_ptr = save_in_ptr;
  return -1;
}
int fetch_type_input_encrypted_chat (struct paramed_type *T) {
  if (in_remaining () < 4) { return -1;}
  if (multiline_output >= 2) { multiline_offset += multiline_offset_size; }
  eprintf (" (");
  int magic = fetch_int ();
  int res = -1;
  switch (magic) {
  case 0xf141b5e1: res = fetch_constructor_input_encrypted_chat (T); break;
  default: return -1;
  }
  if (res >= 0) {
    if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); }
    eprintf (" )");
  }
  return res;
}
int fetch_type_bare_input_encrypted_chat (struct paramed_type *T) {
  if (multiline_output >= 2) { multiline_offset += multiline_offset_size; }
  eprintf (" (");
  if (fetch_constructor_input_encrypted_chat (T) >= 0) { if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); } eprintf (" )");return 0; }
  return -1;
}
int fetch_type_input_encrypted_file (struct paramed_type *T) {
  if (in_remaining () < 4) { return -1;}
  if (multiline_output >= 2) { multiline_offset += multiline_offset_size; }
  eprintf (" (");
  int magic = fetch_int ();
  int res = -1;
  switch (magic) {
  case 0x1837c364: res = fetch_constructor_input_encrypted_file_empty (T); break;
  case 0x64bd0306: res = fetch_constructor_input_encrypted_file_uploaded (T); break;
  case 0x5a17b5e5: res = fetch_constructor_input_encrypted_file (T); break;
  case 0x2dc173c8: res = fetch_constructor_input_encrypted_file_big_uploaded (T); break;
  default: return -1;
  }
  if (res >= 0) {
    if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); }
    eprintf (" )");
  }
  return res;
}
int fetch_type_bare_input_encrypted_file (struct paramed_type *T) {
  int *save_in_ptr = in_ptr;
  if (multiline_output >= 2) { multiline_offset += multiline_offset_size; }
  if (skip_constructor_input_encrypted_file_empty (T) >= 0) { in_ptr = save_in_ptr; eprintf (" ("); assert (!fetch_constructor_input_encrypted_file_empty (T)); if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); } eprintf (" )");return 0; }
  in_ptr = save_in_ptr;
  if (skip_constructor_input_encrypted_file_uploaded (T) >= 0) { in_ptr = save_in_ptr; eprintf (" ("); assert (!fetch_constructor_input_encrypted_file_uploaded (T)); if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); } eprintf (" )");return 0; }
  in_ptr = save_in_ptr;
  if (skip_constructor_input_encrypted_file (T) >= 0) { in_ptr = save_in_ptr; eprintf (" ("); assert (!fetch_constructor_input_encrypted_file (T)); if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); } eprintf (" )");return 0; }
  in_ptr = save_in_ptr;
  if (skip_constructor_input_encrypted_file_big_uploaded (T) >= 0) { in_ptr = save_in_ptr; eprintf (" ("); assert (!fetch_constructor_input_encrypted_file_big_uploaded (T)); if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); } eprintf (" )");return 0; }
  in_ptr = save_in_ptr;
  return -1;
}
int fetch_type_input_file (struct paramed_type *T) {
  if (in_remaining () < 4) { return -1;}
  if (multiline_output >= 2) { multiline_offset += multiline_offset_size; }
  eprintf (" (");
  int magic = fetch_int ();
  int res = -1;
  switch (magic) {
  case 0xf52ff27f: res = fetch_constructor_input_file (T); break;
  case 0xfa4f0bb5: res = fetch_constructor_input_file_big (T); break;
  default: return -1;
  }
  if (res >= 0) {
    if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); }
    eprintf (" )");
  }
  return res;
}
int fetch_type_bare_input_file (struct paramed_type *T) {
  int *save_in_ptr = in_ptr;
  if (multiline_output >= 2) { multiline_offset += multiline_offset_size; }
  if (skip_constructor_input_file (T) >= 0) { in_ptr = save_in_ptr; eprintf (" ("); assert (!fetch_constructor_input_file (T)); if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); } eprintf (" )");return 0; }
  in_ptr = save_in_ptr;
  if (skip_constructor_input_file_big (T) >= 0) { in_ptr = save_in_ptr; eprintf (" ("); assert (!fetch_constructor_input_file_big (T)); if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); } eprintf (" )");return 0; }
  in_ptr = save_in_ptr;
  return -1;
}
int fetch_type_input_file_location (struct paramed_type *T) {
  if (in_remaining () < 4) { return -1;}
  if (multiline_output >= 2) { multiline_offset += multiline_offset_size; }
  eprintf (" (");
  int magic = fetch_int ();
  int res = -1;
  switch (magic) {
  case 0x14637196: res = fetch_constructor_input_file_location (T); break;
  case 0x3d0364ec: res = fetch_constructor_input_video_file_location (T); break;
  case 0xf5235d55: res = fetch_constructor_input_encrypted_file_location (T); break;
  case 0x74dc404d: res = fetch_constructor_input_audio_file_location (T); break;
  case 0x4e45abe9: res = fetch_constructor_input_document_file_location (T); break;
  default: return -1;
  }
  if (res >= 0) {
    if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); }
    eprintf (" )");
  }
  return res;
}
int fetch_type_bare_input_file_location (struct paramed_type *T) {
  int *save_in_ptr = in_ptr;
  if (multiline_output >= 2) { multiline_offset += multiline_offset_size; }
  if (skip_constructor_input_file_location (T) >= 0) { in_ptr = save_in_ptr; eprintf (" ("); assert (!fetch_constructor_input_file_location (T)); if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); } eprintf (" )");return 0; }
  in_ptr = save_in_ptr;
  if (skip_constructor_input_video_file_location (T) >= 0) { in_ptr = save_in_ptr; eprintf (" ("); assert (!fetch_constructor_input_video_file_location (T)); if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); } eprintf (" )");return 0; }
  in_ptr = save_in_ptr;
  if (skip_constructor_input_encrypted_file_location (T) >= 0) { in_ptr = save_in_ptr; eprintf (" ("); assert (!fetch_constructor_input_encrypted_file_location (T)); if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); } eprintf (" )");return 0; }
  in_ptr = save_in_ptr;
  if (skip_constructor_input_audio_file_location (T) >= 0) { in_ptr = save_in_ptr; eprintf (" ("); assert (!fetch_constructor_input_audio_file_location (T)); if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); } eprintf (" )");return 0; }
  in_ptr = save_in_ptr;
  if (skip_constructor_input_document_file_location (T) >= 0) { in_ptr = save_in_ptr; eprintf (" ("); assert (!fetch_constructor_input_document_file_location (T)); if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); } eprintf (" )");return 0; }
  in_ptr = save_in_ptr;
  return -1;
}
int fetch_type_input_geo_chat (struct paramed_type *T) {
  if (in_remaining () < 4) { return -1;}
  if (multiline_output >= 2) { multiline_offset += multiline_offset_size; }
  eprintf (" (");
  int magic = fetch_int ();
  int res = -1;
  switch (magic) {
  case 0x74d456fa: res = fetch_constructor_input_geo_chat (T); break;
  default: return -1;
  }
  if (res >= 0) {
    if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); }
    eprintf (" )");
  }
  return res;
}
int fetch_type_bare_input_geo_chat (struct paramed_type *T) {
  if (multiline_output >= 2) { multiline_offset += multiline_offset_size; }
  eprintf (" (");
  if (fetch_constructor_input_geo_chat (T) >= 0) { if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); } eprintf (" )");return 0; }
  return -1;
}
int fetch_type_input_geo_point (struct paramed_type *T) {
  if (in_remaining () < 4) { return -1;}
  if (multiline_output >= 2) { multiline_offset += multiline_offset_size; }
  eprintf (" (");
  int magic = fetch_int ();
  int res = -1;
  switch (magic) {
  case 0xe4c123d6: res = fetch_constructor_input_geo_point_empty (T); break;
  case 0xf3b7acc9: res = fetch_constructor_input_geo_point (T); break;
  default: return -1;
  }
  if (res >= 0) {
    if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); }
    eprintf (" )");
  }
  return res;
}
int fetch_type_bare_input_geo_point (struct paramed_type *T) {
  int *save_in_ptr = in_ptr;
  if (multiline_output >= 2) { multiline_offset += multiline_offset_size; }
  if (skip_constructor_input_geo_point_empty (T) >= 0) { in_ptr = save_in_ptr; eprintf (" ("); assert (!fetch_constructor_input_geo_point_empty (T)); if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); } eprintf (" )");return 0; }
  in_ptr = save_in_ptr;
  if (skip_constructor_input_geo_point (T) >= 0) { in_ptr = save_in_ptr; eprintf (" ("); assert (!fetch_constructor_input_geo_point (T)); if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); } eprintf (" )");return 0; }
  in_ptr = save_in_ptr;
  return -1;
}
int fetch_type_input_media (struct paramed_type *T) {
  if (in_remaining () < 4) { return -1;}
  if (multiline_output >= 2) { multiline_offset += multiline_offset_size; }
  eprintf (" (");
  int magic = fetch_int ();
  int res = -1;
  switch (magic) {
  case 0x9664f57f: res = fetch_constructor_input_media_empty (T); break;
  case 0xf7aff1c0: res = fetch_constructor_input_media_uploaded_photo (T); break;
  case 0xe9bfb4f3: res = fetch_constructor_input_media_photo (T); break;
  case 0xf9c44144: res = fetch_constructor_input_media_geo_point (T); break;
  case 0xa6e45987: res = fetch_constructor_input_media_contact (T); break;
  case 0xe13fd4bc: res = fetch_constructor_input_media_uploaded_video (T); break;
  case 0x96fb97dc: res = fetch_constructor_input_media_uploaded_thumb_video (T); break;
  case 0x936a4ebd: res = fetch_constructor_input_media_video (T); break;
  case 0x4e498cab: res = fetch_constructor_input_media_uploaded_audio (T); break;
  case 0x89938781: res = fetch_constructor_input_media_audio (T); break;
  case 0xffe76b78: res = fetch_constructor_input_media_uploaded_document (T); break;
  case 0x41481486: res = fetch_constructor_input_media_uploaded_thumb_document (T); break;
  case 0xd184e841: res = fetch_constructor_input_media_document (T); break;
  case 0x2827a81a: res = fetch_constructor_input_media_venue (T); break;
  default: return -1;
  }
  if (res >= 0) {
    if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); }
    eprintf (" )");
  }
  return res;
}
int fetch_type_bare_input_media (struct paramed_type *T) {
  int *save_in_ptr = in_ptr;
  if (multiline_output >= 2) { multiline_offset += multiline_offset_size; }
  if (skip_constructor_input_media_empty (T) >= 0) { in_ptr = save_in_ptr; eprintf (" ("); assert (!fetch_constructor_input_media_empty (T)); if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); } eprintf (" )");return 0; }
  in_ptr = save_in_ptr;
  if (skip_constructor_input_media_uploaded_photo (T) >= 0) { in_ptr = save_in_ptr; eprintf (" ("); assert (!fetch_constructor_input_media_uploaded_photo (T)); if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); } eprintf (" )");return 0; }
  in_ptr = save_in_ptr;
  if (skip_constructor_input_media_photo (T) >= 0) { in_ptr = save_in_ptr; eprintf (" ("); assert (!fetch_constructor_input_media_photo (T)); if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); } eprintf (" )");return 0; }
  in_ptr = save_in_ptr;
  if (skip_constructor_input_media_geo_point (T) >= 0) { in_ptr = save_in_ptr; eprintf (" ("); assert (!fetch_constructor_input_media_geo_point (T)); if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); } eprintf (" )");return 0; }
  in_ptr = save_in_ptr;
  if (skip_constructor_input_media_contact (T) >= 0) { in_ptr = save_in_ptr; eprintf (" ("); assert (!fetch_constructor_input_media_contact (T)); if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); } eprintf (" )");return 0; }
  in_ptr = save_in_ptr;
  if (skip_constructor_input_media_uploaded_video (T) >= 0) { in_ptr = save_in_ptr; eprintf (" ("); assert (!fetch_constructor_input_media_uploaded_video (T)); if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); } eprintf (" )");return 0; }
  in_ptr = save_in_ptr;
  if (skip_constructor_input_media_uploaded_thumb_video (T) >= 0) { in_ptr = save_in_ptr; eprintf (" ("); assert (!fetch_constructor_input_media_uploaded_thumb_video (T)); if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); } eprintf (" )");return 0; }
  in_ptr = save_in_ptr;
  if (skip_constructor_input_media_video (T) >= 0) { in_ptr = save_in_ptr; eprintf (" ("); assert (!fetch_constructor_input_media_video (T)); if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); } eprintf (" )");return 0; }
  in_ptr = save_in_ptr;
  if (skip_constructor_input_media_uploaded_audio (T) >= 0) { in_ptr = save_in_ptr; eprintf (" ("); assert (!fetch_constructor_input_media_uploaded_audio (T)); if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); } eprintf (" )");return 0; }
  in_ptr = save_in_ptr;
  if (skip_constructor_input_media_audio (T) >= 0) { in_ptr = save_in_ptr; eprintf (" ("); assert (!fetch_constructor_input_media_audio (T)); if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); } eprintf (" )");return 0; }
  in_ptr = save_in_ptr;
  if (skip_constructor_input_media_uploaded_document (T) >= 0) { in_ptr = save_in_ptr; eprintf (" ("); assert (!fetch_constructor_input_media_uploaded_document (T)); if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); } eprintf (" )");return 0; }
  in_ptr = save_in_ptr;
  if (skip_constructor_input_media_uploaded_thumb_document (T) >= 0) { in_ptr = save_in_ptr; eprintf (" ("); assert (!fetch_constructor_input_media_uploaded_thumb_document (T)); if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); } eprintf (" )");return 0; }
  in_ptr = save_in_ptr;
  if (skip_constructor_input_media_document (T) >= 0) { in_ptr = save_in_ptr; eprintf (" ("); assert (!fetch_constructor_input_media_document (T)); if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); } eprintf (" )");return 0; }
  in_ptr = save_in_ptr;
  if (skip_constructor_input_media_venue (T) >= 0) { in_ptr = save_in_ptr; eprintf (" ("); assert (!fetch_constructor_input_media_venue (T)); if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); } eprintf (" )");return 0; }
  in_ptr = save_in_ptr;
  return -1;
}
int fetch_type_input_notify_peer (struct paramed_type *T) {
  if (in_remaining () < 4) { return -1;}
  if (multiline_output >= 2) { multiline_offset += multiline_offset_size; }
  eprintf (" (");
  int magic = fetch_int ();
  int res = -1;
  switch (magic) {
  case 0xb8bc5b0c: res = fetch_constructor_input_notify_peer (T); break;
  case 0x193b4417: res = fetch_constructor_input_notify_users (T); break;
  case 0x4a95e84e: res = fetch_constructor_input_notify_chats (T); break;
  case 0xa429b886: res = fetch_constructor_input_notify_all (T); break;
  case 0x4d8ddec8: res = fetch_constructor_input_notify_geo_chat_peer (T); break;
  default: return -1;
  }
  if (res >= 0) {
    if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); }
    eprintf (" )");
  }
  return res;
}
int fetch_type_bare_input_notify_peer (struct paramed_type *T) {
  int *save_in_ptr = in_ptr;
  if (multiline_output >= 2) { multiline_offset += multiline_offset_size; }
  if (skip_constructor_input_notify_peer (T) >= 0) { in_ptr = save_in_ptr; eprintf (" ("); assert (!fetch_constructor_input_notify_peer (T)); if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); } eprintf (" )");return 0; }
  in_ptr = save_in_ptr;
  if (skip_constructor_input_notify_users (T) >= 0) { in_ptr = save_in_ptr; eprintf (" ("); assert (!fetch_constructor_input_notify_users (T)); if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); } eprintf (" )");return 0; }
  in_ptr = save_in_ptr;
  if (skip_constructor_input_notify_chats (T) >= 0) { in_ptr = save_in_ptr; eprintf (" ("); assert (!fetch_constructor_input_notify_chats (T)); if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); } eprintf (" )");return 0; }
  in_ptr = save_in_ptr;
  if (skip_constructor_input_notify_all (T) >= 0) { in_ptr = save_in_ptr; eprintf (" ("); assert (!fetch_constructor_input_notify_all (T)); if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); } eprintf (" )");return 0; }
  in_ptr = save_in_ptr;
  if (skip_constructor_input_notify_geo_chat_peer (T) >= 0) { in_ptr = save_in_ptr; eprintf (" ("); assert (!fetch_constructor_input_notify_geo_chat_peer (T)); if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); } eprintf (" )");return 0; }
  in_ptr = save_in_ptr;
  return -1;
}
int fetch_type_input_peer (struct paramed_type *T) {
  if (in_remaining () < 4) { return -1;}
  if (multiline_output >= 2) { multiline_offset += multiline_offset_size; }
  eprintf (" (");
  int magic = fetch_int ();
  int res = -1;
  switch (magic) {
  case 0x7f3b18ea: res = fetch_constructor_input_peer_empty (T); break;
  case 0x7da07ec9: res = fetch_constructor_input_peer_self (T); break;
  case 0x1023dbe8: res = fetch_constructor_input_peer_contact (T); break;
  case 0x9b447325: res = fetch_constructor_input_peer_foreign (T); break;
  case 0x179be863: res = fetch_constructor_input_peer_chat (T); break;
  default: return -1;
  }
  if (res >= 0) {
    if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); }
    eprintf (" )");
  }
  return res;
}
int fetch_type_bare_input_peer (struct paramed_type *T) {
  int *save_in_ptr = in_ptr;
  if (multiline_output >= 2) { multiline_offset += multiline_offset_size; }
  if (skip_constructor_input_peer_empty (T) >= 0) { in_ptr = save_in_ptr; eprintf (" ("); assert (!fetch_constructor_input_peer_empty (T)); if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); } eprintf (" )");return 0; }
  in_ptr = save_in_ptr;
  if (skip_constructor_input_peer_self (T) >= 0) { in_ptr = save_in_ptr; eprintf (" ("); assert (!fetch_constructor_input_peer_self (T)); if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); } eprintf (" )");return 0; }
  in_ptr = save_in_ptr;
  if (skip_constructor_input_peer_contact (T) >= 0) { in_ptr = save_in_ptr; eprintf (" ("); assert (!fetch_constructor_input_peer_contact (T)); if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); } eprintf (" )");return 0; }
  in_ptr = save_in_ptr;
  if (skip_constructor_input_peer_foreign (T) >= 0) { in_ptr = save_in_ptr; eprintf (" ("); assert (!fetch_constructor_input_peer_foreign (T)); if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); } eprintf (" )");return 0; }
  in_ptr = save_in_ptr;
  if (skip_constructor_input_peer_chat (T) >= 0) { in_ptr = save_in_ptr; eprintf (" ("); assert (!fetch_constructor_input_peer_chat (T)); if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); } eprintf (" )");return 0; }
  in_ptr = save_in_ptr;
  return -1;
}
int fetch_type_input_peer_notify_events (struct paramed_type *T) {
  if (in_remaining () < 4) { return -1;}
  if (multiline_output >= 2) { multiline_offset += multiline_offset_size; }
  eprintf (" (");
  int magic = fetch_int ();
  int res = -1;
  switch (magic) {
  case 0xf03064d8: res = fetch_constructor_input_peer_notify_events_empty (T); break;
  case 0xe86a2c74: res = fetch_constructor_input_peer_notify_events_all (T); break;
  default: return -1;
  }
  if (res >= 0) {
    if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); }
    eprintf (" )");
  }
  return res;
}
int fetch_type_bare_input_peer_notify_events (struct paramed_type *T) {
  int *save_in_ptr = in_ptr;
  if (multiline_output >= 2) { multiline_offset += multiline_offset_size; }
  if (skip_constructor_input_peer_notify_events_empty (T) >= 0) { in_ptr = save_in_ptr; eprintf (" ("); assert (!fetch_constructor_input_peer_notify_events_empty (T)); if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); } eprintf (" )");return 0; }
  in_ptr = save_in_ptr;
  if (skip_constructor_input_peer_notify_events_all (T) >= 0) { in_ptr = save_in_ptr; eprintf (" ("); assert (!fetch_constructor_input_peer_notify_events_all (T)); if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); } eprintf (" )");return 0; }
  in_ptr = save_in_ptr;
  return -1;
}
int fetch_type_input_peer_notify_settings (struct paramed_type *T) {
  if (in_remaining () < 4) { return -1;}
  if (multiline_output >= 2) { multiline_offset += multiline_offset_size; }
  eprintf (" (");
  int magic = fetch_int ();
  int res = -1;
  switch (magic) {
  case 0x46a2ce98: res = fetch_constructor_input_peer_notify_settings (T); break;
  default: return -1;
  }
  if (res >= 0) {
    if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); }
    eprintf (" )");
  }
  return res;
}
int fetch_type_bare_input_peer_notify_settings (struct paramed_type *T) {
  if (multiline_output >= 2) { multiline_offset += multiline_offset_size; }
  eprintf (" (");
  if (fetch_constructor_input_peer_notify_settings (T) >= 0) { if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); } eprintf (" )");return 0; }
  return -1;
}
int fetch_type_input_photo (struct paramed_type *T) {
  if (in_remaining () < 4) { return -1;}
  if (multiline_output >= 2) { multiline_offset += multiline_offset_size; }
  eprintf (" (");
  int magic = fetch_int ();
  int res = -1;
  switch (magic) {
  case 0x1cd7bf0d: res = fetch_constructor_input_photo_empty (T); break;
  case 0xfb95c6c4: res = fetch_constructor_input_photo (T); break;
  default: return -1;
  }
  if (res >= 0) {
    if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); }
    eprintf (" )");
  }
  return res;
}
int fetch_type_bare_input_photo (struct paramed_type *T) {
  int *save_in_ptr = in_ptr;
  if (multiline_output >= 2) { multiline_offset += multiline_offset_size; }
  if (skip_constructor_input_photo_empty (T) >= 0) { in_ptr = save_in_ptr; eprintf (" ("); assert (!fetch_constructor_input_photo_empty (T)); if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); } eprintf (" )");return 0; }
  in_ptr = save_in_ptr;
  if (skip_constructor_input_photo (T) >= 0) { in_ptr = save_in_ptr; eprintf (" ("); assert (!fetch_constructor_input_photo (T)); if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); } eprintf (" )");return 0; }
  in_ptr = save_in_ptr;
  return -1;
}
int fetch_type_input_photo_crop (struct paramed_type *T) {
  if (in_remaining () < 4) { return -1;}
  if (multiline_output >= 2) { multiline_offset += multiline_offset_size; }
  eprintf (" (");
  int magic = fetch_int ();
  int res = -1;
  switch (magic) {
  case 0xade6b004: res = fetch_constructor_input_photo_crop_auto (T); break;
  case 0xd9915325: res = fetch_constructor_input_photo_crop (T); break;
  default: return -1;
  }
  if (res >= 0) {
    if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); }
    eprintf (" )");
  }
  return res;
}
int fetch_type_bare_input_photo_crop (struct paramed_type *T) {
  int *save_in_ptr = in_ptr;
  if (multiline_output >= 2) { multiline_offset += multiline_offset_size; }
  if (skip_constructor_input_photo_crop_auto (T) >= 0) { in_ptr = save_in_ptr; eprintf (" ("); assert (!fetch_constructor_input_photo_crop_auto (T)); if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); } eprintf (" )");return 0; }
  in_ptr = save_in_ptr;
  if (skip_constructor_input_photo_crop (T) >= 0) { in_ptr = save_in_ptr; eprintf (" ("); assert (!fetch_constructor_input_photo_crop (T)); if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); } eprintf (" )");return 0; }
  in_ptr = save_in_ptr;
  return -1;
}
int fetch_type_input_privacy_key (struct paramed_type *T) {
  if (in_remaining () < 4) { return -1;}
  if (multiline_output >= 2) { multiline_offset += multiline_offset_size; }
  eprintf (" (");
  int magic = fetch_int ();
  int res = -1;
  switch (magic) {
  case 0x4f96cb18: res = fetch_constructor_input_privacy_key_status_timestamp (T); break;
  default: return -1;
  }
  if (res >= 0) {
    if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); }
    eprintf (" )");
  }
  return res;
}
int fetch_type_bare_input_privacy_key (struct paramed_type *T) {
  if (multiline_output >= 2) { multiline_offset += multiline_offset_size; }
  eprintf (" (");
  if (fetch_constructor_input_privacy_key_status_timestamp (T) >= 0) { if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); } eprintf (" )");return 0; }
  return -1;
}
int fetch_type_input_privacy_rule (struct paramed_type *T) {
  if (in_remaining () < 4) { return -1;}
  if (multiline_output >= 2) { multiline_offset += multiline_offset_size; }
  eprintf (" (");
  int magic = fetch_int ();
  int res = -1;
  switch (magic) {
  case 0x0d09e07b: res = fetch_constructor_input_privacy_value_allow_contacts (T); break;
  case 0x184b35ce: res = fetch_constructor_input_privacy_value_allow_all (T); break;
  case 0x131cc67f: res = fetch_constructor_input_privacy_value_allow_users (T); break;
  case 0x0ba52007: res = fetch_constructor_input_privacy_value_disallow_contacts (T); break;
  case 0xd66b66c9: res = fetch_constructor_input_privacy_value_disallow_all (T); break;
  case 0x90110467: res = fetch_constructor_input_privacy_value_disallow_users (T); break;
  default: return -1;
  }
  if (res >= 0) {
    if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); }
    eprintf (" )");
  }
  return res;
}
int fetch_type_bare_input_privacy_rule (struct paramed_type *T) {
  int *save_in_ptr = in_ptr;
  if (multiline_output >= 2) { multiline_offset += multiline_offset_size; }
  if (skip_constructor_input_privacy_value_allow_contacts (T) >= 0) { in_ptr = save_in_ptr; eprintf (" ("); assert (!fetch_constructor_input_privacy_value_allow_contacts (T)); if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); } eprintf (" )");return 0; }
  in_ptr = save_in_ptr;
  if (skip_constructor_input_privacy_value_allow_all (T) >= 0) { in_ptr = save_in_ptr; eprintf (" ("); assert (!fetch_constructor_input_privacy_value_allow_all (T)); if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); } eprintf (" )");return 0; }
  in_ptr = save_in_ptr;
  if (skip_constructor_input_privacy_value_allow_users (T) >= 0) { in_ptr = save_in_ptr; eprintf (" ("); assert (!fetch_constructor_input_privacy_value_allow_users (T)); if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); } eprintf (" )");return 0; }
  in_ptr = save_in_ptr;
  if (skip_constructor_input_privacy_value_disallow_contacts (T) >= 0) { in_ptr = save_in_ptr; eprintf (" ("); assert (!fetch_constructor_input_privacy_value_disallow_contacts (T)); if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); } eprintf (" )");return 0; }
  in_ptr = save_in_ptr;
  if (skip_constructor_input_privacy_value_disallow_all (T) >= 0) { in_ptr = save_in_ptr; eprintf (" ("); assert (!fetch_constructor_input_privacy_value_disallow_all (T)); if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); } eprintf (" )");return 0; }
  in_ptr = save_in_ptr;
  if (skip_constructor_input_privacy_value_disallow_users (T) >= 0) { in_ptr = save_in_ptr; eprintf (" ("); assert (!fetch_constructor_input_privacy_value_disallow_users (T)); if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); } eprintf (" )");return 0; }
  in_ptr = save_in_ptr;
  return -1;
}
int fetch_type_input_sticker_set (struct paramed_type *T) {
  if (in_remaining () < 4) { return -1;}
  if (multiline_output >= 2) { multiline_offset += multiline_offset_size; }
  eprintf (" (");
  int magic = fetch_int ();
  int res = -1;
  switch (magic) {
  case 0xffb62b95: res = fetch_constructor_input_sticker_set_empty (T); break;
  case 0x9de7a269: res = fetch_constructor_input_sticker_set_i_d (T); break;
  case 0x861cc8a0: res = fetch_constructor_input_sticker_set_short_name (T); break;
  default: return -1;
  }
  if (res >= 0) {
    if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); }
    eprintf (" )");
  }
  return res;
}
int fetch_type_bare_input_sticker_set (struct paramed_type *T) {
  int *save_in_ptr = in_ptr;
  if (multiline_output >= 2) { multiline_offset += multiline_offset_size; }
  if (skip_constructor_input_sticker_set_empty (T) >= 0) { in_ptr = save_in_ptr; eprintf (" ("); assert (!fetch_constructor_input_sticker_set_empty (T)); if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); } eprintf (" )");return 0; }
  in_ptr = save_in_ptr;
  if (skip_constructor_input_sticker_set_i_d (T) >= 0) { in_ptr = save_in_ptr; eprintf (" ("); assert (!fetch_constructor_input_sticker_set_i_d (T)); if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); } eprintf (" )");return 0; }
  in_ptr = save_in_ptr;
  if (skip_constructor_input_sticker_set_short_name (T) >= 0) { in_ptr = save_in_ptr; eprintf (" ("); assert (!fetch_constructor_input_sticker_set_short_name (T)); if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); } eprintf (" )");return 0; }
  in_ptr = save_in_ptr;
  return -1;
}
int fetch_type_input_user (struct paramed_type *T) {
  if (in_remaining () < 4) { return -1;}
  if (multiline_output >= 2) { multiline_offset += multiline_offset_size; }
  eprintf (" (");
  int magic = fetch_int ();
  int res = -1;
  switch (magic) {
  case 0xb98886cf: res = fetch_constructor_input_user_empty (T); break;
  case 0xf7c1b13f: res = fetch_constructor_input_user_self (T); break;
  case 0x86e94f65: res = fetch_constructor_input_user_contact (T); break;
  case 0x655e74ff: res = fetch_constructor_input_user_foreign (T); break;
  default: return -1;
  }
  if (res >= 0) {
    if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); }
    eprintf (" )");
  }
  return res;
}
int fetch_type_bare_input_user (struct paramed_type *T) {
  int *save_in_ptr = in_ptr;
  if (multiline_output >= 2) { multiline_offset += multiline_offset_size; }
  if (skip_constructor_input_user_empty (T) >= 0) { in_ptr = save_in_ptr; eprintf (" ("); assert (!fetch_constructor_input_user_empty (T)); if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); } eprintf (" )");return 0; }
  in_ptr = save_in_ptr;
  if (skip_constructor_input_user_self (T) >= 0) { in_ptr = save_in_ptr; eprintf (" ("); assert (!fetch_constructor_input_user_self (T)); if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); } eprintf (" )");return 0; }
  in_ptr = save_in_ptr;
  if (skip_constructor_input_user_contact (T) >= 0) { in_ptr = save_in_ptr; eprintf (" ("); assert (!fetch_constructor_input_user_contact (T)); if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); } eprintf (" )");return 0; }
  in_ptr = save_in_ptr;
  if (skip_constructor_input_user_foreign (T) >= 0) { in_ptr = save_in_ptr; eprintf (" ("); assert (!fetch_constructor_input_user_foreign (T)); if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); } eprintf (" )");return 0; }
  in_ptr = save_in_ptr;
  return -1;
}
int fetch_type_input_video (struct paramed_type *T) {
  if (in_remaining () < 4) { return -1;}
  if (multiline_output >= 2) { multiline_offset += multiline_offset_size; }
  eprintf (" (");
  int magic = fetch_int ();
  int res = -1;
  switch (magic) {
  case 0x5508ec75: res = fetch_constructor_input_video_empty (T); break;
  case 0xee579652: res = fetch_constructor_input_video (T); break;
  default: return -1;
  }
  if (res >= 0) {
    if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); }
    eprintf (" )");
  }
  return res;
}
int fetch_type_bare_input_video (struct paramed_type *T) {
  int *save_in_ptr = in_ptr;
  if (multiline_output >= 2) { multiline_offset += multiline_offset_size; }
  if (skip_constructor_input_video_empty (T) >= 0) { in_ptr = save_in_ptr; eprintf (" ("); assert (!fetch_constructor_input_video_empty (T)); if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); } eprintf (" )");return 0; }
  in_ptr = save_in_ptr;
  if (skip_constructor_input_video (T) >= 0) { in_ptr = save_in_ptr; eprintf (" ("); assert (!fetch_constructor_input_video (T)); if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); } eprintf (" )");return 0; }
  in_ptr = save_in_ptr;
  return -1;
}
int fetch_type_int (struct paramed_type *T) {
  if (in_remaining () < 4) { return -1;}
  int magic = fetch_int ();
  int res = -1;
  switch (magic) {
  case 0xa8509bda: res = fetch_constructor_int (T); break;
  default: return -1;
  }
  return res;
}
int fetch_type_bare_int (struct paramed_type *T) {
  if (fetch_constructor_int (T) >= 0) { return 0; }
  return -1;
}
int fetch_type_int128 (struct paramed_type *T) {
  if (in_remaining () < 4) { return -1;}
  if (multiline_output >= 2) { multiline_offset += multiline_offset_size; }
  eprintf (" (");
  int magic = fetch_int ();
  int res = -1;
  switch (magic) {
  case 0x7d36c439: res = fetch_constructor_int128 (T); break;
  default: return -1;
  }
  if (res >= 0) {
    if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); }
    eprintf (" )");
  }
  return res;
}
int fetch_type_bare_int128 (struct paramed_type *T) {
  if (multiline_output >= 2) { multiline_offset += multiline_offset_size; }
  eprintf (" (");
  if (fetch_constructor_int128 (T) >= 0) { if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); } eprintf (" )");return 0; }
  return -1;
}
int fetch_type_int256 (struct paramed_type *T) {
  if (in_remaining () < 4) { return -1;}
  if (multiline_output >= 2) { multiline_offset += multiline_offset_size; }
  eprintf (" (");
  int magic = fetch_int ();
  int res = -1;
  switch (magic) {
  case 0xf2c798b3: res = fetch_constructor_int256 (T); break;
  default: return -1;
  }
  if (res >= 0) {
    if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); }
    eprintf (" )");
  }
  return res;
}
int fetch_type_bare_int256 (struct paramed_type *T) {
  if (multiline_output >= 2) { multiline_offset += multiline_offset_size; }
  eprintf (" (");
  if (fetch_constructor_int256 (T) >= 0) { if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); } eprintf (" )");return 0; }
  return -1;
}
int fetch_type_keyboard_button (struct paramed_type *T) {
  if (in_remaining () < 4) { return -1;}
  int magic = fetch_int ();
  int res = -1;
  switch (magic) {
  case 0xa2fa4880: res = fetch_constructor_keyboard_button (T); break;
  default: return -1;
  }
  return res;
}
int fetch_type_bare_keyboard_button (struct paramed_type *T) {
  if (fetch_constructor_keyboard_button (T) >= 0) { return 0; }
  return -1;
}
int fetch_type_keyboard_button_row (struct paramed_type *T) {
  if (in_remaining () < 4) { return -1;}
  int magic = fetch_int ();
  int res = -1;
  switch (magic) {
  case 0x77608b83: res = fetch_constructor_keyboard_button_row (T); break;
  default: return -1;
  }
  return res;
}
int fetch_type_bare_keyboard_button_row (struct paramed_type *T) {
  if (fetch_constructor_keyboard_button_row (T) >= 0) { return 0; }
  return -1;
}
int fetch_type_long (struct paramed_type *T) {
  if (in_remaining () < 4) { return -1;}
  int magic = fetch_int ();
  int res = -1;
  switch (magic) {
  case 0x22076cba: res = fetch_constructor_long (T); break;
  default: return -1;
  }
  return res;
}
int fetch_type_bare_long (struct paramed_type *T) {
  if (fetch_constructor_long (T) >= 0) { return 0; }
  return -1;
}
int fetch_type_message (struct paramed_type *T) {
  if (in_remaining () < 4) { return -1;}
  if (multiline_output >= 2) { multiline_offset += multiline_offset_size; }
  eprintf (" (");
  int magic = fetch_int ();
  int res = -1;
  switch (magic) {
  case 0x83e5de54: res = fetch_constructor_message_empty (T); break;
  case 0xc3060325: res = fetch_constructor_message (T); break;
  case 0x1d86f70e: res = fetch_constructor_message_service (T); break;
  default: return -1;
  }
  if (res >= 0) {
    if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); }
    eprintf (" )");
  }
  return res;
}
int fetch_type_bare_message (struct paramed_type *T) {
  int *save_in_ptr = in_ptr;
  if (multiline_output >= 2) { multiline_offset += multiline_offset_size; }
  if (skip_constructor_message_empty (T) >= 0) { in_ptr = save_in_ptr; eprintf (" ("); assert (!fetch_constructor_message_empty (T)); if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); } eprintf (" )");return 0; }
  in_ptr = save_in_ptr;
  if (skip_constructor_message (T) >= 0) { in_ptr = save_in_ptr; eprintf (" ("); assert (!fetch_constructor_message (T)); if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); } eprintf (" )");return 0; }
  in_ptr = save_in_ptr;
  if (skip_constructor_message_service (T) >= 0) { in_ptr = save_in_ptr; eprintf (" ("); assert (!fetch_constructor_message_service (T)); if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); } eprintf (" )");return 0; }
  in_ptr = save_in_ptr;
  return -1;
}
int fetch_type_message_action (struct paramed_type *T) {
  if (in_remaining () < 4) { return -1;}
  if (multiline_output >= 2) { multiline_offset += multiline_offset_size; }
  eprintf (" (");
  int magic = fetch_int ();
  int res = -1;
  switch (magic) {
  case 0xb6aef7b0: res = fetch_constructor_message_action_empty (T); break;
  case 0xa6638b9a: res = fetch_constructor_message_action_chat_create (T); break;
  case 0xb5a1ce5a: res = fetch_constructor_message_action_chat_edit_title (T); break;
  case 0x7fcb13a8: res = fetch_constructor_message_action_chat_edit_photo (T); break;
  case 0x95e3fbef: res = fetch_constructor_message_action_chat_delete_photo (T); break;
  case 0x5e3cfc4b: res = fetch_constructor_message_action_chat_add_user (T); break;
  case 0xb2ae9b0c: res = fetch_constructor_message_action_chat_delete_user (T); break;
  case 0x6f038ebc: res = fetch_constructor_message_action_geo_chat_create (T); break;
  case 0x0c7d53de: res = fetch_constructor_message_action_geo_chat_checkin (T); break;
  case 0xf89cf5e8: res = fetch_constructor_message_action_chat_joined_by_link (T); break;
  default: return -1;
  }
  if (res >= 0) {
    if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); }
    eprintf (" )");
  }
  return res;
}
int fetch_type_bare_message_action (struct paramed_type *T) {
  int *save_in_ptr = in_ptr;
  if (multiline_output >= 2) { multiline_offset += multiline_offset_size; }
  if (skip_constructor_message_action_empty (T) >= 0) { in_ptr = save_in_ptr; eprintf (" ("); assert (!fetch_constructor_message_action_empty (T)); if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); } eprintf (" )");return 0; }
  in_ptr = save_in_ptr;
  if (skip_constructor_message_action_chat_create (T) >= 0) { in_ptr = save_in_ptr; eprintf (" ("); assert (!fetch_constructor_message_action_chat_create (T)); if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); } eprintf (" )");return 0; }
  in_ptr = save_in_ptr;
  if (skip_constructor_message_action_chat_edit_title (T) >= 0) { in_ptr = save_in_ptr; eprintf (" ("); assert (!fetch_constructor_message_action_chat_edit_title (T)); if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); } eprintf (" )");return 0; }
  in_ptr = save_in_ptr;
  if (skip_constructor_message_action_chat_edit_photo (T) >= 0) { in_ptr = save_in_ptr; eprintf (" ("); assert (!fetch_constructor_message_action_chat_edit_photo (T)); if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); } eprintf (" )");return 0; }
  in_ptr = save_in_ptr;
  if (skip_constructor_message_action_chat_delete_photo (T) >= 0) { in_ptr = save_in_ptr; eprintf (" ("); assert (!fetch_constructor_message_action_chat_delete_photo (T)); if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); } eprintf (" )");return 0; }
  in_ptr = save_in_ptr;
  if (skip_constructor_message_action_chat_add_user (T) >= 0) { in_ptr = save_in_ptr; eprintf (" ("); assert (!fetch_constructor_message_action_chat_add_user (T)); if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); } eprintf (" )");return 0; }
  in_ptr = save_in_ptr;
  if (skip_constructor_message_action_chat_delete_user (T) >= 0) { in_ptr = save_in_ptr; eprintf (" ("); assert (!fetch_constructor_message_action_chat_delete_user (T)); if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); } eprintf (" )");return 0; }
  in_ptr = save_in_ptr;
  if (skip_constructor_message_action_geo_chat_create (T) >= 0) { in_ptr = save_in_ptr; eprintf (" ("); assert (!fetch_constructor_message_action_geo_chat_create (T)); if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); } eprintf (" )");return 0; }
  in_ptr = save_in_ptr;
  if (skip_constructor_message_action_geo_chat_checkin (T) >= 0) { in_ptr = save_in_ptr; eprintf (" ("); assert (!fetch_constructor_message_action_geo_chat_checkin (T)); if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); } eprintf (" )");return 0; }
  in_ptr = save_in_ptr;
  if (skip_constructor_message_action_chat_joined_by_link (T) >= 0) { in_ptr = save_in_ptr; eprintf (" ("); assert (!fetch_constructor_message_action_chat_joined_by_link (T)); if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); } eprintf (" )");return 0; }
  in_ptr = save_in_ptr;
  return -1;
}
int fetch_type_message_media (struct paramed_type *T) {
  if (in_remaining () < 4) { return -1;}
  if (multiline_output >= 2) { multiline_offset += multiline_offset_size; }
  eprintf (" (");
  int magic = fetch_int ();
  int res = -1;
  switch (magic) {
  case 0x3ded6320: res = fetch_constructor_message_media_empty (T); break;
  case 0x3d8ce53d: res = fetch_constructor_message_media_photo (T); break;
  case 0x5bcf1675: res = fetch_constructor_message_media_video (T); break;
  case 0x56e0d474: res = fetch_constructor_message_media_geo (T); break;
  case 0x5e7d2f39: res = fetch_constructor_message_media_contact (T); break;
  case 0x9f84f49e: res = fetch_constructor_message_media_unsupported (T); break;
  case 0x2fda2204: res = fetch_constructor_message_media_document (T); break;
  case 0xc6b68300: res = fetch_constructor_message_media_audio (T); break;
  case 0xa32dd600: res = fetch_constructor_message_media_web_page (T); break;
  case 0x7912b71f: res = fetch_constructor_message_media_venue (T); break;
  case 0xc8c45a2a: res = fetch_constructor_message_media_photo_l27 (T); break;
  case 0xa2d24290: res = fetch_constructor_message_media_video_l27 (T); break;
  default: return -1;
  }
  if (res >= 0) {
    if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); }
    eprintf (" )");
  }
  return res;
}
int fetch_type_bare_message_media (struct paramed_type *T) {
  int *save_in_ptr = in_ptr;
  if (multiline_output >= 2) { multiline_offset += multiline_offset_size; }
  if (skip_constructor_message_media_empty (T) >= 0) { in_ptr = save_in_ptr; eprintf (" ("); assert (!fetch_constructor_message_media_empty (T)); if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); } eprintf (" )");return 0; }
  in_ptr = save_in_ptr;
  if (skip_constructor_message_media_photo (T) >= 0) { in_ptr = save_in_ptr; eprintf (" ("); assert (!fetch_constructor_message_media_photo (T)); if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); } eprintf (" )");return 0; }
  in_ptr = save_in_ptr;
  if (skip_constructor_message_media_video (T) >= 0) { in_ptr = save_in_ptr; eprintf (" ("); assert (!fetch_constructor_message_media_video (T)); if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); } eprintf (" )");return 0; }
  in_ptr = save_in_ptr;
  if (skip_constructor_message_media_geo (T) >= 0) { in_ptr = save_in_ptr; eprintf (" ("); assert (!fetch_constructor_message_media_geo (T)); if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); } eprintf (" )");return 0; }
  in_ptr = save_in_ptr;
  if (skip_constructor_message_media_contact (T) >= 0) { in_ptr = save_in_ptr; eprintf (" ("); assert (!fetch_constructor_message_media_contact (T)); if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); } eprintf (" )");return 0; }
  in_ptr = save_in_ptr;
  if (skip_constructor_message_media_unsupported (T) >= 0) { in_ptr = save_in_ptr; eprintf (" ("); assert (!fetch_constructor_message_media_unsupported (T)); if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); } eprintf (" )");return 0; }
  in_ptr = save_in_ptr;
  if (skip_constructor_message_media_document (T) >= 0) { in_ptr = save_in_ptr; eprintf (" ("); assert (!fetch_constructor_message_media_document (T)); if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); } eprintf (" )");return 0; }
  in_ptr = save_in_ptr;
  if (skip_constructor_message_media_audio (T) >= 0) { in_ptr = save_in_ptr; eprintf (" ("); assert (!fetch_constructor_message_media_audio (T)); if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); } eprintf (" )");return 0; }
  in_ptr = save_in_ptr;
  if (skip_constructor_message_media_web_page (T) >= 0) { in_ptr = save_in_ptr; eprintf (" ("); assert (!fetch_constructor_message_media_web_page (T)); if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); } eprintf (" )");return 0; }
  in_ptr = save_in_ptr;
  if (skip_constructor_message_media_venue (T) >= 0) { in_ptr = save_in_ptr; eprintf (" ("); assert (!fetch_constructor_message_media_venue (T)); if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); } eprintf (" )");return 0; }
  in_ptr = save_in_ptr;
  if (skip_constructor_message_media_photo_l27 (T) >= 0) { in_ptr = save_in_ptr; eprintf (" ("); assert (!fetch_constructor_message_media_photo_l27 (T)); if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); } eprintf (" )");return 0; }
  in_ptr = save_in_ptr;
  if (skip_constructor_message_media_video_l27 (T) >= 0) { in_ptr = save_in_ptr; eprintf (" ("); assert (!fetch_constructor_message_media_video_l27 (T)); if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); } eprintf (" )");return 0; }
  in_ptr = save_in_ptr;
  return -1;
}
int fetch_type_messages_filter (struct paramed_type *T) {
  if (in_remaining () < 4) { return -1;}
  if (multiline_output >= 2) { multiline_offset += multiline_offset_size; }
  eprintf (" (");
  int magic = fetch_int ();
  int res = -1;
  switch (magic) {
  case 0x57e2f66c: res = fetch_constructor_input_messages_filter_empty (T); break;
  case 0x9609a51c: res = fetch_constructor_input_messages_filter_photos (T); break;
  case 0x9fc00e65: res = fetch_constructor_input_messages_filter_video (T); break;
  case 0x56e9f0e4: res = fetch_constructor_input_messages_filter_photo_video (T); break;
  case 0xd95e73bb: res = fetch_constructor_input_messages_filter_photo_video_documents (T); break;
  case 0x9eddf188: res = fetch_constructor_input_messages_filter_document (T); break;
  case 0xcfc87522: res = fetch_constructor_input_messages_filter_audio (T); break;
  default: return -1;
  }
  if (res >= 0) {
    if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); }
    eprintf (" )");
  }
  return res;
}
int fetch_type_bare_messages_filter (struct paramed_type *T) {
  int *save_in_ptr = in_ptr;
  if (multiline_output >= 2) { multiline_offset += multiline_offset_size; }
  if (skip_constructor_input_messages_filter_empty (T) >= 0) { in_ptr = save_in_ptr; eprintf (" ("); assert (!fetch_constructor_input_messages_filter_empty (T)); if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); } eprintf (" )");return 0; }
  in_ptr = save_in_ptr;
  if (skip_constructor_input_messages_filter_photos (T) >= 0) { in_ptr = save_in_ptr; eprintf (" ("); assert (!fetch_constructor_input_messages_filter_photos (T)); if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); } eprintf (" )");return 0; }
  in_ptr = save_in_ptr;
  if (skip_constructor_input_messages_filter_video (T) >= 0) { in_ptr = save_in_ptr; eprintf (" ("); assert (!fetch_constructor_input_messages_filter_video (T)); if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); } eprintf (" )");return 0; }
  in_ptr = save_in_ptr;
  if (skip_constructor_input_messages_filter_photo_video (T) >= 0) { in_ptr = save_in_ptr; eprintf (" ("); assert (!fetch_constructor_input_messages_filter_photo_video (T)); if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); } eprintf (" )");return 0; }
  in_ptr = save_in_ptr;
  if (skip_constructor_input_messages_filter_photo_video_documents (T) >= 0) { in_ptr = save_in_ptr; eprintf (" ("); assert (!fetch_constructor_input_messages_filter_photo_video_documents (T)); if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); } eprintf (" )");return 0; }
  in_ptr = save_in_ptr;
  if (skip_constructor_input_messages_filter_document (T) >= 0) { in_ptr = save_in_ptr; eprintf (" ("); assert (!fetch_constructor_input_messages_filter_document (T)); if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); } eprintf (" )");return 0; }
  in_ptr = save_in_ptr;
  if (skip_constructor_input_messages_filter_audio (T) >= 0) { in_ptr = save_in_ptr; eprintf (" ("); assert (!fetch_constructor_input_messages_filter_audio (T)); if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); } eprintf (" )");return 0; }
  in_ptr = save_in_ptr;
  return -1;
}
int fetch_type_nearest_dc (struct paramed_type *T) {
  if (in_remaining () < 4) { return -1;}
  if (multiline_output >= 2) { multiline_offset += multiline_offset_size; }
  eprintf (" (");
  int magic = fetch_int ();
  int res = -1;
  switch (magic) {
  case 0x8e1a1775: res = fetch_constructor_nearest_dc (T); break;
  default: return -1;
  }
  if (res >= 0) {
    if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); }
    eprintf (" )");
  }
  return res;
}
int fetch_type_bare_nearest_dc (struct paramed_type *T) {
  if (multiline_output >= 2) { multiline_offset += multiline_offset_size; }
  eprintf (" (");
  if (fetch_constructor_nearest_dc (T) >= 0) { if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); } eprintf (" )");return 0; }
  return -1;
}
int fetch_type_notify_peer (struct paramed_type *T) {
  if (in_remaining () < 4) { return -1;}
  if (multiline_output >= 2) { multiline_offset += multiline_offset_size; }
  eprintf (" (");
  int magic = fetch_int ();
  int res = -1;
  switch (magic) {
  case 0x9fd40bd8: res = fetch_constructor_notify_peer (T); break;
  case 0xb4c83b4c: res = fetch_constructor_notify_users (T); break;
  case 0xc007cec3: res = fetch_constructor_notify_chats (T); break;
  case 0x74d07c60: res = fetch_constructor_notify_all (T); break;
  default: return -1;
  }
  if (res >= 0) {
    if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); }
    eprintf (" )");
  }
  return res;
}
int fetch_type_bare_notify_peer (struct paramed_type *T) {
  int *save_in_ptr = in_ptr;
  if (multiline_output >= 2) { multiline_offset += multiline_offset_size; }
  if (skip_constructor_notify_peer (T) >= 0) { in_ptr = save_in_ptr; eprintf (" ("); assert (!fetch_constructor_notify_peer (T)); if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); } eprintf (" )");return 0; }
  in_ptr = save_in_ptr;
  if (skip_constructor_notify_users (T) >= 0) { in_ptr = save_in_ptr; eprintf (" ("); assert (!fetch_constructor_notify_users (T)); if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); } eprintf (" )");return 0; }
  in_ptr = save_in_ptr;
  if (skip_constructor_notify_chats (T) >= 0) { in_ptr = save_in_ptr; eprintf (" ("); assert (!fetch_constructor_notify_chats (T)); if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); } eprintf (" )");return 0; }
  in_ptr = save_in_ptr;
  if (skip_constructor_notify_all (T) >= 0) { in_ptr = save_in_ptr; eprintf (" ("); assert (!fetch_constructor_notify_all (T)); if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); } eprintf (" )");return 0; }
  in_ptr = save_in_ptr;
  return -1;
}
int fetch_type_null (struct paramed_type *T) {
  if (in_remaining () < 4) { return -1;}
  if (multiline_output >= 2) { multiline_offset += multiline_offset_size; }
  eprintf (" (");
  int magic = fetch_int ();
  int res = -1;
  switch (magic) {
  case 0x56730bcc: res = fetch_constructor_null (T); break;
  default: return -1;
  }
  if (res >= 0) {
    if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); }
    eprintf (" )");
  }
  return res;
}
int fetch_type_bare_null (struct paramed_type *T) {
  if (multiline_output >= 2) { multiline_offset += multiline_offset_size; }
  eprintf (" (");
  if (fetch_constructor_null (T) >= 0) { if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); } eprintf (" )");return 0; }
  return -1;
}
int fetch_type_p_q_inner_data (struct paramed_type *T) {
  if (in_remaining () < 4) { return -1;}
  if (multiline_output >= 2) { multiline_offset += multiline_offset_size; }
  eprintf (" (");
  int magic = fetch_int ();
  int res = -1;
  switch (magic) {
  case 0x83c95aec: res = fetch_constructor_p_q_inner_data (T); break;
  case 0x3c6a84d4: res = fetch_constructor_p_q_inner_data_temp (T); break;
  default: return -1;
  }
  if (res >= 0) {
    if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); }
    eprintf (" )");
  }
  return res;
}
int fetch_type_bare_p_q_inner_data (struct paramed_type *T) {
  int *save_in_ptr = in_ptr;
  if (multiline_output >= 2) { multiline_offset += multiline_offset_size; }
  if (skip_constructor_p_q_inner_data (T) >= 0) { in_ptr = save_in_ptr; eprintf (" ("); assert (!fetch_constructor_p_q_inner_data (T)); if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); } eprintf (" )");return 0; }
  in_ptr = save_in_ptr;
  if (skip_constructor_p_q_inner_data_temp (T) >= 0) { in_ptr = save_in_ptr; eprintf (" ("); assert (!fetch_constructor_p_q_inner_data_temp (T)); if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); } eprintf (" )");return 0; }
  in_ptr = save_in_ptr;
  return -1;
}
int fetch_type_peer (struct paramed_type *T) {
  if (in_remaining () < 4) { return -1;}
  if (multiline_output >= 2) { multiline_offset += multiline_offset_size; }
  eprintf (" (");
  int magic = fetch_int ();
  int res = -1;
  switch (magic) {
  case 0x9db1bc6d: res = fetch_constructor_peer_user (T); break;
  case 0xbad0e5bb: res = fetch_constructor_peer_chat (T); break;
  default: return -1;
  }
  if (res >= 0) {
    if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); }
    eprintf (" )");
  }
  return res;
}
int fetch_type_bare_peer (struct paramed_type *T) {
  int *save_in_ptr = in_ptr;
  if (multiline_output >= 2) { multiline_offset += multiline_offset_size; }
  if (skip_constructor_peer_user (T) >= 0) { in_ptr = save_in_ptr; eprintf (" ("); assert (!fetch_constructor_peer_user (T)); if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); } eprintf (" )");return 0; }
  in_ptr = save_in_ptr;
  if (skip_constructor_peer_chat (T) >= 0) { in_ptr = save_in_ptr; eprintf (" ("); assert (!fetch_constructor_peer_chat (T)); if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); } eprintf (" )");return 0; }
  in_ptr = save_in_ptr;
  return -1;
}
int fetch_type_peer_notify_events (struct paramed_type *T) {
  if (in_remaining () < 4) { return -1;}
  if (multiline_output >= 2) { multiline_offset += multiline_offset_size; }
  eprintf (" (");
  int magic = fetch_int ();
  int res = -1;
  switch (magic) {
  case 0xadd53cb3: res = fetch_constructor_peer_notify_events_empty (T); break;
  case 0x6d1ded88: res = fetch_constructor_peer_notify_events_all (T); break;
  default: return -1;
  }
  if (res >= 0) {
    if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); }
    eprintf (" )");
  }
  return res;
}
int fetch_type_bare_peer_notify_events (struct paramed_type *T) {
  int *save_in_ptr = in_ptr;
  if (multiline_output >= 2) { multiline_offset += multiline_offset_size; }
  if (skip_constructor_peer_notify_events_empty (T) >= 0) { in_ptr = save_in_ptr; eprintf (" ("); assert (!fetch_constructor_peer_notify_events_empty (T)); if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); } eprintf (" )");return 0; }
  in_ptr = save_in_ptr;
  if (skip_constructor_peer_notify_events_all (T) >= 0) { in_ptr = save_in_ptr; eprintf (" ("); assert (!fetch_constructor_peer_notify_events_all (T)); if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); } eprintf (" )");return 0; }
  in_ptr = save_in_ptr;
  return -1;
}
int fetch_type_peer_notify_settings (struct paramed_type *T) {
  if (in_remaining () < 4) { return -1;}
  if (multiline_output >= 2) { multiline_offset += multiline_offset_size; }
  eprintf (" (");
  int magic = fetch_int ();
  int res = -1;
  switch (magic) {
  case 0x70a68512: res = fetch_constructor_peer_notify_settings_empty (T); break;
  case 0x8d5e11ee: res = fetch_constructor_peer_notify_settings (T); break;
  default: return -1;
  }
  if (res >= 0) {
    if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); }
    eprintf (" )");
  }
  return res;
}
int fetch_type_bare_peer_notify_settings (struct paramed_type *T) {
  int *save_in_ptr = in_ptr;
  if (multiline_output >= 2) { multiline_offset += multiline_offset_size; }
  if (skip_constructor_peer_notify_settings_empty (T) >= 0) { in_ptr = save_in_ptr; eprintf (" ("); assert (!fetch_constructor_peer_notify_settings_empty (T)); if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); } eprintf (" )");return 0; }
  in_ptr = save_in_ptr;
  if (skip_constructor_peer_notify_settings (T) >= 0) { in_ptr = save_in_ptr; eprintf (" ("); assert (!fetch_constructor_peer_notify_settings (T)); if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); } eprintf (" )");return 0; }
  in_ptr = save_in_ptr;
  return -1;
}
int fetch_type_photo (struct paramed_type *T) {
  if (in_remaining () < 4) { return -1;}
  if (multiline_output >= 2) { multiline_offset += multiline_offset_size; }
  eprintf (" (");
  int magic = fetch_int ();
  int res = -1;
  switch (magic) {
  case 0x2331b22d: res = fetch_constructor_photo_empty (T); break;
  case 0xc3838076: res = fetch_constructor_photo (T); break;
  case 0x22b56751: res = fetch_constructor_photo_l27 (T); break;
  default: return -1;
  }
  if (res >= 0) {
    if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); }
    eprintf (" )");
  }
  return res;
}
int fetch_type_bare_photo (struct paramed_type *T) {
  int *save_in_ptr = in_ptr;
  if (multiline_output >= 2) { multiline_offset += multiline_offset_size; }
  if (skip_constructor_photo_empty (T) >= 0) { in_ptr = save_in_ptr; eprintf (" ("); assert (!fetch_constructor_photo_empty (T)); if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); } eprintf (" )");return 0; }
  in_ptr = save_in_ptr;
  if (skip_constructor_photo (T) >= 0) { in_ptr = save_in_ptr; eprintf (" ("); assert (!fetch_constructor_photo (T)); if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); } eprintf (" )");return 0; }
  in_ptr = save_in_ptr;
  if (skip_constructor_photo_l27 (T) >= 0) { in_ptr = save_in_ptr; eprintf (" ("); assert (!fetch_constructor_photo_l27 (T)); if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); } eprintf (" )");return 0; }
  in_ptr = save_in_ptr;
  return -1;
}
int fetch_type_photo_size (struct paramed_type *T) {
  if (in_remaining () < 4) { return -1;}
  if (multiline_output >= 2) { multiline_offset += multiline_offset_size; }
  eprintf (" (");
  int magic = fetch_int ();
  int res = -1;
  switch (magic) {
  case 0x0e17e23c: res = fetch_constructor_photo_size_empty (T); break;
  case 0x77bfb61b: res = fetch_constructor_photo_size (T); break;
  case 0xe9a734fa: res = fetch_constructor_photo_cached_size (T); break;
  default: return -1;
  }
  if (res >= 0) {
    if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); }
    eprintf (" )");
  }
  return res;
}
int fetch_type_bare_photo_size (struct paramed_type *T) {
  int *save_in_ptr = in_ptr;
  if (multiline_output >= 2) { multiline_offset += multiline_offset_size; }
  if (skip_constructor_photo_size_empty (T) >= 0) { in_ptr = save_in_ptr; eprintf (" ("); assert (!fetch_constructor_photo_size_empty (T)); if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); } eprintf (" )");return 0; }
  in_ptr = save_in_ptr;
  if (skip_constructor_photo_size (T) >= 0) { in_ptr = save_in_ptr; eprintf (" ("); assert (!fetch_constructor_photo_size (T)); if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); } eprintf (" )");return 0; }
  in_ptr = save_in_ptr;
  if (skip_constructor_photo_cached_size (T) >= 0) { in_ptr = save_in_ptr; eprintf (" ("); assert (!fetch_constructor_photo_cached_size (T)); if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); } eprintf (" )");return 0; }
  in_ptr = save_in_ptr;
  return -1;
}
int fetch_type_privacy_key (struct paramed_type *T) {
  if (in_remaining () < 4) { return -1;}
  if (multiline_output >= 2) { multiline_offset += multiline_offset_size; }
  eprintf (" (");
  int magic = fetch_int ();
  int res = -1;
  switch (magic) {
  case 0xbc2eab30: res = fetch_constructor_privacy_key_status_timestamp (T); break;
  default: return -1;
  }
  if (res >= 0) {
    if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); }
    eprintf (" )");
  }
  return res;
}
int fetch_type_bare_privacy_key (struct paramed_type *T) {
  if (multiline_output >= 2) { multiline_offset += multiline_offset_size; }
  eprintf (" (");
  if (fetch_constructor_privacy_key_status_timestamp (T) >= 0) { if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); } eprintf (" )");return 0; }
  return -1;
}
int fetch_type_privacy_rule (struct paramed_type *T) {
  if (in_remaining () < 4) { return -1;}
  if (multiline_output >= 2) { multiline_offset += multiline_offset_size; }
  eprintf (" (");
  int magic = fetch_int ();
  int res = -1;
  switch (magic) {
  case 0xfffe1bac: res = fetch_constructor_privacy_value_allow_contacts (T); break;
  case 0x65427b82: res = fetch_constructor_privacy_value_allow_all (T); break;
  case 0x4d5bbe0c: res = fetch_constructor_privacy_value_allow_users (T); break;
  case 0xf888fa1a: res = fetch_constructor_privacy_value_disallow_contacts (T); break;
  case 0x8b73e763: res = fetch_constructor_privacy_value_disallow_all (T); break;
  case 0x0c7f49b7: res = fetch_constructor_privacy_value_disallow_users (T); break;
  default: return -1;
  }
  if (res >= 0) {
    if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); }
    eprintf (" )");
  }
  return res;
}
int fetch_type_bare_privacy_rule (struct paramed_type *T) {
  int *save_in_ptr = in_ptr;
  if (multiline_output >= 2) { multiline_offset += multiline_offset_size; }
  if (skip_constructor_privacy_value_allow_contacts (T) >= 0) { in_ptr = save_in_ptr; eprintf (" ("); assert (!fetch_constructor_privacy_value_allow_contacts (T)); if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); } eprintf (" )");return 0; }
  in_ptr = save_in_ptr;
  if (skip_constructor_privacy_value_allow_all (T) >= 0) { in_ptr = save_in_ptr; eprintf (" ("); assert (!fetch_constructor_privacy_value_allow_all (T)); if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); } eprintf (" )");return 0; }
  in_ptr = save_in_ptr;
  if (skip_constructor_privacy_value_allow_users (T) >= 0) { in_ptr = save_in_ptr; eprintf (" ("); assert (!fetch_constructor_privacy_value_allow_users (T)); if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); } eprintf (" )");return 0; }
  in_ptr = save_in_ptr;
  if (skip_constructor_privacy_value_disallow_contacts (T) >= 0) { in_ptr = save_in_ptr; eprintf (" ("); assert (!fetch_constructor_privacy_value_disallow_contacts (T)); if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); } eprintf (" )");return 0; }
  in_ptr = save_in_ptr;
  if (skip_constructor_privacy_value_disallow_all (T) >= 0) { in_ptr = save_in_ptr; eprintf (" ("); assert (!fetch_constructor_privacy_value_disallow_all (T)); if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); } eprintf (" )");return 0; }
  in_ptr = save_in_ptr;
  if (skip_constructor_privacy_value_disallow_users (T) >= 0) { in_ptr = save_in_ptr; eprintf (" ("); assert (!fetch_constructor_privacy_value_disallow_users (T)); if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); } eprintf (" )");return 0; }
  in_ptr = save_in_ptr;
  return -1;
}
int fetch_type_received_notify_message (struct paramed_type *T) {
  if (in_remaining () < 4) { return -1;}
  if (multiline_output >= 2) { multiline_offset += multiline_offset_size; }
  eprintf (" (");
  int magic = fetch_int ();
  int res = -1;
  switch (magic) {
  case 0xa384b779: res = fetch_constructor_received_notify_message (T); break;
  default: return -1;
  }
  if (res >= 0) {
    if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); }
    eprintf (" )");
  }
  return res;
}
int fetch_type_bare_received_notify_message (struct paramed_type *T) {
  if (multiline_output >= 2) { multiline_offset += multiline_offset_size; }
  eprintf (" (");
  if (fetch_constructor_received_notify_message (T) >= 0) { if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); } eprintf (" )");return 0; }
  return -1;
}
int fetch_type_reply_markup (struct paramed_type *T) {
  if (in_remaining () < 4) { return -1;}
  if (multiline_output >= 2) { multiline_offset += multiline_offset_size; }
  eprintf (" (");
  int magic = fetch_int ();
  int res = -1;
  switch (magic) {
  case 0xa03e5b85: res = fetch_constructor_reply_keyboard_hide (T); break;
  case 0xf4108aa0: res = fetch_constructor_reply_keyboard_force_reply (T); break;
  case 0x3502758c: res = fetch_constructor_reply_keyboard_markup (T); break;
  default: return -1;
  }
  if (res >= 0) {
    if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); }
    eprintf (" )");
  }
  return res;
}
int fetch_type_bare_reply_markup (struct paramed_type *T) {
  int *save_in_ptr = in_ptr;
  if (multiline_output >= 2) { multiline_offset += multiline_offset_size; }
  if (skip_constructor_reply_keyboard_hide (T) >= 0) { in_ptr = save_in_ptr; eprintf (" ("); assert (!fetch_constructor_reply_keyboard_hide (T)); if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); } eprintf (" )");return 0; }
  in_ptr = save_in_ptr;
  if (skip_constructor_reply_keyboard_force_reply (T) >= 0) { in_ptr = save_in_ptr; eprintf (" ("); assert (!fetch_constructor_reply_keyboard_force_reply (T)); if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); } eprintf (" )");return 0; }
  in_ptr = save_in_ptr;
  if (skip_constructor_reply_keyboard_markup (T) >= 0) { in_ptr = save_in_ptr; eprintf (" ("); assert (!fetch_constructor_reply_keyboard_markup (T)); if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); } eprintf (" )");return 0; }
  in_ptr = save_in_ptr;
  return -1;
}
int fetch_type_res_p_q (struct paramed_type *T) {
  if (in_remaining () < 4) { return -1;}
  if (multiline_output >= 2) { multiline_offset += multiline_offset_size; }
  eprintf (" (");
  int magic = fetch_int ();
  int res = -1;
  switch (magic) {
  case 0x05162463: res = fetch_constructor_res_p_q (T); break;
  default: return -1;
  }
  if (res >= 0) {
    if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); }
    eprintf (" )");
  }
  return res;
}
int fetch_type_bare_res_p_q (struct paramed_type *T) {
  if (multiline_output >= 2) { multiline_offset += multiline_offset_size; }
  eprintf (" (");
  if (fetch_constructor_res_p_q (T) >= 0) { if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); } eprintf (" )");return 0; }
  return -1;
}
int fetch_type_send_message_action (struct paramed_type *T) {
  if (in_remaining () < 4) { return -1;}
  if (multiline_output >= 2) { multiline_offset += multiline_offset_size; }
  eprintf (" (");
  int magic = fetch_int ();
  int res = -1;
  switch (magic) {
  case 0x16bf744e: res = fetch_constructor_send_message_typing_action (T); break;
  case 0xfd5ec8f5: res = fetch_constructor_send_message_cancel_action (T); break;
  case 0xa187d66f: res = fetch_constructor_send_message_record_video_action (T); break;
  case 0x92042ff7: res = fetch_constructor_send_message_upload_video_action_l27 (T); break;
  case 0xe9763aec: res = fetch_constructor_send_message_upload_video_action (T); break;
  case 0xd52f73f7: res = fetch_constructor_send_message_record_audio_action (T); break;
  case 0xe6ac8a6f: res = fetch_constructor_send_message_upload_audio_action_l27 (T); break;
  case 0xf351d7ab: res = fetch_constructor_send_message_upload_audio_action (T); break;
  case 0xd1d34a26: res = fetch_constructor_send_message_upload_photo_action (T); break;
  case 0x8faee98e: res = fetch_constructor_send_message_upload_document_action_l27 (T); break;
  case 0xaa0cd9e4: res = fetch_constructor_send_message_upload_document_action (T); break;
  case 0x176f8ba1: res = fetch_constructor_send_message_geo_location_action (T); break;
  case 0x628cbc6f: res = fetch_constructor_send_message_choose_contact_action (T); break;
  default: return -1;
  }
  if (res >= 0) {
    if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); }
    eprintf (" )");
  }
  return res;
}
int fetch_type_bare_send_message_action (struct paramed_type *T) {
  int *save_in_ptr = in_ptr;
  if (multiline_output >= 2) { multiline_offset += multiline_offset_size; }
  if (skip_constructor_send_message_typing_action (T) >= 0) { in_ptr = save_in_ptr; eprintf (" ("); assert (!fetch_constructor_send_message_typing_action (T)); if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); } eprintf (" )");return 0; }
  in_ptr = save_in_ptr;
  if (skip_constructor_send_message_cancel_action (T) >= 0) { in_ptr = save_in_ptr; eprintf (" ("); assert (!fetch_constructor_send_message_cancel_action (T)); if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); } eprintf (" )");return 0; }
  in_ptr = save_in_ptr;
  if (skip_constructor_send_message_record_video_action (T) >= 0) { in_ptr = save_in_ptr; eprintf (" ("); assert (!fetch_constructor_send_message_record_video_action (T)); if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); } eprintf (" )");return 0; }
  in_ptr = save_in_ptr;
  if (skip_constructor_send_message_upload_video_action_l27 (T) >= 0) { in_ptr = save_in_ptr; eprintf (" ("); assert (!fetch_constructor_send_message_upload_video_action_l27 (T)); if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); } eprintf (" )");return 0; }
  in_ptr = save_in_ptr;
  if (skip_constructor_send_message_upload_video_action (T) >= 0) { in_ptr = save_in_ptr; eprintf (" ("); assert (!fetch_constructor_send_message_upload_video_action (T)); if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); } eprintf (" )");return 0; }
  in_ptr = save_in_ptr;
  if (skip_constructor_send_message_record_audio_action (T) >= 0) { in_ptr = save_in_ptr; eprintf (" ("); assert (!fetch_constructor_send_message_record_audio_action (T)); if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); } eprintf (" )");return 0; }
  in_ptr = save_in_ptr;
  if (skip_constructor_send_message_upload_audio_action_l27 (T) >= 0) { in_ptr = save_in_ptr; eprintf (" ("); assert (!fetch_constructor_send_message_upload_audio_action_l27 (T)); if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); } eprintf (" )");return 0; }
  in_ptr = save_in_ptr;
  if (skip_constructor_send_message_upload_audio_action (T) >= 0) { in_ptr = save_in_ptr; eprintf (" ("); assert (!fetch_constructor_send_message_upload_audio_action (T)); if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); } eprintf (" )");return 0; }
  in_ptr = save_in_ptr;
  if (skip_constructor_send_message_upload_photo_action (T) >= 0) { in_ptr = save_in_ptr; eprintf (" ("); assert (!fetch_constructor_send_message_upload_photo_action (T)); if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); } eprintf (" )");return 0; }
  in_ptr = save_in_ptr;
  if (skip_constructor_send_message_upload_document_action_l27 (T) >= 0) { in_ptr = save_in_ptr; eprintf (" ("); assert (!fetch_constructor_send_message_upload_document_action_l27 (T)); if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); } eprintf (" )");return 0; }
  in_ptr = save_in_ptr;
  if (skip_constructor_send_message_upload_document_action (T) >= 0) { in_ptr = save_in_ptr; eprintf (" ("); assert (!fetch_constructor_send_message_upload_document_action (T)); if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); } eprintf (" )");return 0; }
  in_ptr = save_in_ptr;
  if (skip_constructor_send_message_geo_location_action (T) >= 0) { in_ptr = save_in_ptr; eprintf (" ("); assert (!fetch_constructor_send_message_geo_location_action (T)); if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); } eprintf (" )");return 0; }
  in_ptr = save_in_ptr;
  if (skip_constructor_send_message_choose_contact_action (T) >= 0) { in_ptr = save_in_ptr; eprintf (" ("); assert (!fetch_constructor_send_message_choose_contact_action (T)); if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); } eprintf (" )");return 0; }
  in_ptr = save_in_ptr;
  return -1;
}
int fetch_type_server_d_h_params (struct paramed_type *T) {
  if (in_remaining () < 4) { return -1;}
  if (multiline_output >= 2) { multiline_offset += multiline_offset_size; }
  eprintf (" (");
  int magic = fetch_int ();
  int res = -1;
  switch (magic) {
  case 0x79cb045d: res = fetch_constructor_server_d_h_params_fail (T); break;
  case 0xd0e8075c: res = fetch_constructor_server_d_h_params_ok (T); break;
  default: return -1;
  }
  if (res >= 0) {
    if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); }
    eprintf (" )");
  }
  return res;
}
int fetch_type_bare_server_d_h_params (struct paramed_type *T) {
  int *save_in_ptr = in_ptr;
  if (multiline_output >= 2) { multiline_offset += multiline_offset_size; }
  if (skip_constructor_server_d_h_params_fail (T) >= 0) { in_ptr = save_in_ptr; eprintf (" ("); assert (!fetch_constructor_server_d_h_params_fail (T)); if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); } eprintf (" )");return 0; }
  in_ptr = save_in_ptr;
  if (skip_constructor_server_d_h_params_ok (T) >= 0) { in_ptr = save_in_ptr; eprintf (" ("); assert (!fetch_constructor_server_d_h_params_ok (T)); if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); } eprintf (" )");return 0; }
  in_ptr = save_in_ptr;
  return -1;
}
int fetch_type_server_d_h_inner_data (struct paramed_type *T) {
  if (in_remaining () < 4) { return -1;}
  if (multiline_output >= 2) { multiline_offset += multiline_offset_size; }
  eprintf (" (");
  int magic = fetch_int ();
  int res = -1;
  switch (magic) {
  case 0xb5890dba: res = fetch_constructor_server_d_h_inner_data (T); break;
  default: return -1;
  }
  if (res >= 0) {
    if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); }
    eprintf (" )");
  }
  return res;
}
int fetch_type_bare_server_d_h_inner_data (struct paramed_type *T) {
  if (multiline_output >= 2) { multiline_offset += multiline_offset_size; }
  eprintf (" (");
  if (fetch_constructor_server_d_h_inner_data (T) >= 0) { if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); } eprintf (" )");return 0; }
  return -1;
}
int fetch_type_set_client_d_h_params_answer (struct paramed_type *T) {
  if (in_remaining () < 4) { return -1;}
  if (multiline_output >= 2) { multiline_offset += multiline_offset_size; }
  eprintf (" (");
  int magic = fetch_int ();
  int res = -1;
  switch (magic) {
  case 0x3bcbf734: res = fetch_constructor_dh_gen_ok (T); break;
  case 0x46dc1fb9: res = fetch_constructor_dh_gen_retry (T); break;
  case 0xa69dae02: res = fetch_constructor_dh_gen_fail (T); break;
  default: return -1;
  }
  if (res >= 0) {
    if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); }
    eprintf (" )");
  }
  return res;
}
int fetch_type_bare_set_client_d_h_params_answer (struct paramed_type *T) {
  int *save_in_ptr = in_ptr;
  if (multiline_output >= 2) { multiline_offset += multiline_offset_size; }
  if (skip_constructor_dh_gen_ok (T) >= 0) { in_ptr = save_in_ptr; eprintf (" ("); assert (!fetch_constructor_dh_gen_ok (T)); if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); } eprintf (" )");return 0; }
  in_ptr = save_in_ptr;
  if (skip_constructor_dh_gen_retry (T) >= 0) { in_ptr = save_in_ptr; eprintf (" ("); assert (!fetch_constructor_dh_gen_retry (T)); if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); } eprintf (" )");return 0; }
  in_ptr = save_in_ptr;
  if (skip_constructor_dh_gen_fail (T) >= 0) { in_ptr = save_in_ptr; eprintf (" ("); assert (!fetch_constructor_dh_gen_fail (T)); if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); } eprintf (" )");return 0; }
  in_ptr = save_in_ptr;
  return -1;
}
int fetch_type_sticker_pack (struct paramed_type *T) {
  if (in_remaining () < 4) { return -1;}
  if (multiline_output >= 2) { multiline_offset += multiline_offset_size; }
  eprintf (" (");
  int magic = fetch_int ();
  int res = -1;
  switch (magic) {
  case 0x12b299d4: res = fetch_constructor_sticker_pack (T); break;
  default: return -1;
  }
  if (res >= 0) {
    if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); }
    eprintf (" )");
  }
  return res;
}
int fetch_type_bare_sticker_pack (struct paramed_type *T) {
  if (multiline_output >= 2) { multiline_offset += multiline_offset_size; }
  eprintf (" (");
  if (fetch_constructor_sticker_pack (T) >= 0) { if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); } eprintf (" )");return 0; }
  return -1;
}
int fetch_type_sticker_set (struct paramed_type *T) {
  if (in_remaining () < 4) { return -1;}
  if (multiline_output >= 2) { multiline_offset += multiline_offset_size; }
  eprintf (" (");
  int magic = fetch_int ();
  int res = -1;
  switch (magic) {
  case 0xa7a43b17: res = fetch_constructor_sticker_set (T); break;
  default: return -1;
  }
  if (res >= 0) {
    if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); }
    eprintf (" )");
  }
  return res;
}
int fetch_type_bare_sticker_set (struct paramed_type *T) {
  if (multiline_output >= 2) { multiline_offset += multiline_offset_size; }
  eprintf (" (");
  if (fetch_constructor_sticker_set (T) >= 0) { if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); } eprintf (" )");return 0; }
  return -1;
}
int fetch_type_string (struct paramed_type *T) {
  if (in_remaining () < 4) { return -1;}
  int magic = fetch_int ();
  int res = -1;
  switch (magic) {
  case 0xb5286e24: res = fetch_constructor_string (T); break;
  default: return -1;
  }
  return res;
}
int fetch_type_bare_string (struct paramed_type *T) {
  if (fetch_constructor_string (T) >= 0) { return 0; }
  return -1;
}
int fetch_type_update (struct paramed_type *T) {
  if (in_remaining () < 4) { return -1;}
  if (multiline_output >= 2) { multiline_offset += multiline_offset_size; }
  eprintf (" (");
  int magic = fetch_int ();
  int res = -1;
  switch (magic) {
  case 0x1f2b0afd: res = fetch_constructor_update_new_message (T); break;
  case 0x4e90bfd6: res = fetch_constructor_update_message_i_d (T); break;
  case 0xa20db0e5: res = fetch_constructor_update_delete_messages (T); break;
  case 0x5c486927: res = fetch_constructor_update_user_typing (T); break;
  case 0x9a65ea1f: res = fetch_constructor_update_chat_user_typing (T); break;
  case 0x07761198: res = fetch_constructor_update_chat_participants (T); break;
  case 0x1bfbd823: res = fetch_constructor_update_user_status (T); break;
  case 0xa7332b73: res = fetch_constructor_update_user_name (T); break;
  case 0x95313b0c: res = fetch_constructor_update_user_photo (T); break;
  case 0x2575bbb9: res = fetch_constructor_update_contact_registered (T); break;
  case 0x9d2e67c5: res = fetch_constructor_update_contact_link (T); break;
  case 0x8f06529a: res = fetch_constructor_update_new_authorization (T); break;
  case 0x5a68e3f7: res = fetch_constructor_update_new_geo_chat_message (T); break;
  case 0x12bcbd9a: res = fetch_constructor_update_new_encrypted_message (T); break;
  case 0x1710f156: res = fetch_constructor_update_encrypted_chat_typing (T); break;
  case 0xb4a2e88d: res = fetch_constructor_update_encryption (T); break;
  case 0x38fe25b7: res = fetch_constructor_update_encrypted_messages_read (T); break;
  case 0x3a0eeb22: res = fetch_constructor_update_chat_participant_add (T); break;
  case 0x6e5f8c22: res = fetch_constructor_update_chat_participant_delete (T); break;
  case 0x8e5e9873: res = fetch_constructor_update_dc_options (T); break;
  case 0x80ece81a: res = fetch_constructor_update_user_blocked (T); break;
  case 0xbec268ef: res = fetch_constructor_update_notify_settings (T); break;
  case 0x382dd3e4: res = fetch_constructor_update_service_notification (T); break;
  case 0xee3b272a: res = fetch_constructor_update_privacy (T); break;
  case 0x12b9417b: res = fetch_constructor_update_user_phone (T); break;
  case 0x9961fd5c: res = fetch_constructor_update_read_history_inbox (T); break;
  case 0x2f2f21bf: res = fetch_constructor_update_read_history_outbox (T); break;
  case 0x2cc36971: res = fetch_constructor_update_web_page (T); break;
  case 0x68c13933: res = fetch_constructor_update_read_messages_contents (T); break;
  case 0x03114739: res = fetch_constructor_update_msg_update (T); break;
  default: return -1;
  }
  if (res >= 0) {
    if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); }
    eprintf (" )");
  }
  return res;
}
int fetch_type_bare_update (struct paramed_type *T) {
  int *save_in_ptr = in_ptr;
  if (multiline_output >= 2) { multiline_offset += multiline_offset_size; }
  if (skip_constructor_update_new_message (T) >= 0) { in_ptr = save_in_ptr; eprintf (" ("); assert (!fetch_constructor_update_new_message (T)); if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); } eprintf (" )");return 0; }
  in_ptr = save_in_ptr;
  if (skip_constructor_update_message_i_d (T) >= 0) { in_ptr = save_in_ptr; eprintf (" ("); assert (!fetch_constructor_update_message_i_d (T)); if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); } eprintf (" )");return 0; }
  in_ptr = save_in_ptr;
  if (skip_constructor_update_delete_messages (T) >= 0) { in_ptr = save_in_ptr; eprintf (" ("); assert (!fetch_constructor_update_delete_messages (T)); if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); } eprintf (" )");return 0; }
  in_ptr = save_in_ptr;
  if (skip_constructor_update_user_typing (T) >= 0) { in_ptr = save_in_ptr; eprintf (" ("); assert (!fetch_constructor_update_user_typing (T)); if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); } eprintf (" )");return 0; }
  in_ptr = save_in_ptr;
  if (skip_constructor_update_chat_user_typing (T) >= 0) { in_ptr = save_in_ptr; eprintf (" ("); assert (!fetch_constructor_update_chat_user_typing (T)); if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); } eprintf (" )");return 0; }
  in_ptr = save_in_ptr;
  if (skip_constructor_update_chat_participants (T) >= 0) { in_ptr = save_in_ptr; eprintf (" ("); assert (!fetch_constructor_update_chat_participants (T)); if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); } eprintf (" )");return 0; }
  in_ptr = save_in_ptr;
  if (skip_constructor_update_user_status (T) >= 0) { in_ptr = save_in_ptr; eprintf (" ("); assert (!fetch_constructor_update_user_status (T)); if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); } eprintf (" )");return 0; }
  in_ptr = save_in_ptr;
  if (skip_constructor_update_user_name (T) >= 0) { in_ptr = save_in_ptr; eprintf (" ("); assert (!fetch_constructor_update_user_name (T)); if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); } eprintf (" )");return 0; }
  in_ptr = save_in_ptr;
  if (skip_constructor_update_user_photo (T) >= 0) { in_ptr = save_in_ptr; eprintf (" ("); assert (!fetch_constructor_update_user_photo (T)); if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); } eprintf (" )");return 0; }
  in_ptr = save_in_ptr;
  if (skip_constructor_update_contact_registered (T) >= 0) { in_ptr = save_in_ptr; eprintf (" ("); assert (!fetch_constructor_update_contact_registered (T)); if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); } eprintf (" )");return 0; }
  in_ptr = save_in_ptr;
  if (skip_constructor_update_contact_link (T) >= 0) { in_ptr = save_in_ptr; eprintf (" ("); assert (!fetch_constructor_update_contact_link (T)); if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); } eprintf (" )");return 0; }
  in_ptr = save_in_ptr;
  if (skip_constructor_update_new_authorization (T) >= 0) { in_ptr = save_in_ptr; eprintf (" ("); assert (!fetch_constructor_update_new_authorization (T)); if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); } eprintf (" )");return 0; }
  in_ptr = save_in_ptr;
  if (skip_constructor_update_new_geo_chat_message (T) >= 0) { in_ptr = save_in_ptr; eprintf (" ("); assert (!fetch_constructor_update_new_geo_chat_message (T)); if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); } eprintf (" )");return 0; }
  in_ptr = save_in_ptr;
  if (skip_constructor_update_new_encrypted_message (T) >= 0) { in_ptr = save_in_ptr; eprintf (" ("); assert (!fetch_constructor_update_new_encrypted_message (T)); if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); } eprintf (" )");return 0; }
  in_ptr = save_in_ptr;
  if (skip_constructor_update_encrypted_chat_typing (T) >= 0) { in_ptr = save_in_ptr; eprintf (" ("); assert (!fetch_constructor_update_encrypted_chat_typing (T)); if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); } eprintf (" )");return 0; }
  in_ptr = save_in_ptr;
  if (skip_constructor_update_encryption (T) >= 0) { in_ptr = save_in_ptr; eprintf (" ("); assert (!fetch_constructor_update_encryption (T)); if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); } eprintf (" )");return 0; }
  in_ptr = save_in_ptr;
  if (skip_constructor_update_encrypted_messages_read (T) >= 0) { in_ptr = save_in_ptr; eprintf (" ("); assert (!fetch_constructor_update_encrypted_messages_read (T)); if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); } eprintf (" )");return 0; }
  in_ptr = save_in_ptr;
  if (skip_constructor_update_chat_participant_add (T) >= 0) { in_ptr = save_in_ptr; eprintf (" ("); assert (!fetch_constructor_update_chat_participant_add (T)); if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); } eprintf (" )");return 0; }
  in_ptr = save_in_ptr;
  if (skip_constructor_update_chat_participant_delete (T) >= 0) { in_ptr = save_in_ptr; eprintf (" ("); assert (!fetch_constructor_update_chat_participant_delete (T)); if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); } eprintf (" )");return 0; }
  in_ptr = save_in_ptr;
  if (skip_constructor_update_dc_options (T) >= 0) { in_ptr = save_in_ptr; eprintf (" ("); assert (!fetch_constructor_update_dc_options (T)); if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); } eprintf (" )");return 0; }
  in_ptr = save_in_ptr;
  if (skip_constructor_update_user_blocked (T) >= 0) { in_ptr = save_in_ptr; eprintf (" ("); assert (!fetch_constructor_update_user_blocked (T)); if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); } eprintf (" )");return 0; }
  in_ptr = save_in_ptr;
  if (skip_constructor_update_notify_settings (T) >= 0) { in_ptr = save_in_ptr; eprintf (" ("); assert (!fetch_constructor_update_notify_settings (T)); if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); } eprintf (" )");return 0; }
  in_ptr = save_in_ptr;
  if (skip_constructor_update_service_notification (T) >= 0) { in_ptr = save_in_ptr; eprintf (" ("); assert (!fetch_constructor_update_service_notification (T)); if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); } eprintf (" )");return 0; }
  in_ptr = save_in_ptr;
  if (skip_constructor_update_privacy (T) >= 0) { in_ptr = save_in_ptr; eprintf (" ("); assert (!fetch_constructor_update_privacy (T)); if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); } eprintf (" )");return 0; }
  in_ptr = save_in_ptr;
  if (skip_constructor_update_user_phone (T) >= 0) { in_ptr = save_in_ptr; eprintf (" ("); assert (!fetch_constructor_update_user_phone (T)); if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); } eprintf (" )");return 0; }
  in_ptr = save_in_ptr;
  if (skip_constructor_update_read_history_inbox (T) >= 0) { in_ptr = save_in_ptr; eprintf (" ("); assert (!fetch_constructor_update_read_history_inbox (T)); if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); } eprintf (" )");return 0; }
  in_ptr = save_in_ptr;
  if (skip_constructor_update_read_history_outbox (T) >= 0) { in_ptr = save_in_ptr; eprintf (" ("); assert (!fetch_constructor_update_read_history_outbox (T)); if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); } eprintf (" )");return 0; }
  in_ptr = save_in_ptr;
  if (skip_constructor_update_web_page (T) >= 0) { in_ptr = save_in_ptr; eprintf (" ("); assert (!fetch_constructor_update_web_page (T)); if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); } eprintf (" )");return 0; }
  in_ptr = save_in_ptr;
  if (skip_constructor_update_read_messages_contents (T) >= 0) { in_ptr = save_in_ptr; eprintf (" ("); assert (!fetch_constructor_update_read_messages_contents (T)); if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); } eprintf (" )");return 0; }
  in_ptr = save_in_ptr;
  if (skip_constructor_update_msg_update (T) >= 0) { in_ptr = save_in_ptr; eprintf (" ("); assert (!fetch_constructor_update_msg_update (T)); if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); } eprintf (" )");return 0; }
  in_ptr = save_in_ptr;
  return -1;
}
int fetch_type_updates (struct paramed_type *T) {
  if (in_remaining () < 4) { return -1;}
  if (multiline_output >= 2) { multiline_offset += multiline_offset_size; }
  eprintf (" (");
  int magic = fetch_int ();
  int res = -1;
  switch (magic) {
  case 0xe317af7e: res = fetch_constructor_updates_too_long (T); break;
  case 0xed5c2127: res = fetch_constructor_update_short_message (T); break;
  case 0x52238b3c: res = fetch_constructor_update_short_chat_message (T); break;
  case 0x78d4dec1: res = fetch_constructor_update_short (T); break;
  case 0x725b04c3: res = fetch_constructor_updates_combined (T); break;
  case 0x74ae4240: res = fetch_constructor_updates (T); break;
  default: return -1;
  }
  if (res >= 0) {
    if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); }
    eprintf (" )");
  }
  return res;
}
int fetch_type_bare_updates (struct paramed_type *T) {
  int *save_in_ptr = in_ptr;
  if (multiline_output >= 2) { multiline_offset += multiline_offset_size; }
  if (skip_constructor_updates_too_long (T) >= 0) { in_ptr = save_in_ptr; eprintf (" ("); assert (!fetch_constructor_updates_too_long (T)); if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); } eprintf (" )");return 0; }
  in_ptr = save_in_ptr;
  if (skip_constructor_update_short_message (T) >= 0) { in_ptr = save_in_ptr; eprintf (" ("); assert (!fetch_constructor_update_short_message (T)); if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); } eprintf (" )");return 0; }
  in_ptr = save_in_ptr;
  if (skip_constructor_update_short_chat_message (T) >= 0) { in_ptr = save_in_ptr; eprintf (" ("); assert (!fetch_constructor_update_short_chat_message (T)); if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); } eprintf (" )");return 0; }
  in_ptr = save_in_ptr;
  if (skip_constructor_update_short (T) >= 0) { in_ptr = save_in_ptr; eprintf (" ("); assert (!fetch_constructor_update_short (T)); if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); } eprintf (" )");return 0; }
  in_ptr = save_in_ptr;
  if (skip_constructor_updates_combined (T) >= 0) { in_ptr = save_in_ptr; eprintf (" ("); assert (!fetch_constructor_updates_combined (T)); if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); } eprintf (" )");return 0; }
  in_ptr = save_in_ptr;
  if (skip_constructor_updates (T) >= 0) { in_ptr = save_in_ptr; eprintf (" ("); assert (!fetch_constructor_updates (T)); if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); } eprintf (" )");return 0; }
  in_ptr = save_in_ptr;
  return -1;
}
int fetch_type_user (struct paramed_type *T) {
  if (in_remaining () < 4) { return -1;}
  if (multiline_output >= 2) { multiline_offset += multiline_offset_size; }
  eprintf (" (");
  int magic = fetch_int ();
  int res = -1;
  switch (magic) {
  case 0x200250ba: res = fetch_constructor_user_empty (T); break;
  case 0x22e49072: res = fetch_constructor_user (T); break;
  default: return -1;
  }
  if (res >= 0) {
    if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); }
    eprintf (" )");
  }
  return res;
}
int fetch_type_bare_user (struct paramed_type *T) {
  int *save_in_ptr = in_ptr;
  if (multiline_output >= 2) { multiline_offset += multiline_offset_size; }
  if (skip_constructor_user_empty (T) >= 0) { in_ptr = save_in_ptr; eprintf (" ("); assert (!fetch_constructor_user_empty (T)); if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); } eprintf (" )");return 0; }
  in_ptr = save_in_ptr;
  if (skip_constructor_user (T) >= 0) { in_ptr = save_in_ptr; eprintf (" ("); assert (!fetch_constructor_user (T)); if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); } eprintf (" )");return 0; }
  in_ptr = save_in_ptr;
  return -1;
}
int fetch_type_user_full (struct paramed_type *T) {
  if (in_remaining () < 4) { return -1;}
  if (multiline_output >= 2) { multiline_offset += multiline_offset_size; }
  eprintf (" (");
  int magic = fetch_int ();
  int res = -1;
  switch (magic) {
  case 0x5a89ac5b: res = fetch_constructor_user_full (T); break;
  default: return -1;
  }
  if (res >= 0) {
    if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); }
    eprintf (" )");
  }
  return res;
}
int fetch_type_bare_user_full (struct paramed_type *T) {
  if (multiline_output >= 2) { multiline_offset += multiline_offset_size; }
  eprintf (" (");
  if (fetch_constructor_user_full (T) >= 0) { if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); } eprintf (" )");return 0; }
  return -1;
}
int fetch_type_user_profile_photo (struct paramed_type *T) {
  if (in_remaining () < 4) { return -1;}
  if (multiline_output >= 2) { multiline_offset += multiline_offset_size; }
  eprintf (" (");
  int magic = fetch_int ();
  int res = -1;
  switch (magic) {
  case 0x4f11bae1: res = fetch_constructor_user_profile_photo_empty (T); break;
  case 0xd559d8c8: res = fetch_constructor_user_profile_photo (T); break;
  default: return -1;
  }
  if (res >= 0) {
    if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); }
    eprintf (" )");
  }
  return res;
}
int fetch_type_bare_user_profile_photo (struct paramed_type *T) {
  int *save_in_ptr = in_ptr;
  if (multiline_output >= 2) { multiline_offset += multiline_offset_size; }
  if (skip_constructor_user_profile_photo_empty (T) >= 0) { in_ptr = save_in_ptr; eprintf (" ("); assert (!fetch_constructor_user_profile_photo_empty (T)); if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); } eprintf (" )");return 0; }
  in_ptr = save_in_ptr;
  if (skip_constructor_user_profile_photo (T) >= 0) { in_ptr = save_in_ptr; eprintf (" ("); assert (!fetch_constructor_user_profile_photo (T)); if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); } eprintf (" )");return 0; }
  in_ptr = save_in_ptr;
  return -1;
}
int fetch_type_user_status (struct paramed_type *T) {
  if (in_remaining () < 4) { return -1;}
  if (multiline_output >= 2) { multiline_offset += multiline_offset_size; }
  eprintf (" (");
  int magic = fetch_int ();
  int res = -1;
  switch (magic) {
  case 0x09d05049: res = fetch_constructor_user_status_empty (T); break;
  case 0xedb93949: res = fetch_constructor_user_status_online (T); break;
  case 0x008c703f: res = fetch_constructor_user_status_offline (T); break;
  case 0xe26f42f1: res = fetch_constructor_user_status_recently (T); break;
  case 0x07bf09fc: res = fetch_constructor_user_status_last_week (T); break;
  case 0x77ebc742: res = fetch_constructor_user_status_last_month (T); break;
  default: return -1;
  }
  if (res >= 0) {
    if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); }
    eprintf (" )");
  }
  return res;
}
int fetch_type_bare_user_status (struct paramed_type *T) {
  int *save_in_ptr = in_ptr;
  if (multiline_output >= 2) { multiline_offset += multiline_offset_size; }
  if (skip_constructor_user_status_empty (T) >= 0) { in_ptr = save_in_ptr; eprintf (" ("); assert (!fetch_constructor_user_status_empty (T)); if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); } eprintf (" )");return 0; }
  in_ptr = save_in_ptr;
  if (skip_constructor_user_status_online (T) >= 0) { in_ptr = save_in_ptr; eprintf (" ("); assert (!fetch_constructor_user_status_online (T)); if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); } eprintf (" )");return 0; }
  in_ptr = save_in_ptr;
  if (skip_constructor_user_status_offline (T) >= 0) { in_ptr = save_in_ptr; eprintf (" ("); assert (!fetch_constructor_user_status_offline (T)); if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); } eprintf (" )");return 0; }
  in_ptr = save_in_ptr;
  if (skip_constructor_user_status_recently (T) >= 0) { in_ptr = save_in_ptr; eprintf (" ("); assert (!fetch_constructor_user_status_recently (T)); if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); } eprintf (" )");return 0; }
  in_ptr = save_in_ptr;
  if (skip_constructor_user_status_last_week (T) >= 0) { in_ptr = save_in_ptr; eprintf (" ("); assert (!fetch_constructor_user_status_last_week (T)); if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); } eprintf (" )");return 0; }
  in_ptr = save_in_ptr;
  if (skip_constructor_user_status_last_month (T) >= 0) { in_ptr = save_in_ptr; eprintf (" ("); assert (!fetch_constructor_user_status_last_month (T)); if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); } eprintf (" )");return 0; }
  in_ptr = save_in_ptr;
  return -1;
}
int fetch_type_vector (struct paramed_type *T) {
  if (in_remaining () < 4) { return -1;}
  if (multiline_output >= 2) { multiline_offset += multiline_offset_size; }
  eprintf (" (");
  int magic = fetch_int ();
  int res = -1;
  switch (magic) {
  case 0x1cb5c415: res = fetch_constructor_vector (T); break;
  default: return -1;
  }
  if (res >= 0) {
    if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); }
    eprintf (" )");
  }
  return res;
}
int fetch_type_bare_vector (struct paramed_type *T) {
  if (multiline_output >= 2) { multiline_offset += multiline_offset_size; }
  eprintf (" (");
  if (fetch_constructor_vector (T) >= 0) { if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); } eprintf (" )");return 0; }
  return -1;
}
int fetch_type_video (struct paramed_type *T) {
  if (in_remaining () < 4) { return -1;}
  if (multiline_output >= 2) { multiline_offset += multiline_offset_size; }
  eprintf (" (");
  int magic = fetch_int ();
  int res = -1;
  switch (magic) {
  case 0xc10658a8: res = fetch_constructor_video_empty (T); break;
  case 0xee9f4a4d: res = fetch_constructor_video (T); break;
  case 0x388fa391: res = fetch_constructor_video_l27 (T); break;
  default: return -1;
  }
  if (res >= 0) {
    if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); }
    eprintf (" )");
  }
  return res;
}
int fetch_type_bare_video (struct paramed_type *T) {
  int *save_in_ptr = in_ptr;
  if (multiline_output >= 2) { multiline_offset += multiline_offset_size; }
  if (skip_constructor_video_empty (T) >= 0) { in_ptr = save_in_ptr; eprintf (" ("); assert (!fetch_constructor_video_empty (T)); if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); } eprintf (" )");return 0; }
  in_ptr = save_in_ptr;
  if (skip_constructor_video (T) >= 0) { in_ptr = save_in_ptr; eprintf (" ("); assert (!fetch_constructor_video (T)); if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); } eprintf (" )");return 0; }
  in_ptr = save_in_ptr;
  if (skip_constructor_video_l27 (T) >= 0) { in_ptr = save_in_ptr; eprintf (" ("); assert (!fetch_constructor_video_l27 (T)); if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); } eprintf (" )");return 0; }
  in_ptr = save_in_ptr;
  return -1;
}
int fetch_type_wall_paper (struct paramed_type *T) {
  if (in_remaining () < 4) { return -1;}
  if (multiline_output >= 2) { multiline_offset += multiline_offset_size; }
  eprintf (" (");
  int magic = fetch_int ();
  int res = -1;
  switch (magic) {
  case 0xccb03657: res = fetch_constructor_wall_paper (T); break;
  case 0x63117f24: res = fetch_constructor_wall_paper_solid (T); break;
  default: return -1;
  }
  if (res >= 0) {
    if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); }
    eprintf (" )");
  }
  return res;
}
int fetch_type_bare_wall_paper (struct paramed_type *T) {
  int *save_in_ptr = in_ptr;
  if (multiline_output >= 2) { multiline_offset += multiline_offset_size; }
  if (skip_constructor_wall_paper (T) >= 0) { in_ptr = save_in_ptr; eprintf (" ("); assert (!fetch_constructor_wall_paper (T)); if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); } eprintf (" )");return 0; }
  in_ptr = save_in_ptr;
  if (skip_constructor_wall_paper_solid (T) >= 0) { in_ptr = save_in_ptr; eprintf (" ("); assert (!fetch_constructor_wall_paper_solid (T)); if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); } eprintf (" )");return 0; }
  in_ptr = save_in_ptr;
  return -1;
}
int fetch_type_web_page (struct paramed_type *T) {
  if (in_remaining () < 4) { return -1;}
  if (multiline_output >= 2) { multiline_offset += multiline_offset_size; }
  eprintf (" (");
  int magic = fetch_int ();
  int res = -1;
  switch (magic) {
  case 0xeb1477e8: res = fetch_constructor_web_page_empty (T); break;
  case 0xc586da1c: res = fetch_constructor_web_page_pending (T); break;
  case 0xa31ea0b5: res = fetch_constructor_web_page (T); break;
  default: return -1;
  }
  if (res >= 0) {
    if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); }
    eprintf (" )");
  }
  return res;
}
int fetch_type_bare_web_page (struct paramed_type *T) {
  int *save_in_ptr = in_ptr;
  if (multiline_output >= 2) { multiline_offset += multiline_offset_size; }
  if (skip_constructor_web_page_empty (T) >= 0) { in_ptr = save_in_ptr; eprintf (" ("); assert (!fetch_constructor_web_page_empty (T)); if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); } eprintf (" )");return 0; }
  in_ptr = save_in_ptr;
  if (skip_constructor_web_page_pending (T) >= 0) { in_ptr = save_in_ptr; eprintf (" ("); assert (!fetch_constructor_web_page_pending (T)); if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); } eprintf (" )");return 0; }
  in_ptr = save_in_ptr;
  if (skip_constructor_web_page (T) >= 0) { in_ptr = save_in_ptr; eprintf (" ("); assert (!fetch_constructor_web_page (T)); if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); } eprintf (" )");return 0; }
  in_ptr = save_in_ptr;
  return -1;
}
int fetch_type_account_authorizations (struct paramed_type *T) {
  if (in_remaining () < 4) { return -1;}
  int magic = fetch_int ();
  int res = -1;
  switch (magic) {
  case 0x1250abde: res = fetch_constructor_account_authorizations (T); break;
  default: return -1;
  }
  return res;
}
int fetch_type_bare_account_authorizations (struct paramed_type *T) {
  if (fetch_constructor_account_authorizations (T) >= 0) { return 0; }
  return -1;
}
int fetch_type_account_password (struct paramed_type *T) {
  if (in_remaining () < 4) { return -1;}
  if (multiline_output >= 2) { multiline_offset += multiline_offset_size; }
  eprintf (" (");
  int magic = fetch_int ();
  int res = -1;
  switch (magic) {
  case 0x96dabc18: res = fetch_constructor_account_no_password (T); break;
  case 0x7c18141c: res = fetch_constructor_account_password (T); break;
  default: return -1;
  }
  if (res >= 0) {
    if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); }
    eprintf (" )");
  }
  return res;
}
int fetch_type_bare_account_password (struct paramed_type *T) {
  int *save_in_ptr = in_ptr;
  if (multiline_output >= 2) { multiline_offset += multiline_offset_size; }
  if (skip_constructor_account_no_password (T) >= 0) { in_ptr = save_in_ptr; eprintf (" ("); assert (!fetch_constructor_account_no_password (T)); if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); } eprintf (" )");return 0; }
  in_ptr = save_in_ptr;
  if (skip_constructor_account_password (T) >= 0) { in_ptr = save_in_ptr; eprintf (" ("); assert (!fetch_constructor_account_password (T)); if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); } eprintf (" )");return 0; }
  in_ptr = save_in_ptr;
  return -1;
}
int fetch_type_account_password_input_settings (struct paramed_type *T) {
  if (in_remaining () < 4) { return -1;}
  if (multiline_output >= 2) { multiline_offset += multiline_offset_size; }
  eprintf (" (");
  int magic = fetch_int ();
  int res = -1;
  switch (magic) {
  case 0xbcfc532c: res = fetch_constructor_account_password_input_settings (T); break;
  default: return -1;
  }
  if (res >= 0) {
    if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); }
    eprintf (" )");
  }
  return res;
}
int fetch_type_bare_account_password_input_settings (struct paramed_type *T) {
  if (multiline_output >= 2) { multiline_offset += multiline_offset_size; }
  eprintf (" (");
  if (fetch_constructor_account_password_input_settings (T) >= 0) { if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); } eprintf (" )");return 0; }
  return -1;
}
int fetch_type_account_password_settings (struct paramed_type *T) {
  if (in_remaining () < 4) { return -1;}
  int magic = fetch_int ();
  int res = -1;
  switch (magic) {
  case 0xb7b72ab3: res = fetch_constructor_account_password_settings (T); break;
  default: return -1;
  }
  return res;
}
int fetch_type_bare_account_password_settings (struct paramed_type *T) {
  if (fetch_constructor_account_password_settings (T) >= 0) { return 0; }
  return -1;
}
int fetch_type_account_privacy_rules (struct paramed_type *T) {
  if (in_remaining () < 4) { return -1;}
  if (multiline_output >= 2) { multiline_offset += multiline_offset_size; }
  eprintf (" (");
  int magic = fetch_int ();
  int res = -1;
  switch (magic) {
  case 0x554abb6f: res = fetch_constructor_account_privacy_rules (T); break;
  default: return -1;
  }
  if (res >= 0) {
    if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); }
    eprintf (" )");
  }
  return res;
}
int fetch_type_bare_account_privacy_rules (struct paramed_type *T) {
  if (multiline_output >= 2) { multiline_offset += multiline_offset_size; }
  eprintf (" (");
  if (fetch_constructor_account_privacy_rules (T) >= 0) { if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); } eprintf (" )");return 0; }
  return -1;
}
int fetch_type_account_sent_change_phone_code (struct paramed_type *T) {
  if (in_remaining () < 4) { return -1;}
  if (multiline_output >= 2) { multiline_offset += multiline_offset_size; }
  eprintf (" (");
  int magic = fetch_int ();
  int res = -1;
  switch (magic) {
  case 0xa4f58c4c: res = fetch_constructor_account_sent_change_phone_code (T); break;
  default: return -1;
  }
  if (res >= 0) {
    if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); }
    eprintf (" )");
  }
  return res;
}
int fetch_type_bare_account_sent_change_phone_code (struct paramed_type *T) {
  if (multiline_output >= 2) { multiline_offset += multiline_offset_size; }
  eprintf (" (");
  if (fetch_constructor_account_sent_change_phone_code (T) >= 0) { if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); } eprintf (" )");return 0; }
  return -1;
}
int fetch_type_auth_authorization (struct paramed_type *T) {
  if (in_remaining () < 4) { return -1;}
  int magic = fetch_int ();
  int res = -1;
  switch (magic) {
  case 0xff036af1: res = fetch_constructor_auth_authorization (T); break;
  default: return -1;
  }
  return res;
}
int fetch_type_bare_auth_authorization (struct paramed_type *T) {
  if (fetch_constructor_auth_authorization (T) >= 0) { return 0; }
  return -1;
}
int fetch_type_auth_checked_phone (struct paramed_type *T) {
  if (in_remaining () < 4) { return -1;}
  int magic = fetch_int ();
  int res = -1;
  switch (magic) {
  case 0x811ea28e: res = fetch_constructor_auth_checked_phone (T); break;
  default: return -1;
  }
  return res;
}
int fetch_type_bare_auth_checked_phone (struct paramed_type *T) {
  if (fetch_constructor_auth_checked_phone (T) >= 0) { return 0; }
  return -1;
}
int fetch_type_auth_exported_authorization (struct paramed_type *T) {
  if (in_remaining () < 4) { return -1;}
  if (multiline_output >= 2) { multiline_offset += multiline_offset_size; }
  eprintf (" (");
  int magic = fetch_int ();
  int res = -1;
  switch (magic) {
  case 0xdf969c2d: res = fetch_constructor_auth_exported_authorization (T); break;
  default: return -1;
  }
  if (res >= 0) {
    if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); }
    eprintf (" )");
  }
  return res;
}
int fetch_type_bare_auth_exported_authorization (struct paramed_type *T) {
  if (multiline_output >= 2) { multiline_offset += multiline_offset_size; }
  eprintf (" (");
  if (fetch_constructor_auth_exported_authorization (T) >= 0) { if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); } eprintf (" )");return 0; }
  return -1;
}
int fetch_type_auth_password_recovery (struct paramed_type *T) {
  if (in_remaining () < 4) { return -1;}
  int magic = fetch_int ();
  int res = -1;
  switch (magic) {
  case 0x137948a5: res = fetch_constructor_auth_password_recovery (T); break;
  default: return -1;
  }
  return res;
}
int fetch_type_bare_auth_password_recovery (struct paramed_type *T) {
  if (fetch_constructor_auth_password_recovery (T) >= 0) { return 0; }
  return -1;
}
int fetch_type_auth_sent_code (struct paramed_type *T) {
  if (in_remaining () < 4) { return -1;}
  if (multiline_output >= 2) { multiline_offset += multiline_offset_size; }
  eprintf (" (");
  int magic = fetch_int ();
  int res = -1;
  switch (magic) {
  case 0xefed51d9: res = fetch_constructor_auth_sent_code (T); break;
  case 0xe325edcf: res = fetch_constructor_auth_sent_app_code (T); break;
  default: return -1;
  }
  if (res >= 0) {
    if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); }
    eprintf (" )");
  }
  return res;
}
int fetch_type_bare_auth_sent_code (struct paramed_type *T) {
  int *save_in_ptr = in_ptr;
  if (multiline_output >= 2) { multiline_offset += multiline_offset_size; }
  if (skip_constructor_auth_sent_code (T) >= 0) { in_ptr = save_in_ptr; eprintf (" ("); assert (!fetch_constructor_auth_sent_code (T)); if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); } eprintf (" )");return 0; }
  in_ptr = save_in_ptr;
  if (skip_constructor_auth_sent_app_code (T) >= 0) { in_ptr = save_in_ptr; eprintf (" ("); assert (!fetch_constructor_auth_sent_app_code (T)); if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); } eprintf (" )");return 0; }
  in_ptr = save_in_ptr;
  return -1;
}
int fetch_type_binlog_encr_key (struct paramed_type *T) {
  if (in_remaining () < 4) { return -1;}
  int magic = fetch_int ();
  int res = -1;
  switch (magic) {
  case 0x0377168f: res = fetch_constructor_binlog_encr_key (T); break;
  default: return -1;
  }
  return res;
}
int fetch_type_bare_binlog_encr_key (struct paramed_type *T) {
  if (fetch_constructor_binlog_encr_key (T) >= 0) { return 0; }
  return -1;
}
int fetch_type_binlog_update (struct paramed_type *T) {
  if (in_remaining () < 4) { return -1;}
  if (multiline_output >= 2) { multiline_offset += multiline_offset_size; }
  eprintf (" (");
  int magic = fetch_int ();
  int res = -1;
  switch (magic) {
  case 0x3b06de69: res = fetch_constructor_binlog_start (T); break;
  case 0xf96feb32: res = fetch_constructor_binlog_dc_option (T); break;
  case 0x7c0d22d8: res = fetch_constructor_binlog_dc_option_new (T); break;
  case 0x71e8c156: res = fetch_constructor_binlog_auth_key (T); break;
  case 0x9e83dbdc: res = fetch_constructor_binlog_default_dc (T); break;
  case 0x26451bb5: res = fetch_constructor_binlog_dc_signed (T); break;
  case 0x68a870e8: res = fetch_constructor_binlog_our_id (T); break;
  case 0xeaeb7826: res = fetch_constructor_binlog_set_dh_params (T); break;
  case 0x2ca8c939: res = fetch_constructor_binlog_set_pts (T); break;
  case 0xd95738ac: res = fetch_constructor_binlog_set_qts (T); break;
  case 0x1d0f4b52: res = fetch_constructor_binlog_set_date (T); break;
  case 0x6eeb2989: res = fetch_constructor_binlog_set_seq (T); break;
  case 0xee1b38e8: res = fetch_constructor_binlog_encr_chat_delete (T); break;
  case 0x84977251: res = fetch_constructor_binlog_encr_chat_new (T); break;
  case 0x9d49488d: res = fetch_constructor_binlog_encr_chat_exchange_new (T); break;
  case 0xac55d447: res = fetch_constructor_binlog_user_delete (T); break;
  case 0x127cf2f9: res = fetch_constructor_binlog_user_new (T); break;
  case 0x0a10aa92: res = fetch_constructor_binlog_chat_new (T); break;
  case 0x535475ea: res = fetch_constructor_binlog_chat_add_participant (T); break;
  case 0x7dd1a1a2: res = fetch_constructor_binlog_chat_del_participant (T); break;
  case 0x3c873416: res = fetch_constructor_binlog_set_msg_id (T); break;
  case 0x847e77b1: res = fetch_constructor_binlog_message_delete (T); break;
  case 0x427cfcdb: res = fetch_constructor_binlog_message_new (T); break;
  case 0x6cf7cabc: res = fetch_constructor_binlog_message_encr_new (T); break;
  case 0x6dd4d85f: res = fetch_constructor_binlog_msg_update (T); break;
  case 0x83327955: res = fetch_constructor_binlog_reset_authorization (T); break;
  default: return -1;
  }
  if (res >= 0) {
    if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); }
    eprintf (" )");
  }
  return res;
}
int fetch_type_bare_binlog_update (struct paramed_type *T) {
  int *save_in_ptr = in_ptr;
  if (multiline_output >= 2) { multiline_offset += multiline_offset_size; }
  if (skip_constructor_binlog_start (T) >= 0) { in_ptr = save_in_ptr; eprintf (" ("); assert (!fetch_constructor_binlog_start (T)); if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); } eprintf (" )");return 0; }
  in_ptr = save_in_ptr;
  if (skip_constructor_binlog_dc_option (T) >= 0) { in_ptr = save_in_ptr; eprintf (" ("); assert (!fetch_constructor_binlog_dc_option (T)); if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); } eprintf (" )");return 0; }
  in_ptr = save_in_ptr;
  if (skip_constructor_binlog_dc_option_new (T) >= 0) { in_ptr = save_in_ptr; eprintf (" ("); assert (!fetch_constructor_binlog_dc_option_new (T)); if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); } eprintf (" )");return 0; }
  in_ptr = save_in_ptr;
  if (skip_constructor_binlog_auth_key (T) >= 0) { in_ptr = save_in_ptr; eprintf (" ("); assert (!fetch_constructor_binlog_auth_key (T)); if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); } eprintf (" )");return 0; }
  in_ptr = save_in_ptr;
  if (skip_constructor_binlog_default_dc (T) >= 0) { in_ptr = save_in_ptr; eprintf (" ("); assert (!fetch_constructor_binlog_default_dc (T)); if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); } eprintf (" )");return 0; }
  in_ptr = save_in_ptr;
  if (skip_constructor_binlog_dc_signed (T) >= 0) { in_ptr = save_in_ptr; eprintf (" ("); assert (!fetch_constructor_binlog_dc_signed (T)); if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); } eprintf (" )");return 0; }
  in_ptr = save_in_ptr;
  if (skip_constructor_binlog_our_id (T) >= 0) { in_ptr = save_in_ptr; eprintf (" ("); assert (!fetch_constructor_binlog_our_id (T)); if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); } eprintf (" )");return 0; }
  in_ptr = save_in_ptr;
  if (skip_constructor_binlog_set_dh_params (T) >= 0) { in_ptr = save_in_ptr; eprintf (" ("); assert (!fetch_constructor_binlog_set_dh_params (T)); if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); } eprintf (" )");return 0; }
  in_ptr = save_in_ptr;
  if (skip_constructor_binlog_set_pts (T) >= 0) { in_ptr = save_in_ptr; eprintf (" ("); assert (!fetch_constructor_binlog_set_pts (T)); if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); } eprintf (" )");return 0; }
  in_ptr = save_in_ptr;
  if (skip_constructor_binlog_set_qts (T) >= 0) { in_ptr = save_in_ptr; eprintf (" ("); assert (!fetch_constructor_binlog_set_qts (T)); if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); } eprintf (" )");return 0; }
  in_ptr = save_in_ptr;
  if (skip_constructor_binlog_set_date (T) >= 0) { in_ptr = save_in_ptr; eprintf (" ("); assert (!fetch_constructor_binlog_set_date (T)); if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); } eprintf (" )");return 0; }
  in_ptr = save_in_ptr;
  if (skip_constructor_binlog_set_seq (T) >= 0) { in_ptr = save_in_ptr; eprintf (" ("); assert (!fetch_constructor_binlog_set_seq (T)); if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); } eprintf (" )");return 0; }
  in_ptr = save_in_ptr;
  if (skip_constructor_binlog_encr_chat_delete (T) >= 0) { in_ptr = save_in_ptr; eprintf (" ("); assert (!fetch_constructor_binlog_encr_chat_delete (T)); if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); } eprintf (" )");return 0; }
  in_ptr = save_in_ptr;
  if (skip_constructor_binlog_encr_chat_new (T) >= 0) { in_ptr = save_in_ptr; eprintf (" ("); assert (!fetch_constructor_binlog_encr_chat_new (T)); if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); } eprintf (" )");return 0; }
  in_ptr = save_in_ptr;
  if (skip_constructor_binlog_encr_chat_exchange_new (T) >= 0) { in_ptr = save_in_ptr; eprintf (" ("); assert (!fetch_constructor_binlog_encr_chat_exchange_new (T)); if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); } eprintf (" )");return 0; }
  in_ptr = save_in_ptr;
  if (skip_constructor_binlog_user_delete (T) >= 0) { in_ptr = save_in_ptr; eprintf (" ("); assert (!fetch_constructor_binlog_user_delete (T)); if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); } eprintf (" )");return 0; }
  in_ptr = save_in_ptr;
  if (skip_constructor_binlog_user_new (T) >= 0) { in_ptr = save_in_ptr; eprintf (" ("); assert (!fetch_constructor_binlog_user_new (T)); if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); } eprintf (" )");return 0; }
  in_ptr = save_in_ptr;
  if (skip_constructor_binlog_chat_new (T) >= 0) { in_ptr = save_in_ptr; eprintf (" ("); assert (!fetch_constructor_binlog_chat_new (T)); if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); } eprintf (" )");return 0; }
  in_ptr = save_in_ptr;
  if (skip_constructor_binlog_chat_add_participant (T) >= 0) { in_ptr = save_in_ptr; eprintf (" ("); assert (!fetch_constructor_binlog_chat_add_participant (T)); if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); } eprintf (" )");return 0; }
  in_ptr = save_in_ptr;
  if (skip_constructor_binlog_chat_del_participant (T) >= 0) { in_ptr = save_in_ptr; eprintf (" ("); assert (!fetch_constructor_binlog_chat_del_participant (T)); if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); } eprintf (" )");return 0; }
  in_ptr = save_in_ptr;
  if (skip_constructor_binlog_set_msg_id (T) >= 0) { in_ptr = save_in_ptr; eprintf (" ("); assert (!fetch_constructor_binlog_set_msg_id (T)); if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); } eprintf (" )");return 0; }
  in_ptr = save_in_ptr;
  if (skip_constructor_binlog_message_delete (T) >= 0) { in_ptr = save_in_ptr; eprintf (" ("); assert (!fetch_constructor_binlog_message_delete (T)); if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); } eprintf (" )");return 0; }
  in_ptr = save_in_ptr;
  if (skip_constructor_binlog_message_new (T) >= 0) { in_ptr = save_in_ptr; eprintf (" ("); assert (!fetch_constructor_binlog_message_new (T)); if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); } eprintf (" )");return 0; }
  in_ptr = save_in_ptr;
  if (skip_constructor_binlog_message_encr_new (T) >= 0) { in_ptr = save_in_ptr; eprintf (" ("); assert (!fetch_constructor_binlog_message_encr_new (T)); if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); } eprintf (" )");return 0; }
  in_ptr = save_in_ptr;
  if (skip_constructor_binlog_msg_update (T) >= 0) { in_ptr = save_in_ptr; eprintf (" ("); assert (!fetch_constructor_binlog_msg_update (T)); if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); } eprintf (" )");return 0; }
  in_ptr = save_in_ptr;
  if (skip_constructor_binlog_reset_authorization (T) >= 0) { in_ptr = save_in_ptr; eprintf (" ("); assert (!fetch_constructor_binlog_reset_authorization (T)); if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); } eprintf (" )");return 0; }
  in_ptr = save_in_ptr;
  return -1;
}
int fetch_type_contacts_blocked (struct paramed_type *T) {
  if (in_remaining () < 4) { return -1;}
  if (multiline_output >= 2) { multiline_offset += multiline_offset_size; }
  eprintf (" (");
  int magic = fetch_int ();
  int res = -1;
  switch (magic) {
  case 0x1c138d15: res = fetch_constructor_contacts_blocked (T); break;
  case 0x900802a1: res = fetch_constructor_contacts_blocked_slice (T); break;
  default: return -1;
  }
  if (res >= 0) {
    if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); }
    eprintf (" )");
  }
  return res;
}
int fetch_type_bare_contacts_blocked (struct paramed_type *T) {
  int *save_in_ptr = in_ptr;
  if (multiline_output >= 2) { multiline_offset += multiline_offset_size; }
  if (skip_constructor_contacts_blocked (T) >= 0) { in_ptr = save_in_ptr; eprintf (" ("); assert (!fetch_constructor_contacts_blocked (T)); if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); } eprintf (" )");return 0; }
  in_ptr = save_in_ptr;
  if (skip_constructor_contacts_blocked_slice (T) >= 0) { in_ptr = save_in_ptr; eprintf (" ("); assert (!fetch_constructor_contacts_blocked_slice (T)); if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); } eprintf (" )");return 0; }
  in_ptr = save_in_ptr;
  return -1;
}
int fetch_type_contacts_contacts (struct paramed_type *T) {
  if (in_remaining () < 4) { return -1;}
  if (multiline_output >= 2) { multiline_offset += multiline_offset_size; }
  eprintf (" (");
  int magic = fetch_int ();
  int res = -1;
  switch (magic) {
  case 0xb74ba9d2: res = fetch_constructor_contacts_contacts_not_modified (T); break;
  case 0x6f8b8cb2: res = fetch_constructor_contacts_contacts (T); break;
  default: return -1;
  }
  if (res >= 0) {
    if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); }
    eprintf (" )");
  }
  return res;
}
int fetch_type_bare_contacts_contacts (struct paramed_type *T) {
  int *save_in_ptr = in_ptr;
  if (multiline_output >= 2) { multiline_offset += multiline_offset_size; }
  if (skip_constructor_contacts_contacts_not_modified (T) >= 0) { in_ptr = save_in_ptr; eprintf (" ("); assert (!fetch_constructor_contacts_contacts_not_modified (T)); if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); } eprintf (" )");return 0; }
  in_ptr = save_in_ptr;
  if (skip_constructor_contacts_contacts (T) >= 0) { in_ptr = save_in_ptr; eprintf (" ("); assert (!fetch_constructor_contacts_contacts (T)); if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); } eprintf (" )");return 0; }
  in_ptr = save_in_ptr;
  return -1;
}
int fetch_type_contacts_found (struct paramed_type *T) {
  if (in_remaining () < 4) { return -1;}
  if (multiline_output >= 2) { multiline_offset += multiline_offset_size; }
  eprintf (" (");
  int magic = fetch_int ();
  int res = -1;
  switch (magic) {
  case 0x0566000e: res = fetch_constructor_contacts_found (T); break;
  default: return -1;
  }
  if (res >= 0) {
    if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); }
    eprintf (" )");
  }
  return res;
}
int fetch_type_bare_contacts_found (struct paramed_type *T) {
  if (multiline_output >= 2) { multiline_offset += multiline_offset_size; }
  eprintf (" (");
  if (fetch_constructor_contacts_found (T) >= 0) { if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); } eprintf (" )");return 0; }
  return -1;
}
int fetch_type_contacts_imported_contacts (struct paramed_type *T) {
  if (in_remaining () < 4) { return -1;}
  if (multiline_output >= 2) { multiline_offset += multiline_offset_size; }
  eprintf (" (");
  int magic = fetch_int ();
  int res = -1;
  switch (magic) {
  case 0xad524315: res = fetch_constructor_contacts_imported_contacts (T); break;
  default: return -1;
  }
  if (res >= 0) {
    if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); }
    eprintf (" )");
  }
  return res;
}
int fetch_type_bare_contacts_imported_contacts (struct paramed_type *T) {
  if (multiline_output >= 2) { multiline_offset += multiline_offset_size; }
  eprintf (" (");
  if (fetch_constructor_contacts_imported_contacts (T) >= 0) { if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); } eprintf (" )");return 0; }
  return -1;
}
int fetch_type_contacts_link (struct paramed_type *T) {
  if (in_remaining () < 4) { return -1;}
  if (multiline_output >= 2) { multiline_offset += multiline_offset_size; }
  eprintf (" (");
  int magic = fetch_int ();
  int res = -1;
  switch (magic) {
  case 0x3ace484c: res = fetch_constructor_contacts_link (T); break;
  default: return -1;
  }
  if (res >= 0) {
    if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); }
    eprintf (" )");
  }
  return res;
}
int fetch_type_bare_contacts_link (struct paramed_type *T) {
  if (multiline_output >= 2) { multiline_offset += multiline_offset_size; }
  eprintf (" (");
  if (fetch_constructor_contacts_link (T) >= 0) { if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); } eprintf (" )");return 0; }
  return -1;
}
int fetch_type_contacts_suggested (struct paramed_type *T) {
  if (in_remaining () < 4) { return -1;}
  if (multiline_output >= 2) { multiline_offset += multiline_offset_size; }
  eprintf (" (");
  int magic = fetch_int ();
  int res = -1;
  switch (magic) {
  case 0x5649dcc5: res = fetch_constructor_contacts_suggested (T); break;
  default: return -1;
  }
  if (res >= 0) {
    if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); }
    eprintf (" )");
  }
  return res;
}
int fetch_type_bare_contacts_suggested (struct paramed_type *T) {
  if (multiline_output >= 2) { multiline_offset += multiline_offset_size; }
  eprintf (" (");
  if (fetch_constructor_contacts_suggested (T) >= 0) { if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); } eprintf (" )");return 0; }
  return -1;
}
int fetch_type_geochats_located (struct paramed_type *T) {
  if (in_remaining () < 4) { return -1;}
  if (multiline_output >= 2) { multiline_offset += multiline_offset_size; }
  eprintf (" (");
  int magic = fetch_int ();
  int res = -1;
  switch (magic) {
  case 0x48feb267: res = fetch_constructor_geochats_located (T); break;
  default: return -1;
  }
  if (res >= 0) {
    if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); }
    eprintf (" )");
  }
  return res;
}
int fetch_type_bare_geochats_located (struct paramed_type *T) {
  if (multiline_output >= 2) { multiline_offset += multiline_offset_size; }
  eprintf (" (");
  if (fetch_constructor_geochats_located (T) >= 0) { if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); } eprintf (" )");return 0; }
  return -1;
}
int fetch_type_geochats_messages (struct paramed_type *T) {
  if (in_remaining () < 4) { return -1;}
  if (multiline_output >= 2) { multiline_offset += multiline_offset_size; }
  eprintf (" (");
  int magic = fetch_int ();
  int res = -1;
  switch (magic) {
  case 0xd1526db1: res = fetch_constructor_geochats_messages (T); break;
  case 0xbc5863e8: res = fetch_constructor_geochats_messages_slice (T); break;
  default: return -1;
  }
  if (res >= 0) {
    if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); }
    eprintf (" )");
  }
  return res;
}
int fetch_type_bare_geochats_messages (struct paramed_type *T) {
  int *save_in_ptr = in_ptr;
  if (multiline_output >= 2) { multiline_offset += multiline_offset_size; }
  if (skip_constructor_geochats_messages (T) >= 0) { in_ptr = save_in_ptr; eprintf (" ("); assert (!fetch_constructor_geochats_messages (T)); if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); } eprintf (" )");return 0; }
  in_ptr = save_in_ptr;
  if (skip_constructor_geochats_messages_slice (T) >= 0) { in_ptr = save_in_ptr; eprintf (" ("); assert (!fetch_constructor_geochats_messages_slice (T)); if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); } eprintf (" )");return 0; }
  in_ptr = save_in_ptr;
  return -1;
}
int fetch_type_geochats_stated_message (struct paramed_type *T) {
  if (in_remaining () < 4) { return -1;}
  if (multiline_output >= 2) { multiline_offset += multiline_offset_size; }
  eprintf (" (");
  int magic = fetch_int ();
  int res = -1;
  switch (magic) {
  case 0x17b1578b: res = fetch_constructor_geochats_stated_message (T); break;
  default: return -1;
  }
  if (res >= 0) {
    if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); }
    eprintf (" )");
  }
  return res;
}
int fetch_type_bare_geochats_stated_message (struct paramed_type *T) {
  if (multiline_output >= 2) { multiline_offset += multiline_offset_size; }
  eprintf (" (");
  if (fetch_constructor_geochats_stated_message (T) >= 0) { if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); } eprintf (" )");return 0; }
  return -1;
}
int fetch_type_help_app_update (struct paramed_type *T) {
  if (in_remaining () < 4) { return -1;}
  if (multiline_output >= 2) { multiline_offset += multiline_offset_size; }
  eprintf (" (");
  int magic = fetch_int ();
  int res = -1;
  switch (magic) {
  case 0x8987f311: res = fetch_constructor_help_app_update (T); break;
  case 0xc45a6536: res = fetch_constructor_help_no_app_update (T); break;
  default: return -1;
  }
  if (res >= 0) {
    if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); }
    eprintf (" )");
  }
  return res;
}
int fetch_type_bare_help_app_update (struct paramed_type *T) {
  int *save_in_ptr = in_ptr;
  if (multiline_output >= 2) { multiline_offset += multiline_offset_size; }
  if (skip_constructor_help_app_update (T) >= 0) { in_ptr = save_in_ptr; eprintf (" ("); assert (!fetch_constructor_help_app_update (T)); if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); } eprintf (" )");return 0; }
  in_ptr = save_in_ptr;
  if (skip_constructor_help_no_app_update (T) >= 0) { in_ptr = save_in_ptr; eprintf (" ("); assert (!fetch_constructor_help_no_app_update (T)); if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); } eprintf (" )");return 0; }
  in_ptr = save_in_ptr;
  return -1;
}
int fetch_type_help_invite_text (struct paramed_type *T) {
  if (in_remaining () < 4) { return -1;}
  int magic = fetch_int ();
  int res = -1;
  switch (magic) {
  case 0x18cb9f78: res = fetch_constructor_help_invite_text (T); break;
  default: return -1;
  }
  return res;
}
int fetch_type_bare_help_invite_text (struct paramed_type *T) {
  if (fetch_constructor_help_invite_text (T) >= 0) { return 0; }
  return -1;
}
int fetch_type_help_support (struct paramed_type *T) {
  if (in_remaining () < 4) { return -1;}
  if (multiline_output >= 2) { multiline_offset += multiline_offset_size; }
  eprintf (" (");
  int magic = fetch_int ();
  int res = -1;
  switch (magic) {
  case 0x17c6b5f6: res = fetch_constructor_help_support (T); break;
  default: return -1;
  }
  if (res >= 0) {
    if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); }
    eprintf (" )");
  }
  return res;
}
int fetch_type_bare_help_support (struct paramed_type *T) {
  if (multiline_output >= 2) { multiline_offset += multiline_offset_size; }
  eprintf (" (");
  if (fetch_constructor_help_support (T) >= 0) { if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); } eprintf (" )");return 0; }
  return -1;
}
int fetch_type_messages_affected_history (struct paramed_type *T) {
  if (in_remaining () < 4) { return -1;}
  if (multiline_output >= 2) { multiline_offset += multiline_offset_size; }
  eprintf (" (");
  int magic = fetch_int ();
  int res = -1;
  switch (magic) {
  case 0xb45c69d1: res = fetch_constructor_messages_affected_history (T); break;
  default: return -1;
  }
  if (res >= 0) {
    if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); }
    eprintf (" )");
  }
  return res;
}
int fetch_type_bare_messages_affected_history (struct paramed_type *T) {
  if (multiline_output >= 2) { multiline_offset += multiline_offset_size; }
  eprintf (" (");
  if (fetch_constructor_messages_affected_history (T) >= 0) { if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); } eprintf (" )");return 0; }
  return -1;
}
int fetch_type_messages_affected_messages (struct paramed_type *T) {
  if (in_remaining () < 4) { return -1;}
  if (multiline_output >= 2) { multiline_offset += multiline_offset_size; }
  eprintf (" (");
  int magic = fetch_int ();
  int res = -1;
  switch (magic) {
  case 0x84d19185: res = fetch_constructor_messages_affected_messages (T); break;
  default: return -1;
  }
  if (res >= 0) {
    if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); }
    eprintf (" )");
  }
  return res;
}
int fetch_type_bare_messages_affected_messages (struct paramed_type *T) {
  if (multiline_output >= 2) { multiline_offset += multiline_offset_size; }
  eprintf (" (");
  if (fetch_constructor_messages_affected_messages (T) >= 0) { if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); } eprintf (" )");return 0; }
  return -1;
}
int fetch_type_messages_all_stickers (struct paramed_type *T) {
  if (in_remaining () < 4) { return -1;}
  if (multiline_output >= 2) { multiline_offset += multiline_offset_size; }
  eprintf (" (");
  int magic = fetch_int ();
  int res = -1;
  switch (magic) {
  case 0xe86602c3: res = fetch_constructor_messages_all_stickers_not_modified (T); break;
  case 0x5ce352ec: res = fetch_constructor_messages_all_stickers (T); break;
  default: return -1;
  }
  if (res >= 0) {
    if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); }
    eprintf (" )");
  }
  return res;
}
int fetch_type_bare_messages_all_stickers (struct paramed_type *T) {
  int *save_in_ptr = in_ptr;
  if (multiline_output >= 2) { multiline_offset += multiline_offset_size; }
  if (skip_constructor_messages_all_stickers_not_modified (T) >= 0) { in_ptr = save_in_ptr; eprintf (" ("); assert (!fetch_constructor_messages_all_stickers_not_modified (T)); if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); } eprintf (" )");return 0; }
  in_ptr = save_in_ptr;
  if (skip_constructor_messages_all_stickers (T) >= 0) { in_ptr = save_in_ptr; eprintf (" ("); assert (!fetch_constructor_messages_all_stickers (T)); if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); } eprintf (" )");return 0; }
  in_ptr = save_in_ptr;
  return -1;
}
int fetch_type_messages_chat_full (struct paramed_type *T) {
  if (in_remaining () < 4) { return -1;}
  if (multiline_output >= 2) { multiline_offset += multiline_offset_size; }
  eprintf (" (");
  int magic = fetch_int ();
  int res = -1;
  switch (magic) {
  case 0xe5d7d19c: res = fetch_constructor_messages_chat_full (T); break;
  default: return -1;
  }
  if (res >= 0) {
    if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); }
    eprintf (" )");
  }
  return res;
}
int fetch_type_bare_messages_chat_full (struct paramed_type *T) {
  if (multiline_output >= 2) { multiline_offset += multiline_offset_size; }
  eprintf (" (");
  if (fetch_constructor_messages_chat_full (T) >= 0) { if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); } eprintf (" )");return 0; }
  return -1;
}
int fetch_type_messages_chats (struct paramed_type *T) {
  if (in_remaining () < 4) { return -1;}
  int magic = fetch_int ();
  int res = -1;
  switch (magic) {
  case 0x64ff9fd5: res = fetch_constructor_messages_chats (T); break;
  default: return -1;
  }
  return res;
}
int fetch_type_bare_messages_chats (struct paramed_type *T) {
  if (fetch_constructor_messages_chats (T) >= 0) { return 0; }
  return -1;
}
int fetch_type_messages_dh_config (struct paramed_type *T) {
  if (in_remaining () < 4) { return -1;}
  if (multiline_output >= 2) { multiline_offset += multiline_offset_size; }
  eprintf (" (");
  int magic = fetch_int ();
  int res = -1;
  switch (magic) {
  case 0xc0e24635: res = fetch_constructor_messages_dh_config_not_modified (T); break;
  case 0x2c221edd: res = fetch_constructor_messages_dh_config (T); break;
  default: return -1;
  }
  if (res >= 0) {
    if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); }
    eprintf (" )");
  }
  return res;
}
int fetch_type_bare_messages_dh_config (struct paramed_type *T) {
  int *save_in_ptr = in_ptr;
  if (multiline_output >= 2) { multiline_offset += multiline_offset_size; }
  if (skip_constructor_messages_dh_config_not_modified (T) >= 0) { in_ptr = save_in_ptr; eprintf (" ("); assert (!fetch_constructor_messages_dh_config_not_modified (T)); if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); } eprintf (" )");return 0; }
  in_ptr = save_in_ptr;
  if (skip_constructor_messages_dh_config (T) >= 0) { in_ptr = save_in_ptr; eprintf (" ("); assert (!fetch_constructor_messages_dh_config (T)); if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); } eprintf (" )");return 0; }
  in_ptr = save_in_ptr;
  return -1;
}
int fetch_type_messages_dialogs (struct paramed_type *T) {
  if (in_remaining () < 4) { return -1;}
  if (multiline_output >= 2) { multiline_offset += multiline_offset_size; }
  eprintf (" (");
  int magic = fetch_int ();
  int res = -1;
  switch (magic) {
  case 0x15ba6c40: res = fetch_constructor_messages_dialogs (T); break;
  case 0x71e094f3: res = fetch_constructor_messages_dialogs_slice (T); break;
  default: return -1;
  }
  if (res >= 0) {
    if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); }
    eprintf (" )");
  }
  return res;
}
int fetch_type_bare_messages_dialogs (struct paramed_type *T) {
  int *save_in_ptr = in_ptr;
  if (multiline_output >= 2) { multiline_offset += multiline_offset_size; }
  if (skip_constructor_messages_dialogs (T) >= 0) { in_ptr = save_in_ptr; eprintf (" ("); assert (!fetch_constructor_messages_dialogs (T)); if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); } eprintf (" )");return 0; }
  in_ptr = save_in_ptr;
  if (skip_constructor_messages_dialogs_slice (T) >= 0) { in_ptr = save_in_ptr; eprintf (" ("); assert (!fetch_constructor_messages_dialogs_slice (T)); if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); } eprintf (" )");return 0; }
  in_ptr = save_in_ptr;
  return -1;
}
int fetch_type_messages_message (struct paramed_type *T) {
  if (in_remaining () < 4) { return -1;}
  if (multiline_output >= 2) { multiline_offset += multiline_offset_size; }
  eprintf (" (");
  int magic = fetch_int ();
  int res = -1;
  switch (magic) {
  case 0x3f4e0648: res = fetch_constructor_messages_message_empty (T); break;
  default: return -1;
  }
  if (res >= 0) {
    if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); }
    eprintf (" )");
  }
  return res;
}
int fetch_type_bare_messages_message (struct paramed_type *T) {
  if (multiline_output >= 2) { multiline_offset += multiline_offset_size; }
  eprintf (" (");
  if (fetch_constructor_messages_message_empty (T) >= 0) { if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); } eprintf (" )");return 0; }
  return -1;
}
int fetch_type_messages_messages (struct paramed_type *T) {
  if (in_remaining () < 4) { return -1;}
  if (multiline_output >= 2) { multiline_offset += multiline_offset_size; }
  eprintf (" (");
  int magic = fetch_int ();
  int res = -1;
  switch (magic) {
  case 0x8c718e87: res = fetch_constructor_messages_messages (T); break;
  case 0x0b446ae3: res = fetch_constructor_messages_messages_slice (T); break;
  default: return -1;
  }
  if (res >= 0) {
    if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); }
    eprintf (" )");
  }
  return res;
}
int fetch_type_bare_messages_messages (struct paramed_type *T) {
  int *save_in_ptr = in_ptr;
  if (multiline_output >= 2) { multiline_offset += multiline_offset_size; }
  if (skip_constructor_messages_messages (T) >= 0) { in_ptr = save_in_ptr; eprintf (" ("); assert (!fetch_constructor_messages_messages (T)); if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); } eprintf (" )");return 0; }
  in_ptr = save_in_ptr;
  if (skip_constructor_messages_messages_slice (T) >= 0) { in_ptr = save_in_ptr; eprintf (" ("); assert (!fetch_constructor_messages_messages_slice (T)); if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); } eprintf (" )");return 0; }
  in_ptr = save_in_ptr;
  return -1;
}
int fetch_type_messages_sent_encrypted_message (struct paramed_type *T) {
  if (in_remaining () < 4) { return -1;}
  if (multiline_output >= 2) { multiline_offset += multiline_offset_size; }
  eprintf (" (");
  int magic = fetch_int ();
  int res = -1;
  switch (magic) {
  case 0x560f8935: res = fetch_constructor_messages_sent_encrypted_message (T); break;
  case 0x9493ff32: res = fetch_constructor_messages_sent_encrypted_file (T); break;
  default: return -1;
  }
  if (res >= 0) {
    if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); }
    eprintf (" )");
  }
  return res;
}
int fetch_type_bare_messages_sent_encrypted_message (struct paramed_type *T) {
  int *save_in_ptr = in_ptr;
  if (multiline_output >= 2) { multiline_offset += multiline_offset_size; }
  if (skip_constructor_messages_sent_encrypted_message (T) >= 0) { in_ptr = save_in_ptr; eprintf (" ("); assert (!fetch_constructor_messages_sent_encrypted_message (T)); if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); } eprintf (" )");return 0; }
  in_ptr = save_in_ptr;
  if (skip_constructor_messages_sent_encrypted_file (T) >= 0) { in_ptr = save_in_ptr; eprintf (" ("); assert (!fetch_constructor_messages_sent_encrypted_file (T)); if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); } eprintf (" )");return 0; }
  in_ptr = save_in_ptr;
  return -1;
}
int fetch_type_messages_sent_message (struct paramed_type *T) {
  if (in_remaining () < 4) { return -1;}
  if (multiline_output >= 2) { multiline_offset += multiline_offset_size; }
  eprintf (" (");
  int magic = fetch_int ();
  int res = -1;
  switch (magic) {
  case 0x4c3d47f3: res = fetch_constructor_messages_sent_message (T); break;
  case 0x35a1a663: res = fetch_constructor_messages_sent_message_link (T); break;
  default: return -1;
  }
  if (res >= 0) {
    if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); }
    eprintf (" )");
  }
  return res;
}
int fetch_type_bare_messages_sent_message (struct paramed_type *T) {
  int *save_in_ptr = in_ptr;
  if (multiline_output >= 2) { multiline_offset += multiline_offset_size; }
  if (skip_constructor_messages_sent_message (T) >= 0) { in_ptr = save_in_ptr; eprintf (" ("); assert (!fetch_constructor_messages_sent_message (T)); if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); } eprintf (" )");return 0; }
  in_ptr = save_in_ptr;
  if (skip_constructor_messages_sent_message_link (T) >= 0) { in_ptr = save_in_ptr; eprintf (" ("); assert (!fetch_constructor_messages_sent_message_link (T)); if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); } eprintf (" )");return 0; }
  in_ptr = save_in_ptr;
  return -1;
}
int fetch_type_messages_sticker_set (struct paramed_type *T) {
  if (in_remaining () < 4) { return -1;}
  if (multiline_output >= 2) { multiline_offset += multiline_offset_size; }
  eprintf (" (");
  int magic = fetch_int ();
  int res = -1;
  switch (magic) {
  case 0xb60a24a6: res = fetch_constructor_messages_sticker_set (T); break;
  default: return -1;
  }
  if (res >= 0) {
    if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); }
    eprintf (" )");
  }
  return res;
}
int fetch_type_bare_messages_sticker_set (struct paramed_type *T) {
  if (multiline_output >= 2) { multiline_offset += multiline_offset_size; }
  eprintf (" (");
  if (fetch_constructor_messages_sticker_set (T) >= 0) { if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); } eprintf (" )");return 0; }
  return -1;
}
int fetch_type_messages_stickers (struct paramed_type *T) {
  if (in_remaining () < 4) { return -1;}
  if (multiline_output >= 2) { multiline_offset += multiline_offset_size; }
  eprintf (" (");
  int magic = fetch_int ();
  int res = -1;
  switch (magic) {
  case 0xf1749a22: res = fetch_constructor_messages_stickers_not_modified (T); break;
  case 0x8a8ecd32: res = fetch_constructor_messages_stickers (T); break;
  default: return -1;
  }
  if (res >= 0) {
    if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); }
    eprintf (" )");
  }
  return res;
}
int fetch_type_bare_messages_stickers (struct paramed_type *T) {
  int *save_in_ptr = in_ptr;
  if (multiline_output >= 2) { multiline_offset += multiline_offset_size; }
  if (skip_constructor_messages_stickers_not_modified (T) >= 0) { in_ptr = save_in_ptr; eprintf (" ("); assert (!fetch_constructor_messages_stickers_not_modified (T)); if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); } eprintf (" )");return 0; }
  in_ptr = save_in_ptr;
  if (skip_constructor_messages_stickers (T) >= 0) { in_ptr = save_in_ptr; eprintf (" ("); assert (!fetch_constructor_messages_stickers (T)); if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); } eprintf (" )");return 0; }
  in_ptr = save_in_ptr;
  return -1;
}
int fetch_type_photos_photo (struct paramed_type *T) {
  if (in_remaining () < 4) { return -1;}
  if (multiline_output >= 2) { multiline_offset += multiline_offset_size; }
  eprintf (" (");
  int magic = fetch_int ();
  int res = -1;
  switch (magic) {
  case 0x20212ca8: res = fetch_constructor_photos_photo (T); break;
  default: return -1;
  }
  if (res >= 0) {
    if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); }
    eprintf (" )");
  }
  return res;
}
int fetch_type_bare_photos_photo (struct paramed_type *T) {
  if (multiline_output >= 2) { multiline_offset += multiline_offset_size; }
  eprintf (" (");
  if (fetch_constructor_photos_photo (T) >= 0) { if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); } eprintf (" )");return 0; }
  return -1;
}
int fetch_type_photos_photos (struct paramed_type *T) {
  if (in_remaining () < 4) { return -1;}
  if (multiline_output >= 2) { multiline_offset += multiline_offset_size; }
  eprintf (" (");
  int magic = fetch_int ();
  int res = -1;
  switch (magic) {
  case 0x8dca6aa5: res = fetch_constructor_photos_photos (T); break;
  case 0x15051f54: res = fetch_constructor_photos_photos_slice (T); break;
  default: return -1;
  }
  if (res >= 0) {
    if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); }
    eprintf (" )");
  }
  return res;
}
int fetch_type_bare_photos_photos (struct paramed_type *T) {
  int *save_in_ptr = in_ptr;
  if (multiline_output >= 2) { multiline_offset += multiline_offset_size; }
  if (skip_constructor_photos_photos (T) >= 0) { in_ptr = save_in_ptr; eprintf (" ("); assert (!fetch_constructor_photos_photos (T)); if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); } eprintf (" )");return 0; }
  in_ptr = save_in_ptr;
  if (skip_constructor_photos_photos_slice (T) >= 0) { in_ptr = save_in_ptr; eprintf (" ("); assert (!fetch_constructor_photos_photos_slice (T)); if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); } eprintf (" )");return 0; }
  in_ptr = save_in_ptr;
  return -1;
}
int fetch_type_storage_file_type (struct paramed_type *T) {
  if (in_remaining () < 4) { return -1;}
  if (multiline_output >= 2) { multiline_offset += multiline_offset_size; }
  eprintf (" (");
  int magic = fetch_int ();
  int res = -1;
  switch (magic) {
  case 0xaa963b05: res = fetch_constructor_storage_file_unknown (T); break;
  case 0x007efe0e: res = fetch_constructor_storage_file_jpeg (T); break;
  case 0xcae1aadf: res = fetch_constructor_storage_file_gif (T); break;
  case 0x0a4f63c0: res = fetch_constructor_storage_file_png (T); break;
  case 0xae1e508d: res = fetch_constructor_storage_file_pdf (T); break;
  case 0x528a0677: res = fetch_constructor_storage_file_mp3 (T); break;
  case 0x4b09ebbc: res = fetch_constructor_storage_file_mov (T); break;
  case 0x40bc6f52: res = fetch_constructor_storage_file_partial (T); break;
  case 0xb3cea0e4: res = fetch_constructor_storage_file_mp4 (T); break;
  case 0x1081464c: res = fetch_constructor_storage_file_webp (T); break;
  default: return -1;
  }
  if (res >= 0) {
    if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); }
    eprintf (" )");
  }
  return res;
}
int fetch_type_bare_storage_file_type (struct paramed_type *T) {
  int *save_in_ptr = in_ptr;
  if (multiline_output >= 2) { multiline_offset += multiline_offset_size; }
  if (skip_constructor_storage_file_unknown (T) >= 0) { in_ptr = save_in_ptr; eprintf (" ("); assert (!fetch_constructor_storage_file_unknown (T)); if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); } eprintf (" )");return 0; }
  in_ptr = save_in_ptr;
  if (skip_constructor_storage_file_jpeg (T) >= 0) { in_ptr = save_in_ptr; eprintf (" ("); assert (!fetch_constructor_storage_file_jpeg (T)); if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); } eprintf (" )");return 0; }
  in_ptr = save_in_ptr;
  if (skip_constructor_storage_file_gif (T) >= 0) { in_ptr = save_in_ptr; eprintf (" ("); assert (!fetch_constructor_storage_file_gif (T)); if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); } eprintf (" )");return 0; }
  in_ptr = save_in_ptr;
  if (skip_constructor_storage_file_png (T) >= 0) { in_ptr = save_in_ptr; eprintf (" ("); assert (!fetch_constructor_storage_file_png (T)); if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); } eprintf (" )");return 0; }
  in_ptr = save_in_ptr;
  if (skip_constructor_storage_file_pdf (T) >= 0) { in_ptr = save_in_ptr; eprintf (" ("); assert (!fetch_constructor_storage_file_pdf (T)); if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); } eprintf (" )");return 0; }
  in_ptr = save_in_ptr;
  if (skip_constructor_storage_file_mp3 (T) >= 0) { in_ptr = save_in_ptr; eprintf (" ("); assert (!fetch_constructor_storage_file_mp3 (T)); if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); } eprintf (" )");return 0; }
  in_ptr = save_in_ptr;
  if (skip_constructor_storage_file_mov (T) >= 0) { in_ptr = save_in_ptr; eprintf (" ("); assert (!fetch_constructor_storage_file_mov (T)); if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); } eprintf (" )");return 0; }
  in_ptr = save_in_ptr;
  if (skip_constructor_storage_file_partial (T) >= 0) { in_ptr = save_in_ptr; eprintf (" ("); assert (!fetch_constructor_storage_file_partial (T)); if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); } eprintf (" )");return 0; }
  in_ptr = save_in_ptr;
  if (skip_constructor_storage_file_mp4 (T) >= 0) { in_ptr = save_in_ptr; eprintf (" ("); assert (!fetch_constructor_storage_file_mp4 (T)); if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); } eprintf (" )");return 0; }
  in_ptr = save_in_ptr;
  if (skip_constructor_storage_file_webp (T) >= 0) { in_ptr = save_in_ptr; eprintf (" ("); assert (!fetch_constructor_storage_file_webp (T)); if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); } eprintf (" )");return 0; }
  in_ptr = save_in_ptr;
  return -1;
}
int fetch_type_updates_difference (struct paramed_type *T) {
  if (in_remaining () < 4) { return -1;}
  if (multiline_output >= 2) { multiline_offset += multiline_offset_size; }
  eprintf (" (");
  int magic = fetch_int ();
  int res = -1;
  switch (magic) {
  case 0x5d75a138: res = fetch_constructor_updates_difference_empty (T); break;
  case 0x00f49ca0: res = fetch_constructor_updates_difference (T); break;
  case 0xa8fb1981: res = fetch_constructor_updates_difference_slice (T); break;
  default: return -1;
  }
  if (res >= 0) {
    if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); }
    eprintf (" )");
  }
  return res;
}
int fetch_type_bare_updates_difference (struct paramed_type *T) {
  int *save_in_ptr = in_ptr;
  if (multiline_output >= 2) { multiline_offset += multiline_offset_size; }
  if (skip_constructor_updates_difference_empty (T) >= 0) { in_ptr = save_in_ptr; eprintf (" ("); assert (!fetch_constructor_updates_difference_empty (T)); if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); } eprintf (" )");return 0; }
  in_ptr = save_in_ptr;
  if (skip_constructor_updates_difference (T) >= 0) { in_ptr = save_in_ptr; eprintf (" ("); assert (!fetch_constructor_updates_difference (T)); if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); } eprintf (" )");return 0; }
  in_ptr = save_in_ptr;
  if (skip_constructor_updates_difference_slice (T) >= 0) { in_ptr = save_in_ptr; eprintf (" ("); assert (!fetch_constructor_updates_difference_slice (T)); if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); } eprintf (" )");return 0; }
  in_ptr = save_in_ptr;
  return -1;
}
int fetch_type_updates_state (struct paramed_type *T) {
  if (in_remaining () < 4) { return -1;}
  if (multiline_output >= 2) { multiline_offset += multiline_offset_size; }
  eprintf (" (");
  int magic = fetch_int ();
  int res = -1;
  switch (magic) {
  case 0xa56c2a3e: res = fetch_constructor_updates_state (T); break;
  default: return -1;
  }
  if (res >= 0) {
    if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); }
    eprintf (" )");
  }
  return res;
}
int fetch_type_bare_updates_state (struct paramed_type *T) {
  if (multiline_output >= 2) { multiline_offset += multiline_offset_size; }
  eprintf (" (");
  if (fetch_constructor_updates_state (T) >= 0) { if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); } eprintf (" )");return 0; }
  return -1;
}
int fetch_type_upload_file (struct paramed_type *T) {
  if (in_remaining () < 4) { return -1;}
  if (multiline_output >= 2) { multiline_offset += multiline_offset_size; }
  eprintf (" (");
  int magic = fetch_int ();
  int res = -1;
  switch (magic) {
  case 0x096a18d5: res = fetch_constructor_upload_file (T); break;
  default: return -1;
  }
  if (res >= 0) {
    if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); }
    eprintf (" )");
  }
  return res;
}
int fetch_type_bare_upload_file (struct paramed_type *T) {
  if (multiline_output >= 2) { multiline_offset += multiline_offset_size; }
  eprintf (" (");
  if (fetch_constructor_upload_file (T) >= 0) { if (multiline_output >= 2) { multiline_offset -= multiline_offset_size; print_offset (); } eprintf (" )");return 0; }
  return -1;
}
int fetch_type_any (struct paramed_type *T) {
  switch (T->type->name) {
  case 0xb8d0afdf: return fetch_type_account_days_t_t_l (T);
  case 0x472f5020: return fetch_type_bare_account_days_t_t_l (T);
  case 0x9fc5ec4e: return fetch_type_audio (T);
  case 0x603a13b1: return fetch_type_bare_audio (T);
  case 0x7bf2e6f6: return fetch_type_authorization (T);
  case 0x840d1909: return fetch_type_bare_authorization (T);
  case 0x250be282: return fetch_type_bool (T);
  case 0xdaf41d7d: return fetch_type_bare_bool (T);
  case 0x75e7ea6c: return fetch_type_bot_command (T);
  case 0x8a181593: return fetch_type_bare_bot_command (T);
  case 0xb2e16f93: return fetch_type_bot_info (T);
  case 0x4d1e906c: return fetch_type_bare_bot_info (T);
  case 0x0ee1379f: return fetch_type_bytes (T);
  case 0xf11ec860: return fetch_type_bare_bytes (T);
  case 0x7bd865dc: return fetch_type_chat (T);
  case 0x84279a23: return fetch_type_bare_chat (T);
  case 0x2e02a614: return fetch_type_chat_full (T);
  case 0xd1fd59eb: return fetch_type_bare_chat_full (T);
  case 0x94f910b1: return fetch_type_chat_invite (T);
  case 0x6b06ef4e: return fetch_type_bare_chat_invite (T);
  case 0x3631cf4c: return fetch_type_chat_located (T);
  case 0xc9ce30b3: return fetch_type_bare_chat_located (T);
  case 0xc8d7493e: return fetch_type_chat_participant (T);
  case 0x3728b6c1: return fetch_type_bare_chat_participant (T);
  case 0x77930f9f: return fetch_type_chat_participants (T);
  case 0x886cf060: return fetch_type_bare_chat_participants (T);
  case 0x56922676: return fetch_type_chat_photo (T);
  case 0xa96dd989: return fetch_type_bare_chat_photo (T);
  case 0x6643b654: return fetch_type_client_d_h_inner_data (T);
  case 0x99bc49ab: return fetch_type_bare_client_d_h_inner_data (T);
  case 0x4e32b894: return fetch_type_config (T);
  case 0xb1cd476b: return fetch_type_bare_config (T);
  case 0xf911c994: return fetch_type_contact (T);
  case 0x06ee366b: return fetch_type_bare_contact (T);
  case 0x561bc879: return fetch_type_contact_blocked (T);
  case 0xa9e43786: return fetch_type_bare_contact_blocked (T);
  case 0xea879f95: return fetch_type_contact_found (T);
  case 0x1578606a: return fetch_type_bare_contact_found (T);
  case 0x522fbc63: return fetch_type_contact_link (T);
  case 0xadd0439c: return fetch_type_bare_contact_link (T);
  case 0xd3680c61: return fetch_type_contact_status (T);
  case 0x2c97f39e: return fetch_type_bare_contact_status (T);
  case 0x3de191a1: return fetch_type_contact_suggested (T);
  case 0xc21e6e5e: return fetch_type_bare_contact_suggested (T);
  case 0x2b1a62f0: return fetch_type_dc_option (T);
  case 0xd4e59d0f: return fetch_type_bare_dc_option (T);
  case 0x535b7918: return fetch_type_decrypted_message (T);
  case 0xaca486e7: return fetch_type_bare_decrypted_message (T);
  case 0x4e0eefde: return fetch_type_decrypted_message_action (T);
  case 0xb1f11021: return fetch_type_bare_decrypted_message_action (T);
  case 0x1be31789: return fetch_type_decrypted_message_layer (T);
  case 0xe41ce876: return fetch_type_bare_decrypted_message_layer (T);
  case 0x34e0d674: return fetch_type_decrypted_message_media (T);
  case 0xcb1f298b: return fetch_type_bare_decrypted_message_media (T);
  case 0xc1dd804a: return fetch_type_dialog (T);
  case 0x3e227fb5: return fetch_type_bare_dialog (T);
  case 0xae636f24: return fetch_type_disabled_feature (T);
  case 0x519c90db: return fetch_type_bare_disabled_feature (T);
  case 0x51a73418: return fetch_type_document (T);
  case 0xae58cbe7: return fetch_type_bare_document (T);
  case 0x97c637a3: return fetch_type_document_attribute (T);
  case 0x6839c85c: return fetch_type_bare_document_attribute (T);
  case 0x2210c154: return fetch_type_double (T);
  case 0xddef3eab: return fetch_type_bare_double (T);
  case 0xb1718213: return fetch_type_encrypted_chat (T);
  case 0x4e8e7dec: return fetch_type_bare_encrypted_chat (T);
  case 0x886fd032: return fetch_type_encrypted_file (T);
  case 0x77902fcd: return fetch_type_bare_encrypted_file (T);
  case 0xce6b8a1e: return fetch_type_encrypted_message (T);
  case 0x319475e1: return fetch_type_bare_encrypted_message (T);
  case 0xc4b9f9bb: return fetch_type_error (T);
  case 0x3b460644: return fetch_type_bare_error (T);
  case 0x95f132d5: return fetch_type_exported_chat_invite (T);
  case 0x6a0ecd2a: return fetch_type_bare_exported_chat_invite (T);
  case 0x2f8ffb30: return fetch_type_file_location (T);
  case 0xd07004cf: return fetch_type_bare_file_location (T);
  case 0xf67b4034: return fetch_type_geo_chat_message (T);
  case 0x0984bfcb: return fetch_type_bare_geo_chat_message (T);
  case 0x315e0a53: return fetch_type_geo_point (T);
  case 0xcea1f5ac: return fetch_type_bare_geo_point (T);
  case 0xd0028438: return fetch_type_imported_contact (T);
  case 0x2ffd7bc7: return fetch_type_bare_imported_contact (T);
  case 0x770656a8: return fetch_type_input_app_event (T);
  case 0x88f9a957: return fetch_type_bare_input_app_event (T);
  case 0xae8e9c7b: return fetch_type_input_audio (T);
  case 0x51716384: return fetch_type_bare_input_audio (T);
  case 0x3a60776d: return fetch_type_input_chat_photo (T);
  case 0xc59f8892: return fetch_type_bare_input_chat_photo (T);
  case 0xf392b7f4: return fetch_type_input_contact (T);
  case 0x0c6d480b: return fetch_type_bare_input_contact (T);
  case 0x6a8963fc: return fetch_type_input_document (T);
  case 0x95769c03: return fetch_type_bare_input_document (T);
  case 0xf141b5e1: return fetch_type_input_encrypted_chat (T);
  case 0x0ebe4a1e: return fetch_type_bare_input_encrypted_chat (T);
  case 0x0b5c064f: return fetch_type_input_encrypted_file (T);
  case 0xf4a3f9b0: return fetch_type_bare_input_encrypted_file (T);
  case 0x0f60f9ca: return fetch_type_input_file (T);
  case 0xf09f0635: return fetch_type_bare_input_file (T);
  case 0xe6daa38b: return fetch_type_input_file_location (T);
  case 0x19255c74: return fetch_type_bare_input_file_location (T);
  case 0x74d456fa: return fetch_type_input_geo_chat (T);
  case 0x8b2ba905: return fetch_type_bare_input_geo_chat (T);
  case 0x17768f1f: return fetch_type_input_geo_point (T);
  case 0xe88970e0: return fetch_type_bare_input_geo_point (T);
  case 0xb32c91dd: return fetch_type_input_media (T);
  case 0x4cd36e22: return fetch_type_bare_input_media (T);
  case 0x02b6911b: return fetch_type_input_notify_peer (T);
  case 0xfd496ee4: return fetch_type_bare_input_notify_peer (T);
  case 0x9e67268d: return fetch_type_input_peer (T);
  case 0x6198d972: return fetch_type_bare_input_peer (T);
  case 0x185a48ac: return fetch_type_input_peer_notify_events (T);
  case 0xe7a5b753: return fetch_type_bare_input_peer_notify_events (T);
  case 0x46a2ce98: return fetch_type_input_peer_notify_settings (T);
  case 0xb95d3167: return fetch_type_bare_input_peer_notify_settings (T);
  case 0xe74279c9: return fetch_type_input_photo (T);
  case 0x18bd8636: return fetch_type_bare_input_photo (T);
  case 0x7477e321: return fetch_type_input_photo_crop (T);
  case 0x8b881cde: return fetch_type_bare_input_photo_crop (T);
  case 0x4f96cb18: return fetch_type_input_privacy_key (T);
  case 0xb06934e7: return fetch_type_bare_input_privacy_key (T);
  case 0x4b815163: return fetch_type_input_privacy_rule (T);
  case 0xb47eae9c: return fetch_type_bare_input_privacy_rule (T);
  case 0xe44d415c: return fetch_type_input_sticker_set (T);
  case 0x1bb2bea3: return fetch_type_bare_input_sticker_set (T);
  case 0xadfe0c6a: return fetch_type_input_user (T);
  case 0x5201f395: return fetch_type_bare_input_user (T);
  case 0xbb5f7a27: return fetch_type_input_video (T);
  case 0x44a085d8: return fetch_type_bare_input_video (T);
  case 0xa8509bda: return fetch_type_int (T);
  case 0x57af6425: return fetch_type_bare_int (T);
  case 0x7d36c439: return fetch_type_int128 (T);
  case 0x82c93bc6: return fetch_type_bare_int128 (T);
  case 0xf2c798b3: return fetch_type_int256 (T);
  case 0x0d38674c: return fetch_type_bare_int256 (T);
  case 0xa2fa4880: return fetch_type_keyboard_button (T);
  case 0x5d05b77f: return fetch_type_bare_keyboard_button (T);
  case 0x77608b83: return fetch_type_keyboard_button_row (T);
  case 0x889f747c: return fetch_type_bare_keyboard_button_row (T);
  case 0x22076cba: return fetch_type_long (T);
  case 0xddf89345: return fetch_type_bare_long (T);
  case 0x5d652a7f: return fetch_type_message (T);
  case 0xa29ad580: return fetch_type_bare_message (T);
  case 0x383415fa: return fetch_type_message_action (T);
  case 0xc7cbea05: return fetch_type_bare_message_action (T);
  case 0x95f2471a: return fetch_type_message_media (T);
  case 0x6a0db8e5: return fetch_type_bare_message_media (T);
  case 0x80895ae0: return fetch_type_messages_filter (T);
  case 0x7f76a51f: return fetch_type_bare_messages_filter (T);
  case 0x8e1a1775: return fetch_type_nearest_dc (T);
  case 0x71e5e88a: return fetch_type_bare_nearest_dc (T);
  case 0x9fcb8237: return fetch_type_notify_peer (T);
  case 0x60347dc8: return fetch_type_bare_notify_peer (T);
  case 0x56730bcc: return fetch_type_null (T);
  case 0xa98cf433: return fetch_type_bare_null (T);
  case 0xbfa3de38: return fetch_type_p_q_inner_data (T);
  case 0x405c21c7: return fetch_type_bare_p_q_inner_data (T);
  case 0x276159d6: return fetch_type_peer (T);
  case 0xd89ea629: return fetch_type_bare_peer (T);
  case 0xc0c8d13b: return fetch_type_peer_notify_events (T);
  case 0x3f372ec4: return fetch_type_bare_peer_notify_events (T);
  case 0xfdf894fc: return fetch_type_peer_notify_settings (T);
  case 0x02076b03: return fetch_type_bare_peer_notify_settings (T);
  case 0xc207550a: return fetch_type_photo (T);
  case 0x3df8aaf5: return fetch_type_bare_photo (T);
  case 0x900f60dd: return fetch_type_photo_size (T);
  case 0x6ff09f22: return fetch_type_bare_photo_size (T);
  case 0xbc2eab30: return fetch_type_privacy_key (T);
  case 0x43d154cf: return fetch_type_bare_privacy_key (T);
  case 0xa8638aec: return fetch_type_privacy_rule (T);
  case 0x579c7513: return fetch_type_bare_privacy_rule (T);
  case 0xa384b779: return fetch_type_received_notify_message (T);
  case 0x5c7b4886: return fetch_type_bare_received_notify_message (T);
  case 0x612ca4a9: return fetch_type_reply_markup (T);
  case 0x9ed35b56: return fetch_type_bare_reply_markup (T);
  case 0x05162463: return fetch_type_res_p_q (T);
  case 0xfae9db9c: return fetch_type_bare_res_p_q (T);
  case 0x70541c7e: return fetch_type_send_message_action (T);
  case 0x8fabe381: return fetch_type_bare_send_message_action (T);
  case 0xa9230301: return fetch_type_server_d_h_params (T);
  case 0x56dcfcfe: return fetch_type_bare_server_d_h_params (T);
  case 0xb5890dba: return fetch_type_server_d_h_inner_data (T);
  case 0x4a76f245: return fetch_type_bare_server_d_h_inner_data (T);
  case 0xdb8a468f: return fetch_type_set_client_d_h_params_answer (T);
  case 0x2475b970: return fetch_type_bare_set_client_d_h_params_answer (T);
  case 0x12b299d4: return fetch_type_sticker_pack (T);
  case 0xed4d662b: return fetch_type_bare_sticker_pack (T);
  case 0xa7a43b17: return fetch_type_sticker_set (T);
  case 0x585bc4e8: return fetch_type_bare_sticker_set (T);
  case 0xb5286e24: return fetch_type_string (T);
  case 0x4ad791db: return fetch_type_bare_string (T);
  case 0x2e02b27e: return fetch_type_update (T);
  case 0xd1fd4d81: return fetch_type_bare_update (T);
  case 0x22499d27: return fetch_type_updates (T);
  case 0xddb662d8: return fetch_type_bare_updates (T);
  case 0x02e6c0c8: return fetch_type_user (T);
  case 0xfd193f37: return fetch_type_bare_user (T);
  case 0x5a89ac5b: return fetch_type_user_full (T);
  case 0xa57653a4: return fetch_type_bare_user_full (T);
  case 0x9a486229: return fetch_type_user_profile_photo (T);
  case 0x65b79dd6: return fetch_type_bare_user_profile_photo (T);
  case 0x76de9570: return fetch_type_user_status (T);
  case 0x89216a8f: return fetch_type_bare_user_status (T);
  case 0x1cb5c415: return fetch_type_vector (T);
  case 0xe34a3bea: return fetch_type_bare_vector (T);
  case 0x1716b174: return fetch_type_video (T);
  case 0xe8e94e8b: return fetch_type_bare_video (T);
  case 0xafa14973: return fetch_type_wall_paper (T);
  case 0x505eb68c: return fetch_type_bare_wall_paper (T);
  case 0x8d8c0d41: return fetch_type_web_page (T);
  case 0x7273f2be: return fetch_type_bare_web_page (T);
  case 0x1250abde: return fetch_type_account_authorizations (T);
  case 0xedaf5421: return fetch_type_bare_account_authorizations (T);
  case 0xeac2a804: return fetch_type_account_password (T);
  case 0x153d57fb: return fetch_type_bare_account_password (T);
  case 0xbcfc532c: return fetch_type_account_password_input_settings (T);
  case 0x4303acd3: return fetch_type_bare_account_password_input_settings (T);
  case 0xb7b72ab3: return fetch_type_account_password_settings (T);
  case 0x4848d54c: return fetch_type_bare_account_password_settings (T);
  case 0x554abb6f: return fetch_type_account_privacy_rules (T);
  case 0xaab54490: return fetch_type_bare_account_privacy_rules (T);
  case 0xa4f58c4c: return fetch_type_account_sent_change_phone_code (T);
  case 0x5b0a73b3: return fetch_type_bare_account_sent_change_phone_code (T);
  case 0xff036af1: return fetch_type_auth_authorization (T);
  case 0x00fc950e: return fetch_type_bare_auth_authorization (T);
  case 0x811ea28e: return fetch_type_auth_checked_phone (T);
  case 0x7ee15d71: return fetch_type_bare_auth_checked_phone (T);
  case 0xdf969c2d: return fetch_type_auth_exported_authorization (T);
  case 0x206963d2: return fetch_type_bare_auth_exported_authorization (T);
  case 0x137948a5: return fetch_type_auth_password_recovery (T);
  case 0xec86b75a: return fetch_type_bare_auth_password_recovery (T);
  case 0x0cc8bc16: return fetch_type_auth_sent_code (T);
  case 0xf33743e9: return fetch_type_bare_auth_sent_code (T);
  case 0x0377168f: return fetch_type_binlog_encr_key (T);
  case 0xfc88e970: return fetch_type_bare_binlog_encr_key (T);
  case 0x66ff03a6: return fetch_type_binlog_update (T);
  case 0x9900fc59: return fetch_type_bare_binlog_update (T);
  case 0x8c1b8fb4: return fetch_type_contacts_blocked (T);
  case 0x73e4704b: return fetch_type_bare_contacts_blocked (T);
  case 0xd8c02560: return fetch_type_contacts_contacts (T);
  case 0x273fda9f: return fetch_type_bare_contacts_contacts (T);
  case 0x0566000e: return fetch_type_contacts_found (T);
  case 0xfa99fff1: return fetch_type_bare_contacts_found (T);
  case 0xad524315: return fetch_type_contacts_imported_contacts (T);
  case 0x52adbcea: return fetch_type_bare_contacts_imported_contacts (T);
  case 0x3ace484c: return fetch_type_contacts_link (T);
  case 0xc531b7b3: return fetch_type_bare_contacts_link (T);
  case 0x5649dcc5: return fetch_type_contacts_suggested (T);
  case 0xa9b6233a: return fetch_type_bare_contacts_suggested (T);
  case 0x48feb267: return fetch_type_geochats_located (T);
  case 0xb7014d98: return fetch_type_bare_geochats_located (T);
  case 0x6d0a0e59: return fetch_type_geochats_messages (T);
  case 0x92f5f1a6: return fetch_type_bare_geochats_messages (T);
  case 0x17b1578b: return fetch_type_geochats_stated_message (T);
  case 0xe84ea874: return fetch_type_bare_geochats_stated_message (T);
  case 0x4ddd9627: return fetch_type_help_app_update (T);
  case 0xb22269d8: return fetch_type_bare_help_app_update (T);
  case 0x18cb9f78: return fetch_type_help_invite_text (T);
  case 0xe7346087: return fetch_type_bare_help_invite_text (T);
  case 0x17c6b5f6: return fetch_type_help_support (T);
  case 0xe8394a09: return fetch_type_bare_help_support (T);
  case 0xb45c69d1: return fetch_type_messages_affected_history (T);
  case 0x4ba3962e: return fetch_type_bare_messages_affected_history (T);
  case 0x84d19185: return fetch_type_messages_affected_messages (T);
  case 0x7b2e6e7a: return fetch_type_bare_messages_affected_messages (T);
  case 0xb485502f: return fetch_type_messages_all_stickers (T);
  case 0x4b7aafd0: return fetch_type_bare_messages_all_stickers (T);
  case 0xe5d7d19c: return fetch_type_messages_chat_full (T);
  case 0x1a282e63: return fetch_type_bare_messages_chat_full (T);
  case 0x64ff9fd5: return fetch_type_messages_chats (T);
  case 0x9b00602a: return fetch_type_bare_messages_chats (T);
  case 0xecc058e8: return fetch_type_messages_dh_config (T);
  case 0x133fa717: return fetch_type_bare_messages_dh_config (T);
  case 0x645af8b3: return fetch_type_messages_dialogs (T);
  case 0x9ba5074c: return fetch_type_bare_messages_dialogs (T);
  case 0x3f4e0648: return fetch_type_messages_message (T);
  case 0xc0b1f9b7: return fetch_type_bare_messages_message (T);
  case 0x8735e464: return fetch_type_messages_messages (T);
  case 0x78ca1b9b: return fetch_type_bare_messages_messages (T);
  case 0xc29c7607: return fetch_type_messages_sent_encrypted_message (T);
  case 0x3d6389f8: return fetch_type_bare_messages_sent_encrypted_message (T);
  case 0x799ce190: return fetch_type_messages_sent_message (T);
  case 0x86631e6f: return fetch_type_bare_messages_sent_message (T);
  case 0xb60a24a6: return fetch_type_messages_sticker_set (T);
  case 0x49f5db59: return fetch_type_bare_messages_sticker_set (T);
  case 0x7bfa5710: return fetch_type_messages_stickers (T);
  case 0x8405a8ef: return fetch_type_bare_messages_stickers (T);
  case 0x20212ca8: return fetch_type_photos_photo (T);
  case 0xdfded357: return fetch_type_bare_photos_photo (T);
  case 0x98cf75f1: return fetch_type_photos_photos (T);
  case 0x67308a0e: return fetch_type_bare_photos_photos (T);
  case 0x3e2838a8: return fetch_type_storage_file_type (T);
  case 0xc1d7c757: return fetch_type_bare_storage_file_type (T);
  case 0xf57a2419: return fetch_type_updates_difference (T);
  case 0x0a85dbe6: return fetch_type_bare_updates_difference (T);
  case 0xa56c2a3e: return fetch_type_updates_state (T);
  case 0x5a93d5c1: return fetch_type_bare_updates_state (T);
  case 0x096a18d5: return fetch_type_upload_file (T);
  case 0xf695e72a: return fetch_type_bare_upload_file (T);
  default: return -1; }
}
