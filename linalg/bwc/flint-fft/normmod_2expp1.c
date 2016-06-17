#ifdef  __GNUC__
#pragma GCC diagnostic ignored "-Wsign-compare"
#pragma GCC diagnostic ignored "-Wunused-parameter"
#endif
/* 
 * 
 * Copyright 2009, 2011 William Hart. All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 * 
 * 1. Redistributions of source code must retain the above copyright notice,
 * this list of conditions and the following disclaimer.
 * 
 * 2. Redistributions in binary form must reproduce the above copyright
 * notice, this list of conditions and the following disclaimer in the
 * documentation and/or other materials provided with the distribution.
 * 
 * THIS SOFTWARE IS PROVIDED BY William Hart ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN 
 * NO EVENT SHALL William Hart OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
 * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER 
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 * 
 * The views and conclusions contained in the software and documentation are
 * those of the authors and should not be interpreted as representing
 * official policies, either expressed or implied, of William Hart.
 * 
 */

#include "gmp.h"
#include "flint.h"
#include "fft.h"

/*
 * Given \code{t} a signed integer of \code{limbs + 1} limbs in twos
 * complement format, reduce \code{t} to the corresponding value modulo the
 * generalised Fermat number \code{B^limbs + 1}, where
 * \code{B = 2^FLINT_BITS}.
 * 
 */
void mpn_normmod_2expp1(mp_limb_t * t, mp_size_t limbs)
{
    mp_limb_signed_t hi = t[limbs];

    if (hi) {
	t[limbs] = 0;

	mpn_addmod_2expp1_1(t, limbs, -hi);

	/* hi will now be in [-1,1] */
	if ((hi = t[limbs])) {
	    t[limbs] = 0;

	    mpn_addmod_2expp1_1(t, limbs, -hi);

	    if (t[limbs] == ~(mp_limb_signed_t) 0) {	/* if we now have -1
							 * (very unlikely) */
		t[limbs] = 0;
		mpn_addmod_2expp1_1(t, limbs, 1);
	    }
	}
    }
}
