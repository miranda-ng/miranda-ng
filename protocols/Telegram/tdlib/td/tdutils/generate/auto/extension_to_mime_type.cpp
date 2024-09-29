/* ANSI-C code produced by gperf version 3.1 */
/* Command-line: 'W:\\Test\\td\\vcpkg\\installed\\x64-windows\\tools\\gperf\\gperf.exe' -m100 --output-file=auto/extension_to_mime_type.cpp auto/extension_to_mime_type.gperf  */
/* Computed positions: -k'1-4,6,$' */

#if !((' ' == 32) && ('!' == 33) && ('"' == 34) && ('#' == 35) \
      && ('%' == 37) && ('&' == 38) && ('\'' == 39) && ('(' == 40) \
      && (')' == 41) && ('*' == 42) && ('+' == 43) && (',' == 44) \
      && ('-' == 45) && ('.' == 46) && ('/' == 47) && ('0' == 48) \
      && ('1' == 49) && ('2' == 50) && ('3' == 51) && ('4' == 52) \
      && ('5' == 53) && ('6' == 54) && ('7' == 55) && ('8' == 56) \
      && ('9' == 57) && (':' == 58) && (';' == 59) && ('<' == 60) \
      && ('=' == 61) && ('>' == 62) && ('?' == 63) && ('A' == 65) \
      && ('B' == 66) && ('C' == 67) && ('D' == 68) && ('E' == 69) \
      && ('F' == 70) && ('G' == 71) && ('H' == 72) && ('I' == 73) \
      && ('J' == 74) && ('K' == 75) && ('L' == 76) && ('M' == 77) \
      && ('N' == 78) && ('O' == 79) && ('P' == 80) && ('Q' == 81) \
      && ('R' == 82) && ('S' == 83) && ('T' == 84) && ('U' == 85) \
      && ('V' == 86) && ('W' == 87) && ('X' == 88) && ('Y' == 89) \
      && ('Z' == 90) && ('[' == 91) && ('\\' == 92) && (']' == 93) \
      && ('^' == 94) && ('_' == 95) && ('a' == 97) && ('b' == 98) \
      && ('c' == 99) && ('d' == 100) && ('e' == 101) && ('f' == 102) \
      && ('g' == 103) && ('h' == 104) && ('i' == 105) && ('j' == 106) \
      && ('k' == 107) && ('l' == 108) && ('m' == 109) && ('n' == 110) \
      && ('o' == 111) && ('p' == 112) && ('q' == 113) && ('r' == 114) \
      && ('s' == 115) && ('t' == 116) && ('u' == 117) && ('v' == 118) \
      && ('w' == 119) && ('x' == 120) && ('y' == 121) && ('z' == 122) \
      && ('{' == 123) && ('|' == 124) && ('}' == 125) && ('~' == 126))
/* The character set is not based on ISO-646.  */
#error "gperf generated tables don't work with this execution character set. Please report a bug to <bug-gperf@gnu.org>."
#endif

#line 12 "auto/extension_to_mime_type.gperf"
struct extension_and_mime_type {
  const char *extension;
  const char *mime_type;
};
#include <string.h>
/* maximum key range = 3365, duplicates = 0 */

#ifndef GPERF_DOWNCASE
#define GPERF_DOWNCASE 1
static unsigned char gperf_downcase[256] =
  {
      0,   1,   2,   3,   4,   5,   6,   7,   8,   9,  10,  11,  12,  13,  14,
     15,  16,  17,  18,  19,  20,  21,  22,  23,  24,  25,  26,  27,  28,  29,
     30,  31,  32,  33,  34,  35,  36,  37,  38,  39,  40,  41,  42,  43,  44,
     45,  46,  47,  48,  49,  50,  51,  52,  53,  54,  55,  56,  57,  58,  59,
     60,  61,  62,  63,  64,  97,  98,  99, 100, 101, 102, 103, 104, 105, 106,
    107, 108, 109, 110, 111, 112, 113, 114, 115, 116, 117, 118, 119, 120, 121,
    122,  91,  92,  93,  94,  95,  96,  97,  98,  99, 100, 101, 102, 103, 104,
    105, 106, 107, 108, 109, 110, 111, 112, 113, 114, 115, 116, 117, 118, 119,
    120, 121, 122, 123, 124, 125, 126, 127, 128, 129, 130, 131, 132, 133, 134,
    135, 136, 137, 138, 139, 140, 141, 142, 143, 144, 145, 146, 147, 148, 149,
    150, 151, 152, 153, 154, 155, 156, 157, 158, 159, 160, 161, 162, 163, 164,
    165, 166, 167, 168, 169, 170, 171, 172, 173, 174, 175, 176, 177, 178, 179,
    180, 181, 182, 183, 184, 185, 186, 187, 188, 189, 190, 191, 192, 193, 194,
    195, 196, 197, 198, 199, 200, 201, 202, 203, 204, 205, 206, 207, 208, 209,
    210, 211, 212, 213, 214, 215, 216, 217, 218, 219, 220, 221, 222, 223, 224,
    225, 226, 227, 228, 229, 230, 231, 232, 233, 234, 235, 236, 237, 238, 239,
    240, 241, 242, 243, 244, 245, 246, 247, 248, 249, 250, 251, 252, 253, 254,
    255
  };
#endif

#ifndef GPERF_CASE_STRCMP
#define GPERF_CASE_STRCMP 1
static int
gperf_case_strcmp (register const char *s1, register const char *s2)
{
  for (;;)
    {
      unsigned char c1 = gperf_downcase[(unsigned char)*s1++];
      unsigned char c2 = gperf_downcase[(unsigned char)*s2++];
      if (c1 != 0 && c1 == c2)
        continue;
      return (int)c1 - (int)c2;
    }
}
#endif

#ifdef __GNUC__
__inline
#else
#ifdef __cplusplus
inline
#endif
#endif
static unsigned int
extension_hash (register const char *str, register size_t len)
{
  static const unsigned short asso_values[] =
    {
      3376, 3376, 3376, 3376, 3376, 3376, 3376, 3376, 3376, 3376,
      3376, 3376, 3376, 3376, 3376, 3376, 3376, 3376, 3376, 3376,
      3376, 3376, 3376, 3376, 3376, 3376, 3376, 3376, 3376, 3376,
      3376, 3376, 3376, 3376, 3376, 3376, 3376, 3376, 3376, 3376,
      3376, 3376, 3376, 3376, 3376,    5, 3376, 3376,    9,   21,
       226,  309,  314,   31,   31,    6,    6,    9, 3376, 3376,
      3376, 3376, 3376, 3376, 3376,  257,  522,   20,   36,  584,
        21,  570,  866,  174, 1292,  522,   94,    6,  381,  407,
         9,  574,  153,    5,   72,  213,  331,   32,   10,  499,
        75, 3376, 3376, 3376, 3376,    5, 3376,  257,  522,   20,
        36,  584,   21,  570,  866,  174, 1292,  522,   94,    6,
       381,  407,    9,  574,  153,    5,   72,  213,  331,   32,
        10,  499,   75,  999, 1528, 1515, 1457,  291,   11,  994,
       952,   35, 1229, 1226,  140,   37,  983,  198,  701,  775,
       149,  641, 1579,    6,  631, 3376, 3376, 3376, 3376, 3376,
      3376,   32,   10,  499,   75,  999, 1528, 1515, 1457,  291,
        11,  994,  952,   35, 1229, 1226,  140,   37,  983,  198,
       701,  775,  149,  641, 1579,    6,  631, 3376, 3376, 3376,
      3376, 3376, 3376, 3376, 3376, 3376, 3376, 3376, 3376, 3376,
      3376, 3376, 3376, 3376, 3376, 3376, 3376, 3376, 3376, 3376,
      3376, 3376, 3376, 3376, 3376, 3376, 3376, 3376, 3376, 3376,
      3376, 3376, 3376, 3376, 3376, 3376, 3376, 3376, 3376, 3376,
      3376, 3376, 3376, 3376, 3376, 3376, 3376, 3376, 3376, 3376,
      3376, 3376, 3376, 3376, 3376, 3376, 3376, 3376, 3376, 3376,
      3376, 3376, 3376, 3376, 3376, 3376, 3376, 3376, 3376, 3376,
      3376, 3376, 3376, 3376, 3376, 3376, 3376, 3376, 3376, 3376,
      3376, 3376, 3376, 3376, 3376, 3376, 3376, 3376, 3376, 3376,
      3376, 3376, 3376, 3376, 3376, 3376, 3376, 3376, 3376, 3376,
      3376, 3376, 3376, 3376, 3376, 3376, 3376, 3376, 3376, 3376,
      3376, 3376, 3376, 3376, 3376, 3376, 3376, 3376, 3376, 3376,
      3376, 3376, 3376, 3376, 3376, 3376, 3376, 3376, 3376, 3376
    };
  register unsigned int hval = len;

  switch (hval)
    {
      default:
        hval += asso_values[(unsigned char)str[5]];
      /*FALLTHROUGH*/
      case 5:
      case 4:
        hval += asso_values[(unsigned char)str[3]];
      /*FALLTHROUGH*/
      case 3:
        hval += asso_values[(unsigned char)str[2]];
      /*FALLTHROUGH*/
      case 2:
        hval += asso_values[(unsigned char)str[1]+54];
      /*FALLTHROUGH*/
      case 1:
        hval += asso_values[(unsigned char)str[0]];
        break;
    }
  return hval + asso_values[(unsigned char)str[len - 1]];
}

