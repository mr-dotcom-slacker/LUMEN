#include <opencv2/opencv.hpp>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <vector>
#include <string>

// ==========================================
// CONFIGURATION CONSTANTS
// ==========================================
// Real-world width of the target object in centimeters (e.g., a tennis ball ~ 6.7 cm)
double knownObjectWidthCm = 6.7;

// Default initial focal length in pixels (will be adjusted via calibration)
// Rough estimate for typical 720p webcams: ~600 - 800 px
double focalLengthPx = 700.0;

// Reference distance for one-click calibration in centimeters
double calibrationDistanceCm = 50.0;

// Minimum contour area to ignore small noise particles
const double MIN_CONTOUR_AREA = 1000.0;

// Default HSV range (configured for a bright green/yellow object like a tennis ball)
// Adjust these ranges to match your specific object
const cv::Scalar LOWER_HSV(29, 86, 6);
const cv::Scalar UPPER_HSV(64, 255, 255);

// ==========================================
// HELPER FUNCTIONS
// ==========================================

/**
 * Preprocesses frame and extracts the largest matching contour.
 * Returns true if a valid object is found.
 */
bool detectObject(const cv::Mat& frame, cv::Rect& outBoundingBox) {
    cv::Mat hsv, mask, blurred;

    // 1. Reduce high-frequency noise
    cv::GaussianBlur(frame, blurred, cv::Size(9, 9), 2);

    // 2. Convert from BGR to HSV color space
    cv::cvtColor(blurred, hsv, cv::COLOR_BGR2HSV);

    // 3. Threshold by color
    cv::inRange(hsv, LOWER_HSV, UPPER_HSV, mask);

    // 4. Morphological opening and closing to remove speckles and fill holes
    cv::Mat kernel = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(5, 5));
    cv::morphologyEx(mask, mask, cv::MORPH_OPEN, kernel);
    cv::morphologyEx(mask, mask, cv::MORPH_CLOSE, kernel);

    // 5. Find contours
    std::vector<std::vector<cv::Point>> contours;
    cv::findContours(mask, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);

    if (contours.empty()) {
        return false;
    }

    // 6. Find the contour with the largest area
    double maxArea = 0.0;
    int maxIndex = -1;
    for (size_t i = 0; i < contours.size(); ++i) {
        double area = cv::contourArea(contours[i]);
        if (area > maxArea && area >= MIN_CONTOUR_AREA) {
            maxArea = area;
            maxIndex = static_cast<int>(i);
        }
    }

    if (maxIndex != -1) {
        outBoundingBox = cv::boundingRect(contours[maxIndex]);
        return true;
    }

    return false;
}

/**
 * Calculates distance in centimeters using triangle similarity.
 */
double calculateDistance(double knownWidthCm, double focalLengthPx, int pixelWidth) {
    if (knownWidthCm <= 0.0 || focalLengthPx <= 0.0 || pixelWidth <= 0) return -1.0;
    return (knownWidthCm * focalLengthPx) / static_cast<double>(pixelWidth);
}

bool calibrateFocalLength(int pixelWidth) {
    if (pixelWidth <= 0 || knownObjectWidthCm <= 0.0 || calibrationDistanceCm <= 0.0) {
        return false;
    }

    focalLengthPx = (pixelWidth * calibrationDistanceCm) / knownObjectWidthCm;
    return true;
}

