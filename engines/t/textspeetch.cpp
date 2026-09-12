#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <stdexcept>

// Include eSpeak NG C API headers 
// Depending on distribution/installation, this header is usually at <espeak-ng/speak_lib.h>
#if __has_include(<espeak-ng/speak_lib.h>)
    #include <espeak-ng/speak_lib.h>
#elif __has_include(<espeak/speak_lib.h>)
    #include <espeak/speak_lib.h>
#else
    #error "eSpeak-NG headers not found. Please install libespeak-ng-dev."
#endif

class TextToSpeech {
public:
    TextToSpeech() {
        // AUDIO_OUTPUT_PLAYBACK plays audio directly to the default audio device in real time.
        // buflength: 500ms internal sound buffer.
        // path: NULL uses default system voice data directory.
        int sampleRate = espeak_Initialize(AUDIO_OUTPUT_PLAYBACK, 500, nullptr, 0);
        if (sampleRate < 0) {
            throw std::runtime_error("Failed to initialize eSpeak-NG TTS engine.");
        }
    }

    ~TextToSpeech() {
        // Wait for all queued audio to finish before terminating
        espeak_Synchronize();
        espeak_Terminate();
    }

    // Set voice language/identifier (e.g., "en", "en-us", "en-uk", "es", "fr", "de")
    bool setVoice(const std::string& voiceName) {
        espeak_ERROR err = espeak_SetVoiceByName(voiceName.c_str());
        if (err != EE_OK) {
            std::cerr << "[Warning] Voice '" << voiceName << "' not found. Falling back to default.\n";
            return false;
        }
        return true;
    }

    // Rate is in words per minute (WPM). Default is ~175, range is typically 80 to 450.
    void setRate(int wordsPerMinute) {
        if (wordsPerMinute < 80)  wordsPerMinute = 80;
        if (wordsPerMinute > 450) wordsPerMinute = 450;
        espeak_SetParameter(espeakRATE, wordsPerMinute, 0);
    }

    // Pitch: 0 to 100. Default is 50.
    void setPitch(int pitch) {
        if (pitch < 0)   pitch = 0;
        if (pitch > 100) pitch = 100;
        espeak_SetParameter(espeakPITCH, pitch, 0);
    }

    // Speak a text string in real time
    bool speak(const std::string& text) {
        if (text.empty()) {
            std::cerr << "[Warning] Attempted to speak empty text.\n";
            return false;
        }

        // POS_CHARACTER = 1
        // flags: espeakCHARS_AUTO automatically detects ASCII/UTF-8.
        espeak_ERROR err = espeak_Synth(
            text.c_str(),
            text.length() + 1,
            0,
            POS_CHARACTER,
            0,
            espeakCHARS_AUTO,
            nullptr,
            nullptr
        );

        if (err != EE_OK) {
            std::cerr << "[Error] Failed to synthesize audio.\n";
            return false;
        }

        // Wait until speech playback completes
        espeak_Synchronize();
        return true;
    }
};

// Helper function to read entire text file
std::string readFileContents(const std::string& filePath) {
    std::ifstream file(filePath);
    if (!file.is_open()) {
        throw std::runtime_error("Could not open file: " + filePath);
    }
    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

void printHelp() {
    std::cout << "\n--- Text-To-Speech Menu ---\n"
              << "Commands:\n"
              << "  :file <path>   - Speak contents of a text file\n"
              << "  :voice <name>  - Change voice (e.g., en, en-us, fr, es, de)\n"
              << "  :speed <wpm>   - Set speed in WPM (80 - 450, default: 175)\n"
              << "  :pitch <val>   - Set pitch (0 - 100, default: 50)\n"
              << "  :exit          - Exit the program\n"
              << "  <any text>     - Speak the typed text immediately\n"
              << "---------------------------\n\n";
}

int main(int argc, char* argv[]) {
    try {
        TextToSpeech tts;

        // Default configuration
        tts.setVoice("en");
        tts.setRate(175);
        tts.setPitch(50);

        // Command-line argument handling: check if a file was passed as an argument
        if (argc > 1) {
            std::string filePath = argv[1];
            std::cout << "[Info] Reading text from file: " << filePath << std::endl;
            std::string fileContent = readFileContents(filePath);
            tts.speak(fileContent);
            return 0;
        }

        printHelp();

        std::string input;
        while (true) {
            std::cout << "TTS > ";
            if (!std::getline(std::cin, input)) {
                break; // Handle EOF (Ctrl+D / Ctrl+Z)
            }

            if (input.empty()) {
                continue;
            }

            // Command handling
            if (input == ":exit" || input == ":quit") {
                std::cout << "Exiting TTS. Goodbye!\n";
                break;
            } else if (input.rfind(":voice ", 0) == 0) {
                std::string voice = input.substr(7);
                tts.setVoice(voice);
                std::cout << "[Config] Voice set to: " << voice << "\n";
            } else if (input.rfind(":speed ", 0) == 0) {
                try {
                    int speed = std::stoi(input.substr(7));
                    tts.setRate(speed);
                    std::cout << "[Config] Speed set to: " << speed << " WPM\n";
                } catch (const std::exception&) {
                    std::cerr << "[Error] Invalid speed value.\n";
                }
            } else if (input.rfind(":pitch ", 0) == 0) {
                try {
                    int pitch = std::stoi(input.substr(7));
                    tts.setPitch(pitch);
                    std::cout << "[Config] Pitch set to: " << pitch << "\n";
                } catch (const std::exception&) {
                    std::cerr << "[Error] Invalid pitch value.\n";
                }
            } else if (input.rfind(":file ", 0) == 0) {
                std::string path = input.substr(6);
                try {
                    std::string content = readFileContents(path);
                    std::cout << "[Info] Speaking file contents...\n";
                    tts.speak(content);
                } catch (const std::exception& e) {
                    std::cerr << "[Error] " << e.what() << "\n";
                }
            } else {
                // Regular text input -> speak
                tts.speak(input);
            }
        }
    } catch (const std::exception& ex) {
        std::cerr << "[Fatal Error] " << ex.what() << std::endl;
        return 1;
    }

    return 0;
}
