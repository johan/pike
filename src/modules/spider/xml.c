#include "global.h"

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif /* HAVE_CONFIG_H */

#include "pike_macros.h"
#include "stralloc.h"
#include "object.h"
#include "interpret.h"
#include "mapping.h"
#include "program.h"
#include "array.h"
#include "builtin_functions.h"
#include "module_support.h"
#include "operators.h"
#include "error.h"
#include "gc.h"
#include "opcodes.h"

/* #define VERBOSE_XMLDEBUG */

struct xmldata
{
  PCHARP datap;
  INT32 len;
  INT32 pos;
  struct svalue *func;
  struct svalue *extra_args;
  INT32 num_extra_args;
  TYPE_FIELD extra_arg_types;
};

struct xmlobj
{
  struct mapping *entities;
  struct mapping *attributes;
  struct mapping *is_cdata;
};

#undef THIS
#define THIS ((struct xmlobj *)(fp->current_storage))


static int isBaseChar(INT32 c)
{
  switch(c>>8)
  {
    case 0x00:
      if(c>=0x0041 && c<=0x005A) return 1;
      if(c>=0x0061 && c<=0x007A) return 1;
      if(c>=0x00C0 && c<=0x00D6) return 1;
      if(c>=0x00D8 && c<=0x00F6) return 1;
      if(c>=0x00F8 && c<=0x00FF) return 1;
      break;

    case 0x01:
      if(c>=0x0100 && c<=0x0131) return 1;
      if(c>=0x0134 && c<=0x013E) return 1;
      if(c>=0x0141 && c<=0x0148) return 1;
      if(c>=0x014A && c<=0x017E) return 1;
      if(c>=0x0180 && c<=0x01C3) return 1;
      if(c>=0x01CD && c<=0x01F0) return 1;
      if(c>=0x01F4 && c<=0x01F5) return 1;
      if(c>=0x01FA && c<=0x0217) return 1;
      break;

    case 0x002:
      if(c>=0x0250 && c<=0x02A8) return 1;
      if(c>=0x02BB && c<=0x02C1) return 1;
      break;

    case 0x03:
      if(c==0x0386) return 1;
      if(c>=0x0388 && c<=0x038A) return 1;
      if(c==0x038C) return 1;
      if(c>=0x038E && c<=0x03A1) return 1;
      if(c>=0x03A3 && c<=0x03CE) return 1;
      if(c>=0x03D0 && c<=0x03D6) return 1;
      if(c==0x03DA) return 1;
      if(c==0x03DC) return 1;
      if(c==0x03DE) return 1;
      if(c==0x03E0) return 1;
      if(c>=0x03E2 && c<=0x03F3) return 1;
      break;

    case 0x04:
      if(c>=0x0401 && c<=0x040C) return 1;
      if(c>=0x040E && c<=0x044F) return 1;
      if(c>=0x0451 && c<=0x045C) return 1;
      if(c>=0x045E && c<=0x0481) return 1;
      if(c>=0x0490 && c<=0x04C4) return 1;
      if(c>=0x04C7 && c<=0x04C8) return 1;
      if(c>=0x04CB && c<=0x04CC) return 1;
      if(c>=0x04D0 && c<=0x04EB) return 1;
      if(c>=0x04EE && c<=0x04F5) return 1;
      if(c>=0x04F8 && c<=0x04F9) return 1;
      break;

    case 0x05:
      if(c>=0x0531 && c<=0x0556) return 1;
      if(c==0x0559) return 1;
      if(c>=0x0561 && c<=0x0586) return 1;
      if(c>=0x05D0 && c<=0x05EA) return 1;
      if(c>=0x05F0 && c<=0x05F2) return 1;
      break;

    case 0x06:
      if(c>=0x0621 && c<=0x063A) return 1;
      if(c>=0x0641 && c<=0x064A) return 1;
      if(c>=0x0671 && c<=0x06B7) return 1;
      if(c>=0x06BA && c<=0x06BE) return 1;
      if(c>=0x06C0 && c<=0x06CE) return 1;
      if(c>=0x06D0 && c<=0x06D3) return 1;
      if(c==0x06D5) return 1;
      if(c>=0x06E5 && c<=0x06E6) return 1;
      break;

    case 0x09:
      if(c>=0x0905 && c<=0x0939) return 1;
      if(c==0x093D) return 1;
      if(c>=0x0958 && c<=0x0961) return 1;
      if(c>=0x0985 && c<=0x098C) return 1;
      if(c>=0x098F && c<=0x0990) return 1;
      if(c>=0x0993 && c<=0x09A8) return 1;
      if(c>=0x09AA && c<=0x09B0) return 1;
      if(c==0x09B2) return 1;
      if(c>=0x09B6 && c<=0x09B9) return 1;
      if(c>=0x09DC && c<=0x09DD) return 1;
      if(c>=0x09DF && c<=0x09E1) return 1;
      if(c>=0x09F0 && c<=0x09F1) return 1;
      break;

    case 0x0a:
      if(c>=0x0A05 && c<=0x0A0A) return 1;
      if(c>=0x0A0F && c<=0x0A10) return 1;
      if(c>=0x0A13 && c<=0x0A28) return 1;
      if(c>=0x0A2A && c<=0x0A30) return 1;
      if(c>=0x0A32 && c<=0x0A33) return 1;
      if(c>=0x0A35 && c<=0x0A36) return 1;
      if(c>=0x0A38 && c<=0x0A39) return 1;
      if(c>=0x0A59 && c<=0x0A5C) return 1;
      if(c==0x0A5E) return 1;
      if(c>=0x0A72 && c<=0x0A74) return 1;
      if(c>=0x0A85 && c<=0x0A8B) return 1;
      if(c==0x0A8D) return 1;
      if(c>=0x0A8F && c<=0x0A91) return 1;
      if(c>=0x0A93 && c<=0x0AA8) return 1;
      if(c>=0x0AAA && c<=0x0AB0) return 1;
      if(c>=0x0AB2 && c<=0x0AB3) return 1;
      if(c>=0x0AB5 && c<=0x0AB9) return 1;
      if(c==0x0ABD) return 1;
      if(c==0x0AE0) return 1;
      break;

    case 0x0b:
      if(c>=0x0B05 && c<=0x0B0C) return 1;
      if(c>=0x0B0F && c<=0x0B10) return 1;
      if(c>=0x0B13 && c<=0x0B28) return 1;
      if(c>=0x0B2A && c<=0x0B30) return 1;
      if(c>=0x0B32 && c<=0x0B33) return 1;
      if(c>=0x0B36 && c<=0x0B39) return 1;
      if(c==0x0B3D) return 1;
      if(c>=0x0B5C && c<=0x0B5D) return 1;
      if(c>=0x0B5F && c<=0x0B61) return 1;
      if(c>=0x0B85 && c<=0x0B8A) return 1;
      if(c>=0x0B8E && c<=0x0B90) return 1;
      if(c>=0x0B92 && c<=0x0B95) return 1;
      if(c>=0x0B99 && c<=0x0B9A) return 1;
      if(c==0x0B9C) return 1;
      if(c>=0x0B9E && c<=0x0B9F) return 1;
      if(c>=0x0BA3 && c<=0x0BA4) return 1;
      if(c>=0x0BA8 && c<=0x0BAA) return 1;
      if(c>=0x0BAE && c<=0x0BB5) return 1;
      if(c>=0x0BB7 && c<=0x0BB9) return 1;
      break;

    case 0x0c:
      if(c>=0x0C05 && c<=0x0C0C) return 1;
      if(c>=0x0C0E && c<=0x0C10) return 1;
      if(c>=0x0C12 && c<=0x0C28) return 1;
      if(c>=0x0C2A && c<=0x0C33) return 1;
      if(c>=0x0C35 && c<=0x0C39) return 1;
      if(c>=0x0C60 && c<=0x0C61) return 1;
      if(c>=0x0C85 && c<=0x0C8C) return 1;
      if(c>=0x0C8E && c<=0x0C90) return 1;
      if(c>=0x0C92 && c<=0x0CA8) return 1;
      if(c>=0x0CAA && c<=0x0CB3) return 1;
      if(c>=0x0CB5 && c<=0x0CB9) return 1;
      if(c==0x0CDE) return 1;
      if(c>=0x0CE0 && c<=0x0CE1) return 1;
      break;

    case 0x0d:
      if(c>=0x0D05 && c<=0x0D0C) return 1;
      if(c>=0x0D0E && c<=0x0D10) return 1;
      if(c>=0x0D12 && c<=0x0D28) return 1;
      if(c>=0x0D2A && c<=0x0D39) return 1;
      if(c>=0x0D60 && c<=0x0D61) return 1;
      break;

    case 0x0e:
      if(c>=0x0E01 && c<=0x0E2E) return 1;
      if(c==0x0E30) return 1;
      if(c>=0x0E32 && c<=0x0E33) return 1;
      if(c>=0x0E40 && c<=0x0E45) return 1;
      if(c>=0x0E81 && c<=0x0E82) return 1;
      if(c==0x0E84) return 1;
      if(c>=0x0E87 && c<=0x0E88) return 1;
      if(c==0x0E8A) return 1;
      if(c==0x0E8D) return 1;
      if(c>=0x0E94 && c<=0x0E97) return 1;
      if(c>=0x0E99 && c<=0x0E9F) return 1;
      if(c>=0x0EA1 && c<=0x0EA3) return 1;
      if(c==0x0EA5) return 1;
      if(c==0x0EA7) return 1;
      if(c>=0x0EAA && c<=0x0EAB) return 1;
      if(c>=0x0EAD && c<=0x0EAE) return 1;
      if(c==0x0EB0) return 1;
      if(c>=0x0EB2 && c<=0x0EB3) return 1;
      if(c==0x0EBD) return 1;
      if(c>=0x0EC0 && c<=0x0EC4) return 1;
      break;

    case 0x0f:
      if(c>=0x0F40 && c<=0x0F47) return 1;
      if(c>=0x0F49 && c<=0x0F69) return 1;
      break;

    case 0x10:
      if(c>=0x10A0 && c<=0x10C5) return 1;
      if(c>=0x10D0 && c<=0x10F6) return 1;
      break;

    case 0x11:
      if(c==0x1100) return 1;
      if(c>=0x1102 && c<=0x1103) return 1;
      if(c>=0x1105 && c<=0x1107) return 1;
      if(c==0x1109) return 1;
      if(c>=0x110B && c<=0x110C) return 1;
      if(c>=0x110E && c<=0x1112) return 1;
      if(c==0x113C) return 1;
      if(c==0x113E) return 1;
      if(c==0x1140) return 1;
      if(c==0x114C) return 1;
      if(c==0x114E) return 1;
      if(c==0x1150) return 1;
      if(c>=0x1154 && c<=0x1155) return 1;
      if(c==0x1159) return 1;
      if(c>=0x115F && c<=0x1161) return 1;
      if(c==0x1163) return 1;
      if(c==0x1165) return 1;
      if(c==0x1167) return 1;
      if(c==0x1169) return 1;
      if(c>=0x116D && c<=0x116E) return 1;
      if(c>=0x1172 && c<=0x1173) return 1;
      if(c==0x1175) return 1;
      if(c==0x119E) return 1;
      if(c==0x11A8) return 1;
      if(c==0x11AB) return 1;
      if(c>=0x11AE && c<=0x11AF) return 1;
      if(c>=0x11B7 && c<=0x11B8) return 1;
      if(c==0x11BA) return 1;
      if(c>=0x11BC && c<=0x11C2) return 1;
      if(c==0x11EB) return 1;
      if(c==0x11F0) return 1;
      if(c==0x11F9) return 1;
      break;

    case 0x1e:
      if(c>=0x1E00 && c<=0x1E9B) return 1;
      if(c>=0x1EA0 && c<=0x1EF9) return 1;
      break;

    case 0x1f:
      if(c>=0x1F00 && c<=0x1F15) return 1;
      if(c>=0x1F18 && c<=0x1F1D) return 1;
      if(c>=0x1F20 && c<=0x1F45) return 1;
      if(c>=0x1F48 && c<=0x1F4D) return 1;
      if(c>=0x1F50 && c<=0x1F57) return 1;
      if(c==0x1F59) return 1;
      if(c==0x1F5B) return 1;
      if(c==0x1F5D) return 1;
      if(c>=0x1F5F && c<=0x1F7D) return 1;
      if(c>=0x1F80 && c<=0x1FB4) return 1;
      if(c>=0x1FB6 && c<=0x1FBC) return 1;
      if(c==0x1FBE) return 1;
      if(c>=0x1FC2 && c<=0x1FC4) return 1;
      if(c>=0x1FC6 && c<=0x1FCC) return 1;
      if(c>=0x1FD0 && c<=0x1FD3) return 1;
      if(c>=0x1FD6 && c<=0x1FDB) return 1;
      if(c>=0x1FE0 && c<=0x1FEC) return 1;
      if(c>=0x1FF2 && c<=0x1FF4) return 1;
      if(c>=0x1FF6 && c<=0x1FFC) return 1;
      break;

    case 0x21:
      if(c==0x2126) return 1;
      if(c>=0x212A && c<=0x212B) return 1;
      if(c==0x212E) return 1;
      if(c>=0x2180 && c<=0x2182) return 1;
      break;

    case 0x30:
      if(c>=0x3041 && c<=0x3094) return 1;
      if(c>=0x30A1 && c<=0x30FA) return 1;
      if(c>=0x3105 && c<=0x312C) return 1;
      break;

    default:
      if(c>=0xAC00 && c<=0xD7A3) return 1;
  }
  return 0;
}

