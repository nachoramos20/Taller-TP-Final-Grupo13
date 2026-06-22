.PHONY: all test clean

compile-debug:
	mkdir -p build/
	cmake -S . -B ./build \
		-DCMAKE_BUILD_TYPE=Debug \
		-DCMAKE_EXPORT_COMPILE_COMMANDS=ON \
		$(EXTRA_GENERATE)
	cmake --build build/ $(EXTRA_COMPILE)

run-tests: compile-debug
	./build/argentum_grupo13_tests

all: clean run-tests

clean:
	rm -Rf build/