#include "qas.h"
#include "instr.h"
#include "ainstr.h"

void parseinstr(char *iname)
{
  char c1, c2, c3, c4;
  int ok = 0;

  strtolower(iname);
  c1 = (*iname);
  c2 = (*(iname+1));
  c3 = (*(iname+2));
  c4 = (*(iname+3));
  switch(c1) {
    case '=': a_declare(iname+1, DT_DCB); ok = 1; break;
    case '&': a_declare(iname+1, DT_DCD); ok = 1; break;
    case '%': a_declare(iname+1, DT_DBB); ok = 1; break;
    case 'a':	/* and add adc adr abs asn acs atn adf align */
      switch(c2) {
        case 'b': if(c3 == 's')  { a_fpunary(iname+3, 0x2); ok = 1; }
          break;
        case 'c': if(c3 == 's')  { a_fpunary(iname+3, 0xc); ok = 1; }
          break;
        case 'd': if(c3 == 'c')  { a_datapro(iname+3, 0x5); ok = 1; }
                  if(c3 == 'r')  { a_adr(iname+3); ok = 1; }
                  if(c3 == 'f')  { a_fpbinry(iname+3, 0x0); ok = 1; }
                  if(c3 == 'd')  { a_datapro(iname+3, 0x4); ok = 1; }
          break;
        case 'l':
          if(!strcmp(iname+1, "lign"))
                                 { alignarea(&odata); ok = 1; }
          break;
        case 'n': if(c3 == 'd')  { a_datapro(iname+3, 0x0); ok = 1; }
          break;
        case 's': if(c3 == 'n')  { a_fpunary(iname+3, 0xb); ok = 1; }
          break;
        case 't': if(c3 == 'n')  { a_fpunary(iname+3, 0xd); ok = 1; }
          break;
      }
      break;
    case 'b':	/* bic b bl bin */
      if(c2 == 'i' && c3 == 'c') { a_datapro(iname+3, 0xe); ok = 1; }
      else if(c2 == 'i' && c3 == 'n')
                                 { a_bin(iname+3); ok = 1; }
      else /* b */               { a_branch(iname+1); ok = 1; }
      break;
    case 'c':	/* cmp cmn cos cmf cnf cdp */
      switch(c2) {
        case 'd': if(c3 == 'p')  { a_cpoper(iname+3, 0); ok = 1; }
          break;
        case 'm': if(c3 == 'p')  { a_datapro(iname+3, 0xa); ok = 1; }
                  if(c3 == 'n')  { a_datapro(iname+3, 0xb); ok = 1; }
                  if(c3 == 'f')  { a_fpcmp(iname+3, 0x9); ok = 1; }
          break;
        case 'n': if(c3 == 'f')  { a_fpcmp(iname+3, 0xb); ok = 1; }
          break;
        case 'o': if(c3 == 's')  { a_fpunary(iname+3, 0x9); ok = 1; }
          break;
      }
      break;
    case 'd':	/* dcb dcw dcd dfn dvf dbb dbw dbd dcfs dcfd dwd dwf dwl dfl */
      switch(c2) {
        case 'b': if(c3 == 'b')  { a_declare(iname+3, DT_DBB); ok = 1; }
                  if(c3 == 'w')  { a_declare(iname+3, DT_DBW); ok = 1; }
                  if(c3 == 'd')  { a_declare(iname+3, DT_DBD); ok = 1; }
          break;
        case 'c': if(c3 == 'b')  { a_declare(iname+3, DT_DCB); ok = 1; }
                  if(c3 == 'w')  { a_declare(iname+3, DT_DCW); ok = 1; }
                  if(c3 == 'd')  { a_declare(iname+3, DT_DCD); ok = 1; }
                  if(c3 == 'f')  { a_dcf(iname+3); ok = 1; }
          break;
        case 'f': if(c3 == 'n')  { a_declare(iname+3, DT_DCD); ok = 1; }
                  if(c3 == 'l')  { a_declare(iname+3, DT_DFL); ok = 1; }
          break;
        case 'v': if(c3 == 'f')  { a_fpbinry(iname+3, 0x4); ok = 1; }
          break;
        case 'w': if(c3 == 'd')  { a_declare(iname+3, DT_DWD); ok = 1; }
                  if(c3 == 'f')  { a_declare(iname+3, DT_DWD); ok = 1; }
                  if(c3 == 'l')  { a_declare(iname+3, DT_DWL); ok = 1; }
          break;
      }
      break;
    case 'e':	/* exp eor */
      switch(c2) {
        case 'o': if(c3 == 'r')  { a_datapro(iname+3, 0x1); ok = 1; }
          break;
        case 'q':
          if(c3 == 'u') {
            switch(c4) {
              case 'b': a_declare(iname+4, DT_DCB); ok = 1; break;
              case 'd': a_declare(iname+4, DT_DCD); ok = 1; break;
              case 's': a_declare(iname+4, DT_DCB); ok = 1; break;
              case 'w': a_declare(iname+4, DT_DCW); ok = 1; break;
            }
          }
          break;
        case 'r':
          if(!strcmp(iname+1, "rror")) {
            error("** ERROR instruction indicates exception **");
            ok = 1;
          }
          break;
        case 'x': if(c3 == 'p')  { a_fpunary(iname+3, 0x7); ok = 1; }
          break;
      }
    case 'f':	/* flt fix fml fdv frd */
    		/* f0d f0e f0f ft6 ft7 ft8 ft9 fta ftb ftc ftd fte ftf */
      switch(c2) {
        case '0': warning("Unusual floating point instruction!");
                  if(c3 == 'd')  { a_fpbinry(iname+3, 0xd); ok = 1; }
                  if(c3 == 'e')  { a_fpbinry(iname+3, 0xe); ok = 1; }
                  if(c3 == 'f')  { a_fpbinry(iname+3, 0xf); ok = 1; }
        case 'd': if(c3 == 'v')  { a_fpbinry(iname+3, 0xa); ok = 1; }
          break;
        case 'i': if(c3 == 'x')  { a_fpregtr(iname+3, 0x1); ok = 1; }
          break;
        case 'l': if(c3 == 't')  { a_fpregtr(iname+3, 0x0); ok = 1; }
          break;
        case 'm': if(c3 == 'l')  { a_fpbinry(iname+3, 0x9); ok = 1; }
          break;
        case 'r': if(c3 == 'd')  { a_fpbinry(iname+3, 0xb); ok = 1; }
        case 't': warning("Unusual floating point instruction!");
                  if(c3 == '6')  { a_fpregtr(iname+3, 0x6); ok = 1; }
                  if(c3 == '7')  { a_fpregtr(iname+3, 0x7); ok = 1; }
                  if(c3 == '8')  { a_fpregtr(iname+3, 0x8); ok = 1; }
                  if(c3 == '9')  { a_fpregtr(iname+3, 0x9); ok = 1; }
                  if(c3 == 'a')  { a_fpregtr(iname+3, 0xa); ok = 1; }
                  if(c3 == 'b')  { a_fpregtr(iname+3, 0xb); ok = 1; }
                  if(c3 == 'c')  { a_fpregtr(iname+3, 0xc); ok = 1; }
                  if(c3 == 'd')  { a_fpregtr(iname+3, 0xd); ok = 1; }
                  if(c3 == 'e')  { a_fpregtr(iname+3, 0xe); ok = 1; }
                  if(c3 == 'f')  { a_fpregtr(iname+3, 0xf); ok = 1; }
      }
      break;
    case 'l':	/* ldr ldm ldf log lgn ldc */
      switch(c2) {
        case 'd': if(c3 == 'r')  { a_dtrans(iname+3, 1); ok = 1; }
                  if(c3 == 'm')  { a_mdtrans(iname+3, 1); ok = 1; }
                  if(c3 == 'f')  { a_fpdtran(iname+3, 1); ok = 1; }
                  if(c3 == 'c')  { a_cpdtran(iname+3, 1); ok = 1; }
          break;
        case 'g': if(c3 == 'n')  { a_fpunary(iname+3, 0x6); ok = 1; }
          break;
        case 'o': if(c3 == 'g')  { a_fpunary(iname+3, 0x5); ok = 1; }
          break;
      }
      break;
    case 'm':	/* mov mvn mul mla mvf mnf muf mcr mrc */
      switch(c2) {
        case 'c': if(c3 == 'r')  { a_cpoper(iname+3, 2); ok = 1; }
          break;
        case 'l': if(c3 == 'a')  { a_mul(iname+3, 1); ok = 1; }
          break;
        case 'n': if(c3 == 'f')  { a_fpunary(iname+3, 0x1); ok = 1; }
          break;
        case 'o': if(c3 == 'v')  { a_datapro(iname+3, 0xd); ok = 1; }
          break;
        case 'r': if(c3 == 'c')  { a_cpoper(iname+3, 1); ok = 1; }
          break;
        case 'u': if(c3 == 'l')  { a_mul(iname+3, 0); ok = 1; }
                  if(c3 == 'f')  { a_fpbinry(iname+3, 0x1); ok = 1; }
          break;
        case 'v': if(c3 == 'n')  { a_datapro(iname+3, 0xf); ok = 1; }
                  if(c3 == 'f')  { a_fpunary(iname+3, 0x0); ok = 1; }
          break;
      }
      break;
    case 'n':	/* nrm */
      if(c2 == 'r' && c3 == 'm') { a_fpunary(iname+3, 0xf); ok = 1; }
      break;
    case 'o':	/* orr */
      if(c2 == 'r' && c3 == 'r') { a_datapro(iname+3, 0xc); ok = 1; }
      break;
    case 'p':	/* pow pol */
      if(c2 == 'o') {
                  if(c3 == 'w')  { a_fpbinry(iname+3, 0x6); ok = 1; }
                  if(c3 == 'l')  { a_fpbinry(iname+3, 0xc); ok = 1; }
      }
      break;
    case 'r':	/* rfs rfc rnd rsf rdf rpw rmf */
      switch(c2) {
        case 'd': if(c3 == 'f')  { a_fpbinry(iname+3, 0x5); ok = 1; }
          break;
        case 'f': if(c3 == 's')  { a_fpregtr(iname+3, 0x3); ok = 1; }
                  if(c3 == 'c')  { a_fpregtr(iname+3, 0x5); ok = 1; }
          break;
        case 'm': if(c3 == 'f')  { a_fpbinry(iname+3, 0x8); ok = 1; }
          break;
        case 'n': if(c3 == 'd')  { a_fpunary(iname+3, 0x3); ok = 1; }
          break;
        case 'p': if(c3 == 'w')  { a_fpbinry(iname+3, 0x7); ok = 1; }
          break;
        case 's': if(c3 == 'b')  { a_datapro(iname+3, 0x3); ok = 1; }
                  if(c3 == 'c')  { a_datapro(iname+3, 0x7); ok = 1; }
                  if(c3 == 'f')  { a_fpbinry(iname+3, 0x3); ok = 1; }
          break;
      }
      break;
    case 's':	/* str stm swi stf sqt sin suf swp stc sbc set */
      switch(c2) {
        case 'b': if(c3 == 'c')  { a_datapro(iname+3, 0x6); ok = 1; }
          break;
        case 'e': if(c3 == 't')  { a_datapro(iname+3, 0xff); ok = 1; }
        case 'i': if(c3 == 'n')  { a_fpunary(iname+3, 0x8); ok = 1; }
          break;
        case 'q': if(c3 == 't')  { a_fpunary(iname+3, 0x4); ok = 1; }
          break;
        case 't': if(c3 == 'r')  { a_dtrans(iname+3, 0); ok = 1; }
                  if(c3 == 'm')  { a_mdtrans(iname+3, 0); ok = 1; }
                  if(c3 == 'f')  { a_fpdtran(iname+3, 0); ok = 1; }
                  if(c3 == 'c')  { a_cpdtran(iname+3, 0); ok = 1; }
          break;
        case 'u': if(c3 == 'b')  { a_datapro(iname+3, 0x2); ok = 1; }
                  if(c3 == 'f')  { a_fpbinry(iname+3, 0x2); ok = 1; }
          break;
        case 'w': if(c3 == 'i')  { a_swi(iname+3); ok = 1; }
                  if(c3 == 'p')  { a_cpswp(iname+3); ok = 1; }
          break;
      }
      break;
    case 't':	/* tst teq tan */
      switch(c2) {
        case 'a': if(c3 == 'n')  { a_fpunary(iname+3, 0xa); ok = 1; }
          break;
        case 'e': if(c3 == 'q')  { a_datapro(iname+3, 0x9); ok = 1; }
          break;
        case 's': if(c3 == 't')  { a_datapro(iname+3, 0x8); ok = 1; }
          break;
      }
      break;
    case 'u':	/* urd */
      if(c2 == 'r' && c3 == 'd') { a_fpunary(iname+3, 0xe); ok = 1; }
      break;
    case 'w':	/* wps wfc */
      switch(c2) {
        case 'f': if(c3 == 'c')  { a_fpregtr(iname+3, 0x4); ok = 1; }
                  if(c3 == 's')  { a_fpregtr(iname+3, 0x2); ok = 1; }
          break;
      }
      break;
    case 'x':	/* xor */
      if(c2 == 'o' && c3 == 'r') { a_datapro(iname+3, 0x1); ok = 1; } break;
  }
  if(!ok)
    error("Unknown instruction `%s'", iname);
}
