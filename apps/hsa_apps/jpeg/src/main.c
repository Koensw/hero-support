#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>

// vogelpi
#include "pulp_host.h"
#include "pulp_func.h"
#include "zynq_pmm_user.h"
#include <time.h> // for time measurements

#include <sys/time.h> // for time measurements

#define PROFILE

//#include "ompOffload.h"
#include "utils.h"

#define PULP_CLK_FREQ_MHZ 75
#define REPETITIONS 100

#ifndef WIDTH
#define WIDTH 240
#endif

#ifndef HEIGHT
#define HEIGHT 160
#endif

#define DCTSIZE 8
#define BLKSIZE 64
#define NBLKS   ((WIDTH*HEIGHT) / BLKSIZE)

unsigned long huffbits[]  =
  {
    0xd3767676, 0xad0b7380, 0x08080822, 0x2235ad6e,
    0x575aecfc, 0xbd5a5667, 0x04045114, 0x104446b5,
    0xadc9eb25, 0xa38fa952, 0x5b000208, 0x38510446,
    0xb58dcaea, 0x64af87a7, 0x5a4b0009, 0x1d2852cd,
    0xf7a8888d, 0x635b93d4, 0x2c389a75, 0xdd6402b5,
    0x18d8e73f, 0x7241111b, 0x246bcf74, 0xab062e9c,
    0x25a406e5, 0x468d1c9a, 0xd6d10101, 0x30fa558b,
    0x0f4e26db, 0x408b2267, 0xa8b049b0, 0xb120892a,
    0x62746b1e, 0x16931968, 0x546e3db5, 0x7be2864d,
    0x228c324b, 0x5b45d89d, 0x18cc3d06, 0xc56c523a,
    0x72c48f8d, 0x6dd92ac3, 0x2be0baec, 0x5e806e15,
    0xf48ad8a3, 0x2b39f0ac, 0x8cb13bea, 0xc76228ed,
    0xae2ef898, 0x37c82e0a, 0x15992312, 0x44b6f950,
    0x068dc5de, 0x56e15d75, 0x7b8a0e8e, 0xab902c5b,
    0x6c802223, 0x71f70306, 0xebeb5c51, 0xfa2dc463,
    0xc6e82922, 0x88c6b9b9, 0x1b61856e, 0x4ad715da,
    0xf2c70655, 0x44b565c9, 0x22a22351, 0xd95ae187,
    0x6a5ab766, 0xd591ad63, 0x6ae75957, 0xb2410421,
    0xb98baa18, 0xb626abb3, 0xa6a8d630, 0x4c49e41b,
    0x2587b4b1, 0x0985a418, 0xd3cf26ea, 0x8232363b,
    0x04be46d6, 0x59b32aa4, 0x78ba018f, 0x3dad9731,
    0x11cd1b1e, 0x6d6d1188, 0x3992b513, 0x3f4c3265,
    0xdab51800, 0xe6b33e86, 0xa35a8ae4, 0x51133f5a,
    0x456bdcaa, 0x39051618, 0x71762263, 0x495b3306,
    0xe78a15ad, 0x0fddc47d, 0xcd5af13a, 0xd0e957df,
    0xeb87fbb5, 0x66736c9f, 0x4acfbd66, 0xb3ef59f7,
    0xacd66b35, 0x9acd66aa, 0xab155445, 0x11468d1a,
    0x35764993, 0x93c02714, 0xa2b15ac8, 0xfdc467fd,
    0xafe95071, 0x32fd697a, 0x55f7fad5, 0xfa5591ff,
    0x465e6b35, 0x9acd66b3, 0x559acd55, 0x55555551,
    0xa3468d1a, 0xbbff587e, 0xa694d2f3, 0x5acaffa2,
    0x29ff6c7f, 0x23519fde, 0xa7d697a5, 0x5f7de4e3,
    0xb5591ff4, 0x71f5acd6, 0x6b359aaa, 0xcd66b70a,
    0xf31738c8, 0xacd55562, 0xb14451a3, 0x468d1abc,
    0xff5bf89a, 0x0691b9ad, 0x5c66c4ff, 0xbe293891,
    0x7eb4a78a, 0xbefe0356, 0x27f71f8d, 0x66aaab35,
    0x52cc2307, 0xb9a96e24, 0x3c838fa1, 0xa33bff78,
    0xfe74b331, 0x3d6a3bb2, 0x8df31245, 0x4522c881,
    0x94f1558a, 0xac511468, 0xd1a346af, 0x7fd6fe26,
    0xa81ad44e, 0x6c5fea29, 0x7ef2e7d6, 0x93a0abdf,
    0xbab5627f, 0x74df5aaa, 0xaaab8b9d, 0x84a2f5ee,
    0x69a5e702, 0x9df26b35, 0x19c1a279, 0xab7b830c,
    0x808fba7a, 0x8a8019d3, 0x746370f5, 0xaf224c7d,
    0xdfd6bc89, 0x3fb87f3a, 0xf225fee7, 0xeb5f6797,
    0xfb9fad1b, 0x797fb86b, 0xecf3138d, 0x847b9e94,
    0x2c723fd6, 0x60ffb946, 0xd1039437, 0x0a1bd303,
    0x3fce9ec9, 0x51493313, 0xec13ffaf, 0x57e009ce,
    0x0e467d2a, 0xaafb9b39, 0x2810587d, 0x693eed5e,
    0x7fab5fad, 0x589f91be, 0xb55559a7, 0x6da85bd0,
    0x53ca0b13, 0xdcd7534c, 0x2b142b1c, 0x74aad22e,
    0x19a27b72, 0x46d1c814, 0x608bba7f, 0xe3c7fc6b,
    0xc98bfe79, 0xffe3c7fc, 0x6bca8ffe, 0x798fd6bc,
    0xb5fee2ff, 0xdf35b107, 0xf027fdf2, 0x2b083b20,
    0xfc056f51, 0xfc407e35, 0xe67a3feb, 0x4ccc47de,
    0x27f1abef, 0xf5bf8d0a, 0xabce6d25, 0xff76bd2a,
    0x33f28abc, 0xe621f5ab, 0x1e8ff5aa, 0xaaaba38b,
    0x76fa51eb, 0x51a64579, 0x59a10e3b, 0x50828c40,
    0x0a9531c8, 0xab072972, 0x3048c8c7, 0x148c7382,
    0x73f5a62a, 0xa3240fca, 0x9a7841e7, 0xf957da2d,
    0xfdbfef9a, 0x13c27a7f, 0x2a6b9854, 0xf24fe55f,
    0x6b8719c9, 0xfca83065, 0x0c3a119a, 0x6e957ffe,
    0xbbf3aaab, 0x9ff8f697, 0xfdd35d79, 0xa8cfcb57,
    0x5cc3f8d5, 0x91fbff85, 0x66b35553, 0x2ef89947,
    0x7145487d, 0xa7ae6861, 0x16bcddbd, 0x54d24c8d,
    0x45940a69, 0x63ce3352, 0x90ca715a, 0x747990b9,
    0xe80714bf, 0x7855c1c2, 0xa9c719a9, 0x41df9c81,
    0x5d08f98e, 0x3dea35dd, 0xc8e84f5c, 0x53c6a460,
    0xf14f091c, 0x839a807e, 0xe23cff74, 0x5374ad43,
    0xfd77e755, 0x53f36f20, 0xff64d76a, 0x8cfca2ae,
    0x7fd49fad, 0x591f99ff, 0x0aaacd54, 0xcc5530bc,
    0x13de8aab, 0xc809e5bb, 0x9a68f70e, 0x28c4d9e4,
    0x935e5e1f, 0xd2a41903, 0x06821049, 0x03f4a642,
    0xcbc8009a, 0xb3184207, 0x6e287515, 0x70bbd000,
    0x71cd3200, 0xd8624834, 0xd10ce060, 0x63be2a32,
    0x41da016f, 0x514e3804, 0xfca7fdae, 0x2a4e1720,
    0x1cfa0a80, 0xe6043eaa, 0x29ab5118, 0x941fad55,
    0x4bcc2ffe, 0xe9fe55d8, 0xd447e415, 0x71fea4d5,
    0x99f9dbe9, 0x59acd554, 0x8bb97029, 0x94231a56,
    0xaa6601b9, 0x34c578c1, 0xa038a90f, 0xa55ae029,
    0xf53cd0ea, 0x29c12054, 0xd1bbf017, 0x8a8ede40,
    0xe771dcbd, 0x813d682b, 0xaa12a837, 0x7a66bca7,
    0x700caa49, 0xf4cf4a78, 0xa43c2c78, 0xfc45440a,
    0xc4808c10, 0x318a35a9, 0x7faf1554, 0xfcc6c3fd,
    0x9346a13f, 0x20ab8ff5, 0x2d5687f7, 0xadf4aaaa,
    0xaa94618f, 0x34b5bb02, 0x9b696c9c, 0x51c678c5,
    0x06e2911a, 0x5976afd6, 0xbcb11baa, 0x8feed0eb,
    0x5dab8aaa, 0xaa346b53, 0xff5c2aa9, 0xbee9fa57,
    0xad407318, 0xa9ffd4b7, 0xd2ad3fd6, 0x9fa5566a,
    0xa914bb85, 0x1d49abe8, 0x1a090839, 0x20f43eb4,
    0x0e2b20f1, 0x5e58ea00, 0xad9ed8fa, 0x5741d6ad,
    0x118ee903, 0x32f38e0e, 0x2a4fbc0f, 0x7c550aaa,
    0xaaa268d6, 0xa9feb96b, 0x359a347a, 0x9ab73fbb,
    0x152ffaa6, 0xfa55affa, 0xefc2aaaa, 0x34691c22,
    0x0c9356f6, 0x6213bd8e, 0xe6c74f4a, 0xba884c85,
    0x1bf0f6a9, 0xe1781f0c, 0x38ec681a, 0x0f4cf48a,
    0x64638e07, 0xad5bae20, 0x603b1a90, 0x90c838c1,
    0x1542aab3, 0x52b158c9, 0x5193e951, 0x48644dc5,
    0x76d1344d, 0x6a9feb57, 0xf1aacd53, 0x7de3f5ab,
    0x63fbb152, 0x7fab6fa5, 0x5b7fae1f, 0x4aaa8e36,
    0x9182a0c9, 0x356d02c1, 0x1e072c7e, 0xf1f5aa71,
    0xebc8a922, 0x59176b00, 0x455ce9ef, 0x192d1f2b,
    0xe83a8a21, 0x8706b1eb, 0x50aed8c7, 0xbf3509f9,
    0x187bd480, 0xe50fa566, 0x81e2b359, 0xacd1359a,
    0x26844cc3, 0x2197f3ad, 0x590abc67, 0x2a739e87,
    0xe9559a1d, 0x69fefb7d, 0x6ad8feec, 0x53ffab6f,
    0xa55b9fdf, 0x2d66aded, 0xde73f2f0, 0xa3a93504,
    0x0902e147, 0x3dc9ea6b, 0x35d0f14e, 0xdc570466,
    0x88cd4f6a, 0x928ce30d, 0xea28c056, 0x6db210a0,
    0x7735226d, 0xc11f74f4, 0xa84e15be, 0xb4ec0aaf,
    0x6f6359a0, 0x78acd6ea, 0xf9bd0fe5, 0x586feeb7,
    0xe55e5ca7, 0xfe59bffd, 0xf268cabf, 0xde1f9d46,
    0x0344ac00, 0x395eb5ab, 0x758bf1fe, 0x95540d4b,
    0xfeb5fea6, 0xad8feec5, 0x37dc6fa5, 0x41feb96a,
    0xcecccb87, 0x9384ec3d, 0x6800a000, 0x300551cf,
    0xad31c753, 0x58e7ad36, 0x55b22b86, 0x191441ad,
    0x525125d1, 0x5038418f, 0xc6aca6dd, 0x6c1187dd,
    0xe0d20c6f, 0x02a56dc1, 0x0e31cf43, 0x44f150ba,
    0x0705f180, 0x3b8cd09a, 0x12c02c71, 0x9fa633f9,
    0x1af3147f, 0x747d4018, 0xa79d48c0, 0x9769f518,
    0xa8ee5570, 0x243bc67e, 0xf6cc63eb, 0x4d25baf0,
    0xc53e8456, 0x74f1fc30, 0x0ff808af, 0x3edcb6d4,
    0x75f60056, 0xb03062ff, 0x817f4acd, 0x66b3537f,
    0xae7ff78d, 0x5b7dca3f, 0x74fd2b49, 0xb132b89e,
    0x40422fdd, 0x1eb42b20, 0x7535b97d, 0x45641e86,
    0x9b06b233, 0xd69f9622, 0x836d39a7, 0x3b632c3e,
    0x957a9b64, 0xcf4ab16c, 0x48cbea33, 0x49d49f61,
    0x5201b41a, 0x6e95da8a, 0x96200a8a, 0x5fb2a709,
    0x9c9e7d6a, 0x1bc8a519, 0x071f5a77, 0x42558c9b,
    0x71e87afe, 0x146e211d, 0x587e028d, 0xdc23f89b,
    0xf2a92e2d, 0x9d70dbc8, 0xad69a33e, 0x488d9c81,
    0xbbef7e15, 0x9acd54df, 0xebdffde3, 0x56fc2541,
    0x1f9d204e, 0xddfe9512, 0x8440aa30, 0x074a67c7,
    0x03ad1392, 0x73cd0231, 0x83592a78, 0x342538c1,
    0xa246ec8e, 0xf4c73cd3, 0x7634e4e0, 0x03d2b500,
    0x36123a83, 0x5667fd24, 0x73c1159d, 0xa327a77a,
    0x949d808e, 0x99a3d2bb, 0x520249c7, 0x6e6a6ff5,
    0x7f88a831, 0x93f2e3e9, 0x46874a34, 0x6b53ff96,
    0x7f8ff4a0, 0x6b359a9f, 0xfd7bffbc, 0x6adfeed5,
    0x9c5b100e, 0xe7934cfd, 0x8566b359, 0xace2b347,
    0x8a3cae45, 0x019522b3, 0x918357c8, 0x42b77cd5,
    0xb1c5c211, 0x4c01041e, 0x86997108, 0x0beb4687,
    0x4a8fef1a, 0x9394c75a, 0x5524ed07, 0xe6eb81da,
    0x973b46ee, 0xbdebb51a, 0x35a9ffcb, 0x3fc7fa52,
    0xc27bb0fc, 0x28429ea6, 0x82c63f87, 0xf3a30db9,
    0x3930c64f, 0xa9514891, 0x29f96341, 0xf4514240,
    0x091de836, 0x79acd035, 0x555d4628, 0x77a1f7ff,
    0x0a7f9643, 0xe86ae977, 0x447b9140, 0xec901ee0,
    0xe681ca02, 0x2bcc50a5, 0x79ce68c9, 0xed42438e,
    0x94252a73, 0x8af304a8, 0x7660b7a5, 0x407f78cc,
    0xa195fa36, 0x78c53124, 0xe4f5a06a, 0x89ad4bac,
    0x7f8ff4a0
  };




