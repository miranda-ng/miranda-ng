/* Generated by the protocol buffer compiler.  DO NOT EDIT! */
/* Generated from: steammessages_clientserver_appinfo.proto */

#ifndef PROTOBUF_C_steammessages_5fclientserver_5fappinfo_2eproto__INCLUDED
#define PROTOBUF_C_steammessages_5fclientserver_5fappinfo_2eproto__INCLUDED

#include "protobuf-c.h"

PROTOBUF_C__BEGIN_DECLS

#if PROTOBUF_C_VERSION_NUMBER < 1000000
# error This file was generated by a newer version of protoc-c which is incompatible with your libprotobuf-c headers. Please update your headers.
#elif 1004001 < PROTOBUF_C_MIN_COMPILER_VERSION
# error This file was generated by an older version of protoc-c which is incompatible with your libprotobuf-c headers. Please regenerate this file with a newer version of protoc-c.
#endif

#include "steammessages_base.pb-c.h"

struct CMsgClientAppInfoUpdate;
struct CMsgClientAppInfoChanges;
struct CMsgClientAppInfoRequest;
struct CMsgClientAppInfoRequest__App;
struct CMsgClientPICSChangesSinceRequest;
struct CMsgClientPICSChangesSinceResponse;
struct CMsgClientPICSChangesSinceResponse__PackageChange;
struct CMsgClientPICSChangesSinceResponse__AppChange;
struct CMsgClientPICSProductInfoRequest;
struct CMsgClientPICSProductInfoRequest__AppInfo;
struct CMsgClientPICSProductInfoRequest__PackageInfo;
struct CMsgClientPICSProductInfoResponse;
struct CMsgClientPICSProductInfoResponse__AppInfo;
struct CMsgClientPICSProductInfoResponse__PackageInfo;
struct CMsgClientPICSAccessTokenRequest;
struct CMsgClientPICSAccessTokenResponse;
struct CMsgClientPICSAccessTokenResponse__PackageToken;
struct CMsgClientPICSAccessTokenResponse__AppToken;


/* --- enums --- */


/* --- descriptors --- */

extern const ProtobufCMessageDescriptor cmsg_client_app_info_update__descriptor;
extern const ProtobufCMessageDescriptor cmsg_client_app_info_changes__descriptor;
extern const ProtobufCMessageDescriptor cmsg_client_app_info_request__descriptor;
extern const ProtobufCMessageDescriptor cmsg_client_app_info_request__app__descriptor;
extern const ProtobufCMessageDescriptor cmsg_client_picschanges_since_request__descriptor;
extern const ProtobufCMessageDescriptor cmsg_client_picschanges_since_response__descriptor;
extern const ProtobufCMessageDescriptor cmsg_client_picschanges_since_response__package_change__descriptor;
extern const ProtobufCMessageDescriptor cmsg_client_picschanges_since_response__app_change__descriptor;
extern const ProtobufCMessageDescriptor cmsg_client_picsproduct_info_request__descriptor;
extern const ProtobufCMessageDescriptor cmsg_client_picsproduct_info_request__app_info__descriptor;
extern const ProtobufCMessageDescriptor cmsg_client_picsproduct_info_request__package_info__descriptor;
extern const ProtobufCMessageDescriptor cmsg_client_picsproduct_info_response__descriptor;
extern const ProtobufCMessageDescriptor cmsg_client_picsproduct_info_response__app_info__descriptor;
extern const ProtobufCMessageDescriptor cmsg_client_picsproduct_info_response__package_info__descriptor;
extern const ProtobufCMessageDescriptor cmsg_client_picsaccess_token_request__descriptor;
extern const ProtobufCMessageDescriptor cmsg_client_picsaccess_token_response__descriptor;
extern const ProtobufCMessageDescriptor cmsg_client_picsaccess_token_response__package_token__descriptor;
extern const ProtobufCMessageDescriptor cmsg_client_picsaccess_token_response__app_token__descriptor;

/* --- messages --- */

extern "C" void message_init_generic(const ProtobufCMessageDescriptor * desc, ProtobufCMessage * message);

struct CMsgClientAppInfoUpdate : public ProtobufCppMessage
{
  CMsgClientAppInfoUpdate() :
     ProtobufCppMessage(cmsg_client_app_info_update__descriptor)
  {}

