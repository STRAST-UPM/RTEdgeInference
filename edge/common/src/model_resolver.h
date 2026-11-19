#ifndef MODEL_RESOLVER_H
#define MODEL_RESOLVER_H
#include "tensorflow/lite/micro/micro_mutable_op_resolver.h"

inline void SetupModelResolver(tflite::MicroMutableOpResolver<50>& resolver) {

    resolver.AddReshape();
    resolver.AddMean();
    resolver.AddSquaredDifference();
    resolver.AddAdd();
    resolver.AddRsqrt();
    resolver.AddMul();
    resolver.AddSub();
    resolver.AddFullyConnected();
    resolver.AddLogistic();
    resolver.AddSplit();
    resolver.AddDepthwiseConv2D();
    resolver.AddConv2D();
    resolver.AddDequantize();
    resolver.AddCast();
    resolver.AddMinimum();
    resolver.AddGather();
    resolver.AddWhile();
    resolver.AddStridedSlice();
    resolver.AddGreater();
    resolver.AddLess();
    resolver.AddLogicalAnd();
    resolver.AddTanh();
    resolver.AddMaximum();
    resolver.AddTranspose();
    resolver.AddSelectV2();
    resolver.AddExpandDims();
    resolver.AddShape();
    resolver.AddPack();
    resolver.AddTransposeConv();
    resolver.AddSoftmax();


}

#endif // MODEL_RESOLVER_H
