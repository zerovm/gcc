/* Target definitions for GCC for NativeClient using ELF
   Copyright (C) 1988, 1991, 1995, 2000, 2001, 2002
   Free Software Foundation, Inc.

   Derived from sysv4.h written by Ron Guilmette (rfg@netcom.com).

This file is part of GCC.

GCC is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2, or (at your option)
any later version.

GCC is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with GCC; see the file COPYING.  If not, write to
the Free Software Foundation, 59 Temple Place - Suite 330,
Boston, MA 02111-1307, USA.  */

/* These definitions modify those in i386elf.h. */

#undef TARGET_VERSION
#define TARGET_VERSION fprintf (stderr, " (NativeClient)");

/* Pass the NativeClient specific options to the assembler */
#undef  ASM_SPEC
#define ASM_SPEC \
  "%{v:-V} %{Qy:} %{!Qn:-Qy} %{n} %{T} " \
  "%{fnacl-library-mode:-nacl-library-mode} " \
  "%{fnacl-align-16:-nacl-align=4} " \
  "%{fnacl-align-32:-nacl-align=5} " \
  "%{Ym,*} %{Yd,*} %{Wa,*:%*}"

/* `crt_platform' contains low-level platform-specific intrinsics in C. */
#undef	LIB_SPEC
#define LIB_SPEC \
  "%{pthread:-lpthread} \
   %{mieee-fp:-lieee} %{profile:-lc_p}%{!profile:-lc} \
   -lnacl \
   %{mieee-fp:-lieee} %{profile:-lc_p}%{!profile:-lc} \
   %{lnosys:-lnosys} \
   -lcrt_platform \
   %{mieee-fp:-lieee} %{profile:-lc_p}%{!profile:-lc}"

/*
 * Set the linker emulation to be elf_nacl rather than linux.h's default
 * (elf_i386).
 */
#ifdef LINK_EMULATION
#undef LINK_EMULATION
#endif
#define LINK_EMULATION "elf_nacl"

/*
 * Because of NaCl's use of segment registers, negative offsets from gs: will
 * not work.  Hence we need to make TLS references explicitly compute the
 * tls base pointer and then indirect relative to it using the default
 * segment descriptor (DS).  That is, instead of
 *    movl gs:i@NTPOFF, %ecx
 * we use
 *   movl %gs:0, %eax
 *   movl i@NTPOFF(%eax), %ecx
 * There is a slight performance penalty for TLS accesses, but there does not
 * seem a way around it.
 */
#undef TARGET_TLS_DIRECT_SEG_REFS_DEFAULT
#define TARGET_TLS_DIRECT_SEG_REFS_DEFAULT 0

#undef TARGET_OS_CPP_BUILTINS
#define TARGET_OS_CPP_BUILTINS()			\
  do							\
    {							\
	LINUX_TARGET_OS_CPP_BUILTINS();			\
	builtin_define ("__native_client__=1");		\
  }							\
  while (0)

/* NaCl uses are using ILP32 model even on x86-84 */
#undef LONG_TYPE_SIZE
#define LONG_TYPE_SIZE 32
#define POINTER_SIZE 32
#define POINTERS_EXTEND_UNSIGNED 1
/* NaCl reserves R15 and makes RBP special in x86-64 mode */
#undef FIXED_REGISTERS
#define FIXED_REGISTERS						\
/*ax,dx,cx,bx,si,di,bp,sp,st,st1,st2,st3,st4,st5,st6,st7*/	\
{  0, 0, 0, 0, 0, 0, 3, 1, 0,  0,  0,  0,  0,  0,  0,  0,	\
/*arg,flags,fpsr,fpcr,frame*/					\
    1,    1,   1,   1,    1,					\
/*xmm0,xmm1,xmm2,xmm3,xmm4,xmm5,xmm6,xmm7*/			\
     0,   0,   0,   0,   0,   0,   0,   0,			\
/* mm0, mm1, mm2, mm3, mm4, mm5, mm6, mm7*/			\
     0,   0,   0,   0,   0,   0,   0,   0,			\
/*  r8,  r9, r10, r11, r12, r13, r14, r15*/			\
     2,   2,   2,   2,   2,   2,   2,   1,			\
/*xmm8,xmm9,xmm10,xmm11,xmm12,xmm13,xmm14,xmm15*/		\
     2,   2,    2,    2,    2,    2,    2,    2 }
#undef CALL_USED_REGISTERS
#define CALL_USED_REGISTERS					\
/*ax,dx,cx,bx,si,di,bp,sp,st,st1,st2,st3,st4,st5,st6,st7*/	\
{  1, 1, 1, 0, 3, 3, 3, 1, 1,  1,  1,  1,  1,  1,  1,  1,	\
/*arg,flags,fpsr,fpcr,frame*/					\
    1,   1,    1,   1,    1,					\
/*xmm0,xmm1,xmm2,xmm3,xmm4,xmm5,xmm6,xmm7*/			\
     1,   1,   1,   1,   1,   1,   1,   1,			\
/* mm0, mm1, mm2, mm3, mm4, mm5, mm6, mm7*/			\
     1,   1,   1,   1,   1,   1,   1,   1,			\
/*  r8,  r9, r10, r11, r12, r13, r14, r15*/			\
     1,   1,   1,   1,   2,   2,   2,   1,			\
/*xmm8,xmm9,xmm10,xmm11,xmm12,xmm13,xmm14,xmm15*/		\
     1,   1,    1,    1,    1,    1,    1,    1 }
/* Leave is forbidden in NaCl mode */
#undef TARGET_USE_LEAVE
#define TARGET_USE_LEAVE	(ix86_tune_features[X86_TUNE_USE_LEAVE] && !flag_control_integrity)
#undef TARGET_USE_BT
#define TARGET_USE_BT		(ix86_tune_features[X86_TUNE_USE_BT] && !flag_control_integrity)

#undef DBX_REGISTER_NUMBER
#define DBX_REGISTER_NUMBER(n) \
  (TARGET_64BIT ? dbx64_register_map[n] : svr4_dbx_register_map[n])

#define DWARF2_ADDR_SIZE \
    (flag_control_integrity ? (TARGET_64BIT ? 8 : 4) : \
                              (POINTER_SIZE / BITS_PER_UNIT))
