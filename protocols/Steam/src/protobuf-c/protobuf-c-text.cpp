/** \file
 * Routines to generate text format protobufs.
 *
 * This file contains the internal support functions as well as the
 * exported functions which are used to generate text format protobufs
 * from C protobuf data types.
 *
 * \author Kevin Lyda <kevin@ie.suberic.net>
 * \date   March 2014
 */

#include "../stdafx.h"

#include <sys/types.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "protobuf-c.h"
#include "protobuf-c-util.h"

 /** Append a string to the ReturnString.
  *
  * Append the string built from \c format and its args to the \c rs
  * string. Note that \c malloc_err is checked and if it's true,
  * this function won't do anything.
  *
  * \param[in,out] rs The string to append to.
  * \param[in] guess A guess at the number of chars being added.
  * \param[in] allocator allocator functions.
  * \param[in] format Printf-style format string.
  * \param[in] ... Variable number of args for \c format.
  */

  /** \defgroup generate Functions to generate text format proto bufs
	* \ingroup internal
	* @{
	*/

	/** Escape string.
	 *
	 * Add escape characters to strings for problematic characters.
	 *
	 * \param[in] src The unescaped string to process.
	 * \param[in] len Length of \c src. Note that \c src might have ASCII
	 *                \c NULs so strlen() isn't good enough here.
	 * \param[in] allocator allocator functions.
	 * \return The fully escaped string, or \c NULL if there has been an
	 *         allocation error.
	 */
static char *
esc_str(const char *src, size_t len)
{
	size_t escapes = 0, dst_len = 0;

	for (size_t i = 0; i < len; i++) {
		if (!isprint(src[i])) {
			escapes++;
		}
	}

	char *dst = (char *)malloc((escapes * 4) + ((len - escapes) * 2) + 1);
	if (!dst) {
		return NULL;
	}

	for (size_t i = 0; i < len; i++) {
		switch (src[i]) {
			/* Special cases. */
		case '\'':
			dst[dst_len++] = '\\';
			dst[dst_len++] = '\'';
			break;
		case '\"':
			dst[dst_len++] = '\\';
			dst[dst_len++] = '\"';
			break;
		case '\\':
			dst[dst_len++] = '\\';
			dst[dst_len++] = '\\';
			break;
		case '\n':
			dst[dst_len++] = '\\';
			dst[dst_len++] = 'n';
			break;
		case '\r':
			dst[dst_len++] = '\\';
			dst[dst_len++] = 'r';
			break;
		case '\t':
			dst[dst_len++] = '\\';
			dst[dst_len++] = 't';
			break;

			/* Escape with octal if !isprint. */
		default:
			if (!isprint(src[i])) {
				dst_len += sprintf(dst + dst_len, "\\%03o", src[i]);
			}
			else {
				dst[dst_len++] = src[i];
			}
			break;
		}
	}
	dst[dst_len] = '\0';

	return dst;
}

/** Internal function to back API function.
 *
 * Has a few extra params to better enable recursion.  This function gets
 * called for each nested message as the \c ProtobufCMessage struct is
 * traversed.
 *
 * \param[in,out] rs The string being built up for the text format protobuf.
 * \param[in] level Indent level - increments in 2's.
 * \param[in] m The \c ProtobufCMessage being serialised.
 * \param[in] d The descriptor for the \c ProtobufCMessage.
 * \param[in] allocator allocator functions.
 */
