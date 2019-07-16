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
        if(IsPrefix(name, (char*)"r8")){result.offset = (offsetof(GPRegistersx86_64 , r15)); result.size = 8;}
        else if(IsPrefix(name, (char*)"r9")) {result.offset = offsetof(Registersx86_64, r9); result.size = 8;} 
        else if(IsPrefix(name, (char*)"cs")) {result.offset = offsetof(Registersx86_64, cs); result.size = 8;} 
        else if(IsPrefix(name, (char*)"ss")) {result.offset = offsetof(Registersx86_64, ss); result.size = 8;} 
        else if(IsPrefix(name, (char*)"ds")) {result.offset = offsetof(Registersx86_64, ds); result.size = 8;} 
        else if(IsPrefix(name, (char*)"es")) {result.offset = offsetof(Registersx86_64, es); result.size = 8;} 
        else if(IsPrefix(name, (char*)"fs")) {result.offset = offsetof(Registersx86_64, fs); result.size = 8;} 
        else if(IsPrefix(name, (char*)"gs")) {result.offset = offsetof(Registersx86_64, gs); result.size = 8;} 
    }
    else if(length == 3)
    {
        if(IsPrefix(name, (char*)"st")) 
        {
            result.offset = offsetof(Registersx86_64, st_space); 
            result.size = 10;
            u64 registerNumber = InterpretDecimalIntegerAs64BitData(name + 2, 1);
            result.offset += (u32)(registerNumber * 4 * sizeof(u32));
        } 
        else if(IsPrefix(name, (char*)"r10")) {result.offset = offsetof(Registersx86_64, r10); result.size = 8;} 
        else if(IsPrefix(name, (char*)"r11")) {result.offset = offsetof(Registersx86_64, r11); result.size = 8;} 
        else if(IsPrefix(name, (char*)"r12")) {result.offset = offsetof(Registersx86_64, r12); result.size = 8;} 
        else if(IsPrefix(name, (char*)"r13")) {result.offset = offsetof(Registersx86_64, r13); result.size = 8;} 
        else if(IsPrefix(name, (char*)"r14")) {result.offset = offsetof(Registersx86_64, r14); result.size = 8;} 
        else if(IsPrefix(name, (char*)"r15")) {result.offset = offsetof(Registersx86_64, r15); result.size = 8;} 
        else if(IsPrefix(name, (char*)"rbp")) {result.offset = offsetof(Registersx86_64, rbp); result.size = 8;} 
        else if(IsPrefix(name, (char*)"rbx")) {result.offset = offsetof(Registersx86_64, rbx); result.size = 8;} 
        else if(IsPrefix(name, (char*)"rax")) {result.offset = offsetof(Registersx86_64, rax); result.size = 8;} 
        else if(IsPrefix(name, (char*)"rcx")) {result.offset = offsetof(Registersx86_64, rcx); result.size = 8;} 
        else if(IsPrefix(name, (char*)"rdx")) {result.offset = offsetof(Registersx86_64, rdx); result.size = 8;} 
        else if(IsPrefix(name, (char*)"rsi")) {result.offset = offsetof(Registersx86_64, rsi); result.size = 8;} 
        else if(IsPrefix(name, (char*)"rdi")) {result.offset = offsetof(Registersx86_64, rdi); result.size = 8;} 
        else if(IsPrefix(name, (char*)"rip")) {result.offset = offsetof(Registersx86_64, rip); result.size = 8;} 
        else if(IsPrefix(name, (char*)"rsp")) {result.offset = offsetof(Registersx86_64, rsp); result.size = 8;} 
        else if(IsPrefix(name, (char*)"cwd")) {result.offset = offsetof(Registersx86_64, cwd); result.size = 8;} 
        else if(IsPrefix(name, (char*)"swd")) {result.offset = offsetof(Registersx86_64, swd); result.size = 8;} 
        else if(IsPrefix(name, (char*)"ftw")) {result.offset = offsetof(Registersx86_64, ftw); result.size = 8;} 
        else if(IsPrefix(name, (char*)"fop")) {result.offset = offsetof(Registersx86_64, fop); result.size = 8;} 
        else if(IsPrefix(name, (char*)"fip")) {result.offset = offsetof(Registersx86_64, fip); result.size = 8;} 
        else if(IsPrefix(name, (char*)"rdp")) {result.offset = offsetof(Registersx86_64, rdp); result.size = 8;} 
    }
    else if(length == 4 || length == 5)
    {
        if(IsPrefix(name, (char*)"flags")) {result.offset = offsetof(Registersx86_64, eflags); result.size = 8;} 
        else if(IsPrefix(name, (char*)"mxcsr")) {result.offset = offsetof(Registersx86_64, mxcsr); result.size = 4;} 
        else if(IsPrefix(name, (char*)"xmm")) 
        {
            name += 3;
            u32 registerNumberLength = length - 3;
            u32 registerNumber = InterpretDecimalIntegerAs32BitData(name, registerNumberLength);
            
            result.size = 4 * sizeof(u32);
            result.offset = (u32)(offsetof(Registersx86_64, xmm_space) + (registerNumber * 4 * sizeof(u32)));
        }
    }
    else if(length == 6 || length == 7)
    {
        if(IsPrefix(name, (char*)"xmm")) 
        {
            name += 3;
            u32 registerNumberLength = 1;
            bool isDoubleWord = false;
            if(name[1] == 'd' || name[2] == 'd'){ isDoubleWord = true; registerNumberLength = (name[1] == 'd' ? 1 : 2);}
            else if(name[1] == 'q' || name[2] == 'q'){ isDoubleWord = false; registerNumberLength = (name[1] == 'q' ? 1 : 2);}
            
            u32 registerNumber = InterpretDecimalIntegerAs32BitData(name, registerNumberLength);
            name += registerNumberLength + 1;
            u32 indexWithinRegister = InterpretDecimalIntegerAs32BitData(name, 1);
            
            result.size = (u32)(isDoubleWord ? sizeof(u32) : sizeof(u64));
            result.offset = (u32)(offsetof(Registersx86_64, xmm_space) + (registerNumber * 4 * sizeof(u32)) + (indexWithinRegister * result.size));
        }
        else if(IsPrefix(name, (char*)"orgrax")) {result.offset = offsetof(Registersx86_64, orig_rax); result.size = 8;} 
        else if(IsPrefix(name, (char*)"fsbase")) {result.offset = offsetof(Registersx86_64, fs_base); result.size = 8;} 
        else if(IsPrefix(name, (char*)"gsbase")) {result.offset = offsetof(Registersx86_64, gs_base); result.size = 8;} 
    }
    else if(length == 8)
    {
        if(IsPrefix(name, (char*)"mxcrmask")) {result.offset = offsetof(Registersx86_64, mxcr_mask); result.size = 4;} 
    }
    
    return result;
}


