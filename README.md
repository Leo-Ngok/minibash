# Sshell

Sshell is a simple shell created as an end of term project for Computer Systems 2023 Class @SEM.

## Table of Contents

- [Minish](#minish)
  - [Table of Contents](#table-of-contents)
  - [Project Details](#project-details)
    - [Description](#description)
    - [Project Requirements](#project-requirements)
  - [Project Breakdown](#project-breakdown)
  - [Getting Started](#getting-started)
    - [Installation](#installation)
  - [Contact](#contact)

---

## Project Breakdown

Task # | Type | Short description
 ---: | --- | --- 
0 | **Mandatory** | See the [README](/README.md)
1 | **Mandatory** | Display prompt information and accept command that user types in.
2 | **Mandatory** | Execute the command.
3 | **Mandatory** | When the execution of the command is finished, display the prompt again and wait for next command.
4 | **Mandatory** | Terminate when the user types **"quit"**.
4.1 | ***Advanced*** | file redirection: **">"** , **"<"** and **">>"**, pipelining with **"\|"** 
5 | ***Advanced*** | Allow multiple commands to be executed in one line.
6 | ***Advanced*** | Execute the command in background when a **"&"** is appended to the command
7 | ***Advanced*** | Prevent the SHELL program from being interrupted by "CTRL-C"
8 | ***Advanced*** | BASH related features. such as: Use the **"tab" key** to autocomplete a command and keep a history of commands and use **up/down arrow keys** to re-use them.

## Getting Started

Using minish is as easy as 1-2-3! Simply clone this repository onto your local machine, compile with `./compile.sh` and run!

### Installation

1. Clone

Download this file at web learning or clone this repository onto your local machine.

2. Compile

On your machine, navigate (`cd`) to the newly created directory then open and run file `compile.sh` to compile and run shell.

3. Run shell

```sh
./minish
```

4. Run commands and enjoy!

5. To exit

```sh
quit
```

## Project Details

### Description

Sshell is a simple UNIX command line shell and was built as an end of final project for SEM. It includes many basic features present in the bash shell.

File Name | Description
 --- | ---
`main.cpp` | Main function for the shell.
`complile.sh` | Compile and run shell (with arrow key browsing/autocompletion with `tab`). **Require libreadline-dev** 
`complile_no_rdl.sh` | Compile and run shell
`defn.hpp` | Header file for all the functions.
`parse.cpp` | Parse the command line.
`eval.cpp` | Evaluate the command line.

### Project Requirements

- All your files will be compiled on Ubuntu 20.04 LTS
- All your files should end with a new line
- A `README.md` file, at the root of the folder of the project is mandatory
- Your code should use the same style as example.
- All your header files should be include guarded
- Unless specified otherwise, your program must have the exact same output as `sh` as long as the exact same error output.

### Implementations

Shell is basically an interpreter that eats user input in terminal or shell file, and spits out processes associated with it.

It is somehow similar to a compiler at its frontend, and a virtual machine at its backend.

Details of the frontend and backend are discussed separately.

### Frontend

It basically eats lines of inputs from users. When a line of input is read,

- A stream of tokens are made by a tokenizer.
- A syntax tree is created by a parser.

i) Tokenizer

The tokenizer simply separates the reserved keywords from normal inputs, creating `tokens` . 

For instance, `ls -lr` is decomposed into tokens: `ls` and  `-lr`.

The tokens are decomposed by recognizing special reserved keywords, and putting the buffered input to a list.

As with a normal bash, **' ... '** and **"..."** to create string arguments with special tokens, overriding the predefined tokens defined in shell.

Warning: Currently, only one line of input is supported, except for the case when escape characters are used. For example, 

```
echo '123
456' test
```

yields

```
123
456 test
```

However, 

```
echo 123456 test |
cat
```

is not possible, since whether or not the token `|`  needs token succeeding it is completely determined by the parser. 

ii) Parser

Based on the tokens provided by tokenizer, the parser converts the token stream into structured data units, these are:

- Command
- And Or statment
- Pipeline statement
- redirections
- list of arguments associated with pipeline statements

These data units are put in ascending order of operator precedences.

The grammar that defines the syntax has made to be similar to LL(1), and currently, tokens are parsed recursively in top-down manner, resembling a recursive descent parser.

As it resembles a recursive descent parser, a list of functions `prepare_*` are implemented to recognize the data units above.

These features are implemented in `parse.cpp`.

### Backend

It basically traverses the syntax tree and runs the code defined by the internal nodes accordingly.

It executes the command sequentially. 

- For a foreground command, it is put in the main thread;

- for a background command, a new thread is created and all executions are run under the newly created thread.

  The shell captures return status of a pipeline by tracking the process ID (pid) of the subprocesses created, and use `WIFEXITSTATUS` to examine the return code. Once the return code is obtained, the next and_or_statment is run base on conjunctions defined.

For each pipeline included in an and_or statement, all pipe file descriptors are created before  actually running the pipeline. 

Once the child process associated with a pipeline element is created, the child process redirect its standard input/output first, and closes all pipe file descriptors.

It then opens file descriptors defined by `>, <, >>` , and finally replaces the process instance by `execvp`.

For the parent process, each time when a child process associated with a pipeline element is created, its pipeline file descriptors associated with that element are closed.

The parent process then waits for all pipeline elements to terminate, and obtain the exit status of the last pipe element.

These features are implemented in `eval.cpp`.

### Discussions

Note that variables are yet to be supported in the future release, and it is not included currently, since implementing symbol tables and expanding them in the arguments should go to compiler construction class, not system classes.

Implementations of parser and tokenizer can be upgraded, by using a bottom-up parser generated by `bison` and a tokenizer `flex`. In this way, more syntactic features of shell could be supported.

It is not that easy to capture keyboard arrow keys before the enter key is pressed. In particular, it is thought that in windows platform, it is even not possible! 

To resolve this issue, `libreadline` is used. It captures arrow keys and tab keys, enabling the browsing of history inputs and autocompletion of commands. However, it is platform dependent, and might not be supported by all Linux distributions, so two compiling scripts are defined.  

### Appendix

Reserved keywords supported by this version of shell (though features of some tokens are not implemented).

## Contact

Thank you for your time. If you have any questions, please feel free to contact me via `liangzc21@mails.tsinghua.edu.cn`. I might update this repository in the future, so star this repository to keep track of it.

Please enjoy!

