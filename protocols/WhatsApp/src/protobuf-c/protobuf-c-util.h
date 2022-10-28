#ifndef PROTOBUF_C_UTIL_H
#define PROTOBUF_C_UTIL_H

/** \file
 * Internal utility header file.
 * Macros used by the generator and parser parts of the library.
 *
 * \author Kevin Lyda <kevin@ie.suberic.net>
 * \date   March 2014
 */

/** \defgroup internal Internal API
 *
 * These are the functions and data structures used internally.  They are
 * not exported and are not useable by users of \c libprotobuf-c-text.
 */

/* These are lifted from the protobuf-c lib */

/** Used to define STRUCT_MEMBER() and STRUCT_MEMBER_PTR(). */
#define STRUCT_MEMBER_P(struct_p, struct_offset) \
      ((void *) ((uint8_t *) (struct_p) + (struct_offset)))

/** Return a field from a message based on offset and type. */
#define STRUCT_MEMBER(member_type, struct_p, struct_offset) \
      (*(member_type *) STRUCT_MEMBER_P((struct_p), (struct_offset)))

/** Return a pointer to a field in a message based on offset and type. */
#define STRUCT_MEMBER_PTR(member_type, struct_p, struct_offset) \
      ((member_type *) STRUCT_MEMBER_P((struct_p), (struct_offset)))

/** Free possibly using the pbc allocator. */
#define PBC_FREE(ptr) (allocator? allocator->free(allocator->allocator_data, \
                                                  ptr): free(ptr))

/** Allocate possibly using the pbc allocator. */
#define PBC_ALLOC(size) (allocator? \
                           allocator->alloc(allocator->allocator_data, size): \
                           malloc(size))

/** Free possibly using the pbc allocator (state members). */
#define ST_FREE(ptr) (state->allocator? \
                        state->allocator->free(state->allocator-> \
                                               allocator_data, ptr): \
                        free(ptr))

/** Allocate possibly using the pbc allocator (state members). */
#define ST_ALLOC(size) (state->allocator? \
                           state->allocator->alloc(state->allocator-> \
                                                   allocator_data, size): \
                           malloc(size))

#endif /* PROTOBUF_C_UTIL_H */
