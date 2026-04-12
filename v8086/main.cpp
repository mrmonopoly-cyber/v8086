#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <string_view>

#include "v8086.h"

struct Inputs{
  FILE* out;
  const char* file_program_path;
};

static inline void _help(void)
{
  printf("usage: v8086 <-h> [bin_file]\n");
  printf("\t -h \t\t print _help\n");
  printf("\t -o [file] \t output file name\n");
}

static inline Inputs _parse_args(int argc, char **argv)
{
  Inputs res;
  ::std::string_view sw;

  for(int i=1; i<argc; i++)
  {
    sw = argv[i];
    if(!sw.compare("-h"))
    {
      _help();
      exit(0);
    }
    else if(!sw.compare("-o"))
    {
      i++;
      if(i>=argc)
      {
        _help();
        exit(1);
      }
      res.out = fopen(argv[i], "rb");
      if(res.out == nullptr)
      {
        fprintf(stderr, "invalid out print: %s\n", strerror(errno));
        exit(2);
      }
    }
    else
    {
      res.file_program_path = argv[i];
    }
  }

  if(res.file_program_path == nullptr)
  {
    printf("missing input program\n");
    _help();
    exit(1);
  }

  return res;
}

int main(int argc, char *argv[])
{
  Inputs input;

  v8086 v8086;
  ProgramInfo info;
  ProgramID pid;
  Instruction instr;

  input = _parse_args(argc, argv);

  v8086PowerOn(v8086);

  pid = loadProgram(v8086, info);
  for(u32 i=0; i<programLength(v8086, pid); i++)
  {
    instr = programDecodeInstrAt(v8086, pid, i);
    print(instr, input.out);
  }

  v8086Shutdown(v8086);

  return 0;
}
