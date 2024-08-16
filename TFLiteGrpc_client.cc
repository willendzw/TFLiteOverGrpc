#include <iostream>
#include <memory>
#include <string>
#include <vector>
#include <string>
#include <cstdlib>
#include <cstdint>
#include <utility>
#include <cassert>
#include <sysexits.h>
#include <chrono>

#include <grpc/grpc.h>
#include <grpc++/channel.h>
#include <grpc++/client_context.h>
#include <grpc++/create_channel.h>
#include <grpc++/security/credentials.h>

#include "utils.h"
#include "sequential_file_writer.h"
#include "file_reader_into_stream.h"

using grpc::Channel;
using grpc::ClientContext;
using grpc::ClientReader;
using grpc::ClientReaderWriter;
using grpc::ClientWriter;
using grpc::Status;

using TFLiteGrpc::RetStatus;
using TFLiteGrpc::FileContent;
using TFLiteGrpc::EdgeAI;
using TFLiteGrpc::InputTensor;
using TFLiteGrpc::OutputTensor;


class TFClient {
public:
    TFClient(std::shared_ptr<Channel> channel): m_stub(EdgeAI::NewStub(channel)){ }

    bool UploadFile(const std::string& filename)
    {
        RetStatus returnedId;
        ClientContext context;
        std::int32_t id = 1234;
        std::unique_ptr<ClientWriter<FileContent>> writer(m_stub->UploadFile(&context, &returnedId));
        try {
            FileReaderIntoStream< ClientWriter<FileContent> > reader(filename, id, *writer);

            const size_t chunk_size = 1UL << 20;    // Hardcoded to 1MB, which seems to be recommended from experience.
            reader.Read(chunk_size);
        }
        catch (const std::exception& ex) {
            std::cerr << "Failed to send the file " << filename << ": " << ex.what() << std::endl;
        }
    
        writer->WritesDone();
        Status status = writer->Finish();
        if (!status.ok()) {
            std::cerr << "File Uploading rpc failed: " << status.error_message() << std::endl;
            return false;
        }
        else {
            std::cout << "Finished sending file with id " << returnedId.id() << std::endl;
        }

        return true;
    }

    bool CreateRequest(std::vector<float>* invec, std::vector<float>* outvec)
    {
        TFLiteGrpc::InputTensor input;
        ClientContext context;
        TFLiteGrpc::OutputTensor  output;

        if (invec == nullptr || outvec == nullptr) {
            std::cout << "The vector pointer is null." << std::endl;
            return false;
        }

        for (size_t i = 0; i < invec->size(); ++i) {
            std::cout << "Element " << i << ": " << (*invec)[i] << std::endl;
            input.add_tensor((*invec)[i]);
        }

        // The actual RPC.
        Status status = m_stub->CreateRunRequest(&context, input, &output);

        for (size_t i = 0; i < output.tensor_size(); ++i) {
            std::cout << "Output " << i << ": " << output.tensor(i) << std::endl;
            (*outvec)[i] = output.tensor(i);
        }

        return true;
    }
private:
    std::unique_ptr<TFLiteGrpc::EdgeAI::Stub> m_stub;
};

void usage (const char* prog_name)
{
    std::cerr << "USAGE: " << prog_name << "[server address] [model filename]" << std::endl;
    std::exit(EX_USAGE);
}

int main(int argc, char** argv)
{
    bool succeeded = false;
    if (3 != argc) {
        usage(argv[0]);
    }

    std::string server_address(argv[1]);
    TFClient client(grpc::CreateChannel(server_address, grpc::InsecureChannelCredentials()));
    const std::string filename = argv[2];
    std::vector<float> image = {1.3,15.1,1.3,15.1,1.3,15.1,1.3,15.1,1.3,15.1};
    std::vector<float> res(10);


    auto timeStart = std::chrono::high_resolution_clock::now();
    succeeded = client.UploadFile(filename);
    auto timeEnd = std::chrono::high_resolution_clock::now();

    long long duration = std::chrono::duration_cast<std::chrono::microseconds>(timeEnd - timeStart).count();
    std::cout<<"Latency: "<<duration<<"ms\n";

    timeStart = std::chrono::high_resolution_clock::now();
    succeeded = client.CreateRequest(&image, &res);
    timeEnd = std::chrono::high_resolution_clock::now();

    duration = std::chrono::duration_cast<std::chrono::microseconds>(timeEnd - timeStart).count();
    std::cout<<"Latency: "<<duration<<"ms\n";


    return succeeded ? EX_OK : EX_IOERR;
}
