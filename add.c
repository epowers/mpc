/* mpc_add -- Add two complex numbers.

Copyright (C) 2002 Andreas Enge, Paul Zimmermann

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

#include "gmp.h"
#include "mpfr.h"
#include "mpc.h"

/* return 0 iff both the real and imaginary parts are exact */
int
mpc_add (mpc_ptr a, mpc_srcptr b, mpc_srcptr c, mp_rnd_t rnd)
{
  int inex_re, inex_im;

  inex_re = mpfr_add (MPC_RE(a), MPC_RE(b), MPC_RE(c), MPC_RND_RE(rnd));
  inex_im = mpfr_add (MPC_IM(a), MPC_IM(b), MPC_IM(c), MPC_RND_IM(rnd));

  return MPC_INEX(inex_re, inex_im);
}
