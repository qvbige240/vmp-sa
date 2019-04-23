TOP=.
include $(TOP)/env.conf
include $(TOP)/make.include

DIRS = vmp common mw bll

APP_DIR =  app
ALL_DIRS = $(DIRS) $(APP_DIR)

.PHONY:	all clean dis  $(ALL_DIRS)

all: version $(ALL_DIRS)

SHELL := /bin/bash
version:
	@echo "DEPEND_DIR: $(DEPEND_DIR)"
	@if test ! -d $(TOP)/lib ; then \
		mkdir -p $(TOP)/lib; \
	fi
	@if test ! -d $(TOP)/bin ; then \
		mkdir -p $(TOP)/bin; \
	fi

$(ALL_DIRS):
	@if test -d $@ ; then cd $@ ; $(MAKE) all ; fi

dis: all
	$(MAKE) -C $(APP_DIR) $@

clean:
	@for dir in $(ALL_DIRS) ;  do \
		if test -d $$dir ; then \
			echo "$$dir: $(MAKE) $@" ; \
			if (cd $$dir; $(MAKE) $@) ; then \
				true; \
			else \
				exit 1; \
			fi; \
		fi \
	done
