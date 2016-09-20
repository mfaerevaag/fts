CC ?= gcc

CFLAGS ?= -W -pedantic -Werror -Wall -std=gnu99 \
	-fno-strict-aliasing -fno-common -Wno-unused-parameter \
	-fstrict-aliasing -fstrict-overflow -fdiagnostics-color=always \
	-Wno-return-local-addr

prefix ?= /usr

SDIR = src
ODIR = bin
SRC  = $(wildcard $(SDIR)/*.c)
OBJ  = $(patsubst $(SDIR)/%.c, $(ODIR)/%.o, $(SRC))

STARGET ?= $(ODIR)/server
CTARGET ?= $(ODIR)/client