unsigned char val_dc_lum[]  =
  {
    0x00, 0x01, 0x02, 0x03, 
    0x04, 0x05, 0x06, 0x07, 
    0x08, 0x09, 0x0A, 0x0B
  };

unsigned char val_ac_lum[]  =
  {
    0x01, 0x02, 0x03, 0x00, 
    0x04, 0x11, 0x05, 0x12, 
    0x21, 0x31, 0x41, 0x06, 
    0x13, 0x51, 0x61, 0x07, 
    0x22, 0x71, 0x14, 0x32, 
    0x81, 0x91, 0xa1, 0x08, 
    0x23, 0x42, 0xb1, 0xc1, 
    0x15, 0x52, 0xd1, 0xf0, 
    0x24, 0x33, 0x62, 0x72, 
    0x82, 0x09, 0x0a, 0x16, 
    0x17, 0x18, 0x19, 0x1a, 
    0x25, 0x26, 0x27, 0x28, 
    0x29, 0x2a, 0x34, 0x35, 
    0x36, 0x37, 0x38, 0x39, 
    0x3a, 0x43, 0x44, 0x45, 
    0x46, 0x47, 0x48, 0x49,
    0x4a, 0x53, 0x54, 0x55,
    0x56, 0x57, 0x58, 0x59,
    0x5a, 0x63, 0x64, 0x65,
    0x66, 0x67, 0x68, 0x69,
    0x6a, 0x73, 0x74, 0x75,
    0x76, 0x77, 0x78, 0x79,
    0x7a, 0x83, 0x84, 0x85,
    0x86, 0x87, 0x88, 0x89,
    0x8a, 0x92, 0x93, 0x94,
    0x95, 0x96, 0x97, 0x98,
    0x99, 0x9a, 0xa2, 0xa3,
    0xa4, 0xa5, 0xa6, 0xa7,
    0xa8, 0xa9, 0xaa, 0xb2,
    0xb3, 0xb4, 0xb5, 0xb6,
    0xb7, 0xb8, 0xb9, 0xba,
    0xc2, 0xc3, 0xc4, 0xc5,
    0xc6, 0xc7, 0xc8, 0xc9,
    0xca, 0xd2, 0xd3, 0xd4,
    0xd5, 0xd6, 0xd7, 0xd8,
    0xd9, 0xda, 0xe1, 0xe2,
    0xe3, 0xe4, 0xe5, 0xe6,
    0xe7, 0xe8, 0xe9, 0xea,
    0xf1, 0xf2, 0xf3, 0xf4,
    0xf5, 0xf6, 0xf7, 0xf8,
    0xf9, 0xfa
  };

