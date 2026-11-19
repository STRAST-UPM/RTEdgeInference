cmake_minimum_required(VERSION 3.20)
project(RTEdgeInferenceSTM32 C CXX ASM)

# Keep object/dependency paths short to avoid deep .obj.d path failures.
set(CMAKE_OBJECT_PATH_MAX 256)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif()

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(NOT CMAKE_TOOLCHAIN_FILE)
    message(FATAL_ERROR "Set CMAKE_TOOLCHAIN_FILE to cmake/arm-none-eabi.cmake")
endif()

option(ENABLE_TRACES "Enable trace instrumentation" ON)
option(ENABLE_CMSIS_NN "Use ARM CMSIS-NN optimized kernels instead of TFLM reference kernels" OFF)
set(STM32_HW_MODEL "STM32_HW_MODEL_H723ZG" CACHE STRING "STM32 hardware profile macro")
set_property(CACHE STM32_HW_MODEL PROPERTY STRINGS
    STM32_HW_MODEL_H723ZG
    STM32_HW_MODEL_H753ZI
    STM32_HW_MODEL_H743ZI2
)

if(STM32_HW_MODEL STREQUAL "STM32_HW_MODEL_H723ZG")
    set(STM32_STARTUP_FILE "${CMAKE_CURRENT_LIST_DIR}/startup/startup_stm32h723xx.s")
    set(STM32_LINKER_FILE "${CMAKE_CURRENT_LIST_DIR}/linker/STM32H723ZGXX_FLASH.ld")
elseif(STM32_HW_MODEL STREQUAL "STM32_HW_MODEL_H753ZI")
    set(STM32_STARTUP_FILE "${CMAKE_CURRENT_LIST_DIR}/startup/startup_stm32h753xx.s")
    set(STM32_LINKER_FILE "${CMAKE_CURRENT_LIST_DIR}/linker/STM32H753ZIXX_FLASH.ld")
elseif(STM32_HW_MODEL STREQUAL "STM32_HW_MODEL_H743ZI2")
    set(STM32_STARTUP_FILE "${CMAKE_CURRENT_LIST_DIR}/startup/startup_stm32h723xx.s")
    set(STM32_LINKER_FILE "${CMAKE_CURRENT_LIST_DIR}/linker/STM32H723ZGXX_FLASH.ld")
else()
    message(FATAL_ERROR "Unsupported STM32_HW_MODEL=${STM32_HW_MODEL}")
endif()

if(DEFINED EDGE_WORKSPACE_ROOT)
    get_filename_component(EDGE_ROOT "${EDGE_WORKSPACE_ROOT}" ABSOLUTE)
else()
    get_filename_component(EDGE_ROOT "${CMAKE_CURRENT_LIST_DIR}/../../.." ABSOLUTE)
endif()

set(EDGE_COMMON_DIR "${EDGE_ROOT}/common")
set(EDGE_STM32_DIR "${EDGE_ROOT}/platform/stm32")

if(DEFINED EDGE_GENERATED_DIR)
    get_filename_component(GEN_BUILD_DIR "${EDGE_GENERATED_DIR}" ABSOLUTE)
else()
    set(GEN_BUILD_DIR "${EDGE_ROOT}/build")
endif()

set(GEN_DIR "${GEN_BUILD_DIR}/generated")
if(NOT DEFINED TFLM_DIR)
    set(TFLM_DIR "${CMAKE_CURRENT_LIST_DIR}/../../../managed_components/espressif__esp-tflite-micro")
endif()
get_filename_component(TFLM_DIR "${TFLM_DIR}" ABSOLUTE)

if(NOT EXISTS "${TFLM_DIR}/tensorflow/lite/micro/micro_interpreter.cc")
    message(FATAL_ERROR "TensorFlow Lite Micro sources not found in TFLM_DIR=${TFLM_DIR}")
endif()

if(NOT EXISTS "${GEN_BUILD_DIR}/config.h")
    message(FATAL_ERROR "Missing generated config: ${GEN_BUILD_DIR}/config.h (run: make generate)")
endif()
if(NOT EXISTS "${GEN_DIR}/models_data.c")
    message(FATAL_ERROR "Missing generated model data: ${GEN_DIR}/models_data.c (run: make generate)")
endif()
if(NOT EXISTS "${GEN_DIR}/memory_events.h")
    message(FATAL_ERROR "Missing generated memory events: ${GEN_DIR}/memory_events.h (run: make generate)")