static INLINE int isIdeographic(INT32 c)
{
  if(c>=0x4E00 && c<=0x9FA5) return 1;
  if(c==0x3007) return 1;
  if(c>=0x3021 && c<=0x3029) return 1;
  return 0;
}

static INLINE int isLetter(INT32 c)
{
  return isBaseChar(c) || isIdeographic(c);
}


static int isCombiningChar(INT32 c)
{
  switch(c>>8)
  {
    case 0x03:
      if(c>=0x0300 && c<=0x0345) return 1;
      if(c>=0x0360 && c<=0x0361) return 1;
      break;

    case 0x04:
      if(c>=0x0483 && c<=0x0486) return 1;
      break;

    case 0x05:
      if(c>=0x0591 && c<=0x05A1) return 1;
      if(c>=0x05A3 && c<=0x05B9) return 1;
      if(c>=0x05BB && c<=0x05BD) return 1;
      if(c==0x05BF) return 1;
      if(c>=0x05C1 && c<=0x05C2) return 1;
      if(c==0x05C4) return 1;
      break;

    case 0x06:
      if(c>=0x064B && c<=0x0652) return 1;
      if(c==0x0670) return 1;
      if(c>=0x06D6 && c<=0x06DC) return 1;
      if(c>=0x06DD && c<=0x06DF) return 1;
      if(c>=0x06E0 && c<=0x06E4) return 1;
      if(c>=0x06E7 && c<=0x06E8) return 1;
      if(c>=0x06EA && c<=0x06ED) return 1;
      break;

    case 0x09:
      if(c>=0x0901 && c<=0x0903) return 1;
      if(c==0x093C) return 1;
      if(c>=0x093E && c<=0x094C) return 1;
      if(c==0x094D) return 1;
      if(c>=0x0951 && c<=0x0954) return 1;
      if(c>=0x0962 && c<=0x0963) return 1;
      if(c>=0x0981 && c<=0x0983) return 1;
      if(c==0x09BC) return 1;
      if(c==0x09BE) return 1;
      if(c==0x09BF) return 1;
      if(c>=0x09C0 && c<=0x09C4) return 1;
      if(c>=0x09C7 && c<=0x09C8) return 1;
      if(c>=0x09CB && c<=0x09CD) return 1;
      if(c==0x09D7) return 1;
      if(c>=0x09E2 && c<=0x09E3) return 1;
      break;

    case 0x0a:
      if(c==0x0A02) return 1;
      if(c==0x0A3C) return 1;
      if(c==0x0A3E) return 1;
      if(c==0x0A3F) return 1;
      if(c>=0x0A40 && c<=0x0A42) return 1;
      if(c>=0x0A47 && c<=0x0A48) return 1;
      if(c>=0x0A4B && c<=0x0A4D) return 1;
      if(c>=0x0A70 && c<=0x0A71) return 1;
      if(c>=0x0A81 && c<=0x0A83) return 1;
      if(c==0x0ABC) return 1;
      if(c>=0x0ABE && c<=0x0AC5) return 1;
      if(c>=0x0AC7 && c<=0x0AC9) return 1;
      if(c>=0x0ACB && c<=0x0ACD) return 1;
      break;

    case 0x0b:
      if(c>=0x0B01 && c<=0x0B03) return 1;
      if(c==0x0B3C) return 1;
      if(c>=0x0B3E && c<=0x0B43) return 1;
      if(c>=0x0B47 && c<=0x0B48) return 1;
      if(c>=0x0B4B && c<=0x0B4D) return 1;
      if(c>=0x0B56 && c<=0x0B57) return 1;
      if(c>=0x0B82 && c<=0x0B83) return 1;
      if(c>=0x0BBE && c<=0x0BC2) return 1;
      if(c>=0x0BC6 && c<=0x0BC8) return 1;
      if(c>=0x0BCA && c<=0x0BCD) return 1;
      if(c==0x0BD7) return 1;
      break;

    case 0x0c:
      if(c>=0x0C01 && c<=0x0C03) return 1;
      if(c>=0x0C3E && c<=0x0C44) return 1;
      if(c>=0x0C46 && c<=0x0C48) return 1;
      if(c>=0x0C4A && c<=0x0C4D) return 1;
      if(c>=0x0C55 && c<=0x0C56) return 1;
      if(c>=0x0C82 && c<=0x0C83) return 1;
      if(c>=0x0CBE && c<=0x0CC4) return 1;
      if(c>=0x0CC6 && c<=0x0CC8) return 1;
      if(c>=0x0CCA && c<=0x0CCD) return 1;
      if(c>=0x0CD5 && c<=0x0CD6) return 1;
      break;

    case 0x0d:
      if(c>=0x0D02 && c<=0x0D03) return 1;
      if(c>=0x0D3E && c<=0x0D43) return 1;
      if(c>=0x0D46 && c<=0x0D48) return 1;
      if(c>=0x0D4A && c<=0x0D4D) return 1;
      if(c==0x0D57) return 1;
      break;

    case 0x0e:
      if(c==0x0E31) return 1;
      if(c>=0x0E34 && c<=0x0E3A) return 1;
      if(c>=0x0E47 && c<=0x0E4E) return 1;
      if(c==0x0EB1) return 1;
      if(c>=0x0EB4 && c<=0x0EB9) return 1;
      if(c>=0x0EBB && c<=0x0EBC) return 1;
      if(c>=0x0EC8 && c<=0x0ECD) return 1;
      break;

    case 0x0f:
      if(c>=0x0F18 && c<=0x0F19) return 1;
      if(c==0x0F35) return 1;
      if(c==0x0F37) return 1;
      if(c==0x0F39) return 1;
      if(c==0x0F3E) return 1;
      if(c==0x0F3F) return 1;
      if(c>=0x0F71 && c<=0x0F84) return 1;
      if(c>=0x0F86 && c<=0x0F8B) return 1;
      if(c>=0x0F90 && c<=0x0F95) return 1;
      if(c==0x0F97) return 1;
      if(c>=0x0F99 && c<=0x0FAD) return 1;
      if(c>=0x0FB1 && c<=0x0FB7) return 1;
      if(c==0x0FB9) return 1;
      break;

    case 0x20:
      if(c>=0x20D0 && c<=0x20DC) return 1;
      if(c==0x20E1) return 1;
      break;

    case 0x30:
      if(c>=0x302A && c<=0x302F) return 1;
      if(c==0x3099) return 1;
      if(c==0x309A) return 1;
  }
  return 0;
}

