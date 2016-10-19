1、install cmake
2、install gcc or cross compiler
3、compiling
	cmake -DCMAKE_BUILD_TYPE=Debug -DPAHO_BUILD_SAMPLES=TRUE -DCMAKE_TOOLCHAIN_FILE=<rel-dir>/cmake/toolchain.linux-arm11.cmake ./
4、set enviroment
	export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/root/dir
5、running
	./tbox
