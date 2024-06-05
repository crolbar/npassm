.PHONY: build

build:
	cmake --build ./build --config Release
	
run:
	cmake --build ./build --config Release
	./build/npassm

clean:
	rf -rf ./build