endif()

set(MCU_FLAGS
    -mcpu=cortex-m7
    -mfpu=fpv5-d16
    -mfloat-abi=hard
    -mthumb
)
add_compile_options(
    ${MCU_FLAGS}
    -ffunction-sections
    -fdata-sections
    -fno-common
    -Wall
    -Wextra
)

add_compile_definitions(
    STM32_HW_MODEL=${STM32_HW_MODEL}
    TF_LITE_STATIC_MEMORY
    TF_LITE_DISABLE_X86_NEON
)
if(ENABLE_TRACES)
    add_compile_definitions(ENABLE_TRACES)
endif()
if(ENABLE_CMSIS_NN)
    add_compile_definitions(CMSIS_NN)
endif()

# Reuse shared local mirrors to avoid flaky network clones during long STM32 runs.
get_filename_component(REPO_ROOT_FROM_EDGE "${EDGE_ROOT}/.." ABSOLUTE)
set(SHARED_FETCHCONTENT_DIR "${REPO_ROOT_FROM_EDGE}/.cache/cmake-fetchcontent")
if(EXISTS "${SHARED_FETCHCONTENT_DIR}/cmsis5-src")
    set(FETCHCONTENT_SOURCE_DIR_CMSIS5 "${SHARED_FETCHCONTENT_DIR}/cmsis5-src" CACHE PATH "Local CMSIS_5 mirror" FORCE)
endif()
if(EXISTS "${SHARED_FETCHCONTENT_DIR}/cmsis_freertos-src")
    set(FETCHCONTENT_SOURCE_DIR_CMSIS_FREERTOS "${SHARED_FETCHCONTENT_DIR}/cmsis_freertos-src" CACHE PATH "Local CMSIS-FreeRTOS mirror" FORCE)
endif()

include(FetchContent)
FetchContent_Declare(
    cmsis5
    GIT_REPOSITORY https://github.com/ARM-software/CMSIS_5.git
    GIT_TAG 5.9.0
    GIT_SHALLOW ON
    GIT_SUBMODULES ""
    GIT_SUBMODULES_RECURSE OFF
)
FetchContent_Declare(
    cmsis_freertos
    GIT_REPOSITORY https://github.com/ARM-software/CMSIS-FreeRTOS.git
    GIT_TAG main
    GIT_SUBMODULES ""
    GIT_SUBMODULES_RECURSE OFF
)
FetchContent_MakeAvailable(cmsis5 cmsis_freertos)