static INLINE int isDigit(INT32 c)
{
  switch(c>>8)
  {
    case 0x00:
      return c>=0x0030 && c<=0x0039;

    case 0x06:
      if(c>=0x0660 && c<=0x0669) return 1;
      if(c>=0x06F0 && c<=0x06F9) return 1;
      break;

    case 0x09:
      if(c>=0x0966 && c<=0x096F) return 1;
      if(c>=0x09E6 && c<=0x09EF) return 1;
      break;

    case 0x0a:
      if(c>=0x0A66 && c<=0x0A6F) return 1;
      if(c>=0x0AE6 && c<=0x0AEF) return 1;
      break;

    case 0x0b:
      if(c>=0x0B66 && c<=0x0B6F) return 1;
      if(c>=0x0BE7 && c<=0x0BEF) return 1;
      break;

    case 0x0c:
      if(c>=0x0C66 && c<=0x0C6F) return 1;
      if(c>=0x0CE6 && c<=0x0CEF) return 1;
      break;

    case 0x0d:
      if(c>=0x0D66 && c<=0x0D6F) return 1;
      break;

    case 0x0e:
      if(c>=0x0E50 && c<=0x0E59) return 1;
      if(c>=0x0ED0 && c<=0x0ED9) return 1;

    case 0x0f:
      if(c>=0x0F20 && c<=0x0F29) return 1;
  }
  return 0;
}

static int isExtender(INT32 c)
{
  switch(c)
  {
    case 0x00B7:
    case 0x02D0:
    case 0x02D1:
    case 0x0387:
    case 0x0640:
    case 0x0E46:
    case 0x0EC6:
    case 0x3005:

    case 0x3031:
    case 0x3032:
    case 0x3033:
    case 0x3034:
    case 0x3035:

    case 0x309D:
    case 0x309E:

    case 0x30FC:
    case 0x30FD:
    case 0x30FE:
      return 1;
  }
  return 0;
}

static int isChar(INT32 c)
{
 
  if(c <= 0xd7ff)
  {
    if(c>0x20 || c==0x9 || c==0x0a || c==0x0d) return 1;
  }else{
    /* FIXME: The test below is always false!
     * /grubba 1998-12-05
     * 
     * So? This function isn't used anyways...
     * /Hubbe 1998-12-16
     */
    if(c <=0xffd)
    {
      if(c>=0xe000) return 1;
    }else{
      if(c>=0x10000 && c<=0x10ffff) return 1;
    }
  }
  return 0;
}

static INLINE int isSpace(INT32 c)
{
  switch(c)
  {
    case 0x20:
    case 0x09:
    case 0x0d:
    case 0x0a:
      return 1;
  }
  return 0;
}

static INLINE int isNameChar(INT32 c)
{
  return isLetter(c) || isDigit(c) || 
    c=='.' || c=='-' || c=='_' ||  c==':' ||
    isCombiningChar(c) || isExtender(c);
}

static INLINE int isFirstNameChar(INT32 c)
{
  return isLetter(c) ||  c=='_' || c==':';
}

static INLINE int isHexChar(INT32 c)
{
  switch(c)
  {
    case '0': return 0;
    case '1': return 1;
    case '2': return 2;
    case '3': return 3;
    case '4': return 4;
    case '5': return 5;
    case '6': return 6;
    case '7': return 7;
    case '8': return 8;
    case '9': return 9;
    case 'a': case 'A': return 10;
    case 'b': case 'B': return 11;
    case 'c': case 'C': return 12;
    case 'd': case 'D': return 13;
    case 'e': case 'E': return 14;
    case 'f': case 'F': return 15;
    default: return -1;
  }
}


#define PEEK(X) INDEX_PCHARP(data->datap,X)

#ifdef VERBOSE_XMLDEBUG
#define POKE(X,Y) string_builder_putchar(&X,Y)
#define READ(Z) do { data->pos+=Z;  fprintf(stderr,"Stepping %d steps to %d (at %d)\n",Z,data->pos,__LINE__); data->len-=Z; INC_PCHARP(data->datap, Z); }while(0)
#define IF_XMLDEBUG(X) X
#else
#define POKE(X,Y) string_builder_putchar(&X,Y)
#define READ(Z) do { data->pos+=Z; data->len-=Z; INC_PCHARP(data->datap, Z); }while(0)
#define IF_XMLDEBUG(X)
#endif

#define SIMPLE_READ_ATTVALUE(X) simple_read_system_attvalue(data,X)
#define SIMPLE_READ_SYSTEMLITERAL() simple_read_system_literal(data)
#define SIMPLE_READ_PUBIDLITERAL() simple_read_pubid_literal(data)
#define SIMPLE_READNAME() simple_readname(data)
#define SIMPLE_READNMTOKEN() simple_readnmtoken(data)

static void simple_readname(struct xmldata *);
static void simple_read_system_literal(struct xmldata *);
static void simple_read_pubid_literal(struct xmldata *);
static int low_parse_xml(struct xmldata *data,
			 struct pike_string *end,
			 int toplevel);

#define XMLERROR(desc) do {			\
    struct svalue * save_sp=sp;			\
    push_text("error");				\
    push_int(0); /* no name */			\
    push_int(0); /* no attributes */            \
    push_text(desc);				\
    SYS();					\
    if(save_sp == sp)				\
      error("%s\n",desc);			\
    READ(1);					\
  }while(0)


#define SKIPSPACE() \
  do { while(isSpace(PEEK(0))) READ(1); }while(0)

static int gobble(struct xmldata *data, char *s)
{
  int e;
  for(e=0;s[e];e++)
  {
    if(((unsigned int)(s[e]))!=PEEK(e))
      return 0;
  }
  if(isNameChar(PEEK(e))) return 0;
  READ(e);
  return 1;
}

#define GOBBLE(X) gobble(data,X)


#define READNAME(X) do {				\
	if(isFirstNameChar(PEEK(0)))			\
	{						\
	  POKE(X, PEEK(0));				\
	  READ(1);					\
	}else{						\
	  XMLERROR("Name expected");			\
	}						\
	while(isNameChar(PEEK(0)))			\
	{						\
	  POKE(X, PEEK(0));				\
	  READ(1);					\
	}						\
      }while(0)

#define READNMTOKEN(X) do {				\
	if(isNameChar(PEEK(0)))				\
	{						\
	  POKE(X, PEEK(0));				\
	  READ(1);					\
	}else{						\
	  XMLERROR("Nametoken expected");			\
	}						\
	while(isNameChar(PEEK(0)))			\
	{						\
	  POKE(X, PEEK(0));				\
	  READ(1);					\
	}						\
      }while(0)


