// NOTE(Karan): 32bit version of this debugger can only debug 32bit applications. 32bit version can debug 32 as well as 64bit applications.

#include <sys/types.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/ptrace.h>
#include <sys/user.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <sys/resource.h>
#include <execinfo.h>
#include <fcntl.h>
#include <sys/stat.h> 
#include <stddef.h>

#include "custom_types.h"
#include "utility.h"
#include "string_operations.h"
// NOTE(Karan): Picked a random number for path length
#define MAX_BACKTRACE_STACK_FRAMES 25
#define MAX_FILEPATH_LENGTH 512
#define MAX_FILENAME_LENGTH 128


enum ProcessMode
{
    x86,
    x86_64
};


/* NOTE(Karan): 
32bit
u short int 2
s long int 4
u long long int 8
u int 4

64bit
u short int 2
s long int 8
u long long int 8
u int 4 */

struct GPRegistersx86_64
{
    u64 r15;
    u64 r14;
    u64 r13;
    u64 r12;
    u64 rbp;
    u64 rbx;
    u64 r11;
    u64 r10;
    u64 r9;
    u64 r8;
    u64 rax;
    u64 rcx;
    u64 rdx;
    u64 rsi;
    u64 rdi;
    u64 orig_rax;
    u64 rip;
    u64 cs;
    u64 eflags;
    u64 rsp;
    u64 ss;
    u64 fs_base;
    u64 gs_base;
    u64 ds;
    u64 es;
    u64 fs;
    u64 gs;
};

struct FPRegistersx86_64
{
    u16 cwd;
    u16 swd;
    u16 ftw;
    u16 fop;
    u64 fip;
    u64 rdp;
    u32 mxcsr;
    u32 mxcr_mask;
    u32 st_space[32];   /* 8*16 bytes for each FP-reg = 128 bytes */
    u32 xmm_space[64];  /* 16*16 bytes for each XMM-reg = 256 bytes */
    u32 padding[24];
};


struct Registersx86_64
{
    struct
    {
        union
        {
            GPRegistersx86_64 gpRegs;
            struct
            {
                u64 r15;
                u64 r14;
                u64 r13;
                u64 r12;
                u64 rbp;
                u64 rbx;
                u64 r11;
                u64 r10;
                u64 r9;
                u64 r8;
                u64 rax;
                u64 rcx;
                u64 rdx;
                u64 rsi;
                u64 rdi;
                u64 orig_rax;
                u64 rip;
                u64 cs;
                u64 eflags;
                u64 rsp;
                u64 ss;
                u64 fs_base;
                u64 gs_base;
                u64 ds;
                u64 es;
                u64 fs;
                u64 gs;
            };
        };
        union
        {
            FPRegistersx86_64 fpRegs;
            struct
            {
                u16 cwd;
                u16 swd;
                u16 ftw;
                u16 fop;
                u64 fip;
                u64 rdp;
                u32 mxcsr;
                u32 mxcr_mask;
                u32 st_space[32];   
                u32 xmm_space[64];  
                u32 padding[24];
            };
        };
    };
};


#ifdef __x86_64__
#define ARE_GPREGS_UNALIGNED (offset(GPRegistersx86_64, r15) - offset(user_regs_struct, r15) + \
offset(GPRegistersx86_64, r14) - offset(user_regs_struct, r14) + \
offset(GPRegistersx86_64, r13) - offset(user_regs_struct, r13) + \
offset(GPRegistersx86_64, r12) - offset(user_regs_struct, r12) + \
offset(GPRegistersx86_64, rbp) - offset(user_regs_struct, rbp) + \
offset(GPRegistersx86_64, rbx) - offset(user_regs_struct, rbx) + \
offset(GPRegistersx86_64, r11) - offset(user_regs_struct, r11) + \
offset(GPRegistersx86_64, r10) - offset(user_regs_struct, r10) + \
offset(GPRegistersx86_64, r9) - offset(user_regs_struct, r9) + \
offset(GPRegistersx86_64, r8) - offset(user_regs_struct, r8) + \
offset(GPRegistersx86_64, rax) - offset(user_regs_struct, rax) + \
offset(GPRegistersx86_64, rcx) - offset(user_regs_struct, rcx) + \
offset(GPRegistersx86_64, rdx) - offset(user_regs_struct, rdx) + \
offset(GPRegistersx86_64, rsi) - offset(user_regs_struct, rsi) + \
offset(GPRegistersx86_64, rdi) - offset(user_regs_struct, rdi) + \
offset(GPRegistersx86_64, orig_rax) - offset(user_regs_struct, orig_rax) + \
offset(GPRegistersx86_64, rip) - offset(user_regs_struct, rip) + \
offset(GPRegistersx86_64, cs) - offset(user_regs_struct, cs) + \
offset(GPRegistersx86_64, eflags) - offset(user_regs_struct, eflags) + \
offset(GPRegistersx86_64, rsp) - offset(user_regs_struct, rsp) + \
offset(GPRegistersx86_64, ss) - offset(user_regs_struct, ss) + \
offset(GPRegistersx86_64, fs_base) - offset(user_regs_struct, fs_base) + \
offset(GPRegistersx86_64, gs_base) - offset(user_regs_struct, gs_base) + \
offset(GPRegistersx86_64, ds) - offset(user_regs_struct, ds) + \
offset(GPRegistersx86_64, es) - offset(user_regs_struct, es) + \
offset(GPRegistersx86_64, fs) - offset(user_regs_struct, fs) + \
offset(GPRegistersx86_64, gs) - offset(user_regs_struct, gs))
#define ARE_GPREGS_ALIGNED (!ARE_GPREGS_UNALIGNED)
#undef ARE_GPREGS_UNALIGNED

#define ARE_FPREGS_UNALIGNED (offset(FPRegistersx86_64, cwd) - offset(user_fpregs_struct, cwd) + \
offset(FPRegistersx86_64, swd) - offset(user_fpregs_struct, swd) + \
offset(FPRegistersx86_64, ftw) - offset(user_fpregs_struct, ftw) + \
offset(FPRegistersx86_64, fop) - offset(user_fpregs_struct, fop) + \
offset(FPRegistersx86_64, fip) - offset(user_fpregs_struct, rip) + \
offset(FPRegistersx86_64, rdp) - offset(user_fpregs_struct, rdp) + \
offset(FPRegistersx86_64, mxcsr) - offset(user_fpregs_struct, mxcsr) + \
offset(FPRegistersx86_64, mxcr_mask) - offset(user_fpregs_struct, mxcr_mask) + \
offset(FPRegistersx86_64, st_space) - offset(user_fpregs_struct, st_space) + \
offset(FPRegistersx86_64, xmm_space) - offset(user_fpregs_struct, xmm_space) + \
offset(FPRegistersx86_64, padding) - offset(user_fpregs_struct, padding))
#define ARE_FPREGS_ALIGNED (!ARE_FPREGS_UNALIGNED)
#undef ARE_FPREGS_UNALIGNED


#if !ARE_GPREGS_ALIGNED
#pragma message("struct GPRegisters does not have the same alignment as struct user_regs_struct")
#endif

#if !ARE_GPREGS_ALIGNED
#pragma message("struct FPRegisters does not have the same alignment as struct user_fpregs_struct")
#endif

