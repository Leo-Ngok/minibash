g++ -c main.cpp -O2 #-g #-I/usr/include/readline
g++ -c eval.cpp -O2 #-g
g++ -c parse.cpp -O2 #-g -I/usr/include/readline
g++ -o minish main.o eval.o parse.o -pthread #-g  #-lreadline
