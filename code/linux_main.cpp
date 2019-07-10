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

#include "custom_types.h"
#include "utility.h"
#include "string_operations.h"

// NOTE(Karan): Picked a random number for path length
#define MAX_BACKTRACE_STACK_FRAMES 25
#define MAX_FILEPATH_LENGTH 512
#define MAX_FILENAME_LENGTH 128

#if 0
enum ProcessMode
{
    x86,
    x86_64
};
#endif

struct Registersx86_64
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


// NOTE(Karan): Peek and Poke  must r/w _at least_ 64 bits of data  
internal s64 PtracePokeText(pid_t pid, u64 address, void *data, u32 bytes)
{
    u32 longSize = sizeof(long);
    ASSERT(bytes % longSize == 0);
    u32 numLongsInData = bytes/longSize;
    
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
    return 0;
}

internal s64 PtracePeekText(pid_t pid, u64 address, void *data, u32 bytes)
{
    u32 longSize = sizeof(long);
    ASSERT(bytes % longSize == 0);
    u32 numLongsInData = bytes/longSize;
    long* currentData = (long*)data;
    for(u32 i = 0; i < numLongsInData; i++)
    {
        u64 currentAddress = address + (i*longSize);
        errno = 0;
        long dataAtCurrentAddress = ptrace(PTRACE_PEEKTEXT, pid, currentAddress, 0);
        if(errno != 0)
        {
            return -1;
        }
        else
        {
            *currentData = dataAtCurrentAddress;
            currentData++;
        }
    }
    return 0;
}

internal s64 PtraceGetRegisters(pid_t pid, Registersx86_64 *registers)
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
        *((Registersx86_64 *)registers) = *((Registersx86_64 *)(&regs));
#endif
    }
    else
    {
        return -1;
    }
    
    return 0;
}

internal s64 PtraceSetRegisters(pid_t pid, Registersx86_64 *registers)
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
    regs = *((user_regs_struct*)registers);
#endif
    
    return ptrace(PTRACE_SETREGS, pid, &regs, &regs);
    
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
        debuggeeProgramID = (pid_t)InterpretStringAsUint64(argv[1]);
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
        while(debugging)
        {
            s32 status = 0;
            pid_t programID = waitpid(-1, &status, WNOHANG | WUNTRACED | WCONTINUED); 
            if(programID > 0)
            {
#if 0
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
#endif
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
                    
                    if(PtraceGetRegisters(programID, &registers) != -1)
                    {
                        hitBreakpoint = FindBreakpointSetAt(registers.rip - 1, breakpoints, ARRAY_COUNT(breakpoints));
                    }
                    else
                    {
                        ERR_WITH_LOCATION_INFO("ERRNO:%d Failed to read instruction pointer. %s", errno, strerror(errno));
                    }
                    
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
                            
                            if(isContinue || isStepIn)
                            {
                                if(hitBreakpoint)
                                {
                                    Registersx86_64 newRegisters = registers;
                                    newRegisters.rip--;
                                    s64 restoreInstructionTrap = (hitBreakpoint->instructionData & 0xFFFFFFFFFFFFFF00) | 0x00000000000000cc;
                                    
                                    if(PtracePokeText(programID, hitBreakpoint->address, &(hitBreakpoint->instructionData), sizeof(hitBreakpoint->instructionData)) != -1)
                                    {
                                        if(PtraceSetRegisters(programID, &newRegisters) != -1)
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
                            else if(AreStringsSame(commandString, (char*)"registers"))
                            {
                                if(PtraceGetRegisters(programID, &registers) != -1)
                                {
                                    OUT("======================+\n");
                                    OUT("RAX : %016"PRIx64"|\n", registers.rax);
                                    OUT("RBX : %016"PRIx64"|\n", registers.rbx);
                                    OUT("RCX : %016"PRIx64"|\n", registers.rcx);
                                    OUT("RDX : %016"PRIx64"|\n", registers.rdx);
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
                                    OUT(" GS : %016"PRIx64"|\n", registers.gs);
                                    OUT("======================+\n");
                                }
                                else
                                {
                                    ERR_WITH_LOCATION_INFO("ERRNO:%d Failed to get registers. %s", errno, strerror(errno));
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
                                        u64 breakpointAddress = InterpretStringAsUint64(addressString, addressStringLength);
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