inline internal void CopyGPRegistersx86_64ToUserRegsx86_64(GPRegistersx86_64 *source, struct user_regs_struct *dest)
{
#if ARE_GPREGS_ALIGNED
    *dest = *((user_regs_struct*)source);
#else
    dest->r15 = source->r15;
    dest->r14 = source->r14;
    dest->r13 = source->r13;
    dest->r12 = source->r12;
    dest->rbp = source->rbp;
    dest->rbx = source->rbx;
    dest->r11 = source->r11;
    dest->r10 = source->r10;
    dest->r9 = source->r9;
    dest->r8 = source->r8;
    dest->rax = source->rax;
    dest->rcx = source->rcx;
    dest->rdx = source->rdx;
    dest->rsi = source->rsi;
    dest->rdi = source->rdi;
    dest->orig_rax = source->orig_rax;
    dest->rip = source->rip;
    dest->cs = source->cs;
    dest->eflags = source->eflags;
    dest->rsp = source->rsp;
    dest->ss = source->ss;
    dest->fs_base = source->fs_base;
    dest->gs_base = source->gs_base;
    dest->ds = source->ds;
    dest->es = source->es;
    dest->fs = source->fs;
    dest->gs = source->gs;
#endif
}

inline internal void CopyUserRegsx86_64ToGPRegistersx86_64(struct user_regs_struct *source, GPRegistersx86_64 *dest)
{
#if ARE_GPREGS_ALIGNED
    *dest = *((GPRegistersx86_64*)source);
#else
    dest->r15 = source->r15;
    dest->r14 = source->r14;
    dest->r13 = source->r13;
    dest->r12 = source->r12;
    dest->rbp = source->rbp;
    dest->rbx = source->rbx;
    dest->r11 = source->r11;
    dest->r10 = source->r10;
    dest->r9 = source->r9;
    dest->r8 = source->r8;
    dest->rax = source->rax;
    dest->rcx = source->rcx;
    dest->rdx = source->rdx;
    dest->rsi = source->rsi;
    dest->rdi = source->rdi;
    dest->orig_rax = source->orig_rax;
    dest->rip = source->rip;
    dest->cs = source->cs;
    dest->eflags = source->eflags;
    dest->rsp = source->rsp;
    dest->ss = source->ss;
    dest->fs_base = source->fs_base;
    dest->gs_base = source->gs_base;
    dest->ds = source->ds;
    dest->es = source->es;
    dest->fs = source->fs;
    dest->gs = source->gs;
#endif
}

inline internal void CopyUserFPRegsx86_64ToFPRegistersx86_64(struct user_fpregs_struct *source, FPRegistersx86_64 *dest)
{
#if ARE_FPREGS_ALIGNED
    *dest = *((FPRegistersx86_64*)source);
#else
    dest->cwd = source->cwd;
    dest->swd = source->swd;
    dest->ftw = source->ftw;
    dest->fop = source->fop;
    dest->fip = source->rip;
    dest->rdp = source->rdp;
    dest->mxcsr = source->mxcsr;
    dest->mxcr_mask = source->mxcr_mask;
    ArrayCopy(source->st_space, dest->st_space, sizeof(dest->st_space));    
    ArrayCopy(source->xmm_space, dest->xmm_space, sizeof(dest->xmm_space));
#endif
}


inline internal void CopyFPRegistersx86_64ToUserFPRegsx86_64(FPRegistersx86_64 *source, struct user_fpregs_struct *dest)
{
#if ARE_FPREGS_ALIGNED
    *dest = *((struct user_fpregs_struct*)source);
#else
    dest->cwd = source->cwd;
    dest->swd = source->swd;
    dest->ftw = source->ftw;
    dest->fop = source->fop;
    dest->rip = source->fip;
    dest->rdp = source->rdp;
    dest->mxcsr = source->mxcsr;
    dest->mxcr_mask = source->mxcr_mask;
    ArrayCopy(source->st_space, dest->st_space, sizeof(dest->st_space));
    ArrayCopy(source->xmm_space, dest->xmm_space, sizeof(dest->xmm_space));
#endif
}
#endif

struct RegisterLocationSizeInfo
{
    u32 offset;
    u32 size;
};

RegisterLocationSizeInfo GetRegisterLocationSizeInfo(char *name, u32 length)
{
    
    // TODO(Karan): This is ultra piggy...Think of how to handle and search strings
    
    RegisterLocationSizeInfo result = {};
    if(length == 2)
    {
        if(IsSubstring((char*)"r8", name)){result.offset = (offsetof(GPRegistersx86_64 , r15)); result.size = 8;}
        else if(IsSubstring((char*)"r9", name)) {result.offset = offsetof(Registersx86_64, r9); result.size = 8;} 
        else if(IsSubstring((char*)"cs", name)) {result.offset = offsetof(Registersx86_64, cs); result.size = 8;} 
        else if(IsSubstring((char*)"ss", name)) {result.offset = offsetof(Registersx86_64, ss); result.size = 8;} 
        else if(IsSubstring((char*)"ds", name)) {result.offset = offsetof(Registersx86_64, ds); result.size = 8;} 
        else if(IsSubstring((char*)"es", name)) {result.offset = offsetof(Registersx86_64, es); result.size = 8;} 
        else if(IsSubstring((char*)"fs", name)) {result.offset = offsetof(Registersx86_64, fs); result.size = 8;} 
        else if(IsSubstring((char*)"gs", name)) {result.offset = offsetof(Registersx86_64, gs); result.size = 8;} 
    }
    else if(length == 3)
    {
        if(IsSubstring((char*)"st", name)) 
        {
            result.offset = offsetof(Registersx86_64, st_space); 
            result.size = 10;
            u64 registerNumber = InterpretDecimalIntegerAs64BitData(name + 2, 1);
            result.offset += (u32)(registerNumber * 4 * sizeof(u32));
        } 
        else if(IsSubstring((char*)"r10", name)) {result.offset = offsetof(Registersx86_64, r10); result.size = 8;} 
        else if(IsSubstring((char*)"r11", name)) {result.offset = offsetof(Registersx86_64, r11); result.size = 8;} 
        else if(IsSubstring((char*)"r12", name)) {result.offset = offsetof(Registersx86_64, r12); result.size = 8;} 
        else if(IsSubstring((char*)"r13", name)) {result.offset = offsetof(Registersx86_64, r13); result.size = 8;} 
        else if(IsSubstring((char*)"r14", name)) {result.offset = offsetof(Registersx86_64, r14); result.size = 8;} 
        else if(IsSubstring((char*)"r15", name)) {result.offset = offsetof(Registersx86_64, r15); result.size = 8;} 
        else if(IsSubstring((char*)"rbp", name)) {result.offset = offsetof(Registersx86_64, rbp); result.size = 8;} 
        else if(IsSubstring((char*)"rbx", name)) {result.offset = offsetof(Registersx86_64, rbx); result.size = 8;} 
        else if(IsSubstring((char*)"rax", name)) {result.offset = offsetof(Registersx86_64, rax); result.size = 8;} 
        else if(IsSubstring((char*)"rcx", name)) {result.offset = offsetof(Registersx86_64, rcx); result.size = 8;} 
        else if(IsSubstring((char*)"rdx", name)) {result.offset = offsetof(Registersx86_64, rdx); result.size = 8;} 
        else if(IsSubstring((char*)"rsi", name)) {result.offset = offsetof(Registersx86_64, rsi); result.size = 8;} 
        else if(IsSubstring((char*)"rdi", name)) {result.offset = offsetof(Registersx86_64, rdi); result.size = 8;} 
        else if(IsSubstring((char*)"rip", name)) {result.offset = offsetof(Registersx86_64, rip); result.size = 8;} 
        else if(IsSubstring((char*)"rsp", name)) {result.offset = offsetof(Registersx86_64, rsp); result.size = 8;} 
        else if(IsSubstring((char*)"cwd", name)) {result.offset = offsetof(Registersx86_64, cwd); result.size = 8;} 
        else if(IsSubstring((char*)"swd", name)) {result.offset = offsetof(Registersx86_64, swd); result.size = 8;} 
        else if(IsSubstring((char*)"ftw", name)) {result.offset = offsetof(Registersx86_64, ftw); result.size = 8;} 
        else if(IsSubstring((char*)"fop", name)) {result.offset = offsetof(Registersx86_64, fop); result.size = 8;} 
        else if(IsSubstring((char*)"fip", name)) {result.offset = offsetof(Registersx86_64, fip); result.size = 8;} 
        else if(IsSubstring((char*)"rdp", name)) {result.offset = offsetof(Registersx86_64, rdp); result.size = 8;} 
    }
    else if(length == 4 || length == 5)
    {
        if(IsSubstring((char*)"xmm", name)) 
        {
            result.offset = offsetof(Registersx86_64, xmm_space); 
            result.size = 4 * sizeof(u32);
            u64 registerNumber = InterpretDecimalIntegerAs64BitData(name + 3, length - 3);
            result.offset += (u32)(registerNumber * 4 * sizeof(u32));
        }
        else if(IsSubstring((char*)"flags", name)) {result.offset = offsetof(Registersx86_64, eflags); result.size = 8;} 
        else if(IsSubstring((char*)"mxcsr", name)) {result.offset = offsetof(Registersx86_64, mxcsr); result.size = 4;} 
    }
    else if(length == 6)
    {
        if(IsSubstring((char*)"orgrax", name)) {result.offset = offsetof(Registersx86_64, orig_rax); result.size = 8;} 
        if(IsSubstring((char*)"fsbase", name)) {result.offset = offsetof(Registersx86_64, fs_base); result.size = 8;} 
        if(IsSubstring((char*)"gsbase", name)) {result.offset = offsetof(Registersx86_64, gs_base); result.size = 8;} 
    }
    else if(length == 8)
    {
        if(IsSubstring((char*)"mxcrmask", name)) {result.offset = offsetof(Registersx86_64, mxcr_mask); result.size = 4;} 
    }
    
    
    return result;
}

