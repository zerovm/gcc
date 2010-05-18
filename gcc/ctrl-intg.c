/* Patch RTL to enforce control flow integrity for GCC.
   Copyright (C) 1987, 1988, 1992, 1997, 1998, 1999, 2000, 2002, 2003,
   2004, 2005, 2007
   Free Software Foundation, Inc.

This file is part of GCC.

GCC is free software; you can redistribute it and/or modify it under
the terms of the GNU General Public License as published by the Free
Software Foundation; either version 3, or (at your option) any later
version.

GCC is distributed in the hope that it will be useful, but WITHOUT ANY
WARRANTY; without even the implied warranty of MERCHANTABILITY or
FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
for more details.

You should have received a copy of the GNU General Public License
along with GCC; see the file COPYING3.  If not see
<http://www.gnu.org/licenses/>.  */

/* This file is compiled twice: once for the generator programs,
   once for the compiler.  */
#ifdef GENERATOR_FILE
#include "bconfig.h"
#else
#include "config.h"
#endif

#include "system.h"
#include "coretypes.h"
#include "tm.h"
#include "rtl.h"
#include "tree-pass.h"
#include "expr.h"

/* These headers all define things which are not available in
   generator programs.  */
#ifndef GENERATOR_FILE
#include "tree.h"
#include "real.h"
#include "flags.h"
#include "hard-reg-set.h"
#include "basic-block.h"
#endif

static bool
gate_func (void) {
  return getenv("NACLSHUTDOWN") == NULL;
  /* return flag_control_integrity; */
}

