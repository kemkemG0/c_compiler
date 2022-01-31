#!/bin/bash
assert() {
  expected="$1"
  input="$2"

  ./main "$input" > tmp.s
  cc -o tmp tmp.s
  ./tmp
  actual="$?"

  if [ "$actual" = "$expected" ]; then
    echo "$input => $actual"
  else
    echo "$input => $expected expected, but got $actual"
    exit 1
  fi
}

assert 0 0
assert 42 42
assert 21 "5+20-4"
assert 41 "  12+  34   -5  "
assert 47 '5+6*  7'
assert 15 '5*(9-6)'
assert 4 '((3+5))/2'
assert 9 '(  (5*9)*9)/45+1-1'
assert 5 '-95 + 100'
assert 195 '+95 +( +100)'
assert 1 '-((5*9)*9)/45+1-1 + 10'
assert 10 '- - +10'
assert 10 '--+10'
assert 0 '0==1'
assert 1 '42==42'
assert 1 '0!=1'
assert 0 '42!=42'
assert 1 '0<1'
assert 0 '1<1'
assert 0 '2<1'
assert 1 '0<=1'
assert 1 '1<=1'
assert 0 '2<=1'
assert 1 '1>0'
assert 0 '1>1'
assert 0 '1>2'
assert 1 '1>=0'
assert 1 '1>=1'
assert 0 '1>=2'
assert 1  '-((5*9)*9)/45+1-1 + 10 == 1'

echo OK
