#include "cpu.h"
#include <v8086_definitions.h>

void print_arg(const Arg* const arg, FILE* out, const Segment seg)
{
  switch (arg->t)
  {
    case ArgReg:
      print_reg(arg->reg, out);
      break;
    case ArgMem:
      print_seg(seg, out);
      fprintf(out, "[%d]", arg->addr.addr);
      break;
    case ArgImm8:
      fprintf(out, "%d", arg->imm8);
      break;
    case ArgImm16:
      fprintf(out, "%d", arg->imm16);
      break;
    case ArgUImm8:
      fprintf(out, "%d", arg->uimm8);
      break;
    case ArgUImm16:
      fprintf(out, "%d", arg->uimm16);
      break;
    case ArgMemRegDisp:
      print_seg(seg, out);
      fprintf(out, "[");
      print_reg(arg->reg_disp.r1, out);
      print_disp(&arg->reg_disp.disp, out);
      fprintf(out, "]");

      break;
    case ArgMemRegRegDisp:
      print_seg(seg, out);
      fprintf(out, "[");
      print_reg(arg->reg_reg_disp.r1, out);
      fprintf(out, "+");
      print_reg(arg->reg_reg_disp.r2, out);
      print_disp(&arg->reg_reg_disp.disp, out);
      fprintf(out, "]");
      break;

    case ArgSegment:
      switch (arg->seg)
      {
        case ES:
          fprintf(out, "es");
          break;
        case SS:
          fprintf(out, "ss");
          break;
        case CS:
          fprintf(out, "cs");
          break;
        case DS:
          fprintf(out, "ds");
          break;
        case SegNone:
          assert(0 && "unreachable _print_arg arg segment None");
          break;
        case __Num_Segment:
          assert(0 && "unreachable _print_arg arg segment __Num_Segment");
          break;
        }
      break;

    case ArgDirInterSeg:
      fprintf(out, "%u:%u\n", arg->dir_inter_seg.addr[0], arg->dir_inter_seg.addr[1]);
      break;

    case ArgIpInc8:
      fprintf(out, "%+d", arg->ip_inc_8);
      break;
    case ArgIpInc16:
      fprintf(out, "%+d", arg->ip_inc_16);
      break;

    case ArgInvalid:
      break;
    }
}

void print_reg(const Register reg, FILE* out)
{
  switch (reg)
  {
#define X(r) case dec_##r: fprintf(out, #r); break;
  REGS
#undef X
    case INVALID_REG:
      assert(0 && "unreachable print_reg: " __FILE__);
      break;
  }
}

void print_disp(const Displacement* disp, FILE *out)
{
  if(disp->t == Disp8 && disp->disp8)
  {
    fprintf(out, "%+d", disp->disp8);
  }
  else if(disp->t == Disp16 && disp->disp16)
  {
    fprintf(out, "%+d", disp->disp16);
  }
}

void print_seg(const Segment seg, FILE* out)
{
  switch (seg)
  {
    case ES:
      fprintf(out, "es:");
      break;
    case CS:
      fprintf(out, "cs:");
      break;
    case SS:
      fprintf(out, "ss:");
      break;
    case DS:
      fprintf(out, "ds:");
      break;
    case SegNone:
      break;
    case __Num_Segment:
      break;
    }
}

void InstructionPrint(const Instruction& instr, FILE* out_f)
{
  assert(out_f);
  const Displacement* disp = nullptr;
  u8 far=0;

  switch (instr.prefix)
  {
#define X(prefix) case Prefix::prefix:  fprintf(out_f, #prefix" "); break;
    PREFIXES
#undef X
  case Prefix::None:
    break;
  }

  switch (instr.op)
  {
#define X(OP) case Opcode::OP:  fprintf(out_f, #OP); break;
    OPS
#undef X
    case Opcode::OpNot: fprintf(out_f, "not"); break;
    case Opcode::OpAnd: fprintf(out_f, "and"); break;
    case Opcode::OpOr: fprintf(out_f, "or"); break;
    case Opcode::OpXor: fprintf(out_f, "xor"); break;
    case Opcode::OpInt: fprintf(out_f, "int");break;

    case Opcode::INVALID: 
      fprintf(out_f, "INVALID OP");
      break;
    case Opcode::__Opcode_count:
      assert(0 && "unreachable InstructionPrint");
      break;
    }

  fprintf(out_f, " ");

  if(instr.args[0].t == ArgMemRegDisp)
  {
    disp = &instr.args[0].reg_disp.disp;
    far = instr.args[0].reg_disp.disp.far;
  }
  else if(instr.args[0].t == ArgMemRegRegDisp)
  {
    disp = &instr.args[0].reg_reg_disp.disp;
    far = instr.args[0].reg_reg_disp.disp.far;
  }

  if(far)
  {
    fprintf(out_f, "far ");
  }

  if(disp)
  {
    if(disp->word || instr.op == Opcode::push || instr.op == Opcode::pop)
    {
      fprintf(out_f, "word ");
    }
    else
    {
      fprintf(out_f, "byte ");
    }
  }
  else if(instr.args[0].t == ArgMem)
  {
    if(instr.args[0].addr.word || instr.op == Opcode::push || instr.op == Opcode::pop)
    {
      fprintf(out_f, "word ");
    }
    else
    {
      fprintf(out_f, "byte ");
    }
  }


  print_arg(&instr.args[0], out_f, instr.seg);
  if(instr.args[1].t)
  {
    fprintf(out_f, ", ");
    print_arg(&instr.args[1], out_f,  instr.seg);
  }
}

void CPUPrint(CPU* cpu, FILE* out)
{
  for(size_t r=0; r<__reg_count; r++)
  {
    switch ((FullRegs)r)
    {
      case ax:
        fprintf(out, "ax: 0x%04x\tah: 0x%02x\tal: 0x%02x",
            cpu->regs[r]._u16, cpu->regs[r]._half.h, cpu->regs[r]._half.l);
        break;
      case bx:
        fprintf(out, "bx: 0x%04x\tbh: 0x%02x\tbl: 0x%02x",
            cpu->regs[r]._u16, cpu->regs[r]._half.h, cpu->regs[r]._half.l);
        break;
      case cx:
        fprintf(out, "cx: 0x%04x\tch: 0x%02x\tcl: 0x%02x",
            cpu->regs[r]._u16, cpu->regs[r]._half.h, cpu->regs[r]._half.l);
        break;
      case dx:
        fprintf(out, "dx: 0x%04x\tdh: 0x%02x\tdl: 0x%02x",
            cpu->regs[r]._u16, cpu->regs[r]._half.h, cpu->regs[r]._half.l);
        break;
      case si:
        fprintf(out, "si: 0x%04x", cpu->regs[r]._u16);
        break;
      case di:
        fprintf(out, "di: 0x%04x", cpu->regs[r]._u16);
        break;
      case sp:
        fprintf(out, "sp: 0x%04x", cpu->regs[r]._u16);
        break;
      case bp:
        fprintf(out, "bp: 0x%04x", cpu->regs[r]._u16);
        break;
      case __reg_count:
        assert(0 && "__reg_count not printable");
        break;
    }
    fprintf(out, "\t(%d)\n", cpu->regs[r]._u16);
  }

  fprintf(out, "IP: 0x%04x\t(%d)\n", cpu->ip, cpu->ip);

  fprintf(out, "FLAGS:");
  flag_print_all(cpu->flags, out);
  fprintf(out, "\n");

}
