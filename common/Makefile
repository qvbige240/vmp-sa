ifndef TOP
	TOP = ..
endif

include $(TOP)/make.include

DIRS = libevent threadpool

.PHONY:	all clean $(DIRS)

all: $(DIRS)

$(DIRS):
	@if test -d $@; then cd $@ ; $(MAKE) MODULE_NAME=m_common all ; fi

clean:
	@for dir in $(DIRS) ; do \
		if test -d $$dir ; then \
			echo "$$dir: $(MAKE) $@" ; \
			if (cd $$dir; $(MAKE) $@) ; then true; else exit 1; fi; \
		fi \
	done;
