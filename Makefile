.PHONY: all clean test

MAIN_BIN = zlsnes
TEST_BIN = test_zlsnes
BINS = $(MAIN_BIN) $(TEST_BIN)
BUILD_DIR = build

all: $(BINS)

test: $(TEST_BIN)
	./$(TEST_BIN)

clean:
	rm -rf $(BUILD_DIR) $(BINS)

$(BINS):
	@mkdir -p $(BUILD_DIR)
	@cd $(BUILD_DIR) && cmake .. && make -j$(nproc)
