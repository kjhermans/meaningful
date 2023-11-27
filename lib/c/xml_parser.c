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
#include <stdlib.h>

#include "xml.h"
#include "xml_parser.h"
#include "xml_slotmap.h"

static
int parser_init = 0;

static
naie_engine_t parser;

static
unsigned xml_char_value
  (naio_resobj_t* resobj)
{
  switch (resobj->type) {
  case 11:
    if (0 == strcmp(resobj->string, "nbsp")) {
      return ' ';
    } else if (0 == strcmp(resobj->string, "lt")) {
      return '<';
    } else if (0 == strcmp(resobj->string, "gt")) {
      return '>';
    } else {
//.. fish out the # escapes
      return 0;
    }
  case 14:
    if (resobj->stringlen > 1) {
      if ((resobj->string[ 0 ] >> 5) == 0x06) {
        return (
          ((resobj->string[ 0 ] & 0x1f) << 6) |
          ((resobj->string[ 1 ] & 0x3f))
        );
      } else if ((resobj->string[ 0 ] >> 4) == 0x0e) {
        return (
          ((resobj->string[ 0 ] & 0x0f) << 12) |
          ((resobj->string[ 1 ] & 0x3f) << 6) |
          ((resobj->string[ 2 ] & 0x3f))
        );
      } else if ((resobj->string[ 0 ] >> 3) == 0x1e) {
        return (
          ((resobj->string[ 0 ] & 0x07) << 18) |
          ((resobj->string[ 1 ] & 0x3f) << 12) |
          ((resobj->string[ 2 ] & 0x3f) << 6) |
          ((resobj->string[ 3 ] & 0x3f))
        );
      } else {
unsigned char* foo = resobj->string;
fprintf(stderr, "NOW WHAT %.2x %.2x %.2x %.2x\n", foo[0], foo[1], foo[2], foo[3]);
      }
    } else {
      return resobj->string[ 0 ];
    }
  }
  return 0;
}

static
xmlstring_t xml_parse_string
  (naio_resobj_t* resobj)
{
  xmlstring_t result = { 0 };

  for (unsigned i=0; i < resobj->nchildren; i++) {
    unsigned charval = xml_char_value(resobj->children[ i ]);
    if (charval > 127) {
fprintf(stderr, "GOT %x\n", charval);
      result.iswide = 1;
    }
  }
  result.length = resobj->nchildren;
  if (result.iswide) {
    result.value.wide = calloc(1, sizeof(uint32_t) * (result.length+1));
    for (unsigned i=0; i < resobj->nchildren; i++) {
      result.value.wide[ i ] = xml_char_value(resobj->children[ i ]);
    }
  } else {
    result.value.bytes = calloc(1, result.length+1);
    memcpy(result.value.bytes, resobj->string, result.length);
  }
  return result;
}

static
xmlattr_t xml_parse_attribute
  (naio_resobj_t* resobj)
{
  xmlattr_t attr = { 0 };

  xmlstring_t name = xml_parse_string(resobj->children[ 0 ]->children[ 0 ]);
  xmlstring_t value = xml_parse_string(resobj->children[ 1 ]->children[ 0 ]);
  attr.name = name;
  attr.value = value;
  return attr;
}

static
attrlist_t xml_parse_attributes
  (naio_resobj_t* resobj)
{
  attrlist_t result = { 0 };

  for (unsigned i=0; i < resobj->nchildren; i++) {
    xmlattr_t attr = xml_parse_attribute(resobj->children[ i ]);
    attrlist_push(&result, attr);
  }
  return result;
}

static
xmlitem_t* xml_parse_create_item
  (naio_resobj_t* resobj)
{
  xmlitem_t* item = calloc(1, sizeof(xmlitem_t));
  if (resobj->type == SLOTMAP_XMLCONTAINER_ABOPENTAGTAGWORDATT) {
    item->form.tag.name = xml_parse_string(resobj->children[ 0 ]->children[ 0 ]);
    item->form.tag.attributes = xml_parse_attributes(resobj->children[ 1 ]);
    for (unsigned i=0; i < resobj->children[ 2 ]->nchildren; i++) {
      xmlitem_t* subitem = xml_parse_create_item(resobj->children[ 2 ]->children[ i ]);
      itemlist_push(&(item->form.tag.items), subitem);
    }
  } else if (resobj->type == SLOTMAP_XMLSINGLETON_ABOPENTAGWORDATTRSS) {
    item->form.tag.name = xml_parse_string(resobj->children[ 0 ]); //->children[ 0 ]);
    item->form.tag.attributes = xml_parse_attributes(resobj->children[ 1 ]);
  } else if (resobj->type == SLOTMAP_XMLWORD_NRTXMLCHAR) {
    item->form.text = xml_parse_string(resobj);
    item->istext = 1;
  }
  return item;
}

static
void xml_parse_restructure
  (naio_resobj_t* resobj, xml_t* xml)
{
  xmlitem_t* item = xml_parse_create_item(resobj->children[ 0 ]);
  xml->main = item->form.tag;
}

static
void xml_debug_tag
  (xmltag_t* tag, unsigned indent)
{
  for (unsigned i=0; i < indent; i++) {
    fprintf(stderr, "  ");
  }
  fprintf(stderr, "<%s", tag->name.value.bytes);
  for (unsigned i=0; i < tag->attributes.count; i++) {
    fprintf(stderr, " %s=\"%s\""
      , tag->attributes.list[ i ].name.value.bytes
      , tag->attributes.list[ i ].value.value.bytes
    );
  }
  if (tag->items.count) {
    fprintf(stderr, ">\n");
    for (unsigned i=0; i < tag->items.count; i++) {
      if (tag->items.list[ i ]->istext) {
        fprintf(stderr, " %s", tag->items.list[ i ]->form.text.value.bytes);
      } else {
        xml_debug_tag(&(tag->items.list[ i ]->form.tag), indent+1);
      }
    }
    fprintf(stderr, "</%s>\n", tag->name.value.bytes);
  } else {
    fprintf(stderr, "/>\n");
  }
}

void xml_debug
  (xml_t* xml)
{
  xml_debug_tag(&(xml->main), 0);
}

/**
 *
 */
int xml_parse
  (char* string, xml_t* xml)
{
  naio_result_t result;
  naio_resobj_t* resobj;

  if (parser_init == 0) {
    NAIG_CHECK_ALT(
      naie_engine_init(
        &parser,
        xml_byc,
        sizeof(xml_byc)
      ),
      ~0
    );
    parser_init = 1;
  }
  NAIG_ERR_T e = 
    naie_engine_run(
      &parser,
      (unsigned char*)string,
      strlen(string),
      &result
    );
  if (e.code) {
    fprintf(stderr, "XML parse error %d.\n", e.code);
    naie_result_free(&result);
    return ~0;
  }
  if ((resobj = naio_result_object(
                      (unsigned char*)string,
                      strlen(string),
                      &result)) == NULL)
  {
    fprintf(stderr, "XML conversion error.\n");
    naie_result_free(&result);
    return ~0;
  }
  naie_result_free(&result);
//  naio_result_object_debug(resobj, 0);
  xml_parse_restructure(resobj, xml);
xml_debug(xml);
  return 0;
}
