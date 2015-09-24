/* Get Dwarf Frame state for local process.
   Copyright (C) 2015 Red Hat, Inc.
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

#include "libdwflP.h"
#include <sys/ptrace.h>
#include <sys/wait.h>
#include <dirent.h>
#include <sys/syscall.h>
#include <unistd.h>
#include <fcntl.h>

#ifndef MAX
# define MAX(a, b) ((a) > (b) ? (a) : (b))
#endif

#ifdef __linux__

static bool
local_memory_read (Dwfl *dwfl __attribute__ ((unused)),
		   Dwarf_Addr addr, Dwarf_Word *result,
		   void *arg __attribute__ ((unused)))
{
  *result = *(Dwarf_Word *) addr;
  return true;
}

static pid_t
local_next_thread (Dwfl *dwfl __attribute__ ((unused)),
		   void *dwfl_arg __attribute ((unused)),
		   void **thread_argp __attribute__ ((unused)))
{
  /* there is only the current thread */
  if (*thread_argp != NULL)
    return 0;

  *thread_argp = dwfl_arg;
  return dwfl_pid(dwfl);
}

static bool
local_getthread (Dwfl *dwfl __attribute__ ((unused)),
		 pid_t tid __attribute__ ((unused)),
		 void *dwfl_arg, void **thread_argp)
{
  *thread_argp = dwfl_arg;
  return true;
}

/* initial unwind callback.  */

static int
initial_unwind_callback (Dwfl_Frame *state, void *arg)
{
  bool *mod_seen = (bool *) arg;

  Dwfl *dwfl = dwfl_thread_dwfl (dwfl_frame_thread (state));
  Dwfl_Module *mod_here;
pc_here:
  mod_here = dwfl_addrmodule (dwfl, (Dwarf_Addr) &&pc_here);

  Dwarf_Addr pc;
  if (! dwfl_frame_pc (state, &pc, NULL))
    return -1;

  Dwfl_Module *mod_frame = dwfl_addrmodule (dwfl, pc);
  if (mod_here == mod_frame)
    {
      *mod_seen = true;
      return DWARF_CB_OK;
    }
  else if (*mod_seen)
    return DWARF_CB_ABORT;

  return DWARF_CB_OK;
}


/* Implement the ebl_set_initial_registers_tid setfunc callback.  */

static bool
local_thread_state_registers_cb (int firstreg, unsigned nregs,
				 const Dwarf_Word *regs, void *arg)
{
  Dwfl_Thread *thread = (Dwfl_Thread *) arg;
  if (firstreg < 0)
    {
      assert (firstreg == -1);
      assert (nregs == 1);
      INTUSE(dwfl_thread_state_register_pc) (thread, *regs);

      // We assume pc is set last. So we are done. Now unwind a bit.
      bool seen = false;
      return initial_thread_unwind (thread, initial_unwind_callback, &seen);
    }
  assert (nregs > 0);
  return INTUSE(dwfl_thread_state_registers) (thread, firstreg, nregs, regs);
}

static bool
local_set_initial_registers (Dwfl_Thread *thread,
			     void *thread_arg __attribute__((unused)))
{
  Dwfl_Process *process = thread->process;
  Ebl *ebl = process->ebl;
  return ebl_set_initial_registers_local (ebl,
					  local_thread_state_registers_cb, thread);
}

static const Dwfl_Thread_Callbacks local_thread_callbacks =
  {
    local_next_thread,
    local_getthread,
    local_memory_read,
    local_set_initial_registers,
    NULL, // detach
    NULL, // thread_detach
  };

int
dwfl_linux_local_attach (Dwfl *dwfl)
{
  int pid = getpid();
  char name[64];
  int i = snprintf (name, sizeof (name), "/proc/%ld/task", (long) pid);
  assert (i > 0 && i < (ssize_t) sizeof (name) - 1);
  DIR *dir = opendir (name);
  if (dir == NULL)
    return errno;

  i = snprintf (name, sizeof (name), "/proc/%ld/exe", (long) pid);
  assert (i > 0 && i < (ssize_t) sizeof (name) - 1);
  int fd = open (name, O_RDONLY);
  if (fd < 0)
    return errno;

  Elf *self = elf_begin (fd, ELF_C_READ_MMAP, NULL);
  if (self == NULL)
    {
      /* We need to return an errno, so we should really translate the
         elf_errno back to an errno, but lets just assume it is ENOMEM.  */
      close (fd);
      return ENOMEM;
    }

  if (! INTUSE(dwfl_attach_state) (dwfl, self, pid, &local_thread_callbacks,
				   NULL))
    {
      return -1;
    }
  return 0;
}

#else	/* __linux__ */

static pid_t
local_next_thread (Dwfl *dwfl __attribute__ ((unused)),
		   void *dwfl_arg __attribute__ ((unused)),
		   void **thread_argp __attribute__ ((unused)))
{
  errno = ENOSYS;
  __libdwfl_seterrno (DWFL_E_ERRNO);
  return -1;
}

static bool
local_getthread (Dwfl *dwfl __attribute__ ((unused)),
		 pid_t tid __attribute__ ((unused)),
		 void *dwfl_arg __attribute__ ((unused)),
		 void **thread_argp __attribute__ ((unused)))
{
  errno = ENOSYS;
  __libdwfl_seterrno (DWFL_E_ERRNO);
  return false;
}

static bool
local_memory_read (Dwfl *dwfl __attribute__ ((unused)),
		   Dwarf_Addr addr __attribute__ ((unused)),
		   Dwarf_Word *result __attribute__ ((unused)),
		   void *arg __attribute__ ((unused)))
{
  errno = ENOSYS;
  __libdwfl_seterrno (DWFL_E_ERRNO);
  return false;
}

static bool
local_set_initial_registers (Dwfl_Thread *thread __attribute__ ((unused)),
			     void *thread_arg __attribute__ ((unused)))
{
  errno = ENOSYS;
  __libdwfl_seterrno (DWFL_E_ERRNO);
  return false;
}

static const Dwfl_Thread_Callbacks local_thread_callbacks =
  {
    local_next_thread,
    local_getthread,
    local_memory_read,
    local_set_initial_registers,
    NULL, // detach
    NULL, // thread_detach
  };

int
dwfl_linux_local_attach (Dwfl *dwfl __attribute__ ((unused)))
{
  return ENOSYS;
}
INTDEF (dwfl_linux_proc_attach)

#endif /* ! __linux __ */
