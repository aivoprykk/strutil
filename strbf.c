#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

#include "strbf.h"
#include "numstr.h"
#if defined(ESP_PLATFORM)
#include "logger_common.h"
#else
#include <stdio.h>
#endif

#define isset(x) (x)
#define is_spacing(x) (*(x) == ' ' || *(x) == '\t' || *(x) == '\r' || *(x) == '\n')
#define is_spacing_safe(x) (isset(x) && is_spacing(x))

#define SB strbf_t
#ifndef BUFSIZ
#define BUFSIZ 512
#endif

SB *strbf_init(SB *sb) {
  if(!sb) return NULL;
  sb->start = calloc(BUFSIZ / 2, sizeof(char));
  sb->cur = sb->start;
  sb->end = sb->start + BUFSIZ / 2 - 1;
  sb->max = 0;
  return sb;
}

SB *strbf_inits(SB *sb, char *str, size_t len) {
  if(!sb || !str) return NULL;
  memset(str, 0, len);
  sb->start = str;
  sb->cur = sb->start;
  sb->end = 0;
  sb->max = sb->start + len - 1;
  return sb;
}

SB *strbf_reset(SB *sb) {
  if(!sb) return NULL;
  if (!sb->start)
    strbf_init(sb);
  else {
    /* Calculate correct buffer size based on type */
    size_t bufsize = sb->max ? (sb->max - sb->start + 1) : (sb->end - sb->start + 1);
    memset(sb->start, 0, bufsize);
    sb->cur = sb->start;
  }
  return sb;
}

/* sb and need may be evaluated multiple times. */
#define sb_need(sb, need)                                                      \
  do {                                                                         \
    /* Check max for fixed buffers, end for dynamic buffers */                \
    if (!(sb)->max && (size_t)((sb)->end - (sb)->cur) < (need)) {                    \
      sb_grow(sb, need);    /* Dynamic buffer needs growth */                 \
    }                                                                          \
  } while (0)

void sb_grow(SB *sb, size_t need) {
  if(!sb || !sb->start) return;

  /* Fixed-size buffer: check bounds but cannot grow */
  if(sb->max) {
    if((size_t)(sb->max - sb->cur) >= need) return;  /* Has space */
    /* Buffer overflow: cannot grow fixed buffer - data will be truncated */
#if defined(ESP_PLATFORM)
    printf("E: strbf: Fixed buffer overflow: need %zu, available %td",
             need, sb->max - sb->cur);
#endif
    return;  /* Cannot grow fixed buffers */
  }

  /* Dynamic buffer: grow to fit */
  size_t length = sb->cur - sb->start;
  size_t alloc = sb->end - sb->start;

  do {
    alloc *= 2;
  } while (alloc < length + need);

  // sb->start = (char*) realloc(sb->start, alloc + 1);
  char *data = calloc(alloc + 1, sizeof(char));
  memcpy(data, sb->start, length);
  free(sb->start);
  sb->start = data;
  sb->cur = sb->start + length;
  sb->end = sb->start + alloc;
}

void strbf_put(SB *sb, const char *bytes, size_t count) {
  if(!sb || !sb->start) return;
  if (bytes && count) {
    sb_need(sb, count);
    memcpy(sb->cur, bytes, count);
    sb->cur += count;
  }
}

void strbf_putu(SB *sb, const uint8_t *bytes, size_t count) {
  if(!sb || !sb->start) return;
  if (bytes && count) {
    sb_need(sb, count);
    memcpy(sb->cur, bytes, count);
    sb->cur += count;
  }
}

/*
#define strbf_putc(sb, c) do {         \
        if ((sb)->cur >= (sb)->end) \
            sb_grow(sb, 1);         \
        *(sb)->cur++ = (c);         \
    } while (0)
*/

void strbf_putc(SB *sb, const char c) {
  if(!sb || !sb->start) return;
  sb_need(sb, 1);
  *sb->cur++ = c;
}

SB *strbf_puts(SB *sb, const char *str) {
  if (str && *str)
    strbf_put(sb, str, strlen(str));
  return sb;
}

SB *strbf_puts_v(SB *sb, int argc, ...) {
  if(!sb) return NULL;
  va_list ptr;
  va_start(ptr, argc);
  const char *str = 0;
  register int i = 0;
  for (; i < argc; ++i) {
    str = va_arg(ptr, const char *);
    strbf_puts(sb, str);
  }
  return sb;
}

