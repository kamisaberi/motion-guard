# File: /home/kami/motion-guard/Makefile
# ==============================================================================
# ARYORITHM TECHNOLOGIES B.V. — ROBOTICS, COBOTS & AUTONOMOUS MOBILITY
# Project: motion-guard (EtherCAT 0x88A4, SocketCAN CAN-FD, ShmRing, Nagumo CBF)
# Invariant: Sub-120 ns Deterministic Tier-1 Safety Gate | Zero Dynamic Alloc
# ==============================================================================

SHELL := /bin/bash
.DEFAULT_GOAL := help

PROJECT_DIR   := $(shell pwd)
BUILD_DIR     := $(PROJECT_DIR)/build
SCENARIOS_DIR := $(PROJECT_DIR)/scenarios/robotics_can_ethercat_grid
MATRIX_DIR    := /home/kami/sentinel-matrix
NEXUS_DIR     := /home/kami/sentinel-nexus
ETC_HARNESSES := /etc/sentinel/lab/harnesses
OPT_HARNESSES := /opt/sentinel-stack/harnesses
JOBS          ?= $(shell nproc)

CYAN   := \033[1;36m
GREEN  := \033[1;32m
YELLOW := \033[1;33m
WHITE  := \033[1;37m
RESET  := \033[0m

.PHONY: help
help:
	@echo -e "$(CYAN)================================================================================$(RESET)"
	@echo -e "$(WHITE)  MOTION-GUARD: ROBOTICS, CAN-FD, ETHERCAT & NAGUMO CBF SAFETY GOVERNOR$(RESET)"
	@echo -e "$(CYAN)================================================================================$(RESET)"
	@echo -e "  $(GREEN)make init$(RESET)         Hard copy all motion harnesses to /etc and /opt"
	@echo -e "  $(GREEN)make build$(RESET)        Compile release targets (-O3 -march=native)"
	@echo -e "  $(GREEN)make test$(RESET)         Run all unit, 3-bus ABI & Tier-1/Tier-2 pipeline tests"
	@echo -e "  $(GREEN)make bench$(RESET)        Execute hardware latency benchmark (< 120 ns SLA)"
	@echo -e "  $(GREEN)make install$(RESET)      Install libraries, CLI & plugin system-wide"
	@echo -e "  $(GREEN)make sync-all$(RESET)     Full synchronization (install + sync-matrix + sync-web)"
	@echo -e "  $(GREEN)make cobot-up$(RESET)     Deploy robotics_can_ethercat_grid simulation mesh"
	@echo -e "  $(GREEN)make cobot-down$(RESET)   Teardown robotics_can_ethercat_grid"
	@echo -e "  $(GREEN)make cobot-status$(RESET) Display running containers and ports"
	@echo -e "  $(GREEN)make cobot-logs$(RESET)   Stream live logs from Cobot EtherCAT Sentry"
	@echo -e "$(CYAN)================================================================================$(RESET)"

.PHONY: init
init:
	@sudo mkdir -p $(ETC_HARNESSES) $(OPT_HARNESSES)
	@chmod +x harnesses/*.py 2>/dev/null || true
	@for f in harnesses/*.py; do \
		if [ -f "$$f" ]; then \
			base=$$(basename $$f); \
			hyphen_base=$$(echo $$base | tr '_' '-'); \
			sudo rm -f $(ETC_HARNESSES)/$$base $(ETC_HARNESSES)/$$hyphen_base; \
			sudo cp -f $$f $(ETC_HARNESSES)/$$base; \
			sudo cp -f $$f $(ETC_HARNESSES)/$$hyphen_base; \
			sudo rm -f $(OPT_HARNESSES)/$$base $(OPT_HARNESSES)/$$hyphen_base; \
			sudo cp -f $$f $(OPT_HARNESSES)/$$base; \
			sudo cp -f $$f $(OPT_HARNESSES)/$$hyphen_base; \
		fi \
	done
	@sudo chmod 755 $(ETC_HARNESSES)/*.py $(OPT_HARNESSES)/*.py 2>/dev/null || true
	@echo -e "$(GREEN)[+] Motion-Guard harnesses installed to $(ETC_HARNESSES) and $(OPT_HARNESSES).$(RESET)"

.PHONY: all build debug
all: build

build:
	@mkdir -p $(BUILD_DIR)
	@cd $(BUILD_DIR) && cmake .. -DCMAKE_BUILD_TYPE=Release
	@$(MAKE) -C $(BUILD_DIR) -j$(JOBS)
	@echo -e "$(GREEN)[+] Build Complete: libmotion_guard.so, motion_guard_dissector_plugin.so, aryo-motion.$(RESET)"

debug:
	@mkdir -p $(BUILD_DIR)
	@cd $(BUILD_DIR) && cmake .. -DCMAKE_BUILD_TYPE=Debug -DCMAKE_CXX_FLAGS="-fsanitize=address,undefined -fno-omit-frame-pointer"
	@$(MAKE) -C $(BUILD_DIR) -j$(JOBS)

.PHONY: test bench
test: build
	@cd $(BUILD_DIR) && ctest --output-on-failure
	@echo -e "$(GREEN)[+] All Motion-Guard Unit, ABI & Pipeline Tests Passed (100%).$(RESET)"

bench: build
	@$(BUILD_DIR)/bench_latency

.PHONY: install sync-matrix sync-web sync-all
install: build init
	@sudo $(MAKE) -C $(BUILD_DIR) install
	@sudo ldconfig

sync-matrix: build
	@mkdir -p $(MATRIX_DIR)/shared/lib
	@cp -f $(BUILD_DIR)/libmotion_guard.so $(MATRIX_DIR)/shared/lib/

sync-web:
	@mkdir -p $(MATRIX_DIR)/web/plugins
	@if [ -d "$(NEXUS_DIR)/web" ]; then \
		cp -r $(NEXUS_DIR)/web/* $(MATRIX_DIR)/web/ 2>/dev/null || true; \
		mkdir -p $(MATRIX_DIR)/web/plugins; \
	fi

sync-all: install sync-matrix sync-web
	@echo -e "$(GREEN)[+] Full Motion-Guard Synchronization Complete.$(RESET)"

.PHONY: cobot-up cobot-down cobot-status cobot-logs
cobot-up: sync-all
	@aryo motion cobot-up
	@echo -e "$(GREEN)[+] Robotics / CAN-FD / EtherCAT Grid Live: http://localhost:9443/$(RESET)"

cobot-down:
	@aryo motion cobot-down || true
	@docker network prune -f >/dev/null 2>&1 || true

cobot-status:
	@docker ps --filter "name=matrix-" --format "table {{.Names}}\t{{.Status}}\t{{.Ports}}"

cobot-logs:
	@docker logs -f matrix-edge-cobot-01

.PHONY: clean distclean
clean:
	@rm -rf $(BUILD_DIR)

distclean: clean
	@$(MAKE) cobot-down >/dev/null 2>&1 || true
	@rm -rf $(MATRIX_DIR)/.matrix_cache/docker-compose.robotics_can_ethercat_grid.yml
