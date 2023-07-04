//+++2001-10-06
//    Copyright (C) 2001, Mike Rieker, Beverly, MA USA
//
//    This program is free software; you can redistribute it and/or modify
//    it under the terms of the GNU General Public License as published by
//    the Free Software Foundation; version 2 of the License.
//
//    This program is distributed in the hope that it will be useful,
//    but WITHOUT ANY WARRANTY; without even the implied warranty of
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//    GNU General Public License for more details.
//
//    You should have received a copy of the GNU General Public License
//    along with this program; if not, write to the Free Software
//    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//---2001-10-06

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef enum Ttype Ttype;
typedef struct Opcode Opcode;
typedef struct Sline Sline;
typedef struct Symbol Symbol;
typedef struct Token Token;
typedef unsigned char uByte;
typedef unsigned int uLong;

struct Opcode { void (*proc) (Opcode *opcode, Token *token);
                uLong parm;
                const char *name;
              };

struct Sline { Sline *next;
               int number;
               char text[1];
             };

struct Symbol { Symbol *next;
                uLong value;
                int reloc;
                int entry;
                char name[1];
              };

struct Token { Token *next;
               Ttype ttype;
               uLong value;
               int column;
               char string[1];
             };

enum Ttype {
	TT_EOL, 
	TT_NAME, 
	TT_REG, 
	TT_WITH, 
	TT_STATE, 
	TT_VALUE, 
	TT_SHFTL, 
	TT_SHFTR, 
  	TT_GE,    
	TT_LE,    
	TT_EQ,    
	TT_NE,   
	TT_GT,    
	TT_LT,    
	TT_SET,   
	TT_COLON, 
	TT_OPEN,  
	TT_CLOSE, 
	TT_COMMA, 
	TT_AND,   
	TT_OR,    
	TT_NOT,   
	TT_PLUS,  
	TT_MINUS, 
	TT_TIMES, 
	TT_SLASH, 
	TT_DOT
};

static const struct { Ttype ttype; const char *string; } tt[] = {
	TT_SHFTL, "<<", 
	TT_SHFTR, ">>", 
	TT_GE,    ">=", 
	TT_LE,    "<=", 
	TT_EQ,    "==", 
	TT_NE,    "!=", 
	TT_GT,    ">", 
	TT_LT,    "<", 
	TT_SET,   "=", 
	TT_COLON, ":", 
	TT_OPEN,  "(", 
	TT_CLOSE, ")", 
	TT_COMMA, ",", 
	TT_AND,   "&", 
	TT_OR,    "|", 
	TT_NOT,   "~", 
	TT_PLUS,  "+", 
	TT_MINUS, "-", 
	TT_TIMES, "*", 
	TT_SLASH, "/", 
	TT_DOT,   "." };

static const struct { int size; int addr; const char *name; } regs[] = {
	1, 0x00, "scntl0", 
	1, 0x01, "scntl1", 
	1, 0x02, "scntl2", 
	1, 0x03, "scntl3", 
	1, 0x04, "scid", 
	1, 0x05, "sxfer", 
	1, 0x06, "sdid", 
	1, 0x07, "gpreg", 
	1, 0x08, "sfbr", 
	1, 0x09, "socl", 
	1, 0x0A, "ssid", 
	1, 0x0B, "sbcl", 
	1, 0x0C, "dstat", 
	1, 0x0D, "sstat0", 
	1, 0x0E, "sstat1", 
	1, 0x0F, "sstat2", 
	4, 0x10, "dsa", 
	1, 0x14, "istat", 
	1, 0x18, "ctest0", 
	1, 0x19, "ctest1", 
	1, 0x1A, "ctest2", 
	1, 0x1B, "ctest3", 
	4, 0x1C, "temp", 
	1, 0x20, "dfifo", 
	1, 0x21, "ctest4", 
	1, 0x22, "ctest5", 
	1, 0x23, "ctest6", 
	4, 0x24, "dbc", 
	4, 0x28, "dnad", 
	4, 0x2C, "dsp", 
	4, 0x30, "dsps", 
	4, 0x34, "scratcha", 
	1, 0x38, "dmode", 
	1, 0x39, "dien", 
	1, 0x3A, "sbr", 
	1, 0x3B, "dcntl", 
	4, 0x3C, "adder", 
	1, 0x40, "sien0", 
	1, 0x41, "sien1", 
	1, 0x42, "sist0", 
	1, 0x43, "sist1", 
	1, 0x44, "slpar", 
	1, 0x45, "swide", 
	1, 0x46, "macntl", 
	1, 0x47, "gpcntl", 
	1, 0x48, "stime0", 
	1, 0x49, "stime1", 
	1, 0x4A, "respid0", 
	1, 0x4B, "respid1", 
	1, 0x4C, "stest0", 
	1, 0x4D, "stest1", 
	1, 0x4E, "stest2", 
	1, 0x4F, "stest3", 
	4, 0x50, "sidl", 
	4, 0x54, "sodl", 
	4, 0x58, "sbdl", 
	4, 0x5C, "scratchb", 
	4, 0x60, "scratchc", 
	4, 0x64, "scratchd", 
	4, 0x68, "scratche", 
	4, 0x6C, "scratchf", 
	4, 0x70, "scratchg", 
	4, 0x74, "scratchh", 
	4, 0x78, "scratchi", 
	4, 0x7C, "scratchj", 
};

