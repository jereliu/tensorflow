/* Copyright 2017 The TensorFlow Authors. All Rights Reserved.

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
==============================================================================*/
#include "tensorflow/contrib/lite/toco/tflite/operator.h"

// TODO(ycling): Consider refactoring to extract the LSTM definition out of
// graph_transformation module.
#include "tensorflow/contrib/lite/toco/graph_transformations/lstm_utils.h"
#include "tensorflow/contrib/lite/toco/tflite/builtin_operator.h"
#include "tensorflow/contrib/lite/toco/tflite/custom_operator.h"
#include "tensorflow/contrib/lite/toco/tflite/simple_operator.h"
#include "tensorflow/contrib/lite/toco/tflite/types.h"

#include "tensorflow/core/framework/attr_value.pb.h"
#include "tensorflow/core/framework/node_def.pb.h"

namespace toco {

namespace tflite {

class AveragePool
    : public BuiltinOperator<AveragePoolOperator, ::tflite::Pool2DOptions,
                             ::tflite::BuiltinOptions_Pool2DOptions> {
 public:
  using BuiltinOperator::BuiltinOperator;

  flatbuffers::Offset<TfLiteOptions> WriteOptions(
      const TocoOperator& op,
      flatbuffers::FlatBufferBuilder* builder) const override {
    auto padding = Padding::Serialize(op.padding.type);
    auto activation_function =
        ActivationFunction::Serialize(op.fused_activation_function);
    return ::tflite::CreatePool2DOptions(*builder, padding, op.stride_width,
                                         op.stride_height, op.kwidth,
                                         op.kheight, activation_function);
  }

  void ReadOptions(const TfLiteOptions& options,
                   TocoOperator* op) const override {
    op->padding.type = Padding::Deserialize(options.padding());
    op->stride_width = options.stride_w();
    op->stride_height = options.stride_h();
    op->kwidth = options.filter_width();
    op->kheight = options.filter_height();
    op->fused_activation_function =
        ActivationFunction::Deserialize(options.fused_activation_function());
  }

  int GetVersion(const Operator& op) const override { return 1; }
};

class Convolution
    : public BuiltinOperator<ConvOperator, ::tflite::Conv2DOptions,
                             ::tflite::BuiltinOptions_Conv2DOptions> {
 public:
  using BuiltinOperator::BuiltinOperator;

  flatbuffers::Offset<TfLiteOptions> WriteOptions(
      const TocoOperator& op,
      flatbuffers::FlatBufferBuilder* builder) const override {
    auto padding = Padding::Serialize(op.padding.type);
    auto activation_function =
        ActivationFunction::Serialize(op.fused_activation_function);
    return ::tflite::CreateConv2DOptions(*builder, padding, op.stride_width,
                                         op.stride_height, activation_function,
                                         op.dilation_width_factor,
                                         op.dilation_height_factor);
  }

  void ReadOptions(const TfLiteOptions& options,
                   TocoOperator* op) const override {
    op->padding.type = Padding::Deserialize(options.padding());
    op->stride_width = options.stride_w();
    op->stride_height = options.stride_h();
    op->dilation_width_factor = options.dilation_w_factor();
    op->dilation_height_factor = options.dilation_h_factor();
    op->fused_activation_function =
        ActivationFunction::Deserialize(options.fused_activation_function());
  }

  int GetVersion(const Operator& op) const override { return 1; }
};

class DepthwiseConvolution
    : public BuiltinOperator<DepthwiseConvOperator,
                             ::tflite::DepthwiseConv2DOptions,
                             ::tflite::BuiltinOptions_DepthwiseConv2DOptions> {
 public:
  using BuiltinOperator::BuiltinOperator;

  flatbuffers::Offset<TfLiteOptions> WriteOptions(
      const TocoOperator& op,
      flatbuffers::FlatBufferBuilder* builder) const override {
    auto padding = Padding::Serialize(op.padding.type);
    auto activation_function =
        ActivationFunction::Serialize(op.fused_activation_function);
    return ::tflite::CreateDepthwiseConv2DOptions(
        *builder, padding, op.stride_width, op.stride_height,
        op.depth_multiplier, activation_function);
  }

  void ReadOptions(const TfLiteOptions& options,
                   TocoOperator* op) const override {
    op->padding.type = Padding::Deserialize(options.padding());
    op->stride_width = options.stride_w();
    op->stride_height = options.stride_h();
    op->depth_multiplier = options.depth_multiplier();
    op->fused_activation_function =
        ActivationFunction::Deserialize(options.fused_activation_function());
  }

  int GetVersion(const Operator& op) const override { return 1; }
};

class Add : public BuiltinOperator<AddOperator, ::tflite::AddOptions,
                                   ::tflite::BuiltinOptions_AddOptions> {
 public:
  using BuiltinOperator::BuiltinOperator;

  flatbuffers::Offset<TfLiteOptions> WriteOptions(
      const TocoOperator& op,
      flatbuffers::FlatBufferBuilder* builder) const override {
    auto activation_function =
        ActivationFunction::Serialize(op.fused_activation_function);
    return ::tflite::CreateAddOptions(*builder, activation_function);
  }

  void ReadOptions(const TfLiteOptions& options,
                   TocoOperator* op) const override {
    op->fused_activation_function =
        ActivationFunction::Deserialize(options.fused_activation_function());
  }

  int GetVersion(const Operator& op) const override { return 1; }
};

class SpaceToBatchND
    : public BuiltinOperator<SpaceToBatchNDOperator,
                             ::tflite::SpaceToBatchNDOptions,
                             ::tflite::BuiltinOptions_SpaceToBatchNDOptions> {
 public:
  using BuiltinOperator::BuiltinOperator;

  flatbuffers::Offset<TfLiteOptions> WriteOptions(
      const TocoOperator& op,
      flatbuffers::FlatBufferBuilder* builder) const override {
    return ::tflite::CreateSpaceToBatchNDOptions(*builder);
  }

  void ReadOptions(const TfLiteOptions& options,
                   TocoOperator* op) const override {}

  int GetVersion(const Operator& op) const override { return 1; }
};

class Sub : public BuiltinOperator<SubOperator, ::tflite::SubOptions,
                                   ::tflite::BuiltinOptions_SubOptions> {
 public:
  using BuiltinOperator::BuiltinOperator;

  flatbuffers::Offset<TfLiteOptions> WriteOptions(
      const TocoOperator& op,
      flatbuffers::FlatBufferBuilder* builder) const override {
    auto activation_function =
        ActivationFunction::Serialize(op.fused_activation_function);
    return ::tflite::CreateSubOptions(*builder, activation_function);
  }

  void ReadOptions(const TfLiteOptions& options,
                   TocoOperator* op) const override {
    op->fused_activation_function =
        ActivationFunction::Deserialize(options.fused_activation_function());
  }

  int GetVersion(const Operator& op) const override { return 1; }
};

class Div : public BuiltinOperator<DivOperator, ::tflite::DivOptions,
                                   ::tflite::BuiltinOptions_DivOptions> {
 public:
  using BuiltinOperator::BuiltinOperator;

  flatbuffers::Offset<TfLiteOptions> WriteOptions(
      const TocoOperator& op,
      flatbuffers::FlatBufferBuilder* builder) const override {
    auto activation_function =
        ActivationFunction::Serialize(op.fused_activation_function);
    return ::tflite::CreateDivOptions(*builder, activation_function);
  }

  void ReadOptions(const TfLiteOptions& options,
                   TocoOperator* op) const override {
    op->fused_activation_function =
        ActivationFunction::Deserialize(options.fused_activation_function());
  }

  int GetVersion(const Operator& op) const override { return 1; }
};

class BatchToSpaceND
    : public BuiltinOperator<BatchToSpaceNDOperator,
                             ::tflite::BatchToSpaceNDOptions,
                             ::tflite::BuiltinOptions_BatchToSpaceNDOptions> {
 public:
  using BuiltinOperator::BuiltinOperator;

  flatbuffers::Offset<TfLiteOptions> WriteOptions(
      const TocoOperator& op,
      flatbuffers::FlatBufferBuilder* builder) const override {
    return ::tflite::CreateBatchToSpaceNDOptions(*builder);
  }

  void ReadOptions(const TfLiteOptions& options,
                   TocoOperator* op) const override {}

  int GetVersion(const Operator& op) const override { return 1; }
};

class Cast : public BuiltinOperator<CastOperator, ::tflite::CastOptions,
                                    ::tflite::BuiltinOptions_CastOptions> {
 public:
  using BuiltinOperator::BuiltinOperator;
  flatbuffers::Offset<TfLiteOptions> WriteOptions(
      const TocoOperator& op,
      flatbuffers::FlatBufferBuilder* builder) const override {
    return ::tflite::CreateCastOptions(*builder,
                                       DataType::Serialize(op.src_data_type),
                                       DataType::Serialize(op.dst_data_type));
  }

  void ReadOptions(const TfLiteOptions& options,
                   TocoOperator* op) const override {
    op->src_data_type = DataType::Deserialize(options.in_data_type());
    op->dst_data_type = DataType::Deserialize(options.out_data_type());
  }

  int GetVersion(const Operator& op) const override { return 1; }
};

class Concatenation
    : public BuiltinOperator<ConcatenationOperator,
                             ::tflite::ConcatenationOptions,
                             ::tflite::BuiltinOptions_ConcatenationOptions> {
 public:
  using BuiltinOperator::BuiltinOperator;
  flatbuffers::Offset<TfLiteOptions> WriteOptions(
      const TocoOperator& op,
      flatbuffers::FlatBufferBuilder* builder) const override {
    return ::tflite::CreateConcatenationOptions(*builder, op.axis);
  }

  void ReadOptions(const TfLiteOptions& options,
                   TocoOperator* op) const override {
    op->axis = options.axis();
  }

  int GetVersion(const Operator& op) const override { return 1; }
};

class DepthToSpace : public CustomOperator<DepthToSpaceOperator> {
 public:
  using CustomOperator::CustomOperator;
  void WriteOptions(const TocoOperator& op,
                    flexbuffers::Builder* fbb) const override {
    fbb->Int("block_size", op.block_size);
  }
  void ReadOptions(const flexbuffers::Map& m, TocoOperator* op) const override {
    op->block_size = m["block_size"].AsInt64();
  }

