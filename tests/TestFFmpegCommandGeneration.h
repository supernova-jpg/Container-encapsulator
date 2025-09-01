#ifndef TESTFFMPEGCOMMANDGENERATION_H
#define TESTFFMPEGCOMMANDGENERATION_H

#include <QObject>
#include <QtTest>
#include "TestHelper.h"
#include "../src/core/FileProcessor.h"
#include "../src/core/MediaAnalyzer.h"
#include "../src/ui/MainWindow.h"

/**
 * TestFFmpegCommandGeneration
 * 
 * Critical tests for the primary bug: FFmpeg commands missing metadata
 * (resolution, framerate, bit depth) causing processing failures
 * 
 * This test class focuses on verifying that FFmpeg commands are generated
 * with complete metadata information extracted from media analysis.
 */
class TestFFmpegCommandGeneration : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanupTestCase();
    void init();
    void cleanup();

    // Core FFmpeg command generation tests
    void testBasicCommandGeneration();
    void testCommandWithMetadata();
    void testCommandWithH264Metadata();
    void testCommandWithH265Metadata();
    void testCommandWithHDRMetadata();
    
    // Metadata inclusion tests
    void testResolutionMetadataInCommand();
    void testFrameRateMetadataInCommand();
    void testBitDepthMetadataInCommand();
    void testColorSpaceMetadataInCommand();
    void testCompleteMetadataIntegration();
    
    // Format-specific command tests
    void testMP4FormatWithMetadata();
    void testMKVFormatWithMetadata();
    void testMOVFormatWithMetadata();
    void testWebMFormatWithMetadata();
    void testTSFormatWithMetadata();
    
    // Raw stream handling tests  
    void testRawH264StreamCommand();
    void testRawH265StreamCommand();
    void testRawStreamWithManualMetadata();
    
    // Error handling tests
    void testCommandGenerationWithMissingMetadata();
    void testCommandGenerationWithInvalidMetadata();
    void testCommandGenerationWithPartialMetadata();
    
    // Integration with MediaAnalyzer tests
    void testCommandAfterSuccessfulAnalysis();
    void testCommandAfterFailedAnalysisWithDefaults();
    void testCommandWithUserEditedMetadata();
    
    // Advanced metadata scenarios
    void testHDR10MetadataIntegration();
    void testDolbyVisionMetadataIntegration();
    void test4KUHDMetadataIntegration();
    void test8KUHDMetadataIntegration();
    void testHighFrameRateMetadata();
    void testVariableFrameRateHandling();
    
    // Regression tests for existing bugs
    void testMetadataMissingBugReproduction();
    void testProcessingFailureDueToMissingMetadata();

private:
    void verifyMetadataInCommand(const QString &command, const MediaInfo &mediaInfo);
    void createTestFileWithMetadata(const QString &filename, const MediaInfo &info);
    QString extractMetadataFromCommand(const QString &command, const QString &parameter);
    
    TestHelper *m_helper;
    FileProcessor *m_processor;
    MediaAnalyzer *m_analyzer;
    MainWindow *m_mainWindow;
    QString m_testVideoFile;
    QString m_testH264File;
    QString m_testH265File;
};

#endif // TESTFFMPEGCOMMANDGENERATION_H