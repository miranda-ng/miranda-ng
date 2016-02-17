/*
** This file has been pre-processed with DynASM.
** http://luajit.org/dynasm.html
** DynASM version 1.3.0, DynASM x64 version 1.3.0
** DO NOT EDIT! The original file is in "call_x86.dasc".
*/

#if DASM_VERSION != 10300
#error "Version mismatch between DynASM and included encoding engine"
#endif

/* vim: ts=4 sw=4 sts=4 et tw=78
 * Portions copyright (c) 2015-present, Facebook, Inc. All rights reserved.
 * Portions copyright (c) 2011 James R. McKaskill.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 */

static const unsigned char build_actionlist[2016] = {
  248,10,184,1,0,0,0,76,139,109,252,240,76,139,101,252,248,72,137,252,236,93,
  195,255,248,11,232,251,1,0,72,185,237,237,137,1,184,0,0,0,0,76,139,109,252,
  240,76,139,101,252,248,72,137,252,236,93,195,255,248,12,102,15,214,68,36,
  32,232,251,1,0,72,185,237,237,137,1,252,243,15,126,68,36,32,76,137,231,232,
  251,1,1,252,233,244,10,255,248,13,15,182,192,137,68,36,32,232,251,1,0,72,
  185,237,237,137,1,139,68,36,32,72,137,198,76,137,231,232,251,1,2,252,233,
  244,10,255,248,14,137,68,36,32,232,251,1,0,72,185,237,237,137,1,139,68,36,
  32,72,137,198,76,137,231,232,251,1,3,252,233,244,10,255,248,15,137,68,36,
  32,232,251,1,0,72,185,237,237,137,1,139,68,36,32,72,137,198,76,137,231,232,
  251,1,4,252,233,244,10,255,248,16,102,184,0,0,72,190,237,237,76,137,231,232,
  251,1,5,255,248,17,102,184,0,0,72,190,237,237,76,137,231,232,251,1,5,255,
  248,18,102,15,214,69,252,240,102,15,214,77,232,102,15,214,85,224,102,15,214,
  93,216,102,15,214,101,208,102,15,214,109,200,102,15,214,117,192,102,15,214,
  125,184,72,137,125,176,72,137,117,168,72,137,85,160,72,137,77,152,76,137,
  69,144,76,137,77,136,195,255,72,139,141,233,255,72,137,132,253,36,233,255,
  221,133,233,255,217,133,233,255,252,243,15,126,133,233,255,252,243,15,90,
  133,233,255,221,156,253,36,233,255,217,156,253,36,233,255,102,15,214,132,
  253,36,233,255,252,242,15,90,192,102,15,214,132,253,36,233,255,252,242,15,
  90,192,102,15,126,132,253,36,233,255,85,72,137,229,65,84,72,129,252,236,239,
  232,244,18,255,73,188,237,237,255,72,199,194,237,72,199,198,237,76,137,231,
  232,251,1,6,255,72,199,194,237,72,199,198,252,255,252,255,252,255,252,255,
  76,137,231,232,251,1,6,255,72,199,194,237,72,199,198,237,76,137,231,232,251,
  1,6,72,186,237,237,72,199,198,252,255,252,255,252,255,252,255,76,137,231,
  232,251,1,7,255,72,137,8,72,199,198,252,254,252,255,252,255,252,255,76,137,
  231,232,251,1,8,255,72,186,237,237,72,199,198,0,0,0,0,76,137,231,232,251,
  1,7,255,72,137,8,255,102,15,214,0,255,217,24,255,217,88,4,255,102,15,214,
  64,8,255,76,137,231,232,251,1,1,255,15,182,201,72,137,206,76,137,231,232,
  251,1,2,255,15,182,201,255,15,190,201,255,72,137,206,76,137,231,232,251,1,
  3,255,15,183,201,255,15,191,201,255,72,137,206,76,137,231,232,251,1,4,255,
  72,185,237,237,72,199,194,237,72,199,198,237,76,137,231,232,251,1,9,255,72,
  199,194,237,72,199,198,252,254,252,255,252,255,252,255,76,137,231,232,251,
  1,6,72,185,237,237,72,199,194,252,255,252,255,252,255,252,255,72,199,198,
  252,254,252,255,252,255,252,255,76,137,231,232,251,1,10,72,137,68,36,32,72,
  199,198,252,252,252,255,252,255,252,255,76,137,231,232,251,1,11,72,139,68,
  36,32,255,72,199,194,237,72,199,198,252,254,252,255,252,255,252,255,76,137,
  231,232,251,1,6,72,185,237,237,72,199,194,252,255,252,255,252,255,252,255,
  72,199,198,252,254,252,255,252,255,252,255,76,137,231,232,251,1,12,137,68,
  36,32,72,199,198,252,252,252,255,252,255,252,255,76,137,231,232,251,1,11,
  139,68,36,32,255,72,199,198,252,254,252,255,252,255,252,255,76,137,231,232,
  251,1,11,255,72,199,198,252,255,252,255,252,255,252,255,76,137,231,232,251,
  1,13,255,72,199,198,252,255,252,255,252,255,252,255,76,137,231,232,251,1,
  14,255,137,68,36,32,72,199,198,252,253,252,255,252,255,252,255,76,137,231,
  232,251,1,11,139,68,36,32,255,72,199,198,252,255,252,255,252,255,252,255,
  76,137,231,232,251,1,15,255,72,199,198,252,255,252,255,252,255,252,255,76,
  137,231,232,251,1,16,255,72,137,68,36,32,72,199,198,252,253,252,255,252,255,
  252,255,76,137,231,232,251,1,11,72,139,68,36,32,255,72,199,198,252,255,252,
  255,252,255,252,255,76,137,231,232,251,1,17,72,137,68,36,32,72,199,198,252,
  253,252,255,252,255,252,255,76,137,231,232,251,1,11,72,139,68,36,32,255,72,
  199,198,252,255,252,255,252,255,252,255,76,137,231,232,251,1,18,102,15,214,
  68,36,32,72,199,198,252,253,252,255,252,255,252,255,76,137,231,232,251,1,
  11,255,252,242,15,90,68,36,32,255,252,243,15,126,68,36,32,255,72,199,198,
  252,255,252,255,252,255,252,255,76,137,231,232,251,1,19,102,15,214,68,36,
  32,72,199,198,252,253,252,255,252,255,252,255,76,137,231,232,251,1,11,252,
  243,15,126,68,36,32,255,72,199,198,252,255,252,255,252,255,252,255,76,137,
  231,232,251,1,20,102,15,214,68,36,32,102,15,214,76,36,40,72,199,198,252,253,
  252,255,252,255,252,255,76,137,231,232,251,1,11,252,243,15,126,68,36,32,252,
  243,15,126,76,36,40,255,72,139,141,233,72,199,194,252,255,252,255,252,255,
  252,255,76,137,230,72,137,207,232,251,1,20,72,131,252,236,4,72,199,198,252,
  253,252,255,252,255,252,255,76,137,231,232,251,1,11,255,76,139,101,252,248,
  72,137,252,236,93,194,236,255,85,72,137,229,65,84,65,85,73,137,252,252,76,
  137,231,232,251,1,21,73,137,197,72,129,252,248,239,15,140,244,16,255,15,143,
  244,17,255,72,193,224,4,72,41,196,72,129,252,236,239,255,72,186,237,237,72,
  199,198,0,0,0,0,76,137,231,232,251,1,7,72,131,252,236,16,255,72,185,237,237,
  72,199,194,237,72,199,198,237,76,137,231,232,251,1,10,255,72,185,237,237,
  72,199,194,237,72,199,198,237,76,137,231,232,251,1,22,255,72,185,237,237,
  72,199,194,237,72,199,198,237,76,137,231,232,251,1,12,255,72,199,198,237,
  76,137,231,232,251,1,14,255,15,182,192,255,15,190,192,255,15,183,192,255,
  15,191,192,255,72,199,198,237,76,137,231,232,251,1,14,131,252,248,0,15,149,
  208,15,182,192,255,72,199,198,237,76,137,231,232,251,1,13,255,72,199,198,
  237,76,137,231,232,251,1,17,255,72,199,198,237,76,137,231,232,251,1,15,255,
  72,199,198,237,76,137,231,232,251,1,16,255,72,199,198,237,76,137,231,232,
  251,1,18,255,72,199,198,237,76,137,231,232,251,1,20,255,252,243,15,126,193,
  255,72,141,132,253,36,233,72,131,252,236,4,72,199,194,237,76,137,230,72,137,
  199,232,251,1,20,255,72,199,198,237,76,137,231,232,251,1,19,255,72,199,198,
  237,76,137,231,232,251,1,19,137,4,36,217,4,36,255,137,20,36,217,4,36,255,
  72,137,224,72,129,192,239,73,137,192,72,199,193,237,76,137,252,234,72,199,
  198,237,76,137,231,232,251,1,23,255,72,137,224,72,129,192,239,73,137,192,
  72,199,193,237,76,137,252,234,72,199,198,237,76,137,231,232,251,1,24,255,
  72,137,224,72,129,192,239,73,137,193,73,199,192,237,72,199,193,237,76,137,
  252,234,72,199,198,237,76,137,231,232,251,1,25,255,72,185,237,237,139,1,72,
  137,199,232,251,1,26,255,72,131,196,32,255,252,243,15,126,188,253,36,233,
  255,252,243,15,126,180,253,36,233,255,252,243,15,126,172,253,36,233,255,252,
  243,15,126,164,253,36,233,255,252,243,15,126,156,253,36,233,255,252,243,15,
  126,148,253,36,233,255,252,243,15,126,140,253,36,233,255,252,243,15,126,132,
  253,36,233,255,76,139,140,253,36,233,255,76,139,132,253,36,233,255,72,139,
  140,253,36,233,255,72,139,148,253,36,233,255,72,139,180,253,36,233,255,72,
  139,60,36,255,72,129,196,239,255,176,8,255,232,251,1,27,72,131,252,236,48,
  255,72,137,68,36,32,232,251,1,0,72,185,237,237,137,1,72,186,237,237,72,199,
  198,237,76,137,231,232,251,1,7,72,139,76,36,32,72,137,8,252,233,244,10,255,
  72,137,68,36,32,232,251,1,0,72,185,237,237,137,1,72,186,237,237,72,199,198,
  0,0,0,0,76,137,231,232,251,1,7,72,139,76,36,32,72,137,8,252,233,244,10,255,
  102,15,214,68,36,32,232,251,1,0,72,185,237,237,137,1,72,186,237,237,72,199,
  198,237,76,137,231,232,251,1,7,72,139,76,36,32,72,137,8,252,233,244,10,255,
  102,15,214,76,36,40,102,15,214,68,36,32,232,251,1,0,72,185,237,237,137,1,
  72,186,237,237,72,199,198,237,76,137,231,232,251,1,7,72,139,76,36,40,72,137,
  72,8,72,139,76,36,32,72,137,8,252,233,244,10,255,252,233,244,11,255,252,233,
  244,13,255,252,233,244,14,255,252,233,244,15,255,252,243,15,90,192,252,233,
  244,12,255
};

