// Tencent is pleased to support the open source community by making ncnn available.
//
// Copyright (C) 2023 THL A29 Limited, a Tencent company. All rights reserved.
//
// Licensed under the BSD 3-Clause License (the "License"); you may not use this file except
// in compliance with the License. You may obtain a copy of the License at
//
// https://opensource.org/licenses/BSD-3-Clause
//
// Unless required by applicable law or agreed to in writing, software distributed
// under the License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
// CONDITIONS OF ANY KIND, either express or implied. See the License for the
// specific language governing permissions and limitations under the License.

#include <stdio.h>
#include <setjmp.h>

#define __arm__ 1
#define __aarch64__ 1

#if defined _WIN32

#include <windows.h>

static int g_sigill_caught = 0;
static jmp_buf g_jmpbuf;

static LONG CALLBACK catch_sigill(struct _EXCEPTION_POINTERS* ExceptionInfo)
{
    if (ExceptionInfo->ExceptionRecord->ExceptionCode == EXCEPTION_ILLEGAL_INSTRUCTION)
    {
        g_sigill_caught = 1;
        longjmp(g_jmpbuf, -1);
    }

    return EXCEPTION_CONTINUE_SEARCH;
}

static int detectisa(const void* some_inst)
{
    g_sigill_caught = 0;

    PVOID eh = AddVectoredExceptionHandler(1, catch_sigill);

    if (setjmp(g_jmpbuf) == 0)
    {
        ((void (*)())some_inst)();
    }

    RemoveVectoredExceptionHandler(eh);

    return g_sigill_caught ? 0 : 1;
}

#if defined(__i386__) || defined(__x86_64__) || defined(_M_IX86) || defined(_M_X64)
#ifdef _MSC_VER
#define DEFINE_INSTCODE(name, ...) __pragma(section(".text")) __declspec(allocate(".text")) static unsigned char name[] = { __VA_ARGS__, 0xc3 };
#else
#define DEFINE_INSTCODE(name, ...) __attribute__((section(".text"))) static unsigned char name[] = { __VA_ARGS__, 0xc3 };
#endif

#elif __aarch64__

#ifdef _MSC_VER
#define DEFINE_INSTCODE(name, ...) __pragma(section(".text")) __declspec(allocate(".text")) static unsigned int name[] = { __VA_ARGS__, 0xd65f03c0 };
#else
#define DEFINE_INSTCODE(name, ...) __attribute__((section(".text"))) static unsigned int name[] = { __VA_ARGS__, 0xd65f03c0 };
#endif

#elif __arm__

#ifdef _MSC_VER
#define DEFINE_INSTCODE(name, ...) __pragma(section(".text")) __declspec(allocate(".text")) static unsigned int name[] = { __VA_ARGS__, 0x4770 };
#else
#define DEFINE_INSTCODE(name, ...) __attribute__((section(".text"))) static unsigned int name[] = { __VA_ARGS__, 0x4770bf00 };
#endif

#endif


#else  // _WIN32
#include <signal.h>

static int g_sigill_caught = 0;
static sigjmp_buf g_jmpbuf;

static void catch_sigill(int signo, siginfo_t* si, void *data)
{
    g_sigill_caught = 1;
    siglongjmp(g_jmpbuf, -1);
}

static int detectisa(void (*some_inst)())
{
    g_sigill_caught = 0;

    struct sigaction sa = { 0 };
    struct sigaction old_sa;
    sa.sa_flags = SA_ONSTACK | SA_RESTART | SA_SIGINFO;
    sa.sa_sigaction = catch_sigill;
    sigaction(SIGILL, &sa, &old_sa);

    if (sigsetjmp(g_jmpbuf, 1) == 0)
    {
        some_inst();
    }

    sigaction(SIGILL, &old_sa, NULL);

    return g_sigill_caught ? 0 : 1;
}

#if defined(__i386__) || defined(__x86_64__) || defined(_M_IX86) || defined(_M_X64)
#define DEFINE_INSTCODE(name, ...) static void name() { asm volatile(".byte " #__VA_ARGS__ : : : ); };
#elif __aarch64__
#define DEFINE_INSTCODE(name, ...) static void name() { asm volatile(".word " #__VA_ARGS__ : : : ); };
#elif __arm__
#define DEFINE_INSTCODE(name, ...) static void name() { asm volatile(".word " #__VA_ARGS__ : : : ); };
#endif