# ---------------------------------------------------------------------------
# CMSIS-NN (optional – only when ENABLE_CMSIS_NN=ON)
# ---------------------------------------------------------------------------
if(ENABLE_CMSIS_NN)
    message(STATUS "[CMSIS-NN] Building with ARM CMSIS-NN optimized kernels")

    # Tell CMSIS-NN where to find CMSIS Core headers (already fetched above)
    set(CMSIS_PATH "${cmsis5_SOURCE_DIR}" CACHE STRING "Path to CMSIS-5" FORCE)
    # Suppress CMSIS-NN's own unit tests
    set(BUILD_CMSIS_NN_TESTS       OFF CACHE BOOL "" FORCE)
    set(CMSIS_NN_BUILD_TESTCASES   OFF CACHE BOOL "" FORCE)

    FetchContent_Declare(
        cmsis_nn
        GIT_REPOSITORY https://github.com/ARM-software/CMSIS-NN.git
        GIT_TAG        v4.1.0
        GIT_SHALLOW    ON
    )
    FetchContent_MakeAvailable(cmsis_nn)

    # -----------------------------------------------------------------------
    # Download CMSIS-NN kernel wrappers from upstream tflite-micro.
    # Files are cached in the build tree; re-downloaded only if absent.
    # Pin TFLM_CMSIS_COMMIT to a specific SHA for reproducibility.
    # This revision predates the newer CMSIS-NN APIs not available in the
    # v4.1.0 library pinned by this project and keeps stm32-cmsis repeatable.
    # -----------------------------------------------------------------------
    set(TFLM_CMSIS_COMMIT "a6e6ea1823e30c05839b7573ade8a16242b056ff")
    set(_TFLM_RAW_BASE
        "https://raw.githubusercontent.com/tensorflow/tflite-micro/${TFLM_CMSIS_COMMIT}/tensorflow/lite/micro/kernels/cmsis_nn"
    )

    # Dir for downloaded .cc kernel files
    set(TFLM_CMSIS_KERN_DIR "${CMAKE_CURRENT_BINARY_DIR}/cmsis_nn_kernels")
    file(MAKE_DIRECTORY "${TFLM_CMSIS_KERN_DIR}")

    # Use wrappers verified against the CMSIS-NN version pinned in this
    # project so every stm32-cmsis experiment inherits the same acceleration.
    set(CMSIS_NN_COMPAT_KERNELS
        add.cc
        conv.cc
        depthwise_conv.cc
        fully_connected.cc
        mul.cc
        pooling.cc
        softmax.cc
    )

    foreach(_kern ${CMSIS_NN_COMPAT_KERNELS})
        set(_dest "${TFLM_CMSIS_KERN_DIR}/${_kern}")
        if(NOT EXISTS "${_dest}")
            message(STATUS "[CMSIS-NN] Downloading ${_kern}")
            file(DOWNLOAD
                "${_TFLM_RAW_BASE}/${_kern}"
                "${_dest}"
                STATUS _dl_status
                LOG    _dl_log
            )
            list(GET _dl_status 0 _dl_err)
            if(_dl_err)
                message(FATAL_ERROR
                    "[CMSIS-NN] Failed to download ${_kern}.\n"
                    "Status : ${_dl_status}\n"
                    "Log    : ${_dl_log}\n"
                    "Ensure internet access is available during CMake configure, "
                    "or pre-populate: ${TFLM_CMSIS_KERN_DIR}/"
                )
            endif()
        endif()

        if(_kern STREQUAL "fully_connected.cc")
            file(READ "${_dest}" _fc_src)
            string(REPLACE
"  data->buffer_idx = -1;\n"
"  data->buffer_idx = -1;\n  data->kernel_sums = nullptr;\n"
                _fc_src "${_fc_src}")
            string(REPLACE
[=[    } else {
      buf_size = arm_fully_connected_s8_get_buffer_size(&filter_dims);

      if (buf_size > 0) {
        data->kernel_sums = static_cast<int32_t*>(
            context->AllocatePersistentBuffer(context, buf_size));

        int8_t* filter_data = GetTensorData<int8_t>(filter);

        if (filter->type == kTfLiteInt4) {
          size_t filter_size = GetTensorShape(filter).FlatSize();
          int8_t* unpacked_filter_buf =
              reinterpret_cast<int8_t*>(micro_context->AllocateTempBuffer(
                  filter_size, tflite::MicroArenaBufferAlignment()));

          tflite::tensor_utils::UnpackDenseInt4IntoInt8(
              filter_data, filter_size, unpacked_filter_buf);
          filter_data = unpacked_filter_buf;
        }

        arm_vector_sum_s8(data->kernel_sums, filter_dims.n, data->output_depth,
                          filter_data);

        if (filter->type == kTfLiteInt4) {
          micro_context->DeallocateTempBuffer(
              reinterpret_cast<uint8_t*>(filter_data));
        }

        // Do not request a scratch buffer since using persistent memory
        buf_size = 0;
      }
    }]=]
[=[    } else {
      buf_size = arm_fully_connected_s8_get_buffer_size(&filter_dims);
    }]=]
                _fc_src "${_fc_src}")
            string(REPLACE
"    ctx.buf = data.kernel_sums;\n"
""
                _fc_src "${_fc_src}")
            file(WRITE "${_dest}" "${_fc_src}")
        endif()
    endforeach()
endif()

set(FREERTOS_KERNEL_DIR "${cmsis_freertos_SOURCE_DIR}/Source")
set(CMSIS_RTOS2_DIR "${cmsis_freertos_SOURCE_DIR}/CMSIS/RTOS2/FreeRTOS")
set(CMSIS_DEVICE_DIR "${cmsis5_SOURCE_DIR}/Device/ARM/ARMCM7/Include")

if(NOT EXISTS "${FREERTOS_KERNEL_DIR}/tasks.c")
    message(FATAL_ERROR "FreeRTOS kernel sources not found in ${FREERTOS_KERNEL_DIR}")
endif()
if(NOT EXISTS "${CMSIS_RTOS2_DIR}/Source/cmsis_os2.c")
    message(FATAL_ERROR "CMSIS-RTOS2 wrapper not found in ${CMSIS_RTOS2_DIR}")
endif()