  protobuf_c_boolean has_last_changenumber;
  uint32_t last_changenumber;
  protobuf_c_boolean has_send_changelist;
  protobuf_c_boolean send_changelist;
};

struct CMsgClientAppInfoChanges : public ProtobufCppMessage
{
  CMsgClientAppInfoChanges() :
     ProtobufCppMessage(cmsg_client_app_info_changes__descriptor)
  {}

  protobuf_c_boolean has_current_change_number;
  uint32_t current_change_number;
  protobuf_c_boolean has_force_full_update;
  protobuf_c_boolean force_full_update;
  size_t n_appids;
  uint32_t *appids;
};

struct CMsgClientAppInfoRequest__App : public ProtobufCppMessage
{
  CMsgClientAppInfoRequest__App() :
     ProtobufCppMessage(cmsg_client_app_info_request__app__descriptor)
  {}

  protobuf_c_boolean has_app_id;
  uint32_t app_id;
  protobuf_c_boolean has_section_flags;
  uint32_t section_flags;
  size_t n_section_crc;
  uint32_t *section_crc;
};

struct CMsgClientAppInfoRequest : public ProtobufCppMessage
{
  CMsgClientAppInfoRequest() :
     ProtobufCppMessage(cmsg_client_app_info_request__descriptor)
  {}

  size_t n_apps;
  CMsgClientAppInfoRequest__App **apps;
  protobuf_c_boolean has_supports_batches;
  protobuf_c_boolean supports_batches;
};

struct CMsgClientPICSChangesSinceRequest : public ProtobufCppMessage
{
  CMsgClientPICSChangesSinceRequest() :
     ProtobufCppMessage(cmsg_client_picschanges_since_request__descriptor)
  {}

  protobuf_c_boolean has_since_change_number;
  uint32_t since_change_number;
  protobuf_c_boolean has_send_app_info_changes;
  protobuf_c_boolean send_app_info_changes;
  protobuf_c_boolean has_send_package_info_changes;
  protobuf_c_boolean send_package_info_changes;
  protobuf_c_boolean has_num_app_info_cached;
  uint32_t num_app_info_cached;
  protobuf_c_boolean has_num_package_info_cached;
  uint32_t num_package_info_cached;
};

struct CMsgClientPICSChangesSinceResponse__PackageChange : public ProtobufCppMessage
{
  CMsgClientPICSChangesSinceResponse__PackageChange() :
     ProtobufCppMessage(cmsg_client_picschanges_since_response__package_change__descriptor)
  {}

  protobuf_c_boolean has_packageid;
  uint32_t packageid;
  protobuf_c_boolean has_change_number;
  uint32_t change_number;
  protobuf_c_boolean has_needs_token;
  protobuf_c_boolean needs_token;
};

struct CMsgClientPICSChangesSinceResponse__AppChange : public ProtobufCppMessage
{
  CMsgClientPICSChangesSinceResponse__AppChange() :
     ProtobufCppMessage(cmsg_client_picschanges_since_response__app_change__descriptor)
  {}

  protobuf_c_boolean has_appid;
  uint32_t appid;
  protobuf_c_boolean has_change_number;
  uint32_t change_number;
  protobuf_c_boolean has_needs_token;
  protobuf_c_boolean needs_token;
};

struct CMsgClientPICSChangesSinceResponse : public ProtobufCppMessage
{
  CMsgClientPICSChangesSinceResponse() :
     ProtobufCppMessage(cmsg_client_picschanges_since_response__descriptor)
  {}

  protobuf_c_boolean has_current_change_number;
  uint32_t current_change_number;
  protobuf_c_boolean has_since_change_number;
  uint32_t since_change_number;
  protobuf_c_boolean has_force_full_update;
  protobuf_c_boolean force_full_update;
  size_t n_package_changes;
  CMsgClientPICSChangesSinceResponse__PackageChange **package_changes;
  size_t n_app_changes;
  CMsgClientPICSChangesSinceResponse__AppChange **app_changes;
  protobuf_c_boolean has_force_full_app_update;
  protobuf_c_boolean force_full_app_update;
  protobuf_c_boolean has_force_full_package_update;
  protobuf_c_boolean force_full_package_update;
};

