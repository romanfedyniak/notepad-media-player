#include <iostream>
#include <chrono>
#include "notepadDraw.h"
#include "ASCIIConverter.h"
#include "inputParser.h"


void start(std::string pathToNotepad, std::string pathToVideo, int charsWidth, int charsHeight, int fontSize, bool isStatusBar)
{
    NotepadDraw notepad(
        pathToNotepad,
        charsWidth,
        charsHeight,
        fontSize,
        isStatusBar
    );
    ASCIIConverter converter(pathToVideo, charsWidth, charsHeight);

    notepad.init();
    converter.init();
    
    std::chrono::milliseconds delay(1000 / converter.getFPS());

    for (int i = 0; i < converter.getFrameCount(); ++i)
    {
        auto start = std::chrono::system_clock::now();
        notepad.clearScreen();

        ASCIIConverter::pixel_vector* pvector = converter.getImage();
        for (auto iter = pvector->cbegin(); iter != pvector->cend(); ++iter)
        {
            //printf((std::to_string(iter->x) + ":" + std::to_string(iter->y) + "\n").c_str());
            notepad.drawChar(iter->x, iter->y, iter->ch);
        }
        delete pvector;

        notepad.swapBuffersAndRedraw();

        auto end = std::chrono::system_clock::now();
        std::chrono::milliseconds millis_elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
        std::this_thread::sleep_for(std::chrono::milliseconds(std::max(std::chrono::milliseconds(1), delay - millis_elapsed)));
    }

    notepad.shutdownNotepad();
}

int main(int argc, char *argv[])
{
    const char helpMessage[] = {
        "-n, --notepad       path to notepad\n"
        "-v, --video         path to video file\n"
        "-w, --width         width in chars\n"
        "-h, --height        height in chars\n"
        "-f, --font          font size, minimum - 1, maximum - 20\n"
        "-s, --statusbar     optional argument. Add it if the status bar is enabled in notepad\n"
        "--help              this message"
    };

    InputParser input(argc, argv);
    
    if (argc < 2 || input.cmdOptionExists("--help"))
    {
        std::cout << helpMessage << std::endl;
        return 0;
    }

    std::string pathToNotepad, pathToVideo;
    int width = 120, height = 30, fontSize = 10;
    bool statusBar;

    if (input.cmdOptionExists("-n"))
        pathToNotepad = input.getCmdOption("-n");
    else if (input.cmdOptionExists("--notepad"))
        pathToNotepad = input.getCmdOption("--notepad");
    else
    {
        std::cout << "the path to the notepad is not specified" << std::endl;
        return 0;
    }

    if (input.cmdOptionExists("-v"))
        pathToVideo = input.getCmdOption("-v");
    else if (input.cmdOptionExists("--video"))
        pathToVideo = input.getCmdOption("--video");
    else
    {
        std::cout << "the path to the video is not specified" << std::endl;
        return 0;
    }

    if (input.cmdOptionExists("-w"))
        width = std::stoi(input.getCmdOption("-w"));
    else if (input.cmdOptionExists("--width"))
        width = std::stoi(input.getCmdOption("--width"));
    if (width < 0)
    {
        std::cout << "width should be more than 0" << std::endl;
        return 0;
    }

    if (input.cmdOptionExists("-h"))
        height = std::stoi(input.getCmdOption("-h"));
    else if (input.cmdOptionExists("--height"))
        height = std::stoi(input.getCmdOption("-height"));
    if (height < 0)
    {
        std::cout << "height should be more than 0" << std::endl;
        return 0;
    }

    if (input.cmdOptionExists("-f"))
        fontSize = std::stoi(input.getCmdOption("-f"));
    else if (input.cmdOptionExists("--font"))
        fontSize = std::stoi(input.getCmdOption("--font"));
    if (fontSize < 1 || fontSize > 20)
    {
        std::cout << "font size should be in the range of 1 to 20" << std::endl;
        return 0;
    }

    statusBar = input.cmdOptionExists("-s") || input.cmdOptionExists("--statusbar");

    start(pathToNotepad, pathToVideo, width, height, fontSize, statusBar);
    return 0;
}