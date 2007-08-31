/* mpc.h -- Include file for mpc.

Copyright (C) 2002, 2003, 2004, 2005 Andreas Enge, Paul Zimmermann

This file is part of the MPC Library.

The MPC Library is free software; you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation; either version 2.1 of the License, or (at your
option) any later version.

The MPC Library is distributed in the hope that it will be useful, but
WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public
License for more details.

You should have received a copy of the GNU Lesser General Public License
along with the MPC Library; see the file COPYING.LIB.  If not, write to
the Free Software Foundation, Inc., 59 Temple Place - Suite 330, Boston,
MA 02111-1307, USA. */

#ifndef __MPC_H
#define __MPC_H

/* Check if stdio.h is included */
#if defined (_GMP_H_HAVE_FILE)
# define _MPC_H_HAVE_FILE 1
#endif

#include "mpfr.h"

/* Return values */

/* Transform negative to 2, positive to 1, leave 0 unchanged */
#define MPC_INEX_POS(inex) (((inex) < 0) ? 2 : ((inex) == 0) ? 0 : 1)
/* Transform 2 to negative, 1 to positive, leave 0 unchanged */
#define MPC_INEX_NEG(inex) (((inex) == 2) ? -1 : ((inex) == 0) ? 0 : 1)

#define MPC_INEX(inex_re, inex_im) \
        (MPC_INEX_POS(inex_re) | (MPC_INEX_POS(inex_im) << 2))
#define MPC_INEX_RE(inex) MPC_INEX_NEG((inex) & 3)
#define MPC_INEX_IM(inex) MPC_INEX_NEG((inex) >> 2)

/* Definition of rounding modes */

/* a complex rounding mode is just a pair of two real rounding modes
   we reserve four bits for a real rounding mode.  */
typedef int mpc_rnd_t;

#define RNDC(r1,r2) (((int)(r1)) + ((int)(r2) << 4))
#define MPC_RND_RE(x) ((mp_rnd_t)((x) & 0x0F))
#define MPC_RND_IM(x) ((mp_rnd_t)((x) >> 4))

#define MPC_RNDNN RNDC(GMP_RNDN,GMP_RNDN)
#define MPC_RNDNZ RNDC(GMP_RNDN,GMP_RNDZ)
#define MPC_RNDNU RNDC(GMP_RNDN,GMP_RNDU)
#define MPC_RNDND RNDC(GMP_RNDN,GMP_RNDD)

#define MPC_RNDZN RNDC(GMP_RNDZ,GMP_RNDN)
#define MPC_RNDZZ RNDC(GMP_RNDZ,GMP_RNDZ)
#define MPC_RNDZU RNDC(GMP_RNDZ,GMP_RNDU)
#define MPC_RNDZD RNDC(GMP_RNDZ,GMP_RNDD)

#define MPC_RNDUN RNDC(GMP_RNDU,GMP_RNDN)
#define MPC_RNDUZ RNDC(GMP_RNDU,GMP_RNDZ)
#define MPC_RNDUU RNDC(GMP_RNDU,GMP_RNDU)
#define MPC_RNDUD RNDC(GMP_RNDU,GMP_RNDD)

#define MPC_RNDDN RNDC(GMP_RNDD,GMP_RNDN)
#define MPC_RNDDZ RNDC(GMP_RNDD,GMP_RNDZ)
#define MPC_RNDDU RNDC(GMP_RNDD,GMP_RNDU)
#define MPC_RNDDD RNDC(GMP_RNDD,GMP_RNDD)


/* Definitions of types and their semantics */

typedef struct {
  mpfr_t re;
  mpfr_t im;
}
__mpc_struct;

#define MPC_RE(x) ((x)->re)
#define MPC_IM(x) ((x)->im)

typedef __mpc_struct mpc_t[1]; 
typedef __mpc_struct *mpc_ptr; 
typedef __gmp_const __mpc_struct *mpc_srcptr;

/* Prototypes: Support of K&R compiler */
#if defined (__GMP_PROTO)
# define __MPC_PROTO __GMP_PROTO
#elif defined (__STDC__) || defined (__cplusplus)
# define __MPC_PROTO(x) x
#else
# define __MPC_PROTO(x) ()
#endif

