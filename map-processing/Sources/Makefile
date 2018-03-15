#-------------------------------------------------------------------------------
# @Author: Thibault Napoléon <Imothep>
# @Company: ISEN Yncréa Ouest
# @Email: thibault.napoleon@isen-ouest.yncrea.fr
# @Created Date: 27-Feb-2018 - 15:26:18
# @Last Modified: 14-Mar-2018 - 15:33:34
#-------------------------------------------------------------------------------

#--- Preferences -----------------------------------------------------------------------------------------------------------------------------------------------
EXECUTABLE=Test
OPENCVLIB=/home/napoleon/Libraries/OpenCV/OpenCV3.3.1/
OPTIONS=-Wall -O3 -ffast-math -m64 -I$(OPENCVLIB)include/
LIBRARIES=-m64 -L$(OPENCVLIB)lib/ -lopencv_core -lopencv_highgui -lopencv_imgproc -lopencv_imgcodecs -lopencv_videoio -lopencv_reg

#--- Build -----------------------------------------------------------------------------------------------------------------------------------------------------
$(EXECUTABLE): Main.o
	g++ $^ -o $@ $(LIBRARIES)

Main.o: Main.cpp
	g++ $(OPTIONS) -c $^ -o $@

clean:
	rm -f *.o $(EXECUTABLE) Results/*

all: clean $(EXECUTABLE)
