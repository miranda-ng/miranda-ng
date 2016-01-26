#include "sender_key_record.h"

#include <string.h>

#include "axolotl_internal.h"
#include "sender_key_state.h"
#include "sender_key.h"
#include "utlist.h"
#include "LocalStorageProtocol.pb-c.h"

#define MAX_STATES 5

typedef struct sender_key_state_node {
    sender_key_state *state;
    struct sender_key_state_node *prev, *next;
} sender_key_state_node;

struct sender_key_record
{
    axolotl_type_base base;
    sender_key_state_node *sender_key_states_head;
    axolotl_context *global_context;
};

int sender_key_record_create(sender_key_record **record,
        axolotl_context *global_context)
{
    sender_key_record *result = malloc(sizeof(sender_key_record));
    if(!result) {
        return AX_ERR_NOMEM;
    }
    memset(result, 0, sizeof(sender_key_record));
    AXOLOTL_INIT(result, sender_key_record_destroy);

    result->global_context = global_context;

    *record = result;
    return 0;
}

int sender_key_record_serialize(axolotl_buffer **buffer, sender_key_record *record)
{
    int result = 0;
    size_t result_size = 0;
    unsigned int i = 0;
    Textsecure__SenderKeyRecordStructure record_structure = TEXTSECURE__SENDER_KEY_RECORD_STRUCTURE__INIT;
    sender_key_state_node *cur_node = 0;
    axolotl_buffer *result_buf = 0;
    uint8_t *data;
    size_t len;

    if(record->sender_key_states_head) {
        unsigned int count;
        DL_COUNT(record->sender_key_states_head, cur_node, count);

        if(count > SIZE_MAX / sizeof(Textsecure__SenderKeyStateStructure *)) {
            result = AX_ERR_NOMEM;
            goto complete;
        }

        record_structure.senderkeystates = malloc(sizeof(Textsecure__SenderKeyStateStructure *) * count);
        if(!record_structure.senderkeystates) {
            result = AX_ERR_NOMEM;
            goto complete;
        }

        i = 0;
        DL_FOREACH(record->sender_key_states_head, cur_node) {
            record_structure.senderkeystates[i] = malloc(sizeof(Textsecure__SenderKeyStateStructure));
            if(!record_structure.senderkeystates[i]) {
                result = AX_ERR_NOMEM;
                break;
            }
            textsecure__sender_key_state_structure__init(record_structure.senderkeystates[i]);

            result = sender_key_state_serialize_prepare(cur_node->state, record_structure.senderkeystates[i]);
            if(result < 0) {
                break;
            }
            i++;
        }
        record_structure.n_senderkeystates = i;
        if(result < 0) {
            goto complete;
        }
    }

    len = textsecure__sender_key_record_structure__get_packed_size(&record_structure);

    result_buf = axolotl_buffer_alloc(len);
    if(!result_buf) {
        result = AX_ERR_NOMEM;
        goto complete;
    }

    data = axolotl_buffer_data(result_buf);
    result_size = textsecure__sender_key_record_structure__pack(&record_structure, data);
    if(result_size != len) {
        axolotl_buffer_free(result_buf);
        result = AX_ERR_INVALID_PROTO_BUF;
        result_buf = 0;
        goto complete;
    }

complete:
    if(record_structure.senderkeystates) {
        for(i = 0; i < record_structure.n_senderkeystates; i++) {
            if(record_structure.senderkeystates[i]) {
                sender_key_state_serialize_prepare_free(record_structure.senderkeystates[i]);
            }
        }
        free(record_structure.senderkeystates);
    }

    if(result >= 0) {
        *buffer = result_buf;
    }
    return result;
}

int sender_key_record_deserialize(sender_key_record **record, const uint8_t *data, size_t len, axolotl_context *global_context)
{
    int result = 0;
    sender_key_record *result_record = 0;
    Textsecure__SenderKeyRecordStructure *record_structure = 0;

    record_structure = textsecure__sender_key_record_structure__unpack(0, len, data);
    if(!record_structure) {
        result = AX_ERR_INVALID_PROTO_BUF;
        goto complete;
    }

    result = sender_key_record_create(&result_record, global_context);
    if(result < 0) {
        goto complete;
    }

    if(record_structure->n_senderkeystates > 0) {
        unsigned int i;
        sender_key_state_node *state_node = 0;
        sender_key_state *state_element = 0;
        for(i = 0; i < record_structure->n_senderkeystates; i++) {
            result = sender_key_state_deserialize_protobuf(&state_element, record_structure->senderkeystates[i], global_context);
            if(result < 0) {
                goto complete;
            }

            state_node = malloc(sizeof(sender_key_state_node));
            if(!state_node) {
                result = AX_ERR_NOMEM;
                goto complete;
            }

            state_node->state = state_element;
            DL_APPEND(result_record->sender_key_states_head, state_node);
        }
    }

complete:
    if(record_structure) {
        textsecure__sender_key_record_structure__free_unpacked(record_structure, 0);
    }
    if(result_record) {
        if(result < 0) {
            AXOLOTL_UNREF(result_record);
        }
        else {
            *record = result_record;
        }
    }
    return result;
}