internal s64 PtracePeekText(pid_t pid, u64 address, void *data, u32 bytes);
internal s64 PtracePokeText(pid_t pid, u64 address, void *data, u32 bytes)
{
    u32 longSize = sizeof(long);
    u32 numLongsInData = bytes/longSize;
    u32 leftOverBytes = bytes - (numLongsInData * longSize);
    
    for(u32 i = 0; i < numLongsInData; i++)
    {
        long* currentLong = (long*)(((char*)data) + (i * longSize));
        u64 currentAddress64 = address + (i*longSize);
#ifdef __x86_64__
        u64 currentAddress = currentAddress64;
#else
        u32 currentAddress = (u32)LOW_32_FROM_64(currentAddress64);
#endif
        long returnValue = ptrace(PTRACE_POKETEXT, pid, currentAddress, *currentLong);
        if(returnValue == -1)
        {
            return -1;
        }
    }
    
    if(leftOverBytes > 0)
    {
        u64 currentAddress64 = address + (numLongsInData*longSize);
#ifdef __x86_64__
        u64 currentAddress = currentAddress64;
#else
        u32 currentAddress = (u32)LOW_32_FROM_64(currentAddress64);
#endif
        u64 dataAtCurrentAddress = 0;
        
        if(PtracePeekText(pid, currentAddress, &dataAtCurrentAddress, sizeof(dataAtCurrentAddress)) != -1)
        {
            char* currentData = (((char*)data) + (numLongsInData * longSize));
            ArrayCopy(currentData, &dataAtCurrentAddress, leftOverBytes);
            long returnValue = ptrace(PTRACE_POKETEXT, pid, currentAddress, dataAtCurrentAddress);
            if(returnValue == -1)
            {
                return -1;
            }
        }
        else
        {
            return -1;
        }
        
    }
    
    return 0;
}

internal s64 PtracePeekText(pid_t pid, u64 address, void *data, u32 bytes)
{
    u64 currentAddress = address;
    char* currentData = (char*)data;
    char* onePastEnd = currentData + bytes;
    while(currentData != onePastEnd)
    {
        // TODO(Karan): How is this working?? I had to convert it to 32 for PokeText to work...Investigate this!!
        errno = 0;
        long dataAtCurrentAddress = ptrace(PTRACE_PEEKTEXT, pid, currentAddress, 0);
        if(errno == 0)
        {
            u32 bytesRemaining = (u32)(onePastEnd - currentData);
            u32 copyCounter = (u32)MIN(bytesRemaining, sizeof(long));
            if(copyCounter == sizeof(long))
            {
                *((long*)currentData) = dataAtCurrentAddress;
                currentData += copyCounter;
                currentAddress += copyCounter;
            }
            else
            {
                char *currentDataAtCurrentAddress = (char*)(&dataAtCurrentAddress);
                while(copyCounter--)
                {
                    *currentData++ = *currentDataAtCurrentAddress++;
                    currentAddress++;
                }
            }
        }
        else
        {
            return -1;
        }
    }
    
    return 0;
}

internal s64 PtraceGetGPRegisters(pid_t pid, Registersx86_64 *registers)
{
    struct user_regs_struct regs;
    if(ptrace(PTRACE_GETREGS, pid, &regs, &regs) != -1)
    {
#ifndef __x86_64__
        registers->rax = (u64)(regs.eax) & 0x00000000ffffffff;
        registers->rbx = (u64)(regs.ebx) & 0x00000000ffffffff;
        registers->rcx = (u64)(regs.ecx) & 0x00000000ffffffff;
        registers->rdx = (u64)(regs.edx) & 0x00000000ffffffff;
        registers->rbp = (u64)(regs.ebp) & 0x00000000ffffffff;
        registers->rsp = (u64)(regs.esp) & 0x00000000ffffffff;
        registers->rsi = (u64)(regs.esi) & 0x00000000ffffffff;
        registers->rdi = (u64)(regs.edi) & 0x00000000ffffffff;
        
        registers->ss = (u64)(regs.xss) & 0x00000000ffffffff;
        registers->cs = (u64)(regs.xcs) & 0x00000000ffffffff;
        registers->ds = (u64)(regs.xds) & 0x00000000ffffffff;
        registers->es = (u64)(regs.xes) & 0x00000000ffffffff;
        registers->fs = (u64)(regs.xfs) & 0x00000000ffffffff;
        registers->gs = (u64)(regs.xgs) & 0x00000000ffffffff;
        
        registers->eflags = (u64)(regs.eflags) & 0x00000000ffffffff;
        registers->orig_rax = (u64)(regs.orig_eax) & 0x00000000ffffffff;
        registers->rip  = (u64)(regs.eip) & 0x00000000ffffffff;
#else 
        CopyUserRegsx86_64ToGPRegistersx86_64(&regs, &(registers->gpRegs));
#endif
    }
    else
    {
        return -1;
    }
    
    return 0;
}

