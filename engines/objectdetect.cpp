#include <iostream>
#include <iomanip>
#include <string>
#include <chrono>
#include <thread>
#include <random>

// Enum representing the criticality level of an obstacle
enum class Criticality {
    SAFE,
    CAUTION,
    DANGER
};

// Struct holding configurable distance thresholds (in meters)
struct ThresholdConfig {//so this just example to just test the code and we can modify the distance after we discuss the limites for the range
    double dangerDistance = 2.0;   // Distance < 2.0m  -> Danger
    double cautionDistance = 5.0;  // 2.0m <= dist <= 5.0m -> Caution
                                   // Distance > 5.0m  -> Safe
};

// ANSI Color Codes for terminal output
namespace Color {
    const std::string RED     = "\033[1;31m";
    const std::string YELLOW  = "\033[1;33m";
    const std::string GREEN   = "\033[1;32m";
    const std::string RESET   = "\033[0m";
    const std::string BOLD    = "\033[1m";
}

class ObstacleDetector {
private:
    ThresholdConfig thresholds;

public:
    explicit ObstacleDetector(ThresholdConfig config = ThresholdConfig{})
        : thresholds(config) {}

    // Update thresholds at runtime
    void setThresholds(double dangerDist, double cautionDist) {
        thresholds.dangerDistance = dangerDist;
        thresholds.cautionDistance = cautionDist;
    }

    // Classify criticality based on measured distance
    Criticality evaluate(double distanceMeters) const {
        if (distanceMeters < thresholds.dangerDistance) {
            return Criticality::DANGER;
        } else if (distanceMeters <= thresholds.cautionDistance) {
            return Criticality::CAUTION;
        } else {
            return Criticality::SAFE;
        }
    }

    // Format output with colored status label
    void logReading(double distanceMeters) const {
        Criticality level = evaluate(distanceMeters);

        std::string statusTag;
        std::string alertMsg;

        switch (level) {
            case Criticality::DANGER:
                statusTag = Color::RED + "[ DANGER  ]" + Color::RESET;
                alertMsg  = "Obstacle critically close! Stop/divert.";
                break;
            case Criticality::CAUTION:
                statusTag = Color::YELLOW + "[ CAUTION ]" + Color::RESET;
                alertMsg  = "Obstacle approaching. Reduce speed.";
                break;
            case Criticality::SAFE:
                statusTag = Color::GREEN + "[  SAFE   ]" + Color::RESET;
                alertMsg  = "Path is clear.";
                break;
        }

        std::cout << statusTag 
                  << " Distance: " << std::fixed << std::setprecision(2) 
                  << std::setw(5) << distanceMeters << " m"
                  << " | Action: " << alertMsg 
                  << std::endl;
    }
};

// Mode 1: Simulated real-time sensor stream
void runSimulation(const ObstacleDetector& detector, int readingCount = 20) {
    std::cout << Color::BOLD << "\n--- Starting Real-Time Sensor Simulation ---\n" << Color::RESET;
    std::cout << "Thresholds: Danger < 2.0m | Caution 2.0m - 5.0m | Safe > 5.0m\n\n";

    std::mt19937 rng(std::random_device{}());
    // Simulate obstacle distances from 0.3m up to 8.0m
    std::uniform_real_distribution<double> distGenerator(0.3, 8.0);

    for (int i = 1; i <= readingCount; ++i) {
        double currentDistance = distGenerator(rng);
        std::cout << "#" << std::setw(2) << std::setfill('0') << i << " ";
        detector.logReading(currentDistance);

        // Simulate 500ms sensor sampling interval
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }
}

// Mode 2: Manual user input for interactive testing
void runManualTest(const ObstacleDetector& detector) {
    std::cout << Color::BOLD << "\n--- Manual Distance Input Mode ---\n" << Color::RESET;
    std::cout << "Enter a distance in meters (or type a negative number to exit):\n";

    double distance = 0.0;
    while (true) {
        std::cout << "> ";
        if (!(std::cin >> distance)) {
            std::cin.clear();
            std::cin.ignore(10000, '\n');
            std::cout << "Invalid input. Please enter a numerical value.\n";
            continue;
        }

        if (distance < 0.0) {
            std::cout << "Exiting manual test mode.\n";
            break;
        }

        detector.logReading(distance);
    }
}

int main() {
    // Configurable thresholds: < 2.0m = Danger, <= 5.0m = Caution, > 5.0m = Safe
    ThresholdConfig config{2.0, 5.0};
    ObstacleDetector detector(config);

    std::cout << Color::BOLD << "==========================================\n";
    std::cout << "     OBSTACLE DISTANCE DETECTION SYSTEM   \n";
    std::cout << "==========================================\n" << Color::RESET;
    std::cout << "Select mode:\n";
    std::cout << " 1. Run real-time simulation (sensor loop)\n";
    std::cout << " 2. Manual distance input\n";
    std::cout << "Enter choice (1 or 2): ";

    int choice = 1;
    if (std::cin >> choice) {
        if (choice == 1) {
            runSimulation(detector);
        } else {
            runManualTest(detector);
        }
    }

    return 0;
}