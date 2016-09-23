CC ?= gcc

CFLAGS ?= -W -pedantic -Werror -Wall -std=gnu99 \
	-fno-strict-aliasing -fno-common -Wno-unused-parameter \
	-fstrict-aliasing -fstrict-overflow -fdiagnostics-color=always \
	-Wno-return-local-addr

prefix ?= /usr

SDIR = src
ODIR = bin

STARGET ?= serv
CTARGET ?= cli

LIBS = -lpthread
