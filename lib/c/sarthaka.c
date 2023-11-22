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

#include <sarthaka.h>
#include <stdio.h> // remove

static
void sarthaka_decode_tuple
  (sarthaka_t* s);

static
void sarthaka_readbit
  (sarthaka_t* s, unsigned* result)
{
  unsigned eof;

  if (s->eof) {
    *result = 0;
    return;
  }
  s->readbit(s->readarg, result, &eof);
  if (eof) {
    s->eof = 1;
  }
}

static
void sarthaka_writebit
  (sarthaka_t* s, unsigned bit)
{
  if (bit) {
    s->writebit(s->writearg, 1);
  } else {
    s->writebit(s->writearg, 0);
  }
}

static
void sarthaka_decode_bits
  (sarthaka_t* s, unsigned nbits, unsigned* result)
{
  *result = 0;

  for (unsigned i=0; i < nbits; i++) {
    unsigned bit;
    sarthaka_readbit(s, (unsigned*)&bit);
    if (bit) {
#ifdef _SARTHAKA_SMALLENDIAN_
      *result |= (1 << i);
#else
      *result |= (1 << (nbits-(i+1)));
#endif
    }
  }
}

static
void sarthaka_encode_bits
  (sarthaka_t* s, unsigned nbits, unsigned value)
{
  for (unsigned i=0; i < nbits; i++) {
#ifdef _SARTHAKA_SMALLENDIAN_
    sarthaka_writebit(s, value & (1 << i));
#else
    sarthaka_writebit(s, value & (1 << (nbits-(i+1))));
#endif
  }
}

static
void sarthaka_decode_byte
  (sarthaka_t* s, unsigned* byte)
{
  unsigned b;

  sarthaka_decode_bits(s, 8, &b);
  *byte = (unsigned char)b;
}

static
void sarthaka_encode_byte
  (sarthaka_t* s, unsigned char byte)
{
  sarthaka_encode_bits(s, 8, (unsigned)byte);
}

static
void sarthaka_decode_bytes
  (sarthaka_t* s, unsigned char* buf, unsigned len)
{
  for (unsigned i=0; i < len; i++) {
    sarthaka_decode_byte(s, (unsigned*)&(buf[ i ]));
  }
}

static
void sarthaka_encode_bytes
  (sarthaka_t* s, unsigned char* buf, unsigned len)
{
  for (unsigned i=0; i < len; i++) {
    sarthaka_encode_byte(s, buf[ i ]);
  }
}

static
void sarthaka_decode_type
  (sarthaka_t* s, unsigned* type)
{
  sarthaka_decode_bits(s, 3, type);
}

static
void sarthaka_encode_type
  (sarthaka_t* s, unsigned type)
{
  sarthaka_encode_bits(s, 3, type);
}

static
void sarthaka_encode_null
  (sarthaka_t* s)
{
  sarthaka_encode_type(s, SARTHAKA_TYPE_EXPLICITNULL);
}

static
void sarthaka_decode_boolean
  (sarthaka_t* s)
{
  unsigned bit;

  sarthaka_decode_bits(s, 1, &bit);
  s->pushelt(s->structarg, SARTHAKA_TYPE_BOOLEAN, bit);
}

static
void sarthaka_encode_boolean
  (sarthaka_t* s, unsigned b)
{
  sarthaka_encode_type(s, SARTHAKA_TYPE_BOOLEAN);
  sarthaka_writebit(s, (b ? 1 : 0));
}

static
void sarthaka_decode_float
  (sarthaka_t* s)
{
  double d = 0;

  sarthaka_decode_bytes(s, (unsigned char*)&d, sizeof(d));
  s->pushelt(s->structarg, SARTHAKA_TYPE_FLOAT, d);
}

static
void sarthaka_encode_float
  (sarthaka_t* s, double d)
{
  sarthaka_encode_type(s, SARTHAKA_TYPE_FLOAT);
  sarthaka_encode_bytes(s, (unsigned char*)&d, sizeof(d));
}

static
void sarthaka_decode_integer
  (sarthaka_t* s)
{
  unsigned sign, cont, byte;
  int64_t i = 0;

  sarthaka_decode_bits(s, 1, &sign);
  for (unsigned i=0; i < 8; i++) {
    sarthaka_decode_bits(s, 1, &cont);
    if (!cont) {
      break;
    }
    if (i < 7) {
      sarthaka_decode_bits(s, 8, &byte);
    } else {
      sarthaka_decode_bits(s, 7, &byte);
    }
    i |= (byte << (i * 8));
  }
  if (sign) {
    i = -i;
  }
  s->pushelt(s->structarg, SARTHAKA_TYPE_INTEGER, i);
}

static
void sarthaka_encode_integer
  (sarthaka_t* s, int64_t value)
{
  unsigned i;

  sarthaka_encode_type(s, SARTHAKA_TYPE_INTEGER);
  if (value < 0) {
    sarthaka_writebit(s, 1);
    value = -value;
  } else {
    sarthaka_writebit(s, 0);
  }
  for (i=0; i < 8 && value; i++) {
    unsigned byte = value & 0xff;
    value >>= 8;
    sarthaka_writebit(s, 1);
    if (i < 7) {
      sarthaka_encode_byte(s, byte);
    } else {
      sarthaka_encode_bits(s, 7, byte);
    }
  }
  if (i < 8) {
    sarthaka_writebit(s, 0);
  }
}