static const char *const globnames[] = {
  "lua_return_arg",
  "lua_return_void",
  "lua_return_double",
  "lua_return_bool",
  "lua_return_int",
  "lua_return_uint",
  "too_few_arguments",
  "too_many_arguments",
  "save_registers",
  (const char *)0
};
static const char *const extnames[] = {
  "GetLastError",
  "lua_pushnumber",
  "lua_pushboolean",
  "push_int",
  "push_uint",
  "luaL_error",
  "lua_rawgeti",
  "push_cdata",
  "lua_remove",
  "lua_callk",
  "check_typed_pointer",
  "lua_settop",
  "check_enum",
  "check_uint32",
  "check_int32",
  "check_uint64",
  "check_int64",
  "check_uintptr",
  "check_double",
  "check_complex_float",
  "check_complex_double",
  "lua_gettop",
  "check_typed_cfunction",
  "unpack_varargs_float",
  "unpack_varargs_int",
  "unpack_varargs_stack_skip",
  "SetLastError",
  "FUNCTION",
  (const char *)0
};



#if defined _WIN64 || defined __amd64__
#define JUMP_SIZE 14
#else
#define JUMP_SIZE 4
#endif

#define MIN_BRANCH INT32_MIN
#define MAX_BRANCH INT32_MAX
#define BRANCH_OFF 4

