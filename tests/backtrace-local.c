/* Test program for unwinding of local process frames.
   Copyright (C) 2013, 2014 Red Hat, Inc.
   This file is part of elfutils.

   This file is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3 of the License, or
   (at your option) any later version.

   elfutils is distributed in the hope that it will be useful, but
   WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.  */

#include <config.h>
#include <unistd.h>
#include ELFUTILS_HEADER(dwfl)

#ifndef __linux__

int
main (int argc __attribute__ ((unused)), char **argv __attribute__ ((unused)))
{
  fprintf (stderr, "%s: Unwinding not supported for this architecture\n",
           argv[0]);
  return 77;
}

#else /* __linux__ */

static
int frame_cb(Dwfl_Frame *frame, void *arg)
{
  Dwfl *dwfl = (Dwfl *) arg;
  Dwarf_Addr pc;
  dwfl_frame_pc (frame, &pc, NULL);
  Dwfl_Module *mod = dwfl_addrmodule (dwfl, pc);
  GElf_Off off;
  const char *sym = dwfl_module_addrinfo (mod, pc,
                                          &off, NULL,
                                          NULL, NULL,
                                          NULL);
  printf("%s\n", sym ? sym : "null");
  return DWARF_CB_OK;
}

static int
dump_backtrace()
{
  int ret = 0;
  static char *debuginfo_path;
  static const Dwfl_Callbacks proc_callbacks =
    {
      .find_debuginfo = dwfl_standard_find_debuginfo,
      .debuginfo_path = &debuginfo_path,
      .find_elf = dwfl_linux_proc_find_elf,
    };

  Dwfl *dwfl = dwfl_begin (&proc_callbacks);
  if (dwfl == NULL) {
    fprintf (stderr, "dwfl_begin failed: %s\n", dwfl_errmsg(dwfl_errno()));
    return 1;
  }

  ret = dwfl_linux_proc_report(dwfl, getpid());
  if (ret < 0) {
    fprintf (stderr, "dwfl_linux_proc_report failed: %s",
             dwfl_errmsg(dwfl_errno()));
    goto done;
  }

  ret = dwfl_report_end (dwfl, NULL, NULL);
  if (ret) {
    fprintf (stderr, "dwfl_report_end failed: %s", dwfl_errmsg(dwfl_errno()));
    goto done;
  }

  ret = dwfl_linux_local_attach (dwfl);
  if (ret) {
    fprintf (stderr, "linux_local_attach failed: %s\n",
             dwfl_errmsg(dwfl_errno()));
    goto done;
  }

  ret = dwfl_getthread_frames (dwfl, getpid(), frame_cb, dwfl) != 0;
  if (ret) {
    fprintf (stderr, "getthread_frames failed: %s\n",
             dwfl_errmsg(dwfl_errno()));
    goto done;
  }

 done:
  dwfl_end(dwfl);
  return ret;
}

int
main (int argc __attribute__ ((unused)),
      char **argv __attribute__ ((unused)))
{
  dump_backtrace();
  return 0;
}

#endif
