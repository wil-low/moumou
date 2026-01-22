######################################################################
#  Project
######################################################################

BINARY		= moumou
SRCFILES	= \
	main.c \


$(BINARY): $(SRCFILES)
	gcc -Wextra -Wall -std=c99 -O0 -g -o $(BINARY) $(SRCFILES)

run: $(BINARY)
	./$(BINARY)

clean:
	rm $(BINARY)
