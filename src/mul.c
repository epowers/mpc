/* mpc_mul -- Multiply two complex numbers

Copyright (C) INRIA, 2002, 2004, 2005, 2008, 2009, 2010, 2011

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

#include "mpc-impl.h"

#define mpz_add_si(z,x,y) do { \
   if (y >= 0) \
      mpz_add_ui (z, x, (long int) y); \
   else \
      mpz_sub_ui (z, x, (long int) (-y)); \
   } while (0);

/* compute z=x*y when x has an infinite part */
static int
mul_infinite (mpc_ptr z, mpc_srcptr x, mpc_srcptr y)
{
   /* Let x=xr+i*xi and y=yr+i*yi; extract the signs of the operands */
   int xrs = mpfr_signbit (MPC_RE (x)) ? -1 : 1;
   int xis = mpfr_signbit (MPC_IM (x)) ? -1 : 1;
   int yrs = mpfr_signbit (MPC_RE (y)) ? -1 : 1;
   int yis = mpfr_signbit (MPC_IM (y)) ? -1 : 1;

   int u, v;

   /* compute the sign of
      u = xrs * yrs * xr * yr - xis * yis * xi * yi
      v = xrs * yis * xr * yi + xis * yrs * xi * yr
      +1 if positive, -1 if negatiye, 0 if NaN */
   if (  mpfr_nan_p (MPC_RE (x)) || mpfr_nan_p (MPC_IM (x))
      || mpfr_nan_p (MPC_RE (y)) || mpfr_nan_p (MPC_IM (y))) {
      u = 0;
      v = 0;
   }
   else if (mpfr_inf_p (MPC_RE (x))) {
      /* x = (+/-inf) xr + i*xi */
      u = (   mpfr_zero_p (MPC_RE (y))
           || (mpfr_inf_p (MPC_IM (x)) && mpfr_zero_p (MPC_IM (y)))
           || (mpfr_zero_p (MPC_IM (x)) && mpfr_inf_p (MPC_IM (y)))
           || (   (mpfr_inf_p (MPC_IM (x)) || mpfr_inf_p (MPC_IM (y)))
              && xrs*yrs == xis*yis)
           ? 0 : xrs * yrs);
      v = (   mpfr_zero_p (MPC_IM (y))
           || (mpfr_inf_p (MPC_IM (x)) && mpfr_zero_p (MPC_RE (y)))
           || (mpfr_zero_p (MPC_IM (x)) && mpfr_inf_p (MPC_RE (y)))
           || (   (mpfr_inf_p (MPC_IM (x)) || mpfr_inf_p (MPC_IM (x)))
               && xrs*yis != xis*yrs)
           ? 0 : xrs * yis);
   }
   else {
      /* x = xr + i*(+/-inf) with |xr| != inf */
      u = (   mpfr_zero_p (MPC_IM (y))
           || (mpfr_zero_p (MPC_RE (x)) && mpfr_inf_p (MPC_RE (y)))
           || (mpfr_inf_p (MPC_RE (y)) && xrs*yrs == xis*yis)
           ? 0 : -xis * yis);
      v = (   mpfr_zero_p (MPC_RE (y))
           || (mpfr_zero_p (MPC_RE (x)) && mpfr_inf_p (MPC_IM (y)))
           || (mpfr_inf_p (MPC_IM (y)) && xrs*yis != xis*yrs)
           ? 0 : xis * yrs);
   }

   if (u == 0 && v == 0) {
      /* Naive result is NaN+i*NaN. Obtain an infinity using the algorithm
         given in Annex G.5.1 of the ISO C99 standard */
      int xr = (mpfr_zero_p (MPC_RE (x)) || mpfr_nan_p (MPC_RE (x)) ? 0
                : (mpfr_inf_p (MPC_RE (x)) ? 1 : 0));
      int xi = (mpfr_zero_p (MPC_IM (x)) || mpfr_nan_p (MPC_IM (x)) ? 0
                : (mpfr_inf_p (MPC_IM (x)) ? 1 : 0));
      int yr = (mpfr_zero_p (MPC_RE (y)) || mpfr_nan_p (MPC_RE (y)) ? 0 : 1);
      int yi = (mpfr_zero_p (MPC_IM (y)) || mpfr_nan_p (MPC_IM (y)) ? 0 : 1);
      if (mpc_inf_p (y)) {
         yr = mpfr_inf_p (MPC_RE (y)) ? 1 : 0;
         yi = mpfr_inf_p (MPC_IM (y)) ? 1 : 0;
      }

      u = xrs * xr * yrs * yr - xis * xi * yis * yi;
      v = xrs * xr * yis * yi + xis * xi * yrs * yr;
   }

   if (u == 0)
      mpfr_set_nan (MPC_RE (z));
   else
      mpfr_set_inf (MPC_RE (z), u);

   if (v == 0)
      mpfr_set_nan (MPC_IM (z));
   else
      mpfr_set_inf (MPC_IM (z), v);

   return MPC_INEX (0, 0); /* exact */
}