unsigned char zz_tbl[32]  =
  {
    0,  1,  8,  16,
    9,  2,  3,  10,
    17, 24, 32, 25,
    18, 11, 4,  5,
    12, 19, 26, 33,
    40, 48, 41, 34,
    27, 20, 13, 6,
    7,  14, 21, 28
  };

int mincode_dc[]  = {
  0x0,
  0x0,
  0x0,
  0x2,
  0xe,
  0x1e,
  0x3e,
  0x7e,
  0xfe,
  0x1fe
};


int maxcode_dc[]  = {
  0x0,
  0xffffffff,
  0x0,
  0x6,
  0xe,
  0x1e,
  0x3e,
  0x7e,
  0xfe,
  0x1ff
};


unsigned char valptr_dc[] = {
  0,  0,   0,   1,
  6,  7,   8,   9,
  10, 11
};

int mincode_ac[] = {
  0x0,
  0x0,
  0x0,
  0x4,
  0xa,
  0x1a,
  0x3a,
  0x78,
  0xf8,
  0x1f6,
  0x3f6,
  0x7f6,
  0xff4,
  0x0,
  0x0,
  0x7fc0,
  0xff82
};


int maxcode_ac[]  = {
  0x0,
  0xffffffff,
  0x1,
  0x4,
  0xc,
  0x1c,
  0x3b,
  0x7b,
  0xfa,
  0x1fa,
  0x3fa,
  0x7f9,
  0xff7,
  0xffffffff,
  0xffffffff,
  0x7fc0,
  0xffff
};

unsigned char valptr_ac[]  = {
  0,   0,   0,   2,
  3,   6,   9,  11,
  15,  18,  23, 28,
  32,  0,   0,  36,
  37
};

unsigned char qtbl_lum[] =
  {
    16, 11, 10, 16, 24, 40, 51, 61,
    12, 12, 14, 19, 26, 58, 60, 55, 
    14, 13, 16, 24, 40, 57, 69, 56,
    14, 17, 22, 29, 51, 87, 80, 62, 
    18, 22, 37, 56, 68, 109, 103, 77,
    24, 35, 55, 64, 81, 104, 113, 92,
    49, 64, 78, 87, 103, 121, 120, 101,
    72, 92, 95, 98, 112, 100, 103, 99
  };


/* Huff AC Dec*/
#define EOB 0x00
#define ZRL 0xF0

/* FAST IDCT */
#define INT32 int

#define ONE 1L

#define SIN_1_4 46341
#define COS_1_4 46341

#define SIN_1_8 25080
#define COS_1_8 60547
#define SIN_3_8 60547
#define COS_3_8 25080

#define SIN_1_16 12785
#define COS_1_16 64277
#define SIN_7_16 64277
#define COS_7_16 12785

#define SIN_3_16 36410
#define COS_3_16 54491
#define SIN_5_16 54491
#define COS_5_16 36410

#define OSIN_1_4 11585
#define OCOS_1_4 11585

#define OSIN_1_8 6270
#define OCOS_1_8 15137
#define OSIN_3_8 15137
#define OCOS_3_8 6270

#define OSIN_1_16 3196
#define OCOS_1_16 16069
#define OSIN_7_16 16069
#define OCOS_7_16 3196

#define OSIN_3_16 9102
#define OCOS_3_16 13623
#define OSIN_5_16 13623
#define OCOS_5_16 9102