static const char *states[8] = { "data_out", "data_in", "cmd", "status", "res_out", "res_in", "msg_out", "msg_in" };

static int errors, passno;
static Sline *sline, *slines;
static Symbol *symbols;
static uByte *relbuff, *objbuff;
static uLong dot, objsize, objused;

static void pass (void);
static Token *tokenise (Sline *sline);
static Token *getexp (Token *token, uLong *value_r, int *reloc_r);
static Token *getoperand (Token *token, uLong *value_r, int *reloc_r);
static void definesymbol (int dupcheck, char *name, uLong value, int reloc);
static void emit (int size, uLong value, int reloc);
static void printerror (const char *format, ...);

int main ()

{
  char slinebuf[4096];
  int i, j, slineno;
  Sline **lsline;
  Symbol *symbol;

  /* Read source file into memory */

  lsline = &slines;
  slineno = 0;
  while (gets (slinebuf) != NULL) {
    sline = malloc (strlen (slinebuf) + sizeof *sline);
    *lsline = sline;
    lsline = &(sline -> next);
    sline -> number = ++ slineno;
    strcpy (sline -> text, slinebuf);
  }
  *lsline = NULL;

  /* Set up an initial object buffer */

  objsize = 32;
  objused = 0;
  relbuff = malloc (objsize);
  objbuff = malloc (objsize);
  symbols = NULL;

  /* Scan through lines to build symbol table */

  passno = 1;
  pass ();

  /* Scan through lines to fill object code array */

  errors = 0;
  passno = 2;
  pass ();
  if (errors) return (4);

  /* Output symbol table */

  for (symbol = symbols; symbol != NULL; symbol = symbol -> next) {
    fprintf (stderr, "%8.8x %c%c  %s\n", symbol -> value, 
                                         symbol -> reloc ? 'R' : ' ', 
                                         symbol -> entry ? 'E' : ' ', 
                                         symbol -> name);
    if (symbol -> entry) {
      printf ("#define script_ent%s_%s 0x%8.8x\n", symbol -> reloc ? "r" : "", symbol -> name, symbol -> value);
    }
  }
  fprintf (stderr, "%8.8x bytes used\n", objused);

  /* Output object file */

  while (objsize & 3) objbuff[objsize++] = 0x69;

  printf ("static const uLong script_obj[] = {");
  for (i = 0; i < objused / 4; i += j) {
    printf ("\n/*%4.4x*/", i * 4);
    for (j = 0; (j < 4) && (i + j < objused / 4); j ++) {
      if (i + j == 0) printf ("  ");
      else printf (" ,");
      printf ("0x%8.8x", ((uLong *)objbuff)[i+j]);
    }
  }
  printf (" };\n");

  j = 0;
  for (i = 0; i < objused / 4; i ++) {
    if (((uLong *)relbuff)[i]) {
      if (!j) {
        printf ("static const uLong script_rel[] = {\n %u", i);
        j = 1;
      } else {
        printf (",%u", i);
      }
    }
  }
  if (j) printf (" };\n");

  /* Done */

  return (0);  
}