struct ErrorInfo
{
    __ptrace_request request;
    s32 errorCode;
    u32 line;
    char *file;
    char *function;
};


struct PtraceErrorBookKeeping
{
    u32 errorCount;
    ErrorInfo info[16];
};

void DumpToStdErr(PtraceErrorBookKeeping *bookKeeping)
{
    char *request;
    char* requestStrings[] = {(char*)"PTRACE_GETREGS", (char*)"PTRACE_GETFPREGS", (char*)"PTRACE_GETFPXREGS", (char*)"PTRACE_SETREGS", (char*)"PTRACE_SETFPREGS", (char*)"PTRACE_SETFPXREGS", (char*)"PTRACE_POKETEXT", (char*)"PTRACE_SINGLESTEP", (char*)"PTRACE_CONT", (char*)"PTRACE_PEEKTEXT"};
    if(bookKeeping->errorCount == (ARRAY_COUNT(bookKeeping->info) - 1))
    {
        ERR("Ptrace error book keeping is full! There might have been more errors!\n");
    }
    
    for(u32 i = 0; i < bookKeeping->errorCount; i++)
    {
        switch(bookKeeping->info[i].request)
        {
            case PTRACE_GETREGS:
            {
                request = requestStrings[0];
            }break;
            case PTRACE_GETFPREGS:
            {
                request = requestStrings[1];
            }break;
            case PTRACE_GETFPXREGS:
            {
                request = requestStrings[2];
            }break;
            case PTRACE_SETREGS:
            {
                request = requestStrings[3];
            }break;
            case PTRACE_SETFPREGS:
            {
                request = requestStrings[4];
            }break;
            case PTRACE_SETFPXREGS:
            {
                request = requestStrings[5];
            }break;
            case PTRACE_POKETEXT:
            {
                request = requestStrings[6];
            }break;
            case PTRACE_SINGLESTEP:
            {
                request = requestStrings[7];
            }break;
            case PTRACE_CONT:
            {
                request = requestStrings[8];
            }break;
            case PTRACE_PEEKTEXT:
            {
                request = requestStrings[9];
            }break;
            INVALID_DEFAULT_CASE;
        }
        
        ERR("ERROR:%d for %s, %s @ %s %s(%d)\n", bookKeeping->info[i].errorCode, request, strerror(bookKeeping->info[i].errorCode), bookKeeping->info[i].function, bookKeeping->info[i].file, bookKeeping->info[i].line);
    }
    bookKeeping->errorCount = 0;
}


