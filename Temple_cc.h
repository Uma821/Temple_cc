#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define x64_mov(rgst, immed) "seti "immed"\n  move "rgst
#define x64_add_immed(rgst, immed) "seti "immed"\n  move $t0\n  nor $allone\n  add "rgst"\n  add $t0\n  move "rgst
#define x64_add_rgst(rgst1, rgst2) "nor $allone\n  add "rgst1"\n  add "rgst2"\n  move "rgst1
#define x64_sub_immed(rgst, immed) "seti "immed"\n  move $t0\n  nor $allone\n  nor $t0\n  add $one\n  add "rgst"\n  move "rgst
#define x64_sub_rgst(rgst1, rgst2) "nor $allone\n  nor "rgst2"\n  add $one\n  add "rgst1"\n  move "rgst1
#define x64_push_immed(immed) "seti -2\n  add $sp\n  move $sp\n  seti "immed"\n  sd $sp"
#define x64_push_rgst(rgst) "seti -2\n  add $sp\n  move $sp\n  nor $allone\n  add "rgst"\n  sd $sp"
#define x64_pop_rgst(rgst) "ld $sp\n  move "rgst"\n  seti 2\n  add $sp\n  move $sp"
#define x64_cmp(rgst1, rgst2) "nor $allone\n  nor "rgst2"\n  add $one\n  add "rgst1
#define x64_sete(rgst)  "seti sete_end%d\n  move $ra\n  seti 1\n  move "rgst"\n  jl $ra 100 $one\n  add $allone\n  move "rgst"\nsete_end%d:"
#define x64_setne(rgst) "seti setne_end%d\n  move $ra\n  seti 0\n  move "rgst"\n  jl $ra 100 $one\n  add $one\n  move "rgst"\nsetne_end%d:"
#define x64_setl(rgst)  "seti setl_end%d\n  move $ra\n  seti 1\n  move "rgst"\n  jl $ra 010 $one\n  add $allone\n  move "rgst"\nsetl_end%d:"
#define x64_setle(rgst) "seti setle_end%d\n  move $ra\n  seti 1\n  move "rgst"\n  jl $ra 110 $one\n  add $allone\n  move "rgst"\nsetle_end%d:"
#define call(immed) "jl $zero 000 $t0\n  seti -2\n  add $sp\n  move $sp\n  seti 16\n  add $t0\n  sd $sp\n  seti "immed"\n  move $t0\n  jl $t0 111 $ra"
#define ret() "ld $sp\n  move $t0\n  seti 2\n  add $sp\n  move $sp\n  jl $t0 111 $ra"
#define x64_imul(rgst1, rgst2) "nor $allone\n  add "rgst1"\n  move $t1\n  nor $allone\n  add "rgst2"\n  move $t2\n  "call("MUL")"\n  nor $allone\n  add $t3\n  move "rgst1
#define x64_idiv(rgst1, rgst2) "nor $allone\n  add "rgst1"\n  move $t1\n  nor $allone\n  add "rgst2"\n  move $t2\n  "call("PRE_DIV")"\n  nor $allone\n  add $t3\n  move "rgst1

//
// tokenize.c
//

// トークンの種類
typedef enum {
  TK_RESERVED, // 記号
  TK_NUM,      // 整数トークン
  TK_EOF,      // 入力の終わりを表すトークン
} TokenKind;

// トークン型
typedef struct Token Token;
struct Token {
  TokenKind kind; // トークンの型
  Token *next;    // 次の入力トークン
  int val;        // kindがTK_NUMの場合、その数値
  char *str;      // トークン文字列
  int len;        // トークンの長さ
};

// 入力プログラム
extern char *user_input;

// 現在着目しているトークン
extern Token *token;

void error(char *fmt, ...);
void error_at(char *loc, char *fmt, ...);
bool consume(char *op);
void expect(char *op);
int expect_number();
Token *tokenize();

//
// parse.c
//

// 抽象構文木のノードの種類
typedef enum {
  ND_ADD, // +
  ND_SUB, // -
  ND_MUL, // *
  ND_DIV, // /
  ND_EQ,  // ==
  ND_NE,  // !=
  ND_LT,  // <
  ND_LE,  // <=
  ND_NUM, // 整数
} NodeKind;

typedef struct Node Node;

// 抽象構文木のノードの型
struct Node {
  NodeKind kind; // ノードの型
  Node *lhs;     // 左辺
  Node *rhs;     // 右辺
  int val;       // kindがND_NUMの場合のみ使う
};

Node *expr();

//
// codegen.c
//

void gen();