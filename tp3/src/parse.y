%{
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#include "vmm.h"
#include "tlb.h"
#include "pt.h"
#include "pm.h"
#include "common.h"

const char *usage =
"\n"
"Usage: %s [-p outfile] [-t outfile] [-c outfile] [-h] file\n"
"\n"
"Options\n"
"-------\n"
"\n"
"  -p --log-physical-memory outfile\n"
"                  Outputs the final state of the physical memory\n"
"                  in outfile.\n"
"\n"
"  -t --log-tlb-state outfile\n"
"                  Outputs the final tlb state in outfile.\n"
"\n"
"  -l --log-page-table outfile\n"
"                  Outputs the final page-table state in outfile.\n"
"\n"
"  -c --log-command outfile\n"
"                  Outputs the command log in outfile.\n"
"\n"
"  -h --help       Prints this message\n"
"\n"
"\n";


/* Parser specific variable.  */
int recovering = false;
extern int yylex (void);
extern char *yytext;
void yyerrorc (const char c);
void yyerror (const char *msg);

%}

%union {
  uint16_t ivalue;
  char     cvalue;
}

%token tREAD "read command"
%token tWRITE "write command"
%token tERROR
%token tEND
%token <ivalue> tADDRESS "address"
%token <cvalue> tCHAR "character litteral"

%start start

%error-verbose

%%


start: commands_list

commands_list:
  /* empty */
| commands_list command
;


command:
  ';'
| tWRITE tADDRESS tCHAR ';'
    { vmm_write ($2, $3); }
| tREAD tADDRESS ';'
    { (void) vmm_read ($2); }
| error ';'
    {
      recovering = false;
      yyerrok;
    }
;

%%
void yyerrorc (const char c) {
  if (!recovering)
    {
      printf ("Invalid command: unexpected character \\%d.\n", c);
      recovering = true;
    }
}

void yyerror (const char *msg) {
  if (!recovering)
    {
      printf ("Invalid command: %s.\n", msg);
      recovering = true;
    }
}

// ༽つ۞﹏۞༼つ

int main (int argc, char *argv[])
{
  FILE *pm_log = NULL;
  FILE *tlb_log = NULL;
  FILE *pt_log = NULL;
  FILE *vmm_log = NULL;
  FILE *backstore;

  size_t opt;
  for (opt = 1; opt < argc && argv[opt][0] == '-'; opt++)
    {
      switch (argv[opt][1])
	{ case '-':
	  if (strcmp (argv[opt], "--log-physical-memory") == 0) case 'p': {
            if (opt == argc - 2)
	      error ("Option `%s` expected file name\n", argv[opt]);
            pm_log = fopen (argv[++opt], "w+");
            if (!pm_log)
	      error ("Could not open file `%s`\n", argv[opt]);

        } else if (strcmp (argv[opt], "--log-tlb") == 0) case 't': {
            if (opt == argc - 2)
	      error ("Option `%s` expected file name\n", argv[opt]);
            tlb_log = fopen (argv[++opt], "w+");
            if (!tlb_log)
	      error ("Could not open file `%s`", argv[opt]);

        } else if (strcmp (argv[opt], "--log-page-table") == 0) case 'l': {
            if (opt == argc - 2)
	      error ("Option `%s` expected file name\n", argv[opt]);
            pt_log = fopen (argv[++opt], "w+");
            if (!pt_log)
	      error ("Could not open file `%s`", argv[opt]);
        }

        else if (strcmp (argv[opt], "--log-command") == 0) case 'c': {
            if (opt == argc - 2)
	      error ("Option `%s` expected file name\n", argv[opt]);
            vmm_log = fopen (argv[++opt], "w+");
            if (!vmm_log)
	      error ("Could not open file `%s`\n", argv[opt]);
        }

        else default: {
            error (usage, argv[0]);
        }}
    }

  if (opt == argc)
    error (usage, argv[0]);
  backstore = fopen (argv[opt], "r+");
  if (!backstore)
    error ("Could not open file `%s`\n", argv[opt]);

  /* Initialisation.  */
  tlb_init (tlb_log);
  pt_init (pt_log);
  pm_init (backstore, pm_log);
  vmm_init (vmm_log);

  /* Exécution.  */
  yyparse ();

  /* Cleanup and print summary.  */
  fprintf (stdout, "\n\n");
  tlb_clean ();
  pt_clean ();
  pm_clean ();
  vmm_clean ();
  if (pm_log)  fclose (pm_log);
  if (tlb_log) fclose (tlb_log);
  if (vmm_log) fclose (vmm_log);
  fclose (backstore);

  return EXIT_SUCCESS;
}

// ༽つ۞﹏۞༼つ