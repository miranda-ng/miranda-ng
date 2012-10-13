/* xmalloc.c -- memory allocation including 'out of memory' checks
 *
 * Copyright 2010 Robin Gareus <robin@gareus.org>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 * 
 */

#include <stdio.h>	
#include <sys/types.h>
#include <string.h>		
#include <stdlib.h>

static void *xmalloc_fatal(size_t size) {
  if (size==0) return NULL;
  fprintf(stderr, "Out of memory.");
  exit(1);
}

void *xmalloc (size_t size) {
  void *ptr = malloc (size);
  if (ptr == NULL) return xmalloc_fatal(size);
  return ptr;
}

void *xcalloc (size_t nmemb, size_t size) {
  void *ptr = calloc (nmemb, size);
  if (ptr == NULL) return xmalloc_fatal(nmemb*size);
  return ptr;
}

void *xrealloc (void *ptr, size_t size) {
  void *p = realloc (ptr, size);
  if (p == NULL) return xmalloc_fatal(size);
  return p;
}

char *xstrdup (const char *s) {
  void *ptr = xmalloc(strlen(s)+1);
  strcpy (ptr, s);
  return (char*) ptr;
}

// vi: sts=2 sw=2 ts=2
