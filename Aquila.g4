grammar Aquila;

program
    : statement* EOF
    ;

statement
    : ifStatement
    | loopStatement
    | forStatement
    | readStatement
    | writeStatement
    | assignStatement
    | callStatement
    | removeStatement
    | runStatement
    ;

ifStatement
    :   'if' condition+=expression then+=block
     ('elif' condition+=expression then+=block)*
     ('else' elseBlock=block)?
    ;

switchStatement
    : 'switch' switchHeadExpression=expression ':'
     ('case' labels+=switchStatementLabels then+=block)*
     ('default' defaultBlock=block)?
    ;

switchStatementLabels
    : expression (',' expression)*
    ;

loopStatement
    : 'while' expression bottom=block
    | 'loop' top=block 'while' expression (';' | bottom=block)
    ;

forStatement
    : 'for' Identifier 'from' from=expression 'to' to=expression ('step' step=expression)? block
    ;

readStatement
    : 'read' lhs ';'
    ;

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

runStatement
    : 'run' rhs=expression ';'
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
    : ifExpression
    | letExpression
    | switchExpression
    | logicalOperation
    ;

ifExpression
    :   'if' condition+=expression ':' then+=expression ';'
     ('elif' condition+=expression ':' then+=expression ';')*
      'else' ':' elseExpression=expression ';'
    ;

letExpression
    : 'let' letBindExpression (',' letBindExpression)* ':' expression ';'
    ;

letBindExpression
    : lhs '=' rhs=expression
    ;

switchExpression
    : 'switch' switchHeadExpression=expression ':'
     ('case' labels+=switchExpressionLabels ':' then+=expression ';')*
      'default' ':' defaultExpression=expression ';'
    ;

switchExpressionLabels
    : expression (',' expression)*
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
    : unaryAdditionalOperator? factorial
    ;

unaryAdditionalOperator
    : '+'
    | '-'
    ;

factorial
    : factor factorialOperator?
    ;

factorialOperator
    : '!'
    ;

factor
    : bracketExpression
    | absExpression
    | dictAggregate
    | lambdaExpression
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

lambdaExpression
    : '\\' (lambdaExpressionParameter (',' lambdaExpressionParameter)*)? '->' (expression | block)
    ;

lambdaExpressionParameter
    : Identifier (':' Type)?
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

Type
    : 'Any'
    | 'Function'
    | 'Dictionary'
    | 'String'
    | 'Integer'
    | 'Boolean'
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
