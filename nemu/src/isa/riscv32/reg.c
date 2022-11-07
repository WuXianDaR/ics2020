#include <isa.h>
#include "local-include/reg.h"
riscv32_CPU_state riscv32_reg;
const char *regs[] = {
  "$0", "ra", "sp", "gp", "tp", "t0", "t1", "t2",
  "s0", "s1", "a0", "a1", "a2", "a3", "a4", "a5",
  "a6", "a7", "s2", "s3", "s4", "s5", "s6", "s7",
  "s8", "s9", "s10", "s11", "t3", "t4", "t5", "t6"
};

void isa_reg_display() {
	int len = sizeof(regs)/sizeof(regs[0]);
	for(int i = 0;i < len;i++)
	{
		printf("%s: %#010x\n",regs[i],riscv32_reg.gpr[i]._32);
	}
}

word_t isa_reg_str2val(const char *s, bool *success) {
  return 0;
}
