#ifndef SENDER_KEY
#define SENDER_KEY

#include <stdint.h>
#include <stddef.h>
#include "axolotl_types.h"

#ifdef __cplusplus
extern "C" {
#endif

int sender_message_key_create(sender_message_key **key,
        uint32_t iteration, axolotl_buffer *seed,
        axolotl_context *global_context);
uint32_t sender_message_key_get_iteration(sender_message_key *key);
axolotl_buffer *sender_message_key_get_iv(sender_message_key *key);
axolotl_buffer *sender_message_key_get_cipher_key(sender_message_key *key);
axolotl_buffer *sender_message_key_get_seed(sender_message_key *key);
void sender_message_key_destroy(axolotl_type_base *type);

int sender_chain_key_create(sender_chain_key **key,
        uint32_t iteration, axolotl_buffer *chain_key,
        axolotl_context *global_context);
uint32_t sender_chain_key_get_iteration(sender_chain_key *key);
int sender_chain_key_create_message_key(sender_chain_key *key, sender_message_key **message_key);
int sender_chain_key_create_next(sender_chain_key *key, sender_chain_key **next_key);
axolotl_buffer *sender_chain_key_get_seed(sender_chain_key *key);
void sender_chain_key_destroy(axolotl_type_base *type);

#ifdef __cplusplus
}
#endif

#endif /* SENDER_KEY */