/* compute z = x*y for Im(y) == 0 */
static int
mul_real (mpc_ptr z, mpc_srcptr x, mpc_srcptr y, mpc_rnd_t rnd)
{
   int xrs, xis, yrs, yis;
   int inex;

   /* save signs of operands */
   xrs = MPFR_SIGNBIT (MPC_RE (x));
   xis = MPFR_SIGNBIT (MPC_IM (x));
   yrs = MPFR_SIGNBIT (MPC_RE (y));
   yis = MPFR_SIGNBIT (MPC_IM (y));

   inex = mpc_mul_fr (z, x, MPC_RE (y), rnd);
   /* Signs of zeroes may be wrong. Their correction does not change the
      inexact flag. */
   if (mpfr_zero_p (MPC_RE (z)))
      mpfr_setsign (MPC_RE (z), MPC_RE (z), MPC_RND_RE(rnd) == GMP_RNDD
                     || (xrs != yrs && xis == yis), GMP_RNDN);
   if (mpfr_zero_p (MPC_IM (z)))
      mpfr_setsign (MPC_IM (z), MPC_IM (z), MPC_RND_IM (rnd) == GMP_RNDD
                     || (xrs != yis && xis != yrs), GMP_RNDN);

   return inex;
}


/* compute z = x*y for Re(y) == 0, and Im(x) != 0 and Im(y) != 0 */
static int
mul_imag (mpc_ptr z, mpc_srcptr x, mpc_srcptr y, mpc_rnd_t rnd)
{
   int sign;
   int inex_re, inex_im;
   int overlap = z == x || z == y;
   mpc_t rop;

   if (overlap)
      mpc_init3 (rop, MPC_PREC_RE (z), MPC_PREC_IM (z));
   else
      rop [0] = z[0];

   sign =    (MPFR_SIGNBIT (MPC_RE (y)) != MPFR_SIGNBIT (MPC_IM (x)))
          && (MPFR_SIGNBIT (MPC_IM (y)) != MPFR_SIGNBIT (MPC_RE (x)));

   inex_re = -mpfr_mul (MPC_RE (rop), MPC_IM (x), MPC_IM (y),
                        INV_RND (MPC_RND_RE (rnd)));
   mpfr_neg (MPC_RE (rop), MPC_RE (rop), GMP_RNDN); /* exact */
   inex_im = mpfr_mul (MPC_IM (rop), MPC_RE (x), MPC_IM (y),
                       MPC_RND_IM (rnd));
   mpc_set (z, rop, MPC_RNDNN);

   /* Sign of zeroes may be wrong (note that Re(z) cannot be zero) */
   if (mpfr_zero_p (MPC_IM (z)))
      mpfr_setsign (MPC_IM (z), MPC_IM (z), MPC_RND_IM (rnd) == GMP_RNDD
                     || sign, GMP_RNDN);

   if (overlap)
      mpc_clear (rop);

   return MPC_INEX (inex_re, inex_im);
}