static void protobuf_c_text_to_string_internal(
	CMStringA &str,
	int level,
	const ProtobufCMessage *m,
	const ProtobufCMessageDescriptor *d)
{
	size_t j, quantifier_offset;
	double float_var;
	const ProtobufCFieldDescriptor *f;
	ProtobufCEnumDescriptor *enumd;
	const ProtobufCEnumValue *enumv;

	f = d->fields;
	for (unsigned i = 0; i < d->n_fields; i++) {
		/* Decide if something needs to be done for this field. */
		switch (f[i].label) {
		case PROTOBUF_C_LABEL_OPTIONAL:
			if (f[i].type == PROTOBUF_C_TYPE_STRING) {
				if (!STRUCT_MEMBER(char *, m, f[i].offset)
					|| (STRUCT_MEMBER(char *, m, f[i].offset)
						== (char *)f[i].default_value)) {
					continue;
				}
			}
			else if (f[i].type == PROTOBUF_C_TYPE_MESSAGE) {
				if (!STRUCT_MEMBER(char *, m, f[i].offset)) {
					continue;
				}
			}
			else {
				if (!STRUCT_MEMBER(protobuf_c_boolean, m, f[i].quantifier_offset)) {
					continue;
				}
			}
			break;
		case PROTOBUF_C_LABEL_REPEATED:
			if (!STRUCT_MEMBER(size_t, m, f[i].quantifier_offset)) {
				continue;
			}
			break;
		}

		quantifier_offset = STRUCT_MEMBER(size_t, m, f[i].quantifier_offset);
		/* Field exists and has data, dump it. */
		switch (f[i].type) {
		case PROTOBUF_C_TYPE_INT32:
		case PROTOBUF_C_TYPE_UINT32:
		case PROTOBUF_C_TYPE_FIXED32:
			if (f[i].label == PROTOBUF_C_LABEL_REPEATED) {
				for (j = 0; j < quantifier_offset; j++) {
					str.AppendFormat(
						"%*s%s: %u\n",
						level, "", f[i].name,
						STRUCT_MEMBER(uint32_t *, m, f[i].offset)[j]);
				}
			}
			else {
				str.AppendFormat(
					"%*s%s: %u\n",
					level, "", f[i].name,
					STRUCT_MEMBER(uint32_t, m, f[i].offset));
			}
			break;
		case PROTOBUF_C_TYPE_SINT32:
		case PROTOBUF_C_TYPE_SFIXED32:
			if (f[i].label == PROTOBUF_C_LABEL_REPEATED) {
				for (j = 0; j < quantifier_offset; j++) {
					str.AppendFormat(
						"%*s%s: %d\n",
						level, "", f[i].name,
						STRUCT_MEMBER(int32_t *, m, f[i].offset)[j]);
				}
			}
			else {
				str.AppendFormat(
					"%*s%s: %d\n",
					level, "", f[i].name,
					STRUCT_MEMBER(int32_t, m, f[i].offset));
			}
			break;
		case PROTOBUF_C_TYPE_INT64:
		case PROTOBUF_C_TYPE_UINT64:
		case PROTOBUF_C_TYPE_FIXED64:
			if (f[i].label == PROTOBUF_C_LABEL_REPEATED) {
				for (j = 0; j < quantifier_offset; j++) {
					str.AppendFormat(
						"%*s%s: %lu\n",
						level, "", f[i].name,
						STRUCT_MEMBER(uint64_t *, m, f[i].offset)[j]);
				}
			}
			else {
				str.AppendFormat(
					"%*s%s: %lu\n",
					level, "", f[i].name,
					STRUCT_MEMBER(uint64_t, m, f[i].offset));
			}
			break;
		case PROTOBUF_C_TYPE_SINT64:
		case PROTOBUF_C_TYPE_SFIXED64:
			if (f[i].label == PROTOBUF_C_LABEL_REPEATED) {
				for (j = 0; j < quantifier_offset; j++) {
					str.AppendFormat(
						"%*s%s: %ld\n",
						level, "", f[i].name,
						STRUCT_MEMBER(int64_t *, m, f[i].offset)[j]);
				}
			}
			else {
				str.AppendFormat(
					"%*s%s: %ld\n",
					level, "", f[i].name,
					STRUCT_MEMBER(int64_t, m, f[i].offset));
			}
			break;
		case PROTOBUF_C_TYPE_FLOAT:
			if (f[i].label == PROTOBUF_C_LABEL_REPEATED) {
				for (j = 0; j < quantifier_offset; j++) {
					float_var = STRUCT_MEMBER(float *, m, f[i].offset)[j];
					str.AppendFormat(
						"%*s%s: %g\n",
						level, "", f[i].name,
						float_var);
				}
			}
			else {
				float_var = STRUCT_MEMBER(float, m, f[i].offset);
				str.AppendFormat(
					"%*s%s: %g\n",
					level, "", f[i].name,
					float_var);
			}
			break;
		case PROTOBUF_C_TYPE_DOUBLE:
			if (f[i].label == PROTOBUF_C_LABEL_REPEATED) {
				for (j = 0; j < quantifier_offset; j++) {
					str.AppendFormat(
						"%*s%s: %g\n",
						level, "", f[i].name,
						STRUCT_MEMBER(double *, m, f[i].offset)[j]);
				}
			}
			else {
				str.AppendFormat(
					"%*s%s: %g\n",
					level, "", f[i].name,
					STRUCT_MEMBER(double, m, f[i].offset));
			}
			break;
		case PROTOBUF_C_TYPE_BOOL:
			if (f[i].label == PROTOBUF_C_LABEL_REPEATED) {
				for (j = 0; j < quantifier_offset; j++) {
					str.AppendFormat(
						"%*s%s: %s\n",
						level, "", f[i].name,
						STRUCT_MEMBER(protobuf_c_boolean *, m, f[i].offset)[j] ?
						"true" : "false");
				}
			}
			else {
				str.AppendFormat(
					"%*s%s: %s\n",
					level, "", f[i].name,
					STRUCT_MEMBER(protobuf_c_boolean, m, f[i].offset) ?
					"true" : "false");
			}
			break;
		case PROTOBUF_C_TYPE_ENUM:
			enumd = (ProtobufCEnumDescriptor *)f[i].descriptor;
			if (f[i].label == PROTOBUF_C_LABEL_REPEATED) {
				for (j = 0; j < quantifier_offset; j++) {
					enumv = protobuf_c_enum_descriptor_get_value(
						enumd, STRUCT_MEMBER(int *, m, f[i].offset)[j]);
					str.AppendFormat(
						"%*s%s: %s\n",
						level, "", f[i].name,
						enumv ? enumv->name : "unknown");
				}
			}
			else {
				enumv = protobuf_c_enum_descriptor_get_value(
					enumd, STRUCT_MEMBER(int, m, f[i].offset));
				str.AppendFormat(
					"%*s%s: %s\n",
					level, "", f[i].name,
					enumv ? enumv->name : "unknown");
			}
			break;
		case PROTOBUF_C_TYPE_STRING:
			if (f[i].label == PROTOBUF_C_LABEL_REPEATED) {
				for (j = 0; j < quantifier_offset; j++) {
					char *escaped = esc_str(
						STRUCT_MEMBER(char **, m, f[i].offset)[j],
						strlen(STRUCT_MEMBER(char **, m, f[i].offset)[j]));
					str.AppendFormat("%*s%s: \"%s\"\n", level, "", f[i].name, escaped);
					free(escaped);
				}
			}
			else {
				char *escaped = esc_str(STRUCT_MEMBER(char *, m, f[i].offset),
					strlen(STRUCT_MEMBER(char *, m, f[i].offset)));
				str.AppendFormat("%*s%s: \"%s\"\n", level, "", f[i].name, escaped);
				free(escaped);
			}
			break;
		case PROTOBUF_C_TYPE_BYTES:
			if (f[i].label == PROTOBUF_C_LABEL_REPEATED) {
				for (j = 0; j < quantifier_offset; j++) {
					auto *member = STRUCT_MEMBER(ProtobufCBinaryData *, m, f[i].offset);
					char *p = (char *)malloc(member->len * 2 + 1);
					bin2hex(member->data, member->len, p);
					str.AppendFormat("%*s%s: \"%s\"\n", level, "", f[i].name, p);
					free(p);
				}
			}
			else {
				auto member = STRUCT_MEMBER(ProtobufCBinaryData, m, f[i].offset);
				char *p = (char *)malloc(member.len * 2 + 1);
				bin2hex(member.data, member.len, p);
				str.AppendFormat("%*s%s: \"%s\"\n", level, "", f[i].name, p);
				free(p);
			}
			break;

		case PROTOBUF_C_TYPE_MESSAGE:
			if (f[i].label == PROTOBUF_C_LABEL_REPEATED) {
				for (j = 0;
					j < STRUCT_MEMBER(size_t, m, f[i].quantifier_offset);
					j++) {
					str.AppendFormat("%*s%s {\n", level, "", f[i].name);
					protobuf_c_text_to_string_internal(str, level + 2,
						STRUCT_MEMBER(ProtobufCMessage **, m, f[i].offset)[j],
						(ProtobufCMessageDescriptor *)f[i].descriptor);
					str.AppendFormat("%*s}\n", level, "");
				}
			}
			else {
				str.AppendFormat("%*s%s {\n", level, "", f[i].name);
				protobuf_c_text_to_string_internal(str, level + 2,
					STRUCT_MEMBER(ProtobufCMessage *, m, f[i].offset),
					(ProtobufCMessageDescriptor *)f[i].descriptor);
				str.AppendFormat("%*s}\n", level, "");
			}
			break;

		default:
			return;
		}
	}
}

/** @} */  /* End of generate group. */

/* See .h file for API docs. */

CMStringA protobuf_c_text_to_string(const ProtobufCMessage &msg)
{
	CMStringA ret;

	ret.AppendFormat("%s {\n", msg.descriptor->c_name);
	protobuf_c_text_to_string_internal(ret, 1, &msg, msg.descriptor);
	ret.Append("}\n");

	return ret;
}
