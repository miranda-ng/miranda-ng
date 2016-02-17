-- vim: ts=4 sw=4 sts=4 et tw=78
-- Portions copyright (c) 2015-present, Facebook, Inc. All rights reserved.
-- Portions copyright (c) 2011 James R. McKaskill.
--
-- This source code is licensed under the BSD-style license found in the
-- LICENSE file in the root directory of this source tree. An additional grant
-- of patent rights can be found in the PATENTS file in the same directory.
--
io.stdout:setvbuf('no')
local ffi = require 'ffi'
local dlls = {}

local function loadlib(lib)
    for pattern in package.cpath:gmatch('[^;]+') do
        local path = pattern:gsub('?', lib)
        local ok, lib = pcall(ffi.load, path)
        if ok then
            return lib
        end
    end
    error("Unable to load", lib)
end

if _VERSION == 'Lua 5.1' then
    dlls.__cdecl = loadlib('ffi/libtest')
else
    dlls.__cdecl = ffi.load(package.searchpath('ffi.libtest', package.cpath))
end

if ffi.arch == 'x86' and ffi.os == 'Windows' then
    dlls.__stdcall = ffi.load('test_stdcall')
    dlls.__fastcall = ffi.load('test_fastcall')
end

local function check(a, b, msg)
    if a ~= b then
        print('check', a, b)
    end
    return _G.assert(a == b, msg)
end

print('Running test')

ffi.cdef [[
enum e8 {
    FOO8,
    BAR8,
};
enum e16 {
    FOO16 = 1 << 8,
    BAR16,
    BIG16 = 1 << 14,
};
enum e32 {
    FOO32 = 1 << 16,
    BAR32,
    BIG32 = 1 << 30,
};
int max_alignment();
bool is_msvc, is_msvc2 __asm__("is_msvc");
bool have_complex(void);
bool have_complex2() __asm__("have" /*foo*/ "\x5F" "complex"); // 5F is _

int8_t add_i8(int8_t a, int8_t b);
uint8_t add_u8(uint8_t a, uint8_t b);
int16_t add_i16(int16_t a, int16_t b);
uint16_t add_i16(uint16_t a, uint16_t b);
int32_t add_i32(int32_t a, int32_t b);
uint32_t add_u32(uint32_t a, uint32_t b);
int64_t add_i64(int64_t a, int64_t b);
uint64_t add_u64(uint64_t a, uint64_t b);
double add_d(double a, double b);
float add_f(float a, float b);
double complex add_dc(double complex a, double complex b);
float complex add_fc(float complex a, float complex b);
enum e8 inc_e8(enum e8);
enum e16 inc_e16(enum e16);
enum e32 inc_e32(enum e32);
bool not_b(bool v);
_Bool not_b2(_Bool v);
typedef bool (*fp)(bool);
fp ret_fp(fp v);
bool (*ret_fp2(bool (*)(bool)))(bool) __asm("ret_fp");

int print_i8(char* buf, int8_t val);
int print_u8(char* buf, uint8_t val);
int print_i16(char* buf, int16_t val);
int print_u16(char* buf, uint16_t val);
int print_i32(char* buf, int32_t val);
int print_u32(char* buf, uint32_t val);
int print_i64(char* buf, int64_t val);
int print_u64(char* buf, uint64_t val);
int print_s(char* buf, const char* val);
int print_b(char* buf, bool val);
int print_b2(char* buf, _Bool val);
int print_d(char* buf, double val);
int print_f(char* buf, float val);
int print_p(char* buf, void* val);
int print_dc(char* buf, double complex val);
int print_fc(char* buf, float complex val);
int print_e8(char* buf, enum e8 val);
int print_e16(char* buf, enum e16 val);
int print_e32(char* buf, enum e32 val);
int sprintf(char* buf, const char* format, ...);

// Examples from MSDN

// bit_fields1.cpp
// compile with: /LD
struct Date {
   unsigned short nWeekDay  : 3;    // 0..7   (3 bits)
   unsigned short nMonthDay : 6;    // 0..31  (6 bits)
   unsigned short nMonth    : 5;    // 0..12  (5 bits)
   unsigned short nYear     : 8;    // 0..100 (8 bits)
};

// bit_fields2.cpp
// compile with: /LD
struct Date2 {
   unsigned nWeekDay  : 3;    // 0..7   (3 bits)
   unsigned nMonthDay : 6;    // 0..31  (6 bits)
   unsigned           : 0;    // Force alignment to next boundary.
   unsigned nMonth    : 5;    // 0..12  (5 bits)
   unsigned nYear     : 8;    // 0..100 (8 bits)
};

// For checking the alignment of short bitfields
struct Date3 {
   char pad;
   unsigned short nWeekDay  : 3;    // 0..7   (3 bits)
   unsigned short nMonthDay : 6;    // 0..31  (6 bits)
   unsigned short nMonth    : 5;    // 0..12  (5 bits)
   unsigned short nYear     : 8;    // 0..100 (8 bits)
};

// For checking the alignment and container of int64 bitfields
struct bit64 {
    char pad;
    uint64_t a : 15;
    uint64_t b : 14;
    uint64_t c : 13;
    uint64_t d : 12;
};

// Examples from SysV X86 ABI
struct sysv1 {
    int     j:5;
    int     k:6;
    int     m:7;
};

struct sysv2 {
    short   s:9;
    int     j:9;
    char    c;
    short   t:9;
    short   u:9;
    char    d;
};

struct sysv3 {
    char    c;
    short   s:8;
};

union sysv4 {
    char    c;
    short   s:8;
};

struct sysv5 {
    char    c;
    int     :0;
    char    d;
    short   :9;
    char    e;
    char    :0;
};

struct sysv6 {
    char    c;
    int     :0;
    char    d;
    int     :9;
    char    e;
};

struct sysv7 {
    int     j:9;
    short   s:9;
    char    c;
    short   t:9;
    short   u:9;
};

int print_date(size_t* sz, size_t* align, char* buf, struct Date* s);
int print_date2(size_t* sz, size_t* align, char* buf, struct Date2* s);
int print_date3(size_t* sz, size_t* align, char* buf, struct Date3* d);
int print_bit64(size_t* sz, size_t* align, char* buf, struct bit64* d);
int print_sysv1(size_t* sz, size_t* align, char* buf, struct sysv1* s);
int print_sysv2(size_t* sz, size_t* align, char* buf, struct sysv2* s);
int print_sysv3(size_t* sz, size_t* align, char* buf, struct sysv3* s);
int print_sysv4(size_t* sz, size_t* align, char* buf, union sysv4* s);
int print_sysv5(size_t* sz, size_t* align, char* buf, struct sysv5* s);
int print_sysv6(size_t* sz, size_t* align, char* buf, struct sysv6* s);
int print_sysv7(size_t* sz, size_t* align, char* buf, struct sysv7* s);

struct fptr {
    int (__cdecl *p)(int);
};
int call_fptr(struct fptr* s, int val);

bool g_b;
int8_t g_i8;
int16_t g_i16;
int32_t g_i32;
int64_t g_i64;
uint8_t g_u8;
uint16_t g_u16;
uint32_t g_u32;
uint64_t g_u64;
float g_f;
double g_d;
double complex g_dc;
float complex g_fc;
bool (*g_fp)(bool);
const char g_s[];
const char* g_sp;
void* g_p;
enum e8 g_e8;
enum e16 g_e16;
enum e32 g_e32;
struct Date g_date;

void set_errno(int val);
int get_errno(void);
]]