add_library(freertos_kernel STATIC
    "${FREERTOS_KERNEL_DIR}/list.c"
    "${FREERTOS_KERNEL_DIR}/queue.c"
    "${FREERTOS_KERNEL_DIR}/tasks.c"
    "${FREERTOS_KERNEL_DIR}/timers.c"
    "${FREERTOS_KERNEL_DIR}/event_groups.c"
    "${FREERTOS_KERNEL_DIR}/stream_buffer.c"
    "${FREERTOS_KERNEL_DIR}/portable/GCC/ARM_CM7/r0p1/port.c"
    "${FREERTOS_KERNEL_DIR}/portable/MemMang/heap_4.c"
)
target_include_directories(freertos_kernel PUBLIC
    "${FREERTOS_KERNEL_DIR}/include"
    "${FREERTOS_KERNEL_DIR}/portable/GCC/ARM_CM7/r0p1"
    "${cmsis5_SOURCE_DIR}/CMSIS/Core/Include"
    "${CMAKE_CURRENT_LIST_DIR}/src"
)

add_library(cmsis_rtos2 STATIC
    "${CMSIS_RTOS2_DIR}/Source/cmsis_os2.c"
)
target_compile_definitions(cmsis_rtos2 PRIVATE
    osThreadPrivileged=0U
    CMSIS_device_header=\"ARMCM7_DP.h\"
)
target_include_directories(cmsis_rtos2 PUBLIC
    "${cmsis5_SOURCE_DIR}/CMSIS/RTOS2/Include"
    "${CMSIS_RTOS2_DIR}/Include"
    "${cmsis5_SOURCE_DIR}/CMSIS/Core/Include"
    "${CMSIS_DEVICE_DIR}"
    "${FREERTOS_KERNEL_DIR}/include"
    "${FREERTOS_KERNEL_DIR}/portable/GCC/ARM_CM7/r0p1"
    "${CMAKE_CURRENT_LIST_DIR}/src"
)
target_link_libraries(cmsis_rtos2 PUBLIC freertos_kernel)

set(tflite_dir "${TFLM_DIR}/tensorflow/lite")
set(signal_dir "${TFLM_DIR}/signal")
set(compiler_mlir_dir "${TFLM_DIR}/tensorflow/compiler/mlir")
set(tfmicro_dir "${tflite_dir}/micro")
set(tfmicro_frontend_dir "${tflite_dir}/experimental/microfrontend/lib")
set(tfmicro_kernels_dir "${tfmicro_dir}/kernels")

set(srcs_micro
    "${tfmicro_dir}/debug_log.cc"
    "${tfmicro_dir}/flatbuffer_utils.cc"
    "${tfmicro_dir}/memory_helpers.cc"
    "${tfmicro_dir}/micro_allocation_info.cc"
    "${tfmicro_dir}/micro_allocator.cc"
    "${tfmicro_dir}/micro_context.cc"
    "${tfmicro_dir}/micro_interpreter_context.cc"
    "${tfmicro_dir}/micro_interpreter_graph.cc"
    "${tfmicro_dir}/micro_interpreter.cc"
    "${tfmicro_dir}/micro_log.cc"
    "${tfmicro_dir}/micro_op_resolver.cc"
    "${tfmicro_dir}/micro_profiler.cc"
    "${tfmicro_dir}/micro_resource_variable.cc"
    "${tfmicro_dir}/micro_time.cc"
    "${tfmicro_dir}/micro_utils.cc"
    "${tfmicro_dir}/recording_micro_allocator.cc"
    "${tfmicro_dir}/system_setup.cc"
)
file(GLOB src_micro_frontend
    "${tfmicro_frontend_dir}/*.c"
    "${tfmicro_frontend_dir}/*.cc"
)
file(GLOB srcs_tflite_bridge
    "${tfmicro_dir}/tflite_bridge/*.c"
    "${tfmicro_dir}/tflite_bridge/*.cc"
)
file(GLOB srcs_kernels
    "${tfmicro_kernels_dir}/*.c"
    "${tfmicro_kernels_dir}/*.cc"
)
file(GLOB_RECURSE srcs_kernels_espnn
    "${tfmicro_kernels_dir}/esp_nn/*.cc"
)
list(REMOVE_ITEM srcs_kernels ${srcs_kernels_espnn})

