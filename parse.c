#include "Temple_cc.h"

LVar *locals;

// 変数を名前で検索する。見つからなかった場合はNULLを返す。
static LVar *find_lvar(Token *tok) {
  for (LVar *var = locals; var; var = var->next)
    if (var->len == tok->len && !memcmp(tok->str, var->name, var->len))
      return var;
  return NULL;
}

static Node *new_node(NodeKind kind, Node *lhs, Node *rhs) {
  Node *node = calloc(1, sizeof(Node));
  node->kind = kind;
  node->lhs = lhs;
  node->rhs = rhs;
  node->tok = token;
  return node;
}

static Node *new_node_num(int val) {
  Node *node = calloc(1, sizeof(Node));
  node->kind = ND_NUM;
  node->val = val;
  node->tok = token;
  return node;
}

static Node *new_node_lvar(LVar *lvar, Token *tok) {
  Node *node = calloc(1, sizeof(Node));
  node->kind = ND_LVAR;
  node->lvar = lvar;
  node->tok = tok;
  return node;
}

static int lvar_size(Type *ty) {
  if (ty->kind != TY_ARRAY) 
    return 2;
  return ty->array_size * lvar_size(ty->base);
}

static LVar *new_lvar(char *name, Type *ty) {
  LVar *lvar = calloc(1, sizeof(LVar));
  lvar->name = name;
  lvar->ty = ty;
  lvar->len = strlen(name);
  lvar->lvar_size = lvar_size(ty);
  lvar->next = locals;
  locals = lvar;
  return lvar;
}

static char *get_ident() {
  if (token->kind != TK_IDENT)
    error_at(token->str, "関数名が必要");
  Token *tok = token;
  token = token->next;
  return strndup(tok->str, tok->len);
}

// decl_basictype = "int"
// 変数宣言のことも考慮して別の関数に振り分けた
static Type *decl_basictype() {
  if (!consume_keyword("int"))
    error_at(token->str, "型名が必要");
  return new_type(TY_INT);
}

static Type *declarator(Type *ty);
Function *parse();
static Node *stmt();
static Node *expr();
static Node *assign();
static Node *equality();
static Node *relational();
static Node *add();
static Node *mul();
static Node *unary();
static Node *primary();

// type-suffix = ("(" func-params? ")")?
// func-params = param ("," param)*
// param       = declarator
static Type *type_suffix(Type *ty) {
  if (consume("(")) {
    Type head = {};
    Type *cur = &head;

    while (!consume(")")) {
      Type *basety = decl_basictype();
      Type *ty = declarator(basety);
      cur = cur->next = ty;
      consume(",");
    }

    ty = func_type(ty);
    ty->params = head.next;
  }
  return ty;
}

// declarator = decl_basictype "*" * ident type-suffix
static Type *declarator(Type *ty) { // 宣言
  while (consume("*"))
    ty = pointer_to(ty);

  Token *tok_lval = token;
  char* name = get_ident();

  while (consume("[")) {
    int array_size = expect_number();
    ty = array_of(ty, array_size);
    expect("]");
  }
  
  ty = type_suffix(ty); // 関数の宣言だった時の引数読み込み等
  ty->name = name;
  ty->tok = tok_lval;
  return ty;
}

// declaration = decl_basictype (declarator ("=" expr)? ("," declarator ("=" expr)?)*)? ";"
static Node *declaration() {
  Type *basety = decl_basictype();

  Node head = {};
  Node *cur = &head;

  while (!consume(";")) {
    Type *ty = declarator(basety);
    LVar *lvar = new_lvar(ty->name, ty);

    if (consume("=")) {
      Node *lhs = new_node_lvar(lvar, ty->tok);
      cur = cur->next = new_node(ND_ASSIGN, lhs, assign());
    }

    consume(",");
  }

  Node *node = new_node(ND_BLOCK, NULL, NULL);
  node->body = head.next;
  return node;
}

static void create_param_lvars(Type *param) {
  if (param) {
    create_param_lvars(param->next);
    new_lvar(param->name, param);
  }
}

static Function *function() {
  Type *ty = decl_basictype();
  ty = declarator(ty);

  locals = NULL; // NULLのときが終端とするため

  Function *fn = calloc(1, sizeof(Function));
  fn->name = ty->name;
  create_param_lvars(ty->params);
  fn->params = locals;

  if (!at_block())
    error("中括弧で覆われていません。");
  fn->body = stmt();
  fn->locals = locals;
  return fn;
}

