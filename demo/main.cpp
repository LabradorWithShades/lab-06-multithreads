// Copyright 2020 LabradorWithShades
// 70892548+LabradorWithShades@users.noreply.github.com

#include <csignal>
#include <hash_argument_finder.hpp>

static HashArgumentFinder finder;

void my_handler(int /*unused*/) {
  finder.stopSearch();
  exit(0);
}

void registerSigIntHandler() {
  struct sigaction sigIntHandler {};
  sigIntHandler.sa_handler = my_handler;
  sigemptyset(&sigIntHandler.sa_mask);
  sigIntHandler.sa_flags = 0;
  sigaction(SIGINT, &sigIntHandler, nullptr);
}

size_t extractThreadCount(char* arg) {
  return std::stoul(arg);
}

int main(int argc, [[maybe_unused]] char* argv[]) {
  registerSigIntHandler();

  if (argc == 1) {
    finder.startSearch("result.json");
  } else if (argc == 2) {
    finder.startSearch(argv[1]);
  } else if (argc == 3) {
    finder.startSearch(argv[1], extractThreadCount(argv[2]));
  }

  while (true) {
    //Infinite loop until SIGINT is caught
  }
}