#ifdef RIGHT_SHIFT_IS_UNSIGNED
#define LG2_DCT_SCALE 15
#define RIGHT_SHIFT(_x,_shft)   ((((_x) + (ONE << 30)) >> (_shft)) - (ONE << (30 - (_shft))))
#else
#define LG2_DCT_SCALE 16
#define RIGHT_SHIFT(_x,_shft)   ((_x) >> (_shft))
#endif

#define DCT_SCALE (ONE << LG2_DCT_SCALE)

#define LG2_OVERSCALE 2
#define OVERSCALE (ONE << LG2_OVERSCALE)

#define UNFIX(x)   RIGHT_SHIFT((x) + (ONE << (LG2_DCT_SCALE-1)), LG2_DCT_SCALE)
#define UNFIXH(x)  RIGHT_SHIFT((x) + (ONE << LG2_DCT_SCALE), LG2_DCT_SCALE+1)
#define UNFIXO(x)  RIGHT_SHIFT((x) + (ONE << (LG2_DCT_SCALE-1-LG2_OVERSCALE)), LG2_DCT_SCALE-LG2_OVERSCALE)
#define OVERSH(x)   ((x) << LG2_OVERSCALE)

#define fast_idct_8_macro(in, stride)			\
  {							\
    INT32 tmp10, tmp11, tmp12, tmp13;			\
    INT32 tmp20, tmp21, tmp22, tmp23;			\
    INT32 tmp30, tmp31;					\
    INT32 tmp40, tmp41, tmp42, tmp43;			\
    INT32 tmp50, tmp51, tmp52, tmp53;			\
    INT32 in0, in1, in2, in3, in4, in5, in6, in7;	\
							\
    in0 = (in)[       0];				\
    in1 = (in)[stride  ];				\
    in2 = (in)[stride*2];				\
    in3 = (in)[stride*3];				\
    in4 = (in)[stride*4];				\
    in5 = (in)[stride*5];				\
    in6 = (in)[stride*6];				\
    in7 = (in)[stride*7];				\
							\
    tmp10 = (in0 + in4) * COS_1_4;			\
    tmp11 = (in0 - in4) * COS_1_4;			\
    tmp12 = in2 * SIN_1_8 - in6 * COS_1_8;		\
    tmp13 = in6 * SIN_1_8 + in2 * COS_1_8;		\
							\
    tmp20 = tmp10 + tmp13;				\
    tmp21 = tmp11 + tmp12;				\
    tmp22 = tmp11 - tmp12;				\
    tmp23 = tmp10 - tmp13;				\
							\
    tmp30 = UNFIXO((in3 + in5) * COS_1_4);		\
    tmp31 = UNFIXO((in3 - in5) * COS_1_4);		\
							\
    tmp40 = OVERSH(in1) + tmp30;			\
    tmp41 = OVERSH(in7) + tmp31;			\
    tmp42 = OVERSH(in1) - tmp30;			\
    tmp43 = OVERSH(in7) - tmp31;			\
							\
    tmp50 = tmp40 * OCOS_1_16 + tmp41 * OSIN_1_16;	\
    tmp51 = tmp40 * OSIN_1_16 - tmp41 * OCOS_1_16;	\
    tmp52 = tmp42 * OCOS_5_16 + tmp43 * OSIN_5_16;	\
    tmp53 = tmp42 * OSIN_5_16 - tmp43 * OCOS_5_16;	\
							\
    (in)[       0] = UNFIXH(tmp20 + tmp50);		\
    (in)[stride  ] = UNFIXH(tmp21 + tmp53);		\
    (in)[stride*2] = UNFIXH(tmp22 + tmp52);		\
    (in)[stride*3] = UNFIXH(tmp23 + tmp51);		\
    (in)[stride*4] = UNFIXH(tmp23 - tmp51);		\
    (in)[stride*5] = UNFIXH(tmp22 - tmp52);		\
    (in)[stride*6] = UNFIXH(tmp21 - tmp53);		\
    (in)[stride*7] = UNFIXH(tmp20 - tmp50);		\
  }

int getbit(unsigned int *lastlong, unsigned long **nextlong, int *bitsleft);
void dquantz_lum (short *data, unsigned int blockSize);
void fast_idct_8 (short *in, int stride);
void huff_ac_dec (short *data,unsigned int *lastlong, unsigned long **nextlong, int *bitsleft);
void huff_dc_dec (int *retval,unsigned int *lastlong, unsigned long **nextlong, int *bitsleft);

#include "utils.h"

typedef struct {
  int n_clusters;
  int nbBlocks;
  int BlockMul;
  int blockSize;
  int dctSize;
  short dctData[NBLKS*BLKSIZE];
  unsigned int cycles;
  unsigned char qtblLum[64];
    
#ifdef PROFILE    
  unsigned int t_comp;
  unsigned int t_dma_in;
  unsigned int t_dma_out;
#endif
  unsigned int padding;
    
  DataDesc* data_desc;
  TaskDesc *desc;
#if MEM_SHARING == 1
  TaskDesc *fdesc;
#endif
  //data_desc_t* data_desc;
  //ompOffload_desc_t *desc;
  //omp_offload_t *offload;
  //uint32_t foffload;
} JPEG_kernel_t;

int JPEG_init(JPEG_kernel_t *jpegInstance);
inline void JPEG_offload_out(JPEG_kernel_t *jpegInstance);
inline int JPEG_exe_start(JPEG_kernel_t *jpegInstance);
inline int JPEG_exe_wait(JPEG_kernel_t *jpegInstance);
inline int JPEG_offload_in(JPEG_kernel_t *jpegInstance);
void JPEG_destroy(JPEG_kernel_t *jpegInstance);
            
#define MULTIPLIER 4
#define LOCALBLOCKS 32

// vogelpi
PulpDev pulp_dev;
PulpDev *pulp;

// for time measurement
#define ACC_CTRL 0 
int accumulate_time(struct timespec *tp1, struct timespec *tp2,
		    unsigned *seconds, unsigned long *nanoseconds,
		    int ctrl);
struct timespec res, tp1, tp2, tp1_local, tp2_local;
unsigned s_duration, s_duration1, s_duration2, s_duration3;
unsigned long ns_duration, ns_duration1, ns_duration2, ns_duration3;

#ifdef ZYNQ_PMM
// for cache miss rate measurement
int *zynq_pmm_fd;
int zynq_pmm, ret;
#endif

