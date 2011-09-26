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

#define TARGET_NACL (flag_control_integrity)

/* ??? Blah -- this macro is used directly by libobjc.  Since it
   supports no vector modes, cut out the complexity and fall back
   on BIGGEST_FIELD_ALIGNMENT.  NaCl uses x86-64 style alignment
   in IA32 mode so we must redefine it. */
#ifdef IN_TARGET_LIBS
#undef BIGGEST_FIELD_ALIGNMENT
#define BIGGEST_FIELD_ALIGNMENT 128
#endif

/* These definitions modify those in i386elf.h. */

#undef TARGET_VERSION
#define TARGET_VERSION fprintf (stderr, " (NativeClient)");

/* Don't enclose system header files in extern "C" {...}.  */
#define NO_IMPLICIT_EXTERN_C

/* Provide a STARTFILE_SPEC.  Here we add
   the GNU/Linux magical crtbegin.o file (see crtstuff.c) which provides part of
   the support for getting C++ file-scope static object constructed before
   entering `main'.  */

#undef	STARTFILE_SPEC
#if defined HAVE_LD_PIE
#define STARTFILE_SPEC \
  "%{!shared: %{pg|p|profile:gcrt1.o%s;pie:Scrt1.o%s;:crt1.o%s}} \
   crti.o%s %{static:crtbeginT.o%s;shared|pie:crtbeginS.o%s;:crtbegin.o%s}"
#else
#define STARTFILE_SPEC \
  "%{!shared: %{pg|p|profile:gcrt1.o%s;:crt1.o%s}} \
   crti.o%s %{static:crtbeginT.o%s;shared|pie:crtbeginS.o%s;:crtbegin.o%s}"
#endif

/* Provide a ENDFILE_SPEC.  Here we tack on
   the GNU/Linux magical crtend.o file (see crtstuff.c) which provides part of
   the support for getting C++ file-scope static object constructed before
   entering `main', followed by a normal GNU/Linux "finalizer" file, `crtn.o'.
   TODO(pasko): add -ffast-math support to ENDFILE_SPEC.  */

#undef	ENDFILE_SPEC
#define ENDFILE_SPEC \
  "%{shared|pie:crtendS.o%s;:crtend.o%s} crtn.o%s"

/* This is for -profile to use -lc_p instead of -lc.  */
#ifndef CC1_SPEC
#define CC1_SPEC "%{profile:-p}"
#endif

/* The GNU C++ standard library requires that these macros be defined.  */
#undef CPLUSPLUS_CPP_SPEC
#define CPLUSPLUS_CPP_SPEC "-D_GNU_SOURCE %(cpp)"

#undef	LIB_SPEC
#define LIB_SPEC \
  "%{pthread:-lpthread} \
   %{shared:-lc} \
   %{!shared:%{mieee-fp:-lieee} %{profile:-lc_p}%{!profile:-lc}}"

/* Pass the NativeClient specific options to the assembler.  */
#undef  ASM_SPEC
#define ASM_SPEC \
  "%{v:-V} %{Qy:} %{!Qn:-Qy} %{n} %{T} " \
  "%{fnacl-library-mode:-nacl-library-mode} " \
  "%{fnacl-align-16:-nacl-align=4} " \
  "%{fnacl-align-32:-nacl-align=5} " \
  "%{Ym,*} %{Yd,*} %{Wa,*:%*} %{m32:--32} %{m64:--64} " \
  "%{!mno-sse2avx:%{mavx:-msse2avx}} %{msse2avx:%{!mavx:-msse2avx}}"

#undef	LIB_SPEC
#define LIB_SPEC \
  "%{pthread:-lpthread} \
   %{shared:-lc} \
   %{!shared:%{mieee-fp:-lieee} %{profile:-lc_p}%{!profile:-lc}}"

/* Define this so we can compile MS code for use with WINE.  */
#define HANDLE_PRAGMA_PACK_PUSH_POP

#if defined(HAVE_LD_EH_FRAME_HDR)
#define LINK_EH_SPEC "--no-add-needed %{!static:--eh-frame-hdr} "
#endif

/* Use --as-needed -lgcc_s for eh support.  */
#ifdef HAVE_LD_AS_NEEDED
#define USE_LD_AS_NEEDED 1
#endif

#define NACL_DYNAMIC_LINKER32 "/lib/ld-nacl-x86-32.so.1"
#define NACL_DYNAMIC_LINKER64 "/lib64/ld-nacl-x86-64.so.1"

/* Determine whether the entire c99 runtime
   is present in the runtime library.  */
#define TARGET_C99_FUNCTIONS (OPTION_GLIBC)

/* Whether we have sincos that follows the GNU extension.  */
#define TARGET_HAS_SINCOS (OPTION_GLIBC)

#define TARGET_POSIX_IO

#if TARGET_64BIT_DEFAULT
#define SPEC_32 "m32"
#define SPEC_64 "!m32"
#else
#define SPEC_32 "!m64"
#define SPEC_64 "m64"
#endif

