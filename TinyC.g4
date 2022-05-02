grammar TinyC;

translationUnit
    :   function+ EOF
    ;

function
    :   type functionName L_PARAN parameters? R_PARAN statements
    ;

type
    :   typeName
    ;

parameters
    :   type variableName ( COMMA type variableName )*
    ;

statements
    :   L_BRACE statement* R_BRACE
    ;

statement
    :   returnStatement SEMICOLON
    |   variableDecl SEMICOLON
    |   assignment SEMICOLON
    |   expression SEMICOLON
    |   ifStatement
    |   statements
    ;

ifStatement
    :   IF L_PARAN expression R_PARAN statement ( ELSE statement )?
    ;

variableDecl
    :   type variableName ( EQUAL expression )?
    ;

assignment
    :   variableName EQUAL expression
    ;

returnStatement
    :   RETURN expression
    ;

expression
    :   compExpression
    ;

compExpression
    :   addSubExpr
        (
            (   EQUALEQUAL
            |   NOTEQUAL
            |   LESS
            |   LESSEQUAL
            |   GREATER
            |   GREATEREQUAL
            )
            addSubExpr
        )?
    ;

addSubExpr
    :   mulDivExpr ( ( PLUS | MINUS ) mulDivExpr )*
    ;

mulDivExpr
    :   operand ( ( ASTERISK | SLASH ) operand )*
    ;

operand
    :   valueExpr
    |   L_PARAN expression R_PARAN
    ;

valueExpr
    :   literal
    |   variableName
    |   callExpr
    ;

callExpr
    :   functionName L_PARAN arguments? R_PARAN
    ;

arguments
    :   argument ( COMMA argument )*
    ;

argument
    :   expression
    ;


literal
    :   INT_LITERAL
    ;

variableName
    :   IDENTIFIER
    ;

typeName
    :   IDENTIFIER
    ;

functionName
    :   IDENTIFIER
    ;


RETURN : 'return';
IF     : 'if';
ELSE   : 'else';


IDENTIFIER
    :   NONDIGID ( NONDIGID | DIGID )*
    ;

INT_LITERAL
    :   DIGID+
    ;

fragment 
NONDIGID
    :   [a-zA-Z_]
    ;

fragment
DIGID
    :   [0-9]
    ;


COMMA     : ',';
SEMICOLON : ';';
L_BRACE   : '{';
R_BRACE   : '}';
L_PARAN   : '(';
R_PARAN   : ')';
ASTERISK  : '*';
EQUAL     : '=';
PLUS      : '+';
MINUS     : '-';
SLASH     : '/';

EQUALEQUAL   : '==';
NOTEQUAL     : '!=';
LESS         : '<';
LESSEQUAL    : '<=';
GREATER      : '>';
GREATEREQUAL : '>=';


WHITESPACE
    :   [ \t]+ 
    -> skip
    ;

NEWLINE
    :   ( '\r' '\n'? | '\n' )
    -> skip
    ;
