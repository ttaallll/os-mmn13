
FLAGS = -Wall  -L./ -m32

all: clean my_cd 

my_cd: my_cd.c 
	gcc ${FLAGS} my_cd.c -o my_cd -lm
	
clean:
	rm -f my_cd 