SB *strbf_sprintf(SB *sb, const char *fmt, ...) {
  if(!sb) return NULL;
  va_list ptr;
  va_start(ptr, fmt);
  size_t len = vsnprintf(0, 0, fmt, ptr); // get the length
  va_end(ptr);
  va_start(ptr, fmt);
  sb_need(sb, len);
  vsnprintf(sb->cur, len + 1, fmt, ptr);
  sb->cur += len;
  va_end(ptr);
  return sb;
}

// Buffer size requirements:
// - int64_t max:  20 chars + null = 21 bytes (-9,223,372,036,854,775,808)
// - uint64_t max: 20 chars + null = 21 bytes (18,446,744,073,709,551,615)
// - double: up to 25+ chars depending on value and precision
// - xdtostrf_b: width parameter can request large padding
// Use 32 bytes for safety margin
#define NUM_MAX_DIGITS 32

/// @brief Put a signed long value into the string buffer.
/// @param sb
/// @param val
void strbf_putl(SB *sb, int64_t val) {
  sb_need(sb, NUM_MAX_DIGITS); // Ensure enough space for int64 (needs up to 21 bytes)
  /* Check if we actually have space (fixed buffer may not grow) */
  // size_t available = sb->max ? (size_t)(sb->max - sb->cur) : (size_t)(sb->end - sb->cur);
  // if (available < 21) return; /* Not enough space, abort */
  sb->cur += xltoa(val, sb->cur);
}

/// @brief Put an unsigned long value into the string buffer.
/// @param sb
/// @param val
void strbf_putul(SB *sb, uint64_t val) {
  sb_need(sb, NUM_MAX_DIGITS); // Ensure enough space for uint64 (needs up to 21 bytes)
  /* Check if we actually have space (fixed buffer may not grow) */
  // size_t available = sb->max ? (size_t)(sb->max - sb->cur) : (size_t)(sb->end - sb->cur);
  // if (available < 21) return; /* Not enough space, abort */
  sb->cur += xultoa(val, sb->cur);
}

/// @brief  Put a double value into the string buffer with 2 decimal places.
/// @param sb
/// @param val
void strbf_putf(SB *sb, double val) {
  strbf_putfd(sb, val, 2);
}

/// @brief  Put a double value into the string buffer with specified decimal places.
/// @param sb
/// @param val
/// @param perc
void strbf_putfd(SB *sb, double val, const uint8_t perc) {
  sb_need(sb, NUM_MAX_DIGITS); // Ensure enough space for double
  /* Check if we actually have space (fixed buffer may not grow) */
  // size_t available = sb->max ? (size_t)(sb->max - sb->cur) : (size_t)(sb->end - sb->cur);
  // if (available < 25) return; /* Not enough space for worst-case double */
  sb->cur += xftoa(val, sb->cur, perc);
}

void strbf_putd_b(SB *sb, double val, const int8_t width, const uint8_t perc, const uint8_t mark) {
  size_t need = NUM_MAX_DIGITS;
  if (width > 0) {
    size_t width_need = (size_t)width + (size_t)perc + 3;
    if (width_need > need) {
      need = width_need;
    }
  }
  sb_need(sb, need); // Ensure enough space for requested format (dynamic buffers)
  /* Check if we actually have space (fixed buffer may not grow) */
  // size_t available = sb->max ? (size_t)(sb->max - sb->cur) : (size_t)(sb->end - sb->cur);
  // size_t needed = (width > 0 ? width : 10) + perc + 2; /* width + precision + sign + decimal */
  // if (available < needed) return; /* Not enough space, abort */
  sb->cur += xdtostrf_b(val, width,  perc, sb->cur, mark);
}

void strbf_putd(SB *sb, double val, const int8_t width, const uint8_t perc) {
  strbf_putd_b(sb, val, width, perc, ' ');
}

/*
    path related string manipulations
*/

static SB *_put_pathsep(SB *sb, char sep) {
  if(!sb) return NULL;
  if (!sep)
    sep = '/';
  if (sb->start && sb->cur && sb->cur > sb->start && *(sb->cur - 1) != sep)
    strbf_putc(sb, sep);
  return sb;
}

SB *strbf_put_pathsep(SB *sb) {
  if(!sb) return NULL;
  _put_pathsep(sb, 0);
  return sb;
}

SB *strbf_put_urisep(SB *sb) {
  if(!sb) return NULL;
  _put_pathsep(sb, '/');
  return sb;
}

