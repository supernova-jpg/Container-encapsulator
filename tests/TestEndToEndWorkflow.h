#ifndef TESTENDTOENDWORKFLOW_H
#define TESTENDTOENDWORKFLOW_H

#include <QObject>
#include <QtTest>
#include <QTimer>
#include <QElapsedTimer>
#include "TestHelper.h"
#include "../src/ui/MainWindow.h"
#include "../src/core/FileProcessor.h"
#include "../src/core/MediaAnalyzer.h"

/**
 * TestEndToEndWorkflow
 * 
 * Comprehensive end-to-end tests that verify the complete processing 
 * workflow from file addition to final output, ensuring all components
 * work together seamlessly with proper metadata handling.
 * 
 * This test class validates the entire user journey and critical 
 * FFmpeg processing pipeline with real-world scenarios.
 */
class TestEndToEndWorkflow : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanupTestCase();
    void init();
    void cleanup();

    // Complete workflow tests
    void testBasicEndToEndWorkflow();
    void testMultiFileWorkflow();
    void testMixedFormatWorkflow();
    void testHDRWorkflow();
    void test4KWorkflow();
    void testRawStreamWorkflow();
    
    // User experience workflows
    void testDragDropToProcessWorkflow();
    void testAddFolderToProcessWorkflow();
    void testQuickProcessWorkflow();
    void testBatchProcessWorkflow();
    
    // Error handling workflows
    void testWorkflowWithCorruptFiles();
    void testWorkflowWithInsufficientSpace();
    void testWorkflowWithMissingFFmpeg();
    void testWorkflowWithPermissionErrors();
    
    // Format conversion workflows
    void testMP4ToMKVWorkflow();
    void testMKVToMP4Workflow();
    void testH264ToH265Workflow();
    void testSDRToHDRWorkflow();
    
    // Performance workflows
    void testLargeFileWorkflow();
    void testManyFilesWorkflow();
    void testHighResolutionWorkflow();
    void testLongDurationWorkflow();
    
    // Quality assurance workflows
    void testMetadataPreservationWorkflow();
    void testQualityVerificationWorkflow();
    void testOutputValidationWorkflow();
    void testChecksumVerificationWorkflow();
    
    // Integration workflows
    void testFFmpegIntegrationWorkflow();
    void testUIIntegrationWorkflow();
    void testFileSystemIntegrationWorkflow();
    void testThemeIntegrationWorkflow();
    
    // Real-world scenario tests
    void testYouTubeVideoProcessing();
    void testSecurityCameraFootage();
    void testScreenRecordingWorkflow();
    void testGamingClipWorkflow();
    void testProfessionalVideoWorkflow();
    
    // Regression prevention tests
    void testOriginalBugScenarios();
    void testPreviouslyFailingFiles();
    void testEdgeCaseFiles();

private:
    // Workflow execution helpers
    bool executeCompleteWorkflow(const QStringList &inputFiles, 
                               const QString &outputFormat = "mp4",
                               int timeoutMs = 60000);
    
    bool addFilesAndWaitForAnalysis(const QStringList &files, int timeoutMs = 30000);
    bool startProcessingAndWaitForCompletion(int timeoutMs = 120000);
    bool verifyOutputFiles(const QStringList &expectedOutputs);
    
    // Verification helpers
    bool verifyOutputFileQuality(const QString &outputFile, const QString &inputFile);
    bool verifyMetadataPreservation(const QString &outputFile, const QString &inputFile);
    bool verifyFileIntegrity(const QString &filePath);
    QString calculateFileChecksum(const QString &filePath);
    
    // Performance measurement
    struct WorkflowMetrics {
        qint64 analysisTimeMs;
        qint64 processingTimeMs;
        qint64 totalTimeMs;
        qint64 memoryUsageMB;
        double cpuUsagePercent;
        QStringList errors;
        QStringList warnings;
    };
    
    WorkflowMetrics measureWorkflowPerformance();
    void logPerformanceMetrics(const WorkflowMetrics &metrics);
    
    // Test data creation
    QStringList createRealWorldTestFiles();
    QString createLargeTestFile(qint64 sizeMB = 100);
    QString createHDRTestFile();
    QString create4KTestFile();
    QStringList createBatchTestFiles(int count = 10);
    
    // Mock scenario setup
    void simulateYouTubeDownload();
    void simulateSecurityCamera();
    void simulateScreenRecording();
    void simulateGamingCapture();
    
    TestHelper *m_helper;
    MainWindow *m_mainWindow;
    FileProcessor *m_processor;
    MediaAnalyzer *m_analyzer;
    
    QStringList m_testFiles;
    QStringList m_outputFiles;
    QString m_outputDir;
    
    // Performance tracking
    QElapsedTimer m_workflowTimer;
    WorkflowMetrics m_lastMetrics;
};

#endif // TESTENDTOENDWORKFLOW_H