#define BEGIN_STRING(STR)			\
  do{						\
    struct string_builder STR;			\
    ONERROR tmp_ ## STR;				\
    init_string_builder(&STR,0);			\
    SET_ONERROR(tmp_ ## STR, free_string_builder, &STR)


#define END_STRING(STR)				\
     check_stack(1);                            \
     UNSET_ONERROR(tmp_ ## STR);		\
     push_string(finish_string_builder(&STR));	\
  }while(0)

#define STRIP_SPACES() do {			\
    IF_XMLDEBUG(fprintf(stderr,"STRIPPING SPACES (%s)\n",sp[-1].u.string->str));       \
    push_text("\n");				\
    push_text("\r");				\
    push_text("\t");				\
    f_aggregate(3);				\
    push_text(" ");				\
    push_text(" ");				\
    push_text(" ");				\
    f_aggregate(3);				\
    f_replace(3);				\
    push_text(" ");				\
    o_divide();					\
    push_text("");				\
    f_aggregate(1);                             \
    o_subtract();				\
    push_text(" ");				\
    o_multiply();				\
    IF_XMLDEBUG(fprintf(stderr,"SPACES STRIPPED (%s)\n",sp[-1].u.string->str));       \
  } while(0)


#define READ_CHAR_REF(X) do { \
	  /* Character reference */					 \
	  INT32 num=0;							 \
									 \
	  READ(1);							 \
	  if(PEEK(0)=='x')						 \
	  {								 \
	    READ(1);							\
	    while(isHexChar(PEEK(0))>=0)				 \
	    {								 \
	      num*=16;							 \
	      num+=isHexChar(PEEK(0));					 \
	      READ(1);							 \
	    }								 \
	  }else{							 \
	    while(PEEK(0)>='0' && PEEK(0)<='9')				 \
	    {								 \
	      num*=10;							 \
	      num+=PEEK(0)-'0';						 \
	      READ(1);							 \
	    }								 \
	  }								 \
	  if(PEEK(0)!=';')						 \
	    XMLERROR("Missing ';' after character reference.");		 \
	  READ(1);							 \
	  POKE(X, num);							 \
   }while(0)



#define READ_REFERENCE(X,PARSE_RECURSIVELY) do {			 \
	READ(1); /* Assume '&' for now */				 \
	if(PEEK(0)=='#')						 \
	{								 \
          READ_CHAR_REF(X);\
	}else{								 \
	  /* Entity reference */					 \
	  if(!THIS->entities)						 \
          {								 \
            XMLERROR("XML->__entities is not a mapping");			 \
	    f_aggregate_mapping(0);					 \
	  }else{							 \
	    ref_push_mapping(THIS->entities);				 \
	  }								 \
	  SIMPLE_READNAME();						 \
          IF_XMLDEBUG(fprintf(stderr,"Found entity: %s\n",sp[-1].u.string->str)); \
	  if(PEEK(0)!=';')						 \
	    XMLERROR("Missing ';' after entity reference.");		 \
	  READ(1);							 \
	  /* lookup entry in mapping and parse it recursively */	 \
	  /* Generate error if entity is not defined */			 \
	  f_index(2);							 \
          if(IS_ZERO(sp-1))						 \
	  {								 \
	    XMLERROR("No such entity.");					 \
	    pop_stack();						 \
	  }else{							 \
	    if(sp[-1].type!=T_STRING)					 \
	    {								 \
	      XMLERROR("XML->__entities value is not a string!");		 \
	    }else{							 \
	      struct pike_string *s=sp[-1].u.string;			 \
	      struct xmldata my_tmp=*data;			      	 \
	      ONERROR tmp2;						 \
	      sp--;							 \
	      SET_ONERROR(tmp2, do_free_string, s);			 \
	      check_stack(10);						 \
	      my_tmp.datap=MKPCHARP_STR(s);			     	 \
	      my_tmp.len=s->len;				       	 \
	      PARSE_RECURSIVELY;					 \
	      UNSET_ONERROR(tmp2);					 \
	      free_string(s);						 \
	    }								 \
	  }								 \
	}								 \
        IF_XMLDEBUG(fprintf(stderr,"Read reference at %d done.\n",__LINE__)); \
      }while(0)



#define READ_PEREFERENCE(X,PARSE_RECURSIVELY) do {			 \
	READ(1); /* Assume '%'  */					 \
	  if(!THIS->entities)						 \
          {								 \
            XMLERROR("XML->__entities is not a mapping");			 \
	    f_aggregate_mapping(0);					 \
	  }else{							 \
	    ref_push_mapping(THIS->entities);				 \
	  }								 \
	  push_text("%");                                                \
	  SIMPLE_READNAME();						 \
          f_add(2);                                                      \
	  if(PEEK(0)!=';')						 \
	    XMLERROR("Missing ';' after entity reference.");		 \
	  READ(1);							 \
	  /* lookup entry in mapping and parse it recursively */	 \
	  /* Generate error if entity is not defined */			 \
	  f_index(2);							 \
          if(IS_ZERO(sp-1))						 \
	  {								 \
	    XMLERROR("No such entity.");					 \
	    pop_stack();						 \
	  }else{							 \
	    if(sp[-1].type!=T_STRING)					 \
	    {								 \
	      XMLERROR("XML->__entities value is not a string!");		 \
	    }else{							 \
	      struct pike_string *s=sp[-1].u.string;			 \
	      struct xmldata my_tmp=*data;			      	 \
	      ONERROR tmp2;						 \
	      sp--;							 \
	      SET_ONERROR(tmp2, do_free_string, s);			 \
	      check_stack(10);						 \
	      my_tmp.datap=MKPCHARP_STR(s);			     	 \
	      my_tmp.len=s->len;				       	 \
	      PARSE_RECURSIVELY;					 \
	      UNSET_ONERROR(tmp2);					 \
	      free_string(s);						 \
	    }								 \
	}								 \
      }while(0)




#define READ_ATTVALUE(X) do {		\
        SKIPSPACE();				\
        switch(PEEK(0))				\
        {					\
          case '\'':				\
            READ(1);				\
            read_attvalue(data,&X,'\'',0);	\
            break;				\
          case '\"':				\
            READ(1);				\
            read_attvalue(data,&X,'\"',0);	\
            break;				\
          default:				\
            XMLERROR("Unquoted attribute value.");	\
            push_text("");                      \
        }					\
      }while(0)

#define READ_PUBID(X) do {		\
        SKIPSPACE();				\
        switch(PEEK(0))				\
        {					\
          case '\'':				\
            READ(1);				\
            read_pubid(data,&X,'\'');	\
            break;				\
          case '\"':				\
            READ(1);				\
            read_pubid(data,&X,'\"');	\
            break;				\
          default:				\
            XMLERROR("Unquoted public id.");	\
        }					\
      }while(0)

#define READ_ATTVALUE2(X) do {			\
        SKIPSPACE();				\
        switch(PEEK(0))				\
        {					\
          case '\'':				\
            READ(1);				\
            read_attvalue2(data,&X,'\'');	\
            break;				\
          case '\"':				\
            READ(1);				\
            read_attvalue2(data,&X,'\"');	\
            break;				\
          default:				\
            XMLERROR("Unquoted attribute value.");	\
        }					\
      }while(0)


#define READ_COMMENT() do { BEGIN_STRING(com);			\
    while(!(PEEK(0)=='-' && PEEK(1)=='-' && PEEK(2)=='>'))	\
    {								\
      POKE(com, PEEK(0));					\
      READ(1);							\
    }								\
    READ(3);	END_STRING(com);				\
      }while(0);


#define INTERMISSION(X) do {			\
   if((X).s->len) {                             \
     check_stack(4);				\
     push_text("");				\
     push_int(0); /* No name */			\
     push_int(0); /* No attributes */		\
     push_string(finish_string_builder(&(X)));	\
     init_string_builder(&(X),0);		\
     SYS();					\
    }  } while (0)


#define SYS() do{						\
	check_stack(1+data->num_extra_args);			\
        push_text("location");                     \
        push_int(data->pos);                       \
        f_aggregate_mapping(2);                    \
        assign_svalues_no_free(sp, data->extra_args,		\
			       data->num_extra_args,		\
			       data->extra_arg_types);		\
        sp+=data->num_extra_args;				\
	apply_svalue(data->func, 5+data->num_extra_args);	\
	if(IS_ZERO(sp-1))					\
	  pop_stack();						\
      }while(0);

static void read_attvalue(struct xmldata *data,
			  struct string_builder *X,
			  p_wchar2 Y,
			  int keepspace)
{
  while(1)
  {
    if(data->len<=0)
    {
      if(Y)
	XMLERROR("End of file while looking for end of attribute value.");
      break;
    }
    if(PEEK(0)==Y)
    {
      READ(1);
      break;
    }
    switch(PEEK(0))
    {
      case '&':
	READ_REFERENCE((*X), read_attvalue(&my_tmp, X, 0,1));
	break;
	
      case 0x0d:
	if(keepspace)
	{
	  POKE(*X, ' ');
	  READ(1);
	  break;
	}
	if(PEEK(1)==0x0a) READ(1);
      case 0x20:
      case 0x0a:
      case 0x09:
	READ(1);
	POKE(*X, 0x20);
	break;
	
      default:
	POKE(*X, PEEK(0));
	READ(1);
    }
  }
}