/* compute z=x*y by the schoolbook method, where x and y are assumed to be
   finite and different, and where direct application of the formulae leads
   to an overflow in at least one part                                      */
static int
mul_naive_overflow (mpc_ptr z, mpc_srcptr x, mpc_srcptr y, mpc_rnd_t rnd)
{
   int overlap, inex_re, inex_im, signu, signv;
   mpfr_t u, v;
   mpc_t rop;
   mpfr_prec_t prec;

   overlap = (z == x) || (z == y);
   if (overlap)
      mpc_init3 (rop, MPC_PREC_RE (z), MPC_PREC_IM (z));
   else
      rop [0] = z [0];

   prec = MPC_MAX_PREC (x) + MPC_MAX_PREC (y);
   mpfr_init2 (u, prec);
   mpfr_init2 (v, prec);

   /* Re(z) = Re(x)*Re(y) - Im(x)*Im(y) */
   mpfr_mul (u, MPC_RE (x), MPC_RE (y), GMP_RNDN);
   mpfr_mul (v, MPC_IM (x), MPC_IM (y), GMP_RNDN);
   signu = (mpfr_signbit (u) ? -1 : 1);
   signv = (mpfr_signbit (v) ? -1 : 1);
   if (mpfr_number_p (u)) {
      if (mpfr_number_p (v))
         inex_re = mpfr_sub (MPC_RE (rop), u, v, MPC_RND_RE (rnd));
      else {
         mpfr_set_inf (MPC_RE (rop), -signv);
         inex_re = -signv;
      }
   }
   else if (mpfr_number_p (v) || signu != signv) {
      mpfr_set_inf (MPC_RE (rop), signu);
      inex_re = signu;
   }
   else {
      /* the difficult case |u|, |v| == inf and signu == signv;
         redo the computations with mpz_t exponents             */
      mpfr_t xr, xi, yr, yi;
      mpz_t eu, ev;

      /* Write x=xr+i*xi, y=yr+i*yi; normalise the real and imaginary parts
         by shifting and keep track of the shifts in the exponents of u and v */
      mpz_init (eu);
      mpz_init (ev);
      mpfr_init2 (xr, MPC_PREC_RE (x));
      mpfr_init2 (xi, MPC_PREC_IM (x));
      mpfr_init2 (yr, MPC_PREC_RE (y));
      mpfr_init2 (yi, MPC_PREC_IM (y));
      mpfr_set (xr, MPC_RE (x), GMP_RNDN);
      mpz_set_si (eu, (long int) mpfr_get_exp (xr));
      mpfr_set_exp (xr, (mp_prec_t) 0);
      mpfr_set (yr, MPC_RE (y), GMP_RNDN);
      mpz_add_si (eu, eu, mpfr_get_exp (yr));
      mpfr_set_exp (yr, (mp_prec_t) 0);
      mpfr_set (xi, MPC_IM (x), GMP_RNDN);
      mpz_set_si (ev, (long int) mpfr_get_exp (xi));
      mpfr_set_exp (xi, (mp_prec_t) 0);
      mpfr_set (yi, MPC_IM (y), GMP_RNDN);
      mpz_add_si (ev, ev, mpfr_get_exp (yi));
      mpfr_set_exp (yi, (mp_prec_t) 0);

      /* recompute u and v and move exponents to eu and ev */
      mpfr_mul (u, xr, yr, GMP_RNDN);
      mpz_add_si (eu, eu, (long int) mpfr_get_exp (u));
      mpfr_set_exp (u, 0);
      mpfr_mul (v, xi, yi, GMP_RNDN);
      mpz_add_si (ev, ev, (long int) mpfr_get_exp (v));
      mpfr_set_exp (v, 0);

      if (mpz_cmp (eu, ev) > 0) {
         mpfr_set_inf (MPC_RE (rop), signu);
         inex_re = signu;
      }
      else if (mpz_cmp (eu, ev) < 0) {
         mpfr_set_inf (MPC_RE (rop), -signv);
         inex_re = -signv;
      }
      else /* both exponents are equal */ {
         inex_re = mpfr_sub (MPC_RE (rop), u, v, MPC_RND_RE (rnd));
         /* FIXME: shift by eu */
      }

      mpz_clear (eu);
      mpz_clear (ev);
      mpfr_clear (xr);
      mpfr_clear (xi);
      mpfr_clear (yr);
      mpfr_clear (yi);
   }

   /* Im(z) = Re(x)*Im(y) + Im(x)*Re(y) */
   mpfr_mul (u, MPC_RE (x), MPC_IM (y), GMP_RNDN);
   mpfr_mul (v, MPC_IM (x), MPC_RE (y), GMP_RNDN);
   signu = (mpfr_signbit (u) ? -1 : 1);
   signv = (mpfr_signbit (v) ? -1 : 1);
   if (mpfr_number_p (u)) {
      if (mpfr_number_p (v))
         inex_im = mpfr_add (MPC_IM (rop), u, v, MPC_RND_IM (rnd));
      else {
         mpfr_set_inf (MPC_IM (rop), signv);
         inex_im = signv;
      }
   }
   else if (mpfr_number_p (v) || signu == signv) {
      mpfr_set_inf (MPC_IM (rop), signu);
      inex_im = signu;
   }
   else {
      /* the difficult case; FIXME */
      inex_im = mpfr_add (MPC_IM (rop), u, v, MPC_RND_IM (rnd));
   }

   mpfr_clear (u);
   mpfr_clear (v);
   mpc_set (z, rop, MPC_RNDNN); /* exact */
   if (overlap)
      mpc_clear (rop);

   return MPC_INEX (inex_re, inex_im);
}


