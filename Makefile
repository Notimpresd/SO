CC = gcc
CFLAGS = -Wall -Wextra -std=c11

all: treasure_manager treasure_hub score_calculator

treasure_manager: treasure_manager.c
	$(CC) $(CFLAGS) -o treasure_manager treasure_manager.c

treasure_hub: treasure_hub.c
	$(CC) $(CFLAGS) -o treasure_hub treasure_hub.c

score_calculator: score_calculator.c
	$(CC) $(CFLAGS) -o score_calculator score_calculator.c

clean:
	rm -f treasure_manager treasure_hub score_calculator
	rm -rf logged_hunt-* monitor_cmd
