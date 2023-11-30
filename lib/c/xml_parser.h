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

#ifndef _XML_PARSER_H_
#define _XML_PARSER_H_

#include <naigama/naigama.h>

typedef struct
{
  union {
    char*             bytes;
    uint32_t*         wide;
  }                 value;
  unsigned          length;
  int               iswide;
}
xmlstring_t;

typedef struct
{
  xmlstring_t       name;
  xmlstring_t       value;
}
xmlattr_t;

typedef struct xmlitem xmlitem_t;

#include "array.h"
MAKE_ARRAY_HEADER(xmlattr_t, attrlist_)
MAKE_ARRAY_HEADER(xmlitem_t*, itemlist_)

typedef struct
{
  xmlstring_t       name;
  attrlist_t        attributes;
  itemlist_t        items;
}
xmltag_t;

struct xmlitem
{
  int               istext;
  union {
    xmltag_t          tag;
    xmlstring_t       text;
  }                 form;
};

typedef struct
{
  xmltag_t          main;
}
xml_t;

extern void xml_debug
  (xml_t* xml);

extern int xml_parse
  (char* string, xml_t* xml);

#endif