/************************************************************************/
/*									*/
/*  Opcode decoding table						*/
/*									*/
/************************************************************************/

static void op_data  (Opcode *opcode, Token *token);
static void op_entry (Opcode *opcode, Token *token);
static void op_org   (Opcode *opcode, Token *token);

static void op_block (Opcode *opcode, Token *token);
static void op_wait  (Opcode *opcode, Token *token);

static const Opcode opcodes[] = {
	op_data,     4, "long", 
	op_data,     2, "word", 
	op_data,     1, "byte", 
	op_org,      0, "org", 
	op_entry,    0, "entry", 

	op_block, 0x00, "chmov", 
	op_block, 0x08, "move", 
	op_jump,  0x??, "call", 
	op_jump,  0x??, "jump", 
	op_wait,     0, "wait" };

/************************************************************************/
/*									*/
/*  Perform a pass over the source file					*/
/*									*/
/************************************************************************/

static void pass (void)

{
  char *name;
  int i, reloc;
  Token *tokens;
  uLong value;

  dot = 0;
  for (sline = slines; sline != NULL; sline = sline -> next) {

    /* Chop the line into tokens */

    tokens = tokenise (sline);

    /* Process any labels on the line */

    while ((tokens -> ttype != TT_EOL) && (tokens -> ttype == TT_NAME) && (tokens -> next != NULL) && (tokens -> next -> ttype == TT_COLON)) {
      definesymbol (1, tokens -> string, dot, 1);
      tokens = tokens -> next -> next;
    }

    /* If null line from there, skip it */

    if (tokens -> ttype == TT_EOL) continue;

    /* If <name> =, set the name equal to the expression */

    if ((tokens -> ttype == TT_NAME) && (tokens -> next -> ttype == TT_SET)) {
      name = tokens -> string;
      tokens = getexp (tokens, &value, &reloc);
      if (tokens -> ttype != TT_EOL) {
        printerror ("extra data following expression at column %d", tokens -> column);
        continue;
      }
      if (reloc >= 0) definesymbol (0, name, value, reloc);
      continue;
    }

    /* Decode the opcode and process it */

    if (tokens -> ttype == TT_NAME) {
      for (i = 0; i < sizeof opcodes / sizeof opcodes[0]; i ++) {
        if (strcasecmp (opcodes[i].name, tokens -> string) == 0) {
          (*(opcodes[i].proc)) (opcodes + i, tokens -> next);
          goto nextline;
        }
      }
    }
    printerror ("unknown opcode %s\n", tokens -> string);
nextline:;
  }
}

/************************************************************************/
/*									*/
/*  Directives								*/
/*									*/
/************************************************************************/

/* Allocate storage for a long/word/byte and fill it in */

static void op_data (Opcode *opcode, Token *token)

{
  int reloc;
  uLong value;

  while (token -> ttype != TT_EOL) {
    token = getexp (token, &value, &reloc);
    if (token -> ttype == TT_COMMA) token = token -> next;
    else if (token -> ttype != TT_EOL) {
      printerror ("bad expression at column %d", token -> column);
      return;
    }
    emit (opcode -> parm, value, reloc);
  }
}

/* Declare a symbol as global (ENTRY) */

static void op_entry (Opcode *opcode, Token *token)

{
  Symbol *symbol;

  while (token -> ttype != TT_EOL) {
    if (token -> ttype != TT_COMMA) {
      if (token -> ttype != TT_NAME) {
        printerror ("must have a symbol name listed at column %d", token -> column);
        return;
      }
      for (symbol = symbols; symbol != NULL; symbol = symbol -> next) {
        if (strcasecmp (symbol -> name, token -> string) == 0) {
          symbol -> entry = 1;
          break;
        }
      }
      if (symbol == NULL) printerror ("undefined symbol %s", token -> string);
    }
    token = token -> next;
  }
}

/* Move the '.' */

static void op_org (Opcode *opcode, Token *token)

{
  int reloc;
  uLong value;

  token = getexp (token, &value, &reloc);
  if (token -> ttype != TT_EOL) {
    printerror ("extra data on end of line at column %d", token -> column);
    return;
  }
  if (reloc != 1) {
    printerror ("expression must be relocatable");
    return;
  }
  dot = value;
}

