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
  s32 written =0;
  u8* mem_ptr = nullptr;

  if(index < prog->segment[CS].written)
  {
    physical_addr = AddrFromSegment(prog->segment[CS].log_seg, index);
    mem_ptr = PhyGetAddrAt(self.memory, physical_addr);
    written = InstructionDecode(mem_ptr, prog_length - index ,out);

    if(written)
    {
      res=0;
      prog->decoding_index += written;
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

int ProgramRun(v8086& self, ProgramID prog_id)
{
  int res=-1;
  Program* prog = &self.running[prog_id];
  const u32 prog_length = prog->segment[CS].written;
  u32 physical_addr = 0;
  s32 written =0;
  u8* mem_ptr = nullptr;
  Instruction instr = {};
  s32 err=0;
  u32 index=0;

  while(index < prog->segment[CS].written)
  {
    physical_addr = AddrFromSegment(prog->segment[CS].log_seg, index);
    mem_ptr = PhyGetAddrAt(self.memory, physical_addr);
    written = InstructionDecode(mem_ptr, prog_length - index, &instr);

    if(written)
    {
      res=0;
      if((err=InstructionExec(&instr, &self.cpu))<0)
      {
        fprintf(stderr, "execution error: %d with instr: ", err);
        InstructionPrint(instr, stderr);
        fprintf(stderr, "\n");
        res = err;
        break;
      }
      instr = {};
      index += written;
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

void V8086Dump(v8086& self, FILE* out)
{
  CPUPrint(&self.cpu, out);
}
