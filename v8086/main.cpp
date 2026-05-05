#include <stdint.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <string_view>

#include "v8086.h"
#include "v8086_definitions.h"

enum ExecOptions : uint32_t
{
  Decode = 1 << 0,
  Run = 1 << 1,
  CyclesEstimation = 1 << 2,
  CyclesEstimationDetails = 1 << 3,
};

struct DumpSeg{
  FILE* o_file = nullptr;
};

struct Inputs{
  FILE* out;
  const char* file_program_path;
  uint32_t exec_options;
  DumpSeg dump_seg[Segment::__Num_Segment];
};

#define TAB_ALIGN_1 "\t"
#define TAB_ALIGN_2 "\t\t"
#define TAB_ALIGN_3 "\t\t\t"

static inline void _help(void)
{
  printf("usage: v8086 [options] [bin_file]\n");
  printf("options:\n");
  printf("\t -h " TAB_ALIGN_3 "print _help\n");
  printf("\t -o [file]" TAB_ALIGN_2 "output file name. If not given stdout is used\n");
  printf("\t -d " TAB_ALIGN_3 "disassemble the given program and save it on output file\n");
  printf("\t -cycles " TAB_ALIGN_2 "disassemble the given program and estimate the number of cycles for that program for each instruction\n");
  printf("\t -cycles_v " TAB_ALIGN_2 "like -cycles but with extra details\n");
  printf("\t -r " TAB_ALIGN_3 "run the program\n");
  printf("\t -dump [seg] " TAB_ALIGN_2 "dump a segment after running the program to stdout\n");
  printf("\t -od [seg] [file] " TAB_ALIGN_1 "dump a segment after running the program to a file. Seg=SS,CS,DS,ES\n");
}

static inline char* _next_argv(int argc=0, char** argv=nullptr)
{
  static int num_elements = 0;
  static int i=0;
  static char** pool = nullptr;
  char* res=nullptr;

  if(argv){
    pool = argv;
  }

  if(argc > 0)
  {
    num_elements = argc;
    i=1;
    return res;
  }

  if(pool && i < num_elements)
  {
    res = pool[i++];
  }

  return res;
}

static inline Inputs _parse_args(int argc, char **argv)
{
  Inputs res;
  ::std::string_view sw;
  ::std::string_view seg_str;
  char* o_file_path;
  Segment seg = Segment::SegNone;
  char* arg;

  _next_argv(argc, argv);

  res.out = stdout;

#define IF_ARG(STR) if(!sw.compare(#STR))
  while((arg=_next_argv()) != nullptr)
  {
    sw = arg;
    IF_ARG(-h)
    {
      _help();
      exit(0);
    }
    else IF_ARG(-o)
    {
      sw = _next_argv();
      if(sw.begin()==nullptr)
      {
        _help();
        exit(1);
      }
      res.out = fopen(sw.begin(), "wa");
      if(res.out == nullptr)
      {
        fprintf(stderr, "invalid out file %s : %s\n", sw.begin(), strerror(errno));
        exit(2);
      }
    }
    else IF_ARG(-d)
    {
      res.exec_options |= ExecOptions::Decode;
    }
    else IF_ARG(-cycles)
    {
      res.exec_options |= ExecOptions::CyclesEstimation;
    }
    else IF_ARG(-cycles_v)
    {
      res.exec_options |= ExecOptions::CyclesEstimationDetails;
    }
    else IF_ARG(-r)
    {
      res.exec_options |= ExecOptions::Run;
    }
    else IF_ARG(-dump)
    {
      if((seg_str = _next_argv()).begin() ==nullptr)
      {
        _help();
        exit(1);
      }
      if(!seg_str.compare("ss") || !seg_str.compare("SS")) seg =Segment::SS;
      else if(!seg_str.compare("ds") || !seg_str.compare("DS")) seg =Segment::DS;
      else if(!seg_str.compare("es") || !seg_str.compare("ES")) seg =Segment::ES;
      else if(!seg_str.compare("cs") || !seg_str.compare("CS")) seg =Segment::CS;
      else
      {
        fprintf(stderr, "invalid seg %s. Acceptable: SS,ss,ds,DS,es,ES,cs,CS\n", seg_str.begin());
        exit(2);
      }
      res.dump_seg[seg].o_file = stdout;
    }
    else IF_ARG(-od)
    {
      if((seg_str = _next_argv()).begin() ==nullptr || (o_file_path = _next_argv()) == nullptr)
      {
        _help();
        exit(1);
      }
      if(!seg_str.compare("ss") || !seg_str.compare("SS")) seg =Segment::SS;
      else if(!seg_str.compare("ds") || !seg_str.compare("DS")) seg =Segment::DS;
      else if(!seg_str.compare("es") || !seg_str.compare("ES")) seg =Segment::ES;
      else if(!seg_str.compare("cs") || !seg_str.compare("CS")) seg =Segment::CS;
      else
      {
        fprintf(stderr, "invalid seg %s. Acceptable: SS,ss,ds,DS,es,ES,cs,CS\n", seg_str.begin());
        exit(2);
      }
      res.dump_seg[seg].o_file = fopen(o_file_path, "aw");
      if(res.dump_seg[seg].o_file == nullptr)
      {
        fprintf(stderr, "invalid out file %s : %s\n", o_file_path, strerror(errno));
        exit(2);
      }
    }
    else
    {
      res.file_program_path = sw.begin();
    }
  }
#undef IF_ARG

  if(res.file_program_path == nullptr)
  {
    printf("missing input programOpt\n");
    _help();
    exit(1);
  }

  return res;
}

