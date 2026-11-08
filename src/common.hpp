#pragma once

#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <set>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <ctime>
#include <cmath>
#include <algorithm>
#include <functional>

#include <sqlite3.h>

#include <QApplication>
#include <QStyleFactory>
#include <QScreen>
#include <QCursor>
#include <QMainWindow>
#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QSplitter>
#include <QTabWidget>
#include <QPushButton>
#include <QLabel>
#include <QLineEdit>
#include <QPlainTextEdit>
#include <QTextEdit>
#include <QTreeWidget>
#include <QTreeWidgetItem>
#include <QMenu>
#include <QAction>
#include <QScrollArea>
#include <QFrame>
#include <QProgressBar>
#include <QDialog>
#include <QInputDialog>
#include <QFileDialog>
#include <QMessageBox>
#include <QColorDialog>
#include <QSpinBox>
#include <QSlider>
#include <QComboBox>
#include <QCheckBox>
#include <QKeySequenceEdit>
#include <QShortcut>
#include <QTimer>
#include <QClipboard>
#include <QMimeData>
#include <QUrl>
#include <QDesktopServices>
#include <QImage>
#include <QBuffer>
#include <QCryptographicHash>
#include <QPalette>
#include <QColor>
#include <QFont>
#include <QFontMetrics>
#include <QPainter>
#include <QPainterPath>
#include <QLinearGradient>
#include <QRadialGradient>
#include <QPen>
#include <QBrush>
#include <QTextCursor>
#include <QTextCharFormat>
#include <QTextBlockFormat>
#include <QTextImageFormat>
#include <QTextBlock>
#include <QTextFragment>
#include <QDataStream>
#include <QByteArray>
#include <QFileSystemWatcher>
#include <QStackedWidget>
#include <QGraphicsDropShadowEffect>
#include <QVariantAnimation>
#include <QResizeEvent>
#include <QFocusEvent>
#include <QEvent>
#include <QFileInfo>
#include <QTextDocument>
#include <QRegularExpression>
#include <QStringList>

namespace fs = std::filesystem;

// ============================================================================
// КРОССПЛАТФОРМЕННЫЕ ХЕЛПЕРЫ ПУТЕЙ И ШРИФТОВ (Windows 11 UTF-8 / Linux)
// ============================================================================
#ifdef _WIN32
inline const char* MONO_FONT_FAMILY = "Consolas";
#else
inline const char* MONO_FONT_FAMILY = "Monospace";
#endif

inline fs::path u8path(const std::string& utf8Str) {
    return fs::u8path(utf8Str);
}

inline std::string pathToUtf8(const fs::path& p) {
    return p.generic_u8string();
}

inline std::string filenameToUtf8(const fs::path& p) {
    return p.filename().u8string();
}

inline std::string stemToUtf8(const fs::path& p) {
    return p.stem().u8string();
}

// Пользовательские свойства QTextCharFormat для точного двустороннего переноса тегов Obsidian
enum ObsidianFormatProperty {
    PropRawMarkdownToken = QTextFormat::UserProperty + 101,
    PropAttachmentRef    = QTextFormat::UserProperty + 102,
    PropImageOrigRef     = QTextFormat::UserProperty + 103,
    PropImageCustomSize  = QTextFormat::UserProperty + 104
};

// ============================================================================
// 1. НАСТРОЙКИ (Включая отключение поля кода по умолчанию и размеры панелей)
// ============================================================================
struct AppConfig {
    bool isMaleTheme       = true;
    int  uiFontSize        = 10;
    int  cardScalePercent  = 85;
    int  cardOpacity       = 220;
    bool enableBlurGlass   = true;
    bool enableShadows     = true;
    int  shadowBlurRadius  = 28;
    bool enableHoverFloat  = true;
    bool useRightDrawer    = true;
    int  leftPanelWidth    = 315;
    int  rightDrawerWidth  = 480;
    bool showTopSectionBar = false; // По умолчанию панель разделов над поиском выключена
    bool showCodeField     = false; // По умолчанию отдельное поле «Код / команда» выключено

    int  trackerCardsForFull = 100;

    QColor bgWindowTop     = QColor("#0c142a");
    QColor bgWindowBottom  = QColor("#14264a");
    QColor bgCardNormal    = QColor("#1a2a4c");
    QColor bgCardPinned    = QColor("#2a2252");
    QColor bgInputField    = QColor("#1e2d4e");
    QColor bgLeftPanel     = QColor("#0e1830");
    QColor bgRightPanel    = QColor("#101a34");
    QColor bgButton        = QColor("#182644");
    QColor bgTrackerCard   = QColor("#121e3a");

    QString hkNewCard      = "Ctrl+N";
    QString hkToggleVault  = "Ctrl+B";
    QString hkToggleDrawer = "Ctrl+D";
    QString hkFormatPopup  = "Ctrl+E";
    QString hkTracker      = "Ctrl+T";
    QString hkSettings     = "Ctrl+,";
    QString hkFocusSearch  = "Ctrl+F";
};

inline AppConfig g_cfg;
inline QTextEdit* g_activeRichEditor = nullptr;
inline bool g_ignoreFsWatcher = false;
inline int  g_ignoreFsWatcherDepth = 0;
inline fs::path g_vaultRoot = u8path("./vault");

inline void beginIgnoreFsWatcher(int durationMs = 900) {
    g_ignoreFsWatcher = true;
    ++g_ignoreFsWatcherDepth;
    QTimer::singleShot(durationMs, []() {
        if (--g_ignoreFsWatcherDepth <= 0) {
            g_ignoreFsWatcherDepth = 0;
            g_ignoreFsWatcher = false;
        }
    });
}

struct NamedColor {
    std::string label;
    QColor color;
};

inline std::vector<NamedColor> g_customPalette;
inline std::function<void()> g_onSaveCustomPaletteToDB;

inline const std::vector<NamedColor> DEFAULT_FG_COLORS = {
    {"Неоновый голубой",    QColor("#38bdf8")},
    {"Кибер-розовый",       QColor("#f472b6")},
    {"Изумрудный неон",     QColor("#4ade80")},
    {"Солнечный янтарь",    QColor("#fbbf24")},
    {"Аметистовый",         QColor("#c084fc")},
    {"Бирюзовый бриз",      QColor("#2dd4bf")},
    {"Коралловый красный",  QColor("#fb7185")},
    {"Электрический синий", QColor("#60a5fa")},
    {"Лаймовый фреш",       QColor("#a3e635")},
    {"Огненный оранжевый",  QColor("#fb923c")}
};

inline const std::vector<NamedColor> DEFAULT_BG_COLORS = {
    {"Малиновый штрих",   QColor("#9d174d")},
    {"Глубокий сапфир",   QColor("#1e40af")},
    {"Темный изумруд",    QColor("#065f46")},
    {"Пурпурный неон",    QColor("#5b21b6")},
    {"Янтарный маркер",   QColor("#92400e")},
    {"Морская волна",     QColor("#115e59")},
    {"Бордовый рубин",    QColor("#881337")},
    {"Ночной индиго",     QColor("#3730a3")},
    {"Оливковый лес",     QColor("#3f6212")},
    {"Графитовый сланец", QColor("#334155")}
};

inline QColor getReadableTextColorForBg(const QColor& bg) {
    double luminance = (0.299 * bg.red() + 0.587 * bg.green() + 0.114 * bg.blue()) / 255.0;
    return (luminance > 0.62) ? QColor(10, 15, 28) : QColor(248, 250, 252);
}

inline QColor getModernColorForName(const std::string& name) {
    if (name.empty()) return QColor("#38bdf8");
    unsigned int hash = 2166136261u;
    for (unsigned char c : name) {
        hash ^= c;
        hash *= 16777619u;
    }
    return DEFAULT_FG_COLORS[hash % DEFAULT_FG_COLORS.size()].color;
}

// ============================================================================
// 2. ПУСТОЙ СПИСОК ПО УМОЛЧАНИЮ
// ============================================================================
struct SkillboxCourse {
    std::string courseName;
    std::vector<std::string> topics;
};

inline const std::vector<SkillboxCourse> SKILLBOX_CURRICULUM = {};

// ============================================================================
// 3. ПОЛОСА ПРОГРЕССА СО СКРУГЛЕНИЕМ 30PX (RoundedProgressBar)
// ============================================================================
class RoundedProgressBar : public QWidget {
private:
    int minVal = 0;
    int maxVal = 100;
    int curVal = 0;
    QColor barColor = QColor("#38bdf8");

public:
    RoundedProgressBar(QWidget* parent = nullptr) : QWidget(parent) {
        setMinimumHeight(32);
        setMaximumHeight(34);
        setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    }

    void setRange(int minimum, int maximum) {
        minVal = minimum;
        maxVal = std::max(minimum + 1, maximum);
        update();
    }

    void setValue(int val) {
        curVal = std::clamp(val, minVal, maxVal);
        update();
    }

    void setBarColor(const QColor& c) {
        barColor = c;
        update();
    }

protected:
    void paintEvent(QPaintEvent*) override {
        QPainter p(this);
        p.setRenderHint(QPainter::Antialiasing, true);

        QRectF trackRect = QRectF(rect()).adjusted(2.0, 2.0, -2.0, -2.0);
        qreal radius = 30.0;

        QPainterPath trackPath;
        trackPath.addRoundedRect(trackRect, radius, radius);
        p.fillPath(trackPath, QColor(8, 13, 26, 230));

        double ratio = static_cast<double>(curVal - minVal) / static_cast<double>(maxVal - minVal);
        if (ratio > 0.0) {
            double fillW = std::max(trackRect.height(), trackRect.width() * ratio);
            fillW = std::min(trackRect.width(), fillW);
            QRectF fillRect(trackRect.left(), trackRect.top(), fillW, trackRect.height());

            QPainterPath fillPath;
            fillPath.addRoundedRect(fillRect, radius, radius);

            QLinearGradient grad(fillRect.topLeft(), fillRect.topRight());
            grad.setColorAt(0.0, barColor.darker(115));
            grad.setColorAt(0.5, barColor);
            grad.setColorAt(1.0, barColor.lighter(125));
            p.fillPath(fillPath, grad);

            QLinearGradient shine(fillRect.topLeft(), QPointF(fillRect.left(), fillRect.top() + fillRect.height() * 0.5));
            shine.setColorAt(0.0, QColor(255, 255, 255, 55));
            shine.setColorAt(1.0, QColor(255, 255, 255, 0));
            p.fillPath(fillPath, shine);
        }

        QColor borderCol = barColor;
        borderCol.setAlpha(150);
        p.setPen(QPen(borderCol, 1.5));
        p.drawPath(trackPath);

        QFont f = font();
        f.setPointSize(g_cfg.uiFontSize);
        f.setBold(true);
        p.setFont(f);
        p.setPen(Qt::white);
        double pct = ratio * 100.0;
        QString txt = (std::fabs(pct - std::round(pct)) < 0.05)
        ? QString::number(static_cast<int>(std::round(pct)))
        : QString::number(pct, 'f', 1);
        p.drawText(trackRect, Qt::AlignCenter, txt + "%");
    }
};

// ============================================================================
// 4. C++ РАМКА ПОЛЕЙ ВВОДА БЕЗ РЕКУРСИИ В PAINTEVENT (NeonInputFrame)
// ============================================================================
class NeonInputFrame : public QFrame {
private:
    QColor borderColor;
    QWidget* innerWidget = nullptr;

public:
    NeonInputFrame(QWidget* childInput, const QColor& borderCol = QColor("#38bdf8"), QWidget* parent = nullptr)
    : QFrame(parent), borderColor(borderCol), innerWidget(childInput)
    {
        QVBoxLayout* l = new QVBoxLayout(this);
        l->setContentsMargins(10, 6, 10, 6);
        l->setSpacing(0);

        childInput->setAutoFillBackground(false);
        if (auto* absScroll = qobject_cast<QAbstractScrollArea*>(childInput)) {
            if (absScroll->viewport()) {
                absScroll->viewport()->setAutoFillBackground(false);
            }
        }

        if (auto* le = qobject_cast<QLineEdit*>(childInput)) {
            le->setFrame(false);
            QPalette pal = le->palette();
            pal.setColor(QPalette::Base, Qt::transparent);
            pal.setColor(QPalette::Text, QColor(248, 250, 252));
            le->setPalette(pal);
        } else if (auto* te = qobject_cast<QTextEdit*>(childInput)) {
            te->setFrameShape(QFrame::NoFrame);
            te->document()->setDocumentMargin(4);
        } else if (auto* pte = qobject_cast<QPlainTextEdit*>(childInput)) {
            pte->setFrameShape(QFrame::NoFrame);
            pte->document()->setDocumentMargin(4);
        } else if (auto* tw = qobject_cast<QTreeWidget*>(childInput)) {
            tw->setFrameShape(QFrame::NoFrame);
        }

        l->addWidget(childInput);
    }

protected:
    void paintEvent(QPaintEvent*) override {
        QPainter p(this);
        p.setRenderHint(QPainter::Antialiasing, true);

        QRectF r = QRectF(rect()).adjusted(1.0, 1.0, -1.0, -1.0);
        p.setBrush(g_cfg.bgInputField);
        QColor bc = g_cfg.isMaleTheme ? borderColor : QColor("#f472b6");
        bc.setAlpha(190);
        p.setPen(QPen(bc, 1.6));
        p.drawRoundedRect(r, 8, 8);
    }
};

// ============================================================================
// 5. НЕОНОВАЯ КНОПКА С ФОНОМ ИЗ НАСТРОЕК И ПЕРЕНОСОМ СЛОВ (NeonButton)
// ============================================================================
class NeonButton : public QPushButton {
private:
    QColor accentColor;
    bool hovered = false;

public:
    NeonButton(const QString& text, const QColor& neonColor = QColor("#38bdf8"), QWidget* parent = nullptr)
    : QPushButton(text, parent), accentColor(neonColor)
    {
        setCursor(Qt::PointingHandCursor);
        setMinimumHeight(32);
        setAttribute(Qt::WA_Hover, true);
        setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Minimum);

        QGraphicsDropShadowEffect* glow = new QGraphicsDropShadowEffect(this);
        glow->setBlurRadius(14);
        glow->setOffset(0, 2);
        QColor shadowCol = accentColor;
        shadowCol.setAlpha(90);
        glow->setColor(shadowCol);
        setGraphicsEffect(glow);
    }

    QSize sizeHint() const override {
        QFont f = font();
        f.setPointSize(g_cfg.uiFontSize);
        f.setBold(true);
        QFontMetrics fm(f);
        int maxW = maximumWidth() < 2000 ? (maximumWidth() - 20) : 260;
        QRect br = fm.boundingRect(QRect(0, 0, std::max(36, maxW), 500), Qt::AlignCenter | Qt::TextWordWrap, text());
        return QSize(br.width() + 24, std::max(32, br.height() + 12));
    }