int sender_key_record_copy(sender_key_record **record, sender_key_record *other_record, axolotl_context *global_context)
{
    int result = 0;
    axolotl_buffer *buffer = 0;
    uint8_t *data;
    size_t len;

    assert(other_record);
    assert(global_context);

    result = sender_key_record_serialize(&buffer, other_record);
    if(result < 0) {
        goto complete;
    }

    data = axolotl_buffer_data(buffer);
    len = axolotl_buffer_len(buffer);

    result = sender_key_record_deserialize(record, data, len, global_context);
    if(result < 0) {
        goto complete;
    }

complete:
    if(buffer) {
        axolotl_buffer_free(buffer);
    }
    return result;
}

int sender_key_record_is_empty(sender_key_record *record)
{
    assert(record);
    if(record->sender_key_states_head) {
        return 0;
    }
    else {
        return 1;
    }
}

int sender_key_record_get_sender_key_state(sender_key_record *record, sender_key_state **state)
{
    assert(record);
    if(record->sender_key_states_head) {
        *state = record->sender_key_states_head->state;
        return 0;
    }
    else {
        axolotl_log(record->global_context, AX_LOG_ERROR, "No key state in record!");
        return AX_ERR_INVALID_KEY_ID;
    }
}

int sender_key_record_get_sender_key_state_by_id(sender_key_record *record, sender_key_state **state, uint32_t key_id)
{
    sender_key_state_node *cur_node;
    assert(record);

    DL_FOREACH(record->sender_key_states_head, cur_node) {
        if(sender_key_state_get_key_id(cur_node->state) == key_id) {
            *state = cur_node->state;
            return 0;
        }
    }

    axolotl_log(record->global_context, AX_LOG_ERROR, "No keys for: %d", key_id);
    return AX_ERR_INVALID_KEY_ID;
}

static int sender_key_record_add_sender_key_state_impl(sender_key_record *record,
        uint32_t id, uint32_t iteration, axolotl_buffer *chain_key,
        ec_public_key *signature_public_key, ec_private_key *signature_private_key)
{
    int result = 0;
    sender_chain_key *chain_key_element = 0;
    sender_key_state *state = 0;
    sender_key_state_node *state_node = 0;
    int count;
    assert(record);

    result = sender_chain_key_create(&chain_key_element, iteration, chain_key, record->global_context);
    if(result < 0) {
        goto complete;
    }

    result = sender_key_state_create(&state, id, chain_key_element,
            signature_public_key, signature_private_key,
            record->global_context);
    if(result < 0) {
        goto complete;
    }

    state_node = malloc(sizeof(sender_key_state_node));
    if(!state_node) {
        result = AX_ERR_NOMEM;
        goto complete;
    }

    state_node->state = state;
    DL_PREPEND(record->sender_key_states_head, state_node);

    DL_COUNT(record->sender_key_states_head, state_node, count);
    while(count > MAX_STATES) {
        state_node = record->sender_key_states_head->prev;
        DL_DELETE(record->sender_key_states_head, state_node);
        if(state_node->state) {
            AXOLOTL_UNREF(state_node->state);
        }
        free(state_node);
        --count;
    }

complete:
    AXOLOTL_UNREF(chain_key_element);
    if(result < 0) {
        AXOLOTL_UNREF(state);
    }
    return result;
}

int sender_key_record_add_sender_key_state(sender_key_record *record,
        uint32_t id, uint32_t iteration, axolotl_buffer *chain_key, ec_public_key *signature_key)
{
    int result = sender_key_record_add_sender_key_state_impl(
            record, id, iteration, chain_key, signature_key, 0);
    return result;
}

int sender_key_record_set_sender_key_state(sender_key_record *record,
        uint32_t id, uint32_t iteration, axolotl_buffer *chain_key, ec_key_pair *signature_key_pair)
{
    int result = 0;
    sender_key_state_node *cur_node;
    sender_key_state_node *tmp_node;
    assert(record);

    DL_FOREACH_SAFE(record->sender_key_states_head, cur_node, tmp_node) {
        DL_DELETE(record->sender_key_states_head, cur_node);
        if(cur_node->state) {
            AXOLOTL_UNREF(cur_node->state);
        }
        free(cur_node);
    }
    record->sender_key_states_head = 0;

    result = sender_key_record_add_sender_key_state_impl(
            record, id, iteration, chain_key,
            ec_key_pair_get_public(signature_key_pair),
            ec_key_pair_get_private(signature_key_pair));
    return result;
}

void sender_key_record_destroy(axolotl_type_base *type)
{
    sender_key_record *record = (sender_key_record *)type;
    sender_key_state_node *cur_node;
    sender_key_state_node *tmp_node;

    DL_FOREACH_SAFE(record->sender_key_states_head, cur_node, tmp_node) {
        DL_DELETE(record->sender_key_states_head, cur_node);
        if(cur_node->state) {
            AXOLOTL_UNREF(cur_node->state);
        }
        free(cur_node);
    }
    record->sender_key_states_head = 0;

    free(record);
}
