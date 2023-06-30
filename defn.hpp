#ifndef DEFN_H
#define DEFN_H
#include <string>
#include <vector>
#include <algorithm>
#include <iostream>
#include <signal.h>

//#define HAS_LIBREADLINE

#define WAIT_PARENT_FD SIGUSR1


struct redir_t;
struct pipeline_t;
struct command_t;
struct andor_t;
typedef std::vector<std::string> args_t;
typedef std::vector<redir_t> redirs_t;
typedef std::vector<pipeline_t> pipelines_t;
typedef std::vector<command_t> commands_t;
typedef std::vector<andor_t> andors_t;
void eval(/*std::string line*/);
void tokenize(/*std::string buffer, */args_t & args);
void prepare_command(args_t &args, commands_t & cmds);
extern bool control_proc;
extern int pipefd[128]; 
extern sighandler_t default_interrupt, default_can_start;
extern std::vector<pid_t> processes;
void wait_parent_close_fd(int sig);

void signal_all(int sig);

extern bool can_start;
extern std::string ln_read;

struct command_t {
    bool background;
    andors_t and_ors;
    pipelines_t pipelines;
};

enum class AndOrOpt {
    UNCOND, AND, OR
};

struct andor_t {
    AndOrOpt opt;
    pipelines_t pipelines;
};

struct pipeline_t {
    args_t args;
    redirs_t redirs;
    bool stdin_redir;
    bool stdout_redir;
    int stdin_refd;
    int stdout_refd;
    bool invert_res;
};

enum class RedirMode {
    IN, OUT, APPEND
};

struct redir_t {
    RedirMode mode;
    int fd;
    std::string filename;
};



std::ostream& operator<<(std::ostream& os, command_t);
std::ostream& operator<<(std::ostream& os, andor_t);
std::ostream& operator<<(std::ostream& os, pipeline_t);
std::ostream& operator<<(std::ostream& os, redir_t);
//std::ostream& operator<<(std::ostream& os, command_t);





// utilities 
bool is_number(const std::string& s); 

#endif // DEFN_H