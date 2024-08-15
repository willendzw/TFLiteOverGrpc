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
    TFClient(std::shared_ptr<Channel> channel)
        : m_stub(EdgeAI::NewStub(channel))
    {
        
    }

    bool UploadFile(std::int32_t id, const std::string& filename)
    {
        RetStatus returnedId;
        ClientContext context;

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
    std::cerr << "USAGE: " << prog_name << "[server address] [put|get|put_all|get_all] [model filename]" << std::endl;
    std::exit(EX_USAGE);
}

int main(int argc, char** argv)
{
    if (argc < 4) {
        usage(argv[0]);
    }

    const std::string verb = argv[2];
    std::int32_t id = -1;
    try {
        id = std::atoi(argv[3]);
    }
    catch (std::invalid_argument) {
        std::cerr << "Invalid Id " << argv[3] << std::endl;
        usage(argv[0]);
    }
    bool succeeded = false;
    std::string server_address(argv[1]);
    TFClient client(grpc::CreateChannel(server_address, grpc::InsecureChannelCredentials()));

    if ("put" == verb) {
        if (5 != argc) {
            usage(argv[0]);
        }
        const std::string filename = argv[4];
        auto timeStart = std::chrono::high_resolution_clock::now();
        succeeded = client.UploadFile(id, filename);
        auto timeEnd = std::chrono::high_resolution_clock::now();

        long long duration = std::chrono::duration_cast<std::chrono::microseconds>(timeEnd - timeStart).count();
        std::cout<<"Latency: "<<duration<<"ms\n";
    }
    else if ("put_all" == verb) {
        if (5 != argc) {
            usage(argv[0]);
        }
        std::int32_t nfiles = atoi(argv[3]);
        const std::string filenamePrefix = argv[4];
        std::string filename;
        for(int i=1; i<=nfiles; i++){
            filename = filenamePrefix + std::to_string(i)+".txt";
            std::cout<<filename<<"\n";
            auto timeStart = std::chrono::high_resolution_clock::now();
            succeeded = client.UploadFile(i, filename);
            auto timeEnd = std::chrono::high_resolution_clock::now();
            
            if(true == succeeded){
                long long duration = std::chrono::duration_cast<std::chrono::microseconds>(timeEnd - timeStart).count();
                std::cout<<"Upload Latency: "<<duration<<"ms\n";
            }
            else
                break;
        }
        
    }
    else {
        std::cerr << "Unknown verb " << verb << std::endl;
        usage(argv[0]);
    }

    return succeeded ? EX_OK : EX_IOERR;
}