  int GetVersion(const Operator& op) const override { return 1; }
};

class FakeQuant
    : public BuiltinOperator<FakeQuantOperator, ::tflite::FakeQuantOptions,
                             ::tflite::BuiltinOptions_FakeQuantOptions> {
 public:
  using BuiltinOperator::BuiltinOperator;
  flatbuffers::Offset<TfLiteOptions> WriteOptions(
      const TocoOperator& op,
      flatbuffers::FlatBufferBuilder* builder) const override {
    return ::tflite::CreateFakeQuantOptions(
        *builder, op.minmax->min, op.minmax->max, op.num_bits, op.narrow_range);
  }
  void ReadOptions(const TfLiteOptions& options,
                   TocoOperator* op) const override {
    auto* minmax = new MinMax;
    minmax->min = options.min();
    minmax->max = options.max();
    op->minmax.reset(minmax);
    op->num_bits = options.num_bits();
    op->narrow_range = options.narrow_range();
  }

  int GetVersion(const Operator& op) const override {
    const auto& fq_op = static_cast<const FakeQuantOperator&>(op);
    return fq_op.narrow_range ? 2 : 1;
  }
};

class FullyConnected
    : public BuiltinOperator<FullyConnectedOperator,
                             ::tflite::FullyConnectedOptions,
                             ::tflite::BuiltinOptions_FullyConnectedOptions> {
 public:
  using BuiltinOperator::BuiltinOperator;
  flatbuffers::Offset<TfLiteOptions> WriteOptions(
      const TocoOperator& op,
      flatbuffers::FlatBufferBuilder* builder) const override {
    auto activation_function =
        ActivationFunction::Serialize(op.fused_activation_function);
    ::tflite::FullyConnectedOptionsWeightsFormat tflite_weights_format;
    switch (op.weights_format) {
      case FullyConnectedWeightsFormat::kDefault:
        tflite_weights_format =
            ::tflite::FullyConnectedOptionsWeightsFormat_DEFAULT;
        break;
      case FullyConnectedWeightsFormat::kShuffled4x16Int8:
        tflite_weights_format =
            ::tflite::FullyConnectedOptionsWeightsFormat_SHUFFLED4x16INT8;
        break;
      default:
        LOG(ERROR) << "Unhandled FC weights format";
        tflite_weights_format =
            ::tflite::FullyConnectedOptionsWeightsFormat_DEFAULT;
    }
    return ::tflite::CreateFullyConnectedOptions(*builder, activation_function,
                                                 tflite_weights_format);
  }

  void ReadOptions(const TfLiteOptions& options,
                   TocoOperator* op) const override {
    op->fused_activation_function =
        ActivationFunction::Deserialize(options.fused_activation_function());
    switch (options.weights_format()) {
      case ::tflite::FullyConnectedOptionsWeightsFormat_DEFAULT:
        op->weights_format = FullyConnectedWeightsFormat::kDefault;
        break;
      case ::tflite::FullyConnectedOptionsWeightsFormat_SHUFFLED4x16INT8:
        op->weights_format = FullyConnectedWeightsFormat::kShuffled4x16Int8;
        break;
      default:
        LOG(ERROR) << "Unhandled FC weights format";
        op->weights_format = FullyConnectedWeightsFormat::kDefault;
    }
  }

  int GetVersion(const Operator& op) const override {
    const auto& fc_op = static_cast<const FullyConnectedOperator&>(op);
    return fc_op.weights_format == FullyConnectedWeightsFormat::kDefault ? 1
                                                                         : 2;
  }
};

class Gather : public BuiltinOperator<GatherOperator, ::tflite::GatherOptions,
                                      ::tflite::BuiltinOptions_GatherOptions> {
 public:
  using BuiltinOperator::BuiltinOperator;
  flatbuffers::Offset<TfLiteOptions> WriteOptions(
      const TocoOperator& op,
      flatbuffers::FlatBufferBuilder* builder) const override {
    int axis = op.axis ? op.axis.value() : 0;
    return ::tflite::CreateGatherOptions(*builder, axis);
  }

  void ReadOptions(const TfLiteOptions& options,
                   TocoOperator* op) const override {
    op->axis = {options.axis()};
  }

  int GetVersion(const Operator& op) const override { return 1; }
};

class Svdf : public BuiltinOperator<SvdfOperator, ::tflite::SVDFOptions,
                                    ::tflite::BuiltinOptions_SVDFOptions> {
 public:
  using BuiltinOperator::BuiltinOperator;
  flatbuffers::Offset<TfLiteOptions> WriteOptions(
      const TocoOperator& op,
      flatbuffers::FlatBufferBuilder* builder) const override {
    auto activation_function =
        ActivationFunction::Serialize(op.fused_activation_function);
    return ::tflite::CreateSVDFOptions(*builder, op.rank, activation_function);
  }

  void ReadOptions(const TfLiteOptions& options,
                   TocoOperator* op) const override {
    op->fused_activation_function =
        ActivationFunction::Deserialize(options.fused_activation_function());
    op->rank = options.rank();
  }

  int GetVersion(const Operator& op) const override { return 1; }
};

class L2Normalization
    : public BuiltinOperator<L2NormalizationOperator, ::tflite::L2NormOptions,
                             ::tflite::BuiltinOptions_L2NormOptions> {
 public:
  using BuiltinOperator::BuiltinOperator;
  flatbuffers::Offset<TfLiteOptions> WriteOptions(
      const TocoOperator& op,
      flatbuffers::FlatBufferBuilder* builder) const override {
    auto activation_function =
        ActivationFunction::Serialize(op.fused_activation_function);
    return ::tflite::CreateL2NormOptions(*builder, activation_function);
  }

  void ReadOptions(const TfLiteOptions& options,
                   TocoOperator* op) const override {
    op->fused_activation_function =
        ActivationFunction::Deserialize(options.fused_activation_function());
  }

  int GetVersion(const Operator& op) const override { return 1; }
};

class L2Pool : public BuiltinOperator<L2PoolOperator, ::tflite::Pool2DOptions,
                                      ::tflite::BuiltinOptions_Pool2DOptions> {
 public:
  using BuiltinOperator::BuiltinOperator;
  flatbuffers::Offset<TfLiteOptions> WriteOptions(
      const TocoOperator& op,
      flatbuffers::FlatBufferBuilder* builder) const override {
    auto padding = Padding::Serialize(op.padding.type);
    auto activation_function =
        ActivationFunction::Serialize(op.fused_activation_function);
    return ::tflite::CreatePool2DOptions(*builder, padding, op.stride_width,
                                         op.stride_height, op.kwidth,
                                         op.kheight, activation_function);
  }

  void ReadOptions(const TfLiteOptions& options,
                   TocoOperator* op) const override {
    op->padding.type = Padding::Deserialize(options.padding());
    op->stride_width = options.stride_w();
    op->stride_height = options.stride_h();
    op->kwidth = options.filter_width();
    op->kheight = options.filter_height();
    op->fused_activation_function =
        ActivationFunction::Deserialize(options.fused_activation_function());
  }

  int GetVersion(const Operator& op) const override { return 1; }
};

class LocalResponseNormalization
    : public BuiltinOperator<
          LocalResponseNormalizationOperator,
          ::tflite::LocalResponseNormalizationOptions,
          ::tflite::BuiltinOptions_LocalResponseNormalizationOptions> {
 public:
  using BuiltinOperator::BuiltinOperator;
  flatbuffers::Offset<TfLiteOptions> WriteOptions(
      const TocoOperator& op,
      flatbuffers::FlatBufferBuilder* builder) const override {
    return ::tflite::CreateLocalResponseNormalizationOptions(
        *builder, op.range, op.bias, op.alpha, op.beta);
  }

  void ReadOptions(const TfLiteOptions& options,
                   TocoOperator* op) const override {
    op->range = options.radius();
    op->bias = options.bias();
    op->alpha = options.alpha();
    op->beta = options.beta();
  }

  int GetVersion(const Operator& op) const override { return 1; }
};

class MaxPool : public BuiltinOperator<MaxPoolOperator, ::tflite::Pool2DOptions,
                                       ::tflite::BuiltinOptions_Pool2DOptions> {
 public:
  using BuiltinOperator::BuiltinOperator;
  flatbuffers::Offset<TfLiteOptions> WriteOptions(
      const TocoOperator& op,
      flatbuffers::FlatBufferBuilder* builder) const override {
    auto padding = Padding::Serialize(op.padding.type);
    auto activation_function =
        ActivationFunction::Serialize(op.fused_activation_function);
    return ::tflite::CreatePool2DOptions(*builder, padding, op.stride_width,
                                         op.stride_height, op.kwidth,
                                         op.kheight, activation_function);
  }

  void ReadOptions(const TfLiteOptions& options,
                   TocoOperator* op) const override {
    op->padding.type = Padding::Deserialize(options.padding());
    op->stride_width = options.stride_w();
    op->stride_height = options.stride_h();
    op->kwidth = options.filter_width();
    op->kheight = options.filter_height();
    op->fused_activation_function =
        ActivationFunction::Deserialize(options.fused_activation_function());
  }

  int GetVersion(const Operator& op) const override { return 1; }
};

class Mul : public BuiltinOperator<MulOperator, ::tflite::MulOptions,
                                   ::tflite::BuiltinOptions_MulOptions> {
 public:
  using BuiltinOperator::BuiltinOperator;

  flatbuffers::Offset<TfLiteOptions> WriteOptions(
      const TocoOperator& op,
      flatbuffers::FlatBufferBuilder* builder) const override {
    auto activation_function =
        ActivationFunction::Serialize(op.fused_activation_function);
    return ::tflite::CreateMulOptions(*builder, activation_function);
  }

  void ReadOptions(const TfLiteOptions& options,
                   TocoOperator* op) const override {
    op->fused_activation_function =
        ActivationFunction::Deserialize(options.fused_activation_function());
  }

  int GetVersion(const Operator& op) const override { return 1; }
};

class Pad : public BuiltinOperator<PadOperator, ::tflite::PadOptions,
                                   ::tflite::BuiltinOptions_PadOptions> {
 public:
  using BuiltinOperator::BuiltinOperator;

  flatbuffers::Offset<TfLiteOptions> WriteOptions(
      const TocoOperator& op,
      flatbuffers::FlatBufferBuilder* builder) const override {
    return ::tflite::CreatePadOptions(*builder);
  }

  void ReadOptions(const TfLiteOptions& options,
                   TocoOperator* op) const override {}

  int GetVersion(const Operator& op) const override { return 1; }
};

class Tile
    : public BuiltinOperator<TensorFlowTileOperator, ::tflite::TileOptions,
                             ::tflite::BuiltinOptions_TileOptions> {
  using BuiltinOperator::BuiltinOperator;

  flatbuffers::Offset<TfLiteOptions> WriteOptions(
      const TocoOperator& op,
      flatbuffers::FlatBufferBuilder* builder) const override {
    return ::tflite::CreateTileOptions(*builder);
  }

  void ReadOptions(const TfLiteOptions& options,
                   TocoOperator* op) const override {}
  int GetVersion(const Operator& op) const override { return 1; }
};

class PadV2 : public BuiltinOperator<PadV2Operator, ::tflite::PadV2Options,
                                     ::tflite::BuiltinOptions_PadV2Options> {
 public:
  using BuiltinOperator::BuiltinOperator;

  flatbuffers::Offset<TfLiteOptions> WriteOptions(
      const TocoOperator& op,
      flatbuffers::FlatBufferBuilder* builder) const override {
    return ::tflite::CreatePadV2Options(*builder);
  }

  void ReadOptions(const TfLiteOptions& options,
                   TocoOperator* op) const override {}

  int GetVersion(const Operator& op) const override { return 1; }
};

class Reshape
    : public BuiltinOperator<TensorFlowReshapeOperator,
                             ::tflite::ReshapeOptions,
                             ::tflite::BuiltinOptions_ReshapeOptions> {
 public:
  using BuiltinOperator::BuiltinOperator;

  flatbuffers::Offset<TfLiteOptions> WriteOptions(
      const TocoOperator& op,
      flatbuffers::FlatBufferBuilder* builder) const override {
    return ::tflite::CreateReshapeOptions(*builder,
                                          builder->CreateVector(op.shape));
  }

  void ReadOptions(const TfLiteOptions& options,
                   TocoOperator* op) const override {
    op->shape.insert(op->shape.end(), options.new_shape()->begin(),
                     options.new_shape()->end());
  }

  int GetVersion(const Operator& op) const override { return 1; }
};

class Softmax
    : public BuiltinOperator<SoftmaxOperator, ::tflite::SoftmaxOptions,
                             ::tflite::BuiltinOptions_SoftmaxOptions> {
 public:
  using BuiltinOperator::BuiltinOperator;
  flatbuffers::Offset<TfLiteOptions> WriteOptions(
      const TocoOperator& op,
      flatbuffers::FlatBufferBuilder* builder) const override {
    return ::tflite::CreateSoftmaxOptions(*builder, op.beta);
  }

  void ReadOptions(const TfLiteOptions& options,
                   TocoOperator* op) const override {
    op->beta = options.beta();
  }

  int GetVersion(const Operator& op) const override { return 1; }
};

class SpaceToDepth
    : public BuiltinOperator<SpaceToDepthOperator,
                             ::tflite::SpaceToDepthOptions,
                             ::tflite::BuiltinOptions_SpaceToDepthOptions> {
 public:
  using BuiltinOperator::BuiltinOperator;
  flatbuffers::Offset<TfLiteOptions> WriteOptions(
      const TocoOperator& op,
      flatbuffers::FlatBufferBuilder* builder) const override {
    return ::tflite::CreateSpaceToDepthOptions(*builder, op.block_size);
  }

  void ReadOptions(const TfLiteOptions& options,
                   TocoOperator* op) const override {
    op->block_size = options.block_size();
  }

  int GetVersion(const Operator& op) const override { return 1; }
};

class Transpose
    : public BuiltinOperator<TransposeOperator, ::tflite::TransposeOptions,
                             ::tflite::BuiltinOptions_TransposeOptions> {
 public:
  using BuiltinOperator::BuiltinOperator;
  flatbuffers::Offset<TfLiteOptions> WriteOptions(
      const TocoOperator& op,
      flatbuffers::FlatBufferBuilder* builder) const override {
    return ::tflite::CreateTransposeOptions(*builder);
  }

  void ReadOptions(const TfLiteOptions& options,
                   TocoOperator* op) const override {}

  int GetVersion(const Operator& op) const override { return 1; }
};

class Lstm : public BuiltinOperator<LstmCellOperator, ::tflite::LSTMOptions,
                                    ::tflite::BuiltinOptions_LSTMOptions> {
 public:
  using BuiltinOperator::BuiltinOperator;
  flatbuffers::Offset<TfLiteOptions> WriteOptions(
      const TocoOperator& op,
      flatbuffers::FlatBufferBuilder* builder) const override {
    ::tflite::LSTMKernelType kernel_type;
    switch (op.kernel_type) {
      case LstmCellOperator::KERNEL_BASIC:
        kernel_type = ::tflite::LSTMKernelType_BASIC;
        break;
      case LstmCellOperator::KERNEL_FULL:
        kernel_type = ::tflite::LSTMKernelType_FULL;
        break;
    }

    // Current toco converter only supports tanh, no clip.
    return ::tflite::CreateLSTMOptions(*builder, /*fused_activation_function=*/
                                       ::tflite::ActivationFunctionType_TANH,
                                       /*cell_clip=*/0.0,
                                       /*proj_clip=*/0.0, kernel_type);
  }

  void ReadOptions(const TfLiteOptions& options,
                   TocoOperator* op) const override {
    // Only support tanh activation, so check that tflite type is tanh.
    CHECK(options.fused_activation_function() ==
          ::tflite::ActivationFunctionType_TANH);

    switch (options.kernel_type()) {
      case ::tflite::LSTMKernelType_BASIC:
        op->kernel_type = LstmCellOperator::KERNEL_BASIC;
        break;
      case ::tflite::LSTMKernelType_FULL:
        op->kernel_type = LstmCellOperator::KERNEL_FULL;
        break;
    }
  }

  int GetVersion(const Operator& op) const override {
    const auto& lstm_op = static_cast<const LstmCellOperator&>(op);
    switch (lstm_op.kernel_type) {
      case LstmCellOperator::KERNEL_FULL:
        return 1;
      case LstmCellOperator::KERNEL_BASIC:
        return 2;
    }
  }

  std::vector<bool> GetMutatingInputVariables(
      const Operator& op) const override {
    const auto& lstm_op = static_cast<const LstmCellOperator&>(op);

    std::vector<bool> mutating_input_variables(op.inputs.size(), false);
    switch (lstm_op.kernel_type) {
      case LstmCellOperator::KERNEL_FULL: {
        mutating_input_variables[kInputActivationStateTensor] = true;
        mutating_input_variables[kInputCellStateTensor] = true;
        break;
      }
      case LstmCellOperator::KERNEL_BASIC: {
        mutating_input_variables[LstmCellOperator::PREV_ACTIV_INPUT] = true;
        mutating_input_variables[LstmCellOperator::PREV_STATE_INPUT] = true;
        break;
      }
    }
    return mutating_input_variables;
  }
};

class Mean : public BuiltinOperator<MeanOperator, ::tflite::ReducerOptions,
                                    ::tflite::BuiltinOptions_ReducerOptions> {
 public:
  using BuiltinOperator::BuiltinOperator;
  flatbuffers::Offset<TfLiteOptions> WriteOptions(
      const TocoOperator& op,
      flatbuffers::FlatBufferBuilder* builder) const override {
    return ::tflite::CreateReducerOptions(*builder, op.keep_dims);
  }

  void ReadOptions(const TfLiteOptions& options,
                   TocoOperator* op) const override {
    op->keep_dims = options.keep_dims();
  }

  int GetVersion(const Operator& op) const override { return 1; }
};

class Sum
    : public BuiltinOperator<TensorFlowSumOperator, ::tflite::ReducerOptions,
                             ::tflite::BuiltinOptions_ReducerOptions> {
 public:
  using BuiltinOperator::BuiltinOperator;
  flatbuffers::Offset<TfLiteOptions> WriteOptions(
      const TocoOperator& op,
      flatbuffers::FlatBufferBuilder* builder) const override {
    return ::tflite::CreateReducerOptions(*builder, op.keep_dims);
  }

  void ReadOptions(const TfLiteOptions& options,
                   TocoOperator* op) const override {
    op->keep_dims = options.keep_dims();
  }

  int GetVersion(const Operator& op) const override { return 1; }
};

class ReduceMax
    : public BuiltinOperator<TensorFlowSumOperator, ::tflite::ReducerOptions,
                             ::tflite::BuiltinOptions_ReducerOptions> {
 public:
  using BuiltinOperator::BuiltinOperator;
  flatbuffers::Offset<TfLiteOptions> WriteOptions(
      const TocoOperator& op,
      flatbuffers::FlatBufferBuilder* builder) const override {
    return ::tflite::CreateReducerOptions(*builder, op.keep_dims);
  }

  void ReadOptions(const TfLiteOptions& options,
                   TocoOperator* op) const override {
    op->keep_dims = options.keep_dims();
  }

  int GetVersion(const Operator& op) const override { return 1; }
};

class ReduceProd
    : public BuiltinOperator<TensorFlowSumOperator, ::tflite::ReducerOptions,
                             ::tflite::BuiltinOptions_ReducerOptions> {
 public:
  using BuiltinOperator::BuiltinOperator;
  flatbuffers::Offset<TfLiteOptions> WriteOptions(
      const TocoOperator& op,
      flatbuffers::FlatBufferBuilder* builder) const override {
    return ::tflite::CreateReducerOptions(*builder, op.keep_dims);
  }

  void ReadOptions(const TfLiteOptions& options,
                   TocoOperator* op) const override {
    op->keep_dims = options.keep_dims();
  }

  int GetVersion(const Operator& op) const override { return 1; }
};

class ResizeBilinear
    : public BuiltinOperator<ResizeBilinearOperator,
                             ::tflite::ResizeBilinearOptions,
                             ::tflite::BuiltinOptions_ResizeBilinearOptions> {
 public:
  using BuiltinOperator::BuiltinOperator;
  flatbuffers::Offset<TfLiteOptions> WriteOptions(
      const TocoOperator& op,
      flatbuffers::FlatBufferBuilder* builder) const override {
    return ::tflite::CreateResizeBilinearOptions(*builder, op.align_corners);
  }

  void ReadOptions(const TfLiteOptions& options,
                   TocoOperator* op) const override {
    op->align_corners = options.align_corners();
  }

  int GetVersion(const Operator& op) const override { return 1; }
};

class Squeeze
    : public BuiltinOperator<SqueezeOperator, ::tflite::SqueezeOptions,
                             ::tflite::BuiltinOptions_SqueezeOptions> {
 public:
  using BuiltinOperator::BuiltinOperator;

  flatbuffers::Offset<TfLiteOptions> WriteOptions(
      const TocoOperator& op,
      flatbuffers::FlatBufferBuilder* builder) const override {
    auto squeeze_dims = builder->CreateVector(op.squeeze_dims);
    return ::tflite::CreateSqueezeOptions(*builder, squeeze_dims);
  }

  void ReadOptions(const TfLiteOptions& options,
                   TocoOperator* op) const override {
    op->squeeze_dims.insert(op->squeeze_dims.end(),
                            options.squeeze_dims()->begin(),
                            options.squeeze_dims()->end());
  }

  int GetVersion(const Operator& op) const override { return 1; }
};

class Split
    : public BuiltinOperator<TensorFlowSplitOperator, ::tflite::SplitOptions,
                             ::tflite::BuiltinOptions_SplitOptions> {
 public:
  using BuiltinOperator::BuiltinOperator;

  flatbuffers::Offset<TfLiteOptions> WriteOptions(
      const TocoOperator& op,
      flatbuffers::FlatBufferBuilder* builder) const override {
    return ::tflite::CreateSplitOptions(*builder, op.num_split);
  }

  void ReadOptions(const TfLiteOptions& options,
                   TocoOperator* op) const override {
    op->num_split = options.num_splits();
  }

  int GetVersion(const Operator& op) const override { return 1; }
};

class StridedSlice
    : public BuiltinOperator<StridedSliceOperator,
                             ::tflite::StridedSliceOptions,
                             ::tflite::BuiltinOptions_StridedSliceOptions> {
 public:
  using BuiltinOperator::BuiltinOperator;
  flatbuffers::Offset<TfLiteOptions> WriteOptions(
      const TocoOperator& op,
      flatbuffers::FlatBufferBuilder* builder) const override {
    return ::tflite::CreateStridedSliceOptions(
        *builder, op.begin_mask, op.end_mask, op.ellipsis_mask,
        op.new_axis_mask, op.shrink_axis_mask);
  }

  void ReadOptions(const TfLiteOptions& options,
                   TocoOperator* op) const override {
    op->begin_mask = options.begin_mask();
    op->end_mask = options.end_mask();
    op->ellipsis_mask = options.ellipsis_mask();
    op->new_axis_mask = options.new_axis_mask();
    op->shrink_axis_mask = options.shrink_axis_mask();
  }

  int GetVersion(const Operator& op) const override { return 1; }
};

class TopK_V2 : public BuiltinOperator<TopKV2Operator, ::tflite::TopKV2Options,
                                       ::tflite::BuiltinOptions_TopKV2Options> {
 public:
  using BuiltinOperator::BuiltinOperator;
  flatbuffers::Offset<TfLiteOptions> WriteOptions(
      const TocoOperator& op,
      flatbuffers::FlatBufferBuilder* builder) const override {
    return ::tflite::CreateTopKV2Options(*builder);
  }

  void ReadOptions(const TfLiteOptions& options,
                   TocoOperator* op) const override {}

  int GetVersion(const Operator& op) const override { return 1; }
};

class ArgMax : public BuiltinOperator<ArgMaxOperator, ::tflite::ArgMaxOptions,
                                      ::tflite::BuiltinOptions_ArgMaxOptions> {
 public:
  using BuiltinOperator::BuiltinOperator;
  flatbuffers::Offset<TfLiteOptions> WriteOptions(
      const TocoOperator& op,
      flatbuffers::FlatBufferBuilder* builder) const override {
    return ::tflite::CreateArgMaxOptions(
        *builder, DataType::Serialize(op.output_data_type));
  }

  void ReadOptions(const TfLiteOptions& options,
                   TocoOperator* op) const override {
    op->output_data_type = DataType::Deserialize(options.output_type());
  }

  int GetVersion(const Operator& op) const override { return 1; }
};

class ArgMin : public BuiltinOperator<ArgMinOperator, ::tflite::ArgMinOptions,
                                      ::tflite::BuiltinOptions_ArgMinOptions> {
 public:
  using BuiltinOperator::BuiltinOperator;
  flatbuffers::Offset<TfLiteOptions> WriteOptions(
      const TocoOperator& op,
      flatbuffers::FlatBufferBuilder* builder) const override {
    return ::tflite::CreateArgMinOptions(
        *builder, DataType::Serialize(op.output_data_type));
  }

  void ReadOptions(const TfLiteOptions& options,
                   TocoOperator* op) const override {
    op->output_data_type = DataType::Deserialize(options.output_type());
  }

  int GetVersion(const Operator& op) const override { return 1; }
};

class TransposeConv
    : public BuiltinOperator<TransposeConvOperator,
                             ::tflite::TransposeConvOptions,
                             ::tflite::BuiltinOptions_TransposeConvOptions> {
 public:
  using BuiltinOperator::BuiltinOperator;

  flatbuffers::Offset<TfLiteOptions> WriteOptions(
      const TocoOperator& op,
      flatbuffers::FlatBufferBuilder* builder) const override {
    auto padding = Padding::Serialize(op.padding.type);
    return ::tflite::CreateTransposeConvOptions(
        *builder, padding, op.stride_width, op.stride_height);
  }

  void ReadOptions(const TfLiteOptions& options,
                   TocoOperator* op) const override {
    op->padding.type = Padding::Deserialize(options.padding());
    op->stride_width = options.stride_w();
    op->stride_height = options.stride_h();
  }

  int GetVersion(const Operator& op) const override { return 1; }
};

class SparseToDense
    : public BuiltinOperator<SparseToDenseOperator,
                             ::tflite::SparseToDenseOptions,
                             ::tflite::BuiltinOptions_SparseToDenseOptions> {
 public:
  using BuiltinOperator::BuiltinOperator;

  flatbuffers::Offset<TfLiteOptions> WriteOptions(
      const TocoOperator& op,
      flatbuffers::FlatBufferBuilder* builder) const override {
    return ::tflite::CreateSparseToDenseOptions(*builder, op.validate_indices);
  }

  void ReadOptions(const TfLiteOptions& options,
                   TocoOperator* op) const override {
    op->validate_indices = options.validate_indices();
  }

  int GetVersion(const Operator& op) const override { return 1; }
};

class ExpandDims
    : public BuiltinOperator<ExpandDimsOperator, ::tflite::ExpandDimsOptions,
                             ::tflite::BuiltinOptions_ExpandDimsOptions> {
 public:
  using BuiltinOperator::BuiltinOperator;

  flatbuffers::Offset<TfLiteOptions> WriteOptions(
      const TocoOperator& op,
      flatbuffers::FlatBufferBuilder* builder) const override {
    return ::tflite::CreateExpandDimsOptions(*builder);
  }

  void ReadOptions(const TfLiteOptions& options,
                   TocoOperator* op) const override {}

  int GetVersion(const Operator& op) const override { return 1; }
};

class Pack : public BuiltinOperator<PackOperator, ::tflite::PackOptions,
                                    ::tflite::BuiltinOptions_PackOptions> {
 public:
  using BuiltinOperator::BuiltinOperator;

  flatbuffers::Offset<TfLiteOptions> WriteOptions(
      const TocoOperator& op,
      flatbuffers::FlatBufferBuilder* builder) const override {
    return ::tflite::CreatePackOptions(*builder, op.values_count, op.axis);
  }

  void ReadOptions(const TfLiteOptions& options,
                   TocoOperator* op) const override {
    op->values_count = options.values_count();
    op->axis = options.axis();
  }

  int GetVersion(const Operator& op) const override { return 1; }
};

class Shape
    : public BuiltinOperator<TensorFlowShapeOperator, ::tflite::ShapeOptions,
                             ::tflite::BuiltinOptions_ShapeOptions> {
 public:
  using BuiltinOperator::BuiltinOperator;
  flatbuffers::Offset<TfLiteOptions> WriteOptions(
      const TocoOperator& op,
      flatbuffers::FlatBufferBuilder* builder) const override {
    return ::tflite::CreateShapeOptions(
        *builder, DataType::Serialize(op.output_data_type));
  }

  void ReadOptions(const TfLiteOptions& options,
                   TocoOperator* op) const override {
    op->output_data_type = DataType::Deserialize(options.out_type());
  }

  int GetVersion(const Operator& op) const override { return 1; }
};

class OneHot : public BuiltinOperator<OneHotOperator, ::tflite::OneHotOptions,
                                      ::tflite::BuiltinOptions_OneHotOptions> {
 public:
  using BuiltinOperator::BuiltinOperator;
  flatbuffers::Offset<TfLiteOptions> WriteOptions(
      const TocoOperator& op,
      flatbuffers::FlatBufferBuilder* builder) const override {
    return ::tflite::CreateOneHotOptions(*builder, op.axis);
  }
  void ReadOptions(const TfLiteOptions& options,
                   TocoOperator* op) const override {
    op->axis = options.axis();
  }

  int GetVersion(const Operator& op) const override { return 1; }
};

class CTCBeamSearchDecoder
    : public CustomOperator<CTCBeamSearchDecoderOperator> {
 public:
  using CustomOperator::CustomOperator;

  void WriteOptions(const TocoOperator& op,
                    flexbuffers::Builder* fbb) const override {
    fbb->Int("beam_width", op.beam_width);
    fbb->Int("top_paths", op.top_paths);
    fbb->Bool("merge_repeated", op.merge_repeated);
  }

  void ReadOptions(const flexbuffers::Map& m, TocoOperator* op) const override {
    op->beam_width = m["beam_width"].AsInt32();
    op->top_paths = m["top_paths"].AsInt32();
    op->merge_repeated = m["merge_repeated"].AsBool();
  }

  int GetVersion(const Operator& op) const override { return 1; }
};

class TensorFlowUnsupported : public BaseOperator {
 public:
  using BaseOperator::BaseOperator;