int main(int argc, char *argv[]) {
  unsigned int i, ii;
  int iter;
  int prev = 0, value;
  short *dct_data;
  int nbBlocks  = NBLKS;
  int blockSize = BLKSIZE;
  int dctSize = DCTSIZE;
#ifdef PIPELINE
  JPEG_kernel_t JPEG_instance[2];
#else
  JPEG_kernel_t JPEG_instance;
#endif
  int error = 0;

  unsigned int lastlong  = 0;
  int bitsleft  = 0;
  unsigned long *nextlong  = huffbits;
    
  // used to boot PULP
  char name[5];
  strcpy(name,"jpeg");
  TaskDesc task_desc;
  task_desc.name = &name[0];

  /* Init the runtime */
  //sthorm_omp_rt_init();
  // vogelpi
  /*
   * Initialization
   */ 
  pulp = &pulp_dev;
  pulp_reserve_v_addr(pulp);
  pulp_mmap(pulp);
  //pulp_print_v_addr(pulp);
  pulp_reset(pulp);
  printf("PULP running at %d MHz\n",pulp_clking_set_freq(pulp,PULP_CLK_FREQ_MHZ));
  pulp_rab_free(pulp,0x0);
  pulp_init(pulp);
  
  pulp_stdout_clear(pulp,0);
  pulp_stdout_clear(pulp,1);
  pulp_stdout_clear(pulp,2);
  pulp_stdout_clear(pulp,3);

  pulp_boot(pulp,&task_desc);

  // setup time measurement
  s_duration = 0, s_duration1 = 0, s_duration2 = 0, s_duration3 = 0;
  ns_duration = 0, ns_duration1 = 0, ns_duration2 = 0, ns_duration3 = 0;
  
#ifdef ZYNQ_PMM
  // setup cache miss rate measurement
  char proc_text[ZYNQ_PMM_PROC_N_CHARS_MAX];
  long long counter_values[(N_ARM_CORES+1)*2];
  double cache_miss_rates[(N_ARM_CORES+1)*2];
  zynq_pmm_fd = &zynq_pmm;
  ret = zynq_pmm_open(zynq_pmm_fd);
  if (ret)
    return ret;    

  // delete the accumulation counters
  ret = zynq_pmm_parse(proc_text, counter_values, -1);
#endif

  // measure time
  clock_getres(CLOCK_REALTIME,&res);
  clock_gettime(CLOCK_REALTIME,&tp1);

#ifndef PIPELINE
  JPEG_init(&JPEG_instance);
    
  for(iter= 0; iter < REPETITIONS; iter++) {
        
    //#ifdef PROFILE
    //printf("\n[APP ] Width %d\n", WIDTH*MULTIPLIER);
    //printf("\n[APP ] Height %d\n", HEIGHT*MULTIPLIER);
    //printf("\n[APP ] Blocks %d\n", NBLKS*MULTIPLIER);
    //#endif

#ifdef ZYNQ_PMM
    zynq_pmm_read(zynq_pmm_fd,proc_text); // reset cache counters
#endif
    clock_gettime(CLOCK_REALTIME,&tp1_local);

    /* Not a DOALL loop. Run sequential */
    for(ii = 0; ii < MULTIPLIER; ++ii)
      {
	lastlong = 0;
	bitsleft = 0;
	nextlong = huffbits;
            
	/* HUFFMAN DC kernel */
	if (DEBUG_LEVEL > 0) printf ("JPEG 1) Huffman DC %d %d..\n",iter,ii);
            
	for (i=0; i<nbBlocks; i++) {
	  huff_dc_dec(&value, &lastlong, &nextlong, &bitsleft);
                
	  JPEG_instance.dctData[i*blockSize] = (value+prev);
	  prev = JPEG_instance.dctData[i*blockSize];
	}
        
	/* HUFFMAN AC kernel */ 
	if (DEBUG_LEVEL > 0) printf ("JPEG 2) Huffman AC %d %d..\n",iter,ii);
        
	/* Not a DOALL loop. Run sequential */
	for (i=0; i< nbBlocks; i++)
	  huff_ac_dec(&JPEG_instance.dctData[i*blockSize],&lastlong, &nextlong, &bitsleft);
            
      }

    clock_gettime(CLOCK_REALTIME,&tp2_local);
    accumulate_time(&tp1_local,&tp2_local,&s_duration2,&ns_duration2,ACC_CTRL);

#ifdef ZYNQ_PMM
    zynq_pmm_read(zynq_pmm_fd, proc_text);
    zynq_pmm_parse(proc_text, counter_values, 1); // accumulate cache counter values
#endif

    if (DEBUG_LEVEL > 0) printf("\n[APP ] Execute offload nbclusters %d\n", JPEG_instance.n_clusters);

    // write command to make PULP continue
    pulp_write32(pulp->mailbox.v_addr,MAILBOX_WRDATA_OFFSET_B,'b',PULP_START);
    
    // offload
    clock_gettime(CLOCK_REALTIME,&tp1_local);
    JPEG_offload_out(&JPEG_instance);
    clock_gettime(CLOCK_REALTIME,&tp2_local);
    accumulate_time(&tp1_local,&tp2_local,&s_duration1,&ns_duration1,ACC_CTRL);

    // start
    ret = JPEG_exe_start(&JPEG_instance);
    if ( ret ) {
      printf("ERROR: Execution start failed. ret = %d\n",ret);
      error = 1;
      break;
    }

    if (DEBUG_LEVEL > 0) printf("[APP ] Offload %d scheduled\n", iter);

    clock_gettime(CLOCK_REALTIME,&tp1_local);
    ret = JPEG_exe_wait(&JPEG_instance);
    clock_gettime(CLOCK_REALTIME,&tp2_local);
    accumulate_time(&tp1_local,&tp2_local,&s_duration3,&ns_duration3,ACC_CTRL);
    if ( ret ) {
      printf("ERROR: Execution wait failed. ret = %d\n",ret);
      error = 1;
      break;
    }

    clock_gettime(CLOCK_REALTIME,&tp1_local);
    ret = JPEG_offload_in(&JPEG_instance);
    clock_gettime(CLOCK_REALTIME,&tp2_local);
    accumulate_time(&tp1_local,&tp2_local,&s_duration1,&ns_duration1,ACC_CTRL);
    if ( ret ) {
      printf("ERROR: Offload in failed. ret = %d\n",ret);
      error = 1;
      break;
    } 
    
    if (DEBUG_LEVEL > 0) printf("\n###########################################################################\n");
    if (DEBUG_LEVEL > 0) printf("\n[APP ] Offload %d terminated\n",iter);  
        
    //#ifdef PROFILE
    //printf("\n[APP ] Kernel Cycles %d\n", JPEG_instance.cycles);
    //printf("\n[APP ] DMA IN Cycles %d\n", JPEG_instance.t_dma_in);
    //printf("\n[APP ] DMA OUT Cycles %d\n", JPEG_instance.t_dma_out);
    //printf("\n[APP ] EXE Cycles %d\n", JPEG_instance.t_comp);
    //printf("\n###########################################################################\n");
    //#endif

    if ((iter % 50) == 49) {
      if (DEBUG_LEVEL > 0) printf("iter = %d \n",iter);
      if (DEBUG_LEVEL > 0) printf("Reset of acc_l3_malloc\n");
      pulp->l3_offset = 0;
    }

    if (DEBUG_LEVEL > 0)  {
      pulp_stdout_print(pulp,0);
      pulp_stdout_print(pulp,1);
      pulp_stdout_print(pulp,2);
      pulp_stdout_print(pulp,3);
    }

  }//iters
    
#else // PIPELINE
  JPEG_init(&JPEG_instance[0]);
  JPEG_init(&JPEG_instance[1]);
    
  int buff_id = 0;
  int next_iter = 0;

  zynq_pmm_read(zynq_pmm_fd,proc_text); // reset cache counters

  clock_gettime(CLOCK_REALTIME,&tp1_local);

  /* Not a DOALL loop. Run sequential */
  for(ii = 0; ii < MULTIPLIER; ++ii) {
    lastlong = 0;
    bitsleft = 0;
    nextlong = huffbits;
            
    /* HUFFMAN DC kernel */
    if (DEBUG_LEVEL > 0) printf ("JPEG 1) Huffman DC %d %d..\n",0, ii);
            
    for (i=0; i<nbBlocks; i++) {
      huff_dc_dec(&value, &lastlong, &nextlong, &bitsleft);
                
      JPEG_instance[buff_id].dctData[i*blockSize] = (value+prev);
      prev = JPEG_instance[buff_id].dctData[i*blockSize];
    }
        
    /* HUFFMAN AC kernel */ 
    if (DEBUG_LEVEL > 0) printf ("JPEG 2) Huffman AC %d %d..\n",0, ii);
        
    /* Not a DOALL loop. Run sequential */
    for (i=0; i< nbBlocks; i++)
      huff_ac_dec(&JPEG_instance[buff_id].dctData[i*blockSize],&lastlong, &nextlong, &bitsleft);
  }
  clock_gettime(CLOCK_REALTIME,&tp2_local);
  accumulate_time(&tp1_local,&tp2_local,&s_duration1,&ns_duration2,ACC_CTRL);

  zynq_pmm_read(zynq_pmm_fd, proc_text);
  zynq_pmm_parse(proc_text, counter_values, 1); // accumulate cache counter values

  if (DEBUG_LEVEL > 0) printf("\n[APP ] Execute offload nbclusters %d\n", JPEG_instance[0].n_clusters);

  clock_gettime(CLOCK_REALTIME,&tp1_local);
  JPEG_launch(&JPEG_instance[buff_id]);
  clock_gettime(CLOCK_REALTIME,&tp2_local);
  accumulate_time(&tp1_local,&tp2_local,&s_duration1,&ns_duration1,ACC_CTRL);
    
  if (DEBUG_LEVEL > 0) printf("[APP ] Offload %d scheduled\n", 0);

  for(iter= 0; iter < REPETITIONS; iter++) {
        
    buff_id = (buff_id == 0) ? 1 : 0;
    next_iter++;

    if (next_iter < REPETITIONS) {

      zynq_pmm_read(zynq_pmm_fd,proc_text); // reset cache counters

      clock_gettime(CLOCK_REALTIME,&tp1_local);
      /* Not a DOALL loop. Run sequential */
      for(ii = 0; ii < MULTIPLIER; ++ii) {
	lastlong = 0;
	bitsleft = 0;
	nextlong = huffbits;
            
	/* HUFFMAN DC kernel */
	if (DEBUG_LEVEL > 0) printf ("JPEG 1) Huffman DC %d %d..\n",next_iter,ii);
            
	for (i=0; i<nbBlocks; i++) {
	  huff_dc_dec(&value, &lastlong, &nextlong, &bitsleft);
                
	  JPEG_instance[buff_id].dctData[i*blockSize] = (value+prev);
	  prev = JPEG_instance[buff_id].dctData[i*blockSize];
	}
        
	/* HUFFMAN AC kernel */ 
	if (DEBUG_LEVEL > 0) printf ("JPEG 2) Huffman AC %d %d..\n",next_iter,ii);
        
	/* Not a DOALL loop. Run sequential */
	for (i=0; i< nbBlocks; i++)
	  huff_ac_dec(&JPEG_instance[buff_id].dctData[i*blockSize],&lastlong, &nextlong, &bitsleft);
      }
      clock_gettime(CLOCK_REALTIME,&tp2_local);
      accumulate_time(&tp1_local,&tp2_local,&s_duration1,&ns_duration2,ACC_CTRL);

      zynq_pmm_read(zynq_pmm_fd, proc_text);
      zynq_pmm_parse(proc_text, counter_values, 1); // accumulate cache counter values

      if (DEBUG_LEVEL > 0) printf("\n[APP ] Execute offload nbclusters %d\n", JPEG_instance[buff_id].n_clusters);
        
      clock_gettime(CLOCK_REALTIME,&tp1_local);
      JPEG_offload_out(&JPEG_instance[buff_id]);
      clock_gettime(CLOCK_REALTIME,&tp2_local);
      accumulate_time(&tp1_local,&tp2_local,&s_duration1,&ns_duration1,ACC_CTRL);

      if (DEBUG_LEVEL > 0) printf("[APP ] Offload %d scheduled\n", next_iter);
    }
    JPEG_wait(&JPEG_instance[!buff_id]);
        
    if (DEBUG_LEVEL > 0) printf("\n###########################################################################\n");
    if (DEBUG_LEVEL > 0) printf("\n[APP ] Offload %d terminated\n",iter);  
        
    //#ifdef PROFILE
    //printf("\n[APP ] Kernel Cycles %d\n", JPEG_instance.cycles);
    //printf("\n[APP ] DMA IN Cycles %d\n", JPEG_instance.t_dma_in);
    //printf("\n[APP ] DMA OUT Cycles %d\n", JPEG_instance.t_dma_out);
    //printf("\n[APP ] EXE Cycles %d\n", JPEG_instance.t_comp);
    //printf("\n###########################################################################\n");
    //#endif

    if ((iter % 50) == 49) {
      if (DEBUG_LEVEL > 0) printf("iter = %d \n",iter);
      if (DEBUG_LEVEL > 0) printf("Reset of acc_l3_malloc\n");
      acc->l3_offset = 0;
    }

  }//iters

#endif // PIPELINE

#if 0
  printf ("JPEG 5) Checksum..\n");
  int npixels = nbBlocks*blockSize;
  int sum = 0;
  int j,k,l;
    
  for (i=0; i<npixels; i=i+dctSize*WIDTH)
    for (j=0; j<blockSize; j=j+dctSize)
      for (k=0; k<dctSize*WIDTH; k=k+blockSize)
	for (l=0; l<dctSize; l++)
	  sum += JPEG_instance.dctData[l+k+j+i]+6MULTIPLIER;
                
  printf (" ---------------------\n");
  if (sum != 23522)
    {
      printf ("|  JPEG: fail\n");
      printf ("SUM = %d\n", sum);
    }
  else
    printf ("|  JPEG: success\n");
  printf (" ---------------------\n");
  printf ("\n");
#endif

#ifndef PIPELINE
  JPEG_destroy(&JPEG_instance);
#else
  JPEG_destroy(&JPEG_instance[0]);
  JPEG_destroy(&JPEG_instance[1]);
#endif

  //free(dct_data);

  // measure time
  clock_gettime(CLOCK_REALTIME,&tp2);
  accumulate_time(&tp1,&tp2,&s_duration,&ns_duration,ACC_CTRL);
  
  // print time measurements
  printf("\n###########################################################################\n");
  printf("Total Offload Time \t : %u.%09lu seconds\n",s_duration1,ns_duration1);
  printf("Total Host Wait Time \t : %u.%09lu seconds\n",s_duration3,ns_duration3);
  printf("Total Host Kernel Time \t : %u.%09lu seconds\n",s_duration2,ns_duration2);
  printf("Total Execution Time \t : %u.%09lu seconds\n",s_duration,ns_duration);
  printf("\n######################################################################\n");
  //double dma_time = (double)acc_read32(acc->mb_mem.v_addr,DMA_TIME_REG_OFFSET_B,'b')/(MB_CLK_FREQ_MHZ * 1000000);
  //printf("Total DMA Time \t\t : %.9f seconds\n", dma_time);
  //double pmca_time = (double)(REPETITIONS*N_STRIPES*KERNEL_CYCLES_PER_STRIPE)/(STHORM_CLK_FREQ_MHZ * 1000000);
  //printf("Total PMCA Kernel Time \t : %.9f seconds\n", pmca_time );
  //printf("\n######################################################################\n");
  ////printf("Avg. Offload Time \t : %u.%09llu seconds\n",s_duration1/REPETITIONS,ns_duration1/REPETITIONS+(((unsigned long long)(s_duration1 % REPETITIONS)*1000000000) / REPETITIONS));
  ////printf("Avg. Host Wait Time \t : %u.%09llu seconds\n",s_duration3/REPETITIONS,ns_duration3/REPETITIONS+(((unsigned long long)(s_duration3 % REPETITIONS)*1000000000) / REPETITIONS));
  ////printf("Avg. Host Kernel Time \t : %u.%09llu seconds\n",s_duration2/REPETITIONS,ns_duration2/REPETITIONS+(((unsigned long long)(s_duration2 % REPETITIONS)*1000000000) / REPETITIONS));
  ////printf("\n######################################################################\n");
  //double dma_bandwidth = (double)(REPETITIONS*N_STRIPES*(DMA_IN_SIZE_B+DMA_OUT_SIZE_B))/(dma_time*1000000);
  //printf("Achieved DMA Bandwidth \t : %.2f MiB/s\n",dma_bandwidth);
  //double dma_bw_util = dma_bandwidth/2400*100;
  //printf("DMA Bandwidth Utilization: %.2f %% \n",dma_bw_util);
  //printf("\n######################################################################\n");
#ifdef ZYNQ_PMM  
  //zynq_pmm_compute_rates(cache_miss_rates, counter_values); // compute cache miss rates
  //double miss_rate_0 = cache_miss_rates[0]*100;
  //double miss_rate_1 = cache_miss_rates[1]*100;
  //printf("Host Kernel L1 D-Cache Miss Rates: %.2f %% (Core 0), %.2f %% (Core 1)\n",miss_rate_0,miss_rate_1);
  //printf("\n###########################################################################\n");
  //printf("MATLAB:\n");
  //printf(" [ %u.%09lu, %u.%09lu, %u.%09lu, %u.%09lu, %.9f, %.9f, %.2f, %.2f, %.2f, %.2f ];\n",s_duration1,ns_duration1,s_duration3,ns_duration3, s_duration2,ns_duration2, s_duration,ns_duration, dma_time, pmca_time, dma_bandwidth, dma_bw_util, miss_rate_0, miss_rate_1 );
#endif
  //// vogelpi
  //printf("Offload status 1 = %u \n",acc_read32(acc->mb_mem.v_addr, OFFLOAD_STATUS_1_REG_OFFSET_B, 'b') );
  //printf("Offload status 2 = %u \n",acc_read32(acc->mb_mem.v_addr, OFFLOAD_STATUS_2_REG_OFFSET_B, 'b') );
  //printf("DMA %i status = %#x\n",0,acc_dma_status(acc,0));
  //printf("DMA %i status = %#x\n",1,acc_dma_status(acc,1));
  ////acc_reset(acc);
    
  sleep(0.5);
  
  /*
   * Cleanup
   */
 cleanup:
  sleep(0.5);
  pulp_stdout_print(pulp,0);
  pulp_stdout_print(pulp,1);
  pulp_stdout_print(pulp,2);
  pulp_stdout_print(pulp,3);
  pulp_stdout_clear(pulp,0);
  pulp_stdout_clear(pulp,1);
  pulp_stdout_clear(pulp,2);
  pulp_stdout_clear(pulp,3);
 
  sleep(1);

#ifdef ZYNQ_PMM  
  zynq_pmm_close(zynq_pmm_fd);
#endif

  pulp_rab_free_striped(pulp);
  pulp_rab_free(pulp,0);
  pulp_free_v_addr(pulp);
  sleep(1);
  pulp_munmap(pulp);
    
  return 0;
}

