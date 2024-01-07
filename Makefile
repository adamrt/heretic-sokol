sokol-shdc:
	wget https://github.com/floooh/sokol-tools-bin/raw/6c8fc754ad73bf0db759ae44b224093290e5b26e/bin/linux/sokol-shdc
	chmod +x sokol-shdc

build:
	mkdir -p build

shader: sokol-shdc
	./sokol-shdc -i src/shaders/standard.glsl -o src/shaders/standard.glsl.h -l glsl330

run: build shader
	cd build && cmake .. && cd .. && cmake --build build && ./build/heretic

release: build shader
	cd build && cmake -DCMAKE_BUILD_TYPE=MinSizeRel .. && cd .. && cmake --build build && ./build/heretic
