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

#include "xml_parser.h"
#include "sarthaka.h"

extern int absorb_file
  (char* path, unsigned char** buf, unsigned* buflen);

static
void x2s_writebit
  (void* arg, unsigned bit)
{
}

struct x2stoken
{
  unsigned    state;
#define X2S_ITEM                0
#define X2S_ATTR                1
#define X2S_VALUE               2

  itemlist_t  stack;
  xmlitem_t*  curitem;
  xmlattr_t*  curattr;
  unsigned    param[ 2 ];
};

static
void x2s_getnexttoken
  (void* arg, sarthaka_token_t* token)
{
  struct x2stoken* t = arg;

  switch (t->state) {
  case X2S_ITEM:
  case X2S_ATTR:
  case X2S_VALUE:
  }
}

/**
 *
 */
int main
  (int argc, char* argv[])
{
  char* file = argv[ 1 ];
  unsigned char* contents = 0;
  unsigned len = 0;
  xml_t xml = { 0 };
  (void)argc;

  if (file) {
    absorb_file(file, &contents, &len);
    xml_parse((char*)contents, &xml);
    free(contents);
  } else {
    fprintf(stderr, "No file argument given.\n");
    return -1;
  }

  sarthaka_t s = { 0 };
  struct x2stoken t = { 0 };
  xmlitem_t item = { 0 };

  s.writebit     = x2s_writebit;
  s.writearg     = 0;
  s.getnexttoken = x2s_getnexttoken;
  s.tokenizerarg = &t;
  t.curitem      = &item;
  item.form.tag  = xml.main;
  sarthaka_encode(&s);

  return 0;
}
