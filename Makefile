# New makefile to build the Cyclone physics engine for Linux.

# Determine architecture (Linux or Mac OS X).
PLATFORM = $(shell uname)

ifeq ($(PLATFORM), Linux)
    LDFLAGS = -lGL -lGLU -lglut
else
    $(error This OS is not Ubuntu Linux. Aborting)
endif

# Demo files path.
DEMOPATH = ./src/demos/

# Demo core files.
DEMOCOREFILES = $(DEMOPATH)main.cpp $(DEMOPATH)app.cpp $(DEMOPATH)timing.cpp

# Demo files.
DEMOLIST = ./tankgame

# Cyclone core files.
CYCLONEFILES = ./src/body.cpp ./src/collide_coarse.cpp ./src/collide_fine.cpp ./src/contacts.cpp ./src/core.cpp ./src/fgen.cpp ./src/joints.cpp ./src/particle.cpp ./src/pcontacts.cpp ./src/pfgen.cpp ./src/plinks.cpp ./src/pworld.cpp ./src/random.cpp ./src/world.cpp

.PHONY: clean

ifneq ("$(wildcard $(DEMOLIST))","")
clean:
	rm $(DEMOLIST)
endif

all: $(DEMOLIST)

$(DEMOLIST):
	g++ -O2 -Iinclude $(DEMOCOREFILES) $(CYCLONEFILES) $(DEMOPATH)$@/$@.cpp -o $@ $(LDFLAGS)