  Options Serialize(const Operator& op,
                    flatbuffers::FlatBufferBuilder* builder) const override {
    auto fbb =
        WriteOptions(static_cast<const TensorFlowUnsupportedOperator&>(op));
    if (fbb) {
      return Options::Custom(builder->CreateVector(fbb->GetBuffer()));
    } else {
      return Options::Custom(0);
    }
  }

  std::unique_ptr<Operator> Deserialize(
      const BuiltinOptions* builtin_options,
      const CustomOptions* custom_options) const override {
    auto op = absl::make_unique<TensorFlowUnsupportedOperator>();
    if (custom_options) {
      auto flexbuffer_map =
          flexbuffers::GetRoot(custom_options->data(), custom_options->size())
              .AsMap();
      ReadOptions(flexbuffer_map, op.get());
    }
    return std::unique_ptr<Operator>(op.release());
  }

  std::unique_ptr<flexbuffers::Builder> WriteOptions(
      const TensorFlowUnsupportedOperator& op) const {
    auto fbb = absl::make_unique<flexbuffers::Builder>();

    ::tensorflow::NodeDef node_def;
    if (!node_def.ParseFromString(op.tensorflow_node_def)) {
      LOG(ERROR) << "Failed to parse TensorFlow NodeDef";
      return std::unique_ptr<flexbuffers::Builder>();
    }

    bool has_valid_attr = false;
    size_t map_start = fbb->StartMap();
    for (const auto& pair : node_def.attr()) {
      const char* key = pair.first.c_str();
      const auto& attr = pair.second;
      switch (attr.value_case()) {
        case ::tensorflow::AttrValue::kS:
          fbb->String(key, attr.s());
          has_valid_attr = true;
          break;
        case ::tensorflow::AttrValue::kI:
          fbb->Int(key, attr.i());
          has_valid_attr = true;
          break;
        case ::tensorflow::AttrValue::kF:
          fbb->Float(key, attr.f());
          has_valid_attr = true;
          break;
        case ::tensorflow::AttrValue::kB:
          fbb->Bool(key, attr.b());
          has_valid_attr = true;
          break;
        case tensorflow::AttrValue::kList:
          if (attr.list().i_size() > 0) {
            auto start = fbb->StartVector(key);
            for (const int64_t v : attr.list().i()) {
              fbb->Add(v);
            }
            fbb->EndVector(start, /*typed=*/true, /*fixed=*/false);
            has_valid_attr = true;
          } else {
            LOG(WARNING)
                << "Ignoring unsupported type in list attribute with key '"
                << key << "'";
          }
          break;
        default:
          LOG(WARNING) << "Ignoring unsupported attribute type with key '"
                       << key << "'";
          break;
      }
    }
    if (!has_valid_attr) {
      return std::unique_ptr<flexbuffers::Builder>();
    }
    fbb->EndMap(map_start);
    fbb->Finish();
    return std::unique_ptr<flexbuffers::Builder>(fbb.release());
  }