static
void sarthaka_decode_string
  (sarthaka_t* s)
{
  unsigned blocksize;
  unsigned byte;

  s->push(s->structarg, SARTHAKA_TYPE_STRING);
  while (!(s->eof)) {
    sarthaka_decode_bits(s, SARTHAKA_STRING_SSEE, &blocksize);
    if (blocksize == 0) {
      break;
    }
    for (unsigned i=0; i < blocksize; i++) {
      if (s->unicode) {
        unsigned is_unicode, cont;
        unsigned low = 0, middle = 0, high = 0;
        sarthaka_decode_bits(s, 1, &is_unicode);
        if (is_unicode) {
          sarthaka_decode_byte(s, &low);
          sarthaka_decode_bits(s, 1, &cont);
          if (cont) {
            sarthaka_decode_byte(s, &middle);
            sarthaka_decode_bits(s, 1, &cont);
            if (cont) {
              sarthaka_decode_bits(s, 4, &high);
            }
          }
          s->pushchar(s->structarg, (unsigned)(low|(middle << 8)|(high << 16)));
        } else {
          sarthaka_decode_bits(s, 7, &byte);
          s->pushchar(s->structarg, (unsigned)byte);
        }
      } else {
        sarthaka_decode_byte(s, &byte);
        s->pushchar(s->structarg, (unsigned)byte);
      }
    }
  }
  s->pop(s->structarg, SARTHAKA_TYPE_STRING);
}

static
void sarthaka_encode_string
  (sarthaka_t* s)
{
}

static
void sarthaka_decode_array
  (sarthaka_t* s)
{
  unsigned cont;

  s->push(s->structarg, SARTHAKA_TYPE_ARRAY);
  while (!(s->eof)) {
    sarthaka_decode_bits(s, 1, &cont);
    if (cont) {
      sarthaka_decode_tuple(s);
    }
  }
  s->pop(s->structarg, SARTHAKA_TYPE_ARRAY);
}

static
void sarthaka_encode_array
  (sarthaka_t* s)
{
}

static
void sarthaka_decode_hashtable
  (sarthaka_t* s)
{
  unsigned cont;

  s->push(s->structarg, SARTHAKA_TYPE_HASHTABLE);
  while (!(s->eof)) {
    sarthaka_decode_bits(s, 1, &cont);
    if (cont) {
      sarthaka_decode_string(s);
      sarthaka_decode_tuple(s);
    }
  }
  s->pop(s->structarg, SARTHAKA_TYPE_HASHTABLE);
}

static
void sarthaka_encode_hashtable
  (sarthaka_t* s)
{
}

static
void sarthaka_decode_tuple
  (sarthaka_t* s)
{
  unsigned type;

  sarthaka_decode_type(s, &type);
  switch (type)
  {
  case SARTHAKA_TYPE_IMPLICITNULL:
  case SARTHAKA_TYPE_EXPLICITNULL:
    s->pushelt(s->structarg, type);
    break;
  case SARTHAKA_TYPE_BOOLEAN:
    sarthaka_decode_boolean(s);
    break;
  case SARTHAKA_TYPE_FLOAT:
    sarthaka_decode_float(s);
    break;
  case SARTHAKA_TYPE_INTEGER:
    sarthaka_decode_integer(s);
    break;
  case SARTHAKA_TYPE_STRING:
    sarthaka_decode_string(s);
    break;
  case SARTHAKA_TYPE_ARRAY:
    sarthaka_decode_array(s);
    break;
  case SARTHAKA_TYPE_HASHTABLE:
    sarthaka_decode_hashtable(s);
    break;
  }
}

void sarthaka_decode
  (sarthaka_t* s)
{
  s->eof = 0;
  s->push(s->structarg, SARTHAKA_TYPE_ARRAY);
  while (!(s->eof)) {
    sarthaka_decode_tuple(s);
  }
  s->pop(s->structarg, SARTHAKA_TYPE_ARRAY);
}

void sarthaka_encode
  (sarthaka_t* s)
{
  sarthaka_token_t token;

  while (1) {
    s->getnexttoken(s->tokenizerarg, &token);
    switch (token.action) {
    case SARTHAKA_TOKEN_ACTION_PUSH:
      switch (token.argument.value) {
      case SARTHAKA_TOKEN_ARG_STRING:
        sarthaka_encode_string(s);
        break;
      case SARTHAKA_TOKEN_ARG_ARRAY:
        sarthaka_encode_array(s);
        break;
      case SARTHAKA_TOKEN_ARG_HASHTABLE:
        sarthaka_encode_hashtable(s);
        break;
      }
      break;
    case SARTHAKA_TOKEN_ACTION_NULL:
      sarthaka_encode_null(s);
      break;
    case SARTHAKA_TOKEN_ACTION_BOOLEAN:
      sarthaka_encode_boolean(s, token.argument.i);
      break;
    case SARTHAKA_TOKEN_ACTION_INTEGER:
      sarthaka_encode_integer(s, token.argument.i);
      break;
    case SARTHAKA_TOKEN_ACTION_FLOAT:
      sarthaka_encode_float(s, token.argument.d);
      break;
    }
  }
}
