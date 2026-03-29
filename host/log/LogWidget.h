#pragma once

#include <QWidget>
#include <QTextCharFormat>
#include <memory>
#include <mutex>
#include <vector>

QT_BEGIN_NAMESPACE
class QTextEdit;
class QLineEdit;
class QPushButton;
class QCheckBox;
class QComboBox;
class QLabel;
QT_END_NAMESPACE

struct LogEntry
{
    qint64 timestamp;
    int level; // 0=Error, 1=Warning, 2=Info, 3=Debug, 4=Trace, 5=Audit
    QString bundleName;
    QString message;
    QString fullText;
};

class LogWidget : public QWidget
{
    Q_OBJECT

public:
    explicit LogWidget(QWidget* parent = nullptr);
    ~LogWidget() override;

    // 添加日志条目
    void addLog(int level, const QString& bundleName, const QString& message);
    
    // 清空日志
    void clearLog();
    
    // 获取日志内容（用于导出）
    QString getAllLogText() const;
    
    // 设置最大日志行数
    void setMaxLogLines(int maxLines);

signals:
    void logEntryAdded(const LogEntry& entry);

private slots:
    void onSearchTextChanged(const QString& text);
    void onSearchNext();
    void onSearchPrevious();
    void onClearLog();
    void onFilterChanged();
    void onAutoScrollToggled(bool checked);

private:
    void setupUI();
    void setupConnections();
    void appendToDisplay(const LogEntry& entry);
    void refreshDisplay();
    QTextCharFormat getFormatForLevel(int level) const;
    QString levelToString(int level) const;
    QColor levelToColor(int level) const;
    bool entryMatchesFilter(const LogEntry& entry) const;
    void highlightSearchResults();

    // UI elements
    QTextEdit* m_logDisplay = nullptr;
    QLineEdit* m_searchEdit = nullptr;
    QPushButton* m_searchNextBtn = nullptr;
    QPushButton* m_searchPrevBtn = nullptr;
    QPushButton* m_clearBtn = nullptr;
    QCheckBox* m_autoScrollCheck = nullptr;
    QCheckBox* m_showErrorCheck = nullptr;
    QCheckBox* m_showWarningCheck = nullptr;
    QCheckBox* m_showInfoCheck = nullptr;
    QCheckBox* m_showDebugCheck = nullptr;
    QLabel* m_statusLabel = nullptr;

    // Data
    std::vector<LogEntry> m_logEntries;
    mutable std::mutex m_logMutex;
    int m_maxLogLines = 10000;
    int m_currentSearchIndex = -1;
    QString m_currentSearchText;
    
    // Format cache
    QTextCharFormat m_errorFormat;
    QTextCharFormat m_warningFormat;
    QTextCharFormat m_infoFormat;
    QTextCharFormat m_debugFormat;
    QTextCharFormat m_traceFormat;
    QTextCharFormat m_auditFormat;
};