/* computes z=x*y by the schoolbook method, where x and y are assumed
   to be finite and different                                        */
int
mpc_mul_naive (mpc_ptr z, mpc_srcptr x, mpc_srcptr y, mpc_rnd_t rnd)
{
   int overlap, inex_re, inex_im;
   mpfr_t v;
   mpc_t rop;

   overlap = (z == x) || (z == y);
   if (overlap)
      mpc_init3 (rop, MPC_PREC_RE (z), MPC_PREC_IM (z));
   else
      rop [0] = z [0];

   mpfr_init2 (v, MPC_PREC_IM (x) + MPC_MAX_PREC (y));

   /* Re(z) = Re(x)*Re(y) - Im(x)*Im(y) */
   mpfr_mul (v, MPC_IM (x), MPC_IM (y), GMP_RNDN); /* exact */
   inex_re = mpfr_fms (MPC_RE (rop), MPC_RE (x), MPC_RE (y), v, MPC_RND_RE (rnd));

   /* Im(z) = Re(x)*Im(y) + Im(x)*Re(y) */
   mpfr_mul (v, MPC_IM (x), MPC_RE (y), GMP_RNDN); /* exact */
   inex_im = mpfr_fma (MPC_IM (rop), MPC_RE (x), MPC_IM (y), v, MPC_RND_IM(rnd));

   mpfr_clear (v);
   mpc_set (z, rop, MPC_RNDNN); /* exact */
   if (overlap)
      mpc_clear (rop);

   if (mpc_fin_p (z))
      return MPC_INEX (inex_re, inex_im);
   else /* intermediate overflow */
      return mul_naive_overflow (z, x, y, rnd);
}


