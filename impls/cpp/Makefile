.PHONY: build rebuild
all: rebuild

build:
	cd build && ninja


rebuild: src/main.cpp
	mkdir -p build
	cd build && cmake .. -G "Ninja" -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
	rm -rf compile_commands.json && ln -s build/compile_commands.json compile_commands.json
	cd build && ninja

clean: build/
	rm -rf ./build

run: build
	build/forchip