struct CMsgClientPICSProductInfoRequest__AppInfo : public ProtobufCppMessage
{
  CMsgClientPICSProductInfoRequest__AppInfo() :
     ProtobufCppMessage(cmsg_client_picsproduct_info_request__app_info__descriptor)
  {}

  protobuf_c_boolean has_appid;
  uint32_t appid;
  protobuf_c_boolean has_access_token;
  uint64_t access_token;
  protobuf_c_boolean has_only_public_obsolete;
  protobuf_c_boolean only_public_obsolete;
};

struct CMsgClientPICSProductInfoRequest__PackageInfo : public ProtobufCppMessage
{
  CMsgClientPICSProductInfoRequest__PackageInfo() :
     ProtobufCppMessage(cmsg_client_picsproduct_info_request__package_info__descriptor)
  {}

  protobuf_c_boolean has_packageid;
  uint32_t packageid;
  protobuf_c_boolean has_access_token;
  uint64_t access_token;
};

struct CMsgClientPICSProductInfoRequest : public ProtobufCppMessage
{
  CMsgClientPICSProductInfoRequest() :
     ProtobufCppMessage(cmsg_client_picsproduct_info_request__descriptor)
  {}

  size_t n_packages;
  CMsgClientPICSProductInfoRequest__PackageInfo **packages;
  size_t n_apps;
  CMsgClientPICSProductInfoRequest__AppInfo **apps;
  protobuf_c_boolean has_meta_data_only;
  protobuf_c_boolean meta_data_only;
  protobuf_c_boolean has_num_prev_failed;
  uint32_t num_prev_failed;
  protobuf_c_boolean has_obsolete_supports_package_tokens;
  uint32_t obsolete_supports_package_tokens;
  protobuf_c_boolean has_sequence_number;
  uint32_t sequence_number;
};

struct CMsgClientPICSProductInfoResponse__AppInfo : public ProtobufCppMessage
{
  CMsgClientPICSProductInfoResponse__AppInfo() :
     ProtobufCppMessage(cmsg_client_picsproduct_info_response__app_info__descriptor)
  {}

  protobuf_c_boolean has_appid;
  uint32_t appid;
  protobuf_c_boolean has_change_number;
  uint32_t change_number;
  protobuf_c_boolean has_missing_token;
  protobuf_c_boolean missing_token;
  protobuf_c_boolean has_sha;
  ProtobufCBinaryData sha;
  protobuf_c_boolean has_buffer;
  ProtobufCBinaryData buffer;
  protobuf_c_boolean has_only_public;
  protobuf_c_boolean only_public;
  protobuf_c_boolean has_size;
  uint32_t size;
};

struct CMsgClientPICSProductInfoResponse__PackageInfo : public ProtobufCppMessage
{
  CMsgClientPICSProductInfoResponse__PackageInfo() :
     ProtobufCppMessage(cmsg_client_picsproduct_info_response__package_info__descriptor)
  {}

  protobuf_c_boolean has_packageid;
  uint32_t packageid;
  protobuf_c_boolean has_change_number;
  uint32_t change_number;
  protobuf_c_boolean has_missing_token;
  protobuf_c_boolean missing_token;
  protobuf_c_boolean has_sha;
  ProtobufCBinaryData sha;
  protobuf_c_boolean has_buffer;
  ProtobufCBinaryData buffer;
  protobuf_c_boolean has_size;
  uint32_t size;
};

struct CMsgClientPICSProductInfoResponse : public ProtobufCppMessage
{
  CMsgClientPICSProductInfoResponse() :
     ProtobufCppMessage(cmsg_client_picsproduct_info_response__descriptor)
  {}

  size_t n_apps;
  CMsgClientPICSProductInfoResponse__AppInfo **apps;
  size_t n_unknown_appids;
  uint32_t *unknown_appids;
  size_t n_packages;
  CMsgClientPICSProductInfoResponse__PackageInfo **packages;
  size_t n_unknown_packageids;
  uint32_t *unknown_packageids;
  protobuf_c_boolean has_meta_data_only;
  protobuf_c_boolean meta_data_only;
  protobuf_c_boolean has_response_pending;
  protobuf_c_boolean response_pending;
  protobuf_c_boolean has_http_min_size;
  uint32_t http_min_size;
  char *http_host;
};