/* Karatsuba multiplication, with 3 real multiplies */
int
mpc_mul_karatsuba (mpc_ptr rop, mpc_srcptr op1, mpc_srcptr op2, mpc_rnd_t rnd)
{
  mpfr_srcptr a, b, c, d;
  int mul_i, ok, inexact, mul_a, mul_c, inex_re = 0, inex_im = 0, sign_x, sign_u;
  mpfr_t u, v, w, x;
  mpfr_prec_t prec, prec_re, prec_u, prec_v, prec_w;
  mpfr_rnd_t rnd_re, rnd_u;
  int overlap;
     /* true if rop == op1 or rop == op2 */
  mpc_t result;
     /* overlap is quite difficult to handle, because we have to tentatively
        round the variable u in the end to either the real or the imaginary
        part of rop (it is not possible to tell now whether the real or
        imaginary part is used). If this fails, we have to start again and
        need the correct values of op1 and op2.
        So we just create a new variable for the result in this case. */
  int loop;
  const int MAX_MUL_LOOP = 1;

  overlap = (rop == op1) || (rop == op2);
  if (overlap)
     mpc_init3 (result, MPC_PREC_RE (rop), MPC_PREC_IM (rop));
  else
     result [0] = rop [0];

  a = MPC_RE(op1);
  b = MPC_IM(op1);
  c = MPC_RE(op2);
  d = MPC_IM(op2);

  /* (a + i*b) * (c + i*d) = [ac - bd] + i*[ad + bc] */

  mul_i = 0; /* number of multiplications by i */
  mul_a = 1; /* implicit factor for a */
  mul_c = 1; /* implicit factor for c */

  if (mpfr_cmp_abs (a, b) < 0)
    {
      MPFR_SWAP (a, b);
      mul_i ++;
      mul_a = -1; /* consider i * (a+i*b) = -b + i*a */
    }

  if (mpfr_cmp_abs (c, d) < 0)
    {
      MPFR_SWAP (c, d);
      mul_i ++;
      mul_c = -1; /* consider -d + i*c instead of c + i*d */
    }

  /* find the precision and rounding mode for the new real part */
  if (mul_i % 2)
    {
      prec_re = MPC_PREC_IM(rop);
      rnd_re = MPC_RND_IM(rnd);
    }
  else /* mul_i = 0 or 2 */
    {
      prec_re = MPC_PREC_RE(rop);
      rnd_re = MPC_RND_RE(rnd);
    }

  if (mul_i)
    rnd_re = INV_RND(rnd_re);

  /* now |a| >= |b| and |c| >= |d| */
  prec = MPC_MAX_PREC(rop);

  mpfr_init2 (u, 2);
  mpfr_init2 (v, prec_v = mpfr_get_prec (a) + mpfr_get_prec (d));
  mpfr_init2 (w, prec_w = mpfr_get_prec (b) + mpfr_get_prec (c));
  mpfr_init2 (x, 2);

  mpfr_mul (v, a, d, GMP_RNDN); /* exact */
  if (mul_a == -1)
    mpfr_neg (v, v, GMP_RNDN);

  mpfr_mul (w, b, c, GMP_RNDN); /* exact */
  if (mul_c == -1)
    mpfr_neg (w, w, GMP_RNDN);

  /* compute sign(v-w) */
  sign_x = mpfr_cmp_abs (v, w);
  if (sign_x > 0)
    sign_x = 2 * mpfr_sgn (v) - mpfr_sgn (w);
  else if (sign_x == 0)
    sign_x = mpfr_sgn (v) - mpfr_sgn (w);
  else
    sign_x = mpfr_sgn (v) - 2 * mpfr_sgn (w);

   sign_u = mul_a * mpfr_sgn (a) * mul_c * mpfr_sgn (c);

   if (sign_x * sign_u < 0)
    {  /* swap inputs */
      MPFR_SWAP (a, c);
      MPFR_SWAP (b, d);
      mpfr_swap (v, w);
      { int tmp; tmp = mul_a; mul_a = mul_c; mul_c = tmp; }
      sign_x = - sign_x;
    }

   /* now sign_x * sign_u >= 0 */
   loop = 0;
   do
     {
        loop++;
         /* the following should give failures with prob. <= 1/prec */
         prec += mpc_ceil_log2 (prec) + 3;

         mpfr_set_prec (u, prec_u = prec);
         mpfr_set_prec (x, prec);

         /* first compute away(b +/- a) and store it in u */
         inexact = (mul_a == -1 ?
                    ROUND_AWAY (mpfr_sub (u, b, a, MPFR_RNDA), u) :
                    ROUND_AWAY (mpfr_add (u, b, a, MPFR_RNDA), u));

         /* then compute away(+/-c - d) and store it in x */
         inexact |= (mul_c == -1 ?
                     ROUND_AWAY (mpfr_add (x, c, d, MPFR_RNDA), x) :
                     ROUND_AWAY (mpfr_sub (x, c, d, MPFR_RNDA), x));
         if (mul_c == -1)
           mpfr_neg (x, x, GMP_RNDN);

         if (inexact == 0)
            mpfr_prec_round (u, prec_u = 2 * prec, GMP_RNDN);

         /* compute away(u*x) and store it in u */
         inexact |= ROUND_AWAY (mpfr_mul (u, u, x, MPFR_RNDA), u);
            /* (a+b)*(c-d) */

	 /* if all computations are exact up to here, it may be that
	    the real part is exact, thus we need if possible to
	    compute v - w exactly */
	 if (inexact == 0)
	   {
	     mpfr_prec_t prec_x;
             if (mpfr_zero_p(v))
               prec_x = prec_w;
             else if (mpfr_zero_p(w))
               prec_x = prec_v;
             else
                 prec_x = SAFE_ABS (mpfr_exp_t, mpfr_get_exp (v) - mpfr_get_exp (w))
                          + MPC_MAX (prec_v, prec_w) + 1;
                 /* +1 is necessary for a potential carry */
	     /* ensure we do not use a too large precision */
	     if (prec_x > prec_u)
               prec_x = prec_u;
	     if (prec_x > prec)
	       mpfr_prec_round (x, prec_x, GMP_RNDN);
	   }

         rnd_u = (sign_u > 0) ? GMP_RNDU : GMP_RNDD;
         inexact |= mpfr_sub (x, v, w, rnd_u); /* ad - bc */

         /* in case u=0, ensure that rnd_u rounds x away from zero */
         if (mpfr_sgn (u) == 0)
           rnd_u = (mpfr_sgn (x) > 0) ? GMP_RNDU : GMP_RNDD;
         inexact |= mpfr_add (u, u, x, rnd_u); /* ac - bd */

         ok = inexact == 0 ||
           mpfr_can_round (u, prec_u - 3, rnd_u, GMP_RNDZ,
                           prec_re + (rnd_re == GMP_RNDN));
         /* this ensures both we can round correctly and determine the correct
            inexact flag (for rounding to nearest) */
     }
   while (!ok && loop <= MAX_MUL_LOOP);
      /* after MAX_MUL_LOOP rounds, use mpc_naive instead */

   if (ok) {
      /* if inexact is zero, then u is exactly ac-bd, otherwise fix the sign
         of the inexact flag for u, which was rounded away from ac-bd */
      if (inexact != 0)
      inexact = mpfr_sgn (u);

      if (mul_i == 0)
      {
         inex_re = mpfr_set (MPC_RE(result), u, MPC_RND_RE(rnd));
         if (inex_re == 0)
            {
            inex_re = inexact; /* u is rounded away from 0 */
            inex_im = mpfr_add (MPC_IM(result), v, w, MPC_RND_IM(rnd));
            }
         else
            inex_im = mpfr_add (MPC_IM(result), v, w, MPC_RND_IM(rnd));
      }
      else if (mul_i == 1) /* (x+i*y)/i = y - i*x */
      {
         inex_im = mpfr_neg (MPC_IM(result), u, MPC_RND_IM(rnd));
         if (inex_im == 0)
            {
            inex_im = -inexact; /* u is rounded away from 0 */
            inex_re = mpfr_add (MPC_RE(result), v, w, MPC_RND_RE(rnd));
            }
         else
            inex_re = mpfr_add (MPC_RE(result), v, w, MPC_RND_RE(rnd));
      }
      else /* mul_i = 2, z/i^2 = -z */
      {
         inex_re = mpfr_neg (MPC_RE(result), u, MPC_RND_RE(rnd));
         if (inex_re == 0)
            {
            inex_re = -inexact; /* u is rounded away from 0 */
            inex_im = -mpfr_add (MPC_IM(result), v, w,
                                 INV_RND(MPC_RND_IM(rnd)));
            mpfr_neg (MPC_IM(result), MPC_IM(result), MPC_RND_IM(rnd));
            }
         else
            {
            inex_im = -mpfr_add (MPC_IM(result), v, w,
                                 INV_RND(MPC_RND_IM(rnd)));
            mpfr_neg (MPC_IM(result), MPC_IM(result), MPC_RND_IM(rnd));
            }
      }

      mpc_set (rop, result, MPC_RNDNN);
   }

   mpfr_clear (u);
   mpfr_clear (v);
   mpfr_clear (w);
   mpfr_clear (x);
   if (overlap)
      mpc_clear (result);

   if (ok)
      return MPC_INEX(inex_re, inex_im);
   else
      return mpc_mul_naive (rop, op1, op2, rnd);
}


