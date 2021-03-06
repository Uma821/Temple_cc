#include "Temple_cc.h"

// 掛け算を行う
bool use_func_MUL = false;
// 割り算を行う
bool use_func_DIV = false;

int main(int argc, char **argv) {
  if (argc != 2) {
    fprintf(stderr, "引数の個数が正しくありません\n");
    return 1;
  }

  // トークナイズしてパースする
  // 結果はcodeに保存される
  user_input = argv[1];
  tokenize();
  Function *prog = parse();

  // アセンブリの前半部分を出力
  printf("  seti 10000\n");
  printf("  move $sp\n");
  printf("  "call("main")"\n");
  printf("  jl $allone 111 $ra\n");

  if (use_func_MUL) {
  // 乗算関数MUL ($t1(掛けられる数), $t2(掛ける数)に与えられた値の乗算結果を$t3に格納する)
  printf("MUL:\n");
  printf("  nor $allone\n");
  printf("  move $t3\n");
  printf("LP:\n");
  printf("  add $t2\n");
  printf("  move $t0\n");
  printf("  nor $allone\n");
  printf("  nor $t0\n");
  printf("  move $t0\n");
  printf("  nor $allone\n");
  printf("  nor $one\n");
  printf("  nor $t0\n");
  printf("  seti ZERO\n");
  printf("  move $t0\n");
  printf("  jl $t0 100 $ra\n");
  printf("  nor $allone\n");
  printf("  add $t1\n");
  printf("  add $t3\n");
  printf("  move $t3\n");
  printf("ZERO:\n");
  printf("  nor $allone\n");
  printf("  add $t1\n");
  printf("  add $t1\n");
  printf("  move $t1\n");
  printf("  nor $allone\n");
  printf("  add $t2\n");
  printf("  srl\n");
  printf("  move $t2\n");
  printf("  seti FIN_MUL\n");
  printf("  move $t0\n");
  printf("  jl $t0 100 $ra\n");
  printf("  seti LP\n");
  printf("  move $t0\n");
  printf("  jl $t0 111 $ra\n");
  printf("FIN_MUL:\n");
  printf("  "ret()"\n");
  }

  if (use_func_DIV) {
  // 除算関数DIVの前処理関数、割る数、割られる数が負の数の時に対応させるため
  printf("PRE_DIV:\n");
  printf("  seti T1_MINUS\n");
  printf("  move $t0\n");
  printf("  nor $allone\n");
  printf("  add $t1\n");
  printf("  jl $t0 010 $ra\n");
  printf("  seti T1_PLUS_T2_MINUS\n");
  printf("  move $t0\n");
  printf("  nor $allone\n");
  printf("  add $t2\n");
  printf("  jl $t0 010 $ra\n");
  printf("  "call("DIV")"\n");
  printf("  "ret()"\n");
  printf("T1_PLUS_T2_MINUS:\n");
  printf("  nor $zero\n");
  printf("  add $one\n");
  printf("  move $t2\n");
  printf("  "call("DIV")"\n");
  printf("  nor $allone\n");
  printf("  nor $t3\n");
  printf("  add $one\n");
  printf("  move $t3\n");
  printf("  "ret()"\n");
  printf("T1_MINUS:\n");
  printf("  nor $zero\n");
  printf("  add $one\n");
  printf("  move $t1\n");
  printf("  seti T1_MINUS_T2_MINUS\n");
  printf("  move $t0\n");
  printf("  nor $allone\n");
  printf("  add $t2\n");
  printf("  jl $t0 010 $ra\n");
  printf("  "call("DIV")"\n");
  printf("  nor $allone\n");
  printf("  nor $t3\n");
  printf("  add $one\n");
  printf("  move $t3\n");
  printf("  nor $allone\n");
  printf("  nor $t1\n");
  printf("  add $one\n");
  printf("  move $t1\n");
  printf("  "ret()"\n");
  printf("T1_MINUS_T2_MINUS:\n");
  printf("  nor $zero\n");
  printf("  add $one\n");
  printf("  move $t2\n");
  printf("  "call("DIV")"\n");
  printf("  nor $allone\n");
  printf("  nor $t1\n");
  printf("  add $one\n");
  printf("  move $t1\n");
  printf("  "ret()"\n");

  // 除算関数DIV ($t1(割られる数), $t2(割る数)に与えられた値の除算結果の商を$t3、余りを$t1に格納する)
  // 疑似命令用レジスタ不足のため、$ra、Mem[$sp-2]をちゃっかり使用する。割る数が0の場合無限ループ
  printf("DIV:\n");
  printf("  nor $allone\n");
  printf("  move $t3\n");
  printf("  add $one\n");
  printf("  move $t0\n");
  printf("  "push_rgst("$t2")"\n");
  printf("LP1:\n");
  printf("  nor $allone\n");
  printf("  nor $t2\n");
  printf("  add $one\n");
  printf("  add $t1\n");
  printf("  seti LP2\n");
  printf("  move $ra\n");
  printf("  jl $ra 010 $one\n");
  printf("  nor $allone\n");
  printf("  add $t2\n");
  printf("  add $t2\n");
  printf("  move $t2\n");
  printf("  nor $allone\n");
  printf("  add $t0\n");
  printf("  add $t0\n");
  printf("  move $t0\n");
  printf("  seti LP1\n");
  printf("  move $ra\n");
  printf("  jl $ra 111 $one\n");
  printf("LP2:\n");
  printf("  nor $allone\n");
  printf("  nor $t2\n");
  printf("  add $one\n");
  printf("  add $t1\n");
  printf("  seti NX3\n");
  printf("  move $ra\n");
  printf("  jl $ra 010 $one\n");
  printf("  nor $allone\n");
  printf("  add $t0\n");
  printf("  add $t3\n");
  printf("  move $t3\n");
  printf("  nor $allone\n");
  printf("  nor $t2\n");
  printf("  add $one\n");
  printf("  add $t1\n");
  printf("  move $t1\n");
  printf("NX3:\n");
  printf("  nor $allone\n");
  printf("  add $t2\n");
  printf("  srl\n");
  printf("  move $t2\n");
  printf("  nor $allone\n");
  printf("  add $t0\n");
  printf("  srl\n");
  printf("  move $t0\n");
  printf("  ld $sp\n");
  printf("  nor $zero\n");
  printf("  add $one\n");
  printf("  add $t1\n");
  printf("  seti FIN_DIV\n");
  printf("  move $ra\n");
  printf("  jl $ra 010 $one\n");
  printf("  seti LP2\n");
  printf("  move $ra\n");
  printf("  jl $ra 111 $one\n");
  printf("FIN_DIV:\n");
  printf("  seti 2\n");
  printf("  add $sp\n");
  printf("  move $sp\n");
  printf("  "ret()"\n");
  }

  codegen(prog);
  return 0;
}