#define PTRACE(a, b, c, d, e) PtraceWrapper((a), (b), (c), (d), (e), (char*)__FILE__, (char*)__func__, __LINE__)
long PtraceWrapper(__ptrace_request request, pid_t pid, void *address, void *data, PtraceErrorBookKeeping *bookKeeping, char *file, char* function, u32 line)
{
    long result = 0;
    errno = 0;
    bool errored = false;
    switch(request)
    {
        case PTRACE_GETREGS:
        {
            result = ptrace(request, pid, address, data);
            if(result == -1)
            {
                ClearArray(data, sizeof(struct user_regs_struct));
                errored = true;
            }
        }break;
        case PTRACE_GETFPREGS:
        {
            result = ptrace(request, pid, address, data);
            if(result == -1)
            {
                ClearArray(data, sizeof(struct user_fpregs_struct));
                errored = true;
            }
        }break;
#ifndef __x86_64__
        case PTRACE_GETFPXREGS:
        {
            result = ptrace(request, pid, address, data);
            if(result == -1)
            {
                ClearArray(data, sizeof(struct user_fpxregs_struct));
                errored = true;
            }
        }break;
#endif
        case PTRACE_SETREGS:
        case PTRACE_SETFPREGS:
        case PTRACE_SETFPXREGS:
        case PTRACE_POKETEXT:
        case PTRACE_SINGLESTEP:
        case PTRACE_CONT:
        {
            if(bookKeeping->errorCount == 0)
            {
                long result = ptrace(request, pid, address, data);
                if(result == -1)
                {
                    errored = true;
                }
            }
            else
            {
                errored = true;
                result = -1;
            }
        }break;
        case PTRACE_PEEKTEXT:
        {
            result = ptrace(request, pid, address, data);
            if(errno != 0)
            {
                errored = true;
                result = -1;
            }
        }break;
        INVALID_DEFAULT_CASE;
    }
    
    if(errored && errno != 0 && bookKeeping)
    {
        if((bookKeeping->errorCount < ARRAY_COUNT(bookKeeping->info)))
        {
            ErrorInfo *info = bookKeeping->info + bookKeeping->errorCount++;
            info->errorCode = errno;
            info->file = file;
            info->function = function;
            info->line = line;
            info->request = request;
        }
        else
        {
            ASSERT(!"Error queue filled!");
        }
    }
    
    return result;
}


