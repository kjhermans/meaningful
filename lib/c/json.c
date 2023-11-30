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

#include "json.h"

#include "json_bytecode.h"
#include "json_slotmap.h"

static
int parser_init = 0;

static
naie_engine_t parser;

/**
 * Like strcmp
 */
int json_string_compare
  (json_string_t* s1, json_string_t* s2)
{
  unsigned smallestlen = (s1->length > s2->length) ? s2->length : s1->length;

  for (unsigned i=0; i < smallestlen; i++) {
    unsigned c1 = (s1->wide ? s1->value.unicode[ i ] : s1->value.bytes[ i ]);
    unsigned c2 = (s2->wide ? s2->value.unicode[ i ] : s2->value.bytes[ i ]);
    if (c1 < c2) {
      return -1;
    } else if (c1 > c2) {
      return 1;
    }
  }
  if (s1->length == s2->length) {
    return 0;
  } else if (s1->length > s2->length) {
    return 1;
  } else {
    return -1;
  }
}

json_t* json_parse
  (char* string)
{
  naio_result_t result;
  naio_resobj_t* resobj;

  if (parser_init == 0) {
    NAIG_CHECK_ALT(
      naie_engine_init(
        &parser,
        json_byc,
        sizeof(json_byc)
      ),
      NULL
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
    fprintf(stderr, "JSON parse error %d.\n", e.code);
    naie_result_free(&result);
    return NULL;
  }
  if ((resobj = naio_result_object(
                      (unsigned char*)string,
                      strlen(string),
                      &result)) == NULL)
  {
    fprintf(stderr, "JSON conversion error.\n");
    naie_result_free(&result);
    return NULL;
  }
  naie_result_free(&result);

//..
}
