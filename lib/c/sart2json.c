/**
 * This file is part of Sarthaka, a structured data binary codec.

Copyright (c) 2023, Kees-Jan Hermans <kees.jan.hermans@gmail.com>
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:
    * Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in the
      documentation and/or other materials provided with the distribution.
    * Neither the name of the organization nor the
      names of its contributors may be used to endorse or promote products
      derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL Kees-Jan Hermans BE LIABLE FOR ANY
DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

 *
 * \file
 * \brief
 */

#include <stdio.h>
#include <string.h>
#include <stdarg.h>

#include <sarthaka.h>

unsigned currentbyte = 0;
unsigned bitoffset = 8;
unsigned eofset = 0;

static void s2j_readbit
  (void* arg, unsigned* bit, unsigned* eof)
{
  if (eofset) {
    *bit = 0;
    *eof = 1;
    return;
  }
  if (bitoffset > 7) {
    bitoffset = 0;
    int b = getc(stdin);
    if (b == EOF) {
      eofset = 1;
      *bit = 0;
      *eof = eofset;
      return;
    }
    currentbyte = b;
  }
  *bit = (currentbyte & (1 << bitoffset)) ? 1 : 0;
  *eof = eofset;
  ++bitoffset;
}

static
void s2j_push
  (void* arg, unsigned type)
{
  switch (type) {
  case SARTHAKA_TYPE_STRING:
    fprintf(stdout, "\"");
    break;
  case SARTHAKA_TYPE_ARRAY:
    fprintf(stdout, "[");
    break;
  case SARTHAKA_TYPE_HASHTABLE:
    fprintf(stdout, "{");
    break;
  }
}

static
void s2j_pop
  (void* arg, unsigned type)
{
  switch (type) {
  case SARTHAKA_TYPE_STRING:
    fprintf(stdout, "\"");
    break;
  case SARTHAKA_TYPE_ARRAY:
    fprintf(stdout, "]");
    break;
  case SARTHAKA_TYPE_HASHTABLE:
    fprintf(stdout, "}");
    break;
  }
}

static
void s2j_pushelt
  (void* arg, unsigned type, ...)
{
  va_list ap;
  unsigned* nelts = arg;

  if (*nelts) {
    fprintf(stdout, ",");
  }
  ++(*nelts);
  va_start(ap, type);
  switch (type) {
  case SARTHAKA_TYPE_IMPLICITNULL:
  case SARTHAKA_TYPE_EXPLICITNULL:
    fprintf(stdout, "null");
    break;
  case SARTHAKA_TYPE_BOOLEAN:
    unsigned b = va_arg(ap, unsigned);
    if (b) {
      fprintf(stdout, "true");
    } else {
      fprintf(stdout, "false");
    }
    break;
  case SARTHAKA_TYPE_FLOAT:
    double d = va_arg(ap, double);
    fprintf(stdout, "%f", d);
    break;
  case SARTHAKA_TYPE_INTEGER:
    int64_t i = va_arg(ap, int64_t);
    fprintf(stdout, "%" PRId64, i);
    break;
  }
  va_end(ap);
}

static
void s2j_pushchar
  (void* arg, unsigned chr)
{
  if (chr <= 127) {
    fprintf(stdout, "%c", chr);
  } else {
  }
}

int main
  (int argc, char* argv[])
{
  sarthaka_t s;
  unsigned count = 0;
  (void)argc;
  (void)argv;

  memset(&s, 0, sizeof(s));
  s.unicode = 1;
  s.readbit = s2j_readbit;
  s.push = s2j_push;
  s.pop = s2j_pop;
  s.pushelt = s2j_pushelt;
  s.pushchar = s2j_pushchar;
  s.structarg = &count;

  sarthaka_decode(&s);
  fprintf(stdout, "\n");
}
