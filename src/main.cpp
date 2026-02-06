#include "includes/clustro.hpp"

int main() {
    // Enable virtual terminal processing for colored output on Windows
    clustro::enableVirtualTerminalProcessing();
    
    Clustro app;
    app.run();
    
    return 0;
}