protected:
    void enterEvent(QEvent* e) override {
        QPushButton::enterEvent(e);
        hovered = true;
        if (auto* eff = qobject_cast<QGraphicsDropShadowEffect*>(graphicsEffect())) {
            QColor sc = accentColor;
            sc.setAlpha(175);
            eff->setColor(sc);
            eff->setBlurRadius(20);
        }
        update();
    }

    void leaveEvent(QEvent* e) override {
        QPushButton::leaveEvent(e);
        hovered = false;
        if (auto* eff = qobject_cast<QGraphicsDropShadowEffect*>(graphicsEffect())) {
            QColor sc = accentColor;
            sc.setAlpha(90);
            eff->setColor(sc);
            eff->setBlurRadius(14);
        }
        update();
    }

    void paintEvent(QPaintEvent*) override {
        QPainter p(this);
        p.setRenderHint(QPainter::Antialiasing, true);

        QRectF r = QRectF(rect()).adjusted(1.5, 1.5, -1.5, -1.5);

        QLinearGradient bg(r.topLeft(), r.bottomRight());
        if (hovered) {
            QColor c1 = accentColor.darker(210);
            c1.setAlpha(235);
            bg.setColorAt(0.0, c1);
            bg.setColorAt(1.0, g_cfg.bgButton.lighter(120));
        } else {
            bg.setColorAt(0.0, g_cfg.bgButton);
            bg.setColorAt(1.0, g_cfg.bgButton.darker(130));
        }

        p.setBrush(bg);
        QColor borderCol = accentColor;
        borderCol.setAlpha(hovered ? 245 : 165);
        p.setPen(QPen(borderCol, hovered ? 1.8 : 1.3));
        p.drawRoundedRect(r, 8, 8);

        p.setPen(hovered ? Qt::white : QColor(235, 242, 255));
        QFont f = font();
        f.setPointSize(g_cfg.uiFontSize);
        f.setBold(true);
        p.setFont(f);

        QRectF textRect = r.adjusted(8.0, 4.0, -8.0, -4.0);
        p.drawText(textRect, Qt::AlignCenter | Qt::TextWordWrap, text());
    }
};

enum class PickerKind {
    StrokeBg,
    TextFg,
    Underline
};

class ColorPickerButton : public QPushButton {
private:
    PickerKind kind;
    QColor currentColor;
    bool hovered = false;
    std::function<void(QColor)> onColorPicked;

public:
    ColorPickerButton(PickerKind k, const QColor& initialColor, std::function<void(QColor)> callback, QWidget* parent = nullptr)
    : QPushButton(parent), kind(k), currentColor(initialColor), onColorPicked(callback)
    {
        setCursor(Qt::PointingHandCursor);
        setAttribute(Qt::WA_Hover, true);
        setFocusPolicy(Qt::NoFocus);

        if (kind == PickerKind::StrokeBg) {
            setFixedSize(92, 32);
            setToolTip("Цвет штриха (фона)");
        } else if (kind == PickerKind::TextFg) {
            setFixedSize(44, 32);
            setToolTip("Цвет текста (А)");
        } else {
            setFixedSize(44, 32);
            setToolTip("Подчеркивание и цвет линии (U)");
        }

        QGraphicsDropShadowEffect* glow = new QGraphicsDropShadowEffect(this);
        glow->setBlurRadius(15);
        glow->setOffset(0, 2);
        QColor sc = currentColor;
        sc.setAlpha(110);
        glow->setColor(sc);
        setGraphicsEffect(glow);

        connect(this, &QPushButton::clicked, [this]() { openPalettePopup(); });
    }

    void setColor(const QColor& c) {
        currentColor = c;
        if (auto* eff = qobject_cast<QGraphicsDropShadowEffect*>(graphicsEffect())) {
            QColor sc = currentColor;
            sc.setAlpha(130);
            eff->setColor(sc);
        }
        update();
    }

private:
    void openPalettePopup() {
        QDialog popup(this, Qt::Popup | Qt::FramelessWindowHint);
        popup.resize(400, 450);

        QVBoxLayout* root = new QVBoxLayout(&popup);
        root->setContentsMargins(16, 16, 16, 16);
        root->setSpacing(10);

        QFont boldF = font();
        boldF.setBold(true);

        QString titleStr = (kind == PickerKind::StrokeBg) ? "🎨 Палитра штриха (10 цветов)"
        : (kind == PickerKind::TextFg)   ? "🎨 Палитра текста «А» (10 цветов)"
        : "🎨 Палитра линии подчеркивания";
        QLabel* header = new QLabel(titleStr, &popup);
        header->setFont(boldF);
        root->addWidget(header);

        const auto& defaults = (kind == PickerKind::StrokeBg) ? DEFAULT_BG_COLORS : DEFAULT_FG_COLORS;
        QGridLayout* defGrid = new QGridLayout();
        defGrid->setSpacing(6);

        for (size_t i = 0; i < defaults.size(); ++i) {
            const auto& item = defaults[i];
            QPushButton* swBtn = new QPushButton(QString::fromStdString(item.label), &popup);
            swBtn->setCursor(Qt::PointingHandCursor);
            swBtn->setMinimumHeight(30);

            QPalette p = swBtn->palette();
            p.setColor(QPalette::Button, item.color);
            p.setColor(QPalette::ButtonText, getReadableTextColorForBg(item.color));
            swBtn->setPalette(p);
            swBtn->setAutoFillBackground(true);

            connect(swBtn, &QPushButton::clicked, [this, &popup, item]() {
                setColor(item.color);
                if (onColorPicked) onColorPicked(item.color);
                popup.accept();
            });

            defGrid->addWidget(swBtn, static_cast<int>(i) / 2, static_cast<int>(i) % 2);
        }
        root->addLayout(defGrid);

        QLabel* customLbl = new QLabel("Мои сохраненные цвета (с подписью):", &popup);
        customLbl->setFont(boldF);
        root->addWidget(customLbl);

        QScrollArea* customScroll = new QScrollArea(&popup);
        customScroll->setWidgetResizable(true);
        customScroll->setMaximumHeight(120);
        QWidget* customContainer = new QWidget(&popup);
        QVBoxLayout* customListLayout = new QVBoxLayout(customContainer);
        customListLayout->setContentsMargins(6, 6, 6, 6);
        customListLayout->setSpacing(4);
        customScroll->setWidget(customContainer);
        root->addWidget(customScroll);

        std::function<void()> rebuildCustomList = [&]() {
            QLayoutItem* ch;
            while ((ch = customListLayout->takeAt(0)) != nullptr) {
                if (ch->widget()) delete ch->widget();
                delete ch;
            }
            if (g_customPalette.empty()) {
                customListLayout->addWidget(new QLabel("Нет сохраненных цветов. Добавьте ниже!", customContainer));
            } else {
                for (size_t idx = 0; idx < g_customPalette.size(); ++idx) {
                    const auto& nc = g_customPalette[idx];
                    QHBoxLayout* row = new QHBoxLayout();
                    QPushButton* pickBtn = new QPushButton(QString::fromStdString(nc.label + " (" + nc.color.name().toStdString() + ")"), customContainer);
                    QPalette pal = pickBtn->palette();
                    pal.setColor(QPalette::Button, nc.color);
                    pal.setColor(QPalette::ButtonText, getReadableTextColorForBg(nc.color));
                    pickBtn->setPalette(pal);

                    QPushButton* delBtn = new QPushButton("×", customContainer);
                    delBtn->setFixedWidth(28);

                    connect(pickBtn, &QPushButton::clicked, [this, &popup, nc]() {
                        setColor(nc.color);
                        if (onColorPicked) onColorPicked(nc.color);
                        popup.accept();
                    });
                    connect(delBtn, &QPushButton::clicked, [&, idx]() {
                        g_customPalette.erase(g_customPalette.begin() + idx);
                        if (g_onSaveCustomPaletteToDB) g_onSaveCustomPaletteToDB();
                        rebuildCustomList();
                    });

                    row->addWidget(pickBtn, 1);
                    row->addWidget(delBtn);
                    QWidget* rowW = new QWidget(customContainer);
                    rowW->setLayout(row);
                    customListLayout->addWidget(rowW);
                }
            }
        };
        rebuildCustomList();

        QHBoxLayout* customActionsRow = new QHBoxLayout();
        NeonButton* pickSpectrumBtn = new NeonButton("🌈 Свой цвет...", QColor("#38bdf8"), &popup);
        NeonButton* addNamedColorBtn = new NeonButton("＋ В палитру с подписью", QColor("#4ade80"), &popup);
        customActionsRow->addWidget(pickSpectrumBtn);
        customActionsRow->addWidget(addNamedColorBtn);
        root->addLayout(customActionsRow);

        QHBoxLayout* filePaletteRow = new QHBoxLayout();
        NeonButton* savePalFileBtn = new NeonButton("💾 Сохранить палитру", QColor("#a78bfa"), &popup);
        NeonButton* loadPalFileBtn = new NeonButton("📂 Загрузить палитру", QColor("#f472b6"), &popup);
        filePaletteRow->addWidget(savePalFileBtn);
        filePaletteRow->addWidget(loadPalFileBtn);
        root->addLayout(filePaletteRow);

        connect(pickSpectrumBtn, &QPushButton::clicked, [this, &popup]() {
            QColor c = QColorDialog::getColor(currentColor, this, "Выберите произвольный цвет");
            if (c.isValid()) {
                setColor(c);
                if (onColorPicked) onColorPicked(c);
                popup.accept();
            }
        });

        connect(addNamedColorBtn, &QPushButton::clicked, [this, &popup, &rebuildCustomList]() {
            QColor c = QColorDialog::getColor(currentColor, this, "1/2: Выберите цвет для сохранения");
            if (!c.isValid()) return;
            bool ok = false;
            QString label = QInputDialog::getText(&popup, "2/2: Подпись цвета",
                                                  "Введите понятную подпись для этого цвета:",
                                                  QLineEdit::Normal, "Мой акцент", &ok);
            if (ok && !label.trimmed().isEmpty()) {
                g_customPalette.push_back({label.trimmed().toStdString(), c});
                if (g_onSaveCustomPaletteToDB) g_onSaveCustomPaletteToDB();
                rebuildCustomList();
            }
        });

        connect(savePalFileBtn, &QPushButton::clicked, [&popup]() {
            QString path = QFileDialog::getSaveFileName(&popup, "Сохранить палитру", "my_palette.pal", "Палитра (*.pal *.txt)");
            if (path.isEmpty()) return;
            std::ofstream out(u8path(path.toStdString()));
            for (const auto& nc : g_customPalette) {
                out << nc.color.name().toStdString() << "|" << nc.label << "\n";
            }
            out.close();
            QMessageBox::information(&popup, "Сохранено", "Палитра с подписями сохранена!");
        });

        connect(loadPalFileBtn, &QPushButton::clicked, [&popup, &rebuildCustomList]() {
            QString path = QFileDialog::getOpenFileName(&popup, "Загрузить палитру", "", "Палитра (*.pal *.txt)");
            if (path.isEmpty()) return;
            std::ifstream in(u8path(path.toStdString()));
            std::string line;
            while (std::getline(in, line)) {
                auto pos = line.find('|');
                if (pos != std::string::npos) {
                    std::string hex = line.substr(0, pos);
                    std::string lbl = line.substr(pos + 1);
                    QColor col(QString::fromStdString(hex));
                    if (col.isValid() && !lbl.empty()) {
                        g_customPalette.push_back({lbl, col});
                    }
                }
            }
            if (g_onSaveCustomPaletteToDB) g_onSaveCustomPaletteToDB();
            rebuildCustomList();
        });

        popup.move(mapToGlobal(QPoint(0, height() + 4)));
        popup.exec();
    }

protected:
    void enterEvent(QEvent* e) override { QPushButton::enterEvent(e); hovered = true; update(); }
    void leaveEvent(QEvent* e) override { QPushButton::leaveEvent(e); hovered = false; update(); }

    void paintEvent(QPaintEvent*) override {
        QPainter p(this);
        p.setRenderHint(QPainter::Antialiasing, true);

        QRectF r = QRectF(rect()).adjusted(1.5, 1.5, -1.5, -1.5);

        if (kind == PickerKind::StrokeBg) {
            p.setBrush(currentColor);
            p.setPen(QPen(hovered ? Qt::white : currentColor.lighter(150), hovered ? 2.0 : 1.3));
            p.drawRoundedRect(r, 8, 8);

            p.setPen(getReadableTextColorForBg(currentColor));
            QFont f = font();
            f.setPointSize(g_cfg.uiFontSize);
            f.setBold(true);
            p.setFont(f);
            p.drawText(r.adjusted(4, 2, -4, -2), Qt::AlignCenter, "Штрих ▾");
        } else if (kind == PickerKind::TextFg) {
            p.setBrush(g_cfg.bgButton);
            p.setPen(QPen(hovered ? currentColor.lighter(135) : currentColor, hovered ? 2.0 : 1.4));
            p.drawRoundedRect(r, 8, 8);

            QFont f = font();
            f.setPointSize(g_cfg.uiFontSize + 3);
            f.setBold(true);
            p.setFont(f);
            p.setPen(currentColor);
            p.drawText(r, Qt::AlignCenter, "А");
        } else {
            p.setBrush(g_cfg.bgButton);
            p.setPen(QPen(hovered ? currentColor.lighter(135) : currentColor, hovered ? 2.0 : 1.4));
            p.drawRoundedRect(r, 8, 8);

            QFont f = font();
            f.setPointSize(g_cfg.uiFontSize + 1);
            f.setBold(true);
            p.setFont(f);
            p.setPen(QColor(235, 242, 255));
            p.drawText(r.adjusted(0, -2, 0, -4), Qt::AlignCenter, "U");

            QPen linePen(currentColor, 3.2, Qt::SolidLine, Qt::RoundCap);
            p.setPen(linePen);
            p.drawLine(QPointF(r.left() + 8, r.bottom() - 6), QPointF(r.right() - 8, r.bottom() - 6));
        }
    }
};

// ============================================================================
// 6. ХРАНИЛИЩЕ КАРТИНОК И ФАЙЛОВ В ПОТОКЕ ТЕКСТА (С ПРОВЕРКОЙ УДАЛЕНИЯ)
// ============================================================================
namespace ImageStorage {

    inline fs::path attachmentsDir() {
        return g_vaultRoot / u8path("attachments");
    }

    inline void ensureImagesDir() {
        std::error_code ec;
        fs::create_directories(attachmentsDir(), ec);
    }

    inline QString saveQImage(const QImage& img) {
        if (img.isNull()) return QString();
        ensureImagesDir();

        QByteArray rawBytes;
        QBuffer buffer(&rawBytes);
        buffer.open(QIODevice::WriteOnly);
        img.save(&buffer, "PNG");

        QByteArray hash = QCryptographicHash::hash(rawBytes, QCryptographicHash::Md5).toHex();
        fs::path target = attachmentsDir() / u8path("paste_" + hash.toStdString() + ".png");
        QString path = QString::fromStdString(pathToUtf8(target));

        std::error_code ec;
        if (!fs::exists(target, ec)) {
            img.save(path, "PNG");
        }
        return path;
    }

    inline QString copyImageFileToStorage(const QString& sourceFilePath) {
        if (sourceFilePath.isEmpty()) return QString();
        QImage img(sourceFilePath);
        if (img.isNull()) return QString();
        return saveQImage(img);
    }