local align = [[
struct align_ALIGN_SUFFIX {
    char pad;
    TYPE v;
};

int print_align_ALIGN_SUFFIX(char* buf, struct align_ALIGN_SUFFIX* p);
]]

local palign = [[
#pragma pack(push)
#pragma pack(ALIGN)
]] .. align .. [[
#pragma pack(pop)
]]

local bitfields = [[
struct bcTNUM {
    uintTNUM_t a : 3;
    intTNUM_t b : 3;
};
struct blzTNUM {
    uintTNUM_t a;
    uintTNUM_t :0;
    uintTNUM_t b;
};
int print_bcTNUM(size_t* sz, size_t* align, char* buf, struct bcTNUM* s);
int print_blzTNUM(size_t* sz, size_t* align, char* buf, struct blzTNUM* s);
]]

local bitalign = [[
struct ba_TNUM_BNUM {
    char a;
    uintTNUM_t b : BNUM;
};
struct bu_TNUM_BNUM {
    char a;
    uintTNUM_t :BNUM;
    char b;
};
int print_ba_TNUM_BNUM(size_t* sz, size_t* align, char* buf, struct ba_TNUM_BNUM* s);
]]

local bitzero = [[
struct bz_TNUM_ZNUM_BNUM {
    uint8_t a;
    uintTNUM_t b : 3;
    uintZNUM_t :BNUM;
    uintTNUM_t c : 3;
};
int print_bz_TNUM_ZNUM_BNUM(size_t* sz, size_t* align, char* buf, struct bz_TNUM_ZNUM_BNUM* s);
]]

local i = ffi.C.i
local test_values = {
    ['void*'] = ffi.new('char[3]'),
    ['const char*'] = 'foo',
    float = 3.4,
    double = 5.6,
    uint16_t = 65000,
    uint32_t = ffi.new('uint32_t', 700000056),
    uint64_t = 12345678901234,
    bool = true,
    _Bool = false,
    ['float complex'] = 3.1+4.2*i,
    ['double complex'] = 5.1+6.2*i,
    ['enum e8'] = ffi.C.FOO8,
    ['enum e16'] = ffi.C.FOO16,
    ['enum e32'] = ffi.C.FOO32,
}

