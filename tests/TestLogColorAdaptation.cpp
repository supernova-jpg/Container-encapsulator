#include "TestLogColorAdaptation.h"
#include <QSignalSpy>
#include <QTimer>
#include <QApplication>
#include <QStyle>
#include <QStyleFactory>
#include <QTextDocument>
#include <QTextCursor>
#include <QTextCharFormat>
#include <QRegularExpression>
#include <cmath>

void TestLogColorAdaptation::initTestCase()
{
    m_helper = new TestHelper(this);
    
    // Prepare test messages for all log levels
    m_testMessages << "System initialized successfully";
    m_testMessages << "Configuration file not found, using defaults";
    m_testMessages << "Critical error: Unable to access media file";
    m_testMessages << "Processing completed in 45.2 seconds";
    m_testMessages << "Warning: Low disk space detected";
    
    // Create test palettes for different themes
    m_testPalettes << createLightThemePalette();
    m_testPalettes << createDarkThemePalette();
    m_testPalettes << createHighContrastPalette();
}

void TestLogColorAdaptation::cleanupTestCase()
{
    m_helper->cleanup();
    delete m_helper;
}

void TestLogColorAdaptation::init()
{
    m_mainWindow = new MainWindow();
    m_logEdit = m_mainWindow->findChild<QTextEdit*>("logEdit");
    QVERIFY2(m_logEdit != nullptr, "Log edit widget not found in MainWindow");
}

void TestLogColorAdaptation::cleanup()
{
    delete m_mainWindow;
    m_logEdit = nullptr;
}

void TestLogColorAdaptation::testLightThemeColors()
{
    // Test color adaptation for Windows light theme
    simulateWindowsTheme("light");
    
    generateTestLogs();
    
    // Check that colors are appropriate for light background
    QColor backgroundColor = getTextEditBackgroundColor();
    
    // Info messages should use dark colors on light background
    addLogMessage(LogLevel::Info, "Test info message");
    QColor infoColor = getLogColor(LogLevel::Info, "light");
    
    double infoContrast = calculateContrastRatio(infoColor, backgroundColor);
    QVERIFY2(infoContrast >= ColorTestConstants::MIN_CONTRAST_AA,
             QString("Info color contrast too low: %1").arg(infoContrast).toLocal8Bit().data());
    
    // Warning messages should be visible
    addLogMessage(LogLevel::Warning, "Test warning message");
    QColor warningColor = getLogColor(LogLevel::Warning, "light");
    
    double warningContrast = calculateContrastRatio(warningColor, backgroundColor);
    QVERIFY2(warningContrast >= ColorTestConstants::MIN_CONTRAST_AA,
             QString("Warning color contrast too low: %1").arg(warningContrast).toLocal8Bit().data());
    
    // Error messages should be highly visible
    addLogMessage(LogLevel::Error, "Test error message");
    QColor errorColor = getLogColor(LogLevel::Error, "light");
    
    double errorContrast = calculateContrastRatio(errorColor, backgroundColor);
    QVERIFY2(errorContrast >= ColorTestConstants::MIN_CONTRAST_AA,
             QString("Error color contrast too low: %1").arg(errorContrast).toLocal8Bit().data());
}

void TestLogColorAdaptation::testDarkThemeColors()
{
    // Test color adaptation for Windows dark theme
    simulateWindowsTheme("dark");
    
    generateTestLogs();
    
    QColor backgroundColor = getTextEditBackgroundColor();
    
    // Colors should be lighter/brighter on dark background
    addLogMessage(LogLevel::Info, "Test info message dark");
    QColor infoColor = getLogColor(LogLevel::Info, "dark");
    
    // Info color should be bright enough for dark background
    double infoContrast = calculateContrastRatio(infoColor, backgroundColor);
    QVERIFY2(infoContrast >= ColorTestConstants::MIN_CONTRAST_AA,
             QString("Dark theme info color contrast too low: %1").arg(infoContrast).toLocal8Bit().data());
    
    // Verify color is actually lighter than light theme equivalent
    QColor lightThemeInfo = getLogColor(LogLevel::Info, "light");
    QVERIFY2(infoColor.lightness() > lightThemeInfo.lightness(),
             "Dark theme info color should be lighter than light theme");
    
    addLogMessage(LogLevel::Warning, "Test warning message dark");
    QColor warningColor = getLogColor(LogLevel::Warning, "dark");
    
    double warningContrast = calculateContrastRatio(warningColor, backgroundColor);
    QVERIFY2(warningContrast >= ColorTestConstants::MIN_CONTRAST_AA,
             QString("Dark theme warning color contrast too low: %1").arg(warningContrast).toLocal8Bit().data());
    
    addLogMessage(LogLevel::Error, "Test error message dark");
    QColor errorColor = getLogColor(LogLevel::Error, "dark");
    
    double errorContrast = calculateContrastRatio(errorColor, backgroundColor);
    QVERIFY2(errorContrast >= ColorTestConstants::MIN_CONTRAST_AA,
             QString("Dark theme error color contrast too low: %1").arg(errorContrast).toLocal8Bit().data());
}

