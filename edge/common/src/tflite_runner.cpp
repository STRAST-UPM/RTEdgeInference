#include "tflite_runner.h"
#include "model_resolver.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <new>
#include <tensorflow/lite/micro/micro_mutable_op_resolver.h>
#include <tensorflow/lite/micro/micro_interpreter.h>
#include <tensorflow/lite/schema/schema_generated.h>
//#include <tensorflow/lite/version.h>


#ifndef ARENA_PER_MODEL
#define ARENA_PER_MODEL (32 *1024)

#endif

extern void SetupModelResolver(tflite::MicroMutableOpResolver<50>&);
static uint8_t *s_arena = nullptr;
static tflite::MicroMutableOpResolver<50> *s_resolver = nullptr;
static tflite::MicroInterpreter    *s_interpreter = nullptr;

size_t count_models_that_fit() {
  tflite::MicroMutableOpResolver<50> resolver;
  SetupModelResolver(resolver);
  size_t fit = 0;
  for (size_t i = 0; i < g_models_count; ++i) {
    auto *flat = tflite::GetModel(g_models[i].data);
    uint8_t *arena = (uint8_t*)malloc(ARENA_PER_MODEL);
    if (!arena) break;
    tflite::MicroInterpreter test_interp(
      flat, resolver,
      arena, ARENA_PER_MODEL,
      nullptr  // no profiler
    );
    if (test_interp.AllocateTensors() == kTfLiteOk) {
      ++fit;
      free(arena);
    } else {
      free(arena);
      break;
    }
  }
  return fit;
}

void tflite_runner_init(const model_t *models, size_t model_count) {
  if (s_arena) return;  
  s_arena     = (uint8_t*)std::malloc(ARENA_PER_MODEL);
  s_resolver  = new tflite::MicroMutableOpResolver<50>();
  SetupModelResolver(*s_resolver);
}

void tflite_runner_reset_after_timeout() {
  if (s_interpreter) {
    delete s_interpreter;
    s_interpreter = nullptr;
  }
}

int tflite_runner_run(const model_t *model,
                      const event_t *input_data,
                      size_t input_len,
                      int *result,
                      size_t output_len) {
  if (!s_arena || !s_resolver) return -1;
  if (s_interpreter) {
    delete s_interpreter;
    s_interpreter = nullptr;
  }
  const tflite::Model *flat = tflite::GetModel(model->data);
  if (!flat) return -1;
  s_interpreter = new tflite::MicroInterpreter(
    flat,
    *s_resolver,
    s_arena,
    ARENA_PER_MODEL,
    nullptr  
  );
  if (s_interpreter->AllocateTensors() != kTfLiteOk) {
    delete s_interpreter;
    s_interpreter = nullptr;
    return -1;
  }
  TfLiteTensor *in = s_interpreter->input(0);
  memset(in->data.uint8, 0, in->bytes);
  size_t copy_count = input_len;
  if (copy_count > (size_t)in->bytes) copy_count = in->bytes;
  for (size_t i = 0; i < copy_count; ++i) {
    in->data.uint8[i] = (uint8_t)input_data[i];
  }
  if (s_interpreter->Invoke() != kTfLiteOk) return -1;
  TfLiteTensor *out = s_interpreter->output(0);
  int8_t quantized_value = out->data.int8[0]; 
  float sclale = out->params.scale;
  int zero_point = out->params.zero_point;
  float prob = (quantized_value - zero_point) * sclale;
  float threshold = model->threshold;
  *result = (prob > threshold) ? 1 : 0;
  return 0;
}
