# GRAMMAR.md

## Overview

This document defines the grammar of via, a powerful, semi-dynamically typed, multi paradigm programming language. The grammar is specified in a format similar to Backus-Naur Form (BNF) for clarity.

## Basic Syntax

### Comments

- **Single-line comment**: Starts with `##`
  ```plaintext
  ## This is a single-line comment
  ```

- **Multi-line comment**: Enclosed in `#/`
  ```plaintext
  #/
  This is a 
  multi-line comment
  #/
  ```

### Identifiers

Identifiers must start with a letter (a-z, A-Z) or an underscore (`_`), followed by letters, digits (0-9), or underscores. Identifiers are case-sensitive.

```plaintext
identifier ::= [a-zA-Z_][a-zA-Z0-9_]*
```

### Literals

- **Number literals**: Integers and floating-point numbers
  ```plaintext
  number_literal ::= [0-9]+ | [0-9]+"."[0-9]+
  ```

- **String literals**: Enclosed in single or double quotes
  ```plaintext
  string_literal ::= '"' [^"]* '"' | "'" [^']* "'"
  ```

- **Boolean literals**: Abstract boolean identifiers
  ```plaintext
  bool_literal ::= "true" | "false"
  ```

## Expressions

- **Binary Expressions**: Binary expressions consist of two operands and an operator.

```plaintext
expression ::= primary_expression (binary_operator primary_expression)*
binary_operator ::= '+' | '-' | '*' | '/' | '==' | '~=' | '<' | '>' | '<=' | '>='
```

- **Grouping Expressions**: Grouping is done using parentheses.

```plaintext
grouped_expression ::= '(' expression ')'
```

- **Lambda Expressions**: Lambda expressions allow defining anonymous functions.

```plaintext
lambda_expression ::= 'func' '(' [identifier (',' identifier)*] ')' '->' expression
```

- **Primary Expressions**: Primary expressions include literals, identifiers, and grouped expressions.

```plaintext
primary_expression ::= number_literal | string_literal | identifier | grouped_expression
```

## Operators

- **Unary operator**: Negates the value of the primitive expression or calls the `__unm` method of an object
  ```plaintext
  unary_expr ::= - expression | literal | identifier
  ```

- **Increment operator**: Increments the value of a variable or calls the `__inc` method of an object
  ```plaintext
  inc_expr ::= identifier "++"
  ```

- **Decrement operator**: Decrements the value of a variable or calls the `__dec` method of an object
  ```plaintext
  dec_expr ::= identifier "--"
  ```
- **Arithmetic assignment operators**: Performs arithmetic operations on the value or calls the respective method of the object
  ```plaintext
  arithmetic_assign_expr ::= identifier "+=" | "-=" | "*=" | "/=" identifer | expression | literal
  ```

## Control Flow

### If Statements

If statements are used for conditional execution.

```plaintext
if_statement ::= 'if' '(' expression ')' 'then' statement ('else' statement)?
```

### Switch Statements

Switch statements allow multi-way branching based on a value.

```plaintext
switch_statement ::= 'switch' '(' expression ')' 'case' case_block+
case_block ::= 'case' expression ':' statement ('break')?
```

## Functions and Modules

### Function Definitions

Functions are defined using the `func` keyword.

```plaintext
function_definition ::= 'func' identifier '(' [identifier (',' identifier)*] ')' '->' statement
```

### Importing Modules

Modules are imported similarly to Lua's `require`.

```plaintext
import_statement ::= 'require' '(' string_literal ')'
```

### Exporting Values

To export a value from a module, return it at the end of the file.

```plaintext
return_statement ::= 'return' expression
```

## Loops

### For Loops

For loops iterate over a range or collection.

```plaintext
for_loop ::= 'for' identifier 'in' expression '{' statement '}'
```

### While Loops

While loops execute as long as the condition is true.

```plaintext
while_loop ::= 'while' '(' expression ')' '{' statement '}'
```

## Structs and Namespaces

### Struct Definitions

Structs are defined to group related data.

```plaintext
struct_definition ::= 'struct' identifier '{' struct_member* '}'
struct_member ::= identifier ':' type_expression
```

### Namespace Definitions

Namespaces allow organizing related functions and variables.

```plaintext
namespace_definition ::= 'namespace' identifier '{' namespace_member* '}'
namespace_member ::= identifier ':' type_expression | function_definition
```

## Types

### Unified Number Types

All numeric values are treated as a single type, providing seamless interaction.

```plaintext
type_expression ::= 'number' 
```

Technically, a number is just a 64-bit float value.

## Conclusion

This grammar defines the core syntax of via. As the language evolves, this document will be updated to reflect new features and changes. For further information and examples, refer to the main documentation.
