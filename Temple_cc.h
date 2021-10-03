#define _XOPEN_SOURCE 700 // POSIX 関数の要求(strndup)
#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define rbp "$r22"

#define mov_immed(rgst, immed) "seti "immed"\n  move "rgst
#define mov_rgst(rgst1, rgst2) "nor $allone\n  add "rgst2"\n  move "rgst1
#define mov_mem_to_rgst(rgst, mem) "ld "mem"\n  move "rgst
#define mov_rgst_to_mem(mem, rgst) "nor $allone\n  add "rgst"\n  sd "mem
#define add_immed(rgst, immed) "seti "immed"\n  add "rgst"\n  move "rgst
#define add_rgst(rgst1, rgst2) "nor $allone\n  add "rgst1"\n  add "rgst2"\n  move "rgst1
#define sub_immed(rgst, immed) "seti "immed"\n  nor $zero\n  add $one\n  add "rgst"\n  move "rgst
#define sub_rgst(rgst1, rgst2) "nor $allone\n  nor "rgst2"\n  add $one\n  add "rgst1"\n  move "rgst1
#define push_immed(immed) "seti -2\n  add $sp\n  move $sp\n  seti "immed"\n  sd $sp"
#define push_rgst(rgst) "seti -2\n  add $sp\n  move $sp\n  nor $allone\n  add "rgst"\n  sd $sp"
#define pop(rgst) "ld $sp\n  move "rgst"\n  seti 2\n  add $sp\n  move $sp"
#define cmp(rgst1, rgst2) "nor $allone\n  nor "rgst2"\n  add $one\n  add "rgst1
#define cmp_immed(rgst, immed) "seti "immed"\n  nor $zero\n  add $one\n  add "rgst
#define sete(rgst)  "seti sete_end%d\n  move $ra\n  seti 1\n  move "rgst"\n  jl $ra 100 $one\n  add $allone\n  move "rgst"\nsete_end%d:"
#define setne(rgst) "seti setne_end%d\n  move $ra\n  seti 0\n  move "rgst"\n  jl $ra 100 $one\n  add $one\n  move "rgst"\nsetne_end%d:"
#define setl(rgst)  "seti setl_end%d\n  move $ra\n  seti 1\n  move "rgst"\n  jl $ra 010 $one\n  add $allone\n  move "rgst"\nsetl_end%d:"
#define setle(rgst) "seti setle_end%d\n  move $ra\n  seti 1\n  move "rgst"\n  jl $ra 110 $one\n  add $allone\n  move "rgst"\nsetle_end%d:"
#define je(lavel) "seti "lavel"\n  move $t0\n  jl $t0 100 $ra\n"
#define jmp(lavel) "seti "lavel"\n  move $t0\n  jl $t0 111 $ra\n"
#define call(immed) "jl $zero 000 $t0\n  seti -2\n  add $sp\n  move $sp\n  seti 16\n  add $t0\n  sd $sp\n  seti "immed"\n  move $t0\n  jl $t0 111 $ra"
#define ret() "ld $sp\n  move $t0\n  seti 2\n  add $sp\n  move $sp\n  jl $t0 111 $ra"
#define imul(rgst1, rgst2) "nor $allone\n  add "rgst1"\n  move $t1\n  nor $allone\n  add "rgst2"\n  move $t2\n  "call("MUL")"\n  nor $allone\n  add $t3\n  move "rgst1
#define idiv(rgst1, rgst2) "nor $allone\n  add "rgst1"\n  move $t1\n  nor $allone\n  add "rgst2"\n  move $t2\n  "call("PRE_DIV")"\n  nor $allone\n  add $t3\n  move "rgst1

typedef struct Type Type;
typedef struct Node Node;

//
// tokenize.c
//

// トークンの種類
typedef enum {
  TK_RESERVED, // 記号
  TK_IDENT,    // 識別子
  TK_NUM,      // 整数トークン
  TK_KEYWORD,  // キーワード
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
bool consume_keyword(char *op);
bool equal_keyword(char *op);
void expect(char *op);
int expect_number();
bool at_eof();
bool at_block();
Token *consume_ident();
void tokenize();

//
// parse.c
//

typedef struct LVar LVar;

// ローカル変数の型
struct LVar {
  LVar *next; // 次の変数かNULL
  char *name; // 変数の名前
  Type *ty;   // Type
  int len;    // 名前の長さ
  int offset; // RBPからのオフセット
};

// ローカル変数
extern LVar *locals;

// 抽象構文木のノードの種類
typedef enum {
  ND_ADD,     // +
  ND_SUB,     // -
  ND_MUL,     // *
  ND_DIV,     // /
  ND_ASSIGN,  // =
  ND_EQ,      // ==
  ND_NE,      // !=
  ND_LT,      // <
  ND_LE,      // <=
  ND_IF,      // if
  ND_LOOP,    // for,while
  ND_RETURN,  // return
  ND_BLOCK,   // { ... }
  ND_FUNCALL, // 関数呼び出し
  ND_ADDR,    // 単項*
  ND_DEREF,   // 単項&
  ND_LVAR,    // ローカル変数
  ND_NUM,     // 整数
} NodeKind;

typedef struct Node Node;

// 抽象構文木のノードの型
struct Node {
  NodeKind kind;  // ノードの型
  Node *next;     // 次のノード
  Token *tok;     // このノードのトークン
  Type *ty;       // そのノードのタイプ,intやintへのポインタなど
  Node *lhs;      // 左辺
  Node *rhs;      // 右辺
  int val;        // kindがND_NUMの場合のみ使う
  LVar *lvar;     // kindがND_LVARの場合のみ使う
  Node *cond;     // kindがND_IF,ND_LOOPの場合のみ使う
  Node *then;     // kindがND_IF,ND_LOOPの場合のみ使う
  Node *els;      // kindがND_IFの場合のみ使う
  Node *init;     // kindがND_LOOPの場合のみ使う
  Node *inc;      // kindがND_LOOPの場合のみ使う
  Node *body;     // kindがND_BLOCK(複文)の場合のみ使う
  char *funcname; // kindがND_FUNCALLの場合のみ使う
  Node *args;     // kindがND_FUNCALLの場合のみ使う
};

// 関数毎に内容、ローカル変数、ローカル変数用のスタックサイズの保存
typedef struct Function Function;
struct Function {
  Function *next;
  char *name;
  LVar *params;

  Node *body;
  LVar *locals;
  int stack_size;
};

Function *parse();

//
// type.c
//

typedef enum {
  TY_INT,
  TY_PTR,
  TY_FUNC,
} TypeKind;

struct Type {
  TypeKind kind;
  Type *base;  // 〇へのポインタ
  char *name;  // 定義
  Token *tok;  // 変数の位置情報など
  // 関数
  Type *return_ty;
  Type *params;
  Type *next;
};

Type *new_type(TypeKind kind);
Type *func_type(Type *return_ty);
bool is_integer(Type *ty);
Type *pointer_to(Type *base);
void add_type(Node *node);

//
// codegen.c
//

void codegen(Function *prog);
