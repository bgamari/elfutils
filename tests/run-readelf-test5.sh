#! /bin/sh
# Copyright (C) 2007 Red Hat, Inc.
# This file is part of elfutils.
#
# This file is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 3 of the License, or
# (at your option) any later version.
#
# elfutils is distributed in the hope that it will be useful, but
# WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.

. $srcdir/test-subr.sh

testfiles testfile72
# This test file originated from the following assembler built by
# GCC 5.2.1 on x86-64.
cat >/dev/null <<\EOF
.section .text
_c2JJ:
        nop
_c2JP:
        nop
_c2K0:
        nop
_c2KN:
        nop
_c2KZ:
        nop
_c2L3:
        nop
.La2_r2HX_info_end:
        nop
a2_r2HX_info:
        nop
.LMain.ffiTest_info_end:
        nop
Main.ffiTest_info:
        nop
.Lsat_s2Ip_info_end:
        nop
sat_s2Ip_info:
        nop

.section .debug_frame,"",@progbits
.Lsection_frame:
_n2PT:
	.long .Ln2PT_end-.Ln2PT_start
.Ln2PT_start:
	.long -1
	.byte 3
	.asciz "S"
	.byte 1
	.byte 120
	.byte 16
	.byte 12
	.byte 6
	.byte 0
	.byte 144
	.byte 0
	.byte 8
	.byte 7
	.byte 20
	.byte 6
	.byte 0
	.align 8
.Ln2PT_end:
	.long .La2_r2HX_info_fde_end-.La2_r2HX_info_fde
.La2_r2HX_info_fde:
	.long _n2PT
	.quad a2_r2HX_info-1
	.quad .La2_r2HX_info_end-a2_r2HX_info+1
	.byte 1
	.quad _c2JJ
	.byte 14
	.byte 16
	.byte 1
	.quad _c2JP
	.byte 14
	.byte 0
	.byte 1
	.quad _c2K0-1
	.byte 14
	.byte 8
	.align 8
.La2_r2HX_info_fde_end:
	.long .LMain.ffiTest_info_fde_end-.LMain.ffiTest_info_fde
.LMain.ffiTest_info_fde:
	.long _n2PT
	.quad Main.ffiTest_info-1
	.quad .LMain.ffiTest_info_end-Main.ffiTest_info+1
	.align 8
.LMain.ffiTest_info_fde_end:
	.long .Lsat_s2Ip_info_fde_end-.Lsat_s2Ip_info_fde
.Lsat_s2Ip_info_fde:
	.long _n2PT
	.quad sat_s2Ip_info-1
	.quad .Lsat_s2Ip_info_end-sat_s2Ip_info+1
	.byte 1
	.quad _c2KN
	.byte 14
	.byte 8
	.byte 1
	.quad _c2KZ
	.byte 14
	.byte 0
	.byte 1
	.quad _c2L3
	.byte 14
	.byte 8
	.align 8
.Lsat_s2Ip_info_fde_end:

EOF

testrun_compare ${abs_top_builddir}/src/readelf --debug-dump=frames testfile72 <<\EOF

DWARF section [ 4] '.debug_frame' at offset 0x50:

 [     0] CIE length=20
   CIE_id:                   18446744073709551615
   version:                  3
   augmentation:             "S"
   code_alignment_factor:    1
   data_alignment_factor:    -8
   return_address_register:  16

   Program:
     def_cfa r6 (rbp) at offset 0
     offset r16 (rip) at cfa+0
     same_value r7 (rsp)
     val_offset 6 at offset 0

 [    18] FDE length=60 cie=[     0]
   CIE_pointer:              0
   initial_location:         .text+0x0000000000000006 <_c2L3+0x1>
   address_range:            0

   Program:
     set_loc 0
     def_cfa_offset 16
     set_loc 0x1
     def_cfa_offset 0
     set_loc 0x1
     def_cfa_offset 8
     nop
     nop
     nop
     nop
     nop
     nop
     nop

 [    58] FDE length=20 cie=[     0]
   CIE_pointer:              0
   initial_location:         .text+0x0000000000000008 <a2_r2HX_info+0x1>
   address_range:            0

   Program:

 [    70] FDE length=60 cie=[     0]
   CIE_pointer:              0
   initial_location:         .text+0x000000000000000a <Main.ffiTest_info+0x1>
   address_range:            0

   Program:
     set_loc 0x3
     def_cfa_offset 8
     set_loc 0x4
     def_cfa_offset 0
     set_loc 0x5
     def_cfa_offset 8
     nop
     nop
     nop
     nop
     nop
     nop
     nop
EOF

exit 0
