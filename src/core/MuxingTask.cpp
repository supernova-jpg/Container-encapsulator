#include "MuxingTask.h"
#include <QRegularExpression>
#include <QDebug>
#include <QDir>

MuxingTask::MuxingTask(QObject *parent)
    : QObject(parent)
    , m_process(nullptr)
    , m_progressTimer(new QTimer(this))
    , m_totalDuration(0)
    , m_currentTime(0)
    , m_durationParsed(false)
{
    m_progressTimer->setSingleShot(false);
    m_progressTimer->setInterval(1000); // Update every second
    connect(m_progressTimer, &QTimer::timeout, this, &MuxingTask::checkProgress);
}

MuxingTask::~MuxingTask()
{
    if (m_process) {
        m_process->kill();
        m_process->waitForFinished(3000);
        m_process->deleteLater();
    }
}

void MuxingTask::setFiles(const QString &inputFile, const QString &outputFile)
{
    m_inputFile = inputFile;
    m_outputFile = outputFile;
}

void MuxingTask::setCommandAndArgs(const QString &program, const QStringList &args)
{
    m_program = program;
    m_arguments = args;
}

void MuxingTask::start()
{
    if (!m_process) {
        m_process = new QProcess(this);
        connect(m_process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
                this, &MuxingTask::onProcessFinished);
        connect(m_process, &QProcess::errorOccurred, this, &MuxingTask::onProcessError);
        connect(m_process, &QProcess::readyReadStandardError, this, &MuxingTask::onProcessReadyRead);
        connect(m_process, &QProcess::readyReadStandardOutput, this, &MuxingTask::onProcessReadyRead);
    }

    if (m_program.isEmpty()) {
        emit finished(false, "No FFmpeg program path specified");
        return;
    }

    m_accumulatedOutput.clear();
    m_totalDuration = 0;
    m_currentTime = 0;
    m_durationParsed = false;


    emit logMessage(QString("Starting FFmpeg: \"%1\" %2").arg(m_program).arg(m_arguments.join(" ")));

    m_elapsedTimer.start();

    // Set working directory to handle relative paths better
    QFileInfo inputInfo(m_inputFile);
    if (inputInfo.exists()) {
        m_process->setWorkingDirectory(inputInfo.absolutePath());
    }
    
    m_process->start(m_program, m_arguments);

    if (!m_process->waitForStarted(5000)) {
        QString errorMsg = QString("Failed to start FFmpeg: %1").arg(m_process->errorString());
        emit logMessage(QString("[ERROR] %1").arg(errorMsg));
        emit finished(false, errorMsg);
        return;
    }

    m_progressTimer->start();
}

void MuxingTask::stop()
{
    if (m_process && m_process->state() == QProcess::Running) {
        emit logMessage("Stopping FFmpeg process...");
        m_process->terminate();
        
        // Give it a chance to terminate gracefully
        if (!m_process->waitForFinished(3000)) {
            m_process->kill();
            m_process->waitForFinished(1000);
        }
    }
    
    m_progressTimer->stop();
}

bool MuxingTask::isRunning() const
{
    return m_process && m_process->state() == QProcess::Running;
}

void MuxingTask::onProcessFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
    m_progressTimer->stop();
    
    QString message;
    bool success = (exitCode == 0 && exitStatus == QProcess::NormalExit);
    
    if (success) {
        message = QString("FFmpeg completed successfully in %1 seconds")
                     .arg(m_elapsedTimer.elapsed() / 1000.0, 0, 'f', 1);
        emit progress(100); // Set to 100% on success
    } else {
        if (exitStatus == QProcess::CrashExit) {
            message = "FFmpeg process crashed";
        } else {
            message = QString("FFmpeg failed with exit code %1").arg(exitCode);
        }
        
        // Include the last few lines of output for debugging
        QStringList outputLines = m_accumulatedOutput.split('\n');
        QString lastOutput;
        // Get the last non-empty lines for better debugging
        for (int i = outputLines.size() - 1; i >= 0 && lastOutput.isEmpty(); i--) {
            lastOutput = outputLines.at(i).trimmed();
        }
        if (!lastOutput.isEmpty()) {
            message += "\nLast output: " + lastOutput;
        } else {
            // If no output at all, this might indicate immediate crash
            message += "\nLast output: (no output captured - process may have crashed immediately)";
        }
    }
    
    // Use appropriate log level based on success/failure
    if (success) {
        emit logMessage(message); // Success messages remain as default (INFO)
    } else {
        emit logMessage(QString("[ERROR] %1").arg(message)); // Failures should be ERROR level
    }
    emit finished(success, message);
}

