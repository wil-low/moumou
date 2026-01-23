######################################################################
#  Project
######################################################################

BINARY		= moumou
SRCFILES	= \
	main.c \
	core.c \
	ui.c \
	ai.c \

HDRFILES	= \
	config.h \
	core.h \
	ui.h \
	ai.h \

$(BINARY): $(SRCFILES) $(HDRFILES)
	gcc -Wextra -Wall -std=c99 -O0 -g -o $(BINARY) $(SRCFILES)

run: $(BINARY)
	./$(BINARY)

clean:
	rm $(BINARY)