#endif // _WIN32

#if defined(__i386__) || defined(__x86_64__) || defined(_M_IX86) || defined(_M_X64)
DEFINE_INSTCODE(some_mmx, 0x0f, 0xdb, 0xc0) // pand mm0,mm0
DEFINE_INSTCODE(some_sse, 0x0f, 0x54, 0xc0) // andps xmm0,xmm0
DEFINE_INSTCODE(some_sse2, 0x66, 0x0f, 0xfe, 0xc0) // paddd xmm0,xmm0
DEFINE_INSTCODE(some_sse3, 0xf2, 0x0f, 0x7c, 0xc0) // haddps xmm0,xmm0
DEFINE_INSTCODE(some_ssse3, 0x66, 0x0f, 0x38, 0x06, 0xc0) // phsubd xmm0,xmm0
DEFINE_INSTCODE(some_sse41, 0x66, 0x0f, 0x38, 0x3d, 0xc0) // pmaxsd xmm0,xmm0
DEFINE_INSTCODE(some_sse42, 0x66, 0x0f, 0x38, 0x37, 0xc0) // pcmpgtq xmm0,xmm0
DEFINE_INSTCODE(some_sse4a, 0x66, 0x0f, 0x79, 0xc0) // extrq xmm0,xmm0
DEFINE_INSTCODE(some_xop, 0x8f, 0xe8, 0x78, 0xb6, 0xc0, 0x00)  // vpmadcswd %xmm0,%xmm0,%xmm0,%xmm0
DEFINE_INSTCODE(some_avx, 0xc5, 0xfc, 0x54, 0xc0) // vandps ymm0,ymm0,ymm0
DEFINE_INSTCODE(some_f16c, 0xc4, 0xe2, 0x7d, 0x13, 0xc0) // vcvtph2ps ymm0,xmm0
DEFINE_INSTCODE(some_fma, 0xc4, 0xe2, 0x7d, 0x98, 0xc0) // vfmadd132ps ymm0,ymm0,ymm0
DEFINE_INSTCODE(some_avx2, 0xc5, 0xfd, 0xfe, 0xc0) // vpaddd ymm0,ymm0,ymm0
DEFINE_INSTCODE(some_avx512f, 0x62, 0xf1, 0x7c, 0x48, 0x58, 0xc0) // vaddps zmm0,zmm0,zmm0
DEFINE_INSTCODE(some_avx512bw, 0x62, 0xf1, 0x7d, 0x48, 0xfd, 0xc0) // vpaddw zmm0,zmm0,zmm0
DEFINE_INSTCODE(some_avx512cd, 0x62, 0xf2, 0xfd, 0x48, 0x44, 0xc0) // vplzcntq zmm0,zmm0
DEFINE_INSTCODE(some_avx512dq, 0x62, 0xf1, 0x7c, 0x48, 0x54, 0xc0) // vandps zmm0,zmm0,zmm0
DEFINE_INSTCODE(some_avx512vl, 0x62, 0xf2, 0xfd, 0x28, 0x1f, 0xc0) // vpabsq ymm0,ymm0
DEFINE_INSTCODE(some_avx512vnni, 0x62, 0xf2, 0x7d, 0x48, 0x52, 0xc0) // vpdpwssd %zmm0,%zmm0,%zmm0
DEFINE_INSTCODE(some_avx512bf16, 0x62, 0xf2, 0x7e, 0x48, 0x52, 0xc0) // vdpbf16ps %zmm0,%zmm0,%zmm0
DEFINE_INSTCODE(some_avx512ifma, 0x62, 0xf2, 0xfd, 0x48, 0xb4, 0xc0) // vpmadd52luq %zmm0,%zmm0,%zmm0
DEFINE_INSTCODE(some_avx512vbmi, 0x62, 0xf2, 0x7d, 0x48, 0x75, 0xc0) // vpermi2b %zmm0,%zmm0,%zmm0
DEFINE_INSTCODE(some_avx512vbmi2, 0x62, 0xf2, 0x7d, 0x48, 0x71, 0xc0) // vpshldvd %zmm0,%zmm0,%zmm0
DEFINE_INSTCODE(some_avx512fp16, 0x62, 0xf6, 0x7d, 0x48, 0x98, 0xc0) // vfmadd132ph %zmm0,%zmm0,%zmm0
DEFINE_INSTCODE(some_avxvnni, 0x62, 0xf2, 0x7d, 0x28, 0x52, 0xc0) // vpdpwssd ymm0,ymm0,ymm0
DEFINE_INSTCODE(some_avxvnniint8, 0xc4, 0xe2, 0x7f, 0x50, 0xc0) // vpdpbssd ymm0,ymm0,ymm0
DEFINE_INSTCODE(some_avxifma, 0x62, 0xf2, 0xfd, 0x28, 0xb4, 0xc0) // vpmadd52luq %ymm0,%ymm0,%ymm0

