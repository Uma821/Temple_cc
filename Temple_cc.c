#include <stdio.h>
#include <stdlib.h>
 
int main(int argc, char **argv) {
  if (argc != 2) {
    fprintf(stderr, "引数の個数が正しくありません\n");
    return 1;
  }

  printf("  seti main\n");
  printf("  move $t0\n");
  printf("  jl $t0 111 $ra\n");
  printf("main:\n");
  printf("  seti %d\n", atoi(argv[1]));
  printf("  move $r1\n");
  printf("  jl $allone 111 $ra\n");
  return 0;
}