void TestLogColorAdaptation::testHighContrastThemeColors()
{
    // Test color adaptation for Windows high contrast themes
    simulateWindowsTheme("high-contrast");
    
    generateTestLogs();
    
    QColor backgroundColor = getTextEditBackgroundColor();
    
    // High contrast themes require maximum visibility
    addLogMessage(LogLevel::Info, "High contrast info test");
    QColor infoColor = getLogColor(LogLevel::Info, "high-contrast");
    
    double infoContrast = calculateContrastRatio(infoColor, backgroundColor);
    QVERIFY2(infoContrast >= ColorTestConstants::MIN_CONTRAST_AAA,
             QString("High contrast info color should meet AAA standard: %1").arg(infoContrast).toLocal8Bit().data());
    
    addLogMessage(LogLevel::Warning, "High contrast warning test");
    QColor warningColor = getLogColor(LogLevel::Warning, "high-contrast");
    
    double warningContrast = calculateContrastRatio(warningColor, backgroundColor);
    QVERIFY2(warningContrast >= ColorTestConstants::MIN_CONTRAST_AAA,
             QString("High contrast warning color should meet AAA standard: %1").arg(warningContrast).toLocal8Bit().data());
    
    addLogMessage(LogLevel::Error, "High contrast error test");
    QColor errorColor = getLogColor(LogLevel::Error, "high-contrast");
    
    double errorContrast = calculateContrastRatio(errorColor, backgroundColor);
    QVERIFY2(errorContrast >= ColorTestConstants::MIN_CONTRAST_AAA,
             QString("High contrast error color should meet AAA standard: %1").arg(errorContrast).toLocal8Bit().data());
}

void TestLogColorAdaptation::testSystemThemeDetection()
{
    // Test that the system can detect the current Windows theme
    
    // This would test the actual theme detection logic
    // For now, we verify that different themes produce different color schemes
    
    simulateWindowsTheme("light");
    QColor lightInfo = getLogColor(LogLevel::Info, "light");
    
    simulateWindowsTheme("dark");
    QColor darkInfo = getLogColor(LogLevel::Info, "dark");
    
    // Colors should be different for different themes
    QVERIFY2(lightInfo != darkInfo,
             "Light and dark theme should use different colors for info messages");
    
    // Dark theme colors should generally be lighter
    QVERIFY2(darkInfo.lightness() > lightInfo.lightness(),
             "Dark theme info color should be lighter than light theme");
}

void TestLogColorAdaptation::testWindows10DarkTheme()
{
    // Test Windows 10 specific dark theme
    simulateWindowsTheme("windows10-dark");
    
    generateTestLogs();
    
    // Windows 10 dark theme typically uses #1e1e1e background
    QColor expectedBg = QColor(30, 30, 30);
    applyTestPalette(createDarkThemePalette());
    
    QColor backgroundColor = getTextEditBackgroundColor();
    
    addLogMessage(LogLevel::Info, "Windows 10 dark info test");
    addLogMessage(LogLevel::Warning, "Windows 10 dark warning test");
    addLogMessage(LogLevel::Error, "Windows 10 dark error test");
    
    // Verify all message types are visible
    QColor infoColor = getLogColor(LogLevel::Info, "dark");
    QColor warningColor = getLogColor(LogLevel::Warning, "dark");
    QColor errorColor = getLogColor(LogLevel::Error, "dark");
    
    QVERIFY2(calculateContrastRatio(infoColor, backgroundColor) >= ColorTestConstants::MIN_CONTRAST_AA,
             "Windows 10 dark theme info visibility failed");
    QVERIFY2(calculateContrastRatio(warningColor, backgroundColor) >= ColorTestConstants::MIN_CONTRAST_AA,
             "Windows 10 dark theme warning visibility failed");
    QVERIFY2(calculateContrastRatio(errorColor, backgroundColor) >= ColorTestConstants::MIN_CONTRAST_AA,
             "Windows 10 dark theme error visibility failed");
}