static void read_pubid(struct xmldata *data,
			  struct string_builder *X,
			  p_wchar2 Y)
{
  while(1)
  {
    if(data->len<=0)
    {
      if(Y)
	XMLERROR("End of file while looking for end of attribute value.");
      break;
    }
    if(PEEK(0)==Y)
    {
      READ(1);
      break;
    }
    switch(PEEK(0))
    {
      case 0x0d: if(PEEK(1)==0x0a) READ(1);
      case 0x20:
      case 0x0a:
      case 0x09:
	READ(1);
	POKE(*X, 0x20);
	break;
	
      default:
	POKE(*X, PEEK(0));
	READ(1);
    }
  }
}

static void read_attvalue2(struct xmldata *data,
			   struct string_builder *X,
			   p_wchar2 Y)
{
  while(1)
  {
    if(data->len<=0)
    {
      if(Y)
	XMLERROR("End of file while looking for end of attribute value.");
      break;
    }
    if(PEEK(0)==Y)
    {
      READ(1);
      break;
    }
    switch(PEEK(0))
    {
	

      case '%':
	READ_PEREFERENCE((*X), read_attvalue2(&my_tmp, X, 0));
	break;
	
      case 0x0d: if(PEEK(1)==0x0a) READ(1);
      case 0x20:
      case 0x0a:
      case 0x09:
	POKE(*X, 0x20);
	READ(1);
	break;
	
#if 0
      case '&':
	READ_REFERENCE((*X), read_attvalue2(&my_tmp, X, 0));
	break;
#else
      case '&':
	if(PEEK(1)=='#')
	{
	  READ(1);
	  READ_CHAR_REF((*X));
	  break;
	}
#endif

      default:
	POKE(*X, PEEK(0));
	READ(1);
    }
  }
}

static void simple_read_system_literal(struct xmldata *data)
{
  BEGIN_STRING(name);
  SKIPSPACE();
  READ_ATTVALUE(name);
  END_STRING(name);
}

static void simple_read_pubid_literal(struct xmldata *data)
{
  BEGIN_STRING(name);
  SKIPSPACE();
  READ_PUBID(name);
  END_STRING(name);
}

static void simple_read_system_attvalue(struct xmldata *data, int cdata)
{
  BEGIN_STRING(name);
  SKIPSPACE();
  READ_ATTVALUE(name);
  END_STRING(name);
  if(!cdata)  STRIP_SPACES();
}


static void simple_readname(struct xmldata *data)
{
  BEGIN_STRING(name);
  READNAME(name);
  END_STRING(name);
}

static void simple_readnmtoken(struct xmldata *data)
{
  BEGIN_STRING(name);
  READNMTOKEN(name);
  END_STRING(name);
}

#define SIMPLE_READ_ATTRIBUTES(CD) simple_read_attributes(data,CD);

static void simple_read_attributes(struct xmldata *data,
				   struct mapping *cdata)
{
  SKIPSPACE();
  
  /* Read unordered attributes */
  while(isFirstNameChar(PEEK(0)))
  {
    int iscd;
    SIMPLE_READNAME();
    SKIPSPACE();
    if(PEEK(0)!='=')
      XMLERROR("Missing '=' in attribute.");
    READ(1);

    iscd=1;
    if(cdata)
    {
      struct svalue *s=low_mapping_lookup(cdata,sp-1);
      if(s && IS_ZERO(s))
	iscd=0;
    }

    SIMPLE_READ_ATTVALUE(iscd);

#ifdef VERBOSE_XMLDEBUG
    fprintf(stderr,"Attribute %s = %s\n",sp[-2].u.string->str, sp[-1].u.string->str);
#endif

    assign_lvalue(sp-3, sp-1);
    pop_n_elems(2);
    SKIPSPACE();
  }
}