int accumulate_time(struct timespec *tp1, struct timespec *tp2, unsigned *seconds, unsigned long *nanoseconds, int ctrl) {

  unsigned tmp_s;
  unsigned long tmp_ns; 

  // compute and output the measured time
  tmp_s = (int)(tp2->tv_sec - tp1->tv_sec);
  if (tp2->tv_nsec > tp1->tv_nsec) { // no overflow
    tmp_ns = (tp2->tv_nsec - tp1->tv_nsec);
  }
  else {//(tp2.tv_nsec < tp1.tv_nsec) {// overflow of tv_nsec
    if (ctrl == 1)
      printf("tp2->tv_nsec < tp1->tv_nsec \n");
    tmp_ns = (1000000000 - tp1->tv_nsec + tp2->tv_nsec) % 1000000000;
    tmp_s -= 1;
  }

  if (ctrl == 1) {
    printf("Elapsed time in seconds = %i\n",tmp_s);
    printf("Elapsed time in nanoseconds = %09li\n",tmp_ns);
  }

  *seconds += tmp_s;
  *nanoseconds += tmp_ns;

  *seconds += (*nanoseconds / 1000000000);
  *nanoseconds = (*nanoseconds % 1000000000);
  
  if (ctrl == 1) {
    printf("Total elapsed time in seconds = %i\n",*seconds);
    printf("Total elapsed time in nanoseconds = %09li\n",*nanoseconds);
  }

  return 0;
}

