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
#define call(immed) "jl $zero 000 $t0\n  seti -2\n  add $sp\n  move $sp\n  seti 16\n  add $t0\n  sd $sp\n  seti "immed"\n  move $t0\n  jl $t0 111 $ra"
#define ret() "ld $sp\n  move $t0\n  seti 2\n  add $sp\n  move $sp\n  jl $t0 111 $ra"
#define x64_imul(rgst1, rgst2) "nor $allone\n  add "rgst1"\n  move $t1\n  nor $allone\n  add "rgst2"\n  move $t2\n  "call("MUL")"\n  nor $allone\n  add $t3\n  move "rgst1
#define x64_idiv(rgst1, rgst2) "nor $allone\n  add "rgst1"\n  move $t1\n  nor $allone\n  add "rgst2"\n  move $t2\n  "call("PRE_DIV")"\n  nor $allone\n  add $t3\n  move "rgst1

// トークンの種類
typedef enum {
  TK_RESERVED, // 記号
  TK_NUM,      // 整数トークン
  TK_EOF,      // 入力の終わりを表すトークン
} TokenKind;

typedef struct Token Token;

// トークン型
struct Token {
  TokenKind kind; // トークンの型
  Token *next;    // 次の入力トークン
  int val;        // kindがTK_NUMの場合、その数値
  char *str;      // トークン文字列
};

// 入力プログラム
char *user_input;

// 現在着目しているトークン
Token *token;

// エラーを報告するための関数
// printfと同じ引数を取る
void error(char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  vfprintf(stderr, fmt, ap);
  fprintf(stderr, "\n");
  exit(1);
}

// エラー箇所を報告する
void error_at(char *loc, char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);

  int pos = loc - user_input;
  fprintf(stderr, "%s\n", user_input);
  fprintf(stderr, "%*s", pos, ""); // pos個の空白を出力
  fprintf(stderr, "^ ");
  vfprintf(stderr, fmt, ap);
  fprintf(stderr, "\n");
  exit(1);
}
// 次のトークンが期待している記号のときには、トークンを1つ読み進めて
// 真を返す。それ以外の場合には偽を返す。
bool consume(char op) {
  if (token->kind != TK_RESERVED || token->str[0] != op)
    return false;
  token = token->next;
  return true;
}

// 次のトークンが期待している記号のときには、トークンを1つ読み進める。
// それ以外の場合にはエラーを報告する。
void expect(char op) {
  if (token->kind != TK_RESERVED || token->str[0] != op)
    error_at(token->str, "'%c'ではありません", op);
  token = token->next;
}

// 次のトークンが数値の場合、トークンを1つ読み進めてその数値を返す。
// それ以外の場合にはエラーを報告する。
int expect_number() {
  if (token->kind != TK_NUM)
    error_at(token->str, "数ではありません");
  int val = token->val;
  token = token->next;
  return val;
}

bool at_eof() {
  return token->kind == TK_EOF;
}

// 新しいトークンを作成してcurに繋げる
Token *new_token(TokenKind kind, Token *cur, char *str) {
  Token *tok = calloc(1, sizeof(Token));
  tok->kind = kind;
  tok->str = str;
  cur->next = tok;
  return tok;
}

// 入力文字列pをトークナイズしてそれを返す
Token *tokenize() {
  char *p = user_input;
  Token head;
  head.next = NULL;
  Token *cur = &head;

  while (*p) {
    // 空白文字をスキップ
    if (isspace(*p)) {
      p++;
      continue;
    }

    if (ispunct(*p)) {
      cur = new_token(TK_RESERVED, cur, p++);
      continue;
    }

    if (isdigit(*p)) {
      cur = new_token(TK_NUM, cur, p);
      cur->val = strtol(p, &p, 10);
      continue;
    }

    error_at(p, "トークナイズできません");
  }

  new_token(TK_EOF, cur, p);
  return head.next;
}

// 抽象構文木のノードの種類
typedef enum {
  ND_ADD, // +
  ND_SUB, // -
  ND_MUL, // *
  ND_DIV, // /
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

Node *new_node(NodeKind kind, Node *lhs, Node *rhs) {
  Node *node = calloc(1, sizeof(Node));
  node->kind = kind;
  node->lhs = lhs;
  node->rhs = rhs;
  return node;
}

Node *new_node_num(int val) {
  Node *node = calloc(1, sizeof(Node));
  node->kind = ND_NUM;
  node->val = val;
  return node;
}

Node *expr();
Node *mul();
Node *primary();

Node *expr() {
  Node *node = mul();

  for (;;) {
    if (consume('+'))
      node = new_node(ND_ADD, node, mul());
    else if (consume('-'))
      node = new_node(ND_SUB, node, mul());
    else
      return node;
  }
}

Node *mul() {
  Node *node = primary();

  for (;;) {
    if (consume('*'))
      node = new_node(ND_MUL, node, primary());
    else if (consume('/'))
      node = new_node(ND_DIV, node, primary());
    else
      return node;
  }
}

Node *primary() {
  // 次のトークンが"("なら、"(" expr ")"のはず
  if (consume('(')) {
    Node *node = expr();
    expect(')');
    return node;
  }

  // そうでなければ数値のはず
  return new_node_num(expect_number());
}

void gen(Node *node) {
  if (node->kind == ND_NUM) {
    printf("  "x64_push_immed("%d")"\n", node->val);
    return;
  }

  gen(node->lhs);
  gen(node->rhs);

  printf("  "x64_pop_rgst("$r1")"\n");
  printf("  "x64_pop_rgst("$r0")"\n");

  switch (node->kind) {
  case ND_ADD:
    printf("  "x64_add_rgst("$r0", "$r1")"\n");
    break;
  case ND_SUB:
    printf("  "x64_sub_rgst("$r0", "$r1")"\n");
    break;
  case ND_MUL:
    printf("  "x64_imul("$r0", "$r1")"\n");
    break;
  case ND_DIV:
    printf("  "x64_idiv("$r0", "$r1")"\n");
    break;
  }

  printf("  "x64_push_rgst("$r0")"\n");
}

int main(int argc, char **argv) {
  if (argc != 2) {
    fprintf(stderr, "引数の個数が正しくありません\n");
    return 1;
  }

    // トークナイズしてパースする
  user_input = argv[1];
  token = tokenize();
  Node *node = expr();

  // アセンブリの前半部分を出力
  printf("  seti main\n");
  printf("  move $t0\n");
  printf("  jl $t0 111 $ra\n");

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

  printf("PRE_DIV:\n"); // 除算関数DIVの前処理関数、割る数、割られる数が負の数の時に対応させるため
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
  printf("  "x64_push_rgst("$t2")"\n");
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


  printf("main:\n");

  // 抽象構文木を下りながらコード生成
  gen(node);

  // スタックトップに式全体の値が残っているはずなので
  // それをr0にロードして関数からの戻り値とする
  printf("  "x64_pop_rgst("$r0")"\n");
  printf("  jl $allone 111 $ra\n");
  return 0;
}