// program = stmt*
Function *parse() {
  Function head = {};
  Function *cur = &head;

  while (!at_eof())
    cur = cur->next = function();
    //error("正しくパースできませんでした。");
  return head.next;
}

// stmt = expr? ";"
//      | "{" stmt* "}"
//      | "if" "(" expr ")" stmt ("else" stmt)?
//      | "for" "(" expr? ";" expr? ";" expr? ")" stmt
//      | "while" "(" expr ")" stmt
//      | "return" expr ";"
static Node *stmt() {
  Node *node;

  if (consume("{")) {
    Node head;
    head.next = NULL;
    Node *cur = &head;
    while (!consume("}")) {
      if (equal_keyword("int"))
        cur = cur->next = declaration();
      else
        cur = cur->next = stmt();
      add_type(cur);
    }
    
    node = new_node(ND_BLOCK, NULL, NULL);
    node->body = head.next;
    return node;
  } else if (consume_keyword("if")) {
    node = new_node(ND_IF, NULL, NULL);
    expect("(");
    node->cond = expr();
    expect(")");
    node->then = stmt();
    if (consume_keyword("else"))
      node->els = stmt();
    return node;
  } else if (consume_keyword("for")) {
    node = new_node(ND_LOOP, NULL, NULL);
    expect("(");
    node->init = stmt(); // ";"の部分も含めてstmt
    if (!consume(";")) {
      node->cond = expr();
      expect(";");
    }
    if (!consume(")")) {
      node->inc = expr();
      expect(")");
    }
    node->then = stmt();
    return node;
  } else if (consume_keyword("while")) {
    node = new_node(ND_LOOP, NULL, NULL);
    expect("(");
    node->cond = expr();
    expect(")");
    node->then = stmt();
    return node;
  } else if (consume_keyword("return")) {
    node = new_node(ND_RETURN, expr(), NULL);
  } else if (consume(";")) { // ";"だけの文
    return new_node(ND_BLOCK, NULL, NULL); // 空のブロック
  } else {
    node = expr();
  }

  expect(";");
  return node;
}

// expr = assign
static Node *expr() {
  return assign();
}

// assign = equality ("=" assign)?
static Node *assign() {
  Node *node = equality();
  if (consume("="))
    node = new_node(ND_ASSIGN, node, assign());
  return node;
}

// equality = relational ("==" relational | "!=" relational)*
static Node *equality() {
  Node *node = relational();

  for (;;) {
    if (consume("=="))
      node = new_node(ND_EQ, node, relational());
    else if (consume("!="))
      node = new_node(ND_NE, node, relational());
    else
      return node;
  }
}

// relational = add ("<" add | "<=" add | ">" add | ">=" add)*
static Node *relational() {
  Node *node = add();

  for (;;) {
    if (consume("<"))
      node = new_node(ND_LT, node, add());
    else if (consume("<="))
      node = new_node(ND_LE, node, add());
    else if (consume(">"))
      node = new_node(ND_LT, add(), node);
    else if (consume(">="))
      node = new_node(ND_LE, add(), node);
    else
      return node;
  }
}

// ポインタ演算の場合、pをポインタ、nを整数とすると
// p + nはpにnをそのまま加算するのではなく、
// sizeof(*p)*nをpの値に加算する。
static Node *new_add(Node *lhs, Node *rhs) {
  add_type(lhs);
  add_type(rhs);

  // num + num
  if (is_integer(lhs->ty) && is_integer(rhs->ty))
    return new_node(ND_ADD, lhs, rhs);

  // ptr + ptr は計算できない
  if (lhs->ty->base && rhs->ty->base)
    error_at(token->str, "無効な演算");

  // num + ptr を ptr + num にひっくり返す.
  if (!lhs->ty->base && rhs->ty->base) {
    Node *tmp = lhs;
    lhs = rhs;
    rhs = tmp;
  }

  // ptr + num
  use_func_MUL = true; // 現状シフト演算子を使わず掛け算として処理するため
  rhs = new_node(ND_MUL, rhs, new_node_num(2)); // シフト演算子を定義したら変更する
  return new_node(ND_ADD, lhs, rhs);
}