struct CMsgClientPICSAccessTokenRequest : public ProtobufCppMessage
{
  CMsgClientPICSAccessTokenRequest() :
     ProtobufCppMessage(cmsg_client_picsaccess_token_request__descriptor)
  {}

  size_t n_packageids;
  uint32_t *packageids;
  size_t n_appids;
  uint32_t *appids;
};

struct CMsgClientPICSAccessTokenResponse__PackageToken : public ProtobufCppMessage
{
  CMsgClientPICSAccessTokenResponse__PackageToken() :
     ProtobufCppMessage(cmsg_client_picsaccess_token_response__package_token__descriptor)
  {}

  protobuf_c_boolean has_packageid;
  uint32_t packageid;
  protobuf_c_boolean has_access_token;
  uint64_t access_token;
};

struct CMsgClientPICSAccessTokenResponse__AppToken : public ProtobufCppMessage
{
  CMsgClientPICSAccessTokenResponse__AppToken() :
     ProtobufCppMessage(cmsg_client_picsaccess_token_response__app_token__descriptor)
  {}

  protobuf_c_boolean has_appid;
  uint32_t appid;
  protobuf_c_boolean has_access_token;
  uint64_t access_token;
};

struct CMsgClientPICSAccessTokenResponse : public ProtobufCppMessage
{
  CMsgClientPICSAccessTokenResponse() :
     ProtobufCppMessage(cmsg_client_picsaccess_token_response__descriptor)
  {}

  size_t n_package_access_tokens;
  CMsgClientPICSAccessTokenResponse__PackageToken **package_access_tokens;
  size_t n_package_denied_tokens;
  uint32_t *package_denied_tokens;
  size_t n_app_access_tokens;
  CMsgClientPICSAccessTokenResponse__AppToken **app_access_tokens;
  size_t n_app_denied_tokens;
  uint32_t *app_denied_tokens;
};

size_t cmsg_client_app_info_update__get_packed_size
                     (const CMsgClientAppInfoUpdate   *message);
size_t cmsg_client_app_info_update__pack
                     (const CMsgClientAppInfoUpdate   *message,
                      uint8_t             *out);
size_t cmsg_client_app_info_update__pack_to_buffer
                     (const CMsgClientAppInfoUpdate   *message,
                      ProtobufCBuffer     *buffer);
CMsgClientAppInfoUpdate *
       cmsg_client_app_info_update__unpack
                     (ProtobufCAllocator  *allocator,
                      size_t               len,
                      const uint8_t       *data);
void   cmsg_client_app_info_update__free_unpacked
                     (CMsgClientAppInfoUpdate *message,
                      ProtobufCAllocator *allocator);
size_t cmsg_client_app_info_changes__get_packed_size
                     (const CMsgClientAppInfoChanges   *message);
size_t cmsg_client_app_info_changes__pack
                     (const CMsgClientAppInfoChanges   *message,
                      uint8_t             *out);
size_t cmsg_client_app_info_changes__pack_to_buffer
                     (const CMsgClientAppInfoChanges   *message,
                      ProtobufCBuffer     *buffer);
CMsgClientAppInfoChanges *
       cmsg_client_app_info_changes__unpack
                     (ProtobufCAllocator  *allocator,
                      size_t               len,
                      const uint8_t       *data);
void   cmsg_client_app_info_changes__free_unpacked
                     (CMsgClientAppInfoChanges *message,
                      ProtobufCAllocator *allocator);
size_t cmsg_client_app_info_request__get_packed_size
                     (const CMsgClientAppInfoRequest   *message);
size_t cmsg_client_app_info_request__pack
                     (const CMsgClientAppInfoRequest   *message,
                      uint8_t             *out);
size_t cmsg_client_app_info_request__pack_to_buffer
                     (const CMsgClientAppInfoRequest   *message,
                      ProtobufCBuffer     *buffer);
CMsgClientAppInfoRequest *
       cmsg_client_app_info_request__unpack
                     (ProtobufCAllocator  *allocator,
                      size_t               len,
                      const uint8_t       *data);
void   cmsg_client_app_info_request__free_unpacked
                     (CMsgClientAppInfoRequest *message,
                      ProtobufCAllocator *allocator);