#elif __aarch64__
DEFINE_INSTCODE(some_neon, 0x4e20d400) // fadd v0.4s,v0.4s,v0.4s
DEFINE_INSTCODE(some_vfpv4, 0x0e216800) // fcvtn v0.4h,v0.4s
DEFINE_INSTCODE(some_cpuid, 0xd5380000) // mrs x0,midr_el1
DEFINE_INSTCODE(some_asimdhp, 0x0e401400) // fadd v0.4h,v0.4h,v0.4h
DEFINE_INSTCODE(some_asimddp, 0x4e809400) // sdot v0.4h,v0.16b,v0.16b
DEFINE_INSTCODE(some_asimdfhm, 0x4e20ec00) // fmlal v0.4s,v0.4h,v0.4h
DEFINE_INSTCODE(some_bf16, 0x6e40ec00) // bfmmla v0.4h,v0.8h,v0.8h
DEFINE_INSTCODE(some_i8mm, 0x4e80a400) // smmla v0.4h,v0.16b,v0.16b
DEFINE_INSTCODE(some_sve, 0x65608000) // fmad z0.h,p0/m,z0.h,z0.h
DEFINE_INSTCODE(some_sve2, 0x44405000) // smlslb z0.h,z0.b,z0.b
DEFINE_INSTCODE(some_svebf16, 0x6460e400) // bfmmla z0.s,z0.h,z0.h
DEFINE_INSTCODE(some_svei8mm, 0x45009800) // smmla z0.s,z0.b,z0.b
DEFINE_INSTCODE(some_svef32mm, 0x64a0e400) // fmmla z0.s,z0.s,z0.s

#elif __arm__
DEFINE_INSTCODE(some_edsp, 0x0000fb20) // smlad r0,r0,r0,r0
DEFINE_INSTCODE(some_neon, 0x0d40ef00) // vadd.f32 q0,q0,q0
DEFINE_INSTCODE(some_vfpv4, 0x0600ffb6) // vcvt.f16.f32 d0,q0

#endif