/************************************************************************/
/*									*/
/*  Opcodes								*/
/*									*/
/************************************************************************/

/* CHMOV/MOVE { count, [PTR] adrs | FROM disp }, WITH/WHILE state */

static void op_block (Opcode *opcode, Token *token)

{
  int count_reloc, where_reloc;
  uByte dcmd;
  uLong count_value, where_value;

  dcmd = opcode -> parm;

  if (token -> ttype == TT_FROM) {
    count_value = 0;
    count_reloc = 0;
    token = getexp (token -> next, &where_value, &where_reloc);
    dcmd |= 0x10;
  } else {
    token = getexp (token, &count_value, &count_reloc);
    if (count_reloc != 0) {
      printerror ("count must be absolute");
      return;
    }
    if (token -> ttype != TT_COMMA) {
      printerror ("missing comma after count");
      return;
    }
    token = token -> next;
    if (token -> ttype == TT_PTR) {
      dcmd |= 0x20;
      token = token -> next;
    }
    token = getexp (token -> next, &where_value, &where_reloc);
  }
  if (token -> ttype != TT_COMMA) {
    printerror ("missing comma after address");
    return;
  }
  token = token -> next;
  if (token -> ttype == TT_WITH) dcmd ^= 0x08;
  else if (token -> ttype != TT_WHEN) {
    printerror ("missing WHEN/WITH clause after address");
    return;
  }
  token = token -> next;
  if (token -> ttype != TT_STATE) {
    printerror ("invalid scsi state");
    return;
  }
  dbc |= token -> value;
  if (token -> next -> ttype != TT_EOL) {
    printerror ("extra data after scsi state in column %d", token -> column);
    return;
  }
  emit (3, count_value, count_reloc);
  emit (1, dbc, 0);
  emit (4, where_value, where_reloc);
}

/* CALL/JUMP {REL (address) | address} [, {IF|WHEN} [NOT] [{ATN|phase}] [AND|OR] [data[AND MASK data]]]

void op_jump (Opcode *opcode, Token *token)

{
  int reloc;
  uLong dcmdl, value;

  dcmdl = (opcode -> parm) << 24;
  if (token -> ttype == TT_REL) {
    dcmdl |= 0x00800000;
    token = token -> next;
  }
  token = getexp (token, &value, &reloc);
  if (token -> ttype == TT_COMMA) {
    token = token -> next;
    if (token -> ttype == TT_WHEN) dcmdl |= 0x00010000;
    else if (token -> ttype != TT_IF) {
      printerror ("missing IF/WHEN clause after address");
      return;
    }
    token = token -> next;
    if (token -> ttype == TT_NOT) token = token -> next;
    else dcmdl |= 0x00080000;
    if (token -> ttype == TT_ATN) {
      dcmdl |= 0x????????;
      token = token -> next;
    } else if (token -> ttype == TT_STATE) {
      dcmdl |= 0x00020000;
      dcmdl |= token -> value << 24;
      token = token -> next;
    }
    ??
    ??
  }

  emit (4, dcmdl, 0);
  emit (4, value, reloc);
}

/* WAIT SELECT/RESELECT, address */

void op_wait (Opcode *opcode, Token *token)

{
  int code, jump_reloc;
  uLong jump_value;

  if ((token == NULL) || (token -> ttype != TT_NAME)) {
    printerror ("missing SELECT/RESELECT after WAIT");
    return;
  }

  code = 1;
  if (strcasecmp (token -> string, "select") != 0) {
    code = 2;
    if (strcasecmp (token -> string, "reselect") == 0) {
      printerror ("missing SELECT/RESELECT after WAIT");
      return;
    }
  }

  if (token -> ttype == TT_COMMA) token = token -> next;

  token = getexp (token, &jump_value, &jump_reloc);
  if (token -> ttype != TT_EOL) printerror ("extra data at column %d", token -> column);

  /* ?? this is all wrong ?? */

  if (jump_reloc) {
    emit (3, jump_value - dot - 8, 0);
    emit (1, 0x80 | code, 0);
    emit (4, 0, 0);
  } else {
    emit (3, 0, 0);
    emit (1, 0x80 | code, 0);
    emit (4, jump_value, 0);
  }
}

