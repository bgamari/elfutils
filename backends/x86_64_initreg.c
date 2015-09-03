/* Fetch live process registers from TID.
   Copyright (C) 2013 Red Hat, Inc.
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

#include <stdlib.h>
#ifdef __x86_64__
# include <sys/user.h>
# include <sys/ptrace.h>
#endif

#define BACKEND x86_64_
#include "libebl_CPU.h"

bool
x86_64_set_initial_registers_tid (pid_t tid __attribute__ ((unused)),
			  ebl_tid_registers_t *setfunc __attribute__ ((unused)),
				  void *arg __attribute__ ((unused)))
{
#if !defined(__x86_64__) || !defined(__linux__)
  return false;
#else /* __x86_64__ */
  struct user_regs_struct user_regs;
  if (ptrace (PTRACE_GETREGS, tid, NULL, &user_regs) != 0)
    return false;
  Dwarf_Word dwarf_regs[17];
  dwarf_regs[0] = user_regs.rax;
  dwarf_regs[1] = user_regs.rdx;
  dwarf_regs[2] = user_regs.rcx;
  dwarf_regs[3] = user_regs.rbx;
  dwarf_regs[4] = user_regs.rsi;
  dwarf_regs[5] = user_regs.rdi;
  dwarf_regs[6] = user_regs.rbp;
  dwarf_regs[7] = user_regs.rsp;
  dwarf_regs[8] = user_regs.r8;
  dwarf_regs[9] = user_regs.r9;
  dwarf_regs[10] = user_regs.r10;
  dwarf_regs[11] = user_regs.r11;
  dwarf_regs[12] = user_regs.r12;
  dwarf_regs[13] = user_regs.r13;
  dwarf_regs[14] = user_regs.r14;
  dwarf_regs[15] = user_regs.r15;
  dwarf_regs[16] = user_regs.rip;
  return setfunc (0, 17, dwarf_regs, arg);
#endif /* __x86_64__ */
}

bool
x86_64_set_initial_registers_local (
			  ebl_tid_registers_t *setfunc __attribute__ ((unused)),
				  void *arg __attribute__ ((unused)))
{
#if !defined(__x86_64__) || !defined(__linux__)
  return false;
#else /* __x86_64__ */
  Dwarf_Word dwarf_regs[17];
  __asm__ ("movq %%rax, 0x00(%0)\n\t"
    "movq %%rdx, 0x08(%0)\n\t"
    "movq %%rcx, 0x10(%0)\n\t"
    "movq %%rbx, 0x18(%0)\n\t"
    "movq %%rsi, 0x20(%0)\n\t"
    "movq %%rdi, 0x28(%0)\n\t"
    "movq %%rbp, 0x30(%0)\n\t"
    "movq %%rsp, 0x38(%0)\n\t"
    "movq %%r8,  0x40(%0)\n\t"
    "movq %%r9,  0x48(%0)\n\t"
    "movq %%r10, 0x50(%0)\n\t"
    "movq %%r11, 0x58(%0)\n\t"
    "movq %%r12, 0x60(%0)\n\t"
    "movq %%r13, 0x68(%0)\n\t"
    "movq %%r14, 0x70(%0)\n\t"
    "movq %%r15, 0x78(%0)\n\t"
    "lea 0(%%rip), %%rax\n\t"
    "movq %%rax, 0x80(%0)\n\t"
    :                            /* no output */
    :"r" (&dwarf_regs[0])        /* input */
    :"%rax"                      /* clobbered */
    );
  return setfunc (0, 17, dwarf_regs, arg);
#endif /* __x86_64__ */
}