#define PTRACE_POKE_VARIABLE_LENGTH_TEXT(a, b, c, d, e) PtracePokeText((a), (b), (c), (d), (e), (char*)__FILE__, (char*)__func__, __LINE__)
internal s64 PtracePeekText(pid_t pid, u64 address, void *data, u32 bytes, PtraceErrorBookKeeping *bookKeeping, char *file, char* function, u32 line);
internal s64 PtracePokeText(pid_t pid, u64 address, void *data, u32 bytes, PtraceErrorBookKeeping *bookKeeping, char *file, char* function, u32 line)
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
        long returnValue = PtraceWrapper(PTRACE_POKETEXT, pid, (void*)currentAddress, (void*)(*currentLong), bookKeeping, file, function, line);
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
        
        if(PtracePeekText(pid, currentAddress, &dataAtCurrentAddress, sizeof(dataAtCurrentAddress), bookKeeping, file, function, line) != -1)
        {
            char* currentData = (((char*)data) + (numLongsInData * longSize));
            ArrayCopy(currentData, &dataAtCurrentAddress, leftOverBytes);
            long returnValue = PtraceWrapper(PTRACE_POKETEXT, pid, (void*)currentAddress, (void*)dataAtCurrentAddress, bookKeeping, file, function, line);
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

#define PTRACE_PEEK_VARIABLE_LENGTH_TEXT(a, b, c, d, e) PtracePeekText((a), (b), (c), (d), (e), (char*)__FILE__, (char*)__func__, __LINE__)
internal s64 PtracePeekText(pid_t pid, u64 address, void *data, u32 bytes, PtraceErrorBookKeeping *bookKeeping, char *file, char* function, u32 line)
{
    u64 currentAddress = address;
    char* currentData = (char*)data;
    char* onePastEnd = currentData + bytes;
    while(currentData != onePastEnd)
    {
        // TODO(Karan): How is this working?? I had to convert it to 32 for PokeText to work...Investigate this!!
        errno = 0;
        long dataAtCurrentAddress = PtraceWrapper(PTRACE_PEEKTEXT, pid, (void*)currentAddress, 0, bookKeeping, file, function, line);
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



#define PTRACE_GET_GPREGISTERS(a, b, c) PtraceGetGPRegisters((a), (b), (c), (char*)__FILE__, (char*)__func__, __LINE__)
internal s64 PtraceGetGPRegisters(pid_t pid, Registersx86_64 *registers, PtraceErrorBookKeeping *bookKeeping, char *file, char *function, u32 line)
{
    struct user_regs_struct regs;
    s64 result = PtraceWrapper(PTRACE_GETREGS, pid, &regs, &regs, bookKeeping, file, function, line);
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
    return result;
}

#define PTRACE_GET_FPREGISTERS(a, b, c) PtraceGetFPRegisters((a), (b), (c), (char*)__FILE__, (char*)__func__, __LINE__)
internal s64 PtraceGetFPRegisters(pid_t pid, Registersx86_64 *registers, PtraceErrorBookKeeping *bookKeeping, char *file, char *function, u32 line)
{
    s64 result = 0;
#ifndef __x86_64__
    struct user_fpxregs_struct fpRegs;
    result = PtraceWrapper(PTRACE_GETFPXREGS, pid, &fpRegs, &fpRegs, bookKeeping, file, function, line);
    
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
#else 
    struct user_fpregs_struct fpRegs;
    result = PtraceWrapper(PTRACE_GETFPREGS, pid, &fpRegs, &fpRegs, bookKeeping, file, function, line);
    CopyUserFPRegsx86_64ToFPRegistersx86_64(&fpRegs, &(registers->fpRegs));
#endif
    return result;
}

#define PTRACE_SET_GPREGISTERS(a, b, c) PtraceSetGPRegisters((a), (b), (c), (char*)__FILE__, (char*)__func__, __LINE__)
internal s64 PtraceSetGPRegisters(pid_t pid, Registersx86_64 *registers, PtraceErrorBookKeeping *bookKeeping, char *file, char *function, u32 line)
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
    
    return PtraceWrapper(PTRACE_SETREGS, pid, &regs, &regs, bookKeeping, file, function, line);
}

#define PTRACE_SET_FPREGISTERS(a, b, c) PtraceSetFPRegisters((a), (b), (c), (char*)__FILE__, (char*)__func__, __LINE__)
internal s64 PtraceSetFPRegisters(pid_t pid, Registersx86_64 *registers, PtraceErrorBookKeeping *bookKeeping, char *file, char *function, u32 line)
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
    return PtraceWrapper(PTRACE_SETFPXREGS, pid, &fpRegs, &fpRegs, bookKeeping, file, function, line);
#else 
    struct user_fpregs_struct fpRegs = {};
    CopyFPRegistersx86_64ToUserFPRegsx86_64(&(registers->fpRegs), &fpRegs);
    return PtraceWrapper(PTRACE_SETFPREGS, pid, &fpRegs, &fpRegs, bookKeeping, file, function, line);
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

enum CommandType
{
    command_invalid,
    command_ls,
    command_continue,
    command_stepin,
    command_read_all_registers,
    command_read_x87,
    command_read_gp, 
    command_read_xmm,
    command_breakpoint,
    command_read_memory,
    command_read_register,
    command_write_register,
};

struct BreakpointCommandArguments
{
    u64 breakpointAddress;
};

struct ReadMemoryCommandArguments
{
    u64 startAddress;
    u64 endAddress;
    u32 columnSize;
    u32 columns;
};

struct ReadWriteRegisterCommandArguments
{
    RegisterLocationSizeInfo registerInfo;
    char data[16];
};

struct CommandInfo
{
    CommandType type;
    union
    {
        BreakpointCommandArguments breakpointCommandArguments;
        ReadMemoryCommandArguments readMemoryCommandArguments;
        ReadWriteRegisterCommandArguments readWriteRegisterCommandArguments;
    };
};

void ParseCommand(Cursor cursor, CommandInfo *commandInfo)
{
    CommandInfo clear = {};
    *commandInfo = clear;
    
    EatAllWhitespace(&cursor);
    if(MatchAndAdvance(&cursor, (char*)"ls"))
    {
        commandInfo->type = command_ls;
        return;
    }
    else if(MatchAndAdvance(&cursor, (char*)"gp"))
    {
        commandInfo->type = command_read_gp;
        return;
    }
    else if(MatchAndAdvance(&cursor, (char*)"xmm\n"))
    {
        commandInfo->type = command_read_xmm;
        return;
    }
    else if(MatchAndAdvance(&cursor, (char*)"x87"))
    {
        commandInfo->type = command_read_x87;
        return;
    }
    else if(MatchAndAdvance(&cursor, (char*)"continue"))
    {
        commandInfo->type = command_continue;
        return;
    }
    else if(MatchAndAdvance(&cursor, (char*)"step in"))
    {
        commandInfo->type = command_stepin;
        return;
    }
    else if(MatchAndAdvance(&cursor, (char*)"registers"))
    {
        commandInfo->type = command_read_all_registers;
        return;
    }
    else if(MatchAndAdvance(&cursor, (char*)"breakpoint"))
    {
        EatAllWhitespace(&cursor);
        Cursor next = cursor;
        EatUntilWhitespace(&next);
        u32 length = (u32)(next.at - cursor.at);
        if(length != 0)
        {
            commandInfo->type = command_breakpoint;
            IdentifyStringifiedDataTypeAndWrite64BitData(cursor.at, &(commandInfo->breakpointCommandArguments.breakpointAddress));
        }
        else
        {
            commandInfo->type = command_invalid;
            ERR("Syntax error: breakpoint <address>\n");
        }
        return;
    }
    else if(MatchAndAdvance(&cursor, (char*)"read"))
    {
        EatAllWhitespace(&cursor);
        Cursor next = cursor;
        EatUntilWhitespace(&next);
        commandInfo->type = command_read_memory;
        u32 length = (u32)(next.at - cursor.at);
        if(length == 0) commandInfo->type = command_invalid;
        IdentifyStringifiedDataTypeAndWrite64BitData(cursor.at, &(commandInfo->readMemoryCommandArguments.startAddress), length);
        
        cursor = next;
        EatAllWhitespace(&cursor);
        next = cursor;
        EatUntilWhitespace(&next);
        length = (u32)(next.at - cursor.at);
        if(length == 0) commandInfo->type = command_invalid;
        IdentifyStringifiedDataTypeAndWrite64BitData(cursor.at, &(commandInfo->readMemoryCommandArguments.endAddress), (u32)(next.at - cursor.at));
        
        cursor = next;
        EatAllWhitespace(&cursor);
        next = cursor;
        EatUntilWhitespace(&next);
        length = (u32)(next.at - cursor.at);
        if(length == 0) commandInfo->type = command_invalid;
        IdentifyStringifiedDataTypeAndWrite32BitData(cursor.at, &(commandInfo->readMemoryCommandArguments.columnSize), (u32)(next.at - cursor.at));
        
        cursor = next;
        EatAllWhitespace(&cursor);
        next = cursor;
        EatUntilWhitespace(&next);
        length = (u32)(next.at - cursor.at);
        if(length == 0) commandInfo->type = command_invalid;
        IdentifyStringifiedDataTypeAndWrite32BitData(cursor.at, &(commandInfo->readMemoryCommandArguments.columns), (u32)(next.at - cursor.at));
        
        if(commandInfo->type == command_invalid) ERR("Syntax error: read <start_address> <end_address> <columnSize> <columns>\n");
        
        return;
    }
    else
    {
        Cursor potentialRegistername = cursor;
        EatUntilWhitespace(&cursor);
        RegisterLocationSizeInfo registerInfo = GetRegisterLocationSizeInfo(potentialRegistername.at, (u32)(cursor.at - potentialRegistername.at));
        if(registerInfo.size != 0)
        {
            EatAllWhitespace(&cursor);
            Cursor potentialRegisterValueString = cursor;
            EatUntilEndOfLine(&cursor);
            u32 dataLength = (u32)(cursor.at - potentialRegisterValueString.at);
            if(dataLength == 0)
            {
                commandInfo->type = command_read_register;
                commandInfo->readWriteRegisterCommandArguments.registerInfo = registerInfo;
                return;
            }
            else
            {
                commandInfo->type = command_write_register;
                commandInfo->readWriteRegisterCommandArguments.registerInfo = registerInfo;
                
                if(registerInfo.size == 8)
                {
                    IdentifyStringifiedDataTypeAndWrite64BitData(potentialRegisterValueString.at, (u64*)(commandInfo->readWriteRegisterCommandArguments.data), dataLength);
                }
                else if(registerInfo.size == 4)
                {
                    IdentifyStringifiedDataTypeAndWrite32BitData(potentialRegisterValueString.at, (u32*)(commandInfo->readWriteRegisterCommandArguments.data), dataLength);
                }
                else if(registerInfo.size == 10  || registerInfo.size == 16)
                {
                    // TODO(Karan): For 80bit floating point value, we can take user's input, 
                    // load it into x87 FPU of the debugger, read the STx register, copy that
                    // data into debuggee's x87 FPU state
                    // HACK(Karan): Handle vector registers separately
                    DataType type = GetDataType(potentialRegisterValueString.at);
                    switch(type)
                    {
                        case numeric_hexadecimal:
                        {
                            InterpretHexadecimalAs80BitData(potentialRegisterValueString.at, (commandInfo->readWriteRegisterCommandArguments.data), dataLength);
                        }break;
                        case numeric_binary:
                        {
                            InterpretBinaryAs80BitData(potentialRegisterValueString.at, (commandInfo->readWriteRegisterCommandArguments.data), dataLength);
                        }break;
                        case string:
                        {
                            ArrayCopy(potentialRegisterValueString.at + 1, (commandInfo->readWriteRegisterCommandArguments.data), MIN(dataLength - 2, registerInfo.size));
                        }break;
                        default:
                        {
                            OUT("Currently only supports hexadecimal,binary,string inputs for 80bit registers)\n");
                        }
                    }
                }
                return;
            }
        }
        else
        {
            commandInfo->type = command_invalid;
            ERR("Invalid command\n");
        }
    }
}


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
        PtraceErrorBookKeeping bookKeeping = {};
        
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
                    siginfo_t signalInfo = {};
                    ptrace(PTRACE_GETSIGINFO, programID, 0, &signalInfo);
                    psiginfo(&signalInfo, 0);
                    
                    Breakpoint *hitBreakpoint = 0;
                    Registersx86_64 registers = {};
                    PTRACE_GET_GPREGISTERS(programID, &registers, &bookKeeping);
                    PTRACE_GET_FPREGISTERS(programID, &registers, &bookKeeping);
                    hitBreakpoint = FindBreakpointSetAt(registers.rip - 1, breakpoints, ARRAY_COUNT(breakpoints));
                    
                    s64 instructionDataAtInstructionPointer = 0;
                    PTRACE_PEEK_VARIABLE_LENGTH_TEXT(programID, registers.rip, &instructionDataAtInstructionPointer, sizeof(instructionDataAtInstructionPointer), 0);
                    OUT("RIP: %"PRIx64": %"PRIx64"\n", registers.rip, instructionDataAtInstructionPointer);
                    
                    bool commandContinue = false;
                    while(!commandContinue)
                    {
                        PTRACE_GET_GPREGISTERS(programID, &registers, &bookKeeping);
                        PTRACE_SET_FPREGISTERS(programID, &registers, &bookKeeping);
                        hitBreakpoint = FindBreakpointSetAt(registers.rip - 1, breakpoints, ARRAY_COUNT(breakpoints));
                        
                        char commandStringBuffer[512] = {};
                        char *commandString = (char*)commandStringBuffer;
                        OUT("> ");
                        
                        commandString = fgets(commandString, ARRAY_COUNT(commandStringBuffer), stdin);
                        CommandInfo commandInfo = {};
                        Cursor cursor = {};
                        cursor.at = commandString;
                        ParseCommand(cursor, &commandInfo);
                        
                        if(commandInfo.type != command_invalid)
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
                            if(commandInfo.type == command_ls)
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
                            else if(commandInfo.type == command_continue || commandInfo.type == command_stepin)
                            {
                                if(hitBreakpoint)
                                {
                                    s64 currentInstructionDataAtHitBreakpoint = 0;
                                    PTRACE_PEEK_VARIABLE_LENGTH_TEXT(programID, hitBreakpoint->address, &currentInstructionDataAtHitBreakpoint, sizeof(currentInstructionDataAtHitBreakpoint), &bookKeeping);
                                    
                                    s64 restoreInstructionTrap = currentInstructionDataAtHitBreakpoint;
                                    s64 removeInstructionTrap = (currentInstructionDataAtHitBreakpoint & 0xFFFFFFFFFFFFFF00) | (hitBreakpoint->instructionData  & 0x00000000000000FF);
                                    
                                    PTRACE_POKE_VARIABLE_LENGTH_TEXT(programID, hitBreakpoint->address, &(removeInstructionTrap), sizeof(removeInstructionTrap), &bookKeeping);
                                    Registersx86_64 newRegisters = registers;
                                    newRegisters.rip--;
                                    PTRACE_SET_GPREGISTERS(programID, &newRegisters, &bookKeeping);
                                    if(PTRACE(PTRACE_SINGLESTEP, programID, 0, 0, &bookKeeping) != -1)
                                    {
                                        siginfo_t clear = {};
                                        signalInfo = clear;
                                        waitid(P_PID, programID, &signalInfo, WNOWAIT | WUNTRACED);
                                    }
                                    
                                    PTRACE_POKE_VARIABLE_LENGTH_TEXT(programID, hitBreakpoint->address, &restoreInstructionTrap, sizeof(restoreInstructionTrap), &bookKeeping);
                                }
                                
                                commandContinue = true;
                                if(commandInfo.type == command_continue) 
                                {
                                    PTRACE(PTRACE_CONT, programID, 0, 0, &bookKeeping);
                                }
                                else if(commandInfo.type == command_stepin && !hitBreakpoint)
                                {
                                    PTRACE(PTRACE_SINGLESTEP, programID, 0, 0, &bookKeeping);
                                }
                            }
                            else if(commandInfo.type == command_write_register || commandInfo.type == command_read_register)
                            {
                                ReadWriteRegisterCommandArguments arguments = commandInfo.readWriteRegisterCommandArguments;
                                Registersx86_64 newRegisters = registers;
                                RegisterLocationSizeInfo registerInfo = arguments.registerInfo;
                                void* registerLocationInStruct = (void*)(((u8*)&newRegisters) + registerInfo.offset);
                                
                                if(commandInfo.type == command_write_register)
                                {
                                    ArrayCopy(arguments.data, registerLocationInStruct, registerInfo.size);
                                    
                                    PTRACE_SET_GPREGISTERS(programID, &newRegisters, &bookKeeping);
                                    PTRACE_SET_FPREGISTERS(programID, &newRegisters, &bookKeeping);
                                    PTRACE_GET_GPREGISTERS(programID, &registers, &bookKeeping);
                                    PTRACE_GET_FPREGISTERS(programID, &registers, &bookKeeping);
                                }
                                
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
                            else if(commandInfo.type == command_read_memory)
                            {
                                ReadMemoryCommandArguments arguments = commandInfo.readMemoryCommandArguments;
                                u64 startAddress = arguments.startAddress;
                                u64 endAddress = arguments.endAddress;
                                u32 columnSize = arguments.columnSize;
                                u32 columns = arguments.columns;
                                
                                u32 bytes = (u32)(endAddress - startAddress);
                                
                                //bytes = ALIGN_POW_2(bytes, columnSize * columns);
                                u8* data = (u8*)malloc(bytes);
                                u8* freeDataPtr = data;
                                u8* onePastEnd = data + bytes;
                                PTRACE_PEEK_VARIABLE_LENGTH_TEXT(programID, startAddress, data, bytes, &bookKeeping);
                                
                                u64 currentAddress = startAddress;
                                while(data != onePastEnd)
                                {
                                    OUT("%08"PRIx64": |", currentAddress);
                                    u8* rowStart = data;
                                    for(u32 i = 0; (i < columns) && (data != onePastEnd); i++)
                                    {
                                        for(u32 j = 0; (j < columnSize) && (data != onePastEnd); j++)
                                        {
                                            OUT("%02"PRIx8, *data++);
                                            currentAddress++;
                                        }
                                        OUT(" ");
                                    }
                                    
                                    u32 stringBytes = (columnSize * columns);
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
                            else if(commandInfo.type == command_read_all_registers || 
                                    commandInfo.type == command_read_x87 || 
                                    commandInfo.type == command_read_gp || 
                                    commandInfo.type == command_read_xmm)
                            {
                                bool displayGPRegs = commandInfo.type == command_read_all_registers  || commandInfo.type == command_read_gp;
                                bool displayx87Regs = commandInfo.type == command_read_all_registers || commandInfo.type == command_read_x87;
                                bool displayXMMRegs = commandInfo.type == command_read_all_registers || commandInfo.type == command_read_xmm;
                                
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
                            else if(commandInfo.type == command_breakpoint)
                            {
                                BreakpointCommandArguments arguments = commandInfo.breakpointCommandArguments;
                                if(breakpointCount < ARRAY_COUNT(breakpoints))
                                {
                                    u64 breakpointAddress = arguments.breakpointAddress;
                                    s64 instructionData = 0;
                                    PTRACE_PEEK_VARIABLE_LENGTH_TEXT(programID, breakpointAddress, &instructionData, sizeof(instructionData), &bookKeeping);
                                    s64 originalInstructionData = instructionData;
                                    
                                    if((instructionData & 0x00000000000000FF) != 0x00000000000000cc)
                                    {
                                        instructionData = (instructionData & 0xFFFFFFFFFFFFFF00) | 0x00000000000000cc;
                                        if(PTRACE_POKE_VARIABLE_LENGTH_TEXT(programID, breakpointAddress, &instructionData, sizeof(instructionData), &bookKeeping) != -1)
                                        {
                                            Breakpoint *b = breakpoints + breakpointCount++;
                                            b->address = breakpointAddress;
                                            b->instructionData = originalInstructionData;
                                            b->isTemporary = false;
                                        }
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
                        
                        DumpToStdErr(&bookKeeping);
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