static void compile_extern_jump(struct jit* jit, lua_State* L, cfunction func, uint8_t* code)
{
    /* The jump code is the function pointer followed by a stub to call the
     * function pointer. The stub exists in 64 bit so we can jump to functions
     * with an offset greater than 2 GB.
     *
     * Note we have to manually set this up since there are commands buffered
     * in the jit state and dynasm doesn't support rip relative addressing.
     *
     * eg on 64 bit:
     * 0-8: function ptr
     * 8-14: jmp aword [rip-14]
     *
     * for 32 bit we only set the function ptr as it can always fit in a 32
     * bit displacement
     */
#if defined _WIN64 || defined __amd64__
    *(cfunction*) code = func;
    code[8] = 0xFF; /* FF /4 operand for jmp */
    code[9] = 0x25; /* RIP displacement */
    *(int32_t*) &code[10] = -14;
#else
    *(cfunction*) code = func;
#endif
}

void compile_globals(struct jit* jit, lua_State* L)
{
    struct jit* Dst = jit;
    int* perr = &jit->last_errno;
    dasm_setup(Dst, build_actionlist);

    /* Note: since the return code uses EBP to reset the stack pointer, we
     * don't have to track the amount of stack space used. It also means we
     * can handle stdcall and cdecl with the same code.
     */

    /* Note the various call_* functions want 32 bytes of 16 byte aligned
     * stack
     */




    /* the general idea for the return functions is:
     * 1) Save return value on stack
     * 2) Call get_errno (this trashes the registers hence #1)
     * 3) Unpack return value from stack
     * 4) Call lua push function
     * 5) Set eax to number of returned args (0 or 1)
     * 6) Call return which pops our stack frame
     */

    dasm_put(Dst, 0);

    dasm_put(Dst, 24, (unsigned int)((uintptr_t)(perr)), (unsigned int)(((uintptr_t)(perr))>>32));

    dasm_put(Dst, 58, (unsigned int)((uintptr_t)(perr)), (unsigned int)(((uintptr_t)(perr))>>32));

    dasm_put(Dst, 95, (unsigned int)((uintptr_t)(perr)), (unsigned int)(((uintptr_t)(perr))>>32));

    dasm_put(Dst, 133, (unsigned int)((uintptr_t)(perr)), (unsigned int)(((uintptr_t)(perr))>>32));

    dasm_put(Dst, 168, (unsigned int)((uintptr_t)(perr)), (unsigned int)(((uintptr_t)(perr))>>32));

    dasm_put(Dst, 203, (unsigned int)((uintptr_t)(&"too few arguments")), (unsigned int)(((uintptr_t)(&"too few arguments"))>>32));

    dasm_put(Dst, 221, (unsigned int)((uintptr_t)(&"too many arguments")), (unsigned int)(((uintptr_t)(&"too many arguments"))>>32));

    dasm_put(Dst, 239);

    compile(Dst, L, NULL, LUA_NOREF);
}

int x86_return_size(lua_State* L, int usr, const struct ctype* ct)
{
    int ret = 0;
    const struct ctype* mt;

    if (ct->calling_convention != C_CALL) {
        size_t i;
        size_t argn = lua_rawlen(L, usr);
        for (i = 1; i <= argn; i++) {
            lua_rawgeti(L, usr, (int) i);
            mt = (const struct ctype*) lua_touserdata(L, -1);

            if (mt->pointers || mt->is_reference) {
                ret += sizeof(void*);
            } else {
                switch (mt->type) {
                case DOUBLE_TYPE:
                case COMPLEX_FLOAT_TYPE:
                case INT64_TYPE:
                    ret += 8;
                    break;
                case COMPLEX_DOUBLE_TYPE:
                    ret += 16;
                    break;
                case INTPTR_TYPE:
                    ret += sizeof(intptr_t);
                    break;
                case FUNCTION_PTR_TYPE:
                    ret += sizeof(cfunction);
                    break;
                case BOOL_TYPE:
                case FLOAT_TYPE:
                case INT8_TYPE:
                case INT16_TYPE:
                case INT32_TYPE:
                case ENUM_TYPE:
                    ret += 4;
                    break;
                default:
                    return luaL_error(L, "NYI - argument type");
                }
            }

            lua_pop(L, 1);
        }
    }

#if !defined _WIN64 && !defined __amd64__
    lua_rawgeti(L, usr, 0);
    mt = (const struct ctype*) lua_touserdata(L, -1);
    if (!mt->pointers && !mt->is_reference && mt->type == COMPLEX_DOUBLE_TYPE) {
        ret += sizeof(void*);
    }
    lua_pop(L, 1);
#endif

    return ret;
}

#ifdef _WIN64
#define MAX_REGISTERS(ct) 4 /* rcx, rdx, r8, r9 */

#elif defined __amd64__
#define MAX_INT_REGISTERS(ct) 6 /* rdi, rsi, rdx, rcx, r8, r9 */
#define MAX_FLOAT_REGISTERS(ct) 8 /* xmm0-7 */

#else
#define MAX_INT_REGISTERS(ct) ((ct)->calling_convention == FAST_CALL ? 2 /* ecx, edx */ : 0)
#define MAX_FLOAT_REGISTERS(ct) 0
#endif

struct reg_alloc {
#ifdef _WIN64
    int regs;
    int is_float[4];
    int is_int[4];
#else
    int floats;
    int ints;
#endif
    int off;
};

#ifdef _WIN64
#define REGISTER_STACK_SPACE(ct) (4*8)
#elif defined __amd64__
#define REGISTER_STACK_SPACE(ct) (14*8)
#else
#define REGISTER_STACK_SPACE(ct) ALIGN_UP(((ct)->calling_convention == FAST_CALL ? 2*4 : 0), 15)
#endif

/* Fastcall:
 * Uses ecx, edx as first two int registers
 * Everything else on stack (include 64bit ints)
 * No overflow stack space
 * Pops the stack before returning
 * Returns int in eax, float in ST0
 * We use the same register allocation logic as posix x64 with 2 int regs and 0 float regs
 */