if(ENABLE_CMSIS_NN)
    # Remove only the reference kernels replaced by the compatibility subset.
    foreach(_op
        add
        conv
        depthwise_conv
        fully_connected
        mul
        pooling
        softmax
    )
        list(FILTER srcs_kernels EXCLUDE REGEX ".*/${_op}\\.cc$")
    endforeach()
    # Add only the downloaded compatibility wrappers (avoid stale files
    # from previous configurations in the build directory).
    set(srcs_kernels_cmsis_nn)
    foreach(_kern ${CMSIS_NN_COMPAT_KERNELS})
        list(APPEND srcs_kernels_cmsis_nn "${TFLM_CMSIS_KERN_DIR}/${_kern}")
    endforeach()
    list(APPEND srcs_kernels ${srcs_kernels_cmsis_nn})
    message(STATUS "[CMSIS-NN] Replaced reference kernels with CMSIS-NN wrappers (${TFLM_CMSIS_KERN_DIR})")
endif()

file(GLOB signal_micro_kernels
    "${signal_dir}/micro/kernels/*.c"
    "${signal_dir}/micro/kernels/*.cc"
)
file(GLOB signal_src
    "${signal_dir}/src/*.c"
    "${signal_dir}/src/*.cc"
)
set(signal_srcs
    ${signal_micro_kernels}
    ${signal_src}
    "${signal_dir}/src/kiss_fft_wrappers/kiss_fft_float.cc"
    "${signal_dir}/src/kiss_fft_wrappers/kiss_fft_int16.cc"
    "${signal_dir}/src/kiss_fft_wrappers/kiss_fft_int32.cc"
)

set(tflm_sources
    ${srcs_micro}
    ${srcs_kernels}
    ${srcs_tflite_bridge}
    ${src_micro_frontend}
    ${signal_srcs}
    "${tflite_dir}/kernels/kernel_util.cc"
    "${tflite_dir}/micro/memory_planner/greedy_memory_planner.cc"
    "${tflite_dir}/micro/memory_planner/linear_memory_planner.cc"
    "${tflite_dir}/micro/arena_allocator/non_persistent_arena_buffer_allocator.cc"
    "${tflite_dir}/micro/arena_allocator/persistent_arena_buffer_allocator.cc"
    "${tflite_dir}/micro/arena_allocator/recording_single_arena_buffer_allocator.cc"
    "${tflite_dir}/micro/arena_allocator/single_arena_buffer_allocator.cc"
    "${tflite_dir}/core/c/common.cc"
    "${tflite_dir}/core/api/flatbuffer_conversions.cc"
    "${tflite_dir}/core/api/tensor_utils.cc"
    "${tflite_dir}/kernels/internal/common.cc"
    "${tflite_dir}/kernels/internal/quantization_util.cc"
    "${tflite_dir}/kernels/internal/portable_tensor_utils.cc"
    "${tflite_dir}/kernels/internal/tensor_utils.cc"
    "${tflite_dir}/kernels/internal/tensor_ctypes.cc"
    "${tflite_dir}/kernels/internal/reference/portable_tensor_utils.cc"
    "${tflite_dir}/kernels/internal/reference/comparisons.cc"
    "${compiler_mlir_dir}/lite/core/api/error_reporter.cc"
    "${compiler_mlir_dir}/lite/schema/schema_utils.cc"
)

add_library(tflite_micro STATIC ${tflm_sources})
target_include_directories(tflite_micro PUBLIC
    "${TFLM_DIR}"
    "${TFLM_DIR}/third_party/gemmlowp"
    "${TFLM_DIR}/third_party/flatbuffers/include"
    "${TFLM_DIR}/third_party/ruy"
    "${TFLM_DIR}/third_party/kissfft"
)
if(ENABLE_CMSIS_NN)
    target_include_directories(tflite_micro PRIVATE
        # CMSIS-NN repo root: resolves #include "Include/arm_nnfunctions.h"
        "${cmsis_nn_SOURCE_DIR}"
        # CMSIS Core: some CMSIS-NN headers include arm_math / cmsis_gcc
        "${cmsis5_SOURCE_DIR}/CMSIS/Core/Include"
    )
endif()
target_compile_options(tflite_micro PRIVATE
    -O3
    -Wno-unused-parameter
    -Wno-sign-compare
    -Wno-shadow
    -Wno-double-promotion
    -Wno-missing-field-initializers
    -Wno-type-limits
    -Wno-return-type
    -fno-exceptions
    -fno-rtti
    -fno-threadsafe-statics
)
target_link_libraries(tflite_micro PUBLIC m)
if(ENABLE_CMSIS_NN)
    target_link_libraries(tflite_micro PUBLIC cmsis-nn)
