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

#include "stdafx.h"

#include <stdio.h>
#include <varargs.h>

#include "protobuf-c/protobuf-c-util.h"

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

static void
protobuf_c_text_to_string_internal(CMStringA &str,
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
		auto &F = f[i];

		/* Decide if something needs to be done for this field. */
		switch (F.label) {
		case PROTOBUF_C_LABEL_OPTIONAL:
			if (F.flags & PROTOBUF_C_FIELD_FLAG_ONEOF)
				if (STRUCT_MEMBER(unsigned, m, F.quantifier_offset) != F.id)
					continue;

			if (F.type == PROTOBUF_C_TYPE_STRING) {
				if (!STRUCT_MEMBER(char *, m, F.offset)
					|| (STRUCT_MEMBER(char *, m, F.offset)
						== (char *)F.default_value)) {
					continue;
				}
			}
			else if (F.type == PROTOBUF_C_TYPE_MESSAGE) {
				if (!STRUCT_MEMBER(char *, m, F.offset)) {
					continue;
				}
			}
			else {
				if (!STRUCT_MEMBER(protobuf_c_boolean, m, F.quantifier_offset)) {
					continue;
				}
			}
			break;
		case PROTOBUF_C_LABEL_REPEATED:
			if (!STRUCT_MEMBER(size_t, m, F.quantifier_offset)) {
				continue;
			}
			break;
		}

		quantifier_offset = STRUCT_MEMBER(size_t, m, F.quantifier_offset);
		/* Field exists and has data, dump it. */
		switch (F.type) {
		case PROTOBUF_C_TYPE_INT32:
		case PROTOBUF_C_TYPE_UINT32:
		case PROTOBUF_C_TYPE_FIXED32:
			if (F.label == PROTOBUF_C_LABEL_REPEATED) {
				for (j = 0; j < quantifier_offset; j++) {
					str.AppendFormat(
						"%*s%s: %u\n",
						level, "", F.name,
						STRUCT_MEMBER(uint32_t *, m, F.offset)[j]);
				}
			}
			else {
				str.AppendFormat(
					"%*s%s: %u\n",
					level, "", F.name,
					STRUCT_MEMBER(uint32_t, m, F.offset));
			}
			break;
		case PROTOBUF_C_TYPE_SINT32:
		case PROTOBUF_C_TYPE_SFIXED32:
			if (F.label == PROTOBUF_C_LABEL_REPEATED) {
				for (j = 0; j < quantifier_offset; j++) {
					str.AppendFormat(
						"%*s%s: %d\n",
						level, "", F.name,
						STRUCT_MEMBER(int32_t *, m, F.offset)[j]);
				}
			}
			else {
				str.AppendFormat(
					"%*s%s: %d\n",
					level, "", F.name,
					STRUCT_MEMBER(int32_t, m, F.offset));
			}
			break;
		case PROTOBUF_C_TYPE_INT64:
		case PROTOBUF_C_TYPE_UINT64:
		case PROTOBUF_C_TYPE_FIXED64:
			if (F.label == PROTOBUF_C_LABEL_REPEATED) {
				for (j = 0; j < quantifier_offset; j++) {
					str.AppendFormat(
						"%*s%s: %lu\n",
						level, "", F.name,
						STRUCT_MEMBER(uint64_t *, m, F.offset)[j]);
				}
			}
			else {
				str.AppendFormat(
					"%*s%s: %lu\n",
					level, "", F.name,
					STRUCT_MEMBER(uint64_t, m, F.offset));
			}
			break;
		case PROTOBUF_C_TYPE_SINT64:
		case PROTOBUF_C_TYPE_SFIXED64:
			if (F.label == PROTOBUF_C_LABEL_REPEATED) {
				for (j = 0; j < quantifier_offset; j++) {
					str.AppendFormat(
						"%*s%s: %ld\n",
						level, "", F.name,
						STRUCT_MEMBER(int64_t *, m, F.offset)[j]);
				}
			}
			else {
				str.AppendFormat(
					"%*s%s: %ld\n",
					level, "", F.name,
					STRUCT_MEMBER(int64_t, m, F.offset));
			}
			break;
		case PROTOBUF_C_TYPE_FLOAT:
			if (F.label == PROTOBUF_C_LABEL_REPEATED) {
				for (j = 0; j < quantifier_offset; j++) {
					float_var = STRUCT_MEMBER(float *, m, F.offset)[j];
					str.AppendFormat(
						"%*s%s: %g\n",
						level, "", F.name,
						float_var);
				}
			}
			else {
				float_var = STRUCT_MEMBER(float, m, F.offset);
				str.AppendFormat(
					"%*s%s: %g\n",
					level, "", F.name,
					float_var);
			}
			break;
		case PROTOBUF_C_TYPE_DOUBLE:
			if (F.label == PROTOBUF_C_LABEL_REPEATED) {
				for (j = 0; j < quantifier_offset; j++) {
					str.AppendFormat(
						"%*s%s: %g\n",
						level, "", F.name,
						STRUCT_MEMBER(double *, m, F.offset)[j]);
				}
			}
			else {
				str.AppendFormat(
					"%*s%s: %g\n",
					level, "", F.name,
					STRUCT_MEMBER(double, m, F.offset));
			}
			break;
		case PROTOBUF_C_TYPE_BOOL:
			if (F.label == PROTOBUF_C_LABEL_REPEATED) {
				for (j = 0; j < quantifier_offset; j++) {
					str.AppendFormat(
						"%*s%s: %s\n",
						level, "", F.name,
						STRUCT_MEMBER(protobuf_c_boolean *, m, F.offset)[j] ?
						"true" : "false");
				}
			}
			else {
				str.AppendFormat(
					"%*s%s: %s\n",
					level, "", F.name,
					STRUCT_MEMBER(protobuf_c_boolean, m, F.offset) ?
					"true" : "false");
			}
			break;
		case PROTOBUF_C_TYPE_ENUM:
			enumd = (ProtobufCEnumDescriptor *)F.descriptor;
			if (F.label == PROTOBUF_C_LABEL_REPEATED) {
				for (j = 0; j < quantifier_offset; j++) {
					enumv = protobuf_c_enum_descriptor_get_value(
						enumd, STRUCT_MEMBER(int *, m, F.offset)[j]);
					str.AppendFormat(
						"%*s%s: %s\n",
						level, "", F.name,
						enumv ? enumv->name : "unknown");
				}
			}
			else {
				enumv = protobuf_c_enum_descriptor_get_value(
					enumd, STRUCT_MEMBER(int, m, F.offset));
				str.AppendFormat(
					"%*s%s: %s\n",
					level, "", F.name,
					enumv ? enumv->name : "unknown");
			}
			break;
		case PROTOBUF_C_TYPE_STRING:
			if (F.label == PROTOBUF_C_LABEL_REPEATED) {
				for (j = 0; j < quantifier_offset; j++) {
					str.AppendFormat(
						"%*s%s: \"%s\"\n", level, "", F.name, STRUCT_MEMBER(unsigned char **, m, F.offset)[j]);
				}
			}
			else {
				str.AppendFormat(
					"%*s%s: \"%s\"\n", level, "", F.name, STRUCT_MEMBER(unsigned char *, m, F.offset));
			}
			break;
		case PROTOBUF_C_TYPE_BYTES:
			if (F.label == PROTOBUF_C_LABEL_REPEATED) {
				for (j = 0; j < quantifier_offset; j++) {
					ProtobufCBinaryData &pData = STRUCT_MEMBER(ProtobufCBinaryData *, m, F.offset)[j];
					ptrA tmp((char *)mir_alloc(pData.len * 2 + 1));
					bin2hex(pData.data, pData.len, tmp);
					str.AppendFormat(
						"%*s%s: \"%s\"\n", level, "", F.name, tmp.get());
				}
			}
			else {
				ProtobufCBinaryData &pData = STRUCT_MEMBER(ProtobufCBinaryData, m, F.offset);
				ptrA tmp((char *)mir_alloc(pData.len * 2 + 1));
				bin2hex(pData.data, pData.len, tmp);
				str.AppendFormat(
					"%*s%s: \"%s\"\n", level, "", F.name, tmp.get());
			}
			break;

		case PROTOBUF_C_TYPE_MESSAGE:
			if (F.label == PROTOBUF_C_LABEL_REPEATED) {
				for (j = 0;
					j < STRUCT_MEMBER(size_t, m, F.quantifier_offset);
					j++) {
					str.AppendFormat(
						"%*s%s {\n", level, "", F.name);
					protobuf_c_text_to_string_internal(str, level + 2,
						STRUCT_MEMBER(ProtobufCMessage **, m, F.offset)[j],
						(ProtobufCMessageDescriptor *)F.descriptor);
					str.AppendFormat(
						"%*s}\n", level, "");
				}
			}
			else {
				str.AppendFormat(
					"%*s%s {\n", level, "", F.name);
				protobuf_c_text_to_string_internal(str, level + 2,
					STRUCT_MEMBER(ProtobufCMessage *, m, F.offset),
					(ProtobufCMessageDescriptor *)F.descriptor);
				str.AppendFormat(
					"%*s}\n", level, "");
			}
			break;

		default:
			return;
		}

	}
}

/** @} */  /* End of generate group. */

/* See .h file for API docs. */

CMStringA protobuf_c_text_to_string(const ProtobufCMessage *m)
{
	CMStringA res;
	protobuf_c_text_to_string_internal(res, 0, m, m->descriptor);
	return res;
}