    inline void insertDeletedFileWarningAtCursor(QTextCursor& cursor, const QString& missingRef, const QString& rawMdToken = QString()) {
        QTextCharFormat errFmt;
        errFmt.setFontPointSize(11);
        errFmt.setFontWeight(QFont::Bold);
        errFmt.setForeground(QColor("#ff4d4f"));
        QString fn = QFileInfo(missingRef).fileName();
        if (fn.isEmpty()) fn = missingRef;
        errFmt.setProperty(PropAttachmentRef, fn);
        errFmt.setProperty(PropRawMarkdownToken, rawMdToken.isEmpty() ? ("![[" + fn + "]]") : rawMdToken);
        cursor.insertText(" [⚠ Файл удалён: " + fn + "] ", errFmt);
    }

    inline void insertImageAtCursor(QTextCursor& cursor,
                                    const QString& imagePath,
                                    int maxWidth = 380,
                                    int customW = 0,
                                    int customH = 0,
                                    const QString& origRef = QString())
    {
        if (imagePath.isEmpty()) {
            insertDeletedFileWarningAtCursor(cursor, origRef.isEmpty() ? "image" : origRef);
            return;
        }
        std::error_code ec;
        if (!fs::exists(u8path(imagePath.toStdString()), ec)) {
            insertDeletedFileWarningAtCursor(cursor, origRef.isEmpty() ? imagePath : origRef);
            return;
        }
        QImage img(imagePath);
        if (img.isNull()) {
            insertDeletedFileWarningAtCursor(cursor, origRef.isEmpty() ? imagePath : origRef);
            return;
        }

        QTextImageFormat imgFmt;
        imgFmt.setName(imagePath);
        if (!origRef.isEmpty()) {
            imgFmt.setProperty(PropImageOrigRef, origRef);
        }
        if (customW > 0 || customH > 0) {
            QString szSpec = (customW > 0 && customH > 0)
            ? QString("%1x%2").arg(customW).arg(customH)
            : QString::number(customW > 0 ? customW : customH);
            imgFmt.setProperty(PropImageCustomSize, szSpec);
        }

        if (customW > 0 && customH > 0) {
            imgFmt.setWidth(customW);
            imgFmt.setHeight(customH);
        } else if (customW > 0) {
            double ratio = static_cast<double>(customW) / std::max(1, img.width());
            imgFmt.setWidth(customW);
            imgFmt.setHeight(std::max(1, static_cast<int>(img.height() * ratio)));
        } else if (img.width() > maxWidth) {
            double ratio = static_cast<double>(maxWidth) / static_cast<double>(img.width());
            imgFmt.setWidth(maxWidth);
            imgFmt.setHeight(static_cast<int>(img.height() * ratio));
        } else {
            imgFmt.setWidth(img.width());
            imgFmt.setHeight(img.height());
        }
        cursor.insertImage(imgFmt);
    }
}

namespace AttachmentStorage {

    inline std::string attachFileToVault(const QString& sourceFilePath) {
        if (sourceFilePath.trimmed().isEmpty()) return std::string();
        ImageStorage::ensureImagesDir();

        fs::path srcPath = u8path(sourceFilePath.toStdString());
        std::error_code ec;
        if (!fs::exists(srcPath, ec)) {
            return filenameToUtf8(srcPath);
        }

        fs::path targetDir = ImageStorage::attachmentsDir();
        fs::path dstPath = targetDir / srcPath.filename();

        std::string srcCanon = pathToUtf8(fs::weakly_canonical(srcPath, ec));
        std::string dstCanon = pathToUtf8(fs::weakly_canonical(dstPath, ec));

        if (srcCanon != dstCanon) {
            beginIgnoreFsWatcher(900);
            fs::copy_file(srcPath, dstPath, fs::copy_options::overwrite_existing, ec);
        }
        return filenameToUtf8(dstPath);
    }

    inline std::string attachFile(const QString& sourceFilePath) {
        return attachFileToVault(sourceFilePath);
    }

    inline QString findInVaultByName(const QString& refIn) {
        QString ref = refIn.trimmed();
        int bar = ref.indexOf('|');
        if (bar >= 0) ref = ref.left(bar).trimmed();
        int hash = ref.indexOf('#');
        if (hash >= 0) ref = ref.left(hash).trimmed();
        ref = QUrl::fromPercentEncoding(ref.toUtf8());
        if (ref.isEmpty()) return QString();

        std::error_code ec;
        fs::path p = u8path(ref.toStdString());
        std::vector<fs::path> directCandidates = {
            p,
            g_vaultRoot / p,
            ImageStorage::attachmentsDir() / p.filename()
        };
        for (const auto& c : directCandidates) {
            if (fs::exists(c, ec) && fs::is_regular_file(c, ec)) {
                return QString::fromStdString(pathToUtf8(c));
            }
            if (p.extension().empty()) {
                fs::path withMd = c;
                withMd.replace_extension(".md");
                if (fs::exists(withMd, ec) && fs::is_regular_file(withMd, ec)) {
                    return QString::fromStdString(pathToUtf8(withMd));
                }
            }
        }

        std::string targetFn = filenameToUtf8(p);
        std::string targetMd = targetFn + ".md";
        if (fs::exists(g_vaultRoot, ec)) {
            for (const auto& e : fs::recursive_directory_iterator(g_vaultRoot, fs::directory_options::skip_permission_denied, ec)) {
                if (!e.is_regular_file(ec)) continue;
                std::string fn = filenameToUtf8(e.path());
                if (fn == targetFn || (p.extension().empty() && fn == targetMd)) {
                    return QString::fromStdString(pathToUtf8(e.path()));
                }
            }
        }
        return QString();
    }

    inline bool isImageExtension(const QString& name) {
        QString lower = name.toLower();
        int hash = lower.indexOf('#');
        if (hash >= 0) lower = lower.left(hash);
        int bar = lower.indexOf('|');
        if (bar >= 0) lower = lower.left(bar);
        return lower.endsWith(".png") || lower.endsWith(".jpg") || lower.endsWith(".jpeg") ||
        lower.endsWith(".webp") || lower.endsWith(".bmp") || lower.endsWith(".gif") ||
        lower.endsWith(".svg");
    }

    inline bool isFileWithExtension(const QString& name) {
        QString clean = name.trimmed();
        int bar = clean.indexOf('|');
        if (bar >= 0) clean = clean.left(bar).trimmed();
        int hash = clean.indexOf('#');
        if (hash >= 0) clean = clean.left(hash).trimmed();
        QString ext = QFileInfo(clean).suffix().toLower();
        return !ext.isEmpty();
    }
}

// ============================================================================
// 6a. КОДЕК ФОРМАТИРОВАННОГО ТЕКСТА RTF3 (С обратной совместимостью RTF2/RTF1)
// ============================================================================
namespace RichTextCodec {

    inline std::string serialize(const QTextDocument* doc) {
        QByteArray bytes;
        QDataStream out(&bytes, QIODevice::WriteOnly);
        out.setVersion(QDataStream::Qt_5_12);

        int blockCount = doc->blockCount();
        out << blockCount;

        for (QTextBlock block = doc->begin(); block.isValid(); block = block.next()) {
            out << static_cast<int>(block.blockFormat().alignment());

            QVector<QTextFragment> fragments;
            for (auto it = block.begin(); !it.atEnd(); ++it) {
                if (it.fragment().isValid()) fragments.push_back(it.fragment());
            }

            out << static_cast<int>(fragments.size());
            for (const QTextFragment& frag : fragments) {
                QTextCharFormat fmt = frag.charFormat();
                bool isImage = fmt.isImageFormat();
                out << isImage;

                if (isImage) {
                    QTextImageFormat imgFmt = fmt.toImageFormat();
                    out << imgFmt.name();
                    out << static_cast<double>(imgFmt.width());
                    out << static_cast<double>(imgFmt.height());
                    out << imgFmt.property(PropImageOrigRef).toString();
                    out << imgFmt.property(PropImageCustomSize).toString();
                } else {
                    out << frag.text();
                    out << static_cast<double>(fmt.fontPointSize() > 0 ? fmt.fontPointSize() : 11.0);
                    out << static_cast<int>(fmt.fontWeight());
                    out << static_cast<bool>(fmt.fontUnderline());
                    out << static_cast<bool>(fmt.fontItalic());
                    out << static_cast<bool>(fmt.fontStrikeOut());
                    out << fmt.fontFamily();
                    out << static_cast<int>(fmt.verticalAlignment());

                    bool hasFg = fmt.hasProperty(QTextFormat::ForegroundBrush);
                    out << hasFg;
                    if (hasFg) out << fmt.foreground().color();

                    bool hasBg = fmt.hasProperty(QTextFormat::BackgroundBrush);
                    out << hasBg;
                    if (hasBg) out << fmt.background().color();

                    bool hasUlColor = fmt.hasProperty(QTextFormat::TextUnderlineColor);
                    out << hasUlColor;
                    if (hasUlColor) out << fmt.underlineColor();

                    bool isAnc = fmt.isAnchor();
                    out << isAnc;
                    if (isAnc) out << fmt.anchorHref();

                    out << fmt.property(PropRawMarkdownToken).toString();
                    out << fmt.property(PropAttachmentRef).toString();
                }
            }
        }
        return "RTF3:" + bytes.toBase64().toStdString();
    }

    inline void deserialize(const std::string& data, QTextEdit* editor, int maxImgWidth = 380) {
        editor->clear();
        if (data.empty()) return;

        bool isRtf3 = (data.rfind("RTF3:", 0) == 0);
        bool isRtf2 = (data.rfind("RTF2:", 0) == 0);
        bool isRtf1 = (data.rfind("RTF1:", 0) == 0);

        if (!isRtf3 && !isRtf2 && !isRtf1) {
            editor->setPlainText(QString::fromStdString(data));
            return;
        }

        QByteArray base64Data = QByteArray::fromStdString(data.substr(5));
        QByteArray bytes = QByteArray::fromBase64(base64Data);
        QDataStream in(&bytes, QIODevice::ReadOnly);
        in.setVersion(QDataStream::Qt_5_12);

        QTextCursor cursor(editor->document());
        int blockCount = 0;
        in >> blockCount;

        for (int b = 0; b < blockCount; ++b) {
            if (b > 0) cursor.insertBlock();

            int alignInt = 0;
            in >> alignInt;
            QTextBlockFormat blockFmt;
            blockFmt.setAlignment(static_cast<Qt::Alignment>(alignInt));
            cursor.setBlockFormat(blockFmt);

            int fragCount = 0;
            in >> fragCount;

            for (int f = 0; f < fragCount; ++f) {
                bool isImage = false;
                if (isRtf2 || isRtf3) in >> isImage;

                if (isImage) {
                    QString imgPath, origRef, customSize;
                    double w = 0, h = 0;
                    in >> imgPath >> w >> h;
                    if (isRtf3) {
                        in >> origRef >> customSize;
                    }
                    int cw = 0, ch = 0;
                    if (!customSize.isEmpty()) {
                        int xPos = customSize.indexOf('x', 0, Qt::CaseInsensitive);
                        if (xPos > 0) {
                            cw = customSize.left(xPos).toInt();
                            ch = customSize.mid(xPos + 1).toInt();
                        } else {
                            cw = customSize.toInt();
                        }
                    }
                    QString checkPath = imgPath;
                    std::error_code ec;
                    if (!fs::exists(u8path(checkPath.toStdString()), ec) && !origRef.isEmpty()) {
                        QString found = AttachmentStorage::findInVaultByName(origRef);
                        if (!found.isEmpty()) checkPath = found;
                    }
                    ImageStorage::insertImageAtCursor(cursor, checkPath, maxImgWidth, cw, ch, origRef);
                } else {
                    QString text, fontFam, anchorHref, rawMdToken, attRef;
                    double ptSize = 11.0;
                    int weight = QFont::Normal;
                    bool underline = false, italic = false, strike = false;
                    int vAlign = 0;
                    bool hasFg = false, hasBg = false, hasUlColor = false, isAnc = false;
                    QColor fgColor, bgColor, ulColor;

                    in >> text >> ptSize >> weight >> underline;
                    if (isRtf3) {
                        in >> italic >> strike >> fontFam >> vAlign;
                    }
                    in >> hasFg;
                    if (hasFg) in >> fgColor;
                    in >> hasBg;
                    if (hasBg) in >> bgColor;
                    in >> hasUlColor;
                    if (hasUlColor) in >> ulColor;
                    if (isRtf3) {
                        in >> isAnc;
                        if (isAnc) in >> anchorHref;
                        in >> rawMdToken >> attRef;
                    }

                    if (!attRef.isEmpty() && AttachmentStorage::isFileWithExtension(attRef)) {
                        QString resolved = AttachmentStorage::findInVaultByName(attRef);
                        if (resolved.isEmpty()) {
                            if (AttachmentStorage::isImageExtension(attRef)) {
                                ImageStorage::insertDeletedFileWarningAtCursor(cursor, attRef, rawMdToken);
                            } else {
                                QTextCharFormat errFmt;
                                errFmt.setFontPointSize(11);
                                errFmt.setFontWeight(QFont::Bold);
                                errFmt.setForeground(QColor("#ff4d4f"));
                                errFmt.setProperty(PropAttachmentRef, attRef);
                                errFmt.setProperty(PropRawMarkdownToken, rawMdToken.isEmpty() ? ("[[" + attRef + "]]") : rawMdToken);
                                cursor.insertText(" [⚠ Файл удалён: " + attRef + "] ", errFmt);
                            }
                            continue;
                        } else if (AttachmentStorage::isImageExtension(attRef) && rawMdToken.startsWith("![[")) {
                            ImageStorage::insertImageAtCursor(cursor, resolved, maxImgWidth, 0, 0, attRef);
                            continue;
                        } else {
                            QTextCharFormat linkFmt;
                            linkFmt.setFontPointSize(11);
                            linkFmt.setFontWeight(QFont::Bold);
                            linkFmt.setForeground(QColor("#38bdf8"));
                            linkFmt.setFontUnderline(true);
                            linkFmt.setAnchor(true);
                            linkFmt.setAnchorHref(QUrl::fromLocalFile(resolved).toString());
                            linkFmt.setProperty(PropAttachmentRef, attRef);
                            linkFmt.setProperty(PropRawMarkdownToken, rawMdToken.isEmpty() ? ("[[" + attRef + "]]") : rawMdToken);
                            cursor.insertText(" [📎 " + attRef + "] ", linkFmt);
                            continue;
                        }
                    }

                    QTextCharFormat charFmt;
                    charFmt.setFontPointSize(ptSize);
                    charFmt.setFontWeight(weight);
                    charFmt.setFontItalic(italic);
                    charFmt.setFontStrikeOut(strike);
                    if (!fontFam.isEmpty()) charFmt.setFontFamily(fontFam);
                    charFmt.setVerticalAlignment(static_cast<QTextCharFormat::VerticalAlignment>(vAlign));

                    if (underline) {
                        charFmt.setFontUnderline(true);
                        charFmt.setUnderlineStyle(QTextCharFormat::SingleUnderline);
                    }
                    if (hasFg && fgColor.isValid()) charFmt.setForeground(fgColor);
                    if (hasBg && bgColor.isValid()) charFmt.setBackground(bgColor);
                    if (hasUlColor && ulColor.isValid()) charFmt.setUnderlineColor(ulColor);
                    if (isAnc) {
                        charFmt.setAnchor(true);
                        charFmt.setAnchorHref(anchorHref);
                    }
                    if (!rawMdToken.isEmpty()) charFmt.setProperty(PropRawMarkdownToken, rawMdToken);
                    if (!attRef.isEmpty())     charFmt.setProperty(PropAttachmentRef, attRef);

                    cursor.insertText(text, charFmt);
                }
            }
        }
    }

