grammar Aquila;

program
    : statement* EOF
    ;

statement
    /* compound statements */
    : ifStatement
    | switchStatement
    | loopStatement
    | forStatement
    /* simple statements */
    | callStatement
    | exitStatement
    | nowStatement
    | randomStatement
    | readStatement
    | removeStatement
    | runStatement
    | sleepStatement
    | writeStatement
    /* assignment */
    | assignStatement
    ;

ifStatement
    :     'If' condition+=expression ':' then+=block
     ('ElseIf' condition+=expression ':' then+=block)*
     ('Else' ':' elseBlock=block)?
       'EndIf'
    ;

switchStatement
    : 'Switch' switchHeadExpression=expression ':'
     ('Case' labels+=switchStatementLabels ':' then+=block)*
     ('Default' ':' defaultBlock=block)?
      'EndSwitch'
    ;

switchStatementLabels
    : expression (',' expression)*
    ;

loopStatement
    : 'While' expression ':' bottom=block 'EndWhile'
    | 'Loop' ':' top=block 'While' expression (':' bottom=block)? 'EndLoop'
    ;

forStatement
    : 'For' Identifier 'From' from=expression 'To' to=expression ('Step' step=expression)? ':' block 'EndFor'
    ;

callStatement
    : 'Call' functionCall ';'
    ;

exitStatement
    : 'Exit' rhs=expression ';'
    ;

nowStatement
    : 'Now' lhs ';'
    ;

randomStatement
    : 'Random' lhs 'From' from=expression 'To' to=expression ';'
    ;

readStatement
    : 'Read' lhs ';'
    ;

removeStatement
    : 'Remove' lhs ';'
    ;

runStatement
    : 'Run' rhs=expression ';'
    ;

sleepStatement
    : 'Sleep' rhs=expression ';'
    ;

writeStatement
    : 'Write' rhs=expression ';'
    ;

assignStatement
    : lhs ':=' rhs=expression ';'
    ;

block
    : statement*
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

ifExpression
    :     'If' condition+=expression ':' then+=expression
     ('ElseIf' condition+=expression ':' then+=expression)*
      'Else' ':' elseExpression=logicalOperation
       'EndIf'
    ;

letExpression
    :    'Let' letBindExpression (',' letBindExpression)* ':' body=logicalOperation
      'EndLet'
    ;

letBindExpression
    : lhs '=' rhs=expression
    ;

switchExpression
    : 'Switch' switchHeadExpression=expression ':'
     ('Case' labels+=switchExpressionLabels ':' then+=expression)*
      'Default' ':' defaultExpression=logicalOperation
      'EndSwitch'
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
    : ifExpression
    | letExpression
    | switchExpression
    | bracketExpression
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
    : '{' (dictAggregatePair (',' dictAggregatePair)* ','?)? '}'
    ;

dictAggregatePair
    : (key=dictAggregateKey ':')? value=expression
    ;

dictAggregateKey
    : Identifier
    | '[' expression ']'
    ;

lambdaExpression
    : '\\' (lambdaExpressionParameter (',' lambdaExpressionParameter)*)? '->' (expression | '{' block '}')
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
    | 'Boolean'
    | 'Integer'
    | 'String'
    | 'Function'
    | 'Dictionary'
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
