%{
    #include "configParser.tab.h"
    #include <stdlib.h>
    extern int tokenValue;
%}

%s number

%%

ssid {BEGIN(0); return SSID;}
password {BEGIN(0); return PASS;}
mqtt-url {BEGIN(0); return URL;}
mqtt-port {BEGIN(number); return PORT;}

& {return AND;}
= {return EQ;}
<number>[0-9]+ {tokenValue = atoi(yytext); return NUM;}
[0-9A-Za-z\._~-] {tokenValue = *yytext; return CHAR;}
%[0-7][0-9A-Fa-f] {
    char* c; 
    long int temp = strtol(yytext+1, &c, 16); 
    tokenValue = (int)temp;
    return CHAR;
}