internal s64 PtraceGetFPRegisters(pid_t pid, Registersx86_64 *registers)
{
#ifndef __x86_64__
    struct user_fpxregs_struct fpRegs;
    if(ptrace(PTRACE_GETFPXREGS, pid, &fpRegs, &fpRegs) != -1)
    {
        registers->cwd  = fpRegs.cwd;
        registers->swd  = fpRegs.swd;
        registers->ftw  = fpRegs.twd;
        registers->fop  = fpRegs.fop;
        registers->fip  = ((u64)fpRegs.fcs << 32) | ((u64)fpRegs.fip & 0x00000000ffffffff);
        registers->rdp  = ((u64)fpRegs.fos << 32) | ((u64)fpRegs.foo & 0x00000000ffffffff);
        registers->mxcsr  = (u32)fpRegs.mxcsr;
        registers->mxcr_mask  = (u32)fpRegs.reserved;
        ArrayCopy(fpRegs.st_space, registers->st_space, sizeof(fpRegs.st_space));
        ArrayCopy(fpRegs.xmm_space, registers->xmm_space, sizeof(fpRegs.xmm_space));
    }
    else
    {
        return -1;
    }
#else 
    struct user_fpregs_struct fpRegs;
    if(ptrace(PTRACE_GETFPREGS, pid, &fpRegs, &fpRegs) != -1)
    {
        CopyUserFPRegsx86_64ToFPRegistersx86_64(&fpRegs, &(registers->fpRegs));
    }
    else
    {
        return -1;
    }
#endif
    return 0;
}

internal s64 PtraceSetGPRegisters(pid_t pid, Registersx86_64 *registers)
{
    struct user_regs_struct regs = {};
#ifndef __x86_64__
    regs.eax = (s32)LOW_32_FROM_64(registers->rax) ;
    regs.ebx = (s32)LOW_32_FROM_64(registers->rbx) ;
    regs.ecx = (s32)LOW_32_FROM_64(registers->rcx) ;
    regs.edx = (s32)LOW_32_FROM_64(registers->rdx) ;
    regs.ebp = (s32)LOW_32_FROM_64(registers->rbp) ;
    regs.esp = (s32)LOW_32_FROM_64(registers->rsp) ;
    regs.esi = (s32)LOW_32_FROM_64(registers->rsi) ;
    regs.edi = (s32)LOW_32_FROM_64(registers->rdi) ;
    
    regs.xss = (s32)LOW_32_FROM_64(registers->ss) ;
    regs.xcs = (s32)LOW_32_FROM_64(registers->cs) ;
    regs.xds = (s32)LOW_32_FROM_64(registers->ds) ;
    regs.xes = (s32)LOW_32_FROM_64(registers->es) ;
    regs.xfs = (s32)LOW_32_FROM_64(registers->fs) ;
    regs.xgs = (s32)LOW_32_FROM_64(registers->gs) ;
    
    regs.eflags = (s32)LOW_32_FROM_64(registers->eflags) ;
    regs.orig_eax = (s32)LOW_32_FROM_64(registers->orig_rax) ;
    regs.eip  = (s32)LOW_32_FROM_64(registers->rip) ;
#else 
    CopyGPRegistersx86_64ToUserRegsx86_64(&(registers->gpRegs), &regs);
#endif
    
    return ptrace(PTRACE_SETREGS, pid, &regs, &regs);
}


internal s64 PtraceSetFPRegisters(pid_t pid, Registersx86_64 *registers)
{
#ifndef __x86_64__
    struct user_fpxregs_struct fpRegs = {};
    fpRegs.cwd  = registers->cwd;
    fpRegs.swd  = registers->swd;
    fpRegs.twd  = registers->ftw;
    fpRegs.fop  = registers->fop;
    fpRegs.fip  = (s32)LOW_32_FROM_64(registers->fip);
    fpRegs.fcs  = (s32)LOW_32_FROM_64((registers->fip >> 32));
    fpRegs.foo  = (s32)LOW_32_FROM_64(registers->rdp);
    fpRegs.fos  = (s32)LOW_32_FROM_64((registers->rdp >> 32));
    fpRegs.mxcsr  = (s32)registers->mxcsr;
    fpRegs.reserved  = (s32)registers->mxcr_mask;
    ArrayCopy(registers->st_space, fpRegs.st_space, sizeof(fpRegs.st_space));
    ArrayCopy(registers->xmm_space, fpRegs.xmm_space, sizeof(fpRegs.xmm_space));
    return ptrace(PTRACE_SETFPXREGS, pid, &fpRegs, &fpRegs);
#else 
    struct user_fpregs_struct fpRegs = {};
    CopyFPRegistersx86_64ToUserFPRegsx86_64(&(registers->fpRegs), &fpRegs);
    return ptrace(PTRACE_SETFPREGS, pid, &fpRegs, &fpRegs);
#endif
}

struct Breakpoint
{
    u64 address;
    s64 instructionData;
    bool isTemporary;
};

internal Breakpoint* FindBreakpointSetAt(u64 address, Breakpoint *breakpoints, u32 length)
{
    Breakpoint* result = 0;
    for(u32 i = 0; i < length; i++)
    {
        if(breakpoints[i].address == address)
        {
            result = breakpoints + i;
            break;
        }
    }
    return result;
}

#if 0
internal void RemoveBreakpointSetAt(u64 address, Breakpoint *breakpoints, u32 *length)
{
    for(u32 i = 0; i < *length; i++)
    {
        if(breakpoints[i].address == address)
        {
            if(i != (*length - 1))
            {
                Breakpoint *last = breakpoints + (*length - 1);
                breakpoints[i] = *last;
            }
            *length = *length - 1;
            break;
        }
    }
}
#endif
struct PlatformState
{
    char cwd[MAX_FILEPATH_LENGTH];
    char debugReportsDir[MAX_FILEPATH_LENGTH];
    char backtraceFilepath[MAX_FILEPATH_LENGTH];
    char coreFilepath[MAX_FILEPATH_LENGTH];
    char coreFilepathSpec[MAX_FILEPATH_LENGTH];
};
global_variable PlatformState globalPlatformState = {};

#if 0
/* TODO(Karan): Think of how to handle errors.
Generate a log report for tracking bugs.
Identify states and critical information that must not be lost on crash and make them crash-safe.
*/
#define CORE_DUMP sprintf(globalPlatformState.coreFilepath, globalPlatformState.coreFilepathSpec, __func__, __LINE__); CoreDump(globalPlatformState.coreFilepath);
internal void CoreDump(char* path)
{
    // TODO(Karan): This is hacky, couldn't find a reliable way to generate a core dump file
    struct rlimit coreLimits;
    coreLimits.rlim_cur = coreLimits.rlim_max = RLIM_INFINITY;
    if(setrlimit(RLIMIT_CORE, &coreLimits) == -1)
    {
        ERR_WITH_LOCATION_INFO("%s", strerror(errno));
    }
    
    pid_t myPID = getpid();
    char myPIDString[42];
    snprintf(myPIDString, ARRAY_COUNT(myPIDString), "%" PRId64, (s64)myPID);
    char *arguments[] = {(char*)"gcore", (char*)"-o", path, myPIDString, (char*)0};
    pid_t childPID = fork();
    if(!childPID)
    {
        if(execvp("gcore", arguments) == -1)
        {
            ERR_WITH_LOCATION_INFO("%s", strerror(errno));
        }
    }
    else
    {
        waitpid(childPID, 0, 0);
    }
}