size_t cmsg_client_picschanges_since_request__get_packed_size
                     (const CMsgClientPICSChangesSinceRequest   *message);
size_t cmsg_client_picschanges_since_request__pack
                     (const CMsgClientPICSChangesSinceRequest   *message,
                      uint8_t             *out);
size_t cmsg_client_picschanges_since_request__pack_to_buffer
                     (const CMsgClientPICSChangesSinceRequest   *message,
                      ProtobufCBuffer     *buffer);
CMsgClientPICSChangesSinceRequest *
       cmsg_client_picschanges_since_request__unpack
                     (ProtobufCAllocator  *allocator,
                      size_t               len,
                      const uint8_t       *data);
void   cmsg_client_picschanges_since_request__free_unpacked
                     (CMsgClientPICSChangesSinceRequest *message,
                      ProtobufCAllocator *allocator);
size_t cmsg_client_picschanges_since_response__get_packed_size
                     (const CMsgClientPICSChangesSinceResponse   *message);
size_t cmsg_client_picschanges_since_response__pack
                     (const CMsgClientPICSChangesSinceResponse   *message,
                      uint8_t             *out);
size_t cmsg_client_picschanges_since_response__pack_to_buffer
                     (const CMsgClientPICSChangesSinceResponse   *message,
                      ProtobufCBuffer     *buffer);
CMsgClientPICSChangesSinceResponse *
       cmsg_client_picschanges_since_response__unpack
                     (ProtobufCAllocator  *allocator,
                      size_t               len,
                      const uint8_t       *data);
void   cmsg_client_picschanges_since_response__free_unpacked
                     (CMsgClientPICSChangesSinceResponse *message,
                      ProtobufCAllocator *allocator);
size_t cmsg_client_picsproduct_info_request__get_packed_size
                     (const CMsgClientPICSProductInfoRequest   *message);
size_t cmsg_client_picsproduct_info_request__pack
                     (const CMsgClientPICSProductInfoRequest   *message,
                      uint8_t             *out);
size_t cmsg_client_picsproduct_info_request__pack_to_buffer
                     (const CMsgClientPICSProductInfoRequest   *message,
                      ProtobufCBuffer     *buffer);
CMsgClientPICSProductInfoRequest *
       cmsg_client_picsproduct_info_request__unpack
                     (ProtobufCAllocator  *allocator,
                      size_t               len,
                      const uint8_t       *data);
void   cmsg_client_picsproduct_info_request__free_unpacked
                     (CMsgClientPICSProductInfoRequest *message,
                      ProtobufCAllocator *allocator);
size_t cmsg_client_picsproduct_info_response__get_packed_size
                     (const CMsgClientPICSProductInfoResponse   *message);
size_t cmsg_client_picsproduct_info_response__pack
                     (const CMsgClientPICSProductInfoResponse   *message,
                      uint8_t             *out);
size_t cmsg_client_picsproduct_info_response__pack_to_buffer
                     (const CMsgClientPICSProductInfoResponse   *message,
                      ProtobufCBuffer     *buffer);
CMsgClientPICSProductInfoResponse *
       cmsg_client_picsproduct_info_response__unpack
                     (ProtobufCAllocator  *allocator,
                      size_t               len,
                      const uint8_t       *data);
void   cmsg_client_picsproduct_info_response__free_unpacked
                     (CMsgClientPICSProductInfoResponse *message,
                      ProtobufCAllocator *allocator);
size_t cmsg_client_picsaccess_token_request__get_packed_size
                     (const CMsgClientPICSAccessTokenRequest   *message);
size_t cmsg_client_picsaccess_token_request__pack
                     (const CMsgClientPICSAccessTokenRequest   *message,
                      uint8_t             *out);
size_t cmsg_client_picsaccess_token_request__pack_to_buffer
                     (const CMsgClientPICSAccessTokenRequest   *message,
                      ProtobufCBuffer     *buffer);
CMsgClientPICSAccessTokenRequest *
       cmsg_client_picsaccess_token_request__unpack
                     (ProtobufCAllocator  *allocator,
                      size_t               len,
                      const uint8_t       *data);
void   cmsg_client_picsaccess_token_request__free_unpacked
                     (CMsgClientPICSAccessTokenRequest *message,
                      ProtobufCAllocator *allocator);
