#pragma once

#include <cstdint>
#include <string>

#include "TFLiteGrpc.grpc.pb.h"

TFLiteGrpc::RetStatus MakeFileId(std::int32_t id);
TFLiteGrpc::FileContent MakeFileContent(std::int32_t id, std::string name, const void* data, size_t data_len);
