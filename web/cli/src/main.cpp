#include "cli/cli_app.h"
#include "cli/config_manager.h"

#include <iostream>

int main(int argc, char* argv[]) {
  // Обработка --verbose
  bool verbose = false;
  for (int i = 1; i < argc; i++) {
    if (std::string(argv[i]) == "--verbose") {
      verbose = true;
    }
  }

  try {
    auto config = std::make_shared<cli::ConfigManager>();
    config->load();

    if (verbose) {
      config->setVerbose(true);
      config->save();
    }

    cli::CLIApp app(config);
    app.run();

  } catch (const std::exception& e) {
    std::cerr << "Ошибка: " << e.what() << std::endl;
    return 1;
  }

  return 0;
}
