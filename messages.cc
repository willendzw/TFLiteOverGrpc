#include "messages.h"

TFLiteGrpc::RetStatus MakeFileId(std::int32_t id)
{
    TFLiteGrpc::RetStatus fid;
    fid.set_id(id);
    return fid;
}

TFLiteGrpc::FileContent MakeFileContent(std::int32_t id, std::string name, const void* data, size_t data_len)
{
    TFLiteGrpc::FileContent fc;
    fc.set_id(id);
    fc.set_name(std::move(name));
    fc.set_content(data, data_len);
    return fc;
}