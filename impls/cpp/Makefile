all: build

build:
	mkdir -p build
	cd build && cmake .. -G "Ninja" -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
	ln -s build/compile_commands.json compile_commands.json
	cd build && ninja

clean: build/
	rm -rf ./build

run: build
	build/forchip