    inline std::string toPlainText(const std::string& data) {
        if (data.rfind("RTF3:", 0) != 0 && data.rfind("RTF2:", 0) != 0 && data.rfind("RTF1:", 0) != 0) return data;
        QTextEdit temp;
        deserialize(data, &temp);
        return temp.toPlainText().toStdString();
    }
}

// ============================================================================
// 6b. ПОЛНЫЙ ПАРСЕР И ГЕНЕРАТОР OBSIDIAN FLAVORED MARKDOWN + HTML
// ============================================================================
namespace MarkdownCodec {

    inline QString imageEmbed(const QTextImageFormat& imgFmt) {
        QString origRef = imgFmt.property(PropImageOrigRef).toString();
        QString fn = origRef.isEmpty() ? QFileInfo(imgFmt.name()).fileName() : origRef;
        QString customSz = imgFmt.property(PropImageCustomSize).toString();
        if (!customSz.isEmpty()) {
            return "![[" + fn + "|" + customSz + "]]";
        }
        return "![[" + fn + "]]";
    }

    inline QString resolveImageRef(QString ref, const fs::path& mdDir, int* outW = nullptr, int* outH = nullptr) {
        ref = ref.trimmed();
        int bar = ref.indexOf('|');
        if (bar >= 0) {
            QString sizePart = ref.mid(bar + 1).trimmed();
            ref = ref.left(bar).trimmed();
            static const QRegularExpression dimRe("^(\\d+)(?:[xX×](\\d+))?$");
            QRegularExpressionMatch dm = dimRe.match(sizePart);
            if (dm.hasMatch()) {
                if (outW) *outW = dm.captured(1).toInt();
                if (outH && !dm.captured(2).isEmpty()) *outH = dm.captured(2).toInt();
            }
        }
        int hash = ref.indexOf('#');
        if (hash >= 0) ref = ref.left(hash).trimmed();
        if (ref.startsWith("http://") || ref.startsWith("https://")) return QString();
            ref = QUrl::fromPercentEncoding(ref.toUtf8());
        if (ref.isEmpty()) return QString();

        fs::path p = u8path(ref.toStdString());
        std::error_code ec;
        std::vector<fs::path> candidates = {
            p, mdDir / p, g_vaultRoot / p, g_vaultRoot / u8path("attachments") / p.filename()
        };
        for (const auto& c : candidates) {
            if (fs::exists(c, ec) && fs::is_regular_file(c, ec)) {
                return QString::fromStdString(pathToUtf8(c));
            }
        }
        return AttachmentStorage::findInVaultByName(ref);
    }

    inline void insertFileEmbedAtCursor(QTextCursor& cursor, const QString& refNameIn, bool asBangEmbed = false) {
        QString refName = refNameIn.trimmed();
        if (refName.isEmpty()) return;

        int cw = 0, ch = 0;
        QString cleanRef = refName;
        int bar = cleanRef.indexOf('|');
        if (bar >= 0) {
            QString szStr = cleanRef.mid(bar + 1).trimmed();
            cleanRef = cleanRef.left(bar).trimmed();
            int xIdx = szStr.indexOf('x', 0, Qt::CaseInsensitive);
            if (xIdx > 0) {
                cw = szStr.left(xIdx).toInt();
                ch = szStr.mid(xIdx + 1).toInt();
            } else {
                cw = szStr.toInt();
            }
        }

        if (AttachmentStorage::isImageExtension(cleanRef)) {
            QString resolved = resolveImageRef(refName, g_vaultRoot, &cw, &ch);
            QString rawToken = "![[" + refName + "]]";
            if (!resolved.isEmpty()) {
                ImageStorage::insertImageAtCursor(cursor, resolved, 380, cw, ch, cleanRef);
            } else {
                ImageStorage::insertDeletedFileWarningAtCursor(cursor, cleanRef, rawToken);
            }
            return;
        }

        QString rawToken = (asBangEmbed ? "![[" : "[[") + refName + "]]";
        QString resolved = AttachmentStorage::findInVaultByName(cleanRef);

        if (AttachmentStorage::isFileWithExtension(cleanRef)) {
            if (resolved.isEmpty()) {
                QTextCharFormat errFmt;
                errFmt.setFontPointSize(11);
                errFmt.setFontWeight(QFont::Bold);
                errFmt.setForeground(QColor("#ff4d4f"));
                errFmt.setProperty(PropAttachmentRef, cleanRef);
                errFmt.setProperty(PropRawMarkdownToken, rawToken);
                cursor.insertText(" [⚠ Файл удалён: " + cleanRef + "] ", errFmt);
            } else {
                QTextCharFormat linkFmt;
                linkFmt.setFontPointSize(11);
                linkFmt.setFontWeight(QFont::Bold);
                linkFmt.setForeground(QColor("#38bdf8"));
                linkFmt.setFontUnderline(true);
                linkFmt.setAnchor(true);
                linkFmt.setAnchorHref(QUrl::fromLocalFile(resolved).toString());
                linkFmt.setProperty(PropAttachmentRef, cleanRef);
                linkFmt.setProperty(PropRawMarkdownToken, rawToken);
                cursor.insertText(" [📎 " + cleanRef + "] ", linkFmt);
            }
        } else {
            QString display = refName;
            if (bar >= 0) {
                display = refName.mid(bar + 1).trimmed();
            }
            QTextCharFormat wikiFmt;
            wikiFmt.setFontPointSize(11);
            wikiFmt.setFontWeight(QFont::Bold);
            wikiFmt.setForeground(QColor("#c084fc"));
            wikiFmt.setFontUnderline(true);
            wikiFmt.setAnchor(true);
            if (!resolved.isEmpty()) {
                wikiFmt.setAnchorHref(QUrl::fromLocalFile(resolved).toString());
            } else {
                wikiFmt.setAnchorHref("obsidian://note/" + cleanRef);
            }
            wikiFmt.setProperty(PropRawMarkdownToken, rawToken);
            cursor.insertText((asBangEmbed ? " 📄 " : " 🔗 ") + display + " ", wikiFmt);
        }
    }

    struct SpanStyle {
        QColor fg;
        QColor bg;
        QColor ulColor;
        double pt = 0.0;
        QString fontFamily;
        int weight = -1;
        int italic = -1;
        int underline = -1;
        int strike = -1;
    };

    inline SpanStyle parseSpanStyle(const QString& css) {
        SpanStyle s;
        for (const QString& declRaw : css.split(';')) {
            QString decl = declRaw.trimmed();
            if (decl.isEmpty()) continue;
            int c = decl.indexOf(':');
            if (c <= 0) continue;
            QString k = decl.left(c).trimmed().toLower();
            QString v = decl.mid(c + 1).trimmed();
            if (k == "color") {
                s.fg = QColor(v);
            } else if (k == "background" || k == "background-color") {
                s.bg = QColor(v);
            } else if (k == "text-decoration-color") {
                s.ulColor = QColor(v);
            } else if (k == "font-family") {
                v.remove('\'').remove('"');
                s.fontFamily = v.trimmed();
            } else if (k == "font-weight") {
                if (v.contains("bold", Qt::CaseInsensitive) || v.toInt() >= 600) s.weight = QFont::Bold;
                else if (v.contains("normal", Qt::CaseInsensitive) || (v.toInt() > 0 && v.toInt() <= 400)) s.weight = QFont::Normal;
            } else if (k == "font-style") {
                if (v.contains("italic", Qt::CaseInsensitive)) s.italic = 1;
            } else if (k == "text-decoration") {
                if (v.contains("underline", Qt::CaseInsensitive)) s.underline = 1;
                if (v.contains("line-through", Qt::CaseInsensitive)) s.strike = 1;
            } else if (k == "font-size") {
                QString num = v;
                num.remove(QRegularExpression("[^0-9.]"));
                s.pt = num.toDouble();
                if (v.endsWith("px", Qt::CaseInsensitive)) s.pt *= 0.75;
            }
        }
        return s;
    }

    inline SpanStyle parseFontTagAttributes(const QString& attrStr) {
        SpanStyle s;
        static const QRegularExpression attrRe("([a-zA-Z_-]+)\\s*=\\s*(?:\"([^\"]*)\"|'([^']*)'|([^\\s>]+))");
        QRegularExpressionMatchIterator it = attrRe.globalMatch(attrStr);
        while (it.hasNext()) {
            QRegularExpressionMatch m = it.next();
            QString key = m.captured(1).toLower();
            QString val = !m.captured(2).isNull() ? m.captured(2)
            : (!m.captured(3).isNull() ? m.captured(3) : m.captured(4));
            val = val.trimmed();
            if (key == "color") {
                s.fg = QColor(val);
            } else if (key == "face") {
                s.fontFamily = val;
            } else if (key == "size") {
                double sz = val.toDouble();
                if (val.endsWith("pt", Qt::CaseInsensitive)) s.pt = sz;
                else if (val.endsWith("px", Qt::CaseInsensitive)) s.pt = sz * 0.75;
                else if (sz >= 1 && sz <= 7) {
                    static const double htmlSizes[8] = {11, 8, 10, 11, 13, 16, 20, 26};
                    s.pt = htmlSizes[static_cast<int>(sz)];
                } else if (sz > 7) {
                    s.pt = sz;
                }
            } else if (key == "style") {
                SpanStyle cssStyle = parseSpanStyle(val);
                if (cssStyle.fg.isValid()) s.fg = cssStyle.fg;
                if (cssStyle.bg.isValid()) s.bg = cssStyle.bg;
                if (cssStyle.ulColor.isValid()) s.ulColor = cssStyle.ulColor;
                if (cssStyle.pt > 0.0) s.pt = cssStyle.pt;
                if (!cssStyle.fontFamily.isEmpty()) s.fontFamily = cssStyle.fontFamily;
                if (cssStyle.weight != -1) s.weight = cssStyle.weight;
                if (cssStyle.italic != -1) s.italic = cssStyle.italic;
                if (cssStyle.underline != -1) s.underline = cssStyle.underline;
                if (cssStyle.strike != -1) s.strike = cssStyle.strike;
            }
        }
        return s;
    }

    struct CalloutTheme {
        QString icon;
        QColor color;
        QString defaultTitle;
    };

    inline CalloutTheme getCalloutTheme(const QString& rawType) {
        QString t = rawType.trimmed().toLower();
        if (t == "abstract" || t == "summary" || t == "tldr")
            return {"📋", QColor("#60a5fa"), "Summary"};
        if (t == "info")
            return {"ℹ️", QColor("#38bdf8"), "Info"};
        if (t == "todo")
            return {"☑️", QColor("#94a3b8"), "Todo"};
        if (t == "tip" || t == "hint" || t == "important")
            return {"💡", QColor("#4ade80"), "Tip"};
        if (t == "success" || t == "check" || t == "done")
            return {"✅", QColor("#4ade80"), "Success"};
        if (t == "question" || t == "help" || t == "faq")
            return {"❓", QColor("#fbbf24"), "Question"};
        if (t == "warning" || t == "caution" || t == "attention")
            return {"⚠️", QColor("#fb923c"), "Warning"};
        if (t == "failure" || t == "fail" || t == "missing")
            return {"❌", QColor("#fb7185"), "Failure"};
        if (t == "danger" || t == "error")
            return {"⚡", QColor("#fb7185"), "Danger"};
        if (t == "bug")
            return {"🐞", QColor("#fb7185"), "Bug"};
        if (t == "example")
            return {"🧪", QColor("#c084fc"), "Example"};
        if (t == "quote" || t == "cite")
            return {"💬", QColor("#94a3b8"), "Quote"};
        return {"📝", QColor("#94a3b8"), "Note"};
    }

    inline QString blockToMarkdown(const QTextBlock& block, bool& inFence) {
        QString plain = block.text();
        if (plain.trimmed().startsWith("```")) {
            if (!(plain.trimmed().length() > 6 && plain.trimmed().endsWith("```"))) {
                inFence = !inFence;
            }
            return plain;
        }
        if (inFence) return plain;

        if (plain.startsWith("☐ ")) {
            plain = "- [ ] " + plain.mid(2);
        } else if (plain.startsWith("☑ ")) {
            plain = "- [x] " + plain.mid(2);
        }

        QString out;
        for (auto it = block.begin(); !it.atEnd(); ++it) {
            QTextFragment frag = it.fragment();
            if (!frag.isValid()) continue;
            QTextCharFormat fmt = frag.charFormat();

            if (fmt.isImageFormat()) {
                out += imageEmbed(fmt.toImageFormat());
                continue;
            }

            QString rawToken = fmt.property(PropRawMarkdownToken).toString();
            if (!rawToken.isEmpty()) {
                out += rawToken;
                continue;
            }

            QString text = frag.text();
            text.replace(QChar::LineSeparator, QLatin1Char('\n'));
            text.remove(QChar::ObjectReplacementCharacter);
            if (text.isEmpty()) continue;

            if (text.startsWith("☐ ")) text = "- [ ] " + text.mid(2);
            else if (text.startsWith("☑ ")) text = "- [x] " + text.mid(2);

            QString trimmedBlock = plain.trimmed();
            bool isStructuralLine = trimmedBlock.startsWith("#") ||
            trimmedBlock.startsWith(">") ||
            trimmedBlock.startsWith("|") ||
            trimmedBlock == "---" || trimmedBlock == "***" || trimmedBlock == "___";
            if (isStructuralLine && !fmt.hasProperty(QTextFormat::BackgroundBrush)) {
                out += text;
                continue;
            }

            int l = 0;
            while (l < text.size() && text[l].isSpace()) ++l;
            int r = text.size();
            while (r > l && text[r - 1].isSpace()) --r;
            QString lead = text.left(l), core = text.mid(l, r - l), trail = text.mid(r);
            if (core.isEmpty()) { out += text; continue; }

            if (fmt.fontFamily() == MONO_FONT_FAMILY && !core.startsWith('`')) {
                out += lead + "`" + core + "`" + trail;
                continue;
            }

            bool bold   = fmt.fontWeight() >= QFont::DemiBold;
            bool italic = fmt.fontItalic();
            bool strike = fmt.fontStrikeOut();
            bool ul     = fmt.fontUnderline();

            bool isObsidianHighlight = false;
            if (fmt.hasProperty(QTextFormat::BackgroundBrush) &&
                fmt.background().color().name().toLower() == "#92400e") {
                isObsidianHighlight = true;
                }

                QStringList styles;
            if (fmt.hasProperty(QTextFormat::ForegroundBrush)) {
                QColor fc = fmt.foreground().color();
                if (fc != QColor(240, 246, 255) && fc != QColor(248, 250, 252) && fc != QColor(235, 242, 255)) {
                    styles << "color:" + fc.name();
                }
            }
            if (fmt.hasProperty(QTextFormat::BackgroundBrush) && !isObsidianHighlight) {
                styles << "background-color:" + fmt.background().color().name();
            }
            double pt = fmt.fontPointSize();
            if (pt > 0.0 && (pt < 10.0 || pt > 12.0)) {
                styles << QString("font-size:%1pt").arg(pt);
            }
            if (ul && fmt.hasProperty(QTextFormat::TextUnderlineColor)) {
                styles << "text-decoration-color:" + fmt.underlineColor().name();
            }

            if (ul)                  core = "<u>" + core + "</u>";
            if (strike)              core = "~~" + core + "~~";
            if (isObsidianHighlight) core = "==" + core + "==";
            if (bold && italic)      core = "***" + core + "***";
            else if (bold)           core = "**" + core + "**";
            else if (italic)         core = "*" + core + "*";

            if (!styles.isEmpty()) {
                core = "<span style=\"" + styles.join(';') + "\">" + core + "</span>";
            }

            out += lead + core + trail;
        }

        Qt::Alignment a = block.blockFormat().alignment();
        if (a & Qt::AlignHCenter)    out = "<p align=\"center\">" + out + "</p>";
        else if (a & Qt::AlignRight) out = "<p align=\"right\">" + out + "</p>";
        return out;
    }

