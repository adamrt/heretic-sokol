shader:
	./sokol-shdc -i src/shaders/basic.glsl -o src/shaders/basic.glsl.h -l glsl330

run: shader
	mkdir -p build && cd build && cmake .. && cmake --build . && ./heretic

release:
	mkdir -p build && cd build && cmake -DCMAKE_BUILD_TYPE=MinSizeRel .. && cmake --build . && ./heretic
