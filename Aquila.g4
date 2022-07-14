grammar Aquila;

program
    : statement* EOF
    ;

statement
    : ifStatement
    | loopStatement
    | forStatement
    | foreachStatement
    | readStatement
    | writeStatement
    | assignStatement
    | callStatement
    | removeStatement
    ;

/* expression has to evaluate to true, everything else is false */
ifStatement
    : 'if' expression block
     ('elif' expression block)*
     ('else' elseBlock=block)?
    ;

/* expression has to evaluate to true, everything else is false */
loopStatement
    : 'while' expression bottom=block
    | 'loop' top=block 'while' expression (';' | bottom=block)
    ;

forStatement
    : 'for' Identifier 'from' from=expression 'to' to=expression ('step' step=expression)? block
    ;

/* iterates over all members of the result of the expression */
foreachStatement
    : 'foreach' ('key' key=Identifier)? ('value' value=Identifier)? 'in' expression block
    ;

/* reads a line from stdin */
readStatement
    : 'read' lhs ';'
    ;

/* writes whatever is the result of expression to stdout (with trailing newline) */
writeStatement
    : 'write' rhs=expression ';'
    ;

assignStatement
    : lhs ':=' rhs=expression ';'
    ;

callStatement
    : 'call' functionCall ';'
    ;

removeStatement
    : 'remove' lhs ';'
    ;

block
    : '(' statement* ')'
    ;

lhs
    : Identifier lhsPart*
    ;

lhsPart
    : '.' Identifier
    | '[' expression ']'
    ;

expression
    : logicalOperation
    ;

logicalOperation
    : unaryLogicalOperation (logicalOperator unaryLogicalOperation)*
    ;

logicalOperator
    : 'and'
    | 'or'
    | 'xor'
    ;

unaryLogicalOperation
    : unaryLogicalOperator? relation
    ;

unaryLogicalOperator
    : 'not'
    ;

relation
    : addition (relationalOperator addition)?
    ;

relationalOperator
    : ':' /* dictionary in (contains) */
    | '<='
    | '<>'
    | '<'
    | '='
    | '>='
    | '>'
    | 'eq' /* string equals */
    | 'ew' /* string ends with */
    | 'in' /* string in (contains) */
    | 'ne' /* string not equals */
    | 'sw' /* string starts with */
    ;

addition
    : multiplication (additionalOperator multiplication)*
    ;

additionalOperator
    : '&' /* string concat */
    | '+'
    | '-'
    ;

multiplication
    : unaryAddition (multiplicationOperator unaryAddition)*
    ;

multiplicationOperator
    : '*'
    | '/'
    | 'mod'
    | 'rem'
    ;

unaryAddition
    : unaryAdditionalOperator? factor
    ;

unaryAdditionalOperator
    : '+'
    | '-'
    ;

factor
    : bracketExpression
    | absExpression
    | dictAggregate
    | literal
    | functionCall
    | accessExpression
    ;

bracketExpression
    : '(' expression ')'
    ;

absExpression
    : '|' expression '|'
    ;

dictAggregate
    : '{' dictAggregatePair* '}'
    ;

dictAggregatePair
    : (key=dictAggregateKey ':')? value=expression ';'
    ;

dictAggregateKey
    : Identifier
    | '[' expression ']'
    ;

literal
    : String
    | Integer
    | False
    | True
    ;

functionCall
    : Identifier '(' (arguments+=expression (',' arguments+=expression)*)? ')'
    ;

accessExpression
    : Identifier accessExpressionPart*
    ;

accessExpressionPart
    : '.' Identifier
    | '[' expression ']'
    ;

Whitespace
    : [\n\r ]+ -> skip
    ;

Comment
    : '#' ~[\n\r]* -> skip
    ;

String
    : '\'' ~[\n\r']* '\''
    ;

Integer
    : '0b' [0-1]+
    | '0o' [0-7]+
    | '0x' [0-9a-f]+
    | [0-9]+
    ;

False
    : 'false'
    ;

True
    : 'true'
    ;

Identifier
    : [A-Za-z] [0-9A-Za-z]*
    ;