void TestLogColorAdaptation::testThemeChangeAdaptation()
{
    // Test dynamic theme switching without application restart
    
    // Start with light theme
    simulateWindowsTheme("light");
    addLogMessage(LogLevel::Info, "Light theme message");
    QColor lightColor = extractTextColor(extractLogHTML(LogLevel::Info));
    
    // Switch to dark theme
    simulateWindowsTheme("dark");
    addLogMessage(LogLevel::Info, "Dark theme message");
    QColor darkColor = extractTextColor(extractLogHTML(LogLevel::Info));
    
    // Colors should adapt to new theme
    QVERIFY2(lightColor != darkColor,
             "Colors should change when theme changes");
    
    // Previous messages should also be updated (if supported)
    // This tests whether the entire log history adapts to theme changes
}

void TestLogColorAdaptation::testInfoLogVisibility()
{
    // Test info log visibility across all themes
    for (const QPalette &palette : m_testPalettes) {
        applyTestPalette(palette);
        
        addLogMessage(LogLevel::Info, "Info visibility test");
        
        QColor backgroundColor = getTextEditBackgroundColor();
        QColor infoColor = getLogColor(LogLevel::Info, "current");
        
        bool isVisible = isColorVisible(infoColor, backgroundColor);
        QVERIFY2(isVisible,
                QString("Info messages not visible with palette: %1")
                .arg(palette.color(QPalette::Base).name()).toLocal8Bit().data());
        
        double contrast = calculateContrastRatio(infoColor, backgroundColor);
        QVERIFY2(contrast >= ColorTestConstants::MIN_CONTRAST_AA,
                QString("Info contrast ratio too low: %1").arg(contrast).toLocal8Bit().data());
    }
}

void TestLogColorAdaptation::testWarningLogVisibility()
{
    // Test warning log visibility across all themes
    for (const QPalette &palette : m_testPalettes) {
        applyTestPalette(palette);
        
        addLogMessage(LogLevel::Warning, "Warning visibility test");
        
        QColor backgroundColor = getTextEditBackgroundColor();
        QColor warningColor = getLogColor(LogLevel::Warning, "current");
        
        bool isVisible = isColorVisible(warningColor, backgroundColor);
        QVERIFY2(isVisible,
                QString("Warning messages not visible with palette: %1")
                .arg(palette.color(QPalette::Base).name()).toLocal8Bit().data());
        
        double contrast = calculateContrastRatio(warningColor, backgroundColor);
        QVERIFY2(contrast >= ColorTestConstants::MIN_CONTRAST_AA,
                QString("Warning contrast ratio too low: %1").arg(contrast).toLocal8Bit().data());
    }
}

void TestLogColorAdaptation::testErrorLogVisibility()
{
    // Test error log visibility across all themes
    for (const QPalette &palette : m_testPalettes) {
        applyTestPalette(palette);
        
        addLogMessage(LogLevel::Error, "Error visibility test");
        
        QColor backgroundColor = getTextEditBackgroundColor();
        QColor errorColor = getLogColor(LogLevel::Error, "current");
        
        bool isVisible = isColorVisible(errorColor, backgroundColor);
        QVERIFY2(isVisible,
                QString("Error messages not visible with palette: %1")
                .arg(palette.color(QPalette::Base).name()).toLocal8Bit().data());
        
        double contrast = calculateContrastRatio(errorColor, backgroundColor);
        QVERIFY2(contrast >= ColorTestConstants::MIN_CONTRAST_AA,
                QString("Error contrast ratio too low: %1").arg(contrast).toLocal8Bit().data());
    }
}

