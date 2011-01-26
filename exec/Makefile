LEVEL =

include $(LEVEL)makeinclude

#########################

EXEC = ../calm

OBJS = $(LO)*.o

# uncomment or change lines in this block to set up your own simulation executable
  # offline training and testing example
	#SRCS = Main.cpp SampleOffline.cpp
  # online training and testing example, demonstration of AnalysisTools
	#SRCS = Main.cpp SampleOnline.cpp
  # feedback sample
	#SRCS = Main.cpp SampleFeedback.cpp
  # training of multiple sequences
	SRCS = Main.cpp MultiSeqSample.cpp MultiSequence.cpp
# do not change other lines

$(EXEC): makeinclude
	@echo -- compiling:
	@for src in $(SRCS); do\
		echo "    $$src";\
		$(CC) $(OPTIONS) $(INCLUDE_DIR) -c $$src;\
		mv *.o $(LO);\
	done
	@echo
	@echo -- making $(EXEC) --
	$(CC) -g $(OPTIONS) $(OBJS) $(LIBDIRS) $(LIBS) -o $@
	@echo -- installing $(EXEC) --
	mv -f $(EXEC) $(HOME)bin/
	@echo done

clean:
	@echo -- cleaning objects and executable --
	-$(RM) $(OBJS) $(EXEC)
	@echo done

clock:
	$(TOUCH) Makefile
	$(TOUCH) makeinclude
	@echo

