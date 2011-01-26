LEVEL =

include $(LEVEL)makeinclude

#########################

DIRS = API Misc Module Unit

APIobj = obj/*.o

LIBRARIES = lib/libcalm.a

recur: 	$(LEVEL)makeinclude
	@echo -- compiling:
	@for dir in $(DIRS); do\
		echo "    $$dir/";\
		(cd $$dir; $(MAKE) $(MFLAGS);) || break;\
	done
	@echo
	make $(LIBRARIES)
	@echo done

all:	recur

lib/libcalm.a: $(APIobj) 
	@echo -- making library --
	$(AR) -r $@ $(APIobj)
	@echo -- running ranlib --
	ranlib $(LIBRARIES)

clean:
	@echo -- cleaning:
	@for dir in $(DIRS); do\
		echo "    $$dir/";\
		(cd $$dir; $(MAKE) rclean) || break;\
	done
	@echo
	@echo -- removing libraries and objects --
	$(RM) $(LIBRARIES)
	$(RM) $(LO)*.o || break;
	@echo done

clock:
	$(TOUCH) Makefile
	$(TOUCH) makeinclude
	@echo -- touching:
	@for dir in $(DIRS); do\
		echo "    $$dir/";\
		(cd $$dir; $(TOUCH) Makefile) || break;\
	done
	@echo
