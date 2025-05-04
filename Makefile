# Makefile for the UNIX Treasure Hunt Game

CC = gcc
CFLAGS = -Wall -Wextra -O2
TARGETS = treasure_manager treasure_hub

all: $(TARGETS)

treasure_manager: treasure_manager.c
	$(CC) $(CFLAGS) -o $@ $<

treasure_hub: treasure_hub.c
	$(CC) $(CFLAGS) -o $@ $<

clean:
	rm -f $(TARGETS)

.PHONY: all clean
