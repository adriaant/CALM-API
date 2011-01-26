LEVEL = ../

include $(LEVEL)makeinclude

#########################

SRCS = `ls *.cpp`

all:
	@for dir in $(SRCS); do\
		echo "        $$dir";\
		$(CC) $(OPTIONS) $(INCLUDE_DIR) -c $$dir;\
		mv *.o $(LO);\
	done

clean:
	-$(RM) *~

rclean:
	$(MAKE) clean

clock:
	$(TOUCH) *
