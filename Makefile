# NOTE: DPF path is hard-coded in HVCC template Makefile. Do NOT specify another path here.
BUILD_DIR := build
VENV_DIR := $(BUILD_DIR)/venv
HVCC_OUTPUT_DIR := $(BUILD_DIR)

all: build-plugin

$(BUILD_DIR)/00-setup-virtual-env.stamp:
	python -m virtualenv $(VENV_DIR)
	touch $@

$(BUILD_DIR)/01-install-hvcc.stamp: $(BUILD_DIR)/00-setup-virtual-env.stamp
	$(VENV_DIR)/bin/python -m pip install dep/hvcc
	touch $@

# NOTICE: Plugin source should always re-generate on every build.
generate-plugin-source: $(BUILD_DIR)/01-install-hvcc.stamp
	$(VENV_DIR)/bin/hvcc comprez.pd -n COMPREZ -m comprez.json -o $(HVCC_OUTPUT_DIR) -g dpf -p dep/heavylib/ dep/ --copyright "Copyright (c) AnClark Liu 2024 - GPL-3.0-or-later"
	python tools/get_plugin_version.py  comprez.json $(HVCC_OUTPUT_DIR)/plugin/source/version.hpp
	cp override/* $(HVCC_OUTPUT_DIR)/plugin/source/

build-plugin: generate-plugin-source
	$(MAKE) -C $(HVCC_OUTPUT_DIR)

clean:
	$(MAKE) -C $(HVCC_OUTPUT_DIR) clean

distclean:
	rm -rf $(BUILD_DIR)
