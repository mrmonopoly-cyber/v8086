#include <stdio.h>
#include <string.h>
#define NOB_IMPLEMENTATION

extern "C"
{
#include "nob.h"
}

#ifndef BUILD_DIR
#define BUILD_DIR "build"
#endif // !BUILD_DIR

#ifndef SRC_DIR
#define SRC_DIR "src"
#endif // !SRC_DIR

#ifndef CPP_COMPILER
#define CPP_COMPILER "g++"
#endif // !CPP_COMPILER

#define COMPILER_ARGS \
  X(-Wall)\
  X(-Wextra)\
  X(-pedantic)\
  X(-xc++)\
  X(-std=c++17)\

#define LINKER_ARGS \
  X(-Wall)\
  X(-Wextra)\

#define O_FILE "main"

bool compile_obj(Nob_Walk_Entry entry)
{
  Cmd cmd = {0};
  size_t len = strlen(entry.path);

  nob_log(INFO, "considering %s", entry.path);

  if(
      entry.type == NOB_FILE_REGULAR &&
      (
        !strncmp(entry.path + len - 2, ".c", 2) ||
        !strncmp(entry.path + len - 4, ".cpp", 2)
      )
    )
  {
#define X(arg) #arg,
    cmd_append(&cmd, CPP_COMPILER, COMPILER_ARGS "-c", entry.path);
#undef X

    if (!cmd_run(&cmd)) return true;
  }

  return true;
}

int main(int argc, char **argv)
{
  GO_REBUILD_URSELF(argc, argv);

  Cmd cmd = {0};
  Dir_Entry dir = {0};
  char src_dir[PATH_MAX] = {0};
  char build_dir[PATH_MAX] = {0};

  const char* pwd = get_current_dir_temp();

  snprintf(src_dir, sizeof(src_dir), "%s/" SRC_DIR, pwd);
  snprintf(build_dir, sizeof(build_dir), "%s/" BUILD_DIR, pwd);

#define X(arg) #arg" "
  nob_log(INFO, "compile args: %s", COMPILER_ARGS);
  nob_log(INFO, "linker args: %s", LINKER_ARGS);
#undef X
  nob_log(INFO, "output_file: %s", O_FILE);
  nob_log(INFO, "src dir: %s", src_dir);
  nob_log(INFO, "build dir: %s", build_dir);

  mkdir_if_not_exists(BUILD_DIR);
  set_current_dir(BUILD_DIR);
  
  if(!walk_dir(src_dir, compile_obj))
  {
    printf("error compiling\n");
    return 1;
  }

#define X(arg) #arg, 
  cmd_append(&cmd, CPP_COMPILER, LINKER_ARGS "-o", O_FILE);
#undef X

  if(!dir_entry_open(build_dir, &dir))
  {
    printf("failed open build dir\n");
    return 2;
  }

  while(dir_entry_next(&dir))
  {
    int len = strlen(dir.name);
    if(
        strncmp(dir.name, "..", 2) &&
        strncmp(dir.name, ".", 1) &&
        !strncmp(dir.name + len - 2, ".o", 2))
    {
      cmd_append(&cmd, dir.name);
    }
  }
  dir_entry_close(dir);


  if (!cmd_run(&cmd)) return 1;

  set_current_dir(pwd);
  copy_file(BUILD_DIR"/main", "./main");
  return 0;
}
