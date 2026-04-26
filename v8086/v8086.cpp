#include "v8086.h"
#include "decoder/decoder.h"
#include "exec/exec.h"
#include "memory/memory.h"
#include "memory/physical.h"
#include "v8086_definitions.h"

#include <assert.h>
#include <string.h>
#include <errno.h>
#include <stdio.h>

static ProgramID next_prog_id(v8086& self)
{
  ProgramID res=-1;

  if(self.next_free < PhyMemoryCapacity(self.memory))
  {
    res = self.next_free++;
  }

  //HACK: only one program can be loaded
  if(res > 0)
  {
    res = -1;
    self.next_free--;
  }

  return res;
}


static ProgramSegment _seg_init(PhyMemory& mem, u32 *physical_addr, const u32 size)
{
  ProgramSegment res;

  u32 paddr = *physical_addr;
  (*physical_addr) += size;

  res.log_seg = MakeSegment(mem, paddr);
  res.written = 0;

  return res;
}

static inline void _get_seg_view(v8086* self, ProgramSegment seg, SegmentView* sw)
{
  u32 physical_addr;
  u8* mem_ptr;

  physical_addr = AddrFromSegment(seg.log_seg);
  mem_ptr = PhyGetAddrAt(self->memory, physical_addr);

  sw->data = mem_ptr;
  sw->len = seg.written;
}

ProgramID ProgramLoad(v8086& self, const char* file_program_path, const ProgramOptInfo& prog_info)
{
  ProgramID res = -1;
  FILE* program_f = nullptr;
  Program* prog = nullptr;
  u8* phy_addr = nullptr;
  u32 physical_addr =0; //INFO: at the moment only one program is loadable

  assert(PhyMemoryIsValid(self.memory));

  if(file_program_path == nullptr)
  {
    return -5;
  }

  res = next_prog_id(self);

  if(res >= 0)
  {
    prog = &self.running[res];

    prog->segment[CS] = _seg_init(self.memory, &physical_addr, prog_info.cs_size);
    prog->segment[SS] = _seg_init(self.memory, &physical_addr, prog_info.ss_size);
    prog->segment[DS] = _seg_init(self.memory, &physical_addr, prog_info.ds_size);
    prog->segment[ES] = _seg_init(self.memory, &physical_addr, prog_info.es_size);

    physical_addr = AddrFromSegment(prog->segment[CS].log_seg);
    phy_addr = PhyGetAddrAt(self.memory, physical_addr);

    program_f = fopen(file_program_path, "rb");

    if(program_f)
    {
      //FIX: you may write more data than what is available in the segment.
      //Correct from the point of view of 8086 but still dangerous. Keep it?
      prog->segment[CS].written =fread(phy_addr,1, prog_info.cs_size, program_f);
      fclose(program_f);
    }
    else
    {

      fprintf(stderr, "error opening input file: %s with error %s\n",
          file_program_path, strerror(errno));
        self.next_free--;
        res = -errno;
        return res;
    }
  }

  return res;
}

int ProgramDumpNextInstr(v8086& self,const ProgramID prog_id, Instruction* out)
{
  assert(out);
  assert(prog_id >=0 && prog_id < MAX_NUM_OF_PROGRAMS );

  int res=-1;
  Program* prog = &self.running[prog_id];
  const u32 prog_length = prog->segment[CS].written;
  u32 index = prog->decoding_index;
  u32 physical_addr = 0;
  s32 status =0;
  u8* mem_ptr = nullptr;

  if(index < prog->segment[CS].written)
  {
    physical_addr = AddrFromSegment(prog->segment[CS].log_seg, index);
    mem_ptr = PhyGetAddrAt(self.memory, physical_addr);
    status = InstructionDecode(mem_ptr, prog_length - index ,out);

    if(status)
    {
      res=0;
      prog->decoding_index += out->size;
    }
    else
    {
      res = -2;
      fprintf(stderr, "Mnemonic not recognized: ");
      _print_byte(*mem_ptr, stderr);
      fprintf(stderr, "\n");
    }
  }

  return res;
}

int ProgramRun(v8086& self, ProgramID prog_id, RunMode mode)
{
  int res=-1;
  Program* prog = &self.running[prog_id];
  const u32 prog_length = prog->segment[CS].written;
  u32 physical_addr = 0;
  u8* mem_ptr = nullptr;
  Instruction instr = {};
  s32 err=0;
  SegmentView segs[__Num_Segment];
  u16 old_val, new_val;
  u8 old_flags = self.cpu.flags;

  for(size_t i=0; i<__Num_Segment; i++)
  {
    _get_seg_view(&self, prog->segment[i], &segs[i]);
  }

  while(self.cpu.ip < prog->segment[CS].written)
  {
    physical_addr = AddrFromSegment(prog->segment[CS].log_seg, self.cpu.ip);
    mem_ptr = PhyGetAddrAt(self.memory, physical_addr);

    err = InstructionDecode(mem_ptr, prog_length - self.cpu.ip, &instr);
    

    if(err)
    {
      res=0;
      if((err=InstructionExec(&instr, &self.cpu, segs, &old_val, &new_val))<0)
      {
        fprintf(stderr, "execution error: %d with instr: ", err);
        InstructionPrint(instr, stderr);
        fprintf(stderr, "\n");
        res = err;
        break;
      }
      else if(mode == RunMode::Debug)
      {
        InstructionPrint(instr);
        fprintf(stdout, "\t ; ");
        if(old_val != new_val)
        {
          print_arg(&instr.args[0], stdout, instr.seg);
          fprintf(stdout, " = 0x%x -> 0x%x", old_val, new_val);
          fprintf(stdout, "\t");
        }
        if(old_flags != self.cpu.flags)
        {
          fprintf(stdout, "FLAGS: ");
          flag_print(old_flags, stdout);
          fprintf(stdout, " -> ");
          flag_print(self.cpu.flags, stdout);
        }
        fprintf(stdout, "\n");
      }
      old_flags = self.cpu.flags;
    }
    else
    {
      fprintf(stderr, "Mnemonic not recognized: ");
      _print_byte(*mem_ptr, stderr);
      fprintf(stderr, "\n");
      res = -2;
      break;
    }
  }

  return res;
}

void V8086Dump(v8086& self, ProgramID prog_id, FILE* out)
{
  if(out == nullptr) out = stdout;
  CPUPrint(&self.cpu, out);
  Program* prog = &self.running[prog_id];
  if(prog_id>=0)
  {
    for(size_t i=0; i<__Num_Segment; i++)
    {
      switch ((Segment)i)
      {
        case ES:
          fprintf(out, "ES");
          break;
        case CS:
          fprintf(out, "CS");
          break;
        case SS:
          fprintf(out, "SS");
          break;
        case DS:
          fprintf(out, "DS");
          break;
        case SegNone:
          assert(0 && "unreachable V8086Dump, SegNone");
          break;
        case __Num_Segment:
          assert(0 && "unreachable V8086Dump, __Num_Segment");
          break;
      }
      fprintf(out, " segment\n");
      SegmentPrint(&prog->segment[i].log_seg, &self.memory);
    }
  }

}