static void get_int(Dst_DECL, const struct ctype* ct, struct reg_alloc* reg, int is_int64)
{
    /* grab the register from the shadow space */
#ifdef _WIN64
    if (reg->regs < MAX_REGISTERS(ct)) {
        dasm_put(Dst, 308, 16 + 8*reg->regs);
        reg->regs++;
    }
#elif __amd64__
    if (reg->ints < MAX_INT_REGISTERS(ct)) {
        dasm_put(Dst, 308, - 80 - 8*reg->ints);
        reg->ints++;
    }
#else
    if (!is_int64 && reg->ints < MAX_INT_REGISTERS(ct)) {
        dasm_put(Dst, 309, - 8 - 4*reg->ints);
        reg->ints++;
    }
#endif
    else if (is_int64) {
        dasm_put(Dst, 308, reg->off);
        reg->off += 8;
    } else {
        dasm_put(Dst, 309, reg->off);
        reg->off += 4;
    }
}

static void add_int(Dst_DECL, const struct ctype* ct, struct reg_alloc* reg, int is_int64)
{
#ifdef _WIN64
    if (reg->regs < MAX_REGISTERS(ct)) {
        dasm_put(Dst, 313, 32 + 8*(reg->regs));
        reg->is_int[reg->regs++] = 1;
    }
#elif __amd64__
    if (reg->ints < MAX_INT_REGISTERS(ct)) {
        dasm_put(Dst, 313, 32 + 8*reg->ints);
        reg->ints++;
    }
#else
    if (!is_int64 && reg->ints < MAX_INT_REGISTERS(ct)) {
        dasm_put(Dst, 313, 32 + 4*reg->ints);
        reg->ints++;
    }
#endif
    else {
#if defined _WIN64 || defined __amd64__
        if (reg->off % 8 != 0) {
            reg->off += 8 - (reg->off % 8);
        }
#endif
        if (is_int64) {
            dasm_put(Dst, 313, reg->off);
            reg->off += 8;
        } else {
            dasm_put(Dst, 314, reg->off);
            reg->off += 4;
        }
    }
}

static void get_float(Dst_DECL, const struct ctype* ct, struct reg_alloc* reg, int is_double)
{
#if !defined _WIN64 && !defined __amd64__
    assert(MAX_FLOAT_REGISTERS(ct) == 0);
    if (is_double) {
        dasm_put(Dst, 320, reg->off);
        reg->off += 8;
    } else {
        dasm_put(Dst, 324, reg->off);
        reg->off += 4;
    }
#else
    int off;

#ifdef _WIN64
    if (reg->regs < MAX_REGISTERS(ct)) {
        off = -16 - 8*reg->regs;
        reg->regs++;
    }
#else
    if (reg->floats < MAX_FLOAT_REGISTERS(ct)) {
        off = -16 - 8*reg->floats;
        reg->floats++;
    }
#endif
    else {
        off = reg->off;
        reg->off += is_double ? 8 : 4;
    }

    if (is_double) {
        dasm_put(Dst, 328, off);
    } else {
        dasm_put(Dst, 335, off);
    }
#endif
}

static void add_float(Dst_DECL, const struct ctype* ct, struct reg_alloc* reg, int is_double)
{
#if !defined _WIN64 && !defined __amd64__
    assert(MAX_FLOAT_REGISTERS(ct) == 0);
    if (is_double) {
        dasm_put(Dst, 342, reg->off);
        reg->off += 8;
    } else {
        dasm_put(Dst, 348, reg->off);
        reg->off += 4;
    }
#else

#ifdef _WIN64
    if (reg->regs < MAX_REGISTERS(ct)) {
        if (is_double) {
            dasm_put(Dst, 354, 32 + 8*(reg->regs));
        } else {
            dasm_put(Dst, 362, 32 + 8*(reg->regs));
        }
        reg->is_float[reg->regs++] = 1;
    }
#else
    if (reg->floats < MAX_FLOAT_REGISTERS(ct)) {
        if (is_double) {
            dasm_put(Dst, 354, 32 + 8*(MAX_INT_REGISTERS(ct) + reg->floats));
        } else {
            dasm_put(Dst, 362, 32 + 8*(MAX_INT_REGISTERS(ct) + reg->floats));
        }
        reg->floats++;
    }
#endif

    else if (is_double) {
        dasm_put(Dst, 354, reg->off);
        reg->off += 8;
    } else {
        dasm_put(Dst, 375, reg->off);
        reg->off += 4;
    }
#endif
}

#if defined _WIN64 || defined __amd64__
#define add_pointer(jit, ct, reg) add_int(jit, ct, reg, 1)
#define get_pointer(jit, ct, reg) get_int(jit, ct, reg, 1)
#else
#define add_pointer(jit, ct, reg) add_int(jit, ct, reg, 0)
#define get_pointer(jit, ct, reg) get_int(jit, ct, reg, 0)
#endif

