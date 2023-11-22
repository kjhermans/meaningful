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

#ifndef _SARTHAKA_H_
#define _SARTHAKA_H_

#include <inttypes.h>

typedef struct sarthaka_token
{
  unsigned          action;
#define SARTHAKA_TOKEN_ACTION_PUSH      1
#define SARTHAKA_TOKEN_ACTION_NULL      2
#define SARTHAKA_TOKEN_ACTION_BOOLEAN   3
#define SARTHAKA_TOKEN_ACTION_INTEGER   4
#define SARTHAKA_TOKEN_ACTION_FLOAT     5
  union {
#define SARTHAKA_TOKEN_ARG_STRING       1
#define SARTHAKA_TOKEN_ARG_ARRAY        2
#define SARTHAKA_TOKEN_ARG_HASHTABLE    3
    unsigned          value;
    int64_t           i;
    double            d;
    struct {
      char*             buf;
      unsigned          len;
    }                 string;
  }                 argument;
}
sarthaka_token_t;

#define SARTHAKA_TYPE_IMPLICITNULL      0
#define SARTHAKA_TYPE_EXPLICITNULL      1
#define SARTHAKA_TYPE_BOOLEAN           2
#define SARTHAKA_TYPE_FLOAT             3
#define SARTHAKA_TYPE_INTEGER           4
#define SARTHAKA_TYPE_STRING            5
#define SARTHAKA_TYPE_ARRAY             6
#define SARTHAKA_TYPE_HASHTABLE         7

#define SARTHAKA_STRING_SSEE            10 // string enc exp (number of bits)
#define SARTHAKA_STRING_BLOCKSIZE       ((1<<(SARTHAKA_SSEE-1))-1)

typedef struct
{
  int               unicode; // boolean

  void            (*readbit)(void* arg, unsigned* bit, unsigned* eof);
  void*             readarg;

  void            (*writebit)(void* arg, unsigned bit);
  void*             writearg;

  void            (*push)(void* arg, unsigned type);
  void            (*pop)(void* arg, unsigned type);
  void            (*pushelt)(void* arg, unsigned type, ...);
  void            (*pushchar)(void* arg, unsigned chr);
  void*             structarg;

  void            (*getnexttoken)(void* arg, sarthaka_token_t* tok);
  void*             tokenizerarg;

/** Below here is not for the API user to fill in and should be zero **/

  int               eof;
}
sarthaka_t;

extern
void sarthaka_decode
  (sarthaka_t* s);

extern
void sarthaka_encode
  (sarthaka_t* s);

#endif