int JPEG_init(JPEG_kernel_t *jpegInstance){
  jpegInstance->nbBlocks  = NBLKS*MULTIPLIER;
  jpegInstance->blockSize = BLKSIZE;
  jpegInstance->dctSize   = DCTSIZE;
  jpegInstance->BlockMul  = LOCALBLOCKS;
  memcpy(jpegInstance->qtblLum,qtbl_lum,sizeof(unsigned char)*64);
    
#ifdef PROFILE
  jpegInstance->data_desc  = (DataDesc *)malloc(10*sizeof(DataDesc));
#else
  jpegInstance->data_desc  = (DataDesc *)malloc(7*sizeof(DataDesc));
#endif  // PROFILE

  jpegInstance->n_clusters = 1;
  return 0;
}

void JPEG_offload_out(JPEG_kernel_t *jpegInstance){
  jpegInstance->data_desc[0].ptr  = &jpegInstance->nbBlocks;
  jpegInstance->data_desc[0].size = sizeof(int);
  jpegInstance->data_desc[0].type = 1;
  jpegInstance->data_desc[1].ptr  = &jpegInstance->blockSize;
  jpegInstance->data_desc[1].size = sizeof(int);
  jpegInstance->data_desc[1].type = 1;
  jpegInstance->data_desc[2].ptr  = &jpegInstance->dctSize;
  jpegInstance->data_desc[2].size = sizeof(int);
  jpegInstance->data_desc[2].type = 1;
  jpegInstance->data_desc[3].ptr  = jpegInstance->dctData;
  jpegInstance->data_desc[3].size = sizeof(short)*jpegInstance->blockSize*(jpegInstance->nbBlocks/4);
  jpegInstance->data_desc[3].type = 0;
  jpegInstance->data_desc[4].ptr  = jpegInstance->qtblLum;
  jpegInstance->data_desc[4].size = 64*sizeof(unsigned char);
  jpegInstance->data_desc[4].type = 1;
  jpegInstance->data_desc[5].ptr  = &jpegInstance->cycles;
  jpegInstance->data_desc[5].size = sizeof(unsigned int);
  jpegInstance->data_desc[5].type = 2;
  jpegInstance->data_desc[6].ptr  = &jpegInstance->BlockMul;
  jpegInstance->data_desc[6].size = sizeof(unsigned int);
  jpegInstance->data_desc[6].type = 1;
#ifdef PROFILE    
  jpegInstance->data_desc[7].ptr  = &jpegInstance->t_comp;
  jpegInstance->data_desc[7].size = sizeof(unsigned int);
  jpegInstance->data_desc[7].type = 2;
  jpegInstance->data_desc[8].ptr  = &jpegInstance->t_dma_in;
  jpegInstance->data_desc[8].size = sizeof(unsigned int);
  jpegInstance->data_desc[8].type = 2;
  jpegInstance->data_desc[9].ptr  = &jpegInstance->t_dma_out;
  jpegInstance->data_desc[9].size = sizeof(unsigned int);
  jpegInstance->data_desc[9].type = 2;
#endif // PROFILE
    
  jpegInstance->desc              = (TaskDesc *)malloc(sizeof(TaskDesc));

#ifdef PROFILE
  jpegInstance->desc->n_data      = 10;
#else
  jpegInstance->desc->n_data      = 7;
#endif // PROFILE     
  jpegInstance->desc->data_desc   = jpegInstance->data_desc;
  jpegInstance->desc->n_clusters  = jpegInstance->n_clusters;
  jpegInstance->desc->name        = (void *) 6;
    
#if (MEM_SHARING == 1)
  pulp_offload_out_contiguous(pulp, jpegInstance->desc, &jpegInstance->fdesc);
#else // 2
  pulp_offload_out(pulp, jpegInstance->desc);
#endif // MEM_SHARING
}

