/* 
*   voidstep.c - A powerful minimal debugger for `x86_64` Linux.
*    - Copyright (C) 2026 NopAngel.
*
*   This program is free software: you can redistribute it and/or modify
*   it under the terms of the GNU General Public License as published by
*   the Free Software Foundation, either version 3 of the License, or
*   (at your option) any later version. 
*/

#include <stdio.h>
#include <stdlib.h>
#include <sys/ptrace.h>
#include <sys/wait.h>
#include <sys/user.h>
#include <unistd.h>
#include <stdint.h>
#include <string.h>

/* Structure to store breakpoint information. */
struct breakpoint {
    uintptr_t addr;
    long orig_data;
};

/* Display current CPU state. */
void
print_registers (struct user_regs_struct *regs)
{
  printf ("\n--- REGISTER STATE ---\n");
  printf ("RIP: 0x%016llx   RSP: 0x%016llx\n", regs->rip, regs->rsp);
  printf ("RAX: 0x%016llx   RBX: 0x%016llx\n", regs->rax, regs->rbx);
  printf ("RCX: 0x%016llx   RDX: 0x%016llx\n", regs->rcx, regs->rdx);
  printf ("----------------------\n");
}

/* Set a software breakpoint (INT 3). */
void
set_breakpoint (pid_t pid, struct breakpoint *bp)
{
  bp->orig_data = ptrace (PTRACE_PEEKDATA, pid, bp->addr, NULL);
  long trap_data = (bp->orig_data & ~0xFF) | 0xCC;
  ptrace (PTRACE_POKEDATA, pid, bp->addr, (void *)trap_data);
  printf ("Debugger: Breakpoint set at 0x%lx\n", bp->addr);
}

void
run_debugger (pid_t child_pid, uintptr_t bp_addr)
{
  int status;
  struct user_regs_struct regs;
  struct breakpoint my_bp = { .addr = bp_addr };
  int bp_active = (bp_addr != 0);

  waitpid (child_pid, &status, 0);

  if (bp_active)
    set_breakpoint (child_pid, &my_bp);

  while (WIFSTOPPED (status))
    {
      ptrace (PTRACE_GETREGS, child_pid, NULL, &regs);

      /* Handle Breakpoint Hit. */
      if (bp_active && regs.rip == my_bp.addr + 1)
        {
          printf ("\n*** BREAKPOINT HIT AT 0x%lx ***\n", my_bp.addr);
          regs.rip = my_bp.addr;
          ptrace (PTRACE_SETREGS, child_pid, NULL, &regs);
          /* Restore original instruction. */
          ptrace (PTRACE_POKEDATA, child_pid, my_bp.addr, (void *)my_bp.orig_data);
          print_registers (&regs);
        }

      printf ("[0x%llx] (s)tep, (c)ontinue, (x)examine, (w)rite: ", regs.rip);
      char cmd[10];
      fgets(cmd, sizeof(cmd), stdin);

      if (cmd[0] == 'x') /* Examine Memory. */
        {
          uintptr_t addr;
          printf ("  Address to read (hex): ");
          scanf ("%lx", &addr);
          getchar(); // flush
          long data = ptrace (PTRACE_PEEKDATA, child_pid, addr, NULL);
          printf ("  [0x%lx] -> 0x%016lx\n", addr, (unsigned long)data);
          continue;
        }
      else if (cmd[0] == 'w') /* Write Memory. */
        {
          uintptr_t addr;
          long val;
          printf ("  Address to write (hex): ");
          scanf ("%lx", &addr);
          printf ("  New value (hex): ");
          scanf ("%lx", &val);
          getchar(); // flush
          ptrace (PTRACE_POKEDATA, child_pid, addr, (void *)val);
          printf ("  Write complete.\n");
          continue;
        }
      else if (cmd[0] == 'c')
        {
          ptrace (PTRACE_CONT, child_pid, NULL, NULL);
        }
      else
        {
          ptrace (PTRACE_SINGLESTEP, child_pid, NULL, NULL);
        }

      waitpid (child_pid, &status, 0);
    }
  printf ("Debugger: Target process terminated.\n");
}

int
main (int argc, char **argv)
{
  if (argc < 2)
    {
      printf ("Usage: %s <program> [breakpoint_addr_hex]\n", argv[0]);
      return 1;
    }

  uintptr_t bp_addr = (argc > 2) ? strtoull (argv[2], NULL, 16) : 0;
  pid_t child = fork ();

  if (child == 0)
    {
      ptrace (PTRACE_TRACEME, 0, NULL, NULL);
      execl (argv[1], argv[1], NULL);
    }
  else
    {
      run_debugger (child, bp_addr);
    }

  return 0;
}
