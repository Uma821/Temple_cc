#include "Temple_cc.h"

void gen(Node *node) {
  if (node->kind == ND_NUM) {
    printf("  "x64_push_immed("%d")"\n", node->val);
    return;
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
  case ND_NUM:
    break;
  }

  printf("  "x64_push_rgst("$r0")"\n");
}