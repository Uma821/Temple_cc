#include "Temple_cc.h"

void gen_lval(Node *node) {
  if (node->kind != ND_LVAR)
    error("代入の左辺値が変数ではありません");

  printf("  "x64_mov_rgst("$r0", x64_rbp)"\n");
  printf("  "x64_sub_immed("$r0", "%d")"\n", node->offset);
  printf("  "x64_push_rgst("$r0")"\n");
}

void gen(Node *node) {
  switch (node->kind) {
  case ND_NUM:
    printf("  "x64_push_immed("%d")"\n", node->val);
    return;
  case ND_LVAR:
    gen_lval(node);
    printf("  "x64_pop_rgst("$r0")"\n");
    printf("  "x64_mov_mem_to_rgst("$r0", "$r0")"\n");
    printf("  "x64_push_rgst("$r0")"\n");
    return;
  case ND_ASSIGN:
    gen_lval(node->lhs);
    gen(node->rhs);

    printf("  "x64_pop_rgst("$r1")"\n");
    printf("  "x64_pop_rgst("$r0")"\n");
    printf("  "x64_mov_rgst_to_mem("$r0", "$r1")"\n");
    printf("  "x64_push_rgst("$r1")"\n");
    return;
  default:
    break;
  }

  gen(node->lhs);
  gen(node->rhs);

  printf("  "x64_pop_rgst("$r1")"\n");
  printf("  "x64_pop_rgst("$r0")"\n");

  static int count_eq, count_ne, count_lt, count_le;

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
  case ND_EQ:
    ++count_eq;
    printf("  "x64_cmp("$r0", "$r1")"\n");
    printf("  "x64_sete("$r0")"\n", count_eq, count_eq);
    break;
  case ND_NE:
    ++count_ne;
    printf("  "x64_cmp("$r0", "$r1")"\n");
    printf("  "x64_setne("$r0")"\n", count_ne, count_ne);
    break;
  case ND_LT:
    ++count_lt;
    printf("  "x64_cmp("$r0", "$r1")"\n");
    printf("  "x64_setl("$r0")"\n", count_lt, count_lt);
    break;
  case ND_LE:
    ++count_le;
    printf("  "x64_cmp("$r0", "$r1")"\n");
    printf("  "x64_setle("$r0")"\n", count_le, count_le);
    break;
  default:
    break;
  }

  printf("  "x64_push_rgst("$r0")"\n");
}