int main(int argc, char *argv[])
{
  int res=0;
  v8086 v8086;
  ProgramID pid;
  Instruction instr;

  Inputs input = _parse_args(argc, argv);

  if(!v8086PowerOn(v8086))
  {
    fprintf(stderr, "error init v8086\n");
    res = 1;
    goto end;
  }

  pid = ProgramLoad(v8086, input.file_program_path);

  if(pid<0)
  {
    fprintf(stderr, "error loading programOpt: %s with error %d\n", input.file_program_path, pid);
    res = 1;
    goto end;
  }

  if (input.exec_options & ExecOptions::CyclesEstimation || input.exec_options & ExecOptions::CyclesEstimationDetails)
  {
    u32 tot_cycles =0;
    while(ProgramDumpNextInstr(v8086, pid, &instr, DecodeOpt::Cycles) >= 0)
    {
      InstructionPrint(instr, input.out);
      if(input.exec_options & ExecOptions::CyclesEstimationDetails)
      {
        TODO("print cycles in details");
      }
      else
      {
        tot_cycles += instr.cycles;
        fprintf(input.out, " ; cycles = %d", instr.cycles);
      }
      fprintf(input.out, "\n");
    }
    fprintf(input.out, "tot cycles = %d\n", tot_cycles);
  
  }else if(input.exec_options & ExecOptions::Decode)
  {
    fprintf(input.out, "bits %d\n\n", v8086RegSize(v8086));
    while(ProgramDumpNextInstr(v8086, pid, &instr) >= 0)
    {
      InstructionPrint(instr, input.out);
      fprintf(input.out, "\n");
    }
  }

  fprintf(input.out, "\n\n");
  if(input.exec_options & ExecOptions::Run)
  {
    if((res=ProgramRun(v8086, pid, input.out, RunMode::Debug))<0)
    {
      fprintf(stderr, "error running the program: %d\n", res);
    }
    V8086Dump(v8086, pid, input.out);
    for(int i =0 ; i<Segment::__Num_Segment;i++)
    {
      if(input.dump_seg[i].o_file != nullptr)
      {
        V8086DumpSegment(v8086, pid, (Segment) i, input.dump_seg[i].o_file);
      }
    }
  }


end:
  v8086Shutdown(v8086);
  for(int i=0; i<Segment::__Num_Segment; i++)
  {
    if(input.dump_seg[i].o_file != nullptr && input.dump_seg[i].o_file != stdout)
    {
      fclose(input.dump_seg[i].o_file);
    }
  }
  return res;
}
