#include "defn.hpp"
#include <algorithm>
#include <iostream>
#include <sstream>
#include <unistd.h>
#include <cstdlib>
#ifdef HAS_LIBREADLINE
#include <readline.h>
#endif
void cond_push(args_t &args, std::string & token) {
    if(!token.empty()) 
        args.push_back(token);
    token = "";
}
std::string ln_read;
int take_char(void) {
    //static int k = 0;
    static std::string in_buffer = "";
    /*if(k == in_buffer.size()) {
        std::getline(std::cin, in_buffer);
        k = 0;
    }
    return in_buffer[k++];*/
    if(in_buffer.empty()) {
#ifdef HAS_LIBREADLINE
        char *in_buf_cstr = readline(ln_read.empty() ? "minish $ " : "");
        if(!in_buf_cstr) {
            exit(0);
            return EOF;
        }
        in_buffer = in_buf_cstr;
        delete [] in_buf_cstr;
#else
        std::getline(std::cin, in_buffer);
        if(std::cin.eof()) {
            exit(0);
            return EOF;
        }
#endif
        in_buffer += '\n';

        ln_read += in_buffer;
    }
    int res = in_buffer.front();
    in_buffer.erase(0,1);
    return res;
}

void tokenize(/*std::string buffer, */args_t & args) {
    std::string token;
    /*std::stringstream inss(buffer);
    while(std::getline(inss, token, ' ')) {
        if(!token.empty())
        args.push_back(token);
    }
    int bg = 0;
    if(args.back() == "&") {
        bg = 1;
        args.pop_back();
    }
    return bg;*/
    int c = take_char(); // std::cin.get(); //getchar();
    bool fin = false;
    //std::string token;
    while(c > 0 && c < 128) { 
        if(std::cin.eof())
            break;
        bool use_read = false;
        switch (c) {
        case 0x4:
            exit(0);
        case '\n':{
            cond_push(args, token);
            fin = true;
            break;
        } case '#': {
            cond_push(args, token);
            do c = take_char(); // std::cin.get(); // getchar();
            while(c != '\n' && !std::cin.eof());
            fin = true;
            break;
        } case '\'': {
            do {
                c = take_char(); // std::cin.get(); //getchar();
                token += c;
                if(c == '\n') std::cout << "> ";
                flush(std::cout);
            } while (c != '\'' && !std::cin.eof());
            token.pop_back();
            break;
        } case '\"': {
            do {
                c = take_char(); // std::cin.get(); //getchar();
                if(!(c == '\n' || c == '\"' || c == '`'|| c == '$')) {
                    token += '\\';
                }
                token += c;
                if(c == '\n')
                    std::cout << "> ";
            } while(c != '\"' && !std::cin.eof());
            token.pop_back();
            break;
        } case '\\': {
            c = take_char(); // std::cin.get(); //getchar();
            token += c;
            break;
        }
        case '!': {
            cond_push(args, token);
            token = c;
            cond_push(args, token);
            break;
        }
        case '\t':
        case ' ': {
            cond_push(args, token);
            break;
        }
        case ';': {
            cond_push(args, token);
            c = take_char(); // std::cin.get(); //getchar();
            if(c == ';') {
                token = ";;";
            } else {
                token = ";";
                use_read = true; // 字符返还
            }
            cond_push(args, token);
            break;
        }
        case '|': {
            cond_push(args, token);
            c = take_char(); // std::cin.get(); //getchar();
            if(c == '|') {
                token = "||";
            } else {
                token = "|";
                use_read = true; // 字符返还
            }
            cond_push(args, token);
            break;
        }
        case '&': {
            cond_push(args, token);
            c = take_char(); // std::cin.get(); //getchar();
            if(c == '&') {
                token = "&&";
            } else {
                token = "&";
                use_read = true; // 字符返还
            }
            cond_push(args, token);
            break;
        }
        case '<': {
            cond_push(args, token);
            c = take_char(); // std::cin.get(); //getchar();
            if(c == '<') {
                c = take_char(); // std::cin.get(); //getchar();
                if(c == '-'){
                    token = "<<-";
                } else {
                    token = "<<";
                    use_read = true;
                }
            } else if(c == '>') {
                token = "<>";
            } else if(c == '&') {
                token = "<&";
            }else {
                token = "<";
                use_read = true; // 字符返还
            }
            cond_push(args, token);
            break;
        }
        case '>': {
            cond_push(args, token);
            c = take_char(); // std::cin.get(); //getchar();
            if(c == '>') {
                token = ">>";
            } else if(c == '|') {
                token = ">|";
            } else if(c == '&') {
                token = ">&";
            }else {
                token = ">";
                use_read = true; // 字符返还
            }
            cond_push(args, token);
            break;
        }
        default:
            token += c;
            break;
        }
        if(fin) break;
        if(!use_read) c = take_char(); // std::cin.get(); //getchar();
    }
    cond_push(args, token);
    /*for(auto && arg: args) {
        std::cout << arg << std::endl;
    }*/
}

