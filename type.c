#include "Temple_cc.h"

Type *new_type(TypeKind kind) {
  Type *ty = calloc(1, sizeof(Type));
  ty->kind = kind;
  ty->size = 2;
  return ty;
}

Type *func_type(Type *return_ty) {
  Type *ty = calloc(1, sizeof(Type));
  ty->kind = TY_FUNC;
  ty->return_ty = return_ty;
  return ty;
}

bool is_integer(Type *ty) {
  return ty->kind == TY_INT;
}

Type *pointer_to(Type *base) {
  Type *ty = calloc(1, sizeof(Type));
  ty->kind = TY_PTR;
  ty->size = 2;
  ty->base = base;
  return ty;
}

Type *array_of(Type *base, int array_size) {
  Type *ty = calloc(1, sizeof(Type));
  ty->kind = TY_ARRAY;
  // ty->size = 2;
  ty->array_size = array_size;
  ty->base = base;
  return ty;
}

// ポインタの演算と整数型の演算では処理が変わるため
void add_type(Node *node) {
  if (!node || node->ty)
    return;

  add_type(node->lhs);
  add_type(node->rhs);
  add_type(node->cond);
  add_type(node->then);
  add_type(node->els);
  add_type(node->init);
  add_type(node->inc);

  for (Node *n = node->body; n; n = n->next)
    add_type(n);

  switch (node->kind) {
  case ND_ADD: // new_addでひっくり返すから問題なし
  case ND_SUB:
  case ND_MUL:
  case ND_DIV:
    node->ty = node->lhs->ty;
    return;
  case ND_ASSIGN: {
    TypeKind lvar_type = node->lhs->ty->kind;
    TypeKind rvar_type = node->rhs->ty->kind;
    if (lvar_type == rvar_type) {
      node->ty = node->lhs->ty;
      return;
    }
    if (lvar_type == TY_PTR && rvar_type == TY_ARRAY) {
      node->ty = node->lhs->ty;
      return;
    }
    error_at(node->lhs->tok->str, "代入不可");
  }
  case ND_EQ:
  case ND_NE:
  case ND_LT:
  case ND_LE:
  case ND_NUM:
  case ND_FUNCALL:
    node->ty = new_type(TY_INT);
    return;
  case ND_LVAR:
    node->ty = node->lvar->ty;
    return;
  case ND_ADDR:
    node->ty = pointer_to(node->lhs->ty);
    return;
  case ND_DEREF: {
    TypeKind lvar_type = node->lhs->ty->kind;
    if (lvar_type != TY_PTR && lvar_type != TY_ARRAY)
      error_at(node->lhs->tok->str, "間接演算子の型として無効");
    node->ty = node->lhs->ty->base;
    return;
  }
  default:
    return;
  }
}