static int really_low_parse_dtd(struct xmldata *data)
{
  int done=0;
#ifdef PIKE_DEBUG
    struct svalue *save_sp=sp;
#endif

  while(!done && data->len>0)
  {
#ifdef PIKE_DEBUG
    if(sp<save_sp)
      fatal("Stack underflow.\n");
#endif
    switch(PEEK(0))
    {
      default:
	if(!isSpace(PEEK(0)))
	{
#ifdef VERBOSE_XMLDEBUG
	  fprintf(stderr,"Non-space character on DTD top level: %c.",PEEK(0));
#endif
	  XMLERROR("Non-space character on DTD top level.");
	  while(data->len>0 && PEEK(0)!='<')
	    READ(1);
	  break;
	}
	READ(1);
	SKIPSPACE();
	break;

      case '%': /* PEReference */
	READ_PEREFERENCE(guggel, really_low_parse_dtd(&my_tmp));
	break;

      case '<':
	switch(PEEK(1))
	{
	  case '!':
	    switch(PEEK(2))
	    {
	      case '-': /* Comment */
		if(PEEK(3)=='-')
		{
		  /* Comment */
		  push_text("<!--");
		  push_int(0); /* No name */
		  push_int(0); /* No attribues */
		  READ(4);
		  READ_COMMENT();
		  SYS();
		}else{
		  XMLERROR("Expected <!-- but got something else.");
		}
		break;

	      case 'E': /* ELEMENT or ENTITY */
		if(PEEK(3)=='N' &&
		   PEEK(4)=='T' &&
		   PEEK(5)=='I' &&
		   PEEK(6)=='T' &&
		   PEEK(7)=='Y' &&
		   isSpace(PEEK(8)))
		{
		  int may_have_ndata=0;
		  int attributes=0;
		  READ(9);
		  SKIPSPACE();

		  push_text("<!ENTITY");

		  if(PEEK(0)=='%')
		  {
		    READ(1);
		    SKIPSPACE();
		    push_text("%");
		    SIMPLE_READNAME();
		    f_add(2);
		  }else{
		    may_have_ndata=1;
		    SIMPLE_READNAME();
		  }

		  SKIPSPACE();

		  switch(PEEK(0))
		  {
		    case '\'':
		    case '"':
		      push_int(0); /* no attributes */
		      BEGIN_STRING(value);
		      READ_ATTVALUE2(value);
		      END_STRING(value);
		      if(THIS->entities)
		      {
			if(low_mapping_string_lookup(THIS->entities,
						     sp[-3].u.string))
			{
			  /* Duplicate entry, we should issue a warning */
			}else{
			  mapping_string_insert(THIS->entities,
						sp[-3].u.string,
						sp-1);
			}
		      }else{
			XMLERROR("XML->__entities is not a mapping.");
		      }
		      SYS();
		      break;

		    case 'S': /* SYSTEM */
		      if(PEEK(1)=='Y' &&
			 PEEK(2)=='S' &&
			 PEEK(3)=='T' &&
			 PEEK(4)=='E' &&
			 PEEK(5)=='M' &&
			 isSpace(PEEK(6)))
		      {
			READ(7);
			SKIPSPACE();
			push_text("SYSTEM");
			SIMPLE_READ_SYSTEMLITERAL();
			attributes++;
			goto check_ndata;
		      }
		      goto not_system;
			 
		    case 'P': /* PUBLIC */
		      if(PEEK(1)=='U' &&
			 PEEK(2)=='B' &&
			 PEEK(3)=='L' &&
			 PEEK(4)=='I' &&
			 PEEK(5)=='C' &&
			 isSpace(PEEK(6)))
		      {
			READ(7);
			attributes++;
			push_text("PUBLIC");
			SKIPSPACE();
			SIMPLE_READ_PUBIDLITERAL();
			SKIPSPACE();
			attributes++;
			push_text("SYSTEM");
			SIMPLE_READ_SYSTEMLITERAL();
			

		      check_ndata:
			SKIPSPACE();
			if(GOBBLE("NDATA"))
			{
			  if(!may_have_ndata)
			  {
			    XMLERROR("This entity is not allowed to have an NDATA keyword.");
			  }
			  
			  attributes++;
			  push_text("NDATA");
			  SKIPSPACE();
			  SIMPLE_READNAME();
			  SKIPSPACE();
			}
			f_aggregate_mapping(attributes*2);
			push_int(0); /* no data */
			SYS();
			break;
		      }

		    default:
		  not_system:
		      /* FIXME, DTD's are IGNORED! */
		      XMLERROR("Unexpected data in <!ENTITY");
		      while(data->len>0 && PEEK(0)!='>')
			READ(1);
		  }
		  
		  SKIPSPACE();
		  if(PEEK(0)!='>')
		    XMLERROR("Missing '>' in ENTITY.");
		  READ(1);
		  break;

		}
		if(PEEK(3)=='L' &&
		   PEEK(4)=='E' &&
		   PEEK(5)=='M' &&
		   PEEK(6)=='E' &&
		   PEEK(7)=='N' &&
		   PEEK(8)=='T' &&
		   isSpace(PEEK(9)))
		{
		  goto ignored_dtd_entry;
		}

		goto unknown_entry_in_dtd;

	      case 'A': /* ATTLIST */
		if(PEEK(3)=='T' &&
		   PEEK(4)=='T' &&
		   PEEK(5)=='L' &&
		   PEEK(6)=='I' &&
		   PEEK(7)=='S' &&
		   PEEK(8)=='T' &&
		   isSpace(PEEK(9)))
		{
		  READ(10);
		  SKIPSPACE();
		  push_text("<!ATTLIST");
		  SIMPLE_READNAME();

		  push_mapping(allocate_mapping(10)); /* Attributes */
		  while(1)
		  {
		    int cdata=1;
		    struct svalue *save;
		    if(data->len<=0)
		    {
		      XMLERROR("End of file while parsing ATTLIST.");
		      break;
		    }
		    SKIPSPACE();
		    if(PEEK(0)=='>')
		    {
		      READ(1);
		      break;
		    }

		    SIMPLE_READNAME();
		    SKIPSPACE();

		    save=sp;
		    switch(PEEK(0))
		    {
		      case 'C': /* CDATA */
		      case 'I': /* ID, IDREF or IDREFS */
		      case 'E': /* ENTITY or ENTITIES */
		      case 'N': /* NOTATION, NMTOKEN or NMTOKENS */
			SIMPLE_READNAME();

			/* Update is_ctype */
			{
			  struct svalue sval;
			  sval.type=T_INT;
			  sval.subtype=NUMBER_NUMBER;
			  if(!strcmp(sp[-1].u.string->str,"CDATA"))
			  {
			    IF_XMLDEBUG(fprintf(stderr,"IS CDATA\n"));
			    cdata=sval.u.integer=1;
			  }else{
			    IF_XMLDEBUG(fprintf(stderr,"IS NOT CDATA\n"));
			    cdata=sval.u.integer=0;
			  }

			  if(THIS->is_cdata)
			  {
			    /* Stack is:
			     * -4 tagname 
			     * -3 attmapping
			     * -2 attname
			     * -1 type
			     */
			    struct svalue *s; 
			    struct mapping *tmp;
			    s=low_mapping_string_lookup(THIS->is_cdata,
							sp[-4].u.string);
			    if(!s || s->type!=T_MAPPING)
			    {
			      f_aggregate_mapping(0);
			      tmp=sp[-1].u.mapping;
			      
#ifdef VERBOSE_XMLDEBUG
			      fprintf(stderr,"Creating CDATA mapping for %s\n",
				      sp[-5].u.string->str);
#endif
			      mapping_string_insert(THIS->is_cdata,
						    sp[-5].u.string,
						    sp-1);
			      pop_stack();
			    }else{
			      tmp=s->u.mapping;
			    }
			    s=low_mapping_string_lookup(tmp,
							sp[-2].u.string);
			    if(!s)
			    {
#ifdef VERBOSE_XMLDEBUG			    
			      
			      fprintf(stderr,
				      "Inserting CDATA for %s %s = %ld\n",
				      sp[-4].u.string->str,
				      sp[-2].u.string->str,
				      (long)sval.u.integer);
#endif
			      mapping_string_insert(tmp,
						    sp[-2].u.string,
						    &sval);
			    }
			  }
			}


			if(!strcmp(sp[-1].u.string->str,"NOTATION"))
			{
			  SKIPSPACE();
			  if(PEEK(0)!='(')
			    XMLERROR("Expected '(' after NOTATION.");
			  READ(1);

			  SIMPLE_READNAME();
			  SKIPSPACE();
			  while(PEEK(0)=='|')
			  {
			    READ(1);
			    SIMPLE_READNAME();
			    SKIPSPACE();
			    check_stack(1);
			  }
			  if(PEEK(0)!=')')
			    XMLERROR("Expected ')' after NOTATION enumeration.");
			  READ(1);
			}
			break;

		      case '(': /* Enumeration */
			push_text("");
			READ(1);
			SKIPSPACE();
			
			SIMPLE_READNMTOKEN();
			SKIPSPACE();
			while(PEEK(0)=='|')
			{
			  READ(1);
			  SIMPLE_READNMTOKEN();
			  SKIPSPACE();
			  check_stack(1);
			}
			if(PEEK(0)!=')')
			  XMLERROR("Expected ')' after enumeration.");
			READ(1);
			break;
		    }
#ifdef PIKE_DEBUG
		    if(sp<save)
		      fatal("Stack underflow.\n");
#endif
		    f_aggregate(sp-save);
		    SKIPSPACE();
		    save=sp;
		    switch(PEEK(0))
		    {
		      case '#':
			switch(PEEK(1))
			{
			  case 'R':
			    if(PEEK(2)=='E' && 
			       PEEK(3)=='Q' && 
			       PEEK(4)=='U' && 
			       PEEK(5)=='I' && 
			       PEEK(6)=='R' && 
			       PEEK(7)=='E' && 
			       PEEK(8)=='D')
			    {
			      READ(9);
			      push_text("#REQUIRED");
			      break;
			    }
			    goto bad_defaultdecl;
			       
			  case 'I':
			    if(PEEK(2)=='M' && 
			       PEEK(3)=='P' && 
			       PEEK(4)=='L' && 
			       PEEK(5)=='I' && 
			       PEEK(6)=='E' && 
			       PEEK(7)=='D')
			    {
			      READ(8);
			      push_text("#IMPLIED");
			      break;
			    }
			    goto bad_defaultdecl;

			  case 'F':
			    if(PEEK(2)=='I' && 
			       PEEK(3)=='X' && 
			       PEEK(4)=='E' && 
			       PEEK(5)=='D')
			    {
			      READ(6);
			      push_text("#FIXED");
			      SKIPSPACE();
			      goto comefrom_fixed;
			    }

			  default:
			bad_defaultdecl:
			    XMLERROR("Bad default declaration.");
			    break;
			}
			break;

		      default:
			push_text("");

		    comefrom_fixed:
#ifdef VERBOSE_XMLDEBUG
			fprintf(stderr,"READING ATTVALUE (cdata = %d)\n",cdata);
#endif
			SIMPLE_READ_ATTVALUE(cdata);
			if(THIS->attributes)
			{
			  /* Stack is:
			   * -6 tagname 
			   * -5 attmapping
			   * -4 attname
			   * -3 type
			   * -2 "#FIXED"/""
			   * -1 attvalue
			   */
			  struct svalue *s; 
			  struct mapping *tmp;
			  s=low_mapping_string_lookup(THIS->attributes,
						      sp[-6].u.string);
			  if(!s || s->type!=T_MAPPING)
			  {
			    f_aggregate_mapping(0);
			    tmp=sp[-1].u.mapping;

#ifdef VERBOSE_XMLDEBUG
			    fprintf(stderr,"Creating defaults for %s\n",
				    sp[-7].u.string->str);
#endif
			    mapping_string_insert(THIS->attributes,
						  sp[-7].u.string,
						  sp-1);
			    pop_stack();
			  }else{
			    tmp=s->u.mapping;
			  }
			  s=low_mapping_string_lookup(tmp,
						      sp[-4].u.string);
			  if(!s)
			  {
#ifdef VERBOSE_XMLDEBUG			    

			    fprintf(stderr,
				    "Inserting default for %s %s = '%s'\n",
				    sp[-6].u.string->str,
				    sp[-4].u.string->str,
				    sp[-1].u.string->str);
#endif
			    mapping_string_insert(tmp,
						  sp[-4].u.string,
						  sp-1);
			  }
			}
#ifdef VERBOSE_XMLDEBUG
			else{
			  fprintf(stderr,"THIS->attributes is zero!\n");
			}
#endif
			break;
		    }
#ifdef PIKE_DEBUG
		    if(sp<save)
		      fatal("Stack underflow.\n");
#endif
		    f_aggregate(sp-save);
		    f_aggregate(2);
		    assign_lvalue(sp-3, sp-1);
		    pop_n_elems(2);
		  }
		  push_int(0); /* No data */
		  SYS();
		  break;
		}
		goto unknown_entry_in_dtd;

	      case 'N': /* NOTATION */
	    ignored_dtd_entry:
		push_text("<!");
		READ(2);
		SIMPLE_READNAME();
		SKIPSPACE();

		/* FIXME, DTD's are IGNORED! */
		while(data->len>0 && PEEK(0)!='>')
		  READ(1);

		if(PEEK(0) != '>')
		  XMLERROR("Missing '>' in DTD");
		READ(1);

		push_int(0); /* No attributes */
		push_int(0); /* No data - yet */
		SYS();
		break;

	      default:
	    unknown_entry_in_dtd:
		XMLERROR("Unknown entry in DTD.");

		/* Try to recover */
		while(data->len>0 && PEEK(0)!='>')
		  READ(1);
		break;
	    }
	    break;


	  case '?': /* Processing Info */
	    READ(2);
	    push_text("<?");
	    SIMPLE_READNAME();
	    push_int(0); /* No attributes */
	    SKIPSPACE();
	    BEGIN_STRING(foo);
	    while(data->len>0 && !(PEEK(0)=='?' && PEEK(1)=='>'))
	    {
	      if(PEEK(0)=='\r')
	      {
		READ(1);
		if(PEEK(0)=='\n')
		  READ(1);
		POKE(foo,'\n');
	      }else{
		POKE(foo, PEEK(0));
		READ(1);
	      }
	    }
	    READ(2);
	    END_STRING(foo);
	    SYS();
	    break;

	  default:
	    XMLERROR("Unknown entry in DTD.");
	    break;

	}
	break;

      case ']':
#ifdef VERBOSE_XMLDEBUG
	fprintf(stderr,"low_parse_dtd found ']'\n");
#endif
	done=1;
    }
  }
  return done;
}