  void ReadOptions(const flexbuffers::Map& m,
                   TensorFlowUnsupportedOperator* op) const {
    ::tensorflow::NodeDef node_def;
    auto attr = node_def.mutable_attr();

    const auto& keys = m.Keys();
    for (size_t i = 0; i < keys.size(); ++i) {
      const auto key = keys[i].AsKey();
      const auto& value = m[key];
      switch (value.GetType()) {
        case flexbuffers::TYPE_STRING:
          (*attr)[key].set_s(value.AsString().c_str());
          break;
        case flexbuffers::TYPE_INT:
          (*attr)[key].set_i(value.AsInt64());
          break;
        case flexbuffers::TYPE_FLOAT:
          (*attr)[key].set_f(value.AsFloat());
          break;
        case flexbuffers::TYPE_BOOL:
          (*attr)[key].set_b(value.AsBool());
          if (string(key) == "_output_quantized") {
            op->quantized = value.AsBool();
          }
          if (string(key) == "_support_output_type_float_in_quantized_op") {
            op->support_output_type_float_in_quantized_op = value.AsBool();
          }
          break;
        case flexbuffers::TYPE_VECTOR_INT: {
          auto* list = (*attr)[key].mutable_list();
          const auto& vector = value.AsTypedVector();
          for (size_t i = 0; i < vector.size(); i++) {
            list->add_i(vector[i].AsInt64());
          }
          break;
        }
        default:
          LOG(WARNING) << "Ignoring unsupported attribute type with key '"
                       << key << "'";
          break;
      }
    }
    node_def.SerializeToString(&op->tensorflow_node_def);
  }