local types = {
    b = 'bool',
    b2 = '_Bool',
    d = 'double',
    f = 'float',
    u64 = 'uint64_t',
    u32 = 'uint32_t',
    u16 = 'uint16_t',
    s = 'const char*',
    p = 'void*',
    e8 = 'enum e8',
    e16 = 'enum e16',
    e32 = 'enum e32',
}

local buf = ffi.new('char[256]')

local function checkbuf(type, ret, msg)
    local str = tostring(test_values[type]):gsub('^cdata%b<>: ', '')
    check(ffi.string(buf), str, msg)
    check(ret, #str, msg)
end

local function checkalign(type, v, ret)
    --print(v)
    local str = tostring(test_values[type]):gsub('^cdata%b<>: ', '')
    check(ffi.string(buf), ('size %d offset %d align %d value %s'):format(ffi.sizeof(v), ffi.offsetof(v, 'v'), ffi.alignof(v, 'v'), str))
    check(ret, #str)
end

local u64 = ffi.typeof('uint64_t')
local i64 = ffi.typeof('int64_t')

local first = true

for convention,c in pairs(dlls) do
    check(c.add_i8(1,1), 2)
    check(c.add_i8(256,1), 1)
    check(c.add_i8(127,1), -128)
    check(c.add_i8(-120,120), 0)
    check(c.add_u8(255,1), 0)
    check(c.add_u8(120,120), 240)
    check(c.add_i16(2000,4000), 6000)
    check(c.add_d(20, 12), 32)
    check(c.add_f(40, 32), 72)
    check(c.not_b(true), false)
    check(c.not_b2(false), true)
    check(c.inc_e8(c.FOO8), c.BAR8)
    check(c.inc_e8('FOO8'), c.BAR8)
    check(c.inc_e16(c.FOO16), c.BAR16)
    check(c.inc_e32(c.FOO32), c.BAR32)
    check(c.ret_fp(c.g_fp), c.g_fp)
    check(c.ret_fp2(c.g_fp), c.g_fp)

    if c.have_complex() then
        check(c.add_dc(3+4*i, 4+5*i), 7+9*i)
        check(c.add_fc(2+4*i, 6+8*i), 8+12*i)
        types.dc = 'double complex'
        types.fc = 'float complex'
    else
        types.dc = nil
        types.fc = nil
    end
    check((3+4*i).re, 3)
    check((3+4*i).im, 4)
    check(ffi.new('complex float', 2+8*i).re, 2)
    check(ffi.new('complex float', 5+6*i).im, 6)

    check(c.have_complex(), c.have_complex2())
    check(c.is_msvc, c.is_msvc2)

    check(c.g_b, true)
    check(c.g_i8, -8)
    check(c.g_i16, -16)
    check(c.g_i32, -32)
    check(c.g_i64, i64(-64))
    check(c.g_u8, 8)
    check(c.g_u16, 16)
    check(c.g_u32, 32)
    check(c.g_u64, u64(64))
    check(c.g_f, 3)
    check(c.g_d, 5)
    if c.have_complex() then
        check(c.g_dc, 7 + 8*i)
        check(c.g_fc, 6 + 9*i)
    end
    check(ffi.cast('void*', c.g_fp), c.g_p)
    check(c.g_s, 'g_s')
    check(c.g_sp, 'g_sp')
    check(c.g_e8, c.FOO8)
    check(c.g_e16, c.FOO16)
    check(c.g_e32, c.FOO32)
    check(c.g_date.nWeekDay, 1)
    check(c.g_date.nMonthDay, 2)
    check(c.g_date.nMonth, 3)
    check(c.g_date.nYear, 4)

    c.g_b = false; check(c.g_b, false)
    c.g_i8 = -108; check(c.g_i8, -108)
    c.g_i16 = -1016; check(c.g_i16, -1016)
    c.g_i32 = -1032; check(c.g_i32, -1032)
    c.g_i64 = -1064; check(c.g_i64, i64(-1064))
    c.g_u8 = 208; check(c.g_u8, 208)
    c.g_u16 = 2016; check(c.g_u16, 2016)
    c.g_u32 = 2032; check(c.g_u32, 2032)
    c.g_u64 = 2064; check(c.g_u64, u64(2064))
    c.g_f = 13; check(c.g_f, 13)
    c.g_d = 15; check(c.g_d, 15)
    if c.have_complex() then
        c.g_dc = 17+18*i; check(c.g_dc, 17+18*i)
        c.g_fc = 16+19*i; check(c.g_fc, 16+19*i)
    end
    c.g_sp = 'foo'; check(c.g_sp, 'foo')
    c.g_e8 = c.BAR8; check(c.g_e8, c.BAR8)
    c.g_e16 = c.BAR16; check(c.g_e16, c.BAR16)
    c.g_e32 = c.BAR32; check(c.g_e32, c.BAR32)
    c.g_date.nWeekDay = 3; check(c.g_date.nWeekDay, 3)

    local align_attr = c.is_msvc and [[
        struct align_attr_ALIGN_SUFFIX {
            char pad;
            __declspec(align(ALIGN)) TYPE v;
        };

        int print_align_attr_ALIGN_SUFFIX(char* buf, struct align_attr_ALIGN_SUFFIX* p);
        ]] or [[
        struct align_attr_ALIGN_SUFFIX {
            char pad;
            TYPE v __attribute__(aligned(ALIGN));
        };

        int print_align_attr_ALIGN_SUFFIX(char* buf, struct align_attr_ALIGN_SUFFIX* p);
        ]]

    for suffix, type in pairs(types) do
        local test = test_values[type]
        --print('checkbuf', suffix, type, buf, test)
        checkbuf(type, c['print_' .. suffix](buf, test), suffix)

        if first then
            ffi.cdef(align:gsub('SUFFIX', suffix):gsub('TYPE', type):gsub('ALIGN', 0))
        end

        local v = ffi.new('struct align_0_' .. suffix, {0, test})
        checkalign(type, v, c['print_align_0_' .. suffix](buf, v))

        for _,align in ipairs{1,2,4,8,16} do
            if align > c.max_alignment() then
                break
            end

            if first then
                ffi.cdef(palign:gsub('SUFFIX', suffix):gsub('TYPE', type):gsub('ALIGN', align))
                ffi.cdef(align_attr:gsub('SUFFIX', suffix):gsub('TYPE', type):gsub('ALIGN', align))
            end

            local v = ffi.new('struct align_' .. align .. '_' .. suffix, {0, test})
            checkalign(type, v, c['print_align_' .. align .. '_' .. suffix](buf, v))

            -- MSVC doesn't support aligned attributes on enums
            if not type:match('^enum e[0-9]*$') or not c.is_msvc then
                local v2 = ffi.new('struct align_attr_' .. align .. '_' .. suffix, {0, test})
                checkalign(type, v2, c['print_align_attr_' .. align .. '_' .. suffix](buf, v2))
            end
        end

        if not c.is_msvc then
            if first then
                local h = [[
                struct align_attr_def_SUFFIX {
                    char pad;
                    TYPE v __attribute__(aligned);
                };
                int print_align_attr_def_SUFFIX(char* buf, struct align_attr_def_SUFFIX* p);
                ]]
                ffi.cdef(h:gsub('SUFFIX', suffix):gsub('TYPE', type))
            end

            local v = ffi.new('struct align_attr_def_' .. suffix, {0, test})
            -- print(type)
            -- print("Align " .. c['print_align_attr_def_' .. suffix](buf, v))
            -- print(ffi.string(buf))
            -- checkalign(type, v, c['print_align_attr_def_' .. suffix](buf, v))
        end
    end

    local psz = ffi.new('size_t[1]')
    local palign = ffi.new('size_t[1]')
    local function check_align(type, test, ret)
        --print('check_align', type, test, ret, ffi.string(buf), psz[0], palign[0])
        check(tonumber(palign[0]), ffi.alignof(type))
        check(tonumber(psz[0]), ffi.sizeof(type))
        check(ret, #test)
        check(test, ffi.string(buf))
    end

    for _, tnum in ipairs{8, 16, 32, 64} do
        if first then
            ffi.cdef(bitfields:gsub('TNUM',tnum))
        end

        check_align('struct bc'..tnum, '1 2', c['print_bc'..tnum](psz, palign, buf, {1,2}))
        check_align('struct blz'..tnum, '1 2', c['print_blz'..tnum](psz, palign, buf, {1,2}))

        for _, znum in ipairs{8, 16, 32, 64} do
            for _, bnum in ipairs{7, 15, 31, 63} do
                if bnum > znum then
                    break
                end
                if first then
                    ffi.cdef(bitzero:gsub('TNUM',tnum):gsub('ZNUM',znum):gsub('BNUM', bnum))
                end
                check_align('struct bz_'..tnum..'_'..znum..'_'..bnum, '1 2 3', c['print_bz_'..tnum..'_'..znum..'_'..bnum](psz, palign, buf, {1,2,3}))
            end
        end

        for _, bnum in ipairs{7, 15, 31, 63} do
            if bnum > tnum then
                break
            end
            if first then
                ffi.cdef(bitalign:gsub('TNUM',tnum):gsub('BNUM',bnum))
            end
            check_align('struct ba_'..tnum..'_'..bnum, '1 2', c['print_ba_'..tnum..'_'..bnum](psz, palign, buf, {1,2}))
        end
    end

    check_align('struct Date', '1 2 3 4', c.print_date(psz, palign, buf, {1,2,3,4}))
    check_align('struct Date2', '1 2 3 4', c.print_date2(psz, palign, buf, {1,2,3,4}))
    check_align('struct sysv1', '1 2 3', c.print_sysv1(psz, palign, buf, {1,2,3}))
    check_align('struct sysv2', '1 2 3 4 5 6', c.print_sysv2(psz, palign, buf, {1,2,3,4,5,6}))
    check_align('struct sysv3', '1 2', c.print_sysv3(psz, palign, buf, {1,2}))
    check_align('union sysv4', '1', c.print_sysv4(psz, palign, buf, {1}))
    check_align('struct sysv5', '1 2 3', c.print_sysv5(psz, palign, buf, {1,2,3}))
    check_align('struct sysv6', '1 2 3', c.print_sysv6(psz, palign, buf, {1,2,3}))
    check_align('struct sysv7', '1 2 3 4 5', c.print_sysv7(psz, palign, buf, {1,2,3,4,5}))

    local cbs = [[
    typedef const char* (*__cdecl sfunc)(const char*);
    int call_i(int (*__cdecl func)(int), int arg);
    float call_f(float (*__cdecl func)(float), float arg);
    double call_d(double (*__cdecl func)(double), double arg);
    const char* call_s(sfunc func, const char* arg);
    _Bool call_b(_Bool (*__cdecl func)(_Bool), _Bool arg);
    double complex call_dc(double complex (*__cdecl func)(double complex), double complex arg);
    float complex call_fc(float complex (*__cdecl func)(float complex), float complex arg);
    enum e8 call_e8(enum e8 (*__cdecl func)(enum e8), enum e8 arg);
    enum e16 call_e16(enum e16 (*__cdecl func)(enum e16), enum e16 arg);
    enum e32 call_e32(enum e32 (*__cdecl func)(enum e32), enum e32 arg);
    ]]

    ffi.cdef(cbs:gsub('__cdecl', convention))

    local u3 = ffi.new('uint64_t', 3)
    check(c.call_i(function(a) return 2*a end, 3), 6)
    assert(math.abs(c.call_d(function(a) return 2*a end, 3.2) - 6.4) < 0.0000000001)
    assert(math.abs(c.call_f(function(a) return 2*a end, 3.2) - 6.4) < 0.000001)
    check(ffi.string(c.call_s(function(s) return s + u3 end, 'foobar')), 'bar')
    check(c.call_b(function(v) return not v end, true), false)
    check(c.call_e8(function(v) return v + 1 end, c.FOO8), c.BAR8)
    check(c.call_e16(function(v) return v + 1 end, c.FOO16), c.BAR16)
    check(c.call_e32(function(v) return v + 1 end, c.FOO32), c.BAR32)

    if c.have_complex() then
        check(c.call_dc(function(v) return v + 2+3*i end, 4+6*i), 6+9*i)
        check(c.call_fc(function(v) return v + 1+2*i end, 7+4*i), 8+6*i)
    end

    local u2 = ffi.new('uint64_t', 2)
    local cb = ffi.new('sfunc', function(s) return s + u3 end)
    check(ffi.string(cb('foobar')), 'bar')
    check(ffi.string(c.call_s(cb, 'foobar')), 'bar')
    cb:set(function(s) return s + u2 end)
    check(ffi.string(c.call_s(cb, 'foobar')), 'obar')

    local fp = ffi.new('struct fptr')
    assert(fp.p == ffi.C.NULL)
    fp.p = function(a) return 2*a end
    assert(fp.p ~= ffi.C.NULL)
    check(c.call_fptr(fp, 4), 8)
    local suc, err = pcall(function() fp.p:set(function() end) end)
    assert(not suc)
    check(err:gsub('^.*: ',''), "can't set the function for a non-lua callback")

    check(c.call_fptr({function(a) return 3*a end}, 5), 15)

    local suc, err = pcall(c.call_s, function(s) error(ffi.string(s), 0) end, 'my error')
    check(suc, false)
    check(err, 'my error')

    check(ffi.errno(), c.get_errno())
    c.set_errno(3)
    check(ffi.errno(), 3)
    check(c.get_errno(), 3)
    check(ffi.errno(4), 3)
    check(ffi.errno(), 4)
    check(c.get_errno(), 4)

    local gccattr = {
        __cdecl = 'int test_pow(int v) __attribute__((cdecl));',
        __stdcall = 'int test_pow(int v) __attribute__(stdcall);',
        __fastcall = '__attribute__(fastcall) int test_pow(int v);',
    }

    ffi.cdef(gccattr[convention])
    check(c.test_pow(5), 25)

    ffi.cdef [[
        int va_list_size, va_list_align;
        int vsnprintf(char* buf, size_t sz, const char* fmt, va_list ap);
    ]]
    ffi.new('va_list')
    assert(ffi.debug().functions.vsnprintf ~= nil)
    assert(ffi.istype('va_list', ffi.new('__builtin_va_list')))
    assert(ffi.istype('va_list', ffi.new('__gnuc_va_list')))
    check(ffi.sizeof('va_list'), c.va_list_size)
    check(ffi.alignof('va_list'), c.va_list_align)

    first = false
end

local c = ffi.C

assert(c.sprintf(buf, "%g", 5.3) == 3 and ffi.string(buf) == '5.3')
assert(c.sprintf(buf, "%d", false) == 1 and ffi.string(buf) == '0')
assert(c.sprintf(buf, "%d%g", false, 6.7) == 4 and ffi.string(buf) == '06.7')

assert(ffi.sizeof('uint32_t[?]', 32) == 32 * 4)
assert(ffi.sizeof(ffi.new('uint32_t[?]', 32)) == 32 * 4)

ffi.cdef [[
struct vls {
    struct {
        char a;
        struct {
            char b;
            char v[?];
        } c;
    } d;
};
struct vls2 {
    char pad;
    union {
        uint8_t a;
        uint16_t b;
    };
};
]]

assert(ffi.sizeof('struct vls', 3) == 5)
assert(ffi.sizeof(ffi.new('struct vls', 4).d.c) == 5)
assert(ffi.offsetof('struct vls2', 'a') == 2)
assert(ffi.sizeof('struct vls2') == 4)

ffi.cdef [[ static const int DUMMY = 8 << 2; ]]
assert(ffi.C.DUMMY == 32)

ffi.new('struct {const char* foo;}', {'foo'})

assert(not pcall(function()
    ffi.new('struct {char* foo;}', {'ff'})
end))

local mt = {}
local vls = ffi.new(ffi.metatype('struct vls', mt), 1)

assert(not pcall(function() return vls.key end))

mt.__index = function(vls, key)
    return function(vls, a, b)
        return 'in index ' .. key .. ' ' .. vls.d.a .. ' ' .. a .. ' ' .. b
    end
end

vls.d.a = 3
check(vls:key('a', 'b'), 'in index key 3 a b')

assert(not pcall(function() vls.k = 3 end))

mt.__newindex = function(vls, key, val)
    error('in newindex ' .. key .. ' ' .. vls.d.a .. ' ' .. val, 0)
end

vls.d.a = 4
local suc, err = pcall(function() vls.key = 'val' end)
assert(not suc)
check(err, 'in newindex key 4 val')

mt.__add = function(vls, a) return vls.d.a + a end
mt.__sub = function(vls, a) return vls.d.a - a end
mt.__mul = function(vls, a) return vls.d.a * a end
mt.__div = function(vls, a) return vls.d.a / a end
mt.__mod = function(vls, a) return vls.d.a % a end
mt.__pow = function(vls, a) return vls.d.a ^ a end
mt.__eq = function(vls, a) return u64(vls.d.a) == a end
mt.__lt = function(vls, a) return u64(vls.d.a) < a end
mt.__le = function(vls, a) return u64(vls.d.a) <= a end
mt.__call = function(vls, a, b) return '__call', vls.d.a .. a .. (b or 'nil')  end
mt.__unm = function(vls) return -vls.d.a end
mt.__concat = function(vls, a) return vls.d.a .. a end
mt.__len = function(vls) return vls.d.a end
mt.__tostring = function(vls) return 'string ' .. vls.d.a end

vls.d.a = 5
check(vls + 5, 10)
check(vls - 5, 0)
check(vls * 5, 25)
check(vls / 5, 1)
check(vls % 3, 2)
check(vls ^ 3, 125)
check(vls == u64(4), false)
check(vls == u64(5), true)
check(vls == u64(6), false)
check(vls < u64(4), false)
check(vls < u64(5), false)
check(vls < u64(6), true)
check(vls <= u64(4), false)
check(vls <= u64(5), true)
check(vls <= u64(6), true)
check(-vls, -5)
local a,b = vls('6')
check(a, '__call')
check(b, '56nil')
check(tostring(vls), 'string 5')

if _VERSION ~= 'Lua 5.1' then
    check(vls .. 'str', '5str')
    check(#vls, 5)
end

check(tostring(1.1+3.2*i), '1.1+3.2i')
check((1+3*i)*(2+4*i), -10+10*i)
check((3+2*i)*(3-2*i), 13+0*i)

-- Should ignore unknown attributes
ffi.cdef [[
typedef int ALenum;
__attribute__((dllimport)) void __attribute__((__cdecl__)) alEnable( ALenum capability );
]]

check(ffi.sizeof('struct {char foo[alignof(uint64_t)];}'), ffi.alignof('uint64_t'))

-- Long double is not supported yet but it should be parsed
ffi.cdef('long double foo(long double val);')
check(tostring(ffi.debug().functions.foo):match('ctype(%b<>)'), '<long double (*)(long double)>')

ffi.cdef [[
typedef int byte1 __attribute__(mode(QI));
typedef int byte2 __attribute__(mode(HI));
typedef int byte4 __attribute__(mode(SI));
typedef int byte8 __attribute__(mode(DI));
typedef unsigned ubyte8 __attribute__(mode(DI));
typedef int word __attribute__(mode(word));
typedef int pointer __attribute__(mode(pointer));
typedef int byte __attribute__(mode(byte));
typedef float float4 __attribute__(mode(SF));
typedef float float8 __attribute__(mode(DF));
]]
assert(ffi.istype('int8_t', ffi.new('byte1')))
assert(ffi.istype('int16_t', ffi.new('byte2')))
assert(ffi.istype('int32_t', ffi.new('byte4')))
assert(ffi.istype('int64_t', ffi.new('byte8')))
assert(ffi.istype('uint64_t', ffi.new('ubyte8')))
check(ffi.sizeof('void*'), ffi.sizeof('pointer'))
check(ffi.alignof('void*'), ffi.alignof('pointer'))
check(ffi.sizeof('void*'), ffi.sizeof('word'))
check(ffi.alignof('void*'), ffi.alignof('word'))
assert(ffi.istype('int8_t', ffi.new('byte')))
assert(ffi.istype('float', ffi.new('float4')))
assert(ffi.istype('double', ffi.new('float8')))

ffi.cdef('void register_foo(register int val);')
check(tostring(ffi.debug().functions.register_foo):match('%b<>'), '<void (*)(int)>')

ffi.cdef [[
    typedef struct __sFILE FILE;
]]

assert(not ffi.istype('int', ffi.new('int*')))
assert(not ffi.istype('int[]', ffi.new('int*')))
assert(not ffi.istype('int[3]', ffi.new('int*')))
assert(not ffi.istype('int[3]', ffi.new('int[2]')))
assert(ffi.istype('const int[3]', ffi.new('const int[3]')))
assert(ffi.istype('int[3]', ffi.new('const int[3]')))

-- Crazy function pointer that takes an int and a function pointer and returns
-- a function pointer. Type of &signal.
check(tostring(ffi.typeof('void (*foo(int, void(*)(int)))(int)')):match('%b<>'), '<void (*(*)(int, void (*)(int)))(int)>')

-- Make sure we pass all arguments to tonumber
check(tonumber('FE', 16), 0xFE)

-- Allow casts from pointer to numeric types
ffi.cast('long', ffi.C.NULL)
ffi.cast('int8_t', ffi.C.NULL)
assert(not pcall(function() ffi.new('long', ffi.C.NULL) end))

-- ffi.new and ffi.cast allow unpacked struct/arrays
assert(ffi.new('int[3]', 1)[0] == 1)
assert(ffi.new('int[3]', {1})[0] == 1)
assert(ffi.new('int[3]', 1, 2)[1] == 2)
assert(ffi.new('int[3]', {1, 2})[1] == 2)

ffi.cdef[[
struct var {
    char ch[?];
};
]]
local d = ffi.new('char[4]')
local v = ffi.cast('struct var*', d)
v.ch = {1,2,3,4}
assert(v.ch[3] == 4)
v.ch = "bar"
assert(v.ch[3] == 0)
assert(v.ch[2] == string.byte('r'))
assert(d[1] == string.byte('a'))

ffi.cast('char*', 1)

-- 2 arg form of ffi.copy
ffi.copy(d, 'bar')

-- unsigned should be ignored for pointer rules
ffi.cdef[[
int strncmp(const signed char *s1, const unsigned char *s2, size_t n);
]]
assert(ffi.C.strncmp("two", "three", 3) ~= 0)

ffi.fill(d, 3, 1)
assert(d[2] == 1)
ffi.fill(d, 3)
assert(d[2] == 0)

-- tests for __new
ffi.cdef[[
struct newtest {
    int a;
    int b;
    int c;
};
]]

local tp = ffi.metatype("struct newtest", {__new =
  function(tp, x, y, z)
    tp = ffi.new(tp)
    tp.a, tp.b, tp.c = x, y, z
    return tp
  end})
local v = tp(1, 2, 3)
assert(v.a == 1 and v.b == 2 and v.c == 3)

local tp = ffi.metatype("struct newtest", {__new =
  function(tp, x, y, z)
    tp = ffi.new(tp, {a = x, b = y, c = z})
    return tp
  end})
local v = tp(1, 2, 3)
assert(v.a == 1 and v.b == 2 and v.c == 3)

-- tests for __pairs and __ipairs; not iterating just testing what is returned
local tp = ffi.metatype("struct newtest",
  {__pairs = function(tp) return tp.a, tp.b end, __ipairs = function(tp) return tp.b, tp.c end}
)
if _VERSION ~= 'Lua 5.1' then
    local v = tp(1, 2, 3)
    x, y = pairs(v)
    assert(x == 1 and y == 2)
    x, y = ipairs(v)
    assert(x == 2 and y == 3)
end

-- test for pointer to struct having same metamethods
local st = ffi.cdef "struct ptest {int a, b;};"
local tp = ffi.metatype("struct ptest", {__index = function(s, k) return k end, __len = function(s) return 3 end})

local a = tp(1, 2)
assert(a.banana == "banana")
assert(#a == 3)
local b = ffi.new("int[2]")
local c = ffi.cast("struct ptest *", b)
assert(c.banana == "banana") -- should have same methods
assert(#c == 3)


ffi.cdef [[
char buf[512];
void test_call_echo(const char* c);
void test_call_pppppii(void* a, void* b, void* c, void* d, void* e, int f, int g);
void test_call_pppppiiiiii(void* p1, void* p2, void* p3, void* p4, void* p5, int i1, int i2, int i3, int i4, int i5, int i6);
void test_call_pppppffffff(void* p1, void* p2, void* p3, void* p4, void* p5, float f1, float f2, float f3, float f4, float f5, float f6);
void test_call_pppppiifiii(void* p1, void* p2, void* p3, void* p4, void* p5, int i1, int i2, float f3, int i4, int i5, int i6);
void test_call_pppppiiifii(void* p1, void* p2, void* p3, void* p4, void* p5, int i1, int i2, int i3, float i4, int i5, int i6);
]]

ffi.C.test_call_echo("input")
assert(ffi.C.buf == "input")

local function ptr(x) return ffi.new('void*', x) end

ffi.C.test_call_pppppii(ptr(1), ptr(2), ptr(3), ptr(4), ptr(5), 6, 7)
assert(ffi.C.buf == "0x1 0x2 0x3 0x4 0x5 6 7")

ffi.C.test_call_pppppiiiiii(ptr(1), ptr(2), ptr(3), ptr(4), ptr(5), 6, 7, 8, 9, 10, 11)
assert(ffi.C.buf == "0x1 0x2 0x3 0x4 0x5 6 7 8 9 10 11")

ffi.C.test_call_pppppffffff(ptr(1), ptr(2), ptr(3), ptr(4), ptr(5), 6.5, 7.5, 8.5, 9.5, 10.5, 11.5)
assert(ffi.C.buf == "0x1 0x2 0x3 0x4 0x5 6.5 7.5 8.5 9.5 10.5 11.5")

ffi.C.test_call_pppppiifiii(ptr(1), ptr(2), ptr(3), ptr(4), ptr(5), 6, 7, 8.5, 9, 10, 11)
assert(ffi.C.buf == "0x1 0x2 0x3 0x4 0x5 6 7 8.5 9 10 11")

ffi.C.test_call_pppppiiifii(ptr(1), ptr(2), ptr(3), ptr(4), ptr(5), 6, 7, 8, 9.5, 10, 11)
assert(ffi.C.buf == "0x1 0x2 0x3 0x4 0x5 6 7 8 9.5 10 11")

local sum = ffi.C.add_dc(ffi.new('complex', 1, 2), ffi.new('complex', 3, 5))
assert(ffi.istype('complex', sum))

sum = ffi.C.add_fc(ffi.new('complex float', 1, 2), ffi.new('complex float', 3, 5))
assert(ffi.istype('complex float', sum))

ffi.cdef [[
struct Arrays {
    int ints[3];
    unsigned int uints[3];
};
struct ArrayOfArrays {
    struct Arrays arrays[3];
};
]]

local struct = ffi.new('struct Arrays')
local structOfStructs = ffi.new('struct ArrayOfArrays')
for i=0,2 do
    struct.ints[i] = i
    struct.uints[i] = i
    structOfStructs.arrays[0].ints[i] = i
end
for i=0,2 do
    assert(struct.ints[i] == i)
    assert(struct.uints[i] == i)
    assert(structOfStructs.arrays[0].ints[i] == i)
end

-- Test ffi.string
local buf = ffi.new('char[5]')
ffi.fill(buf, 4, 97)
buf[4] = 0

assert(ffi.string(buf) == 'aaaa')
assert(ffi.string(buf, 4) == 'aaaa')
assert(ffi.string(buf, 2) == 'aa')
assert(ffi.string(buf, 0) == '')
assert(ffi.string(buf, ffi.new('long long', 2)) == 'aa')
assert(ffi.string(buf, ffi.new('int', 2)) == 'aa')

-- Test io.tmpfile()
ffi.cdef [[
    int fprintf ( FILE * stream, const char * format, ... );
]]
local f = io.tmpfile()
ffi.C.fprintf(f, "test: %s\n", "foo")

f:seek("set", 0)
local str = f:read('*l')
assert(str == 'test: foo', str)
f:close()

print('Test PASSED')

