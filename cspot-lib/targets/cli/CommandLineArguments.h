#ifndef COMMANDLINEARGUMENTS_H
#define COMMANDLINEARGUMENTS_H
#include <protobuf/metadata.pb.h>
#include <memory>
#include <string>

class CommandLineArguments {
 public:

  std::string username;

  std::string password;

  std::string credentials;

  bool setBitrate = false;
  AudioFormat bitrate;

  bool shouldShowHelp;

  CommandLineArguments(std::string username, std::string password,
                       std::string credentials, bool shouldShowHelp);

  static std::shared_ptr<CommandLineArguments> parse(int argc, char** argv);
};

#endif