int main()
{
#if defined(__i386__) || defined(__x86_64__) || defined(_M_IX86) || defined(_M_X64)
    int has_mmx = detectisa(some_mmx);
    int has_sse = detectisa(some_sse);
    int has_sse2 = detectisa(some_sse2);
    int has_sse3 = detectisa(some_sse3);
    int has_ssse3 = detectisa(some_ssse3);
    int has_sse41 = detectisa(some_sse41);
    int has_sse42 = detectisa(some_sse42);
    int has_sse4a = detectisa(some_sse4a);
    int has_xop = detectisa(some_xop);
    int has_avx = detectisa(some_avx);
    int has_f16c = detectisa(some_f16c);
    int has_fma = detectisa(some_fma);
    int has_avx2 = detectisa(some_avx2);
    int has_avx512f = detectisa(some_avx512f);
    int has_avx512bw = detectisa(some_avx512bw);
    int has_avx512cd = detectisa(some_avx512cd);
    int has_avx512dq = detectisa(some_avx512dq);
    int has_avx512vl = detectisa(some_avx512vl);
    int has_avx512vnni = detectisa(some_avx512vnni);
    int has_avx512bf16 = detectisa(some_avx512bf16);
    int has_avx512ifma = detectisa(some_avx512ifma);
    int has_avx512vbmi = detectisa(some_avx512vbmi);
    int has_avx512vbmi2 = detectisa(some_avx512vbmi2);
    int has_avx512fp16 = detectisa(some_avx512fp16);
    int has_avxvnni = detectisa(some_avxvnni);
    int has_avxvnniint8 = detectisa(some_avxvnniint8);
    int has_avxifma = detectisa(some_avxifma);

    fprintf(stderr, "has_mmx = %d\n", has_mmx);
    fprintf(stderr, "has_sse = %d\n", has_sse);
    fprintf(stderr, "has_sse2 = %d\n", has_sse2);
    fprintf(stderr, "has_sse3 = %d\n", has_sse3);
    fprintf(stderr, "has_ssse3 = %d\n", has_ssse3);
    fprintf(stderr, "has_sse41 = %d\n", has_sse41);
    fprintf(stderr, "has_sse42 = %d\n", has_sse42);
    fprintf(stderr, "has_sse4a = %d\n", has_sse4a);
    fprintf(stderr, "has_xop = %d\n", has_xop);
    fprintf(stderr, "has_avx = %d\n", has_avx);
    fprintf(stderr, "has_f16c = %d\n", has_f16c);
    fprintf(stderr, "has_fma = %d\n", has_fma);
    fprintf(stderr, "has_avx2 = %d\n", has_avx2);
    fprintf(stderr, "has_avx512f = %d\n", has_avx512f);
    fprintf(stderr, "has_avx512bw = %d\n", has_avx512bw);
    fprintf(stderr, "has_avx512cd = %d\n", has_avx512cd);
    fprintf(stderr, "has_avx512dq = %d\n", has_avx512dq);
    fprintf(stderr, "has_avx512vl = %d\n", has_avx512vl);
    fprintf(stderr, "has_avx512vnni = %d\n", has_avx512vnni);
    fprintf(stderr, "has_avx512bf16 = %d\n", has_avx512bf16);
    fprintf(stderr, "has_avx512ifma = %d\n", has_avx512ifma);
    fprintf(stderr, "has_avx512vbmi = %d\n", has_avx512vbmi);
    fprintf(stderr, "has_avx512vbmi2 = %d\n", has_avx512vbmi2);
    fprintf(stderr, "has_avx512fp16 = %d\n", has_avx512fp16);
    fprintf(stderr, "has_avxvnni = %d\n", has_avxvnni);
    fprintf(stderr, "has_avxvnniint8 = %d\n", has_avxvnniint8);
    fprintf(stderr, "has_avxifma = %d\n", has_avxifma);
#elif __aarch64__
    int has_neon = detectisa(some_neon);
    int has_vfpv4 = detectisa(some_vfpv4);
    int has_cpuid = detectisa(some_cpuid);
    int has_asimdhp = detectisa(some_asimdhp);
    int has_asimddp = detectisa(some_asimddp);
    int has_asimdfhm = detectisa(some_asimdfhm);
    int has_bf16 = detectisa(some_bf16);
    int has_i8mm = detectisa(some_i8mm);
    int has_sve = detectisa(some_sve);
    int has_sve2 = detectisa(some_sve2);
    int has_svebf16 = detectisa(some_svebf16);
    int has_svei8mm = detectisa(some_svei8mm);
    int has_svef32mm = detectisa(some_svef32mm);

    fprintf(stderr, "has_neon = %d\n", has_neon);
    fprintf(stderr, "has_vfpv4 = %d\n", has_vfpv4);
    fprintf(stderr, "has_cpuid = %d\n", has_cpuid);
    fprintf(stderr, "has_asimdhp = %d\n", has_asimdhp);
    fprintf(stderr, "has_asimddp = %d\n", has_asimddp);
    fprintf(stderr, "has_asimdfhm = %d\n", has_asimdfhm);
    fprintf(stderr, "has_bf16 = %d\n", has_bf16);
    fprintf(stderr, "has_i8mm = %d\n", has_i8mm);
    fprintf(stderr, "has_sve = %d\n", has_sve);
    fprintf(stderr, "has_sve2 = %d\n", has_sve2);
    fprintf(stderr, "has_svebf16 = %d\n", has_svebf16);
    fprintf(stderr, "has_svei8mm = %d\n", has_svei8mm);
    fprintf(stderr, "has_svef32mm = %d\n", has_svef32mm);
#elif __arm__
    int has_edsp = detectisa(some_edsp);
    int has_neon = detectisa(some_neon);
    int has_vfpv4 = detectisa(some_vfpv4);

    fprintf(stderr, "has_edsp = %d\n", has_edsp);
    fprintf(stderr, "has_neon = %d\n", has_neon);
    fprintf(stderr, "has_vfpv4 = %d\n", has_vfpv4);
#endif

    getchar();

    return 0;
}
