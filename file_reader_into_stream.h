#pragma once

#include <cstdint>
#include <string>
#include "sys/errno.h"

#include "sequential_file_reader.h"
#include "utils.h"
#include "TFLiteGrpc.grpc.pb.h"

template <class StreamWriter>
class FileReaderIntoStream : public SequentialFileReader {
public:
    FileReaderIntoStream(const std::string& filename, std::int32_t id, StreamWriter& writer)
        : SequentialFileReader(filename)
        , m_writer(writer)
        , m_id(id)
    {
    }

    using SequentialFileReader::SequentialFileReader;
    using SequentialFileReader::operator=;

protected:
    virtual void OnChunkAvailable(const void* data, size_t size) override
    {
        const std::string remote_filename = extract_basename(GetFilePath());
        TFLiteGrpc::FileContent fc;
        fc.set_id(m_id);
        fc.set_name(std::move(remote_filename));
        fc.set_content(data, size);

        if (! m_writer.Write(fc)) {
            raise_from_system_error_code("The server aborted the connection.", ECONNRESET);
        }
    }

private:
    StreamWriter& m_writer;
    std::uint32_t m_id;
};
