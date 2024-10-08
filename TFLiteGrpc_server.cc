#include <iostream>
#include <sstream>
#include <memory>
#include <string>
#include <map>
#include <cstdint>
#include <stdexcept>

#include "grpcpp/grpcpp.h"
#include <grpc++/server.h>
#include <grpc++/server_builder.h>
#include <grpc++/server_context.h>
#include <grpc++/security/server_credentials.h>

#include "sequential_file_writer.h"
#include "file_reader_into_stream.h"

using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::ServerReader;
using grpc::ServerReaderWriter;
using grpc::ServerWriter;
using grpc::Status;
using grpc::StatusCode;

using TFLiteGrpc::RetStatus;
using TFLiteGrpc::FileContent;
using TFLiteGrpc::EdgeAI;

class EdgeAIServerImpl final : public EdgeAI::Service {
private:
    typedef google::protobuf::int32 FileIdKey;

public:
    EdgeAIServerImpl() = default;

    Status UploadFile( ServerContext* context, ServerReader<FileContent>* reader, RetStatus* summary ) override
    {
        FileContent contentPart;
        SequentialFileWriter writer;

        while (reader->Read(&contentPart)) {
            try {
                std::cout << "Receive " << contentPart.name() << std::endl;
                std::string savedfile = "saved_" + contentPart.name();
                writer.OpenIfNecessary(savedfile);
                auto* const data = contentPart.mutable_content();
                writer.Write(*data);
                summary->set_id(contentPart.id());
                m_FileIdToName[contentPart.id()] = contentPart.name();
            }
            catch (const std::system_error& ex) {
                const auto status_code = writer.NoSpaceLeft() ? StatusCode::RESOURCE_EXHAUSTED : StatusCode::ABORTED;
                return Status(status_code, ex.what());
            }
        }

        return Status::OK;
    }

    Status CreateRunRequest(::grpc::ServerContext* context, const ::TFLiteGrpc::InputTensor* request,
                            ::TFLiteGrpc::OutputTensor* response) override 
    {
        if (request == nullptr || response == nullptr) {
            std::cout << "The vector pointer is null." << std::endl;
            return Status::OK;
        }

        for (size_t i = 0; i < request->tensor_size(); ++i) {
            std::cout << "Element " << i << ": " << request->tensor(i) << std::endl;
            response->add_tensor(3.14);
        }
        return Status::OK;
    }

private:
    std::map<FileIdKey, std::string> m_FileIdToName;
};


void RunServer(std::string server_address) {
//   std::string server_address("0.0.0.0:50051");
  EdgeAIServerImpl service;

  ServerBuilder builder;
  builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
  builder.RegisterService(&service);
  std::unique_ptr<Server> server(builder.BuildAndStart());
    std::cout << "Server listening on " << server_address << ". Press Ctrl-C to end." << std::endl;
  server->Wait();
}

int main(int argc, char** argv)
{
    std::string server_address("0.0.0.0:50051");
    if(argc==2)
        server_address = std::string(argv[1]);

    RunServer(server_address);
    return 0;
}