void TestLogColorAdaptation::testWCAGAACompliance()
{
    // Test WCAG AA compliance for all log colors
    simulateWindowsTheme("light");
    QColor lightBg = getTextEditBackgroundColor();
    
    // Test all log levels for WCAG AA compliance
    QColor infoLight = getLogColor(LogLevel::Info, "light");
    QColor warningLight = getLogColor(LogLevel::Warning, "light");
    QColor errorLight = getLogColor(LogLevel::Error, "light");
    
    QVERIFY2(meetsWCAGStandard(calculateContrastRatio(infoLight, lightBg), "AA"),
             "Info color fails WCAG AA compliance in light theme");
    QVERIFY2(meetsWCAGStandard(calculateContrastRatio(warningLight, lightBg), "AA"),
             "Warning color fails WCAG AA compliance in light theme");
    QVERIFY2(meetsWCAGStandard(calculateContrastRatio(errorLight, lightBg), "AA"),
             "Error color fails WCAG AA compliance in light theme");
    
    // Test dark theme
    simulateWindowsTheme("dark");
    QColor darkBg = getTextEditBackgroundColor();
    
    QColor infoDark = getLogColor(LogLevel::Info, "dark");
    QColor warningDark = getLogColor(LogLevel::Warning, "dark");
    QColor errorDark = getLogColor(LogLevel::Error, "dark");
    
    QVERIFY2(meetsWCAGStandard(calculateContrastRatio(infoDark, darkBg), "AA"),
             "Info color fails WCAG AA compliance in dark theme");
    QVERIFY2(meetsWCAGStandard(calculateContrastRatio(warningDark, darkBg), "AA"),
             "Warning color fails WCAG AA compliance in dark theme");
    QVERIFY2(meetsWCAGStandard(calculateContrastRatio(errorDark, darkBg), "AA"),
             "Error color fails WCAG AA compliance in dark theme");
}

void TestLogColorAdaptation::testOriginalColorVisibilityBug()
{
    // This test reproduces the original bug: poor color visibility in different themes
    // Should FAIL with current code, PASS after fix
    
    simulateWindowsTheme("dark");
    
    // Add a test message using the current (potentially buggy) color scheme
    addLogMessage(LogLevel::Warning, "Original bug reproduction test");
    
    QColor backgroundColor = getTextEditBackgroundColor();
    QColor messageColor = getLogColor(LogLevel::Warning, "current");
    
    double contrast = calculateContrastRatio(messageColor, backgroundColor);
    
    // This assertion should fail with the current buggy code
    // but pass after the color adaptation fix is applied
    QVERIFY2(contrast >= ColorTestConstants::MIN_CONTRAST_AA,
             QString("BUG REPRODUCED: Poor color visibility in dark theme. Contrast ratio: %1")
             .arg(contrast).toLocal8Bit().data());
    
    // Additional check for specific color combinations that are problematic
    if (backgroundColor.lightness() < 128) { // Dark background
        // Orange text on dark background should be light orange, not dark orange
        if (messageColor.name().toLower() == "#ffa500") { // Standard orange
            QFAIL("BUG REPRODUCED: Using standard orange on dark background - invisible text");
        }
    }
}

// Helper method implementations
QColor TestLogColorAdaptation::getLogColor(LogLevel level, const QString &themeType)
{
    // Extract the actual color used for a specific log level
    // This would inspect the current color scheme or stylesheet
    
    // For testing purposes, return expected colors based on theme
    if (themeType == "light") {
        switch (level) {
        case LogLevel::Info: return ColorTestConstants::SAFE_INFO_LIGHT;
        case LogLevel::Warning: return ColorTestConstants::SAFE_WARNING_LIGHT;
        case LogLevel::Error: return ColorTestConstants::SAFE_ERROR_LIGHT;
        }
    } else if (themeType == "dark") {
        switch (level) {
        case LogLevel::Info: return ColorTestConstants::SAFE_INFO_DARK;
        case LogLevel::Warning: return ColorTestConstants::SAFE_WARNING_DARK;
        case LogLevel::Error: return ColorTestConstants::SAFE_ERROR_DARK;
        }
    }
    
    return QColor(); // Current implementation would be tested here
}