size_t cmsg_client_picsaccess_token_response__get_packed_size
                     (const CMsgClientPICSAccessTokenResponse   *message);
size_t cmsg_client_picsaccess_token_response__pack
                     (const CMsgClientPICSAccessTokenResponse   *message,
                      uint8_t             *out);
size_t cmsg_client_picsaccess_token_response__pack_to_buffer
                     (const CMsgClientPICSAccessTokenResponse   *message,
                      ProtobufCBuffer     *buffer);
CMsgClientPICSAccessTokenResponse *
       cmsg_client_picsaccess_token_response__unpack
                     (ProtobufCAllocator  *allocator,
                      size_t               len,
                      const uint8_t       *data);
void   cmsg_client_picsaccess_token_response__free_unpacked
                     (CMsgClientPICSAccessTokenResponse *message,
                      ProtobufCAllocator *allocator);
/* --- per-message closures --- */

typedef void (*CMsgClientAppInfoUpdate_Closure)
                 (const CMsgClientAppInfoUpdate *message,
                  void *closure_data);
typedef void (*CMsgClientAppInfoChanges_Closure)
                 (const CMsgClientAppInfoChanges *message,
                  void *closure_data);
typedef void (*CMsgClientAppInfoRequest__App_Closure)
                 (const CMsgClientAppInfoRequest__App *message,
                  void *closure_data);
typedef void (*CMsgClientAppInfoRequest_Closure)
                 (const CMsgClientAppInfoRequest *message,
                  void *closure_data);
typedef void (*CMsgClientPICSChangesSinceRequest_Closure)
                 (const CMsgClientPICSChangesSinceRequest *message,
                  void *closure_data);
typedef void (*CMsgClientPICSChangesSinceResponse__PackageChange_Closure)
                 (const CMsgClientPICSChangesSinceResponse__PackageChange *message,
                  void *closure_data);
typedef void (*CMsgClientPICSChangesSinceResponse__AppChange_Closure)
                 (const CMsgClientPICSChangesSinceResponse__AppChange *message,
                  void *closure_data);
typedef void (*CMsgClientPICSChangesSinceResponse_Closure)
                 (const CMsgClientPICSChangesSinceResponse *message,
                  void *closure_data);
typedef void (*CMsgClientPICSProductInfoRequest__AppInfo_Closure)
                 (const CMsgClientPICSProductInfoRequest__AppInfo *message,
                  void *closure_data);
typedef void (*CMsgClientPICSProductInfoRequest__PackageInfo_Closure)
                 (const CMsgClientPICSProductInfoRequest__PackageInfo *message,
                  void *closure_data);
typedef void (*CMsgClientPICSProductInfoRequest_Closure)
                 (const CMsgClientPICSProductInfoRequest *message,
                  void *closure_data);
typedef void (*CMsgClientPICSProductInfoResponse__AppInfo_Closure)
                 (const CMsgClientPICSProductInfoResponse__AppInfo *message,
                  void *closure_data);
typedef void (*CMsgClientPICSProductInfoResponse__PackageInfo_Closure)
                 (const CMsgClientPICSProductInfoResponse__PackageInfo *message,
                  void *closure_data);
typedef void (*CMsgClientPICSProductInfoResponse_Closure)
                 (const CMsgClientPICSProductInfoResponse *message,
                  void *closure_data);
typedef void (*CMsgClientPICSAccessTokenRequest_Closure)
                 (const CMsgClientPICSAccessTokenRequest *message,
                  void *closure_data);
typedef void (*CMsgClientPICSAccessTokenResponse__PackageToken_Closure)
                 (const CMsgClientPICSAccessTokenResponse__PackageToken *message,
                  void *closure_data);
typedef void (*CMsgClientPICSAccessTokenResponse__AppToken_Closure)
                 (const CMsgClientPICSAccessTokenResponse__AppToken *message,
                  void *closure_data);
typedef void (*CMsgClientPICSAccessTokenResponse_Closure)
                 (const CMsgClientPICSAccessTokenResponse *message,
                  void *closure_data);

/* --- services --- */


PROTOBUF_C__END_DECLS


#endif  /* PROTOBUF_C_steammessages_5fclientserver_5fappinfo_2eproto__INCLUDED */