bool is_number(const std::string& s) {
    return !s.empty() && std::find_if(s.begin(), 
        s.end(), [](unsigned char c) { return !std::isdigit(c); }) == s.end();
}

void prepare_pipelines(args_t & args, pipelines_t & pipelines);
void prepare_pipeline(args_t args, pipeline_t & pipeline) ;
void prepare_andors(args_t &args, andors_t & andors);
void prepare_command(args_t &args, commands_t & commands) {
    commands.clear();
    args_t temp_args;
    command_t cmd_elem;
    for(int k = 0; k < args.size(); ++k) {
        std::string arg = args[k];
        if(arg == "&") {
            cmd_elem.background = true;
            prepare_andors(temp_args, cmd_elem.and_ors);
            //prepare_pipelines(temp_args, cmd_elem.pipelines);
            commands.push_back(cmd_elem);
            temp_args.clear();
        } else if(arg == ";") {
            cmd_elem.background = false;
            prepare_andors(temp_args, cmd_elem.and_ors);
            //prepare_pipelines(temp_args, cmd_elem.pipelines);
            commands.push_back(cmd_elem);
            temp_args.clear();
        } else {
            temp_args.push_back(arg);
        }
    }
    if(!temp_args.empty()) {
        cmd_elem.background = false;
        prepare_andors(temp_args, cmd_elem.and_ors);
        prepare_pipelines(temp_args, cmd_elem.pipelines);
        commands.push_back(cmd_elem);
        temp_args.clear();
    }
}


void prepare_andors(args_t &args, andors_t & andors) {
    if(args.empty()) {
        std::cerr << "-minish: syntax error" << std::endl;
        return;
    }
    andors.clear();
    args_t temp_args;
    //command_t cmd_elem;
    andor_t andor_elem;
    for(int k = 0; k < args.size(); ++k) {
        std::string arg = args[k];
        if(arg == "&&") {
            andor_elem.opt = AndOrOpt::AND;
            prepare_pipelines(temp_args, andor_elem.pipelines);
            andors.push_back(andor_elem);
            temp_args.clear();
        } else if(arg == "||") {
            andor_elem.opt = AndOrOpt::OR;
            prepare_pipelines(temp_args, andor_elem.pipelines);
            andors.push_back(andor_elem);
            temp_args.clear();
        } else {
            temp_args.push_back(arg);
        }
    }
    if(!temp_args.empty()) {
        andor_elem.opt = AndOrOpt::UNCOND;
        prepare_pipelines(temp_args, andor_elem.pipelines);
        andors.push_back(andor_elem);
        temp_args.clear();
    }
}

void prepare_pipelines(args_t & args, pipelines_t & pipelines) {
    pipelines.clear();
    args_t __arg_block;
    redirs_t redirs;
    pipeline_t pipe;
    for(int k = 0 ; k < args.size(); ++k) {
        std::string arg = args[k];
        if(arg == "|"){
            prepare_pipeline(__arg_block, pipe);
            pipelines.push_back(pipe);
            __arg_block.clear();
        } else {
            __arg_block.push_back(arg);
        }
    }
    if(!pipelines.empty() && __arg_block.empty()) {
        std::cerr 
        << "Syntax error: Syntax for pipeline is (pipe `|`)* pipe" 
        << std::endl;
    }
    if(!__arg_block.empty()) {
        prepare_pipeline(__arg_block, pipe);
        pipelines.push_back(pipe);
    }
}

