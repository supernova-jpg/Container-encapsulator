#ifndef TESTAUTOANALYSIS_H
#define TESTAUTOANALYSIS_H

#include <QObject>
#include <QtTest>
#include "TestHelper.h"
#include "../src/core/MediaAnalyzer.h"
#include "../src/ui/MainWindow.h"

/**
 * TestAutoAnalysis
 * 
 * Tests for the second major bug: Manual file analysis requirement 
 * instead of automatic analysis when files are added.
 * 
 * This test class verifies that files are automatically analyzed
 * when added to the application, eliminating the need for users
 * to manually click the "Analyze Files" button.
 */
class TestAutoAnalysis : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanupTestCase();
    void init();
    void cleanup();

    // Core automatic analysis tests
    void testAutoAnalysisOnFileAdd();
    void testAutoAnalysisOnDragAndDrop();
    void testAutoAnalysisOnFolderAdd();
    void testAutoAnalysisOnMultipleFiles();
    
    // UI workflow tests
    void testNoManualAnalyzeButtonRequired();
    void testAnalyzeButtonBecomesSuperfluous();
    void testImplicitAnalysisVsExplicitAnalysis();
    
    // Analysis trigger tests
    void testAnalysisTriggersOnAddFiles();
    void testAnalysisTriggersOnAddFolder();
    void testAnalysisTriggersOnDropFiles();
    void testAnalysisTriggersOnProgrammaticAdd();
    
    // Performance and timing tests
    void testAutoAnalysisDoesNotBlockUI();
    void testAutoAnalysisStartsImmediately();
    void testAutoAnalysisCompletesBefore Processing();
    void testProgressIndicationDuringAutoAnalysis();
    
    // Error handling in auto-analysis
    void testAutoAnalysisWithInvalidFiles();
    void testAutoAnalysisWithMixedValidInvalidFiles();
    void testAutoAnalysisFailureRecovery();
    void testAutoAnalysisWithAccessDeniedFiles();
    
    // Raw stream handling in auto-analysis
    void testAutoAnalysisRawH264Streams();
    void testAutoAnalysisRawH265Streams();
    void testAutoAnalysisAppliesSmartDefaults();
    
    // Batch auto-analysis tests
    void testAutoAnalysisLargeFileBatch();
    void testAutoAnalysisQueueManagement();
    void testAutoAnalysisCancellation();
    
    // Integration with file processor
    void testAutoAnalysisResultsAvailableForProcessing();
    void testProcessingUsesAutoAnalyzedMetadata();
    void testNoProcessingWithoutAnalysis();
    
    // User experience tests
    void testSeamlessWorkflowWithoutManualAnalysis();
    void testUserCanOverrideAutoAnalyzedValues();
    void testAnalysisStatusVisibleToUser();
    void testAnalysisProgressFeedback();
    
    // Regression tests for manual analysis workflow
    void testManualAnalyzeButtonStillWorks();
    void testReAnalysisOfChangedFiles();
    void testAnalysisOnFileReplace();
    
    // Performance benchmarks
    void testAutoAnalysisPerformanceBaseline();
    void testMemoryUsageDuringAutoAnalysis();

private:
    void verifyFileAnalyzedAutomatically(const QString &filePath);
    void simulateFileAdd(const QStringList &files);
    void simulateDragDrop(const QStringList &files);
    void waitForAnalysisComplete();
    bool isAnalysisInProgress();
    
    TestHelper *m_helper;
    MediaAnalyzer *m_analyzer;
    MainWindow *m_mainWindow;
    QStringList m_testFiles;
};

#endif // TESTAUTOANALYSIS_H