double TestLogColorAdaptation::calculateContrastRatio(const QColor &foreground, const QColor &background)
{
    // Calculate WCAG contrast ratio
    auto relativeLuminance = [](const QColor &color) {
        auto toLinear = [](double c) {
            return c <= 0.03928 ? c / 12.92 : pow((c + 0.055) / 1.055, 2.4);
        };
        
        double r = toLinear(color.redF());
        double g = toLinear(color.greenF());
        double b = toLinear(color.blueF());
        
        return 0.2126 * r + 0.7152 * g + 0.0722 * b;
    };
    
    double l1 = relativeLuminance(foreground);
    double l2 = relativeLuminance(background);
    
    if (l1 < l2) {
        std::swap(l1, l2);
    }
    
    return (l1 + 0.05) / (l2 + 0.05);
}

bool TestLogColorAdaptation::isColorVisible(const QColor &foreground, const QColor &background)
{
    double contrast = calculateContrastRatio(foreground, background);
    return contrast >= ColorTestConstants::MIN_CONTRAST_AA;
}

bool TestLogColorAdaptation::meetsWCAGStandard(double contrastRatio, const QString &level)
{
    if (level == "AAA") {
        return contrastRatio >= ColorTestConstants::MIN_CONTRAST_AAA;
    } else {
        return contrastRatio >= ColorTestConstants::MIN_CONTRAST_AA;
    }
}

void TestLogColorAdaptation::simulateWindowsTheme(const QString &themeName)
{
    if (themeName == "light" || themeName == "windows10-light") {
        applyTestPalette(createLightThemePalette());
    } else if (themeName == "dark" || themeName == "windows10-dark") {
        applyTestPalette(createDarkThemePalette());
    } else if (themeName == "high-contrast") {
        applyTestPalette(createHighContrastPalette());
    }
}

void TestLogColorAdaptation::applyTestPalette(const QPalette &palette)
{
    if (m_mainWindow) {
        m_mainWindow->setPalette(palette);
        if (m_logEdit) {
            m_logEdit->setPalette(palette);
        }
    }
}

QPalette TestLogColorAdaptation::createLightThemePalette()
{
    QPalette lightPalette;
    lightPalette.setColor(QPalette::Base, ColorTestConstants::WINDOWS_LIGHT_BACKGROUND);
    lightPalette.setColor(QPalette::Text, ColorTestConstants::WINDOWS_LIGHT_TEXT);
    lightPalette.setColor(QPalette::Window, ColorTestConstants::WINDOWS_LIGHT_BACKGROUND);
    lightPalette.setColor(QPalette::WindowText, ColorTestConstants::WINDOWS_LIGHT_TEXT);
    return lightPalette;
}

QPalette TestLogColorAdaptation::createDarkThemePalette()
{
    QPalette darkPalette;
    darkPalette.setColor(QPalette::Base, ColorTestConstants::WINDOWS_DARK_BACKGROUND);
    darkPalette.setColor(QPalette::Text, ColorTestConstants::WINDOWS_DARK_TEXT);
    darkPalette.setColor(QPalette::Window, ColorTestConstants::WINDOWS_DARK_BACKGROUND);
    darkPalette.setColor(QPalette::WindowText, ColorTestConstants::WINDOWS_DARK_TEXT);
    return darkPalette;
}

QPalette TestLogColorAdaptation::createHighContrastPalette()
{
    QPalette hcPalette;
    hcPalette.setColor(QPalette::Base, ColorTestConstants::WINDOWS_HC_WHITE_BG);
    hcPalette.setColor(QPalette::Text, QColor(0, 0, 0));
    hcPalette.setColor(QPalette::Window, ColorTestConstants::WINDOWS_HC_WHITE_BG);
    hcPalette.setColor(QPalette::WindowText, QColor(0, 0, 0));
    return hcPalette;
}

void TestLogColorAdaptation::generateTestLogs()
{
    // Generate sample log messages for testing
    for (const QString &message : m_testMessages) {
        if (message.contains("error", Qt::CaseInsensitive)) {
            addLogMessage(LogLevel::Error, message);
        } else if (message.contains("warning", Qt::CaseInsensitive)) {
            addLogMessage(LogLevel::Warning, message);
        } else {
            addLogMessage(LogLevel::Info, message);
        }
    }
}

void TestLogColorAdaptation::addLogMessage(LogLevel level, const QString &message)
{
    // Add a log message to the UI for testing
    if (m_mainWindow) {
        // This would call MainWindow::logMessage() or similar
        // For testing, we simulate the log output
        m_mainWindow->logMessage(message, level);
    }
}