int
mpc_mul (mpc_ptr a, mpc_srcptr b, mpc_srcptr c, mpc_rnd_t rnd)
{
   /* Conforming to ISO C99 standard (G.5.1 multiplicative operators),
      infinities are treated specially if both parts are NaN when computed
      naively. */
   if (mpc_inf_p (b))
      return mul_infinite (a, b, c);
   if (mpc_inf_p (c))
      return mul_infinite (a, c, b);

   /* NaN contamination of both parts in result */
   if (mpfr_nan_p (MPC_RE (b)) || mpfr_nan_p (MPC_IM (b))
       || mpfr_nan_p (MPC_RE (c)) || mpfr_nan_p (MPC_IM (c))) {
      mpfr_set_nan (MPC_RE (a));
      mpfr_set_nan (MPC_IM (a));
      return MPC_INEX (0, 0);
   }

   /* check for real multiplication */
   if (mpfr_zero_p (MPC_IM (b)))
      return mul_real (a, c, b, rnd);
   if (mpfr_zero_p (MPC_IM (c)))
      return mul_real (a, b, c, rnd);

   /* check for purely imaginary multiplication */
   if (mpfr_zero_p (MPC_RE (b)))
      return mul_imag (a, c, b, rnd);
   if (mpfr_zero_p (MPC_RE (c)))
      return mul_imag (a, b, c, rnd);

   /* Check if b==c and call mpc_sqr in this case, to make sure            */
   /* mpc_mul(a,b,b) behaves exactly like mpc_sqr(a,b) concerning          */
   /* internal overflows etc.                                              */
   if (mpc_cmp (b, c) == 0)
      return mpc_sqr (a, b, rnd);

   /* If the real and imaginary part of one argument have a very different */
   /* exponent, it is not reasonable to use Karatsuba multiplication.      */
   if (   SAFE_ABS (mpfr_exp_t,
                     mpfr_get_exp (MPC_RE (b)) - mpfr_get_exp (MPC_IM (b)))
         > (mpfr_exp_t) MPC_MAX_PREC (b) / 2
      || SAFE_ABS (mpfr_exp_t,
                     mpfr_get_exp (MPC_RE (c)) - mpfr_get_exp (MPC_IM (c)))
         > (mpfr_exp_t) MPC_MAX_PREC (c) / 2)
      return mpc_mul_naive (a, b, c, rnd);
   else
      return ((MPC_MAX_PREC(a)
               <= (mpfr_prec_t) MUL_KARATSUBA_THRESHOLD * BITS_PER_MP_LIMB)
            ? mpc_mul_naive : mpc_mul_karatsuba) (a, b, c, rnd);
}
