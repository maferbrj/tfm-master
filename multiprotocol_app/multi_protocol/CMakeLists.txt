# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.20.0)
set(QEMU_EXTRA_FLAGS -s)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(mesh)

target_sources(app PRIVATE src/main.c)
target_sources(app PRIVATE src/fsm.c)
target_sources(app PRIVATE src/fsm_protocol_controller.c)
target_sources(app PRIVATE src/fsm_mesh.c)
target_sources(app PRIVATE src/model_handler.c)
target_sources(app PRIVATE src/fsm_thread.c)
target_sources(app PRIVATE src/coap_client.c)
target_sources(app PRIVATE src/coap_client_utils.c)



if (CONFIG_BUILD_WITH_TFM)
  target_include_directories(app PRIVATE
    $<TARGET_PROPERTY:tfm,TFM_BINARY_DIR>/install/interface/include
  )
endif()