// ==========================================
// MAIN FUNCTION
// ==========================================
int main(int argc, char** argv) {
    // 1. Initialize Video Capture
    int cameraID = 0;
    if (argc > 1) {
        cameraID = std::stoi(argv[1]);
    }
    if (argc > 2) {
        knownObjectWidthCm = std::stod(argv[2]);
    }
    if (argc > 3) {
        calibrationDistanceCm = std::stod(argv[3]);
    }
    if (knownObjectWidthCm <= 0.0 || calibrationDistanceCm <= 0.0) {
        std::cerr << "[ERROR] Object width and calibration distance must be positive.\n";
        return -1;
    }

    std::cout << "[INFO] Opening camera ID: " << cameraID << "..." << std::endl;
    cv::VideoCapture cap(cameraID);

    // Error Handling: Camera failed to open
    if (!cap.isOpened()) {
        std::cerr << "[ERROR] Could not open camera. Check permissions or device index." << std::endl;
        return -1;
    }

    // Set resolution (optional, 640x480 or 1280x720)
    cap.set(cv::CAP_PROP_FRAME_WIDTH, 640);
    cap.set(cv::CAP_PROP_FRAME_HEIGHT, 480);

    std::cout << "====================================================\n"
              << " Controls:\n"
              << "  [C] : Calibrate camera at " << calibrationDistanceCm << " cm\n"
              << "  [ESC] or [Q] : Exit program\n"
              << "====================================================" << std::endl;

    cv::Mat frame;
    cv::TickMeter tm;

    while (true) {
        tm.start();

        // 2. Read frame
        bool success = cap.read(frame);
        if (!success || frame.empty()) {
            std::cerr << "[WARNING] Blank or dropped frame captured." << std::endl;
            break;
        }

        cv::Rect detectedBox;
        bool objectDetected = detectObject(frame, detectedBox);

        // 3. Distance Estimation and HUD Overlay
        if (objectDetected) {
            // Calculate distance
            double distance = calculateDistance(knownObjectWidthCm, focalLengthPx, detectedBox.width);

            // Draw bounding box
            cv::rectangle(frame, detectedBox, cv::Scalar(0, 255, 0), 2);

            // Draw center point
            cv::Point center(detectedBox.x + detectedBox.width / 2, detectedBox.y + detectedBox.height / 2);
            cv::circle(frame, center, 4, cv::Scalar(0, 0, 255), -1);

            // Prepare text annotations
            std::ostringstream distStream, infoStream;
            distStream << std::fixed << std::setprecision(1) << distance << " cm";
            infoStream << "Width: " << detectedBox.width << " px";

            // Draw distance text above bounding box
            int baseline = 0;
            cv::Size textSize = cv::getTextSize(distStream.str(), cv::FONT_HERSHEY_SIMPLEX, 0.7, 2, &baseline);
            cv::Point textOrigin(detectedBox.x, std::max(25, detectedBox.y - 10));

            // Background rectangle for text contrast
            cv::rectangle(frame, 
                          cv::Point(textOrigin.x - 2, textOrigin.y - textSize.height - 2),
                          cv::Point(textOrigin.x + textSize.width + 2, textOrigin.y + baseline),
                          cv::Scalar(0, 0, 0), cv::FILLED);

            cv::putText(frame, distStream.str(), textOrigin,
                        cv::FONT_HERSHEY_SIMPLEX, 0.7, cv::Scalar(0, 255, 0), 2);

            cv::putText(frame, infoStream.str(), cv::Point(detectedBox.x, detectedBox.y + detectedBox.height + 20),
                        cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(255, 255, 255), 1);
        } else {
            // Overlay warning if no target is found
            cv::putText(frame, "Status: Searching for object...", cv::Point(20, 40),
                        cv::FONT_HERSHEY_SIMPLEX, 0.7, cv::Scalar(0, 0, 255), 2);
        }

        // FPS and Calibration info overlay
        tm.stop();
        double fps = tm.getFPS();
        tm.reset();

        std::string statusText = "FPS: " + std::to_string(static_cast<int>(fps)) + 
                                 " | Focal Length: " + std::to_string(static_cast<int>(focalLengthPx)) + " px";
        cv::putText(frame, statusText, cv::Point(15, frame.rows - 15),
                    cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(200, 200, 200), 1);

        // 4. Display Frame
        cv::imshow("Real-Time Distance Estimator", frame);

        // 5. Handle Keyboard Controls
        char key = static_cast<char>(cv::waitKey(1));
        if (key == 27 || key == 'q' || key == 'Q') { // ESC or Q
            std::cout << "[INFO] User initiated termination." << std::endl;
            break;
        } else if (key == 'c' || key == 'C') { // Interactive Calibration
            if (objectDetected) {
                if (calibrateFocalLength(detectedBox.width)) {
                    std::cout << "[CALIBRATION] Calibrated successfully! New Focal Length: "
                              << focalLengthPx << " px at " << calibrationDistanceCm << " cm distance." << std::endl;
                } else {
                    std::cout << "[CALIBRATION FAILED] Calibration values must be positive." << std::endl;
                }
            } else {
                std::cout << "[CALIBRATION FAILED] Cannot calibrate: No object in sight." << std::endl;
            }
        }
    }

    cap.release();
    cv::destroyAllWindows();
    return 0;
}