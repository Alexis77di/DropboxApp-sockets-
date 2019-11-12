#Makefile
OBJS = Dropbox_Client.o list.o
SOURCE = Dropbox_Client.cpp list.cpp 
HEADER = list.h 
OUT = dropbox_client
FLAGS = -g -c

# -g option enables debugging mode
# -c flag generates object code for separate files

all: $(OBJS)
	g++ -g Dropbox_Client.o list.o -lpthread -o $(OUT)

# create / compile the individual files separately
Dropbox_Client.o: Dropbox_Client.cpp
	$(CC) $(FLAGS) Dropbox_Client.cpp

list.o: list.cpp
	$(CC) $(FLAGS) list.cpp


# cleaning
clean:
	rm -f $(OBJS) $(OUT)

# accounting
count:
	wc $(SOURCE) $(HEADER)
