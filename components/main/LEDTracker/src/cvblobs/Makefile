#OPENCV=opencv-2.3.1
#CPPFLAGS=`pkg-config --cflags gtk+-2.0` -I/usr/include/opencv-2.3.1/opencv
#LDFLAGS= `pkg-config --libs ${OPENCV} gtk+-2.0` -L. -lblob 
#CPPFLAGS+=-I/usr/include/opencv-2.3.1/opencv -I/usr/include/opencv-2.3.1
#LDFLAGS+=-lopencv_contrib -lopencv_legacy -lopencv_objdetect -lopencv_calib3d -lopencv_features2d -lopencv_video -lopencv_highgui -lopencv_ml -lopencv_imgproc -lopencv_flann -lopencv_core  
#LDFLAGS+=-lcv -lhighgui -lcxcore

CC=g++
CXXFLAGS+=-ggdb -O0
CPPFLAGS+=-I/usr/include/opencv
CPPFLAGS+=-I/usr/include/opencv-2.3.1
CPPFLAGS+=-I/usr/include/opencv-2.3.1/opencv

#CPPFILES= \
	#blob.cpp\
	#BlobContour.cpp\
	#BlobOperators.cpp\
	#BlobProperties.cpp\
	#BlobResult.cpp\
	#ComponentLabeling.cpp


OBJFILES= \
	blob.o\
	BlobContour.o\
	BlobOperators.o\
	BlobProperties.o\
	BlobResult.o\
	ComponentLabeling.o

#libblob.a: $(CPPFILES:.cpp=.o)
	#ar ru libblob.a $(CPPFILES:.cpp=.o)
	#ranlib libblob.a

.DEFAULT:all
.PHONY:all
all:example

#example:example.o $(CPPFILES:.cpp=.o)

LDFLAGS+=-lcxcore -lcv -lhighgui -lcvaux 

example:example.o ${OBJFILES}
	gcc example.o ${OBJFILES} -o example

#example2:example2.o $(OBJFILES)


.PHONY:clean
clean:
	rm -f *.o liblob.a example

