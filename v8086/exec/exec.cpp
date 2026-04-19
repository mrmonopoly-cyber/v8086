#include "exec.h"
#include "v8086_definitions.h"

typedef s32 (*op_exec)(Instruction* instr, CPU* cpu);

static op_exec executor [(size_t)Opcode::__Opcode_count];

s32 invalid_op_exec(Instruction* instr, CPU* cpu)
{
  s32 res=-1;

  UNUSED(cpu);
  UNUSED(instr);

  return res;
}

__attribute__((constructor))
static void init_executor_array(void)
{
  for(size_t i=0; i<ArraySize(executor); i++)
  {
    executor[i] = invalid_op_exec;
  }
}

s32 InstructionExec(Instruction* const instr, CPU* cpu)
{
  s32 res=0;
  op_exec exec = executor[(size_t)instr->op];

  res = exec(instr, cpu);

  return res;
}