SB *_put_path(SB *sb, const char *str, char sep) {
  if(!sb) return NULL;
  if (str) {
    if (*str == sep) {
      if (sb->cur > sb->start && *(sb->cur - 1) == sep)
        --sb->cur;
    } else {
      _put_pathsep(sb, sep);
    }
    strbf_puts(sb, str);
  }
  return sb;
}

SB *_put_path_n(SB *sb, const char *str, char sep, size_t len) {
  if(!sb) return NULL;
  if (str) {
    if (*str == sep) {
      if (sb->cur > sb->start && *(sb->cur - 1) == sep)
        --sb->cur;
    } else {
      _put_pathsep(sb, sep);
    }
    strbf_put(sb, str, len);
  }
  return sb;
}

SB *strbf_put_path(SB *sb, const char *str) { return _put_path(sb, str, '/'); }

SB *strbf_put_path_n(SB *sb, const char *str, size_t len) { return _put_path_n(sb, str, '/', len); }

SB *strbf_put_path_at(SB *sb, const char *str, size_t len) {
  if(!sb) return NULL;
  strbf_shape(sb, len);
  return strbf_put_path(sb, str);
}

SB *strbf_put_uri(SB *sb, const char *str) { return _put_path(sb, str, '/'); }

SB *strbf_put_uri_at(SB *sb, const char *str, size_t len) {
  if(!sb) return NULL;
  strbf_shape(sb, len);
  return strbf_put_uri(sb, str);
}

SB *strbf_put_path_v(SB *sb, int argc, ...) {
  if(!sb) return NULL;
  va_list ptr;
  va_start(ptr, argc);
  const char *str = 0;
  register int i = 0;
  for (; i < argc; ++i) {
    str = va_arg(ptr, const char *);
    strbf_put_path(sb, str);
  }
  return sb;
}

SB *strbf_put_uri_v(SB *sb, int argc, ...) {
  if(!sb) return NULL;
  va_list ptr;
  va_start(ptr, argc);
  const char *str = 0;
  register int i = 0;
  for (; i < argc; ++i) {
    str = va_arg(ptr, const char *);
    strbf_put_uri(sb, str);
  }
  return sb;
}

static SB *_pop_path(SB *sb, char sep) {
  if(!sb) return NULL;
  if (sb->cur) {
    if (!sep)
      sep = '/';
    char *cur = sb->cur - 1;
    if (cur && *cur == sep) {
      --cur;
    }
    while (cur && cur > sb->start && *cur != sep)
      --cur;
    if (cur > sb->start && *cur == sep) {
      strbf_pop(sb, sb->cur - cur);
    }
  }
  return sb;
}

SB *strbf_pop_path(SB *sb) { return _pop_path(sb, 0); }

SB *strbf_pop_url(SB *sb) { return _pop_path(sb, '/'); }

static SB *_insert_pathsep(SB *sb, char sep, size_t at) {
  if(!sb) return NULL;
  if (!sep)
    sep = '/';
  char *a = sb->start + at;
  if(a < sb->start || a > sb->cur) return sb;
  if (a > sb->start && *(a - 1) != sep)
    strbf_insertc(sb, sep, at++);

  return sb;
}

SB *strbf_insert_pathsep(SB *sb, size_t at) {
  if(!sb) return NULL;
  _insert_pathsep(sb, 0, at);
  return sb;
}

SB *strbf_insert_urisep(SB *sb, size_t at) {
  if(!sb) return NULL;
  _insert_pathsep(sb, '/', at);
  return sb;
}

static SB *_insert_path(SB *sb, const char *str, size_t at, char sep) {
  if(!sb) return NULL;
  if (str) {
    char *a = sb->start + at, *b = a;
    if(a < sb->start || a > sb->cur) return sb;
    if (*str == sep) {
      if (a > sb->start && *(a - 1) == sep) {
        a -= 1;
        --at;
      }
    } else {
      if (a > sb->start && *(a) == sep) {
        a += 1;
        ++at;
      }
    }
    if (a != b)
      _insert_pathsep(sb, sep, at);
    strbf_inserts(sb, str, at);
  }
  return sb;
}

SB *strbf_insert_path(SB *sb, const char *str, size_t at) {
  return _insert_path(sb, str, at, '/');
}

SB *strbf_insert_uri(SB *sb, const char *str, size_t at) {
  return _insert_path(sb, str, at, '/');
}

