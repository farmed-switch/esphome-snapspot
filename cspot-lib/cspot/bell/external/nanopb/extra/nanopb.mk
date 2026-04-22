

NANOPB_DIR := $(patsubst %/,%,$(dir $(patsubst %/,%,$(dir $(lastword $(MAKEFILE_LIST))))))

NANOPB_CORE = $(NANOPB_DIR)/pb_encode.c $(NANOPB_DIR)/pb_decode.c $(NANOPB_DIR)/pb_common.c

ifdef windir
WINDOWS = 1
endif
ifdef WINDIR
WINDOWS = 1
endif

ifneq "$(wildcard $(NANOPB_DIR)/generator-bin)" ""

	PROTOC = $(NANOPB_DIR)/generator-bin/protoc
	PROTOC_OPTS =
else

	PROTOC_OPTS =
	ifdef WINDOWS
	    PROTOC = python $(NANOPB_DIR)/generator/protoc
	else
	    PROTOC = $(NANOPB_DIR)/generator/protoc
	endif
endif

%.pb.c %.pb.h: %.proto $(wildcard %.options)
	$(PROTOC) $(PROTOC_OPTS) --nanopb_out=. $<

