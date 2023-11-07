#include <stdio.h>
#include <stdint.h>
#include <inttypes.h>

static int finished = 0;
static unsigned pop_null = 0;
static unsigned toplist_ntuples = 0;
static uint8_t current_byte = 0;
static unsigned current_bit = 7;
static uint8_t last_type = 0;

#define TYPE_NULL         0
#define TYPE_EXPLICITNULL 1
#define TYPE_BOOLEAN      2
#define TYPE_INT64        3
#define TYPE_FLOAT        4
#define TYPE_STRING       5
#define TYPE_ARRAY        6
#define TYPE_HASHTABLE    7

static
void render_tuple ();

static
uint8_t read_bits
  (unsigned n)
{
  uint8_t result = 0;

  for (unsigned i=0; i < 8 && n--; i++) {
    current_bit++;
    if (current_bit > 7) {
      current_bit = 0;
      if (finished) {
        current_byte = 0;
      } else {
        int c = fgetc(stdin);
        if (c == EOF) {
          c = 0;
          finished = 1;
        }
        current_byte = c;
      }
    }
    result <<= 1;
    result |= ((current_byte >> (7 - current_bit)) & 0x01);
  }
  return result;
}

static
void render_boolean
  ()
{
  uint8_t bool = read_bits(1);
  if (bool) {
    printf("true");
  } else {
    printf("false");
  }
}

static
void render_int64
  ()
{
  int64_t n = 0;
  unsigned char* _n = (unsigned char*)&n;

  for (unsigned i=0; i < sizeof(uint64_t); i++) {
#ifdef _BIGENDIAN_
    _n[ sizeof(uint64_t)-i-1 ] = read_bits(8);
#else
    _n[ i ] = read_bits(8);
#endif
  }
  printf("%" PRId64, n);
}

static
void render_float
  ()
{
  double d = 0;
  unsigned char* _d = (unsigned char*)&d;

  for (unsigned i=0; i < sizeof(double); i++) {
    _d[ sizeof(double)-i-1 ] = read_bits(8);
  }
  printf("%f", d);
}

static
void render_escaped
  (uint8_t b)
{
  switch (b) {
  case '\a': printf("\\a"); break;
  case '\b': printf("\\b"); break;
  case '\t': printf("\\t"); break;
  case '\n': printf("\\n"); break;
  case '\v': printf("\\v"); break;
  case '\f': printf("\\f"); break;
  case '\r': printf("\\r"); break;
  default:
    printf("\\0%o", b);
  }
}

static
void render_string
  ()
{
  printf("\"");
  while (!finished) {
    uint8_t cont = read_bits(1);
    if (cont) {
      uint8_t byte = read_bits(8);
      if (byte >= 32 && byte < 127) {
        printf("%c", byte);
      } else {
        render_escaped(byte);
      }
    } else {
      break;
    }
  }
  printf("\"");
}

void render_array
  ()
{
  int first = 1;

  printf("[");
  while (!finished) {
    uint8_t cont = read_bits(1);
    if (cont) {
      if (first) {
        first = 0;
      } else {
        printf(",");
      }
      render_tuple();
    } else {
      break;
    }
  }
  printf("]");
}

void render_hashtable
  ()
{
  int first = 1;

  printf("{");
  while (!finished) {
    uint8_t cont = read_bits(1);
    if (cont) {
      if (first) {
        first = 0;
      } else {
        printf(",");
      }
      render_string();
      printf(":");
      render_tuple();
    } else {
      break;
    }
  }
  printf("}");
}

static
void render_tuple
  ()
{
  last_type = read_bits(3);
  switch (last_type) {
  case TYPE_NULL:
  case TYPE_EXPLICITNULL: printf("null"); break;
  case TYPE_BOOLEAN: render_boolean(); break;
  case TYPE_INT64: render_int64(); break;
  case TYPE_FLOAT: render_float(); break;
  case TYPE_STRING: render_string(); break;
  case TYPE_ARRAY: render_array(); break;
  case TYPE_HASHTABLE: render_hashtable(); break;
  }
}

/**
 *
 */
int main
  (int argc, char* argv[])
{
  (void)argc;
  (void)argv;

  printf("var result = [");
  while (1) {
    render_tuple();
    ++toplist_ntuples;
    if (last_type == TYPE_NULL) {
      pop_null++;
    } else {
      pop_null = 0;
    }
    if (!finished) {
      printf(",");
    } else {
      break;
    }
  }
  printf("];\n");
  toplist_ntuples -= pop_null;
  while (pop_null--) {
    printf("pop result;\n");
  }
  if (toplist_ntuples == 1) {
    printf("result = result[ 0 ];\n");
  }
  return 0;
}
