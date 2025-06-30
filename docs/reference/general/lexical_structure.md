# Lexical structure

via does not support extended character sets (as of version 0.1) such as UTF-8, only ASCII. It also does not rely on token context.

## Tokens

### Keywords

The via compiler reserves identifiers that abide by the following form:
```
KEYWORD ::= "var" |
            "func" |
            "type" |
            "return" |
            "if" |
            "for" |
            "while" |
            "break" |
            "continue" |
            "macro" |
            "namespace"
```

### Identifiers

via classifies identifiers in two ways: regular identifiers and macro identifiers. This unusual classification is due to the principle of not relying on token contexts. These are defined in the following form:
```
IDENTIFIER ::= [A-Za-z] [A-Za-z0-9_]*
MACRO_IDENTIFIER ::= IDENTIFIER "!"
```

### Literals

The via compiler defines multiple kinds of literal tokens, these include strings, numbers, booleans, and `nil`. Tokens that abide by the following form are classified as literals:
```
LITERAL ::= NUMBER | STRING | "true" | "false" | "nil"
```

#### Number literals

Number literals are classified in four forms: floating point, decimal, hexadecimal, and binary. You can also optionally use `_`s in order to improve readability of numbers for humans, while not having any lexical meaning.

Decimal number literals are defined in the following form:
```
DECIMAL_DIGIT ::= [0-9]
DECIMAL_NUMBER ::= DECIMAL_DIGIT (("_" DECIMAL_DIGIT)*)*
```

Hexadecimal number literals are defined in the following form:
```
HEX_DIGIT ::= [0-9a-fA-F]
HEX_NUMBER ::= "0x" HEX_DIGIT (("_" HEX_DIGIT)*)*
```

Binary number literals are defined in the following form:
```
BIN_DIGIT ::= [0-1]
BIN_NUMBER ::= "0b" BIN_DIGIT (("_" BIN_DIGIT)*)*
```

Floating point numbers are defined in the following form:
```
FP_NUMBER ::= DECIMAL_NUMBER ["." DECIMAL_NUMBER]
```

And finally, numbers are defined in the following form:
```
NUMBER ::= DECIMAL_NUMBER | HEX_NUMBER | BIN_NUMBER | FP_NUMBER
```

#### String literals

String literals in via are defined in the following form:
```
QUOTE ::= """ | "'" | "`"
STRING ::= QUOTE (ESCAPED_CHAR | NORMAL_CHAR)* QUOTE
ESCAPED_CHAR ::= "\" ANY_CHAR
NORMAL_CHAR ::= any character except """ or "\n" or "\r" or "\"
ANY_CHAR ::= any single character
```

### Operators and symbols

{TBA}
