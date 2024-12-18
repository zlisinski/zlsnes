#include <QApplication>
#include "ui/MainWindow.h"
#include "ui/debugger/DebuggerWindow.h"


int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    app.setOrganizationName("zlisinski");
    app.setApplicationName("zlsnes");

    QCommandLineParser parser;
    parser.addHelpOption();
    parser.addPositionalArgument("romfile", "ROM file to load");
    parser.addOptions({
        {"d", "Start in debugging mode"},
        {"r", "Run-to address (implies -d)", "address"},
        {"n", "Dont save to recent files"}
    });
    parser.process(app);

    bool debug = false;
    bool saveToRecent = true;
    uint32_t runToAddress = INVALID_ADDR;
    QString filename;

    // Check for a ROM file to load.
    if (parser.positionalArguments().size() > 0)
    {
        filename = parser.positionalArguments()[0];
        QFileInfo file(filename);
        if (!file.exists())
        {
            fprintf(stderr, "File %s doesnt exist\n", qPrintable(filename));
            exit(-1);
        }
    }

    // Check for debug flag.
    if (parser.isSet("d"))
    {
        debug = true;
    }

    // Check for saving to recent files list
    if (parser.isSet("n"))
    {
        saveToRecent = false;
    }

    // Check for run-to address.
    if (parser.isSet("r"))
    {
        bool ok;
        runToAddress = parser.value("r").toUInt(&ok, 16);
        if (!ok)
        {
            fprintf(stderr, "Invalid run-to address\n");
            exit(-1);
        }
        if (runToAddress > 0xFFFFFF)
        {
            fprintf(stderr, "Run-to address is too large (0-FFFFFF)\n");
            exit(-1);
        }
        if (filename == "")
        {
            fprintf(stderr, "Using the run-to feature requires a ROM file to load\n");
            exit(-1);
        }
        debug = true;
    }

    MainWindow window(filename, debug, runToAddress, saveToRecent);
    window.show();

    return app.exec();
}