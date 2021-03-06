#!/bin/bash
assert() {
  expected="$1"
  input="$2"

  ./Temple_cc "$input" > tmp.s
  ./Temple_simulator3.exe './tmp.s' '-oExe' pppp.dat
  #./tmp
  actual="$?"

  if [ "$actual" = "$expected" ]; then
    echo "$input => $actual"
  else
    echo "$input => $expected expected, but got $actual"
    exit 1
  fi
}

assert 0 'int main() { return 0; }'
assert 42 'int main() { return 42; }'
assert 21 'int main() { return 5+20-4; }'
assert 41 'int main() { return  12 + 34 - 5 ; }'
assert 47 'int main() { return 5+6*7; }'
assert 15 'int main() { return 5*(9-6); }'
assert 4 'int main() { return (3+5)/2; }'
assert 10 'int main() { return -10+20; }'

assert 0 'int main() { return 0==1; }'
assert 1 'int main() { return 42==42; }'
assert 1 'int main() { return 0!=1; }'
assert 0 'int main() { return 42!=42; }'

assert 1 'int main() { return 0<1; }'
assert 0 'int main() { return 1<1; }'
assert 0 'int main() { return 2<1; }'
assert 1 'int main() { return 0<=1; }'
assert 1 'int main() { return 1<=1; }'
assert 0 'int main() { return 2<=1; }'

assert 1 'int main() { return 1>0; }'
assert 0 'int main() { return 1>1; }'
assert 0 'int main() { return 1>2; }'
assert 1 'int main() { return 1>=0; }'
assert 1 'int main() { return 1>=1; }'
assert 0 'int main() { return 1>=2; }'

assert 3 'int main() { int a; a=3; return a; }'
assert 3 'int main() { int a=3; return a; }'
assert 8 'int main() { int a=3; int z=5; return a+z; }'

assert 1 'int main() { return 1; 2; 3; }'
assert 2 'int main() { 1; return 2; 3; }'
assert 3 'int main() { 1; 2; return 3; }'

assert 3 'int main() { int a=3; return a; }'
assert 8 'int main() { int a=3; int z=5; return a+z; }'
assert 6 'int main() { int a; int b; a=b=3; return a+b; }'
assert 3 'int main() { int foo=3; return foo; }'
assert 8 'int main() { int foo123=3; int bar=5; return foo123+bar; }'

assert 3 'int main( ){ {1; {2;} return 3;} }'
assert 5 'int main( ){ ;;; return 5; }'

assert 3 'int main(){int a = 1; if(a) return 3; return 4;}'
assert 3 'int main(){ if (0) return 2; return 3; }'
assert 3 'int main(){ if (1-1) return 2; return 3; }'
assert 2 'int main(){ if (1) return 2; return 3; }'
assert 2 'int main(){ if (2-1) return 2; return 3; }'
assert 4 'int main(){ if (0) { 1; 2; return 3; } else { return 4; } }'
assert 3 'int main(){ if (1) { 1; 2; return 3; } else { return 4; } }'

assert 55 'int main() { int i=0; int j=0; for (i=0; i<=10; i=i+1) j=i+j; return j; }'
assert 3 'int main() { for (;;) return 3; return 5; }'

assert 10 'int main() { int i=0; while(i<10) i=i+1; return i; }'

assert 3 'int main() { int x=3; return *&x; }'
assert 3 'int main() { int x=3; int *y=&x; int **z=&y; return **z; }'
assert 5 'int main() { int x=3; int y=5; return *(&x+1); }'
assert 3 'int main() { int x=3; int y=5; return *(&y-1); }'
assert 5 'int main() { int x=3; int y=5; return *(&x-(-1)); }'
assert 5 'int main() { int x=3; int *y=&x; *y=5; return x; }'
assert 7 'int main() { int x=3; int y=5; *(&x+1)=7; return y; }'
assert 7 'int main() { int x=3; int y=5; *(&y-2+1)=7; return x; }'
assert 5 'int main() { int x=3; return (&x+2)-&x+3; }'
assert 8 'int main() { int x, y; x=3; y=5; return x+y; }'
assert 8 'int main() { int x=3, y=5; return x+y; }'

assert 3 'int main(){ return ret3(); }int ret3() { return 3; }'
assert 5 'int main(){ return ret5(); } int ret5() { return 5; }'
assert 6 'int main(){ return ret5()+1; } int ret5() { return ret3()+2; }int ret3() { return 3; }'
assert 8 'int main() { return add(3, 5); }int add(int x,int y) { return x+y; }'
assert 2 'int main() { return sub(5, 3); }int sub(int x,int y) { return x-y; }'
assert 21 ' int main() { return add6(1,2,3,4,5,6); }int add6(int a,int b,int c,int d,int e,int f){return a+b+c+d+e+f;}'
assert 66 ' int main() { return add6(1,2,add6(3,4,5,6,7,8),9,10,11); }int add6(int a,int b,int c,int d,int e,int f){return a+b+c+d+e+f;}'
assert 136 'int main() { return add6(1,2,add6(3,add6(4,5,6,7,8,9),10,11,12,13),14,15,16); }int add6(int a,int b,int c,int d,int e,int f){return a+b+c+d+e+f;}'

assert 32 'int main() { return ret32(); } int ret32() { return 32; }'
assert 7 'int main() { return add2(3,4); } int add2(int x, int y) { return x+y; }'
assert 1 'int main() { return sub2(4,3); } int sub2(int x, int y) { return x-y; }'
assert 55 'int main() { return fib(9); } int fib(int x) { if (x<=1) return 1; return fib(x-1) + fib(x-2); }'
assert 55 'int sum(int n){int ret=0;for(;1<=n;n=n-1)ret=ret+n; return ret;} int main() {return sum(10);}'

assert 2 'int main() { int x; return sizeof(x); }'
assert 2 'int main() { int x; return sizeof x; }'
assert 2 'int main() { int *x; return sizeof(x); }'
assert 2 'int main() { int x=1; return sizeof(x=2); }'
assert 1 'int main() { int x=1; sizeof(x=2); return x; }'

assert 3 'int main() { int a[2];*a=1;*(a + 1) = 2;int *p;p = a;return *p + *(p + 1); }'
assert 100 'int main() { int b=100;int a[1];int c=50; *a=1;return a[-1]; }'
assert 4 'int main() {int a[4];int p; return &p-a;}'
assert 6 'int main() { int a[3]; *a=1; 1[a]=2; a[2]= 3;int sum = 0;int i=0;for (i=0; i<3; i=i+1)sum=sum+i[a];return sum; }'

echo OK