static int low_parse_dtd(struct xmldata *data)
{
  int done=0;
  struct svalue *save_sp=sp;

  done=really_low_parse_dtd(data);
#ifdef PIKE_DEBUG
  if(sp<save_sp)
    fatal("Stack underflow.\n");
#endif
#ifdef VERBOSE_XMLDEBUG
  fprintf(stderr,"Exiting low_parse_dtd %p %p\n",sp,save_sp);
#endif
  f_aggregate(sp-save_sp);
#ifdef VERBOSE_XMLDEBUG
  fprintf(stderr,"Exiting low_parse_dtd done\n");
#endif
  /* There is now one value on the stack */
  return done;
}  
static struct pike_string *very_low_parse_xml(struct xmldata *data,
					      struct pike_string *end,
					      int toplevel,
					      struct string_builder *text,
					      int keepspaces)
{
  int done=0;
  while(!done && data->len>0)
  {
    switch(PEEK(0))
    {
      default:
	if(toplevel)
	{
	  if(!isSpace(PEEK(0)))
	  {
	    XMLERROR("All data must be inside tags");
	    READ(1);
	  }
	  SKIPSPACE();
	  break;
	}
	POKE(*text, PEEK(0));
	READ(1);
	continue;

	/* Strangely enough, \r and \r\n should be reported as \n,
	 * but \n\r should be reported as \n\n
	 */
      case '\r':
	if(keepspaces)
	{
	  POKE(*text,'\r');
	  READ(1);
	  break;
	}
	if(toplevel)
	{
	  SKIPSPACE();
	  break;
	}
	POKE(*text,'\n');
	READ(1);
	if(PEEK(0) == '\n') READ(1);
	continue;

      case '&':
	READ_REFERENCE(*text,very_low_parse_xml(&my_tmp, end, toplevel, text, 1));
	continue;

      case '<':
	INTERMISSION(*text);
	
	switch(PEEK(1))
	{
	  case '?': /* Ends with ?> */
	    if(PEEK(2)=='x' &&
	       PEEK(3)=='m' &&
	       PEEK(4)=='l' &&
	       isSpace(PEEK(5)))
	    {
	      push_text("<?xml");
	      READ(6);
	      push_int(0);
	      push_mapping(allocate_mapping(10)); /* Attributes */

	      SIMPLE_READ_ATTRIBUTES(0);
	      
	      if(PEEK(0) != '?' && PEEK(1)!='>')
		XMLERROR("Missing ?> at end of <?xml.");
	      READ(2);
	      
	      push_int(0); /* No data */
	    }else{
	      READ(2);
	      push_text("<?");
	      SIMPLE_READNAME();
	      push_int(0); /* No attributes */
	      SKIPSPACE();
	      BEGIN_STRING(foo);
	      while(!(PEEK(0)=='?' && PEEK(1)=='>'))
	      {
		if(PEEK(0)=='\r')
		{
		  READ(1);
		  if(PEEK(0)=='\n')
		    READ(1);
		  POKE(foo,'\n');
		}else{
		  POKE(foo, PEEK(0));
		  READ(1);
		}
	      }
	      READ(2);
	      END_STRING(foo);
	    }
	    SYS();
	    break;

	  case '!':
	    switch(PEEK(2))
	    {
	      case '-': /* Comment */
		if(PEEK(3)=='-')
		{
		  /* Comment */
		  push_text("<!--");
		  push_int(0); /* No name */
		  push_int(0); /* No attribues */
		  READ(4);
		  READ_COMMENT();
		  SYS();
		}else{
		  XMLERROR("Expected <!-- but got something else.");
		}
		break;


	      case 'A': /* ATTLIST? */
	      case 'E': /* ENTITY? ELEMENT?  */
		XMLERROR("Invalid entry outside DTD.");
		break;

	      case '[':
		if(PEEK(3)=='C' &&
		   PEEK(4)=='D' &&
		   PEEK(5)=='A' &&
		   PEEK(6)=='T' &&
		   PEEK(7)=='A' &&
		   PEEK(8)=='[')
		{
		  READ(9);
		  push_text("<![CDATA[");
		  push_int(0);
		  push_int(0);
		  BEGIN_STRING(cdata);
		  while(data->len>0 && !(PEEK(0)==']' &&
					 PEEK(1)==']' &&
					 PEEK(2)=='>'))
		  {
		    if(PEEK(0)=='\r')
		    {
		      READ(1);
		      if(PEEK(0)=='\n')
			READ(1);
		      POKE(cdata,'\n');
		    }else{
		      POKE(cdata, PEEK(0));
		      READ(1);
		    }
		  }
		  READ(3);
		  END_STRING(cdata);
		  SYS();
		  break;
		}

	      default:
		XMLERROR("Invalid entry.");
		break;

	      case 'D': /* DOCTYPE? */
/*		fprintf(stderr,"FOO: %c%c%c%c\n",PEEK(3),PEEK(4),PEEK(5),PEEK(6)); */
		if(PEEK(3)!='O' ||
		   PEEK(4)!='C' ||
		   PEEK(5)!='T' ||
		   PEEK(6)!='Y' ||
		   PEEK(7)!='P' ||
		   PEEK(8)!='E' ||
		   !isSpace(PEEK(9)))
		{
		  XMLERROR("Expected 'DOCTYPE', got something else.");
		}else{
		  READ(9);
		  SKIPSPACE();
		  push_text("<!DOCTYPE");
		  SIMPLE_READNAME(); /* NAME */
		  SKIPSPACE();
		  switch(PEEK(0))
		  {
		    case 'P':
		      if(PEEK(1)=='U' &&
			 PEEK(2)=='B' &&
			 PEEK(3)=='L' &&
			 PEEK(4)=='I' &&
			 PEEK(5)=='C')
		      {
			SIMPLE_READNAME();
			SIMPLE_READ_PUBIDLITERAL();
			push_text("SYSTEM");
			SIMPLE_READ_SYSTEMLITERAL();
			SKIPSPACE();
			f_aggregate_mapping(4);
		      }else{
			XMLERROR("Expected PUBLIC, found something else.");
			f_aggregate_mapping(0);
		      }
		      break;

		    case 'S':
		      if(PEEK(1)=='Y' &&
			 PEEK(2)=='S' &&
			 PEEK(3)=='T' &&
			 PEEK(4)=='E' &&
			 PEEK(5)=='M')
		      {
			SIMPLE_READNAME();
			SIMPLE_READ_SYSTEMLITERAL();
			SKIPSPACE();
			f_aggregate_mapping(2);
		      }else{
			XMLERROR("Expected SYSTEM, found something else.");
			f_aggregate_mapping(0);
		      }
		      break;

		    default:
		      f_aggregate_mapping(0);
		  }

		  if(PEEK(0)=='[')
		  {
		    READ(1);
		    low_parse_dtd(data);
#ifdef VERBOSE_XMLDEBUG
		    fprintf(stderr,"FOO: %c%c%c%c\n",PEEK(0),PEEK(1),PEEK(2),PEEK(3));
#endif
		    if(PEEK(0) != ']')
		      XMLERROR("Missing ] in DOCTYPE tag.");
		    READ(1);
		    SKIPSPACE();
		  }else{
		    push_int(0);
		  }
		  if(PEEK(0)!='>')
		    XMLERROR("Missing '>' in DOCTYPE tag.");
		  READ(1);
		  SYS();
		}
		break;
	    }
	    break;

	  case '/': /* End tag */
	    READ(2);
	    SIMPLE_READNAME();
	    SKIPSPACE();
	    if(PEEK(0)!='>')
	      XMLERROR("Missing > in end tag.");
	    else
	      READ(1);
	    if(end!=sp[-1].u.string)
	    {
	      XMLERROR("Unmatched end tag.");
	    }else{
	      end=0;
	    }
	    done=1;
	    pop_stack();
	    break;

	  default:
	    /* 'Normal' tag (we hope) */
	    push_text(">"); /* This might change */

	    READ(1);
	    SIMPLE_READNAME();
	    if(THIS->attributes)
	    {
	      struct svalue *s;
	      s=low_mapping_string_lookup(THIS->attributes,
					      sp[-1].u.string);
	      if(s && s->type==T_MAPPING)
	      {
		ref_push_mapping(s->u.mapping);
		f_copy_value(1);
	      }else{
		push_mapping(allocate_mapping(10)); /* Attributes */
	      }
	    }else{
	      push_mapping(allocate_mapping(10)); /* Attributes */
	    }

	    {
	      struct mapping *m=0;
	      if(THIS->is_cdata)
	      {
		struct svalue *s=low_mapping_lookup(THIS->is_cdata,sp-2);
		if(s && s->type==T_MAPPING)
		  m=s->u.mapping;
	      }
	      SIMPLE_READ_ATTRIBUTES(m);
	    }

	    switch(PEEK(0))
	    {
	      case '>':
		READ(1);
		if(low_parse_xml(data, sp[-2].u.string,0))
		  XMLERROR("Unmatched tag.");
		SYS();
		break;

	      case '/':
		READ(1);
		if(PEEK(0)!='>')
		  XMLERROR("Missing '>' in empty tag.");
		READ(1);
		/* Self-contained tag */
		free_string(sp[-3].u.string);
		sp[-3].u.string=make_shared_string("<>");
		push_int(0); /* No data */
		SYS();
		break;
		
	    }
	}
    }
  }
  return end;
}