inline int JPEG_exe_start(JPEG_kernel_t *jpegInstance) {
//inline void JPEG_exe_start(JPEG_kernel_t *jpegInstance) {
  //pulp_offload_start(pulp, jpegInstance->desc);
  //return;
  return pulp_offload_start(pulp, jpegInstance->desc);
}

inline int JPEG_exe_wait(JPEG_kernel_t *jpegInstance) {
  //inline void JPEG_exe_wait(JPEG_kernel_t *jpegInstance) {
  //pulp_offload_wait(pulp, jpegInstance->desc);
  //return;
  return pulp_offload_wait(pulp, jpegInstance->desc);
}

inline int JPEG_offload_in(JPEG_kernel_t *jpegInstance) {
  //inline void JPEG_offload_in(JPEG_kernel_t *jpegInstance) {
  int ret;
#if (MEM_SHARING == 1)
  ret = pulp_offload_in_contiguous(pulp, jpegInstance->desc, &jpegInstance->fdesc);
#else // 2
  ret = pulp_offload_in(pulp, jpegInstance->desc); 
#endif // MEM_SHARING
  free(jpegInstance->desc);
  return ret;
}

void JPEG_destroy(JPEG_kernel_t *jpegInstance){
  free(jpegInstance->data_desc);
}

void
huff_dc_dec (int *retval, unsigned int *lastlong, unsigned long **nextlong, int *bitsleft)
{
  int i, s, l, p, code;
    
  l = 1;
    
  code = getbit(lastlong, nextlong, bitsleft);
    
  while (code > maxcode_dc[l]) {
    l++;
    code = (code << 1) + getbit(lastlong, nextlong, bitsleft);
  }
    
  p = valptr_dc[l];
  p = p + code - mincode_dc[l];
  s = val_dc_lum[p];
    
  *retval = 0;
  for (i=0; i<s; i++) 
    *retval = (*retval << 1) + getbit(lastlong, nextlong, bitsleft);
    
  i = 1 << (s-1);
    
  while (*retval < i) {
    i = (-1 << s) + 1;
    *retval = *retval + i;
  }
}

void
huff_ac_dec (short *data, unsigned int *lastlong, unsigned long **nextlong, int *bitsleft)
{
  int i, j, icnt, ns, n, s, l, p, code, dindex;
  int temp, data_zz[64];
    
  dindex = 0;
    
  while (dindex < 63) {
    l = 1;
    code = getbit(lastlong, nextlong, bitsleft);
        
    while (code > maxcode_ac[l]) {
      l = l+1;
      code = (code << 1) + getbit(lastlong, nextlong, bitsleft);
    }
        
    p = valptr_ac[l];
    p = p + code - mincode_ac[l];
    ns = val_ac_lum[p];
    s = ns & 0x0f;
    n = ns >> 4;
        
    if (ns != EOB) {
      if (ns !=  ZRL ) {
	for (i=0; i<n; i++) {
	  data_zz[dindex] = 0;
	  dindex = dindex + 1;
	}
                
	temp = 0;
                
	for (i=0; i<s; i++)
	  temp = (temp << 1) + getbit(lastlong, nextlong, bitsleft);
                
	i = 1 << (s-1);
                
	while (temp < i) {
	  i = (-1 << s) + 1;
	  temp = temp + i;
	}
                
	data_zz[dindex] = temp;
	dindex = dindex + 1;
      }
      else {
	for (i=0; i<16; i++) {
	  data_zz[dindex] = 0;
	  dindex = dindex + 1;
	}
      }
    }
    else {
      icnt = 63-dindex;
      for (i=0; i<icnt; i++) {
	data_zz[dindex] = 0;      
	dindex = dindex + 1;
      }
    }
  }
  for (i=0; i<31; i++) {
    *(data+zz_tbl[i+1]) = data_zz[i];
  }
  for (j=31,i=31; i>0; i--,j++) {
    *(data+63-zz_tbl[i]) = data_zz[j];
  }
    
  *(data+63) = data_zz[62];
}

int getbit(unsigned int *lastlong, unsigned long **nextlong, int *bitsleft)
{
  int bit;
    
  *bitsleft = *bitsleft - 1;
  if (*bitsleft < 0)
    {

      if (*nextlong >= &huffbits[sizeof(huffbits)]){
	*nextlong = huffbits;
	*lastlong = **nextlong;
	*nextlong = *nextlong + 1;
	*bitsleft = 31;
      }
      else
        {
	  *lastlong = **nextlong;
	  *nextlong = *nextlong + 1;
	  *bitsleft = 31;
        }        
    }
  bit = (*lastlong & 0x80000000) != 0;
  *lastlong = *lastlong << 1;    
  return (bit);
}
