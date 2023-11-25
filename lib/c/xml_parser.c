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
xmlitem_t* xml_parse_create_item
  (naio_resobj_t* resobj)
{
  xmlitem_t* item = calloc(1, sizeof(xmlitem_t));
  char* tagname = resobj->children[ 0 ]->string;
  item->form.tag = calloc(1, sizeof(xmltag_t));
  item->form.tag->name = strdup(tagname);
  return item;
}

static
void xml_parse_restructure
  (naio_resobj_t* resobj, xml_t* xml)
{
  xmlitem_t* item = xml_parse_create_item(resobj->children[ 0 ]);
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
  naio_result_object_debug(resobj, 0);
  xml_parse_restructure(resobj, xml);
  return 0;
}