static int low_parse_xml(struct xmldata *data,
			 struct pike_string *end,
			 int toplevel)
{
  struct svalue *save_sp=sp;
  BEGIN_STRING(text);
  end=very_low_parse_xml(data,end,toplevel,&text,0);
  INTERMISSION(text);
  END_STRING(text);
  pop_stack();
#ifdef PIKE_DEBUG
  if(sp<save_sp)
    fatal("Stack underflow.\n");
#endif
  f_aggregate(sp-save_sp);
  /* There is now one value on the stack */
  return !!end;
}

static void parse_xml(INT32 args)
{
  struct svalue tmp;
  struct pike_string *s;
  struct xmldata data;

  s=sp[-args].u.string;
  if(args<2)
    error("Too few arguments to XML->parse()\n");

#if 0
  if(!s->size_shift)
  {
    if(STR0(s)[0]==0xfe && STR1(s)[0]==0xff)
    {
      /* String is UTF8, convert to unicode here */
      
    }
  }
#endif
  data.datap=MKPCHARP_STR(s);
  data.len=s->len;
  data.pos=0;
  data.func=sp+1-args;
  data.extra_args=sp+2-args;
  data.num_extra_args=args-2;
  data.extra_arg_types=-1; /* FIXME */
    
  low_parse_xml(&data,0,1);
  tmp=*--sp;
  pop_n_elems(args);
  *sp++=tmp;
}

static void parse_dtd(INT32 args)
{
  struct svalue tmp;
  struct pike_string *s;
  struct xmldata data;

  s=sp[-args].u.string;
  if(args<2)
    error("Too few arguments to XML->parse()\n");

#if 0
  if(!s->size_shift)
  {
    if(STR0(s)[0]==0xfe && STR1(s)[0]==0xff)
    {
      /* String is UTF8, convert to unicode here */
      
    }
  }
#endif
  data.datap=MKPCHARP_STR(s);
  data.len=s->len;
  data.pos=0;
  data.func=sp+1-args;
  data.extra_args=sp+2-args;
  data.num_extra_args=args-2;
  data.extra_arg_types=-1; /* FIXME */
    
  low_parse_dtd(&data);
  tmp=*--sp;
  pop_n_elems(args);
  *sp++=tmp;
}

static void create(INT32 args)
{
  pop_n_elems(args);
  if(!THIS->entities)
  {
    push_text("lt");    push_text("&#60;");
    push_text("gt");    push_text(">");
    push_text("amp");   push_text("&#38;");
    push_text("apos");  push_text("'");
    push_text("quot");  push_text("\"");

    f_aggregate_mapping(10);
    THIS->entities=sp[-1].u.mapping;
    sp--;
  }

  if(!THIS->attributes)
  {
    f_aggregate_mapping(0);
    THIS->attributes=sp[-1].u.mapping;
    sp--;
  }

  if(!THIS->is_cdata)
  {
    f_aggregate_mapping(0);
    THIS->is_cdata=sp[-1].u.mapping;
    sp--;
  }

  push_int(0);
}

static void autoconvert(INT32 args)
{
  INT32 e;
  struct string_builder b;
  struct pike_string *s;

  if(!args)
    error("Too few arguments to XML->autoconvert.\n");

  pop_n_elems(args-1);
  if(sp[-1].type != T_STRING)
    error("Bad argument 1 to XML->autoconvert.\n");

  s=sp[-1].u.string;
  if(!s->size_shift)
  {
    switch((STR0(s)[0]<<8) | STR0(s)[1])
    {
      case 0xfffe: /* UTF-16, little-endian */
      {
	struct pike_string *t=begin_shared_string(s->len);
	IF_XMLDEBUG(fprintf(stderr,"UTF-16, little endian detected.\n"));
	for(e=0;e<s->len;e+=1) t->str[e]=s->str[e^1];
	pop_stack();
	push_string(end_shared_string(t));
      }
	
      case 0xfeff: /* UTF-16, big-endian */
	IF_XMLDEBUG(fprintf(stderr,"UTF-16, big endian detected.\n"));
	push_int(2);
	push_int(0x7fffffff);
	o_range();
	f_unicode_to_string(1);
	return;
    }

    switch((STR0(s)[0]<<24) | (STR0(s)[1]<<16) | (STR0(s)[2]<<8) | STR0(s)[3])
    {
      case 0x0000003c: /* UCS4 1234 byte order (big endian) */
	IF_XMLDEBUG(fprintf(stderr,"UCS4(1234) detected.\n"));
	init_string_builder(&b,4);
	for(e=0;e<s->len;e+=4)
	  string_builder_putchar(&b,
	    (STR0(s)[e+0]<<24) | (STR0(s)[e+1]<<16) | (STR0(s)[e+2]<<8) | STR0(s)[e+3]);
	pop_stack();
	push_string(finish_string_builder(&b));
	return;
	
      case 0x3c000000: /* UCS4 4321 byte order (little endian)*/
	IF_XMLDEBUG(fprintf(stderr,"UCS4(4321) detected.\n"));
	init_string_builder(&b,4);
	for(e=0;e<s->len;e+=4)
	  string_builder_putchar(&b,
	    (STR0(s)[e+3]<<24) | (STR0(s)[e+2]<<16) | (STR0(s)[e+1]<<8) | STR0(s)[e+0]);
	pop_stack();
	push_string(finish_string_builder(&b));
	return;

      case 0x00003c00: /* UCS4 2143 byte order */
	IF_XMLDEBUG(fprintf(stderr,"UCS4(2143) detected.\n"));
	init_string_builder(&b,4);
	for(e=0;e<s->len;e+=4)
	  string_builder_putchar(&b,
	    (STR0(s)[e+1]<<24) | (STR0(s)[e+0]<<16) | (STR0(s)[e+3]<<8) | STR0(s)[e+2]);
	pop_stack();
	push_string(finish_string_builder(&b));
	return;

      case 0x003c0000: /* UCS4 3412 byte order */
	IF_XMLDEBUG(fprintf(stderr,"UCS4(3412) detected.\n"));
	init_string_builder(&b,4);
	for(e=0;e<s->len;e+=4)
	  string_builder_putchar(&b,
	    (STR0(s)[e+2]<<24) | (STR0(s)[e+3]<<16) | (STR0(s)[e+0]<<8) | STR0(s)[e+1]);
	pop_stack();
	push_string(finish_string_builder(&b));
	return;

      case 0x003c003f: /* UTF-16, big-endian, no byte order mark */
	IF_XMLDEBUG(fprintf(stderr,"UTF-16, bit-endian, no byte order mark detected.\n"));
	f_unicode_to_string(1);
	break;

      case 0x3c003f00: /* UTF-16, little endian, no byte order mark */
	IF_XMLDEBUG(fprintf(stderr,"UTF-16, little-endian, no byte order mark detected.\n"));
	error("XML: Little endian byte order not supported yet.\n");

      case 0x3c3f786d: /* ASCII? UTF-8? ISO-8859? */
	IF_XMLDEBUG(fprintf(stderr,"Extended ASCII detected (assuming UTF8).\n"));
	/* Assume utf8 for now */
	f_utf8_to_string(1);
	break;
	
      case 0x4c6fa794: /* EBCDIC */
	IF_XMLDEBUG(fprintf(stderr,"EBCDIC detected.\n"));
	error("XML: EBCDIC not supported yet.\n");
    }
  }
  IF_XMLDEBUG(fprintf(stderr,"No encoding detected.\n"));
  f_utf8_to_string(1);
}

void init_xml(void)
{
  INT32 off;
  start_new_program();
  off=add_storage(sizeof(struct xmlobj));
  map_variable("__entities","mapping",0,
	       off + OFFSETOF(xmlobj, entities),T_MAPPING);
  map_variable("__attributes","mapping",0,
	       off + OFFSETOF(xmlobj, attributes),T_MAPPING);
  map_variable("__is_cdata","mapping",0,
	       off + OFFSETOF(xmlobj, is_cdata),T_MAPPING);

#define CALLBACKTYPE \
 "function(string,string,mapping,array|string,mapping(string:mixed):0=mixed)"

#define PARSETYPE \
 "function(string," CALLBACKTYPE ",mixed...:array(0))"

  add_function("autoconvert",autoconvert,"function(string:string)",0);
  add_function("parse",parse_xml,PARSETYPE,0);
  add_function("parse_dtd",parse_dtd,PARSETYPE,0);
  add_function("create",create,"function(:void)",0);
  end_class("XML",0);
}