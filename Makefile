
all: bigtiff_lib weaver

clean:
	touch bigtiff/libjpeg/jpeg-6b/jconfig.h
	cd bigtiff; make clean; cd -
	cd src; make clean; cd -

weaver:
	cd src; make -f Makefile; cd - 

bigtiff_lib:
	cd bigtiff; make; cd -

