debug:
	mkdir -p build && cd build && cmake .. && cmake --build . && ./heretic

release:
	mkdir -p build && cd build && cmake -DCMAKE_BUILD_TYPE=MinSizeRel .. && cmake --build . && ./heretic