/************************************************************************/
/*									*/
/*  Tokenise a source line						*/
/*									*/
/************************************************************************/

static Token *tokenise (Sline *sline)

{
  char c, *p, *q;
  int i, l;
  Token **ltoken, *token;
  uLong value;

  static Token *last_tokens = NULL;

  /* Free off last batch of tokens */

  if (last_tokens != NULL) {
    while ((token = last_tokens) -> ttype != TT_EOL) {
      last_tokens = token -> next;
      free (token);
    }
    free (last_tokens);
    last_tokens = NULL;
  }

  /* Parse this source line up into tokens */

  ltoken = &last_tokens;
  for (p = sline -> text; (c = *p) != 0; p ++) {
    if (c <= ' ') continue;
    if (c == ';') break;

    /* Names are a letter followed by letters/numbers/underscores */

    if ((c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z')) {
      for (q = p; (c = *q) != 0; q ++) {
        if ((c < '0' || c > '9') && (c < 'A' || c > 'Z') && (c < 'a' || c > 'z') && (c != '_')) break;
      }
      token = malloc ((q - p) + sizeof *token);
      token -> ttype  = TT_NAME;
      token -> column = p - sline -> text;
      memcpy (token -> string, p, q - p);
      token -> string[q-p] = 0;
      p = -- q;

      /* Some names are reserved-words so we give them special token types */

      for (i = 0; i < sizeof regs / sizeof regs[0]; i ++) {
        if (strcasecmp (regs[i].name, token -> string) == 0) {
          token -> ttype = TT_REG;
          token -> value = regs[i].addr | (regs[i].size << 16);
          break;
        }
        if (regs[i].size == 4) {
          l = strlen (regs[i].name);
          if ((strncasecmp (regs[i].name, token -> string, l) == 0) 
           && (token -> string[l+1] == 0) 
           && (token -> string[l] >= '0') 
           && (token -> string[l] <= '3')) {
            token -> ttype = TT_REG;
            token -> value = (regs[i].addr + token -> string[l] - '0') | (1 << 16);
            break;
          }
        }
      }
      if ((strcasecmp (token -> string, "with") == 0) || (strcasecmp (token -> string, "when") == 0)) {
        token -> ttype = TT_WITH;
      }
      for (i = 0; i < sizeof states / sizeof states[0]; i ++) {
        if (strcasecmp (states[i], token -> string) == 0) {
          token -> ttype = TT_STATE;
          token -> value = i;
          break;
        }
      }
    }

    /* Values are a digit followed by other digits (per strtoul) */

    else if (c >= '0' && c <= '9') {
      value = strtoul (p, &q, 0);
      token = malloc ((q - p) + sizeof *token);
      token -> ttype  = TT_VALUE;
      token -> column = p - sline -> text;
      token -> value  = value;
      memcpy (token -> string, p, q - p);
      token -> string[q-p] = 0;
      p = -- q;
    }

    /* Find other things in the table */

    else {
      for (i = 0; i < sizeof tt / sizeof tt[0]; i ++) {
        l = strlen (tt[i].string);
        if (strncasecmp (tt[i].string, p, l) == 0) break;
      }
      if (tt[i].string == NULL) {
        printerror ("unknown char %c at column %d", c, p - sline -> text);
        break;
      }
      token = malloc ((q - p) + sizeof *token);
      token -> ttype  = tt[i].ttype;
      token -> column = p - sline -> text;
      token -> value  = value;
      memcpy (token -> string, p, l);
      token -> string[l] = 0;
      p += -- l;
    }

    /* Link new token on end of list */

    *ltoken = token;
    ltoken = &(token -> next);
  }

  /* End token list and return pointer to first in list */

  token = malloc (sizeof *token);
  *ltoken = token;
  token -> ttype  = TT_EOL;
  token -> column = p - sline -> text;
  token -> value  = 0;
  token -> string[0] = 0;
  token -> next = token;
  return (last_tokens);
}

/************************************************************************/
/*									*/
/*  Get expression							*/
/*									*/
/*    Input:								*/
/*									*/
/*	token = starting token of expression				*/
/*									*/
/*    Output:								*/
/*									*/
/*	getexp = terminating token (or NULL if end of line)		*/
/*	*value_r = value of expression					*/
/*	*reloc_r = relocation of expression				*/
/*									*/
/************************************************************************/

static Token *getexp (Token *token, uLong *value_r, int *reloc_r)

{
  int left_reloc, rite_reloc;
  Symbol *symbol;
  Ttype operator;
  uLong left_value, rite_value;

  *value_r = 0;
  *reloc_r = 0;

  token = getoperand (token, &left_value, &left_reloc);
  if (left_reloc < 0) return (token);

get_operator:
  if (token == NULL) {
    *value_r = left_value;
    *reloc_r = left_reloc;
    return (NULL);
  }

  operator = token -> ttype;
  switch (operator) {
    case TT_SHFTL:
    case TT_SHFTR:
    case TT_GE:
    case TT_LE:
    case TT_EQ:
    case TT_NE:
    case TT_GT:
    case TT_LT:
    case TT_AND:
    case TT_OR:
    case TT_PLUS:
    case TT_MINUS:
    case TT_TIMES:
    case TT_SLASH: break;
    default: {
      *value_r = left_value;
      *reloc_r = left_reloc;
      return (token);
    }
  }

  token = getoperand (token -> next, &rite_value, &rite_reloc);
  if (rite_reloc < 0) return (token);

  switch (operator) {
    case TT_SHFTL: {
      if ((left_reloc | rite_reloc) != 0) goto bad_reloc;
      left_value <<= rite_value;
      break;
    }
    case TT_SHFTR: {
      if ((left_reloc | rite_reloc) != 0) goto bad_reloc;
      left_value >>= rite_value;
      break;
    }
    case TT_GE: {
      if (left_reloc != rite_reloc) goto bad_reloc;
      left_value = (left_value >= rite_value);
      left_reloc = 0;
      break;
    }
    case TT_LE: {
      if (left_reloc != rite_reloc) goto bad_reloc;
      left_value = (left_value <= rite_value);
      left_reloc = 0;
      break;
    }
    case TT_EQ: {
      if (left_reloc != rite_reloc) goto bad_reloc;
      left_value = (left_value == rite_value);
      left_reloc = 0;
      break;
    }
    case TT_NE: {
      if (left_reloc != rite_reloc) goto bad_reloc;
      left_value = (left_value != rite_value);
      left_reloc = 0;
      break;
    }
    case TT_GT: {
      if (left_reloc != rite_reloc) goto bad_reloc;
      left_value = (left_value > rite_value);
      left_reloc = 0;
      break;
    }
    case TT_LT: {
      if (left_reloc != rite_reloc) goto bad_reloc;
      left_value = (left_value < rite_value);
      left_reloc = 0;
      break;
    }
    case TT_AND: {
      if ((left_reloc | rite_reloc) != 0) goto bad_reloc;
      left_value &= rite_value;
      break;
    }
    case TT_OR: {
      if ((left_reloc | rite_reloc) != 0) goto bad_reloc;
      left_value |= rite_value;
      break;
    }
    case TT_PLUS: {
      if (left_reloc + rite_reloc > 1) goto bad_reloc;
      left_value += rite_value;
      left_reloc += rite_reloc;
      break;
    }
    case TT_MINUS: {
      if (rite_reloc > left_reloc) goto bad_reloc;
      left_value -= rite_value;
      left_reloc -= rite_reloc;
      break;
    }
    case TT_TIMES: {
      if ((left_reloc | rite_reloc) != 0) goto bad_reloc;
      left_value *= rite_value;
      break;
    }
    case TT_SLASH: {
      if ((left_reloc | rite_reloc) != 0) goto bad_reloc;
      left_value /= rite_value;
      break;
    }
  }

  goto get_operator;

bad_reloc:
  printerror ("bad relocation for expression");
  *value_r = 0;
  *reloc_r = 1;
  return (NULL);
}

static Token *getoperand (Token *token, uLong *value_r, int *reloc_r)

{
  int startcol;
  Symbol *symbol;

  switch (token -> ttype) {
    case TT_NAME: {
      for (symbol = symbols; symbol != NULL; symbol = symbol -> next) {
        *value_r = symbol -> value;
        *reloc_r = symbol -> reloc;
        return (token -> next);
      }
      printerror ("undefined symbol %s\n", token -> string);
      *value_r = 0;
      *reloc_r = 0;
      return (token -> next);
    }
    case TT_VALUE: {
      *value_r = token -> value;
      *reloc_r = 0;
      return (token -> next);
    }
    case TT_OPEN: {
      startcol = token -> column;
      token = getexp (token -> next, value_r, reloc_r);
      if (token -> ttype != TT_CLOSE) {
        printerror ("missing close paren for expression starting in column %d", startcol);
        return (token);
      }
      return (token -> next);
    }
    case TT_NOT: {
      token = getexp (token -> next, value_r, reloc_r);
      if (*reloc_r != 0) printerror ("bad relocation for unary '~'");
      else *value_r = ~ *value_r;
      return (token);
    }
    case TT_MINUS: {
      token = getexp (token -> next, value_r, reloc_r);
      if (*reloc_r != 0) printerror ("bad relocation for unary '-'");
      else *value_r = - *value_r;
      return (token);
    }
    case TT_DOT: {
      *value_r = dot;
      *reloc_r = 1;
      return (token -> next);
    }
    default: {
      printerror ("missing operand at column %d", token -> column);
      *value_r = 0;
      *reloc_r = 0;
      return (token);
    }
  }
}

/************************************************************************/
/*									*/
/*  Define a symbol							*/
/*									*/
/************************************************************************/

static void definesymbol (int dupcheck, char *name, uLong value, int reloc)

{
  Symbol *symbol;

  for (symbol = symbols; symbol != NULL; symbol = symbol -> next) {
    if (strcasecmp (name, symbol -> name) == 0) {
      if (dupcheck && ((symbol -> value != value) || (symbol -> reloc != reloc))) {
        printerror ("duplicate symbol '%s'", name);
        printerror (" - old value %8.8x %c", symbol -> value, symbol -> reloc ? '*' : ' ');
        printerror (" - new value %8.8x %c",           value,           reloc ? '*' : ' ');
      }
      symbol -> value = value;
      symbol -> reloc = reloc;
      return;
    }
  }

  symbol = malloc (strlen (name) + sizeof *symbol);
  symbol -> next  = symbols;
  symbol -> value = value;
  symbol -> reloc = reloc;
  symbol -> entry = 0;
  strcpy (symbol -> name, name);
  symbols = symbol;
}

/************************************************************************/
/*									*/
/*  Emit a value to the object file					*/
/*									*/
/*    Input:								*/
/*									*/
/*	size  = number of bytes in value				*/
/*	value = value to be output					*/
/*	reloc = 0 : value is absolute					*/
/*	        1 : value is relative					*/
/*									*/
/************************************************************************/

static void emit (int size, uLong value, int reloc)

{
  if (passno == 1) dot += size;
  if (passno == 2) {
    if (reloc && ((dot & 3) || (size != 4))) {
      printerror ("unaligned relocation at %x", dot);
      return;
    }
    if (dot + size > objsize) {
      objsize = dot + size + 32;
      objbuff = realloc (objbuff, objsize);
      relbuff = realloc (relbuff, objsize);
    }
    if (dot > objused) {
      memset (objbuff + objused, 0x69, dot - objused);
      memset (relbuff + objused,    0, dot - objused);
    }
    memset (relbuff + dot, 0, size);
    relbuff[dot] = reloc;
    while (-- size >= 0) {
      objbuff[dot++] = value;
      value >>= 8;
    }
    if (dot > objused) objused = dot;
  }
}

/************************************************************************/
/*									*/
/*  Print error message only on pass 2					*/
/*									*/
/************************************************************************/

static void printerror (const char *format, ...)

{
  va_list ap;

  if (passno == 2) {
    errors = 1;
    va_start (ap, format);
    fprintf (stderr, "%d: ", sline -> number);
    vfprintf (stderr, format, ap);
    fprintf (stderr, "\n");
    va_end (ap);
  }
}