    inline QString toMarkdown(const std::string& rtfData) {
        if (rtfData.empty()) return QString();
        QTextEdit temp;
        RichTextCodec::deserialize(rtfData, &temp);
        QStringList lines;
        bool inFence = false;
        for (QTextBlock b = temp.document()->begin(); b.isValid(); b = b.next()) {
            lines << blockToMarkdown(b, inFence);
        }
        return lines.join("\n");
    }

    inline std::string fromMarkdown(const QString& mdIn, const fs::path& mdDir) {
        if (mdIn.trimmed().isEmpty()) return std::string();
        QString md = mdIn;
        md.replace("\r\n", "\n");

        QTextDocument doc;
        QTextCursor cursor(&doc);

        static const QRegularExpression mdImgRe("^!\\[([^\\]]*)\\]\\(([^)]+)\\)");
        static const QRegularExpression mdLinkRe("^\\[([^\\]]+)\\]\\(([^)]+)\\)");
        static const QRegularExpression openTagRe("^<([a-zA-Z0-9]+)(\\s+[^>]*)?>");
        static const QRegularExpression closeTagRe("^</([a-zA-Z0-9]+)\\s*>");
        static const QRegularExpression pAlignRe("^\\s*<p\\s+align=[\"']?(center|right|left)[\"']?\\s*>(.*)</p>\\s*$", QRegularExpression::CaseInsensitiveOption);
        static const QRegularExpression headingRe("^(#{1,6})\\s+(.*)$");
        static const QRegularExpression calloutRe("^(\\s*>+\\s*)\\[!([a-zA-Z0-9_-]+)\\]([+-]?)\\s*(.*)$");
        static const QRegularExpression quoteRe("^(\\s*>+\\s*)(.*)$");
        static const QRegularExpression taskRe("^(\\s*(?:[-*+]|\\d+\\.)\\s+)\\[([ xX])\\]\\s+(.*)$");
        static const QRegularExpression hrRe("^\\s*(?:---|\\*\\*\\*|___|<hr\\s*/?>)\\s*$", QRegularExpression::CaseInsensitiveOption);
        static const QRegularExpression tagTokenRe("^#([a-zA-Zа-яА-ЯёЁ0-9_/-]+(?:\\{[0-9-]+\\})?)");
        static const QRegularExpression footnoteRefRe("^\\[\\^([^\\]]+)\\](?!:)");

        QTextCharFormat codeFmt;
        codeFmt.setFontFamily(MONO_FONT_FAMILY);
        codeFmt.setFontPointSize(11);
        codeFmt.setForeground(QColor(240, 246, 255));

        QTextCharFormat mathBlockFmt;
        mathBlockFmt.setFontFamily(MONO_FONT_FAMILY);
        mathBlockFmt.setFontPointSize(11.5);
        mathBlockFmt.setFontItalic(true);
        mathBlockFmt.setForeground(QColor("#2dd4bf"));
        mathBlockFmt.setBackground(QColor("#0d1f2d"));

        QStringList lines = md.split('\n');
        bool inFence = false;
        bool inMathBlock = false;
        QColor activeCalloutColor;

        for (int li = 0; li < lines.size(); ++li) {
            if (li > 0) cursor.insertBlock();
            QString line = lines[li];

            QTextBlockFormat bf;
            bf.setAlignment(Qt::AlignLeft);
            cursor.setBlockFormat(bf);

            QString trimmed = line.trimmed();

            if (trimmed.startsWith("```")) {
                if (!inFence && trimmed.length() > 6 && trimmed.endsWith("```")) {
                    cursor.insertText(line, codeFmt);
                    continue;
                }
                inFence = !inFence;
                cursor.insertText(line, codeFmt);
                continue;
            }
            if (inFence) {
                cursor.insertText(line, codeFmt);
                continue;
            }

            if (trimmed.startsWith("$$")) {
                if (!inMathBlock && trimmed.length() > 4 && trimmed.endsWith("$$")) {
                    cursor.insertText(line, mathBlockFmt);
                    continue;
                }
                inMathBlock = !inMathBlock;
                cursor.insertText(line, mathBlockFmt);
                continue;
            }
            if (inMathBlock) {
                cursor.insertText(line, mathBlockFmt);
                continue;
            }

            if (hrRe.match(line).hasMatch()) {
                QTextCharFormat hrFmt;
                hrFmt.setFontPointSize(11);
                hrFmt.setFontWeight(QFont::Bold);
                hrFmt.setForeground(QColor("#475569"));
                hrFmt.setProperty(PropRawMarkdownToken, line);
                cursor.insertText("────────────────────────────────────────", hrFmt);
                continue;
            }

            if (trimmed.startsWith("|") && trimmed.endsWith("|")) {
                QTextCharFormat tblFmt;
                tblFmt.setFontFamily(MONO_FONT_FAMILY);
                tblFmt.setFontPointSize(10.5);
                bool isSep = trimmed.contains(QRegularExpression("^\\|[\\s:-]+\\|"));
                tblFmt.setForeground(isSep ? QColor("#64748b") : QColor("#bae6fd"));
                if (!isSep && li + 1 < lines.size() && lines[li + 1].trimmed().contains(QRegularExpression("^\\|[\\s:-]+\\|"))) {
                    tblFmt.setFontWeight(QFont::Bold);
                    tblFmt.setForeground(QColor("#38bdf8"));
                }
                cursor.insertText(line, tblFmt);
                continue;
            }

            QRegularExpressionMatch am = pAlignRe.match(line);
            if (am.hasMatch()) {
                QString a = am.captured(1).toLower();
                bf.setAlignment(a == "center" ? Qt::AlignHCenter : (a == "right" ? Qt::AlignRight : Qt::AlignLeft));
                cursor.setBlockFormat(bf);
                line = am.captured(2);
            }

            double basePtSize = 11.0;
            bool baseBold = false;
            QColor baseFgColor;

            QRegularExpressionMatch calloutMatch = calloutRe.match(line);
            QRegularExpressionMatch quoteMatch   = quoteRe.match(line);
            QRegularExpressionMatch headingMatch = headingRe.match(line);
            QRegularExpressionMatch taskMatch    = taskRe.match(line);

            if (calloutMatch.hasMatch()) {
                QString quotePrefix = calloutMatch.captured(1);
                QString cType       = calloutMatch.captured(2);
                QString foldMod     = calloutMatch.captured(3);
                QString customTitle = calloutMatch.captured(4).trimmed();

                CalloutTheme theme = getCalloutTheme(cType);
                activeCalloutColor = theme.color;

                QTextCharFormat cFmt;
                cFmt.setFontPointSize(11.5);
                cFmt.setFontWeight(QFont::Bold);
                cFmt.setForeground(theme.color);
                cFmt.setProperty(PropRawMarkdownToken, line);

                QString foldIcon = (foldMod == "-") ? " ▸" : (foldMod == "+" ? " ▾" : "");
                QString titleTxt = customTitle.isEmpty() ? theme.defaultTitle : customTitle;
                cursor.insertText(quotePrefix + theme.icon + " [" + cType.toUpper() + "]" + foldIcon + " " + titleTxt, cFmt);
                continue;
            } else if (quoteMatch.hasMatch()) {
                baseFgColor = activeCalloutColor.isValid() ? activeCalloutColor.lighter(125) : QColor("#94a3b8");
            } else {
                activeCalloutColor = QColor();
            }

            if (headingMatch.hasMatch()) {
                int hLevel = headingMatch.captured(1).length();
                static const double hSizes[7] = {11.0, 17.0, 15.0, 13.5, 12.5, 11.5, 11.0};
                basePtSize = hSizes[std::clamp(hLevel, 1, 6)];
                baseBold = true;
                static const QColor hColors[7] = {
                    QColor("#f8fafc"),
                    QColor("#38bdf8"),
                    QColor("#c084fc"),
                    QColor("#4ade80"),
                    QColor("#fbbf24"),
                    QColor("#f472b6"),
                    QColor("#94a3b8")
                };
                baseFgColor = hColors[std::clamp(hLevel, 1, 6)];
            } else if (taskMatch.hasMatch()) {
                bool isDone = (taskMatch.captured(2).toLower() == "x");
                QString rest = taskMatch.captured(3);
                line = (isDone ? "☑ " : "☐ ") + rest;
                if (isDone) {
                    baseFgColor = QColor("#4ade80");
                }
            }

            bool bold = baseBold, italic = false, ul = false, strike = false;
            int vAlign = 0;
            bool inHighlight = false;
            QColor highlightBgColor;
            std::vector<SpanStyle> spans;
            QString buf;

            auto currentFmt = [&]() {
                QTextCharFormat f;
                f.setFontPointSize(basePtSize);
                f.setFontWeight(bold ? QFont::Bold : QFont::Normal);
                f.setFontItalic(italic);
                f.setFontStrikeOut(strike);
                if (vAlign != 0) {
                    f.setVerticalAlignment(vAlign > 0 ? QTextCharFormat::AlignSuperScript : QTextCharFormat::AlignSubScript);
                }
                if (baseFgColor.isValid()) {
                    f.setForeground(baseFgColor);
                }
                if (ul) {
                    f.setFontUnderline(true);
                    f.setUnderlineStyle(QTextCharFormat::SingleUnderline);
                }
                if (inHighlight) {
                    QColor bg = highlightBgColor.isValid() ? highlightBgColor : QColor("#92400e");
                    f.setBackground(bg);
                    f.setForeground(getReadableTextColorForBg(bg));
                }
                for (const auto& s : spans) {
                    if (s.fg.isValid())             f.setForeground(s.fg);
                    if (s.bg.isValid())             f.setBackground(s.bg);
                    if (s.pt > 0.0)                 f.setFontPointSize(s.pt);
                    if (s.ulColor.isValid())        f.setUnderlineColor(s.ulColor);
                    if (!s.fontFamily.isEmpty())    f.setFontFamily(s.fontFamily);
                    if (s.weight != -1)             f.setFontWeight(s.weight);
                    if (s.italic != -1)             f.setFontItalic(s.italic == 1);
                    if (s.underline != -1) {
                        f.setFontUnderline(s.underline == 1);
                        f.setUnderlineStyle(QTextCharFormat::SingleUnderline);
                    }
                    if (s.strike != -1)             f.setFontStrikeOut(s.strike == 1);
                }
                return f;
            };

            auto flush = [&]() {
                if (!buf.isEmpty()) {
                    cursor.insertText(buf, currentFmt());
                    buf.clear();
                }
            };

            auto startsAt = [&](int idx, const char* s) {
                return QStringView(line).mid(idx).startsWith(QLatin1String(s), Qt::CaseInsensitive);
            };

            int i = 0;
            while (i < line.size()) {
                if (line[i] == '\\' && i + 1 < line.size()) {
                    QChar nextCh = line[i + 1];
                    if (QString("*_~=`[]#<>\\|!").contains(nextCh)) {
                        flush();
                        QTextCharFormat escFmt = currentFmt();
                        escFmt.setProperty(PropRawMarkdownToken, line.mid(i, 2));
                        cursor.insertText(QString(nextCh), escFmt);
                        i += 2;
                        continue;
                    }
                }

                if (startsAt(i, "%%")) {
                    int endC = line.indexOf("%%", i + 2);
                    if (endC > i) {
                        flush();
                        QString rawComm = line.mid(i, endC + 2 - i);
                        QTextCharFormat commFmt;
                        commFmt.setFontPointSize(9.5);
                        commFmt.setFontItalic(true);
                        commFmt.setForeground(QColor("#64748b"));
                        commFmt.setProperty(PropRawMarkdownToken, rawComm);
                        cursor.insertText(rawComm, commFmt);
                        i = endC + 2;
                        continue;
                    }
                }

                if (startsAt(i, "```")) {
                    int endFence = line.indexOf("```", i + 3);
                    if (endFence > i) {
                        flush();
                        QString rawTriple = line.mid(i, endFence + 3 - i);
                        QString innerCode = line.mid(i + 3, endFence - (i + 3)).trimmed();
                        QTextCharFormat icFmt;
                        icFmt.setFontFamily(MONO_FONT_FAMILY);
                        icFmt.setFontPointSize(10.5);
                        icFmt.setForeground(QColor("#38bdf8"));
                        icFmt.setBackground(QColor("#0b1329"));
                        icFmt.setProperty(PropRawMarkdownToken, rawTriple);
                        cursor.insertText(" " + innerCode + " ", icFmt);
                        i = endFence + 3;
                        continue;
                    }
                }

                if (line[i] == '`') {
                    int endTick = line.indexOf('`', i + 1);
                    if (endTick > i) {
                        flush();
                        QString codeContent = line.mid(i + 1, endTick - i - 1);
                        QTextCharFormat icFmt;
                        icFmt.setFontFamily(MONO_FONT_FAMILY);
                        icFmt.setFontPointSize(10.5);
                        icFmt.setForeground(QColor("#fbbf24"));
                        icFmt.setBackground(QColor("#0f172a"));
                        icFmt.setProperty(PropRawMarkdownToken, "`" + codeContent + "`");
                        cursor.insertText(codeContent, icFmt);
                        i = endTick + 1;
                        continue;
                    }
                }

                if (startsAt(i, "$$")) {
                    int endM = line.indexOf("$$", i + 2);
                    if (endM > i) {
                        flush();
                        QString rawMath = line.mid(i, endM + 2 - i);
                        QString expr = line.mid(i + 2, endM - (i + 2));
                        QTextCharFormat mf = mathBlockFmt;
                        mf.setProperty(PropRawMarkdownToken, rawMath);
                        cursor.insertText(" ∑(" + expr + ") ", mf);
                        i = endM + 2;
                        continue;
                    }
                }
                if (line[i] == '$' && (i + 1 < line.size() && !line[i + 1].isSpace())) {
                    int endM = line.indexOf('$', i + 1);
                    if (endM > i + 1 && !line[endM - 1].isSpace()) {
                        flush();
                        QString rawMath = line.mid(i, endM + 1 - i);
                        QString expr = line.mid(i + 1, endM - i - 1);
                        QTextCharFormat mf;
                        mf.setFontFamily(MONO_FONT_FAMILY);
                        mf.setFontPointSize(10.5);
                        mf.setFontItalic(true);
                        mf.setForeground(QColor("#2dd4bf"));
                        mf.setBackground(QColor("#0d1f2d"));
                        mf.setProperty(PropRawMarkdownToken, rawMath);
                        cursor.insertText(expr, mf);
                        i = endM + 1;
                        continue;
                    }
                }

                if (startsAt(i, "![[")) {
                    int end = line.indexOf("]]", i + 3);
                    if (end > 0) {
                        flush();
                        QString ref = line.mid(i + 3, end - i - 3);
                        insertFileEmbedAtCursor(cursor, ref, true);
                        i = end + 2;
                        continue;
                    }
                }

                if (startsAt(i, "[[")) {
                    int end = line.indexOf("]]", i + 2);
                    if (end > 0) {
                        flush();
                        QString ref = line.mid(i + 2, end - i - 2);
                        insertFileEmbedAtCursor(cursor, ref, false);
                        i = end + 2;
                        continue;
                    }
                }

                if (startsAt(i, "![")) {
                    QRegularExpressionMatch m = mdImgRe.match(line.mid(i));
                    if (m.hasMatch()) {
                        flush();
                        int cw = 0, ch = 0;
                        QString alt = m.captured(1);
                        static const QRegularExpression dimRe("^(\\d+)(?:[xX×](\\d+))?$");
                        QRegularExpressionMatch dm = dimRe.match(alt.trimmed());
                        if (dm.hasMatch()) {
                            cw = dm.captured(1).toInt();
                            if (!dm.captured(2).isEmpty()) ch = dm.captured(2).toInt();
                        }
                        QString ref = m.captured(2);
                        QString p = resolveImageRef(ref, mdDir);
                        if (!p.isEmpty()) ImageStorage::insertImageAtCursor(cursor, p, 380, cw, ch, ref);
                        else ImageStorage::insertDeletedFileWarningAtCursor(cursor, ref, m.captured(0));
                        i += m.capturedLength();
                        continue;
                    }
                }

                if (startsAt(i, "[^")) {
                    QRegularExpressionMatch fm = footnoteRefRe.match(line.mid(i));
                    if (fm.hasMatch()) {
                        flush();
                        QTextCharFormat fnFmt = currentFmt();
                        fnFmt.setVerticalAlignment(QTextCharFormat::AlignSuperScript);
                        fnFmt.setForeground(QColor("#38bdf8"));
                        fnFmt.setFontWeight(QFont::Bold);
                        fnFmt.setProperty(PropRawMarkdownToken, fm.captured(0));
                        cursor.insertText("[" + fm.captured(1) + "]", fnFmt);
                        i += fm.capturedLength();
                        continue;
                    }
                }

                if (line[i] == '[') {
                    QRegularExpressionMatch lm = mdLinkRe.match(line.mid(i));
                    if (lm.hasMatch()) {
                        flush();
                        QTextCharFormat lFmt = currentFmt();
                        lFmt.setForeground(QColor("#38bdf8"));
                        lFmt.setFontUnderline(true);
                        lFmt.setAnchor(true);
                        lFmt.setAnchorHref(lm.captured(2));
                        lFmt.setProperty(PropRawMarkdownToken, lm.captured(0));
                        cursor.insertText(lm.captured(1), lFmt);
                        i += lm.capturedLength();
                        continue;
                    }
                }

                if (startsAt(i, "==")) {
                    flush();
                    if (!inHighlight) {
                        inHighlight = true;
                        highlightBgColor = QColor("#92400e");
                        QString rest = line.mid(i + 2);
                        struct EmojiColor { const char* emoji; const char* hex; };
                        static const EmojiColor emojiColors[] = {
                            {"🔴", "#881337"}, {"🟢", "#065f46"}, {"🔵", "#1e40af"},
                            {"🟡", "#92400e"}, {"🟣", "#5b21b6"}, {"🟠", "#9a3412"},
                            {"🩵", "#115e59"}, {"⚪", "#334155"}
                        };
                        for (const auto& ec : emojiColors) {
                            QString em = QString::fromUtf8(ec.emoji);
                            if (rest.startsWith(em)) {
                                highlightBgColor = QColor(ec.hex);
                                break;
                            }
                        }
                    } else {
                        inHighlight = false;
                        highlightBgColor = QColor();
                    }
                    i += 2;
                    continue;
                }

                if (startsAt(i, "~~")) {
                    flush();
                    strike = !strike;
                    i += 2;
                    continue;
                }

                if (startsAt(i, "***")) {
                    flush();
                    bool bothActive = (bold && italic);
                    bold   = !bothActive;
                    italic = !bothActive;
                    i += 3;
                    continue;
                }

                if (startsAt(i, "**")) {
                    flush();
                    bold = !bold;
                    i += 2;
                    continue;
                }

                if (line[i] == '*' && (i + 1 < line.size() && !line[i + 1].isSpace())) {
                    flush();
                    italic = !italic;
                    i += 1;
                    continue;
                }

                if (line[i] == '#' && (i == 0 || line[i - 1].isSpace())) {
                    QRegularExpressionMatch tm = tagTokenRe.match(line.mid(i));
                    if (tm.hasMatch()) {
                        flush();
                        QTextCharFormat tagFmt = currentFmt();
                        tagFmt.setForeground(QColor("#38bdf8"));
                        tagFmt.setBackground(QColor("#13284c"));
                        tagFmt.setFontWeight(QFont::Bold);
                        tagFmt.setProperty(PropRawMarkdownToken, tm.captured(0));
                        cursor.insertText(tm.captured(0), tagFmt);
                        i += tm.capturedLength();
                        continue;
                    }
                }

                if (line[i] == '<') {
                    QRegularExpressionMatch closeM = closeTagRe.match(line.mid(i));
                    if (closeM.hasMatch()) {
                        flush();
                        QString tag = closeM.captured(1).toLower();
                        if (tag == "u" || tag == "ins") ul = false;
                        else if (tag == "b" || tag == "strong") bold = baseBold;
                        else if (tag == "i" || tag == "em") italic = false;
                        else if (tag == "s" || tag == "del" || tag == "strike") strike = false;
                        else if (tag == "sup" || tag == "sub") vAlign = 0;
                        else if (tag == "font" || tag == "span" || tag == "mark" || tag == "kbd" || tag == "div") {
                            if (!spans.empty()) spans.pop_back();
                        }
                        i += closeM.capturedLength();
                        continue;
                    }

                    QRegularExpressionMatch openM = openTagRe.match(line.mid(i));
                    if (openM.hasMatch()) {
                        QString tag = openM.captured(1).toLower();
                        QString attrs = openM.captured(2);

                        if (tag == "br") {
                            flush();
                            cursor.insertBlock();
                            i += openM.capturedLength();
                            continue;
                        }
                        if (tag == "u" || tag == "ins") {
                            flush();
                            ul = true;
                            i += openM.capturedLength();
                            continue;
                        }
                        if (tag == "b" || tag == "strong") {
                            flush();
                            bold = true;
                            i += openM.capturedLength();
                            continue;
                        }
                        if (tag == "i" || tag == "em") {
                            flush();
                            italic = true;
                            i += openM.capturedLength();
                            continue;
                        }
                        if (tag == "s" || tag == "del" || tag == "strike") {
                            flush();
                            strike = true;
                            i += openM.capturedLength();
                            continue;
                        }
                        if (tag == "sup") {
                            flush();
                            vAlign = 1;
                            i += openM.capturedLength();
                            continue;
                        }
                        if (tag == "sub") {
                            flush();
                            vAlign = -1;
                            i += openM.capturedLength();
                            continue;
                        }
                        if (tag == "font" || tag == "span" || tag == "div") {
                            flush();
                            spans.push_back(parseFontTagAttributes(attrs));
                            i += openM.capturedLength();
                            continue;
                        }
                        if (tag == "mark") {
                            flush();
                            SpanStyle ms = parseFontTagAttributes(attrs);
                            if (!ms.bg.isValid()) ms.bg = QColor("#92400e");
                            if (!ms.fg.isValid()) ms.fg = getReadableTextColorForBg(ms.bg);
                            spans.push_back(ms);
                            i += openM.capturedLength();
                            continue;
                        }
                        if (tag == "kbd") {
                            flush();
                            SpanStyle ks;
                            ks.fontFamily = MONO_FONT_FAMILY;
                            ks.bg = QColor("#1e293b");
                            ks.fg = QColor("#f8fafc");
                            spans.push_back(ks);
                            i += openM.capturedLength();
                            continue;
                        }
                        if (tag == "details") {
                            flush();
                            i += openM.capturedLength();
                            continue;
                        }
                        if (tag == "summary") {
                            flush();
                            bold = true;
                            buf += "▾ ";
                            i += openM.capturedLength();
                            continue;
                        }
                        if (tag == "iframe") {
                            flush();
                            static const QRegularExpression srcRe("src\\s*=\\s*[\"']([^\"']+)[\"']", QRegularExpression::CaseInsensitiveOption);
                            QRegularExpressionMatch sm = srcRe.match(attrs);
                            QString url = sm.hasMatch() ? sm.captured(1) : "iframe";
                            QTextCharFormat ifFmt = currentFmt();
                            ifFmt.setForeground(QColor("#38bdf8"));
                            ifFmt.setFontUnderline(true);
                            ifFmt.setAnchor(true);
                            ifFmt.setAnchorHref(url);
                            ifFmt.setProperty(PropRawMarkdownToken, openM.captured(0) + "</iframe>");
                            cursor.insertText(" [🌐 IFrame: " + url + "] ", ifFmt);
                            i += openM.capturedLength();
                            continue;
                        }
                    }
                }

                buf += line[i];
                ++i;
            }
            flush();
        }

        return RichTextCodec::serialize(&doc);
    }

