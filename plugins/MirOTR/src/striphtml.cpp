#include "stdafx.h"

void starttag_cb (void *cbdata, ekhtml_string_t *tag, ekhtml_attr_t *attrs) {
	STRIPHTML_DATA *data = (STRIPHTML_DATA *)cbdata;
	switch (tag->len) {
		case 1:
			switch (tag->str[0]) {
				case 'a':
				case 'A':
					for(ekhtml_attr_t *attr=attrs; attr; attr=attr->next) {
						if (_strnicmp(attr->name.str, "href", attr->name.len)==0) {
							data->stack.push(mir_strncpy((char*)mir_calloc(attr->val.len+1), attr->val.str, attr->val.len));
							break;
						}
					}
					break;
				case 'i':
				case 'I':
					data->buffer.append(" *");
					break;
				case 'b':
				case 'B':
					data->buffer.append(" _");
					break;
			}
			break;
		case 2:
			if (toupper(tag->str[0]) == 'B' && toupper(tag->str[1]) == 'R') 
				data->buffer.append("\r\n");
			break;
		case 3:
			if (_strnicmp(tag->str, "img", 3) == 0) {
				data->buffer.append("IMAGE [ ");
				for(ekhtml_attr_t *attr=attrs; attr; attr=attr->next) {
					if (_strnicmp(attr->name.str, "src", attr->name.len)==0) {
						data->buffer.append(attr->val.str, attr->val.len);
						break;
					}
				}
				data->buffer.append(" ] ");
			}
			break;
	}
}

void endtag_cb (void *cbdata, ekhtml_string_t *tag) {
	STRIPHTML_DATA *data = (STRIPHTML_DATA *)cbdata;
	switch (tag->len) {
		case 1:
			switch (*(tag->str)) {
				case 'a':
				case 'A':
					if (data->stack.empty()) break;
					data->buffer.append(" [ ");
					data->buffer.append(data->stack.top());
					mir_free(data->stack.top());
					data->stack.pop();
					data->buffer.append(" ] ");
					break;
				case 'i':
				case 'I':
					data->buffer.append("* ");
					break;
				case 'b':
				case 'B':
					data->buffer.append("_ ");
					break;
			}
	}

}

void data_cb (void *cbdata, ekhtml_string_t *text) {
	STRIPHTML_DATA *data = (STRIPHTML_DATA *)cbdata;
	if(text->len <= 0)
		return;
	char* s = (char*) mir_calloc(text->len+1);
	decode_html_entities_utf8(s, text->str, text->len);

	if (!data->stack.empty()) {
		char *top = data->stack.top();
		if (_stricmp(s, top)==0) {
			mir_free(top);
			data->stack.pop();
		}
	}
	

	data->buffer.append(s);
	mir_free(s);
}

char * striphtml(char *html) {
	STRIPHTML_DATA data;
	ekhtml_string_t ekstring;

	ekstring.len = mir_strlen(html);
	ekstring.str = html;

	data.buffer.clear();
	data.buffer.reserve(ekstring.len);

	ekhtml_parser_t *parser = ekhtml_parser_new(&data);
	ekhtml_parser_datacb_set(parser, &data_cb);
	ekhtml_parser_startcb_add(parser, NULL, &starttag_cb);
	ekhtml_parser_endcb_add(parser, NULL, &endtag_cb);

	ekhtml_parser_feed(parser, &ekstring);
	ekhtml_parser_flush(parser, 1);

	while (!data.stack.empty()) {
		mir_free(data.stack.top());
		data.stack.pop();
	}

	ekhtml_parser_destroy(parser);

	char *s = mir_strdup(data.buffer.c_str());
	data.buffer.erase();
	return s;
	//ekhtml_parser_create(
}
