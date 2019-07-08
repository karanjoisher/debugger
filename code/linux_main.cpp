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


struct Breakpoint
{
    u64 address;
    s64 instructionData;
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
    
    if(argc < 3)
    {
        OUT("Missing arguments\nUsage: debugger <path_to_executable> <working_directory> [<arguments>]\n");
        return -1;
    }
    
    char* programPath = argv[1];
    char* workingDirectoryPath = argv[2];
    char **arguments = 0;
    if(argc > 3)
    {
        arguments = argv + 3;
    }
    
    pid_t debuggeeProgramID = fork();
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
    else if(debuggeeProgramID >= 1)
    {
        bool debugging = true;
        Breakpoint breakpoints[16] = {};
        u32 breakpointCount = 0;
        while(debugging)
        {
            s32 status = 0;
            pid_t programID = waitpid(-1, &status, WNOHANG | WUNTRACED | WCONTINUED); 
            if(programID > 0)
            {
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
                    struct user_regs_struct registers;
                    if(ptrace(PTRACE_GETREGS, programID, &registers, &registers) != -1)
                    {
                        hitBreakpoint = FindBreakpointSetAt(registers.rip - 1, breakpoints, ARRAY_COUNT(breakpoints));
                    }
                    else
                    {
                        ERR_WITH_LOCATION_INFO("ERRNO:%d Failed to read instruction pointer. %s", errno, strerror(errno));
                    }
                    
                    OUT("RIP: %llx\n", registers.rip);
                    
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
                            if(AreStringsSame(commandString, (char*)"continue"))
                            {
                                if(hitBreakpoint)
                                {
                                    registers.rip--;
                                    s64 restoreInstructionTrap = (hitBreakpoint->instructionData & 0xFFFFFFFFFFFFFF00) | 0x00000000000000cc;
                                    
                                    if(ptrace(PTRACE_POKETEXT, programID, hitBreakpoint->address, hitBreakpoint->instructionData) != -1)
                                    {
                                        if(ptrace(PTRACE_SETREGS, programID, &registers, &registers) != -1)
                                        {
                                            if(ptrace(PTRACE_SINGLESTEP, programID, 0, 0) != -1)
                                            {
                                                if(waitpid(programID, &status, 0) != -1)
                                                {
                                                    ptrace(PTRACE_GETREGS, programID, &registers, &registers);
                                                    if(ptrace(PTRACE_POKETEXT, programID, hitBreakpoint->address, restoreInstructionTrap) != -1)
                                                    {
                                                    }
                                                    else
                                                    {
                                                        ERR_WITH_LOCATION_INFO("ERRNO:%d Failed to restore breakpoint at %"PRIu64" after stepping over it. %s", errno, hitBreakpoint->address, strerror(errno));
                                                    }
                                                }
                                                else goto breakpoint_stepover_failure;
                                            }else goto breakpoint_stepover_failure;
                                        }else goto breakpoint_stepover_failure;
                                    }
                                    else
                                    {
                                        breakpoint_stepover_failure: ERR_WITH_LOCATION_INFO("ERRNO:%d Failed to step over breakpoint at %"PRIu64". %s", errno, hitBreakpoint->address, strerror(errno));
                                    }
                                }
                                
                                commandContinue = true;
                                if(ptrace(PTRACE_CONT, programID, 0, 0) == -1)
                                {
                                    ERR_WITH_LOCATION_INFO("ERRNO:%d Failed to continue. %s", errno, strerror(errno));
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
                                        errno = 0;
                                        s64 instructionData = ptrace(PTRACE_PEEKTEXT, programID, breakpointAddress, 0);
                                        s64 originalInstructionData = instructionData;
                                        if(errno == 0)
                                        {
                                            instructionData = (instructionData & 0xFFFFFFFFFFFFFF00) | 0x00000000000000cc;
                                            if(ptrace(PTRACE_POKETEXT, programID, breakpointAddress, instructionData) != -1)
                                            {
                                                Breakpoint *b = breakpoints + breakpointCount++;
                                                b->address = breakpointAddress;
                                                b->instructionData = originalInstructionData;
                                            }
                                            else
                                            {
                                                ERR_WITH_LOCATION_INFO("ERRNO:%d Failed to set breakpoint at %s = %"PRIu64". %s", errno, addressString, breakpointAddress, strerror(errno));
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