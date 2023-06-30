#include <iostream>
#include <string>
#include <cstdlib>
#include <signal.h>
#include "defn.hpp"
#ifdef HAS_LIBREADLINE
#include <readline.h>
#include <history.h>
#endif
std::string buffer;
bool control_proc;
sighandler_t default_interrupt, default_can_start;
bool can_start;
bool idle;
void sigint_handler(int sig);
/*void init_pipefd() {
    for(int i = 0; i < 128; ++i) {
        pipefd[i] = i + 3;
    }
}*/

int main(void) {
    control_proc = true;
    default_interrupt = signal(SIGINT, sigint_handler);
    while(true) {
        if(!control_proc) {
            break;
        }

    prepare:
        if(!ln_read.empty()) {
#ifdef HAS_LIBREADLINE
            ln_read.pop_back();
            add_history(ln_read.c_str());
#endif
            ln_read.clear();
        }
#ifndef HAS_LIBREADLINE
        std::cout << "minish $ ";
        std::flush(std::cout);
#endif
        //idle = true;
        //std::getline(std::cin, buffer);
        //idle = false;
        /*if(std::cin.eof()) {
            std::cout << std::endl;
            exit(0);
        }*/
        eval(/*buffer*/);
    }
    return 0;
}

void signal_all(int sig) {
    for(auto && proc : processes) {
        //std::cout << "Signalling " << proc << std::endl;
        kill(proc, sig);
    }
}

void sigint_handler(int sig) {
    // std::cout << "Signal Interrupt (Ctrl+C) detected " << std::endl;
    //goto prepare;
    signal_all(sig);
    if(idle) {
        std::cout<<std::endl;
#ifndef HAS_LIBREADLINE
        std::cout << "minish $ ";
        std::flush(std::cout);
#endif
    }
}

void wait_parent_close_fd(int sig) {
    //std::cout << "parent has closed related fd" << std::endl;
    if(sig == WAIT_PARENT_FD) {
        can_start = true;
    }
}