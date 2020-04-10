#Compile source file and produce executable
all : sort.o
	cc -o sort sort.c sort.h -pthread

#Remove object and temp files
clean :
	rm sort *.o
