# Host unit tests (Unity from ESP-IDF). Usage: source <mcu>/esp-idf/env.sh && make -C tests/host test
UNITY ?= $(IDF_PATH)/components/unity/unity/src
OUT   ?= build
$(OUT)/test_calc: test_calc.c ../../components/calc/calc.c
	mkdir -p $(OUT)
	gcc -Wall -Wextra -I../../components/calc/include -I$(UNITY) $^ $(UNITY)/unity.c -o $@

test: $(OUT)/test_calc
	$(OUT)/test_calc
.PHONY: test