// -演算子にもポインタと整数に対しての演算がある
static Node *new_sub(Node *lhs, Node *rhs) {
  add_type(lhs);
  add_type(rhs);

  // num - num
  if (is_integer(lhs->ty) && is_integer(rhs->ty))
    return new_node(ND_SUB, lhs, rhs);

  // ptr - num
  if (lhs->ty->base && is_integer(rhs->ty)) {
    use_func_MUL = true; // 現状シフト演算子を使わず掛け算として処理するため
    rhs = new_node(ND_MUL, rhs, new_node_num(2)); // シフト演算子を定義したら変更する
    add_type(rhs);
    Node *node = new_node(ND_SUB, lhs, rhs);
    node->ty = lhs->ty;
    return node;
  }

  // ptr - ptr
  // 2つのポインタ間の要素数を返す
  if (lhs->ty->base && rhs->ty->base) {
    Node *node = new_node(ND_SUB, lhs, rhs);
    node->ty = new_type(TY_INT);
    use_func_DIV = true; // 現状シフト演算子を使わず割り算として処理するため
    return new_node(ND_DIV, node, new_node_num(2)); // シフト演算子を定義したら変更する
  }

  error_at(token->str, "無効な演算");
  return NULL;
}

// add = mul ("+" mul | "-" mul)*
static Node *add() {
  Node *node = mul();

  for (;;) {
    if (consume("+"))
      node = new_add(node, mul());
      // ポインタ演算との場合分けなどの関数
    else if (consume("-"))
      node = new_sub(node, mul());
      // ポインタ演算との場合分けなどの関数
    else
      return node;
  }
}

// mul = unary ("*" unary | "/" unary)*
static Node *mul() {
  Node *node = unary();

  for (;;) {
    if (consume("*")) {
      use_func_MUL = true;
      node = new_node(ND_MUL, node, unary());
    } else if (consume("/")) {
      use_func_DIV = true;
      node = new_node(ND_DIV, node, unary());
    } else
      return node;
  }
}

// unary = ("+" | "-")? primary
//       | "*" unary
//       | "&" unary 
static Node *unary() {
  if (consume("+"))
    return primary();
  if (consume("-"))
    return new_node(ND_SUB, new_node_num(0), primary());
  if (consume("*"))
    return new_node(ND_DEREF,unary(), NULL);
  if (consume("&"))
    return new_node(ND_ADDR, unary(), NULL);
  return primary();
}

// funcall = "(" (assign ("," assign)*)? ")"
static Node *funcall(Token *tok) {
  Node head = {};
  Node *cur = &head;

  while (!consume(")")) {
    cur = cur->next = assign();
    consume(",");
  }

  Node *node = new_node(ND_FUNCALL, NULL, NULL);
  node->funcname = strndup(tok->str, tok->len);
  node->args = head.next;
  return node;  
}

// primary = num | ident | "(" expr ")" | "sizeof" unary
static Node *primary() {
  // 次のトークンが"("なら、"(" expr ")"のはず
  if (consume("(")) {
    Node *node = expr();
    expect(")");
    return node;
  }

  if (consume_keyword("sizeof")) {
    Node *node = unary();
    add_type(node);
    return new_node_num(node->kind == ND_LVAR ? node->lvar->lvar_size : node->ty->size);
  }

  Token *tok = consume_ident();
  if (tok) {
    // 関数呼び出し
    if (consume("(")) {
      return funcall(tok);
    }

    LVar *lvar = find_lvar(tok);
    if (!lvar) 
      error_at(token->str, "未定義の変数");

    if (consume("[")) { // 添え字演算子
      Node *node = expr();
      expect("]");
      // x[y]を*(x+y)に変換する
      return new_node(ND_DEREF, new_add(new_node_lvar(lvar, tok), node), NULL);
    }
    return new_node_lvar(lvar, tok);
  }

  // そうでなければ数値のはず
  Node *node_num = new_node_num(expect_number());
  if (consume("[")) { // 添え字演算子
    Node *node = expr();
    expect("]");
    // x[y]を*(x+y)に変換する
    return new_node(ND_DEREF, new_add(node_num, node), NULL);
  }
  return node_num;
}