const struct extension_and_mime_type *
search_extension (register const char *str, register size_t len)
{
  enum
    {
      TOTAL_KEYWORDS = 993,
      MIN_WORD_LENGTH = 1,
      MAX_WORD_LENGTH = 11,
      MIN_HASH_VALUE = 11,
      MAX_HASH_VALUE = 3375
    };

  static const struct extension_and_mime_type wordlist[] =
    {
      {"",nullptr}, {"",nullptr}, {"",nullptr}, {"",nullptr},
      {"",nullptr}, {"",nullptr}, {"",nullptr}, {"",nullptr},
      {"",nullptr}, {"",nullptr}, {"",nullptr},
#line 694 "auto/extension_to_mime_type.gperf"
      {"s", "text/x-asm"},
      {"",nullptr}, {"",nullptr}, {"",nullptr}, {"",nullptr},
      {"",nullptr}, {"",nullptr}, {"",nullptr},
#line 580 "auto/extension_to_mime_type.gperf"
      {"p", "text/x-pascal"},
      {"",nullptr}, {"",nullptr}, {"",nullptr}, {"",nullptr},
      {"",nullptr}, {"",nullptr}, {"",nullptr}, {"",nullptr},
#line 587 "auto/extension_to_mime_type.gperf"
      {"p7s", "application/pkcs7-signature"},
      {"",nullptr},
#line 585 "auto/extension_to_mime_type.gperf"
      {"p7m", "application/pkcs7-mime"},
      {"",nullptr}, {"",nullptr}, {"",nullptr},
#line 592 "auto/extension_to_mime_type.gperf"
      {"pbm", "image/x-portable-bitmap"},
#line 947 "auto/extension_to_mime_type.gperf"
      {"xbm", "image/x-xbitmap"},
      {"",nullptr}, {"",nullptr}, {"",nullptr}, {"",nullptr},
      {"",nullptr},
#line 84 "auto/extension_to_mime_type.gperf"
      {"c", "text/x-c"},
#line 242 "auto/extension_to_mime_type.gperf"
      {"f77", "text/x-fortran"},
#line 240 "auto/extension_to_mime_type.gperf"
      {"f", "text/x-fortran"},
#line 245 "auto/extension_to_mime_type.gperf"
      {"fbs", "image/vnd.fastbidsheet"},
#line 97 "auto/extension_to_mime_type.gperf"
      {"cb7", "application/x-cbr"},
      {"",nullptr}, {"",nullptr},
#line 744 "auto/extension_to_mime_type.gperf"
      {"sm", "application/vnd.stepmania.stepchart"},
      {"",nullptr}, {"",nullptr}, {"",nullptr}, {"",nullptr},
#line 974 "auto/extension_to_mime_type.gperf"
      {"xm", "audio/xm"},
#line 589 "auto/extension_to_mime_type.gperf"
      {"pas", "text/x-pascal"},
#line 901 "auto/extension_to_mime_type.gperf"
      {"wbs", "application/vnd.criticaltools.wbs+xml"},
      {"",nullptr}, {"",nullptr},
#line 584 "auto/extension_to_mime_type.gperf"
      {"p7c", "application/pkcs7-mime"},
      {"",nullptr}, {"",nullptr}, {"",nullptr}, {"",nullptr},
#line 943 "auto/extension_to_mime_type.gperf"
      {"xap", "application/x-silverlight-app"},
#line 265 "auto/extension_to_mime_type.gperf"
      {"fm", "application/vnd.framemaker"},
      {"",nullptr}, {"",nullptr}, {"",nullptr}, {"",nullptr},
      {"",nullptr},
#line 900 "auto/extension_to_mime_type.gperf"
      {"wbmp", "image/vnd.wap.wbmp"},
      {"",nullptr}, {"",nullptr},
#line 94 "auto/extension_to_mime_type.gperf"
      {"cap", "application/vnd.tcpdump.pcap"},
      {"",nullptr},
#line 912 "auto/extension_to_mime_type.gperf"
      {"wm", "video/x-ms-wm"},
#line 136 "auto/extension_to_mime_type.gperf"
      {"cmp", "application/vnd.yellowriver-custom-menu"},
      {"",nullptr},
#line 137 "auto/extension_to_mime_type.gperf"
      {"cmx", "image/x-cmx"},
      {"",nullptr}, {"",nullptr}, {"",nullptr},
#line 696 "auto/extension_to_mime_type.gperf"
      {"saf", "application/vnd.yamaha.smaf-audio"},
      {"",nullptr},
#line 185 "auto/extension_to_mime_type.gperf"
      {"dms", "application/octet-stream"},
#line 745 "auto/extension_to_mime_type.gperf"
      {"smf", "application/vnd.stardivision.math"},
#line 469 "auto/extension_to_mime_type.gperf"
      {"mmf", "application/vnd.smaf"},
#line 899 "auto/extension_to_mime_type.gperf"
      {"wax", "audio/x-ms-wax"},
#line 433 "auto/extension_to_mime_type.gperf"
      {"mads", "application/mads+xml"},
#line 1003 "auto/extension_to_mime_type.gperf"
      {"z7", "application/x-zmachine"},
#line 921 "auto/extension_to_mime_type.gperf"
      {"wmx", "video/x-ms-wmx"},
      {"",nullptr},
#line 184 "auto/extension_to_mime_type.gperf"
      {"dmp", "application/vnd.tcpdump.pcap"},
      {"",nullptr},
#line 591 "auto/extension_to_mime_type.gperf"
      {"pbd", "application/vnd.powerbuilder6"},
#line 946 "auto/extension_to_mime_type.gperf"
      {"xbd", "application/vnd.fujixerox.docuworks.binder"},
      {"",nullptr},
#line 93 "auto/extension_to_mime_type.gperf"
      {"caf", "audio/x-caf"},
#line 133 "auto/extension_to_mime_type.gperf"
      {"cmc", "application/vnd.cosmocaller"},
      {"",nullptr},
#line 949 "auto/extension_to_mime_type.gperf"
      {"xdm", "application/vnd.syncml.dm+xml"},
#line 709 "auto/extension_to_mime_type.gperf"
      {"sdp", "application/sdp"},
      {"",nullptr}, {"",nullptr}, {"",nullptr}, {"",nullptr},
#line 950 "auto/extension_to_mime_type.gperf"
      {"xdp", "application/vnd.adobe.xdp+xml"},
      {"",nullptr},
#line 590 "auto/extension_to_mime_type.gperf"
      {"paw", "application/vnd.pawaafile"},
      {"",nullptr}, {"",nullptr}, {"",nullptr},
#line 915 "auto/extension_to_mime_type.gperf"
      {"wmf", "application/x-msmetafile"},
#line 160 "auto/extension_to_mime_type.gperf"
      {"daf", "application/vnd.mobius.daf"},
      {"",nullptr}, {"",nullptr},
#line 468 "auto/extension_to_mime_type.gperf"
      {"mmd", "application/vnd.chipnuts.karaoke-mmd"},
      {"",nullptr},
#line 113 "auto/extension_to_mime_type.gperf"
      {"cdx", "chemical/x-cdx"},
      {"",nullptr},
#line 951 "auto/extension_to_mime_type.gperf"
      {"xdssc", "application/dssc+xml"},
      {"",nullptr}, {"",nullptr},
#line 705 "auto/extension_to_mime_type.gperf"
      {"sdc", "application/vnd.stardivision.calc"},
      {"",nullptr},
#line 1009 "auto/extension_to_mime_type.gperf"
      {"zmm", "application/vnd.handheld-entertainment+xml"},
      {"",nullptr}, {"",nullptr},
#line 905 "auto/extension_to_mime_type.gperf"
      {"wdp", "image/vnd.ms-photo"},
#line 601 "auto/extension_to_mime_type.gperf"
      {"pdf", "application/pdf"},
#line 948 "auto/extension_to_mime_type.gperf"
      {"xdf", "application/xcap-diff+xml"},
      {"",nullptr}, {"",nullptr}, {"",nullptr}, {"",nullptr},
      {"",nullptr}, {"",nullptr},
#line 134 "auto/extension_to_mime_type.gperf"
      {"cmdf", "chemical/x-cmdf"},
      {"",nullptr},
#line 897 "auto/extension_to_mime_type.gperf"
      {"wad", "application/x-doom"},
#line 106 "auto/extension_to_mime_type.gperf"
      {"cdf", "application/x-netcdf"},
#line 248 "auto/extension_to_mime_type.gperf"
      {"fdf", "application/vnd.fdf"},
#line 914 "auto/extension_to_mime_type.gperf"
      {"wmd", "application/x-ms-wmd"},
      {"",nullptr},
#line 928 "auto/extension_to_mime_type.gperf"
      {"wqd", "application/vnd.wqd"},
#line 796 "auto/extension_to_mime_type.gperf"
      {"t", "text/troff"},
      {"",nullptr},
#line 710 "auto/extension_to_mime_type.gperf"
      {"sdw", "application/vnd.stardivision.writer"},
      {"",nullptr}, {"",nullptr}, {"",nullptr}, {"",nullptr},
#line 952 "auto/extension_to_mime_type.gperf"
      {"xdw", "application/vnd.fujixerox.docuworks"},
      {"",nullptr}, {"",nullptr},
#line 706 "auto/extension_to_mime_type.gperf"
      {"sdd", "application/vnd.stardivision.impress"},
      {"",nullptr},
#line 902 "auto/extension_to_mime_type.gperf"
      {"wbxml", "application/vnd.wap.wbxml"},
      {"",nullptr}, {"",nullptr}, {"",nullptr},
#line 492 "auto/extension_to_mime_type.gperf"
      {"mpm", "application/vnd.blueice.multipass"},
#line 629 "auto/extension_to_mime_type.gperf"
      {"pps", "application/vnd.ms-powerpoint"},
#line 982 "auto/extension_to_mime_type.gperf"
      {"xps", "application/vnd.ms-xpsdocument"},
#line 628 "auto/extension_to_mime_type.gperf"
      {"ppm", "image/x-portable-pixmap"},
#line 980 "auto/extension_to_mime_type.gperf"
      {"xpm", "image/x-xpixmap"},
#line 757 "auto/extension_to_mime_type.gperf"
      {"spp", "application/scvp-vp-response"},
#line 494 "auto/extension_to_mime_type.gperf"
      {"mpp", "application/vnd.ms-project"},
#line 759 "auto/extension_to_mime_type.gperf"
      {"spx", "audio/ogg"},
#line 993 "auto/extension_to_mime_type.gperf"
      {"xyz", "chemical/x-xyz"},
#line 630 "auto/extension_to_mime_type.gperf"
      {"ppsm", "application/vnd.ms-powerpoint.slideshow.macroenabled.12"},
      {"",nullptr}, {"",nullptr},
#line 984 "auto/extension_to_mime_type.gperf"
      {"xpx", "application/vnd.intercon.formnet"},
#line 990 "auto/extension_to_mime_type.gperf"
      {"xvm", "application/xv+xml"},
#line 918 "auto/extension_to_mime_type.gperf"
      {"wmls", "text/vnd.wap.wmlscript"},
      {"",nullptr},
#line 100 "auto/extension_to_mime_type.gperf"
      {"cbt", "application/x-cbr"},
#line 631 "auto/extension_to_mime_type.gperf"
      {"ppsx", "application/vnd.openxmlformats-officedocument.presentationml.slideshow"},
#line 403 "auto/extension_to_mime_type.gperf"
      {"lbd", "application/vnd.llamagraphics.life-balance.desktop"},
      {"",nullptr},
#line 142 "auto/extension_to_mime_type.gperf"
      {"cpp", "text/x-c"},
      {"",nullptr},
#line 101 "auto/extension_to_mime_type.gperf"
      {"cbz", "application/x-cbr"},
#line 268 "auto/extension_to_mime_type.gperf"
      {"fpx", "image/vnd.fpx"},
#line 927 "auto/extension_to_mime_type.gperf"
      {"wps", "application/vnd.ms-works"},
#line 168 "auto/extension_to_mime_type.gperf"
      {"ddd", "application/vnd.fujixerox.ddd"},
#line 193 "auto/extension_to_mime_type.gperf"
      {"dp", "application/vnd.osgi.dp"},
#line 753 "auto/extension_to_mime_type.gperf"
      {"spc", "application/x-pkcs7-certificates"},
#line 485 "auto/extension_to_mime_type.gperf"
      {"mpc", "application/vnd.mophun.certificate"},
#line 754 "auto/extension_to_mime_type.gperf"
      {"spf", "application/vnd.yamaha.smaf-phrase"},
#line 919 "auto/extension_to_mime_type.gperf"
      {"wmlsc", "application/vnd.wap.wmlscriptc"},
      {"",nullptr}, {"",nullptr},
#line 695 "auto/extension_to_mime_type.gperf"
      {"s3m", "audio/s3m"},
      {"",nullptr},
#line 677 "auto/extension_to_mime_type.gperf"
      {"rm", "application/vnd.rn-realmedia"},
#line 783 "auto/extension_to_mime_type.gperf"
      {"svc", "application/vnd.dvb.service"},
#line 664 "auto/extension_to_mime_type.gperf"
      {"ras", "image/x-cmu-raster"},
#line 96 "auto/extension_to_mime_type.gperf"
      {"cat", "application/vnd.ms-pki.seccat"},
#line 662 "auto/extension_to_mime_type.gperf"
      {"ram", "audio/x-pn-realaudio"},
#line 680 "auto/extension_to_mime_type.gperf"
      {"rms", "application/vnd.jcp.javame.midlet-rms"},
#line 1002 "auto/extension_to_mime_type.gperf"
      {"z6", "application/x-zmachine"},
      {"",nullptr},
#line 934 "auto/extension_to_mime_type.gperf"
      {"wvx", "video/x-ms-wvx"},
#line 917 "auto/extension_to_mime_type.gperf"
      {"wmlc", "application/vnd.wap.wmlc"},
      {"",nullptr}, {"",nullptr}, {"",nullptr},
#line 679 "auto/extension_to_mime_type.gperf"
      {"rmp", "audio/x-pn-realaudio-plugin"},
#line 114 "auto/extension_to_mime_type.gperf"
      {"cdxml", "application/vnd.chemdraw+xml"},
#line 500 "auto/extension_to_mime_type.gperf"
      {"ms", "text/troff"},
      {"",nullptr},
#line 697 "auto/extension_to_mime_type.gperf"
      {"sbml", "application/sbml+xml"},
#line 639 "auto/extension_to_mime_type.gperf"
      {"ps", "application/postscript"},
      {"",nullptr}, {"",nullptr},
#line 983 "auto/extension_to_mime_type.gperf"
      {"xpw", "application/vnd.intercon.formnet"},
      {"",nullptr}, {"",nullptr},
#line 922 "auto/extension_to_mime_type.gperf"
      {"wmz", "application/x-ms-wmz"},
      {"",nullptr},
#line 352 "auto/extension_to_mime_type.gperf"
      {"ims", "application/vnd.ms-ims"},
#line 987 "auto/extension_to_mime_type.gperf"
      {"xsm", "application/vnd.syncml+xml"},
#line 627 "auto/extension_to_mime_type.gperf"
      {"ppd", "application/vnd.cups-ppd"},
      {"",nullptr}, {"",nullptr}, {"",nullptr}, {"",nullptr},
#line 784 "auto/extension_to_mime_type.gperf"
      {"svd", "application/vnd.svd"},
#line 351 "auto/extension_to_mime_type.gperf"
      {"imp", "application/vnd.accpac.simply.imp"},
#line 151 "auto/extension_to_mime_type.gperf"
      {"css", "text/css"},
      {"",nullptr},
#line 760 "auto/extension_to_mime_type.gperf"
      {"sql", "application/x-sql"},
      {"",nullptr},
#line 619 "auto/extension_to_mime_type.gperf"
      {"pml", "application/vnd.ctc-posml"},
#line 975 "auto/extension_to_mime_type.gperf"
      {"xml", "application/xml"},
#line 633 "auto/extension_to_mime_type.gperf"
      {"pptm", "application/vnd.ms-powerpoint.presentation.macroenabled.12"},
      {"",nullptr},
#line 150 "auto/extension_to_mime_type.gperf"
      {"csp", "application/vnd.commonspace"},
#line 942 "auto/extension_to_mime_type.gperf"
      {"xaml", "application/xaml+xml"},
      {"",nullptr}, {"",nullptr}, {"",nullptr}, {"",nullptr},
#line 634 "auto/extension_to_mime_type.gperf"
      {"pptx", "application/vnd.openxmlformats-officedocument.presentationml.presentation"},
#line 135 "auto/extension_to_mime_type.gperf"
      {"cml", "chemical/x-cml"},
#line 925 "auto/extension_to_mime_type.gperf"
      {"wpd", "application/vnd.wordperfect"},
#line 767 "auto/extension_to_mime_type.gperf"
      {"ssf", "application/vnd.epson.ssf"},
#line 504 "auto/extension_to_mime_type.gperf"
      {"msf", "application/vnd.epson.msf"},
      {"",nullptr}, {"",nullptr},
#line 642 "auto/extension_to_mime_type.gperf"
      {"psf", "application/x-font-linux-psf"},
      {"",nullptr}, {"",nullptr}, {"",nullptr}, {"",nullptr},
      {"",nullptr},
#line 916 "auto/extension_to_mime_type.gperf"
      {"wml", "text/vnd.wap.wml"},
#line 936 "auto/extension_to_mime_type.gperf"
      {"x3d", "model/x3d+xml"},
#line 1005 "auto/extension_to_mime_type.gperf"
      {"zaz", "application/vnd.zzazz.deck+xml"},
      {"",nullptr},
#line 270 "auto/extension_to_mime_type.gperf"
      {"fsc", "application/vnd.fsc.weblaunch"},
#line 988 "auto/extension_to_mime_type.gperf"
      {"xspf", "application/xspf+xml"},
#line 417 "auto/extension_to_mime_type.gperf"
      {"lvp", "audio/vnd.lucent.voice"},
      {"",nullptr}, {"",nullptr}, {"",nullptr}, {"",nullptr},
      {"",nullptr}, {"",nullptr}, {"",nullptr}, {"",nullptr},
#line 666 "auto/extension_to_mime_type.gperf"
      {"rdf", "application/rdf+xml"},
      {"",nullptr}, {"",nullptr}, {"",nullptr},
#line 196 "auto/extension_to_mime_type.gperf"
      {"dsc", "text/prs.lines.tag"},
      {"",nullptr}, {"",nullptr}, {"",nullptr},
#line 896 "auto/extension_to_mime_type.gperf"
      {"w3d", "application/x-director"},
#line 641 "auto/extension_to_mime_type.gperf"
      {"psd", "image/vnd.adobe.photoshop"},
#line 197 "auto/extension_to_mime_type.gperf"
      {"dssc", "application/dssc+der"},
      {"",nullptr}, {"",nullptr}, {"",nullptr}, {"",nullptr},
      {"",nullptr}, {"",nullptr}, {"",nullptr}, {"",nullptr},
      {"",nullptr},
#line 495 "auto/extension_to_mime_type.gperf"
      {"mpt", "application/vnd.ms-project"},
      {"",nullptr}, {"",nullptr},
#line 632 "auto/extension_to_mime_type.gperf"
      {"ppt", "application/vnd.ms-powerpoint"},
#line 432 "auto/extension_to_mime_type.gperf"
      {"ma", "application/mathematica"},
      {"",nullptr},
#line 945 "auto/extension_to_mime_type.gperf"
      {"xbap", "application/x-ms-xbap"},
#line 109 "auto/extension_to_mime_type.gperf"
      {"cdmic", "application/cdmi-container"},
      {"",nullptr},
#line 26 "auto/extension_to_mime_type.gperf"
      {"aas", "application/x-authorware-seg"},
#line 749 "auto/extension_to_mime_type.gperf"
      {"smzip", "application/vnd.stepmania.package"},
#line 25 "auto/extension_to_mime_type.gperf"
      {"aam", "application/x-authorware-map"},
      {"",nullptr}, {"",nullptr},
#line 143 "auto/extension_to_mime_type.gperf"
      {"cpt", "application/mac-compactpro"},
      {"",nullptr},
#line 733 "auto/extension_to_mime_type.gperf"
      {"sis", "application/vnd.symbian.install"},
      {"",nullptr}, {"",nullptr},
#line 686 "auto/extension_to_mime_type.gperf"
      {"rpss", "application/vnd.nokia.radio-presets"},
      {"",nullptr},
#line 685 "auto/extension_to_mime_type.gperf"
      {"rp9", "application/vnd.cloanto.rp9"},
      {"",nullptr},
#line 110 "auto/extension_to_mime_type.gperf"
      {"cdmid", "application/cdmi-domain"},
#line 274 "auto/extension_to_mime_type.gperf"
      {"fvt", "video/vnd.fvt"},
      {"",nullptr}, {"",nullptr}, {"",nullptr}, {"",nullptr},
      {"",nullptr}, {"",nullptr},
#line 586 "auto/extension_to_mime_type.gperf"
      {"p7r", "application/x-pkcs7-certreqresp"},
#line 734 "auto/extension_to_mime_type.gperf"
      {"sisx", "application/vnd.symbian.install"},
      {"",nullptr}, {"",nullptr}, {"",nullptr},
#line 501 "auto/extension_to_mime_type.gperf"
      {"mscml", "application/mediaservercontrol+xml"},
      {"",nullptr}, {"",nullptr},
#line 24 "auto/extension_to_mime_type.gperf"
      {"aac", "audio/x-aac"},
      {"",nullptr},
#line 27 "auto/extension_to_mime_type.gperf"
      {"abw", "application/x-abiword"},
#line 401 "auto/extension_to_mime_type.gperf"
      {"lasxml", "application/vnd.las.las+xml"},
#line 755 "auto/extension_to_mime_type.gperf"
      {"spl", "application/x-futuresplash"},
      {"",nullptr}, {"",nullptr},
#line 99 "auto/extension_to_mime_type.gperf"
      {"cbr", "application/x-cbr"},
#line 177 "auto/extension_to_mime_type.gperf"
      {"dis", "application/vnd.mobius.dis"},
#line 979 "auto/extension_to_mime_type.gperf"
      {"xpl", "application/xproc+xml"},
#line 459 "auto/extension_to_mime_type.gperf"
      {"mif", "application/vnd.mif"},
#line 611 "auto/extension_to_mime_type.gperf"
      {"pic", "image/x-pict"},
#line 364 "auto/extension_to_mime_type.gperf"
      {"ivp", "application/vnd.immervision-ivp"},
#line 825 "auto/extension_to_mime_type.gperf"
      {"tsd", "application/timestamped-data"},
#line 960 "auto/extension_to_mime_type.gperf"
      {"xif", "image/vnd.xiff"},
#line 437 "auto/extension_to_mime_type.gperf"
      {"mar", "application/octet-stream"},
      {"",nullptr}, {"",nullptr},
#line 470 "auto/extension_to_mime_type.gperf"
      {"mmr", "image/vnd.fujixerox.edmics-mmr"},
#line 944 "auto/extension_to_mime_type.gperf"
      {"xar", "application/vnd.xara"},
      {"",nullptr},
#line 33 "auto/extension_to_mime_type.gperf"
      {"adp", "audio/adpcm"},
      {"",nullptr}, {"",nullptr},
#line 122 "auto/extension_to_mime_type.gperf"
      {"cif", "chemical/x-cif"},
#line 991 "auto/extension_to_mime_type.gperf"
      {"xvml", "application/xv+xml"},
#line 689 "auto/extension_to_mime_type.gperf"
      {"rs", "application/rls-services+xml"},
#line 821 "auto/extension_to_mime_type.gperf"
      {"tpt", "application/vnd.trid.tpt"},
      {"",nullptr},
#line 95 "auto/extension_to_mime_type.gperf"
      {"car", "application/vnd.curl.car"},
      {"",nullptr},
#line 926 "auto/extension_to_mime_type.gperf"
      {"wpl", "application/vnd.ms-wpl"},
#line 691 "auto/extension_to_mime_type.gperf"
      {"rss", "application/rss+xml"},
#line 152 "auto/extension_to_mime_type.gperf"
      {"cst", "application/x-director"},
#line 271 "auto/extension_to_mime_type.gperf"
      {"fst", "image/vnd.fst"},
      {"",nullptr}, {"",nullptr},
#line 161 "auto/extension_to_mime_type.gperf"
      {"dart", "application/vnd.dart"},
#line 175 "auto/extension_to_mime_type.gperf"
      {"dic", "text/x-c"},
#line 729 "auto/extension_to_mime_type.gperf"
      {"sid", "image/x-mrsid-image"},
#line 456 "auto/extension_to_mime_type.gperf"
      {"mid", "audio/midi"},
      {"",nullptr},
#line 941 "auto/extension_to_mime_type.gperf"
      {"x3dz", "model/x3d+xml"},
#line 857 "auto/extension_to_mime_type.gperf"
      {"uvs", "video/vnd.dece.sd"},
      {"",nullptr},
#line 855 "auto/extension_to_mime_type.gperf"
      {"uvm", "video/vnd.dece.mobile"},
      {"",nullptr}, {"",nullptr}, {"",nullptr},
#line 667 "auto/extension_to_mime_type.gperf"
      {"rdz", "application/vnd.data-vision.rdz"},
      {"",nullptr},
#line 856 "auto/extension_to_mime_type.gperf"
      {"uvp", "video/vnd.dece.pd"},
      {"",nullptr},
#line 875 "auto/extension_to_mime_type.gperf"
      {"uvx", "application/vnd.dece.unspecified"},
      {"",nullptr},
#line 1006 "auto/extension_to_mime_type.gperf"
      {"zip", "application/zip"},
      {"",nullptr}, {"",nullptr}, {"",nullptr},
#line 746 "auto/extension_to_mime_type.gperf"
      {"smi", "application/smil+xml"},
#line 736 "auto/extension_to_mime_type.gperf"
      {"sitx", "application/x-stuffitx"},
      {"",nullptr}, {"",nullptr},
#line 507 "auto/extension_to_mime_type.gperf"
      {"msl", "application/vnd.mobius.msl"},
      {"",nullptr},
#line 19 "auto/extension_to_mime_type.gperf"
      {"3ds", "image/x-3ds"},
#line 588 "auto/extension_to_mime_type.gperf"
      {"p8", "application/pkcs8"},
#line 985 "auto/extension_to_mime_type.gperf"
      {"xsl", "application/xml"},
      {"",nullptr},
#line 768 "auto/extension_to_mime_type.gperf"
      {"ssml", "application/ssml+xml"},
      {"",nullptr},
#line 820 "auto/extension_to_mime_type.gperf"
      {"tpl", "application/vnd.groove-tool-template"},
      {"",nullptr}, {"",nullptr},
#line 747 "auto/extension_to_mime_type.gperf"
      {"smil", "application/smil+xml"},
#line 851 "auto/extension_to_mime_type.gperf"
      {"uvf", "application/vnd.dece.data"},
#line 816 "auto/extension_to_mime_type.gperf"
      {"tif", "image/tiff"},
#line 428 "auto/extension_to_mime_type.gperf"
      {"m3u8", "application/vnd.apple.mpegurl"},
      {"",nullptr}, {"",nullptr},
#line 522 "auto/extension_to_mime_type.gperf"
      {"nbp", "application/vnd.wolfram.player"},
#line 800 "auto/extension_to_mime_type.gperf"
      {"tar", "application/x-tar"},
      {"",nullptr}, {"",nullptr},
#line 149 "auto/extension_to_mime_type.gperf"
      {"csml", "chemical/x-csml"},
      {"",nullptr},
#line 478 "auto/extension_to_mime_type.gperf"
      {"mp21", "application/mp21"},
      {"",nullptr}, {"",nullptr}, {"",nullptr},
#line 626 "auto/extension_to_mime_type.gperf"
      {"ppam", "application/vnd.ms-powerpoint.addin.macroenabled.12"},
      {"",nullptr}, {"",nullptr}, {"",nullptr},
#line 690 "auto/extension_to_mime_type.gperf"
      {"rsd", "application/rsd+xml"},
      {"",nullptr}, {"",nullptr}, {"",nullptr},
#line 817 "auto/extension_to_mime_type.gperf"
      {"tiff", "image/tiff"},
#line 765 "auto/extension_to_mime_type.gperf"
      {"ssdl", "application/ssdl+xml"},
#line 446 "auto/extension_to_mime_type.gperf"
      {"mdi", "image/vnd.ms-modi"},
      {"",nullptr}, {"",nullptr}, {"",nullptr}, {"",nullptr},
#line 850 "auto/extension_to_mime_type.gperf"
      {"uvd", "application/vnd.dece.data"},
      {"",nullptr}, {"",nullptr}, {"",nullptr}, {"",nullptr},
      {"",nullptr},
#line 735 "auto/extension_to_mime_type.gperf"
      {"sit", "application/x-stuffit"},
#line 661 "auto/extension_to_mime_type.gperf"
      {"ra", "audio/x-pn-realaudio"},
      {"",nullptr},
#line 687 "auto/extension_to_mime_type.gperf"
      {"rpst", "application/vnd.nokia.radio-preset"},
#line 441 "auto/extension_to_mime_type.gperf"
      {"mbox", "application/mbox"},
      {"",nullptr},
#line 243 "auto/extension_to_mime_type.gperf"
      {"f90", "text/x-fortran"},
#line 986 "auto/extension_to_mime_type.gperf"
      {"xslt", "application/xslt+xml"},
      {"",nullptr},
#line 540 "auto/extension_to_mime_type.gperf"
      {"oas", "application/vnd.fujitsu.oasys"},
      {"",nullptr}, {"",nullptr}, {"",nullptr}, {"",nullptr},
#line 673 "auto/extension_to_mime_type.gperf"
      {"ris", "application/x-research-info-systems"},
#line 931 "auto/extension_to_mime_type.gperf"
      {"wsdl", "application/wsdl+xml"},
#line 981 "auto/extension_to_mime_type.gperf"
      {"xpr", "application/vnd.is-xpr"},
      {"",nullptr}, {"",nullptr}, {"",nullptr}, {"",nullptr},
#line 1004 "auto/extension_to_mime_type.gperf"
      {"z8", "application/x-zmachine"},
#line 672 "auto/extension_to_mime_type.gperf"
      {"rip", "audio/vnd.rip"},
      {"",nullptr}, {"",nullptr}, {"",nullptr}, {"",nullptr},
#line 52 "auto/extension_to_mime_type.gperf"
      {"asm", "text/x-asm"},
      {"",nullptr},
#line 461 "auto/extension_to_mime_type.gperf"
      {"mj2", "video/mj2"},
      {"",nullptr},
#line 483 "auto/extension_to_mime_type.gperf"
      {"mp4s", "application/mp4"},
      {"",nullptr}, {"",nullptr}, {"",nullptr},
#line 54 "auto/extension_to_mime_type.gperf"
      {"asx", "video/x-ms-asf"},
      {"",nullptr},
#line 178 "auto/extension_to_mime_type.gperf"
      {"dist", "application/octet-stream"},
      {"",nullptr},
#line 462 "auto/extension_to_mime_type.gperf"
      {"mjp2", "video/mj2"},
      {"",nullptr},
#line 179 "auto/extension_to_mime_type.gperf"
      {"distz", "application/octet-stream"},
#line 409 "auto/extension_to_mime_type.gperf"
      {"list3820", "application/vnd.ibm.modcap"},
#line 244 "auto/extension_to_mime_type.gperf"
      {"fb2", "application/x-fictionbook+xml"},
#line 731 "auto/extension_to_mime_type.gperf"
      {"sil", "audio/silk"},
      {"",nullptr},
#line 671 "auto/extension_to_mime_type.gperf"
      {"rif", "application/reginfo+xml"},
      {"",nullptr}, {"",nullptr},
#line 541 "auto/extension_to_mime_type.gperf"
      {"obd", "application/x-msbinder"},
      {"",nullptr},
#line 663 "auto/extension_to_mime_type.gperf"
      {"rar", "application/x-rar-compressed"},
#line 552 "auto/extension_to_mime_type.gperf"
      {"ods", "application/vnd.oasis.opendocument.spreadsheet"},
      {"",nullptr},
#line 550 "auto/extension_to_mime_type.gperf"
      {"odm", "application/vnd.oasis.opendocument.text-master"},
#line 50 "auto/extension_to_mime_type.gperf"
      {"asc", "application/pgp-signature"},
#line 410 "auto/extension_to_mime_type.gperf"
      {"listafp", "application/vnd.ibm.modcap"},
#line 51 "auto/extension_to_mime_type.gperf"
      {"asf", "video/x-ms-asf"},
#line 978 "auto/extension_to_mime_type.gperf"
      {"xpi", "application/x-xpinstall"},
#line 124 "auto/extension_to_mime_type.gperf"
      {"cil", "application/vnd.ms-artgalry"},
#line 551 "auto/extension_to_mime_type.gperf"
      {"odp", "application/vnd.oasis.opendocument.presentation"},
      {"",nullptr}, {"",nullptr}, {"",nullptr}, {"",nullptr},
      {"",nullptr},
#line 858 "auto/extension_to_mime_type.gperf"
      {"uvt", "application/vnd.dece.ttml+xml"},
#line 350 "auto/extension_to_mime_type.gperf"
      {"iif", "application/vnd.shana.informed.interchange"},
      {"",nullptr}, {"",nullptr}, {"",nullptr}, {"",nullptr},
#line 876 "auto/extension_to_mime_type.gperf"
      {"uvz", "application/vnd.dece.zip"},
      {"",nullptr},
#line 702 "auto/extension_to_mime_type.gperf"
      {"scs", "application/scvp-cv-response"},
      {"",nullptr},
#line 700 "auto/extension_to_mime_type.gperf"
      {"scm", "application/vnd.lotus-screencam"},
      {"",nullptr}, {"",nullptr}, {"",nullptr}, {"",nullptr},
#line 358 "auto/extension_to_mime_type.gperf"
      {"ipfix", "application/ipfix"},
#line 545 "auto/extension_to_mime_type.gperf"
      {"odc", "application/vnd.oasis.opendocument.chart"},
#line 698 "auto/extension_to_mime_type.gperf"
      {"sc", "application/vnd.ibm.secure-container"},
#line 546 "auto/extension_to_mime_type.gperf"
      {"odf", "application/vnd.oasis.opendocument.formula"},
      {"",nullptr}, {"",nullptr}, {"",nullptr},
#line 599 "auto/extension_to_mime_type.gperf"
      {"pcx", "image/x-pcx"},
#line 648 "auto/extension_to_mime_type.gperf"
      {"pya", "audio/vnd.ms-playready.media.pya"},
#line 247 "auto/extension_to_mime_type.gperf"
      {"fcs", "application/vnd.isac.fcs"},
#line 782 "auto/extension_to_mime_type.gperf"
      {"sv4crc", "application/x-sv4crc"},
      {"",nullptr},
#line 204 "auto/extension_to_mime_type.gperf"
      {"dvi", "application/x-dvi"},
#line 108 "auto/extension_to_mime_type.gperf"
      {"cdmia", "application/cdmi-capability"},
#line 408 "auto/extension_to_mime_type.gperf"
      {"list", "text/plain"},
#line 678 "auto/extension_to_mime_type.gperf"
      {"rmi", "audio/midi"},
#line 439 "auto/extension_to_mime_type.gperf"
      {"mb", "application/mathematica"},
#line 102 "auto/extension_to_mime_type.gperf"
      {"cc", "text/x-c"},
#line 891 "auto/extension_to_mime_type.gperf"
      {"vss", "application/vnd.visio"},
      {"",nullptr},
#line 533 "auto/extension_to_mime_type.gperf"
      {"npx", "image/vnd.net-fpx"},
      {"",nullptr},
#line 903 "auto/extension_to_mime_type.gperf"
      {"wcm", "application/vnd.ms-works"},
#line 98 "auto/extension_to_mime_type.gperf"
      {"cba", "application/x-cbr"},
      {"",nullptr}, {"",nullptr},
#line 442 "auto/extension_to_mime_type.gperf"
      {"mc1", "application/vnd.medcalcdata"},
      {"",nullptr}, {"",nullptr},
#line 594 "auto/extension_to_mime_type.gperf"
      {"pcf", "application/x-font-pcf"},
      {"",nullptr},
#line 506 "auto/extension_to_mime_type.gperf"
      {"msi", "application/x-msdownload"},
      {"",nullptr},
#line 797 "auto/extension_to_mime_type.gperf"
      {"t3", "application/x-t3vm-image"},
      {"",nullptr}, {"",nullptr},
#line 999 "auto/extension_to_mime_type.gperf"
      {"z3", "application/x-zmachine"},
      {"",nullptr}, {"",nullptr},
#line 635 "auto/extension_to_mime_type.gperf"
      {"pqa", "application/vnd.palm"},
      {"",nullptr}, {"",nullptr},
#line 167 "auto/extension_to_mime_type.gperf"
      {"dd2", "application/vnd.oma.dd2+xml"},
      {"",nullptr}, {"",nullptr}, {"",nullptr}, {"",nullptr},
      {"",nullptr}, {"",nullptr}, {"",nullptr},
#line 890 "auto/extension_to_mime_type.gperf"
      {"vsf", "application/vnd.vsf"},
      {"",nullptr}, {"",nullptr}, {"",nullptr},
#line 76 "auto/extension_to_mime_type.gperf"
      {"bmp", "image/bmp"},
#line 699 "auto/extension_to_mime_type.gperf"
      {"scd", "application/x-msschedule"},
#line 443 "auto/extension_to_mime_type.gperf"
      {"mcd", "application/vnd.mcd"},
      {"",nullptr},
#line 18 "auto/extension_to_mime_type.gperf"
      {"3dml", "text/vnd.in3d.3dml"},
      {"",nullptr},
#line 913 "auto/extension_to_mime_type.gperf"
      {"wma", "audio/x-ms-wma"},
      {"",nullptr}, {"",nullptr}, {"",nullptr}, {"",nullptr},
      {"",nullptr}, {"",nullptr}, {"",nullptr},
#line 563 "auto/extension_to_mime_type.gperf"
      {"opf", "application/oebps-package+xml"},
#line 39 "auto/extension_to_mime_type.gperf"
      {"aif", "audio/x-aiff"},
      {"",nullptr}, {"",nullptr},
#line 893 "auto/extension_to_mime_type.gperf"
      {"vsw", "application/vnd.visio"},
#line 704 "auto/extension_to_mime_type.gperf"
      {"sda", "application/vnd.stardivision.draw"},
      {"",nullptr}, {"",nullptr},
#line 581 "auto/extension_to_mime_type.gperf"
      {"p10", "application/pkcs10"},
#line 477 "auto/extension_to_mime_type.gperf"
      {"mp2", "audio/mpeg"},
      {"",nullptr}, {"",nullptr},
#line 889 "auto/extension_to_mime_type.gperf"
      {"vsd", "application/vnd.visio"},
      {"",nullptr}, {"",nullptr},
#line 529 "auto/extension_to_mime_type.gperf"
      {"nml", "application/vnd.enliven"},
      {"",nullptr},
#line 427 "auto/extension_to_mime_type.gperf"
      {"m3u", "audio/x-mpegurl"},
      {"",nullptr}, {"",nullptr},
#line 69 "auto/extension_to_mime_type.gperf"
      {"bdm", "application/vnd.syncml.dm+wbxml"},
#line 40 "auto/extension_to_mime_type.gperf"
      {"aifc", "audio/x-aiff"},
      {"",nullptr},
#line 41 "auto/extension_to_mime_type.gperf"
      {"aiff", "audio/x-aiff"},
      {"",nullptr},
#line 282 "auto/extension_to_mime_type.gperf"
      {"gam", "application/x-tads"},
#line 708 "auto/extension_to_mime_type.gperf"
      {"sdkm", "application/vnd.solent.sdkm+xml"},
      {"",nullptr},
#line 299 "auto/extension_to_mime_type.gperf"
      {"gqs", "application/vnd.grafeq"},
#line 650 "auto/extension_to_mime_type.gperf"
      {"qam", "application/vnd.epson.quickanime"},
#line 534 "auto/extension_to_mime_type.gperf"
      {"nsc", "application/x-conference"},
      {"",nullptr},
#line 535 "auto/extension_to_mime_type.gperf"
      {"nsf", "application/vnd.lotus-notes"},
      {"",nullptr}, {"",nullptr}, {"",nullptr},
#line 294 "auto/extension_to_mime_type.gperf"
      {"gmx", "application/vnd.gmx"},
#line 553 "auto/extension_to_mime_type.gperf"
      {"odt", "application/vnd.oasis.opendocument.text"},
#line 1001 "auto/extension_to_mime_type.gperf"
      {"z5", "application/x-zmachine"},
#line 940 "auto/extension_to_mime_type.gperf"
      {"x3dvz", "model/x3d+vrml"},
      {"",nullptr}, {"",nullptr},
#line 104 "auto/extension_to_mime_type.gperf"
      {"ccxml", "application/ccxml+xml"},
#line 883 "auto/extension_to_mime_type.gperf"
      {"vis", "application/vnd.visionary"},
#line 176 "auto/extension_to_mime_type.gperf"
      {"dir", "application/x-director"},
      {"",nullptr}, {"",nullptr},
#line 935 "auto/extension_to_mime_type.gperf"
      {"x32", "application/x-authorware-bin"},
      {"",nullptr}, {"",nullptr},
#line 68 "auto/extension_to_mime_type.gperf"
      {"bdf", "application/x-font-bdf"},
#line 44 "auto/extension_to_mime_type.gperf"
      {"ami", "application/vnd.amiga.ami"},
      {"",nullptr},
#line 281 "auto/extension_to_mime_type.gperf"
      {"gac", "application/vnd.groove-account"},
      {"",nullptr}, {"",nullptr}, {"",nullptr}, {"",nullptr},
#line 568 "auto/extension_to_mime_type.gperf"
      {"osf", "application/vnd.yamaha.openscoreformat"},
#line 547 "auto/extension_to_mime_type.gperf"
      {"odft", "application/vnd.oasis.opendocument.formula-template"},
#line 298 "auto/extension_to_mime_type.gperf"
      {"gqf", "application/vnd.grafeq"},
      {"",nullptr}, {"",nullptr},
#line 597 "auto/extension_to_mime_type.gperf"
      {"pct", "image/x-pict"},
      {"",nullptr}, {"",nullptr}, {"",nullptr}, {"",nullptr},
      {"",nullptr}, {"",nullptr},
#line 123 "auto/extension_to_mime_type.gperf"
      {"cii", "application/vnd.anser-web-certificate-issue-initiation"},
      {"",nullptr},
#line 218 "auto/extension_to_mime_type.gperf"
      {"emf", "application/x-msmetafile"},
#line 277 "auto/extension_to_mime_type.gperf"
      {"fzs", "application/vnd.fuzzysheet"},
#line 103 "auto/extension_to_mime_type.gperf"
      {"cct", "application/x-director"},
      {"",nullptr},
#line 997 "auto/extension_to_mime_type.gperf"
      {"z1", "application/x-zmachine"},
      {"",nullptr}, {"",nullptr}, {"",nullptr}, {"",nullptr},
#line 1008 "auto/extension_to_mime_type.gperf"
      {"zirz", "application/vnd.zul"},
#line 213 "auto/extension_to_mime_type.gperf"
      {"edm", "application/vnd.novadigm.edm"},
#line 1007 "auto/extension_to_mime_type.gperf"
      {"zir", "application/vnd.zul"},
#line 892 "auto/extension_to_mime_type.gperf"
      {"vst", "application/vnd.visio"},
      {"",nullptr},
#line 707 "auto/extension_to_mime_type.gperf"
      {"sdkd", "application/vnd.solent.sdkm+xml"},
      {"",nullptr},
#line 649 "auto/extension_to_mime_type.gperf"
      {"pyv", "video/vnd.ms-playready.media.pyv"},
      {"",nullptr},
#line 214 "auto/extension_to_mime_type.gperf"
      {"edx", "application/vnd.novadigm.edx"},
      {"",nullptr}, {"",nullptr},
#line 457 "auto/extension_to_mime_type.gperf"
      {"midi", "audio/midi"},
#line 341 "auto/extension_to_mime_type.gperf"
      {"ics", "text/calendar"},
#line 111 "auto/extension_to_mime_type.gperf"
      {"cdmio", "application/cdmi-object"},
#line 339 "auto/extension_to_mime_type.gperf"
      {"icm", "application/vnd.iccprofile"},
      {"",nullptr}, {"",nullptr},
#line 788 "auto/extension_to_mime_type.gperf"
      {"swf", "application/x-shockwave-flash"},
#line 513 "auto/extension_to_mime_type.gperf"
      {"mwf", "application/vnd.mfer"},
      {"",nullptr}, {"",nullptr},
#line 43 "auto/extension_to_mime_type.gperf"
      {"ait", "application/vnd.dvb.ait"},
#line 394 "auto/extension_to_mime_type.gperf"
      {"kpxx", "application/vnd.ds-keypoint"},
#line 426 "auto/extension_to_mime_type.gperf"
      {"m3a", "audio/mpeg"},
#line 163 "auto/extension_to_mime_type.gperf"
      {"davmount", "application/davmount+xml"},
#line 595 "auto/extension_to_mime_type.gperf"
      {"pcl", "application/vnd.hp-pcl"},
#line 756 "auto/extension_to_mime_type.gperf"
      {"spot", "text/vnd.in3d.spot"},
#line 66 "auto/extension_to_mime_type.gperf"
      {"bat", "application/x-msdownload"},
      {"",nullptr}, {"",nullptr},
#line 246 "auto/extension_to_mime_type.gperf"
      {"fcdt", "application/vnd.adobe.formscentral.fcdt"},
#line 748 "auto/extension_to_mime_type.gperf"
      {"smv", "video/x-smv"},
#line 48 "auto/extension_to_mime_type.gperf"
      {"apr", "application/vnd.lotus-approach"},
#line 869 "auto/extension_to_mime_type.gperf"
      {"uvvs", "video/vnd.dece.sd"},
      {"",nullptr},
#line 867 "auto/extension_to_mime_type.gperf"
      {"uvvm", "video/vnd.dece.mobile"},
#line 388 "auto/extension_to_mime_type.gperf"
      {"kmz", "application/vnd.google-earth.kmz"},
#line 596 "auto/extension_to_mime_type.gperf"
      {"pclxl", "application/vnd.hp-pclxl"},
#line 180 "auto/extension_to_mime_type.gperf"
      {"djv", "image/vnd.djvu"},
#line 854 "auto/extension_to_mime_type.gperf"
      {"uvi", "image/vnd.dece.graphic"},
#line 22 "auto/extension_to_mime_type.gperf"
      {"7z", "application/x-7z-compressed"},
#line 868 "auto/extension_to_mime_type.gperf"
      {"uvvp", "video/vnd.dece.pd"},
#line 337 "auto/extension_to_mime_type.gperf"
      {"icc", "application/vnd.iccprofile"},
#line 873 "auto/extension_to_mime_type.gperf"
      {"uvvx", "application/vnd.dece.unspecified"},
#line 994 "auto/extension_to_mime_type.gperf"
      {"xz", "application/x-xz"},
      {"",nullptr},
#line 509 "auto/extension_to_mime_type.gperf"
      {"mts", "model/vnd.mts"},
      {"",nullptr},
#line 205 "auto/extension_to_mime_type.gperf"
      {"dwf", "model/vnd.dwf"},
      {"",nullptr},
#line 38 "auto/extension_to_mime_type.gperf"
      {"ai", "application/postscript"},
      {"",nullptr},
#line 992 "auto/extension_to_mime_type.gperf"
      {"xwd", "image/x-xwindowdump"},
#line 653 "auto/extension_to_mime_type.gperf"
      {"qps", "application/vnd.publishare-delta-tree"},
#line 156 "auto/extension_to_mime_type.gperf"
      {"cww", "application/prs.cww"},
#line 898 "auto/extension_to_mime_type.gperf"
      {"wav", "audio/x-wav"},
      {"",nullptr}, {"",nullptr},
#line 920 "auto/extension_to_mime_type.gperf"
      {"wmv", "video/x-ms-wmv"},
#line 297 "auto/extension_to_mime_type.gperf"
      {"gpx", "application/gpx+xml"},
      {"",nullptr}, {"",nullptr}, {"",nullptr},
#line 224 "auto/extension_to_mime_type.gperf"
      {"eps", "application/postscript"},
      {"",nullptr},
#line 863 "auto/extension_to_mime_type.gperf"
      {"uvvf", "application/vnd.dece.data"},
      {"",nullptr},
#line 395 "auto/extension_to_mime_type.gperf"
      {"ksp", "application/vnd.kde.kspread"},
      {"",nullptr}, {"",nullptr},
#line 565 "auto/extension_to_mime_type.gperf"
      {"oprc", "application/vnd.palm"},
#line 564 "auto/extension_to_mime_type.gperf"
      {"opml", "text/x-opml"},
      {"",nullptr}, {"",nullptr},
#line 387 "auto/extension_to_mime_type.gperf"
      {"kml", "application/vnd.google-earth.kml+xml"},
#line 770 "auto/extension_to_mime_type.gperf"
      {"stc", "application/vnd.sun.xml.calc.template"},
#line 200 "auto/extension_to_mime_type.gperf"
      {"dts", "audio/vnd.dts"},
#line 772 "auto/extension_to_mime_type.gperf"
      {"stf", "application/vnd.wt.stf"},
#line 365 "auto/extension_to_mime_type.gperf"
      {"ivu", "application/vnd.immervision-ivu"},
      {"",nullptr}, {"",nullptr}, {"",nullptr},
#line 418 "auto/extension_to_mime_type.gperf"
      {"lwp", "application/vnd.lotus-wordpro"},
#line 61 "auto/extension_to_mime_type.gperf"
      {"avi", "video/x-msvideo"},
      {"",nullptr}, {"",nullptr}, {"",nullptr}, {"",nullptr},
#line 802 "auto/extension_to_mime_type.gperf"
      {"tcl", "application/x-tcl"},
#line 831 "auto/extension_to_mime_type.gperf"
      {"twds", "application/vnd.simtech-mindmapper"},
      {"",nullptr},
#line 272 "auto/extension_to_mime_type.gperf"
      {"ftc", "application/vnd.fluxtime.clip"},
#line 688 "auto/extension_to_mime_type.gperf"
      {"rq", "application/sparql-query"},
#line 480 "auto/extension_to_mime_type.gperf"
      {"mp3", "audio/mpeg"},
      {"",nullptr},
#line 862 "auto/extension_to_mime_type.gperf"
      {"uvvd", "application/vnd.dece.data"},
      {"",nullptr}, {"",nullptr},
#line 221 "auto/extension_to_mime_type.gperf"
      {"emz", "application/x-msmetafile"},
#line 777 "auto/extension_to_mime_type.gperf"
      {"stw", "application/vnd.sun.xml.writer.template"},
#line 566 "auto/extension_to_mime_type.gperf"
      {"opus", "audio/ogg"},
      {"",nullptr}, {"",nullptr},
#line 481 "auto/extension_to_mime_type.gperf"
      {"mp4", "video/mp4"},
#line 28 "auto/extension_to_mime_type.gperf"
      {"ac", "application/pkix-attr-cert"},
      {"",nullptr},
#line 769 "auto/extension_to_mime_type.gperf"
      {"st", "application/vnd.sailingtracker.track"},
#line 771 "auto/extension_to_mime_type.gperf"
      {"std", "application/vnd.sun.xml.draw.template"},
      {"",nullptr}, {"",nullptr}, {"",nullptr}, {"",nullptr},
      {"",nullptr},
#line 593 "auto/extension_to_mime_type.gperf"
      {"pcap", "application/vnd.tcpdump.pcap"},
#line 830 "auto/extension_to_mime_type.gperf"
      {"twd", "application/vnd.simtech-mindmapper"},
#line 226 "auto/extension_to_mime_type.gperf"
      {"es", "application/ecmascript"},
#line 527 "auto/extension_to_mime_type.gperf"
      {"nitf", "application/vnd.nitf"},
#line 859 "auto/extension_to_mime_type.gperf"
      {"uvu", "video/vnd.uvvu.mp4"},
      {"",nullptr},
#line 779 "auto/extension_to_mime_type.gperf"
      {"sus", "application/vnd.sus-calendar"},
#line 510 "auto/extension_to_mime_type.gperf"
      {"mus", "application/vnd.musician"},
      {"",nullptr},
#line 293 "auto/extension_to_mime_type.gperf"
      {"gml", "application/gml+xml"},
#line 402 "auto/extension_to_mime_type.gperf"
      {"latex", "application/x-latex"},
      {"",nullptr},
#line 29 "auto/extension_to_mime_type.gperf"
      {"acc", "application/vnd.americandynamics.acc"},
      {"",nullptr}, {"",nullptr}, {"",nullptr},
#line 436 "auto/extension_to_mime_type.gperf"
      {"man", "text/troff"},
      {"",nullptr}, {"",nullptr}, {"",nullptr},
#line 780 "auto/extension_to_mime_type.gperf"
      {"susp", "application/vnd.sus-calendar"},
#line 181 "auto/extension_to_mime_type.gperf"
      {"djvu", "image/vnd.djvu"},
#line 393 "auto/extension_to_mime_type.gperf"
      {"kpt", "application/vnd.kde.kpresenter"},
#line 219 "auto/extension_to_mime_type.gperf"
      {"eml", "message/rfc822"},
#line 280 "auto/extension_to_mime_type.gperf"
      {"g3w", "application/vnd.geospace"},
#line 199 "auto/extension_to_mime_type.gperf"
      {"dtd", "application/xml-dtd"},
#line 305 "auto/extension_to_mime_type.gperf"
      {"gsf", "application/x-font-ghostscript"},
      {"",nullptr}, {"",nullptr},
#line 827 "auto/extension_to_mime_type.gperf"
      {"ttc", "font/collection"},
      {"",nullptr},
#line 828 "auto/extension_to_mime_type.gperf"
      {"ttf", "font/ttf"},
      {"",nullptr}, {"",nullptr}, {"",nullptr},
#line 938 "auto/extension_to_mime_type.gperf"
      {"x3dbz", "model/x3d+binary"},
      {"",nullptr}, {"",nullptr}, {"",nullptr}, {"",nullptr},
#line 229 "auto/extension_to_mime_type.gperf"
      {"esf", "application/vnd.epson.esf"},
      {"",nullptr}, {"",nullptr}, {"",nullptr}, {"",nullptr},
      {"",nullptr},
#line 549 "auto/extension_to_mime_type.gperf"
      {"odi", "application/vnd.oasis.opendocument.image"},
      {"",nullptr}, {"",nullptr},
#line 285 "auto/extension_to_mime_type.gperf"
      {"gdl", "model/vnd.gdl"},
      {"",nullptr}, {"",nullptr},
#line 202 "auto/extension_to_mime_type.gperf"
      {"dump", "application/octet-stream"},
#line 416 "auto/extension_to_mime_type.gperf"
      {"ltf", "application/vnd.frogans.ltf"},
#line 870 "auto/extension_to_mime_type.gperf"
      {"uvvt", "application/vnd.dece.ttml+xml"},
#line 835 "auto/extension_to_mime_type.gperf"
      {"u32", "application/x-authorware-bin"},
#line 881 "auto/extension_to_mime_type.gperf"
      {"vcs", "text/x-vcalendar"},
#line 165 "auto/extension_to_mime_type.gperf"
      {"dcr", "application/x-director"},
      {"",nullptr}, {"",nullptr},
#line 874 "auto/extension_to_mime_type.gperf"
      {"uvvz", "application/vnd.dece.zip"},
      {"",nullptr}, {"",nullptr},
#line 801 "auto/extension_to_mime_type.gperf"
      {"tcap", "application/vnd.3gpp2.tcap"},
      {"",nullptr}, {"",nullptr},
#line 882 "auto/extension_to_mime_type.gperf"
      {"vcx", "application/vnd.vcx"},
#line 112 "auto/extension_to_mime_type.gperf"
      {"cdmiq", "application/cdmi-queue"},
      {"",nullptr},
#line 643 "auto/extension_to_mime_type.gperf"
      {"pskcxml", "application/pskc+xml"},
#line 42 "auto/extension_to_mime_type.gperf"
      {"air", "application/vnd.adobe.air-application-installer-package+zip"},
      {"",nullptr}, {"",nullptr}, {"",nullptr}, {"",nullptr},
      {"",nullptr},
#line 383 "auto/extension_to_mime_type.gperf"
      {"kar", "audio/midi"},
      {"",nullptr}, {"",nullptr},
#line 520 "auto/extension_to_mime_type.gperf"
      {"n3", "text/n3"},
      {"",nullptr}, {"",nullptr}, {"",nullptr}, {"",nullptr},
      {"",nullptr}, {"",nullptr}, {"",nullptr}, {"",nullptr},
#line 879 "auto/extension_to_mime_type.gperf"
      {"vcf", "text/x-vcard"},
#line 292 "auto/extension_to_mime_type.gperf"
      {"gim", "application/vnd.groove-identity-message"},
#line 693 "auto/extension_to_mime_type.gperf"
      {"rtx", "text/richtext"},
#line 786 "auto/extension_to_mime_type.gperf"
      {"svgz", "image/svg+xml"},
#line 849 "auto/extension_to_mime_type.gperf"
      {"uva", "audio/vnd.dece.audio"},
      {"",nullptr}, {"",nullptr}, {"",nullptr},
#line 153 "auto/extension_to_mime_type.gperf"
      {"csv", "text/csv"},
      {"",nullptr}, {"",nullptr},
#line 939 "auto/extension_to_mime_type.gperf"
      {"x3dv", "model/x3d+vrml"},
      {"",nullptr}, {"",nullptr},
#line 283 "auto/extension_to_mime_type.gperf"
      {"gbr", "application/rpki-ghostbusters"},
#line 479 "auto/extension_to_mime_type.gperf"
      {"mp2a", "audio/mpeg"},
      {"",nullptr}, {"",nullptr}, {"",nullptr},
#line 538 "auto/extension_to_mime_type.gperf"
      {"oa2", "application/vnd.fujitsu.oasys2"},
      {"",nullptr},
#line 363 "auto/extension_to_mime_type.gperf"
      {"itp", "application/vnd.shana.informed.formtemplate"},
#line 775 "auto/extension_to_mime_type.gperf"
      {"stl", "application/vnd.ms-pki.stl"},
#line 846 "auto/extension_to_mime_type.gperf"
      {"ustar", "application/x-ustar"},
#line 692 "auto/extension_to_mime_type.gperf"
      {"rtf", "application/rtf"},
      {"",nullptr},
#line 64 "auto/extension_to_mime_type.gperf"
      {"azs", "application/vnd.airzip.filesecure.azs"},
#line 523 "auto/extension_to_mime_type.gperf"
      {"nc", "application/x-netcdf"},
#line 524 "auto/extension_to_mime_type.gperf"
      {"ncx", "application/x-dtbncx+xml"},
      {"",nullptr},
#line 878 "auto/extension_to_mime_type.gperf"
      {"vcd", "application/x-cdlink"},
#line 291 "auto/extension_to_mime_type.gperf"
      {"gif", "image/gif"},
      {"",nullptr},
#line 75 "auto/extension_to_mime_type.gperf"
      {"bmi", "application/vnd.bmi"},
      {"",nullptr},
#line 558 "auto/extension_to_mime_type.gperf"
      {"omdoc", "application/omdoc+xml"},
#line 493 "auto/extension_to_mime_type.gperf"
      {"mpn", "application/vnd.mophun.application"},
#line 47 "auto/extension_to_mime_type.gperf"
      {"application", "application/x-ms-application"},
      {"",nullptr},
#line 85 "auto/extension_to_mime_type.gperf"
      {"c11amc", "application/vnd.cluetrust.cartomobile-config"},
#line 521 "auto/extension_to_mime_type.gperf"
      {"nb", "application/mathematica"},
      {"",nullptr},
#line 423 "auto/extension_to_mime_type.gperf"
      {"m21", "application/mp21"},
#line 798 "auto/extension_to_mime_type.gperf"
      {"taglet", "application/vnd.mynfc"},
      {"",nullptr}, {"",nullptr},
#line 799 "auto/extension_to_mime_type.gperf"
      {"tao", "application/vnd.tao.intent-module-archive"},
      {"",nullptr}, {"",nullptr},
#line 818 "auto/extension_to_mime_type.gperf"
      {"tmo", "application/vnd.tmobile-livetv"},
      {"",nullptr},
#line 329 "auto/extension_to_mime_type.gperf"
      {"hqx", "application/mac-binhex40"},
      {"",nullptr}, {"",nullptr}, {"",nullptr}, {"",nullptr},
      {"",nullptr},
#line 62 "auto/extension_to_mime_type.gperf"
      {"aw", "application/applixware"},
#line 63 "auto/extension_to_mime_type.gperf"
      {"azf", "application/vnd.airzip.filesecure.azf"},
      {"",nullptr},
#line 826 "auto/extension_to_mime_type.gperf"
      {"tsv", "text/tab-separated-values"},
      {"",nullptr}, {"",nullptr}, {"",nullptr}, {"",nullptr},
#line 216 "auto/extension_to_mime_type.gperf"
      {"ei6", "application/vnd.pg.osasli"},
      {"",nullptr}, {"",nullptr}, {"",nullptr}, {"",nullptr},
      {"",nullptr}, {"",nullptr}, {"",nullptr}, {"",nullptr},
      {"",nullptr}, {"",nullptr}, {"",nullptr}, {"",nullptr},
      {"",nullptr}, {"",nullptr},
#line 65 "auto/extension_to_mime_type.gperf"
      {"azw", "application/vnd.amazon.ebook"},
      {"",nullptr}, {"",nullptr}, {"",nullptr}, {"",nullptr},
#line 644 "auto/extension_to_mime_type.gperf"
      {"ptid", "application/vnd.pvi.ptid1"},
      {"",nullptr}, {"",nullptr}, {"",nullptr},
#line 829 "auto/extension_to_mime_type.gperf"
      {"ttl", "text/turtle"},
      {"",nullptr}, {"",nullptr}, {"",nullptr}, {"",nullptr},
#line 703 "auto/extension_to_mime_type.gperf"
      {"scurl", "text/vnd.curl.scurl"},
#line 444 "auto/extension_to_mime_type.gperf"
      {"mcurl", "text/vnd.curl.mcurl"},
#line 392 "auto/extension_to_mime_type.gperf"
      {"kpr", "application/vnd.kde.kpresenter"},
      {"",nullptr},
#line 598 "auto/extension_to_mime_type.gperf"
      {"pcurl", "application/vnd.curl.pcurl"},
#line 618 "auto/extension_to_mime_type.gperf"
      {"pls", "application/pls+xml"},
#line 966 "auto/extension_to_mime_type.gperf"
      {"xls", "application/vnd.ms-excel"},
#line 989 "auto/extension_to_mime_type.gperf"
      {"xul", "application/vnd.mozilla.xul+xml"},
#line 965 "auto/extension_to_mime_type.gperf"
      {"xlm", "application/vnd.ms-excel"},
#line 482 "auto/extension_to_mime_type.gperf"
      {"mp4a", "audio/mp4"},
#line 467 "auto/extension_to_mime_type.gperf"
      {"mlp", "application/vnd.dolby.mlp"},
      {"",nullptr},
#line 59 "auto/extension_to_mime_type.gperf"
      {"atx", "application/vnd.antix.game-component"},
      {"",nullptr},
#line 968 "auto/extension_to_mime_type.gperf"
      {"xlsm", "application/vnd.ms-excel.sheet.macroenabled.12"},
      {"",nullptr}, {"",nullptr},
#line 319 "auto/extension_to_mime_type.gperf"
      {"hdf", "application/x-hdf"},
      {"",nullptr}, {"",nullptr}, {"",nullptr}, {"",nullptr},
#line 969 "auto/extension_to_mime_type.gperf"
      {"xlsx", "application/vnd.openxmlformats-officedocument.spreadsheetml.sheet"},
      {"",nullptr},
#line 132 "auto/extension_to_mime_type.gperf"
      {"clp", "application/x-msclip"},
#line 162 "auto/extension_to_mime_type.gperf"
      {"dataless", "application/vnd.fdsn.seed"},
      {"",nullptr},
#line 263 "auto/extension_to_mime_type.gperf"
      {"flx", "text/vnd.fmi.flexstor"},
#line 789 "auto/extension_to_mime_type.gperf"
      {"swi", "application/vnd.aristanetworks.swi"},
      {"",nullptr},
#line 543 "auto/extension_to_mime_type.gperf"
      {"oda", "application/oda"},
#line 166 "auto/extension_to_mime_type.gperf"
      {"dcurl", "text/vnd.curl.dcurl"},
#line 55 "auto/extension_to_mime_type.gperf"
      {"atc", "application/vnd.acucorp"},
      {"",nullptr}, {"",nullptr},
#line 616 "auto/extension_to_mime_type.gperf"
      {"plc", "application/vnd.mobius.plc"},
#line 963 "auto/extension_to_mime_type.gperf"
      {"xlc", "application/vnd.ms-excel"},
#line 617 "auto/extension_to_mime_type.gperf"
      {"plf", "application/vnd.pocketlearn"},
#line 964 "auto/extension_to_mime_type.gperf"
      {"xlf", "application/x-xliff+xml"},
      {"",nullptr},
#line 741 "auto/extension_to_mime_type.gperf"
      {"sldm", "application/vnd.ms-powerpoint.slide.macroenabled.12"},
#line 154 "auto/extension_to_mime_type.gperf"
      {"cu", "application/cu-seeme"},
#line 764 "auto/extension_to_mime_type.gperf"
      {"srx", "application/sparql-results+xml"},
      {"",nullptr},
#line 465 "auto/extension_to_mime_type.gperf"
      {"mks", "video/x-matroska"},
#line 738 "auto/extension_to_mime_type.gperf"
      {"skm", "application/vnd.koan"},
#line 776 "auto/extension_to_mime_type.gperf"
      {"str", "application/vnd.pg.format"},
      {"",nullptr},
#line 742 "auto/extension_to_mime_type.gperf"
      {"sldx", "application/vnd.openxmlformats-officedocument.presentationml.slide"},
      {"",nullptr},
#line 328 "auto/extension_to_mime_type.gperf"
      {"hps", "application/vnd.hp-hps"},
#line 739 "auto/extension_to_mime_type.gperf"
      {"skp", "application/vnd.koan"},
      {"",nullptr}, {"",nullptr}, {"",nullptr},
#line 86 "auto/extension_to_mime_type.gperf"
      {"c11amz", "application/vnd.cluetrust.cartomobile-config-pkg"},
      {"",nullptr}, {"",nullptr},
#line 860 "auto/extension_to_mime_type.gperf"
      {"uvv", "video/vnd.dece.video"},
#line 335 "auto/extension_to_mime_type.gperf"
      {"hvs", "application/vnd.yamaha.hv-script"},
#line 973 "auto/extension_to_mime_type.gperf"
      {"xlw", "application/vnd.ms-excel"},
      {"",nullptr},
#line 761 "auto/extension_to_mime_type.gperf"
      {"src", "application/x-wais-source"},
#line 498 "auto/extension_to_mime_type.gperf"
      {"mrc", "application/marc"},
#line 499 "auto/extension_to_mime_type.gperf"
      {"mrcx", "application/marcxml+xml"},
#line 582 "auto/extension_to_mime_type.gperf"
      {"p12", "application/x-pkcs12"},
#line 636 "auto/extension_to_mime_type.gperf"
      {"prc", "application/x-mobipocket-ebook"},
#line 334 "auto/extension_to_mime_type.gperf"
      {"hvp", "application/vnd.yamaha.hv-voice"},
#line 638 "auto/extension_to_mime_type.gperf"
      {"prf", "application/pics-rules"},
#line 46 "auto/extension_to_mime_type.gperf"
      {"appcache", "text/cache-manifest"},
#line 911 "auto/extension_to_mime_type.gperf"
      {"wks", "application/vnd.ms-works"},
#line 262 "auto/extension_to_mime_type.gperf"
      {"flw", "application/vnd.kde.kivio"},
      {"",nullptr}, {"",nullptr}, {"",nullptr},
#line 497 "auto/extension_to_mime_type.gperf"
      {"mqy", "application/vnd.mobius.mqy"},
#line 866 "auto/extension_to_mime_type.gperf"
      {"uvvi", "image/vnd.dece.graphic"},
      {"",nullptr}, {"",nullptr}, {"",nullptr}, {"",nullptr},
#line 971 "auto/extension_to_mime_type.gperf"
      {"xltm", "application/vnd.ms-excel.template.macroenabled.12"},
      {"",nullptr},
#line 309 "auto/extension_to_mime_type.gperf"
      {"gv", "text/vnd.graphviz"},
      {"",nullptr}, {"",nullptr},
#line 279 "auto/extension_to_mime_type.gperf"
      {"g3", "image/g3fax"},
#line 715 "auto/extension_to_mime_type.gperf"
      {"semf", "application/vnd.semf"},
#line 773 "auto/extension_to_mime_type.gperf"
      {"sti", "application/vnd.sun.xml.impress.template"},
#line 972 "auto/extension_to_mime_type.gperf"
      {"xltx", "application/vnd.openxmlformats-officedocument.spreadsheetml.template"},
      {"",nullptr},
#line 539 "auto/extension_to_mime_type.gperf"
      {"oa3", "application/vnd.fujitsu.oasys3"},
      {"",nullptr},
#line 583 "auto/extension_to_mime_type.gperf"
      {"p7b", "application/x-pkcs7-certificates"},
#line 440 "auto/extension_to_mime_type.gperf"
      {"mbk", "application/vnd.mobius.mbk"},
      {"",nullptr}, {"",nullptr},
#line 32 "auto/extension_to_mime_type.gperf"
      {"acutc", "application/vnd.acucorp"},
#line 847 "auto/extension_to_mime_type.gperf"
      {"utz", "application/vnd.uiq.theme"},
      {"",nullptr}, {"",nullptr},
#line 824 "auto/extension_to_mime_type.gperf"
      {"trm", "application/x-msterminal"},
      {"",nullptr},
#line 511 "auto/extension_to_mime_type.gperf"
      {"musicxml", "application/vnd.recordare.musicxml+xml"},
#line 273 "auto/extension_to_mime_type.gperf"
      {"fti", "application/vnd.anser-web-funds-transfer-initiation"},
#line 737 "auto/extension_to_mime_type.gperf"
      {"skd", "application/vnd.koan"},
      {"",nullptr},
#line 781 "auto/extension_to_mime_type.gperf"
      {"sv4cpio", "application/x-sv4cpio"},
      {"",nullptr},
#line 144 "auto/extension_to_mime_type.gperf"
      {"crd", "application/x-mscardfile"},
      {"",nullptr},
#line 170 "auto/extension_to_mime_type.gperf"
      {"def", "text/plain"},
      {"",nullptr}, {"",nullptr}, {"",nullptr}, {"",nullptr},
      {"",nullptr},
#line 714 "auto/extension_to_mime_type.gperf"
      {"semd", "application/vnd.semd"},
      {"",nullptr}, {"",nullptr},
#line 317 "auto/extension_to_mime_type.gperf"
      {"hal", "application/vnd.hal+xml"},
#line 333 "auto/extension_to_mime_type.gperf"
      {"hvd", "application/vnd.yamaha.hv-dic"},
#line 451 "auto/extension_to_mime_type.gperf"
      {"mets", "application/mets+xml"},
#line 415 "auto/extension_to_mime_type.gperf"
      {"lrm", "application/vnd.ms-lrm"},
#line 164 "auto/extension_to_mime_type.gperf"
      {"dbk", "application/docbook+xml"},
#line 806 "auto/extension_to_mime_type.gperf"
      {"tex", "application/x-tex"},
      {"",nullptr},
#line 115 "auto/extension_to_mime_type.gperf"
      {"cdy", "application/vnd.cinderella"},
      {"",nullptr}, {"",nullptr},
#line 92 "auto/extension_to_mime_type.gperf"
      {"cab", "application/vnd.ms-cab-compressed"},
      {"",nullptr}, {"",nullptr}, {"",nullptr}, {"",nullptr},
#line 743 "auto/extension_to_mime_type.gperf"
      {"slt", "application/vnd.epson.salt"},
      {"",nullptr},
#line 405 "auto/extension_to_mime_type.gperf"
      {"les", "application/vnd.hhe.lesson-player"},
      {"",nullptr}, {"",nullptr},
#line 970 "auto/extension_to_mime_type.gperf"
      {"xlt", "application/vnd.ms-excel"},
      {"",nullptr}, {"",nullptr}, {"",nullptr}, {"",nullptr},
      {"",nullptr}, {"",nullptr}, {"",nullptr}, {"",nullptr},
      {"",nullptr}, {"",nullptr}, {"",nullptr},
#line 576 "auto/extension_to_mime_type.gperf"
      {"ots", "application/vnd.oasis.opendocument.spreadsheet-template"},
#line 414 "auto/extension_to_mime_type.gperf"
      {"lrf", "application/octet-stream"},
#line 871 "auto/extension_to_mime_type.gperf"
      {"uvvu", "video/vnd.uvvu.mp4"},
      {"",nullptr}, {"",nullptr},
#line 484 "auto/extension_to_mime_type.gperf"
      {"mp4v", "video/mp4"},
#line 536 "auto/extension_to_mime_type.gperf"
      {"ntf", "application/vnd.nitf"},
#line 445 "auto/extension_to_mime_type.gperf"
      {"mdb", "application/x-msaccess"},
#line 575 "auto/extension_to_mime_type.gperf"
      {"otp", "application/vnd.oasis.opendocument.presentation-template"},
      {"",nullptr},
#line 600 "auto/extension_to_mime_type.gperf"
      {"pdb", "application/vnd.palm"},
      {"",nullptr}, {"",nullptr}, {"",nullptr},
#line 762 "auto/extension_to_mime_type.gperf"
      {"srt", "application/x-subrip"},
      {"",nullptr}, {"",nullptr}, {"",nullptr}, {"",nullptr},
#line 155 "auto/extension_to_mime_type.gperf"
      {"curl", "text/vnd.curl"},
      {"",nullptr}, {"",nullptr},
#line 220 "auto/extension_to_mime_type.gperf"
      {"emma", "application/emma+xml"},
      {"",nullptr}, {"",nullptr},
#line 740 "auto/extension_to_mime_type.gperf"
      {"skt", "application/vnd.koan"},
#line 496 "auto/extension_to_mime_type.gperf"
      {"mpy", "application/vnd.ibm.minipay"},
#line 675 "auto/extension_to_mime_type.gperf"
      {"rlc", "image/vnd.fujixerox.edmics-rlc"},
      {"",nullptr},
#line 146 "auto/extension_to_mime_type.gperf"
      {"crt", "application/x-x509-ca-cert"},
#line 570 "auto/extension_to_mime_type.gperf"
      {"otc", "application/vnd.oasis.opendocument.chart-template"},
#line 141 "auto/extension_to_mime_type.gperf"
      {"cpio", "application/x-cpio"},
#line 571 "auto/extension_to_mime_type.gperf"
      {"otf", "font/otf"},
#line 904 "auto/extension_to_mime_type.gperf"
      {"wdb", "application/vnd.ms-works"},
      {"",nullptr}, {"",nullptr}, {"",nullptr}, {"",nullptr},
      {"",nullptr}, {"",nullptr}, {"",nullptr}, {"",nullptr},
#line 787 "auto/extension_to_mime_type.gperf"
      {"swa", "application/x-director"},
      {"",nullptr},
#line 669 "auto/extension_to_mime_type.gperf"
      {"res", "application/x-dtbresource+xml"},
      {"",nullptr}, {"",nullptr}, {"",nullptr},
#line 998 "auto/extension_to_mime_type.gperf"
      {"z2", "application/x-zmachine"},
#line 438 "auto/extension_to_mime_type.gperf"
      {"mathml", "application/mathml+xml"},
      {"",nullptr},
#line 360 "auto/extension_to_mime_type.gperf"
      {"irm", "application/vnd.ibm.rights-management"},
#line 668 "auto/extension_to_mime_type.gperf"
      {"rep", "application/vnd.businessobjects"},
      {"",nullptr}, {"",nullptr}, {"",nullptr}, {"",nullptr},
#line 361 "auto/extension_to_mime_type.gperf"
      {"irp", "application/vnd.irepository.package+xml"},
#line 182 "auto/extension_to_mime_type.gperf"
      {"dll", "application/x-msdownload"},
#line 676 "auto/extension_to_mime_type.gperf"
      {"rld", "application/resource-lists-diff+xml"},
#line 434 "auto/extension_to_mime_type.gperf"
      {"mag", "application/vnd.ecowin.chart"},
      {"",nullptr}, {"",nullptr}, {"",nullptr},
#line 31 "auto/extension_to_mime_type.gperf"
      {"acu", "application/vnd.acucobol"},
      {"",nullptr}, {"",nullptr},
#line 839 "auto/extension_to_mime_type.gperf"
      {"ulx", "application/x-glulx"},
#line 362 "auto/extension_to_mime_type.gperf"
      {"iso", "application/x-iso9660-image"},
      {"",nullptr}, {"",nullptr}, {"",nullptr}, {"",nullptr},
#line 145 "auto/extension_to_mime_type.gperf"
      {"crl", "application/pkix-crl"},
      {"",nullptr}, {"",nullptr},
#line 420 "auto/extension_to_mime_type.gperf"
      {"m13", "application/x-msmediaview"},
      {"",nullptr}, {"",nullptr}, {"",nullptr},
#line 674 "auto/extension_to_mime_type.gperf"
      {"rl", "application/resource-lists+xml"},
#line 512 "auto/extension_to_mime_type.gperf"
      {"mvb", "application/x-msmediaview"},
#line 848 "auto/extension_to_mime_type.gperf"
      {"uu", "text/x-uuencode"},
      {"",nullptr},
#line 646 "auto/extension_to_mime_type.gperf"
      {"pvb", "application/vnd.3gpp.pic-bw-var"},
#line 930 "auto/extension_to_mime_type.gperf"
      {"wrl", "model/vrml"},
#line 421 "auto/extension_to_mime_type.gperf"
      {"m14", "application/x-msmediaview"},
#line 732 "auto/extension_to_mime_type.gperf"
      {"silo", "model/mesh"},
      {"",nullptr},
#line 822 "auto/extension_to_mime_type.gperf"
      {"tr", "text/troff"},
#line 861 "auto/extension_to_mime_type.gperf"
      {"uvva", "audio/vnd.dece.audio"},
      {"",nullptr}, {"",nullptr},
#line 183 "auto/extension_to_mime_type.gperf"
      {"dmg", "application/x-apple-diskimage"},
      {"",nullptr}, {"",nullptr}, {"",nullptr},
#line 342 "auto/extension_to_mime_type.gperf"
      {"ief", "image/ief"},
#line 105 "auto/extension_to_mime_type.gperf"
      {"cdbcmsg", "application/vnd.contact.cmsg"},
      {"",nullptr}, {"",nullptr}, {"",nullptr}, {"",nullptr},
      {"",nullptr}, {"",nullptr}, {"",nullptr}, {"",nullptr},
      {"",nullptr},
#line 809 "auto/extension_to_mime_type.gperf"
      {"text", "text/plain"},
#line 82 "auto/extension_to_mime_type.gperf"
      {"bz", "application/x-bzip"},
      {"",nullptr},
#line 203 "auto/extension_to_mime_type.gperf"
      {"dvb", "video/vnd.dvb.file"},
      {"",nullptr},
#line 490 "auto/extension_to_mime_type.gperf"
      {"mpga", "audio/mpeg"},
#line 962 "auto/extension_to_mime_type.gperf"
      {"xlam", "application/vnd.ms-excel.addin.macroenabled.12"},
      {"",nullptr}, {"",nullptr},
#line 399 "auto/extension_to_mime_type.gperf"
      {"kwd", "application/vnd.kde.kword"},
#line 159 "auto/extension_to_mime_type.gperf"
      {"dae", "model/vnd.collada+xml"},
      {"",nullptr},
#line 422 "auto/extension_to_mime_type.gperf"
      {"m1v", "video/mpeg"},
      {"",nullptr}, {"",nullptr},
#line 126 "auto/extension_to_mime_type.gperf"
      {"class", "application/java-vm"},
      {"",nullptr},
#line 397 "auto/extension_to_mime_type.gperf"
      {"ktx", "image/ktx"},
#line 60 "auto/extension_to_mime_type.gperf"
      {"au", "audio/basic"},
#line 318 "auto/extension_to_mime_type.gperf"
      {"hbci", "application/vnd.hbci"},
      {"",nullptr},
#line 232 "auto/extension_to_mime_type.gperf"
      {"eva", "application/x-eva"},
      {"",nullptr}, {"",nullptr},
#line 621 "auto/extension_to_mime_type.gperf"
      {"pnm", "image/x-portable-anymap"},
#line 640 "auto/extension_to_mime_type.gperf"
      {"psb", "application/vnd.3gpp.pic-bw-small"},
#line 577 "auto/extension_to_mime_type.gperf"
      {"ott", "application/vnd.oasis.opendocument.text-template"},
#line 327 "auto/extension_to_mime_type.gperf"
      {"hpid", "application/vnd.hp-hpid"},
#line 977 "auto/extension_to_mime_type.gperf"
      {"xop", "application/xop+xml"},
      {"",nullptr}, {"",nullptr}, {"",nullptr},
#line 139 "auto/extension_to_mime_type.gperf"
      {"com", "application/x-msdownload"},
      {"",nullptr}, {"",nullptr}, {"",nullptr}, {"",nullptr},
      {"",nullptr}, {"",nullptr},
#line 937 "auto/extension_to_mime_type.gperf"
      {"x3db", "model/x3d+binary"},
      {"",nullptr}, {"",nullptr}, {"",nullptr},
#line 53 "auto/extension_to_mime_type.gperf"
      {"aso", "application/vnd.accpac.simply.aso"},
      {"",nullptr},
#line 258 "auto/extension_to_mime_type.gperf"
      {"flac", "audio/x-flac"},
#line 404 "auto/extension_to_mime_type.gperf"
      {"lbe", "application/vnd.llamagraphics.life-balance.exchange+xml"},
      {"",nullptr},
#line 34 "auto/extension_to_mime_type.gperf"
      {"aep", "application/vnd.audiograph"},
#line 508 "auto/extension_to_mime_type.gperf"
      {"msty", "application/vnd.muvee.style"},
#line 751 "auto/extension_to_mime_type.gperf"
      {"snf", "application/x-font-snf"},
      {"",nullptr},
#line 877 "auto/extension_to_mime_type.gperf"
      {"vcard", "text/vcard"},
#line 474 "auto/extension_to_mime_type.gperf"
      {"mods", "application/mods+xml"},
#line 49 "auto/extension_to_mime_type.gperf"
      {"arc", "application/x-freearc"},
      {"",nullptr}, {"",nullptr},
#line 307 "auto/extension_to_mime_type.gperf"
      {"gtm", "application/vnd.groove-tool-message"},
#line 884 "auto/extension_to_mime_type.gperf"
      {"viv", "video/vnd.vivo"},
      {"",nullptr},
#line 488 "auto/extension_to_mime_type.gperf"
      {"mpg", "video/mpeg"},
#line 655 "auto/extension_to_mime_type.gperf"
      {"qwd", "application/vnd.quark.quarkxpress"},
      {"",nullptr},
#line 237 "auto/extension_to_mime_type.gperf"
      {"ez", "application/andrew-inset"},
#line 266 "auto/extension_to_mime_type.gperf"
      {"fnc", "application/vnd.frogans.fnc"},
      {"",nullptr}, {"",nullptr},
#line 758 "auto/extension_to_mime_type.gperf"
      {"spq", "application/scvp-vp-request"},
#line 785 "auto/extension_to_mime_type.gperf"
      {"svg", "image/svg+xml"},
#line 188 "auto/extension_to_mime_type.gperf"
      {"docm", "application/vnd.ms-word.document.macroenabled.12"},
#line 228 "auto/extension_to_mime_type.gperf"
      {"esa", "application/vnd.osgi.subsystem"},
      {"",nullptr}, {"",nullptr},
#line 435 "auto/extension_to_mime_type.gperf"
      {"maker", "application/vnd.framemaker"},
      {"",nullptr},
#line 845 "auto/extension_to_mime_type.gperf"
      {"urls", "text/uri-list"},
#line 187 "auto/extension_to_mime_type.gperf"
      {"doc", "application/msword"},
#line 189 "auto/extension_to_mime_type.gperf"
      {"docx", "application/vnd.openxmlformats-officedocument.wordprocessingml.document"},
      {"",nullptr},
#line 231 "auto/extension_to_mime_type.gperf"
      {"etx", "text/x-setext"},
#line 750 "auto/extension_to_mime_type.gperf"
      {"snd", "audio/basic"},
#line 400 "auto/extension_to_mime_type.gperf"
      {"kwt", "application/vnd.kde.kword"},
      {"",nullptr}, {"",nullptr},
#line 716 "auto/extension_to_mime_type.gperf"
      {"ser", "application/java-serialized-object"},
      {"",nullptr}, {"",nullptr}, {"",nullptr},
#line 486 "auto/extension_to_mime_type.gperf"
      {"mpe", "video/mpeg"},
#line 954 "auto/extension_to_mime_type.gperf"
      {"xer", "application/patch-ops-error+xml"},
#line 194 "auto/extension_to_mime_type.gperf"
      {"dpg", "application/vnd.dpgraph"},
      {"",nullptr},
#line 138 "auto/extension_to_mime_type.gperf"
      {"cod", "application/vnd.rim.cod"},
      {"",nullptr},
#line 624 "auto/extension_to_mime_type.gperf"
      {"potm", "application/vnd.ms-powerpoint.template.macroenabled.12"},
#line 259 "auto/extension_to_mime_type.gperf"
      {"fli", "video/x-fli"},
#line 923 "auto/extension_to_mime_type.gperf"
      {"woff", "font/woff"},
      {"",nullptr},
#line 932 "auto/extension_to_mime_type.gperf"
      {"wspolicy", "application/wspolicy+xml"},
#line 116 "auto/extension_to_mime_type.gperf"
      {"cer", "application/pkix-cert"},
      {"",nullptr},
#line 386 "auto/extension_to_mime_type.gperf"
      {"kia", "application/vnd.kidspiration"},
#line 625 "auto/extension_to_mime_type.gperf"
      {"potx", "application/vnd.openxmlformats-officedocument.presentationml.template"},
      {"",nullptr},
#line 90 "auto/extension_to_mime_type.gperf"
      {"c4p", "application/vnd.clonk.c4group"},
      {"",nullptr}, {"",nullptr},
#line 23 "auto/extension_to_mime_type.gperf"
      {"aab", "application/x-authorware-bin"},
      {"",nullptr},
#line 308 "auto/extension_to_mime_type.gperf"
      {"gtw", "model/vnd.gtw"},
#line 367 "auto/extension_to_mime_type.gperf"
      {"jam", "application/vnd.jam"},
      {"",nullptr}, {"",nullptr}, {"",nullptr}, {"",nullptr},
#line 172 "auto/extension_to_mime_type.gperf"
      {"der", "application/x-x509-ca-cert"},
      {"",nullptr}, {"",nullptr}, {"",nullptr},
#line 489 "auto/extension_to_mime_type.gperf"
      {"mpg4", "video/mp4"},
#line 654 "auto/extension_to_mime_type.gperf"
      {"qt", "video/quicktime"},
#line 191 "auto/extension_to_mime_type.gperf"
      {"dotm", "application/vnd.ms-word.template.macroenabled.12"},
      {"",nullptr}, {"",nullptr}, {"",nullptr},
#line 613 "auto/extension_to_mime_type.gperf"
      {"pki", "application/pkixcmp"},
      {"",nullptr},
#line 407 "auto/extension_to_mime_type.gperf"
      {"link66", "application/vnd.route66.link66+xml"},
#line 88 "auto/extension_to_mime_type.gperf"
      {"c4f", "application/vnd.clonk.c4group"},
#line 192 "auto/extension_to_mime_type.gperf"
      {"dotx", "application/vnd.openxmlformats-officedocument.wordprocessingml.template"},
#line 872 "auto/extension_to_mime_type.gperf"
      {"uvvv", "video/vnd.dece.video"},
      {"",nullptr},
#line 359 "auto/extension_to_mime_type.gperf"
      {"ipk", "application/vnd.shana.informed.package"},
#line 656 "auto/extension_to_mime_type.gperf"
      {"qwt", "application/vnd.quark.quarkxpress"},
      {"",nullptr}, {"",nullptr}, {"",nullptr},
#line 929 "auto/extension_to_mime_type.gperf"
      {"wri", "application/x-mswrite"},
      {"",nullptr}, {"",nullptr}, {"",nullptr}, {"",nullptr},
      {"",nullptr}, {"",nullptr}, {"",nullptr},
#line 766 "auto/extension_to_mime_type.gperf"
      {"sse", "application/vnd.kodak-descriptor"},
      {"",nullptr},
#line 398 "auto/extension_to_mime_type.gperf"
      {"ktz", "application/vnd.kahootz"},
      {"",nullptr}, {"",nullptr}, {"",nullptr}, {"",nullptr},
#line 56 "auto/extension_to_mime_type.gperf"
      {"atom", "application/atom+xml"},
#line 623 "auto/extension_to_mime_type.gperf"
      {"pot", "application/vnd.ms-powerpoint"},
      {"",nullptr},
#line 844 "auto/extension_to_mime_type.gperf"
      {"uris", "text/uri-list"},
#line 463 "auto/extension_to_mime_type.gperf"
      {"mk3d", "video/x-matroska"},
      {"",nullptr},
#line 87 "auto/extension_to_mime_type.gperf"
      {"c4d", "application/vnd.clonk.c4group"},
      {"",nullptr},
#line 424 "auto/extension_to_mime_type.gperf"
      {"m2a", "audio/mpeg"},
      {"",nullptr}, {"",nullptr}, {"",nullptr}, {"",nullptr},
      {"",nullptr}, {"",nullptr}, {"",nullptr}, {"",nullptr},
      {"",nullptr},
#line 366 "auto/extension_to_mime_type.gperf"
      {"jad", "text/vnd.sun.j2me.app-descriptor"},
      {"",nullptr},
#line 651 "auto/extension_to_mime_type.gperf"
      {"qbo", "application/vnd.intu.qbo"},
      {"",nullptr},
#line 227 "auto/extension_to_mime_type.gperf"
      {"es3", "application/vnd.eszigno3+xml"},
      {"",nullptr}, {"",nullptr}, {"",nullptr}, {"",nullptr},
      {"",nullptr},
#line 190 "auto/extension_to_mime_type.gperf"
      {"dot", "application/msword"},
      {"",nullptr}, {"",nullptr}, {"",nullptr},
#line 502 "auto/extension_to_mime_type.gperf"
      {"mseed", "application/vnd.fdsn.mseed"},
      {"",nullptr},
#line 647 "auto/extension_to_mime_type.gperf"
      {"pwn", "application/vnd.3m.post-it-notes"},
      {"",nullptr},
#line 763 "auto/extension_to_mime_type.gperf"
      {"sru", "application/sru+xml"},
      {"",nullptr}, {"",nullptr}, {"",nullptr}, {"",nullptr},
#line 804 "auto/extension_to_mime_type.gperf"
      {"tei", "application/tei+xml"},
      {"",nullptr}, {"",nullptr},
#line 682 "auto/extension_to_mime_type.gperf"
      {"rnc", "application/relax-ng-compact-syntax"},
      {"",nullptr}, {"",nullptr},
#line 665 "auto/extension_to_mime_type.gperf"
      {"rcprofile", "application/vnd.ipunplugged.rcprofile"},
      {"",nullptr}, {"",nullptr}, {"",nullptr},
#line 805 "auto/extension_to_mime_type.gperf"
      {"teicorpus", "application/tei+xml"},
#line 807 "auto/extension_to_mime_type.gperf"
      {"texi", "application/x-texinfo"},
#line 953 "auto/extension_to_mime_type.gperf"
      {"xenc", "application/xenc+xml"},
      {"",nullptr}, {"",nullptr}, {"",nullptr}, {"",nullptr},
#line 730 "auto/extension_to_mime_type.gperf"
      {"sig", "application/pgp-signature"},
      {"",nullptr}, {"",nullptr}, {"",nullptr},
#line 81 "auto/extension_to_mime_type.gperf"
      {"btif", "image/prs.btif"},
#line 45 "auto/extension_to_mime_type.gperf"
      {"apk", "application/vnd.android.package-archive"},
      {"",nullptr},
#line 684 "auto/extension_to_mime_type.gperf"
      {"roff", "text/troff"},
#line 379 "auto/extension_to_mime_type.gperf"
      {"jpm", "video/jpm"},
      {"",nullptr}, {"",nullptr}, {"",nullptr}, {"",nullptr},
      {"",nullptr}, {"",nullptr}, {"",nullptr},
#line 257 "auto/extension_to_mime_type.gperf"
      {"fig", "application/x-xfig"},
      {"",nullptr}, {"",nullptr}, {"",nullptr},
#line 574 "auto/extension_to_mime_type.gperf"
      {"oti", "application/vnd.oasis.opendocument.image-template"},
      {"",nullptr},
#line 894 "auto/extension_to_mime_type.gperf"
      {"vtu", "model/vnd.vtu"},
      {"",nullptr}, {"",nullptr}, {"",nullptr}, {"",nullptr},
      {"",nullptr}, {"",nullptr},
#line 458 "auto/extension_to_mime_type.gperf"
      {"mie", "application/x-mie"},
      {"",nullptr}, {"",nullptr}, {"",nullptr}, {"",nullptr},
      {"",nullptr}, {"",nullptr},
#line 460 "auto/extension_to_mime_type.gperf"
      {"mime", "message/rfc822"},
      {"",nullptr}, {"",nullptr}, {"",nullptr},
#line 961 "auto/extension_to_mime_type.gperf"
      {"xla", "application/vnd.ms-excel"},
      {"",nullptr}, {"",nullptr}, {"",nullptr}, {"",nullptr},
      {"",nullptr}, {"",nullptr}, {"",nullptr},
#line 610 "auto/extension_to_mime_type.gperf"
      {"php", "text/x-php"},
      {"",nullptr},
#line 125 "auto/extension_to_mime_type.gperf"
      {"cla", "application/vnd.claymore"},
#line 340 "auto/extension_to_mime_type.gperf"
      {"ico", "image/x-icon"},
      {"",nullptr},
#line 120 "auto/extension_to_mime_type.gperf"
      {"chm", "application/vnd.ms-htmlhelp"},
#line 255 "auto/extension_to_mime_type.gperf"
      {"fh7", "image/x-freehand"},
      {"",nullptr}, {"",nullptr}, {"",nullptr},
#line 380 "auto/extension_to_mime_type.gperf"
      {"js", "application/javascript"},
      {"",nullptr}, {"",nullptr}, {"",nullptr}, {"",nullptr},
      {"",nullptr},
#line 278 "auto/extension_to_mime_type.gperf"
      {"g2w", "application/vnd.geoplan"},
#line 413 "auto/extension_to_mime_type.gperf"
      {"lostxml", "application/lost+xml"},
#line 852 "auto/extension_to_mime_type.gperf"
      {"uvg", "image/vnd.dece.graphic"},
      {"",nullptr},
#line 728 "auto/extension_to_mime_type.gperf"
      {"shf", "application/shf+xml"},
#line 17 "auto/extension_to_mime_type.gperf"
      {"123", "application/vnd.lotus-1-2-3"},
      {"",nullptr}, {"",nullptr}, {"",nullptr},
#line 888 "auto/extension_to_mime_type.gperf"
      {"vrml", "model/vrml"},
      {"",nullptr}, {"",nullptr}, {"",nullptr},
#line 128 "auto/extension_to_mime_type.gperf"
      {"clkp", "application/vnd.crick.clicker.palette"},
#line 464 "auto/extension_to_mime_type.gperf"
      {"mka", "audio/x-matroska"},
#line 131 "auto/extension_to_mime_type.gperf"
      {"clkx", "application/vnd.crick.clicker"},
      {"",nullptr}, {"",nullptr},
#line 256 "auto/extension_to_mime_type.gperf"
      {"fhc", "image/x-freehand"},
      {"",nullptr}, {"",nullptr}, {"",nullptr}, {"",nullptr},
      {"",nullptr}, {"",nullptr},
#line 713 "auto/extension_to_mime_type.gperf"
      {"sema", "application/vnd.sema"},
#line 544 "auto/extension_to_mime_type.gperf"
      {"odb", "application/vnd.oasis.opendocument.database"},
      {"",nullptr},
#line 924 "auto/extension_to_mime_type.gperf"
      {"woff2", "font/woff2"},
#line 396 "auto/extension_to_mime_type.gperf"
      {"ktr", "application/vnd.kahootz"},
      {"",nullptr}, {"",nullptr},
#line 724 "auto/extension_to_mime_type.gperf"
      {"sgm", "text/sgml"},
#line 195 "auto/extension_to_mime_type.gperf"
      {"dra", "audio/vnd.dra"},
#line 425 "auto/extension_to_mime_type.gperf"
      {"m2v", "video/mpeg"},
      {"",nullptr},
#line 607 "auto/extension_to_mime_type.gperf"
      {"pgm", "image/x-portable-graymap"},
      {"",nullptr}, {"",nullptr},
#line 454 "auto/extension_to_mime_type.gperf"
      {"mgp", "application/vnd.osgeo.mapguide.package"},
#line 254 "auto/extension_to_mime_type.gperf"
      {"fh5", "image/x-freehand"},
      {"",nullptr},
#line 609 "auto/extension_to_mime_type.gperf"
      {"pgp", "application/pgp-encrypted"},
#line 720 "auto/extension_to_mime_type.gperf"
      {"sfs", "application/vnd.spotfire.sfs"},
#line 843 "auto/extension_to_mime_type.gperf"
      {"uri", "text/uri-list"},
      {"",nullptr},
#line 452 "auto/extension_to_mime_type.gperf"
      {"mfm", "application/vnd.mfmp"},
#line 118 "auto/extension_to_mime_type.gperf"
      {"cgm", "image/cgm"},
      {"",nullptr},
#line 604 "auto/extension_to_mime_type.gperf"
      {"pfm", "application/x-font-type1"},
      {"",nullptr}, {"",nullptr},
#line 996 "auto/extension_to_mime_type.gperf"
      {"yin", "application/yin+xml"},
#line 267 "auto/extension_to_mime_type.gperf"
      {"for", "text/x-fortran"},
      {"",nullptr}, {"",nullptr},
#line 815 "auto/extension_to_mime_type.gperf"
      {"thmx", "application/vnd.ms-officetheme"},
#line 606 "auto/extension_to_mime_type.gperf"
      {"pfx", "application/x-pkcs12"},
#line 117 "auto/extension_to_mime_type.gperf"
      {"cfs", "application/x-cfs-compressed"},
#line 130 "auto/extension_to_mime_type.gperf"
      {"clkw", "application/vnd.crick.clicker.wordbank"},
      {"",nullptr}, {"",nullptr}, {"",nullptr}, {"",nullptr},
#line 681 "auto/extension_to_mime_type.gperf"
      {"rmvb", "application/vnd.rn-realmedia-vbr"},
      {"",nullptr},
#line 907 "auto/extension_to_mime_type.gperf"
      {"webm", "video/webm"},
      {"",nullptr}, {"",nullptr},
#line 823 "auto/extension_to_mime_type.gperf"
      {"tra", "application/vnd.trueapp"},
      {"",nullptr}, {"",nullptr},
#line 908 "auto/extension_to_mime_type.gperf"
      {"webp", "image/webp"},
      {"",nullptr}, {"",nullptr},
#line 72 "auto/extension_to_mime_type.gperf"
      {"bin", "application/octet-stream"},
#line 217 "auto/extension_to_mime_type.gperf"
      {"elc", "application/octet-stream"},
#line 887 "auto/extension_to_mime_type.gperf"
      {"vox", "application/x-authorware-bin"},
      {"",nullptr},
#line 331 "auto/extension_to_mime_type.gperf"
      {"htm", "text/html"},
      {"",nullptr}, {"",nullptr}, {"",nullptr},
#line 284 "auto/extension_to_mime_type.gperf"
      {"gca", "application/x-gca-compressed"},
      {"",nullptr}, {"",nullptr}, {"",nullptr}, {"",nullptr},
#line 447 "auto/extension_to_mime_type.gperf"
      {"me", "text/troff"},
#line 287 "auto/extension_to_mime_type.gperf"
      {"gex", "application/vnd.geometry-explorer"},
      {"",nullptr},
#line 174 "auto/extension_to_mime_type.gperf"
      {"dgc", "application/x-dgc-compressed"},
      {"",nullptr},
#line 70 "auto/extension_to_mime_type.gperf"
      {"bed", "application/vnd.realvnc.bed"},
      {"",nullptr},
#line 517 "auto/extension_to_mime_type.gperf"
      {"mxs", "application/vnd.triscape.mxs"},
#line 794 "auto/extension_to_mime_type.gperf"
      {"sxm", "application/vnd.sun.xml.math"},
#line 813 "auto/extension_to_mime_type.gperf"
      {"tgs", "application/x-tgsticker"},
#line 250 "auto/extension_to_mime_type.gperf"
      {"fg5", "application/vnd.fujitsu.oasysgp"},
      {"",nullptr}, {"",nullptr}, {"",nullptr}, {"",nullptr},
      {"",nullptr}, {"",nullptr},
#line 83 "auto/extension_to_mime_type.gperf"
      {"bz2", "application/x-bzip2"},
      {"",nullptr},
#line 370 "auto/extension_to_mime_type.gperf"
      {"jisp", "application/vnd.jisp"},
#line 251 "auto/extension_to_mime_type.gperf"
      {"fgd", "application/x-director"},
      {"",nullptr}, {"",nullptr},
#line 957 "auto/extension_to_mime_type.gperf"
      {"xht", "application/xhtml+xml"},
#line 811 "auto/extension_to_mime_type.gperf"
      {"tfm", "application/x-tex-tfm"},
#line 67 "auto/extension_to_mime_type.gperf"
      {"bcpio", "application/x-bcpio"},
      {"",nullptr}, {"",nullptr}, {"",nullptr},
#line 955 "auto/extension_to_mime_type.gperf"
      {"xfdf", "application/vnd.adobe.xfdf"},
#line 275 "auto/extension_to_mime_type.gperf"
      {"fxp", "application/vnd.adobe.fxp"},
#line 158 "auto/extension_to_mime_type.gperf"
      {"cxx", "text/x-c"},
#line 531 "auto/extension_to_mime_type.gperf"
      {"nns", "application/vnd.noblenet-sealer"},
      {"",nullptr},
#line 548 "auto/extension_to_mime_type.gperf"
      {"odg", "application/vnd.oasis.opendocument.graphics"},
      {"",nullptr},
#line 790 "auto/extension_to_mime_type.gperf"
      {"sxc", "application/vnd.sun.xml.calc"},
      {"",nullptr},
#line 719 "auto/extension_to_mime_type.gperf"
      {"sfd-hdstx", "application/vnd.hydrostatix.sof-data"},
#line 514 "auto/extension_to_mime_type.gperf"
      {"mxf", "application/mxf"},
      {"",nullptr}, {"",nullptr},
#line 368 "auto/extension_to_mime_type.gperf"
      {"jar", "application/java-archive"},
      {"",nullptr}, {"",nullptr},
#line 208 "auto/extension_to_mime_type.gperf"
      {"dxp", "application/vnd.spotfire.dxp"},
#line 842 "auto/extension_to_mime_type.gperf"
      {"uoml", "application/vnd.uoml+xml"},
#line 261 "auto/extension_to_mime_type.gperf"
      {"flv", "video/x-flv"},
      {"",nullptr},
#line 752 "auto/extension_to_mime_type.gperf"
      {"so", "application/octet-stream"},
      {"",nullptr},
#line 129 "auto/extension_to_mime_type.gperf"
      {"clkt", "application/vnd.crick.clicker.template"},
      {"",nullptr},
#line 958 "auto/extension_to_mime_type.gperf"
      {"xhtml", "application/xhtml+xml"},
#line 976 "auto/extension_to_mime_type.gperf"
      {"xo", "application/vnd.olpc-sugar"},
      {"",nullptr}, {"",nullptr}, {"",nullptr},
#line 201 "auto/extension_to_mime_type.gperf"
      {"dtshd", "audio/vnd.dts.hd"},
      {"",nullptr},
#line 795 "auto/extension_to_mime_type.gperf"
      {"sxw", "application/vnd.sun.xml.writer"},
      {"",nullptr},
#line 449 "auto/extension_to_mime_type.gperf"
      {"meta4", "application/metalink4+xml"},
      {"",nullptr},
#line 701 "auto/extension_to_mime_type.gperf"
      {"scq", "application/scvp-cv-request"},
      {"",nullptr}, {"",nullptr}, {"",nullptr},
#line 791 "auto/extension_to_mime_type.gperf"
      {"sxd", "application/vnd.sun.xml.draw"},
#line 207 "auto/extension_to_mime_type.gperf"
      {"dxf", "image/vnd.dxf"},
      {"",nullptr}, {"",nullptr}, {"",nullptr},
#line 712 "auto/extension_to_mime_type.gperf"
      {"seed", "application/vnd.fdsn.seed"},
#line 466 "auto/extension_to_mime_type.gperf"
      {"mkv", "video/x-matroska"},
      {"",nullptr}, {"",nullptr},
#line 304 "auto/extension_to_mime_type.gperf"
      {"grxml", "application/srgs+xml"},
      {"",nullptr},
#line 238 "auto/extension_to_mime_type.gperf"
      {"ez2", "application/vnd.ezpix-album"},
      {"",nullptr}, {"",nullptr},
#line 140 "auto/extension_to_mime_type.gperf"
      {"conf", "text/plain"},
#line 455 "auto/extension_to_mime_type.gperf"
      {"mgz", "application/vnd.proteus.magazine"},
#line 356 "auto/extension_to_mime_type.gperf"
      {"install", "application/x-install-instructions"},
      {"",nullptr},
#line 532 "auto/extension_to_mime_type.gperf"
      {"nnw", "application/vnd.noblenet-web"},
      {"",nullptr}, {"",nullptr}, {"",nullptr},
#line 453 "auto/extension_to_mime_type.gperf"
      {"mft", "application/rpki-manifest"},
      {"",nullptr},
#line 1000 "auto/extension_to_mime_type.gperf"
      {"z4", "application/x-zmachine"},
      {"",nullptr},
#line 530 "auto/extension_to_mime_type.gperf"
      {"nnd", "application/vnd.noblenet-directory"},
      {"",nullptr}, {"",nullptr}, {"",nullptr}, {"",nullptr},
#line 808 "auto/extension_to_mime_type.gperf"
      {"texinfo", "application/x-texinfo"},
      {"",nullptr}, {"",nullptr}, {"",nullptr},
#line 910 "auto/extension_to_mime_type.gperf"
      {"wgt", "application/widget"},
      {"",nullptr},
#line 833 "auto/extension_to_mime_type.gperf"
      {"txf", "application/vnd.mobius.txf"},
      {"",nullptr}, {"",nullptr}, {"",nullptr}, {"",nullptr},
      {"",nullptr},
#line 348 "auto/extension_to_mime_type.gperf"
      {"igs", "model/iges"},
      {"",nullptr},
#line 347 "auto/extension_to_mime_type.gperf"
      {"igm", "application/vnd.insors.igm"},
#line 107 "auto/extension_to_mime_type.gperf"
      {"cdkey", "application/vnd.mediastation.cdkey"},
      {"",nullptr},
#line 57 "auto/extension_to_mime_type.gperf"
      {"atomcat", "application/atomcat+xml"},
      {"",nullptr},
#line 80 "auto/extension_to_mime_type.gperf"
      {"bpk", "application/octet-stream"},
      {"",nullptr},
#line 723 "auto/extension_to_mime_type.gperf"
      {"sgl", "application/vnd.stardivision.writer-global"},
#line 349 "auto/extension_to_mime_type.gperf"
      {"igx", "application/vnd.micrografx.igx"},
      {"",nullptr}, {"",nullptr}, {"",nullptr}, {"",nullptr},
#line 344 "auto/extension_to_mime_type.gperf"
      {"ifm", "application/vnd.shana.informed.formdata"},
#line 725 "auto/extension_to_mime_type.gperf"
      {"sgml", "text/sgml"},
      {"",nullptr}, {"",nullptr}, {"",nullptr}, {"",nullptr},
      {"",nullptr}, {"",nullptr}, {"",nullptr},
#line 832 "auto/extension_to_mime_type.gperf"
      {"txd", "application/vnd.genomatix.tuxedo"},
#line 430 "auto/extension_to_mime_type.gperf"
      {"m4u", "video/vnd.mpegurl"},
      {"",nullptr},
#line 58 "auto/extension_to_mime_type.gperf"
      {"atomsvc", "application/atomsvc+xml"},
      {"",nullptr}, {"",nullptr}, {"",nullptr},
#line 312 "auto/extension_to_mime_type.gperf"
      {"h", "text/x-c"},
#line 233 "auto/extension_to_mime_type.gperf"
      {"evy", "application/x-envoy"},
      {"",nullptr}, {"",nullptr}, {"",nullptr}, {"",nullptr},
      {"",nullptr}, {"",nullptr},
#line 91 "auto/extension_to_mime_type.gperf"
      {"c4u", "application/vnd.clonk.c4group"},
      {"",nullptr}, {"",nullptr}, {"",nullptr}, {"",nullptr},
#line 157 "auto/extension_to_mime_type.gperf"
      {"cxt", "application/x-director"},
      {"",nullptr}, {"",nullptr}, {"",nullptr}, {"",nullptr},
      {"",nullptr}, {"",nullptr},
#line 774 "auto/extension_to_mime_type.gperf"
      {"stk", "application/hyperstudio"},
      {"",nullptr}, {"",nullptr}, {"",nullptr}, {"",nullptr},
      {"",nullptr}, {"",nullptr}, {"",nullptr}, {"",nullptr},
#line 528 "auto/extension_to_mime_type.gperf"
      {"nlu", "application/vnd.neurolanguage.nlu"},
      {"",nullptr}, {"",nullptr},
#line 332 "auto/extension_to_mime_type.gperf"
      {"html", "text/html"},
#line 956 "auto/extension_to_mime_type.gperf"
      {"xfdl", "application/vnd.xfdl"},
      {"",nullptr},
#line 326 "auto/extension_to_mime_type.gperf"
      {"hpgl", "application/vnd.hp-hpgl"},
      {"",nullptr}, {"",nullptr},
#line 78 "auto/extension_to_mime_type.gperf"
      {"box", "application/vnd.previewsystems.box"},
#line 390 "auto/extension_to_mime_type.gperf"
      {"knp", "application/vnd.kinar"},
      {"",nullptr}, {"",nullptr}, {"",nullptr},
#line 515 "auto/extension_to_mime_type.gperf"
      {"mxl", "application/vnd.recordare.musicxml"},
      {"",nullptr},
#line 121 "auto/extension_to_mime_type.gperf"
      {"chrt", "application/vnd.kde.kchart"},
      {"",nullptr},
#line 933 "auto/extension_to_mime_type.gperf"
      {"wtb", "application/vnd.webturbo"},
#line 569 "auto/extension_to_mime_type.gperf"
      {"osfpvg", "application/vnd.yamaha.openscoreformat.osfpvg+xml"},
#line 186 "auto/extension_to_mime_type.gperf"
      {"dna", "application/vnd.dna"},
#line 516 "auto/extension_to_mime_type.gperf"
      {"mxml", "application/xv+xml"},
#line 198 "auto/extension_to_mime_type.gperf"
      {"dtb", "application/x-dtbook+xml"},
#line 211 "auto/extension_to_mime_type.gperf"
      {"ecelp7470", "audio/vnd.nuera.ecelp7470"},
#line 353 "auto/extension_to_mime_type.gperf"
      {"in", "text/plain"},
      {"",nullptr},
#line 212 "auto/extension_to_mime_type.gperf"
      {"ecelp9600", "audio/vnd.nuera.ecelp9600"},
      {"",nullptr},
#line 260 "auto/extension_to_mime_type.gperf"
      {"flo", "application/vnd.micrografx.flo"},
      {"",nullptr}, {"",nullptr}, {"",nullptr}, {"",nullptr},
      {"",nullptr}, {"",nullptr}, {"",nullptr},
#line 834 "auto/extension_to_mime_type.gperf"
      {"txt", "text/plain"},
      {"",nullptr},
#line 35 "auto/extension_to_mime_type.gperf"
      {"afm", "application/x-font-type1"},
#line 276 "auto/extension_to_mime_type.gperf"
      {"fxpl", "application/vnd.adobe.fxp"},
      {"",nullptr}, {"",nullptr}, {"",nullptr}, {"",nullptr},
#line 36 "auto/extension_to_mime_type.gperf"
      {"afp", "application/vnd.ibm.modcap"},
      {"",nullptr}, {"",nullptr},
#line 313 "auto/extension_to_mime_type.gperf"
      {"h261", "video/h261"},
      {"",nullptr}, {"",nullptr},
#line 491 "auto/extension_to_mime_type.gperf"
      {"mpkg", "application/vnd.apple.installer+xml"},
      {"",nullptr}, {"",nullptr},
#line 429 "auto/extension_to_mime_type.gperf"
      {"m4a", "audio/mp4"},
#line 837 "auto/extension_to_mime_type.gperf"
      {"ufd", "application/vnd.ufdl"},
      {"",nullptr}, {"",nullptr}, {"",nullptr},
#line 206 "auto/extension_to_mime_type.gperf"
      {"dwg", "image/vnd.dwg"},
      {"",nullptr}, {"",nullptr}, {"",nullptr}, {"",nullptr},
      {"",nullptr},
#line 300 "auto/extension_to_mime_type.gperf"
      {"gram", "application/srgs"},
#line 778 "auto/extension_to_mime_type.gperf"
      {"sub", "image/vnd.dvb.subtitle"},
      {"",nullptr},
#line 316 "auto/extension_to_mime_type.gperf"
      {"h265", "video/h265"},
      {"",nullptr},
#line 645 "auto/extension_to_mime_type.gperf"
      {"pub", "application/x-mspublisher"},
#line 301 "auto/extension_to_mime_type.gperf"
      {"gramps", "application/x-gramps-xml"},
      {"",nullptr}, {"",nullptr}, {"",nullptr},
#line 239 "auto/extension_to_mime_type.gperf"
      {"ez3", "application/vnd.ezpix-package"},
#line 864 "auto/extension_to_mime_type.gperf"
      {"uvvg", "image/vnd.dece.graphic"},
#line 306 "auto/extension_to_mime_type.gperf"
      {"gtar", "application/x-gtar"},
#line 325 "auto/extension_to_mime_type.gperf"
      {"hlp", "application/winhlp"},
      {"",nullptr}, {"",nullptr}, {"",nullptr}, {"",nullptr},
#line 338 "auto/extension_to_mime_type.gperf"
      {"ice", "x-conference/x-cooltalk"},
#line 21 "auto/extension_to_mime_type.gperf"
      {"3gp", "video/3gpp"},
#line 605 "auto/extension_to_mime_type.gperf"
      {"pfr", "application/font-tdpfr"},
      {"",nullptr}, {"",nullptr}, {"",nullptr}, {"",nullptr},
      {"",nullptr}, {"",nullptr}, {"",nullptr}, {"",nullptr},
      {"",nullptr},
#line 269 "auto/extension_to_mime_type.gperf"
      {"frame", "application/vnd.framemaker"},
      {"",nullptr}, {"",nullptr}, {"",nullptr}, {"",nullptr},
      {"",nullptr}, {"",nullptr}, {"",nullptr}, {"",nullptr},
#line 173 "auto/extension_to_mime_type.gperf"
      {"dfac", "application/vnd.dreamfactory"},
#line 886 "auto/extension_to_mime_type.gperf"
      {"vor", "application/vnd.stardivision.writer"},
      {"",nullptr}, {"",nullptr}, {"",nullptr}, {"",nullptr},
#line 722 "auto/extension_to_mime_type.gperf"
      {"sgi", "image/sgi"},
      {"",nullptr}, {"",nullptr},
#line 487 "auto/extension_to_mime_type.gperf"
      {"mpeg", "video/mpeg"},
      {"",nullptr}, {"",nullptr}, {"",nullptr}, {"",nullptr},
      {"",nullptr},
#line 346 "auto/extension_to_mime_type.gperf"
      {"igl", "application/vnd.igloader"},
      {"",nullptr},
#line 119 "auto/extension_to_mime_type.gperf"
      {"chat", "application/x-chat"},
      {"",nullptr}, {"",nullptr}, {"",nullptr}, {"",nullptr},
      {"",nullptr}, {"",nullptr}, {"",nullptr}, {"",nullptr},
      {"",nullptr}, {"",nullptr}, {"",nullptr}, {"",nullptr},
      {"",nullptr},
#line 683 "auto/extension_to_mime_type.gperf"
      {"roa", "application/rpki-roa"},
#line 475 "auto/extension_to_mime_type.gperf"
      {"mov", "video/quicktime"},
      {"",nullptr}, {"",nullptr}, {"",nullptr},
#line 79 "auto/extension_to_mime_type.gperf"
      {"boz", "application/x-bzip2"},
      {"",nullptr},
#line 959 "auto/extension_to_mime_type.gperf"
      {"xhvml", "application/xv+xml"},
      {"",nullptr}, {"",nullptr},
#line 230 "auto/extension_to_mime_type.gperf"
      {"et3", "application/vnd.eszigno3+xml"},
      {"",nullptr}, {"",nullptr}, {"",nullptr}, {"",nullptr},
      {"",nullptr}, {"",nullptr}, {"",nullptr}, {"",nullptr},
      {"",nullptr}, {"",nullptr}, {"",nullptr}, {"",nullptr},
      {"",nullptr},
#line 836 "auto/extension_to_mime_type.gperf"
      {"udeb", "application/x-debian-package"},
      {"",nullptr}, {"",nullptr}, {"",nullptr},
#line 209 "auto/extension_to_mime_type.gperf"
      {"dxr", "application/x-director"},
      {"",nullptr}, {"",nullptr},
#line 30 "auto/extension_to_mime_type.gperf"
      {"ace", "application/x-ace-compressed"},
      {"",nullptr}, {"",nullptr}, {"",nullptr}, {"",nullptr},
      {"",nullptr}, {"",nullptr}, {"",nullptr},
#line 793 "auto/extension_to_mime_type.gperf"
      {"sxi", "application/vnd.sun.xml.impress"},
      {"",nullptr}, {"",nullptr}, {"",nullptr},
#line 505 "auto/extension_to_mime_type.gperf"
      {"msh", "model/mesh"},
#line 503 "auto/extension_to_mime_type.gperf"
      {"mseq", "application/vnd.mseq"},
      {"",nullptr}, {"",nullptr}, {"",nullptr}, {"",nullptr},
#line 557 "auto/extension_to_mime_type.gperf"
      {"ogx", "application/ogg"},
      {"",nullptr}, {"",nullptr}, {"",nullptr}, {"",nullptr},
      {"",nullptr},
#line 810 "auto/extension_to_mime_type.gperf"
      {"tfi", "application/thraud+xml"},
      {"",nullptr},
#line 148 "auto/extension_to_mime_type.gperf"
      {"csh", "application/x-csh"},
      {"",nullptr}, {"",nullptr}, {"",nullptr},
#line 223 "auto/extension_to_mime_type.gperf"
      {"eot", "application/vnd.ms-fontobject"},
      {"",nullptr}, {"",nullptr}, {"",nullptr}, {"",nullptr},
      {"",nullptr},
#line 431 "auto/extension_to_mime_type.gperf"
      {"m4v", "video/x-m4v"},
      {"",nullptr}, {"",nullptr}, {"",nullptr}, {"",nullptr},
      {"",nullptr},
#line 838 "auto/extension_to_mime_type.gperf"
      {"ufdl", "application/vnd.ufdl"},
      {"",nullptr}, {"",nullptr}, {"",nullptr},
#line 880 "auto/extension_to_mime_type.gperf"
      {"vcg", "application/vnd.groove-vcard"},
#line 264 "auto/extension_to_mime_type.gperf"
      {"fly", "text/vnd.fly"},
      {"",nullptr}, {"",nullptr}, {"",nullptr},
#line 241 "auto/extension_to_mime_type.gperf"
      {"f4v", "video/x-f4v"},
      {"",nullptr}, {"",nullptr}, {"",nullptr}, {"",nullptr},
      {"",nullptr}, {"",nullptr},
#line 225 "auto/extension_to_mime_type.gperf"
      {"epub", "application/epub+zip"},
      {"",nullptr}, {"",nullptr}, {"",nullptr}, {"",nullptr},
#line 357 "auto/extension_to_mime_type.gperf"
      {"iota", "application/vnd.astraea-software.iota"},
      {"",nullptr}, {"",nullptr}, {"",nullptr}, {"",nullptr},
      {"",nullptr}, {"",nullptr},
#line 384 "auto/extension_to_mime_type.gperf"
      {"karbon", "application/vnd.kde.karbon"},
      {"",nullptr}, {"",nullptr}, {"",nullptr},
#line 222 "auto/extension_to_mime_type.gperf"
      {"eol", "audio/vnd.digital-winds"},
      {"",nullptr}, {"",nullptr}, {"",nullptr}, {"",nullptr},
      {"",nullptr}, {"",nullptr},
#line 615 "auto/extension_to_mime_type.gperf"
      {"plb", "application/vnd.3gpp.pic-bw-large"},
#line 578 "auto/extension_to_mime_type.gperf"
      {"oxps", "application/oxps"},
      {"",nullptr}, {"",nullptr}, {"",nullptr}, {"",nullptr},
#line 518 "auto/extension_to_mime_type.gperf"
      {"mxu", "video/vnd.mpegurl"},
#line 967 "auto/extension_to_mime_type.gperf"
      {"xlsb", "application/vnd.ms-excel.sheet.binary.macroenabled.12"},
      {"",nullptr}, {"",nullptr},
#line 377 "auto/extension_to_mime_type.gperf"
      {"jpgm", "video/jpm"},
      {"",nullptr}, {"",nullptr}, {"",nullptr}, {"",nullptr},
      {"",nullptr}, {"",nullptr}, {"",nullptr}, {"",nullptr},
      {"",nullptr}, {"",nullptr},
#line 727 "auto/extension_to_mime_type.gperf"
      {"shar", "application/x-shar"},
#line 355 "auto/extension_to_mime_type.gperf"
      {"inkml", "application/inkml+xml"},
      {"",nullptr}, {"",nullptr}, {"",nullptr}, {"",nullptr},
      {"",nullptr}, {"",nullptr}, {"",nullptr},
#line 450 "auto/extension_to_mime_type.gperf"
      {"metalink", "application/metalink+xml"},
      {"",nullptr}, {"",nullptr}, {"",nullptr}, {"",nullptr},
      {"",nullptr}, {"",nullptr}, {"",nullptr}, {"",nullptr},
      {"",nullptr}, {"",nullptr}, {"",nullptr}, {"",nullptr},
      {"",nullptr}, {"",nullptr}, {"",nullptr},
#line 602 "auto/extension_to_mime_type.gperf"
      {"pfa", "application/x-font-type1"},
      {"",nullptr},
#line 995 "auto/extension_to_mime_type.gperf"
      {"yang", "application/yang"},
      {"",nullptr}, {"",nullptr},
#line 537 "auto/extension_to_mime_type.gperf"
      {"nzb", "application/x-nzb"},
      {"",nullptr}, {"",nullptr}, {"",nullptr}, {"",nullptr},
#line 819 "auto/extension_to_mime_type.gperf"
      {"torrent", "application/x-bittorrent"},
      {"",nullptr}, {"",nullptr}, {"",nullptr},
#line 406 "auto/extension_to_mime_type.gperf"
      {"lha", "application/x-lzh-compressed"},
#line 321 "auto/extension_to_mime_type.gperf"
      {"heics", "image/heic-sequence"},
#line 323 "auto/extension_to_mime_type.gperf"
      {"heifs", "image/heif-sequence"},
#line 906 "auto/extension_to_mime_type.gperf"
      {"weba", "audio/webm"},
#line 290 "auto/extension_to_mime_type.gperf"
      {"ghf", "application/vnd.groove-help"},
      {"",nullptr}, {"",nullptr}, {"",nullptr}, {"",nullptr},
      {"",nullptr}, {"",nullptr}, {"",nullptr}, {"",nullptr},
      {"",nullptr},
#line 169 "auto/extension_to_mime_type.gperf"
      {"deb", "application/x-debian-package"},
#line 320 "auto/extension_to_mime_type.gperf"
      {"heic", "image/heic"},
      {"",nullptr},
#line 322 "auto/extension_to_mime_type.gperf"
      {"heif", "image/heif"},
      {"",nullptr}, {"",nullptr}, {"",nullptr},
#line 717 "auto/extension_to_mime_type.gperf"
      {"setpay", "application/set-payment-initiation"},
      {"",nullptr}, {"",nullptr},
#line 803 "auto/extension_to_mime_type.gperf"
      {"teacher", "application/vnd.smart.teacher"},
#line 210 "auto/extension_to_mime_type.gperf"
      {"ecelp4800", "audio/vnd.nuera.ecelp4800"},
      {"",nullptr}, {"",nullptr}, {"",nullptr},
#line 853 "auto/extension_to_mime_type.gperf"
      {"uvh", "video/vnd.dece.hd"},
      {"",nullptr}, {"",nullptr}, {"",nullptr}, {"",nullptr},
      {"",nullptr}, {"",nullptr},
#line 812 "auto/extension_to_mime_type.gperf"
      {"tga", "image/x-tga"},
      {"",nullptr},
#line 473 "auto/extension_to_mime_type.gperf"
      {"mobi", "application/x-mobipocket-ebook"},
      {"",nullptr},
#line 895 "auto/extension_to_mime_type.gperf"
      {"vxml", "application/voicexml+xml"},
#line 253 "auto/extension_to_mime_type.gperf"
      {"fh4", "image/x-freehand"},
      {"",nullptr}, {"",nullptr}, {"",nullptr}, {"",nullptr},
      {"",nullptr}, {"",nullptr}, {"",nullptr}, {"",nullptr},
      {"",nullptr},
#line 909 "auto/extension_to_mime_type.gperf"
      {"wg", "application/vnd.pmi.widget"},
      {"",nullptr}, {"",nullptr}, {"",nullptr}, {"",nullptr},
      {"",nullptr},
#line 652 "auto/extension_to_mime_type.gperf"
      {"qfx", "application/vnd.intu.qfx"},
      {"",nullptr}, {"",nullptr}, {"",nullptr}, {"",nullptr},
      {"",nullptr},
#line 614 "auto/extension_to_mime_type.gperf"
      {"pkipath", "application/pkix-pkipath"},
      {"",nullptr},
#line 579 "auto/extension_to_mime_type.gperf"
      {"oxt", "application/vnd.openofficeorg.extension"},
      {"",nullptr}, {"",nullptr}, {"",nullptr}, {"",nullptr},
      {"",nullptr}, {"",nullptr}, {"",nullptr}, {"",nullptr},
#line 171 "auto/extension_to_mime_type.gperf"
      {"deploy", "application/octet-stream"},
      {"",nullptr}, {"",nullptr}, {"",nullptr},
#line 612 "auto/extension_to_mime_type.gperf"
      {"pkg", "application/octet-stream"},
      {"",nullptr}, {"",nullptr}, {"",nullptr}, {"",nullptr},
      {"",nullptr}, {"",nullptr}, {"",nullptr}, {"",nullptr},
      {"",nullptr}, {"",nullptr}, {"",nullptr}, {"",nullptr},
      {"",nullptr}, {"",nullptr}, {"",nullptr}, {"",nullptr},
#line 637 "auto/extension_to_mime_type.gperf"
      {"pre", "application/vnd.lotus-freelance"},
      {"",nullptr}, {"",nullptr}, {"",nullptr}, {"",nullptr},
      {"",nullptr}, {"",nullptr}, {"",nullptr}, {"",nullptr},
      {"",nullptr},
#line 369 "auto/extension_to_mime_type.gperf"
      {"java", "text/x-java-source"},
      {"",nullptr},
#line 711 "auto/extension_to_mime_type.gperf"
      {"see", "application/vnd.seemail"},
      {"",nullptr}, {"",nullptr}, {"",nullptr}, {"",nullptr},
      {"",nullptr}, {"",nullptr}, {"",nullptr},
#line 336 "auto/extension_to_mime_type.gperf"
      {"i2g", "application/vnd.intergeo"},
      {"",nullptr}, {"",nullptr}, {"",nullptr}, {"",nullptr},
      {"",nullptr}, {"",nullptr}, {"",nullptr}, {"",nullptr},
      {"",nullptr}, {"",nullptr},
#line 310 "auto/extension_to_mime_type.gperf"
      {"gxf", "application/gxf"},
      {"",nullptr}, {"",nullptr}, {"",nullptr},
#line 721 "auto/extension_to_mime_type.gperf"
      {"sfv", "text/x-sfv"},
#line 295 "auto/extension_to_mime_type.gperf"
      {"gnumeric", "application/x-gnumeric"},
      {"",nullptr}, {"",nullptr}, {"",nullptr}, {"",nullptr},
      {"",nullptr}, {"",nullptr}, {"",nullptr},
#line 249 "auto/extension_to_mime_type.gperf"
      {"fe_launch", "application/vnd.denovo.fcselayout-link"},
      {"",nullptr}, {"",nullptr}, {"",nullptr}, {"",nullptr},
      {"",nullptr}, {"",nullptr}, {"",nullptr}, {"",nullptr},
      {"",nullptr}, {"",nullptr},
#line 303 "auto/extension_to_mime_type.gperf"
      {"grv", "application/vnd.groove-injector"},
      {"",nullptr}, {"",nullptr}, {"",nullptr}, {"",nullptr},
      {"",nullptr}, {"",nullptr}, {"",nullptr}, {"",nullptr},
      {"",nullptr},
#line 658 "auto/extension_to_mime_type.gperf"
      {"qxd", "application/vnd.quark.quarkxpress"},
      {"",nullptr}, {"",nullptr}, {"",nullptr},
#line 289 "auto/extension_to_mime_type.gperf"
      {"ggt", "application/vnd.geogebra.tool"},
      {"",nullptr}, {"",nullptr}, {"",nullptr},
#line 472 "auto/extension_to_mime_type.gperf"
      {"mny", "application/x-msmoney"},
      {"",nullptr}, {"",nullptr}, {"",nullptr}, {"",nullptr},
      {"",nullptr}, {"",nullptr}, {"",nullptr}, {"",nullptr},
      {"",nullptr}, {"",nullptr}, {"",nullptr}, {"",nullptr},
      {"",nullptr},
#line 841 "auto/extension_to_mime_type.gperf"
      {"unityweb", "application/vnd.unity"},
#line 572 "auto/extension_to_mime_type.gperf"
      {"otg", "application/vnd.oasis.opendocument.graphics-template"},
#line 814 "auto/extension_to_mime_type.gperf"
      {"tgv", "application/x-tgwallpattern"},
      {"",nullptr}, {"",nullptr}, {"",nullptr}, {"",nullptr},
      {"",nullptr}, {"",nullptr}, {"",nullptr}, {"",nullptr},
      {"",nullptr}, {"",nullptr}, {"",nullptr}, {"",nullptr},
      {"",nullptr},
#line 526 "auto/extension_to_mime_type.gperf"
      {"ngdat", "application/vnd.nokia.n-gage.data"},
      {"",nullptr}, {"",nullptr}, {"",nullptr}, {"",nullptr},
      {"",nullptr}, {"",nullptr}, {"",nullptr}, {"",nullptr},
      {"",nullptr}, {"",nullptr}, {"",nullptr}, {"",nullptr},
#line 20 "auto/extension_to_mime_type.gperf"
      {"3g2", "video/3gpp2"},
      {"",nullptr}, {"",nullptr}, {"",nullptr}, {"",nullptr},
      {"",nullptr}, {"",nullptr}, {"",nullptr},
#line 345 "auto/extension_to_mime_type.gperf"
      {"iges", "model/iges"},
      {"",nullptr},
#line 608 "auto/extension_to_mime_type.gperf"
      {"pgn", "application/x-chess-pgn"},
      {"",nullptr}, {"",nullptr}, {"",nullptr}, {"",nullptr},
      {"",nullptr}, {"",nullptr},
#line 311 "auto/extension_to_mime_type.gperf"
      {"gxt", "application/vnd.geonext"},
      {"",nullptr}, {"",nullptr}, {"",nullptr},
#line 660 "auto/extension_to_mime_type.gperf"
      {"qxt", "application/vnd.quark.quarkxpress"},
      {"",nullptr}, {"",nullptr}, {"",nullptr}, {"",nullptr},
      {"",nullptr}, {"",nullptr}, {"",nullptr}, {"",nullptr},
      {"",nullptr},
#line 236 "auto/extension_to_mime_type.gperf"
      {"ext", "application/vnd.novadigm.ext"},
      {"",nullptr}, {"",nullptr}, {"",nullptr}, {"",nullptr},
      {"",nullptr},
#line 560 "auto/extension_to_mime_type.gperf"
      {"onetmp", "application/onenote"},
      {"",nullptr}, {"",nullptr}, {"",nullptr}, {"",nullptr},
      {"",nullptr}, {"",nullptr}, {"",nullptr}, {"",nullptr},
      {"",nullptr},
#line 476 "auto/extension_to_mime_type.gperf"
      {"movie", "video/x-sgi-movie"},
      {"",nullptr}, {"",nullptr}, {"",nullptr},
#line 726 "auto/extension_to_mime_type.gperf"
      {"sh", "application/x-sh"},
      {"",nullptr},
#line 215 "auto/extension_to_mime_type.gperf"
      {"efif", "application/vnd.picsel"},
      {"",nullptr}, {"",nullptr}, {"",nullptr}, {"",nullptr},
      {"",nullptr},
#line 561 "auto/extension_to_mime_type.gperf"
      {"onetoc", "application/onenote"},
      {"",nullptr}, {"",nullptr}, {"",nullptr}, {"",nullptr},
      {"",nullptr},
#line 659 "auto/extension_to_mime_type.gperf"
      {"qxl", "application/vnd.quark.quarkxpress"},
      {"",nullptr},
#line 252 "auto/extension_to_mime_type.gperf"
      {"fh", "image/x-freehand"},
      {"",nullptr}, {"",nullptr}, {"",nullptr}, {"",nullptr},
      {"",nullptr}, {"",nullptr}, {"",nullptr}, {"",nullptr},
      {"",nullptr}, {"",nullptr}, {"",nullptr}, {"",nullptr},
      {"",nullptr}, {"",nullptr}, {"",nullptr}, {"",nullptr},
      {"",nullptr}, {"",nullptr}, {"",nullptr}, {"",nullptr},
      {"",nullptr}, {"",nullptr}, {"",nullptr},
#line 411 "auto/extension_to_mime_type.gperf"
      {"lnk", "application/x-ms-shortcut"},
      {"",nullptr}, {"",nullptr}, {"",nullptr}, {"",nullptr},
#line 718 "auto/extension_to_mime_type.gperf"
      {"setreg", "application/set-registration-initiation"},
      {"",nullptr}, {"",nullptr},
#line 471 "auto/extension_to_mime_type.gperf"
      {"mng", "video/x-mng"},
      {"",nullptr}, {"",nullptr},
#line 620 "auto/extension_to_mime_type.gperf"
      {"png", "image/png"},
      {"",nullptr}, {"",nullptr}, {"",nullptr},
#line 314 "auto/extension_to_mime_type.gperf"
      {"h263", "video/h263"},
#line 286 "auto/extension_to_mime_type.gperf"
      {"geo", "application/vnd.dynageo"},
      {"",nullptr}, {"",nullptr}, {"",nullptr}, {"",nullptr},
#line 371 "auto/extension_to_mime_type.gperf"
      {"jlt", "application/vnd.hp-jlyt"},
      {"",nullptr}, {"",nullptr}, {"",nullptr},
#line 315 "auto/extension_to_mime_type.gperf"
      {"h264", "video/h264"},
      {"",nullptr}, {"",nullptr}, {"",nullptr}, {"",nullptr},
      {"",nullptr}, {"",nullptr},
#line 519 "auto/extension_to_mime_type.gperf"
      {"n-gage", "application/vnd.nokia.n-gage.symbian.install"},
      {"",nullptr}, {"",nullptr}, {"",nullptr}, {"",nullptr},
      {"",nullptr}, {"",nullptr}, {"",nullptr}, {"",nullptr},
      {"",nullptr}, {"",nullptr}, {"",nullptr}, {"",nullptr},
      {"",nullptr}, {"",nullptr}, {"",nullptr}, {"",nullptr},
      {"",nullptr}, {"",nullptr}, {"",nullptr}, {"",nullptr},
      {"",nullptr}, {"",nullptr}, {"",nullptr}, {"",nullptr},
      {"",nullptr}, {"",nullptr},
#line 865 "auto/extension_to_mime_type.gperf"
      {"uvvh", "video/vnd.dece.hd"},
      {"",nullptr}, {"",nullptr}, {"",nullptr}, {"",nullptr},
#line 71 "auto/extension_to_mime_type.gperf"
      {"bh2", "application/vnd.fujitsu.oasysprs"},
      {"",nullptr}, {"",nullptr}, {"",nullptr}, {"",nullptr},
#line 554 "auto/extension_to_mime_type.gperf"
      {"oga", "audio/ogg"},
      {"",nullptr}, {"",nullptr}, {"",nullptr}, {"",nullptr},
      {"",nullptr},
#line 296 "auto/extension_to_mime_type.gperf"
      {"gph", "application/vnd.flographit"},
      {"",nullptr}, {"",nullptr}, {"",nullptr}, {"",nullptr},
#line 354 "auto/extension_to_mime_type.gperf"
      {"ink", "application/inkml+xml"},
      {"",nullptr}, {"",nullptr}, {"",nullptr}, {"",nullptr},
#line 89 "auto/extension_to_mime_type.gperf"
      {"c4g", "application/vnd.clonk.c4group"},
      {"",nullptr}, {"",nullptr}, {"",nullptr}, {"",nullptr},
#line 419 "auto/extension_to_mime_type.gperf"
      {"lzh", "application/x-lzh-compressed"},
      {"",nullptr}, {"",nullptr},
#line 412 "auto/extension_to_mime_type.gperf"
      {"log", "text/plain"},
      {"",nullptr}, {"",nullptr}, {"",nullptr}, {"",nullptr},
      {"",nullptr}, {"",nullptr}, {"",nullptr}, {"",nullptr},
#line 382 "auto/extension_to_mime_type.gperf"
      {"jsonml", "application/jsonml+json"},
      {"",nullptr}, {"",nullptr}, {"",nullptr}, {"",nullptr},
      {"",nullptr}, {"",nullptr}, {"",nullptr}, {"",nullptr},
      {"",nullptr}, {"",nullptr}, {"",nullptr}, {"",nullptr},
      {"",nullptr}, {"",nullptr}, {"",nullptr}, {"",nullptr},
      {"",nullptr}, {"",nullptr}, {"",nullptr}, {"",nullptr},
      {"",nullptr}, {"",nullptr}, {"",nullptr}, {"",nullptr},
      {"",nullptr}, {"",nullptr}, {"",nullptr}, {"",nullptr},
      {"",nullptr}, {"",nullptr}, {"",nullptr}, {"",nullptr},
      {"",nullptr}, {"",nullptr}, {"",nullptr}, {"",nullptr},
      {"",nullptr}, {"",nullptr}, {"",nullptr},
#line 147 "auto/extension_to_mime_type.gperf"
      {"cryptonote", "application/vnd.rig.cryptonote"},
#line 391 "auto/extension_to_mime_type.gperf"
      {"kon", "application/vnd.kde.kontour"},
#line 235 "auto/extension_to_mime_type.gperf"
      {"exi", "application/exi"},
      {"",nullptr}, {"",nullptr}, {"",nullptr}, {"",nullptr},
      {"",nullptr}, {"",nullptr},
#line 73 "auto/extension_to_mime_type.gperf"
      {"blb", "application/x-blorb"},
      {"",nullptr}, {"",nullptr}, {"",nullptr}, {"",nullptr},
      {"",nullptr}, {"",nullptr}, {"",nullptr}, {"",nullptr},
      {"",nullptr}, {"",nullptr}, {"",nullptr},
#line 567 "auto/extension_to_mime_type.gperf"
      {"org", "application/vnd.lotus-organizer"},
      {"",nullptr}, {"",nullptr}, {"",nullptr}, {"",nullptr},
      {"",nullptr}, {"",nullptr}, {"",nullptr}, {"",nullptr},
#line 127 "auto/extension_to_mime_type.gperf"
      {"clkk", "application/vnd.crick.clicker.keyboard"},
      {"",nullptr}, {"",nullptr},
#line 562 "auto/extension_to_mime_type.gperf"
      {"onetoc2", "application/onenote"},
      {"",nullptr}, {"",nullptr}, {"",nullptr}, {"",nullptr},
      {"",nullptr}, {"",nullptr}, {"",nullptr}, {"",nullptr},
      {"",nullptr}, {"",nullptr}, {"",nullptr}, {"",nullptr},
      {"",nullptr},
#line 622 "auto/extension_to_mime_type.gperf"
      {"portpkg", "application/vnd.macports.portpkg"},
      {"",nullptr},
#line 74 "auto/extension_to_mime_type.gperf"
      {"blorb", "application/x-blorb"},
      {"",nullptr}, {"",nullptr}, {"",nullptr}, {"",nullptr},
      {"",nullptr}, {"",nullptr}, {"",nullptr}, {"",nullptr},
      {"",nullptr}, {"",nullptr}, {"",nullptr}, {"",nullptr},
      {"",nullptr},
#line 376 "auto/extension_to_mime_type.gperf"
      {"jpg", "image/jpeg"},
      {"",nullptr}, {"",nullptr}, {"",nullptr}, {"",nullptr},
      {"",nullptr}, {"",nullptr}, {"",nullptr}, {"",nullptr},
#line 603 "auto/extension_to_mime_type.gperf"
      {"pfb", "application/x-font-type1"},
      {"",nullptr}, {"",nullptr},
#line 556 "auto/extension_to_mime_type.gperf"
      {"ogv", "video/ogg"},
      {"",nullptr}, {"",nullptr}, {"",nullptr}, {"",nullptr},
      {"",nullptr}, {"",nullptr}, {"",nullptr}, {"",nullptr},
#line 37 "auto/extension_to_mime_type.gperf"
      {"ahead", "application/vnd.ahead.space"},
      {"",nullptr}, {"",nullptr}, {"",nullptr}, {"",nullptr},
      {"",nullptr}, {"",nullptr},
#line 374 "auto/extension_to_mime_type.gperf"
      {"jpe", "image/jpeg"},
#line 885 "auto/extension_to_mime_type.gperf"
      {"vob", "video/x-ms-vob"},
      {"",nullptr}, {"",nullptr}, {"",nullptr}, {"",nullptr},
      {"",nullptr}, {"",nullptr}, {"",nullptr}, {"",nullptr},
      {"",nullptr}, {"",nullptr}, {"",nullptr}, {"",nullptr},
      {"",nullptr}, {"",nullptr}, {"",nullptr}, {"",nullptr},
      {"",nullptr}, {"",nullptr}, {"",nullptr}, {"",nullptr},
      {"",nullptr}, {"",nullptr}, {"",nullptr}, {"",nullptr},
      {"",nullptr}, {"",nullptr}, {"",nullptr}, {"",nullptr},
      {"",nullptr}, {"",nullptr}, {"",nullptr}, {"",nullptr},
#line 372 "auto/extension_to_mime_type.gperf"
      {"jnlp", "application/x-java-jnlp-file"},
      {"",nullptr}, {"",nullptr}, {"",nullptr}, {"",nullptr},
      {"",nullptr}, {"",nullptr}, {"",nullptr}, {"",nullptr},
      {"",nullptr}, {"",nullptr}, {"",nullptr}, {"",nullptr},
      {"",nullptr}, {"",nullptr}, {"",nullptr}, {"",nullptr},
      {"",nullptr}, {"",nullptr}, {"",nullptr}, {"",nullptr},
      {"",nullptr}, {"",nullptr}, {"",nullptr}, {"",nullptr},
      {"",nullptr},
#line 381 "auto/extension_to_mime_type.gperf"
      {"json", "application/json"},
      {"",nullptr}, {"",nullptr}, {"",nullptr}, {"",nullptr},
#line 378 "auto/extension_to_mime_type.gperf"
      {"jpgv", "video/jpeg"},
      {"",nullptr}, {"",nullptr}, {"",nullptr}, {"",nullptr},
      {"",nullptr}, {"",nullptr}, {"",nullptr}, {"",nullptr},
      {"",nullptr}, {"",nullptr}, {"",nullptr}, {"",nullptr},
      {"",nullptr}, {"",nullptr}, {"",nullptr}, {"",nullptr},
      {"",nullptr}, {"",nullptr}, {"",nullptr}, {"",nullptr},
      {"",nullptr}, {"",nullptr}, {"",nullptr}, {"",nullptr},
      {"",nullptr}, {"",nullptr}, {"",nullptr}, {"",nullptr},
      {"",nullptr}, {"",nullptr}, {"",nullptr}, {"",nullptr},
      {"",nullptr}, {"",nullptr}, {"",nullptr}, {"",nullptr},
      {"",nullptr}, {"",nullptr}, {"",nullptr}, {"",nullptr},
      {"",nullptr}, {"",nullptr}, {"",nullptr}, {"",nullptr},
      {"",nullptr}, {"",nullptr},
#line 670 "auto/extension_to_mime_type.gperf"
      {"rgb", "image/x-rgb"},
      {"",nullptr}, {"",nullptr}, {"",nullptr}, {"",nullptr},
      {"",nullptr}, {"",nullptr}, {"",nullptr}, {"",nullptr},
#line 302 "auto/extension_to_mime_type.gperf"
      {"gre", "application/vnd.geometry-explorer"},
      {"",nullptr},
#line 525 "auto/extension_to_mime_type.gperf"
      {"nfo", "text/x-nfo"},
#line 792 "auto/extension_to_mime_type.gperf"
      {"sxg", "application/vnd.sun.xml.writer.global"},
      {"",nullptr}, {"",nullptr}, {"",nullptr}, {"",nullptr},
      {"",nullptr}, {"",nullptr}, {"",nullptr}, {"",nullptr},
      {"",nullptr}, {"",nullptr}, {"",nullptr}, {"",nullptr},
      {"",nullptr}, {"",nullptr}, {"",nullptr}, {"",nullptr},
      {"",nullptr}, {"",nullptr},
#line 448 "auto/extension_to_mime_type.gperf"
      {"mesh", "model/mesh"},
      {"",nullptr}, {"",nullptr},
#line 343 "auto/extension_to_mime_type.gperf"
      {"ifb", "text/calendar"},
      {"",nullptr}, {"",nullptr}, {"",nullptr}, {"",nullptr},
      {"",nullptr}, {"",nullptr}, {"",nullptr}, {"",nullptr},
      {"",nullptr}, {"",nullptr}, {"",nullptr}, {"",nullptr},
      {"",nullptr}, {"",nullptr}, {"",nullptr}, {"",nullptr},
      {"",nullptr}, {"",nullptr}, {"",nullptr}, {"",nullptr},
      {"",nullptr}, {"",nullptr}, {"",nullptr}, {"",nullptr},
      {"",nullptr}, {"",nullptr}, {"",nullptr}, {"",nullptr},
      {"",nullptr}, {"",nullptr}, {"",nullptr}, {"",nullptr},
      {"",nullptr}, {"",nullptr}, {"",nullptr}, {"",nullptr},
      {"",nullptr}, {"",nullptr}, {"",nullptr}, {"",nullptr},
      {"",nullptr}, {"",nullptr}, {"",nullptr}, {"",nullptr},
      {"",nullptr}, {"",nullptr}, {"",nullptr}, {"",nullptr},
      {"",nullptr}, {"",nullptr}, {"",nullptr}, {"",nullptr},
      {"",nullptr}, {"",nullptr}, {"",nullptr}, {"",nullptr},
      {"",nullptr}, {"",nullptr}, {"",nullptr}, {"",nullptr},
      {"",nullptr}, {"",nullptr}, {"",nullptr}, {"",nullptr},
      {"",nullptr}, {"",nullptr}, {"",nullptr}, {"",nullptr},
      {"",nullptr}, {"",nullptr}, {"",nullptr}, {"",nullptr},
      {"",nullptr}, {"",nullptr}, {"",nullptr}, {"",nullptr},
      {"",nullptr}, {"",nullptr}, {"",nullptr}, {"",nullptr},
      {"",nullptr}, {"",nullptr}, {"",nullptr}, {"",nullptr},
      {"",nullptr},
#line 840 "auto/extension_to_mime_type.gperf"
      {"umj", "application/vnd.umajin"},
      {"",nullptr}, {"",nullptr}, {"",nullptr}, {"",nullptr},
      {"",nullptr}, {"",nullptr}, {"",nullptr},
#line 573 "auto/extension_to_mime_type.gperf"
      {"oth", "application/vnd.oasis.opendocument.text-web"},
      {"",nullptr}, {"",nullptr}, {"",nullptr}, {"",nullptr},
      {"",nullptr}, {"",nullptr}, {"",nullptr}, {"",nullptr},
      {"",nullptr}, {"",nullptr}, {"",nullptr}, {"",nullptr},
      {"",nullptr}, {"",nullptr}, {"",nullptr}, {"",nullptr},
      {"",nullptr}, {"",nullptr}, {"",nullptr}, {"",nullptr},
      {"",nullptr}, {"",nullptr}, {"",nullptr},
#line 385 "auto/extension_to_mime_type.gperf"
      {"kfo", "application/vnd.kde.kformula"},
      {"",nullptr}, {"",nullptr}, {"",nullptr}, {"",nullptr},
      {"",nullptr}, {"",nullptr}, {"",nullptr}, {"",nullptr},
      {"",nullptr}, {"",nullptr}, {"",nullptr}, {"",nullptr},
      {"",nullptr}, {"",nullptr}, {"",nullptr}, {"",nullptr},
      {"",nullptr}, {"",nullptr}, {"",nullptr}, {"",nullptr},
      {"",nullptr}, {"",nullptr}, {"",nullptr}, {"",nullptr},
      {"",nullptr}, {"",nullptr}, {"",nullptr}, {"",nullptr},
      {"",nullptr}, {"",nullptr}, {"",nullptr}, {"",nullptr},
      {"",nullptr}, {"",nullptr}, {"",nullptr}, {"",nullptr},
      {"",nullptr}, {"",nullptr}, {"",nullptr}, {"",nullptr},
      {"",nullptr}, {"",nullptr}, {"",nullptr}, {"",nullptr},
      {"",nullptr}, {"",nullptr}, {"",nullptr}, {"",nullptr},
      {"",nullptr}, {"",nullptr}, {"",nullptr}, {"",nullptr},
      {"",nullptr}, {"",nullptr},
#line 389 "auto/extension_to_mime_type.gperf"
      {"kne", "application/vnd.kinar"},
      {"",nullptr}, {"",nullptr}, {"",nullptr}, {"",nullptr},
      {"",nullptr}, {"",nullptr}, {"",nullptr}, {"",nullptr},
      {"",nullptr}, {"",nullptr}, {"",nullptr}, {"",nullptr},
      {"",nullptr}, {"",nullptr}, {"",nullptr}, {"",nullptr},
      {"",nullptr}, {"",nullptr}, {"",nullptr}, {"",nullptr},
      {"",nullptr}, {"",nullptr}, {"",nullptr}, {"",nullptr},
      {"",nullptr}, {"",nullptr}, {"",nullptr}, {"",nullptr},
      {"",nullptr}, {"",nullptr}, {"",nullptr}, {"",nullptr},
      {"",nullptr}, {"",nullptr}, {"",nullptr}, {"",nullptr},
      {"",nullptr}, {"",nullptr}, {"",nullptr}, {"",nullptr},
      {"",nullptr}, {"",nullptr}, {"",nullptr}, {"",nullptr},
      {"",nullptr}, {"",nullptr}, {"",nullptr}, {"",nullptr},
      {"",nullptr}, {"",nullptr}, {"",nullptr}, {"",nullptr},
      {"",nullptr}, {"",nullptr}, {"",nullptr}, {"",nullptr},
      {"",nullptr}, {"",nullptr}, {"",nullptr}, {"",nullptr},
      {"",nullptr}, {"",nullptr}, {"",nullptr}, {"",nullptr},
      {"",nullptr}, {"",nullptr}, {"",nullptr}, {"",nullptr},
      {"",nullptr}, {"",nullptr}, {"",nullptr}, {"",nullptr},
      {"",nullptr}, {"",nullptr}, {"",nullptr}, {"",nullptr},
      {"",nullptr}, {"",nullptr}, {"",nullptr}, {"",nullptr},
      {"",nullptr},
#line 542 "auto/extension_to_mime_type.gperf"
      {"obj", "application/x-tgif"},
      {"",nullptr}, {"",nullptr}, {"",nullptr}, {"",nullptr},
      {"",nullptr}, {"",nullptr}, {"",nullptr}, {"",nullptr},
      {"",nullptr}, {"",nullptr}, {"",nullptr}, {"",nullptr},
      {"",nullptr}, {"",nullptr}, {"",nullptr}, {"",nullptr},
      {"",nullptr}, {"",nullptr}, {"",nullptr}, {"",nullptr},
      {"",nullptr}, {"",nullptr}, {"",nullptr}, {"",nullptr},
      {"",nullptr}, {"",nullptr}, {"",nullptr}, {"",nullptr},
      {"",nullptr}, {"",nullptr}, {"",nullptr}, {"",nullptr},
      {"",nullptr}, {"",nullptr}, {"",nullptr}, {"",nullptr},
      {"",nullptr}, {"",nullptr}, {"",nullptr}, {"",nullptr},
      {"",nullptr}, {"",nullptr}, {"",nullptr}, {"",nullptr},
      {"",nullptr}, {"",nullptr}, {"",nullptr}, {"",nullptr},
      {"",nullptr}, {"",nullptr}, {"",nullptr}, {"",nullptr},
      {"",nullptr}, {"",nullptr}, {"",nullptr}, {"",nullptr},
      {"",nullptr}, {"",nullptr}, {"",nullptr}, {"",nullptr},
#line 555 "auto/extension_to_mime_type.gperf"
      {"ogg", "audio/ogg"},
      {"",nullptr}, {"",nullptr}, {"",nullptr}, {"",nullptr},
      {"",nullptr}, {"",nullptr},
#line 373 "auto/extension_to_mime_type.gperf"
      {"joda", "application/vnd.joost.joda-archive"},
      {"",nullptr}, {"",nullptr}, {"",nullptr}, {"",nullptr},
      {"",nullptr}, {"",nullptr}, {"",nullptr}, {"",nullptr},
      {"",nullptr}, {"",nullptr}, {"",nullptr}, {"",nullptr},
      {"",nullptr}, {"",nullptr}, {"",nullptr}, {"",nullptr},
      {"",nullptr}, {"",nullptr}, {"",nullptr}, {"",nullptr},
      {"",nullptr}, {"",nullptr}, {"",nullptr}, {"",nullptr},
      {"",nullptr}, {"",nullptr}, {"",nullptr}, {"",nullptr},
      {"",nullptr}, {"",nullptr}, {"",nullptr}, {"",nullptr},
      {"",nullptr}, {"",nullptr}, {"",nullptr}, {"",nullptr},
      {"",nullptr}, {"",nullptr}, {"",nullptr}, {"",nullptr},
      {"",nullptr}, {"",nullptr}, {"",nullptr}, {"",nullptr},
      {"",nullptr}, {"",nullptr}, {"",nullptr}, {"",nullptr},
      {"",nullptr}, {"",nullptr}, {"",nullptr}, {"",nullptr},
      {"",nullptr}, {"",nullptr}, {"",nullptr}, {"",nullptr},
      {"",nullptr}, {"",nullptr}, {"",nullptr},
#line 288 "auto/extension_to_mime_type.gperf"
      {"ggb", "application/vnd.geogebra.file"},
      {"",nullptr}, {"",nullptr}, {"",nullptr}, {"",nullptr},
      {"",nullptr}, {"",nullptr}, {"",nullptr}, {"",nullptr},
      {"",nullptr}, {"",nullptr}, {"",nullptr}, {"",nullptr},
      {"",nullptr}, {"",nullptr}, {"",nullptr}, {"",nullptr},
      {"",nullptr}, {"",nullptr}, {"",nullptr}, {"",nullptr},
      {"",nullptr}, {"",nullptr}, {"",nullptr}, {"",nullptr},
      {"",nullptr}, {"",nullptr}, {"",nullptr},
#line 375 "auto/extension_to_mime_type.gperf"
      {"jpeg", "image/jpeg"},
      {"",nullptr}, {"",nullptr}, {"",nullptr}, {"",nullptr},
      {"",nullptr}, {"",nullptr}, {"",nullptr}, {"",nullptr},
      {"",nullptr}, {"",nullptr}, {"",nullptr}, {"",nullptr},
      {"",nullptr}, {"",nullptr}, {"",nullptr}, {"",nullptr},
      {"",nullptr}, {"",nullptr}, {"",nullptr}, {"",nullptr},
      {"",nullptr}, {"",nullptr}, {"",nullptr}, {"",nullptr},
      {"",nullptr}, {"",nullptr}, {"",nullptr}, {"",nullptr},
      {"",nullptr}, {"",nullptr},
#line 324 "auto/extension_to_mime_type.gperf"
      {"hh", "text/x-c"},
      {"",nullptr}, {"",nullptr}, {"",nullptr}, {"",nullptr},
      {"",nullptr}, {"",nullptr}, {"",nullptr}, {"",nullptr},
#line 657 "auto/extension_to_mime_type.gperf"
      {"qxb", "application/vnd.quark.quarkxpress"},
      {"",nullptr}, {"",nullptr},
#line 77 "auto/extension_to_mime_type.gperf"
      {"book", "application/vnd.framemaker"},
      {"",nullptr}, {"",nullptr}, {"",nullptr}, {"",nullptr},
      {"",nullptr}, {"",nullptr}, {"",nullptr}, {"",nullptr},
      {"",nullptr}, {"",nullptr}, {"",nullptr}, {"",nullptr},
      {"",nullptr}, {"",nullptr}, {"",nullptr}, {"",nullptr},
      {"",nullptr}, {"",nullptr}, {"",nullptr}, {"",nullptr},
      {"",nullptr}, {"",nullptr}, {"",nullptr}, {"",nullptr},
      {"",nullptr}, {"",nullptr}, {"",nullptr}, {"",nullptr},
      {"",nullptr}, {"",nullptr}, {"",nullptr}, {"",nullptr},
      {"",nullptr}, {"",nullptr}, {"",nullptr}, {"",nullptr},
      {"",nullptr}, {"",nullptr}, {"",nullptr}, {"",nullptr},
      {"",nullptr}, {"",nullptr}, {"",nullptr}, {"",nullptr},
      {"",nullptr}, {"",nullptr}, {"",nullptr}, {"",nullptr},
      {"",nullptr}, {"",nullptr}, {"",nullptr}, {"",nullptr},
      {"",nullptr}, {"",nullptr}, {"",nullptr}, {"",nullptr},
      {"",nullptr},
#line 330 "auto/extension_to_mime_type.gperf"
      {"htke", "application/vnd.kenameaapp"},
      {"",nullptr}, {"",nullptr}, {"",nullptr}, {"",nullptr},
      {"",nullptr}, {"",nullptr}, {"",nullptr}, {"",nullptr},
      {"",nullptr}, {"",nullptr}, {"",nullptr}, {"",nullptr},
      {"",nullptr}, {"",nullptr}, {"",nullptr}, {"",nullptr},
      {"",nullptr}, {"",nullptr}, {"",nullptr}, {"",nullptr},
      {"",nullptr}, {"",nullptr}, {"",nullptr}, {"",nullptr},
      {"",nullptr}, {"",nullptr}, {"",nullptr}, {"",nullptr},
      {"",nullptr}, {"",nullptr}, {"",nullptr}, {"",nullptr},
      {"",nullptr}, {"",nullptr}, {"",nullptr}, {"",nullptr},
      {"",nullptr}, {"",nullptr}, {"",nullptr}, {"",nullptr},
      {"",nullptr}, {"",nullptr}, {"",nullptr}, {"",nullptr},
      {"",nullptr}, {"",nullptr}, {"",nullptr}, {"",nullptr},
      {"",nullptr}, {"",nullptr}, {"",nullptr}, {"",nullptr},
      {"",nullptr}, {"",nullptr}, {"",nullptr}, {"",nullptr},
      {"",nullptr}, {"",nullptr}, {"",nullptr}, {"",nullptr},
      {"",nullptr}, {"",nullptr}, {"",nullptr}, {"",nullptr},
      {"",nullptr}, {"",nullptr}, {"",nullptr}, {"",nullptr},
      {"",nullptr}, {"",nullptr}, {"",nullptr}, {"",nullptr},
#line 234 "auto/extension_to_mime_type.gperf"
      {"exe", "application/x-msdownload"},
      {"",nullptr}, {"",nullptr}, {"",nullptr}, {"",nullptr},
      {"",nullptr}, {"",nullptr}, {"",nullptr}, {"",nullptr},
      {"",nullptr}, {"",nullptr}, {"",nullptr}, {"",nullptr},
      {"",nullptr}, {"",nullptr}, {"",nullptr}, {"",nullptr},
      {"",nullptr}, {"",nullptr}, {"",nullptr}, {"",nullptr},
      {"",nullptr}, {"",nullptr}, {"",nullptr}, {"",nullptr},
      {"",nullptr}, {"",nullptr}, {"",nullptr}, {"",nullptr},
      {"",nullptr}, {"",nullptr}, {"",nullptr}, {"",nullptr},
      {"",nullptr}, {"",nullptr}, {"",nullptr}, {"",nullptr},
      {"",nullptr}, {"",nullptr}, {"",nullptr}, {"",nullptr},
#line 559 "auto/extension_to_mime_type.gperf"
      {"onepkg", "application/onenote"}
    };

  if (len <= MAX_WORD_LENGTH && len >= MIN_WORD_LENGTH)
    {
      register unsigned int key = extension_hash (str, len);

      if (key <= MAX_HASH_VALUE)
        {
          register const char *s = wordlist[key].extension;

          if ((((unsigned char)*str ^ (unsigned char)*s) & ~32) == 0 && !gperf_case_strcmp (str, s))
            return &wordlist[key];
        }
    }
  return 0;
}
#line 1010 "auto/extension_to_mime_type.gperf"

const char *extension_to_mime_type(const char *extension, size_t extension_len) {
  const auto &result = search_extension(extension, extension_len);
  if (result == nullptr) {
    return nullptr;
  }

  return result->mime_type;
}