static void
process_call_insn(rtx insn) {
  rtx return_value_expr, call_expr, mem_expr, addr_expr, parallel_expr;
  rtx sp_size_expr, tls_disp;

  /*
   * Get the expression to be examined from the instruction.
   */
  call_expr = XEXP (insn, 5);

  if (GET_CODE (call_expr) == PARALLEL) {
    /*
     * Calls that pop the stack use a PARALLEL containing a CALL and a SET.
     */
    rtx vec1 = XVECEXP (call_expr, 0, 1);
    parallel_expr = call_expr;
    tls_disp = NULL_RTX;

    if (TARGET_64BIT &&
        GET_CODE(vec1) == UNSPEC && XINT(vec1, 1) == UNSPEC_TPOFF) {
      /*
       * TLS calls use a PARALLEL containing a CALL and UNSPEC_TPOFF.
       */
      tls_disp = XVECEXP(vec1, 0, 0);
      gcc_assert(GET_CODE(tls_disp) == SYMBOL_REF);
    } else {
      sp_size_expr = XEXP (XEXP (vec1, 1), 1);
    }
    call_expr = XVECEXP (call_expr, 0, 0);
  }
  else {
    parallel_expr = NULL_RTX;
  }

  /*
   * Get the call expression and return value (if any).
   */
  if (GET_CODE (call_expr) == SET) {
    /*
     * Functions with return values use a SET instruction wrapper.
     * Get the call out of the set if needed.
     */
    return_value_expr = XEXP (call_expr, 0);
    call_expr = XEXP (call_expr, 1);
  }
  else {
    return_value_expr = NULL_RTX;
  }

  /*
   *  Extract the target address expression of the function.
   */
  mem_expr = XEXP (call_expr, 0);

  /*
   * Get the address expression from the MEM.
   */

  gcc_assert (GET_CODE (mem_expr) == MEM);
  addr_expr = XEXP (mem_expr, 0);

  if (GET_CODE (addr_expr) != SYMBOL_REF) {
    rtx insns_head, call, call_insn;
    int enable_print;

    {
      static int calls_converted=0;
      static int printed=0;
      char* call_limit = getenv("NONACLCALL");
      char* name_compare = getenv("NACLBINS");
      if (name_compare && strcmp(main_input_filename, name_compare) > 0) {
        if (printed == 0) {
          fprintf(stderr, "NACL: name test shut off\n");
          printed = 1;
        }
        return;
      }

      ++calls_converted;
      enable_print = (call_limit && calls_converted == atoi(call_limit));
      if (call_limit && calls_converted > atoi(call_limit)) {
        if (printed == 0) {
          fprintf(stderr, "NACL: '%s' call limit exceeded\n",
                  main_input_filename);
          printed = 1;
        }
        return;
      }
      /* fprintf(stderr, "NACL: converted call %d\n", calls_converted); */
    }

    if (return_value_expr && parallel_expr) {
      if (getenv("NACLDBGBOTH")) return;
    } else if (return_value_expr) {
      if (getenv("NACLDBGRET")) return;
    } else if (parallel_expr) {
      if (getenv("NACLDBGPAR")) return;
    } else {
      if (SIBLING_CALL_P (insn)) {
        if (getenv("NACLDBGNONE1")) return;
      } else {
        char* str = getenv("NACLDBGNONE2");
        if (str) {
          FILE* fp = fopen("/home/sehr/NACLDBGCOUNT", "r+");
          int current_count;
          fscanf(fp, "%d\n", &current_count);
            fprintf(stderr, "NACLDEBUGCOUNT = %d \n", current_count);
          rewind(fp);
          fprintf(fp, "%d\n", current_count+1);
          fclose(fp);
          if (current_count > atoi(str)) {
            fprintf(stderr, "NACLDEBUGCOUNT %d exceeded %d\n",
                    current_count, atoi(str));
            return;
          }
        }
      }
    }

    start_sequence ();

    if (enable_print) {
      fprintf(stderr, "Before:\n");
      print_rtl_single(stderr, insn);
    }

    /*
     * Force the called function address to be in a register.
     */
    addr_expr = force_reg (GET_MODE (addr_expr), addr_expr);

#define gen_nacl(suffix) \
  (TARGET_64BIT ? gen_nacl ## suffix ## di : \
		  gen_nacl ## suffix ## si)
    /*
     * Generate the appropriate template for the call
     */
    if (return_value_expr && parallel_expr) {
      if (!tls_disp) {
        call = gen_nacl(call_value_pop) (return_value_expr, addr_expr,
                                         XEXP (call_expr, 1), sp_size_expr);
      } else {
        call = gen_naclcall_tls(mem_expr, XEXP (call_expr, 1));
      }
    } else if (return_value_expr) {
      if (SIBLING_CALL_P (insn)) {
        call = gen_nacl(sibcall_value) (return_value_expr,
                                        addr_expr, XEXP (call_expr, 1));
      } else {
        call = gen_nacl(call_value) (return_value_expr,
                                     addr_expr, XEXP (call_expr, 1));
      }
    } else if (parallel_expr) {
      call = gen_nacl(call_pop) (addr_expr, XEXP (call_expr, 1),
                                 sp_size_expr);
    } else {
      if (SIBLING_CALL_P (insn)) {
        call = gen_nacl(sibcall) (addr_expr, XEXP (call_expr, 1));
      } else {
	call = gen_nacl(call) (addr_expr, XEXP (call_expr, 1));
      }
    }

    call_insn = emit_call_insn (call);

    RTL_CONST_CALL_P (call_insn) = RTL_CONST_CALL_P (insn);
    RTL_PURE_CALL_P (call_insn) = RTL_PURE_CALL_P (insn);
    SIBLING_CALL_P (call_insn) = SIBLING_CALL_P (insn);
    REG_NOTES (call_insn) = REG_NOTES (insn);
    CALL_INSN_FUNCTION_USAGE (call_insn) = CALL_INSN_FUNCTION_USAGE (insn);

    insns_head = get_insns ();

    if (enable_print) {
      fprintf(stderr, "After: (%d, %d) \n", RTL_CONST_OR_PURE_CALL_P (call_insn),
              SIBLING_CALL_P (call_insn));
      print_rtl(stderr, insns_head);
    }

    end_sequence ();
    emit_insn_before (insns_head, insn);

    delete_insn (insn);
  }
}


static void
process_jump_insn(rtx insn) {
  rtx par_expr, set_expr, addr_expr;
  rtx jmp;

  /*
   * Get the contained expression.
   */
  par_expr = XEXP (insn, 5);

  if (GET_CODE (par_expr) == PARALLEL) {
    set_expr = XVECEXP (par_expr, 0, 0);

    if (GET_CODE (set_expr) == SET) {
      addr_expr = XEXP (set_expr, 1);

      if (GET_CODE (addr_expr) == IF_THEN_ELSE) {
        /*
         * Ordinary branches uses parallel/set/if_then_else.
         * Leave them unmodified.
         */
      }
      else {
        /*
         * A table indirect jump instruction has parallel/set/other
         */
        rtx insns_head, jmp_insn;
        int enable_print;

        {
          static int calls_converted=0;
          static int printed=0;
          char* name_compare = getenv("NACLBINS");
          char* call_limit = getenv("NONACLJMP");
          if (name_compare && strcmp(main_input_filename, name_compare) > 0) {
            fprintf(stderr, "NACL: name test shut off\n");
            return;
          }

          ++calls_converted;
          enable_print = (call_limit && calls_converted == atoi(call_limit));
          if (call_limit && calls_converted > atoi(call_limit)) {
            if (printed == 0) {
              fprintf(stderr, "NACL: '%s' call limit exceeded\n",
                      main_input_filename);
              printed = 1;
            }
            return;
          }
          /*fprintf(stderr, "NACL: converted branch %d\n", calls_converted);*/
        }

        start_sequence ();

        if (enable_print) {
          fprintf(stderr, "Before:\n");
          print_rtl_single(stderr, insn);
        }
        addr_expr = force_reg (GET_MODE (addr_expr), addr_expr);
        jmp = gen_nacl(jmp_table) (addr_expr,
                                   XEXP (XEXP (XVECEXP (par_expr, 0, 1), 0), 0));
        jmp_insn = emit_jump_insn (jmp);

        if (JUMP_LABEL (insn) != NULL_RTX) {
           JUMP_LABEL (jmp_insn) = JUMP_LABEL (insn);
           LABEL_NUSES (JUMP_LABEL (insn))++;
        }

        insns_head = get_insns ();
        if (enable_print) {
          fprintf(stderr, "After %p:\n", (void*) JUMP_LABEL (jmp_insn));
          print_rtl(stderr, insns_head);
        }

        end_sequence ();
        emit_insn_before (insns_head, insn);

        delete_insn (insn);
      }
    }
  } else {
    /*
     * Other indirect jumps remain to be identified.
     */
  }
}

extern int nacl_special_commands;

static int
execute_func (void) {
  basic_block bb;

  int save_nacl_special_commands = nacl_special_commands;
  nacl_special_commands = 1;
  
  if (getenv("NACLSHUTDOWN4")) return 0;
  /* Even if reload is not yet completed - fake it to make reload impossible */
  FOR_EACH_BB (bb) {
    rtx insn, last;

    if (getenv("NACLSHUTDOWN3")) continue;
    for (insn = BB_HEAD (bb), last = NEXT_INSN (BB_END (bb)); insn != last;
         insn = NEXT_INSN(insn)) {
      if (getenv("NACLSHUTDOWN2")) continue;
      if (JUMP_P (insn)) {
        if (flag_control_integrity)
          process_jump_insn (insn);
      }
      if (CALL_P (insn)) {
        if (flag_control_integrity)
          process_call_insn (insn);
      }
    }
  }

  nacl_special_commands = save_nacl_special_commands;
  return 0;
}

struct rtl_opt_pass pass_control_integrity = {
  {
    RTL_PASS,
    "ctrl_intg_insert",
    gate_func,
    execute_func,
    0, /* sub */
    0, /* next */
    0, /* static_pass_number */
    0, /* tv_id */
    0, /* properties_required */
    0, /* properties_provided */
    0, /* properties_destroyed */
    TODO_dump_func, /* todo_flags_start */
    TODO_dump_func, /* todo_flags_finish */
  }
};