cfunction compile_callback(lua_State* L, int fidx, int ct_usr, const struct ctype* ct)
{
    int i, nargs;
    cfunction* pf;
    struct ctype ct2 = *ct;
    const struct ctype* mt;
    struct reg_alloc reg;
    int num_upvals = 0;
    int top = lua_gettop(L);
    struct jit* Dst = get_jit(L);
    int ref;
    int hidden_arg_off = 0;

    ct_usr = lua_absindex(L, ct_usr);
    fidx = lua_absindex(L, fidx);

    assert(lua_isnil(L, fidx) || lua_isfunction(L, fidx));

    memset(&reg, 0, sizeof(reg));
#ifdef _WIN64
    reg.off = 16 + REGISTER_STACK_SPACE(ct); /* stack registers are above the shadow space */
#elif __amd64__
    reg.off = 16;
#else
    reg.off = 8;
#endif

    dasm_setup(Dst, build_actionlist);

    // add a table to store ctype and function upvalues
    // callback_set assumes the first value is the lua function
    nargs = (int) lua_rawlen(L, ct_usr);
    lua_newtable(L);
    lua_pushvalue(L, -1);
    ref = luaL_ref(L, LUA_REGISTRYINDEX);

    if (ct->has_var_arg) {
        luaL_error(L, "can't create callbacks with varargs");
    }

    // setup a stack frame to hold args for the call into lua_call

    dasm_put(Dst, 388, 8 + 16 + 32 + REGISTER_STACK_SPACE(ct));
    if (ct->calling_convention == FAST_CALL) {
    }

    // hardcode the lua_State* value into the assembly
    dasm_put(Dst, 403, (unsigned int)((uintptr_t)(L)), (unsigned int)(((uintptr_t)(L))>>32));

    /* get the upval table */
    dasm_put(Dst, 408, ref, LUA_REGISTRYINDEX);

    /* get the lua function */
    lua_pushvalue(L, fidx);
    lua_rawseti(L, -2, ++num_upvals);
    assert(num_upvals == CALLBACK_FUNC_USR_IDX);
    dasm_put(Dst, 424, num_upvals);

#if !defined _WIN64 && !defined __amd64__
    lua_rawgeti(L, ct_usr, 0);
    mt = (const struct ctype*) lua_touserdata(L, -1);
    if (!mt->pointers && !mt->is_reference && mt->type == COMPLEX_DOUBLE_TYPE) {
        hidden_arg_off = reg.off;
        reg.off += sizeof(void*);
    }
    lua_pop(L, 1);
#else
    (void) hidden_arg_off;
#endif

    for (i = 1; i <= nargs; i++) {
        lua_rawgeti(L, ct_usr, i);
        mt = (const struct ctype*) lua_touserdata(L, -1);

        if (mt->pointers || mt->is_reference) {
            lua_getuservalue(L, -1);
            lua_rawseti(L, -3, ++num_upvals); /* usr value */
            lua_rawseti(L, -2, ++num_upvals); /* mt */
            /* on the lua stack in the callback:
             * upval tbl, lua func, i-1 args
             */
            dasm_put(Dst, 447, num_upvals-1, -i-1, (unsigned int)((uintptr_t)(mt)), (unsigned int)(((uintptr_t)(mt))>>32));
            get_pointer(Dst, ct, &reg);
            dasm_put(Dst, 485);
        } else {
            switch (mt->type) {
            case INT64_TYPE:
                lua_getuservalue(L, -1);
                lua_rawseti(L, -3, ++num_upvals); /* mt */
                lua_pop(L, 1);
                dasm_put(Dst, 507, (unsigned int)((uintptr_t)(mt)), (unsigned int)(((uintptr_t)(mt))>>32));
                get_int(Dst, ct, &reg, 1);
                dasm_put(Dst, 526);
                break;

            case INTPTR_TYPE:
                lua_getuservalue(L, -1);
                lua_rawseti(L, -3, ++num_upvals); /* mt */
                lua_pop(L, 1);
                dasm_put(Dst, 507, (unsigned int)((uintptr_t)(mt)), (unsigned int)(((uintptr_t)(mt))>>32));
                get_pointer(Dst, ct, &reg);
                dasm_put(Dst, 526);
                break;

            case COMPLEX_FLOAT_TYPE:
                lua_pop(L, 1);
#if defined _WIN64 || defined __amd64__
                /* complex floats are two floats packed into a double */
                dasm_put(Dst, 507, (unsigned int)((uintptr_t)(mt)), (unsigned int)(((uintptr_t)(mt))>>32));
                get_float(Dst, ct, &reg, 1);
                dasm_put(Dst, 530);
#else
                /* complex floats are real followed by imag on the stack */
                dasm_put(Dst, 507, (unsigned int)((uintptr_t)(mt)), (unsigned int)(((uintptr_t)(mt))>>32));
                get_float(Dst, ct, &reg, 0);
                dasm_put(Dst, 535);
                get_float(Dst, ct, &reg, 0);
                dasm_put(Dst, 538);
#endif
                break;

            case COMPLEX_DOUBLE_TYPE:
                lua_pop(L, 1);
                dasm_put(Dst, 507, (unsigned int)((uintptr_t)(mt)), (unsigned int)(((uintptr_t)(mt))>>32));
                /* real */
                get_float(Dst, ct, &reg, 1);
                dasm_put(Dst, 530);
                /* imag */
                get_float(Dst, ct, &reg, 1);
                dasm_put(Dst, 542);
                break;

            case FLOAT_TYPE:
            case DOUBLE_TYPE:
                lua_pop(L, 1);
                get_float(Dst, ct, &reg, mt->type == DOUBLE_TYPE);
                dasm_put(Dst, 548);
                break;

            case BOOL_TYPE:
                lua_pop(L, 1);
                get_int(Dst, ct, &reg, 0);
                dasm_put(Dst, 556);
                break;

            case INT8_TYPE:
                lua_pop(L, 1);
                get_int(Dst, ct, &reg, 0);
                if (mt->is_unsigned) {
                    dasm_put(Dst, 570);
                } else {
                    dasm_put(Dst, 574);
                }
                dasm_put(Dst, 578);
                break;

            case INT16_TYPE:
                lua_pop(L, 1);
                get_int(Dst, ct, &reg, 0);
                if (mt->is_unsigned) {
                    dasm_put(Dst, 589);
                } else {
                    dasm_put(Dst, 593);
                }
                dasm_put(Dst, 578);
                break;

            case ENUM_TYPE:
            case INT32_TYPE:
                lua_pop(L, 1);
                get_int(Dst, ct, &reg, 0);
                if (mt->is_unsigned) {
                    dasm_put(Dst, 597);
                } else {
                    dasm_put(Dst, 578);
                }
                break;

            default:
                luaL_error(L, "NYI: callback arg type");
            }
        }
    }

    lua_rawgeti(L, ct_usr, 0);
    mt = (const struct ctype*) lua_touserdata(L, -1);

    dasm_put(Dst, 608, (unsigned int)((uintptr_t)(0)), (unsigned int)(((uintptr_t)(0))>>32), (mt->pointers || mt->is_reference || mt->type != VOID_TYPE) ? 1 : 0, nargs);

    // Unpack the return argument if not "void", also clean-up the lua stack
    // to remove the return argument and bind table. Use lua_settop rather
    // than lua_pop as lua_pop is implemented as a macro.
    if (mt->pointers || mt->is_reference) {
        lua_getuservalue(L, -1);
        lua_rawseti(L, -3, ++num_upvals); /* usr value */
        lua_rawseti(L, -2, ++num_upvals); /* mt */
        dasm_put(Dst, 628, num_upvals-1, (unsigned int)((uintptr_t)(mt)), (unsigned int)(((uintptr_t)(mt))>>32));

    } else {
        switch (mt->type) {
        case ENUM_TYPE:
            lua_getuservalue(L, -1);
            lua_rawseti(L, -3, ++num_upvals); /* usr value */
            lua_rawseti(L, -2, ++num_upvals); /* mt */
            dasm_put(Dst, 712, num_upvals-1, (unsigned int)((uintptr_t)(mt)), (unsigned int)(((uintptr_t)(mt))>>32));
            break;

        case VOID_TYPE:
            lua_pop(L, 1);
            dasm_put(Dst, 794);
            break;

        case BOOL_TYPE:
        case INT8_TYPE:
        case INT16_TYPE:
        case INT32_TYPE:
            lua_pop(L, 1);
            if (mt->is_unsigned) {
                dasm_put(Dst, 813);
            } else {
                dasm_put(Dst, 832);
            }
            dasm_put(Dst, 851);
            break;

        case INT64_TYPE:
            lua_pop(L, 1);

            if (mt->is_unsigned) {
                dasm_put(Dst, 878);
            } else {
                dasm_put(Dst, 897);
            }

            dasm_put(Dst, 916);
            break;

        case INTPTR_TYPE:
            lua_pop(L, 1);
            dasm_put(Dst, 945);
            break;

        case FLOAT_TYPE:
        case DOUBLE_TYPE:
            lua_pop(L, 1);
            dasm_put(Dst, 992);
            if (mt->type == FLOAT_TYPE) {
                dasm_put(Dst, 1035);
            } else {
                dasm_put(Dst, 1043);
            }
            break;

        case COMPLEX_FLOAT_TYPE:
            lua_pop(L, 1);
#if !defined HAVE_COMPLEX
            luaL_error(L, "ffi lib compiled without complex number support");
#endif
            /* on 64 bit complex floats are two floats packed into a double,
             * on 32 bit returned complex floats use eax and edx */
            dasm_put(Dst, 1051);
            break;

        case COMPLEX_DOUBLE_TYPE:
            lua_pop(L, 1);
#if !defined HAVE_COMPLEX
            luaL_error(L, "ffi lib compiled without complex number support");
#endif
            /* on 64 bit, returned complex doubles use xmm0, xmm1, on 32 bit
             * there is a hidden first parameter that points to 16 bytes where
             * the returned arg is stored which is popped by the called
             * function */
#if defined _WIN64 || defined __amd64__
            dasm_put(Dst, 1101);
#else
            dasm_put(Dst, 1164, hidden_arg_off);
#endif
            break;

        default:
            luaL_error(L, "NYI: callback return type");
        }
    }

    dasm_put(Dst, 1213, x86_return_size(L, ct_usr, ct));

    lua_pop(L, 1); /* upval table - already in registry */
    assert(lua_gettop(L) == top);

    ct2.is_jitted = 1;
    pf = (cfunction*) push_cdata(L, ct_usr, &ct2);
    *pf = compile(Dst, L, NULL, ref);

    assert(lua_gettop(L) == top + 1);

    return *pf;
}

