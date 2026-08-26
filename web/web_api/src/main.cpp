#include "web_api/app.h"
#include <iostream>
#include <exception>
#include <locale>
#include <QCoreApplication>

int main(int argc, char* argv[]) {
    // Устанавливаем C-локаль для корректной сериализации чисел в JSON
    // (точка вместо запятой в качестве десятичного разделителя)
    std::setlocale(LC_NUMERIC, "C");
    std::locale::global(std::locale("C"));

    try {
        QCoreApplication qtApp(argc, argv);
        
        std::cout << "Starting FridgeFusion API Server..." << std::endl;
        
        web_api::FridgeFusionApp app;
        
        app.initialize();
        
        app.setupRoutes();
        
        app.run();
        
        return 0;
        
    } catch (const std::exception& e) {
        std::cerr << "\n========================================" << std::endl;
        std::cerr << "FATAL ERROR: " << e.what() << std::endl;
        std::cerr << "========================================\n" << std::endl;
        return 1;
    } catch (...) {
        std::cerr << "\n========================================" << std::endl;
        std::cerr << "FATAL ERROR: Unknown exception occurred" << std::endl;
        std::cerr << "========================================\n" << std::endl;
        return 1;
    }
}