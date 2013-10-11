WEAVER_DIR=..

COMPILER=g++

OPENCV_INC_FLAGS=-I/usr/local/include/opencv/
OPENCV_LIB_FLAGS=-L/usr/local/lib/ -lcv -lcxcore -lhighgui

TIFF_INC_FLAGS=-I$(WEAVER_DIR)/bigtiff/tiff-4.0/libtiff/
TIFF_LIB_FLAGS=$(WEAVER_DIR)/bigtiff/libtiff.a $(WEAVER_DIR)/bigtiff/libz.a $(WEAVER_DIR)/bigtiff/libjpeg.a

MPI_INC_FLAGS=-I/opt/mpich2/gnu/include
MPI_LIB_FLAGS=-L/opt/mpich2/gnu/lib -lmpich -pthread -lrt

QT_DIR=/usr/local/Trolltech/Qt-4.7.0/
QT_INC_FLAGS=-I$(QT_DIR)/include/ -I$(QT_DIR)/include/QtGui -I$(QT_DIR)/include/QtCore/
QT_LIB_FLAGS=-L$(QT_DIR)/lib -lQtGui -lQtCore -lQtOpenGL

CFLAGS=-g -O3 -I../include $(OPENCV_INC_FLAGS) $(TIFF_INC_FLAGS) -I./weaverGui $(QT_INC_FLAGS)

#all: weaver weaver_mpi
all: weaver

clean:
	rm -f *.o ../bin/weaver ../bin/weaver_mpi
	cd weaverGui; make clean; qmake; cd -

weaver: wStitch.o prepQtGui qImageOperations.o weavermanualalign.o moc_weavermanualalign.o weaverprojectdialog.o moc_weaverprojectdialog.o
	$(COMPILER) $(CFLAGS) main.cpp wStitch.o qImageOperations.o weavermanualalign.o moc_weavermanualalign.o weaverprojectdialog.o moc_weaverprojectdialog.o -o ../bin/weaver $(TIFF_LIB_FLAGS) $(OPENCV_LIB_FLAGS) $(QT_LIB_FLAGS)
		
weaver_mpi: wStitch.o
	$(COMPILER) $(CFLAGS) $(MPI_INC_FLAGS) main_mpi.cpp wStitch.o -o ../bin/weaver_mpi $(TIFF_LIB_FLAGS) $(OPENCV_LIB_FLAGS) $(MPI_LIB_FLAGS)
		
wStitch.o: wStitch.cpp
	$(COMPILER) $(CFLAGS) -c wStitch.cpp 

prepQtGui:
	cd weaverGui; qmake; make; cd - 

qImageOperations.o:weaverGui/qImageOperations.cpp
	$(COMPILER) $(CFLAGS) -c weaverGui/qImageOperations.cpp

weavermanualalign.o:weaverGui/weavermanualalign.cpp
	$(COMPILER) $(CFLAGS) -c weaverGui/weavermanualalign.cpp 

moc_weavermanualalign.o:weaverGui/moc_weavermanualalign.cpp
	$(COMPILER) $(CFLAGS) -c weaverGui/moc_weavermanualalign.cpp 

weaverprojectdialog.o:weaverGui/weaverprojectdialog.cpp
	$(COMPILER) $(CFLAGS) -c weaverGui/weaverprojectdialog.cpp 

moc_weaverprojectdialog.o:weaverGui/moc_weaverprojectdialog.cpp
	$(COMPILER) $(CFLAGS) -c weaverGui/moc_weaverprojectdialog.cpp 