void compile_function(lua_State* L, cfunction func, int ct_usr, const struct ctype* ct)
{
    size_t i, nargs;
    int num_upvals;
    const struct ctype* mbr_ct;
    struct jit* Dst = get_jit(L);
    struct reg_alloc reg;
    void* p;
    int top = lua_gettop(L);
    int* perr = &Dst->last_errno;

    ct_usr = lua_absindex(L, ct_usr);

    memset(&reg, 0, sizeof(reg));
    reg.off = 32 + REGISTER_STACK_SPACE(ct);

    dasm_setup(Dst, build_actionlist);

    p = push_cdata(L, ct_usr, ct);
    *(cfunction*) p = func;
    num_upvals = 1;

    nargs = lua_rawlen(L, ct_usr);

    if (ct->calling_convention != C_CALL && ct->has_var_arg) {
        luaL_error(L, "vararg is only allowed with the c calling convention");
    }

    dasm_put(Dst, 1226, nargs);

    if (!ct->has_var_arg) {
        dasm_put(Dst, 1258);
    }

    /* no need to zero extend eax returned by lua_gettop to rax as x86-64
     * preguarentees that the upper 32 bits will be zero */
    dasm_put(Dst, 1263, 32 + REGISTER_STACK_SPACE(ct));

#if !defined _WIN64 && !defined __amd64__
    /* Returned complex doubles require a hidden first parameter where the
     * data is stored, which is popped by the calling code. */
    lua_rawgeti(L, ct_usr, 0);
    mbr_ct = (const struct ctype*) lua_touserdata(L, -1);
    if (!mbr_ct->pointers && !mbr_ct->is_reference && mbr_ct->type == COMPLEX_DOUBLE_TYPE) {
        /* we can allocate more space for arguments as long as no add_*
         * function has been called yet, mbr_ct will be added as an upvalue in
         * the return processing later */
        dasm_put(Dst, 1276, (unsigned int)((uintptr_t)(mbr_ct)), (unsigned int)(((uintptr_t)(mbr_ct))>>32));
        add_pointer(Dst, ct, &reg);
    }
    lua_pop(L, 1);
#endif

    for (i = 1; i <= nargs; i++) {
        lua_rawgeti(L, ct_usr, (int) i);
        mbr_ct = (const struct ctype*) lua_touserdata(L, -1);

        if (mbr_ct->pointers || mbr_ct->is_reference) {
            lua_getuservalue(L, -1);
            num_upvals += 2;
            dasm_put(Dst, 1300, (unsigned int)((uintptr_t)(mbr_ct)), (unsigned int)(((uintptr_t)(mbr_ct))>>32), lua_upvalueindex(num_upvals), i);
            add_pointer(Dst, ct, &reg);
        } else {
            switch (mbr_ct->type) {
            case FUNCTION_PTR_TYPE:
                lua_getuservalue(L, -1);
                num_upvals += 2;
                dasm_put(Dst, 1320, (unsigned int)((uintptr_t)(mbr_ct)), (unsigned int)(((uintptr_t)(mbr_ct))>>32), lua_upvalueindex(num_upvals), i);
                add_pointer(Dst, ct, &reg);
                break;

            case ENUM_TYPE:
                lua_getuservalue(L, -1);
                num_upvals += 2;
                dasm_put(Dst, 1340, (unsigned int)((uintptr_t)(mbr_ct)), (unsigned int)(((uintptr_t)(mbr_ct))>>32), lua_upvalueindex(num_upvals), i);
                add_int(Dst, ct, &reg, 0);
                break;

            case INT8_TYPE:
                dasm_put(Dst, 1360, i);
                if (mbr_ct->is_unsigned) {
                    dasm_put(Dst, 1372);
                } else {
                    dasm_put(Dst, 1376);
                }
                add_int(Dst, ct, &reg, 0);
                lua_pop(L, 1);
                break;

            case INT16_TYPE:
                dasm_put(Dst, 1360, i);
                if (mbr_ct->is_unsigned) {
                    dasm_put(Dst, 1380);
                } else {
                    dasm_put(Dst, 1384);
                }
                add_int(Dst, ct, &reg, 0);
                lua_pop(L, 1);
                break;

            case BOOL_TYPE:
                dasm_put(Dst, 1388, i);
                add_int(Dst, ct, &reg, 0);
                lua_pop(L, 1);
                break;

            case INT32_TYPE:
                if (mbr_ct->is_unsigned) {
                    dasm_put(Dst, 1410, i);
                } else {
                    dasm_put(Dst, 1360, i);
                }
                add_int(Dst, ct, &reg, 0);
                lua_pop(L, 1);
                break;

            case INTPTR_TYPE:
                dasm_put(Dst, 1422, i);
                add_pointer(Dst, ct, &reg);
                lua_pop(L, 1);
                break;

            case INT64_TYPE:
                if (mbr_ct->is_unsigned) {
                    dasm_put(Dst, 1434, i);
                } else {
                    dasm_put(Dst, 1446, i);
                }
                add_int(Dst, ct, &reg, 1);
                lua_pop(L, 1);
                break;

            case DOUBLE_TYPE:
                dasm_put(Dst, 1458, i);
                add_float(Dst, ct, &reg, 1);
                lua_pop(L, 1);
                break;

            case COMPLEX_DOUBLE_TYPE:
                /* on 64 bit, returned complex doubles use xmm0, xmm1, on 32 bit
                 * there is a hidden first parameter that points to 16 bytes where
                 * the returned arg is stored (this is popped by the called
                 * function) */
#if defined _WIN64 || defined __amd64__
                dasm_put(Dst, 1470, i);
                add_float(Dst, ct, &reg, 1);
                dasm_put(Dst, 1482);
                add_float(Dst, ct, &reg, 1);
#else
                dasm_put(Dst, 1488, reg.off, i);
                reg.off += 16;
#endif
                lua_pop(L, 1);
                break;

            case FLOAT_TYPE:
                dasm_put(Dst, 1458, i);
                add_float(Dst, ct, &reg, 0);
                lua_pop(L, 1);
                break;

            case COMPLEX_FLOAT_TYPE:
#if defined _WIN64 || defined __amd64__
                dasm_put(Dst, 1514, i);
                /* complex floats are two floats packed into a double */
                add_float(Dst, ct, &reg, 1);
#else
                /* returned complex floats use eax and edx */
                dasm_put(Dst, 1526, i);
                add_float(Dst, ct, &reg, 0);
                dasm_put(Dst, 1544);
                add_float(Dst, ct, &reg, 0);
#endif
                lua_pop(L, 1);
                break;

            default:
                luaL_error(L, "NYI: call arg type");
            }
        }
    }

    if (ct->has_var_arg) {
#ifdef _WIN64
        if (reg.regs < MAX_REGISTERS(ct)) {
            assert(reg.regs == nargs);
        } else {
        }

        for (i = nargs; i < MAX_REGISTERS(ct); i++) {
            reg.is_int[i] = reg.is_float[i] = 1;
        }
        reg.regs = MAX_REGISTERS(ct);
#elif defined __amd64__
        if (reg.floats < MAX_FLOAT_REGISTERS(ct)) {
            dasm_put(Dst, 1551, 32 + 8*(MAX_INT_REGISTERS(ct) + reg.floats), MAX_FLOAT_REGISTERS(ct) - reg.floats, nargs+1);
        }

        if (reg.ints < MAX_INT_REGISTERS(ct)) {
            dasm_put(Dst, 1581, 32 + 8*(reg.ints), MAX_INT_REGISTERS(ct) - reg.ints, nargs+1);
        }

        dasm_put(Dst, 1611, reg.off, MAX_FLOAT_REGISTERS(ct) - reg.floats, MAX_INT_REGISTERS(ct) - reg.ints, nargs+1);

        reg.floats = MAX_FLOAT_REGISTERS(ct);
        reg.ints = MAX_INT_REGISTERS(ct);
#else
#endif
    }

    dasm_put(Dst, 1645, (unsigned int)((uintptr_t)(perr)), (unsigned int)(((uintptr_t)(perr))>>32));

    /* remove the stack space to call local functions */
    dasm_put(Dst, 1659);

#ifdef _WIN64
    switch (reg.regs) {
    case 4:
        if (reg.is_float[3]) {
        }
        if (reg.is_int[3]) {
        }
    case 3:
        if (reg.is_float[2]) {
        }
        if (reg.is_int[2]) {
        }
    case 2:
        if (reg.is_float[1]) {
        }
        if (reg.is_int[1]) {
        }
    case 1:
        if (reg.is_float[0]) {
        }
        if (reg.is_int[0]) {
        }
    case 0:
        break;
    }

    /* don't remove the space for the registers as we need 32 bytes of register overflow space */
    assert(REGISTER_STACK_SPACE(ct) == 32);

#elif defined __amd64__
    switch (reg.floats) {
    case 8:
        dasm_put(Dst, 1664, 8*(MAX_INT_REGISTERS(ct)+7));
    case 7:
        dasm_put(Dst, 1673, 8*(MAX_INT_REGISTERS(ct)+6));
    case 6:
        dasm_put(Dst, 1682, 8*(MAX_INT_REGISTERS(ct)+5));
    case 5:
        dasm_put(Dst, 1691, 8*(MAX_INT_REGISTERS(ct)+4));
    case 4:
        dasm_put(Dst, 1700, 8*(MAX_INT_REGISTERS(ct)+3));
    case 3:
        dasm_put(Dst, 1709, 8*(MAX_INT_REGISTERS(ct)+2));
    case 2:
        dasm_put(Dst, 1718, 8*(MAX_INT_REGISTERS(ct)+1));
    case 1:
        dasm_put(Dst, 1727, 8*(MAX_INT_REGISTERS(ct)));
    case 0:
        break;
    }

    switch (reg.ints) {
    case 6:
        dasm_put(Dst, 1736, 8*5);
    case 5:
        dasm_put(Dst, 1743, 8*4);
    case 4:
        dasm_put(Dst, 1750, 8*3);
    case 3:
        dasm_put(Dst, 1757, 8*2);
    case 2:
        dasm_put(Dst, 1764, 8*1);
    case 1:
        dasm_put(Dst, 1771);
    case 0:
        break;
    }

    dasm_put(Dst, 1776, REGISTER_STACK_SPACE(ct));
#else
    if (ct->calling_convention == FAST_CALL) {
        switch (reg.ints) {
        case 2:
        case 1:
        case 0:
            break;
        }

    }
#endif

#ifdef __amd64__
    if (ct->has_var_arg) {
        /* al stores an upper limit on the number of float register, note that
         * its allowed to be more than the actual number of float registers used as
         * long as its 0-8 */
        dasm_put(Dst, 1781);
    }
#endif

    dasm_put(Dst, 1784);

    /* note on windows X86 the stack may be only aligned to 4 (stdcall will
     * have popped a multiple of 4 bytes), but we don't need 16 byte alignment on
     * that platform
     */

    lua_rawgeti(L, ct_usr, 0);
    mbr_ct = (const struct ctype*) lua_touserdata(L, -1);

    if (mbr_ct->pointers || mbr_ct->is_reference || mbr_ct->type == INTPTR_TYPE) {
        lua_getuservalue(L, -1);
        num_upvals += 2;
        dasm_put(Dst, 1794, (unsigned int)((uintptr_t)(perr)), (unsigned int)(((uintptr_t)(perr))>>32), (unsigned int)((uintptr_t)(mbr_ct)), (unsigned int)(((uintptr_t)(mbr_ct))>>32), lua_upvalueindex(num_upvals));

    } else {
        switch (mbr_ct->type) {
        case FUNCTION_PTR_TYPE:
            lua_getuservalue(L, -1);
            num_upvals += 2;
            dasm_put(Dst, 1794, (unsigned int)((uintptr_t)(perr)), (unsigned int)(((uintptr_t)(perr))>>32), (unsigned int)((uintptr_t)(mbr_ct)), (unsigned int)(((uintptr_t)(mbr_ct))>>32), lua_upvalueindex(num_upvals));
            break;

        case INT64_TYPE:
            num_upvals++;
            dasm_put(Dst, 1837, (unsigned int)((uintptr_t)(perr)), (unsigned int)(((uintptr_t)(perr))>>32), (unsigned int)((uintptr_t)(mbr_ct)), (unsigned int)(((uintptr_t)(mbr_ct))>>32));
            break;

        case COMPLEX_FLOAT_TYPE:
            lua_getuservalue(L, -1);
            num_upvals += 2;
            dasm_put(Dst, 1883, (unsigned int)((uintptr_t)(perr)), (unsigned int)(((uintptr_t)(perr))>>32), (unsigned int)((uintptr_t)(mbr_ct)), (unsigned int)(((uintptr_t)(mbr_ct))>>32), lua_upvalueindex(num_upvals));
            break;

        case COMPLEX_DOUBLE_TYPE:
            lua_getuservalue(L, -1);
            num_upvals += 2;
            dasm_put(Dst, 1927, (unsigned int)((uintptr_t)(perr)), (unsigned int)(((uintptr_t)(perr))>>32), (unsigned int)((uintptr_t)(mbr_ct)), (unsigned int)(((uintptr_t)(mbr_ct))>>32), lua_upvalueindex(num_upvals));
            break;

        case VOID_TYPE:
            lua_pop(L, 1);
            dasm_put(Dst, 1986);
            break;

        case BOOL_TYPE:
            lua_pop(L, 1);
            dasm_put(Dst, 1991);
            break;

        case INT8_TYPE:
            lua_pop(L, 1);
            if (mbr_ct->is_unsigned) {
                dasm_put(Dst, 1372);
            } else {
                dasm_put(Dst, 1376);
            }
            dasm_put(Dst, 1996);
            break;

        case INT16_TYPE:
            lua_pop(L, 1);
            if (mbr_ct->is_unsigned) {
                dasm_put(Dst, 1380);
            } else {
                dasm_put(Dst, 1384);
            }
            dasm_put(Dst, 1996);
            break;

        case INT32_TYPE:
        case ENUM_TYPE:
            lua_pop(L, 1);
            if (mbr_ct->is_unsigned) {
                dasm_put(Dst, 2001);
            } else {
                dasm_put(Dst, 1996);
            }
            break;

        case FLOAT_TYPE:
            lua_pop(L, 1);
            dasm_put(Dst, 2006);
            break;

        case DOUBLE_TYPE:
            lua_pop(L, 1);
            dasm_put(Dst, 2011);
            break;

        default:
            luaL_error(L, "NYI: call return type");
        }
    }

    assert(lua_gettop(L) == top + num_upvals);
    {
        cfunction f = compile(Dst, L, func, LUA_NOREF);
        /* add a callback as an upval so that the jitted code gets cleaned up when
         * the function gets gc'd */
        push_callback(L, f, func);
        lua_pushcclosure(L, (lua_CFunction) f, num_upvals+1);
    }
}

