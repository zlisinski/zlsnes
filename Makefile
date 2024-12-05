.PHONY: all clean test test_data

BUILD_DIR = build
BINS = zlsnes test_zlsnes

all:
	@mkdir -p $(BUILD_DIR)
	@cd $(BUILD_DIR) && cmake .. && make -j$(nproc)

test_data:
	git submodule update --init

test: all test_data
	cd build/src/core && GTEST_COLOR=1 ctest -V

clean:
	rm -rf $(BUILD_DIR) $(BINS)
