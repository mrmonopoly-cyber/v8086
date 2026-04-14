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

  res.out = stdout;

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
      res.out = fopen(argv[i], "wa");
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

  fprintf(input.out, "bits %d\n\n", v8086RegSize(v8086));
  while(ProgramDumpNextInstr(v8086, pid, &instr) >= 0)
  {
    InstructionPrint(instr, input.out);
  }


end:
  v8086Shutdown(v8086);
  return res;
}