#undef	LINK_SPEC
#define LINK_SPEC "%{" SPEC_64 ":-m elf64_nacl} %{" SPEC_32 ":-m elf_nacl} \
  %{shared:-shared} \
  %{!shared: \
    %{!static: \
      %{rdynamic:-export-dynamic} \
      %{" SPEC_32 ":%{!dynamic-linker:-dynamic-linker " NACL_DYNAMIC_LINKER32 "}} \
      %{" SPEC_64 ":%{!dynamic-linker:-dynamic-linker " NACL_DYNAMIC_LINKER64 "}}} \
    %{static:-static}}"

#undef LINK_GCC_C_SEQUENCE_SPEC
#define LINK_GCC_C_SEQUENCE_SPEC \
  "%{static:--start-group} %G %L %{static:--end-group}%{!static:%G}"

#if TARGET_64BIT_DEFAULT
#define MULTILIB_DEFAULTS { "m64" }
#else
#define MULTILIB_DEFAULTS { "m32" }
#endif

#undef NEED_INDICATE_EXEC_STACK
#define NEED_INDICATE_EXEC_STACK 1

#define MD_UNWIND_SUPPORT "config/i386/linux-unwind.h"

/* This macro may be overridden in i386/k*bsd-gnu.h.  */
#define REG_NAME(reg) reg

#ifdef TARGET_LIBC_PROVIDES_SSP
/* i386 glibc provides __stack_chk_guard in %gs:0x14,
   x86_64 glibc provides it in %fs:0x28.  */
#define TARGET_THREAD_SSP_OFFSET	(TARGET_64BIT ? 0x28 : 0x14)
#endif

/* Because of NaCl's use of segment registers, negative offsets from gs: will
   not work.  Hence we need to make TLS references explicitly compute the
   tls base pointer and then indirect relative to it using the default
   segment descriptor (DS).  That is, instead of
      movl gs:i@NTPOFF, %ecx
   we use
     movl %gs:0, %eax
     movl i@NTPOFF(%eax), %ecx
   There is a slight performance penalty for TLS accesses, but there does not
   seem a way around it.  */
#undef TARGET_TLS_DIRECT_SEG_REFS_DEFAULT
#define TARGET_TLS_DIRECT_SEG_REFS_DEFAULT 0

#define LINUX_TARGET_OS_CPP_BUILTINS()				\
    do {							\
	builtin_define_std ("unix");				\
	builtin_assert ("system=unix");				\
	builtin_assert ("system=posix");			\
    } while (0)

/* When running in Native Client all inode numbers are identical.
   TODO(pasko): re-enable inode numbers once the relevant bug is fixed:
   http://code.google.com/p/nativeclient/issues/detail?id=1555  */
#ifdef __native_client__
#define HOST_LACKS_INODE_NUMBERS 1
#endif

#undef TARGET_OS_CPP_BUILTINS
#define TARGET_OS_CPP_BUILTINS()			\
  do							\
    {							\
	LINUX_TARGET_OS_CPP_BUILTINS();			\
	builtin_define ("__native_client__=1");		\
  }							\
  while (0)

/* NaCl uses are using ILP32 model even on x86-84.  */
#undef LONG_TYPE_SIZE
#define LONG_TYPE_SIZE 32
#define POINTER_SIZE 32
#define POINTERS_EXTEND_UNSIGNED 1
#undef LONG_DOUBLE_TYPE_SIZE
#define LONG_DOUBLE_TYPE_SIZE 64

#undef TARGET_SUBTARGET_DEFAULT
#define TARGET_SUBTARGET_DEFAULT (MASK_IEEE_FP)

#undef TARGET_SUBTARGET32_DEFAULT
#define TARGET_SUBTARGET32_DEFAULT (MASK_80387 | MASK_ALIGN_DOUBLE | MASK_FLOAT_RETURNS)

#undef TARGET_SUBTARGET64_DEFAULT
#define TARGET_SUBTARGET64_DEFAULT 0

/* Configure script incorrectly detects this GAS capability on x86-64 and hence
   forces JUMP_TABLES_IN_TEXT_SECTION which cannot validate in NaCl.  */
#undef HAVE_AS_GOTOFF_IN_DATA
#define HAVE_AS_GOTOFF_IN_DATA 1

/* Configure script incorrectly detects HAVE_GAS_CFI_DIRECTIVE when readelf is
   not found in PATH.  */
#undef HAVE_GAS_CFI_DIRECTIVE
#define HAVE_GAS_CFI_DIRECTIVE 1

/* NaCl reserves R15 and makes RBP special in x86-64 mode.  */
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
#define TARGET_USE_LEAVE	(ix86_tune_features[X86_TUNE_USE_LEAVE] && !TARGET_NACL)
#undef TARGET_USE_BT
#define TARGET_USE_BT		(ix86_tune_features[X86_TUNE_USE_BT] && !TARGET_NACL)

#undef DBX_REGISTER_NUMBER
#define DBX_REGISTER_NUMBER(n) \
  (TARGET_64BIT ? dbx64_register_map[n] : svr4_dbx_register_map[n])

#define DWARF2_ADDR_SIZE \
    (TARGET_NACL ? (TARGET_64BIT ? 8 : 4) : \
                   (POINTER_SIZE / BITS_PER_UNIT))