  int GetVersion(const Operator& op) const override {
    // TODO(ycling): Deisng and implement a way to plumb the version of
    // custom ops.
    return 1;
  }
};

namespace {
// Build a vector containing all the known operators.
std::vector<std::unique_ptr<BaseOperator>> BuildOperatorList() {
  std::vector<std::unique_ptr<BaseOperator>> ops;

  // Builtin Operators.
  ops.emplace_back(new Add(::tflite::BuiltinOperator_ADD, OperatorType::kAdd));
  ops.emplace_back(new Div(::tflite::BuiltinOperator_DIV, OperatorType::kDiv));
  ops.emplace_back(new Sub(::tflite::BuiltinOperator_SUB, OperatorType::kSub));
  ops.emplace_back(new AveragePool(::tflite::BuiltinOperator_AVERAGE_POOL_2D,
                                   OperatorType::kAveragePool));
  ops.emplace_back(
      new SpaceToBatchND(::tflite::BuiltinOperator_SPACE_TO_BATCH_ND,
                         OperatorType::kSpaceToBatchND));
  ops.emplace_back(
      new BatchToSpaceND(::tflite::BuiltinOperator_BATCH_TO_SPACE_ND,
                         OperatorType::kBatchToSpaceND));
  ops.emplace_back(new Concatenation(::tflite::BuiltinOperator_CONCATENATION,
                                     OperatorType::kConcatenation));
  ops.emplace_back(
      new Convolution(::tflite::BuiltinOperator_CONV_2D, OperatorType::kConv));
  ops.emplace_back(
      new DepthwiseConvolution(::tflite::BuiltinOperator_DEPTHWISE_CONV_2D,
                               OperatorType::kDepthwiseConv));
  ops.emplace_back(new FullyConnected(::tflite::BuiltinOperator_FULLY_CONNECTED,
                                      OperatorType::kFullyConnected));
  ops.emplace_back(
      new Gather(::tflite::BuiltinOperator_GATHER, OperatorType::kGather));
  ops.emplace_back(
      new L2Normalization(::tflite::BuiltinOperator_L2_NORMALIZATION,
                          OperatorType::kL2Normalization));
  ops.emplace_back(
      new L2Pool(::tflite::BuiltinOperator_L2_POOL_2D, OperatorType::kL2Pool));
  ops.emplace_back(new LocalResponseNormalization(
      ::tflite::BuiltinOperator_LOCAL_RESPONSE_NORMALIZATION,
      OperatorType::kLocalResponseNormalization));
  ops.emplace_back(new MaxPool(::tflite::BuiltinOperator_MAX_POOL_2D,
                               OperatorType::kMaxPool));
  ops.emplace_back(new Mul(::tflite::BuiltinOperator_MUL, OperatorType::kMul));
  ops.emplace_back(new Pad(::tflite::BuiltinOperator_PAD, OperatorType::kPad));
  ops.emplace_back(
      new PadV2(::tflite::BuiltinOperator_PADV2, OperatorType::kPadV2));
  ops.emplace_back(
      new Reshape(::tflite::BuiltinOperator_RESHAPE, OperatorType::kReshape));
  ops.emplace_back(
      new Softmax(::tflite::BuiltinOperator_SOFTMAX, OperatorType::kSoftmax));
  ops.emplace_back(new SpaceToDepth(::tflite::BuiltinOperator_SPACE_TO_DEPTH,
                                    OperatorType::kSpaceToDepth));
  ops.emplace_back(
      new Svdf(::tflite::BuiltinOperator_SVDF, OperatorType::kSvdf));
  ops.emplace_back(new Transpose(::tflite::BuiltinOperator_TRANSPOSE,
                                 OperatorType::kTranspose));
  ops.emplace_back(
      new Mean(::tflite::BuiltinOperator_MEAN, OperatorType::kMean));
  ops.emplace_back(new Sum(::tflite::BuiltinOperator_SUM, OperatorType::kSum));
  ops.emplace_back(new ReduceProd(::tflite::BuiltinOperator_REDUCE_PROD,
                                  OperatorType::kReduceProd));
  ops.emplace_back(new ReduceMax(::tflite::BuiltinOperator_REDUCE_MAX,
                                 OperatorType::kReduceMax));
  ops.emplace_back(new ResizeBilinear(::tflite::BuiltinOperator_RESIZE_BILINEAR,
                                      OperatorType::kResizeBilinear));
  ops.emplace_back(
      new Squeeze(::tflite::BuiltinOperator_SQUEEZE, OperatorType::kSqueeze));
  ops.emplace_back(
      new Split(::tflite::BuiltinOperator_SPLIT, OperatorType::kSplit));
  ops.emplace_back(new StridedSlice(::tflite::BuiltinOperator_STRIDED_SLICE,
                                    OperatorType::kStridedSlice));
  ops.emplace_back(
      new TopK_V2(::tflite::BuiltinOperator_TOPK_V2, OperatorType::kTopK_V2));
  ops.emplace_back(
      new Lstm(::tflite::BuiltinOperator_LSTM, OperatorType::kLstmCell));
  ops.emplace_back(
      new Cast(::tflite::BuiltinOperator_CAST, OperatorType::kCast));
  ops.emplace_back(
      new ArgMax(::tflite::BuiltinOperator_ARG_MAX, OperatorType::kArgMax));
  ops.emplace_back(
      new ArgMin(::tflite::BuiltinOperator_ARG_MIN, OperatorType::kArgMin));
  ops.emplace_back(
      new Tile(::tflite::BuiltinOperator_TILE, OperatorType::kTile));
  ops.emplace_back(new ExpandDims(::tflite::BuiltinOperator_EXPAND_DIMS,
                                  OperatorType::kExpandDims));
  ops.emplace_back(new TransposeConv(::tflite::BuiltinOperator_TRANSPOSE_CONV,
                                     OperatorType::kTransposeConv));
  ops.emplace_back(new SparseToDense(::tflite::BuiltinOperator_SPARSE_TO_DENSE,
                                     OperatorType::kSparseToDense));
  ops.emplace_back(
      new Shape(::tflite::BuiltinOperator_SHAPE, OperatorType::kShape));
  ops.emplace_back(new FakeQuant(::tflite::BuiltinOperator_FAKE_QUANT,
                                 OperatorType::kFakeQuant));
  ops.emplace_back(
      new Pack(::tflite::BuiltinOperator_PACK, OperatorType::kPack));
  ops.emplace_back(
      new OneHot(::tflite::BuiltinOperator_ONE_HOT, OperatorType::kOneHot));

  // Custom Operators.
  ops.emplace_back(
      new DepthToSpace("DEPTH_TO_SPACE", OperatorType::kDepthToSpace));
  ops.emplace_back(new CTCBeamSearchDecoder(
      "CTC_BEAM_SEARCH_DECODER", OperatorType::kCTCBeamSearchDecoder));
  ops.emplace_back(new TensorFlowUnsupported("TENSORFLOW_UNSUPPORTED",
                                             OperatorType::kUnsupported));

  // There operators are supported by Toco, but not by TF Lite, and has no
  // attributes.
  ops.emplace_back(
      new SimpleOperator<AddNOperator>("ADDN", OperatorType::kAddN));
  // Simple Operators.
  ops.emplace_back(new SimpleOperator<DequantizeOperator>(
      "DEQUANTIZE", OperatorType::kDequantize));
  ops.emplace_back(
      new SimpleOperator<FloorOperator>("FLOOR", OperatorType::kFloor));
  ops.emplace_back(
      new SimpleOperator<ReluOperator>("RELU", OperatorType::kRelu));
  ops.emplace_back(
      new SimpleOperator<Relu1Operator>("RELU_N1_TO_1", OperatorType::kRelu1));
  ops.emplace_back(
      new SimpleOperator<Relu6Operator>("RELU6", OperatorType::kRelu6));
  ops.emplace_back(
      new SimpleOperator<PReluOperator>("PRELU", OperatorType::kPRelu));
  ops.emplace_back(new SimpleOperator<LogisticOperator>(
      "LOGISTIC", OperatorType::kLogistic));
  ops.emplace_back(
      new SimpleOperator<TanhOperator>("TANH", OperatorType::kTanh));
  ops.emplace_back(new SimpleOperator<ExpOperator>("EXP", OperatorType::kExp));
  ops.emplace_back(new SimpleOperator<LogSoftmaxOperator>(
      "LOG_SOFTMAX", OperatorType::kLogSoftmax));
  ops.emplace_back(new SimpleOperator<TensorFlowMaximumOperator>(
      "MAXIMUM", OperatorType::kMaximum));  //  Element-wise Maximum
  ops.emplace_back(new SimpleOperator<TensorFlowMinimumOperator>(
      "MINIMUM", OperatorType::kMinimum));  //  Element-wise Minimum
  ops.emplace_back(new SimpleOperator<TensorFlowGreaterOperator>(
      "GREATER", OperatorType::kGreater));
  ops.emplace_back(new SimpleOperator<TensorFlowGreaterEqualOperator>(
      "GREATER_EQUAL", OperatorType::kGreaterEqual));
  ops.emplace_back(
      new SimpleOperator<TensorFlowLessOperator>("LESS", OperatorType::kLess));
  ops.emplace_back(new SimpleOperator<TensorFlowLessEqualOperator>(
      "LESS_EQUAL", OperatorType::kLessEqual));
  ops.emplace_back(new SimpleOperator<TensorFlowEqualOperator>(
      "EQUAL", OperatorType::kEqual));
  ops.emplace_back(new SimpleOperator<TensorFlowNotEqualOperator>(
      "NOT_EQUAL", OperatorType::kNotEqual));
  ops.emplace_back(new SimpleOperator<NegOperator>("NEG", OperatorType::kNeg));
  ops.emplace_back(
      new SimpleOperator<SelectOperator>("SELECT", OperatorType::kSelect));
  ops.emplace_back(
      new SimpleOperator<SliceOperator>("SLICE", OperatorType::kSlice));
  ops.emplace_back(new SimpleOperator<PowOperator>("POW", OperatorType::kPow));
  ops.emplace_back(new SimpleOperator<LogicalOrOperator>(
      "LOGICAL_OR", OperatorType::kLogicalOr));
  ops.emplace_back(new SimpleOperator<LogicalAndOperator>(
      "LOGICAL_AND", OperatorType::kLogicalAnd));
  ops.emplace_back(new SimpleOperator<LogicalNotOperator>(
      "LOGICAL_NOT", OperatorType::kLogicalNot));
  // Element-wise operator
  ops.emplace_back(new SimpleOperator<SinOperator>("SIN", OperatorType::kSin));
  ops.emplace_back(new SimpleOperator<LogOperator>("LOG", OperatorType::kLog));
  ops.emplace_back(
      new SimpleOperator<TensorFlowSqrtOperator>("SQRT", OperatorType::kSqrt));
  ops.emplace_back(new SimpleOperator<TensorFlowRsqrtOperator>(
      "RSQRT", OperatorType::kRsqrt));

  return ops;
}
}  // namespace

std::map<OperatorType, std::unique_ptr<BaseOperator>> BuildOperatorByTypeMap() {
  std::map<OperatorType, std::unique_ptr<BaseOperator>> result;

  std::vector<std::unique_ptr<BaseOperator>> ops = BuildOperatorList();
  for (auto& op : ops) {
    result[op->type()] = std::move(op);
  }

  return result;
}

std::map<string, std::unique_ptr<BaseOperator>> BuildOperatorByNameMap() {
  std::map<string, std::unique_ptr<BaseOperator>> result;

  std::vector<std::unique_ptr<BaseOperator>> ops = BuildOperatorList();
  for (auto& op : ops) {
    result[op->name()] = std::move(op);
  }

  return result;
}

}  // namespace tflite

}  // namespace toco
