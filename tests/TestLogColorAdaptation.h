#ifndef TESTLOGCOLORADAPTATION_H
#define TESTLOGCOLORADAPTATION_H

#include <QObject>
#include <QtTest>
#include <QColor>
#include <QTextEdit>
#include <QApplication>
#include <QPalette>
#include "TestHelper.h"
#include "../src/ui/MainWindow.h"

/**
 * TestLogColorAdaptation
 * 
 * Tests for the third major bug: Log output color visibility issues 
 * with Windows themes (especially dark mode and high contrast).
 * 
 * This test class verifies that log colors adapt appropriately to
 * the system theme, ensuring visibility and accessibility across
 * light, dark, and high-contrast Windows themes.
 */
class TestLogColorAdaptation : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanupTestCase();
    void init();
    void cleanup();

    // Core color adaptation tests
    void testLightThemeColors();
    void testDarkThemeColors();
    void testHighContrastThemeColors();
    void testSystemThemeDetection();
    
    // Windows theme integration
    void testWindows10LightTheme();
    void testWindows10DarkTheme();
    void testWindows11LightTheme();
    void testWindows11DarkTheme();
    void testWindowsHighContrastWhite();
    void testWindowsHighContrastBlack();
    void testWindowsClassicTheme();
    
    // Color visibility tests
    void testInfoLogVisibility();
    void testWarningLogVisibility();
    void testErrorLogVisibility();
    void testTimestampVisibility();
    void testLogLevelLabelVisibility();
    
    // Contrast ratio tests
    void testInfoLogContrastRatio();
    void testWarningLogContrastRatio();
    void testErrorLogContrastRatio();
    void testMinimumContrastCompliance();
    
    // Dynamic theme switching tests
    void testThemeChangeAdaptation();
    void testRuntimeThemeSwitching();
    void testColorUpdateOnThemeChange();
    void testLogHistoryColorUpdate();
    
    // Accessibility compliance tests
    void testWCAGAACompliance();
    void testWCAGAAACompliance();
    void testColorBlindnessAccessibility();
    void testHighContrastAccessibility();
    
    // Custom color scheme tests
    void testCustomLightColorScheme();
    void testCustomDarkColorScheme();
    void testUserDefinedColors();
    void testColorSchemePreferences();
    
    // Log formatting tests
    void testColoredTimestamps();
    void testColoredLogLevels();
    void testColoredMessageContent();
    void testNestedColorFormatting();
    
    // Performance tests
    void testColorRenderingPerformance();
    void testLargeLogColorPerformance();
    void testColorMemoryUsage();
    
    // Edge case tests
    void testTransparentBackgrounds();
    void testGradientBackgrounds();
    void testTextSelectionColors();
    void testScrollbarColors();
    
    // Integration with Qt theming
    void testQtStyleSheetIntegration();
    void testQPaletteIntegration();
    void testNativeWindowsTheming();
    
    // Regression tests for color bugs
    void testOriginalColorVisibilityBug();
    void testDarkModeInvisibleText();
    void testHighContrastColorClash();

private:
    // Color testing utilities
    QColor getLogColor(LogLevel level, const QString &themeType);
    double calculateContrastRatio(const QColor &foreground, const QColor &background);
    bool isColorVisible(const QColor &foreground, const QColor &background);
    bool meetsWCAGStandard(double contrastRatio, const QString &level = "AA");
    
    // Theme simulation utilities
    void simulateWindowsTheme(const QString &themeName);
    void applyTestPalette(const QPalette &palette);
    QPalette createLightThemePalette();
    QPalette createDarkThemePalette();
    QPalette createHighContrastPalette();
    
    // Log content generation
    void generateTestLogs();
    void addLogMessage(LogLevel level, const QString &message);
    QString extractLogHTML(LogLevel level);
    
    // Color extraction from UI
    QColor extractTextColor(const QString &htmlLog);
    QColor getTextEditBackgroundColor();
    QStringList getUsedColors();
    
    TestHelper *m_helper;
    MainWindow *m_mainWindow;
    QTextEdit *m_logEdit;
    
    // Test data
    QStringList m_testMessages;
    QList<QPalette> m_testPalettes;
};

// Color constants for testing
namespace ColorTestConstants {
    // WCAG contrast ratios
    const double MIN_CONTRAST_AA = 4.5;  // WCAG AA standard
    const double MIN_CONTRAST_AAA = 7.0; // WCAG AAA standard
    const double MIN_CONTRAST_LARGE = 3.0; // WCAG AA for large text
    
    // Standard Windows theme colors
    const QColor WINDOWS_LIGHT_BACKGROUND = QColor(255, 255, 255);
    const QColor WINDOWS_LIGHT_TEXT = QColor(0, 0, 0);
    const QColor WINDOWS_DARK_BACKGROUND = QColor(32, 32, 32);
    const QColor WINDOWS_DARK_TEXT = QColor(255, 255, 255);
    const QColor WINDOWS_HC_WHITE_BG = QColor(255, 255, 255);
    const QColor WINDOWS_HC_BLACK_BG = QColor(0, 0, 0);
    
    // Log level color expectations
    const QColor SAFE_INFO_LIGHT = QColor(0, 100, 0);      // Dark green on light
    const QColor SAFE_INFO_DARK = QColor(144, 238, 144);   // Light green on dark
    const QColor SAFE_WARNING_LIGHT = QColor(204, 102, 0); // Dark orange on light
    const QColor SAFE_WARNING_DARK = QColor(255, 165, 0);  // Orange on dark
    const QColor SAFE_ERROR_LIGHT = QColor(139, 0, 0);     // Dark red on light
    const QColor SAFE_ERROR_DARK = QColor(255, 99, 71);    // Light red on dark
}

#endif // TESTLOGCOLORADAPTATION_H