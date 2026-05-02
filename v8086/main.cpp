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

static inline void _help(void)
{
  printf("usage: v8086 <-h> [bin_file]\n");
  printf("\t -h \t\tprint _help\n");
  printf("\t -o [file]\toutput file name. If not given stdout is used\n");
  printf("\t -d \t\tdisassemble the given program and save it on output file\n");
  printf("\t -r \t\trun the program\n");
  printf("\t -dump [seg]\tdump a segment after running the program to stdout\n");
  printf("\t -od [seg] [file]\tdump a segment after running the program to a file. Seg=SS,CS,DS,ES\n");
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

  while((arg=_next_argv()) != nullptr)
  {
    sw = arg;
    if(!sw.compare("-h"))
    {
      _help();
      exit(0);
    }
    else if(!sw.compare("-o"))
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
    else if(!sw.compare("-d"))
    {
      res.exec_options |= ExecOptions::Decode;
    }
    else if(!sw.compare("-r"))
    {
      res.exec_options |= ExecOptions::Run;
    }
    else if(!sw.compare("-dump"))
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
    else if(!sw.compare("-od"))
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

  if(input.exec_options & ExecOptions::Decode)
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
