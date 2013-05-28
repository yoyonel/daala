/*Daala video codec
Copyright (c) 2012 Daala project contributors.  All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

- Redistributions of source code must retain the above copyright notice, this
  list of conditions and the following disclaimer.

- Redistributions in binary form must reproduce the above copyright notice,
  this list of conditions and the following disclaimer in the documentation
  and/or other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE
FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.*/

#include <stdio.h>

#include "generic_code.h"
#include "entenc.h"
#include "entdec.h"
#include "logging.h"
#include "odintrin.h"
#include "pvq_code.h"

/** Initializes the cdfs and freq counts for a model
 *
 * @param [out] model model being initialized
 */
void generic_model_init(GenericEncoder *model)
{
  int i, j;
  model->increment = 64;
  for (i=0;i<GENERIC_TABLES;i++)
  {
    for(j=0;j<16;j++)
    {
      /* FIXME: Come on, we can do better than flat initialization! */
      model->cdf[i][j] = (j+1)<<10;
    }
  }
}

/** Encodes a random variable using a "generic" model, assuming that the distribution is
 * one-sided (zero and up), has a single mode, and decays exponentially passed the mode
 *
 * @param [in,out] enc   range encoder
 * @param [in,out] model generic probability model
 * @param [in]     x     variable being encoded
 * @param [in,out] ExQ16 expectation of x (adapted)
 * @param [in]     integration integration period of ExQ16 (leaky average over 1<<integration samples)
 */
void generic_encode(od_ec_enc *enc, GenericEncoder *model, int x, int *ExQ16, int integration)
{
  int lgQ1;
  int shift;
  int id;
  ogg_uint16_t *cdf;
  int xs;

  lgQ1=logEx(*ExQ16);

  OD_LOG((OD_LOG_ENTROPY_CODER, OD_LOG_DEBUG,
          "%d %d", *ExQ16, lgQ1));
  /* If expectation is too large, shift x to ensure that
     all we have past xs=15 is the exponentially decaying tail
     of the distribution */
  shift=OD_MAXI(0,(lgQ1-5)>>1);

  /* Choose the cdf to use: we have two per "octave" of ExQ16 */
  id=OD_MINI(GENERIC_TABLES-1,lgQ1);
  cdf=model->cdf[id];

  xs=(x+(1<<shift>>1))>>shift;

  od_ec_encode_cdf(enc,OD_MINI(15,xs),cdf,16);

  if (xs>=15){
    int E;
    unsigned decay;
    /* Estimate decay based on the assumption that the distribution is close to Laplacian for large values.
       We should probably have an adaptive estimate instead. */
    E = ((*ExQ16>>8)+(1<<shift>>1))>>shift;
    decay = OD_MAXI(2,OD_MINI(254,256*E/(E+256)));

    /* Encode the tail of the distribution assuming exponential decay */
    laplace_encode_special(enc,xs-15,decay,-1);
  }

  if (shift!=0){
    int special;
    /* Because of the rounding, there's only half the number of possibilities for xs=0 */
    special=(xs==0);
    if (shift-special>0)
      od_ec_enc_bits(enc,x-(xs<<shift)+(!special<<(shift-1)),shift-special);
  }

  generic_model_update(model,ExQ16,x,xs,id,integration);
  OD_LOG((OD_LOG_ENTROPY_CODER, OD_LOG_DEBUG,
          "enc: %d %d %d %d %d %x", *ExQ16, x, shift, id, xs, enc->rng));
}