    inline std::string yamlEscape(const std::string& s) {
        std::string out;
        for (char ch : s) {
            if (ch == '\\') out += "\\\\";
            else if (ch == '"') out += "\\\"";
            else if (ch == '\n') out += ' ';
            else out += ch;
        }
        return out;
    }

    inline std::string yamlUnquote(QString v) {
        v = v.trimmed();
        if (v.size() >= 2 && ((v.startsWith('"') && v.endsWith('"')) || (v.startsWith('\'') && v.endsWith('\'')))) {
            v = v.mid(1, v.size() - 2);
            v.replace("\\\"", "\"");
            v.replace("\\\\", "\\");
        }
        return v.toStdString();
    }

    struct ParsedNote {
        std::map<std::string, std::string> meta;
        std::string title;
        std::string code;
        QString descMd;
        QString extMd;
        bool hasExplicitExtSection = false;
    };

    inline void trimBlankLines(QStringList& l) {
        while (!l.isEmpty() && l.first().trimmed().isEmpty()) l.removeFirst();
        while (!l.isEmpty() && l.last().trimmed().isEmpty())  l.removeLast();
    }

    inline ParsedNote parseMarkdownFile(const std::string& content) {
        ParsedNote res;
        QString text = QString::fromStdString(content);
        text.replace("\r\n", "\n");
        QStringList lines = text.split('\n');
        int i = 0;

        bool hasExplicitCodeField = false;
        if (!lines.isEmpty() && lines[0].trimmed() == "---") {
            int j = 1;
            while (j < lines.size() && lines[j].trimmed() != "---") {
                int c = lines[j].indexOf(':');
                if (c > 0) {
                    std::string key = lines[j].left(c).trimmed().toLower().toStdString();
                    std::string val = yamlUnquote(lines[j].mid(c + 1));
                    res.meta[key] = val;
                    if (key == "has_code_field" && (val == "true" || val == "1")) {
                        hasExplicitCodeField = true;
                    }
                }
                ++j;
            }
            if (j < lines.size()) i = j + 1;
        }

        while (i < lines.size() && lines[i].trimmed().isEmpty()) ++i;

        if (i < lines.size() && lines[i].startsWith("# ")) {
            res.title = lines[i].mid(2).trimmed().toStdString();
            ++i;
            while (i < lines.size() && lines[i].trimmed().isEmpty()) ++i;
        }

        // Извлекаем верхний блок ``` в отдельное поле code ТОЛЬКО если оно было явно сохранено при включённом поле кода.
        // Все остальные блоки ``` ... ``` остаются внутри потока описания карточки!
        if (hasExplicitCodeField && i < lines.size() && lines[i].trimmed().startsWith("```") &&
            !(lines[i].trimmed().length() > 6 && lines[i].trimmed().endsWith("```"))) {
            ++i;
        QStringList codeLines;
        while (i < lines.size() && !lines[i].trimmed().startsWith("```")) codeLines << lines[i++];
        if (i < lines.size()) ++i;
        res.code = codeLines.join('\n').toStdString();
            }

            QStringList descLines, extLines;
            bool inExt = false, fence = false;
            for (; i < lines.size(); ++i) {
                QString t = lines[i].trimmed();
                if (t.startsWith("```") && !(t.length() > 6 && t.endsWith("```"))) {
                    fence = !fence;
                }
                if (!fence && !inExt && t.startsWith("## Подробнее")) {
                    inExt = true;
                    res.hasExplicitExtSection = true;
                    continue;
                }
                (inExt ? extLines : descLines) << lines[i];
            }
            trimBlankLines(descLines);
            trimBlankLines(extLines);
            res.descMd = descLines.join('\n');
            res.extMd  = extLines.join('\n');
            return res;
    }
}

// ============================================================================
// 7. БАЗА ДАННЫХ SQLITE И ДВУСТОРОННЯЯ СИНХРОНИЗАЦИЯ С OBSIDIAN
// ============================================================================
struct Card {
    long long id = 0;
    bool pinned = false;
    std::string chapter;
    std::string section;
    std::string title;
    std::string code;
    std::string description;
    std::string extendedDesc;
    std::string filePath;
    long long fileMtime = 0;
};

namespace MarkdownCodec {
    inline std::string buildCardMarkdown(const Card& c) {
        bool writeCodeBlock = g_cfg.showCodeField && !QString::fromStdString(c.code).trimmed().isEmpty();

        std::ostringstream out;
        out << "---\n";
        out << "id: " << c.id << "\n";
        out << "title: \"" << yamlEscape(c.title) << "\"\n";
        out << "chapter: \"" << yamlEscape(c.chapter) << "\"\n";
        out << "section: \"" << yamlEscape(c.section) << "\"\n";
        out << "pinned: " << (c.pinned ? "true" : "false") << "\n";
        if (writeCodeBlock) {
            out << "has_code_field: true\n";
        }
        out << "tags: [kb-card]\n";
        out << "---\n\n";
        out << "# " << c.title << "\n\n";

        if (writeCodeBlock) {
            out << "```\n" << c.code;
            if (c.code.empty() || c.code.back() != '\n') out << "\n";
            out << "```\n\n";
        }

        QString descMd = toMarkdown(c.description).trimmed();
        QString extMd  = toMarkdown(c.extendedDesc).trimmed();

        if (!descMd.isEmpty()) {
            out << descMd.toStdString() << "\n";
        }

        // Чёткое разделение между описанием карточки и правой панелью («## Подробнее»)
        if (!extMd.isEmpty()) {
            out << "\n## Подробнее\n\n" << extMd.toStdString() << "\n";
        }
        return out.str();
    }
}