#if defined (__cplusplus)
extern "C" {
#endif

extern mp_prec_t __mpc_default_fp_bit_precision;

int  mpc_add    __MPC_PROTO ((mpc_ptr, mpc_srcptr, mpc_srcptr, mpc_rnd_t));
int  mpc_add_fr __MPC_PROTO ((mpc_ptr, mpc_srcptr, mpfr_srcptr, mpc_rnd_t));
int  mpc_add_ui __MPC_PROTO ((mpc_ptr, mpc_srcptr, unsigned long int, mpc_rnd_t));
int  mpc_sub    __MPC_PROTO ((mpc_ptr, mpc_srcptr, mpc_srcptr, mpc_rnd_t));
int  mpc_sub_ui __MPC_PROTO ((mpc_ptr, mpc_srcptr, unsigned long int, mpc_rnd_t));
int  mpc_ui_ui_sub __MPC_PROTO ((mpc_ptr, unsigned long int, unsigned long int, mpc_srcptr, mpc_rnd_t));
int  mpc_mul    __MPC_PROTO ((mpc_ptr, mpc_srcptr, mpc_srcptr, mpc_rnd_t));
int  mpc_mul_fr __MPC_PROTO ((mpc_ptr, mpc_srcptr, mpfr_srcptr, mpc_rnd_t));
int  mpc_mul_ui __MPC_PROTO ((mpc_ptr, mpc_srcptr, unsigned long int, mpc_rnd_t));
int  mpc_mul_si __MPC_PROTO ((mpc_ptr, mpc_srcptr, long int, mpc_rnd_t));
int  mpc_mul_i  __MPC_PROTO ((mpc_ptr, mpc_srcptr, int, mpc_rnd_t));
int  mpc_sqr    __MPC_PROTO ((mpc_ptr, mpc_srcptr, mpc_rnd_t));
int  mpc_div    __MPC_PROTO ((mpc_ptr, mpc_srcptr, mpc_srcptr, mpc_rnd_t));
int  mpc_div_fr __MPC_PROTO ((mpc_ptr, mpc_srcptr, mpfr_srcptr, mpc_rnd_t));
int  mpc_div_ui __MPC_PROTO ((mpc_ptr, mpc_srcptr, unsigned long int, mpc_rnd_t));
int  mpc_ui_div __MPC_PROTO ((mpc_ptr, unsigned long int, mpc_srcptr, mpc_rnd_t));
int  mpc_div_2exp __MPC_PROTO ((mpc_ptr, mpc_srcptr, unsigned long int, mpc_rnd_t));
int  mpc_mul_2exp __MPC_PROTO ((mpc_ptr, mpc_srcptr, unsigned long int, mpc_rnd_t));
int  mpc_conj  __MPC_PROTO ((mpc_ptr, mpc_srcptr, mpc_rnd_t));
int  mpc_neg   __MPC_PROTO ((mpc_ptr, mpc_srcptr, mpc_rnd_t));
int  mpc_norm  __MPC_PROTO ((mpfr_ptr, mpc_srcptr, mp_rnd_t));
int  mpc_abs   __MPC_PROTO ((mpfr_ptr, mpc_srcptr, mp_rnd_t));
int  mpc_sqrt  __MPC_PROTO ((mpc_ptr, mpc_srcptr, mpc_rnd_t));
int  mpc_set   __MPC_PROTO ((mpc_ptr, mpc_srcptr, mpc_rnd_t));
int  mpc_set_d_d __MPC_PROTO ((mpc_ptr, double, double, mpc_rnd_t));
int  mpc_set_ui_fr __MPC_PROTO ((mpc_ptr, unsigned long int, mpfr_srcptr, mpc_rnd_t));
int  mpc_set_ui_ui __MPC_PROTO ((mpc_ptr, unsigned long int, unsigned long int, mpc_rnd_t));
int  mpc_set_si_si __MPC_PROTO ((mpc_ptr, long int, long int, mpc_rnd_t));
int  mpc_cmp __MPC_PROTO ((mpc_srcptr, mpc_srcptr));
int  mpc_cmp_si_si __MPC_PROTO ((mpc_srcptr, long int, long int));
void mpc_exp __MPC_PROTO ((mpc_ptr, mpc_srcptr, mpc_rnd_t));
void mpc_sin __MPC_PROTO ((mpc_ptr, mpc_srcptr, mpc_rnd_t));
void mpc_clear __MPC_PROTO ((mpc_ptr));
void mpc_init  __MPC_PROTO ((mpc_ptr));
void mpc_random __MPC_PROTO ((mpc_ptr));
void mpc_random2 __MPC_PROTO ((mpc_ptr, mp_size_t, mp_exp_t));
void mpc_init2  __MPC_PROTO ((mpc_ptr, mp_prec_t));
void mpc_init3  __MPC_PROTO ((mpc_ptr, mp_prec_t, mp_prec_t));
void mpc_set_prec __MPC_PROTO ((mpc_ptr, mp_prec_t));
void mpc_set_default_prec __MPC_PROTO ((mp_prec_t));
mp_prec_t mpc_get_default_prec __MPC_PROTO ((void));
#ifdef _MPC_H_HAVE_FILE
size_t mpc_inp_str __MPC_PROTO ((mpc_ptr, FILE *, int, mpc_rnd_t));
size_t mpc_out_str __MPC_PROTO ((FILE *, int, size_t, mpc_srcptr, mpc_rnd_t));
#endif

#if defined (__cplusplus)
}
#endif

#define mpc_set_d(x, y, rnd) mpc_set_d_d(x, y, 0.0, rnd)
#define mpc_set_ui(x, y, rnd) mpc_set_ui_ui(x, y, 0, rnd)
#define mpc_set_si(x, y, rnd) mpc_set_si_si(x, y, 0, rnd)
#define mpc_init_set(x, y, rnd) \
 ( mpc_init(x), mpc_set((x), (y), (rnd)) )
#define mpc_init_set_ui_fr(x, y, z, rnd) \
 ( mpc_init(x), mpc_set_ui_fr((x), (y), (z), (rnd)) )
#define mpc_init_set_ui(x, y, rnd) \
 ( mpc_init(x), mpc_set_ui((x), (y), (rnd)) )
#define mpc_init_set_ui_ui(x, y, z, rnd) \
 ( mpc_init(x), mpc_set_ui_ui((x), (y), (z), (rnd)) )
#define mpc_init_set_si_si(x, y, z, rnd) \
 ( mpc_init(x), mpc_set_si_si((x), (y), (z), (rnd)) )
#define mpc_add_si(x, y, z, rnd) \
 ( (z) >= 0 ? mpc_add_ui ((x), (y), (z), (rnd)) : mpc_sub_ui ((x), (y), -(z), (rnd)) )
#define mpc_cmp_si(x, y) \
 ( mpc_cmp_si_si ((x), (y), 0) )
#define mpc_ui_sub(x, y, z, r) mpc_ui_ui_sub (x, y, 0, z, r)
#endif /* ifndef __MPC_H */ 
