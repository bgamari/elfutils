/* Standard libdwfl callbacks for getting memory map out of the Linux
   dynamic linker.

   Copyright (C) 2005-2010, 2013, 2014 Red Hat, Inc.
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
// We need _GNU_SOURCE defined to get dl_iterate_phdr
#include <link.h>
#include <unistd.h>
#include <stdlib.h>

static int
phdr_cb (struct dl_phdr_info *info, size_t size __attribute__ ((unused)),
         void *data)
{
  Dwfl *dwfl = (Dwfl *) data;
  int i;
  // Find end of object
  GElf_Addr high_addr = 0;
  for (i = 0; i < info->dlpi_phnum; i++) {
    const ElfW(Phdr) *phdr = &info->dlpi_phdr[i];
    GElf_Addr start_addr = info->dlpi_addr + phdr->p_vaddr;
    GElf_Addr end_addr = start_addr + phdr->p_memsz;
    if (end_addr > high_addr)
      high_addr = end_addr;
  }

  // Resolve absolute path
  char *real_path = realpath(info->dlpi_name, NULL);
  if (real_path == NULL)
    return 0;

  // Report the object
  Dwfl_Module *mod = INTUSE(dwfl_report_module) (dwfl, real_path,
                                                 info->dlpi_addr, high_addr);
  free(real_path);
  if (mod == NULL)
    return -1;

  // Report its segments
  for (i = 0; i < info->dlpi_phnum; i++) {
    const ElfW(Phdr) *phdr = &info->dlpi_phdr[i];
    dwfl_report_segment (dwfl, i, phdr, 0, NULL);
  }
  return 0;
}

int
dwfl_linux_dl_report (Dwfl *dwfl)
{
  int res = dl_iterate_phdr (phdr_cb, dwfl);
  return res;
}