#define BACKTRACE(b) Backtrace((char*)(globalPlatformState.backtraceFilepath), __LINE__, __func__, __FILE__, (char*)(b))
internal void Backtrace(char *filepath, u32 line, const char *function, const char *file, char* title)
{
    void *stackFrameReturnAddresses[MAX_BACKTRACE_STACK_FRAMES];
    s32 backtraceFrames = backtrace((void**)&stackFrameReturnAddresses, MAX_BACKTRACE_STACK_FRAMES);
    
    s32 fd = open(filepath, O_RDWR | O_CREAT);
    if(fd != -1)
    {
        if(lseek(fd, 0, SEEK_END) != -1)
        {
            char separator[] = "=============================================";
            dprintf(fd, "%s\n@ %s: %s(%d)\nERRNO:%d (%s)\n", separator, function, file, line, errno, strerror(errno));
            if(title)
            {
                dprintf(fd, "%s\n", title);
            }
            backtrace_symbols_fd(stackFrameReturnAddresses, backtraceFrames, fd);
            dprintf(fd, "\n");
            close(fd);
        }
        else
        {
            ERR_WITH_LOCATION_INFO("ERRNO:%d Failed to seek the end of backtrace file: %s. %s", errno, filepath, strerror(errno));
        }
    }
    else
    {
        ERR_WITH_LOCATION_INFO("ERRNO:%d Failed to open the backtrace file: %s. %s", errno, filepath, strerror(errno));
    }
}
#endif

