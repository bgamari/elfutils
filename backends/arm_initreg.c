/* Fetch live process registers from TID.
   Copyright (C) 2014 Red Hat, Inc.
   This file is part of elfutils.

   This file is free software; you can redistribute it and/or modify
   it under the terms of either

     * the GNU Lesser General Public License as published by the Free
       Software Foundation; either version 3 of the License, or (at
       your option) any later version

   or

     * the GNU General Public License as published by the Free
       Software Foundation; either version 2 of the License, or (at
       your option) any later version

   or both in parallel, as here.

   elfutils is distributed in the hope that it will be useful, but
   WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.

   You should have received copies of the GNU General Public License and
   the GNU Lesser General Public License along with this program.  If
   not, see <http://www.gnu.org/licenses/>.  */

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#if defined __arm__
# include <sys/types.h>
# include <sys/user.h>
# include <sys/ptrace.h>
#endif

#ifdef __aarch64__
# include <linux/uio.h>
# include <sys/user.h>
# include <sys/ptrace.h>
/* Deal with old glibc defining user_pt_regs instead of user_regs_struct.  */
# ifndef HAVE_SYS_USER_REGS
#  define user_regs_struct user_pt_regs
# endif
#endif

#define BACKEND arm_
#include "libebl_CPU.h"

bool
arm_set_initial_registers_tid (pid_t tid __attribute__ ((unused)),
			  ebl_tid_registers_t *setfunc __attribute__ ((unused)),
			       void *arg __attribute__ ((unused)))
{
#if !defined __arm__ && !defined __aarch64__
  return false;
#else	/* __arm__ || __aarch64__ */
#if defined __arm__
  struct user_regs user_regs;
  if (ptrace (PTRACE_GETREGS, tid, NULL, &user_regs) != 0)
    return false;

  Dwarf_Word dwarf_regs[16];
  /* R0..R12 SP LR PC */
  for (int i = 0; i < 16; i++)
    dwarf_regs[i] = user_regs.uregs[i];

  return setfunc (0, 16, dwarf_regs, arg);
#elif defined __aarch64__
  /* Compat mode: arm compatible code running on aarch64 */
  int i;
  struct user_regs_struct gregs;
  struct iovec iovec;
  iovec.iov_base = &gregs;
  iovec.iov_len = sizeof (gregs);
  if (ptrace (PTRACE_GETREGSET, tid, NT_PRSTATUS, &iovec) != 0)
    return false;

  Dwarf_Word dwarf_regs[16];
  /* R0..R12 SP LR PC, encoded as 32 bit quantities */
  uint32_t *u32_ptr = (uint32_t *) &gregs.regs[0];
  for (i = 0; i < 16; i++)
    dwarf_regs[i] = u32_ptr[i];

  return setfunc (0, 16, dwarf_regs, arg);
#else
# error "source file error, it cannot happen"
#endif
#endif
}

bool
arm_set_initial_registers_local (
			  ebl_tid_registers_t *setfunc __attribute__ ((unused)),
				  void *arg __attribute__ ((unused)))
{
#if !defined(__arm__) || !defined(__aarch64__)
  return false;
#else /* __arm__ */
  Dwarf_Word dwarf_regs[16];
  __asm__ (
    "str r0,  [%0, 0x00]\n\t"
    "str r1,  [%0, 0x08]\n\t"
    "str r2,  [%0, 0x10]\n\t"
    "str r3,  [%0, 0x18]\n\t"
    "str r4,  [%0, 0x20]\n\t"
    "str r5,  [%0, 0x28]\n\t"
    "str r6,  [%0, 0x30]\n\t"
    "str r7,  [%0, 0x38]\n\t"
    "str r8,  [%0, 0x40]\n\t"
    "str r9,  [%0, 0x48]\n\t"
    "str r10, [%0, 0x50]\n\t"
    "str r11, [%0, 0x58]\n\t"
    "str r12, [%0, 0x60]\n\t"
    "str r13, [%0, 0x68]\n\t"
    "str r14, [%0, 0x70]\n\t"
    "str r15, [%0, 0x78]\n\t"
    :                            /* no output */
    :"r" (&dwarf_regs[0])        /* input */
    :"%r0"                       /* clobbered */
    );
  if (! setfunc (0, 17, dwarf_regs, arg))
    return false;

  /* Explicitly set pc, to signal we are done setting registers.  */
  return setfunc (-1, 1, &dwarf_regs[16], arg);
#endif /* __arm__ */
}
