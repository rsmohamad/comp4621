#ifndef GZIP_H
#define GZIP_H

#include "zlib/zlib.h"

/*
   Modified version of compress2 from zlib/compress.c
   Outputs gzip instead of deflate
*/
static int compressToGzip(dest, destLen, source, sourceLen) Bytef *dest;
uLongf *destLen;
const Bytef *source;
uLong sourceLen;
{
  z_stream stream;
  int err;
  const uInt max = (uInt)-1;
  uLong left;

  left = *destLen;
  *destLen = 0;

  stream.zalloc = (alloc_func)0;
  stream.zfree = (free_func)0;
  stream.opaque = (voidpf)0;

  // err = deflateInit(&stream, level);

  err = deflateInit2(&stream, Z_DEFAULT_COMPRESSION, Z_DEFLATED, 15 | 16, 8,
                     Z_DEFAULT_STRATEGY);

  if (err != Z_OK)
    return err;

  stream.next_out = dest;
  stream.avail_out = 0;
  stream.next_in = (z_const Bytef *)source;
  stream.avail_in = 0;

  do {
    if (stream.avail_out == 0) {
      stream.avail_out = left > (uLong)max ? max : (uInt)left;
      left -= stream.avail_out;
    }
    if (stream.avail_in == 0) {
      stream.avail_in = sourceLen > (uLong)max ? max : (uInt)sourceLen;
      sourceLen -= stream.avail_in;
    }
    err = deflate(&stream, sourceLen ? Z_NO_FLUSH : Z_FINISH);
  } while (err == Z_OK);

  *destLen = stream.total_out;
  deflateEnd(&stream);
  return err == Z_STREAM_END ? Z_OK : err;
}

#endif