int main(int argc, char **argv)
{
    {
        // NOTE(Karan): Initialization for storing stack trace and core dump reports
        
        getcwd(globalPlatformState.cwd, ARRAY_COUNT(globalPlatformState.cwd));
        ConcateToPath((char*)globalPlatformState.cwd, (char*)"debug_reports", (char*)globalPlatformState.debugReportsDir);
        ConcateToPath((char*)globalPlatformState.debugReportsDir, (char*)"backtrace.txt", (char*)globalPlatformState.backtraceFilepath);
        ConcateToPath((char*)globalPlatformState.debugReportsDir, (char*)"core_%s_%d", (char*)globalPlatformState.coreFilepathSpec);
        struct stat stat_buf = {};
        stat(globalPlatformState.backtraceFilepath, &stat_buf);
        if(stat_buf.st_size >= GIGABYTES(1))
        {
            char newFilename[MAX_FILENAME_LENGTH] = {};
            // NOTE(Karan): Attaching pid to create a random name
            sprintf(newFilename, "backtrace_%d.txt", getpid());
            char newBacktraceFilepath[MAX_FILEPATH_LENGTH] = {};
            ConcateToPath((char*)globalPlatformState.debugReportsDir, (char*)newFilename, (char*)newBacktraceFilepath);
            rename(globalPlatformState.backtraceFilepath, newBacktraceFilepath);
        }
    }
    
    pid_t debuggeeProgramID = 0;
    char* programPath;
    char* workingDirectoryPath;
    char **arguments;
    
    if(argc < 2)
    {
        OUT("Missing arguments\nUsage: debugger <path_to_executable> <working_directory> [<arguments>]\nOR\nUsage: debugger <pid>\n");
        return -1;
    }
    
    if(argc == 2)
    {
        // TODO(Karan): pid_t is signed int, make better string to number type functions to handle signedness and different data types
        u64 temp = InterpretDecimalIntegerAs64BitData(argv[1]);
        ArrayCopy(&temp, &debuggeeProgramID, sizeof(debuggeeProgramID)) ;
    }
    
    if(argc == 3)
    {
        programPath = argv[1];
        workingDirectoryPath = argv[2];
        arguments = 0;
    }
    
    if(argc > 3)
    {
        arguments = argv + 3;
    }
    
    if(argc > 2)
    {
        debuggeeProgramID = fork();
        if(debuggeeProgramID == 0)
        {
            if(chdir(workingDirectoryPath) != -1)
            {
                if(ptrace(PTRACE_TRACEME, 0, 0, 0) != -1)
                {
                    if(execl(programPath, programPath, arguments, (char*)0) != -1)
                    {
                        
                    }
                    else
                    {
                        ERR_WITH_LOCATION_INFO("ERRNO:%d Failed to launch debuggee. %s", errno, strerror(errno));
                        return errno;
                    }
                }
                else
                {
                    ERR_WITH_LOCATION_INFO("ERRNO:%d Failed to attach debugger to debuggee. %s", errno, strerror(errno));
                    return errno;
                }
            }
            else
            {
                ERR_WITH_LOCATION_INFO("ERRNO:%d Failed to change to debuggee's working directory. %s", errno, strerror(errno));
                return errno;
            }
        }
    }
    
    if(debuggeeProgramID >= 1)
    {
        bool debugging = true;
        if(argc == 2)
        {
            if(ptrace(PTRACE_ATTACH, debuggeeProgramID, 0, 0) == -1)
            {
                debugging = false;
                ERR_WITH_LOCATION_INFO("ERRNO:%d Failed to attach to pid %d. %s", errno, debuggeeProgramID, strerror(errno));
            }
        }
        
        Breakpoint breakpoints[16] = {};
        u32 breakpointCount = 0;
        OUT("> ls to list all commands\n");
        while(debugging)
        {
            s32 status = 0;
            pid_t programID = waitpid(-1, &status, WNOHANG | WUNTRACED | WCONTINUED); 
            if(programID > 0)
            {
                ProcessMode mode = x86_64;
                char procFile[32] = {};
                sprintf(procFile, "/proc/%d/exe", programID);  
                s32 fd = open(procFile, O_RDONLY);
                lseek(fd, 4, SEEK_SET);
                char c = 0;
                read(fd, &c, 1);
                if(c == 1)
                {
                    mode = x86;
                }
                
                if(WIFSTOPPED(status))
                {
                    /* NOTE(Karan): Ideas for decoupling platform layer and debugger logic:
                    On receiving trap signal, call into independent layer which would handle accepting 
                    commands from the user and then after resolving that interaction the independent layer
                    can send back the desired action like "CONTINUE" back to platform layer which would then 
                    execute platform specific command like ptrace(PTRACE_CONT, ..)
                    */
                    siginfo_t signalInfo;
                    ptrace(PTRACE_GETSIGINFO, programID, 0, &signalInfo);
                    psiginfo(&signalInfo, 0);
                    
                    Breakpoint *hitBreakpoint = 0;
                    Registersx86_64 registers = {};
                    if(PtraceGetGPRegisters(programID, &registers) != -1)
                    {
                        hitBreakpoint = FindBreakpointSetAt(registers.rip - 1, breakpoints, ARRAY_COUNT(breakpoints));
                    }
                    else
                    {
                        ERR_WITH_LOCATION_INFO("ERRNO:%d Failed to read instruction pointer. %s", errno, strerror(errno));
                    }
                    PtraceGetFPRegisters(programID, &registers);
                    
                    s64 instructionDataAtInstructionPointer = 0;
                    PtracePeekText(programID, registers.rip, &instructionDataAtInstructionPointer, sizeof(instructionDataAtInstructionPointer));
                    OUT("RIP: %"PRIx64": %"PRIx64"\n", registers.rip, instructionDataAtInstructionPointer);
                    
                    bool commandContinue = false;
                    while(!commandContinue)
                    {
                        char commandStringBuffer[512] = {};
                        char *commandString = (char*)commandStringBuffer;
                        OUT("> ");
                        
                        commandString = fgets(commandString, ARRAY_COUNT(commandStringBuffer), stdin);
                        u32 commandStringLength = GetStringLength(commandString);
                        ASSERT(commandString[commandStringLength - 1] == '\n');
                        commandString[commandStringLength - 1] = 0;
                        commandStringLength--;
                        if(commandString)
                        {
                            /* TODO(Karan): Currently we issue ptrace commands and waitpid for the debuggee to transition into "stop state" at the head of while(debugging)
                            loop. Consider we are at a breakpoint and the user issues "step over" command. In order to "step over" breakpoint, we restore the original 
                            instruction data at that address, single step over it, waitpid on it and then restore int3 at that instruction. Since we "waitpid"ed over it,
                            the waitpid at the head of while(debugging) won't catch this transition and we are left in hang state. To resolve this we can:
                            
                            1. Not waitpid for inserting int3, mark it dirty and restore it on next stop 
                            2. Look for a another function like waitpid that returns appropriate status when debuggee is _in_ stop state instead of returning only  
    when the debuggee _transitions_ into stop state.
                            3. After we step over breakpoint, if the command is "step over" set a temporary breakpoint at current address and issue "continue".
                            4. Send some kind of trapping/stopping signal to the debuggee right after we have done all the shenanigans with breakpoint restoration.
                            
                            Solution: Went with option 2. waitpid can wait for process that have transitioned into waitable state _AND_ keep them waitable for future waitpid call.
                            */
                            bool isContinue = AreStringsSame(commandString, (char*)"continue");
                            bool isStepIn = AreStringsSame(commandString, (char*)"step in");
                            bool isRegisters = AreStringsSame(commandString, (char*)"registers");
                            bool isGP = AreStringsSame(commandString, (char*)"gp");
                            bool isX87 = AreStringsSame(commandString, (char*)"x87");
                            bool isXMM = AreStringsSame(commandString, (char*)"xmm");
                            s64 spaceIndex = GetFirstIndex(commandString, ' ');
                            u32 terminator = commandStringLength;
                            if(spaceIndex != -1) terminator = (u32)spaceIndex;
                            RegisterLocationSizeInfo registerInfo = GetRegisterLocationSizeInfo(commandString, terminator);
                            
                            if(AreStringsSame(commandString, (char*)"ls"))
                            {
                                OUT("continue\n");
                                OUT("step in\n");
                                OUT("registers\n");
                                OUT("gp\n");
                                OUT("x87\n");
                                OUT("xmm\n");
                                OUT("<register_name> [<value>]\n");
                                OUT("read <start_address> <end_address> <block_size> <columns>\n");
                                OUT("breakpoint 1234 or breakpoint 0xABCD or breakpoint 0765 or breakpoint 0b1010101\n");
                            }
                            else if(isContinue || isStepIn)
                            {
                                if(hitBreakpoint)
                                {
                                    s64 currentInstructionDataAtHitBreakpoint = 0;
                                    if(PtracePeekText(programID, hitBreakpoint->address, &currentInstructionDataAtHitBreakpoint, sizeof(currentInstructionDataAtHitBreakpoint)) != -1)
                                    {
                                        s64 restoreInstructionTrap = currentInstructionDataAtHitBreakpoint;
                                        s64 removeInstructionTrap = (currentInstructionDataAtHitBreakpoint & 0xFFFFFFFFFFFFFF00) | (hitBreakpoint->instructionData        & 0x00000000000000FF);
                                        
                                        if(PtracePokeText(programID, hitBreakpoint->address, &(removeInstructionTrap), sizeof(removeInstructionTrap)) != -1)
                                        {
                                            Registersx86_64 newRegisters = registers;
                                            newRegisters.rip--;
                                            if(PtraceSetGPRegisters(programID, &newRegisters) != -1)
                                            {
                                                if(ptrace(PTRACE_SINGLESTEP, programID, 0, 0) != -1)
                                                {
                                                    siginfo_t clear = {};
                                                    signalInfo = clear;
                                                    if(waitid(P_PID, programID, &signalInfo, WNOWAIT | WUNTRACED) != -1)
                                                    {
                                                        if(PtracePokeText(programID, hitBreakpoint->address, &restoreInstructionTrap, sizeof(restoreInstructionTrap)) != -1)
                                                        {
                                                            
                                                        }
                                                        else
                                                        {
                                                            ERR_WITH_LOCATION_INFO("ERRNO:%d Failed to restore breakpoint at %"PRIu64" after stepping over it. %s", errno, hitBreakpoint->address, strerror(errno));
                                                        }
                                                    }else goto breakpoint_stepover_failure;
                                                }else goto breakpoint_stepover_failure;
                                            }else goto breakpoint_stepover_failure;
                                        }else goto breakpoint_stepover_failure;
                                    }
                                    else
                                    {
                                        breakpoint_stepover_failure: ERR_WITH_LOCATION_INFO("ERRNO:%d Failed to step over breakpoint at %"PRIu64". %s", errno, hitBreakpoint->address, strerror(errno));
                                    }
                                }
                                
                                commandContinue = true;
                                if(isContinue)
                                {
                                    if(ptrace(PTRACE_CONT, programID, 0, 0) == -1)
                                    {
                                        ERR_WITH_LOCATION_INFO("ERRNO:%d Failed to continue. %s", errno, strerror(errno));
                                    }
                                }
                                else if(!hitBreakpoint)
                                {
                                    ASSERT(isStepIn);
                                    if(ptrace(PTRACE_SINGLESTEP, programID, 0, 0) == -1)
                                    {
                                        ERR_WITH_LOCATION_INFO("ERRNO:%d Failed to step over. %s", errno, strerror(errno));
                                    }
                                }
                                
                            }
                            else if(registerInfo.size != 0)
                            {
                                char *name = commandString;
                                s64 spaceIndex = GetFirstIndex(name, ' ');
                                if(spaceIndex != -1)
                                {
                                    u32 nameLength = (u32)spaceIndex;
                                    Registersx86_64 newRegisters = registers;
                                    RegisterLocationSizeInfo m = registerInfo;
                                    
                                    char *data = name + nameLength + 1;
                                    u32 dataLength = commandStringLength - nameLength - 1;
                                    if(m.size == 8)
                                    {
                                        u64 *registerLocationInStruct = (u64*)(((u8*)&newRegisters) + m.offset);
                                        IdentifyStringifiedDataTypeAndWrite64BitData(data, registerLocationInStruct, dataLength);
                                    }
                                    else if(m.size == 4)
                                    {
                                        u32 *registerLocationInStruct = (u32*)(((u8*)&newRegisters) + m.offset);
                                        IdentifyStringifiedDataTypeAndWrite32BitData(data, registerLocationInStruct, dataLength);
                                    }
                                    else if(m.size == 10)
                                    {
                                        // TODO(Karan): For 80bit floating point value, we can take user's input, 
                                        // load it into x87 FPU of the debugger, read the STx register, copy that
                                        // data into debuggee's x87 FPU state
                                        DataType type = GetDataType(data, dataLength);
                                        switch(type)
                                        {
                                            case numeric_hexadecimal:
                                            {
                                                if(dataLength > 16)
                                                {
                                                    u32 lowQuadStrLength = 16;
                                                    u32 highQuadStrLength = dataLength - lowQuadStrLength;
                                                    
                                                    char *highQuadStr = data;
                                                    char *lowQuadStr = data + highQuadStrLength;
                                                    
                                                    u64 highQuad = InterpretHexadecimalAs64BitData(highQuadStr, highQuadStrLength);
                                                    u64 lowQuad = InterpretHexadecimalAs64BitData(lowQuadStr, lowQuadStrLength); 
                                                    
                                                    void *registerLocation = (((u8*)&newRegisters) + m.offset);
                                                    ArrayCopy(&lowQuad, registerLocation, sizeof(u64));
                                                    registerLocation = (((u8*)registerLocation) + sizeof(u64));
                                                    ArrayCopy(&highQuad, registerLocation, sizeof(u16));
                                                }
                                                else
                                                {
                                                    u64 *registerLocationInStruct = (u64*)(((u8*)&newRegisters) + m.offset);
                                                    *registerLocationInStruct = InterpretHexadecimalAs64BitData(data, dataLength);
                                                }
                                            }break;
                                            case numeric_binary:
                                            {
                                                if(dataLength > 64)
                                                {
                                                    u32 lowQuadStrLength = 64;
                                                    u32 highQuadStrLength = dataLength - lowQuadStrLength;
                                                    
                                                    char *highQuadStr = data;
                                                    char *lowQuadStr = data + highQuadStrLength;
                                                    
                                                    u64 highQuad = InterpretBinaryAs64BitData(highQuadStr, highQuadStrLength);
                                                    u64 lowQuad = InterpretBinaryAs64BitData(lowQuadStr, lowQuadStrLength); 
                                                    
                                                    void *registerLocation = (((u8*)&newRegisters) + m.offset);
                                                    ArrayCopy(&lowQuad, registerLocation, sizeof(u64));
                                                    registerLocation = (((u8*)registerLocation) + sizeof(u64));
                                                    ArrayCopy(&highQuad, registerLocation, sizeof(u16));
                                                }
                                                else
                                                {
                                                    u64 *registerLocationInStruct = (u64*)(((u8*)&newRegisters) + m.offset);
                                                    *registerLocationInStruct = InterpretBinaryAs64BitData(data, dataLength);
                                                }
                                            }break;
                                            case string:
                                            {
                                                void* registerLocationInStruct = (((u8*)&newRegisters) + m.offset);
                                                ArrayCopy(data + 1, registerLocationInStruct, MIN(dataLength - 2, m.size));
                                            }break;
                                            default:
                                            {
                                                OUT("Currently only supports hexadecimal,binary,string inputs for 80bit registers)\n");
                                            }
                                        }
                                    }
                                    else if(m.size == 16)
                                    {
                                        s64 spaceIndex = GetFirstIndex(data, ' ');
                                        if(spaceIndex == 1)
                                        {
                                            u32 quadWordIndex = data[0] - '0';
                                            u64 *registerLocationInStruct = (u64*)(((u8*)&newRegisters) + m.offset);
                                            registerLocationInStruct += quadWordIndex;
                                            IdentifyStringifiedDataTypeAndWrite64BitData(data + 2, registerLocationInStruct, dataLength - 2);
                                        }
                                        else if(spaceIndex == 2)
                                        {
                                            u32 doubleWordIndex = InterpretBinaryAs32BitData(data, 2);
                                            u32 *registerLocationInStruct = (u32*)(((u8*)&newRegisters) + m.offset);
                                            registerLocationInStruct += doubleWordIndex;
                                            IdentifyStringifiedDataTypeAndWrite32BitData(data + 3, registerLocationInStruct, dataLength - 3);
                                        }
                                    }
                                    PtraceSetGPRegisters(programID, &newRegisters);
                                    PtraceSetFPRegisters(programID, &newRegisters);
                                    PtraceGetGPRegisters(programID, &registers);
                                    PtraceGetFPRegisters(programID, &registers);
                                }
                                void *registerLocationInStruct = ((u8*)&registers) + registerInfo.offset;
                                switch(registerInfo.size)
                                {
                                    case 4:
                                    {
                                        OUT("%08"PRIx32"\n", *((u32*)registerLocationInStruct));
                                    }break;
                                    case 8:
                                    {
                                        OUT("%08"PRIx64"\n", *((u64*)registerLocationInStruct));
                                    }break;
                                    case 10:
                                    {
                                        OUT("%04"PRIx64"%016"PRIx64"\n", *(((u64*)registerLocationInStruct) + 1), *((u64*)registerLocationInStruct));
                                    }break;
                                    case 16:
                                    {
                                        u32 *doubleWord = (u32*)registerLocationInStruct;
                                        OUT("%08"PRIx32"|%08"PRIx32"|%08"PRIx32"|%08"PRIx32"\n", doubleWord[3], doubleWord[2], doubleWord[1], doubleWord[0]);
                                    }break;
                                    INVALID_DEFAULT_CASE;
                                }
                            }
                            else if(IsSubstring((char*)"read", commandString))
                            {
                                s64 spaceIndex = GetFirstIndex(commandString, ' ');
                                
                                char *startAddressString = commandString + (u32)spaceIndex + 1;
                                u32 startAddressStringLength = (u32)GetFirstIndex(startAddressString, ' ');
                                u64 startAddress = 0; IdentifyStringifiedDataTypeAndWrite64BitData(startAddressString, &startAddress, startAddressStringLength);
                                char *endAddressString = startAddressString + startAddressStringLength + 1;
                                u32 endAddressStringLength = (u32)GetFirstIndex(startAddressString, ' ');
                                u64 endAddress = 0; IdentifyStringifiedDataTypeAndWrite64BitData(endAddressString, &endAddress, endAddressStringLength);
                                
                                char *displayTypeString = endAddressString + endAddressStringLength + 1;
                                u32 displayTypeStringLength = (u32)GetFirstIndex(displayTypeString, ' ');
                                u32 displayType = 0;
                                IdentifyStringifiedDataTypeAndWrite32BitData(displayTypeString, &displayType, displayTypeStringLength);
                                
                                char *columnsString = displayTypeString + displayTypeStringLength + 1;
                                u32 columnsStringLength = (u32)((commandString + commandStringLength) - columnsString);
                                u32 columns = 0;
                                IdentifyStringifiedDataTypeAndWrite32BitData(columnsString, &columns, columnsStringLength);
                                
                                u32 bytes = (u32)(endAddress - startAddress);
                                
                                //bytes = ALIGN_POW_2(bytes, displayType * columns);
                                u8* data = (u8*)malloc(bytes);
                                u8* freeDataPtr = data;
                                u8* onePastEnd = data + bytes;
                                PtracePeekText(programID, startAddress, data, bytes);
                                
                                u64 currentAddress = startAddress;
                                while(data != onePastEnd)
                                {
                                    OUT("%08"PRIx64": |", currentAddress);
                                    u8* rowStart = data;
                                    for(u32 i = 0; (i < columns) && (data != onePastEnd); i++)
                                    {
                                        for(u32 j = 0; (j < displayType) && (data != onePastEnd); j++)
                                        {
                                            OUT("%02"PRIx8, *data++);
                                            currentAddress++;
                                        }
                                        OUT(" ");
                                    }
                                    
                                    u32 stringBytes = (displayType * columns);
                                    char *c = (char*)rowStart;
                                    OUT("| ");
                                    while(stringBytes--)
                                    {
                                        if(*c >= 32 && *c <= 126)
                                        {
                                            OUT("%c", *c);
                                        }
                                        else
                                        {
                                            OUT(".");
                                        }
                                        c++;
                                    }
                                    OUT("|\n");
                                }
                                
                                free(freeDataPtr);
                                
                            }
                            else if(isRegisters || isX87 || isXMM || isGP)
                            {
                                bool displayGPRegs = isRegisters || isGP;
                                bool displayx87Regs = isRegisters || isX87;
                                bool displayXMMRegs = isRegisters || isXMM;
                                
                                if(displayGPRegs)
                                {
                                    OUT("======================+\n");
                                    OUT("General Purpose       +\n");
                                    OUT("======================+\n");
                                    OUT("RAX : %016"PRIx64"|\n", registers.rax);
                                    OUT("ORGRAX: %016"PRIx64"|\n", registers.orig_rax);
                                    OUT("RBX : %016"PRIx64"|\n", registers.rbx);
                                    OUT("RCX : %016"PRIx64"|\n", registers.rcx);
                                    OUT("RDX : %016"PRIx64"|\n", registers.rdx);
                                    if(mode == x86_64)
                                    {
                                        OUT("R08 : %016"PRIx64"|\n", registers.r8);
                                        OUT("R09 : %016"PRIx64"|\n", registers.r9);
                                        OUT("R10 : %016"PRIx64"|\n", registers.r10);
                                        OUT("R11 : %016"PRIx64"|\n", registers.r11);
                                        OUT("R12 : %016"PRIx64"|\n", registers.r12);
                                        OUT("R13 : %016"PRIx64"|\n", registers.r13);
                                        OUT("R14 : %016"PRIx64"|\n", registers.r14);
                                        OUT("R15 : %016"PRIx64"|\n", registers.r15);
                                    }
                                    OUT("RIP : %016"PRIx64"|\n", registers.rip);
                                    OUT("RBP : %016"PRIx64"|\n", registers.rbp);
                                    OUT("RSP : %016"PRIx64"|\n", registers.rsp);
                                    OUT("RSI : %016"PRIx64"|\n", registers.rsi);
                                    OUT("RDI : %016"PRIx64"|\n", registers.rdi);
                                    OUT(" SS : %016"PRIx64"|\n", registers.ss);
                                    OUT(" CS : %016"PRIx64"|\n", registers.cs);
                                    OUT(" DS : %016"PRIx64"|\n", registers.ds);
                                    OUT(" RS : %016"PRIx64"|\n", registers.es);
                                    OUT(" FS : %016"PRIx64"|\n", registers.fs);
                                    OUT("FSBASE: %016"PRIx64"|\n", registers.fs_base);
                                    OUT(" GS : %016"PRIx64"|\n", registers.gs);
                                    OUT("GSBASE: %016"PRIx64"|\n", registers.gs_base);
                                    OUT("FLAGS: %016"PRIx64"|\n", registers.eflags);
                                }
                                if(displayx87Regs)
                                {
                                    OUT("=========================+\n");
                                    OUT("x87 FPU                  +\n");
                                    OUT("=========================+\n");
                                    OUT("CWD: %020"PRIx16"|\n", registers.cwd);
                                    OUT("SWD: %020"PRIx16"|\n", registers.swd);
                                    OUT("FTW: %020"PRIx16"|\n", registers.ftw);
                                    OUT("FOP: %020"PRIx16"|\n", registers.fop);
                                    
                                    OUT("RIP: %020"PRIx64"|\n", registers.fip);
                                    OUT("RDP: %020"PRIx64"|\n", registers.rdp);
                                    
                                    for(u32 i = 0; i < ARRAY_COUNT(registers.st_space); i += 4)
                                    {
                                        OUT("ST%d: %04"PRIx32"%08"PRIx32"%08"PRIx32"|\n", i/4, registers.st_space[i + 2], registers.st_space[i + 1], registers.st_space[i]);
                                        ASSERT(registers.st_space[i + 3] == 0);
                                    }
                                }
                                
                                if(displayXMMRegs)
                                {
                                    OUT("===========================================+\n");
                                    OUT("XMM                                        +\n");
                                    OUT("===========================================+\n");
                                    u32 numRegs = mode == x86_64 ? 16 : 8;
                                    for(u32 i = 0; i < numRegs; i++)
                                    {
                                        u32 lsdw = i * 4;
                                        OUT("XMM%02d  :%08"PRIx32"|%08"PRIx32"|%08"PRIx32"|%08"PRIx32"|\n", i, registers.xmm_space[lsdw + 3], registers.xmm_space[lsdw + 2], registers.xmm_space[lsdw + 1], registers.xmm_space[lsdw + 0]); 
                                    }
                                    OUT("MXCSR  :%08"PRIx32"%27s|\n", registers.mxcsr, "");
                                    OUT("MXCRMASK:%08"PRIx32"%27s|\n", registers.mxcr_mask, "");
                                }
                            }
                            else if(IsSubstring((char*)"breakpoint", commandString))
                            {
                                if(breakpointCount < ARRAY_COUNT(breakpoints))
                                {
                                    s64 spaceIndex = GetFirstIndex(commandString, ' ');
                                    if(spaceIndex != -1)
                                    {
                                        u32 addressStringLength = commandStringLength - ((u32)spaceIndex + 1);
                                        char *addressString = commandString + (spaceIndex + 1);
                                        
                                        u64 breakpointAddress = 0;
                                        IdentifyStringifiedDataTypeAndWrite64BitData(addressString, &breakpointAddress, addressStringLength);
                                        
                                        s64 instructionData = 0;
                                        if(PtracePeekText(programID, breakpointAddress, &instructionData, sizeof(instructionData)) != -1)
                                        {
                                            s64 originalInstructionData = instructionData;
                                            if((instructionData & 0x00000000000000FF) != 0x00000000000000cc)
                                            {
                                                instructionData = (instructionData & 0xFFFFFFFFFFFFFF00) | 0x00000000000000cc;
                                                if(PtracePokeText(programID, breakpointAddress, &instructionData, sizeof(instructionData)) != -1)
                                                {
                                                    
                                                    Breakpoint *b = breakpoints + breakpointCount++;
                                                    b->address = breakpointAddress;
                                                    b->instructionData = originalInstructionData;
                                                    b->isTemporary = false;
                                                }
                                                else
                                                {
                                                    ERR_WITH_LOCATION_INFO("ERRNO:%d Failed to set breakpoint at %s = %"PRIu64". %s", errno, addressString, breakpointAddress, strerror(errno));
                                                }
                                            }
                                        }
                                        else
                                        {
                                            ERR_WITH_LOCATION_INFO("ERRNO:%d Failed to set breakpoint at %s = %"PRIu64". %s", errno, addressString, breakpointAddress, strerror(errno));
                                        }
                                    }
                                    else
                                    {
                                        OUT("Specify breakpoint address\n");
                                    }
                                }
                                else
                                {
                                    OUT("Max breakpoints reached!\n");
                                }
                            }
                            else
                            {
                                OUT("Invalid command\n");
                                INVALID_CODE_PATH;
                            }
                        }
                        
                        
                        PtraceGetGPRegisters(programID, &registers);
                        PtraceGetFPRegisters(programID, &registers);
                        hitBreakpoint = FindBreakpointSetAt(registers.rip - 1, breakpoints, ARRAY_COUNT(breakpoints));
                        
                    }
                }
                else if(WIFEXITED(status))
                {
                    OUT("Debuggee exited with code: %d\n", WEXITSTATUS(status));
                    debugging = false;
                }
                else if(WIFSIGNALED(status))
                {
                    OUT("Debuggee was terminated with signal: %d\n", WTERMSIG(status));
                    debugging = false;
                }
                else
                {
                    ASSERT(!"Not implemented");
                }
            }
            else if(programID == -1)
            {
                ERR_WITH_LOCATION_INFO("ERRNO:%d, %s", errno, strerror(errno));
                INVALID_CODE_PATH;
                return errno;
            }
        }
    }
    else
    {
        ERR_WITH_LOCATION_INFO("ERRNO:%d Failed to fork the process to launch debuggee. %s", errno, strerror(errno));
        return errno;
    }
    
    return 0;
}