void strbf_concat(SB *sb, const char *str, size_t count) {
  if(!sb || !sb->cur) return;
  strbf_put(sb, str, count);
  *sb->cur = 0;
}

void strbf_concatc(SB *sb, const char c) {
  if(!sb || !sb->cur) return;
  strbf_putc(sb, c);
  *sb->cur = 0;
}

void strbf_concats(SB *sb, const char *str) {
  if (str) {
    if(!sb || !sb->cur) return;
    strbf_put(sb, str, strlen(str));
    *sb->cur = 0;
  }
}

void strbf_insert(SB *sb, const char *str, size_t after, size_t count) {
  if (str) {
    if(!sb || !sb->cur) return;
    if (!count)
      count = strlen(str);
    sb_need(sb, count);
    memmove(sb->start + after + count, sb->start + after,
            sb->cur - sb->start + after);
    memcpy(sb->start + after, str, count);
    sb->cur += count;
    //*sb->cur=0;
  }
}

void strbf_insertc(SB *sb, const char str, size_t after) {
  if (str) {
    if(!sb || !sb->cur) return;
    sb_need(sb, 1);
    memmove(sb->start + after + 1, sb->start + after,
            sb->cur - sb->start + after);
    *(sb->start + after) = str;
    sb->cur += 1;
    //*sb->cur=0;
  }
}

void strbf_inserts(SB *sb, const char *str, size_t after) {
  if (str) {
    strbf_insert(sb, str, after, strlen(str));
  }
}

void strbf_prepend(SB *sb, const char *str, size_t count) {
  if (str) {
    if(!sb || !sb->cur) return;
    if (!count)
      count = strlen(str);
    sb_need(sb, count);
    memmove(sb->start + count, sb->start, sb->cur - sb->start);
    memcpy(sb->start, str, count);
    sb->cur += count;
    //*sb->cur=0;
  }
}

void strbf_prependc(SB *sb, const char c) {
  if(!sb || !sb->start) return;
  sb_need(sb, 1);
  memmove(sb->start + 1, sb->start, sb->cur - sb->start);
  *sb->start = c;
  sb->cur += 1;
  //*sb->cur=0;
}

void strbf_prepends(SB *sb, const char *str) {
  if (str) {
    strbf_prepend(sb, str, strlen(str));
  }
}

void strbf_shift(SB *sb, size_t count) {
  if (count) {
    if(!sb || !sb->start) return;
    memmove(sb->start, sb->start + count, sb->cur - sb->start - count);
    sb->cur -= count;
  }
}

void strbf_pop(SB *sb, size_t count) {
  if (count) {
    if(!sb || !sb->start) return;
    sb->cur -= count;
  }
}

strbf_t *strbf_shape(SB *sb, size_t count) {
    if(!sb || !sb->start) return 0;
    sb->cur = sb->start + count;
    *sb->cur = 0;
  return sb;
}

void strbf_trim(SB *sb) {
  if(!sb || !sb->start) return;
  if (sb->start) {
    // Right trim
    while (sb->cur>sb->start && is_spacing((sb->cur - 1))) {
      --sb->cur;
    }
    // Left trim
    if (sb->start && is_spacing(sb->start)) {
      int i;
      for (i = 0; is_spacing((sb->start + i)); i++)
        ;
      memmove(sb->start, sb->start + i, sb->cur - sb->start - i);
      sb->cur -= i;
    }
  }
}

char *strbf_finish(const SB *sb) {
  if(!sb || !sb->start) return 0;
  *sb->cur = 0;
  if(sb->start > sb->cur || strlen(sb->start) != (size_t)(sb->cur - sb->start)) return 0;
  return sb->start;
}

char *strbf_finish_url(SB *sb) {
  _put_pathsep(sb, '/');
  return strbf_finish(sb);
}

char *strbf_get(const SB *sb) {
  if(!sb || !sb->start) return 0;
  return sb->start;
}

size_t strbf_len(SB *sb) {
  if(!sb || !sb->start) return 0;
  return sb->cur - sb->start;
}

char *strbf_cur(SB *sb) {
  if(!sb || !sb->start) return 0;
  return sb->cur;
}

void strbf_free(SB *sb) {
  if(!sb || sb->max) return;
  if (sb->start) {
    free(sb->start);
    sb->start = 0;
  }
  sb->cur = 0;
  sb->end = 0;
}

#undef SB
