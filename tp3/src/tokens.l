%{
#include "parse.tab.h"
#include <stdint.h>

extern void yyerror (const char*);
extern void yyerrorc (const char);

%}

COMMAND [WwRr]
ALPHA   [a-zA-Z]
DIGIT   [0-9]
INTEGER (0|([1-9]{DIGIT}*))
CHAR    [ -~]

%option noyywrap
%option noinput
%option nounput

%%
[ \t\n\r]+	/* ignore whitespaces */
";"		return ';';

{COMMAND} {
  switch (yytext[0])
    {
    case 'W': case 'w': return tWRITE;
    case 'R': case 'r': return tREAD;
    }
}

{INTEGER} {
  int i32 = atoi(yytext);
  uint16_t i16 = (uint16_t) i32;
  if (i16 != i32)
    {
      yyerror ("uint16 expected"); 
      return tERROR;
    }
  yylval.ivalue = i16;
  return tADDRESS;
}

'{CHAR}[^']+ {
  yyerror ("too many charaters in character litteral");
  return tERROR;
}

'{CHAR}' {
  if (yytext[1] <= 31 || yytext[1] >= 127)
    {
      yyerror ("non-printable character");
      return tERROR;
    }
  yylval.cvalue = yytext[1];
  return tCHAR;
}

. {
  if (yytext[0] <= 31 || yytext[0] >= 127)
    {
      yyerrorc (yytext[0]);
    }
  else
    {
      char str[] = "unexpected character 'c'";
      str[22] = yytext[0];
      yyerror (str);
    }
  return tERROR;
}

%%
