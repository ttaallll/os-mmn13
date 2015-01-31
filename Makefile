
FLAGS = -g -Wall  -L./ -m32

all: clean my_dir my_cd

my_dir: my_dir.c
	gcc ${FLAGS} common.c my_dir.c -o my_dir -lm

my_cd: my_cd.c 
	gcc ${FLAGS} common.c my_cd.c -o my_cd -lm
	
clean:
	rm -f my_cd 