void prepare_pipeline(args_t args, pipeline_t & pipeline) {
    pipeline.stdin_redir = false;
    pipeline.stdout_redir = false;
    pipeline.args.clear();
    pipeline.redirs.clear();
    pipeline.stdin_refd = -1;
    pipeline.stdout_refd = -1;
    pipeline.invert_res = false;
    args_t pipeline_arg;
    redirs_t redirs;
    for(int k = 0; k < args.size(); ++k) {
        std::string arg = args[k];
        if(arg == ">") {
            redir_t redir;
            if(is_number(pipeline_arg.back())) {
                redir.fd = atoi(pipeline_arg.back().c_str());
                pipeline_arg.pop_back();

            } else redir.fd = 1; // fd of stdout
            redir.mode = RedirMode::OUT;
            redir.filename = args[k+1];
            k++;
            //std::cout <<"redirfd of pipeline" << k << " is: " << redir.fd << std::endl;;
            pipeline.stdin_redir |= (redir.fd == STDIN_FILENO);
            pipeline.stdout_redir |= (redir.fd == STDOUT_FILENO);
            redirs.push_back(redir);
        } else if(arg == "<") {
            redir_t redir;
            if(is_number(pipeline_arg.back())) {
                redir.fd = atoi(pipeline_arg.back().c_str());
                pipeline_arg.pop_back();

            } else redir.fd = 0; // fd of stdin
            redir.mode = RedirMode::IN;
            redir.filename = args[k+1];
            k++;
            //std::cout <<"redirfd of pipeline" << k << " is: " << redir.fd << std::endl;;
            pipeline.stdin_redir |= (redir.fd == STDIN_FILENO);
            pipeline.stdout_redir |= (redir.fd == STDOUT_FILENO);
            redirs.push_back(redir);
        } else if(arg == ">>") {
            redir_t redir;
            if(is_number(pipeline_arg.back())) {
                redir.fd = atoi(pipeline_arg.back().c_str());
                pipeline_arg.pop_back();

            } else redir.fd = 1; // fd of stdout
            redir.mode = RedirMode::APPEND;
            redir.filename = args[k+1];
            k++;
            //std::cout <<"redirfd of pipeline" << k << " is: " << redir.fd << std::endl;;
            pipeline.stdin_redir |= (redir.fd == STDIN_FILENO);
            pipeline.stdout_redir |= (redir.fd == STDOUT_FILENO);
            redirs.push_back(redir);
        } else if(arg == "!") {
            pipeline.invert_res = true;

        } else{
            pipeline_arg.push_back(arg);
        }
    }
    pipeline.args = pipeline_arg;
    pipeline.redirs = redirs;
    //std::cout << "pipeline has redir stdin? "<<pipeline.stdin_redir << std::endl;
    //std::cout << "pipeline has redir stdout? "<<pipeline.stdout_redir << std::endl;
}

std::ostream& operator<<(std::ostream& os, command_t cmd) {
    os << "Begin command" << std::endl;
    os << "Type: " << (cmd.background ? "Background" : "Foreground") << std::endl;
    for(auto &&andor_elem : cmd.and_ors) {
        os << andor_elem << std::endl;
    }
    os << "End command" << std::endl;
    return os;
}
std::ostream& operator<<(std::ostream& os, andor_t andor_elem) {
    os << "    Begin and_or block" << std::endl;
    os << "    Type: ";
    switch(andor_elem.opt) {
        case AndOrOpt::AND:
        os << "AND"; break;
        case AndOrOpt::OR:
        os << "OR"; break;
        case AndOrOpt::UNCOND:
        os << "UNCOND"; break;
    } 
    os << std::endl;
    for(auto && pipe : andor_elem.pipelines) {
        os << pipe << std::endl;
    }
    os << "    End and_or block" << std::endl;
    return os;
}
std::ostream& operator<<(std::ostream& os, pipeline_t pipe) {
    os << "        Begin pipeline" << std::endl;
    os << "            STDIN fd: " << pipe.stdin_refd << std::endl;
    os << "            STDOUT fd: " << pipe.stdout_refd << std::endl;
    for( auto && arg: pipe.args) {
        os << "            arg: [" << arg << "]"<< std::endl;
    }
    for( auto && arg: pipe.redirs) {
        os << arg << std::endl;
    }
    os << "        End pipeline" << std::endl;
    return os;
}
std::ostream& operator<<(std::ostream& os, redir_t arg) {
    os << "        Redirect fd: "<< arg.fd << " filename: " << arg.filename << std::endl;
    return os;
}
