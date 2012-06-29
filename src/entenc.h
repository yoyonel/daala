/* Copyright (c) 2001-2011 Timothy B. Terriberry
   Copyright (c) 2008-2009 Xiph.Org Foundation */
/*
   Redistribution and use in source and binary forms, with or without
   modification, are permitted provided that the following conditions
   are met:

   - Redistributions of source code must retain the above copyright
   notice, this list of conditions and the following disclaimer.

   - Redistributions in binary form must reproduce the above copyright
   notice, this list of conditions and the following disclaimer in the
   documentation and/or other materials provided with the distribution.

   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
   ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
   LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
   A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER
   OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
   EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
   PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
   PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
   LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
   NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
   SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
#if !defined(_entenc_H)
# define _entenc_H (1)
# include <stddef.h>
# include "entcode.h"



typedef struct od_ec_enc od_ec_enc;



/*The entropy encoder context.*/
struct od_ec_enc{
  /*Buffered output.
    This contains only the raw bits until the final call to od_ec_enc_done(),
     where all the arithmetic-coded data gets prepended to it.*/
  unsigned char *buf;
  /*The size of the buffer.*/
  ogg_uint32_t   storage;
  /*The offset at which the last byte containing raw bits was written.*/
  ogg_uint32_t   end_offs;
  /*Bits that will be read from/written at the end.*/
  od_ec_window   end_window;
  /*Number of valid bits in end_window.*/
  int            nend_bits;
  /*A buffer for output bytes with their associated carry flags.*/
  ogg_uint16_t  *precarry_buf;
  /*The size of the pre-carry buffer.*/
  ogg_uint32_t   precarry_storage;
  /*The offset at which the next entropy-coded byte will be written.*/
  ogg_uint32_t   offs;
  /*The low end of the current range.*/
  od_ec_window   low;
  /*The number of values in the current range.*/
  ogg_uint16_t   rng;
  /*The number of bits of data in the current value.*/
  ogg_int16_t    cnt;
  /*Nonzero if an error occurred.*/
  int            error;

};


/*See entenc.c for further documentation.*/


void od_ec_enc_init(od_ec_enc *_this,ogg_uint32_t _size);
void od_ec_enc_reset(od_ec_enc *_this);
void od_ec_enc_clear(od_ec_enc *_this);

void od_ec_encode_bool(od_ec_enc *_this,int _val,unsigned _fz,unsigned _ft);
void od_ec_encode_bool_q15(od_ec_enc *_this,int _val,unsigned _fz_q15);
void od_ec_encode_cdf(od_ec_enc *_this,int _s,
 const ogg_uint16_t *_cdf,int _nsyms);
void od_ec_encode_cdf_q15(od_ec_enc *_this,int _s,
 const ogg_uint16_t *_cdf,int _nsyms);
void od_ec_encode_cdf_unscaled(od_ec_enc *_this,int _s,
 const ogg_uint16_t *_cdf,int _nsyms);
void od_ec_encode_cdf_unscaled_dyadic(od_ec_enc *_this,int _s,
 const ogg_uint16_t *_cdf,int _nsyms,unsigned _ftb);

void od_ec_enc_uint(od_ec_enc *_this,ogg_uint32_t _fl,ogg_uint32_t _ft);

void od_ec_enc_bits(od_ec_enc *_this,ogg_uint32_t _fl,unsigned _ftb);

void od_ec_enc_patch_initial_bits(od_ec_enc *_this,unsigned _val,int _nbits);
unsigned char *od_ec_enc_done(od_ec_enc *_this,ogg_uint32_t *_nbytes);

int od_ec_enc_tell(od_ec_enc *_this);
ogg_uint32_t od_ec_enc_tell_frac(od_ec_enc *_this);

#endif