void MuxingTask::onProcessError(QProcess::ProcessError error)
{
    m_progressTimer->stop();
    
    QString errorString;
    switch (error) {
    case QProcess::FailedToStart:
        errorString = "Failed to start FFmpeg process";
        break;
    case QProcess::Crashed:
        errorString = "FFmpeg process crashed";
        break;
    case QProcess::Timedout:
        errorString = "FFmpeg process timed out";
        break;
    case QProcess::WriteError:
        errorString = "Write error to FFmpeg process";
        break;
    case QProcess::ReadError:
        errorString = "Read error from FFmpeg process";
        break;
    case QProcess::UnknownError:
    default:
        errorString = "Unknown FFmpeg process error";
        break;
    }
    
    emit logMessage(QString("[ERROR] Process error: %1").arg(errorString));
    emit finished(false, errorString);
}

void MuxingTask::onProcessReadyRead()
{
    if (!m_process) return;
    
    // Read both stderr and stdout
    QByteArray stderrData = m_process->readAllStandardError();
    QByteArray stdoutData = m_process->readAllStandardOutput();
    
    // Convert and accumulate output
    QString stderrOutput = QString::fromLocal8Bit(stderrData);
    QString stdoutOutput = QString::fromLocal8Bit(stdoutData);
    
    if (!stderrOutput.isEmpty()) {
        m_accumulatedOutput += stderrOutput;
        parseFFmpegOutput(stderrOutput);
    }
    
    if (!stdoutOutput.isEmpty()) {
        m_accumulatedOutput += stdoutOutput;
        parseFFmpegOutput(stdoutOutput);
    }
}

void MuxingTask::checkProgress()
{
    // This is called periodically to update progress even if no new output is available
    if (m_totalDuration > 0 && m_currentTime > 0) {
        int percentage = qMin(99, (int)((m_currentTime * 100) / m_totalDuration));
        emit progress(percentage);
    }
}

void MuxingTask::parseFFmpegOutput(const QString &output)
{
    // Parse duration (appears early in the output)
    if (!m_durationParsed) {
        QRegularExpression durationRegex(R"(Duration:\s*(\d+):(\d+):(\d+)\.(\d+))");
        QRegularExpressionMatch durationMatch = durationRegex.match(output);
        if (durationMatch.hasMatch()) {
            int hours = durationMatch.captured(1).toInt();
            int minutes = durationMatch.captured(2).toInt();
            int seconds = durationMatch.captured(3).toInt();
            int milliseconds = durationMatch.captured(4).toInt();
            
            m_totalDuration = hours * 3600 + minutes * 60 + seconds;
            m_totalDuration = m_totalDuration * 1000 + milliseconds; // Convert to milliseconds
            m_durationParsed = true;
            
            emit logMessage(QString("Input duration: %1").arg(formatDuration(m_totalDuration / 1000)));
        }
    }
    
    // Parse current time (appears in progress lines)
    QRegularExpression timeRegex(R"(time=(\d+):(\d+):(\d+)\.(\d+))");
    QRegularExpressionMatchIterator timeIterator = timeRegex.globalMatch(output);
    
    while (timeIterator.hasNext()) {
        QRegularExpressionMatch timeMatch = timeIterator.next();
        int hours = timeMatch.captured(1).toInt();
        int minutes = timeMatch.captured(2).toInt();
        int seconds = timeMatch.captured(3).toInt();
        int milliseconds = timeMatch.captured(4).toInt();
        
        m_currentTime = hours * 3600 + minutes * 60 + seconds;
        m_currentTime = m_currentTime * 1000 + milliseconds; // Convert to milliseconds
    }
    
    // Look for error messages
    if (output.contains("Error", Qt::CaseInsensitive) || 
        output.contains("Invalid", Qt::CaseInsensitive) ||
        output.contains("Cannot", Qt::CaseInsensitive)) {
        // Don't emit error immediately, wait for process to finish
        // This prevents false positives on warnings
    }
}

QString MuxingTask::formatDuration(qint64 seconds)
{
    qint64 hours = seconds / 3600;
    qint64 minutes = (seconds % 3600) / 60;
    qint64 secs = seconds % 60;
    
    if (hours > 0) {
        return QString("%1:%2:%3").arg(hours).arg(minutes, 2, 10, QChar('0')).arg(secs, 2, 10, QChar('0'));
    } else {
        return QString("%1:%2").arg(minutes).arg(secs, 2, 10, QChar('0'));
    }
}
