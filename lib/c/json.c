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

static
json_t* json_reconstruct
  (naio_resobj_t* obj);

static
unsigned json_charval
  (naio_resobj_t* resobj)
{
  if (resobj->stringlen == 1 && resobj->string[ 0 ] < 128) {
    return (unsigned)(resobj->string[ 0 ]);
  } else {
    if (0 == strcmp(resobj->string, "\\n")) {
      return (unsigned)'\n';
    } else if (0 == strcmp(resobj->string, "\\r")) {
      return (unsigned)'\r';
    } else if (0 == strcmp(resobj->string, "\\t")) {
      return (unsigned)'\t';
    } else if (0 == strcmp(resobj->string, "\\v")) {
      return (unsigned)'\v';
    } else if (0 == strcmp(resobj->string, "\\\"")) {
      return (unsigned)'"';
    } else {
//..
    }
  }
  return 0;
}

static
json_string_t json_reconstruct_jsonstring
  (naio_resobj_t* obj)
{
  json_string_t result = { 0 };

  for (unsigned i=0; i < obj->nchildren; i++) {
    unsigned charval = json_charval(obj->children[ i ]);
    if (charval > 127) {
      result.wide = 1;
    }
  }
  if (result.wide) {
    result.value.unicode = calloc(4, obj->nchildren + 1);
  } else {
    result.value.bytes = calloc(1, obj->nchildren + 1);
  }
  for (unsigned i=0; i < obj->nchildren; i++) {
    unsigned charval = json_charval(obj->children[ i ]);
    if (result.wide) {
      result.value.unicode[ i ] = charval;
    } else {
      result.value.bytes[ i ] = charval;
    }
  }
  return result;
}

static
json_t* json_reconstruct_hashtable
  (naio_resobj_t* obj)
{
  json_t* result = calloc(1, sizeof(json_t));

  result->type = JSON_TYPE_HASHTABLE;
  for (unsigned i=0; i < obj->nchildren; i++) {
    json_string_t key = json_reconstruct_jsonstring(obj->children[ i ]->children[ 0 ]);
    json_t* value = json_reconstruct(obj->children[ i ]->children[ 1 ]);
    json_hashtable_put(&(result->value.hashtable), key, value);
  }
  return result;
}

static
json_t* json_reconstruct_array
  (naio_resobj_t* obj)
{
  json_t* result = calloc(1, sizeof(json_t));

  result->type = JSON_TYPE_ARRAY;
  for (unsigned i=0; i < obj->nchildren; i++) {
    json_t* value = json_reconstruct(obj->children[ i ]);
    json_array_push(&(result->value.array), value);
  }
  return result;
}

static
json_t* json_reconstruct_string
  (naio_resobj_t* obj)
{
  json_t* result = calloc(1, sizeof(json_t));

  result->type = JSON_TYPE_STRING;
  result->value.string = json_reconstruct_jsonstring(obj);
  return result;
}

static
json_t* json_reconstruct_int
  (naio_resobj_t* obj)
{
  json_t* result = calloc(1, sizeof(json_t));

  result->type = JSON_TYPE_INTEGER;
  result->value.boolint = strtol(obj->string, 0, 10);
  return result;
}

static
json_t* json_reconstruct_float
  (naio_resobj_t* obj)
{
  json_t* result = calloc(1, sizeof(json_t));

  result->type = JSON_TYPE_FLOAT;
  result->value.boolint = strtod(obj->string, 0);
  return result;
}

static
json_t* json_reconstruct_boolean
  (naio_resobj_t* obj)
{
  json_t* result = calloc(1, sizeof(json_t));

  result->type = JSON_TYPE_BOOLEAN;
  result->value.boolint = (0 == strcmp(obj->string, "true")) ? 1 : 0;
  return result;
}

static
json_t* json_reconstruct_null
  ()
{
  json_t* result = calloc(1, sizeof(json_t));

  result->type = JSON_TYPE_NULL;
  return result;
}

static
json_t* json_reconstruct
  (naio_resobj_t* obj)
{
  switch (obj->type) {
  case 0: // SLOTMAP_HASH_CBOPENOPTHASHELTSCBCLOSE 
    return json_reconstruct_hashtable(obj);
  case 2: // SLOTMAP_ARRAY_ABOPENOPTARRAYELTSABCLOSE
    return json_reconstruct_array(obj);
  case 4: // SLOTMAP_STRING_NRTVUTFCHAR
    return json_reconstruct_string(obj);
  case 6: // SLOTMAP_INT_
    return json_reconstruct_int(obj);
  case 7: // SLOTMAP_FLOAT_
    return json_reconstruct_float(obj);
  case 8: // SLOTMAP_BOOL_TRUEFALSE
    return json_reconstruct_boolean(obj);
  case 9: // SLOTMAP_NULL_NULL
    return json_reconstruct_null();
default: fprintf(stderr, "UNKNOWN TOKEN TYPE %u\n", obj->type);
  }
  abort();
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
naio_resobj_debug(resobj, 0);
  return json_reconstruct(resobj->children[ 0 ]);
}

void json_debug
  (json_t* json)
{
  //..
}