#!/bin/bash
try() {
	expected="$1"
	input="$2"

	./9cc "$input" > tmp.s
	gcc -o tmp tmp.s
	./tmp
	actual="$?"

	if [ "$actual" = "$expected" ]
	then
		echo "$input => $actual"
	else
		echo "$input => $expected expected, but got $actual."
		exit 1
	fi
}

try 42 42
try 24 '5 + 20 - 4 + 3'
try 88 '5 + 20 * 4 + 3'

echo OK