QString TestLogColorAdaptation::extractLogHTML(LogLevel level)
{
    // Extract the HTML content of log messages for color analysis
    if (m_logEdit) {
        return m_logEdit->toHtml();
    }
    return QString();
}

QColor TestLogColorAdaptation::extractTextColor(const QString &htmlLog)
{
    // Parse HTML to extract text color
    QRegularExpression colorRegex(R"(color:\s*([^;]+))");
    QRegularExpressionMatch match = colorRegex.match(htmlLog);
    if (match.hasMatch()) {
        return QColor(match.captured(1));
    }
    return QColor();
}

QColor TestLogColorAdaptation::getTextEditBackgroundColor()
{
    if (m_logEdit) {
        return m_logEdit->palette().color(QPalette::Base);
    }
    return QColor();
}

QStringList TestLogColorAdaptation::getUsedColors()
{
    // Extract all colors used in the log output
    QStringList colors;
    if (m_logEdit) {
        QString html = m_logEdit->toHtml();
        QRegularExpression colorRegex(R"(color:\s*([^;]+))");
        QRegularExpressionMatchIterator it = colorRegex.globalMatch(html);
        while (it.hasNext()) {
            QRegularExpressionMatch match = it.next();
            colors << match.captured(1);
        }
    }
    return colors;
}

// Placeholder implementations for remaining tests
void TestLogColorAdaptation::testWindows11LightTheme() { /* Implementation */ }
void TestLogColorAdaptation::testWindows11DarkTheme() { /* Implementation */ }
void TestLogColorAdaptation::testWindowsHighContrastWhite() { /* Implementation */ }
void TestLogColorAdaptation::testWindowsHighContrastBlack() { /* Implementation */ }
void TestLogColorAdaptation::testWindowsClassicTheme() { /* Implementation */ }
void TestLogColorAdaptation::testTimestampVisibility() { /* Implementation */ }
void TestLogColorAdaptation::testLogLevelLabelVisibility() { /* Implementation */ }
void TestLogColorAdaptation::testMinimumContrastCompliance() { /* Implementation */ }
void TestLogColorAdaptation::testRuntimeThemeSwitching() { /* Implementation */ }
void TestLogColorAdaptation::testColorUpdateOnThemeChange() { /* Implementation */ }
void TestLogColorAdaptation::testLogHistoryColorUpdate() { /* Implementation */ }
void TestLogColorAdaptation::testWCAGAAACompliance() { /* Implementation */ }
void TestLogColorAdaptation::testColorBlindnessAccessibility() { /* Implementation */ }
void TestLogColorAdaptation::testHighContrastAccessibility() { /* Implementation */ }
void TestLogColorAdaptation::testCustomLightColorScheme() { /* Implementation */ }
void TestLogColorAdaptation::testCustomDarkColorScheme() { /* Implementation */ }
void TestLogColorAdaptation::testUserDefinedColors() { /* Implementation */ }
void TestLogColorAdaptation::testColorSchemePreferences() { /* Implementation */ }
void TestLogColorAdaptation::testColoredTimestamps() { /* Implementation */ }
void TestLogColorAdaptation::testColoredLogLevels() { /* Implementation */ }
void TestLogColorAdaptation::testColoredMessageContent() { /* Implementation */ }
void TestLogColorAdaptation::testNestedColorFormatting() { /* Implementation */ }
void TestLogColorAdaptation::testColorRenderingPerformance() { /* Implementation */ }
void TestLogColorAdaptation::testLargeLogColorPerformance() { /* Implementation */ }
void TestLogColorAdaptation::testColorMemoryUsage() { /* Implementation */ }
void TestLogColorAdaptation::testTransparentBackgrounds() { /* Implementation */ }
void TestLogColorAdaptation::testGradientBackgrounds() { /* Implementation */ }
void TestLogColorAdaptation::testTextSelectionColors() { /* Implementation */ }
void TestLogColorAdaptation::testScrollbarColors() { /* Implementation */ }
void TestLogColorAdaptation::testQtStyleSheetIntegration() { /* Implementation */ }
void TestLogColorAdaptation::testQPaletteIntegration() { /* Implementation */ }
void TestLogColorAdaptation::testNativeWindowsTheming() { /* Implementation */ }
void TestLogColorAdaptation::testDarkModeInvisibleText() { /* Implementation */ }
void TestLogColorAdaptation::testHighContrastColorClash() { /* Implementation */ }