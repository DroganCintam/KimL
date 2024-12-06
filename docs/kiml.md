# KimL

KimL is an esoteric programming language
implemented in C++ for learning compiler
techniques. It is a simple language with basic
features, straightforward syntax, and next to zero
practical use.

## Features

There are 5 data types in KimL:

- `bool` for Boolean values
- `int` for 32-bit integers
- `real` for 32-bit floating-point numbers
- `string` for text
- `object` for native bindings

`int` and `real` are interchangeable with some
precision loss in certain cases. They can also be
converted implicitly to `string`. `bool` can be
converted to `string` as well and any type can be
converted to `bool`.

There is a native stack for storing values of any
type. We call it the *k-stack*.

Besides, there is an array of 128 freely typed
values called the *tape*.

There are loop and conditional structures, but not
in the way you might expect from a normal
language.

## Syntax

### Overview

KimL has a simple syntax. Every statement is in the
form described below:

```
<group>.<command> <arg1> <arg2> ...
```

The following are command groups in KimL:

- `io` for input/output
- `var` for variable manipulation
- `stack` for stack manipulation
- `tape` for tape manipulation
- `ctrl` for control structures
- `func` for function management
- `event` for event handling

### Constant declaration

```
const name = value
```

### Enum declaration

```
enum value1 value2 ...
```

### Comments

Comments start with the `;` character and end at the
end of the line.

## `io` group

### `io.in`

```
io.in int
io.in real
io.in string
```

Reads a value from the standard input, converts it
to the specified type and pushes it to the
k-stack. If the conversion fails, pushes the default
value of that type to the k-stack.

```
io.in variable_name
```

Reads a value from the standard input, converts it
to the type of the variable, and assigns it to the
variable. If the conversion fails, assigns default
value to the variable.

### `io.out`

```
io.out expression
```

Evaluates the expression and prints the result to
the standard output.

## `var` group

Variables in KimL are globally declared and can be
used anywhere in the program, even before the
point of declaration, as long as they are declared
before the program branches to the location where
they are accessed.

### `var.decl`

```
var.decl bool b
var.decl int i
var.decl real r
var.decl string s
```

Declares a variable of the specified type.

```
var.decl bool b = true
var.decl int i = 42
var.decl real r = 3.14
var.decl string s = "Hello, world! Pi is " + r
```

Declares a variable of the specified type and
initializes it with the value of the expression
after the `=` sign.

### `var.set`

```
var.set greeting = "Hello, " + userName
```

Evaluates the expression and assigns the result to
the variable.

### `var.del`

```
var.del variable_name
```

Deletes the variable.

## `stack` group

The *k-stack* is a last-in-first-out data
structure that can store values of any type. It is
used for temporary storage of values and has
theoretically infinite capacity.

### `stack.push`

```
stack.push n + r / 2
```

Evaluates the given expression and pushes the
result to the k-stack.

### `stack.pop`

```
stack.pop
```

Pops the top value from the k-stack. If the stack
is empty, an error occurs.

```
stack.pop variable_name
```

Pops the top value from the k-stack and assigns it
to the variable. If the stack is empty, an error
occurs.

### `stack.peek`

```
stack.peek variable_name
```

Copies the top value from the k-stack to the
variable. If the stack is empty, an error occurs.

### `stack.swap`

```
stack.swap
```

Swaps the top two values on the k-stack. An error
occurs if the stack has less than two values.

### `stack.clear`

```
stack.clear
```

Clears the whole k-stack.

```
stack.clear 5
```

Clears the top N values from the k-stack. The
argument must be an integer constant.

### `stack.read`

```
stack.read foo at 3
```

Reads the value at the specified offset from the
top of the k-stack and assigns it to the variable.
The offset must be an integer constant starting
from 1. If the offset is out of bounds, an error
occurs.

### `stack.write`

```
stack.write "Hello, world!" at 3
```

Writes the value to the specified offset from the
top of the k-stack. The offset must be an integer
constant starting from 1. If the offset is out of
bounds, an error occurs.

## `tape` group

The *tape* is a 128-element array of freely typed
values. It is accompanied by a pointer that points
to the current element.

### `tape.read`

```
tape.read foo
tape.read int
```

Copies the value at the current position of the
tape to the variable or to the *k-stack*.

```
tape.read foo at 42
tape.read int at 42
```

Copies the value at the specified position of the
tape to the variable or to the *k-stack*. The
position must be an integer constant between 0 and
127.

### `tape.write`

```
tape.write firstName + " " + lastName
```

Evaluates the expression and writes the result to
the current position of the tape.

```
tape.write "Hello, world!" at 42
```

Writes the value to the specified position of the
tape. The position must be an integer constant
between 0 and 127.

### `tape.move`

```
tape.move i * 10 + j
```

Moves the tape pointer to the specified position.
If the expression fails to evaluate to an integer,
an error occurs.

### `tape.next`

```
tape.next
```

Moves the tape pointer to the next position. If
the pointer is at the end of the tape, it wraps
around to the beginning.

### `tape.prev`

```
tape.prev
```

Moves the tape pointer to the previous position.
If the pointer is at the beginning of the tape, it
wraps around to the end.