endif()

set(runtime_common_sources
    "${EDGE_COMMON_DIR}/src/events_mgr.c"
    "${EDGE_COMMON_DIR}/src/models_mgr.c"
    "${EDGE_COMMON_DIR}/src/scheduler.c"
    "${EDGE_COMMON_DIR}/src/prediction_batch_context.c"
    "${EDGE_COMMON_DIR}/src/prediction_batch_cleanup.c"
    "${EDGE_COMMON_DIR}/src/prediction_inference_reset.c"
    "${EDGE_COMMON_DIR}/src/prediction_inference_runner.c"
    "${EDGE_COMMON_DIR}/src/prediction_trace.c"
    "${EDGE_COMMON_DIR}/src/prediction_timing.c"
    "${EDGE_COMMON_DIR}/src/prediction_watchdog.c"
    "${EDGE_COMMON_DIR}/src/tflite_runner.cpp"
)

set(runtime_stm32_sources
    "${EDGE_STM32_DIR}/src/main_stm32.c"
    "${EDGE_STM32_DIR}/src/runtime_app_stm32.c"
    "${EDGE_STM32_DIR}/src/predictions_mgr_stm32.c"
    "${EDGE_STM32_DIR}/src/data_reader_stm32.c"
    "${EDGE_STM32_DIR}/src/trace_stm32.c"
    "${EDGE_STM32_DIR}/src/platform_api_stm32.c"
    "${EDGE_STM32_DIR}/src/prediction_batch_queue_stm32.c"
    "${EDGE_STM32_DIR}/src/prediction_worker_sync_stm32.c"
    "${EDGE_STM32_DIR}/src/prediction_timer_isr_bridge_stm32.c"
    "${EDGE_STM32_DIR}/src/prediction_runtime_bootstrap_stm32.c"
    "${EDGE_STM32_DIR}/src/prediction_runtime_profile_stm32.c"
    "${EDGE_STM32_DIR}/src/stm32_hw_params.c"
    "${EDGE_STM32_DIR}/src/prediction_dispatch_stm32.c"
    "${EDGE_STM32_DIR}/src/prediction_slot_runner_stm32.c"
    "${EDGE_STM32_DIR}/src/prediction_timer_arm_stm32.c"
    "${EDGE_STM32_DIR}/src/prediction_timeout_recovery_stm32.c"
)

add_executable(RTEdgeInference-STM32
    "${STM32_STARTUP_FILE}"
    "${CMAKE_CURRENT_LIST_DIR}/src/system_stm32h7xx.c"
    "${GEN_DIR}/models_data.c"
    ${runtime_common_sources}
    ${runtime_stm32_sources}
)

target_include_directories(RTEdgeInference-STM32 PRIVATE
    "${EDGE_COMMON_DIR}/include"
    "${EDGE_COMMON_DIR}/include/platform"
    "${EDGE_STM32_DIR}/include"
    "${GEN_BUILD_DIR}"
    "${GEN_DIR}"
    "${cmsis5_SOURCE_DIR}/CMSIS/Core/Include"
    "${cmsis5_SOURCE_DIR}/CMSIS/RTOS2/Include"
    "${cmsis_freertos_SOURCE_DIR}/CMSIS/RTOS2/FreeRTOS/Include"
    "${FREERTOS_KERNEL_DIR}/include"
    "${FREERTOS_KERNEL_DIR}/portable/GCC/ARM_CM7/r0p1"
    "${CMAKE_CURRENT_LIST_DIR}/src"
)

target_link_libraries(RTEdgeInference-STM32 PRIVATE
    cmsis_rtos2
    freertos_kernel
    tflite_micro
    m
    c
    gcc
    nosys
)

target_link_options(RTEdgeInference-STM32 PRIVATE
    ${MCU_FLAGS}
    -T${STM32_LINKER_FILE}
    -Wl,--gc-sections
    -Wl,-Map=${CMAKE_BINARY_DIR}/RTEdgeInference-STM32.map
    --specs=nosys.specs
)

add_custom_command(TARGET RTEdgeInference-STM32 POST_BUILD
    COMMAND ${CMAKE_SIZE} $<TARGET_FILE:RTEdgeInference-STM32>
)
