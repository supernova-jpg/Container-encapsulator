#ifndef PATCHVERIFICATIONFRAMEWORK_H
#define PATCHVERIFICATIONFRAMEWORK_H

#include <QObject>
#include <QString>
#include <QStringList>
#include <QDir>
#include <QProcess>
#include <QTemporaryDir>
#include <QElapsedTimer>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

/**
 * PatchVerificationFramework
 * 
 * Comprehensive framework for verifying patches created by other agents.
 * This framework can:
 * 1. Create clean temporary copies of source code
 * 2. Apply patches using git apply or patch command
 * 3. Build the patched code
 * 4. Run the complete test suite
 * 5. Analyze results and generate detailed reports
 * 
 * Designed to work in MODE 2: verify_patch as specified in the QA Agent role.
 */
class PatchVerificationFramework : public QObject
{
    Q_OBJECT

public:
    explicit PatchVerificationFramework(QObject *parent = nullptr);
    ~PatchVerificationFramework();

    // Main patch verification interface
    enum VerificationResult {
        PASSED,
        FAILED,
        BUILD_ERROR,
        PATCH_APPLY_ERROR,
        TIMEOUT_ERROR,
        UNKNOWN_ERROR
    };

    struct PatchVerificationReport {
        VerificationResult result;
        QString patchFile;
        QStringList buildErrors;
        QStringList testFailures;
        QStringList newFailures;
        QStringList regressions;
        QStringList warnings;
        qint64 buildTimeMs;
        qint64 testTimeMs;
        qint64 totalTimeMs;
        int totalTests;
        int passedTests;
        int failedTests;
        int skippedTests;
        QString detailedReport;
        QString sprintLogEntry;
    };

    // Core verification methods
    PatchVerificationReport verifyPatch(const QString &patchFilePath);
    bool setupCleanEnvironment();
    bool applyPatch(const QString &patchFilePath);
    bool buildPatchedCode();
    bool runFullTestSuite();
    PatchVerificationReport generateReport();

    // Configuration
    void setSourceDirectory(const QString &sourceDir);
    void setTestCommand(const QString &testCommand);
    void setBuildCommand(const QString &buildCommand);
    void setTimeout(int timeoutSeconds);
    void setVerboseOutput(bool verbose);

    // Utilities
    QString getLastError() const { return m_lastError; }
    QStringList getDetailedLogs() const { return m_detailedLogs; }

private:
    // Environment management
    bool createWorkingCopy();
    bool validateSourceDirectory();
    void cleanupWorkingCopy();

    // Patch operations
    bool detectPatchFormat(const QString &patchFilePath);
    bool applyGitPatch(const QString &patchFilePath);
    bool applyUnifiedPatch(const QString &patchFilePath);
    QStringList extractPatchInfo(const QString &patchFilePath);

    // Build operations
    bool buildWithQMake();
    bool buildWithCMake();
    bool buildWithMake();
    QStringList parseBuildErrors(const QString &buildOutput);

    // Test execution
    bool runQTestSuite();
    bool runCustomTestSuite();
    struct TestResult parseTestOutput(const QString &testOutput);
    QStringList identifyRegressions(const TestResult &current, const TestResult &baseline);

    // Analysis and reporting
    void analyzeTestResults();
    void generateSprintLogEntry();
    QString formatDetailedReport();
    bool compareWithBaseline();

    // Process execution helpers
    bool executeCommand(const QString &program, const QStringList &arguments, 
                       QString &output, QString &error, int timeoutMs = 300000);
    bool executeCommandInDirectory(const QString &workingDir, const QString &program, 
                                 const QStringList &arguments, QString &output, 
                                 QString &error, int timeoutMs = 300000);

    // Configuration
    QString m_sourceDirectory;
    QString m_testCommand;
    QString m_buildCommand;
    int m_timeoutSeconds;
    bool m_verboseOutput;

    // Working environment
    QTemporaryDir *m_workingDir;
    QString m_workingCopyPath;

    // State tracking
    QString m_currentPatchFile;
    QString m_lastError;
    QStringList m_detailedLogs;
    PatchVerificationReport m_currentReport;

    // Build and test results
    struct TestResult {
        QStringList passedTests;
        QStringList failedTests;
        QStringList skippedTests;
        QStringList errors;
        QStringList warnings;
        qint64 executionTimeMs;
    };

    TestResult m_baselineResults;
    TestResult m_patchedResults;
    
    // Timing
    QElapsedTimer m_verificationTimer;
    QElapsedTimer m_buildTimer;
    QElapsedTimer m_testTimer;
};

/**
 * PatchAnalyzer - Helper class for detailed patch analysis
 */
class PatchAnalyzer
{
public:
    struct PatchInfo {
        QString filename;
        QStringList modifiedFiles;
        QStringList addedFiles;
        QStringList deletedFiles;
        int addedLines;
        int deletedLines;
        QStringList affectedFunctions;
        QStringList riskAreas;
        QString summary;
    };

    static PatchInfo analyzePatch(const QString &patchFilePath);
    static QStringList identifyRiskAreas(const PatchInfo &info);
    static QString generatePatchSummary(const PatchInfo &info);

private:
    static QStringList parseUnifiedDiff(const QString &patchContent);
    static QStringList extractFunctionNames(const QString &diffContent);
};

/**
 * TestSuiteRunner - Specialized test execution with detailed reporting
 */
class TestSuiteRunner
{
public:
    struct TestSuiteResult {
        bool success;
        QStringList testFiles;
        QStringList passedTests;
        QStringList failedTests;
        QStringList skippedTests;
        QStringList newFailures;
        QStringList fixedTests;
        QStringList regressions;
        QString summary;
        qint64 totalTimeMs;
    };

    static TestSuiteResult runProMuxerTests(const QString &workingDir);
    static TestSuiteResult runSpecificTests(const QString &workingDir, const QStringList &testNames);
    static bool compareResults(const TestSuiteResult &baseline, const TestSuiteResult &current);

private:
    static QStringList parseQTestOutput(const QString &output);
    static QStringList identifyTestRegressions(const QStringList &baselineFailed, 
                                             const QStringList &currentFailed);
};

/**
 * SprintLogger - Manages sprint_log.md updates
 */
class SprintLogger
{
public:
    static bool updateSprintLog(const PatchVerificationFramework::PatchVerificationReport &report);
    static QString formatLogEntry(const PatchVerificationFramework::PatchVerificationReport &report);
    static bool appendToSprintLog(const QString &entry);

private:
    static QString getSprintLogPath();
    static QString getCurrentTimestamp();
    static QString formatResultStatus(PatchVerificationFramework::VerificationResult result);
};

#endif // PATCHVERIFICATIONFRAMEWORK_H