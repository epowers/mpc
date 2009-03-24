/* mpc_inp_str -- Input a complex number from a given stream.

Copyright (C) 2009 Andreas Enge, Philippe Th\'eveny

This file is part of the MPC Library.

The MPC Library is free software; you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation; either version 2.1 of the License, or (at your
option) any later version.

The MPC Library is distributed in the hope that it will be useful, but
WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public
License for more details.

You should have received a copy of the GNU Lesser General Public License
along with the MPC Library; see the file COPYING.LIB.  If not, write to
the Free Software Foundation, Inc., 59 Temple Place - Suite 330, Boston,
MA 02111-1307, USA. */

#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include "mpc-impl.h"

static size_t
skip_whitespace (FILE *stream) {
   int c = getc (stream);
   size_t size = 0;
   while (c != EOF && isspace ((unsigned char) c)) {
      c = getc (stream);
      size++;
   }
   if (c != EOF)
      ungetc (c, stream);
   return size;
}


static char *
mpc_alloc_str (size_t len) {
   void * (*allocfunc) (size_t);
   mp_get_memory_functions (&allocfunc, NULL, NULL);
   return (char *) ((*allocfunc) (len * sizeof (char)));
}


static char *
mpc_realloc_str (char * str, size_t oldlen, size_t newlen) {
   void * (*reallocfunc) (void *, size_t, size_t);
   mp_get_memory_functions (NULL, &reallocfunc, NULL);
   return (char *) ((*reallocfunc) (str, oldlen * sizeof (char),
                    newlen * sizeof (char)));
}


/* Extract from stream the longest string made up of alphanumeric char and
   '_' (i.e. n-char-sequence).
   The user must free the returned string. */
static char *
extract_suffix (FILE *stream)
{
  int c;
  size_t nread = 0;
  size_t strsize = 100;
  char *str = mpc_alloc_str (strsize);

  c = getc (stream);
  while (isalnum ((unsigned char) c) || c == '_') {
    str [nread] = (char) c;
    nread++;
    if (nread == strsize) {
      str = mpc_realloc_str (str, strsize, 2 * strsize);
      strsize *= 2;
         }
    c = getc (stream);
  }

  str = mpc_realloc_str (str, strsize, nread + 1);
  strsize = nread + 1;
  str [nread] = '\0';

  if (c != EOF)
    ungetc (c, stream);
  return str;
}


/* Extract from the stream the longest string of caracters which are not
   whitespace with optional bracketed n-char_sequence after a NaN.
   The user must free the returned string.
   FIXME: The returned string is not always a valid floating-point number */
static char *
extract_string (FILE *stream)
{
  int c;
  size_t nread = 0;
  size_t strsize = 100;
  char *str = mpc_alloc_str (strsize);

  c = getc (stream);
  while (c != EOF && c != '\n'
         && !isspace ((unsigned char) c)
         && c != '(' && c != ')') {
    str [nread] = (char) c;
    nread++;
    if (nread == strsize) {
      str = mpc_realloc_str (str, strsize, 2 * strsize);
      strsize *= 2;
    }
    c = getc (stream);
  }

  str = mpc_realloc_str (str, strsize, nread + 1);
  strsize = nread + 1;
  str [nread] = '\0';

  if (nread == 0)
    return str;

  if (c == '(') {
    size_t n;
    char *suffix;

    /* (n-char-sequence) only after a NaN */
    if ((nread != 3
         || tolower (str[0]) != 'n'
         || tolower (str[1]) != 'a'
         || tolower (str[2]) != 'n')
        && (nread != 5 
            || str[0] != '@'
            || tolower (str[1]) != 'n'
            || tolower (str[2]) != 'a'
            || tolower (str[3]) != 'n'
            || str[4] != '@')) {
      ungetc (c, stream);
      return str;
    }

    suffix = extract_suffix (stream);
    nread += strlen (suffix) + 1;
    if (nread >= strsize) {
      str = mpc_realloc_str (str, strsize, nread + 1);
      strsize = nread + 1;
    }

    n = sprintf (str, "%s(%s", str, suffix);
    MPC_ASSERT (n == nread);

    c = getc (stream);
    if (c == ')') {
      str = mpc_realloc_str (str, strsize, nread + 2);
      strsize = nread + 2;
      str [nread] = c;
      str [nread+1] = '\0';
      nread++;
    }
    else if (c != EOF)
      ungetc (c, stream);

    mpc_free_str (suffix);
  }
  else if (c != EOF)
    ungetc (c, stream);

  return str;
}


int
mpc_inp_str (mpc_ptr rop, FILE *stream, size_t *read, int base, mpc_rnd_t rnd_mode) {
   size_t white, nread = 0;
   int inex = -1;
   int c;
   char *str;

   if (stream == NULL)
      stream = stdin;

   white = skip_whitespace (stream);
   c = getc (stream);
   if (c != EOF) {
     if (c == '(') {
       char *real_str;
       char *imag_str;
       size_t n;

       nread++; /* the opening parenthesis */
       white = skip_whitespace (stream);
       real_str = extract_string (stream);
       nread += strlen(real_str);

       c = getc (stream);
       if (!isspace ((unsigned int) c)) {
         if (c != EOF)
           ungetc (c, stream);
         mpc_free_str (real_str);
         goto error;
       }
       else
         ungetc (c, stream);

       white += skip_whitespace (stream);
       imag_str = extract_string (stream);
       nread += strlen (imag_str);

       str = mpc_alloc_str (nread + 2);
       n = sprintf (str, "(%s %s", real_str, imag_str);
       MPC_ASSERT (n == nread + 1);
       mpc_free_str (real_str);
       mpc_free_str (imag_str);

       white += skip_whitespace (stream);
       c = getc (stream);
       if (c == ')') {
         str = mpc_realloc_str (str, nread +2, nread + 3);
         str [nread+1] = c;
         str [nread+2] = '\0';
         nread++;
       }
       else if (c != EOF)
         ungetc (c, stream);
     }
     else {
       if (c != EOF)
         ungetc (c, stream);
       str = extract_string (stream);
       nread += strlen (str);
     }

     inex = mpc_set_str (rop, str, base, rnd_mode);

     mpc_free_str (str);
   }

 error:
   if (inex == -1) {
      mpfr_set_nan (MPC_RE(rop));
      mpfr_set_nan (MPC_IM(rop));
   }
   if (read != NULL)
     *read = white + nread;
   return inex;
}
