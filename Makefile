BASE_DIR = .
SRC_DIR = $(BASE_DIR)/src

CFLAGS = -c -O2 -Wall
LDFLAGS =

SYSTEMD_LDFLAGS = -lsystemd

FILE_NAME = journalctl_regex

default: $(FILE_NAME)

$(FILE_NAME): $(FILE_NAME).o
	$(CC) $(FILE_NAME).o $(LDFLAGS) $(SYSTEMD_LDFLAGS) -o $(FILE_NAME)
	rm -f *.o

$(FILE_NAME).o: $(SRC_DIR)/$(FILE_NAME).c
	$(CC) $(CFLAGS) $(SRC_DIR)/$(FILE_NAME).c -o $(FILE_NAME).o

clean:
	rm -f *.o $(FILE_NAME)
