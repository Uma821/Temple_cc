#include "Temple_cc.h"

static char *argreg[] = {"$r1", "$r2", "$r3", "$r4", "$r5", "$r6"};
static Function *current_fn;

static void gen();

static int count(void) {
  static int i = 1;
  return i++;
}

// 'n'を'align'の最も近い倍数に切り上げる。
// align_to(5, 8) == 8, align_to(11, 8) == 16.
static int align_to(int n, int align) {
  return (n + align - 1) / align * align;
}

void gen_lval(Node *node) {
  switch (node->kind) {
  case ND_LVAR:
    printf("  "mov_rgst("$r0", rbp)"\n");
    printf("  "sub_immed("$r0", "%d")"\n", node->lvar->offset);
    // if (node->ty->kind == TY_ARRAY)fprintf(stderr, "配列です%d\n", node->lvar->offset);
    printf("  "push_rgst("$r0")"\n");
    return;
  case ND_DEREF:
    gen(node->lhs);
    return;

  default:
    error("代入の左辺値が変数ではありません");
  }

}

void gen(Node *node) {
  switch (node->kind) {
  case ND_NUM:
    printf("  "push_immed("%d")"\n", node->val);
    return;
  case ND_LVAR:
    gen_lval(node);
    if (node->ty->kind == TY_ARRAY) 
      return;
    printf("  "pop("$r0")"\n");
    printf("  "mov_mem_to_rgst("$r0", "$r0")"\n");
    printf("  "push_rgst("$r0")"\n");
    return;
  case ND_ADDR:
    gen_lval(node->lhs);
    return;
  case ND_DEREF:
    gen(node->lhs);
    printf("  "pop("$r0")"\n");
    printf("  "mov_mem_to_rgst("$r0", "$r0")"\n");
    printf("  "push_rgst("$r0")"\n");
    return;

  case ND_ASSIGN:
    gen_lval(node->lhs);
    gen(node->rhs);

    printf("  "pop("$r1")"\n");
    printf("  "pop("$r0")"\n");
    printf("  "mov_rgst_to_mem("$r0", "$r1")"\n");
    printf("  "push_rgst("$r1")"\n");
    return;
  case ND_BLOCK:
    for (Node *n = node->body; n; n = n->next) {
      gen(n);
      printf("  "pop("$r0")"\n");
    }
    printf("  "push_rgst("$r0")"\n");
    return;
  case ND_RETURN:
    gen(node->lhs);
    printf("  "pop("$r0")"\n");
    printf("  "jmp(".L.return.%s")"\n", current_fn->name);
    return;

  case ND_IF: {
    int c = count();
    gen(node->cond);
    printf("  "pop("$r0")"\n");
    printf("  "cmp_immed("$r0", "0")"\n");
    printf("  "je(".L.else.%d")"\n", c);
    gen(node->then);
    printf("  "pop("$r0")"\n");
    printf("  "jmp(".L.end.%d")"\n", c);
    printf(".L.else.%d:\n", c);
    if (node->els){
      gen(node->els);
      printf("  "pop("$r0")"\n");
    }
    printf(".L.end.%d:\n", c);
    printf("  "push_rgst("$r0")"\n");
    return;
  }
  case ND_LOOP: {
    int c = count();
    if (node->init) {
      gen(node->init);
      printf("  "pop("$r0")"\n");
    }
    printf(".L.begin.%d:\n", c);
    if (node->cond) {
      gen(node->cond);
      printf("  "pop("$r0")"\n");
      printf("  "cmp_immed("$r0", "0")"\n");
      printf("  "je(".L.end.%d")"\n", c);
    }
    gen(node->then);
    printf("  "pop("$r0")"\n");
    if (node->inc) {
      gen(node->inc);
      printf("  "pop("$r0")"\n");
    }
    printf("  "jmp(".L.begin.%d")"\n", c);
    printf(".L.end.%d:\n", c);
    printf("  "push_rgst("$r0")"\n");
    return;
  }
  case ND_FUNCALL: {
    int nargs = 0;
    for (Node *arg = node->args; arg; arg = arg->next) {
      gen(arg);
      nargs++;
    }

    for (int i = nargs - 1; i >= 0; i--)
      printf("  "pop("%s")"\n", argreg[i]);
    printf("  "mov_immed("$r0", "0")"\n");
    printf("  "call("%s")"\n", node->funcname);
    printf("  "push_rgst("$r0")"\n");
    return;
  }  
  default:
    break;
  }

  gen(node->lhs);
  gen(node->rhs);

  printf("  "pop("$r1")"\n");
  printf("  "pop("$r0")"\n");

  static int count_eq, count_ne, count_lt, count_le;

  switch (node->kind) {
  case ND_ADD:
    printf("  "add_rgst("$r0", "$r1")"\n");
    break;
  case ND_SUB:
    printf("  "sub_rgst("$r0", "$r1")"\n");
    break;
  case ND_MUL:
    printf("  "imul("$r0", "$r1")"\n");
    break;
  case ND_DIV:
    printf("  "idiv("$r0", "$r1")"\n");
    break;
  case ND_EQ:
    ++count_eq;
    printf("  "cmp("$r0", "$r1")"\n");
    printf("  "sete("$r0")"\n", count_eq, count_eq);
    break;
  case ND_NE:
    ++count_ne;
    printf("  "cmp("$r0", "$r1")"\n");
    printf("  "setne("$r0")"\n", count_ne, count_ne);
    break;
  case ND_LT:
    ++count_lt;
    printf("  "cmp("$r0", "$r1")"\n");
    printf("  "setl("$r0")"\n", count_lt, count_lt);
    break;
  case ND_LE:
    ++count_le;
    printf("  "cmp("$r0", "$r1")"\n");
    printf("  "setle("$r0")"\n", count_le, count_le);
    break;
  default:
    break;
  }

  printf("  "push_rgst("$r0")"\n");
}

// ローカル変数にオフセット割り当て
static void assign_lvar_offsets(Function *prog) {
  for (Function *fn = prog; fn; fn = fn->next) {
    int offset = 0;
    for (LVar *lvar = fn->locals; lvar; lvar = lvar->next) {
      offset += lvar->lvar_size;
      lvar->offset = offset;
    }
    fn->stack_size = align_to(offset, 2); // stack_sizeを2の倍数にする(RSPを16の倍数にしなければならないらしい)
  }
}

void codegen(Function *prog) {
  assign_lvar_offsets(prog);

  for (Function *fn = prog; fn; fn = fn->next) {
    // アセンブリの前半部分を出力
    printf("%s:\n", fn->name);
    current_fn = fn;
  
    // プロローグ
    // 使用した変数分の領域を確保する
    printf("  "push_rgst(rbp)"\n");
    printf("  "mov_rgst(rbp, "$sp")"\n");
    printf("  "sub_immed("$sp", "%d")"\n", fn->stack_size);

    // レジスタによって渡された引数をスタックに保存する
    int i = 0;
    for (LVar *lvar = fn->params; lvar; lvar = lvar->next) {
      printf("  "mov_rgst("$r0", rbp)"\n");
      printf("  "sub_immed("$r0", "%d")"\n", lvar->offset);
      printf("  "mov_rgst_to_mem("$r0", "%s")"\n", argreg[i++]);
    }

    gen(fn->body);
    printf("  "pop("$r0")"\n");
  
    // エピローグ
    // 最後の式の結果がRAXに残っているのでそれが返り値になる
    printf(".L.return.%s:\n", fn->name);
    printf("  "mov_rgst("$sp", rbp)"\n");
    printf("  "pop(rbp)"\n");
    printf("  "ret()"\n");
  }
}
