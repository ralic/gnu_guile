/*	Copyright (C) 1995,1996,1998 Free Software Foundation, Inc.
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this software; see the file COPYING.  If not, write to
 * the Free Software Foundation, Inc., 59 Temple Place, Suite 330,
 * Boston, MA 02111-1307 USA
 *
 * As a special exception, the Free Software Foundation gives permission
 * for additional uses of the text contained in its release of GUILE.
 *
 * The exception is that, if you link the GUILE library with other files
 * to produce an executable, this does not by itself cause the
 * resulting executable to be covered by the GNU General Public License.
 * Your use of that executable is in no way restricted on account of
 * linking the GUILE library code into it.
 *
 * This exception does not however invalidate any other reasons why
 * the executable file might be covered by the GNU General Public License.
 *
 * This exception applies only to the code released by the
 * Free Software Foundation under the name GUILE.  If you copy
 * code from other Free Software Foundation releases into a copy of
 * GUILE, as the General Public License permits, the exception does
 * not apply to the code that you add in this way.  To avoid misleading
 * anyone as to the status of such modified files, you must delete
 * this exception notice from them.
 *
 * If you write modifications of your own for GUILE, it is your choice
 * whether to permit this exception to apply to your modifications.
 * If you do not wish that, delete this exception notice.  */


#include <stdio.h>
#include "_scm.h"
#include "eval.h"
#include "chars.h"
#include "fports.h"

#include "vports.h"

#ifdef HAVE_STRING_H
#include <string.h>
#endif



/* {Ports - soft ports}
 * 
 */



static int prinsfpt SCM_P ((SCM exp, SCM port, scm_print_state *pstate));

static int 
prinsfpt (exp, port, pstate)
     SCM exp;
     SCM port;
     scm_print_state *pstate;
{
  scm_prinport (exp, port, "soft");
  return !0;
}

/* called with a single char at most.  */
static void
sfflush (SCM port)
{
  struct scm_port_table *pt = SCM_PTAB_ENTRY (port);
  SCM stream = pt->stream;

  if (pt->write_pos > pt->write_buf)
    {
      /* write the char. */
      scm_apply (SCM_VELTS (stream)[0], SCM_MAKICHR (*pt->write_buf),
		 scm_listofnull);
      pt->write_pos = pt->write_buf;
  
      /* flush the output.  */
      {
	SCM f = SCM_VELTS (stream)[2];

	if (f != SCM_BOOL_F)
	  scm_apply (f, SCM_EOL, SCM_EOL);
      }
    }
}

/* string output proc (element 1) is no longer called.  */

/* calling the flush proc (element 2) is in case old code needs it,
   but perhaps softports could the use port buffer in the same way as
   fports.  */

/* returns a single character.  */
static int 
sf_fill_buffer (SCM port)
{
  SCM p = SCM_STREAM (port);
  SCM ans;

  ans = scm_apply (SCM_VELTS (p)[3], SCM_EOL, SCM_EOL); /* get char.  */
  if (SCM_FALSEP (ans) || SCM_EOF_OBJECT_P (ans))
    return EOF;
  SCM_ASSERT (SCM_ICHRP (ans), ans, SCM_ARG1, "sf_fill_buffer");
  return SCM_ICHR (ans);
}


static int 
sfclose (SCM port)
{
  SCM p = SCM_STREAM (port);
  SCM f = SCM_VELTS (p)[4];
  if (SCM_BOOL_F == f)
    return 0;
  f = scm_apply (f, SCM_EOL, SCM_EOL);
  errno = 0;
  return SCM_BOOL_F == f ? EOF : 0;
}



SCM_PROC(s_make_soft_port, "make-soft-port", 2, 0, 0, scm_make_soft_port);

SCM 
scm_make_soft_port (pv, modes)
     SCM pv;
     SCM modes;
{
  struct scm_port_table * pt;
  SCM z;
  SCM_ASSERT (SCM_NIMP (pv) && SCM_VECTORP (pv) && 5 == SCM_LENGTH (pv), pv, SCM_ARG1, s_make_soft_port);
  SCM_ASSERT (SCM_NIMP (modes) && SCM_ROSTRINGP (modes), modes, SCM_ARG2, s_make_soft_port);
  SCM_COERCE_SUBSTR (modes);
  SCM_NEWCELL (z);
  SCM_DEFER_INTS;
  pt = scm_add_to_port_table (z);
  SCM_SETCAR (z, scm_tc16_sfport | scm_mode_bits (SCM_ROCHARS (modes)));
  SCM_SETPTAB_ENTRY (z, pt);
  SCM_SETSTREAM (z, pv);
  pt->read_buf = pt->read_pos = pt->read_end = &pt->shortbuf;
  pt->write_buf = pt->write_pos = &pt->shortbuf;
  pt->read_buf_size = pt->write_buf_size = 1;
  pt->write_end = pt->write_buf + pt->write_buf_size;
  SCM_ALLOW_INTS;
  return z;
}


static int 
noop0 (SCM port)
{
  return 0;
}


scm_ptobfuns scm_sfptob =
{
  scm_markstream,
  noop0,
  prinsfpt,
  0,
  sfflush,
  sfclose,
  sf_fill_buffer,
  0,
  0,
};



void
scm_init_vports ()
{
#include "vports.x"
}

