.PHONY: all clean test test_data

BUILD_DIR = build
BINS = zlsnes test_zlsnes

all:
	@mkdir -p $(BUILD_DIR)
	@cd $(BUILD_DIR) && cmake .. && make -j$(nproc)

test_data:
	git submodule update --init

test: all test_data
	./test_zlsnes

clean:
	rm -rf $(BUILD_DIR) $(BINS)
