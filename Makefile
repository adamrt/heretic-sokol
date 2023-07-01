sokol-shdc:
	wget https://github.com/floooh/sokol-tools-bin/raw/master/bin/linux/sokol-shdc
	chmod +x sokol-shdc

build:
	mkdir -p build

shader: sokol-shdc
	./sokol-shdc -i src/shaders/basic.glsl -o src/shaders/basic.glsl.h -l glsl330

run: build shader
	cd build && cmake .. && cd .. && cmake --build build && ./build/heretic

release:
	cd build && cmake -DCMAKE_BUILD_TYPE=MinSizeRel .. && cd .. && cmake --build build && ./build/heretic
