#include <QApplication>
#include <QStyleFactory>
#include <QDir>
#include <QPalette>
#include "ui/MainWindow.h"

int main(int argc, char *argv[])
{
    // CRITICAL PATH: Application entry point - initialize Qt framework
    QApplication app(argc, argv);
    
    app.setApplicationName("Pro Muxer");
    app.setApplicationVersion("1.0.0");
    app.setOrganizationName("ProMuxer");
    
    // Set modern style
    app.setStyle(QStyleFactory::create("Fusion"));
    
    // Set larger font for better readability
    QFont font = app.font();
    font.setPointSize(10); // Increase from default 9 to 10
    font.setFamily("Segoe UI"); // Use modern Windows font
    app.setFont(font);
    
    // Apply dark theme
    QPalette darkPalette;
    darkPalette.setColor(QPalette::Window, QColor(53, 53, 53));
    darkPalette.setColor(QPalette::WindowText, Qt::white);
    darkPalette.setColor(QPalette::Base, QColor(25, 25, 25));
    darkPalette.setColor(QPalette::AlternateBase, QColor(53, 53, 53));
    darkPalette.setColor(QPalette::ToolTipBase, Qt::white);
    darkPalette.setColor(QPalette::ToolTipText, Qt::white);
    darkPalette.setColor(QPalette::Text, Qt::white);
    darkPalette.setColor(QPalette::Button, QColor(53, 53, 53));
    darkPalette.setColor(QPalette::ButtonText, Qt::white);
    darkPalette.setColor(QPalette::BrightText, Qt::red);
    darkPalette.setColor(QPalette::Link, QColor(42, 130, 218));
    darkPalette.setColor(QPalette::Highlight, QColor(42, 130, 218));
    darkPalette.setColor(QPalette::HighlightedText, Qt::black);
    app.setPalette(darkPalette);
    
    MainWindow window;
    window.show();
    
    return app.exec();
}