## `ctrl` group

The `ctrl` group contains commands for controlling
the flow of the program.

### `ctrl.goto`

```
ctrl.goto label
; ...
label:
; ...
```

Jumps to the specified label. If the label is not
found, an error occurs.

```
ctrl.goto label if a > 10
```

Jumps to the specified label if the condition is
true. If the label is not found, an error occurs.

### `ctrl.call`

```
ctrl.call procedure_name
ctrl.call procedure_name if a > 10
```

Works like `ctrl.goto` but also saves the current
program counter to the call stack so that it can
return later.

### `ctrl.return`

```
ctrl.return
```

Returns from the current procedure to the point
where it was called.

### `ctrl.end`

```
ctrl.end
```

Terminates the program execution.

## `func` group

Functions in KimL are as basic as procedures in
assembly language. They are located at specific
labels, accept arguments from the k-stack, and
conventionally return values to the tape.

### `func.decl`

```
func.decl int factorial(int n) = label_start
; ...
label_start:
; ...
```

Declares a function with the specified signature
and where its implementation starts.

The parameter name is optional.

### `func.call`

```
func.call factorial(5)
```

Calls a function with the specified arguments.

By convention, the function should be responsible
for clearing the arguments from the k-stack and
storing the result to the tape at position 0.

The function returns using `ctrl.return`.

## `event` group

### `event.handle`

```
event.handle ev1 by label1
; ...
label1:
; ...
```

Registers an event handler for the specified event
at the specified label. If the label has been
registered before, it is overwritten.

### `event.unhandle`

```
event.unhandle ev1 by label1
```

Unregisters the event handler for the specified
event at the specified label.

Known issue: you should not unhandle an event from
within a handler because that will mess up the
handler list.

### `event.raise`

```
event.raise ev1
```

Raises the specified event. If there is no handler
for the event, nothing happens.

The handler code should return using `ctrl.return`.

## Constants

Constants are declared using the `const` keyword.

```
const pi = 3.14159
const greeting = "Hello, world!"
```

## Enums

Enums are declared using the `enum` keyword.

```
enum red green blue
; which is equivalent to
const red = 0
const green = 1
const blue = 2
```

## Expressions

### Literals

Valid `bool` literals are `true` and `false`.

Valid `int` literals are 0, 10, 300, 1000000, etc.

Valid `real` literals are 0.0, 1.23, 0.245, etc.

`string` literals are enclosed by double-quotes.
Allowed escape sequences are `\n`, `\r`, `\t`,
`\\`, `\"`, `\x##` and `\u####`.

Valid `object` literals are only `null`. This type
is used for native bindings.

### Operators

The following list are allowed operators in KimL, in descending order of priority.

- `#()` converts to real, `@()` converts to int, `$()` converts to object
- `^` exponentiation
- unary `+` and `-`, `not` (logic)
- `*` multiplication, `/` division, `\` integral division
- `+` addition, `-` subtraction
- `&` string concatenation
- `<` less than, `<=` less than or equal, `>` greater than, `>=` greater than or equal
- `=` equal, `<>` not equal
- `xor` (logic)
- `and` (logic)
- `or` (logic)

### Built-in functions

KimL provides a set of built-in functions as listed below:

- `abs(numeric)`: returns the absolute value of a number, the return type depends on the argument type.
- `real sqrt(numeric)`: returns the square root of a number.
- `real sin(numeric)`: returns the sine of an angle (in radians).
- `real cos(numeric)`: returns the cosine of an angle.
- `real tan(numeric)`: returns the tangent of an angle.
- `real asin(numeric)`: returns the arc-sine of a number.
- `real acos(numeric)`: returns the arc-cosine of a number.
- `real atan(numeric)`: returns the arc-tangent of a number.
- `string chr(numeric)`: returns a character from the given number(mod 256).
- `int asc(string s)`: returns the ASCII value of the first character in the given string or 0 if the string is empty.
- `int len(string s)`: returns the length of the given string.
- `string left(string s, int n)`: returns the leading n characters of the given string, or the string itself if n is bigger than its length.
- `string mid(string s, int i, int n)`: returns n characters of the given string, starting from i; or the string itself if i or i+n is bigger than its length.
- `string right(string s, int n)`: returns the trailing n characters of the given string, or the string itself if n is bigger than its length.
- `iif(cond, t, f)`: returns t if cond is true, otherwise f; the return type depends on the type of t or f.
- `_tape(int idx)`: returns a value on the tape, pointed to by idx.
- `_stack(int offset)`: returns a value on the stack, at the given offset from the top of the stack. _stack(1) returns the top value, _stack(2) returns the next to top value, and so on.
- `_pop()`: pops and returns the top value on the stack.
- `_peek()`: returns the top value on the stack.
- `real min(real a, real b)`: takes 2 values and returns the lesser.
- `real max(real a, real b)`: takes 2 values and returns the greater.
- `real random()`: returns a random number in the range 0 (inclusive) through 1 (exclusive).
- `int print(string s)`: prints the given value to stdout.
- `string readln()`: reads a string from stdin.
