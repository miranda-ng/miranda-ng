#include "group_session_builder.h"

#include <assert.h>
#include <string.h>
#include "axolotl_internal.h"
#include "sender_key_record.h"
#include "sender_key_state.h"
#include "sender_key.h"
#include "protocol.h"
#include "key_helper.h"

struct group_session_builder
{
    axolotl_store_context *store;
    axolotl_context *global_context;
};

int group_session_builder_create(group_session_builder **builder,
        axolotl_store_context *store, axolotl_context *global_context)
{
    group_session_builder *result = 0;

    assert(store);
    assert(global_context);

    result = malloc(sizeof(group_session_builder));
    if(!result) {
        return AX_ERR_NOMEM;
    }
    memset(result, 0, sizeof(group_session_builder));

    result->store = store;
    result->global_context = global_context;

    *builder = result;
    return 0;
}

int group_session_builder_process_session(group_session_builder *builder,
        const axolotl_sender_key_name *sender_key_name,
        sender_key_distribution_message *distribution_message)
{
    int result = 0;
    sender_key_record *record = 0;

    assert(builder);
    assert(builder->store);
    axolotl_lock(builder->global_context);

    result = axolotl_sender_key_load_key(builder->store, &record, sender_key_name);
    if(result < 0) {
        goto complete;
    }

    result = sender_key_record_add_sender_key_state(record,
            sender_key_distribution_message_get_id(distribution_message),
            sender_key_distribution_message_get_iteration(distribution_message),
            sender_key_distribution_message_get_chain_key(distribution_message),
            sender_key_distribution_message_get_signature_key(distribution_message));
    if(result < 0) {
        goto complete;
    }

    result = axolotl_sender_key_store_key(builder->store, sender_key_name, record);

complete:
    AXOLOTL_UNREF(record);
    axolotl_unlock(builder->global_context);
    return result;
}

int group_session_builder_create_session(group_session_builder *builder,
        sender_key_distribution_message **distribution_message,
        const axolotl_sender_key_name *sender_key_name)
{
    int result = 0;
    sender_key_record *record = 0;
    sender_key_state *state = 0;
    uint32_t sender_key_id = 0;
    axolotl_buffer *sender_key = 0;
    ec_key_pair *sender_signing_key = 0;
    sender_chain_key *chain_key = 0;
    axolotl_buffer *seed = 0;

    assert(builder);
    assert(builder->store);
    axolotl_lock(builder->global_context);

    result = axolotl_sender_key_load_key(builder->store, &record, sender_key_name);
    if(result < 0) {
        goto complete;
    }

    if(sender_key_record_is_empty(record)) {
        result = axolotl_key_helper_generate_sender_key_id(&sender_key_id, builder->global_context);
        if(result < 0) {
            goto complete;
        }

        result = axolotl_key_helper_generate_sender_key(&sender_key, builder->global_context);
        if(result < 0) {
            goto complete;
        }

        result = axolotl_key_helper_generate_sender_signing_key(&sender_signing_key, builder->global_context);
        if(result < 0) {
            goto complete;
        }

        result = sender_key_record_set_sender_key_state(record, sender_key_id, 0, sender_key, sender_signing_key);
        if(result < 0) {
            goto complete;
        }

        result = axolotl_sender_key_store_key(builder->store, sender_key_name, record);
        if(result < 0) {
            goto complete;
        }
    }

    result = sender_key_record_get_sender_key_state(record, &state);
    if(result < 0) {
        goto complete;
    }

    chain_key = sender_key_state_get_chain_key(state);
    seed = sender_chain_key_get_seed(chain_key);

    result = sender_key_distribution_message_create(distribution_message,
            sender_key_state_get_key_id(state),
            sender_chain_key_get_iteration(chain_key),
            axolotl_buffer_data(seed), axolotl_buffer_len(seed),
            sender_key_state_get_signing_key_public(state),
            builder->global_context);

complete:
    axolotl_buffer_free(sender_key);
    AXOLOTL_UNREF(sender_signing_key);
    AXOLOTL_UNREF(record);
    axolotl_unlock(builder->global_context);
    return result;
}

void group_session_builder_free(group_session_builder *builder)
{
    if(builder) {
        free(builder);
    }
}