class DatabaseManager {
private:
    sqlite3* db = nullptr;
    fs::path vaultRoot;

public:
    DatabaseManager(const std::string& dbFilename, const fs::path& vaultDir) : vaultRoot(vaultDir) {
        g_vaultRoot = vaultDir;
        if (sqlite3_open(dbFilename.c_str(), &db) != SQLITE_OK) {
            std::cerr << "Ошибка SQLite: " << sqlite3_errmsg(db) << std::endl;
        }
        std::error_code ec;
        fs::create_directories(vaultRoot, ec);
        ImageStorage::ensureImagesDir();
        initTables();
        loadConfig(g_cfg);
        loadCustomPalette();

        g_onSaveCustomPaletteToDB = [this]() { saveCustomPalette(); };
    }

    ~DatabaseManager() {
        if (db) sqlite3_close(db);
    }

    static std::pair<std::string, std::string> extractChapterAndSectionFromRelDir(const fs::path& relDir) {
        std::vector<std::string> parts;
        for (const auto& p : relDir) {
            std::string s = p.u8string();
            if (!s.empty() && s != "." && s != "/" && s != "\\") parts.push_back(s);
        }
        if (parts.empty()) return {"all", "all"};

        if (parts[0] == "Skillbox") {
            if (parts.size() == 1) return {"all", "all"};
            if (parts.size() == 2) return {parts[1], "all"};
            std::string sec;
            for (size_t i = 2; i < parts.size(); ++i) {
                if (!sec.empty()) sec += "/";
                sec += parts[i];
            }
            return {parts[1], sec};
        }

        if (parts.size() == 1) return {parts[0], "all"};
        std::string sec;
        for (size_t i = 1; i < parts.size(); ++i) {
            if (!sec.empty()) sec += "/";
            sec += parts[i];
        }
        return {parts[0], sec};
    }

    void executeSQL(const std::string& sql) {
        char* errMsg = nullptr;
        if (sqlite3_exec(db, sql.c_str(), nullptr, nullptr, &errMsg) != SQLITE_OK) {
            sqlite3_free(errMsg);
        }
    }

    void initTables() {
        executeSQL("CREATE TABLE IF NOT EXISTS kb_cards ("
        "id INTEGER PRIMARY KEY, "
        "pinned INTEGER DEFAULT 0, "
        "chapter TEXT NOT NULL, "
        "section TEXT NOT NULL, "
        "title TEXT NOT NULL, "
        "rawCode TEXT, "
        "desc_text TEXT, "
        "extended_desc TEXT, "
        "file_path TEXT DEFAULT '', "
        "file_mtime INTEGER DEFAULT 0);");

        executeSQL("ALTER TABLE kb_cards ADD COLUMN file_path TEXT DEFAULT '';");
        executeSQL("ALTER TABLE kb_cards ADD COLUMN file_mtime INTEGER DEFAULT 0;");

        executeSQL("CREATE TABLE IF NOT EXISTS kb_app_settings ("
        "k TEXT PRIMARY KEY, "
        "v TEXT);");

        executeSQL("CREATE TABLE IF NOT EXISTS kb_color_palette ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT, "
        "label TEXT NOT NULL, "
        "hex TEXT NOT NULL);");
    }

    std::vector<std::string> getExistingChapters() {
        std::set<std::string> chapters;
        std::error_code ec;
        if (fs::exists(vaultRoot, ec)) {
            for (const auto& entry : fs::directory_iterator(vaultRoot, fs::directory_options::skip_permission_denied, ec)) {
                if (!entry.is_directory(ec)) continue;
                std::string name = filenameToUtf8(entry.path());
                if (name.empty() || name[0] == '.' || name == "attachments" || name == "images") continue;
                if (name == "Skillbox") {
                    for (const auto& sbEntry : fs::directory_iterator(entry.path(), fs::directory_options::skip_permission_denied, ec)) {
                        if (sbEntry.is_directory(ec)) {
                            std::string sbName = filenameToUtf8(sbEntry.path());
                            if (!sbName.empty() && sbName[0] != '.') chapters.insert(sbName);
                        }
                    }
                } else {
                    chapters.insert(name);
                }
            }
        }
        for (const auto& c : getAllCards()) {
            if (!c.chapter.empty()) chapters.insert(c.chapter);
        }
        return std::vector<std::string>(chapters.begin(), chapters.end());
    }

    std::vector<std::string> getExistingSections(const std::string& chapter) {
        std::set<std::string> sections;
        if (chapter.empty() || chapter == "all" || chapter == "__pinned__") return {};

        std::error_code ec;
        fs::path chFolder = resolveCardFolder(chapter, "all");
        if (fs::exists(chFolder, ec) && fs::is_directory(chFolder, ec)) {
            for (const auto& entry : fs::recursive_directory_iterator(chFolder, fs::directory_options::skip_permission_denied, ec)) {
                if (!entry.is_directory(ec)) continue;
                std::string dname = filenameToUtf8(entry.path());
                if (dname.empty() || dname[0] == '.' || dname == "attachments" || dname == "images") continue;

                fs::path rel = fs::relative(entry.path(), chFolder, ec);
                std::string joined;
                for (const auto& p : rel) {
                    std::string ps = p.u8string();
                    if (!ps.empty() && ps != "." && ps != "/" && ps != "\\") {
                        if (!joined.empty()) joined += "/";
                        joined += ps;
                    }
                }
                if (!joined.empty()) sections.insert(joined);
            }
        }
        for (const auto& c : getAllCards()) {
            if (c.chapter == chapter && !c.section.empty()) {
                sections.insert(c.section);
            }
        }
        return std::vector<std::string>(sections.begin(), sections.end());
    }

