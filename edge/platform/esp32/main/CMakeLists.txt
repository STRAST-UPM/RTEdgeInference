if(DEFINED ENABLE_TRACES)
    add_compile_definitions(ENABLE_TRACES)
endif()
if(DEFINED ESP32_HW_MODEL)
    add_compile_definitions(ESP32_HW_MODEL=${ESP32_HW_MODEL})
endif()

if(DEFINED EDGE_WORKSPACE_ROOT)
    get_filename_component(EDGE_ROOT "${EDGE_WORKSPACE_ROOT}" ABSOLUTE)
else()
    get_filename_component(EDGE_ROOT "${CMAKE_CURRENT_LIST_DIR}/../../.." ABSOLUTE)
endif()

set(EDGE_COMMON_SRC "${EDGE_ROOT}/common/src")
set(EDGE_COMMON_INC "${EDGE_ROOT}/common/include")
set(EDGE_COMMON_PLATFORM_INC "${EDGE_ROOT}/common/include/platform")
set(EDGE_ESP32_SRC "${EDGE_ROOT}/platform/esp32/src")
set(EDGE_ESP32_INC "${EDGE_ROOT}/platform/esp32/include")

if(DEFINED EDGE_GENERATED_DIR)
    get_filename_component(EDGE_GEN_ROOT "${EDGE_GENERATED_DIR}" ABSOLUTE)
else()
    set(EDGE_GEN_ROOT "${CMAKE_BINARY_DIR}")
endif()

idf_component_register(
    SRCS
    "${EDGE_COMMON_SRC}/events_mgr.c"
    "${EDGE_COMMON_SRC}/prediction_batch_context.c"
    "${EDGE_COMMON_SRC}/prediction_batch_cleanup.c"
    "${EDGE_COMMON_SRC}/prediction_inference_reset.c"
    "${EDGE_COMMON_SRC}/prediction_inference_runner.c"
    "${EDGE_COMMON_SRC}/prediction_trace.c"
    "${EDGE_COMMON_SRC}/prediction_timing.c"
    "${EDGE_ESP32_SRC}/main.c"
    "${EDGE_ESP32_SRC}/data_reader.c"
    "${EDGE_ESP32_SRC}/runtime_app_esp32.c"
    "${EDGE_ESP32_SRC}/platform_api_esp32.c"
    "${EDGE_ESP32_SRC}/prediction_batch_queue_esp32.c"
    "${EDGE_ESP32_SRC}/prediction_timeout_recovery_esp32.c"
    "${EDGE_ESP32_SRC}/prediction_timer_arm_esp32.c"
    "${EDGE_ESP32_SRC}/prediction_dispatch_esp32.c"
    "${EDGE_ESP32_SRC}/prediction_slot_runner_esp32.c"
    "${EDGE_ESP32_SRC}/prediction_runtime_bootstrap_esp32.c"
    "${EDGE_ESP32_SRC}/prediction_runtime_profile_esp32.c"
    "${EDGE_ESP32_SRC}/esp32_hw_params.c"
    "${EDGE_ESP32_SRC}/prediction_worker_sync_esp32.c"
    "${EDGE_ESP32_SRC}/prediction_timer_isr_bridge_esp32.c"
    "${EDGE_COMMON_SRC}/models_mgr.c"
    "${EDGE_GEN_ROOT}/generated/models_data.c"
    "${EDGE_COMMON_SRC}/scheduler.c"
    "${EDGE_ESP32_SRC}/predictions_mgr.c"
    "${EDGE_COMMON_SRC}/tflite_runner.cpp"
    "${EDGE_COMMON_SRC}/prediction_watchdog.c"
    "${EDGE_ESP32_SRC}/trace.c"
   
    INCLUDE_DIRS
        "."
        "${EDGE_COMMON_INC}"
        "${EDGE_COMMON_PLATFORM_INC}"
        "${EDGE_ESP32_INC}"
        "${EDGE_GEN_ROOT}"
        "${EDGE_GEN_ROOT}/generated"
    REQUIRES
        freertos
        driver
        #tflite_runner
        esp-tflite-micro
        esp_timer
)
