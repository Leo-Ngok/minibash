#include "defn.hpp"
#include <cstdlib>
#include <unistd.h> // execve, fork
#include <cstring>
#include <iostream>
#include <sys/wait.h> // waitpid
#include <fcntl.h>
#include <cstdio>
#include <sys/stat.h>
#include <cassert>
#include <pthread.h>

#define PERMIT_ALL (S_IRUSR|S_IWUSR)
static int builtin(args_t & args) {
    if(args[0] == "quit")
        exit(0);
    return 0;
}

std::vector<command_t> commands;
std::vector<pid_t> processes;

/*void make_dir(char *path) {
    struct stat st = {0};

    if (stat(path, &st) == -1) {
        mkdir(path, 0700);
    }
}*/
void create_pipes(pipelines_t & pipelines)  {
    size_t n = pipelines.size() - 1;
    for(int k = 0 ; k < n; ++k) {
        if(pipelines[k].stdout_redir || pipelines[k+1].stdin_redir) {
            continue;
        }
        int pipefd[2];
        int _ = pipe(pipefd);
        pipelines[k].stdout_refd  = pipefd[1]; //prefd;
        pipelines[k+1].stdin_refd = pipefd[0]; // succfd;
    }
    
}

char **prepare_argv(args_t & args) {
    char ** __argv = new char *[args.size()+1];
    int k = 0;
    for (auto && arg : args) {
        __argv[k] = new char[arg.size() + 2];
        memset(__argv[k], 0, sizeof(char) * (arg.size() + 2));
        strncpy(__argv[k], arg.c_str(), arg.size());
        k++;
    }
    __argv[args.size()] = nullptr;
    return __argv;
}

int configure_fd(pipelines_t & pipelines, pipeline_t & pipe) {
    int flags = 0;
    if(pipe.stdin_refd > 0) {
        dup2(pipe.stdin_refd, STDIN_FILENO);
        //dup2(STDIN_FILENO, pipe.stdin_refd);
    }
    if(pipe.stdout_refd > 0) {
        dup2(pipe.stdout_refd, STDOUT_FILENO);
    }
    for(auto &&pp : pipelines ) {
       if(pp.stdin_refd > 0) {
        close(pp.stdin_refd);
       }
       if(pp.stdout_refd > 0) {
        close(pp.stdout_refd);
       }
    }
    for(auto && rdir : pipe.redirs) {
        int oflag = 0;
        int mode_flg = 0;
        switch(rdir.mode) {
            case RedirMode::IN: oflag = O_RDONLY; /*mode_flg = S_IREAD; */ break;
            case RedirMode::OUT: oflag = O_WRONLY|O_CREAT|O_TRUNC; mode_flg = S_IWRITE; break;
            case RedirMode::APPEND: oflag = O_WRONLY|O_CREAT|O_APPEND; mode_flg = S_IWRITE; break;
            default: assert(false);
        }
        int fd_new = open(rdir.filename.c_str(), oflag, mode_flg);

        if(fd_new < 0) {
            std::cerr << "Open file failed with code " << errno << std::endl;
            perror("");
            flags=-1;
        } else {
            dup2(fd_new, rdir.fd);
        }
    }
    return flags;
}
void *execute_command(void *__cmd);
void eval(/*std::string buffer*/) {
    //if(buffer.empty())
    //    return;
    args_t argv;
    pid_t pid;
    tokenize(/*buffer,*/ argv);
    if(argv.size() == 0) {
        return; // empty lines
    }
    
    // step one : parse the input line.
    prepare_command(argv, commands);

    // step two: execute the commands

    for(auto && cmd: commands) {
        command_t *bgcmd = new command_t(cmd);
        if(cmd.background) {
            pthread_t tid;
            pthread_create(&tid, nullptr, execute_command, bgcmd);
        } else {
            execute_command(bgcmd);
        }
        
    }
    commands.clear();
}
void *execute_command(void *__cmd) {
    command_t *cmd = (command_t *)__cmd;
    //std::cout << "Executing a command. " << std::endl;
    std::vector<pid_t> *local_processes = (cmd->background) ? new std::vector<pid_t>() : &processes;
    local_processes->clear();
    for(auto && andor_elem : cmd->and_ors) {
        create_pipes(andor_elem.pipelines);
        for(auto && pipe: andor_elem.pipelines) {
            if(!builtin(pipe.args)) {
                pid_t pid;
                int n = pipe.args.size();
                if((pid = fork()) == 0) {
                    control_proc = false;
                    signal(SIGINT, default_interrupt);
                    int res = configure_fd(cmd->pipelines, pipe);
                    if(res < 0) {
                        exit(errno);
                    }
                    
                    char **argv = prepare_argv(pipe.args);
                    if(execvpe(argv[0], argv, environ)<0) {
                        std::cerr << "Attempt to execute " << argv[0] << ", however it fails with code " << errno << std::endl;
                        perror("");
                        exit(errno);
                    }
                } else {
                    // parent process
                    local_processes->push_back(pid);
                    close(pipe.stdin_refd);
                    close(pipe.stdout_refd);
                }
            }
        }
        int count = 0;
        int status;
        int exit_pid;
        int exit_code;
        while((exit_pid = wait(&status)) > 0) {
            count++;
        //    std::cout << "Process " << exit_pid << " exited with " << WEXITSTATUS(status) << std::endl;
            if(exit_pid == local_processes->back()) {
                exit_code = WEXITSTATUS(status);
                if(andor_elem.pipelines.back().invert_res) {
                    exit_code = !exit_code;
                }
            }
        }
        local_processes->clear();
        if(cmd->background) {
            delete local_processes;
        }
        if(andor_elem.opt == AndOrOpt::AND && exit_code)
            break;
        if(andor_elem.opt == AndOrOpt::OR && !exit_code)
            break;
    }
        
       // if(!cmd.background) {
    //int count = 0;
    //int status;
    //int exit_pid;
    //while((exit_pid = wait(&status)) > 0) {
   //     count++;
    //    std::cout << "Process " << exit_pid << " exited with " << WEXITSTATUS(status) << std::endl;
    //}
    //std::cout << "Note: reaped " << count << " processes." << std::endl;
    //std::cout << "Process created: " << std::endl;
    //for(auto && proc : processes) {
    //    std::cout << "Process pid " << proc << std::endl;
    //}
    /*if(count >= processes.size()) {
        // succeed
        std::cout << "All process terminated successfully." << std::endl;
    } else {
        // error
        std::cerr << "Process terminated, however with error (code " << errno << "). " << std::endl;
    }*/
      //  } else {
      //      std::cout << "[1] " << processes.front() << std::endl;
     //   }
    delete cmd;
    return nullptr;
}