    void saveCustomPalette() {
        executeSQL("DELETE FROM kb_color_palette;");
        const char* sql = "INSERT INTO kb_color_palette (label, hex) VALUES (?, ?);";
        sqlite3_stmt* stmt = nullptr;
        if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) == SQLITE_OK) {
            for (const auto& nc : g_customPalette) {
                sqlite3_reset(stmt);
                std::string hex = nc.color.name().toStdString();
                sqlite3_bind_text(stmt, 1, nc.label.c_str(), -1, SQLITE_TRANSIENT);
                sqlite3_bind_text(stmt, 2, hex.c_str(), -1, SQLITE_TRANSIENT);
                sqlite3_step(stmt);
            }
        }
        sqlite3_finalize(stmt);
    }

    void loadCustomPalette() {
        g_customPalette.clear();
        const char* sql = "SELECT label, hex FROM kb_color_palette ORDER BY id ASC;";
        sqlite3_stmt* stmt = nullptr;
        if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) == SQLITE_OK) {
            while (sqlite3_step(stmt) == SQLITE_ROW) {
                std::string lbl = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
                std::string hex = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
                QColor c(QString::fromStdString(hex));
                if (c.isValid()) g_customPalette.push_back({lbl, c});
            }
        }
        sqlite3_finalize(stmt);
    }

    void saveSetting(const std::string& key, const std::string& val) {
        const char* sql = "INSERT INTO kb_app_settings (k, v) VALUES (?, ?) "
        "ON CONFLICT(k) DO UPDATE SET v = excluded.v;";
        sqlite3_stmt* stmt = nullptr;
        if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) == SQLITE_OK) {
            sqlite3_bind_text(stmt, 1, key.c_str(), -1, SQLITE_TRANSIENT);
            sqlite3_bind_text(stmt, 2, val.c_str(), -1, SQLITE_TRANSIENT);
            sqlite3_step(stmt);
        }
        sqlite3_finalize(stmt);
    }

    void saveConfig(const AppConfig& cfg) {
        saveSetting("isMaleTheme",         cfg.isMaleTheme ? "1" : "0");
        saveSetting("uiFontSize",          std::to_string(cfg.uiFontSize));
        saveSetting("cardScalePercent",    std::to_string(cfg.cardScalePercent));
        saveSetting("cardOpacity",         std::to_string(cfg.cardOpacity));
        saveSetting("enableBlurGlass",     cfg.enableBlurGlass ? "1" : "0");
        saveSetting("enableShadows",       cfg.enableShadows ? "1" : "0");
        saveSetting("shadowBlurRadius",    std::to_string(cfg.shadowBlurRadius));
        saveSetting("enableHoverFloat",    cfg.enableHoverFloat ? "1" : "0");
        saveSetting("useRightDrawer",      cfg.useRightDrawer ? "1" : "0");
        saveSetting("leftPanelWidth",      std::to_string(cfg.leftPanelWidth));
        saveSetting("rightDrawerWidth",    std::to_string(cfg.rightDrawerWidth));
        saveSetting("showTopSectionBar",   cfg.showTopSectionBar ? "1" : "0");
        saveSetting("showCodeField",       cfg.showCodeField ? "1" : "0");
        saveSetting("trackerCardsForFull", std::to_string(cfg.trackerCardsForFull));

        saveSetting("bgWindowTop",         cfg.bgWindowTop.name().toStdString());
        saveSetting("bgWindowBottom",      cfg.bgWindowBottom.name().toStdString());
        saveSetting("bgCardNormal",        cfg.bgCardNormal.name().toStdString());
        saveSetting("bgCardPinned",        cfg.bgCardPinned.name().toStdString());
        saveSetting("bgInputField",        cfg.bgInputField.name().toStdString());
        saveSetting("bgLeftPanel",         cfg.bgLeftPanel.name().toStdString());
        saveSetting("bgRightPanel",        cfg.bgRightPanel.name().toStdString());
        saveSetting("bgButton",            cfg.bgButton.name().toStdString());
        saveSetting("bgTrackerCard",       cfg.bgTrackerCard.name().toStdString());

        saveSetting("hkNewCard",           cfg.hkNewCard.toStdString());
        saveSetting("hkToggleVault",       cfg.hkToggleVault.toStdString());
        saveSetting("hkToggleDrawer",      cfg.hkToggleDrawer.toStdString());
        saveSetting("hkFormatPopup",       cfg.hkFormatPopup.toStdString());
        saveSetting("hkTracker",           cfg.hkTracker.toStdString());
        saveSetting("hkSettings",          cfg.hkSettings.toStdString());
        saveSetting("hkFocusSearch",       cfg.hkFocusSearch.toStdString());
    }

    void loadConfig(AppConfig& cfg) {
        const char* sql = "SELECT k, v FROM kb_app_settings;";
        sqlite3_stmt* stmt = nullptr;
        if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) == SQLITE_OK) {
            while (sqlite3_step(stmt) == SQLITE_ROW) {
                std::string k = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
                std::string v = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
                if (k == "isMaleTheme")         cfg.isMaleTheme = (v == "1");
                if (k == "uiFontSize")          cfg.uiFontSize = std::clamp(std::stoi(v), 8, 18);
                if (k == "cardScalePercent")    cfg.cardScalePercent = std::clamp(std::stoi(v), 60, 120);
                if (k == "cardOpacity")         cfg.cardOpacity = std::clamp(std::stoi(v), 80, 255);
                if (k == "enableBlurGlass")     cfg.enableBlurGlass = (v == "1");
                if (k == "enableShadows")       cfg.enableShadows = (v == "1");
                if (k == "shadowBlurRadius")    cfg.shadowBlurRadius = std::clamp(std::stoi(v), 5, 50);
                if (k == "enableHoverFloat")    cfg.enableHoverFloat = (v == "1");
                if (k == "useRightDrawer")      cfg.useRightDrawer = (v == "1");
                if (k == "leftPanelWidth")      cfg.leftPanelWidth = std::clamp(std::stoi(v), 200, 800);
                if (k == "rightDrawerWidth")    cfg.rightDrawerWidth = std::clamp(std::stoi(v), 240, 950);
                if (k == "showTopSectionBar")   cfg.showTopSectionBar = (v == "1");
                if (k == "showCodeField")       cfg.showCodeField = (v == "1");
                if (k == "trackerCardsForFull") cfg.trackerCardsForFull = std::clamp(std::stoi(v), 1, 100000);

                if (k == "bgWindowTop"    && QColor(QString::fromStdString(v)).isValid()) cfg.bgWindowTop    = QColor(QString::fromStdString(v));
                if (k == "bgWindowBottom" && QColor(QString::fromStdString(v)).isValid()) cfg.bgWindowBottom = QColor(QString::fromStdString(v));
                if (k == "bgCardNormal"   && QColor(QString::fromStdString(v)).isValid()) cfg.bgCardNormal   = QColor(QString::fromStdString(v));
                if (k == "bgCardPinned"   && QColor(QString::fromStdString(v)).isValid()) cfg.bgCardPinned   = QColor(QString::fromStdString(v));
                if (k == "bgInputField"   && QColor(QString::fromStdString(v)).isValid()) cfg.bgInputField   = QColor(QString::fromStdString(v));
                if (k == "bgLeftPanel"    && QColor(QString::fromStdString(v)).isValid()) cfg.bgLeftPanel    = QColor(QString::fromStdString(v));
                if (k == "bgRightPanel"   && QColor(QString::fromStdString(v)).isValid()) cfg.bgRightPanel   = QColor(QString::fromStdString(v));
                if (k == "bgButton"       && QColor(QString::fromStdString(v)).isValid()) cfg.bgButton       = QColor(QString::fromStdString(v));
                if (k == "bgTrackerCard"  && QColor(QString::fromStdString(v)).isValid()) cfg.bgTrackerCard  = QColor(QString::fromStdString(v));

                if (k == "hkNewCard")        cfg.hkNewCard = QString::fromStdString(v);
                if (k == "hkToggleVault")    cfg.hkToggleVault = QString::fromStdString(v);
                if (k == "hkToggleDrawer")   cfg.hkToggleDrawer = QString::fromStdString(v);
                if (k == "hkFormatPopup")    cfg.hkFormatPopup = QString::fromStdString(v);
                if (k == "hkTracker")        cfg.hkTracker = QString::fromStdString(v);
                if (k == "hkSettings")       cfg.hkSettings = QString::fromStdString(v);
                if (k == "hkFocusSearch")    cfg.hkFocusSearch = QString::fromStdString(v);
            }
        }
        sqlite3_finalize(stmt);
    }

    fs::path resolveCardFolder(const std::string& chapter, const std::string& section) {
        std::error_code ec;
        bool inSkillboxSubdir = fs::exists(vaultRoot / u8path("Skillbox") / u8path(chapter), ec);

        fs::path target = inSkillboxSubdir
        ? (vaultRoot / u8path("Skillbox") / u8path(chapter))
        : (vaultRoot / u8path(chapter));

        if (!section.empty() && section != "all") {
            QString secStr = QString::fromStdString(section);
            QStringList parts = secStr.split(QRegularExpression("[/\\\\]"), Qt::SkipEmptyParts);
            for (const QString& part : parts) {
                std::string safePart = part.trimmed().toStdString();
                for (char& ch : safePart) {
                    if (ch == ':' || ch == '*' || ch == '?' || ch == '"' || ch == '<' || ch == '>' || ch == '|') {
                        ch = '-';
                    }
                }
                if (!safePart.empty()) {
                    target /= u8path(safePart);
                }
            }
        }
        return target;
    }

    void saveCard(Card c, bool syncToDisk = true) {
        if (c.chapter.empty() || c.chapter == "__pinned__" || c.chapter == "all") c.chapter = "Общее";
        if (c.section.empty() || c.section == "all") c.section = "Заметки";
        if (c.title.empty())   c.title   = "Без названия";

        if (c.id == 0) {
            c.id = static_cast<long long>(std::time(nullptr)) * 1000 + (std::rand() % 999);
        }

        if (syncToDisk) {
            beginIgnoreFsWatcher(900);
            std::error_code ec;
            fs::path targetFolder = resolveCardFolder(c.chapter, c.section);
            fs::create_directories(targetFolder, ec);

            if (c.filePath.empty()) {
                std::string safeName = c.title;
                for (char& ch : safeName) {
                    if (ch == '/' || ch == '\\' || ch == ':' || ch == '*' || ch == '?' || ch == '"' ||
                        ch == '<' || ch == '>' || ch == '|' || ch == '#' || ch == '^' || ch == '[' || ch == ']') ch = '_';
                }
                fs::path candidate = targetFolder / u8path(safeName + ".md");
                int n = 2;
                while (fs::exists(candidate, ec)) {
                    candidate = targetFolder / u8path(safeName + " " + std::to_string(n++) + ".md");
                }
                c.filePath = pathToUtf8(candidate);
            } else {
                fs::path p = u8path(c.filePath);
                if (p.extension() != ".md") {
                    fs::path mdPath = p;
                    mdPath.replace_extension(".md");
                    if (fs::exists(p, ec) && !fs::exists(mdPath, ec)) fs::rename(p, mdPath, ec);
                    else if (fs::exists(p, ec)) fs::remove(p, ec);
                    c.filePath = pathToUtf8(mdPath);
                }
            }

            fs::path cardFsPath = u8path(c.filePath);
            fs::create_directories(cardFsPath.parent_path(), ec);
            std::ofstream out(cardFsPath, std::ios::binary | std::ios::trunc);
            if (out.is_open()) {
                out << MarkdownCodec::buildCardMarkdown(c);
                out.close();
            }
            c.fileMtime = fileMtime(c.filePath);
        }

        const char* sql = "INSERT INTO kb_cards (id, pinned, chapter, section, title, rawCode, desc_text, extended_desc, file_path, file_mtime) "
        "VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?) "
        "ON CONFLICT(id) DO UPDATE SET "
        "pinned=excluded.pinned, chapter=excluded.chapter, section=excluded.section, "
        "title=excluded.title, rawCode=excluded.rawCode, desc_text=excluded.desc_text, "
        "extended_desc=excluded.extended_desc, file_path=excluded.file_path, file_mtime=excluded.file_mtime;";

        sqlite3_stmt* stmt = nullptr;
        if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) == SQLITE_OK) {
            sqlite3_bind_int64(stmt, 1, c.id);
            sqlite3_bind_int(stmt, 2, c.pinned ? 1 : 0);
            sqlite3_bind_text(stmt, 3, c.chapter.c_str(), -1, SQLITE_TRANSIENT);
            sqlite3_bind_text(stmt, 4, c.section.c_str(), -1, SQLITE_TRANSIENT);
            sqlite3_bind_text(stmt, 5, c.title.c_str(), -1, SQLITE_TRANSIENT);
            sqlite3_bind_text(stmt, 6, c.code.c_str(), -1, SQLITE_TRANSIENT);
            sqlite3_bind_text(stmt, 7, c.description.c_str(), -1, SQLITE_TRANSIENT);
            sqlite3_bind_text(stmt, 8, c.extendedDesc.c_str(), -1, SQLITE_TRANSIENT);
            sqlite3_bind_text(stmt, 9, c.filePath.c_str(), -1, SQLITE_TRANSIENT);
            sqlite3_bind_int64(stmt, 10, c.fileMtime);
            sqlite3_step(stmt);
        }
        sqlite3_finalize(stmt);
    }

    void deleteCardRecordOnly(long long id) {
        const char* sql = "DELETE FROM kb_cards WHERE id = ?;";
        sqlite3_stmt* stmt = nullptr;
        if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) == SQLITE_OK) {
            sqlite3_bind_int64(stmt, 1, id);
            sqlite3_step(stmt);
        }
        sqlite3_finalize(stmt);
    }

    void deleteCard(long long id) {
        beginIgnoreFsWatcher(900);
        std::error_code ec;
        auto cards = getAllCards();
        for (const auto& c : cards) {
            if (c.id == id && !c.filePath.empty()) {
                fs::path fp = u8path(c.filePath);
                if (fs::exists(fp, ec)) {
                    fs::remove(fp, ec);
                }
            }
        }
        deleteCardRecordOnly(id);
    }

    std::vector<Card> getAllCards() {
        std::vector<Card> result;
        const char* sql = "SELECT id, pinned, chapter, section, title, rawCode, desc_text, extended_desc, file_path, file_mtime "
        "FROM kb_cards ORDER BY pinned DESC, id DESC;";
        sqlite3_stmt* stmt = nullptr;

        if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) == SQLITE_OK) {
            while (sqlite3_step(stmt) == SQLITE_ROW) {
                Card c;
                c.id           = sqlite3_column_int64(stmt, 0);
                c.pinned       = sqlite3_column_int(stmt, 1) != 0;
                c.chapter      = sqlite3_column_text(stmt, 2) ? reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2)) : "";
                c.section      = sqlite3_column_text(stmt, 3) ? reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3)) : "";
                c.title        = sqlite3_column_text(stmt, 4) ? reinterpret_cast<const char*>(sqlite3_column_text(stmt, 4)) : "";
                c.code         = sqlite3_column_text(stmt, 5) ? reinterpret_cast<const char*>(sqlite3_column_text(stmt, 5)) : "";
                c.description  = sqlite3_column_text(stmt, 6) ? reinterpret_cast<const char*>(sqlite3_column_text(stmt, 6)) : "";
                c.extendedDesc = sqlite3_column_text(stmt, 7) ? reinterpret_cast<const char*>(sqlite3_column_text(stmt, 7)) : "";
                c.filePath     = sqlite3_column_text(stmt, 8) ? reinterpret_cast<const char*>(sqlite3_column_text(stmt, 8)) : "";
                c.fileMtime    = sqlite3_column_int64(stmt, 9);
                result.push_back(c);
            }
        }
        sqlite3_finalize(stmt);
        return result;
    }

    static long long fileMtime(const std::string& utf8PathStr) {
        std::error_code ec;
        auto t = fs::last_write_time(u8path(utf8PathStr), ec);
        if (ec) return 0;
        return static_cast<long long>(t.time_since_epoch().count());
    }

    static bool isNoteFile(const fs::path& p) {
        std::string ext = p.extension().u8string();
        std::transform(ext.begin(), ext.end(), ext.begin(), [](unsigned char ch) { return std::tolower(ch); });
        return ext == ".md" || ext == ".markdown" || ext == ".txt";
    }

    static bool isIgnoredRelPath(const fs::path& rel) {
        for (const auto& part : rel) {
            std::string s = part.u8string();
            if (s.empty()) continue;
            if (s[0] == '.' && s != "." && s != "..") return true;
            if (s == "attachments" || s == "images") return true;
        }
        return false;
    }

    static std::string canonicalOf(const fs::path& p) {
        std::error_code ec;
        fs::path c = fs::weakly_canonical(p, ec);
        return ec ? pathToUtf8(p) : pathToUtf8(c);
    }

    void fillCardFromFile(Card& card, const fs::path& filePath) {
        std::ifstream in(filePath, std::ios::binary);
        std::stringstream buffer;
        buffer << in.rdbuf();
        MarkdownCodec::ParsedNote parsed = MarkdownCodec::parseMarkdownFile(buffer.str());

        std::error_code ec;
        fs::path relDir = fs::relative(filePath.parent_path(), vaultRoot, ec);
        auto [chapter, section] = extractChapterAndSectionFromRelDir(relDir);

        auto metaOr = [&](const char* key, const std::string& def) {
            auto it = parsed.meta.find(key);
            return (it != parsed.meta.end() && !it->second.empty()) ? it->second : def;
        };

        if (chapter.empty() || chapter == "all") chapter = metaOr("chapter", "Общее");
        if (section.empty() || section == "all") section = metaOr("section", "Заметки");

        card.chapter = chapter;
        card.section = section;
        card.title   = !parsed.title.empty() ? parsed.title : metaOr("title", stemToUtf8(filePath));

        auto pinIt = parsed.meta.find("pinned");
        if (pinIt != parsed.meta.end()) card.pinned = (pinIt->second == "true" || pinIt->second == "1");

        if (card.id == 0) {
            auto idIt = parsed.meta.find("id");
            if (idIt != parsed.meta.end()) {
                try { card.id = std::stoll(idIt->second); } catch (...) { card.id = 0; }
            }
        }

        fs::path mdDir = filePath.parent_path();
        card.code         = parsed.code;
        card.description  = MarkdownCodec::fromMarkdown(parsed.descMd, mdDir);
        card.extendedDesc = MarkdownCodec::fromMarkdown(parsed.extMd, mdDir);
        card.filePath     = pathToUtf8(filePath);
        card.fileMtime    = fileMtime(card.filePath);
    }

    bool syncDirectoryWithDatabase() {
        bool anyChanged = false;
        std::error_code ec;
        if (!fs::exists(vaultRoot, ec)) fs::create_directories(vaultRoot, ec);

        auto existingCards = getAllCards();
        std::map<std::string, Card> byPath;
        std::set<long long> usedIds;

        for (const auto& c : existingCards) {
            usedIds.insert(c.id);
            if (c.filePath.empty()) continue;
            fs::path fp = u8path(c.filePath);
            if (!fs::exists(fp, ec)) {
                deleteCardRecordOnly(c.id);
                anyChanged = true;
                continue;
            }
            byPath[canonicalOf(fp)] = c;
        }

        long long nextId = static_cast<long long>(std::time(nullptr)) * 1000 + (std::rand() % 999);

        for (const auto& entry : fs::recursive_directory_iterator(vaultRoot, fs::directory_options::skip_permission_denied, ec)) {
            if (!entry.is_regular_file(ec)) continue;
            fs::path rel = fs::relative(entry.path(), vaultRoot, ec);
            if (isIgnoredRelPath(rel) || !isNoteFile(entry.path())) continue;

            std::string canon = canonicalOf(entry.path());
            std::string entryPathUtf8 = pathToUtf8(entry.path());
            long long mt = fileMtime(entryPathUtf8);

            fs::path relDir = fs::relative(entry.path().parent_path(), vaultRoot, ec);
            auto [expectedCh, expectedSec] = extractChapterAndSectionFromRelDir(relDir);

            auto it = byPath.find(canon);
            if (it != byPath.end()) {
                Card known = it->second;
                bool folderMismatch = (expectedCh != "all" && expectedSec != "all" &&
                (known.chapter != expectedCh || known.section != expectedSec));

                if (known.fileMtime == 0 || (mt != 0 && mt != known.fileMtime)) {
                    fillCardFromFile(known, entry.path());
                    if (expectedCh != "all")  known.chapter = expectedCh;
                    if (expectedSec != "all") known.section = expectedSec;
                    saveCard(known, false);
                    anyChanged = true;
                } else if (folderMismatch) {
                    known.chapter   = expectedCh;
                    known.section   = expectedSec;
                    known.filePath  = entryPathUtf8;
                    known.fileMtime = mt;
                    saveCard(known, false);
                    anyChanged = true;
                }
                continue;
            }

            Card nc;
            fillCardFromFile(nc, entry.path());
            if (nc.id == 0 || usedIds.count(nc.id) > 0) nc.id = ++nextId;
            while (usedIds.count(nc.id) > 0) nc.id = ++nextId;
            usedIds.insert(nc.id);
            saveCard(nc, false);
            anyChanged = true;
        }

        return anyChanged;
    }
};

// ============================================================================
// 8. ВКЛАДКА «ТРЕКЕР» ДЛЯ ОКНА НАСТРОЕК
// ============================================================================
class TrackerSettingsTab : public QWidget {
public:
    explicit TrackerSettingsTab(DatabaseManager& dbRef, QWidget* parent = nullptr) : QWidget(parent) {
        QVBoxLayout* l = new QVBoxLayout(this);
        l->setContentsMargins(18, 18, 18, 18);
        l->setSpacing(12);

        QFont boldF = font();
        boldF.setBold(true);

        QLabel* title = new QLabel("📊 Лимит шкалы прогресса (Трекер)", this);
        title->setFont(boldF);

        QLabel* descr = new QLabel(
            "Сколько карточек нужно создать в директории/теме, чтобы её шкала заполнилась на 100%.\n"
            "Например: 100 → 1 карточка = 1%, 200 → 1 карточка = 0.5%, 50 → 1 карточка = 2%.",
            this
        );
        descr->setWordWrap(true);

        QHBoxLayout* row = new QHBoxLayout();
        QLabel* spinLbl = new QLabel("Максимум шкалы (100%):", this);
        QSpinBox* spin = new QSpinBox(this);
        spin->setRange(1, 100000);
        spin->setValue(g_cfg.trackerCardsForFull);
        spin->setSuffix(" карт.");
        spin->setFixedHeight(32);
        spin->setMinimumWidth(140);
        row->addWidget(spinLbl);
        row->addWidget(spin);
        row->addStretch(1);

        QHBoxLayout* presets = new QHBoxLayout();
        for (int v : {10, 25, 50, 100, 200, 500, 1000}) {
            NeonButton* b = new NeonButton(QString::number(v), QColor("#38bdf8"), this);
            b->setFixedHeight(30);
            connect(b, &QPushButton::clicked, [spin, v]() { spin->setValue(v); });
            presets->addWidget(b);
        }
        presets->addStretch(1);

        QLabel* hint = new QLabel(this);
        hint->setFont(boldF);
        auto refreshHint = [hint](int v) {
            hint->setText(QString("Сейчас: %1 карточек = 100%  (1 карточка = %2% шкалы)")
            .arg(v)
            .arg(100.0 / v, 0, 'f', v > 100 ? 2 : 1));
        };
        refreshHint(spin->value());

        connect(spin, QOverload<int>::of(&QSpinBox::valueChanged), [&dbRef, refreshHint](int v) {
            g_cfg.trackerCardsForFull = v;
            dbRef.saveConfig(g_cfg);
            refreshHint(v);
        });

        l->addWidget(title);
        l->addWidget(descr);
        l->addLayout(row);
        l->addLayout(presets);
        l->addWidget(hint);
        l->addStretch(1);
    }
};
