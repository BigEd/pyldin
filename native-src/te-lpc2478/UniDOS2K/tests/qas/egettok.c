#include <ctype.h>
#include "qas.h"
#include "eval.h"

static int handle_string(void)
{
  char str[256], c, *cur = str;
  int gotend = 0;

  for(;;) {
    if(cur - str > 255) {
      error("String too long");
      eval_error = 1;
      return -1;
    }
    c = *input++;
    if(c == 0) {
      error("Unexpected end of string");
      eval_error = 1;
      return -1;
    }
    if(c == '"') {
      *cur = 0;
      return insert(str, STRING);
    }
    if(gotend == 1) {
      warning("NULL character found within string");
      gotend = 2;
    }
    if(c == '\\') {
      c = *input++;
      switch(c) {
        case 0    : error("Line splicing not available"); break;
        case 'n'  : *cur++ = '\n'; break;
        case 't'  : *cur++ = '\t'; break;
        case '\'' : *cur++ = '\''; break;
        case '"'  : *cur++ = '"' ; break;
        case '?'  : *cur++ = '\?'; break;
        case '\\' : *cur++ = '\\'; break;
        case 'a'  : *cur++ = '\a'; break;
        case 'b'  : *cur++ = '\b'; break;
        case 'f'  : *cur++ = '\f'; break;
        case 'r'  : *cur++ = '\r'; break;
        case 'v'  : *cur++ = '\v'; break;
        case 'x'  :
        case 'X'  : error("Hexadecial constants not available in this form"); break;
        case '0'  : *cur++ = 0; gotend = 1;
        	    if(*input >= '0' && *input <= '7')
        	      warning("Possible (unsupported) octal constant?");
        	    break;
        default:
          error("Unknown escape character `\\%c'", c);
          eval_error = 1;
          return -1;
      }
    } else {
      *cur++ = c;
    }
  }
}

int egettok(void)
{
  int c, cconst, p, b=0, s;
  struct Value *pos;
  enum ValueType type;

  for(;;) {
    c = *input++;

    /* skip whitespace */
    if(c == ' ')
      ;

    /* get number */
    else if(isdigit(c)) {
      input--;
      if(c == '0') {
        if(*(input+1) == 'x' || *(input+1) == 'X') {
          input += 2;
          s = sscanf(input, "%x", &tokenval);	/* for hex nos */
        } else if(isdigit(*(input+1))) {
          input++;
          s = sscanf(input, "%o", &tokenval);	/* for octal nos */
        } else {
          tokenval = 0;
          s = 1;
        }
      } else {
        s = sscanf(input, "%d", &tokenval);	/* decimal otherwise */
      }
      /* skip 0-9a-fA-F */
      while((*input >= '0' && *input <= '9') ||
            (*input >= 'a' && *input <= 'f') ||
            (*input >= 'A' && *input <= 'F'))
        input++;
      if(s != 1) {
        error("Bad number");
        done = 1;
        return UNKNOWN;
      }
      return NUM;
    }

    /* check end of string */
    else if(c == '\0') {
      done = 1;
      return DONE;
    }

    /* check char constants */
    else if(c == '\'') {
      if((cconst = *input++) == '\\') {
        if(isdigit(*input)) {
          if(*input == '0') {
            if(*(input+1) == '\'') {
              input += 2;
              tokenval = 0;
              return NUM;
            }
            if(*(input+1) == 'x' || *(input+1) == 'X') {
              input += 2;
              s = sscanf(input, "%x", &tokenval);	/* for hex nos */
            } else {
              input++;
              s = sscanf(input, "%o", &tokenval);	/* for octal nos */
            }
          } else {
            s = sscanf(input, "%d", &tokenval);	/* decimal otherwise */
          }
          if(s != 1) {
            error("Bad number");
            done = 1;
            return UNKNOWN;
          }
          /* skip to ' */
          while(*input != '\'')
            input++;
          input++;
          return NUM;
        } else {
          if(*(input+1) != '\'') {
            error("Bad character constant");
            done = 1;
            return UNKNOWN;
          }
          input+=2;
          switch(*(input-2)) {
            case 0	: error("Line splicing not available");
            		  done = 1; return UNKNOWN;
            case 'n'	: tokenval = '\n'; return NUM;
            case 't'	: tokenval = '\t'; return NUM;
            case '\''	: tokenval = '\''; return NUM;
            case '"'	: tokenval = '\"'; return NUM;
            case '?'	: tokenval = '\?'; return NUM;
            case '\\'	: tokenval = '\\'; return NUM;
            case 'a'	: tokenval = '\a'; return NUM;
            case 'b'	: tokenval = '\b'; return NUM;
            case 'f'	: tokenval = '\f'; return NUM;
            case 'r'	: tokenval = '\r'; return NUM;
            case 'v'	: tokenval = '\v'; return NUM;
            case '0'	: tokenval = 0   ; return NUM;
          }
          error("Unknown escape character `\\%c'", *(input-2));
          done = 1;
          return UNKNOWN;
        }
      } else {
        if(*input++ != '\'') {
          error("Bad character constant");
          done = 1;
          return UNKNOWN;
        }
        tokenval = cconst;
        return NUM;
      }
    }

    /* check strings */
    else if(c == '"') {
      p = handle_string();
      if(p == -1)
        return UNKNOWN;
      tokenval = p;
      return STRING;
    }

    /* check single-char ops */
    else if(c=='(' || c==')') {
      return ((c=='(') ? OBRACK : CBRACK);
    }

    /* check operators */
    else if(strchr("&=|<>~!+-/%*^", c)) {
      while(strchr("&=|<>~!+-/%*^", c)) {
        lexbuf[b++] = c;
        c = *input++;
        if(b > 3) {
          lexbuf[b] = EOS;
          error("Operator `%s...' too long", lexbuf);
          done = 1;
          return UNKNOWN;
        }
      }
      lexbuf[b] = EOS;
      if(c != EOS)
        input--;
      if((p = olookup(lexbuf)) == 0) {
        error("Unknown operator `%s'", lexbuf);
        done = 1;
        return UNKNOWN;
      } else
        return p;
    }

    /* must be an id */
    else {
      input--;
      for(;;) {
        c = *input++;
        if(isalnum(c) || c == '`' || c == '_' || c == '$' || c == '.') {
          lexbuf[b++] = c;
          if(b >= MAX_VNAME) {
            error("Value name too long");
            done = 1;
            return UNKNOWN;
          }
        } else
          break;
      }
      lexbuf[b] = EOS;
      input--;


      pos = getvaluepos(&sets, lexbuf);
      if(pos == NULL)
        pos = getvaluepos(&llabels, lexbuf);

      if(pos == NULL) {
        done = 1;
        error("Unknown identifier `%s'", lexbuf);
        return UNKNOWN;
      }
      type = getvaluetype(pos);
      if(type == V_STRING) {
        tokenval = insert(getvaluestr(pos), STRING);
        return STRING;
      } else if(type == V_INT) {
        tokenval = getvalueint(pos);
        return NUM;
      } else {
        done = 1;
        error("Unknown type of identifier `%s'", lexbuf);
        return UNKNOWN;
      }
    }
  }
}
