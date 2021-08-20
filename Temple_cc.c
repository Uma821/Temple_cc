#include <stdio.h>
#include <stdlib.h>
 
#define x64_move(rgst, immed) "seti "immed"\n  move "rgst
#define x64_add(rgst, immed) "seti "immed"\n  move $t0\n  nor $allone\n  add "rgst"\n  add $t0\n  move "rgst
#define x64_sub(rgst, immed) "seti "immed"\n  move $t0\n  nor $allone\n  nor $t0\n  add $one\n  add "rgst"\n  move "rgst

int main(int argc, char **argv) {
  if (argc != 2) {
    fprintf(stderr, "引数の個数が正しくありません\n");
    return 1;
  }

  char *p = argv[1];

  printf("  seti main\n");
  printf("  move $t0\n");
  printf("  jl $t0 111 $ra\n");
  printf("main:\n");
  printf("  "x64_move("$r0", "%d")"\n", (int)strtol(p, &p, 10));

  while (*p) {
    if (*p == '+') {
      p++;
      printf("  "x64_add("$r0", "%d")"\n", (int)strtol(p, &p, 10));
      continue;
    }

    if (*p == '-') {
      p++;
      printf("  "x64_sub("$r0", "%d")"\n", (int)strtol(p, &p, 10));
      continue;
    }

    fprintf(stderr, "予期しない文字です: '%c'\n", *p);
    return 1;
  }

  printf("  jl $allone 111 $ra\n");
  return 0;
}
