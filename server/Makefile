#Makefile
OBJS = Dropbox_Server.o list.o
SOURCE = Dropbox_Server.cpp list.cpp 
HEADER = list.h 
OUT = dropbox_server
FLAGS = -g -c

# -g option enables debugging mode
# -c flag generates object code for separate files

all: $(OBJS)
	g++ -g Dropbox_Server.o list.o -o $(OUT)

# create / compile the individual files separately
Dropbox_Server.o: Dropbox_Server.cpp
	$(CC) $(FLAGS) Dropbox_Server.cpp

list.o: list.cpp
	$(CC) $(FLAGS) list.cpp


# cleaning
clean:
	rm -f $(OBJS) $(OUT)

# accounting
count:
	wc $(SOURCE) $(HEADER)
