#ifndef GLYPH_H
#define GLYPH_H

#ifdef __cplusplus
extern "C" {
#endif

extern double text_width;
extern double text_height;

void draw_glyph(int);
void draw_string(const char *s);
void draw_string_centered(const char *s);

void set_text_size(const double ttext_width, const double ttext_height);

#ifdef __cplusplus
};
#endif

#endif
