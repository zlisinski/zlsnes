.PHONY: all clean test

BUILD_DIR = build

all:
	@mkdir -p $(BUILD_DIR)
	@cd $(BUILD_DIR) && cmake .. && make -j$(nproc)

test:
	@mkdir -p $(BUILD_DIR)
	@cd $(BUILD_DIR) && cmake .. && make -j$(nproc)
	./test_zlsnes

clean:
	rm -rf $(BUILD_DIR) $(BINS)
